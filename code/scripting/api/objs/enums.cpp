
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
	{"ORDER_ATTACK_SHIP_TYPE", LE_ORDER_ATTACK_SHIP_TYPE, true},
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
	{"SEXPVAR_PERSIST_CAMPAIGN", LE_SEXPVAR_CAMPAIGN_PERSISTENT, true},
	{"SEXPVAR_CAMPAIGN_PERSISTENT", LE_SEXPVAR_CAMPAIGN_PERSISTENT, true},
	{"SEXPVAR_PERSIST_NONE", LE_SEXPVAR_NOT_PERSISTENT, true},
	{"SEXPVAR_NOT_PERSISTENT", LE_SEXPVAR_NOT_PERSISTENT, true},
	{"SEXPVAR_PERSIST_PLAYER", LE_SEXPVAR_PLAYER_PERSISTENT, true},
	{"SEXPVAR_PLAYER_PERSISTENT", LE_SEXPVAR_PLAYER_PERSISTENT, true},
	{"SEXPVAR_TYPE_NUMBER", LE_SEXPVAR_TYPE_NUMBER, true},
	{"SEXPVAR_TYPE_STRING", LE_SEXPVAR_TYPE_STRING, true},
	{"TEXTURE_STATIC", LE_TEXTURE_STATIC, true},
	{"TEXTURE_DYNAMIC", LE_TEXTURE_DYNAMIC, true},
	{"LOCK", LE_LOCK, true},
	{"UNLOCK", LE_UNLOCK, true},
	{"SHIELD_NONE", LE_NONE, true},
	{"NONE", LE_NONE, true},
	{"SHIELD_FRONT", LE_SHIELD_FRONT, true},
	{"SHIELD_LEFT", LE_SHIELD_LEFT, true},
	{"SHIELD_RIGHT", LE_SHIELD_RIGHT, true},
	{"SHIELD_BACK", LE_SHIELD_BACK, true},
	{"MISSION_REPEAT", LE_MISSION_REPEAT, true},
	{"FLIGHT_CONTROL_NORMAL", LE_NORMAL_CONTROLS, true},
	{"NORMAL_CONTROLS", LE_NORMAL_CONTROLS, true},
	{"FLIGHT_CONTROL_LUA_STEERING", LE_LUA_STEERING_CONTROLS, true},
	{"LUA_STEERING_CONTROLS", LE_LUA_STEERING_CONTROLS, true},
	{"FLIGHT_CONTROL_LUA_FULL", LE_LUA_FULL_CONTROLS, true},
	{"LUA_FULL_CONTROLS", LE_LUA_FULL_CONTROLS, true},
	{"BUTTON_CONTROL_NORMAL", LE_NORMAL_BUTTON_CONTROLS, true},
	{"NORMAL_BUTTON_CONTROLS", LE_NORMAL_BUTTON_CONTROLS, true},
	{"BUTTON_CONTROL_LUA_ADDITIVE", LE_LUA_ADDITIVE_BUTTON_CONTROL, true},
	{"LUA_ADDITIVE_BUTTON_CONTROL", LE_LUA_ADDITIVE_BUTTON_CONTROL, true},
	{"BUTTON_CONTROL_LUA_OVERRIDE", LE_LUA_OVERRIDE_BUTTON_CONTROL, true},
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
	{"SHIP_STATUS_INVALID", LE_INVALID, true},
	{"INVALID", LE_INVALID, true},
	{"SHIP_STATUS_NOT_YET_PRESENT", LE_NOT_YET_PRESENT, true},
	{"NOT_YET_PRESENT", LE_NOT_YET_PRESENT, true},
	{"SHIP_STATUS_PRESENT", LE_PRESENT, true},
	{"PRESENT", LE_PRESENT, true},
	{"SHIP_STATUS_DEATH_ROLL", LE_DEATH_ROLL, true},
	{"DEATH_ROLL", LE_DEATH_ROLL, true},
	{"SHIP_STATUS_EXITED", LE_EXITED, true},
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

struct deprecated_enum_name_info {
	lua_enum value;
	const char* replacement;
	gameversion::version deprecated_since;
};

