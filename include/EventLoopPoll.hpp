#pragma once

#include <EventLoop.hpp>
#include <poll.h>
#include <vector>
#include <cstring>
#include <errno.h>
#include <iostream>

class EventLoopPoll : public EventLoop
{
public:
    EventLoopPoll();
    ~EventLoopPoll();
    void addToWatch(int fd);
    void removeFromWatch(int fd);
    std::vector<Event> waitForEvents(int timeoutMs);
    void shutdown();

private:
    std::vector<struct pollfd> _pollFds;
    uint32_t _eventsToTrack;
};
