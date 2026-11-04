// Package session turns app requests into Slack calls, and Slack data into the
// ordered event stream src/slack/slackmapper.cpp expects.
//
// Event order matters: the app crashes or drops messages if a chat's position
// arrives before the chat, a message before its sender, or a last_message
// before its message. All events go through one queue to keep that order.
package session

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"sync"
	"time"

	"yottaslack/internal/api"
	"yottaslack/internal/ids"
	"yottaslack/internal/text"
)

const (
	eventQueueSize  = 1024
	batchSize       = 200
	pollInterval    = 5 * time.Second
	maxHistoryLimit = 100
	pollFirstLimit  = 15
	maxNewerPages   = 5
	newerPageSize   = 200
	maxLoginBackoff = 30 * time.Second
	listRetries     = 3
	previewLimit    = 50
	maxDownloads    = 4
	markDelay       = 2 * time.Second
	offlineCheck    = 5 * time.Second

	slackbotID = "USLACKBOT"

	folderChannels = 1
	folderDirect   = 2
	folderApps     = 3

	// conversations.history allows about 50 calls a minute. This uses 30, the
	// rest is for opening chats
	previewInterval = 2 * time.Second

	// Smaller message ids are row indexes or other junk from the UI
	minPlausibleMessageID = int64(100000000000000)

	codeBadRequest   = 400
	codeUnauthorized = 401
	codeFailed       = 500
)

type request struct {
	ID     uint64          `json:"id"`
	Method string          `json:"method"`
	Params json.RawMessage `json:"params"`
}

type user struct {
	SlackID   string `json:"slack_id"`
	ID        int64  `json:"id"`
	FirstName string `json:"first_name"`
	LastName  string `json:"last_name"`
	Username  string `json:"username"`
	Avatar    string `json:"avatar,omitempty"`
	AvatarBig string `json:"avatar_big,omitempty"`
	Bot       bool   `json:"bot,omitempty"`

	Photo      *wirePhoto `json:"photo,omitempty"`
	avatarSent bool
}

type wireChat struct {
	ID           int64  `json:"id"`
	Type         string `json:"type"`
	Title        string `json:"title"`
	UserID       int64  `json:"user_id,omitempty"`
	SupergroupID int64  `json:"supergroup_id,omitempty"`
	Order        int64  `json:"order"`
	Folder       int    `json:"folder"`
	ReadOnly     bool   `json:"read_only,omitempty"`
	dm           bool

	Photo *wirePhoto `json:"photo,omitempty"`
}

type message struct {
	ID       int64  `json:"id"`
	Chat     int64  `json:"chat"`
	Sender   int64  `json:"sender"`
	Outgoing bool   `json:"outgoing"`
	Date     int64  `json:"date"`
	Text     string `json:"text"`
	EditDate int64  `json:"edit_date,omitempty"`

	Entities []text.Entity `json:"entities,omitempty"`
	Media    *media        `json:"media,omitempty"`
	Preview  *linkPreview  `json:"preview,omitempty"`

	Reactions []wireReaction `json:"reactions,omitempty"`
}

type chat struct {
	id       int64
	slackID  string
	order    int64
	loaded   bool
	newest   int64
	inflight int
	sendGen  uint64
	stopPoll context.CancelFunc

	// 0 for direct messages
	supergroupID int64
	dm           bool

	lastRead int64
	unread   []int64
	notified []int32
	marking  bool

	// Held for writing during a send and for reading during a history request.
	// A history reply during a send can contain the message under its real id
	// while the app still has it under a temporary one, and the swap then
	// leaves it in the list twice.
	sending sync.RWMutex
}

// Session is one logged-in Slack account.
type Session struct {
	api          *api.Client
	appAPI       *api.Client
	ids          *ids.Store
	usersPath    string
	pollInterval time.Duration

	ctx      context.Context
	cancel   context.CancelFunc
	events   chan string
	closeOne sync.Once

	mu     sync.Mutex
	me     *user
	users  map[string]*user
	chats  map[int64]*chat
	tempID int64

	previewInterval time.Duration
	markDelay       time.Duration

	filesDir         string
	files            map[string]*fileEntry
	filesByID        map[int32]*fileEntry
	nextFile         int32
	downloads        chan struct{}
	nextNotification int32
	offline          bool

	bySlackID    map[string]*chat
	own          map[int64]bool
	reactions    map[int64][]api.Reaction
	customEmoji  map[string]customEmoji
	customNames  map[int64]string
	nextOrder    int64
	stopRealtime context.CancelFunc
	realtimeUp   bool
	synced       bool
	appToken     string
	readOnly     bool
}

