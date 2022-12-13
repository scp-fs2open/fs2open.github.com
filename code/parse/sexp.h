/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _SEXP_H
#define _SEXP_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"	// for NULL

class ship_subsys;
class ship;
class waypoint_list;
class object;
class waypoint;
class p_object;
struct ship_obj;

// bumped to 30 by Goober5000
#define	OPERATOR_LENGTH	30  // if this ever exceeds TOKEN_LENGTH, let JasonH know!

#define MAX_SEXP_VARIABLES 250

// Operator argument formats (data types of an argument)
#define	OPF_NONE				1		// argument cannot exist at this position if it's this
#define	OPF_NULL				2		// no value.  Can still be used for type matching, however
#define	OPF_BOOL				3
#define	OPF_NUMBER				4
#define	OPF_SHIP				5
#define	OPF_WING				6
#define	OPF_SUBSYSTEM			7
#define	OPF_POINT				8		// either a 3d point in space, or a waypoint name
#define	OPF_IFF					9
#define	OPF_AI_GOAL				10		// special to match ai goals
#define	OPF_DOCKER_POINT		11		// docking point on docker ship
#define	OPF_DOCKEE_POINT		12		// docking point on dockee ship
#define	OPF_MESSAGE				13		// the name (id) of a message in Messages[] array
#define	OPF_WHO_FROM			14		// who sent the message -- doesn't necessarily have to be a ship!!!
#define	OPF_PRIORITY			15		// priority for messages
#define	OPF_WAYPOINT_PATH		16		// name of a waypoint
#define	OPF_POSITIVE			17		// positive number or zero
#define	OPF_MISSION_NAME		18		// name of a mission for various mission related things
#define	OPF_SHIP_POINT			19		// a waypoint or a ship
#define	OPF_GOAL_NAME			20		// name of goal (or maybe event?) from a mission
#define	OPF_SHIP_WING			21		// either a ship or wing name (they don't conflict)
#define OPF_SHIP_WING_WHOLETEAM			22	// Karajorma - Ship, wing or an entire team's worth of ships
#define	OPF_SHIP_WING_SHIPONTEAM_POINT	23	// name of a ship, wing, any ship on a team, or a point
#define OPF_SHIP_WING_POINT		24
#define OPF_SHIP_WING_POINT_OR_NONE	25	// WMC - Ship, wing, point or none
#define	OPF_SHIP_TYPE			26		// type of ship (fighter/bomber/etc)
#define	OPF_KEYPRESS			27		// a default key
#define	OPF_EVENT_NAME			28		// name of an event
#define	OPF_AI_ORDER			29		// a squadmsg order player can give to a ship
#define	OPF_SKILL_LEVEL			30		// current skill level of the game
#define	OPF_MEDAL_NAME			31		// name of medals
#define	OPF_WEAPON_NAME			32		// name of a weapon
#define	OPF_SHIP_CLASS_NAME		33		// name of a ship class
#define	OPF_CUSTOM_HUD_GAUGE	34		// name of custom HUD gauge
#define	OPF_HUGE_WEAPON			35		// name of a secondary bomb type weapon
#define	OPF_SHIP_NOT_PLAYER		36		// a ship, but not a player ship
#define	OPF_JUMP_NODE_NAME		37		// name of a jump node
#define	OPF_VARIABLE_NAME		38		// variable name
#define	OPF_AMBIGUOUS			39		// type used with variable
#define	OPF_AWACS_SUBSYSTEM		40		// an awacs subsystem
#define OPF_CARGO				41		// Goober5000 - a cargo string (currently used for set-cargo and is-cargo)
#define OPF_AI_CLASS			42		// Goober5000 - an AI class
#define OPF_SUPPORT_SHIP_CLASS	43		// Goober5000 - a support ship class
#define OPF_ARRIVAL_LOCATION	44		// Goober5000 - a ship arrival location
#define OPF_ARRIVAL_ANCHOR_ALL	45		// Goober5000 - all of a ship's possible arrival anchors
#define OPF_DEPARTURE_LOCATION	46		// Goober5000 - a ship departure location
#define OPF_SHIP_WITH_BAY		47		// Goober5000 - a ship with a fighter bay
#define OPF_SOUNDTRACK_NAME		48		// Goober5000 - the name of a music soundtrack
#define OPF_INTEL_NAME			49		// Goober5000 - the name of an intel entry in species.tbl
#define OPF_STRING				50		// Goober5000 - any old string
#define OPF_ROTATING_SUBSYSTEM	51		// Goober5000 - a rotating subsystem
#define OPF_NAV_POINT			52		// Kazan	  - a Nav Point name
#define OPF_SSM_CLASS			53		// Goober5000 - an SSM class
#define OPF_FLEXIBLE_ARGUMENT	54		// Goober5000 - special to match for when-argument
#define OPF_ANYTHING			55		// Goober5000 - anything goes, except containers
#define OPF_SKYBOX_MODEL_NAME	56		// taylor - changing skybox model
#define OPF_SHIP_OR_NONE		57		// Goober5000 - an "optional" ship argument
#define OPF_BACKGROUND_BITMAP	58		// phreak - name of a background bitmap
#define OPF_SUN_BITMAP			59		// phreak - name of a background bitmap
#define OPF_NEBULA_STORM_TYPE	60		// phreak - name a nebula storm
#define OPF_NEBULA_POOF			61		// phreak - name of a nebula poof
#define OPF_TURRET_TARGET_ORDER	62		// WMC - name of a turret target type (see aiturret.cpp)
#define OPF_SUBSYSTEM_OR_NONE	63		// Goober5000 - an "optional" subsystem argument
#define OPF_PERSONA				64		// Karajorma - name of a persona
#define OPF_SUBSYS_OR_GENERIC	65		// Karajorma - a subsystem or a generic name (like engine) which covers all subsystems of that type
#define OPF_ORDER_RECIPIENT		66		// Karajorma - since orders can go to All Fighters as well as a ship or wing
#define OPF_SUBSYSTEM_TYPE		67		// Goober5000 - a generic subsystem type (navigation, engines, etc.) rather than a specific subsystem
#define OPF_POST_EFFECT			68		// Hery - type of post-processing effect
#define OPF_TARGET_PRIORITIES	69		// FUBAR - Target priority groups
#define OPF_ARMOR_TYPE			70		// FUBAR - Armor type or <none>
#define OPF_FONT				71		// Goober5000 - a FreeSpace font
#define OPF_HUD_ELEMENT			72		// A magic name of a specific HUD element
#define OPF_SOUND_ENVIRONMENT	73		// Goober5000 - one of EFX_presets, per Taylor
#define OPF_SOUND_ENVIRONMENT_OPTION 74	// Goober5000 - one of Taylor's options
#define OPF_EXPLOSION_OPTION	75		// Goober5000
#define OPF_AUDIO_VOLUME_OPTION 76		// The E
#define OPF_WEAPON_BANK_NUMBER	77		// Karajorma - The number of a primary/secondary/tertiary weapon bank or all of them
#define OPF_MESSAGE_OR_STRING	78		// Goober5000 - provides a list of messages like OPF_MESSAGE, but also allows entering arbitrary strings
#define OPF_BUILTIN_HUD_GAUGE	79		// The E
#define OPF_DAMAGE_TYPE			80		// FUBAR - Damage type or <none>
#define OPF_SHIP_EFFECT			81		// The E - per-ship effects, as defined in post-processing.tbl
#define OPF_ANIMATION_TYPE		82		// Goober5000 - as defined in modelanim.h
#define OPF_MISSION_MOOD		83		// Karajorma - Moods determine which builtin messages will be sent
#define OPF_SHIP_FLAG			84		// Karajorma - The name of a ship flag
#define OPF_TEAM_COLOR			85		// The E - Color settings as defined in Colors.tbl
#define OPF_NEBULA_PATTERN		86		// Axem - Full Nebula Background Patterns, as defined in nebula.tbl
#define OPF_SKYBOX_FLAGS		87		// niffiwan - valid skybox flags
#define OPF_GAME_SND			88		// m!m - A game sound
#define OPF_FIREBALL			89		// Goober5000 - an entry in fireball.tbl
#define OPF_SPECIES				90		// Goober5000
#define OPF_LANGUAGE			91		// Goober5000
#define OPF_FUNCTIONAL_WHEN_EVAL_TYPE	92	// Goober5000
#define OPF_CONTAINER_NAME		93		// Karajorma/jg18 - The name of a SEXP container
#define OPF_LIST_CONTAINER_NAME	94		// Karajorma/jg18 - The name of a SEXP list container
#define OPF_MAP_CONTAINER_NAME	95		// Karajorma/jg18 - The name of a SEXP map container
#define OPF_ANIMATION_NAME 		96		// Lafiel
#define OPF_CONTAINER_VALUE		97		// jg18 - Container data and map container keys
#define OPF_DATA_OR_STR_CONTAINER	98	// jg18 - any data, or a container that is accessed via strings
#define OPF_TRANSLATING_SUBSYSTEM	99	// Goober5000 - a translating subsystem
#define OPF_ANY_HUD_GAUGE		100		// Goober5000 - both custom and builtin
#define OPF_WING_FLAG			101		// Goober5000 - The name of a wing flag
#define OPF_ASTEROID_DEBRIS		102		// MjnMixael - Debris types as defined in asteroids.tbl
#define OPF_WING_FORMATION		103		// Goober5000 - as defined in ships.tbl

// Operand return types
#define	OPR_NUMBER				1	// returns number
#define	OPR_BOOL				2	// returns true/false value
#define	OPR_NULL				3	// doesn't return a value
#define	OPR_AI_GOAL				4	// is an ai operator (doesn't really return a value, but used for type matching)
#define	OPR_POSITIVE			5	// returns a non-negative number
#define	OPR_STRING				6	// not really a return type, but used for type matching.
#define	OPR_AMBIGUOUS			7	// not really a return type, but used for type matching.
#define OPR_FLEXIBLE_ARGUMENT	8	// Goober5000 - is an argument operator (doesn't really return a value, but used for type matching)

