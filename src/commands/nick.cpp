#include <Server.hpp>
#include <Client.hpp>
#include <responses.hpp>
#include <unordered_map>
#include <IRCValidator.hpp>
#include <ClientIndex.hpp>
#include <CommandProcessor.hpp>
#include <commandHandlers.hpp>
#include <ConnectionManager.hpp>

void nick(const CommandProcessor::CommandContext &ctx)
{
    Server &server = Server::getInstance();
    ClientIndex &clients = server.getClients();
    Client &client = clients.getByFd(ctx.clientFd);
    std::string oldNickname = client.getNickname();
    std::string newNickname = ctx.params[0];

    if (!client.getPasswordVerified()) {
        sendToClient(ctx.clientFd, ERR_PASSWDMISMATCH(ctx.source));
        return;
    }

    if (newNickname.empty()) {
        sendToClient(ctx.clientFd, ERR_NONICKNAMEGIVEN(oldNickname));
        return;
    }

    if (!IRCValidator::isValidNickname(ctx.clientFd, oldNickname, newNickname)) {
        sendToClient(ctx.clientFd, ERR_ERRONEUSNICKNAME(oldNickname, newNickname));
        return;
    }

    if (clients.nickExists(newNickname)) {
        sendToClient(ctx.clientFd, ERR_NICKNAMEINUSE(oldNickname, newNickname));
        return;
    }

    client.setNickname(newNickname);
    if (client.getIsRegistered()) {
        // maybe somewhere else has nick need to be updated?
        server.getClients().updateNick(oldNickname, newNickname);
        sendToClient(ctx.clientFd, NICK(oldNickname, newNickname));
    }

    if (!client.getIsRegistered() && client.getPasswordVerified() &&
        !client.getNickname().empty() && !client.getUsername().empty()) {
        client.setIsRegistered(true);
        server.getClients().addNick(ctx.clientFd);
        sendWelcome(ctx.clientFd);
    }
}
