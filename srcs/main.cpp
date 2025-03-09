#include "../includes/webserv.hpp"

int main()
{
	const std::string request = "GET / HTTP/1.1\r\n"
								"Host: localhost\r\n"
								"\r\n";

	RequestParser parser(request);
	parser.print_request();

	ResponseBuilder response(parser);
	std::cout << "\n--------------------------------" << std::endl;
	std::cout << YELLOW "** HTTP RESPONSE **" RESET << std::endl;
	std::cout << "--------------------------------" << std::endl;
	std::cout << response.get_response() << std::endl;
}
