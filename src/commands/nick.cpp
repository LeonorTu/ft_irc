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
    std::string currentNick = client.getNickname();
    std::string requestedNick = ctx.params[0];
    // Determine what to use in error messages (use * if no current nickname)
    std::string sourceNick = currentNick.empty() ? "*" : currentNick;

    if (!client.getPasswordVerified()) {
        sendToClient(ctx.clientFd, ERR_PASSWDMISMATCH(ctx.source));
        server.getConnectionManager().disconnectClient(client);
        return;
    }

    if (requestedNick.empty()) {
        sendToClient(ctx.clientFd, ERR_NONICKNAMEGIVEN(sourceNick));
        return;
    }

    if (!IRCValidator::isValidNickname(ctx.clientFd, sourceNick, requestedNick)) {
        sendToClient(ctx.clientFd, ERR_ERRONEUSNICKNAME(sourceNick, requestedNick));
        return;
    }

    if (clients.nickExists(requestedNick)) {
        sendToClient(ctx.clientFd, ERR_NICKNAMEINUSE(sourceNick, requestedNick));
        return;
    }

    client.setNickname(requestedNick);
    if (!currentNick.empty() && client.getIsRegistered()) {
        // maybe somewhere else has nick need to be updated?
        server.getClients().updateNick(currentNick, requestedNick);
        sendToClient(ctx.clientFd, NICKNAMECHANGE(sourceNick, requestedNick));
    }

    if (!client.getIsRegistered() && client.getPasswordVerified() &&
        !client.getNickname().empty() && !client.getUsername().empty()) {
        client.setIsRegistered(true);
        sendWelcome(ctx.clientFd);
    }
}
