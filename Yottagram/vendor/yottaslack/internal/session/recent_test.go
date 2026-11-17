package session

import (
	"fmt"
	"os"
	"sync/atomic"
	"testing"
	"time"
)

const testMessageDate = 1712345678

// fakeClock lets a test move time while the session runs.
func fakeClock(s *Session, start int64) *atomic.Int64 {
	var clock atomic.Int64
	clock.Store(start)
	s.now = func() time.Time { return time.Unix(clock.Load(), 0) }
	return &clock
}

// nextPosition skips position events for other lists.
func nextPosition(t *testing.T, s *Session, folder int) event {
	t.Helper()

	deadline := time.Now().Add(testTimeout)
	for time.Now().Before(deadline) {
		if e := nextEvent(t, s, "chat_position"); e["folder"] == float64(folder) {
			return e
		}
	}
	t.Fatalf("no chat_position event for folder %d", folder)
	return nil
}

func chatEntry(t *testing.T, chats event, title string) map[string]any {
	t.Helper()
	for _, value := range chats["chats"].([]any) {
		if entry := value.(map[string]any); entry["title"] == title {
			return entry
		}
	}
	t.Fatalf("no chat %s", title)
	return nil
}

func TestQuietChatLeavesMainList(t *testing.T) {
	s, _ := newTestSession(t)
	clock := fakeClock(s, testMessageDate+int64(24*time.Hour/time.Second))
	chats := login(t, s)
	nextEvent(t, s, "ready")

	general := chatEntry(t, chats, "#general")
	if general["recent"] != true {
		t.Fatalf("a chat with unknown activity must start in the main list, got %v", general)
	}

	s.handleEvent(messageEvent("1712345678.000300", "UANN", "hi"))
	last := nextEvent(t, s, "last_message")
	if last["folder"] != float64(0) || last["order"] != general["order"] {
		t.Fatalf("a day old message keeps the chat in the main list, got %v", last)
	}

	clock.Store(testMessageDate + int64(31*24*time.Hour/time.Second))
	s.updateMainPosition(s.findChat(int64(general["id"].(float64))))
	if position := nextPosition(t, s, 0); position["order"] != float64(0) {
		t.Fatalf("a month quiet chat leaves the main list, got %v", position)
	}
}

func TestOldMessageKeepsChatOutOfMainList(t *testing.T) {
	s, _ := newTestSession(t)
	fakeClock(s, testMessageDate+int64(31*24*time.Hour/time.Second))
	login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent(messageEvent("1712345678.000300", "UANN", "hi"))
	if last := nextEvent(t, s, "last_message"); last["order"] != float64(0) {
		t.Fatalf("a month old message isn't recent, got %v", last)
	}
}

func TestCachedActivityAppliesAtLogin(t *testing.T) {
	s, _ := newTestSession(t)
	fakeClock(s, testMessageDate+int64(31*24*time.Hour/time.Second))

	id := s.ids.Chat("CGEN")
	content := fmt.Sprintf(`{"%d":%d}`, id, testMessageDate)
	if err := os.WriteFile(s.activityPath, []byte(content), 0o600); err != nil {
		t.Fatal(err)
	}

	chats := login(t, s)
	if general := chatEntry(t, chats, "#general"); general["recent"] != false {
		t.Fatalf("a chat quiet since the last run starts out of the main list, got %v", general)
	}
}

func TestUnreadFolder(t *testing.T) {
	s, _ := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	if folders := nextEvent(t, s, "folders")["folders"].([]any); folders[0].(map[string]any)["id"] != float64(folderUnread) {
		t.Fatalf("the Unread folder comes first, got %v", folders)
	}

	general := chatEntry(t, chats, "#general")
	s.handleEvent(messageEvent("1712345678.000300", "UANN", "hi"))
	if added := nextPosition(t, s, folderUnread); added["order"] != general["order"] {
		t.Fatalf("an unread message adds the chat to the Unread folder, got %v", added)
	}

	call(s, 8, "view", fmt.Sprintf(`{"chat":%d,"message":1712345678000300}`, int64(general["id"].(float64))))
	if removed := nextPosition(t, s, folderUnread); removed["order"] != float64(0) {
		t.Fatalf("reading it takes the chat out of the Unread folder, got %v", removed)
	}
}

func TestCountsSetMainListAtLogin(t *testing.T) {
	s, fake := newTestSession(t)
	now := int64(testMessageDate + 30*24*60*60)
	fakeClock(s, now)

	fake.mu.Lock()
	fake.counts = fmt.Sprintf(`{"ok":true,
		"channels":[{"id":"CGEN","latest":"%d.000100","has_unreads":false}],
		"ims":[{"id":"DANN","latest":"%d.000100","has_unreads":true}]}`, testMessageDate, now-60*60)
	fake.mu.Unlock()

	call(s, 2, "login", testLogin)
	chats := nextEvent(t, s, "chats")

	if general := chatEntry(t, chats, "#general"); general["recent"] != false {
		t.Fatalf("a channel quiet for a month starts out of the main list, got %v", general)
	}
	if ann := chatEntry(t, chats, "Ann"); ann["recent"] != true {
		t.Fatalf("a DM from an hour ago is in the main list, got %v", ann)
	}

	s.mu.Lock()
	ann := s.bySlackID["DANN"]
	general := s.bySlackID["CGEN"]
	unreadFirst := s.previewRankLocked(ann) < s.previewRankLocked(general)
	s.mu.Unlock()
	if !unreadFirst {
		t.Fatal("previews must load unread chats first")
	}
}

func TestCountsFailureKeepsPreviewPath(t *testing.T) {
	s, _ := newTestSession(t)
	call(s, 2, "login", testLogin)
	chats := nextEvent(t, s, "chats")

	if general := chatEntry(t, chats, "#general"); general["recent"] != true {
		t.Fatalf("without counts a chat waits for its preview in the main list, got %v", general)
	}
}
