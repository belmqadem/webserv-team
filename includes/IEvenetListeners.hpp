#pragma once

#include "webserv.hpp"

class IEvenetListeners
{
private:
public:
	IEvenetListeners() {};
	IEvenetListeners(const IEvenetListeners &);
	IEvenetListeners &operator=(const IEvenetListeners &);
	virtual ~IEvenetListeners() {};

	virtual void onEvent(int fd, epoll_event ev) = 0;
	virtual void terminate() = 0;
};
