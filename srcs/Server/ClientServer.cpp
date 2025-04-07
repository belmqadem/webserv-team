#include "ClientServer.hpp"

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
		LOG_ERROR("WARNING: Attempting to registre an already started fd " + Utils::to_string(_peer_socket_fd));
		return;
	}
	_epoll_ev.data.fd = _peer_socket_fd;
	_epoll_ev.events = EPOLLIN | EPOLLOUT;
	try
	{
		IOMultiplexer::getInstance().addListener(this, _epoll_ev);
		_is_started = true;

		std::string addr = inet_ntoa(_client_addr.sin_addr);
		LOG_CLIENT("Connected on " + addr + ":" + Utils::to_string(ntohs(_client_addr.sin_port)) + " Fd " + Utils::to_string(_peer_socket_fd));
	}
	catch (std::exception &e)
	{
		LOG_ERROR("Failed to register client fd " + Utils::to_string(_peer_socket_fd) + " with IO multiplexer. Connection terminated. > " + std::string(e.what()));
		close(_peer_socket_fd);
	}
}

ClientServer::ClientServer(const int &server_socket_fd, const int &peer_socket_fd) : _is_started(false),
																					 _server_socket_fd(server_socket_fd),
																					 _peer_socket_fd(peer_socket_fd),
																					 _parser(NULL),
																					 _last_activity(time(NULL)),
																					 _continue_sent(false),
																					 _pendingCgi(NULL),
																					 _waitingForCGI(false),
																					 _responseBuilder(NULL) {}

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

	// Clean up response builder
	if (_responseBuilder)
	{
		delete _responseBuilder;
		_responseBuilder = NULL;
	}

	// Clean up pending CGI
	if (_pendingCgi)
	{
		delete _pendingCgi;
		_pendingCgi = NULL;
	}

	// Clear any buffered data
	_request_buffer.clear();
	_response_buffer.clear();
	_response_ready = false;
	_waitingForCGI = false;

	try
	{
		IOMultiplexer::getInstance().removeListener(_epoll_ev, _peer_socket_fd);
	}
	catch (std::exception &e)
	{
		LOG_ERROR("Error while removing listener from IO multiplexer " + Utils::to_string(e.what()));
	}

	std::string addr = inet_ntoa(_client_addr.sin_addr);
	LOG_CLIENT(addr + " Fd " + Utils::to_string(_peer_socket_fd) + " Disconnected!");
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
	// Read incoming data into buffer
	if (!readIncomingData())
		return;

	// Handle 100-continue expectations
	handleExpectContinue();

	// Process ongoing body parsing
	if (isParsingRequestBody())
		return;

	// Process new request headers
	processRequestHeaders();
}

bool ClientServer::readIncomingData()
{
	// Early size check based on Content-Length header
	if (_parser && _parser->has_content_length)
	{
		// Get the client_max_body_size from the configuration
		size_t max_size = _parser->get_server_config()->clientMaxBodySize;

		// Check if the declared size exceeds the limit
		if (_parser->get_content_length_value() > max_size)
		{
			sendErrorResponse(413, HTTP_PARSE_PAYLOAD_TOO_LARGE);
			return false;
		}
	}

	// Proceed with normal read
	char buffer[RD_SIZE];
	ssize_t rd_count = recv(this->_peer_socket_fd, buffer, RD_SIZE, MSG_DONTWAIT);

	if (rd_count <= 0)
	{
		this->terminate();
		return false;
	}

	updateActivity();
	_request_buffer.append(buffer, rd_count);
	return true;
}

void ClientServer::sendErrorResponse(int status_code, const std::string &message)
{
	LOG_ERROR(message);

	// Create a temporary ResponseBuilder to generate the error page
	ResponseBuilder errorResponse(*_parser);
	errorResponse.set_status(status_code);
	errorResponse.set_body(errorResponse.generate_error_page());

	// Send the response immediately
	std::string response = errorResponse.generate_response_only();
	send(_peer_socket_fd, response.c_str(), response.size(), 0);

	// Close the connection
	this->terminate();
}

void ClientServer::handleExpectContinue()
{
	if (_parser && _parser->get_headers().count("expect") && !_continue_sent)
	{
		LOG_INFO("Received Expect: 100-continue header");
		std::string continue_response = "HTTP/1.1 100 Continue\r\n\r\n";
		send(_peer_socket_fd, continue_response.c_str(), continue_response.length(), 0);
		_continue_sent = true;
	}
}

