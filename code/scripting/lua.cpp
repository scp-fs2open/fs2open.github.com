#include "ai/ai.h"
#include "ai/aigoals.h"
#include "asteroid/asteroid.h"
#include "camera/camera.h"
#include "cfile/cfilesystem.h"
#include "cmdline/cmdline.h"
#include "cutscene/movie.h"
#include "debris/debris.h"
#include "freespace.h"
#include "gamesequence/gamesequence.h"
#include "globalincs/linklist.h"
#include "graphics/2d.h"
#include "graphics/font.h"
#include "graphics/generic.h"
#include "graphics/opengl/gropenglpostprocessing.h"
#include "headtracking/headtracking.h"
#include "hud/hudbrackets.h"
#include "hud/hudconfig.h"
#include "hud/hudescort.h"
#include "hud/hudets.h"
#include "hud/hudgauges.h"
#include "hud/hudshield.h"
#include "iff_defs/iff_defs.h"
#include "io/joy.h"
#include "io/key.h"
#include "io/mouse.h"
#include "io/timer.h"
#include "jumpnode/jumpnode.h"
#include "lighting/lighting.h"
#include "menuui/credits.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "mission/missionload.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h"
#include "mission/missiontraining.h"
#include "missionui/missionbrief.h"
#include "model/model.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "object/objectshield.h"
#include "object/waypoint.h"
#include "scripting/lua.h"
#include "parse/parselo.h"
#include "scripting/scripting.h"
#include "particle/particle.h"
#include "playerman/player.h"
#include "render/3d.h"
#include "render/3dinternal.h"
#include "scripting/ade_api.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"
#include "sound/audiostr.h"
#include "sound/ds.h"
#include "weapon/beam.h"
#include "weapon/weapon.h"

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"

#include "scripting/api/bitops.h"
#include "scripting/api/enums.h"
#include "scripting/api/vecmath.h"
#include "scripting/api/event.h"
#include "scripting/api/file.h"
#include "scripting/api/font.h"
#include "scripting/api/gameevent.h"
#include "scripting/api/gamestate.h"
#include "scripting/api/hudgauge.h"
#include "scripting/api/eye.h"
#include "scripting/api/model.h"
#include "scripting/api/physics_info.h"
#include "scripting/api/sexpvar.h"
#include "scripting/api/shields.h"
#include "scripting/api/shiptype.h"
#include "scripting/api/species.h"
#include "scripting/api/team.h"
#include "scripting/api/streaminganim.h"
#include "scripting/api/texture.h"
#include "scripting/api/texturemap.h"
#include "scripting/api/weaponclass.h"
#include "scripting/api/mc_info.h"
#include "scripting/api/object.h"
#include "scripting/api/asteroid.h"
#include "scripting/api/cockpit_display.h"
#include "scripting/api/shipclass.h"
#include "scripting/api/debris.h"
#include "scripting/api/waypoint.h"
#include "scripting/api/ship_bank.h"
#include "scripting/api/subsystem.h"
#include "scripting/api/order.h"
#include "scripting/api/ship.h"

using namespace scripting;
using namespace scripting::api;

int ade_set_object_with_breed(lua_State *L, int obj_idx);

//**********Handles
/*ade_obj<int> l_Camera("camera", "Camera handle");
ade_obj<int> l_Cmission("cmission", "Campaign mission handle"); //WMC - We can get away with a pointer right now, but if it ever goes dynamic, it'd be a prob
ade_obj<enum_h> l_Enum("enumeration", "Enumeration object");
ade_obj<int> l_Event("event", "Mission event handle");
ade_obj<int> l_Font("font", "font handle");
ade_obj<matrix_h> l_Matrix("orientation", "Orientation matrix object");
ade_obj<int> l_Model("model", "3D Model (POF) handle");
ade_obj<object_h> l_Object("object", "Object handle");
ade_obj<physics_info_h> l_Physics("physics", "Physics handle");
ade_obj<int> l_Player("player", "Player handle");
ade_obj<object_h> l_Shields("shields", "Shields handle");
ade_obj<object_h> l_Ship("ship", "Ship handle", &l_Object);
ade_obj<int> l_Shipclass("shipclass", "Ship class handle");
ade_obj<object_h> l_ShipTextures("shiptextures", "Ship textures handle");
ade_obj<int> l_Shiptype("shiptype", "Ship type handle");
ade_obj<int> l_Species("species", "Species handle");
ade_obj<ship_subsys_h> l_Subsystem("subsystem", "Ship subsystem handle");
ade_obj<int> l_Team("team", "Team handle");
ade_obj<int> l_Texture("texture", "Texture handle");
ade_obj<int> l_Wing("wing", "Wing handle");
ade_obj<vec3d> l_Vector("vector", "Vector object");
ade_obj<object_h> l_Weapon("weapon", "Weapon handle", &l_Object);
ade_obj<ship_bank_h> l_WeaponBank("weaponbank", "Ship/subystem weapons bank handle");
ade_obj<ship_banktype_h> l_WeaponBankType("weaponbanktype", "Ship/subsystem weapons bank type handle");
ade_obj<int> l_Weaponclass("weaponclass", "Weapon class handle");
*/
//###########################################################
//########################<IMPORTANT>########################
//###########################################################
//If you are a coder who wants to add libraries, functions,
//or objects to Lua, then you want to be below this point.
//###########################################################
//########################</IMPORTANT>#######################
//###########################################################

flag_def_list plr_commands[] = {
	{	"TARGET_NEXT",							TARGET_NEXT,							0	},
	{	"TARGET_PREV",							TARGET_PREV,							0	},
	{	"TARGET_NEXT_CLOSEST_HOSTILE",			TARGET_NEXT_CLOSEST_HOSTILE,			0	},
	{	"TARGET_PREV_CLOSEST_HOSTILE",			TARGET_PREV_CLOSEST_HOSTILE,			0	},
	{	"TOGGLE_AUTO_TARGETING",				TOGGLE_AUTO_TARGETING,					0	},
	{	"TARGET_NEXT_CLOSEST_FRIENDLY",			TARGET_NEXT_CLOSEST_FRIENDLY,			0	},
	{	"TARGET_PREV_CLOSEST_FRIENDLY",			TARGET_PREV_CLOSEST_FRIENDLY,			0	},
	{	"TARGET_SHIP_IN_RETICLE",				TARGET_SHIP_IN_RETICLE,					0	},
	{	"TARGET_CLOSEST_SHIP_ATTACKING_TARGET",	TARGET_CLOSEST_SHIP_ATTACKING_TARGET,	0	},
	{	"TARGET_LAST_TRANMISSION_SENDER",		TARGET_LAST_TRANMISSION_SENDER,			0	},
	{	"STOP_TARGETING_SHIP",					STOP_TARGETING_SHIP,					0	},
	{	"TARGET_SUBOBJECT_IN_RETICLE",			TARGET_SUBOBJECT_IN_RETICLE,			0	},
	{	"TARGET_NEXT_SUBOBJECT",				TARGET_NEXT_SUBOBJECT,					0	},
	{	"TARGET_PREV_SUBOBJECT",				TARGET_PREV_SUBOBJECT,					0	},
	{	"STOP_TARGETING_SUBSYSTEM",				STOP_TARGETING_SUBSYSTEM,				0	},
	{	"MATCH_TARGET_SPEED",					MATCH_TARGET_SPEED,						0	},
	{	"TOGGLE_AUTO_MATCH_TARGET_SPEED",		TOGGLE_AUTO_MATCH_TARGET_SPEED,			0	},
	{	"FIRE_PRIMARY",							FIRE_PRIMARY,							0	},
	{	"FIRE_SECONDARY",						FIRE_SECONDARY,							0	},
	{	"CYCLE_NEXT_PRIMARY",					CYCLE_NEXT_PRIMARY,						0	},
	{	"CYCLE_PREV_PRIMARY",					CYCLE_PREV_PRIMARY,						0	},
	{	"CYCLE_SECONDARY",						CYCLE_SECONDARY,						0	},
	{	"CYCLE_NUM_MISSLES",					CYCLE_NUM_MISSLES,						0	},
	{	"LAUNCH_COUNTERMEASURE",				LAUNCH_COUNTERMEASURE,					0	},
	{	"FORWARD_THRUST",						FORWARD_THRUST,							0	},
	{	"REVERSE_THRUST",						REVERSE_THRUST,							0	},
	{	"BANK_LEFT",							BANK_LEFT,								0	},
	{	"BANK_RIGHT",							BANK_RIGHT,								0	},
	{	"PITCH_FORWARD",						PITCH_FORWARD,							0	},
	{	"PITCH_BACK",							PITCH_BACK,								0	},
	{	"YAW_LEFT",								YAW_LEFT,								0	},
	{	"YAW_RIGHT",							YAW_RIGHT,								0	},
	{	"ZERO_THROTTLE",						ZERO_THROTTLE,							1	},
	{	"MAX_THROTTLE",							MAX_THROTTLE,							1	},
	{	"ONE_THIRD_THROTTLE",					ONE_THIRD_THROTTLE,						1	},
	{	"TWO_THIRDS_THROTTLE",					TWO_THIRDS_THROTTLE,					1	},
	{	"PLUS_5_PERCENT_THROTTLE",				PLUS_5_PERCENT_THROTTLE,				1	},
	{	"MINUS_5_PERCENT_THROTTLE",				MINUS_5_PERCENT_THROTTLE,				1	},
	{	"ATTACK_MESSAGE",						ATTACK_MESSAGE,							1	},
	{	"DISARM_MESSAGE",						DISARM_MESSAGE,							1	},
	{	"DISABLE_MESSAGE",						DISABLE_MESSAGE,						1	},
	{	"ATTACK_SUBSYSTEM_MESSAGE",				ATTACK_SUBSYSTEM_MESSAGE,				1	},
	{	"CAPTURE_MESSAGE",						CAPTURE_MESSAGE,						1	},
	{	"ENGAGE_MESSAGE",						ENGAGE_MESSAGE,							1	},
	{	"FORM_MESSAGE",							FORM_MESSAGE,							1	},
	{	"IGNORE_MESSAGE",						IGNORE_MESSAGE,							1	},
	{	"PROTECT_MESSAGE",						PROTECT_MESSAGE,						1	},
	{	"COVER_MESSAGE",						COVER_MESSAGE,							1	},
	{	"WARP_MESSAGE",							WARP_MESSAGE,							1	},
	{	"REARM_MESSAGE",						REARM_MESSAGE,							1	},
	{	"TARGET_CLOSEST_SHIP_ATTACKING_SELF",	TARGET_CLOSEST_SHIP_ATTACKING_SELF,		1	},
	{	"VIEW_CHASE",							VIEW_CHASE,								1	},
	{	"VIEW_EXTERNAL",						VIEW_EXTERNAL,							1	},
	{	"VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK",		VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK,		1	},
	{	"VIEW_SLEW",							VIEW_SLEW,								1	},
	{	"VIEW_OTHER_SHIP",						VIEW_OTHER_SHIP,						1	},
	{	"VIEW_DIST_INCREASE",					VIEW_DIST_INCREASE,						1	},
	{	"VIEW_DIST_DECREASE",					VIEW_DIST_DECREASE,						1	},
	{	"VIEW_CENTER",							VIEW_CENTER,							1	},
	{	"PADLOCK_UP",							PADLOCK_UP,								1	},
	{	"PADLOCK_DOWN",							PADLOCK_DOWN,							1	},
	{	"PADLOCK_LEFT",							PADLOCK_LEFT,							1	},
	{	"PADLOCK_RIGHT",						PADLOCK_RIGHT,							1	},
	{	"RADAR_RANGE_CYCLE",					RADAR_RANGE_CYCLE,						1	},
	{	"SQUADMSG_MENU",						SQUADMSG_MENU,							2	},
	{	"SHOW_GOALS",							SHOW_GOALS,								2	},
	{	"END_MISSION",							END_MISSION,							2	},
	{	"TARGET_TARGETS_TARGET",				TARGET_TARGETS_TARGET,					2	},
	{	"AFTERBURNER",							AFTERBURNER,							2	},
	{	"INCREASE_WEAPON",						INCREASE_WEAPON,						2	},
	{	"DECREASE_WEAPON",						DECREASE_WEAPON,						2	},
	{	"INCREASE_SHIELD",						INCREASE_SHIELD,						2	},
	{	"DECREASE_SHIELD",						DECREASE_SHIELD,						2	},
	{	"INCREASE_ENGINE",						INCREASE_ENGINE,						2	},
	{	"DECREASE_ENGINE",						DECREASE_ENGINE,						2	},
	{	"ETS_EQUALIZE",							ETS_EQUALIZE,							2	},
	{	"SHIELD_EQUALIZE",						SHIELD_EQUALIZE,						2	},
	{	"SHIELD_XFER_TOP",						SHIELD_XFER_TOP,						2	},
	{	"SHIELD_XFER_BOTTOM",					SHIELD_XFER_BOTTOM,						2	},
	{	"SHIELD_XFER_LEFT",						SHIELD_XFER_LEFT,						2	},
	{	"SHIELD_XFER_RIGHT",					SHIELD_XFER_RIGHT,						2	},
	{	"XFER_SHIELD",							XFER_SHIELD,							2	},
	{	"XFER_LASER",							XFER_LASER,								2	},
	{	"GLIDE_WHEN_PRESSED",					GLIDE_WHEN_PRESSED,						2	},
	{	"BANK_WHEN_PRESSED",					BANK_WHEN_PRESSED,						2	},
	{	"SHOW_NAVMAP",							SHOW_NAVMAP,							2	},
	{	"ADD_REMOVE_ESCORT",					ADD_REMOVE_ESCORT,						2	},
	{	"ESCORT_CLEAR",							ESCORT_CLEAR,							2	},
	{	"TARGET_NEXT_ESCORT_SHIP",				TARGET_NEXT_ESCORT_SHIP,				2	},
	{	"TARGET_CLOSEST_REPAIR_SHIP",			TARGET_CLOSEST_REPAIR_SHIP,				2	},
	{	"TARGET_NEXT_UNINSPECTED_CARGO",		TARGET_NEXT_UNINSPECTED_CARGO,			2	},
	{	"TARGET_PREV_UNINSPECTED_CARGO",		TARGET_PREV_UNINSPECTED_CARGO,			2	},
	{	"TARGET_NEWEST_SHIP",					TARGET_NEWEST_SHIP,						2	},
	{	"TARGET_NEXT_LIVE_TURRET",				TARGET_NEXT_LIVE_TURRET,				2	},
	{	"TARGET_PREV_LIVE_TURRET",				TARGET_PREV_LIVE_TURRET,				2	},
	{	"TARGET_NEXT_BOMB",						TARGET_NEXT_BOMB,						2	},
	{	"TARGET_PREV_BOMB",						TARGET_PREV_BOMB,						3	},
	{	"MULTI_MESSAGE_ALL",					MULTI_MESSAGE_ALL,						3	},
	{	"MULTI_MESSAGE_FRIENDLY",				MULTI_MESSAGE_FRIENDLY,					3	},
	{	"MULTI_MESSAGE_HOSTILE",				MULTI_MESSAGE_HOSTILE,					3	},
	{	"MULTI_MESSAGE_TARGET",					MULTI_MESSAGE_TARGET,					3	},
	{	"MULTI_OBSERVER_ZOOM_TO",				MULTI_OBSERVER_ZOOM_TO,					3	},
	{	"TIME_SPEED_UP",						TIME_SPEED_UP,							3	},
	{	"TIME_SLOW_DOWN",						TIME_SLOW_DOWN,							3	},
	{	"TOGGLE_HUD_CONTRAST",					TOGGLE_HUD_CONTRAST,					3	},
	{	"MULTI_TOGGLE_NETINFO",					MULTI_TOGGLE_NETINFO,					3	},
	{	"MULTI_SELF_DESTRUCT",					MULTI_SELF_DESTRUCT,					3	},
	{	"TOGGLE_HUD",							TOGGLE_HUD,								3	},
	{	"RIGHT_SLIDE_THRUST",					RIGHT_SLIDE_THRUST,						3	},
	{	"LEFT_SLIDE_THRUST",					LEFT_SLIDE_THRUST,						3	},
	{	"UP_SLIDE_THRUST",						UP_SLIDE_THRUST,						3	},
	{	"DOWN_SLIDE_THRUST",					DOWN_SLIDE_THRUST,						3	},
	{	"HUD_TARGETBOX_TOGGLE_WIREFRAME",		HUD_TARGETBOX_TOGGLE_WIREFRAME,			3	},
	{	"VIEW_TOPDOWN",							VIEW_TOPDOWN,							3	},
	{	"VIEW_TRACK_TARGET",					VIEW_TRACK_TARGET,						3	},
	{	"AUTO_PILOT_TOGGLE",					AUTO_PILOT_TOGGLE,						3	},
	{	"NAV_CYCLE",							NAV_CYCLE,								3	},
	{	"TOGGLE_GLIDING",						TOGGLE_GLIDING,							3	},
};

int num_plr_commands = sizeof(plr_commands)/sizeof(flag_def_list);


//**********HANDLE: SoundEntry
struct sound_entry_h
{
	int idx;

	sound_entry_h()
	{
		idx = -1;
	}

	sound_entry_h(int n_idx)
	{
		idx = n_idx;
	}

	game_snd *Get()
	{
		if (!this->IsValid())
			return NULL;

		return &Snds[idx];
	}

	bool IsValid()
	{
		if (idx < 0 || idx >= (int) Snds.size())
			return false;

		return true;
	}

	int getId() 
	{
		if (!IsValid())
			return -1;

		game_snd *snd = Get();

		if (snd == NULL)
			return -1;

		return snd->id;
	}

};

//**********HANDLE: SoundEntry
ade_obj<sound_entry_h> l_SoundEntry("soundentry", "sounds.tbl table entry handle");

ADE_VIRTVAR(DefaultVolume, l_SoundEntry, "number", "The default volume of this game sound", "number", "Volume in the range from 1 to 0 or -1 on error")
{
	sound_entry_h *seh = NULL;
	float newVal = -1.0f;

	if (!ade_get_args(L, "o|f", l_SoundEntry.GetPtr(&seh), &newVal))
		return ade_set_error(L, "f", -1.0f);

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "f", -1.0f);

	if (ADE_SETTING_VAR)
	{
		if (seh->Get() != NULL)
		{
			CAP(newVal, 0.0f, 1.0f);

			seh->Get()->default_volume = newVal;
		}
	}

	return ade_set_args(L, "f", seh->Get()->default_volume);
}

ADE_FUNC(getFilename, l_SoundEntry, NULL, "Returns the filename of this sound", "string", "filename or empty string on error")
{
	sound_entry_h *seh = NULL;

	if (!ade_get_args(L, "o", l_SoundEntry.GetPtr(&seh)))
		return ade_set_error(L, "s", "");

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", seh->Get()->filename);
}

ADE_FUNC(getDuration, l_SoundEntry, NULL, "Gives the length of the sound in seconds.", "number", "the length, or -1 on error")
{
	sound_entry_h *seh = NULL;

	if (!ade_get_args(L, "o", l_SoundEntry.GetPtr(&seh)))
		return ade_set_error(L, "f", -1.0f);

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "f", -1.0f);

	return ade_set_args(L, "f", (i2fl(snd_get_duration(seh->getId())) / 1000.0f));
}

ADE_FUNC(get3DValues, l_SoundEntry, "vector Postion[, number radius=0.0]", "Computes the volume and the panning of the sound when it would be played from the specified position.<br>"
	"If range is given then the volume will diminish when the listener is withing that distance to the source.<br>"
	"The position of the listener is always the the current viewing position.", "number, number", "The volume and the panning, in that sequence, or both -1 on error")
{
	sound_entry_h *seh = NULL;
	vec3d *sourcePos = NULL;
	float radius = 0.0f;

	float vol = 0.0f;
	float pan = 0.0f;

	if (!ade_get_args(L, "oo|f", l_SoundEntry.GetPtr(&seh), l_Vector.GetPtr(&sourcePos), &radius))
		return ade_set_error(L, "ff", -1.0f, -1.0f);

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "ff", -1.0f, -1.0f);

	int result = snd_get_3d_vol_and_pan(seh->Get(), sourcePos, &vol, &pan, radius);

	if (result < 0)
	{
		return ade_set_args(L, "ff", -1.0f, -1.0f);
	}
	else
	{
		return ade_set_args(L, "ff", vol, pan);
	}
}

ADE_FUNC(isValid, l_SoundEntry, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	sound_entry_h *seh;
	if(!ade_get_args(L, "o", l_SoundEntry.GetPtr(&seh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", seh->IsValid());
}

//**********HANDLE: Sound
struct sound_h : public sound_entry_h
{
	int sig;

	sound_h():sound_entry_h()
	{
		sig=-1;
	}

	sound_h(int n_gs_idx, int n_sig) : sound_entry_h(n_gs_idx)
	{
		sig=n_sig;
	}

	int getSignature()
	{
		if (!IsValid())
			return -1;

		return sig;
	}

	bool IsSoundValid()
	{
		if(sig < 0 || ds_get_channel(sig) < 0)
			return false;

		return true;
	}

	bool IsValid()
	{
		if(!sound_entry_h::IsValid())
			return false;

		if(sig < 0 || ds_get_channel(sig) < 0)
			return false;

		return true;
	}
};

ade_obj<sound_h> l_Sound("sound", "sound instance handle");

ADE_VIRTVAR(Pitch, l_Sound, "number", "Pitch of sound, from 100 to 100000", "number", "Pitch, or 0 if handle is invalid")
{
	sound_h *sh;
	int newpitch = 100;
	if(!ade_get_args(L, "o|i", l_Sound.GetPtr(&sh), &newpitch))
		return ade_set_error(L, "f", 0.0f);

	if (!sh->IsSoundValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR)
	{
		if(newpitch < 100)
			newpitch = 100;
		if(newpitch > 100000)
			newpitch = 100000;

		snd_set_pitch(sh->sig, newpitch);
	}

	return ade_set_args(L, "f", snd_get_pitch(sh->sig));
}

ADE_FUNC(getRemainingTime, l_Sound, NULL, "The remaining time of this sound handle", "number", "Remaining time, or -1 on error")
{
	sound_h *sh;
	if(!ade_get_args(L, "o", l_Sound.GetPtr(&sh)))
		return ade_set_error(L, "f", -1.0f);

	if (!sh->IsSoundValid())
		return ade_set_error(L, "f", -1.0f);

	int remaining = snd_time_remaining(sh->getSignature());

	return ade_set_args(L, "f", i2fl(remaining) / 1000.0f);
}

ADE_FUNC(setVolume, l_Sound, "number", "Sets the volume of this sound instance", "boolean", "true if succeeded, false otherwise")
{
	sound_h *sh;
	float newVol = -1.0f;
	if(!ade_get_args(L, "of", l_Sound.GetPtr(&sh), &newVol))
		return ADE_RETURN_FALSE;

	if (!sh->IsSoundValid())
		return ADE_RETURN_FALSE;

	CAP(newVol, 0.0f, 1.0f);

	snd_set_volume(sh->getSignature(), newVol);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(setPanning, l_Sound, "number", "Sets the panning of this sound. Argument ranges from -1 for left to 1 for right", "boolean", "true if succeeded, false otherwise")
{
	sound_h *sh;
	float newPan = -1.0f;
	if(!ade_get_args(L, "of", l_Sound.GetPtr(&sh), &newPan))
		return ADE_RETURN_FALSE;

	if (!sh->IsSoundValid())
		return ADE_RETURN_FALSE;

	CAP(newPan, -1.0f, 1.0f);

	snd_set_pan(sh->getSignature(), newPan);

	return ADE_RETURN_TRUE;
}


ADE_FUNC(setPosition, l_Sound, "number[,boolean = true]", 
		 "Sets the absolute position of the sound. If boolean argument is true then the value is given as a percentage<br>"
		 "This operation fails if there is no backing soundentry!", 
		 "boolean", "true if successfull, false otherwise")
{
	sound_h *sh;
	float val = -1.0f;
	bool percent = true;
	if(!ade_get_args(L, "of|b", l_Sound.GetPtr(&sh), &val, &percent))
		return ADE_RETURN_FALSE;

	if (!sh->IsValid())
		return ADE_RETURN_FALSE;

	if (val <= 0.0f)
		return ADE_RETURN_FALSE;

	snd_set_pos(sh->getSignature(), sh->Get(), val, percent);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(rewind, l_Sound, "number", "Rewinds the sound by the given number of seconds<br>"
		 "This operation fails if there is no backing soundentry!", "boolean", "true if succeeded, false otherwise")
{
	sound_h *sh;
	float val = -1.0f;
	if(!ade_get_args(L, "of", l_Sound.GetPtr(&sh), &val))
		return ADE_RETURN_FALSE;

	if (!sh->IsValid())
		return ADE_RETURN_FALSE;

	if (val <= 0.0f)
		return ADE_RETURN_FALSE;

	snd_rewind(sh->getSignature(), sh->Get(), val);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(skip, l_Sound, "number", "Skips the given number of seconds of the sound<br>"
		 "This operation fails if there is no backing soundentry!", "boolean", "true if succeeded, false otherwise")
{
	sound_h *sh;
	float val = -1.0f;
	if(!ade_get_args(L, "of", l_Sound.GetPtr(&sh), &val))
		return ADE_RETURN_FALSE;

	if (!sh->IsValid())
		return ADE_RETURN_FALSE;

	if (val <= 0.0f)
		return ADE_RETURN_FALSE;

	snd_ffwd(sh->getSignature(), sh->Get(), val);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(isPlaying, l_Sound, NULL, "Specifies if this handle is currently playing", "boolean", "true if playing, false if otherwise")
{
	sound_h *sh;
	if(!ade_get_args(L, "o", l_Sound.GetPtr(&sh)))
		return ade_set_error(L, "b", false);

	if (!sh->IsSoundValid())
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", snd_is_playing(sh->getSignature()) == 1);
}

ADE_FUNC(stop, l_Sound, NULL, "Stops the sound of this handle", "boolean", "true if succeeded, false otherwise")
{
	sound_h *sh;
	if(!ade_get_args(L, "o", l_Sound.GetPtr(&sh)))
		return ade_set_error(L, "b", false);

	if (!sh->IsSoundValid())
		return ade_set_error(L, "b", false);

	snd_stop(sh->getSignature());

	return ADE_RETURN_TRUE;
}

ADE_FUNC(isValid, l_Sound, NULL, "Detects whether the whole handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	sound_h *sh;
	if(!ade_get_args(L, "o", l_Sound.GetPtr(&sh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", sh->IsValid());
}

ADE_FUNC(isSoundValid, l_Sound, NULL, "Checks if only the sound is valid, should be used for non soundentry sounds",
		 "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	sound_h *sh;
	if(!ade_get_args(L, "o", l_Sound.GetPtr(&sh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", sh->IsSoundValid());
}

ade_obj<sound_h> l_Sound3D("3Dsound", "3D sound instance handle", &l_Sound);

ADE_FUNC(updatePosition, l_Sound3D, "vector Position[, number radius = 0.0]", "Updates the given 3D sound with the specified position and an optional range value", "boolean", "true if succeesed, false otherwise")
{
	sound_h *sh;
	vec3d *newPos = NULL;
	float radius = 0.0f;

	if(!ade_get_args(L, "oo|f", l_Sound.GetPtr(&sh), l_Vector.GetPtr(&newPos), &radius))
		return ade_set_error(L, "b", false);

	if (!sh->IsValid() || newPos == NULL)
		return ade_set_error(L, "b", false);

	snd_update_3d_pos(sh->getSignature(), sh->Get(), newPos, radius);

	return ADE_RETURN_TRUE;
}

//**********HANDLE: Soundfile
ade_obj<int> l_Soundfile("soundfile", "Handle to a sound file");

ADE_VIRTVAR(Duration, l_Soundfile, "number", "The duration of the sound file, in seconds", "number", "The duration or -1 on error")
{
	int snd_idx = -1;

	if (!ade_get_args(L, "o", l_Soundfile.Get(&snd_idx)))
		return ade_set_error(L, "f", -1.0f);

	if (snd_idx < 0)
		return ade_set_error(L, "f", -1.0f);

	int duration = snd_get_duration(snd_idx);

	return ade_set_args(L, "f", i2fl(duration) / 1000.0f);
}

ADE_VIRTVAR(Filename, l_Soundfile, "string", "The filename of the file", "string", "The file name or empty string on error")
{
	int snd_idx = -1;

	if (!ade_get_args(L, "o", l_Soundfile.Get(&snd_idx)))
		return ade_set_error(L, "s", "");

	if (snd_idx < 0)
		return ade_set_error(L, "s", "");

	const char* filename = snd_get_filename(snd_idx);

	return ade_set_args(L, "s", filename);
}


ADE_FUNC(play, l_Soundfile, "[number volume = 1.0[, number panning = 0.0]]", "Plays the sound", "sound", "A sound handle or invalid handle on error")
{
	int snd_idx = -1;
	float volume = 1.0f;
	float panning = 0.0f;

	if (!ade_get_args(L, "o|ff", l_Soundfile.Get(&snd_idx)))
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	if (snd_idx < 0)
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	if (volume < 0.0f)
	{
		LuaError(L, "Invalid volume value of %f specified!", volume);
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));
	}

	int handle = snd_play_raw(snd_idx, panning, volume);
	
	return ade_set_args(L, "o", l_Sound.Set(sound_h(-1, handle)));
}

ADE_FUNC(isValid, l_Soundfile, NULL, "Checks if the soundfile handle is valid", "boolean", "true if valid, false otherwise")
{	
	int idx = -1;
	if (!ade_get_args(L, "o", l_Soundfile.Get(&idx)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", idx >= 0);
}

//**********HANDLE: Persona
ade_obj<int> l_Persona("persona", "Persona handle");

ADE_VIRTVAR(Name, l_Persona, "string", "The name of the persona", "string", "The name or empty string on error")
{
	int idx = -1;

	if (!ade_get_args(L, "o", l_Persona.Get(&idx)))
		return ade_set_error(L, "s", "");

	if (Personas == NULL)
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= Num_personas)
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Personas[idx].name);
}

ADE_FUNC(isValid, l_Persona, NULL, "Detect if the handle is valid", "boolean", "true if valid, false otherwise")
{
	int idx = -1;

	if (!ade_get_args(L, "o", l_Persona.Get(&idx)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", idx >= 0 && idx < Num_personas);
}

//**********HANDLE: Message
ade_obj<int> l_Message("message", "Handle to a mission message");

ADE_VIRTVAR(Name, l_Message, "string", "The name of the message as specified in the mission file", "string", "The name or an empty string if handle is invalid")
{
	int idx = -1;
	if (!ade_get_args(L, "o", l_Message.Get(&idx)))
		return ade_set_error(L, "s", "");
	
	if (idx < 0 && idx >= (int) Messages.size())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Messages[idx].name);
}

ADE_VIRTVAR(Message, l_Message, "string", "The unaltered text of the message, see getMessage() for options to replace variables<br>"
			"<b>NOTE:</b> Changing the text will also change the text for messages not yet played but already in the message queue!",
			"string", "The message or an empty string if handle is invalid")
{
	int idx = -1;
	char* newText = NULL;
	if (!ade_get_args(L, "o|s", l_Message.Get(&idx), &newText))
		return ade_set_error(L, "s", "");
	
	if (idx < 0 && idx >= (int) Messages.size())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR && newText != NULL)
	{
		if (strlen(newText) > MESSAGE_LENGTH)
			LuaError(L, "New message text is too long, maximum is %d!", MESSAGE_LENGTH);
		else
			strcpy_s(Messages[idx].message, newText);
	}

	return ade_set_args(L, "s", Messages[idx].message);
}

// from mission/missionmessage.cpp
extern int add_wave( const char *wave_name );
ADE_VIRTVAR(VoiceFile, l_Message, "soundfile", "The voice file of the message", "soundfile", "The voice file handle or invalid handle when not present")
{
	int idx = -1;
	int sndIdx = -1;

	if (!ade_get_args(L, "o|o", l_Message.Get(&idx), l_Soundfile.Get(&sndIdx)))
		return ade_set_error(L, "o", l_Soundfile.Set(-1));
	
	if (idx < 0 && idx >= (int) Messages.size())
		return ade_set_error(L, "o", l_Soundfile.Set(-1));

	MissionMessage* msg = &Messages[idx];

	if (ADE_SETTING_VAR)
	{
		if (sndIdx >= 0)
		{
			const char* newFilename = snd_get_filename(sndIdx);

			msg->wave_info.index = add_wave(newFilename);
		}
		else
		{
			msg->wave_info.index = -1;
		}
	}

	if (msg->wave_info.index < 0)
	{
		return ade_set_args(L, "o", l_Soundfile.Set(-1));
	}
	else
	{
		int index = msg->wave_info.index;
		// Load the sound before using it
		message_load_wave(index, Message_waves[index].name);
		
		return ade_set_args(L, "o", l_Soundfile.Set(Message_waves[index].num));
	}
}

ADE_VIRTVAR(Persona, l_Message, "persona", "The persona of the message", "persona", "The persona handle or invalid handle if not present")
{
	int idx = -1;
	int newPersona = -1;

	if (!ade_get_args(L, "o|o", l_Message.Get(&idx), l_Persona.Get(&newPersona)))
		return ade_set_error(L, "o", l_Soundfile.Set(-1));
	
	if (idx < 0 && idx >= (int) Messages.size())
		return ade_set_error(L, "o", l_Soundfile.Set(-1));
	
	if (ADE_SETTING_VAR && newPersona >= 0 && newPersona < Num_personas)
	{
		Messages[idx].persona_index = newPersona;
	}

	return ade_set_args(L, "o", l_Persona.Set(Messages[idx].persona_index));
}

ADE_FUNC(getMessage, l_Message, "[boolean replaceVars = true]", "Gets the text of the message and optionally replaces SEXP variables with their respective values.", "string", "The message or an empty string if handle is invalid")
{
	int idx = -1;
	bool replace = true;
	if (!ade_get_args(L, "o|b", l_Message.Get(&idx), &replace))
		return ade_set_error(L, "s", "");
	
	if (idx < 0 && idx >= (int) Messages.size())
		return ade_set_error(L, "s", "");

	if (!replace)
		return ade_set_args(L, "s", Messages[idx].message);
	else
	{
		char temp_buf[MESSAGE_LENGTH];
		strcpy_s(temp_buf, Messages[idx].message);

		sexp_replace_variable_names_with_values(temp_buf, MESSAGE_LENGTH);

		return ade_set_args(L, "s", temp_buf);
	}
}

ADE_FUNC(isValid, l_Message, NULL, "Checks if the message handle is valid", "boolean", "true if valid, false otherwise")
{	
	int idx = -1;
	if (!ade_get_args(L, "o", l_Message.Get(&idx)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", idx >= 0 && idx < (int) Messages.size());
}

//**********HANDLE: Wing
ade_obj<int> l_Wing("wing", "Wing handle");

ADE_INDEXER(l_Wing, "number Index", "Array of ships in the wing", "ship", "Ship handle, or invalid ship handle if index is invalid or wing handle is invalid")
{
	int wdx;
	int sdx;
	object_h *ndx=NULL;
	if(!ade_get_args(L, "oi|o", l_Wing.Get(&wdx), &sdx, l_Ship.GetPtr(&ndx)))
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	if(wdx < 0 || wdx >= Num_wings || sdx < 1 || sdx > Wings[wdx].current_count) {
		return ade_set_error(L, "o", l_Ship.Set(object_h()));
	}

	//Lua-->FS2
	sdx--;

	if(ADE_SETTING_VAR && ndx != NULL && ndx->IsValid()) {
		Wings[wdx].ship_index[sdx] = ndx->objp->instance;
	}

	return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[Wings[wdx].ship_index[sdx]].objnum])));
}

ADE_FUNC(__len, l_Wing, NULL, "Gets the number of ships in the wing", "number", "Number of ships in wing, or 0 if invalid handle")
{
	int wdx;
	if(!ade_get_args(L, "o", l_Wing.Get(&wdx)) || wdx < 0 || wdx >= Num_wings)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", Wings[wdx].current_count);
}

ADE_VIRTVAR(Name, l_Wing, "string", "Name of Wing", "string", "Wing name, or empty string if handle is invalid")
{
	int wdx;
	char *s = NULL;
	if ( !ade_get_args(L, "o|s", l_Wing.Get(&wdx), &s) || wdx < 0 || wdx >= Num_wings )
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != NULL) {
		strncpy(Wings[wdx].name, s, sizeof(Wings[wdx].name)-1);
	}

	return ade_set_args(L, "s", Wings[wdx].name);
}

ADE_FUNC(isValid, l_Wing, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if(!ade_get_args(L, "o", l_Wing.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= Num_wings)
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

//**********HANDLE: Ship
ade_obj<object_h> l_Ship("ship", "Ship handle", &get_l_Object());

ADE_INDEXER(l_Ship, "string Name/number Index", "Array of ship subsystems", "subsystem", "Subsystem handle, or invalid subsystem handle if index or ship handle is invalid")
{
	object_h *objh;
	char *s = NULL;
	ship_subsys_h *sub = nullptr;
	if(!ade_get_args(L, "o|so", l_Ship.GetPtr(&objh), &s, l_Subsystem.GetPtr(&sub)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	ship *shipp = &Ships[objh->objp->instance];
	ship_subsys *ss = ship_get_subsys(shipp, s);

	if(ss == NULL)
	{
		int idx = atoi(s);
		if(idx > 0 && idx <= ship_get_num_subsys(shipp))
		{
			idx--; //Lua->FS2
			ss = ship_get_indexed_subsys(shipp, idx);
		}
	}

	if(ss == NULL)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(objh->objp, ss)));
}

ADE_FUNC(__len, l_Ship, NULL, "Number of subsystems on ship", "number", "Subsystem number, or 0 if handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "i", 0);

	if(!objh->IsValid())
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", ship_get_num_subsys(&Ships[objh->objp->instance]));
}

ADE_VIRTVAR(ShieldArmorClass, l_Ship, "string", "Current Armor class of the ships' shield", "string", "Armor class name, or empty string if none is set")
{
	object_h *objh;
	char *s = NULL;
	char *name = NULL;
	
	if(!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp->instance];
	int atindex = -1;
	if (ADE_SETTING_VAR && s != NULL) {
		atindex = armor_type_get_idx(s);
		shipp->shield_armor_type_idx = atindex;
	}

	if (atindex != -1)
		name = Armor_types[atindex].GetNamePtr();
	else
		name = "";

	return ade_set_args(L, "s", name);
}

ADE_VIRTVAR(ArmorClass, l_Ship, "string", "Current Armor class", "string", "Armor class name, or empty string if none is set")
{
	object_h *objh;
	char *s = NULL;
	char *name = NULL;
	
	if(!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp->instance];
	int atindex = -1;
	if (ADE_SETTING_VAR && s != NULL) {
		atindex = armor_type_get_idx(s);
		shipp->armor_type_idx = atindex;
	}

	if (atindex != -1)
		name = Armor_types[atindex].GetNamePtr();
	else
		name = "";

	return ade_set_args(L, "s", name);
}

ADE_VIRTVAR(Name, l_Ship, "string", "Ship name", "string", "Ship name, or empty string if handle is invalid")
{
	object_h *objh;
	char *s = NULL;
	if(!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && s != NULL) {
		strncpy(shipp->ship_name, s, sizeof(shipp->ship_name)-1);
	}

	return ade_set_args(L, "s", shipp->ship_name);
}

ADE_VIRTVAR(AfterburnerFuelLeft, l_Ship, "number", "Afterburner fuel left", "number", "Afterburner fuel left, or 0 if handle is invalid")
{
	object_h *objh;
	float fuel = -1.0f;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fuel))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && fuel >= 0.0f)
		shipp->afterburner_fuel = fuel;

	return ade_set_args(L, "f", shipp->afterburner_fuel);
}

ADE_VIRTVAR(AfterburnerFuelMax, l_Ship, "number", "Afterburner fuel capacity", "number", "Afterburner fuel capacity, or 0 if handle is invalid")
{
	object_h *objh;
	float fuel = -1.0f;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fuel))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship_info *sip = &Ship_info[Ships[objh->objp->instance].ship_info_index];

	if(ADE_SETTING_VAR && fuel >= 0.0f)
		sip->afterburner_fuel_capacity = fuel;

	return ade_set_args(L, "f", sip->afterburner_fuel_capacity);
}

ADE_VIRTVAR(Class, l_Ship, "shipclass", "Ship class", "shipclass", "Ship class, or invalid shipclass handle if ship handle is invalid")
{
	object_h *objh;
	int idx=-1;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Shipclass.Get(&idx)))
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && idx > -1) {
		change_ship_type(objh->objp->instance, idx, 1);
		if (shipp == Player_ship) {
			set_current_hud();
		}
	}

	if(shipp->ship_info_index < 0)
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	return ade_set_args(L, "o", l_Shipclass.Set(shipp->ship_info_index));
}

ADE_VIRTVAR(CountermeasuresLeft, l_Ship, "number", "Number of countermeasures left", "number", "Countermeasures left, or 0 if ship handle is invalid")
{
	object_h *objh;
	int newcm = -1;
	if(!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &newcm))
		return ade_set_error(L, "i", 0);

	if(!objh->IsValid())
		return ade_set_error(L, "i", 0);

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && newcm > -1)
		shipp->cmeasure_count = newcm;

	return ade_set_args(L, "i", shipp->cmeasure_count);
}

