#pragma once


#include <exception>
#include <string>


class IOMultiplexerExceptions : public std::exception
{
private:
    std::string _reason;
public:
    IOMultiplexerExceptions(std::string reason) __THROW;
    const char *what() const __THROW;
    ~IOMultiplexerExceptions() __THROW;
};
