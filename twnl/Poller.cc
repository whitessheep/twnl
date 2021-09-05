//
//7.1        WhiteSheep
//
//

#include "Poller.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Poller/EpollPoller.h"
#include "Poller/PollPoller.h"
#include <stdlib.h>

using namespace twnl;

Poller::Poller(EventLoop* loop)
	: ownerLoop_(loop)
{ }

bool Poller::hasChannel(Channel* channel) const  {
	assertInLoopThread();
	ChannelMap::const_iterator it = channels_.find(channel->fd());
	return it != channels_.end() && it->second == channel;
}

Poller* Poller::newDefaultPoller(EventLoop* loop) {
	if (::getenv("USE_POLL")) {
		return new PollPoller(loop);
	}
	else {
		return new EpollPoller(loop);
	}
}
