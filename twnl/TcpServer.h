//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef TWNL_TCPSERVER_H
#define TWNL_TCPSERVER_H
#include <map>
#include <functional>
#include <atomic>
#include <string>

#include "Callbacks.h"
#include "TcpConnection.h"
#include "noncopyable.h"

namespace twnl
{

	class Acceptor;
	class EventLoop;
	class EventLoopThreadPool;

	//整体数据
	//上层应用，发起监听，修饰channel
	//同时保存所有用户数据
	//用锁控制Tcpserver的启动
	class TcpServer : noncopyable
	{
	public:

		typedef std::function<void(EventLoop*)> ThreadInitCallback;

		enum Option {
			kNoReusePort,
			kReusePort,
		};
		enum EventMode {
			LT,
			ET,
		};

		TcpServer(EventLoop* loop,
			const InetAddress& listenAddr,
			const std::string& nameArg,
			EventMode mode = LT,
			Option option = kNoReusePort);
		~TcpServer();

		const std::string& ipPort() const { return ipPort_; }
		const std::string& name() const { return name_; }
		EventLoop* getLoop() const { return loop_; }
		std::shared_ptr<EventLoopThreadPool> threadPool()
		{
			return threadPool_;
		}

		void setThreadNum(int numThreads);

		void start();   //建立环境，开启多线程

		void setThreadInitCallback(const ThreadInitCallback& cb)
		{
			threadInitCallback_ = cb;
		}

		void setConnectionCallback(const ConnectionCallback& cb) {
			connectionCallback_ = cb;
		}

		void setMessageCallback(const MessageCallback& cb) {
			messageCallback_ = cb;
		}

		void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
			writeCompleteCallback_ = cb;
		}
	private:
		void newConnection(int sockfd, const InetAddress& peerAddr);
		void removeConnection(const TcpConnectionPtr& conn);
		void removeConnectionInLoop(const TcpConnectionPtr& conn);

		typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

		EventLoop* loop_;
		const std::string ipPort_;
		const std::string name_;
		std::unique_ptr<Acceptor> acceptor_;
		std::shared_ptr<EventLoopThreadPool> threadPool_;
		ConnectionCallback connectionCallback_;
		MessageCallback messageCallback_;
		WriteCompleteCallback writeCompleteCallback_;
		ThreadInitCallback threadInitCallback_;
		std::atomic<bool> started_;
		int nextConnId_;
		ConnectionMap connections_;

	};
}


#endif //TWNL_TCPSERVER_H
