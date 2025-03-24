#include <Server.hpp>
#include <common.hpp>
#include <SocketManager.hpp>
#include <EventLoop.hpp>
#include <ClientIndex.hpp>
#include <ChannelManager.hpp>
#include <ConnectionManager.hpp>
#include <responses.hpp>
#include <PongManager.hpp>
#include <Error.hpp>


Server *Server::_instance = nullptr;

Server::Server(int port, std::string password, bool startBlocking)
    : _running(false)
    , _paused(false)
    , _serverFd(-1)
    , _port(port)
    , _password(password)
    , _clients(std::make_unique<ClientIndex>())
    , _channels(std::make_unique<ChannelManager>())
    , _socketManager(std::make_unique<SocketManager>(_port))
    , _eventLoop(createEventLoop())
    , _PongManager(std::make_unique<PongManager>())
    , _connectionManager(
          std::make_unique<ConnectionManager>(*_socketManager, *_eventLoop, *_clients))
    , _createdTime(getCurrentTime())
{
    // setup signalshandlers
    _instance = this;
    signal(SIGINT, signalHandler);  // Handle Ctrl+C
    signal(SIGTERM, signalHandler); // Handle termination request
    signal(SIGTSTP, signalHandler); // handle server pause
    signal(SIGPIPE, SIG_IGN);       // Ignore SIGPIPE (broken pipe)

    _serverFd = getSocketManager().initialize();
    if (_serverFd < 0) {
        // std::cerr << "server failed to start" << std::endl;
        throw ServerError("Server failed to start");
        return;
    }
    getEventLoop().addToWatch(_serverFd);
    if (startBlocking) {
        loop();
    }
}

Server::~Server() noexcept
{
    _connectionManager->cleanUp();
    _socketManager->closeServerSocket();
    try
    {
        _eventLoop->removeFromWatch(_serverFd);
    }
    catch(const EventError& e)
    {
        std::cerr << e.what() << '\n';
    }
    std::cout << "Server shutdown complete" << std::endl;
}

void Server::loop()
{
    _running = true;
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count();
    ConnectionManager &connection = getConnectionManager();
    while (_running) {
        std::vector<Event> events = getEventLoop().waitForEvents(100);
        for (const Event &event : events) {
            if (event.fd == _serverFd) {
                Error::handleNewClientError(connection); 
            }
            else {
                Error::receiveDataError(connection, event.fd);
            }
        }
        pingSchedule(now);
        getConnectionManager().rmDisconnectedClients();
        getChannels().rmEmptyChannels();
        if (_paused) {
            std::cout << "Server paused. Waiting for SIGTSTP to resume..." << std::endl;
            while (_paused && _running) {
                sleep(1);
            }
            std::cout << "Server resumed!" << std::endl;
        }
    }
}

// void Server::sendPingToInactivityClients(int timeoutMs, const int pingTimeout)
// {
//     // Check for inactive clients and send PING if needed
//     getClients().forEachClient([this, timeoutMs](Client &client) {
//         if (client.getTimeForNoActivity() > timeoutMs) {
//             std::cout << "Client " << client.getNickname() << " inactive for "
//                       << client.getTimeForNoActivity() / 1000 << " seconds. Sending PING."
//                       << std::endl;
//             getPongManager().sendPingToClient(client);
//         }
//     });
// }

void Server::pingSchedule(int64_t &last_ping)
{
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count();
    const int pingCheckInterval = 120 * 1000;
    const int pingTimeout = 60 * 1000;
    // const int inactivityTimeout = 120 * 1000;
    if (now - last_ping > pingCheckInterval) {
        getPongManager().sendPingToAllClients(*_clients);
        last_ping = now;
    }
    // getConnectionManager().checkInactivityClients(inactivityTimeout);
    getPongManager().checkAllPingTimeouts(pingTimeout, *_clients, *_connectionManager);
}

Server &Server::getInstance()
{
    return *_instance;
}

int Server::getServerFD() const
{
    return this->_serverFd;
}

ClientIndex &Server::getClients()
{
    return *_clients;
}

ChannelManager &Server::getChannels()
{
    return *_channels;
}

SocketManager &Server::getSocketManager()
{
    return *_socketManager;
}

EventLoop &Server::getEventLoop()
{
    return *_eventLoop;
}

PongManager &Server::getPongManager()
{
    return *_PongManager;
}

ConnectionManager &Server::getConnectionManager()
{
    return *_connectionManager;
}

const std::string &Server::getPassword()
{
    return _password;
}

const std::string &Server::getCreatedTime()
{
    return _createdTime;
}

void Server::pause()
{
    _paused = true;
    std::cout << "Server pausing..." << std::endl;
}

void Server::resume()
{
    _paused = false;
    std::cout << "Server resuming..." << std::endl;
}

void Server::signalHandler(int signum)
{
    if (_instance) {
        if (signum == SIGTSTP) {
            if (_instance->_paused) {
                _instance->_paused = false;
                std::cout << "Server resuming..." << std::endl;
            }
            else {
                _instance->_paused = true;
                std::cout << "Server pausing..." << std::endl;
            }
        }
        else {
            std::cout << "\nCaught signal " << signum << std::endl;
            _instance->_running = false;
        }
    }
}

void Server::shutdown()
{
    _running = false;
}
