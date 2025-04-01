#pragma once

#include "IEvenetListeners.hpp"
#include "ClientServer.hpp"
#include "ResponseBuilder.hpp"

class RequestParser;
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
	std::string root_path;

	// New fields for async operation
	pid_t pid;
	int output_fd;
	std::string cgi_output;
	ResponseBuilder *responseBuilder;
	ClientServer *clientServer;
	bool isCompleted;
	time_t startTime;

	// Helper method to set up environment variables
	void setupEnvironment(std::vector<std::string> &env);
	void finalizeCGI(); // Helper method to finalize CGI processing
public:
	CGIHandler(RequestParser &request, const std::string &php_cgi_path,
			   ResponseBuilder *response, ClientServer *client);
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

	ResponseBuilder *getResponseBuilder() const { return responseBuilder; }
};
