#include "timerservice.h"

#define MAX_TIMER_COUNT		0xFFFFFFFF

TimerService::TimerService(unsigned int id)
	: tids_(id + 1, MAX_TIMER_COUNT)
{
}

TimerService::~TimerService()
{
	for (TimerList::iterator it = timers_.begin();
			it != timers_.end(); ++it)
	{
		Timer* timer = *it;
		delete timer;
	}
}

unsigned int TimerService::CreateTimer(int interval, Lnode* node)
{
	unsigned int tid = tids_.Assign();
	if (tid == 0)
		return 0;

	Timer* timer = new Timer;
	timer->id = tid;
	timer->interval = interval;
	timer->next_time = std::chrono::system_clock::now() + 
		std::chrono::milliseconds(interval);
	timer->node = node;

	timers_.push_back(timer);
	timers_.sort();

	node->AddTimer(timer);
	return tid;
}

void TimerService::DestroyTimer(unsigned int tid, Lnode* node)
{
	node->RemoveTimer(tid);
}

void TimerService::OnTick()
{
	bool need_sort = false;
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	for (TimerList::iterator it = timers_.begin();
			it != timers_.end(); )
	{
		Timer* timer = *it;
		if (timer->interval == -1)
		{
			// remove
			tids_.Recycle(timer->id);
			delete timer;
			it = timers_.erase(it);
		}
		else
		{
			if (now < timer->next_time)
				break;

			timer->next_time = now + std::chrono::milliseconds(timer->interval);
			timer->node->OnTimer(timer->id);
			++it;

			need_sort = true;
		}
	}

	if (need_sort)
	{
		timers_.sort();
	}
}

