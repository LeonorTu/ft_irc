#include <CommandRunner.hpp>
#include <Error.hpp>

void CommandRunner::privmsg()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_TARGET, VAL_TEXT};
    if (!validateParams(2, 2, pattern))
        return;

    for (auto &[type,target] : _targets)
    {
        if(type == CHANNEL)
        {
            Channel *channel = nullptr;
            try{
                channel = &_channels.getChannel(target);
            }
            catch(const ChannelNotFound &e)
            {
                sendToClient(_clientFd, ERR_NOSUCHCHANNEL(_nickname, target));
                continue;
            }
            channel->broadcastToOthers(_client, ":" + _client.getUserHost() + " PRIVMSG " +
                                                    target + " :" + _message);
            std::cout << "Sending message to channel: " << target << std::endl;
        }
        else if (type == NICKNAME)
        {
            if (!_clients.nickExists(target))
            {
                sendToClient(_clientFd, ERR_NOSUCHNICK(_nickname, target));
                continue;;
            }
            Client &targetClient = _clients.getByNick(target);
            sendToClient(targetClient.getFd(),
                         ":" + _client.getUserHost() + " PRIVMSG " + target + " :" + _message);
            std::cout << "Sending message to nickname: " << target << std::endl;
        }
    }
}