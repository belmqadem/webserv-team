#pragma once

#include "ClientServer.hpp"
#include "ResponseBuilder.hpp"

class RequestParser;
class ClientServer;

class CGIHandler : public IEvenetListeners
{
private:
	std::string scriptPath;
	std::string method;
	std::string uri;
	std::string queryString;
	std::map<std::string, std::string> headers;
	std::string root_path;

	// New fields for async operation
	pid_t pid;
	int output_fd;
	std::string cgi_output;
	ResponseBuilder *responseBuilder;
	ClientServer *clientServer;
	bool isCompleted;
	std::time_t startTime;
	std::string interpreter;

	int bodyFd;
	std::string contentType;

	void setupEnvironment(std::vector<std::string> &env);
	void finalizeCGI();

public:
	CGIHandler(RequestParser &request, const std::string &cgi_path, ResponseBuilder *response, ClientServer *client);
	~CGIHandler();

	// IEvenetListeners interface implementation
	void onEvent(int fd, epoll_event ev);
	void terminate();

	void keepClientAlive();
	void startCGI();

	void processCGIOutput();

	bool isProcessCompleted() const;
	std::time_t getStartTime() const;
	ResponseBuilder *getResponseBuilder() const;
};
