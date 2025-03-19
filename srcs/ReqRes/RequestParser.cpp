#include "RequestParser.hpp"
#include "Logger.hpp"

// LIMITS FOR SOME REQUEST ELEMENTS
#define MAX_REQUEST_LINE_LENGTH 8192
#define MAX_URI_LENGTH 2048
#define MAX_HEADER_LENGTH 8192
#define MAX_HEADER_COUNT 100

// Constructor
RequestParser::RequestParser(const std::string &request, const std::vector<ServerConfig> &servers)
{
	this->state = REQUEST_LINE;
	this->error_code = 1;
	this->has_content_length = false;
	this->has_transfer_encoding = false;
	this->bytes_read = 0;
	this->body_size = 0;
	this->server_config = NULL;
	this->location_config = NULL;
	this->is_headers_completed = false;
	this->is_body_completed = false;
	this->bytes_read += parse_request(request);
	set_request_line();
	if (this->bytes_read > 0)
	{
		match_location(servers); // Match the request to the correct server and location block
		body_size = this->body.size();
		if (body_size > server_config->clientMaxBodySize)
			log_error(HTTP_PARSE_PAYLOAD_TOO_LARGE, 413);
	}
}

// Copy Constructor
RequestParser::RequestParser(const RequestParser &other) : state(other.state),
														   request_line(other.request_line),
														   http_method(other.http_method),
														   request_uri(other.request_uri),
														   query_string(other.query_string),
														   http_version(other.http_version),
														   headers(other.headers),
														   body(other.body),
														   bytes_read(other.bytes_read),
														   error_code(other.error_code),
														   has_content_length(other.has_content_length),
														   has_transfer_encoding(other.has_transfer_encoding),
														   server_config(other.server_config),
														   location_config(other.location_config)
{
	// Note: No need to deep-copy server_config and location_config
	// as they are pointers to configurations owned by ConfigManager
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
			if (state == ERROR_PARSE || pos == end)
				return pos - start;
			break;
		case HEADERS:
			pos = parse_headers(pos, end);
			if (state == ERROR_PARSE || pos == end)
				return pos - start;
			break;
		case BODY:
			pos = parse_body(pos, end);
			if (state == ERROR_PARSE || pos == end)
				return pos - start;
			break;
		default:
			state = ERROR_PARSE;
		}
	}
	return (pos - start);
}

// Method to extract the request line
const char *RequestParser::parse_request_line(const char *pos, const char *end)
{
	const char *line_end = find_line_end(pos, end); // Points to the end of the line (after CRLF)
	if (line_end == end)
	{
		return pos; // Wait for more data
	}

	if ((line_end - pos) > MAX_REQUEST_LINE_LENGTH) // Check for long Request line
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
			if (has_content_length && has_transfer_encoding)
			{
				log_error(HTTP_PARSE_CONFLICTING_HEADERS, 400);
				return pos;
			}
			if (!has_content_length && !has_transfer_encoding)
			{
				if (http_method == "POST")
				{

					log_error(HTTP_PARSE_MISSING_CONTENT_LENGTH, 411);
					return pos;
				}
				state = DONE; // Ignore the Body by marking the request as DONE
				this->is_headers_completed = true;
				this->is_body_completed = true;
				return header_end;
			}
			state = BODY;
			this->is_headers_completed = true;
			return header_end;
		}

		if (*pos == ' ' || *pos == '\t') // Reject obsolete line folding
		{
			log_error(HTTP_PARSE_INVALID_HEADER_FIELD, 400);
			return pos;
		}

		if ((header_end - pos) > MAX_HEADER_LENGTH) // Header field too large
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

		if (key == "transfer-encoding")
			has_transfer_encoding = true;

		// Special Handling for `Host`
		if (key == "host")
		{
			if (has_host)
			{
				log_error(HTTP_PARSE_INVALID_HOST, 400);
				return pos;
			}

			has_host = true;
			std::string host_value = value;
			uint16_t port = 80;

			size_t colon_pos = host_value.find(':');
			if (colon_pos != std::string::npos)
			{
				std::string port_str = host_value.substr(colon_pos + 1);
				host_value = host_value.substr(0, colon_pos);

				char *endptr;
				long parsed_port = std::strtol(port_str.c_str(), &endptr, 10);

				if (*endptr != '\0' || parsed_port < 1 || parsed_port > 65535)
				{
					log_error(HTTP_PARSE_INVALID_PORT, 400);
					return pos;
				}

				port = static_cast<uint16_t>(parsed_port);
			}
			this->port = port;
		}

		if (headers.size() >= MAX_HEADER_COUNT)
		{
			log_error(HTTP_PARSE_HEADER_FIELDS_TOO_LARGE, 431);
			return pos;
		}

		this->headers[key] = value;
		pos = header_end;
	}
	return pos;
}

