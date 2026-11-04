// Package ids maps Slack's string identifiers to the integers the app expects.
//
// Limits come from how Yottagram stores ids: anything reaching QML must stay
// below 2^53, and users and supergroups must fit in an int32.
package ids

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
)

const (
	firstChatID  int64 = 1 << 40
	firstSmallID int64 = 1
	tsFraction         = 6
)

// ParseTS converts a Slack timestamp to integer microseconds, which keeps
// message ids unique, time-ordered and reversible.
func ParseTS(ts string) (int64, error) {
	seconds, fraction, found := strings.Cut(ts, ".")
	if !found {
		fraction = ""
	}
	if len(fraction) > tsFraction {
		return 0, fmt.Errorf("timestamp %q has too many digits", ts)
	}
	fraction += strings.Repeat("0", tsFraction-len(fraction))

	value, err := strconv.ParseInt(seconds+fraction, 10, 64)
	if err != nil {
		return 0, fmt.Errorf("invalid timestamp %q: %w", ts, err)
	}
	if value <= 0 {
		return 0, errors.New("timestamp must be positive")
	}
	return value, nil
}

// FormatTS is the inverse of ParseTS.
func FormatTS(id int64) string {
	return fmt.Sprintf("%d.%06d", id/1000000, id%1000000)
}

type table struct {
	Next int64            `json:"next"`
	IDs  map[string]int64 `json:"ids"`
}

func newTable(first int64) *table {
	return &table{Next: first, IDs: map[string]int64{}}
}

func (t *table) get(key string) (int64, bool) {
	id, known := t.IDs[key]
	if known {
		return id, false
	}
	id = t.Next
	t.Next++
	t.IDs[key] = id
	return id, true
}

// Store is a persisted set of id tables. Ids must survive restarts because
// the app keys per-chat settings on them.
type Store struct {
	mu    sync.Mutex
	path  string
	dirty bool
	data  storeData
}

type storeData struct {
	Users       *table `json:"users"`
	Supergroups *table `json:"supergroups"`
	Chats       *table `json:"chats"`
}

// Open loads the store at path. A missing file gives an empty store.
func Open(path string) (*Store, error) {
	s := &Store{path: path}

	content, err := os.ReadFile(path)
	if err == nil {
		if err := json.Unmarshal(content, &s.data); err != nil {
			return nil, fmt.Errorf("reading %s: %w", path, err)
		}
	} else if !errors.Is(err, os.ErrNotExist) {
		return nil, err
	}

	if s.data.Users == nil {
		s.data.Users = newTable(firstSmallID)
	}
	if s.data.Supergroups == nil {
		s.data.Supergroups = newTable(firstSmallID)
	}
	if s.data.Chats == nil {
		s.data.Chats = newTable(firstChatID)
	}
	return s, nil
}

func (s *Store) lookup(t *table, key string) int64 {
	s.mu.Lock()
	defer s.mu.Unlock()

	id, created := t.get(key)
	if created {
		s.dirty = true
	}
	return id
}

// User returns the id for a Slack user or bot.
func (s *Store) User(slackID string) int64 { return s.lookup(s.data.Users, slackID) }

// Supergroup returns the id for a Slack channel when treated as a supergroup.
func (s *Store) Supergroup(slackID string) int64 { return s.lookup(s.data.Supergroups, slackID) }

// Chat returns the id for a Slack conversation.
func (s *Store) Chat(slackID string) int64 { return s.lookup(s.data.Chats, slackID) }

// Save writes the store if anything changed.
func (s *Store) Save() error {
	s.mu.Lock()
	defer s.mu.Unlock()

	if !s.dirty {
		return nil
	}

	content, err := json.Marshal(&s.data)
	if err != nil {
		return err
	}
	if err := WriteFile(s.path, content); err != nil {
		return err
	}
	s.dirty = false
	return nil
}

// WriteFile replaces path atomically, readable by the owner only.
func WriteFile(path string, content []byte) error {
	temp, err := os.CreateTemp(filepath.Dir(path), filepath.Base(path)+".*")
	if err != nil {
		return err
	}
	defer os.Remove(temp.Name())

	if _, err := temp.Write(content); err != nil {
		temp.Close()
		return err
	}
	if err := temp.Close(); err != nil {
		return err
	}
	return os.Rename(temp.Name(), path)
}
