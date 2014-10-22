#pragma once

#include <stdint.h>
#include <list>
#include "message.h"
#include "eventhandler.h"
#include "netmsg.h"

class SocketServer;
class StreamSocket : public EventHandler
{
public:
	StreamSocket(int fd, SocketServer* server);
	virtual ~StreamSocket();

	void Close();
	void SendMsg(int msgtype, const char* msg, int len);
	bool IsActive() { return (fd_ != -1); }
	int GetId() const { return id_; }

private:
	// EventHandler
	virtual int GetFd() { return fd_; }
	virtual void OnEvent(int events);

	void OnRead();
	void OnWrite();

	int ReadHeader();
	int ReadBody();
	int ReadBlock(char* block, size_t size);

	int SendList();
	int SendBlock(const char* block, int size);

private:
	int fd_;
	int id_;
	SocketServer* server_;

	// write operation
	typedef std::list<NetMsg*> OutMsgList;
	OutMsgList outmsgs_;
	bool poll_write_;
	int has_send_;

	// read operation
	uint16_t msglen_;
	char* msgbody_;
	int has_read_;
	enum ReadStatus
	{
		StatusReadHeader,
		StatusReadBody,
	};
	ReadStatus rstatus_;
};

