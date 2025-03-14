#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

struct Event
{
    int fd;
    uint32_t events;
};

class EventLoop
{
public:
    virtual ~EventLoop() = default;
    virtual void addToWatch(int fd) = 0;
    virtual void removeFromWatch(int fd) = 0;
    virtual std::vector<Event> waitForEvents(int timeoutMs) = 0;
    virtual void shutdown() = 0;

private:
};

std::unique_ptr<EventLoop> createEventLoop();
