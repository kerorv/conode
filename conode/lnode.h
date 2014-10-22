#pragma once

#include <string>
#include <list>
#include "lua.hpp"
#include "message.h"

struct Timer;
class Lnode
{
public:
	Lnode(unsigned int id);
	~Lnode();

	bool Create(
			lua_State* L, 
			const std::string& class_name, 
			const std::string& config, 
			int refnew);
	void Destroy();
	void ProcessMsg(const Message& msg);
	void OnTimer(unsigned int tid);

	void AddTimer(Timer* timer);
	void RemoveTimer(unsigned int tid);

	unsigned int GetId() const { return id_; }

private:
	bool CallNew(const char* config, int ref);
	void CallRelease();
	void CallOnMessage(const Message& msg);
	void CallOnTimer(unsigned int tid);

private:
	unsigned int id_;
	lua_State* ls_;
	std::string name_;
	typedef std::list<Timer*> TimerList;
	TimerList timers_;
	int ref_;
};

