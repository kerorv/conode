#include <stdlib.h>
#include <string.h>
#include <string>
#include "scheduler.h"
#include "capi.h"

int C_SpawnNode(lua_State* L)
{
	// lua code:
	// node_id = spawnnode(entity)	-- entity is a string which is the classname 
	//								-- of node's subclass
	//								-- return node id
	const char* sz = luaL_checkstring(L, 1);
	std::string name(sz);
	unsigned int node_id = Scheduler::GetInstance()->SpawnNode(name);
	lua_pushinteger(L, (int)node_id);

	return 1;
}

int C_CloseNode(lua_State* L)
{
	// lua code:
	// closenode(node_id)
	unsigned int node_id = (unsigned int)luaL_checkint(L, 1);
	Scheduler::GetInstance()->CloseNode(node_id);
	return 0;
}

int C_SendMsg(lua_State* L)
{
	// lua code:
	// sendmsg(node_id, type, content)
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

	Scheduler::GetInstance()->SendMsg(msg);
	return 0;
}

int C_SetTimer(lua_State* L)
{
	// lua code:
	// timer_id = settimer(node_id, interval)
	unsigned int nid = (unsigned int)luaL_checkint(L, 1);
	int interval = luaL_checkint(L, 2);
	unsigned int timer_id = Scheduler::GetInstance()->SetTimer(nid, interval);
	lua_pushinteger(L, (int)timer_id);
	return 1;
}

int C_KillTimer(lua_State* L)
{
	// lua code:
	// killtimer(node_id, timer_id)
	unsigned int nid = (unsigned int)luaL_checkint(L, 1);
	unsigned int tid = luaL_checkint(L, 2);
	Scheduler::GetInstance()->KillTimer(nid, tid);
	return 0;
}

