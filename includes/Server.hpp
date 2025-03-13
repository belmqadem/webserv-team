#pragma once

#include "Logger.hpp"
#include "IEvenetListeners.hpp"
#include "ConfigManager.hpp"
#include "IOMultiplexer.hpp"
#include "webserv.hpp"

#define RD_SIZE 1024

template <class T>
std::string to_string(T t)
{
	std::stringstream str;
	str << t;
	return str.str();
}

class ClientServer : IEvenetListeners
{
private:
	bool _is_started;
	int _server_socket_fd;
	int _peer_socket_fd;
	epoll_event _epoll_ev;
	sockaddr_in _client_addr;
	RequestParser request;
	std::string _request_buffer;
	std::string _response_buffer;

public:
	/* getters and setters */
	bool isStarted() const;
	void setPeerSocketFd(uint32_t fd);
	void setServerSocketFd(uint32_t fd);
	void setClientAddr(sockaddr_in addr);

public:
	void RegisterWithIOMultiplexer();

	ClientServer(const int &server_socket_fd, const int &peer_socket_fd);
	~ClientServer();

	virtual void terminate();

	virtual void onEvent(int fd, epoll_event ev);

private:
	void handleIncomingData();
	void handleResponse();
	void enableWriteEvent();
};

class Server : public IEvenetListeners
{
private:
	/* List of Server configs */
	std::vector<ServerConfig> _config;
	/* State of our Server */
	bool _is_started;
	/* Socket fd and event it interested to */
	epoll_event _listen_sock_ev;
	/* Pool of server sockets fds */
	std::vector<int> _listen_fds;
	/* List of ClientServer connection objects */
	std::vector<ClientServer *> _clients;

	~Server();
	Server(std::vector<ServerConfig> config);

public:
	void StartServer();

	static Server &getInstance(std::vector<ServerConfig> config);

	sockaddr_in getListenAddress(ServerConfig conf);
	void listenOnAddr(sockaddr_in addr);
	void accept_peer(int fd);

	virtual void terminate();
	virtual void onEvent(int fd, epoll_event ev);
};