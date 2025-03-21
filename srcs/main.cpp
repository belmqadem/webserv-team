#include "webserv.hpp"
#include "Server.hpp"

void printServerConfig(const ServerConfig &server)
{
	std::cout << "Server Configuration:" << std::endl;
	std::cout << "  Port: " << server.port << std::endl;
	std::cout << "  Host: " << server.host << std::endl;
	std::cout << "  Client Max Body Size: " << server.clientMaxBodySize
			  << " bytes (" << server.clientMaxBodySize / SIZE_MB << "MB)" << std::endl;

	// Print server names
	std::cout << "  Server Names: ";
	for (size_t i = 0; i < server.serverNames.size(); i++)
	{
		std::cout << server.serverNames[i];
		if (i < server.serverNames.size() - 1)
			std::cout << ", ";
	}
	std::cout << std::endl;

	// Print error pages
	std::cout << "  Error Pages:" << std::endl;
	for (std::map<short, std::string>::const_iterator it = server.errorPages.begin();
		 it != server.errorPages.end(); ++it)
	{
		std::cout << "    " << it->first << ": " << it->second << std::endl;
	}

	// Print locations
	std::cout << "  Locations:" << std::endl;
	for (size_t i = 0; i < server.locations.size(); i++)
	{
		const Location &loc = server.locations[i];
		std::cout << "  Location: " << loc.location << std::endl;
		std::cout << "    Allowed Methods: ";
		for (size_t j = 0; j < loc.allowedMethods.size(); j++)
		{
			std::cout << loc.allowedMethods[j];
			if (j < loc.allowedMethods.size() - 1)
				std::cout << ", ";
		}
		std::cout << std::endl;
		if (!loc.root.empty())
			std::cout << "    Root: " << loc.root << std::endl;
		std::cout << "    Index: " << loc.index << std::endl;
		std::cout << "    Autoindex: " << (loc.autoindex ? "on" : "off") << std::endl;

		if (loc.isRedirect)
		{
			std::cout << "    Redirect: " << loc.redirectUrl << std::endl;
			std::cout << "    Permanent: " << (loc.isRedirectPermanent ? "yes" : "no") << std::endl;
			std::cout << "    Redirect code: " << loc.redirectCode << std::endl;
		}

		if (loc.useCgi)
		{
			std::cout << "    CGI Enabled: yes" << std::endl;
			std::cout << "    CGI Path: " << loc.cgiPath << std::endl;
			std::cout << "    CGI Extensions:" << std::endl;
			for (std::map<std::string, std::string>::const_iterator it = loc.cgiExtensions.begin();
				 it != loc.cgiExtensions.end(); ++it)
			{
				std::cout << "        " << it->first << ": " << it->second << std::endl;
			}
		}

		if (!loc.uploadStore.empty())
		{
			std::cout << "    Upload Store: " << loc.uploadStore << std::endl;
		}
	}
}

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
		std::vector<ServerConfig> servers = ConfigManager::getInstance()->getServers();
		// for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); ++it)
		// 	printServerConfig(*it);

		Server &server = Server::getInstance(ConfigManager::getInstance()->getServers());
		server.StartServer();
		LOG_INFO("Our Webserver *Not Nginx* Starting...");
		IOMultiplexer::getInstance().runEventLoop();
	}
	catch (std::exception &e)
	{
		std::cerr << RED "Fatal error: \n"
				  << e.what() << RESET << "\n";
	}
}