bool ClientServer::isParsingRequestBody()
{
	if (!_parser || _parser->get_state() != BODY || _waitingForCGI)
		return false;

	try
	{
		size_t bytes_read = _parser->parse_request(_request_buffer);
		if (bytes_read > 0)
			_request_buffer.erase(0, bytes_read);

		if (_parser->get_state() == DONE)
		{
			LOG_INFO("Request body fully received");
			processCompletedRequest();
		}
		return true;
	}
	catch (std::exception &e)
	{
		LOG_ERROR("Exception in chunked body processing > " + std::string(e.what()));
		return false;
	}
}

void ClientServer::processRequestHeaders()
{
	size_t header_end = _request_buffer.find(DOUBLE_CRLF);
	if (header_end == std::string::npos)
		return;

	try
	{
		parseHeaders();

		if (_parser->get_state() == BODY)
		{
			LOG_INFO("Headers processed, waiting for more body data");
			return;
		}

		processCompletedRequest();
	}
	catch (std::exception &e)
	{
		handleRequestException(e);
	}
}

void ClientServer::parseHeaders()
{
	// Reset the request buffer and state for a new request
	RequestParser parser;
	size_t bytes_read = parser.parse_request(_request_buffer);

	// Only remove the bytes we've successfully processed
	if (bytes_read > 0)
		_request_buffer.erase(0, bytes_read);

	parser.match_location(ConfigManager::getInstance().getServers());

	cleanupParser();

	// Create a clean parser instance
	_parser = new RequestParser(parser);

	LOG_REQUEST(_parser->get_request_line());
}

void ClientServer::processCompletedRequest()
{
	if (_parser->is_cgi_request() && _parser->get_http_method() != "DELETE")
	{
		processCGIRequest();
	}
	else
	{
		processNormalRequest();
	}
}

void ClientServer::processNormalRequest()
{
	// Clean up any existing response builder
	if (_responseBuilder)
	{
		delete _responseBuilder;
	}

	// Create a new response builder
	_responseBuilder = new ResponseBuilder(*_parser);

	// Special case for the /upload endpoint - always use the upload.php handler
	if (_parser->get_request_uri() == "/upload" &&
		_parser->get_http_method() == "POST" &&
		_parser->get_header_value("content-type").find("multipart/form-data") != std::string::npos)
	{
		std::string upload_handler = "www/cgi/phpcgi/upload.php";
		struct stat handler_stat;

		if (stat(upload_handler.c_str(), &handler_stat) == 0)
		{
			LOG_INFO("Redirecting /upload to CGI handler: " + upload_handler);

			std::string uri_path = "/phpcgi/upload.php";

			// Update the parser to directly use upload.php as the CGI script
			_parser->set_cgi_script(upload_handler);
			_parser->set_request_uri(uri_path); // Set the URI as if it was requested directly
			_parser->set_cgi_flag(true);
			_parser->match_location(ConfigManager::getInstance().getServers());

			// Process as CGI instead of normal request
			delete _responseBuilder;
			_responseBuilder = NULL;
			processCGIRequest();
			return;
		}
	}
	// Check for other multipart uploads that should be handled by CGI
	else if (_parser->get_header_value("content-type").find("multipart/form-data") != std::string::npos &&
			 _parser->get_http_method() == "POST" &&
			 !_parser->is_cgi_request())
	{
		std::string upload_handler = "www/cgi/phpcgi/upload.php";
		struct stat handler_stat;

		if (stat(upload_handler.c_str(), &handler_stat) == 0)
		{
			LOG_INFO("Redirecting multipart form to CGI handler: " + upload_handler);

			// Update the parser to point to the CGI script
			upload_handler.insert(upload_handler.begin(), '/');
			_parser->set_request_uri(upload_handler);
			_parser->set_cgi_script(upload_handler);
			_parser->set_cgi_flag(true);

			// Process as CGI instead of normal request
			delete _responseBuilder;
			_responseBuilder = NULL;
			processCGIRequest();
			return;
		}
	}

	// Rest of the method remains the same...
	_response_buffer = _responseBuilder->build_response();
	_response_ready = true;
}

