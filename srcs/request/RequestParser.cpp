#include "../../includes/RequestParser.hpp"

// Constructor
RequestParser::RequestParser(const std::string &request)
{
	this->state = REQUEST_LINE; // Starting by Request Line (Start line of the request)
	this->http_method = "";
	this->request_uri = "";
	this->query_string = "";
	this->http_version = "";
	this->headers = std::map<std::string, std::string>();
	this->body = "";
	this->error_code = 1;	// If no error, error_code is set to 1 else it will be set to the error code that i will return in the response directly
	parse_request(request); // Here we parse the request
}

// Main Function that parses the request
void RequestParser::parse_request(const std::string &request)
{
	const char *start = request.c_str();
	const char *end = start + request.size();
	const char *pos = start;

	while (pos < end && state != DONE && state != ERROR_PARSE)
	{
		switch (state)
		{
		case REQUEST_LINE:
			pos = parse_request_line(pos, end);
			break;
		case HEADERS:
			pos = parse_headers(pos, end);
			break;
		case BODY:
			pos = parse_body(pos, end);
			break;
		default:
			state = ERROR_PARSE;
		}
	}
}

// Method to extract the request line
const char *RequestParser::parse_request_line(const char *pos, const char *end)
{
	const char *line_end = find_line_end(pos, end); // Points to the end of the line (after CRLF)

	// Check for long Request line > 8k bytes
	if ((line_end - pos) > MAX_REQUEST_LINE_LENGTH)
	{
		std::cerr << HTTP_PARSE_URI_TOO_LONG << std::endl;
		error_code = 414;
		state = ERROR_PARSE;
		return pos;
	}

	if (line_end == end) // If no CRLF found in the request
	{
		std::cerr << HTTP_PARSE_INVALID_REQUEST << std::endl;
		error_code = 400;
		state = ERROR_PARSE;
		return pos;
	}

	// Split the request line by exactly one SP and check if its 3 parts ([METHOD] SP [URI] SP [VERSION])
	std::vector<std::string> parts = split(pos, line_end - 2, ' ');
	if (parts.size() != 3)
	{
		std::cerr << HTTP_PARSE_INVALID_REQUEST_LINE << std::endl;
		error_code = 400;
		state = ERROR_PARSE;
		return pos;
	}

	// Check for all parts of request line before setting them
	if (!set_http_method(parts[0]) || !set_request_uri(parts[1]) || !set_http_version(parts[2]))
	{
		state = ERROR_PARSE;
		return pos;
	}
	state = HEADERS;
	return line_end;
}

