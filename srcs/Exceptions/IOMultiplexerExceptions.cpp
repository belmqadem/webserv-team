#include "Exceptions.hpp"

IOMultiplexerExceptions::IOMultiplexerExceptions(std::string reason) __THROW : _reason("[IOMultiplexerExceptions] [REASON]: " + reason) {}

IOMultiplexerExceptions::~IOMultiplexerExceptions() __THROW {}

const char *IOMultiplexerExceptions::what() const __THROW
{
	return _reason.c_str();
}
