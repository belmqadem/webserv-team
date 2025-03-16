#pragma once

#include <vector>
#include <string>
#include <map>
#include "Parser.hpp"
#include <string>
#include <fstream>


// Size constants
#define SIZE_KB (1024UL)
#define SIZE_MB (1024UL * SIZE_KB)
#define SIZE_GB (1024UL * SIZE_MB)

// Default size limits
#define DEFAULT_CLIENT_MAX_BODY_SIZE (1 * SIZE_MB)  // 1MB
#define DEFAULT_MAX_UPLOAD_SIZE (10 * SIZE_MB)     // 10MB

struct Location
{
    std::string location;
    std::string root;
    std::map<int, std::string> errorPages;
    std::vector<std::string> allowedMethods;
    bool autoindex;
    std::string index;
    
    // redirection parameters
    bool redirect;
    std::string redirectUrl;
    bool redirectPermanent;
    
    // CGI parameters
    bool useCgi;
    std::string cgiPath;
    std::map<std::string, std::string> cgiExtensions;
    
    // file upload parameters
    std::string uploadStore;
    size_t maxUploadSize;

    Location() : autoindex(false), redirect(false), 
                 redirectPermanent(false), useCgi(false),
                 maxUploadSize(DEFAULT_MAX_UPLOAD_SIZE) {}
};

struct ServerConfig
{
	/* TO ADD
		client_max_body_size
	*/
	uint16_t port;
	std::string host;
	std::vector<std::string> serverNames;
	std::string root;
	std::map<int, std::string> errorPages;
	std::vector<Location> locations;
	// client max body size parameter (in bytes)
    size_t clientMaxBodySize;

	ServerConfig() : port(80), host("0.0.0.0"), clientMaxBodySize(DEFAULT_CLIENT_MAX_BODY_SIZE) {}
};

class ConfigManager
{
private:
	static ConfigManager *_instance;

	std::vector<ServerConfig> _servers; // List of servers instances 

	ConfigManager();

	ConfigManager(const ConfigManager &);
	ConfigManager &operator=(const ConfigManager &);

public:
	// Get singleton instance
	static ConfigManager *getInstance();

	// Destroy singleton instance (for cleanup)
	static void destroyInstance();

	// Load configuration from file
	bool loadConfig(const std::string &configFile);

	// Getters for configuration data
	const std::vector<ServerConfig> &getServers() const;
	const ServerConfig *getServerByPort(int port) const;
	const ServerConfig *getServerByName(const std::string &name) const;

	// Helper methods for parsingz
	void setServers(const std::vector<ServerConfig> &servers);
};
