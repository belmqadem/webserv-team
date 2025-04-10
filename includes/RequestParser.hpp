#pragma once

#include "ConfigManager.hpp"

#define MAX_REQUEST_LINE_LENGTH 8192
#define MAX_URI_LENGTH 2048
#define MAX_HEADER_LENGTH 8192
#define MAX_HEADER_COUNT 100

typedef uint8_t byte;

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
	const ServerConfig *server_config;
	const Location *location_config;
	bool reading_chunk_data;
	size_t current_chunk_size;
	size_t current_chunk_read;
	std::string cgi_script;
	bool is_cgi_request_flag;
	size_t content_length_value;

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
	std::string normalize_path(const std::string &path);

public:
	RequestParser();
	RequestParser(const RequestParser &other);
	RequestParser &operator=(const RequestParser &other);

	bool has_content_length;
	bool has_transfer_encoding;

	// Helper methods
	size_t parse_request(const std::string &request);
	void match_location(const std::vector<ServerConfig *> &servers);
	bool is_connection_close();
	bool is_cgi_request();
	void print_request();

	// Setters
	void set_request_line();
	bool set_http_method(const std::string &http_method);
	bool set_request_uri(const std::string &request_uri);
	void set_query_string(const std::string &query_string);
	bool set_http_version(const std::string &http_version);
	void set_port(uint16_t port);

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
	size_t get_content_length_value();
};
