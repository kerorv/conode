#pragma once

#include "sprotomessage.h"

class EchoMsg : public SprotoMessage
{
public:
	EchoMsg() {}
	virtual ~EchoMsg() {}

	virtual std::string GetMessageName()
	{
		return "EchoMsg";
	}

	virtual const char* GetStringField(const char* name, int index, int& len)
	{
		if (strcmp(name, "msg") == 0)
		{
			if (msg_.empty())
			{
				len = 0;
				return nullptr;
			}
			else
			{
				len = msg_.length();
				return &msg_[0];
			}
		}

		return nullptr;
	}

	virtual bool SetStringField(const char* name, int index, 
			const char* value, int len)
	{
		if (strcmp(name, "msg") == 0)
		{
			msg_.assign(value, len);
			return true;
		}
		else
		{
			return false;
		}
	}

	void SetMsg(const std::string& msg) { msg_ = msg; }
	std::string GetMsg() const { return msg_; }

private:
	std::string msg_;
};


