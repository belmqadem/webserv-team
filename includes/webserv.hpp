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
#define STATUS_100 "100 Continue"
#define STATUS_101 "101 Switching Protocols"
#define STATUS_102 "102 Processing"
#define STATUS_200 "200 OK"
#define STATUS_201 "201 Created"
#define STATUS_202 "202 Accepted"
#define STATUS_203 "203 Non-authoritative Information"
#define STATUS_204 "204 No Content"
#define STATUS_205 "205 Reset Content"
#define STATUS_206 "206 Partial Content"
#define STATUS_207 "207 Multi-Status"
#define STATUS_208 "208 Already Reported"
#define STATUS_226 "226 IM Used"
#define STATUS_300 "300 Multiple Choices"
#define STATUS_301 "301 Moved Permanently"
#define STATUS_302 "302 Found"
#define STATUS_303 "303 See Other"
#define STATUS_304 "304 Not Modified"
#define STATUS_305 "305 Use Proxy"
#define STATUS_307 "307 Temporary Redirect"
#define STATUS_308 "308 Permanent Redirect"
#define STATUS_400 "400 Bad Request"
#define STATUS_401 "401 Unauthorized"
#define STATUS_402 "402 Payment Required"
#define STATUS_403 "403 Forbidden"
#define STATUS_404 "404 Not Found"
#define STATUS_405 "405 Method Not Allowed"
#define STATUS_406 "406 Not Acceptable"
#define STATUS_407 "407 Proxy Authentication Required"
#define STATUS_408 "408 Request Timeout"
#define STATUS_409 "409 Conflict"
#define STATUS_410 "410 Gone"
#define STATUS_411 "411 Length Required"
#define STATUS_412 "412 Precondition Failed"
#define STATUS_413 "413 Payload Too Large"
#define STATUS_414 "414 Request-URI Too Long"
#define STATUS_415 "415 Unsupported Media Type"
#define STATUS_416 "416 Requested Range Not Satisfiable"
#define STATUS_417 "417 Expectation Failed"
#define STATUS_418 "418 I'm a teapot"
#define STATUS_421 "421 Misdirected Request"
#define STATUS_422 "422 Unprocessable Entity"
#define STATUS_423 "423 Locked"
#define STATUS_424 "424 Failed Dependency"
#define STATUS_426 "426 Upgrade Required"
#define STATUS_429 "429 Too Many Requests"
#define STATUS_431 "431 Request Header Fields Too Large"
#define STATUS_444 "444 Connection Closed Without Response"
#define STATUS_451 "451 Unavailable For Legal Reasons"
#define STATUS_499 "499 Client Closed Request"
#define STATUS_500 "500 Internal Server Error"
#define STATUS_501 "501 Not Implemented"
#define STATUS_502 "502 Bad Gateway"
#define STATUS_503 "503 Service Unavailable"
#define STATUS_504 "504 Gateway Timeout"
#define STATUS_505 "505 HTTP Version Not Supported"
#define STATUS_506 "506 Variant Also Negotiates"
#define STATUS_507 "507 Insufficient Storage"
#define STATUS_508 "508 Loop Detected"
#define STATUS_510 "510 Not Extended"
#define STATUS_511 "511 Network Authentication Required"
#define STATUS_599 "599 Network Connect Timeout Error"
#define UNDEFINED_STATUS "UNDEFINED HTTP STATUS CODE"

// SPECIAL CHARACTERS
#define SP " "
#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"

// PROGRAM MACROS
#define WEBSERV_NAME "Not Nginx/4.2"
#define DEFAULT_CONF "config/webserv.conf"
#define LOG_FILE "Webserv.log"
#define USAGE(progname) "Usage " + std::string(progname) + " [/path/to/config/file]"

// PARSE ERRORS
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
#define HTTP_PARSE_PAYLOAD_TOO_LARGE "Client sent a request with large body size"
#define HTTP_PARSE_URI_TOO_LONG "Client sent a request with uri too long"
#define HTTP_PARSE_HEADER_FIELDS_TOO_LARGE "Client sent a request with too large header fields"
#define HTTP_PARSE_METHOD_NOT_IMPLEMENTED "Client sent a request with unimplemented method"
#define HTTP_PARSE_HTTP_VERSION_NOT_SUPPORTED "Client sent a request with unsupported http version"
#define HTTP_PARSE_INVALID_EXPECT_VALUE "Client sent a request with invalid expect header value"
#define HTTP_PARSE_NO_LOCATION_BLOCK "Server has no location block defined in config file"
#define HTTP_PARSE_NULL_BYTE "Client attempting null byte injection via %00"

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
#include <limits>
#include "Logger.hpp"
#include "Exceptions.hpp"