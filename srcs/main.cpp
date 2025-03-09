#include "../includes/webserv.hpp"

int main()
{
	const std::string request = "POST /style.css HTTP/1.1\r\n"
						  "Cookie: session=abc123\r\n"
						  "Host: localhost\r\n"
						  "Transfer-Encoding: chunked\r\n"
						  "\r\n"
						  "5\r\nhello\r\n0\r\n\r\n";

	RequestParser parser(request);
	parser.print_request();

	ResponseBuilder response(parser);
	std::cout << "\n--------------------------------" << std::endl;
	std::cout << YELLOW "** HTTP RESPONSE **" RESET << std::endl;
	std::cout << "--------------------------------" << std::endl;
	std::cout << response.get_response() << std::endl;
}
