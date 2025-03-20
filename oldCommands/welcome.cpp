#include <string>
#include <responses.hpp>
#include <Server.hpp>
#include <Client.hpp>
#include <ClientIndex.hpp>


// Send welcome messages to new client
void sendWelcome(int clientFd)
{
    Server &server = Server::getInstance();
    std::string nickname = server.getClients().getByFd(clientFd).getNickname();
    // Send the welcome messages
    sendToClient(clientFd, RPL_WELCOME(nickname));
    sendToClient(clientFd, RPL_YOURHOST(nickname));
    sendToClient(clientFd, RPL_CREATED(nickname, server.getCreatedTime()));
    sendToClient(clientFd, RPL_MYINFO(nickname));
    sendToClient(clientFd, RPL_ISUPPORT(nickname));
    /*
    still need to include the RPL_ISUPPORT(005) messages based on server
    I guess we could have all these in the common.hpp files ince its like a settings file.
    CASEMAPPING=ascii //could also add to common.hpp
    CHANNELLEN=(read from CHANNEL_NAME_MAX in common.hpp)
    CHANLIMIT=#&50 /example add to common.hpp
    CHANTYPES=#&
    //

    Type A: Modes that add or remove a user address to a list (always takes a parameter)
    Type B: Modes that change a channel setting and always have a parameter
    Type C: Modes that change a channel setting and only have a parameter when set
    Type D: Modes that change a channel setting and never have a parameter
    maybe need to format the modes more corretly in the common.hpp also
    Format: CHANMODES=Atypes,Btypes,Ctypes,Dtypes
    so for us I think its k and l type C, i and t type D
    CHANMODES=,,kl,it CHANNEL_MODES const on common.hpp

    PREFIX=o(@) // just one we have op means @ sightn in front of name I guess
    MODES=3 //standard value for max modes per command
    NICKLEN=30 read from common.hpp, 30 or 31 is typical
    TOPICLEN=307 add to common.hpp, 307 is a typical length
    USERLEN=12 add to common.hpp, I see 12 or 18 as typical values.

    */
}
