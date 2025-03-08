#pragma once

#include "webserv.hpp"

enum ParseState
{
	REQUEST_LINE,
	HEADERS,
	BODY,
	DONE,
	ERROR
};

class RequestParser
{
private:
	ParseState state;
	std::string http_method;
	std::string request_uri;
	std::string query_string;
	std::string http_version;
	std::map<std::string, std::string> headers;
	std::string body;
	short error_code;

	// Helper Methods to validate the request
	const char *parse_request_line(const char *pos, const char *end);
	const char *parse_headers(const char *pos, const char *end);
	const char *parse_body(const char *pos, const char *end);
	const char *parse_chunked_body(const char *pos, const char *end);
	const char *find_line_end(const char *pos, const char *end);
	std::string normalize_uri(const std::string &uri);
	std::string decode_percent_encoding(const std::string &str);
	bool is_keep_alive();
	bool is_valid_header_name(const std::string &name);
	bool is_valid_header_value(const std::string &value);

	RequestParser(const RequestParser &other);
	RequestParser &operator=(const RequestParser &other);

public:
	RequestParser(const std::string &request);

	// Setters
	bool set_http_method(const std::string &http_method);
	bool set_request_uri(const std::string &request_uri);
	void set_query_string(const std::string &query_string);
	bool set_http_version(const std::string &http_version);
	void set_headers(const std::string &key, const std::string &value);
	void set_body(const std::string &body);
	void set_error_code(short error_code);

	// Getters
	std::string &get_http_method();
	std::string &get_request_uri();
	std::string &get_query_string();
	std::string &get_http_version();
	std::map<std::string, std::string> &get_headers();
	std::string &get_header_value(const std::string &key);
	std::string &get_body();
	short get_error_code();

	// Main Method
	void parse_request(const std::string &request);
	void print_request();
};
