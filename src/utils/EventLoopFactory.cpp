#include <EventLoop.hpp>
#include <EventLoopEpoll.hpp>
#include <EventLoopPoll.hpp>

std::unique_ptr<EventLoop> createEventLoop()
{
#if defined(__linux__)
    return std::make_unique<EventLoopEpoll>();
#else
    return std::make_unique<EventLoopPoll>();
#endif
}
