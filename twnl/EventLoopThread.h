//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.9.1
//Description:
//	生成一个loop线程
//	在生成销毁EventLoop时加锁   3处
#ifndef NET_TOOL_EVENTLOOPTHREAD_H
#define NET_TOOL_EVENTLOOPTHREAD_H
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>

#include "noncopyable.h"
namespace twnl
{

using std::string;
class EventLoop;

class EventLoopThread: noncopyable   
{
public:
	typedef std::function<void(EventLoop*)> ThreadInitCallback;

	EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
			        const string& name = string());

	~EventLoopThread();
	EventLoop* startLoop();

private:
	void threadFunc();

	EventLoop* loop_;  
	string name_;
	bool started_;
	std::thread thread_;
	std::mutex mutex_;
	std::condition_variable cond_;
	ThreadInitCallback callback_; 
};
}
#endif  // NET_TOOL_EVENTLOOPTHREAD_H
