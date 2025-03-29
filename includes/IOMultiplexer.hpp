#pragma once

#include "Exceptions.hpp"
#include "IEvenetListeners.hpp"
#include "Logger.hpp"

#define EPOLL_MAX_EVENTS 255

class IOMultiplexer
{

private:
	std::map<int, IEvenetListeners *> _listeners;
	epoll_event _events[EPOLL_MAX_EVENTS];
	int _epoll_fd;
	bool _is_started;

private:
	IOMultiplexer();
	IOMultiplexer(const IOMultiplexer &rhs);
	IOMultiplexer &operator=(const IOMultiplexer &rhs);
	~IOMultiplexer();

public:
	void runEventLoop();
	void terminate();
	void addListener(IEvenetListeners *listener, epoll_event ev);
	void modifyListener(IEvenetListeners *listener, epoll_event ev);
	void removeListener(epoll_event ev, int fd);

public:
	const int &getEpollFd() const;
	size_t	getListenersCount() const ;
	void setStarted(bool);
	static IOMultiplexer &getInstance();
};
