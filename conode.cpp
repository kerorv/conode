#include "conode.h"
#include "scheduler.h"

CONODE_API void* conode_start(int worker, const char* mainnode)
{
	Scheduler* sched = new Scheduler();
	if (!sched->Create(worker, mainnode))
	{
		sched->Close();
		return NULL;
	}

	return sched;
}

CONODE_API void conode_stop(void* handle)
{
	Scheduler* sched = (Scheduler*)handle;
	if (sched)
	{
		sched->Close();
		delete sched;
	}
}