static const SCP_unordered_map<SCP_string, deprecated_enum_name_info> Deprecated_enumeration_names = {
	// Deprecated due to renaming during the great enum cleanup of 2026
	{"NONE", {LE_NONE, "SHIELD_NONE", gameversion::version(26, 0)}},
	{"NORMAL_CONTROLS", {LE_NORMAL_CONTROLS, "FLIGHT_CONTROL_NORMAL", gameversion::version(26, 0)}},
	{"LUA_STEERING_CONTROLS", {LE_LUA_STEERING_CONTROLS, "FLIGHT_CONTROL_LUA_STEERING", gameversion::version(26, 0)}},
	{"LUA_FULL_CONTROLS", {LE_LUA_FULL_CONTROLS, "FLIGHT_CONTROL_LUA_FULL", gameversion::version(26, 0)}},
	{"NORMAL_BUTTON_CONTROLS", {LE_NORMAL_BUTTON_CONTROLS, "BUTTON_CONTROL_NORMAL", gameversion::version(26, 0)}},
	{"LUA_ADDITIVE_BUTTON_CONTROL", {LE_LUA_ADDITIVE_BUTTON_CONTROL, "BUTTON_CONTROL_LUA_ADDITIVE", gameversion::version(26, 0)}},
	{"LUA_OVERRIDE_BUTTON_CONTROL", {LE_LUA_OVERRIDE_BUTTON_CONTROL, "BUTTON_CONTROL_LUA_OVERRIDE", gameversion::version(26, 0)}},
	{"INVALID", {LE_INVALID, "SHIP_STATUS_INVALID", gameversion::version(26, 0)}},
	{"NOT_YET_PRESENT", {LE_NOT_YET_PRESENT, "SHIP_STATUS_NOT_YET_PRESENT", gameversion::version(26, 0)}},
	{"PRESENT", {LE_PRESENT, "SHIP_STATUS_PRESENT", gameversion::version(26, 0)}},
	{"DEATH_ROLL", {LE_DEATH_ROLL, "SHIP_STATUS_DEATH_ROLL", gameversion::version(26, 0)}},
	{"EXITED", {LE_EXITED, "SHIP_STATUS_EXITED", gameversion::version(26, 0)}},
	{"SEXPVAR_CAMPAIGN_PERSISTENT", {LE_SEXPVAR_CAMPAIGN_PERSISTENT, "SEXPVAR_PERSIST_CAMPAIGN", gameversion::version(26, 0)}},
	{"SEXPVAR_NOT_PERSISTENT", {LE_SEXPVAR_NOT_PERSISTENT, "SEXPVAR_PERSIST_NONE", gameversion::version(26, 0)}},
	{"SEXPVAR_PLAYER_PERSISTENT", {LE_SEXPVAR_PLAYER_PERSISTENT, "SEXPVAR_PERSIST_PLAYER", gameversion::version(26, 0)}},
	// Previously this was a manual deprecation and is now moved here with version 26.0.0 because the old way never actually errored
	{"VM_EXTERNAL_CAMERA_LOCKED", {LE_VM_EXTERNAL_CAMERA_LOCKED, "VM_CAMERA_LOCKED", gameversion::version(26, 0)}},
	// These target enums are unused - no Lua API function accepts them
	{"LOCK", {LE_LOCK, "nothing (this enum was never implemented)", gameversion::version(26, 0)}},
	{"UNLOCK", {LE_UNLOCK, "nothing (this enum was never implemented)", gameversion::version(26, 0)}},
	// These cfile enums were never implemented — no Lua API function accepts them
	{"CFILE_TYPE_NORMAL", {LE_CFILE_TYPE_NORMAL, "nothing (these enums were never implemented)", gameversion::version(26, 0)}},
	{"CFILE_TYPE_MEMORY_MAPPED", {LE_CFILE_TYPE_MEMORY_MAPPED, "nothing (these enums were never implemented)", gameversion::version(26, 0)}},
	// Deprecated/unused particle types
	{"PARTICLE_DEBUG", {LE_PARTICLE_DEBUG, "nothing (debug particles were removed in FSO 25.0)", gameversion::version(25, 0)}},
	{"PARTICLE_PERSISTENT_BITMAP", {LE_PARTICLE_PERSISTENT_BITMAP, "PARTICLE_BITMAP via createPersistentParticle (this enum was never implemented)", gameversion::version(26, 0)}}
};

static std::optional<deprecated_enum_name_info> get_deprecated_enum_info(const SCP_string& enum_name) {
	auto deprecated_enum = Deprecated_enumeration_names.find(enum_name);
	if (deprecated_enum != Deprecated_enumeration_names.end()) {
		return deprecated_enum->second;
	}

	return std::nullopt;
}

bool is_deprecated_enum_name(const char* enum_name) {
	return enum_name != nullptr && Deprecated_enumeration_names.find(enum_name) != Deprecated_enumeration_names.end();
}

std::optional<enum_group_info> get_enum_group_info(const char* enum_name) {
	if (enum_name == nullptr) {
		return std::nullopt;
	}

	auto starts = [enum_name](const char* prefix) { return strncmp(enum_name, prefix, strlen(prefix)) == 0; };

	static const SCP_vector<enum_group_info> Enum_groups = {
		{"ALPHABLEND_", "alphablend", "Alpha Blending", "Alpha blending modes used by rendering functions."},
		{"MOUSE_", "mouse-buttons", "Mouse Buttons", "Mouse button constants."},
		{"FLIGHTMODE_", "flight-modes", "Flight Modes", "Primary flight cursor/ship lock modes."},
		{"ORDER_", "orders", "Orders", "AI and command order constants."},
		{"PARTICLE_", "particles", "Particle Types", "Particle rendering and creation types."},
		{"SEXPVAR_TYPE_", "sexpvar-type", "SEXP Variable Type", "SEXP variable value type constants."},
		{"SEXPVAR_PERSIST_", "sexpvar-persist", "SEXP Variable Persistence", "SEXP variable persistence level constants."},
		{"TEXTURE_", "textures", "Texture Sources", "Texture handle source types."},
		{"MISSION_", "mission-flow", "Mission Flow", "Mission-level control and mission selection constants."},
		{"SHIELD_", "shield-quadrants", "Shield Quadrants", "Shield quadrant constants."},
		{"FLIGHT_CONTROL_", "flight-controls", "Flight Control Mode", "Specifies whether steering is handled by normal controls or Lua control modes."},
		{"BUTTON_CONTROL_", "button-controls", "Button Control Mode", "Defines how Lua interacts with normal button input handling."},
		{"VM_", "view-modes", "View Modes", "View and camera modes for rendering/player perspective."},
		{"MESSAGE_PRIORITY_", "message-priority", "Message Priority", "Mission message priority levels."},
		{"OPTION_TYPE_", "option-types", "Option Types", "Option UI and value representation types."},
		{"AUDIOSTREAM_", "audio-stream-types", "Audio Stream Types", "Audio stream categories."},
		{"CONTEXT_", "execution-context", "Execution Context", "Lua execution context state values."},
		{"FIREBALL_", "fireball-types", "Fireball Types", "Fireball effect types."},
		{"GR_RESIZE_", "resize-modes", "Resize Modes", "Graphics resize mode constants."},
		{"OS_", "object-sound", "Object Sound", "Object sound flags and sound slots."},
		{"MOVIE_", "movies", "Movie Events", "Campaign/mission movie event positions."},
		{"TBOX_", "targetbox-flash", "Targetbox Flash", "Targetbox flashing indicator selectors."},
		{"LUAAI_", "luaai-goals", "Lua AI Goal State", "Lua AI achievability state values."},
		{"SCORE_", "score-events", "Score Events", "Identifies a music score slot for a specific game screen."},
		{"SHIP_STATUS_", "ship-status", "Ship Presence/State", "Describes high-level ship registry presence and lifecycle state."},
		{"DC_", "object-create-flags", "Object Creation Flags", "Flags that modify object creation behavior."},
		{"RPC_", "rpc", "Remote Procedure Call", "RPC routing and reliability mode constants."},
		{"HOTKEY_", "hotkey-lines", "Hotkey Lines", "Hotkey list line type constants."},
		{"SCROLLBACK_", "scrollback-sources", "Scrollback Sources", "Mission scrollback/source category constants."},
		{"MULTI_TYPE_", "multi-types", "Multiplayer Mission Types", "Multiplayer mission type constants."},
		{"MULTI_OPTION_", "multi-options", "Multiplayer Options", "Multiplayer option scope/permission constants."},
		{"MULTI_GAME_TYPE_", "multi-game-types", "Multiplayer Game Visibility", "Multiplayer game visibility/filter constants."},
		{"SEXP_", "sexp-result", "SEXP Results", "SEXP evaluation result state constants."},
		{"COMMIT_", "commit-status", "Loadout Commit Status", "Loadout/briefing commit result constants."},
		{"SQUAD_MESSAGE_", "squad-messages", "Squad Messages", "Wingman command/squad message constants."},
		{"BUILTIN_MESSAGE_", "builtin-messages", "Built-in Messages", "Built-in message event constants."}
	};
	for (const auto& group : Enum_groups) {
		if (starts(group.prefix)) {
			return group;
		}
	}

	return std::nullopt;
}

