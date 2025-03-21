#include "ClientServer.hpp"
#include "webserv.hpp"

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
																					 _server_socket_fd(server_socket_fd), _peer_socket_fd(peer_socket_fd), _parser(NULL), _last_activity(time(NULL)) {}

ClientServer::~ClientServer()
{
	terminate();
}

void ClientServer::terminate()
{
	if (_is_started == false)
		return ;
	_is_started = false;
	if (_parser)
	{
		delete _parser;
		_parser = NULL;
	}
	IOMultiplexer::getInstance().removeListener(_epoll_ev, _peer_socket_fd);
	_is_started = false;

	std::string addr = inet_ntoa(_client_addr.sin_addr);
	LOG_INFO("Client " + addr + " Fd " + to_string(_peer_socket_fd) + " Disconnected!");
	close(_peer_socket_fd);
}

void ClientServer::onEvent(int fd, epoll_event ev)
{

	if (hasTimeOut()) {
		LOG_INFO("Client: " + to_string(fd) + "Reached Timeout after " + to_string(TIME_OUT_SECONDS) + " Seconds of inactivity");
		terminate();
		return ;
	}
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
	updateActivity();
	_request_buffer.append(buffer, rd_count);
	size_t header_end = _request_buffer.find(CRLF CRLF);
	if (header_end != std::string::npos)
	{
		try
		{
			RequestParser parser(_request_buffer, ConfigManager::getInstance().getServers());
			if (_parser)
				delete _parser;
			_parser = new RequestParser(parser);
			if (parser.get_error_code() != 1)
			{
				LOG_ERROR("Error parsing request: " + to_string(parser.get_error_code()));
			}
			parser.print_request();
			ResponseBuilder response(parser);
			_response_buffer = response.get_response();
			_response_ready = true;
			_request_buffer.clear();

		}
		catch (std::exception &e)
		{
			LOG_ERROR("Exception in request processing " + std::string(e.what()));
		}
	}
}

bool ClientServer::shouldKeepAlive() const
{
	// If we don't have a parser object stored, we can't check
	if (!_parser)
	{
		return false;
	}
    // Get HTTP version and Connection header
    const std::string& version = _parser->get_http_version();
    const std::string& connection = _parser->get_header_value("Connection");

    // HTTP/1.1: keep-alive by default unless "Connection: close"
    if (version == "HTTP/1.1") {
        return connection != "close";
    }
    // Unknown HTTP version or other cases: close the connection
	return false;
}

void ClientServer::modifyEpollEvent(uint32_t events)
{
	_epoll_ev.events = events;

	try
	{
		IOMultiplexer::getInstance().modifyListener(this, _epoll_ev);
	}
	catch (std::exception &e)
	{
		LOG_ERROR("Failed to modify epoll event: " + std::string(e.what()));
	}
}

void ClientServer::handleResponse()
{
	if (!_response_ready || _response_buffer.empty())
	{
		return;
	}
	ssize_t bytes_sent = send(_peer_socket_fd, _response_buffer.c_str(),
	_response_buffer.size(), MSG_DONTWAIT);
	if (bytes_sent <= 0)
	{
		LOG_ERROR("Error sending response to client");
		this->terminate();
		return;
	}
	updateActivity();

	if (static_cast<size_t>(bytes_sent) < _response_buffer.size())
	{
		_response_buffer = _response_buffer.substr(bytes_sent);
	}
	else
	{
		_response_buffer.clear();
		_response_ready = false;

		if (!shouldKeepAlive())
		{
			this->terminate();
		}
	}
}


void ClientServer::updateActivity() {
	_last_activity = time(NULL);
}

bool ClientServer::hasTimeOut() const {
	std::cout << time(NULL) - _last_activity << std::endl;
	return ((time(NULL) - _last_activity) > TIME_OUT_SECONDS);
}


// void ClientServer::enableWriteEvent()
// {
// 	_epoll_ev.events = EPOLLIN | EPOLLOUT;

// 	try {
//         IOMultiplexer::getInstance().modifyListener(this, _epoll_ev);
//     } catch (std::exception &e) {
//         LOG_ERROR("Failed to modify epoll event: " + std::string(e.what()));
//     }
// }