#include "ResponseBuilder.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
// #include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>

class SessionCookieHandler
{
public:
    // Generate a session ID based on current time and a random number
    static std::string generate_session_id()
    {
        time_t now = time(NULL);
        struct tm *timeinfo = localtime(&now);

        char buffer[14];
        strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", timeinfo);

        srand(time(NULL)); // Seed for random number generation
        int random_number = rand() % 10000;

        // Using manual string concatenation in C++98
        std::string session_id(buffer);
        session_id += "_" + int_to_string(random_number); // Convert integer to string

        return session_id;
    }

    // Convert integer to string (manual conversion)
    static std::string int_to_string(int number)
    {
        std::ostringstream oss;
        oss << number;
        return oss.str(); // Convert int to string using ostringstream (valid in C++98)
    }

    // Set a cookie
    static void set_cookie(ResponseBuilder &response, const std::string &name, const std::string &value, int max_age_seconds = 3600)
    {
        std::string cookie_header = name + "=" + value;
        if (max_age_seconds > 0)
        {
            cookie_header += "; Max-Age=" + int_to_string(max_age_seconds);
        }
        cookie_header += "; Path=/";
        cookie_header += "; HttpOnly; SameSite=Strict";

        response.set_headers("Set-Cookie", cookie_header); // Add cookie to headers
    }

    // Get a cookie value from request headers
    static std::string get_cookie( RequestParser &request, const std::string &name)
    {
        std::string cookie_header = request.get_header_value("Cookie");
        size_t start_pos = cookie_header.find(name + "=");
        if (start_pos != std::string::npos)
        {
            start_pos += name.length() + 1;
            size_t end_pos = cookie_header.find(";", start_pos);
            if (end_pos == std::string::npos)
            {
                end_pos = cookie_header.length();
            }
            return cookie_header.substr(start_pos, end_pos - start_pos);
        }
        return ""; // Return empty string if cookie not found
    }

    // Delete a cookie (set its Max-Age to 0)
    static void delete_cookie(ResponseBuilder &response, const std::string &name)
    {
        std::string cookie_header = name + "=; Max-Age=0; Path=/";
        response.set_headers("Set-Cookie", cookie_header); // Add cookie to headers for deletion
    }

    // Validate the session (check if a session ID exists in cookies)
    static bool validate_session( RequestParser &request)
    {
        std::string session_id = get_cookie(request, "session_id");
        return !session_id.empty(); // If session_id exists, consider the session valid
    }
};




