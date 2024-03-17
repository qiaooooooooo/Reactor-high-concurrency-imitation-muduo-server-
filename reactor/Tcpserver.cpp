#include "Tcpserver.h"

Tcpserver::Tcpserver(const std::string &ip,const uint16_t port,int threadnum)
:threadnum_(threadnum),mainloop_(new Eventloop(true,5,10)),acceptor_(mainloop_.get(),ip,port),threadpool_(threadnum_,"IO")
{
    mainloop_->setepolltimeoutcallback(std::bind(&Tcpserver::epolltimeout,this,std::placeholders::_1));
    acceptor_.setnewconnectioncb(std::bind(&Tcpserver::newconnection,this,std::placeholders::_1));
    for(int ii;ii<threadnum;ii++)
    {
        subloops_.emplace_back(new Eventloop(false,5,10));
        subloops_[ii]->setepolltimeoutcallback(std::bind(&Tcpserver::epolltimeout,this,std::placeholders::_1));
        subloops_[ii]->settimercallback(std::bind(&Tcpserver::removeconn,this,std::placeholders::_1));
        threadpool_.addtask(std::bind(&Eventloop::run,subloops_[ii].get()));
    }
   
}

Tcpserver::~Tcpserver()
{
    //delete acceptor_;
    //delete mainloop_;

    // 释放全部的Connection对象。
    /*for(auto &aa:subloops_)
    {
        delete aa;
    }*/
    //delete threadpool_;
}

// 运行事件循环。
void Tcpserver::start()          
{
    mainloop_->run();
}
void Tcpserver::stop()
{
    mainloop_->stop();
    for(int ii;ii<threadnum_;ii++)
    {
        subloops_[ii]->stop();
    }
    threadpool_.stop();
}

// 处理新客户端连接请求。
void Tcpserver::newconnection(std::unique_ptr<Socket> clientsock)
{
    //Connection *conn=new Connection(mainloop_,clientsock); 
    spConnection conn(new Connection(subloops_[clientsock->fd()%threadnum_].get(),std::move(clientsock)));   
    conn->setclosecallback(std::bind(&Tcpserver::closeconnection,this,std::placeholders::_1));
    conn->seterrorcallback(std::bind(&Tcpserver::errorconnection,this,std::placeholders::_1));
    conn->setonmessagecallback(std::bind(&Tcpserver::onmessage,this,std::placeholders::_1,std::placeholders::_2));
    conn->setsendcompletecallback(std::bind(&Tcpserver::sendcomplete,this,std::placeholders::_1));

    // printf ("new connection(fd=%d,ip=%s,port=%d) ok.\n",conn->fd(),conn->ip().c_str(),conn->port());

    {
        std::lock_guard<std::mutex> gb(mmutex_);
        conns_[conn->fd()]=conn; 

    }
    conns_[conn->fd()]=conn;            // 把conn存放map容器中。
    subloops_[conn->fd()%threadnum_]->newconnection(conn);

    if (newconnectioncb_) newconnectioncb_(conn);             // 回调Echoserver::HandleNewConnection()。
}

 // 关闭客户端的连接，在Connection类中回调此函数。 
 void Tcpserver::closeconnection(spConnection conn)
 {
    if (closeconnectioncb_) closeconnectioncb_(conn);       // 回调Echoserver::HandleClose()。

    // printf("client(eventfd=%d) disconnected.\n",conn->fd());
    {
        std::lock_guard<std::mutex> gb(mmutex_);
        conns_.erase(conn->fd());        // 从map中删除conn。
        
    }
    
 }

// 客户端的连接错误，在Connection类中回调此函数。
void Tcpserver::errorconnection(spConnection conn)
{
    if (errorconnectioncb_) errorconnectioncb_(conn);     // 回调Echoserver::HandleError()。

    {
        std::lock_guard<std::mutex> gb(mmutex_);
        conns_.erase(conn->fd());        // 从map中删除conn。
        
    }
}

// 处理客户端的请求报文，在Connection类中回调此函数。
void Tcpserver::onmessage(spConnection conn,std::string &message)
{
    /*
    // 在这里，将经过若干步骤的运算。
    message="reply:"+message;          // 回显业务。
                
    int len=message.size();                   // 计算回应报文的大小。
    std::string tmpbuf((char*)&len,4);  // 把报文头部填充到回应报文中。
    tmpbuf.append(message);             // 把报文内容填充到回应报文中。
                
    conn->send(tmpbuf.data(),tmpbuf.size());   // 把临时缓冲区中的数据发送出去。
    */
    if (onmessagecb_) onmessagecb_(conn,message);     // 回调Echoserver::HandleMessage()。
}

// 数据发送完成后，在Connection类中回调此函数。
void Tcpserver::sendcomplete(spConnection conn)     
{
    //printf("send complete.\n");

    if (sendcompletecb_) sendcompletecb_(conn);     // 回调Echoserver::HandleSendComplete()。
}

// epoll_wait()超时，在Eventloop类中回调此函数。
void Tcpserver::epolltimeout(Eventloop *loop)         
{
    // printf("epoll_wait() timeout.\n");

    if (timeoutcb_)  timeoutcb_(loop);           // 回调Echoserver::HandleTimeOut()。
}

void Tcpserver::setnewconnectioncb(std::function<void(spConnection)> fn)
{
    newconnectioncb_=fn;
}

void Tcpserver::setcloseconnectioncb(std::function<void(spConnection)> fn)
{
    closeconnectioncb_=fn;
}

void Tcpserver::seterrorconnectioncb(std::function<void(spConnection)> fn)
{
    errorconnectioncb_=fn;
}

void Tcpserver::setonmessagecb(std::function<void(spConnection,std::string &message)> fn)
{
    onmessagecb_=fn;
}

void Tcpserver::setsendcompletecb(std::function<void(spConnection)> fn)
{
    sendcompletecb_=fn;
}

void Tcpserver::settimeoutcb(std::function<void(Eventloop*)> fn)
{
    timeoutcb_=fn;
}
void Tcpserver::removeconn(int fd)
{
    {
        std::lock_guard<std::mutex> gb(mmutex_);
        conns_.erase(fd);
    }
        
    }
    
