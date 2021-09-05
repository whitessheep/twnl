//
//5.29
//

#include "TcpServer.h"

#include "Log/Logging.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "EventLoopThreadPool.h"

#include <functional>
#include <stdio.h>

using namespace twnl;

TcpServer::TcpServer(EventLoop* loop,
	const InetAddress& listenAddr,
	const string& nameArg,
	EventMode mode,
	Option option)
	: loop_(loop),
	name_(listenAddr.toIpPort()),
	acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
	threadPool_(new EventLoopThreadPool(loop, nameArg)),
	started_(false),
	nextConnId_(1)
{
	Channel::initDefaultEvent(mode == LT ? Channel::LT : Channel::ET);
	acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
	loop_->assertInLoopThread();
	LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";    //!

	for (auto& item : connections_) {
		TcpConnectionPtr conn(item.second);
		item.second.reset();
		conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
	}
}

void TcpServer::setThreadNum(int numThreads) {
	assert(0 <= numThreads);
	threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
	if (!started_.exchange(true)) {
		threadPool_->start(threadInitCallback_);
		assert(!acceptor_->listenning());
		acceptor_->listen();
		LOG_INFO << "Server starts listenning on " << name_;
	}

}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
	loop_->assertInLoopThread();
	EventLoop* ioLoop = threadPool_->getNextLoop();

	char buf[64];
	snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);

	++nextConnId_;
	std::string connName = name_ + buf;

	LOG_INFO << "TcpServer::newConnection [" << name_
		<< "] - new connection [" << connName
		<< "] from " << peerAddr.toIpPort();

	InetAddress localAddr(sockets::getLocalAddr(sockfd));

	TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
	connections_[connName] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
	ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
	LOG_DEBUG << "TcpServer::newConnection end";
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
	loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
	loop_->assertInLoopThread();
	LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
		<< "] - connection " << conn->name();
	size_t n = connections_.erase(conn->name());
	assert(n = 1); (void)n;
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));

}
