#pragma once
#include <sys/epoll.h>
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Eventloop.h"
#include<functional>
#include <memory>

class Eventloop;

class Channel
{
private:
     int fd_ = -1;
     Eventloop *loop_;
     bool inepoll_ = false;
     uint32_t events_ =0;
     uint32_t revents_ =0;
     std::function<void()> readcallback_;
     std::function<void()> closecallback_;
     std::function<void()> errorcallback_;
     std::function<void()> writecallback_;
public:
    Channel(Eventloop *loop,int fd);
    ~Channel();

    int fd();
    void useet();
    void enablereading();
    void disablereading();

    void enablewriting();
    void disablewriting();
    
    void disableall();
    void remove();
    void setinepoll();
    void setrevents(uint32_t ev);
    bool inpoll();
    uint32_t events();
    uint32_t revents();
    void handleevent();

    void setreadcallback(std::function<void()> fn);
    void setclosecallback(std::function<void()> fn);
    void seterrorcallback(std::function<void()> fn);
    void setwritecallback(std::function<void()> fn);



};