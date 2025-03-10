// filepath: /webserv-team/webserv-team/src/IOMultiplexer.cpp
#include "IOMultiplexer.hpp"
#include <stdexcept>

IOMultiplexer& IOMultiplexer::getInstance() {
    static IOMultiplexer instance;
    return instance;
}

void IOMultiplexer::addListener(IEvenetListeners* listener, epoll_event ev) {
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1) {
        throw std::runtime_error("Failed to add listener to I/O multiplexer");
    }
    _listeners[ev.data.fd] = listener;
}

void IOMultiplexer::removeListener(epoll_event ev, int fd) {
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        throw std::runtime_error("Failed to remove listener from I/O multiplexer");
    }
    _listeners.erase(fd);
}

void IOMultiplexer::waitForEvents() {
    epoll_event events[MAX_EVENTS];
    int event_count = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
    for (int i = 0; i < event_count; ++i) {
        if (_listeners.find(events[i].data.fd) != _listeners.end()) {
            _listeners[events[i].data.fd]->onEvent(events[i].data.fd, events[i]);
        }
    }
}

IOMultiplexer::IOMultiplexer() {
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor");
    }
}

IOMultiplexer::~IOMultiplexer() {
    close(_epoll_fd);
}