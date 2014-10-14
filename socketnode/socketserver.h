#pragma once

#include <map>
#include <list>
#include <deque>
#include <thread>
#include <mutex>
#include "cnode.h"
#include "msgrouter.h"
#include "epoll.h"
#include "listensocket.h"
#include "streamsocket.h"

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
	void SendClientMsg(const Message& msg);

	int SpawnId();
	void RecycleId(int id);
	void AddSocket(StreamSocket* ss);
	void CloseSocket(StreamSocket* ss);
	void RemoveSocket(int id);
	void UpdateSocketWriteEvent(StreamSocket* ss, bool add);

	void OnClientMsg(char* msg, int len);
	void OnNodeMsg(const Message& msg);

	StreamSocket* FindConnection(int sid);
	int ParseConfig(const char* config);

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
};

