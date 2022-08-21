#include "lua_ext.h"

#define linit_c
#define LUA_LIB

#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"


// NOTE: The last item in this array MUST be {NULL, NULL}.
static const luaL_Reg lualibs_ext[] = {
  {LUA_BITLIBNAME, luaopen_bit},
  {NULL, NULL}
};

LUALIB_API void luaL_openlibs_ext (lua_State *L) {
  const luaL_Reg *lib = lualibs_ext;
  for (; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
}
