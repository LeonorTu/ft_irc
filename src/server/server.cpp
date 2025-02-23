#include "server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

Server::Server(int port) : port(port), server_fd(-1) {
}

void Server::start() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(this->port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    listen(server_fd, -1);

    sockaddr_in client_fd;
    socklen_t client_len = sizeof(client_fd);

    while (true) {
        int clientSocket =
            accept(server_fd, (struct sockaddr *)&client_fd, &client_len);

        char buffer[1024] = {0};
        recv(clientSocket, buffer, sizeof(buffer), 0);

        std::cout << "message from client: " << buffer << std::endl;
    }
    close(server_fd);
}
