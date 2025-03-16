#include <CommandProcessor.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <ClientIndex.hpp>
#include <commandHandlers.hpp>
#include <responses.hpp>
#include <ChannelManager.hpp>
#include <IRCValidator.hpp>

void topic(const CommandProcessor::CommandContext &ctx)
{
    Server &server = Server::getInstance();
    ClientIndex &clients = server.getClients();
    Client &client = clients.getByFd(ctx.clientFd);
    ChannelManager &channels = server.getChannels();

    if (ctx.params.empty()) {
        sendToClient(ctx.clientFd, ERR_NEEDMOREPARAMS(client.getNickname(), "JOIN"));
        return;
    }
    std::string channelName = ctx.params[0];
    if (!IRCValidator::isValidChannelName(ctx.clientFd, client.getNickname(), channelName))
        return;
    if (!channels.channelExists(channelName))
        sendToClient(ctx.clientFd, ERR_NOSUCHCHANNEL(client.getNickname(), channelName));
    Channel &channel = channels.getChannel(ctx.params[0]);
    if (ctx.params.size() == 1) {
        channel.checkTopic(client);
    }
    else if (ctx.params.size() == 2) {
        std::string topic = ctx.params[1];
        channel.changeTopic(client, topic);
    }
}