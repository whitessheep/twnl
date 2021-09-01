# TWNL

twnl是使用C++2.0实现的一个仿照muduo网络库基于Reactor的多线程网络库

## Technical points
* 非阻塞的IO+主从reactor模式
* 使用C++标准库实现，使用thread库实现多线程，使用chrono库实现时钟定时，mutex库保证线程同步，以及atomic等等
* 支持POLL和EPOLL两种IO复用技术
* 搭配基本的日志库
* 仅支持TCP














