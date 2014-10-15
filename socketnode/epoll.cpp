#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "eventhandler.h"
#include "epoll.hpp"

#define MAX_EVENTS 64

Epoll::Epoll()
	: efd_(-1)
{
	events_ = (struct epoll_event*)malloc(MAX_EVENTS * 
			sizeof(struct epoll_event));
}

Epoll::~Epoll()
{
	Close();
	free(events_);
}

bool Epoll::Create()
{
	int fd = epoll_create(1);
	if (fd == -1)
		return false;

	efd_ = fd;
	return true;
}

void Epoll::Close()
{
	if (efd_ == -1)
		return;

	close(efd_);
	efd_ = -1;
}

int Epoll::Poll(int timeout)
{
	int nfds = epoll_wait(efd_, events_, MAX_EVENTS, timeout);
	for (int i = 0; i < nfds; ++i)
	{
		EventHandler* handler = (EventHandler*)events_[i].data.ptr;
		if (handler == NULL)
			continue;

		int events = 0;
		if ((events_[i].events & EPOLLERR) || (events_[i].events & EPOLLHUP))
		{
			events = EventError;
		}
		else
		{
			if (events_[i].events & EPOLLIN)
			{
				events |= EventRead;
			}

			if (events_[i].events & EPOLLOUT)
			{
				events |= EventWrite;
			}
		}
		
		if (events != 0)
		{
			handler->OnEvent(events);
		}
	}

	return nfds;
}

void Epoll::RegisteHandler(EventHandler* handler, int events)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.data.fd = handler->GetFd();
	ev.data.ptr = handler;
	if (events & EventRead)
		ev.events |= EPOLLIN;
	if (events & EventWrite)
		ev.events |= EPOLLOUT;

	epoll_ctl(efd_, EPOLL_CTL_ADD, handler->GetFd(), &ev);
}

void Epoll::UnregisteHandler(EventHandler* handler)
{
	epoll_ctl(efd_, EPOLL_CTL_DEL, handler->GetFd(), NULL);
}

void Epoll::UpdateHandler(EventHandler* handler, int events)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.data.fd = handler->GetFd();
	ev.data.ptr = handler;
	if (events & EventRead)
		ev.events |= EPOLLIN;
	if (events & EventWrite)
		ev.events |= EPOLLOUT;

	epoll_ctl(efd_, EPOLL_CTL_MOD, handler->GetFd(), &ev);
}

