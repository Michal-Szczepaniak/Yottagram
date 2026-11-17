// Package api is a small client for the Slack Web API.
package api

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"net/http/httptrace"
	"net/url"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"time"
)

const (
	defaultBaseURL    = "https://slack.com/api"
	requestTimeout    = 30 * time.Second
	maxRateRetries    = 5
	defaultRetryAfter = 5 * time.Second
	slowCallThreshold = time.Second
	progressInterval  = 300 * time.Millisecond
	pageSize          = 200
)

// Error is a failure reported by Slack, as opposed to a network failure.
type Error struct {
	Method string
	Code   string
}

func (e *Error) Error() string { return e.Method + ": " + e.Code }

// IsAuthError reports whether err means the token itself is unusable.
func IsAuthError(err error) bool {
	var apiError *Error
	if !errors.As(err, &apiError) {
		return false
	}
	switch apiError.Code {
	case "invalid_auth", "not_authed", "token_revoked", "token_expired", "account_inactive", "not_allowed_token_type":
		return true
	}
	return false
}

// IsCode reports whether Slack answered with this error.
func IsCode(err error, code string) bool {
	var apiError *Error
	return errors.As(err, &apiError) && apiError.Code == code
}

// IsAPIError reports whether Slack answered, even if with an error.
func IsAPIError(err error) bool {
	var apiError *Error
	return errors.As(err, &apiError)
}

// timeline records when the stages of a request finished. Connection attempts
// can run in parallel, hence the lock.
type timeline struct {
	started time.Time

	mu     sync.Mutex
	stages []string
}

func (t *timeline) mark(stage string) {
	t.mu.Lock()
	defer t.mu.Unlock()
	t.stages = append(t.stages, fmt.Sprintf("%s=%dms", stage, time.Since(t.started).Milliseconds()))
}

func (t *timeline) String() string {
	t.mu.Lock()
	defer t.mu.Unlock()
	return strings.Join(t.stages, " ")
}

func (t *timeline) trace() *httptrace.ClientTrace {
	return &httptrace.ClientTrace{
		GotConn: func(info httptrace.GotConnInfo) {
			if info.Reused {
				t.mark("reused")
			} else {
				t.mark("connected")
			}
		},
		DNSDone: func(httptrace.DNSDoneInfo) { t.mark("dns") },
		ConnectDone: func(network, addr string, err error) {
			if err != nil {
				t.mark("failed " + addr)
			} else {
				t.mark("tcp " + addr)
			}
		},
		TLSHandshakeDone:     func(tls.ConnectionState, error) { t.mark("tls") },
		WroteRequest:         func(httptrace.WroteRequestInfo) { t.mark("sent") },
		GotFirstResponseByte: func() { t.mark("response") },
	}
}

// Client calls Slack. A 429 pauses the method that hit it, as Slack's limits are
// per method. Everything else keeps working, so previews can't delay an edit.
type Client struct {
	BaseURL string
	HTTP    *http.Client

	// RateLimited, if set, is called when a method gets paused
	RateLimited func(method string, wait time.Duration)

	// SlowCall, if set, is called when a request takes unusually long, with
	// when each stage of it finished
	SlowCall func(method string, took time.Duration, stages string)

	mu          sync.Mutex
	token       string
	cookie      string
	pausedUntil map[string]time.Time
}

// New returns a client for the production API.
func New() *Client {
	return &Client{
		BaseURL:     defaultBaseURL,
		HTTP:        &http.Client{Timeout: requestTimeout},
		pausedUntil: map[string]time.Time{},
	}
}

// SetSessionCookie switches to a browser session: an xoxc- token, which only
// works together with the d cookie. Browsers show it URL encoded, which is the
// form Slack wants, so anything else gets encoded here.
func (c *Client) SetSessionCookie(value string) {
	value = strings.TrimPrefix(strings.TrimSpace(value), "d=")
	if !strings.Contains(value, "%") {
		value = url.QueryEscape(value)
	}

	c.mu.Lock()
	defer c.mu.Unlock()
	c.cookie = "d=" + value
}

