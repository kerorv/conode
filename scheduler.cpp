#include "lua.hpp"
#include "capi.h"
#include "worker.h"
#include "cnode.h"
#include "cnodemodule.h"
#include "scheduler.h"

#define MAX_WORKER_COUNT	255

Scheduler* Scheduler::GetInstance()
{
	static Scheduler scheduler;
	return &scheduler;
}

Scheduler::Scheduler()
	: worker_count_(1)
	, main_node_id_(0)
{
}

Scheduler::~Scheduler()
{
}

bool Scheduler::Create(size_t worker_count, const char* main_node)
{
	lua_State* L = luaL_newstate();
	luaL_dofile(L, "config.lua");

	// worker count
	lua_getglobal(L, "worker");
	worker_count_ = luaL_checkint(L, -1);
	if (worker_count_ == 0)
		worker_count_ = 1;
	else if (worker_count_ > MAX_WORKER_COUNT)
		worker_count_ = MAX_WORKER_COUNT;
	lua_pop(L, 1);

	// main node
	lua_getglobal(L, "main");
	this->main_node_ = luaL_checkstring(L, -1);
	lua_pop(L, 1);

	if (main_node_.empty())
	{
		lua_close(L);
		return false;
	}

	// load lnode
	for (size_t i = 0; i < (size_t)worker_count_; ++i)
	{
		Worker* worker = new Worker((unsigned int)(i + 1) << 24);
		if (!worker->Create())
			return false;

		workers_.push_back(worker);
	}

	main_node_id_ = SpawnNode(main_node);
	if (main_node_id_ == 0)
	{
		lua_close(L);
		return false;
	}

	// load cnode
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
		node_id = (unsigned int)luaL_checkint(L, -1);
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

		Cnode* node = LoadCnode(node_name, node_id, node_config);
		if (node == nullptr)
		{
			// TODO
		}
	}
	lua_close(L);

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

	for (CnodeModuleMap::iterator it = cnode_modules_.begin();
			it != cnode_modules_.end(); ++it)
	{
		CnodeModule* module = it->second;
		module->Close();
		delete module;
	}

	cnode_modules_.clear();
	cnodes_.clear();
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
		CnodeMap::iterator it = cnodes_.find(msg.to);
		if (it != cnodes_.end())
		{
			Cnode* node = it->second;
			node->OnMessage(msg);
		}
	}
	else	// send to lnode
	{
		Worker* worker = GetWorkerByNodeId(msg.to);
		if (worker == nullptr)
			return;

		worker->SendMsg(msg);
	}
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

Cnode* Scheduler::LoadCnode(const std::string& name, unsigned int id, 
		const std::string& config)
{
	CnodeModule* module = nullptr;
	CnodeModuleMap::iterator it = cnode_modules_.find(name);
	if (it == cnode_modules_.end())
	{
		CnodeModule* new_module = new CnodeModule;
		if (!new_module->Load(name.c_str()))
		{
			delete module;
			return nullptr;
		}

		cnode_modules_.insert(std::make_pair(name, new_module));
		module = new_module;
	}
	else
	{
		module = it->second;
	}

	Cnode* node = module->CreateCnode();
	if (node)
	{
		if (!node->Create(id, this, config.c_str()))
		{
			module->ReleaseCnode(node);
			return nullptr;
		}

		cnodes_.insert(std::make_pair(id, node));
	}

	return node;
}

