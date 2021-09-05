//
//5.29
//

#include <functional>
#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace twnl;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, 
		 						 const string& name):
	loop_(nullptr),
	name_(name),
	started_(false),
	callback_(cb)
{ }

EventLoopThread::~EventLoopThread(){
	if (started_) {
		if (loop_ != nullptr){
			loop_->quit();
		}
		thread_.join();
	}
}

EventLoop* EventLoopThread::startLoop(){
	assert(!started_);
	started_ = true;
	thread_ = std::thread([this](){threadFunc();});
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while(loop_ == nullptr){
			cond_.wait(lock);
		}
	}
	return loop_;
}

void EventLoopThread::threadFunc(){
	EventLoop loop;
	
	if (callback_) {
		callback_(&loop);
	}

	{
		std::lock_guard<std::mutex> lock(mutex_);
		loop_ = &loop;
		cond_.notify_one();
	}
	loop.loop();
	loop_ = NULL;
}