static const SCP_unordered_map<SCP_string, const char*> Enum_descriptions = {
	// ALPHABLEND
	{"ALPHABLEND_NONE", "Always uses standard alpha blending (Alpha * Src + (1 - Alpha) * Dst). Opacity controls transparency."},
	{"ALPHABLEND_FILTER", "Adaptive blending. For bitmaps with an alpha channel, behaves identically to ALPHABLEND_NONE. For bitmaps without an alpha channel, uses additive blending where the bitmap is added to the background with its intensity scaled by opacity."},

	// MOUSE
	{"MOUSE_LEFT_BUTTON", "The left mouse button click."},
	{"MOUSE_RIGHT_BUTTON", "The right mouse button click."},
	{"MOUSE_MIDDLE_BUTTON", "The middle mouse button click. Scroll wheel movement does not trigger this."},
	{"MOUSE_X1_BUTTON", "The first extra mouse button (typically the rear side/thumb button). Only available on hardware that provides it."},
	{"MOUSE_X2_BUTTON", "The second extra mouse button (typically the forward side/thumb button). Only available on hardware that provides it."},

	// FLIGHTMODE
	{"FLIGHTMODE_FLIGHTCURSOR", "Player inputs move a cursor within a cone in front of the ship; the ship turns toward the cursor."},
	{"FLIGHTMODE_SHIPLOCKED", "Player pitch and heading inputs directly rotate the ship. This is the standard flight control mode."},

	// ORDER
	{"ORDER_ATTACK", "Attack the target. Behavior depends on target type: attacks a ship, chases a weapon, or destroys a subsystem."},
	{"ORDER_ATTACK_WING", "Attack all ships in the wing that the target ship belongs to."},
	{"ORDER_ATTACK_SHIP_CLASS", "Attack all ships of the specified ship class."},
	{"ORDER_ATTACK_SHIP_TYPE", "Attack all ships of the specified ship type."},
	{"ORDER_ATTACK_ANY", "Attack any enemy ship in the mission."},
	{"ORDER_DEPART", "Warp out of the mission."},
	{"ORDER_DISABLE", "Destroy the target ship's engines to disable it. Purges conflicting goals from all ships in the mission."},
	{"ORDER_DISABLE_TACTICAL", "Destroy the target ship's engines to disable it. Does not purge any other goals."},
	{"ORDER_DISARM", "Destroy the target ship's turrets to disarm it. Purges conflicting goals from all ships in the mission."},
	{"ORDER_DISARM_TACTICAL", "Destroy the target ship's turrets to disarm it. Does not purge any other goals."},
	{"ORDER_DOCK", "Dock with the target ship."},
	{"ORDER_EVADE", "Actively fly away from and evade the target ship."},
	{"ORDER_FLY_TO", "Fly toward the target ship; goal completes when within range."},
	{"ORDER_FORM_ON_WING", "Fly in formation on the wing of the target ship."},
	{"ORDER_GUARD", "Patrol around and protect the target ship."},
	{"ORDER_GUARD_WING", "Patrol around and protect the entire wing that the target ship belongs to."},
	{"ORDER_IGNORE_SHIP", "Stop attacking the target ship and purge conflicting goals from all other ships in the mission."},
	{"ORDER_IGNORE_SHIP_NEW", "Stop attacking the target ship. Only removes conflicting goals from this ship, not others."},
	{"ORDER_KEEP_SAFE_DISTANCE", "Maintain a minimum safe distance from the target ship."},
	{"ORDER_PLAY_DEAD", "Ship drifts inertially and ignores all inputs including hits. Clears all other goals on the ship when issued."},
	{"ORDER_PLAY_DEAD_PERSISTENT", "Ship drifts inertially and ignores all inputs including hits. Does not clear other goals when issued; queued goals are preserved."},
	{"ORDER_REARM", "Fly to the target support ship to rearm and repair."},
	{"ORDER_STAY_NEAR", "Maintain ongoing proximity to the target ship."},
	{"ORDER_STAY_STILL", "Stop all movement and hold the current position."},
	{"ORDER_UNDOCK", "Undock from the target ship."},
	{"ORDER_WAYPOINTS", "Follow the target waypoint path, looping back to the start when complete."},
	{"ORDER_WAYPOINTS_ONCE", "Follow the target waypoint path once and stop at the final waypoint."},
	{"ORDER_LUA", "Execute a Lua-defined custom AI goal."},

	// PARTICLE
	{"PARTICLE_BITMAP", "Renders a particle using a custom texture supplied via the Texture parameter."},
	{"PARTICLE_FIRE", "Renders a particle using the built-in fire animation (particleexp01)."},
	{"PARTICLE_SMOKE", "Renders a particle using the first built-in smoke animation (particlesmoke01)."},
	{"PARTICLE_SMOKE2", "Renders a particle using the second built-in smoke animation (particlesmoke02)."},

	// SEXPVAR_PERSIST
	{"SEXPVAR_PERSIST_NONE", "Variable is reset to its initial value at the start of the mission."},
	{"SEXPVAR_PERSIST_CAMPAIGN", "Variable is saved to the campaign save file and resets when the campaign is restarted."},
	{"SEXPVAR_PERSIST_PLAYER", "Variable is saved to the player save file and never resets."},

	// SEXPVAR_TYPE
	{"SEXPVAR_TYPE_NUMBER", "SEXP variable holds an integer numeric value. Floats are not supported."},
	{"SEXPVAR_TYPE_STRING", "SEXP variable holds a string value."},

	// TEXTURE
	{"TEXTURE_STATIC", "Render target texture written to infrequently. Use when the texture content rarely changes between frames."},
	{"TEXTURE_DYNAMIC", "Render target texture written to frequently, e.g. every frame. This is the default for gr.createTexture()."},

	// MISSION
	{"MISSION_REPEAT", "Sentinel value for mn.startMission(): restarts the most recently played mission instead of specifying a filename."},

	// SHIELD
	{"SHIELD_NONE", "Refers to the entire shield as a whole rather than a specific quadrant. Gets or sets total shield strength across all quadrants."},
	{"SHIELD_FRONT", "The front shield quadrant."},
	{"SHIELD_LEFT", "The left shield quadrant."},
	{"SHIELD_RIGHT", "The right shield quadrant."},
	{"SHIELD_BACK", "The rear shield quadrant."},

	// FLIGHT_CONTROL
	{"FLIGHT_CONTROL_NORMAL", "Standard engine handles all steering and flight controls."},
	{"FLIGHT_CONTROL_LUA_STEERING", "Lua overrides rotational inputs (pitch, heading, bank); the engine still handles throttle and lateral/vertical translation."},
	{"FLIGHT_CONTROL_LUA_FULL", "Lua overrides all movement axes including pitch, heading, bank, throttle, and lateral/vertical translation. Weapon firing inputs are not affected."},

	// BUTTON_CONTROL
	{"BUTTON_CONTROL_NORMAL", "Standard engine processes all button inputs."},
	{"BUTTON_CONTROL_LUA_ADDITIVE", "Lua button inputs are processed alongside normal engine button inputs."},
	{"BUTTON_CONTROL_LUA_OVERRIDE", "Lua button processing replaces normal engine button input handling entirely."},

	// VM
	{"VM_INTERNAL", "Cockpit (internal) view. True when no external view flags are active; camera lock and centering state are excluded from this check."},
	{"VM_EXTERNAL", "View is external to the cockpit. A base flag combined with others such as VM_CHASE or VM_OTHER_SHIP."},
	{"VM_TRACK", "The external camera rotates to face the currently targeted ship or subsystem."},
	{"VM_DEAD_VIEW", "Camera view used when the player ship has been destroyed."},
	{"VM_CHASE", "Chase camera positioned behind and above the player ship."},
	{"VM_OTHER_SHIP", "Camera positioned on the player's currently targeted ship."},
	{"VM_CAMERA_LOCKED", "The player does not have manual control of the camera; it is managed by the engine or scripting. Often set alongside padlock and external view flags."},
	{"VM_WARP_CHASE", "Camera view during the player's warp-out sequence; the camera remains at the departure point as the ship accelerates away."},
	{"VM_PADLOCK_UP", "Padlock quick-look view directed upward."},
	{"VM_PADLOCK_REAR", "Padlock quick-look view directed toward the rear."},
	{"VM_PADLOCK_LEFT", "Padlock quick-look view directed to the left."},
	{"VM_PADLOCK_RIGHT", "Padlock quick-look view directed to the right."},
	{"VM_WARPIN_ANCHOR", "Stationary camera anchor point used during a warp-in sequence."},
	{"VM_TOPDOWN", "Top-down overhead camera looking down at the player ship."},
	{"VM_FREECAMERA", "A scripted camera not attached to any ship or object, typically set by a SEXP or Lua camera call."},
	{"VM_CENTERING", "The cockpit view angle is springing back to the forward-facing orientation after freelook or padlock was released."},

	// MESSAGE_PRIORITY
	{"MESSAGE_PRIORITY_LOW", "Lowest priority. Message is dropped if the sender has been destroyed or departed. Will not interrupt messages that are currently playing."},
	{"MESSAGE_PRIORITY_NORMAL", "Normal priority. Message is dropped if the sender has been destroyed or departed. Interrupts lower-priority messages currently playing."},
	{"MESSAGE_PRIORITY_HIGH", "Highest priority. If the sender has been destroyed or departed, the message plays anyway from Terran Command. Interrupts lower-priority messages currently playing."},

	// OPTION_TYPE
	{"OPTION_TYPE_SELECTION", "Option has a fixed set of discrete values to choose from, such as a dropdown."},
	{"OPTION_TYPE_RANGE", "Option has a continuous numeric range with interpolation, such as a slider."},

	// AUDIOSTREAM
	{"AUDIOSTREAM_EVENTMUSIC", "Stream type for in-mission dynamic event music. Volume is controlled by the music volume setting."},
	{"AUDIOSTREAM_MENUMUSIC", "Stream type for background music in menus and briefings, such as the main hall, credits, and briefing screens. Volume follows the music volume setting."},
	{"AUDIOSTREAM_VOICE", "Stream type for spoken voice audio such as mission briefings, training narration, and debriefs. Volume is controlled by the voice volume setting."},

	// CONTEXT
	{"CONTEXT_VALID", "The Lua execution context is running normally."},
	{"CONTEXT_SUSPENDED", "The context exists but is not currently active; the executor will retry later. For example, a game state that is on the stack but not currently at the top."},
	{"CONTEXT_INVALID", "The Lua execution context has been destroyed or is unusable."},

	// FIREBALL
	{"FIREBALL_MEDIUM_EXPLOSION", "A medium-sized explosion fireball effect."},
	{"FIREBALL_LARGE_EXPLOSION", "A large explosion fireball effect."},
	{"FIREBALL_WARP_EFFECT", "A warp portal effect used for ship warp-in and warp-out sequences."},

	// MOVIE
	{"MOVIE_PRE_FICTION", "Plays just before the fiction viewer game state."},
	{"MOVIE_PRE_CMD_BRIEF", "Plays just before the command briefing game state."},
	{"MOVIE_PRE_BRIEF", "Plays just before the briefing game state."},
	{"MOVIE_PRE_GAME", "Plays just before the mission starts after Accept has been pressed."},
	{"MOVIE_PRE_DEBRIEF", "Plays just before the debriefing game state."},
	{"MOVIE_POST_DEBRIEF", "Plays when the debriefing has been accepted but before exiting the mission."},
	{"MOVIE_END_CAMPAIGN", "Plays when the campaign has been completed."},

	// GR_RESIZE
	{"GR_RESIZE_NONE", "Coordinates are raw pixels with no scaling applied."},
	{"GR_RESIZE_FULL", "Scale to fill the full screen dimensions."},
	{"GR_RESIZE_FULL_CENTER", "Scale to fill the full screen, centered within the viewport."},
	{"GR_RESIZE_MENU", "Coordinates use the standard UI/menu coordinate space."},
	{"GR_RESIZE_MENU_ZOOMED", "Menu coordinate space that extends into the full display area, including any letterbox or pillarbox borders beyond the standard menu boundary."},
	{"GR_RESIZE_MENU_NO_OFFSET", "Menu coordinate space without positional offset applied."},

	// OS (object sound slots/flags)
	{"OS_NONE", "No flags. Assigns a basic persistent sound with default distance attenuation."},
	{"OS_MAIN", "Sound for which attenuation does not apply within the object's radius. Typically used for primary continuous sounds such as engine idle loops."},
	{"OS_ENGINE", "Secondary engine sound, typically tied to throttle level."},
	{"OS_TURRET_BASE_ROTATION", "Sound played while a turret base rotates."},
	{"OS_TURRET_GUN_ROTATION", "Sound played while a turret gun barrel rotates."},
	{"OS_SUBSYS_ALIVE", "Continuous sound played while the subsystem is operational."},
	{"OS_SUBSYS_DEAD", "Continuous sound that plays only while the subsystem is destroyed (has zero hit points)."},
	{"OS_SUBSYS_DAMAGED", "Sound whose volume scales proportionally with remaining subsystem health; full volume at full health, silent when destroyed."},
	{"OS_SUBSYS_ROTATION", "Sound played during a subsystem rotation animation."},
	{"OS_PLAY_ON_PLAYER", "Allow this sound to play even when the source object is the player's own ship, bypassing normal player-ship audio suppression in cockpit view."},
	{"OS_LOOPING_DISABLED", "Prevent the sound from looping after it finishes playing."},

	// TBOX_FLASH
	{"TBOX_FLASH_NAME", "The target name section of the target box."},
	{"TBOX_FLASH_CARGO", "The cargo information section of the target box."},
	{"TBOX_FLASH_HULL", "The hull integrity section of the target box."},
	{"TBOX_FLASH_STATUS", "The target status section of the target box."},
	{"TBOX_FLASH_SUBSYS", "The targeted subsystem section of the target box."},

	// DC (object creation flags)
	{"DC_IS_HULL", "Mark the debris piece as a hull fragment, using a specific ship geometry submodel with hull-appropriate physics and lifetime."},
	{"DC_VAPORIZE", "For non-hull debris, use the vaporize-effect model instead of the generic debris model and extend the piece's lifetime."},
	{"DC_SET_VELOCITY", "Apply explosion-based velocity to the debris, adding a radial kick from the explosion center to the inherited source ship velocity."},
	{"DC_FIRE_HOOK", "Fire the OnDebrisCreated scripting hook after the debris piece is created."},

	// RPC
	{"RPC_SERVER", "Send the RPC call to the server only."},
	{"RPC_CLIENTS", "Send the RPC call to all connected clients."},
	{"RPC_BOTH", "Send the RPC call to both the server and all clients."},
	{"RPC_RELIABLE", "Use reliable delivery; packet is guaranteed to arrive."},
	{"RPC_ORDERED", "Use sequenced delivery over unreliable transport; out-of-order or stale packets are discarded so only the most recent call is processed."},
	{"RPC_UNRELIABLE", "Use unreliable delivery; fastest option but packets may be dropped."},

	// SEXP results
	{"SEXP_TRUE", "SEXP evaluated to a true result."},
	{"SEXP_FALSE", "SEXP evaluated to a false result."},
	{"SEXP_KNOWN_FALSE", "SEXP is permanently false and will never become true."},
	{"SEXP_KNOWN_TRUE", "SEXP is permanently true and will never become false."},
	{"SEXP_UNKNOWN", "The SEXP result is indeterminate and cannot yet be resolved."},
	{"SEXP_NAN", "Numeric result is unavailable because a referenced ship or wing has not yet arrived. The value may become valid later once the ship arrives."},
	{"SEXP_NAN_FOREVER", "SEXP will never produce a valid numeric result."},
	{"SEXP_CANT_EVAL", "The SEXP cannot yet be evaluated, typically because a referenced object does not yet exist. Acts like false until conditions are met."},

	// COMMIT (loadout validation results)
	{"COMMIT_SUCCESS", "Loadout validation passed; the mission is ready to launch."},
	{"COMMIT_FAIL", "Generic loadout validation failure."},
	{"COMMIT_PLAYER_NO_WEAPONS", "Player ship has no weapons assigned."},
	{"COMMIT_NO_REQUIRED_WEAPON", "A weapon required by the mission is missing from the loadout."},
	{"COMMIT_NO_REQUIRED_WEAPON_MULTIPLE", "Multiple mission-required weapons are missing from the loadout."},
	{"COMMIT_BANK_GAP_ERROR", "Weapon banks contain gaps; empty slots appear between assigned weapons."},
	{"COMMIT_PLAYER_NO_SLOT", "Player has no ship slot assigned in the briefing."},
	{"COMMIT_MULTI_PLAYERS_NO_SHIPS", "Not all multiplayer players have been assigned to their ship slots yet."},
	{"COMMIT_MULTI_NOT_ALL_ASSIGNED", "Not all multiplayer pilots have been assigned to a ship."},
	{"COMMIT_MULTI_NO_PRIMARY", "A multiplayer ship has no primary weapon assigned."},
	{"COMMIT_MULTI_NO_SECONDARY", "A multiplayer ship has no secondary weapon assigned."},

	// MULTI_OPTION
	{"MULTI_OPTION_RANK", "Restricts the action to the highest-ranking player(s) in the game. Used by NetGame.Orders and NetGame.EndMission."},
	{"MULTI_OPTION_LEAD", "Restricts the action to wing or team leaders only. Used by NetGame.Orders and NetGame.EndMission."},
	{"MULTI_OPTION_ANY", "Allows any non-observer player to perform the action. Used by NetGame.Orders and NetGame.EndMission."},
	{"MULTI_OPTION_HOST", "Restricts the action to the game host only. Used by NetGame.Orders and NetGame.EndMission."},

	// LUAAI
	{"LUAAI_ACHIEVABLE", "The Lua AI goal can be executed immediately."},
	{"LUAAI_NOT_YET_ACHIEVABLE", "The Lua AI goal is valid but cannot be started yet."},
	{"LUAAI_UNACHIEVABLE", "The Lua AI goal can never be achieved and will be cancelled."},

	// SCORE
	{"SCORE_BRIEFING", "Music score slot for the mission briefing screen."},
	{"SCORE_DEBRIEFING_SUCCESS", "Music score slot for the debriefing screen when the mission was successful."},
	{"SCORE_DEBRIEFING_AVERAGE", "Music score slot for the debriefing screen when the mission result was average."},
	{"SCORE_DEBRIEFING_FAILURE", "Music score slot for the debriefing screen when the mission failed."},
	{"SCORE_FICTION_VIEWER", "Music score slot for the fiction viewer screen."},

	// SHIP_STATUS
	{"SHIP_STATUS_INVALID", "The ship registry entry has not been initialized yet."},
	{"SHIP_STATUS_NOT_YET_PRESENT", "The ship is on the arrival list but has not yet arrived in-mission."},
	{"SHIP_STATUS_PRESENT", "The ship is currently in-mission; its object and ship pointers are valid."},
	{"SHIP_STATUS_DEATH_ROLL", "The ship has been destroyed but is still in its death roll; its object and ship pointers remain valid."},
	{"SHIP_STATUS_EXITED", "The ship has been destroyed, departed, or vanished; its object and ship pointers are no longer valid."},

	// BUILTIN_MESSAGE
	{"BUILTIN_MESSAGE_ATTACK_TARGET", "Confirmation sent by a wingman acknowledging an Attack order."},
	{"BUILTIN_MESSAGE_DISABLE_TARGET", "Confirmation sent by a wingman acknowledging a Disable order."},
	{"BUILTIN_MESSAGE_DISARM_TARGET", "Confirmation sent by a wingman acknowledging a Disarm order."},
	{"BUILTIN_MESSAGE_ATTACK_SUBSYSTEM", "Confirmation sent by a wingman acknowledging an order to destroy a specific subsystem."},
	{"BUILTIN_MESSAGE_PROTECT_TARGET", "Confirmation sent by a wingman acknowledging a Protect order."},
	{"BUILTIN_MESSAGE_FORM_ON_MY_WING", "Confirmation sent by a wingman acknowledging a Form on Wing order."},
	{"BUILTIN_MESSAGE_COVER_ME", "Confirmation sent by a wingman acknowledging a Cover Me order."},
	{"BUILTIN_MESSAGE_IGNORE", "Confirmation sent by a wingman acknowledging an Ignore order."},
	{"BUILTIN_MESSAGE_ENGAGE", "Confirmation sent by a wingman acknowledging an Engage Enemy order."},
	{"BUILTIN_MESSAGE_WARP_OUT", "Confirmation sent by a wingman acknowledging a Depart order."},
	{"BUILTIN_MESSAGE_DOCK_YES", "Sent by a ship when it begins a docking sequence."},
	{"BUILTIN_MESSAGE_YESSIR", "Generic affirmative acknowledgment sent by a wingman when complying with an order."},
	{"BUILTIN_MESSAGE_NOSIR", "Generic refusal sent by a wingman when an order cannot be carried out."},
	{"BUILTIN_MESSAGE_NO_TARGET", "Sent by a wingman when the order requires a target but no valid target is available."},
	{"BUILTIN_MESSAGE_CHECK_6", "Warning sent by a wingman when an enemy fighter or bomber is attacking the player from behind."},
	{"BUILTIN_MESSAGE_PLAYER_DIED", "Sent by a wingman when the player enters the death roll."},
	{"BUILTIN_MESSAGE_PRAISE", "Compliment sent by a wingman when the player destroys an enemy ship."},
	{"BUILTIN_MESSAGE_HIGH_PRAISE", "Enhanced compliment sent by a wingman when the player has accumulated more than ten kills in the mission. Falls back to BUILTIN_MESSAGE_PRAISE."},
	{"BUILTIN_MESSAGE_BACKUP", "Sent by a ship from a newly arrived friendly wing announcing support."},
	{"BUILTIN_MESSAGE_HELP", "Distress call sent by a wingman when their hull or shields are critically low."},
	{"BUILTIN_MESSAGE_WINGMAN_SCREAM", "Death cry sent by a wingman as they are destroyed."},
	{"BUILTIN_MESSAGE_PRAISE_SELF", "Boast sent by a wingman when they destroy an enemy ship."},
	{"BUILTIN_MESSAGE_REARM_REQUEST", "Request sent by a wingman when their secondary missile ammunition falls below fifty percent."},
	{"BUILTIN_MESSAGE_REPAIR_REQUEST", "Request sent by a wingman when their hull integrity is critically low or a subsystem is disabled."},
	{"BUILTIN_MESSAGE_PRIMARIES_LOW", "Alert sent by a wingman when their ballistic primary weapon ammunition falls below thirty percent. Sent at most once per ship per mission."},
	{"BUILTIN_MESSAGE_REARM_PRIMARIES", "Request sent by a wingman for primary ballistic ammunition resupply. Falls back to BUILTIN_MESSAGE_REARM_REQUEST."},
	{"BUILTIN_MESSAGE_REARM_WARP", "Sent by a support ship as it warps in to provide repair or rearm service."},
	{"BUILTIN_MESSAGE_ON_WAY", "Sent by a support ship confirming it is en route to perform repairs or rearming."},
	{"BUILTIN_MESSAGE_ALREADY_ON_WAY", "Sent by a support ship when the player requests rearm but it is already en route. Falls back to BUILTIN_MESSAGE_ON_WAY."},
	{"BUILTIN_MESSAGE_REPAIR_DONE", "Sent by a support ship when it completes repair or rearm service."},
	{"BUILTIN_MESSAGE_REPAIR_ABORTED", "Sent by a support ship when its repair or rearm operation is interrupted."},
	{"BUILTIN_MESSAGE_SUPPORT_KILLED", "Sent by Command when the support ship is destroyed."},
	{"BUILTIN_MESSAGE_ALL_ALONE", "Sent by Command when the player is the last surviving friendly ship in the mission."},
	{"BUILTIN_MESSAGE_ARRIVE_ENEMY", "Warning sent when a new enemy wing arrives in the mission."},
	{"BUILTIN_MESSAGE_OOPS", "Sent by a wingman when they are hit by friendly fire."},
	{"BUILTIN_MESSAGE_HAMMER_SWINE", "Sent by a wingman when the player is identified as a traitor due to repeated friendly fire."},
	{"BUILTIN_MESSAGE_AWACS_75", "Warning sent by an AWACS ship when its hull integrity drops below seventy-five percent."},
	{"BUILTIN_MESSAGE_AWACS_25", "Warning sent by an AWACS ship when its hull integrity drops below twenty-five percent."},
	{"BUILTIN_MESSAGE_STRAY_WARNING", "Repeating warning sent by Command when the player strays outside the mission boundary."},
	{"BUILTIN_MESSAGE_STRAY_WARNING_FINAL", "Final warning sent by Command immediately before the player is destroyed for straying too far from the mission area."},
	{"BUILTIN_MESSAGE_INSTRUCTOR_HIT", "Sent during training missions when the player hits the instructor ship with friendly fire."},
	{"BUILTIN_MESSAGE_INSTRUCTOR_ATTACK", "Sent during training missions when the player directly attacks the instructor ship."},
	{"BUILTIN_MESSAGE_ALL_CLEAR", "Indicates the area is clear of enemies. Not currently triggered by the engine."},
	{"BUILTIN_MESSAGE_PERMISSION", "Permission-related response. Not currently triggered by the engine."},
	{"BUILTIN_MESSAGE_STRAY", "Intended as an initial stray warning. Not currently triggered by the engine."},

	// SQUAD_MESSAGE
	{"SQUAD_MESSAGE_ATTACK_TARGET", "Order the wingman to attack the player's current target. Removes the target's Protected flag if the ship type permits."},
	{"SQUAD_MESSAGE_DISABLE_TARGET", "Order the wingman to destroy the target's engines, immobilizing it without destroying the ship."},
	{"SQUAD_MESSAGE_DISARM_TARGET", "Order the wingman to destroy the target's turrets, neutralizing its weapons while leaving the ship mobile."},
	{"SQUAD_MESSAGE_PROTECT_TARGET", "Order the wingman to guard and protect the player's current friendly target."},
	{"SQUAD_MESSAGE_IGNORE_TARGET", "Order the wingman to stop attacking the player's current enemy target."},
	{"SQUAD_MESSAGE_FORMATION", "Order the wingman to fly in formation on the player's wing."},
	{"SQUAD_MESSAGE_COVER_ME", "Order the wingman to guard and protect the player."},
	{"SQUAD_MESSAGE_ENGAGE_ENEMY", "Order the wingman to attack any enemy ship. No target is required."},
	{"SQUAD_MESSAGE_CAPTURE_TARGET", "Order the wingman to dock with and capture the player's current enemy target. Sets the Protected flag on the target to prevent accidental destruction."},
	{"SQUAD_MESSAGE_REARM_REPAIR_ME", "Order the designated support ship to rearm and repair the player. Cannot be sent to wings."},
	{"SQUAD_MESSAGE_ABORT_REARM_REPAIR", "Order the designated support ship to abort an in-progress rearm and repair. Cannot be sent to wings."},
	{"SQUAD_MESSAGE_STAY_NEAR_ME", "Order a support ship to remain within close range of the player. Cannot be sent to wings."},
	{"SQUAD_MESSAGE_STAY_NEAR_TARGET", "Order a support ship to remain within close range of the player's current target. Cannot be sent to wings."},
	{"SQUAD_MESSAGE_KEEP_SAFE_DIST", "Order a support ship to maintain a safe distance from the player. Cannot be sent to wings."},
	{"SQUAD_MESSAGE_DEPART", "Order the wingman or wing to warp out of the mission."},
	{"SQUAD_MESSAGE_DISABLE_SUBSYSTEM", "Order the wingman to destroy the player's currently targeted subsystem on an enemy ship."},
	{"SQUAD_MESSAGE_LUA_AI", "Issues a custom Lua AI order. Not a standard squad menu command; used programmatically to invoke scripted AI behaviors."},

	// MULTI_GAME_TYPE
	{"MULTI_GAME_TYPE_OPEN", "Game is open to all players with no join restrictions."},
	{"MULTI_GAME_TYPE_PASSWORD", "Game requires players to provide the correct password to join. Pass the password string as the second argument to setGameType()."},
	{"MULTI_GAME_TYPE_RANK_ABOVE", "Only players at or above a specified rank may join. Pass the rank index as the second argument to setGameType()."},
	{"MULTI_GAME_TYPE_RANK_BELOW", "Only players at or below a specified rank may join. Pass the rank index as the second argument to setGameType()."},

	// MULTI_TYPE
	{"MULTI_TYPE_COOP", "Cooperative multiplayer mode. All players form a single team and play together against AI opposition."},
	{"MULTI_TYPE_TEAM", "Team vs. Team multiplayer mode. Players are divided into two competing teams. See also MULTI_TYPE_SQUADWAR for the ranked variant."},
	{"MULTI_TYPE_DOGFIGHT", "Free-for-all multiplayer mode. All players compete individually against each other for score."},
	{"MULTI_TYPE_SQUADWAR", "Squad War mode. A ranked, PXO tracker-integrated variant of Team vs. Team play. This is a runtime-only mode with no corresponding mission file flag; it can only be active when the mission supports MULTI_TYPE_TEAM."},

	// SCROLLBACK_SOURCE
	{"SCROLLBACK_SOURCE_COMPUTER", "Standard HUD system messages. Displayed in normal text color in the scrollback viewer."},
	{"SCROLLBACK_SOURCE_TRAINING", "Training mission narration and instructor messages. Displayed in bright blue text."},
	{"SCROLLBACK_SOURCE_HIDDEN", "Message is stored in the scrollback log but not rendered in the scrollback viewer; invisible to the player during normal play."},
	{"SCROLLBACK_SOURCE_IMPORTANT", "Critical mission notification. Displayed in bright white text."},
	{"SCROLLBACK_SOURCE_FAILED", "A mission objective has failed. Displayed in bright white text with a red circle icon in the scrollback viewer."},
	{"SCROLLBACK_SOURCE_SATISFIED", "A mission objective has been completed. Displayed in bright white text with a green circle icon in the scrollback viewer."},
	{"SCROLLBACK_SOURCE_COMMAND", "A message from Terran Command or the mission's commanding authority. Displayed in bright white text."},
	{"SCROLLBACK_SOURCE_NETPLAYER", "A chat message or communication from a multiplayer network player. Text color is determined by the player's IFF team."},

	// HOTKEY_LINE
	{"HOTKEY_LINE_NONE", "Sentinel value marking the end of the active hotkey list. Lines at this index and beyond are unused. Not encountered during normal iteration using the standard length operator."},
	{"HOTKEY_LINE_HEADING", "A non-interactive section header row in the hotkey list, such as 'Friendly Ships' or 'Enemy Ships'. Cannot be selected or assigned a hotkey."},
	{"HOTKEY_LINE_WING", "A wing (squadron) entry in the hotkey list. Can be expanded to show its constituent ships as HOTKEY_LINE_SUBSHIP entries beneath it, or collapsed to hide them."},
	{"HOTKEY_LINE_SHIP", "An individual ship that is not part of any wing; appears as a top-level entry in the hotkey list."},
	{"HOTKEY_LINE_SUBSHIP", "An individual ship that belongs to a wing, displayed as an indented child entry beneath its parent HOTKEY_LINE_WING entry when the wing is expanded."},
};

