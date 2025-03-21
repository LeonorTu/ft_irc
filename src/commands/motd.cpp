#include <CommandRunner.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <responses.hpp>
#include <common.hpp>

void CommandRunner::motd()
{
    sendToClient(_clientFd, RPL_MOTDSTART(_nickname));
	for (const std::string &line : MOTD_LINES)
	{
		sendToClient(_clientFd, RPL_MOTD(_nickname, line));
	}
    sendToClient(_clientFd, RPL_ENDOFMOTD(_nickname));
}
