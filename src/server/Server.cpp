#include <Server.hpp>
#include <common.hpp>
#include <SocketManager.hpp>
#include <EventLoop.hpp>
#include <ClientIndex.hpp>
#include <ChannelManager.hpp>
#include <ConnectionManager.hpp>
#include <responses.hpp>

Server *Server::_instance = nullptr;

Server::Server()
    : _serverFD(-1)
    , _password("42")
    , _paused(false)
    , _clients(std::make_unique<ClientIndex>())
    , _channels(std::make_unique<ChannelManager>())
    , _socketManager(std::make_unique<SocketManager>(SERVER_PORT))
    , _eventLoop(createEventLoop())
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
}

Server::~Server()
{
    shutdown();
}

void Server::start()
{
    _serverFD = getSocketManager().initialize();
    getEventLoop().addToWatch(_serverFD);
    _running = true;
    loop();
}

void Server::loop()
{
    while (_running) {
        std::vector<Event> events = getEventLoop().waitForEvents(100);
        for (const Event &event : events) {
            if (event.fd == _serverFD) {
                int clientFd = getConnectionManager().handleNewClient();
                // sendWelcome(clientFd);
            }
            else {
                getConnectionManager().recieveData(event.fd);
            }
        }
        if (_paused) {
            std::cout << "Server paused. Waiting for SIGTSTP to resume..." << std::endl;
            while (_paused && _running) {
                sleep(1);
            }
            std::cout << "Server resumed!" << std::endl;
        }
    }
}

Server &Server::getInstance()
{
    return *_instance;
}

const int Server::getServerFD() const
{
    return this->_serverFD;
}

const bool Server::getIsPaused() const
{
    return this->_paused;
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
    _connectionManager->disconnectAllClients();
    _socketManager->closeServerSocket();
    std::cout << "Server shutdown complete" << std::endl;
}
