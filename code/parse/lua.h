#ifndef _LUA_H
#define _LUA_H
#ifdef USE_LUA

extern "C" {
	#include "lauxlib.h"
	#include "lualib.h"
}

//Used to parse arguments on the stack to C values
int lua_get_args(lua_State *L, char *fmt, ...);
int lua_set_args(lua_State *L, char* fmt, ...);

//WMC - Hack to allow for quick&easy return value parsing
extern int Lua_get_args_skip;

extern void lua_stackdump(lua_State *L, char *stackdump);

#endif //USE_LUA
#endif //_LUA_H
