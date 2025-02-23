#pragma once

#include <string>

class Server {
public:
    Server(int port);
    void start();
    void stop();
    void broadcastMessage(const std::string &message);

private:
    int port;
    int server_fd;
    void handleClient(int clientSocket);
};
