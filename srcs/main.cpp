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
	// if (ac != 2) {
	// 	std::cout << USAGE(av[0]) << "\n";
	// 	return (1);
	// }

	// Logger::getInstance().setLevel(DEBUG);
	// Logger::getInstance().setOutput(true, true);
	// Logger::getInstance().setLogFile("WebServe.log");
	// try {
	// 	ConfigManager::getInstance()->loadConfig(av[1]);
	// 	LOG_INFO("Webserver Starting...");
	// 	Server &server = Server::getInstance(ConfigManager::getInstance()->getServers());
	// 	IOMultiplexer::getInstance().runEventLoop();
	// } catch (std::exception &e) {
	// 	std::cerr << "Fatal error: \n" << e.what() << "\n";
	// }
	// return (0);
	const std::string request = "POST / HTTP/1.1\r\n"
						  "Host: webserv.42\r\n"
						  "Cookie: session=abc123\r\n"
						  "Content-Length: 6\r\n"
						  "\r\n"
						  "hello world";

	RequestParser parser(request);
	parser.print_request();
}