ADE_VIRTVAR(CockpitDisplays, l_Ship, "displays", "An array of the cockpit displays on this ship.<br>NOTE: Only the ship of the player has these", "displays", "displays handle or invalid handle on error")
{
	object_h *objh;
	cockpit_displays_h *cdh = NULL;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_CockpitDisplays.GetPtr(&cdh)))
		return ade_set_error(L, "o", l_CockpitDisplays.Set(cockpit_displays_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_CockpitDisplays.Set(cockpit_displays_h()));

	if(ADE_SETTING_VAR)
	{
		LuaError(L, "Attempted to use incomplete feature: Cockpit displays copy");
	}

	return ade_set_args(L, "o", l_CockpitDisplays.Set(cockpit_displays_h(objh->objp)));
}

ADE_VIRTVAR(CountermeasureClass, l_Ship, "weaponclass", "Weapon class mounted on this ship's countermeasure point", "weaponclass", "Countermeasure hardpoint weapon class, or invalid weaponclass handle if no countermeasure class or ship handle is invalid")
{
	object_h *objh;
	int newcm = -1;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Weaponclass.Get(&newcm)))
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));;

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR) {
			shipp->current_cmeasure = newcm;
	}

	if(shipp->current_cmeasure > -1)
		return ade_set_args(L, "o", l_Weaponclass.Set(shipp->current_cmeasure));
	else
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));;
}

ADE_VIRTVAR(HitpointsMax, l_Ship, "number", "Total hitpoints", "number", "Ship maximum hitpoints, or 0 if handle is invalid")
{
	object_h *objh;
	float newhits = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &newhits))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && newhits > -1)
		shipp->ship_max_hull_strength = newhits;

	return ade_set_args(L, "f", shipp->ship_max_hull_strength);
}

ADE_VIRTVAR(WeaponEnergyLeft, l_Ship, "number", "Current weapon energy reserves", "number", "Ship current weapon energy reserve level, or 0 if invalid")
{
	object_h *objh;
	float neweng = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &neweng))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && neweng > -1)
		shipp->weapon_energy = neweng;

	return ade_set_args(L, "f", shipp->weapon_energy);
}

ADE_VIRTVAR(WeaponEnergyMax, l_Ship, "number", "Maximum weapon energy", "number", "Ship maximum weapon energy reserve level, or 0 if invalid")
{
	object_h *objh;
	float neweng = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &neweng))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship_info *sip = &Ship_info[Ships[objh->objp->instance].ship_info_index];

	if(ADE_SETTING_VAR && neweng > -1)
		sip->max_weapon_reserve = neweng;

	return ade_set_args(L, "f", sip->max_weapon_reserve);
}

ADE_VIRTVAR(AutoaimFOV, l_Ship, "number", "FOV of ship's autoaim, if any", "number", "FOV (in degrees), or 0 if ship uses no autoaim or if handle is invalid")
{
	object_h *objh;
	float fov = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fov))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR && fov >= 0.0f) {
		if (fov > 180.0)
			fov = 180.0;

		shipp->autoaim_fov = fov * PI / 180.0f;
	}

	return ade_set_args(L, "f", shipp->autoaim_fov * 180.0f / PI);
}

ADE_VIRTVAR(PrimaryTriggerDown, l_Ship, "boolean", "Determines if primary trigger is pressed or not", "boolean", "True if pressed, false if not, nil if ship handle is invalid")
{
	object_h *objh;
	bool trig = false;
	if(!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &trig))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
    {
		if(trig)
			shipp->flags.set(Ship::Ship_Flags::Trigger_down);
		else
			shipp->flags.remove(Ship::Ship_Flags::Trigger_down);
    }

	if (shipp->flags[Ship::Ship_Flags::Trigger_down])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}


ADE_VIRTVAR(PrimaryBanks, l_Ship, "weaponbanktype", "Array of primary weapon banks", "weaponbanktype", "Primary weapon banks, or invalid weaponbanktype handle if ship handle is invalid")
{
	object_h *objh;
	ship_banktype_h *swh = nullptr;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &Ships[objh->objp->instance].weapons;

	if(ADE_SETTING_VAR && swh && swh->IsValid()) {
		ship_weapon *src = &Ships[swh->objp->instance].weapons;

		dst->current_primary_bank = src->current_primary_bank;
		dst->num_primary_banks = src->num_primary_banks;

		memcpy(dst->next_primary_fire_stamp, src->next_primary_fire_stamp, sizeof(dst->next_primary_fire_stamp));
		memcpy(dst->primary_animation_done_time, src->primary_animation_done_time, sizeof(dst->primary_animation_done_time));
		memcpy(dst->primary_animation_position, src->primary_animation_position, sizeof(dst->primary_animation_position));
		memcpy(dst->primary_bank_ammo, src->primary_bank_ammo, sizeof(dst->primary_bank_ammo));
		memcpy(dst->primary_bank_capacity, src->primary_bank_capacity, sizeof(dst->primary_bank_capacity));
		memcpy(dst->primary_bank_rearm_time, src->primary_bank_rearm_time, sizeof(dst->primary_bank_rearm_time));
		memcpy(dst->primary_bank_start_ammo, src->primary_bank_start_ammo, sizeof(dst->primary_bank_start_ammo));
		memcpy(dst->primary_bank_weapons, src->primary_bank_weapons, sizeof(dst->primary_bank_weapons));
	}

	return ade_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp, dst, SWH_PRIMARY)));
}

ADE_VIRTVAR(SecondaryBanks, l_Ship, "weaponbanktype", "Array of secondary weapon banks", "weaponbanktype", "Secondary weapon banks, or invalid weaponbanktype handle if ship handle is invalid")
{
	object_h *objh;
	ship_banktype_h *swh = nullptr;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &Ships[objh->objp->instance].weapons;

	if(ADE_SETTING_VAR && swh && swh->IsValid()) {
		ship_weapon *src = &Ships[swh->objp->instance].weapons;

		dst->current_secondary_bank = src->current_secondary_bank;
		dst->num_secondary_banks = src->num_secondary_banks;

		memcpy(dst->next_secondary_fire_stamp, src->next_secondary_fire_stamp, sizeof(dst->next_secondary_fire_stamp));
		memcpy(dst->secondary_animation_done_time, src->secondary_animation_done_time, sizeof(dst->secondary_animation_done_time));
		memcpy(dst->secondary_animation_position, src->secondary_animation_position, sizeof(dst->secondary_animation_position));
		memcpy(dst->secondary_bank_ammo, src->secondary_bank_ammo, sizeof(dst->secondary_bank_ammo));
		memcpy(dst->secondary_bank_capacity, src->secondary_bank_capacity, sizeof(dst->secondary_bank_capacity));
		memcpy(dst->secondary_bank_rearm_time, src->secondary_bank_rearm_time, sizeof(dst->secondary_bank_rearm_time));
		memcpy(dst->secondary_bank_start_ammo, src->secondary_bank_start_ammo, sizeof(dst->secondary_bank_start_ammo));
		memcpy(dst->secondary_bank_weapons, src->secondary_bank_weapons, sizeof(dst->secondary_bank_weapons));
		memcpy(dst->secondary_next_slot, src->secondary_next_slot, sizeof(dst->secondary_next_slot));
	}

	return ade_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp, dst, SWH_SECONDARY)));
}

ADE_VIRTVAR(TertiaryBanks, l_Ship, "weaponbanktype", "Array of tertiary weapon banks", "weaponbanktype", "Tertiary weapon banks, or invalid weaponbanktype handle if ship handle is invalid")
{
	object_h *objh;
	ship_banktype_h *swh = nullptr;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &Ships[objh->objp->instance].weapons;

	if(ADE_SETTING_VAR && swh && swh->IsValid()) {
		ship_weapon *src = &Ships[swh->objp->instance].weapons;

		dst->current_tertiary_bank = src->current_tertiary_bank;
		dst->num_tertiary_banks = src->num_tertiary_banks;

		dst->next_tertiary_fire_stamp = src->next_tertiary_fire_stamp;
		dst->tertiary_bank_ammo = src->tertiary_bank_ammo;
		dst->tertiary_bank_capacity = src->tertiary_bank_capacity;
		dst->tertiary_bank_rearm_time = src->tertiary_bank_rearm_time;
		dst->tertiary_bank_start_ammo = src->tertiary_bank_start_ammo;
	}

	return ade_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp, dst, SWH_TERTIARY)));
}

ADE_VIRTVAR(Target, l_Ship, "object", "Target of ship. Value may also be a deriviative of the 'object' class, such as 'ship'.", "object", "Target object, or invalid object handle if no target or ship handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	//WMC - Maybe use two argument return capabilities of Lua to set/return subsystem?
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	ai_info *aip = NULL;
	if(Ships[objh->objp->instance].ai_index > -1)
		aip = &Ai_info[Ships[objh->objp->instance].ai_index];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR && newh)
	{
		if(aip->target_signature != newh->sig)
		{
			if(newh->IsValid())
			{
				aip->target_objnum = OBJ_INDEX(newh->objp);
				aip->target_signature = newh->sig;
				aip->target_time = 0.0f;

				if (aip == Player_ai)
					hud_shield_hit_reset(newh->objp);
			}
			else
			{
				aip->target_objnum = -1;
				aip->target_signature = 0;
				aip->target_time = 0.0f;
			}

			set_targeted_subsys(aip, NULL, -1);
		}
	}

	return ade_set_object_with_breed(L, aip->target_objnum);
}

ADE_VIRTVAR(TargetSubsystem, l_Ship, "subsystem", "Target subsystem of ship.", "subsystem", "Target subsystem, or invalid subsystem handle if no target or ship handle is invalid")
{
	object_h *oh;
	ship_subsys_h *newh = nullptr;
	//WMC - Maybe use two argument return capabilities of Lua to set/return subsystem?
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&oh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	ai_info *aip = NULL;
	if(Ships[oh->objp->instance].ai_index > -1)
		aip = &Ai_info[Ships[oh->objp->instance].ai_index];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if (aip == Player_ai) {
				if (aip->target_signature != newh->sig)
					hud_shield_hit_reset(newh->objp);

				Ships[newh->ss->parent_objnum].last_targeted_subobject[Player_num] = newh->ss;
			}

			aip->target_objnum = OBJ_INDEX(newh->objp);
			aip->target_signature = newh->sig;
			aip->target_time = 0.0f;
			set_targeted_subsys(aip, newh->ss, aip->target_objnum);
		}
		else
		{
			aip->target_objnum = -1;
			aip->target_signature = 0;
			aip->target_time = 0.0f;

			set_targeted_subsys(aip, NULL, -1);
		}
	}

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(&Objects[aip->target_objnum], aip->targeted_subsys)));
}

ADE_VIRTVAR(Team, l_Ship, "team", "Ship's team", "team", "Ship team, or invalid team handle if ship handle is invalid")
{
	object_h *oh=NULL;
	int nt=-1;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&oh), l_Team.Get(&nt)))
		return ade_set_error(L, "o", l_Team.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Team.Set(-1));

	ship *shipp = &Ships[oh->objp->instance];

	if(ADE_SETTING_VAR && nt > -1) {
		shipp->team = nt;
	}

	return ade_set_args(L, "o", l_Team.Set(shipp->team));
}

ADE_VIRTVAR(Textures, l_Ship, "shiptextures", "Gets ship textures", "shiptextures", "Ship textures, or invalid shiptextures handle if ship handle is invalid")
{
	object_h *sh = nullptr;
	object_h *dh;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&dh), l_Ship.GetPtr(&sh)))
		return ade_set_error(L, "o", l_ShipTextures.Set(object_h()));

	if(!dh->IsValid())
		return ade_set_error(L, "o", l_ShipTextures.Set(object_h()));

	if(ADE_SETTING_VAR && sh && sh->IsValid()) {
		ship *src = &Ships[sh->objp->instance];
		ship *dest = &Ships[dh->objp->instance];
		
		if (src->ship_replacement_textures != NULL)
		{
			if (dest->ship_replacement_textures == NULL)
				dest->ship_replacement_textures = (int *) vm_malloc(MAX_REPLACEMENT_TEXTURES * sizeof(int));

			memcpy(dest->ship_replacement_textures, src->ship_replacement_textures, MAX_REPLACEMENT_TEXTURES * sizeof(int));
		}
	}

	return ade_set_args(L, "o", l_ShipTextures.Set(object_h(dh->objp)));
}

ADE_VIRTVAR(FlagAffectedByGravity, l_Ship, "boolean", "Checks for the \"affected-by-gravity\" flag", "boolean", "True if flag is set, false if flag is not set and nil on error") 
{
	object_h *objh=NULL;
	bool set = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &set))
		return ADE_RETURN_NIL;

	if (!objh->IsValid())
		return ADE_RETURN_NIL;
	
	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
    {
		shipp->flags.set(Ship::Ship_Flags::Affected_by_gravity, set);
    }

	if (shipp->flags[Ship::Ship_Flags::Affected_by_gravity])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

extern void ship_reset_disabled_physics(object *objp, int ship_class);
ADE_VIRTVAR(Disabled, l_Ship, "boolean", "The disabled state of this ship", "boolean", "true if ship is diabled, false otherwise")
{
	object_h *objh=NULL;
	bool set = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &set))
		return ADE_RETURN_FALSE;

	if (!objh->IsValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
	{
		shipp->flags.set(Ship::Ship_Flags::Disabled, set);
		if(set)
		{
			mission_log_add_entry(LOG_SHIP_DISABLED, shipp->ship_name, NULL );
		}
		else
		{
			ship_reset_disabled_physics( &Objects[shipp->objnum], shipp->ship_info_index );
		}
	}

	if (shipp->flags[Ship::Ship_Flags::Disabled])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Stealthed, l_Ship, "boolean", "Stealth status of this ship", "boolean", "true if stealthed, false otherwise or on error")
{
	object_h *objh=NULL;
	bool stealthed = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &stealthed))
		return ADE_RETURN_FALSE;

	if (!objh->IsValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
	{
        shipp->flags.set(Ship::Ship_Flags::Stealth, stealthed);
	}

	if (shipp->flags[Ship::Ship_Flags::Stealth])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(HiddenFromSensors, l_Ship, "boolean", "Hidden from sensors status of this ship", "boolean", "true if invisible to hidden from sensors, false otherwise or on error")
{
	object_h *objh=NULL;
	bool hidden = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &hidden))
		return ADE_RETURN_FALSE;

	if (!objh->IsValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
	{
        shipp->flags.set(Ship::Ship_Flags::Hidden_from_sensors, hidden);
	}

	if (shipp->flags[Ship::Ship_Flags::Hidden_from_sensors])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Gliding, l_Ship, "boolean", "Specifies whether this ship is currently gliding or not.", "boolean", "true if gliding, false otherwise or in case of error")
{
	object_h *objh=NULL;
	bool gliding = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &gliding))
		return ADE_RETURN_FALSE;

	if (!objh->IsValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp->instance];

	if(ADE_SETTING_VAR)
	{
		if (Ship_info[shipp->ship_info_index].can_glide)
		{
			object_set_gliding(&Objects[shipp->objnum], gliding, true);
		}
	}

	if (objh->objp->phys_info.flags & PF_GLIDING || objh->objp->phys_info.flags & PF_FORCE_GLIDE)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(EtsEngineIndex, l_Ship, "number", "(SET not implemented, see EtsSetIndexes)", "number", "Ships ETS Engine index value, 0 to MAX_ENERGY_INDEX")
{
	object_h *objh=NULL;
	int ets_idx = 0;

	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &ets_idx))
		return ade_set_error(L, "i", 0);

	if (!objh->IsValid())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR)
		LuaError(L, "Attempted to set incomplete feature: ETS Engine Index (see EtsSetIndexes)");

	return ade_set_args(L, "i", Ships[objh->objp->instance].engine_recharge_index);
}

ADE_VIRTVAR(EtsShieldIndex, l_Ship, "number", "(SET not implemented, see EtsSetIndexes)", "number", "Ships ETS Shield index value, 0 to MAX_ENERGY_INDEX")
{
	object_h *objh=NULL;
	int ets_idx = 0;

	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &ets_idx))
		return ade_set_error(L, "i", 0);

	if (!objh->IsValid())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR)
		LuaError(L, "Attempted to set incomplete feature: ETS Shield Index (see EtsSetIndexes)");

	return ade_set_args(L, "i", Ships[objh->objp->instance].shield_recharge_index);
}

ADE_VIRTVAR(EtsWeaponIndex, l_Ship, "number", "(SET not implemented, see EtsSetIndexes)", "number", "Ships ETS Weapon index value, 0 to MAX_ENERGY_INDEX")
{
	object_h *objh=NULL;
	int ets_idx = 0;

	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &ets_idx))
		return ade_set_error(L, "i", 0);

	if (!objh->IsValid())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR)
		LuaError(L, "Attempted to set incomplete feature: ETS Weapon Index (see EtsSetIndexes)");

	return ade_set_args(L, "i", Ships[objh->objp->instance].weapon_recharge_index);
}

ADE_VIRTVAR(Orders, l_Ship, "shiporders", "Array of ship orders", "shiporders", "Ship orders, or invalid handle if ship handle is invalid")
{
	object_h *objh = NULL;
	object_h *newh = NULL;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_ShipOrders.GetPtr(&newh)))
		return ade_set_error(L, "o", l_ShipOrders.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_ShipOrders.Set(object_h()));;

	if(ADE_SETTING_VAR)
	{
		LuaError(L, "Attempted to use incomplete feature: Ai orders copy. Use giveOrder instead");
	}

	return ade_set_args(L, "o", l_ShipOrders.Set(object_h(objh->objp)));
}

ADE_FUNC(kill, l_Ship, "[object Killer]", "Kills the ship. Set \"Killer\" to the ship you are killing to self-destruct", "boolean", "True if successful, false or nil otherwise")
{
	object_h *victim,*killer=NULL;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&victim), l_Ship.GetPtr(&killer)))
		return ADE_RETURN_NIL;

	if(!victim->IsValid())
		return ADE_RETURN_NIL;

	if(!killer || !killer->IsValid())
		return ADE_RETURN_NIL;

	//Ripped straight from shiphit.cpp
	float percent_killed = -get_hull_pct(victim->objp);
	if (percent_killed > 1.0f){
		percent_killed = 1.0f;
	}

	ship_hit_kill(victim->objp, killer->objp, percent_killed, (victim->sig == killer->sig) ? 1 : 0);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(addShipEffect, l_Ship, "string name, int duration (in milliseconds)", "Activates an effect for this ship. Effect names are defined in Post_processing.tbl, and need to be implemented in the main shader. This functions analogous to the ship-effect sexp. NOTE: only one effect can be active at any time, adding new effects will override effects already in progress.\n", "boolean", "Returns true if the effect was successfully added, false otherwise") {
	object_h *shiph;
	char* effect = NULL;
	int duration;
	int effect_num;

	if (!ade_get_args(L, "o|si", l_Ship.GetPtr(&shiph), &effect, &duration))
		return ade_set_error(L, "b", false);

	if (!shiph->IsValid())
		return ade_set_error(L, "b", false);

	effect_num = get_effect_from_name(effect);
	if (effect_num == -1)
		return ade_set_error(L, "b", false);

	ship* shipp = &Ships[shiph->objp->instance];

	shipp->shader_effect_active = true;
	shipp->shader_effect_num = effect_num;
	shipp->shader_effect_duration = duration;
	shipp->shader_effect_start_time = timer_get_milliseconds();

	return ade_set_args(L, "b", true);
}

ADE_FUNC(hasShipExploded, l_Ship, NULL, "Checks if the ship explosion event has already happened", "number", "Returns 1 if first explosion timestamp is passed, 2 if second is passed, 0 otherwise")
{
	object_h *shiph;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&shiph)))
		return ade_set_error(L, "i", 0);

	if(!shiph->IsValid())
		return ade_set_error(L, "i", 0);

	ship *shipp = &Ships[shiph->objp->instance];

	if (shipp->flags[Ship::Ship_Flags::Dying]) {
		if (shipp->final_death_time == 0) {
			return ade_set_args(L, "i", 2);
		}		
		if (shipp->pre_death_explosion_happened == 1) {
			return ade_set_args(L, "i", 1);
		}
		return ade_set_args(L, "i", 3);
	}

	return ade_set_args(L, "i", 0);
}

ADE_FUNC(fireCountermeasure, l_Ship, NULL, "Launches a countermeasure from the ship", "boolean", "Whether countermeasure was launched or not")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "b", false);

	if(!objh->IsValid())
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", ship_launch_countermeasure(objh->objp));
}

