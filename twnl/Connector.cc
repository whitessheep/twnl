//
//6.3
//
#include <error.h>

#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "Log/Logging.h"

using namespace twnl;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr) :
	loop_(loop),
	serverAddr_(serverAddr),
	connect_(false),
	state_(kDisconnected),
	retryDelayMs_(kInitRetryDelayMs) {
	LOG_DEBUG << "ctor [" << this << "]";
}

Connector::~Connector() {
	LOG_DEBUG << "ctor [" << this << "]";
	loop_->cancelTimer(timer_);
	assert(!channel_);
}
void Connector::start() {
	connect_ = true;
	loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop() {
	loop_->assertInLoopThread();
	assert(state_ == kDisconnected);
	if (connect_) {
		connect();
	}
	else {
		LOG_DEBUG << "do not connect";
	}
}

void Connector::connect() {
	int sockfd = sockets::createNonblockingOrDie(serverAddr_.family());
	int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
	int savedErrno = (ret == 0) ? 0 : errno;
	switch (savedErrno) {
	case 0:
	case EINPROGRESS:
	case EINTR:
	case EISCONN:
		connecting(sockfd);
		break;

	case EAGAIN:
	case EADDRINUSE:
	case EADDRNOTAVAIL:
	case ECONNREFUSED:
	case ENETUNREACH:
		retry(sockfd);
		break;

	case EACCES:
	case EPERM:
	case EAFNOSUPPORT:
	case EALREADY:
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
		LOG_WARN << "connect error in Connector::startInLoop " << savedErrno;
		sockets::close(sockfd);
		break;

	default:
		LOG_WARN << "Unexpected error in Connector::startInLoop " << savedErrno;
		sockets::close(sockfd);
		break;
	}
}

void Connector::restart() {
	loop_->assertInLoopThread();
	setState(kDisconnected);
	retryDelayMs_ = kInitRetryDelayMs;
	connect_ = true;
	startInLoop();
}

//取消重新连接
void Connector::stop() {
	connect_ = false;
	loop_->cancelTimer(timer_);
}


void Connector::connecting(int sockfd) {
	setState(kConnecting);
	assert(!channel_);
	channel_.reset(new Channel(loop_, sockfd));
	channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
	channel_->setErrorCallback(std::bind(&Connector::handleError, this));
	channel_->enableWriting();
}

//移除事件
int  Connector::removeAndResetChannel() {
	channel_->disableAll();
	loop_->removeChannel(channel_.get());
	int sockfd = channel_->fd();
	loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
	return sockfd;
}

//移除连接
void Connector::resetChannel() {
	channel_.reset();
}

//只负责建立连接，如果监听到socket可写，则注销自己的channel
//同时把sock交由上层 
void Connector::handleWrite() {
	LOG_TRACE << "Connector::handleWrite " << state_;

	if (state_ == kConnecting) {
		int sockfd = removeAndResetChannel();
		int err = sockets::getSocketError(sockfd);
		if (err) {
			LOG_WARN << "Connector::hanldeWrite SO_ERROR"
				<< err << " " << strerror_tl(err);
			retry(sockfd);
		}
		else if (sockets::isSelfConnect(sockfd)) {
			LOG_WARN << "Connector::handleWrite - self connect";
			retry(sockfd);
		}
		else {
			setState(kConnected);
			if (connect_) {
				newConnectionCallback_(sockfd);
			}
			else {
				sockets::close(sockfd);
			}

		}
	}
	else {
		assert(state_ == kDisconnected);
	}
}

void Connector::handleError()
{
	LOG_ERROR << "Connector::handleError";
	assert(state_ == kConnecting);

	int sockfd = removeAndResetChannel();
	int err = sockets::getSocketError(sockfd);
	LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
	retry(sockfd);
}

void Connector::retry(int sockfd) {
	sockets::close(sockfd);
	setState(kDisconnected);
	if (connect_) {
		LOG_INFO << "Connector::retry - Retry connection to "
			<< serverAddr_.toIpPort() << " in "
			<< retryDelayMs_ << " milliseconds. ";
		timer_ = loop_->runAfter(Millisecond(retryDelayMs_), std::bind(&Connector::startInLoop, this));
		retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);

	}
	else {
		LOG_DEBUG << "do not connect ";
	}
}
