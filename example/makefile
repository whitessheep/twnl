CC= g++
CXXFLAGS = -std=c++17 -g  -Wall -I../source -pthread 
LDFLAGS = -lpthread

BASE_SRC = ../source/Log/Logging.cc ../source/Log/LogStream.cc 

LIB_SRC = ../source/Acceptor.cc \
          ../source/Buffer.cc \
          ../source/Channel.cc \
          ../source/EventLoop.cc \
          ../source/EventLoopThread.cc \
          ../source/EventLoopThreadPool.cc \
          ../source/InetAddress.cc \
		  ../source/DefaultPoller.cc  \
		  ../source/EpollPoller.cc   \
		  ../source/PollPoller.cc \
		  ../source/Poller.cc  \
          ../source/Socket.cc \
          ../source/SocketsOps.cc \
          ../source/TcpConnection.cc \
          ../source/TcpServer.cc \
          ../source/TimerQueue.cc

BINARIES = echo 

all: $(BINARIES)

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS) 

clean:
	rm -f $(BINARIES) core

echo: echo.cc
