package session

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
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
	marked   string
	scopes   string
	reacted  string
	uploaded string
	comment  string
	posted   string
}

func (f *fakeSlack) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	r.ParseForm()

	switch r.URL.Path {
	case "/auth.test":
		f.mu.Lock()
		w.Header().Set("X-OAuth-Scopes", f.scopes)
		f.mu.Unlock()
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
		fmt.Fprintf(w, `{"ok":true,"channel":{"id":%q,"name":"new","num_members":42,"last_read":"1712345678.000100","purpose":{"value":"talk"}}}`, r.Form.Get("channel"))
	case "/conversations.mark":
		f.mu.Lock()
		f.marked = r.Form.Get("ts")
		f.mu.Unlock()
		fmt.Fprint(w, `{"ok":true}`)
	case "/chat.update":
		fmt.Fprintf(w, `{"ok":true,"message":{"text":%q,"edited":{"ts":"1712345700.000000"}}}`, r.Form.Get("text"))
	case "/reactions.add", "/reactions.remove":
		f.mu.Lock()
		f.reacted = r.URL.Path + " " + r.Form.Get("name")
		f.mu.Unlock()
		fmt.Fprint(w, `{"ok":true}`)
	case "/files.getUploadURLExternal":
		fmt.Fprintf(w, `{"ok":true,"upload_url":"http://%s/upload","file_id":"F1"}`, r.Host)
	case "/upload":
		content, _ := io.ReadAll(r.Body)
		f.mu.Lock()
		f.uploaded = string(content)
		f.mu.Unlock()
	case "/files.completeUploadExternal":
		f.mu.Lock()
		f.comment = r.Form.Get("initial_comment") + " in " + r.Form.Get("channel_id")
		f.mu.Unlock()
		fmt.Fprint(w, `{"ok":true}`)
	case "/emoji.list":
		fmt.Fprint(w, `{"ok":true,"emoji":{"party_parrot":"https://emoji.example/p.gif","parrot":"alias:party_parrot","broken":"alias:missing"}}`)
	case "/chat.delete":
		fmt.Fprint(w, `{"ok":true}`)
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
		f.mu.Lock()
		f.posted = r.Form.Get("text")
		f.mu.Unlock()
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
	s.appAPI.BaseURL = server.URL
	s.pollInterval = time.Hour
	s.previewInterval = time.Hour
	s.markDelay = time.Millisecond
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

func TestLoginLoadsPreviews(t *testing.T) {
	s, fake := newTestSession(t)
	s.previewInterval = time.Millisecond
	fake.setHistory(slackMessage("1712345678.000300", "UANN", "hi"))

	chats := login(t, s)

	last := nextEvent(t, s, "last_message")
	if last["chat"] != float64(chatID(t, chats, "#general")) {
		t.Fatalf("the top of the list goes first, got %v", last)
	}
}

func messageEvent(ts, sender, body string) []byte {
	return []byte(fmt.Sprintf(`{"type":"message","channel":"CGEN","user":%q,"text":%q,"ts":%q}`, sender, body, ts))
}

func TestEventDeliversMessage(t *testing.T) {
	s, _ := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent(messageEvent("1712345678.000300", "UANN", "hi"))

	item := nextEvent(t, s, "new_message")["message"].(map[string]any)
	if item["text"] != "hi" || item["id"] != float64(1712345678000300) {
		t.Fatalf("got %v", item)
	}
	nextEvent(t, s, "last_message")
}

func TestEventWaitsForSend(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	gate := make(chan struct{})
	fake.mu.Lock()
	fake.postGate = gate
	fake.postTS = "1712345678.000300"
	fake.mu.Unlock()

	call(s, 8, "send", fmt.Sprintf(`{"chat":%d,"text":"hi"}`, chatID(t, chats, "#general")))
	nextEvent(t, s, "last_message")

	// The socket is faster than the chat.postMessage reply
	done := make(chan struct{})
	go func() {
		s.handleEvent(messageEvent("1712345678.000300", "UME", "hi"))
		close(done)
	}()
	if raw, ok := s.Next(200 * time.Millisecond); ok {
		t.Fatalf("delivered during the send: %s", raw)
	}

	close(gate)
	nextEvent(t, s, "send_succeeded")
	<-done
}

func TestEventSkipsThreadReplies(t *testing.T) {
	s, _ := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "folders")

	s.handleEvent([]byte(`{"type":"message","channel":"CGEN","user":"UANN","text":"reply","ts":"1712345678.000300","thread_ts":"1712345678.000100"}`))

	if raw, ok := s.Next(200 * time.Millisecond); ok {
		t.Fatalf("got %s", raw)
	}
}

