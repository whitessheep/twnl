//
//5.27
//
#include <poll.h>
#include <sstream>

#include "Channel.h"
#include "EventLoop.h"
#include "Log/Logging.h"

using namespace twnl;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fdarg):
	loop_(loop),
	fd_(fdarg),
	events_(0),
	revents_(0),
	index_(-1),
	logHup_(true),
	tied_(false),
	eventHanding_(false),
	addedToLoop_(false)
{ }
 
Channel::~Channel(){
	assert(!eventHanding_);
}

void Channel::tie(const std::shared_ptr<void>& obj) {
	tie_ = obj;
	tied_ = true;
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

// void Channel::handleEvent(Timestamp receieTime){
//     eventHanding_ = true;
//     if (revents_ & POLLNVAL){
//         LOG_WARN << " Channel::handleEvent() POLLNVAL";
//     }
//
//     if ((revents_ & POLLHUP) && !(revents_ & POLLIN)){
//         LOG_WARN << "Channel::handleEvent() POLLHUP";
//         if (closeCallback_) closeCallback_();
//     }
//
//     if (revents_ & (POLLERR | POLLNVAL)){
//         if (errorCallback_) errorCallback_();
//     }
//
//     if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)){
//         if (readCallBack_) readCallBack_(receiveTime);
//     }
//
//     if (revents_ & (POLLOUT)){
//         if (writeCallback_) writeCallback_();
//     }
//     eventHanding_ = false;
// }

void Channel::handleEvent(Timestamp receiveTime) {
	std::shared_ptr<void> guard;
	if (tied_) {
		guard = tie_.lock();
		if (guard) {
			handleEventWithGuard(receiveTime);
		}
	}
	else {
		handleEventWithGuard(receiveTime);
	}
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
	eventHanding_ = true;
	LOG_TRACE << reventsToString();
	
	if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
		if (logHup_) {
			LOG_TRACE << " fd = " << fd_ << "Channel::handleEvent POLLHUP";
		}
		if (closeCallback_) closeCallback_();
	}
	if (revents_ & POLLNVAL) {
		LOG_WARN << " fd = " << fd_ << "Channel::handleEvent POLLNVAL";
	}
	if (revents_ & (POLLNVAL | POLLERR)) {
		if (errorCallback_) errorCallback_();
	}
	if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
		if (readCallBack_) readCallBack_(receiveTime);
	}
	if (revents_ & POLLOUT) {
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
	if (ev & POLLIN) {
		oss << "IN ";
	}
	if (ev & POLLPRI) {
		oss << "PRI ";
	}
	if (ev & POLLOUT) {
		oss << "OUT ";
	}
	if (ev & POLLHUP) {
		oss << "HUP " ;
	}
	if (ev & POLLRDHUP) {
		oss << "RDHUP ";
	}
	if (ev & POLLERR) {
		oss << "ERR ";
	}
	if (ev & POLLNVAL) {
		oss << "NVAL ";
	}

	return oss.str();
}

