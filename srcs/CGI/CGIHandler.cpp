#include "CGIHandler.hpp"
#include "ResponseBuilder.hpp"
#include "ClientServer.hpp"

void CGIHandler::keepClientAlive()
{
	if (clientServer)
	{
		clientServer->updateActivity();
	}
}

CGIHandler::CGIHandler(RequestParser &request, const std::string &php_cgi_path,
					   ResponseBuilder *response, ClientServer *client)
	: pid(-1), output_fd(-1), responseBuilder(response), clientServer(client), isCompleted(false)
{
	root_path = request.get_location_config()->root;
	scriptPath = root_path + request.get_request_uri();
	method = request.get_http_method();
	queryString = request.get_query_string();
	body = std::string(request.get_body().begin(), request.get_body().end());
	headers = request.get_headers();

	size_t dotPos = scriptPath.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		std::string extension = scriptPath.substr(dotPos);
		if (extension == ".php")
		{
			interpreter = php_cgi_path;
		}
	}

	if (interpreter.empty())
	{
		LOG_ERROR("No CGI interpreter found");
		throw std::runtime_error("500 Internal Server Error: No CGI interpreter found");
	}
	LOG_INFO("CGIHandler initialized with script: " + scriptPath);
}

CGIHandler::~CGIHandler()
{
	terminate();
}

void CGIHandler::setupEnvironment(std::vector<std::string> &env)
{
	std::ostringstream oss;
	oss << body.length();

	env.push_back("REQUEST_METHOD=" + method);
	env.push_back("QUERY_STRING=" + queryString);
	env.push_back("CONTENT_LENGTH=" + oss.str());
	env.push_back("CONTENT_TYPE=" + (headers.count("Content-Type") ? headers["Content-Type"] : "application/x-www-form-urlencoded"));
	env.push_back("REDIRECT_STATUS=200");
	env.push_back("SCRIPT_FILENAME=" + scriptPath);
	env.push_back("SCRIPT_NAME=" + scriptPath);
	env.push_back("DOCUMENT_ROOT=" + root_path);
	env.push_back("PHP_SELF=" + scriptPath);
	env.push_back("PATH_TRANSLATED=" + scriptPath);

	// Add server-specific variables
	env.push_back("SERVER_SOFTWARE=webserv/1.0");
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");

	// Log all environment variables for debugging
	LOG_INFO("CGI environment variables:");
	for (size_t i = 0; i < env.size(); ++i)
		LOG_INFO("  " + env[i]);
}

