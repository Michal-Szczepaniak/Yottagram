// Package main is the C ABI of yottaslack. It is built with -buildmode=c-shared
// and loaded by src/slack/slackbackend.cpp. Only this package may import "C".
package main

/*
#include <stdlib.h>
*/
import "C"

import (
	"sync"
	"time"
	"unsafe"

	"yottaslack/internal/session"
)

var (
	currentMu sync.RWMutex
	current   *session.Session
)

func getSession() *session.Session {
	currentMu.RLock()
	defer currentMu.RUnlock()
	return current
}

//export ys_init
func ys_init(dataDir *C.char) (result C.int) {
	defer func() {
		if recover() != nil {
			result = 1
		}
	}()

	s, err := session.New(C.GoString(dataDir))
	if err != nil {
		return 1
	}

	currentMu.Lock()
	current = s
	currentMu.Unlock()
	return 0
}

//export ys_send
func ys_send(request *C.char) {
	defer func() { _ = recover() }()

	if s := getSession(); s != nil {
		s.Handle([]byte(C.GoString(request)))
	}
}

// ys_receive returns the next event or NULL on timeout. The caller frees the
// result with ys_free.
//
//export ys_receive
func ys_receive(timeout C.double) (result *C.char) {
	defer func() {
		if recover() != nil {
			result = nil
		}
	}()

	s := getSession()
	if s == nil {
		return nil
	}

	event, ok := s.Next(time.Duration(float64(timeout) * float64(time.Second)))
	if !ok {
		return nil
	}
	return C.CString(event)
}

//export ys_free
func ys_free(value *C.char) {
	C.free(unsafe.Pointer(value))
}

//export ys_close
func ys_close() {
	defer func() { _ = recover() }()

	if s := getSession(); s != nil {
		s.Close()
	}
}

func main() {}
