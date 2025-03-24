#pragma once

// COLORS
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD_RED "\033[1;31m"
#define BOLD_GREEN "\033[1;32m"
#define BOLD_BLUE "\033[1;34m"
#define BOLD_MAGENTA "\033[1;35m"
#define BOLD_YELLOW "\033[1;33m"
#define BOLD_CYAN "\033[1;36m"
#define BOLD_WHITE "\033[1;37m"
#define UNDERLINE "\033[4m"

// STATUS CODES
#define STATUS_200 "200 OK"
#define STATUS_201 "201 Created"
#define STATUS_204 "204 No Content"
#define STATUS_301 "301 Moved Permanently"
#define STATUS_302 "302 Found"
#define STATUS_303 "303 See Other"
#define STATUS_304 "304 Not Modified"
#define STATUS_400 "400 Bad Request"
#define STATUS_403 "403 Forbidden"
#define STATUS_404 "404 Not Found"
#define STATUS_405 "405 Method Not Allowed"
#define STATUS_409 "409 Conflict"
#define STATUS_410 "410 Gone"
#define STATUS_413 "413 Payload Too Large"
#define STATUS_414 "414 URI Too Long"
#define STATUS_415 "415 Unsupported Media Type"
#define STATUS_431 "431 Request Header Fields Too Large"
#define STATUS_500 "500 Internal Server Error"
#define STATUS_501 "501 Not Implemented"
#define STATUS_505 "505 HTTP Version Not Supported"

// SPECIAL CHARACTERS
#define SP " "
#define CRLF "\r\n"

// PROGRAM MACROS
#define WEBSERV_NAME "Not Nginx/4.2"
#define DEFAULT_CONF "config/webserv.conf"
#define LOG_FILE "Webserv.log"
#define USAGE(progname) "Usage " + std::string(progname) + " [/path/to/config/file]"

// ERROR MESSAGES (Response: 400 Bad Request)
#define HTTP_PARSE_INVALID_REQUEST_LINE "Client sent a request with invalid request line"
#define HTTP_PARSE_INVALID_METHOD "Client sent a request with invalid http method"
#define HTTP_PARSE_INVALID_VERSION "Client sent a request with invalid http version"
#define HTTP_PARSE_MISSING_REQUEST_URI "Client sent a request without uri"
#define HTTP_PARSE_INVALID_URI "Client sent a request with invalid uri"
#define HTTP_PARSE_MISSING_HOST "Client sent a request without host header"
#define HTTP_PARSE_INVALID_HOST "Client sent a request with invalid host header"
#define HTTP_PARSE_INVALID_CONTENT_LENGTH "Client sent a request with invalid content-length value"
#define HTTP_PARSE_INVALID_HEADER_FIELD "Client sent a request with malformed header field"
#define HTTP_PARSE_INVALID_CHUNKED_TRANSFER "Client sent a request with invalid chunked body"
#define HTTP_PARSE_INVALID_TRANSFER_ENCODING "Client sent a request with invalid transfer-encoding value"
#define HTTP_PARSE_INVALID_PERCENT_ENCODING "Client sent a request with invalid percent encoding"
#define HTTP_PARSE_CONFLICTING_HEADERS "Client sent a request with transfer-encoding and content-length"
#define HTTP_PARSE_INVALID_PORT "Client sent a request with invalid port number"
#define HTTP_PARSE_INVALID_LOCATION "Client sent a request with invalid location"
#define HTTP_PARSE_MISSING_CONTENT_LENGTH "Client sent a request without content-length header"
#define HTTP_PARSE_PAYLOAD_TOO_LARGE "Client sent a request with payload too large"
#define HTTP_PARSE_URI_TOO_LONG "Client sent a request with uri too long"
#define HTTP_PARSE_HEADER_FIELDS_TOO_LARGE "Client sent a request with too large header fields"
#define HTTP_PARSE_METHOD_NOT_IMPLEMENTED "Client sent a request with unimplemented method"
#define HTTP_PARSE_HTTP_VERSION_NOT_SUPPORTED "Client sent a request with unsupported http version"

// HEADER FILES
#include <ctime>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <cstring>
#include <sys/epoll.h>
struct ServerConfig;

// Utils Functions
std::vector<std::string> split(const char *start, const char *end, char delimiter);
std::string trim(const std::string &str, const std::string &delim);
bool is_numeric(const std::string &str);
std::string get_timestamp_str();
void sigint_handle(int sig);
void signalhandler();
void printServerConfig(const ServerConfig &server);

// Template Function
template <class T>
std::string to_string(T t)
{
	std::stringstream str;
	str << t;
	return str.str();
}
