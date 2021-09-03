//
//5.29
//
#include <functional>
#include <errno.h>
#include <stdio.h>

#include "TcpConnection.h"
#include "Log/Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"

using namespace twnl;

void twnl::defaultConnectionCallback(const TcpConnectionPtr& conn) {
	LOG_TRACE << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");
}

void twnl::defaultMessageCallback(const TcpConnectionPtr&,
	Buffer* buf,
	Timestamp) {
	buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop,
	const std::string& nameArg,
	int sockfd,
	const InetAddress& localAddr,
	const InetAddress& peerAddr) :
	loop_(loop),
	name_(nameArg),
	state_(kConnecting),
	reading_(true),
	socket_(new Socket(sockfd)),
	channel_(new Channel(loop, sockfd)),
	localAddr_(localAddr),
	peerAddr_(peerAddr),
	highWaterMark_(64 * 1024 * 1024)
{
	channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
	channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
	LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
		<< "fd =" << sockfd;
	socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
	LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
		<< " fd=" << channel_->fd();
}

void TcpConnection::shutdown() {
	if (state_ == kConnected) {
		setState(kDisconnecting);
		loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::shutdownInLoop() {
	loop_->assertInLoopThread();
	if (!channel_->isWriting()) {
		socket_->shutdownWrite();
	}
}

void TcpConnection::forceClose() {
	if (state_ == kConnected || state_ == kDisconnecting) {
		setState(kDisconnecting);
		loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
	}
}


void TcpConnection::forceCloseInLoop() {
	loop_->assertInLoopThread();
	if (state_ == kDisconnecting || state_ == kConnected) {
		handleClose();
	}
}

void TcpConnection::connectEstablished() {
	loop_->assertInLoopThread();
	assert(state_ == kConnecting);
	setState(kConnected);
	channel_->enableReading();

	connectionCallback_(shared_from_this());
}


void TcpConnection::setTcpNoDelay(bool on) {
	socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead() {
	loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop() {
	loop_->assertInLoopThread();
	if (!reading_ || !channel_->isReading()) {
		channel_->enableReading();
		reading_ = true;
	}
}

void TcpConnection::stopRead() {
	loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop() {
	loop_->assertInLoopThread();
	if (reading_ || channel_->isReading()) {
		channel_->disableReading();
		reading_ = false;
	}
}


void TcpConnection::send(const void* data, size_t len) {
	if (state_ == kConnected) {
		loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, data, len));
	}
}

void TcpConnection::send(std::string_view message) {
	send(message.data(), message.size());

}

void TcpConnection::send(Buffer* buf) {
	sendInLoop(buf->peek(), buf->readableBytes());
	buf->retrieveAll();
}


void TcpConnection::sendInLoop(const void* data, size_t len) {
	loop_->assertInLoopThread();
	ssize_t nwrote = 0;
	size_t remaining = len;
	bool faultError = false;

	if (state_ == kDisconnected) {
		LOG_WARN << "disconnected, give up writing";
		return;
	}

	//若已在发送数据则不发送，防止乱序
	if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
		nwrote = sockets::write(channel_->fd(), data, len);
		if (nwrote >= 0) {
			remaining = len - nwrote;

			//当无数据时调用Callback
			if (remaining == 0 && writeCompleteCallback_) {
				loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));  //保证TcpConnection存在
			}
		}
		else {
			nwrote = 0;
			if (errno != EWOULDBLOCK) {
				LOG_WARN << "TcpConnection::sendInLoop";
				if (errno == EPIPE || errno == ECONNRESET) {
					faultError = true;
				}
			}
		}

		assert(remaining <= len);

		//把剩余数据放入缓冲，由handleWriting发送
		if (!faultError && remaining > 0) {
			size_t oldLen = outputBuffer_.readableBytes();
			if (oldLen + remaining >= highWaterMark_ &&
				oldLen < highWaterMark_ &&
				highWaterMarkCallback_) {
				loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
			}
			outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
			if (!channel_->isWriting()) {
				channel_->enableWriting();
			}

		}

	}
}



void TcpConnection::handleRead(Timestamp receiveTime) {
	int savedErrno = 0;
	ssize_t n;
	if (channel_->isET()) {
		n = inputBuffer_.readFdET(channel_->fd(), &savedErrno);
	}
	else {
		n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
	}
	if (n > 0) {
		messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
	}
	else if (n == 0) {
		handleClose();
	}
	else {
		errno = savedErrno;
		LOG_WARN << "TcpConnection::handleRead";
		handleError();
	}
}

void TcpConnection::handleWrite() {
	loop_->assertInLoopThread();
	if (channel_->isWriting()) {
		int savedErrno = 0;
		ssize_t n;
		if (channel_->isET()) {
			n = outputBuffer_.writeFdET(channel_->fd(), &savedErrno);
		}
		else {
			n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);
		}
		if (n > 0) {
			if (outputBuffer_.readableBytes() == 0) {
				channel_->disableWriting();         //写完毕
				if (writeCompleteCallback_) {
					loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
				}
				if (state_ == kDisconnecting) {    //发送完数据，重新开始关闭 
					shutdownInLoop();
				}
			}
			else {
				LOG_TRACE << "I am going to write more data";
			}

		}
		else {
			LOG_WARN << "TcpConnection::handleWrite";
		}
	}
	else {
		LOG_TRACE << "Conneciont is down, no more writing";
	}

}
void TcpConnection::handleClose() {
	loop_->assertInLoopThread();
	LOG_TRACE << "TcpConnection::handleClose state = " << state_;
	assert(state_ == kConnected || state_ == kDisconnecting);
	setState(kDisconnected);
	channel_->disableAll();

	TcpConnectionPtr guardThis(shared_from_this());
	connectionCallback_(guardThis);
	closeCallback_(guardThis);

}

void TcpConnection::handleError() {
	int err = sockets::getSocketError(channel_->fd());
	LOG_ERROR << "TcpConnection::handleError [" << name_
		<< "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::connectDestroyed() {
	loop_->assertInLoopThread();
	if (state_ == kConnected) {
		setState(kDisconnected);
		channel_->disableAll();
		connectionCallback_(shared_from_this());
	}
	channel_->remove();
}
















