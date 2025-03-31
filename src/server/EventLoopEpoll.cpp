#if defined(__linux__)
#include <EventLoopEpoll.hpp>
#include <common.hpp>
#include <Error.hpp>

EventLoopEpoll::EventLoopEpoll()
    : _epollFd(epoll_create1(0))
    , _eventsToTrack(EPOLLIN)
{
    if (_epollFd < 0) {
        throw EventError("epoll create error");
    }
}

EventLoopEpoll::~EventLoopEpoll()
{
    shutdown();
}

void EventLoopEpoll::addToWatch(int fd)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = _eventsToTrack;
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        throw EventError("Failed to add fd to epoll: " + std::string(strerror(errno)));
    }
}

void EventLoopEpoll::removeFromWatch(int fd)
{
    if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        throw EventError("Failed to remove fd from epoll: " + std::string(strerror(errno)));
    }
}

std::vector<Event> EventLoopEpoll::waitForEvents(int timeoutMs)
{
    epoll_event epollEvents[EPOLL_MAX_EVENTS] = {};
    std::vector<Event> results;
    int nfds = epoll_wait(_epollFd, epollEvents, EPOLL_MAX_EVENTS, timeoutMs);
    if (nfds < 0) {
        throw EventError("epoll failed: " + std::string(strerror(errno)));
    }
    for (int i = 0; i < nfds; i++) {
        Event event;
        event.fd = epollEvents[i].data.fd;
        event.events = epollEvents[i].events;
        results.push_back(event);
    }
    return results;
}

void EventLoopEpoll::shutdown()
{
    if (_epollFd >= 0) {
        close(_epollFd);
        _epollFd = -1;
    }
}
#endif
