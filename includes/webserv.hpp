// This is the Main project header
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <cerrno>
#include <cstring>

#include <poll.h>
#include <sys/epoll.h>
#include <cstdlib>


typedef uint8_t byte ;
#define RD_SIZE 1024

#define USAGE(progname) "Usage " + std::string(progname) + " [/path/to/config/file]"

template<class T> std::string to_string(T t)
{
	std::stringstream str;
	str << t;
	return str.str();
}