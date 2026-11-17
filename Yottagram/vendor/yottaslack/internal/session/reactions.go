package session

import (
	"regexp"
	"sort"
	"strconv"
	"strings"

	"yottaslack/internal/api"
	"yottaslack/internal/ids"
	"yottaslack/internal/text"
)

const (
	// Stands in for emoji with no Unicode form or picture
	unknownReaction = "❔"

	maxEmojiAliases = 5

	memberSearchLimit = 20
)

// The app's input marks mentions with these, around a @username or #user id
var mentionMarker = regexp.MustCompile("([^]*)")

type wireReaction struct {
	Emoji  string `json:"emoji"`
	Custom int64  `json:"custom,omitempty"`
	Count  int64  `json:"count"`
	Chosen bool   `json:"chosen"`
}

type customEmoji struct {
	id    int64
	url   string
	alias bool
}

// loadCustomEmoji is best effort, without it they stay as :names:.
func (s *Session) loadCustomEmoji() {
	list, err := s.api.CustomEmoji(s.ctx)
	if err != nil {
		s.log("emoji.list failed: %v", err)
		return
	}

	names := make([]string, 0, len(list))
	for name := range list {
		names = append(names, name)
	}
	sort.Strings(names)

	s.mu.Lock()
	defer s.mu.Unlock()

	for i, name := range names {
		address := list[name]
		alias := strings.HasPrefix(address, "alias:")
		for hops := 0; hops < maxEmojiAliases && strings.HasPrefix(address, "alias:"); hops++ {
			address = list[strings.TrimPrefix(address, "alias:")]
		}
		if address == "" || strings.HasPrefix(address, "alias:") {
			continue
		}

		id := int64(i + 1)
		s.customEmoji[name] = customEmoji{id: id, url: address, alias: alias}
		s.customNames[id] = name
	}
}

// logEmojiObjects writes every custom emoji to the log as Slack describes it,
// one line each, to find what tells the workspace's own from the packs Slack
// adds. Nothing uses them otherwise.
func (s *Session) logEmojiObjects() {
	emojis, err := s.api.AdminEmoji(s.ctx)
	if err != nil {
		s.log("emoji.adminList failed: %v", err)
		return
	}
	if len(emojis) == 0 {
		return
	}

	s.log("emoji.adminList: %d emoji", len(emojis))
	for _, emoji := range emojis {
		s.log("emoji: %s", emoji)
	}
}

// emitEmojiSets tells the app which custom emoji sets there are. The app asks
// at login, before the emoji are loaded, so this follows the load.
func (s *Session) emitEmojiSets() {
	s.emit(map[string]any{"event": "custom_emoji_sets", "sets": s.emojiSets(s.pickerEmojis())})
}

func (s *Session) customEmojiID(name string) int64 {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.customEmoji[name].id
}

// customEmojis answers the app's request for their pictures.
func (s *Session) customEmojis(id uint64, emojiIDs []int64) {
	emojis := make([]map[string]any, 0, len(emojiIDs))

	for _, emojiID := range emojiIDs {
		s.mu.Lock()
		name := s.customNames[emojiID]
		address := s.customEmoji[name].url
		s.mu.Unlock()

		if address != "" {
			emojis = append(emojis, map[string]any{"id": emojiID, "name": ":" + name + ":", "file": s.imageFile(address)})
		}
	}
	s.reply(id, map[string]any{"type": "custom_emojis", "emojis": emojis})
}

func (s *Session) wireReactions(reactions []api.Reaction) []wireReaction {
	s.mu.Lock()
	me := s.me
	s.mu.Unlock()

	wire := make([]wireReaction, 0, len(reactions))
	for _, reaction := range reactions {
		emoji, ok := text.Emoji(reaction.Name)
		custom := int64(0)
		if !ok {
			emoji = unknownReaction
			custom = s.customEmojiID(reaction.Name)
		}

		chosen := false
		for _, user := range reaction.Users {
			chosen = chosen || me != nil && user == me.SlackID
		}
		wire = append(wire, wireReaction{Emoji: emoji, Custom: custom, Count: int64(reaction.Count), Chosen: chosen})
	}
	return wire
}

