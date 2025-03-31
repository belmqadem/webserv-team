#pragma once

#include "ResponseBuilder.hpp"

class ResponseBuilder;

class SessionCookieHandler
{
public:
	static std::string generate_session_id();
	static void set_cookie(ResponseBuilder &response, const std::string &name, const std::string &value, int max_age_seconds = 3600);
	static std::string get_cookie(RequestParser &request, const std::string &name);
	static void delete_cookie(ResponseBuilder &response, const std::string &name);
	static bool validate_session(RequestParser &request);
};
