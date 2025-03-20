#include <CommandProcessor.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <responses.hpp>
#include <ClientIndex.hpp>
#include <ConnectionManager.hpp>
#include <commandHandlers.hpp>

void pass(const CommandProcessor::CommandContext &ctx)
{
    Server &server = Server::getInstance();
    Client &client = server.getClients().getByFd(ctx.clientFd);
    std::string clientPassword = ctx.params[0];

    if (client.getIsRegistered()) {
        sendToClient(ctx.clientFd, ERR_ALREADYREGISTERED(ctx.source));
        return;
    }

    if (clientPassword.empty()) {
        sendToClient(ctx.clientFd, ERR_NEEDMOREPARAMS(ctx.source, "PASS"));
        return;
    }

    std::string serverPassword = server.getPassword();
    if (clientPassword != serverPassword) {
        sendToClient(ctx.clientFd, ERR_PASSWDMISMATCH(ctx.source));
        server.getConnectionManager().disconnectClient(client);
        return;
    }

    client.setPasswordVerified(true);
}