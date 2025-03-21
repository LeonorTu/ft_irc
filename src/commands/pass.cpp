#include <CommandRunner.hpp>

void CommandRunner::pass()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_PASS};
    if (!validateParams(1, 1, pattern))
        return;

    std::string clientPassword = _params[0];
    std::string serverPassword = Server::getInstance().getPassword();
    if (clientPassword != serverPassword) {
        sendToClient(_clientFd, ERR_PASSWDMISMATCH(_nickname));
        _client.setPasswordVerified(false);
        _server.getConnectionManager().disconnectClient(_client, "Wrong password");
        return;
    }
    _client.setPasswordVerified(true);
}
