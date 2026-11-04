// Command yscli drives the session from a terminal, for trying it against a
// real workspace without the app. It logs in with $SLACK_TOKEN, prints every
// event and sends each line of stdin as a request:
//
//	{"id":10,"method":"history","params":{"chat":1099511627776,"from":0,"offset":0,"limit":5}}
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"time"

	"yottaslack/internal/session"
)

func main() {
	token := os.Getenv("SLACK_TOKEN")
	if token == "" {
		log.Fatal("SLACK_TOKEN is not set")
	}

	dataDir, err := os.MkdirTemp("", "yottaslack")
	if err != nil {
		log.Fatal(err)
	}
	defer os.RemoveAll(dataDir)

	s, err := session.New(dataDir)
	if err != nil {
		log.Fatal(err)
	}
	defer s.Close()

	go func() {
		for {
			if event, ok := s.Next(time.Second); ok {
				fmt.Println(event)
			}
		}
	}()

	login, err := json.Marshal(map[string]any{
		"id":     2,
		"method": "login",
		"params": map[string]any{"token": token},
	})
	if err != nil {
		log.Fatal(err)
	}
	s.Handle(login)

	reader := bufio.NewReader(os.Stdin)
	for {
		line, err := reader.ReadBytes('\n')
		if len(line) > 0 {
			s.Handle(line)
		}
		if err != nil {
			return
		}
	}
}
