package session

import (
	"path/filepath"
	"testing"

	"yottaslack/internal/api"
)

// A bot's icon URL can end in .png and serve a JPEG, which Qt fails to decode
// when the file is named .png.
func TestAvatarFilesHaveNoExtension(t *testing.T) {
	s, err := New(t.TempDir())
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(s.Close)

	photo := s.avatarPhoto(&user{Avatar: "https://a/icon_48.png", AvatarBig: "https://a/icon_192.png"})
	for _, file := range []wireFile{photo.Small, photo.Big} {
		if extension := filepath.Ext(file.Path); extension != "" {
			t.Fatalf("avatar saved as %s", file.Path)
		}
	}

	if attachment := s.fileFor("https://a/picture.png", "", 0, false); filepath.Ext(attachment.Path) != ".png" {
		t.Fatalf("other files keep their extension, got %s", attachment.Path)
	}
}

func TestBotPersonasAreSeparateUsers(t *testing.T) {
	s, err := New(t.TempDir())
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(s.Close)

	persona := func(name, icon string) api.Message {
		return api.Message{BotID: "B1", Username: name, Icons: &api.Icons{Image48: icon, Image192: icon + "big"}}
	}

	alice := s.sender(persona("Alice", "https://a/48"))
	bob := s.sender(persona("Bob", "https://b/48"))
	again := s.sender(persona("Alice", "https://a/48"))

	if alice.ID == bob.ID {
		t.Fatal("different personas share a user")
	}
	if again.ID != alice.ID {
		t.Fatal("the same persona got a new user")
	}
	if alice.FirstName != "Alice" || bob.FirstName != "Bob" {
		t.Fatalf("names are %q and %q", alice.FirstName, bob.FirstName)
	}
	if alice.Avatar != "https://a/48" || alice.AvatarBig != "https://a/48big" {
		t.Fatalf("avatar is %q, %q", alice.Avatar, alice.AvatarBig)
	}

	plain := s.sender(api.Message{BotID: "B2", BotProfile: &api.BotProfile{Name: "Deploys", Icons: &api.Icons{Image72: "https://d/72"}}})
	if plain.SlackID != "B2" || plain.FirstName != "Deploys" || plain.Avatar != "https://d/72" {
		t.Fatalf("plain bot is %+v", plain)
	}
}
