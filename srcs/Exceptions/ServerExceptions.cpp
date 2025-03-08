#include "exceptions.hpp"

ServerExceptions::ServerExceptions(std::string reason) throw() : _reason("[ServerExceptions::ServerExceptions] : " + reason) { }

ServerExceptions::~ServerExceptions() { }

const char *ServerExceptions::what() const throw() {
    return _reason.c_str();
}