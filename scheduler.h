#pragma once

#include <vector>
#include <map>
#include <string>
#include <atomic>
#include "message.h"
#include "msgrouter.h"

class Worker;
class CnodeModule;
class Cnode;
class Scheduler : public MsgRouter
{
public:
	static Scheduler* GetInstance();

	bool Create(size_t worker_count, const char* main_node);
	void Close();

	unsigned int SpawnNode(const std::string& node_name);
	void CloseNode(unsigned int nid);
	// MsgRouter implement
	virtual void SendMsg(const Message& msg);
	unsigned int SetTimer(unsigned int nid, int interval);
	void KillTimer(unsigned int nid, unsigned int tid);

private:
	Scheduler();
	~Scheduler();

	int NextWorkerIdx();
	Worker* GetWorkerByNodeId(unsigned int nid);

	// Cnode
	Cnode* LoadCnode(const std::string& name, unsigned int id, 
		const std::string& config);

private:
	int worker_count_;
	std::string main_node_;
	unsigned int main_node_id_;
	typedef std::vector<Worker*> WorkerVec;
	WorkerVec workers_;
	std::atomic<unsigned int> counter_;
	typedef std::map<std::string, CnodeModule*> CnodeModuleMap;
	CnodeModuleMap cnode_modules_;
	typedef std::map<unsigned int, Cnode*> CnodeMap;
	CnodeMap cnodes_;
};

