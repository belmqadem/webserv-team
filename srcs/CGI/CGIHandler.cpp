#include "CGIHandler.hpp"
#include "ResponseBuilder.hpp"
#include "ClientServer.hpp"

#define ROOT_DIRECTORY "www/html"

void CGIHandler::keepClientAlive()
{
    if (clientServer)
    {
        clientServer->updateActivity();
    }
}

CGIHandler::CGIHandler(RequestParser &request, const std::string &php_cgi_path, 
                       ResponseBuilder* response, ClientServer* client)
    : pid(-1), output_fd(-1), responseBuilder(response), clientServer(client), isCompleted(false)
{
    Logger &logger = Logger::getInstance();
    scriptPath = ROOT_DIRECTORY + request.get_request_uri();
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
        logger.error("No CGI interpreter found");
        throw std::runtime_error("500 Internal Server Error: No CGI interpreter found");
    }
    logger.info("CGIHandler initialized with script: " + scriptPath);
}

CGIHandler::~CGIHandler()
{
    terminate();
}

void CGIHandler::setupEnvironment(std::vector<std::string>& env)
{
    std::ostringstream oss;
    oss << body.length();
    
    env.push_back("REQUEST_METHOD=" + method);
    env.push_back("QUERY_STRING=" + queryString);
    env.push_back("CONTENT_LENGTH=" + oss.str());
    env.push_back("CONTENT_TYPE=" + (headers.count("Content-Type") ? 
                                    headers["Content-Type"] : 
                                    "application/x-www-form-urlencoded"));
    env.push_back("REDIRECT_STATUS=200");
    env.push_back("SCRIPT_FILENAME=" + scriptPath);
    env.push_back("SCRIPT_NAME=" + scriptPath);
    env.push_back("DOCUMENT_ROOT=" + std::string(ROOT_DIRECTORY));
    env.push_back("PHP_SELF=" + scriptPath);
    env.push_back("PATH_TRANSLATED=" + scriptPath);
    
    // Add server-specific variables
    env.push_back("SERVER_SOFTWARE=webserv/1.0");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    
    // Log all environment variables for debugging
    Logger::getInstance().info("CGI environment variables:");
    for (size_t i = 0; i < env.size(); ++i)
        Logger::getInstance().info("  " + env[i]);
}

void CGIHandler::startCGI()
{
    Logger &logger = Logger::getInstance();
    startTime = time(NULL);
    // Create a pipe for CGI output
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        logger.error("pipe() failed: " + std::string(strerror(errno)));
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
            logger.error("pipe() failed for input: " + std::string(strerror(errno)));
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
        if (input_pipe[0] != -1) close(input_pipe[0]);
        if (input_pipe[1] != -1) close(input_pipe[1]);
        
        logger.error("fork() failed: " + std::string(strerror(errno)));
        throw std::runtime_error("500 Internal Server Error: Fork failed");
    }
    
    if (pid == 0) // Child process
    {
        // Close read end of the output pipe
        close(pipe_fd[0]);
        
        // Redirect stdout to the write end of the pipe
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1)
        {
            logger.error("dup2() failed for stdout: " + std::string(strerror(errno)));
            exit(1);
        }
        close(pipe_fd[1]);
        
        // For POST requests, set up stdin redirection
        if (method == "POST" && input_pipe[0] != -1)
        {
            close(input_pipe[1]); // Close write end
            if (dup2(input_pipe[0], STDIN_FILENO) == -1)
            {
                logger.error("dup2() failed for stdin: " + std::string(strerror(errno)));
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
        logger.info("Executing CGI: " + interpreter + " " + scriptPath);
        execve(argv[0], argv, envp);
        
        // If execve returns, there was an error
        logger.error("execve() failed: " + std::string(strerror(errno)));
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
                logger.error("Error writing to CGI input: " + std::string(strerror(errno)));
            
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
            logger.info("CGI process started with PID: " + to_string(pid) + 
                        ", registered for events on fd: " + to_string(output_fd));
        }
        catch (const std::exception& e)
        {
            logger.error("Failed to register CGI with IOMultiplexer: " + std::string(e.what()));
            close(output_fd);
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);
            throw;
        }
    }
}

