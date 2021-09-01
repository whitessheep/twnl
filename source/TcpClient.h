//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef TWNL_TCPCLIENT_H
#define TWNL_TCPCLIENT_H

#include "noncopyable.h"
#include "thread/Mutex.h"
#include "TcpConnection.h"

namespace twnl
{

	class Connector;
	typedef std::shared_ptr<Connector> ConnectorPtr;

	//通过Connector发起连接，之后自己建立连接
	//同时负责断线重新连接
	class TcpClient : noncopyable
	{
	public:
		TcpClient(EventLoop* loop,
			const InetAddress& serverAddr);
		~TcpClient();  

		void connect();
		void disconnect();
		void stop();

		TcpConnectionPtr connection() const
		{
			MutexLockGuard lock(mutex_);
			return connection_;
		}

		bool retry() const;   //负责断开重新连接
		void enableRetry() { retry_ = true; }

		/// Set connection callback.
		/// Not thread safe.
		void setConnectionCallback(const ConnectionCallback& cb)
		{
			connectionCallback_ = cb;
		}

		/// Set message callback.
		/// Not thread safe.
		void setMessageCallback(const MessageCallback& cb)
		{
			messageCallback_ = cb;
		}

		/// Set write complete callback.
		/// Not thread safe.
		void setWriteCompleteCallback(const WriteCompleteCallback& cb)
		{
			writeCompleteCallback_ = cb;
		}

	private:
		void newConnection(int sockfd);
		void removeConnection(const TcpConnectionPtr& conn);

		EventLoop* loop_;
		ConnectorPtr connector_; 
		ConnectionCallback connectionCallback_;
		MessageCallback messageCallback_;
		WriteCompleteCallback writeCompleteCallback_;
		bool retry_;  c
		bool connect_; 
		int nextConnId_;
		mutable MutexLock mutex_;
		TcpConnectionPtr connection_; 
	};

}

#endif //TWNL_TCPCLIENT_H
