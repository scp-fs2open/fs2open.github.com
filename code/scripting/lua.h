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

//*************************Lua globals*************************
extern scripting::ade_obj<object_h> l_Object;
extern scripting::ade_obj<object_h> l_Weapon;
extern scripting::ade_obj<object_h> l_Ship;
extern scripting::ade_obj<object_h> l_Debris;
extern scripting::ade_obj<object_h> l_Asteroid;

#endif //_LUA_H
