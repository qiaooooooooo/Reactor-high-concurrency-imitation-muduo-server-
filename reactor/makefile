all:client tcpepoll

client:client.cpp
	g++ -g -o client client.cpp
tcpepoll:tcpepoll.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp Eventloop.cpp Tcpserver.cpp Acceptor.cpp Connection.cpp Buffer.cpp Echoserver.cpp ThreadPool.cpp Timestamp.cpp
	g++ -g -o tcpepoll tcpepoll.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp Eventloop.cpp Tcpserver.cpp  Acceptor.cpp Connection.cpp Buffer.cpp Echoserver.cpp  Timestamp.cpp ThreadPool.cpp -lpthread

clean:
	rm -f client tcpepoll
