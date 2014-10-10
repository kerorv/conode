#pragma once

#include <string>
#include <map>
#include <mutex>
#include <thread>
#include "lua.hpp"
#include "message.h"
#include "concurrentqueue.h"
#include "idallocator.h"
#include "timerservice.h"

class Node;
class Worker
{
public:
	Worker(unsigned int id);
	~Worker();

	bool Create();
	void Run();
	void Destroy();

	unsigned int SpawnNode(const std::string& node_name);
	void CloseNode(unsigned int nid);
	void SendMsg(const Message& msg);
	unsigned int SetTimer(unsigned nid, int interval);
	void KillTimer(unsigned int nid, unsigned int tid);

	unsigned int GetId() const { return id_; }

private:
	void DispatchMsg(const Message& msg);
	void ProcessMsg(const Message& msg);
	void CreateNode(unsigned int nid, const std::string& srcfile, 
		const std::string& class_name);
	void DestroyNode(unsigned int nid);
	void CreateTimer(unsigned int nid, unsigned int tid, int interval);
	void DestroyTimer(unsigned int nid, unsigned int tid);
	struct LuaNodeCache;
	bool LoadLuaNode(const std::string& file, const std::string& node, 
			Worker::LuaNodeCache& cache);

private:
	unsigned int id_;
	lua_State* ls_;
	std::thread* thread_;
	bool quit_;
	typedef std::map<int, Node*> NodeMap;
	NodeMap nodes_;
	struct LuaNodeCache
	{
		int refnew;
	};
	typedef std::map<std::string, LuaNodeCache> LuaNodeCacheMap;
	LuaNodeCacheMap lua_node_caches_;
	MsgQueue msgs_;
	IdAllocator nids_;
	std::mutex idmutex_;
	TimerService ts_;
};

