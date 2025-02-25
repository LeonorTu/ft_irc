#pragma once

#include <string>

class Client;

class Channel {
public:
    Channel(const std::string &name);
    void addClient(Client *client);
    void removeClient(Client *client);
    void sendMessageToChannel(const std::string &message);

private:
    std::string name;
};
