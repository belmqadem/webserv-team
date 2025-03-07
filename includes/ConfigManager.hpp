#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <vector>
#include <string>
#include <map>
#include "Parser.hpp"
#include <string>
#include <fstream>

struct ServerConfig;

class ConfigManager {
private:
    static ConfigManager* _instance;
    
    std::vector<ServerConfig> _servers;
    
    ConfigManager();
    
    ConfigManager(const ConfigManager&);
    ConfigManager& operator=(const ConfigManager&);
    
public:
    // Get singleton instance
    static ConfigManager* getInstance();
    
    // Destroy singleton instance (for cleanup)
    static void destroyInstance();
    
    // Load configuration from file
    bool loadConfig(const std::string& configFile);
    
    // Getters for configuration data
    const std::vector<ServerConfig>& getServers() const;
    const ServerConfig* getServerByPort(int port) const;
    const ServerConfig* getServerByName(const std::string& name) const;
    
    // Helper methods for parsing
    void setServers(const std::vector<ServerConfig>& servers);
};

#endif