#define	OP_INSERT_FLAG			0x8000
#define	OP_REPLACE_FLAG			0x4000

// if we ever have more than 1024 (!)
// total sexps, we're going to have to
// figure out a different way of
// distinguishing between sexp identifier
// and sexp array index
#define	FIRST_OP				0x0400

// We'd use enums here, except that we can add categories and subcategories via dynamic SEXPs
enum SEXP_CATEGORIES : int {
	OP_CATEGORY_NONE,
	OP_CATEGORY_OBJECTIVE,
	OP_CATEGORY_TIME,
	OP_CATEGORY_LOGICAL,
	OP_CATEGORY_ARITHMETIC,
	OP_CATEGORY_STATUS,
	OP_CATEGORY_CHANGE,
	OP_CATEGORY_CONDITIONAL,
	OP_CATEGORY_AI,
	OP_CATEGORY_TRAINING,
	OP_CATEGORY_UNLISTED,
	OP_CATEGORY_GOAL_EVENT,
	// this should come after every category
	First_available_category_id
};

// New subcategories! :) -- Goober5000
// Adding more subcategories is possible with the new code.  All that needs to be done is
// to add a line here, some appropriate case statements in get_subcategory() and
// category_from_subcategory(), and the submenu name in the op_submenu[] array in sexp.cpp.
enum SEXP_SUBCATEGORIES : int {
	OP_SUBCATEGORY_NONE,
	CHANGE_SUBCATEGORY_MESSAGING,
	CHANGE_SUBCATEGORY_AI_CONTROL,
	CHANGE_SUBCATEGORY_SHIP_STATUS,
	CHANGE_SUBCATEGORY_SHIELDS_ENGINES_AND_WEAPONS,
	CHANGE_SUBCATEGORY_SUBSYSTEMS,
	CHANGE_SUBCATEGORY_CARGO,
	CHANGE_SUBCATEGORY_ARMOR_AND_DAMAGE_TYPES,
	CHANGE_SUBCATEGORY_BEAMS_AND_TURRETS,
	CHANGE_SUBCATEGORY_MODELS_AND_TEXTURES,
	CHANGE_SUBCATEGORY_COORDINATE_MANIPULATION,
	CHANGE_SUBCATEGORY_MISSION_AND_CAMPAIGN,
	CHANGE_SUBCATEGORY_MUSIC_AND_SOUND,
	CHANGE_SUBCATEGORY_HUD,
	CHANGE_SUBCATEGORY_NAV,
	CHANGE_SUBCATEGORY_CUTSCENES,
	CHANGE_SUBCATEGORY_BACKGROUND_AND_NEBULA,
	CHANGE_SUBCATEGORY_JUMP_NODES,
	CHANGE_SUBCATEGORY_SPECIAL_EFFECTS,
	CHANGE_SUBCATEGORY_VARIABLES,
	CHANGE_SUBCATEGORY_CONTAINERS,
	CHANGE_SUBCATEGORY_OTHER,
	STATUS_SUBCATEGORY_MISSION,
	STATUS_SUBCATEGORY_PLAYER,
	STATUS_SUBCATEGORY_MULTIPLAYER,
	STATUS_SUBCATEGORY_SHIP_STATUS,
	STATUS_SUBCATEGORY_SHIELDS_ENGINES_AND_WEAPONS,
	STATUS_SUBCATEGORY_CARGO,
	STATUS_SUBCATEGORY_DAMAGE,
	STATUS_SUBCATEGORY_DISTANCE_AND_COORDINATES,
	STATUS_SUBCATEGORY_VARIABLES,
	STATUS_SUBCATEGORY_CONTAINERS,
	STATUS_SUBCATEGORY_OTHER,
	// this should come after every subcategory
	First_available_subcategory_id
};

enum SEXP_OPERATORS : int {
	OP_NOT_AN_OP = 0, // zero represents a non-operator
	// OP_CATEGORY_ARITHMETIC

	OP_PLUS = FIRST_OP,
	OP_MINUS,
	OP_MOD,
	OP_MUL,
	OP_DIV,
	OP_RAND,
	OP_ABS,	// Goober5000
	OP_MIN,	// Goober5000
	OP_MAX,	// Goober5000
	OP_AVG,	// Goober5000
	OP_RAND_MULTIPLE,	// Goober5000
	OP_POW,	// Goober5000
	OP_BITWISE_AND,	// Goober5000
	OP_BITWISE_OR,	// Goober5000
	OP_BITWISE_NOT,	// Goober5000
	OP_BITWISE_XOR,	// Goober5000

	OP_SET_BIT,	// Goober5000
	OP_UNSET_BIT,	// Goober5000
	OP_IS_BIT_SET,	// Goober5000
	OP_SIGNUM,	// Goober5000
	OP_IS_NAN,	// Goober5000
	OP_NAN_TO_NUMBER,	// Goober5000
	OP_ANGLE_VECTORS,	// Lafiel

	// OP_CATEGORY_LOGICAL

	OP_TRUE,
	OP_FALSE,
	OP_AND,
	OP_AND_IN_SEQUENCE,
	OP_OR,
	OP_EQUALS,
	OP_GREATER_THAN,
	OP_LESS_THAN,
	OP_HAS_TIME_ELAPSED,
	OP_NOT,
	OP_STRING_EQUALS,
	OP_STRING_GREATER_THAN,
	OP_STRING_LESS_THAN,
	OP_NOT_EQUAL,	// Goober5000
	OP_GREATER_OR_EQUAL,	// Goober5000
	OP_LESS_OR_EQUAL,	// Goober5000

	OP_XOR,	// Goober5000
	OP_PERFORM_ACTIONS_BOOL_FIRST,	// Goober5000
	OP_PERFORM_ACTIONS_BOOL_LAST,	// Goober5000

   // OP_CATEGORY_GOAL_EVENT

   OP_GOAL_INCOMPLETE,
   OP_GOAL_TRUE_DELAY,
   OP_GOAL_FALSE_DELAY,
   OP_EVENT_INCOMPLETE,
   OP_EVENT_TRUE_DELAY,
   OP_EVENT_FALSE_DELAY,
   OP_PREVIOUS_EVENT_TRUE,
   OP_PREVIOUS_EVENT_FALSE,
   OP_PREVIOUS_GOAL_TRUE,
   OP_PREVIOUS_GOAL_FALSE,
   OP_EVENT_TRUE_MSECS_DELAY,
   OP_EVENT_FALSE_MSECS_DELAY,

   // OP_CATEGORY_OBJECTIVE

   OP_IS_DESTROYED_DELAY,
   OP_IS_SUBSYSTEM_DESTROYED_DELAY,
   OP_IS_DISABLED_DELAY,
   OP_IS_DISARMED_DELAY,
   OP_HAS_DOCKED_DELAY,
   OP_HAS_UNDOCKED_DELAY,
   OP_HAS_ARRIVED_DELAY,
   OP_HAS_DEPARTED_DELAY,
   OP_WAYPOINTS_DONE_DELAY,
   OP_SHIP_TYPE_DESTROYED,
   OP_PERCENT_SHIPS_DEPARTED,
   OP_PERCENT_SHIPS_DESTROYED,
   OP_DEPART_NODE_DELAY,
   OP_DESTROYED_DEPARTED_DELAY,
   OP_PERCENT_SHIPS_DISARMED,	// Goober5000
   OP_PERCENT_SHIPS_DISABLED,	// Goober5000
   OP_PERCENT_SHIPS_ARRIVED,	// FUBAR-BDHR
   OP_NAV_IS_VISITED,	// Kazan
   OP_WAS_DESTROYED_BY_DELAY,	// WCS

   // OP_CATEGORY_TIME

   OP_TIME_SHIP_DESTROYED,
   OP_TIME_SHIP_ARRIVED,
   OP_TIME_SHIP_DEPARTED,
   OP_TIME_WING_DESTROYED,
   OP_TIME_WING_ARRIVED,
   OP_TIME_WING_DEPARTED,
   OP_MISSION_TIME,
   OP_MISSION_TIME_MSECS,	// Goober5000
   OP_TIME_DOCKED,
   OP_TIME_UNDOCKED,
   OP_TIME_TO_GOAL, // tcrayford

   // OP_CATEGORY_STATUS

   OP_SHIELDS_LEFT,
   OP_HITS_LEFT,
   OP_HITS_LEFT_SUBSYSTEM,	// deprecated
   OP_SIM_HITS_LEFT,
   OP_DISTANCE,
   OP_DISTANCE_CENTER_SUBSYSTEM,	// Goober5000
   OP_LAST_ORDER_TIME,
   OP_NUM_PLAYERS,
   OP_SKILL_LEVEL_AT_LEAST,
   OP_WAS_PROMOTION_GRANTED,
   OP_WAS_MEDAL_GRANTED,
   OP_CARGO_KNOWN_DELAY,
   OP_CAP_SUBSYS_CARGO_KNOWN_DELAY,
   OP_HAS_BEEN_TAGGED_DELAY,
   OP_IS_TAGGED,
   OP_NUM_KILLS,

   OP_NUM_TYPE_KILLS,
   OP_NUM_CLASS_KILLS,
   OP_SHIELD_RECHARGE_PCT,
   OP_ENGINE_RECHARGE_PCT,
   OP_WEAPON_RECHARGE_PCT,
   OP_SHIELD_QUAD_LOW,
   OP_SECONDARY_AMMO_PCT,
   OP_IS_SECONDARY_SELECTED,
   OP_IS_PRIMARY_SELECTED,
   OP_SPECIAL_WARP_DISTANCE,
   OP_IS_SHIP_VISIBLE,
   OP_TEAM_SCORE,
   OP_PRIMARY_AMMO_PCT,	// Goober5000
   OP_IS_SHIP_STEALTHY,	// Goober5000
   OP_IS_CARGO,	// Goober5000
   OP_IS_FRIENDLY_STEALTH_VISIBLE,	// Goober5000

