#include <server.hpp>
#include <Client.hpp>
#include <message.hpp>
#include <responses.hpp>
#include <unordered_map>
#include <regex>

void nick(Server &server, Client &client, message cmd)
{
    std::string nickname = cmd.parameters[0];

    if (!client.getIsRegistered()) {
        client.setNickname(nickname);
    }
    if (nickname.empty()) {
        send(client.getFd(), ERR_NONICKNAMEGIVEN(client.getNickname()).c_str(),
             ERR_NONICKNAMEGIVEN(client.getNickname()).size(), 0);
    }
    else if (!isValidNickname(nickname))
    {
        send(client.getFd(), ERR_ERRONEUSNICKNAME(client.getNickname(), nickname).c_str(),
             ERR_ERRONEUSNICKNAME(client.getNickname(), nickname).size(), 0);
    }
    else if (isUsed(server, client.getFd(), nickname)) {
        send(client.getFd(), ERR_NICKNAMEINUSE(client.getNickname(), nickname).c_str(),
             ERR_NICKNAMEINUSE(client.getNickname(), nickname).size(), 0);
    }
    else{
        send(client.getFd(), NICKNAMECHANGE(client.getNickname(), nickname).c_str(),
             NICKNAMECHANGE(client.getNickname(), nickname).size(), 0);
        client.setNickname(nickname);
    }

}

bool isUsed(Server &server, int clientFd, std::string &nickname)
{
    std::unordered_map<int, Client*> &client_list = server.getClients();
    std::unordered_map<int, Client*>::iterator client = client_list.begin();
    while (client != client_list.end()) {
        if (client->second->getFd() != clientFd && client->second->getNickname() == nickname)
            return (true);
        client++;
    }
    return (false);
}

bool isValidNickname(const std::string &nickname)
{
    if (nickname.empty() || nickname.length() > 30)
        return false;
    std::regex nicknamePattern("^[a-zA-Z\\[\\]\\\\`_^{|}][a-zA-Z0-9\\[\\]\\\\`_^{|}-]*$");

    return std::regex_match(nickname, nicknamePattern);
}