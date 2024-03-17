#include "Acceptor.h"

Acceptor::Acceptor(Eventloop *loop,const std::string &ip,const uint16_t port)
:loop_(loop),servsock_(createnonblocking()),acceptchannel_(loop_,servsock_.fd())
{
    InetAddress servaddr(ip,port);
    servsock_.setkeepalive(true);
    servsock_.setreuseaddr(true);
    servsock_.setreuseport(true);
    servsock_.settcpnodelay(true);
    servsock_.bind(servaddr);
    servsock_.listen(128);

    //acceptchannel_ = new Channel(loop_,servsock_.fd());
    acceptchannel_.setreadcallback(std::bind(&Acceptor::newconnection,this));
    acceptchannel_.enablereading();
}
Acceptor::~Acceptor()
{
    //delete servsock_;
    //delete acceptchannel_;
}
#include "Connection.h"
void Acceptor::newconnection()
{
    InetAddress clientaddr;
    std::unique_ptr<Socket> clientsock(new Socket(servsock_.accept(clientaddr)));
    clientsock->setipport(clientaddr.ip(),clientaddr.port());

    //printf ("accept client(fd=%d,ip=%s,port=%d) ok.\n",clientsock->fd(),clientaddr.ip(),clientaddr.port());
    newconnectioncb_(std::move(clientsock));
    
}
void Acceptor::setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn)
{
    newconnectioncb_ = fn;
}