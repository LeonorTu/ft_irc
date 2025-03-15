#if !defined(__linux__)
#include <EventLoopPoll.hpp>

EventLoopPoll::EventLoopPoll()
    : _eventsToTrack(POLLIN)
{}

EventLoopPoll::~EventLoopPoll()
{
    shutdown();
}

void EventLoopPoll::addToWatch(int fd)
{
    struct pollfd pfd;

    pfd.fd = fd;
    pfd.events = _eventsToTrack;
    pfd.revents = 0;
    _pollFds.push_back(pfd);
}

void EventLoopPoll::removeFromWatch(int fd)
{
    for (auto it = _pollFds.begin(); it != _pollFds.end(); ++it) {
        if (it->fd == fd) {
            _pollFds.erase(it);
            break;
        }
    }
}

std::vector<Event> EventLoopPoll::waitForEvents(int timeoutMs)
{
    std::vector<Event> events;
    if (_pollFds.empty())
        return events;
    int nfds = poll(_pollFds.data(), _pollFds.size(), timeoutMs);
    if (nfds < 0) {
        if (errno != EINTR) {
            std::cerr << "epoll failed: " << strerror(errno) << std::endl;
        }
        return events;
    }
    if (nfds > 0) {
        for (pollfd &pfd : _pollFds) {
            if (pfd.revents != 0) {
                Event event;
                event.fd = pfd.fd;
                event.events = pfd.revents;
                events.push_back(event);
                pfd.revents = 0;
            }
        }
    }
    return events;
}

void EventLoopPoll::shutdown()
{
    _pollFds.clear();
}
#endif