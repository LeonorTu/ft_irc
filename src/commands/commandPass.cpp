#include <CommandRunner.hpp>
#include <ConnectionManager.hpp>

void CommandRunner::pass()
{
    std::array<ParamType, MAX_PARAMS> pattern = {PASS};
    if (!validateParams(1, 1, pattern))
        return;

    std::string clientPassword = _params[0];
    std::string serverPassword = Server::getInstance().getPassword();
    if (clientPassword != serverPassword) {
        sendToClient(_clientFd, ERR_PASSWDMISMATCH(_nickname));
        _client.setPasswordVerified(false);
        Server::getInstance().getConnectionManager().disconnectClient(_client);
        return;
    }
    _client.setPasswordVerified(true);
}
