package session

import (
	"encoding/json"
	"os"
	"strconv"
	"time"

	"yottaslack/internal/ids"
)

// The main list only shows chats with activity in the last week, like the
// desktop sidebar's recent filter. The per-type folders keep every chat, and
// the Unread folder shows chats with unread messages.

const (
	folderUnread = 5
	recentWindow = 7 * 24 * time.Hour
	recentCheck  = time.Minute
)

// mainOrderLocked is the chat's order in the main list, 0 to leave it out.
// A chat whose activity isn't known yet stays in until its preview loads.
// Callers hold s.mu.
func (s *Session) mainOrderLocked(c *chat) int64 {
	if c.threadTS != "" {
		return 0
	}
	if c.activityKnown && !time.Unix(c.lastActivity, 0).After(s.now().Add(-recentWindow)) {
		return 0
	}
	return c.order
}

// noteActivityLocked records a message date, 0 for a chat without messages.
// Callers hold s.mu.
func (s *Session) noteActivityLocked(c *chat, date int64) {
	if c.activityKnown && date <= c.lastActivity {
		return
	}
	c.activityKnown = true
	c.lastActivity = date
	if c.threadTS == "" {
		s.activity[c.id] = date
		s.activityDirty = true
	}
}

// applyCachedActivityLocked restores what the last run knew, so the main list
// is right before the previews load. Callers hold s.mu.
func (s *Session) applyCachedActivityLocked(c *chat) {
	if date, ok := s.activity[c.id]; ok {
		c.activityKnown = true
		c.lastActivity = date
	}
}

// updateMainPosition adds or removes the chat from the main list if its
// recency changed.
func (s *Session) updateMainPosition(c *chat) {
	s.mu.Lock()
	order := s.mainOrderLocked(c)
	changed := c.inMain != (order != 0)
	c.inMain = order != 0
	s.mu.Unlock()

	if changed {
		s.emit(map[string]any{"event": "chat_position", "chat": c.id, "folder": 0, "order": order})
	}
}

// watchActivity takes chats out of the main list as they go quiet, and saves
// the activity dates. Not saving on close loses at most a minute of them.
func (s *Session) watchActivity() {
	ticker := time.NewTicker(recentCheck)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
		case <-s.ctx.Done():
			return
		}

		s.mu.Lock()
		chats := make([]*chat, 0, len(s.chats))
		for _, c := range s.chats {
			chats = append(chats, c)
		}
		s.mu.Unlock()

		for _, c := range chats {
			s.updateMainPosition(c)
		}
		s.saveActivity()
	}
}

func (s *Session) loadActivity() {
	content, err := os.ReadFile(s.activityPath)
	if err != nil {
		return
	}

	var cached map[string]int64
	if err := json.Unmarshal(content, &cached); err != nil {
		s.log("ignoring activity cache: %v", err)
		return
	}

	s.mu.Lock()
	defer s.mu.Unlock()
	for key, date := range cached {
		if id, err := strconv.ParseInt(key, 10, 64); err == nil {
			s.activity[id] = date
		}
	}
}

func (s *Session) saveActivity() {
	s.mu.Lock()
	if !s.activityDirty {
		s.mu.Unlock()
		return
	}
	cached := make(map[string]int64, len(s.activity))
	for id, date := range s.activity {
		cached[strconv.FormatInt(id, 10)] = date
	}
	s.activityDirty = false
	s.mu.Unlock()

	content, err := json.Marshal(cached)
	if err != nil {
		return
	}
	if err := ids.WriteFile(s.activityPath, content); err != nil {
		s.log("saving activity failed: %v", err)
	}
}
