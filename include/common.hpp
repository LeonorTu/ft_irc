#pragma once
#include <string>
#include <limits>

// Common constants for the IRC server
const int MSG_BUFFER_SIZE = 512; // Buffer size for message handling
const int TEXT_LIMIT = 300;      // Limit for text messages
// ISUPPORT
const std::string CASEMAPPING = "ascii";
const int CHANNELLEN = 50;
const std::string CHANLIMIT = "#&:50";
const int LOCCHANLMAX = 50;
const int REGCHANLMAX = 50;
const std::string CHANTYPES = "#&";
const std::string CHANMODES = ",,kl,it";
const std::string PREFIX = "o(@)";
const int MODES = 3;
const int NICKLEN = 30;
const int TOPICLEN = 307;
const int USERLEN = 32;
const int REALLEN = 128;
const int EPOLL_MAX_EVENTS = 128;
const int MAX_PARAMS = 4;
const int MIN_PASS = 2;
const int MAX_PASS = 32;
const int MAXTARGETS = 4;
const unsigned long MIN_CHANNEL_LIMIT = 1;
const unsigned long MAX_CHANNEL_LIMIT = std::numeric_limits<unsigned long>::max();
// server info
const int SERVER_PORT = 6667;
const std::string SERVER_NAME = "JAS 42";
const std::string NETWORK_NAME = "J-A-S";
const std::string SERVER_VERSION = "0210";
const std::string USER_MODES = "";
const std::string CHANNEL_MODES = "itklo";

// MOTD array with funny messages
const std::string MOTD_LINES[] = {
    "Welcome to ft_irc - Where bugs come to party!",
    "Remember: If your code works the first time, you're doing it wrong.",
    "IRC: Because sometimes you need to talk to humans without seeing them.",
    "42 is not just the answer to life, it's also our uptime in seconds.",
    "If you find any bugs, they're features we haven't documented yet."};
const int MOTD_LINE_COUNT = 5;