// Cookie returns the session cookie header, if there is one.
func (c *Client) Cookie() string {
	c.mu.Lock()
	defer c.mu.Unlock()
	return c.cookie
}

// SetToken sets the token used for every call.
func (c *Client) SetToken(token string) {
	c.mu.Lock()
	defer c.mu.Unlock()
	c.token = token
}

func (c *Client) waitForPause(ctx context.Context, method string) error {
	c.mu.Lock()
	wait := time.Until(c.pausedUntil[method])
	c.mu.Unlock()

	if wait <= 0 {
		return nil
	}

	timer := time.NewTimer(wait)
	defer timer.Stop()

	select {
	case <-timer.C:
		return nil
	case <-ctx.Done():
		return ctx.Err()
	}
}

func (c *Client) pause(method string, duration time.Duration) {
	c.mu.Lock()
	if until := time.Now().Add(duration); until.After(c.pausedUntil[method]) {
		c.pausedUntil[method] = until
	}
	c.mu.Unlock()

	if c.RateLimited != nil {
		c.RateLimited(method, duration)
	}
}

func (c *Client) post(ctx context.Context, method string, params url.Values) ([]byte, time.Duration, error) {
	c.mu.Lock()
	token := c.token
	cookie := c.cookie
	c.mu.Unlock()

	form := params.Encode()
	if cookie != "" {
		// Session tokens go in the form, like Slack's own web client sends them
		withToken := url.Values{"token": {token}}
		for key, values := range params {
			withToken[key] = values
		}
		form = withToken.Encode()
	}

	request, err := http.NewRequestWithContext(ctx, http.MethodPost, c.BaseURL+"/"+method, strings.NewReader(form))
	if err != nil {
		return nil, 0, err
	}

	request.Header.Set("Content-Type", "application/x-www-form-urlencoded")
	if cookie != "" {
		request.Header.Set("Cookie", cookie)
	} else {
		request.Header.Set("Authorization", "Bearer "+token)
	}

	response, err := c.HTTP.Do(request)
	if err != nil {
		return nil, 0, err
	}
	defer response.Body.Close()

	if response.StatusCode == http.StatusTooManyRequests {
		retryAfter := defaultRetryAfter
		if seconds, err := strconv.Atoi(response.Header.Get("Retry-After")); err == nil && seconds > 0 {
			retryAfter = time.Duration(seconds) * time.Second
		}
		return nil, retryAfter, nil
	}

	body, err := io.ReadAll(response.Body)
	if err != nil {
		return nil, 0, err
	}
	return body, 0, nil
}

// Call invokes method and decodes the response into out, which may be nil.
func (c *Client) Call(ctx context.Context, method string, params url.Values, out any) error {
	if params == nil {
		params = url.Values{}
	}

	for attempt := 0; ; attempt++ {
		if err := c.waitForPause(ctx, method); err != nil {
			return err
		}

		started := time.Now()
		stages := &timeline{started: started}
		body, retryAfter, err := c.post(httptrace.WithClientTrace(ctx, stages.trace()), method, params)
		if took := time.Since(started); took > slowCallThreshold && c.SlowCall != nil {
			c.SlowCall(method, took, stages.String())
		}
		if err != nil {
			return err
		}

		if retryAfter > 0 {
			if attempt >= maxRateRetries {
				return &Error{Method: method, Code: "ratelimited"}
			}
			c.pause(method, retryAfter)
			continue
		}

		var envelope struct {
			OK    bool   `json:"ok"`
			Error string `json:"error"`
		}
		if err := json.Unmarshal(body, &envelope); err != nil {
			return fmt.Errorf("%s: invalid response: %w", method, err)
		}
		if !envelope.OK {
			return &Error{Method: method, Code: envelope.Error}
		}
		if out == nil {
			return nil
		}
		return json.Unmarshal(body, out)
	}
}

