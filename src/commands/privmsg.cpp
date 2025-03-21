#include <CommandRunner.hpp>

void CommandRunner::privmsg()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_TARGET};
    if (!validateParams(2, 62, pattern))
        return;
}


