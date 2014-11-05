#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "lnode.h"
#include "capi.h"

int C_SpawnNode(lua_State* L)
{
	// lua code:
	// node_id = spawnnode(name, config)
	// -- name is node's name
	// -- config is node's param(can be nil)
	// -- return node id
	Scheduler* sched = (Scheduler*)lua_touserdata(L, lua_upvalueindex(1));
	if (sched == nullptr)
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	const char* name = luaL_checkstring(L, 1);
	const char* config = luaL_checkstring(L, 2);
	unsigned int node_id = sched->SpawnNode(name, config);
	lua_pushinteger(L, (int)node_id);
	return 1;
}

int C_CloseNode(lua_State* L)
{
	// lua code:
	// closenode(node_id)
	Scheduler* sched = (Scheduler*)lua_touserdata(L, lua_upvalueindex(1));
	if (sched == nullptr)
		return 0;

	unsigned int node_id = (unsigned int)luaL_checkint(L, 1);
	sched->CloseNode(node_id);
	return 0;
}

int C_CnodeId(lua_State* L)
{
	// lua code:
	// node_id = cnodeid(name)
	Scheduler* sched = (Scheduler*)lua_touserdata(L, lua_upvalueindex(1));
	if (sched == nullptr)
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	const char* name = luaL_checkstring(L, 1);
	unsigned int cnode_id = sched->GetCnodeId(name);
	lua_pushinteger(L, (int)cnode_id);
	return 1;
}

int C_SendMsg(lua_State* L)
{
	// lua code:
	// sendmsg(node_id, type, content)
	Scheduler* sched = (Scheduler*)lua_touserdata(L, lua_upvalueindex(1));
	if (sched == nullptr)
		return 0;

	unsigned int node_id = (unsigned int)luaL_checkint(L, 1);
	int type = luaL_checkint(L, 2);
	size_t len = 0;
	const char* content = luaL_checklstring(L, 3, &len);

	Message msg;
	msg.type = type;
	msg.size = len;
	msg.content = (char*)malloc(len);
	memcpy(msg.content, content, len);
	msg.to = node_id;
	sched->SendMsg(msg);
	return 0;
}

int C_SetTimer(lua_State* L)
{
	// lua code:
	// timer_id = settimer(interval)
	Lnode* node = (Lnode*)lua_touserdata(L, lua_upvalueindex(1));
	if (node == nullptr)
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	int interval = luaL_checkint(L, 1);
	unsigned int timer_id = node->SetTimer(interval);
	lua_pushinteger(L, (int)timer_id);
	return 1;
}

int C_KillTimer(lua_State* L)
{
	// lua code:
	// killtimer(timer_id)
	Lnode* node = (Lnode*)lua_touserdata(L, lua_upvalueindex(1));
	if (node == nullptr)
		return 0;

	unsigned int tid = luaL_checkint(L, 1);
	node->KillTimer(tid);
	return 0;
}

