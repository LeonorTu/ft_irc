#pragma once

#include <string>
#include <unordered_map>

class Client;
class Server;

enum ChannelMode
{
    INVITE_ONLY = 'i',
    KEY = 'k',
    LIMIT = 'l',
    PROTECTED_TOPIC = 't',
    OP = 'o'
};

class Channel
{
public:
    Channel(const std::string &name, Client *creator);
    ~Channel();
    void join(Client *client, std::string key = "");
    void leave(Client *client);
    void changeTopic(Client *client, std::string &newTopic);
    const std::string &getName() const;
    bool hasMode(ChannelMode mode) const;
    bool hasMode(const char mode) const;
    void setMode(Client *client, bool enable, ChannelMode mode, std::string param = "");

private:
    std::string channelName;
    std::string topic;
    std::string topicAuthor;
    std::string topicTime;
    std::unordered_map<std::string, Client *> connectedClients;
    std::unordered_map<std::string, Client *> ops;
    std::unordered_map<std::string, Client *> invites;
    // itkl
    std::string modes;
    std::string key;
    int userLimit;

    void broadcastMessage(std::string &message);
    void enableMode(ChannelMode mode);
    void disableMode(ChannelMode mode);
    std::string prefixNick(Client *client);
    void sendNameReply(Client *client);
    void sendTopic(Client *client);
    bool hasOp(Client *client);
    bool isInvited(Client *client);
    bool isJoinable(Client *client, std::string key);
    bool isOnChannel(Client *client);
    void removeFromInvites(Client *client);
};