// Method to extract headers from the request
const char *RequestParser::parse_headers(const char *pos, const char *end)
{
	bool has_content_length = false;
	bool has_host = false;
	std::string content_length_value;

	while (pos < end)
	{
		const char *header_end = find_line_end(pos, end);

		// If an empty line that means end of headers
		if (pos == header_end - 2)
		{
			// Ensure Host is present in headers
			if (!has_host)
			{
				std::cerr << HTTP_PARSE_MISSING_HOST << std::endl;
				error_code = 400;
				state = ERROR_PARSE;
				return pos;
			}
			state = BODY;
			return header_end;
		}

		// Reject obsolete line folding
		if (*pos == ' ' || *pos == '\t')
		{
			std::cerr << HTTP_PARSE_INVALID_HEADER_FIELD << std::endl;
			error_code = 400;
			state = ERROR_PARSE;
			return pos;
		}

		// Header field too large
		if ((header_end - pos) > MAX_HEADER_LENGTH)
		{
			std::cerr << HTTP_PARSE_HEADER_FIELDS_TOO_LARGE << std::endl;
			error_code = 431;
			state = ERROR_PARSE;
			return pos;
		}

		std::string header_line(pos, header_end - 2);
		size_t colon_pos = header_line.find(':');

		// If no colon found or no header key found or space before colon
		if (colon_pos == std::string::npos || colon_pos == 0 || header_line[colon_pos - 1] == ' ')
		{
			std::cerr << HTTP_PARSE_INVALID_HEADER_FIELD << std::endl;
			error_code = 400;
			state = ERROR_PARSE;
			return pos;
		}

		std::string key = trim(header_line.substr(0, colon_pos), " \t");
		std::string value = trim(header_line.substr(colon_pos + 1), " \t");

		// Convert header names to lowercase
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);

		// Validate header name (only a-z, A-Z, 0-9, `-`, `_`)
		if (!is_valid_header_name(key))
		{
			std::cerr << HTTP_PARSE_INVALID_HEADER_FIELD << std::endl;
			error_code = 400;
			state = ERROR_PARSE;
			return pos;
		}

		// Validate header value (no control characters except tab & space)
		if (!is_valid_header_value(value))
		{
			std::cerr << HTTP_PARSE_INVALID_HEADER_FIELD << std::endl;
			error_code = 400;
			state = ERROR_PARSE;
			return pos;
		}

		// Special Handling for `Content-Length`
		if (key == "content-length")
		{
			// Check for numeric value
			if (!is_numeric(value))
			{
				std::cerr << HTTP_PARSE_INVALID_CONTENT_LENGTH << std::endl;
				error_code = 400;
				state = ERROR_PARSE;
				return pos;
			}

			if (has_content_length && content_length_value != value)
			{
				std::cerr << HTTP_PARSE_INVALID_CONTENT_LENGTH << std::endl;
				error_code = 400;
				state = ERROR_PARSE;
				return pos;
			}

			has_content_length = true;
			content_length_value = value;
		}

		// Special Handling for `Transfer-Encoding`
		if (key == "transfer-encoding")
		{
			if (has_content_length)
			{
				std::cerr << HTTP_PARSE_CONFLICTING_HEADERS << std::endl;
				error_code = 400;
				state = ERROR_PARSE;
				return pos;
			}

			if (value != "chunked" && value != "compress" && value != "deflate" && value != "gzip")
			{
				std::cerr << HTTP_PARSE_INVALID_TRANSFER_ENCODING << std::endl;
				error_code = 400;
				state = ERROR_PARSE;
				return pos;
			}
		}

		// Special Handling for `Host`
		if (key == "host")
		{
			if (has_host)
			{
				std::cerr << HTTP_PARSE_INVALID_HOST << std::endl;
				error_code = 400;
				state = ERROR_PARSE;
				return pos;
			}
			has_host = true;
		}

		// Special Handling for `Connection`
		if (key == "connection" && value != "keep-alive" && value != "close")
		{
			std::cerr << HTTP_PARSE_INVALID_CONNECTION_HEADER << std::endl;
			error_code = 400;
			state = ERROR_PARSE;
			return pos;
		}

		// Special Handling for `Expect`
		if (key == "expect" && value != "100-continue")
		{
			std::cerr << HTTP_PARSE_INVALID_EXPECT_HEADER << std::endl;
			error_code = 417;
			state = ERROR_PARSE;
			return pos;
		}

		// Special Handling for `Upgrade`
		if (key == "upgrade" && value != "h2c" && value != "websocket")
		{
			std::cerr << HTTP_PARSE_INVALID_UPGRADE_HEADER << std::endl;
			error_code = 400;
			state = ERROR_PARSE;
			return pos;
		}

		if (headers.size() >= MAX_HEADER_COUNT)
		{
			std::cerr << HTTP_PARSE_HEADER_FIELDS_TOO_LARGE << std::endl;
			error_code = 431;
			state = ERROR_PARSE;
			return pos;
		}

		headers[key] = value;
		pos = header_end;
	}
	return pos;
}

// Method to extract body of the request
const char *RequestParser::parse_body(const char *pos, const char *end)
{
	std::map<std::string, std::string>::iterator it = headers.find("transfer-encoding");

	// Case 1: Transfer-Encoding: chunked
	if (it != headers.end())
	{
		if (it->second == "chunked")
			return parse_chunked_body(pos, end);
		else
		{
			std::cerr << HTTP_PARSE_METHOD_NOT_IMPLEMENTED << std::endl;
			error_code = 501;
			state = ERROR_PARSE;
			return pos;
		}
	}

	// Case 2: Content-Length is present
	it = headers.find("content-length");
	if (it != headers.end())
	{
		char *endptr = NULL;
		std::string content_length_str = it->second;
		unsigned long content_length = std::strtoul(content_length_str.c_str(), &endptr, 10);

		// Check for invalid content length
		if (*endptr != '\0' || content_length > static_cast<size_t>(end - pos))
		{
			std::cerr << HTTP_PARSE_INVALID_CONTENT_LENGTH << std::endl;
			error_code = 400;
			state = ERROR_PARSE;
			return pos;
		}

		if (content_length == 0) // No body
		{
			state = DONE;
			return pos;
		}

		// Read exactly Content-Length bytes
		body.assign(pos, pos + content_length);
		pos += content_length;
		state = DONE;
		return pos;
	}
	else // No Content-Length header
	{
		if (http_method == "POST") // POST requests must have Content-Length
		{
			std::cerr << HTTP_PARSE_MISSING_CONTENT_LENGTH << std::endl;
			error_code = 411;
			state = ERROR_PARSE;
			return pos;
		}
	}

	// Case 3: Read until connection closes (for HTTP/1.0)
	if (headers.find("connection") != headers.end() && headers["connection"] == "close")
	{
		body.assign(pos, end);
		state = DONE;
	}
	else
	{
		std::cerr << HTTP_PARSE_INVALID_CONTENT_LENGTH << std::endl;
		error_code = 400; // Bad Request
		state = ERROR_PARSE;
	}
	return pos;
}

