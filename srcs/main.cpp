#include "webserv.hpp"
#include "Server.hpp"

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
	Utils::signalhandler();
	try
	{
		(ac == 2) ? ConfigManager::getInstance().loadConfig(av[1]) : ConfigManager::getInstance().loadConfig(DEFAULT_CONF);
		if (!ConfigManager::getInstance().check_open())
			return (1);
		std::vector<ServerConfig> virtual_servers = ConfigManager::getInstance().getServers();

		// for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); ++it)
		// 	Utils::printServerConfig(*it);

		Server &server = Server::getInstance(virtual_servers);
		server.StartServer();
		IOMultiplexer::getInstance().runEventLoop();
	}
	catch (std::exception &e)
	{
		LOG_ERROR("Fatal error -- " + std::string(e.what()));
	}
	return (0);
}
