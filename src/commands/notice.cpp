#include <CommandRunner.hpp>
#include <Error.hpp>

void CommandRunner::notice()
{
    if (_params.size() != 2)
        return;
    _targets = splitTargets(_params[0]);
    _message = _params[1];
    for (auto &[type, target] : _targets) {
        if (type == CHANNEL) {
            if (!_channels.channelExists(target))
                continue;
            Channel &channel = _channels.getChannel(target);
            channel.broadcastToOthers(_client,
                                      NOTICE(_client.getUserHost(), channel.getName(), _message));
        }
        else if (type == NICKNAME) {
            if (!_clients.nickExists(target))
                continue;
            Client &targetClient = _clients.getByNick(target);
            sendToClient(targetClient.getFd(),
                         NOTICE(_client.getUserHost(), targetClient.getNickname(), _message));
        }
    }
}
