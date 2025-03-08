#include "Server.hpp"
#include "exceptions.hpp"
#include <string.h>

Server::Server(std::vector<ServerConfig> config) : _config(config), _is_started(false) {
	_listen_sock_ev.events = EPOLLIN;
}


Server &Server::getInstance(std::vector<ServerConfig> config) {
	static Server inst(config);
	return inst;
}

sockaddr_in Server::getListenAddress(ServerConfig conf) {
	
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(conf.port);
	if (conf.host == "0.0.0.0") {
		addr.sin_addr.s_addr = INADDR_ANY;
		return addr;
	}
	if (conf.host == "127.0.0.1") {
		addr.sin_addr.s_addr = INADDR_LOOPBACK;
		return addr;
	}
	if (inet_pton(AF_INET, conf.host.c_str(), &(addr.sin_addr)) != 1) {
		throw ServerExceptions("Invalid host listen addresse : " + conf.host);
	}
	return addr;
}

void    Server::listenOnAddr(sockaddr_in addr) {
	int socket_fd;
	if ((socket_fd =  socket(AF_INET, SOCK_STREAM, 0)) == -1)
	throw ServerExceptions("socket(): failed.");

	int flag = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1)
		close(socket_fd), throw ServerExceptions("setsockopt(): failed");
	if (bind(socket_fd, (sockaddr *)&addr, sizeof(addr)) == -1)
		close(socket_fd), throw ServerExceptions("bind(): failed");
	if (listen(socket_fd, SOMAXCONN) == -1)
		close(socket_fd), throw ServerExceptions("listen(): failed");
	try {
		_listen_sock_ev.data.fd = socket_fd;
		IOMultiplexer::getInstance().addListener(this, _listen_sock_ev);
	} catch (std::exception &e) {
		close(socket_fd);
		throw e;
	}
	_listen_fds.push_back(socket_fd);
}


void    Server::StartServer(void) {
	_is_started = true;
	
	std::vector<ServerConfig>::iterator it = _config.begin();
	for (; it != _config.end(); it++) {
		try {
			sockaddr_in addr = getListenAddress(*it);
			listenOnAddr(addr);
		} catch (std::exception &e) {
			std::cout << "Failed to listen on addr " << it->host << ":" << it->port
			<< "\n" << "Reason : " << e.what() << std::endl;
		}
	}
}

void    Server::terminate() {
	_is_started = false;
	{
		std::vector<int>::iterator i = _listen_fds.begin();
		for (; i != _listen_fds.end(); i++) {
			IOMultiplexer::getInstance().removeListener(_listen_sock_ev, *i);
			close(*i);
		}
	}
	{
		std::vector<ClientServer *>::iterator i = _clients.begin();
		for (; i != _clients.end(); i++) {
			delete *i;
		}
		_clients.clear();
	}
}