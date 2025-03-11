#include <iostream>
#include <Server.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

int main()
{
    // std::cout << "Hello" << std::endl;

    Server myserver;

    myserver.start();
}
