#include <string.h>
#include <stdlib.h>
#include "socketsendclientmsg.h"

SocketSendClientMsg::SocketSendClientMsg()
	: sid_(0)
	, msg_(NULL)
	, msglen_(0)
{
}

SocketSendClientMsg::~SocketSendClientMsg()
{
	if (msg_)
		free(msg_);
}

std::string SocketSendClientMsg::GetMessageName()
{
	return "SocketSendClientMsg";
}

bool SocketSendClientMsg::SetIntegerField(const char* name, int index, int64_t value)
{
	if (strcmp(name, "sid") == 0)
	{
		sid_ = (int)value;
		return true;
	}
	else if (strcmp(name, "msgtype") == 0)
	{
		msgtype_ = (int)value;
		return true;
	}

	return false;
}

bool SocketSendClientMsg::SetStringField(const char* name, int index, 
		const char* value, int len)
{
	if (strcmp(name, "msg") == 0)
	{
		msg_ = (char*)realloc(msg_, len);
		memcpy(msg_, value, len);
		msglen_ = len;
		return true;
	}

	return false;
}

