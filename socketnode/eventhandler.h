#pragma once

enum PollEvent
{
	EventNone = 0,
	EventError = 1,
	EventRead = 2,
	EventWrite = 4,
};

class EventHandler
{
public:
	virtual ~EventHandler() {}

	virtual int GetFd() = 0;
	virtual void OnEvent(int events) = 0;
};

