//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.9.1
//Description:
#ifndef TWNL_CHANNEL_H
#define TWNL_CHANNEL_H
#include <functional>
#include <memory>

#include "noncopyable.h"
#include "Timestamp.h"

namespace twnl
{
using std::string;
class EventLoop;
class Channel: noncopyable 
{
public:
	typedef std::function<void()> EventCallback;
	typedef std::function<void(Timestamp)> ReadEventCallback;
	enum mode {
		LT,
		ET
	};

	Channel(EventLoop* loop, int fd);
	~Channel();

	void handleEvent(Timestamp receiveTime);
	void setReadCallback(const ReadEventCallback& cb){ 
		readCallBack_ = cb;
	}
	void setWriteCallback(const EventCallback& cb){
		writeCallback_ = cb;
	}
	void setErrorCallback(const EventCallback& cb){
		errorCallback_ = cb;
	}
 	void setCloseCallback(const EventCallback& cb){
		closeCallback_ = cb;
	}	
	
	int fd() const { return fd_; }
	int events() const { return events_;}
	void set_revents(int revt) { revents_ = revt; }
	bool isNoneEvent() const { return events_ == kNoneEvent; }
	
	//监听事件入口
	void enableReading() { events_ |= kReadEvent; update(); }
	void disableReading() { events_ &= ~kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update();}
	void disableWriting() { events_ &= ~kWriteEvent; update();}
	void disableAll() { events_ = kNoneEvent; update(); }
	bool isReading() const { return events_ & kReadEvent; }
	bool isWriting() const {return events_ & kWriteEvent;}
	
	int index() { return index_; }
	void set_index(int idx) { index_ = idx; }

	string reventsToString() const;
	string eventsToString() const;

	EventLoop* ownerloop() { return loop_; }

	void remove();

	static void  initDefaultEvent(mode m);
	static bool isET() { return mode_ == ET;}
private:
	static string eventsToString(int fd, int ev);

	void update();


	static mode mode_;
	static int kNoneEvent;
	static int kReadEvent;
	static int kWriteEvent;
	
	EventLoop* loop_;
	const int fd_;
	int events_;  
	int revents_;
	int index_;
	bool logHup_;              //记录挂起事件
	 
	bool eventHanding_;
	bool addedToLoop_;

	ReadEventCallback readCallBack_;
	EventCallback writeCallback_;
	EventCallback errorCallback_;
	EventCallback closeCallback_;
};

}     

#endif
