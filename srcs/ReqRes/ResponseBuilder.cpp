#include "ResponseBuilder.hpp"
#include "Logger.hpp"

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

	return mime_types;
}

// All the mime types are stored in this map
std::map<std::string, std::string> ResponseBuilder::mime_types = init_mime_types();

// Method for initializing the Request Matching configuration for server and location
void ResponseBuilder::init_config(RequestParser &request)
{
	this->server_config = request.get_server_config();
	this->location_config = request.get_location_config();
}

// Constructor Takes Request as parameter
ResponseBuilder::ResponseBuilder(RequestParser &request)
{
	this->http_version = "HTTP/1.1";		  // Only version 1.1 is implemented
	this->status = STATUS_200;				  // Set to success
	this->status_code = 200;				  // Set to success
	this->init_config(request);				  // Init the config from request match
	init_routes();							  // set routes with allowed methods
	this->response = build_response(request); // Here we build the response
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
std::string ResponseBuilder::build_response(RequestParser &request)
{
	short request_error_code = request.get_error_code();

	if (request_error_code != 1) // If an error in request parsing
	{
		set_status(request_error_code);
		body = generate_error_page(status_code);
		include_required_headers(request);
		return generate_response_string();
	}

	std::string method = request.get_http_method();

	// Return error if the method is not allowed
	if (routes.find(method) == routes.end())
	{
		set_status(405);
		body = generate_error_page(status_code);
		include_required_headers(request);
		return generate_response_string();
	}

	// Handle redirections (if there is any)
	if (handle_redirection())
		return generate_response_string();

	// Route the request to the correct handler
	(this->*routes[method])(request);

	// Set required headers
	include_required_headers(request);

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

// GET method implementation
void ResponseBuilder::doGET(RequestParser &request)
{
	std::cout << "GET METHOD EXECUTED" << std::endl;
	std::string uri = request.get_request_uri();
	std::string path = location_config->root + uri;
	std::cout << path << std::endl;
	if(is_cgi_request(uri))
	{
		path = "www/cgi-bin/test.php";
	}
	// path = "www/cgi-bin/test.php";
	std::cout << path << std::endl;
	// Check if the file exists
	struct stat file_stat;
	if (stat(path.c_str(), &file_stat) == -1)
	{
		set_status(404);
		body = generate_error_page(status_code);
		return;
	}

	// Check if the file is a directory
	if (S_ISDIR(file_stat.st_mode))
	{
		// If the URI does not end with '/', redirect to the correct URL
		if (uri[uri.size() - 1] != '/')
		{
			set_status(301);
			set_headers("Location", uri + "/");
			body = generate_error_page(status_code);
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

	// CGI Execution
	if (is_cgi_request(path))
	{
		// std::cout << "hello from get===================================================================================================================================================================================="<< std::endl; 
		CGIHandler cgi(request,*server_config ,*location_config);
		cgi.executeCGI();
		body = cgi.getOutput();  // Store CGI output in the response bodys
		set_status(200);  // Assuming CGI executed successfully
		set_headers("Content-Type", "text/html"); // Adjust based on CGI output
		return;
	}

	// Check Read persmission
	if (!(file_stat.st_mode & S_IRUSR))
	{
		set_status(403);
		body = generate_error_page(status_code);
		return;
	}

	// 	Open The file
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file)
	{
		set_status(500);
		body = generate_error_page(status_code);
		return;
	}

	// Determine mime type
	std::string extension = path.substr(path.find_last_of('.'));
	std::map<std::string, std::string>::iterator it = mime_types.find(extension);
	if (it != mime_types.end())
		set_headers("Content-Type", it->second);
	else
		set_headers("Content-Type", "application/octet-stream");

	// Read File
	std::ostringstream file_stream;
	file_stream << file.rdbuf();
	body = file_stream.str();
	set_status(200);
}

// POST method implementation
void ResponseBuilder::doPOST(RequestParser &request)
{
	std::cout << "POST METHOD EXECUTED" << std::endl;
	std::string uri = request.get_request_uri();
	std::string path = location_config->root + uri;
	std::vector<byte> req_body = request.get_body();
	std::string content_type = request.get_header_value("content-type");

	if (is_cgi_request(path))
	{
		// std::cout << "hello from get===================================================================================================================================================================================="<< std::endl; 
		CGIHandler cgi(request,*server_config ,*location_config);
		cgi.executeCGI();
		body = cgi.getOutput();  // Store CGI output in the response bodys
		set_status(200);  // Assuming CGI executed successfully
		set_headers("Content-Type", "text/html"); // Adjust based on CGI output
		return;
	}

	if (req_body.size() == 0)
	{
		set_status(400);
		body = generate_error_page(status_code);
		return;
	}

	// Handle file upload if content type is multipart/form-data
	if (content_type.find("multipart/form-data") != std::string::npos)
	{
		if (!handle_file_upload(request, path))
		{
			set_status(500);
			set_body(generate_error_page(status_code));
			return;
		}
		set_status(201);
		set_body("File uploaded successfully");
		return;
	}

	// Handle file upload if content type is application/octet-stream (binary)
	if (content_type == "application/json")
	{
		if (!handle_json_upload(request, path))
		{
			set_status(500);
			body = generate_error_page(status_code);
			return;
		}
		set_status(201);
		body = "File uploaded successfully";
		return;
	}

	// Handle normal POST request -- Regular data
	std::ofstream file(path.c_str(), std::ios::binary);
	if (!file)
	{
		set_status(403);
		body = generate_error_page(status_code);
		return;
	}
	file.write(reinterpret_cast<const char *>(&req_body[0]), req_body.size());
	file.close();
	set_status(201);
	set_body("Data written successfully");
}

// DELETE method implementation
void ResponseBuilder::doDELETE(RequestParser &request)
{
	std::cout << "DELETE METHOD EXECUTED" << std::endl;

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

	// Check if it's a file or directory
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
		return;
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
	if (headers.find("Location") != headers.end())
	{
		short redirect_code = (headers["Location"].find("permanent") != std::string::npos) ? 301 : 302;
		set_status(redirect_code);
		body = generate_error_page(status_code);
		return true;
	}
	return false;
}

// Method to handle file upload (multipart/form-data)
bool ResponseBuilder::handle_file_upload(RequestParser &request, const std::string &path)
{
	std::string boundary = request.get_header_value("content-type");
	boundary = boundary.substr(boundary.find("boundary=") + 9);
	std::string end_boundary = "--" + boundary + "--";
	std::string content = std::string(request.get_body().begin(), request.get_body().end());
	size_t start = content.find(boundary);
	size_t end = content.find(end_boundary);
	if (start == std::string::npos || end == std::string::npos)
		return false;

	std::string file_content = content.substr(start, end - start);
	std::string file_name = path + "uploaded_file";

	/*Later the file name should be generated randomly*/

	std::ofstream file(file_name.c_str(), std::ios::binary);
	if (!file)
		return false;
	file.write(file_content.c_str(), file_content.size());
	file.close();
	return true;
}

// Method to handle json upload (application/json)
bool ResponseBuilder::handle_json_upload(RequestParser &request, const std::string &path)
{
	// Implement this method later
	(void)request;
	(void)path;
	return true;
}

// Method to generate error pages
std::string ResponseBuilder::generate_error_page(short status_code)
{
	std::string error_page_name = server_config->errorPages.at(status_code);
	std::string error_page_file = readFile(error_page_name);
	if (error_page_file == "")
		LOG_ERROR("Error: Open");
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
	page << "<html><head><title>Directory Listing</title></head>";
	page << "<body><h1>Directory Listing</h1>";
	page << "<ul>";
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			page << "<li><a href=\"" << ent->d_name << "\">" << ent->d_name << "</a></li>";
		}
		closedir(dir);
	}
	else
	{
		page << "<p>Error: Could not open directory</p>";
	}
	page << "</ul></body></html>";
	return page.str();
}

// Method to check if the requested uri is for cgi
bool ResponseBuilder::is_cgi_request(const std::string &file_path)
{
	std::vector<std::string> cgi_extensions;
	cgi_extensions.push_back(".php");
	cgi_extensions.push_back(".py");
	// cgi_extensions.push_back(".js");
    size_t dot_pos = file_path.find_last_of('.');
    
    if (dot_pos != std::string::npos) {
        std::string ext = file_path.substr(dot_pos);
        for (size_t i = 0; i < cgi_extensions.size(); i++) {
            if (ext == cgi_extensions[i])
                return true;
        }
    }
    return false;
	// (void)file_path;
	// return true;
}

// Method to add the required headers into response
void ResponseBuilder::include_required_headers(RequestParser &request)
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
		headers["Connection"] = (request.is_connection_keep_alive()) ? "keep-alive" : "close";

	// `Allow` header for 405 Method Not Allowed
	if (status_code == 405 && !headers.count("Allow"))
		headers["Allow"] = "GET, POST, DELETE";
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
	case 206:
		this->status = STATUS_206;
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