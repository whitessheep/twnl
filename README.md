# TWNL

twnl是使用C++2.0实现的一个仿照muduo网络库基于Reactor的多线程网络库

## Technical points
* 非阻塞的IO+主从reactor模式
* 使用C++标准库实现，使用thread库实现多线程，使用chrono库实现时钟定时，mutex库保证线程同步，以及atomic等等
* 支持POLL和EPOLL两种IO复用技术
* 搭配基本的日志库
* 仅支持TCP


## build
```shell
$ ./build.sh 
$ ./build.sh install
```


## 环境
* os: centos 7
* complier: gcc 10.2.1


## 使用

* 一个基础的服务器示范

```c++
	EventLoop loop(EventLoop::EPOLL, EventLoop::ET);                                 //设置事件循环的模式
	InetAddress listenAddr(12345);                                                   //监听端口
	EchoServer server(&loop, listenAddr, "EchoServer", TcpServer::kNoReusePort);     //用户设计的服务器
	server.start();                                                                  //启动服务器
	loop.loop();                                                                     //开启事件循环处理事件
```

* TcpServer，负责最基础的服务器初始化工作
* 用户可以通过往TcpServer中注册回调在函数中处理事件以及设置开启的线程总数
```c++
class EchoServer
{
public:

	EchoServer(EventLoop* loop,
		  const InetAddress& listenAddr,
		  const std::string& name,
                  const int numThread = 1) 
        : server_(loop, listenAddr, name){
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
        server_.setThreadNum(numThread);
    }

	void start() {
        server_.start();
    }
    ...
private:
    TcpServer server_;
};
```

待续

## 致谢
	《linux多线程服务端编程》 陈硕














