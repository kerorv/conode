#pragma once

#include <string>
#include <map>
#include <thread>
#include <mutex>
#include "message.h"
#include "concurrentqueue.h"
#include "idallocator.h"

class Cnode;
class MsgRouter;
class CnodeModule;
struct CnodeInst
{
	Cnode* node;
	CnodeModule* module;
};

class CnodeManager
{
public:
	CnodeManager();
	~CnodeManager();

	bool Create(MsgRouter* router);
	void Destroy();
	unsigned int SpawnNode(
			const std::string& name, 
			const std::string& config);
	void CloseNode(unsigned int id);
	void SendMsg(const Message& msg);

private:
	void Run();
	void DispatchMsg(const Message& msg);

	bool CreateCnode(
			unsigned int id, 
			const std::string& name,
			const std::string& config);
	void DestroyCnode(unsigned int id);
	void RouteCnodeMsg(const Message& msg);

private:
	MsgRouter* router_;
	bool quit_;
	std::thread* thread_;
	MsgQueue msgs_;
	typedef std::map<std::string, CnodeModule*> CnodeModuleMap;
	CnodeModuleMap cnode_modules_;
	typedef std::map<unsigned int, CnodeInst> CnodeInstMap;
	CnodeInstMap cnodeinsts_;
	IdAllocator ids_;
	std::mutex id_mutex_;
};

