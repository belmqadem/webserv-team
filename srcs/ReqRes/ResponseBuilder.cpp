#include "ResponseBuilder.hpp"
#include "Logger.hpp"

std::map<std::string, std::string> ResponseBuilder::mime_types = init_mime_types();
std::map<std::string, std::string> ResponseBuilder::init_mime_types()
{
	std::map<std::string, std::string> mime_types;

	mime_types[".html"] = "text/html";
	mime_types[".htm"] = "text/html";
	mime_types[".css"] = "text/css";
	mime_types[".js"] = "application/javascript";
	mime_types[".json"] = "application/json";
	mime_types[".xml"] = "application/xml";
	mime_types[".txt"] = "text/plain";
	mime_types[".jpg"] = "image/jpeg";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".png"] = "image/png";
	mime_types[".gif"] = "image/gif";
	mime_types[".svg"] = "image/svg+xml";
	mime_types[".ico"] = "image/x-icon";
	mime_types[".webp"] = "image/webp";
	mime_types[".mp4"] = "video/mp4";
	mime_types[".webm"] = "video/webm";
	mime_types[".ogg"] = "video/ogg";
	mime_types[".mp3"] = "audio/mpeg";
	mime_types[".wav"] = "audio/wav";
	mime_types[".pdf"] = "application/pdf";
	mime_types[".csv"] = "text/csv";
	mime_types[".doc"] = "application/msword";
	mime_types[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mime_types[".xls"] = "application/vnd.ms-excel";
	mime_types[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	mime_types[".ppt"] = "application/vnd.ms-powerpoint";
	mime_types[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mime_types[".zip"] = "application/zip";
	mime_types[".rar"] = "application/x-rar-compressed";
	mime_types[".tar"] = "application/x-tar";
	mime_types[".gz"] = "application/gzip";
	mime_types[".iso"] = "application/x-iso9660-image";
	mime_types[".bin"] = "application/octet-stream";
	mime_types[".apk"] = "application/vnd.android.package-archive";
	mime_types[".exe"] = "application/x-msdownload";

	return mime_types;
}

// Response Constructor Takes Request as parameter
ResponseBuilder::ResponseBuilder(RequestParser &raw_request) : request(raw_request), http_version("HTTP/1.1")
{
	init_config();

	// Don't automatically build response for CGI requests
	if (!(is_cgi_request(request.get_request_uri()) &&
		  location_config && location_config->useCgi))
	{
		this->response = build_response();
	}
}

// Method for initializing the Request Matching configuration for server and location
void ResponseBuilder::init_config()
{
	this->server_config = request.get_server_config();
	this->location_config = request.get_location_config();
}

// Method to initialize the routes (GET | POST | DELETE)
void ResponseBuilder::init_routes()
{
	struct MethodMap
	{
		const char *methodName;
		void (ResponseBuilder::*methodPtr)(void);
	};

	static const MethodMap methods[] = {
		{"GET", &ResponseBuilder::doGET},
		{"POST", &ResponseBuilder::doPOST},
		{"DELETE", &ResponseBuilder::doDELETE}};

	for (std::vector<std::string>::const_iterator it = location_config->allowedMethods.begin();
		 it != location_config->allowedMethods.end(); ++it)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			if (*it == methods[i].methodName && routes.find(*it) == routes.end())
			{
				routes.insert(std::make_pair(*it, methods[i].methodPtr));
				break;
			}
		}
	}
}

// Add to ResponseBuilder.cpp:
std::string ResponseBuilder::generate_response_only()
{
    include_required_headers();
    return generate_response_string();
}

// Method to process the response
std::string ResponseBuilder::build_response()
{
	short request_error_code = request.get_error_code();
	if (request_error_code != 1) // If an error in request parsing
	{
		set_status(request_error_code);
		body = generate_error_page();
	}
	else
	{
		init_routes();
		std::string method = request.get_http_method();
		std::map<std::string, void (ResponseBuilder::*)(void)>::iterator it = routes.find(method);
		if (it == routes.end())
		{
			set_status(405);
			body = generate_error_page();
		}
		else
		{
			(this->*(it->second))();
		}
	}

	// handle_session_cookies();

	if (handle_redirection())
	{
		include_required_headers();
		response = generate_response_string();
		return response;
	}

	include_required_headers();
	response = generate_response_string();
	return response;
}

// Method for managing session cookies
void ResponseBuilder::handle_session_cookies()
{
	std::string session_id = SessionCookieHandler::get_cookie(request, "session_id");
	if (session_id.empty())
	{
		session_id = SessionCookieHandler::generate_session_id();
		SessionCookieHandler::set_cookie(*this, "session_id", session_id, 3600); // 1-hour expiration
		LOG_INFO("New session created: " + session_id);
	}
}

