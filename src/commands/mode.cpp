#include <CommandRunner.hpp>

void CommandRunner::mode()
{
    std::string target = _params[0];
    std::string modeString = (_params.size() > 1) ? _params[1] : "";
    std::string param = (_params.size() > 2) ? _params[2] : "";

    if (!_channels.channelExists(target)) {
        sendToClient(_clientFd, ERR_NOSUCHCHANNEL(_client.getNickname(), target));
    }

    Channel &channel = _channels.getChannel(target);
    bool adding = true;
    size_t argIndex = 0;
    for (char mode : modeString) {
        if (mode == '+')
            adding = true;
        else if (mode == '-')
            adding = false;
        else
            channel.setMode(_client, adding, mode, param);
    }
}