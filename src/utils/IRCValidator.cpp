#include <IRCValidator.hpp>
#include <common.hpp>
#include <regex>
#include <responses.hpp>
#include <ClientIndex.hpp>
#include <Client.hpp>
#include <ChannelManager.hpp>
#include <Channel.hpp>

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

bool IRCValidator::isValidTopic(int clientFd, const std::string &nickname, std::string &text)
{
    std::regex printablePattern("[[:print:]]*");
    if (text.length() > TOPICLEN)
        text = text.substr(0, TOPICLEN);
    if (!std::regex_match(text, printablePattern)) {
        sendToClient(clientFd, ERR_INVALIDTEXT(nickname, text));
        return false;
    }
    return true;
}

bool IRCValidator::isPrintable(int clientFd, const std::string &nickname, const std::string &text,
                               size_t limit)
{
    std::regex printablePattern("[[:print:]]+");
    if (text.length() > limit || !std::regex_match(text, printablePattern)) {
        sendToClient(clientFd, ERR_INVALIDTEXT(nickname, text));
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

bool IRCValidator::isValidPort(const std::string &portStr)
{
    try {
        int port = std::stoi(portStr);
        if (port < 1 || port > 65535) {
            return false;
        }
    }
    catch (const std::invalid_argument &e) {
        return false;
    }
    catch (const std::out_of_range &e) {
        return false;
    }
    return true;
}

bool IRCValidator::isValidServerPassword(const std::string &password)
{
    if (password.length() < MIN_PASS || password.length() > MAX_PASS) {
        return false;
    }
    std::regex passwordPattern(R"(^[a-zA-Z0-9!@#$%^&*()\-_=+\[\]{}|;:'",.<>?/]+$)");
    if (!std::regex_match(password, passwordPattern)) {
        return false;
    }
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

bool IRCValidator::isValidChannelLimit(const std::string &limit)
{
    std::regex digitPattern(R"(^\d+$)");
    if (!std::regex_match(limit, digitPattern)) {
        return false;
    }
    try {
        unsigned long numLimit = std::stoul(limit);
        if (numLimit < MIN_CHANNEL_LIMIT || numLimit > MAX_CHANNEL_LIMIT) {
            return false;
        }
    }
    catch (const std::exception &e) {
        return false;
    }
    return true;
}

bool IRCValidator::isValidTarget(const std::unordered_multimap<WhichType, std::string> &targets,
                                 int clientFd, std::string nickname)
{

    for (auto &it : targets) {
        if (it.first == CHANNEL) {
            if (!isValidChannelName(clientFd, it.second))
                return false;
        }
        else if (it.first == NICKNAME) {
            if (!isValidNickname(clientFd, nickname, it.second))
                return false;
        }
    }
    return true;
}

bool IRCValidator::isValidText(int clientFd, const std::string &nickname,
                               const std::string &message)
{
    std::regex printablePattern("[[:print:]]*");
    if (message.empty()) {
        sendToClient(clientFd, ERR_NOTEXTTOSEND(nickname));
        return false;
    }
    if (!std::regex_match(message, printablePattern)) {
        sendToClient(clientFd, ERR_INVALIDTEXT(nickname, message));
        return false;
    }
    return true;
}