func TestEventEditsMessage(t *testing.T) {
	s, _ := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent([]byte(`{"type":"message","subtype":"message_changed","channel":"CGEN","ts":"1712345679.000000",
		"message":{"ts":"1712345678.000300","user":"UANN","text":"fixed"}}`))

	edit := nextEvent(t, s, "message_content")
	if edit["text"] != "fixed" || edit["message_id"] != float64(1712345678000300) {
		t.Fatalf("got %v", edit)
	}
}

func TestDeletingNewestRefreshesLastMessage(t *testing.T) {
	s, fake := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent(messageEvent("1712345678.000300", "UANN", "oops"))
	nextEvent(t, s, "last_message")

	// Slack can still list the deleted message for a moment
	fake.setHistory(slackMessage("1712345678.000300", "UANN", "oops"), slackMessage("1712345678.000100", "UANN", "older"))
	s.handleEvent([]byte(`{"type":"message","subtype":"message_deleted","channel":"CGEN","deleted_ts":"1712345678.000300","ts":"1712345679.000000"}`))

	nextEvent(t, s, "delete_messages")
	last := nextEvent(t, s, "last_message")["message"].(map[string]any)
	if last["text"] != "older" {
		t.Fatalf("got %v", last)
	}
}

func TestEventAddsUnknownChat(t *testing.T) {
	s, _ := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent([]byte(`{"type":"message","channel":"CNEW","user":"UANN","text":"hi","ts":"1712345678.000300"}`))

	added := nextEvent(t, s, "chats")["chats"].([]any)[0].(map[string]any)
	if added["title"] != "#new" {
		t.Fatalf("got %v", added)
	}
	for _, value := range chats["chats"].([]any) {
		if value.(map[string]any)["order"].(float64) >= added["order"].(float64) {
			t.Fatalf("a new chat goes on top, got %v", added)
		}
	}
	nextEvent(t, s, "new_message")
}

func TestEditFromAppMarksMessageEdited(t *testing.T) {
	s, _ := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	call(s, 8, "edit", fmt.Sprintf(`{"chat":%d,"message":1712345678000300,"text":"fixed\n"}`, chatID(t, chats, "#general")))

	edit := nextEvent(t, s, "message_content")
	if edit["text"] != "fixed" || edit["edit_date"] != float64(1712345700) {
		t.Fatalf("got %v", edit)
	}
}

func TestDeleteFromApp(t *testing.T) {
	s, _ := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	call(s, 8, "delete", fmt.Sprintf(`{"chat":%d,"messages":[1712345678000300]}`, chatID(t, chats, "#general")))

	deleted := nextEvent(t, s, "delete_messages")["ids"].([]any)
	if deleted[0] != float64(1712345678000300) {
		t.Fatalf("got %v", deleted)
	}
}

func TestPropertiesOnlyForOwnMessages(t *testing.T) {
	s, _ := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent(messageEvent("1712345678.000300", "UME", "mine"))
	s.handleEvent(messageEvent("1712345678.000400", "UANN", "theirs"))

	reply := func(id int, message int64) event {
		call(s, id, "properties", fmt.Sprintf(`{"message":%d}`, message))
		for {
			if e := next(t, s); e["id"] == float64(id) {
				return e
			}
		}
	}

	if e := reply(8, 1712345678000300); e["result"] == nil {
		t.Fatalf("got %v", e)
	}
	if e := reply(9, 1712345678000400); e["error"] == nil {
		t.Fatalf("got %v", e)
	}
}