   OP_GET_OBJECT_X,	// Goober5000
   OP_GET_OBJECT_Y,	// Goober5000
   OP_GET_OBJECT_Z,	// Goober5000
   OP_IS_AI_CLASS,	// Goober5000
   OP_IS_SHIP_TYPE,	// Goober5000
   OP_IS_SHIP_CLASS,	// Goober5000
   OP_NUM_SHIPS_IN_BATTLE,	// phreak
   OP_CURRENT_SPEED, // WMCoolmon
   OP_IS_IFF,	// Goober5000
   OP_NUM_WITHIN_BOX,	// WMCoolmon
   OP_SCRIPT_EVAL_NUM, // WMCoolmon
   OP_SCRIPT_EVAL_STRING, // WMCoolmon
   OP_NUM_SHIPS_IN_WING,	// Karajorma
   OP_GET_PRIMARY_AMMO, // Karajorma
   OP_GET_SECONDARY_AMMO, // Karajorma
   OP_NUM_ASSISTS, // Karajorma

   OP_SHIP_SCORE, // Karajorma
   OP_SHIP_DEATHS, // Karajorma
   OP_RESPAWNS_LEFT, // Karajorma
   OP_IS_PLAYER, // Karajorma
   OP_GET_DAMAGE_CAUSED, // Karajorma
   OP_AFTERBURNER_LEFT, // Karajorma
   OP_WEAPON_ENERGY_LEFT, // Karajorma
   OP_PRIMARY_FIRED_SINCE, // Karajorma
   OP_SECONDARY_FIRED_SINCE, // Karajorma
   OP_CUTSCENES_GET_FOV, // Echelon9
   OP_GET_THROTTLE_SPEED, // Karajorma
   OP_HITS_LEFT_SUBSYSTEM_GENERIC, // Goober5000
   OP_HITS_LEFT_SUBSYSTEM_SPECIFIC, // Goober5000
   OP_GET_OBJECT_PITCH,	// Goober5000
   OP_GET_OBJECT_BANK,	// Goober5000
   OP_GET_OBJECT_HEADING,	// Goober5000

   OP_HAS_PRIMARY_WEAPON, // Karajorma
   OP_HAS_SECONDARY_WEAPON, // Karajorma
   OP_STRING_TO_INT, // Karajorma
   OP_STRING_GET_LENGTH, // Goober5000
   OP_GET_OBJECT_SPEED_X,
   OP_GET_OBJECT_SPEED_Y,
   OP_GET_OBJECT_SPEED_Z,
   OP_NAV_DISTANCE, // Kazan
   OP_NAV_ISLINKED, // Kazan
   OP_IS_FACING, // The E
   OP_DIRECTIVE_VALUE, // Karajorma
   OP_GET_NUM_COUNTERMEASURES, // Karajorma
   OP_IS_IN_BOX, // Sushi
   OP_IS_IN_MISSION, // Goober5000
   OP_ARE_SHIP_FLAGS_SET, // Karajorma
   OP_TURRET_GET_PRIMARY_AMMO, // DahBlount, part of the turret ammo code

   OP_TURRET_GET_SECONDARY_AMMO,	// DahBlount, part of the turret ammo code
   OP_IS_DOCKED,	// Goober5000
   OP_IS_IN_TURRET_FOV,	// Goober5000
   OP_GET_HOTKEY, // wookieejedi
   OP_DISTANCE_CENTER, // Goober5000
   OP_DISTANCE_BBOX, // Goober5000
   OP_DISTANCE_BBOX_SUBSYSTEM, // Goober5000
   OP_IS_LANGUAGE,						// Goober5000
   OP_SCRIPT_EVAL_BOOL, // Goober5000
   OP_IS_CONTAINER_EMPTY,	// Karajorma/jg18
   OP_GET_CONTAINER_SIZE,	// Karajorma/jg18
   OP_LIST_HAS_DATA,	// Karajorma/jg18
   OP_LIST_DATA_INDEX,	// Karajorma/jg18
   OP_MAP_HAS_KEY,	// Karajorma/jg18
   OP_MAP_HAS_DATA_ITEM,	// Karajorma/jg18
   OP_ANGLE_FVEC_TARGET, // Lafiel

   OP_ARE_WING_FLAGS_SET, // Goober5000
   OP_PLAYER_IS_CHEATING_BASTARD,	// The E

   // OP_CATEGORY_CONDITIONAL
   // conditional sexpressions

   OP_WHEN,
   OP_WHEN_ARGUMENT,	// Goober5000
   OP_EVERY_TIME,	// Goober5000
   OP_EVERY_TIME_ARGUMENT,	// Goober5000
   OP_ANY_OF,	// Goober5000
   OP_EVERY_OF,	// Goober5000
   OP_RANDOM_OF,	// Goober5000
   OP_NUMBER_OF,	// Goober5000
   OP_INVALIDATE_ARGUMENT,	// Goober5000
   OP_RANDOM_MULTIPLE_OF,	// Karajorma
   OP_IN_SEQUENCE,	// Karajorma
   OP_VALIDATE_ARGUMENT,	// Karajorma
   OP_DO_FOR_VALID_ARGUMENTS,	// Karajorma
   OP_INVALIDATE_ALL_ARGUMENTS,	// Karajorma
   OP_VALIDATE_ALL_ARGUMENTS,	// Karajorma
   OP_FOR_COUNTER,	// Goober5000

   OP_IF_THEN_ELSE,	// Goober5000
   OP_NUM_VALID_ARGUMENTS,	// Karajorma
   OP_FUNCTIONAL_IF_THEN_ELSE,	// Goober5000
   OP_FOR_SHIP_CLASS,	// Goober5000
   OP_FOR_SHIP_TYPE,	// Goober5000
   OP_FOR_SHIP_TEAM,	// Goober5000
   OP_FOR_SHIP_SPECIES,	// Goober5000
   OP_FOR_PLAYERS,	// Goober5000
   OP_FIRST_OF,	// MageKing17
   OP_SWITCH,	// Goober5000
   OP_FUNCTIONAL_SWITCH,	// Goober5000
   OP_FUNCTIONAL_WHEN,	// Goober5000
   OP_FOR_CONTAINER_DATA,	// jg18
   OP_FOR_MAP_CONTAINER_KEYS,	// jg18

   // OP_CATEGORY_CHANGE
   // sexpressions with side-effects

   OP_CHANGE_IFF,
   OP_REPAIR_SUBSYSTEM,
   OP_SABOTAGE_SUBSYSTEM,
   OP_SET_SUBSYSTEM_STRNGTH,
   OP_PROTECT_SHIP,
   OP_SEND_MESSAGE,
   OP_SELF_DESTRUCT,
   OP_CLEAR_GOALS,
   OP_ADD_GOAL,
   OP_REMOVE_GOAL,	// Goober5000
   OP_INVALIDATE_GOAL,
   OP_VALIDATE_GOAL,
   OP_SEND_RANDOM_MESSAGE,
   OP_TRANSFER_CARGO,
   OP_EXCHANGE_CARGO,
   OP_UNPROTECT_SHIP,

   OP_GOOD_REARM_TIME,
   OP_BAD_REARM_TIME,
   OP_GRANT_PROMOTION,
   OP_GRANT_MEDAL,
   OP_ALLOW_SHIP,
   OP_ALLOW_WEAPON,
   OP_GOOD_SECONDARY_TIME,
   OP_WARP_BROKEN,
   OP_WARP_NOT_BROKEN,
   OP_WARP_NEVER,
   OP_WARP_ALLOWED,
   OP_SHIP_INVISIBLE,
   OP_SHIP_VISIBLE,
   OP_SHIP_INVULNERABLE,
   OP_SHIP_VULNERABLE,
   OP_RED_ALERT,

   OP_TECH_ADD_SHIP,
   OP_TECH_ADD_WEAPON,
   OP_END_CAMPAIGN,
   OP_JETTISON_CARGO_DELAY,
   OP_MODIFY_VARIABLE,
   OP_NOP,
   OP_BEAM_FIRE,
   OP_BEAM_FREE,
   OP_BEAM_FREE_ALL,
   OP_BEAM_LOCK,
   OP_BEAM_LOCK_ALL,
   OP_BEAM_PROTECT_SHIP,
   OP_BEAM_UNPROTECT_SHIP,
   OP_TURRET_FREE,
   OP_TURRET_FREE_ALL,
   OP_TURRET_LOCK,

   OP_TURRET_LOCK_ALL,
   OP_ADD_REMOVE_ESCORT,
   OP_AWACS_SET_RADIUS,
   OP_SEND_MESSAGE_LIST,
   OP_CAP_WAYPOINT_SPEED,
   OP_SHIP_GUARDIAN,
   OP_SHIP_NO_GUARDIAN,
   OP_TURRET_TAGGED_ONLY_ALL,
   OP_TURRET_TAGGED_CLEAR_ALL,
   OP_SUBSYS_SET_RANDOM,
   OP_SUPERNOVA_START,
   OP_CARGO_NO_DEPLETE,
   OP_SET_SPECIAL_WARPOUT_NAME,
   OP_SHIP_VANISH,
   OP_SHIELDS_ON,	//-Sesquipedalian
   OP_SHIELDS_OFF,	//-Sesquipedalian

