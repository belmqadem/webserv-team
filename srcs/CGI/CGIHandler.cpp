#include "CGIHandler.hpp"
#include "ResponseBuilder.hpp"

#define ROOT_DIRECTORY "www/html"

CGIHandler::CGIHandler(RequestParser &request, const std::string &php_cgi_path, ResponseBuilder* response)
    : pid(-1), output_fd(-1), isCompleted(false), responseBuilder(response)
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

void CGIHandler::startCGI()
{
    Logger &logger = Logger::getInstance();
    int cgi_socket[2];
    
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, cgi_socket) == -1)
    {
        throw std::runtime_error("500 Internal Server Error: Socketpair creation failed");
    }
    
    // Make the socket non-blocking for asynchronous I/O
    fcntl(cgi_socket[0], F_SETFL, O_NONBLOCK);
    
    pid = fork();
    if (pid == -1)
    {
        close(cgi_socket[0]);
        close(cgi_socket[1]);
        logger.error("Fork failed");
        throw std::runtime_error("500 Internal Server Error: Fork failed");
    }
    
    if (pid == 0) // Child process
    {
        // Close the parent's end
        close(cgi_socket[0]);
        
        // Redirect stdout to socket
        dup2(cgi_socket[1], STDOUT_FILENO);
        close(STDIN_FILENO); // Close stdin first to avoid issues
        // Only redirect stdin for POST requests
        if (method == "POST")
            dup2(cgi_socket[1], STDIN_FILENO);
            
        close(cgi_socket[1]); // Close after dup2
        
        // Prepare arguments
        std::ostringstream oss;
        oss << body.length();
        std::vector<std::string> args;
        args.push_back(interpreter);
        args.push_back(scriptPath);
        char *argv[args.size() + 1];
        for (size_t i = 0; i < args.size(); ++i)
            argv[i] = const_cast<char *>(args[i].c_str());
        argv[args.size()] = NULL;
        
        // Prepare environment variables
        std::vector<std::string> env;
        env.push_back("REQUEST_METHOD=" + method);
        env.push_back("QUERY_STRING=" + queryString);
        env.push_back("CONTENT_LENGTH=" + oss.str());
        env.push_back("CONTENT_TYPE=" + headers["Content-Type"]);
        env.push_back("REDIRECT_STATUS=200");
        env.push_back("SCRIPT_FILENAME=" + scriptPath);
        env.push_back("SCRIPT_NAME=" + scriptPath);
        env.push_back("DOCUMENT_ROOT=" + std::string(ROOT_DIRECTORY));
        env.push_back("PHP_SELF=" + scriptPath);
        env.push_back("PATH_TRANSLATED=" + scriptPath);
        env.push_back("REQUEST_URI=" + responseBuilder->request.get_request_uri());
        env.push_back("SERVER_SOFTWARE=webserv/1.0");
        env.push_back("SERVER_PROTOCOL=HTTP/1.1");
        env.push_back("GATEWAY_INTERFACE=CGI/1.1");
        env.push_back("SERVER_NAME=localhost");
        char *envp[env.size() + 1];
        for (size_t i = 0; i < env.size(); ++i)
            envp[i] = const_cast<char *>(env[i].c_str());
        envp[env.size()] = NULL;
        
        // Debug the environment variables 
        Logger::getInstance().info("Executing: " + interpreter + " " + scriptPath);

        // Execute the CGI script with error checking
        if (execve(argv[0], argv, envp) == -1) {
            std::cerr << "CGI Execution failed: " << strerror(errno) << std::endl;
            exit(1);
        }
    }
    else // Parent process
    {
        // Close the child's end
        close(cgi_socket[1]);
        
        // Store the output file descriptor
        output_fd = cgi_socket[0];
        
        // Critical fix - ensure non-blocking IO 
        if (fcntl(output_fd, F_SETFL, O_NONBLOCK) == -1) {
            Logger::getInstance().error("Failed to set non-blocking: " + std::string(strerror(errno)));
        }
        
        // Write POST data immediately, don't rely on event system for this
        if (method == "POST" && !body.empty()) {
            write(output_fd, body.c_str(), body.length());
            // Important - write returns immediately for non-blocking sockets
            // No need to check return value here
        }
        
        // Register with MORE debug info
        epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;  // Add Edge Triggered mode
        ev.data.fd = output_fd;
        
        try {
            IOMultiplexer::getInstance().addListener(this, ev);
            Logger::getInstance().info("CGI listener registered with fd: " + to_string(output_fd));
        } catch (const std::exception& e) {
            Logger::getInstance().error("Failed to register CGI: " + std::string(e.what()));
            throw;
        }
        
        logger.info("CGI process started with PID: " + to_string(pid));
    }
}

