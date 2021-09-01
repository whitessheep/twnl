//
//7.2 		WhiteSheep
//
#include "Poller.h"
#include "EpollPoller.h"
#include "PollPoller.h"

#include <stdlib.h>

using namespace twnl;

Poller* Poller::newDefaultPoller(EventLoop* loop) {
	if (::getenv("USE_POLL")) {
		return new PollPoller(loop);
	}
	else {
		return new EpollPoller(loop);
	}
}

