#include <iostream>
#include <Server.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <IRCValidator.hpp>
#include <Error.hpp>

int main(int argc, char *argv[])
{
    std::string portStr;
    std::string password;
    if (argc == 1) {
        std::cerr << "running with defaults, port: 6667 and password 42" << std::endl;
        portStr = "6667";
        password = "42";
    }
    else if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    else {
        portStr = argv[1];
        password = argv[2];
    }
    IRCValidator validator;
    if (!validator.isValidPort(portStr)) {
        std::cerr << "Invalid port. Port must be an integer in the range 1-65535." << std::endl;
        return 1;
    }
    int port = std::stoi(portStr);
    if (!validator.isValidServerPassword(password)) {
        std::cerr << "Invalid password. Password must be 2-32 characters long and contain only "
                     "printable characters without spaces."
                  << std::endl;
        return 1;
    }
    try {
        Server myserver(port, password, true);
    }
    catch(const ServerError &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