   OP_CHANGE_AI_LEVEL,	//-Sesquipedalian
   OP_END_MISSION, //-Sesquipedalian. replaces end-mission-delay, which did nothing
   OP_SET_SCANNED,	// Goober5000
   OP_SET_UNSCANNED,	// Goober5000
   OP_SHIP_STEALTHY,	// Goober5000
   OP_SHIP_UNSTEALTHY,	// Goober5000
   OP_SET_CARGO,	// Goober5000
   OP_CHANGE_AI_CLASS,	// Goober5000
   OP_FRIENDLY_STEALTH_INVISIBLE,	// Goober5000
   OP_FRIENDLY_STEALTH_VISIBLE,	// Goober5000
   OP_DAMAGED_ESCORT_LIST, //phreak
   OP_DAMAGED_ESCORT_LIST_ALL,	// Goober5000
   OP_SHIP_VAPORIZE,	// Goober5000
   OP_SHIP_NO_VAPORIZE,	// Goober5000
   OP_COLLIDE_INVISIBLE,	// Goober5000
   OP_DONT_COLLIDE_INVISIBLE,	// Goober5000

   OP_PRIMITIVE_SENSORS_SET_RANGE,	// Goober5000
   OP_CHANGE_SHIP_CLASS,	// Goober5000
   OP_SCRIPT_EVAL, //WMC
   OP_SET_SUPPORT_SHIP,	// Goober5000
   OP_DEACTIVATE_GLOW_POINTS,	//-Bobboau
   OP_ACTIVATE_GLOW_POINTS,	//-Bobboau
   OP_DEACTIVATE_GLOW_MAPS,	//-Bobboau
   OP_ACTIVATE_GLOW_MAPS,	//-Bobboau
   OP_DEACTIVATE_GLOW_POINT_BANK,	//-Bobboau
   OP_ACTIVATE_GLOW_POINT_BANK,	//-Bobboau
   OP_CHANGE_SOUNDTRACK,	// Goober5000
   OP_TECH_ADD_INTEL,	// Goober5000
   OP_TECH_RESET_TO_DEFAULT,	// Goober5000
   OP_EXPLOSION_EFFECT,	// Goober5000
   OP_WARP_EFFECT,	// Goober5000
   OP_SET_OBJECT_FACING,	// Goober5000

   OP_SET_OBJECT_FACING_OBJECT,	// Goober5000
   OP_SET_OBJECT_POSITION,	// Goober5000
   OP_PLAY_SOUND_FROM_TABLE,	// Goober5000
   OP_PLAY_SOUND_FROM_FILE,	// Goober5000
   OP_CLOSE_SOUND_FROM_FILE,	// Goober5000
   OP_HUD_DISABLE,	// Goober5000
   OP_KAMIKAZE,	//-Sesquipedalian
   OP_MISSION_SET_SUBSPACE,
   OP_TURRET_TAGGED_SPECIFIC, //phreak
   OP_TURRET_TAGGED_CLEAR_SPECIFIC, //phreak
   OP_LOCK_ROTATING_SUBSYSTEM,	// Goober5000
   OP_FREE_ROTATING_SUBSYSTEM,	// Goober5000
   OP_REVERSE_ROTATING_SUBSYSTEM,	// Goober5000
   OP_ROTATING_SUBSYS_SET_TURN_TIME,	// Goober5000
   OP_PLAYER_USE_AI,	// Goober5000
   OP_PLAYER_NOT_USE_AI,	// Goober5000

   OP_HUD_DISABLE_EXCEPT_MESSAGES,	// Goober5000
   OP_FORCE_JUMP,	// Goober5000
   OP_HUD_SET_TEXT, //WMC
   OP_HUD_SET_TEXT_NUM, //WMC
   OP_HUD_SET_COORDS, //WMC
   OP_HUD_SET_FRAME, //WMC
   OP_HUD_SET_COLOR, //WMC
   OP_HUD_SET_MAX_TARGETING_RANGE, // Goober5000
   OP_SHIP_TAG, // Goober5000
   OP_SHIP_UNTAG, // Goober5000
   OP_SHIP_CHANGE_ALT_NAME,	// Goober5000
   OP_SCRAMBLE_MESSAGES,	// phreak
   OP_UNSCRAMBLE_MESSAGES,	// phreak
   OP_CUTSCENES_SET_CUTSCENE_BARS,	// WMC
   OP_CUTSCENES_UNSET_CUTSCENE_BARS,	// WMC
   OP_CUTSCENES_FADE_IN,	// WMC

   OP_CUTSCENES_FADE_OUT,	// WMC
   OP_CUTSCENES_SET_CAMERA_POSITION,	// WMC
   OP_CUTSCENES_SET_CAMERA_FACING,	// WMC
   OP_CUTSCENES_SET_CAMERA_FACING_OBJECT,	// WMC
   OP_CUTSCENES_SET_CAMERA_ROTATION,	// WMC
   OP_CUTSCENES_SET_FOV,	// WMC
   OP_CUTSCENES_RESET_FOV,	// WMC
   OP_CUTSCENES_RESET_CAMERA,	// WMC
   OP_CUTSCENES_SHOW_SUBTITLE,	// WMC / deprecated
   OP_CUTSCENES_SET_TIME_COMPRESSION,	// WMC
   OP_CUTSCENES_RESET_TIME_COMPRESSION,	// WMC
   OP_CUTSCENES_FORCE_PERSPECTIVE,	// WMC
   OP_JUMP_NODE_SET_JUMPNODE_NAME,	// CommanderDJ
   OP_JUMP_NODE_SET_JUMPNODE_COLOR,	// WMC
   OP_JUMP_NODE_SET_JUMPNODE_MODEL,	// WMC
   OP_JUMP_NODE_SHOW_JUMPNODE,	// WMC

   OP_JUMP_NODE_HIDE_JUMPNODE,	// WMC
   OP_SHIP_GUARDIAN_THRESHOLD,	// Goober5000
   OP_SHIP_SUBSYS_GUARDIAN_THRESHOLD,	// Goober5000
   OP_SET_SKYBOX_MODEL, // taylor
   OP_SHIP_CREATE,
   OP_WEAPON_CREATE,	// Goober5000
   OP_SET_OBJECT_SPEED_X, // Deprecated by wookieejedi
   OP_SET_OBJECT_SPEED_Y, // Deprecated by wookieejedi
   OP_SET_OBJECT_SPEED_Z, // Deprecated by wookieejedi
   OP_MISSION_SET_NEBULA,
   OP_ADD_BACKGROUND_BITMAP,
   OP_REMOVE_BACKGROUND_BITMAP,
   OP_ADD_SUN_BITMAP,
   OP_REMOVE_SUN_BITMAP,
   OP_NEBULA_CHANGE_STORM,
   OP_NEBULA_TOGGLE_POOF,

   OP_TURRET_CHANGE_WEAPON,
   OP_TURRET_SET_TARGET_ORDER,
   OP_SHIP_TURRET_TARGET_ORDER,
   OP_SET_PRIMARY_AMMO, // Karajorma
   OP_SET_SECONDARY_AMMO, // Karajorma
   OP_SHIP_BOMB_TARGETABLE,	//WMC
   OP_SHIP_BOMB_UNTARGETABLE,	//WMC
   OP_SHIP_SUBSYS_TARGETABLE,	// Goober5000
   OP_SHIP_SUBSYS_UNTARGETABLE,	// Goober5000
   OP_SET_DEATH_MESSAGE,	// Goober5000
   OP_SET_PRIMARY_WEAPON, // Karajorma
   OP_SET_SECONDARY_WEAPON, // Karajorma
   OP_DISABLE_BUILTIN_MESSAGES, // Karajorma
   OP_ENABLE_BUILTIN_MESSAGES, // Karajorma
   OP_LOCK_PRIMARY_WEAPON, // Karajorma
   OP_UNLOCK_PRIMARY_WEAPON, // Karajorma

   OP_LOCK_SECONDARY_WEAPON, // Karajorma
   OP_UNLOCK_SECONDARY_WEAPON, // Karajorma
   OP_SET_CAMERA_SHUDDER,	// Goober5000
   OP_ALLOW_TREASON, // Karajorma
   OP_SHIP_COPY_DAMAGE,	// Goober5000
   OP_CHANGE_SUBSYSTEM_NAME,	// Karajorma
   OP_SET_PERSONA, // Karajorma
   OP_CHANGE_PLAYER_SCORE, // Karajorma
   OP_CHANGE_TEAM_SCORE, // Karajorma
   OP_CUTSCENES_SET_CAMERA_FOV,	// WMC
   OP_CUTSCENES_SET_CAMERA, // WMC
   OP_CUTSCENES_SET_CAMERA_HOST, // WMC
   OP_CUTSCENES_SET_CAMERA_TARGET, // WMC
   OP_LOCK_AFTERBURNER, // KeldorKatarn
   OP_UNLOCK_AFTERBURNER, // KeldorKatarn
   OP_SHIP_CHANGE_CALLSIGN,	// FUBAR

   OP_SET_RESPAWNS, // Karajorma
   OP_SET_AFTERBURNER_ENERGY, // Karajorma
   OP_SET_WEAPON_ENERGY, // Karajorma
   OP_SET_SHIELD_ENERGY, // Karajorma
   OP_SET_AMBIENT_LIGHT, // Karajorma
   OP_CHANGE_IFF_COLOR, // Wanderer
   OP_TURRET_SUBSYS_TARGET_DISABLE, // Wanderer
   OP_TURRET_SUBSYS_TARGET_ENABLE, // Wanderer
   OP_CLEAR_WEAPONS, // Wanderer
   OP_SHIP_MANEUVER, // Wanderer 
   OP_SHIP_ROT_MANEUVER, // Wanderer
   OP_SHIP_LAT_MANEUVER, // Wanderer
   OP_GET_VARIABLE_BY_INDEX, // Goober5000
   OP_SET_VARIABLE_BY_INDEX, // Goober5000
   OP_SET_POST_EFFECT, // Hery
   OP_TURRET_SET_OPTIMUM_RANGE, // FUBAR

