#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

class Client;

class ClientIndex
{
public:
    ClientIndex() = default;
    ~ClientIndex();

    // Core operations
    void add(int clientFd);
    void remove(Client &client);
    void updateNick(const std::string &oldNick, const std::string &newNick);

    // Lookup functions
    Client &getByFd(int fd) const;
    Client &getByNick(const std::string &nick) const;

    // Utility functions
    void forEachClient(std::function<void(Client &)> callback);
    bool nickExists(const std::string &nick) const;
    size_t size() const;

private:
    // std::unordered_map<int, Client *> byFd;
    std::unordered_map<int, std::unique_ptr<Client>> _byFd;
    std::unordered_map<std::string, Client *> _byNick;
};
