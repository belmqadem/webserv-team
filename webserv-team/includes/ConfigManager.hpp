#pragma once

#include <string>
#include <vector>

class ConfigManager {
    private:
        std::string _configFilePath;
        std::vector<std::string> _serverConfigs;

    public:
        ConfigManager(const std::string &configFilePath);
        void loadConfigurations();
        const std::vector<std::string>& getServerConfigs() const;
        void parseConfigFile();
};