const char* get_enum_description(const char* enum_name) {
	if (enum_name == nullptr) {
		return nullptr;
	}
	auto it = Enum_descriptions.find(enum_name);
	if (it != Enum_descriptions.end()) {
		return it->second;
	}
	return nullptr;
}

static void maybe_warn_deprecated_enum(lua_State* L, const enum_h* e) {
	if (e == nullptr || !e->isValid()) {
		return;
	}
	auto deprecated_info = get_deprecated_enum_info(e->getName());
	if (deprecated_info) {
		const auto& deprecation_version = deprecated_info->deprecated_since;
		if (mod_supports_version(deprecation_version.major, deprecation_version.minor, deprecation_version.build)) {
			LuaError(L, "Enumeration '%s' is deprecated since version %s and cannot be used if the mod targets that version or higher. Use '%s' instead.",
				e->getName().c_str(), gameversion::format_version(deprecation_version).c_str(), deprecated_info->replacement);
		} else {
			Warning(LOCATION, "Enumeration '%s' is deprecated from version %s and should be replaced with '%s'.",
				e->getName().c_str(), gameversion::format_version(deprecation_version).c_str(), deprecated_info->replacement);
		}
	}
}


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

	maybe_warn_deprecated_enum(L, e2);

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

	maybe_warn_deprecated_enum(L, e);

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

	maybe_warn_deprecated_enum(L, e1);
	maybe_warn_deprecated_enum(L, e2);

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

	maybe_warn_deprecated_enum(L, e1);
	maybe_warn_deprecated_enum(L, e2);

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

	maybe_warn_deprecated_enum(L, e1);
	maybe_warn_deprecated_enum(L, e2);

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

	maybe_warn_deprecated_enum(L, e);

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

	maybe_warn_deprecated_enum(L, e);

	if (ADE_SETTING_VAR) {
		LuaError(L, "Value is read only!");
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "i", e->value.value_or(-1));
}

}
}
