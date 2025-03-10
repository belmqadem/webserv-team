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
        // Request     _request;
        sockaddr_in _client_addr;

    public:
        /* getters and setters */
        bool    isStarted() const {return _is_started;};
        void    setPeerSocketFd(uint32_t fd) { _peer_socket_fd = fd;};
        void    setServerSocketFd(uint32_t fd){ _server_socket_fd = fd;};
        void    setClientAddr(sockaddr_in addr) { _client_addr = addr;};

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
                LOG_INFO("Client Connected: " + addr + ":" + to_string(ntohs(_client_addr.sin_port)) + " Fd " + to_string(_peer_socket_fd));
            } catch (std::exception &e) {
                close(_peer_socket_fd);
                std::cerr << "Failed to register client fd " << _peer_socket_fd 
                        << " with IO multiplexer. Connection terminated.\n"
                        << "Error: " << e.what() << std::endl;
            }
        }

        ClientServer(const int &server_socket_fd, const int &peer_socket_fd) : _is_started(false),
            _server_socket_fd(server_socket_fd), _peer_socket_fd(peer_socket_fd) {};
        ~ClientServer() { terminate(); };
        
        virtual void    terminate() {
            IOMultiplexer::getInstance().removeListener(_epoll_ev, _peer_socket_fd);
            _is_started = false;

            std::string addr =  inet_ntoa(_client_addr.sin_addr);
            LOG_INFO("Client " + addr + " Fd " + to_string(_peer_socket_fd) + " Disconnected!");
            close(_peer_socket_fd);
        };
        virtual void    onEvent(int fd, epoll_event ev) {
            (void) fd;
            if (ev.events & EPOLLIN) {
                std::vector<byte> buff(RD_SIZE, 0);
                ssize_t rd_count = recv(this->_peer_socket_fd, buff.data(), RD_SIZE, MSG_DONTWAIT);
                if (rd_count < 0) {
                    LOG_ERROR("recv() Failed For Client " + to_string(this->_peer_socket_fd));
                    LOG_INFO("Clinet " + to_string(this->_peer_socket_fd) + "Will terminate");
                    this->terminate();
                }
                if (rd_count < RD_SIZE)
                    buff.erase(buff.begin()  + rd_count, buff.end());
                printf("%.*s\n", (int)rd_count, buff.data());
            }
        }
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
        virtual void    onEvent(int fd, epoll_event ev);
};
