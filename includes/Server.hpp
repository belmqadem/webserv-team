#pragma once

#include "Logger.hpp"
#include "IEvenetListeners.hpp"
#include "ConfigManager.hpp"
#include "IOMultiplexer.hpp"
#include "webserv.hpp"
#include "ClientServer.hpp"

#define RD_SIZE 1024

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

	const ServerConfig *getMatchingServerConfig(const std::string &host) const
	{
		for (size_t i = 0; i < _config.size(); i++)
		{
			if (_config[i].serverNames[0] == host)
			{
				return &_config[i];
			}
		}
		return &_config[0];
	}
};