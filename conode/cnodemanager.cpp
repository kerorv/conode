#include <string.h>
#include <functional>
#include "cnode.h"
#include "msgdefine.h"
#include "cnodemodule.h"
#include "cnodemanager.h"

#define MAX_CNODE_COUNT	0x00FFFFFE	// 1 is scheduler

CnodeManager::CnodeManager()
	: router_(nullptr)
	, quit_(false)
	, thread_(nullptr)
	, ids_(2, MAX_CNODE_COUNT)
{
}

CnodeManager::~CnodeManager()
{
}

bool CnodeManager::Create(MsgRouter* router)
{
	router_ = router;
	quit_ = false;
	thread_ = new std::thread(std::bind(&CnodeManager::Run, this));
	return true;
}

void CnodeManager::Destroy()
{
	if (thread_)
	{
		Message msgquit;
		msgquit.type = MSG_TYPE_CNODEMGR_QUIT;
		msgquit.size = 0;
		msgquit.content = nullptr;
		msgquit.to = 1;
		SendMsg(msgquit);

		thread_->join();
		delete thread_;
		thread_ = nullptr;
	}

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
}

void CnodeManager::Run()
{
	const int timeout = 50;
	while (!quit_)
	{
		Message msg;
		if (msgs_.Take(msg, timeout))
		{
			if (msg.to == 1) // self
			{
				ProcessMsg(msg);
				free(msg.content);
			}
			else // forwarding to cnode
			{
				RouteCnodeMsg(msg);
				// Don't free msg.content
				// cnode will free it
			}
		}
	}
}

unsigned int CnodeManager::SpawnNode(
		const char* name, 
		const char* config)
{
	unsigned int id = 0;

	id_mutex_.lock();
	id = ids_.Assign();
	id_mutex_.unlock();

	if (id != 0)
	{
		Message msg_create_node;
		msg_create_node.type = MSG_TYPE_CNODEMGR_CREATENODE;
		msg_create_node.size = sizeof(CreateNodeST);
		CreateNodeST* ptr = (CreateNodeST*)malloc(sizeof(CreateNodeST));
		ptr->id = id;
		ptr->name = strdup(name);
		if (config == nullptr)
			ptr->config = nullptr;
		else
			ptr->config = strdup(config);
		msg_create_node.content = (char*)ptr;
		msg_create_node.to = 1;	// scheduler
		SendMsg(msg_create_node);
	}

	return id;
}

void CnodeManager::CloseNode(unsigned int id)
{
	Message msg_destroy_node;
	msg_destroy_node.type = MSG_TYPE_CNODEMGR_DESTROYNODE;
	msg_destroy_node.size = sizeof(DestroyNodeST);
	DestroyNodeST* ptr = (DestroyNodeST*)malloc(sizeof(DestroyNodeST));
	ptr->id = id;
	msg_destroy_node.content = (char*)ptr;
	msg_destroy_node.to = 1;	// scheduler
	SendMsg(msg_destroy_node);
}

void CnodeManager::SendMsg(const Message& msg)
{
	msgs_.Add(msg);
}

void CnodeManager::ProcessMsg(const Message& msg)
{
	switch (msg.type)
	{
		case MSG_TYPE_CNODEMGR_QUIT:
			{
			quit_ = true;
			}
			break;
		case MSG_TYPE_CNODEMGR_CREATENODE:
			{
			CreateNodeST* ptr = (CreateNodeST*)msg.content;
			CreateCnode(ptr->id, ptr->name, ptr->config);
			free(ptr->name);
			free(ptr->config);
			}
			break;
		case MSG_TYPE_CNODEMGR_DESTROYNODE:
			{
			DestroyNodeST* ptr = (DestroyNodeST*)msg.content;
			DestroyCnode(ptr->id);
			}
			break;
		default:
			break;
	}
}

bool CnodeManager::CreateCnode(
		unsigned int id, 
		const char* name,
		const char* config)
{
	CnodeModule* module = nullptr;
	CnodeModuleMap::iterator it = cnode_modules_.find(name);
	if (it == cnode_modules_.end())
	{
		CnodeModule* new_module = new CnodeModule;
		if (!new_module->Load(name))
		{
			delete module;
			return false;
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
		if (!node->Create(id, router_, config))
		{
			module->ReleaseCnode(node);
			return false;
		}

		CnodeInst inst;
		inst.node = node;
		inst.module = module;
		cnodeinsts_.insert(std::make_pair(id, inst));
		return true;
	}

	return false;
}

void CnodeManager::DestroyCnode(unsigned int id)
{
	CnodeInstMap::iterator it = cnodeinsts_.find(id);
	if (it != cnodeinsts_.end())
	{
		CnodeInst& inst = it->second;
		Cnode* node = inst.node;
		CnodeModule* module = inst.module;
		node->Close();
		module->ReleaseCnode(node);

		cnodeinsts_.erase(it);
		id_mutex_.lock();
		ids_.Recycle(id);
		id_mutex_.unlock();
	}
}

void CnodeManager::RouteCnodeMsg(const Message& msg)
{
	CnodeInstMap::iterator it = cnodeinsts_.find(msg.to);
	if (it != cnodeinsts_.end())
	{
		CnodeInst& inst = it->second;
		Cnode* node = inst.node;
		node->OnMessage(msg);
	}
}

