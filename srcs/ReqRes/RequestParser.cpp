#include "RequestParser.hpp"
#include "Logger.hpp"

// Constructor
RequestParser::RequestParser()
{
	this->state = REQUEST_LINE; // Starting by Request Line State (Start line of the request)
	this->error_code = 1;		// If no error, error_code is set to 1 else it will be set to the error code that i will return in the response directly
	this->has_content_length = false;
	this->has_transfer_encoding = false;
}

// Main Function that parses the request
size_t RequestParser::parse_request(const std::string &request)
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
			if (state == ERROR_PARSE)
				return pos - start;
			break;
		case HEADERS:
			pos = parse_headers(pos, end);
			if (state == ERROR_PARSE)
			{
				return pos - start;
			}
			break;
		case BODY:
			if (!has_content_length && !has_transfer_encoding)
			{
				state = DONE; // No body expected -> Mark request as complete
				return pos - start;
			}
			parse_body(pos, end);
			break;
		default:
			state = ERROR_PARSE;
		}
	}
	set_request_line();
	return (pos - start);
}

// Method to extract the request line
const char *RequestParser::parse_request_line(const char *pos, const char *end)
{
	const char *line_end = find_line_end(pos, end); // Points to the end of the line (after CRLF)
	if (line_end == end)
		return pos; // Wait for more data

	if ((line_end - pos) > MAX_REQUEST_LINE_LENGTH) // Check for long Request line > 8k bytes
	{
		log_error(HTTP_PARSE_URI_TOO_LONG, 414);
		return pos;
	}

	// Split the request line by exactly one SP and check if its 3 parts ([METHOD] SP [URI] SP [VERSION])
	std::vector<std::string> parts = split(pos, line_end - 2, ' ');
	if (parts.size() != 3)
	{
		log_error(HTTP_PARSE_INVALID_REQUEST_LINE, 400);
		return pos;
	}

	// Check for all parts of request line before setting them
	if (!set_http_method(parts[0]) || !set_request_uri(parts[1]) || !set_http_version(parts[2]))
		return pos;

	state = HEADERS;
	return line_end;
}

// Method to extract headers from the request
const char *RequestParser::parse_headers(const char *pos, const char *end)
{
	bool has_host = false;
	std::string content_length_value;

	while (pos < end)
	{
		const char *header_end = find_line_end(pos, end);
		if (pos == header_end - 2) // If an empty line that means end of headers
		{
			if (!has_host)
			{
				log_error(HTTP_PARSE_MISSING_HOST, 400);
				return pos;
			}
			if (!has_content_length && !has_transfer_encoding)
			{
				if (http_method == "POST")
				{

					log_error(HTTP_PARSE_MISSING_CONTENT_LENGTH, 411);
					return pos;
				}
				state = DONE;
				return header_end;
			}
			state = BODY;
			return header_end;
		}

		// Reject obsolete line folding
		if (*pos == ' ' || *pos == '\t')
		{
			log_error(HTTP_PARSE_INVALID_HEADER_FIELD, 400);
			return pos;
		}

		// Header field too large
		if ((header_end - pos) > MAX_HEADER_LENGTH)
		{
			log_error(HTTP_PARSE_HEADER_FIELDS_TOO_LARGE, 431);
			return pos;
		}

		std::string header_line(pos, header_end - 2);
		size_t colon_pos = header_line.find(':');

		// No colon found || No header key || Space before colon
		if (colon_pos == std::string::npos || colon_pos == 0 || header_line[colon_pos - 1] == ' ')
		{
			log_error(HTTP_PARSE_INVALID_HEADER_FIELD, 400);
			return pos;
		}

		// Set the header key and header value
		std::string key = header_line.substr(0, colon_pos);
		std::string value = trim(header_line.substr(colon_pos + 1), " \t");

		// Convert header names to lowercase
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);

		// Validate header name and value
		if (!is_valid_header_name(key) || !is_valid_header_value(value))
		{
			log_error(HTTP_PARSE_INVALID_HEADER_FIELD, 400);
			return pos;
		}

		// Special Handling for `Content-Length`
		if (key == "content-length")
		{
			if (!is_numeric(value) || (has_content_length && content_length_value != value))
			{
				log_error(HTTP_PARSE_INVALID_CONTENT_LENGTH, 400);
				return pos;
			}
			has_content_length = true;
			content_length_value = value;
		}

		// Special Handling for `Transfer-Encoding`
		if (key == "transfer-encoding")
		{
			has_transfer_encoding = true;
			if (has_content_length)
			{
				log_error(HTTP_PARSE_CONFLICTING_HEADERS, 400);
				return pos;
			}
		}

		// Special Handling for `Host`
		if (key == "host")
		{
			if (has_host) // Later check if this host is in the config file and the right location
			{
				log_error(HTTP_PARSE_INVALID_HOST, 400);
				return pos;
			}
			has_host = true;
		}

		if (headers.size() >= MAX_HEADER_COUNT)
		{
			log_error(HTTP_PARSE_HEADER_FIELDS_TOO_LARGE, 431);
			return pos;
		}

		headers[key] = value;
		pos = header_end;
	}
	std::cout << "hello" << std::endl;
	return pos;
}