// Profile is the part of a Slack user profile the app shows.
type Profile struct {
	DisplayName string `json:"display_name"`
	RealName    string `json:"real_name"`
	Image72     string `json:"image_72"`
	Image192    string `json:"image_192"`

	StatusEmoji      string `json:"status_emoji"`
	StatusExpiration Number `json:"status_expiration"`
}

// Number accepts both forms Slack uses for some numbers, and treats anything
// else as 0, so one odd field can't lose a whole page of messages.
type Number int64

// UnmarshalJSON implements json.Unmarshaler.
func (n *Number) UnmarshalJSON(data []byte) error {
	value, err := strconv.ParseFloat(strings.Trim(string(data), `"`), 64)
	if err != nil {
		*n = 0
		return nil
	}
	*n = Number(value)
	return nil
}

// Described is a conversation's topic or purpose.
type Described struct {
	Value string `json:"value"`
}

// User is a Slack user.
type User struct {
	ID      string  `json:"id"`
	Name    string  `json:"name"`
	Deleted bool    `json:"deleted"`
	IsBot   bool    `json:"is_bot"`
	IsApp   bool    `json:"is_app_user"`
	Profile Profile `json:"profile"`
}

// Conversation is a channel, private channel, group DM or DM.
type Conversation struct {
	ID         string `json:"id"`
	Name       string `json:"name"`
	IsIM       bool   `json:"is_im"`
	IsMpim     bool   `json:"is_mpim"`
	User       string `json:"user"`
	NumMembers int    `json:"num_members"`
	LastRead   string `json:"last_read"`

	Topic   Described `json:"topic"`
	Purpose Described `json:"purpose"`
}

// File is a message attachment uploaded to Slack.
type File struct {
	Name       string `json:"name"`
	Title      string `json:"title"`
	Mimetype   string `json:"mimetype"`
	Size       Number `json:"size"`
	URLPrivate string `json:"url_private"`
	Thumb360   string `json:"thumb_360"`
	Thumb720   string `json:"thumb_720"`
	Thumb360W  Number `json:"thumb_360_w"`
	Thumb360H  Number `json:"thumb_360_h"`
	OriginalW  Number `json:"original_w"`
	OriginalH  Number `json:"original_h"`
}

// Room is the call behind a huddle message.
type Room struct {
	DateStart Number `json:"date_start"`
	DateEnd   Number `json:"date_end"`
}

// Attachment is a legacy message attachment.
type Attachment struct {
	Fallback    string `json:"fallback"`
	Text        string `json:"text"`
	Title       string `json:"title"`
	FromURL     string `json:"from_url"`
	OriginalURL string `json:"original_url"`
	ServiceName string `json:"service_name"`
	ImageURL    string `json:"image_url"`
	ImageWidth  Number `json:"image_width"`
	ImageHeight Number `json:"image_height"`
	ImageBytes  Number `json:"image_bytes"`
}

// BotProfile names the integration that posted a message.
type BotProfile struct {
	Name  string `json:"name"`
	Icons *Icons `json:"icons"`
}

// Icons are the images a bot posted with, its own or one set per message.
type Icons struct {
	Image36  string `json:"image_36"`
	Image48  string `json:"image_48"`
	Image72  string `json:"image_72"`
	Image192 string `json:"image_192"`
	Image512 string `json:"image_512"`
}

// Small is the image for a list, Big the one for a profile.
func (i *Icons) Small() string {
	if i == nil {
		return ""
	}
	return firstOf(i.Image72, i.Image48, i.Image36, i.Image192, i.Image512)
}

func (i *Icons) Big() string {
	if i == nil {
		return ""
	}
	return firstOf(i.Image192, i.Image512, i.Image72, i.Image48, i.Image36)
}

