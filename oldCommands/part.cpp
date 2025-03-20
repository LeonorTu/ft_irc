#include <CommandProcessor.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <ClientIndex.hpp>
#include <commandHandlers.hpp>
#include <responses.hpp>
#include <sstream>
#include <ChannelManager.hpp>

void part(const CommandProcessor::CommandContext &ctx)
{
    Server &server = Server::getInstance();
    ClientIndex &clients = server.getClients();
    Client &client = clients.getByFd(ctx.clientFd);
    ChannelManager &channels = server.getChannels();

    if (!client.getIsRegistered()) {
        sendToClient(ctx.clientFd, ERR_NOTREGISTERED(client.getNickname()));
        return;
    }
    if (ctx.params.empty()) {
        sendToClient(ctx.clientFd, ERR_NEEDMOREPARAMS(client.getNickname(), "PART"));
        return;
    }

    std::istringstream channelList(ctx.params[0]);
    std::string channelName;
    std::string reason = ctx.params.size() >= 2 ? ctx.params[1] : "";
    while (std::getline(channelList, channelName, ',')) {
        if (!channelName.empty()) {
            if (channels.channelExists(channelName)) {
                Channel &channel = channels.getChannel(channelName);
                channel.part(client, reason);
            }
            else
                sendToClient(ctx.clientFd, ERR_NOSUCHCHANNEL(client.getNickname(), channelName));
        }
    }
}