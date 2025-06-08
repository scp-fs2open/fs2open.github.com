
#include "enums.h"

#include "io/mouse.h"
#include "mission/missionparse.h"
#include "object/objectsnd.h"
#include "scripting/ade.h"

#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"

namespace scripting {
namespace api {

const lua_enum_def_list Enumerations[] = {
	{"ALPHABLEND_FILTER", LE_ALPHABLEND_FILTER, true},
	{"ALPHABLEND_NONE", LE_ALPHABLEND_NONE, true},
	{"CFILE_TYPE_NORMAL", LE_CFILE_TYPE_NORMAL, true},
	{"CFILE_TYPE_MEMORY_MAPPED", LE_CFILE_TYPE_MEMORY_MAPPED, true},
	{"MOUSE_LEFT_BUTTON", LE_MOUSE_LEFT_BUTTON, MOUSE_LEFT_BUTTON, true},
	{"MOUSE_RIGHT_BUTTON", LE_MOUSE_RIGHT_BUTTON, MOUSE_RIGHT_BUTTON, true},
	{"MOUSE_MIDDLE_BUTTON", LE_MOUSE_MIDDLE_BUTTON, MOUSE_MIDDLE_BUTTON, true},
	{"MOUSE_X1_BUTTON", LE_MOUSE_X1_BUTTON, MOUSE_X1_BUTTON, true},
	{"MOUSE_X2_BUTTON", LE_MOUSE_X2_BUTTON, MOUSE_X2_BUTTON, true},
	{"FLIGHTMODE_FLIGHTCURSOR", LE_FLIGHTMODE_FLIGHTCURSOR, true},
	{"FLIGHTMODE_SHIPLOCKED", LE_FLIGHTMODE_SHIPLOCKED, true},
	{"ORDER_ATTACK", LE_ORDER_ATTACK, true},
	{"ORDER_ATTACK_WING", LE_ORDER_ATTACK_WING, true},
	{"ORDER_ATTACK_SHIP_CLASS", LE_ORDER_ATTACK_SHIP_CLASS, true},
	{"ORDER_ATTACK_ANY", LE_ORDER_ATTACK_ANY, true},
	{"ORDER_DEPART", LE_ORDER_DEPART, true},
	{"ORDER_DISABLE", LE_ORDER_DISABLE, true},
	{"ORDER_DISABLE_TACTICAL", LE_ORDER_DISABLE_TACTICAL, true},
	{"ORDER_DISARM", LE_ORDER_DISARM, true},
	{"ORDER_DISARM_TACTICAL", LE_ORDER_DISARM_TACTICAL, true},
	{"ORDER_DOCK", LE_ORDER_DOCK, true},
	{"ORDER_EVADE", LE_ORDER_EVADE, true},
	{"ORDER_FLY_TO", LE_ORDER_FLY_TO, true},
	{"ORDER_FORM_ON_WING", LE_ORDER_FORM_ON_WING, true},
	{"ORDER_GUARD", LE_ORDER_GUARD, true},
	{"ORDER_GUARD_WING", LE_ORDER_GUARD_WING, true},
	{"ORDER_IGNORE_SHIP", LE_ORDER_IGNORE, true},
	{"ORDER_IGNORE_SHIP_NEW", LE_ORDER_IGNORE_NEW, true},
	{"ORDER_KEEP_SAFE_DISTANCE", LE_ORDER_KEEP_SAFE_DISTANCE, true},
	{"ORDER_PLAY_DEAD", LE_ORDER_PLAY_DEAD, true},
	{"ORDER_PLAY_DEAD_PERSISTENT", LE_ORDER_PLAY_DEAD_PERSISTENT, true},
	{"ORDER_REARM", LE_ORDER_REARM, true},
	{"ORDER_STAY_NEAR", LE_ORDER_STAY_NEAR, true},
	{"ORDER_STAY_STILL", LE_ORDER_STAY_STILL, true},
	{"ORDER_UNDOCK", LE_ORDER_UNDOCK, true},
	{"ORDER_WAYPOINTS", LE_ORDER_WAYPOINTS, true},
	{"ORDER_WAYPOINTS_ONCE", LE_ORDER_WAYPOINTS_ONCE, true},
	{"ORDER_LUA", LE_ORDER_LUA, true},
	{"PARTICLE_DEBUG", LE_PARTICLE_DEBUG, true},
	{"PARTICLE_BITMAP", LE_PARTICLE_BITMAP, true},
	{"PARTICLE_FIRE", LE_PARTICLE_FIRE, true},
	{"PARTICLE_SMOKE", LE_PARTICLE_SMOKE, true},
	{"PARTICLE_SMOKE2", LE_PARTICLE_SMOKE2, true},
	{"PARTICLE_PERSISTENT_BITMAP", LE_PARTICLE_PERSISTENT_BITMAP, true},
	{"SEXPVAR_CAMPAIGN_PERSISTENT", LE_SEXPVAR_CAMPAIGN_PERSISTENT, true},
	{"SEXPVAR_NOT_PERSISTENT", LE_SEXPVAR_NOT_PERSISTENT, true},
	{"SEXPVAR_PLAYER_PERSISTENT", LE_SEXPVAR_PLAYER_PERSISTENT, true},
	{"SEXPVAR_TYPE_NUMBER", LE_SEXPVAR_TYPE_NUMBER, true},
	{"SEXPVAR_TYPE_STRING", LE_SEXPVAR_TYPE_STRING, true},
	{"TEXTURE_STATIC", LE_TEXTURE_STATIC, true},
	{"TEXTURE_DYNAMIC", LE_TEXTURE_DYNAMIC, true},
	{"LOCK", LE_LOCK, true},
	{"UNLOCK", LE_UNLOCK, true},
	{"NONE", LE_NONE, true},
	{"SHIELD_FRONT", LE_SHIELD_FRONT, true},
	{"SHIELD_LEFT", LE_SHIELD_LEFT, true},
	{"SHIELD_RIGHT", LE_SHIELD_RIGHT, true},
	{"SHIELD_BACK", LE_SHIELD_BACK, true},
	{"MISSION_REPEAT", LE_MISSION_REPEAT, true},
	{"NORMAL_CONTROLS", LE_NORMAL_CONTROLS, true},
	{"LUA_STEERING_CONTROLS", LE_LUA_STEERING_CONTROLS, true},
	{"LUA_FULL_CONTROLS", LE_LUA_FULL_CONTROLS, true},
	{"NORMAL_BUTTON_CONTROLS", LE_NORMAL_BUTTON_CONTROLS, true},
	{"LUA_ADDITIVE_BUTTON_CONTROL", LE_LUA_ADDITIVE_BUTTON_CONTROL, true},
	{"LUA_OVERRIDE_BUTTON_CONTROL", LE_LUA_OVERRIDE_BUTTON_CONTROL, true},
	{"VM_INTERNAL", LE_VM_INTERNAL, true},
	{"VM_EXTERNAL", LE_VM_EXTERNAL, true},
	{"VM_TRACK", LE_VM_TRACK, true},
	{"VM_DEAD_VIEW", LE_VM_DEAD_VIEW, true},
	{"VM_CHASE", LE_VM_CHASE, true},
	{"VM_OTHER_SHIP", LE_VM_OTHER_SHIP, true},
	{"VM_EXTERNAL_CAMERA_LOCKED", LE_VM_EXTERNAL_CAMERA_LOCKED, true},
	{"VM_CAMERA_LOCKED", LE_VM_CAMERA_LOCKED, true},
	{"VM_WARP_CHASE", LE_VM_WARP_CHASE, true},
	{"VM_PADLOCK_UP", LE_VM_PADLOCK_UP, true},
	{"VM_PADLOCK_REAR", LE_VM_PADLOCK_REAR, true},
	{"VM_PADLOCK_LEFT", LE_VM_PADLOCK_LEFT, true},
	{"VM_PADLOCK_RIGHT", LE_VM_PADLOCK_RIGHT, true},
	{"VM_WARPIN_ANCHOR", LE_VM_WARPIN_ANCHOR, true},
	{"VM_TOPDOWN", LE_VM_TOPDOWN, true},
	{"VM_FREECAMERA", LE_VM_FREECAMERA, true},
	{"VM_CENTERING", LE_VM_CENTERING, true},
	{"MESSAGE_PRIORITY_LOW", LE_MESSAGE_PRIORITY_LOW, true},
	{"MESSAGE_PRIORITY_NORMAL", LE_MESSAGE_PRIORITY_NORMAL, true},
	{"MESSAGE_PRIORITY_HIGH", LE_MESSAGE_PRIORITY_HIGH, true},
	{"OPTION_TYPE_SELECTION", LE_OPTION_TYPE_SELECTION, true},
	{"OPTION_TYPE_RANGE", LE_OPTION_TYPE_RANGE, true},
	{"AUDIOSTREAM_EVENTMUSIC", LE_ASF_EVENTMUSIC, true},
	{"AUDIOSTREAM_MENUMUSIC", LE_ASF_MENUMUSIC, true},
	{"AUDIOSTREAM_VOICE", LE_ASF_VOICE, true},
	{"CONTEXT_VALID", LE_CONTEXT_VALID, true},
	{"CONTEXT_SUSPENDED", LE_CONTEXT_SUSPENDED, true},
	{"CONTEXT_INVALID", LE_CONTEXT_INVALID, true},
	{"FIREBALL_MEDIUM_EXPLOSION", LE_FIREBALL_MEDIUM_EXPLOSION, true},
	{"FIREBALL_LARGE_EXPLOSION", LE_FIREBALL_LARGE_EXPLOSION, true},
	{"FIREBALL_WARP_EFFECT", LE_FIREBALL_WARP_EFFECT, true},
	{"GR_RESIZE_NONE", LE_GR_RESIZE_NONE, true},
	{"GR_RESIZE_FULL", LE_GR_RESIZE_FULL, true},
	{"GR_RESIZE_FULL_CENTER", LE_GR_RESIZE_FULL_CENTER, true},
	{"GR_RESIZE_MENU", LE_GR_RESIZE_MENU, true},
	{"GR_RESIZE_MENU_ZOOMED", LE_GR_RESIZE_MENU_ZOOMED, true},
	{"GR_RESIZE_MENU_NO_OFFSET", LE_GR_RESIZE_MENU_NO_OFFSET, true},
	{"OS_NONE", LE_OS_NONE, 0, true},
	{"OS_MAIN", LE_OS_MAIN, OS_MAIN, true},
	{"OS_ENGINE", LE_OS_ENGINE, OS_ENGINE, true},
	{"OS_TURRET_BASE_ROTATION", LE_OS_TURRET_BASE_ROTATION, OS_TURRET_BASE_ROTATION, true},
	{"OS_TURRET_GUN_ROTATION", LE_OS_TURRET_GUN_ROTATION, OS_TURRET_GUN_ROTATION, true},
	{"OS_SUBSYS_ALIVE", LE_OS_SUBSYS_ALIVE, OS_SUBSYS_ALIVE, true},
	{"OS_SUBSYS_DEAD", LE_OS_SUBSYS_DEAD, OS_SUBSYS_DEAD, true},
	{"OS_SUBSYS_DAMAGED", LE_OS_SUBSYS_DAMAGED, OS_SUBSYS_DAMAGED, true},
	{"OS_SUBSYS_ROTATION", LE_OS_SUBSYS_ROTATION, OS_SUBSYS_ROTATION, true},
	{"OS_PLAY_ON_PLAYER", LE_OS_PLAY_ON_PLAYER, OS_PLAY_ON_PLAYER, true},
	{"OS_LOOPING_DISABLED", LE_OS_LOOPING_DISABLED, OS_LOOPING_DISABLED, true},
	{ "MOVIE_PRE_FICTION", LE_MOVIE_PRE_FICTION, true },
	{ "MOVIE_PRE_CMD_BRIEF", LE_MOVIE_PRE_CMD_BRIEF, true },
	{ "MOVIE_PRE_BRIEF", LE_MOVIE_PRE_BRIEF, true },
	{ "MOVIE_PRE_GAME", LE_MOVIE_PRE_GAME, true },
	{ "MOVIE_PRE_DEBRIEF", LE_MOVIE_PRE_DEBRIEF, true },
	{ "MOVIE_POST_DEBRIEF", LE_MOVIE_POST_DEBRIEF, true },
	{ "MOVIE_END_CAMPAIGN", LE_MOVIE_END_CAMPAIGN, true },
	{"TBOX_FLASH_NAME", LE_TBOX_FLASH_NAME, true},
	{"TBOX_FLASH_CARGO", LE_TBOX_FLASH_CARGO, true},
	{"TBOX_FLASH_HULL", LE_TBOX_FLASH_HULL, true},
	{"TBOX_FLASH_STATUS", LE_TBOX_FLASH_STATUS, true},
	{"TBOX_FLASH_SUBSYS", LE_TBOX_FLASH_SUBSYS, true},
	{"LUAAI_ACHIEVABLE", LE_LUAAI_ACHIEVABLE, true},
	{"LUAAI_NOT_YET_ACHIEVABLE", LE_LUAAI_NOT_YET_ACHIEVABLE, true},
	{"LUAAI_UNACHIEVABLE", LE_LUAAI_UNACHIEVABLE, true},
	{"SCORE_BRIEFING", LE_SCORE_BRIEFING, true},
	{"SCORE_DEBRIEFING_SUCCESS", LE_SCORE_DEBRIEFING_SUCCESS, true},
	{"SCORE_DEBRIEFING_AVERAGE", LE_SCORE_DEBRIEFING_AVERAGE, true},
	{"SCORE_DEBRIEFING_FAILURE", LE_SCORE_DEBRIEFING_FAILURE, true},
	{"SCORE_FICTION_VIEWER", LE_SCORE_FICTION_VIEWER, true},
	{"INVALID", LE_INVALID, true},
	{"NOT_YET_PRESENT", LE_NOT_YET_PRESENT, true},
	{"PRESENT", LE_PRESENT, true},
	{"DEATH_ROLL", LE_DEATH_ROLL, true},
	{"EXITED", LE_EXITED, true},
	{"DC_IS_HULL", LE_DC_IS_HULL, (1 << 0), true},
	{"DC_VAPORIZE", LE_DC_VAPORIZE, (1 << 1), true},
	{"DC_SET_VELOCITY", LE_DC_SET_VELOCITY, (1 << 2), true},
	{"DC_FIRE_HOOK", LE_DC_FIRE_HOOK, (1 << 3), true},
	{"RPC_SERVER", LE_RPC_SERVER, true},
	{"RPC_CLIENTS", LE_RPC_CLIENTS, true},
	{"RPC_BOTH", LE_RPC_BOTH, true},
	{"RPC_RELIABLE", LE_RPC_RELIABLE, true},
	{"RPC_ORDERED", LE_RPC_ORDERED, true},
	{"RPC_UNRELIABLE", LE_RPC_UNRELIABLE, true},
	{"HOTKEY_LINE_NONE", LE_HOTKEY_LINE_NONE, true},
	{"HOTKEY_LINE_HEADING", LE_HOTKEY_LINE_HEADING, true},
	{"HOTKEY_LINE_WING", LE_HOTKEY_LINE_WING, true},
	{"HOTKEY_LINE_SHIP", LE_HOTKEY_LINE_SHIP, true},
	{"HOTKEY_LINE_SUBSHIP", LE_HOTKEY_LINE_SUBSHIP, true},
	{"SCROLLBACK_SOURCE_COMPUTER", LE_SCROLLBACK_SOURCE_COMPUTER, true},
	{"SCROLLBACK_SOURCE_TRAINING", LE_SCROLLBACK_SOURCE_TRAINING, true},
	{"SCROLLBACK_SOURCE_HIDDEN", LE_SCROLLBACK_SOURCE_HIDDEN, true},
	{"SCROLLBACK_SOURCE_IMPORTANT", LE_SCROLLBACK_SOURCE_IMPORTANT, true},
	{"SCROLLBACK_SOURCE_FAILED", LE_SCROLLBACK_SOURCE_FAILED, true},
	{"SCROLLBACK_SOURCE_SATISFIED", LE_SCROLLBACK_SOURCE_SATISFIED, true},
	{"SCROLLBACK_SOURCE_COMMAND", LE_SCROLLBACK_SOURCE_COMMAND, true},
	{"SCROLLBACK_SOURCE_NETPLAYER", LE_SCROLLBACK_SOURCE_NETPLAYER, true},
	{"MULTI_TYPE_COOP", LE_MULTI_TYPE_COOP, true},
	{"MULTI_TYPE_TEAM", LE_MULTI_TYPE_TEAM, true},
	{"MULTI_TYPE_DOGFIGHT", LE_MULTI_TYPE_DOGFIGHT, true},
	{"MULTI_TYPE_SQUADWAR", LE_MULTI_TYPE_SQUADWAR, true},
	{"MULTI_OPTION_RANK", LE_MULTI_OPTION_RANK, true},
	{"MULTI_OPTION_LEAD", LE_MULTI_OPTION_LEAD, true},
	{"MULTI_OPTION_ANY", LE_MULTI_OPTION_ANY, true},
	{"MULTI_OPTION_HOST", LE_MULTI_OPTION_HOST, true},
	{"MULTI_GAME_TYPE_OPEN", LE_MULTI_GAME_TYPE_OPEN, true},
	{"MULTI_GAME_TYPE_PASSWORD", LE_MULTI_GAME_TYPE_PASSWORD, true},
	{"MULTI_GAME_TYPE_RANK_ABOVE", LE_MULTI_GAME_TYPE_RANK_ABOVE, true},
	{"MULTI_GAME_TYPE_RANK_BELOW", LE_MULTI_GAME_TYPE_RANK_BELOW, true},
	{"SEXP_TRUE", LE_SEXP_TRUE, SEXP_TRUE, true},
	{"SEXP_FALSE", LE_SEXP_FALSE, SEXP_FALSE, true},
	{"SEXP_KNOWN_FALSE", LE_SEXP_KNOWN_FALSE, SEXP_KNOWN_FALSE, true},
	{"SEXP_KNOWN_TRUE", LE_SEXP_KNOWN_TRUE, SEXP_KNOWN_TRUE, true},
	{"SEXP_UNKNOWN", LE_SEXP_UNKNOWN, SEXP_UNKNOWN, true},
	{"SEXP_NAN", LE_SEXP_NAN, SEXP_NAN, true},
	{"SEXP_NAN_FOREVER", LE_SEXP_NAN_FOREVER, SEXP_NAN_FOREVER, true},
	{"SEXP_CANT_EVAL", LE_SEXP_CANT_EVAL, SEXP_CANT_EVAL, true},
	{"COMMIT_SUCCESS", LE_COMMIT_SUCCESS, true},
	{"COMMIT_FAIL", LE_COMMIT_FAIL, true},
	{"COMMIT_PLAYER_NO_WEAPONS", LE_COMMIT_PLAYER_NO_WEAPONS, true},
	{"COMMIT_NO_REQUIRED_WEAPON", LE_COMMIT_NO_REQUIRED_WEAPON, true},
	{"COMMIT_NO_REQUIRED_WEAPON_MULTIPLE", LE_COMMIT_NO_REQUIRED_WEAPON_MULTIPLE, true},
	{"COMMIT_BANK_GAP_ERROR", LE_COMMIT_BANK_GAP_ERROR, true},
	{"COMMIT_PLAYER_NO_SLOT", LE_COMMIT_PLAYER_NO_SLOT, true},
	{"COMMIT_MULTI_PLAYERS_NO_SHIPS", LE_COMMIT_MULTI_PLAYERS_NO_SHIPS, true},
	{"COMMIT_MULTI_NOT_ALL_ASSIGNED", LE_COMMIT_MULTI_NOT_ALL_ASSIGNED, true},
	{"COMMIT_MULTI_NO_PRIMARY", LE_COMMIT_MULTI_NO_PRIMARY, true},
	{"COMMIT_MULTI_NO_SECONDARY", LE_COMMIT_MULTI_NO_SECONDARY, true},
	{"SQUAD_MESSAGE_ATTACK_TARGET", LE_SQUAD_MESSAGE_ATTACK_TARGET, true},
	{"SQUAD_MESSAGE_DISABLE_TARGET", LE_SQUAD_MESSAGE_DISABLE_TARGET, true},
	{"SQUAD_MESSAGE_DISARM_TARGET", LE_SQUAD_MESSAGE_DISARM_TARGET, true},
	{"SQUAD_MESSAGE_PROTECT_TARGET", LE_SQUAD_MESSAGE_PROTECT_TARGET, true},
	{"SQUAD_MESSAGE_IGNORE_TARGET", LE_SQUAD_MESSAGE_IGNORE_TARGET, true},
	{"SQUAD_MESSAGE_FORMATION", LE_SQUAD_MESSAGE_FORMATION, true},
	{"SQUAD_MESSAGE_COVER_ME", LE_SQUAD_MESSAGE_COVER_ME, true},
	{"SQUAD_MESSAGE_ENGAGE_ENEMY", LE_SQUAD_MESSAGE_ENGAGE_ENEMY, true},
	{"SQUAD_MESSAGE_CAPTURE_TARGET", LE_SQUAD_MESSAGE_CAPTURE_TARGET, true},
	{"SQUAD_MESSAGE_REARM_REPAIR_ME", LE_SQUAD_MESSAGE_REARM_REPAIR_ME, true},
	{"SQUAD_MESSAGE_ABORT_REARM_REPAIR", LE_SQUAD_MESSAGE_ABORT_REARM_REPAIR, true},
	{"SQUAD_MESSAGE_STAY_NEAR_ME", LE_SQUAD_MESSAGE_STAY_NEAR_ME, true},
	{"SQUAD_MESSAGE_STAY_NEAR_TARGET", LE_SQUAD_MESSAGE_STAY_NEAR_TARGET, true},
	{"SQUAD_MESSAGE_KEEP_SAFE_DIST", LE_SQUAD_MESSAGE_KEEP_SAFE_DIST, true},
	{"SQUAD_MESSAGE_DEPART", LE_SQUAD_MESSAGE_DEPART, true},
	{"SQUAD_MESSAGE_DISABLE_SUBSYSTEM", LE_SQUAD_MESSAGE_DISABLE_SUBSYSTEM, true},
	{"SQUAD_MESSAGE_LUA_AI", LE_SQUAD_MESSAGE_LUA_AI, true},
	{"BUILTIN_MESSAGE_ATTACK_TARGET", LE_BUILTIN_MESSAGE_ATTACK_TARGET, true},
	{"BUILTIN_MESSAGE_DISABLE_TARGET", LE_BUILTIN_MESSAGE_DISABLE_TARGET, true},
	{"BUILTIN_MESSAGE_DISARM_TARGET", LE_BUILTIN_MESSAGE_DISARM_TARGET, true},
	{"BUILTIN_MESSAGE_ATTACK_SUBSYSTEM", LE_BUILTIN_MESSAGE_ATTACK_SUBSYSTEM, true},
	{"BUILTIN_MESSAGE_PROTECT_TARGET", LE_BUILTIN_MESSAGE_PROTECT_TARGET, true},
	{"BUILTIN_MESSAGE_FORM_ON_MY_WING", LE_BUILTIN_MESSAGE_FORM_ON_MY_WING, true},
	{"BUILTIN_MESSAGE_COVER_ME", LE_BUILTIN_MESSAGE_COVER_ME, true},
	{"BUILTIN_MESSAGE_IGNORE", LE_BUILTIN_MESSAGE_IGNORE, true},
	{"BUILTIN_MESSAGE_ENGAGE", LE_BUILTIN_MESSAGE_ENGAGE, true},
	{"BUILTIN_MESSAGE_WARP_OUT", LE_BUILTIN_MESSAGE_WARP_OUT, true},
	{"BUILTIN_MESSAGE_DOCK_YES", LE_BUILTIN_MESSAGE_DOCK_YES, true},
	{"BUILTIN_MESSAGE_YESSIR", LE_BUILTIN_MESSAGE_YESSIR, true},
	{"BUILTIN_MESSAGE_NOSIR", LE_BUILTIN_MESSAGE_NOSIR, true},
	{"BUILTIN_MESSAGE_NO_TARGET", LE_BUILTIN_MESSAGE_NO_TARGET, true},
	{"BUILTIN_MESSAGE_CHECK_6", LE_BUILTIN_MESSAGE_CHECK_6, true},
	{"BUILTIN_MESSAGE_PLAYER_DIED", LE_BUILTIN_MESSAGE_PLAYER_DIED, true},
	{"BUILTIN_MESSAGE_PRAISE", LE_BUILTIN_MESSAGE_PRAISE, true},
	{"BUILTIN_MESSAGE_HIGH_PRAISE", LE_BUILTIN_MESSAGE_HIGH_PRAISE, true},
	{"BUILTIN_MESSAGE_BACKUP", LE_BUILTIN_MESSAGE_BACKUP, true},
	{"BUILTIN_MESSAGE_HELP", LE_BUILTIN_MESSAGE_HELP, true},
	{"BUILTIN_MESSAGE_WINGMAN_SCREAM", LE_BUILTIN_MESSAGE_WINGMAN_SCREAM, true},
	{"BUILTIN_MESSAGE_PRAISE_SELF", LE_BUILTIN_MESSAGE_PRAISE_SELF, true},
	{"BUILTIN_MESSAGE_REARM_REQUEST", LE_BUILTIN_MESSAGE_REARM_REQUEST, true},
	{"BUILTIN_MESSAGE_REPAIR_REQUEST", LE_BUILTIN_MESSAGE_REPAIR_REQUEST, true},
	{"BUILTIN_MESSAGE_PRIMARIES_LOW", LE_BUILTIN_MESSAGE_PRIMARIES_LOW, true},
	{"BUILTIN_MESSAGE_REARM_PRIMARIES", LE_BUILTIN_MESSAGE_REARM_PRIMARIES, true},
	{"BUILTIN_MESSAGE_REARM_WARP", LE_BUILTIN_MESSAGE_REARM_WARP, true},
	{"BUILTIN_MESSAGE_ON_WAY", LE_BUILTIN_MESSAGE_ON_WAY, true},
	{"BUILTIN_MESSAGE_ALREADY_ON_WAY", LE_BUILTIN_MESSAGE_ALREADY_ON_WAY, true},
	{"BUILTIN_MESSAGE_REPAIR_DONE", LE_BUILTIN_MESSAGE_REPAIR_DONE, true},
	{"BUILTIN_MESSAGE_REPAIR_ABORTED", LE_BUILTIN_MESSAGE_REPAIR_ABORTED, true},
	{"BUILTIN_MESSAGE_SUPPORT_KILLED", LE_BUILTIN_MESSAGE_SUPPORT_KILLED, true},
	{"BUILTIN_MESSAGE_ALL_ALONE", LE_BUILTIN_MESSAGE_ALL_ALONE, true},
	{"BUILTIN_MESSAGE_ARRIVE_ENEMY", LE_BUILTIN_MESSAGE_ARRIVE_ENEMY, true},
	{"BUILTIN_MESSAGE_OOPS", LE_BUILTIN_MESSAGE_OOPS, true},
	{"BUILTIN_MESSAGE_HAMMER_SWINE", LE_BUILTIN_MESSAGE_HAMMER_SWINE, true},
	{"BUILTIN_MESSAGE_AWACS_75", LE_BUILTIN_MESSAGE_AWACS_75, true},
	{"BUILTIN_MESSAGE_AWACS_25", LE_BUILTIN_MESSAGE_AWACS_25, true},
	{"BUILTIN_MESSAGE_STRAY_WARNING", LE_BUILTIN_MESSAGE_STRAY_WARNING, true},
	{"BUILTIN_MESSAGE_STRAY_WARNING_FINAL", LE_BUILTIN_MESSAGE_STRAY_WARNING_FINAL, true},
	{"BUILTIN_MESSAGE_INSTRUCTOR_HIT", LE_BUILTIN_MESSAGE_INSTRUCTOR_HIT, true},
	{"BUILTIN_MESSAGE_INSTRUCTOR_ATTACK", LE_BUILTIN_MESSAGE_INSTRUCTOR_ATTACK, true},
	{"BUILTIN_MESSAGE_ALL_CLEAR", LE_BUILTIN_MESSAGE_ALL_CLEAR, true},
	{"BUILTIN_MESSAGE_PERMISSION", LE_BUILTIN_MESSAGE_PERMISSION, true},
	{"BUILTIN_MESSAGE_STRAY", LE_BUILTIN_MESSAGE_STRAY, true}

};

const size_t Num_enumerations = sizeof(Enumerations) / sizeof(lua_enum_def_list);


enum_h::enum_h() {
	index = ENUM_INVALID;
	is_constant = false;
}
enum_h::enum_h(lua_enum n_index)
{
	index = n_index;
	is_constant = false;
	for (size_t i = 0; i < Num_enumerations; i++) {
		if (Enumerations[i].def == index) {
			value = Enumerations[i].value;
			break;
		}
	}
}
SCP_string enum_h::getName() const
{
	if (name)
		return *name;

	for (size_t i = 0; i < Num_enumerations; i++) {
		if (Enumerations[i].def == index) {
			return Enumerations[i].name;
		}
	}

	return SCP_string();
}
bool enum_h::isValid() const { return index < ENUM_NEXT_INDEX || index == ENUM_COMBINATION; }

enum_h operator&(const enum_h& l, const enum_h& other) {
	Assertion(l.value && other.value, "Tried to and-combine non-combinable enums %s and %s!", l.getName().c_str(), other.getName().c_str());

	enum_h enumerator{ lua_enum::ENUM_COMBINATION };
	enumerator.value = *l.value & *other.value;

	if (l.last_op == enum_h::last_combine_op::OR)
		enumerator.name = '(' + l.getName() + ')';
	else
		enumerator.name = l.getName();

	*enumerator.name += " & ";

	if (other.last_op == enum_h::last_combine_op::OR)
		*enumerator.name += '(' + other.getName() + ')';
	else
		*enumerator.name += other.getName();

	enumerator.last_op = enum_h::last_combine_op::AND;

	return enumerator;
}

enum_h operator|(const enum_h& l, const enum_h& other) {
	Assertion(l.value && other.value, "Tried to or-combine non-combinable enums %s and %s!", l.getName().c_str(), other.getName().c_str());

	enum_h enumerator{ lua_enum::ENUM_COMBINATION };
	enumerator.value = *l.value | *other.value;

	if (l.last_op == enum_h::last_combine_op::AND)
		enumerator.name = '(' + l.getName() + ')';
	else
		enumerator.name = l.getName();

	*enumerator.name += " | ";

	if (other.last_op == enum_h::last_combine_op::AND)
		*enumerator.name += '(' + other.getName() + ')';
	else
		*enumerator.name += other.getName();

	enumerator.last_op = enum_h::last_combine_op::OR;

	return enumerator;
}

void enum_h::serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& lvalue, ubyte* data, int& packet_size) {
	enum_h enumeration;
	lvalue.getValue(l_Enum.Get(&enumeration));
	ADD_INT(enumeration.index);
	if (enumeration.index == lua_enum::ENUM_COMBINATION) {
		ADD_INT(*enumeration.value);
		//If networked enums should have the name string tracking, it needs to be set here. However, due to bandwidth, this is disabled for now
	}
}

void enum_h::deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset) {
	int enum_index;
	GET_INT(enum_index);
	enum_h deserialized{ static_cast<lua_enum>(enum_index) };
	if (static_cast<lua_enum>(enum_index) == lua_enum::ENUM_COMBINATION) {
		int enum_value;
		GET_INT(enum_value);
		deserialized.value = enum_value;
		deserialized.name = "<network transmitted enum>";
	}

	new(data_ptr) enum_h(std::move(deserialized));
}

