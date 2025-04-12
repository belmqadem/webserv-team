#include "IOMultiplexer.hpp"

void IOMultiplexer::debugPrintListeners(const std::string &message) const
{
	LOG_INFO(message + " - Listeners map contents:");
	LOG_INFO("Map size: " + Utils::to_string(_listeners.size()));

	for (std::map<int, IEvenetListeners *>::const_iterator it = _listeners.begin();
		 it != _listeners.end(); ++it)
	{
		LOG_INFO("  FD: " + Utils::to_string(it->first) +
				 ", Listener addr: " + Utils::to_string((void *)it->second) +
				 ", Type: " + (dynamic_cast<Server *>(it->second) ? "Server" : (dynamic_cast<ClientServer *>(it->second) ? "ClientServer" : (dynamic_cast<CGIHandler *>(it->second) ? "CGIHandler" : "Unknown"))));
	}
}

IOMultiplexer::IOMultiplexer() : _epoll_fd(epoll_create(__INT32_MAX__)), _is_started(false)
{
	if (_epoll_fd == -1)
	{
		throw IOMultiplexerExceptions("epoll_create failed!");
	}
}

size_t IOMultiplexer::getListenersCount() const
{
	return _listeners.size();
}

void IOMultiplexer::setStarted(bool state)
{
	_is_started = state;
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

#define DEBUG_MODE true
bool debug_mode()
{
	return DEBUG_MODE;
}

void IOMultiplexer::runEventLoop(void)
{
	if (_is_started)
		throw IOMultiplexerExceptions("Server is already started!");
	if (_listeners.size() == 0)
	{
		LOG_ERROR("NO listeners available the program will quit");
		return;
	}
	_is_started = true;
	while (_is_started)
	{
		int events_count = epoll_wait(_epoll_fd, _events, EPOLL_MAX_EVENTS, -1);
		if (events_count == -1)
		{
			if (debug_mode())
				continue;
			if (_is_started)
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
}

void IOMultiplexer::terminate(void)
{
	std::map<int, IEvenetListeners *>::reverse_iterator it = _listeners.rend();
	for (; _listeners.size() ;)
	{
		try
		{
			(it)->second->terminate();
		}
		catch (const std::exception &e)
		{
			LOG_ERROR("Error terminating listener: " + std::string(e.what()));
		}
	}
}