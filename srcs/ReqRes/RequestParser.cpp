#include "RequestParser.hpp"

RequestParser::RequestParser()
{
	this->state = REQUEST_LINE;
	this->port = 80;
	this->error_code = 0;
	this->has_content_length = false;
	this->has_transfer_encoding = false;
	this->server_config = NULL;
	this->location_config = NULL;
	this->reading_chunk_data = false;
	this->current_chunk_size = 0;
	this->current_chunk_read = 0;
	this->is_cgi_request_flag = false;
	this->content_length_value = 0;
}

RequestParser::RequestParser(const RequestParser &other) : state(other.state),
														   request_line(other.request_line),
														   http_method(other.http_method),
														   request_uri(other.request_uri),
														   query_string(other.query_string),
														   http_version(other.http_version),
														   headers(other.headers),
														   body(other.body),
														   port(other.port),
														   error_code(other.error_code),
														   server_config(other.server_config),
														   location_config(other.location_config),
														   reading_chunk_data(other.reading_chunk_data),
														   current_chunk_size(other.current_chunk_size),
														   current_chunk_read(other.current_chunk_read),
														   cgi_script(other.cgi_script),
														   is_cgi_request_flag(other.is_cgi_request_flag),
														   content_length_value(other.content_length_value),
														   has_content_length(other.has_content_length),
														   has_transfer_encoding(other.has_transfer_encoding) {}

RequestParser &RequestParser::operator=(const RequestParser &other)
{
	if (this != &other)
	{
		this->state = other.state;
		this->request_line = other.request_line;
		this->http_method = other.http_method;
		this->request_uri = other.request_uri;
		this->query_string = other.query_string;
		this->http_version = other.http_version;
		this->headers = other.headers;
		this->body = other.body;
		this->port = other.port;
		this->error_code = other.error_code;
		this->has_content_length = other.has_content_length;
		this->has_transfer_encoding = other.has_transfer_encoding;
		this->server_config = other.server_config;
		this->location_config = other.location_config;
		this->reading_chunk_data = other.reading_chunk_data;
		this->current_chunk_size = other.current_chunk_size;
		this->current_chunk_read = other.current_chunk_read;
		this->cgi_script = other.cgi_script;
		this->is_cgi_request_flag = other.is_cgi_request_flag;
		this->content_length_value = other.content_length_value;
	}
	return *this;
}