func firstOf(values ...string) string {
	for _, value := range values {
		if value != "" {
			return value
		}
	}
	return ""
}

// Edited marks a message as changed after it was posted.
type Edited struct {
	TS string `json:"ts"`
}

// Reaction is one emoji on a message, Users may not list everyone.
type Reaction struct {
	Name  string   `json:"name"`
	Count Number   `json:"count"`
	Users []string `json:"users"`
}

// Message is a Slack message.
type Message struct {
	TS          string       `json:"ts"`
	ThreadTS    string       `json:"thread_ts"`
	Subtype     string       `json:"subtype"`
	Room        *Room        `json:"room"`

	ReplyCount   Number   `json:"reply_count"`
	ReplyUsers   []string `json:"reply_users"`
	LatestReply  string   `json:"latest_reply"`
	ParentUserID string   `json:"parent_user_id"`
	User        string       `json:"user"`
	BotID       string       `json:"bot_id"`
	Username    string       `json:"username"`
	Text        string       `json:"text"`
	Files       []File       `json:"files"`
	Attachments []Attachment `json:"attachments"`
	BotProfile  *BotProfile  `json:"bot_profile"`
	Icons       *Icons       `json:"icons"`
	Edited      *Edited      `json:"edited"`
	Reactions   []Reaction   `json:"reactions"`
}

type responseMetadata struct {
	NextCursor string `json:"next_cursor"`
}

// AuthTest returns the id of the token's user.
func (c *Client) AuthTest(ctx context.Context) (string, error) {
	var response struct {
		UserID string `json:"user_id"`
	}
	if err := c.Call(ctx, "auth.test", nil, &response); err != nil {
		return "", err
	}
	return response.UserID, nil
}

// UserInfo fetches one user.
func (c *Client) UserInfo(ctx context.Context, id string) (User, error) {
	var response struct {
		User User `json:"user"`
	}
	err := c.Call(ctx, "users.info", url.Values{"user": {id}}, &response)
	return response.User, err
}

// Users lists the whole workspace.
func (c *Client) Users(ctx context.Context) ([]User, error) {
	var users []User
	cursor := ""

	for {
		var response struct {
			Members  []User           `json:"members"`
			Metadata responseMetadata `json:"response_metadata"`
		}
		params := url.Values{"limit": {strconv.Itoa(pageSize)}}
		if cursor != "" {
			params.Set("cursor", cursor)
		}
		if err := c.Call(ctx, "users.list", params, &response); err != nil {
			return nil, err
		}

		users = append(users, response.Members...)
		cursor = response.Metadata.NextCursor
		if cursor == "" {
			return users, nil
		}
	}
}

// Conversations lists what the token's user is a member of.
func (c *Client) Conversations(ctx context.Context) ([]Conversation, error) {
	var conversations []Conversation
	cursor := ""

	for {
		var response struct {
			Channels []Conversation   `json:"channels"`
			Metadata responseMetadata `json:"response_metadata"`
		}
		params := url.Values{
			"types":            {"public_channel,private_channel,mpim,im"},
			"exclude_archived": {"true"},
			"limit":            {strconv.Itoa(pageSize)},
		}
		if cursor != "" {
			params.Set("cursor", cursor)
		}
		if err := c.Call(ctx, "users.conversations", params, &response); err != nil {
			return nil, err
		}

		conversations = append(conversations, response.Channels...)
		cursor = response.Metadata.NextCursor
		if cursor == "" {
			return conversations, nil
		}
	}
}

// HistoryParams selects a page of conversations.history.
type HistoryParams struct {
	Oldest    string
	Latest    string
	Inclusive bool
	Limit     int
	Cursor    string
}

// HistoryPage is one page of messages, in the order Slack returned them.
type HistoryPage struct {
	Messages   []Message
	NextCursor string
}

