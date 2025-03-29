#pragma once

#include "Logger.hpp"
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"
#include "Parser.hpp"
#include "IEvenetListeners.hpp"
#include "ClientServer.hpp"

class RequestParser;
class ResponseBuilder;
class ClientServer;

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
    ResponseBuilder* responseBuilder;
    ClientServer* clientServer;
    bool isCompleted;
	time_t startTime;
    
    // Helper method to set up environment variables
    void setupEnvironment(std::vector<std::string>& env);
	std::pair<std::string, std::string> parseCGIOutput(const std::string& output);
    void finalizeCGI();  // Helper method to finalize CGI processing
public:
    CGIHandler(RequestParser &request, const std::string &php_cgi_path, 
               ResponseBuilder* response, ClientServer* client);
    ~CGIHandler();
    
    // IEvenetListeners interface implementation
    void onEvent(int fd, epoll_event ev);
    void terminate();
    // Start async CGI execution
    void keepClientAlive();
    void startCGI();
    
    // Check if CGI processing is complete
    bool isProcessCompleted() const { return isCompleted; }
	time_t getStartTime() const { return startTime; }
    
    // Process the CGI output when complete
    void processCGIOutput();

    ResponseBuilder* getResponseBuilder() const { return responseBuilder; }
};
