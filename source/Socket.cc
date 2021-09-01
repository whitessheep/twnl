//
//5.29
//

#include "Socket.h"

#include "InetAddress.h"
#include "SocketsOps.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>


using namespace twnl;

Socket::~Socket() {
	sockets::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr) {
	sockets::bindOrDie(sockfd_, addr.getSockAddr());
}

void Socket::listen() {
	sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peeraddr) {
	struct sockaddr_in6 addr;
	bzero(&addr, sizeof addr);
	int connfd = sockets::accept(sockfd_, &addr);
	if (connfd >= 0) {
		peeraddr->setSockAddrInet6(addr);
	}
	return connfd;
}

void Socket::setReuseAddr(bool on) {
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on) {
	int optval = on ? 1 : 0;
	 ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setTcpNoDelay(bool on) {
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setKeepAlive(bool on) {
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::shutdownWrite() {
	sockets::shutdownWrite(sockfd_);
}