// History fetches one page of a conversation.
// SetPresence sets presence to "auto", where Slack decides from activity, or
// "away". Slack has no way to force being online.
func (c *Client) SetPresence(ctx context.Context, presence string) error {
	return c.Call(ctx, "users.setPresence", url.Values{"presence": {presence}}, nil)
}

// ConnectRTM returns a URL for the classic socket, which is what a browser
// session uses.
func (c *Client) ConnectRTM(ctx context.Context) (string, error) {
	var response struct {
		URL string `json:"url"`
	}
	err := c.Call(ctx, "rtm.connect", url.Values{}, &response)
	return response.URL, err
}

func (c *Client) ConversationInfo(ctx context.Context, channel string) (Conversation, error) {
	var response struct {
		Channel Conversation `json:"channel"`
	}
	params := url.Values{"channel": {channel}, "include_num_members": {"true"}}
	err := c.Call(ctx, "conversations.info", params, &response)
	return response.Channel, err
}

// ConversationCount is one chat's entry in client.counts.
type ConversationCount struct {
	ID         string `json:"id"`
	LastRead   string `json:"last_read"`
	Latest     string `json:"latest"`
	HasUnreads bool   `json:"has_unreads"`
}

// Counts returns every chat's latest message and read state in one call. It's
// what the web client uses, undocumented and only for browser sessions.
func (c *Client) Counts(ctx context.Context) ([]ConversationCount, error) {
	var response struct {
		Channels []ConversationCount `json:"channels"`
		MPIMs    []ConversationCount `json:"mpims"`
		IMs      []ConversationCount `json:"ims"`
	}
	err := c.Call(ctx, "client.counts", url.Values{}, &response)

	counts := append(response.Channels, response.MPIMs...)
	return append(counts, response.IMs...), err
}

func (c *Client) History(ctx context.Context, channel string, p HistoryParams) (HistoryPage, error) {
	params := url.Values{"channel": {channel}, "limit": {strconv.Itoa(p.Limit)}}
	if p.Oldest != "" {
		params.Set("oldest", p.Oldest)
	}
	if p.Latest != "" {
		params.Set("latest", p.Latest)
	}
	if p.Inclusive {
		params.Set("inclusive", "true")
	}
	if p.Cursor != "" {
		params.Set("cursor", p.Cursor)
	}

	var response struct {
		Messages []Message        `json:"messages"`
		Metadata responseMetadata `json:"response_metadata"`
	}
	if err := c.Call(ctx, "conversations.history", params, &response); err != nil {
		return HistoryPage{}, err
	}
	return HistoryPage{Messages: response.Messages, NextCursor: response.Metadata.NextCursor}, nil
}

// PostMessage sends text and returns the stored message.
// UpdateMessage edits a message. Slack only allows that for your own.
func (c *Client) UpdateMessage(ctx context.Context, channel, ts, text string) (Message, error) {
	var response struct {
		Text    string  `json:"text"`
		Message Message `json:"message"`
	}
	err := c.Call(ctx, "chat.update", url.Values{"channel": {channel}, "ts": {ts}, "text": {text}}, &response)
	if err != nil {
		return Message{}, err
	}
	if response.Message.Text == "" {
		response.Message.Text = response.Text
	}
	return response.Message, nil
}

// DeleteMessage deletes a message.
func (c *Client) DeleteMessage(ctx context.Context, channel, ts string) error {
	return c.Call(ctx, "chat.delete", url.Values{"channel": {channel}, "ts": {ts}}, nil)
}

// describeURL names where a URL points without its file name or signature.
func describeURL(address *url.URL) string {
	first, _, _ := strings.Cut(strings.TrimPrefix(address.Path, "/"), "/")
	return address.Host + "/" + first
}

// CustomEmoji returns the workspace's emoji as name to URL, or to alias:name.
func (c *Client) CustomEmoji(ctx context.Context) (map[string]string, error) {
	var response struct {
		Emoji map[string]string `json:"emoji"`
	}
	err := c.Call(ctx, "emoji.list", nil, &response)
	return response.Emoji, err
}

