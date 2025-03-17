<<<<<<< HEAD
#include "ResponseBuilder.hpp"
#include "Logger.hpp"

// Here the default directives if not overrided in config file
#define DEFAULT_ERROR_PAGE "error.html"
#define CLIENT_MAX_BODY_SIZE 1048576
=======
#include "../../includes/ResponseBuilder.hpp"

// Here the default directives if not overrided in config file
#define DEFAULT_ERROR_PAGE "error.html"
#define DEFAULT_CLIENT_BODY_SIZE 1048576
>>>>>>> mergeOne
#define DEFAULT_INDEX "index.html"
#define DIRECTORY_LISTING_ENABLED 1
#define UPLOAD_ENABLED 1
#define MAX_UPLOAD_SIZE 5242880

// Here i will define some value to work with (later extract from config file)
#define ROOT_DIRECTORY "www/html"
<<<<<<< HEAD
=======
#define HTTP_REDIRECTION "/redirected_path"
#define UPLOAD_DIRECTORY "www/uploads"
>>>>>>> mergeOne
#define ALLOW_GET 1
#define ALLOW_POST 1
#define ALLOW_DELETE 1

<<<<<<< HEAD
// Static function to initialize the mime types
=======
// function to initialize the mime types
>>>>>>> mergeOne
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
<<<<<<< HEAD
=======
	mime_types[".mkv"] = "video/x-matroska";
	mime_types[".avi"] = "video/x-msvideo";
>>>>>>> mergeOne
	mime_types[".mpg"] = "video/mpeg";
	mime_types[".mpeg"] = "video/mpeg";
	mime_types[".mov"] = "video/quicktime";
	mime_types[".csv"] = "text/csv";
	mime_types[".md"] = "text/markdown";
	mime_types[".yaml"] = "text/yaml";
	mime_types[".yml"] = "text/yaml";
	mime_types[".xhtml"] = "application/xhtml+xml";
<<<<<<< HEAD
	mime_types[".apk"] = "application/vnd.android.package-archive";
	mime_types[".exe"] = "application/x-msdownload";
=======
	mime_types[".rss"] = "application/rss+xml";
	mime_types[".apk"] = "application/vnd.android.package-archive";
	mime_types[".exe"] = "application/x-msdownload";
	mime_types[".dll"] = "application/x-msdownload";
>>>>>>> mergeOne
	mime_types[".bin"] = "application/octet-stream";
	mime_types[".iso"] = "application/x-iso9660-image";

	return mime_types;
}

// All the mime types are stored in this map
std::map<std::string, std::string> ResponseBuilder::mime_types = init_mime_types();

<<<<<<< HEAD
// Constructor Takes Request as parameter
ResponseBuilder::ResponseBuilder(RequestParser &request)
{
	this->http_version = "HTTP/1.1";		  // Only version 1.1 is implemented
	this->status = STATUS_200;				  // Set to success
	this->status_code = 200;				  // Set to success
	init_routes();							  // set routes with allowed methods
	this->response = build_response(request); // Here we build the response
}

// Method to initialize the routes (GET | POST | DELETE)
=======
// Default Constructor
ResponseBuilder::ResponseBuilder(RequestParser &request)
{
	this->http_version = "HTTP/1.1"; // Only version 1.1 is implemented
	this->status = STATUS_200;
	this->headers = std::map<std::string, std::string>();
	this->body = "";
	this->status_code = 200;				  // Default value is success (if no error occurs)
	init_routes();							  // init routes will check for method if allowed or not
	this->response = build_response(request); // Here we build the response
}

// Method to initializ the routes
>>>>>>> mergeOne
void ResponseBuilder::init_routes()
{
	if (ALLOW_GET)
		routes["GET"] = &ResponseBuilder::doGET;
	if (ALLOW_POST)
		routes["POST"] = &ResponseBuilder::doPOST;
	if (ALLOW_DELETE)
		routes["DELETE"] = &ResponseBuilder::doDELETE;
}