void CGIHandler::onEvent(int fd, epoll_event ev)
{
    Logger &logger = Logger::getInstance();
    
    logger.info("CGI onEvent called for fd: " + to_string(fd) + 
                ", events: " + to_string(ev.events));
    
    if (fd != output_fd)
    {
        logger.error("CGI onEvent called with unexpected fd: " + to_string(fd));
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
            logger.info("Read " + to_string(bytesRead) + " bytes from CGI process");
        }
        else if (bytesRead == 0 || (bytesRead < 0 && errno != EAGAIN))
        {
            // End of data or error
            if (bytesRead < 0)
                logger.error("Read error from CGI: " + std::string(strerror(errno)));
            else
                logger.info("CGI process completed output");
            
            // Process the output and finalize
            finalizeCGI();
        }
    }
    
    // EPOLLHUP is normal when the CGI process finishes and closes its pipe
    if (ev.events & EPOLLHUP)
    {
        logger.info("CGI pipe closed (EPOLLHUP)");
        
        // Try to read any remaining data
        char buffer[1024];
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            cgi_output.append(buffer, bytesRead);
            logger.info("Read " + to_string(bytesRead) + " final bytes from CGI process");
        }
        
        // Process the output and finalize
        finalizeCGI();
    }
    
    // Only treat EPOLLERR as an actual error
    if (ev.events & EPOLLERR)
    {
        logger.error("Error on CGI pipe (EPOLLERR)");
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
    try {
        IOMultiplexer::getInstance().removeListener(rm_ev, output_fd);
    } catch (const std::exception& e) {
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
    Logger &logger = Logger::getInstance();
    
    if (output_fd != -1)
    {
        try {
            epoll_event ev;
            ev.data.fd = output_fd;
            IOMultiplexer::getInstance().removeListener(ev, output_fd);
        } catch (const std::exception& e) {
            logger.error("Error removing CGI listener: " + std::string(e.what()));
        }
        
        close(output_fd);
        output_fd = -1;
    }
    
    if (pid > 0)
    {
        logger.info("Killing CGI process: " + to_string(pid));
        kill(pid, SIGKILL);
        
        // Wait for the process to avoid zombies, but with a timeout
        int status;
        time_t start = time(NULL);
        while (waitpid(pid, &status, WNOHANG) == 0) {
            // Timeout after 2 seconds 
            if (time(NULL) - start > 2) {
                logger.error("Timeout waiting for CGI process to terminate");
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
    
    // Use the appropriate version of parseCGIOutput
    std::pair<std::string, std::string> parsed = parseCGIOutput(cgi_output);
    
    // Update the response builder
    responseBuilder->set_headers("Content-Type", parsed.first);
    responseBuilder->set_body(parsed.second);
    responseBuilder->set_status(200);
}

std::pair<std::string, std::string> CGIHandler::parseCGIOutput(const std::string& output)
{
    std::istringstream stream(output);
    std::string line;
    std::string headers;
    std::string body;
    bool headerParsed = false;

    // Read line by line
    while (std::getline(stream, line))
    {
        // Trim any trailing carriage return
        if (!line.empty() && line[line.size() - 1] == '\r')
        {
            line.erase(line.size() - 1);
        }

        // Check for the blank line separating headers from the body
        if (line.empty())
        {
            headerParsed = true;
            break;
        }

        // Append to headers
        headers += line + "\n";
    }

    // If headers are parsed, the rest is the body
    if (headerParsed)
    {
        char buffer[1024];
        while (stream.read(buffer, sizeof(buffer)))
        {
            body.append(buffer, stream.gcount());
        }
        body.append(buffer, stream.gcount());
    }

    // Extract the Content-Type from headers
    std::string contentType = "text/html"; // Default content type
    std::istringstream headerStream(headers);
    while (std::getline(headerStream, line))
    {
        if (line.find("Content-Type:") == 0)
        {
            contentType = line.substr(13); // Extract content type value
            // Trim any whitespace
            size_t start = contentType.find_first_not_of(" \t");
            size_t end = contentType.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos)
            {
                contentType = contentType.substr(start, end - start + 1);
            }
            else
            {
                contentType = ""; // No valid content type found
            }
            break;
        }
    }

    return std::make_pair(contentType, body);
}
