#include "Exceptions.hpp"

IOMultiplexerExceptions::IOMultiplexerExceptions(std::string reason) __THROW : _reason("[IOMultiplexerExceptions] [REASON]: " + reason) { }

const char *IOMultiplexerExceptions::what() const __THROW {
    return _reason.c_str();
}

IOMultiplexerExceptions::~IOMultiplexerExceptions() {}