#include "webserv.hpp"
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
	if (ac > 2)
	{
		std::cout << BOLD_BLUE << USAGE(av[0]) << RESET << "\n";
		return (0);
	}
	Logger::getInstance().setLevel(DEBUG);
	Logger::getInstance().setOutput(true, true);
	Logger::getInstance().setLogFile(LOG_FILE);
	signalhandler();
	try
	{
		(ac == 2) ? ConfigManager::getInstance().loadConfig(av[1]) : ConfigManager::getInstance().loadConfig(DEFAULT_CONF);
		std::vector<ServerConfig> virtual_servers = ConfigManager::getInstance().getServers();

		// for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); ++it)
		// 	printServerConfig(*it);

		Server &server = Server::getInstance(virtual_servers);
		server.StartServer();
		LOG_INFO("Our Webserver *Not Nginx* Starting...");
		IOMultiplexer::getInstance().runEventLoop();
	}
	catch (std::exception &e)
	{
		LOG_ERROR("Fatal error -- " + std::string(e.what()));
	}
}

// int main()
// {
// 	Logger::getInstance().setLevel(DEBUG);
// 	Logger::getInstance().setOutput(true, true);
// 	Logger::getInstance().setLogFile("WebServe.log");

// 	try
// 	{
// 		ConfigManager::getInstance()->loadConfig("config/webserv.conf");
// 		std::vector<ServerConfig> servers = ConfigManager::getInstance()->getServers();

// 		std::string request = "POST /submit HTTP/1.1\r\n"
// 							  "Host: localhost:5050\r\n"
// 							  "Content-Type: application/octet-stream\r\n"
// 							  "Content-Length: 10\r\n"
// 							  "\r\n"
// 							  "0001101010101010100101010101010101010101010101010101010101010101010101010";

// 		std::cout << CYAN "** START REQUEST PARSING **" RESET << std::endl;
// 		RequestParser parser(request, servers);
// 		parser.print_request();
// 		std::cout << CYAN "** REQUEST PARSING DONE **" RESET << std::endl;

// 		ResponseBuilder response(parser);
// 		std::cout << CYAN "** START RESPONSE GENERATING **" RESET << std::endl;
// 		std::cout << response.get_response() << std::endl;
// 		std::cout << CYAN "** RESPONSE GENERATING DONE**" RESET << std::endl;
// 	}
// 	catch (std::exception &e)
// 	{
// 		std::cerr << RED "Fatal error: \n"
// 				  << e.what() << RESET << "\n";
// 	}
// }