// changeReaction applies one reaction, once per user: the app's own change is
// followed by the same change from the socket.
func (s *Session) changeReaction(c *chat, messageID int64, name, userID string, add bool) {
	s.mu.Lock()
	var changed []api.Reaction
	found := false

	for _, reaction := range s.reactions[messageID] {
		if reaction.Name != name {
			changed = append(changed, reaction)
			continue
		}
		found = true

		present := false
		var users []string
		for _, user := range reaction.Users {
			if user == userID {
				present = true
			} else {
				users = append(users, user)
			}
		}

		switch {
		case add && !present:
			reaction.Count++
			users = append(users, userID)
		case add:
			users = append(users, userID)
		case present:
			reaction.Count--
		}
		reaction.Users = users

		if reaction.Count > 0 {
			changed = append(changed, reaction)
		}
	}

	if !found && add {
		changed = append(changed, api.Reaction{Name: name, Count: 1, Users: []string{userID}})
	}
	s.reactions[messageID] = changed
	s.mu.Unlock()

	s.emit(map[string]any{"event": "reactions", "chat": c.id, "message_id": messageID, "reactions": s.wireReactions(changed)})
}

// reactionName finds what Slack calls a reaction from the app. What's already on
// the message comes first, as it may have a skin tone or be a custom emoji.
func (s *Session) reactionName(messageID int64, emoji string) string {
	emoji = text.NormalizeEmoji(emoji)

	s.mu.Lock()
	defer s.mu.Unlock()

	for _, reaction := range s.reactions[messageID] {
		char, ok := text.Emoji(reaction.Name)
		if ok && text.NormalizeEmoji(char) == emoji || !ok && emoji == unknownReaction {
			return reaction.Name
		}
	}
	return text.EmojiName(emoji)
}

func (s *Session) react(id uint64, chatID, messageID int64, emoji string, custom int64, add bool) {
	c := s.findChat(chatID)
	if c == nil {
		s.replyError(id, codeBadRequest, "unknown chat")
		return
	}

	var name string
	if custom != 0 {
		s.mu.Lock()
		name = s.customNames[custom]
		s.mu.Unlock()
	} else {
		name = s.reactionName(messageID, emoji)
	}
	if name == "" {
		s.replyError(id, codeBadRequest, "unknown emoji")
		return
	}

	ts := ids.FormatTS(messageID)
	var err error
	if add {
		err = s.api.AddReaction(s.ctx, c.slackID, ts, name)
	} else {
		err = s.api.RemoveReaction(s.ctx, c.slackID, ts, name)
	}
	if err != nil && !api.IsCode(err, "already_reacted") && !api.IsCode(err, "no_reaction") {
		s.log("reacting failed: %v", err)
		s.replyError(id, codeFailed, err.Error())
		return
	}

	s.mu.Lock()
	me := s.me.SlackID
	s.mu.Unlock()

	s.changeReaction(c, messageID, name, me, add)
	s.replyOK(id)
}

// availableReactions lists the standard emoji, then the workspace's own by
// :name:, which is how the picker shows them.
func (s *Session) availableReactions(id uint64) {
	emojis := text.Emojis()
	for _, emoji := range s.pickerEmojis() {
		emojis = append(emojis, ":"+emoji.name+":")
	}
	s.reply(id, map[string]any{"type": "available_reactions", "emojis": emojis})
}

type pickerEmoji struct {
	id   int64
	name string
	url  string
}

// pickerEmojis are the workspace's custom emoji by name, without aliases,
// which would show the same picture twice.
func (s *Session) pickerEmojis() []pickerEmoji {
	s.mu.Lock()
	emojis := make([]pickerEmoji, 0, len(s.customEmoji))
	for name, emoji := range s.customEmoji {
		if !emoji.alias {
			emojis = append(emojis, pickerEmoji{id: emoji.id, name: name, url: emoji.url})
		}
	}
	s.mu.Unlock()

	sort.Slice(emojis, func(a, b int) bool { return emojis[a].name < emojis[b].name })
	return emojis
}

const workspaceEmojiSet = 1

type emojiSet struct {
	ID    int64  `json:"id"`
	Title string `json:"title"`
}

// emojiSets are the picker's custom emoji sets: one with all of the
// workspace's, once there are any.
func (s *Session) emojiSets(emojis []pickerEmoji) []emojiSet {
	if len(emojis) == 0 {
		return []emojiSet{}
	}
	return []emojiSet{{ID: workspaceEmojiSet, Title: "Workspace"}}
}