ADE_OBJ(l_Enum, enum_h, "enumeration", "Enumeration object");

ADE_FUNC(__newindex,
		 l_Enum,
		 "enumeration",
		 "Sets enumeration to specified value (if it is not a global)",
		 "enumeration",
		 "enumeration") {
	enum_h* e1 = nullptr, * e2 = nullptr;
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
		 nullptr,
		 "Returns enumeration name",
		 "string",
		 "Enumeration name, or \"<INVALID>\" if invalid") {
	enum_h* e = nullptr;
	if (!ade_get_args(L, "o", l_Enum.GetPtr(&e))) {
		return ade_set_args(L, "s", "<INVALID>");
	}

	if (!e->isValid()) {
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
	enum_h* e1 = nullptr;
	enum_h* e2 = nullptr;

	if (!ade_get_args(L, "oo", l_Enum.GetPtr(&e1), l_Enum.GetPtr(&e2))) {
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));
	}

	if (e1 == nullptr || e2 == nullptr) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", e1->index == ENUM_COMBINATION ? e1->value == e2->value : e1->index == e2->index);
}

ADE_FUNC(__add,
	l_Enum,
	"enumeration",
	"Calculates the logical OR of the two enums. Only applicable for certain bitfield enums (OS_*, DC_*, ...)",
	"enumeration",
	"Result of the OR operation. Invalid enum if input was not a valid enum or a bitfield enum.") {
	enum_h* e1 = nullptr;
	enum_h* e2 = nullptr;

	if (!ade_get_args(L, "oo", l_Enum.GetPtr(&e1), l_Enum.GetPtr(&e2))) {
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));
	}

	if (e1 == nullptr || e2 == nullptr || !e1->isValid() || !e2->isValid() || !e1->value ||!e2->value) {
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));
	}

	return ade_set_args(L, "o", l_Enum.Set(*e1 | *e2));
}

