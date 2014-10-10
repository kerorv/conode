#include "capi.h"
#include "worker.h"
#include "scheduler.h"

#define MAX_WORKER_COUNT	255

Scheduler* Scheduler::GetInstance()
{
	static Scheduler scheduler;
	return &scheduler;
}

Scheduler::Scheduler()
	: ls_(nullptr)
{
}

Scheduler::~Scheduler()
{
}

bool Scheduler::Create(size_t worker_count, const char* main_node)
{
	if (worker_count == 0)
		worker_count = 1;
	else if (worker_count > MAX_WORKER_COUNT)
		worker_count = MAX_WORKER_COUNT;

	for (size_t i = 0; i < worker_count; ++i)
	{
		Worker* worker = new Worker((unsigned int)(i + 1) << 24);
		if (!worker->Create())
			return false;

		workers_.push_back(worker);
	}

	SpawnNode(main_node);
	return true;
}

void Scheduler::Close()
{
	for (WorkerVec::iterator it = workers_.begin();
			it != workers_.end(); ++it)
	{
		Worker* worker = *it;
		worker->Destroy();
		delete worker;
	}

	workers_.clear();

	if (ls_ != nullptr)
	{
		lua_close(ls_);
		ls_ = nullptr;
	}
}

unsigned int Scheduler::SpawnNode(const std::string& node_name)
{
	int worker_idx = NextWorkerIdx();
	Worker* worker = workers_[worker_idx];
	if (worker == nullptr)
		return 0;

	return worker->SpawnNode(node_name);
}

void Scheduler::CloseNode(unsigned int nid)
{
	Worker* worker = GetWorkerByNodeId(nid);
	if (worker == nullptr)
		return;

	worker->CloseNode(nid);
}

void Scheduler::SendMsg(const Message& msg)
{
	Worker* worker = GetWorkerByNodeId(msg.to);
	if (worker == nullptr)
		return;

	worker->SendMsg(msg);
}

unsigned int Scheduler::SetTimer(unsigned int nid, int interval)
{
	Worker* worker = GetWorkerByNodeId(nid);
	if (worker == nullptr)
		return 0;

	return worker->SetTimer(nid, interval);
}

void Scheduler::KillTimer(unsigned int nid, unsigned int tid)
{
	Worker* worker = GetWorkerByNodeId(nid);
	if (worker == nullptr)
		return;

	worker->KillTimer(nid, tid);
}

int Scheduler::NextWorkerIdx()
{
	return counter_.fetch_add(1) % workers_.size();
}

Worker* Scheduler::GetWorkerByNodeId(unsigned int nid)
{
	if (nid == 0)
		return nullptr;

	size_t worker_idx = (nid >> 24) - 1;
	if (worker_idx >= workers_.size())
		return nullptr;

	Worker* w = workers_[worker_idx];
	return w;
}

