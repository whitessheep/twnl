//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.1
//Description:
//socket底端封装类，保存所有可以的socket操作
#ifndef TWNL_SOCKET_H
#define TWNL_TOOL_SOCKET_H 

#include "noncopyable.h"

namespace twnl
{

class InetAddress;


class Socket
{
public:
	explicit Socket(int sockfd):
		sockfd_(sockfd)
	{}
	
	~Socket();

	int fd() const { return sockfd_; }

	void bindAddress( const InetAddress& localaddr);
	void listen();

	int accept(InetAddress* peeraddr);

	void setReuseAddr(bool on);

	void setTcpNoDelay(bool on);

	void setReusePort(bool on);

	void setKeepAlive(bool on);
	
	void shutdownWrite();
	
private:
	const int sockfd_;
};

}

#endif
