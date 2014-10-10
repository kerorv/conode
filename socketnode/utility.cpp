#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "utility.h"

bool SetSockReuseAddress(int sock)
{
	int reuse = 1;
	int ret = setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (int));
	return (ret != -1);
}

bool SetNonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		flags = 0;

	flags |= O_NONBLOCK;
	int ret = fcntl(fd, F_SETFL, flags);
	return (ret != -1);
}

