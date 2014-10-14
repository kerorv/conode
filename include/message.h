#pragma once

#include <stdint.h>

struct Message
{
	int32_t type;
	int32_t size;
	char* content;
	unsigned int to;
};

// 0
#define MSG_TYPE_INVALID				0

// 1~9999 for framework
#define MSG_TYPE_WORKER_QUIT				1
#define MSG_TYPE_WORKER_CREATENODE			2
#define MSG_TYPE_WORKER_DESTROYNODE			3
#define MSG_TYPE_WORKER_CREATETIMER			4
#define MSG_TYPE_WORKER_DESTROYTIMER		5
#define MSG_TYPE_SOCKET_SENDCLIENTMSG		11
#define MSG_TYPE_SOCKET_RECVCLIENTMSG		12
#define MSG_TYPE_SOCKET_CONNECTIONBREAK		13
#define MSG_TYPE_SOCKET_KICKOFFCONNECTION	14
// 10000~ for user define

struct CreateNodeST
{
	unsigned int id;
	char* name;
};

struct DestroyNodeST
{
	unsigned int id;
};

struct CreateTimerST
{
	unsigned int nid;
	unsigned int tid;
	int interval;
};

struct DestroyTimerST
{
	unsigned int nid;
	unsigned int tid;
};

