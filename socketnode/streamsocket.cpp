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
}

void StreamSocket::Close()
{
	if (fd_ != -1)
	{
		close(fd_);
		fd_ = -1;
	}
}

void StreamSocket::OnEvent(int events)
{
	if (events & EventError)
	{
		Close();
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
	for (bool loop = false; loop;)
	{
		switch (rstatus_)
		{
			case StatusReadHeader:
				loop = ReadHeader();
				break;
			case StatusReadBody:
				loop = ReadBody();
				break;
			default:
				// assert(false);
				loop = false;
				break;
		}
	}
}

bool StreamSocket::ReadHeader()
{
	char* buff = (char*)&msglen_ + haslen_;
	int size = sizeof(uint16_t) - haslen_;

	int ret = recv(fd_, buff, size, 0);
	switch (ret)
	{
		case 0:
			{
				server_->CloseSocket(this, CS_REASON_RECV);
				return false;
			}
			break;
		case -1:
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				{
					// do nothing
				}
				else
				{
					server_->CloseSocket(this, CS_REASON_RECV);
				}
				return false;
			}
			break;
		default:
			break;
	}

	haslen_ += ret;
	if (haslen_ == 2)
	{
		msgbody_ = (char*)malloc(msglen_);
		haslen_ = 0;
		rstatus_ = StatusReadBody;
	}

	return true;
}

bool StreamSocket::ReadBody()
{
	char* buff = msgbody_ + haslen_;
	int size = msglen_ - haslen_;

	int ret = recv(fd_, buff, size, 0);
	switch (ret)
	{
		case 0:
			{
				server_->CloseSocket(this, CS_REASON_RECV);
				return false;
			}
			break;
		case -1:
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				{
					// do nothing
				}
				else
				{
					server_->CloseSocket(this, CS_REASON_RECV);
				}
				return false;
			}
			break;
		default:
			break;
	}

	haslen_ += ret;
	if (msglen_ == haslen_)
	{
		server_->OnClientMsg(msgbody_, msglen_);

		msgbody_ = NULL;
		msglen_ = 0;
		haslen_ = 0;
		rstatus_ = StatusReadHeader;
	}

	return true;
}

void StreamSocket::OnWrite()
{
	SendList();

	if (outmsgs_.size() == 0 && poll_write_)
	{
		server_->UpdateSocketWriteEvent(this, false);
		poll_write_ = false;
	}
}

void StreamSocket::SendMsg(const char* msg, int length)
{
	if (length > UINT16_MAX)
		return;

	OutMsg outmsg;
	outmsg.len = length;
	outmsg.data = msg;
	outmsg.send_bytes = 0;
	outmsgs_.push_back(outmsg);

	// out message list limit
	// TODO
	
	SendList();

	if (outmsgs_.size() > 0 && !poll_write_)
	{
		server_->UpdateSocketWriteEvent(this, true);
		poll_write_ = true;
	}
}

void StreamSocket::SendList()
{
	for (OutMsgList::iterator it = outmsgs_.begin();
			it != outmsgs_.end(); )
	{
		OutMsg& msg = *it;
		if (!SendBlock(msg.data, msg.len, msg.send_bytes))
		{
			server_->CloseSocket(this, CS_REASON_SEND);
			return;
		}
		else
		{
			if (msg.len != msg.send_bytes)
				break;

			free(const_cast<char*>(msg.data));
			it = outmsgs_.erase(it);
		}
	}
}

bool StreamSocket::SendBlock(const char* data, int size, int& has_send)
{
	while (size < has_send)
	{
		int ret = send(fd_, data + has_send, size - has_send, 0);
		if (ret > 0)
		{
			has_send += ret;
		}
		else if (ret == 0)
		{
			break;
		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				break;
			else
				return false;
		}
	}

	return true;
}

