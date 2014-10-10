#pragma once

#include "message.h"

class MsgRouter
{
public:
	virtual ~MsgRouter() {}

	virtual void SendMsg(const Message& msg) = 0;
};

