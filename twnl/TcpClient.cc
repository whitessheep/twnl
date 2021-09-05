//
//6.3
//
#include "TcpClient.h"
#include "Connector.h"         
#include "EventLoop.h"         
#include "SocketsOps.h"

#include "Log/Logging.h"
#include <stdio.h>  // snprintf

using namespace twnl;

namespace 
{

    void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
    {
        loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }

}

TcpClient::TcpClient(EventLoop* loop,
	const InetAddress& serverAddr)
	: loop_(loop),
	connector_(new Connector(loop, serverAddr)),
	connectionCallback_(defaultConnectionCallback),
	messageCallback_(defaultMessageCallback),
	retry_(false),
	connect_(true),
	nextConnId_(1)
{
	connector_->setNewConnectionCallback(
		std::bind(&TcpClient::newConnection, this, _1));
	LOG_INFO << "TcpClient::TcpClient[" << this
		<< "] - connector " << connector_.get();
}

TcpClient::~TcpClient()
{
	LOG_INFO << "TcpClient::~TcpClient[" << this
		<< "] - connector " << connector_.get();
	TcpConnectionPtr conn;
	{
        std::lock_guard<std::mutex> lock(mutex_);
		conn = connection_;
	}
	if (conn)
	{
		CloseCallback cb = std::bind(&::removeConnection, loop_, _1);
		loop_->runInLoop(
			std::bind(&TcpConnection::setCloseCallback, conn, cb));
	}
	else
	{
		connector_->stop();
	}
}

void TcpClient::connect()
{
	LOG_INFO << "TcpClient::connect[" << this << "] - connecting to "
		<< connector_->serverAddress().toIpPort();
	connect_ = true;
	connector_->start();
}

void TcpClient::disconnect()
{
	connect_ = false;

	{
        std::lock_guard<std::mutex> lock(mutex_);
		if (connection_)
		{
			connection_->shutdown();
		}
	}
}

void TcpClient::stop()
{
	connect_ = false;
	connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
	loop_->assertInLoopThread();
	InetAddress peerAddr(sockets::getPeerAddr(sockfd));
	char buf[32];
	snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
	++nextConnId_;
	string connName = buf;

	InetAddress localAddr(sockets::getLocalAddr(sockfd));
	TcpConnectionPtr conn(new TcpConnection(loop_,
		connName,
		sockfd,
		localAddr,
		peerAddr));

	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(
		std::bind(&TcpClient::removeConnection, this, _1)); 
	{
        std::lock_guard<std::mutex> lock(mutex_);
		connection_ = conn;
	}
	conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
	loop_->assertInLoopThread();
	assert(loop_ == conn->getLoop());

	{
        std::lock_guard<std::mutex> lock(mutex_);
		assert(connection_ == conn);
		connection_.reset();
	}

	loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
	if (retry_ && connect_)
	{
		LOG_INFO << "TcpClient::connect[" << this << "] - Reconnecting to "
			<< connector_->serverAddress().toIpPort();
		connector_->restart();
	}
}

