#include "webserv.hpp"
#include "Server.hpp"

std::vector<std::string> split(const char *start, const char *end, char delimiter)
{
	std::vector<std::string> result;
	const char *current = start;
	while (current < end)
	{
		const char *next = std::find(current, end, delimiter);
		result.push_back(std::string(current, next));
		if (next == end)
			break;
		current = next + 1;
	}
	return result;
}

std::string trim(const std::string &str, const std::string &delim)
{
	size_t start = str.find_first_not_of(delim);
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(delim);
	return str.substr(start, end - start + 1);
}

std::string readFile(const std::string &filename)
{
	std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
	if (!file)
		return "";
	std::ostringstream content;
	content << file.rdbuf();
	return content.str();
}

bool writeFile(const std::string &filename, const std::string &content)
{
	std::ofstream file(filename.c_str());
	if (!file)
		return false;
	file << content;
	return true;
}

bool is_numeric(const std::string &str)
{
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (!std::isdigit(static_cast<unsigned char>(str[i])))
			return false;
	}
	return true;
}

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
