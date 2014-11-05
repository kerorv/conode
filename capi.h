#pragma once

#include "lua.hpp"

int C_SpawnNode(lua_State* L);
int C_CloseNode(lua_State* L);
int C_CnodeId(lua_State* L);
int C_SendMsg(lua_State* L);
int C_SetTimer(lua_State* L);
int C_KillTimer(lua_State* L);

