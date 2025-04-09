# ft_irc - IRC Server Implementation

An IRC (Internet Relay Chat) server implementation written in C++17, featuring a robust event-driven architecture and basic command support.

## Features

- **Full IRC Protocol Support**: Implements the core IRC protocol commands
- **Channel Management**: Create, join, and manage channels with various modes
- **User Authentication**: Basic user registration and authentication
- **Event-Driven Architecture**: Non-blocking I/O using epoll for efficient connection handling
- **Modern C++ Design**: Built with C++17 standards and practices
- **Comprehensive Testing**: Unit and integration tests using Google Test framework

## System Components

The server consists of several key components working together:

- **ConnectionManager**: Handles client connections, message parsing, and disconnection
- **ClientIndex**: Manages client registration and lookup
- **ChannelManager**: Handles channel creation and lookup
- **EventLoop**: Manages the epoll-based event loop for non-blocking I/O
- **CommandRunner**: Processes and executes IRC commands
- **MessageParser**: Parses incoming messages according to IRC protocol

## Building and Installation

### Prerequisites

- CMake 3.10 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourusername/ft_irc.git
cd ft_irc

# Build with CMake
mkdir build && cd build
cmake ..
make

# Run tests (optional)
make test
```

## Usage

### Running the Server

```bash
# Basic usage
./ft_irc <port> <password>

# Example
./ft_irc 6667 serverpassword

# Default settings (port 6667, password "42")
./ft_irc
```

### Connecting with a Client

Use any standard IRC client (irssi, hexchat, etc.) to connect:

```
/server localhost 6667
/pass serverpassword
/nick yournickname
/user username 0 * :Your Real Name
```

## Supported Commands

- **Channel Operations**: JOIN, PART, TOPIC, MODE, KICK, INVITE
- **Messaging**: PRIVMSG, NOTICE
- **User Operations**: NICK, USER, QUIT
- **Server Operations**: PING, PONG, CAP, MOTD

## Channel Modes

- `+i`: Invite only - users must be invited to join
- `+t`: Topic protection - only operators can change the topic
- `+k`: Channel key (password) - users need the key to join
- `+l`: User limit - limits the number of users in a channel
- `+o`: Operator status - grants special privileges to a user

## Testing

The project includes extensive tests built with Google Test:

```bash
# Build and run tests
cd build
make test
```

- **Message Handling Tests**: Tests for oversized message handling, chunked messages, etc.
- **Command Tests**: Comprehensive tests for all supported commands
- **Edge Case Tests**: Tests for various error conditions and edge cases

## Contributors

- [Wassaaa](https://github.com/wassaaa)
- [LeonorTu](https://github.com/LeonorTu)
- [Stella-Kwon](https://github.com/Stella-Kwon)
