#pragma once

#include <exception>
#include <string>

class IOMultiplexerExceptions : public std::exception
{
private:
	std::string _reason;

public:
	IOMultiplexerExceptions(std::string reason) throw();
	virtual const char *what() const throw();
	virtual ~IOMultiplexerExceptions() throw();
};

class ServerExceptions : public std::exception
{
private:
	std::string _reason;

public:
	ServerExceptions(std::string reason) throw();
	virtual const char *what() const throw();
	virtual ~ServerExceptions() throw();
};