ADE_FUNC(firePrimary, l_Ship, NULL, "Fires ship primary bank(s)", "number", "Number of primary banks fired")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "i", 0);

	if(!objh->IsValid())
		return ade_set_error(L, "i", 0);

	int i = 0;
	i += ship_fire_primary(objh->objp, 0);
	i += ship_fire_primary(objh->objp, 1);

	return ade_set_args(L, "i", i);
}

ADE_FUNC(fireSecondary, l_Ship, NULL, "Fires ship secondary bank(s)", "number", "Number of secondary banks fired")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "i", 0);

	if(!objh->IsValid())
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", ship_fire_secondary(objh->objp, 0));
}

ADE_FUNC(getAnimationDoneTime, l_Ship, "number Type, number Subtype", "Gets time that animation will be done", "number", "Time (seconds), or 0 if ship handle is invalid")
{
	object_h *objh;
	char *s = NULL;
	int subtype=-1;
	if(!ade_get_args(L, "o|si", l_Ship.GetPtr(&objh), &s, &subtype))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	int type = model_anim_match_type(s);
	if(type < 0)
		return ADE_RETURN_FALSE;

	int time_ms = model_anim_get_time_type(&Ships[objh->objp->instance], type, subtype);
	float time_s = (float)time_ms / 1000.0f;

	return ade_set_args(L, "f", time_s);
}

ADE_FUNC(clearOrders, l_Ship, NULL, "Clears a ship's orders list", "boolean", "True if successful, otherwise false or nil")
{
	object_h *objh = NULL;
	if(!ade_get_args(L, "o", l_Object.GetPtr(&objh)))
		return ADE_RETURN_NIL;
	if(!objh->IsValid())
		return ade_set_error(L, "b", false);

	//The actual clearing of the goals
	ai_clear_ship_goals( &Ai_info[Ships[objh->objp->instance].ai_index]);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(giveOrder, l_Ship, "enumeration Order, [object Target=nil, subsystem TargetSubsystem=nil, number Priority=1.0, shipclass TargetShipclass=nil]", "Uses the goal code to execute orders", "boolean", "True if order was given, otherwise false or nil")
{
	object_h *objh = NULL;
	enum_h *eh = NULL;
	float priority = 1.0f;
	int sclass = -1;
	object_h *tgh = NULL;
	ship_subsys_h *tgsh = NULL;
	if(!ade_get_args(L, "oo|oofo", l_Object.GetPtr(&objh), l_Enum.GetPtr(&eh), l_Object.GetPtr(&tgh), l_Subsystem.GetPtr(&tgsh), &priority, l_Shipclass.Get(&sclass)))
		return ADE_RETURN_NIL;

	if(!objh->IsValid() || !eh->IsValid())
		return ade_set_error(L, "b", false);

	//wtf...
	if(priority < 0.0f)
		return ade_set_error(L, "b", false);

	if(priority > 1.0f)
		priority = 1.0f;

	bool tgh_valid = tgh && tgh->IsValid();
	bool tgsh_valid = tgsh && tgsh->IsValid();
	int ai_mode = AI_GOAL_NONE;
	int ai_submode = -1234567;
	char *ai_shipname = NULL;
	switch(eh->index)
	{
		case LE_ORDER_ATTACK:
		{
			if(tgsh_valid)
			{
				ai_mode = AI_GOAL_DESTROY_SUBSYSTEM;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = ship_get_subsys_index( &Ships[tgsh->objp->instance], tgsh->ss->system_info->subobj_name );
			}
			else if(tgh_valid && tgh->objp->type == OBJ_WEAPON)
			{
				ai_mode = AI_GOAL_CHASE_WEAPON;
				ai_submode = tgh->objp->instance;
			}
			else if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_CHASE;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = SM_ATTACK;
			}
			break;
		}
		case LE_ORDER_DOCK:
		{
			ai_shipname = Ships[tgh->objp->instance].ship_name;
			ai_mode = AI_GOAL_DOCK;
			ai_submode = AIS_DOCK_0;
			break;
		}
		case LE_ORDER_WAYPOINTS:
		{
			if(tgh_valid && tgh->objp->type == OBJ_WAYPOINT)
			{
				ai_mode = AI_GOAL_WAYPOINTS;
				waypoint_list *wp_list = find_waypoint_list_with_instance(tgh->objp->instance);
				if(wp_list != NULL)
					ai_shipname = wp_list->get_name();
			}
			break;
		}
		case LE_ORDER_WAYPOINTS_ONCE:
		{
			if(tgh_valid && tgh->objp->type == OBJ_WAYPOINT)
			{
				ai_mode = AI_GOAL_WAYPOINTS_ONCE;
				waypoint_list *wp_list = find_waypoint_list_with_instance(tgh->objp->instance);
				if(wp_list != NULL)
					ai_shipname = wp_list->get_name();
			}
			break;
		}
		case LE_ORDER_DEPART:
		{
			ai_mode = AI_GOAL_WARP;
			ai_submode = -1;
			break;
		}
		case LE_ORDER_FORM_ON_WING:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_FORM_ON_WING;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = 0;
			}
			break;
		}
		case LE_ORDER_UNDOCK:
		{
			ai_mode = AI_GOAL_UNDOCK;
			ai_submode = AIS_UNDOCK_0;

			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_GUARD:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_GUARD;
				ai_submode = AIS_GUARD_PATROL;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_DISABLE:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_DISABLE_SHIP;
				ai_submode = -SUBSYSTEM_ENGINE;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_DISARM:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_DISARM_SHIP;
				ai_submode = -SUBSYSTEM_TURRET;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_ATTACK_ANY:
		{
			ai_mode = AI_GOAL_CHASE_ANY;
			ai_submode = SM_ATTACK;
			break;
		}
		case LE_ORDER_IGNORE:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_IGNORE_NEW;
				ai_submode = 0;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_EVADE:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_EVADE_SHIP;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_STAY_NEAR:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_STAY_NEAR_SHIP;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = -1;
			}
			break;
		}
		case LE_ORDER_KEEP_SAFE_DISTANCE:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_KEEP_SAFE_DISTANCE;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = -1;
			}
			break;
		}
		case LE_ORDER_REARM:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_REARM_REPAIR;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = 0;
			}
			break;
		}
		case LE_ORDER_STAY_STILL:
		{
			ai_mode = AI_GOAL_STAY_STILL;
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_shipname = Ships[tgh->objp->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_PLAY_DEAD:
		{
			ai_mode = AI_GOAL_PLAY_DEAD;
			break;
		}
		case LE_ORDER_FLY_TO:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_FLY_TO_SHIP;
				ai_shipname = Ships[tgh->objp->instance].ship_name;
				ai_submode = 0;
			}
			break;
		}
		case LE_ORDER_ATTACK_WING:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ship *shipp = &Ships[tgh->objp->instance];
				if (shipp->wingnum != -1)
				{
					ai_mode = AI_GOAL_CHASE_WING;
					ai_shipname = Wings[shipp->wingnum].name;
					ai_submode = SM_ATTACK;
				}
			}
			break;
		}
		case LE_ORDER_GUARD_WING:
		{
			if(tgh_valid && tgh->objp->type == OBJ_SHIP)
			{
				ship *shipp = &Ships[tgh->objp->instance];
				if (shipp->wingnum != -1)
				{
					ai_mode = AI_GOAL_GUARD_WING;
					ai_shipname = Wings[shipp->wingnum].name;
					ai_submode = AIS_GUARD_STATIC;
				}
			}
			break;
		}
		case LE_ORDER_ATTACK_SHIP_CLASS:
		{
			if(sclass >= 0)
			{
				ai_mode = AI_GOAL_CHASE_SHIP_CLASS;
				ai_shipname = Ship_info[sclass].name;
				ai_submode = SM_ATTACK;
			}
			break;
		}
	}

	//Nothing got set!
	if(ai_mode == AI_GOAL_NONE)
		return ade_set_error(L, "b", false);

	//Fire off the goal
	ai_add_ship_goal_scripting(ai_mode, ai_submode, (int)(priority*100.0f), ai_shipname, &Ai_info[Ships[objh->objp->instance].ai_index]);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(doManeuver, l_Ship, "number Duration, number Heading, number Pitch, number Bank, boolean Force Rotation, number Vertical, number Horizontal, number Forward, boolean Force Movement", "Sets ship maneuver over the defined time period", "boolean", "True if maneuver order was given, otherwise false or nil")
{
	object_h *objh;
	float arr[6];
	bool f_rot = false, f_move = false;
	int t, i;
	if(!ade_get_args(L, "oifffbfffb", l_Ship.GetPtr(&objh), &t, &arr[0], &arr[1], &arr[2], &f_rot, &arr[3], &arr[4], &arr[5], &f_move))
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];
	ai_info *aip = &Ai_info[shipp->ai_index];
	control_info *cip = &aip->ai_override_ci;

	aip->ai_override_timestamp = timestamp(t);
	aip->ai_override_flags.reset();

	if (t < 2)
		return ADE_RETURN_FALSE;

	for(i = 0; i < 6; i++) {
		if((arr[i] < -1.0f) || (arr[i] > 1.0f))
			arr[i] = 0;
	}

	if(f_rot) {
		aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Full);
		cip->heading = arr[0];
		cip->pitch = arr[1];
		cip->bank = arr[2];
	} else {
		if (arr[0] != 0) {
			cip->heading = arr[0];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Heading);
		} 
		if (arr[1] != 0) {
			cip->pitch = arr[1];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Pitch);
		} 
		if (arr[2] != 0) {
			cip->bank = arr[2];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Roll);
		} 
	}
	if(f_move) {
		aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Full_lat);
		cip->vertical = arr[3];
		cip->sideways = arr[4];
		cip->forward = arr[5];	
	} else {
		if (arr[3] != 0) {
			cip->vertical = arr[3];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Up);
		} 
		if (arr[4] != 0) {
			cip->sideways = arr[4];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Sideways);
		} 
		if (arr[5] != 0) {
			cip->forward = arr[5];
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Forward);
		} 
	}
	return ADE_RETURN_TRUE;
}

ADE_FUNC(triggerAnimation, l_Ship, "string Type, [number Subtype, boolean Forwards, boolean Instant]",
		 "Triggers an animation. Type is the string name of the animation type, "
		 "Subtype is the subtype number, such as weapon bank #, Forwards and Instant are boolean, defaulting to true & false respectively."
		 "<br><strong>IMPORTANT: Function is in testing and should not be used with official mod releases</strong>",
		 "boolean",
		 "True if successful, false or nil otherwise")
{
	object_h *objh;
	char *s = NULL;
	bool b = true;
	bool instant = false;
	int subtype=-1;
	if(!ade_get_args(L, "o|sibb", l_Ship.GetPtr(&objh), &s, &subtype, &b, &instant))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	int type = model_anim_match_type(s);
	if(type < 0)
		return ADE_RETURN_FALSE;

	int dir = 1;
	if(!b)
		dir = -1;

	model_anim_start_type(&Ships[objh->objp->instance], type, subtype, dir, instant);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(warpIn, l_Ship, NULL, "Warps ship in", "boolean", "True if successful, or nil if ship handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	shipfx_warpin_start(objh->objp);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(warpOut, l_Ship, NULL, "Warps ship out", "boolean", "True if successful, or nil if ship handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	shipfx_warpout_start(objh->objp);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(canWarp, l_Ship, NULL, "Checks whether ship has a working subspace drive and is allowed to use it", "boolean", "True if successful, or nil if ship handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];
	if(shipp->flags[Ship::Ship_Flags::No_subspace_drive]){
		return ADE_RETURN_FALSE;
	}

	return ADE_RETURN_TRUE;
}

// Aardwolf's function for finding if a ship should be drawn as blue on the radar/minimap
ADE_FUNC(isWarpingIn, l_Ship, NULL, "Checks if ship is warping in", "boolean", "True if the ship is warping in, false or nil otherwise")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
	return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];
	if(shipp->flags[Ship::Ship_Flags::Arriving_stage_1]){
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

ADE_FUNC(getEMP, l_Ship, NULL, "Returns the current emp effect strength acting on the object", "number", "Current EMP effect strength or NIL if object is invalid")
{
	object_h *objh = NULL;
	object *obj = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh))) {
		return ADE_RETURN_NIL;
	}

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	obj = objh->objp;

	ship *shipp = &Ships[obj->instance];

	return ade_set_args(L, "f", shipp->emp_intensity);
}

ADE_FUNC(getTimeUntilExplosion, l_Ship, NULL, "Returns the time in seconds until the ship explodes", "number", "Time until explosion or -1, if invalid handle or ship isn't exploding")
{
	object_h *objh = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh))) {
		return ade_set_error(L, "f", -1.0f);
	}

	if(!objh->IsValid())
		return ade_set_error(L, "f", -1.0f);

	ship *shipp = &Ships[objh->objp->instance];

	if (!timestamp_valid(shipp->final_death_time))
	{
		return ade_set_args(L, "f", -1.0f);
	}

	int time_until = timestamp_until(shipp->final_death_time);

	return ade_set_args(L, "f", (i2fl(time_until) / 1000.0f));
}

ADE_FUNC(getCallsign, l_Ship, NULL, "Gets the callsign of the ship in the current mission", "string", "The callsign or an empty string if the ship doesn't have a callsign or an error occurs")
{
	object_h *objh = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh))) {
		return ade_set_error(L, "s", "");
	}

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp->instance];

	if (shipp->callsign_index < 0)
		return ade_set_args(L, "s", "");
	
	char temp_callsign[NAME_LENGTH];
	
	*temp_callsign = 0;
	mission_parse_lookup_callsign_index(shipp->callsign_index, temp_callsign);

	if (*temp_callsign)
		return ade_set_args(L, "s", temp_callsign);
	else
		return ade_set_args(L, "s", "");
}

ADE_FUNC(getAltClassName, l_Ship, NULL, "Gets the alternate class name of the ship", "string", "The alternate class name or an empty string if the ship doesn't have such a thing or an error occurs")
{
	object_h *objh = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh))) {
		return ade_set_error(L, "s", "");
	}

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp->instance];

	if (shipp->alt_type_index < 0)
		return ade_set_args(L, "s", "");
	
	char temp[NAME_LENGTH];
	
	*temp = 0;
	mission_parse_lookup_alt_index(shipp->alt_type_index, temp);

	if (*temp)
		return ade_set_args(L, "s", temp);
	else
		return ade_set_args(L, "s", "");
}

ADE_FUNC(getMaximumSpeed, l_Ship, "[number energy = 0.333]", "Gets the maximum speed of the ship with the given energy on the engines", "number", "The maximum speed or -1 on error")
{
	object_h *objh = NULL;
	float energy = 0.333f;

	if (!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &energy)) {
		return ade_set_error(L, "f", -1.0f);
	}

	if(!objh->IsValid())
		return ade_set_error(L, "f", -1.0f);

	if (energy < 0.0f || energy > 1.0f)
	{
		LuaError(L, "Invalid energy level %f! Needs to be in [0, 1].", energy);

		return ade_set_args(L, "f", -1.0f);
	}
	else
	{
		return ade_set_args(L, "f", ets_get_max_speed(objh->objp, energy));
	}
}

ADE_FUNC(EtsSetIndexes, l_Ship, "number Engine Index, number Shield Index, number Weapon Index",
		"Sets ships ETS systems to specified values",
		"boolean",
		"True if successful, false if target ships ETS was missing, or only has one system")
{
	object_h *objh=NULL;
	int ets_idx[num_retail_ets_gauges] = {0};

	if (!ade_get_args(L, "oiii", l_Ship.GetPtr(&objh), &ets_idx[ENGINES], &ets_idx[SHIELDS], &ets_idx[WEAPONS]))
		return ADE_RETURN_FALSE;

	if (!objh->IsValid())
		return ADE_RETURN_FALSE;

	sanity_check_ets_inputs(ets_idx);

	int sindex = objh->objp->instance;
	if (validate_ship_ets_indxes(sindex, ets_idx)) {
		Ships[sindex].engine_recharge_index = ets_idx[ENGINES];
		Ships[sindex].shield_recharge_index = ets_idx[SHIELDS];
		Ships[sindex].weapon_recharge_index = ets_idx[WEAPONS];
		return ADE_RETURN_TRUE;
	} else {
		return ADE_RETURN_FALSE;
	}
}

ADE_FUNC(getWing, l_Ship, NULL, "Returns the ship's wing", "wing", "Wing handle, or invalid wing handle if ship is not part of a wing")
{
	object_h *objh = NULL;
	ship *shipp = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "o", l_Wing.Set(-1));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Wing.Set(-1));

	shipp = &Ships[objh->objp->instance];
	return ade_set_args(L, "o", l_Wing.Set(shipp->wingnum));
}

//**********HANDLE: Weapon
ade_obj<object_h> l_Weapon("weapon", "Weapon handle", &get_l_Object());

ADE_VIRTVAR(Class, l_Weapon, "weaponclass", "Weapon's class", "weaponclass", "Weapon class, or invalid weaponclass handle if weapon handle is invalid")
{
	object_h *oh=NULL;
	int nc=-1;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&oh), l_Weaponclass.Get(&nc)))
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && nc > -1) {
		wp->weapon_info_index = nc;
	}

	return ade_set_args(L, "o", l_Weaponclass.Set(wp->weapon_info_index));
}

ADE_VIRTVAR(DestroyedByWeapon, l_Weapon, "boolean", "Whether weapon was destroyed by another weapon", "boolean", "True if weapon was destroyed by another weapon, false if weapon was destroyed by another object or if weapon handle is invalid")
{
	object_h *oh=NULL;
	bool b = false;

	int numargs = ade_get_args(L, "o|b", l_Weapon.GetPtr(&oh), &b);
	
	if(!numargs)
		return ade_set_error(L, "b", false);

	if(!oh->IsValid())
		return ade_set_error(L, "b", false);

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && numargs > 1) {
		wp->weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon, b);
	}

	return ade_set_args(L, "b", wp->weapon_flags[Weapon::Weapon_Flags::Destroyed_by_weapon]);
}

ADE_VIRTVAR(LifeLeft, l_Weapon, "number", "Weapon life left (in seconds)", "number", "Life left (seconds) or 0 if weapon handle is invalid")
{
	object_h *oh=NULL;
	float nll = -1.0f;
	if(!ade_get_args(L, "o|f", l_Weapon.GetPtr(&oh), &nll))
		return ade_set_error(L, "f", 0.0f);

	if(!oh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && nll >= 0.0f) {
		wp->lifeleft = nll;
	}

	return ade_set_args(L, "f", wp->lifeleft);
}

ADE_VIRTVAR(FlakDetonationRange, l_Weapon, "number", "Range at which flak will detonate (meters)", "number", "Detonation range (meters) or 0 if weapon handle is invalid")
{
	object_h *oh=NULL;
	float rng = -1.0f;
	if(!ade_get_args(L, "o|f", l_Weapon.GetPtr(&oh), &rng))
		return ade_set_error(L, "f", 0.0f);

	if(!oh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && rng >= 0.0f) {
		wp->det_range = rng;
	}

	return ade_set_args(L, "f", wp->det_range);
}

ADE_VIRTVAR(Target, l_Weapon, "object", "Target of weapon. Value may also be a deriviative of the 'object' class, such as 'ship'.", "object", "Weapon target, or invalid object handle if weapon handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if(wp->target_sig != newh->sig)
			{
				weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, OBJ_INDEX(newh->objp), 1);
			}
		}
		else
		{
			weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, -1);
		}
	}

	return ade_set_object_with_breed(L, wp->target_num);
}

ADE_VIRTVAR(ParentTurret, l_Weapon, "subsystem", "Turret which fired this weapon.", "subsystem", "Turret subsystem handle, or an invalid handle if the weapon not fired from a turret")
{
	object_h *objh;
	ship_subsys_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if(wp->turret_subsys != newh->ss)
			{
				wp->turret_subsys = newh->ss;
			}
		}
		else
		{
			wp->turret_subsys = NULL;
		}
	}

    if(wp->turret_subsys == NULL)
        return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));
    else
        return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(&Objects[wp->turret_subsys->parent_objnum], wp->turret_subsys)));
}

ADE_VIRTVAR(HomingObject, l_Weapon, "object", "Object that weapon will home in on. Value may also be a deriviative of the 'object' class, such as 'ship'", "object", "Object that weapon is homing in on, or an invalid object handle if weapon is not homing or the weapon handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR)
	{
		if (newh && newh->IsValid())
		{
			if (wp->target_sig != newh->sig)
			{
				weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, OBJ_INDEX(newh->objp), 1);
			}
		}
		else
		{
			weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, -1);
		}
	}

	if(wp->homing_object == &obj_used_list)
		return ade_set_args(L, "o", l_Object.Set(object_h()));
	else
		return ade_set_object_with_breed(L, OBJ_INDEX(wp->homing_object));
}

ADE_VIRTVAR(HomingPosition, l_Weapon, "vector", "Position that weapon will home in on (World vector), setting this without a homing object in place will not have any effect!",
	"vector", "Homing point, or null vector if weapon handle is invalid")
{
	object_h *objh;
	vec3d *v3 = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR)
	{
		if(v3)
		{
			wp->homing_pos = *v3;
		}
		else
		{
			wp->homing_pos = vmd_zero_vector;
		}
	}

	return ade_set_args(L, "o", l_Vector.Set(wp->homing_pos));
}

ADE_VIRTVAR(HomingSubsystem, l_Weapon, "subsystem", "Subsystem that weapon will home in on.", "subsystem", "Homing subsystem, or invalid subsystem handle if weapon is not homing or weapon handle is invalid")
{
	object_h *objh;
	ship_subsys_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if(wp->target_sig != newh->sig)
			{
				wp->homing_object = newh->objp;
				wp->homing_subsys = newh->ss;
				get_subsystem_pos(&wp->homing_pos, wp->homing_object, wp->homing_subsys);
			}
		}
		else
		{
			wp->homing_object = &obj_used_list;
			wp->homing_pos = vmd_zero_vector;
			wp->homing_subsys = NULL;
		}
	}

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(wp->homing_object, wp->homing_subsys)));
}

ADE_VIRTVAR(Team, l_Weapon, "team", "Weapon's team", "team", "Weapon team, or invalid team handle if weapon handle is invalid")
{
	object_h *oh=NULL;
	int nt=-1;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&oh), l_Team.Get(&nt)))
		return ade_set_error(L, "o", l_Team.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Team.Set(-1));

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && nt > -1 && nt < Num_teams) {
		wp->team = nt;
	}

	return ade_set_args(L, "o", l_Team.Set(wp->team));
}

ADE_FUNC(isArmed, l_Weapon, "[boolean Hit target]", "Checks if the weapon is armed.", "boolean", "boolean value of the weapon arming status")
{
	object_h *oh = NULL;
	bool hit_target = false;
	if(!ade_get_args(L, "o|b", l_Weapon.GetPtr(&oh), &hit_target))
		return ADE_RETURN_FALSE;

	if(!oh->IsValid())
		return ADE_RETURN_FALSE;

	weapon *wp = &Weapons[oh->objp->instance];
	
	if(weapon_armed(wp, hit_target))
		return ADE_RETURN_TRUE;
	
	return ADE_RETURN_FALSE;
}

ADE_FUNC(getCollisionInformation, l_Weapon, NULL, "Returns the collision information for this weapon", "collision info", "The collision information or invalid handle if none")
{
	object_h *oh=NULL;
	if(!ade_get_args(L, "o", l_Weapon.GetPtr(&oh)))
		return ADE_RETURN_NIL;

	if(!oh->IsValid())
		return ADE_RETURN_NIL;

	weapon *wp = &Weapons[oh->objp->instance];
	
	if (wp->collisionInfo != nullptr)
		return ade_set_args(L, "o", l_ColInfo.Set(mc_info_h(new mc_info(*(wp->collisionInfo)))));
	else
		return ade_set_args(L, "o", l_ColInfo.Set(mc_info_h()));
}


//**********HANDLE: Beam
ade_obj<object_h> l_Beam("beam", "Beam handle", &get_l_Object());

ADE_VIRTVAR(Class, l_Beam, "weaponclass", "Weapon's class", "weaponclass", "Weapon class, or invalid weaponclass handle if beam handle is invalid")
{
	object_h *oh=NULL;
	int nc=-1;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&oh), l_Weaponclass.Get(&nc)))
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	beam *bp = &Beams[oh->objp->instance];

	if(ADE_SETTING_VAR && nc > -1) {
		bp->weapon_info_index = nc;
	}

	return ade_set_args(L, "o", l_Weaponclass.Set(bp->weapon_info_index));
}

ADE_VIRTVAR(LastShot, l_Beam, "vector", "End point of the beam", "vector", "vector or null vector if beam handle is not valid")
{
	object_h *oh=NULL;
	vec3d *vec3 = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&oh), l_Vector.GetPtr(&vec3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam *bp = &Beams[oh->objp->instance];

	if(ADE_SETTING_VAR && vec3) {
		bp->last_shot = *vec3;
	}

	return ade_set_args(L, "o", l_Vector.Set(bp->last_shot));
}

ADE_VIRTVAR(LastStart, l_Beam, "vector", "Start point of the beam", "vector", "vector or null vector if beam handle is not valid")
{
	object_h *oh=NULL;
	vec3d *v3 = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&oh), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam *bp = &Beams[oh->objp->instance];

	if(ADE_SETTING_VAR && v3) {
		bp->last_start = *v3;
	}

	return ade_set_args(L, "o", l_Vector.Set(bp->last_start));
}

ADE_VIRTVAR(Target, l_Beam, "object", "Target of beam. Value may also be a deriviative of the 'object' class, such as 'ship'.", "object", "Beam target, or invalid object handle if beam handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if(bp->target_sig != newh->sig)
			{
				bp->target = newh->objp;
				bp->target_sig = newh->sig;
			}
		}
		else
		{
			bp->target = NULL;
			bp->target_sig = 0;
		}
	}

	return ade_set_object_with_breed(L, OBJ_INDEX(bp->target));
}

ADE_VIRTVAR(TargetSubsystem, l_Beam, "subsystem", "Subsystem that beam is targeting.", "subsystem", "Target subsystem, or invalid subsystem handle if beam handle is invalid")
{
	object_h *objh;
	ship_subsys_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&objh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if(bp->target_sig != newh->sig)
			{
				bp->target = newh->objp;
				bp->target_subsys = newh->ss;
				bp->target_sig = newh->sig;
			}
		}
		else
		{
			bp->target = NULL;
			bp->target_subsys = NULL;
		}
	}

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(bp->target, bp->target_subsys)));
}

ADE_VIRTVAR(ParentShip, l_Beam, "object", "Parent of the beam.", "object", "Beam parent, or invalid object handle if beam handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if(bp->sig != newh->sig)
			{
				bp->objp = newh->objp;
				bp->sig = newh->sig;
			}
		}
		else
		{
			bp->objp = NULL;
			bp->sig = 0;
		}
	}

	return ade_set_object_with_breed(L, OBJ_INDEX(bp->objp));
}

ADE_VIRTVAR(ParentSubsystem, l_Beam, "subsystem", "Subsystem that beam is fired from.", "subsystem", "Parent subsystem, or invalid subsystem handle if beam handle is invalid")
{
	object_h *objh;
	ship_subsys_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&objh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if(bp->sig != newh->sig)
			{
				bp->objp = newh->objp;
				bp->subsys = newh->ss;
			}
		}
		else
		{
			bp->objp = NULL;
			bp->subsys = NULL;
		}
	}

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(bp->objp, bp->subsys)));
}

ADE_FUNC(getCollisionCount, l_Beam, NULL, "Get the number of collisions in frame.", "number", "Number of beam collisions")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Beam.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ADE_RETURN_NIL;

	return ade_set_args(L, "i", bp->f_collision_count);
}

ADE_FUNC(getCollisionPosition, l_Beam, "number", "Get the position of the defined collision.", "vector", "World vector")
{
	object_h *objh;
	int idx;
	if(!ade_get_args(L, "oi", l_Beam.GetPtr(&objh), &idx))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	// convert from Lua to C
	idx--;
	if ((idx >= MAX_FRAME_COLLISIONS) || (idx < 0))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	// so we have valid beam and valid indexer
	return ade_set_args(L, "o", l_Vector.Set(bp->f_collisions[idx].cinfo.hit_point_world));
}

ADE_FUNC(getCollisionInformation, l_Beam, "number", "Get the collision information of the specified collision", "collision info", "handle to information or invalid handle on error")
{
	object_h *objh;
	int idx;
	if(!ade_get_args(L, "oi", l_Beam.GetPtr(&objh), &idx))
		return ade_set_error(L, "o", l_ColInfo.Set(mc_info_h()));

	// convert from Lua to C
	idx--;
	if ((idx >= MAX_FRAME_COLLISIONS) || (idx < 0))
		return ade_set_error(L, "o", l_ColInfo.Set(mc_info_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_ColInfo.Set(mc_info_h()));

	// so we have valid beam and valid indexer
	return ade_set_args(L, "o", l_ColInfo.Set(mc_info_h(new mc_info(bp->f_collisions[idx].cinfo))));
}

ADE_FUNC(getCollisionObject, l_Beam, "number", "Get the target of the defined collision.", "object", "Object the beam collided with")
{
	object_h *objh;
	int idx;
	if(!ade_get_args(L, "oi", l_Beam.GetPtr(&objh), &idx))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	// convert from Lua to C
	idx--;
	if ((idx >= MAX_FRAME_COLLISIONS) || (idx < 0))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	// so we have valid beam and valid indexer
	return ade_set_object_with_breed(L, bp->f_collisions[idx].c_objnum);
}

ADE_FUNC(isExitCollision, l_Beam, "number", "Checks if the defined collision was exit collision.", "boolean", "True if the collision was exit collision, false if entry, nil otherwise")
{
	object_h *objh;
	int idx;
	if(!ade_get_args(L, "oi", l_Beam.GetPtr(&objh), &idx))
		return ADE_RETURN_NIL;

	// convert from Lua to C
	idx--;
	if ((idx >= MAX_FRAME_COLLISIONS) || (idx < 0))
		return ADE_RETURN_NIL;

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ADE_RETURN_NIL;

	// so we have valid beam and valid indexer
	if (bp->f_collisions[idx].is_exit_collision)
		return ADE_RETURN_TRUE;
	
	return ADE_RETURN_FALSE;
}

ADE_FUNC(getStartDirectionInfo, l_Beam, NULL, "Gets the start information about the direction. The vector is a normalized vector from LastStart showing the start direction of a slashing beam", "vector", "The start direction or null vector if invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Beam.GetPtr(&objh)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	
	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam_info inf = bp->binfo;
		
	return ade_set_args(L, "o", l_Vector.Set(inf.dir_a));
}

ADE_FUNC(getEndDirectionInfo, l_Beam, NULL, "Gets the end information about the direction. The vector is a normalized vector from LastStart showing the end direction of a slashing beam", "vector", "The start direction or null vector if invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Beam.GetPtr(&objh)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	
	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam_info inf = bp->binfo;
		
	return ade_set_args(L, "o", l_Vector.Set(inf.dir_b));
}

//**********HANDLE: Player
ade_obj<int> l_Player("player", "Player handle");

ADE_FUNC(isValid, l_Player, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if(!ade_get_args(L, "o", l_Player.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= MAX_PLAYERS)
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getName, l_Player, NULL, "Gets current player name", "string", "Player name, or empty string if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Player.Get(&idx)))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= MAX_PLAYERS)
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Players[idx].callsign);
}

ADE_FUNC(getCampaignFilename, l_Player, NULL, "Gets current player campaign filename", "string", "Campaign name, or empty string if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Player.Get(&idx)))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= MAX_PLAYERS)
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Players[idx].current_campaign);
}

