#include "../../includes/webserv.hpp"

bool is_empty(char *arg)
{
	int i = 0;
	while (arg[i])
	{
		if (arg[i] == ' ' || arg[i] == '\t')
			i++;
		else
			break;
	}
	if (arg[i] == '\0')
		return true;
	return false;
}
