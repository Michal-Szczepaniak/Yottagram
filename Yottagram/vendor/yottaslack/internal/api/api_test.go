package api

import (
	"context"
	"net/http"
	"net/http/httptest"
	"sync/atomic"
	"testing"
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