ADE_FUNC(getImageFilename, l_Player, NULL, "Gets current player image filename", "string", "Player image filename, or empty string if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Player.Get(&idx)))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= MAX_PLAYERS)
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Players[idx].image_filename);
}

ADE_FUNC(getMainHallName, l_Player, NULL, "Gets player's current main hall name", "string", "Main hall name, or name of first mainhall in campaign if something goes wrong")
{
	SCP_string hallname;
	// FS2-->Lua
	if (Campaign.next_mission == -1) {
		hallname = Campaign.missions[0].main_hall;
	} else {
		hallname = Campaign.missions[Campaign.next_mission].main_hall;
	}

	return ade_set_args(L, "i", hallname.c_str());
}

// use getMainHallName if at all possible.
ADE_FUNC(getMainHallIndex, l_Player, NULL, "Gets player's current main hall number", "number", "Main hall index, or index of first mainhall in campaign if something goes wrong")
{
	int hallnum = 0;
	//FS2-->Lua
	if (Campaign.next_mission == -1) {
		hallnum = main_hall_get_index(Campaign.missions[0].main_hall);
	} else {
		hallnum = main_hall_get_index(Campaign.missions[Campaign.next_mission].main_hall);
	}

	return ade_set_args(L, "i", hallnum);
}

ADE_FUNC(getSquadronName, l_Player, NULL, "Gets current player squad name", "string", "Squadron name, or empty string if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Player.Get(&idx)))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= MAX_PLAYERS)
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Players[idx].s_squad_name);
}

ADE_FUNC(getMultiSquadronName, l_Player, NULL, "Gets current player multi squad name", "string", "Squadron name, or empty string if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Player.Get(&idx)))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= MAX_PLAYERS)
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Players[idx].m_squad_name);
}

//WMC - This isn't working
/*
ADE_FUNC(getSquadronImage, l_Player, NULL, "Squad image (string)", "Gets current player squad image")
{
	int idx;
	player_helper(L, &idx);

	return ade_set_args(L, "s", Players[idx].squad_filename);
}*/

//**********HANDLE: Camera
ade_obj<camid> l_Camera("camera", "Camera handle");

ADE_FUNC(__tostring, l_Camera, NULL, "Camera name", "string", "Camera name, or an empty string if handle is invalid")
{
	camid cid;
	if(!ade_get_args(L, "o", l_Camera.Get(&cid)))
		return ade_set_error(L, "s", "");

	if(!cid.isValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", cid.getCamera()->get_name());
}

ADE_FUNC(isValid, l_Camera, NULL, "True if valid, false or nil if not", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	camid cid;
	if(!ade_get_args(L, "o", l_Camera.Get(&cid)))
		return ADE_RETURN_NIL;

	if(!cid.isValid())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(Name, l_Camera, "string", "New camera name", "string", "Camera name")
{
	camid cid;
	char *s = NULL;
	if(!ade_get_args(L, "o|s", l_Camera.Get(&cid), &s))
		return ade_set_error(L, "s", "");

	if(!cid.isValid())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != NULL) {
		cid.getCamera()->set_name(s);
	}

	return ade_set_args(L, "s", cid.getCamera()->get_name());
}

ADE_VIRTVAR(FOV, l_Camera, "number", "New camera FOV (in radians)", "number", "Camera FOV (in radians)")
{
	camid cid;
	float f = VIEWER_ZOOM_DEFAULT;
	if(!ade_get_args(L, "o|f", l_Camera.Get(&cid), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!cid.isValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		cid.getCamera()->set_fov(f);
	}

	return ade_set_args(L, "f", cid.getCamera()->get_fov());
}

ADE_VIRTVAR(Orientation, l_Camera, "orientation", "New camera orientation", "orientation", "Camera orientation")
{
	camid cid;
	matrix_h *mh = NULL;
	if(!ade_get_args(L, "o|o", l_Camera.Get(&cid), l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "f", 0.0f);

	if(!cid.isValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR && mh != NULL) {
		cid.getCamera()->set_rotation(mh->GetMatrix());
	}

	matrix mtx;
	cid.getCamera()->get_info(NULL, &mtx);
	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&mtx)));
}

ADE_VIRTVAR(Position, l_Camera, "vector", "New camera position", "vector", "Camera position")
{
	camid cid;
	vec3d *pos = NULL;
	if(!ade_get_args(L, "o|o", l_Camera.Get(&cid), l_Vector.GetPtr(&pos)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!cid.isValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && pos) {
		cid.getCamera()->set_position(pos);
	}

	vec3d v = vmd_zero_vector;
	cid.getCamera()->get_info(&v, NULL);
	return ade_set_args(L, "o", l_Vector.Set(v));
}

ADE_VIRTVAR(Self, l_Camera, "object", "New mount object", "object", "Camera object")
{
	camid cid;
	object_h *oh = NULL;
	if(!ade_get_args(L, "o|o", l_Camera.Get(&cid), l_Object.GetPtr(&oh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!cid.isValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR && oh && oh->IsValid()) {
		cid.getCamera()->set_object_host(oh->objp);
	}

	return ade_set_args(L, "o", l_Object.Set(object_h(cid.getCamera()->get_object_host())));
}

ADE_VIRTVAR(SelfSubsystem, l_Camera, "subsystem", "New mount object subsystem", "subsystem", "Subsystem that the camera is mounted on")
{
	camid cid;
	ship_subsys_h *sso = NULL;
	if(!ade_get_args(L, "o|o", l_Camera.Get(&cid), l_Subsystem.GetPtr(&sso)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!cid.isValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR && sso && sso->IsValid()) {
		cid.getCamera()->set_object_host(sso->objp, sso->ss->system_info->subobj_num);
	}

	object *objp = cid.getCamera()->get_object_host();
	if(objp == NULL || objp->type != OBJ_SHIP)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	int submodel = cid.getCamera()->get_object_host_submodel();
	if(submodel < 0)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	ship *shipp = &Ships[objp->instance];
	polymodel *pm = model_get(Ship_info[shipp->ship_info_index].model_num);

	if(pm == NULL)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	bsp_info *sm = &pm->submodel[submodel];

	ship_subsys *ss = ship_get_subsys(shipp, sm->name);
	
	if(ss == NULL)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(objp, ss)));
}

ADE_VIRTVAR(Target, l_Camera, "object", "New target object", "object", "Camera target object")
{
	camid cid;
	object_h *oh = NULL;
	if(!ade_get_args(L, "o|o", l_Camera.Get(&cid), l_Object.GetPtr(&oh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!cid.isValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR && oh && oh->IsValid()) {
		cid.getCamera()->set_object_target(oh->objp);
	}

	return ade_set_args(L, "o", l_Object.Set(object_h(cid.getCamera()->get_object_target())));
}

ADE_VIRTVAR(TargetSubsystem, l_Camera, "subsystem", "New target subsystem", "subsystem", "Subsystem that the camera is pointed at")
{
	camid cid;
	ship_subsys_h *sso = NULL;
	if(!ade_get_args(L, "o|o", l_Camera.Get(&cid), l_Subsystem.GetPtr(&sso)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!cid.isValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR && sso && sso->IsValid()) {
		cid.getCamera()->set_object_target(sso->objp, sso->ss->system_info->subobj_num);
	}

	object *objp = cid.getCamera()->get_object_target();
	if(objp == NULL || objp->type != OBJ_SHIP)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	int submodel = cid.getCamera()->get_object_target_submodel();
	if(submodel < 0)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	ship *shipp = &Ships[objp->instance];
	polymodel *pm = model_get(Ship_info[shipp->ship_info_index].model_num);

	if(pm == NULL)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	bsp_info *sm = &pm->submodel[submodel];

	ship_subsys *ss = ship_get_subsys(shipp, sm->name);
	
	if(ss == NULL)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(objp, ss)));
}

ADE_FUNC(setFOV, l_Camera, "[number FOV, number Zoom Time, number Zoom Acceleration Time, number Zoom deceleration Time]",
		 "Sets camera FOV"
		 "<br>FOV is the final field of view, in radians, of the camera."
		 "<br>Zoom Time is the total time to take zooming in or out."
		 "<br>Acceleration Time is the total time it should take the camera to get up to full zoom speed."
		 "<br>Deceleration Time is the total time it should take the camera to slow down from full zoom speed.",
		 "boolean", "true if successful, false or nil otherwise")
{
	camid cid;
	float n_fov = VIEWER_ZOOM_DEFAULT;
	float time=0.0f;
	float acc_time=0.0f;
	float dec_time=0.0f;
	if(!ade_get_args(L, "o|ffff", l_Camera.Get(&cid), &n_fov, &time, &acc_time, &dec_time))
		return ADE_RETURN_NIL;
	
	if(!cid.isValid())
		return ADE_RETURN_NIL;

	cid.getCamera()->set_fov(n_fov, time, acc_time, dec_time);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(setOrientation, l_Camera, "[world orientation Orientation, number Rotation Time, number Acceleration Time, number Deceleration time]",
		"Sets camera orientation and velocity data."
		"<br>Orientation is the final orientation for the camera, after it has finished moving. If not specified, the camera will simply stop at its current orientation."
		"<br>Rotation time (seconds) is how long total, including acceleration, the camera should take to rotate. If it is not specified, the camera will jump to the specified orientation."
		"<br>Acceleration time (seconds) is how long it should take the camera to get 'up to speed'. If not specified, the camera will instantly start moving."
		"<br>Deceleration time (seconds) is how long it should take the camera to slow down. If not specified, the camera will instantly stop moving.",
		"boolean", "true if successful, false or nil otherwise")
{
	camid cid;
	matrix_h *mh=NULL;
	float time=0.0f;
	float acc_time=0.0f;
	float dec_time=0.0f;
	if(!ade_get_args(L, "o|offf", l_Camera.Get(&cid), l_Matrix.GetPtr(&mh), &time, &acc_time, &dec_time))
		return ADE_RETURN_NIL;
	
	if(!cid.isValid())
		return ADE_RETURN_NIL;

	camera *cam = cid.getCamera();

	if(mh != NULL)
	{
		cam->set_rotation(mh->GetMatrix(), time, acc_time, dec_time);
	}
	else
	{
		cam->set_rotation();
	}

	return ADE_RETURN_TRUE;
}

ADE_FUNC(setPosition, l_Camera, "[wvector Position, number Translation Time, number Acceleration Time, number Deceleration Time]",
		"Sets camera position and velocity data."
		"<br>Position is the final position for the camera. If not specified, the camera will simply stop at its current position."
		"<br>Translation time (seconds) is how long total, including acceleration, the camera should take to move. If it is not specified, the camera will jump to the specified position."
		"<br>Acceleration time (seconds) is how long it should take the camera to get 'up to speed'. If not specified, the camera will instantly start moving."
		"<br>Deceleration time (seconds) is how long it should take the camera to slow down. If not specified, the camera will instantly stop moving.",
		"boolean", "true if successful, false or nil otherwise")
{
	camid cid;
	vec3d *pos=NULL;
	float time=0.0f;
	float acc_time=0.0f;
	float dec_time=0.0f;
	if(!ade_get_args(L, "o|offf", l_Camera.Get(&cid), l_Vector.GetPtr(&pos), &time, &acc_time, &dec_time))
		return ADE_RETURN_NIL;
	
	if(!cid.isValid())
		return ADE_RETURN_NIL;

	cid.getCamera()->set_position(pos, time, acc_time, dec_time);

	return ADE_RETURN_TRUE;
}

//**********HANDLE: Control Info
ade_obj<int> l_Control_Info("control info", "control info handle");

ADE_VIRTVAR(Pitch, l_Control_Info, "number", "Pitch of the player ship", "number", "Pitch")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci))
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.pitch = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.pitch);
}

ADE_VIRTVAR(Heading, l_Control_Info, "number", "Heading of the player ship", "number", "Heading")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci))
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.heading = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.heading);
}

ADE_VIRTVAR(Bank, l_Control_Info, "number", "Bank of the player ship", "number", "Bank")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci))
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.bank = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.bank);
}

ADE_VIRTVAR(Vertical, l_Control_Info, "number", "Vertical control of the player ship", "number", "Vertical control")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci))
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.vertical = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.vertical);
}

ADE_VIRTVAR(Sideways, l_Control_Info, "number", "Sideways control of the player ship", "number", "Sideways control")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci))
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.sideways = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.sideways);
}

ADE_VIRTVAR(Forward, l_Control_Info, "number", "Forward control of the player ship", "number", "Forward")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci))
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.forward = new_ci;
	}

	return ade_set_args(L, "f", Player->lua_ci.forward);
}

ADE_VIRTVAR(ForwardCruise, l_Control_Info, "number", "Forward control of the player ship", "number", "Forward")
{
	int idx;
	float new_ci = 0.0f;

	if(!ade_get_args(L, "o|f", l_Control_Info.Get(&idx), &new_ci))
		return ade_set_error(L, "f", new_ci);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.forward_cruise_percent = new_ci*100.0f;
	}

	return ade_set_args(L, "f", Player->lua_ci.forward_cruise_percent*0.01f);
}

ADE_VIRTVAR(PrimaryCount, l_Control_Info, "number", "Number of primary weapons that will fire", "number", "Number of weapons to fire, or 0 if handle is invalid")
{
	int idx;
	int new_pri = 0;

	if(!ade_get_args(L, "o|i", l_Control_Info.Get(&idx), &new_pri))
		return ade_set_error(L, "i", new_pri);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.fire_primary_count = new_pri;
	}

	return ade_set_args(L, "i", Player->lua_ci.fire_primary_count);
}

ADE_VIRTVAR(SecondaryCount, l_Control_Info, "number", "Number of secondary weapons that will fire", "number", "Number of weapons to fire, or 0 if handle is invalid")
{
	int idx;
	int new_sec = 0;

	if(!ade_get_args(L, "o|i", l_Control_Info.Get(&idx), &new_sec))
		return ade_set_error(L, "i", new_sec);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.fire_secondary_count = new_sec;
	}

	return ade_set_args(L, "i", Player->lua_ci.fire_secondary_count);
}

ADE_VIRTVAR(CountermeasureCount, l_Control_Info, "number", "Number of countermeasures that will launch", "number", "Number of countermeasures to launch, or 0 if handle is invalid")
{
	int idx;
	int new_cm = 0;

	if(!ade_get_args(L, "o|i", l_Control_Info.Get(&idx), &new_cm))
		return ade_set_error(L, "i", new_cm);

	if(ADE_SETTING_VAR) {
		Player->lua_ci.fire_countermeasure_count = new_cm;
	}

	return ade_set_args(L, "i", Player->lua_ci.fire_countermeasure_count);
}

ADE_FUNC(clearLuaButtonInfo, l_Control_Info, NULL, "Clears the lua button control info", NULL, NULL)
{
	button_info_clear(&Player->lua_bi);

	return ADE_RETURN_NIL;
}

ADE_FUNC(getButtonInfo, l_Control_Info, NULL, "Access the four bitfields containing the button info", "number, number, number,number", "Four bitfields")
{
	int i;
	int bi_status[4];

	for(i=0;i<4;i++)
		bi_status[i] = Player->lua_bi.status[i];

	return ade_set_args(L, "iiii", bi_status[0], bi_status[1], bi_status[2], bi_status[3]);
}

ADE_FUNC(accessButtonInfo, l_Control_Info, "number, number, number, number", "Access the four bitfields containing the button info", "number, number, number,number", "Four bitfields")
{
	int i;
	int bi_status[4];
	
	for(i=0;i<4;i++)
		bi_status[i] = 0;

	if(!ade_get_args(L, "|iiii", &bi_status[0], &bi_status[1], &bi_status[2], &bi_status[3]))
		return ADE_RETURN_NIL;

	if(ADE_SETTING_VAR) {
		for(i=0;i<4;i++)
			Player->lua_bi.status[i] = bi_status[i];
	}

	for(i=0;i<4;i++)
		bi_status[i] = Player->lua_bi.status[i];

	return ade_set_args(L, "iiii", bi_status[0], bi_status[1], bi_status[2], bi_status[3]);
}

ADE_FUNC(useButtonControl, l_Control_Info, "number, string", "Adds the defined button control to lua button control data, if number is -1 it tries to use the string", NULL, NULL)
{
	int index;
	char *buf = NULL;

	if(!ade_get_args(L, "i|s", &index, &buf))
		return ADE_RETURN_NIL;

	if(index != -1) {
		// Process the number
		if (index > (4 * 32))
			return ADE_RETURN_NIL;

		int a, b;
		a = index / 32;
		b = index % 32;

		// now add the processed bit
		Player->lua_bi.status[a] |= (1<<b);
	} else if (buf != NULL) {
		int i;
		for(i=0; i<num_plr_commands; i++) {
			if(!(strcmp(buf, plr_commands[i].name))) {
				int a;
				a = plr_commands[i].def / 32;
				Player->lua_bi.status[plr_commands[i].var] |= (1<<a);
				break;
			}
		}
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(getButtonControlName, l_Control_Info, "number", "Gives the name of the command corresponding with the given number", "string", "Name of the command")
{
	int index;

	if(!ade_get_args(L, "i", &index))
		return ade_set_error(L, "s", "");

	if((index < 0) || (index > num_plr_commands))
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", plr_commands[index].name);
}

ADE_FUNC(getButtonControlNumber, l_Control_Info, "string", "Gives the number of the command corresponding with the given string", "number", "Number of the command")
{
	int i;
	char *buf;

	if(!ade_get_args(L, "s", &buf))
		return ade_set_error(L, "i", -1);

	for(i = 0; i < num_plr_commands; i++) {
		if (!(strcmp(buf, plr_commands[i].name))) {
			return ade_set_args(L, "i", plr_commands[i].def);
		}
	}

	return ade_set_error(L, "i", -1);
}

ADE_VIRTVAR(AllButtonPolling, l_Control_Info, "boolean", "Toggles the all button polling for lua", "boolean", "If the all button polling is enabled or not")
{
	bool p;
	int idx;

	if(!ade_get_args(L, "o|b", l_Control_Info.Get(&idx), &p))
		return ADE_RETURN_FALSE;

	if (ADE_SETTING_VAR) {
		if (p)
			lua_game_control |= LGC_B_POLL_ALL;
		else
			lua_game_control &= ~LGC_B_POLL_ALL;
	}

	if (lua_game_control & LGC_B_POLL_ALL)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(pollAllButtons, l_Control_Info, NULL, "Access the four bitfields containing the button info", "number, number, number,number", "Four bitfields")
{
	int i;
	int bi_status[4];

	if(!(lua_game_control & LGC_B_POLL_ALL))
		return ADE_RETURN_NIL;

	for(i=0;i<4;i++)
		bi_status[i] = Player->lua_bi_full.status[i];

	return ade_set_args(L, "iiii", bi_status[0], bi_status[1], bi_status[2], bi_status[3]);
}

class particle_h
{
protected:
	particle::WeakParticlePtr part;
public:
	particle_h()
	{
        part = particle::WeakParticlePtr();
	}

    particle_h(particle::WeakParticlePtr part_p)
	{
		this->part = part_p;
	}

	particle::WeakParticlePtr Get()
	{
		return this->part;
	}

	bool isValid()
	{
		return !part.expired();
	}

	~particle_h()
	{
	}
};

//**********HANDLE: Particle
ade_obj<particle_h> l_Particle("particle", "Handle to a particle");

ADE_VIRTVAR(Position, l_Particle, "vector", "The current position of the particle (world vector)", "vector", "The current position")
{
	particle_h *ph = NULL;
	vec3d newVec = vmd_zero_vector;
	if (!ade_get_args(L, "o|o", l_Particle.GetPtr(&ph), l_Vector.Get(&newVec)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (ph == NULL)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!ph->isValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (ADE_SETTING_VAR)
	{
		ph->Get().lock()->pos = newVec;
	}

	return ade_set_args(L, "o", l_Vector.Set(ph->Get().lock()->pos));
}

ADE_VIRTVAR(Velocity, l_Particle, "vector", "The current velocity of the particle (world vector)", "vector", "The current velocity")
{
	particle_h *ph = NULL;
	vec3d newVec = vmd_zero_vector;
	if (!ade_get_args(L, "o|o", l_Particle.GetPtr(&ph), l_Vector.Get(&newVec)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	
	if (ph == NULL)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!ph->isValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (ADE_SETTING_VAR)
	{
		ph->Get().lock()->velocity = newVec;
	}

	return ade_set_args(L, "o", l_Vector.Set(ph->Get().lock()->velocity));
}

ADE_VIRTVAR(Age, l_Particle, "number", "The time this particle already lives", "number", "The current age or -1 on error")
{
	particle_h *ph = NULL;
	float newAge = -1.0f;
	if (!ade_get_args(L, "o|f", l_Particle.GetPtr(&ph), &newAge))
		return ade_set_error(L, "f", -1.0f);
	
	if (ph == NULL)
		return ade_set_error(L, "f", -1.0f);

	if (!ph->isValid())
		return ade_set_error(L, "f", -1.0f);

	if (ADE_SETTING_VAR)
	{
		if (newAge >= 0)
			ph->Get().lock()->age = newAge;
	}

	return ade_set_args(L, "f", ph->Get().lock()->age);
}

ADE_VIRTVAR(MaximumLife, l_Particle, "number", "The time this particle can live", "number", "The maximal life or -1 on error")
{
	particle_h *ph = NULL;
	float newLife = -1.0f;
	if (!ade_get_args(L, "o|f", l_Particle.GetPtr(&ph), &newLife))
		return ade_set_error(L, "f", -1.0f);
	
	if (ph == NULL)
		return ade_set_error(L, "f", -1.0f);

	if (!ph->isValid())
		return ade_set_error(L, "f", -1.0f);

	if (ADE_SETTING_VAR)
	{
		if (newLife >= 0)
			ph->Get().lock()->max_life = newLife;
	}

	return ade_set_args(L, "f", ph->Get().lock()->max_life);
}

ADE_VIRTVAR(Radius, l_Particle, "number", "The radius of the particle", "number", "The radius or -1 on error")
{
	particle_h *ph = NULL;
	float newRadius = -1.0f;
	if (!ade_get_args(L, "o|f", l_Particle.GetPtr(&ph), &newRadius))
		return ade_set_error(L, "f", -1.0f);
	
	if (ph == NULL)
		return ade_set_error(L, "f", -1.0f);

	if (!ph->isValid())
		return ade_set_error(L, "f", -1.0f);

	if (ADE_SETTING_VAR)
	{
		if (newRadius >= 0)
			ph->Get().lock()->radius = newRadius;
	}

	return ade_set_args(L, "f", ph->Get().lock()->radius);
}

ADE_VIRTVAR(TracerLength, l_Particle, "number", "The tracer legth of the particle", "number", "The radius or -1 on error")
{
	particle_h *ph = NULL;
	float newTracer = -1.0f;
	if (!ade_get_args(L, "o|f", l_Particle.GetPtr(&ph), &newTracer))
		return ade_set_error(L, "f", -1.0f);
	
	if (ph == NULL)
		return ade_set_error(L, "f", -1.0f);

	if (!ph->isValid())
		return ade_set_error(L, "f", -1.0f);

	// tracer_length has been deprecated
	return ade_set_args(L, "f", -1.0f);
}

ADE_VIRTVAR(AttachedObject, l_Particle, "object", "The object this particle is attached to. If valid the position will be relativ to this object and the velocity will be ignored.", "object", "Attached object or invalid object handle on error")
{
	particle_h *ph = NULL;
	object_h *newObj = nullptr;
	if (!ade_get_args(L, "o|o", l_Particle.GetPtr(&ph), l_Object.GetPtr(&newObj)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));
	
	if (ph == NULL)
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if (!ph->isValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if (ADE_SETTING_VAR)
	{
		if (newObj->IsValid())
			ph->Get().lock()->attached_objnum = newObj->objp->signature;
	}

	return ade_set_args(L, "o", l_Object.Set(object_h(&Objects[ph->Get().lock()->attached_objnum])));
}

ADE_FUNC(isValid, l_Particle, NULL, "Detects whether this handle is valid", "boolean", "true if valid false if not")
{
	particle_h *ph = NULL;
	if (!ade_get_args(L, "o", l_Particle.GetPtr(&ph)))
		return ADE_RETURN_FALSE;
	
	if (ph == NULL)
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", ph->isValid());
}

//**********LIBRARY: Audio
ade_lib l_Audio("Audio", NULL, "ad", "Sound/Music Library");

ADE_FUNC(getSoundentry, l_Audio, "string/number", "Return a sound entry matching the specified index or name. If you are using a number then the first valid index is 1", "soundentry", "soundentry or invalid handle on error")
{
	int index = -1;

	if (lua_isnumber(L, 1))
	{
		int idx = -1;
		if(!ade_get_args(L, "i", &idx))
			return ade_set_error(L, "o", l_SoundEntry.Set(sound_entry_h()));
		
		index = gamesnd_get_by_tbl_index(idx);
	}
	else
	{
		char *s = NULL;
		if(!ade_get_args(L, "s", &s))
			return ade_set_error(L, "o", l_SoundEntry.Set(sound_entry_h()));

		if (s == NULL)
			return ade_set_error(L, "o", l_SoundEntry.Set(sound_entry_h()));

		index = gamesnd_get_by_name(s);
	}

	if (index < 0)
	{
		return ade_set_args(L, "o", l_SoundEntry.Set(sound_entry_h()));
	}
	else
	{
		return ade_set_args(L, "o", l_SoundEntry.Set(sound_entry_h(index)));
	}
}

ADE_FUNC(loadSoundfile, l_Audio, "string filename", "Loads the specified sound file", "soundfile", "A soundfile handle")
{
	char* fileName = NULL;

	if (!ade_get_args(L, "s", &fileName))
		return ade_set_error(L, "o", l_Soundfile.Set(-1));

	game_snd tmp_gs;
	strcpy_s( tmp_gs.filename, fileName );
	int n = snd_load( &tmp_gs, 0 );

	return ade_set_error(L, "o", l_Soundfile.Set(n));
}

ADE_FUNC(playSound, l_Audio, "soundentry", "Plays the specified sound entry handle", "sound", "A handle to the playing sound")
{
	sound_entry_h *seh = NULL;

	if (!ade_get_args(L, "o", l_SoundEntry.GetPtr(&seh)))
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	int handle = snd_play(seh->Get());

	if (handle < 0)
	{
		return ade_set_args(L, "o", l_Sound.Set(sound_h()));
	}
	else
	{
		return ade_set_args(L, "o", l_Sound.Set(sound_h(seh->idx, handle)));
	}
}

ADE_FUNC(playLoopingSound, l_Audio, "soundentry", "Plays the specified sound as a looping sound", "sound", "A handle to the playing sound or invalid handle if playback failed")
{
	sound_entry_h *seh = NULL;

	if (!ade_get_args(L, "o", l_SoundEntry.GetPtr(&seh)))
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	int handle = snd_play_looping(seh->Get());

	if (handle < 0)
	{
		return ade_set_args(L, "o", l_Sound.Set(sound_h()));
	}
	else
	{
		return ade_set_args(L, "o", l_Sound.Set(sound_h(seh->idx, handle)));
	}
}

ADE_FUNC(play3DSound, l_Audio, "soundentry[, vector source[, vector listener]]", "Plays the specified sound entry handle. Source if by default 0, 0, 0 and listener is by default the current viewposition", "3Dsound", "A handle to the playing sound")
{
	sound_entry_h *seh = NULL;
	vec3d *source = &vmd_zero_vector;
	vec3d *listener = &View_position;

	if (!ade_get_args(L, "o|oo", l_SoundEntry.GetPtr(&seh), l_Vector.GetPtr(&source), l_Vector.GetPtr(&listener)))
		return ade_set_error(L, "o", l_Sound3D.Set(sound_h()));

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "o", l_Sound3D.Set(sound_h()));

	int handle = snd_play_3d(seh->Get(), source, listener);

	if (handle < 0)
	{
		return ade_set_args(L, "o", l_Sound3D.Set(sound_h()));
	}
	else
	{
		return ade_set_args(L, "o", l_Sound3D.Set(sound_h(seh->idx, handle)));
	}
}

ADE_FUNC(playGameSound, l_Audio, "Sound index, [Panning (-1.0 left to 1.0 right), Volume %, Priority 0-3, Voice Message?]", "Plays a sound from #Game Sounds in sounds.tbl. A priority of 0 indicates that the song must play; 1-3 will specify the maximum number of that sound that can be played", "boolean", "True if sound was played, false if not (Replaced with a sound instance object in the future)")
{
	int idx, gamesnd_idx;
	float pan=0.0f;
	float vol=100.0f;
	int pri=0;
	bool voice_msg = false;
	if(!ade_get_args(L, "i|ffib", &idx, &pan, &vol, &pri, &voice_msg))
		return ADE_RETURN_NIL;
	
	if(idx < 0)
		return ADE_RETURN_FALSE;

	if(pri < 0 || pri > 3)
		pri = 0;

    CLAMP(pan, -1.0f, 1.0f);
    CLAMP(vol, 0.0f, 100.0f);

	gamesnd_idx = gamesnd_get_by_tbl_index(idx);

	if (gamesnd_idx >= 0) {
		int sound_handle = snd_play(&Snds[gamesnd_idx], pan, vol*0.01f, pri, voice_msg);
		return ade_set_args(L, "b", sound_handle >= 0);
	} else {
		LuaError(L, "Invalid sound index %i (Snds[%i]) in playGameSound()", idx, gamesnd_idx);
		return ADE_RETURN_FALSE;
	}
}

ADE_FUNC(playInterfaceSound, l_Audio, "Sound index", "Plays a sound from #Interface Sounds in sounds.tbl", "boolean", "True if sound was played, false if not")
{
	int idx, gamesnd_idx;
	if(!ade_get_args(L, "i", &idx))
		return ade_set_error(L, "b", false);

	gamesnd_idx = gamesnd_get_by_iface_tbl_index(idx);

	if (gamesnd_idx >= 0) {
		gamesnd_play_iface(gamesnd_idx);
		return ade_set_args(L, "b", true);
	} else {
		LuaError(L, "Invalid sound index %i (Snds[%i]) in playInterfaceSound()", idx, gamesnd_idx);
		return ADE_RETURN_FALSE;
	}
}

extern float Master_event_music_volume;
ADE_FUNC(playMusic, l_Audio, "string Filename, [float volume = 1.0, bool looping = true]", "Plays a music file using FS2Open's builtin music system. Volume is currently ignored, uses players music volume setting. Files passed to this function are looped by default.", "number", "Audiohandle of the created audiostream, or -1 on failure")
{
	char *s;
	float volume = 1.0f;
	bool loop = true;
	if (!ade_get_args(L, "s|fb", &s, &volume, &loop))
		return ade_set_error(L, "i", -1);

	int ah = audiostream_open(s, ASF_MENUMUSIC);
	if(ah < 0)
		return ade_set_error(L, "i", -1);

	// didn't remove the volume parameter because it'll break the API
	volume = Master_event_music_volume;

	audiostream_play(ah, volume, loop ? 1 : 0);
	return ade_set_args(L, "i", ah);
}

ADE_FUNC(stopMusic, l_Audio, "int audiohandle, [bool fade = false], [string 'briefing|credits|mainhall']", "Stops a playing music file, provided audiohandle is valid. If the 3rd arg is set to one of briefing,credits,mainhall then that music will be stopped despite the audiohandle given.", NULL, NULL)
{
	int ah;
	bool fade = false;
	char *music_type = NULL;

	if(!ade_get_args(L, "i|bs", &ah, &fade, &music_type))
		return ADE_RETURN_NIL;

	if (ah >= MAX_AUDIO_STREAMS || ah < 0 )
		return ADE_RETURN_NIL;

	if (music_type == NULL) {
		audiostream_close_file(ah, fade);
	} else {
		if (!stricmp(music_type, "briefing"))	{
			briefing_stop_music(fade);
		} else if (!stricmp(music_type, "credits")) {
			credits_stop_music(fade);
		} else if (!stricmp(music_type, "mainhall")) {
			main_hall_stop_music(fade);
		} else {
			LuaError(L, "Invalid music type (%s) passed to stopMusic", music_type);
		}
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(pauseMusic, l_Audio, "int audiohandle, bool pause", "Pauses or unpauses a playing music file, provided audiohandle is valid. The boolean argument should be true to pause and false to unpause. If the audiohandle is -1, *all* audio streams are paused or unpaused.", NULL, NULL)
{
	int ah;
	bool pause;

	if(!ade_get_args(L, "ib", &ah, &pause))
		return ADE_RETURN_NIL;

	if (ah >= 0 && ah < MAX_AUDIO_STREAMS)
	{
		if (pause)
			audiostream_pause(ah, true);
		else
			audiostream_unpause(ah, true);
	}
	else if (ah == -1)
	{
		if (pause)
			audiostream_pause_all(true);
		else
			audiostream_unpause_all(true);
	}

		return ADE_RETURN_NIL;
}


//**********LIBRARY: Base
ade_lib l_Base("Base", NULL, "ba", "Base FreeSpace 2 functions");

ADE_FUNC(print, l_Base, "string Message", "Prints a string", NULL, NULL)
{
	mprintf(("%s", lua_tostring(L, -1)));

	return ADE_RETURN_NIL;
}

ADE_FUNC(warning, l_Base, "string Message", "Displays a FreeSpace warning (debug build-only) message with the string provided", NULL, NULL)
{
	Warning(LOCATION, "%s", lua_tostring(L, -1));

	return ADE_RETURN_NIL;
}

ADE_FUNC(error, l_Base, "string Message", "Displays a FreeSpace error message with the string provided", NULL, NULL)
{
	Error(LOCATION, "%s", lua_tostring(L, -1));

	return ADE_RETURN_NIL;
}

ADE_FUNC(createOrientation, l_Base, "[p/r1c1, b/r1c2, h/r1c3, r2c1, r2c2, r2c3, r3c1, r3c2, r3c3]", "Given 0, 3, or 9 arguments, creates an orientation object with that orientation.", "orientation", "New orientation object, or null orientation on failure")
{
	matrix m;
	int numargs = ade_get_args(L, "|fffffffff", &m.a1d[0], &m.a1d[1], &m.a1d[2], &m.a1d[3], &m.a1d[4], &m.a1d[5], &m.a1d[6], &m.a1d[7], &m.a1d[8]);
	if(!numargs)
	{
		return ade_set_args(L, "o", l_Matrix.Set( matrix_h(&vmd_identity_matrix) ));
	}
	else if(numargs == 3)
	{
		angles a = {m.a1d[0], m.a1d[1], m.a1d[2]};
		return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&a)));
	}
	else if(numargs == 9)
	{
		return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&m)));
	}

	return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));
}

ADE_FUNC(createVector, l_Base, "[x, y, z]", "Creates a vector object", "vector", "Vector object")
{
	vec3d v3 = vmd_zero_vector;
	ade_get_args(L, "|fff", &v3.xyz.x, &v3.xyz.y, &v3.xyz.z);

	return ade_set_args(L, "o", l_Vector.Set(v3));
}

ADE_FUNC(getFrametimeOverall, l_Base, NULL, "The overall frame time in seconds since the engine has started", "number", "Overall time (seconds)")
{
	return ade_set_args(L, "x", game_get_overall_frametime());
}

ADE_FUNC(getFrametime, l_Base, "[Do not adjust for time compression (Boolean)]", "Gets how long this frame is calculated to take. Use it to for animations, physics, etc to make incremental changes.", "number", "Frame time (seconds)")
{
	bool b=false;
	ade_get_args(L, "|b", &b);

	return ade_set_args(L, "f", b ? flRealframetime : flFrametime);
}

ADE_FUNC(getCurrentGameState, l_Base, "[Depth (number)]", "Gets current FreeSpace state; if a depth is specified, the state at that depth is returned. (IE at the in-game options game, a depth of 1 would give you the game state, while the function defaults to 0, which would be the options screen.", "gamestate", "Current game state at specified depth, or invalid handle if no game state is active yet")
{
	int depth = 0;
	ade_get_args(L, "|i", &depth);

	if(depth > gameseq_get_depth())
		return ade_set_args(L, "o", l_GameState.Set(gamestate_h()));

	return ade_set_args(L, "o", l_GameState.Set(gamestate_h(gameseq_get_state(depth))));
}

ADE_FUNC(getCurrentMPStatus, l_Base, "NIL", "Gets this computers current MP status", "string", "Current MP status" )
{
	if ( MULTIPLAYER_MASTER )
		return ade_set_args(L, "s", "MULTIPLAYER_MASTER");

	if ( MULTIPLAYER_HOST )
		return ade_set_args(L, "s", "MULTIPLAYER_HOST");

	if ( MULTIPLAYER_CLIENT )
		return ade_set_args(L, "s", "MULTIPLAYER_CLIENT");

	if ( MULTIPLAYER_STANDALONE )
		return ade_set_args(L, "s", "MULTIPLAYER_STANDALONE");

	return ade_set_args(L, "s", "SINGLEPLAYER");
}

ADE_FUNC(getCurrentPlayer, l_Base, NULL, "Gets a handle of the currently used player.<br><b>Note:</b> If there is no current player then the first player will be returned, check the game state to make sure you have a valid player handle.", "player", "Player handle")
{
	return ade_set_args(L, "o", l_Player.Set(Player_num));
}

ADE_FUNC(setControlMode, l_Base, "NIL or enumeration LE_*_CONTROL", "Sets the current control mode for the game.", "string", "Current control mode")
{
	enum_h *e = NULL;
	if (!(ade_get_args(L, "|o", l_Enum.GetPtr(&e)))) {
		if (lua_game_control & LGC_NORMAL)
			return ade_set_args(L, "s", "NORMAL");
		else if (lua_game_control & LGC_STEERING)
			return ade_set_args(L, "s", "STEERING");
		else if (lua_game_control & LGC_FULL)
			return ade_set_args(L, "s", "FULL");
		else
			return ade_set_error(L, "s", "");
	}

	if (!e) {
		return ade_set_error(L, "s", "");
	}

	switch (e->index) {
		case LE_NORMAL_CONTROLS:
			lua_game_control |= LGC_NORMAL;
			lua_game_control &= ~(LGC_STEERING|LGC_FULL);
			return ade_set_args(L, "s", "NORMAL CONTROLS");
		case LE_LUA_STEERING_CONTROLS:
			lua_game_control |= LGC_STEERING;
			lua_game_control &= ~(LGC_NORMAL|LGC_FULL);
			return ade_set_args(L, "s", "LUA STEERING CONTROLS");
		case LE_LUA_FULL_CONTROLS:
			lua_game_control |= LGC_FULL;
			lua_game_control &= ~(LGC_STEERING|LGC_NORMAL);
			return ade_set_args(L, "s", "LUA FULL CONTROLS");
		default:
			return ade_set_error(L, "s", "");
	}
}

ADE_FUNC(setButtonControlMode, l_Base, "NIL or enumeration LE_*_BUTTON_CONTROL", "Sets the current control mode for the game.", "string", "Current control mode")
{
	enum_h *e = NULL;
	if (!(ade_get_args(L, "|o", l_Enum.GetPtr(&e)))) {
		if (lua_game_control & LGC_B_NORMAL)
			return ade_set_args(L, "s", "NORMAL");
		else if (lua_game_control & LGC_B_OVERRIDE)
			return ade_set_args(L, "s", "OVERRIDE");
		else if (lua_game_control & LGC_B_ADDITIVE)
			return ade_set_args(L, "s", "ADDITIVE");
		else
			return ade_set_error(L, "s", "");
	}

	if (!e) {
		return ade_set_error(L, "s", "");
	}

	switch (e->index) {
		case LE_NORMAL_BUTTON_CONTROLS:
			lua_game_control |= LGC_B_NORMAL;
			lua_game_control &= ~(LGC_B_ADDITIVE|LGC_B_OVERRIDE);
			return ade_set_args(L, "s", "NORMAL BUTTON CONTROL");
		case LE_LUA_ADDITIVE_BUTTON_CONTROL:
			lua_game_control |= LGC_B_ADDITIVE;
			lua_game_control &= ~(LGC_B_NORMAL|LGC_B_OVERRIDE);
			return ade_set_args(L, "s", "LUA OVERRIDE BUTTON CONTROL");
		case LE_LUA_OVERRIDE_BUTTON_CONTROL:
			lua_game_control |= LGC_B_OVERRIDE;
			lua_game_control &= ~(LGC_B_ADDITIVE|LGC_B_NORMAL);
			return ade_set_args(L, "s", "LUA ADDITIVE BUTTON CONTROL");
		default:
			return ade_set_error(L, "s", "");
	}
}

ADE_FUNC(getControlInfo, l_Base, NULL, "Gets the control info handle.", "control info", "control info handle")
{
	return ade_set_args(L, "o", l_Control_Info.Set(1));
}

ADE_FUNC(setTips, l_Base, "True or false", "Sets whether to display tips of the day the next time the current pilot enters the mainhall.", NULL, NULL)
{
	if (Player == NULL)
		return ADE_RETURN_NIL;

	bool tips = false;

	ade_get_args(L, "b", &tips);

	if (tips)
		Player->tips = 1;
	else
		Player->tips = 0;

	return ADE_RETURN_NIL;
}

ADE_FUNC(getGameDifficulty, l_Base, NULL, "Returns the difficulty level from 1-5, 1 being the lowest, (Very Easy) and 5 being the highest (Insane)", "integer", "Difficulty level as integer")
{
	return ade_set_args(L, "i", Game_skill_level+1);
}

ADE_FUNC(postGameEvent, l_Base, "gameevent Event", "Sets current game event. Note that you can crash FreeSpace 2 by posting an event at an improper time, so test extensively if you use it.", "boolean", "True if event was posted, false if passed event was invalid")
{
	gameevent_h *gh = NULL;
	if(!ade_get_args(L, "o", l_GameEvent.GetPtr(&gh)))
		return ade_set_error(L, "b", false);

	if(!gh->IsValid())
		return ade_set_error(L, "b", false);

	if (Om_tracker_flag) 
		Multi_options_g.protocol = NET_TCP;
	psnet_use_protocol(Multi_options_g.protocol);
	
	gameseq_post_event(gh->Get());

	return ADE_RETURN_TRUE;
}

//**********SUBLIBRARY: Base/Events
ade_lib l_Base_Events("GameEvents", &l_Base, NULL, "Freespace 2 game events");

ADE_INDEXER(l_Base_Events, "number Index/string Name", "Array of game events", "gameevent", "Game event, or invalid gameevent handle if index is invalid")
{
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_GameEvent.Set(gameevent_h()));

	int idx = gameseq_get_event_idx(name);

	if(idx < 0)
	{
		idx = atoi(name);

		//Lua-->FS2
		idx--;

		if(idx < 0 || idx >= Num_gs_event_text)
			return ade_set_error(L, "o", l_GameEvent.Set(gameevent_h()));
	}

	return ade_set_args(L, "o", l_GameEvent.Set(gameevent_h(idx)));
}

