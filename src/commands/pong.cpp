#include <CommandRunner.hpp>

void CommandRunner::handlePongFromClient(const std::string &token)
{
    auto it = _pingPongManager._pingTokens.find(token);
    if (it != _pingTokens.end()) {
        // auto now = std::chrono::steady_clock::now();
        // auto pingTime = it->second;
        // auto duration =
        // std::chrono::duration_cast<std::chrono::milliseconds>(now - pingTime).count();
        // std::cout << "PONG received from " << _nickname << " in " << duration << "ms" <<
        // std::endl;
        _pingTokens.erase(it);
    }
}


void CommandRunner::pong()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_NONE};
    if (!validateParams(1, 1, pattern))
        return;
}