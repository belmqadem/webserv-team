#include "../includes/webserv.hpp"

int main()
{
	std::string request = "POST /page HTTP/1.1\r\n"
						  "Host: webserv.42\r\n"
						  "Cookie: session=abc123\r\n"
						  "Cookie: theme=dark\r\n"
						  "Transfer-Encoding: chunked\r\n"
						  "\r\n"
						  "5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n";
	RequestParser parser(request);
	parser.print_request();

	std::cout << YELLOW "\n** WEBSERV ** ** HTTP RESPONSE **" RESET << std::endl;
	ResponseBuilder response;
	std::cout << response.build_response(parser);
}

// void handle_request(RequestParser &request)
// {
// 	Router router;
// 	ResponseBuilder response;

// 	std::string handler_type = router.get_handler(request);

// 	if (handler_type == "REDIRECT")
// 	{
// 		response.set_status(302, STATUS_302);
// 		response.set_headers("Location", router.resolve_route(request.get_request_uri()));
// 	}
// 	else if (handler_type == "STATIC")
// 	{
// 		std::string file_path = router.resolve_route(request.get_request_uri());
// 		std::ifstream file(file_path.c_str());

// 		if (!file.is_open())
// 		{
// 			response.set_status(404, STATUS_400);
// 			response.set_body("404 Not Found");
// 		}
// 		else
// 		{
// 			std::stringstream buffer;
// 			buffer << file.rdbuf();
// 			response.set_body(buffer.str());
// 			response.set_headers("Content-Type", "text/html");
// 		}
// 	}
// 	else if (handler_type == "CGI")
// 	{
// 		response.set_status(200, STATUS_200);
// 		response.set_body("CGI execution not implemented yet.");
// 	}
// 	else
// 	{
// 		response.set_status(404, "Not Found");
// 		response.set_body("404 Not Found");
// 	}

// 	std::cout << response.build_response();
// }