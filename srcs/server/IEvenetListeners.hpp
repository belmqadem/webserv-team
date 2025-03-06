#include "sys/epoll.h"

class IEvenetListeners
{
private:
public:
    IEvenetListeners() {};
    virtual ~IEvenetListeners() {};
    IEvenetListeners(const IEvenetListeners&) = delete;
    IEvenetListeners &operator=(const IEvenetListeners&) = delete;
public:
    virtual void onEvent(int fd, uint32_t ev) = 0;
    virtual void terminate() = 0;
};
