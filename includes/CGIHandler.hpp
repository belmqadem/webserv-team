#pragma once

#include "Logger.hpp"
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"
#include "Parser.hpp"
#include "IEvenetListeners.hpp"
#include "IOMultiplexer.hpp"

class ResponseBuilder;

class CGIHandler : public IEvenetListeners
{
private:
    std::string scriptPath;
    std::string interpreter;
    std::string method;
    std::string queryString;
    std::string body;
    std::map<std::string, std::string> headers;
    
    // New fields for async operation
    pid_t pid;
    int output_fd;
    std::string cgi_output;
    bool isCompleted;
    ResponseBuilder* responseBuilder;
    
    // Helper method to parse CGI output
    std::pair<std::string, std::string> parseCGIOutput();

public:
    CGIHandler(RequestParser &request, const std::string &php_cgi_path, ResponseBuilder* response);
    ~CGIHandler();
    
    // Start the CGI process but don't wait for it to complete
    void startCGI();
    
    // Implement IEvenetListeners interface
    virtual void onEvent(int fd, epoll_event ev);
    virtual void terminate();
    
    // Check if CGI process is complete
    bool isProcessCompleted() const { return isCompleted; }
    
    // Get the CGI output
    std::string getOutput() const { return cgi_output; }
};