// Main Function that parses the request
size_t RequestParser::parse_request(const std::string &request)
{
	const char *start = request.c_str();
	const char *end = start + request.size();
	const char *pos = start;

	if (state == BODY)
	{
		pos = parse_body(pos, end);
		return pos - start;
	}

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
	const char *line_end = find_line_end(pos, end);
	if (line_end == end)
	{
		return pos; // Wait for more data
	}

	if ((line_end - pos) > MAX_REQUEST_LINE_LENGTH)
	{
		log_error(HTTP_PARSE_URI_TOO_LONG, 414);
		return pos;
	}

	std::vector<std::string> parts = Utils::split(pos, line_end - 2, ' '); /* REQUEST LINE ==> [METHOD] SP [URI] SP [VERSION] */
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

// Method to extract headers
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
				state = DONE; // Ignore the Body
				return header_end;
			}
			state = BODY;
			return header_end;
		}

		if (*pos == ' ' || *pos == '\t') // Reject obsolete line folding
		{
			log_error(HTTP_PARSE_INVALID_HEADER_FIELD, 400);
			return pos;
		}

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

		std::string key = header_line.substr(0, colon_pos);
		std::string value = Utils::trim(header_line.substr(colon_pos + 1), " \t");

		std::transform(key.begin(), key.end(), key.begin(), ::tolower);

		if (!is_valid_header_name(key) || !is_valid_header_value(value))
		{
			log_error(HTTP_PARSE_INVALID_HEADER_FIELD, 400);
			return pos;
		}

		// Special Handling for `Content-Length`
		if (key == "content-length")
		{
			if (!Utils::is_numeric(value) || (has_content_length && content_length_value != value))
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
			if (value == "chunked" || value == "compress" || value == "deflate" || value == "gzip")
				has_transfer_encoding = true;
			else
			{
				log_error(HTTP_PARSE_INVALID_TRANSFER_ENCODING, 400);
				return pos;
			}
		}

		// Special Handling for `Host`
		if (key == "host")
		{
			if (has_host)
			{
				log_error(HTTP_PARSE_INVALID_HOST, 400);
				return pos;
			}

			has_host = true;
			// std::string host_value = value;
			// size_t colon_pos = host_value.find(':');
			// if (colon_pos != std::string::npos)
			// {
			// 	std::string port_str = host_value.substr(colon_pos + 1);
			// 	host_value = host_value.substr(0, colon_pos);
			// 	char *endptr;
			// 	long parsed_port = std::strtol(port_str.c_str(), &endptr, 10);
			// 	if (*endptr != '\0' || parsed_port < 1 || parsed_port > 65535)
			// 	{
			// 		log_error(HTTP_PARSE_INVALID_PORT, 400);
			// 		return pos;
			// 	}
			// 	this->port = static_cast<uint16_t>(parsed_port);
		}

		// Special Handling for `Expect`
		if (key == "expect")
		{
			std::transform(value.begin(), value.end(), value.begin(), ::tolower);
			if (value != "100-continue")
			{
				log_error(HTTP_PARSE_INVALID_EXPECT_VALUE, 417);
				return pos;
			}
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

// Method to extract body
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
		this->content_length_value = content_length;
		size_t bytes_in_body = static_cast<size_t>(end - pos);

		// Check for invalid content length value
		if (*endptr != '\0' || content_length == 0)
		{
			log_error(HTTP_PARSE_INVALID_CONTENT_LENGTH, 400);
			return pos;
		}

		size_t remaining_bytes = content_length - body.size();
		size_t bytes_to_read = std::min(remaining_bytes, bytes_in_body);

		// Partial body reading
		if (bytes_to_read > 0)
		{
			body.insert(body.end(), pos, pos + bytes_to_read);
			pos += bytes_to_read;
		}

		// If body is fully received -> Parse is done
		if (body.size() == content_length)
		{
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
		// If we're currently reading chunk data
		if (reading_chunk_data)
		{
			size_t remaining = current_chunk_size - current_chunk_read;
			size_t available = static_cast<size_t>(end - pos);
			size_t to_read = std::min(remaining, available);

			body.insert(body.end(), pos, pos + to_read);
			pos += to_read;
			current_chunk_read += to_read;

			// Finish to read this chunk
			if (current_chunk_read == current_chunk_size)
			{
				if (end - pos < 2)
					return pos; // Wait for more data

				if (pos[0] != '\r' || pos[1] != '\n')
				{
					log_error(HTTP_PARSE_INVALID_CHUNKED_TRANSFER, 400);
					return pos;
				}
				pos += 2;

				current_chunk_read = 0;
				current_chunk_size = 0;
				reading_chunk_data = false;
			}
			else
			{
				return pos; // More data needed for current chunk
			}
		}
		else
		{
			const char *line_end = find_line_end(pos, end);
			if (line_end == end)
				return pos; // Incomplete size line

			std::string size_str = Utils::trim(std::string(pos, line_end - pos), "\r\n \t");
			char *endptr = NULL;
			unsigned long parsed = std::strtoul(size_str.c_str(), &endptr, 16);
			if (*endptr != '\0')
			{
				log_error(HTTP_PARSE_INVALID_CHUNKED_TRANSFER, 400);
				return pos;
			}

			current_chunk_size = static_cast<size_t>(parsed);
			pos = line_end;

			if (current_chunk_size == 0)
			{
				// Final chunk ==> expect CRLF
				if (end - pos < 2)
					return pos;

				if (pos[0] != '\r' || pos[1] != '\n')
				{
					log_error(HTTP_PARSE_INVALID_CHUNKED_TRANSFER, 400);
					return pos;
				}
				pos += 2;

				// Done reading chunked body
				state = DONE;
				return pos;
			}

			// Prepare to read the next chunk data
			current_chunk_read = 0;
			reading_chunk_data = true;
		}
	}
	return pos;
}

