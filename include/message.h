#pragma once

#include <stdint.h>

struct Message
{
	int32_t type;
	int32_t size;
	char* content;
	unsigned int to;
};

