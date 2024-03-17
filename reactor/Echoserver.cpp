#include "Echoserver.h"

/*
class Echoserver
{
private:
    TcpServer tcpserver_;

public:
    Echoserver(const std::string &ip,const uint16_t port);
    ~Echoserver();

    void Start();                // 启动服务。

    void HandleNewConnection(Socket *clientsock);    // 处理新客户端连接请求，在TcpServer类中回调此函数。
    void HandleClose(spConnection conn);  // 关闭客户端的连接，在TcpServer类中回调此函数。 
    void HandleError(spConnection conn);  // 客户端的连接错误，在TcpServer类中回调此函数。
    void HandleMessage(spConnection conn,std::string message);     // 处理客户端的请求报文，在TcpServer类中回调此函数。
    void HandleSendComplete(spConnection conn);     // 数据发送完成后，在TcpServer类中回调此函数。
    void HandleTimeOut(EventLoop *loop);         // epoll_wait()超时，在TcpServer类中回调此函数。
};
*/

Echoserver::Echoserver(const std::string &ip,const uint16_t port,int subthreadnum,int workthreadnum)
:tcpserver_(ip,port,subthreadnum),threadpool_(workthreadnum,"WORK")
{
    // 以下代码不是必须的，业务关心什么事件，就指定相应的回调函数。
    tcpserver_.setnewconnectioncb(std::bind(&Echoserver::HandleNewConnection, this, std::placeholders::_1));
    tcpserver_.setcloseconnectioncb(std::bind(&Echoserver::HandleClose, this, std::placeholders::_1));
    tcpserver_.seterrorconnectioncb(std::bind(&Echoserver::HandleError, this, std::placeholders::_1));
    tcpserver_.setonmessagecb(std::bind(&Echoserver::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setsendcompletecb(std::bind(&Echoserver::HandleSendComplete, this, std::placeholders::_1));
    // tcpserver_.settimeoutcb(std::bind(&Echoserver::HandleTimeOut, this, std::placeholders::_1));
}

Echoserver::~Echoserver()
{
   
}

// 启动服务。
void Echoserver::Start()                
{
    tcpserver_.start();
}
void Echoserver::Stop()
{
    tcpserver_.stop();
}

// 处理新客户端连接请求，在TcpServer类中回调此函数。
void Echoserver::HandleNewConnection(spConnection conn)    
{
    std::cout << "New Connection Come in." << std::endl;
    //printf("Echoserver::HandleNewConnection thread is %d\n",syscall(SYS_gettid));

    // 根据业务的需求，在这里可以增加其它的代码。
    printf ("%s new connection(fd=%d,ip=%s,port=%d) ok.\n",Timestamp::now().tostring().c_str(),conn->fd(),conn->ip().c_str(),conn->port());
}

// 关闭客户端的连接，在TcpServer类中回调此函数。 
void Echoserver::HandleClose(spConnection conn)  
{
    //std::cout << "Echoserver conn closed." << std::endl;
    
    printf ("%s new connection closed(fd=%d,ip=%s,port=%d) ok.\n",Timestamp::now().tostring().c_str(),conn->fd(),conn->ip().c_str(),conn->port());

    // 根据业务的需求，在这里可以增加其它的代码。
}

// 客户端的连接错误，在TcpServer类中回调此函数。
void Echoserver::HandleError(spConnection conn)  
{
    std::cout << "Echoserver conn error." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

// 处理客户端的请求报文，在TcpServer类中回调此函数。
void Echoserver::HandleMessage(spConnection conn,std::string &message)     
{
    //printf("Echoserver::HandleMessage thread is %d\n",syscall(SYS_gettid));
    // 在这里，将经过若干步骤的运算。
    if(threadpool_.size()==0)
    {
        OnMessage(conn,message);
    }
    else
    {
        threadpool_.addtask(std::bind(&Echoserver::OnMessage,this,conn,message));
    }
    
    
}

// 数据发送完成后，在TcpServer类中回调此函数。
void Echoserver::HandleSendComplete(spConnection conn)     
{
    //std::cout << "Message send complete." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

/*
// epoll_wait()超时，在TcpServer类中回调此函数。
void Echoserver::HandleTimeOut(EventLoop *loop)         
{
    std::cout << "Echoserver timeout." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}
*/

void Echoserver::OnMessage(spConnection conn,std::string&message)
{
    //printf("%s message (eventfd=%d):%s\n",Timestamp::now().tostring(),conn->fd(),message.c_str());
    message="reply:"+message;          // 回显业务。           
    conn->send(message.data(),message.size());   // 把临时缓冲区中的数据发送出去。

}