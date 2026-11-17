package session

import (
	"sort"
	"strings"
	"unicode/utf8"

	"yottaslack/internal/api"
	"yottaslack/internal/ids"
)

const (
	folderThreads = 4

	threadTitleLength = 40
)

// Threads are chats of their own, in a folder of their own: the app has no
// view for a message's replies, and a chat already does everything a thread
// needs. Their messages are the parent and its replies.

func threadKey(channel, ts string) string {
	return channel + "/" + ts
}

// historyPage reads a chat's messages newest first, from the channel or from
// the thread.
func (s *Session) historyPage(c *chat, p api.HistoryParams) (api.HistoryPage, error) {
	if c.threadTS == "" {
		return s.api.History(s.ctx, c.slackID, p)
	}

	page, err := s.api.Replies(s.ctx, c.slackID, c.threadTS, p)
	sort.SliceStable(page.Messages, func(a, b int) bool { return tsLess(page.Messages[b].TS, page.Messages[a].TS) })
	return page, err
}

func (s *Session) findThread(channel *chat, ts string) *chat {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.threads[threadKey(channel.slackID, ts)]
}

// threadParent fetches the message a thread hangs off.
func (s *Session) threadParent(channel *chat, ts string) (api.Message, bool) {
	page, err := s.api.Replies(s.ctx, channel.slackID, ts, api.HistoryParams{Limit: 1, Latest: ts, Inclusive: true})
	if err != nil {
		s.log("loading thread failed: %v", err)
		return api.Message{}, false
	}

	for _, m := range page.Messages {
		if m.TS == ts {
			return m, true
		}
	}
	return api.Message{}, false
}

// threadChat returns the chat for a thread, adding it if it's new.
func (s *Session) threadChat(channel *chat, parent api.Message) *chat {
	if known := s.findThread(channel, parent.TS); known != nil {
		return known
	}

	parentID, err := ids.ParseTS(parent.TS)
	if err != nil {
		s.log("skipping thread: %v", err)
		return nil
	}

	order := parentID
	if latest, err := ids.ParseTS(parent.LatestReply); err == nil && latest > order {
		order = latest
	}

	key := threadKey(channel.slackID, parent.TS)
	entry := wireChat{
		ID:           s.ids.Chat(key),
		Type:         "group",
		Title:        threadTitle(channel.title, s.render(parent).Text),
		SupergroupID: s.ids.Supergroup(key),
		Order:        order,
		Folder:       folderThreads,
		Thread:       true,
	}

	s.mu.Lock()
	if known := s.threads[key]; known != nil {
		s.mu.Unlock()
		return known
	}
	c := &chat{
		id:           entry.ID,
		slackID:      channel.slackID,
		supergroupID: entry.SupergroupID,
		threadTS:     parent.TS,
		order:        order,
		title:        entry.Title,
	}
	s.chats[c.id] = c
	s.threads[key] = c
	s.mu.Unlock()

	s.emit(map[string]any{"event": "chats", "chats": []wireChat{entry}})
	if err := s.ids.Save(); err != nil {
		s.log("saving ids failed: %v", err)
	}

	// Gives it a last message, which the app needs to show anything new in it
	if item, err := s.toMessage(c, parent); err == nil {
		s.mu.Lock()
		c.newest = item.ID
		s.mu.Unlock()
		s.emitLastMessage(item)
	}
	return c
}

func threadTitle(channel, text string) string {
	line, _, _ := strings.Cut(strings.TrimSpace(text), "\n")
	if line == "" {
		line = "Thread"
	}
	if utf8.RuneCountInString(line) > threadTitleLength {
		line = string([]rune(line)[:threadTitleLength]) + "…"
	}
	return channel + ": " + line
}

func (s *Session) isMine(slackID string) bool {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.me != nil && slackID == s.me.SlackID
}

// followsThread is Slack's rule for the Threads tab: threads you started,
// replied in, or got mentioned in.
func (s *Session) followsThread(m api.Message) bool {
	if s.isMine(m.User) || s.isMine(m.ParentUserID) {
		return true
	}
	for _, user := range m.ReplyUsers {
		if s.isMine(user) {
			return true
		}
	}

	s.mu.Lock()
	mention := s.me != nil && strings.Contains(m.Text, "<@"+s.me.SlackID)
	s.mu.Unlock()
	return mention
}

// discoverThreads adds the followed threads among a page of channel history.
func (s *Session) discoverThreads(channel *chat, messages []api.Message) {
	for _, m := range messages {
		if m.ReplyCount > 0 && (m.ThreadTS == "" || m.ThreadTS == m.TS) && s.followsThread(m) {
			s.threadChat(channel, m)
		}
	}
}

// receiveReply puts a thread reply in its thread's chat.
func (s *Session) receiveReply(channel *chat, m api.Message) {
	thread := s.findThread(channel, m.ThreadTS)
	if thread == nil {
		if !s.followsThread(m) {
			return
		}

		parent, ok := s.threadParent(channel, m.ThreadTS)
		if !ok {
			return
		}
		if thread = s.threadChat(channel, parent); thread == nil {
			return
		}
	}

	thread.sending.RLock()
	defer thread.sending.RUnlock()

	if id, err := ids.ParseTS(m.TS); err == nil {
		s.mu.Lock()
		if id > thread.order {
			thread.order = id
		}
		s.mu.Unlock()
	}
	s.receiveMessage(thread, m)
}

// replyTarget is the thread chat a reply to messageID goes to, creating it
// the first time.
func (s *Session) replyTarget(channel *chat, messageID int64) *chat {
	ts := ids.FormatTS(messageID)
	if thread := s.findThread(channel, ts); thread != nil {
		return thread
	}

	parent, ok := s.threadParent(channel, ts)
	if !ok {
		return nil
	}
	return s.threadChat(channel, parent)
}

// chatOfMessage is where a message's updates go: its thread for a reply.
func (s *Session) chatOfMessage(channel *chat, messageID int64) *chat {
	s.mu.Lock()
	defer s.mu.Unlock()
	if thread := s.threadOf[messageID]; thread != nil {
		return thread
	}
	return channel
}
