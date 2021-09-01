//
//7.1 		WhiteSheep
//
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>

#include "EpollPoller.h"
#include "Log/Logging.h"
#include  "Channel.h"
using namespace twnl;
namespace
{
	const int kNew = -1;
	const int kAdded = 1;
	const int kDeleted = 2;
}


EpollPoller::EpollPoller(EventLoop* loop)
	: Poller(loop),
	epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
	events_(kInitEventListSize)
{
	if (epollfd_ < 0) {
		LOG_FATAL << " EpollPoller::Epollpoller";
	}
}

EpollPoller::~EpollPoller() {
	::close(epollfd_);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
	Poller::assertInLoopThread();
	int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
		static_cast<size_t>(events_.size()),
		timeoutMs);
	int savedErrno = errno;
	Timestamp now(clock::now());

	if (numEvents > 0) {
		LOG_TRACE << numEvents << " events happened";
		fillActiveChannels(numEvents, activeChannels);
		if (static_cast<size_t>(numEvents) == events_.size()) {
			events_.resize(events_.size() * 2);
		}
	}
	else if (numEvents == 0) {
		LOG_TRACE << "nothing happened";
	}
	else {
		if (savedErrno != EINTR) {
			errno = savedErrno;
			LOG_WARN << "EpollPoll::poll()";
		}
	}
	return now;
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
	assert(static_cast<size_t>(numEvents) <= events_.size());
	for (int i = 0; i < numEvents; ++i) {
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
		channel->set_revents(events_[i].events);
		activeChannels->push_back(channel);
	}
}

void EpollPoller::updateChannel(Channel* channel) {
	Poller::assertInLoopThread();
	const int index = channel->index();
	LOG_TRACE << " fd = " << channel->fd()
		<< " events = " << channel->fd()
		<< " index = " << channel->index();

	if (index == kNew || index == kDeleted) {
		int fd = channel->fd();
		if (index == kNew) {
			assert(channels_.find(fd) == channels_.end());
			channels_[fd] = channel;
		}
		else {
			assert(channels_.find(fd) != channels_.end());
			assert(channels_[fd] == channel);
		}
		channel->set_index(kAdded);
		update(EPOLL_CTL_ADD, channel);
	}
	else {
		int fd = channel->fd();
		(void)fd;
		if (channel->isNoneEvent()) {
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kDeleted);
		}
		else {
			update(EPOLL_CTL_MOD, channel);
		}
	}
}


void EpollPoller::removeChannel(Channel* channel) {
	Poller::assertInLoopThread();
	int fd = channel->fd();
	LOG_TRACE << "fd = " << fd;

	int index = channel->index();
	size_t n = channels_.erase(fd);
	(void)n;
	if (index == kAdded) {
		update(EPOLL_CTL_DEL, channel);
	}

	channel->set_index(kDeleted);
}

void EpollPoller::update(int operation, Channel* channel) {
	struct epoll_event event;
	memset(&event, 0, sizeof event);
	event.events = channel->events();
	event.data.ptr = channel;
	int fd = channel->fd();
	LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
		<< " fd = " << fd
		<< " events =  { " << channel->events();

	if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
		if (operation == EPOLL_CTL_DEL) {
			LOG_WARN << "epoll_ctl op = " << operationToString(operation) << " fd = " << fd;
		}
		else {
			LOG_FATAL << "epoll_ctl op = " << operationToString(operation) << "fd = " << fd;
		}

	}
}

const char* EpollPoller::operationToString(int op) {
	switch (op) {
	case EPOLL_CTL_ADD:
		return "ADD";
	case EPOLL_CTL_DEL:
		return "DEL";
	case EPOLL_CTL_MOD:
		return "MOD";
	default:
		assert(false && "ERROR op");
		return "Unknown operation";
	}
}
