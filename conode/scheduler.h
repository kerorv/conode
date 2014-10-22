#pragma once

#include <vector>
#include <map>
#include <string>
#include <atomic>
#include "message.h"
#include "msgrouter.h"

class Worker;
class CnodeManager;
class Scheduler : public MsgRouter
{
public:
	Scheduler();
	~Scheduler();

	bool Create(int worker_count, const char* mainnode);
	void Close();

	unsigned int SpawnNode(const std::string& name, const std::string& config);
	void CloseNode(unsigned int nid);
	void* SpawnCnode(const std::string& name, const std::string& config);
	void CloseCnode(void* ptr);
	// MsgRouter implement
	virtual void SendMsg(const Message& msg);
	unsigned int SetTimer(unsigned int nid, int interval);
	void KillTimer(unsigned int nid, unsigned int tid);

private:
	int NextWorkerIdx();
	Worker* GetWorkerByNodeId(unsigned int nid);

private:
	int worker_count_;
	std::string main_node_;
	unsigned int main_node_id_;
	typedef std::vector<Worker*> WorkerVec;
	WorkerVec workers_;
	std::atomic<unsigned int> counter_;
	CnodeManager* cnodemgr_;
};