ADE_FUNC(__mul,
	l_Enum,
	"enumeration",
	"Calculates the logical AND of the two enums. Only applicable for certain bitfield enums (OS_*, DC_*, ...)",
	"enumeration",
	"Result of the AND operation. Invalid enum if input was not a valid enum or a bitfield enum.") {
	enum_h* e1 = nullptr;
	enum_h* e2 = nullptr;

	if (!ade_get_args(L, "oo", l_Enum.GetPtr(&e1), l_Enum.GetPtr(&e2))) {
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));
	}

	if (e1 == nullptr || e2 == nullptr || !e1->isValid() || !e2->isValid() || !e1->value || !e2->value) {
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));
	}

	return ade_set_args(L, "o", l_Enum.Set(*e1 & *e2));
}

ADE_VIRTVAR_DEPRECATED(IntValue, l_Enum, "enumeration", "Internal value of the enum.  Probably not useful unless this enum is a bitfield or corresponds to a #define somewhere else in the source code.", "number", "Integer (index) value of the enum", gameversion::version(23), "Deprecated in favor of Value")
{
	enum_h* e = nullptr;
	if (!ade_get_args(L, "o", l_Enum.GetPtr(&e))) {
		return ade_set_args(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "IntValue is read only!");
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "i", static_cast<int32_t>(e->index));
}

ADE_VIRTVAR(Value, l_Enum, "enumeration", "Internal bitfield value of the enum. -1 if the enum is not a bitfield", "number", "Integer value of the enum")
{
	enum_h* e = nullptr;
	if (!ade_get_args(L, "o", l_Enum.GetPtr(&e))) {
		return ade_set_args(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "Value is read only!");
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "i", e->value.value_or(-1));
}

}
}
