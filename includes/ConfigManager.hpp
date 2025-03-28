#pragma once

#include "Parser.hpp"

// Size constants
#define SIZE_KB (1024UL)
#define SIZE_MB (1024UL * SIZE_KB)
#define SIZE_GB (1024UL * SIZE_MB)

// Default size limits
#define DEFAULT_CLIENT_MAX_BODY_SIZE (1 * SIZE_MB) // 1MB

struct Location
{
	std::string location;
	std::vector<std::string> allowedMethods;
	std::string root;
	bool autoindex;
	std::string index;

	// redirection parameters
	bool isRedirect;
	std::string redirectUrl;
	bool isRedirectPermanent;
	int redirectCode;

	// CGI parameters
	bool useCgi;
	std::string cgiPath;
	std::map<std::string, std::string> cgiExtensions;
	std::string cgiWorkingDirectory;

	// file upload parameters
	std::string uploadStore;

	Location() : autoindex(false), index("index.html"), isRedirect(false),
				 isRedirectPermanent(false), useCgi(false)

	{
		allowedMethods.push_back("GET");
		allowedMethods.push_back("POST");
		allowedMethods.push_back("DELETE");
	}
};

struct ServerConfig
{
	uint16_t port;
	std::string host;
	std::vector<std::string> serverNames;
	bool defaultServer;
	std::map<short, std::string> errorPages;
	size_t clientMaxBodySize;
	std::vector<Location> locations;

	ServerConfig() : port(80), host("0.0.0.0"), defaultServer(false), clientMaxBodySize(DEFAULT_CLIENT_MAX_BODY_SIZE)
	{
		serverNames.push_back("localhost");
	}
};

class ConfigManager
{
private:
	static ConfigManager *_instance;

	std::vector<ServerConfig> _servers; // List of servers instances
	bool checkOpen;

	ConfigManager();
	ConfigManager(const ConfigManager &);
	ConfigManager &operator=(const ConfigManager &);

public:
	// Get singleton instance
	static ConfigManager &getInstance();

	// Destroy singleton instance (for cleanup)
	static void destroyInstance();

	// Load configuration from file
	bool loadConfig(const std::string &configFile);

	// Getters for configuration data
	const std::vector<ServerConfig> &getServers() const;
	const ServerConfig *getServerByPort(uint16_t port) const;
	const ServerConfig *getServerByName(const std::string &name) const;

	// Helper methods for parsing
	void setServers(const std::vector<ServerConfig> &servers);
	bool check_open();
};
