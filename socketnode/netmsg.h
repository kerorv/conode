#pragma once

#include <stdint.h>

struct NetMsg
{
	uint16_t size;
	uint16_t type;
	char content[];
};

