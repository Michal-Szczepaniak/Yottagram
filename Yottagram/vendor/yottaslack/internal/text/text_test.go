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
