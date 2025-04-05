#pragma once

#include "ConfigManager.hpp"
#include "IOMultiplexer.hpp"
#include "CGIHandler.hpp"

class CGIHandler;
class RequestParser;

#define RD_SIZE 1024
#define TIME_OUT_SECONDS 60
#define DOUBLE_CRLF "\r\n\r\n"

class ClientServer : public IEvenetListeners
{
private:
    bool _is_started;
    int _server_socket_fd;
    int _peer_socket_fd;
    epoll_event _epoll_ev;
    sockaddr_in _client_addr;
    RequestParser *_parser;
    std::string _request_buffer;
    std::string _response_buffer;
    bool _response_ready;
    time_t _last_activity;
    bool _continue_sent;
    std::map<std::string, std::string> _tempHeaders; // Temp headers for CGI response

    // Add new members for CGI support
    CGIHandler *_pendingCgi;
    bool _waitingForCGI;

private:
    // Main processing methods
    void handleIncomingData();
    void handleResponse();
    void modifyEpollEvent(uint32_t events);

    // Request processing methods
    bool readIncomingData();
    void handleExpectContinue();
    bool isParsingRequestBody();
    void processRequestHeaders();
    void parseHeaders();
    void processSessionCookie();
    void processCompletedRequest();
    void processNormalRequest();
    void handleRequestException(const std::exception &e);
    void cleanupParser();

    // Response handling methods
    bool isResponseReady();
    void handleTimeout();
    bool sendResponseChunk();
    void finalizeResponse();
    
    // Timeout check
    bool hasTimeOut() const;

public:
    void updateActivity();
    void checkCGIProgress();
    bool isStarted() const;
    void setPeerSocketFd(uint32_t fd);
    void setServerSocketFd(uint32_t fd);
    void setClientAddr(sockaddr_in addr);
    void RegisterWithIOMultiplexer();
    bool shouldKeepAlive() const;

    ClientServer(const int &server_socket_fd, const int &peer_socket_fd);
    ~ClientServer();

    virtual void terminate();
    virtual void onEvent(int fd, epoll_event ev);

    // CGI methods
    void processCGIRequest();
    void onCGIComplete(CGIHandler *handler);
};
