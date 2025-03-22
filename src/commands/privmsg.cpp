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
                *channel = _channels.getChannel(target);
            }
            catch(const std::exception &e)
            {
                sendToClient(_clientFd, ERR_NOSUCHCHANNEL(_nickname, target));
                return;
            }

            channel->broadcastMessage(":" + _client.getPrefixPrivmsg() + " PRIVMSG " + target + " :" + _message);
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
            sendToClient(_clientFd,
                         ":" + _client.getPrefixPrivmsg() + " PRIVMSG " + target + " :" + _message);
            std::cout << "Sending message to nickname: " << target << std::endl;
        }
    }
    return;
}