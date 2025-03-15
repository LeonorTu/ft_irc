#include <EventLoop.hpp>
#if defined(__linux__)
#include <EventLoopEpoll.hpp>
#else
#include <EventLoopPoll.hpp>
#endif

std::unique_ptr<EventLoop> createEventLoop()
{
#if defined(__linux__)
    return std::make_unique<EventLoopEpoll>();
#else
    return std::make_unique<EventLoopPoll>();
#endif
}
