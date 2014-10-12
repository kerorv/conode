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

bool Scheduler::LoadCnodes()
{
	lua_State* L = lua_newstate();
	lua_dofile(L, "config.lua");
	lua_getglobal(L, "worker_count");
	int worker_count = luaL_checkint(L, -1);
	lua_pop(L, 1);
	lua_getglobal(L, "cnodes");
	int cnode_count = luaL_len(L, -1);
	for (int i = 0; i < cnode_count; ++i)
	{
		lua_rawgeti(L, -1, i);
		std::string node_name;
		unsigned int node_id;
		std::string node_config;
		// node name
		lua_getfield(L, -1, "name");
		node_name = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		// node id
		lua_getfield(L, -2, "id");
		node_id = std::to_string(luaL_checkint(L, -1));
		lua_pop(L, 1);
		// node config
		lua_getfield(L, -3, "config");
		while (lua_next(L, -1))
		{
			node_config += lua_tostring(L, -2); // key
			node_config += "=";
			node_config += lua_tostring(L, -1); // value
			node_config += "\n";
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		CnodeModule* module = new CnodeModule;
		if (!module->Load(node_name.c_str()))
		{
			delete module;
			return false;
		}

		Cnode* node = module->CreateCnode();
		if (node)
		{
			cnodes_.push_back(std::make_pair(node_id, node));
		}
	}
	lua_close(L);
}