ADE_FUNC(__len, l_Base_Events, NULL, "Number of events", "number", "Number of events")
{
	return ade_set_args(L, "i", Num_gs_event_text);
}

//**********SUBLIBRARY: Base/States
ade_lib l_Base_States("GameStates", &l_Base, NULL, "Freespace 2 states");

ADE_INDEXER(l_Base_States, "number Index/string Name", "Array of game states", "gamestate", "Game state, or invalid gamestate handle if index is invalid")
{
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_GameState.Set(gamestate_h()));

	int idx = gameseq_get_state_idx(name);

	if(idx < 0)
	{
		idx = atoi(name);

		//Lua-->FS2
		idx--;

		if(idx < 0 || idx >= Num_gs_state_text)
			return ade_set_error(L, "o", l_GameState.Set(gamestate_h()));
	}

	return ade_set_args(L, "o", l_GameState.Set(gamestate_h(idx)));
}

ADE_FUNC(__len, l_Base_States, NULL, "Number of states", "number", "Number of states")
{
	return ade_set_args(L, "i", Num_gs_state_text);
}

//**********LIBRARY: CFILE
//WMC - It's on my to-do list! (Well, if I had one anyway)
//WMC - Did it. I had to invent a to-do list first, though.
//Ironically, I never actually put this on it.
ade_lib l_CFile("CFile", NULL, "cf", "CFile FS2 filesystem access");

int l_cf_get_path_id(char* n_path)
{
	size_t i;
	size_t path_len = strlen(n_path);

	char *buf = (char*) vm_malloc((path_len+1) * sizeof(char));
	
	if (!buf) 
		return CF_TYPE_INVALID;
		
	strcpy(buf, n_path);

	//Remove trailing slashes; avoid buffer overflow on 1-char strings
	i = path_len - 1;
	while(i < std::numeric_limits<size_t>::max() && (buf[i] == '\\' || buf[i] == '/'))
		buf[i--] = '\0';

	//Remove leading slashes
	i = 0;
	while(i < path_len && (buf[i] == '\\' || buf[i] == '/'))
		buf[i++] = '\0';

	//Use official DIR_SEPARATOR_CHAR
	for(i = 0; i < path_len; i++)
	{
		if(buf[i] == '\\' || buf[i] == '/')
			buf[i] = DIR_SEPARATOR_CHAR;
	}
	for(i = 0; i < CF_MAX_PATH_TYPES; i++)
	{
		if(Pathtypes[i].path != NULL && !stricmp(buf, Pathtypes[i].path)) {
			vm_free(buf);
			buf = NULL;
			return Pathtypes[i].index;
		}
	}

	vm_free(buf);
	buf = NULL;
	return CF_TYPE_INVALID;
}

