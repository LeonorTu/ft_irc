#include <CommandProcessor.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <ClientIndex.hpp>
#include <commandHandlers.hpp>
#include <responses.hpp>
#include <ChannelManager.hpp>

void quit(const CommandProcessor::CommandContext &ctx)
{
    Server &server = Server::getInstance();
    ClientIndex &clients = server.getClients();
    Client &client = clients.getByFd(ctx.clientFd);
    std::unordered_map<std::string, Channel *> channels = client.getMyChannels();

    if (!client.getIsRegistered()) {
        sendToClient(ctx.clientFd, ERR_NOTREGISTERED(client.getNickname()));
        return;
    }
    std::string reason = ctx.params.empty() ? "" : ctx.params[0];

    for (auto &pair : channels) {
        Channel *channel = pair.second;
        channel->quit(client, reason);
    }
}