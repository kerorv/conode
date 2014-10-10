#include <stdio.h>
#include <stdlib.h>
#include "timerservice.h"
#include "node.h"

Node::Node(unsigned int id)
	: id_(id)
	, ref_(LUA_NOREF)
{
}

Node::~Node()
{
}

bool Node::Create(lua_State* L,	const std::string& class_name, int refnew)
{
	ls_ = L;
	name_ = class_name;

	// call SomeNode:New
	if (!CallNew(refnew))
		return false;

	// check return is a table
	if (lua_istable(ls_, -1) == 0)
	{
		lua_pop(ls_, -1);
		return false;
	}
	
	// make reference of entity
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	if (ref == LUA_REFNIL || ref == LUA_NOREF)
		return false;

	ref_ = ref;
	return true;
}

void Node::Destroy()
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

void Node::ProcessMsg(const Message& msg)
{
	// call Entity:onmessage
	CallOnMessage(msg);
}

void Node::OnTimer(unsigned int tid)
{
	CallOnTimer(tid);
}

void Node::AddTimer(Timer* timer)
{
	if (timer == nullptr)
		return;

	timers_.push_back(timer);
}

void Node::RemoveTimer(unsigned int tid)
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

bool Node::CallNew(int ref)
{
	if (ref == LUA_NOREF || ref == LUA_REFNIL)
		return false;

	lua_rawgeti(ls_, LUA_REGISTRYINDEX, ref);
	lua_pushinteger(ls_, id_);
	if (lua_pcall(ls_, 1, 1, 0) != LUA_OK)
	{
		// TODO
		// const char* err = lua_tostring(ls_, -1);
		lua_pop(ls_, -1);	// pop error message
		return false;
	}

	return true;
/*
	lua_getglobal(ls_, name_.c_str());
	if (lua_istable(ls_, -1) == 0)
	{
		lua_pop(ls_, 1);
		return false;
	}

	lua_pushstring(ls_, "new");
	lua_gettable(ls_, -2);
	if (lua_isfunction(ls_, -1) == 0)
	{
		lua_pop(ls_, 2);
		return false;
	}

	lua_insert(ls_, 1);
	lua_pushinteger(ls_, id_);
	if (lua_pcall(ls_, 2, 1, 0) != 0)
	{
		// log error message
		// TODO
		// const char* err = lua_tostring(ls_, -1);
		lua_pop(ls_, -1);
		return false;
	}

	return true;*/
}

void Node::CallRelease()
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

void Node::CallOnMessage(const Message& msg)
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

void Node::CallOnTimer(unsigned int tid)
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

