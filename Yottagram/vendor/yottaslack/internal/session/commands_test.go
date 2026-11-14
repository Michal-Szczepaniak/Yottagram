package session

import "testing"

func TestParseCommand(t *testing.T) {
	cases := []struct {
		in, command, args string
		ok                bool
	}{
		{"/bye", "/bye", "", true},
		{"/remind me in 5 minutes to stretch", "/remind", "me in 5 minutes to stretch", true},
		{"/Giphy\ncats", "/giphy", "cats", true},
		{"/home/me/file.txt", "", "", false},
		{"/", "", "", false},
		{"/ hello", "", "", false},
		{"hello /bye", "", "", false},
	}
	for _, c := range cases {
		command, args, ok := parseCommand(c.in)
		if ok != c.ok || command != c.command || args != c.args {
			t.Errorf("parseCommand(%q) = %q, %q, %v", c.in, command, args, ok)
		}
	}
}
