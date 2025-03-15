#pragma once

#include "Parser.hpp"

struct ServerConfig;

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
