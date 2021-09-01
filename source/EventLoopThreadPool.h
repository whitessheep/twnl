//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.9.1
//Description:
//	  创建多个EventLoopThread
#ifndef TWNL_EVENTLOOPTHREADPOOL_H
#define TWNL_EVENTLOOPTHREADPOOL_H
#include <vector>
#include <functional>
#include <string>
#include <memory>

#include "noncopyable.h"

namespace twnl
{

	class EventLoop;
	class EventLoopThread;

	
	class EventLoopThreadPool : noncopyable
	{
	public:
		typedef std::function<void(EventLoop*)> ThreadInitCallback;

		EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
		~EventLoopThreadPool();

		bool started() const { return started_; }
		const std::string& name() const { return name_; }

		void setThreadNum(int numThreads) { numThreads_ = numThreads; }

		void start(const ThreadInitCallback& cb = ThreadInitCallback());

		EventLoop* getNextLoop();
		std::vector<EventLoop*> getAllLoops();
		EventLoop* getLoopForHash(size_t hashCode);




	private:
		EventLoop* baseLoop_;
		std::string name_;
		bool started_;
		int numThreads_;
		int next_;  // always in loop thread
		std::vector<std::unique_ptr<EventLoopThread>> threads_;
		std::vector<EventLoop*> loops_;
	};


}

#endif //TWNL_EVENTLOOPTHREADPOOL_H
