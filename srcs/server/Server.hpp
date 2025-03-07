#pragma once

#include "IEvenetListeners.hpp"
#include "ConfigManager.hpp"
#include "IOMultiplexer.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

class Server : public IEvenetListeners {
    private:
        std::vector<ServerConfig> _config;
        bool                      _is_started;
        epoll_event               _listen_sock_ev;
        std::vector<int>          _listen_fds;

    private:
        ~Server();
        Server(std::vector<ServerConfig> config);
    public:
        void StartServer();
    public:
        static Server &getInstance(std::vector<ServerConfig> config);
    public:
        void listenOnAddr(sockaddr_in addr);

        virtual void    terminate();
        virtual void    onEvent(int fd, uint32_t ev);
};