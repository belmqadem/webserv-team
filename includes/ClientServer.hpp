#pragma once

#include "IEvenetListeners.hpp"
#include "ConfigManager.hpp"
#include "IOMultiplexer.hpp"
#include "RequestParser.hpp"

class ClientServer : IEvenetListeners
{
private:
	bool _is_started;
	int _server_socket_fd;
	int _peer_socket_fd;
	epoll_event _epoll_ev;
	sockaddr_in _client_addr;
	// RequestParser request;
	std::string _request_buffer;
	std::string _response_buffer;

	void handleIncomingData();
	void handleResponse();
	void enableWriteEvent();

public:
	bool isStarted() const;
	void setPeerSocketFd(uint32_t fd);
	void setServerSocketFd(uint32_t fd);
	void setClientAddr(sockaddr_in addr);
	void RegisterWithIOMultiplexer();

	ClientServer(const int &server_socket_fd, const int &peer_socket_fd);
	~ClientServer();

	virtual void terminate();
	virtual void onEvent(int fd, epoll_event ev);
};
