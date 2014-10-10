#pragma once

class EventHandler;
struct epoll_event;
class Epoll
{
public:
	Epoll();
	~Epoll();

	bool Create();
	void Close();
	int Poll(int timeout);

	void RegisteHandler(EventHandler* handler, int events);
	void UnregisteHandler(EventHandler* handler);
	void UpdateHandler(EventHandler* handler, int events);

private:
	int efd_;
	struct epoll_event* events_;
};

