#include <CommandProcessor.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <responses.hpp>
#include <ClientIndex.hpp>
#include <IRCValidator.hpp>
#include <commandHandlers.hpp>
#include <ConnectionManager.hpp>

void user(const CommandProcessor::CommandContext &ctx)
{
    Server &server = Server::getInstance();
    Client &client = server.getClients().getByFd(ctx.clientFd);
    std::string nickname = client.getNickname().empty() ? "*" : client.getNickname();

    if (!client.getPasswordVerified()) {
        sendToClient(ctx.clientFd, ERR_PASSWDMISMATCH(ctx.source));
        return;
    }

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

    IRCValidator validator;
    if (validator.isValidUsername(ctx.clientFd, nickname, username) && validator.isValidRealname(ctx.clientFd, nickname, realname)) {
        client.setUsername(username);
        client.setRealname(realname);
    }

    if (!client.getIsRegistered() && client.getPasswordVerified() &&
        !client.getNickname().empty() && !client.getUsername().empty()) {
        client.setIsRegistered(true);
        server.getClients().addNick(ctx.clientFd);
        sendWelcome(ctx.clientFd);
    }
}