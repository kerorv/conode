#include <string.h>
#include <stdlib.h>
#include <vector>
#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "socketsendclientmsg.h"
#include "socketserver.h"

#define DEFAULT_LISTEN_PORT		8021
#define POLL_INTERVAL			10	// ms

SocketServer::SocketServer()
	: maxid_(0)
	, stop_(false)
	, nid_(0)
	, router_(nullptr)
	, thread_(nullptr)
{
}

SocketServer::~SocketServer()
{
	Close();
}

int SocketServer::ParseConfig(const char* config)
{
	char* pos = strstr(config, "port=");
	if (pos == nullptr)
		return 0;
	pos += 5;

	int port = strtol(pos, nullptr, 10);
	return port;
}

bool SocketServer::Create(unsigned int id, MsgRouter* router,
		const char* config)
{
	int port = ParseConfig(config);
	if (port == 0)
		port = DEFAULT_LISTEN_PORT;

	if (!listener_.Create(port, this))
		return false;

	if (!poller_.Create())
		return false;

	poller_.RegisteHandler(&listener_, EventRead);
	nid_ = id;
	router_ = router;
	stop_ = false;
	thread_ = new std::thread(std::bind(&SocketServer::Run, this));
	return true;
}

void SocketServer::Close()
{
	stop_ = true;

	if (thread_)
	{
		thread_->join();
		delete thread_;
		thread_ = nullptr;
	}

	poller_.Close();
	listener_.Close();
}

void SocketServer::Run()
{
	typedef std::vector<Message> MsgVector;
	MsgVector sendmsgs;
	sendmsgs.reserve(64);

	while (!stop_)
	{
		poller_.Poll(POLL_INTERVAL);

		msg_mutex_.lock();
		while (msgq_.size() > 0)
		{
			sendmsgs.push_back(msgq_.front());
			msgq_.pop_front();
		}
		msg_mutex_.unlock();

		for (MsgVector::iterator it = sendmsgs.begin();
				it != sendmsgs.end(); ++it)
		{
			const Message& msg = *it;
			OnNodeMsg(msg);
		}
		sendmsgs.clear();
	}
}

void SocketServer::SendClientMsg(const Message& msg)
{
	std::lock_guard<std::mutex> lock(msg_mutex_);
	msgq_.push_back(msg);
}

int SocketServer::SpawnId()
{
	int id = 0;
	if (rids_.size() == 0)
	{
		id = ++maxid_;
	}
	else
	{
		id = rids_.front();
		rids_.pop_front();
	}

	return id;
}

void SocketServer::RecycleId(int id)
{
	rids_.push_back(id);
}

void SocketServer::AddSocket(StreamSocket* ss)
{
	poller_.RegisteHandler(ss, EventRead);
	conns_.insert(std::make_pair(ss->GetId(), ss));
}

void SocketServer::CloseSocket(StreamSocket* ss)
{
	if (ss == NULL)
		return;

	ss->Close();
	poller_.UnregisteHandler(ss);

	// notify upper layer
	// TODO
}

void SocketServer::RemoveSocket(int id)
{
	ConnectionMap::iterator it = conns_.find(id);
	if (it == conns_.end())
		return;

	StreamSocket* ss = it->second;
	if (ss)
	{
		delete ss;
	}

	conns_.erase(it);
	RecycleId(id);
}

void SocketServer::UpdateSocketWriteEvent(StreamSocket* ss, bool add)
{
	int events = EventRead;
	if (add)
	{
		events |= EventWrite;
	}

	poller_.UpdateHandler(ss, events);
}

StreamSocket* SocketServer::FindConnection(int sid)
{
	ConnectionMap::iterator it = conns_.find(sid);
	if (it == conns_.end())
		return nullptr;

	return it->second;
}

void SocketServer::OnClientMsg(char* msg, int len)
{
	if (router_ == nullptr)
		return;

	Message inmsg;
	inmsg.type = MSG_TYPE_SOCKET_RECVCLIENTMSG;
	inmsg.size = len;
	inmsg.content = msg;
	inmsg.to = 1;	// scheduler
	router_->SendMsg(inmsg);
}

void SocketServer::OnNodeMsg(const Message& msg)
{
	switch (msg.type)
	{
		case MSG_TYPE_SOCKET_SENDCLIENTMSG:
			{
				SocketSendClientMsg real_msg;
				// if (!sp_.Decode(real_msg, msg.content, msg.size))
				//     return;
				StreamSocket* ss = FindConnection(real_msg.GetSid());
				if (ss == nullptr)
					return;

				int outmsglen = 0;
				const char* outmsg = real_msg.MoveMsg(outmsglen);
				if (outmsg && outmsglen > 0)
					ss->SendMsg(outmsg, outmsglen);
			}
			break;
		default:
			break;
	}
}

