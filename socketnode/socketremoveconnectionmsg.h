#pragma once

#include <string.h>
#include "sprotomessage.h"

class SocketRemoveConnectionMsg : public SprotoMessage
{
public:
	SocketRemoveConnectionMsg() {}
	virtual ~SocketRemoveConnectionMsg() {}

	virtual std::string GetMessageName() {
		return "SocketRemoveConnectionMsg";
	}
	virtual bool GetIntegerField(const char* name, int index, int64_t& value) {
		if (strcmp(name, "sid") == 0)
		{
			value = sid_;
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

		return false;
	}

	int GetSid() const { return sid_; }

private:
	int sid_;
};

