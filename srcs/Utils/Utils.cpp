#include "Server.hpp"

namespace Utils
{
	std::vector<std::string> split(const char *start, const char *end, char delimiter)
	{
		std::vector<std::string> result;
		const char *current = start;
		while (current < end)
		{
			const char *next = std::find(current, end, delimiter);
			result.push_back(std::string(current, next));
			if (next == end)
				break;
			current = next + 1;
		}
		return result;
	}

	std::string trim(const std::string &str, const std::string &delim)
	{
		size_t start = str.find_first_not_of(delim);
		if (start == std::string::npos)
			return "";
		size_t end = str.find_last_not_of(delim);
		return str.substr(start, end - start + 1);
	}

	bool is_numeric(const std::string &str)
	{
		for (size_t i = 0; i < str.size(); ++i)
		{
			if (!std::isdigit(static_cast<unsigned char>(str[i])))
				return false;
		}
		return true;
	}

	std::string get_timestamp_str()
	{
		std::time_t now = std::time(NULL);
		std::tm *tm_info = std::localtime(&now);

		std::ostringstream oss;
		oss << std::setfill('0')
			<< (tm_info->tm_year + 1900)
			<< std::setw(2) << (tm_info->tm_mon + 1)
			<< std::setw(2) << tm_info->tm_mday
			<< std::setw(2) << tm_info->tm_hour
			<< std::setw(2) << tm_info->tm_min
			<< std::setw(2) << tm_info->tm_sec;
		oss << std::setfill('0') << std::setw(2) << tm_info->tm_sec;
		return oss.str();
	}

	bool string_to_size_t(const std::string &str, int &result)
	{
		std::stringstream ss(str);

		ss >> result;

		return !ss.fail() && ss.eof();
	}

	void sigint_handle(int sig)
	{
		(void)sig;

		// Then shut down
		IOMultiplexer::getInstance().setStarted(false);
		return;
	}

	void signalhandler()
	{
		if (signal(SIGQUIT, SIG_IGN) == SIG_ERR)
			throw std::runtime_error(RED "signal() faild." RESET);
		if (signal(SIGINT, &sigint_handle) == SIG_ERR)
			throw std::runtime_error(RED "signal() faild." RESET);
		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error(RED "signal() failed for SIGPIPE." RESET);
		if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
			throw std::runtime_error(RED "signal() failed for SIGPIPE." RESET);
	}
}