// Helper method returns the end of the line (after \r\n)
const char *RequestParser::find_line_end(const char *pos, const char *end)
{
	while (pos < end - 1)
	{
		if (*pos == '\r' && *(pos + 1) == '\n')
			return pos + 2;
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
			// directory traversal (`/../../a/b`)
			if (!is_absolute || parts.empty())
			{
				log_error(HTTP_PARSE_INVALID_URI, 400);
				return "";
			}
			parts.pop_back();
		}
		else
		{
			parts.push_back(segment);
		}
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
			char hex1 = str[i + 1];
			char hex2 = str[i + 2];

			if (std::isxdigit(hex1) && std::isxdigit(hex2))
			{
				int value = (std::isdigit(hex1) ? hex1 - '0' : std::toupper(hex1) - 'A' + 10) * 16 + (std::isdigit(hex2) ? hex2 - '0' : std::toupper(hex2) - 'A' + 10);

				if (value == '\0')
				{
					log_error(HTTP_PARSE_NULL_BYTE, 400);
					return "";
				}

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
	std::ostringstream err;
	if (!this->error_code)
	{
		err << error_str << " (code: " << error_code << ")";
		LOG_ERROR(err.str());
		this->error_code = error_code;
		if (this->error_code)
			this->headers["connection"] = "close";
		state = ERROR_PARSE;
	}
	else
	{
		err << error_str << " (code: " << error_code << ") - not overriding existing error " << this->error_code;
		LOG_ERROR(err.str());
	}
}

std::string RequestParser::normalize_path(const std::string &path)
{
	std::string result;
	bool last_was_slash = false;

	for (size_t i = 0; i < path.length(); ++i)
	{
		if (path[i] == '/')
		{
			if (!last_was_slash)
			{
				result += '/';
				last_was_slash = true;
			}
		}
		else
		{
			result += path[i];
			last_was_slash = false;
		}
	}
	return result;
}

// Method for setting the right location for the request
void RequestParser::match_location(const std::vector<ServerConfig *> &servers)
{
	this->server_config = servers[0];

	std::string host;
	std::map<std::string, std::string>::const_iterator host_it = headers.find("host");
	if (host_it != headers.end())
	{
		host = host_it->second;
	}

	size_t colon_pos = host.find(':');
	if (colon_pos != std::string::npos)
	{
		host = host.substr(0, colon_pos);
	}

	std::vector<ServerConfig *>::const_iterator it = servers.begin();
	for (; it != servers.end(); ++it)
	{
		if (std::find((*it)->serverNames.begin(), (*it)->serverNames.end(), host) != (*it)->serverNames.end())
		{
			this->server_config = *it;
		}
	}

	size_t best_match_length = 0;
	const Location *exact_match = NULL;
	const Location *best_prefix_match = NULL;

	for (size_t i = 0; i < server_config->locations.size(); ++i)
	{
		const std::string &location_path = server_config->locations[i].location;
		std::string normalize_location = normalize_path(location_path);

		if (request_uri == normalize_location)
		{
			exact_match = &server_config->locations[i];
			break;
		}
		else if (request_uri.find(normalize_location) == 0 && normalize_location.length() > best_match_length)
		{
			best_match_length = normalize_location.length();
			best_prefix_match = &server_config->locations[i];
		}
	}

	if (exact_match)
		location_config = exact_match;
	else if (best_prefix_match)
		location_config = best_prefix_match;

	if (!location_config)
		log_error(HTTP_PARSE_NO_LOCATION_BLOCK, 404);
}

// Method to check if the request is for cgi
bool RequestParser::is_cgi_request()
{
	if (is_cgi_request_flag)
		return true;

	std::string path = request_uri;

	if (!path.empty() && path[path.size() - 1] == '/' && location_config)
	{
		if (!location_config->index.empty())
			path += location_config->index;
	}

	size_t dot_pos = path.find_last_of('.');
	if (dot_pos == std::string::npos || path.find_last_of('/') > dot_pos)
		return false;

	std::string extension = path.substr(dot_pos);
	if ((extension == ".php" || extension == ".py"))
		return true;

	return false;
}

/****************************
		START SETTERS
****************************/
void RequestParser::set_request_line()
{
	std::ostringstream ss;
	ss << http_method << SP << request_uri << SP << http_version;
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
void RequestParser::set_query_string(const std::string &query_string)
{
	this->query_string = query_string;
}
void RequestParser::set_port(uint16_t port)
{
	this->port = port;
}
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
short &RequestParser::get_error_code() { return error_code; }
uint16_t &RequestParser::get_port_number() { return port; }
ParseState &RequestParser::get_state() { return state; }
const ServerConfig *RequestParser::get_server_config() { return server_config; }
const Location *RequestParser::get_location_config() { return location_config; }
size_t RequestParser::get_content_length_value() { return this->content_length_value; }
/****************************
		END GETTERS
****************************/

void RequestParser::print_request()
{
	if (!error_code)
	{
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
