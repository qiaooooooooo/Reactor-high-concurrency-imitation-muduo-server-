#pragma once
#include "Eventloop.h"
#include "Socket.h"
#include "Channel.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include <map>
#include<memory>
#include <mutex>

// TCP网络服务类。
class Tcpserver
{
private:
    std::unique_ptr<Eventloop> mainloop_;
    std::vector<std::unique_ptr<Eventloop>> subloops_;
    int threadnum_;   
    std::mutex mmutex_;
    ThreadPool threadpool_;
    Acceptor acceptor_;   // 一个Tcpserver只有一个Acceptor对象。
    std::map<int,spConnection>  conns_;           // 一个Tcpserver有多个Connection对象，存放在map容器中。
    std::function<void(spConnection)> newconnectioncb_;          // 回调Echoserver::HandleNewConnection()。
    std::function<void(spConnection)> closeconnectioncb_;        // 回调Echoserver::HandleClose()。
    std::function<void(spConnection)> errorconnectioncb_;         // 回调Echoserver::HandleError()。
    std::function<void(spConnection,std::string &message)> onmessagecb_;        // 回调Echoserver::HandleMessage()。
    std::function<void(spConnection)> sendcompletecb_;            // 回调Echoserver::HandleSendComplete()。
    std::function<void(Eventloop*)>  timeoutcb_;                       // 回调Echoserver::HandleTimeOut()。
public:
    Tcpserver(const std::string &ip,const uint16_t port,int threadnum = 3);
    ~Tcpserver();

    void start();          // 运行事件循环。
    void stop(); 

    void newconnection(std::unique_ptr<Socket> clientsock);    // 处理新客户端连接请求，在Acceptor类中回调此函数。
    void closeconnection(spConnection conn);  // 关闭客户端的连接，在Connection类中回调此函数。 
    void errorconnection(spConnection conn);  // 客户端的连接错误，在Connection类中回调此函数。
    void onmessage(spConnection conn,std::string &message);     // 处理客户端的请求报文，在Connection类中回调此函数。
    void sendcomplete(spConnection conn);     // 数据发送完成后，在Connection类中回调此函数。
    void epolltimeout(Eventloop *loop);         // epoll_wait()超时，在Eventloop类中回调此函数。

    void setnewconnectioncb(std::function<void(spConnection)> fn);
    void setcloseconnectioncb(std::function<void(spConnection)> fn);
    void seterrorconnectioncb(std::function<void(spConnection)> fn);
    void setonmessagecb(std::function<void(spConnection,std::string &message)> fn);
    void setsendcompletecb(std::function<void(spConnection)> fn);
    void settimeoutcb(std::function<void(Eventloop*)> fn);
    void removeconn(int fd);
};