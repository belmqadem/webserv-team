#pragma once

#include "IEvenetListeners.hpp"
#include "../ParseConf/Parser.hpp"


class Server : public IEvenetListeners {
    public:
        ~Server();
    private:
        Server();
    public:
        virtual void    terminate();
        virtual void    onEvent(int fd, uint32_t ev);
};