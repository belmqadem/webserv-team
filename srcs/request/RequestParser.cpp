#include "../../includes/RequestParser.hpp"

// Constructor
RequestParser::RequestParser(const std::string &request)
{
	this->state = REQUEST_LINE; // Starting by Request Line
	this->http_method = "";
	this->request_uri = "";
	this->query_string = "";
	this->http_version = "";
	this->headers = std::map<std::string, std::string>();
	this->body = "";
	this->error_code = 200; // First error code is set to success
	parse_request(request); // Here we parse the request
}

// Main Function that parse the request
bool RequestParser::parse_request(const std::string &request)
{
	const char *start = request.c_str();
	const char *end = start + request.size();
	const char *pos = start;

	while (pos < end && state != DONE && state != ERROR)
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
			state = ERROR;
		}
	}
	return (state == DONE);
}

// Method to extract the request line
const char *RequestParser::parse_request_line(const char *pos, const char *end)
{
	const char *line_end = find_line_end(pos, end);
	if (line_end == end)
	{
		std::cout << HTTP_PARSE_MISSING_REQUEST_LINE << std::endl;
		error_code = 400;
		state = ERROR;
		return pos;
	}

	std::vector<std::string> parts = split(pos, line_end - 2, ' ');
	if (parts.size() != 3)
	{
		std::cout << HTTP_PARSE_INVALID_REQUEST_LINE << std::endl;
		error_code = 400;
		state = ERROR;
		return pos;
	}
	if (!set_http_method(parts[0]) || !set_http_version(parts[2]))
	{
		state = ERROR;
		return pos;
	}
	set_request_uri(parts[1]);
	state = HEADERS;
	return line_end;
}

// Method to extract headers from the request
const char *RequestParser::parse_headers(const char *pos, const char *end)
{
	while (pos < end)
	{
		const char *header_end = find_line_end(pos, end);
		if (header_end == end)
		{
			error_code = 400;
			state = ERROR;
			return pos;
		}

		// If an empty line that means end of headers
		if (pos == header_end - 2)
		{
			state = BODY;
			return header_end;
		}

		std::string header_line(pos, header_end - 2);
		size_t colon_pos = header_line.find(':');
		if (colon_pos == std::string::npos || colon_pos == 0)
		{
			std::cout << HTTP_PARSE_INVALID_HEADER_FIELD << std::endl;
			error_code = 400;
			state = ERROR;
			return pos;
		}
		std::string key = trim(header_line.substr(0, colon_pos), " \t");
		std::string value = trim(header_line.substr(colon_pos + 1), " \t");
		headers[key] = to_lower(value);
		pos = header_end;
	}
	return pos;
}

