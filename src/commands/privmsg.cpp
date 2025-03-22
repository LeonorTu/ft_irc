#include <CommandRunner.hpp>

void CommandRunner::privmsg()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_TARGET};
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
            catch(const std::exception &e)
            {
                sendToClient(_clientFd, ERR_NOSUCHCHANNEL(_nickname, target));
                return;
            }
            channel->broadcastToOthers( _client, ":" + _client.getUserHost() + " PRIVMSG " + target + " :" + _message);
            std::cout << "Sending message to channel: " << target << std::endl;
            return;
        }
        else if (type == NICKNAME)
        {
            if (!_clients.nickExists(target)) 
            {
                sendToClient(_clientFd, ERR_NOSUCHNICK(_nickname, target));
                return;
            }
            Client &targetClient = _clients.getByNick(target);
            sendToClient(targetClient.getFd(),
                         ":" + _client.getUserHost() + " PRIVMSG " + target + " :" + _message);
            std::cout << "Sending message to nickname: " << target << std::endl;
        }
    }
    return;
}