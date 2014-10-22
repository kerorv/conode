#include <stdio.h>
#include <stdlib.h>
#include "timerservice.h"
#include "lnode.h"

Lnode::Lnode(unsigned int id)
	: id_(id)
	, ref_(LUA_NOREF)
{
}

Lnode::~Lnode()
{
}

bool Lnode::Create(
		lua_State* L,
		const std::string& class_name, 
		const std::string& config,
		int refnew)
{
	ls_ = L;
	name_ = class_name;

	// call SomeLnode:New
	// return node(it's a table) if ok
	if (!CallNew(config.c_str(), refnew))
		return false;

	// check return is a table
	if (lua_istable(ls_, -1) == 0)
	{
		lua_pop(ls_, -1);
		return false;
	}
	
	// make reference of node then pop the node
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	if (ref == LUA_REFNIL || ref == LUA_NOREF)
		return false;

	ref_ = ref;
	return true;
}

void Lnode::Destroy()
{
	CallRelease();

	// remove timers
	for (TimerList::iterator it = timers_.begin();
			it != timers_.end(); ++it)
	{
		Timer* timer = *it;
		timer->interval = -1;
	}

	timers_.clear();

	// free ref
	if (ref_ != LUA_NOREF)
	{
		luaL_unref(ls_, LUA_REGISTRYINDEX, ref_);
		ref_ = LUA_NOREF;
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

void Lnode::AddTimer(Timer* timer)
{
	if (timer == nullptr)
		return;

	timers_.push_back(timer);
}

void Lnode::RemoveTimer(unsigned int tid)
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

bool Lnode::CallNew(const char* config, int ref)
{
	if (ref == LUA_NOREF || ref == LUA_REFNIL)
		return false;

	lua_rawgeti(ls_, LUA_REGISTRYINDEX, ref);
	lua_pushinteger(ls_, id_);
	lua_pushstring(ls_, config);
	if (lua_pcall(ls_, 2, 1, 0) != LUA_OK)
	{
		// TODO
		// const char* err = lua_tostring(ls_, -1);
		lua_pop(ls_, -1);	// pop error message
		return false;
	}

	return true;
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

