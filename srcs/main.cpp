#include "../includes/webserv.hpp"

int main()
{
	const std::string request = "POST / HTTP/1.1\r\n"
						  "Host: webserv.42\r\n"
						  "Cookie: session=abc123\r\n"
						  "Content-Length: 6\r\n"
						  "\r\n"
						  "hello world";

	RequestParser parser(request);
	parser.print_request();
}
