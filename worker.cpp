#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include "msgdefine.h"
#include "capi.h"
#include "lnode.h"
#include "worker.h"

#define MAX_NODE_COUNT	0x00FFFFFF

Worker::Worker(Scheduler* sched, unsigned int id)
	: sched_(sched)
	, id_(id)
	, thread_(nullptr)
	, quit_(false)
	, nids_(id + 1, MAX_NODE_COUNT)
	, tm_(id)
{
}

Worker::~Worker()
{
}

bool Worker::Create()
{
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

		tm_.OnTick();
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
		delete thread_;
		thread_ = nullptr;
	}

	for (LnodeMap::iterator it = nodes_.begin();
			it != nodes_.end(); ++it)
	{
		Lnode* node = it->second;
		node->Destroy();
		delete node;
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
		LnodeMap::iterator it = nodes_.find(msg.to);
		if (it == nodes_.end())
			return;

		Lnode* node = it->second;
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
				nodes_.insert(std::make_pair(ptr->id, (Lnode*)ptr->node));
			}
			break;
		case MSG_TYPE_WORKER_DESTROYNODE:
			{
				DestroyNodeST* ptr = (DestroyNodeST*)msg.content;
				unsigned int nid = ptr->id;
				LnodeMap::iterator it = nodes_.find(nid);
				if (it == nodes_.end())
					return;

				Lnode* node = it->second;
				nodes_.erase(it);

				node->Destroy();
				delete node;

				idmutex_.lock();
				nids_.Recycle(nid);
				idmutex_.unlock();
			}
			break;
		default:
			break;
	}
}

unsigned int Worker::SpawnNode(
		const char* name, 
		const char* config)
{
	unsigned int nid = 0;

	idmutex_.lock();
	nid = nids_.Assign();
	idmutex_.unlock();
	if (nid == 0)
		return 0;

	std::string class_name(name);
	std::string file_name(name);
	transform(class_name.begin(), class_name.end(), 
			file_name.begin(), tolower);
	file_name += ".lua";
	Lnode* node = new Lnode(this, nid);
	if (!node->Create(file_name.c_str(), class_name.c_str(), config))
	{
		delete node;
		idmutex_.lock();
		nids_.Recycle(nid);
		idmutex_.unlock();
		return 0;
	}
	
	// send create_node message
	Message msg_create_node;
	msg_create_node.to = id_; // to worker
	msg_create_node.type = MSG_TYPE_WORKER_CREATENODE;
	msg_create_node.size = sizeof(CreateNodeST);
	CreateNodeST* ptr = (CreateNodeST*)malloc(sizeof(CreateNodeST));
	ptr->id = nid;
	ptr->node = node;
	msg_create_node.content = (char*)ptr;
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

