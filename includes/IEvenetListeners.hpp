#pragma once

#include "webserv.hpp"

class IEvenetListeners
{
private:
public:
	IEvenetListeners() {};
	virtual ~IEvenetListeners() {};
	IEvenetListeners(const IEvenetListeners &);
	IEvenetListeners &operator=(const IEvenetListeners &);

public:
	virtual void onEvent(int fd, epoll_event ev) = 0;
	virtual void terminate() = 0;
};
