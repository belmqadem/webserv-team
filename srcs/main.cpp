#include "webserv.hpp"
#include "IOMultiplexer.hpp"
#include "ConfigManager.hpp"
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


int main(int ac, char **av)
{
	if (ac != 2) {
		std::cout << USAGE << "\n";
		return (1);
	}

	try {
		ConfigManager::getInstance()->loadConfig(av[1]);
		//server.start();
		IOMultiplexer::getInstance().runEventLoop();
	} catch (std::exception &e) {
		std::cerr << "Fatal error: \n" << e.what() << "\n";
	}
	return (0);
}
