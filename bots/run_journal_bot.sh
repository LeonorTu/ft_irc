#!/bin/bash

# Default IRC settings - can be overridden with environment variables
SERVER=${IRC_SERVER:-"127.0.0.1"}
PORT=${IRC_PORT:-6667}
PASSWORD=${IRC_PASSWORD:-"password"}
NICKNAME=${IRC_NICKNAME:-"JournalBot"}
CHANNEL=${IRC_CHANNEL:-"#logs"}

# journalctl options - modify as needed
JOURNAL_OPTS="-f --no-hostname"

echo "Starting Journal IRC Bot..."
echo "Server: $SERVER:$PORT"
echo "Channel: $CHANNEL"
echo "Bot nickname: $NICKNAME"

# Run journalctl and pipe output to the LogBot
journalctl $JOURNAL_OPTS | ./logbot "$SERVER" "$PORT" "$PASSWORD" "$NICKNAME" "$CHANNEL"