// Static function to initialize the mime types
std::map<std::string, std::string> ResponseBuilder::init_mime_types()
{
	std::map<std::string, std::string> mime_types;

	mime_types[".html"] = "text/html";
	mime_types[".htm"] = "text/html";
	mime_types[".shtml"] = "text/html";
	mime_types[".css"] = "text/css";
	mime_types[".xml"] = "application/xml";
	mime_types[".js"] = "application/javascript";
	mime_types[".json"] = "application/json";
	mime_types[".atom"] = "application/atom+xml";
	mime_types[".jpg"] = "image/jpeg";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".png"] = "image/png";
	mime_types[".gif"] = "image/gif";
	mime_types[".svg"] = "image/svg+xml";
	mime_types[".ico"] = "image/x-icon";
	mime_types[".txt"] = "text/plain";
	mime_types[".pdf"] = "application/pdf";
	mime_types[".mp3"] = "audio/mpeg";
	mime_types[".mp4"] = "video/mp4";
	mime_types[".wav"] = "audio/wav";
	mime_types[".zip"] = "application/zip";
	mime_types[".rar"] = "application/x-rar-compressed";
	mime_types[".tar"] = "application/x-tar";
	mime_types[".gz"] = "application/gzip";
	mime_types[".xz"] = "application/x-xz";
	mime_types[".svgz"] = "image/svg+xml";
	mime_types[".wasm"] = "application/wasm";
	mime_types[".map"] = "application/json";
	mime_types[".webp"] = "image/webp";
	mime_types[".avif"] = "image/avif";
	mime_types[".bmp"] = "image/bmp";
	mime_types[".ogv"] = "video/ogg";
	mime_types[".opus"] = "audio/opus";
	mime_types[".flac"] = "audio/flac";
	mime_types[".webm"] = "video/webm";
	mime_types[".mpg"] = "video/mpeg";
	mime_types[".mpeg"] = "video/mpeg";
	mime_types[".mov"] = "video/quicktime";
	mime_types[".csv"] = "text/csv";
	mime_types[".md"] = "text/markdown";
	mime_types[".yaml"] = "text/yaml";
	mime_types[".yml"] = "text/yaml";
	mime_types[".xhtml"] = "application/xhtml+xml";
	mime_types[".apk"] = "application/vnd.android.package-archive";
	mime_types[".exe"] = "application/x-msdownload";
	mime_types[".bin"] = "application/octet-stream";
	mime_types[".iso"] = "application/x-iso9660-image";
	mime_types[".img"] = "application/x-iso9660-image";
	mime_types[".avi"] = "video/x-msvideo";
	mime_types[".ppt"] = "application/vnd.ms-powerpoint";
	mime_types[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mime_types[".xls"] = "application/vnd.ms-excel";
	mime_types[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	mime_types[".doc"] = "application/msword";
	mime_types[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mime_types[".odt"] = "application/vnd.oasis.opendocument.text";
	mime_types[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
	mime_types[".odp"] = "application/vnd.oasis.opendocument.presentation";
	mime_types[".ics"] = "text/calendar";
	mime_types[".vcf"] = "text/x-vcard";
	mime_types[".torrent"] = "application/x-bittorrent";
	mime_types[".m3u"] = "audio/x-mpegurl";
	mime_types[".m3u8"] = "application/vnd.apple.mpegurl";
	mime_types[".ts"] = "video/mp2t";
	mime_types[".mkv"] = "video/x-matros";
	mime_types[".webmanifest"] = "application/manifest+json";

	return mime_types;
}

// All the mime types are stored in this map
std::map<std::string, std::string> ResponseBuilder::mime_types = init_mime_types();

// Method for initializing the Request Matching configuration for server and location
void ResponseBuilder::init_config()
{
	this->server_config = request.get_server_config();
	this->location_config = request.get_location_config();
}

// Constructor Takes Request as parameter
ResponseBuilder::ResponseBuilder(RequestParser &raw_request)
{
	this->http_version = "HTTP/1.1";
	this->request = raw_request;
	init_config();
	this->response = build_response();
}

// Method to initialize the routes (GET | POST | DELETE)
void ResponseBuilder::init_routes()
{
	std::vector<std::string>::const_iterator it = location_config->allowedMethods.begin();
	for (; it != location_config->allowedMethods.end(); ++it)
	{
		if (*it == "GET")
			routes["GET"] = &ResponseBuilder::doGET;
		else if (*it == "POST")
			routes["POST"] = &ResponseBuilder::doPOST;
		else if (*it == "DELETE")
			routes["DELETE"] = &ResponseBuilder::doDELETE;
	}
}

// Method to process the response
std::string ResponseBuilder::build_response()
{
	short request_error_code = request.get_error_code();

	if (request_error_code != 1) // If an error in request parsing
	{
		set_status(request_error_code);
		body = generate_error_page(status_code);
		include_required_headers();
		return generate_response_string();
	}

	init_routes();

	std::string method = request.get_http_method();

	// Check if the method is not allowed in config file
	if (routes.find(method) == routes.end())
	{
		set_status(405);
		body = generate_error_page(status_code);
		include_required_headers();
		return generate_response_string();
	}

	// Handle redirections (if there is any)
	if (handle_redirection())
		return generate_response_string();

	// Route the request to the correct handler
	(this->*routes[method])();

	// Set required headers
	include_required_headers();

	return generate_response_string();
}

// Method for creating the response
std::string ResponseBuilder::generate_response_string()
{
	std::ostringstream response;
	response << http_version << SP << status;
	LOG_RESPONSE(response.str());
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
	std::string path;

	// Check if (uri is /) and (no root directive) in config
	(is_root && root.empty()) ? path = "default/root.html" : path = root + uri;

	// Check if the file exists
	struct stat file_stat;
	if (stat(path.c_str(), &file_stat) == -1)
	{
		set_status(404);
		body = generate_error_page(status_code);
		return;
	}

	// If the requested path is a CGI script
	std::string session_id = SessionCookieHandler::get_cookie(request, "session_id");
	if (session_id.empty())
	{
		// If no session, generate and set a new session ID
		session_id = SessionCookieHandler::generate_session_id();
		SessionCookieHandler::set_cookie(*this, "session_id", session_id, 3600); // Set session cookie with 1-hour expiration
		LOG_INFO("New session created: " + session_id);
	}
	if (is_cgi_request(uri) && location_config->useCgi)
	{
		CGIHandler cgiHandler(request, "/usr/bin/php-cgi");
		try
		{
			std::string cgiOutput = cgiHandler.executeCGI();
			std::pair<std::string, std::string> parsedOutput = parseCGIOutput(cgiOutput);
			// Set headers and body in the response
			set_headers("Content-Type", parsedOutput.first); // Use the content type from the CGI output
			body = parsedOutput.second;
			set_status(200);
		}
		catch (const std::exception &e)
		{
			set_status(500);
			body = generate_error_page(status_code);
			Logger::getInstance().error(e.what());
		}
		return;
	}
	// Check if the file is a directory
	if (S_ISDIR(file_stat.st_mode))
	{
		// If the URI does not end with '/', redirect to the correct URL
		if (uri[uri.size() - 1] != '/')
		{
			set_status(301);
			std::string file_name = "default/" + to_string(status_code) + ".html";
			body = read_html_file(file_name);
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
				body = generate_error_page(status_code);
				return;
			}
			set_status(200);
			set_headers("Content-Type", "text/html");
			return;
		}
		else
		{
			set_status(403);
			body = generate_error_page(status_code);
			return;
		}
	}

	// Check Read persmission
	if (!(file_stat.st_mode & S_IRUSR))
	{
		set_status(403);
		body = generate_error_page(status_code);
		return;
	}

	body = read_html_file(path);
	set_status(200);
}
bool ResponseBuilder::isFileUpload( RequestParser request) {
    // Ensure the configuration is initialized
		std::string content_type = request.get_header_value("Content-Type");
		LOG_DEBUG("Content-Type: " + content_type);
		return content_type.find("multipart/form-data") != std::string::npos;
	 // File upload is allowed
}
bool ResponseBuilder::handleMultipartFormData(const std::vector<unsigned char>& req_body, const std::string& content_type, const std::string& upload_path) {
    // Parse the boundary from the content-type header
    size_t boundary_pos = content_type.find("boundary=");
    if (boundary_pos == std::string::npos) {
        std::cerr << "Boundary not found in content-type" << std::endl;
        return false;
    }
    std::string boundary = "--" + content_type.substr(boundary_pos + 9);

    // Convert request body to a string
    std::string body(req_body.begin(), req_body.end());

    // Split the body into parts based on the boundary
    size_t pos = 0, next_pos;
    while ((next_pos = body.find(boundary, pos)) != std::string::npos) {
        std::string part = body.substr(pos, next_pos - pos);
        pos = next_pos + boundary.length();

        // Parse headers and content
        size_t header_end = part.find("\r\n\r\n");
        if (header_end == std::string::npos) continue;
        std::string headers = part.substr(0, header_end);
        std::string content = part.substr(header_end + 4);

        // Parse filename from headers
        size_t filename_pos = headers.find("filename=\"");
        if (filename_pos != std::string::npos) {
            size_t filename_end = headers.find("\"", filename_pos + 10);
            std::string filename = headers.substr(filename_pos + 10, filename_end - (filename_pos + 10));

            // Save the file
            std::string full_path = upload_path + "/" + filename;
            std::ofstream file(full_path.c_str(), std::ios::binary);
            if (!file) {
                std::cerr << "Failed to open file for writing: " << full_path << std::endl;
                return false;
            }
            file.write(content.c_str(), content.size());
            file.close();
            std::cout << "File uploaded: " << full_path << std::endl;
			generate_upload_success_page(filename);
        }
    }

    return true;
}

// POST method implementation
void ResponseBuilder::doPOST()
{
	LOG_DEBUG("POST METHOD EXECUTED");
	std::string uri = request.get_request_uri();
	std::string path = location_config->root + uri;
	std::string content_type = request.get_header_value("content-type");
	std::vector<byte> req_body = request.get_body();
	

	if (req_body.size() > server_config->clientMaxBodySize)
	{
		LOG_ERROR(HTTP_PARSE_PAYLOAD_TOO_LARGE);
		set_status(413);
		body = generate_error_page(status_code);
		return;
	}
	if(content_type.find("multipart/form-data") != std::string::npos)
	{
		if(!handleMultipartFormData(req_body,content_type,location_config->uploadStore))
		{
			set_status(403);
			body = generate_error_page(status_code);
			return;
		}
		return;
	}
	std::string session_id = SessionCookieHandler::get_cookie(request, "session_id");
    if (session_id.empty())
    {
        // If no session, generate and set a new session ID
        session_id = SessionCookieHandler::generate_session_id();
        SessionCookieHandler::set_cookie(*this, "session_id", session_id, 3600); // Set session cookie with 1-hour expiration
        LOG_INFO("New session created: " + session_id);
    }
	// Reject file upload if content type is multipart/form-data

	if (is_cgi_request(uri))
	{
		CGIHandler cgiHandler(request, "/usr/bin/php-cgi");
		try
		{
			std::string cgiOutput = cgiHandler.executeCGI();
			std::pair<std::string, std::string> parsedOutput = parseCGIOutput(cgiOutput);
			// Set headers and body in the response
			set_headers("Content-Type", parsedOutput.first); // Use the content type from the CGI output
			body = parsedOutput.second;
			set_status(200);
		}
		catch (const std::exception &e)
		{
			set_status(500);
			body = generate_error_page(status_code);
			Logger::getInstance().error(e.what());
		}
		return;
	}

	std::string upload_path = location_config->uploadStore;

	// Just to make sure if someone tries to upload a file to a non-existing directory
	struct stat dir_stat;
	if (stat(upload_path.c_str(), &dir_stat) == -1 || !S_ISDIR(dir_stat.st_mode))
	{
		LOG_ERROR("Upload path not found: " + upload_path);
		set_status(500);
		body = generate_error_page(status_code);
		return;
	}

	std::string filename = "uploaded" + Utils::get_timestamp_str() + ".bin";
	std::string full_path = upload_path + "/" + filename;

	// Write the data to the file
	std::ofstream file(full_path.c_str(), std::ios::binary);
	if (!file)
	{
		set_status(403);
		body = generate_error_page(status_code);
		return;
	}
	file.write(reinterpret_cast<const char *>(&req_body[0]), req_body.size());
	file.close();
	// set_cookie("session_id", generate_session_id(), 3600);
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
		body = generate_error_page(status_code);
		return;
	}

	// Check if The file is a directory
	if (S_ISDIR(path_stat.st_mode))
	{
		// Ensure URI ends with '/'
		if (uri[uri.size() - 1] != '/')
		{
			set_status(409);
			body = generate_error_page(status_code);
			return;
		}

		// Check if the directory is empty
		DIR *dir = opendir(path.c_str());
		if (!dir)
		{
			set_status(403);
			body = generate_error_page(status_code);
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
			body = generate_error_page(status_code);
			return;
		}

		// Check write permissions
		if (access(path.c_str(), W_OK) != 0)
		{
			set_status(403);
			body = generate_error_page(status_code);
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
			body = generate_error_page(status_code);
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
		body = generate_error_page(status_code);
	}
}

// Method to handle redirection
bool ResponseBuilder::handle_redirection()
{
	if (location_config->isRedirect)
	{
		std::string redirect_url = location_config->redirectUrl;
		if (location_config->isRedirectPermanent)
			set_status(301);
		else
			set_status(302);
		std::string file_name = "default/" + to_string(status_code) + ".html";
		body = read_html_file(file_name);
		this->headers["Location"] = redirect_url;
		include_required_headers();
		return true;
	}
	return false;
}

// Method to generate error pages
std::string ResponseBuilder::generate_error_page(short status_code)
{
	std::string error_page_name = server_config->errorPages.at(status_code);
	std::string error_page_file = read_html_file(error_page_name);
	return error_page_file;
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
	page << "<body>\n<h1>Directory Listing</h1><hr>\n";
	page << "<ul>\n";
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			page << "<li style=\"letter-spacing: 1.5\"><a href=\"" << ent->d_name << "/\">" << ent->d_name << "</a>&emsp;&emsp;&emsp;" << get_http_date() << "&emsp;&emsp;&emsp;-</li><br>\n";
		}
		closedir(dir);
	}
	else
	{
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
	if (!headers.count("Content-Type"))
		headers["Content-Type"] = detect_mime_type(request.get_request_uri());

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
		headers["Connection"] = (request.is_connection_close()) ? "close" : "keep-alive";

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
		set_status(500);
		body = generate_error_page(status_code);
		return "";
	}

	// Determine mime type
	std::string extension = filename.substr(filename.find_last_of('.'));
	std::map<std::string, std::string>::iterator it = mime_types.find(extension);
	if (it != mime_types.end())
		set_headers("Content-Type", it->second);
	else
		set_headers("Content-Type", "application/octet-stream");

	std::ostringstream content;
	content << file.rdbuf();
	file.close();
	return content.str();
}

