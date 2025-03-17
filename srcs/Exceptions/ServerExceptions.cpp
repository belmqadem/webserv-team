#include "Exceptions.hpp"

ServerExceptions::ServerExceptions(std::string reason) throw() : _reason("[ServerExceptions::ServerExceptions] : " + reason) {}

ServerExceptions::~ServerExceptions() throw() {}

const char *ServerExceptions::what() const throw()
{
	return _reason.c_str();
}