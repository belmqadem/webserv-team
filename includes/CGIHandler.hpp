#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include "RequestParser.hpp"
#include "IEvenetListeners.hpp"
#include "ResponseBuilder.hpp"
class CGIHandler {
	public:
		CGIHandler(RequestParser& request, const ServerConfig& serverConfig, const Location& locationConfig);
		~CGIHandler();
	
		void executeCGI();
		void setupEnvironment();
		void setupArgs();
		void executeScript();
		std::string getOutput() const;
	
	private:
		RequestParser& _request;
		const ServerConfig& _serverConfig;
		const Location& _locationConfig;
		std::map<std::string, std::string> _env;
		std::vector<std::string> _args;
		std::string _cgiOutput;
	};
#endif // CGI_HANDLER_HPP