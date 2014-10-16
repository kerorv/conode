#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "socketserver.h"
#include "streamsocket.h"

StreamSocket::StreamSocket(int fd, SocketServer* server)
	: fd_(fd)
	, server_(server)
	, poll_write_(false)
	, has_send_(0)
	, msglen_(0)
	, has_read_(0)
	, rstatus_(StatusReadHeader)
{
	id_ = server_->SpawnId();
}

StreamSocket::~StreamSocket()
{
	Close();
}

void StreamSocket::Close()
{
	if (fd_ != -1)
	{
		close(fd_);
		fd_ = -1;
	}
}

void StreamSocket::SendMsg(int msgtype, const char* msg, int length)
{
	if (msgtype > UINT16_MAX || length + sizeof(uint16_t) > UINT16_MAX)
	{
		// log
		// TODO
		return;
	}

	// out message list limit
	// TODO

	NetMsg* nmsg = (NetMsg*)malloc(sizeof(NetMsg) + length);
	nmsg->size = sizeof(uint16_t) + length;
	nmsg->type = (uint16_t)msgtype;
	memcpy(nmsg->content, msg, length);
	outmsgs_.push_back(nmsg);
//	OutMsg outmsg;
//	outmsg.len = length;
//	outmsg.data = msg;
//	outmsg.send_bytes = 0;
//	outmsg.status = StatusSendHeader;
//	outmsgs_.push_back(outmsg);

	if (SendList() == -1)
	{
		server_->CloseSocket(this, CS_REASON_SEND);
		return;
	}

	if (outmsgs_.size() > 0 && !poll_write_)
	{
		server_->UpdateSocketWriteEvent(this, true);
		poll_write_ = true;
	}
}

void StreamSocket::OnEvent(int events)
{
	if (events & EventError)
	{
		server_->CloseSocket(this, CS_REASON_EVENT);
		return;
	}

	if (events & EventRead)
	{
		OnRead();
	}

	if (events & EventWrite)
	{
		OnWrite();
	}
}

void StreamSocket::OnRead()
{
	if (!this->IsActive())
		return;

	for (;;)
	{
		switch (rstatus_)
		{
			case StatusReadHeader:
				{
					int ret = ReadHeader();
					if (ret <= 0)
					{
						if (ret == -1)
						{
							server_->CloseSocket(this, CS_REASON_READ);
						}
						return;
					}
				}
				break;
			case StatusReadBody:
				{
					int ret = ReadBody();
					if (ret <= 0)
					{
						if (ret == -1)
						{
							server_->CloseSocket(this, CS_REASON_READ);
						}
						return;
					}
				}
				break;
			default:
				return;
				break;
		}
	}
}

void StreamSocket::OnWrite()
{
	if (!this->IsActive())
		return;

	if (SendList() == -1)
	{
		server_->CloseSocket(this, CS_REASON_SEND);
		return;
	}

	if (outmsgs_.size() == 0 && poll_write_)
	{
		server_->UpdateSocketWriteEvent(this, false);
		poll_write_ = false;
	}
}

int StreamSocket::ReadHeader()
{
	char* buff = (char*)&msglen_ + has_read_;
	int size = sizeof(uint16_t) - has_read_;

	int len = ReadBlock(buff, size);
	if (len > 0)
	{
		has_read_ += len;
		if (has_read_ == sizeof(uint16_t))
		{
			msgbody_ = (char*)malloc(msglen_);
			has_read_ = 0;
			rstatus_ = StatusReadBody;
		}
	}

	return len;
}

int StreamSocket::ReadBody()
{
	char* buff = msgbody_ + has_read_;
	int size = msglen_ - has_read_;

	int len = ReadBlock(buff, size);
	if (len > 0)
	{
		has_read_ += len;
		if (msglen_ == has_read_)
		{
			server_->OnClientMsg(msgbody_, msglen_);

			msgbody_ = NULL;
			msglen_ = 0;
			has_read_ = 0;
			rstatus_ = StatusReadHeader;
		}
	}

	return len;
}

// return
// -1: fail or peer close socket
// 0: pending
// N: recv bytes
int StreamSocket::ReadBlock(char* block, size_t size)
{
	int ret = recv(fd_, block, size, 0);
	if (ret > 0)
	{
		return ret;
	}
	else if (ret == 0)
	{
		return -1;
	}
	else
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
}

// return
// -1: io fail
// 0: io pending
// >0: normal
int StreamSocket::SendList()
{
	for (OutMsgList::iterator it = outmsgs_.begin();
			it != outmsgs_.end(); )
	{
		NetMsg* msg = *it;

		for (;;)
		{
			const char* buff = (const char*)msg + has_send_;
			int size = msg->size + 2/*LENGTH field's len*/ - has_send_;
			int ret = SendBlock(buff, size);
			if (ret <= 0)
				return ret;

			has_send_ += ret;
			if (has_send_ == msg->size + 2)
			{
				has_send_ = 0;
				free(msg);
				it = outmsgs_.erase(it);
				break;
			}
		}
	}

	return 1;
}

// return
// -1:fail
// 0: pending
// N: send bytes
int StreamSocket::SendBlock(const char* block, int size)
{
	int ret = send(fd_, block, size, 0);
	if (ret >= 0)
	{
		return ret;
	}
	else
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			return 0;
		else
			return -1;
	}
}

