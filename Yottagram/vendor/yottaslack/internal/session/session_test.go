package session

import (
	"encoding/json"
	"fmt"
	"net/http"
	"net/http/httptest"
	"strings"
	"sync"
	"testing"
	"time"
)

const testTimeout = 5 * time.Second

// fakeSlack serves a tiny workspace. Set postGate to hold chat.postMessage open.
type fakeSlack struct {
	mu       sync.Mutex
	history  []string
	postGate chan struct{}
	postTS   string
}

func (f *fakeSlack) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	r.ParseForm()

	switch r.URL.Path {
	case "/auth.test":
		fmt.Fprint(w, `{"ok":true,"user_id":"UME"}`)
	case "/users.info":
		fmt.Fprintf(w, `{"ok":true,"user":{"id":%q,"name":%q}}`, r.Form.Get("user"), strings.ToLower(r.Form.Get("user")))
	case "/users.list":
		fmt.Fprint(w, `{"ok":true,"members":[{"id":"UME","name":"me"},{"id":"UANN","name":"ann","profile":{"display_name":"Ann"}}]}`)
	case "/users.conversations":
		fmt.Fprint(w, `{"ok":true,"channels":[
			{"id":"CGEN","name":"general"},
			{"id":"DANN","is_im":true,"user":"UANN"},
			{"id":"GMP","name":"mpdm-ann--me-1","is_mpim":true}]}`)
	case "/conversations.info":
		fmt.Fprint(w, `{"ok":true,"channel":{"id":"CGEN","num_members":42}}`)
	case "/conversations.history":
		f.mu.Lock()
		items := strings.Join(f.history, ",")
		f.mu.Unlock()
		fmt.Fprintf(w, `{"ok":true,"messages":[%s]}`, items)
	case "/chat.postMessage":
		f.mu.Lock()
		gate := f.postGate
		ts := f.postTS
		f.mu.Unlock()
		if gate != nil {
			<-gate
		}
		if ts == "" {
			fmt.Fprint(w, `{"ok":false,"error":"channel_not_found"}`)
			return
		}
		fmt.Fprintf(w, `{"ok":true,"ts":%q,"message":{"ts":%q,"user":"UME","text":%q}}`, ts, ts, r.Form.Get("text"))
	default:
		http.NotFound(w, r)
	}
}

func (f *fakeSlack) setHistory(items ...string) {
	f.mu.Lock()
	defer f.mu.Unlock()
	f.history = items
}

func slackMessage(ts, sender, body string) string {
	return fmt.Sprintf(`{"ts":%q,"user":%q,"text":%q}`, ts, sender, body)
}

func newTestSession(t *testing.T) (*Session, *fakeSlack) {
	t.Helper()

	fake := &fakeSlack{}
	server := httptest.NewServer(fake)
	t.Cleanup(server.Close)

	s, err := New(t.TempDir())
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(s.Close)

	s.api.BaseURL = server.URL
	s.pollInterval = time.Hour
	return s, fake
}

type event map[string]any

func next(t *testing.T, s *Session) event {
	t.Helper()

	raw, ok := s.Next(testTimeout)
	if !ok {
		t.Fatal("timed out waiting for an event")
	}

	var e event
	if err := json.Unmarshal([]byte(raw), &e); err != nil {
		t.Fatal(err)
	}
	return e
}

func nextEvent(t *testing.T, s *Session, name string) event {
	t.Helper()

	deadline := time.Now().Add(testTimeout)
	for time.Now().Before(deadline) {
		if e := next(t, s); e["event"] == name {
			return e
		}
	}
	t.Fatalf("no %s event", name)
	return nil
}

func call(s *Session, id int, method string, params string) {
	s.Handle([]byte(fmt.Sprintf(`{"id":%d,"method":%q,"params":%s}`, id, method, params)))
}

func login(t *testing.T, s *Session) event {
	t.Helper()
	call(s, 2, "login", `{"token":"xoxp-test"}`)
	return nextEvent(t, s, "chats")
}

func chatID(t *testing.T, chats event, title string) int64 {
	t.Helper()
	for _, value := range chats["chats"].([]any) {
		entry := value.(map[string]any)
		if entry["title"] == title {
			return int64(entry["id"].(float64))
		}
	}
	t.Fatalf("no chat titled %q", title)
	return 0
}

