#pragma once

#include "sprotomessage.h"

class LoginMsg : public SprotoMessage
{
public:
	LoginMsg() {}
	virtual ~LoginMsg() {}

	virtual std::string GetMessageName()
	{
		return "LoginMsg";
	}

	virtual const char* GetStringField(const char* name, int index, int& len)
	{
		if (strcmp(name, "username") == 0)
		{
			if (username_.empty())
			{
				len = 0;
				return nullptr;
			}
			else
			{
				len = username_.length();
				return &username_[0];
			}
		}
		else if (strcmp(name, "password") == 0)
		{
			if (password_.empty())
			{
				len = 0;
				return nullptr;
			}
			else
			{
				len = password_.length();
				return &password_[0];
			}
		}

		return nullptr;
	}

	virtual bool SetStringField(const char* name, int index, 
			const char* value, int len)
	{
		if (strcmp(name, "username") == 0)
		{
			username_.assign(value, len);
			return true;
		}
		else if (strcmp(name, "password") == 0)
		{
			password_.assign(value, len);
			return true;
		}
		else
		{
			return false;
		}
	}

	void SetUsername(const std::string& username) { username_ = username; }
	void SetPassword(const std::string& password) { password_ = password; }

private:
	std::string username_;
	std::string password_;
};

