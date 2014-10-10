#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include "capi.h"
#include "node.h"
#include "worker.h"

#define MAX_NODE_COUNT	0x00FFFFFF

Worker::Worker(unsigned int id)
	: id_(id)
	, ls_(nullptr)
	, thread_(nullptr)
	, quit_(false)
	, nids_(id, MAX_NODE_COUNT)
{
}

Worker::~Worker()
{
}

bool Worker::Create()
{
	ls_ = luaL_newstate();
	if (ls_ == nullptr)
		return false;

	luaL_openlibs(ls_);

	lua_register(ls_, "spawnnode", C_SpawnNode);
	lua_register(ls_, "closenode", C_CloseNode);
	lua_register(ls_, "sendmsg", C_SendMsg);
	lua_register(ls_, "settimer", C_SetTimer);
	lua_register(ls_, "killtimer", C_KillTimer);

	quit_ = false;
	thread_ = new std::thread(std::bind(&Worker::Run, this));

	return true;
}

void Worker::Run()
{
	const int timeout = 50;
	while (!quit_)
	{
		Message msg;
		if (msgs_.Take(msg, timeout))
		{
			DispatchMsg(msg);
			free(msg.content);
		}

		ts_.OnTick();
	}
}

void Worker::Destroy()
{
	if (thread_)
	{
		Message msgquit;
		msgquit.type = MSG_TYPE_WORKER_QUIT;
		msgquit.size = 0;
		msgquit.content = nullptr;
		msgquit.to = id_;
		SendMsg(msgquit);

		thread_->join();
	}

	for (NodeMap::iterator it = nodes_.begin();
			it != nodes_.end(); ++it)
	{
		Node* node = it->second;
		node->Destroy();
		delete node;
	}

	if (ls_)
	{
		lua_close(ls_);
		ls_ = nullptr;
	}
}

void Worker::DispatchMsg(const Message& msg)
{
	if (msg.to == id_)
	{
		ProcessMsg(msg);
	}
	else
	{
		NodeMap::iterator it = nodes_.find(msg.to);
		if (it == nodes_.end())
			return;

		Node* node = it->second;
		if (node == nullptr)
			return;

		node->ProcessMsg(msg);
	}
}

void Worker::ProcessMsg(const Message& msg)
{
	switch (msg.type)
	{
		case MSG_TYPE_WORKER_QUIT:
			quit_ = true;
			break;
		case MSG_TYPE_WORKER_CREATENODE:
			{
			CreateNodeST* ptr = (CreateNodeST*)msg.content;
			std::string class_name(ptr->name);
			std::string file_name(class_name);
			transform(class_name.begin(), class_name.end(), 
					file_name.begin(), tolower);
			file_name += ".lua";
			CreateNode(ptr->id, file_name, class_name);
			free(ptr->name);
			}
			break;
		case MSG_TYPE_WORKER_DESTROYNODE:
			{
			DestroyNodeST* ptr = (DestroyNodeST*)msg.content;
			DestroyNode(ptr->id);
			}
			break;
		case MSG_TYPE_WORKER_CREATETIMER:
			{
			CreateTimerST* ptr = (CreateTimerST*)msg.content;
			CreateTimer(ptr->nid, ptr->tid, ptr->interval);
			}
			break;
		case MSG_TYPE_WORKER_DESTROYTIMER:
			{
			DestroyTimerST* ptr = (DestroyTimerST*)msg.content;
			DestroyTimer(ptr->nid, ptr->tid);
			}
			break;
		default:
			break;
	}
}

unsigned int Worker::SpawnNode(const std::string& node_name)
{
	unsigned int nid = 0;

	idmutex_.lock();
	nid = nids_.Assign();
	idmutex_.unlock();
	if (nid == 0)
		return 0;

	// use a new lua state to load this node implement
	// to check whether this node is completly implement
	// TODO
	
	// send create_node message
	Message msg_create_node;
	msg_create_node.type = MSG_TYPE_WORKER_CREATENODE;
	msg_create_node.size = sizeof(CreateNodeST);
	CreateNodeST* ptr = (CreateNodeST*)malloc(sizeof(CreateNodeST));
	ptr->id = nid;
	ptr->name = strdup(node_name.c_str());
	msg_create_node.content = (char*)ptr;
	msg_create_node.to = id_;
	SendMsg(msg_create_node);

	return nid;
}

