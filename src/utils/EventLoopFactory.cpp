#include <EventLoop.hpp>
#include <EventLoopEpoll.hpp>

std::unique_ptr<EventLoop> createEventLoop()
{
    return std::make_unique<EventLoopEpoll>();
}
