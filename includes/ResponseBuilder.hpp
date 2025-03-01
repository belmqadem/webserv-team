#pragma once

#include "webserv.hpp"

class RequestParser;

class ResponseBuilder
{
private:
	std::string http_version;
	std::string status;
	std::map<std::string, std::string> headers;
	std::string body;
	short status_code;

	// Required HTTP Methods
	void doGET(RequestParser &request);
	void doPOST(RequestParser &request);
	void doDELETE(RequestParser &request);

	// Helper Method to handle redirections
	void handle_redirection();

	ResponseBuilder(const ResponseBuilder &);
	ResponseBuilder &operator=(const ResponseBuilder &);

public:
	ResponseBuilder();

	// Setters
	void set_http_version(const std::string &http_version);
	void set_status(short status_code, const std::string &status);
	void set_headers(const std::string &key, const std::string &value);
	void set_body(const std::string &body);

	// Getters
	std::string get_http_version();
	std::string get_status();
	std::map<std::string, std::string> get_headers();
	std::string get_header(std::string &key);
	std::string get_body();
	short get_status_code();

	// Core Function that builds the response
	std::string build_response(RequestParser &request);
};
