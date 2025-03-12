#pragma once

#include <string>

class IRCValidator
{
public:
    static bool isValidNickname(int clientFd, const std::string &nickname);
    static bool isValidUsername(int clientFd, const std::string &username);
    static bool isValidChannelName(int clientFd, const std::string &channelName);
    static bool isValidChannelMode(int clientFd, const std::string &mode);
    static bool isValidServerPassword(int clientFd, const std::string &password);

private:
};
