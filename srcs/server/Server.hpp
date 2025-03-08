#pragma once

#include "IEvenetListeners.hpp"
#include "ConfigManager.hpp"
#include "IOMultiplexer.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

class ClientServer : public IEvenetListeners {};

class Server : public IEvenetListeners {
    private:
        /* List of Server configs */
        std::vector<ServerConfig>    _config;
        /* State of our Server */
        bool                         _is_started;
        /* Socket fd and event it interested to */
        epoll_event                  _listen_sock_ev;
        /* Pool of server sockets fds */
        std::vector<int>             _listen_fds;
        /* List of ClientServer connection objects */
        std::vector<ClientServer*>   _clients;

    private:
        ~Server();
        Server(std::vector<ServerConfig> config);
    public:
        void StartServer();
    public:
        static Server &getInstance(std::vector<ServerConfig> config);
    public:
        sockaddr_in getListenAddress(ServerConfig conf);
        void listenOnAddr(sockaddr_in addr);

        virtual void    terminate();
        virtual void    onEvent(int fd, uint32_t ev);
};