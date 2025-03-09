#pragma once

#include "Logger.hpp"
#include "IEvenetListeners.hpp"
#include "ConfigManager.hpp"
#include "IOMultiplexer.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <fcntl.h>
#include "webserv.hpp"

class Request {};

class ClientServer : IEvenetListeners {
    private:
        bool        _is_started;
        int         _server_socket_fd;
        int         _peer_socket_fd;
        epoll_event _epoll_ev;
        Request     _request;
        sockaddr_in _client_addr;

    public:
        /* getters and setters */
        bool    isStarted() const;
        void    setPeerSocketFd(uint32_t fd);
        void    setServerSocketFd(uint32_t fd);
        void    setClientAddr(sockaddr_in addr);

    public:
        void RegisterWithIOMultiplexer() {
            if (_is_started == true) {
                std::cerr << "WARNING: Attempting to registre an already started fd " << _peer_socket_fd << std::endl;
                return ;
            }
            _epoll_ev.data.fd = _peer_socket_fd;
            _epoll_ev.events = EPOLLIN | EPOLLOUT;
            try {
                IOMultiplexer::getInstance().addListener(this, _epoll_ev);
                _is_started = true;

                std::string addr = inet_ntoa(_client_addr.sin_addr);
                LOG_INFO("Client Connected: " + addr + ":" + to_string(ntohs(_client_addr.sin_port)));
            } catch (std::exception &e) {
                close(_peer_socket_fd);
                std::cerr << "Failed to register client fd " << _peer_socket_fd 
                        << " with IO multiplexer. Connection terminated.\n"
                        << "Error: " << e.what() << std::endl;
            }
        }

        ClientServer(const int &server_socket_fd, const int &peer_socket_fd) : _is_started(false),
            _server_socket_fd(server_socket_fd), _peer_socket_fd(peer_socket_fd) {};
        ~ClientServer() {};
        
        virtual void    terminate() {};
        virtual void    onEvent(int fd, uint32_t ev) {};
};

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
        sockaddr_in     getListenAddress(ServerConfig conf);
        void            listenOnAddr(sockaddr_in addr);
        void            accept_peer(int fd);

        virtual void    terminate();
        virtual void    onEvent(int fd, uint32_t ev);
};