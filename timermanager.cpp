#include "timermanager.h"

#define MAX_TIMER_COUNT		0x00FFFFFF

TimerManager::TimerManager(unsigned int id)
	: tids_(id + 1, MAX_TIMER_COUNT)
{
	timerevents_.reserve(64);
}

TimerManager::~TimerManager()
{
	for (TimerList::iterator it = timers_.begin();
			it != timers_.end(); ++it)
	{
		Timer* timer = *it;
		delete timer;
	}
}

Timer* TimerManager::CreateTimer(int interval, Lnode* node)
{
	unsigned int tid = tids_.Assign();
	if (tid == 0)
		return nullptr;

	Timer* timer = new Timer;
	timer->id = tid;
	timer->interval = interval;
	timer->next_time = std::chrono::system_clock::now() + 
		std::chrono::milliseconds(interval);
	timer->node = node;

	timers_.push_back(timer);
	timers_.sort();

	return timer;
}

void TimerManager::OnTick()
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
			timerevents_.push_back(timer);
			++it;

			need_sort = true;
		}
	}

	for (TimerVector::iterator it = timerevents_.begin();
			it != timerevents_.end(); ++it)
	{
		Timer* timer = *it;
		timer->node->OnTimer(timer->id);
	}
	timerevents_.clear();

	if (need_sort)
	{
		timers_.sort();
	}
}

