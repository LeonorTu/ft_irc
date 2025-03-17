#include <CommandRunner.hpp>

void CommandRunner::nick()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_NICK};
    if (!validateParams(1, 1, pattern))
        return;

    const std::string &oldNickname = _nickname;
    std::string &newNickname = _params[0];

    if (_clients.nickExists(newNickname))
        sendToClient(_clientFd, ERR_NICKNAMEINUSE(oldNickname, newNickname));

    _client.setNickname(newNickname);
    if (tryRegisterClient())
        _clients.addNick(_clientFd);
}
