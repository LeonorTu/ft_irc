#include <iostream>
#include "server/server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

int main() {
    // std::cout << "Hello" << std::endl;

    Server myserver = Server(6667);

    myserver.start();
}
