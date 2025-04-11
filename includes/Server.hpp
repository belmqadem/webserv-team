#pragma once

#include "ClientServer.hpp"

class ClientServer;

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
	/* Maps socket FDs to their server configs */
	std::map<int, std::vector<ServerConfig *> > _socket_configs;

	~Server();
	Server(std::vector<ServerConfig> config);

public:
	void StartServer();

	static Server &getInstance(std::vector<ServerConfig> config);

	sockaddr_in getListenAddress(ServerConfig conf);
	void listenOnAddr(sockaddr_in addr, std::vector<ServerConfig *> configs);
	void accept_peer(int fd);

	virtual void terminate();
	virtual void onEvent(int fd, epoll_event ev);
};