#pragma once
#include <string>

// Common definitions and constants for the IRC server
const int MAX_CLIENTS = 100;     // Maximum number of clients
const int MSG_BUFFER_SIZE = 512; // Buffer size for message handling
const int SERVER_PORT = 6667;    // Server port

const std::string SERVER_NAME = "localhost";
const std::string NETWORK_NAME = "J-A";
const std::string SERVER_VERSION = "v1.0";
const std::string USER_MODES = "itkol";
const std::string CHANNEL_MODES = "bklov";

const int EPOLL_MAX_EVENTS = 10;
