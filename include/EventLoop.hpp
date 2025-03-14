#pragma once

#include <vector>
#include <sys/epoll.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

struct Event
{
    int fd;
    uint32_t events;
};

class EventLoop
{
public:
    EventLoop();
    ~EventLoop();

    void addToWatch(int fd, uint32_t events);
    void removeFromWatch(int fd);
    std::vector<Event> waitForEvents(int timeoutMs);
    void shutdown();

private:
    int _epollFd;
};
