#pragma once

#include <vector>
#include <map>
#include <string>
#include <atomic>
#include "lua.hpp"
#include "message.h"
#include "cnode.h"
#include "msgrouter.h"

class Worker;
class CnodeModule;
class Scheduler : public MsgRouter
{
public:
	Scheduler();
	~Scheduler();

	bool Create();
	void Close();

	unsigned int SpawnNode(const char* name, const char* config);
	void CloseNode(unsigned int nid);
	unsigned int GetCnodeId(const char* name);
	// MsgRouter implement
	virtual void SendMsg(const Message& msg);

private:
	int NextWorkerIdx();
	Worker* GetWorkerByNodeId(unsigned int nid);
	bool LoadCnode(
			const std::string& libname, 
			unsigned int id, 
			const std::string& name, 
			const std::string& config);

private:
	unsigned int main_node_id_;
	typedef std::vector<Worker*> WorkerVec;
	WorkerVec workers_;
	std::atomic<unsigned int> counter_;

	// cnode
	struct CnodeInst
	{
		std::string name;
		Cnode* node;
		CnodeModule* module;
	};
	typedef std::map<std::string, CnodeModule*> CnodeModuleMap;
	CnodeModuleMap cnode_modules_;
	typedef std::map<unsigned int, CnodeInst> CnodeInstMap;
	CnodeInstMap cnodeinsts_;
};

