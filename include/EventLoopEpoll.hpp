#pragma once

#include <vector>
#include <sys/epoll.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <EventLoop.hpp>

class EventLoopEpoll : public EventLoop
{
public:
    EventLoopEpoll();
    ~EventLoopEpoll();

    void addToWatch(int fd);
    void removeFromWatch(int fd);
    std::vector<Event> waitForEvents(int timeoutMs);
    void shutdown();

private:
    int _epollFd;
    uint32_t _eventsToTrack;
};