void ClientServer::handleRequestException(const std::exception &e)
{
	LOG_ERROR("Exception in request processing > " + std::string(e.what()));
	cleanupParser();
}

void ClientServer::cleanupParser()
{
	if (_parser)
	{
		delete _parser;
		_parser = NULL;
	}
}

void ClientServer::processCGIRequest()
{
	// Create a response builder if not existing
	if (!_responseBuilder)
	{
		_responseBuilder = new ResponseBuilder(*_parser);
	}

	if (!_responseBuilder->get_location_config()->useCgi)
	{
		LOG_ERROR("CGI NOT ALLOWED IN CONFIG FILE (" + _responseBuilder->getRequest().get_request_uri() + ")");
		_responseBuilder->set_status(405);
		_responseBuilder->set_body(_responseBuilder->generate_error_page());
		_response_buffer = _responseBuilder->generate_response_only();
		_response_ready = true;
		return;
	}

	// Create a CGI handler
	std::string cgi_path = _responseBuilder->get_location_config()->cgiPath;
	_pendingCgi = new CGIHandler(*_parser, cgi_path, _responseBuilder, this);

	try
	{
		// Start the CGI process
		_pendingCgi->startCGI();
		_waitingForCGI = true;
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("CGI failed: " + std::string(e.what()));
		delete _pendingCgi;
		_pendingCgi = NULL;

		// Create an error response
		_responseBuilder->set_status(500);
		_responseBuilder->set_body(_responseBuilder->generate_error_page());
		_response_buffer = _responseBuilder->generate_response_only();
		_response_ready = true;
	}
}

void ClientServer::onCGIComplete(CGIHandler *handler)
{
	if (_pendingCgi == handler)
	{
		LOG_INFO("CGI processing complete, sending response");

		_response_buffer = _responseBuilder->generate_response_only();
		_response_ready = true;
		_waitingForCGI = false;

		// Clean up only the CGI handler
		delete _pendingCgi;
		_pendingCgi = NULL;
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
		if (elapsed > TIME_OUT_SECONDS)
		{
			LOG_ERROR("CGI execution exceeded maximum allowed time");
			delete _pendingCgi;
			_pendingCgi = NULL;
			_waitingForCGI = false;

			// Create an error response
			ResponseBuilder response(*_parser);
			response.set_status(504); // Gateway Timeout
			response.set_body(response.generate_error_page());
			_response_buffer = response.generate_response_only(); //  CONSTRUCTING RESPONSE
			_response_ready = true;
		}
	}
}

void ClientServer::handleResponse()
{
	if (!isResponseReady())
		return;

	if (hasTimeOut())
	{
		handleTimeout();
		return;
	}

	if (!sendResponseChunk())
		return;

	if (_response_buffer.empty())
		finalizeResponse();
}

bool ClientServer::isResponseReady()
{
	return _response_ready && !_response_buffer.empty();
}

void ClientServer::handleTimeout()
{
	LOG_INFO("Client: " + Utils::to_string(this->_peer_socket_fd) +
			 " reached timeout after " + Utils::to_string(TIME_OUT_SECONDS) +
			 " seconds of inactivity");
	terminate();
}

bool ClientServer::sendResponseChunk()
{
	ssize_t bytes_sent = send(_peer_socket_fd, _response_buffer.c_str(),
							  _response_buffer.size(), MSG_DONTWAIT);
	if (bytes_sent <= 0)
	{
		LOG_ERROR("Error sending response to client");
		this->terminate();
		return false;
	}

	updateActivity();

	if (static_cast<size_t>(bytes_sent) < _response_buffer.size())
	{
		_response_buffer = _response_buffer.substr(bytes_sent);
	}
	else
	{
		_response_buffer.clear();
	}

	return true;
}

void ClientServer::finalizeResponse()
{
	_response_ready = false;

	// Clean up the response builder now that we're done with it
	if (_responseBuilder)
	{
		delete _responseBuilder;
		_responseBuilder = NULL;
	}

	if (!shouldKeepAlive())
	{
		this->terminate();
	}
	else if (_parser && _parser->get_error_code())
	{
		cleanupParser();
	}
}

bool ClientServer::shouldKeepAlive() const
{
	return (_parser && !_parser->is_connection_close());
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
		LOG_ERROR("Failed to modify epoll event > " + std::string(e.what()));
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
