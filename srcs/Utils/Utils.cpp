#include "Server.hpp"

namespace Utils
{
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

	bool is_numeric(const std::string &str)
	{
		for (size_t i = 0; i < str.size(); ++i)
		{
			if (!std::isdigit(static_cast<unsigned char>(str[i])))
				return false;
		}
		return true;
	}

	std::string get_timestamp_str()
	{
		std::time_t now = std::time(NULL);
		std::tm *tm_info = std::localtime(&now);

		std::ostringstream oss;
		oss << std::setfill('0')
			<< (tm_info->tm_year + 1900)
			<< std::setw(2) << (tm_info->tm_mon + 1)
			<< std::setw(2) << tm_info->tm_mday
			<< "_"
			<< std::setw(2) << tm_info->tm_hour
			<< std::setw(2) << tm_info->tm_min
			<< std::setw(2) << tm_info->tm_sec;

		return oss.str();
	}

	bool string_to_size_t(const std::string &str, int &result)
	{
		std::stringstream ss(str);

		ss >> result;

		return !ss.fail() && ss.eof();
	}

	void sigint_handle(int sig)
	{
		(void)sig;
		LOG_INFO("SIG_INT The Server will shut down");

		// Log active resources before shutdown
		LOG_INFO("Active connections: " + Utils::to_string(IOMultiplexer::getInstance().getListenersCount()));

		// Then shut down
		IOMultiplexer::getInstance().setStarted(false);
		return;
	}

	void signalhandler()
	{
		if (signal(SIGQUIT, SIG_IGN) == SIG_ERR)
			throw std::runtime_error(RED "signal() faild." RESET);
		if (signal(SIGINT, &sigint_handle) == SIG_ERR)
			throw std::runtime_error(RED "signal() faild." RESET);
		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error(RED "signal() failed for SIGPIPE." RESET);
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
			else
			{
				std::cout << "    CGI Enabled: no" << std::endl;
			}

			if (!loc.uploadStore.empty())
			{
				std::cout << "    Upload Store: " << loc.uploadStore << std::endl;
			}
		}
	}
}