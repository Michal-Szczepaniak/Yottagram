package session

import (
	"context"
	"encoding/json"
	"time"

	"yottaslack/internal/api"
	"yottaslack/internal/ids"
	"yottaslack/internal/socket"
)

const (
	socketReadTimeout  = 150 * time.Second
	socketPingInterval = 60 * time.Second
	maxSocketBackoff   = 60 * time.Second
	safetyPollInterval = 60 * time.Second
	eventBacklog       = 256
)

type envelope struct {
	EnvelopeID string `json:"envelope_id"`
	Type       string `json:"type"`
	Payload    struct {
		Event json.RawMessage `json:"event"`
	} `json:"payload"`
}

type slackEvent struct {
	api.Message
	Type      string      `json:"type"`
	Subtype   string      `json:"subtype"`
	Channel   string      `json:"channel"`
	Hidden    bool        `json:"hidden"`
	DeletedTS string      `json:"deleted_ts"`
	Changed   api.Message `json:"message"`
}

// startRealtime replaces the running connection, if any. An empty token leaves
// the session on polling.
func (s *Session) startRealtime(token string) {
	s.mu.Lock()
	if s.stopRealtime != nil {
		s.stopRealtime()
		s.stopRealtime = nil
	}
	s.realtimeUp = false

	if token == "" {
		s.mu.Unlock()
		return
	}

	ctx, cancel := context.WithCancel(s.ctx)
	s.stopRealtime = cancel
	s.mu.Unlock()

	s.appAPI.SetToken(token)

	events := make(chan json.RawMessage, eventBacklog)
	s.spawn(func() { s.handleEvents(ctx, events) })
	s.spawn(func() { s.realtime(ctx, events) })
}

func (s *Session) setRealtimeUp(up bool) {
	s.mu.Lock()
	s.realtimeUp = up
	s.mu.Unlock()
}

func (s *Session) isRealtimeUp() bool {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.realtimeUp
}

// realtime keeps a connection open. Slack drops them routinely, so this is the
// normal path and not only for network failures.
func (s *Session) realtime(ctx context.Context, events chan<- json.RawMessage) {
	backoff := time.Second

	for {
		connected, err := s.runSocket(ctx, events)
		if ctx.Err() != nil {
			return
		}
		s.setRealtimeUp(false)

		if api.IsAuthError(err) {
			s.log("realtime stopped: %v", err)
			return
		}
		if err != nil {
			s.log("realtime disconnected: %v", err)
		}
		if connected {
			backoff = time.Second
		}

		select {
		case <-time.After(backoff):
		case <-ctx.Done():
			return
		}

		if backoff *= 2; backoff > maxSocketBackoff {
			backoff = maxSocketBackoff
		}
	}
}

// runSocket reads one connection until it ends, and reports whether Slack
// accepted it.
func (s *Session) runSocket(ctx context.Context, events chan<- json.RawMessage) (bool, error) {
	url, err := s.appAPI.OpenSocket(ctx)
	if err != nil {
		return false, err
	}

	conn, err := socket.Dial(ctx, url, socketReadTimeout)
	if err != nil {
		return false, err
	}
	defer conn.Close()

	done := make(chan struct{})
	defer close(done)
	s.spawn(func() { s.keepAlive(ctx, conn, done) })

	connected := false
	for {
		raw, err := conn.Read()
		if err != nil {
			return connected, err
		}

		var e envelope
		if err := json.Unmarshal(raw, &e); err != nil {
			s.log("invalid socket message: %v", err)
			continue
		}

		// Slack resends what isn't acknowledged within seconds
		if e.EnvelopeID != "" {
			ack, err := json.Marshal(map[string]string{"envelope_id": e.EnvelopeID})
			if err != nil {
				return connected, err
			}
			if err := conn.WriteText(ack); err != nil {
				return connected, err
			}
		}

		switch e.Type {
		case "hello":
			connected = true
			s.setRealtimeUp(true)
			s.log("realtime connected")
			s.spawn(s.catchUp)
		case "disconnect":
			return connected, nil
		case "events_api":
			select {
			case events <- e.Payload.Event:
			case <-ctx.Done():
				return connected, ctx.Err()
			}
		}
	}
}

func (s *Session) keepAlive(ctx context.Context, conn *socket.Conn, done <-chan struct{}) {
	ticker := time.NewTicker(socketPingInterval)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			if err := conn.Ping(); err != nil {
				conn.Close()
				return
			}
		case <-ctx.Done():
			conn.Close()
			return
		case <-done:
			return
		}
	}
}

