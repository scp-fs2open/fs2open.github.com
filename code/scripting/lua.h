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
#include "scripting/api/debris.h"
#include "scripting/api/ship.h"
#include "scripting/api/weapon.h"

//*************************Lua globals*************************
using scripting::api::l_Object;
using scripting::api::l_Weapon;
using scripting::api::l_Ship;
using scripting::api::l_Debris;
using scripting::api::l_Asteroid;

#endif //_LUA_H