void CGIHandler::startCGI()
{	
	startTime = time(NULL);
	// Create a pipe for CGI output
	int pipe_fd[2];
	if (pipe(pipe_fd) == -1)
	{
		LOG_ERROR("pipe() failed: " + std::string(strerror(errno)));
		throw std::runtime_error("500 Internal Server Error: Pipe creation failed");
	}

	// Create a pipe for CGI input (if needed for POST)
	int input_pipe[2] = {-1, -1};
	if (method == "POST" && !body.empty())
	{
		if (pipe(input_pipe) == -1)
		{
			close(pipe_fd[0]);
			close(pipe_fd[1]);
			LOG_ERROR("pipe() failed for input: " + std::string(strerror(errno)));
			throw std::runtime_error("500 Internal Server Error: Input pipe creation failed");
		}
	}

	// Fork a child process
	pid = fork();
	if (pid == -1)
	{
		// Close all pipes on error
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		if (input_pipe[0] != -1)
			close(input_pipe[0]);
		if (input_pipe[1] != -1)
			close(input_pipe[1]);

		LOG_ERROR("fork() failed: " + std::string(strerror(errno)));
		throw std::runtime_error("500 Internal Server Error: Fork failed");
	}

	if (pid == 0) // Child process
	{
		// Close read end of the output pipe
		close(pipe_fd[0]);

		// Redirect stdout to the write end of the pipe
		if (dup2(pipe_fd[1], STDOUT_FILENO) == -1)
		{
			LOG_ERROR("dup2() failed for stdout: " + std::string(strerror(errno)));
			exit(1);
		}
		close(pipe_fd[1]);

		// For POST requests, set up stdin redirection
		if (method == "POST" && input_pipe[0] != -1)
		{
			close(input_pipe[1]); // Close write end
			if (dup2(input_pipe[0], STDIN_FILENO) == -1)
			{
				LOG_ERROR("dup2() failed for stdin: " + std::string(strerror(errno)));
				exit(1);
			}
			close(input_pipe[0]);
		}

		// Prepare arguments for execve
		std::vector<std::string> args;
		args.push_back(interpreter);
		args.push_back(scriptPath);

		char *argv[args.size() + 1];
		for (size_t i = 0; i < args.size(); ++i)
			argv[i] = const_cast<char *>(args[i].c_str());
		argv[args.size()] = NULL;

		// Prepare environment variables
		std::vector<std::string> env;
		setupEnvironment(env);

		char *envp[env.size() + 1];
		for (size_t i = 0; i < env.size(); ++i)
			envp[i] = const_cast<char *>(env[i].c_str());
		envp[env.size()] = NULL;

		// Execute the CGI script
		LOG_INFO("Executing CGI: " + interpreter + " " + scriptPath);
		execve(argv[0], argv, envp);

		// If execve returns, there was an error
		LOG_ERROR("execve() failed: " + std::string(strerror(errno)));
		exit(1);
	}
	else // Parent process
	{
		// Close write end of the output pipe
		close(pipe_fd[1]);

		// For POST requests, write data to the input pipe
		if (method == "POST" && input_pipe[0] != -1)
		{
			close(input_pipe[0]); // Close read end

			// Write the body to the input pipe
			ssize_t written = write(input_pipe[1], body.c_str(), body.length());
			if (written < 0)
				LOG_ERROR("Error writing to CGI input: " + std::string(strerror(errno)));

			close(input_pipe[1]); // Close after writing
		}

		// Set the output pipe to non-blocking
		int flags = fcntl(pipe_fd[0], F_GETFL, 0);
		fcntl(pipe_fd[0], F_SETFL, flags | O_NONBLOCK);

		// Store the output file descriptor
		output_fd = pipe_fd[0];

		// Register with the IOMultiplexer to be notified when data is available
		epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = output_fd;

		try
		{
			IOMultiplexer::getInstance().addListener(this, ev);
			LOG_INFO("CGI process started with PID: " + to_string(pid) +
						", registered for events on fd: " + to_string(output_fd));
		}
		catch (const std::exception &e)
		{
			LOG_ERROR("Failed to register CGI with IOMultiplexer: " + std::string(e.what()));
			close(output_fd);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			throw;
		}
	}
}

void CGIHandler::onEvent(int fd, epoll_event ev)
{
	LOG_INFO("CGI onEvent called for fd: " + to_string(fd) +
				", events: " + to_string(ev.events));

	if (fd != output_fd)
	{
		LOG_ERROR("CGI onEvent called with unexpected fd: " + to_string(fd));
		return;
	}

	if (ev.events & EPOLLIN)
	{
		// Read data from the CGI process
		char buffer[1024];
		ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);

		if (bytesRead > 0)
		{
			// Add the data to the output buffer
			buffer[bytesRead] = '\0';
			cgi_output.append(buffer, bytesRead);
			LOG_INFO("Read " + to_string(bytesRead) + " bytes from CGI process");
		}
		else if (bytesRead == 0 || (bytesRead < 0 && errno != EAGAIN))
		{
			// End of data or error
			if (bytesRead < 0)
				LOG_ERROR("Read error from CGI: " + std::string(strerror(errno)));
			else
				LOG_INFO("CGI process completed output");

			// Process the output and finalize
			finalizeCGI();
		}
	}

	// EPOLLHUP is normal when the CGI process finishes and closes its pipe
	if (ev.events & EPOLLHUP)
	{
		LOG_INFO("CGI pipe closed (EPOLLHUP)");

		// Try to read any remaining data
		char buffer[1024];
		ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
		if (bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			cgi_output.append(buffer, bytesRead);
			LOG_INFO("Read " + to_string(bytesRead) + " final bytes from CGI process");
		}

		// Process the output and finalize
		finalizeCGI();
	}

	// Only treat EPOLLERR as an actual error
	if (ev.events & EPOLLERR)
	{
		LOG_ERROR("Error on CGI pipe (EPOLLERR)");
		terminate();
	}
}

