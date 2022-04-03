//
//

#include "testing.h"

#include "scripting/api/objs/vecmath.h"
#include "scripting/api/objs/enums.h"
#include "scripting/api/objs/texture.h"
#include "scripting/api/objs/object.h"
#include "scripting/api/objs/particle.h"

#include "scripting/lua/LuaValue.h"

#include "scripting/api/objs/bytearray.h"
#include "scripting/api/objs/audio_stream.h"
#include "sound/audiostr.h"

#include "physics/physics.h"
#include "graphics/2d.h"
#include "io/timer.h"
#include "particle/particle.h"
#include "playerman/player.h"
#include "cutscene/movie.h"
#include "network/multi_options.h"


namespace scripting {
namespace api {

//*************************Testing stuff*************************
//This section is for stuff that's considered experimental.
ADE_LIB(l_Testing, "Testing", "ts", "Experimental or testing stuff");

ADE_FUNC(openAudioStreamMem,
	l_Testing,
	"string snddata, enumeration stream_type /* AUDIOSTREAM_* values */",
	"Opens an audio stream of the specified in-memory file contents and type.",
	"audio_stream",
	"A handle to the opened stream or invalid on error")
{
	luacpp::LuaValue snddata_arr(L);
	enum_h streamTypeEnum;
	if (!ade_get_args(L, "ao", &snddata_arr, l_Enum.Get(&streamTypeEnum))) {
		return ade_set_args(L, "o", l_AudioStream.Set(-1));
	}

	int streamType;
	switch (streamTypeEnum.index) {
	case LE_ASF_EVENTMUSIC:
		streamType = ASF_EVENTMUSIC;
		break;
	case LE_ASF_MENUMUSIC:
		streamType = ASF_MENUMUSIC;
		break;
	case LE_ASF_VOICE:
		streamType = ASF_VOICE;
		break;
	default:
		LuaError(L, "Invalid audio stream type %d.", streamTypeEnum.index);
		return ade_set_args(L, "o", l_AudioStream.Set(-1));
	}
	
	
	if (!snddata_arr.pushValue(L))
	    return ade_set_args(L, "o", l_AudioStream.Set(-1));
    if (!lua_isstring(L, -1)) {
        lua_pop(L, 1);
        LuaError(L, "Expected binary string containing audio.");
        return ade_set_args(L, "o", l_AudioStream.Set(-1));
    }
    
    size_t snd_len;
    auto snddata = lua_tolstring(L, -1, &snd_len);
	
    int ah = audiostream_open_mem(reinterpret_cast<const uint8_t *>(snddata), snd_len, streamType);
	lua_pop(L, 1);
    if (ah < 0) {
        LuaError(L,"Stream could not be opened. Did you pass valid audio?");
        return ade_set_args(L, "o", l_AudioStream.Set(-1));
    }

	return ade_set_args(L, "o", l_AudioStream.Set(ah));
}


ADE_FUNC(avdTest, l_Testing, NULL, "Test the AVD Physics code", NULL, NULL)
{
	(void)L; // unused parameter

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

ADE_FUNC_DEPRECATED(createParticle,
	l_Testing,
	"vector Position, vector Velocity, number Lifetime, number Radius, enumeration Type, [number "
	"TracerLength=-1, boolean Reverse=false, texture Texture=Nil, object AttachedObject=Nil]",
	"Creates a particle. Use PARTICLE_* enumerations for type."
	"Reverse reverse animation, if one is specified"
	"Attached object specifies object that Position will be (and always be) relative to.",
	"particle",
	"Handle to the created particle",
	gameversion::version(19, 0, 0, 0),
	"Not available in the testing library anymore. Use gr.createPersistentParticle instead.")
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
	texture_h* texture = nullptr;
	if (!ade_get_args(L, "ooffo|fboo", l_Vector.Get(&pi.pos), l_Vector.Get(&pi.vel), &pi.lifetime, &pi.rad,
	                  l_Enum.GetPtr(&type), &temp, &rev, l_Texture.GetPtr(&texture), l_Object.GetPtr(&objh)))
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
			    if (texture == nullptr || !texture->isValid()) {
				    LuaError(L, "Invalid texture specified for createParticle()!");
				    return ADE_RETURN_NIL;
			    } else {
				    pi.optional_data = texture->handle;
				    pi.type          = particle::PARTICLE_BITMAP;
			    }
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

	particle::WeakParticlePtr p = particle::createPersistent(&pi);

	if (!p.expired())
		return ade_set_args(L, "o", l_Particle.Set(particle_h(p)));
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
	if(!(Multi_options_g.pxo))
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(playCutscene, l_Testing, NULL, "Forces a cutscene by the specified filename string to play. Should really only be used in a non-gameplay state (i.e. start of GS_STATE_BRIEFING) otherwise odd side effects may occur. Highly Experimental.", "string", NULL)
{
	//This whole thing is a quick hack and can probably be done way better, but is currently functioning fine for my purposes.
	const char* filename;

	if (!ade_get_args(L, "s", &filename))
		return ADE_RETURN_FALSE;

	movie::play(filename);

	return ADE_RETURN_TRUE;
}


}
}

