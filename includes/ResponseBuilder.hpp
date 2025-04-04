#pragma once

#include "RequestParser.hpp"
#include "SessionCookieHandler.hpp"

class ResponseBuilder
{
private:
	RequestParser request; // The request that will be processed
	std::string response;  // The response that will be sent to the client
	std::string http_version;
	std::string status;
	std::map<std::string, std::string> headers;
	std::string body;
	short status_code;
	std::map<std::string, void (ResponseBuilder::*)(void)> routes; // A map to route the request to the correct method
	const ServerConfig *server_config;							   // Pointer to the matched server block
	const Location *location_config;							   // Pointer to the matched location block

	// Required HTTP Methods
	void doGET();
	void doPOST();
	void doDELETE();

	// Helper Methods
	void init_config();
	void init_routes();
	bool handle_redirection();
	std::string generate_default_root();
	std::string generate_directory_listing(const std::string &path);
	std::string generate_response_string();
	std::string detect_mime_type(const std::string &path);
	void include_required_headers();
	std::string get_http_date();
	std::string read_file(const std::string &filename);
	std::string generate_upload_success_page(const std::string &filename);
	bool handleMultipartFormData(std::string &content_type, std::vector<byte> &req_body);
	bool validate_upload_path(const std::string &upload_path);
	bool save_uploaded_file(const std::string &full_path, const std::vector<byte> &req_body);

	static std::map<std::string, std::string> mime_types;
	static std::map<std::string, std::string> init_mime_types();

	ResponseBuilder(const ResponseBuilder &);
	ResponseBuilder &operator=(const ResponseBuilder &);

	public:
	ResponseBuilder(RequestParser &request);
	std::string build_response();
	
	// Helper Methods
	std::string generate_error_page();
	std::string generate_response_only();

	// Setters
	void set_status(short status_code);
	void set_headers(const std::string &key, const std::string &value);
	void set_all_headers(const std::map<std::string, std::string> &headers);
	void set_body(const std::string &body);

	// Getters
	RequestParser getRequest() const;
	std::string get_response();
	std::string get_http_version();
	std::string get_status();
	std::map<std::string, std::string> get_headers();
	std::string get_header_value(std::string &key);
	std::string get_body();
	short get_status_code();
	const ServerConfig *get_server_config();
	const Location *get_location_config();
};