void CGIHandler::onEvent(int fd, epoll_event ev)
{
    Logger &logger = Logger::getInstance();
    
    logger.info("CGI onEvent called! fd=" + to_string(fd) + " events=" + to_string(ev.events));
    
    if (fd == output_fd)
    {
        if (ev.events & EPOLLIN)
        {
            // Read data from the CGI process
            char buffer[1024];
            ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
            
            if (bytesRead > 0)
            {
                // Add the data to our output buffer
                buffer[bytesRead] = '\0';
                cgi_output += buffer;
                logger.info("Read " + to_string(bytesRead) + " bytes from CGI process");
            }
            else if (bytesRead == 0 || (bytesRead == -1 && errno != EAGAIN))
            {
                // End of data or error
                logger.info("CGI process complete, processing output");
                
                // Remove from IOMultiplexer
                epoll_event ev;
                ev.data.fd = output_fd;
                IOMultiplexer::getInstance().removeListener(ev, output_fd);
                
                // Close the file descriptor
                close(output_fd);
                output_fd = -1;
                
                // Wait for the process to complete
                int status;
                waitpid(pid, &status, 0);
                pid = -1;
                
                // Parse the CGI output and update the response builder
                if (responseBuilder)
                {
                    try
                    {
                        std::pair<std::string, std::string> parsed = parseCGIOutput();
                        /* debug */
                        std::cout << parsed.first << std::endl;
                        /* SIGFAULT HAPPENS HERE HELP! */
                        responseBuilder->set_headers("Content-Type", parsed.first);
                        /***************************** */
                        responseBuilder->set_body(parsed.second);
                        responseBuilder->set_status(200);
                    }
                    catch (const std::exception &e)
                    {
                        logger.error(e.what());
                        responseBuilder->set_status(500);
                        responseBuilder->set_body(responseBuilder->generate_error_page(500));
                    }
                }
                
                isCompleted = true;
            }
        }
        
        if (ev.events & (EPOLLERR | EPOLLHUP))
        {
            // Handle errors
            logger.error("Error with CGI process");
            
            // Remove from IOMultiplexer
            epoll_event ev;
            ev.data.fd = output_fd;
            IOMultiplexer::getInstance().removeListener(ev, output_fd);
            
            // Close the file descriptor
            close(output_fd);
            output_fd = -1;
            
            // Terminate the process if still running
            if (pid > 0)
            {
                kill(pid, SIGTERM);
                int status;
                waitpid(pid, &status, 0);
                pid = -1;
            }
            
            // Set error in response builder
            if (responseBuilder)
            {
                responseBuilder->set_status(500);
                responseBuilder->set_body(responseBuilder->generate_error_page(500));
            }
            
            isCompleted = true;
        }
    }
}

void CGIHandler::terminate()
{
    Logger &logger = Logger::getInstance();
    
    // Remove from IOMultiplexer if still registered
    if (output_fd != -1)
    {
        epoll_event ev;
        ev.data.fd = output_fd;
        try
        {
            IOMultiplexer::getInstance().removeListener(ev, output_fd);
        }
        catch (const std::exception &e)
        {
            logger.error("Error removing CGI listener: " + std::string(e.what()));
        }
        
        // Close the file descriptor
        close(output_fd);
        output_fd = -1;
    }
    
    // Kill the CGI process if still running
    if (pid > 0)
    {
        logger.info("Killing CGI process: " + to_string(pid));
        kill(pid, SIGTERM);
        int status;
        waitpid(pid, &status, 0);
        pid = -1;
    }
    
    isCompleted = true;
}

std::pair<std::string, std::string> CGIHandler::parseCGIOutput()
{
    std::string contentType = "text/html"; // Default
    std::string body;
    
    size_t headerEnd = cgi_output.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        headerEnd = cgi_output.find("\n\n");
        if (headerEnd == std::string::npos)
        {
            // No headers, treat everything as body
            return std::make_pair(contentType, cgi_output);
        }
        headerEnd += 2;
    }
    else
    {
        headerEnd += 4;
    }
    
    // Extract headers
    std::string headers = cgi_output.substr(0, headerEnd);
    
    // Look for Content-Type in headers
    size_t ctPos = headers.find("Content-Type:");
    if (ctPos != std::string::npos)
    {
        size_t ctStart = headers.find_first_not_of(" \t", ctPos + 13);
        size_t ctEnd = headers.find("\n", ctStart);
        
        if (ctEnd != std::string::npos && ctStart < ctEnd)
        {
            contentType = headers.substr(ctStart, ctEnd - ctStart);
            if (contentType[contentType.size() - 1] == '\r')
                contentType.erase(contentType.size() - 1);
        }
    }
    
    // Extract body
    body = cgi_output.substr(headerEnd);
    
    return std::make_pair(contentType, body);
}
