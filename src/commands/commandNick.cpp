#include <CommandRunner.hpp>

void CommandRunner::nick()
{
    std::array<ParamType, MAX_PARAMS> pattern = {NICK};
    if (!validateParams(1, 1, pattern))
        return;

    std::string &oldNickname = _nickname;
    std::string &newNickname = _params[0];
}