// Method to handle transfer encoding: chunked
const char *RequestParser::parse_chunked_body(const char *pos, const char *end)
{
	while (pos < end)
	{
		const char *chunk_size_end = find_line_end(pos, end);

		if (chunk_size_end == end)
		{
			std::cerr << HTTP_PARSE_INVALID_CHUNKED_TRANSFER << std::endl;
			error_code = 400;
			state = ERROR_PARSE;
			return pos;
		}

		// Convert hex size to int
		std::string chunk_size_str(pos, chunk_size_end - 2);
		unsigned long chunk_size = std::strtoul(chunk_size_str.c_str(), NULL, 16);

		if (chunk_size == 0) // Last chunk
		{
			state = DONE;
			return chunk_size_end;
		}

		pos = chunk_size_end;

		if (pos + chunk_size > end)
		{
			std::cerr << HTTP_PARSE_INVALID_CHUNKED_TRANSFER << std::endl;
			error_code = 400;
			state = ERROR_PARSE;
			return pos;
		}

		body.append(pos, chunk_size);
		pos += chunk_size;

		// Ensure each chunk ends with CRLF
		if (pos + 2 > end || pos[0] != '\r' || pos[1] != '\n')
		{
			std::cerr << HTTP_PARSE_INVALID_CHUNKED_TRANSFER << std::endl;
			error_code = 400;
			state = ERROR_PARSE;
			return pos;
		}
		pos += 2;
	}
	return pos;
}

// Helper function returns the end of the line
const char *RequestParser::find_line_end(const char *pos, const char *end)
{
	while (pos < end - 1)
	{
		if (*pos == '\r' && *(pos + 1) == '\n')
			return pos + 2; // points past the CRLF
		pos++;
	}
	return end; // If no line is found
}

// Helper method to normalize the uri
std::string RequestParser::normalize_uri(const std::string &uri)
{
	std::string decoded = decode_percent_encoding(uri);
	std::vector<std::string> parts;
	std::istringstream stream(decoded);
	std::string segment;
	bool is_absolute = (!decoded.empty() && decoded[0] == '/');

	while (std::getline(stream, segment, '/'))
	{
		if (segment.empty() || segment == ".")
			continue;

		if (segment == "..")
		{
			if (!parts.empty())
				parts.pop_back(); // Move up one directory level
		}
		else
			parts.push_back(segment);
	}

	// Prevent directory traversal attack (`/../../etc`)
	if (!is_absolute && !parts.empty() && parts[0] == "..")
	{
		std::cerr << HTTP_PARSE_INVALID_URI << std::endl;
		error_code = 400;
		return "";
	}

	std::ostringstream normalized;
	if (is_absolute)
		normalized << "/";

	for (size_t i = 0; i < parts.size(); ++i)
	{
		normalized << parts[i];
		if (i < parts.size() - 1)
			normalized << "/";
	}

	return normalized.str().empty() ? "/" : normalized.str();
}

// Helper method to decode the percent encoding in uri
std::string RequestParser::decode_percent_encoding(const std::string &str)
{
	std::ostringstream decoded;
	size_t len = str.length();

	for (size_t i = 0; i < len; ++i)
	{
		if (str[i] == '%' && i + 2 < len)
		{
			char hex[3] = {str[i + 1], str[i + 2], '\0'};
			int value;

			if (std::isxdigit(hex[0]) && std::isxdigit(hex[1]) && sscanf(hex, "%2x", &value) == 1)
			{
				decoded << static_cast<char>(value);
				i += 2;
			}
			else
			{
				std::cerr << HTTP_PARSE_INVALID_PERCENT_ENCODING << std::endl;
				error_code = 400;
				return "";
			}
		}
		else
		{
			decoded << str[i];
		}
	}
	return decoded.str();
}

// Helper method to check if the connection is keep-alive
bool RequestParser::is_keep_alive()
{
	std::map<std::string, std::string>::iterator it = headers.find("connection");
	if (it != headers.end())
	{
		if (it->second == "keep-alive")
			return true;
	}
	return false;
}

// Helper method to check for the header name
bool RequestParser::is_valid_header_name(const std::string &name)
{
	if (name.empty())
		return false;

	for (size_t i = 0; i < name.size(); ++i)
	{
		char c = name[i];
		if (!(isalnum(c) || c == '-' || c == '_'))
			return false;
	}
	return true;
}

// Helper method to check for the header value
bool RequestParser::is_valid_header_value(const std::string &value)
{
	for (size_t i = 0; i < value.size(); ++i)
	{
		char c = value[i];
		if ((c < 32 || c == 127) && c != '\t' && c != ' ')
			return false;
	}
	return true;
}