// AdminEmoji returns every custom emoji as Slack describes it, unparsed. It
// isn't documented and may be refused, so callers treat it as optional.
func (c *Client) AdminEmoji(ctx context.Context) ([]json.RawMessage, error) {
	const pageSize = 500
	const maxPages = 50

	var all []json.RawMessage
	for page := 1; page <= maxPages; page++ {
		var response struct {
			Emoji  []json.RawMessage `json:"emoji"`
			Paging struct {
				Pages Number `json:"pages"`
			} `json:"paging"`
		}
		params := url.Values{"count": {strconv.Itoa(pageSize)}, "page": {strconv.Itoa(page)}}
		if err := c.Call(ctx, "emoji.adminList", params, &response); err != nil {
			return nil, err
		}

		all = append(all, response.Emoji...)

		// Paging may be missing, a short page is the last either way
		if len(response.Emoji) < pageSize || (response.Paging.Pages > 0 && Number(page) >= response.Paging.Pages) {
			break
		}
	}
	return all, nil
}

// AddReaction adds an emoji to a message.
func (c *Client) AddReaction(ctx context.Context, channel, ts, name string) error {
	return c.Call(ctx, "reactions.add", url.Values{"channel": {channel}, "timestamp": {ts}, "name": {name}}, nil)
}

// RemoveReaction removes an emoji from a message.
func (c *Client) RemoveReaction(ctx context.Context, channel, ts, name string) error {
	return c.Call(ctx, "reactions.remove", url.Values{"channel": {channel}, "timestamp": {ts}, "name": {name}}, nil)
}

// Upload posts a file to a conversation, into a thread if threadTS isn't empty.
func (c *Client) Upload(ctx context.Context, channel, path, comment, threadTS string) error {
	file, err := os.Open(path)
	if err != nil {
		return err
	}
	defer file.Close()

	info, err := file.Stat()
	if err != nil {
		return err
	}

	name := filepath.Base(path)
	var target struct {
		UploadURL string `json:"upload_url"`
		FileID    string `json:"file_id"`
	}
	params := url.Values{"filename": {name}, "length": {strconv.FormatInt(info.Size(), 10)}}
	if err := c.Call(ctx, "files.getUploadURLExternal", params, &target); err != nil {
		return err
	}

	request, err := http.NewRequestWithContext(ctx, http.MethodPost, target.UploadURL, file)
	if err != nil {
		return err
	}
	request.ContentLength = info.Size()
	request.Header.Set("Content-Type", "application/octet-stream")

	response, err := (&http.Client{}).Do(request)
	if err != nil {
		return err
	}
	response.Body.Close()
	if response.StatusCode != http.StatusOK {
		return fmt.Errorf("upload: %s", response.Status)
	}

	files, err := json.Marshal([]map[string]string{{"id": target.FileID, "title": name}})
	if err != nil {
		return err
	}

	params = url.Values{"files": {string(files)}, "channel_id": {channel}}
	if comment != "" {
		params.Set("initial_comment", comment)
	}
	if threadTS != "" {
		params.Set("thread_ts", threadTS)
	}
	return c.Call(ctx, "files.completeUploadExternal", params, nil)
}

// Mark tells Slack the conversation is read up to ts.
func (c *Client) Mark(ctx context.Context, channel, ts string) error {
	return c.Call(ctx, "conversations.mark", url.Values{"channel": {channel}, "ts": {ts}}, nil)
}

// progressWriter counts what passes through it, reporting now and then.
type progressWriter struct {
	done, total int64
	reported    time.Time
	report      func(done, total int64)
}

func (w *progressWriter) Write(content []byte) (int, error) {
	w.done += int64(len(content))
	if time.Since(w.reported) >= progressInterval {
		w.reported = time.Now()
		w.report(w.done, w.total)
	}
	return len(content), nil
}

