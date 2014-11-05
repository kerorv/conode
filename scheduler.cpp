#include <string.h>
#include "capi.h"
#include "worker.h"
#include "cnodemodule.h"
#include "scheduler.h"

#define MAX_WORKER_COUNT	255

Scheduler::Scheduler()
	: main_node_id_(0)
{
}

Scheduler::~Scheduler()
{
}

bool Scheduler::Create()
{
	int worker_count;
	std::string mainnode;
	std::string nodeconfig;
	lua_State* L = luaL_newstate();
	luaL_dofile(L, "config.lua");

	// worker count
	lua_getglobal(L, "worker");
	worker_count = luaL_checkint(L, -1);
	if (worker_count == 0)
		worker_count = 1;
	else if (worker_count > MAX_WORKER_COUNT)
		worker_count = MAX_WORKER_COUNT;
	lua_pop(L, 1);

	// main node
	lua_getglobal(L, "main");
	if (!lua_istable(L, -1))
	{
		lua_close(L);
		return false;
	}
	lua_getfield(L, -1, "name");
	const char* sz = luaL_checkstring(L, -1);
	if (sz == nullptr || strlen(sz) == 0)
	{
		lua_close(L);
		return false;
	}
	mainnode = sz;
	lua_pop(L, 1);
	lua_getfield(L, -1, "config");
	sz = lua_tostring(L, -1);
	if (sz)
	{
		nodeconfig = sz;
	}
	lua_pop(L, 2);

	// load lnode
	for (size_t i = 0; i < (size_t)worker_count; ++i)
	{
		Worker* worker = new Worker(this, (unsigned int)(i + 1) << 24);
		if (!worker->Create())
			return false;

		workers_.push_back(worker);
	}

	// load cnode
	lua_getglobal(L, "cnode");
	int cnode_count = luaL_len(L, -1);
	for (int i = 1; i <= cnode_count; ++i)
	{
		unsigned int node_id = i + 1;
		std::string node_name;
		std::string lib_name;
		std::string node_config;

		lua_rawgeti(L, -1, i);
		// node name
		lua_getfield(L, -1, "name");
		node_name = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		// lib name
		lua_getfield(L, -1, "lib");
		lib_name = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		// node config
		lua_getfield(L, -1, "config");
		if (!lua_isnil(L, -1))
		{
			node_config = luaL_checkstring(L, -1);
		}
		lua_pop(L, 1);

		if (!LoadCnode(lib_name, node_id, node_name, node_config))
		{
			lua_close(L);
			return false;
		}
	}
	lua_close(L);

	// spawn main node
	main_node_id_ = SpawnNode(mainnode.c_str(), nodeconfig.c_str());
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
	for (CnodeInstMap::iterator it = cnodeinsts_.begin();
			it != cnodeinsts_.end(); ++it)
	{
		CnodeInst& inst = it->second;
		Cnode* node = inst.node;
		CnodeModule* module = inst.module;
		node->Close();
		module->ReleaseCnode(node);
	}
	cnodeinsts_.clear();

	for (CnodeModuleMap::iterator it = cnode_modules_.begin();
			it != cnode_modules_.end(); ++it)
	{
		CnodeModule* module = it->second;
		module->Close();
		delete module;
	}
	cnode_modules_.clear();

	// clear lnode
	for (WorkerVec::iterator it = workers_.begin();
			it != workers_.end(); ++it)
	{
		Worker* worker = *it;
		delete worker;
	}
	workers_.clear();
}

unsigned int Scheduler::SpawnNode(
		const char* name, 
		const char* config)
{
	if (name == nullptr)
		return 0;

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
	else
	{
		Worker* worker = GetWorkerByNodeId(nid);
		if (worker == nullptr)
			return;

		worker->CloseNode(nid);
	}
}

unsigned int Scheduler::GetCnodeId(const char* name)
{
	if (name == nullptr)
		return 0;

	std::string strname(name);
	for (CnodeInstMap::iterator it = cnodeinsts_.begin();
			it != cnodeinsts_.end(); ++it)
	{
		CnodeInst& inst = it->second;
		if (inst.name == strname)
			return it->first;
	}

	return 0;
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
		CnodeInstMap::iterator it = cnodeinsts_.find(msg.to);
		if (it != cnodeinsts_.end())
		{
			CnodeInst& inst = it->second;
			Cnode* node = inst.node;
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

bool Scheduler::LoadCnode(
		const std::string& libname, 
		unsigned int id, 
		const std::string& name, 
		const std::string& config)
{
	// find whether has the same name cnode
	for (CnodeInstMap::iterator it = cnodeinsts_.begin();
			it != cnodeinsts_.end(); ++it)
	{
		CnodeInst& inst = it->second;
		if (inst.name == name)
			return false;
	}
	
	// load module
	CnodeModule* module = nullptr;
	CnodeModuleMap::iterator it = cnode_modules_.find(libname);
	if (it == cnode_modules_.end())
	{
		CnodeModule* new_module = new CnodeModule;
		if (!new_module->Load(libname.c_str()))
		{
			delete module;
			return false;
		}

		cnode_modules_.insert(std::make_pair(libname, new_module));
		module = new_module;
	}
	else
	{
		module = it->second;
	}

	// create node
	Cnode* node = module->CreateCnode();
	if (node)
	{
		if (!node->Create(id, this, config.c_str()))
		{
			module->ReleaseCnode(node);
			return false;
		}

		CnodeInst inst;
		inst.name = name;
		inst.node = node;
		inst.module = module;
		cnodeinsts_.insert(std::make_pair(id, inst));
		return true;
	}

	return false;
}

