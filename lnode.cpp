#include <stdio.h>
#include <stdlib.h>
#include "capi.h"
#include "timermanager.h"
#include "worker.h"
#include "lnode.h"

Lnode::Lnode(Worker* worker, unsigned int id)
	: worker_(worker)
	, id_(id)
	, ref_(LUA_NOREF)
{
}

Lnode::~Lnode()
{
}

bool Lnode::Create(
		const char* file,
		const char* name,
		const char* config)
{
	ls_ = luaL_newstate();
	if (ls_ == nullptr)
		return false;

	luaL_openlibs(ls_);
	// register api
	RegisterApi();

	luaL_dofile(ls_, file);
	// new node
	lua_newtable(ls_);
	// load metatable
	lua_getglobal(ls_, name);
	if (lua_gettop(ls_) != 2 && lua_istable(ls_, -1) == 0)
	{
		lua_close(ls_);
		ls_ = nullptr;
		return false;
	}
	lua_setmetatable(ls_, 1);
	// make reference of node
	int ref = luaL_ref(ls_, LUA_REGISTRYINDEX);
	if (ref == LUA_REFNIL || ref == LUA_NOREF)
	{
		lua_close(ls_);
		ls_ = nullptr;
		return false;
	}
	ref_ = ref;
	// call init
	CallInit(id_, config);
	return true;
}

void Lnode::Destroy()
{
	// call release
	CallRelease();

	// remove timers
	for (TimerList::iterator it = timers_.begin();
			it != timers_.end(); ++it)
	{
		Timer* timer = *it;
		timer->interval = -1;
	}

	timers_.clear();

	// close vm
	if (ls_)
	{
		lua_close(ls_);
		ls_ = nullptr;
	}
}

void Lnode::ProcessMsg(const Message& msg)
{
	// call Entity:onmessage
	CallOnMessage(msg);
}

void Lnode::OnTimer(unsigned int tid)
{
	CallOnTimer(tid);
}

unsigned int Lnode::SetTimer(int interval)
{
	Timer* timer = worker_->GetTimerManager()->CreateTimer(interval, this);
	if (timer == nullptr)
		return 0;

	timers_.push_back(timer);
	return timer->id;
}

void Lnode::KillTimer(unsigned int tid)
{
	for (TimerList::iterator it = timers_.begin();
			it != timers_.end(); ++it)
	{
		Timer* timer = *it;
		if (timer->id == tid)
		{
			timer->interval = -1; // set remove flag
			timers_.erase(it);
			break;
		}
	}
}

static void RegisterFunction(
		lua_State* L, 
		const char* name,
		lua_CFunction fn, 
		void* upvalue)
{
	lua_pushlightuserdata(L, upvalue);
	lua_pushcclosure(L, fn, 1);
	lua_setfield(L, 1, name);
}

void Lnode::RegisterApi()
{
	lua_createtable(ls_, 0, 6);

	RegisterFunction(ls_, "spawnnode", C_SpawnNode, worker_->GetScheduler());
	RegisterFunction(ls_, "closenode", C_CloseNode, worker_->GetScheduler());
	RegisterFunction(ls_, "cnodeid", C_CnodeId, worker_->GetScheduler());
	RegisterFunction(ls_, "sendmsg", C_SendMsg, worker_->GetScheduler());
	RegisterFunction(ls_, "settimer", C_SetTimer, this);
	RegisterFunction(ls_, "killtimer", C_KillTimer, this);

	lua_setglobal(ls_, "conode");
}

void Lnode::CallInit(unsigned int id, const char* config)
{
	if (ref_ == LUA_NOREF)
		return;

	lua_rawgeti(ls_, LUA_REGISTRYINDEX, ref_);
	lua_getfield(ls_, -1, "Init");
	if (lua_isfunction(ls_, -1) == 0)
	{
		lua_pop(ls_, 2);
		return;
	}

	lua_insert(ls_, 1);
	lua_pushinteger(ls_, id);
	lua_pushstring(ls_, config);
	if (lua_pcall(ls_, 4, 0, 0) != LUA_OK)
	{
		// log error message
		// TODO
		lua_pop(ls_, -1);	// pop error message
	}
}

void Lnode::CallRelease()
{
	if (ref_ == LUA_NOREF)
		return;

	lua_rawgeti(ls_, LUA_REGISTRYINDEX, ref_);
	lua_getfield(ls_, -1, "Release");
	if (lua_isfunction(ls_, -1) == 0)
	{
		lua_pop(ls_, 2);
		return;
	}

	lua_insert(ls_, 1);
	if (lua_pcall(ls_, 1, 0, 0) != LUA_OK)
	{
		// log error message
		// TODO
		lua_pop(ls_, -1);	// pop error message
	}
}

void Lnode::CallOnMessage(const Message& msg)
{
	if (ref_ == LUA_NOREF)
		return;

	lua_rawgeti(ls_, LUA_REGISTRYINDEX, ref_);
	lua_getfield(ls_, -1, "OnMessage");
	if (lua_isfunction(ls_, -1) == 0)
	{
		lua_pop(ls_, 2);
		return;
	}

	lua_insert(ls_, 1);
	lua_pushinteger(ls_, msg.type);
	lua_pushlightuserdata(ls_, msg.content);
	lua_pushinteger(ls_, msg.size);
	if (lua_pcall(ls_, 4, 0, 0) != LUA_OK)
	{
		// log error message
		// TODO
		lua_pop(ls_, -1);	// pop error message
	}
}

void Lnode::CallOnTimer(unsigned int tid)
{
	if (ref_ == LUA_NOREF)
		return;

	lua_rawgeti(ls_, LUA_REGISTRYINDEX, ref_);
	lua_getfield(ls_, -1, "OnTimer");
	if (lua_isfunction(ls_, -1) == 0)
	{
		lua_pop(ls_, 2);
		return;
	}

	lua_insert(ls_, 1);
	lua_pushinteger(ls_, (int)tid);
	if (lua_pcall(ls_, 2, 0, 0) != LUA_OK)
	{
		// log error message
		// TODO
		lua_pop(ls_, -1);	// pop error message
	}
}