// Download saves a URL to path and returns its size, reporting progress on the
// way. auth sends the token, which Slack's own files need. Without the
// files:read scope Slack answers with its login page, so HTML is an error there.
func (c *Client) Download(ctx context.Context, address string, auth bool, path string, report func(done, total int64)) (int64, error) {
	request, err := http.NewRequestWithContext(ctx, http.MethodGet, address, nil)
	if err != nil {
		return 0, err
	}

	if auth {
		c.mu.Lock()
		token := c.token
		cookie := c.cookie
		c.mu.Unlock()

		request.Header.Set("Authorization", "Bearer "+token)
		if cookie != "" {
			request.Header.Set("Cookie", cookie)
		}
	}

	response, err := (&http.Client{}).Do(request)
	if err != nil {
		return 0, err
	}
	defer response.Body.Close()

	if response.StatusCode != http.StatusOK {
		return 0, fmt.Errorf("download: %s from %s, asked %s, auth %v", response.Status, describeURL(response.Request.URL), describeURL(request.URL), auth)
	}
	if auth && strings.HasPrefix(response.Header.Get("Content-Type"), "text/html") {
		return 0, errors.New("download: got a login page, is the files:read scope missing?")
	}

	temporary := path + ".part"
	file, err := os.Create(temporary)
	if err != nil {
		return 0, err
	}

	progress := &progressWriter{total: response.ContentLength, report: report}
	size, err := io.Copy(io.MultiWriter(file, progress), response.Body)
	if closeErr := file.Close(); err == nil {
		err = closeErr
	}
	if err == nil {
		err = os.Rename(temporary, path)
	}
	if err != nil {
		os.Remove(temporary)
		return 0, err
	}
	return size, nil
}

// Replies reads a thread, oldest first, starting with its parent.
func (c *Client) Replies(ctx context.Context, channel, ts string, p HistoryParams) (HistoryPage, error) {
	params := url.Values{"channel": {channel}, "ts": {ts}, "limit": {strconv.Itoa(p.Limit)}}
	if p.Oldest != "" {
		params.Set("oldest", p.Oldest)
	}
	if p.Latest != "" {
		params.Set("latest", p.Latest)
	}
	if p.Inclusive {
		params.Set("inclusive", "true")
	}
	if p.Cursor != "" {
		params.Set("cursor", p.Cursor)
	}

	var response struct {
		Messages []Message        `json:"messages"`
		Metadata responseMetadata `json:"response_metadata"`
	}
	if err := c.Call(ctx, "conversations.replies", params, &response); err != nil {
		return HistoryPage{}, err
	}
	return HistoryPage{Messages: response.Messages, NextCursor: response.Metadata.NextCursor}, nil
}

// Command runs a slash command such as "/remind" in a channel, like typing it
// in the Slack client. It returns the reply that only the sender sees, if any.
func (c *Client) Command(ctx context.Context, channel, command, text, threadTS string) (string, error) {
	var response struct {
		Response string `json:"response"`
	}
	params := url.Values{"channel": {channel}, "command": {command}, "disp": {command}, "text": {text}}
	if threadTS != "" {
		params.Set("thread_ts", threadTS)
	}
	if err := c.Call(ctx, "chat.command", params, &response); err != nil {
		return "", err
	}
	return response.Response, nil
}

// PostMessage sends a message, into a thread if threadTS isn't empty.
func (c *Client) PostMessage(ctx context.Context, channel, text, threadTS string) (Message, error) {
	var response struct {
		TS      string  `json:"ts"`
		Message Message `json:"message"`
	}
	params := url.Values{"channel": {channel}, "text": {text}}
	if threadTS != "" {
		params.Set("thread_ts", threadTS)
	}
	err := c.Call(ctx, "chat.postMessage", params, &response)
	if err != nil {
		return Message{}, err
	}
	if response.Message.TS == "" {
		response.Message.TS = response.TS
	}
	return response.Message, nil
}