func TestLoginEventOrder(t *testing.T) {
	s, _ := newTestSession(t)

	call(s, 2, "login", `{"token":"xoxp-test"}`)

	var order []string
	for len(order) == 0 || order[len(order)-1] != "ready" {
		e := next(t, s)
		if name, ok := e["event"].(string); ok {
			order = append(order, name)
		} else {
			order = append(order, "reply")
		}
	}

	joined := strings.Join(order, " ")
	if !strings.HasPrefix(joined, "reply auth_ready users") {
		t.Fatalf("bad start: %s", joined)
	}
	if !strings.HasSuffix(joined, "chats ready") {
		t.Fatalf("bad end: %s", joined)
	}
}

func TestChatOrderIsFixed(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	orders := map[string]float64{}
	for _, value := range chats["chats"].([]any) {
		entry := value.(map[string]any)
		orders[entry["type"].(string)] = entry["order"].(float64)
		if entry["title"] == "#general" {
			orders["general"] = entry["order"].(float64)
		}
	}
	if orders["general"] <= orders["private"] {
		t.Fatalf("channels must come before direct messages, got %v", orders)
	}

	fake.mu.Lock()
	fake.postTS = "1712345678.000300"
	fake.mu.Unlock()

	call(s, 8, "send", fmt.Sprintf(`{"chat":%d,"text":"hi"}`, chatID(t, chats, "#general")))
	if last := nextEvent(t, s, "last_message"); last["order"] != orders["general"] {
		t.Fatalf("activity moved the chat, got %v", last["order"])
	}
}

func TestOpenReportsMemberCount(t *testing.T) {
	s, _ := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	call(s, 8, "open", fmt.Sprintf(`{"chat":%d}`, chatID(t, chats, "#general")))

	count := nextEvent(t, s, "member_count")
	if count["count"] != float64(42) || count["supergroup_id"] == float64(0) {
		t.Fatalf("got %v", count)
	}
}

func TestChatTitles(t *testing.T) {
	s, _ := newTestSession(t)

	chats := login(t, s)
	for _, title := range []string{"#general", "Ann", "ann, me"} {
		chatID(t, chats, title)
	}
}

func TestInvalidTokenIsUnauthorized(t *testing.T) {
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprint(w, `{"ok":false,"error":"invalid_auth"}`)
	}))
	defer server.Close()

	s, err := New(t.TempDir())
	if err != nil {
		t.Fatal(err)
	}
	defer s.Close()
	s.api.BaseURL = server.URL

	call(s, 2, "login", `{"token":"bad","retry":true}`)

	e := next(t, s)
	failure, ok := e["error"].(map[string]any)
	if !ok {
		t.Fatalf("expected an error, got %v", e)
	}
	if failure["code"].(float64) != codeUnauthorized {
		t.Fatalf("got %v", failure)
	}
}

func TestHistoryReplyPrecedesLastMessage(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	fake.setHistory(
		slackMessage("1712345678.000200", "UANN", "second"),
		slackMessage("1712345678.000100", "UME", "first"),
	)

	id := chatID(t, chats, "#general")
	call(s, 7, "history", fmt.Sprintf(`{"chat":%d,"from":0,"offset":-20,"limit":40}`, id))

	reply := next(t, s)
	for reply["id"] == nil {
		reply = next(t, s)
	}

	result := reply["result"].(map[string]any)
	messages := result["messages"].([]any)
	if len(messages) != 2 {
		t.Fatalf("got %d messages", len(messages))
	}
	newest := messages[0].(map[string]any)
	if newest["text"] != "second" {
		t.Fatalf("history must be newest first, got %v", newest)
	}
	if messages[1].(map[string]any)["outgoing"] != true {
		t.Fatal("own message must be outgoing")
	}

	last := nextEvent(t, s, "last_message")
	if last["message"].(map[string]any)["id"] != newest["id"] {
		t.Fatalf("got %v", last)
	}
}

func TestSendSwapsTemporaryID(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	fake.mu.Lock()
	fake.postTS = "1712345678.000300"
	fake.mu.Unlock()

	id := chatID(t, chats, "#general")
	call(s, 8, "send", fmt.Sprintf(`{"chat":%d,"text":"a < b"}`, id))

	temp := nextEvent(t, s, "new_message")["message"].(map[string]any)
	if temp["text"] != "a < b" {
		t.Fatalf("got %v", temp)
	}

	succeeded := nextEvent(t, s, "send_succeeded")
	if succeeded["old_id"] != temp["id"] {
		t.Fatalf("got %v", succeeded)
	}
	sent := succeeded["message"].(map[string]any)
	if sent["text"] != "a < b" {
		t.Fatalf("escaping must round trip, got %v", sent)
	}
}

