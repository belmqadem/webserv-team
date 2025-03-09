#pragma once

// COLORS
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

// STATUS CODES
#define STATUS_200 GREEN "200 OK" RESET
#define STATUS_204 GREEN "204 No Content" RESET
#define STATUS_301 YELLOW "301 Moved Permanently" RESET
#define STATUS_302 YELLOW "302 Found"
#define STATUS_400 RED "400 Bad Request" RESET
#define STATUS_403 RED "403 Forbidden" RESET
#define STATUS_404 RED "404 Not Found" RESET
#define STATUS_405 RED "405 Method Not Allowed" RESET
#define STATUS_409 RED "409 Conflict" RESET
#define STATUS_410 RED "410 Gone" RESET
#define STATUS_413 RED "413 Payload Too Large" RESET
#define STATUS_414 RED "414 URI Too Long" RESET
#define STATUS_431 RED "431 Request Header Fields Too Large" RESET
#define STATUS_500 RED "500 Internal Server Error" RESET
#define STATUS_501 RED "501 Not Implemented" RESET
#define STATUS_505 RED "505 HTTP Version Not Supported" RESET

// SPECIAL CHARACTERS
#define SP " "
#define CR "\r"
#define LF "\n"
#define CRLF "\r\n"
#define CRLF_DOUBLE "\r\n\r\n"

// ERROR MESSAGES (Response: 400 Bad Request)
#define HTTP_PARSE_INVALID_METHOD RED "Error: client sent invalid method" RESET
#define HTTP_PARSE_INVALID_VERSION RED "Error: client sent invalid version" RESET
#define HTTP_PARSE_INVALID_REQUEST_LINE RED "Error: client sent invalid request line" RESET
#define HTTP_PARSE_MISSING_REQUEST_LINE RED "Error: client sent a request without request line" RESET
#define HTTP_PARSE_INVALID_HOST RED "Error: client sent request with invalid Host header" RESET
#define HTTP_PARSE_INVALID_CONTENT_LENGTH RED "Error: client sent request with invalid Content-Length value" RESET
#define HTTP_PARSE_INVALID_HEADER_FIELD RED "Error: client sent request with malformed header field" RESET
#define HTTP_PARSE_INVALID_CHUNKED_BODY RED "Error: client sent request with invalid chunked body" RESET

// ERROR MESSAGES (Response: 411 Lenght Required)
#define HTTP_PARSE_MISSING_CONTENT_LENGTH RED "Error: client sent request without Content-Length header" RESET

// ERROR MESSAGES (Response: 413 Payload Too Large)
#define HTTP_PARSE_PAYLOAD_TOO_LARGE RED "Error: client sent request with payload too large" RESET

// ERROR MESSAGES (Response: 414 URI Too Long)
#define HTTP_PARSE_URI_TOO_LONG RED "Error: client sent request with URI too long" RESET

// ERROR MESSAGES (Response: 431 Request Header Fields Too Large)
#define HTTP_PARSE_HEADER_FIELDS_TOO_LARGE RED "Error: client sent request with header fields too large" RESET

// ERROR MESSAGES (Response: 501 Not Implemented)
#define HTTP_PARSE_METHOD_NOT_IMPLEMENTED RED "Error: client sent request with method not implemented" RESET

// ERROR MESSAGES (Response: 505 HTTP Version Not Supported)
#define HTTP_PARSE_HTTP_VERSION_NOT_SUPPORTED RED "Error: client sent request with unsupported HTTP version" RESET

// HEADER FILES
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
#include <cerrno>
#include <cstring>
#include <poll.h>
#include <sys/epoll.h>
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"

// Utils Functions
std::string &to_upper(std::string &str);
std::string &to_lower(std::string &str);
std::vector<std::string> split(const char *start, const char *end, char delimiter);
std::string trim(const std::string &str, const std::string &delim);
std::string readFile(const std::string &filename);
bool writeFile(const std::string &filename, const std::string &content);
std::string getCurrentTime();


#define USAGE(progname) "Usage " + std::string(progname) + " [/path/to/config/file]"
