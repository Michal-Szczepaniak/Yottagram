# yottaslack

Slack backend for Yottagram, written in Go and loaded by `src/slack/slackbackend.cpp` with `QLibrary`.

Run `yottagram --slack`, or use the Yottagram Slack launcher, to start in Slack mode. It has its own settings and D-Bus name, so it can run next to the Telegram instance.

## Setup

1. Create a Slack app from `slack-app-manifest.yml` and install it to your workspace.
2. Build the app as usual. `yottaslack.pri` runs `build.sh` before linking and installs the library to `/usr/share/yottagram/lib`. The build target needs one of:

   - `go` installed in it, which compiles with the target's `gcc`. Go runs under `qemu` there, so `build.sh` turns its garbage collector off and uses one thread, otherwise it crashes. The first build takes a while.
   - `prebuilds/<QT_ARCH>/libyottaslack.so`, tracked by Git LFS and used when `go` isn't found. Fill it from the host with cross compilers built against glibc 2.30 or older:

     ```sh
     CC_ARM=... CC_ARM64=... CC_I386=... ./build.sh
     ```

   With neither, the build stops with an error.
3. Start it in Slack mode and paste the User OAuth Token (`xoxp-...`). It's stored in plaintext in `QSettings` under `slackToken`.

## Testing

```sh
go vet ./...
go test -race ./...
```

Try it against a real workspace without the app:

```sh
SLACK_TOKEN=xoxp-... go run ./cmd/yscli
```

Each line of stdin is sent as a request, and every event is printed.

## Layout

| Path | Job |
|---|---|
| `main.go` | The `//export` C ABI. The only package importing `C` |
| `internal/api` | Slack Web API client, pagination, 429 handling |
| `internal/ids` | `ts` to message id, persisted Slack id to integer tables |
| `internal/text` | mrkdwn to and from plain text |
| `internal/session` | Requests, startup sequence, history, send, polling |
| `cmd/yscli` | Terminal driver |

## Protocol

`ys_send` takes one JSON request, and `ys_receive` returns one JSON reply or event at a time.

```json
{"id":7,"method":"history","params":{"chat":1099511627776,"from":0,"offset":0,"limit":40}}
{"id":7,"result":{"type":"messages","messages":[]}}
{"id":7,"error":{"code":500,"message":"..."}}
{"event":"new_message","message":{}}
```

Methods: `login`, `me`, `history`, `send`, `open`, `close`.

Events: `auth_ready`, `users`, `chats`, `ready`, `new_message`, `last_message`, `send_succeeded`, `message_content`, `delete_messages`, `member_count`, `log`.

The order of events matters; see the comment at the top of `internal/session/session.go`.
