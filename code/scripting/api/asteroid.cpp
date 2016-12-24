//
//

#include "asteroid.h"
#include "object.h"
#include "vecmath.h"
#include "asteroid/asteroid.h"
#include "scripting/lua.h"

extern int ade_set_object_with_breed(lua_State *L, int obj_idx);

namespace scripting {
namespace api {

//**********HANDLE: Asteroid
ADE_OBJ_DERIV(l_Asteroid, object_h, "asteroid", "Asteroid handle", l_Object);

ADE_VIRTVAR(Target, l_Asteroid, "object", "Asteroid target object; may be object derivative, such as ship.", "object", "Target object, or invalid handle if asteroid handle is invalid")
{
	object_h *oh = NULL;
	object_h *th = NULL;
	if(!ade_get_args(L, "o|o", l_Asteroid.GetPtr(&oh), l_Object.GetPtr(&th)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	asteroid *asp = &Asteroids[oh->objp->instance];

	if(ADE_SETTING_VAR && th != NULL) {
		if(th->IsValid())
			asp->target_objnum = OBJ_INDEX(th->objp);
		else
			asp->target_objnum = -1;
	}

	if(asp->target_objnum > 0 && asp->target_objnum < MAX_OBJECTS)
		return ade_set_object_with_breed(L, asp->target_objnum);
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

}

ADE_FUNC(kill, l_Asteroid, "[ship killer=nil, wvector hitpos=nil]", "Kills the asteroid. Set \"killer\" to designate a specific ship as having been the killer, and \"hitpos\" to specify the world position of the hit location; if nil, the asteroid center is used.", "boolean", "True if successful, false or nil otherwise")
{
	object_h *victim,*killer=NULL;
	vec3d *hitpos=NULL;
	if(!ade_get_args(L, "o|oo", l_Asteroid.GetPtr(&victim), l_Ship.GetPtr(&killer), l_Vector.GetPtr(&hitpos)))
		return ADE_RETURN_NIL;

	if(!victim->IsValid())
		return ADE_RETURN_NIL;

	if(killer != NULL && !killer->IsValid())
		return ADE_RETURN_NIL;

	if (!hitpos)
		hitpos = &victim->objp->pos;

	if (killer)
		asteroid_hit(victim->objp, killer->objp, hitpos, victim->objp->hull_strength + 1);
	else
		asteroid_hit(victim->objp, NULL,         hitpos, victim->objp->hull_strength + 1);

	return ADE_RETURN_TRUE;
}

}
}
