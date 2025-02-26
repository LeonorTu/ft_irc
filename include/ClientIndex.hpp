#pragma once

#include <string>
#include <unordered_map>

class Client;

class ClientIndex {
public:
    ClientIndex() = default;
    ~ClientIndex();

    // Core operations
    void add(Client *client);
    void remove(Client *client);
    void updateNick(const std::string &oldNick, const std::string &newNick);
    void addUnregistered(Client *client);

    // Lookup functions
    Client *getByFd(int fd) const;
    Client *getByNick(const std::string &nick) const;

    std::unordered_map<int, Client *> &getClientsForCleanup();

    // Utility functions
    bool nickExists(const std::string &nick) const;
    size_t size() const;

private:
    std::unordered_map<int, Client *> byFd;
    std::unordered_map<std::string, Client *> byNick;
};
