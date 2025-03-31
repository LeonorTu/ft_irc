#include "IRCBot.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <thread>
#include <chrono>
#include <netdb.h>

IRCBot::IRCBot(const std::string &server, int port, const std::string &password,
               const std::string &nickname, const std::string &channel)
    : _server(server)
    , _port(port)
    , _password(password)
    , _nickname(nickname)
    , _channel(channel)
    , _socket(-1)
    , _connected(false)
{
    std::cout << "[DEBUG] Creating IRC Bot: " << nickname << " for server " << server << ":" << port
              << std::endl;
}

IRCBot::~IRCBot()
{
    std::cout << "[DEBUG] Destroying IRC Bot" << std::endl;
    disconnect();
}

bool IRCBot::connect()
{
    std::cout << "[DEBUG] Connecting to server: " << _server << ":" << _port << std::endl;

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0) {
        std::cerr << "[ERROR] Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }
    std::cout << "[DEBUG] Socket created successfully: " << _socket << std::endl;

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(_port);

    // Try different ways to resolve the hostname
    if (inet_addr(_server.c_str()) != INADDR_NONE) {
        // It's a valid IP address
        serverAddr.sin_addr.s_addr = inet_addr(_server.c_str());
        std::cout << "[DEBUG] Using direct IP address: " << _server << std::endl;
    }
    else {
        // It's a hostname that needs to be resolved
        struct hostent *host = gethostbyname(_server.c_str());
        if (!host) {
            std::cerr << "[ERROR] Failed to resolve hostname: " << _server
                      << ", error: " << strerror(errno) << std::endl;
            close(_socket);
            _socket = -1;
            return false;
        }
        memcpy(&serverAddr.sin_addr, host->h_addr, host->h_length);
        std::cout << "[DEBUG] Resolved hostname " << _server
                  << " to IP: " << inet_ntoa(*(struct in_addr *)host->h_addr) << std::endl;
    }

    std::cout << "[DEBUG] Attempting connection to " << inet_ntoa(serverAddr.sin_addr) << ":"
              << ntohs(serverAddr.sin_port) << std::endl;

    if (::connect(_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[ERROR] Failed to connect to server: " << strerror(errno) << std::endl;
        close(_socket);
        _socket = -1;
        return false;
    }

    std::cout << "[DEBUG] Connected successfully. Sending registration commands." << std::endl;

    // Send registration commands
    sendRaw("PASS " + _password);
    sendRaw("NICK " + _nickname);
    sendRaw("USER " + _nickname + " 0 * :IRC Bot");

    // Give server time to process registration
    std::cout << "[DEBUG] Waiting for server to process registration..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Join channel
    std::cout << "[DEBUG] Joining channel: " << _channel << std::endl;
    sendRaw("JOIN " + _channel);

    _connected = true;
    std::cout << "[DEBUG] Connection and registration completed." << std::endl;
    return true;
}

void IRCBot::disconnect()
{
    if (_socket >= 0) {
        std::cout << "[DEBUG] Disconnecting from server." << std::endl;
        sendRaw("QUIT :Bot shutting down");
        close(_socket);
        _socket = -1;
        _connected = false;
    }
}

void IRCBot::sendMessage(const std::string &message)
{
    if (!_connected) {
        std::cout << "[DEBUG] Not connected, attempting to reconnect..." << std::endl;
        if (!reconnect()) {
            std::cerr << "[ERROR] Failed to reconnect, message not sent: " << message << std::endl;
            return;
        }
    }

    std::cout << "[DEBUG] Sending message to " << _channel << ": " << message << std::endl;
    sendRaw("PRIVMSG " + _channel + " :" + message);
}

void IRCBot::handleMessage(const std::string &message)
{
    std::cout << "[DEBUG] Handling message: " << message << std::endl;
}

bool IRCBot::isConnected() const
{
    return _connected;
}

bool IRCBot::processServerMessages()
{
    if (!_connected) {
        std::cout << "[DEBUG] Not connected, can't process server messages." << std::endl;
        return false;
    }

    struct pollfd fds[1];
    fds[0].fd = _socket;
    fds[0].events = POLLIN;

    // Quick poll to check for server messages
    int ret = poll(fds, 1, 0);

    if (ret > 0 && (fds[0].revents & POLLIN)) {
        char buffer[1024];
        int bytesRead = recv(_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead <= 0) {
            std::cerr << "[ERROR] Connection closed by server: "
                      << (bytesRead == 0 ? "graceful close" : strerror(errno)) << std::endl;
            _connected = false;
            close(_socket);
            _socket = -1;
            return false;
        }

        buffer[bytesRead] = '\0';
        std::string response(buffer);

        std::cout << "[DEBUG] Received from server (" << bytesRead << " bytes): " << response
                  << std::endl;

        // Process PING messages
        if (response.find("PING") != std::string::npos) {
            std::string pong = "PONG" + response.substr(response.find("PING") + 4);
            pong = pong.substr(0, pong.find("\r\n"));
            std::cout << "[DEBUG] Responding to PING with: " << pong << std::endl;
            sendRaw(pong);
        }

        // Check for JOIN messages
        if (response.find("JOIN") != std::string::npos) {
            std::cout << "[DEBUG] JOIN message detected!" << std::endl;
            handleMessage(response);
        }

        // Parse for PRIVMSG and call the handler
        size_t pos = response.find("PRIVMSG " + _channel);
        if (pos != std::string::npos) {
            // Extract sender
            size_t nickStart = response.find(":") + 1;
            size_t nickEnd = response.find("!", nickStart);
            std::string sender = response.substr(nickStart, nickEnd - nickStart);

            // Extract message
            size_t msgStart = response.find(":", pos) + 1;
            std::string message = response.substr(msgStart);
            message = message.substr(0, message.find("\r\n"));

            std::cout << "[DEBUG] PRIVMSG from " << sender << ": " << message << std::endl;

            // Call the handler
            handleMessage(response);
        }
    }

    return true;
}

bool IRCBot::reconnect()
{
    std::cout << "[DEBUG] Attempting to reconnect..." << std::endl;
    if (_socket >= 0) {
        close(_socket);
        _socket = -1;
    }

    // Try to reconnect
    return connect();
}

void IRCBot::sendRaw(const std::string &command)
{
    if (_socket < 0) {
        std::cerr << "[ERROR] Socket not valid, cannot send: " << command << std::endl;
        return;
    }

    std::string fullCommand = command + "\r\n";
    std::cout << "[DEBUG] Sending raw command: " << command << std::endl;

    int sent = send(_socket, fullCommand.c_str(), fullCommand.length(), 0);

    if (sent < 0) {
        std::cerr << "[ERROR] Failed to send command: " << strerror(errno) << std::endl;
        _connected = false;
    }
    else {
        std::cout << "[DEBUG] Successfully sent " << sent << " bytes." << std::endl;
    }
}
