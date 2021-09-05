#include <assert.h>
#include <sys/eventfd.h>

#include "EventLoop.h"
#include "Log/Logging.h"
#include "Channel.h"
#include "Poller.h"
#include "SocketsOps.h"
#include "signal.h"

using namespace twnl;
__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

static int createEventfd() {
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0) {
		LOG_FATAL << "Failed in eventfd";
	}
	return evtfd;
}

class IgnoreSigPipe {
public:
	IgnoreSigPipe() {
		::signal(SIGPIPE, SIG_IGN);
	}
};

IgnoreSigPipe initobj;

EventLoop::EventLoop() :
	looping_(false),
	quit_(false),
	callingPendingFunctors_(false),
	threadId_(std::this_thread::get_id()),
	poller_(Poller::newDefaultPoller(this)),
	timerQueue_(this),
	wakeupFd_(createEventfd()),
	wakeupChannel_(new Channel(this, wakeupFd_))
{
	LOG_DEBUG << "EventLoop created" << this << " in thread " << threadId_;
	if (t_loopInThisThread) {
		LOG_FATAL << "Another EventLoop" << t_loopInThisThread
			<< " exist in this thread " << threadId_;
	}
	else {
		t_loopInThisThread = this;
	}
	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
	wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
	assert(!looping_);
	t_loopInThisThread = NULL;
}

void EventLoop::quit() {
	quit_ = true;
	if (!isInLoopThread()) {
		wakeup();
	}
}

void EventLoop::updateChannel(Channel* channel) {
	assert(channel->ownerloop() == this);
	assertInLoopThread();
	poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
	assert(channel->ownerloop() == this);
	assertInLoopThread();
	poller_->removeChannel(channel);
}

Timer* EventLoop::runAt(Timestamp when, TimerCallback callback)
{
	return timerQueue_.addTimer(std::move(callback), when, Millisecond::zero());
}

Timer* EventLoop::runAfter(Nanosecond interval, TimerCallback callback)
{
	return runAt(clock::now() + interval, std::move(callback));
}

Timer* EventLoop::runEvery(Nanosecond interval, TimerCallback callback)
{
	return timerQueue_.addTimer(std::move(callback),
		clock::now() + interval,
		interval);
}

void EventLoop::cancelTimer(Timer* timer)
{
	timerQueue_.cancelTimer(timer);
}

void EventLoop::loop() {
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;

	while (!quit_) {
		activeChannels_.clear();
		pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
		for (ChannelList::iterator it = activeChannels_.begin();
			it != activeChannels_.end();
			++it) {
			(*it)->handleEvent(pollReturnTime_);
		}
		doPendingFunctors();
	}
	LOG_TRACE << "EventLoop " << this << " stop looping";
	looping_ = false;
}

void EventLoop::runInLoop(const Functor& cb) {
	if (isInLoopThread()) {
		cb();
	}
	else {
		LOG_DEBUG << "NotInLoopThread joining ioLoop";
		queueInLoop(cb);
	}
}

void EventLoop::queueInLoop(const Functor& cb) {
	{
		std::lock_guard<std::mutex> lock(mutex_);
		pendingFunctors_.push_back(cb);
	}
	if (!isInLoopThread() || callingPendingFunctors_) {
		wakeup();
	}
}

void EventLoop::wakeup() {
	uint64_t one = 1;
	ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
	if (n != sizeof one) {
		LOG_ERROR << " EventLoop " << n << "bytes instead of 8";
	}
}

void EventLoop::handleRead() {
	uint64_t  one = 1;
	ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
	if (n != sizeof one) {
		LOG_ERROR << " EventLoop::handleRead reads " << n << "bytes instead of 8";
	}
}

void EventLoop::doPendingFunctors() {
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;

	{
		std::lock_guard<std::mutex> lock(mutex_);
		functors.swap(pendingFunctors_);
	}

	for (size_t i = 0; i < functors.size(); ++i) {
		functors[i]();
	}

	callingPendingFunctors_ = false;
}
void EventLoop::abortNotInLoopThread() {
	LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
		<< " was created in threadId_ = " << threadId_
		<< ", current thread id = " << std::this_thread::get_id();
}

