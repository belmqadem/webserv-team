#include "webserv.hpp"
#include "ConfigManager.hpp"
#include "IOMultiplexer.hpp"
#include "Server.hpp"
#include "Logger.hpp"
#include <signal.h>

extern int webserv_signal;

void	sigint_handle(int sig) {
	webserv_signal = sig;
	std::cout << "Interrupt signal catched." << std::endl;
}

void signalhandler() {
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR)
		throw std::runtime_error("signal() faild.");
	if (signal(SIGINT, &sigint_handle) == SIG_ERR)
		throw std::runtime_error("signal() faild.");
}


int main()
{
	if (ac != 2) {
		std::cout << USAGE(av[0]) << "\n";
		return (1);
	}

	Logger::getInstance().setLevel(DEBUG);
	Logger::getInstance().setOutput(true, true);
	Logger::getInstance().setLogFile("WebServe.log");
	try {
		ConfigManager::getInstance()->loadConfig(av[1]);
		LOG_INFO("Webserver Starting...");
		Server &server = Server::getInstance(ConfigManager::getInstance()->getServers());
		IOMultiplexer::getInstance().runEventLoop();
	} catch (std::exception &e) {
		std::cerr << "Fatal error: \n" << e.what() << "\n";
	}
	return (0);
}

// void handle_request(RequestParser &request)
// {
// 	Router router;
// 	ResponseBuilder response;

// 	std::string handler_type = router.get_handler(request);

// 	if (handler_type == "REDIRECT")
// 	{
// 		response.set_status(302, STATUS_302);
// 		response.set_headers("Location", router.resolve_route(request.get_request_uri()));
// 	}
// 	else if (handler_type == "STATIC")
// 	{
// 		std::string file_path = router.resolve_route(request.get_request_uri());
// 		std::ifstream file(file_path.c_str());

// 		if (!file.is_open())
// 		{
// 			response.set_status(404, STATUS_400);
// 			response.set_body("404 Not Found");
// 		}
// 		else
// 		{
// 			std::stringstream buffer;
// 			buffer << file.rdbuf();
// 			response.set_body(buffer.str());
// 			response.set_headers("Content-Type", "text/html");
// 		}
// 	}
// 	else if (handler_type == "CGI")
// 	{
// 		response.set_status(200, STATUS_200);
// 		response.set_body("CGI execution not implemented yet.");
// 	}
// 	else
// 	{
// 		response.set_status(404, "Not Found");
// 		response.set_body("404 Not Found");
// 	}

// 	std::cout << response.build_response();
// }