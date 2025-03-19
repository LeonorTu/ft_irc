#include <IRCValidator.hpp>
#include <common.hpp>
#include <regex>
#include <responses.hpp>

bool IRCValidator::isValidNickname(int clientFd, const std::string &oldNickname,
                                   const std::string &newNickname)
{
    std::regex nicknamePattern(R"(^[a-zA-Z\[\]\\`_^{|}][a-zA-Z0-9\[\]\\`_^{|}-]*$)");
    if (newNickname.length() > NICKLEN || !std::regex_match(newNickname, nicknamePattern)) {
        sendToClient(clientFd, ERR_ERRONEUSNICKNAME(oldNickname, newNickname));
        return false;
    }
    if (newNickname.empty()) {
        sendToClient(clientFd, ERR_NONICKNAMEGIVEN(oldNickname));
        return false;
    }
    return true;
}

bool IRCValidator::isValidChannelName(int clientFd, const std::string &channelName)
{
    std::regex channelNamePattern(R"(^[#&][^\x00\x07\x0A\x0D ,:]{1,49}$)");
    if (!std::regex_match(channelName, channelNamePattern)) {
        sendToClient(clientFd, ERR_BADCHANMASK(channelName));
        return false;
    }
    return true;
}

bool IRCValidator::isValidTopic(int clientFd, const std::string &nickname, const std::string &topic)
{
    std::regex topicPattern("[[:print:]]+");
    if (topic.length() > TOPICLEN || !std::regex_match(topic, topicPattern)) {
        sendToClient(clientFd, ERR_INVALIDTOPIC(nickname, topic));
        return false;
    }
    return true;
}

bool IRCValidator::isValidUsername(int clientFd, const std::string &nickname, std::string &username)
{
    if (username.length() > USERLEN) {
        username = username.substr(0, USERLEN);
    }
    std::regex usernamePattern(R"(^[a-zA-Z0-9_-]+$)");
    if (!std::regex_match(username, usernamePattern)) {
        sendToClient(clientFd, ERR_INVALIDUSERNAME(nickname, username));
        return false;
    }
    return true;
}

bool IRCValidator::isValidRealname(int clientFd, const std::string &nickname,
                                   const std::string &realname)
{
    std::regex realnamePattern(R"(^[^\r\n\0]+$)");
    if (realname.length() > REALLEN || !std::regex_match(realname, realnamePattern)) {
        sendToClient(clientFd, ERR_INVALIDREALNAME(nickname, realname));
        return false;
    }
    return true;
}

bool IRCValidator::isValidChannelMode()
{
    return true;
}

bool IRCValidator::isValidServerPassword()
{
    return true;
}

bool IRCValidator::isValidChannelKey(int clientFd, const std::string &nickname,
                                     const std::string &key)
{
    std::regex keyPattern(R"(^[a-zA-Z0-9!@#$%^&*()\-_=+\[\]{}|;:'",.<>?/]+$)");
    if (!std::regex_match(key, keyPattern)) {
        sendToClient(clientFd, ERR_INVALIDKEY(nickname, key));
        return false;
    }
    return true;
}
