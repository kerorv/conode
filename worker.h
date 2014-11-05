#pragma once

#include <string>
#include <map>
#include <mutex>
#include <thread>
#include "lua.hpp"
#include "message.h"
#include "concurrentqueue.h"
#include "idallocator.h"
#include "timermanager.h"

class Scheduler;
class Lnode;
class Worker
{
public:
	Worker(Scheduler* sched, unsigned int id);
	~Worker();

	bool Create();
	void Run();
	void Destroy();

	unsigned int SpawnNode(const char* name, const char* config);
	void CloseNode(unsigned int nid);
	void SendMsg(const Message& msg);

	unsigned int GetId() const { return id_; }
	Scheduler* GetScheduler() { return sched_; }
	TimerManager* GetTimerManager() { return &tm_; }

private:
	void DispatchMsg(const Message& msg);
	void ProcessMsg(const Message& msg);

private:
	Scheduler* sched_;
	unsigned int id_;
	std::thread* thread_;
	bool quit_;
	typedef std::map<int, Lnode*> LnodeMap;
	LnodeMap nodes_;
	MsgQueue msgs_;
	IdAllocator nids_;
	std::mutex idmutex_;
	TimerManager tm_;
};

