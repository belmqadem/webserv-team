// filepath: /webserv-team/webserv-team/src/Server.cpp
#include "Server.hpp"
#include "Logger.hpp"
#include "IOMultiplexer.hpp"
#include <iostream>

Server::Server(std::vector<ServerConfig> config) : _config(config), _is_started(false) {
    // Initialization code if needed
}

Server::~Server() {
    terminate();
}

void Server::StartServer() {
    // Implementation for starting the server
    // This may include binding sockets, listening for connections, etc.
}

void Server::listenOnAddr(sockaddr_in addr) {
    // Implementation for listening on the specified address
}

void Server::accept_peer(int fd) {
    // Implementation for accepting a new peer connection
}

void Server::terminate() {
    // Implementation for terminating the server
    // This may include closing sockets and cleaning up resources
}

void Server::onEvent(int fd, epoll_event ev) {
    // Implementation for handling events from the I/O multiplexer
}