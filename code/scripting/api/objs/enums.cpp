
#include "enums.h"

#include "scripting/ade.h"

namespace scripting {
namespace api {

//**********OBJECT: constant class
//WMC NOTE -
//While you can have enumeration indexes in any order, make sure
//that any new enumerations have indexes of NEXT INDEX (see below)
//or after. Don't forget to increment NEXT INDEX after you're done.
//=====================================
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
flag_def_list Enumerations[] = {
	{
		"ALPHABLEND_FILTER",
		LE_ALPHABLEND_FILTER,
		0
	},

	{
		"ALPHABLEND_NONE",
		LE_ALPHABLEND_NONE,
		0
	},

	{
		"CFILE_TYPE_NORMAL",
		LE_CFILE_TYPE_NORMAL,
		0
	},

	{
		"CFILE_TYPE_MEMORY_MAPPED",
		LE_CFILE_TYPE_MEMORY_MAPPED,
		0
	},

	{
		"MOUSE_LEFT_BUTTON",
		LE_MOUSE_LEFT_BUTTON,
		0
	},

	{
		"MOUSE_RIGHT_BUTTON",
		LE_MOUSE_RIGHT_BUTTON,
		0
	},

	{
		"MOUSE_MIDDLE_BUTTON",
		LE_MOUSE_MIDDLE_BUTTON,
		0
	},

	{
		"ORDER_ATTACK",
		LE_ORDER_ATTACK,
		0
	},

	{
		"ORDER_ATTACK_ANY",
		LE_ORDER_ATTACK_ANY,
		0
	},

	{
		"ORDER_DEPART",
		LE_ORDER_DEPART,
		0
	},

	{
		"ORDER_DISABLE",
		LE_ORDER_DISABLE,
		0
	},

	{
		"ORDER_DISARM",
		LE_ORDER_DISARM,
		0
	},

	{
		"ORDER_DOCK",
		LE_ORDER_DOCK,
		0
	},

	{
		"ORDER_EVADE",
		LE_ORDER_EVADE,
		0
	},

	{
		"ORDER_FLY_TO",
		LE_ORDER_FLY_TO,
		0
	},

	{
		"ORDER_FORM_ON_WING",
		LE_ORDER_FORM_ON_WING,
		0
	},

	{
		"ORDER_GUARD",
		LE_ORDER_GUARD,
		0
	},

	{
		"ORDER_IGNORE_SHIP",
		LE_ORDER_IGNORE,
		0
	},

	{
		"ORDER_KEEP_SAFE_DISTANCE",
		LE_ORDER_KEEP_SAFE_DISTANCE,
		0
	},

	{
		"ORDER_PLAY_DEAD",
		LE_ORDER_PLAY_DEAD,
		0
	},

	{
		"ORDER_REARM",
		LE_ORDER_REARM,
		0
	},

	{
		"ORDER_STAY_NEAR",
		LE_ORDER_STAY_NEAR,
		0
	},

	{
		"ORDER_STAY_STILL",
		LE_ORDER_STAY_STILL,
		0
	},

	{
		"ORDER_UNDOCK",
		LE_ORDER_UNDOCK,
		0
	},

	{
		"ORDER_WAYPOINTS",
		LE_ORDER_WAYPOINTS,
		0
	},

	{
		"ORDER_WAYPOINTS_ONCE",
		LE_ORDER_WAYPOINTS_ONCE,
		0
	},

	{
		"ORDER_ATTACK_WING",
		LE_ORDER_ATTACK_WING,
		0
	},

	{
		"ORDER_GUARD_WING",
		LE_ORDER_GUARD_WING,
		0
	},

	{
		"ORDER_ATTACK_SHIP_CLASS",
		LE_ORDER_ATTACK_SHIP_CLASS,
		0
	},

	{
		"PARTICLE_DEBUG",
		LE_PARTICLE_DEBUG,
		0
	},

	{
		"PARTICLE_BITMAP",
		LE_PARTICLE_BITMAP,
		0
	},

	{
		"PARTICLE_FIRE",
		LE_PARTICLE_FIRE,
		0
	},

	{
		"PARTICLE_SMOKE",
		LE_PARTICLE_SMOKE,
		0
	},

	{
		"PARTICLE_SMOKE2",
		LE_PARTICLE_SMOKE2,
		0
	},

	{
		"PARTICLE_PERSISTENT_BITMAP",
		LE_PARTICLE_PERSISTENT_BITMAP,
		0
	},

	{
		"SEXPVAR_CAMPAIGN_PERSISTENT",
		LE_SEXPVAR_CAMPAIGN_PERSISTENT,
		0
	},

	{
		"SEXPVAR_NOT_PERSISTENT",
		LE_SEXPVAR_NOT_PERSISTENT,
		0
	},

	{
		"SEXPVAR_PLAYER_PERSISTENT",
		LE_SEXPVAR_PLAYER_PERSISTENT,
		0
	},

	{
		"SEXPVAR_TYPE_NUMBER",
		LE_SEXPVAR_TYPE_NUMBER,
		0
	},

	{
		"SEXPVAR_TYPE_STRING",
		LE_SEXPVAR_TYPE_STRING,
		0
	},

	{
		"TEXTURE_STATIC",
		LE_TEXTURE_STATIC,
		0
	},

	{
		"TEXTURE_DYNAMIC",
		LE_TEXTURE_DYNAMIC,
		0
	},

	{
		"LOCK",
		LE_LOCK,
		0
	},

	{
		"UNLOCK",
		LE_UNLOCK,
		0
	},

	{
		"NONE",
		LE_NONE,
		0
	},

	{
		"SHIELD_FRONT",
		LE_SHIELD_FRONT,
		0
	},

	{
		"SHIELD_LEFT",
		LE_SHIELD_LEFT,
		0
	},

	{
		"SHIELD_RIGHT",
		LE_SHIELD_RIGHT,
		0
	},

	{
		"SHIELD_BACK",
		LE_SHIELD_BACK,
		0
	},

	{
		"MISSION_REPEAT",
		LE_MISSION_REPEAT,
		0
	},

	{
		"NORMAL_CONTROLS",
		LE_NORMAL_CONTROLS,
		0
	},

	{
		"LUA_STEERING_CONTROLS",
		LE_LUA_STEERING_CONTROLS,
		0
	},

	{
		"LUA_FULL_CONTROLS",
		LE_LUA_FULL_CONTROLS,
		0
	},

	{
		"NORMAL_BUTTON_CONTROLS",
		LE_NORMAL_BUTTON_CONTROLS,
		0
	},

	{
		"LUA_ADDITIVE_BUTTON_CONTROL",
		LE_LUA_ADDITIVE_BUTTON_CONTROL,
		0
	},

	{
		"LUA_OVERRIDE_BUTTON_CONTROL",
		LE_LUA_OVERRIDE_BUTTON_CONTROL,
		0
	},

	{
		"VM_INTERNAL",
		LE_VM_INTERNAL,
		0
	},

	{
		"VM_EXTERNAL",
		LE_VM_EXTERNAL,
		0
	},

	{
		"VM_TRACK",
		LE_VM_TRACK,
		0
	},

	{
		"VM_DEAD_VIEW",
		LE_VM_DEAD_VIEW,
		0
	},

	{
		"VM_CHASE",
		LE_VM_CHASE,
		0
	},

	{
		"VM_OTHER_SHIP",
		LE_VM_OTHER_SHIP,
		0
	},

	{
		"VM_EXTERNAL_CAMERA_LOCKED",
		LE_VM_EXTERNAL_CAMERA_LOCKED,
		0
	},

	{
		"VM_CAMERA_LOCKED",
		LE_VM_CAMERA_LOCKED,
		0
	},

	{
		"VM_WARP_CHASE",
		LE_VM_WARP_CHASE,
		0
	},

	{
		"VM_PADLOCK_UP",
		LE_VM_PADLOCK_UP,
		0
	},

	{
		"VM_PADLOCK_REAR",
		LE_VM_PADLOCK_REAR,
		0
	},

	{
		"VM_PADLOCK_LEFT",
		LE_VM_PADLOCK_LEFT,
		0
	},

	{
		"VM_PADLOCK_RIGHT",
		LE_VM_PADLOCK_RIGHT,
		0
	},

	{
		"VM_WARPIN_ANCHOR",
		LE_VM_WARPIN_ANCHOR,
		0
	},

	{
		"VM_TOPDOWN",
		LE_VM_TOPDOWN,
		0
	},

	{
		"VM_FREECAMERA",
		LE_VM_FREECAMERA,
		0
	},

	{
		"VM_CENTERING",
		LE_VM_CENTERING,
		0
	},

	{
		"MESSAGE_PRIORITY_LOW",
		LE_MESSAGE_PRIORITY_LOW,
		0
	},

	{
		"MESSAGE_PRIORITY_NORMAL",
		LE_MESSAGE_PRIORITY_NORMAL,
		0
	},

	{
		"MESSAGE_PRIORITY_HIGH",
		LE_MESSAGE_PRIORITY_HIGH,
		0
	},
};

//DO NOT FORGET to increment NEXT INDEX: !!!!!!!!!!!!!

size_t Num_enumerations = sizeof(Enumerations) / sizeof(flag_def_list);


enum_h::enum_h() {
	index = -1;
	is_constant = false;
}
enum_h::enum_h(int n_index) {
	index = n_index;
	is_constant = false;
}
bool enum_h::IsValid() const { return (index > -1 && index < ENUM_NEXT_INDEX); }

ADE_OBJ(l_Enum, enum_h, "enumeration", "Enumeration object");

ADE_FUNC(__newindex,
		 l_Enum,
		 "enumeration",
		 "Sets enumeration to specified value (if it is not a global",
		 "enumeration",
		 "enumeration") {
	enum_h* e1 = NULL, * e2 = NULL;
	if (!ade_get_args(L, "oo", l_Enum.GetPtr(&e1), l_Enum.GetPtr(&e2))) {
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));
	}

