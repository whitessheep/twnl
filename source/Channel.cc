//
//5.27
//
#include <sys/epoll.h>
#include <sstream>

#include "Channel.h"
#include "EventLoop.h"
#include "Log/Logging.h"

using namespace twnl;

Channel::mode Channel::mode_;
int Channel::kNoneEvent;
int Channel::kReadEvent;
int Channel::kWriteEvent;

void Channel::initDefaultEvent(mode m) {
	if (m == LT) {
		mode_ = LT;
		kNoneEvent = 0;
		kReadEvent = EPOLLIN | EPOLLPRI;
		kWriteEvent = EPOLLOUT;
	}
	else {
		mode_ = ET;
		kNoneEvent = 0;
		kReadEvent = EPOLLET | EPOLLIN | EPOLLPRI;
		kWriteEvent = EPOLLET | EPOLLOUT;
	}
}


Channel::Channel(EventLoop* loop, int fdarg):
	loop_(loop),
	fd_(fdarg),
	events_(0),
	revents_(0),
	index_(-1),
	logHup_(true),
	eventHanding_(false),
	addedToLoop_(false)
{ }
 
Channel::~Channel(){
	assert(!eventHanding_);
}

void Channel::update(){
	addedToLoop_ = true;
	loop_->updateChannel(this);
}

void Channel::remove() {
	assert(isNoneEvent());
	addedToLoop_ = false;
	loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
	eventHanding_ = true;
	LOG_TRACE << reventsToString();
	
	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
		if (logHup_) {
			LOG_TRACE << " fd = " << fd_ << "Channel::handleEvent POLLHUP";
		}
		if (closeCallback_) closeCallback_();
	}
	if (revents_ & (EPOLLERR)) {
		if (errorCallback_) errorCallback_();
	}
	if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
		if (readCallBack_) readCallBack_(receiveTime);
	}
	if (revents_ & EPOLLOUT) {
		if (writeCallback_) writeCallback_();
	}
	eventHanding_ = false;
}

string Channel::reventsToString() const {
	return eventsToString(fd_, revents_);
}

string Channel::eventsToString() const {
	return eventsToString(fd_, events_);
}

string Channel::eventsToString(int fd, int ev) {
	std::ostringstream oss;
	oss << fd << ": ";
	if (ev & EPOLLIN) {
		oss << "IN ";
	}
	if (ev & EPOLLPRI) {
		oss << "PRI ";
	}
	if (ev & EPOLLOUT) {
		oss << "OUT ";
	}
	if (ev & EPOLLHUP) {
		oss << "HUP " ;
	}
	if (ev & EPOLLRDHUP) {
		oss << "RDHUP ";
	}
	if (ev & EPOLLERR) {
		oss << "ERR ";
	}

	return oss.str();
}

