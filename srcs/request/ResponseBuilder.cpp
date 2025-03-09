#include "../../includes/ResponseBuilder.hpp"

// Here the default directives if not overrided in config file
#define DEFAULT_ERROR_PAGE "error.html"
#define DEFAULT_CLIENT_BODY_SIZE 1048576
#define DEFAULT_INDEX "index.html"
#define DIRECTORY_LISTING_ENABLED 1
#define UPLOAD_ENABLED 1
#define MAX_UPLOAD_SIZE 5242880

// Here i will define some value to work with (later extract from config file)
#define ROOT_DIRECTORY "www/html"
#define HTTP_REDIRECTION "/redirected_path"
#define UPLOAD_DIRECTORY "www/uploads"
#define ALLOW_GET 1
#define ALLOW_POST 1
#define ALLOW_DELETE 1

// function to initialize the mime types
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
	mime_types[".mkv"] = "video/x-matroska";
	mime_types[".avi"] = "video/x-msvideo";
	mime_types[".mpg"] = "video/mpeg";
	mime_types[".mpeg"] = "video/mpeg";
	mime_types[".mov"] = "video/quicktime";
	mime_types[".csv"] = "text/csv";
	mime_types[".md"] = "text/markdown";
	mime_types[".yaml"] = "text/yaml";
	mime_types[".yml"] = "text/yaml";
	mime_types[".xhtml"] = "application/xhtml+xml";
	mime_types[".rss"] = "application/rss+xml";
	mime_types[".apk"] = "application/vnd.android.package-archive";
	mime_types[".exe"] = "application/x-msdownload";
	mime_types[".dll"] = "application/x-msdownload";
	mime_types[".bin"] = "application/octet-stream";
	mime_types[".iso"] = "application/x-iso9660-image";

	return mime_types;
}

// All the mime types are stored in this map
std::map<std::string, std::string> ResponseBuilder::mime_types = init_mime_types();

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
void ResponseBuilder::init_routes()
{
	if (ALLOW_GET)
		routes["GET"] = &ResponseBuilder::doGET;
	if (ALLOW_POST)
		routes["POST"] = &ResponseBuilder::doPOST;
	if (ALLOW_DELETE)
		routes["DELETE"] = &ResponseBuilder::doDELETE;
}

// Method to handle the request and build the response message
std::string ResponseBuilder::build_response(RequestParser &request)
{
	short request_error_code = request.get_error_code();

	if (request_error_code != 1) // If an error in request parsing
	{
		set_status(request_error_code);
		body = generate_error_page(request_error_code, "Invalid Request");
		return generate_response_string();
	}

	std::string method = request.get_http_method();
	std::string path = request.get_request_uri();

	// Return error if the method is not allowed
	if (routes.find(method) == routes.end())
	{
		set_status(405);
		set_headers("Allow", "GET, POST, DELETE"); // Check later in nginx and compare
		body = generate_error_page(405, "Method Not Allowed");
		if (!headers.count("Content-Length"))
		{
			std::ostringstream ss;
			ss << body.size();
			headers["Content-Length"] = ss.str();
		}
		return generate_response_string();
	}

	// Handle redirections (if there is any)
	if (handle_redirection())
		return generate_response_string();

	// Route the request to the correct handler
	(this->*routes[method])(request);

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

	return generate_response_string();
}

std::string ResponseBuilder::generate_response_string()
{
	std::ostringstream response;
	response << http_version << SP << status << CRLF;
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
		response << it->first << ": " << it->second << CRLF;
	response << CRLF << body;
	return response.str();
}

void ResponseBuilder::doGET(RequestParser &request)
{
	(void)request;
	std::cout << "GET METHOD EXECUTED" << std::endl;
}

void ResponseBuilder::doPOST(RequestParser &request)
{
	(void)request;
	std::cout << "POST METHOD EXECUTED" << std::endl;
}

void ResponseBuilder::doDELETE(RequestParser &request)
{
	(void)request;
	std::cout << "DELETE METHOD EXECUTED" << std::endl;
}

// Method to handle redirection
bool ResponseBuilder::handle_redirection()
{
	if (headers.find("Location") != headers.end())
	{
		short redirect_code = (headers["Location"].find("permanent") != std::string::npos) ? 301 : 302;
		set_status(redirect_code);
		body = generate_error_page(redirect_code, "Redirecting...");
		return true;
	}
	return false;
}

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

// Method for directory listing
// std::string ResponseBuilder::generate_directory_listing(const std::string &path)
// {
// 	std::ostringstream page;
// 	page << "<html><head><title>Directory Listing</title></head>";
// 	page << "<body><h1>Directory Listing</h1>";
// 	page << "<ul>";
// 	DIR *dir;
// 	struct dirent *ent;
// 	if ((dir = opendir(path.c_str())) != NULL)
// 	{
// 		while ((ent = readdir(dir)) != NULL)
// 		{
// 			page << "<li><a href=\"" << ent->d_name << "\">" << ent->d_name << "</a></li>";
// 		}
// 		closedir(dir);
// 	}
// 	else
// 	{
// 		page << "<p>Error: Could not open directory</p>";
// 	}
// 	page << "</ul></body></html>";
// 	return page.str();
// }

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
	case 417:
		this->status = STATUS_417;
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
		this->status = "UNDEFINED";
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
std::string ResponseBuilder::get_header(std::string &key) { return headers[key]; }
std::string ResponseBuilder::get_body() { return body; }
short ResponseBuilder::get_status_code() { return status_code; }
/****************************
		END GETTERS
****************************/