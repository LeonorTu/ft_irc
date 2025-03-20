#include <CommandRunner.hpp>

void CommandRunner::pong()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_NONE};
    if (!validateParams(1, 1, pattern))
        return;
}