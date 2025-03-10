// filepath: /home/nmellal/Projects/webserv-team/includes/IEvenetListeners.hpp
#pragma once

class IEvenetListeners {
public:
    virtual void terminate() = 0;
    virtual void onEvent(int fd, epoll_event ev) = 0;
};