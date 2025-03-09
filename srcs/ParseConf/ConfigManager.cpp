#include "ConfigManager.hpp"
#include "Tokenize.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

// Initialize static instance to NULL
ConfigManager* ConfigManager::_instance = NULL;

ConfigManager::ConfigManager() {
    // Initialize any needed properties
}

ConfigManager* ConfigManager::getInstance() {
    if (_instance == NULL) {
        _instance = new ConfigManager();
    }
    return _instance;
}

void ConfigManager::destroyInstance() {
    if (_instance != NULL) {
        delete _instance;
        _instance = NULL;
    }
}

bool ConfigManager::loadConfig(const std::string& configFile) {
    std::string config;
    std::ifstream file(configFile.c_str());
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open config file: " << configFile << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    config = buffer.str();
    file.close();
    
    try {
        std::vector<Token> tokens = tokenize(config);
        Parser parser(tokens);
        parser.parseConfig();
        
        // Store the parsed configuration
        _servers = parser.getServers();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config: " << e.what() << std::endl;
        return false;
    }
}

const std::vector<ServerConfig>& ConfigManager::getServers() const {
    return _servers;
}

const ServerConfig* ConfigManager::getServerByPort(int port) const {
    for (size_t i = 0; i < _servers.size(); i++) {
        if (_servers[i].port == port) {
            return &_servers[i];
        }
    }
    return NULL;
}

const ServerConfig* ConfigManager::getServerByName(const std::string& name) const {
    for (size_t i = 0; i < _servers.size(); i++) {
        for (size_t j = 0; j < _servers[i].serverNames.size(); j++) {
            if (_servers[i].serverNames[j] == name) {
                return &_servers[i];
            }
        }
    }
    return NULL;
}

void ConfigManager::setServers(const std::vector<ServerConfig>& servers) {
    _servers = servers;
}
