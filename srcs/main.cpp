#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"
#include "Logger.hpp"
#include "Parser.hpp"
#include "Server.hpp"

extern int webserv_signal;

void sigint_handle(int sig)
{
	webserv_signal = sig;
	std::cout << "Interrupt signal catched." << std::endl;
}

void signalhandler()
{
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR)
		throw std::runtime_error(RED "signal() faild." RESET);
	if (signal(SIGINT, &sigint_handle) == SIG_ERR)
		throw std::runtime_error(RED "signal() faild." RESET);
}

int main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cout << BOLD_BLUE << USAGE(av[0]) << RESET << "\n";
		return (0);
	}
	Logger::getInstance().setLevel(DEBUG);
	Logger::getInstance().setOutput(true, true);
	Logger::getInstance().setLogFile("WebServe.log");
	signalhandler();
	try
	{
		ConfigManager::getInstance()->loadConfig(av[1]);
		LOG_INFO("Our Webserver *Not Nginx* Starting...");
		Server &server = Server::getInstance(ConfigManager::getInstance()->getServers());
		server.StartServer();
		IOMultiplexer::getInstance().runEventLoop();
	}
	catch (std::exception &e)
	{
		std::cerr << RED "Fatal error: \n"
				  << e.what() << RESET << "\n";
	}
}
