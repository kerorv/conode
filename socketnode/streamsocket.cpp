#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "socketserver.h"
#include "streamsocket.h"

StreamSocket::StreamSocket(int fd, SocketServer* server)
	: fd_(fd)
	, server_(server)
	, poll_write_(false)
	, msglen_(0)
	, haslen_(0)
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

void StreamSocket::SendMsg(const char* msg, int length)
{
	if (length > UINT16_MAX)
	{
		// log
		// TODO
		return;
	}

	OutMsg outmsg;
	outmsg.len = length;
	outmsg.data = msg;
	outmsg.send_bytes = 0;
	outmsg.status = StatusSendHeader;
	outmsgs_.push_back(outmsg);

	// out message list limit
	// TODO

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
	char* buff = (char*)&msglen_ + haslen_;
	int size = sizeof(uint16_t) - haslen_;

	int len = ReadBlock(buff, size);
	if (len > 0)
	{
		haslen_ += len;
		if (haslen_ == sizeof(uint16_t))
		{
			msgbody_ = (char*)malloc(msglen_);
			haslen_ = 0;
			rstatus_ = StatusReadBody;
		}
	}

	return len;
}

int StreamSocket::ReadBody()
{
	char* buff = msgbody_ + haslen_;
	int size = msglen_ - haslen_;

	int len = ReadBlock(buff, size);
	if (len > 0)
	{
		haslen_ += len;
		if (msglen_ == haslen_)
		{
			server_->OnClientMsg(msgbody_, msglen_);

			msgbody_ = NULL;
			msglen_ = 0;
			haslen_ = 0;
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
		OutMsg& msg = *it;
		for (;msg.status != StatusSendOk;)
		{
			switch (msg.status)
			{
				case StatusSendHeader:
					{
						int ret = SendHeader(msg);
						if (ret <= 0)
							return ret;
					}
					break;
				case StatusSendBody:
					{
						int ret = SendBody(msg);
						if (ret <= 0)
							return ret;
					}
					break;
				default:
					return -1;
					break;
			}
		}

		if (msg.status == StatusSendOk)
		{
			free(const_cast<char*>(msg.data));
			it = outmsgs_.erase(it);
		}
	}

	return 1;
}

int StreamSocket::SendHeader(OutMsg& msg)
{
	const char* buff = (const char*)&msg.len + msg.send_bytes;
	int size = sizeof(uint16_t) - msg.send_bytes;

	int len = SendBlock(buff, size);
	if (len > 0)
	{
		msg.send_bytes += len;
		if (msg.send_bytes == sizeof(uint16_t))
		{
			msg.send_bytes = 0;
			msg.status = StatusSendBody;
		}
	}

	return len;
}

int StreamSocket::SendBody(OutMsg& msg)
{
	const char* buff = msg.data + msg.send_bytes;
	int size = msg.len - msg.send_bytes;

	int len = SendBlock(buff, size);
	if (len > 0)
	{
		msg.send_bytes += len;
		if (msg.len == msg.send_bytes)
		{
			msg.status = StatusSendOk;
		}
	}

	return len;
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

