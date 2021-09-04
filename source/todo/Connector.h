//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.9.1
//Description:
#ifndef TWNL_CONNECTOR_H
#define TWNL_CONNECTOR_H
#include <memory>
#include <functional>

#include "InetAddress.h"
#include "noncopyable.h"

namespace twnl
{
class Channel;
class EventLoop;

class Connector: noncopyable,
				 public std::enable_shared_from_this<Connector>
{
public:
	typedef std::function<void (int sockfd)> NewConnectionCallback;
	
	Connector(EventLoop* loop, const InetAddress& serverAddr_);
	~Connector();

	void setNewConnectionCallback(const NewConnectionCallback& cb){
		newConnectionCallback_ = cb;
	}

	void start();  //开始连接对外接口
	void restart();  //connector 重置
	void stop();

	const InetAddress& serverAddress() const { return serverAddr_; }
	
private:
	enum States {kDisconnected, kConnecting, kConnected};
	static const int kMaxRetryDelayMs = 30*1000;
	static const int kInitRetryDelayMs = 50;
	
	void setState(States s){
		state_ = s;
	}
	void startInLoop();
	void connect();      //连接主逻辑
	void connecting(int sockfd);  //底层连接调用
	void handleWrite();
	void handleError();
	void retry(int sockfd);       //底层连接调用
	int removeAndResetChannel();
	void resetChannel();


	EventLoop* loop_;
	InetAddress serverAddr_;
	bool connect_;
	States state_;
	std::unique_ptr<Channel> channel_;
	NewConnectionCallback newConnectionCallback_;
	int retryDelayMs_;
	TimerId timerId_;
};

typedef std::shared_ptr<Connector> ConnectorPtr;
}

#endif //TWNL_CONNECTOR_H
