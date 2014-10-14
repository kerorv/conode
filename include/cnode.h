#pragma once

#include "message.h"
#include "msgrouter.h"

class Cnode
{
public:
	virtual ~Cnode() {}

	virtual bool Create(unsigned int id, MsgRouter* router, 
			const char* config) = 0;
	virtual void Close() = 0;
	virtual void OnMessage(const Message& msg) = 0;
};

