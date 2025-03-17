#include "webserv.hpp"

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

std::string readFile(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file)
		return "";
	std::ostringstream content;
	content << file.rdbuf();
	return content.str();
}

bool writeFile(const std::string &filename, const std::string &content)
{
	std::ofstream file(filename.c_str());
	if (!file)
		return false;
	file << content;
	return true;
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
