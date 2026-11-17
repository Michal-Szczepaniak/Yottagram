# yottaslack

Slack backend for Yottagram, written in Go and loaded by `src/slack/slackbackend.cpp` with `QLibrary`.

Slack support is packaged separately as `yottagram-slack`, which holds this library, the launcher and its D-Bus service. Run `yottagram --slack`, or use the Yottagram Slack launcher, to start in Slack mode. It has its own settings and D-Bus name, so it can run next to the Telegram instance.

## Setup

1. Build the app as usual. `yottaslack.pri` runs `build.sh` before linking and installs the library to `/usr/share/yottagram/lib`. The build target needs one of:

   - `go` installed in it, which compiles with the target's `gcc`. Go runs under `qemu` there, so `build.sh` turns its garbage collector off and uses one thread, otherwise it crashes. The first build takes a while.
   - `prebuilds/<QT_ARCH>/libyottaslack.so`, tracked by Git LFS and used when `go` isn't found. Fill it from the host with cross compilers built against glibc 2.30 or older:

     ```sh
     CC_ARM=... CC_ARM64=... CC_I386=... ./build.sh
     ```

   With neither, the build stops with an error.
2. Start it in Slack mode and log in with a browser session, the way Slack's web client does: an `xoxc-` token and the `d` cookie. Nothing needs creating or installing, and realtime uses the classic RTM socket. In a browser logged in to the workspace, open the developer tools:

   - Token: in the console, run `Object.values(JSON.parse(localStorage.localConfig_v2).teams).map(t => t.name + ": " + t.token)` and copy your workspace's.
   - Cookie: the value of `d` for `slack.com`, under Storage or Application. It starts with `xoxd-`.

   Both are stored in plaintext in `QSettings`, under `slackToken` and `slackCookie`.

This is undocumented and can break or get logged out at any time. The session can do anything you can.

## Testing

```sh
go vet ./...
go test -race ./...
```

## Layout

| Path | Job |
|---|---|
| `main.go` | The `//export` C ABI. The only package importing `C` |
| `internal/api` | Slack Web API client, pagination, 429 handling |
| `internal/ids` | `ts` to message id, persisted Slack id to integer tables |
| `internal/text` | mrkdwn to and from plain text |
| `internal/session` | Requests, startup sequence, history, send, polling, realtime events, files, unread state |
| `internal/socket` | Minimal WebSocket client, so the library has no dependencies |

## Protocol

`ys_send` takes one JSON request, and `ys_receive` returns one JSON reply or event at a time.

```json
{"id":7,"method":"history","params":{"chat":1099511627776,"from":0,"offset":0,"limit":40}}
{"id":7,"result":{"type":"messages","messages":[]}}
{"id":7,"error":{"code":500,"message":"..."}}
{"event":"new_message","message":{}}
```

Methods: `login`, `network`, `me`, `history`, `send`, `upload`, `react`, `available_reactions`, `custom_emojis`, `members`, `view`, `download`, `cancel_download`, `properties`, `edit`, `delete`, `open`, `close`.

Events: `auth_ready`, `users`, `chats`, `ready`, `new_message`, `last_message`, `send_succeeded`, `message_content`, `delete_messages`, `chat_title`, `member_count`, `reactions`, `read_inbox`, `notify`, `notify_clear`, `file`, `folders`, `chat_position`, `log`.

The order of events matters; see the comment at the top of `internal/session/session.go`.
