#include <server.hpp>
#include <common.hpp>
#include <SocketManager.hpp>
#include <EventLoop.hpp>
#include <ClientIndex.hpp>
#include <ConnectionManager.hpp>

Server *Server::instance = nullptr;

Server::Server()
    : serverFD(-1)
    , paused(false)
    , _clients(std::make_unique<ClientIndex>())
    , _socketManager(std::make_unique<SocketManager>(SERVER_PORT))
    , _eventLoop(std::make_unique<EventLoop>())
    , _connectionManager(std::make_unique<ConnectionManager>(*_socketManager, *_eventLoop, *_clients))
{
    // get current time for server start time with chrono
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%a %d %b %H:%M:%S %Y", std::localtime(&now_time_t));
    createdTime = std::string(buffer);

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
    serverFD = getSocketManager().initialize();
    getEventLoop().addToWatch(serverFD, EPOLLIN | EPOLLET);
    running = true;
    loop();
}

void Server::loop()
{
    while (running) {
        std::vector<Event> events = getEventLoop().waitForEvents(100);
        for (const Event &event : events) {
            if (event.fd == serverFD) {
                int clientFd = getConnectionManager().handleNewClient();
                sendWelcome(clientFd);
            }
            else {
                getConnectionManager().recieveData(event.fd);
            }
        }
        if (paused) {
            std::cout << "Server paused. Waiting for SIGTSTP to resume..." << std::endl;
            while (paused && running) {
                sleep(1);
            }
            std::cout << "Server resumed!" << std::endl;
        }
    }
}

void Server::stop()
{
    running = false;
}

const int Server::getServerFD() const
{
    return this->serverFD;
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
    instance = server;
}

void Server::pause()
{
    paused = true;
    std::cout << "Server pausing..." << std::endl;
}

void Server::resume()
{
    paused = false;
    std::cout << "Server resuming..." << std::endl;
}

void Server::signalHandler(int signum)
{
    if (instance) {
        if (signum == SIGTSTP) {
            if (instance->paused) {
                instance->resume();
            }
            else
                instance->pause();
        }
        else {
            std::cout << "\nCaught signal " << signum << std::endl;
            instance->stop();
        }
    }
}

// ":<source> <command/REPL> <parameters> <crlf>"
void Server::sendWelcome(int clientFD)
{
    // 001 RPL_WELCOME
    std::stringstream welcome;
    welcome << ":" << SERVER_NAME << " 001 " << clientFD << " :Welcome to " << NETWORK_NAME << " Network, " << clientFD
            << "\r\n";
    send(clientFD, welcome.str().c_str(), welcome.str().size(), 0);
    // 002 RPL_YOURHOST
    std::stringstream yourHost;
    yourHost << ":" << SERVER_NAME << " 002 " << clientFD << " :Your host is " << SERVER_NAME << ", running version "
             << SERVER_VERSION << "\r\n";
    send(clientFD, yourHost.str().c_str(), yourHost.str().size(), 0);
    // 003 RPL_CREATED
    std::stringstream created;
    created << ":" << SERVER_NAME << " 003 " << clientFD << " :This server was created " << createdTime << "\r\n";
    send(clientFD, created.str().c_str(), created.str().size(), 0);
    // 004 RPL_MYINFO
    std::stringstream myInfo;
    myInfo << ":" << SERVER_NAME << " 004 " << clientFD << " " << SERVER_NAME << " " << SERVER_VERSION << " "
           << USER_MODES << " " << CHANNEL_MODES << "\r\n";
    send(clientFD, myInfo.str().c_str(), myInfo.str().size(), 0);
}

void Server::shutdown()
{
    running = false;
    _connectionManager->disconnectAllClients();
    _socketManager->closeServerSocket();
    std::cout << "Server shutdown complete" << std::endl;
}
