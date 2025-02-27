#pragma once

#include <string>
#include <unordered_map>

class Client;
class Server;

class Channel {
public:
    Channel();
    Channel(const std::string &name, Client *creator);
    ~Channel();
    void join(Client *client, std::string key = "");
    void leave(Client *client);
    void message(const std::string &message);
    void toggleMode(char mode);
    void giveOp(Client *creator);

    const std::string &getName() const;
    bool hasMode(const char mode) const;

private:
    std::string name;
    std::string topic;
    std::unordered_map<int, Client *> connectedClients;
    std::unordered_map<int, Client *> ops;
    // itkl
    std::string modes;
    std::string key;
    int userLimit;
};
