#include <Server.hpp>
#include <common.hpp>
#include <SocketManager.hpp>
#include <EventLoop.hpp>
#include <ClientIndex.hpp>
#include <ConnectionManager.hpp>
#include <responses.hpp>

Server *Server::_instance = nullptr;

Server::Server()
    : _serverFD(-1)
    , _paused(false)
    , _clients(std::make_unique<ClientIndex>())
    , _socketManager(std::make_unique<SocketManager>(SERVER_PORT))
    , _eventLoop(std::make_unique<EventLoop>())
    , _connectionManager(std::make_unique<ConnectionManager>(*_socketManager, *_eventLoop, *_clients))
    , _createdTime(getCurrentTime())
{
    // setup signalshandlers
    setInstance(this);
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
    getEventLoop().addToWatch(_serverFD, EPOLLIN | EPOLLET);
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
                sendWelcome(clientFd);
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

void Server::setInstance(Server *server)
{
    _instance = server;
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

// Send welcome messages to new client
void Server::sendWelcome(int clientFd)
{
    std::string nickname = getClients().getByFd(clientFd).getNickname();
    // Send the welcome messages
    sendToClient(clientFd, RPL_WELCOME(nickname));
    sendToClient(clientFd, RPL_YOURHOST(nickname));
    sendToClient(clientFd, RPL_CREATED(nickname, _createdTime));
    sendToClient(clientFd, RPL_MYINFO(nickname));
    // still need to include the RPL_ISUPPORT(005) messages based on server
}

void Server::shutdown()
{
    _running = false;
    _connectionManager->disconnectAllClients();
    _socketManager->closeServerSocket();
    std::cout << "Server shutdown complete" << std::endl;
}
