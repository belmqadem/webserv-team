#pragma once

#include "webserv.hpp"
#include "CGIHandler.hpp"
class RequestParser;

class ResponseBuilder
{
private:
	std::string response; // This is the response that will be sent to the client
	std::string http_version;
	std::string status;
	std::map<std::string, std::string> headers;
	std::string body;
	short status_code;
	std::map<std::string, void (ResponseBuilder::*)(RequestParser &)> routes; // A map to route the request to the correct method

	// Required HTTP Methods
	void doPOST(RequestParser &request);
	void doDELETE(RequestParser &request);
	
	// Helper Methods
	void init_routes();
	bool handle_redirection();
	std::string generate_error_page(short status_code, const std::string &message);
	std::string generate_directory_listing(const std::string &path);
	std::string generate_response_string();
	std::string detect_mime_type(const std::string &path);
	bool is_cgi_request(const std::string &file_path);
	
	// A map to save the mime types
	static std::map<std::string, std::string> mime_types;
	
	ResponseBuilder(const ResponseBuilder &);
	ResponseBuilder &operator=(const ResponseBuilder &);
	
	public:
	ResponseBuilder(RequestParser &request);
	void doGET(RequestParser &request);

	// Setters
	void set_http_version(const std::string &http_version);
	void set_status(short status_code);
	void set_headers(const std::string &key, const std::string &value);
	void set_body(const std::string &body);

	// Getters
	std::string get_response();
	std::string get_http_version();
	std::string get_status();
	std::map<std::string, std::string> get_headers();
	std::string get_header(std::string &key);
	std::string get_body();
	short get_status_code();

	// Method to initialize the mime types
	static std::map<std::string, std::string> init_mime_types();

	// Core Function that builds the response
	std::string build_response(RequestParser &request);
};
