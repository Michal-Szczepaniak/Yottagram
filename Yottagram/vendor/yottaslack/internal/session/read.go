package session

import (
	"strings"
	"time"

	"yottaslack/internal/api"
	"yottaslack/internal/ids"
)

// noteIncoming counts a message as unread, or as read up to it if it's ours,
// since writing in a chat means having read it.
func (s *Session) noteIncoming(c *chat, item message) {
	if item.Outgoing {
		s.readUpTo(c, item.ID, false)
		return
	}

	s.mu.Lock()
	known := item.ID <= c.lastRead
	for _, id := range c.unread {
		known = known || id == item.ID
	}
	if !known {
		c.unread = append(c.unread, item.ID)
	}
	s.mu.Unlock()

	if !known {
		s.emitRead(c)
	}
}

// readUpTo marks messages as read. mark also tells Slack, which only happens
// for what the user actually looked at here.
func (s *Session) readUpTo(c *chat, id int64, mark bool) {
	s.mu.Lock()
	if id <= c.lastRead {
		s.mu.Unlock()
		return
	}

	c.lastRead = id
	remaining := c.unread[:0]
	for _, unread := range c.unread {
		if unread > id {
			remaining = append(remaining, unread)
		}
	}
	c.unread = remaining

	notified := c.notified
	c.notified = nil

	startMark := mark && !c.marking && !s.readOnly
	if startMark {
		c.marking = true
	}
	s.mu.Unlock()

	s.emitRead(c)
	if len(notified) > 0 {
		s.emit(map[string]any{"event": "notify_clear", "chat": c.id, "group": c.order, "ids": notified})
	}
	if startMark {
		s.spawn(func() { s.markLater(c) })
	}
}

// markLater sends one conversations.mark for a burst of scrolling.
func (s *Session) markLater(c *chat) {
	select {
	case <-time.After(s.markDelay):
	case <-s.ctx.Done():
		return
	}

	s.mu.Lock()
	id := c.lastRead
	c.marking = false
	s.mu.Unlock()

	if err := s.api.Mark(s.ctx, c.slackID, ids.FormatTS(id)); err != nil {
		s.log("marking as read failed: %v", err)
	}
}

func (s *Session) view(id uint64, chatID, messageID int64) {
	c := s.findChat(chatID)
	if c == nil {
		s.replyError(id, codeBadRequest, "unknown chat")
		return
	}

	// A message still being sent has a temporary id ahead of everything else
	s.mu.Lock()
	if messageID > c.newest {
		messageID = c.newest
	}
	s.mu.Unlock()

	s.readUpTo(c, messageID, true)
	s.replyOK(id)
}

func (s *Session) emitRead(c *chat) {
	s.mu.Lock()
	lastRead := c.lastRead
	unread := len(c.unread)

	messages, chats := 0, 0
	for _, other := range s.chats {
		messages += len(other.unread)
		if len(other.unread) > 0 {
			chats++
		}
	}
	total := len(s.chats)
	s.mu.Unlock()

	s.emit(map[string]any{
		"event": "read_inbox", "chat": c.id, "last_read": lastRead, "unread": unread,
		"unread_messages": messages, "unread_chats": chats, "total_chats": total,
	})
}

// applyHistoryReads takes what Slack says was read, and counts the messages
// after it. Slack only says that for one chat at a time, so counts stop at the
// size of a preview page.
func (s *Session) applyHistoryReads(c *chat, lastRead string, messages []api.Message) {
	read, err := ids.ParseTS(lastRead)
	if err != nil {
		return
	}

	s.mu.Lock()
	if read > c.lastRead {
		c.lastRead = read
	}

	for _, m := range messages {
		id, err := ids.ParseTS(m.TS)
		if err != nil || id <= c.lastRead || s.me != nil && m.User == s.me.SlackID {
			continue
		}

		known := false
		for _, unread := range c.unread {
			known = known || unread == id
		}
		if !known {
			c.unread = append(c.unread, id)
		}
	}
	s.mu.Unlock()

	s.emitRead(c)
}

// notify follows Slack's defaults: direct messages and mentions.
func (s *Session) notify(c *chat, item message, m api.Message) {
	s.mu.Lock()
	mine := "<@" + s.me.SlackID
	s.mu.Unlock()

	mentioned := strings.Contains(m.Text, mine) || strings.Contains(m.Text, "<!here") || strings.Contains(m.Text, "<!channel")
	if item.Outgoing || !c.dm && !mentioned {
		return
	}

	s.mu.Lock()
	if item.ID <= c.lastRead {
		s.mu.Unlock()
		return
	}
	s.nextNotification++
	id := s.nextNotification
	c.notified = append(c.notified, id)
	s.mu.Unlock()

	s.emit(map[string]any{"event": "notify", "chat": c.id, "group": c.order, "id": id, "date": item.Date, "message": item})
}
