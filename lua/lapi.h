/*
** $Id: lapi.h,v 1.2 2007-01-04 04:46:13 Goober5000 Exp $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#include "lobject.h"


LUAI_FUNC void luaA_pushobject (lua_State *L, const TValue *o);

#endif
