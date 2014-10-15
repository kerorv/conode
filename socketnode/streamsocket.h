#pragma once

#include <stdint.h>
#include <list>
#include "eventhandler.h"

class SocketServer;
class StreamSocket : public EventHandler
{
public:
	StreamSocket(int fd, SocketServer* server);
	virtual ~StreamSocket();

	void Close();
	void SendMsg(const char* msg, int len);
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
	struct OutMsg;
	int SendHeader(OutMsg& msg);
	int SendBody(OutMsg& msg);
	int SendBlock(const char* block, int size);

private:
	int fd_;
	int id_;
	SocketServer* server_;
	// write operation
	enum SendStatus
	{
		StatusSendHeader,
		StatusSendBody,
		StatusSendOk,
	};
	struct OutMsg
	{
		uint16_t len;
		const char* data;
		int send_bytes;
		SendStatus status;
	};
	typedef std::list<OutMsg> OutMsgList;
	OutMsgList outmsgs_;
	bool poll_write_;

	// read operation
	uint16_t msglen_;
	char* msgbody_;
	int haslen_;
	enum ReadStatus
	{
		StatusReadHeader,
		StatusReadBody,
	};
	ReadStatus rstatus_;
};

