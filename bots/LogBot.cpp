#include "LogBot.hpp"
#include <iostream>

LogBot::LogBot(const std::string& server, int port, const std::string& password,
               const std::string& nickname, const std::string& channel)
    : IRCBot(server, port, password, nickname, channel)
{
}

void LogBot::handleMessage(const std::string& sender, const std::string& message) {
    std::cout << "[Log] <" << sender << "> " << message << std::endl;
}

void LogBot::logMessage(const std::string& message) {
    sendMessage(message);
}
