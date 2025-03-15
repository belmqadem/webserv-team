#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"
#include "Logger.hpp"
#include "Parser.hpp"

int main()
{
	Logger::getInstance().setLevel(DEBUG);
	Logger::getInstance().setOutput(true, true);
	Logger::getInstance().setLogFile("WebServe.log");

	ConfigManager::getInstance()->loadConfig("config/webserv.conf");
	std::vector<ServerConfig> servers = ConfigManager::getInstance()->getServers();

	std::string request = "POST ////uploads/test/../file.txt HTTP/1.1\r\n"
						  "Host: localhost\r\n"
						  "Content-Type: plain/txt\r\n"
						  "Transfer-Encoding: chunked\r\n"
						  "\r\n"
						  "5\r\nhello\r\n5\r\nworld\r\n0\r\n\r\n";

	std::cout << CYAN "** START REQUEST PARSING **" RESET << std::endl;
	RequestParser parser(request, &servers[0]);
	parser.print_request();
	std::cout << CYAN "** REQUEST PARSING DONE **" RESET << std::endl;

	ResponseBuilder response(parser);
	std::cout << CYAN "** START RESPONSE GENERATING **" RESET << std::endl;
	std::cout << response.get_response() << std::endl;
	std::cout << CYAN "** RESPONSE GENERATING DONE**" RESET << std::endl;
}
