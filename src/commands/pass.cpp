#include <CommandProcessor>
#include <Client>
#include <responses>

void pass(const CommandProcessor::CommandContext &ctx)
{
    Client *client;
    if (client->getIsRegistered()) {
        sendToClient(client->getFd(), ERR_ALREADYREGISTERED(ctx.sender));
        return;
    }
    if (ctx.pass.empty()) {
        sendToClient(client->getFd(), ERR_NEEDMOREPARAMS(ctx.sender, "pass"));
    }

    std::string serverPassword = server.getPassword();
    if(ctx.pass != serverPassword)
    {
        sentToClient(client->getFd(), ERR_PASSWDMISMATCH(ctx.sender));
    }
    client->setPasswordVerified(true);
}