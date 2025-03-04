#include <server.hpp>
#include <Client.hpp>
#include <message.hpp>
#include <responses.hpp>
#include <unordered_map>
#include <regex>
#include <ClientIndex.hpp>

// can now search for clients getClients() function, that returns a brand new 2am ClientIndex that has special functions
// to get clients by name and fd
bool isUsed(Server &server, int clientFd, std::string &nickname)
{
    // std::unordered_map<int, Client*> &client_list = server.getClients();
    // std::unordered_map<int, Client*>::iterator client = client_list.begin();
    // while (client != client_list.end()) {
    //     if (client->second->getFd() != clientFd && client->second->getNickname() == nickname)
    //         return (true);
    //     client++;
    // }
    // return (false);
    ClientIndex &clients = server.getClients();
    return clients.nickExists(nickname);
}

bool isValidNickname(const std::string &nickname)
{
    if (nickname.empty() || nickname.length() > 30)
        return false;
    std::regex nicknamePattern("^[a-zA-Z\\[\\]\\\\`_^{|}][a-zA-Z0-9\\[\\]\\\\`_^{|}-]*$");

    return std::regex_match(nickname, nicknamePattern);
}

void nick(Server &server, Client &client, message cmd)
{
    std::string nickname = cmd.parameters[0];
    std::string oldNickname = client.getNickname();

    if (!client.getIsRegistered()) {
        client.setNickname(nickname);
        // server.getClients()->add(client);
    }
    if (nickname.empty()) {
        sendToClient(client.getFd(), ERR_NONICKNAMEGIVEN(oldNickname));
    }
    else if (!isValidNickname(nickname)) {
        sendToClient(client.getFd(), ERR_ERRONEUSNICKNAME(oldNickname, nickname));
    }
    else if (isUsed(server, client.getFd(), nickname)) {
        sendToClient(client.getFd(), ERR_NICKNAMEINUSE(oldNickname, nickname));
    }
    else {
        sendToClient(client.getFd(), NICKNAMECHANGE(oldNickname, nickname));
        client.setNickname(nickname);
        // server.getClients()->updateNick(nickname);
    }
}
