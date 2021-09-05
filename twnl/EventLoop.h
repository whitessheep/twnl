//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.9.1
//Description:
//	处理主要IO逻辑,保存事件信息
//	包括 调用活动事件处理，定时器， 处理线程外IO
//	保存事件分发器，时间处理器
#ifndef TWNL_EVENTLOOP_H
#define TWNL_EVENTLOOP_H

#include "TimerQueue.h"
#include "Callbacks.h"
#include "noncopyable.h"

#include <functional>
#include <vector>
#include <thread>
#include <mutex>

namespace twnl 
{
	
class Poller;
class Channel;
class TimerQueue;
class EPoller;

class EventLoop: noncopyable {
public:

    enum Mode {
        POLL,
        EPOLL,
    };

    enum EpollMode {
        LT,
        ET,
    };

	typedef std::function<void()> Functor;
	EventLoop(Mode mode = EPOLL, EpollMode emode= LT);
	~EventLoop();

	void loop();  //开启事件循环，主逻辑， 主循环
	void quit();

	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	
	Timestamp pollReturnTime() const { return pollReturnTime_; }

    Timer* runAt(Timestamp when, TimerCallback callback);
    Timer* runAfter(Nanosecond interval, TimerCallback callback);
    Timer* runEvery(Nanosecond interval, TimerCallback callback);
    void cancelTimer(Timer* timer); 

	void runInLoop(const Functor& cb); //处理IO逻辑
	void queueInLoop(const Functor& cb);// 转移IO到IO线程
	void wakeup();   //唤醒IO
	
	void assertInLoopThread(){
		if (!isInLoopThread()){
			abortNotInLoopThread();
		}
	}

	bool isInLoopThread() const {
		return threadId_ == std::this_thread::get_id();
	}

private:
	void abortNotInLoopThread();
	void handleRead();
	void doPendingFunctors();  //
	typedef std::vector<Channel*> ChannelList;
 	

	bool looping_;
	bool quit_;
	bool callingPendingFunctors_ ;       //在 doPendingFunctors 中可能会继续有事件加入队列，用来及时wakeup处理事件
	const std::thread::id threadId_;
	Timestamp pollReturnTime_;
	std::unique_ptr<Poller> poller_;
    TimerQueue timerQueue_;
	int wakeupFd_;       //通知处理线程外IO
	std::unique_ptr<Channel> wakeupChannel_;   
	ChannelList activeChannels_;               //活动事件
	std::mutex mutex_;                
	std::vector<Functor> pendingFunctors_;     //IO线程外加入的事件

};

}

#endif