// Method to extract body of the request
size_t RequestParser::parse_body(const char *pos, const char *end)
{
	size_t bytes_received = 0;

	if (!has_content_length && !has_transfer_encoding)
	{
		state = DONE; // No body expected -> Mark as complete
		return 0;
	}

	// Case 1: Transfer-Encoding: chunked
	if (has_transfer_encoding)
	{
		if (has_transfer_encoding && headers["transfer-encoding"] == "chunked")
			return parse_chunked_body(pos, end);
		else
			log_error(HTTP_PARSE_METHOD_NOT_IMPLEMENTED, 501);
		return bytes_received;
	}

	// Case 2: Content-Length is present
	if (has_content_length)
	{
		char *endptr = NULL;
		std::string content_length_str = headers["content-length"];
		unsigned long content_length = std::strtoul(content_length_str.c_str(), &endptr, 10);

		// Check for invalid content length
		if (*endptr != '\0' || content_length > static_cast<size_t>(end - pos))
		{
			log_error(HTTP_PARSE_INVALID_CONTENT_LENGTH, 400);
			return bytes_received;
		}

		size_t remaining_bytes = content_length - body.size();
		size_t bytes_to_read = std::min(remaining_bytes, static_cast<size_t>(end - pos));

		body.insert(body.end(), pos, pos + bytes_to_read);
		pos += bytes_to_read;
		bytes_received += bytes_to_read;

		// If body is fully received ==> Parse done
		if (body.size() == content_length)
			state = DONE;

		return bytes_received;
	}

	return bytes_received;
}

// Method to handle transfer encoding: chunked
size_t RequestParser::parse_chunked_body(const char *pos, const char *end)
{
	size_t bytes_received = 0;
	while (pos < end)
	{
		const char *chunk_size_end = find_line_end(pos, end);
		if (chunk_size_end == end)
			return bytes_received;

		// Convert hex size to int
		std::string chunk_size_str = trim(std::string(pos, chunk_size_end - pos), "\r\n \t");
		char *endptr = NULL;
		size_t chunk_size = std::strtoul(chunk_size_str.c_str(), &endptr, 16);
		if (*endptr != '\0')
		{
			log_error(HTTP_PARSE_INVALID_CHUNKED_TRANSFER, 400);
			return bytes_received;
		}

		pos = chunk_size_end;

		if (chunk_size == 0) // Last chunk
		{
			if (pos + 2 > end)
				return bytes_received; // Wait for final CRLF
			if (pos[0] != '\r' || pos[1] != '\n')
			{
				log_error(HTTP_PARSE_INVALID_CHUNKED_TRANSFER, 400);
				return bytes_received;
			}
			pos += 2;
			state = DONE;
			return bytes_received;
		}

		// Ensure Enough bytes in this segment
		size_t bytes_to_read = std::min(chunk_size, static_cast<size_t>(end - pos));
		body.insert(body.end(), pos, pos + bytes_to_read);
		pos += bytes_to_read;
		bytes_received += bytes_to_read;

		// If not enough bytes received, wait for the next segment
		if (body.size() < chunk_size)
			return bytes_received; // Wait for next data segment

		// Ensure chunk ends with CRLF
		if (pos + 2 > end || pos[0] != '\r' || pos[1] != '\n')
		{
			log_error(HTTP_PARSE_INVALID_CHUNKED_TRANSFER, 400);
			return bytes_received;
		}
		pos += 2; // Move past CRLF
	}
	return bytes_received;
}

// Helper function returns the end of the line
const char *RequestParser::find_line_end(const char *pos, const char *end)
{
	while (pos < end - 1)
	{
		if (*pos == '\r' && *(pos + 1) == '\n')
			return pos + 2; // Found full CRLF
		pos++;
	}
	return (pos < end) ? pos : end;
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
			// Prevent directory traversal (`/../../etc/passwd`)
			if (!is_absolute || parts.empty())
				return "";
			parts.pop_back(); // Move up one directory level
		}
		else
			parts.push_back(segment);
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

// Method to check if the header name is valid
bool RequestParser::is_valid_header_name(const std::string &name)
{
	for (size_t i = 0; i < name.size(); ++i)
	{
		if (!(isalnum(name[i]) || name[i] == '-' || name[i] == '_'))
			return false;
	}
	return true;
}

// Method to check if the header value is valid
bool RequestParser::is_valid_header_value(const std::string &value)
{
	for (size_t i = 0; i < value.size(); ++i)
	{
		if ((value[i] < 32 || value[i] == 127) && value[i] != '\t' && value[i] != ' ')
			return false;
	}
	return true;
}