// catchUp fetches what was missed while disconnected, for the chats on screen.
// The others catch up when they're opened.
func (s *Session) catchUp() {
	s.mu.Lock()
	var open []*chat
	for _, c := range s.chats {
		if c.stopPoll != nil {
			open = append(open, c)
		}
	}
	s.mu.Unlock()

	for _, c := range open {
		if err := s.pollOnce(c); err != nil {
			s.log("catching up failed: %v", err)
		}
	}
}

// handleEvents runs apart from the socket, as an event can wait on a send
// while pings still have to be answered.
func (s *Session) handleEvents(ctx context.Context, events <-chan json.RawMessage) {
	for {
		select {
		case raw := <-events:
			s.handleEvent(raw)
		case <-ctx.Done():
			return
		}
	}
}

func (s *Session) handleEvent(raw []byte) {
	defer func() {
		if r := recover(); r != nil {
			s.log("panic: %v", r)
		}
	}()

	var e slackEvent
	if err := json.Unmarshal(raw, &e); err != nil {
		s.log("invalid event: %v", err)
		return
	}
	if e.Type != "message" {
		return
	}

	c := s.chatForSlackID(e.Channel)
	if c == nil {
		return
	}

	// Our own message can arrive before chat.postMessage answers. Waiting for
	// the send means the app already has it under its real id, and ignores this.
	c.sending.RLock()
	defer c.sending.RUnlock()

	switch e.Subtype {
	case "message_changed":
		s.changeMessage(c, e.Changed)
	case "message_deleted":
		s.deleteMessage(c, e.DeletedTS)
	default:
		isReply := e.ThreadTS != "" && e.ThreadTS != e.TS && e.Subtype != "thread_broadcast"
		if !e.Hidden && !isReply {
			s.receiveMessage(c, e.Message)
		}
	}
}

func (s *Session) receiveMessage(c *chat, m api.Message) {
	item, err := s.toMessage(c, m)
	if err != nil {
		s.log("skipping event: %v", err)
		return
	}

	s.mu.Lock()
	isNewest := item.ID > c.newest
	if isNewest {
		c.newest = item.ID
	}
	s.mu.Unlock()

	s.emit(map[string]any{"event": "new_message", "message": item})
	if isNewest {
		s.emitLastMessage(item)
	}
}

func (s *Session) changeMessage(c *chat, m api.Message) {
	item, err := s.toMessage(c, m)
	if err != nil {
		s.log("skipping edit: %v", err)
		return
	}

	s.mu.Lock()
	isNewest := item.ID == c.newest
	s.mu.Unlock()

	s.emit(map[string]any{"event": "message_content", "chat": c.id, "message_id": item.ID, "text": item.Text, "edit_date": item.EditDate})

	// Must follow the content, this is what refreshes the chat list's preview
	if isNewest {
		s.emitLastMessage(item)
	}
}

func (s *Session) deleteMessage(c *chat, ts string) {
	id, err := ids.ParseTS(ts)
	if err != nil {
		s.log("skipping delete: %v", err)
		return
	}

	s.mu.Lock()
	wasNewest := id == c.newest
	s.mu.Unlock()

	s.emit(map[string]any{"event": "delete_messages", "chat": c.id, "ids": []int64{id}})
	if !wasNewest {
		return
	}

	// The app only shows new messages while its newest one is the chat's last.
	// Right after a delete, Slack can still return the deleted message.
	page, err := s.api.History(s.ctx, c.slackID, api.HistoryParams{Limit: 2})
	if err != nil {
		s.log("refreshing last message failed: %v", err)
		return
	}

	messages := page.Messages
	if len(messages) > 0 && messages[0].TS == ts {
		messages = messages[1:]
	}
	if len(messages) == 0 {
		return
	}

	item, err := s.toMessage(c, messages[0])
	if err != nil {
		s.log("skipping message: %v", err)
		return
	}

	s.mu.Lock()
	c.newest = item.ID
	s.mu.Unlock()

	s.emitLastMessage(item)
}

// chatForSlackID returns the chat for an event, adding it if it's new, such as
// a DM someone just started.
func (s *Session) chatForSlackID(slackID string) *chat {
	s.mu.Lock()
	known := s.bySlackID[slackID]
	s.mu.Unlock()
	if known != nil {
		return known
	}

	conversation, err := s.api.ConversationInfo(s.ctx, slackID)
	if err != nil {
		s.log("conversations.info %s failed: %v", slackID, err)
		return nil
	}

	entry := s.makeWireChat(conversation)
	c := s.trackChat(entry, slackID)

	s.mu.Lock()
	s.nextOrder++
	entry.Order = s.nextOrder
	c.order = entry.Order
	s.mu.Unlock()

	s.emit(map[string]any{"event": "chats", "chats": []wireChat{entry}})
	if err := s.ids.Save(); err != nil {
		s.log("saving ids failed: %v", err)
	}
	return c
}
