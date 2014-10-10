#include <stdio.h>
#include <string>
#include "scheduler.h"

#define WORKER_COUNT	2

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("conode main_node_name");
		return -1;
	}

	const char* main_node = argv[1];
	Scheduler* sched = Scheduler::GetInstance();
	if (!sched->Create(WORKER_COUNT, main_node))
	{
		printf("scheduler create fail!\n");
		return -1;
	}
	
	getchar();

	sched->Close();
	return 0;
}

