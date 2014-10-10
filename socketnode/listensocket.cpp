#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "utility.h"
#include "socketserver.h"
#include "streamsocket.h"
#include "listensocket.h"

ListenSocket::ListenSocket()
	: fd_(-1)
	, server_(NULL)
{
}

ListenSocket::~ListenSocket()
{
}

bool ListenSocket::Create(int port, SocketServer* server)
{
	fd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_ == -1)
		return false;

	if (!SetSockReuseAddress(fd_) || !SetNonblocking(fd_))
		goto error;

	struct sockaddr_in bind_addr;
	memset(&bind_addr, 0, sizeof(sockaddr_in));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_addr.s_addr = INADDR_ANY;
	bind_addr.sin_port = htons((unsigned short)port);
	if (bind(fd_, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) < 0)
		goto error;

	if (listen(fd_, 64) < 0)
		goto error;

	server_ = server;
	return true;

error:
	if (fd_ != -1)
	{
		close(fd_);
		fd_ = -1;
	}
	return false;
}

void ListenSocket::Close()
{
	if (fd_ != -1)
	{
		close(fd_);
		fd_ = -1;
	}
}

StreamSocket* ListenSocket::Accept()
{
	int client = accept(fd_, NULL, NULL);
	if (client == -1)
		return NULL;

	if (!SetNonblocking(client))
	{
		close(client);
		return NULL;
	}

	return new StreamSocket(client, server_);
}

void ListenSocket::OnEvent(int events)
{
	if (events & EventError)
	{
		// TODO
		// assert(false);
		return;
	}

	if (events & EventRead)
	{
		StreamSocket* ss = Accept();
		if (ss)
		{
			// add to server
			server_->AddSocket(ss);
		}
	}
}

