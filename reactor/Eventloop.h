#pragma once
#include "Epoll.h"
#include<functional>
#include <memory>
#include <sys/syscall.h>
#include <unistd.h>
#include <queue>
#include <mutex>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include "Connection.h"
#include <map>
#include <atomic>

class Connection;
class Channel;
class Epoll;
using spConnection = std::shared_ptr<Connection>;

// 事件循环类。
class Eventloop
{
private:
    std::unique_ptr<Epoll> ep_;
    std::function<void(Eventloop*)> epolltimeoutcallback_;                       // 每个事件循环只有一个Epoll。
    pid_t threadid_;
    std::queue<std::function<void()>> taskqueue_;
    std::mutex mutex_;
    std::mutex mmutex_;
    int wakeupfd_;
    std::unique_ptr<Channel> wakechnnel_;
    int timerfd_;
    std::unique_ptr<Channel> timerchnnel_;
    bool mainloop_;
    std::map<int,spConnection> conns_;
    std::function<void(int)> timercallback_;
    int timevl_;
    int timeout_;
    std::atomic_bool stop_;

public:
    Eventloop(bool mainloop,int timevl = 30,int timeout = 80);                   // 在构造函数中创建Epoll对象ep_。
    ~Eventloop();                // 在析构函数中销毁ep_。

    void run();                      // 运行事件循环。
    void stop();

    void updatechannel(Channel *ch);
    void removechannel(Channel *ch);
    void setepolltimeoutcallback(std::function<void(Eventloop*)> fn);                        // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    bool isinloopthread();
    void queueinloop(std::function<void()> fn);
    void wakeup();
    void handlewakeup();
    void handletimer();
    void newconnection(spConnection conn);
    void settimercallback(std::function<void(int)> fn);
};

