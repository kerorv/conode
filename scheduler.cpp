#include <string.h>
#include "capi.h"
#include "worker.h"
#include "cnodemanager.h"
#include "scheduler.h"

#define MAX_WORKER_COUNT	255

Scheduler::Scheduler()
	: main_node_id_(0)
{
}

Scheduler::~Scheduler()
{
}

bool Scheduler::Create(int worker_count, const char* mainnode)
{
	// cnode manager
	cnodemgr_ = new CnodeManager;
	if (!cnodemgr_->Create(this))
		return false;

	// worker
	for (size_t i = 0; i < (size_t)worker_count; ++i)
	{
		Worker* worker = new Worker((unsigned int)(i + 1) << 24);
		if (!worker->Create(this))
			return false;

		workers_.push_back(worker);
	}

	// spawn main node
	main_node_id_ = SpawnNode(mainnode, nullptr);
	if (main_node_id_ == 0)
	{
		return false;
	}

	return true;
}

void Scheduler::Close()
{
	// stop lnode
	for (WorkerVec::iterator it = workers_.begin();
			it != workers_.end(); ++it)
	{
		Worker* worker = *it;
		worker->Destroy();
	}

	// stop cnode
	if (cnodemgr_)
	{
		cnodemgr_->Destroy();
	}

	// clear lnode
	for (WorkerVec::iterator it = workers_.begin();
			it != workers_.end(); ++it)
	{
		Worker* worker = *it;
		delete worker;
	}
	workers_.clear();

	// clear cnode
	delete cnodemgr_;
	cnodemgr_ = nullptr;
}

unsigned int Scheduler::SpawnNode(
		const char* name, 
		const char* config)
{
	if (name == nullptr)
		return 0;

	if (strlen(name) > 3)
	{
		const char* suffix = name + strlen(name) - 3;
		if (strcmp(suffix, ".so") == 0)
		{
			// cnode
			return cnodemgr_->SpawnNode(name, config);
		}
	}

	// lnode
	int worker_idx = NextWorkerIdx();
	Worker* worker = workers_[worker_idx];
	if (worker == nullptr)
		return 0;

	return worker->SpawnNode(name, config);
}

void Scheduler::CloseNode(unsigned int nid)
{
	if (nid == 0)
	{
		return;
	}
	else if (nid == 1)
	{
		// scheduler
		// internal quit
		// TODO
	}
	else if (nid > 1 && nid < workers_[0]->GetId())
	{
		// cnode
		cnodemgr_->CloseNode(nid);
	}
	else
	{
		// lnode
		Worker* worker = GetWorkerByNodeId(nid);
		if (worker == nullptr)
			return;

		worker->CloseNode(nid);
	}
}

void Scheduler::SendMsg(const Message& msg)
{
	if (msg.to == 0)
	{
		return;
	}
	else if (msg.to == 1) // send to scheduler
	{
		// send to main lnode temporarily
		Message redirect_msg = msg;
		redirect_msg.to = main_node_id_;

		Worker* worker = GetWorkerByNodeId(redirect_msg.to);
		if (worker == nullptr)
			return;

		worker->SendMsg(redirect_msg);
	}
	else if (msg.to < workers_[0]->GetId())	// send to cnode
	{
		cnodemgr_->SendMsg(msg);
	}
	else	// send to lnode
	{
		Worker* worker = GetWorkerByNodeId(msg.to);
		if (worker == nullptr)
			return;

		worker->SendMsg(msg);
	}
}

unsigned int Scheduler::SetTimer(lua_State* L, unsigned int nid, int interval)
{
	Worker* worker = GetWorkerByNodeId(nid);
	if (worker == nullptr)
		return 0;

	return worker->SetTimer(L, nid, interval);
}

void Scheduler::KillTimer(lua_State* L, unsigned int nid, unsigned int tid)
{
	Worker* worker = GetWorkerByNodeId(nid);
	if (worker == nullptr)
		return;

	worker->KillTimer(L, nid, tid);
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

