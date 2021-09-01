//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef TWNL_TCPCONNECTION_H
#define TWNL_TCPCONNECTION_H

#include "Callbacks.h"
#include "InetAddress.h"
#include "Buffer.h"
#include "noncopyable.h"
#include <boost/any.hpp>
#include <functional>
#include <memory>

namespace twnl
{

	class Channel;
	class EventLoop;
	class Socket;

	//负责与分配的单个用户的交互，保存用户数据及读写数据,关闭连接
	//TcpServer底部调用类
	class TcpConnection : noncopyable,
		public std::enable_shared_from_this<TcpConnection>    //保证this指针运作
	{ 																			//时不会析构
	public: 																	//用于std::bind
		TcpConnection(EventLoop* Loop,
			const std::string& name,
			int sockfd,
			const InetAddress& localAddr,
			const InetAddress& peerAddr);
		~TcpConnection();

		EventLoop* getLoop() const { return loop_; }
		const std::string& name() const { return name_; }
		const InetAddress& localAddress() { return localAddr_; }
		const InetAddress& peerAddress() { return peerAddr_; }

		bool connected() const { return state_ == kConnected; }
		bool disconnected() const { return state_ == kDisconnected; }

		void send(const void* message, size_t len);
		void send(std::string_view message);
		void send(Buffer* message);

		void shutdown();   //主动关闭， 关闭写端
		void forceClose();
		void forceCloseWithDelay(double seconds);

		//NoDelay, 禁用nagle算法
		//Nagle:网络减少拥塞控制， 短字节信息缓冲
		void setTcpNoDelay(bool on);

		void startRead();

		void stopRead();

		bool isReading() const { return reading_; }

		void setContext(const boost::any& context) {
			context_ = context;
		}

		const boost::any& getContext() const {
			return context_;
		}

		boost::any* getMutableContext() {
			return &context_;
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

		void setCloseCallback(const CloseCallback& cb) {
			closeCallback_ = cb;
		}

		void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t hightWaterMark)
		{
			highWaterMarkCallback_ = cb; highWaterMark_ = hightWaterMark;
		}

		void connectEstablished();
		void connectDestroyed();

		Buffer* inputBuffer()
		{
			return &inputBuffer_;
		}

		Buffer* outputBuffer()
		{
			return &outputBuffer_;
		}

	private:
		enum StateE { kConnecting, kConnected, kDisconnected, kDisconnecting, };

		void setState(StateE s) { state_ = s; }
		void handleRead(Timestamp receiveTime);
		void handleWrite();
		//被动关闭，当读到0关闭所有事件
		void handleClose();
		//TcpConnection表，Poller表移除
		//1、channel，关闭监听
		//2、通过closeCallback，调用TcpServer::remove             shared_from_this
		//        TcpConnection中释放                                     +
		//        调用Destory
		//               通过EventLoop:removeChannel从Poller中移除对应的channel 
		void handleError();
		void sendInLoop(const void* data, size_t len);
		void shutdownInLoop();
		void forceCloseInLoop();
		void startReadInLoop();
		void stopReadInLoop();

		EventLoop* loop_;
		std::string name_;
		StateE state_;
		bool reading_;
		std::unique_ptr<Socket> socket_;
		std::unique_ptr<Channel> channel_;
		const InetAddress localAddr_;
		const InetAddress peerAddr_;
		ConnectionCallback connectionCallback_;
		MessageCallback messageCallback_;
		WriteCompleteCallback writeCompleteCallback_;
		HighWaterMarkCallback highWaterMarkCallback_;
		CloseCallback closeCallback_;
		size_t highWaterMark_;
		Buffer inputBuffer_;
		Buffer outputBuffer_;
		boost::any context_;
	};

	typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
}
#endif //TWNL_TCPCONNECTION_H
