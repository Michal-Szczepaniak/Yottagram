# Slack features to implement

## Simple

- [x] Member count in the chat header
- [x] Chat info page: topic and purpose
- [ ] Replies: the input's reply is dropped today, decide between a thread reply and a quote
- [ ] Failed send: put the text back in the input box
- [x] Pause polling while offline
- [x] Retry `users.conversations` instead of showing an empty list at once
- [ ] Translations for the new strings

## Core

- [x] Realtime updates over Socket Mode
- [x] Unread counts and marking as read (`conversations.mark`)
- [ ] Reads made in other clients, Slack only reports them at the next launch
- [x] Notifications for direct messages and mentions
- [ ] Respect muted channels and Do Not Disturb
- [x] Chat list previews before a chat is opened
- [ ] Recent-first order for direct messages

## Messages

- [x] Formatting: bold, italic, strikethrough, code and links, in both directions
- [x] Sending mentions as real Slack mentions
- [x] Showing edits and deletes from Slack
- [x] Editing and deleting from the app
- [x] Reactions, shown and added, with the common emoji
- [x] `:emoji:` names in messages, with the common emoji
- [x] Custom workspace emoji, shown in text and reactions
- [ ] Picking custom emoji in the app
- [ ] Threads
- [x] Avatars
- [x] Link previews
- [x] File layer: downloads reported to the app as TDLib files
- [x] Huddles shown as call messages
- [x] Folders for channels, direct messages and apps
- [ ] Evicting old files from the download cache
- [x] Receiving files and images
- [x] Uploading files
- [ ] Upload progress, and hiding the media panel buttons Slack has no use for

## Before a public release

- [ ] Secure token storage instead of plaintext `QSettings`
- [ ] OAuth instead of pasting a token
- [ ] Multiple workspaces
- [ ] Libraries for `armv7hl` and `i486`
- [ ] `golang` in `BuildRequires`
- [x] A separate icon for the Slack launcher
- [ ] Measure idle RAM, CPU and battery use
