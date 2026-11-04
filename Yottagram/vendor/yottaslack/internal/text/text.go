// Package text converts between Slack's mrkdwn and plain text.
package text

import (
	"regexp"
	"slices"
	"sort"
	"strings"
	"unicode"
	"unicode/utf16"
	"unicode/utf8"
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

// Entity is a formatted range of text, in UTF-16 code units like Qt and TDLib.
type Entity struct {
	Offset int    `json:"offset"`
	Length int    `json:"length"`
	Type   string `json:"type"`
	URL    string `json:"url,omitempty"`
	Emoji  int64  `json:"emoji,omitempty"`
}

type formatter struct {
	out      strings.Builder
	length   int
	entities []Entity
	userName func(id string) string
	custom   func(name string) int64
}

// Format renders Slack's message markup as text with entities, ordered so that
// an entity comes before the ones nested in it.
func Format(value string, userName func(id string) string) (string, []Entity) {
	return FormatWith(value, userName, nil)
}

// FormatWith also marks custom emoji. custom returns an emoji's id, or 0. The
// :name: stays in the text, for showing until its picture has loaded.
func FormatWith(value string, userName func(id string) string, custom func(name string) int64) (string, []Entity) {
	f := &formatter{userName: userName, custom: custom}
	f.parse(value)

	// Inner entities finish first, this puts them after outer ones of the same size
	slices.Reverse(f.entities)
	sort.SliceStable(f.entities, func(a, b int) bool {
		if f.entities[a].Offset != f.entities[b].Offset {
			return f.entities[a].Offset < f.entities[b].Offset
		}
		return f.entities[a].Length > f.entities[b].Length
	})
	return f.out.String(), f.entities
}

func (f *formatter) write(value string) {
	f.out.WriteString(value)
	f.length += utf16Length(value)
}

func (f *formatter) mark(start int, kind, url string) {
	if f.length > start {
		f.entities = append(f.entities, Entity{Offset: start, Length: f.length - start, Type: kind, URL: url})
	}
}

func (f *formatter) parse(value string) {
	for i := 0; i < len(value); {
		switch value[i] {
		case '<':
			if end := strings.IndexByte(value[i:], '>'); end > 0 {
				f.token(value[i+1 : i+end])
				i += end + 1
				continue
			}
		case '&':
			if decoded, size := decodeEntity(value[i:]); size > 0 {
				f.write(decoded)
				i += size
				continue
			}
		case '`':
			if strings.HasPrefix(value[i:], "```") {
				if end := strings.Index(value[i+3:], "```"); end > 0 {
					f.literal(value[i+3:i+3+end], "pre")
					i += end + 6
					continue
				}
			} else if end := strings.IndexByte(value[i+1:], '`'); end > 0 && !strings.Contains(value[i+1:i+1+end], "\n") {
				f.literal(value[i+1:i+1+end], "code")
				i += end + 2
				continue
			}
		case ':':
			if char, size := emojiAt(value[i:]); size > 0 {
				f.write(char)
				i += size
				continue
			}
			if size := f.customEmoji(value[i:]); size > 0 {
				i += size
				continue
			}
		case '*', '_', '~':
			if end := closingMarker(value, i); end > 0 {
				start := f.length
				f.parse(value[i+1 : end])
				f.mark(start, markerType(value[i]), "")
				i = end + 1
				continue
			}
		}

		_, size := utf8.DecodeRuneInString(value[i:])
		f.write(value[i : i+size])
		i += size
	}
}

func (f *formatter) literal(value, kind string) {
	start := f.length
	f.write(FromMrkdwn(value, f.userName))
	f.mark(start, kind, "")
}

func (f *formatter) token(body string) {
	target, label, hasLabel := strings.Cut(body, "|")
	start := f.length

	switch {
	case strings.HasPrefix(target, "@"):
		f.write(renderToken(body, f.userName))
		f.mark(start, "bold", "")
	case strings.HasPrefix(target, "#"), strings.HasPrefix(target, "!"):
		f.write(renderToken(body, f.userName))
	default:
		address := unescaper.Replace(target)
		if hasLabel && label != target {
			f.write(unescaper.Replace(label))
			f.mark(start, "text_url", address)
		} else {
			f.write(address)
			f.mark(start, "url", "")
		}
	}
}

func (f *formatter) customEmoji(value string) int {
	end := strings.IndexByte(value[1:], ':')
	if f.custom == nil || end <= 0 {
		return 0
	}

	id := f.custom(value[1 : end+1])
	if id == 0 {
		return 0
	}

	start := f.length
	f.write(value[:end+2])
	f.entities = append(f.entities, Entity{Offset: start, Length: f.length - start, Type: "custom_emoji", Emoji: id})
	return end + 2
}

// emojiAt reads a :name: from the start of value, with its skin tone if any.
func emojiAt(value string) (string, int) {
	end := strings.IndexByte(value[1:], ':')
	if end <= 0 {
		return "", 0
	}

	char, ok := Emoji(value[1 : end+1])
	if !ok {
		return "", 0
	}

	size := end + 2
	if rest := value[size:]; strings.HasPrefix(rest, ":skin-tone-") {
		if close := strings.IndexByte(rest[1:], ':'); close > 0 {
			size += close + 2
		}
	}
	return char, size
}

func markerType(marker byte) string {
	switch marker {
	case '*':
		return "bold"
	case '_':
		return "italic"
	default:
		return "strikethrough"
	}
}

// closingMarker finds where the formatting opened at value[i] ends, or -1.
// Markers only count at word edges, so snake_case_names stay as they are.
func closingMarker(value string, i int) int {
	if i > 0 && isWord(value[i-1]) {
		return -1
	}
	if i+1 >= len(value) || value[i+1] == ' ' {
		return -1
	}

	for j := i + 2; j < len(value); j++ {
		if value[j] == '\n' {
			return -1
		}
		if value[j] == value[i] && value[j-1] != ' ' && (j+1 == len(value) || !isWord(value[j+1])) {
			return j
		}
	}
	return -1
}

func isWord(c byte) bool {
	return c >= 0x80 || c >= '0' && c <= '9' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z'
}

func decodeEntity(value string) (string, int) {
	for _, entity := range [][2]string{{"&amp;", "&"}, {"&lt;", "<"}, {"&gt;", ">"}} {
		if strings.HasPrefix(value, entity[0]) {
			return entity[1], len(entity[0])
		}
	}
	return "", 0
}

func utf16Length(value string) int {
	return len(utf16.Encode([]rune(value)))
}

// Trim removes surrounding whitespace, keeping the entities on the same text.
func Trim(value string, entities []Entity) (string, []Entity) {
	left := strings.TrimLeftFunc(value, unicode.IsSpace)
	shift := utf16Length(value[:len(value)-len(left)])
	trimmed := strings.TrimRightFunc(left, unicode.IsSpace)
	length := utf16Length(trimmed)

	kept := make([]Entity, 0, len(entities))
	for _, entity := range entities {
		start := entity.Offset - shift
		end := start + entity.Length
		if start < 0 {
			start = 0
		}
		if end > length {
			end = length
		}
		if end > start {
			entity.Offset = start
			entity.Length = end - start
			kept = append(kept, entity)
		}
	}
	return trimmed, kept
}
