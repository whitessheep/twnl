//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.1
//Description:
#ifndef TWNL_POLLPOLLER_H
#define TWNL_POLLPOLLER_H
#include "../Poller.h"

#include <vector>

struct pollfd;

namespace twnl
{

	class PollPoller : public Poller
	{
	public:
		PollPoller(EventLoop* loop);
		virtual ~PollPoller()  override;

		virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);

		virtual void updateChannel(Channel* channel) override;
		virtual void removeChannel(Channel* channel) override;

	private:
		void fillActiveChannels(int numEvents,
			ChannelList* activeChannels) const;

		typedef std::vector<pollfd> PollFdList;
		PollFdList pollfds_;

	};
}

#endif  //TWNL_POLLPOLLER_H
