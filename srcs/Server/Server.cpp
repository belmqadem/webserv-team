#include "Server.hpp"

Server::Server(std::vector<ServerConfig> config) : IEvenetListeners(), _config(config), _is_started(false)
{
	_listen_sock_ev.events = EPOLLIN;
}

Server::~Server() {
	terminate();
	std::vector<ClientServer *>::iterator it = _clients.begin();
	for (; it != _clients.end(); ++it) {
		delete *it;
	}
}

Server &Server::getInstance(std::vector<ServerConfig> config)
{
	static Server inst(config);
	return inst;
}

sockaddr_in Server::getListenAddress(ServerConfig conf)
{

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(conf.port);
	if (conf.host == "0.0.0.0")
	{
		addr.sin_addr.s_addr = INADDR_ANY;
		return addr;
	}
	if (conf.host == "127.0.0.1")
	{
		addr.sin_addr.s_addr = INADDR_LOOPBACK;
		return addr;
	}
	if (inet_pton(AF_INET, conf.host.c_str(), &(addr.sin_addr)) != 1)
	{
		throw ServerExceptions("Invalid host listen addresse : " + conf.host);
	}
	return addr;
}

void Server::listenOnAddr(sockaddr_in addr)
{
	int socket_fd;
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		throw ServerExceptions("socket(): failed.");

	int flag = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1)
		close(socket_fd), throw ServerExceptions("setsockopt(): failed");
	if (bind(socket_fd, (sockaddr *)&addr, sizeof(addr)) == -1)
		close(socket_fd), throw ServerExceptions("bind(): failed");
	if (listen(socket_fd, SOMAXCONN) == -1)
		close(socket_fd), throw ServerExceptions("listen(): failed");
	fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK);
	try
	{
		_listen_sock_ev.data.fd = socket_fd;
		IOMultiplexer::getInstance().addListener(this, _listen_sock_ev);

		std::string ip_address = inet_ntoa(addr.sin_addr);
		int port = ntohs(addr.sin_port);
		LOG_SERVER("Listening on " + ip_address + ":" + Utils::to_string(port));
	}
	catch (std::exception &e)
	{
		close(socket_fd);
		throw e;
	}
	_listen_fds.push_back(socket_fd);
}

void Server::StartServer(void)
{
	LOG_SERVER("Our Webserver *Not Nginx* Is Starting . . .");
	_is_started = true;

	std::vector<ServerConfig>::iterator it = _config.begin();
	for (; it != _config.end(); it++)
	{
		try
		{
			sockaddr_in addr = getListenAddress(*it);
			listenOnAddr(addr);
		}
		catch (std::exception &e)
		{
			LOG_ERROR("Failed to listen on addr " + it->host + ":" + Utils::to_string(it->port) + " > " + std::string(e.what()));
		}
	}
}

void Server::terminate()
{
	if (_is_started == false)
		return;
	_is_started = false;
	{
		std::vector<int>::iterator i = _listen_fds.begin();
		for (; i != _listen_fds.end(); i++)
		{
			IOMultiplexer::getInstance().removeListener(_listen_sock_ev, *i);
			close(*i);
		}
	}
	{
		std::vector<ClientServer *>::iterator i = _clients.begin();
		for (; i != _clients.end(); i++)
		{
			(*i)->terminate();
		}
	}
	LOG_SERVER("Our Webserver *Not Nginx* is Shuted down !");
}

void Server::onEvent(int fd, epoll_event ev)
{
	std::vector<int>::iterator listen_fd = std::find(_listen_fds.begin(), _listen_fds.end(), ev.data.fd);
	if (listen_fd != _listen_fds.end())
		accept_peer(fd);
}

void Server::accept_peer(int fd)
{
	sockaddr_in peer_addrr;
	socklen_t peer_addrr_len = sizeof(peer_addrr);
	int client_fd = accept(fd, (sockaddr *)&peer_addrr, &peer_addrr_len);
	if (client_fd == -1)
	{
		LOG_ERROR("accept() failed to accept this peer");
		return;
	}
	fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL, 0) | O_NONBLOCK);
	std::vector<ClientServer *>::iterator client = _clients.begin();
	for (; client != _clients.end() && (*client)->isStarted(); client++)
		;
	if (client != _clients.end())
		(*client)->setPeerSocketFd(client_fd), (*client)->setServerSocketFd(fd);
	else
	{
		ClientServer *new_client = new ClientServer(fd, client_fd);
		_clients.push_back(new_client);
		client = _clients.begin() + _clients.size() - 1;
	}

	(*client)->setClientAddr(peer_addrr), (*client)->RegisterWithIOMultiplexer();
}