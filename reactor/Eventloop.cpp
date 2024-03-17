#include "Eventloop.h"

int createtimerfd(int sec = 30)
{
    int tfd=timerfd_create(CLOCK_MONOTONIC,TFD_CLOEXEC|TFD_NONBLOCK);   // 创建timerfd。
    struct itimerspec timeout;                                // 定时时间的数据结构。
    memset(&timeout,0,sizeof(struct itimerspec));
    timeout.it_value.tv_sec = sec;                             // 定时时间为5秒。
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(tfd,0,&timeout,0);                  // 开始计时。alarm(5)
    return tfd;
}
// 在构造函数中创建Epoll对象ep_。
Eventloop::Eventloop(bool mainloop,int timevl, int timeout)
:ep_(new Epoll),mainloop_(mainloop),timevl_(timevl),timeout_(timeout),stop_(false),
wakeupfd_(eventfd(0,EFD_NONBLOCK)),wakechnnel_(new Channel(this,wakeupfd_)),
timerfd_(createtimerfd(timeout)),timerchnnel_(new Channel(this,timerfd_))                   
{
    wakechnnel_->setreadcallback(std::bind(&Eventloop::handlewakeup,this));
    wakechnnel_->enablereading();

    timerchnnel_->setreadcallback(std::bind(&Eventloop::handletimer,this));
    timerchnnel_->enablereading();
}

// 在析构函数中销毁ep_。
Eventloop::~Eventloop()
{
    //delete ep_;
}

// 运行事件循环。
void Eventloop::run()                      
{
    threadid_ = syscall(SYS_gettid);
   
    while (stop_==false)        // 事件循环。
    {
        
        std::vector<Channel *> channels=ep_->loop(10*1000);         // 等待监视的fd有事件发生。


        if(channels.size()==0)
        {
            epolltimeoutcallback_(this);
        }
        else
        {
            for (auto &ch:channels)
            {
                ch->handleevent();        // 处理epoll_wait()返回的事件。
            }
        }
        
    }
}
void Eventloop::stop()
{
    stop_=true;
}

// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
void Eventloop::updatechannel(Channel *ch)                   
{
    ep_->updatechannel(ch);
}
void Eventloop::setepolltimeoutcallback(std::function<void(Eventloop*)> fn)
{
    epolltimeoutcallback_ = fn;
}
void Eventloop::removechannel(Channel *ch)
{
    ep_->removechannel(ch);
}

bool Eventloop::isinloopthread()
{
    return threadid_ == syscall(SYS_gettid);
}

void Eventloop::queueinloop(std::function<void()> fn)
{
    {
        std::lock_guard<std::mutex> gd(mutex_);
        taskqueue_.push(fn);
    }
    wakeup();
}
void Eventloop::wakeup()
{
    uint64_t val = 1;
    write(wakeupfd_,&val,sizeof(val));
}
void Eventloop::handlewakeup()
{
    uint64_t val;
    read(wakeupfd_,&val,sizeof(val));

    std::function<void()> fn;
    std::lock_guard<std::mutex> gd(mutex_);

    while (taskqueue_.size()>0)
    {
        fn = std::move(taskqueue_.front());
        taskqueue_.pop();
        fn();
    }
    
}
void Eventloop::handletimer()
{
    struct itimerspec timeout;                                // 定时时间的数据结构。
    memset(&timeout,0,sizeof(struct itimerspec));
    timeout.it_value.tv_sec = timevl_;                             // 定时时间为5秒。
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timerfd_,0,&timeout,0);                  // 开始计时。alarm(5)

    if(mainloop_)
    {
        //printf("主事件循环的时间到了\n");
    }
    else
    {
        //printf("从事件循环的时间到了\n");
        time_t now=time(0);
        for (auto aa:conns_)
        {
            if(aa.second->timeout(now,timeout_))
            {
                {
                    std::lock_guard<std::mutex> gb(mmutex_);
                    conns_.erase(aa.first);
                }
                timercallback_(aa.first);
            }
        } 
    }
    
}
void Eventloop::newconnection(spConnection conn)
{
    std::lock_guard<std::mutex> gb(mmutex_);
    conns_[conn->fd()] = conn;
}
void Eventloop::settimercallback(std::function<void(int)> fn)
{
    timercallback_ = fn;
}