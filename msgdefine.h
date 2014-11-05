#pragma once

// 0
#define MSG_TYPE_INVALID					0

// 1~999 for framework
#define MSG_TYPE_WORKER_QUIT				1
#define MSG_TYPE_WORKER_CREATENODE			2
#define MSG_TYPE_WORKER_DESTROYNODE			3

// 1000~ for user define

struct CreateNodeST
{
	unsigned int id;
	void* node;
};

struct DestroyNodeST
{
	unsigned int id;
};

