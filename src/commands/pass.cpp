#include <CommandProcessor.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <responses.hpp>
#include <ClientIndex.hpp>

void pass(const CommandProcessor::CommandContext &ctx)
{
    Server &server = Server::getInstance();
    Client &client = server.getClients().getByFd(ctx.clientFd);
    std::string clientPassword = ctx.params[0];

    if (client.getIsRegistered()) {
        sendToClient(ctx.clientFd, ERR_ALREADYREGISTERED(ctx.sender));
        return;
    }

    if (clientPassword.empty()) {
        sendToClient(ctx.clientFd, ERR_NEEDMOREPARAMS(ctx.sender, "PASS"));
        return;
    }

    std::string serverPassword = server.getPassword();
    if (clientPassword != serverPassword) {
        sendToClient(ctx.clientFd, ERR_PASSWDMISMATCH(ctx.sender));
        return;
    }

    client.setPasswordVerified(true);
}