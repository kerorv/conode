#pragma once

#include "sprotomessage.h"

class SocketSendClientMsg : public SprotoMessage
{
public:
	SocketSendClientMsg();
	virtual ~SocketSendClientMsg();

	virtual std::string GetMessageName();
	virtual bool GetIntegerField(const char* name, int index, int64_t& value);
	virtual const char* GetStringField(const char* name, int index, int& len);
	virtual bool SetIntegerField(const char* name, int index, int64_t value);
	virtual bool SetStringField(const char* name, int index, 
			const char* value, int len);

	int GetSid() const { return sid_; }
	const char* MoveMsg(int& len) {
		const char* tmp = msg_;
		len = msglen_;
		msg_ = NULL;
		msglen_ = 0;
		return tmp;
	}

private:
	int sid_;
	char* msg_;
	int msglen_;
};

