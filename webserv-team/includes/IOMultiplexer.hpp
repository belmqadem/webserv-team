// filepath: /home/nmellal/Projects/webserv-team/includes/IOMultiplexer.hpp
#pragma once

#include <vector>
#include <sys/epoll.h>

class IEventListeners;

class IOMultiplexer {
public:
    static IOMultiplexer& getInstance();

    void addListener(IEventListeners* listener, epoll_event& event);
    void removeListener(epoll_event& event, int fd);
    void waitForEvents(int timeout);
    void dispatchEvents();

private:
    IOMultiplexer();
    ~IOMultiplexer();

    int _epoll_fd;
    std::vector<IEventListeners*> _listeners;
};