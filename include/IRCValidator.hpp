#pragma once

#include <string>
#include <vector>
#include <unordered_map>

enum WhichType
{
    CHANNEL,
    NICKNAME
};

class IRCValidator
{
public:
    static bool isValidNickname(int clientFdconst, const std::string &sourceNick,
                                const std::string &requestedNick);
    static bool isValidUsername(int clientFd, const std::string &nickname, std::string &username);
    static bool isValidRealname(int clientFd, const std::string &nickname,
                                const std::string &realname);
    static bool isValidChannelName(int clientFd, const std::string &channelName);
    static bool isValidTopic(int clientFd, const std::string &nickname, std::string &text);
    static bool isPrintable(int clientFd, const std::string &nickname, const std::string &text,
                            size_t limit);
    static bool isValidPort(const std::string &portStr);
    static bool isValidServerPassword(const std::string &password);
    static bool isValidChannelKey(int clientFd, const std::string &nickname,
                                  const std::string &key);
    static bool isValidChannelLimit(const std::string &limit);
    static bool isValidTarget(const std::unordered_multimap<WhichType, std::string> &targets,
                              int clientFd, std::string nickname);
    static bool isValidText(int clientFd, const std::string &nickname, const std::string &message);

private:
};
