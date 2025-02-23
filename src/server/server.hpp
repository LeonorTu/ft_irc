#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <iostream>
#include <arpa/inet.h>

class Client;
class Server {
public:
    Server();
    ~Server();
    void start();
    void loop();
    void stop();
    void broadcastMessage(const std::string &message);

    // getters
    const int getServerFD() const;
    const int getPort() const;

private:
    bool running;
    int serverFD;
    int port;
    sockaddr_in serverAddress;
    std::vector<pollfd> pollFDs;
    std::unordered_map<int, Client *> clients;

    void handleNewClient(int clientSocket);
    void parseMessage(Client *from);
    void cleanup();
};