// Method to extract body of the request
const char *RequestParser::parse_body(const char *pos, const char *end)
{
	// Case 1: Transfer-Encoding: chunked
	if (has_transfer_encoding)
	{
		if (has_transfer_encoding && headers["transfer-encoding"] == "chunked")
			return parse_chunked_body(pos, end);
		else
			log_error(HTTP_PARSE_METHOD_NOT_IMPLEMENTED, 501);
		return pos;
	}

	// Case 2: Content-Length is present
	if (has_content_length)
	{
		char *endptr = NULL;
		std::string content_length_str = headers["content-length"];
		size_t content_length = std::strtoul(content_length_str.c_str(), &endptr, 10);
		size_t bytes_in_body = static_cast<size_t>(end - pos);

		// Check for invalid content length value
		if (*endptr != '\0' || content_length > bytes_in_body)
		{
			log_error(HTTP_PARSE_INVALID_CONTENT_LENGTH, 400);
			return pos;
		}

		size_t remaining_bytes = content_length - body.size();
		size_t bytes_to_read = std::min(remaining_bytes, bytes_in_body);
		body.insert(body.end(), pos, pos + bytes_to_read);
		pos += bytes_to_read;

		// If body is fully received -> Parse is done
		if (body.size() == content_length)
		{
			this->is_body_completed = true;
			state = DONE;
		}

		return pos;
	}
	return pos;
}

// Method to extract chunked body
const char *RequestParser::parse_chunked_body(const char *pos, const char *end)
{
	while (pos < end)
	{
		const char *chunk_size_end = find_line_end(pos, end);
		if (chunk_size_end == end)
		{
			return pos; // wait for more data
		}

		// Convert hex size to int
		std::string chunk_size_str = trim(std::string(pos, chunk_size_end - pos), "\r\n \t");
		char *endptr = NULL;
		size_t chunk_size = std::strtoul(chunk_size_str.c_str(), &endptr, 16);
		if (*endptr != '\0')
		{
			log_error(HTTP_PARSE_INVALID_CHUNKED_TRANSFER, 400);
			return pos;
		}

		pos = chunk_size_end;

		if (chunk_size == 0) // Last chunk
		{
			if (pos + 2 > end)
				return pos; // Wait for final CRLF

			if (pos[0] != '\r' || pos[1] != '\n')
			{
				log_error(HTTP_PARSE_INVALID_CHUNKED_TRANSFER, 400);
				return pos;
			}

			pos += 2;
			state = DONE;
			this->is_body_completed = true;
			return pos;
		}

		// Ensure Enough bytes in this segment
		size_t remaining_bytes = static_cast<size_t>(end - pos);
		size_t bytes_to_read = std::min(chunk_size, remaining_bytes);
		body.insert(body.end(), pos, pos + bytes_to_read);
		pos += bytes_to_read;

		if (body.size() < chunk_size) // If not enough bytes received, wait for the next segment
			return pos;

		// Ensure chunk ends with CRLF
		if (pos + 2 > end || pos[0] != '\r' || pos[1] != '\n')
		{
			log_error(HTTP_PARSE_INVALID_CHUNKED_TRANSFER, 400);
			return pos;
		}
		pos += 2;
	}
	return pos;
}

