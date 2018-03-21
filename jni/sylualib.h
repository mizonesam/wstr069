#ifndef SYLUALIB_H
#define SYLUALIB_H
#include <string.h>
#include "./thirdLib/lua/lualib.h"
#include "./thirdLib/lua/lauxlib.h"
#include "./thirdLib/lua/lua.h"
#define LUA_TYPE_NUMBER 0
#define LUA_TYPE_STRING 1
void addCFunc(lua_State* L, lua_CFunction fun, char* name);
void callLuaFunc(lua_State* L, const char* func, const char *sig, ...);

#endif
