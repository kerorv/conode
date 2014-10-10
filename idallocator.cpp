#include "idallocator.h"


IdAllocator::IdAllocator(unsigned int seed_id, unsigned int range)
	: maxid_(seed_id + range)
	, lastid_(seed_id + 1)
{
}

IdAllocator::~IdAllocator()
{
}

unsigned int IdAllocator::Assign()
{
	unsigned int id = 0;

	if (rids_.empty())
	{
		if (lastid_ <= maxid_)
		{
			id = lastid_++;
		}
	}
	else
	{
		id = rids_.front();
		rids_.pop_front();
	}
	
	return id;
}

void IdAllocator::Recycle(unsigned int id)
{
	if (id == 0)
		return;

	rids_.push_back(id);
}

