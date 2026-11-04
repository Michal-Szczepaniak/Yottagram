package socket

import (
	"bufio"
	"bytes"
	"context"
	"io"
	"net"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
	"time"
)

const testTimeout = 5 * time.Second

// serve upgrades one connection and hands it to script as a server-side Conn.
func serve(t *testing.T, script func(conn net.Conn, reader *bufio.Reader)) string {
	t.Helper()

	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		conn, buffer, err := w.(http.Hijacker).Hijack()
		if err != nil {
			t.Error(err)
			return
		}
		defer conn.Close()

		accept := AcceptKey(r.Header.Get("Sec-WebSocket-Key"))
		io.WriteString(conn, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: "+accept+"\r\n\r\n")
		script(conn, buffer.Reader)
	}))
	t.Cleanup(server.Close)

	return "ws" + strings.TrimPrefix(server.URL, "http")
}

func dial(t *testing.T, url string) *Conn {
	t.Helper()

	conn, err := Dial(context.Background(), url, testTimeout)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { conn.Close() })
	return conn
}

func TestReadJoinsFragmentsAndAnswersPings(t *testing.T) {
	pong := make(chan []byte, 1)
	url := serve(t, func(conn net.Conn, reader *bufio.Reader) {
		conn.Write([]byte{0x89, 2, 'h', 'i'})
		conn.Write([]byte{0x01, 3, 'a', 'b', 'c'})
		conn.Write([]byte{0x80, 3, 'd', 'e', 'f'})

		server := &Conn{conn: conn, reader: reader}
		_, opcode, payload, err := server.readFrame()
		if err != nil || opcode != opPong {
			t.Errorf("got opcode %d, %v", opcode, err)
		}
		pong <- payload
		conn.Write([]byte{0x88, 0})
	})

	client := dial(t, url)
	message, err := client.Read()
	if err != nil || string(message) != "abcdef" {
		t.Fatalf("got %q, %v", message, err)
	}
	if got := <-pong; string(got) != "hi" {
		t.Fatalf("pong must echo the ping, got %q", got)
	}
	if _, err := client.Read(); err != io.EOF {
		t.Fatalf("got %v", err)
	}
}

func TestWriteTextIsMaskedForAllLengths(t *testing.T) {
	sizes := []int{5, 300, 70000}
	received := make(chan []byte, len(sizes))
	url := serve(t, func(conn net.Conn, reader *bufio.Reader) {
		server := &Conn{conn: conn, reader: reader}
		for range sizes {
			_, _, payload, err := server.readFrame()
			if err != nil {
				t.Error(err)
				return
			}
			received <- payload
		}
	})

	client := dial(t, url)
	for _, size := range sizes {
		want := bytes.Repeat([]byte("x"), size)
		if err := client.WriteText(want); err != nil {
			t.Fatal(err)
		}
		if got := <-received; !bytes.Equal(got, want) {
			t.Fatalf("size %d came back as %d bytes", size, len(got))
		}
	}
}

func TestDialRejectsBadAcceptKey(t *testing.T) {
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		conn, _, err := w.(http.Hijacker).Hijack()
		if err != nil {
			t.Error(err)
			return
		}
		defer conn.Close()
		io.WriteString(conn, "HTTP/1.1 101 Switching Protocols\r\nSec-WebSocket-Accept: wrong\r\n\r\n")
	}))
	defer server.Close()

	url := "ws" + strings.TrimPrefix(server.URL, "http")
	if _, err := Dial(context.Background(), url, testTimeout); err == nil {
		t.Fatal("expected an error")
	}
}
