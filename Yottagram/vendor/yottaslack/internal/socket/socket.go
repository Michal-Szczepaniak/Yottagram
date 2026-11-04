// Package socket is a minimal WebSocket client, enough for Slack's Socket Mode.
// It lives here so that the library keeps building without any dependencies.
package socket

import (
	"bufio"
	"context"
	"crypto/rand"
	"crypto/sha1"
	"crypto/tls"
	"encoding/base64"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"net"
	"net/http"
	"net/url"
	"sync"
	"time"
)

const (
	opContinuation = 0
	opText         = 1
	opBinary       = 2
	opClose        = 8
	opPing         = 9
	opPong         = 10

	handshakeTimeout = 30 * time.Second
	writeTimeout     = 10 * time.Second
	maxMessageSize   = 16 << 20

	acceptGUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
)

// Conn is a WebSocket connection. Read must only be called from one goroutine,
// the other methods can be called from any.
type Conn struct {
	conn        net.Conn
	reader      *bufio.Reader
	readTimeout time.Duration
	writeMu     sync.Mutex
}

// Dial connects to a ws:// or wss:// URL. Read fails if nothing arrives for
// readTimeout, so a dead network doesn't go unnoticed.
func Dial(ctx context.Context, rawURL string, readTimeout time.Duration) (*Conn, error) {
	parsed, err := url.Parse(rawURL)
	if err != nil {
		return nil, errors.New("invalid socket url")
	}

	var conn net.Conn
	switch parsed.Scheme {
	case "wss":
		conn, err = (&tls.Dialer{}).DialContext(ctx, "tcp", address(parsed, "443"))
	case "ws":
		conn, err = (&net.Dialer{}).DialContext(ctx, "tcp", address(parsed, "80"))
	default:
		return nil, fmt.Errorf("unsupported socket scheme %q", parsed.Scheme)
	}
	if err != nil {
		return nil, err
	}

	reader, err := handshake(conn, parsed)
	if err != nil {
		conn.Close()
		return nil, err
	}
	return &Conn{conn: conn, reader: reader, readTimeout: readTimeout}, nil
}

func address(parsed *url.URL, defaultPort string) string {
	if parsed.Port() != "" {
		return parsed.Host
	}
	return net.JoinHostPort(parsed.Hostname(), defaultPort)
}

func handshake(conn net.Conn, parsed *url.URL) (*bufio.Reader, error) {
	nonce := make([]byte, 16)
	if _, err := rand.Read(nonce); err != nil {
		return nil, err
	}
	key := base64.StdEncoding.EncodeToString(nonce)

	if err := conn.SetDeadline(time.Now().Add(handshakeTimeout)); err != nil {
		return nil, err
	}

	_, err := fmt.Fprintf(conn, "GET %s HTTP/1.1\r\nHost: %s\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Version: 13\r\n\r\n",
		parsed.RequestURI(), parsed.Host, key)
	if err != nil {
		return nil, err
	}

	reader := bufio.NewReader(conn)
	response, err := http.ReadResponse(reader, &http.Request{Method: http.MethodGet})
	if err != nil {
		return nil, err
	}
	if response.StatusCode != http.StatusSwitchingProtocols {
		return nil, fmt.Errorf("socket upgrade failed: %s", response.Status)
	}
	if response.Header.Get("Sec-WebSocket-Accept") != AcceptKey(key) {
		return nil, errors.New("socket upgrade failed: bad accept key")
	}

	return reader, conn.SetDeadline(time.Time{})
}

// AcceptKey is the server's expected answer to a Sec-WebSocket-Key.
func AcceptKey(key string) string {
	sum := sha1.Sum([]byte(key + acceptGUID))
	return base64.StdEncoding.EncodeToString(sum[:])
}

// Read returns the next message, answering pings on the way. It returns io.EOF
// when the server closes the connection.
func (c *Conn) Read() ([]byte, error) {
	var message []byte

	for {
		if err := c.conn.SetReadDeadline(time.Now().Add(c.readTimeout)); err != nil {
			return nil, err
		}

		fin, opcode, payload, err := c.readFrame()
		if err != nil {
			return nil, err
		}

		switch opcode {
		case opPing:
			if err := c.writeFrame(opPong, payload); err != nil {
				return nil, err
			}
		case opClose:
			return nil, io.EOF
		case opContinuation, opText, opBinary:
			if len(message)+len(payload) > maxMessageSize {
				return nil, errors.New("socket message too large")
			}
			message = append(message, payload...)
			if fin {
				return message, nil
			}
		}
	}
}

func (c *Conn) readFrame() (bool, byte, []byte, error) {
	var header [2]byte
	if _, err := io.ReadFull(c.reader, header[:]); err != nil {
		return false, 0, nil, err
	}

	length := uint64(header[1] & 0x7f)
	switch length {
	case 126:
		var extended [2]byte
		if _, err := io.ReadFull(c.reader, extended[:]); err != nil {
			return false, 0, nil, err
		}
		length = uint64(binary.BigEndian.Uint16(extended[:]))
	case 127:
		var extended [8]byte
		if _, err := io.ReadFull(c.reader, extended[:]); err != nil {
			return false, 0, nil, err
		}
		length = binary.BigEndian.Uint64(extended[:])
	}
	if length > maxMessageSize {
		return false, 0, nil, errors.New("socket frame too large")
	}

	var mask [4]byte
	masked := header[1]&0x80 != 0
	if masked {
		if _, err := io.ReadFull(c.reader, mask[:]); err != nil {
			return false, 0, nil, err
		}
	}

	payload := make([]byte, length)
	if _, err := io.ReadFull(c.reader, payload); err != nil {
		return false, 0, nil, err
	}
	if masked {
		for i := range payload {
			payload[i] ^= mask[i%4]
		}
	}

	return header[0]&0x80 != 0, header[0] & 0x0f, payload, nil
}

func (c *Conn) writeFrame(opcode byte, payload []byte) error {
	frame := []byte{0x80 | opcode}

	switch {
	case len(payload) < 126:
		frame = append(frame, 0x80|byte(len(payload)))
	case len(payload) < 1<<16:
		frame = append(frame, 0x80|126)
		frame = binary.BigEndian.AppendUint16(frame, uint16(len(payload)))
	default:
		frame = append(frame, 0x80|127)
		frame = binary.BigEndian.AppendUint64(frame, uint64(len(payload)))
	}

	var mask [4]byte
	if _, err := rand.Read(mask[:]); err != nil {
		return err
	}
	frame = append(frame, mask[:]...)
	for i, value := range payload {
		frame = append(frame, value^mask[i%4])
	}

	c.writeMu.Lock()
	defer c.writeMu.Unlock()

	if err := c.conn.SetWriteDeadline(time.Now().Add(writeTimeout)); err != nil {
		return err
	}
	_, err := c.conn.Write(frame)
	return err
}

// WriteText sends a text message.
func (c *Conn) WriteText(message []byte) error {
	return c.writeFrame(opText, message)
}

// Ping sends a ping. Its pong resets the read timeout.
func (c *Conn) Ping() error {
	return c.writeFrame(opPing, nil)
}

// Close closes the connection, unblocking Read.
func (c *Conn) Close() error {
	return c.conn.Close()
}
