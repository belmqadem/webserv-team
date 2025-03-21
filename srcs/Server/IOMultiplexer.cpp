#include "IOMultiplexer.hpp"
#include <cstdio>
#include <signal.h>
#include "Logger.hpp"
#include "Server.hpp"

int webserv_signal = 0;

IOMultiplexer::IOMultiplexer() : _epoll_fd(epoll_create(__INT32_MAX__)), _is_started(false)
{
	if (_epoll_fd == -1)
	{
		throw IOMultiplexerExceptions("epoll_create failed!");
	}
}

IOMultiplexer::~IOMultiplexer()
{
	terminate();
	close(_epoll_fd);
}

const int &IOMultiplexer::getEpollFd() const
{
	return (_epoll_fd);
}

IOMultiplexer &IOMultiplexer::getInstance()
{
	static IOMultiplexer inst;
	return inst;
}

void IOMultiplexer::runEventLoop(void)
{
	if (_is_started)
		throw IOMultiplexerExceptions("Server is already started!");
	if (_listeners.size() == 0)
	{
		std::cerr << RED "NO listeners available the program will quit" RESET << std::endl;
		return;
	}
	_is_started = true;
	while (true)
	{
		int events_count = epoll_wait(_epoll_fd, _events, EPOLL_MAX_EVENTS, -1);
		if (events_count == -1)
		{
			if (webserv_signal == SIGINT) {
				LOG_INFO("Signal catched");
				break;
			}
			terminate();
			throw IOMultiplexerExceptions("epoll_wait() failed.");
		}
		for (int i = 0; i < events_count; i++)
		{
			std::map<int, IEvenetListeners *>::iterator it = _listeners.find(_events[i].data.fd);
			if (it == _listeners.end())
			{
				throw IOMultiplexerExceptions(
					"fd not found in [std::map<int, IEvenetListeners*>::iterator it = _listeners.begin()].");
			}
			it->second->onEvent(_events[i].data.fd, _events[i]);
		}
	}
	terminate();
}

// Add this method to IOMultiplexer class in IOMultiplexer.cpp
void IOMultiplexer::modifyListener(IEvenetListeners *listener, epoll_event ev)
{
    std::map<int, IEvenetListeners *>::iterator it = _listeners.find(ev.data.fd);
    if (it == _listeners.end())
    {
        throw IOMultiplexerExceptions("modifyListener() event listener not found.");
    }
    
    if (it->second != listener)
    {
        throw IOMultiplexerExceptions("modifyListener() listener mismatch.");
    }
    
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, ev.data.fd, &ev) == -1)
    {
        throw IOMultiplexerExceptions("epoll_ctl() failed in modifyListener.");
    }
    
    // LOG_INFO("EventListener modified " + to_string(ev.data.fd));
}

void IOMultiplexer::addListener(IEvenetListeners *listener, epoll_event ev)
{
	std::map<int, IEvenetListeners *>::iterator it = _listeners.find(ev.data.fd);
	if (it != _listeners.end())
	{
		throw IOMultiplexerExceptions("addListener() event listener already added.");
	}
	_listeners.insert(std::pair<int, IEvenetListeners *>(ev.data.fd, listener));
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1)
	{
		throw IOMultiplexerExceptions("epoll_ctl() failed.");
	}
	// LOG_INFO("EventListner added " + to_string(ev.data.fd));
}

void IOMultiplexer::removeListener(epoll_event ev, int fd)
{

	std::map<int, IEvenetListeners *>::iterator it = _listeners.find(fd);
	if (it == _listeners.end())
		throw IOMultiplexerExceptions("removeListener() fd EventListener not found.");
	ev.data.fd = fd;
	_listeners.erase(fd);
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, ev.data.fd, &ev) == -1)
	{
		throw IOMultiplexerExceptions("epoll_ctl() failed.");
	}
	// LOG_INFO("EventListner removed " + to_string(fd));
}

void IOMultiplexer::terminate(void)
{
	if (_is_started == false)
	{
		return;
	}
	_is_started = false;
	std::map<int, IEvenetListeners *>::reverse_iterator it = _listeners.rbegin();
	for (; it != _listeners.rend(); it = _listeners.rbegin())
	{
		it->second->terminate();
	}
}