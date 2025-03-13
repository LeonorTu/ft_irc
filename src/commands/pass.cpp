#include <CommandProcessor.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <responses.hpp>
#include <ClientIndex.hpp>

void pass(const CommandProcessor::CommandContext &ctx)
{
    Server &server = Server::getInstance();
    Client &client = server.getClients().getByFd(ctx.clientFd);

    if (client.getIsRegistered()) {
        sendToClient(ctx.clientFd, ERR_ALREADYREGISTERED(ctx.sender));
        return;
    }

    if (ctx.password.empty()) {
        sendToClient(ctx.clientFd, ERR_NEEDMOREPARAMS(ctx.sender, "PASS"));
    }

    std::string serverPassword = server.getPassword();
    if (ctx.password != serverPassword) {
        sendToClient(ctx.clientFd, ERR_PASSWDMISMATCH(ctx.sender));
        return;
    }

    client.setPasswordVerified(true);
}