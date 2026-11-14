package text

import (
	"sort"
	"strings"
)

// The tables are in emoji_data.go, generated from Slack's own emoji names.
// Custom emoji have no Unicode form, so they're handled apart.

// Emoji returns the character for an emoji name, ignoring any skin tone.
func Emoji(name string) (string, bool) {
	base, _, _ := strings.Cut(name, "::")
	char, ok := emojiNames[base]
	return char, ok
}

// EmojiName returns Slack's name for a character, or "" for an unknown one.
func EmojiName(char string) string {
	return emojiByChar[NormalizeEmoji(char)]
}

// NormalizeEmoji drops the variation selector, which only some sources include.
func NormalizeEmoji(char string) string {
	return strings.ReplaceAll(char, "️", "")
}

// Emojis returns every known character, sorted.
func Emojis() []string {
	chars := make([]string, 0, len(emojiByChar))
	for char := range emojiByChar {
		chars = append(chars, char)
	}
	sort.Strings(chars)
	return chars
}