// Method to Log the error into console and logg file
void RequestParser::log_error(const std::string &error_str, short error_code)
{
	std::ostringstream ss;
	ss << error_str << " (code: " << error_code << ")";
	LOG_ERROR(ss.str());
	this->error_code = error_code;
	state = ERROR_PARSE;
}

// Method to check in `Content-Length` exists in headers
bool RequestParser::content_length_exists()
{
	return (has_content_length ? true : false);
}

// Method to check in `Transfer-Encoding` exists in headers
bool RequestParser::transfer_encoding_exists()
{
	return (has_transfer_encoding ? true : false);
}

/****************************
		START SETTERS
****************************/
void RequestParser::set_request_line()
{
	std::ostringstream ss;
	ss << http_method << " " << request_uri << " " << http_version;
	this->request_line = ss.str();
}
bool RequestParser::set_http_method(const std::string &http_method)
{
	if (http_method == "GET" || http_method == "POST" || http_method == "DELETE")
	{
		this->http_method = http_method;
		return true;
	}
	if (http_method == "HEAD" || http_method == "OPTIONS" || http_method == "TRACE" || http_method == "PUT" || http_method == "PATCH" || http_method == "CONNECT")
		log_error(HTTP_PARSE_METHOD_NOT_IMPLEMENTED, 501);
	else
		log_error(HTTP_PARSE_INVALID_METHOD, 400);
	return false;
}
bool RequestParser::set_request_uri(const std::string &request_uri)
{
	if (request_uri.empty())
	{
		log_error(HTTP_PARSE_MISSING_REQUEST_URI, 400);
		return false;
	}

	// Ensure no invalid chars in URI
	if (request_uri.find_first_of(" \t\r\n<>\"{}|\\^`") != std::string::npos)
	{
		log_error(HTTP_PARSE_INVALID_URI, 400);
		return false;
	}

	// Check for ascii characters and delete
	for (size_t i = 0; i < request_uri.length(); ++i)
	{
		if (request_uri[i] <= 31 || request_uri[i] == 127)
		{
			log_error(HTTP_PARSE_INVALID_URI, 400);
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
		log_error(HTTP_PARSE_INVALID_URI, 400);
		return false;
	}

	// Ensure URI is not too long
	if (uri.size() > MAX_URI_LENGTH)
	{
		log_error(HTTP_PARSE_URI_TOO_LONG, 414);
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
	if (http_version == "HTTP/0.9" || http_version == "HTTP/1.0")
		log_error(HTTP_PARSE_HTTP_VERSION_NOT_SUPPORTED, 505);
	else
		log_error(HTTP_PARSE_INVALID_VERSION, 400);
	return false;
}
void RequestParser::set_query_string(const std::string &query_string) { this->query_string = query_string; }
void RequestParser::set_body(std::vector<byte> &body) { this->body = body; }
void RequestParser::set_headers(const std::string &key, const std::string &value) { headers[key] = value; }
void RequestParser::set_error_code(short error_code) { this->error_code = error_code; }
/****************************
		END SETTERS
****************************/

/****************************
		START GETTERS
****************************/
std::string &RequestParser::get_request_line() { return request_line; }
std::string &RequestParser::get_http_method() { return http_method; }
std::string &RequestParser::get_request_uri() { return request_uri; }
std::string &RequestParser::get_query_string() { return query_string; }
std::string &RequestParser::get_http_version() { return http_version; }
std::map<std::string, std::string> &RequestParser::get_headers() { return headers; }
std::string &RequestParser::get_header_value(const std::string &key) { return headers[key]; }
std::vector<byte> &RequestParser::get_body() { return body; }
short RequestParser::get_error_code() { return error_code; }
ParseState &RequestParser::get_state() { return state; }
/****************************
		END GETTERS
****************************/

// Print the parsed request
void RequestParser::print_request()
{
	if (error_code == 1)
	{
		LOG_REQUEST(request_line);
		std::cout << BLUE "Method: " RESET << http_method << std::endl;
		std::cout << BLUE "PATH: " RESET << request_uri << std::endl;
		if (!query_string.empty())
			std::cout << BLUE "Query: " RESET << query_string << std::endl;
		std::cout << BLUE "Version: " RESET << http_version << std::endl;
		std::cout << BLUE "Headers:" RESET << std::endl;
		for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
			std::cout << MAGENTA "-- " << it->first << ": " RESET << it->second << std::endl;
		std::cout << BLUE "Body:" RESET << std::endl;
		for (std::vector<byte>::iterator it = body.begin(); it != body.end(); ++it)
		{
			std::cout << *it;
		}
		std::cout << std::endl;
	}
	std::cout << CYAN "** REQUEST PARSING DONE **" RESET << std::endl;
}
