//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.9.1
//Description:
//	监听socket，保存监听socket, channel,
#ifndef TWNL_ACCEPROT_H
#define TWNL_ACCEPROT_H
#include <functional>

#include "Socket.h"
#include "Channel.h"
#include "noncopyable.h"

namespace twnl
{

	class EventLoop;
	class InetAddress;
	class Acceptor : noncopyable
	{
	public:
		typedef std::function<void(int sockfd, const InetAddress&)> NewConnectonCallback;

		Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);

		void setNewConnectionCallback(const NewConnectonCallback& cb) {
			newConnectionCallback_ = cb;
		}

		bool listenning() const { return listenning_; }
		void listen();

	private:
		void handleRead();

		EventLoop* loop_;
		Socket acceptSocket_;
		Channel acceptChannel_;
		NewConnectonCallback newConnectionCallback_;
		bool listenning_;
		int idleFd_;
	};
}

#endif
