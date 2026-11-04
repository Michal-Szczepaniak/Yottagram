package ids

import (
	"path/filepath"
	"testing"
)

func TestTSRoundTrip(t *testing.T) {
	for _, ts := range []string{
		"1712345678.123456",
		"1712345678.000100",
		"1712345678.100000",
		"1712345678.000000",
	} {
		id, err := ParseTS(ts)
		if err != nil {
			t.Fatalf("ParseTS(%q): %v", ts, err)
		}
		if id >= 1<<53 {
			t.Fatalf("id %d doesn't fit in a double", id)
		}
		if got := FormatTS(id); got != ts {
			t.Fatalf("round trip of %q gave %q", ts, got)
		}
	}
}

func TestParseTSRejectsGarbage(t *testing.T) {
	for _, ts := range []string{"", "abc", "1.1234567", "0.000000"} {
		if _, err := ParseTS(ts); err == nil {
			t.Fatalf("ParseTS(%q) should fail", ts)
		}
	}
}

func TestStorePersists(t *testing.T) {
	path := filepath.Join(t.TempDir(), "ids.json")

	store, err := Open(path)
	if err != nil {
		t.Fatal(err)
	}

	user := store.User("U1")
	chat := store.Chat("C1")
	if user != firstSmallID || chat != firstChatID {
		t.Fatalf("unexpected first ids: %d %d", user, chat)
	}
	if store.User("U2") == user {
		t.Fatal("ids must be unique")
	}
	if err := store.Save(); err != nil {
		t.Fatal(err)
	}

	reopened, err := Open(path)
	if err != nil {
		t.Fatal(err)
	}
	if got := reopened.User("U1"); got != user {
		t.Fatalf("user id changed from %d to %d", user, got)
	}
	if got := reopened.Chat("C1"); got != chat {
		t.Fatalf("chat id changed from %d to %d", chat, got)
	}
}