/****************************
		START SETTERS
****************************/
bool RequestParser::set_http_method(const std::string &http_method)
{
	if (http_method == "GET" || http_method == "POST" || http_method == "DELETE")
	{
		// TODO
		// Later I should check if the method is allowed in the config file
		// For now I will allow all the methods
		this->http_method = http_method;
		return true;
	}
	if (http_method == "HEAD" || http_method == "OPTIONS" || http_method == "TRACE" || http_method == "PUT" || http_method == "PATCH" || http_method == "CONNECT")
	{
		std::cerr << HTTP_PARSE_METHOD_NOT_IMPLEMENTED << std::endl;
		error_code = 501;
	}
	else
	{
		std::cerr << HTTP_PARSE_INVALID_METHOD << std::endl;
		error_code = 400;
	}
	return false;
}
bool RequestParser::set_request_uri(const std::string &request_uri)
{
	if (request_uri.empty())
	{
		std::cerr << HTTP_PARSE_MISSING_REQUEST_URI << std::endl;
		error_code = 400;
		return false;
	}

	// Ensure no invalid chars in URI
	if (request_uri.find_first_of(" \t\r\n<>\"{}|\\^`") != std::string::npos)
	{
		std::cerr << HTTP_PARSE_INVALID_URI << std::endl;
		error_code = 400;
		return false;
	}

	// Check for ascii characters and delete
	for (size_t i = 0; i < request_uri.length(); ++i)
	{
		char c = request_uri[i];
		if (c <= 31 || c == 127)
		{
			std::cerr << HTTP_PARSE_INVALID_URI << std::endl;
			error_code = 400;
			return false;
		}
	}

	// Extract fragment (#) if present it should be ignored
	size_t fragment_start = request_uri.find('#');
	std::string uri = (fragment_start != std::string::npos) ? request_uri.substr(0, fragment_start) : request_uri;

	// Decode and Normalize URI
	uri = normalize_uri(uri);

	// Extract query string if present
	size_t query_start = uri.find('?');
	if (query_start != std::string::npos)
	{
		set_query_string(uri.substr(query_start + 1));
		uri = uri.substr(0, query_start);
	}

	// Ensure the final URI not empty, starts with a '/'
	if (uri.empty() || uri[0] != '/')
	{
		std::cerr << HTTP_PARSE_INVALID_URI << std::endl;
		error_code = 400;
		return false;
	}

	// Ensure URI is not too long
	if (uri.size() > MAX_URI_LENGTH)
	{
		std::cerr << HTTP_PARSE_URI_TOO_LONG << std::endl;
		error_code = 414;
		return false;
	}

	this->request_uri = uri;
	return true;
}
bool RequestParser::set_http_version(const std::string &http_version)
{
	if (http_version == "HTTP/1.1")
	{
		this->http_version = http_version;
		return true;
	}
	if (http_version == "HTTP/1.0")
	{
		std::cerr << HTTP_PARSE_HTTP_VERSION_NOT_SUPPORTED << std::endl;
		error_code = 505;
	}
	else
	{
		std::cerr << HTTP_PARSE_INVALID_VERSION " " << http_version << std::endl;
		error_code = 400;
	}
	return false;
}
void RequestParser::set_query_string(const std::string &query_string) { this->query_string = query_string; }
void RequestParser::set_body(const std::string &body) { this->body = body; }
void RequestParser::set_headers(const std::string &key, const std::string &value) { headers[key] = value; }
void RequestParser::set_error_code(short error_code) { this->error_code = error_code; }
/****************************
		END SETTERS
****************************/

/****************************
		START GETTERS
****************************/
std::string &RequestParser::get_http_method() { return http_method; }
std::string &RequestParser::get_request_uri() { return request_uri; }
std::string &RequestParser::get_query_string() { return query_string; }
std::string &RequestParser::get_http_version() { return http_version; }
std::map<std::string, std::string> &RequestParser::get_headers() { return headers; }
std::string &RequestParser::get_header_value(const std::string &key) { return headers[key]; }
std::string &RequestParser::get_body() { return body; }
short RequestParser::get_error_code() { return error_code; }
/****************************
		END GETTERS
****************************/

// Print the parsed request
void RequestParser::print_request()
{
	if (error_code == 1)
	{
		std::cout << BLUE "Method: " RESET << http_method << std::endl;
		std::cout << BLUE "PATH: " RESET << request_uri << std::endl;
		if (!query_string.empty())
			std::cout << BLUE "Query: " RESET << query_string << std::endl;
		std::cout << BLUE "Version: " RESET << http_version << std::endl;
		std::cout << BLUE "Headers:" RESET << std::endl;
		for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
			std::cout << MAGENTA "-- " << it->first << ": " RESET << it->second << std::endl;
		std::cout << BLUE "Body:\n" RESET << body << std::endl;
	}
	else
		std::cout << RED "Error Code: " << error_code << RESET << std::endl;
	std::cout << CYAN "** REQUEST PARSING DONE **" RESET << std::endl;
}