func TestEditingNewestRefreshesPreview(t *testing.T) {
	s, _ := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent(messageEvent("1712345678.000300", "UANN", "oops"))
	nextEvent(t, s, "last_message")

	s.handleEvent([]byte(`{"type":"message","subtype":"message_changed","channel":"CGEN","ts":"1712345679.000000",
		"message":{"ts":"1712345678.000300","user":"UANN","text":"fixed"}}`))

	nextEvent(t, s, "message_content")
	last := nextEvent(t, s, "last_message")["message"].(map[string]any)
	if last["text"] != "fixed" {
		t.Fatalf("got %v", last)
	}
}

func TestPreviewCountsUnread(t *testing.T) {
	s, fake := newTestSession(t)
	s.previewInterval = time.Millisecond
	fake.setHistory(
		slackMessage("1712345678.000300", "UANN", "new"),
		slackMessage("1712345678.000200", "UME", "mine"),
		slackMessage("1712345678.000100", "UANN", "read"),
	)

	login(t, s)

	read := nextEvent(t, s, "read_inbox")
	if read["unread"] != float64(1) || read["last_read"] != float64(1712345678000100) {
		t.Fatalf("got %v", read)
	}
}

func TestViewingMarksReadOnSlack(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent(messageEvent("1712345678.000300", "UANN", "hi"))
	if read := nextEvent(t, s, "read_inbox"); read["unread"] != float64(1) {
		t.Fatalf("got %v", read)
	}

	call(s, 8, "view", fmt.Sprintf(`{"chat":%d,"message":1712345678000300}`, chatID(t, chats, "#general")))
	if read := nextEvent(t, s, "read_inbox"); read["unread"] != float64(0) {
		t.Fatalf("got %v", read)
	}

	deadline := time.Now().Add(testTimeout)
	for time.Now().Before(deadline) {
		fake.mu.Lock()
		marked := fake.marked
		fake.mu.Unlock()
		if marked == "1712345678.000300" {
			return
		}
		time.Sleep(time.Millisecond)
	}
	t.Fatal("conversations.mark wasn't called")
}

func TestNotifiesForMentionsOnly(t *testing.T) {
	s, _ := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent(messageEvent("1712345678.000300", "UANN", "hello all"))
	s.handleEvent(messageEvent("1712345678.000400", "UANN", "hi <@UME>"))

	notified := nextEvent(t, s, "notify")["message"].(map[string]any)
	if notified["id"] != float64(1712345678000400) {
		t.Fatalf("got %v", notified)
	}
}

func TestHuddleBecomesCall(t *testing.T) {
	s, _ := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent(messageEvent("1712345678.000300", "UANN", "Ann and Bob were in the huddle for 14m."))

	item := nextEvent(t, s, "new_message")["message"].(map[string]any)
	content := item["media"].(map[string]any)
	if content["kind"] != "call" || content["duration"] != float64(840) {
		t.Fatalf("got %v", item)
	}
}

func TestMessageWithImage(t *testing.T) {
	s, _ := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent([]byte(`{"type":"message","channel":"CGEN","user":"UANN","text":"look","ts":"1712345678.000300","files":[
		{"name":"a.png","mimetype":"image/png","size":10,"url_private":"https://f.example/a.png","thumb_360":"https://f.example/t.png",
		"thumb_360_w":360,"thumb_360_h":"240","original_w":720,"original_h":480}]}`))

	item := nextEvent(t, s, "new_message")["message"].(map[string]any)
	content := item["media"].(map[string]any)
	if content["kind"] != "photo" || content["thumb_height"] != float64(240) || item["text"] != "look" {
		t.Fatalf("got %v", item)
	}
}

func TestDownloadReportsFile(t *testing.T) {
	s, _ := newTestSession(t)

	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprint(w, "content")
	}))
	t.Cleanup(server.Close)

	file := s.fileFor(server.URL+"/a.png", "", 0, false)
	call(s, 8, "download", fmt.Sprintf(`{"file":%d}`, file.ID))

	deadline := time.Now().Add(testTimeout)
	for time.Now().Before(deadline) {
		state := nextEvent(t, s, "file")["file"].(map[string]any)
		if state["done"] == true {
			if state["size"] != float64(7) {
				t.Fatalf("got %v", state)
			}
			return
		}
	}
	t.Fatal("no finished file")
}

