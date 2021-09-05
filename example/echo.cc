#include <unistd.h>
#include <string>

#include "twnl/Log/Logging.h"
#include "twnl/EventLoop.h"
#include "twnl/TcpServer.h"

using namespace twnl;
using namespace std;
class EchoServer
{
public:

	EchoServer(EventLoop* loop,
			   const InetAddress& listenAddr,
			   const std::string& name,
               const int numThread = 1) 
        : server_(loop, listenAddr, name){
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
        server_.setThreadNum(numThread);
    }

	void start() {
        server_.start();
    }


private:

	void onConnection(const TcpConnectionPtr& conn) {
        LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
                 << conn->localAddress().toIpPort() << " is "
                 << (conn->connected() ? "UP" : "DOWN");
    }
	void onMessage(const TcpConnectionPtr& conn,
				   Buffer* buf,
				   Timestamp time) {
        std::string msg(buf->retrieveAllAsString());
        LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
                 << "data received at "<< clock::toString(time);
        conn->send(msg);
    }

	TcpServer server_;

};
int main(){
	EventLoop loop;
	InetAddress listenAddr(12345);
	EchoServer server(&loop, listenAddr, "EchoServer");
	server.start();
	loop.loop();
}
