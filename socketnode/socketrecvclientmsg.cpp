#include <string.h>
#include <stdlib.h>
#include "socketrecvclientmsg.h"

SocketRecvClientMsg::SocketRecvClientMsg()
	: sid_(0)
	, msg_(NULL)
	, msglen_(0)
{
}

SocketRecvClientMsg::~SocketRecvClientMsg()
{
}

std::string SocketRecvClientMsg::GetMessageName()
{
	return "SocketRecvClientMsg";
}

bool SocketRecvClientMsg::GetIntegerField(const char* name, int index, int64_t& value)
{
	if (strcmp(name, "sid") == 0)
	{
		value = sid_;
		return true;
	}
	else if (strcmp(name, "msgtype") == 0)
	{
		value = msgtype_;
		return true;
	}

	return false;
}

const char* SocketRecvClientMsg::GetStringField(const char* name, int index, int& len)
{
	if (strcmp(name, "msg") == 0)
	{
		len = msglen_;
		return msg_;
	}

	return NULL;
}