ADE_FUNC(deleteFile, l_CFile, "string Filename, string Path", "Deletes given file. Path must be specified. Use a slash for the root directory.", "boolean", "True if deleted, false")
{
	char *n_filename = NULL;
	char *n_path = "";
	if(!ade_get_args(L, "ss", &n_filename, &n_path))
		return ade_set_error(L, "b", false);

	int path = CF_TYPE_INVALID;
	if(n_path != NULL && strlen(n_path))
		path = l_cf_get_path_id(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", cf_delete(n_filename, path) != 0);
}

ADE_FUNC(fileExists, l_CFile, "string Filename, [string Path = \"\", boolean CheckVPs = false]", "Checks if a file exists. Use a blank string for path for any directory, or a slash for the root directory.", "boolean", "True if file exists, false or nil otherwise")
{
	char *n_filename = NULL;
	char *n_path = "";
	bool check_vps = false;
	if(!ade_get_args(L, "s|sb", &n_filename, &n_path, &check_vps))
		return ADE_RETURN_NIL;

	int path = CF_TYPE_ANY;
	if(n_path != NULL && strlen(n_path))
		path = l_cf_get_path_id(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "b", false);

	if(!check_vps)
		return ade_set_args(L, "b", cf_exists(n_filename, path) != 0);
	else
		return ade_set_args(L, "b", cf_exists_full(n_filename, path ) != 0);
}

ADE_FUNC(openFile, l_CFile, "string Filename, [string Mode=\"r\", string Path = \"\"]",
		 "Opens a file. 'Mode' uses standard C fopen arguments. Use a blank string for path for any directory, or a slash for the root directory."
		 "Be EXTREMELY CAREFUL when using this function, as you may PERMANENTLY delete any file by accident",
		 "file",
		 "File handle, or invalid file handle if the specified file couldn't be opened")
{
	char *n_filename = NULL;
	char *n_mode = "r";
	char *n_path = "";
	if(!ade_get_args(L, "s|ss", &n_filename, &n_mode, &n_path))
		return ade_set_error(L, "o", l_File.Set(NULL));

	int type = CFILE_NORMAL;

	int path = CF_TYPE_ANY;
	if(n_path != NULL && strlen(n_path))
		path = l_cf_get_path_id(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "o", l_File.Set(NULL));

	CFILE *cfp = cfopen(n_filename, n_mode, type, path);
	
	if(!cf_is_valid(cfp))
		return ade_set_error(L, "o", l_File.Set(NULL));

	return ade_set_args(L, "o", l_File.Set(cfp));
}

ADE_FUNC(openTempFile, l_CFile, NULL, "Opens a temp file that is automatically deleted when closed", "file", "File handle, or invalid file handle if tempfile couldn't be created")
{
	return ade_set_args(L, "o", l_File.Set(ctmpfile()));
}

ADE_FUNC(renameFile, l_CFile, "string CurrentFilename, string NewFilename, string Path", "Renames given file. Path must be specified. Use a slash for the root directory.", "boolean", "True if file was renamed, otherwise false")
{
	char *n_filename = NULL;
	char *n_new_filename = NULL;
	char *n_path = "";
	if(!ade_get_args(L, "ss|s", &n_filename, &n_new_filename, &n_path))
		return ade_set_error(L, "b", false);

	int path = CF_TYPE_INVALID;
	if(n_path != NULL && strlen(n_path))
		path = l_cf_get_path_id(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", cf_rename(n_filename, n_new_filename, path) != 0);
}

//**********LIBRARY: Controls library
ade_lib l_Mouse("Controls", NULL, "io", "Controls library");

extern int mouse_inited;

ADE_FUNC(getMouseX, l_Mouse, NULL, "Gets Mouse X pos", "number", "Mouse x position, or 0 if mouse is not initialized yet")
{
	if(!mouse_inited)
		return ade_set_error(L, "i", 0);

	int x;

	mouse_get_pos(&x, NULL);

	return ade_set_args(L, "i", x);
}

ADE_FUNC(getMouseY, l_Mouse, NULL, "Gets Mouse Y pos", "number", "Mouse y position, or 0 if mouse is not initialized yet")
{
	if(!mouse_inited)
		return ade_set_error(L, "i", 0);

	int y;

	mouse_get_pos(NULL, &y);

	return ade_set_args(L, "i", y);
}

ADE_FUNC(isMouseButtonDown, l_Mouse, "{MOUSE_*_BUTTON enumeration}, [..., ...]", "Returns whether the specified mouse buttons are up or down", "boolean", "Whether specified mouse buttons are down, or false if mouse is not initialized yet")
{
	if(!mouse_inited)
		return ade_set_error(L, "b", false);

	enum_h *e[3] = {NULL, NULL, NULL};
	ade_get_args(L, "o|oo", l_Enum.GetPtr(&e[0]), l_Enum.GetPtr(&e[1]), l_Enum.GetPtr(&e[2]));	//Like a snake!

	bool rtn = false;
	int check_flags = 0;

	for(int i = 0; i < 3; i++)
	{
		if(e[i] == NULL)
			break;

		if(e[i]->index == LE_MOUSE_LEFT_BUTTON)
			check_flags |= MOUSE_LEFT_BUTTON;
		if(e[i]->index == LE_MOUSE_MIDDLE_BUTTON)
			check_flags |= MOUSE_MIDDLE_BUTTON;
		if(e[i]->index == LE_MOUSE_RIGHT_BUTTON)
			check_flags |= MOUSE_RIGHT_BUTTON;
	}

	if(mouse_down(check_flags))
		rtn = true;

	return ade_set_args(L, "b", rtn);
}

ADE_FUNC(setCursorImage, l_Mouse, "Image filename", "Sets mouse cursor image, and allows you to lock/unlock the image. (A locked cursor may only be changed with the unlock parameter)", "boolean", "true if successful, false otherwise")
{
	using namespace io::mouse;

	if(!mouse_inited || !Gr_inited)
		return ade_set_error(L, "b", false);

	if (Is_standalone)
		return ade_set_error(L, "b", false);

	char *s = NULL;
	enum_h *u = NULL; // This isn't used anymore

	if(!ade_get_args(L, "s|o", &s, l_Enum.GetPtr(&u)))
		return ade_set_error(L, "b", false);

	Cursor* cursor = CursorManager::get()->loadCursor(s);

	if (cursor == NULL)
	{
		return ade_set_error(L, "b", false);
	}

	CursorManager::get()->setCurrentCursor(cursor);
	return ade_set_args(L, "b", true);
}

ADE_FUNC(setCursorHidden, l_Mouse, "boolean hide[, boolean grab]", "Hides the cursor when <i>hide</i> is true, otherwise shows it. <i>grab</i> determines if "
				"the mouse will be restricted to the window. Set this to true when hiding the cursor while in game. By default grab will be true when we are in the game play state, false otherwise.", NULL, NULL)
{
	if(!mouse_inited)
		return ADE_RETURN_NIL;

	bool b = false;
	bool grab = gameseq_get_state() == GS_STATE_GAME_PLAY;
	ade_get_args(L, "b|b", &b, &grab);

	io::mouse::CursorManager::get()->showCursor(!b, grab);

	return ADE_RETURN_NIL;
}

ADE_FUNC(forceMousePosition, l_Mouse, "number, number (coordinates)", "function to force mouse position", "boolean", "if the operation succeeded or not")
{
	if(!mouse_inited)
		return ADE_RETURN_FALSE;

	if(!Gr_inited)
		return ADE_RETURN_FALSE;

	int x, y;
	if (!(ade_get_args(L, "ii", &x, &y)))
		return ADE_RETURN_FALSE;

	if (!((x >= 0) && (x <= gr_screen.max_w)))
		return ADE_RETURN_FALSE;

	if (!((y >= 0) && (y <= gr_screen.max_h)))
		return ADE_RETURN_FALSE;

	mouse_set_pos(x, y);

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(MouseControlStatus, l_Mouse, "boolean", "Gets and sets the retail mouse control status", "boolean", "if the retail mouse is on or off")
{
	if(!mouse_inited)
		return ADE_RETURN_NIL;

	bool newVal = false;
	if (!ade_get_args(L, "*|b", &newVal))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
	{
		if (newVal)
		{
			Use_mouse_to_fly = 1;
		}
		else
		{
			Use_mouse_to_fly = 0;
		}
	}

	if (Use_mouse_to_fly)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(getMouseSensitivity, l_Mouse, NULL, "Gets mouse sensitivity setting", "number", "Mouse sensitivity in range of 0-9")
{
	return ade_set_args(L, "i", Mouse_sensitivity);
}

ADE_FUNC(getJoySensitivity, l_Mouse, NULL, "Gets joystick sensitivity setting", "number", "Joystick sensitivity in range of 0-9")
{
	return ade_set_args(L, "i", Joy_sensitivity);
}

ADE_FUNC(getJoyDeadzone, l_Mouse, NULL, "Gets joystick deadzone setting", "number", "Joystick deadzone in range of 0-9")
{
	return ade_set_args(L, "i", Joy_dead_zone_size / 5);
}

//trackir funcs
ADE_FUNC(updateTrackIR, l_Mouse, NULL, "Updates Tracking Data. Call before using get functions", "boolean", "Checks if trackir is available and updates variables, returns true if successful, otherwise false")
{
	if( !headtracking::isEnabled() )
		return ADE_RETURN_FALSE;

	if (!headtracking::query())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getTrackIRPitch, l_Mouse, NULL, "Gets pitch axis from last update", "number", "Pitch value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args( L, "f", headtracking::getStatus()->pitch);
}

ADE_FUNC(getTrackIRYaw, l_Mouse, NULL, "Gets yaw axis from last update", "number", "Yaw value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->yaw);
}

ADE_FUNC(getTrackIRRoll, l_Mouse, NULL, "Gets roll axis from last update", "number", "Roll value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->roll);
}

ADE_FUNC(getTrackIRX, l_Mouse, NULL, "Gets x position from last update", "number", "X value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->x);
}

ADE_FUNC(getTrackIRY, l_Mouse, NULL, "Gets y position from last update", "number", "Y value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->y);
}

ADE_FUNC(getTrackIRZ, l_Mouse, NULL, "Gets z position from last update", "number", "Z value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled() )
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->z);
}

//**********LIBRARY: HUD library
ade_lib l_HUD("HUD", NULL, "hu", "HUD library");

ADE_VIRTVAR(HUDDrawn, l_HUD, "boolean", "Current HUD draw status", "boolean", "If the HUD is drawn or not")
{
	bool to_draw = false;

	if(!ade_get_args(L, "*|b", &to_draw))
		return ADE_RETURN_NIL;

	if(ADE_SETTING_VAR)
	{
		if (to_draw)
			HUD_draw = 1;
		else
			HUD_draw = 0;
	}

	if (HUD_draw)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(HUDDisabledExceptMessages, l_HUD, "boolean", "Specifies if only the messages gauges of the hud are drawn", "boolean", "true if only the message gauges are drawn, false otherwise")
{
	bool to_draw = false;

	if (!ade_get_args(L, "*|b", &to_draw))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
	{
		hud_disable_except_messages(to_draw);
	}

	if (hud_disabled_except_messages())
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(setHUDGaugeColor, l_HUD, "number (index number of the gauge), [integer red, number green, number blue, number alpha]", "Color used to draw the gauge", "boolean", "If the operation was successful")
{
	int idx = -1; 
	int r = 0;
	int g = 0;
	int b = 0;
	int a = 0;

	if(!ade_get_args(L, "i|iiii", &idx, &r, &g, &b, &a))
		return ADE_RETURN_FALSE;

	if ((idx < 0) || (idx >= NUM_HUD_GAUGES))
		return ADE_RETURN_FALSE;

	gr_init_alphacolor(&HUD_config.clr[idx], r, g, b, a);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getHUDGaugeColor, l_HUD, "number (index number of the gauge)", "Color used to draw the gauge", "number, number, number, number", "Red, green, blue, and alpha of the gauge")
{
	int idx = -1;

	if(!ade_get_args(L, "i", &idx))
		return ADE_RETURN_NIL;

	if ((idx < 0) || (idx >= NUM_HUD_GAUGES))
		return ADE_RETURN_NIL;

	color c = HUD_config.clr[idx];
	
	return ade_set_args(L, "iiii", (int) c.red, (int) c.green, (int) c.blue, (int) c.alpha);	
}

ADE_FUNC(getHUDGaugeHandle, l_HUD, "string Name", "Returns a handle to a specified HUD gauge", "HudGauge", "HUD Gauge handle, or nil if invalid")
{
	char* name;
	if (!ade_get_args(L, "s", &name))
		return ADE_RETURN_NIL;
	HudGauge* gauge = NULL;

	gauge = hud_get_gauge(name);

	if (gauge == NULL)
		return ADE_RETURN_NIL;
	else
		return ade_set_args(L, "o", l_HudGauge.Set(*gauge));
}

//**********LIBRARY: Graphics
ade_lib l_Graphics("Graphics", NULL, "gr", "Graphics Library");

static float lua_Opacity = 1.0f;
static int lua_Opacity_type = GR_ALPHABLEND_FILTER;

//****SUBLIBRARY: Graphics/Cameras
ade_lib l_Graphics_Cameras("Cameras", &l_Graphics, NULL, "Cameras");

ADE_INDEXER(l_Graphics_Cameras, "number Index/string Name", "Gets camera", "camera", "Ship handle, or invalid ship handle if index was invalid")
{
	char *s = NULL;
	if(!ade_get_args(L, "*s", &s))
		return ade_set_error(L, "o", l_Camera.Set(camid()));

	camid cid = cam_lookup(s);
	if(!cid.isValid())
	{
		int cn = atoi(s);
		if(cn > 0)
		{
			//Lua-->FS2
			cn--;
			cid = cam_get_camera(cn);
		}
	}

	return ade_set_args(L, "o", l_Camera.Set(cid));
}

ADE_FUNC(__len, l_Graphics_Cameras, NULL, "Gets number of cameras", "number", "Number of cameras")
{
	return ade_set_args(L, "i", (int)cam_get_num());
}

//****SUBLIBRARY: Graphics/Fonts
ade_lib l_Graphics_Fonts("Fonts", &l_Graphics, NULL, "Font library");

ADE_FUNC(__len, l_Graphics_Fonts, NULL, "Number of loaded fonts", "number", "Number of loaded fonts")
{
	return ade_set_args(L, "i", font::FontManager::numberOfFonts());
}

ADE_INDEXER(l_Graphics_Fonts, "number Index/string Filename", "Array of loaded fonts", "font", "Font handle, or invalid font handle if index is invalid")
{
	if (lua_isnumber(L, 2))
	{
		int index = -1;

		if (!ade_get_args(L, "*i", &index))
			return ade_set_error(L, "o", l_Font.Set(font_h()));

		auto realIdx = index - 1;

		if (realIdx < 0 || realIdx >= font::FontManager::numberOfFonts())
		{
			LuaError(L, "Invalid font index %d specified, must be between 1 and %d!", index, font::FontManager::numberOfFonts());
		}

		return ade_set_args(L, "o", l_Font.Set(font_h(font::FontManager::getFont(index - 1))));
	}
	else
	{
		char *s = NULL;

		if(!ade_get_args(L, "*s", &s))
			return ade_set_error(L, "o", l_Font.Set(font_h()));

		return ade_set_args(L, "o", l_Font.Set(font_h(font::FontManager::getFont(s))));
	}
}

ADE_VIRTVAR(CurrentFont, l_Graphics, "font", "Current font", "font", NULL)
{
	font_h *newFh = NULL;

	if(!ade_get_args(L, "*|o", l_Font.GetPtr(&newFh)))
		return ade_set_error(L, "o", l_Font.Set(font_h()));

	if(ADE_SETTING_VAR && newFh->isValid()) {
		font::FontManager::setCurrentFont(newFh->Get());
	}
	
	return ade_set_args(L, "o", l_Font.Set(font_h(font::FontManager::getCurrentFont())));
}

//****SUBLIBRARY: Graphics/PostEffects
ade_lib l_Graphics_Posteffects("PostEffects", &l_Graphics, NULL, "Post processing effects");

ADE_INDEXER(l_Graphics_Posteffects, "number index", "Gets the name of the specified post processing index", "string", "post processing name or empty string on error")
{
	int index = -1;
	if(!ade_get_args(L, "*i", &index))
		return ade_set_error(L, "s", "");

	index--; // Lua -> C/C++

	if (index < 0)
		return ade_set_error(L, "s", "");

	SCP_vector<SCP_string> names;
	get_post_process_effect_names(names);
	names.push_back(SCP_string("lightshafts"));

	if (index >= (int) names.size())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", const_cast<char*>(names[index].c_str()));
}

ADE_FUNC(__len, l_Graphics_Posteffects, NULL, "Gets the number or available post processing effects", "number", "number of post processing effects or 0 on error")
{
	SCP_vector<SCP_string> names;
	get_post_process_effect_names(names);

	// Add one for lightshafts
	return ade_set_args(L, "i", ((int) names.size()) + 1);
}

ADE_FUNC(setPostEffect, l_Graphics, "string name, [number value=0]", "Sets the intensity of the specified post processing effect", "boolean", "true when successful, false otherwise")
{
	char* name = NULL;
	int intensity = 0;

	if (!ade_get_args(L, "s|i", &name, &intensity))
		return ADE_RETURN_FALSE;

	if (name == NULL || intensity < 0)
		return ADE_RETURN_FALSE;

	gr_post_process_set_effect(name, intensity);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(resetPostEffects, l_Graphics, NULL, "Resets all post effects to their default values", "boolean", "true if successful, false otherwise")
{
	gr_post_process_set_defaults();

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(CurrentOpacityType, l_Graphics, "enumeration", "Current alpha blending type; uses ALPHABLEND_* enumerations", "enumeration", NULL)
{
	enum_h *alphatype = NULL;

	if(!ade_get_args(L, "*|o", l_Enum.GetPtr(&alphatype)))
		return ADE_RETURN_NIL;

	if(ADE_SETTING_VAR)
	{
		if(alphatype != NULL && alphatype->index == LE_ALPHABLEND_FILTER)
			lua_Opacity_type = GR_ALPHABLEND_FILTER;
		else
			lua_Opacity_type = GR_ALPHABLEND_NONE;
	}

	int rtn;
	switch(lua_Opacity_type)
	{
		case GR_ALPHABLEND_FILTER:
			rtn = LE_ALPHABLEND_FILTER;
			break;
		default:
			rtn = LE_ALPHABLEND_NONE;
	}

	return ade_set_args(L, "o", l_Enum.Set(enum_h(rtn)));
}

ADE_VIRTVAR(CurrentRenderTarget, l_Graphics, "texture", "Current rendering target", "texture", "Current rendering target, or invalid texture handle if screen is render target")
{
	int newtx = -1;

	if(ADE_SETTING_VAR && lua_isnil(L, 2))
	{
		bm_set_render_target(-1);
		return ade_set_args(L, "o", l_Texture.Set(gr_screen.rendering_to_texture));
	}
	else
	{

		if(!ade_get_args(L, "*|o", l_Texture.Get(&newtx)))
			return ade_set_error(L, "o", l_Texture.Set(-1));

		if(ADE_SETTING_VAR) {
			if(newtx > -1 && bm_is_valid(newtx))
				bm_set_render_target(newtx, 0);
			else
				bm_set_render_target(-1);
		}

		return ade_set_args(L, "o", l_Texture.Set(gr_screen.rendering_to_texture));
	}
}

ADE_FUNC(clearScreen, l_Graphics, "[integer red, number green, number blue, number alpha]", "Clears the screen to black, or the color specified.", NULL, NULL)
{
	int r,g,b,a;
	r=g=b=0;
	a=255;
	ade_get_args(L, "|iiii", &r, &g, &b, &a);

	//WMC - Set to valid values
	if(r != 0 || g != 0 || b != 0 || a!= 255)
	{
		CAP(r,0,255);
		CAP(g,0,255);
		CAP(b,0,255);
		CAP(a,0,255);
		gr_set_clear_color(r,g,b);
		gr_screen.current_clear_color.alpha = (ubyte)a;
		gr_clear();
		gr_set_clear_color(0,0,0);

		return ADE_RETURN_NIL;
	}

	gr_clear();
	return ADE_RETURN_NIL;
}

ADE_FUNC(createCamera, l_Graphics,
		 "string Name, [wvector Position, orientation Orientation]",
		 "Creates a new camera using the specified position and orientation (World)",
		 "camera",
		 "Camera handle, or invalid camera handle if camera couldn't be created")
{
	char *s = NULL;
	vec3d *v = &vmd_zero_vector;
	matrix_h *mh = NULL;
	if(!ade_get_args(L, "s|oo", &s, l_Vector.GetPtr(&v), l_Matrix.GetPtr(&mh)))
		return ADE_RETURN_NIL;

	matrix *mtx = &vmd_identity_matrix;
	if(mh != NULL)
		mtx = mh->GetMatrix();
	camid cid = cam_create(s, v, mtx);

	//Set position
	return ade_set_args(L, "o", l_Camera.Set(cid));
}

ADE_FUNC(isMenuStretched, l_Graphics, NULL, "Returns whether the standard interface is stretched", "boolean", "True if stretched, false if aspect ratio is maintained")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", 0);

	return ade_set_args(L, "b", Cmdline_stretch_menu);
}

ADE_FUNC(getScreenWidth, l_Graphics, NULL, "Gets screen width", "number", "Width in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.max_w);
}

ADE_FUNC(getScreenHeight, l_Graphics, NULL, "Gets screen height", "number", "Height in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.max_h);
}

ADE_FUNC(getCenterWidth, l_Graphics, NULL, "Gets width of center monitor (should be used in conjuction with getCenterOffsetX)", "number", "Width of center monitor in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_w);
}

ADE_FUNC(getCenterHeight, l_Graphics, NULL, "Gets height of center monitor (should be used in conjuction with getCenterOffsetY)", "number", "Height of center monitor in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_h);
}

ADE_FUNC(getCenterOffsetX, l_Graphics, NULL, "Gets X offset of center monitor", "number", "X offset of center monitor in pixels")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_offset_x);
}

ADE_FUNC(getCenterOffsetY, l_Graphics, NULL, "Gets Y offset of center monitor", "number", "Y offset of center monitor in pixels")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_offset_y);
}

ADE_FUNC(getCurrentCamera, l_Graphics, "[boolean]", "Gets the current camera handle, if argument is <i>true</i> then it will also return the main camera when no custom camera is in use", "camera", "camera handle or invalid handle on error")
{
	camid current;

	bool rtnMain = false;

	ade_get_args(L, "|b", &rtnMain);

	if (!rtnMain || Viewer_mode & VM_FREECAMERA)
		current = cam_get_current();
	else
		current = Main_camera;

	return ade_set_args(L, "o", l_Camera.Set(current));
}

ADE_FUNC(getVectorFromCoords, l_Graphics,
		 "[number X=center, number Y=center, number Depth, boolean normalize = false]",
		 "Returns a vector through screen coordinates x and y. "
		 "If depth is specified, vector is extended to Depth units into space"
		 "If normalize is true, vector will be normalized.",
		 "vector",
		 "Vector, or zero vector on failure")
{
	int x = gr_screen.max_w/2;
	int y = gr_screen.max_h/2;
	float depth = 0.0f;
	bool normalize = false; 
	ade_get_args(L, "|iifb", &x, &y, &depth, &normalize);

	vec3d pos = vmd_zero_vector;

	bool in_frame = g3_in_frame() > 0;
	if(!in_frame) {
		g3_start_frame(0);

		vec3d cam_pos;
		matrix cam_orient;

		camid cid;
		if (Viewer_mode & VM_FREECAMERA)
			cid = cam_get_current();
		else
			cid = Main_camera;

		camera *cam = cid.getCamera();

		if (cam != NULL) {
			cam->get_info(&cam_pos, &cam_orient);
			g3_set_view_matrix(&cam_pos, &cam_orient, View_zoom);
		} else {
			g3_set_view_matrix(&Eye_position, &Eye_matrix, View_zoom);
		}

		gr_set_proj_matrix( Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}

	g3_point_to_vec(&pos, x, y);

	if(!in_frame) {
		gr_end_view_matrix();
		gr_end_proj_matrix();
		g3_end_frame();
	}

	if(depth)
		vm_vec_scale(&pos, depth);

	if (normalize)
		vm_vec_normalize_quick(&pos);

	vm_vec_add2(&pos, &View_position);

	return ade_set_args(L, "o", l_Vector.Set(pos));
}

ADE_FUNC(setTarget, l_Graphics, "[texture Texture]",
		"If texture is specified, sets current rendering surface to a texture."
		"Otherwise, sets rendering surface back to screen.",
		"boolean",
		"True if successful, false otherwise")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", false);

	int idx = -1;
	ade_get_args(L, "|o", l_Texture.Get(&idx));

	return ade_set_args(L, "b", bm_set_render_target(idx, 0));
}

ADE_FUNC(setCamera, l_Graphics, "[camera handle Camera]", "Sets current camera, or resets camera if none specified", "boolean", "true if successful, false or nil otherwise")
{
	camid cid;
	if(!ade_get_args(L, "|o", l_Camera.Get(&cid)))
	{
		cam_reset_camera();
		return ADE_RETURN_NIL;
	}

	if(!cid.isValid())
		return ADE_RETURN_NIL;

	cam_set_camera(cid);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(setColor, l_Graphics, "integer Red, number Green, number Blue, [integer Alpha]", "Sets 2D drawing color; each color number should be from 0 (darkest) to 255 (brightest)", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int r,g,b,a=255;

	if(!ade_get_args(L, "iii|i", &r, &g, &b, &a))
		return ADE_RETURN_NIL;

	color ac;
	gr_init_alphacolor(&ac,r,g,b,a);
	gr_set_color_fast(&ac);

	return ADE_RETURN_NIL;
}

ADE_FUNC(setLineWidth, l_Graphics, "[number width=1.0]", "Sets the line width for lines. This call might fail if the specified width is not supported by the graphics implementation. Then the width will be the nearest supported value.", "boolean", "true if succeeded, false otherwise")
{
	if(!Gr_inited)
		return ADE_RETURN_FALSE;

	float width = 1.0f;

	ade_get_args(L, "|f", &width);

	if (width <= 0.0f)
	{
		return ADE_RETURN_FALSE;
	}

	gr_set_line_width(width);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawCircle, l_Graphics, "number Radius, number X, number Y, [boolean Filled=true]", "Draws a circle", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y,ra;
	bool fill = true;

	if(!ade_get_args(L, "iii|b", &ra,&x,&y,&fill))
		return ADE_RETURN_NIL;

	if (fill) {
		//WMC - Circle takes...diameter.
		gr_circle(x,y, ra*2, GR_RESIZE_NONE);
	} else {
		gr_unfilled_circle(x,y, ra*2, GR_RESIZE_NONE);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawArc, l_Graphics, "number Radius, number X, number Y, number StartAngle, number EndAngle, [boolean Filled=true]", "Draws an arc", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y;
	float ra,angle_start,angle_end;
	bool fill = true;

	if(!ade_get_args(L, "fiiff|b", &ra,&x,&y,&angle_start,&angle_end,&fill)) {
		return ADE_RETURN_NIL;
	}

	gr_arc(x,y, ra, angle_start, angle_end, fill, GR_RESIZE_NONE);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawCurve, l_Graphics, "number X, number Y, number Radius", "Draws a curve", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y,ra,dir = 0;

	if(!ade_get_args(L, "iii|i", &x,&y,&ra, &dir))
		return ADE_RETURN_NIL;

	//WMC - direction should be settable at a certain point via enumerations.
	//Not gonna deal with it now.
    gr_curve(x, y, ra, dir, GR_RESIZE_NONE);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawGradientLine, l_Graphics, "number X1, number Y1, number X2, number Y2", "Draws a line from (x1,y1) to (x2,y2) with the CurrentColor that steadily fades out", NULL, NULL)
{
	if(!Gr_inited)
		return 0;

	int x1,y1,x2,y2;

	if(!ade_get_args(L, "iiii", &x1, &y1, &x2, &y2))
		return ADE_RETURN_NIL;

	gr_gradient(x1,y1,x2,y2,GR_RESIZE_NONE);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawLine, l_Graphics, "number X1, number Y1, number X2, number Y2", "Draws a line from (x1,y1) to (x2,y2) with CurrentColor", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x1,y1,x2,y2;

	if(!ade_get_args(L, "iiii", &x1, &y1, &x2, &y2))
		return ADE_RETURN_NIL;

	gr_line(x1,y1,x2,y2,GR_RESIZE_NONE);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawPixel, l_Graphics, "number X, number Y", "Sets pixel to CurrentColor", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y;

	if(!ade_get_args(L, "ii", &x, &y))
		return ADE_RETURN_NIL;

	gr_pixel(x,y,GR_RESIZE_NONE);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawPolygon, l_Graphics, "texture Texture, [vector Position={0,0,0}, orientation Orientation=nil, number Width=1.0, number Height=1.0]", "Draws a polygon. May not work properly in hooks other than On Object Render.", NULL, NULL)
{
	int tdx = -1;
	vec3d pos = vmd_zero_vector;
	matrix_h *mh = NULL;
	float width = 1.0f;
	float height = 1.0f;
	if(!ade_get_args(L, "o|ooff", l_Texture.Get(&tdx), l_Vector.Get(&pos), l_Matrix.GetPtr(&mh), &width, &height))
		return ADE_RETURN_NIL;

	if(!bm_is_valid(tdx))
		return ADE_RETURN_FALSE;

	matrix *orip = &vmd_identity_matrix;
	if(mh != NULL)
		orip = mh->GetMatrix();

	//Do 3D stuff
	bool in_frame = g3_in_frame() > 0;
	if(!in_frame)
		g3_start_frame(0);

	//gr_set_bitmap(tdx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL, lua_Opacity);
	//g3_draw_polygon(&pos, orip, width, height, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
	material mat_params;
	material_set_unlit(&mat_params, tdx, lua_Opacity, lua_Opacity_type == GR_ALPHABLEND_FILTER ? true : false, false);
	g3_render_rect_oriented(&mat_params, &pos, orip, width, height);

	if(!in_frame)
		g3_end_frame();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawRectangle, l_Graphics, "number X1, number Y1, number X2, number Y2, [boolean Filled=true]", "Draws a rectangle with CurrentColor", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x1,y1,x2,y2;
	bool f=true;

	if(!ade_get_args(L, "iiii|b", &x1, &y1, &x2, &y2, &f))
		return ADE_RETURN_NIL;

	if(f)
	{
		gr_set_bitmap(0);  // gr_rect will use the last bitmaps info, so set to zero to flush any previous alpha state
		gr_rect(x1, y1, x2-x1, y2-y1, GR_RESIZE_NONE);
	}
	else
	{
		gr_line(x1,y1,x2,y1,GR_RESIZE_NONE);	//Top
		gr_line(x1,y2,x2,y2,GR_RESIZE_NONE); //Bottom
		gr_line(x1,y1,x1,y2,GR_RESIZE_NONE);	//Left
		gr_line(x2,y1,x2,y2,GR_RESIZE_NONE);	//Right
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawSphere, l_Graphics, "[number Radius = 1.0, vector Position]", "Draws a sphere with radius Radius at world vector Position. May not work properly in hooks other than On Object Render.", "boolean", "True if successful, false or nil otherwise")
{
	float rad = 1.0f;
	vec3d pos = vmd_zero_vector;
	ade_get_args(L, "|fo", &rad, l_Vector.Get(&pos));

	bool in_frame = g3_in_frame() > 0;
	if(!in_frame) {
		g3_start_frame(0);

		vec3d cam_pos;
		matrix cam_orient;

		camid cid;
		
		if (Viewer_mode & VM_FREECAMERA)
			cid = cam_get_current();
		else
			cid = Main_camera;

		camera *cam = cid.getCamera();

		if (cam != NULL) {
			cam->get_info(&cam_pos, &cam_orient);
			g3_set_view_matrix(&cam_pos, &cam_orient, View_zoom);
		} else {
			g3_set_view_matrix(&Eye_position, &Eye_matrix, View_zoom);
		}

		gr_set_proj_matrix( Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}

	vertex vtx;
	vtx.world = pos;
	g3_rotate_vertex(&vtx, &pos);
	g3_draw_sphere(&vtx, rad);

	if(!in_frame) {
		gr_end_view_matrix();
		gr_end_proj_matrix();
		g3_end_frame();
	}
	return ADE_RETURN_TRUE;
}

// Aardwolf's test code to render a model, supposed to emulate WMC's gr.drawModel function
ADE_FUNC(drawModel, l_Graphics, "model, position, orientation", "Draws the given model with the specified position and orientation - Use with extreme care, may not work properly in all scripting hooks.", "int", "Zero if successful, otherwise an integer error code")
{
	model_h *mdl = NULL;
	vec3d *v = &vmd_zero_vector;
	matrix_h *mh = NULL;
	if(!ade_get_args(L, "ooo", l_Model.GetPtr(&mdl), l_Vector.GetPtr(&v), l_Matrix.GetPtr(&mh)))
		return ade_set_args(L, "i", 1);

	if(mdl == NULL)
		return ade_set_args(L, "i", 2);

	int model_num = mdl->GetID();
	if(model_num < 0)
		return ade_set_args(L, "i", 3);

	//Handle angles
	matrix *orient = mh->GetMatrix();

	//Clip
	gr_set_clip(0, 0, gr_screen.max_w, gr_screen.max_h, GR_RESIZE_NONE);

	//Handle 3D init stuff
	g3_start_frame(1);

	vec3d cam_pos;
	matrix cam_orient;

	camid cid;
	if (Viewer_mode & VM_FREECAMERA)
		cid = cam_get_current();
	else
		cid = Main_camera;

	camera *cam = cid.getCamera();

	if (cam != NULL) {
		cam->get_info(&cam_pos, &cam_orient);
		g3_set_view_matrix(&cam_pos, &cam_orient, View_zoom);
	} else {
		g3_set_view_matrix(&Eye_position, &Eye_matrix, View_zoom);
	}

	gr_set_proj_matrix( Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	//Draw the ship!!
	model_clear_instance(model_num);
	model_set_detail_level(0);
	model_render_params render_info;

	render_info.set_detail_level_lock(0);

	model_render_immediate(&render_info, model_num, orient, v);

	//OK we're done
	gr_end_view_matrix();
	gr_end_proj_matrix();

	//Bye!!
	g3_end_frame();
	gr_reset_clip();

	return ade_set_args(L, "i", 0);
}

// Wanderer
ADE_FUNC(drawModelOOR, l_Graphics, "model Model, vector Position, matrix Orientation, [integer Flags]", "Draws the given model with the specified position and orientation - Use with extreme care, designed to operate properly only in On Object Render hooks.", "int", "Zero if successful, otherwise an integer error code")
{
	model_h *mdl = NULL;
	vec3d *v = &vmd_zero_vector;
	matrix_h *mh = NULL;
	int flags = MR_NORMAL;
	if(!ade_get_args(L, "ooo|i", l_Model.GetPtr(&mdl), l_Vector.GetPtr(&v), l_Matrix.GetPtr(&mh), &flags))
		return ade_set_args(L, "i", 1);

	if(mdl == NULL)
		return ade_set_args(L, "i", 2);

	polymodel *pm = mdl->Get();

	if (pm == NULL)
		return ade_set_args(L, "i", 3);

	int model_num = pm->id;

	if(model_num < 0)
		return ade_set_args(L, "i", 3);

	//Handle angles
	matrix *orient = mh->GetMatrix();

	//Draw the ship!!
	model_clear_instance(model_num);

	model_render_params render_info;
	render_info.set_flags(flags);

	model_render_immediate(&render_info, model_num, orient, v);

	return ade_set_args(L, "i", 0);
}

// Aardwolf's targeting brackets function
ADE_FUNC(drawTargetingBrackets, l_Graphics, "object Object, [boolean draw=true, int padding=5]",
	"Gets the edge positions of targeting brackets for the specified object. The brackets will only be drawn if draw is true or the default value of draw is used. Brackets are drawn with the current color. The brackets will have a padding (distance from the actual bounding box); the default value (used elsewhere in FS2) is 5.",
	"number,number,number,number",
	"Left, top, right, and bottom positions of the brackets, or nil if invalid")
{
	if(!Gr_inited) {
		return ADE_RETURN_NIL;
	}

	object_h *objh = NULL;
	bool draw_box = true;
	int padding = 5;

	if( !ade_get_args(L, "o|bi", l_Object.GetPtr(&objh), &draw_box, &padding) ) {
		return ADE_RETURN_NIL;
	}

	// The following code is mostly copied from
	// void hud_show_brackets(object *targetp, vertex *projected_v)
	// in hudtarget.cpp

	if( !objh->IsValid()) {
		return ADE_RETURN_NIL;
	}

	object *targetp = objh->objp;

	int x1,x2,y1,y2;
	int bound_rc, pof;
	int modelnum;
	bool entered_frame = false;
	SCP_list<CJumpNode>::iterator jnp;
	
	if ( !(g3_in_frame( ) > 0 ) )
	{
		g3_start_frame( 0 );
		entered_frame = true;
	}


	switch ( targetp->type ) {
		case OBJ_SHIP:
			modelnum = Ship_info[Ships[targetp->instance].ship_info_index].model_num;
			bound_rc = model_find_2d_bound_min( modelnum, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			if ( bound_rc != 0 ) {
				if ( entered_frame )
					g3_end_frame( );
				return ADE_RETURN_NIL;
			}
			break;
		case OBJ_DEBRIS:
			modelnum = Debris[targetp->instance].model_num;
			bound_rc = submodel_find_2d_bound_min( modelnum, Debris[targetp->instance].submodel_num, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			if ( bound_rc != 0 ) {
				if ( entered_frame )
					g3_end_frame( );
				return ADE_RETURN_NIL;
			}
			break;
		case OBJ_WEAPON:
			Assert(Weapon_info[Weapons[targetp->instance].weapon_info_index].subtype == WP_MISSILE);
			modelnum = Weapon_info[Weapons[targetp->instance].weapon_info_index].model_num;
			bound_rc = model_find_2d_bound_min( modelnum, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			break;
		case OBJ_ASTEROID:
			pof = Asteroids[targetp->instance].asteroid_subtype;
			modelnum = Asteroid_info[Asteroids[targetp->instance].asteroid_type].model_num[pof];
			bound_rc = model_find_2d_bound_min( modelnum, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			break;
		case OBJ_JUMP_NODE:
			for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
				if(jnp->GetSCPObject() == targetp)
					break;
			}
			
			modelnum = jnp->GetModelNumber();
			bound_rc = model_find_2d_bound_min( modelnum, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			break;
		default: //Someone passed an invalid pointer.
			if ( entered_frame )
				g3_end_frame( );
			return ADE_RETURN_NIL;
	}

	x1 -= padding;
	x2 += padding;
	y1 -= padding;
	y2 += padding;
	if ( draw_box ) {
		draw_brackets_square(x1, y1, x2, y2, GR_RESIZE_NONE);
	}

	if ( entered_frame )
		g3_end_frame( );

	return ade_set_args(L, "iiii", x1, y1, x2, y2);
}

ADE_FUNC(drawSubsystemTargetingBrackets, l_Graphics, "subsystem subsys, [boolean draw=true, boolean setColor=false]",
	"Gets the edge position of the targeting brackets drawn for a subsystem as if they were drawn on the HUD. Only actually draws the brackets if <i>draw</i> is true, optionally sets the color the as if it was drawn on the HUD",
	"number,number,number,number",
	"Left, top, right, and bottom positions of the brackets, or nil if invalid or off-screen")
{
	if(!Gr_inited) {
		return ADE_RETURN_NIL;
	}

	ship_subsys_h *sshp = NULL;
	bool draw = true;
	bool set_color = false;

	if( !ade_get_args(L, "o|bb", l_Subsystem.GetPtr(&sshp), &draw, &set_color) ) {
		return ADE_RETURN_NIL;
	}

	if (!sshp->IsValid())
	{
		return ADE_RETURN_NIL;
	}

	bool entered_frame = false;
	
	if ( !(g3_in_frame( ) > 0 ) )
	{
		g3_start_frame( 0 );
		entered_frame = true;
	}

	int coords[4];

	int in_sight = draw_subsys_brackets(sshp->ss, 24, 24, draw, set_color, coords);

	if ( entered_frame )
		g3_end_frame( );

	if (in_sight > 0)
	{
		return ade_set_args(L, "iiii", coords[0], coords[1], coords[2], coords[3]);
	}
	else
	{
		return ADE_RETURN_NIL;
	}
}

ADE_FUNC(drawOffscreenIndicator, l_Graphics, "object Object, [boolean draw=true, boolean setColor=false]",
	"Draws an off-screen indicator for the given object. The indicator will not be drawn if draw=false, but the coordinates will be returned in either case. The indicator will be drawn using the current color if setColor=true and using the IFF color of the object if setColor=false.",
	"number,number",
	"Coordinates of the indicator (at the very edge of the screen), or nil if object is on-screen")
{
	object_h *objh = NULL;
	bool draw = false;
	bool setcolor = false;
	vec2d outpoint = { -1.0f, -1.0f };
	
	if(!Gr_inited) {
		return ADE_RETURN_NIL;
	}
	
	if( !ade_get_args(L, "o|bb", l_Object.GetPtr(&objh), &draw, &setcolor) ) {
		return ADE_RETURN_NIL;
	}

	if( !objh->IsValid()) {
		return ADE_RETURN_NIL;
	}

	object *targetp = objh->objp;
	bool in_frame = g3_in_frame() > 0;

	if (!in_frame)
		g3_start_frame(0);

	vertex target_point;
	g3_rotate_vertex(&target_point, &targetp->pos);
	g3_project_vertex(&target_point);

	if (!in_frame)
		g3_end_frame();

	if(target_point.codes == 0)
		return ADE_RETURN_NIL;

	hud_target_clear_display_list();
	hud_target_add_display_list(targetp, &target_point, &targetp->pos, 5, NULL, NULL, TARGET_DISPLAY_DIST);

	size_t j, num_gauges;
	num_gauges = default_hud_gauges.size();

	for(j = 0; j < num_gauges; j++) {
		if (default_hud_gauges[j]->getObjectType() == HUD_OBJECT_OFFSCREEN) {
			HudGaugeOffscreen *offscreengauge = static_cast<HudGaugeOffscreen*>(default_hud_gauges[j]);
			
			offscreengauge->preprocess();
			offscreengauge->onFrame(flFrametime);

			if ( !offscreengauge->canRender() ) {
				break;
			}

			offscreengauge->resetClip();
			offscreengauge->setFont();
			int dir;
			float tri_separation;

			offscreengauge->calculatePosition(&target_point, &targetp->pos, &outpoint, &dir, &tri_separation);

			if (draw) {
				float distance = hud_find_target_distance(targetp, Player_obj);

				if (!setcolor)
					hud_set_iff_color(targetp, 1);

				offscreengauge->renderOffscreenIndicator(&outpoint, dir, distance, tri_separation, true);
			}

			offscreengauge->resize(&outpoint.x, &outpoint.y);

			break;
		}
	}

	if (outpoint.x >= 0 && outpoint.y >=0)
		return ade_set_args(L, "ii", (int)outpoint.x, (int)outpoint.y);
	else
		return ADE_RETURN_NIL;
}

#define MAX_TEXT_LINES		256
static char *BooleanValues[] = {"False", "True"};
static const int NextDrawStringPosInitial[] = {0, 0};
static int NextDrawStringPos[] = {NextDrawStringPosInitial[0], NextDrawStringPosInitial[1]};
ADE_FUNC(drawString, l_Graphics, "string Message, [number X1, number Y1, number X2, number Y2]",
		 "Draws a string. Use x1/y1 to control position, x2/y2 to limit textbox size."
		 "Text will automatically move onto new lines, if x2/y2 is specified."
		 "Additionally, calling drawString with only a string argument will automatically"
		 "draw that string below the previously drawn string (or 0,0 if no strings"
		 "have been drawn yet",
		 "number",
		 "Number of lines drawn, or 0 on failure")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	int x=NextDrawStringPos[0];
	int y = NextDrawStringPos[1];

	char *s = "(null)";
	int x2=-1,y2=-1;
	int num_lines = 0;

	if(lua_isboolean(L, 1))
	{
		bool b = false;
		if(!ade_get_args(L, "b|iiii", &b, &x, &y, &x2, &y2))
			return ade_set_error(L, "i", 0);

		if(b)
			s = BooleanValues[1];
		else
			s = BooleanValues[0];
	}
	else if(lua_isstring(L, 1))
	{
		if(!ade_get_args(L, "s|iiii", &s, &x, &y, &x2, &y2))
			return ade_set_error(L, "i", 0);
	}
	else
	{
		ade_get_args(L, "|*iiii", &x, &y, &x2, &y2);
	}

	NextDrawStringPos[0] = x;
	if(x2 < 0)
	{
		num_lines = 1;
		gr_string(x,y,s,GR_RESIZE_NONE);

		int height = 0;
		gr_get_string_size(NULL, &height, s);
		NextDrawStringPos[1] = y+height;
	}
	else
	{
		int linelengths[MAX_TEXT_LINES];
		const char *linestarts[MAX_TEXT_LINES];

		if (y2 >= 0 && y2 < y)
		{
			// Invalid y2 value
			Warning(LOCATION, "Illegal y2 value passed to drawString. Got %d y2 value but %d for y.", y2, y);
		
			int temp = y;
			y = y2;
			y2 = temp;
		}

		num_lines = split_str(s, x2-x, linelengths, linestarts, MAX_TEXT_LINES);

		//Make sure we don't go over size
		int line_ht = gr_get_font_height();
		num_lines = MIN(num_lines, (y2 - y) / line_ht);

		int curr_y = y;
		for(int i = 0; i < num_lines; i++)
		{
			//Contrary to WMC's previous comment, let's make a new string each line
			int len = linelengths[i];
			char *buf = new char[len+1];
			strncpy(buf, linestarts[i], len);
			buf[len] = '\0';

			//Draw the string
			gr_string(x,curr_y,buf,GR_RESIZE_NONE);

			//Free the string we made
			delete[] buf;

			//Increment line height
			curr_y += line_ht;
		}
		
		if (num_lines <= 0)
		{
			// If no line was drawn then we need to add one so the next line is 
			// aligned right
			curr_y += line_ht;
		}
		
		NextDrawStringPos[1] = curr_y;
	}
	return ade_set_error(L, "i", num_lines);
}

ADE_FUNC(getStringWidth, l_Graphics, "string String", "Gets string width", "number", "String width, or 0 on failure")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	char *s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "i", 0);

	int w;

	gr_get_string_size(&w, NULL, s);
	
	return ade_set_args(L, "i", w);
}

ADE_FUNC(loadStreamingAnim, l_Graphics, "string Filename, [boolean loop, boolean reverse, boolean pause, boolean cache]",
		 "Plays a streaming animation, returning its handle. The optional booleans (except cache) can also be set via the handles virtvars<br>"
		 "cache is best set to false when loading animations that are only intended to play once, e.g. headz<br>"
		 "Remember to call the unload() function when you're finished using the animation to free up memory.",
		 "streaminganim",
		 "Streaming animation handle, or invalid handle if animation couldn't be loaded")
{
	char *s;
	int rc = -1;
	bool loop = false, reverse = false, pause = false, cache = true;

	if(!ade_get_args(L, "s|bbbb", &s, &loop, &reverse, &pause, &cache))
		return ADE_RETURN_NIL;

	streaminganim_h sah(s);
	if (loop == false) {
		sah.ga.direction |= GENERIC_ANIM_DIRECTION_NOLOOP;
	}
	if (reverse == true) {
		sah.ga.direction |= GENERIC_ANIM_DIRECTION_BACKWARDS;
	}
	if (pause == true) {
		sah.ga.direction |= GENERIC_ANIM_DIRECTION_PAUSED;
	}
	rc = generic_anim_stream(&sah.ga, cache);

	if(rc < 0)
		return ade_set_args(L, "o", l_streaminganim.Set(sah)); // this object should be "invalid", matches loadTexture behaviour

	return ade_set_args(L, "o", l_streaminganim.Set(sah));
}

ADE_FUNC(createTexture, l_Graphics, "[number Width=512, number Height=512, enumeration Type=TEXTURE_DYNAMIC]",
		 "Creates a texture for rendering to."
		 "Types are TEXTURE_STATIC - for infrequent rendering - and TEXTURE_DYNAMIC - for frequent rendering.",
		 "texture",
		 "New texture handle, or invalid texture handle if texture couldn't be created")
{
	int w=512;
	int h=512;
	enum_h *e = NULL;

	//GET ARGS
	ade_get_args(L, "|iio", &w, &h, l_Enum.GetPtr(&e));

	int t = BMP_FLAG_RENDER_TARGET_DYNAMIC;
	if(e != NULL)
	{
		if(e->index == LE_TEXTURE_STATIC)
			t = BMP_FLAG_RENDER_TARGET_STATIC;
		else if(e->index == LE_TEXTURE_DYNAMIC)
			t = BMP_FLAG_RENDER_TARGET_DYNAMIC;
	}

	int idx = bm_make_render_target(w, h, t);

	if(idx < 0)
		return ade_set_error(L, "o", l_Texture.Set(-1));

	return ade_set_args(L, "o", l_Texture.Set(idx));
}

ADE_FUNC(loadTexture, l_Graphics, "string Filename, [boolean LoadIfAnimation, boolean NoDropFrames]",
		 "Gets a handle to a texture. If second argument is set to true, animations will also be loaded."
		 "If third argument is set to true, every other animation frame will not be loaded if system has less than 48 MB memory."
		 "<br><strong>IMPORTANT:</strong> Textures will not be unload themselves unless you explicitly tell them to do so."
		 "When you are done with a texture, call the Unload() function to free up memory.",
		 "texture",
		 "Texture handle, or invalid texture handle if texture couldn't be loaded")
{
	char *s;
	int idx=-1;
	bool b=false;
	bool d=false;

	if(!ade_get_args(L, "s|bb", &s, &b, &d))
		return ade_set_error(L, "o", l_Texture.Set(-1));

	if(b == true) {
		idx = bm_load_animation(s, nullptr, nullptr, nullptr, nullptr, d);
	}
	if(idx < 0) {
		idx = bm_load(s);
	}

	if(idx < 0)
		return ade_set_error(L, "o", l_Texture.Set(-1));

	return ade_set_args(L, "o", l_Texture.Set(idx));
}

ADE_FUNC(drawImage, l_Graphics, "string Filename/texture Texture, [number X1=0, Y1=0, number X2, number Y2, number UVX1 = 0.0, number UVY1 = 0.0, number UVX2=1.0, number UVY2=1.0, number alpha=1.0]",
		 "Draws an image or texture. Any image extension passed will be ignored."
		 "The UV variables specify the UV value for each corner of the image. "
		 "In UV coordinates, (0,0) is the top left of the image; (1,1) is the lower right.",
		 "boolean",
		 "Whether image was drawn")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", false);

	int idx;
	int x1 = 0;
	int y1 = 0;
	int x2=INT_MAX;
	int y2=INT_MAX;
	float uv_x1=0.0f;
	float uv_y1=0.0f;
	float uv_x2=1.0f;
	float uv_y2=1.0f;
	float alpha=1.0f;

	if(lua_isstring(L, 1))
	{
		char *s = NULL;
		if(!ade_get_args(L, "s|iiiifffff", &s,&x1,&y1,&x2,&y2,&uv_x1,&uv_y1,&uv_x2,&uv_y2,&alpha))
			return ade_set_error(L, "b", false);

		idx = Script_system.LoadBm(s);

		if(idx < 0)
			return ADE_RETURN_FALSE;
	}
	else
	{
		if(!ade_get_args(L, "o|iiiifffff", l_Texture.Get(&idx),&x1,&y1,&x2,&y2,&uv_x1,&uv_y1,&uv_x2,&uv_y2,&alpha))
			return ade_set_error(L, "b", false);
	}

	if(!bm_is_valid(idx))
		return ade_set_error(L, "b", false);

	int w, h;
	if(bm_get_info(idx, &w, &h) < 0)
		return ADE_RETURN_FALSE;

	if(x2!=INT_MAX)
		w = x2-x1;

	if(y2!=INT_MAX)
		h = y2-y1;

	gr_set_bitmap(idx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL, alpha);
	bitmap_rect_list brl = bitmap_rect_list(x1, y1, w, h, uv_x1, uv_y1, uv_x2, uv_y2);
	gr_bitmap_list(&brl, 1, GR_RESIZE_NONE);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawMonochromeImage, l_Graphics, "string Filename/texture Texture, number X1, number Y1, [number X2, number Y2, number alpha=1.0]", "Draws a monochrome image using the current color", "boolean", "Whether image was drawn")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", false);

	int idx;
	int x,y;
	int x2=INT_MAX;
	int y2=INT_MAX;
	int sx=0;
	int sy=0;
	bool m = false;
	float alpha=1.0;

	if(lua_isstring(L, 1))
	{
		char *s = NULL;
		if(!ade_get_args(L, "sii|iif", &s,&x,&y,&x2,&y2,&alpha))
			return ade_set_error(L, "b", false);

		idx = Script_system.LoadBm(s);

		if(idx < 0)
			return ADE_RETURN_FALSE;
	}
	else
	{
		if(!ade_get_args(L, "oii|iif", l_Texture.Get(&idx),&x,&y,&x2,&y2,&alpha))
			return ade_set_error(L, "b", false);
	}

	if(!bm_is_valid(idx))
		return ade_set_error(L, "b", false);

	int w, h;
	if(bm_get_info(idx, &w, &h) < 0)
		return ADE_RETURN_FALSE;

	if(sx < 0)
		sx = w + sx;

	if(sy < 0)
		sy = h + sy;
	
	if(x2!=INT_MAX)
		w = x2-x;

	if(y2!=INT_MAX)
		h = y2-y;

	gr_set_bitmap(idx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL,alpha);
	gr_aabitmap_ex(x, y, w, h, sx, sy, GR_RESIZE_NONE, m);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getImageWidth, l_Graphics, "string Filename", "Gets image width", "number", "Image width, or 0 if filename is invalid")
{
	char *s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "i", 0);

	int w;
	
	int idx = bm_load(s);

	if(idx < 0)
		return ade_set_error(L, "i", 0);

	bm_get_info(idx, &w);
	return ade_set_args(L, "i", w);
}

ADE_FUNC(getImageHeight, l_Graphics, "Image name", "Gets image height", "number", "Image height, or 0 if filename is invalid")
{
	char *s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "i", 0);

	int h;
	
	int idx = bm_load(s);

	if(idx < 0)
		return ade_set_error(L, "i", 0);

	bm_get_info(idx, NULL, &h);
	return ade_set_args(L, "i", h);
}

ADE_FUNC(flashScreen, l_Graphics, "number Red, number Green, number Blue", "Flashes the screen", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int r,g,b;

	if(!ade_get_args(L, "iii", &r, &g, &b))
		return ADE_RETURN_NIL;

	gr_flash(r,g,b);

	return ADE_RETURN_NIL;
}

ADE_FUNC(loadModel, l_Graphics, "string Filename", "Loads the model - will not setup subsystem data, DO NOT USE FOR LOADING SHIP MODELS", "model", "Handle to a model")
{
	char *s;
	int model_num = -1;

	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	if (s[0] == '\0')
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	model_num = model_load(s, 0, NULL);

	return ade_set_args(L, "o", l_Model.Set(model_h(model_num)));
}

ADE_FUNC(hasViewmode, l_Graphics, "enumeration", "Specifies if the current viemode has the specified flag, see VM_* enumeration", "boolean", "true if flag is present, false otherwise")
{
	enum_h *type = NULL;

	if (!ade_get_args(L, "o", l_Enum.GetPtr(&type)))
		return ade_set_error(L, "b", false);

	if (type == NULL || !type->IsValid())
		return ade_set_error(L, "b", false);

	int bit = 0;

	switch(type->index)
	{
	case LE_VM_INTERNAL:
		return ade_set_args(L, "b", (Viewer_mode & ~(VM_CAMERA_LOCKED | VM_CENTERING)) == 0);	// z64: Ignore camera lock state and centering state
		break;

	case LE_VM_EXTERNAL:
		bit = VM_EXTERNAL;
		break;

	case LE_VM_OTHER_SHIP:
		bit = VM_OTHER_SHIP;
		break;

	case LE_VM_CHASE:
		bit = VM_CHASE;
		break;

	case LE_VM_DEAD_VIEW:
		bit = VM_DEAD_VIEW;
		break;

	case LE_VM_EXTERNAL_CAMERA_LOCKED:
		return ade_set_args(L, "b", (Viewer_mode & VM_CAMERA_LOCKED) && (Viewer_mode & VM_EXTERNAL));
		break;

	case LE_VM_CAMERA_LOCKED:
		bit = VM_CAMERA_LOCKED;
		break;

	case LE_VM_FREECAMERA:
		bit = VM_FREECAMERA;
		break;

	case LE_VM_PADLOCK_LEFT:
		bit = VM_PADLOCK_LEFT;
		break;

	case LE_VM_PADLOCK_REAR:
		bit = VM_PADLOCK_REAR;
		break;

	case LE_VM_PADLOCK_RIGHT:
		bit = VM_PADLOCK_RIGHT;
		break;

	case LE_VM_PADLOCK_UP:
		bit = VM_PADLOCK_UP;
		break;

	case LE_VM_TOPDOWN:
		bit = VM_TOPDOWN;
		break;

	case LE_VM_TRACK:
		bit = VM_TRACK;
		break;

	case LE_VM_WARP_CHASE:
		bit = VM_WARP_CHASE;
		break;

	case LE_VM_WARPIN_ANCHOR:
		bit = VM_WARPIN_ANCHOR;
		break;

	case LE_VM_CENTERING:
		bit = VM_CENTERING;
		break;

	default:
		LuaError(L, "Attempted to use hasViewmode with an invalid enumeration! Only VM_* enumerations are allowed!");
		return ade_set_error(L, "b", false);
		break;
	}

	return ade_set_args(L, "b", (Viewer_mode & bit) != 0);
}

ADE_FUNC(setClip, l_Graphics, "x, y, width, height", "Sets the clipping region to the specified rectangle. Most drawing functions are able to handle the offset.", "boolean", "true if successful, false otherwise")
{
	int x, y, width, height;

	if (!ade_get_args(L, "iiii", &x, &y, &width, &height))
		return ADE_RETURN_FALSE;

	gr_set_clip(x, y, width, height, GR_RESIZE_NONE);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(resetClip, l_Graphics, NULL, "Resets the clipping region that might have been set", "boolean", "true if successful, false otherwise")
{
	gr_reset_clip();

	return ADE_RETURN_TRUE;
}

//**********LIBRARY: Scripting Variables
ade_lib l_HookVar("HookVariables", NULL, "hv", "Hook variables repository");

//WMC: IMPORTANT
//Be very careful when modifying this library, as the Globals[] library does depend
//on the current number of items in the library. If you add _anything_, modify __len.
//Or run changes by me.

//*****LIBRARY: Scripting Variables
ade_lib l_HookVar_Globals("Globals", &l_HookVar);

ADE_INDEXER(l_HookVar_Globals, "number Index", "Array of current HookVariable names", "string", "Hookvariable name, or empty string if invalid index specified")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");

	//Get lib
	lua_getglobal(L, l_HookVar.GetName());
	int lib_ldx = lua_gettop(L);
	if(!lua_isuserdata(L, lib_ldx))
	{
		lua_pop(L, 1);
		return ade_set_error(L, "s", "");
	}

	//Get metatable
	lua_getmetatable(L, lib_ldx);
	int mtb_ldx = lua_gettop(L);
	if(!lua_istable(L, mtb_ldx))
	{
		lua_pop(L, 2);
		return ade_set_error(L, "s", "");
	}

	//Get ade members table
	lua_pushstring(L, "__ademembers");
	lua_rawget(L, mtb_ldx);
	int amt_ldx = lua_gettop(L);
	if(!lua_istable(L, amt_ldx))
	{
		lua_pop(L, 3);
		return ade_set_error(L, "s", "");
	}

	//List 'em
	char *keyname = NULL;
	int count = 1;
	lua_pushnil(L);
	while(lua_next(L, amt_ldx))
	{
		//Now on stack: Key, value
		lua_pushvalue(L, -2);
		keyname = (char *)lua_tostring(L, -1);
		if(strcmp(keyname, "Globals"))
		{
			if(count == idx)
			{
				//lib, mtb, amt, key, value, string go bye-bye
				lua_pop(L, 5);
				return ade_set_args(L, "s", keyname);
			}
			count++;
		}
		lua_pop(L, 2);	//Value, string
	}

	lua_pop(L, 3);	//lib, mtb, amt

	return ade_set_error(L, "s", "");
}

ADE_FUNC(__len, l_HookVar_Globals, NULL, "Number of HookVariables", "number", "Number of HookVariables")
{
	//Get metatable
	lua_getglobal(L, l_HookVar.GetName());
	int lib_ldx = lua_gettop(L);
	if(!lua_isuserdata(L, lib_ldx))
	{
		lua_pop(L, 1);
		return ade_set_error(L, "i", 0);
	}

	lua_getmetatable(L, lib_ldx);
	int mtb_ldx = lua_gettop(L);
	if(!lua_istable(L, mtb_ldx))
	{
		lua_pop(L, 2);
		return ade_set_error(L, "i", 0);
	}

	//Get ade members table
	lua_pushstring(L, "__ademembers");
	lua_rawget(L, mtb_ldx);
	int amt_ldx = lua_gettop(L);
	if(!lua_istable(L, amt_ldx))
	{
		lua_pop(L, 3);
		return ade_set_error(L, "i", 0);
	}

	int total_len = 0;
	lua_pushnil(L);
	while(lua_next(L, amt_ldx))
	{
		total_len++;
		lua_pop(L, 1);	//value
	}
	size_t num_sub = ade_manager::getInstance()->getEntry(l_HookVar.GetIdx()).Num_subentries;

	lua_pop(L, 3);

	//WMC - Return length, minus the 'Globals' library
	return ade_set_args(L, "i", (int)(total_len - num_sub));
}

//**********LIBRARY: Mission
ade_lib l_Mission("Mission", NULL, "mn", "Mission library");

// for use in creating faster metadata systems, use in conjunction with getSignature()
ADE_FUNC(getObjectFromSignature, l_Mission, "number Signature", "Gets a handle of an object from its signature", "object", "Handle of object with signaure, invalid handle if signature is not in use")
{
	int sig = -1;
	int objnum;
	if(!ade_get_args(L, "i", &sig))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if (sig == -1) {
		return ade_set_error(L, "o", l_Object.Set(object_h()));
	}

	objnum = obj_get_by_signature(sig);

	return ade_set_object_with_breed(L, objnum);
}

ADE_FUNC(evaluateSEXP, l_Mission, "string", "Runs the defined SEXP script", "boolean", "if the operation was successful")
{
	char *s;
	int r_val;

	if(!ade_get_args(L, "s", &s))
		return ADE_RETURN_FALSE;

	r_val = run_sexp(s);

	if (r_val == SEXP_TRUE)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(runSEXP, l_Mission, "string", "Runs the defined SEXP script", "boolean", "if the operation was successful")
{
	char *s;
	int r_val;
	char buf[8192];

	if(!ade_get_args(L, "s", &s))
		return ADE_RETURN_FALSE;

	while (is_white_space(*s))
		s++;
	if (*s != '(')
	{
		static bool Warned_about_runSEXP_parentheses = false;
		if (!Warned_about_runSEXP_parentheses)
		{
			Warned_about_runSEXP_parentheses = true;
			Warning(LOCATION, "Invalid SEXP syntax: SEXPs must be surrounded by parentheses.  For backwards compatibility, the string has been enclosed in parentheses.  This may not be correct in all use cases.");
		}
		// this is the old sexp handling method, which is incorrect
		snprintf(buf, 8191, "( when ( true ) ( %s ) )", s);
	}
	else
	{
		// this is correct usage
		snprintf(buf, 8191, "( when ( true ) %s )", s);
	}

	r_val = run_sexp(buf);

	if (r_val == SEXP_TRUE)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

//****SUBLIBRARY: Mission/Asteroids
ade_lib l_Mission_Asteroids("Asteroids", &l_Mission, NULL, "Asteroids in the mission");

ADE_INDEXER(l_Mission_Asteroids, "number Index", "Gets asteroid", "asteroid", "Asteroid handle, or invalid handle if invalid index specified")
{
	int idx = -1;
	if( !ade_get_args(L, "*i", &idx) ) {
		return ade_set_error( L, "o", l_Asteroid.Set( object_h() ) );
	}
	if( idx > -1 && idx < asteroid_count() ) {
		idx--; //Convert from Lua to C, as lua indices start from 1, not 0
		return ade_set_args( L, "o", l_Asteroid.Set( object_h( &Objects[Asteroids[idx].objnum] ), Objects[Asteroids[idx].objnum].signature ) );
	}

	return ade_set_error(L, "o", l_Asteroid.Set( object_h() ) );
}

ADE_FUNC(__len, l_Mission_Asteroids, NULL,
		 "Number of asteroids in mission. Note that the value returned is only good until an asteroid is destroyed, and so cannot be relied on for more than one frame.",
		 "number",
		 "Number of asteroids in the mission, or 0 if asteroids are not enabled")
{
	if(Asteroids_enabled) {
		return ade_set_args(L, "i", asteroid_count());
	}
	return ade_set_args(L, "i", 0);
}

//****SUBLIBRARY: Mission/Debris
ade_lib l_Mission_Debris("Debris", &l_Mission, NULL, "debris in the mission");

ADE_INDEXER(l_Mission_Debris, "number Index", "Array of debris in the current mission", "debris", "Debris handle, or invalid debris handle if index wasn't valid")
{
	int idx = -1;
	if( !ade_get_args( L, "*i", &idx ) ) {
		return ade_set_error(L, "o", l_Debris.Set(object_h()));
	}
	if( idx > -1 && idx < Num_debris_pieces ) {
		idx--; // Lua -> C
		if (Debris[idx].objnum == -1) //Somehow accessed an invalid debris piece
			return ade_set_error(L, "o", l_Debris.Set(object_h()));
		return ade_set_args(L, "o", l_Debris.Set(object_h(&Objects[Debris[idx].objnum]), Objects[Debris[idx].objnum].signature));
	}

	return ade_set_error(L, "o", l_Debris.Set(object_h()));
}

ADE_FUNC(__len, l_Mission_Debris, NULL, 
		 "Number of debris pieces in the mission. "
		 "Note that the value returned is only good until a piece of debris is destroyed, and so cannot be relied on for more than one frame.",
		 "number",
		 "Current number of debris particles")
{
	return ade_set_args(L, "i", Num_debris_pieces);
}

//****SUBLIBRARY: Mission/EscortShips
ade_lib l_Mission_EscortShips("EscortShips", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_EscortShips, "number Index", "Gets escort ship at specified index on escort list", "ship", "Specified ship, or invalid ship handle if invalid index")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	if(idx < 1 || idx > hud_escort_num_ships_on_list())
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	//Lua->FS2
	idx--;

	idx = hud_escort_return_objnum(idx);
	
	if(idx < 0)
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[idx])));
}

ADE_FUNC(__len, l_Mission_EscortShips, NULL, "Current number of escort ships", "number", "Current number of escort ships")
{
	return ade_set_args(L, "i", hud_escort_num_ships_on_list());
}

//****SUBLIBRARY: Mission/Events
ade_lib l_Mission_Events("Events", &l_Mission, NULL, "Events");

ADE_INDEXER(l_Mission_Events, "number Index/string Name", "Indexes events list", "event", "Event handle, or invalid event handle if index was invalid")
{
	char *s;
	if(!ade_get_args(L, "*s", &s))
		return ade_set_error(L, "o", l_Event.Set(-1));

	int i;
	for(i = 0; i < Num_mission_events; i++)
	{
		if(!stricmp(Mission_events[i].name, s))
			return ade_set_args(L, "o", l_Event.Set(i));
	}

	//Now try as a number
	i = atoi(s);
	if(i < 1 || i > Num_mission_events)
		return ade_set_error(L, "o", l_Event.Set(-1));

	//Lua-->FS2
	i--;

	return ade_set_args(L, "o", l_Event.Set(i));
}

ADE_FUNC(__len, l_Mission_Events, NULL, "Number of events in mission", "number", "Number of events in mission")
{
	return ade_set_args(L, "i", Num_mission_events);
}

//****SUBLIBRARY: Mission/SEXPVariables
ade_lib l_Mission_SEXPVariables("SEXPVariables", &l_Mission, NULL, "SEXP Variables");

ADE_INDEXER(l_Mission_SEXPVariables, "number Index/string Name", "Array of SEXP variables. Note that you can set a sexp variable using the array, eg \'SEXPVariables[\"newvariable\"] = \"newvalue\"\'", "sexpvariable", "Handle to SEXP variable, or invalid sexpvariable handle if index was invalid")
{
	char *name = NULL;
	char *newval = NULL;
	if(!ade_get_args(L, "*s|s", &name, &newval))
		return ade_set_error(L, "o", l_SEXPVariable.Set(sexpvar_h()));

	int idx = get_index_sexp_variable_name(name);
	if(idx < 0)
	{
		idx = atoi(name);

		//Lua-->FS2
		idx--;
	}

	if(idx < 0 || idx >= MAX_SEXP_VARIABLES)
	{
		if(ADE_SETTING_VAR && newval != NULL)
		{
			idx = sexp_add_variable(newval, name, lua_type(L, 2) == LUA_TNUMBER ? SEXP_VARIABLE_NUMBER : SEXP_VARIABLE_STRING);
		}

		//We have failed.
		if(idx < 0)
		{
			return ade_set_error(L, "o", l_SEXPVariable.Set(sexpvar_h()));
		}
	}
	else
	{
		if(ADE_SETTING_VAR && newval != NULL)
		{
			sexp_modify_variable(newval, idx, false);
		}
	}

	return ade_set_args(L, "o", l_SEXPVariable.Set(sexpvar_h(idx)));
}

ADE_FUNC(__len, l_Mission_SEXPVariables, NULL, "Current number of SEXP variables", "number", "Counts number of loaded SEXP Variables. May be slow.")
{
	return ade_set_args(L, "i", sexp_variable_count());
}

//****SUBLIBRARY: Mission/Ships
ade_lib l_Mission_Ships("Ships", &l_Mission, NULL, "Ships in the mission");

ADE_INDEXER(l_Mission_Ships, "number Index/string Name", "Gets ship", "ship", "Ship handle, or invalid ship handle if index was invalid")
{
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	int idx = ship_name_lookup(name);

	if(idx > -1)
	{
		return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[idx].objnum]), Objects[Ships[idx].objnum].signature));
	}
	else
	{
		idx = atoi(name);
		if(idx > 0)
		{
			int count=1;

			for(int i = 0; i < MAX_SHIPS; i++)
			{
				if (Ships[i].objnum < 0 || Objects[Ships[i].objnum].type != OBJ_SHIP)
					continue;

				if(count == idx) {
					return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[i].objnum]), Objects[Ships[i].objnum].signature));
				}

				count++;
			}
		}
	}

	return ade_set_error(L, "o", l_Ship.Set(object_h()));
}