	if (!e1->is_constant) {
		e1->index = e2->index;
	}

	return ade_set_args(L, "o", l_Enum.Set(*e1));
}

ADE_FUNC(__tostring,
		 l_Enum,
		 NULL,
		 "Returns enumeration name",
		 "string",
		 "Enumeration name, or \"<INVALID>\" if invalid") {
	enum_h* e = NULL;
	if (!ade_get_args(L, "o", l_Enum.GetPtr(&e))) {
		return ade_set_args(L, "s", "<INVALID>");
	}

	if (e->index < 1 || e->index >= ENUM_NEXT_INDEX) {
		return ade_set_args(L, "s", "<INVALID>");
	}

	uint i;
	for (i = 0; i < Num_enumerations; i++) {
		if (Enumerations[i].def == e->index) {
			return ade_set_args(L, "s", Enumerations[i].name);
		}
	}

	return ade_set_args(L, "s", "<INVALID>");
}

ADE_FUNC(__eq,
		 l_Enum,
		 "enumeration",
		 "Compares the two enumerations for equality",
		 "boolean",
		 "true if equal, false otherwise") {
	enum_h* e1 = NULL;
	enum_h* e2 = NULL;

	if (!ade_get_args(L, "oo", l_Enum.GetPtr(&e1), l_Enum.GetPtr(&e2))) {
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));
	}

	if (e1 == NULL || e2 == NULL) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", e1->index == e2->index);
}

}
}