// Method to extract body of the request
const char *RequestParser::parse_body(const char *pos, const char *end)
{
	std::map<std::string, std::string>::iterator it = headers.find("Transfer-Encoding");

	// Case 1: Transfer-Encoding: chunked
	if (it != headers.end())
	{
		if (it->second == "chunked")
			return parse_chunked_body(pos, end);
		else
		{
			std::cout << HTTP_PARSE_METHOD_NOT_IMPLEMENTED << std::endl;
			error_code = 501;
			state = ERROR;
			return pos;
		}
	}

	// Case 2: Content-Length is present
	it = headers.find("Content-Length");
	if (it != headers.end())
	{
		char *endptr = NULL;
		std::string content_length_str = trim(it->second, " \t");
		unsigned long content_length = std::strtoul(content_length_str.c_str(), &endptr, 10);

		// Check for invalid content length
		if (*endptr != '\0' || content_length > static_cast<size_t>(end - pos))
		{
			std::cout << HTTP_PARSE_INVALID_CONTENT_LENGTH << std::endl;
			error_code = 400;
			state = ERROR;
			return pos;
		}

		// Read exactly Content-Length bytes
		body.assign(pos, pos + content_length);
		pos += content_length;
		state = DONE;
		return pos;
	}

	// Case 3: Read until connection closes
	body.assign(pos, end);
	state = DONE;
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
			error_code = 400;
			state = ERROR;
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
			error_code = 400;
			state = ERROR;
			return pos;
		}

		body.append(pos, chunk_size);
		pos += chunk_size;

		// Ensure each chunk ends with CRLF
		if (pos + 2 > end || pos[0] != '\r' || pos[1] != '\n')
		{
			std::cout << HTTP_PARSE_INVALID_CHUNKED_BODY << std::endl;
			error_code = 400;
			state = ERROR;
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

/****************************
		START SETTERS
****************************/
bool RequestParser::set_http_method(const std::string &http_method)
{
	if (http_method == "GET" || http_method == "POST" || http_method == "DELETE")
	{
		this->http_method = http_method;
		return true;
	}
	if (http_method == "HEAD" || http_method == "OPTIONS" || http_method == "TRACE" || http_method == "PUT" || http_method == "PATCH" || http_method == "CONNECT")
	{
		std::cout << HTTP_PARSE_METHOD_NOT_IMPLEMENTED << std::endl;
		error_code = 501;
	}
	else
	{
		std::cout << HTTP_PARSE_INVALID_METHOD << std::endl;
		error_code = 400;
	}
	return false;
}
void RequestParser::set_request_uri(const std::string &request_uri)
{
	this->request_uri = request_uri;
	size_t query_pos = request_uri.find("?");
	if (query_pos != std::string::npos)
	{
		set_query_string(request_uri.substr(query_pos + 1));
		this->request_uri = request_uri.substr(0, query_pos);
	}
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
		std::cout << HTTP_PARSE_HTTP_VERSION_NOT_SUPPORTED << std::endl;
		error_code = 505;
	}
	else
	{
		std::cout << HTTP_PARSE_INVALID_VERSION " " << http_version << std::endl;
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
std::string &RequestParser::get_body() { return body; }
short RequestParser::get_error_code() { return error_code; }
std::string &RequestParser::get_header_value(const std::string &key) { return headers[key]; }
/****************************
		END GETTERS
****************************/

// Print the parsed request
void RequestParser::print_request()
{
	if (error_code == 200)
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
	std::cout << CYAN "\n** WEBSERV ** ** REQUEST PARSING DONE **" RESET << std::endl;
}

// ********************************* Old version *********************************
// The main function for parsing the request message
// bool RequestParser::parse_request(const std::string &request)
// {
// 	std::istringstream request_stream(request);
// 	std::string first_line;

// 	// Skip leading empty lines
// 	while (std::getline(request_stream, first_line))
// 	{
// 		if (first_line != CRLF && first_line != CR && first_line != LF)
// 			break;
// 	}

// 	// Check if request is empty
// 	if (first_line.empty())
// 	{
// 		std::cerr << RED "Error: Empty request" RESET << std::endl; // To remove later
// 		valid_request = false;
// 		return valid_request;
// 	}

// 	// Remove CR if present
// 	if (!first_line.empty() && first_line[first_line.size() - 1] == '\r')
// 		first_line.erase(first_line.size() - 1);

// 	std::vector<std::string> request_line = split(first_line, SP);
// 	if (request_line.size() != 3)
// 	{
// 		std::cerr << RED "Error in Request line" RESET << std::endl; // To remove later
// 		valid_request = false;
// 		return valid_request;
// 	}

// 	// Validate Request Line
// 	if (!set_http_method(request_line[0]) || !set_request_uri(request_line[1]) || !set_http_version(request_line[2]))
// 		return valid_request;

// 	// Headers Parsing
// 	bool headers_started = false;
// 	bool headers_ended = false;
// 	while (std::getline(request_stream, first_line))
// 	{
// 		if (!first_line.empty() && first_line[first_line.size() - 1] == '\r')
// 			first_line.erase(first_line.size() - 1);

// 		// Detect end of headers
// 		if (first_line.empty())
// 		{
// 			if (headers_ended)
// 			{
// 				std::cerr << RED "Error: Multiple empty lines before body" RESET << std::endl; // To remove later
// 				valid_request = false;
// 				return valid_request;
// 			}
// 			headers_ended = true;
// 			break;
// 		}

// 		// Reject whitespace before headers
// 		if (!headers_started && std::isspace(first_line[0]))
// 		{
// 			valid_request = false;
// 			std::cerr << RED "Error: whitespace before headers" RESET << std::endl; // To remove later
// 			return valid_request;
// 		}

// 		headers_started = true;
// 		size_t colon_pos = first_line.find(":");
// 		if (colon_pos == std::string::npos || colon_pos == 0 || colon_pos == first_line.size() - 1)
// 		{
// 			std::cerr << RED "Error: malformed header -> [" << first_line << "]" RESET << std::endl; // Debugging output
// 			valid_request = false;
// 			return valid_request;
// 		}

// 		std::string key = first_line.substr(0, colon_pos);
// 		std::string value = first_line.substr(colon_pos + 1);
// 		set_headers(trim(key, " \t"), trim(value, " \t"));
// 	}

// 	// Body parsing
// 	std::ostringstream body_stream;
// 	body_stream << request_stream.rdbuf();
// 	std::string body_content = body_stream.str();

// 	if (http_method == "GET" && !body_content.empty())
// 	{
// 		std::cerr << RED "Error: GET request should not have a body" RESET << std::endl; // To remove later
// 		valid_request = false;
// 		return valid_request;
// 	}

// 	set_body(body_content);
// 	return valid_request;
// }