// Method for creating the response
std::string ResponseBuilder::generate_response_string()
{
    std::ostringstream response;
    response << http_version << SP << status;
    // Fix the logging to show the complete status line
    LOG_RESPONSE(http_version + " " + status);
    response << CRLF;
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
        response << it->first << ": " << it->second << CRLF;
    response << CRLF << body;
    return response.str();
}

std::pair<std::string, std::string> parseCGIOutput(const std::string &cgiOutput)
{
	std::istringstream stream(cgiOutput);
	std::string line;
	std::string headers;
	std::string body;
	bool headerParsed = false;

	// Read line by line
	while (std::getline(stream, line))
	{
		// Trim any trailing carriage return
		if (!line.empty() && line[line.size() - 1] == '\r')
		{
			line.erase(line.size() - 1);
		}

		// Check for the blank line separating headers from the body
		if (line.empty())
		{
			headerParsed = true;
			break;
		}

		// Append to headers
		headers += line + "\n";
	}

	// If headers are parsed, the rest is the body
	if (headerParsed)
	{
		char buffer[1024];
		while (stream.read(buffer, sizeof(buffer)))
		{
			body.append(buffer, stream.gcount());
		}
		body.append(buffer, stream.gcount());
	}
	// else
	// {
	// 	throw std::runtime_error("Invalid CGI output: Missing headers");
	// }

	// Extract the Content-Type from headers
	std::string contentType = "text/html"; // Default content type
	std::istringstream headerStream(headers);
	while (std::getline(headerStream, line))
	{
		if (line.find("Content-Type:") == 0)
		{
			contentType = line.substr(13); // Extract content type value
			// Trim any whitespace
			size_t start = contentType.find_first_not_of(" \t");
			size_t end = contentType.find_last_not_of(" \t");
			if (start != std::string::npos && end != std::string::npos)
			{
				contentType = contentType.substr(start, end - start + 1);
			}
			else
			{
				contentType = ""; // No valid content type found
			}
			break;
		}
	}

	return std::make_pair(contentType, body);
}

// GET method implementation
void ResponseBuilder::doGET()
{
	LOG_DEBUG("GET METHOD EXECUTED");
	std::string uri = request.get_request_uri();
	bool is_root = (uri == "/") ? true : false;
	std::string root = location_config->root;

	// Check if (uri is /) and (no root directive) in config
	if (is_root && root.empty())
	{
		body = generate_default_root();
		set_status(200);
		return;
	}

	std::string path = root + uri;

	// Check if the file exists
	struct stat file_stat;
	if (stat(path.c_str(), &file_stat) == -1)
	{
		set_status(404);
		body = generate_error_page();
		return;
	}

	// If the requested path is a CGI script - this is now handled by ClientServer
	if (is_cgi_request(uri) && location_config->useCgi)
	{
		// Just return, actual CGI processing happens in ClientServer
		return;
	}

	// Check if the file is a directory
	if (S_ISDIR(file_stat.st_mode))
	{
		// If the URI does not end with '/', redirect to the correct URL
		if (uri[uri.size() - 1] != '/')
		{
			set_status(301);
			body = generate_error_page();
			this->headers["Location"] = uri + "/";
			return;
		}

		// If an index file exists, use it
		std::string index_path = path + location_config->index;
		if (stat(index_path.c_str(), &file_stat) == 0)
		{
			path = index_path;
		}
		else if (location_config->autoindex) // If auto index is enabled in the config file
		{
			body = generate_directory_listing(path);
			if (body == "")
			{
				set_status(403);
				body = generate_error_page();
				return;
			}
			set_status(200);
			set_headers("Content-Type", "text/html");
			return;
		}
		else
		{
			set_status(403);
			body = generate_error_page();
			return;
		}
	}

	// Check Read persmission
	if (!(file_stat.st_mode & S_IRUSR))
	{
		set_status(403);
		body = generate_error_page();
		return;
	}

	body = read_html_file(path);
	set_status(200);
}

