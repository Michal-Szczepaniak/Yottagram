# Slack features to implement

## Simple

- [x] Member count in the chat header
- [ ] Chat info page: real data behind `SupergroupInfo.qml` (topic, purpose)
- [ ] Replies: the input's reply is dropped today, decide between a thread reply and a quote
- [ ] Failed send: put the text back in the input box
- [ ] Pause polling while offline
- [ ] Show an error when `users.conversations` fails, instead of an empty list
- [ ] Translations for the new strings

## Core

- [x] Realtime updates over Socket Mode
- [ ] Unread counts and marking as read (`conversations.mark`)
- [ ] Notifications, which have to be synthesized because they only come from `updateNotificationGroup`
- [x] Chat list previews before a chat is opened
- [ ] Recent-first order for direct messages

## Messages

- [ ] Formatting: bold, italic, code, links and mentions as text entities, in both directions
- [x] Showing edits and deletes from Slack
- [x] Editing and deleting from the app
- [ ] Reactions
- [ ] Threads
- [ ] Avatars
- [ ] Receiving files and images
- [ ] Uploading files, which also brings back the media panel

## Before a public release

- [ ] Secure token storage instead of plaintext `QSettings`
- [ ] OAuth instead of pasting a token
- [ ] Multiple workspaces
- [ ] Libraries for `armv7hl` and `i486`
- [ ] `golang` in `BuildRequires`
- [ ] A separate icon for the Slack launcher
- [ ] Measure idle RAM, CPU and battery use