// Helper method returns the end of the line (after \r\n)
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
	if (decoded.empty())
		return "";

	std::vector<std::string> parts;
	std::istringstream stream(decoded);
	std::string segment;
	bool is_absolute = (decoded[0] == '/');
	bool has_trailing_slash = (decoded[decoded.size() - 1] == '/');

	while (std::getline(stream, segment, '/'))
	{
		if (segment.empty() || segment == ".")
			continue;

		if (segment == "..")
		{
			// Prevent directory traversal (`/../../etc/passwd`)
			if (!is_absolute || parts.empty())
			{
				log_error(HTTP_PARSE_INVALID_URI, 400);
				return "";
			}
			parts.pop_back();
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
		if (i < parts.size() - 1 || has_trailing_slash)
			normalized << "/";
	}

	return normalized.str().empty() ? "/" : normalized.str();
}

// Helper method to handle percent encoding in uri
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
				log_error(HTTP_PARSE_INVALID_PERCENT_ENCODING, 400);
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
bool RequestParser::is_connection_keep_alive()
{
	std::map<std::string, std::string>::iterator it = headers.find("connection");
	if (it != headers.end())
	{
		if (it->second == "keep-alive")
			return true;
	}
	return false;
}

// Helper method to check if the connection is close
bool RequestParser::is_connection_close()
{
	std::map<std::string, std::string>::iterator it = headers.find("connection");
	if (it != headers.end())
	{
		if (it->second == "close")
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

// Method to check if `Content-Length` exists in headers
bool RequestParser::content_length_exists()
{
	return (has_content_length ? true : false);
}

// Method to check if `Transfer-Encoding` exists in headers
bool RequestParser::transfer_encoding_exists()
{
	return (has_transfer_encoding ? true : false);
}

// Method for setting the right location for the request
void RequestParser::match_location(const std::vector<ServerConfig> &servers)
{
	std::string host = headers["host"];

	this->server_config = ConfigManager::getInstance()->getServerByName(host);
	if (!this->server_config)
	{
		this->server_config = ConfigManager::getInstance()->getServerByPort(port);
	}

	// If no exact match -> point to the first configured server
	if (!server_config && !servers.empty())
		this->server_config = &servers[0];

	// Find the best matching Location
	size_t best_match_length = 0;
	for (size_t i = 0; i < server_config->locations.size(); ++i)
	{
		const std::string &location_path = server_config->locations[i].location;

		if (request_uri.find(location_path) == 0 && location_path.length() > best_match_length)
		{
			best_match_length = location_path.length();
			this->location_config = &server_config->locations[i];
		}
	}

	// this check will be in config parse (Remove later)
	if (!location_config)
	{
		log_error(HTTP_PARSE_INVALID_LOCATION, 404);
		return;
	}
}

bool RequestParser::headers_completed() { return is_headers_completed; }
bool RequestParser::body_completed() { return is_body_completed; }

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
	if (uri.empty())
		return false;

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
size_t &RequestParser::get_body_size() { return body_size; }
short &RequestParser::get_error_code() { return error_code; }
uint16_t &RequestParser::get_port_number() { return port; }
ParseState &RequestParser::get_state() { return state; }
const ServerConfig *RequestParser::get_server_config() { return server_config; }
const Location *RequestParser::get_location_config() { return location_config; }
/****************************
		END GETTERS
****************************/

// Print the parsed request
void RequestParser::print_request()
{
	if (error_code == 1)
	{
		LOG_REQUEST(request_line);
		std::cout << "PORT NUMBER = " << port << std::endl;
		std::cout << BLUE "Method: " RESET << http_method << std::endl;
		std::cout << BLUE "PATH: " RESET << request_uri << std::endl;
		if (!query_string.empty())
			std::cout << BLUE "Query: " RESET << query_string << std::endl;
		std::cout << BLUE "Version: " RESET << http_version << std::endl;
		std::cout << BLUE "Headers:" RESET << std::endl;
		for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
			std::cout << MAGENTA "-- " << it->first << ": " RESET << it->second << std::endl;
		std::cout << BLUE "Body: " RESET << std::endl;
		for (std::vector<byte>::iterator it = body.begin(); it != body.end(); ++it)
		{
			std::cout << *it;
		}
		std::cout << std::endl;
	}
}
