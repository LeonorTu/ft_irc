#pragma once
#include <string>

// Common constants for the IRC server
const int MSG_BUFFER_SIZE = 512; // Buffer size for message handling

// ISUPPORT
const std::string CASEMAPPING = "ascii";
const int CHANNELLEN = 50;
const std::string CHANLIMIT = "#&50";
const std::string CHANTYPES = "#&";
const std::string CHANMODES = ",,kl,it";
const std::string PREFIX = "o(@)";
const int MODES = 3;
const int NICKLEN = 30;
const int TOPICLEN = 307;
const int USERLEN = 32;
const int REALLEN = 128;
const int EPOLL_MAX_EVENTS = 128;

// server info
const int SERVER_PORT = 6667;
const std::string SERVER_NAME = "localhost";
const std::string NETWORK_NAME = "J-A-S";
const std::string SERVER_VERSION = "0210";
const std::string USER_MODES = "";
const std::string CHANNEL_MODES = "itklo";