// Add this helper method to reduce duplication:
void CGIHandler::finalizeCGI()
{
	// Process the CGI output
	processCGIOutput();

	// Clean up
	epoll_event rm_ev;
	rm_ev.data.fd = output_fd;
	try
	{
		IOMultiplexer::getInstance().removeListener(rm_ev, output_fd);
	}
	catch (const std::exception &e)
	{
		Logger::getInstance().error("Error removing listener: " + std::string(e.what()));
	}
	close(output_fd);
	output_fd = -1;

	// Check process status
	int status;
	waitpid(pid, &status, 0);
	pid = -1;

	// Set completed flag
	isCompleted = true;

	// If we have a client server, notify it
	if (clientServer)
	{
		clientServer->onCGIComplete(this);
	}
}

void CGIHandler::terminate()
{
	if (clientServer->isStarted() == false)	
		return;
	if (output_fd != -1)
	{
		try
		{
			epoll_event ev;
			ev.data.fd = output_fd;
			IOMultiplexer::getInstance().removeListener(ev, output_fd);
		}
		catch (const std::exception &e)
		{
			LOG_ERROR("Error removing CGI listener: " + std::string(e.what()));
		}

		close(output_fd);
		output_fd = -1;
	}

	if (pid > 0)
	{
		LOG_INFO("Killing CGI process: " + to_string(pid));
		kill(pid, SIGKILL);

		// Wait for the process to avoid zombies, but with a timeout
		int status;
		time_t start = time(NULL);
		while (waitpid(pid, &status, WNOHANG) == 0)
		{
			// Timeout after 2 seconds
			if (time(NULL) - start > 2)
			{
				LOG_ERROR("Timeout waiting for CGI process to terminate");
				break;
			}
			usleep(10000); // 10ms sleep
		}
		pid = -1;
	}

	isCompleted = true;
}

void CGIHandler::processCGIOutput()
{
	if (!responseBuilder)
		return;
	LOG_INFO("Processing CGI output: " + to_string(cgi_output.size()) + " bytes");

	responseBuilder->set_status(200); // Start with 200 OK by default

	// Identify headers and body
	std::string headers;
	std::string body;
	size_t headerEnd = cgi_output.find(CRLF CRLF);

	if (headerEnd != std::string::npos)
	{
		headers = cgi_output.substr(0, headerEnd);
		body = cgi_output.substr(headerEnd + 4);
	}
	else
	{
		// No header/body split found, treat as body
		body = cgi_output;
	}

	// Look for Content-Type
	std::string contentType = "text/html"; // Default

	// Look for status code in headers
	bool statusSet = false;
	std::istringstream headerStream(headers);
	std::string line;

	while (std::getline(headerStream, line))
	{
		// Trim any trailing CR
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string headerName = line.substr(0, colonPos);
			std::string headerValue = line.substr(colonPos + 1);
			// Trim spaces
			headerValue.erase(0, headerValue.find_first_not_of(" \t"));

			// Special handling for Content-Type and Status
			if (headerName == "Content-Type")
			{
				contentType = headerValue;
			}
			else if (headerName == "Status")
			{
				// Process status as before
				std::string statusLine = line;
				size_t codeStart = statusLine.find_first_of("0123456789");
				if (codeStart != std::string::npos)
				{
					std::string codeStr = statusLine.substr(codeStart, 3);
					int statusCode = std::atoi(codeStr.c_str());
					if (statusCode >= 100 && statusCode < 600)
					{
						responseBuilder->set_status(statusCode);
						statusSet = true;
						LOG_INFO("Setting status code: " + to_string(statusCode));
					}
				}
			}
			else
			{
				// Add all other headers to the response
				responseBuilder->set_headers(headerName, headerValue);
			}
		}
	}

	// Set default status if none was found
	if (!statusSet)
	{
		responseBuilder->set_status(200); // Default to 200 OK
		LOG_INFO("No status found, using default 200 OK");
	}

	// Set content type and body
	responseBuilder->set_headers("Content-Type", contentType);
	responseBuilder->set_body(body);

	// After processing is done, dump debug info about final state
	LOG_INFO("Final CGI status code: " + to_string(responseBuilder->get_status_code()));
}
