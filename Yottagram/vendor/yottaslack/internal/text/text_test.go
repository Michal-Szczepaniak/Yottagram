package text

import "testing"

func name(id string) string { return "name-" + id }

func TestFromMrkdwn(t *testing.T) {
	cases := map[string]string{
		"plain":                                 "plain",
		"a &amp; b &lt;c&gt;":                   "a & b <c>",
		"&amp;lt;":                              "&lt;",
		"hi <@U1>":                              "hi @name-U1",
		"hi <@U1|ignored>":                      "hi @name-U1",
		"in <#C1|general>":                      "in #general",
		"<!here> look":                          "@here look",
		"<!subteam^S1|@team>":                   "@team",
		"<!date^1392734382^{date}|Feb 18>":      "Feb 18",
		"<https://a.example>":                   "https://a.example",
		"<https://a.example|a site>":            "a site (https://a.example)",
		"<https://a.example|https://a.example>": "https://a.example",
		"<mailto:a@b.example|a@b.example>":      "a@b.example",
	}

	for input, want := range cases {
		if got := FromMrkdwn(input, name); got != want {
			t.Errorf("FromMrkdwn(%q) = %q, want %q", input, got, want)
		}
	}
}

func TestToMrkdwn(t *testing.T) {
	if got := ToMrkdwn("a & <b>"); got != "a &amp; &lt;b&gt;" {
		t.Fatalf("got %q", got)
	}
}

func TestEscapedRoundTrip(t *testing.T) {
	const input = "1 < 2 && 3 > 2 &amp;"
	if got := FromMrkdwn(ToMrkdwn(input), name); got != input {
		t.Fatalf("got %q", got)
	}
}

func TestFormat(t *testing.T) {
	cases := []struct {
		input, text string
		entities    []Entity
	}{
		{"plain snake_case_name 2*3*4", "plain snake_case_name 2*3*4", nil},
		{"*bold* and _it_", "bold and it", []Entity{{Offset: 0, Length: 4, Type: "bold"}, {Offset: 9, Length: 2, Type: "italic"}}},
		{"~gone~", "gone", []Entity{{Offset: 0, Length: 4, Type: "strikethrough"}}},
		{"*_both_*", "both", []Entity{{Offset: 0, Length: 4, Type: "bold"}, {Offset: 0, Length: 4, Type: "italic"}}},
		{"`*a*`", "*a*", []Entity{{Offset: 0, Length: 3, Type: "code"}}},
		{"```x\ny```", "x\ny", []Entity{{Offset: 0, Length: 3, Type: "pre"}}},
		{"😀 *b*", "😀 b", []Entity{{Offset: 3, Length: 1, Type: "bold"}}},
		{"<https://a.example|site>", "site", []Entity{{Offset: 0, Length: 4, Type: "text_url", URL: "https://a.example"}}},
		{"<https://a.example>", "https://a.example", []Entity{{Offset: 0, Length: 17, Type: "url"}}},
		{"hi <@U1>", "hi @name-U1", []Entity{{Offset: 3, Length: 8, Type: "bold"}}},
		{"a &amp; *b*", "a & b", []Entity{{Offset: 4, Length: 1, Type: "bold"}}},
		{"nice :+1:", "nice 👍", nil},
		{":+1::skin-tone-3: ok", "👍 ok", nil},
		{"at 12:30:45 :not_an_emoji:", "at 12:30:45 :not_an_emoji:", nil},
		{"`:smile:`", ":smile:", []Entity{{Offset: 0, Length: 7, Type: "code"}}},
	}

	for _, c := range cases {
		got, entities := Format(c.input, name)
		if got != c.text {
			t.Errorf("Format(%q) = %q, want %q", c.input, got, c.text)
		}
		if len(entities) != len(c.entities) {
			t.Errorf("Format(%q) entities = %v, want %v", c.input, entities, c.entities)
			continue
		}
		for i := range entities {
			if entities[i] != c.entities[i] {
				t.Errorf("Format(%q) entity %d = %v, want %v", c.input, i, entities[i], c.entities[i])
			}
		}
	}
}

func TestFormatWithCustomEmoji(t *testing.T) {
	custom := func(name string) int64 {
		if name == "party_parrot" {
			return 7
		}
		return 0
	}

	got, entities := FormatWith("a :party_parrot: :nope:", name, custom)
	if got != "a :party_parrot: :nope:" {
		t.Fatalf("got %q", got)
	}
	if len(entities) != 1 || entities[0] != (Entity{Offset: 2, Length: 14, Type: "custom_emoji", Emoji: 7}) {
		t.Fatalf("got %v", entities)
	}
}

func TestTrimKeepsEntitiesOnText(t *testing.T) {
	value, entities := Trim("\n bold \n", []Entity{{Offset: 2, Length: 4, Type: "bold"}})
	if value != "bold" || entities[0].Offset != 0 || entities[0].Length != 4 {
		t.Fatalf("got %q, %v", value, entities)
	}
}
