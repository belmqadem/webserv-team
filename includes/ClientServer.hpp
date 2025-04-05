#pragma once

#include "ConfigManager.hpp"
#include "IOMultiplexer.hpp"
#include "CGIHandler.hpp"

class CGIHandler;
class RequestParser;

#define RD_SIZE 1024
#define TIME_OUT_SECONDS 15

class ClientServer : public IEvenetListeners
{
private:
	bool _is_started;
	int _server_socket_fd;
	int _peer_socket_fd;
	epoll_event _epoll_ev;
	sockaddr_in _client_addr;
	RequestParser *_parser;
	std::string _request_buffer;
	std::string _response_buffer;
	bool _response_ready;
	time_t _last_activity;
	bool _continue_sent;

	// Add new members for CGI support
	CGIHandler *_pendingCgi;
	bool _waitingForCGI;

private:
	void handleIncomingData();
	void handleResponse();
	void modifyEpollEvent(uint32_t events);

	bool hasTimeOut() const;

public:
	void updateActivity();
	void checkCGIProgress();
	bool isStarted() const;
	void setPeerSocketFd(uint32_t fd);
	void setServerSocketFd(uint32_t fd);
	void setClientAddr(sockaddr_in addr);
	void RegisterWithIOMultiplexer();
	bool shouldKeepAlive() const;

	ClientServer(const int &server_socket_fd, const int &peer_socket_fd);
	~ClientServer();

	virtual void terminate();
	virtual void onEvent(int fd, epoll_event ev);

	// Add new method for CGI completion callback
	void processCGIRequest();
	void onCGIComplete(CGIHandler *handler);
};