// POST method implementation
void ResponseBuilder::doPOST()
{
	LOG_DEBUG("POST METHOD EXECUTED");
	std::string uri = request.get_request_uri();
	std::string path = location_config->root + uri;
	std::string content_type = request.get_header_value("content-type");
	std::vector<byte> req_body = request.get_body();
	std::string upload_path = location_config->uploadStore;

	if (req_body.size() > server_config->clientMaxBodySize)
	{
		LOG_ERROR(HTTP_PARSE_PAYLOAD_TOO_LARGE);
		set_status(413);
		body = generate_error_page();
		return;
	}

	if (is_cgi_request(uri))
	{
		// Just return, actual CGI processing happens in ClientServer
		set_status(200);
		return;
	}

	if (content_type.find("multipart/form-data") != std::string::npos)
	{
		if (!handleMultipartFormData())
		{
			set_status(403);
			body = generate_error_page();
			return;
		}
		set_status(200);
		return;
	}

	if (!validate_upload_path(upload_path))
	{
		set_status(500);
		body = generate_error_page();
		return;
	}

	std::string filename = "uploaded" + Utils::get_timestamp_str() + ".bin";
	std::string full_path = upload_path + "/" + filename;

	// Write the data to the file
	if (!save_uploaded_file(full_path, req_body))
	{
		set_status(403);
		body = generate_error_page();
		return;
	}

	set_status(201);
	LOG_INFO("File uploaded: " + full_path);
	set_body(generate_upload_success_page(filename));
}

// DELETE method implementation
void ResponseBuilder::doDELETE()
{
	LOG_DEBUG("DELETE METHOD EXECUTED");
	std::string uri = request.get_request_uri();
	std::string path = location_config->root + uri;
	struct stat path_stat;

	// Check if the file exists
	if (stat(path.c_str(), &path_stat) != 0)
	{
		set_status(404);
		body = generate_error_page();
		return;
	}

	// Check if The file is a directory
	if (S_ISDIR(path_stat.st_mode))
	{
		// Ensure URI ends with '/'
		if (uri[uri.size() - 1] != '/')
		{
			set_status(409);
			body = generate_error_page();
			return;
		}

		// Check if the directory is empty
		DIR *dir = opendir(path.c_str());
		if (!dir)
		{
			set_status(403);
			body = generate_error_page();
			return;
		}

		struct dirent *entry;
		bool is_empty = true;
		while ((entry = readdir(dir)) != NULL)
		{
			if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
			{
				is_empty = false;
				break;
			}
		}
		closedir(dir);

		if (!is_empty)
		{
			set_status(409);
			body = generate_error_page();
			return;
		}

		// Check write permissions
		if (access(path.c_str(), W_OK) != 0)
		{
			set_status(403);
			body = generate_error_page();
			return;
		}

		// Deleting the directory
		if (rmdir(path.c_str()) == 0)
		{
			set_status(204);
			body = "";
			return;
		}
		else
		{
			set_status(500);
			body = generate_error_page();
			return;
		}
	}

	// Delete file
	if (remove(path.c_str()) == 0)
	{
		set_status(204);
		body = "";
	}
	else
	{
		set_status(500);
		body = generate_error_page();
	}
}

// Method to handle multipart/form-data
bool ResponseBuilder::handleMultipartFormData()
{
	std::string content_type = request.get_header_value("content-type");
	std::vector<byte> req_body = request.get_body();

	// Parse the boundary from the content-type header
	size_t boundary_pos = content_type.find("boundary=");
	if (boundary_pos == std::string::npos)
	{
		LOG_ERROR("Boundary not found in content-type");
		return false;
	}
	std::string boundary = "--" + content_type.substr(boundary_pos + 9);

	// Convert request body to a string
	std::string body(req_body.begin(), req_body.end());

	// Split the body into parts based on the boundary
	size_t pos = 0, next_pos;
	while ((next_pos = body.find(boundary, pos)) != std::string::npos)
	{
		std::string part = body.substr(pos, next_pos - pos);
		pos = next_pos + boundary.length();

		// Parse headers and content
		size_t header_end = part.find("\r\n\r\n");
		if (header_end == std::string::npos)
			continue;
		std::string headers = part.substr(0, header_end);
		std::string content = part.substr(header_end + 4);

		// Parse filename from headers
		size_t filename_pos = headers.find("filename=\"");
		if (filename_pos != std::string::npos)
		{
			size_t filename_end = headers.find("\"", filename_pos + 10);
			std::string filename = headers.substr(filename_pos + 10, filename_end - (filename_pos + 10));

			// Save the file
			std::string full_path = location_config->uploadStore + "/" + filename;
			if (!save_uploaded_file(full_path, std::vector<byte>(content.begin(), content.end())))
			{
				LOG_ERROR("Failed to save uploaded file: " + full_path);
				return false;
			}
			LOG_INFO("File uploaded: " + full_path);
			set_body(generate_upload_success_page(filename));
		}
	}
	return true;
}