// customEmojiSets answers which custom emoji sets the picker has.
func (s *Session) customEmojiSets(id uint64) {
	s.reply(id, map[string]any{"type": "custom_emoji_sets", "sets": s.emojiSets(s.pickerEmojis())})
}

// customEmojiSet answers the app's request for one of those sets, which the
// app treats as a custom emoji sticker set.
func (s *Session) customEmojiSet(id uint64, setID int64) {
	if setID != workspaceEmojiSet {
		s.replyError(id, codeBadRequest, "unknown emoji set")
		return
	}

	picker := s.pickerEmojis()
	emojis := make([]map[string]any, 0, len(picker))
	for _, emoji := range picker {
		emojis = append(emojis, map[string]any{"id": emoji.id, "name": ":" + emoji.name + ":", "file": s.imageFile(emoji.url)})
	}

	reply := map[string]any{"type": "custom_emoji_set", "id": setID, "title": "Workspace", "emojis": emojis}
	if icon := s.slackbotIcon(); icon != "" {
		reply["icon"] = s.imageFile(icon)
	}
	s.reply(id, reply)
}

// slackbotIcon is Slackbot's picture, which the workspace's emoji set shows as
// its icon. users.list includes Slackbot, so there's no need to fetch it.
func (s *Session) slackbotIcon() string {
	s.mu.Lock()
	defer s.mu.Unlock()

	bot := s.users[slackbotID]
	if bot == nil {
		return ""
	}
	if bot.AvatarBig != "" {
		return bot.AvatarBig
	}
	return bot.Avatar
}

// searchMembers looks through the workspace, not only the chat. Asking Slack
// for a channel's members would cost a call per keystroke.
func (s *Session) searchMembers(id uint64, query string, limit int) {
	query = strings.ToLower(query)
	if limit <= 0 || limit > memberSearchLimit {
		limit = memberSearchLimit
	}

	s.mu.Lock()
	var found []*user
	for _, u := range s.users {
		if strings.Contains(strings.ToLower(u.Username), query) || strings.Contains(strings.ToLower(u.FirstName), query) {
			found = append(found, u)
		}
	}
	s.mu.Unlock()

	sort.Slice(found, func(a, b int) bool { return found[a].Username < found[b].Username })
	if len(found) > limit {
		found = found[:limit]
	}

	users := make([]int64, 0, len(found))
	for _, u := range found {
		users = append(users, u.ID)
	}
	s.reply(id, map[string]any{"type": "members", "users": users})
}

// findMention resolves what's between the mention markers.
func (s *Session) findMention(mention string) *user {
	s.mu.Lock()
	defer s.mu.Unlock()

	if number, ok := strings.CutPrefix(mention, "#"); ok {
		id, err := strconv.ParseInt(number, 10, 64)
		if err != nil {
			return nil
		}
		for _, u := range s.users {
			if u.ID == id {
				return u
			}
		}
		return nil
	}

	name := strings.TrimPrefix(mention, "@")
	for _, u := range s.users {
		if u.Username == name {
			return u
		}
	}
	return nil
}

// withMentions returns a message as shown in the app, and as sent to Slack.
func (s *Session) withMentions(content string) (string, string) {
	display := mentionMarker.ReplaceAllStringFunc(content, func(match string) string {
		mention := mentionMarker.FindStringSubmatch(match)[1]
		if u := s.findMention(mention); u != nil {
			return "@" + u.Username
		}
		return mention
	})

	markup := mentionMarker.ReplaceAllStringFunc(text.ToMrkdwn(content), func(match string) string {
		mention := mentionMarker.FindStringSubmatch(match)[1]
		if u := s.findMention(mention); u != nil {
			return "<@" + u.SlackID + ">"
		}
		return mention
	})
	return display, markup
}

type upload struct {
	Path    string `json:"path"`
	Caption string `json:"caption"`
}

// sendFiles uploads to Slack, which posts the messages itself. They arrive like
// anyone else's.
func (s *Session) sendFiles(id uint64, chatID int64, files []upload) {
	c := s.findChat(chatID)
	if c == nil {
		s.replyError(id, codeBadRequest, "unknown chat")
		return
	}

	for _, file := range files {
		_, caption := s.withMentions(strings.TrimSpace(file.Caption))
		if err := s.api.Upload(s.ctx, c.slackID, file.Path, caption, c.threadTS); err != nil {
			s.log("upload failed: %v", err)
			s.replyError(id, codeFailed, err.Error())
			return
		}
	}
	s.replyOK(id)
}
