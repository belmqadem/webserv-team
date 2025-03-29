#pragma once

#include "ConfigManager.hpp"

#define MAX_REQUEST_LINE_LENGTH 8192
#define MAX_URI_LENGTH 2048
#define MAX_HEADER_LENGTH 8192
#define MAX_HEADER_COUNT 100

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
	short error_code;
	bool has_content_length;
	bool has_transfer_encoding;
	const ServerConfig *server_config; // Pointer to the matched server block
	const Location *location_config;   // Pointer to the matched location block

	size_t current_chunk_size; // Size of the current chunk being processed
	size_t current_chunk_read; // How much of the current chunk has been read
	bool reading_chunk_data;   // Are we currently reading chunk data?

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

public:
	RequestParser();
	RequestParser(const RequestParser &other);
	RequestParser &operator=(const RequestParser &other);

	// Public Helper methods
	size_t parse_request(const std::string &request);
	void match_location(const std::vector<ServerConfig> &servers);
	void print_request();
	bool is_connection_close();

	// Setters
	void set_request_line();
	bool set_http_method(const std::string &http_method);
	bool set_request_uri(const std::string &request_uri);
	void set_query_string(const std::string &query_string);
	bool set_http_version(const std::string &http_version);

	// Getters
	std::string &get_request_line();
	std::string &get_http_method();
	std::string &get_request_uri();
	std::string &get_query_string();
	std::string &get_http_version();
	std::map<std::string, std::string> &get_headers();
	std::string &get_header_value(const std::string &key);
	std::vector<byte> &get_body();
	short &get_error_code();
	uint16_t &get_port_number();
	ParseState &get_state();
	const ServerConfig *get_server_config();
	const Location *get_location_config();
};
