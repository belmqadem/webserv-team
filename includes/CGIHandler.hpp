#pragma once

#include "Logger.hpp"
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"
#include "Parser.hpp"

class RequestParser;

class CGIHandler
{
private:
	std::string scriptPath;
	std::string interpreter;
	std::string method;
	std::string queryString;
	std::string body;
	std::map<std::string, std::string> headers;

public:
	std::string executeCGI();
	CGIHandler(RequestParser &request, const std::string &php_cgi_path);
};
