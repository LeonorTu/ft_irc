#include <CommandProcessor.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <ClientIndex.hpp>
#include <commandHandlers.hpp>
#include <responses.hpp>
#include <ChannelManager.hpp>
#include <IRCValidator.hpp>
#include <common.hpp>

void join(const CommandProcessor::CommandContext &ctx)
{
    Server &server = Server::getInstance();
    ClientIndex &clients = server.getClients();
    Client &client = clients.getByFd(ctx.clientFd);
    ChannelManager &channels = server.getChannels();

    if (!client.getIsRegistered()) {
        sendToClient(ctx.clientFd, ERR_NOTREGISTERED(client.getNickname()));
    }
    if (ctx.params.empty()) {
        sendToClient(ctx.clientFd, ERR_NEEDMOREPARAMS(client.getNickname(), "JOIN"));
        return;
    }

    if (ctx.params[0] == "0") {
        std::unordered_map<std::string, Channel *> channels = client.getMyChannels();
        for (auto &pair : channels) {
            Channel *channel = pair.second;
            channel->part(client, "Client is leaving all the channels");
        }
        return;
    }

    std::istringstream channelList(ctx.params[0]);
    std::istringstream keyList(ctx.params.size() > 1 ? ctx.params[1] : "");
    std::string channelName;
    std::string key;
    IRCValidator validator;
    int local = 0;
    int regular = 0;
    while (std::getline(channelList, channelName, ',') && std::getline(keyList, key, ',')) {
        key = key.empty() ? "" : key;
        if (!channelName.empty()) {
            if (!validator.isValidChannelName(ctx.clientFd, client.getNickname(), channelName) &&
                !channels.channelExists(channelName)) {
                sendToClient(ctx.clientFd, ERR_NOSUCHCHANNEL(client.getNickname(), channelName));
                continue;
            }
            Channel &channel = channels.getChannel(channelName);
            if (channelName[0] == '#' && regular <= REGCHANLMAX) {
                channel.join(client, key);
                regular++;
            }
            else if (channelName[0] == '&' && local <= LOCCHANLMAX) {
                channel.join(client, key);
                local++;
            }
            else {
                sendToClient(ctx.clientFd, ERR_TOOMANYCHANNELS(client.getNickname(), channelName));
                continue;
            }
        }
    }
}