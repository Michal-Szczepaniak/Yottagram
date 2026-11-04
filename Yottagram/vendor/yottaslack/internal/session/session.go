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
}

type wireChat struct {
	ID           int64  `json:"id"`
	Type         string `json:"type"`
	Title        string `json:"title"`
	UserID       int64  `json:"user_id,omitempty"`
	SupergroupID int64  `json:"supergroup_id,omitempty"`
	Order        int64  `json:"order"`
	dm           bool
}

type message struct {
	ID       int64  `json:"id"`
	Chat     int64  `json:"chat"`
	Sender   int64  `json:"sender"`
	Outgoing bool   `json:"outgoing"`
	Date     int64  `json:"date"`
	Text     string `json:"text"`
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

	// Held for writing during a send and for reading during a history request.
	// A history reply during a send can contain the message under its real id
	// while the app still has it under a temporary one, and the swap then
	// leaves it in the list twice.
	sending sync.RWMutex
}

// Session is one logged-in Slack account.
type Session struct {
	api          *api.Client
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
}

// New creates a session storing its state in dataDir.
func New(dataDir string) (*Session, error) {
	if err := os.MkdirAll(dataDir, 0o700); err != nil {
		return nil, err
	}

	store, err := ids.Open(filepath.Join(dataDir, "ids.json"))
	if err != nil {
		return nil, err
	}

	ctx, cancel := context.WithCancel(context.Background())
	return &Session{
		api:          api.New(),
		ids:          store,
		usersPath:    filepath.Join(dataDir, "users.json"),
		pollInterval: pollInterval,
		ctx:          ctx,
		cancel:       cancel,
		events:       make(chan string, eventQueueSize),
		users:        map[string]*user{},
		chats:        map[int64]*chat{},
	}, nil
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
			Token string `json:"token"`
			Retry bool   `json:"retry"`
		}
		if err = json.Unmarshal(r.Params, &p); err == nil {
			s.login(r.ID, p.Token, p.Retry)
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

func (s *Session) login(id uint64, token string, retry bool) {
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

	s.replyOK(id)
	s.emit(map[string]any{"event": "auth_ready", "my_id": me.ID})
	s.emit(map[string]any{"event": "users", "users": []*user{me}})

	s.sync()
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
	cached := s.loadUserCache()

	if len(cached) > 0 {
		s.emitUsers(cached)
	} else if err := s.refreshUsers(); err != nil {
		s.log("users.list failed: %v", err)
	}

	conversations, err := s.api.Conversations(s.ctx)
	if err != nil {
		s.log("users.conversations failed: %v", err)
	}

	s.emitChats(conversations)
	s.emit(map[string]any{"event": "ready"})

	if err := s.ids.Save(); err != nil {
		s.log("saving ids failed: %v", err)
	}

	if len(cached) > 0 {
		if err := s.refreshUsers(); err != nil {
			s.log("users.list failed: %v", err)
		}
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
		if known, ok := s.users[u.SlackID]; !ok || *known != *u {
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
		id := s.ids.Chat(conversation.ID)
		entry := wireChat{ID: id, Title: s.conversationTitle(conversation), dm: conversation.IsIM || conversation.IsMpim}

		if conversation.IsIM {
			entry.Type = "private"
			entry.UserID = s.ensureUser(conversation.User).ID
		} else {
			entry.Type = "group"
			entry.SupergroupID = s.ids.Supergroup(conversation.ID)
		}

		chats = append(chats, entry)

		s.mu.Lock()
		if _, known := s.chats[id]; !known {
			s.chats[id] = &chat{id: id, slackID: conversation.ID, supergroupID: entry.SupergroupID}
		}
		s.mu.Unlock()
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
	s.mu.Unlock()

	for start := 0; start < len(chats); start += batchSize {
		end := start + batchSize
		if end > len(chats) {
			end = len(chats)
		}
		s.emit(map[string]any{"event": "chats", "chats": chats[start:end]})
	}
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

func (s *Session) messageText(m api.Message) string {
	var parts []string

	if m.Text != "" {
		parts = append(parts, text.FromMrkdwn(m.Text, s.userName))
	}
	for _, file := range m.Files {
		name := file.Title
		if name == "" {
			name = file.Name
		}
		parts = append(parts, "[file: "+name+"]")
	}
	if len(parts) == 0 {
		for _, attachment := range m.Attachments {
			if attachment.Text != "" {
				parts = append(parts, text.FromMrkdwn(attachment.Text, s.userName))
			} else if attachment.Fallback != "" {
				parts = append(parts, attachment.Fallback)
			}
		}
	}
	if len(parts) == 0 {
		return "[unsupported message]"
	}
	return strings.TrimSpace(strings.Join(parts, "\n"))
}

func (s *Session) toMessage(c *chat, m api.Message) (message, error) {
	id, err := ids.ParseTS(m.TS)
	if err != nil {
		return message{}, err
	}

	sender := s.sender(m)
	s.mu.Lock()
	outgoing := s.me != nil && sender.ID == s.me.ID
	s.mu.Unlock()

	return message{
		ID:       id,
		Chat:     c.id,
		Sender:   sender.ID,
		Outgoing: outgoing,
		Date:     id / 1000000,
		Text:     s.messageText(m),
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

func (s *Session) send(id uint64, chatID int64, content string) {
	c := s.findChat(chatID)
	if c == nil {
		s.replyError(id, codeBadRequest, "unknown chat")
		return
	}

	// TDLib does this for Telegram, the app relies on it
	content = strings.TrimSpace(content)
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

	posted, err := s.api.PostMessage(s.ctx, c.slackID, text.ToMrkdwn(content))

	var sent message
	if err == nil {
		sent, err = s.sentMessage(c, posted, me)
	}

	s.mu.Lock()
	c.inflight--
	c.sendGen++
	if err == nil && sent.ID > c.newest {
		c.newest = sent.ID
	}
	s.mu.Unlock()

	if err != nil {
		s.emit(map[string]any{"event": "delete_messages", "chat": c.id, "ids": []int64{tempID}})
		s.replyError(id, codeFailed, err.Error())
		return
	}

	s.emit(map[string]any{"event": "send_succeeded", "old_id": tempID, "message": sent})
	if sent.Text != content {
		s.emit(map[string]any{"event": "message_content", "chat": c.id, "message_id": sent.ID, "text": sent.Text})
	}
	s.emitLastMessage(sent)
	s.replyOK(id)
}

func (s *Session) sentMessage(c *chat, posted api.Message, me *user) (message, error) {
	id, err := ids.ParseTS(posted.TS)
	if err != nil {
		return message{}, err
	}
	return message{
		ID:       id,
		Chat:     c.id,
		Sender:   me.ID,
		Outgoing: true,
		Date:     id / 1000000,
		Text:     s.messageText(posted),
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
	s.emit(map[string]any{"event": "member_count", "supergroup_id": c.supergroupID, "count": info.NumMembers})
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

	for {
		select {
		case <-ticker.C:
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
		}
	}
	return nil
}
