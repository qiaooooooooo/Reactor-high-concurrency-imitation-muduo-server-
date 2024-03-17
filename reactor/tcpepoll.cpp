
#include "Echoserver.h"
#include <signal.h>

Echoserver *echoserver;

void Stop(int sig)
{
    printf("sig = %d\n",sig);
    echoserver ->Stop();
    printf("tcpepoll已经停止。");
    delete echoserver;
    printf("delete echoserver已经停止。");
    exit(0);
}



int main(int argc,char *argv[])
{
    if (argc != 3) 
    { 
        printf("usage: ./tcpepoll ip port\n"); 
        printf("example: ./tcpepoll 192.168.235.131 5005\n\n"); 
        return -1; 
    }

    signal(SIGINT,Stop);
    signal(SIGTERM,Stop);
    //Tcpserver tcpservver(argv[1],atoi(argv[2]));

    //tcpservver.start();
   echoserver = new Echoserver (argv[1],atoi(argv[2]),3,2);
   echoserver->Start();
   




  return 0;
}