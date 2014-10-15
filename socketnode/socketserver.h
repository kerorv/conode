#pragma once

#include <map>
#include <list>
#include <deque>
#include <thread>
#include <mutex>
#include "cnode.h"
#include "msgrouter.h"
#include "cppsproto.h"
#include "epoll.hpp"
#include "listensocket.h"
#include "streamsocket.h"

// close socket reason
#define CS_REASON_EVENT		1
#define CS_REASON_READ		2
#define CS_REASON_SEND		3
#define CS_REASON_KICKOFF	4

class SocketServer : public Cnode
{
	friend class ListenSocket;
	friend class StreamSocket;
public:
	SocketServer();
	virtual ~SocketServer();

	// Cnode implement
	virtual bool Create(unsigned int id, MsgRouter* router, 
			const char* config);
	virtual void Close();
	virtual void OnMessage(const Message& msg);

protected:
	void Run();

	int SpawnId();
	void RecycleId(int id);
	void AddSocket(StreamSocket* ss);
	void CloseSocket(StreamSocket* ss, int reason);
	void RemoveSocket(int id);
	void UpdateSocketWriteEvent(StreamSocket* ss, bool add);

	void OnClientMsg(char* msg, int len);
	void OnNodeMsg(const Message& msg);

	StreamSocket* FindConnection(int sid);
	int ParseConfig(const char* config);
	CppSproto* LoadSproto();

private:
	ListenSocket listener_;
	Epoll poller_;
	typedef std::map<int, StreamSocket*> ConnectionMap;
	ConnectionMap conns_;
	typedef std::list<int> RecycleIdList;
	RecycleIdList rids_;
	int maxid_;
	volatile bool stop_;
	unsigned int nid_;
	MsgRouter* router_;
	std::thread* thread_;
	typedef std::deque<Message> MsgQueue;
	MsgQueue msgq_;
	std::mutex msg_mutex_;
	CppSproto* sp_;
};