extern int ships_inited;
ADE_FUNC(__len, l_Mission_Ships, NULL,
		 "Number of ships in the mission. "
		 "This function is somewhat slow, and should be set to a variable for use in looping situations. "
		 "Note that the value returned is only good until a ship is destroyed, and so cannot be relied on for more than one frame.",
		 "number",
		 "Number of ships in the mission, or 0 if ships haven't been initialized yet")
{
	if(ships_inited)
		return ade_set_args(L, "i", ship_get_num_ships());
	else
		return ade_set_args(L, "i", 0);
}

//****SUBLIBRARY: Mission/Waypoints
ade_lib l_Mission_Waypoints("Waypoints", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Waypoints, "number Index", "Array of waypoints in the current mission", "waypoint", "Waypoint handle, or invalid waypoint handle if index was invalid")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Waypoint.Set(object_h()));

	//Remember, Lua indices start at 0.
	int count=0;

	object *ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		if (ptr->type == OBJ_WAYPOINT)
			count++;

		if(count == idx) {
			return ade_set_args(L, "o", l_Waypoint.Set(object_h(ptr)));
		}

		ptr = GET_NEXT(ptr);
	}

	return ade_set_error(L, "o", l_Weapon.Set(object_h()));
}

ADE_FUNC(__len, l_Mission_Waypoints, NULL, "Gets number of waypoints in mission. Note that this is only accurate for one frame.", "number", "Number of waypoints in the mission")
{
	uint count=0;
	for(uint i = 0; i < MAX_OBJECTS; i++)
	{
		if (Objects[i].type == OBJ_WAYPOINT)
			count++;
	}

	return ade_set_args(L, "i", count);
}

//****SUBLIBRARY: Mission/WaypointLists
ade_lib l_Mission_WaypointLists("WaypointLists", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_WaypointLists, "number Index/string WaypointListName", "Array of waypoint lists", "waypointlist", "Gets waypointlist handle")
{
	waypointlist_h wpl;
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_WaypointList.Set(waypointlist_h()));

	wpl = waypointlist_h(name);

	if (!wpl.IsValid()) {
		int idx = atoi(name) - 1;
		wpl = waypointlist_h(find_waypoint_list_at_index(idx));
	}

	if (wpl.IsValid()) {
		return ade_set_args(L, "o", l_WaypointList.Set(wpl));
	}

	return ade_set_error(L, "o", l_WaypointList.Set(waypointlist_h()));
}

ADE_FUNC(__len, l_Mission_WaypointLists, NULL, "Number of waypoint lists in mission. Note that this is only accurate for one frame.", "number", "Number of waypoint lists in the mission")
{
	return ade_set_args(L, "i", Waypoint_lists.size());
}

//****SUBLIBRARY: Mission/Weapons
ade_lib l_Mission_Weapons("Weapons", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Weapons, "number Index", "Gets handle to a weapon object in the mission.", "weapon", "Weapon handle, or invalid weapon handle if index is invalid")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Weapon.Set(object_h()));

	//Remember, Lua indices start at 0.
	int count=1;

	for(int i = 0; i < MAX_WEAPONS; i++)
	{
		if (Weapons[i].weapon_info_index < 0 || Weapons[i].objnum < 0 || Objects[Weapons[i].objnum].type != OBJ_WEAPON)
			continue;

		if(count == idx) {
			return ade_set_args(L, "o", l_Weapon.Set(object_h(&Objects[Weapons[i].objnum])));
		}

		count++;
	}

	return ade_set_error(L, "o", l_Weapon.Set(object_h()));
}
ADE_FUNC(__len, l_Mission_Weapons, NULL, "Number of weapon objects in mission. Note that this is only accurate for one frame.", "number", "Number of weapon objects in mission")
{
	return ade_set_args(L, "i", Num_weapons);
}

//****SUBLIBRARY: Mission/Beams
ade_lib l_Mission_Beams("Beams", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Beams, "number Index", "Gets handle to a beam object in the mission.", "beam", "Beam handle, or invalid beam handle if index is invalid")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Beam.Set(object_h()));

	//Remember, Lua indices start at 0.
	int count=1;

	for(int i = 0; i < MAX_BEAMS; i++)
	{
		if (Beams[i].weapon_info_index < 0 || Beams[i].objnum < 0 || Objects[Beams[i].objnum].type != OBJ_BEAM)
			continue;

		if(count == idx) {
			return ade_set_args(L, "o", l_Beam.Set(object_h(&Objects[Beams[i].objnum])));
		}

		count++;
	}

	return ade_set_error(L, "o", l_Beam.Set(object_h()));
}
ADE_FUNC(__len, l_Mission_Beams, NULL, "Number of beam objects in mission. Note that this is only accurate for one frame.", "number", "Number of beam objects in mission")
{
	return ade_set_args(L, "i", Beam_count);
}

//****SUBLIBRARY: Campaign
ade_lib l_Campaign("Campaign", NULL, "ca", "Campaign Library");

ADE_FUNC(getNextMissionFilename, l_Campaign, NULL, "Gets next mission filename", "string", "Next mission filename, or nil if the next mission is invalid")
{
	if (Campaign.next_mission < 0 || Campaign.next_mission >= MAX_CAMPAIGN_MISSIONS) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", Campaign.missions[Campaign.next_mission].name);
}

ADE_FUNC(getPrevMissionFilename, l_Campaign, NULL, "Gets previous mission filename", "string", "Previous mission filename, or nil if the previous mission is invalid")
{
	if (Campaign.prev_mission < 0 || Campaign.prev_mission >= MAX_CAMPAIGN_MISSIONS) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", Campaign.missions[Campaign.prev_mission].name);
}

// DahBlount - This jumps to a mission, the reason it accepts a boolean value is so that players can return to campaign maps
ADE_FUNC(jumpToMission, l_Campaign, "string filename, [boolean hub]", "Jumps to a mission based on the filename. Optionally, the player can be sent to a hub mission without setting missions to skipped.", "boolean", "Jumps to a mission, or returns nil.")
{
	char *filename = NULL;;
	bool hub = false;
	if (!ade_get_args(L, "s|b", &filename, &hub))
		return ADE_RETURN_NIL;

	mission_campaign_jump_to_mission(filename, hub);

	return ADE_RETURN_TRUE;
}

// TODO: add a proper indexer type that returns a handle
// something like ca.Mission[filename/index]

//****SUBLIBRARY: Mission/Wings
ade_lib l_Mission_Wings("Wings", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Wings, "number Index/string WingName", "Wings in the mission", "wing", "Wing handle, or invalid wing handle if index or name was invalid")
{
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_Wing.Set(-1));

	//MageKing17 - Make the count-ignoring version of the lookup and leave checking if the wing has any ships to the scripter
	int idx = wing_lookup(name);
	
	if(idx < 0)
	{
		idx = atoi(name);
		if(idx < 1 || idx > Num_wings)
			return ade_set_error(L, "o", l_Wing.Set(-1));

		idx--;	//Lua->FS2
	}

	return ade_set_args(L, "o", l_Wing.Set(idx));
}

