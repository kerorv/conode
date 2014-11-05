#pragma once

#include <list>
#include "lua.hpp"
#include "message.h"

class Worker;
struct Timer;
class Lnode
{
public:
	Lnode(Worker* worker, unsigned int id);
	~Lnode();

	bool Create(const char* file, const char* name, const char* config);
	void Destroy();
	void ProcessMsg(const Message& msg);
	void OnTimer(unsigned int tid);

	unsigned int SetTimer(int interval);
	void KillTimer(unsigned int tid);

	unsigned int GetId() const { return id_; }

private:
	void RegisterApi();
	void CallInit(unsigned int id, const char* config);
	void CallRelease();
	void CallOnMessage(const Message& msg);
	void CallOnTimer(unsigned int tid);

private:
	Worker* worker_;
	unsigned int id_;
	lua_State* ls_;
	typedef std::list<Timer*> TimerList;
	TimerList timers_;
	int ref_;
};

