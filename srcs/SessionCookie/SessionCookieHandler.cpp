#include "SessionCookieHandler.hpp"

// Generate a session ID based on current time and a random number
std::string SessionCookieHandler::generate_session_id()
{
	std::time_t now = std::time(NULL);
	struct tm *timeinfo = localtime(&now);

	char buffer[1024];
	strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", timeinfo);

	srand(std::time(NULL));
	int random_number = rand() % 10000;

	std::string session_id(buffer);
	session_id += "_" + Utils::to_string(random_number);

	return session_id;
}

// Set a cookie
void SessionCookieHandler::set_cookie(ResponseBuilder &response, const std::string &name, const std::string &value, int max_age_seconds)
{
	std::string cookie_header = name + "=" + value;
	if (max_age_seconds > 0)
	{
		cookie_header += "; Max-Age=" + Utils::to_string(max_age_seconds);
	}
	cookie_header += "; Path=/";
	cookie_header += "; HttpOnly; SameSite=Strict";

	response.set_headers("Set-Cookie", cookie_header);
}

// Get a cookie value from request headers
std::string SessionCookieHandler::get_cookie(RequestParser &request, const std::string &name)
{
	std::string cookie_header = request.get_header_value("cookie");
	size_t start_pos = cookie_header.find(name + "=");
	if (start_pos != std::string::npos)
	{
		start_pos += name.length() + 1;
		size_t end_pos = cookie_header.find(";", start_pos);
		if (end_pos == std::string::npos)
		{
			end_pos = cookie_header.length();
		}
		return cookie_header.substr(start_pos, end_pos - start_pos);
	}
	return ""; // Return empty string if cookie not found
}

// Delete a cookie (set its Max-Age to 0)
void SessionCookieHandler::delete_cookie(ResponseBuilder &response, const std::string &name)
{
	std::string cookie_header = name + "=; Max-Age=0; Path=/";
	response.set_headers("Set-Cookie", cookie_header);
}

// Validate the session (check if a session ID exists in cookies)
bool SessionCookieHandler::validate_session(RequestParser &request)
{
	std::string session_id = get_cookie(request, "session_id");
	return !session_id.empty(); // If session_id exists, consider the session valid
}