// void ResponseBuilder::handleFileUpload( std::string body, const std::map<std::string, std::string> &headers) {
//     // Check Content-Type for multipart/form-data
//     if (headers.find("Content-Type") == headers.end() || headers.at("Content-Type").find("multipart/form-data") == std::string::npos) {
//         set_status(400);
//         body = "400 Bad Request: Not a multipart form-data request.";
//         return;
//     }

//     // Extract boundary from Content-Type
//     std::string boundary = headers.at("Content-Type");
//     size_t boundaryPos = boundary.find("boundary=");
//     if (boundaryPos == std::string::npos) {
//         set_status(400);
//         body = "400 Bad Request: Missing boundary.";
//         return;
//     }
//     boundary = "--" + boundary.substr(boundaryPos + 9); // "boundary=" is 9 characters long

//     // Split body into parts based on boundary
//     std::vector<std::string> parts;
//     size_t start = body.find(boundary);
//     while (start != std::string::npos) {
//         size_t end = body.find(boundary, start + boundary.length());
//         parts.push_back(body.substr(start + boundary.length() + 2, end - start - boundary.length() - 4));
//         start = end;
//     }

//     // Process each part
//     for (const std::string &part : parts) {
//         size_t headerEnd = part.find("\r\n\r\n");
//         if (headerEnd == std::string::npos) continue;

//         // Extract headers
//         std::string headersPart = part.substr(0, headerEnd);
//         std::string fileContent = part.substr(headerEnd + 4);

//         // Look for Content-Disposition
//         size_t filenamePos = headersPart.find("filename=\"");
//         if (filenamePos != std::string::npos) {
//             size_t filenameEnd = headersPart.find("\"", filenamePos + 10);
//             std::string filename = headersPart.substr(filenamePos + 10, filenameEnd - filenamePos - 10);

//             // Save file to upload directory
//             std::string filePath = location_config->uploadStore + "/" + filename;
//             std::ofstream outFile(filePath.c_str(), std::ios::binary);
//             if (outFile) {
//                 outFile.write(fileContent.c_str(), fileContent.size());
//                 outFile.close();
//             } else {
//                 set_status(500);
//                 body = "500 Internal Server Error: Failed to save file.";
//                 return;
//             }
//         }
//     }

//     // Respond with success
//     set_status(200);
//     body = "File uploaded successfully.";
// }

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
void ResponseBuilder::set_http_version(const std::string &http_version) { this->http_version = http_version; }
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