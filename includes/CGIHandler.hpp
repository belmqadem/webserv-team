#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"

class CGIHandler {
	public:
		CGIHandler( RequestParser& request, const ServerConfig& serverConfig, const Location& locationConfig);
		~CGIHandler();
	
		void executeCGI();
		std::string getOutput() const;
	
	private:
		void setupEnvironment();
		void setupArgs();
		void executeScript();
		void readOutput(int sockfd);
		void handlePostRequest(int sockfd);
	
		 RequestParser& _request;
		const ServerConfig& _serverConfig;
		const Location& _locationConfig;
		std::map<std::string, std::string> _env;
		std::vector<std::string> _args;
		std::string _output;
		std::string _cgiOutput;
	
		static void timeoutHandler(int signum);
	};
	

#endif // CGI_HANDLER_HPP