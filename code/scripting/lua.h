#ifndef _LUA_H
#define _LUA_H

extern "C" {
	#include <lauxlib.h>
	#include <lualib.h>
}

#include "globalincs/pstypes.h"
#include "menuui/mainhallmenu.h"
#include "object/object.h"
#include "scripting/ade.h"
#include "scripting/api/object.h"
#include "scripting/api/asteroid.h"

//*************************Lua globals*************************
using scripting::api::l_Object;
extern scripting::ade_obj<object_h> l_Weapon;
extern scripting::ade_obj<object_h> l_Ship;
extern scripting::ade_obj<object_h> l_Debris;
using scripting::api::l_Asteroid;

#endif //_LUA_H
