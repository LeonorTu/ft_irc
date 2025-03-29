#pragma once

#include <exception>
#include <string>
#include <stdexcept>

class ConnectionManager;

class Error : public std::exception
{
    public:
    explicit Error(const std::string &message):_message(message){};
    virtual ~Error() noexcept{};

    virtual const char *what() const noexcept override
    {
        return _message.c_str();
    };
    
    static void catchError();

    private:
    std::string _message;

};

class ChannelNotCreated : public Error
{
    public:
        explicit ChannelNotCreated(const std::string &message): Error(message){};
        ~ChannelNotCreated() noexcept{};
};

class ChannelNotFound : public Error
{
    public:
        explicit ChannelNotFound(const std::string &message): Error(message){};
        ~ChannelNotFound() noexcept{};
};
class BrokenPipe : public Error
{
    public:
        explicit BrokenPipe(const std::string &message): Error(message){};
        ~BrokenPipe() noexcept{};
};

class SocketError : public Error 
{
public:
    explicit SocketError(const std::string &message) : Error(message) {};
    ~SocketError() noexcept {};
};

class EventError : public Error 
{
public:
    explicit EventError(const std::string &message) : Error(message) {};
    ~EventError() noexcept {};
};

class MessageError : public Error 
{
public:
    explicit MessageError(const std::string &message) : Error(message) {};
    ~MessageError() noexcept {};
};

class ServerError : public Error 
{
    public:
        explicit ServerError(const std::string &message) : Error(message) {};
        ~ServerError() noexcept {};
};

