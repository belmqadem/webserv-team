#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include "webserv.hpp"

std::string readConfig(const std::string &filename)
{
	std::ifstream conf(filename.c_str());
	if (!conf)
	{
		std::cerr << RED "Error: Could not open the file " << filename << RESET << std::endl;
		return "";
	}

	std::ostringstream buffer;
	buffer << conf.rdbuf();
	conf.close();

	return (buffer.str());
}