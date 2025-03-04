#include "../../includes/ResponseBuilder.hpp"

// Default Constructor
ResponseBuilder::ResponseBuilder()
{
	this->http_version = "HTTP/1.1"; // Only version 1.1 is implemented
	this->status = STATUS_200;
	this->headers = std::map<std::string, std::string>();
	this->body = "";
	this->status_code = 200; // Default value is success
}

// Method to build the response message
std::string ResponseBuilder::build_response(RequestParser &request)
{
	std::string method = request.get_http_method();
	if (method == "GET")
		doGET(request);
	else if (method == "POST")
		doPOST(request);
	else if (method == "DELETE")
		doDELETE(request);

	// Handle Redirections
	handle_redirection();

	// Construt Response string
	std::ostringstream response;
	response << http_version << SP << status << CRLF;
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
		response << it->first << ": " << it->second << CRLF;
	response << "Content-Length: " << body.length() << CRLF;
	response << "Connection: close" << CRLF_DOUBLE;
	response << body;
	return response.str();
}

void ResponseBuilder::doGET(RequestParser &request)
{
	(void)request;
}

void ResponseBuilder::doPOST(RequestParser &request)
{
	(void)request;
}

void ResponseBuilder::doDELETE(RequestParser &request)
{
	(void)request;
}

// Method to handle redirection
void ResponseBuilder::handle_redirection()
{
}

/****************************
		START SETTERS
****************************/
void ResponseBuilder::set_http_version(const std::string &http_version) { this->http_version = http_version; }
void ResponseBuilder::set_status(short status_code, const std::string &status)
{
	this->status_code = status_code;
	this->status = status;
}
void ResponseBuilder::set_headers(const std::string &key, const std::string &value) { this->headers[key] = value; }
void ResponseBuilder::set_body(const std::string &body) { this->body = body; }
/****************************
		END SETTERS
****************************/

/****************************
		START GETTERS
****************************/
std::string ResponseBuilder::get_http_version() { return http_version; }
std::string ResponseBuilder::get_status() { return status; }
std::map<std::string, std::string> ResponseBuilder::get_headers() { return headers; }
std::string ResponseBuilder::get_header(std::string &key) { return headers[key]; }
std::string ResponseBuilder::get_body() { return body; }
short ResponseBuilder::get_status_code() { return status_code; }
/****************************
		END GETTERS
****************************/
