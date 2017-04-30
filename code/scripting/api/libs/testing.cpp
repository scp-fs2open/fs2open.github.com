//
//

#include "testing.h"

#include "scripting/api/objs/vecmath.h"
#include "scripting/api/objs/enums.h"
#include "scripting/api/objs/texture.h"
#include "scripting/api/objs/object.h"
#include "scripting/api/objs/particle.h"

#include "physics/physics.h"
#include "graphics/2d.h"
#include "io/timer.h"
#include "particle/particle.h"
#include "playerman/player.h"
#include "cutscene/movie.h"

// Om_tracker_flag should already be set in FreeSpace.cpp, needed to determine if PXO is enabled from the registry
extern int Om_tracker_flag; // needed for FS2OpenPXO config

namespace scripting {
namespace api {

//*************************Testing stuff*************************
//This section is for stuff that's considered experimental.
ADE_LIB(l_Testing, "Testing", "ts", "Experimental or testing stuff");

ADE_FUNC(avdTest, l_Testing, NULL, "Test the AVD Physics code", NULL, NULL)
{
	static bool initialized = false;
	static avd_movement avd;

	if(!initialized)
	{
		avd.setAVD(10.0f, 3.0f, 1.0f, 1.0f, 0.0f);
		initialized = true;
	}
	for(int i = 0; i < 3000; i++)
	{
		float Pc, Vc;
		avd.get((float)i/1000.0f, &Pc, &Vc);
		gr_set_color(0, 255, 0);
		gr_pixel(i/10, gr_screen.clip_bottom - (int)(Pc*10.0f), GR_RESIZE_NONE);
		gr_set_color(255, 0, 0);
		gr_pixel(i/10, gr_screen.clip_bottom - (int)(Vc*10.0f), GR_RESIZE_NONE);

		avd.get(&Pc, &Vc);
		gr_set_color(255, 255, 255);
		gr_pixel((timestamp()%3000)/10, gr_screen.clip_bottom - (int)(Pc*10.0f), GR_RESIZE_NONE);
		gr_pixel((timestamp()%3000)/10, gr_screen.clip_bottom - (int)(Vc*10.0f), GR_RESIZE_NONE);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(createParticle, l_Testing, "vector Position, vector Velocity, number Lifetime, number Radius, enumeration Type, [number Tracer length=-1, boolean Reverse=false, texture Texture=Nil, object Attached Object=Nil]",
		 "Creates a particle. Use PARTICLE_* enumerations for type."
			 "Reverse reverse animation, if one is specified"
			 "Attached object specifies object that Position will be (and always be) relative to.",
		 "particle",
		 "Handle to the created particle")
{
	particle::particle_info pi;
	pi.type = particle::PARTICLE_DEBUG;
	pi.optional_data = -1;
	pi.attached_objnum = -1;
	pi.attached_sig = -1;
	pi.reverse = 0;

	// Need to consume tracer_length parameter but it isn't used anymore
	float temp;

	enum_h *type = NULL;
	bool rev=false;
	object_h *objh=NULL;
	if(!ade_get_args(L, "ooffo|fboo", l_Vector.Get(&pi.pos), l_Vector.Get(&pi.vel), &pi.lifetime, &pi.rad, l_Enum.GetPtr(&type), &temp, &rev, l_Texture.Get((int*)&pi.optional_data), l_Object.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(type != NULL)
	{
		switch(type->index)
		{
			case LE_PARTICLE_DEBUG:
				pi.type = particle::PARTICLE_DEBUG;
				break;
			case LE_PARTICLE_FIRE:
				pi.type = particle::PARTICLE_FIRE;
				break;
			case LE_PARTICLE_SMOKE:
				pi.type = particle::PARTICLE_SMOKE;
				break;
			case LE_PARTICLE_SMOKE2:
				pi.type = particle::PARTICLE_SMOKE2;
				break;
			case LE_PARTICLE_BITMAP:
				if (pi.optional_data < 0)
				{
					LuaError(L, "Invalid texture specified for createParticle()!");
				}

				pi.type = particle::PARTICLE_BITMAP;
				break;
		}
	}

	if(rev)
		pi.reverse = 0;

	if(objh != NULL && objh->IsValid())
	{
		pi.attached_objnum = OBJ_INDEX(objh->objp);
		pi.attached_sig = objh->objp->signature;
	}

	particle::WeakParticlePtr p = particle::create(&pi);

	if (!p.expired())
		return ade_set_args(L, "o", l_Particle.Set(new particle_h(p)));
	else
		return ADE_RETURN_NIL;
}

ADE_FUNC(getStack, l_Testing, NULL, "Generates an ADE stackdump", "string", "Current Lua stack")
{
	char buf[10240] = {'\0'};
	ade_stackdump(L, buf);
	return ade_set_args(L, "s", buf);
}

ADE_FUNC(isCurrentPlayerMulti, l_Testing, NULL, "Returns whether current player is a multiplayer pilot or not.", "boolean", "Whether current player is a multiplayer pilot or not")
{
	if(Player == NULL)
		return ade_set_error(L, "b", false);

	if(!(Player->flags & PLAYER_FLAGS_IS_MULTI))
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(isPXOEnabled, l_Testing, NULL, "Returns whether PXO is currently enabled in the configuration.", "boolean", "Whether PXO is enabled or not")
{
	if(!(Om_tracker_flag))
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(playCutscene, l_Testing, NULL, "Forces a cutscene by the specified filename string to play. Should really only be used in a non-gameplay state (i.e. start of GS_STATE_BRIEFING) otherwise odd side effects may occur. Highly Experimental.", "string", NULL)
{
	//This whole thing is a quick hack and can probably be done way better, but is currently functioning fine for my purposes.
	char *filename;

	if (!ade_get_args(L, "s", &filename))
		return ADE_RETURN_FALSE;

	movie::play(filename);

	return ADE_RETURN_TRUE;
}


}
}

