#pragma once

#include "sprotomessage.h"

class SocketSendClientMsg : public SprotoMessage
{
public:
	SocketSendClientMsg();
	virtual ~SocketSendClientMsg();

	virtual std::string GetMessageName();
	virtual bool SetIntegerField(const char* name, int index, int64_t value);
	virtual bool SetStringField(const char* name, int index, 
			const char* value, int len);

	int GetSid() const { return sid_; }
	int GetMsgType() const { return msgtype_; }
	const char* MoveMsg(int& len) {
		const char* tmp = msg_;
		len = msglen_;
		msg_ = NULL;
		msglen_ = 0;
		return tmp;
	}

private:
	int sid_;
	int msgtype_;
	char* msg_;
	int msglen_;
};