<<<<<<< HEAD
// Method to process the response
=======
// Method to handle the request and build the response message
>>>>>>> mergeOne
std::string ResponseBuilder::build_response(RequestParser &request)
{
	short request_error_code = request.get_error_code();

	if (request_error_code != 1) // If an error in request parsing
	{
		set_status(request_error_code);
<<<<<<< HEAD
		body = generate_error_page(status_code);
		include_required_headers(request);
=======
		body = generate_error_page(request_error_code, "Invalid Request");
>>>>>>> mergeOne
		return generate_response_string();
	}

	std::string method = request.get_http_method();
<<<<<<< HEAD
=======
	std::string path = request.get_request_uri();
>>>>>>> mergeOne

	// Return error if the method is not allowed
	if (routes.find(method) == routes.end())
	{
		set_status(405);
<<<<<<< HEAD
		body = generate_error_page(status_code);
		include_required_headers(request);
=======
		set_headers("Allow", "GET, POST, DELETE"); // Check later in nginx and compare
		body = generate_error_page(405, "Method Not Allowed");
>>>>>>> mergeOne
		return generate_response_string();
	}

	// Handle redirections (if there is any)
	if (handle_redirection())
		return generate_response_string();

	// Route the request to the correct handler
	(this->*routes[method])(request);

<<<<<<< HEAD
	// Set required headers
	include_required_headers(request);
=======
	// Set required headers (Later i will write a function for that)
	if (!headers.count("Content-Type"))
		headers["Content-Type"] = detect_mime_type(path); // Set  the content type based on the extension of the request uri

	if (!headers.count("Content-Length"))
	{
		std::ostringstream ss;
		ss << body.size();
		headers["Content-Length"] = ss.str();
	}

	if (!headers.count("Connection"))
		headers["Connection"] = (request.is_keep_alive()) ? "keep-alive" : "close";
>>>>>>> mergeOne

	return generate_response_string();
}

