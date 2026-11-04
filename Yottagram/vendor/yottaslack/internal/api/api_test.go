package api

import (
	"context"
	"net/http"
	"net/http/httptest"
	"path/filepath"
	"sync"
	"sync/atomic"
	"testing"
	"time"
)

func TestCallRetriesAfterRateLimit(t *testing.T) {
	var calls int32
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if atomic.AddInt32(&calls, 1) == 1 {
			w.Header().Set("Retry-After", "1")
			w.WriteHeader(http.StatusTooManyRequests)
			return
		}
		if r.Header.Get("Authorization") != "Bearer token" {
			t.Errorf("missing token: %q", r.Header.Get("Authorization"))
		}
		w.Write([]byte(`{"ok":true,"user_id":"U1"}`))
	}))
	defer server.Close()

	client := New()
	client.BaseURL = server.URL
	client.SetToken("token")

	id, err := client.AuthTest(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if id != "U1" {
		t.Fatalf("got %q", id)
	}
	if atomic.LoadInt32(&calls) != 2 {
		t.Fatalf("got %d calls", calls)
	}
}

func TestRateLimitOnlyPausesThatMethod(t *testing.T) {
	limited := make(chan struct{})
	var once sync.Once
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.URL.Path == "/slow" {
			w.Header().Set("Retry-After", "60")
			w.WriteHeader(http.StatusTooManyRequests)
			once.Do(func() { close(limited) })
			return
		}
		w.Write([]byte(`{"ok":true}`))
	}))
	defer server.Close()

	client := New()
	client.BaseURL = server.URL

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	paused := make(chan string, 1)
	client.RateLimited = func(method string, wait time.Duration) { paused <- method }

	go client.Call(ctx, "slow", nil, nil)
	<-limited
	if method := <-paused; method != "slow" {
		t.Fatalf("got %q", method)
	}

	done := make(chan error, 1)
	go func() { done <- client.Call(ctx, "fast", nil, nil) }()

	select {
	case err := <-done:
		if err != nil {
			t.Fatal(err)
		}
	case <-time.After(5 * time.Second):
		t.Fatal("another method's rate limit held this call back")
	}
}

func TestDownloadReportsProgress(t *testing.T) {
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Content-Length", "7")
		w.Write([]byte("content"))
	}))
	defer server.Close()

	var done, total int64
	path := filepath.Join(t.TempDir(), "file")
	size, err := New().Download(context.Background(), server.URL, false, path, func(d, n int64) { done, total = d, n })
	if err != nil {
		t.Fatal(err)
	}
	if size != 7 || done != 7 || total != 7 {
		t.Fatalf("got %d, %d of %d", size, done, total)
	}
}

func TestAuthErrors(t *testing.T) {
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte(`{"ok":false,"error":"invalid_auth"}`))
	}))
	defer server.Close()

	client := New()
	client.BaseURL = server.URL

	_, err := client.AuthTest(context.Background())
	if !IsAuthError(err) {
		t.Fatalf("got %v", err)
	}
}

func TestUsersFollowsCursor(t *testing.T) {
	var calls int32
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		r.ParseForm()
		if atomic.AddInt32(&calls, 1) == 1 {
			w.Write([]byte(`{"ok":true,"members":[{"id":"U1"}],"response_metadata":{"next_cursor":"next"}}`))
			return
		}
		if r.Form.Get("cursor") != "next" {
			t.Errorf("got cursor %q", r.Form.Get("cursor"))
		}
		w.Write([]byte(`{"ok":true,"members":[{"id":"U2"}],"response_metadata":{"next_cursor":""}}`))
	}))
	defer server.Close()

	client := New()
	client.BaseURL = server.URL

	users, err := client.Users(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if len(users) != 2 {
		t.Fatalf("got %d users", len(users))
	}
}
