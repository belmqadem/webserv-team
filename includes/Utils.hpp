#pragma once

#include "webserv.hpp"

namespace Utils
{
	std::vector<std::string> split(const char *start, const char *end, char delimiter);
	std::string trim(const std::string &str, const std::string &delim);
	bool is_numeric(const std::string &str);
	std::string get_timestamp_str();
	void sigint_handle(int sig);
	void signalhandler();
	bool string_to_size_t(const std::string &str, int &result);

	template <class T>
	std::string to_string(T t)
	{
		std::stringstream str;
		str << t;
		return str.str();
	}
}