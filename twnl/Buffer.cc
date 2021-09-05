#include <errno.h>
#include <memory.h>
#include <sys/uio.h>
#include <unistd.h>
#include <assert.h>

#include "Buffer.h"

using namespace twnl;
const char Buffer::kCRLF[] = "\r\n";
const size_t Buffer::kInitialSize;


ssize_t Buffer::readFd(int fd, int* savedErrno) {
	char  extrabuf[65536];
	struct iovec vec[2];
	const size_t writable = writableBytes();
	vec[0].iov_base = begin() + writerIndex_;
	vec[0].iov_len = writable;
	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof extrabuf;
	const ssize_t n = readv(fd, vec, 2);
	if (n < 0) {
		*savedErrno = errno;
	}
	else if (static_cast<size_t>(n) <= writable) {
		writerIndex_ += n;
	}
	else {
		writerIndex_ = buffer_.size();
		append(extrabuf, n - writable);
	}
	return n;
}

ssize_t Buffer::writeFd(int fd, int* savedErrno) {
	const size_t readable = readableBytes();
	const ssize_t n = write(fd, peek(), readable);
	if (n < 0) {
		*savedErrno = errno;
		return n;
	}
	readerIndex_ += n;
	return n;

}

ssize_t Buffer::readFdET(int fd, int* savedErrno) {
	ssize_t n = 0;
	while (true) {
		ssize_t len = readFd(fd, savedErrno);
		if (len <= 0) {
			if (*savedErrno == EAGAIN) {
				break;
			}
			else 
				return len;
		}
		n += len;
	}
	return n;
}

ssize_t Buffer::writeFdET(int fd, int* savedErrno) {
	ssize_t n = 0;
	while (true) {
		ssize_t len = writeFd(fd, savedErrno);
		if (len <= 0) {
			if (*savedErrno == EAGAIN) {
				break;
			}
			else 
				return len;
		}
		n += len;
	}
	return n;
}
