#include "ClientServer.hpp"
#include "CGIHandler.hpp"

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
		LOG_ERROR("WARNING: Attempting to registre an already started fd " + to_string(_peer_socket_fd));
		return;
	}
	_epoll_ev.data.fd = _peer_socket_fd;
	_epoll_ev.events = EPOLLIN | EPOLLOUT;
	try
	{
		IOMultiplexer::getInstance().addListener(this, _epoll_ev);
		_is_started = true;

		std::string addr = inet_ntoa(_client_addr.sin_addr);
		LOG_CLIENT("Connected on " + addr + ":" + to_string(ntohs(_client_addr.sin_port)) + " Fd " + to_string(_peer_socket_fd));
	}
	catch (std::exception &e)
	{
		LOG_ERROR("Failed to register client fd " + to_string(_peer_socket_fd) + " with IO multiplexer. Connection terminated. -- " + std::string(e.what()));
		close(_peer_socket_fd);
	}
}

ClientServer::ClientServer(const int &server_socket_fd, const int &peer_socket_fd) : _is_started(false),
																					 _server_socket_fd(server_socket_fd), _peer_socket_fd(peer_socket_fd), _parser(NULL), _last_activity(time(NULL)), _pendingCgi(NULL), _waitingForCGI(false) {}

ClientServer::~ClientServer()
{
	terminate();
}

void ClientServer::terminate()
{
    if (_is_started == false)
        return;
    _is_started = false;
    
    // Clean up parser
    if (_parser)
    {
        delete _parser;
        _parser = NULL;
    }
    
    // Clean up pending CGI and its ResponseBuilder
    if (_pendingCgi)
    {
        ResponseBuilder* respBuilder = _pendingCgi->getResponseBuilder();
        delete _pendingCgi;
        _pendingCgi = NULL;
        
        // Also delete the response builder if it exists
        if (respBuilder)
        {
            delete respBuilder;
        }
    }
    
    IOMultiplexer::getInstance().removeListener(_epoll_ev, _peer_socket_fd);

    std::string addr = inet_ntoa(_client_addr.sin_addr);
    LOG_CLIENT(addr + " Fd " + to_string(_peer_socket_fd) + " Disconnected!");
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
		if (_waitingForCGI)
			checkCGIProgress();
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
			RequestParser parser;
			size_t bytes_read = 0;
			bytes_read += parser.parse_request(_request_buffer);
			parser.match_location(ConfigManager::getInstance().getServers());
			if (_parser)
				delete _parser;
			_parser = new RequestParser(parser);
			if (_parser->get_error_code() == 1)
				LOG_REQUEST(_parser->get_request_line());

			// Check if this is a CGI request
			if (_parser->get_location_config() &&
				_parser->get_location_config()->useCgi &&
				ResponseBuilder::is_cgi_request(_parser->get_request_uri()))
			{
				LOG_INFO("Processing CGI request");

				// Create a response builder but don't execute it yet
				ResponseBuilder *respBuilder = new ResponseBuilder(*_parser);

				// Create a CGI handler
				_pendingCgi = new CGIHandler(*_parser, "/usr/bin/php-cgi", respBuilder, this);

				try
				{
					// Start the CGI process
					_pendingCgi->startCGI();
					_waitingForCGI = true;

					// Don't set _response_ready, we'll wait for CGI completion
				}
				catch (const std::exception &e)
				{
					LOG_ERROR("CGI failed: " + std::string(e.what()));
					delete _pendingCgi;
					_pendingCgi = NULL;

					// Create an error response
					respBuilder->set_status(500);
					respBuilder->set_body(respBuilder->generate_error_page());
					_response_buffer = respBuilder->get_response();
					_response_ready = true;
					delete respBuilder;
				}
			}
			else
			{
				// Normal (non-CGI) request processing
				ResponseBuilder response(*_parser);
				_response_buffer = response.get_response();
				_response_ready = true;
			}

			_request_buffer.clear();
		}
		catch (std::exception &e)
		{
			LOG_ERROR("Exception in request processing -- " + std::string(e.what()));
		}
	}
}

void ClientServer::onCGIComplete(CGIHandler *handler)
{
    if (_pendingCgi == handler)
    {
        LOG_INFO("CGI processing complete, sending response");

        // Get the response builder
        ResponseBuilder *respBuilder = handler->getResponseBuilder();

        // Build the response
        _response_buffer = respBuilder->build_response();
        _response_ready = true;
        _waitingForCGI = false;
        
        updateActivity();

        // Clean up both the CGI handler and the response builder
        delete _pendingCgi;
        _pendingCgi = NULL;
        
        // Clean up the response builder after we've generated the response
        delete respBuilder;
    }
}

void ClientServer::checkCGIProgress()
{
    if (_waitingForCGI && _pendingCgi)
    {
        // Keep the connection alive while CGI is processing
        updateActivity();
        
        // Check if it's been too long (e.g., 60 seconds)
        time_t elapsed = time(NULL) - _pendingCgi->getStartTime();
        if (elapsed > TIME_OUT_SECONDS) {
            LOG_ERROR("CGI execution exceeded maximum allowed time");
            delete _pendingCgi;
            _pendingCgi = NULL;
            _waitingForCGI = false;
            
            // Create an error response
            ResponseBuilder response(*_parser);
            response.set_status(504);  // Gateway Timeout
            response.set_body(response.generate_error_page());
            _response_buffer = response.get_response();
            _response_ready = true;
        }
    }
}

void ClientServer::handleResponse()
{
	if (!_response_ready || _response_buffer.empty())
	{
		return;
	}
	if (hasTimeOut())
	{
		LOG_INFO("Client: " + to_string(this->_peer_socket_fd) + "Reached Timeout after " + to_string(TIME_OUT_SECONDS) + " Seconds of inactivity");
		terminate();
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

bool ClientServer::shouldKeepAlive() const
{
	// If we don't have a parser object stored, we can't check
	if (!_parser)
	{
		return false;
	}
	return (!_parser->is_connection_close());
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
		LOG_ERROR("Failed to modify epoll event -- " + std::string(e.what()));
	}
}

void ClientServer::updateActivity()
{
	_last_activity = time(NULL);
}

bool ClientServer::hasTimeOut() const
{
	return ((time(NULL) - _last_activity) > TIME_OUT_SECONDS);
}
