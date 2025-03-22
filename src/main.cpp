#include <iostream>
#include <Server.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <IRCValidator.hpp>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    std::string portStr = argv[1];
    std::string password = argv[2];
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

    Server myserver(port, password);

    return 0;
}
