#pragma once

#include <deque>

class IdAllocator
{
public:
	IdAllocator(unsigned int seed_id, unsigned int range);
	~IdAllocator();

	unsigned int Assign();
	void Recycle(unsigned int id);

private:
	const unsigned int maxid_;
	unsigned int lastid_;
	typedef std::deque<unsigned int> RecycleIdQueue;
	RecycleIdQueue rids_;
};

