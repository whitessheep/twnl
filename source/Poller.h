//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.1
//Description:
//    以poller为基类
#ifndef TWNL_POLLER_H
#define TWNL_POLLER_H
#include <map>
#include <vector>

#include "EventLoop.h"
#include "noncopyable.h"

namespace twnl
{

	class Channel;

	class Poller : noncopyable {
	public:
		typedef std::vector<Channel*> ChannelList;

		Poller(EventLoop* loop);
		virtual ~Poller() = default;

		virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

		virtual void updateChannel(Channel* channel) = 0;

		virtual void removeChannel(Channel* channel) = 0;

		virtual bool hasChannel(Channel* channel) const;

		static Poller* newDefaultPoller(EventLoop* ownerLoop);

		void assertInLoopThread() const
		{
			ownerLoop_->assertInLoopThread();
		}


	protected:

		typedef std::map<int, Channel*> ChannelMap;
		ChannelMap channels_;

	private:
		EventLoop* ownerLoop_;
	};

}   //TWNL_POLLER_H


#endif   //TWNL_POLLER_H
