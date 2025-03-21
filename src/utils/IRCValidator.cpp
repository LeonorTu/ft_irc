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

// bool IRCValidator::isValidUserList(int clientFd, const std::string &nickname,
//                                    const std::string &userList)
// {
//     std::istringstream userStream(userList);
//     std::string username;
//     while (std::getline(userStream, username, ',') )
//     {
//         if (isValidNickname(clientFd, ))
//     }

//     return false;
// }

// std::vector<std::string> returnTargets(std::vector<std::string> params)
// {
//     std::vector<std::string> targets;
//     for (auto it = params.begin(); it != params.end() - 1; ++it) {
//         targets.push_back(*it);
//     }
//     return targets;
// }

// std::string returnMessage(std::vector<std::string> params)
// {
//     std::string message = params.back();
//     return message;
// }

std::string truncateAfterCommaInTargets(std::string target)
{
    size_t commaPos = target.find(',');
    if (commaPos != std::string::npos) {
        target.erase(commaPos);
    }
    return target;
}

std::string putMessageInOne(std::vector<std::string> params)
{
    std::string message;
    for (size_t i = 1; i < params.size(); ++i) {
        message += params[i];
        if (i != params.size() - 1) {
            message += " ";
        }
    }
    return message;
}

bool IRCValidator::isValidTarget(ClientIndex &clients, ChannelManager &channelManager,
                                 std::vector<std::string> params, Client &client)
{

    std::string target = params[0];

    // client will handle the message adding ':' instead but should i handle it or not
    if (params[1].empty()) {
        sendToClient(client.getFd(), ERR_NOTEXTTOSEND(client.getNickname()));
        return false;
    }
    std::string message = putMessageInOne(params);
    if (target[0] == CHANTYPES[0] || target[0] == CHANTYPES[1]) {
        try {
            Channel &channel = channelManager.getChannel(target);
            channel.broadcastMessage(":" + client.getPrefixPrivmsg() + " PRIVMSG " + target + " :" +
                                     message);
            std::cout << "Sending message to channel: " << target << std::endl;
        }
        catch (const std::exception &e) {
            sendToClient(client.getFd(), ERR_NOSUCHCHANNEL(client.getNickname(), target));
            return false;
        }
    }
    else {
        target = truncateAfterCommaInTargets(target);
        if (!clients.nickExists(target)) {
            sendToClient(client.getFd(), ERR_NOSUCHNICK(client.getNickname(), target));
            return false;
        }
        sendToClient(clients.getByNick(target).getFd(),
                     ":" + client.getPrefixPrivmsg() + " PRIVMSG " + target + " :" + message);
        // std::cout << "Sending message to channel: " << target << std::endl;
    }
    return true;
}