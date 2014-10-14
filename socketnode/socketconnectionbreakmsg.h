#pragma once

#include <string.h>
#include "sprotomessage.h"

class SocketConnectionBreakMsg : public SprotoMessage
{
public:
	SocketConnectionBreakMsg() {}
	virtual ~SocketConnectionBreakMsg() {}

	virtual std::string GetMessageName() {
		return "SocketConnectionBreakMsg";
	}
	virtual bool GetIntegerField(const char* name, int index, int64_t& value) {
		if (strcmp(name, "sid") == 0)
		{
			value = sid_;
			return true;
		}
		else if (strcmp(name, "reason") == 0)
		{
			value = reason_;
			return true;
		}

		return false;
	}
	virtual bool SetIntegerField(const char* name, int index, int64_t value) {
		if (strcmp(name, "sid") == 0)
		{
			sid_ = (int)value;
			return true;
		}
		else if (strcmp(name, "reason") == 0)
		{
			reason_ = (int)value;
			return true;
		}

		return false;
	}

	void SetSid(int sid) { sid_ = sid; }
	void SetReason(int reason) { reason_ = reason; }

private:
	int sid_;
	int reason_;
};

