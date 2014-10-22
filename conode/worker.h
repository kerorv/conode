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

class Lnode;
class Worker
{
public:
	Worker(unsigned int id);
	~Worker();

	bool Create(void* sched);
	void Run();
	void Destroy();

	unsigned int SpawnNode(const std::string& name, const std::string& config);
	void CloseNode(unsigned int nid);
	void SendMsg(const Message& msg);
	unsigned int SetTimer(unsigned nid, int interval);
	void KillTimer(unsigned int nid, unsigned int tid);

	unsigned int GetId() const { return id_; }

private:
	void DispatchMsg(const Message& msg);
	void ProcessMsg(const Message& msg);
	void CreateNode(unsigned int nid, const std::string& srcfile, 
		const std::string& class_name, const std::string& config);
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
	typedef std::map<int, Lnode*> LnodeMap;
	LnodeMap nodes_;
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

