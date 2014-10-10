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

protected:
	// EventHandler
	virtual int GetFd() { return fd_; }
	virtual void OnEvent(int events);

	void OnRead();
	void OnWrite();

	bool ReadHeader();
	bool ReadBody();

	void SendList();
	bool SendBlock(const char* data, int size, int& has_send);

private:
	int fd_;
	int id_;
	SocketServer* server_;
	// write operation
	struct OutMsg
	{
		uint16_t len;
		const char* data;
		int send_bytes;
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

