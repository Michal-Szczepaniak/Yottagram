package session

import (
	"context"
	"crypto/sha1"
	"encoding/hex"
	"errors"
	"net/url"
	"os"
	"path"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"yottaslack/internal/api"
	"yottaslack/internal/text"
)

// Files the app knows by a small id, like TDLib's, and asks to download.
type wireFile struct {
	ID     int32  `json:"id"`
	Size   int64  `json:"size"`
	Path   string `json:"path"`
	Active bool   `json:"active"`
	Done   bool   `json:"done"`

	Downloaded int64 `json:"downloaded"`
}

type wirePhoto struct {
	Small wireFile `json:"small"`
	Big   wireFile `json:"big"`
}

type media struct {
	Kind        string    `json:"kind"`
	Name        string    `json:"name,omitempty"`
	Mime        string    `json:"mime,omitempty"`
	File        *wireFile `json:"file,omitempty"`
	Width       int64     `json:"width,omitempty"`
	Height      int64     `json:"height,omitempty"`
	Thumb       *wireFile `json:"thumb,omitempty"`
	ThumbWidth  int64     `json:"thumb_width,omitempty"`
	ThumbHeight int64     `json:"thumb_height,omitempty"`
	Duration    int64     `json:"duration,omitempty"`
}

type linkPreview struct {
	URL    string    `json:"url"`
	Site   string    `json:"site"`
	Title  string    `json:"title"`
	Text   string    `json:"text"`
	Image  *wireFile `json:"image,omitempty"`
	Width  int64     `json:"width,omitempty"`
	Height int64     `json:"height,omitempty"`
}

type fileEntry struct {
	wireFile
	url    string
	auth   bool
	cancel context.CancelFunc
}

type rendered struct {
	Text     string
	Entities []text.Entity
	Media    *media
	Preview  *linkPreview
}

var huddleLength = regexp.MustCompile(`in the huddle for (?:(\d+)h)? ?(?:(\d+)m)? ?(?:(\d+)s)?`)

// fileFor returns the file for a URL. auth is for Slack's own files, which
// need the token, while avatars and link images are public.
func (s *Session) fileFor(address, name string, size int64, auth bool) wireFile {
	s.mu.Lock()
	defer s.mu.Unlock()

	if entry := s.files[address]; entry != nil {
		return entry.wireFile
	}

	sum := sha1.Sum([]byte(address))
	local := filepath.Join(s.filesDir, hex.EncodeToString(sum[:])+fileExtension(address, name))

	s.nextFile++
	entry := &fileEntry{wireFile: wireFile{ID: s.nextFile, Size: size, Path: local}, url: address, auth: auth}
	if info, err := os.Stat(local); err == nil {
		entry.Size = info.Size()
		entry.Done = true
	}

	s.files[address] = entry
	s.filesByID[entry.ID] = entry
	return entry.wireFile
}

func fileExtension(address, name string) string {
	if extension := path.Ext(name); extension != "" {
		return extension
	}
	if parsed, err := url.Parse(address); err == nil {
		return path.Ext(parsed.Path)
	}
	return ""
}

// download fetches a file in the background, the app follows it through file
// events.
func (s *Session) download(id uint64, fileID int32) {
	s.mu.Lock()
	entry := s.filesByID[fileID]
	if entry == nil {
		s.mu.Unlock()
		s.replyError(id, codeBadRequest, "unknown file")
		return
	}

	start := !entry.Done && !entry.Active
	if start {
		entry.Active = true
	}
	state := entry.wireFile
	s.mu.Unlock()

	s.emit(map[string]any{"event": "file", "file": state})
	s.replyOK(id)

	if start {
		s.spawn(func() { s.fetchFile(entry) })
	}
}

func (s *Session) fetchFile(entry *fileEntry) {
	ctx, cancel := context.WithCancel(s.ctx)
	defer cancel()

	s.mu.Lock()
	entry.cancel = cancel
	s.mu.Unlock()

	var size int64
	var err error

	select {
	case s.downloads <- struct{}{}:
		size, err = s.api.Download(ctx, entry.url, entry.auth, entry.Path, func(done, total int64) {
			s.fileProgress(entry, done, total)
		})
		<-s.downloads
	case <-ctx.Done():
		err = ctx.Err()
	}

	s.mu.Lock()
	entry.Active = false
	entry.cancel = nil
	entry.Downloaded = 0
	if err == nil {
		entry.Size = size
		entry.Done = true
	}
	state := entry.wireFile
	s.mu.Unlock()

	if err != nil && !errors.Is(err, context.Canceled) {
		s.log("download failed: %v", err)
	}
	s.emit(map[string]any{"event": "file", "file": state})
}

func (s *Session) fileProgress(entry *fileEntry, done, total int64) {
	s.mu.Lock()
	entry.Downloaded = done
	if total > 0 {
		entry.Size = total
	}
	state := entry.wireFile
	s.mu.Unlock()

	s.emit(map[string]any{"event": "file", "file": state})
}