   OP_TURRET_SET_DIRECTION_PREFERENCE, // FUBAR
   OP_TURRET_SET_TARGET_PRIORITIES, // FUBAR
   OP_SET_ARMOR_TYPE, // FUBAR
   OP_CUTSCENES_SHOW_SUBTITLE_TEXT,	// Goober5000
   OP_CUTSCENES_SHOW_SUBTITLE_IMAGE,	// Goober5000
   OP_HUD_DISPLAY_GAUGE,
   OP_SET_SOUND_ENVIRONMENT,	// Taylor
   OP_UPDATE_SOUND_ENVIRONMENT,	// Taylor
   OP_SET_EXPLOSION_OPTION,	// Goober5000
   OP_ADJUST_AUDIO_VOLUME, // The E
   OP_FORCE_GLIDE, // The E
   OP_TURRET_SET_RATE_OF_FIRE, // FUBAR
   OP_HUD_SET_MESSAGE, // The E
   OP_SHIP_SUBSYS_NO_REPLACE, // FUBAR
   OP_SET_IMMOBILE,	// Goober5000
   OP_SET_MOBILE,	// Goober5000

   OP_SHIP_SUBSYS_NO_LIVE_DEBRIS, // FUBAR
   OP_SHIP_SUBSYS_VANISHED, // FUBAR
   OP_SHIP_SUBSYS_IGNORE_IF_DEAD, // FUBAR
   OP_HUD_SET_DIRECTIVE, // The E
   OP_HUD_GAUGE_SET_ACTIVE, // The E - slightly deprecated
   OP_HUD_ACTIVATE_GAUGE_TYPE, // The E - slightly deprecated
   OP_SET_OBJECT_ORIENTATION,	// Goober5000
   OP_STRING_CONCATENATE,	// Goober5000
   OP_INT_TO_STRING, // Goober5000
   OP_WEAPON_SET_DAMAGE_TYPE, // FUBAR
   OP_SHIP_SET_DAMAGE_TYPE, // FUBAR
   OP_SHIP_SHOCKWAVE_SET_DAMAGE_TYPE, // FUBAR
   OP_FIELD_SET_DAMAGE_TYPE, // FUBAR
   OP_TURRET_PROTECT_SHIP,	// Goober5000
   OP_TURRET_UNPROTECT_SHIP,	// Goober5000
   OP_DISABLE_ETS, // The E

   OP_ENABLE_ETS, // The E
   OP_NAV_ADD_WAYPOINT,	// Kazan
   OP_NAV_ADD_SHIP,	// Kazan
   OP_NAV_DEL,	// Kazan
   OP_NAV_HIDE,	// Kazan
   OP_NAV_RESTRICT,	// Kazan
   OP_NAV_UNHIDE,	// Kazan
   OP_NAV_UNRESTRICT,	// Kazan
   OP_NAV_SET_VISITED,	// Kazan
   OP_NAV_SET_CARRY,	// Kazan
   OP_NAV_UNSET_CARRY,	// Kazan
   OP_NAV_UNSET_VISITED,	// Kazan
   OP_NAV_SET_NEEDSLINK,	// Kazan
   OP_NAV_UNSET_NEEDSLINK,	// Kazan
   OP_NAV_USECINEMATICS,	// Kazan
   OP_NAV_USEAP,	// Kazan

   // OP_CATEGORY_CHANGE2

   OP_STRING_GET_SUBSTRING,	// Goober5000
   OP_STRING_SET_SUBSTRING,	// Goober5000
   OP_SET_NUM_COUNTERMEASURES, // Karajorma
   OP_ADD_TO_COLGROUP, // The E
   OP_REMOVE_FROM_COLGROUP, // The E
   OP_GET_COLGROUP_ID, // The E
   OP_SHIP_EFFECT, // Valathil
   OP_CLEAR_SUBTITLES, // The E
   OP_BEAM_FIRE_COORDS,	// Goober5000
   OP_SET_DOCKED, // Sushi
   OP_SET_THRUSTERS, // The E
   OP_TRIGGER_SUBMODEL_ANIMATION,	// Goober5000
   OP_HUD_CLEAR_MESSAGES, // Swifty
   OP_SET_PLAYER_ORDERS,	// Karajorma
   OP_SUPERNOVA_STOP, //CommanderDJ
   OP_SET_PLAYER_THROTTLE_SPEED, //CommanderDJ

   OP_SET_DEBRIEFING_TOGGLED,	// Goober5000
   OP_SET_SUBSPACE_DRIVE,	// Goober5000
   OP_SET_ARRIVAL_INFO,	// Goober5000
   OP_SET_DEPARTURE_INFO,	// Goober5000
   OP_SET_SKYBOX_ORIENT,	// Goober5000
   OP_DESTROY_INSTANTLY,	// Admiral MS
   OP_DESTROY_SUBSYS_INSTANTLY,	// Admiral MS
   OP_DEBUG,	// Karajorma
   OP_SET_MISSION_MOOD,	// Karajorma
   OP_NAV_SELECT, 	// Talon1024
   OP_NAV_UNSELECT, 	// Talon1024
   OP_ALTER_SHIP_FLAG,	// Karajorma
   OP_CHANGE_TEAM_COLOR,	// The E
   OP_NEBULA_CHANGE_PATTERN,	// Axem
   OP_SET_WING_FORMATION,	// Goober5000
   OP_TECH_ADD_INTEL_XSTR,	// Goober5000

   OP_COPY_VARIABLE_FROM_INDEX, // Goober5000
   OP_COPY_VARIABLE_BETWEEN_INDEXES, // Goober5000
   OP_GET_ETS_VALUE,	// niffiwan
   OP_SET_ETS_VALUES,	// niffiwan
   OP_CALL_SSM_STRIKE, // X3N0-Life-Form
   OP_SET_MOTION_DEBRIS,    // The E
   OP_HUD_SET_CUSTOM_GAUGE_ACTIVE, 	// The E, just revamped a bit by Axem
   OP_HUD_SET_BUILTIN_GAUGE_ACTIVE, 	// The E, just revamped a bit by Axem
   OP_SCRIPT_EVAL_MULTI,	// Karajorma
   OP_PAUSE_SOUND_FROM_FILE,	// Goober5000
   OP_SCRIPT_EVAL_BLOCK, // niffiwan
   OP_BEAM_FLOATING_FIRE,	// MageKing17
   OP_TURRET_SET_PRIMARY_AMMO,	// DahBlount, part of the turret ammo changes
   OP_TURRET_SET_SECONDARY_AMMO,	// DahBlount, part of the turret ammo changes
   OP_JETTISON_CARGO_NEW,	// Goober5000
   OP_STRING_CONCATENATE_BLOCK,	// Goober5000

   OP_MODIFY_VARIABLE_XSTR,	// m!m
   OP_RESET_POST_EFFECTS,	// Goober5000
   OP_ADD_REMOVE_HOTKEY,    // wookieejedi
   OP_TECH_REMOVE_INTEL_XSTR,    // wookieejedi
   OP_TECH_REMOVE_INTEL,   // wookieejedi
   OP_CHANGE_BACKGROUND,	// Goober5000
   OP_CLEAR_DEBRIS,	// Goober5000
   OP_SET_DEBRIEFING_PERSONA,	// Goober5000
   OP_ADD_TO_COLGROUP_NEW,	// Goober5000
   OP_REMOVE_FROM_COLGROUP_NEW,	// Goober5000
   OP_GET_POWER_OUTPUT,	// The E
   OP_TURRET_SET_FORCED_TARGET,	// Asteroth
   OP_TURRET_SET_FORCED_SUBSYS_TARGET,	// Asteroth
   OP_TURRET_CLEAR_FORCED_TARGET,	// Asteroth
   OP_SEND_MESSAGE_CHAIN,	// Goober5000
   OP_TURRET_SET_INACCURACY,	// Asteroth

   OP_REPLACE_TEXTURE,	// Lafiel
   OP_NEBULA_CHANGE_FOG_COLOR,	// Asteroth
   OP_SET_ALPHA_MULT,	// Lafiel
   OP_DESTROY_INSTANTLY_WITH_DEBRIS,	// Asteroth
   OP_TRIGGER_ANIMATION_NEW,	// Lafiel
   OP_UPDATE_MOVEABLE,	// Lafiel
   OP_NAV_SET_COLOR, 	// Goober5000
   OP_NAV_SET_VISITED_COLOR, 	// Goober5000
   OP_CONTAINER_ADD_TO_LIST,	// Karajorma/jg18
   OP_CONTAINER_REMOVE_FROM_LIST,	// Karajorma/jg18
   OP_CONTAINER_ADD_TO_MAP,	// Karajorma/jg18
   OP_CONTAINER_REMOVE_FROM_MAP,	// Karajorma/jg18
   OP_CONTAINER_GET_MAP_KEYS,	// Karajorma/jg18
   OP_CLEAR_CONTAINER,	// Karajorma/jg18
   OP_ADD_BACKGROUND_BITMAP_NEW,	// Goober5000
   OP_ADD_SUN_BITMAP_NEW,	// Goober5000

   OP_CANCEL_FUTURE_WAVES,	// naomimyselfandi
   OP_COPY_CONTAINER,	// jg18
   OP_APPLY_CONTAINER_FILTER,	// jg18
   OP_STOP_LOOPING_ANIMATION,	// Lafiel
   OP_LOCK_TRANSLATING_SUBSYSTEM,	// Goober5000
   OP_FREE_TRANSLATING_SUBSYSTEM,	// Goober5000
   OP_REVERSE_TRANSLATING_SUBSYSTEM,	// Goober5000
   OP_TRANSLATING_SUBSYS_SET_SPEED,	// Goober5000
   OP_ALTER_WING_FLAG,	// Goober5000
   OP_TOGGLE_ASTEROID_FIELD,	// MjnMixael
   OP_HUD_FORCE_SENSOR_STATIC,	// MjnMixael
   OP_SET_GRAVITY_ACCEL,	// Asteroth
   OP_SET_ORDER_ALLOWED_TARGET,	// MjnMixael
   OP_USED_CHEAT,	// Kiloku
   OP_SET_ASTEROID_FIELD,	// MjnMixael
   OP_SET_DEBRIS_FIELD,	// MjnMixael

