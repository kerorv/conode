#pragma once

#include "eventhandler.h"

class StreamSocket;
class SocketServer;
class ListenSocket : public EventHandler
{
public:
	ListenSocket();
	virtual ~ListenSocket();

	bool Create(int port, SocketServer* server);
	void Close();

protected:
	// EventHandler
	virtual int GetFd() { return fd_; }
	virtual void OnEvent(int events);

	StreamSocket* Accept();

private:
	int fd_;
	SocketServer* server_;
};

