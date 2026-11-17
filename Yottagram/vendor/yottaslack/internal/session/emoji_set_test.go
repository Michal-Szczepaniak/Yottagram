package session

import "testing"

func TestCustomEmojiSetSkipsAliases(t *testing.T) {
	s, err := New(t.TempDir())
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(s.Close)

	s.mu.Lock()
	s.customEmoji["party"] = customEmoji{id: 2, url: "https://e/party.gif"}
	s.customEmoji["yay"] = customEmoji{id: 3, url: "https://e/party.gif", alias: true}
	s.customEmoji["approved"] = customEmoji{id: 1, url: "https://e/approved.png"}
	s.mu.Unlock()

	s.customEmojiSet(5, workspaceEmojiSet)
	e := next(t, s)
	if e["id"] != float64(5) {
		t.Fatalf("unexpected reply %v", e)
	}
	emojis := e["result"].(map[string]any)["emojis"].([]any)
	if len(emojis) != 2 {
		t.Fatalf("aliases must be left out, got %v", emojis)
	}
	first := emojis[0].(map[string]any)
	if first["name"] != ":approved:" || first["id"] != float64(1) || first["file"] == nil {
		t.Fatalf("emoji are sorted by name with a file, got %v", first)
	}

	s.availableReactions(6)
	e = next(t, s)
	found := false
	for _, emoji := range e["result"].(map[string]any)["emojis"].([]any) {
		found = found || emoji == ":party:"
	}
	if !found {
		t.Fatal("custom emoji must be available as reactions")
	}
}