   // OP_CATEGORY_AI
   // defined for AI goals

   OP_AI_CHASE,
   OP_AI_DOCK,
   OP_AI_UNDOCK,
   OP_AI_WARP_OUT,
   OP_AI_WAYPOINTS,
   OP_AI_WAYPOINTS_ONCE,
   OP_AI_DESTROY_SUBSYS,
   OP_AI_DISABLE_SHIP,
   OP_AI_DISARM_SHIP,
   OP_AI_GUARD,
   OP_AI_CHASE_ANY,
   OP_AI_EVADE_SHIP,
   OP_AI_STAY_NEAR_SHIP,
   OP_AI_KEEP_SAFE_DISTANCE,
   OP_AI_IGNORE,
   OP_AI_STAY_STILL,
   OP_AI_PLAY_DEAD,
   OP_AI_IGNORE_NEW,	// Goober5000
   OP_AI_FORM_ON_WING, // The E
   OP_AI_CHASE_SHIP_CLASS,	// Goober5000
   OP_AI_PLAY_DEAD_PERSISTENT,	// Goober5000
   OP_AI_FLY_TO_SHIP,	// Goober5000
   OP_AI_REARM_REPAIR,	// Goober5000

   // OP_CATEGORY_UNLISTED

   OP_GOALS_ID,
   OP_NEXT_MISSION,		// used in campaign files for branching
   OP_IS_DESTROYED,
   OP_IS_SUBSYSTEM_DESTROYED,
   OP_IS_DISABLED,
   OP_IS_DISARMED,
   OP_HAS_DOCKED,
   OP_HAS_UNDOCKED,
   OP_HAS_ARRIVED,
   OP_HAS_DEPARTED,
   OP_WAYPOINTS_DONE,
   OP_ADD_SHIP_GOAL,
   OP_CLEAR_SHIP_GOALS,
   OP_ADD_WING_GOAL,
   OP_CLEAR_WING_GOALS,
   OP_AI_CHASE_WING,
   OP_AI_GUARD_WING,
   OP_EVENT_TRUE,
   OP_EVENT_FALSE,
   OP_PREVIOUS_GOAL_INCOMPLETE,
   OP_PREVIOUS_EVENT_INCOMPLETE,
   OP_AI_WARP,
   OP_IS_CARGO_KNOWN,
   OP_COND,
   OP_END_OF_CAMPAIGN,

   // OP_CATEGORY_TRAINING
   // training sexps

   OP_KEY_PRESSED,
   OP_KEY_RESET,
   OP_TARGETED,
   OP_SPEED,
   OP_FACING,
   OP_ORDER,
   OP_WAYPOINT_MISSED,
   OP_PATH_FLOWN,
   OP_WAYPOINT_TWICE,
   OP_TRAINING_MSG,
   OP_FLASH_HUD_GAUGE,
   OP_SPECIAL_CHECK,
   OP_SECONDARIES_DEPLETED,
   OP_FACING2,
   OP_PRIMARIES_DEPLETED,	// Goober5000
   OP_MISSILE_LOCKED,	// Sesquipedalian
   OP_SET_TRAINING_CONTEXT_FLY_PATH,
   OP_SET_TRAINING_CONTEXT_SPEED,
   OP_KEY_RESET_MULTIPLE,	// Goober5000
   OP_RESET_ORDERS, // Karajorma
   OP_QUERY_ORDERS, // Karajorma
   OP_NODE_TARGETED, // FUBAR
   OP_IGNORE_KEY, // Karajorma

   // this should come after every operator
   First_available_operator_id
};

// defines for string constants
#define SEXP_HULL_STRING			"Hull"
#define SEXP_SIM_HULL_STRING		"Simulated Hull"
#define SEXP_SHIELD_STRING			"Shields"
#define SEXP_ALL_ENGINES_STRING		"<all engines>"
#define SEXP_ALL_TURRETS_STRING		"<all turrets>"
#define SEXP_ARGUMENT_STRING		"<argument>"
#define SEXP_NONE_STRING			"<none>"
#define SEXP_ANY_STRING				"<any string>"
#define SEXP_ALL_BANKS_STRING		"<all weapon banks>"

// macros for accessing sexpression atoms
/**
 * @brief Returns the first element of a SEXP list
 *
 * The name CAR originates from the original LISP language where it was a function which retrieved the first element of
 * a list.
 *
 * @see https://en.wikipedia.org/wiki/CAR_and_CDR
 */
#define CAR(n)		((n < 0) ? -1 : Sexp_nodes[n].first)
/**
 * @brief Returns the rest of a SEXP list. The rest is everything starting from the second element.
 *
 * The name CDR originates from the original LISP language where it was a function which retrieved the "rest" of a list.
 *
 * @see https://en.wikipedia.org/wiki/CAR_and_CDR
 */
#define CDR(n)		((n < 0) ? -1 : Sexp_nodes[n].rest)
#define CADR(n)		CAR(CDR(n))
// #define CTEXT(n)	(Sexp_nodes[n].text)
const char *CTEXT(int n);

// added by Goober5000
#define CDDR(n)		CDR(CDR(n))
#define CDDDR(n)	CDR(CDDR(n))
#define CDDDDR(n)	CDR(CDDDR(n))
#define CDDDDDR(n)	CDR(CDDDDR(n))
#define CADDR(n)	CAR(CDDR(n))
#define CADDDR(n)	CAR(CDDDR(n))
#define CADDDDR(n)	CAR(CDDDDR(n))
#define CADDDDDR(n)	CAR(CDDDDDR(n))

#define REF_TYPE_SHIP		1
#define REF_TYPE_WING		2
#define REF_TYPE_PLAYER		3
#define REF_TYPE_WAYPOINT	4
#define REF_TYPE_PATH		5	// waypoint path

#define SRC_SHIP_ARRIVAL	0x10000
#define SRC_SHIP_DEPARTURE	0x20000
#define SRC_WING_ARRIVAL	0x30000
#define SRC_WING_DEPARTURE	0x40000
#define SRC_EVENT				0x50000
#define SRC_MISSION_GOAL	0x60000
#define SRC_SHIP_ORDER		0x70000
#define SRC_WING_ORDER		0x80000
#define SRC_DEBRIEFING		0x90000
#define SRC_BRIEFING			0xa0000
#define SRC_UNKNOWN			0xffff0000
#define SRC_MASK				0xffff0000
#define SRC_DATA_MASK		0xffff

#define SEXP_MODE_GENERAL	0
#define SEXP_MODE_CAMPAIGN	1

// defines for type field of sexp nodes.  The actual type of the node will be stored in the lower
// two bytes of the field.  The upper two bytes will be used for flags (bleah...)
// Be sure not to conflict with type field of sexp_variable
#define SEXP_NOT_USED		0
#define SEXP_LIST				1
#define SEXP_ATOM				2

// flags for sexpressions -- masked onto the end of the type field
#define SEXP_FLAG_PERSISTENT				(1<<31)		// should this sexp node be persistant across missions
#define SEXP_FLAG_VARIABLE					(1<<30)

// sexp variable definitions
#define SEXP_VARIABLE_CHAR					('@')
// defines for type field of sexp_variable.  Be sure not to conflict with type field of sexp_node
#define SEXP_VARIABLE_NUMBER				(1<<4)	//	(0x0010)
#define SEXP_VARIABLE_STRING				(1<<5)	//	(0x0020)
#define SEXP_VARIABLE_UNKNOWN				(1<<6)	//	(0x0040)
#define SEXP_VARIABLE_NOT_USED				(1<<7)	//	(0x0080)

#define SEXP_VARIABLE_BLOCK					(1<<0)	//	(0x0001)
/*
#define SEXP_VARIABLE_BLOCK_EXP				(1<<1)	//	(0x0002)
#define SEXP_VARIABLE_BLOCK_HIT				(1<<2)	//	(0x0004)
*/
#define SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE		(1<<3)	//	(0x0008)

// Goober5000 - hopefully this should work and not conflict with anything
#define SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS	(1<<29)	//	(0x0100)
//Karajorma
#define SEXP_VARIABLE_NETWORK				(1<<28)
#define SEXP_VARIABLE_SAVE_TO_PLAYER_FILE	(1<<27)

#define SEXP_VARIABLE_IS_PERSISTENT (SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS|SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE)

#define BLOCK_EXP_SIZE					6
#define INNER_RAD							0
#define OUTER_RAD							1
#define DAMAGE								2
#define BLAST								3
#define PROPAGATE							4
#define SHOCK_SPEED						5

#define BLOCK_HIT_SIZE					2
#define SHIELD_STRENGTH					0
#define HULL_STRENGTH					1


#define SEXP_VARIABLE_SET				(0x0100)
#define SEXP_VARIABLE_MODIFIED		(0x0200)

#define SEXP_TYPE_MASK(t)	(t & 0x00ff)
#define SEXP_NODE_TYPE(n)	(Sexp_nodes[n].type & 0x00ff)

// defines for subtypes of atoms
#define SEXP_ATOM_LIST				0
#define SEXP_ATOM_OPERATOR			1
#define SEXP_ATOM_NUMBER			2
#define SEXP_ATOM_STRING			3
#define SEXP_ATOM_CONTAINER_NAME	4
#define SEXP_ATOM_CONTAINER_DATA	5

// defines to short circuit evaluation when possible. Also used when goals can't
// be satisfied yet because ship (or wing) hasn't been created yet.

#define SEXP_TRUE			1
#define SEXP_FALSE			0

