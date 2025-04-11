#include "ResponseBuilder.hpp"

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
	processSessionCookie();
}

// Method to initialize session cookies
void ResponseBuilder::processSessionCookie()
{
	std::string session_id = SessionCookieHandler::get_cookie(request, "session_id");
	if (session_id.empty())
	{
		session_id = SessionCookieHandler::generate_session_id();
		SessionCookieHandler::set_cookie(*this, "session_id", session_id, 3600);
	}
}

// Method for initializing the Request Matching configuration for server and location
void ResponseBuilder::init_config()
{
	this->server_config = request.get_server_config();
	this->location_config = request.get_location_config();

	if (!this->server_config || !this->location_config)
	{
		set_status(404);
		body = generate_error_page();
		return;
	}
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

// Method to process the response
std::string ResponseBuilder::build_response()
{
	short request_error_code = request.get_error_code();
	if (request_error_code) // If an error in request parsing (error code != 0)
	{
		set_status(request_error_code);
		body = generate_error_page();
	}
	else
	{
		// Check if location has a return directive
		if (location_config && location_config->has_return)
		{
			set_status(location_config->return_code);
			set_body(location_config->return_message);

			// Set content type to plain text if it's a message
			if (!location_config->return_message.empty())
			{
				set_headers("Content-Type", "text/plain");
			}
		}
		else
		{
			// Existing route handling
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

		handle_redirection();
	}

	return generate_response_only();
}

// Generate response structure with including headers
std::string ResponseBuilder::generate_response_only()
{
	include_required_headers();
	return generate_response_string();
}

// Method for creating the response
std::string ResponseBuilder::generate_response_string()
{
	LOG_RESPONSE(http_version + SP + status);

	std::ostringstream response;
	response << http_version << SP << status << CRLF;
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
		response << it->first << ": " << it->second << CRLF;
	response << CRLF << body;
	return response.str();
}

// GET method implementation
void ResponseBuilder::doGET()
{
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

	body = read_file(path);
	set_status(200);
}

// POST method implementation
void ResponseBuilder::doPOST()
{
	std::string content_type = request.get_header_value("content-type");
	std::vector<byte> req_body = request.get_body();
	std::string upload_path = location_config->uploadStore;

	if (!validate_upload_path(upload_path))
	{
		set_status(500);
		body = generate_error_page();
		return;
	}

	if (content_type.find("multipart/form-data") != std::string::npos)
	{
		if (!handleMultipartFormData(content_type, req_body))
		{
			set_status(403);
			body = generate_error_page();
			return;
		}
		return;
	}

	std::string filename = "upload_" + Utils::get_timestamp_str() + ".bin";
	std::string full_path = upload_path + "/" + filename;

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
			set_status(301);
			set_headers("Location", uri + "/");
			body = "";
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
bool ResponseBuilder::handleMultipartFormData(std::string &content_type, std::vector<byte> &req_body)
{
	size_t boundary_pos = content_type.find("boundary=");
	if (boundary_pos == std::string::npos)
	{
		LOG_ERROR("Boundary not found in content-type multipart/form-data.");
		return false;
	}

	std::string boundary = "--" + content_type.substr(boundary_pos + 9);
	std::string body(req_body.begin(), req_body.end());

	size_t pos = 0, next_pos;
	while ((next_pos = body.find(boundary, pos)) != std::string::npos)
	{
		pos = next_pos + boundary.length();

		if (pos < body.size() && body.substr(pos, 2) == "--")
			break; // End of multipart

		if (body.substr(pos, 2) == CRLF)
			pos += 2;

		size_t header_end = body.find(DOUBLE_CRLF, pos);
		if (header_end == std::string::npos)
		{
			LOG_ERROR("Malformed part: missing headers.");
			break;
		}

		std::string headers = body.substr(pos, header_end - pos);
		pos = header_end + 4; // Move past '\r\n\r\n'

		size_t part_end = body.find(boundary, pos);
		if (part_end == std::string::npos)
		{
			LOG_ERROR("Malformed part: missing boundary end.");
			break;
		}

		size_t content_length = part_end;
		if (part_end > 2 && body.substr(part_end - 2, 2) == CRLF)
		{
			content_length -= 2;
		}

		std::string content = body.substr(pos, part_end - pos);

		size_t filename_pos = headers.find("filename=\"");
		if (filename_pos == std::string::npos)
			continue;

		size_t filename_end = headers.find("\"", filename_pos + 10);
		if (filename_end == std::string::npos)
		{
			LOG_ERROR("Malformed filename in multipart.");
			continue;
		}

		std::string filename = headers.substr(filename_pos + 10, filename_end - (filename_pos + 10));

		if (filename.empty() || filename.find("..") != std::string::npos || filename.find('/') != std::string::npos)
		{
			LOG_ERROR("Invalid filename: " + filename);
			continue;
		}

		std::string safe_name = "upload_" + Utils::get_timestamp_str() + "_" + filename;
		std::string full_path = location_config->uploadStore + "/" + safe_name;

		if (!save_uploaded_file(full_path, std::vector<byte>(content.begin(), content.end())))
		{
			LOG_ERROR("Failed to save: " + full_path);
			continue;
		}

		LOG_INFO("File uploaded: " + full_path);
		set_headers("Location", full_path);
		set_status(201);
		set_body(generate_upload_success_page(safe_name));
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
void ResponseBuilder::handle_redirection()
{
	if (location_config && location_config->isRedirect)
	{
		if (location_config->isRedirectPermanent)
			set_status(301);
		else
			set_status(302);
		body = generate_error_page();
		this->headers["Location"] = location_config->redirectUrl;
	}
}

// Method to generate error pages
std::string ResponseBuilder::generate_error_page()
{
	if (server_config && server_config->errorPages.find(status_code) != server_config->errorPages.end())
	{
		std::string error_page_name = server_config->errorPages.at(status_code);
		std::string error_page_file = read_file(error_page_name);
		if (error_page_file != "")
			return error_page_file;
	}

	std::ostringstream page;
	page << "<html>\n<head>\n<title>" << status << "</title>\n</head>\n";
	page << "<body style=\"font-family:sans-serif;\">\n<center><h1>" << status << "</h1></center><hr />\n";
	page << "<center>" << WEBSERV_NAME << "</center>\n";
	page << "</body>\n</html>\n";

	headers["Content-Type"] = "text/html";

	return page.str();
}

// Method to generate default root page
std::string ResponseBuilder::generate_default_root()
{
	std::ostringstream page;
	page << "<html>\n<head>\n<title>Welcome to Webserv!</title>\n</head>\n";
	page << "<body style=\"font-family:sans-serif;\">\n<center><h1>Welcome to Webserv!</h1></center>\n";
	page << "<center><p style=\"font-size:20px;\">If you see this page, the web server is successfully compiled and working. Further configuration is required.<br />Thank you for testing our web server.</p></center>\n";
	page << "</body>\n</html>\n";

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
	page << "<html>\n<head>\n<title>Directory Listing</title>\n</head>\n";
	page << "<body>\n<h1 style=\"color:#4b4b4b;font-family:sans-serif;\">Directory Listing for " << path << "</h1><hr>\n";
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
				page << "<li style=\"letter-spacing:1.5;font-size:18px;\"><a href=\"" << entry_name << "/\">" << entry_name << "/</a>&emsp;&emsp;&emsp;" << get_http_date() << "&emsp;&emsp;&emsp;-</li><br>\n";
			}
			else
			{
				page << "<li style=\"letter-spacing:1.5;font-size:18px;\"><a href=\"" << entry_name << "\">" << entry_name << "</a>&emsp;&emsp;&emsp;" << get_http_date() << "&emsp;&emsp;&emsp;-</li><br>\n";
			}
		}
		closedir(dir);
	}
	else
	{
		LOG_ERROR("Failed to open directory: " + path);
		return "";
	}

	page << "</ul>\n</body>\n</html>";

	headers["Content-Type"] = "text/html";

	return page.str();
}

// Method to add the required headers into response
void ResponseBuilder::include_required_headers()
{
	// Include standard headers
	headers["Server"] = WEBSERV_NAME;
	headers["Date"] = get_http_date();

	// `Content-Type` header should always be present
	if (headers.find("Content-Type") == headers.end() && !request.is_cgi_request())
	{
		headers["Content-Type"] = detect_mime_type(request.get_request_uri());
	}

	// `Content-Length` and `Transfer-Encoding`
	if (headers.find("Transfer-Encoding") != headers.end())
	{
		if (headers["Transfer-Encoding"] == "chunked")
			headers.erase("Content-Length");
	}
	else
	{
		headers["Content-Length"] = Utils::to_string(body.size());
	}

	// Determine connection behavior
	if (!headers.count("Connection"))
	{
		headers["Connection"] = (request.is_connection_close()) ? "close" : "keep-alive";
	}

	// `Allow` header for 405 Method Not Allowed
	if (this->status_code == 405 && !headers.count("Allow") && location_config)
	{
		std::string allowed_methods;
		for (size_t i = 0; i < location_config->allowedMethods.size(); ++i)
		{
			allowed_methods += location_config->allowedMethods[i];
			if (i != location_config->allowedMethods.size() - 1)
				allowed_methods += ", ";
		}
		headers["Allow"] = allowed_methods;
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
std::string ResponseBuilder::read_file(const std::string &filename)
{
	std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
	if (!file)
	{
		LOG_ERROR("Error: Cannot open file: " + filename);
		return "";
	}

	std::ostringstream content;
	content << file.rdbuf();
	file.close();

	headers["Content-Type"] = detect_mime_type(filename);

	return content.str();
}

// // Method to generate upload success page
std::string ResponseBuilder::generate_upload_success_page(const std::string &filename)
{
	std::ostringstream page;
	page << "<html>\n"
		 << "<head>\n<title>Upload Successful</title>\n</head>\n"
		 << "<body>\n"
		 << "<h1 style=\"color:#4b4b4b;font-family:sans-serif;\">File Upload Successful</h1>\n"
		 << "<p>Your file has been uploaded successfully.</p>\n"
		 << "<p><strong>Saved as:</strong> " << filename << "</p>\n"
		 << "</body>\n"
		 << "</html>\n";

	headers["Content-Type"] = "text/html";

	return page.str();
}

/****************************
		START SETTERS
****************************/
void ResponseBuilder::set_status(short status_code)
{
	this->status_code = status_code;
	switch (status_code)
	{
	case 100:
		this->status = STATUS_100;
		break;
	case 101:
		this->status = STATUS_101;
		break;
	case 102:
		this->status = STATUS_102;
		break;
	case 200:
		this->status = STATUS_200;
		this->headers["Accept-Ranges"] = "bytes";
		break;
	case 201:
		this->status = STATUS_201;
		break;
	case 202:
		this->status = STATUS_202;
		break;
	case 203:
		this->status = STATUS_203;
		break;
	case 204:
		this->status = STATUS_204;
		break;
	case 205:
		this->status = STATUS_205;
		break;
	case 206:
		this->status = STATUS_206;
		break;
	case 207:
		this->status = STATUS_207;
		break;
	case 208:
		this->status = STATUS_208;
		break;
	case 226:
		this->status = STATUS_226;
		break;
	case 300:
		this->status = STATUS_300;
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
	case 305:
		this->status = STATUS_305;
		break;
	case 307:
		this->status = STATUS_307;
		break;
	case 308:
		this->status = STATUS_308;
		break;
	case 400:
		this->status = STATUS_400;
		break;
	case 401:
		this->status = STATUS_401;
		break;
	case 402:
		this->status = STATUS_402;
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
	case 406:
		this->status = STATUS_406;
		break;
	case 407:
		this->status = STATUS_407;
		break;
	case 408:
		this->status = STATUS_408;
		break;
	case 409:
		this->status = STATUS_409;
		break;
	case 410:
		this->status = STATUS_410;
		break;
	case 411:
		this->status = STATUS_411;
		break;
	case 412:
		this->status = STATUS_412;
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
	case 416:
		this->status = STATUS_416;
		break;
	case 417:
		this->status = STATUS_417;
		break;
	case 418:
		this->status = STATUS_418;
		break;
	case 421:
		this->status = STATUS_421;
		break;
	case 422:
		this->status = STATUS_422;
		break;
	case 423:
		this->status = STATUS_423;
		break;
	case 424:
		this->status = STATUS_424;
		break;
	case 426:
		this->status = STATUS_426;
		break;
	case 429:
		this->status = STATUS_429;
		break;
	case 431:
		this->status = STATUS_431;
		break;
	case 444:
		this->status = STATUS_444;
		break;
	case 451:
		this->status = STATUS_451;
		break;
	case 499:
		this->status = STATUS_499;
		break;
	case 500:
		this->status = STATUS_500;
		break;
	case 501:
		this->status = STATUS_501;
		break;
	case 502:
		this->status = STATUS_502;
		break;
	case 503:
		this->status = STATUS_503;
		break;
	case 504:
		this->status = STATUS_504;
		break;
	case 505:
		this->status = STATUS_505;
		break;
	case 506:
		this->status = STATUS_506;
		break;
	case 507:
		this->status = STATUS_507;
		break;
	case 508:
		this->status = STATUS_508;
		break;
	case 510:
		this->status = STATUS_510;
		break;
	case 511:
		this->status = STATUS_511;
		break;
	case 599:
		this->status = STATUS_599;
		break;
	default:
		this->status = UNDEFINED_STATUS;
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
RequestParser ResponseBuilder::getRequest() { return request; }
std::string ResponseBuilder::get_response() { return response; }
std::string ResponseBuilder::get_http_version() { return http_version; }
std::string ResponseBuilder::get_status() { return status; }
std::map<std::string, std::string> ResponseBuilder::get_headers() { return headers; }
std::string ResponseBuilder::get_header_value(std::string &key) { return headers[key]; }
std::string ResponseBuilder::get_body() { return body; }
short ResponseBuilder::get_status_code() { return status_code; }
const ServerConfig *ResponseBuilder::get_server_config() { return server_config; }
const Location *ResponseBuilder::get_location_config() { return location_config; }
std::map<std::string, void (ResponseBuilder::*)(void)> ResponseBuilder::get_routes() { return routes; }
/****************************
		END GETTERS
****************************/