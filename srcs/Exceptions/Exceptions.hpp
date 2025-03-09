#pragma once


#include <exception>
#include <string>


class IOMultiplexerExceptions : public std::exception
{
private:
    std::string _reason;
public:
    IOMultiplexerExceptions(std::string reason) throw() ;
    const char *what() const throw() ;
    ~IOMultiplexerExceptions() throw() ;
};


class ServerExceptions : public std::exception
{
private:
    std::string _reason;
public:
    ServerExceptions(std::string reason) throw() ;
    const char *what() const throw() ;
    ~ServerExceptions() throw() ;
};
