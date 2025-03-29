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

IRCBot::IRCBot(const std::string& server, int port, const std::string& password,
               const std::string& nickname, const std::string& channel)
    : _server(server)
    , _port(port)
    , _password(password)
    , _nickname(nickname)
    , _channel(channel)
    , _socket(-1)
    , _connected(false)
{}

IRCBot::~IRCBot() {
    disconnect();
}

bool IRCBot::connect() {
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(_port);
    serverAddr.sin_addr.s_addr = inet_addr(_server.c_str());

    if (::connect(_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to connect to server: " << strerror(errno) << std::endl;
        close(_socket);
        _socket = -1;
        return false;
    }

    // Send registration commands
    sendRaw("PASS " + _password);
    sendRaw("NICK " + _nickname);
    sendRaw("USER " + _nickname + " 0 * :IRC Bot");
    
    // Give server time to process registration
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Join channel
    sendRaw("JOIN " + _channel);
    
    _connected = true;
    return true;
}

void IRCBot::disconnect() {
    if (_socket >= 0) {
        sendRaw("QUIT :Bot shutting down");
        close(_socket);
        _socket = -1;
        _connected = false;
    }
}

void IRCBot::sendMessage(const std::string& message) {
    if (!_connected) {
        if (!reconnect()) {
            return;
        }
    }
    
    sendRaw("PRIVMSG " + _channel + " :" + message);
}

void IRCBot::handleMessage(const std::string &sender, const std::string &message)
{
	std::cout << "[IRC] <" << sender << "> " << message << std::endl;
}

bool IRCBot::isConnected() const
{
	return _connected;
}

bool IRCBot::processServerMessages()
{
    if (!_connected) {
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
            std::cerr << "Connection closed by server" << std::endl;
            _connected = false;
            close(_socket);
            _socket = -1;
            return false;
        }
        
        buffer[bytesRead] = '\0';
        
        // Process PING messages
        std::string response(buffer);
        if (response.find("PING") != std::string::npos) {
            std::string pong = "PONG" + response.substr(response.find("PING") + 4);
            pong = pong.substr(0, pong.find("\r\n"));
            sendRaw(pong);
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
            
            // Call the handler
            handleMessage(sender, message);
        }
    }
    
    return true;
}

bool IRCBot::reconnect() {
    std::cerr << "Attempting to reconnect..." << std::endl;
    if (_socket >= 0) {
        close(_socket);
        _socket = -1;
    }
    
    // Try to reconnect
    return connect();
}

void IRCBot::sendRaw(const std::string& command) {
    if (_socket < 0) {
        return;
    }
    
    std::string fullCommand = command + "\r\n";
    int sent = send(_socket, fullCommand.c_str(), fullCommand.length(), 0);
    
    if (sent < 0) {
        std::cerr << "Failed to send command: " << strerror(errno) << std::endl;
        _connected = false;
    }
}