ADE_FUNC(__len, l_Mission_Wings, NULL, "Number of wings in mission", "number", "Number of wings in mission")
{
	return ade_set_args(L, "i", Num_wings);
}

//****SUBLIBRARY: Mission/Teams
ade_lib l_Mission_Teams("Teams", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Teams, "number Index/string TeamName", "Teams in the mission", "team", "Team handle or invalid team handle if the requested team could not be found")
{
	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_Team.Set(-1));

	int idx = iff_lookup(name);
	
	if(idx < 0)
	{
		idx = atoi(name);

		idx--;	//Lua->FS2
	}

	if(idx < 0 || idx >= Num_iffs)
		return ade_set_error(L, "o", l_Team.Set(-1));

	return ade_set_args(L, "o", l_Team.Set(idx));
}

ADE_FUNC(__len, l_Mission_Teams, NULL, "Number of teams in mission", "number", "Number of teams in mission")
{
	return ade_set_args(L, "i", Num_iffs);
}

//****SUBLIBRARY: Mission/Messages
ade_lib l_Mission_Messages("Messages", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Messages, "number Index/string messageName", "Messages of the mission", "message", "Message handle or invalid handle on error")
{
	int idx = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "*i", &idx))
			return ade_set_args(L, "o", l_Message.Set(-1));

		idx--; // Lua --> FS2

		idx += Num_builtin_messages;
	}
	else
	{
		char* name = NULL;

		if (!ade_get_args(L, "*s", &name))
			return ade_set_args(L, "o", l_Message.Set(-1));

		if (name == NULL)
			return ade_set_args(L, "o", l_Message.Set(-1));

		for (int i = Num_builtin_messages; i < (int) Messages.size(); i++)
		{
			if (!stricmp(Messages[i].name, name))
			{
				idx = i;
				break;
			}
		}
	}

	if (idx < Num_builtin_messages || idx >= (int) Messages.size())
		return ade_set_args(L, "o", l_Message.Set(-1));
	else
		return ade_set_args(L, "o", l_Message.Set(idx));
}

ADE_FUNC(__len, l_Mission_Messages, NULL, "Number of messages in the mission", "number", "Number of messages in mission")
{
	return ade_set_args(L, "i", (int) Messages.size() - Num_builtin_messages);
}

//****SUBLIBRARY: Mission/BuiltinMessages
ade_lib l_Mission_BuiltinMessages("BuiltinMessages", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_BuiltinMessages, "number Index/string messageName", "Built-in messages of the mission", "message", "Message handle or invalid handle on error")
{
	int idx = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "*i", &idx))
			return ade_set_args(L, "o", l_Message.Set(-1));

		idx--; // Lua --> FS2
	}
	else
	{
		char* name = NULL;

		if (!ade_get_args(L, "*s", &name))
			return ade_set_args(L, "o", l_Message.Set(-1));

		if (name == NULL)
			return ade_set_args(L, "o", l_Message.Set(-1));

		for (int i = 0; i < Num_builtin_messages; i++)
		{
			if (!stricmp(Messages[i].name, name))
			{
				idx = i;
				break;
			}
		}
	}

	if (idx < 0 || idx >= Num_builtin_messages)
		return ade_set_args(L, "o", l_Message.Set(-1));
	else
		return ade_set_args(L, "o", l_Message.Set(idx));
}

ADE_FUNC(__len, l_Mission_BuiltinMessages, NULL, "Number of built-in messages in the mission", "number", "Number of messages in mission")
{
	return ade_set_args(L, "i", Num_builtin_messages);
}

//****SUBLIBRARY: Mission/Personas
ade_lib l_Mission_Personas("Personas", &l_Mission, NULL, NULL);

ADE_INDEXER(l_Mission_Personas, "number Index/string name", "Personas of the mission", "persona", "Persona handle or invalid handle on error")
{
	int idx = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "*i", &idx))
			return ade_set_args(L, "o", l_Persona.Set(-1));

		idx--; // Lua --> FS2
	}
	else
	{
		char* name = NULL;

		if (!ade_get_args(L, "*s", &name))
			return ade_set_args(L, "o", l_Persona.Set(-1));

		if (name == NULL)
			return ade_set_args(L, "o", l_Persona.Set(-1));

		idx = message_persona_name_lookup(name);
	}

	if (idx < 0 || idx >= Num_personas)
		return ade_set_args(L, "o", l_Persona.Set(-1));
	else
		return ade_set_args(L, "o", l_Persona.Set(idx));
}

ADE_FUNC(__len, l_Mission_Personas, NULL, "Number of personas in the mission", "number", "Number of messages in mission")
{
	return ade_set_args(L, "i", Num_personas);
}

ADE_FUNC(addMessage, l_Mission, "string name, string text[, persona persona]", "Adds a message", "message", "The new message or invalid handle on error")
{
	char* name = NULL;
	char* text = NULL;
	int personaIdx = -1;

	if (!ade_get_args(L, "ss|o", &name, &text, l_Persona.Get(&personaIdx)))
		return ade_set_error(L, "o", l_Message.Set(-1));

	if (name == NULL || text == NULL)
		return ade_set_error(L, "o", l_Message.Set(-1));

	if (personaIdx < 0 || personaIdx >= Num_personas)
		personaIdx = -1;

	add_message(name, text, personaIdx, 0);

	return ade_set_error(L, "o", l_Message.Set((int) Messages.size() - 1));
}

ADE_FUNC(sendMessage, l_Mission, "string sender, message message[, number delay=0.0[, enumeration priority = MESSAGE_PRIORITY_NORMAL[, boolean fromCommand = false]]]",
		 "Sends a message from the given source (not from a ship!) with the given priority or optionally sends it from the missions command source.<br>"
		 "If delay is specified the message will be delayed by the specified time in seconds<br>"
		 "If you pass <i>nil</i> as the sender then the message will not have a sender.",
		 "boolean", "true if successfull, false otherwise")
{
	char* sender = NULL;
	int messageIdx = -1;
	int priority = MESSAGE_PRIORITY_NORMAL;
	bool fromCommand = false;
	int messageSource = MESSAGE_SOURCE_SPECIAL;
	float delay = 0.0f;

	enum_h* ehp = NULL;

	// if first is nil then use no source
	if (lua_isnil(L, 1))
	{
		if (!ade_get_args(L, "*o|fob", l_Message.Get(&messageIdx), &delay, l_Enum.GetPtr(&ehp), &fromCommand))
			return ADE_RETURN_FALSE;

		messageSource = MESSAGE_SOURCE_NONE;
	}
	else
	{
		if (!ade_get_args(L, "so|fob", &sender, l_Message.Get(&messageIdx), &delay, l_Enum.GetPtr(&ehp), &fromCommand))
			return ADE_RETURN_FALSE;

		if (sender == NULL)
			return ADE_RETURN_FALSE;
	}

	if (fromCommand)
		messageSource = MESSAGE_SOURCE_COMMAND;

	if (messageIdx < 0 || messageIdx >= (int) Messages.size())
		return ADE_RETURN_FALSE;

	if (messageIdx < Num_builtin_messages)
	{
		LuaError(L, "Cannot send built-in messages!");
		return ADE_RETURN_FALSE;
	}

	if (delay < 0.0f)
	{
		LuaError(L, "Invalid negative delay of %f!", delay);
		return ADE_RETURN_FALSE;
	}

	if (ehp != NULL)
	{
		switch(ehp->index)
		{
		case LE_MESSAGE_PRIORITY_HIGH:
			priority = MESSAGE_PRIORITY_HIGH;
			break;
		case LE_MESSAGE_PRIORITY_NORMAL:
			priority = MESSAGE_PRIORITY_NORMAL;
			break;
		case LE_MESSAGE_PRIORITY_LOW:
			priority = MESSAGE_PRIORITY_LOW;
			break;
		default:
			LuaError(L, "Invalid enumeration used! Must be one of MESSAGE_PRIORITY_*.");
			return ADE_RETURN_FALSE;
		}
	}

	if (messageSource == MESSAGE_SOURCE_NONE)
		message_send_unique_to_player(Messages[messageIdx].name, NULL, MESSAGE_SOURCE_NONE, priority, 0, fl2i(delay * 1000.0f));
	else
		message_send_unique_to_player(Messages[messageIdx].name, (void*) sender, messageSource, priority, 0, fl2i(delay * 1000.0f));

	return ADE_RETURN_TRUE;
}

ADE_FUNC(sendTrainingMessage, l_Mission, "message message, number time[, number delay=0.0]",
		 "Sends a training message to the player. <i>time</i> is the amount in seconds to display the message, only whole seconds are used!",
		 "boolean", "true if successfull, false otherwise")
{
	int messageIdx = -1;
	float delay = 0.0f;
	int time = -1;

	if (!ade_get_args(L, "oi|f", l_Message.Get(&messageIdx), &time, &delay))
		return ADE_RETURN_FALSE;

	if (messageIdx < 0 || messageIdx >= (int) Messages.size())
		return ADE_RETURN_FALSE;

	if (delay < 0.0f)
	{
		LuaError(L, "Got invalid delay of %f seconds!", delay);
		return ADE_RETURN_FALSE;
	}

	if (time < 0)
	{
		LuaError(L, "Got invalid time of %d seconds!", time);
		return ADE_RETURN_FALSE;
	}

	message_training_queue(Messages[messageIdx].name, timestamp(fl2i(delay * 1000.0f)), time);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(createShip, l_Mission, "[string Name, shipclass Class=Shipclass[1], orientation Orientation=null, vector Position={0,0,0}]", "Creates a ship and returns a handle to it using the specified name, class, world orientation, and world position", "ship", "Ship handle, or invalid ship handle if ship couldn't be created")
{
	char *name = NULL;
	int sclass = -1;
	matrix_h *orient = NULL;
	vec3d pos = vmd_zero_vector;
	ade_get_args(L, "|sooo", &name, l_Shipclass.Get(&sclass), l_Matrix.GetPtr(&orient), l_Vector.Get(&pos));

	matrix *real_orient = &vmd_identity_matrix;
	if(orient != NULL)
	{
		real_orient = orient->GetMatrix();
	}
	
	int obj_idx = ship_create(real_orient, &pos, sclass, name);

	if(obj_idx > -1) {
		model_page_in_textures(Ship_info[sclass].model_num, sclass);

		return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[obj_idx]), Objects[obj_idx].signature));
	} else
		return ade_set_error(L, "o", l_Ship.Set(object_h()));
}

ADE_FUNC(createWaypoint, l_Mission, "[vector Position, waypointlist List]",
		 "Creates a waypoint",
		 "waypoint",
		 "Waypoint handle, or invalid waypoint handle if waypoint couldn't be created")
{
	vec3d *v3 = NULL;
	waypointlist_h *wlh = NULL;
	if(!ade_get_args(L, "|oo", l_Vector.GetPtr(&v3), l_WaypointList.GetPtr(&wlh)))
		return ade_set_error(L, "o", l_Waypoint.Set(object_h()));

	// determine where we need to create it - it looks like we were given a waypoint list but not a waypoint itself
	int waypoint_instance = -1;
	if (wlh && wlh->IsValid())
	{
		int wp_list_index = find_index_of_waypoint_list(wlh->wlp);
		int wp_index = (int) wlh->wlp->get_waypoints().size() - 1;
		waypoint_instance = calc_waypoint_instance(wp_list_index, wp_index);
	}
	int obj_idx = waypoint_add(v3 != NULL ? v3 : &vmd_zero_vector, waypoint_instance);

	if(obj_idx >= 0)
		return ade_set_args(L, "o", l_Waypoint.Set(object_h(&Objects[obj_idx])));
	else
		return ade_set_args(L, "o", l_Waypoint.Set(object_h()));
}

ADE_FUNC(createWeapon, l_Mission, "[weaponclass Class=WeaponClass[1], orientation Orientation=null, world vector Position={0,0,0}, object Parent = nil, number Group = -1",
		 "Creates a weapon and returns a handle to it. 'Group' is used for lighting grouping purposes;"
		 " for example, quad lasers would only need to act as one light source.",
		 "weapon",
		 "Weapon handle, or invalid weapon handle if weapon couldn't be created.")
{
	int wclass = -1;
	object_h *parent = NULL;
	int group = -1;
	matrix_h *orient = NULL;
	vec3d pos = vmd_zero_vector;
	ade_get_args(L, "|ooooi", l_Weaponclass.Get(&wclass), l_Matrix.GetPtr(&orient), l_Vector.Get(&pos), l_Object.GetPtr(&parent), &group);

	matrix *real_orient = &vmd_identity_matrix;
	if(orient != NULL)
	{
		real_orient = orient->GetMatrix();
	}

	int parent_idx = (parent && parent->IsValid()) ? OBJ_INDEX(parent->objp) : -1;

	int obj_idx = weapon_create(&pos, real_orient, wclass, parent_idx, group);

	if(obj_idx > -1)
		return ade_set_args(L, "o", l_Weapon.Set(object_h(&Objects[obj_idx]), Objects[obj_idx].signature));
	else
		return ade_set_error(L, "o", l_Weapon.Set(object_h()));
}

ADE_FUNC(getMissionFilename, l_Mission, NULL, "Gets mission filename", "string", "Mission filename, or empty string if game is not in a mission")
{
	return ade_set_args(L, "s", Game_current_mission_filename);
}

ADE_FUNC(startMission, l_Mission, "[Filename or MISSION_* enumeration, Briefing = true]", "Starts the defined mission", "boolean", "True, or false if the function fails")
{
	bool b = true;
	char s[MAX_FILENAME_LEN];
	char *str = s;

	if(lua_isstring(L, 1))
	{
		if (!ade_get_args(L, "s|b", &str, &b))
			return ade_set_args(L, "b", false);

	} else {
		enum_h *e = NULL;

		if (!ade_get_args(L, "o|b", l_Enum.GetPtr(&e), &b))
			return ade_set_args(L, "b", false);

		if (e->index == LE_MISSION_REPEAT) {
			if (Num_recent_missions > 0)  {
				strncpy( s, Recent_missions[0], MAX_FILENAME_LEN );
			} else {
				return ade_set_args(L, "b", false);
			}
		} else {
			return ade_set_args(L, "b", false);
		}
	}

	// no filename... bail
	if (str == NULL)
		return ade_set_args(L, "b", false);

	// if mission name has extension... it needs to be removed...
	char *file_ext;

	file_ext = strrchr(str, '.');
	if (file_ext)
		*file_ext = 0;

	// game is in MP mode... or if the file does not exist... bail
	if ((Game_mode & GM_MULTIPLAYER) || (cf_exists_full(str, CF_TYPE_MISSIONS) != 0))
		return ade_set_args(L, "b", false);

	// mission is already running...
	if (Game_mode & GM_IN_MISSION) {
		// TO DO... All the things needed if this function is called in any state of the game while mission is running.
		//    most likely all require 'stricmp(str, Game_current_mission_filename)' to make sure missions arent mixed
		//    but after that it might be possible to imprement method for jumping directly into already running 
		//    missions.
		return ade_set_args(L, "b", false);
	// if mission is not running
	} else {
		// due safety checks of the game_start_mission() function allow only main menu for now.
		if (gameseq_get_state(gameseq_get_depth()) == GS_STATE_MAIN_MENU) {
			strcpy_s( Game_current_mission_filename, str );
			if (b == true) {
				// start mission - go via briefing screen
				gameseq_post_event(GS_EVENT_START_GAME);
			} else {
				// start mission - enter the game directly
				gameseq_post_event(GS_EVENT_START_GAME_QUICK);
			}
			return ade_set_args(L, "b", true);
		}
	}
	return ade_set_args(L, "b", false);
}

ADE_FUNC(getMissionTime, l_Mission, NULL, "Game time in seconds since the mission was started; is affected by time compression", "number", "Mission time (seconds), or 0 if game is not in a mission")
{
	if(!(Game_mode & GM_IN_MISSION))
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "x", Missiontime);
}

//WMC - These are in freespace.cpp
ADE_FUNC(loadMission, l_Mission, "Mission name", "Loads a mission", "boolean", "True if mission was loaded, otherwise false")
{
	char *s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "b", false);

	// clear post processing settings
	gr_post_process_set_defaults();

	//NOW do the loading stuff
	game_stop_time();
	get_mission_info(s, &The_mission, false);
	game_level_init();

	if(mission_load(s) == -1)
		return ADE_RETURN_FALSE;

	game_post_level_init();

	Game_mode |= GM_IN_MISSION;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(unloadMission, l_Mission, NULL, "Stops the current mission and unloads it", NULL, NULL)
{
	if(Game_mode & GM_IN_MISSION)
	{
		game_level_close();
		Game_mode &= ~GM_IN_MISSION;
		strcpy_s(Game_current_mission_filename, "");
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(simulateFrame, l_Mission, NULL, "Simulates mission frame", NULL, NULL)
{
	game_update_missiontime();
	game_simulation_frame();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(renderFrame, l_Mission, NULL, "Renders mission frame, but does not move anything", NULL, NULL)
{
	camid cid = game_render_frame_setup();
	game_render_frame( cid );
	game_render_post_frame();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(applyShudder, l_Mission, "number time, number intesity", "Applies a shudder effects to the camera. Time is in seconds. Intensity specifies the shudder effect strength, the Maxim has a value of 1440.", "boolean", "true if successfull, false otherwise")
{
	float time = -1.0f;
	float intensity = -1.0f;

	if (!ade_get_args(L, "ff", &time, &intensity))
		return ADE_RETURN_FALSE;

	if (time < 0.0f || intensity < 0.0f)
	{
		LuaError(L, "Illegal shudder values given. Must be bigger than zero, got time of %f and intensity of %f.", time, intensity);
		return ADE_RETURN_FALSE;
	}

	int int_time = fl2i(time * 1000.0f);

	game_shudder_apply(int_time, intensity * 0.01f);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(isInCampaign, l_Mission, NULL, "Get whether or not the current mission being played in a campaign (as opposed to the tech room's simulator)", "boolean", "true if in campaign, false if not")
{
	bool b = false;

	if (Game_mode & GM_CAMPAIGN_MODE) {
		b = true;
	}

	return ade_set_args(L, "b", b);
}

//**********LIBRARY: Tables
ade_lib l_Tables("Tables", NULL, "tb", "Tables library");

//*****SUBLIBRARY: Tables/ShipClasses
ade_lib l_Tables_ShipClasses("ShipClasses", &l_Tables, NULL, NULL);
ADE_INDEXER(l_Tables_ShipClasses, "number Index/string Name", "Array of ship classes", "shipclass", "Ship handle, or invalid ship handle if index is invalid")
{
	if(!ships_inited)
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	char *name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	int idx = ship_info_lookup(name);
	
	if(idx < 0) {
		idx = atoi(name);
		if(idx < 1 || idx >= static_cast<int>(Ship_info.size()))
			return ade_set_error(L, "o", l_Shipclass.Set(-1));

		idx--;	//Lua->FS2
	}

	return ade_set_args(L, "o", l_Shipclass.Set(idx));
}

ADE_FUNC(__len, l_Tables_ShipClasses, NULL, "Number of ship classes", "number", "Number of ship classes, or 0 if ship classes haven't been loaded yet")
{
	if(!ships_inited)
		return ade_set_args(L, "i", 0);	//No ships loaded...should be 0

	return ade_set_args(L, "i", Ship_info.size());
}

//*****SUBLIBRARY: Tables/WeaponClasses
ade_lib l_Tables_WeaponClasses("WeaponClasses", &l_Tables, NULL, NULL);

extern int Weapons_inited;

ADE_INDEXER(l_Tables_WeaponClasses, "number Index/string WeaponName", "Array of weapon classes", "weapon", "Weapon class handle, or invalid weaponclass handle if index is invalid")
{
	if(!Weapons_inited)
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	char *name;
	if(!ade_get_args(L, "*s", &name))
		return 0;

	int idx = weapon_info_lookup(name);
	
	if(idx < 0) {
		idx = atoi(name);

		// atoi is good enough here, 0 is invalid anyway
		if (idx > 0)
		{
			idx--; // Lua --> C/C++
		}
		else
		{
			return ade_set_args(L, "o", l_Weaponclass.Set(-1));
		}
	}

	return ade_set_args(L, "o", l_Weaponclass.Set(idx));
}

ADE_FUNC(__len, l_Tables_WeaponClasses, NULL, "Number of weapon classes", "number", "Number of weapon classes, or 0 if weapon classes haven't been loaded yet")
{
	if(!Weapons_inited)
		return ade_set_args(L, "i", 0);

	return ade_set_args(L, "i", Num_weapon_types);
}

//*************************Testing stuff*************************
//This section is for stuff that's considered experimental.
ade_lib l_Testing("Testing", NULL, "ts", "Experimental or testing stuff");

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

// Om_tracker_flag should already be set in FreeSpace.cpp, needed to determine if PXO is enabled from the registry
extern int Om_tracker_flag; // needed for FS2OpenPXO config

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

// *************************Helper functions*********************
//WMC - This should be used anywhere that an 'object' is set, so
//that scripters can get access to as much relevant data to that
//object as possible.
//It should also be updated as new types are added to Lua.
int ade_set_object_with_breed(lua_State *L, int obj_idx)
{
	if(obj_idx < 0 || obj_idx >= MAX_OBJECTS)
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	object *objp = &Objects[obj_idx];

	switch(objp->type)
	{
		case OBJ_SHIP:
			return ade_set_args(L, "o", l_Ship.Set(object_h(objp)));
		case OBJ_ASTEROID:
			return ade_set_args(L, "o", l_Asteroid.Set(object_h(objp)));
		case OBJ_DEBRIS:
			return ade_set_args(L, "o", l_Debris.Set(object_h(objp)));
		case OBJ_WAYPOINT:
			return ade_set_args(L, "o", l_Waypoint.Set(object_h(objp)));
		case OBJ_WEAPON:
			return ade_set_args(L, "o", l_Weapon.Set(object_h(objp)));
		case OBJ_BEAM:
			return ade_set_args(L, "o", l_Beam.Set(object_h(objp)));
		default:
			return ade_set_args(L, "o", l_Object.Set(object_h(objp)));
	}
}

//###########################################################
//########################<IMPORTANT>########################
//###########################################################
//If you are a coder who wants to add libraries, functions,
//or objects to Lua, then you want to be above this point.
//###########################################################
//########################</IMPORTANT>#######################
//###########################################################

// *************************Housekeeping*************************
//WMC - The miraculous lines of code that make Lua debugging worth something.
lua_Debug Ade_debug_info;
char debug_stack[4][32];

void ade_debug_ret(lua_State *L, lua_Debug *ar)
{
	Assert(L != NULL);
	Assert(ar != NULL);
	lua_getstack(L, 1, ar);
	lua_getinfo(L, "nSlu", ar);
	memcpy(&Ade_debug_info, ar, sizeof(lua_Debug));

	int n;
	for (n = 0; n < 4; n++) {
		debug_stack[n][0] = '\0';
	}

	for (n = 0; n < 4; n++) {
		if (lua_getstack(L,n+1, ar) == 0)
			break;
		lua_getinfo(L,"n", ar);
		if (ar->name == NULL)
			break;
		strcpy_s(debug_stack[n],ar->name);
	}
}

//WMC - because the behavior of the return keyword
//was changed, I now have to use this in hooks.
static int ade_return_hack(lua_State *L)
{
	int i = 0;
	int num = lua_gettop(L);
	for(i = 0; i < num; i++)
	{
		lua_pushvalue(L, i+1);
	}

	return num;
}

static void *vm_lua_alloc(void*, void *ptr, size_t, size_t nsize) {
	if (nsize == 0)
	{
		vm_free(ptr);
		return NULL;
	}
	else
	{
		return vm_realloc(ptr, nsize);
	}
}

//Inits LUA
//Note that "libraries" must end with a {NULL, NULL}
//element
int script_state::CreateLuaState()
{
	mprintf(("LUA: Opening LUA state...\n"));
	lua_State *L = lua_newstate(vm_lua_alloc, nullptr);

	if(L == NULL)
	{
		Warning(LOCATION, "Could not initialize Lua");
		return 0;
	}

	//*****INITIALIZE AUXILIARY LIBRARIES
	mprintf(("LUA: Initializing base Lua libraries...\n"));
	luaL_openlibs(L);

	//*****DISABLE DANGEROUS COMMANDS
	lua_pushstring(L, "os");
	lua_rawget(L, LUA_GLOBALSINDEX);
	int os_ldx = lua_gettop(L);
	if(lua_istable(L, os_ldx))
	{
		lua_pushstring(L, "execute");
		lua_pushnil(L);
		lua_rawset(L, os_ldx);
		lua_pushstring(L, "remove");
		lua_pushnil(L);
		lua_rawset(L, os_ldx);
		lua_pushstring(L, "rename");
		lua_pushnil(L);
		lua_rawset(L, os_ldx);
	}
	lua_pop(L, 1);	//os table

	//*****SET DEBUG HOOKS
#ifndef NDEBUG
	lua_sethook(L, ade_debug_ret, LUA_MASKRET, 0);
#endif

	//*****INITIALIZE ADE
	uint i;
	mprintf(("LUA: Beginning ADE initialization\n"));
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		//WMC - Do only toplevel table entries, doi
		if(ade_manager::getInstance()->getEntry(i).ParentIdx == UINT_MAX)			//WMC - oh hey, we're done with the meaty point in < 10 lines.
			ade_manager::getInstance()->getEntry(i).SetTable(L, LUA_GLOBALSINDEX, LUA_GLOBALSINDEX);	//Oh the miracles of OOP.
	}

	//*****INITIALIZE RETURN HACK FUNCTION
	lua_pushstring(L, "ade_return_hack");
	lua_pushboolean(L, 0);
	lua_pushcclosure(L, ade_return_hack, 2);
	lua_setglobal(L, "ade_return_hack");

	//*****INITIALIZE ENUMERATION CONSTANTS
	mprintf(("ADE: Initializing enumeration constants...\n"));
	enum_h eh;
	for(i = 0; i < Num_enumerations; i++)
	{
		eh.index = Enumerations[i].def;
		eh.is_constant = true;

		ade_set_args(L, "o", l_Enum.Set(eh));
		lua_setglobal(L, Enumerations[i].name);
	}

	//*****ASSIGN LUA SESSION
	mprintf(("ADE: Assigning Lua session...\n"));
	SetLuaSession(L);

//	(void)l_BitOps.GetName();

	return 1;
}

void script_state::EndLuaFrame()
{
	memcpy(NextDrawStringPos, NextDrawStringPosInitial, sizeof(NextDrawStringPos));
}

void ade_output_toc(FILE *fp, ade_table_entry *ate)
{
	Assert(fp != NULL);
	Assert(ate != NULL);

	//WMC - sanity checking
	if(ate->Name == NULL && ate->ShortName == NULL) {
		Warning(LOCATION, "Found ade_table_entry with no name or shortname");
		return;
	}

	fputs("<dd>", fp);

	if(ate->Name == NULL)
	{
		fprintf(fp, "<a href=\"#%s\">%s", ate->ShortName, ate->ShortName);
	}
	else
	{
		fprintf(fp, "<a href=\"#%s\">%s", ate->Name, ate->Name);
		if(ate->ShortName)
			fprintf(fp, " (%s)", ate->ShortName);
	}
	fputs("</a>", fp);

	if(ate->Description)
		fprintf(fp, " - %s\n", ate->Description);

	fputs("</dd>\n", fp);
}

static bool sort_table_entries(const ade_table_entry* left, const ade_table_entry* right) {
	const char* leftCmp = left->Name != nullptr ? left->Name : left->ShortName;
	const char* rightCmp = right->Name != nullptr ? right->Name : left->ShortName;

	if (leftCmp == nullptr) {
		return false;
	}
	if (rightCmp == nullptr) {
		return false;
	}

	SCP_string leftStr(leftCmp);
	std::transform(std::begin(leftStr), std::end(leftStr), std::begin(leftStr), ::tolower);

	SCP_string rightStr(rightCmp);
	std::transform(std::begin(rightStr), std::end(rightStr), std::begin(rightStr), ::tolower);

	return leftStr < rightStr;
}

static bool sort_doc_entries(const ade_table_entry* left, const ade_table_entry* right) {
	if (left->Instanced == right->Instanced) {
		// Same type -> compare as normal
		return sort_table_entries(left, right);
	}
	if (left->Instanced) {
		return true;
	}
	return false;
}

void script_state::OutputLuaMeta(FILE *fp)
{
	uint i;
	ade_table_entry *ate;
	fputs("<dl>\n", fp);

	SCP_vector<ade_table_entry*> table_entries;

	//***TOC: Libraries
	fputs("<dt><b>Libraries</b></dt>\n", fp);
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		ate = &ade_manager::getInstance()->getEntry(i);
		if(ate->ParentIdx == UINT_MAX && ate->Type == 'o' && ate->Instanced) {
			table_entries.push_back(ate);
		}
	}
	std::sort(std::begin(table_entries), std::end(table_entries), sort_table_entries);
	for (auto entry : table_entries) {
		ade_output_toc(fp, entry);
	}
	table_entries.clear();

	//***TOC: Objects
	fputs("<dt><b>Types</b></dt>\n", fp);
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		ate = &ade_manager::getInstance()->getEntry(i);
		if(ate->ParentIdx == UINT_MAX && ate->Type == 'o' && !ate->Instanced) {
			table_entries.push_back(ate);
		}
	}
	std::sort(std::begin(table_entries), std::end(table_entries), sort_table_entries);
	for (auto entry : table_entries) {
		ade_output_toc(fp, entry);
	}
	table_entries.clear();

	//***TOC: Enumerations
	fputs("<dt><b><a href=\"#Enumerations\">Enumerations</a></b></dt>", fp);

	//***End TOC
	fputs("</dl><br/><br/>", fp);

	//***Everything
	fputs("<dl>\n", fp);
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		ate = &ade_manager::getInstance()->getEntry(i);
		if(ate->ParentIdx == UINT_MAX)
			table_entries.push_back(ate);
	}

	std::sort(std::begin(table_entries), std::end(table_entries), sort_doc_entries);
	for (auto entry : table_entries) {
		entry->OutputMeta(fp);
	}
	table_entries.clear();

	//***Enumerations
	fprintf(fp, "<dt id=\"Enumerations\"><h2>Enumerations</h2></dt>");
	for(i = 0; i < Num_enumerations; i++)
	{
		//WMC - This is in case we ever want to add descriptions to enums.
		//fprintf(fp, "<dd><dl><dt><b>%s</b></dt><dd>%s</dd></dl></dd>", Enumerations[i].name, Enumerations[i].desc);

		//WMC - Otherwise, just use this.
		fprintf(fp, "<dd><b>%s</b></dd>", Enumerations[i].name);
	}
	fputs("</dl>\n", fp);

	//***End LUA
	fputs("</dl>\n", fp);
}
