#pragma once

#include <string>
#include <vector>

class ClientIndex;
class Client;
class ChannelManager;
class IRCValidator
{
public:
    static bool isValidNickname(int clientFdconst, const std::string &sourceNick,
                                const std::string &requestedNick);
    static bool isValidUsername(int clientFd, const std::string &nickname, std::string &username);
    static bool isValidRealname(int clientFd, const std::string &nickname,
                                const std::string &realname);
    static bool isValidChannelName(int clientFd, const std::string &channelName);
    static bool isPrintable(int clientFd, const std::string &nickname, const std::string &text,
                            size_t limit);
    static bool isValidPort(const std::string &portStr);
    static bool isValidServerPassword(const std::string &password);
    static bool isValidChannelKey(int clientFd, const std::string &nickname, const std::string &key);
    static bool isValidTarget(ClientIndex &clients, ChannelManager &channelManager, std::vector<std::string> params, Client &client);

private:
};