// Goober5000: changed these to unlikely values, because now we have sexps using negative numbers
#define SEXP_KNOWN_FALSE	(INT_MIN+10)
#define SEXP_KNOWN_TRUE		(INT_MIN+11)
#define SEXP_UNKNOWN		(INT_MIN+12)
#define SEXP_NAN			(INT_MIN+13)	// not a number -- used when ships/wing part of boolean and haven't arrived yet
#define SEXP_NAN_FOREVER	(INT_MIN+14)	// not a number and will never change -- used to falsify boolean sexpressions
#define SEXP_CANT_EVAL		(INT_MIN+15)	// can't evaluate yet for whatever reason (acts like false)
#define SEXP_NUM_EVAL		(INT_MIN+16)	// already completed an arithmetic operation and result is stored
// in case we want to test for any of the above
#define SEXP_UNLIKELY_RETURN_VALUE_BOUND		(INT_MIN+17)

// defines for check_sexp_syntax
enum sexp_error_check
{
	SEXP_CHECK_NO_ERROR = 0,

	SEXP_CHECK_NONOP_ARGS,              // non-operator has arguments
	SEXP_CHECK_OP_EXPECTED,             // operator expected, but found data instead
	SEXP_CHECK_UNKNOWN_OP,              // unrecognized operator
	SEXP_CHECK_TYPE_MISMATCH,           // return type or data type mismatch
	SEXP_CHECK_BAD_ARG_COUNT,           // argument count in incorrect
	SEXP_CHECK_UNKNOWN_TYPE,            // unrecognized return type of data type

	SEXP_CHECK_INVALID_NUM = 101,       // number is not valid
	SEXP_CHECK_INVALID_SHIP,            // invalid ship name
	SEXP_CHECK_INVALID_WING,            // invalid wing name
	SEXP_CHECK_INVALID_SUBSYS,          // invalid subsystem
	SEXP_CHECK_INVALID_IFF,             // invalid iff string
	SEXP_CHECK_INVALID_POINT,           // invalid point
	SEXP_CHECK_NEGATIVE_NUM,            // negative number wasn't allowed
	SEXP_CHECK_INVALID_SHIP_WING,       // invalid ship/wing
	SEXP_CHECK_INVALID_SHIP_TYPE,       // invalid ship type
	SEXP_CHECK_UNKNOWN_MESSAGE,         // invalid message
	SEXP_CHECK_INVALID_PRIORITY,        // invalid priority for a message
	SEXP_CHECK_INVALID_MISSION_NAME,    // invalid mission name
	SEXP_CHECK_INVALID_GOAL_NAME,       // invalid goal name
	SEXP_CHECK_INVALID_LEVEL,           // mission level too high in campaign
	SEXP_CHECK_INVALID_MSG_SOURCE,      // invalid 'who-from' for a message being sent
	SEXP_CHECK_INVALID_DOCKER_POINT,
	SEXP_CHECK_INVALID_DOCKEE_POINT,
	SEXP_CHECK_ORDER_NOT_ALLOWED,       // ship goal (order) isn't allowed for given ship
	SEXP_CHECK_DOCKING_NOT_ALLOWED,
	SEXP_CHECK_NUM_RANGE_INVALID,
	SEXP_CHECK_INVALID_EVENT_NAME,
	SEXP_CHECK_INVALID_SKILL_LEVEL,
	SEXP_CHECK_INVALID_MEDAL_NAME,
	SEXP_CHECK_INVALID_WEAPON_NAME,
	SEXP_CHECK_INVALID_SHIP_CLASS_NAME,
	SEXP_CHECK_INVALID_CUSTOM_HUD_GAUGE,
	SEXP_CHECK_INVALID_JUMP_NODE,
	SEXP_CHECK_INVALID_VARIABLE,
	SEXP_CHECK_INVALID_AI_CLASS,
	SEXP_CHECK_UNKNOWN_ERROR,
	SEXP_CHECK_INVALID_SUPPORT_SHIP_CLASS,
	SEXP_CHECK_INVALID_SHIP_WITH_BAY,
	SEXP_CHECK_INVALID_ARRIVAL_LOCATION,
	SEXP_CHECK_INVALID_DEPARTURE_LOCATION,
	SEXP_CHECK_INVALID_ARRIVAL_ANCHOR_ALL,
	SEXP_CHECK_INVALID_SOUNDTRACK_NAME,
	SEXP_CHECK_INVALID_INTEL_NAME,
	SEXP_CHECK_INVALID_SKYBOX_NAME,
	SEXP_CHECK_INVALID_PERSONA_NAME,
	SEXP_CHECK_INVALID_VARIABLE_TYPE,
	SEXP_CHECK_INVALID_SUBSYS_TYPE,
	SEXP_CHECK_INVALID_FONT,
	SEXP_CHECK_INVALID_HUD_ELEMENT,
	SEXP_CHECK_INVALID_SOUND_ENVIRONMENT,
	SEXP_CHECK_INVALID_SOUND_ENVIRONMENT_OPTION,
	SEXP_CHECK_INVALID_EXPLOSION_OPTION,
	SEXP_CHECK_INVALID_SHIP_EFFECT,
	SEXP_CHECK_INVALID_TURRET_TARGET_ORDER,
	SEXP_CHECK_INVALID_ARMOR_TYPE,
	SEXP_CHECK_INVALID_DAMAGE_TYPE,
	SEXP_CHECK_INVALID_TARGET_PRIORITIES,
	SEXP_CHECK_INVALID_AUDIO_VOLUME_OPTION,
	SEXP_CHECK_INVALID_BUILTIN_HUD_GAUGE,
	SEXP_CHECK_INVALID_ANIMATION_TYPE,
	SEXP_CHECK_INVALID_MISSION_MOOD,
	SEXP_CHECK_INVALID_SHIP_FLAG,
	SEXP_CHECK_INVALID_TEAM_COLOR,
	SEXP_CHECK_INVALID_SKYBOX_FLAG,
	SEXP_CHECK_INVALID_GAME_SND,
	SEXP_CHECK_INVALID_SSM_CLASS,
	SEXP_CHECK_INVALID_FIREBALL,
	SEXP_CHECK_INVALID_SPECIES,
	SEXP_CHECK_INVALID_FUNCTIONAL_WHEN_EVAL_TYPE,
	SEXP_CHECK_MISPLACED_SPECIAL_ARGUMENT,
	SEXP_CHECK_AMBIGUOUS_GOAL_NAME,
	SEXP_CHECK_AMBIGUOUS_EVENT_NAME,
	SEXP_CHECK_MISSING_CONTAINER_MODIFIER,
	SEXP_CHECK_INVALID_LIST_MODIFIER,
	SEXP_CHECK_WRONG_MAP_KEY_TYPE,
	SEXP_CHECK_WRONG_CONTAINER_TYPE,
	SEXP_CHECK_INVALID_ANIMATION,
	SEXP_CHECK_WRONG_CONTAINER_DATA_TYPE,
	SEXP_CHECK_INVALID_SPECIAL_ARG_TYPE,
	SEXP_CHECK_INVALID_AWACS_SUBSYS,
	SEXP_CHECK_INVALID_ROTATING_SUBSYS,
	SEXP_CHECK_INVALID_TRANSLATING_SUBSYS,
	SEXP_CHECK_INVALID_ANY_HUD_GAUGE,
	SEXP_CHECK_INVALID_WING_FLAG,
	SEXP_CHECK_INVALID_WING_FORMATION,
};


#define TRAINING_CONTEXT_SPEED		(1<<0)
#define TRAINING_CONTEXT_FLY_PATH	(1<<1)

// numbers used in special_training_check() function
#define SPECIAL_CHECK_TRAINING_FAILURE	2000

typedef struct sexp_ai_goal_link {
	int ai_goal;
	int op_code;
} sexp_ai_goal_link;


enum class sexp_oper_type
{
	NONE = 0,
	CONDITIONAL,
	ARGUMENT,
	ACTION,
	ARITHMETIC,
	BOOLEAN,
	INTEGER,
	GOAL
};

typedef struct sexp_oper {
	SCP_string text;
	int	value;
	int	min, max;
	sexp_oper_type type;
} sexp_oper;

// Goober5000
struct sexp_cached_data
{
	int sexp_node_data_type = OPF_NONE;		// an OPF_ #define
	int numeric_literal = 0;				// i.e. a number
	int ship_registry_index = -1;			// because ship status is pretty common
	void *pointer = nullptr;				// could be an IFF, a wing, a goal, or other unchanging reference
	// jg18 - used to store result from sexp_container_CTEXT()
	char container_CTEXT_result[TOKEN_LENGTH] = "";

	sexp_cached_data() = default;

	sexp_cached_data(int _sexp_node_data_type)
		: sexp_node_data_type(_sexp_node_data_type)
	{}

	sexp_cached_data(int _sexp_node_data_type, void *_pointer)
		: sexp_node_data_type(_sexp_node_data_type), pointer(_pointer)
	{}

	sexp_cached_data(int _sexp_node_data_type, int _numeric_literal, int _ship_registry_index)
		: sexp_node_data_type(_sexp_node_data_type), numeric_literal(_numeric_literal), ship_registry_index(_ship_registry_index)
	{}

	sexp_cached_data(int _sexp_node_data_type, const SCP_string &_container_CTEXT_result)
		: sexp_node_data_type(_sexp_node_data_type)
	{
		update_container_CTEXT_result(_container_CTEXT_result);
	}

	void update_container_CTEXT_result(const SCP_string &_container_CTEXT_result)
	{
		if (_container_CTEXT_result.empty()) {
			Warning(LOCATION, "assigning empty string to SEXP node text");
		} else if (_container_CTEXT_result.length() >= sizeof(container_CTEXT_result)) {
			Warning(LOCATION,
				"attempt to assign CTEXT() result %s which is too long (limit %d)",
				_container_CTEXT_result.c_str(),
				(int)(sizeof(container_CTEXT_result) - 1));
		}

		const auto length = _container_CTEXT_result.copy(container_CTEXT_result, sizeof(container_CTEXT_result) - 1);
		container_CTEXT_result[length] = 0;
	}
};

