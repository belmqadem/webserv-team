#include "webserv.hpp"
#include "Server.hpp"

extern int webserv_signal;

void sigint_handle(int sig)
{
	webserv_signal = sig;
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
