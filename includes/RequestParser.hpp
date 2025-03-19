#pragma once

#include "ConfigManager.hpp"

typedef uint8_t byte; // 8 bit unsigned integers

// This is an enum to save the parsing state of the request
enum ParseState
{
	REQUEST_LINE,
	HEADERS,
	BODY,
	DONE,
	ERROR_PARSE
};

class RequestParser
{
private:
	ParseState state;
	std::string request_line;
	std::string http_method;
	std::string request_uri;
	std::string query_string;
	std::string http_version;
	std::map<std::string, std::string> headers;
	std::vector<byte> body;
	uint16_t port;
	size_t bytes_read;
	size_t body_size;
	short error_code;
	bool has_content_length;
	bool has_transfer_encoding;
	bool is_headers_completed;
	bool is_body_completed;

	const ServerConfig *server_config; // Pointer to the matched server block
	const Location *location_config;   // Pointer to the matched location block

	// Private Helper Methods
	const char *parse_request_line(const char *pos, const char *end);
	const char *parse_headers(const char *pos, const char *end);
	const char *parse_body(const char *pos, const char *end);
	const char *parse_chunked_body(const char *pos, const char *end);
	const char *find_line_end(const char *pos, const char *end);
	std::string normalize_uri(const std::string &uri);
	std::string decode_percent_encoding(const std::string &str);
	bool is_valid_header_name(const std::string &name);
	bool is_valid_header_value(const std::string &value);
	void log_error(const std::string &error_str, short error_code);
	void match_location(std::vector<ServerConfig> &servers);

	// Restrict copying and assigning object
	RequestParser(const RequestParser &other);
	RequestParser &operator=(const RequestParser &other);

public:
	RequestParser(const std::string &request, std::vector<ServerConfig> &servers);

	size_t parse_request(const std::string &request);
	void print_request();

	// Setters
	void set_request_line();
	bool set_http_method(const std::string &http_method);
	bool set_request_uri(const std::string &request_uri);
	void set_query_string(const std::string &query_string);
	bool set_http_version(const std::string &http_version);
	void set_headers(const std::string &key, const std::string &value);
	void set_body(std::vector<byte> &body);
	void set_error_code(short error_code);

	// Getters
	std::string &get_request_line();
	std::string &get_http_method();
	std::string &get_request_uri();
	std::string &get_query_string();
	std::string &get_http_version();
	std::map<std::string, std::string> &get_headers();
	std::string &get_header_value(const std::string &key);
	std::vector<byte> &get_body();
	size_t &get_body_size();
	short &get_error_code();
	uint16_t &get_port_number();
	ParseState &get_state();
	const ServerConfig *get_server_config();
	const Location *get_location_config();

	// Public Helper methods
	bool is_connection_keep_alive();
	bool is_connection_close();
	bool content_length_exists();
	bool transfer_encoding_exists();
	bool headers_completed();
	bool body_completed();
};
