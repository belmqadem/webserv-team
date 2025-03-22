#include "Logger.hpp"
#include "webserv.hpp"

std::string readConfig(const std::string &filename)
{
	std::ifstream conf(filename.c_str());
	if (!conf)
	{
		LOG_ERROR("Error: Could not open the file " + filename);
		return "";
	}

	std::ostringstream buffer;
	buffer << conf.rdbuf();
	conf.close();

	return (buffer.str());
}