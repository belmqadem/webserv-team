#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR2,
    FATAL
};

class Logger {
private:
    static Logger* _instance;
    std::ofstream _logFile;
    LogLevel _level;
    bool _toConsole;
    bool _toFile;
    
    Logger();
    
    std::string getTimestamp();
    std::string getLevelString(LogLevel level);

public:
    ~Logger();
    
    static Logger& getInstance();
    static void cleanup();

    void setLevel(LogLevel level);
    void setOutput(bool toConsole, bool toFile);
    void setLogFile(const std::string& path);
    
    void log(LogLevel level, const std::string& message);
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);
};

// Convenience macros
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_FATAL(msg) Logger::getInstance().fatal(msg)