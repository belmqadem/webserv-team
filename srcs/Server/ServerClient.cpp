#include "Server.hpp"

bool ClientServer::isStarted() const
{
	return _is_started;
}

void ClientServer::setPeerSocketFd(uint32_t fd)
{
	_peer_socket_fd = fd;
}

void ClientServer::setServerSocketFd(uint32_t fd)
{
	_server_socket_fd = fd;
}

void ClientServer::setClientAddr(sockaddr_in addr)
{
	_client_addr = addr;
}

void ClientServer::RegisterWithIOMultiplexer()
{
	if (_is_started == true)
	{
		std::cerr << RED "WARNING: Attempting to registre an already started fd " << _peer_socket_fd << RESET << std::endl;
		return;
	}
	_epoll_ev.data.fd = _peer_socket_fd;
	_epoll_ev.events = EPOLLIN | EPOLLOUT;
	try
	{
		IOMultiplexer::getInstance().addListener(this, _epoll_ev);
		_is_started = true;

		std::string addr = inet_ntoa(_client_addr.sin_addr);
		LOG_INFO("Client Connected: " + addr + ":" + to_string(ntohs(_client_addr.sin_port)) + " Fd " + to_string(_peer_socket_fd));
	}
	catch (std::exception &e)
	{
		close(_peer_socket_fd);
		std::cerr << RED "Failed to register client fd " << _peer_socket_fd
				  << " with IO multiplexer. Connection terminated.\n"
				  << "Error: " << e.what() << RESET << std::endl;
	}
}

ClientServer::ClientServer(const int &server_socket_fd, const int &peer_socket_fd) : _is_started(false),
																					 _server_socket_fd(server_socket_fd), _peer_socket_fd(peer_socket_fd) {}

ClientServer::~ClientServer()
{
	terminate();
}

void ClientServer::terminate()
{
	IOMultiplexer::getInstance().removeListener(_epoll_ev, _peer_socket_fd);
	_is_started = false;

	std::string addr = inet_ntoa(_client_addr.sin_addr);
	LOG_INFO("Client " + addr + " Fd " + to_string(_peer_socket_fd) + " Disconnected!");
	close(_peer_socket_fd);
}

void ClientServer::onEvent(int fd, epoll_event ev)
{
	(void)fd;

	if (ev.events & EPOLLIN)
	{
		handleIncomingData();
	}

	if (ev.events & EPOLLOUT)
	{
		handleResponse();
	}
}

void ClientServer::handleIncomingData()
{
	char buffer[RD_SIZE];
	ssize_t rd_count = recv(this->_peer_socket_fd, buffer, RD_SIZE, MSG_DONTWAIT);

	if (rd_count <= 0)
	{
		this->terminate();
		return;
	}

	_request_buffer.append(buffer, rd_count);

	/*Implement the parsing handling and build the response*/

	enableWriteEvent();
}

void ClientServer::handleResponse()
{
	/*Implement response sending*/
}

void ClientServer::enableWriteEvent()
{
	/*Enable write event*/
}