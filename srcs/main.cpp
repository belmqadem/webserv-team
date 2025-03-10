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

int main(int ac, char**av)
{
	(void)ac;
	const std::string request = "GET /a.php?name=hamid HTTP/1.1\r\n"
								"Host: localhost\r\n"
								"\r\n";
	RequestParser parser(request);
	
	// parser.print_request();
	// ResponseBuilder response(parser);
	// // response.doGET(parser);
	// std::cout << "\n--------------------------------" << std::endl;
	// std::cout << YELLOW "** HTTP RESPONSE **" RESET << std::endl;
	// std::cout << "--------------------------------" << std::endl;
	// std::cout << response.get_response() << std::endl;
	Logger::getInstance().setLevel(DEBUG);
	Logger::getInstance().setOutput(true, true);
	Logger::getInstance().setLogFile("WebServe.log");
	try {
		ConfigManager::getInstance()->loadConfig(av[1]);
		LOG_INFO("Webserver Starting...");
		Server &server = Server::getInstance(ConfigManager::getInstance()->getServers());
		server.StartServer();
		ResponseBuilder response(parser);
		response.doGET(parser);
		IOMultiplexer::getInstance().runEventLoop();
	} catch (std::exception &e) {
		std::cerr << "Fatal error: \n" << e.what() << "\n";
	}
	return (0);
}
