// Package text converts between Slack's mrkdwn and plain text.
package text

import (
	"regexp"
	"strings"
)

var token = regexp.MustCompile(`<([^<>]*)>`)

var unescaper = strings.NewReplacer("&lt;", "<", "&gt;", ">", "&amp;", "&")

var escaper = strings.NewReplacer("&", "&amp;", "<", "&lt;", ">", "&gt;")

// FromMrkdwn renders Slack's message markup as plain text. userName resolves
// a Slack user id to a display name.
func FromMrkdwn(value string, userName func(id string) string) string {
	rendered := token.ReplaceAllStringFunc(value, func(match string) string {
		return renderToken(match[1:len(match)-1], userName)
	})
	return unescaper.Replace(rendered)
}

func renderToken(body string, userName func(id string) string) string {
	target, label, hasLabel := strings.Cut(body, "|")

	switch {
	case strings.HasPrefix(target, "@"):
		return "@" + userName(target[1:])
	case strings.HasPrefix(target, "#"):
		if hasLabel {
			return "#" + label
		}
		return target
	case strings.HasPrefix(target, "!"):
		if hasLabel {
			return label
		}
		return "@" + target[1:]
	case hasLabel && label != target && strings.TrimPrefix(target, "mailto:") != label:
		return label + " (" + target + ")"
	case hasLabel:
		return label
	default:
		return target
	}
}

// ToMrkdwn escapes plain text for chat.postMessage.
func ToMrkdwn(value string) string {
	return escaper.Replace(value)
}