// New creates a session storing its state in dataDir.
func New(dataDir string) (*Session, error) {
	if err := os.MkdirAll(dataDir, 0o700); err != nil {
		return nil, err
	}

	filesDir := filepath.Join(dataDir, "files")
	if err := os.MkdirAll(filesDir, 0o700); err != nil {
		return nil, err
	}

	store, err := ids.Open(filepath.Join(dataDir, "ids.json"))
	if err != nil {
		return nil, err
	}

	ctx, cancel := context.WithCancel(context.Background())
	s := &Session{
		api:             api.New(),
		appAPI:          api.New(),
		ids:             store,
		usersPath:       filepath.Join(dataDir, "users.json"),
		pollInterval:    pollInterval,
		previewInterval: previewInterval,
		markDelay:       markDelay,
		filesDir:        filesDir,
		files:           map[string]*fileEntry{},
		filesByID:       map[int32]*fileEntry{},
		downloads:       make(chan struct{}, maxDownloads),
		ctx:             ctx,
		cancel:          cancel,
		events:          make(chan string, eventQueueSize),
		users:           map[string]*user{},
		chats:           map[int64]*chat{},
		bySlackID:       map[string]*chat{},
		own:             map[int64]bool{},
		reactions:       map[int64][]api.Reaction{},
		customEmoji:     map[string]customEmoji{},
		customNames:     map[int64]string{},
	}

	s.api.RateLimited = func(method string, wait time.Duration) {
		s.log("%s rate limited for %v", method, wait)
	}
	s.api.SlowCall = func(method string, took time.Duration, stages string) {
		s.log("%s took %v: %s", method, took, stages)
	}
	return s, nil
}

// Next returns the next event, or false on timeout or after Close.
func (s *Session) Next(timeout time.Duration) (string, bool) {
	timer := time.NewTimer(timeout)
	defer timer.Stop()

	select {
	case event := <-s.events:
		return event, true
	case <-timer.C:
		return "", false
	case <-s.ctx.Done():
		return "", false
	}
}

// Close stops all work. Pending events are dropped.
func (s *Session) Close() {
	s.closeOne.Do(func() {
		s.cancel()
		s.ids.Save()
	})
}

// Handle processes one JSON request without blocking the caller.
func (s *Session) Handle(line []byte) {
	var r request
	if err := json.Unmarshal(line, &r); err != nil {
		s.log("invalid request: %v", err)
		return
	}
	s.spawn(func() { s.dispatch(r) })
}

// spawn keeps a Go panic from taking the whole app down with it.
func (s *Session) spawn(work func()) {
	go func() {
		defer func() {
			if r := recover(); r != nil {
				s.log("panic: %v", r)
			}
		}()
		work()
	}()
}

func (s *Session) emit(value any) {
	content, err := json.Marshal(value)
	if err != nil {
		return
	}

	select {
	case s.events <- string(content):
	case <-s.ctx.Done():
	}
}

func (s *Session) log(format string, args ...any) {
	s.emit(map[string]any{"event": "log", "message": fmt.Sprintf(format, args...)})
}

func (s *Session) reply(id uint64, result map[string]any) {
	s.emit(map[string]any{"id": id, "result": result})
}

func (s *Session) replyOK(id uint64) {
	s.reply(id, map[string]any{"type": "ok"})
}

func (s *Session) replyError(id uint64, code int, message string) {
	s.emit(map[string]any{"id": id, "error": map[string]any{"code": code, "message": message}})
}

