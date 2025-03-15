#if defined(__linux__)
#include <EventLoopEpoll.hpp>
#include <common.hpp>

EventLoopEpoll::EventLoopEpoll()
    : _epollFd(epoll_create1(0))
    , _eventsToTrack(EPOLLIN | EPOLLET)
{
    if (_epollFd < 0) {
        std::cerr << "epoll create error" << std::endl;
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
        std::cerr << "Failed to add fd to epoll: " << strerror(errno) << std::endl;
    }
}

void EventLoopEpoll::removeFromWatch(int fd)
{
    if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        std::cerr << "Failed to remove fd from epoll: " << strerror(errno) << std::endl;
    }
}

std::vector<Event> EventLoopEpoll::waitForEvents(int timeoutMs)
{
    epoll_event epollEvents[EPOLL_MAX_EVENTS] = {0};
    std::vector<Event> results;
    int nfds = epoll_wait(_epollFd, epollEvents, EPOLL_MAX_EVENTS, timeoutMs);
    if (nfds < 0) {
        if (errno != EINTR) {
            std::cerr << "epoll failed: " << strerror(errno) << std::endl;
        }
        return results;
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