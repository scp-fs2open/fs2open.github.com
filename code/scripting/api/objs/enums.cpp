
#include "enums.h"

#include "mission/missionparse.h"
#include "object/objectsnd.h"
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
// clang-format off
flag_def_list Enumerations[] = {
	{"ALPHABLEND_FILTER", LE_ALPHABLEND_FILTER, 0},
	{"ALPHABLEND_NONE", LE_ALPHABLEND_NONE, 0},
	{"CFILE_TYPE_NORMAL", LE_CFILE_TYPE_NORMAL, 0},
	{"CFILE_TYPE_MEMORY_MAPPED", LE_CFILE_TYPE_MEMORY_MAPPED, 0},
	{"MOUSE_LEFT_BUTTON", LE_MOUSE_LEFT_BUTTON, 0},
	{"MOUSE_RIGHT_BUTTON", LE_MOUSE_RIGHT_BUTTON, 0},
	{"MOUSE_MIDDLE_BUTTON", LE_MOUSE_MIDDLE_BUTTON, 0},
	{"ORDER_ATTACK", LE_ORDER_ATTACK, 0},
	{"ORDER_ATTACK_ANY", LE_ORDER_ATTACK_ANY, 0},
	{"ORDER_DEPART", LE_ORDER_DEPART, 0},
	{"ORDER_DISABLE", LE_ORDER_DISABLE, 0},
	{"ORDER_DISARM", LE_ORDER_DISARM, 0},
	{"ORDER_DOCK", LE_ORDER_DOCK, 0},
	{"ORDER_EVADE", LE_ORDER_EVADE, 0},
	{"ORDER_FLY_TO", LE_ORDER_FLY_TO, 0},
	{"ORDER_FORM_ON_WING", LE_ORDER_FORM_ON_WING, 0},
	{"ORDER_GUARD", LE_ORDER_GUARD, 0},
	{"ORDER_IGNORE_SHIP", LE_ORDER_IGNORE, 0},
	{"ORDER_KEEP_SAFE_DISTANCE", LE_ORDER_KEEP_SAFE_DISTANCE, 0},
	{"ORDER_PLAY_DEAD", LE_ORDER_PLAY_DEAD, 0},
	{"ORDER_PLAY_DEAD_PERSISTENT", LE_ORDER_PLAY_DEAD_PERSISTENT, 0},
	{"ORDER_REARM", LE_ORDER_REARM, 0},
	{"ORDER_STAY_NEAR", LE_ORDER_STAY_NEAR, 0},
	{"ORDER_STAY_STILL", LE_ORDER_STAY_STILL, 0},
	{"ORDER_UNDOCK", LE_ORDER_UNDOCK, 0},
	{"ORDER_WAYPOINTS", LE_ORDER_WAYPOINTS, 0},
	{"ORDER_WAYPOINTS_ONCE", LE_ORDER_WAYPOINTS_ONCE, 0},
	{"ORDER_ATTACK_WING", LE_ORDER_ATTACK_WING, 0},
	{"ORDER_GUARD_WING", LE_ORDER_GUARD_WING, 0},
	{"ORDER_ATTACK_SHIP_CLASS", LE_ORDER_ATTACK_SHIP_CLASS, 0},
	{"PARTICLE_DEBUG", LE_PARTICLE_DEBUG, 0},
	{"PARTICLE_BITMAP", LE_PARTICLE_BITMAP, 0},
	{"PARTICLE_FIRE", LE_PARTICLE_FIRE, 0},
	{"PARTICLE_SMOKE", LE_PARTICLE_SMOKE, 0},
	{"PARTICLE_SMOKE2", LE_PARTICLE_SMOKE2, 0},
	{"PARTICLE_PERSISTENT_BITMAP", LE_PARTICLE_PERSISTENT_BITMAP, 0},
	{"SEXPVAR_CAMPAIGN_PERSISTENT", LE_SEXPVAR_CAMPAIGN_PERSISTENT, 0},
	{"SEXPVAR_NOT_PERSISTENT", LE_SEXPVAR_NOT_PERSISTENT, 0},
	{"SEXPVAR_PLAYER_PERSISTENT", LE_SEXPVAR_PLAYER_PERSISTENT, 0},
	{"SEXPVAR_TYPE_NUMBER", LE_SEXPVAR_TYPE_NUMBER, 0},
	{"SEXPVAR_TYPE_STRING", LE_SEXPVAR_TYPE_STRING, 0},
	{"TEXTURE_STATIC", LE_TEXTURE_STATIC, 0},
	{"TEXTURE_DYNAMIC", LE_TEXTURE_DYNAMIC, 0},
	{"LOCK", LE_LOCK, 0},
	{"UNLOCK", LE_UNLOCK, 0},
	{"NONE", LE_NONE, 0},
	{"SHIELD_FRONT", LE_SHIELD_FRONT, 0},
	{"SHIELD_LEFT", LE_SHIELD_LEFT, 0},
	{"SHIELD_RIGHT", LE_SHIELD_RIGHT, 0},
	{"SHIELD_BACK", LE_SHIELD_BACK, 0},
	{"MISSION_REPEAT", LE_MISSION_REPEAT, 0},
	{"NORMAL_CONTROLS", LE_NORMAL_CONTROLS, 0},
	{"LUA_STEERING_CONTROLS", LE_LUA_STEERING_CONTROLS, 0},
	{"LUA_FULL_CONTROLS", LE_LUA_FULL_CONTROLS, 0},
	{"NORMAL_BUTTON_CONTROLS", LE_NORMAL_BUTTON_CONTROLS, 0},
	{"LUA_ADDITIVE_BUTTON_CONTROL", LE_LUA_ADDITIVE_BUTTON_CONTROL, 0},
	{"LUA_OVERRIDE_BUTTON_CONTROL", LE_LUA_OVERRIDE_BUTTON_CONTROL, 0},
	{"VM_INTERNAL", LE_VM_INTERNAL, 0},
	{"VM_EXTERNAL", LE_VM_EXTERNAL, 0},
	{"VM_TRACK", LE_VM_TRACK, 0},
	{"VM_DEAD_VIEW", LE_VM_DEAD_VIEW, 0},
	{"VM_CHASE", LE_VM_CHASE, 0},
	{"VM_OTHER_SHIP", LE_VM_OTHER_SHIP, 0},
	{"VM_EXTERNAL_CAMERA_LOCKED", LE_VM_EXTERNAL_CAMERA_LOCKED, 0},
	{"VM_CAMERA_LOCKED", LE_VM_CAMERA_LOCKED, 0},
	{"VM_WARP_CHASE", LE_VM_WARP_CHASE, 0},
	{"VM_PADLOCK_UP", LE_VM_PADLOCK_UP, 0},
	{"VM_PADLOCK_REAR", LE_VM_PADLOCK_REAR, 0},
	{"VM_PADLOCK_LEFT", LE_VM_PADLOCK_LEFT, 0},
	{"VM_PADLOCK_RIGHT", LE_VM_PADLOCK_RIGHT, 0},
	{"VM_WARPIN_ANCHOR", LE_VM_WARPIN_ANCHOR, 0},
	{"VM_TOPDOWN", LE_VM_TOPDOWN, 0},
	{"VM_FREECAMERA", LE_VM_FREECAMERA, 0},
	{"VM_CENTERING", LE_VM_CENTERING, 0},
	{"MESSAGE_PRIORITY_LOW", LE_MESSAGE_PRIORITY_LOW, 0},
	{"MESSAGE_PRIORITY_NORMAL", LE_MESSAGE_PRIORITY_NORMAL, 0},
	{"MESSAGE_PRIORITY_HIGH", LE_MESSAGE_PRIORITY_HIGH, 0},
	{"OPTION_TYPE_SELECTION", LE_OPTION_TYPE_SELECTION, 0},
	{"OPTION_TYPE_RANGE", LE_OPTION_TYPE_RANGE, 0},
	{"AUDIOSTREAM_EVENTMUSIC", LE_ASF_EVENTMUSIC, 0},
	{"AUDIOSTREAM_MENUMUSIC", LE_ASF_MENUMUSIC, 0},
	{"AUDIOSTREAM_VOICE", LE_ASF_VOICE, 0},
	{"CONTEXT_VALID", LE_CONTEXT_VALID, 0},
	{"CONTEXT_SUSPENDED", LE_CONTEXT_SUSPENDED, 0},
	{"CONTEXT_INVALID", LE_CONTEXT_INVALID, 0},
	{"FIREBALL_MEDIUM_EXPLOSION", LE_FIREBALL_MEDIUM_EXPLOSION, 0},
	{"FIREBALL_LARGE_EXPLOSION", LE_FIREBALL_LARGE_EXPLOSION, 0},
	{"FIREBALL_WARP_EFFECT", LE_FIREBALL_WARP_EFFECT, 0},
	{"GR_RESIZE_NONE", LE_GR_RESIZE_NONE, 0},
	{"GR_RESIZE_FULL", LE_GR_RESIZE_FULL, 0},
	{"GR_RESIZE_FULL_CENTER", LE_GR_RESIZE_FULL_CENTER, 0},
	{"GR_RESIZE_MENU", LE_GR_RESIZE_MENU, 0},
	{"GR_RESIZE_MENU_ZOOMED", LE_GR_RESIZE_MENU_ZOOMED, 0},
	{"GR_RESIZE_MENU_NO_OFFSET", LE_GR_RESIZE_MENU_NO_OFFSET, 0},
	// the following OS_ definitions use bitfield values, not the indexes in enums.h
	{"OS_NONE", 0, 0},
	{"OS_MAIN", OS_MAIN, 0},
	{"OS_ENGINE", OS_ENGINE, 0},
	{"OS_TURRET_BASE_ROTATION", OS_TURRET_BASE_ROTATION, 0},
	{"OS_TURRET_GUN_ROTATION", OS_TURRET_GUN_ROTATION, 0},
	{"OS_SUBSYS_ALIVE", OS_SUBSYS_ALIVE, 0},
	{"OS_SUBSYS_DEAD", OS_SUBSYS_DEAD, 0},
	{"OS_SUBSYS_DAMAGED", OS_SUBSYS_DAMAGED, 0},
	{"OS_SUBSYS_ROTATION", OS_SUBSYS_ROTATION, 0},
	{"OS_PLAY_ON_PLAYER", OS_PLAY_ON_PLAYER, 0},
	{"OS_LOOPING_DISABLED", OS_LOOPING_DISABLED, 0},
	// end of OS_ definitions
	{ "MOVIE_PRE_FICTION", LE_MOVIE_PRE_FICTION, 0 },
	{ "MOVIE_PRE_CMD_BRIEF", LE_MOVIE_PRE_CMD_BRIEF, 0 },
	{ "MOVIE_PRE_BRIEF", LE_MOVIE_PRE_BRIEF, 0 },
	{ "MOVIE_PRE_GAME", LE_MOVIE_PRE_GAME, 0 },
	{ "MOVIE_PRE_DEBRIEF", LE_MOVIE_PRE_DEBRIEF, 0 },
	{ "MOVIE_POST_DEBRIEF", LE_MOVIE_POST_DEBRIEF, 0 },
	{ "MOVIE_END_CAMPAIGN", LE_MOVIE_END_CAMPAIGN, 0 },
	{"TBOX_FLASH_NAME", LE_TBOX_FLASH_NAME, 0},
	{"TBOX_FLASH_CARGO", LE_TBOX_FLASH_CARGO, 0},
	{"TBOX_FLASH_HULL", LE_TBOX_FLASH_HULL, 0},
	{"TBOX_FLASH_STATUS", LE_TBOX_FLASH_STATUS, 0},
	{"TBOX_FLASH_SUBSYS", LE_TBOX_FLASH_SUBSYS, 0},
	{"LUAAI_ACHIEVABLE", LE_LUAAI_ACHIEVABLE, 0},
	{"LUAAI_NOT_YET_ACHIEVABLE", LE_LUAAI_NOT_YET_ACHIEVABLE, 0},
	{"LUAAI_UNACHIEVABLE", LE_LUAAI_UNACHIEVABLE, 0},
	{"SCORE_BRIEFING", LE_SCORE_BRIEFING, 0},
	{"SCORE_DEBRIEFING_SUCCESS", LE_SCORE_DEBRIEFING_SUCCESS, 0},
	{"SCORE_DEBRIEFING_AVERAGE", LE_SCORE_DEBRIEFING_AVERAGE, 0},
	{"SCORE_DEBRIEFING_FAILURE", LE_SCORE_DEBRIEFING_FAILURE, 0},
	{"SCORE_FICTION_VIEWER", LE_SCORE_FICTION_VIEWER, 0},
	{"NOT_YET_PRESENT", LE_NOT_YET_PRESENT, 0},
	{"PRESENT", LE_PRESENT, 0},
	{"EXITED", LE_EXITED, 0},
};
// clang-format on

//DO NOT FORGET to increment NEXT INDEX: !!!!!!!!!!!!!

size_t Num_enumerations = sizeof(Enumerations) / sizeof(flag_def_list);


enum_h::enum_h() {
	index = -1;
	is_constant = false;
}
enum_h::enum_h(int n_index)
{
	index = n_index;
	is_constant = false;
}
SCP_string enum_h::getName() const
{
	for (size_t i = 0; i < Num_enumerations; i++) {
		if (Enumerations[i].def == index) {
			return Enumerations[i].name;
		}
	}

	return SCP_string();
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

	const auto name = e->getName();
	if (name.empty()) {
		return ade_set_args(L, "s", "<INVALID>");
	}
	return ade_set_args(L, "s", name);
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

ADE_VIRTVAR(IntValue, l_Enum, "enumeration", "Internal value of the enum.  Probably not useful unless this enum is a bitfield or corresponds to a #define somewhere else in the source code.", "number", "Integer (index) value of the enum")
{
	enum_h* e = nullptr;
	if (!ade_get_args(L, "o", l_Enum.GetPtr(&e))) {
		return ade_set_args(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "IntValue is read only!");
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "i", e->index);
}

}
}