func (s *Session) dispatch(r request) {
	var err error

	switch r.Method {
	case "login":
		var p struct {
			Token    string `json:"token"`
			AppToken string `json:"app_token"`
			Retry    bool   `json:"retry"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.login(r.ID, p.Token, p.AppToken, p.Retry)
		}
	case "realtime":
		var p struct {
			Token string `json:"token"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.setAppToken(p.Token)
			s.replyOK(r.ID)
		}
	case "me":
		s.getMe(r.ID)
	case "history":
		var p struct {
			Chat   int64 `json:"chat"`
			From   int64 `json:"from"`
			Offset int   `json:"offset"`
			Limit  int   `json:"limit"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.history(r.ID, p.Chat, p.From, p.Offset, p.Limit)
		}
	case "send":
		var p struct {
			Chat int64  `json:"chat"`
			Text string `json:"text"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.send(r.ID, p.Chat, p.Text)
		}
	case "view":
		var p struct {
			Chat    int64 `json:"chat"`
			Message int64 `json:"message"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.view(r.ID, p.Chat, p.Message)
		}
	case "download", "cancel_download":
		var p struct {
			File int32 `json:"file"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			if r.Method == "download" {
				s.download(r.ID, p.File)
			} else {
				s.cancelDownload(r.ID, p.File)
			}
		}
	case "network":
		var p struct {
			Online bool `json:"online"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.mu.Lock()
			s.offline = !p.Online
			s.mu.Unlock()
			s.replyOK(r.ID)
		}
	case "react":
		var p struct {
			Chat    int64  `json:"chat"`
			Message int64  `json:"message"`
			Emoji   string `json:"emoji"`
			Custom  int64  `json:"custom"`
			Add     bool   `json:"add"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.react(r.ID, p.Chat, p.Message, p.Emoji, p.Custom, p.Add)
		}
	case "custom_emojis":
		var p struct {
			IDs []int64 `json:"ids"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.customEmojis(r.ID, p.IDs)
		}
	case "available_reactions":
		s.availableReactions(r.ID)
	case "members":
		var p struct {
			Query string `json:"query"`
			Limit int    `json:"limit"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.searchMembers(r.ID, p.Query, p.Limit)
		}
	case "upload":
		var p struct {
			Chat  int64    `json:"chat"`
			Files []upload `json:"files"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.sendFiles(r.ID, p.Chat, p.Files)
		}
	case "properties":
		var p struct {
			Message int64 `json:"message"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.properties(r.ID, p.Message)
		}
	case "edit":
		var p struct {
			Chat    int64  `json:"chat"`
			Message int64  `json:"message"`
			Text    string `json:"text"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.edit(r.ID, p.Chat, p.Message, p.Text)
		}
	case "delete":
		var p struct {
			Chat     int64   `json:"chat"`
			Messages []int64 `json:"messages"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.deleteMessages(r.ID, p.Chat, p.Messages)
		}
	case "open", "close":
		var p struct {
			Chat int64 `json:"chat"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			if r.Method == "open" {
				s.open(p.Chat)
			} else {
				s.closeChat(p.Chat)
			}
			s.replyOK(r.ID)
		}
	default:
		s.replyError(r.ID, codeBadRequest, "unknown method "+r.Method)
	}

	if err != nil {
		s.replyError(r.ID, codeBadRequest, err.Error())
	}
}

func (s *Session) login(id uint64, token, appToken string, retry bool) {
	s.api.SetToken(token)

	selfID, err := s.authTest(retry)
	if err != nil {
		s.replyLoginError(id, err)
		return
	}

	self, err := s.api.UserInfo(s.ctx, selfID)
	if err != nil {
		s.replyLoginError(id, err)
		return
	}

	me := s.makeUser(self)
	s.mu.Lock()
	s.me = me
	s.users[me.SlackID] = me
	s.mu.Unlock()

	if scopes := s.api.Scopes(); scopes != "" {
		s.log("token scopes: %s", scopes)

		s.mu.Lock()
		s.readOnly = !hasScope(scopes, "chat:write")
		s.mu.Unlock()
	}

	s.replyOK(id)
	s.emit(map[string]any{"event": "auth_ready", "my_id": me.ID})
	s.emit(map[string]any{"event": "users", "users": []*user{me}})

	s.sync()

	s.mu.Lock()
	s.synced = true
	if appToken == "" {
		appToken = s.appToken
	}
	s.mu.Unlock()

	s.startRealtime(appToken)
	s.spawn(s.loadPreviews)
}

func hasScope(scopes, scope string) bool {
	for _, item := range strings.Split(scopes, ",") {
		if strings.TrimSpace(item) == scope {
			return true
		}
	}
	return false
}

func (s *Session) isReadOnly() bool {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.readOnly
}

// refuseWrite is for a token without write scopes. Slack would refuse too, this
// gives a clear answer and never lets a request leave the phone.
func (s *Session) refuseWrite(id uint64) bool {
	if s.isReadOnly() {
		s.replyError(id, codeBadRequest, "This workspace is connected read-only")
		return true
	}
	return false
}

// setAppToken holds a token set during login until the chats are known, as
// events for them would otherwise look like new chats.
func (s *Session) setAppToken(token string) {
	s.mu.Lock()
	s.appToken = token
	synced := s.synced
	s.mu.Unlock()

	if synced {
		s.startRealtime(token)
	}
}

// authTest retries network failures when logging in with a stored token, as
// the phone may simply not be online yet. Answers from Slack are final.
func (s *Session) authTest(retry bool) (string, error) {
	backoff := time.Second

	for {
		selfID, err := s.api.AuthTest(s.ctx)
		if err == nil || !retry || api.IsAPIError(err) {
			return selfID, err
		}

		select {
		case <-time.After(backoff):
		case <-s.ctx.Done():
			return "", s.ctx.Err()
		}

		if backoff *= 2; backoff > maxLoginBackoff {
			backoff = maxLoginBackoff
		}
	}
}

func (s *Session) replyLoginError(id uint64, err error) {
	code := codeFailed
	if api.IsAuthError(err) {
		code = codeUnauthorized
	}
	s.replyError(id, code, err.Error())
}

// sync sends everything the chat list needs. Users must come first: a DM's
// title and every message preview depend on them.
func (s *Session) sync() {
	s.loadCustomEmoji()

	cached := s.loadUserCache()

	if len(cached) > 0 {
		s.emitUsers(cached)
	} else if err := s.refreshUsers(); err != nil {
		s.log("users.list failed: %v", err)
	}

	s.emitChats(s.loadConversations())
	s.emit(map[string]any{"event": "ready"})
	s.emit(map[string]any{"event": "folders", "folders": []map[string]any{
		{"id": folderChannels, "name": "Channels", "icon": "Channels"},
		{"id": folderDirect, "name": "Direct messages", "icon": "Private"},
		{"id": folderApps, "name": "Apps", "icon": "Bots"},
	}})

	if err := s.ids.Save(); err != nil {
		s.log("saving ids failed: %v", err)
	}

	if len(cached) > 0 {
		if err := s.refreshUsers(); err != nil {
			s.log("users.list failed: %v", err)
		}
	}
}

// loadConversations retries, as an empty list looks like having no chats.
func (s *Session) loadConversations() []api.Conversation {
	backoff := time.Second

	for attempt := 1; ; attempt++ {
		conversations, err := s.api.Conversations(s.ctx)
		if err == nil {
			return conversations
		}

		s.log("users.conversations failed: %v", err)
		if attempt == listRetries || api.IsAuthError(err) {
			return nil
		}

		select {
		case <-time.After(backoff):
		case <-s.ctx.Done():
			return nil
		}
		backoff *= 2
	}
}

func (s *Session) loadUserCache() []*user {
	content, err := os.ReadFile(s.usersPath)
	if err != nil {
		return nil
	}

	var cached []*user
	if err := json.Unmarshal(content, &cached); err != nil {
		s.log("ignoring user cache: %v", err)
		return nil
	}

	s.mu.Lock()
	defer s.mu.Unlock()
	for _, u := range cached {
		if _, known := s.users[u.SlackID]; !known {
			s.users[u.SlackID] = u
		}
	}
	return cached
}

// refreshUsers fetches the workspace, sends what changed and updates the cache.
func (s *Session) refreshUsers() error {
	slackUsers, err := s.api.Users(s.ctx)
	if err != nil {
		return err
	}

	var changed []*user
	all := make([]*user, 0, len(slackUsers))

	s.mu.Lock()
	for _, slackUser := range slackUsers {
		u := s.makeUser(slackUser)
		all = append(all, u)
		known, ok := s.users[u.SlackID]
		if ok {
			u.avatarSent = known.avatarSent
		}
		if !ok || *known != *u {
			s.users[u.SlackID] = u
			changed = append(changed, u)
		}
	}
	s.mu.Unlock()

	s.emitUsers(changed)

	content, err := json.Marshal(all)
	if err != nil {
		return err
	}
	return ids.WriteFile(s.usersPath, content)
}

func (s *Session) emitUsers(users []*user) {
	for start := 0; start < len(users); start += batchSize {
		end := start + batchSize
		if end > len(users) {
			end = len(users)
		}
		s.emit(map[string]any{"event": "users", "users": users[start:end]})
	}
}

// uploadedAvatar drops what Slack fills in for people without a photo: a
// Gravatar lookup of their email, or one of its generic pictures.
func uploadedAvatar(address string) string {
	if strings.Contains(address, "gravatar.com") || strings.Contains(address, "/avatars/ava_") {
		return ""
	}
	return address
}

func (s *Session) makeUser(slackUser api.User) *user {
	name := slackUser.Profile.DisplayName
	if name == "" {
		name = slackUser.Profile.RealName
	}
	if name == "" {
		name = slackUser.Name
	}

	return &user{
		SlackID:   slackUser.ID,
		ID:        s.ids.User(slackUser.ID),
		FirstName: name,
		Username:  slackUser.Name,
		Avatar:    uploadedAvatar(slackUser.Profile.Image72),
		AvatarBig: uploadedAvatar(slackUser.Profile.Image192),
		Bot:       slackUser.IsBot || slackUser.IsApp || slackUser.ID == slackbotID,
	}
}

// ensureUser returns a user, fetching and announcing it if it's new.
func (s *Session) ensureUser(slackID string) *user {
	s.mu.Lock()
	known := s.users[slackID]
	s.mu.Unlock()
	if known != nil {
		return known
	}

	slackUser, err := s.api.UserInfo(s.ctx, slackID)
	if err != nil {
		s.log("users.info %s failed: %v", slackID, err)
		slackUser = api.User{ID: slackID, Name: slackID}
	}

	return s.addUser(s.makeUser(slackUser))
}

func (s *Session) addUser(u *user) *user {
	s.mu.Lock()
	if known := s.users[u.SlackID]; known != nil {
		s.mu.Unlock()
		return known
	}
	s.users[u.SlackID] = u
	s.mu.Unlock()

	s.emit(map[string]any{"event": "users", "users": []*user{u}})
	return u
}

func (s *Session) emitChats(conversations []api.Conversation) {
	chats := make([]wireChat, 0, len(conversations))

	for _, conversation := range conversations {
		entry := s.makeWireChat(conversation)
		chats = append(chats, entry)
		s.trackChat(entry, conversation.ID)
	}

	// The API doesn't expose the sidebar, so this is its default layout:
	// channels, then direct messages. Chats keep these positions, activity
	// doesn't reorder them.
	sort.SliceStable(chats, func(a, b int) bool {
		if chats[a].dm != chats[b].dm {
			return chats[b].dm
		}
		return strings.ToLower(chats[a].Title) < strings.ToLower(chats[b].Title)
	})

	s.mu.Lock()
	for i := range chats {
		chats[i].Order = int64(len(chats) - i)
		s.chats[chats[i].ID].order = chats[i].Order
	}
	s.nextOrder = int64(len(chats))
	s.mu.Unlock()

	for start := 0; start < len(chats); start += batchSize {
		end := start + batchSize
		if end > len(chats) {
			end = len(chats)
		}
		s.emit(map[string]any{"event": "chats", "chats": chats[start:end]})
	}
}

func (s *Session) makeWireChat(conversation api.Conversation) wireChat {
	entry := wireChat{
		ID:       s.ids.Chat(conversation.ID),
		Title:    s.conversationTitle(conversation),
		ReadOnly: s.isReadOnly(),
		dm:       conversation.IsIM || conversation.IsMpim,
	}

	entry.Folder = folderChannels
	if entry.dm {
		entry.Folder = folderDirect
	}

	if conversation.IsIM {
		u := s.ensureUser(conversation.User)
		entry.Type = "private"
		entry.UserID = u.ID
		if u.Bot {
			entry.Folder = folderApps
		}
		if u.Avatar != "" {
			entry.Photo = s.avatarPhoto(u)
		}
	} else {
		entry.Type = "group"
		entry.SupergroupID = s.ids.Supergroup(conversation.ID)
	}
	return entry
}

func (s *Session) trackChat(entry wireChat, slackID string) *chat {
	s.mu.Lock()
	defer s.mu.Unlock()

	c := s.chats[entry.ID]
	if c == nil {
		c = &chat{id: entry.ID, slackID: slackID, supergroupID: entry.SupergroupID, dm: entry.dm}
		s.chats[entry.ID] = c
		s.bySlackID[slackID] = c
	}
	return c
}

func (s *Session) conversationTitle(conversation api.Conversation) string {
	switch {
	case conversation.IsIM:
		return s.ensureUser(conversation.User).FirstName
	case conversation.IsMpim:
		return mpimTitle(conversation.Name)
	default:
		return "#" + conversation.Name
	}
}

// mpimTitle turns "mpdm-ann--bob--cid-1" into "ann, bob, cid".
func mpimTitle(name string) string {
	name = strings.TrimPrefix(name, "mpdm-")
	if index := strings.LastIndex(name, "-"); index > 0 {
		name = name[:index]
	}
	return strings.Join(strings.Split(name, "--"), ", ")
}

func (s *Session) getMe(id uint64) {
	s.mu.Lock()
	me := s.me
	s.mu.Unlock()

	if me == nil {
		s.replyError(id, codeUnauthorized, "not logged in")
		return
	}
	s.reply(id, map[string]any{"type": "user", "user": me})
}

func (s *Session) findChat(id int64) *chat {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.chats[id]
}

// sender returns the app user for a message, inventing one for bots and
// integrations, which have no Slack user.
func (s *Session) sender(m api.Message) *user {
	if m.User != "" {
		return s.ensureUser(m.User)
	}

	key := m.BotID
	if key == "" {
		key = "unknown"
	}

	name := m.Username
	if name == "" && m.BotProfile != nil {
		name = m.BotProfile.Name
	}
	if name == "" {
		name = "Bot"
	}

	return s.addUser(&user{SlackID: key, ID: s.ids.User(key), FirstName: name})
}

func (s *Session) userName(slackID string) string {
	return s.ensureUser(slackID).FirstName
}

func (s *Session) toMessage(c *chat, m api.Message) (message, error) {
	id, err := ids.ParseTS(m.TS)
	if err != nil {
		return message{}, err
	}

	sender := s.sender(m)
	s.announceAvatar(sender)

	s.mu.Lock()
	outgoing := s.me != nil && sender.ID == s.me.ID
	if outgoing {
		s.own[id] = true
	}
	s.mu.Unlock()

	var editDate int64
	if m.Edited != nil {
		if edited, err := ids.ParseTS(m.Edited.TS); err == nil {
			editDate = edited / 1000000
		}
	}

	s.mu.Lock()
	if len(m.Reactions) > 0 {
		s.reactions[id] = m.Reactions
	} else {
		delete(s.reactions, id)
	}
	s.mu.Unlock()

	content := s.render(m)
	return message{
		ID:        id,
		Chat:      c.id,
		Sender:    sender.ID,
		Outgoing:  outgoing,
		Date:      id / 1000000,
		Text:      content.Text,
		EditDate:  editDate,
		Entities:  content.Entities,
		Media:     content.Media,
		Preview:   content.Preview,
		Reactions: s.wireReactions(m.Reactions),
	}, nil
}

func (s *Session) toMessages(c *chat, messages []api.Message) []message {
	converted := make([]message, 0, len(messages))
	for _, m := range messages {
		item, err := s.toMessage(c, m)
		if err != nil {
			s.log("skipping message: %v", err)
			continue
		}
		converted = append(converted, item)
	}
	return converted
}

func (s *Session) emitLastMessage(m message) {
	c := s.findChat(m.Chat)

	s.mu.Lock()
	order := c.order
	s.mu.Unlock()

	s.emit(map[string]any{"event": "last_message", "chat": m.Chat, "order": order, "message": m})
}

// fetchNewer returns up to limit messages after oldest, oldest first. It sorts
// itself so it doesn't depend on which end of the range Slack returns.
func (s *Session) fetchNewer(c *chat, oldest int64, limit int) ([]api.Message, error) {
	var messages []api.Message
	cursor := ""

	for page := 0; page < maxNewerPages; page++ {
		result, err := s.api.History(s.ctx, c.slackID, api.HistoryParams{
			Oldest: ids.FormatTS(oldest),
			Limit:  newerPageSize,
			Cursor: cursor,
		})
		if err != nil {
			return nil, err
		}

		messages = append(messages, result.Messages...)
		cursor = result.NextCursor
		if cursor == "" {
			break
		}
	}

	sort.SliceStable(messages, func(a, b int) bool { return tsLess(messages[a].TS, messages[b].TS) })
	if len(messages) > limit {
		messages = messages[:limit]
	}
	return messages, nil
}

func tsLess(a, b string) bool {
	first, _ := ids.ParseTS(a)
	second, _ := ids.ParseTS(b)
	return first < second
}

func clampLimit(limit int) int {
	if limit < 1 {
		return 1
	}
	if limit > maxHistoryLimit {
		return maxHistoryLimit
	}
	return limit
}

func (s *Session) fetchHistory(c *chat, from int64, offset, limit int) ([]api.Message, error) {
	if from < minPlausibleMessageID {
		page, err := s.api.History(s.ctx, c.slackID, api.HistoryParams{Limit: clampLimit(limit)})
		return page.Messages, err
	}

	// Same meaning as TDLib: a negative offset also returns -offset messages
	// starting at from and going forward.
	var messages []api.Message
	older := limit
	if offset < 0 {
		newer, err := s.fetchNewer(c, from, -offset)
		if err != nil {
			return nil, err
		}
		messages = append(messages, newer...)
		older = limit + offset
	}

	if older > 0 {
		page, err := s.api.History(s.ctx, c.slackID, api.HistoryParams{
			Latest:    ids.FormatTS(from),
			Inclusive: offset < 0,
			Limit:     clampLimit(older),
		})
		if err != nil {
			return nil, err
		}
		messages = append(messages, page.Messages...)
	}
	return messages, nil
}

func (s *Session) history(id uint64, chatID, from int64, offset, limit int) {
	c := s.findChat(chatID)
	if c == nil {
		s.replyError(id, codeBadRequest, "unknown chat")
		return
	}

	c.sending.RLock()
	defer c.sending.RUnlock()

	fetched, err := s.fetchHistory(c, from, offset, limit)
	if err != nil {
		s.replyError(id, codeFailed, err.Error())
		return
	}

	messages := s.toMessages(c, fetched)
	sort.SliceStable(messages, func(a, b int) bool { return messages[a].ID > messages[b].ID })

	s.mu.Lock()
	c.loaded = true
	hasNewer := len(messages) > 0 && messages[0].ID > c.newest
	if hasNewer {
		c.newest = messages[0].ID
	}
	s.mu.Unlock()

	s.reply(id, map[string]any{"type": "messages", "messages": messages})

	// Must follow the reply, or the app marks a message as the chat's last one
	// before it's in the list and stops showing new messages.
	if hasNewer {
		s.emitLastMessage(messages[0])
	}
}

// loadPreviews fetches each chat's last message, as Slack's chat list doesn't
// include it. It starts at the top of the list, which is what's on screen.
func (s *Session) loadPreviews() {
	s.mu.Lock()
	chats := make([]*chat, 0, len(s.chats))
	for _, c := range s.chats {
		chats = append(chats, c)
	}
	sort.Slice(chats, func(a, b int) bool { return chats[a].order > chats[b].order })
	s.mu.Unlock()

	for _, c := range chats {
		select {
		case <-time.After(s.previewInterval):
		case <-s.ctx.Done():
			return
		}
		s.loadPreview(c)
	}
}

func (s *Session) loadPreview(c *chat) {
	conversation, err := s.api.ConversationInfo(s.ctx, c.slackID)
	if err != nil && !errors.Is(err, context.Canceled) {
		s.log("loading read state failed: %v", err)
	}

	page, err := s.api.History(s.ctx, c.slackID, api.HistoryParams{Limit: previewLimit})
	if err != nil {
		if !errors.Is(err, context.Canceled) {
			s.log("loading preview failed: %v", err)
		}
		return
	}
	if len(page.Messages) == 0 {
		return
	}

	// A send in progress has a newer message than this one
	c.sending.RLock()
	defer c.sending.RUnlock()

	item, err := s.toMessage(c, page.Messages[0])
	if err != nil {
		s.log("skipping preview: %v", err)
		return
	}

	s.mu.Lock()
	isNewest := item.ID > c.newest
	if isNewest {
		c.newest = item.ID
	}
	s.mu.Unlock()

	if isNewest {
		s.emitLastMessage(item)
	}
	s.applyHistoryReads(c, conversation.LastRead, page.Messages)
}

func (s *Session) send(id uint64, chatID int64, content string) {
	if s.refuseWrite(id) {
		return
	}

	c := s.findChat(chatID)
	if c == nil {
		s.replyError(id, codeBadRequest, "unknown chat")
		return
	}

	// TDLib does this for Telegram, the app relies on it
	content, markup := s.withMentions(strings.TrimSpace(content))
	if content == "" {
		s.replyError(id, codeBadRequest, "message is empty")
		return
	}

	c.sending.Lock()
	defer c.sending.Unlock()

	now := time.Now()

	s.mu.Lock()
	me := s.me
	if me == nil {
		s.mu.Unlock()
		s.replyError(id, codeUnauthorized, "not logged in")
		return
	}

	tempID := now.UnixMicro()
	if tempID <= c.newest {
		tempID = c.newest + 1
	}
	if tempID <= s.tempID {
		tempID = s.tempID + 1
	}
	s.tempID = tempID
	c.inflight++
	c.sendGen++
	s.mu.Unlock()

	temp := message{ID: tempID, Chat: c.id, Sender: me.ID, Outgoing: true, Date: now.Unix(), Text: content}
	s.emit(map[string]any{"event": "new_message", "message": temp})
	s.emitLastMessage(temp)

	posted, err := s.api.PostMessage(s.ctx, c.slackID, markup)

	var sent message
	if err == nil {
		sent, err = s.sentMessage(c, posted, me)
	}

	s.mu.Lock()
	c.inflight--
	c.sendGen++
	if err == nil {
		s.own[sent.ID] = true
		if sent.ID > c.newest {
			c.newest = sent.ID
		}
	}
	s.mu.Unlock()

	if err != nil {
		s.emit(map[string]any{"event": "delete_messages", "chat": c.id, "ids": []int64{tempID}})
		s.replyError(id, codeFailed, err.Error())
		return
	}

	s.emit(map[string]any{"event": "send_succeeded", "old_id": tempID, "message": sent})
	if sent.Text != content || len(sent.Entities) > 0 || sent.Media != nil || sent.Preview != nil {
		s.emit(contentEvent(sent))
	}
	s.emitLastMessage(sent)
	s.readUpTo(c, sent.ID, false)
	s.replyOK(id)
}

// properties answers with an error for messages that aren't ours, which leaves
// them without the edit and delete actions.
func (s *Session) properties(id uint64, messageID int64) {
	s.mu.Lock()
	own := s.own[messageID] && !s.readOnly
	s.mu.Unlock()

	if !own {
		s.replyError(id, codeBadRequest, "not your message")
		return
	}
	s.reply(id, map[string]any{"type": "properties"})
}

func (s *Session) edit(id uint64, chatID, messageID int64, content string) {
	if s.refuseWrite(id) {
		return
	}

	c := s.findChat(chatID)
	if c == nil {
		s.replyError(id, codeBadRequest, "unknown chat")
		return
	}

	content, markup := s.withMentions(strings.TrimSpace(content))
	if content == "" {
		s.replyError(id, codeBadRequest, "message is empty")
		return
	}

	ts := ids.FormatTS(messageID)
	posted, err := s.api.UpdateMessage(s.ctx, c.slackID, ts, markup)
	if err != nil {
		s.replyError(id, codeFailed, err.Error())
		return
	}

	s.mu.Lock()
	posted.User = s.me.SlackID
	s.mu.Unlock()

	posted.TS = ts
	if posted.Edited == nil {
		posted.Edited = &api.Edited{TS: ids.FormatTS(time.Now().UnixMicro())}
	}

	s.changeMessage(c, posted)
	s.replyOK(id)
}

func (s *Session) deleteMessages(id uint64, chatID int64, messageIDs []int64) {
	if s.refuseWrite(id) {
		return
	}

	c := s.findChat(chatID)
	if c == nil {
		s.replyError(id, codeBadRequest, "unknown chat")
		return
	}

	for _, messageID := range messageIDs {
		ts := ids.FormatTS(messageID)
		if err := s.api.DeleteMessage(s.ctx, c.slackID, ts); err != nil {
			s.replyError(id, codeFailed, err.Error())
			return
		}
		s.deleteMessage(c, ts)
	}
	s.replyOK(id)
}

func (s *Session) sentMessage(c *chat, posted api.Message, me *user) (message, error) {
	id, err := ids.ParseTS(posted.TS)
	if err != nil {
		return message{}, err
	}
	content := s.render(posted)
	return message{
		ID:       id,
		Chat:     c.id,
		Sender:   me.ID,
		Outgoing: true,
		Date:     id / 1000000,
		Text:     content.Text,
		Entities: content.Entities,
		Media:    content.Media,
		Preview:  content.Preview,
	}, nil
}

func (s *Session) open(chatID int64) {
	c := s.findChat(chatID)
	if c == nil {
		return
	}

	s.mu.Lock()
	if c.stopPoll != nil {
		s.mu.Unlock()
		return
	}
	ctx, cancel := context.WithCancel(s.ctx)
	c.stopPoll = cancel
	s.mu.Unlock()

	s.spawn(func() { s.poll(ctx, c) })
	s.spawn(func() { s.fetchMemberCount(ctx, c) })
}

func (s *Session) fetchMemberCount(ctx context.Context, c *chat) {
	if c.supergroupID == 0 {
		return
	}

	info, err := s.api.ConversationInfo(ctx, c.slackID)
	if err != nil {
		if !errors.Is(err, context.Canceled) {
			s.log("member count failed: %v", err)
		}
		return
	}

	var description []string
	for _, value := range []string{info.Topic.Value, info.Purpose.Value} {
		if value != "" {
			description = append(description, text.FromMrkdwn(value, s.userName))
		}
	}

	s.emit(map[string]any{
		"event": "member_count", "supergroup_id": c.supergroupID, "count": info.NumMembers,
		"description": strings.Join(description, "\n\n"),
	})
}

func (s *Session) isOffline() bool {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.offline
}

func (s *Session) closeChat(chatID int64) {
	c := s.findChat(chatID)
	if c == nil {
		return
	}

	s.mu.Lock()
	defer s.mu.Unlock()
	if c.stopPoll != nil {
		c.stopPoll()
		c.stopPoll = nil
	}
}

func (s *Session) poll(ctx context.Context, c *chat) {
	ticker := time.NewTicker(s.pollInterval)
	defer ticker.Stop()

	var lastPoll time.Time

	for {
		select {
		case <-ticker.C:
			// With realtime up this is only a safety net. The first one still
			// runs, to catch up on what arrived while the chat was closed.
			if s.isOffline() || s.isRealtimeUp() && time.Since(lastPoll) < safetyPollInterval {
				continue
			}
			lastPoll = time.Now()

			if err := s.pollOnce(c); err != nil && !errors.Is(err, context.Canceled) {
				s.log("poll failed: %v", err)
			}
		case <-ctx.Done():
			return
		}
	}
}

func (s *Session) pollOnce(c *chat) error {
	s.mu.Lock()
	skip := !c.loaded || c.inflight > 0
	generation := c.sendGen
	newest := c.newest
	s.mu.Unlock()

	if skip {
		return nil
	}

	var fetched []api.Message
	var err error

	if newest == 0 {
		var page api.HistoryPage
		page, err = s.api.History(s.ctx, c.slackID, api.HistoryParams{Limit: pollFirstLimit})
		fetched = page.Messages
	} else {
		fetched, err = s.fetchNewer(c, newest, maxHistoryLimit)
	}
	if err != nil {
		return err
	}

	messages := s.toMessages(c, fetched)
	sort.SliceStable(messages, func(a, b int) bool { return messages[a].ID < messages[b].ID })

	// A send that started or finished during the fetch may be in these results
	// under its real id while the app still has it under a temporary one.
	// Dropping the batch is safe, the next poll fetches it again.
	s.mu.Lock()
	stale := c.sendGen != generation
	s.mu.Unlock()
	if stale {
		return nil
	}

	for _, m := range messages {
		s.mu.Lock()
		isNew := m.ID > c.newest
		if isNew {
			c.newest = m.ID
		}
		s.mu.Unlock()

		if isNew {
			s.emit(map[string]any{"event": "new_message", "message": m})
			s.emitLastMessage(m)
			s.noteIncoming(c, m)
		}
	}
	return nil
}
