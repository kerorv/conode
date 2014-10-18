#pragma once

#include "sprotomessage.h"

class SocketRecvClientMsg : public SprotoMessage
{
public:
	SocketRecvClientMsg();
	virtual ~SocketRecvClientMsg();

	virtual std::string GetMessageName();
	virtual bool GetIntegerField(const char* name, int index, int64_t& value);
	virtual const char* GetStringField(const char* name, int index, int& len);

	void SetSid(int sid) { sid_ = sid; }
	void SetMsgType(int msgtype) { msgtype_ = msgtype; }
	void SetMsg(char* msg, int len) {
		msg_ = msg;
		msglen_ = len;
	}

private:
	int sid_;
	int msgtype_;
	char* msg_;
	int msglen_;
};


