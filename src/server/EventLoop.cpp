#include <EventLoop.hpp>
#include <common.hpp>

EventLoop::EventLoop()
    : _epollFd(epoll_create1(0))
    , _running(true)
{
    if (_epollFd < 0) {
        std::cerr << "epoll create error" << std::endl;
    }
}

EventLoop::~EventLoop()
{
    shutdown();
}

void EventLoop::addToWatch(int fd, uint32_t events)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        std::cerr << "Failed to add fd to epoll: " << strerror(errno) << std::endl;
    }
}

void EventLoop::removeFromWatch(int fd)
{
    if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        std::cerr << "Failed to remove fd from epoll: " << strerror(errno) << std::endl;
    }
}

std::vector<Event> EventLoop::waitForEvents(int timeoutMs)
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

void EventLoop::shutdown()
{
    _running = false;
    if (_epollFd >= 0) {
        close(_epollFd);
        _epollFd = -1;
    }
}