func TestSendTrimsWhitespace(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	fake.mu.Lock()
	fake.postTS = "1712345678.000300"
	fake.mu.Unlock()

	id := chatID(t, chats, "#general")
	call(s, 8, "send", fmt.Sprintf(`{"chat":%d,"text":"hi\n"}`, id))

	temp := nextEvent(t, s, "new_message")["message"].(map[string]any)
	if temp["text"] != "hi" {
		t.Fatalf("got %q", temp["text"])
	}
}

func TestFailedSendRemovesMessage(t *testing.T) {
	s, _ := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	id := chatID(t, chats, "#general")
	call(s, 8, "send", fmt.Sprintf(`{"chat":%d,"text":"hi"}`, id))

	temp := nextEvent(t, s, "new_message")["message"].(map[string]any)
	deleted := nextEvent(t, s, "delete_messages")
	if deleted["ids"].([]any)[0] != temp["id"] {
		t.Fatalf("got %v", deleted)
	}

	if reply := next(t, s); reply["error"] == nil {
		t.Fatalf("expected an error reply, got %v", reply)
	}
}

func TestPollSkipsWhileSendInFlight(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	id := chatID(t, chats, "#general")
	c := s.findChat(id)

	s.mu.Lock()
	c.loaded = true
	c.newest = 1712345678000100
	s.mu.Unlock()

	gate := make(chan struct{})
	fake.mu.Lock()
	fake.postGate = gate
	fake.postTS = "1712345678.000300"
	fake.mu.Unlock()

	call(s, 8, "send", fmt.Sprintf(`{"chat":%d,"text":"hi"}`, id))
	nextEvent(t, s, "last_message")

	// Slack already has the message, but its reply hasn't arrived
	fake.setHistory(slackMessage("1712345678.000300", "UME", "hi"))
	if err := s.pollOnce(c); err != nil {
		t.Fatal(err)
	}

	close(gate)
	nextEvent(t, s, "send_succeeded")
	nextEvent(t, s, "last_message")

	// Now it's known, so polling must not announce it again
	if err := s.pollOnce(c); err != nil {
		t.Fatal(err)
	}
	if raw, ok := s.Next(200 * time.Millisecond); ok && strings.Contains(raw, "new_message") {
		t.Fatalf("message delivered twice: %s", raw)
	}
}

func TestHistoryWaitsForSendInFlight(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	id := chatID(t, chats, "#general")

	gate := make(chan struct{})
	fake.mu.Lock()
	fake.postGate = gate
	fake.postTS = "1712345678.000300"
	fake.mu.Unlock()

	call(s, 8, "send", fmt.Sprintf(`{"chat":%d,"text":"hi"}`, id))
	nextEvent(t, s, "last_message")

	// Slack already has the message, but its reply hasn't arrived
	fake.setHistory(slackMessage("1712345678.000300", "UME", "hi"))
	call(s, 9, "history", fmt.Sprintf(`{"chat":%d,"from":0,"offset":0,"limit":10}`, id))
	if raw, ok := s.Next(200 * time.Millisecond); ok {
		t.Fatalf("answered during the send: %s", raw)
	}

	close(gate)
	nextEvent(t, s, "send_succeeded")
	for next(t, s)["id"] != float64(9) {
	}
}

func TestPollDeliversNewMessagesInOrder(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	id := chatID(t, chats, "#general")
	c := s.findChat(id)

	s.mu.Lock()
	c.loaded = true
	c.newest = 1712345678000100
	s.mu.Unlock()

	fake.setHistory(
		slackMessage("1712345678.000300", "UANN", "b"),
		slackMessage("1712345678.000200", "UANN", "a"),
	)
	if err := s.pollOnce(c); err != nil {
		t.Fatal(err)
	}

	for _, want := range []string{"a", "b"} {
		got := nextEvent(t, s, "new_message")["message"].(map[string]any)
		if got["text"] != want {
			t.Fatalf("got %v, want %q", got, want)
		}
		nextEvent(t, s, "last_message")
	}
}

func TestMpimTitle(t *testing.T) {
	if got := mpimTitle("mpdm-ann--bob--cid-1"); got != "ann, bob, cid" {
		t.Fatalf("got %q", got)
	}
}
