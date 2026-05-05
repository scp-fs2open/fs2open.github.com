
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
	{"TARGET_LOCK", LE_LOCK, true},
	{"LOCK", LE_LOCK, true},
	{"TARGET_UNLOCK", LE_UNLOCK, true},
	{"UNLOCK", LE_UNLOCK, true},
	{"TARGET_NONE", LE_NONE, true},
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
	{"LOCK", {LE_LOCK, "TARGET_LOCK", gameversion::version(26, 0)}},
	{"UNLOCK", {LE_UNLOCK, "TARGET_UNLOCK", gameversion::version(26, 0)}},
	{"NONE", {LE_NONE, "TARGET_NONE or SHIELD_NONE depending on context", gameversion::version(26, 0)}},
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
	{"VM_EXTERNAL_CAMERA_LOCKED", {LE_VM_EXTERNAL_CAMERA_LOCKED, "VM_CAMERA_LOCKED", gameversion::version(26, 0)}}
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
		{"CFILE_TYPE_", "cfile-types", "CFile Types", "File backend types for cfile operations."},
		{"MOUSE_", "mouse-buttons", "Mouse Buttons", "Mouse button constants."},
		{"FLIGHTMODE_", "flight-modes", "Flight Modes", "Primary flight cursor/ship lock modes."},
		{"ORDER_", "orders", "Orders", "AI and command order constants."},
		{"PARTICLE_", "particles", "Particle Types", "Particle rendering and creation types."},
		{"SEXPVAR_TYPE_", "sexpvar-type", "SEXP Variable Type", "SEXP variable value type constants."},
		{"SEXPVAR_PERSIST_", "sexpvar-persist", "SEXP Variable Persistence", "SEXP variable persistence level constants."},
		{"TEXTURE_", "textures", "Texture Sources", "Texture handle source types."},
		{"TARGET_", "target-lock", "Target Lock Control", "Controls how targeting lock behavior is set or queried."},
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
		{"SCORE_", "score-events", "Score Events", "Score event source constants."},
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
