#include "Logger.hpp"

Logger *Logger::_instance = NULL;

Logger::Logger() : _level(INFO), _toConsole(true), _toFile(false) {}

Logger::~Logger()
{
	if (_logFile.is_open())
	{
		_logFile.close();
	}
}

Logger &Logger::getInstance()
{
	if (_instance == NULL)
	{
		_instance = new Logger();
	}
	return *_instance;
}

void Logger::cleanup()
{
	if (_instance != NULL)
	{
		delete _instance;
		_instance = NULL;
	}
}

std::string Logger::getTimestamp()
{
	std::time_t now = std::time(NULL);
	std::tm *tm = std::localtime(&now);

	std::ostringstream oss;
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
	oss << buffer;
	return oss.str();
}

std::string Logger::getLevelString(LogLevel level)
{
	switch (level)
	{
	case DEBUG:
		return "DEBUG";
	case INFO:
		return "INFO";
	case WARNING:
		return "WARNING";
	case ERROR:
		return "ERROR";
	case FATAL:
		return "FATAL";
	default:
		return "UNKNOWN";
	}
}

void Logger::setLevel(LogLevel level)
{
	_level = level;
}

void Logger::setOutput(bool toConsole, bool toFile)
{
	_toConsole = toConsole;
	_toFile = toFile;
}

void Logger::setLogFile(const std::string &path)
{
	if (_logFile.is_open())
	{
		_logFile.close();
	}

	_logFile.open(path.c_str(), std::ios::out | std::ios::app);
	if (!_logFile.is_open())
	{
		std::cerr << "Failed to open log file: " << path << std::endl;
		_toFile = false;
	}
	else
	{
		_toFile = true;
	}
}

void Logger::log(LogLevel level, const std::string &message)
{
	if (level < _level)
	{
		return;
	}

	std::string timestamp = getTimestamp();
	std::string levelStr = getLevelString(level);
	std::string logMessage = "[" + timestamp + "] [" + levelStr + "] " + message;

	if (_toConsole)
	{
		if (level >= ERROR)
		{
			std::cerr << logMessage << std::endl;
		}
		else
		{
			std::cout << logMessage << std::endl;
		}
	}

	if (_toFile && _logFile.is_open())
	{
		_logFile << logMessage << std::endl;
	}
}

void Logger::debug(const std::string &message)
{
	log(DEBUG, message);
}

void Logger::info(const std::string &message)
{
	log(INFO, message);
}

void Logger::warning(const std::string &message)
{
	log(WARNING, message);
}

void Logger::error(const std::string &message)
{
	log(ERROR, message);
}

void Logger::fatal(const std::string &message)
{
	log(FATAL, message);
}