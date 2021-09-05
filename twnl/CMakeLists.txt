set(SOURCE_FILES
        EventLoop.cc 
        EventLoopThread.cc
        EventLoopThreadPool.cc
        Socket.cc
        Poller.cc
        Poller/EpollPoller.cc 
        Poller/PollPoller.cc 
        Log/LogStream.cc 
        Log/Logging.cc 
        SocketsOps.cc
        Channel.cc 
        Acceptor.cc 
        Connector.cc
        TcpClient.cc
        InetAddress.cc 
        TcpConnection.cc 
        TcpServer.cc 
        Buffer.cc
        TimerQueue.cc 
        )

add_library(twnl STATIC ${SOURCE_FILES})
target_link_libraries(twnl pthread)

install(TARGETS twnl DESTINATION lib)
set(HEADERS
        Acceptor.h
        Buffer.h
        Callbacks.h
        Channel.h
        Poller.h
        Poller/EpollPoller.h 
        Poller/PollPoller.h 
        EventLoop.h
        EventLoopThread.h
        EventLoopThreadPool.h
        InetAddress.h
        Socket.h
        SocketsOps.h
        Connector.h 
        TcpClient.h
        noncopyable.h
        TcpConnection.h
        TcpServer.h
        Timer.h
        TimerQueue.h
        Timestamp.h
        Log/LogStream.h 
        Log/Logging.h
        )
install(FILES ${HEADERS} DESTINATION include)
