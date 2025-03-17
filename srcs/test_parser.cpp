#include "webserv.hpp"
#include "ConfigManager.hpp"
#include <iostream>

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
	for (std::map<int, std::string>::const_iterator it = server.errorPages.begin();
		 it != server.errorPages.end(); ++it)
	{
		std::cout << "    " << it->first << ": " << it->second << std::endl;
	}

	// Print locations
	std::cout << "  Locations:" << std::endl;
	for (size_t i = 0; i < server.locations.size(); i++)
	{
		const Location &loc = server.locations[i];
		std::cout << "    Location: " << loc.location << std::endl;
		if (!loc.root.empty())
			std::cout << "      Root: " << loc.root << std::endl;
		std::cout << "      Autoindex: " << (loc.autoindex ? "on" : "off") << std::endl;
		if (!loc.index.empty())
			std::cout << "      Index: " << loc.index << std::endl;

		if (loc.isRedirect)
		{
			std::cout << "      Redirect: " << loc.redirectUrl << std::endl;
			std::cout << "      Permanent: " << (loc.isRedirectPermanent ? "yes" : "no") << std::endl;
		}

		if (loc.useCgi)
		{
			std::cout << "      CGI Enabled: yes" << std::endl;
			std::cout << "      CGI Path: " << loc.cgiPath << std::endl;
			std::cout << "      CGI Extensions:" << std::endl;
			for (std::map<std::string, std::string>::const_iterator it = loc.cgiExtensions.begin();
				 it != loc.cgiExtensions.end(); ++it)
			{
				std::cout << "        " << it->first << ": " << it->second << std::endl;
			}
		}

		if (!loc.uploadStore.empty())
		{
			std::cout << "      Upload Store: " << loc.uploadStore << std::endl;
		}

		if (!loc.allowedMethods.empty())
		{
			std::cout << "      Allowed Methods: ";
			for (size_t j = 0; j < loc.allowedMethods.size(); j++)
			{
				std::cout << loc.allowedMethods[j];
				if (j < loc.allowedMethods.size() - 1)
					std::cout << ", ";
			}
			std::cout << std::endl;
		}
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return 1;
	}

	try
	{
		// Load configuration
		ConfigManager::getInstance()->loadConfig(argv[1]);

		// Get and print servers
		const std::vector<ServerConfig> &servers = ConfigManager::getInstance()->getServers();
		std::cout << "Loaded " << servers.size() << " server(s)" << std::endl;

		for (size_t i = 0; i < servers.size(); i++)
		{
			std::cout << "Server #" << i + 1 << std::endl;
			printServerConfig(servers[i]);
			std::cout << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}