func TestUploadedAvatar(t *testing.T) {
	cases := map[string]string{
		"https://avatars.slack-edge.com/2024/a_72.png":               "https://avatars.slack-edge.com/2024/a_72.png",
		"https://secure.gravatar.com/avatar/abc.jpg?s=72":            "",
		"https://a.slack-edge.com/df10d/img/avatars/ava_0001-72.png": "",
	}

	for input, want := range cases {
		if got := uploadedAvatar(input); got != want {
			t.Errorf("uploadedAvatar(%q) = %q, want %q", input, got, want)
		}
	}
}

func TestReactionsFromHistoryAndEvents(t *testing.T) {
	s, _ := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent([]byte(`{"type":"message","channel":"CGEN","user":"UANN","text":"hi","ts":"1712345678.000300",
		"reactions":[{"name":"+1::skin-tone-2","count":2,"users":["UME","UANN"]},{"name":"company_logo","count":1,"users":["UANN"]}]}`))

	item := nextEvent(t, s, "new_message")["message"].(map[string]any)
	reactions := item["reactions"].([]any)
	first := reactions[0].(map[string]any)
	if first["emoji"] != "👍" || first["count"] != float64(2) || first["chosen"] != true {
		t.Fatalf("got %v", reactions)
	}
	if reactions[1].(map[string]any)["emoji"] != unknownReaction {
		t.Fatalf("got %v", reactions)
	}

	added := []byte(`{"type":"reaction_added","user":"UANN","reaction":"tada","item":{"type":"message","channel":"CGEN","ts":"1712345678.000300"}}`)
	s.handleEvent(added)
	s.handleEvent(added)

	nextEvent(t, s, "reactions")
	updated := nextEvent(t, s, "reactions")["reactions"].([]any)
	last := updated[len(updated)-1].(map[string]any)
	if len(updated) != 3 || last["emoji"] != "🎉" || last["count"] != float64(1) {
		t.Fatalf("the same event twice must count once, got %v", updated)
	}
}

func TestReactFromApp(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	id := chatID(t, chats, "#general")
	call(s, 8, "react", fmt.Sprintf(`{"chat":%d,"message":1712345678000300,"emoji":"❤️","add":true}`, id))

	reactions := nextEvent(t, s, "reactions")["reactions"].([]any)
	first := reactions[0].(map[string]any)
	if first["chosen"] != true {
		t.Fatalf("got %v", reactions)
	}

	fake.mu.Lock()
	defer fake.mu.Unlock()
	if fake.reacted != "/reactions.add heart" {
		t.Fatalf("got %q", fake.reacted)
	}
}

func TestMentionsAreSentToSlack(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	fake.mu.Lock()
	fake.postTS = "1712345678.000300"
	fake.mu.Unlock()

	call(s, 8, "send", fmt.Sprintf(`{"chat":%d,"text":"hi \ue000@ann\ue001 & co"}`, chatID(t, chats, "#general")))

	temp := nextEvent(t, s, "new_message")["message"].(map[string]any)
	if temp["text"] != "hi @ann & co" {
		t.Fatalf("got %q", temp["text"])
	}
	nextEvent(t, s, "send_succeeded")

	fake.mu.Lock()
	defer fake.mu.Unlock()
	if fake.posted != "hi <@UANN> &amp; co" {
		t.Fatalf("got %q", fake.posted)
	}
}

func TestSearchMembers(t *testing.T) {
	s, _ := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "ready")

	call(s, 8, "members", `{"query":"AN","limit":5}`)
	for {
		e := next(t, s)
		if e["id"] != float64(8) {
			continue
		}
		users := e["result"].(map[string]any)["users"].([]any)
		if len(users) != 1 {
			t.Fatalf("got %v", users)
		}
		return
	}
}

func TestUpload(t *testing.T) {
	s, fake := newTestSession(t)
	chats := login(t, s)
	nextEvent(t, s, "ready")

	path := filepath.Join(t.TempDir(), "a.txt")
	if err := os.WriteFile(path, []byte("content"), 0o600); err != nil {
		t.Fatal(err)
	}

	call(s, 8, "upload", fmt.Sprintf(`{"chat":%d,"files":[{"path":%q,"caption":"look"}]}`, chatID(t, chats, "#general"), path))
	for next(t, s)["id"] != float64(8) {
	}

	fake.mu.Lock()
	defer fake.mu.Unlock()
	if fake.uploaded != "content" || fake.comment != "look in CGEN" {
		t.Fatalf("got %q, %q", fake.uploaded, fake.comment)
	}
}