func (s *Session) cancelDownload(id uint64, fileID int32) {
	s.mu.Lock()
	if entry := s.filesByID[fileID]; entry != nil && entry.cancel != nil {
		entry.cancel()
	}
	s.mu.Unlock()

	s.replyOK(id)
}

func (s *Session) avatarPhoto(u *user) *wirePhoto {
	big := u.AvatarBig
	if big == "" {
		big = u.Avatar
	}
	return &wirePhoto{
		Small: s.fileFor(u.Avatar, "", 0, false),
		Big:   s.fileFor(big, "", 0, false),
	}
}

// announceAvatar gives a user their photo the first time they're on screen. The
// app downloads every avatar it's told about, which is too many for all of a
// workspace's users.
func (s *Session) announceAvatar(u *user) {
	s.mu.Lock()
	if u.avatarSent || u.Avatar == "" {
		s.mu.Unlock()
		return
	}
	u.avatarSent = true
	announced := *u
	s.mu.Unlock()

	announced.Photo = s.avatarPhoto(&announced)
	s.emit(map[string]any{"event": "users", "users": []*user{&announced}})
}

func (s *Session) fileMedia(file api.File) *media {
	name := file.Name
	if name == "" {
		name = file.Title
	}

	thumb := file.Thumb360
	big := file.Thumb720
	if big == "" {
		big = file.URLPrivate
	}

	if strings.HasPrefix(file.Mimetype, "image/") && thumb != "" && file.Thumb360W > 0 && file.Thumb360H > 0 {
		small := s.fileFor(thumb, name, 0, true)
		photo := s.fileFor(big, name, int64(file.Size), true)
		width, height := int64(file.OriginalW), int64(file.OriginalH)
		if width == 0 || height == 0 {
			width, height = int64(file.Thumb360W), int64(file.Thumb360H)
		}
		return &media{
			Kind: "photo", File: &photo, Width: width, Height: height,
			Thumb: &small, ThumbWidth: int64(file.Thumb360W), ThumbHeight: int64(file.Thumb360H),
		}
	}

	document := s.fileFor(file.URLPrivate, name, int64(file.Size), true)
	return &media{Kind: "document", Name: name, Mime: file.Mimetype, File: &document}
}

func (s *Session) linkPreview(m api.Message) *linkPreview {
	for _, attachment := range m.Attachments {
		address := attachment.FromURL
		if address == "" {
			address = attachment.OriginalURL
		}
		if address == "" || attachment.Title == "" && attachment.Text == "" {
			continue
		}

		preview := &linkPreview{
			URL:   address,
			Site:  attachment.ServiceName,
			Title: attachment.Title,
			Text:  text.FromMrkdwn(attachment.Text, s.userName),
		}

		if attachment.ImageURL != "" && attachment.ImageWidth > 0 && attachment.ImageHeight > 0 {
			image := s.fileFor(attachment.ImageURL, "", int64(attachment.ImageBytes), false)
			preview.Image = &image
			preview.Width = int64(attachment.ImageWidth)
			preview.Height = int64(attachment.ImageHeight)
		}
		return preview
	}
	return nil
}

// huddleDuration returns how long a finished huddle lasted, in seconds.
func huddleDuration(m api.Message) int64 {
	if m.Room != nil && m.Room.DateEnd > m.Room.DateStart && m.Room.DateStart > 0 {
		return int64(m.Room.DateEnd - m.Room.DateStart)
	}

	match := huddleLength.FindStringSubmatch(m.Text)
	if match == nil {
		return 0
	}
	hours, _ := strconv.ParseInt(match[1], 10, 64)
	minutes, _ := strconv.ParseInt(match[2], 10, 64)
	seconds, _ := strconv.ParseInt(match[3], 10, 64)
	return hours*3600 + minutes*60 + seconds
}

// render turns a Slack message into what the app shows. The app has one piece
// of media per message, so further files are listed in the text.
func (s *Session) render(m api.Message) rendered {
	if duration := huddleDuration(m); duration > 0 {
		return rendered{Media: &media{Kind: "call", Duration: duration}}
	}

	var r rendered
	body, entities := text.FormatWith(m.Text, s.userName, s.customEmojiID)

	var extras []string
	for _, file := range m.Files {
		if r.Media == nil && file.URLPrivate != "" {
			r.Media = s.fileMedia(file)
			continue
		}

		name := file.Title
		if name == "" {
			name = file.Name
		}
		extras = append(extras, "[file: "+name+"]")
	}

	r.Preview = s.linkPreview(m)

	if body == "" && len(extras) == 0 && r.Media == nil && r.Preview == nil {
		for _, attachment := range m.Attachments {
			if attachment.Text != "" {
				extras = append(extras, text.FromMrkdwn(attachment.Text, s.userName))
			} else if attachment.Fallback != "" {
				extras = append(extras, attachment.Fallback)
			}
		}
		if len(extras) == 0 {
			extras = append(extras, "[unsupported message]")
		}
	}

	parts := extras
	if body != "" {
		parts = append([]string{body}, extras...)
	}
	r.Text, r.Entities = text.Trim(strings.Join(parts, "\n"), entities)
	return r
}