typedef struct sexp_node {
	char	text[TOKEN_LENGTH];
	int op_index;				// the index in the Operators array for the operator at this node (or -1 if not an operator)
	int	type;						// atom, list, or not used
	int	subtype;					// type of atom or list?
	int	first;					// if first parameter is sexp, index into Sexp_nodes
	int	rest;						// index into Sexp_nodes of rest of parameters
	int	value;					// known to be true, known to be false, or not known
	int flags;					// Goober5000

	sexp_cached_data *cache;	// Goober5000
	int cached_variable_index;	// Goober5000
} sexp_node;

// Goober5000
#define SNF_ARGUMENT_VALID			(1<<0)
#define SNF_ARGUMENT_SELECT			(1<<1)
#define SNF_SPECIAL_ARG_IN_NODE		(1<<2)
#define SNF_SPECIAL_ARG_IN_TREE		(1<<3)
#define SNF_SPECIAL_ARG_NOT_IN_TREE	(1<<4)
#define SNF_CHECKED_ARG_FOR_VAR		(1<<5)
#define SNF_DEFAULT_VALUE			SNF_ARGUMENT_VALID

typedef struct sexp_variable {
	int		type;
	char	text[TOKEN_LENGTH];
	char	variable_name[TOKEN_LENGTH];
} sexp_variable;

// next define used to eventually mark a directive as satisfied even though there may be more
// waves for a wing.  bascially a hack for the directives display.
#define DIRECTIVE_WING_ZERO		-999

// Goober5000 - it's dynamic now
//extern sexp_node Sexp_nodes[MAX_SEXP_NODES];

extern int Num_sexp_nodes;
extern sexp_node *Sexp_nodes;

extern sexp_variable Sexp_variables[MAX_SEXP_VARIABLES];
extern sexp_variable Block_variables[MAX_SEXP_VARIABLES];

extern SCP_vector<sexp_oper> Operators;

extern int Locked_sexp_true, Locked_sexp_false;
extern int Directive_count;
extern int Sexp_useful_number;  // a variable to pass useful info in from external modules
extern int Training_context;
extern int Training_context_speed_min;
extern int Training_context_speed_max;
extern int Training_context_speed_set;
extern int Training_context_speed_timestamp;
extern waypoint_list *Training_context_path;
extern int Training_context_goal_waypoint;
extern int Training_context_at_waypoint;
extern float Training_context_distance;
extern int Players_target;
extern int Players_mlocked;
extern ship_subsys *Players_targeted_subsys;
extern int Players_target_timestamp;
extern int Players_mlocked_timestamp;
extern int Sexp_clipboard;  // used by Fred

extern SCP_vector<int> Current_sexp_operator;


// event log stuff
extern SCP_vector<SCP_string> *Current_event_log_buffer;
extern SCP_vector<SCP_string> *Current_event_log_variable_buffer;
extern SCP_vector<SCP_string> *Current_event_log_container_buffer;
extern SCP_vector<SCP_string> *Current_event_log_argument_buffer;

extern void init_sexp();
extern void sexp_startup();
extern void sexp_shutdown();
extern int alloc_sexp(const char *text, int type, int subtype, int first, int rest);
extern int find_free_sexp();
extern int free_one_sexp(int num);
extern int free_sexp(int num, int calling_node = -1);
extern int free_sexp2(int num, int calling_node = -1);
extern int dup_sexp_chain(int node);
extern int cmp_sexp_chains(int node1, int node2);
extern int find_sexp_list(int num);
extern int find_parent_operator(int num);
extern int is_sexp_top_level( int node );

// Goober5000 - renamed these to be more clear, to prevent bugs :p
extern int get_operator_index(const char *token);
extern int get_operator_index(int node);
extern int get_operator_const(const char *token);
extern int get_operator_const(int node);

extern int check_sexp_syntax(int node, int return_type = OPR_BOOL, int recursive = 0, int *bad_node = 0 /*NULL*/, int mode = 0);
extern int get_sexp_main(void);	//	Returns start node
extern int run_sexp(const char* sexpression, bool run_eval_num = false, bool *is_nan_or_nan_forever = nullptr); // debug and lua sexps
extern int stuff_sexp_variable_list();
extern int eval_sexp(int cur_node, int referenced_node = -1);
extern int eval_num(int n, bool &is_nan, bool &is_nan_forever);
extern bool is_sexp_true(int cur_node, int referenced_node = -1);
extern int query_operator_return_type(int op);
extern int query_operator_argument_type(int op, int argnum);
extern void update_sexp_references(const char *old_name, const char *new_name);
extern void update_sexp_references(const char *old_name, const char *new_name, int format);
extern int query_referenced_in_sexp(int mode, const char *name, int *node);
extern void stuff_sexp_text_string(SCP_string &dest, int node, int mode);
extern int build_sexp_string(SCP_string &accumulator, int cur_node, int level, int mode);
extern int sexp_query_type_match(int opf, int opr);
extern bool sexp_recoverable_error(int num);
extern const char *sexp_error_message(int num);
extern int count_free_sexp_nodes();

struct ship_registry_entry;
struct wing;

// Goober5000 - stuff with caching
// (included in the header file because Lua uses the first three)
extern const ship_registry_entry *eval_ship(int node);
extern wing *eval_wing(int node);
extern int sexp_get_variable_index(int node);
extern int sexp_atoi(int node);
extern bool sexp_can_construe_as_integer(int node);

// Goober5000
void do_action_for_each_special_argument(int cur_node);
bool special_argument_appears_in_sexp_tree(int node);
bool special_argument_appears_in_sexp_list(int node);

// functions to change the attributes of an sexpression tree to persistent or not persistent
extern void sexp_unmark_persistent( int n );
extern void sexp_mark_persistent( int n );
extern int verify_sexp_tree(int node);
extern int query_sexp_ai_goal_valid(int sexp_ai_goal, int ship);
int query_node_in_sexp(int node, int sexp);
void flush_sexp_tree(int node);

// sexp_variable
void sexp_modify_variable(const char *text, int index, bool sexp_callback = true);
int get_index_sexp_variable_name(const char *text);
int get_index_sexp_variable_name(SCP_string &text);	// Goober5000
int get_index_sexp_variable_name_special(const char *text);	// Goober5000
int get_index_sexp_variable_name_special(SCP_string &text, size_t startpos);	// Goober5000
bool sexp_replace_variable_names_with_values(char *text, int max_len);	// Goober5000
bool sexp_replace_variable_names_with_values(SCP_string &text);	// Goober5000
int get_nth_variable_index(int nth, int variable_type);	// Karajorma
int sexp_variable_count();
int sexp_campaign_file_variable_count();	// Goober5000
int sexp_variable_typed_count(int sexp_variables_index, int variable_type); // Karajorma
void sexp_variable_delete(int index);
void sexp_variable_sort();
void sexp_fred_modify_variable(const char *text, const char *var_name, int index, int type);
int sexp_add_variable(const char *text, const char *var_name, int type, int index=-1);
bool generate_special_explosion_block_variables();
int num_block_variables();
bool has_special_explosion_block_index(ship *shipp, int *index);

extern bool usable_in_campaign(int op_id);
extern int get_category(int op_id);
extern int category_of_subcategory(int subcategory_id);
extern int get_subcategory(int op_id);
extern const char *get_category_name(int category_id);

// Goober5000
extern void sexp_music_close();

//WMC - moved here from FRED
typedef struct sexp_help_struct {
	int id;
	SCP_string help;
} sexp_help_struct;

extern SCP_vector<sexp_help_struct> Sexp_help;

typedef struct op_menu_struct {
	SCP_string name;
	int id;
} op_menu_struct;

extern SCP_vector<op_menu_struct> op_menu;
extern SCP_vector<op_menu_struct> op_submenu;

//WMC
//Outputs sexp.html file
bool output_sexps(const char *filepath);

void multi_sexp_eval();

// Goober5000/Taylor
extern int Num_sound_environment_options;
extern const char *Sound_environment_option[];

// Goober5000
extern int Num_explosion_options;
extern const char *Explosion_option[];

extern int Num_functional_when_eval_types;
extern const char *Functional_when_eval_type[];

//The E
extern int Num_adjust_audio_options;
extern const char *Adjust_audio_options[];

extern int Num_skybox_flags;
extern const char *Skybox_flags[];

/** Global state variables for the hud-display-gauge sexp.
They all should be named Sexp_hud_display_*;
They all should follow the following symantics for the value of the
variable:
=0	don't show
=1	show until canceled
>1	timestamp when gauge should stop showing (set zero when expired)
\sa sexp_hud_display_warpout
*/
extern int Sexp_hud_display_warpout;

//Needed for scripting access to ship effects
int get_effect_from_name(const char* name);

void maybe_write_to_event_log(int result);

//OSWPT Stuff

enum class oswpt_type
{
	NONE = 0,
	SHIP,
	WING,
	WAYPOINT,
	SHIP_ON_TEAM,		// e.g. <any friendly>
	WHOLE_TEAM,			// e.g. Friendly
	PARSE_OBJECT,		// a "ship" that hasn't arrived yet
	EXITED,
	WING_NOT_PRESENT	// a wing that hasn't arrived yet or is between waves
};

// Goober5000
struct object_ship_wing_point_team
{
	const char* object_name = nullptr;
	oswpt_type type = oswpt_type::NONE;

	const ship_registry_entry* ship_entry = nullptr;
	object* objp = nullptr;
	wing* wingp = nullptr;
	waypoint* waypointp = nullptr;
	int team = -1;

	object_ship_wing_point_team() = default;
	object_ship_wing_point_team(ship* sp);
	object_ship_wing_point_team(p_object* pop);
	object_ship_wing_point_team(ship_obj* sop);
	object_ship_wing_point_team(wing* wp);

	void clear();
};

void eval_object_ship_wing_point_team(object_ship_wing_point_team* oswpt, int node, const char* ctext_override = nullptr);

#endif
