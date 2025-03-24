#pragma once

#include "RequestParser.hpp"

class ResponseBuilder
{
private:
	std::string response; // The html response that will be sent to the client
	std::string http_version;
	std::string status;
	std::map<std::string, std::string> headers;
	std::string body;
	short status_code;
	std::map<std::string, void (ResponseBuilder::*)(void)> routes; // A map to route the request to the correct method
	RequestParser request;
	const ServerConfig *server_config; // Pointer to the matched server block
	const Location *location_config;   // Pointer to the matched location block

	// Required HTTP Methods
	void doGET();
	void doPOST();
	void doDELETE();

	// Helper Methods
	void init_config();
	void init_routes();
	bool handle_redirection();
	std::string generate_error_page(short status_code);
	std::string generate_directory_listing(const std::string &path);
	std::string generate_response_string();
	std::string detect_mime_type(const std::string &path);
	void include_required_headers();
	bool is_cgi_request(const std::string &file_path);
	std::string get_http_date();
	std::string read_html_file(const std::string &filename);
	std::string generate_upload_success_page(const std::string &filename);

	// Map to save the mime types
	static std::map<std::string, std::string> mime_types;

	// Method to initialize the mime types
	static std::map<std::string, std::string> init_mime_types();

	ResponseBuilder(const ResponseBuilder &);
	ResponseBuilder &operator=(const ResponseBuilder &);

public:
	ResponseBuilder(RequestParser &request);

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
	std::string get_header_value(std::string &key);
	std::string get_body();
	short get_status_code();

	// Main Function that builds the response
	std::string build_response();
};
