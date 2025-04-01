#include "webserv.hpp"

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
	static Logger inst;
	return inst;
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
	case SERVERS:
		return "SERVER";
	case CLIENT:
		return "CLIENT";
	case REQUEST:
		return "REQUEST";
	case RESPONSE:
		return "RESPONSE";
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
		LOG_ERROR("Failed to open log file: " + path);
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
		if (level == ERROR)
			std::cerr << BOLD_RED << logMessage << RESET << std::endl;
		else if (level == SERVERS)
			std::cout << BOLD_WHITE UNDERLINE << logMessage << RESET << std::endl;
		else if (level == CLIENT)
			std::cout << BOLD_YELLOW << logMessage << RESET << std::endl;
		else if (level == REQUEST)
			std::cout << BLUE << logMessage << RESET << std::endl;
		else if (level == RESPONSE)
			std::cout << BOLD_BLUE << logMessage << RESET << std::endl;
		else if (level == INFO)
			std::cout << MAGENTA << logMessage << RESET << std::endl;
		else
			std::cout << logMessage << std::endl;
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

void Logger::server(const std::string &message)
{
	log(SERVERS, message);
}

void Logger::client(const std::string &message)
{
	log(CLIENT, message);
}

void Logger::request(const std::string &message)
{
	log(REQUEST, message);
}

void Logger::response(const std::string &message)
{
	log(RESPONSE, message);
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