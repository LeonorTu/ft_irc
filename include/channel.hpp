#pragma once

#include <string>
#include <unordered_map>

class Client;
class Server;

class Channel {
public:
    Channel(const std::string &name, Client *creator);
    ~Channel();
    void join(Client *client, std::string key = "");
    void leave(Client *client);
    void changeTopic(Client *client, std::string &newTopic);
    void giveOp(Client *creator);

    const std::string &getName() const;
    bool hasMode(const char mode) const;

private:
    std::string channelName;
    std::string topic;
    std::string topicAuthor;
    std::string topicTime;
    std::unordered_map<int, Client *> connectedClients;
    std::unordered_map<int, Client *> ops;
    std::unordered_map<std::string, Client *> invites;
    // itkl
    std::string modes;
    std::string key;
    int userLimit;

    void enableMode(char mode);
    void disableMode(char mode);
    std::string prefixNick(Client *client);
    void sendNameReply(Client *client);
    void sendTopic(Client *client);
    bool hasOp(Client *client);
    bool isInvited(Client *client);
    bool isJoinable(Client *client, std::string key);
    bool isOnChannel(Client *client);
};
