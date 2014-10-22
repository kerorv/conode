#include <stdio.h>
#include <string>
#include "conode.h"

#define WORKER_COUNT	2

int main(int argc, char* argv[])
{
	void* handle = conode_start(WORKER_COUNT, "MainNode");
	if (handle == NULL)
	{
		printf("scheduler create fail!\n");
		return -1;
	}
	
	getchar();

	conode_stop(handle);
	return 0;
}

