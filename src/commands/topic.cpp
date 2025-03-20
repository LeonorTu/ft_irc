#include <CommandRunner.hpp>

void CommandRunner::topic()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_CHAN, VAL_TOPIC};
    if (!validateParams(1, 2, pattern))
        return;

    std::string channelName = _params[0];
    if (channelNotFound(channelName))
        return;

    Channel &channel = _channels.getChannel(channelName);
    if (_params.size() == 1) {
        channel.checkTopic(_client);
    }
    else if (_params.size() == 2) {
        std::string topic = _params[1];
        channel.changeTopic(_client, topic);
    }
}
