// filepath: /home/nmellal/Projects/webserv-team/includes/Logger.hpp
#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

class Logger {
public:
    static void logInfo(const std::string &message) {
        log("INFO", message);
    }

    static void logError(const std::string &message) {
        log("ERROR", message);
    }

    static void logWarning(const std::string &message) {
        log("WARNING", message);
    }

private:
    static void log(const std::string &level, const std::string &message) {
        std::ofstream logFile("server.log", std::ios_base::app);
        if (logFile.is_open()) {
            std::time_t now = std::time(nullptr);
            logFile << std::ctime(&now) << " [" << level << "] " << message << std::endl;
            logFile.close();
        } else {
            std::cerr << "Unable to open log file!" << std::endl;
        }
    }
};