#pragma once

#include <chrono>
#include <list>
#include "lnode.h"
#include "idallocator.h"

struct Timer
{
	unsigned int id;
	int interval;
	std::chrono::system_clock::time_point next_time;
	Lnode* node;
	
	bool operator < (const Timer& t)
	{
		return next_time < t.next_time;
	}
};

class TimerService
{
public:
	TimerService(unsigned int id);
	~TimerService();

	unsigned int CreateTimer(int interval, Lnode* node);
	void DestroyTimer(unsigned int tid, Lnode* node);
	void OnTick();

	unsigned int AssignTimerId();
	void ReleaseTimerId(unsigned int id);

private:
	typedef std::list<Timer*> TimerList;
	TimerList timers_;
	IdAllocator tids_;
};

