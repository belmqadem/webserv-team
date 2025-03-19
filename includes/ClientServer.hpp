#pragma once

#include "ConfigManager.hpp"
#include "IOMultiplexer.hpp"
#include "ResponseBuilder.hpp"

#define RD_SIZE 1024

class ClientServer : IEvenetListeners
{
private:
	bool _is_started;
	int _server_socket_fd;
	int _peer_socket_fd;
	epoll_event _epoll_ev;
	sockaddr_in _client_addr;
	RequestParser *_parser;
	// RequestParser request;
	std::string _request_buffer;
	std::string _response_buffer;
	bool _response_ready;

private:
	void handleIncomingData();
	void handleResponse();
	void enableWriteEvent();
	void modifyEpollEvent(uint32_t events);

public:
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
};