void Worker::CloseNode(unsigned int nid)
{
	Message msg_destroy_node;
	msg_destroy_node.type = MSG_TYPE_WORKER_DESTROYNODE;
	msg_destroy_node.size = sizeof(DestroyNodeST);
	DestroyNodeST* ptr = (DestroyNodeST*)malloc(sizeof(DestroyNodeST));
	ptr->id = nid;
	msg_destroy_node.content = (char*)ptr;
	msg_destroy_node.to = id_;
	SendMsg(msg_destroy_node);
}

void Worker::SendMsg(const Message& msg)
{
	msgs_.Add(msg);
}

unsigned int Worker::SetTimer(unsigned int nid, int interval)
{
	unsigned int tid = ts_.AssignTimerId();
	if (tid == 0)
		return 0;

	Message msg_create_timer;
	msg_create_timer.type = MSG_TYPE_WORKER_CREATETIMER;
	msg_create_timer.size = sizeof(CreateTimerST);
	CreateTimerST* ptr = (CreateTimerST*)malloc(sizeof(CreateTimerST));
	ptr->nid = nid;
	ptr->tid = tid;
	ptr->interval = interval;
	msg_create_timer.content = (char*)ptr;
	msg_create_timer.to = id_;
	SendMsg(msg_create_timer);

	return tid;
}

void Worker::KillTimer(unsigned int nid, unsigned int tid)
{
	Message msg_destroy_timer;
	msg_destroy_timer.type = MSG_TYPE_WORKER_DESTROYTIMER;
	msg_destroy_timer.size = sizeof(DestroyTimerST);
	DestroyTimerST* ptr = (DestroyTimerST*)malloc(sizeof(DestroyTimerST));
	ptr->nid = nid;
	ptr->tid = tid;
	msg_destroy_timer.content = (char*)ptr;
	msg_destroy_timer.to = id_;
	SendMsg(msg_destroy_timer);
}

bool Worker::LoadLuaNode(const std::string& file, const std::string& node,
		Worker::LuaNodeCache& cache)
{
	LuaNodeCacheMap::iterator it = lua_node_caches_.find(file);
	if (it != lua_node_caches_.end())
	{
		cache = it->second;
	}
	else
	{
		luaL_dofile(ls_, file.c_str());
		lua_getglobal(ls_, node.c_str());
		if (lua_istable(ls_, -1) == 0)
		{
			lua_pop(ls_, 1);
			return false;
		}

		lua_getfield(ls_, -1, "New");
		if (lua_isnil(ls_, -1) == 1)
		{
			lua_pop(ls_, 2);
			return false;
		}

		int ref = luaL_ref(ls_, LUA_REGISTRYINDEX);
		lua_pop(ls_, 1);	// global node

		LuaNodeCache newcache;
		newcache.refnew = ref;
		lua_node_caches_.insert(std::make_pair(file, newcache));
		cache = newcache;
	}

	return true;
}

void Worker::CreateNode(unsigned int nid, const std::string& srcfile, 
		const std::string& class_name)
{
	LuaNodeCache cache;
	if (!LoadLuaNode(srcfile, class_name, cache))
	{
		// TODO
		return ;
	}

	Node* node = new Node(nid);
	if (node->Create(ls_, class_name, cache.refnew))
	{
		nodes_.insert(std::make_pair(nid, node));
	}
	else
	{
		// TODO
	}
}

void Worker::DestroyNode(unsigned int nid)
{
	NodeMap::iterator it = nodes_.find(nid);
	if (it == nodes_.end())
		return;

	Node* node = it->second;
	nodes_.erase(it);

	node->Destroy();
	delete node;

	idmutex_.lock();
	nids_.Recycle(nid);
	idmutex_.unlock();
}

void Worker::CreateTimer(unsigned int nid, unsigned int tid, int interval)
{
	NodeMap::iterator it = nodes_.find(nid);
	if (it == nodes_.end())
		return;

	Node* node = it->second;
	ts_.CreateTimer(tid, interval, node);
}

void Worker::DestroyTimer(unsigned int nid, unsigned int tid)
{
	NodeMap::iterator it = nodes_.find(nid);
	if (it == nodes_.end())
		return;

	Node* node = it->second;
	ts_.DestroyTimer(tid, node);
}