func TestCustomEmoji(t *testing.T) {
	s, _ := newTestSession(t)
	login(t, s)
	nextEvent(t, s, "ready")

	s.handleEvent([]byte(`{"type":"message","channel":"CGEN","user":"UANN","text":"hi :parrot:","ts":"1712345678.000300",
		"reactions":[{"name":"party_parrot","count":1,"users":["UME"]}]}`))

	item := nextEvent(t, s, "new_message")["message"].(map[string]any)
	entity := item["entities"].([]any)[0].(map[string]any)
	reaction := item["reactions"].([]any)[0].(map[string]any)
	if entity["type"] != "custom_emoji" || reaction["custom"] == nil {
		t.Fatalf("got %v", item)
	}

	call(s, 8, "custom_emojis", fmt.Sprintf(`{"ids":[%d]}`, int64(reaction["custom"].(float64))))
	for {
		e := next(t, s)
		if e["id"] != float64(8) {
			continue
		}
		emojis := e["result"].(map[string]any)["emojis"].([]any)
		if len(emojis) != 1 || emojis[0].(map[string]any)["name"] != ":party_parrot:" {
			t.Fatalf("got %v", emojis)
		}
		return
	}
}

func TestAppTokenWaitsForLogin(t *testing.T) {
	s, _ := newTestSession(t)

	s.setAppToken("xapp-test")

	s.mu.Lock()
	started := s.stopRealtime != nil
	s.mu.Unlock()
	if started {
		t.Fatal("realtime must wait for the chat list")
	}
}

func TestReadOnlyTokenWritesNothing(t *testing.T) {
	s, fake := newTestSession(t)
	fake.mu.Lock()
	fake.scopes = "channels:read,channels:history"
	fake.postTS = "1712345678.000300"
	fake.mu.Unlock()

	chats := login(t, s)
	nextEvent(t, s, "ready")

	for _, entry := range chats["chats"].([]any) {
		if entry.(map[string]any)["read_only"] != true {
			t.Fatalf("got %v", entry)
		}
	}

	id := chatID(t, chats, "#general")
	s.handleEvent(messageEvent("1712345678.000300", "UME", "mine"))

	requests := []string{
		fmt.Sprintf(`{"id":8,"method":"send","params":{"chat":%d,"text":"hi"}}`, id),
		fmt.Sprintf(`{"id":9,"method":"edit","params":{"chat":%d,"message":1712345678000300,"text":"hi"}}`, id),
		fmt.Sprintf(`{"id":10,"method":"delete","params":{"chat":%d,"messages":[1712345678000300]}}`, id),
		fmt.Sprintf(`{"id":11,"method":"react","params":{"chat":%d,"message":1712345678000300,"emoji":"👍","add":true}}`, id),
		fmt.Sprintf(`{"id":12,"method":"upload","params":{"chat":%d,"files":[{"path":"/nonexistent"}]}}`, id),
		`{"id":13,"method":"properties","params":{"message":1712345678000300}}`,
	}
	for _, request := range requests {
		s.Handle([]byte(request))
	}

	refused := map[float64]bool{}
	for len(refused) < len(requests) {
		e := next(t, s)
		if id, ok := e["id"].(float64); ok && id >= 8 {
			if e["error"] == nil {
				t.Fatalf("got %v", e)
			}
			refused[id] = true
		}
	}

	// Reading only changes what's shown here, Slack isn't told
	s.handleEvent(messageEvent("1712345678.000400", "UANN", "theirs"))
	call(s, 14, "view", fmt.Sprintf(`{"chat":%d,"message":1712345678000400}`, id))
	for next(t, s)["id"] != float64(14) {
	}
	time.Sleep(50 * time.Millisecond)

	fake.mu.Lock()
	defer fake.mu.Unlock()
	if fake.posted != "" || fake.reacted != "" || fake.uploaded != "" || fake.marked != "" {
		t.Fatalf("a write reached Slack: %q %q %q %q", fake.posted, fake.reacted, fake.uploaded, fake.marked)
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