<<<<<<< HEAD
// Method for creating the response
std::string ResponseBuilder::generate_response_string()
{
	std::ostringstream response;
	response << http_version << SP << status;
	LOG_RESPONSE(response.str());
	response << CRLF;
=======
std::string ResponseBuilder::generate_response_string()
{
	std::ostringstream response;
	response << http_version << SP << status << CRLF;
>>>>>>> mergeOne
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
		response << it->first << ": " << it->second << CRLF;
	response << CRLF << body;
	return response.str();
}

<<<<<<< HEAD
// GET method implementation
=======
>>>>>>> mergeOne
void ResponseBuilder::doGET(RequestParser &request)
{
	std::cout << "GET METHOD EXECUTED" << std::endl;
	std::string uri = request.get_request_uri();
	std::string path = ROOT_DIRECTORY + uri;
<<<<<<< HEAD

	// Check if the file exists
	struct stat file_stat;
	if (stat(path.c_str(), &file_stat) == -1)
	{
		set_status(404);
		body = generate_error_page(status_code);
=======
	std::string index = DEFAULT_INDEX;
	std::string file_path = path + index;

	// Check if the file exists
	struct stat file_stat;
	if (stat(file_path.c_str(), &file_stat) == -1)
	{
		set_status(404);
		body = generate_error_page(404, "File Not Found");
>>>>>>> mergeOne
		return;
	}

	// Check if the file is a directory
	if (S_ISDIR(file_stat.st_mode))
	{
<<<<<<< HEAD
=======

>>>>>>> mergeOne
		// If the URI does not end with '/', redirect to the correct URL
		if (uri[uri.size() - 1] != '/')
		{
			set_status(301);
			set_headers("Location", uri + "/");
<<<<<<< HEAD
			body = generate_error_page(status_code);
=======
			body = generate_error_page(301, "Moved Permanently");
>>>>>>> mergeOne
			return;
		}

		// If an index file exists, use it
<<<<<<< HEAD
		std::string index_path = path + DEFAULT_INDEX;
		if (stat(index_path.c_str(), &file_stat) == 0)
		{
			path = index_path;
=======
		std::string index_path = path + index;
		if (stat(index_path.c_str(), &file_stat) == 0)
		{
			file_path = index_path;
>>>>>>> mergeOne
		}
		else if (DIRECTORY_LISTING_ENABLED) // If auto index is enabled in the config file
		{
			body = generate_directory_listing(path);
			set_status(200);
			set_headers("Content-Type", "text/html");
			return;
		}
		else
		{
			set_status(403);
<<<<<<< HEAD
			body = generate_error_page(status_code);
=======
			body = generate_error_page(403, "Directory Listing Denied");
>>>>>>> mergeOne
			return;
		}
	}

	// CGI Execution
<<<<<<< HEAD
	if (is_cgi_request(path))
	{
		// HANDLE CGI IN GET
=======
	if (is_cgi_request(file_path))
	{
		// HANDLE CGI IN GET

>>>>>>> mergeOne
		return;
	}

	// Check Read persmission
	if (!(file_stat.st_mode & S_IRUSR))
	{
		set_status(403);
<<<<<<< HEAD
		body = generate_error_page(status_code);
		return;
	}

	// 	Open The file
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file)
	{
		set_status(500);
		body = generate_error_page(status_code);
=======
		body = generate_error_page(403, "Forbidden");
		return;
	}


	// 	Open The file
	std::ifstream file(file_path.c_str(), std::ios::in | std::ios::binary);
	if (!file)
	{
		set_status(500);
		body = generate_error_page(500, "Internal Server Error");
>>>>>>> mergeOne
		return;
	}

	// Determine mime type
<<<<<<< HEAD
	std::string extension = path.substr(path.find_last_of('.'));
=======
	std::string extension = file_path.substr(file_path.find_last_of('.'));
>>>>>>> mergeOne
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

<<<<<<< HEAD
// POST method implementation
void ResponseBuilder::doPOST(RequestParser &request)
{
	std::cout << "POST METHOD EXECUTED" << std::endl;
	std::string uri = request.get_request_uri();
	std::string path = "www" + uri;
	std::vector<byte> req_body = request.get_body();
	std::string content_type = request.get_header_value("content-type");

	if (is_cgi_request(path))
	{
		// HANDLE CGI IN POST
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
	std::string path = ROOT_DIRECTORY + uri;
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
=======
void ResponseBuilder::doPOST(RequestParser &request)
{
	(void)request;
	std::cout << "POST METHOD EXECUTED" << std::endl;
}

void ResponseBuilder::doDELETE(RequestParser &request)
{
	(void)request;
	std::cout << "DELETE METHOD EXECUTED" << std::endl;
	// std::string uri = request.get_request_uri();
	// std::string path = ROOT_DIRECTORY + uri;
	// if (remove(path.c_str()) == 0)
	// {
	// 	set_status(204);
	// 	body = "";
	// }
	// else
	// {
	// 	set_status(404);
	// 	body = generate_error_page(404, "File Not Found");
	// }
>>>>>>> mergeOne
}

// Method to handle redirection
bool ResponseBuilder::handle_redirection()
{
	if (headers.find("Location") != headers.end())
	{
		short redirect_code = (headers["Location"].find("permanent") != std::string::npos) ? 301 : 302;
		set_status(redirect_code);
<<<<<<< HEAD
		body = generate_error_page(status_code);
=======
		body = generate_error_page(redirect_code, "Redirecting...");
>>>>>>> mergeOne
		return true;
	}
	return false;
}

<<<<<<< HEAD
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
	std::string error_page_name = "errors/" + to_string(status_code) + ".html";

	/* Later I will Bring the name from the server config*/

	std::string error_page_file = readFile(error_page_name);
	if (error_page_file == "")
		LOG_ERROR("Error: Open");
	return error_page_file;
}

// Method to detect the right mime type
=======
// Method to generate error page (temporary)
/* Later This error will be built from reading the html page */
std::string ResponseBuilder::generate_error_page(short status_code, const std::string &message)
{
	std::ostringstream page;
	page << "<html>\n<head>\n  <title>Error " << status_code << "</title>\n</head>\n";
	page << "<body>\n  <h1>" << status << "</h1>\n";
	page << "  <p>" << message << "</p>\n</body>\n</html>";
	return page.str();
}

// Method to find the mime type
>>>>>>> mergeOne
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

<<<<<<< HEAD
// Method generates html page for directory listing
=======
// Method for directory listing
>>>>>>> mergeOne
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

<<<<<<< HEAD
// Method to check if the requested uri is for cgi
=======
>>>>>>> mergeOne
bool ResponseBuilder::is_cgi_request(const std::string &file_path)
{
	(void)file_path;
	return false;
}

<<<<<<< HEAD
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

=======
>>>>>>> mergeOne
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
<<<<<<< HEAD
	case 206:
		this->status = STATUS_206;
		break;
=======
>>>>>>> mergeOne
	case 301:
		this->status = STATUS_301;
		break;
	case 302:
		this->status = STATUS_302;
		break;
<<<<<<< HEAD
	case 303:
		this->status = STATUS_303;
		break;
	case 304:
		this->status = STATUS_304;
		break;
=======
>>>>>>> mergeOne
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
<<<<<<< HEAD
	case 415:
		this->status = STATUS_415;
=======
	case 417:
		this->status = STATUS_417;
>>>>>>> mergeOne
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
<<<<<<< HEAD
		this->status = "UNDEFINED STATUS";
=======
		this->status = "UNDEFINED";
>>>>>>> mergeOne
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
<<<<<<< HEAD
std::string ResponseBuilder::get_header_value(std::string &key) { return headers[key]; }
=======
std::string ResponseBuilder::get_header(std::string &key) { return headers[key]; }
>>>>>>> mergeOne
std::string ResponseBuilder::get_body() { return body; }
short ResponseBuilder::get_status_code() { return status_code; }
/****************************
		END GETTERS
****************************/