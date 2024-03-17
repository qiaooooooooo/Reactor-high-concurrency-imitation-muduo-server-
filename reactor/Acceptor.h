#pragma noce
#include <functional>
#include "InetAddress.h"
#include "Eventloop.h"
#include "Channel.h"
#include <memory>


class Acceptor
{
private:
    Eventloop *loop_;
    Socket servsock_;
    Channel acceptchannel_;
    std::function<void( std::unique_ptr<Socket>)> newconnectioncb_;
public:
    Acceptor(Eventloop *loop,const std::string &ip,const uint16_t port);
    ~Acceptor();
    void newconnection();
    void setnewconnectioncb(std::function<void( std::unique_ptr<Socket>)> fn);
};


