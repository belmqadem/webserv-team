#include "CGIHandler.hpp"
#include <fcntl.h>

void CGIHandler::keepClientAlive()
{
	if (clientServer)
	{
		clientServer->updateActivity();
	}
}

CGIHandler::CGIHandler(RequestParser &request, const std::string &cgi_path,
					   ResponseBuilder *response, ClientServer *client)
	: pid(-1), output_fd(-1), responseBuilder(response), clientServer(client), isCompleted(false), interpreter(cgi_path), bodyFd(-1)
{
	root_path = request.get_location_config()->root;
	uri = request.get_request_uri();
	scriptPath = root_path + uri;
	method = request.get_http_method();
	queryString = request.get_query_string();
	if (access(scriptPath.c_str(), F_OK) != 0)
		throw std::runtime_error("CGI script not found");
	if (access(scriptPath.c_str(), X_OK) != 0)
		throw std::runtime_error("CGI script not executable");
	const std::vector<byte> &rawBody = request.get_body();
	if (rawBody.size())
	{
		bodyFd = open("/tmp", O_TMPFILE | O_RDWR, S_IRUSR | S_IWUSR);
		if (bodyFd < 0)
		{
			throw std::runtime_error("No CGI interpreter found");
		}
		write(bodyFd, rawBody.data(), rawBody.size());
		lseek(bodyFd, 0, SEEK_SET);
	}
	headers = request.get_headers();
	// Initialize contentType here
	contentType = headers.count("content-type") ? headers["content-type"] : "application/x-www-form-urlencoded";
	if (interpreter.empty())
	{
		throw std::runtime_error("No CGI interpreter found");
	}
}

CGIHandler::~CGIHandler()
{
	terminate();
}

void CGIHandler::setupEnvironment(std::vector<std::string> &env)
{
	std::string script_name = uri;
	size_t last_slash = scriptPath.find_last_of('/');
	if (last_slash != std::string::npos)
		script_name = scriptPath.substr(last_slash + 1);
	env.push_back("REQUEST_METHOD=" + method); 
    env.push_back("QUERY_STRING=" + queryString); 
    env.push_back("CONTENT_LENGTH=" + Utils::to_string(responseBuilder->getRequest().get_body().size()));
    env.push_back("CONTENT_TYPE=" + contentType); 
    env.push_back("SCRIPT_FILENAME=" + scriptPath); 
    env.push_back("SCRIPT_NAME=" + uri); 
    env.push_back("REQUEST_URI=" + uri); 
    env.push_back("SERVER_PROTOCOL=HTTP/1.1"); 
    env.push_back("GATEWAY_INTERFACE=CGI/1.1"); 
    env.push_back("REDIRECT_STATUS=200");
    if (headers.count("Host"))
        env.push_back("HTTP_HOST=" + headers.at("Host"));
    if (headers.count("User-Agent"))
        env.push_back("HTTP_USER_AGENT=" + headers.at("User-Agent"));
	// Sensitive information (e.g., passwords or tokens) is not exposed to the CGI script
    // if (headers.count("Authorization"))
    //     env.push_back("HTTP_AUTHORIZATION=" + headers.at("Authorization"));
    if (headers.count("Cookie"))
        env.push_back("HTTP_COOKIE=" + headers.at("Cookie"));
}

void CGIHandler::startCGI()
{
    startTime = time(NULL);

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        LOG_ERROR("pipe() failed: " + std::string(strerror(errno)));
        throw std::runtime_error("Pipe creation failed");
    }

    pid = fork();
    if (pid == -1)
    {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        throw std::runtime_error("Fork failed");
    }

    if (pid == 0) // Child process
    {
        close(pipe_fd[0]);

        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1)
        {
            LOG_ERROR("dup2() failed for stdout: " + std::string(strerror(errno)));
            exit(1);
        }
        close(pipe_fd[1]);

        if (method == "POST" && bodyFd != -1)
        {
            if (dup2(bodyFd, STDIN_FILENO) == -1)
            {
                LOG_ERROR("dup2() failed for stdin: " + std::string(strerror(errno)));
                exit(1);
            }
            close(bodyFd);
        }
        else
        {
            close(STDIN_FILENO);
        }
        std::vector<std::string> args;
        args.push_back(interpreter);
        args.push_back(scriptPath);
        char *argv[args.size() + 1];
        for (size_t i = 0; i < args.size(); ++i)
            argv[i] = const_cast<char *>(args[i].c_str());
        argv[args.size()] = NULL;
        std::vector<std::string> env;
        setupEnvironment(env);
        char *envp[env.size() + 1];
        for (size_t i = 0; i < env.size(); ++i)
            envp[i] = const_cast<char *>(env[i].c_str());
        envp[env.size()] = NULL;
        execve(argv[0], argv, envp);
        LOG_ERROR("execve() failed: " + std::string(strerror(errno)));
        exit(1);
    }
    else // Parent process
    {
        close(pipe_fd[1]);
        if (bodyFd != -1)
            close(bodyFd);
        bodyFd = -1;

        int flags = fcntl(pipe_fd[0], F_GETFL, 0);
        fcntl(pipe_fd[0], F_SETFL, flags | O_NONBLOCK);

        output_fd = pipe_fd[0];

        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = output_fd;
        try
        {
            IOMultiplexer::getInstance().addListener(this, ev);
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
    if (fd != output_fd)
    {
        LOG_ERROR("CGI onEvent called with unexpected fd: " + Utils::to_string(fd));
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
        }
        else
        {
            // End of data or error
            if (bytesRead < 0)
                LOG_ERROR("Read error from CGI: " + std::string(strerror(errno)));
            finalizeCGI();
        }
    }

    if (ev.events & EPOLLHUP)
    {
        // Finalize directly on hang-up
        finalizeCGI();
    }

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
	if (bodyFd != -1)
	{
		close(bodyFd);
		bodyFd = -1;
	}
	isCompleted = true;
}

void CGIHandler::processCGIOutput()
{
	if (!responseBuilder)
		return;

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
			headerValue.erase(headerValue.find_last_not_of(" \t") + 1);

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
	}

	// Set content type and body
	responseBuilder->set_headers("Content-Type", contentType);
	responseBuilder->set_body(body);
}
