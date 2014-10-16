#include <stdio.h>
#include <string>
#include "scheduler.h"

#define WORKER_COUNT	2

int main(int argc, char* argv[])
{
	Scheduler* sched = Scheduler::GetInstance();
	if (!sched->Create())
	{
		printf("scheduler create fail!\n");
		return -1;
	}
	
	getchar();

	sched->Close();
	return 0;
}

