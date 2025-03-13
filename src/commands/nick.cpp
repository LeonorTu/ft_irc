#include <Server.hpp>
#include <Client.hpp>
#include <message.hpp>
#include <responses.hpp>
#include <unordered_map>
#include <regex>
#include <ClientIndex.hpp>
#include <CommandProcessor.hpp>

// can now search for clients getClients() function, that returns a brand new 2am ClientIndex that
// has special functions to get clients by name and fd
bool isUsed(Server &server, const std::string &nickname)
{
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

void nick(const CommandProcessor::CommandContext &ctx)
{
    Server &server = Server::getInstance();
    Client &client = server.getClients().getByFd(ctx.clientFd);
    std::string oldNickname = ctx.oldNickname.empty() ? "*" : ctx.oldNickname;

    if (ctx.newNickname.empty()) {
        sendToClient(ctx.clientFd, ERR_NONICKNAMEGIVEN(oldNickname));
        return;
    }

    if (!isValidNickname(ctx.newNickname)) {
        sendToClient(ctx.clientFd, ERR_ERRONEUSNICKNAME(oldNickname, ctx.newNickname));
        return;
    }

    if (isUsed(server, ctx.newNickname)) {
        sendToClient(ctx.clientFd, ERR_NICKNAMEINUSE(oldNickname, ctx.newNickname));
        return;
    }

    client.setNickname(ctx.newNickname);
    server.getClients().updateNick(oldNickname, ctx.newNickname);
    if (ctx.oldNickname.empty()) {
        sendToClient(ctx.clientFd, NICKNAMECHANGE(oldNickname, ctx.newNickname));
    }
}
