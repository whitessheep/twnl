//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.9.1
//Description:
#ifndef TWNL_CALLBACKS_H
#define TWNL_CALLBACKS_H

#include <functional>
#include <memory>
#include "Timestamp.h"

namespace twnl
{

	using std::placeholders::_1;
	using std::placeholders::_2;
	using std::placeholders::_3;

	template<typename T>
	inline T* get_pointer(const std::shared_ptr<T>& ptr) {
		return ptr.get();
	}
	template<typename T>
	inline T* get_pointer(const std::unique_ptr<T>& ptr) {
		return ptr.get();
	}
	class Buffer;
	class TcpConnection;
	typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

	typedef std::function<void()> TimerCallback;
	typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
	typedef std::function<void(const TcpConnectionPtr&, Buffer* buf, Timestamp)> MessageCallback;
	typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
	typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
	typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

	void defaultConnectionCallback(const TcpConnectionPtr& conn);
	void defaultMessageCallback(const TcpConnectionPtr& conn,
		Buffer* buffer,
		Timestamp receiveTime);

}

#endif
