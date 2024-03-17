#include "Channel.h"

Channel::Channel(Eventloop *loop,int fd):loop_(loop),fd_(fd)
{

}
Channel::~Channel()
{

}
int Channel::fd()
{
    return fd_;
}
void Channel::useet()
{
    events_ = events_|EPOLLET;
}
void Channel::enablereading()
{
    events_|=EPOLLIN;
    loop_->updatechannel(this);
}
void Channel::disablereading()
{
    events_&=~EPOLLIN;
    loop_->updatechannel(this);
}

void Channel::enablewriting()
{
    events_|=EPOLLOUT;
    loop_->updatechannel(this);
}
void Channel::disablewriting()
{
    events_&=~EPOLLOUT;
    loop_->updatechannel(this);
}
void Channel::setinepoll()
{
    inepoll_ = true;
}
void Channel::setrevents(uint32_t ev)
{
    revents_ =ev;
}
bool Channel::inpoll()
{
    return inepoll_;
}
uint32_t Channel::events()
{
    return events_;
}
uint32_t Channel::revents()
{
    return revents_; 
}
void Channel::handleevent()
{
     if (revents_&EPOLLRDHUP)                     // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
        {
           
            
            closecallback_();
        }                                //  普通数据  带外数据
        else if (revents_&(EPOLLIN|EPOLLPRI))  
        {
           
            readcallback_();
        }
        else if (revents_&EPOLLOUT)                  // 有数据需要写，暂时没有代码，以后再说。
        {
            
            writecallback_();
        }
        else                                                                    // 其它事件，都视为错误。
        {
          
           
            errorcallback_();
        }
}

void Channel::setreadcallback(std::function<void()> fn)
{
    readcallback_=fn;
}
void Channel::setclosecallback(std::function<void()> fn)
{
    closecallback_=fn;
}
void Channel::seterrorcallback(std::function<void()> fn)
{
    errorcallback_=fn;
}
void Channel::setwritecallback(std::function<void()> fn)
{
    writecallback_=fn;
}
void Channel::disableall()
{
    events_=0;
    loop_->updatechannel(this);
}
void Channel::remove()
{
    disableall();
    loop_->removechannel(this);
}

