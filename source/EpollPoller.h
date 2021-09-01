//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.9.1
//Description:
#ifndef NEET_TOOL_EPOLLPOLLER_H
#define NEET_TOOL_EPOLLPOLLER_H
#include <vector>
#include "Poller.h"

struct epoll_event;

namespace twnl 
{

class EventLoop;

class EpollPoller : public Poller
{
public:
	EpollPoller(EventLoop* loop);
	virtual ~EpollPoller()  override;

	
	virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
	
	virtual void updateChannel(Channel* channel) override;

	virtual void removeChannel(Channel* channel) override;

private:
	static const int kInitEventListSize = 16;
	
	static const char* operationToString(int op);

	void fillActiveChannels(int numEvents, 
							ChannelList* activeChannels) const;
	
	void update(int operation, Channel* channel);

	typedef std::vector<struct epoll_event> EventList;

	int epollfd_;
	EventList events_;
};

} //namespace twnl

#endif  //TWNL_EPOLLPOLLER_H
