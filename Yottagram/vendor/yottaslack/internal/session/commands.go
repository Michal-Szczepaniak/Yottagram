package session

import (
	"regexp"
	"strings"
	"time"
)

// slashCommand matches "/name" at the start of a message, followed by the end
// or whitespace, so paths like "/home/me" stay plain text.
var slashCommand = regexp.MustCompile(`^(/[\p{L}\p{N}_.\-]+)(?:\s+([\s\S]*))?$`)

// parseCommand splits markup into a slash command and its arguments.
func parseCommand(markup string) (command, args string, ok bool) {
	match := slashCommand.FindStringSubmatch(markup)
	if match == nil {
		return "", "", false
	}
	return strings.ToLower(match[1]), strings.TrimSpace(match[2]), true
}

// runCommand runs a slash command instead of posting it as a message. Whatever
// the command posts arrives like any other message, its private reply is shown
// locally as coming from Slackbot.
func (s *Session) runCommand(id uint64, c *chat, command, args string) {
	response, err := s.api.Command(s.ctx, c.slackID, command, args, c.threadTS)
	if err != nil {
		s.replyError(id, codeFailed, err.Error())
		return
	}

	if response = strings.TrimSpace(response); response != "" {
		bot := s.ensureUser(slackbotID)
		s.announceAvatar(bot)

		s.mu.Lock()
		tempID := time.Now().UnixMicro()
		if tempID <= c.newest {
			tempID = c.newest + 1
		}
		if tempID <= s.tempID {
			tempID = s.tempID + 1
		}
		s.tempID = tempID
		s.mu.Unlock()

		s.emit(map[string]any{"event": "new_message", "message": message{
			ID:     tempID,
			Chat:   c.id,
			Sender: bot.ID,
			Date:   tempID / 1000000,
			Text:   response,
		}})
	}

	s.replyOK(id)
}
