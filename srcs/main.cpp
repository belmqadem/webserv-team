#include "webserv.hpp"
#include "ConfigManager.hpp"
#include "IOMultiplexer.hpp"
#include "Server.hpp"
#include "Logger.hpp"
#include <signal.h>

extern int webserv_signal;

void	sigint_handle(int sig) {
	webserv_signal = sig;
	std::cout << "Interrupt signal catched." << std::endl;
}

void signalhandler() {
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR)
		throw std::runtime_error("signal() faild.");
	if (signal(SIGINT, &sigint_handle) == SIG_ERR)
		throw std::runtime_error("signal() faild.");
}


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
