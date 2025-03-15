#include <Server.hpp>
#include <Client.hpp>
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
    std::string currentNick = client.getNickname();
    std::string requestedNick = ctx.params[0];
    // Determine what to use in error messages (use * if no current nickname)
    std::string sourceNick = currentNick.empty() ? "*" : currentNick;

    if (requestedNick.empty()) {
        sendToClient(ctx.clientFd, ERR_NONICKNAMEGIVEN(sourceNick));
        return;
    }

    if (!isValidNickname(requestedNick)) {
        sendToClient(ctx.clientFd, ERR_ERRONEUSNICKNAME(sourceNick, requestedNick));
        return;
    }

    if (isUsed(server, requestedNick)) {
        sendToClient(ctx.clientFd, ERR_NICKNAMEINUSE(sourceNick, requestedNick));
        return;
    }

    client.setNickname(requestedNick);
    if (!currentNick.empty()) {
        // maybe somewhere else has nick need to be updated?
        server.getClients().updateNick(currentNick, requestedNick);
        sendToClient(ctx.clientFd, NICKNAMECHANGE(sourceNick, requestedNick));
    }
}
