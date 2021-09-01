//
//7.4 		WhiteSheep
//
//Program:
// 		简单的echo测试程序
//
//

#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Log/Logging.h"

#include <unistd.h>

using namespace twnl;

const size_t kHeaderLen = 3;

void onConnection(const TcpConnectionPtr& conn) {
	LOG_INFO << "echo" << conn->localAddress().toIpPort() << " -> "
			 << conn->peerAddress().toIpPort() << " is "
			 << (conn->connected() ? "UP" : "DOWN");
}

void onMessage(const TcpConnectionPtr& conn, 
			   Buffer* buf,
			   Timestamp receiveTime) {
	string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() <<  " echo " << msg.size() << " bytes, "
			 << "data received at ";
	conn->send(msg);
}

int main() {
	EventLoop loop;

	InetAddress listenAddr(9999);
	
	TcpServer server(&loop, listenAddr, "echo");
	server.setConnectionCallback(std::bind(&onConnection, _1));
	server.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
	server.setThreadNum(4);
	server.start();
	loop.loop();
}
