#include <ClientIndex.hpp>
#include <Client.hpp>

ClientIndex::~ClientIndex()
{
    _byNick.clear();
    _byFd.clear();
    std::cout << "clientIndex cleared" << std::endl;
}

void ClientIndex::add(int clientFd)
{
    if (_byFd.find(clientFd) == _byFd.end()) {
        auto inserted = _byFd.emplace(clientFd, std::make_unique<Client>(clientFd));
        bool successful = inserted.second;
        if (successful) {
            // what a fun way to get the inserted unique_ptr's pointer
            Client *client = inserted.first->second.get();
            // Client *client = _byFd[clientFd].get();
            if (client->getIsRegistered())
                _byNick[client->getNickname()] = client;
        }
    }
}

void ClientIndex::remove(Client &client)
{
    _byNick.erase(client.getNickname());
    _byFd.erase(client.getFd());
}

void ClientIndex::updateNick(const std::string &oldNick, const std::string &newNick)
{
    auto it = _byNick.find(oldNick);
    if (it == _byNick.end())
        return;

    Client *client = it->second;
    _byNick.erase(oldNick);
    _byNick.insert_or_assign(newNick, client);
}

Client &ClientIndex::getByFd(int fd) const
{
    auto it = _byFd.find(fd);
    if (it == _byFd.end()) {
        throw std::out_of_range("Client with fd " + std::to_string(fd) + " not found");
    }
    return *it->second;
}

Client &ClientIndex::getByNick(const std::string &nick) const
{
    auto it = _byNick.find(nick);
    if (it == _byNick.end()) {
        throw std::out_of_range("Client with nick " + nick + " not found");
    }
    return *it->second;
}

void ClientIndex::forEachClient(std::function<void(Client &)> callback)
{
    // Copy all file descriptors first
    std::vector<int> fds;
    for (const auto &[fd, _] : _byFd) {
        fds.push_back(fd);
    }

    // Then iterate through the copy
    for (int fd : fds) {
        // std::unordered_map<int, std::unique_ptr<Client>>::iterator it = _byFd.find(fd);
        auto it = _byFd.find(fd);
        if (it != _byFd.end()) {
            callback(*it->second);
        }
    }
}

bool ClientIndex::nickExists(const std::string &nick) const
{
    return _byNick.find(nick) != _byNick.end();
}

size_t ClientIndex::size() const
{
    return _byFd.size();
}
