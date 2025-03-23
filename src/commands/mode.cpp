#include <CommandRunner.hpp>

void CommandRunner::mode()
{
    std::string target = _params[0];
    if (CHANTYPES.find(target[0]) == std::string::npos)
        return;
    if (!_channels.channelExists(target)) {
        sendToClient(_clientFd, ERR_NOSUCHCHANNEL(_client.getNickname(), target));
        return;
    }
    Channel &channel = _channels.getChannel(target);

    std::string modeString = (_params.size() > 1) ? _params[1] : "";
    std::vector<std::string> params;
    for (size_t i = 2; i < _params.size(); ++i) {
        params.push_back(_params[i]);
    }

    if (modeString.empty())
        channel.printModes(_client);

    bool adding = true;
    size_t i = 0;
    for (char mode : modeString) {
        if (mode == '+')
            adding = true;
        else if (mode == '-')
            adding = false;
        else {
            if ( mode == 'o'|| (adding && (mode == 'k'|| mode == 'l'))) {
                if (i >= params.size()) {
                    sendToClient(_clientFd, ERR_NEEDMOREPARAMS(_client.getNickname(), "MODE"));
                    return;
                }
                IRCValidator validator;
                std::string param = params[i++];
                if (mode == 'k' &&
                    (param.empty() || !validator.isValidChannelKey(_clientFd, _nickname, param))) {
                    return;
                }
                if (mode == 'o' && nickNotFound(param))
                    return;
                if (mode == 'l' && !validator.isValidChannelLimit(param))
                    return;
                channel.setMode(_client, adding, mode, param);
            }
            else if (mode == 'i' || mode == 't' || (!adding && (mode == 'k' || mode == 'l')))
                channel.setMode(_client, adding, mode);
        }
    }
}
