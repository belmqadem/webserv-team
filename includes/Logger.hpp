#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

enum LogLevel
{
	DEBUG,
	INFO,
	SERVERS,
	CLIENT,
	REQUEST,
	RESPONSE,
	WARNING,
	ERROR,
	FATAL
};

class Logger
{
private:
	static Logger *_instance;
	std::ofstream _logFile;
	LogLevel _level;
	bool _toConsole;
	bool _toFile;

	Logger();

	std::string getTimestamp();
	std::string getLevelString(LogLevel level);

public:
	~Logger();

	static Logger &getInstance();

	void setLevel(LogLevel level);
	void setOutput(bool toConsole, bool toFile);
	void setLogFile(const std::string &path);

	void log(LogLevel level, const std::string &message);

	void debug(const std::string &message);
	void info(const std::string &message);
	void server(const std::string &message);
	void client(const std::string &message);
	void request(const std::string &message);
	void response(const std::string &message);
	void warning(const std::string &message);
	void error(const std::string &message);
	void fatal(const std::string &message);
};

// Convenience macros
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_SERVER(msg) Logger::getInstance().server(msg)
#define LOG_CLIENT(msg) Logger::getInstance().client(msg)
#define LOG_REQUEST(msg) Logger::getInstance().request(msg)
#define LOG_RESPONSE(msg) Logger::getInstance().response(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_FATAL(msg) Logger::getInstance().fatal(msg)
