#pragma once

#include <chrono>
#include <list>
#include <vector>
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

class TimerManager
{
public:
	TimerManager(unsigned int id);
	~TimerManager();

	Timer* CreateTimer(int interval, Lnode* node);
	void OnTick();

private:
	typedef std::list<Timer*> TimerList;
	TimerList timers_;
	typedef std::vector<Timer*> TimerVector;
	TimerVector timerevents_;
	IdAllocator tids_;
};