// Method to make sure if someone tries to upload a file to a non-existing directory
bool ResponseBuilder::validate_upload_path(const std::string &upload_path)
{
	struct stat dir_stat;
	if (stat(upload_path.c_str(), &dir_stat) == -1 || !S_ISDIR(dir_stat.st_mode))
	{
		LOG_ERROR("Upload path not found: " + upload_path);
		return false;
	}
	return true;
}

// Method to save uploaded files to the server
bool ResponseBuilder::save_uploaded_file(const std::string &full_path, const std::vector<byte> &req_body)
{
	std::ofstream file(full_path.c_str(), std::ios::binary);
	if (!file)
	{
		LOG_ERROR("Error: Cannot open file: " + full_path);
		return false;
	}
	file.write(reinterpret_cast<const char *>(&req_body[0]), req_body.size());
	file.close();
	return true;
}

// Method to handle redirection
bool ResponseBuilder::handle_redirection()
{
	if (location_config && location_config->isRedirect)
	{
		std::string redirect_url = location_config->redirectUrl;
		if (location_config->isRedirectPermanent)
			set_status(301);
		else
			set_status(302);
		body = generate_error_page();
		this->headers["Location"] = redirect_url;
		return true;
	}
	return false;
}

// Method to generate error pages
std::string ResponseBuilder::generate_error_page()
{
	if (server_config && server_config->errorPages.find(status_code) != server_config->errorPages.end())
	{
		std::string error_page_name = server_config->errorPages.at(status_code);
		std::string error_page_file = read_html_file(error_page_name);
		if (error_page_file != "")
			return error_page_file;
	}

	std::ostringstream page;
	page << "<html>\n<head><title>" << status << "</title></head>\n";
	page << "<body>\n<center><h1>" << status << "</h1></center><hr />\n";
	page << "<center>" << WEBSERV_NAME << "</center>\n";
	page << "</body></html>";

	headers["Content-Type"] = "text/html";
	return page.str();
}

// Method to generate default root page
std::string ResponseBuilder::generate_default_root()
{
	std::ostringstream page;
	page << "<html>\n<head><title>Welcome to Webserv!" << "</title></head>\n";
	page << "<body>\n<center><h1>Welcome to Webserv!</h1></center>\n";
	page << "<center><p>If you see this page, the web server is successfully compiled and working. Further configuration is required.<br />Thank you for testing our web server.</p></center>\n";
	page << "</body></html>";

	headers["Content-Type"] = "text/html";
	return page.str();
}

// Method to detect the right mime type
std::string ResponseBuilder::detect_mime_type(const std::string &path)
{
	size_t ext_pos = path.find_last_of(".");
	if (ext_pos != std::string::npos)
	{
		std::string ext = path.substr(ext_pos);
		if (mime_types.find(ext) != mime_types.end())
			return mime_types[ext];
	}
	return "application/octet-stream"; // Default binary type
}

// Method generates html page for directory listing
std::string ResponseBuilder::generate_directory_listing(const std::string &path)
{
	std::ostringstream page;
	page << "<html>\n<head><title>Directory Listing</title></head>\n";
	page << "<body>\n<h1 style=\"color:blue;\">Directory Listing for " << path << "</h1><hr>\n";
	page << "<ul>\n";

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			std::string entry_name = ent->d_name;

			if (entry_name == "." || entry_name == "..")
				continue;

			std::string full_path = path + "/" + entry_name;

			struct stat path_stat;
			if (stat(full_path.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
			{
				page << "<li style=\"letter-spacing: 1.5\"><a href=\"" << entry_name << "/\">" << entry_name << "/</a>&emsp;&emsp;&emsp;" << get_http_date() << "&emsp;&emsp;&emsp;-</li><br>\n";
			}
			else
			{
				page << "<li style=\"letter-spacing: 1.5\"><a href=\"" << entry_name << "\">" << entry_name << "</a>&emsp;&emsp;&emsp;" << get_http_date() << "&emsp;&emsp;&emsp;-</li><br>\n";
			}
		}
		closedir(dir);
	}
	else
	{
		LOG_ERROR("Failed to open directory: " + path);
		return "";
	}

	page << "</ul>\n</body></html>";
	return page.str();
}

// Method to check if the requested uri is for cgi
bool ResponseBuilder::is_cgi_request(const std::string &file_path)
{
	return file_path.size() >= 4 && file_path.compare(file_path.size() - 4, 4, ".php") == 0;
}

