#include <stdlib.h>
#include <string.h>
#include <string>
#include "scheduler.h"
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

	const char* sz1 = luaL_checkstring(L, 1);
	const char* sz2 = luaL_checkstring(L, 2);
	std::string name(sz1);
	std::string config(sz2);
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
	// timer_id = settimer(node_id, interval)
	Scheduler* sched = (Scheduler*)lua_touserdata(L, lua_upvalueindex(1));
	if (sched == nullptr)
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	unsigned int nid = (unsigned int)luaL_checkint(L, 1);
	int interval = luaL_checkint(L, 2);
	unsigned int timer_id = sched->SetTimer(nid, interval);
	lua_pushinteger(L, (int)timer_id);
	return 1;
}

int C_KillTimer(lua_State* L)
{
	// lua code:
	// killtimer(node_id, timer_id)
	Scheduler* sched = (Scheduler*)lua_touserdata(L, lua_upvalueindex(1));
	if (sched == nullptr)
		return 0;

	unsigned int nid = (unsigned int)luaL_checkint(L, 1);
	unsigned int tid = luaL_checkint(L, 2);
	sched->KillTimer(nid, tid);
	return 0;
}
