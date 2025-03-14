#include <CommandProcessor.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <responses.hpp>
#include <ClientIndex.hpp>

void user(const CommandProcessor::CommandContext &ctx)
{
    Server &server = Server::getInstance();
    Client &client = server.getClients().getByFd(ctx.clientFd);
    std::string nickname = client.getNickname().empty() ? "*" : client.getNickname();

    if (client.getIsRegistered()) {
        sendToClient(ctx.clientFd, ERR_ALREADYREGISTERED(nickname));
        return;
    }
    if (ctx.params.size() < 4) {
        sendToClient(ctx.clientFd, ERR_NEEDMOREPARAMS(nickname, "USER"));
        return;
    }

    std::string username = ctx.params[0];
    std::string realname = ctx.params[3];
    client.setUsername(username);
    client.setRealname(realname);
}