// Method to add the required headers into response
void ResponseBuilder::include_required_headers()
{
	// Include standard headers
	headers["Server"] = WEBSERV_NAME;
	headers["Date"] = get_http_date();

	// `Content-Type` header should always be present
	if (headers.find("Content-Type") == headers.end())
	{
		headers["Content-Type"] = detect_mime_type(request.get_request_uri());
	}

	// Only `Content-Length` if no `Transfer-Encoding`
	if (!headers.count("Transfer-Encoding"))
	{
		headers["Content-Length"] = to_string(body.size());
	}
	else if (headers["Transfer-Encoding"] == "chunked")
	{
		headers.erase("Content-Length"); // Chunked encoding should NOT have Content-Length
	}

	// Determine connection behavior
	if (!headers.count("Connection"))
	{
		headers["Connection"] = (request.is_connection_close()) ? "close" : "keep-alive";
	}

	// `Allow` header for 405 Method Not Allowed
	if (this->status_code == 405 && !headers.count("Allow"))
	{
		std::string allowed;
		std::vector<std::string>::const_iterator it = location_config->allowedMethods.begin();
		for (; it != location_config->allowedMethods.end(); ++it)
		{
			if (it != location_config->allowedMethods.end() - 1)
				allowed += (*it) + ", ";
			else
				allowed += (*it);
		}
		headers["Allow"] = allowed;
	}
}

// Method to get date for setting the Date header in response
std::string ResponseBuilder::get_http_date()
{
	char buffer[100];
	std::time_t now = std::time(0);
	std::tm *gmt = std::gmtime(&now);
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
	return std::string(buffer);
}

// Method to read the html file
std::string ResponseBuilder::read_html_file(const std::string &filename)
{
	std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
	if (!file)
	{
		LOG_ERROR("Error: Cannot open file: " + filename);
		return "";
	}

	headers["Content-Type"] = detect_mime_type(filename);

	std::ostringstream content;
	content << file.rdbuf();
	file.close();
	return content.str();
}

// // Method to generate upload success page
std::string ResponseBuilder::generate_upload_success_page(const std::string &filename)
{
	std::ostringstream html;
	html << "<!DOCTYPE html>\n"
		 << "<html>\n"
		 << "<head><title>Upload Successful</title></head>\n"
		 << "<body>\n"
		 << "<h1>File Upload Successful</h1>\n"
		 << "<p>Your file has been uploaded successfully.</p>\n"
		 << "<p><strong>Saved as:</strong> " << filename << "</p>\n"
		 << "<a href=\"/\">Return to Home</a>\n"
		 << "</body>\n"
		 << "</html>";

	set_headers("Content-Type", "text/html");
	return html.str();
}

/****************************
		START SETTERS
****************************/
void ResponseBuilder::set_status(short status_code)
{
	this->status_code = status_code;
	switch (status_code)
	{
	case 200:
		this->status = STATUS_200;
		break;
	case 201:
		this->status = STATUS_201;
		break;
	case 204:
		this->status = STATUS_204;
		break;
	case 301:
		this->status = STATUS_301;
		break;
	case 302:
		this->status = STATUS_302;
		break;
	case 303:
		this->status = STATUS_303;
		break;
	case 304:
		this->status = STATUS_304;
		break;
	case 400:
		this->status = STATUS_400;
		break;
	case 403:
		this->status = STATUS_403;
		break;
	case 404:
		this->status = STATUS_404;
		break;
	case 405:
		this->status = STATUS_405;
		break;
	case 409:
		this->status = STATUS_409;
		break;
	case 410:
		this->status = STATUS_410;
		break;
	case 413:
		this->status = STATUS_413;
		break;
	case 414:
		this->status = STATUS_414;
		break;
	case 415:
		this->status = STATUS_415;
		break;
	case 431:
		this->status = STATUS_431;
		break;
	case 500:
		this->status = STATUS_500;
		break;
	case 501:
		this->status = STATUS_501;
		break;
	case 505:
		this->status = STATUS_505;
		break;
	default:
		this->status = "UNDEFINED STATUS";
		break;
	}
}
void ResponseBuilder::set_headers(const std::string &key, const std::string &value) { this->headers[key] = value; }
void ResponseBuilder::set_body(const std::string &body) { this->body = body; }
/****************************
		END SETTERS
****************************/

/****************************
		START GETTERS
****************************/
std::string ResponseBuilder::get_response() { return response; }
std::string ResponseBuilder::get_http_version() { return http_version; }
std::string ResponseBuilder::get_status() { return status; }
std::map<std::string, std::string> ResponseBuilder::get_headers() { return headers; }
std::string ResponseBuilder::get_header_value(std::string &key) { return headers[key]; }
std::string ResponseBuilder::get_body() { return body; }
short ResponseBuilder::get_status_code() { return status_code; }
/****************************
		END GETTERS
****************************/