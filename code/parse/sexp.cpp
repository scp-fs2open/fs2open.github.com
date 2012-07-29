/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



//	Parse a symbolic expression.
//	These are identical to Lisp functions.
//	It uses a very baggy format, allocating 16 characters per token, regardless
//	of how many are used.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#ifdef _MSC_VER
	#include "globalincs/msvc/stdint.h"
#else
	#include <stdint.h>
#endif

#include "parse/parselo.h"
#include "parse/sexp.h"
#include "ship/ship.h"
#include "freespace2/freespace.h"
#include "weapon/weapon.h"
#include "mission/missionlog.h"
#include "mission/missionparse.h"		// for p_object definition
#include "mission/missionmessage.h"
#include "mission/missiontraining.h"
#include "globalincs/linklist.h"
#include "ai/aigoals.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "mission/missionbriefcommon.h"
#include "io/timer.h"
#include "ship/shiphit.h"
#include "gamesequence/gamesequence.h"
#include "stats/medals.h"
#include "playerman/player.h"
#include "hud/hud.h"
#include "hud/hudconfig.h"
#include "missionui/redalert.h"
#include "jumpnode/jumpnode.h"
#include "hud/hudshield.h"
#include "hud/hudescort.h"
#include "weapon/beam.h"
#include "starfield/supernova.h"
#include "hud/hudets.h"
#include "math/fvi.h"
#include "ship/awacs.h"
#include "hud/hudsquadmsg.h"		// for the order sexp
#include "gamesnd/eventmusic.h"		// for change-soundtrack
#include "menuui/techmenu.h"		// for intel stuff
#include "fireball/fireballs.h"		// for explosion stuff
#include "gamesnd/gamesnd.h"
#include "render/3d.h"
#include "asteroid/asteroid.h"
#include "weapon/shockwave.h"
#include "weapon/emp.h"
#include "sound/audiostr.h"
#include "sound/ds.h"
#include "sound/sound.h"
#include "cmdline/cmdline.h"
#include "hud/hudparse.h"
#include "hud/hudmessage.h"
#include "starfield/starfield.h"
#include "hud/hudartillery.h"
#include "object/objectdock.h"
#include "globalincs/systemvars.h"
#include "globalincs/version.h"
#include "camera/camera.h"
#include "iff_defs/iff_defs.h"
#include "nebula/neb.h"
#include "nebula/neblightning.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multi_team.h"
#include "network/multi_obj.h"
#include "parse/lua.h"
#include "parse/scripting.h"
#include "object/objcollide.h"
#include "object/waypoint.h"
#include "graphics/2d.h"
#include "object/objectsnd.h"
#include "graphics/font.h"
#include "asteroid/asteroid.h"
#include "mod_table/mod_table.h"
#include "ship/afterburner.h"

#ifndef NDEBUG
#include "hud/hudmessage.h"
#endif

#include "autopilot/autopilot.h"
#include "object/objectshield.h"
#include "network/multi_sexp.h"
#include "io/keycontrol.h"



#define TRUE	1
#define FALSE	0


sexp_oper Operators[] = {
//   Operator, Identity, Min / Max arguments
	{ "+",					OP_PLUS,			2,	INT_MAX	},
	{ "-",					OP_MINUS,			2,	INT_MAX	},
	{ "*",					OP_MUL,				2,	INT_MAX	},
	{ "/",					OP_DIV,				2,	INT_MAX	},
	{ "mod",				OP_MOD,				2,	INT_MAX	},
	{ "rand",				OP_RAND,			2,	3	},
	{ "rand-multiple",		OP_RAND_MULTIPLE,	2,	3	},	// Goober5000
	{ "abs",				OP_ABS,				1,	1 },	// Goober5000
	{ "min",				OP_MIN,				1,	INT_MAX },	// Goober5000
	{ "max",				OP_MAX,				1,	INT_MAX },	// Goober5000
	{ "avg",				OP_AVG,				1,	INT_MAX },	// Goober5000
	{ "pow",				OP_POW,				2,	2 },	// Goober5000
	{ "signum",				OP_SIGNUM,			1,	1 },	// Goober5000
	{ "set-bit",			OP_SET_BIT,			2,	2 },	// Goober5000
	{ "unset-bit",			OP_UNSET_BIT,		2,	2 },	// Goober5000
	{ "is-bit-set",			OP_IS_BIT_SET,		2,	2 },	// Goober5000
	{ "bitwise-and",		OP_BITWISE_AND,		2,	INT_MAX },	// Goober5000
	{ "bitwise-or",			OP_BITWISE_OR,		2,	INT_MAX },	// Goober5000
	{ "bitwise-not",		OP_BITWISE_NOT,		1,	1 },	// Goober5000
	{ "bitwise-xor",		OP_BITWISE_XOR,		2,	2 },	// Goober5000

	{ "true",							OP_TRUE,							0,	0,			},
	{ "false",							OP_FALSE,						0,	0,			},
	{ "and",								OP_AND,							2,	INT_MAX,	},
	{ "and-in-sequence",				OP_AND_IN_SEQUENCE,			2, INT_MAX, },
	{ "or",								OP_OR,							2,	INT_MAX,	},
	{ "not",								OP_NOT,							1, 1,			},
	{ "xor",								OP_XOR,							2, INT_MAX,			},	// Goober5000
	{ "=",								OP_EQUALS,						2,	INT_MAX,	},
	{ "!=",								OP_NOT_EQUAL,						2,	INT_MAX,	},	// Goober5000
	{ ">",								OP_GREATER_THAN,				2,	INT_MAX,	},
	{ ">=",								OP_GREATER_OR_EQUAL,				2,	INT_MAX,	},	// Goober5000
	{ "<",								OP_LESS_THAN,					2,	INT_MAX,	},
	{ "<=",								OP_LESS_OR_EQUAL,					2,	INT_MAX,	},	// Goober5000
	{ "string-equals",						OP_STRING_EQUALS,				2,	INT_MAX,	},
	{ "string-greater-than",				OP_STRING_GREATER_THAN,			2,	INT_MAX,	},
	{ "string-less-than",					OP_STRING_LESS_THAN,			2,	INT_MAX,	},
	{ "perform-actions",			OP_PERFORM_ACTIONS,						2,	INT_MAX,	},	// Goober5000
	{ "has-time-elapsed",			OP_HAS_TIME_ELAPSED,			1,	1,			},

	{ "is-goal-true-delay",					OP_GOAL_TRUE_DELAY,				2, 2,	},
	{ "is-goal-false-delay",				OP_GOAL_FALSE_DELAY,				2, 2,	},
	{ "is-goal-incomplete",					OP_GOAL_INCOMPLETE,				1, 1,	},
	{ "is-event-true",						OP_EVENT_TRUE,							1, 1,			},
	{ "is-event-true-delay",				OP_EVENT_TRUE_DELAY,				2, 3,	},
	{ "is-event-true-msecs-delay",			OP_EVENT_TRUE_MSECS_DELAY,			2, 3,	},
	{ "is-event-false",						OP_EVENT_FALSE,						1, 1,			},
	{ "is-event-false-delay",				OP_EVENT_FALSE_DELAY,			2, 3,	},
	{ "is-event-false-msecs-delay",			OP_EVENT_FALSE_MSECS_DELAY,		2, 3,	},
	{ "is-event-incomplete",				OP_EVENT_INCOMPLETE,				1, 1,	},
	{ "is-previous-goal-true",				OP_PREVIOUS_GOAL_TRUE,			2, 3,	},
	{ "is-previous-goal-false",				OP_PREVIOUS_GOAL_FALSE,			2, 3,	},
	{ "is-previous-goal-incomplete",		OP_PREVIOUS_GOAL_INCOMPLETE,	2, 3,	},
	{ "is-previous-event-true",				OP_PREVIOUS_EVENT_TRUE,			2, 3,	},
	{ "is-previous-event-false",			OP_PREVIOUS_EVENT_FALSE,		2, 3,	},
	{ "is-previous-event-incomplete",		OP_PREVIOUS_EVENT_INCOMPLETE,	2, 3,	},

	{ "is-destroyed",							OP_IS_DESTROYED,						1,	INT_MAX,	},
	{ "is-destroyed-delay",					OP_IS_DESTROYED_DELAY,				2,	INT_MAX,	},
	{ "is-subsystem-destroyed",			OP_IS_SUBSYSTEM_DESTROYED,			2, 2,			},
	{ "is-subsystem-destroyed-delay",	OP_IS_SUBSYSTEM_DESTROYED_DELAY,	3, 3,			},
	{ "is-disabled",							OP_IS_DISABLED,						1, INT_MAX,	},
	{ "is-disabled-delay",					OP_IS_DISABLED_DELAY,				2, INT_MAX,	},
	{ "is-disarmed",							OP_IS_DISARMED,						1, INT_MAX,	},
	{ "is-disarmed-delay",					OP_IS_DISARMED_DELAY,				2, INT_MAX,	},
	{ "has-docked",							OP_HAS_DOCKED,							3, 3,			},
	{ "has-docked-delay",					OP_HAS_DOCKED_DELAY,					4, 4,			},
	{ "has-undocked",							OP_HAS_UNDOCKED,						3, 3,			},
	{ "has-undocked-delay",					OP_HAS_UNDOCKED_DELAY,				4, 4,			},
	{ "has-arrived",							OP_HAS_ARRIVED,						1, INT_MAX,	},
	{ "has-arrived-delay",					OP_HAS_ARRIVED_DELAY,				2, INT_MAX,	},
	{ "has-departed",							OP_HAS_DEPARTED,						1, INT_MAX,	},
	{ "has-departed-delay",					OP_HAS_DEPARTED_DELAY,				2, INT_MAX,	},
	{ "are-waypoints-done",					OP_WAYPOINTS_DONE,					2, 2,			},
	{ "are-waypoints-done-delay",			OP_WAYPOINTS_DONE_DELAY,			3, 4,			},
	{ "is-nav-visited",					OP_NAV_IS_VISITED,				1, 1 }, // Kazan
	{ "ship-type-destroyed",				OP_SHIP_TYPE_DESTROYED,				2, 2,			},
	{ "percent-ships-destroyed",			OP_PERCENT_SHIPS_DESTROYED,		2, INT_MAX,	},
	{ "percent-ships-disabled",			OP_PERCENT_SHIPS_DISABLED,			2, INT_MAX,	},
	{ "percent-ships-disarmed",			OP_PERCENT_SHIPS_DISARMED,			2, INT_MAX,	},
	{ "percent-ships-departed",			OP_PERCENT_SHIPS_DEPARTED,			2, INT_MAX,	},
	{ "percent-ships-arrived",			OP_PERCENT_SHIPS_ARRIVED,			2, INT_MAX,	},
	{ "depart-node-delay",					OP_DEPART_NODE_DELAY,				3, INT_MAX, },	
	{ "destroyed-or-departed-delay",		OP_DESTROYED_DEPARTED_DELAY,		2, INT_MAX, },	

	{ "is-cargo-known",						OP_IS_CARGO_KNOWN,					1, INT_MAX,	},
	{ "is-cargo-known-delay",				OP_CARGO_KNOWN_DELAY,				2, INT_MAX,	},
	{ "cap-subsys-cargo-known-delay",	OP_CAP_SUBSYS_CARGO_KNOWN_DELAY,	3, INT_MAX,	},
	{ "is-cargo",						OP_IS_CARGO,						2, 3 },
	{ "is-ship-visible",				OP_IS_SHIP_VISIBLE,			1, 1, },
	{ "is-ship-stealthy",				OP_IS_SHIP_STEALTHY,	1, 1, },
	{ "is-friendly-stealth-visible",		OP_IS_FRIENDLY_STEALTH_VISIBLE,	1, 1, },
	{ "is_tagged",								OP_IS_TAGGED,							1, 1			},
	{ "has-been-tagged-delay",				OP_HAS_BEEN_TAGGED_DELAY,			2, INT_MAX,	},
	{ "is-iff",							OP_IS_IFF,						2, INT_MAX,	},
	{ "is-ai-class",					OP_IS_AI_CLASS,					2, INT_MAX,	},
	{ "is-ship-type",					OP_IS_SHIP_TYPE,					2, INT_MAX,	},
	{ "is-ship-class",					OP_IS_SHIP_CLASS,					2, INT_MAX,	},
	{ "is-facing",						OP_IS_FACING,						3, 4, },
	{ "is-in-mission",					OP_IS_IN_MISSION,					1, INT_MAX, },	// Goober5000
	{ "shield-recharge-pct",				OP_SHIELD_RECHARGE_PCT,				1, 1			},
	{ "engine-recharge-pct",				OP_ENGINE_RECHARGE_PCT,				1, 1			},
	{ "weapon-recharge-pct",				OP_WEAPON_RECHARGE_PCT,				1, 1			},
	{ "shield-quad-low",					OP_SHIELD_QUAD_LOW,					2,	2			},
	{ "primary-ammo-pct",					OP_PRIMARY_AMMO_PCT,				2,	2			},
	{ "secondary-ammo-pct",					OP_SECONDARY_AMMO_PCT,				2,	2			},
	{ "get-primary-ammo",					OP_GET_PRIMARY_AMMO,				2,	2			}, // Karajorma
	{ "get-secondary-ammo",					OP_GET_SECONDARY_AMMO,				2,	2			}, // Karajorma
	{ "get-num-countermeasures",			OP_GET_NUM_COUNTERMEASURES,			1,	1			}, // Karajorma
	{ "is-primary-selected",				OP_IS_PRIMARY_SELECTED,				2,	2			},
	{ "is-secondary-selected",				OP_IS_SECONDARY_SELECTED,			2,	2			},
	{ "afterburner-energy-pct",				OP_AFTERBURNER_LEFT,		1, 1			},
	{ "weapon-energy-pct",					OP_WEAPON_ENERGY_LEFT,			1, 1			},
	{ "shields-left",						OP_SHIELDS_LEFT,					1, 1,			},
	{ "hits-left",						OP_HITS_LEFT,					1, 1, },
	{ "hits-left-subsystem",		OP_HITS_LEFT_SUBSYSTEM,		2, 3, },
	{ "hits-left-subsystem-generic",	OP_HITS_LEFT_SUBSYSTEM_GENERIC,		2,	2,	},	// Goober5000
	{ "hits-left-subsystem-specific",	OP_HITS_LEFT_SUBSYSTEM_SPECIFIC,	2,	2,	},	// Goober5000
	{ "sim-hits-left",						OP_SIM_HITS_LEFT,					1, 1, }, // Turey
	{ "distance",						OP_DISTANCE,					2, 2, },
	{ "distance-ship-subsystem",	OP_DISTANCE_SUBSYSTEM,	3, 3 },					// Goober5000
	{ "distance-to-nav",				OP_NAV_DISTANCE,				1, 1 }, // Kazan
	{ "num-within-box",				OP_NUM_WITHIN_BOX,					7,	INT_MAX},	//WMC
	{ "is-in-box",					OP_IS_IN_BOX,					7,	8},	//Sushi
	{ "special-warp-dist",			OP_SPECIAL_WARP_DISTANCE,	1, 1,	},
	{ "get-damage-caused",			OP_GET_DAMAGE_CAUSED,		2, INT_MAX	},

	{ "get-object-x",				OP_GET_OBJECT_X,				1,	5	},	// Goober5000
	{ "get-object-y",				OP_GET_OBJECT_Y,				1,	5	},	// Goober5000
	{ "get-object-z",				OP_GET_OBJECT_Z,				1,	5	},	// Goober5000
	{ "set-object-position",		OP_SET_OBJECT_POSITION,			4,	4	},	// WMC
	{ "get-object-pitch",				OP_GET_OBJECT_PITCH,			1,	1	},	// Goober5000
	{ "get-object-bank",				OP_GET_OBJECT_BANK,				1,	1	},	// Goober5000
	{ "get-object-heading",				OP_GET_OBJECT_HEADING,			1,	1	},	// Goober5000
	{ "set-object-orientation",		OP_SET_OBJECT_ORIENTATION,			4,	4	},	// Goober5000
	{ "set-object-facing",			OP_SET_OBJECT_FACING,					4,	6	},	// Goober5000
	{ "set-object-facing-object",	OP_SET_OBJECT_FACING_OBJECT,			2,	4	},	// Goober5000
	{ "set-object-speed-x",				OP_SET_OBJECT_SPEED_X,			2,	3	},	// WMC
	{ "set-object-speed-y",				OP_SET_OBJECT_SPEED_Y,			2,	3	},	// WMC
	{ "set-object-speed-z",				OP_SET_OBJECT_SPEED_Z,			2,	3	},	// WMC
	{ "get-object-speed-x",				OP_GET_OBJECT_SPEED_X,			1,	2	},
	{ "get-object-speed-y",				OP_GET_OBJECT_SPEED_Y,			1,	2	},
	{ "get-object-speed-z",				OP_GET_OBJECT_SPEED_Z,			1,	2	},

	{ "time-elapsed-last-order",	OP_LAST_ORDER_TIME,			2, 2, },
	{ "skill-level-at-least",		OP_SKILL_LEVEL_AT_LEAST,	1, 1, },
	{ "num-ships-in-battle",		OP_NUM_SHIPS_IN_BATTLE,			0,	INT_MAX},	//phreak modified by FUBAR
	{ "num-ships-in-wing",			OP_NUM_SHIPS_IN_WING,			1,	INT_MAX},	// Karajorma
	{ "is-player",					OP_IS_PLAYER,				2,	INT_MAX},	// Karajorma
	{ "num-players",				OP_NUM_PLAYERS,				0, 0, },
	{ "num_kills",					OP_NUM_KILLS,				1, 1			},
	{ "num_assists",				OP_NUM_ASSISTS,				1, 1			},
	{ "ship_score",					OP_SHIP_SCORE,				1, 1			},
	{ "ship-deaths",				OP_SHIP_DEATHS,				1, 1			},
	{ "respawns-left",				OP_RESPAWNS_LEFT,				1, 1			},	
	{ "num_type_kills",				OP_NUM_TYPE_KILLS,			2,	2			},
	{ "num_class_kills",			OP_NUM_CLASS_KILLS,			2,	2			},
	{ "team-score",					OP_TEAM_SCORE,				1,	1,	}, 
	{ "was-promotion-granted",		OP_WAS_PROMOTION_GRANTED,	0, 1,			},
	{ "was-medal-granted",			OP_WAS_MEDAL_GRANTED,		0, 1,			},
	{ "current-speed",				OP_CURRENT_SPEED,				1, 1},
	{ "primary-fired-since",		OP_PRIMARY_FIRED_SINCE,		3,	3},	// Karajorma
	{ "secondary-fired-since",		OP_SECONDARY_FIRED_SINCE,	3,	3},	// Karajorma
	{ "get-throttle-speed",			OP_GET_THROTTLE_SPEED,		1, 1,			}, // Karajorma
	{ "set-player-throttle-speed",	OP_SET_PLAYER_THROTTLE_SPEED,		2, 2,	}, //CommanderDJ
	{ "has-primary-weapon",			OP_HAS_PRIMARY_WEAPON,		3,	INT_MAX},	// Karajorma
	{ "has-secondary-weapon",		OP_HAS_SECONDARY_WEAPON,	3,	INT_MAX},	// Karajorma
	{ "directive-value",			OP_DIRECTIVE_VALUE,	1,	2},	// Karajorma
	
	{ "time-ship-destroyed",	OP_TIME_SHIP_DESTROYED,		1,	1,	},
	{ "time-ship-arrived",		OP_TIME_SHIP_ARRIVED,		1,	1,	},
	{ "time-ship-departed",		OP_TIME_SHIP_DEPARTED,		1,	1,	},
	{ "time-wing-destroyed",	OP_TIME_WING_DESTROYED,		1,	1,	},
	{ "time-wing-arrived",		OP_TIME_WING_ARRIVED,		1,	1,	},
	{ "time-wing-departed",		OP_TIME_WING_DEPARTED,		1,	1,	},
	{ "mission-time",			OP_MISSION_TIME,			0, 0,	},
	{ "mission-time-msecs",		OP_MISSION_TIME_MSECS,		0, 0,	},	// Goober5000
	{ "time-docked",			OP_TIME_DOCKED,				3, 3, },
	{ "time-undocked",			OP_TIME_UNDOCKED,			3, 3, },

	{ "cond",						OP_COND,					1, INT_MAX, },
	{ "when",						OP_WHEN,					2, INT_MAX, },
	{ "when-argument",				OP_WHEN_ARGUMENT,			3, INT_MAX, },	// Goober5000
	{ "every-time",					OP_EVERY_TIME,				2, INT_MAX, },	// Goober5000
	{ "every-time-argument",		OP_EVERY_TIME_ARGUMENT,		3, INT_MAX, },	// Goober5000
	{ "if-then-else",				OP_IF_THEN_ELSE,			3, INT_MAX,	},	// Goober5000
	{ "any-of",						OP_ANY_OF,					1, INT_MAX, },	// Goober5000
	{ "every-of",					OP_EVERY_OF,				1, INT_MAX, },	// Goober5000
	{ "random-of",					OP_RANDOM_OF,				1, INT_MAX, },	// Goober5000
	{ "random-multiple-of",			OP_RANDOM_MULTIPLE_OF,		1, INT_MAX, },	// Karajorma
	{ "number-of",					OP_NUMBER_OF,				2, INT_MAX, },	// Goober5000
	{ "in-sequence",				OP_IN_SEQUENCE,				1, INT_MAX, },	// Karajorma
	{ "for-counter",				OP_FOR_COUNTER,				2, 3,		},	// Goober5000
	{ "invalidate-argument",		OP_INVALIDATE_ARGUMENT,		1, INT_MAX, },	// Goober5000
	{ "invalidate-all-arguments",	OP_INVALIDATE_ALL_ARGUMENTS,	0, 0, },	// Karajorma
	{ "validate-argument",			OP_VALIDATE_ARGUMENT,		1, INT_MAX, },	// Karajorma
	{ "validate-all-arguments",		OP_VALIDATE_ALL_ARGUMENTS,		0, 0, },	// Karajorma
	{ "do-for-valid-arguments",		OP_DO_FOR_VALID_ARGUMENTS,	1, INT_MAX, },	// Karajorma
	{ "num-valid-arguments",		OP_NUM_VALID_ARGUMENTS,		0, 0, },		// Karajorma

	{ "send-message-list",			OP_SEND_MESSAGE_LIST,		4,	INT_MAX	},
	{ "send-message",				OP_SEND_MESSAGE,			3,	3,		},
	{ "send-random-message",		OP_SEND_RANDOM_MESSAGE,		3,	INT_MAX,},
	{ "invalidate-goal",			OP_INVALIDATE_GOAL,			1,	INT_MAX,},
	{ "validate-goal",				OP_VALIDATE_GOAL,			1,	INT_MAX,},
	{ "scramble-messages",			OP_SCRAMBLE_MESSAGES,		0,	0,},
	{ "unscramble-messages",		OP_UNSCRAMBLE_MESSAGES,		0,	0,},
	{ "disable-builtin-messages",	OP_DISABLE_BUILTIN_MESSAGES,	0,	INT_MAX,},	// Karajorma
	{ "enable-builtin-messages",	OP_ENABLE_BUILTIN_MESSAGES,		0,	INT_MAX,},	// Karajorma
	{ "set-persona",				OP_SET_PERSONA,					2,	INT_MAX,},	// Karajorma
	{ "clear-subtitles",			OP_CLEAR_SUBTITLES,				0, 0},

	{ "add-goal",					OP_ADD_GOAL,					2, 2, },
	{ "remove-goal",				OP_REMOVE_GOAL,					2, 2, },			// Goober5000
	{ "add-ship-goal",				OP_ADD_SHIP_GOAL,				2, 2,			},
	{ "add-wing-goal",				OP_ADD_WING_GOAL,				2, 2,			},
	{ "clear-goals",					OP_CLEAR_GOALS,				1, INT_MAX,	},
	{ "clear-ship-goals",			OP_CLEAR_SHIP_GOALS,			1, 1,			},
	{ "clear-wing-goals",			OP_CLEAR_WING_GOALS,			1, 1,			},
	{ "good-rearm-time",				OP_GOOD_REARM_TIME,			2,	2,			},
	{ "good-secondary-time",		OP_GOOD_SECONDARY_TIME,			4, 4,			},
	{ "change-iff",					OP_CHANGE_IFF,					2,	INT_MAX,	},
	{ "change-iff-color",			OP_CHANGE_IFF_COLOR,			6,	INT_MAX,	},
	{ "change-ai-class",			OP_CHANGE_AI_CLASS,				2,	INT_MAX,	},
	{ "protect-ship",				OP_PROTECT_SHIP,				1, INT_MAX,	},
	{ "unprotect-ship",				OP_UNPROTECT_SHIP,				1, INT_MAX,	},
	{ "beam-protect-ship",			OP_BEAM_PROTECT_SHIP,			1, INT_MAX,	},
	{ "beam-unprotect-ship",		OP_BEAM_UNPROTECT_SHIP,			1, INT_MAX,	},
	{ "turret-protect-ship",		OP_TURRET_PROTECT_SHIP,			2, INT_MAX,	},	// Goober5000
	{ "turret-unprotect-ship",		OP_TURRET_UNPROTECT_SHIP,		2, INT_MAX,	},	// Goober5000
	{ "kamikaze",					OP_KAMIKAZE,					2, INT_MAX }, //-Sesquipedalian
	{ "player-use-ai",				OP_PLAYER_USE_AI,				0, 0 },			// Goober5000
	{ "player-not-use-ai",			OP_PLAYER_NOT_USE_AI,			0, 0 },			// Goober5000
	{ "allow-treason",				OP_ALLOW_TREASON,				1, 1 },			// Karajorma
	{ "set-player-orders",			OP_SET_PLAYER_ORDERS,			3, INT_MAX },	// Karajorma

	{ "sabotage-subsystem",			OP_SABOTAGE_SUBSYSTEM,			3, 3,			},
	{ "repair-subsystem",			OP_REPAIR_SUBSYSTEM,			3, 4,			},
	{ "set-subsystem-strength",	OP_SET_SUBSYSTEM_STRNGTH,			3, 4,			},
	{ "subsys-set-random",			OP_SUBSYS_SET_RANDOM,			3, INT_MAX	},
	{ "self-destruct",				OP_SELF_DESTRUCT,				1, INT_MAX,	},
	{ "transfer-cargo",				OP_TRANSFER_CARGO,				2, 2,			},
	{ "exchange-cargo",				OP_EXCHANGE_CARGO,				2, 2,			},
	{ "set-cargo",					OP_SET_CARGO,					2, 3,			},
	{ "jettison-cargo-delay",		OP_JETTISON_CARGO,				2, INT_MAX,		},
	{ "set-docked",					OP_SET_DOCKED,					4, 4 },				// Sushi
	{ "cargo-no-deplete",			OP_CARGO_NO_DEPLETE,			1,	2			},
	{ "set-scanned",				OP_SET_SCANNED,					1, 2 },
	{ "set-unscanned",				OP_SET_UNSCANNED,				1, 2 },
	{ "lock-rotating-subsystem",	OP_LOCK_ROTATING_SUBSYSTEM,		2, INT_MAX },	// Goober5000
	{ "free-rotating-subsystem",	OP_FREE_ROTATING_SUBSYSTEM,		2, INT_MAX },	// Goober5000
	{ "reverse-rotating-subsystem",	OP_REVERSE_ROTATING_SUBSYSTEM,	2, INT_MAX },	// Goober5000
	{ "rotating-subsys-set-turn-time", OP_ROTATING_SUBSYS_SET_TURN_TIME,	3, INT_MAX	},	// Goober5000
	{ "trigger-submodel-animation",	OP_TRIGGER_SUBMODEL_ANIMATION,	4, 6 },		// Goober5000
	{ "set-primary-ammo",			OP_SET_PRIMARY_AMMO,			3, 4 },		// Karajorma
	{ "set-secondary-ammo",			OP_SET_SECONDARY_AMMO,			3, 4 },		// Karajorma
	{ "set-primary-weapon",			OP_SET_PRIMARY_WEAPON,			3, 5 },		// Karajorma
	{ "set-secondary-weapon",		OP_SET_SECONDARY_WEAPON,		3, 5 },		// Karajorma
	{ "set-num-countermeasures",	OP_SET_NUM_COUNTERMEASURES,		2, 2 },		// Karajorma
	{ "lock-primary-weapon",		OP_LOCK_PRIMARY_WEAPON,			1, INT_MAX },		// Karajorma
	{ "unlock-primary-weapon",		OP_UNLOCK_PRIMARY_WEAPON,		1, INT_MAX },		// Karajorma
	{ "lock-secondary-weapon",		OP_LOCK_SECONDARY_WEAPON,		1, INT_MAX },		// Karajorma
	{ "unlock-secondary-weapon",	OP_UNLOCK_SECONDARY_WEAPON,		1, INT_MAX },		// Karajorma
	{ "change-subsystem-name",		OP_CHANGE_SUBSYSTEM_NAME,		3, INT_MAX },		// Karajorma
	{ "lock-afterburner",			OP_LOCK_AFTERBURNER,			1, INT_MAX },		// KeldorKatarn
	{ "unlock-afterburner",			OP_UNLOCK_AFTERBURNER,			1, INT_MAX },		// KeldorKatarn
	{ "set-afterburner-energy",		OP_SET_AFTERBURNER_ENERGY,		2, INT_MAX },		// Karajorma
	{ "set-weapon-energy",			OP_SET_WEAPON_ENERGY,			2, INT_MAX },		// Karajorma
	{ "set-shield-energy",			OP_SET_SHIELD_ENERGY,			2, INT_MAX },		// Karajorma
	{ "set-ambient-light",			OP_SET_AMBIENT_LIGHT,			3, 3 },				// Karajorma
	{ "set-post-effect",			OP_SET_POST_EFFECT,				2, 2 },				// Hery

	{ "ship-invulnerable",			OP_SHIP_INVULNERABLE,			1, INT_MAX	},
	{ "ship-vulnerable",			OP_SHIP_VULNERABLE,			1, INT_MAX	},
	{ "ship-guardian",				OP_SHIP_GUARDIAN,				1, INT_MAX	},
	{ "ship-no-guardian",			OP_SHIP_NO_GUARDIAN,			1, INT_MAX	},
	{ "ship-guardian-threshold",	OP_SHIP_GUARDIAN_THRESHOLD,				2, INT_MAX	},
	{ "ship-subsys-guardian-threshold",	OP_SHIP_SUBSYS_GUARDIAN_THRESHOLD,	3, INT_MAX	},
	{ "ship-invisible",				OP_SHIP_INVISIBLE,				1, INT_MAX	},
	{ "ship-visible",				OP_SHIP_VISIBLE,				1, INT_MAX	},
	{ "ship-stealthy",				OP_SHIP_STEALTHY,				1, INT_MAX },
	{ "ship-unstealthy",			OP_SHIP_UNSTEALTHY,				1, INT_MAX },			// Goober5000
	{ "friendly-stealth-invisible",	OP_FRIENDLY_STEALTH_INVISIBLE,	1, INT_MAX },	// Goober5000
	{ "friendly-stealth-visible",	OP_FRIENDLY_STEALTH_VISIBLE,	1, INT_MAX },	// Goober5000
	{ "ship-targetable-as-bomb",	OP_SHIP_BOMB_TARGETABLE,			1, INT_MAX	},
	{ "ship-untargetable-as-bomb",	OP_SHIP_BOMB_UNTARGETABLE,			1, INT_MAX	},
	{ "ship-subsys-targetable",		OP_SHIP_SUBSYS_TARGETABLE,		2, INT_MAX },	// Goober5000
	{ "ship-subsys-no-replace",		OP_SHIP_SUBSYS_NO_REPLACE,		3, INT_MAX },	// FUBAR
	{ "ship-subsys-no-live-debris",	OP_SHIP_SUBSYS_NO_LIVE_DEBRIS,	3, INT_MAX },	// FUBAR
	{ "ship-subsys-vanish",			OP_SHIP_SUBSYS_VANISHED,		3, INT_MAX },	// FUBAR
	{ "ship-subsys-ignore_if_dead",	OP_SHIP_SUBSYS_IGNORE_IF_DEAD,	3, INT_MAX },	// FUBAR
	{ "ship-subsys-untargetable",	OP_SHIP_SUBSYS_UNTARGETABLE,	2, INT_MAX },	// Goober5000
	{ "ship-vaporize",				OP_SHIP_VAPORIZE,				1, INT_MAX },	// Goober5000
	{ "ship-no-vaporize",			OP_SHIP_NO_VAPORIZE,			1, INT_MAX },	// Goober5000
	{ "set-explosion-option",		OP_SET_EXPLOSION_OPTION,		3, INT_MAX	},	// Goober5000
	{ "break-warp",					OP_WARP_BROKEN,					1, INT_MAX,	},
	{ "fix-warp",					OP_WARP_NOT_BROKEN,				1, INT_MAX,	},
	{ "never-warp",					OP_WARP_NEVER,					1, INT_MAX, },
	{ "allow-warp",					OP_WARP_ALLOWED,				1, INT_MAX, },
	{ "set-subspace-drive",			OP_SET_SUBSPACE_DRIVE,			2, INT_MAX, },
	{ "set-armor-type",				OP_SET_ARMOR_TYPE,				4, INT_MAX, },  // FUBAR
	{ "add-to-collision-group",		OP_ADD_TO_COLGROUP,				2, INT_MAX },	// The E
	{ "remove-from-collision-group",OP_REMOVE_FROM_COLGROUP,		2, INT_MAX },
	{ "get-collision-group",		OP_GET_COLGROUP_ID,				1, 1 },
	{ "ship-effect",				OP_SHIP_EFFECT,					3, INT_MAX },	// Valathil

	{ "fire-beam",						OP_BEAM_FIRE,					3, 5		},
	{ "fire-beam-at-coordinates",		OP_BEAM_FIRE_COORDS,			5, 9		},
	{ "beam-free",						OP_BEAM_FREE,					2, INT_MAX	},
	{ "beam-free-all",					OP_BEAM_FREE_ALL,				1, INT_MAX	},
	{ "beam-lock",						OP_BEAM_LOCK,					2, INT_MAX	},
	{ "beam-lock-all",					OP_BEAM_LOCK_ALL,				1, INT_MAX	},
	{ "turret-free",					OP_TURRET_FREE,					2, INT_MAX	},
	{ "turret-free-all",				OP_TURRET_FREE_ALL,				1, INT_MAX	},
	{ "turret-lock",					OP_TURRET_LOCK,					2, INT_MAX	},
	{ "turret-lock-all",				OP_TURRET_LOCK_ALL,				1, INT_MAX	},
	{ "turret-tagged-only",				OP_TURRET_TAGGED_ONLY_ALL,		1,	1		},
	{ "turret-tagged-clear",			OP_TURRET_TAGGED_CLEAR_ALL,		1,	1		},
	{ "turret-tagged-specific",			OP_TURRET_TAGGED_SPECIFIC,		2, INT_MAX }, //phreak
	{ "turret-tagged-clear-specific",	OP_TURRET_TAGGED_CLEAR_SPECIFIC, 2, INT_MAX}, //phreak
	{ "turret-change-weapon",			OP_TURRET_CHANGE_WEAPON,		5, 5},	//WMC
	{ "turret-set-direction-preference",	OP_TURRET_SET_DIRECTION_PREFERENCE,		3, INT_MAX},	//FUBAR
	{ "turret-set-rate-of-fire",			OP_TURRET_SET_RATE_OF_FIRE,			3, INT_MAX},	//FUBAR
	{ "turret-set-optimum-range",			OP_TURRET_SET_OPTIMUM_RANGE,			3, INT_MAX},	//FUBAR
	{ "turret-set-target-priorities",			OP_TURRET_SET_TARGET_PRIORITIES,	3, INT_MAX},	//FUBAR
	{ "turret-set-target-order",			OP_TURRET_SET_TARGET_ORDER,			2, 2+NUM_TURRET_ORDER_TYPES},	//WMC
	{ "ship-turret-target-order",			OP_SHIP_TURRET_TARGET_ORDER,		1, 1+NUM_TURRET_ORDER_TYPES},	//WMC
	{ "turret-subsys-target-disable",	OP_TURRET_SUBSYS_TARGET_DISABLE, 2, INT_MAX	},
	{ "turret-subsys-target-enable",	OP_TURRET_SUBSYS_TARGET_ENABLE,	2, INT_MAX	},


	{ "red-alert",						OP_RED_ALERT,					0, 0 },
	{ "end-mission",					OP_END_MISSION,					0, 2 }, //-Sesquipedalian
	{ "force-jump",						OP_FORCE_JUMP,					0, 0 }, // Goober5000
	{ "next-mission",					OP_NEXT_MISSION,				1, 1 },
	{ "end-campaign",					OP_END_CAMPAIGN,				0, 0 },
	{ "end-of-campaign",				OP_END_OF_CAMPAIGN,				0, 0 },
	{ "set-debriefing-toggled",			OP_SET_DEBRIEFING_TOGGLED,		1, 1 },	// Goober5000

	{ "add-nav-waypoint",				OP_NAV_ADD_WAYPOINT,			3, 4 }, //kazan
	{ "add-nav-ship",					OP_NAV_ADD_SHIP,				2, 2 }, //kazan
	{ "del-nav",						OP_NAV_DEL,						1, 1 }, //kazan
	{ "hide-nav",						OP_NAV_HIDE,					1, 1 }, //kazan
	{ "restrict-nav",					OP_NAV_RESTRICT,				1, 1 }, //kazan
	{ "unhide-nav",						OP_NAV_UNHIDE,					1, 1 }, //kazan
	{ "unrestrict-nav",					OP_NAV_UNRESTRICT,				1, 1 }, //kazan
	{ "set-nav-visited",				OP_NAV_SET_VISITED,				1, 1 }, //kazan
	{ "unset-nav-visited",				OP_NAV_UNSET_VISITED,			1, 1 }, //kazan
	{ "set-nav-carry",					OP_NAV_SET_CARRY,				1, INT_MAX }, //kazan
	{ "unset-nav-carry",				OP_NAV_UNSET_CARRY,				1, INT_MAX }, //kazan
	{ "set-nav-needslink",				OP_NAV_SET_NEEDSLINK,			1, INT_MAX }, //kazan
	{ "unset-nav-needslink",			OP_NAV_UNSET_NEEDSLINK,			1, INT_MAX }, //kazan
	{ "is-nav-linked",					OP_NAV_ISLINKED,				1, 1 }, //kazan
	{ "use-nav-cinematics",				OP_NAV_USECINEMATICS,			1, 1 }, //kazan
	{ "use-autopilot",					OP_NAV_USEAP,					1, 1 }, //kazan

	{ "grant-promotion",				OP_GRANT_PROMOTION,				0, 0,			},
	{ "grant-medal",					OP_GRANT_MEDAL,					1, 1,			},
	{ "allow-ship",					OP_ALLOW_SHIP,						1, 1,			},
	{ "allow-weapon",					OP_ALLOW_WEAPON,					1, 1,			},
	{ "tech-add-ships",				OP_TECH_ADD_SHIP,					1, INT_MAX	},
	{ "tech-add-weapons",			OP_TECH_ADD_WEAPON,				1, INT_MAX	},
	{ "tech-add-intel",				OP_TECH_ADD_INTEL,				1, INT_MAX	},	// Goober5000
	{ "tech-reset-to-default",		OP_TECH_RESET_TO_DEFAULT,		0, 0 },	// Goober5000
	{ "change-player-score",		OP_CHANGE_PLAYER_SCORE,			2, INT_MAX },	// Karajorma
	{ "change-team-score",			OP_CHANGE_TEAM_SCORE,			2, 2 },			// Karajorma
	{ "set-respawns",			OP_SET_RESPAWNS,			2, INT_MAX },	// Karajorma

	{ "don't-collide-invisible",	OP_DONT_COLLIDE_INVISIBLE,		1, INT_MAX },	// Goober5000
	{ "collide-invisible",			OP_COLLIDE_INVISIBLE,			1, INT_MAX },	// Goober5000
	{ "change-ship-class",			OP_CHANGE_SHIP_CLASS,			2, INT_MAX },	// Goober5000
	{ "deactivate-glow-points",		OP_DEACTIVATE_GLOW_POINTS,		1, INT_MAX },	//-Bobboau
	{ "activate-glow-points",		OP_ACTIVATE_GLOW_POINTS,		1, INT_MAX },	//-Bobboau
	{ "deactivate-glow-maps",		OP_DEACTIVATE_GLOW_MAPS,		1, INT_MAX },	//-Bobboau
	{ "activate-glow-maps",			OP_ACTIVATE_GLOW_MAPS,			1, INT_MAX },	//-Bobboau
	{ "deactivate-glow-point-bank",	OP_DEACTIVATE_GLOW_POINT_BANK,	2, INT_MAX },	//-Bobboau
	{ "activate-glow-point-bank",	OP_ACTIVATE_GLOW_POINT_BANK,	2, INT_MAX },	//-Bobboau
	{ "set-thrusters-status",		OP_SET_THRUSTERS,				2, INT_MAX },	// The E

	{ "change-soundtrack",				OP_CHANGE_SOUNDTRACK,				1, 1 },		// Goober5000	
	{ "play-sound-from-table",		OP_PLAY_SOUND_FROM_TABLE,		4, 4 },		// Goober5000
	{ "play-sound-from-file",		OP_PLAY_SOUND_FROM_FILE,		1, 3 },		// Goober5000
	{ "close-sound-from-file",		OP_CLOSE_SOUND_FROM_FILE,	1, 1 },		// Goober5000
	{ "set-sound-environment",		OP_SET_SOUND_ENVIRONMENT,		1, INT_MAX },	// Taylor
	{ "update-sound-environment",	OP_UPDATE_SOUND_ENVIRONMENT,	2, INT_MAX },	// Taylor
	{ "adjust-audio-volume",		OP_ADJUST_AUDIO_VOLUME,			1, 3},

	{ "modify-variable",				OP_MODIFY_VARIABLE,			2,	2,			},
	{ "variable-array-get",				OP_GET_VARIABLE_BY_INDEX,	1,	1,			},
	{ "variable-array-set",				OP_SET_VARIABLE_BY_INDEX,	2,	2,			},
	{ "add-remove-escort",			OP_ADD_REMOVE_ESCORT,			2, 2			},
	{ "damaged-escort-priority",		OP_DAMAGED_ESCORT_LIST,		3, INT_MAX },					//phreak
	{ "damaged-escort-priority-all",	OP_DAMAGED_ESCORT_LIST_ALL,	1, MAX_COMPLETE_ESCORT_LIST },					// Goober5000
	{ "awacs-set-radius",			OP_AWACS_SET_RADIUS,				3,	3			},
	{ "primitive-sensors-set-range",OP_PRIMITIVE_SENSORS_SET_RANGE,	2,	2 },	// Goober5000
	{ "set-support-ship",			OP_SET_SUPPORT_SHIP,			6, 7 },	// Goober5000
	{ "set-arrival-info",			OP_SET_ARRIVAL_INFO,			2, 6 },	// Goober5000
	{ "set-departure-info",			OP_SET_DEPARTURE_INFO,			2, 5 },	// Goober5000
	{ "cap-waypoint-speed",			OP_CAP_WAYPOINT_SPEED,			2, 2			},
	{ "special-warpout-name",		OP_SET_SPECIAL_WARPOUT_NAME,	2, 2 },
	{ "ship-create",					OP_SHIP_CREATE,					5, 8	},	//WMC
	{ "weapon-create",					OP_WEAPON_CREATE,				5, 10	},	// Goober5000
	{ "ship-vanish",					OP_SHIP_VANISH,					1, INT_MAX	},
	{ "supernova-start",				OP_SUPERNOVA_START,				1,	1			},
	{ "supernova-stop",					OP_SUPERNOVA_STOP,				0,	0			}, //CommanderDJ
	{ "shields-on",					OP_SHIELDS_ON,					1, INT_MAX			}, //-Sesquipedalian
	{ "shields-off",					OP_SHIELDS_OFF,					1, INT_MAX			}, //-Sesquipedalian
	{ "ship-tag",				OP_SHIP_TAG,				3, 8			},	// Goober5000
	{ "ship-untag",				OP_SHIP_UNTAG,				1, 1			},	// Goober5000
	{ "explosion-effect",			OP_EXPLOSION_EFFECT,			11, 13 },			// Goober5000
	{ "warp-effect",			OP_WARP_EFFECT,					12, 12 },		// Goober5000
	{ "ship-change-alt-name",		OP_SHIP_CHANGE_ALT_NAME,	2, INT_MAX	},	// Goober5000
	{ "ship-change-callsign",		OP_SHIP_CHANGE_CALLSIGN,	2, INT_MAX	},	// FUBAR
	{ "ship-copy-damage",			OP_SHIP_COPY_DAMAGE,			2, INT_MAX },	// Goober5000
	{ "set-death-message",		OP_SET_DEATH_MESSAGE,			1, 1 },			// Goober5000
	{ "remove-weapons",			OP_REMOVE_WEAPONS,			0, 1 },	// Karajorma
	{ "ship-maneuver",			OP_SHIP_MANEUVER,			10, 10 }, // Wanderer
	{ "ship-rot-maneuver",		OP_SHIP_ROT_MANEUVER,		6, 6 }, // Wanderer
	{ "ship-lat-maneuver",		OP_SHIP_LAT_MANEUVER,		6, 6 }, // Wanderer
	{ "force-glide",			OP_FORCE_GLIDE,				2, 2 }, // The E
	{ "weapon-set-damage-type",		OP_WEAPON_SET_DAMAGE_TYPE,		4, INT_MAX }, // FUBAR
	{ "ship-set-damage-type",		OP_SHIP_SET_DAMAGE_TYPE,		4, INT_MAX }, // FUBAR
	{ "ship-set-shockwave-damage-type",		OP_SHIP_SHOCKWAVE_SET_DAMAGE_TYPE,		3, INT_MAX }, // FUBAR
	{ "field-set-damage-type",		OP_FIELD_SET_DAMAGE_TYPE,		2,2 }, // FUBAR
	{ "disable-ets",				OP_DISABLE_ETS,			1, INT_MAX}, // The E
	{ "enable-ets",					OP_ENABLE_ETS,			1, INT_MAX}, // The E
	{ "set-immobile",		OP_SET_IMMOBILE,			1, INT_MAX	},	// Goober5000
	{ "set-mobile",			OP_SET_MOBILE,			1, INT_MAX	},	// Goober5000
	
	//background and nebula sexps
	{ "mission-set-nebula",			OP_MISSION_SET_NEBULA,				1, 1 }, //-Sesquipedalian
	{ "mission-set-subspace",		OP_MISSION_SET_SUBSPACE,			1, 1 },
	{ "add-background-bitmap",		OP_ADD_BACKGROUND_BITMAP,			9, 9 }, // phreak
	{ "remove-background-bitmap",	OP_REMOVE_BACKGROUND_BITMAP,		1, 1 }, // phreak
	{ "add-sun-bitmap",				OP_ADD_SUN_BITMAP,					6, 6 }, // phreak
	{ "remove-sun-bitmap",			OP_REMOVE_SUN_BITMAP,				1, 1 }, // phreak
	{ "nebula-change-storm",		OP_NEBULA_CHANGE_STORM,				1, 1 }, // phreak
	{ "nebula-toggle-poof",			OP_NEBULA_TOGGLE_POOF,				2, 2 }, // phreak
	{ "set-skybox-model",			OP_SET_SKYBOX_MODEL,				1, 1 },	// taylor
	{ "set-skybox-orientation",		OP_SET_SKYBOX_ORIENT,				3, 3 },	// Goober5000

	//HUD funcs -C
	{ "hud-disable",			OP_HUD_DISABLE,					1, 1 },	// Goober5000
	{ "hud-disable-except-messages",	OP_HUD_DISABLE_EXCEPT_MESSAGES,	1, 1 },	// Goober5000
	{ "hud-set-text",			OP_HUD_SET_TEXT,				2, 2 },	//WMCoolmon
	{ "hud-set-text-num",			OP_HUD_SET_TEXT_NUM,			2, 2 },	//WMCoolmon
	{ "hud-set-message",			OP_HUD_SET_MESSAGE,				2, 2 }, //The E
	{ "hud-set-directive",			OP_HUD_SET_DIRECTIVE,			2, 2 }, //The E
	{ "hud-set-coords",				OP_HUD_SET_COORDS,				3, 3 },	//WMCoolmon
	{ "hud-set-frame",				OP_HUD_SET_FRAME,				2, 2 },	//WMCoolmon
	{ "hud-set-color",				OP_HUD_SET_COLOR,				4, 4 }, //WMCoolmon
	{ "hud-set-max-targeting-range",	OP_HUD_SET_MAX_TARGETING_RANGE,		1, 1 }, // Goober5000
	{ "hud-display-gauge",			OP_HUD_DISPLAY_GAUGE,		2, 2 },
	{ "hud-gauge-set-active",			OP_HUD_GAUGE_SET_ACTIVE,		2, 2 },
	{ "hud-activate-gauge-type",		OP_HUD_ACTIVATE_GAUGE_TYPE,		2, 2},
	{ "hud-clear-messages",			OP_HUD_CLEAR_MESSAGES, 0, 0},	// swifty

	{ "ai-chase",					OP_AI_CHASE,					2, 2, },
	{ "ai-chase-wing",			OP_AI_CHASE_WING,				2, 2, },
	{ "ai-chase-any",				OP_AI_CHASE_ANY,				1, 1, },
	{ "ai-guard",					OP_AI_GUARD,					2, 2, },
	{ "ai-guard-wing",			OP_AI_GUARD_WING,				2, 2, },
	{ "ai-destroy-subsystem",	OP_AI_DESTROY_SUBSYS,		3, 3, },
	{ "ai-disable-ship",			OP_AI_DISABLE_SHIP,			2, 2, },
	{ "ai-disarm-ship",			OP_AI_DISARM_SHIP,			2, 2, },
	{ "ai-warp",					OP_AI_WARP,						2, 2, },
	{ "ai-warp-out",				OP_AI_WARP_OUT,				1, 1, },
	{ "ai-dock",					OP_AI_DOCK,						4, 4, },
	{ "ai-undock",					OP_AI_UNDOCK,					1, 2, },
	{ "ai-waypoints",				OP_AI_WAYPOINTS,				2, 2, },
	{ "ai-waypoints-once",		OP_AI_WAYPOINTS_ONCE,		2, 2, },
	{ "ai-ignore",					OP_AI_IGNORE,					2, 2, },
	{ "ai-ignore-new",				OP_AI_IGNORE_NEW,				2, 2, },
	{ "ai-stay-near-ship",		OP_AI_STAY_NEAR_SHIP,		2, 2, },
	{ "ai-evade-ship",			OP_AI_EVADE_SHIP,				2, 2,	},
	{ "ai-keep-safe-distance",	OP_AI_KEEP_SAFE_DISTANCE,	1, 1, },
	{ "ai-stay-still",			OP_AI_STAY_STILL,				2, 2, },
	{ "ai-play-dead",				OP_AI_PLAY_DEAD,				1, 1, },
	{ "ai-form-on-wing",		OP_AI_FORM_ON_WING,			1,	1 },

	{ "goals",	OP_GOALS_ID,	1, INT_MAX, },

	{ "key-pressed",				OP_KEY_PRESSED,				1,	2,			},
	{ "key-reset",					OP_KEY_RESET,					1, INT_MAX,	},
	{ "key-reset-multiple",			OP_KEY_RESET_MULTIPLE,			1, INT_MAX,	},
	{ "ignore-key",			OP_IGNORE_KEY,			2, INT_MAX	},	// Karajorma
	{ "targeted",					OP_TARGETED,					1, 3,			},
	{ "node-targeted",				OP_NODE_TARGETED,					1, 2,		}, // FUBAR
	{ "missile-locked",				OP_MISSILE_LOCKED,			1,	3	},	// Sesquipedalian
	{ "speed",						OP_SPEED,						1, 1,			},
	{ "facing",						OP_FACING,						2, 2,			},
	{ "facing-waypoint",			OP_FACING2,						2, 2,			},
	{ "order",						OP_ORDER,						2, 3,			},
	{ "query-orders",				OP_QUERY_ORDERS,				3, 6,			}, // Karajorma
	{ "reset-orders",				OP_RESET_ORDERS,				0, 0,			}, // Karajorma
	{ "waypoint-missed",			OP_WAYPOINT_MISSED,			0, 0,			},
	{ "waypoint-twice",			OP_WAYPOINT_TWICE,			0, 0,			},
	{ "path-flown",				OP_PATH_FLOWN,					0, 0,			},
	{ "training-msg",				OP_TRAINING_MSG,				1, 4,			},
	{ "flash-hud-gauge",			OP_FLASH_HUD_GAUGE,			1, 1,			},
	{ "primaries-depleted",		OP_PRIMARIES_DEPLETED,		1, 1,			},
	{ "secondaries-depleted",	OP_SECONDARIES_DEPLETED,	1, 1,			},
	{ "special-check",			OP_SPECIAL_CHECK,				1, 1,			},

	{ "set-training-context-fly-path",	OP_SET_TRAINING_CONTEXT_FLY_PATH,	2, 2, },
	{ "set-training-context-speed",		OP_SET_TRAINING_CONTEXT_SPEED,		2, 2, },

	//Cutscene stuff
	{ "set-cutscene-bars",			OP_CUTSCENES_SET_CUTSCENE_BARS,			0, 1, },
	{ "unset-cutscene-bars",		OP_CUTSCENES_UNSET_CUTSCENE_BARS,		0, 1, },
	{ "fade-in",					OP_CUTSCENES_FADE_IN,					0, 1, },
	{ "fade-out",					OP_CUTSCENES_FADE_OUT,					0, 2, },
	{ "set-camera",					OP_CUTSCENES_SET_CAMERA,				0, 1, },
	{ "set-camera-facing",			OP_CUTSCENES_SET_CAMERA_FACING,			3, 6, },
	{ "set-camera-facing-object",	OP_CUTSCENES_SET_CAMERA_FACING_OBJECT,	1, 4, },
	{ "set-camera-fov",				OP_CUTSCENES_SET_CAMERA_FOV,			1, 5, },
	{ "set-camera-host",			OP_CUTSCENES_SET_CAMERA_HOST,			1, 2, },
	{ "set-camera-position",		OP_CUTSCENES_SET_CAMERA_POSITION,		3, 6, },
	{ "set-camera-rotation",		OP_CUTSCENES_SET_CAMERA_ROTATION,		3, 6, },
	{ "set-camera-target",			OP_CUTSCENES_SET_CAMERA_TARGET,			1, 2, },
	{ "set-fov",					OP_CUTSCENES_SET_FOV,					1, 1, },
	{ "get-fov",					OP_CUTSCENES_GET_FOV,					0, 0, },
	{ "reset-fov",					OP_CUTSCENES_RESET_FOV,					0, 0, },
	{ "reset-camera",				OP_CUTSCENES_RESET_CAMERA,				0, 1, },
	{ "show-subtitle",				OP_CUTSCENES_SHOW_SUBTITLE,				4, 13, },
	{ "show-subtitle-text",			OP_CUTSCENES_SHOW_SUBTITLE_TEXT,		6, 13, },
	{ "show-subtitle-image",		OP_CUTSCENES_SHOW_SUBTITLE_IMAGE,		8, 10, },
	{ "set-time-compression",		OP_CUTSCENES_SET_TIME_COMPRESSION,		1, 3, },
	{ "reset-time-compression",		OP_CUTSCENES_RESET_TIME_COMPRESSION,	0, 0, },
	{ "lock-perspective",			OP_CUTSCENES_FORCE_PERSPECTIVE,			1, 2, },
	{ "set-camera-shudder",			OP_SET_CAMERA_SHUDDER,					2, 2, },

	{ "set-jumpnode-name",			OP_JUMP_NODE_SET_JUMPNODE_NAME,			2, 2, }, //CommanderDJ
	{ "set-jumpnode-color",			OP_JUMP_NODE_SET_JUMPNODE_COLOR,		5, 5, },
	{ "set-jumpnode-model",			OP_JUMP_NODE_SET_JUMPNODE_MODEL,		3, 3, },
	{ "show-jumpnode",				OP_JUMP_NODE_SHOW_JUMPNODE,				1, INT_MAX, },
	{ "hide-jumpnode",				OP_JUMP_NODE_HIDE_JUMPNODE,				1, INT_MAX, },

	{ "script-eval-num",			OP_SCRIPT_EVAL_NUM,						1, 1, },
	{ "script-eval-string",			OP_SCRIPT_EVAL_STRING,					1, 1, },
	{ "script-eval",				OP_SCRIPT_EVAL,							1, INT_MAX},
	{ "string-to-int",				OP_STRING_TO_INT,						1, 1,			}, // Karajorma
	{ "int-to-string",				OP_INT_TO_STRING,						2, 2,			}, // Goober5000
	{ "string-concatenate",			OP_STRING_CONCATENATE,					3, 3,			}, // Goober5000
	{ "string-get-substring",		OP_STRING_GET_SUBSTRING,				4, 4,	}, // Goober5000
	{ "string-set-substring",		OP_STRING_SET_SUBSTRING,				5, 5,	}, // Goober5000
	{ "string-get-length",			OP_STRING_GET_LENGTH,					1, 1,	}, // Goober5000

	{ "do-nothing",	OP_NOP,	0, 0,			},
};

sexp_ai_goal_link Sexp_ai_goal_links[] = {
	{ AI_GOAL_CHASE, OP_AI_CHASE },
	{ AI_GOAL_CHASE_WING, OP_AI_CHASE_WING },
	{ AI_GOAL_DOCK, OP_AI_DOCK },
	{ AI_GOAL_UNDOCK, OP_AI_UNDOCK },
	{ AI_GOAL_WARP, OP_AI_WARP_OUT },
	{ AI_GOAL_WARP, OP_AI_WARP },
	{ AI_GOAL_WAYPOINTS, OP_AI_WAYPOINTS },
	{ AI_GOAL_WAYPOINTS_ONCE, OP_AI_WAYPOINTS_ONCE },
	{ AI_GOAL_DESTROY_SUBSYSTEM, OP_AI_DESTROY_SUBSYS },
	{ AI_GOAL_DISABLE_SHIP, OP_AI_DISABLE_SHIP },
	{ AI_GOAL_DISARM_SHIP, OP_AI_DISARM_SHIP },
	{ AI_GOAL_GUARD, OP_AI_GUARD },
	{ AI_GOAL_CHASE_ANY, OP_AI_CHASE_ANY },
	{ AI_GOAL_GUARD_WING, OP_AI_GUARD_WING },
	{ AI_GOAL_EVADE_SHIP, OP_AI_EVADE_SHIP },
	{ AI_GOAL_STAY_NEAR_SHIP, OP_AI_STAY_NEAR_SHIP },
	{ AI_GOAL_KEEP_SAFE_DISTANCE, OP_AI_KEEP_SAFE_DISTANCE },
	{ AI_GOAL_IGNORE, OP_AI_IGNORE },
	{ AI_GOAL_IGNORE_NEW, OP_AI_IGNORE_NEW },
	{ AI_GOAL_STAY_STILL, OP_AI_STAY_STILL },
	{ AI_GOAL_PLAY_DEAD, OP_AI_PLAY_DEAD },
	{ AI_GOAL_FORM_ON_WING, OP_AI_FORM_ON_WING }
};

char *HUD_gauge_text[NUM_HUD_GAUGES] = 
{
	"LEAD_INDICATOR",
	"ORIENTATION_TEE",
	"HOSTILE_TRIANGLE",
	"TARGET_TRIANGLE",
	"MISSION_TIME",
	"RETICLE_CIRCLE",
	"THROTTLE_GAUGE",
	"RADAR",
	"TARGET_MONITOR",
	"CENTER_RETICLE",
	"TARGET_MONITOR_EXTRA_DATA",
	"TARGET_SHIELD_ICON",
	"PLAYER_SHIELD_ICON",
	"ETS_GAUGE",
	"AUTO_TARGET",
	"AUTO_SPEED",
	"WEAPONS_GAUGE",
	"ESCORT_VIEW",
	"DIRECTIVES_VIEW",
	"THREAT_GAUGE",
	"AFTERBURNER_ENERGY",
	"WEAPONS_ENERGY",
	"WEAPON_LINKING_GAUGE",
	"TARGER_MINI_ICON",
	"OFFSCREEN_INDICATOR",
	"TALKING_HEAD",
	"DAMAGE_GAUGE",
	"MESSAGE_LINES",
	"MISSILE_WARNING_ARROW",
	"CMEASURE_GAUGE",
	"OBJECTIVES_NOTIFY_GAUGE",
	"WINGMEN_STATUS",
	"OFFSCREEN RANGE",
	"KILLS GAUGE",
	"ATTACKING TARGET COUNT",
	"TEXT FLASH",
	"MESSAGE BOX",
	"SUPPORT GUAGE",
	"LAG GUAGE"
};


void sexp_set_skybox_model_preload(char *name); // taylor

int	Directive_count;
int	Sexp_useful_number;  // a variable to pass useful info in from external modules
int	Locked_sexp_true, Locked_sexp_false;
int	Num_operators = sizeof(Operators) / sizeof(sexp_oper);
int	Num_sexp_ai_goal_links = sizeof(Sexp_ai_goal_links) / sizeof(sexp_ai_goal_link);
int	Sexp_clipboard = -1;  // used by Fred
int	Training_context = 0;
int	Training_context_speed_set;
int	Training_context_speed_min;
int	Training_context_speed_max;
int	Training_context_speed_timestamp;
waypoint_list *Training_context_path;
int Training_context_goal_waypoint;
int Training_context_at_waypoint;
float	Training_context_distance;

#define SEXP_NODE_INCREMENT	250
int Num_sexp_nodes = 0;
sexp_node *Sexp_nodes = NULL;

sexp_variable Sexp_variables[MAX_SEXP_VARIABLES];
sexp_variable Block_variables[MAX_SEXP_VARIABLES];			// used for compatibility with retail. 

int Num_special_expl_blocks;

SCP_vector<int> Current_sexp_operator;

int Players_target = UNINITIALIZED;
int Players_mlocked = UNINITIALIZED; // for is-missile-locked - Sesquipedalian
ship_subsys *Players_targeted_subsys;
int Players_target_timestamp;
int Players_mlocked_timestamp;

// for play-music - Goober5000
int	Sexp_music_handle = -1;
void sexp_stop_music(int fade = 1);

// for sound environments - Goober5000/Taylor
#define SEO_VOLUME		0
#define SEO_DECAY_TIME	1
#define SEO_DAMPING		2
int sexp_sound_environment_option_lookup(char *text);
char *Sound_environment_option[] = { "volume", "decay time", "damping" };
int Num_sound_environment_options = 3;

// for adjust-audio-volume - The E
char *Adjust_audio_options[] = { "Music", "Voice", "Effects" };
int Num_adjust_audio_options = 3;
int audio_volume_option_lookup(char *text);

int hud_gauge_type_lookup(char* name);

// for explosions - Goober5000
#define EO_DAMAGE			0
#define EO_BLAST			1
#define EO_INNER_RADIUS		2
#define EO_OUTER_RADIUS		3
#define EO_SHOCKWAVE_SPEED	4
#define EO_DEATH_ROLL_TIME	5
int sexp_explosion_option_lookup(char *text);
char *Explosion_option[] = { "damage", "blast", "inner radius", "outer radius", "shockwave speed", "death roll time" };
int Num_explosion_options = 6;

int get_sexp(char *token);
void build_extended_sexp_string(SCP_string &accumulator, int cur_node, int level, int mode);
void update_sexp_references(const char *old_name, const char *new_name, int format, int node);
int sexp_determine_team(char *subj);
int extract_sexp_variable_index(int node);
void init_sexp_vars();
int eval_num(int node);

// for handling variables
void add_block_variable(const char *text, const char *var_name, int type, int index);
void sexp_modify_variable(int node);
int sexp_get_variable_by_index(int node);
void sexp_set_variable_by_index(int node);

SCP_vector<char*> Sexp_replacement_arguments;
int Sexp_current_argument_nesting_level;
SCP_vector<char*> Applicable_arguments_temp;

// Goober5000
arg_item Sexp_applicable_argument_list;
bool is_blank_argument_op(int op_const);
bool is_blank_of_op(int op_const);

int get_handler_for_x_of_operator(int node);

//Karajorma
int get_generic_subsys(char *subsy_name);
bool ship_class_unchanged(int ship_index); 
void multi_sexp_modify_variable();

int get_effect_from_name(char* name);

#define NO_OPERATOR_INDEX_DEFINED		-2
#define NOT_A_SEXP_OPERATOR				-1

// Karajorma - some useful helper methods
player * get_player_from_ship_node(int node, bool test_respawns = false);
ship * sexp_get_ship_from_node(int node);

// hud-display-gauge magic values
#define SEXP_HUD_GAUGE_WARPOUT "warpout"

// Goober5000 - arg_item class stuff, borrowed from sexp_list_item class stuff -------------
void arg_item::add_data(char *str)
{
	arg_item *item, *ptr;

	// create item
	item = new arg_item;
	item->text = str;
	item->nesting_level = Sexp_current_argument_nesting_level;

	// prepend item to existing list
	ptr = this->next;
	this->next = item;
	item->next = ptr;
}

void arg_item::add_data_dup(char *str)
{
	arg_item *item, *ptr;

	// create item
	item = new arg_item;
	item->text = strdup(str);
	item->flags |= ARG_ITEM_F_DUP;
	item->nesting_level = Sexp_current_argument_nesting_level;

	// prepend item to existing list
	ptr = this->next;
	this->next = item;
	item->next = ptr;
}

void arg_item::add_data_set_dup(char *str)
{
	arg_item *item, *ptr;

	// create item
	item = new arg_item;
	item->text = str;
	item->flags |= ARG_ITEM_F_DUP;
	item->nesting_level = Sexp_current_argument_nesting_level;

	// prepend item to existing list
	ptr = this->next;
	this->next = item;
	item->next = ptr;
}

arg_item* arg_item::get_next()
{
	if (this->next != NULL) {
		if (this->next->nesting_level >= Sexp_current_argument_nesting_level) {
			return this->next; 
		}
	}

	return NULL;
}

void arg_item::expunge()
{
	arg_item *ptr;

	// contiually delete first item of list
	while (this->next != NULL)
	{
		ptr = this->next->next;

		if (this->next->flags & ARG_ITEM_F_DUP)
			free(this->next->text);
		delete this->next;

		this->next = ptr;
	}
}

void arg_item::clear_nesting_level()
{
	arg_item *ptr;

	// contiually delete first item of list
	while (this->next != NULL && this->next->nesting_level >= Sexp_current_argument_nesting_level )
	{
		ptr = this->next->next;

		if (this->next->flags & ARG_ITEM_F_DUP)
			free(this->next->text);
		delete this->next;

		this->next = ptr;
	}
}

int arg_item::is_empty()
{
	return (this->next == NULL);
}
//-------------------------------------------------------------------------------------------------

void sexp_nodes_init()
{
	if (Num_sexp_nodes == 0 || Sexp_nodes == NULL)
		return;

	nprintf(("SEXP", "Reinitializing sexp nodes...\n"));
	nprintf(("SEXP", "Entered function with %d nodes.\n", Num_sexp_nodes));

	// usually, the persistent nodes are grouped at the beginning of the array;
	// so we ought to be able to free all the subsequent nodes
	int i, last_persistent_node = -1;

	for (i = 0; i < Num_sexp_nodes; i++)
	{
		if (Sexp_nodes[i].type & SEXP_FLAG_PERSISTENT)
			last_persistent_node = i;					// keep track of it
		else
			Sexp_nodes[i].type = SEXP_NOT_USED;			// it's not needed
	}

	nprintf(("SEXP", "Last persistent node index is %d.\n", last_persistent_node));

	// if all the persistent nodes are gone, free all the nodes
	if (last_persistent_node == -1)
	{
		vm_free(Sexp_nodes);
		Sexp_nodes = NULL;
		Num_sexp_nodes = 0;
	}
	// if there's enough of a difference to make it worthwhile, free some nodes
	else if (Num_sexp_nodes - (last_persistent_node + 1) > 2 * SEXP_NODE_INCREMENT)
	{
		// round it up to the next evenly divisible size
		Num_sexp_nodes = (last_persistent_node + 1);
		Num_sexp_nodes += SEXP_NODE_INCREMENT - (Num_sexp_nodes % SEXP_NODE_INCREMENT);

		Sexp_nodes = (sexp_node *) vm_realloc(Sexp_nodes, sizeof(sexp_node) * Num_sexp_nodes);
		Verify(Sexp_nodes != NULL);
	}

	nprintf(("SEXP", "Exited function with %d nodes.\n", Num_sexp_nodes));
}

static void sexp_nodes_close()
{
	// free all sexp nodes... should only be done on game shutdown
	if (Sexp_nodes != NULL)
	{
		vm_free(Sexp_nodes);
		Sexp_nodes = NULL;
		Num_sexp_nodes = 0;
	}
}

void init_sexp()
{
	// Goober5000
	Sexp_replacement_arguments.clear();
	Sexp_applicable_argument_list.expunge();
	Sexp_current_argument_nesting_level = 0;
	initalise_sexp_packet();

	static bool done_sexp_atexit = false;
	if (!done_sexp_atexit)
	{
		atexit(sexp_nodes_close);
		done_sexp_atexit = true;
	}

	sexp_nodes_init();
	init_sexp_vars();
	Locked_sexp_false = Locked_sexp_true = -1;

	Locked_sexp_false = alloc_sexp("false", SEXP_LIST, SEXP_ATOM_OPERATOR, -1, -1);
	Assert(Locked_sexp_false != -1);
	Sexp_nodes[Locked_sexp_false].type = SEXP_ATOM;  // fix bypassing value
	Sexp_nodes[Locked_sexp_false].value = SEXP_KNOWN_FALSE;

	Locked_sexp_true = alloc_sexp("true", SEXP_LIST, SEXP_ATOM_OPERATOR, -1, -1);
	Assert(Locked_sexp_true != -1);
	Sexp_nodes[Locked_sexp_true].type = SEXP_ATOM;  // fix bypassing value
	Sexp_nodes[Locked_sexp_true].value = SEXP_KNOWN_TRUE;
}

/**
 * Allocate an sexp node.
 */
int alloc_sexp(char *text, int type, int subtype, int first, int rest)
{
	int node;
	int sexp_const = get_operator_const(text);

	if ((sexp_const == OP_TRUE) && (type == SEXP_ATOM) && (subtype == SEXP_ATOM_OPERATOR))
		return Locked_sexp_true;

	else if ((sexp_const == OP_FALSE) && (type == SEXP_ATOM) && (subtype == SEXP_ATOM_OPERATOR))
		return Locked_sexp_false;

	node = find_free_sexp();

	// need more sexp nodes?
	if (node == Num_sexp_nodes || node == -1)
	{
		int old_size = Num_sexp_nodes;

		Assert(SEXP_NODE_INCREMENT > 0);

		// allocate in blocks of SEXP_NODE_INCREMENT
		Num_sexp_nodes += SEXP_NODE_INCREMENT;
		Sexp_nodes = (sexp_node *) vm_realloc(Sexp_nodes, sizeof(sexp_node) * Num_sexp_nodes);

		Verify(Sexp_nodes != NULL);
		nprintf(("SEXP", "Bumping dynamic sexp node limit from %d to %d...\n", old_size, Num_sexp_nodes));

		// clear all the new sexp nodes we just allocated
		memset(&Sexp_nodes[old_size], 0, sizeof(sexp_node) * SEXP_NODE_INCREMENT); //-V512

		// our new sexp is the first out of the ones we just created
		node = old_size;
	}

	Assert(node != Locked_sexp_true);
	Assert(node != Locked_sexp_false);
	Assert(strlen(text) < TOKEN_LENGTH);
	Assert(type >= 0);

	strcpy_s(Sexp_nodes[node].text, text);
	Sexp_nodes[node].type = type;
	Sexp_nodes[node].subtype = subtype;
	Sexp_nodes[node].first = first;
	Sexp_nodes[node].rest = rest;
	Sexp_nodes[node].value = SEXP_UNKNOWN;
	Sexp_nodes[node].flags = SNF_DEFAULT_VALUE;	// Goober5000
	Sexp_nodes[node].op_index = NO_OPERATOR_INDEX_DEFINED;

	return node;
}

static int Sexp_hwm = 0;

int count_free_sexp_nodes()
{
	int i, f = 0, p = 0;

	for (i = 0; i < Num_sexp_nodes; i++)
	{
		if (Sexp_nodes[i].type == SEXP_NOT_USED)
			f++;
		else if (Sexp_nodes[i].type & SEXP_FLAG_PERSISTENT)
			p++;
	}

	if (Num_sexp_nodes - f > Sexp_hwm)
	{
		nprintf(("Sexp", "Sexp nodes: Free=%d, Used=%d, Persistent=%d\n", f, Num_sexp_nodes - f, p));
		Sexp_hwm = Num_sexp_nodes - f;
	}

	return f;
}

/**
 * Find the next free sexp and return its index.
 */
int find_free_sexp()
{
	int i;

	// sanity
	if (Num_sexp_nodes == 0 || Sexp_nodes == NULL)
		return -1;

	for (i = 0; i < Num_sexp_nodes; i++)
	{
		if (Sexp_nodes[i].type == SEXP_NOT_USED)
			return i;
	}

	return -1;
}

/**
 * Mark a whole sexp tree with the persistent flag so that it won't get re-used between missions
 */
void sexp_mark_persistent(int n)
{
	if (n == -1){
		return;
	}

	// total hack because of the true/false locked sexps -- we should make those persistent as well
	if ( (n == Locked_sexp_true) || (n == Locked_sexp_false) ){
		return;
	}

	Assert( !(Sexp_nodes[n].type & SEXP_FLAG_PERSISTENT) );
	Sexp_nodes[n].type |= SEXP_FLAG_PERSISTENT;

	sexp_mark_persistent(Sexp_nodes[n].first);
	sexp_mark_persistent(Sexp_nodes[n].rest);

}

/**
 * Remove the persistent flag from all nodes in the tree
 */
void sexp_unmark_persistent(int n)
{
	if (n == -1){
		return;
	}

	if ( (n == Locked_sexp_true) || (n == Locked_sexp_false) ){
		return;
	}

	Assert( Sexp_nodes[n].type & SEXP_FLAG_PERSISTENT );
	Sexp_nodes[n].type &= ~SEXP_FLAG_PERSISTENT;

	sexp_unmark_persistent(Sexp_nodes[n].first);
	sexp_unmark_persistent(Sexp_nodes[n].rest);
}

/**
 * Free up the specified sexp node,  Leaves link chains untouched.
 */
int free_one_sexp(int num)
{
	Assert((num >= 0) && (num < Num_sexp_nodes));
	Assert(Sexp_nodes[num].type != SEXP_NOT_USED);  // make sure it is actually used
	Assert(!(Sexp_nodes[num].type & SEXP_FLAG_PERSISTENT));

	if ((num == Locked_sexp_true) || (num == Locked_sexp_false))
		return 0;

	Sexp_nodes[num].type = SEXP_NOT_USED;
	return 1;
}

/**
 * Free a used sexp node, so it can be reused later.  
 *
 * Should only be called on an atom or a list, and not an operator.  If on a list, the 
 * list and everything in it will be freed (including the operator).
 */
int free_sexp(int num)
{
	int i, rest, count = 0;

	Assert((num >= 0) && (num < Num_sexp_nodes));
	Assert(Sexp_nodes[num].type != SEXP_NOT_USED);  // make sure it is actually used
	Assert(!(Sexp_nodes[num].type & SEXP_FLAG_PERSISTENT));

	if ((num == Locked_sexp_true) || (num == Locked_sexp_false))
		return 0;

	Sexp_nodes[num].type = SEXP_NOT_USED;
	count++;

	i = Sexp_nodes[num].first;
	while (i != -1)
	{
		count += free_sexp(i);
		i = Sexp_nodes[i].rest;
	}

	rest = Sexp_nodes[num].rest;
	for (i = 0; i < Num_sexp_nodes; i++)
	{
		if (Sexp_nodes[i].first == num)
			Sexp_nodes[i].first = rest;

		if (Sexp_nodes[i].rest == num)
			Sexp_nodes[i].rest = rest;
	}

	return count;  // total elements freed up.
}

/**
 * Free up an entire sexp tree.  
 * 
 * Because the root node is an operator, instead of a list, we can't simply call free_sexp().  
 * This function should only be called on the root node of an sexp, otherwise the linking will get screwed up.
 */
int free_sexp2(int num)
{	
	int i, count = 0;

	if ((num == -1) || (num == Locked_sexp_true) || (num == Locked_sexp_false)){
		return 0;
	}

	i = Sexp_nodes[num].rest;
	while (i != -1) {
		count += free_sexp(i);
		i = Sexp_nodes[i].rest;
	}

	count += free_sexp(num);
	return count;
}

/**
 * Reset the status of all the nodes in a tree, forcing them to all be evaulated again.
 */
void flush_sexp_tree(int node)
{
	if (node < 0){
		return;
	}

	Sexp_nodes[node].value = SEXP_UNKNOWN;
	flush_sexp_tree(Sexp_nodes[node].first);
	flush_sexp_tree(Sexp_nodes[node].rest);
}

int verify_sexp_tree(int node)
{
	if (node == -1){
		return 0;
	}

	if ((Sexp_nodes[node].type == SEXP_NOT_USED) ||
		(Sexp_nodes[node].first == node) ||
		(Sexp_nodes[node].rest == node)) {
		Error(LOCATION, "Sexp node is corrupt");
		return -1;
	}

	if (Sexp_nodes[node].first != -1){
		verify_sexp_tree(Sexp_nodes[node].first);
	}
	if (Sexp_nodes[node].rest != -1){
		verify_sexp_tree(Sexp_nodes[node].rest);
	}

	return 0;
}

/**
 * @todo CASE OF SEXP VARIABLES - ONLY 1 COPY OF VARIABLE
 */
int dup_sexp_chain(int node)
{
	int cur, first, rest;

	if (node == -1){
		return -1;
	}

	// TODO - CASE OF SEXP VARIABLES - ONLY 1 COPY OF VARIABLE
	first = dup_sexp_chain(Sexp_nodes[node].first);
	rest = dup_sexp_chain(Sexp_nodes[node].rest);
	cur = alloc_sexp(Sexp_nodes[node].text, Sexp_nodes[node].type, Sexp_nodes[node].subtype, first, rest);

	if (cur == -1) {
		if (first != -1){
			free_sexp(first);
		}
		if (rest != -1){
			free_sexp(rest);
		}
	}

	return cur;
}

/**
 * Compare SEXP chains
 * @return 1 if they are the same, 0 if different
 */
int cmp_sexp_chains(int node1, int node2)
{
	if ((node1 == -1) && (node2 == -1)){
		return 1;
	}

	if ((node1 == -1) || (node2 == -1)){
		return 0;
	}

	// DA: 1/7/99 Need to check the actual Sexp_node.text, not possible variable, which can be equal
	if (stricmp(Sexp_nodes[node1].text, Sexp_nodes[node2].text)){
		return 0;
	}

	if (!cmp_sexp_chains(Sexp_nodes[node1].first, Sexp_nodes[node2].first)){
		return 0;
	}

	if (!cmp_sexp_chains(Sexp_nodes[node1].rest, Sexp_nodes[node2].rest)){
		return 0;
	}

	return 1;
}

/**
 * Determine if an sexp node is within the given sexp chain.
 */
int query_node_in_sexp(int node, int sexp)
{
	if (sexp == -1){
		return 0;
	}
	if (node == sexp){
		return 1;
	}

	if (query_node_in_sexp(node, Sexp_nodes[sexp].first)){
		return 1;
	}
	if (query_node_in_sexp(node, Sexp_nodes[sexp].rest)){
		return 1;
	}

	return 0;
}

/**
 * Find the index of the list associated with an operator
 */
int find_sexp_list(int num)
{
	int i;

	for (i = 0; i < Num_sexp_nodes; i++)
	{
		if (Sexp_nodes[i].first == num)
			return i;
	}

	// assume that it was the first item in the list
	return num;
}

/**
 * Find node of operator that item is an argument of.
 */
int find_parent_operator(int node)
{
	int i;
	int n = node;

	Assert((node >= 0) && (node < Num_sexp_nodes));

	if (Sexp_nodes[n].subtype == SEXP_ATOM_OPERATOR)
		n = find_sexp_list(n);

	Assert( (n >= 0) && (n < Num_sexp_nodes) );

	while (Sexp_nodes[n].subtype != SEXP_ATOM_OPERATOR)
	{
		for (i = 0; i < Num_sexp_nodes; i++)
		{
			if (Sexp_nodes[i].rest == n)
				break;
		}

		if (i == Num_sexp_nodes)
			return -1;  // not found, probably at top node already.

		n = i;
	}

	return n;
}

/**
 * Determine if an sexpression node is the top level node of an sexpression tree.
 *
 * Top level nodes do not have their node id in anyone elses first or rest index.
 */
int is_sexp_top_level( int node )
{
	int i;

	Assert((node >= 0) && (node < Num_sexp_nodes));

	if (Sexp_nodes[node].type == SEXP_NOT_USED)
		return 0;

	for (i = 0; i < Num_sexp_nodes; i++)
	{
		if ((Sexp_nodes[i].type == SEXP_NOT_USED) || (i == node ))				// don't check myself or unused nodes
			continue;

		if ((Sexp_nodes[i].first == node) || (Sexp_nodes[i].rest == node))
			return 0;
	}

	return 1;
}

/**
 * Find argument number
 */
int find_argnum(int parent, int arg)
{
	int n, tally;

	n = CDR(parent);
	tally = 0;
	
	while (CAR(n) != arg)
	{
		if (n == -1)
			return -1;

		tally++;
		n = CDR(n);
	}

	return tally;
}

/**
 * From an operator name, return its index in the array Operators
 */
int get_operator_index(char *token)
{
	int	i;

	for (i=0; i<Num_operators; i++){
		if (!stricmp(token, Operators[i].text)){
			return i;
		}
	}

	return NOT_A_SEXP_OPERATOR;
}

/**
 * From a sexp node, return the index in the array Operators or 0 if not an operator
 */
int get_operator_index(int node)
{
	if (!Fred_running && (Sexp_nodes[node].op_index != NO_OPERATOR_INDEX_DEFINED) ) {
		return Sexp_nodes[node].op_index;
	}

	int index = get_operator_index(Sexp_nodes[node].text); 
	Sexp_nodes[node].op_index = index;
	return index;
}


/**
 * From an operator name, return its constant (the number it was define'd with)
 */
int get_operator_const(char *token)
{
	int	idx = get_operator_index(token);

	if (idx == NOT_A_SEXP_OPERATOR)
		return 0;

	return Operators[idx].value;
}

int get_operator_const(int node)
{
	if (!Fred_running && Sexp_nodes[node].op_index >= 0) {
		return Operators[Sexp_nodes[node].op_index].value;
	}

	int	idx = get_operator_index(node);

	if (idx == NOT_A_SEXP_OPERATOR)
		return 0;

	return Operators[idx].value;
}

int query_sexp_args_count(int node, bool only_valid_args = false)
{
	int count = 0;
	int n = CDR(node);

	for ( ; n != -1; n = CDR(n))
	{
		if (only_valid_args && !(Sexp_nodes[n].flags & SNF_ARGUMENT_VALID))
			continue;

		count++;
	}

	return count;
}

/**
 * Needed to fix bug with sexps like send-message list which have arguments that need to be supplied as a block
 * 
 * @return 0 if the number of arguments for the supplied operation is wrong, 1 otherwise.
 */
int check_operator_argument_count(int count, int op)
{
	if (count < Operators[op].min || count > Operators[op].max)
		return 0;

	//send-message-list has arguments as blocks of 4
	if (op == OP_SEND_MESSAGE_LIST)
		if (count % 4 != 0)
			return 0;

	return 1;
}

/**
 * Check SEXP syntax
 * @return 0 if ok, negative if there's an error in expression..
 * See the returns types in sexp.h
 */
int check_sexp_syntax(int node, int return_type, int recursive, int *bad_node, int mode)
{
	int i = 0, z, t, type, argnum = 0, count, op, type2 = 0, op2;
	int op_node;
	int var_index = -1;
	size_t st;

	Assert(node >= 0 && node < Num_sexp_nodes);
	Assert(Sexp_nodes[node].type != SEXP_NOT_USED);
	if (Sexp_nodes[node].subtype == SEXP_ATOM_NUMBER && return_type == OPR_BOOL) {
		// special case Mark seems to want supported
		Assert(Sexp_nodes[node].first == -1);  // only lists should have a first pointer
		if (Sexp_nodes[node].rest != -1)  // anything after the number?
			return SEXP_CHECK_NONOP_ARGS; // if so, it's a syntax error

		return 0;
	}

	op_node = node;		// save the node of the operator since we need to get to other args.
	if (bad_node)
		*bad_node = op_node;

	if (Sexp_nodes[op_node].subtype != SEXP_ATOM_OPERATOR)
		return SEXP_CHECK_OP_EXPECTED;  // not an operator, which it should always be

	op = get_operator_index(CTEXT(op_node));
	if (op == -1)
		return SEXP_CHECK_UNKNOWN_OP;  // unrecognized operator

	// check that types match - except that OPR_AMBIGUOUS matches everything
	if (return_type != OPR_AMBIGUOUS)
	{
		// get the return type of the next thing
		z = query_operator_return_type(op);
		if (z == OPR_POSITIVE && return_type == OPR_NUMBER)
		{
			// positive data type can map to number data type just fine
		}
		// Goober5000's number hack
		else if (z == OPR_NUMBER && return_type == OPR_POSITIVE)
		{
			// this isn't kosher, but we hack it to make it work
		}
		else if (z != return_type)
		{
			// anything else is a mismatch
			return SEXP_CHECK_TYPE_MISMATCH;
		}
	}

	count = query_sexp_args_count(op_node);

	if (!check_operator_argument_count(count, op))
		return SEXP_CHECK_BAD_ARG_COUNT;  // incorrect number of arguments

	// Goober5000 - if this is a list of stuff that has the special argument as
	// an item in the list, assume it's valid
	if (special_argument_appears_in_sexp_list(op_node))
		return 0;

	node = Sexp_nodes[op_node].rest;
	while (node != -1) {
		type = query_operator_argument_type(op, argnum);
		Assert(Sexp_nodes[node].type != SEXP_NOT_USED);
		if (bad_node)
			*bad_node = node;

		if (Sexp_nodes[node].subtype == SEXP_ATOM_LIST) {
			i = Sexp_nodes[node].first;
			if (bad_node)
				*bad_node = i;
			
			// be sure to check to see if this node is a list of stuff and not an actual operator type
			// thing.  (i.e. in the case of a cond statement, the conditional will fall into this if
			// statement.  MORE TO DO HERE!!!!
			if (Sexp_nodes[i].subtype == SEXP_ATOM_LIST)
				return 0;

			op2 = get_operator_index(CTEXT(i));
			if (op2 == -1)
				return SEXP_CHECK_UNKNOWN_OP;

			type2 = query_operator_return_type(op2);
			if (recursive) {
				switch (type) {
					case OPF_NUMBER:
						t = OPR_NUMBER;
						break;

					case OPF_POSITIVE:
						t = OPR_POSITIVE;
						break;

					case OPF_BOOL:
						t = OPR_BOOL;
						break;

					case OPF_NULL:
						t = OPR_NULL;
						break;

					// Goober5000
					case OPF_FLEXIBLE_ARGUMENT:
						t = OPR_FLEXIBLE_ARGUMENT;
						break;

					case OPF_AI_GOAL:
						t = OPR_AI_GOAL;
						break;

					// special case for modify-variable
					case OPF_AMBIGUOUS:
						t = OPR_AMBIGUOUS;
						break;

					default:
						return SEXP_CHECK_UNKNOWN_TYPE;  // no other return types available
				}

				if ((z = check_sexp_syntax(i, t, recursive, bad_node)) != 0) {
					return z;
				}
			}

		} else if (Sexp_nodes[node].subtype == SEXP_ATOM_NUMBER) {
			char *ptr;

			type2 = OPR_POSITIVE;
			ptr = CTEXT(node);
			if (*ptr == '-') {
				type2 = OPR_NUMBER;
				ptr++;
			}

			if (type == OPF_BOOL)  // allow numbers to be used where boolean is required.
				type2 = OPR_BOOL;

			while (*ptr) {
				if (!isdigit(*ptr))
					return SEXP_CHECK_INVALID_NUM;  // not a valid number

				ptr++;
			}

			i = atoi(CTEXT(node));
			z = get_operator_const(CTEXT(op_node));
			if ( (z == OP_HAS_DOCKED_DELAY) || (z == OP_HAS_UNDOCKED_DELAY) )
				if ( (argnum == 2) && (i < 1) )
					return SEXP_CHECK_NUM_RANGE_INVALID;

			z = get_operator_index(CTEXT(op_node));
			if ( (query_operator_return_type(z) == OPR_AI_GOAL) && (argnum == Operators[op].min - 1) )
				if ( (i < 0) || (i > 200) )
					return SEXP_CHECK_NUM_RANGE_INVALID;

		} else if (Sexp_nodes[node].subtype == SEXP_ATOM_STRING) {
			type2 = SEXP_ATOM_STRING;

		} else {
			Assert(0);
		}

		// variables should only be typechecked. 
		if ((Sexp_nodes[node].type & SEXP_FLAG_VARIABLE) && (type != OPF_VARIABLE_NAME)) {
			var_index = get_index_sexp_variable_from_node(node);
			Assert(var_index != -1);
	
			switch (type) {
				case OPF_NUMBER:
				case OPF_POSITIVE:
					if (!(Sexp_variables[var_index].type & SEXP_VARIABLE_NUMBER)) 
						return SEXP_CHECK_INVALID_VARIABLE_TYPE; 
				break;

                case OPF_AMBIGUOUS:
                    break;

				default: 
					if (!(Sexp_variables[var_index].type & SEXP_VARIABLE_STRING)) 
						return SEXP_CHECK_INVALID_VARIABLE_TYPE; 
			}			
			node = Sexp_nodes[node].rest;
			argnum++;
			continue; 
		}

		switch (type) {
			case OPF_NAV_POINT:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}
				break;

			case OPF_NUMBER:
				if ((type2 != OPR_NUMBER) && (type2 != OPR_POSITIVE)){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				break;

			case OPF_POSITIVE:
				if (type2 == OPR_NUMBER){
					// Goober5000's number hack
					break;
					// return SEXP_CHECK_NEGATIVE_NUM;
				}

				if (type2 != OPR_POSITIVE){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				break;

			case OPF_SHIP_NOT_PLAYER:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (ship_name_lookup(CTEXT(node), 0) < 0)
				{
					if (Fred_running || !mission_parse_get_arrival_ship(CTEXT(node)))
					{
						return SEXP_CHECK_INVALID_SHIP;
					}
				}

				break;

			case OPF_SHIP_OR_NONE:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (stricmp(CTEXT(node), SEXP_NONE_STRING))		// none is okay
				{
					if (ship_name_lookup(CTEXT(node), 1) < 0)
					{
						if (Fred_running || !mission_parse_get_arrival_ship(CTEXT(node)))
						{
							return SEXP_CHECK_INVALID_SHIP;
						}
					}
				}

				break;

			case OPF_SHIP:
			case OPF_SHIP_POINT:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (ship_name_lookup(CTEXT(node), 1) < 0) {
					if (Fred_running || !mission_parse_get_arrival_ship(CTEXT(node)))
					{
						if (type == OPF_SHIP)
						{													// return invalid ship if not also looking for point
							return SEXP_CHECK_INVALID_SHIP;
						}

						if (find_matching_waypoint(CTEXT(node)) == NULL)
						{
							if (verify_vector(CTEXT(node)))					// verify return non-zero on invalid point
							{
								return SEXP_CHECK_INVALID_POINT;
							}
						}
					}
				}

				break;

			case OPF_WING:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (wing_name_lookup(CTEXT(node), 1) < 0){
					return SEXP_CHECK_INVALID_WING;
				}

				break;

			case OPF_SHIP_WING:
			case OPF_SHIP_WING_POINT:
			case OPF_SHIP_WING_TEAM:
			case OPF_SHIP_WING_POINT_OR_NONE:
			case OPF_ORDER_RECIPIENT:
				if ( type2 != SEXP_ATOM_STRING ){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (type == OPF_ORDER_RECIPIENT) {
					if (!strcmp ("<all fighters>", CTEXT(node))) {
						break;
					}
				}

				// none is okay for _or_none
				if (type == OPF_SHIP_WING_POINT_OR_NONE && !stricmp(CTEXT(node), SEXP_NONE_STRING))	{
					break;
				}

				// IFFs are fine for ship_wing_team
				if (type == OPF_SHIP_WING_TEAM && iff_lookup(CTEXT(node)) >= 0)	{
					break;
				}

				if ((ship_name_lookup(CTEXT(node), 1) < 0) && (wing_name_lookup(CTEXT(node), 1) < 0)) {
					if (Fred_running || !mission_parse_get_arrival_ship(CTEXT(node)))
					{
						if (type != OPF_SHIP_WING_POINT){									// return invalid if not also looking for point
							return SEXP_CHECK_INVALID_SHIP_WING;
						}

						if (find_matching_waypoint(CTEXT(node)) == NULL){
							if (verify_vector(CTEXT(node))){  // non-zero on verify vector mean invalid!
								if (sexp_determine_team(CTEXT(node)) < 0){
									return SEXP_CHECK_INVALID_POINT;
								}
							}
						}
					}
				}

				break;

			case OPF_AWACS_SUBSYSTEM:
			case OPF_ROTATING_SUBSYSTEM:
			case OPF_SUBSYSTEM:
			case OPF_SUBSYSTEM_OR_NONE:
			case OPF_SUBSYS_OR_GENERIC:
			{
				char *shipname;
				int shipnum,ship_class;
				int ship_index;				

				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				// none is okay for subsys_or_none
				if (type == OPF_SUBSYSTEM_OR_NONE && !stricmp(CTEXT(node), SEXP_NONE_STRING))
				{
					break;
				}

				//  subsys_or_generic has a few extra options it accepts
				if (type == OPF_SUBSYS_OR_GENERIC && (!(stricmp(CTEXT(node), SEXP_ALL_ENGINES_STRING)) || !(stricmp(CTEXT(node), SEXP_ALL_TURRETS_STRING)) )) {
					break;
				}

				// we must get the model of the ship that is part of this sexpression and find a subsystem
				// with that name.  This code assumes by default that the ship is *always* the first name
				// in the sexpression.  If this is ever not the case, the code here must be changed to
				// get the correct ship name.
				switch(get_operator_const(CTEXT(op_node)))
				{
					case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
					case OP_DISTANCE_SUBSYSTEM:
					case OP_SET_CARGO:
					case OP_IS_CARGO:
					case OP_CHANGE_AI_CLASS:
					case OP_IS_AI_CLASS:
					case OP_MISSILE_LOCKED:
					case OP_SHIP_SUBSYS_GUARDIAN_THRESHOLD:
						ship_index = CDR(CDR(op_node));
						break;

					case OP_BEAM_FIRE:
						if(argnum == 1){
							ship_index = CDR(op_node);
						} else {
							ship_index = CDR(CDR(CDR(op_node)));
						}
						break;

					case OP_QUERY_ORDERS:
						ship_index = CDR(CDR(CDR(CDR(op_node))));
						break;
	
					case OP_WEAPON_CREATE:
						ship_index = CDDDDDR(CDDDDR(op_node));
						break;

					default :
						ship_index = CDR(op_node);
						break;
				}

				shipname = CTEXT(ship_index);
				shipnum = ship_name_lookup(shipname, 1);
				if (shipnum >= 0)
				{
					ship_class = Ships[shipnum].ship_info_index;
				}
				else
				{
					// must try to find the ship in the arrival list
					p_object *p_objp = mission_parse_get_arrival_ship(shipname);

					if (!p_objp)
					{
						if (type == OPF_SUBSYSTEM_OR_NONE)
							break;
						else
							return SEXP_CHECK_INVALID_SHIP;
					}

					ship_class = p_objp->ship_class;
				}

				// check for the special "hull" value
				if ( (Operators[op].value == OP_SABOTAGE_SUBSYSTEM) || (Operators[op].value == OP_REPAIR_SUBSYSTEM) || (Operators[op].value == OP_SET_SUBSYSTEM_STRNGTH) || (Operators[op].value == OP_SET_ARMOR_TYPE) || (Operators[op].value == OP_BEAM_FIRE)) {
					if ( !stricmp( CTEXT(node), SEXP_HULL_STRING) || !stricmp( CTEXT(node), SEXP_SIM_HULL_STRING) ){
						break;
					}
				}
				// check for special "shields" value for armor types
				if (Operators[op].value == OP_SET_ARMOR_TYPE) {
					if ( !stricmp( CTEXT(node), SEXP_SHIELD_STRING) || !stricmp( CTEXT(node), SEXP_SIM_HULL_STRING) ){
						break;
					}
				}

				for (i=0; i<Ship_info[ship_class].n_subsystems; i++)
				{
					if (!subsystem_stricmp(Ship_info[ship_class].subsystems[i].subobj_name, CTEXT(node)))
					{
						break;
					}
				}

				if (i == Ship_info[ship_class].n_subsystems)
				{
					return SEXP_CHECK_INVALID_SUBSYS;
				}

				if(Fred_running)
				{
					// if we're checking for an AWACS subsystem and this is not an awacs subsystem
					if((type == OPF_AWACS_SUBSYSTEM) && !(Ship_info[ship_class].subsystems[i].flags & MSS_FLAG_AWACS))
					{
						return SEXP_CHECK_INVALID_SUBSYS;
					}

					// rotating subsystem, like above - Goober5000
					if ((type == OPF_ROTATING_SUBSYSTEM) && !(Ship_info[ship_class].subsystems[i].flags & MSS_FLAG_ROTATES))
					{
						return SEXP_CHECK_INVALID_SUBSYS;
					}
				}

				break;
			}

			case OPF_SUBSYSTEM_TYPE:
				for (i = 0; i < SUBSYSTEM_MAX; i++)
				{
					if (!stricmp(CTEXT(node), Subsystem_types[i]))
						break;
				}

				if (i == SUBSYSTEM_MAX)
					return SEXP_CHECK_INVALID_SUBSYS_TYPE;

				break;

			case OPF_POINT:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (find_matching_waypoint(CTEXT(node)) == NULL)
				{
					if (verify_vector(CTEXT(node)))
					{
						return SEXP_CHECK_INVALID_POINT;
					}
				}

				break;

			case OPF_IFF:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (iff_lookup(CTEXT(node)) < 0)
				{
					return SEXP_CHECK_INVALID_IFF;
				}

				break;

			case OPF_AI_CLASS:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				for (i=0; i<Num_ai_classes; i++)
				{
					if (!stricmp(Ai_class_names[i], CTEXT(node)))
					{
						break;
					}
				}

				if (i == Num_ai_classes)
				{
					return SEXP_CHECK_INVALID_AI_CLASS;
				}

				break;

			case OPF_ARRIVAL_LOCATION:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				for (i=0; i<MAX_ARRIVAL_NAMES; i++)
				{
					if (!stricmp(Arrival_location_names[i], CTEXT(node)))
					{
						break;
					}
				}

				if (i == MAX_ARRIVAL_NAMES)
				{
					return SEXP_CHECK_INVALID_ARRIVAL_LOCATION;
				}

				break;

			case OPF_DEPARTURE_LOCATION:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				for (i=0; i<MAX_DEPARTURE_NAMES; i++)
				{
					if (!stricmp(Departure_location_names[i], CTEXT(node)))
					{
						break;
					}
				}

				if (i == MAX_DEPARTURE_NAMES)
				{
					return SEXP_CHECK_INVALID_DEPARTURE_LOCATION;
				}

				break;

			case OPF_ARRIVAL_ANCHOR_ALL:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}
				else
				{
					int get_special_anchor(char *name);
					int valid = 0;

					// <any friendly>, etc.
					if (get_special_anchor(CTEXT(node)) >= 0)
					{
						valid = 1;
					}

					if (ship_name_lookup(CTEXT(node), 1) >= 0)
					{
						valid = 1;
					}

					if (!Fred_running && mission_parse_get_arrival_ship(CTEXT(node)))
					{
						valid = 1;
					}

					if (!valid)
					{
						return SEXP_CHECK_INVALID_ARRIVAL_ANCHOR_ALL;
					}
				}

				break;

			case OPF_SOUNDTRACK_NAME:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (!stricmp(CTEXT(node), "<No Music>"))
					break;

				if (Cmdline_freespace_no_music)
					break;

				for (i=0; i<Num_soundtracks; i++)
				{
					if (!stricmp(CTEXT(node), Soundtracks[i].name))
					{
						break;
					}
				}

				if (i == Num_soundtracks)
					return SEXP_CHECK_INVALID_SOUNDTRACK_NAME;

				break;

			case OPF_SHIP_WITH_BAY:
			{
				char *name = CTEXT(node);
				p_object *p_objp;
				int shipnum = -1;

				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				if (!stricmp(name, "<no anchor>"))
					break;

				shipnum = ship_name_lookup(name, 1);
				if (shipnum < 0)
				{
					if (Fred_running)
						return SEXP_CHECK_INVALID_SHIP;

					p_objp = mission_parse_get_arrival_ship(name);
					if (p_objp == NULL)
						return SEXP_CHECK_INVALID_SHIP;

					// Goober5000 - since we can't check POFs for ships which have yet to arrive
					// (not without a bit of work anyway), just assume they're okay
					break;
				}

				// ship exists at this point

				// now determine if this ship has a docking bay
				if (!ship_has_dock_bay(shipnum))
					return SEXP_CHECK_INVALID_SHIP_WITH_BAY;

				break;
			}

			case OPF_SUPPORT_SHIP_CLASS:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (!stricmp(CTEXT(node), "<species support ship class>"))
					break;

				if (!stricmp(CTEXT(node), "<any support ship class>"))
					break;

				for (i = 0; i < Num_ship_classes; i++ ) {
					if ( !stricmp(CTEXT(node), Ship_info[i].name) )
					{
						if (Ship_info[i].flags & SIF_SUPPORT)
						{
							break;
						}
					}
				}

				if ( i == Num_ship_classes )
					return SEXP_CHECK_INVALID_SUPPORT_SHIP_CLASS;

				break;

			case OPF_BOOL:
				if (type2 != OPR_BOOL){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				break;

			case OPF_AI_ORDER:
				if ( type2 != SEXP_ATOM_STRING ){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				break;

			case OPF_NULL:
				if (type2 != OPR_NULL){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				break;

			// Goober5000
			case OPF_FLEXIBLE_ARGUMENT:
				if (type2 != OPR_FLEXIBLE_ARGUMENT) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}
				break;

			// Goober5000
			case OPF_ANYTHING:
				break;

			case OPF_AI_GOAL:
				if (type2 != OPR_AI_GOAL){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (Fred_running) {
					int ship_num, ship2, w = 0;

					// if it's the "goals" operator, this is part of initial orders, so just assume it's okay
					if (get_operator_const(Sexp_nodes[op_node].text) == OP_GOALS_ID) {
						break;
					}

					ship_num = ship_name_lookup(CTEXT(Sexp_nodes[op_node].rest), 1);	// Goober5000 - include players
					if (ship_num < 0) {
						w = wing_name_lookup(CTEXT(Sexp_nodes[op_node].rest));
						if (w < 0) {
							if (bad_node){
								*bad_node = Sexp_nodes[op_node].rest;
							}

							return SEXP_CHECK_INVALID_SHIP;  // should have already been caught earlier, but just in case..
						}
					}

					Assert(Sexp_nodes[node].subtype == SEXP_ATOM_LIST);
					z = Sexp_nodes[node].first;
					Assert(Sexp_nodes[z].subtype != SEXP_ATOM_LIST);
					z = get_operator_const(CTEXT(z));
					if (ship_num >= 0) {
						if (!query_sexp_ai_goal_valid(z, ship_num)){
							return SEXP_CHECK_ORDER_NOT_ALLOWED;
						}

					} else {
						for (i=0; i<Wings[w].wave_count; i++){
							if (!query_sexp_ai_goal_valid(z, Wings[w].ship_index[i])){
								return SEXP_CHECK_ORDER_NOT_ALLOWED;
							}
						}
					}

					if ((z == OP_AI_DOCK) && (Sexp_nodes[node].rest >= 0)) {
						ship2 = ship_name_lookup(CTEXT(Sexp_nodes[node].rest), 1);	// Goober5000 - include players
						if ((ship_num < 0) || !ship_docking_valid(ship_num, ship2)){
							return SEXP_CHECK_DOCKING_NOT_ALLOWED;
						}
					}
				}

				// we should check the syntax of the actual goal!!!!
				z = Sexp_nodes[node].first;
				if ((z = check_sexp_syntax(z, OPR_AI_GOAL, recursive, bad_node)) != 0){
					return z;
				}

				break;

			case OPF_SHIP_TYPE:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				i = ship_type_name_lookup(CTEXT(node));

				if (i < 0){
					return SEXP_CHECK_INVALID_SHIP_TYPE;
				}

				break;

			case OPF_WAYPOINT_PATH:
				if (find_matching_waypoint_list(CTEXT(node)) == NULL) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}
				break;

			case OPF_MESSAGE:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				if (Fred_running) {
					for (i=0; i<Num_messages; i++)
						if (!stricmp(Messages[i].name, CTEXT(node)))
							break;

					if (i == Num_messages)
						return SEXP_CHECK_UNKNOWN_MESSAGE;
				}
				
				break;

			case OPF_PRIORITY: {
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				if (Fred_running) {  // should still check in Fred though..
					char *name;

					name = CTEXT(node);
					if (!stricmp(name, "low") || !stricmp(name, "normal") || !stricmp(name, "high"))
						break;

					return SEXP_CHECK_INVALID_PRIORITY;
				}

				break;
			}

			case OPF_MISSION_NAME:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				if (Fred_running) {
					if (mode == SEXP_MODE_CAMPAIGN) {
						for (i=0; i<Campaign.num_missions; i++)
							if (!stricmp(CTEXT(node), Campaign.missions[i].name)) {
								if ((i != Sexp_useful_number) && (Campaign.missions[i].level >= Campaign.missions[Sexp_useful_number].level))
									return SEXP_CHECK_INVALID_LEVEL;

								break;
							}

						if (i == Campaign.num_missions)
							return SEXP_CHECK_INVALID_MISSION_NAME;

					} else {
						// mwa -- put the following if statement to prevent Fred errors for possibly valid
						// conditions.  We should do something else here!!!
						if ( (Operators[op].value == OP_PREVIOUS_EVENT_TRUE) || (Operators[op].value == OP_PREVIOUS_EVENT_FALSE) || (Operators[op].value == OP_PREVIOUS_EVENT_INCOMPLETE)
							|| (Operators[op].value == OP_PREVIOUS_GOAL_TRUE) || (Operators[op].value == OP_PREVIOUS_GOAL_FALSE) || (Operators[op].value == OP_PREVIOUS_GOAL_INCOMPLETE) )
							break;

						if (!(*Mission_filename) || stricmp(Mission_filename, CTEXT(node)))
							return SEXP_CHECK_INVALID_MISSION_NAME;
					}
				}

				break;

			case OPF_GOAL_NAME:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				// we only need to check the campaign list if running in Fred and are in campaign mode.
				// otherwise, check the set of current goals
				if ( Fred_running && (mode == SEXP_MODE_CAMPAIGN) ) {
					z = find_parent_operator(node);
					Assert(z >= 0);
					z = Sexp_nodes[z].rest;  // first argument of operator should be mission name
					Assert(z >= 0);
					for (i=0; i<Campaign.num_missions; i++)
						if (!stricmp(CTEXT(z), Campaign.missions[i].name))
							break;

					// read the goal/event list from the mission file if both num_goals and num_events
					// are < 0
					if ((Campaign.missions[i].num_goals <= 0) && (Campaign.missions[i].num_events <= 0) )
						read_mission_goal_list(i);
					
					if (i < Campaign.num_missions) {
						for (t=0; t<Campaign.missions[i].num_goals; t++)
							if (!stricmp(CTEXT(node), Campaign.missions[i].goals[t].name))
								break;

						if (t == Campaign.missions[i].num_goals)
							return SEXP_CHECK_INVALID_GOAL_NAME;
					}
				} else {
					// MWA -- short circuit evaluation of these things for now.
					if ( (Operators[op].value == OP_PREVIOUS_GOAL_TRUE) || (Operators[op].value == OP_PREVIOUS_GOAL_FALSE) || (Operators[op].value == OP_PREVIOUS_GOAL_INCOMPLETE) )
						break;

					for (i=0; i<Num_goals; i++)
						if (!stricmp(CTEXT(node), Mission_goals[i].name))
							break;

					if (i == Num_goals)
						return SEXP_CHECK_INVALID_GOAL_NAME;
				}

				break;

			case OPF_EVENT_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				// like above checking for goals, check events in the campaign only if in Fred
				// and only if in campaign mode.  Otherwise, check the current set of events
				if ( Fred_running && (mode == SEXP_MODE_CAMPAIGN) ) {
					z = find_parent_operator(node);
					Assert(z >= 0);
					z = Sexp_nodes[z].rest;  // first argument of operator should be mission name
					Assert(z >= 0);
					for (i=0; i<Campaign.num_missions; i++)
						if (!stricmp(CTEXT(z), Campaign.missions[i].name))
							break;

					// read the goal/event list from the mission file if both num_goals and num_events
					// are < 0
					if ((Campaign.missions[i].num_goals <= 0) && (Campaign.missions[i].num_events <= 0) )
						read_mission_goal_list(i);
					
					if (i < Campaign.num_missions) {
						for (t=0; t<Campaign.missions[i].num_events; t++)
							if (!stricmp(CTEXT(node), Campaign.missions[i].events[t].name))
								break;

						if (t == Campaign.missions[i].num_events)
							return SEXP_CHECK_INVALID_EVENT_NAME;
					}
				} else {
					// MWA -- short circuit evaluation of these things for now.
					if ( (Operators[op].value == OP_PREVIOUS_EVENT_TRUE) || (Operators[op].value == OP_PREVIOUS_EVENT_FALSE) || (Operators[op].value == OP_PREVIOUS_EVENT_INCOMPLETE) )
						break;

					for ( i = 0; i < Num_mission_events; i++ ) {
						if ( !stricmp(CTEXT(node), Mission_events[i].name) )
							break;
					}
					if ( i == Num_mission_events )
						return SEXP_CHECK_INVALID_EVENT_NAME;
				}
				break;

			case OPF_DOCKER_POINT:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				// This makes massive assumptions about the structure of the SEXP using it. If you add any 
				// new SEXPs that use this OPF, you will probably need to edit this section to accommodate them.
				if (Fred_running) {
					int ship_num, model;

					// Look for the node containing the docker ship as its first argument. For set-docked, we want 
					// the current SEXP. Otherwise (for ai-dock), we want its parent.
					if (get_operator_const(Sexp_nodes[op_node].text) == OP_SET_DOCKED) {
						z = op_node;
					}
					else {
						z = find_parent_operator(op_node);
					
						// if it's the "goals" operator, this is part of initial orders, so just assume it's okay
						if (get_operator_const(Sexp_nodes[z].text) == OP_GOALS_ID) {
							break;
						}
					}

					// look for the ship this goal is being assigned to
					ship_num = ship_name_lookup(CTEXT(Sexp_nodes[z].rest), 1);
					if (ship_num < 0) {
						if (bad_node)
							*bad_node = Sexp_nodes[z].rest;

						return SEXP_CHECK_INVALID_SHIP;  // should have already been caught earlier, but just in case..
					}

					model = Ship_info[Ships[ship_num].ship_info_index].model_num;
					z = model_get_num_dock_points(model);
					for (i=0; i<z; i++)
						if (!stricmp(CTEXT(node), model_get_dock_name(model, i)))
							break;

					if (i == z)
						return SEXP_CHECK_INVALID_DOCKER_POINT;
				}

				break;

			case OPF_DOCKEE_POINT:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				// This makes massive assumptions about the structure of the SEXP using it. If you add any 
				// new SEXPs that use this OPF, you will probably need to edit this section to accommodate them.
				if (Fred_running) {
					int ship_num, model;

					// If we're using set-docked, we want to look up the ship from the third SEXP argument.
					if (get_operator_const(Sexp_nodes[op_node].text) == OP_SET_DOCKED) {
						//Navigate to the third argument
						z = op_node;
						for (i = 0; i < 3; i++)
							z = Sexp_nodes[z].rest;

						ship_num = ship_name_lookup(Sexp_nodes[z].text, 1);
					}
					else {
						ship_num = ship_name_lookup(CTEXT(Sexp_nodes[op_node].rest), 1);
					}

					if (ship_num < 0) {
						if (bad_node)
							*bad_node = Sexp_nodes[op_node].rest;

						return SEXP_CHECK_INVALID_SHIP;  // should have already been caught earlier, but just in case..
					}

					model = Ship_info[Ships[ship_num].ship_info_index].model_num;
					z = model_get_num_dock_points(model);
					for (i=0; i<z; i++)
						if (!stricmp(CTEXT(node), model_get_dock_name(model, i)))
							break;

					if (i == z)
						return SEXP_CHECK_INVALID_DOCKEE_POINT;
				}

				break;

			case OPF_WHO_FROM:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				if (*CTEXT(node) != '#') {  // not a manual source?
					if ( stricmp(CTEXT(node), "<any wingman>"))  
						if ( stricmp(CTEXT(node), "<none>") ) // not a special token?
							if ((ship_name_lookup(CTEXT(node), TRUE) < 0) && (wing_name_lookup(CTEXT(node), 1) < 0))  // is it in the mission?
								if (Fred_running || !mission_parse_get_arrival_ship(CTEXT(node)))
									return SEXP_CHECK_INVALID_MSG_SOURCE;
				}

				break;

			//Karajorma
			case OPF_PERSONA:
				if (type2 != SEXP_ATOM_STRING) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				for (i=0; i < Num_personas ; i++) {
					if (!strcmp(CTEXT(node), Personas[i].name)) {
						break;
					}
				}

				if (i == Num_personas) {
					return SEXP_CHECK_INVALID_PERSONA_NAME; 
				}
				break;

			case OPF_FONT:
				if (type2 != SEXP_ATOM_STRING) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				for (i = 0; i < Num_fonts; i++) {
					if (!stricmp(CTEXT(node), Fonts[i].filename)) {
						break;
					}
				}

				if (i == Num_fonts) {
					return SEXP_CHECK_INVALID_FONT;
				}
				break;
				
			case OPF_SOUND_ENVIRONMENT:
				if (type2 != SEXP_ATOM_STRING) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (stricmp(CTEXT(node), SEXP_NONE_STRING) && ds_eax_get_preset_id(CTEXT(node)) < 0) {
					return SEXP_CHECK_INVALID_SOUND_ENVIRONMENT;
				}
				break;

			case OPF_AUDIO_VOLUME_OPTION:
				if (type2 != SEXP_ATOM_STRING) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (audio_volume_option_lookup(CTEXT(node)) == -1)
					return SEXP_CHECK_INVALID_AUDIO_VOLUME_OPTION;
				break;

			case OPF_HUD_GAUGE:
				if (type2 != SEXP_ATOM_STRING) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (hud_gauge_type_lookup(CTEXT(node)) == -1)
					return SEXP_CHECK_INVALID_HUD_GAUGE;
				break;

			case OPF_SOUND_ENVIRONMENT_OPTION:
				if (type2 != SEXP_ATOM_STRING) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (sexp_sound_environment_option_lookup(CTEXT(node)) < 0) {
					return SEXP_CHECK_INVALID_SOUND_ENVIRONMENT_OPTION; 
				}
				break;

			case OPF_EXPLOSION_OPTION:
				if (type2 != SEXP_ATOM_STRING) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (sexp_explosion_option_lookup(CTEXT(node)) < 0) {
					return SEXP_CHECK_INVALID_EXPLOSION_OPTION; 
				}
				break;

			case OPF_KEYPRESS:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				break;

			case OPF_CARGO:
			case OPF_STRING:
			case OPF_MESSAGE_OR_STRING:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;
				break;

			case OPF_SKILL_LEVEL:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				for (i = 0; i < NUM_SKILL_LEVELS; i++) {
					if ( !stricmp(CTEXT(node), Skill_level_names(i, 0)) )
						break;
				}
				if ( i == NUM_SKILL_LEVELS )
					return SEXP_CHECK_INVALID_SKILL_LEVEL;
				break;

			case OPF_MEDAL_NAME:
				if ( type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				for (i = 0; i < Num_medals; i++) {
					if ( !stricmp(CTEXT(node), Medals[i].name) )
						break;
				}

				if ( i == Num_medals )
					return SEXP_CHECK_INVALID_MEDAL_NAME;
				break;

			case OPF_HUGE_WEAPON:
			case OPF_WEAPON_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				for (i = 0; i < Num_weapon_types; i++ ) {
					if ( !stricmp(CTEXT(node), Weapon_info[i].name) )
						break;
				}

				if ( i == Num_weapon_types )
					return SEXP_CHECK_INVALID_WEAPON_NAME;

				// we need to be sure that for huge weapons, the WIF_HUGE flag is set
				if ( type == OPF_HUGE_WEAPON ) {
					if ( !(Weapon_info[i].wi_flags & WIF_HUGE) )
						return SEXP_CHECK_INVALID_WEAPON_NAME;
				}

				break;

			// Goober5000
			case OPF_INTEL_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				for (i = 0; i < Intel_info_size; i++ ) {
					if ( !stricmp(CTEXT(node), Intel_info[i].name) )
						break;
				}

				if ( i == Intel_info_size )
					return SEXP_CHECK_INVALID_INTEL_NAME;
				
				break;

			case OPF_TURRET_TARGET_ORDER:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				for (i = 0; i < NUM_TURRET_ORDER_TYPES; i++ ) {
					if ( !stricmp(CTEXT(node), Turret_target_order_names[i]) )
						break;
				}

				if ( i == NUM_TURRET_ORDER_TYPES )
					return SEXP_CHECK_INVALID_TURRET_TARGET_ORDER;
				
				break;

			case OPF_ARMOR_TYPE:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				if (!stricmp(CTEXT(node), SEXP_NONE_STRING))
					break;

				for (st = 0; st < Armor_types.size(); st++ ) {
					if ( !stricmp(CTEXT(node), Armor_types[st].GetNamePtr()) )
						break;
				}

				if ( st == Armor_types.size() )
					return SEXP_CHECK_INVALID_ARMOR_TYPE;
				
				break;

			case OPF_DAMAGE_TYPE:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				if (!stricmp(CTEXT(node), SEXP_NONE_STRING))
					break;

				for (st = 0; st < Damage_types.size(); st++ ) {
					if ( !stricmp(CTEXT(node), Damage_types[st].name) )
						break;
				}

				if ( st == Armor_types.size() )
					return SEXP_CHECK_INVALID_DAMAGE_TYPE;
				
				break;

			case OPF_ANIMATION_TYPE:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				i = model_anim_match_type(CTEXT(node));
				if ( i == TRIGGER_TYPE_NONE )
					return SEXP_CHECK_INVALID_ANIMATION_TYPE;

				break;
	
			case OPF_TARGET_PRIORITIES:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;
	
				for(st = 0; st < Ai_tp_list.size(); st++) {
					if ( !stricmp(CTEXT(node), Ai_tp_list[st].name) )
						break;
				}

				if ( st == Ai_tp_list.size() )
					return SEXP_CHECK_INVALID_TARGET_PRIORITIES;
				
				break;
	
			case OPF_SHIP_CLASS_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				if (ship_info_lookup(CTEXT(node)) < 0)
					return SEXP_CHECK_INVALID_SHIP_CLASS_NAME;

				break;

			case OPF_HUD_GAUGE_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				for ( i = 0; i < NUM_HUD_GAUGES; i++ ) {
					if ( !stricmp(CTEXT(node), HUD_gauge_text[i]) )
						break;
				}

				// if we reached the end of the list, then the name is invalid
				if ( i == NUM_HUD_GAUGES )
					return SEXP_CHECK_INVALID_GAUGE_NAME;
				
				break;

			case OPF_SKYBOX_MODEL_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				if ( stricmp(CTEXT(node), NOX("default")) && !strstr(CTEXT(node), NOX(".pof")) )
					return SEXP_CHECK_INVALID_SKYBOX_NAME;

				break;

			case OPF_JUMP_NODE_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				if (jumpnode_get_by_name(CTEXT(node)) == NULL)
					return SEXP_CHECK_INVALID_JUMP_NODE;

				break;


			case OPF_VARIABLE_NAME:
				var_index = get_index_sexp_variable_from_node(node);
				if ( var_index  == -1) {
					return SEXP_CHECK_INVALID_VARIABLE;
				}

				// some SEXPs demand a number variable
				if ((argnum == 8 && Operators[op].value == OP_ADD_BACKGROUND_BITMAP) ||
					(argnum == 5 && Operators[op].value == OP_ADD_SUN_BITMAP))
				{
					if (!(Sexp_variables[var_index].type & SEXP_VARIABLE_NUMBER)) 
						return SEXP_CHECK_INVALID_VARIABLE_TYPE; 
				}
				// otherwise anything goes
				break;

			case OPF_AMBIGUOUS:
				// type checking for modify-variable
				// string or number -- anything goes
				break;						

			case OPF_BACKGROUND_BITMAP:
			case OPF_SUN_BITMAP:
			case OPF_NEBULA_STORM_TYPE:
			case OPF_NEBULA_POOF:
			case OPF_POST_EFFECT:
				if (type2 != SEXP_ATOM_STRING) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}
				break;

			case OPF_HUD_ELEMENT:
				if (type2 != SEXP_ATOM_STRING) {
					return SEXP_CHECK_TYPE_MISMATCH;
				} else {
					char *gauge = CTEXT(node);
					if ( strcmp(SEXP_HUD_GAUGE_WARPOUT, gauge) == 0 ) {
						break;
					}
				}
				return SEXP_CHECK_INVALID_HUD_ELEMENT;

			case OPF_WEAPON_BANK_NUMBER:
				if (type2 != SEXP_ATOM_STRING) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (!stricmp(CTEXT(node), SEXP_ALL_BANKS_STRING)) {
					break;
				}

				// if we haven't specified all banks we need to check the number of the bank is legal
				else {
					int num_banks = atoi(CTEXT(node));
					if ((num_banks >= MAX_SHIP_PRIMARY_BANKS) && (num_banks >= MAX_SHIP_SECONDARY_BANKS)) {
						return SEXP_CHECK_NUM_RANGE_INVALID;
					}
				}
				break;

			case OPF_SHIP_EFFECT:
				if (type2 != SEXP_ATOM_STRING) {
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (get_effect_from_name(CTEXT(node)) == -1 ) {
					return SEXP_CHECK_INVALID_SHIP_EFFECT;
				}
				break;
				

			default:
				Error(LOCATION, "Unhandled argument format");
		}

		node = Sexp_nodes[node].rest;
		argnum++;
	}

	return 0;
}

// Goober5000
void get_unformatted_sexp_variable_name(char *unformatted, char *formatted_pre)
{
	int end_index;
	char *formatted;

	// Goober5000 - trim @ if needed
	if (formatted_pre[0] == SEXP_VARIABLE_CHAR)
		formatted = formatted_pre+1;
	else
		formatted = formatted_pre;

	// get variable name (up to '['
	end_index = strcspn(formatted, "[");
	Assert( (end_index != 0) && (end_index < TOKEN_LENGTH-1) );
	strncpy(unformatted, formatted, end_index);
	unformatted[end_index] = '\0';
}

/**
 * Get text to stuff into Sexp_node in case of variable
 *
 * If Fred_running - stuff Sexp_variables[].variable_name
 * otherwise - stuff index into Sexp_variables array.
 */
void get_sexp_text_for_variable(char *text, char *token)
{
	int sexp_var_index;
	
	get_unformatted_sexp_variable_name(text, token);

	if ( !Fred_running ) {
		// freespace - get index into Sexp_variables array
		sexp_var_index = get_index_sexp_variable_name(text);
		Assert(sexp_var_index != -1);
		sprintf(text, "%d", sexp_var_index);
	}
}

// Goober5000
void do_preload_for_arguments(void (*preloader)(char *), int arg_node, int arg_handler_node)
{
	// we have a special argument
	if (!strcmp(Sexp_nodes[arg_node].text, SEXP_ARGUMENT_STRING))
	{
		int n;

		// we might not be handling it, either because this is not a *_of operator
		// or because we've disabled preloading for special arguments
		if (arg_handler_node < 0)
			return;

		// preload for each argument
		for (n = CDR(arg_handler_node); n != -1; n = CDR(n))
		{
			preloader(CTEXT(n));
		}
	}
	// we don't
	else
	{
		// preload for just the one argument
		preloader(CTEXT(arg_node));
	}
}

// Goober5000
void preload_change_ship_class(char *text)
{
	int idx;
	ship_info *sip;

	idx = ship_info_lookup(text);
	if (idx < 0)
		return;

	// preload the model, just in case there is no other ship of this class in the mission
	// (this eliminates the slight pause during a mission when changing to a previously unloaded model)
	sip = &Ship_info[idx];
	sip->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);

	if (sip->model_num >= 0)
		model_page_in_textures(sip->model_num, idx);
}

// Goober5000
void preload_turret_change_weapon(char *text)
{
	int idx;

	idx = weapon_info_lookup(text);
	if (idx < 0)
		return;

	weapon_mark_as_used(idx);
}

/**
 * Returns the first sexp index of data this function allocates. (start of this sexp)
 */
int get_sexp(char *token)
{
	int start, node, last, op, count;
	char variable_text[TOKEN_LENGTH];

	// start - the node allocated in first instance of fuction
	// node - the node allocated in current instance of function
	// count - number of nodes allocated this instance of function [do we set last.rest or .first]
	// variable - whether string or number is a variable referencing Sexp_variables

	// initialization
	start = last = -1;
	count = 0;

	ignore_white_space();
	while (*Mp != ')') {
		Assert(*Mp != EOF_CHAR);
		if (*Mp == '(') {
			// Sexp list
			Mp++;
			node = alloc_sexp("", SEXP_LIST, SEXP_ATOM_LIST, get_sexp(token), -1);

		} else if (*Mp == '\"')	{
			// Sexp string
			int len = strcspn(Mp + 1, "\"");
			
			Assert(Mp[len + 1] == '\"');    // hit EOF first (unterminated string)

			if(len >= TOKEN_LENGTH)
			{
				char * errortoken = new char[len+1];
				memset(errortoken, 0, len+1);
				strncpy(errortoken, Mp, len);
				char * message = new char[95 + len]; // 95 approximate fixed string length.
				memset(message, 0, 95 + len);
				sprintf(message, "Token '%s' is too long. Needs to be %d characters or shorter and will be truncated to fit.", errortoken, (TOKEN_LENGTH-1));
				MessageBox(NULL,message,NULL,MB_OK); // token is too long.
				delete errortoken;
				delete message;
				len = TOKEN_LENGTH;
			}

			// check if string variable
			if ( *(Mp + 1) == SEXP_VARIABLE_CHAR ) {

				// reduce length by 1 for end \"
				int length = len - 1;
				Assert(length < 2*TOKEN_LENGTH+2);

				// start copying after skipping 1st char
				strncpy(token, Mp + 2, length);
				token[length] = 0;

				get_sexp_text_for_variable(variable_text, token);
				node = alloc_sexp(variable_text, (SEXP_ATOM | SEXP_FLAG_VARIABLE), SEXP_ATOM_STRING, -1, -1);
			} else {
				strncpy(token, Mp + 1, len);
				token[len] = 0;
				node = alloc_sexp(token, SEXP_ATOM, SEXP_ATOM_STRING, -1, -1);
			}

			// bump past closing \" by 1 char
			Mp += len + 2;

		} else {
			// Sexp operator or number
			int len = 0;
			bool variable = false;
			while (*Mp != ')' && !is_white_space(*Mp)) {
				if ( (len == 0) && (*Mp == SEXP_VARIABLE_CHAR) ) {
					variable = true;
					Mp++;
					continue;
				}
				Assert(*Mp != EOF_CHAR);
				Assert(len < TOKEN_LENGTH - 1);
				token[len++] = *Mp++;
			}
			token[len] = 0;

			// maybe replace deprecated names
			if (!stricmp(token, "set-ship-position"))
				strcpy(token, "set-object-position");
			else if (!stricmp(token, "set-ship-facing"))
				strcpy(token, "set-object-facing");
			else if (!stricmp(token, "set-ship-facing-object"))
				strcpy(token, "set-object-facing-object");
			else if (!stricmp(token, "ai-chase-any-except"))
				strcpy(token, "ai-chase-any");
			else if (!stricmp(token, "change-ship-model"))
				strcpy(token, "change-ship-class");
			else if (!stricmp(token, "radar-set-max-range"))
				strcpy(token, "hud-set-max-targeting-range");
			else if (!stricmp(token, "ship-subsys-vanished"))
				strcpy(token, "ship-subsys-vanish");
			else if (!stricmp(token, "directive-is-variable"))
				strcpy(token, "directive-value");

			op = get_operator_index(token);
			if (op != -1) {
				node = alloc_sexp(token, SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
			} else {
				if ( variable ) {
					// convert token text for variable
					get_sexp_text_for_variable(variable_text, token);

					node = alloc_sexp(variable_text, (SEXP_ATOM | SEXP_FLAG_VARIABLE), SEXP_ATOM_NUMBER, -1, -1);
				} else {
					node = alloc_sexp(token, SEXP_ATOM, SEXP_ATOM_NUMBER, -1, -1);
				}
			}
		}

		// update links
		if (count++) {
			Assert(last != -1);
			Sexp_nodes[last].rest = node;
		} else {
			start = node;
		}

		Assert(node != -1);  // ran out of nodes.  Time to raise the MAX!
		last = node;
		ignore_white_space();
	}

	Mp++;  // skip past the ')'

	
	// Goober5000 - backwards compatibility for removed ai-chase-any-except
	if (get_operator_const(CTEXT(start)) == OP_AI_CHASE_ANY)
	{
		// if there is more than one argument, free the extras
		int n = CDR(CDR(start));
		if (n >= 0)
		{
			// free the entire rest of the argument list
			free_sexp2(n);	
		}
	}

	// Goober5000 - preload stuff for certain sexps
	if (!Fred_running)
	{
		int n, parent, arg_handler = -1;

		// see if we're using special arguments
		parent = find_parent_operator(start);
		if (parent >= 0 && is_blank_argument_op(get_operator_const(CTEXT(parent))))
		{
			// get the first op of the parent, which should be a *_of operator
			arg_handler = CADR(parent);
			if (!is_blank_of_op(get_operator_const(CTEXT(arg_handler))))
				arg_handler = -1;
		}

		// DISABLE ARGUMENT PRELOADING FOR NOW
		// see Mantis #925 for discussion
		// Also, the preloader will have to be moved to a different function (after the parsing is finished)
		// in order to work properly with special arguments.  The current node is an orphan until get_sexp
		// completes, at which time it will be linked into the sexp node list; this means that it is
		// impossible to get the parent node.
		arg_handler = -1;

		// preload according to sexp
		op = get_operator_const(CTEXT(start));
		switch (op)
		{
			case OP_CHANGE_SHIP_CLASS:
				// model is argument #1
				n = CDR(start);
				do_preload_for_arguments(preload_change_ship_class, n, arg_handler);
				break;

			case OP_SET_SPECIAL_WARPOUT_NAME:
				// set flag for taylor
				Knossos_warp_ani_used = 1;
				break;

			case OP_MISSION_SET_NEBULA:
				// set flag for WMC
				Nebula_sexp_used = true;
				Dynamic_environment = true;
				break;
				
			case OP_MISSION_SET_SUBSPACE:
				Dynamic_environment = true;
				break;

			case OP_WARP_EFFECT:
				// type of warp is argument #11
				n = CDDDDDR(start);
				n = CDDDDDR(n);
				n = CDR(n);

				// set flag for taylor
				if (CAR(n) != -1 || !strcmp(Sexp_nodes[n].text, SEXP_ARGUMENT_STRING))	// if it's evaluating a sexp or a special argument
					Knossos_warp_ani_used = 1;												// set flag just in case
				else if (atoi(CTEXT(n)) == 1)											// if it's the Knossos type
					Knossos_warp_ani_used = 1;												// set flag for sure
				break;

			case OP_SET_SKYBOX_MODEL:
				// model is argument #1
				n = CDR(start);
				do_preload_for_arguments(sexp_set_skybox_model_preload, n, arg_handler);
				Dynamic_environment = true;
				break;

			case OP_TURRET_CHANGE_WEAPON:
				// weapon to change to is arg #3
				n = CDDDR(start);
				do_preload_for_arguments(preload_turret_change_weapon, n, arg_handler);
				break;

			case OP_ADD_SUN_BITMAP:
				n = CDR(start);
				do_preload_for_arguments(stars_preload_sun_bitmap, n, arg_handler);
				Dynamic_environment = true;
				break;

			case OP_ADD_BACKGROUND_BITMAP:
				n = CDR(start);
				do_preload_for_arguments(stars_preload_background_bitmap, n, arg_handler);
				Dynamic_environment = true;
				break;
		}
	}

	return start;
}


/**
 * Stuffs a list of sexp variables
 */
int stuff_sexp_variable_list()
{	
	int count;
	char var_name[TOKEN_LENGTH];
	char default_value[TOKEN_LENGTH];
	char str_type[TOKEN_LENGTH];
	char persistent[TOKEN_LENGTH];
	char network[TOKEN_LENGTH];
	int index;
	int type;

	count = 0;
	required_string("$Variables:");
	ignore_white_space();

	// check for start of list
	if (*Mp != '(') {
		error_display(1, "Reading sexp variable list.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 6);
	}

	Mp++;
	ignore_white_space();

	while (*Mp != ')') {
		Assert(count < MAX_SEXP_VARIABLES);

		// get index - for debug
		stuff_int(&index);
		ignore_gray_space();

		// get var_name
		get_string(var_name);
		ignore_gray_space();

		// get default_value;
		get_string(default_value);
		ignore_gray_space();

		// get type
		get_string(str_type);
		ignore_white_space();

		// determine type
		if (!stricmp(str_type, "number")) {
			type = SEXP_VARIABLE_NUMBER;
		} else if (!stricmp(str_type, "string")) {
			type = SEXP_VARIABLE_STRING;
		} else if (!stricmp(str_type, "block")) {
			// Goober5000 - This looks dangerous... these flags are needed for certain things, but it
			// looks like BLOCK_*_SIZE is the only thing that keeps a block from running off the end
			// of its boundary.
			type = SEXP_VARIABLE_BLOCK;
		} else {
			type = SEXP_VARIABLE_UNKNOWN;
			Error(LOCATION, "SEXP variable '%s' is an unknown type!", var_name);
		}

		// possibly get network-variable
		if (check_for_string("\"network-variable\"")) {
			// eat it
			get_string(network);
			ignore_white_space();

			// set type
			type |= SEXP_VARIABLE_NETWORK;
		}

		// possibly get player-persistent
		if (check_for_string("\"player-persistent\"")) {
			// eat it
			get_string(persistent);
			ignore_white_space();

			// set type
			type |= SEXP_VARIABLE_PLAYER_PERSISTENT;
		// possibly get campaign-persistent
		} else if (check_for_string("\"campaign-persistent\"")) {
			// eat it
			get_string(persistent);
			ignore_white_space();

			// set type
			type |= SEXP_VARIABLE_CAMPAIGN_PERSISTENT;
		// trap error
		} else if (check_for_string("\"")) {
			// eat garbage
			get_string(persistent);
			ignore_white_space();

			// notify of error
			Error(LOCATION, "Error parsing sexp variables - unknown persistence type encountered.  You can continue from here without trouble.");
		}

		// check if variable name already exists
		if ( (type & SEXP_VARIABLE_NUMBER) || (type & SEXP_VARIABLE_STRING) ) {
			Assert(get_index_sexp_variable_name(var_name) == -1);
		}

		if ( type & SEXP_VARIABLE_BLOCK ) {
			add_block_variable(default_value, var_name, type, index);
		}
		else {			
			count++;
			sexp_add_variable(default_value, var_name, type, index);
		}
	}

	Mp++;

	return count;
}

bool has_special_explosion_block_index(ship *shipp, int *index)
{
	int current_index;

	for ( current_index = (MAX_SEXP_VARIABLES - BLOCK_EXP_SIZE) ; current_index >= (MAX_SEXP_VARIABLES - (BLOCK_EXP_SIZE * Num_special_expl_blocks)) ; current_index = (current_index - BLOCK_EXP_SIZE) ) {
		if ( 
			(atoi(Block_variables[current_index+INNER_RAD].text) == shipp->special_exp_inner) &&
			(atoi(Block_variables[current_index+OUTER_RAD].text) == shipp->special_exp_outer) &&
			(atoi(Block_variables[current_index+DAMAGE].text) == shipp->special_exp_damage) &&
			(atoi(Block_variables[current_index+BLAST].text) == shipp->special_exp_blast) &&
			(atoi(Block_variables[current_index+PROPAGATE].text) == (shipp->use_shockwave ? 1:0) ) &&
			(atoi(Block_variables[current_index+SHOCK_SPEED].text) == shipp->special_exp_shockwave_speed)
			) {
				*index = current_index;
				return true;
		}
	}	

	// if we got here, this ship's special explosions aren't represented in the Block_variables array
	*index = current_index;
	return false;
}

bool generate_special_explosion_block_variables()
{
	ship *shipp; 
	ship_obj *sop;
	int current_index;
	bool already_added = false; 
	int num_sexp_variables;
	int i;

	// since we're (re)generating the block variable index, we don't start off with any block variables 
	Num_special_expl_blocks = 0;

	// get the number of sexp_variables we currently have. We must not try to add a block variable with an index below this.
	num_sexp_variables = sexp_variable_count();

	for ( sop = GET_FIRST(&Ship_obj_list); sop != END_OF_LIST(&Ship_obj_list); sop = GET_NEXT(sop) ) {
		shipp=&Ships[Objects[sop->objnum].instance];

		if (!(shipp->use_special_explosion)) {
			continue;
		}

		already_added = has_special_explosion_block_index(shipp, &current_index);

		// if we can't add an index for this add this ship to the list of those which failed
		if (current_index < num_sexp_variables) {
			// fail list code goes here
			continue;
		}

		//if we haven't added this entry already, do so
		if (!already_added) {
			sprintf(Block_variables[current_index+INNER_RAD].text, "%d", shipp->special_exp_inner);
			sprintf(Block_variables[current_index+OUTER_RAD].text, "%d", shipp->special_exp_outer);
			sprintf(Block_variables[current_index+DAMAGE].text, "%d", shipp->special_exp_damage);
			sprintf(Block_variables[current_index+BLAST].text, "%d", shipp->special_exp_blast);
			sprintf(Block_variables[current_index+PROPAGATE].text, "%d", (shipp->use_shockwave ? 1:0) );
			sprintf(Block_variables[current_index+SHOCK_SPEED].text, "%d", shipp->special_exp_shockwave_speed);

			// add the names
			for (i = current_index; i < (current_index + BLOCK_EXP_SIZE); i++ ) {
				sprintf(Block_variables[i].variable_name, "%s", shipp->ship_name); 
			}

			Num_special_expl_blocks++;
		}			
	}

	return true;
}

int num_block_variables()
{
	return Num_special_expl_blocks * BLOCK_EXP_SIZE;
}

/**
 * Stuff SEXP text string
 */
void stuff_sexp_text_string(SCP_string &dest, int node, int mode)
{
	Assert( (node >= 0) && (node < Num_sexp_nodes) );

	if (Sexp_nodes[node].type & SEXP_FLAG_VARIABLE) {

		int sexp_variables_index = get_index_sexp_variable_name(Sexp_nodes[node].text);
		Assertion(sexp_variables_index != -1, "Couldn't find variable: %s\n", Sexp_nodes[node].text);
		Assert( (Sexp_variables[sexp_variables_index].type & SEXP_VARIABLE_NUMBER) || (Sexp_variables[sexp_variables_index].type & SEXP_VARIABLE_STRING) );

		// number
		if (Sexp_nodes[node].subtype == SEXP_ATOM_NUMBER) {
			Assert(Sexp_variables[sexp_variables_index].type & SEXP_VARIABLE_NUMBER);
		
			// Error check - can be Fred or FreeSpace
			if (mode == SEXP_ERROR_CHECK_MODE) {
				if ( Fred_running ) {
					sprintf(dest, "%s[%s] ", Sexp_nodes[node].text, Sexp_variables[sexp_variables_index].text);
				} else {
					sprintf(dest, "%s[%s] ", Sexp_variables[sexp_variables_index].variable_name, Sexp_variables[sexp_variables_index].text);
				}
			} else {
				// Save as string - only  Fred
				Assert(mode == SEXP_SAVE_MODE);
				sprintf(dest, "@%s[%s] ", Sexp_nodes[node].text, Sexp_variables[sexp_variables_index].text);
			}
		} else {
			// string
			Assert(Sexp_nodes[node].subtype == SEXP_ATOM_STRING);
			Assert(Sexp_variables[sexp_variables_index].type & SEXP_VARIABLE_STRING);

			// Error check - can be Fred or FreeSpace
			if (mode == SEXP_ERROR_CHECK_MODE) {
				if ( Fred_running ) {
					sprintf(dest, "%s[%s] ", Sexp_variables[sexp_variables_index].variable_name, Sexp_variables[sexp_variables_index].text);
				} else {
					sprintf(dest, "%s[%s] ", Sexp_nodes[node].text, Sexp_variables[sexp_variables_index].text);
				}
			} else {
				// Save as string - only Fred
				Assert(mode == SEXP_SAVE_MODE);
				sprintf(dest, "\"@%s[%s]\" ", Sexp_nodes[node].text, Sexp_variables[sexp_variables_index].text);
			}
		}
	} else {
		// not a variable
		if (Sexp_nodes[node].subtype == SEXP_ATOM_STRING) {
			sprintf(dest, "\"%s\" ", CTEXT(node));
		} else {
			sprintf(dest, "%s ", CTEXT(node));
		}
	}
}

int build_sexp_string(SCP_string &accumulator, int cur_node, int level, int mode)
{
	SCP_string buf;
	int node, old_length = accumulator.length();

	accumulator += "( ";
	node = cur_node;
	while (node != -1) {
		Assert(node >= 0 && node < Num_sexp_nodes);
		if (Sexp_nodes[node].first == -1) {
			// build text to string
			stuff_sexp_text_string(buf, node, mode);
			accumulator += buf;

		} else {
			build_sexp_string(accumulator, Sexp_nodes[node].first, level + 1, mode);
		}

		node = Sexp_nodes[node].rest;
	}

	accumulator += ") ";
	if ((accumulator.length() - old_length) > 40) {
		accumulator.resize(old_length);
		build_extended_sexp_string(accumulator, cur_node, level, mode);
		return 1;
	}

	return 0;
}

void build_extended_sexp_string(SCP_string &accumulator, int cur_node, int level, int mode)
{
	SCP_string buf;
	int i, flag = 0, node;

	accumulator += "( ";
	node = cur_node;
	while (node != -1) {
		// not the first line?
		if (flag) {
			for (i=0; i<level + 1; i++)
				accumulator += "   ";
		}

		flag = 1;
		Assert(node >= 0 && node < Num_sexp_nodes);
		if (Sexp_nodes[node].first == -1) {
			stuff_sexp_text_string(buf, node, mode);
			accumulator += buf;

		} else {
			build_sexp_string(accumulator, Sexp_nodes[node].first, level + 1, mode);
		}

		accumulator += "\n";
		node = Sexp_nodes[node].rest;
	}

	for (i=0; i<level; i++)
		accumulator += "   ";

	accumulator += ")";
}

void convert_sexp_to_string(SCP_string &dest, int cur_node, int mode)
{
	if (cur_node >= 0) {
		dest = "";
		build_sexp_string(dest, cur_node, 0, mode);
	} else {
		dest = "( )";
	}
}


// ----------------------------------------------------------------------------------- 
// Helper methods for getting data from nodes. Cause it's stupid to keep re-rolling this stuff for every single SEXP
// -----------------------------------------------------------------------------------

/**
 * Takes a SEXP node which contains the name of a ship and returns the player for that ship or NULL if it is an AI ship
 */
player * get_player_from_ship_node(int node, bool test_respawns)
{
	int sindex, np_index = -1;	
	p_object *p_objp;
	
	Assert (node != -1);

	sindex = ship_name_lookup(CTEXT(node));

	// singleplayer
	if (!(Game_mode & GM_MULTIPLAYER)){	
		if(sindex >= 0){
			if(Player_obj == &Objects[Ships[sindex].objnum]){
				return Player;
			}
		}
		return NULL; 
	}
	// multiplayer
	else {
		if(sindex >= 0){
			if(Ships[sindex].objnum >= 0) {
				// try and find the player
				np_index = multi_find_player_by_object(&Objects[Ships[sindex].objnum]);
			}
		}
		if (test_respawns && np_index < 0) {
			// Respawning ships don't have an objnum so we need to take a different approach 
			p_objp = mission_parse_get_arrival_ship(CTEXT(node));
			if (p_objp != NULL) {
				np_index = multi_find_player_by_parse_object(p_objp);
			}
		}

		if((np_index >= 0) && (np_index < MAX_PLAYERS)){
			return Net_players[np_index].m_player; 
		}

		return NULL; 
	}
}

/**
 * Given a node, returns a pointer to the ship or NULL if this isn't the name of a ship
 */
ship * sexp_get_ship_from_node(int node)
{
	int sindex;
	ship *shipp = NULL;

	sindex = ship_name_lookup( CTEXT(node) );

	if (sindex < 0) {
		return shipp;
	}

	if (Ships[sindex].objnum < 0) {
		return shipp;
	}

	shipp = &Ships[sindex]; 
	return shipp;
}

/**
 * Determine if the named ship or wing hasn't arrived yet (wing or ship must be on arrival list)
 */
int sexp_query_has_yet_to_arrive(char *name)
{
	int i;

	if (ship_query_state(name) < 0)
		return 1;

	i = wing_name_lookup(name, 1);

	// has not arrived yet, and never will arrive
	if ((i >= 0) && (Wings[i].num_waves >= 0) && (Wings[i].flags & WF_NEVER_EXISTED)){
		return 1;
	}

	// has not arrived yet
	if ((i >= 0) && (Wings[i].num_waves >= 0) && !Wings[i].total_arrived_count){
		return 1;
	}

	return 0;
}

// arithmetic functions
int add_sexps(int n)
{
	int	sum = 0, val;

	if (n != -1) {
		if ( CAR(n) != -1) {
			sum = eval_sexp( CAR(n) );
			// be sure to check for the NAN value when doing arithmetic -- this value should
			// get propagated to the next highest function.
			if ( Sexp_nodes[CAR(n)].value == SEXP_NAN )
				return SEXP_NAN;
			else if ( Sexp_nodes[CAR(n)].value == SEXP_NAN_FOREVER )
				return SEXP_NAN_FOREVER;
		}
		else
			sum = atoi( CTEXT(n) );

		while (CDR(n) != -1) {
			val = eval_sexp( CDR(n) );
			// be sure to check for the NAN value when doing arithmetic -- this value should
			// get propagated to the next highest function.
			if ( Sexp_nodes[CDR(n)].value == SEXP_NAN )
				return SEXP_NAN;
			else if ( Sexp_nodes[CDR(n)].value == SEXP_NAN_FOREVER )
				return SEXP_NAN_FOREVER;
			sum += val;
			n = CDR(n);
		}
	}

	return sum;
}

int sub_sexps(int n)
{
	int	sum = 0;

	if (n != -1) { 
		if (Sexp_nodes[n].first != -1)
			sum = eval_sexp(CAR(n));
		else
			sum = atoi(CTEXT(n));

		while (CDR(n) != -1) {
			sum -= eval_sexp(CDR(n));
			n = CDR(n);
		}
	}

	return sum;
}

int mul_sexps(int n)
{
	int	sum = 0;

	if (n != -1) {
		if (Sexp_nodes[n].first != -1)
			sum = eval_sexp(Sexp_nodes[n].first);
		else
			sum = atoi(CTEXT(n));

		while (Sexp_nodes[n].rest != -1) {
			sum *= eval_sexp(Sexp_nodes[n].rest);
			n = Sexp_nodes[n].rest;
		}
	}

	return sum;
}

int div_sexps(int n)
{
	int	sum = 0;

	if (n != -1) {
		if (Sexp_nodes[n].first != -1)
			sum = eval_sexp(Sexp_nodes[n].first);
		else
			sum = atoi(CTEXT(n));

		while (Sexp_nodes[n].rest != -1) {
			int div = eval_sexp(Sexp_nodes[n].rest);
			n = Sexp_nodes[n].rest;
			if (div == 0) {
				Warning(LOCATION, "Division by zero in sexp. Please check all uses of the / operator for possible causes.\n");
				Int3();
				continue;
			} 
			sum /= div;
		}
	}

	return sum;
}

int mod_sexps(int n)
{
	int	sum = 0;

	if (n != -1) {
		if (Sexp_nodes[n].first != -1)
			sum = eval_sexp(Sexp_nodes[n].first);
		else
			sum = atoi(CTEXT(n));

		while (Sexp_nodes[n].rest != -1) {
			sum = sum % eval_sexp(Sexp_nodes[n].rest);
			n = Sexp_nodes[n].rest;
		}
	}

	return sum;
}

int rand_internal(int low, int high, int seed = 0)
{
	int diff;

	// maybe seed it
	if (seed > 0)
		srand(seed);

	// get diff - don't allow negative or zero
	diff = high - low;
	if (diff < 0)
		diff = 0;

	return (low + rand32() % (diff + 1));
}

// Goober5000
int abs_sexp(int n)
{
	return abs(eval_num(n));
}

// Goober5000
int min_sexp(int n)
{
	int temp, min_val = INT_MAX;

	while (n != -1)
	{
		temp = eval_num(n);

		if (temp < min_val)
			min_val = temp;

		n = CDR(n);
	}

	return min_val;
}

// Goober5000
int max_sexp(int n)
{
	int temp, max_val = INT_MIN;

	while (n != -1)
	{
		temp = eval_num(n);

		if (temp > max_val)
			max_val = temp;

		n = CDR(n);
	}

	return max_val;
}

// Goober5000
int avg_sexp(int n)
{
	int num = 0, avg_val = 0;

	while (n != -1)
	{
		avg_val += eval_num(n);
		num++;

		n = CDR(n);
	}

	return (int) floor(((double) avg_val / num) + 0.5);
}

// Goober5000
int pow_sexp(int node)
{
	int num_1 = eval_num(node);
	int num_2 = eval_num(CDR(node));

	// this is disallowed in FRED, but can still happen through careless arithmetic
	if (num_2 < 0)
	{
		Warning(LOCATION, "Power function pow(%d, %d) attempted to raise to a negative power!", num_1, num_2);
		return 0;
	}

	double pow_result = pow(static_cast<double>(num_1), num_2);

	if (pow_result > static_cast<double>(INT_MAX))
	{
		nprintf(("SEXP", "Power function pow(%d, %d) is greater than INT_MAX!  Returning INT_MAX.", num_1, num_2));
		return INT_MAX;
	}
	else if (pow_result < static_cast<double>(INT_MIN))
	{
		nprintf(("SEXP", "Power function pow(%d, %d) is less than INT_MIN!  Returning INT_MIN.", num_1, num_2));
		return INT_MIN;
	}

	return static_cast<int>(pow_result);
}

// Goober5000
int signum_sexp(int node)
{
	int num = eval_num(node);

	if (num == 0)
		return 0;

	if (num < 0)
		return -1;

	// hurr durr math
	Assert(num > 0);
	return 1;
}

// Goober5000
int sexp_set_bit(int node, bool set_it)
{
	int val = eval_num(node);
	int bit_index = eval_num(CDR(node));

	if (bit_index < 0 || bit_index > 31)
	{
		Warning(LOCATION, "Bit index %d out of range!  Must be between 0 and 31.", bit_index);
		return SEXP_NAN;
	}

	if (set_it)
		return val | (1<<bit_index);
	else
		return val & ~(1<<bit_index);
}

// Goober5000
int sexp_is_bit_set(int node)
{
	int val = eval_num(node);
	int bit_index = eval_num(CDR(node));

	if (bit_index < 0 || bit_index > 31)
	{
		Warning(LOCATION, "Bit index %d out of range!  Must be between 0 and 31.", bit_index);
		return SEXP_NAN;
	}

	if (val & (1<<bit_index))
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

// Goober5000
int sexp_bitwise_and(int node)
{
	int val = eval_num(node);

	for (int n = CDR(node); n != -1; n = CDR(n))
		val &= eval_num(n);

	return val;
}

// Goober5000
int sexp_bitwise_or(int node)
{
	int val = eval_num(node);

	for (int n = CDR(node); n != -1; n = CDR(n))
		val |= eval_num(n);

	return val;
}

// Goober5000
int sexp_bitwise_not(int node)
{
	int result = ~(eval_num(node));

	// clear the sign bit
	return result & INT_MAX;
}

// Goober5000
int sexp_bitwise_xor(int node)
{
	return eval_num(node) ^ eval_num(CDR(node));
}

// seeding added by Karajorma and Goober5000
int rand_sexp(int n, bool multiple)
{
	int low, high, rand_num, seed;

	if (n < 0)
	{
		Int3();
		return 0;
	}

	// when getting a saved value
	if (Sexp_nodes[n].value == SEXP_NUM_EVAL)
	{
		// don't regenerate new random number
		return atoi(CTEXT(n));
	}

	low = eval_num(n);

	// get high
	high = eval_num(CDR(n));
			
	// is there a seed provided?
	if (CDDR(n) != -1)
		seed = eval_num(CDDR(n));
	else
		seed = 0;

	// get the random number
	rand_num = rand_internal(low, high, seed);

	// when saving the value
	if (!multiple)
	{
		// set .value and .text so random number is generated only once.
		Sexp_nodes[n].value = SEXP_NUM_EVAL;
		sprintf(Sexp_nodes[n].text, "%d", rand_num);
	}
	// if this is multiple with a nonzero seed provided
	else if (seed > 0)
	{
		// Set the seed to a new seeded random value. This will ensure that the next time the method
		// is called it will return a predictable but different number from the previous time. 
		sprintf(Sexp_nodes[CDDR(n)].text, "%d", rand_internal(1, INT_MAX, seed));
	}

	return rand_num;
}

// boolean evaluation functions.  Evaluate all sexpressions in the 'or' operator.  Needed to mark
// entries in the mission log as essential so when pruning the log, we know which entries we might
// need to keep.
int sexp_or(int n)
{
	int all_false, result;

	all_false = 1;
	result = 0;
	if (n != -1)
	{
		if (CAR(n) != -1)
		{
			result |= is_sexp_true(CAR(n));
			if ( Sexp_nodes[CAR(n)].value == SEXP_KNOWN_TRUE )
				return SEXP_KNOWN_TRUE;								// if one of the OR clauses is TRUE, whole clause is true
			if ( Sexp_nodes[CAR(n)].value != SEXP_KNOWN_FALSE )		// if the value is still unknown, they all can't be false
				all_false = 0;
		}
		else
			result |= atoi(CTEXT(n));

		while (CDR(n) != -1)
		{
			result |= is_sexp_true(CDR(n));
			if ( Sexp_nodes[CDR(n)].value == SEXP_KNOWN_TRUE )
				return SEXP_KNOWN_TRUE;								// if one of the OR clauses is TRUE, whole clause is true
			if ( Sexp_nodes[CDR(n)].value != SEXP_KNOWN_FALSE )		// if the value is still unknown, they all can't be false
				all_false = 0;

			n = CDR(n);
		}
	}

	if (all_false)
		return SEXP_KNOWN_FALSE;

	return result;
}

// this function does the 'and' operator.  It will short circuit evaluation  *but* it will still
// evaluate other members of the and construct.  I do this because I need events in the mission log
// to get marked as essential for goal purposes, and evaluation is pretty much the only way
int sexp_and(int n)
{
	int all_true, result;

	result = -1;
	all_true = 1;
	if (n != -1)
	{
		if (CAR(n) != -1)
		{
			result &= is_sexp_true(CAR(n));
			if ( Sexp_nodes[CAR(n)].value == SEXP_KNOWN_FALSE )
				return SEXP_KNOWN_FALSE;							// if one of the AND clauses is FALSE, whole clause is false
			if ( Sexp_nodes[CAR(n)].value != SEXP_KNOWN_TRUE )		// if the value is still unknown, they all can't be true
				all_true = 0;
		}
		else
			result &= atoi(CTEXT(n));

		while (CDR(n) != -1)
		{
			int new_result;

			new_result = is_sexp_true(CDR(n));
			result &= new_result;
			if ( Sexp_nodes[CDR(n)].value == SEXP_KNOWN_FALSE )
				return SEXP_KNOWN_FALSE;							// if one of the AND clauses is FALSE, whole clause is false
			if ( Sexp_nodes[CDR(n)].value != SEXP_KNOWN_TRUE )		// if the value is still unknown, they all can't be true
				all_true = 0;

			n = CDR(n);
		}
	}

	if (all_true)
		return SEXP_KNOWN_TRUE;

	return result;
}

// this version of the 'and' operator determines whether or not it's arguments become true
// in the order in which they are specified in the when statement.  Should be a simple matter of 
// seeing if anything evaluates to true later than something that evalueated to false
int sexp_and_in_sequence(int n)
{
	int result = -1;
	int all_true;

	all_true = 1;											// represents whether or not all nodes we have seen so far are true
	if (n != -1)
	{
		if (CAR(n) != -1)
		{
			result &= is_sexp_true(CAR(n));
			if ( Sexp_nodes[CAR(n)].value == SEXP_KNOWN_FALSE )
				return SEXP_KNOWN_FALSE;														// if one of the AND clauses is FALSE, whole clause is false
			if ( Sexp_nodes[CAR(n)].value != SEXP_KNOWN_TRUE )		// if value is true, mark our all_true variable for later checking
				all_true = 0;
		}
		else
			result &= atoi(CTEXT(n));

		// a little test -- if the previous sexpressions was true, then mark the node itself as always
		// true.  I did this because of the distance function.  It might become true, then when waiting for
		// the second evalation, it might become false, rendering this function false.  So, when one becomes
		// true -- mark it true forever.
		if ( result )
			Sexp_nodes[CAR(n)].value = SEXP_KNOWN_TRUE;

		while (CDR(n) != -1)
		{
			int next_result;

			next_result = is_sexp_true(CDR(n));
			if ( next_result && !result )				// if current result is true, and our running result is false, thngs didn't become true in order
				return SEXP_KNOWN_FALSE;
			result &= next_result;
			if ( Sexp_nodes[CDR(n)].value == SEXP_KNOWN_FALSE )
				return SEXP_KNOWN_FALSE;															// if one of the OR clauses is TRUE, whole clause is true
			if ( Sexp_nodes[CDR(n)].value != SEXP_KNOWN_TRUE )				// if the value is still unknown, they all can't be false
				all_true = 0;

			// see comment above for explanation of next lines
			if ( result )
				Sexp_nodes[CDR(n)].value = SEXP_KNOWN_TRUE;

			n = CDR(n);
		}
	}

	if ( all_true )
		return SEXP_KNOWN_TRUE;

	return result;
}

// for these four basic boolean operations (not, <, >, and =), we have special cases that we must deal
// with.  We have sexpressions operators that might return a NAN type return value (such as the distance
// between two ships when one of the ships is destroyed or departed).  These operations need to check for
// this special NAN value and adjust their return types accordingly.  NAN values represent false return values
int sexp_not(int n)
{
	int result = 0;

	if (n != -1)
	{
		if (CAR(n) != -1)
		{
			result = is_sexp_true(CAR(n));
			if ( Sexp_nodes[CAR(n)].value == SEXP_KNOWN_FALSE )
				return SEXP_KNOWN_TRUE;												// not KNOWN_FALSE == KNOWN_TRUE;
			else if ( Sexp_nodes[CAR(n)].value == SEXP_KNOWN_TRUE )		// not KNOWN_TRUE == KNOWN_FALSE
				return SEXP_KNOWN_FALSE;
			else if ( Sexp_nodes[CAR(n)].value == SEXP_NAN )				// not NAN == TRUE (I think)
				return SEXP_TRUE;
			else if ( Sexp_nodes[CAR(n)].value == SEXP_NAN_FOREVER )
				return SEXP_TRUE;
		}
		else
			result = atoi(CTEXT(n));
	}

	return !result;
}

int sexp_xor(int node)
{
	int num_true = 0;

	for (int n = node; n != -1; n = CDR(n))
	{
		if (is_sexp_true(n))
			num_true++;
	}

	return (num_true == 1);
}

// Goober5000
int sexp_number_compare(int n, int op)
{
	int first_node = n;
	int current_node;
	int first_number = eval_sexp(first_node);
	int current_number;

	// bail on NANs
	if (CAR(first_node) != -1)
	{
		if (Sexp_nodes[CAR(first_node)].value == SEXP_NAN) return SEXP_FALSE;
		if (Sexp_nodes[CAR(first_node)].value == SEXP_NAN_FOREVER) return SEXP_KNOWN_FALSE;
	}
	if (CDR(first_node) != -1)
	{
		if (Sexp_nodes[CDR(first_node)].value == SEXP_NAN) return SEXP_FALSE;
		if (Sexp_nodes[CDR(first_node)].value == SEXP_NAN_FOREVER) return SEXP_KNOWN_FALSE;
	}

	// compare first node with each of the others
	for (current_node = CDR(first_node); current_node != -1; current_node = CDR(current_node))
	{
		// bail on NANs
		if (CAR(current_node) != -1)
		{
			if (Sexp_nodes[CAR(current_node)].value == SEXP_NAN) return SEXP_FALSE;
			if (Sexp_nodes[CAR(current_node)].value == SEXP_NAN_FOREVER) return SEXP_KNOWN_FALSE;
		}
		if (CDR(current_node) != -1)
		{
			if (Sexp_nodes[CDR(current_node)].value == SEXP_NAN) return SEXP_FALSE;
			if (Sexp_nodes[CDR(current_node)].value == SEXP_NAN_FOREVER) return SEXP_KNOWN_FALSE;
		}

		current_number = eval_sexp(current_node);

		// must satisfy our particular operator
		switch(op)
		{
			case OP_EQUALS:
				if (first_number != current_number) return SEXP_FALSE;
				break;

			case OP_NOT_EQUAL:
				if (first_number == current_number) return SEXP_FALSE;
				break;

			case OP_GREATER_THAN:
				if (first_number <= current_number) return SEXP_FALSE;
				break;

			case OP_GREATER_OR_EQUAL:
				if (first_number < current_number) return SEXP_FALSE;
				break;

			case OP_LESS_THAN:
				if (first_number >= current_number) return SEXP_FALSE;
				break;

			case OP_LESS_OR_EQUAL:
				if (first_number > current_number) return SEXP_FALSE;
				break;

			default:
				Warning(LOCATION, "Unhandled comparison case!  Operator = ", op);
				break;
		}
	}

	// it satisfies the operator for all the arguments
	return SEXP_TRUE;
}

// Goober5000
int sexp_string_compare(int n, int op)
{
	int first_node = n;
	int current_node;
	int val;
	char *first_string = CTEXT(first_node);

	// compare first node with each of the others
	for (current_node = CDR(first_node); current_node != -1; current_node = CDR(current_node))
	{
		val = strcmp(first_string, CTEXT(current_node));

		// must satisfy our particular operator
		switch(op)
		{
			case OP_STRING_EQUALS:
				if (val != 0) return SEXP_FALSE;
				break;

			case OP_STRING_GREATER_THAN:
				if (val <= 0) return SEXP_FALSE;
				break;

			case OP_STRING_LESS_THAN:
				if (val >= 0) return SEXP_FALSE;
				break;
		}
	}

	// it satisfies the operator for all the arguments
	return SEXP_TRUE;
}

#define OSWPT_TYPE_NONE				0
#define OSWPT_TYPE_SHIP				1
#define OSWPT_TYPE_WING				2
#define OSWPT_TYPE_WAYPOINT			3
#define OSWPT_TYPE_TEAM				4
#define OSWPT_TYPE_PARSE_OBJECT		5	// a "ship" that hasn't arrived yet
#define OSWPT_TYPE_EXITED			6
#define OSWPT_TYPE_WING_NOT_PRESENT	7	// a wing that hasn't arrived yet or is between waves

// Goober5000
typedef struct object_ship_wing_point_team
{
	char *object_name;
	int type;

	p_object *p_objp;
	object *objp;
	ship *shipp;
	wing *wingp;
	waypoint *waypointp;
	int team;
}
object_ship_wing_point_team;

// Goober5000
void sexp_get_object_ship_wing_point_team(object_ship_wing_point_team *oswpt, char *object_name)
{
	int team, ship_num, wing_num;
	waypoint *wpt;
	p_object *p_objp;

	Assert(oswpt != NULL);
	Assert(object_name != NULL);

	oswpt->object_name = object_name;
	oswpt->type = OSWPT_TYPE_NONE;

	oswpt->p_objp = NULL;
	oswpt->objp = NULL;
	oswpt->shipp = NULL;
	oswpt->waypointp = NULL;
	oswpt->wingp = NULL;
	oswpt->team = -1;

	if(!stricmp(object_name, SEXP_NONE_STRING))
	{
		oswpt->type = OSWPT_TYPE_NONE;
		return;
	}

	// check to see if ship destroyed or departed.  In either case, do nothing.
	if (mission_log_get_time(LOG_SHIP_DEPARTED, object_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, object_name, NULL, NULL))
	{
		oswpt->type = OSWPT_TYPE_EXITED;
		return;
	}

	// the object might be the name of a wing.  Check to see if the wing is destroyed or departed.
	if (mission_log_get_time(LOG_WING_DESTROYED, object_name, NULL, NULL) || mission_log_get_time(LOG_WING_DEPARTED, object_name, NULL, NULL)) 
	{
		oswpt->type = OSWPT_TYPE_EXITED;
		return;
	}


	// if we have a team type, pick the first ship of that team
	team = sexp_determine_team(object_name);
	if (team >= 0)
	{
		for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
		{
			object *objp = &Objects[so->objnum];
			ship *shipp = &Ships[objp->instance];

			if (shipp->team == team)
			{
				oswpt->type = OSWPT_TYPE_TEAM;

				oswpt->team = team;
				oswpt->objp = objp;
				oswpt->shipp = shipp;

				return;
			}			
		}

		// no match
		return;
	}


	// at this point, we must have a ship, wing, or point for a target
	ship_num = ship_name_lookup(object_name);
	if (ship_num >= 0)
	{
		oswpt->type = OSWPT_TYPE_SHIP;

		oswpt->shipp = &Ships[ship_num];
		oswpt->objp = &Objects[oswpt->shipp->objnum];

		return;
	}


	// check to see if we have a parse object instead
	p_objp = mission_parse_get_arrival_ship(object_name);
	if (p_objp != NULL)
	{
		oswpt->type = OSWPT_TYPE_PARSE_OBJECT;

		oswpt->p_objp = p_objp;

		return;
	}


	// at this point, we must have a wing or point for a target
	wing_num = wing_name_lookup(object_name, 1);
	if (wing_num >= 0)
	{
		wing *wingp = &Wings[wing_num];

		// make sure that at least one ship exists
		if (wingp->current_count > 0)
		{
			oswpt->type = OSWPT_TYPE_WING;
			oswpt->wingp = wingp;

			// point to wing leader if he is valid
			if ((wingp->special_ship >= 0) && (wingp->ship_index[wingp->special_ship] >= 0))
			{
				oswpt->shipp = &Ships[wingp->ship_index[wingp->special_ship]];
				oswpt->objp = &Objects[oswpt->shipp->objnum];
			}
			// boo... well, just point to ship at index 0
			else
			{
				oswpt->shipp = &Ships[wingp->ship_index[0]];
				oswpt->objp = &Objects[oswpt->shipp->objnum];
				Warning(LOCATION, "Substituting ship '%s' at index 0 for nonexistent wing leader at index %d!", oswpt->shipp->ship_name, oswpt->wingp->special_ship);
			}
		}
		// it's still a valid wing even if nobody is here
		else
		{
			oswpt->type = OSWPT_TYPE_WING_NOT_PRESENT;
			oswpt->wingp = wingp;
		}

		return;
	}


	// at this point, we must have a point for a target
	wpt = find_matching_waypoint(object_name);
	if ((wpt != NULL) && (wpt->get_objnum() >= 0))
	{
		oswpt->type = OSWPT_TYPE_WAYPOINT;

		oswpt->waypointp = wpt;
		oswpt->objp = &Objects[wpt->get_objnum()];

		return;
	}


	// we apparently don't have anything legal
	return;
}

/**
 * Return the number of ships of a given team in the area battle
 */
int sexp_num_ships_in_battle(int n)
{
	int team=-1;
	int count=0;
	ship_obj	*so;
	ship		*shipp;
	object_ship_wing_point_team oswpt1;
	if ( n == -1) {
    	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		    shipp=&Ships[Objects[so->objnum].instance];
		    count++;
	    }

	    return count;
	}

	while (n != -1) {
		sexp_get_object_ship_wing_point_team(&oswpt1, CTEXT(n));

	    switch (oswpt1.type){
			// Should use OSWPT_TYPE_TEAM but can't in order to keep compatibility with the existing SEXP
 		    case OSWPT_TYPE_NONE:
			  team = iff_lookup(CTEXT(n));
			  if (team >= 0) {
				  for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
					  shipp=&Ships[Objects[so->objnum].instance];
					  if (shipp->team == team) {
						 count++;
					  }
				  }
			  }
			  break;

  		    case OSWPT_TYPE_SHIP:
			  count++;
			  break;

		    case OSWPT_TYPE_WING:
			  count += oswpt1.wingp->current_count;
			  break;
	    }

        n = CDR(n); 
	}

	return count;
}

/** 
 * Return the number of ships of a given wing or wings in the battle area
 */
int sexp_num_ships_in_wing(int n)
{
	char *name;
	int num_ships = 0 ;

	// A wing name must be provided, Assert that there is one.
	Assert ( n != -1 );

	//Cycle through the list of ships given
	while (n != -1)
	{
		// Get the name of the wing
		name = CTEXT(n);

		int wingnum;
		wingnum = wing_name_lookup( name, 0 );

		//If the wing exists add the number of ships in it to the total
		if (wingnum > -1)
		{
			num_ships += Wings[wingnum].current_count ;
		}

		//get the next node
		n = CDR (n) ; 
	}

	return num_ships ;
}

/**
 * Gets the 'real' speed of an object, taking into account docking
 */
int sexp_get_real_speed(object *obj)
{
	return fl2i(dock_calc_docked_speed(obj));
}

/**
 * Gets the current speed of the specified object
 *
 * Uses a lot of code shamelessly ripped from get_object_coordinates
 */
int sexp_current_speed(int n)
{
	object_ship_wing_point_team oswpt;
	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));

	switch (oswpt.type)
	{
		case OSWPT_TYPE_EXITED:
			return SEXP_NAN;

		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WING:
			return sexp_get_real_speed(oswpt.objp);
	}

	return 0;
}

/**
 * Evaluate if given ship is destroyed.
 * @return true if the ship in the expression has been destroyed.
 */
int sexp_is_destroyed(int n, fix *latest_time)
{
	char	*name;
	int	count, num_destroyed, wing_index;
	fix	time;

	Assert ( n != -1 );

	count = 0;
	num_destroyed = 0;
	wing_index = -1;
	while (n != -1) {
		count++;
		name = CTEXT(n);

		if (sexp_query_has_yet_to_arrive(name))
			return SEXP_CANT_EVAL;

		// check to see if this ship/wing has departed.  If so, then function is known false
		if ( mission_log_get_time (LOG_SHIP_DEPARTED, name, NULL, NULL) || mission_log_get_time (LOG_WING_DEPARTED, name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;

		// check the mission log.  If ship/wing not destroyed, immediately return SEXP_FALSE.
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, &time) || mission_log_get_time(LOG_WING_DESTROYED, name, NULL, &time) || mission_log_get_time(LOG_SELF_DESTRUCTED, name, NULL, &time)) {
			num_destroyed++;
			if ( latest_time && (time > *latest_time) )
				*latest_time = time;
		} else {
			// ship or wing isn't destroyed -- add to directive count
			if ( (wing_index = wing_name_lookup( name, 1 )) >= 0 ) {
				Directive_count += Wings[wing_index].current_count;
			} else
				Directive_count++;
		}

		// move to next ship/wing in list
		n = CDR(n);
	}

	// special case to mark a directive for destroy wing objectives true after a short amount
	// of time when there are more waves for this wing.
	if ( (count == 1) && (wing_index >= 0) && (Directive_count == 0) ) {
		if ( Wings[wing_index].current_wave < Wings[wing_index].num_waves )
			Directive_count =	DIRECTIVE_WING_ZERO;
	}

	if ( count == num_destroyed )
		return SEXP_KNOWN_TRUE;
	else
		return SEXP_FALSE;
}


/**
 * Return true if the subsystem of the given ship has been destroyed
 */
int sexp_is_subsystem_destroyed(int n)
{
	char *ship_name, *subsys_name;

	Assert( n != -1 );
	
	ship_name = CTEXT(n);
	subsys_name = CTEXT(CDR(n));

	if (sexp_query_has_yet_to_arrive(ship_name))
		return SEXP_CANT_EVAL;

	// if the ship has departed, no way to destroy it's subsystem.
	if ( mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL ))
		return SEXP_KNOWN_FALSE;
	
	if ( mission_log_get_time(LOG_SHIP_SUBSYS_DESTROYED, ship_name, subsys_name, NULL) )
		return SEXP_KNOWN_TRUE;

	return SEXP_FALSE;

}

/**
 * Determine if a ship has docked
 */
int sexp_has_docked(int n)
{
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = eval_num(CDR(CDR(n)));		// count of times that we should look for

	if (sexp_query_has_yet_to_arrive(docker))
		return SEXP_CANT_EVAL;

	if (sexp_query_has_yet_to_arrive(dockee))
		return SEXP_CANT_EVAL;

	Assert ( count > 0 );
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, docker, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, dockee, NULL, NULL) )
		return SEXP_KNOWN_FALSE;

	if ( !mission_log_get_time_indexed(LOG_SHIP_DOCKED, docker, dockee, count, NULL) )
		return SEXP_FALSE;

	return SEXP_KNOWN_TRUE;
}

/**
 * Determine if a ship has undocked
 */
int sexp_has_undocked(int n)
{
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = eval_num(CDR(CDR(n)));

	if (sexp_query_has_yet_to_arrive(docker))
		return SEXP_CANT_EVAL;

	if (sexp_query_has_yet_to_arrive(dockee))
		return SEXP_CANT_EVAL;

	Assert ( count > 0 );
	if ( !mission_log_get_time_indexed(LOG_SHIP_UNDOCKED, docker, dockee, count, NULL) ) {
		// if either ship destroyed before they dock, then sexp is known false
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, docker, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, dockee, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else
			return SEXP_FALSE;
	}

	return SEXP_KNOWN_TRUE;
}

/**
 * Determine if a ship has arrived onto the scene
 */
int sexp_has_arrived(int n, fix *latest_time)
{
	char *name;
	int	count, num_arrived;
	fix	time;

	count = 0;
	num_arrived = 0;
	while ( n != -1 ) {
		count++;
		name = CTEXT(n);
		// if there is no log entry for this ship/wing for arrival, sexpression is false
		if ( mission_log_get_time(LOG_SHIP_ARRIVED, name, NULL, &time) || mission_log_get_time(LOG_WING_ARRIVED, name, NULL, &time) ) {
			num_arrived++;
			if ( latest_time && (time > *latest_time) )
				*latest_time = time;
		}
		n = CDR(n);
	}

	if ( count == num_arrived )
		return SEXP_KNOWN_TRUE;
	else
		return SEXP_FALSE;
}

/**
 * Determine if a ship/wing has departed
 */
int sexp_has_departed(int n, fix *latest_time)
{
	char *name;
	int count, num_departed;
	fix time;

	count = 0;
	num_departed = 0;
	while ( n != -1 ) {
		count++;
		name = CTEXT(n);

		if (sexp_query_has_yet_to_arrive(name))
			return SEXP_CANT_EVAL;

		// if ship/wing destroyed, sexpression is known false.  Also, if there is no departure log entry, then
		// the sexpression is not true.
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, NULL) || mission_log_get_time(LOG_WING_DESTROYED, name, NULL, NULL))
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_SHIP_DEPARTED, name, NULL, &time) || mission_log_get_time(LOG_WING_DEPARTED, name, NULL, &time) ) {
			num_departed++;
			if ( latest_time && (time > *latest_time) )
				*latest_time = time;
		}
		n = CDR(n);
	}

	if ( count == num_departed )
		return SEXP_KNOWN_TRUE;
	else
		return SEXP_FALSE;
}

/**
 * Determine if a ship is disabled
 */
int sexp_is_disabled( int n, fix *latest_time )
{
	char *name;
	int count, num_disabled;
	fix time;

	count = 0;
	num_disabled = 0;
	while ( n != -1 ) {
		count++;
		name = CTEXT(n);

		if (sexp_query_has_yet_to_arrive(name))
			return SEXP_CANT_EVAL;

		// if ship/wing destroyed, sexpression is known false.  Also, if there is no disable log entry, then
		// the sexpression is not true.
		if ( mission_log_get_time(LOG_SHIP_DEPARTED, name, NULL, &time) || mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, &time) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_SHIP_DISABLED, name, NULL, &time) ) {
			num_disabled++;
			if ( latest_time && (time > *latest_time) )
				*latest_time = time;
		}
		n = CDR(n);
	}

	if ( count == num_disabled )
		return SEXP_KNOWN_TRUE;
	else
		return SEXP_FALSE;
}

/**
 * Determine if a ship is done flying waypoints
 */
int sexp_are_waypoints_done(int n)
{
	char *ship_name, *waypoint_name;

	ship_name = CTEXT(n);
	waypoint_name = CTEXT(CDR(n));

	if (sexp_query_has_yet_to_arrive(ship_name))
		return SEXP_CANT_EVAL;

	// a destroyed or departed ship will never reach their goal -- return known false
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
		return SEXP_KNOWN_FALSE;
	else if ( mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL) )
		return SEXP_KNOWN_FALSE;

	// now check the log for the waypoints done entry
	if ( mission_log_get_time(LOG_WAYPOINTS_DONE, ship_name, waypoint_name, NULL) )
		return SEXP_KNOWN_TRUE;

	return SEXP_FALSE;
}


/**
 * Determine if ships are disarmed
 */
int sexp_is_disarmed( int n, fix *latest_time )
{
	char *name;
	int count, num_disarmed;
	fix time;

	count = 0;
	num_disarmed = 0;
	while ( n != -1 ) {
		count++;
		name = CTEXT(n);

		if (sexp_query_has_yet_to_arrive(name))
			return SEXP_CANT_EVAL;

		// if ship/wing destroyed, sexpression is known false.  Also, if there is no disarm log entry, then
		// the sexpression is not true.
		if ( mission_log_get_time(LOG_SHIP_DEPARTED, name, NULL, &time) || mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, &time) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_SHIP_DISARMED, name, NULL, &time) ) {
			num_disarmed++;
			if ( latest_time && (time > *latest_time) )
				*latest_time = time;
		}
		n = CDR(n);
	}

	if ( count == num_disarmed )
		return SEXP_KNOWN_TRUE;
	else
		return SEXP_FALSE;
}

// the following functions are similar to the above objective functions but return true/false
// if N seconds have elasped after the corresponding function is true.
int sexp_is_destroyed_delay(int n)
{
	fix delay, time;
	int val;

	Assert ( n >= 0 );

	time = 0;

	delay = i2f(eval_num(n));

	// check value of is_destroyed function.  KNOWN_FALSE should be returned immediately
	val = sexp_is_destroyed( CDR(n), &time );
	if ( val == SEXP_KNOWN_FALSE )
		return val;

	if ( val == SEXP_CANT_EVAL )
		return SEXP_CANT_EVAL;

	if ( val ) {

		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return SEXP_FALSE;
}

int sexp_is_subsystem_destroyed_delay(int n)
{
	char *ship_name, *subsys_name;
	fix delay, time;

	Assert( n != -1 );
	
	ship_name = CTEXT(n);
	subsys_name = CTEXT(CDR(n));
	delay = i2f(eval_num(CDR(CDR(n))));

	if (sexp_query_has_yet_to_arrive(ship_name))
		return SEXP_CANT_EVAL;

	// if the ship has departed, no way to destroy it's subsystem.
	if ( mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL ))
		return SEXP_KNOWN_FALSE;

	if ( mission_log_get_time(LOG_SHIP_SUBSYS_DESTROYED, ship_name, subsys_name, &time) ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return SEXP_FALSE;
}

int sexp_is_disabled_delay(int n)
{
	fix delay, time;
	int val;

	Assert ( n >= 0 );

	time = 0;
	delay = i2f(eval_num(n));

	// check value of is_disable for known false and return immediately if it is.
	val = sexp_is_disabled( CDR(n), &time );
	if ( val == SEXP_KNOWN_FALSE )
		return val;

	if ( val == SEXP_CANT_EVAL )
		return SEXP_CANT_EVAL;

	if ( val ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return SEXP_FALSE;
}

int sexp_is_disarmed_delay(int n)
{
	fix delay, time;
	int val;

	Assert ( n >= 0 );

	time = 0;
	delay = i2f(eval_num(n));
	
	// check value of is_disarmed for a known false value and return that immediately if it is
	val = sexp_is_disarmed( CDR(n), &time );
	if ( val == SEXP_KNOWN_FALSE )
		return val;

	if ( val == SEXP_CANT_EVAL )
		return SEXP_CANT_EVAL;

	if ( val ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return SEXP_FALSE;
}

int sexp_has_docked_delay(int n)
{
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = eval_num(CDR(CDR(n)));		// count of times that we should look for
	fix delay = i2f(eval_num(CDR(CDR(CDR(n)))));
	fix time;
	if (count <= 0)
	{
		Warning(LOCATION, "Has-docked-delay count should be at least 1!  This has been automatically adjusted.");
		count = 1;
	}

	if ( mission_log_get_time(LOG_SHIP_DESTROYED, docker, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, dockee, NULL, NULL) )
		return SEXP_KNOWN_FALSE;

	if (sexp_query_has_yet_to_arrive(docker))
		return SEXP_CANT_EVAL;

	if (sexp_query_has_yet_to_arrive(dockee))
		return SEXP_CANT_EVAL;

	if ( !mission_log_get_time_indexed(LOG_SHIP_DOCKED, docker, dockee, count, &time) )
		return SEXP_FALSE;

	if ( (Missiontime - time) >= delay )
		return SEXP_KNOWN_TRUE;
	else
		return SEXP_FALSE;
}

int sexp_has_undocked_delay(int n)
{
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = eval_num(CDR(CDR(n)));
	fix delay = i2f(eval_num(CDR(CDR(CDR(n)))));
	fix time;
	if (count <= 0)
	{
		Warning(LOCATION, "Has-undocked-delay count should be at least 1!  This has been automatically adjusted.");
		count = 1;
	}

	if (sexp_query_has_yet_to_arrive(docker))
		return SEXP_CANT_EVAL;

	if (sexp_query_has_yet_to_arrive(dockee))
		return SEXP_CANT_EVAL;

	if ( !mission_log_get_time_indexed(LOG_SHIP_UNDOCKED, docker, dockee, count, &time) ) {
		// if either ship destroyed before they dock, then sexp is known false
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, docker, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, dockee, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else
			return SEXP_FALSE;
	}

	if ( (Missiontime - time) >= delay )
		return SEXP_KNOWN_TRUE;
	else
		return SEXP_FALSE;
}

int sexp_has_arrived_delay(int n)
{
	fix delay, time;
	int val;

	Assert ( n >= 0 );

	time = 0;
	delay = i2f(eval_num(n));

	// check return value from arrived function.  if can never arrive, then return that value here as well
	val = sexp_has_arrived( CDR(n), &time );
	if ( val == SEXP_KNOWN_FALSE )
		return val;

	if ( val == SEXP_CANT_EVAL )
		return SEXP_CANT_EVAL;

	if ( val ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return SEXP_FALSE;
}

int sexp_has_departed_delay(int n)
{
	fix delay, time;
	int val;

	Assert ( n >= 0 );

	time = 0;
	delay = i2f(eval_num(n));

	// must first check to see if the departed function could ever be true/false or is true or false.
	// if it can never be true, return that value
	val = sexp_has_departed( CDR(n), &time);
	if ( val == SEXP_KNOWN_FALSE )
		return val;

	if ( val == SEXP_CANT_EVAL )
		return SEXP_CANT_EVAL;

	if ( val ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return SEXP_FALSE;
}

/**
 * Determine if a ship is done flying waypoints after N seconds
 */
int sexp_are_waypoints_done_delay(int node)
{
	char *ship_name, *waypoint_name;
	int count, n = node;
	fix time, delay;

	ship_name = CTEXT(n);
	n = CDR(n);
	waypoint_name = CTEXT(n);
	n = CDR(n);
	delay = i2f(eval_num(n));
	n = CDR(n);
	count = (n >= 0) ? eval_num(n) : 1;
	if (count <= 0)
	{
		Warning(LOCATION, "Are-waypoints-done-delay count should be at least 1!  This has been automatically adjusted.");
		count = 1;
	}

	if (sexp_query_has_yet_to_arrive(ship_name))
		return SEXP_CANT_EVAL;

	// a destroyed or departed ship will never reach their goal -- return known false
	// 
	// Not checking the entries below.  Ships which warp out after reaching their goal (or getting
	// destroyed after their goal), but after reaching their waypoints, may have this goal incorrectly
	// marked false!!!!

	// now check the log for the waypoints done entry
	if ( mission_log_get_time_indexed(LOG_WAYPOINTS_DONE, ship_name, waypoint_name, count, &time) ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	} else {
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
	}

	return SEXP_FALSE;
}

/**
 * Determine is all of a given ship type are destroyed
 */
int sexp_ship_type_destroyed(int n)
{
	int percent;
	int type;
	char *shiptype;

	percent = eval_num(n);
	shiptype = CTEXT(CDR(n));

	type = ship_type_name_lookup(shiptype);

	// bogus if we reach the end of this array!!!!
	if ( type < 0 ) {
		Warning(LOCATION, "Invalid shiptype passed to ship-type-destroyed");
		return SEXP_FALSE;
	}

	if ( type >= (int)Ship_type_counts.size() || Ship_type_counts[type].total == 0 )
		return SEXP_FALSE;

	//We are safe from array indexing probs b/c of previous if.
	// determine if the percentage of killed/total is >= percentage given in the expression
	if ( (Ship_type_counts[type].killed * 100 / Ship_type_counts[type].total) >= percent)
		return SEXP_KNOWN_TRUE;
	
	return SEXP_FALSE;
}


// following are time based functions
int sexp_has_time_elapsed(int n)
{
	int time = eval_num(n);

	if ( f2i(Missiontime) >= time )
		return SEXP_KNOWN_TRUE;

	return SEXP_FALSE;
}

/**
 * Returns the time into the mission
 */
int sexp_mission_time()
{
	return f2i(Missiontime);
}

/**
 * Returns the time into the mission, in milliseconds
 */
int sexp_mission_time_msecs()
{
	// multiplying by 1000 can go over the limit for LONG_MAX so cast to long long int first
	return f2i((longlong)Missiontime * 1000);
}

/**
 * Returns percent of length of distance to special warpout plane
 */
int sexp_special_warp_dist( int n)
{
	char *ship_name;
	int shipnum;

	// get shipname
	ship_name = CTEXT(n);

	// check to see if either ship was destroyed or departed.  If so, then make this node known
	// false
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPARTED, ship_name, NULL, NULL) ) {
		return SEXP_NAN_FOREVER;
	}

	// get ship name
	shipnum = ship_name_lookup(ship_name);
	if (shipnum < 0) {
		return SEXP_NAN;
	}

	// check that ship has warpout_objnum
	if (Ships[shipnum].special_warpout_objnum < 0) {
		return SEXP_NAN;
	}
	
	Assert( (Ships[shipnum].special_warpout_objnum >= 0) && (Ships[shipnum].special_warpout_objnum < MAX_OBJECTS));
	if ( (Ships[shipnum].special_warpout_objnum < 0) && (Ships[shipnum].special_warpout_objnum >= MAX_OBJECTS) ) {
		return SEXP_NAN;
	}

	// check the special warpout device is valid
	int valid = FALSE;
	object *ship_objp = &Objects[Ships[shipnum].objnum];
	object *warp_objp = &Objects[Ships[shipnum].special_warpout_objnum];
	if (warp_objp->type == OBJ_SHIP) {
		if (Ship_info[Ships[warp_objp->instance].ship_info_index].flags & SIF_KNOSSOS_DEVICE) {
			valid = TRUE;
		}
	}

	if (!valid) {
		return SEXP_NAN;
	}

	// check if within 45 degree half-angle cone of facing 
	float dot = fl_abs(vm_vec_dotprod(&warp_objp->orient.vec.fvec, &ship_objp->orient.vec.fvec));
	if (dot < 0.707f) {
		return SEXP_NAN;
	}

	// get distance
	vec3d hit_pt;
	float dist = fvi_ray_plane(&hit_pt, &warp_objp->pos, &warp_objp->orient.vec.fvec, &ship_objp->pos, &ship_objp->orient.vec.fvec, 0.0f);
	polymodel *pm = model_get(Ship_info[Ships[shipnum].ship_info_index].model_num);
	dist += pm->mins.xyz.z;

	// return as a percent of length
	return (int) (100.0f * dist / ship_class_get_length(&Ship_info[Ships[shipnum].ship_info_index]));
}


int sexp_time_destroyed(int n)
{
	fix time;

	if ( !mission_log_get_time( LOG_SHIP_DESTROYED, CTEXT(n), NULL, &time) ){		// returns 0 when not found
		return SEXP_NAN;
	}
	
	return f2i(time);
}

int sexp_time_wing_destroyed(int n)
{
	fix time;

	if ( !mission_log_get_time( LOG_WING_DESTROYED, CTEXT(n), NULL, &time) ){
		return SEXP_NAN;
	}
	
	return f2i(time);
}

int sexp_time_docked(int n)
{
	fix time;
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = eval_num(CDR(CDR(n)));

	Assert ( count > 0 );
	if ( !mission_log_get_time_indexed(LOG_SHIP_DOCKED, docker, dockee, count, &time) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

int sexp_time_undocked(int n)
{
	fix time;
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = eval_num(CDR(CDR(n)));

	Assert ( count > 0 );
	if ( !mission_log_get_time_indexed(LOG_SHIP_UNDOCKED, docker, dockee, count, &time) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

int sexp_time_ship_arrived(int n)
{
	fix time;

	Assert( n != -1 );
	if ( !mission_log_get_time( LOG_SHIP_ARRIVED, CTEXT(n), NULL, &time ) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

int sexp_time_wing_arrived(int n)
{
	fix time;

	Assert( n != -1 );
	if ( !mission_log_get_time( LOG_WING_ARRIVED, CTEXT(n), NULL, &time ) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

int sexp_time_ship_departed(int n)
{
	fix time;

	Assert( n != -1 );
	if ( !mission_log_get_time( LOG_SHIP_DEPARTED, CTEXT(n), NULL, &time ) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

int sexp_time_wing_departed(int n)
{
	fix time;

	Assert( n != -1 );
	if ( !mission_log_get_time(LOG_WING_DEPARTED, CTEXT(n), NULL, &time ) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

void sexp_set_energy_pct (int node, int op_num)
{
	int sindex;
	float new_pct; 
	char *shipname;
	ship * shipp; 
	ship_info * sip; 

	Assert (node > -1);
	new_pct = eval_num(node) / 100.0f;

	// deal with ridiculous percentages
    CLAMP(new_pct, 0.0f, 1.0f);
	
	// only need to send a packet for afterburners because shields and weapon energy are sent from server to clients
	if (MULTIPLAYER_MASTER && (op_num == OP_SET_AFTERBURNER_ENERGY)) {
		multi_start_callback();
		multi_send_float(new_pct); 
	}

	node = CDR(node); 

	while (node > -1) {
		// get the ship
		shipname = CTEXT(node);

		sindex = ship_name_lookup( shipname );
		if ( sindex == -1 ){					// hmm.. if true, must not have arrived yet
			node = CDR(node);
			continue;
		}
		
		shipp = &Ships[sindex]; 
		sip = &Ship_info[Ships[sindex].ship_info_index]; 

		switch (op_num) {
			case OP_SET_AFTERBURNER_ENERGY:
				shipp->afterburner_fuel = sip->afterburner_fuel_capacity * new_pct; 
				break;

			case OP_SET_WEAPON_ENERGY:
				if (!(ship_has_energy_weapons(shipp)) ) {
					node = CDR(node); 
					continue;
				}
				
				shipp->weapon_energy = sip->max_weapon_reserve * new_pct;
				break; 

			case OP_SET_SHIELD_ENERGY:
				if (shipp->ship_max_shield_strength == 0.0f) {
					node = CDR(node); 
					continue;
				}	

				shield_set_strength(&Objects[shipp->objnum], (shipp->ship_max_shield_strength * new_pct));
				break;
		}

		if (MULTIPLAYER_MASTER && (op_num == OP_SET_AFTERBURNER_ENERGY)) {
			multi_send_ship(shipp); 
		}

		node = CDR(node); 
	}

	if (MULTIPLAYER_MASTER && (op_num == OP_SET_AFTERBURNER_ENERGY)) {
		multi_end_callback(); 
	}
}

void multi_sexp_set_energy_pct() 
{
	ship *shipp; 
	float new_pct; 
	ship_info * sip; 

	int op_num = multi_sexp_get_operator(); 

	multi_get_float(new_pct); 
	while (multi_get_ship(shipp)) {
		sip = &Ship_info[shipp->ship_info_index]; 

		switch (op_num) {
			case OP_SET_AFTERBURNER_ENERGY:
				shipp->afterburner_fuel = sip->afterburner_fuel_capacity * new_pct; 
				break;

			case OP_SET_WEAPON_ENERGY:
				shipp->weapon_energy = sip->max_weapon_reserve * new_pct;
				break; 

			case OP_SET_SHIELD_ENERGY:
				shield_set_strength(&Objects[shipp->objnum], (shipp->ship_max_shield_strength * new_pct));
				break;
		}
	}
}


int sexp_get_energy_pct (int node, int op_num)
{
	int sindex;
	float maximum = 0.0f, current = 0.0f; 
	ship * shipp; 
	ship_info * sip; 
	char *shipname;

	// get the ship
	shipname = CTEXT(node);

	sindex = ship_name_lookup( shipname );
	if ( sindex == -1 ){					// has either not arrived yet or has departed
		return SEXP_NAN;
	}

	shipp = &Ships[sindex]; 
	sip = &Ship_info[Ships[sindex].ship_info_index]; 

	switch (op_num) {
		case OP_AFTERBURNER_LEFT:
			maximum = sip->afterburner_fuel_capacity;
			current = shipp->afterburner_fuel;
			break; 
		case OP_WEAPON_ENERGY_LEFT:
			if (ship_has_energy_weapons(shipp)) {
				maximum = sip->max_weapon_reserve; 
				current = shipp->weapon_energy;
			}
			break; 
	}
	if (maximum < WEAPON_RESERVE_THRESHOLD || current < WEAPON_RESERVE_THRESHOLD) {
		return 0;
	}

	return (int)(100 * (current/maximum));
}

/**
 * Return the remaining shields as a percentage of the given ship.
 */
int sexp_shields_left(int n)
{
	int shipnum, percent;
	char *shipname;

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPARTED, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ){					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	// Goober5000: in case ship has no shields
	if (Ships[shipnum].ship_max_shield_strength == 0.0f)
	{
		return 0;
	}

	// now return the amount of shields left as a percentage of the whole.
	percent = fl2i((get_shield_pct(&Objects[Ships[shipnum].objnum]) * 100.0f) + 0.5f);
	return percent;
}

/**
 * Return the remaining hits left as a percentage of the whole.
 *
 * This hit amount counts for all hits on the ship (hull + subsystems).  Use hits_left_hull to find hull hits remaining.
 */
int sexp_hits_left(int n)
{
	int shipnum, percent;
	char *shipname;

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPARTED, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ){					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	// now return the amount of hits left as a percentage of the whole.  Subtract the percentage from 100
	// since we are working with total hit points taken, not total remaining.
	ship		*shipp = &Ships[shipnum];
	object	*objp = &Objects[shipp->objnum];
	percent = fl2i((100.0f * get_hull_pct(objp)) + 0.5f);
	return percent;
}

int sexp_sim_hits_left(int n)
{
	int shipnum, percent;
	char *shipname;

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPARTED, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ){					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	// now return the amount of hits left as a percentage of the whole.  Subtract the percentage from 100
	// since we are working with total hit points taken, not total remaining.
	ship		*shipp = &Ships[shipnum];
	object	*objp = &Objects[shipp->objnum];
	percent = fl2i((100.0f * get_sim_hull_pct(objp)) + 0.5f);
	return percent;
}

/**
 * Determine if ship visible on radar
 * 
 * @return 0 - not visible
 * @return 1 - marginally targetable (jiggly on radar)
 * @return 2 - fully targetable
 */
int sexp_is_ship_visible(int n)
{
	char *shipname;
	int shipnum;
	int ship_is_visible = 0;

	// if multiplayer, bail
	if (Game_mode & GM_MULTIPLAYER) {
		return SEXP_NAN_FOREVER;
	}

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPARTED, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ){					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	// get ship's *radar* visiblity
	if (Player_ship != NULL)
	{
		if (ship_is_visible_by_team(&Objects[Ships[shipnum].objnum], Player_ship))
		{
			ship_is_visible = 2;
		}
	}

	// only check awacs level if ship is not visible by team
	if (Player_ship != NULL && !ship_is_visible) {
		float awacs_level = awacs_get_level(&Objects[Ships[shipnum].objnum], Player_ship);
		if (awacs_level >= 1.0f) {
			ship_is_visible = 2;
		} else if (awacs_level > 0) {
			ship_is_visible = 1;
		}
	}

	return ship_is_visible;
}

/**
 * Determine if the stealth flag set on this ship
 */
int sexp_is_ship_stealthy(int n)
{
	char *shipname;
	int shipnum;

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPARTED, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ) {					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	if (Ships[shipnum].flags2 & SF2_STEALTH)
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

/**
 * Determine if the friendly stealth ship visible
 */
int sexp_is_friendly_stealth_visible(int n)
{
	char *shipname;
	int shipnum;

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPARTED, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ) {					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	if (Ships[shipnum].flags2 & SF2_FRIENDLY_STEALTH_INVIS)
		return SEXP_FALSE;
	else
		return SEXP_TRUE;
}

// get multi team v team score
// if not multi team v team return 0
// if invalid team return 0
int sexp_team_score(int node)
{
	// if multi t vs t
	if (Game_mode & GM_MULTIPLAYER) {
		if (Netgame.type_flags & NG_TYPE_TEAM) {

			int team = eval_num(node);

			// Teams can only be 1 or 2 at the moment but we should use Num_teams in case more become possible in the future
			if (team <= 0 || team > Num_teams)
			{
				// invalid team index
				Warning(LOCATION, "sexp-team-score: team %d is not a valid team #", team);
				return 0;
			}

			return Multi_team_score[team - 1];
		}
	}

	return 0;
}


/**
 * Return the remaining hits left on a subsystem as a percentage of the whole.
 *
 * Goober5000 - this sexp is DEPRECATED because it works just like the new hits-left-substem-generic
 */
int sexp_hits_left_subsystem(int n)
{
	int shipnum, percent, type, single_subsystem = 0;
	char *shipname;
	char *subsys_name;

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPARTED, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ){					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	subsys_name = CTEXT(CDR(n));
	ship_subsys *ss = ship_get_subsys(&Ships[shipnum], subsys_name);
	if (ss != NULL)
		type = ss->system_info->type;
	else
		type = SUBSYSTEM_NONE;

	if ( (type >= 0) && (type < SUBSYSTEM_MAX) ) {
		// check for the optional argument
		n = CDDR(n); 
		if (n >= 0) {
			single_subsystem = is_sexp_true(n);
		}

		// if the third option is present or if this is an unknown subsystem type we only want to find the percentage of the 
		// named subsystem
		if (single_subsystem || (type == SUBSYSTEM_UNKNOWN)) {
			if (ss != NULL) {
				percent = fl2i((ss->current_hits / ss->max_hits * 100.0f) + 0.5f);
				return percent;
			}

			// we reached end of ship subsys list without finding subsys_name
			if (ship_class_unchanged(shipnum)) {
				Error(LOCATION, "Invalid subsystem '%s' passed to hits-left-subsystem", subsys_name);
			}
			return SEXP_NAN;

		// by default we return as a percentage the hits remaining on the subsystem as a whole (i.e. for 3 engines,
		// we are returning the sum of the hits on the 3 engines)
		} else {
			percent = fl2i((ship_get_subsystem_strength(&Ships[shipnum],type) * 100.0f) + 0.5f);
			return percent;
		}
	}
	return SEXP_NAN;			// if for some strange reason, the type field of the subsystem is bogus
}

// Goober5000
int sexp_hits_left_subsystem_generic(int node)
{
	int i, ship_num, subsys_type;
	char *ship_name, *subsys_type_name;

	ship_name = CTEXT(node);
	subsys_type_name = CTEXT(CDR(node));
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if (mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL)) {
		return SEXP_NAN_FOREVER;
	}

	ship_num = ship_name_lookup(ship_name);
	if (ship_num < 0) {
		return SEXP_NAN;
	}

	// find subsystem type
	subsys_type = -1;
	for (i = 0; i < SUBSYSTEM_MAX; i++)
	{
		if (!stricmp(subsys_type_name, Subsystem_types[i]))
			subsys_type = i;
	}

	// error checking
	if (subsys_type < 0) {
		Warning(LOCATION, "Subsystem type '%s' not recognized in hits-left-subsystem-generic!", subsys_type_name);
		return SEXP_NAN_FOREVER;
	} else if (subsys_type == SUBSYSTEM_NONE) {
		// as you wish...?
		return 0;
	} else if (subsys_type == SUBSYSTEM_UNKNOWN) {
		Warning(LOCATION, "Cannot use SUBSYSTEM_UNKNOWN in hits-left-subsystem-generic!");
		return SEXP_NAN_FOREVER;
	}

	// return as a percentage the hits remaining on the subsystem as a whole (i.e. for 3 engines,
	// we are returning the sum of the hits on the 3 engines)
	return fl2i((ship_get_subsystem_strength(&Ships[ship_num], subsys_type) * 100.0f) + 0.5f);
}

// Goober5000
int sexp_hits_left_subsystem_specific(int node)
{
	int ship_num;
	char *ship_name, *subsys_name;
	ship_subsys *ss;

	ship_name = CTEXT(node);
	subsys_name = CTEXT(CDR(node));
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if (mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL)) {
		return SEXP_NAN_FOREVER;
	}

	ship_num = ship_name_lookup(ship_name);
	if (ship_num < 0) {
		return SEXP_NAN;
	}

	// find subsystem
	ss = ship_get_subsys(&Ships[ship_num], subsys_name);
	if (ss != NULL) {
		// return as a percentage the hits remaining on this subsystem only
		return fl2i((ss->current_hits / ss->max_hits * 100.0f) + 0.5f);
	}

	// we reached end of ship subsys list without finding subsys_name
	if (ship_class_unchanged(ship_num)) {
		Error(LOCATION, "Invalid subsystem '%s' passed to hits-left-subsystem", subsys_name);
	}
	return SEXP_NAN;
}

int sexp_directive_value(int n)
{	
	int replace_current_value = SEXP_TRUE; 
	int directive_value;

	Assert(n >= 0);

	directive_value = eval_num(n);

	if ((directive_value == SEXP_NAN) || (directive_value == SEXP_NAN_FOREVER)) {
		directive_value = 0;
	}

	n = CDR(n);
	if (n > -1) {
		replace_current_value = eval_sexp(n);
	}

	if ((replace_current_value == SEXP_KNOWN_FALSE) || (replace_current_value == SEXP_FALSE) ) {
		Directive_count += directive_value;
	}
	else {
		Directive_count = directive_value;
	}

		
	return SEXP_TRUE;
}

int sexp_determine_team(char *subj)
{
	int len;
	char team_name[NAME_LENGTH];

	// quick check
	if (strnicmp(subj, "<any ", 5))
		return -1;

	// grab IFF (rest of string except for closing angle bracket)
	len = strlen(subj + 5) - 1;
	strncpy(team_name, subj + 5, len);
	team_name[len] = '\0';

	// find it
	return iff_lookup(team_name);
}

/**
 * Check distance between two given objects
 */
int sexp_distance3(object *objp1, object *objp2)
{
	// if either object isn't present in the mission now
	if (objp1 == NULL || objp2 == NULL)
		return SEXP_NAN;

	if ((objp1->type == OBJ_SHIP) && (objp2->type == OBJ_SHIP))
	{
		if (Player_obj == objp1)
			return (int) hud_find_target_distance(objp2, objp1);
		else
			return (int) hud_find_target_distance(objp1, objp2);
	}
	else
	{
		return (int) vm_vec_dist_quick(&objp1->pos, &objp2->pos);
	}
}

/**
 * Check distance between a given ship and a given subject (ship, wing, any <team>).
 */
int sexp_distance2(object *objp1, object_ship_wing_point_team *oswpt2)
{
	int dist, dist_min = 0, inited = 0;

	switch (oswpt2->type)
	{
		// we have a team type, so check all ships of that type
		case OSWPT_TYPE_TEAM:
		{
			for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
			{
				if (Ships[Objects[so->objnum].instance].team == oswpt2->team)
				{
					dist = sexp_distance3(objp1, &Objects[so->objnum]);
					if (dist != SEXP_NAN)
					{
						if (!inited || (dist < dist_min))
						{
							dist_min = dist;
							inited = 1;
						}
					}
				}
			}

			// no objects were checked
			if (!inited)
				return SEXP_NAN;

			return dist_min;
		}

		// check ships and points
		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WAYPOINT:
		{
			return sexp_distance3(objp1, oswpt2->objp);
		}

		// check wings
		case OSWPT_TYPE_WING:
		{
			for (int i = 0; i < oswpt2->wingp->current_count; i++)
			{
				dist = sexp_distance3(objp1, &Objects[Ships[oswpt2->wingp->ship_index[i]].objnum]);
				if (dist != SEXP_NAN)
				{
					if (!inited || (dist < dist_min))
					{
						dist_min = dist;
						inited = 1;
					}
				}
			}

			// no objects were checked
			if (!inited)
				return SEXP_NAN;

			return dist_min;
		}
	}

	return SEXP_NAN;
}

/**
 * Returns the distance between two objects.
 *
 * If a wing is specified as one (or both) of the arguments to this function, we are looking for the closest distance
 */
int sexp_distance(int n)
{
	int dist, dist_min = 0, inited = 0;
	object_ship_wing_point_team oswpt1, oswpt2;

	sexp_get_object_ship_wing_point_team(&oswpt1, CTEXT(n));
	sexp_get_object_ship_wing_point_team(&oswpt2, CTEXT(CDR(n)));

	// check to see if either object was destroyed or departed
	if (oswpt1.type == OSWPT_TYPE_EXITED || oswpt2.type == OSWPT_TYPE_EXITED)
		return SEXP_NAN_FOREVER;

	switch (oswpt1.type)
	{
		// we have a team type, so check all ships of that type
		case OSWPT_TYPE_TEAM:
		{
			for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
			{
				if (Ships[Objects[so->objnum].instance].team == oswpt1.team)
				{
					dist = sexp_distance2(&Objects[so->objnum], &oswpt2);
					if (dist != SEXP_NAN)
					{
						if (!inited || (dist < dist_min))
						{
							dist_min = dist;
							inited = 1;
						}
					}
				}
			}

			// no objects were checked
			if (!inited)
				return SEXP_NAN;

			return dist_min;
		}

		// check ships and points
		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WAYPOINT:
		{
			return sexp_distance2(oswpt1.objp, &oswpt2);
		}

		// check wings
		case OSWPT_TYPE_WING:
		{
			for (int i = 0; i < oswpt1.wingp->current_count; i++)
			{
				dist = sexp_distance2(&Objects[Ships[oswpt1.wingp->ship_index[i]].objnum], &oswpt2);
				if (dist != SEXP_NAN)
				{
					if (!inited || (dist < dist_min))
					{
						dist_min = dist;
						inited = 1;
					}
				}
			}

			// no objects were checked
			if (!inited)
				return SEXP_NAN;

			return dist_min;
		}
	}

	return SEXP_NAN;
}

/**
 * Locate the subsystem on a ship - Goober5000
 * 
 * Switched to a boolean so that it can report failure to do so
 */
bool sexp_get_subsystem_world_pos(vec3d *subsys_world_pos, int shipnum, char *subsys_name)
{
	Assert(subsys_name);
	Assert(subsys_world_pos);

	if(shipnum < 0)
	{
		Error(LOCATION, "Error - nonexistent ship.\n");
	}

	// find the ship subsystem
	ship_subsys *ss = ship_get_subsys(&Ships[shipnum], subsys_name);
	if (ss != NULL)
	{
		// find world position of subsystem on this object (the ship)
		get_subsystem_world_pos(&Objects[Ships[shipnum].objnum], ss, subsys_world_pos);
		return true;
	}

	// we reached end of ship subsys list without finding subsys_name 
	if (ship_class_unchanged(shipnum)) {
		// this ship should have had the subsystem named as it shouldn't have changed class
		Error(LOCATION, "sexp_get_subsystem_world_pos could not find subsystem '%s'", subsys_name);
	}
	return false;
}

/**
 * Returns the distance between an object and a ship subsystem.
 *
 * If a wing is specified as the object argument to this function, we are looking for the closest distance
 */
int sexp_distance_subsystem(int n)
{
	int ship_with_subsys_num, dist, dist_min = 0, inited = 0;
	char *ship_with_subsys_name, *subsys_name;
	vec3d subsys_pos;
	object_ship_wing_point_team oswpt;

	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));
	ship_with_subsys_name = CTEXT(CDR(n));
	subsys_name = CTEXT(CDR(CDR(n)));

	// for the ship with the subsystem - see if it was destroyed or departed
	if (mission_log_get_time(LOG_SHIP_DESTROYED, ship_with_subsys_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DEPARTED, ship_with_subsys_name, NULL, NULL))
		return SEXP_NAN_FOREVER;

	// check the other ship too
	if (oswpt.type == OSWPT_TYPE_EXITED)
		return SEXP_NAN_FOREVER;

	// for the ship with the subsystem - get its index
	ship_with_subsys_num = ship_name_lookup(ship_with_subsys_name);
	if (ship_with_subsys_num < 0)
		return SEXP_NAN;

	// get the subsystem's coordinates or bail if we can't
	if (!sexp_get_subsystem_world_pos(&subsys_pos, ship_with_subsys_num, subsys_name)) 
		return SEXP_NAN;

	switch (oswpt.type)
	{
		// we have a team type, so check all ships of that type
		case OSWPT_TYPE_TEAM:
		{
			for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
			{
				if (Ships[Objects[so->objnum].instance].team == oswpt.team)
				{
					dist = (int) vm_vec_dist_quick(&Objects[so->objnum].pos, &subsys_pos);

					if (!inited || (dist < dist_min))
					{
						dist_min = dist;
						inited = 1;
					}
				}
			}

			// no objects were checked
			if (!inited)
				return SEXP_NAN;

			return dist_min;
		}

		// check ships and points
		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WAYPOINT:
		{
			return (int) vm_vec_dist_quick(&oswpt.objp->pos, &subsys_pos);
		}

		// check wings
		case OSWPT_TYPE_WING:
		{
			for (int i = 0; i < oswpt.wingp->current_count; i++)
			{
				dist = (int) vm_vec_dist_quick(&Objects[Ships[oswpt.wingp->ship_index[i]].objnum].pos, &subsys_pos);

				if (!inited || (dist < dist_min))
				{
					dist_min = dist;
					inited = 1;
				}
			}

			// no objects were checked
			if (!inited)
				return SEXP_NAN;

			return dist_min;
		}
	}

	return SEXP_NAN;
}

bool sexp_helper_is_within_box(float *box_vals, vec3d *pos)
{
	int i;
	for(i = 0; i < 3; i++)
	{
		if(pos->a1d[i] < (box_vals[i] - box_vals[i+3])
			|| pos->a1d[i] > (box_vals[i] + box_vals[i+3]))
		{
			return false;
		}
	}

	return true;
}

int sexp_num_within_box(int n)
{
	float box_vals[6];//x,y,z,width,height,depth
	char *ship_wing;
	int i, idx;
	int retval = 0;

	for(i = 0; i < 6; i++)
	{
		box_vals[i] = i2fl(eval_num(n));
		n = CDR(n);
	}

	
	for(;n != -1; n = CDR(n))
	{
		ship_wing = CTEXT(n);

		idx = ship_name_lookup(ship_wing);
		if(idx > -1)
		{
			if(sexp_helper_is_within_box(box_vals, &Objects[Ships[idx].objnum].pos))
				retval++;
		}
		else
		{
			idx = wing_name_lookup(ship_wing);
			if(idx > -1)
			{
				bool wing_check = true;
				for(i = 0; i < Wings[idx].current_count; i++)
				{
					if(!sexp_helper_is_within_box(box_vals, &Objects[Ships[Wings[idx].ship_index[i]].objnum].pos))
					{
						wing_check = false;
						break;
					}
				}

				if(wing_check)
					retval++;
			}
		}
	}

	return retval;
}	

// Goober5000
void sexp_set_object_speed(object *objp, int speed, int axis, int subjective)
{
	Assert(axis >= 0 && axis <= 2);

	if (subjective)
	{
		vec3d subjective_vel;

		// translate objective into subjective velocity
		vm_vec_rotate(&subjective_vel, &objp->phys_info.vel, &objp->orient);

		// set it
		subjective_vel.a1d[axis] = i2fl(speed);

		// translate it back to objective
		vm_vec_unrotate(&objp->phys_info.vel, &subjective_vel, &objp->orient);
	}
	else
	{
		objp->phys_info.vel.a1d[axis] = i2fl(speed);
	}
}

// Goober5000
void sexp_set_object_speed(int n, int axis)
{
	Assert(n >= 0);

	int speed, subjective = 0;
	object_ship_wing_point_team oswpt;

	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));
	n = CDR(n);

	speed = eval_num(n);
	n = CDR(n);

	if (n >= 0)
	{
		subjective = is_sexp_true(n);
		n = CDR(n);
	}

	switch (oswpt.type)
	{
		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WING:
		case OSWPT_TYPE_WAYPOINT:
		case OSWPT_TYPE_TEAM:
		{
			sexp_set_object_speed(oswpt.objp, speed, axis, subjective);

			//CommanderDJ - we put the multiplayer callback stuff in here to prevent doing unnecessary checks clientside
			multi_start_callback();
			multi_send_object(oswpt.objp);
			multi_send_int(speed);
			multi_send_int(axis);
			multi_send_int(subjective);
			multi_end_callback();

			break;
		}
	}
}

//CommanderDJ
void multi_sexp_set_object_speed()
{
	object *objp;
	int speed = 0, axis = 0, subjective = 0;

	multi_get_object(objp);
	multi_get_int(speed);
	multi_get_int(axis);
	multi_get_int(subjective);

	sexp_set_object_speed(objp, speed, axis, subjective);
}

int sexp_get_object_speed(object *objp, int axis, int subjective)
{
	Assertion(((axis >= 0) && (axis <= 2)), "Axis is out of range (%d)", axis);
	int speed;

	if (subjective)
	{
		// return the speed based on the orentation of the object
		vec3d subjective_vel;
		vm_vec_rotate(&subjective_vel, &objp->phys_info.vel, &objp->orient);
		speed = fl2i(subjective_vel.a1d[axis]);
		vm_vec_unrotate(&objp->phys_info.vel, &subjective_vel, &objp->orient);
	}
	else
	{
		// return the speed according to the grid
		speed = fl2i(objp->phys_info.vel.a1d[axis]);
	}
	return speed;
}

int sexp_get_object_speed(int n, int axis)
{
	Assert(n >= 0);

	int speed, subjective = 0;
	object_ship_wing_point_team oswpt;

	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));
	n = CDR(n);

	if (n >= 0)
	{
		subjective = is_sexp_true(n);
		n = CDR(n);
	}

	switch (oswpt.type)
	{
		case OSWPT_TYPE_EXITED:
			return SEXP_NAN_FOREVER;

		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WING:
		case OSWPT_TYPE_WAYPOINT:
		case OSWPT_TYPE_TEAM:
			speed = sexp_get_object_speed(oswpt.objp, axis, subjective);
			break;

		default:
			return SEXP_NAN;
			break;
	}
	return speed;
}

// Goober5000
int sexp_calculate_coordinate(vec3d *origin, matrix *orient, vec3d *relative_location, int axis)
{
	Assert(origin != NULL);
	Assert(orient != NULL);
	Assert(axis >= 0 && axis <= 2);

	if (relative_location == NULL)
	{
		return fl2i(origin->a1d[axis]);
	}
	else
	{
		vec3d new_world_pos;

		vm_vec_unrotate(&new_world_pos, relative_location, orient);
		vm_vec_add2(&new_world_pos, origin);

		return fl2i(new_world_pos.a1d[axis]);
	}
}

// Goober5000
int sexp_calculate_angle(matrix *orient, int axis)
{
	Assert(orient != NULL);
	Assert(axis >= 0 && axis <= 2);

	angles a;
	vm_extract_angles_matrix(&a, orient);

	// blugh
	float rad;
	switch (axis)
	{
		case 0:	rad = a.p; break;
		case 1:	rad = a.b; break;
		case 2:	rad = a.h; break;
		default: rad = 0.0f; break;
	}

	int deg = static_cast<int>(fl_degrees(rad));
	if (deg < 0)
		deg += 360;

	return deg;
}

// Goober5000
int sexp_get_object_coordinate(int n, int axis) 
{
	Assert(n >= 0);

	char *subsystem_name = NULL;
	vec3d *pos = NULL, *relative_location = NULL, relative_location_buf, subsys_pos_buf;
	object_ship_wing_point_team oswpt;

	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));
	n = CDR(n);

	if (n >= 0)
	{
		subsystem_name = CTEXT(n);
		n = CDR(n);

		if (n >= 0)
		{
			relative_location = &relative_location_buf;

			relative_location->xyz.x = (float) eval_num(n);
			n = CDR(n);
			if (n >= 0) {
				relative_location->xyz.y = (float) eval_num(n);
				n = CDR(n);
				if (n >= 0) {
					relative_location->xyz.z = (float) eval_num(n);
					n = CDR(n);
				}
			}
		}
	}

	switch (oswpt.type)
	{
		case OSWPT_TYPE_EXITED:
			return SEXP_NAN_FOREVER;

		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WING:
		case OSWPT_TYPE_WAYPOINT:
		case OSWPT_TYPE_TEAM:
			pos = &oswpt.objp->pos;
			break;

		default:
			return SEXP_NAN;
	}

	// see if we have a subsys
	if (oswpt.objp->type == OBJ_SHIP)
	{
		if ((subsystem_name != NULL) && stricmp(subsystem_name, SEXP_NONE_STRING) && stricmp(subsystem_name, SEXP_HULL_STRING))
		{
			pos = &subsys_pos_buf;
			// get the world pos but bail if we can't get one
			if (!sexp_get_subsystem_world_pos(pos, oswpt.objp->instance, subsystem_name))
				return SEXP_NAN;
		}
	}

	return sexp_calculate_coordinate(pos, &oswpt.objp->orient, relative_location, axis);
}

// Goober5000
int sexp_get_object_angle(int n, int axis) 
{
	Assert(n >= 0);

	object_ship_wing_point_team oswpt;
	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));

	switch (oswpt.type)
	{
		case OSWPT_TYPE_EXITED:
			return SEXP_NAN_FOREVER;

		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WING:
		case OSWPT_TYPE_WAYPOINT:
		case OSWPT_TYPE_TEAM:
			return sexp_calculate_angle(&oswpt.objp->orient, axis);

		default:
			return SEXP_NAN;
	}
}

void set_object_for_clients(object *objp)
{
	if (!(Game_mode & GM_MULTIPLAYER)) {
		return;
	}

	// Tell the player (if this is a client) that they've moved.
	if ((objp->flags & OF_PLAYER_SHIP) && (objp != Player_obj) ){
		multi_oo_send_changed_object(objp);
	}
}

void sexp_set_object_position(int n) 
{
	vec3d target_vec, orig_leader_vec;
	object_ship_wing_point_team oswpt;

	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));
	n = CDR(n);

	target_vec.xyz.x = i2fl(eval_num(n));
	n = CDR(n);
	target_vec.xyz.y = i2fl(eval_num(n));
	n = CDR(n);
	target_vec.xyz.z = i2fl(eval_num(n));
	n = CDR(n);

	// retime all collision checks so they're performed
	obj_all_collisions_retime();

	// if this is a nebula mission and a player is being moved far enough,
	// regenerate the nebula
	extern neb2_detail *Nd;

	if ( (oswpt.objp == Player_obj) 
		&& (The_mission.flags & MISSION_FLAG_FULLNEB) 
		&& (vm_vec_dist(&oswpt.objp->pos, &target_vec) >= Nd->cube_inner) )
	{
		neb2_eye_changed();
	}

	switch (oswpt.type)
	{
		case OSWPT_TYPE_TEAM:
		{
			// move the first one first
			orig_leader_vec = oswpt.objp->pos;
			oswpt.objp->pos = target_vec;
			set_object_for_clients(oswpt.objp);

			// move everything on the team
			for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
			{
				object *objp = &Objects[so->objnum];

				if ((Ships[objp->instance].team == oswpt.team) && (objp != oswpt.objp))
				{
					vm_vec_sub2(&objp->pos, &orig_leader_vec);
					vm_vec_add2(&objp->pos, &target_vec);
					set_object_for_clients(objp);
				}
			}

			return;
		}

		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WAYPOINT:
		{
			oswpt.objp->pos = target_vec;
			set_object_for_clients(oswpt.objp);
			return;
		}

		case OSWPT_TYPE_WING:
		{
			// move the wing leader first
			orig_leader_vec = oswpt.objp->pos;
			oswpt.objp->pos = target_vec;
			set_object_for_clients(oswpt.objp);

			// move everything in the wing
			for (int i = 0; i < oswpt.wingp->current_count; i++)
			{
				object *objp = &Objects[Ships[oswpt.wingp->ship_index[i]].objnum];

				if (objp != oswpt.objp)
				{
					vm_vec_sub2(&objp->pos, &orig_leader_vec);
					vm_vec_add2(&objp->pos, &target_vec);
					set_object_for_clients(objp);
				}
			}

			return;
		}
	}
}

// Goober5000
void sexp_set_object_orientation(int n) 
{
	angles a;
	matrix target_orient;
	object_ship_wing_point_team oswpt;

	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));
	n = CDR(n);

	a.p = fl_radians(eval_num(n) % 360);
	n = CDR(n);
	a.b = fl_radians(eval_num(n) % 360);
	n = CDR(n);
	a.h = fl_radians(eval_num(n) % 360);
	n = CDR(n);

	vm_angles_2_matrix(&target_orient, &a);

	// retime all collision checks so they're performed
	obj_all_collisions_retime();

	switch (oswpt.type)
	{
		case OSWPT_TYPE_TEAM:
		{
			// move everything on the team
			for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
			{
				object *objp = &Objects[so->objnum];

				if (Ships[objp->instance].team == oswpt.team)
				{
					objp->orient = target_orient;
					set_object_for_clients(objp);
				}
			}

			return;
		}

		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WAYPOINT:
		{
			oswpt.objp->orient = target_orient;
			set_object_for_clients(oswpt.objp);
			return;
		}

		case OSWPT_TYPE_WING:
		{
			// move everything in the wing
			for (int i = 0; i < oswpt.wingp->current_count; i++)
			{
				object *objp = &Objects[Ships[oswpt.wingp->ship_index[i]].objnum];
				objp->orient = target_orient;
				set_object_for_clients(objp);
			}

			return;
		}
	}
}

// Goober5000
// this is different from sexp_set_object_orientation
void sexp_set_object_orient_sub(object *objp, vec3d *location, int turn_time, int bank)
{
	Assert(objp && location);

	vec3d v_orient;
	matrix m_orient;


	// are we doing this via ai? -------------------
	if (turn_time)
	{
		// set flag
		int bankflag = 0;
		if (!bank) 
		{
			bankflag = AITTV_IGNORE_BANK;
		}

		// turn
		ai_turn_towards_vector(location, objp, flFrametime, float(turn_time)/(1000.0f), NULL, NULL, 0.0f, 0, NULL, (AITTV_VIA_SEXP | bankflag));

		// return
		return;
	}


	// calculate orientation matrix ----------------
	memset(&v_orient, 0, sizeof(vec3d));

	vm_vec_sub(&v_orient, location, &objp->pos);

	if (IS_VEC_NULL_SQ_SAFE(&v_orient))
	{
		Warning(LOCATION, "error in sexp setting ship orientation: can't point to self; quitting...\n");
		return;
	}

	vm_vector_2_matrix(&m_orient, &v_orient, NULL, NULL);


	// set orientation -----------------------------
	objp->orient = m_orient;
	// Tell the player (assuming it's a client) that they've moved.
	set_object_for_clients(objp);
}

// Goober5000
void sexp_set_oswpt_facing(object_ship_wing_point_team *oswpt, vec3d *location, int turn_time = 0, int bank = 0)
{
	Assert(oswpt && location);

	switch (oswpt->type)
	{
		case OSWPT_TYPE_TEAM:
		{
			for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != GET_LAST(&Ship_obj_list); so = GET_NEXT(so))
			{
				object *objp = &Objects[so->objnum];

				if (obj_team(objp) == oswpt->team)
					sexp_set_object_orient_sub(objp, location, turn_time, bank);
			}

			break;
		}

		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WAYPOINT:
			sexp_set_object_orient_sub(oswpt->objp, location, turn_time, bank);
			break;

		case OSWPT_TYPE_WING:
		{
			for (int i = 0; i < oswpt->wingp->current_count; i++)
			{
				object *objp = &Objects[Ships[oswpt->wingp->ship_index[i]].objnum];

				sexp_set_object_orient_sub(objp, location, turn_time, bank);
			}

			break;
		}
	}
}

// Goober5000
void sexp_set_object_facing(int n, bool facing_object)
{
	vec3d *location, location_buf;
	int turn_time, bank;
	object_ship_wing_point_team oswpt1, oswpt2;

	sexp_get_object_ship_wing_point_team(&oswpt1, CTEXT(n));
	n = CDR(n);

	// ensure it's valid
	if (oswpt1.objp == NULL)
		return;

	// get location
	if (facing_object)
	{
		sexp_get_object_ship_wing_point_team(&oswpt2, CTEXT(n));
		n = CDR(n);

		// ensure it's valid
		if (oswpt2.objp == NULL)
			return;

		location = &oswpt2.objp->pos;
	}
	else
	{
		location = &location_buf;

		location->xyz.x = (float) eval_num(n);
		n = CDR(n);
		location->xyz.y = (float) eval_num(n);
		n = CDR(n);
		location->xyz.z = (float) eval_num(n);
		n = CDR(n);
	}

	// get optional turn time and bank
	turn_time = bank = 0;
	if (n != -1)
	{
		turn_time = eval_num(n);
		n = CDR(n);
	}
	if (n != -1)
	{
		bank = eval_num(n);
	}

	sexp_set_oswpt_facing(&oswpt1, location, turn_time, bank);
}

void sexp_set_ship_man(object *objp, int duration, int heading, int pitch, int bank, bool apply_all_rotate, int up, int sideways, int forward, bool apply_all_lat)
{
	if (objp->type != OBJ_SHIP)
		return;

	ship *shipp = &Ships[objp->instance];
	ai_info	*aip = &Ai_info[shipp->ai_index];
	
	aip->ai_override_timestamp = timestamp(duration);
	aip->ai_override_flags = 0;
	
	if (apply_all_rotate) {
		aip->ai_override_flags |= AIORF_FULL;
		aip->ai_override_ci.bank = bank / 100.0f;
		aip->ai_override_ci.pitch = pitch / 100.0f;
		aip->ai_override_ci.heading = heading / 100.0f;
	} else {
		if (bank != 0) {
			aip->ai_override_flags |= AIORF_ROLL;
			aip->ai_override_ci.bank = bank / 100.0f;
		}
		if (pitch != 0) {
			aip->ai_override_flags |= AIORF_PITCH;
			aip->ai_override_ci.pitch = pitch / 100.0f;
		}
		if (heading != 0) {
			aip->ai_override_flags |= AIORF_HEADING;
			aip->ai_override_ci.heading = heading / 100.0f;
		}
	}
	if (apply_all_lat) {
		aip->ai_override_flags |= AIORF_FULL_LAT;
		aip->ai_override_ci.vertical = up / 100.0f;
		aip->ai_override_ci.sideways = sideways / 100.0f;
		aip->ai_override_ci.forward = forward / 100.0f;
	} else {
		if (up != 0) {
			aip->ai_override_flags |= AIORF_UP;
			aip->ai_override_ci.vertical = up / 100.0f;
		}
		if (sideways != 0) {
			aip->ai_override_flags |= AIORF_SIDEWAYS;
			aip->ai_override_ci.sideways = sideways / 100.0f;
		}
		if (forward != 0) {
			aip->ai_override_flags |= AIORF_FORWARD;
			aip->ai_override_ci.forward = forward / 100.0f;
		}
	}
}

void sexp_set_oswpt_maneuver(object_ship_wing_point_team *oswpt, int duration, int heading, int pitch, int bank, bool apply_all_rotate, int up, int sideways, int forward, bool apply_all_lat)
{
	Assert(oswpt);

	switch (oswpt->type)
	{
		case OSWPT_TYPE_TEAM:
		{
			for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != GET_LAST(&Ship_obj_list); so = GET_NEXT(so))
			{
				object *objp = &Objects[so->objnum];

				if (obj_team(objp) == oswpt->team)
					sexp_set_ship_man(objp, duration, heading, pitch, bank, apply_all_rotate, up, sideways, forward, apply_all_lat);
			}

			break;
		}

		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WAYPOINT:
			sexp_set_ship_man(oswpt->objp, duration, heading, pitch, bank, apply_all_rotate, up, sideways, forward, apply_all_lat);
			break;

		case OSWPT_TYPE_WING:
		{
			for (int i = 0; i < oswpt->wingp->current_count; i++)
			{
				object *objp = &Objects[Ships[oswpt->wingp->ship_index[i]].objnum];

				sexp_set_ship_man(objp, duration, heading, pitch, bank, apply_all_rotate, up, sideways, forward, apply_all_lat);
			}

			break;
		}
	}
}

void sexp_set_ship_maneuver(int n, int op_num)
{
	int bank = 0, heading = 0, pitch = 0;
	int up = 0, sideways = 0, forward = 0;
	int duration, i, temp;
	bool apply_all_rotate = false, apply_all_lat = false;
	object_ship_wing_point_team oswpt;

	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));

	n = CDR(n);
	duration = eval_num(n);
	if (duration < 2)
		return;

	if (op_num == OP_SHIP_ROT_MANEUVER || op_num == OP_SHIP_MANEUVER) {
		for(i=0;i<3;i++) {
			n = CDR(n);

			temp = eval_num(n);
			if (temp > 100) temp = 100;
			if (temp < -100) temp = -100;

			if (i == 0)
				heading = temp;
			else if (i == 1)
				pitch = temp;
			else
				bank = temp;
		}

		n = CDR(n);
		apply_all_rotate = (is_sexp_true(n) != 0);
	}

	if (op_num == OP_SHIP_LAT_MANEUVER || op_num == OP_SHIP_MANEUVER) {
		for(i=0;i<3;i++) {
			n = CDR(n);

			temp = eval_num(n);
			if (temp > 100) temp = 100;
			if (temp < -100) temp = -100;

			if (i == 0)
				up = temp;
			else if (i == 1)
				sideways = temp;
			else
				forward = temp;
		}

		n = CDR(n);
		apply_all_lat = (is_sexp_true(n) != 0);
	}

	if ((bank == 0) && (pitch == 0) && (heading == 0) && !apply_all_rotate && (up == 0) && (sideways == 0) && (forward == 0) && !apply_all_lat)
		return;

	sexp_set_oswpt_maneuver(&oswpt, duration, heading, pitch, bank, apply_all_rotate, up, sideways, forward, apply_all_lat);
}

/**
 * Determine when the last meaningful order was given to one or more ships.
 * 
 * @return true or false depending on whether or not a meaningful order was received
 */
int sexp_last_order_time(int n)
{
	int instance, i;
	fix time;
	char *name;
	ai_goal *aigp;

	time = i2f(eval_num(n));
	Assert ( time >= 0 );

	n = CDR(n);
	while ( n != -1 ) {
		name = CTEXT(n);
		instance = ship_name_lookup(name);
		if ( instance != -1 ) {
			aigp = Ai_info[Ships[instance].ai_index].goals;
		} else {
			instance = wing_name_lookup(name);
			if ( instance == -1 )						// if we cannot find ship or wing, return SEXP_FALSE
				return SEXP_FALSE;
			aigp = Wings[instance].ai_goals;
		}

		// with the ship, check the ai_goals structure for this ship and determine if there are any
		// orders which are < time seconds since current mission time
		for ( i = 0; i < MAX_AI_GOALS; i++ ) {
			int mode;

			mode = aigp->ai_mode;
			if ( (mode  != AI_GOAL_NONE) && (mode != AI_GOAL_WARP) )
				if ( (aigp->time + time) > Missiontime )
					break;
			aigp++;
		}
		if ( i == MAX_AI_GOALS )
			return SEXP_TRUE;

		n = CDR(n);
	}

	return SEXP_FALSE;
}

/**
 * Return the number of players in the mission
 */
int sexp_num_players()
{
	int count;
	object *objp;

	count = 0;
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type == OBJ_SHIP) && (objp->flags & OF_PLAYER_SHIP) )
			count++;
	}

	return count;
}

/**
 * Determine if the current skill level of the game is at least the skill level given in the SEXP
 */
int sexp_skill_level_at_least(int n)
{
	int i;
	char *level_name;

	level_name = CTEXT(n);

	if (level_name == NULL)
		return SEXP_FALSE;

	for (i = 0; i < NUM_SKILL_LEVELS; i++ ) {
		if ( !stricmp(level_name, Skill_level_names(i, 0)) ) {
			if ( Game_skill_level >= i ){
				return SEXP_TRUE;
			} else {
				return SEXP_FALSE;
			}
		}
	}

	// return SEXP_FALSE if not found!!!
	return SEXP_FALSE;
}

int sexp_was_promotion_granted(int n)
{
	if (Player->flags & PLAYER_FLAGS_PROMOTED)
		return SEXP_TRUE;

	return SEXP_FALSE;
}

int sexp_was_medal_granted(int n)
{
	int i;
	char *medal_name;

	if (n < 0) {
		if (Player->stats.m_medal_earned >= 0)
			return SEXP_TRUE;

		return SEXP_FALSE;
	}

	medal_name = CTEXT(n);

	if (medal_name == NULL)
		return SEXP_FALSE;

	for (i=0; i<Num_medals; i++) {
		if (!stricmp(medal_name, Medals[i].name))
			break;
	}

	if ( (i < Num_medals) && (Player->stats.m_medal_earned == i) )
		return SEXP_TRUE;

	return SEXP_FALSE;
}

/**
 * @todo Add code to check the damage ships which have exited have taken
 */
float get_damage_caused(int damaged_ship, int attacker ) 
{
	int sindex, idx;
	float damage_total = 0.0f; 


	// is the ship that took damage in the mission still?
	sindex = ship_get_by_signature(damaged_ship);

	if (sindex >= 0 ) {
		for(idx=0; idx<MAX_DAMAGE_SLOTS; idx++){
			if (Ships[sindex].damage_ship_id[idx] == attacker) {
				damage_total += Ships[sindex].damage_ship[idx];
				break;
			}
		}
	}
	else {
		//TO DO - Add code to check the damage ships which have exited have taken
	
		sindex = ship_find_exited_ship_by_signature(damaged_ship);
		for(idx=0; idx<MAX_DAMAGE_SLOTS; idx++){
			if (Ships_exited[sindex].damage_ship_id[idx] == attacker) {
				damage_total += Ships_exited[sindex].damage_ship[idx];
				break;
			}
		}

	
	}
	return damage_total; 
}

// Karajorma
int sexp_get_damage_caused(int node) 
{
	int sindex, damaged_sig, attacker_sig; 
	float damage_caused = 0.0f;
	char	*name;
	int ship_class;

	name = CTEXT(node);
	sindex = ship_name_lookup(name); 
	if (sindex < 0) {
		// this ship may have exited already.
		sindex = ship_find_exited_ship_by_name(CTEXT(node));
		if (sindex < 0) {
			// this is probably a ship which hasn't arrived and thus can't have taken any damage yet
			return fl2i(damage_caused);
		}
		else {
			damaged_sig = Ships_exited[sindex].obj_signature;
			ship_class = Ships_exited[sindex].ship_class;
		}
	}
	else {
		damaged_sig = Objects[Ships[sindex].objnum].signature ;
		ship_class = Ships[sindex].ship_info_index;
	}


	node = CDR(node);
	Assert (node != -1);

	// go through the list of ships who we think may have attacked the ship
	for ( ; node != -1; node = CDR(node) ) {
		name = CTEXT(node);
		sindex = ship_name_lookup(name); 
		if (sindex < 0) {
			sindex = ship_find_exited_ship_by_name(name);
			attacker_sig = Ships_exited[sindex].obj_signature; 
		}
		else {
			attacker_sig = Objects[Ships[sindex].objnum].signature ;
		}

		if (attacker_sig < 0) {
			continue;
		}

		damage_caused += get_damage_caused (damaged_sig, attacker_sig);
	}
	
	Assert ((ship_class > -1) && (ship_class < MAX_SHIP_CLASSES));
	return (int) ((damage_caused/Ship_info[ship_class].max_hull_strength) * 100.0f);
}

/**
 * Returns true if the percentage of ships (and ships in wings) departed is at least the percentage given.  
 * 
 * what determine if we should check destroyed or departed status
 * Goober5000 - added disarm and disable
 */
int sexp_percent_ships_arrive_depart_destroy_disarm_disable(int n, int what)
{
	int percent;
	int total, count;
	char *name;

	percent = eval_num(n);

	total = 0;
	count = 0;
	// iterate through the rest of the ships/wings in the list and tally the departures and the
	// total
	for ( n = CDR(n); n != -1; n = CDR(n) ) {
		int wingnum;

		name = CTEXT(n);

		wingnum = wing_name_lookup( name, 1 );
		if ( wingnum != -1 ) {
			// for wings, we can increment the total by the total number of ships that we expect for
			// this wing, and the departures by the number of departures stored for this wing
			total += (Wings[wingnum].wave_count * Wings[wingnum].num_waves);
			if ( what == OP_PERCENT_SHIPS_DEPARTED )
				count += Wings[wingnum].total_departed;
			else if ( what == OP_PERCENT_SHIPS_DESTROYED )
				count += Wings[wingnum].total_destroyed;
			else if ( what == OP_PERCENT_SHIPS_ARRIVED )
				count += Wings[wingnum].total_arrived_count;
			else
				Error(LOCATION, "Invalid status check '%d' for wing '%s' in sexp_percent_ships_arrive_depart_destroy_disarm_disable", what, name);
		} else {
			// must be a ship, so increment the total by 1, then determine if this ship has departed
			total++;
			if ( what == OP_PERCENT_SHIPS_DEPARTED ) {
				if ( mission_log_get_time(LOG_SHIP_DEPARTED, name, NULL, NULL) )
					count++;
			} else if ( what == OP_PERCENT_SHIPS_DESTROYED ) {
				if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, NULL) )
					count++;
			} else if ( what == OP_PERCENT_SHIPS_DISABLED ) {
				if ( mission_log_get_time(LOG_SHIP_DISABLED, name, NULL, NULL) )
					count++;
			} else if ( what == OP_PERCENT_SHIPS_DISARMED ) {
				if ( mission_log_get_time(LOG_SHIP_DISARMED, name, NULL, NULL) )
					count++;
			} else if ( what == OP_PERCENT_SHIPS_ARRIVED ) {
				if ( mission_log_get_time(LOG_SHIP_ARRIVED, name, NULL, NULL) )
					count++;
			} else
				Error("Invalid status check '%d' for ship '%s' in sexp_percent_ships_depart_destroy_disarm_disable", what, name);

		}
	}

	// now, look at the percentage
	if ( ((count * 100) / total) >= percent )
		return SEXP_KNOWN_TRUE;
	else
		return SEXP_FALSE;
}

/**
 * Determine if a list of ships has departed from within a radius of a given jump node.
 * @return true N seconds after the list of ships have departed
 */
int sexp_depart_node_delay(int n)
{
	int delay, count, num_departed;
	char *jump_node_name, *name;
	fix latest_time, this_time;

	Assert( n >= 0 );

	delay = eval_num(n);
	n = CDR(n);
	jump_node_name = CTEXT(n);

	// iterate through the list of ships
	n = CDR(n);
	latest_time = 0;
	count = 0;
	num_departed = 0;
	while ( n != -1 ) {
		count++;
		name = CTEXT(n);

		if (sexp_query_has_yet_to_arrive(name))
			return SEXP_CANT_EVAL;

		// if ship/wing destroyed, sexpression is known false.  Also, if there is no departure log entry, then
		// the sexpression is not true.
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_SHIP_DEPARTED, name, jump_node_name, &this_time) ) {
			num_departed++;
			if ( this_time > latest_time )
				latest_time = this_time;
		}
		n = CDR(n);
	}

	if ( (count == num_departed) && ((Missiontime - latest_time) >= delay) )
		return SEXP_KNOWN_TRUE;
	else
		return SEXP_FALSE;
}

/**
 * Returns true when the listed ships/wings have all been destroyed or have departed.
 */
int sexp_destroyed_departed_delay(int n)
{
	int count, total;
	fix delay, latest_time;
	char *name;

	Assert( n >= 0 );

	// get the delay
	delay = i2f(eval_num(n));
	n = CDR(n);

	count = 0;					// number destroyed or departed
	total = 0;					// total number of ships/wings to check
	latest_time = 0;
	while ( n != -1 ) {
		int wingnum;
		fix time_gone = 0;

		total++;
		name = CTEXT(n);

		// for wings, check the WF_GONE flag to see if there are no more ships in this wing to arrive.
		wingnum = wing_name_lookup(name, 1);
		if ( wingnum != -1 ) {
			if ( Wings[wingnum].flags & WF_WING_GONE ) {
				// be sure to get the latest time of one of these 
				if ( Wings[wingnum].time_gone > latest_time ){
					time_gone = Wings[wingnum].time_gone;
				}
				count++;
			}
		} else if ( mission_log_get_time(LOG_SHIP_DEPARTED, name, NULL, &time_gone) ) {
			count++;
		} else if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, &time_gone) ) {
			count++;
		} else if ( mission_log_get_time(LOG_SELF_DESTRUCTED, name, NULL, &time_gone) ) {
			count++;
		}

		// check our latest time
		if ( time_gone > latest_time ){
			latest_time = time_gone;
		}

		n = CDR(n);
	}

	if ( (count == total) && (Missiontime > (latest_time + delay)) )
		return SEXP_KNOWN_TRUE;
	else
		return SEXP_FALSE;
}

int sexp_special_warpout_name( int node )
{
	int shipnum, knossos_shipnum;
	char *ship_name, *knossos;

	ship_name = CTEXT(node);
	knossos = CTEXT(CDR(node));

	// check to see if either ship was destroyed or departed.  If so, then make this node known
	// false
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPARTED, ship_name, NULL, NULL) ||
		  mission_log_get_time(LOG_SHIP_DESTROYED, knossos, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPARTED, knossos, NULL, NULL) ) 
		return SEXP_NAN_FOREVER;

	// get ship name
	shipnum = ship_name_lookup(ship_name);
	if (shipnum < 0) {
		return SEXP_NAN;
	}

	// get knossos ship
	knossos_shipnum = ship_name_lookup(knossos);
	if (knossos_shipnum < 0) {
		return SEXP_NAN;
	}

	// set special warpout objnum
	Ships[shipnum].special_warpout_objnum = Ships[knossos_shipnum].objnum;
	return SEXP_FALSE;
}

/**
 * Determines if N seconds have elapsed since all discovery of all cargo of given ships
 *
 * Goober5000 - I reworked this function to allow for the set-scanned and set-unscanned sexps
 * to work multiple times in a row and also to fix the potential bug where exited ships are
 * checked against their departure time, not against their cargo known time
 */
int sexp_is_cargo_known( int n, int check_delay )
{
	int count, ship_num, num_known, delay;

	char *name;

	Assert ( n >= 0 );

	count = 0;
	num_known = 0;

	// get the delay value (if there is one)
	delay = 0;
	if ( check_delay )
	{
		delay = eval_num(n);
		n = CDR(n);
	}

	while ( n != -1 )
	{
		fix time_known;
		int is_known;

		is_known = 0;

		count++;

		// see if we have already checked this entry
		if ( Sexp_nodes[n].value == SEXP_KNOWN_TRUE )
		{
			num_known++;
		}
		else
		{
			name = CTEXT(n);

			// find the index in the ship array (will be -1 if not in mission)
			ship_num = ship_name_lookup(name);

			// see if the ship has already exited the mission (either through departure or destruction)
			int exited_index = ship_find_exited_ship_by_name(name);
			if (exited_index != -1)
			{
				// if not known, the whole thing is known false
				if ( !(Ships_exited[exited_index].flags & SEF_CARGO_KNOWN) )
					return SEXP_KNOWN_FALSE;

				// check the delay of when we found out
				time_known = Missiontime - Ships_exited[exited_index].time_cargo_revealed;
				if ( f2i(time_known) >= delay )
				{
					is_known = 1;

					// here is the only place in the new sexp that this can be known true
					Sexp_nodes[n].value = SEXP_KNOWN_TRUE;
				}
			}
			// ship either in mission or not arrived yet
			else
			{
				// if ship_name_lookup returns -1, then ship is either exited or yet to arrive,
				// and we've already checked exited
				if ( ship_num != -1 )
				{
					if ( Ships[ship_num].flags & SF_CARGO_REVEALED )
					{
						time_known = Missiontime - Ships[ship_num].time_cargo_revealed;
						if ( f2i(time_known) >= delay )
						{
							is_known = 1;
						}
					}
				}
			}
		}

		// if cargo is known, mark our variable, but not the sexp, because it may change later
		if ( is_known )
		{
			num_known++;
		}

		n = CDR(n);
	}

	Directive_count += count - num_known;
	if ( count == num_known )
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

void get_cap_subsys_cargo_flags(int shipnum, char *subsys_name, int *known, fix *time_revealed)
{
	// find the ship subsystem
	ship_subsys *ss = ship_get_subsys(&Ships[shipnum], subsys_name);
	if (ss != NULL)
	{
		// set the flags
		*known = (ss->flags & SSF_CARGO_REVEALED);
		*time_revealed = ss->time_subsys_cargo_revealed;
	}
	// if we didn't find the subsystem, the ship hasn't arrived yet
	else
	{
		*known = -1;
		*time_revealed = 0;
	}
}

// reworked by Goober5000 to allow for set-scanned and set-unscanned to be used more than once
int sexp_cap_subsys_cargo_known_delay(int n)
{
	int delay, count, num_known, ship_num;
	char *ship_name, *subsys_name;

	num_known = 0;
	count = 0;

	Assert( n >= 0 );

	// get delay
	delay = eval_num(n);
	n = CDR(n);

	// get ship name
	ship_name = CTEXT(n);
	n = CDR(n);

	// find the index in the ship array
	ship_num = ship_name_lookup(ship_name);

	while ( n != -1 )
	{
		fix time_known;
		int is_known;

		is_known = 0;
		count++;

		// see if we have already checked this entry
		if ( Sexp_nodes[n].value == SEXP_KNOWN_TRUE )
		{
			num_known++;
		}
		else
		{
			// get subsys name
			subsys_name = CTEXT(n);

			// see if the ship has already exited the mission (either through departure or destruction)
			if (ship_find_exited_ship_by_name(ship_name) != -1)
			{
				// check the delay of when we found out...
				// Since there is no way to keep track of subsystem status once a ship has departed
				// or has been destroyed, check the mission log.  This will work in 99.9999999% of
				// all cases; however, if the mission designer repeatedly sets and resets the scanned
				// status of the subsystem, the mission log will only return the first occurrence of the
				// subsystem cargo being revealed (regardless of whether it was first hidden using
				// set-unscanned).  Normally, ships keep track of cargo data in the subsystem struct,
				// but once/ the ship has left the mission, the subsystem linked list is purged,
				// causing the loss of this information.  I judged the significant rework of the
				// subsystem code not worth the rare instance that this sexp may be required to work
				// in this way, especially since this problem only occurs after the ship departs.  If
				// the mission designer really needs this functionality, he or she can achieve the
				// same result with creative combinations of event chaining and is-event-true.
				if (!mission_log_get_time(LOG_CAP_SUBSYS_CARGO_REVEALED, ship_name, subsys_name, &time_known))
				{
					// if not known, the whole thing is known false
					return SEXP_KNOWN_FALSE;
				}

				if (f2i(Missiontime - time_known) >= delay)
				{
					is_known = 1;

					// here is the only place in the new sexp that this can be known true
					Sexp_nodes[n].value = SEXP_KNOWN_TRUE;
				}
			}
			// ship either in mission or not arrived yet
			else
			{
				// if ship_name_lookup returns -1, then ship is either exited or yet to arrive,
				// and we've already checked exited
				if ( ship_num != -1 )
				{
					int cargo_revealed(0);
					fix time_revealed(0);

					// get flags
					get_cap_subsys_cargo_flags(ship_num, subsys_name, &cargo_revealed, &time_revealed);

					if (cargo_revealed)
					{
						time_known = Missiontime - time_revealed;
						if ( f2i(time_known) >= delay )
						{
							is_known = 1;
						}
					}
				}
			}
		}

		// if cargo is known, mark our variable, but not the sexp, because it may change later
		if (is_known)
		{
			num_known++;
		}

		n = CDR(n);
	}

	Directive_count += count - num_known;
	if ( count == num_known )
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

// Goober5000
void sexp_set_scanned_unscanned(int n, int flag)
{
	char *ship_name, *subsys_name;
	int shipnum;
	ship_subsys *ss;

	// get ship name
	ship_name = CTEXT(n);

	// check to see the ship was destroyed or departed - if so, do nothing
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPARTED, ship_name, NULL, NULL) )
	{
		return;
	}

	// get ship number
	shipnum = ship_name_lookup(ship_name);

	// if the ship isn't in the mission, do nothing
	if (shipnum == -1)
	{
		return;
	}

	// check for possible next optional argument: subsystem
	n = CDR(n);

	// if no subsystem specified, just do it for the ship and exit
	if (n == -1)
	{
		if (flag)
			ship_do_cargo_revealed(&Ships[shipnum]);
		else
			ship_do_cargo_hidden(&Ships[shipnum]);

		return;
	}

	// iterate through all subsystems
	while (n != -1)
	{
		subsys_name = CTEXT(n);

		// find the ship subsystem
		ss = ship_get_subsys(&Ships[shipnum], subsys_name);
		if (ss != NULL)
		{
			// do it for the subsystem
			if (flag)
				ship_do_cap_subsys_cargo_revealed(&Ships[shipnum], ss);
			else
				ship_do_cap_subsys_cargo_hidden(&Ships[shipnum], ss);
		}

		// if we didn't find the subsystem -- bad
		if (ss == NULL && ship_class_unchanged(shipnum)) {
			Error(LOCATION, "Couldn't find subsystem '%s' on ship '%s' in sexp_set_scanned_unscanned", subsys_name, ship_name);
		}

		// but if it did, loop again
		n = CDR(n);
	}
}

int sexp_has_been_tagged_delay(int n)
{
	int count, shipnum, num_known, delay;
	char *name;

	Assert ( n >= 0 );

	count = 0;
	num_known = 0;

	// get the delay value
	delay = eval_num(n);

	n = CDR(n);

	while ( n != -1 ) {
		fix time_known;
		int is_known;

		is_known = 0;

		count++;

		// see if we have already checked this entry
		if ( Sexp_nodes[n].value == SEXP_KNOWN_TRUE ) {
			num_known++;
		} else {
			int exited_index;

			name = CTEXT(n);

			// see if the ship has already exited the mission (either through departure or destruction).  If so,
			// grab the status of whether the cargo is known from this list
			exited_index = ship_find_exited_ship_by_name( name );
			if (exited_index != -1 ) {
				if ( !(Ships_exited[exited_index].flags & SEF_BEEN_TAGGED) )
					return SEXP_KNOWN_FALSE;

				// check the delay of when we found out.  We use the ship died time which isn't entirely accurate
				// but won't cause huge delays.
				time_known = Missiontime - Ships_exited[exited_index].time;
				if ( f2i(time_known) >= delay )
					is_known = 1;
			} else {

				// otherwise, ship should still be in the mission.  If ship_name_lookup returns -1, then ship
				// is yet to arrive.
				shipnum = ship_name_lookup( name );
				if ( shipnum != -1 ) {
					if ( Ships[shipnum].time_first_tagged != 0 ) {
						time_known = Missiontime - Ships[shipnum].time_first_tagged;
						if ( f2i(time_known) >= delay )
							is_known = 1;
					}
				}
			}
		}

		// if cargo is known, mark our variable and this sexpression.
		if ( is_known ) {
			num_known++;
			Sexp_nodes[n].value = SEXP_KNOWN_TRUE;
		}

		n = CDR(n);
	}

	Directive_count += count - num_known;
	if ( count == num_known )
		return SEXP_KNOWN_TRUE;
	else
		return SEXP_FALSE;
}

// Karajorma
void eval_when_for_each_special_argument( int cur_node )
{
	arg_item *ptr;

	// loop through all the supplied arguments
	ptr = Sexp_applicable_argument_list.get_next();
	while (ptr != NULL)
	{
		// acquire argument to be used
		Sexp_replacement_arguments.push_back(ptr->text);	

		Sexp_current_argument_nesting_level++;
		Sexp_applicable_argument_list.add_data(ptr->text);

		// execute sexp... CTEXT will insert the argument as necessary
		eval_sexp(cur_node);
		
		// clean up any special sexp stuff
		Sexp_applicable_argument_list.clear_nesting_level();
		Sexp_current_argument_nesting_level--;

		// remove the argument 
		Sexp_replacement_arguments.pop_back(); 

		// continue along argument list
		ptr = ptr->get_next();
	}
}


// Goober5000
void do_action_for_each_special_argument( int cur_node )
{
	arg_item *ptr;

	// loop through all the supplied arguments
	ptr = Sexp_applicable_argument_list.get_next();
	while (ptr != NULL)
	{
		// acquire argument to be used
		Sexp_replacement_arguments.push_back(ptr->text);

		// execute sexp... CTEXT will insert the argument as necessary
		// (since these are all actions, they don't return any meaningful values)
		eval_sexp(cur_node);

		// remove the argument 
		Sexp_replacement_arguments.pop_back(); 
		// continue along argument list
		ptr = ptr->get_next();
	}
}

// Goober5000
int special_argument_appears_in_sexp_tree(int node)
{
	// empty tree
	if (node < 0)
		return 0;

	// special argument?
	if (!strcmp(Sexp_nodes[node].text, SEXP_ARGUMENT_STRING))
		return 1;

	// we don't want to include special arguments if they are nested in a new argument SEXP
	if (Sexp_nodes[node].type == SEXP_ATOM && Sexp_nodes[node].subtype == SEXP_ATOM_OPERATOR) {
		if (is_blank_argument_op(get_operator_const(CTEXT(node)))) {
			return 0; 
		}
	}

	return special_argument_appears_in_sexp_tree(CAR(node))
		|| special_argument_appears_in_sexp_tree(CDR(node));
}

// Goober5000
int special_argument_appears_in_sexp_list(int node)
{
	// look through list
	while (node != -1)
	{
		// special argument?
		if (!strcmp(Sexp_nodes[node].text, SEXP_ARGUMENT_STRING))
			return 1;

		node = CDR(node);
	}

	return 0;
}

// conditional sexpressions follow

// Goober5000
void eval_when_do_one_exp(int exp)
{	
	arg_item *ptr;
	int do_node;

	switch (get_operator_const(CTEXT(exp)))
	{
		// if the op is a conditional then we just evaluate it
		case OP_WHEN:
		case OP_EVERY_TIME:
		case OP_IF_THEN_ELSE:
			// need to account for the possibility this call uses <arguments>
			if (special_argument_appears_in_sexp_tree(exp)) { 
				ptr = Sexp_applicable_argument_list.get_next();
				if (ptr != NULL) {
					eval_when_for_each_special_argument(exp);
				}
				else {
					eval_sexp(exp);
				}
			}
			else {
				eval_sexp(exp);
			}
			break;

		case OP_DO_FOR_VALID_ARGUMENTS:
			if (special_argument_appears_in_sexp_tree(exp)) { 
				Warning(LOCATION, "<Argument> used within Do-for-valid-arguments SEXP. Skipping entire SEXP"); 
				break; 
			}

			do_node = CDR(exp); 
			while (do_node != -1) {
				do_action_for_each_special_argument(do_node); 
				do_node = CDR(do_node); 
			}
			break;

		case OP_WHEN_ARGUMENT:
		case OP_EVERY_TIME_ARGUMENT:
			eval_sexp(exp);
			break; 

		// otherwise we need to check if arguments are used
		default: 
			// if we're using the special argument in this action
			if (special_argument_appears_in_sexp_tree(exp))
			{
				do_action_for_each_special_argument(exp);			// these sexps eval'd only for side effects
			}
			// if not, just evaluate it once as-is
			else
			{
				// Goober5000 - possible bug? (see when val is used below)
				/*val = */eval_sexp(exp);							// these sexps eval'd only for side effects
			}
	}
}


// Karajorma
void eval_when_do_all_exp(int all_actions, int when_op_num)
{
	arg_item *ptr;
	int exp;
	int actions; 
	int op_num;

	bool first_loop = true;

	// loop through all the supplied arguments
	ptr = Sexp_applicable_argument_list.get_next();

	while (ptr != NULL)
	{
		// acquire argument to be used
		Sexp_replacement_arguments.push_back(ptr->text);
		actions = all_actions; 

		while (actions != -1)
		{	
			exp = CAR(actions);	

			op_num = get_operator_const(CTEXT(exp));

			if (op_num == OP_DO_FOR_VALID_ARGUMENTS) {
				int do_node = CDR(exp); 
				while (do_node != -1) {
					eval_sexp(do_node); 
					do_node = CDR(do_node); 
				}
			}
			else if ( first_loop || special_argument_appears_in_sexp_tree(exp) ) {
				switch (op_num)
				{
					// if the op is a conditional we have to make sure that it can access arguments
					case OP_WHEN:
					case OP_EVERY_TIME:
					case OP_IF_THEN_ELSE:				
						Sexp_current_argument_nesting_level++;
						Sexp_applicable_argument_list.add_data(ptr->text);
						eval_sexp(exp);
						Sexp_applicable_argument_list.clear_nesting_level();
						Sexp_current_argument_nesting_level--;
						break;

					default:
						eval_sexp(exp);
				}
			}
			
			// iterate
			actions = CDR(actions);

			// if-then-else only has one "if" action
			if (when_op_num == OP_IF_THEN_ELSE)
				break;
		}
		
		first_loop = false;

		// remove the argument 
		Sexp_replacement_arguments.pop_back(); 
		// continue along argument list
		ptr = ptr->get_next();
	}
}
	
/**
 * Evaluates the when conditional
 *
 * @note Goober5000 - added capability for arguments
 * @note Goober5000 - and also if-then-else and perform-actions
 */
int eval_when(int n, int when_op_num)
{
	int cond, val, actions;
	Assert( n >= 0 );

	// get the parts of the sexp and evaluate the conditional
	if (is_blank_argument_op(when_op_num))
	{
		int arg_handler = CAR(n);
		cond = CADR(n);
		actions = CDDR(n);

		Sexp_current_argument_nesting_level++;
		// evaluate for custom arguments
		val = eval_sexp(arg_handler, cond);
	}
	// normal evaluation
	else
	{
		cond = CAR(n);
		actions = CDR(n);

		// evaluate just as-is
		val = eval_sexp(cond);
	}


	// if value is true, perform the actions in the 'then' part
	if (val == SEXP_TRUE || val == SEXP_KNOWN_TRUE || when_op_num == OP_PERFORM_ACTIONS)
	{
		// get the operator
		int exp = CAR(actions);

		// if the mod.tbl setting is in effect we want to each evaluate all the SEXPs for 
		// each argument	
		if (True_loop_argument_sexps && special_argument_appears_in_sexp_tree(exp)) {	
			if (exp != -1) {
				eval_when_do_all_exp(actions, when_op_num);
			}
		}
		// without the mod.tbl setting (or if there are no arguments in this SEXP) we loop 
		// through every action performing them for all arguments
		else {
			while (actions != -1)
			{
				// get the operator
				exp = CAR(actions);
				if (exp != -1)
					eval_when_do_one_exp(exp);

				// iterate
				actions = CDR(actions);

				// if-then-else only has one "if" action
				if (when_op_num == OP_IF_THEN_ELSE)
					break;
			}
		}
	}
	// if-then-else has actions to perform under "else"
	else if ((val == SEXP_FALSE || val == SEXP_KNOWN_FALSE) && when_op_num == OP_IF_THEN_ELSE)
	{
		// skip past the "if" action
		actions = CDR(actions);

		// loop through every action
		while (actions != -1)
		{
			// get the operator
			int exp = CAR(actions);
			if (exp != -1)
				eval_when_do_one_exp(exp);

			// iterate
			actions = CDR(actions);
		}

		// invert val so that we behave like a when with opposite results
		if (val == SEXP_KNOWN_FALSE)
			val = SEXP_KNOWN_TRUE;
		else
			val = SEXP_TRUE;
	}

	if (is_blank_argument_op(when_op_num))
	{
		// clean up any special sexp stuff
		Sexp_applicable_argument_list.clear_nesting_level();
		Sexp_current_argument_nesting_level--;
	}

	// perform-actions should return whatever val was, but should not return known-*
	if (when_op_num == OP_PERFORM_ACTIONS)
	{
		if (val == SEXP_KNOWN_TRUE)
			return SEXP_TRUE;
		else if (val == SEXP_KNOWN_FALSE)
			return SEXP_FALSE;
		else
			return val;
	}

	if (Sexp_nodes[cond].value == SEXP_KNOWN_FALSE)
		return SEXP_KNOWN_FALSE;  // no need to waste time on this anymore

	if (val == SEXP_KNOWN_FALSE)
		return SEXP_FALSE;  // can't return known false, as this would bypass future actions under the when

	return val;
}

/**
 * Evaluate the conditional
 */
int eval_cond(int n)
{
	int cond = 0, node, val = SEXP_FALSE;

	Assert (n >= 0);
	while (n >= 0)
	{
		node = CAR(n);
		cond = CAR(node);
		val = eval_sexp(cond);

		// if the conditional evaluated to true, then we must evaluate the rest of the expression returning
		// the value of this evaluation
		if (val == SEXP_TRUE || val == SEXP_KNOWN_TRUE)
		{
			int actions, exp;

			val = SEXP_FALSE;
			actions = CDR(node);
			while (actions >= 0)
			{
				exp = CAR(actions);
				if (exp >= -1)
					val = eval_sexp(exp);								// these sexp evaled only for side effects

				actions = CDR(actions);
			}

			break;
		}

		// move onto the next cond clause
		n = CDR(n);
	}

	return val;
}

// Goober5000
// NOTE: if you change this function, check to see if the following function should also be changed!
int test_argument_nodes_for_condition(int n, int condition_node, int *num_true, int *num_false, int *num_known_true, int *num_known_false)
{
	int val, num_valid_arguments;
	Assert(n != -1 && condition_node != -1);
	Assert((num_true != NULL) && (num_false != NULL) && (num_known_true != NULL) && (num_known_false != NULL));

	// ensure special argument list is empty
	Sexp_applicable_argument_list.clear_nesting_level();
	Applicable_arguments_temp.clear();

	// ditto for counters
	num_valid_arguments = 0;
	*num_true = 0;
	*num_false = 0;
	*num_known_true = 0;
	*num_known_false = 0;

	// loop through all arguments
	while (n != -1)
	{
		// only eval this argument if it's valid
		if (Sexp_nodes[n].flags & SNF_ARGUMENT_VALID)
		{
			// flush conditional to avoid short-circuiting
			flush_sexp_tree(condition_node);

			// evaluate conditional for current argument
			Sexp_replacement_arguments.push_back(Sexp_nodes[n].text);
			val = eval_sexp(condition_node);

			switch (val)
			{
				case SEXP_TRUE:
					(*num_true)++;
					Applicable_arguments_temp.push_back(Sexp_nodes[n].text);
					break;

				case SEXP_FALSE:
					(*num_false)++;
					break;

				case SEXP_KNOWN_TRUE:
					(*num_known_true)++;
					Applicable_arguments_temp.push_back(Sexp_nodes[n].text);
					break;

				case SEXP_KNOWN_FALSE:
					(*num_known_false)++;
					break;
			}

			// clear argument, but not list, as we'll need it later
			Sexp_replacement_arguments.pop_back();

			// increment
			num_valid_arguments++;
		}

		// continue along argument list
		n = CDR(n);
	}

	// now we write from the temporary store into the real one, reversing the order. We do this because 
	// Sexp_applicable_argument_list is a stack and we want the first argument in the list to be the first one out
	while (!Applicable_arguments_temp.empty())
	{
		Sexp_applicable_argument_list.add_data(Applicable_arguments_temp.back());
		Applicable_arguments_temp.pop_back(); 
	}

	return num_valid_arguments;
}

// Goober5000
// NOTE: if you change this function, check to see if the previous function should also be changed!
int test_argument_vector_for_condition(SCP_vector<char*> argument_vector, bool already_dupped, int condition_node, int *num_true, int *num_false, int *num_known_true, int *num_known_false)
{
	int val, num_valid_arguments;
	Assert(condition_node != -1);
	Assert((num_true != NULL) && (num_false != NULL) && (num_known_true != NULL) && (num_known_false != NULL));

	// ensure special argument list is empty
	Sexp_applicable_argument_list.clear_nesting_level();
	Applicable_arguments_temp.clear();

	// ditto for counters
	num_valid_arguments = 0;
	*num_true = 0;
	*num_false = 0;
	*num_known_true = 0;
	*num_known_false = 0;

	// loop through all arguments
	for (size_t i = 0; i < argument_vector.size(); i++)
	{
		// since we can't see or modify the validity, assume all are valid
		{
			// flush conditional to avoid short-circuiting
			flush_sexp_tree(condition_node);

			// evaluate conditional for current argument
			Sexp_replacement_arguments.push_back(argument_vector[i]);
			val = eval_sexp(condition_node);

			switch (val)
			{
				case SEXP_TRUE:
					(*num_true)++;
					Applicable_arguments_temp.push_back(argument_vector[i]);
					break;

				case SEXP_FALSE:
					(*num_false)++;
					break;

				case SEXP_KNOWN_TRUE:
					(*num_known_true)++;
					Applicable_arguments_temp.push_back(argument_vector[i]);
					break;

				case SEXP_KNOWN_FALSE:
					(*num_known_false)++;
					break;
			}

			// if the argument was already dup'd, but not added as an applicable argument,
			// we need to free it here before we cause a memory leak
			if ((val == SEXP_FALSE || val == SEXP_KNOWN_FALSE) && already_dupped)
				free(argument_vector[i]);

			// clear argument, but not list, as we'll need it later
			Sexp_replacement_arguments.pop_back();

			// increment
			num_valid_arguments++;
		}
	}

	// now we write from the temporary store into the real one, reversing the order. We do this because 
	// Sexp_applicable_argument_list is a stack and we want the first argument in the list to be the first one out
	while (!Applicable_arguments_temp.empty())
	{
		// we need to dup the strings regardless, since we're not using Sexp_nodes[n].text as a string buffer,
		// but we need to know whether the calling function dup'd them, or whether we should dup them here
		if (already_dupped)
			Sexp_applicable_argument_list.add_data_set_dup(Applicable_arguments_temp.back());
		else
			Sexp_applicable_argument_list.add_data_dup(Applicable_arguments_temp.back());

		Applicable_arguments_temp.pop_back(); 
	}

	return num_valid_arguments;
}

// Goober5000
int eval_any_of(int arg_handler_node, int condition_node)
{
	int n, num_valid_arguments, num_true, num_false, num_known_true, num_known_false;
	Assert(arg_handler_node != -1 && condition_node != -1);

	// the arguments should just be data, not operators, so we can skip the CAR
	n = CDR(arg_handler_node);

	// test the whole argument list
	num_valid_arguments = test_argument_nodes_for_condition(n, condition_node, &num_true, &num_false, &num_known_true, &num_known_false);

	// use the sexp_or algorithm
	if (num_known_true)
		return SEXP_KNOWN_TRUE;
	else if (num_known_false == num_valid_arguments)
		return SEXP_KNOWN_FALSE;
	else if (num_true)
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

// Goober5000
int eval_every_of(int arg_handler_node, int condition_node)
{
	int n, num_valid_arguments, num_true, num_false, num_known_true, num_known_false;
	Assert(arg_handler_node != -1 && condition_node != -1);

	// the arguments should just be data, not operators, so we can skip the CAR
	n = CDR(arg_handler_node);

	// test the whole argument list
	num_valid_arguments = test_argument_nodes_for_condition(n, condition_node, &num_true, &num_false, &num_known_true, &num_known_false);

	// use the sexp_and algorithm
	if (num_known_false)
		return SEXP_KNOWN_FALSE;
	else if (num_known_true == num_valid_arguments)
		return SEXP_KNOWN_TRUE;
	else if (num_false)
		return SEXP_FALSE;
	else
		return SEXP_TRUE;
}

// Goober5000
int eval_number_of(int arg_handler_node, int condition_node)
{
	int n, num_valid_arguments, num_true, num_false, num_known_true, num_known_false, threshold;
	Assert(arg_handler_node != -1 && condition_node != -1);

	// the arguments should just be data, not operators, so we can skip the CAR
	n = CDR(arg_handler_node);

	// the first argument is the number threshold
	threshold = eval_num(n);
	n = CDR(n);

	// test the whole argument list
	num_valid_arguments = test_argument_nodes_for_condition(n, condition_node, &num_true, &num_false, &num_known_true, &num_known_false);

	// use the sexp_or algorithm, modified
	// (true if at least threshold arguments are true)
	if (num_known_true >= threshold)
		return SEXP_KNOWN_TRUE;
	else if (num_valid_arguments - num_known_false < threshold)
		return SEXP_KNOWN_FALSE;
	else if (num_true + num_known_true >= threshold)
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

// Goober5000
// this works a little differently... we randomly pick one argument to use
// for our condition, but this argument must be saved among sexp calls...
// so we select an argument and set its flag
// addendum: hook karajorma's random-multiple-of into the same sexp
int eval_random_of(int arg_handler_node, int condition_node, bool multiple)
{
	int n = -1, i, val, num_valid_args, random_argument;
	Assert(arg_handler_node != -1 && condition_node != -1);

	// find which argument we picked, if we picked one
	if (!multiple)
	{
		n = CDR(arg_handler_node);

		// iterate to the argument we previously selected
		for ( ; n != -1; n = CDR(n))
		{
			if (Sexp_nodes[n].flags & SNF_ARGUMENT_SELECT)
				break;
		}
	}

	// if argument not found (or never specified in the first place), we have to pick one
	if (n == -1)
	{
		n = CDR(arg_handler_node);

		// get the number of valid arguments
		num_valid_args = query_sexp_args_count(arg_handler_node, true);
		if (num_valid_args == 0)
		{
			return SEXP_FALSE;
		}

		// pick an argument and iterate to it
		random_argument = rand_internal(1, num_valid_args);
		i = 0;
		while (true)
		{
			Assert(n >= 0);

			// count only valid arguments
			if (Sexp_nodes[n].flags & SNF_ARGUMENT_VALID)
				i++;

			// if we're at the right one, we're done
			if (i >= random_argument)
				break;

			// iterate
			n = CDR(n);
		}

		// save it, if we're saving
		if (!multiple)
		{
			Sexp_nodes[n].flags |= SNF_ARGUMENT_SELECT;
		}
	}

	// only eval this argument if it's valid
	val = SEXP_FALSE;
	if (Sexp_nodes[n].flags & SNF_ARGUMENT_VALID)
	{
		// flush stuff
		Sexp_applicable_argument_list.clear_nesting_level();
		flush_sexp_tree(condition_node);

		// evaluate conditional for current argument
		Sexp_replacement_arguments.push_back(Sexp_nodes[n].text);
		val = eval_sexp(condition_node);

		// true?
		if (val == SEXP_TRUE || val == SEXP_KNOWN_TRUE)
		{
			Sexp_applicable_argument_list.add_data(Sexp_nodes[n].text);
		}
		
		// clear argument, but not list, as we'll need it later
		Sexp_replacement_arguments.pop_back();
	}

	// true if our selected argument is true
	return val;
}

// Karajorma - this conditional returns the first valid option on its list. 
int eval_in_sequence(int arg_handler_node, int condition_node)
{
	int val = SEXP_FALSE;
	int n = -1 ;
	
	Assert(arg_handler_node != -1 && condition_node != -1);

	// get the first argument
	n = CDR(arg_handler_node);
	Assert (n != -1);

	// loop through the nodes until we find one that is holds a valid argument or run out of nodes
	for (int i=1 ; i<query_sexp_args_count(arg_handler_node) ; i++)
	{
		if (!(Sexp_nodes[n].flags & SNF_ARGUMENT_VALID)) {
			n = CDR(n) ;
		}
		// if we've found a valid node there is no need to continue
		else {
			break; 
		}
	}

	// Only execute if the argument is valid (if all nodes were invalid we would still reach this point)
	if (Sexp_nodes[n].flags & SNF_ARGUMENT_VALID)
	{
		// flush stuff
		Sexp_applicable_argument_list.clear_nesting_level();
		flush_sexp_tree(condition_node);

		// evaluate conditional for current argument
		Sexp_replacement_arguments.push_back(Sexp_nodes[n].text);
		val = eval_sexp(condition_node);

		// true?
		if (val == SEXP_TRUE || val == SEXP_KNOWN_TRUE)
		{
			Sexp_applicable_argument_list.add_data(Sexp_nodes[n].text);
		}

		// clear argument, but not list, as we'll need it later
		Sexp_replacement_arguments.pop_back();
	}

	// return the value of the conditional
	return val;
}

// is there a better place to put this?  seems useful...
// credit to http://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
template <typename T>
T sign(T t) 
{
    if (t == 0)
        return T(0);
    else
        return (t < 0) ? T(-1) : T(1);
}

// Goober5000
int eval_for_counter(int arg_handler_node, int condition_node)
{
	int n, num_valid_arguments, num_true, num_false, num_known_true, num_known_false;
	int i, counter_start, counter_stop, counter_step;
	char buf[NAME_LENGTH];
	Assert(arg_handler_node != -1 && condition_node != -1);

	// determine the counter parameters
	n = CDR(arg_handler_node);
	counter_start = eval_num(n);
	n = CDR(n);
	counter_stop = eval_num(n);
	n = CDR(n);
	counter_step = (n >= 0) ? eval_num(n) : 1;

	// a bunch of error checking
	if (counter_step == 0)
	{
		Warning(LOCATION, "A counter increment of 0 is illegal!  (start=%d, stop=%d, increment=%d)", counter_start, counter_stop, counter_step);
		return SEXP_KNOWN_FALSE;
	}
	else if (counter_start == counter_stop)
	{
		Warning(LOCATION, "The counter start and stop values are identical!  (start=%d, stop=%d, increment=%d)", counter_start, counter_stop, counter_step);
		return SEXP_KNOWN_FALSE;
	}
	else if (sign(counter_stop - counter_start) != sign(counter_step))
	{
		Warning(LOCATION, "The counter cannot complete with the given values!  (start=%d, stop=%d, increment=%d)", counter_start, counter_stop, counter_step);
		return SEXP_KNOWN_FALSE;
	}

	// build a vector of counter values
	SCP_vector<char*> argument_vector;
	for (i = counter_start; ((counter_step > 0) ? i <= counter_stop : i >= counter_stop); i += counter_step)
	{
		sprintf(buf, "%d", i);
		argument_vector.push_back(strdup(buf));
	}

	// test the whole argument vector
	num_valid_arguments = test_argument_vector_for_condition(argument_vector, true, condition_node, &num_true, &num_false, &num_known_true, &num_known_false);

	// use the sexp_or algorithm
	if (num_known_true)
		return SEXP_KNOWN_TRUE;
	else if (num_known_false == num_valid_arguments)
		return SEXP_KNOWN_FALSE;
	else if (num_true)
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

void sexp_change_all_argument_validity(int n, bool invalidate)
{
	int arg_handler, arg_n;

	arg_handler = get_handler_for_x_of_operator(n);

	// can't change validity of for-counter
	if (get_operator_const(CTEXT(arg_handler)) == OP_FOR_COUNTER)
		return;
		
	while (n != -1)
	{
		arg_n = CDR(arg_handler);
		while (arg_n != -1) {
			if (invalidate) {
				Sexp_nodes[arg_n].flags &= ~SNF_ARGUMENT_VALID;
			}
			else {
				Sexp_nodes[arg_n].flags |= SNF_ARGUMENT_VALID;
			}
			// iterate
			arg_n = CDR(arg_n);
		}
		
		// iterate
		n = CDR(n);
	}
}

int sexp_num_valid_arguments( int n )
{
	int arg_handler, arg_n;
	int matches = 0;

	arg_handler = get_handler_for_x_of_operator(n);

	// loop through arguments
	arg_n = CDR(arg_handler);
	while (arg_n != -1) {
		if (Sexp_nodes[arg_n].flags & SNF_ARGUMENT_VALID) {
			matches++;
		}

		
		// iterate
		arg_n = CDR(arg_n);
	}

	return matches;
}

// Goober5000
void sexp_change_argument_validity(int n, bool invalidate)
{
	int arg_handler, arg_n;
	bool toggled;

	arg_handler = get_handler_for_x_of_operator(n);

	// can't change validity of for-counter
	if (get_operator_const(CTEXT(arg_handler)) == OP_FOR_COUNTER)
		return;
		
	// loop through arguments
	while (n != -1)
	{
		toggled = false; 

		// first we must check if the arg_handler marks a selection. At the moment random-of is the only one that does this
		arg_n = CDR(arg_handler);
		while (invalidate && (arg_n != -1)) {
			if (Sexp_nodes[arg_n].flags & SNF_ARGUMENT_SELECT) {
				// now check if the selected argument matches the one we want to invalidate
				if (!strcmp(CTEXT(n), CTEXT(arg_n))) {
					// set it as invalid
					Sexp_nodes[arg_n].flags &= ~SNF_ARGUMENT_VALID;
					toggled = true; 
				}
			}

			// iterate
			arg_n = CDR(arg_n);
		}
		
		if (!toggled) {
			// search for argument in arg_handler list
			arg_n = CDR(arg_handler);
			while (arg_n != -1)
			{
				// match?
				if (!strcmp(CTEXT(n), CTEXT(arg_n)))
				{
					if (invalidate) {
						// we need to check if the argument is already invalid as some argument lists may contain duplicates
						if (Sexp_nodes[arg_n].flags & SNF_ARGUMENT_VALID) {
							// set it as invalid
							Sexp_nodes[arg_n].flags &= ~SNF_ARGUMENT_VALID;

							// exit inner loop
							break;
						}
					}
					else {
						if (!(Sexp_nodes[arg_n].flags & SNF_ARGUMENT_VALID)) {
							// set it as valid
							Sexp_nodes[arg_n].flags |= SNF_ARGUMENT_VALID;

							// exit inner loop
							break;
						}
					}
				}

				// iterate
				arg_n = CDR(arg_n);
			}
		}

		// iterate
		n = CDR(n);
	}
}

int get_handler_for_x_of_operator(int n)
{
	int conditional, arg_handler;

	if (n < 0) {
		return -1;
	}

	conditional = n; 
	do {
		// find the conditional sexp
		conditional = find_parent_operator(conditional);
		if (conditional == -1) {
			return -1;
		}
	}
	while (!is_blank_argument_op(get_operator_const(CTEXT(conditional))));

	// get the first op of the parent, which should be a *_of operator
	arg_handler = CADR(conditional);
	if (!is_blank_of_op(get_operator_const(CTEXT(arg_handler)))) {
		return -1;
	}

	return arg_handler;

}

// Goober5000
bool is_blank_argument_op(int op_const)
{
	switch (op_const)
	{
		case OP_WHEN_ARGUMENT:
		case OP_EVERY_TIME_ARGUMENT:
			return true;

		default:
			return false;
	}
}

// Goober5000
bool is_blank_of_op(int op_const)
{
	switch (op_const)
	{
		case OP_ANY_OF:
		case OP_EVERY_OF:
		case OP_NUMBER_OF:
		case OP_RANDOM_OF:
		case OP_RANDOM_MULTIPLE_OF:
		case OP_IN_SEQUENCE:
		case OP_FOR_COUNTER:
			return true;

		default:
			return false;
	}
}

// Goober5000 - added wing capability
int sexp_is_iff(int n)
{
	int i, team;

	// iff value is the first parameter, second is a list of one or more ships/wings to check to see if the
	// iff value matches
	team = iff_lookup(CTEXT(n));

	n = CDR(n);
	for ( ; n != -1; n = CDR(n) )
	{
		object_ship_wing_point_team oswpt;
		sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));

		switch (oswpt.type)
		{
			case OSWPT_TYPE_SHIP:
			{
				// if the team doesn't match the team specified, return false immediately
				if (oswpt.shipp->team != team)
					return SEXP_FALSE;

				break;
			}

			case OSWPT_TYPE_PARSE_OBJECT:
			{
				// if the team doesn't match the team specified, return false immediately
				if (oswpt.p_objp->team != team)
					return SEXP_FALSE;

				break;
			}

			case OSWPT_TYPE_WING:
			{
				for (i = 0; i < oswpt.wingp->current_count; i++)
				{
					// if the team doesn't match the team specified, return false immediately
					if (Ships[oswpt.wingp->ship_index[i]].team != team)
						return SEXP_FALSE;
				}

				break;
			}
	
		}

	}

	// got this far: we must be okay for all ships/wings
	return SEXP_TRUE;
}

// Goober5000
void sexp_ingame_ship_change_iff(ship *shipp, int new_team)
{
	Assert(shipp != NULL);

	shipp->team = new_team;

	// send a network packet if we need to
	if( MULTIPLAYER_MASTER && (Net_player != NULL) && (shipp->objnum >= 0))
		send_change_iff_packet(Objects[shipp->objnum].net_signature, new_team);
}

// Goober5000
void sexp_parse_ship_change_iff(p_object *parse_obj, int new_team)
{
	Assert(parse_obj);

	parse_obj->team = new_team;
}

// Goober5000 - added wing capability
void sexp_change_iff(int n)
{
	int new_team;

	new_team = iff_lookup(CTEXT(n));
	n = CDR(n);

	for ( ; n != -1; n = CDR(n) )
	{
		object_ship_wing_point_team oswpt;
		sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));

		switch (oswpt.type)
		{
			// change ingame ship
			case OSWPT_TYPE_SHIP:
			{
				sexp_ingame_ship_change_iff(oswpt.shipp, new_team);

				break;
			}

			// change ship yet to arrive
			case OSWPT_TYPE_PARSE_OBJECT:
			{
				sexp_parse_ship_change_iff(oswpt.p_objp, new_team);

				break;
			}

			// change wing (we must set the flags for all ships present as well as all ships yet to arrive)
			case OSWPT_TYPE_WING:
			case OSWPT_TYPE_WING_NOT_PRESENT:
			{
				// current ships
				for (int i = 0; i < oswpt.wingp->current_count; i++)
					sexp_ingame_ship_change_iff(&Ships[oswpt.wingp->ship_index[i]], new_team);

				// ships yet to arrive
				for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
				{
					if (p_objp->wingnum == WING_INDEX(oswpt.wingp))
						sexp_parse_ship_change_iff(p_objp, new_team);
				}

				break;
			}
		}
	}
}

void sexp_ingame_ship_change_iff_color(ship *shipp, int observer_team, int observed_team, int alternate_iff_color)
{
	Assert(shipp != NULL);

	shipp->ship_iff_color[observer_team][observed_team] = alternate_iff_color;

	// Like the rest of the change_iff_color functions copied with minor alterations from earlier change_iff functions.
	if( MULTIPLAYER_MASTER && (Net_player != NULL) && (shipp->objnum >= 0))
		send_change_iff_color_packet(Objects[shipp->objnum].net_signature, observer_team, observed_team, alternate_iff_color);
}

void sexp_parse_ship_change_iff_color(p_object *parse_obj, int observer_team, int observed_team, int alternate_iff_color)
{
	Assert(parse_obj);

	parse_obj->alt_iff_color[observer_team][observed_team] = alternate_iff_color;
}

 // Wanderer
void sexp_change_iff_color(int n)
{
	int observer_team, observed_team, alternate_iff_color;
	int i;
	int rgb[3];

	// First node
	if(n == -1){
		Warning(LOCATION, "Detected missing observer team parameter in sexp-change_iff_color");
		return;
	}
	observer_team = iff_lookup(CTEXT(n));

	// Second node
	n = CDR(n);
	if(n == -1){
		Warning(LOCATION, "Detected missing observed team parameter in sexp-change_iff_color");
		return;
	}
	observed_team = iff_lookup(CTEXT(n));

	// Three following nodes
	for (i=0;i<3;i++)
	{
		n = CDR(n);
		if(n == -1){
			Warning(LOCATION, "Detected incomplete color parameter list in sexp-change_iff_color\n");
			return;
		}
		rgb[i] = eval_num(n);
		if (rgb[i] > 255) {
			Warning(LOCATION, "Invalid argument for iff color in sexp-change-iff-color. Valid range is 0 to 255.\n");
			rgb[i] = 255;
		}
	}
	alternate_iff_color = iff_init_color(rgb[0],rgb[1],rgb[2]);

	// Rest of the nodes
	n = CDR(n);
	for ( ; n != -1; n = CDR(n) )
	{
		object_ship_wing_point_team oswpt;
		sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));

		switch (oswpt.type)
		{
			// change ingame ship
			case OSWPT_TYPE_SHIP:
			{
				sexp_ingame_ship_change_iff_color(oswpt.shipp, observer_team, observed_team, alternate_iff_color);

				break;
			}

			// change ship yet to arrive
			case OSWPT_TYPE_PARSE_OBJECT:
			{
				sexp_parse_ship_change_iff_color(oswpt.p_objp, observer_team, observed_team, alternate_iff_color);

				break;
			}

			// change wing (we must set the flags for all ships present as well as all ships yet to arrive)
			case OSWPT_TYPE_WING:
			case OSWPT_TYPE_WING_NOT_PRESENT:
			{
				// current ships
				for (i = 0; i < oswpt.wingp->current_count; i++)
					sexp_ingame_ship_change_iff_color(&Ships[oswpt.wingp->ship_index[i]], observer_team, observed_team, alternate_iff_color);

				// ships yet to arrive
				for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
				{
					if (p_objp->wingnum == WING_INDEX(oswpt.wingp))
						sexp_parse_ship_change_iff_color(p_objp, observer_team, observed_team, alternate_iff_color);
				}

				break;
			}
		}
	}
}

// Goober5000
int sexp_is_ship_class(int n)
{
	int ship_num, ship_class_num;

	Assert( n >= 0 );

	// get class
	ship_class_num = ship_info_lookup(CTEXT(n));
	n = CDR(n);

	// eval ships
	while (n != -1)
	{
		// get ship
		ship_num = ship_name_lookup(CTEXT(n));

		// we can't do anything with ships that aren't present
		if (ship_num < 0)
			return SEXP_CANT_EVAL;

		// if it doesn't match, return false
		if (Ships[ship_num].ship_info_index != ship_class_num)
			return SEXP_FALSE;

		// increment
		n = CDR(n);
	}

	// we're this far; it must be true
	return SEXP_TRUE;
}

// Goober5000
int sexp_is_ship_type(int n)
{
	int ship_num, ship_type_num;

	Assert( n >= 0 );

	// get type
	ship_type_num = ship_type_name_lookup(CTEXT(n));
	n = CDR(n);

	// eval ships
	while (n != -1)
	{
		// get ship
		ship_num = ship_name_lookup(CTEXT(n));

		// we can't do anything with ships that aren't present
		if (ship_num < 0)
			return SEXP_CANT_EVAL;

		// if it doesn't match, return false
		if(ship_class_query_general_type(Ships[ship_num].ship_info_index) != ship_type_num)
			return SEXP_FALSE;

		// increment
		n = CDR(n);
	}

	// we're this far; it must be true
	return SEXP_TRUE;
}

// Goober5000
// ai class value is the first parameter, second is a ship, rest are subsystems to check
int sexp_is_ai_class(int n)
{
	char *ship_name, *subsystem;
	int i, ship_num, ai_class;

	Assert ( n >= 0 );

	// find ai class
	ai_class = -1;
	for (i=0; i<Num_ai_classes; i++)
	{
		if (!stricmp(Ai_class_names[i], CTEXT(n)))
			ai_class = i;
	}

	Assert(ai_class >= 0);

	n = CDR(n);
	ship_name = CTEXT(n);
	n = CDR(n);

	// find ship
	ship_num = ship_name_lookup(ship_name);

	// we can't do anything with ships that aren't present
	if (ship_num < 0)
		return SEXP_CANT_EVAL;

	// subsys?
	if (n != -1)
	{
		ship_subsys *ss;

		// loopity-loop
		for ( ; n != -1; n = CDR(n) )
		{
			subsystem = CTEXT(n);

			// find the ship subsystem
			// if it doesn't match, or subsys isn't found, return false immediately
			ss = ship_get_subsys(&Ships[ship_num], subsystem);
			if (ss != NULL)
			{
				if (ss->weapons.ai_class != ai_class)
					return SEXP_FALSE;
			}
			else
				return SEXP_FALSE;
		}

		// we've come this far; it must all be true
		return SEXP_TRUE;
	}
	// just the ship
	else
	{
		if (Ai_info[Ships[ship_num].ai_index].ai_class == ai_class)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
	}
}

// Goober5000
void sexp_change_ai_class(int n)
{
	int i, ship_num, new_ai_class;

	Assert ( n >= 0 );

	// find ai class
	new_ai_class = -1;
	for (i=0; i<Num_ai_classes; i++)
	{
		if (!stricmp(Ai_class_names[i], CTEXT(n)))
			new_ai_class = i;
	}

	Assert(new_ai_class >= 0);

	// find ship
	n = CDR(n);
	ship_num = ship_name_lookup(CTEXT(n));
	n = CDR(n);

	// we can't do anything with ships that aren't present
	if (ship_num < 0)
		return;

	// subsys?
	if (n != -1)
	{
		// loopity-loop
		for ( ; n != -1; n = CDR(n) )
		{
			ship_subsystem_set_new_ai_class(ship_num, CTEXT(n), new_ai_class);

			// send a network packet if we need to
			if( MULTIPLAYER_MASTER && (Net_player != NULL) && (Ships[ship_num].objnum >= 0))
			{
				send_change_ai_class_packet(Objects[Ships[ship_num].objnum].net_signature, CTEXT(n), new_ai_class);
			}
		}
	}
	// just the one ship
	else
	{
		ship_set_new_ai_class(ship_num, new_ai_class);

		// send a network packet if we need to
		if( MULTIPLAYER_MASTER && (Net_player != NULL) && (Ships[ship_num].objnum >= 0))
		{
			send_change_ai_class_packet(Objects[Ships[ship_num].objnum].net_signature, NULL, new_ai_class);
		}
	}
}

// following routine adds an ai goal to a ship structure.  The sexpression index
// passed in should be an ai-goal of the proper form.  The code in MissionGoal should
// check the syntax.

void sexp_add_ship_goal(int n)
{
	int num, sindex;
	char *ship_name;

	Assert ( n >= 0 );
	ship_name = CTEXT(n);
	num = ship_name_lookup(ship_name, 1);	// Goober5000 - including player
	if ( num < 0 )									// ship not around anymore???? then forget it!
		return;

	sindex = CDR(n);
	ai_add_ship_goal_sexp( sindex, AIG_TYPE_EVENT_SHIP, &(Ai_info[Ships[num].ai_index]) );
}

// identical to above, except add a wing
void sexp_add_wing_goal(int n)
{
	int num, sindex;
	char *wing_name;

	Assert ( n >= 0 );
	wing_name = CTEXT(n);
	num = wing_name_lookup(wing_name);
	if ( num < 0 )									// ship not around anymore???? then forget it!
		return;

	sindex = CDR(n);
	ai_add_wing_goal_sexp( sindex, AIG_TYPE_EVENT_WING, num );
}

/**
 * Adds a goal to the specified entry (ships and wings have unique names between the two sets).
 */
void sexp_add_goal(int n)
{
	int num, sindex;
	char *name;

	Assert ( n >= 0 );
	name = CTEXT(n);
	sindex = CDR(n);

	// first, look for ship name -- if found, then add ship goal.  else look for wing name -- if
	// found, add wing goal
	if ( (num = ship_name_lookup(name, 1)) != -1 )	// Goober5000 - include players
		ai_add_ship_goal_sexp( sindex, AIG_TYPE_EVENT_SHIP, &(Ai_info[Ships[num].ai_index]) );
	else if ( (num = wing_name_lookup(name)) != -1 )
		ai_add_wing_goal_sexp( sindex, AIG_TYPE_EVENT_WING, num );
}

// Goober5000
void sexp_remove_goal(int n)
{
	Assert( n >= 0 );
	/* Grab the information that we need about this goal removal action */
	int num, sindex;
	int goalindex;
	char *name;

	name = CTEXT(n);
	sindex = CDR(n);

	// first, look for ship name -- if found, then add ship goal.  else look for wing name -- if
	// found, add wing goal
	if ( (num = ship_name_lookup(name, 1)) != -1 )
	{
		goalindex = ai_remove_goal_sexp_sub( sindex, Ai_info[Ships[num].ai_index].goals );
		if ( goalindex >= 0 )
		{
			if ( Ai_info[Ships[num].ai_index].active_goal == goalindex )
				Ai_info[Ships[num].ai_index].active_goal = AI_GOAL_NONE;
		}
	}
	else if ( (num = wing_name_lookup(name)) != -1 )
	{
		ai_remove_wing_goal_sexp( sindex, num );
	}
	
}

/**
 * Clear out all AI goals for a ship
 */
void sexp_clear_ship_goals(int n)
{
	int num;
	char *ship_name;

	Assert ( n >= 0 );
	ship_name = CTEXT(n);
	num = ship_name_lookup(ship_name, 1);	// Goober5000 - include players
	ai_clear_ship_goals( &(Ai_info[Ships[num].ai_index]) );
}

/**
 * Clear out AI goals for a wing
 */
void sexp_clear_wing_goals(int n)
{
	int num;
	char *wing_name;

	Assert ( n >= 0 );
	wing_name = CTEXT(n);
	num = wing_name_lookup(wing_name);
	if ( num < 0 )
		return;
	ai_clear_wing_goals( num );
}

/**
 * Clear all AI goals for the given ship or wing
 */
void sexp_clear_goals(int n)
{
	int num;
	char *name;

	Assert ( n >= 0 );
	while ( n != -1 ) {
		name = CTEXT(n);
		if ( (num = ship_name_lookup(name, 1)) != -1 )	// Goober5000 - include players
			ai_clear_ship_goals( &(Ai_info[Ships[num].ai_index]) );
		else if ( (num = wing_name_lookup(name)) != -1 )
			ai_clear_wing_goals( num );

		n = CDR(n);
	}
}

// Goober5000
void sexp_hud_disable(int n)
{
	int disable_hud = eval_num(n);
	hud_set_draw(!disable_hud);

	multi_start_callback();
	multi_send_int(disable_hud);
	multi_end_callback();
}

void multi_sexp_hud_disable()
{
	int disable_hud;

	if (multi_get_int(disable_hud)) {
		hud_set_draw(!disable_hud);
	}
}

// Goober5000
void sexp_hud_disable_except_messages(int n)
{
	int disable_hud = eval_num(n);
	hud_disable_except_messages(disable_hud);

	multi_start_callback();
	multi_send_int(disable_hud);
	multi_end_callback();
}

void multi_sexp_hud_disable_except_messages()
{
	int disable_hud;

	if (multi_get_int(disable_hud)) {
		hud_disable_except_messages(disable_hud);
	}
}

void sexp_hud_set_text_num(int n)
{
	char* gaugename = CTEXT(n);
	char tmp[16] = "";

	HudGauge* cg = hud_get_gauge(gaugename);
	if(cg) {
		sprintf( tmp, "%d", eval_num(CDR(n)) );
		cg->updateCustomGaugeText(tmp);
	}
}

void sexp_hud_set_text(int n)
{
	char* gaugename = CTEXT(n);
	char* text = CTEXT(CDR(n));

	HudGauge* cg = hud_get_gauge(gaugename);
	if(cg) {
		cg->updateCustomGaugeText(text);
	}
}

void sexp_hud_set_message(int n)
{
	char* gaugename = CTEXT(n);
	char* text = CTEXT(CDR(n));
	SCP_string message;

	for (int i = 0; i < Num_messages; i++) {
		if ( !stricmp(text, Messages[i].name) ) {
			message = Messages[i].message;

			sexp_replace_variable_names_with_values(message);

			HudGauge* cg = hud_get_gauge(gaugename);
			if(cg) {
				cg->updateCustomGaugeText(message);
			} else {
				WarningEx(LOCATION, "Could not find a hud gauge named %s\n", gaugename);
			}
			return;
		}
	}

	WarningEx(LOCATION, "sexp_hud_set_message couldn't find a message by the name of %s in the mission\n", text);
}

void sexp_hud_set_directive(int n)
{
	char* gaugename = CTEXT(n);
	char* text = CTEXT(CDR(n));
	char message[MESSAGE_LENGTH];

	message_translate_tokens(message, text);

	if (strlen(message) > MESSAGE_LENGTH) {
		WarningEx(LOCATION, "Message %s is too long for use in a HUD gauge. Please shorten it to %d characters or less.", message, MESSAGE_LENGTH);
		return;
	}

	HudGauge* cg = hud_get_gauge(gaugename);
	if(cg) {
		cg->updateCustomGaugeText(message);
	} else {
		WarningEx(LOCATION, "Could not find a hud gauge named %s\n", gaugename);
	}
}

void sexp_hud_clear_messages()
{
	if(Ship_info[Player_ship->ship_info_index].hud_gauges.size() > 0) {
		size_t num_gauges = Ship_info[Player_ship->ship_info_index].hud_gauges.size();

		for(size_t i = 0; i < num_gauges; i++) {
			if (Ship_info[Player_ship->ship_info_index].hud_gauges[i]->getObjectType() == HUD_OBJECT_MESSAGES) {
				HudGaugeMessages* gauge = static_cast<HudGaugeMessages*>(Ship_info[Player_ship->ship_info_index].hud_gauges[i]);
				gauge->clearMessages();
			}
		}
	} else {
		size_t num_gauges = default_hud_gauges.size();

		for(size_t i = 0; i < num_gauges; i++) {
			if (default_hud_gauges[i]->getObjectType() == HUD_OBJECT_MESSAGES) {
				HudGaugeMessages* gauge = static_cast<HudGaugeMessages*>(default_hud_gauges[i]);
				gauge->clearMessages();
			}
		}
	}
}

void sexp_hud_set_coords(int n)
{
	char* gaugename = CTEXT(n);
	int coord_x = eval_num(CDR(n));
	int coord_y = eval_num(CDR(CDR(n)));

	HudGauge* cg = hud_get_gauge(gaugename);
	if(cg) {
		cg->updateCustomGaugeCoords(coord_x, coord_y);
	}
}

void sexp_hud_set_frame(int n)
{
	char* gaugename = CTEXT(n);
	int frame_num = eval_num(CDR(n));

	HudGauge* cg = hud_get_gauge(gaugename);
	if(cg) {
		cg->updateCustomGaugeFrame(frame_num);
	}
}

void sexp_hud_set_color(int n)
{
	char* gaugename = CTEXT(n);
	ubyte red = (ubyte) eval_num(CDR(n));
	ubyte green = (ubyte) eval_num(CDR(CDR(n)));
	ubyte blue = (ubyte) eval_num(CDR(CDR(CDR(n))));

	HudGauge* cg = hud_get_gauge(gaugename);
	if(cg) {
		cg->sexpLockConfigColor(false);
		cg->updateColor(red, green, blue, (HUD_color_alpha+1)*16);
		cg->sexpLockConfigColor(true);
	}
}


// Goober5000
void sexp_hud_set_max_targeting_range(int n)
{
	Hud_max_targeting_range = eval_num(n);

	if (Hud_max_targeting_range < 0)
		Hud_max_targeting_range = 0;
}

/* Make sure that the Sexp_hud_display_* get added to the game_state
transitions in freespace.cpp (game_enter_state()). */
int Sexp_hud_display_warpout = 0;

void sexp_hud_display_gauge(int n) {
	int show_for = eval_num(n);
	char* gauge = CTEXT(CDR(n));

	if ( stricmp(SEXP_HUD_GAUGE_WARPOUT, gauge) == 0 ) {
		Sexp_hud_display_warpout = (show_for > 1)? timestamp(show_for) : (show_for);
	} 
}

void sexp_hud_gauge_set_active(int n) {
	HudGauge* hg;
	char* name = CTEXT(n);
	bool active = (is_sexp_true(CDR(n)) != 0);

	hg = hud_get_gauge(name);

	if (hg != NULL) {
		hg->updateActive(active);
	}
}

int hud_gauge_type_lookup(char* name) {
	for(int i = 0; i < Num_hud_gauge_types; i++) {
		if(!stricmp(name, Hud_gauge_types[i].name))
			return Hud_gauge_types[i].def;
	}
	return -1;
}

void sexp_hud_activate_gauge_type(int n) {
	int config_type = hud_gauge_type_lookup(CTEXT(n));
	bool active = (is_sexp_true(CDR(n)) != 0);
	
	if (config_type != -1) { 
		if(Ship_info[Player_ship->ship_info_index].hud_gauges.size() > 0) {
			size_t num_gauges = Ship_info[Player_ship->ship_info_index].hud_gauges.size();

			for(size_t i = 0; i < num_gauges; i++) {
				if (Ship_info[Player_ship->ship_info_index].hud_gauges[i]->getObjectType() == config_type)
					Ship_info[Player_ship->ship_info_index].hud_gauges[i]->updateSexpOverride(!active);
			}
		} else {
			size_t num_gauges = default_hud_gauges.size();

			for(size_t i = 0; i < num_gauges; i++) {
				if (default_hud_gauges[i]->getObjectType() == config_type)
					default_hud_gauges[i]->updateSexpOverride(!active);
			}
		}
	}
}

void multi_sexp_hud_display_gauge()
{
	int show_for; 

	if (multi_get_int(show_for)) {
		Sexp_hud_display_warpout = (show_for > 1)? timestamp(show_for) : (show_for);
	}
}

// Goober5000
/**
 * Trigger whether player uses the game AI for stuff
 */
void sexp_player_use_ai(int flag)
{
	Player_use_ai = flag ? 1 : 0;
}

// Karajorma
void sexp_allow_treason (int n) 
{
	n = CDR(n);
	if (n != -1) {
		if ( is_sexp_true(n) ) {
			The_mission.flags |= MISSION_FLAG_NO_TRAITOR;
		} 
		else {
			The_mission.flags &= ~MISSION_FLAG_NO_TRAITOR;
		}
	}
}

void sexp_set_player_orders(int n) 
{
	ship *shipp; 
	int i;
	int allow_order;
	int orders = 0;
	int default_orders; 

	shipp = sexp_get_ship_from_node(n);

	// we need to know which orders this ship class can accept.
	default_orders = ship_get_default_orders_accepted(&Ship_info[shipp->ship_info_index]);
	n = CDR(n);
	allow_order = is_sexp_true(n);
	n = CDR(n);
	do {
		for (i = 0 ; i < NUM_COMM_ORDER_ITEMS ; i++) {
			// since it's the cheaper test, test first if the ship will even accept this order first
			if (default_orders & Sexp_comm_orders[i].item) {
				if (!stricmp(CTEXT(n), Sexp_comm_orders[i].name)) {
					orders |= Sexp_comm_orders[i].item;
					break;
				}
			}
		}

		n = CDR(n);
	}while (n >= 0);
		
	// set or unset the orders
	if (allow_order) {
		shipp->orders_accepted |= orders;
	}
	else {
		shipp->orders_accepted &= ~orders;
	}
}

// Goober5000
void sexp_change_soundtrack(int n)
{
	event_sexp_change_soundtrack(CTEXT(n));

	if (MULTIPLAYER_MASTER) {
		multi_start_callback(); 
		multi_send_string(CTEXT(n));
		multi_end_callback(); 
	}
}

void multi_sexp_change_soundtrack()
{
	char new_track[NAME_LENGTH]; 

	if (multi_get_string(new_track)) {
		event_sexp_change_soundtrack(new_track);
	}
}

// Goober5000
void sexp_stop_music(int fade)
{
	if ( Sexp_music_handle != -1 ) {
		audiostream_close_file(Sexp_music_handle, fade);
		Sexp_music_handle = -1;
	}
}

// Goober5000
void sexp_music_close()
{
	if ( Cmdline_freespace_no_music ) {
		return;
	}

	sexp_stop_music();
}

// Goober5000
void sexp_load_music(char* fname, int type = -1)
{
	if ( Cmdline_freespace_no_music ) {
		return;
	}

	if ( Sexp_music_handle != -1 )
	{
		sexp_stop_music();
	}

	if ( type < 0 )
	{
		type = ASF_MENUMUSIC;
	}

	if ( fname )
	{
		Sexp_music_handle = audiostream_open( fname, type );
	}
}

// Goober5000
void sexp_start_music(int loop)
{
	if ( Sexp_music_handle != -1 ) {
		if ( !audiostream_is_playing(Sexp_music_handle) )
			audiostream_play(Sexp_music_handle, (Master_event_music_volume * aav_music_volume), loop);
	}
	else {
		nprintf(("Warning", "Can not play music. sexp_start_music called when no music file is set for Sexp_music_handle!\n"));
	}
}

// Goober5000
void sexp_play_sound_from_table(int n)
{
	vec3d origin;
	int sound_index;

	Assert( n >= 0 );

	// read in data --------------------------------
	origin.xyz.x = (float)eval_num(n);
	n = CDR(n);
	origin.xyz.y = (float)eval_num(n);
	n = CDR(n);
	origin.xyz.z = (float)eval_num(n);
	n = CDR(n);
	sound_index = eval_num(n);


	// play sound effect ---------------------------
	if (sound_index >= 0) {
		snd_play_3d( &Snds[gamesnd_get_by_tbl_index(sound_index)], &origin, &View_position, 0.0f, NULL, 0, 1.0f, SND_PRIORITY_MUST_PLAY  );
	}

	if (MULTIPLAYER_MASTER) {
		multi_start_callback();
		multi_send_float(origin.xyz.x);
		multi_send_float(origin.xyz.y);
		multi_send_float(origin.xyz.z);
		multi_send_int(sound_index);
		multi_end_callback();
	}
}

void multi_sexp_play_sound_from_table()
{
	vec3d origin;
	int sound_index = -1;

	multi_get_float(origin.xyz.x);
	multi_get_float(origin.xyz.y);
	multi_get_float(origin.xyz.z);
	multi_get_int(sound_index);


	if (sound_index != -1) {
		// play sound effect ---------------------------
		snd_play_3d( &Snds[gamesnd_get_by_tbl_index(sound_index)], &origin, &View_position, 0.0f, NULL, 0, 1.0f, SND_PRIORITY_MUST_PLAY  );
	}
}

// Goober5000
void sexp_close_sound_from_file(int n)
{
	int fade = is_sexp_true(n);
	sexp_stop_music(fade);

	if (MULTIPLAYER_MASTER) {
		multi_start_callback();
		multi_send_bool(fade ? true : false); 
		multi_end_callback();
	}
}

void multi_sexp_close_sound_from_file()
{
	bool fade; 
	if (multi_get_bool(fade)) {
		sexp_stop_music(fade);
	}
}

// Goober5000
void sexp_play_sound_from_file(int n)
{
	int loop = 0;
	int soundfx = 0;
	int type = ASF_MENUMUSIC;

	// third option, but needed when loading music -
	soundfx = CDDR(n);

	if (soundfx >= 0) {
		soundfx = eval_sexp(soundfx);

		if (soundfx > 0) {
			type = ASF_SOUNDFX;
		}
	}

	// load sound file -----------------------------
	sexp_load_music(CTEXT(n), type);
	
	if (MULTIPLAYER_MASTER) {
		multi_start_callback();
		multi_send_string(CTEXT(n));
	}

	n = CDR(n);
	if (n >= 0) {
		loop = eval_sexp(n);
	}

	// play sound file -----------------------------
	sexp_start_music(loop);	

	if (MULTIPLAYER_MASTER) {
		multi_send_bool(loop ? true : false); 
		multi_end_callback();
	}
}

void multi_sexp_play_sound_from_file()
{
	char filename[NAME_LENGTH];
	bool (loop);
	if (!multi_get_string(filename)) {
		return;
	}
	sexp_load_music(filename);

	multi_get_bool(loop);
	sexp_start_music(loop);	
}

int sexp_sound_environment_option_lookup(char *text)
{
	int i;

	Assert(text != NULL);
	if (text == NULL) {
		return -1;
	}
	
	for (i = 0; i < Num_sound_environment_options; i++) {
		if (!strcmp(text, Sound_environment_option[i])) {
			return i;
		}
	}

	return -1;
}

// Taylor
void sexp_set_sound_environment(int node)
{
	int n = node;
	sound_env env;
	int preset_id = -1;

	char *preset = CTEXT(n);
	n = CDR(n);

	if ( preset && !stricmp(preset, SEXP_NONE_STRING) ) {
		sound_env_disable();
		return;
	}

	preset_id = ds_eax_get_preset_id( preset );
	if (preset_id < 0) {
		return;
	}

	// fill in defaults for this preset, in case we don't set everything
	if ( sound_env_get(&env, preset_id) ) {
		return;
	}

	while (n >= 0) {
		int option = sexp_sound_environment_option_lookup(CTEXT(n));
		n = CDR(n);

		// watch out for bogus options
		if (n < 0) {
			break;
		}

		float val = (float)eval_num(n) / 1000.0f;
		n = CDR(n);

		if ( option == SEO_VOLUME ) {
			env.volume = val;
		} else if ( option == SEO_DECAY_TIME ) {
			env.decay = val;
		} else if ( option == SEO_DAMPING ) {
			env.damping = val;
		}
	}
	
	sound_env_set(&env);
}

// Taylor
void sexp_update_sound_environment(int node)
{
	int n = node;

	while (n >= 0) {
		int option = sexp_sound_environment_option_lookup(CTEXT(n));
		n = CDR(n);

		// watch out for bogus options
		if (n < 0) {
			break;
		}

		float val = (float)eval_num(n) / 1000.0f;
		n = CDR(n);

		if ( option == SEO_VOLUME ) {
			ds_eax_set_volume(val);
		} else if ( option == SEO_DECAY_TIME ) {
			ds_eax_set_decay_time(val);
		} else if ( option == SEO_DAMPING ) {
			ds_eax_set_damping(val);
		}
	}
}

//The E
	//From sexp help:
	//{ OP_ADJUST_AUDIO_VOLUME, "adjust-audio-volume\r\n"
	//	"Adjusts the relative volume of one sound type. Takes 2 or 3 arguments....\r\n"
	//	"\t1:\tSound Type to adjust, either Music, Voice or Effects\r\n"
	//	"\t2:\tPercentage of the users' settings to adjust to, 0 will be silence, 100 means the maximum volume as set by the user\r\n"
	//	"\t3:\tFade time (optional), time in milliseconds to adjust the volume"},

int audio_volume_option_lookup(char *text)
{
	int i;

	Assert(text != NULL);
	if (text == NULL) {
		return -1;
	}
	
	for (i = 0; i < Num_adjust_audio_options; i++) {
		if (!strcmp(text, Adjust_audio_options[i])) {
			return i;
		}
	}

	return -1;
}

void sexp_adjust_audio_volume(int node)
{
	int n = node;

	if (n > 0) {
		int option = audio_volume_option_lookup(CTEXT(n));
		if (option > 0) {
			n = CDR(n);

			float target_volume = 1.0f;
			if (n >= 0) {
				target_volume = (float)eval_num(n) / 100;
				CLAMP(target_volume, 0.0f, 1.0f);
				n = CDR(n);
			}

			int time = 0;
			if (n >= 0)
				time = eval_num(n);

			snd_adjust_audio_volume(option, target_volume, time);
		}
	}
}

int sexp_explosion_option_lookup(char *text)
{
	int i;

	Assert(text != NULL);
	if (text == NULL) {
		return -1;
	}
	
	for (i = 0; i < Num_explosion_options; i++) {
		if (!strcmp(text, Explosion_option[i])) {
			return i;
		}
	}

	return -1;
}

// Goober5000
void sexp_set_explosion_option(int node)
{
	int n = node, ship_num;
	ship *shipp;
	ship_info *sip;
	shockwave_create_info *sci;

	// get ship
	ship_num = ship_name_lookup(CTEXT(n));
	if (ship_num < 0)
		return;

	shipp = &Ships[ship_num];
	sip = &Ship_info[shipp->ship_info_index];
	sci = &sip->shockwave;

	n = CDR(n);

	// if we haven't changed anything yet, create a new special-exp with the same values as a standard exp
	if (!shipp->use_special_explosion)
	{
		shipp->special_exp_damage = fl2i(sci->damage);
		shipp->special_exp_blast = fl2i(sci->blast);
		shipp->special_exp_inner = fl2i(sci->inner_rad);
		shipp->special_exp_outer = fl2i(sci->outer_rad);
		shipp->special_exp_shockwave_speed = fl2i(sci->speed);
		shipp->special_exp_deathroll_time = 0;

		shipp->use_special_explosion = true;
		shipp->use_shockwave = (sci->speed > 0);
	}

	// process all options
	while (n >= 0)
	{
		int option = sexp_explosion_option_lookup(CTEXT(n));
		n = CDR(n);

		// watch out for bogus options
		if (n < 0)
			break;

		int val = eval_num(n);
		Assert(val >= 0);	// should be true due to OPF_POSITIVE
		n = CDR(n);

		if (option == EO_DAMAGE) {
			shipp->special_exp_damage = val;
		} else if (option == EO_BLAST) {
			shipp->special_exp_blast = val;
		} else if (option == EO_INNER_RADIUS) {
			shipp->special_exp_inner = val;
		} else if (option == EO_OUTER_RADIUS) {
			shipp->special_exp_outer = val;
		} else if (option == EO_SHOCKWAVE_SPEED) {
			shipp->special_exp_shockwave_speed = val;
			shipp->use_shockwave = (val > 0);
		} else if (option == EO_DEATH_ROLL_TIME) {
			shipp->special_exp_deathroll_time = val;

			// hmm, it would be cool to modify the explosion in progress
			if (shipp->flags & SF_DYING && val >= 2) {
				shipp->final_death_time = timestamp(val);
			}
		}
	}

	// if all our values are the same as a standard exp, turn off the special exp
	if ((shipp->special_exp_damage == sci->damage) && (shipp->special_exp_blast == sci->blast) && (shipp->special_exp_inner == sci->inner_rad)
		&& (shipp->special_exp_outer == sci->outer_rad) && (shipp->special_exp_shockwave_speed == sci->speed) && (shipp->special_exp_deathroll_time == 0))
	{
		shipp->use_special_explosion = false;
		shipp->use_shockwave = false;

		shipp->special_exp_damage = -1;
		shipp->special_exp_blast = -1;
		shipp->special_exp_inner = -1;
		shipp->special_exp_outer = -1;
		shipp->special_exp_shockwave_speed = -1;
		shipp->special_exp_deathroll_time = 0;
	}
}

// Goober5000
void sexp_explosion_effect(int n)
/* From the SEXP help...
	{ OP_EXPLOSION_EFFECT, "explosion-effect\r\n"
		"\tCauses an explosion at a given origin, with the given parameters.  "
		"Takes 11 or 13 arguments...\r\n"
		"\t1:  Origin X\r\n"
		"\t2:  Origin Y\r\n"
		"\t3:  Origin Z\r\n"
		"\t4:  Damage\r\n"
		"\t5:  Blast force\r\n"
		"\t6:  Size of explosion (if 0, explosion will not be visible)\r\n"
		"\t7:  Inner radius to apply damage (if 0, explosion will not be visible)\r\n"
		"\t8:  Outer radius to apply damage (if 0, explosion will not be visible)\r\n"
		"\t9:  Shockwave speed (if 0, there will be no shockwave)\r\n"
		"\t10: Type (0 = medium, 1 = large1, 2 = large2)\r\n"  (otherwise use the index in fireball.tbl - FUBAR)
		"\t11: Sound (index into sounds.tbl)\r\n"
		"\t12: EMP intensity (optional)\r\n"
		"\t13: EMP duration in seconds (optional)" },
*/
// Basically, this function pretends that there's a ship at the origin that's blowing up, and
// it does stuff accordingly.  In some places, it has to tiptoe around a little because the
// code often expects a parent object when in fact there is none. <.<  >.>
{
	vec3d origin;
	int max_damage, max_blast, explosion_size, inner_radius, outer_radius, shockwave_speed, fireball_type, sound_index;
	int emp_intensity, emp_duration;
	shockwave_create_info sci;

	Assert( n >= 0 );

	// read in data --------------------------------
	origin.xyz.x = (float)eval_num(n);
	n = CDR(n);
	origin.xyz.y = (float)eval_num(n);
	n = CDR(n);
	origin.xyz.z = (float)eval_num(n);
	n = CDR(n);

	max_damage = eval_num(n);
	n = CDR(n);
	max_blast = eval_num(n);
	n = CDR(n);

	explosion_size = eval_num(n);
	n = CDR(n);
	inner_radius = eval_num(n);
	n = CDR(n);
	outer_radius = eval_num(n);
	n = CDR(n);

	shockwave_speed = eval_num(n);
	n = CDR(n);

	// fireball type
	if (eval_num(n) == 0)
	{
		fireball_type = FIREBALL_EXPLOSION_MEDIUM;
	}
	else if (eval_num(n) == 1)
	{
		fireball_type = FIREBALL_EXPLOSION_LARGE1;
	}
	else if (eval_num(n) == 2)
	{
		fireball_type = FIREBALL_EXPLOSION_LARGE2;
	}
	else if (eval_num(n) >= Num_fireball_types)	{
		Warning(LOCATION, "explosion-effect type is out of range; quitting the explosion...\n");
		return;
	}
	else {
		fireball_type = eval_num(n);
	}
	n = CDR(n);

	sound_index = eval_num(n);
	n = CDR(n);

	// optional EMP
	emp_intensity = 0;
	emp_duration = 0;
	if (n != -1)
	{
		emp_intensity = eval_num(n);
		n = CDR(n);
	}
	if (n != -1)
	{
		emp_duration = eval_num(n);
		n = CDR(n);
	}


	// play sound effect ---------------------------
	if (sound_index >= 0)
	{
		snd_play_3d( &Snds[sound_index], &origin, &View_position, 0.0f, NULL, 0, 1.0f, SND_PRIORITY_MUST_PLAY  );
	}


	// create the fireball -------------------------
	if (explosion_size && inner_radius && outer_radius)
	{
		if(fireball_type == FIREBALL_EXPLOSION_MEDIUM)
			fireball_create( &origin, fireball_type, FIREBALL_MEDIUM_EXPLOSION, -1, (float)explosion_size );
		else
			fireball_create( &origin, fireball_type, FIREBALL_LARGE_EXPLOSION, -1, (float)explosion_size );
	}

	// apply area affect damage --------------------
	if (max_damage || max_blast)
	{
		if ( shockwave_speed > 0 )
		{
			sci.inner_rad = (float)inner_radius;
			sci.outer_rad = (float)outer_radius;
			sci.blast = (float)max_blast;
			sci.damage = (float)max_damage;
			sci.speed = (float)shockwave_speed;
			sci.rot_angles.p = frand_range(0.0f, 1.99f*PI);
			sci.rot_angles.b = frand_range(0.0f, 1.99f*PI);
			sci.rot_angles.h = frand_range(0.0f, 1.99f*PI);
			shockwave_create(-1, &origin, &sci, SW_SHIP_DEATH);
		}
		else
		{
			object *objp;
			float t_blast = 0.0f;
			float t_damage = 0.0f;
			for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
			{
				if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID) )
				{
					continue;
				}
		
				// don't blast navbuoys
				if ( objp->type == OBJ_SHIP )
				{
					if ( ship_get_SIF(objp->instance) & SIF_NAVBUOY )
					{
						continue;
					}
				}

				if ( ship_explode_area_calc_damage( &origin, &objp->pos, (float)inner_radius, (float)outer_radius, (float)max_damage, (float)max_blast, &t_damage, &t_blast ) == -1 )
				{
					continue;
				}
	
				switch ( objp->type )
				{
					case OBJ_SHIP:
						ship_apply_global_damage( objp, NULL, &origin, t_damage );
						vec3d force, vec_ship_to_impact;
						vm_vec_sub( &vec_ship_to_impact, &objp->pos, &origin );
						if (!IS_VEC_NULL_SQ_SAFE( &vec_ship_to_impact )) {
							vm_vec_copy_normalize( &force, &vec_ship_to_impact );
							vm_vec_scale( &force, (float)max_blast );
							ship_apply_whack( &force, &vec_ship_to_impact, objp );
						}
						break;

					case OBJ_ASTEROID:
						asteroid_hit(objp, NULL, NULL, t_damage);
						break;
	
					default:
						Int3();
						break;
				}
			}	// end for
		}
	}


	// apply emp damage if applicable --------------
	if (emp_intensity && emp_duration)
	{
		emp_apply(&origin, (float)inner_radius, (float)outer_radius, (float)emp_intensity, (float)emp_duration);
	}
}

// Goober5000
void sexp_warp_effect(int n)
/* From the SEXP help...
	{ OP_WARP_EFFECT, "warp-effect\r\n"
		"\tCauses a subspace warp effect at a given origin, facing toward a given location, with the given parameters.  "
		"Takes 12 arguments...\r\n"
		"\t1:  Origin X\r\n"
		"\t2:  Origin Y\r\n"
		"\t3:  Origin Z\r\n"
		"\t4:  Location X\r\n"
		"\t5:  Location Y\r\n"
		"\t6:  Location Z\r\n"
		"\t7:  Radius\r\n"
		"\t8:  Duration in seconds\r\n"
		"\t9:  Warp opening sound (index into sounds.tbl)\r\n"
		"\t10: Warp closing sound (index into sounds.tbl)\r\n"
		"\t11: Type (0 for standard blue [default], 1 for Knossos green)\r\n"
		"\t12: Shape (0 for 2-D [default], 1 for 3-D)" },
*/
{
	vec3d origin, location, v_orient;
	matrix m_orient;
	int radius, duration, warp_open_sound_index, warp_close_sound_index, fireball_type, extra_flags;
	extra_flags = FBF_WARP_VIA_SEXP;

	// read in data --------------------------------
	origin.xyz.x = (float)eval_num(n);
	n = CDR(n);
	origin.xyz.y = (float)eval_num(n);
	n = CDR(n);
	origin.xyz.z = (float)eval_num(n);
	n = CDR(n);

	location.xyz.x = (float)eval_num(n);
	n = CDR(n);
	location.xyz.y = (float)eval_num(n);
	n = CDR(n);
	location.xyz.z = (float)eval_num(n);
	n = CDR(n);

	radius = eval_num(n);
	n = CDR(n);
	duration = eval_num(n);
	if (duration < 4) duration = 4;
	n = CDR(n);

	warp_open_sound_index = eval_num(n);
	n = CDR(n);
	warp_close_sound_index = eval_num(n);
	n = CDR(n);

	// fireball type
	if (eval_num(n) == 0)
	{
		fireball_type = FIREBALL_WARP;
	}
	else if (eval_num(n) == 1)
	{
		fireball_type = FIREBALL_KNOSSOS;
	}
	else
	{
		Warning(LOCATION, "warp-effect type is out of range; quitting the warp...\n");
		return;
	}
	n = CDR(n);

	// shape
	if (eval_num(n) == 0)
	{
		// do nothing; this is standard
	}
	else if (eval_num(n) == 1)
	{
		extra_flags |= FBF_WARP_3D;
	}
	else
	{
		Warning(LOCATION, "warp-effect shape is out of range; quitting the warp...\n");
		return;
	}


	// calculate orientation matrix ----------------
	memset(&v_orient, 0, sizeof(vec3d));

	vm_vec_sub(&v_orient, &location, &origin);

	if (IS_VEC_NULL_SQ_SAFE(&v_orient))
	{
		Warning(LOCATION, "error in warp-effect: warp can't point to itself; quitting the warp...\n");
		return;
	}

	vm_vector_2_matrix(&m_orient, &v_orient, NULL, NULL);

	// create fireball -----------------------------

	fireball_create(&origin, fireball_type, FIREBALL_WARP_EFFECT, -1, (float)radius, 0, NULL, (float)duration, -1, &m_orient, 0, extra_flags, warp_open_sound_index, warp_close_sound_index);
}

// this function get called by send-message or send-message random with the name of the message, sender,
// and priority.
void sexp_send_one_message( char *name, char *who_from, char *priority, int group, int delay )
{
	int ipriority, num, ship_index, source;
	ship *shipp;

	if(physics_paused){
		return;
	}

	Assert( (name != NULL) && (who_from != NULL) && (priority != NULL) );

	// determine the priority of the message
	if ( !stricmp(priority, "low") )
		ipriority = MESSAGE_PRIORITY_LOW;
	else if ( !stricmp(priority, "normal") )
		ipriority = MESSAGE_PRIORITY_NORMAL;
	else if ( !stricmp(priority, "high") )
		ipriority = MESSAGE_PRIORITY_HIGH;
	else {
		Int3();
		ipriority = MESSAGE_PRIORITY_NORMAL;
	}

	// check to see if the 'who_from' string is a ship that had been destroyed or departed.  If so,
	// then don't send the message.  We must look at 'who_from' to determine what to look for.  who_from
	// may be any allied person, any wingman, a wingman from a specific wing, or a specific ship
	ship_index = -1;
	shipp = NULL;
	source = MESSAGE_SOURCE_COMMAND;
	if ( who_from[0] == '#' ) {
		message_send_unique_to_player( name, &(who_from[1]), MESSAGE_SOURCE_SPECIAL, ipriority, group, delay );
		return;
	} else if (!stricmp(who_from, "<any allied>")) {
		return;
	} else if ( (num = wing_name_lookup(who_from)) != -1 ) {
		// message from a wing
		// choose wing leader to speak for wing (hence "1" at end of ship_get_random_ship_in_wing)
		ship_index = ship_get_random_ship_in_wing( num, SHIP_GET_UNSILENCED, 1 );
		if ( ship_index == -1 ) {
			if ( ipriority != MESSAGE_PRIORITY_HIGH )
				return;
		}

	} else if ( mission_log_get_time(LOG_SHIP_DESTROYED, who_from, NULL, NULL) || mission_log_get_time(LOG_SHIP_DEPARTED, who_from, NULL, NULL) 
		|| mission_log_get_time(LOG_WING_DESTROYED, who_from, NULL, NULL) || mission_log_get_time(LOG_WING_DEPARTED, who_from, NULL, NULL) ) {
		// getting into this if statement means that the ship or wing (sender) is no longer in the mission
		// if message is high priority, make it come from Terran Command
		if ( ipriority != MESSAGE_PRIORITY_HIGH )
			return;
		
		source = MESSAGE_SOURCE_COMMAND;

	} else if ( !stricmp(who_from, "<any wingman>") || (wing_name_lookup(who_from) != -1) ) {
		source = MESSAGE_SOURCE_WINGMAN;
	} else if ( !stricmp(who_from, "<none>") ) {
		source = MESSAGE_SOURCE_NONE;
	} else {
		// Message from a apecific ship
		source = MESSAGE_SOURCE_SHIP;
		ship_index = ship_name_lookup(who_from);
		if ( ship_index == -1 ) {
			// bail if not high priority, otherwise reroute to command
			if ( ipriority != MESSAGE_PRIORITY_HIGH )
				return;
			source = MESSAGE_SOURCE_COMMAND;
		}
	}

	if ( ship_index == -1 ){
		shipp = NULL;
	} else {
		shipp = &Ships[ship_index];
	}

	message_send_unique_to_player( name, shipp, source, ipriority, group, delay );
}

void sexp_send_message(int n)
{
	char *name, *who_from, *priority, *tmp;

	if(physics_paused){
		return;
	}

	Assert ( n != -1 );
	who_from = CTEXT(n);
	priority = CTEXT(CDR(n));
	name = CTEXT(CDR(CDR(n)));

	// a temporary check to see if the name field matched a priority since I am in the process
	// of reordering the arguments
	if ( !stricmp(name, "low") || !stricmp(name, "normal") || !stricmp(name, "high") ) {
		tmp = name;
		name = priority;
		priority = tmp;
	}

	sexp_send_one_message( name, who_from, priority, 0, 0 );
}

void sexp_send_message_list(int n)
{
	char *name, *who_from, *priority;
	int delay;

	if(physics_paused){
		return;
	}

	// send a bunch of messages
	delay = 0;
	while(n != -1){
		who_from = CTEXT(n);

		// next node
		n = CDR(n);
		if(n == -1){
			Warning(LOCATION, "Detected incomplete parameter list in sexp-send-message-list");
			return;
		}
		priority = CTEXT(n);

		// next node
		n = CDR(n);
		if(n == -1){
			Warning(LOCATION, "Detected incomplete parameter list in sexp-send-message-list");
			return;
		}
		name = CTEXT(n);

		// next node
		n = CDR(n);
		if(n == -1){
			Warning(LOCATION, "Detected incomplete parameter list in sexp-send-message-list");
			return;
		}
		delay += eval_num(n);

		// send the message
		sexp_send_one_message(name, who_from, priority, 1, delay);

		// next node
		n = CDR(n);
	}
}

void sexp_send_random_message(int n)
{
	char *name, *who_from, *priority;
	int temp, num_messages, message_num;

	Assert ( n != -1 );
	who_from = CTEXT(n);
	priority = CTEXT(CDR(n));

	if(physics_paused){
		return;
	}

	// count the number of messages that we have
	n = CDR(CDR(n));
	temp = n;
	num_messages = 0;
	while ( n != -1 ) {
		n = CDR(n);
		num_messages++;
	}
	Assert ( num_messages >= 1 );
	
	// get a random message, and pass the parameters to send_one_message
	message_num = myrand() % num_messages;
	n = temp;
	while ( n != -1 ) {
		if ( message_num == 0 )
			break;
		message_num--;
		n = CDR(n);
	}
	Assert (n != -1);		// should have found the message!!!
	name = CTEXT(n);

	sexp_send_one_message( name, who_from, priority, 0, 0 );
}

void sexp_self_destruct(int node)
{
	int n, ship_num;

	for (n = node; n != -1; n = CDR(n))	{
		// get the ship
		ship_num = ship_name_lookup(CTEXT(n));

		// if it still exists, destroy it
		if (ship_num >= 0) {
			ship_self_destruct(&Objects[Ships[ship_num].objnum]);
		}
	}
}

void sexp_next_mission(int n)
{
	char *mission_name;
	int i;

	mission_name = CTEXT(n);

	if (mission_name == NULL) {
		Error( LOCATION, "Mission name is NULL in campaign file for next-mission command!");
	}

	for (i = 0; i < Campaign.num_missions; i++) {
		if ( !stricmp(Campaign.missions[i].name, mission_name) ) {
			Campaign.next_mission = i;
			return;
		}
	}
	Error(LOCATION, "Mission name %s not found in campaign file for next-mission command", mission_name);
}

/**
 * Deal with the end-of-campaign sexpression. 
 */
void sexp_end_of_campaign(int n)
{
	// this is really a do-nothing sexpression.  It is pretty much a placeholder to allow
	// campaigns to have repeat-mission branches at the end of the campaign.  By not setting
	// anything in this function, the higher level campaign code will see this as end-of-campaign
	// since next_mission isn't set to anything.  (To be safe, we'll set to -1).
	Campaign.next_mission = -1;	
}

// sexpression to end everything.  One parameter is the movie to play when this is over.
// Goober5000 - edited to only to the FS2-specific code when actually ending the FS2 main
// campaign, and otherwise to do the conventional code
void sexp_end_campaign(int n)
{
	if (!(Game_mode & GM_CAMPAIGN_MODE)) {
		return;
	}

	// in FS2 our ending is a bit wacky. we'll just flag the mission as having ended the campaign	
	//
	// changed this to check for an active supernova rather than a special campaign since the supernova
	// code needs special time to execute and will post GS_EVENT_END_CAMPAIGN with Game_mode check
	// or show death-popup when it's done - taylor
	if (supernova_active() /*&& !stricmp(Campaign.filename, "freespace2")*/) {
		Campaign_ending_via_supernova = 1;
	} else {
		// post and event to move us to the end-of-campaign state
		gameseq_post_event(GS_EVENT_END_CAMPAIGN);
	}
}

/**
 * Reduces the strength of a subsystem by the given percentage.
 *
 * If it is reduced to below 0%, then the hits of the subsystem are set to 0
 */
void sexp_sabotage_subsystem(int n)
{
	char *shipname, *subsystem;
	int	percentage, shipnum, index, generic_type;
	float sabotage_hits;
	ship	*shipp;
	ship_subsys *ss = NULL, *ss_start;
	bool do_loop = true;

	shipname = CTEXT(n);
	subsystem = CTEXT(CDR(n));
	percentage = eval_num(CDR(CDR(n)));

	shipnum = ship_name_lookup(shipname);
	
	// if no ship, then return immediately.
	if ( shipnum == -1 )
		return;
	shipp = &Ships[shipnum];

	// see if we are dealing with the HULL
	if ( !stricmp( subsystem, SEXP_HULL_STRING) ) {
		float ihs;
		object *objp;

		ihs = shipp->ship_max_hull_strength;
		sabotage_hits = ihs * ((float)percentage / 100.0f);
		objp = &Objects[shipp->objnum];
		objp->hull_strength -= sabotage_hits;

		// self destruct the ship if <= 0.
		if ( objp->hull_strength <= 0.0f )
			ship_self_destruct( objp );
		return;
	}

	// see if we are dealing with the Simulated HULL
	if ( !stricmp( subsystem, SEXP_SIM_HULL_STRING) ) {
		float ihs;
		object *objp;

		ihs = shipp->ship_max_hull_strength;
		sabotage_hits = ihs * ((float)percentage / 100.0f);
		objp = &Objects[shipp->objnum];
		objp->sim_hull_strength -= sabotage_hits;

		return;
	}

	// now find the given subsystem on the ship.  This could be a generic type like <All Engines>
	generic_type = get_generic_subsys(subsystem);
	ss_start = GET_FIRST(&shipp->subsys_list); 

	while (do_loop) {
		if (generic_type) {
			// loop until we find a subsystem of that type
			for ( ; ss_start != END_OF_LIST(&Ships[shipnum].subsys_list); ss_start = GET_NEXT(ss_start)) {
				ss = NULL;
				if (generic_type == ss_start->system_info->type) {
					ss = ss_start;
					ss_start = GET_NEXT(ss_start);
					break;
				}
			}

			// reached the end of the subsystem list 
			if (ss_start == END_OF_LIST(&Ships[shipnum].subsys_list)) {
				// If the last subsystem wasn't of interest we don't need to go any further
				if (ss == NULL) { 
					return;
				}
				do_loop = false;
			}			
		}
		else {
			do_loop = false;
			index = ship_get_subsys_index(shipp, subsystem, 1);	// Bypass any error since we supply one here
			if ( index == -1 ) {
				nprintf(("Warning", "Couldn't find subsystem %s on ship %s for sabotage subsystem\n", subsystem, shipp->ship_name));
				return;
			}
			// get the pointer to the subsystem.  Check it's current hits against it's max hits, and
			// set the strength to the given percentage if current strength is > given percentage
			ss = ship_get_indexed_subsys( shipp, index );
			if (ss == NULL) {
				nprintf(("Warning", "Nonexistent subsystem for index %d on ship %s for sabotage subsystem\n", index, shipp->ship_name));
				return;
			}
		}

		sabotage_hits = ss->max_hits * ((float)percentage / 100.0f);
		ss->current_hits -= sabotage_hits;
		if ( ss->current_hits < 0.0f )
			ss->current_hits = 0.0f;

		// maybe blow up subsys
		if (ss->current_hits <= 0) {
			do_subobj_destroyed_stuff(shipp, ss, NULL);
		}

		ship_recalc_subsys_strength( shipp );
	}
}

/**
 * Adds some percentage of hits to a subsystem.
 * 
 * Anything repaired about 100% is set to max hits
 */
void sexp_repair_subsystem(int n)
{
	char *shipname, *subsystem;
	int	percentage, shipnum, index, do_submodel_repair, generic_type;
	float repair_hits;
	ship *shipp;
	ship_subsys *ss = NULL, *ss_start;
	bool do_loop = true;

	shipname = CTEXT(n);
	subsystem = CTEXT(CDR(n));
	shipnum = ship_name_lookup(shipname);
	
	do_submodel_repair = is_sexp_true(CDDDR(n));
	
	// if no ship, then return immediately.
	if ( shipnum == -1 ) {
		return;
	}
	shipp = &Ships[shipnum];
	
	// get percentage
	percentage = eval_num(CDR(CDR(n)));

	// see if we are dealing with the HULL
	if ( !stricmp( subsystem, SEXP_HULL_STRING) ) {
		float ihs;
		object *objp;

		ihs = shipp->ship_max_hull_strength;
		repair_hits = ihs * ((float)percentage / 100.0f);
		objp = &Objects[shipp->objnum];
		objp->hull_strength += repair_hits;
		if ( objp->hull_strength > ihs )
			objp->hull_strength = ihs;
		return;
	}

	// see if we are dealing with the Simulated HULL
	if ( !stricmp( subsystem, SEXP_SIM_HULL_STRING) ) {
		float ihs;
		object *objp;

		ihs = shipp->ship_max_hull_strength;
		repair_hits = ihs * ((float)percentage / 100.0f);
		objp = &Objects[shipp->objnum];
		objp->hull_strength += repair_hits;
		if ( objp->sim_hull_strength > ihs )
			objp->sim_hull_strength = ihs;
		return;
	}

	// now find the given subsystem on the ship.This could be a generic type like <All Engines>
	generic_type = get_generic_subsys(subsystem);
	ss_start = GET_FIRST(&shipp->subsys_list); 

	while (do_loop) {
		if (generic_type) {
			// loop until we find a subsystem of that type
			for ( ; ss_start != END_OF_LIST(&Ships[shipnum].subsys_list); ss_start = GET_NEXT(ss_start)) {
				ss = NULL;
				if (generic_type == ss_start->system_info->type) {
					ss = ss_start;
					ss_start = GET_NEXT(ss_start);
					break;
				}
			}

			// reached the end of the subsystem list 
			if (ss_start == END_OF_LIST(&Ships[shipnum].subsys_list)) {
				// If the last subsystem wasn't of interest we don't need to go any further
				if (ss == NULL) { 
					return;
				}
				do_loop = false;
			}			
		}
		else {
			do_loop = false;
			index = ship_get_subsys_index(shipp, subsystem, 1);	// Bypass any error since we supply one here
			if ( index == -1 ) {
				nprintf(("Warning", "Couldn't find subsystem %s on ship %s for repair subsystem\n", subsystem, shipp->ship_name));
				return;
			}
			// get the pointer to the subsystem.  Check it's current hits against it's max hits, and
			// set the strength to the given percentage if current strength is < given percentage
			ss = ship_get_indexed_subsys( shipp, index );
			if (ss == NULL) {
				nprintf(("Warning", "Nonexistent subsystem for index %d on ship %s for repair subsystem\n", index, shipp->ship_name));
				return;
			}
		}
	
		repair_hits = ss->max_hits * ((float)percentage / 100.0f);
		ss->current_hits += repair_hits;
		if ( ss->current_hits > ss->max_hits )
			ss->current_hits = ss->max_hits;

		if ((ss->current_hits > 0) && (do_submodel_repair))
		{
			ss->submodel_info_1.blown_off = 0;
			ss->submodel_info_2.blown_off = 0;
		}

		ship_recalc_subsys_strength( shipp );
	}
}

/**
 * Set a subsystem of a ship at a specific percentage
 */
void sexp_set_subsystem_strength(int n)
{
	char *shipname, *subsystem;
	int	percentage, shipnum, index, do_submodel_repair, generic_type;
	ship *shipp;
	ship_subsys *ss = NULL, *ss_start;
	bool do_loop = true;

	shipname = CTEXT(n);
	subsystem = CTEXT(CDR(n));
	percentage = eval_num(CDR(CDR(n)));

	do_submodel_repair = is_sexp_true(CDDDR(n));

	shipnum = ship_name_lookup(shipname);
	
	// if no ship, then return immediately.
	if ( shipnum == -1 )
		return;
	shipp = &Ships[shipnum];

	if ( percentage > 100 ) {
		mprintf(("Percentage for set_subsystem_strength > 100 on ship %s for subsystem '%s'-- setting to 100\n", shipname, subsystem));
		percentage = 100;
	} else if ( percentage < 0 ) {
		mprintf(("Percantage for set_subsystem_strength < 0 on ship %s for subsystem '%s' -- setting to 0\n", shipname, subsystem));
		percentage = 0;
	}

	// see if we are dealing with the HULL
	if ( !stricmp( subsystem, SEXP_HULL_STRING) ) {
		float ihs;
		object *objp;

		objp = &Objects[shipp->objnum];

		// destroy the ship if percentage is 0
		if ( percentage == 0 ) {
			ship_self_destruct( objp );
		} else {
			ihs = shipp->ship_max_hull_strength;
			objp->hull_strength = ihs * ((float)percentage / 100.0f);
		}

		return;
	}

	// see if we are dealing with the Simulated HULL
	if ( !stricmp( subsystem, SEXP_SIM_HULL_STRING) ) {
		float ihs;
		object *objp;

		objp = &Objects[shipp->objnum];
		ihs = shipp->ship_max_hull_strength;
		objp->sim_hull_strength = ihs * ((float)percentage / 100.0f);

		return;
	}

	// now find the given subsystem on the ship.This could be a generic type like <All Engines>
	generic_type = get_generic_subsys(subsystem);
	ss_start = GET_FIRST(&shipp->subsys_list); 

	while (do_loop) {
		if (generic_type) {
			// loop until we find a subsystem of that type
			for ( ; ss_start != END_OF_LIST(&Ships[shipnum].subsys_list); ss_start = GET_NEXT(ss_start)) {
				ss = NULL;
				if (generic_type == ss_start->system_info->type) {
					ss = ss_start;
					ss_start = GET_NEXT(ss_start);
					break;
				}
			}

			// reached the end of the subsystem list 
			if (ss_start == END_OF_LIST(&Ships[shipnum].subsys_list)) {
				// If the last subsystem wasn't of interest we don't need to go any further
				if (ss == NULL) { 
					return;
				}
				do_loop = false;
			}			
		}
		else {
			do_loop = false;
			index = ship_get_subsys_index(shipp, subsystem, 1);	// Bypass any error since we supply one here
			if ( index == -1 ) {
				nprintf(("Warning", "Couldn't find subsystem %s on ship %s for set subsystem strength\n", subsystem, shipp->ship_name));
				return;
			}

			// get the pointer to the subsystem.  Check it's current hits against it's max hits, and
			// set the strength to the given percentage
			ss = ship_get_indexed_subsys( shipp, index );
			if (ss == NULL) {
				nprintf(("Warning", "Nonexistent subsystem for index %d on ship %s for set subsystem strength\n", index, shipp->ship_name));
				return;
			}
		}
		
		// maybe blow up subsys
		if (ss->current_hits > 0) {
			if (percentage < 1) {
				do_subobj_destroyed_stuff(shipp, ss, NULL);
			}
		}

		// set hit points
		ss->current_hits = ss->max_hits * ((float)percentage / 100.0f);

		if ((ss->current_hits > 0) && (do_submodel_repair))
		{
			ss->submodel_info_1.blown_off = 0;
			ss->submodel_info_2.blown_off = 0;
		}

		ship_recalc_subsys_strength( shipp );
	}
}

/**
 * Changes the validity of a goal.
 * 
 * The flag paramater tells us whether to mark the goals as valid or invalid
 */
void sexp_change_goal_validity( int n, int flag )
{
	char *name;

	while ( n != -1 ) {
		name = CTEXT(n);
		if ( flag )
			mission_goal_mark_valid( name );
		else
			mission_goal_mark_invalid( name );

		n = CDR(n);
	}
}

// Goober5000
// yeesh - be careful of the cargo-no-deplete flag :p
int sexp_is_cargo(int n)
{
	char *cargo, *ship, *subsystem;
	int ship_num, cargo_index;

	cargo = CTEXT(n);
	ship = CTEXT(CDR(n));
	if (CDR(CDR(n)) != -1)
		subsystem = CTEXT(CDR(CDR(n)));
	else
		subsystem = NULL;

	cargo_index = -1;

	// find ship
	ship_num = ship_name_lookup(ship);

	// in-mission?
	if (ship_num != -1)
	{
		if (subsystem)
		{
			// find the ship subsystem
			ship_subsys *ss = ship_get_subsys(&Ships[ship_num], subsystem);
			if (ss != NULL)
			{
				// set cargo
				cargo_index = ss->subsys_cargo_name;
			}
		}
		else
		{
			cargo_index = Ships[ship_num].cargo1;
		}
	}
	else
	{
		// can't check subsys of ships not in mission
		if (subsystem)
		{
			return SEXP_CANT_EVAL;
		}

		// departed?
		int exited_index = ship_find_exited_ship_by_name(ship);
		if (exited_index != -1)
		{
			cargo_index = Ships_exited[exited_index].cargo1;
		}
		// not arrived yet
		else
		{
			p_object *p_objp;

			// find cargo for the parse object
			p_objp = mission_parse_get_arrival_ship(ship);
			Assert (p_objp);
			cargo_index = (int) p_objp->cargo1;
		}
	}

	// did we get any cargo
	if (cargo_index < 0)
		return SEXP_FALSE;

	// check cargo
	if (!stricmp(Cargo_names[cargo_index & CARGO_INDEX_MASK], cargo))
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

// Goober5000
// yeesh - be careful of the cargo-no-deplete flag :p
void sexp_set_cargo(int n)
{
	char *cargo, *ship, *subsystem;
	int ship_num, i, cargo_index;

	cargo = CTEXT(n);
	ship = CTEXT(CDR(n));
	if (CDR(CDR(n)) != -1)
		subsystem = CTEXT(CDR(CDR(n)));
	else
		subsystem = NULL;

	// check to see if ship destroyed or departed.  In either case, do nothing.
	if ( mission_log_get_time(LOG_SHIP_DEPARTED, ship, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship, NULL, NULL) )
		return;

	cargo_index = -1;
	// find this cargo in the cargo list
	for (i = 0; i < Num_cargo; i++)
	{
		// found it?
		if (!stricmp(cargo, Cargo_names[i]))
		{
			cargo_index = i;
			break;
		}
	}

	// not found
	if (cargo_index == -1)
	{
		// make new entry if possible
		if (Num_cargo + 1 >= MAX_CARGO)
		{
			Warning(LOCATION, "set-cargo: Maximum number of cargo names (%d) reached.  Ignoring new name.\n", MAX_CARGO);
			return;
		}

		Assert(strlen(cargo) <= NAME_LENGTH - 1);

		cargo_index = Num_cargo;
		Num_cargo++;

		strcpy(Cargo_names[cargo_index], cargo);
	}

	// get the ship
	ship_num = ship_name_lookup(ship);

	// we can only set subsystems if the ship is in the mission
	if (ship_num != -1)
	{
		if (subsystem)
		{
			ship_subsys *ss = ship_get_subsys(&Ships[ship_num], subsystem);
			if (ss == NULL) {
				Assert (!ship_class_unchanged(ship_num));
				return;
			}

			// set cargo
			ss->subsys_cargo_name = cargo_index | (ss->subsys_cargo_name & CARGO_NO_DEPLETE);
		}
		else
		{
			// simply set the ship cargo
			Ships[ship_num].cargo1 = char(cargo_index | (Ships[ship_num].cargo1 & CARGO_NO_DEPLETE));
		}
	}
	else
	{
		if (!subsystem)
		{
			p_object *p_objp;

			// set cargo for the parse object
			p_objp = mission_parse_get_arrival_ship(ship);
			Assert (p_objp);
			p_objp->cargo1 = char(cargo_index | (p_objp->cargo1 & CARGO_NO_DEPLETE));
		}
	}
}

/**
 * Transfer cargo from one ship to another
 */
void sexp_transfer_cargo(int n)
{
	char *shipname1, *shipname2;
	int shipnum1, shipnum2, i;

	shipname1 = CTEXT(n);
	shipname2 = CTEXT(CDR(n));

	// find the ships -- if neither in the mission, the abort
	shipnum1 = ship_name_lookup(shipname1);
	shipnum2 = ship_name_lookup(shipname2);
	if ( (shipnum1 == -1) || (shipnum2 == -1) )
		return;

	// we must be sure that these two objects are indeed docked
	if (!dock_check_find_direct_docked_object(&Objects[Ships[shipnum1].objnum], &Objects[Ships[shipnum2].objnum]))
	{
		return;
	}

	if ( !stricmp(Cargo_names[Ships[shipnum1].cargo1 & CARGO_INDEX_MASK], "nothing") ) {
		return;
	}

	// transfer cargo from ship1 to ship2
#ifndef NDEBUG
	// Don't give warning for large ships (cruiser on up) 
	if (! (Ship_info[Ships[shipnum2].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
		if ( stricmp(Cargo_names[Ships[shipnum2].cargo1 & CARGO_INDEX_MASK], "nothing") ) {
			Warning(LOCATION, "Transfering cargo to %s which already\nhas cargo %s.\nCargo will be replaced", Ships[shipnum2].ship_name, Cargo_names[Ships[shipnum2].cargo1 & CARGO_INDEX_MASK] );
		}
	}
#endif
	Ships[shipnum2].cargo1 = char((Ships[shipnum1].cargo1 & CARGO_INDEX_MASK) | (Ships[shipnum2].cargo1 & CARGO_NO_DEPLETE));

	if ( !(Ships[shipnum1].cargo1 & CARGO_NO_DEPLETE) ) {
		// need to set ship1's cargo to nothing.  scan the cargo_names array looking for the string nothing.
		// add it if not found
		for (i = 0; i < Num_cargo; i++ ) {
			if ( !stricmp(Cargo_names[i], "nothing") ) {
				Ships[shipnum1].cargo1 = char(i);
				return;
			}
		}
		strcpy(Cargo_names[i], "Nothing");
		Num_cargo++;
	}
}

/**
 * Exchanges cargo between two ships
 */
void sexp_exchange_cargo(int n)
{
	char *shipname1, *shipname2;
	int shipnum1, shipnum2, temp;

	shipname1 = CTEXT(n);
	shipname2 = CTEXT(CDR(n));

	// find the ships -- if neither in the mission, abort
	shipnum1 = ship_name_lookup(shipname1);
	shipnum2 = ship_name_lookup(shipname2);
	if ( (shipnum1 == -1) || (shipnum2 == -1) )
		return;

	// we must be sure that these two objects are indeed docked
	if (!dock_check_find_direct_docked_object(&Objects[Ships[shipnum1].objnum], &Objects[Ships[shipnum2].objnum]))
	{
		Int3();			// you are trying to transfer cargo between two ships not docked
		return;
	}

	temp = (Ships[shipnum1].cargo1 & CARGO_INDEX_MASK);
	Ships[shipnum1].cargo1 = char(Ships[shipnum2].cargo1 & CARGO_INDEX_MASK);
	Ships[shipnum2].cargo1 = char(temp);
}

void sexp_cap_waypoint_speed(int n)
{
	char *shipname;
	int shipnum;
	int speed;

	shipname = CTEXT(n);
	speed = eval_num(CDR(n));

	shipnum = ship_name_lookup(shipname);

	if (shipnum == -1) {
		// trying to set waypoint speed of ship not already in game
		return;
	}

	// cap speed to range (-1, 32767) to store within int
	if (speed < 0) {
		speed = -1;
	}

	if (speed > 32767) {
		speed = 32767;
	}

	Ai_info[Ships[shipnum].ai_index].waypoint_speed_cap = speed;
}

/**
 * Causes a ship to jettison its cargo
 */
void sexp_jettison_cargo(int n)
{
	char *shipname;
	int jettison_delay, ship_index;	

	// get some data
	shipname = CTEXT(n);
	jettison_delay = eval_num(CDR(n));

	// lookup the ship
	ship_index = ship_name_lookup(shipname);
	if(ship_index < 0)
		return;
	object *parent_objp = &Objects[Ships[ship_index].objnum];

	n = CDDR(n);

	// no arguments - jettison all docked objects
	if (n == -1)
	{
		// Goober5000 - as with ai_deathroll_start, we can't simply iterate through the dock list while we're
		// undocking things.  So just repeatedly jettison the first object.
		while (object_is_docked(parent_objp))
		{
			object_jettison_cargo(parent_objp, dock_get_first_docked_object(parent_objp));
		}
	}
	// arguments - jettison only those objects
	else
	{
		for (; n != -1; n = CDR(n))
		{
			// make sure ship exists
			ship_index = ship_name_lookup(CTEXT(n));
			if (ship_index < 0)
				continue;

			// make sure we are docked to it
			if (!dock_check_find_direct_docked_object(parent_objp, &Objects[Ships[ship_index].objnum]))
				continue;

			object_jettison_cargo(parent_objp, &Objects[Ships[ship_index].objnum]);			
		}
	}
}

void sexp_set_docked(int n)
{
	// get some data
	char* docker_ship_name = CTEXT(n);
	n = CDR(n);
	char* docker_point_name = CTEXT(n);
	n = CDR(n);
	char* dockee_ship_name = CTEXT(n);
	n = CDR(n);
	char* dockee_point_name = CTEXT(n);
	n = CDR(n);

	// lookup the ships
	int docker_ship_index = ship_name_lookup(docker_ship_name);
	int dockee_ship_index = ship_name_lookup(dockee_ship_name);
	if(docker_ship_index < 0 || dockee_ship_index < 0)
		return;

	ship* docker_ship = &Ships[docker_ship_index];
	ship* dockee_ship = &Ships[dockee_ship_index];
	object* docker_objp = &Objects[docker_ship->objnum];
	object* dockee_objp = &Objects[dockee_ship->objnum];

	//Get dockpoints by name
	int docker_point_index = model_find_dock_name_index(Ship_info[docker_ship->ship_info_index].model_num, docker_point_name);
	int dockee_point_index = model_find_dock_name_index(Ship_info[dockee_ship->ship_info_index].model_num, dockee_point_name);

	Assertion(docker_point_index >= 0, "Docker point '%s' not found on docker ship '%s'", docker_point_name, docker_ship_name);
	Assertion(dockee_point_index >= 0, "Dockee point '%s' not found on dockee ship '%s'", dockee_point_name, dockee_ship_name);

	//Make sure that the specified dockpoints are all free (if not, do nothing)
	if (dock_find_object_at_dockpoint(docker_objp, docker_point_index) != NULL || 
		dock_find_object_at_dockpoint(dockee_objp, dockee_point_index) != NULL)
	{
		return;
	}

	//Set docked
	dock_orient_and_approach(docker_objp, docker_point_index, dockee_objp, dockee_point_index, DOA_DOCK_STAY);
	ai_do_objects_docked_stuff(docker_objp, docker_point_index, dockee_objp, dockee_point_index, true);
}

void sexp_cargo_no_deplete(int n)
{
	char *shipname;
	int ship_index, no_deplete = 1;

	// get some data
	shipname = CTEXT(n);

	// lookup the ship
	ship_index = ship_name_lookup(shipname);
	if(ship_index < 0){
		return;
	}

	if ( !(Ship_info[Ships[ship_index].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
		Warning(LOCATION, "Trying to make non BIG or HUGE ship %s with non-depletable cargo.\n", Ships[ship_index].ship_name);
		return;
	}

	if (CDR(n) != -1) {
		no_deplete = eval_num(CDR(n));
		Assert((no_deplete == 0) || (no_deplete == 1));
		if ( (no_deplete != 0) && (no_deplete != 1) ) {
			no_deplete = 1;
		}
	}

	if (no_deplete) {
		Ships[ship_index].cargo1 |= CARGO_NO_DEPLETE;
	} else {
		Ships[ship_index].cargo1 &= (~CARGO_NO_DEPLETE);
	}

}

// Goober5000
void sexp_force_jump()
{
	// Shouldn't be gliding now....
	Player_obj->phys_info.flags &= ~PF_GLIDING;
	Player_obj->phys_info.flags &= ~PF_FORCE_GLIDE;

	if (Game_mode & GM_MULTIPLAYER) {
		multi_handle_end_mission_request(); 
	}
	else {
		// forced warp, taken from training failure code
		gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_START_FORCED );	//	Force player to warp out.
	}

}

void sexp_mission_set_nebula(int n)
{
	stars_set_nebula(eval_num(n) > 0);
}

/* freespace.cpp does not have these availiable externally, and we must call
them so that the main simulation loop does not have to constantly check for
Game_subspace_effect so that it could turn on the subspace sounds.

Because these are in freespace.cpp there are also stubs of these functions
in fred.cpp as it does not deal with the game loop (obviously) but still
links against code.lib. */
extern void game_start_subspace_ambient_sound();
extern void game_stop_subspace_ambient_sound();

void sexp_mission_set_subspace(int n)
{
	if (eval_num(n) > 0) {
		The_mission.flags |= MISSION_FLAG_SUBSPACE;
		Game_subspace_effect = 1;
		game_start_subspace_ambient_sound();
	} else {
		The_mission.flags &= ~MISSION_FLAG_SUBSPACE;
		Game_subspace_effect = 0;
		game_stop_subspace_ambient_sound();
	}
}

void sexp_add_background_bitmap(int n)
{
	int sexp_var;
	int new_number;
	char number_as_str[TOKEN_LENGTH];
	starfield_list_entry sle;

	// filename
	strcpy_s(sle.filename, CTEXT(n));
	n = CDR(n);

	// sanity checking
	if (stars_find_bitmap(sle.filename) < 0)
	{
		Error(LOCATION, "sexp-add-background-bitmap: Background bitmap %s not found!", sle.filename);
		return;
	}

	// angles
	sle.ang.p = fl_radians(eval_num(n) % 360);
	n = CDR(n);
	sle.ang.b = fl_radians(eval_num(n) % 360);
	n = CDR(n);
	sle.ang.h = fl_radians(eval_num(n) % 360);
	n = CDR(n);

	// scale
	sle.scale_x = eval_num(n) / 100.0f;
	n = CDR(n);
	sle.scale_y = eval_num(n) / 100.0f;
	n = CDR(n);

	// div
	sle.div_x = eval_num(n);
	n = CDR(n);
	sle.div_y = eval_num(n);
	n = CDR(n);

	// restrict parameters
	if (sle.scale_x > 18) sle.scale_x = 18;
	if (sle.scale_x < 0.1f) sle.scale_x = 0.1f;
	if (sle.scale_y > 18) sle.scale_y = 18;
	if (sle.scale_y < 0.1f) sle.scale_y = 0.1f;
	if (sle.div_x > 5) sle.div_x = 5;
	if (sle.div_x < 1) sle.div_x = 1;
	if (sle.div_y > 5) sle.div_y = 5;
	if (sle.div_y < 1) sle.div_y = 1;

	Assert((n >= 0) && (n < Num_sexp_nodes));

	// ripped from sexp_modify_variable()
	// get sexp_variable index
	Assert(Sexp_nodes[n].first == -1);
	sexp_var = atoi(Sexp_nodes[n].text);
	
	// verify variable set
	Assert(Sexp_variables[sexp_var].type & SEXP_VARIABLE_SET);

	if (Sexp_variables[sexp_var].type & SEXP_VARIABLE_NUMBER)
	{
        new_number = stars_add_bitmap_entry(&sle);
        if (new_number < 0)
        {
		    Warning(LOCATION, "Unable to add starfield bitmap: '%s'!", sle.filename);
            new_number = 0;
        }

		sprintf(number_as_str, "%d", new_number);

		// assign to variable
		sexp_modify_variable(number_as_str, sexp_var);
	}
	else
	{
		Error(LOCATION, "sexp-add-background-bitmap: Variable %s must be a number variable!", Sexp_variables[sexp_var].variable_name);
		return;
	}
}

void sexp_remove_background_bitmap(int n)
{
	int slot = eval_num(n);

	if (slot >= 0) {
		int instances = stars_get_num_bitmaps();
		if ( instances > slot ) {
			stars_mark_bitmap_unused( slot );
		} else {
			Error(LOCATION, "remove-background-bitmap: slot %d does not exist. Slot must be less than %d.",
				slot, instances);
		}
	}
}

void sexp_add_sun_bitmap(int n)
{
	int sexp_var;
	int new_number;
	char number_as_str[TOKEN_LENGTH];
	starfield_list_entry sle;

	// filename
	strcpy_s(sle.filename, CTEXT(n));
	n = CDR(n);

	// sanity checking
	if (stars_find_sun(sle.filename) < 0)
	{
		Error(LOCATION, "sexp-add-sun-bitmap: Sun %s not found!", sle.filename);
		return;
	}

	// angles
	sle.ang.p = fl_radians(eval_num(n) % 360);
	n = CDR(n);
	sle.ang.b = fl_radians(eval_num(n) % 360);
	n = CDR(n);
	sle.ang.h = fl_radians(eval_num(n) % 360);
	n = CDR(n);

	// scale
	sle.scale_x = eval_num(n) / 100.0f;
	n = CDR(n);
	sle.scale_y = sle.scale_x;

	// div
	sle.div_x = 1;
	sle.div_y = 1;

	// restrict parameters
	if (sle.scale_x > 50) sle.scale_x = 50;
	if (sle.scale_x < 0.1f) sle.scale_x = 0.1f;
	if (sle.scale_y > 50) sle.scale_y = 50;
	if (sle.scale_y < 0.1f) sle.scale_y = 0.1f;
	
	Assert((n >= 0) && (n < Num_sexp_nodes));

	// ripped from sexp_modify_variable()
	// get sexp_variable index
	Assert(Sexp_nodes[n].first == -1);
	sexp_var = atoi(Sexp_nodes[n].text);
	
	// verify variable set
	Assert(Sexp_variables[sexp_var].type & SEXP_VARIABLE_SET);

	if (Sexp_variables[sexp_var].type & SEXP_VARIABLE_NUMBER)
	{
		// get new numerical value
        new_number = stars_add_sun_entry(&sle);

        if (new_number < 0)
        {
		    Warning(LOCATION, "Unable to add sun: '%s'!", sle.filename);
            new_number = 0;
        }

		sprintf(number_as_str, "%d", new_number);

		// assign to variable
		sexp_modify_variable(number_as_str, sexp_var);
	}
	else
	{
		Error(LOCATION, "sexp-add-sun-bitmap: Variable %s must be a number variable!", Sexp_variables[sexp_var].variable_name);
		return;
	}
}

void sexp_remove_sun_bitmap(int n)
{
	int slot = eval_num(n);

	if (slot >= 0) {
		int instances = stars_get_num_suns();
		if ( instances > slot ) {
			stars_mark_sun_unused( slot );
		} else {
			Error(LOCATION, "remove-sun-bitmap: slot %d does not exist. Slot must be less than %d.",
				slot, instances);
		}
	}
}

void sexp_nebula_change_storm(int n)
{
	if (!(The_mission.flags & MISSION_FLAG_FULLNEB)) return;
	
	nebl_set_storm(CTEXT(n));
}

void sexp_nebula_toggle_poof(int n)
{
	char *name = CTEXT(n);
	int result = is_sexp_true(CDR(n));
	int i;

	if (name == NULL) return;

	for (i = 0; i < MAX_NEB2_POOFS; i++)
	{
		if (!stricmp(name,Neb2_poof_filenames[i])) break;
	}

	//coulnd't find the poof
	if (i == MAX_NEB2_POOFS) return;

	if (result) Neb2_poof_flags |= (1 << i);
	else Neb2_poof_flags &= ~(1 << i);
	
	neb2_eye_changed();
}

/**
 * End the mission.
 *
 * Implemented by Sesquipedalian; fixed by EdrickV; enhanced by others
 */
void sexp_end_mission(int n)
{
	int ignore_player_mortality = 1;
	int boot_to_main_hall = 0;

	if (n != -1) {
		ignore_player_mortality = is_sexp_true(n);
		n = CDR(n);
	}
	if (n != -1) {
		boot_to_main_hall = is_sexp_true(n);
		n = CDR(n);
	}

	// if the player is dead we may want to let the death screen handle things
	if (!ignore_player_mortality && (Player_ship->flags & SF_DYING)) {
		return;
	}

	// if we go straight to the main hall we have to clean up the mission without entering the debriefing
	if (boot_to_main_hall && !(Game_mode & GM_MULTIPLAYER)) {
		gameseq_post_event(GS_EVENT_MAIN_MENU);
	} else {
		send_debrief_event();
	}

	// Karajorma - callback all the clients here. 
	if (MULTIPLAYER_MASTER)
	{
		multi_handle_sudden_mission_end();
		send_force_end_mission_packet();
	}
}

// Goober5000
void sexp_set_debriefing_toggled(int node)
{
	if (is_sexp_true(node))
		The_mission.flags |= MISSION_FLAG_TOGGLE_DEBRIEFING;
	else
		The_mission.flags &= ~MISSION_FLAG_TOGGLE_DEBRIEFING;
}

/**
 * Toggle the status bit for the AI code which tells the AI if it is a good time to rearm.
 *
 * The status being set means good time.  Status not being set (unset), means bad time. Designers must implement this.
 */
void sexp_good_time_to_rearm(int n)
{
	int team, time;

	team = iff_lookup(CTEXT(n));
	time = eval_num(CDR(n));						// this is the time for how long a good rearm is active -- in seconds

	ai_set_rearm_status(team, time);
}

/**
 * Grants promotion to the player
 */
void sexp_grant_promotion()
{
	// short circuit multiplayer for now until we figure out what to do.
	if ( Game_mode & GM_MULTIPLAYER )
		return;

	// set a bit to tell player should get promoted at the end of the mission.  I suppose the other
	// thing that we could do would be to set the players score to at least the amount of
	// points for the next level, but this way is better I think.
	if ( Game_mode & GM_CAMPAIGN_MODE ) {
		Player->flags |= PLAYER_FLAGS_PROMOTED;
	}
}

/**
 * Gives the named medal to the players in the mission
 */
void sexp_grant_medal(int n)
{
	int i;
	char *medal_name;

	// don't give medals in normal gameplay when not in campaign mode
	if ( (Game_mode & GM_NORMAL) && !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	medal_name = CTEXT(n);
	if (medal_name == NULL)
		return;

	if (Player->stats.m_medal_earned >= 0) {
		Warning(LOCATION, "Cannot grant more than one medal per mission!  New medal '%s' will replace old medal '%s'!", medal_name, Medals[Player->stats.m_medal_earned].name);
	}

	for (i = 0; i < Num_medals; i++ ) {
		if ( !stricmp(medal_name, Medals[i].name) )
			break;
	}

	if ( i < Num_medals ) {
		Player->stats.m_medal_earned = i;

		if ( Game_mode & GM_MULTIPLAYER ) {
			for ( int j = 0; j < MAX_PLAYERS; j++ ) {
				if ( MULTI_CONNECTED(Net_players[j]) ) {
					Net_players[j].m_player->stats.m_medal_earned = i;
				}
			}
		}
	}
}

void sexp_change_player_score(int node)
{
	int sindex;	
	int score;
	int plr_index;

	score = eval_num(node); 
	node = CDR(node);

	if(!(Game_mode & GM_MULTIPLAYER)){
		sindex = ship_name_lookup(CTEXT(node));

		if (Player_ship != &Ships[sindex]) {
			Warning(LOCATION, "Can not award points to '%s'. Ship is not a player!", CTEXT(node));
			return; 	
		}
		Player->stats.m_score += score; 
		if (Player->stats.m_score < 0) {
			Player->stats.m_score = 0; 
		}
	}
	else {
		while (node >= 0) {
			plr_index = multi_find_player_by_ship_name(CTEXT(node), true);
			if (plr_index >= 0) {
				Net_player[plr_index].m_player->stats.m_score += score;
				
				if (Net_player[plr_index].m_player->stats.m_score < 0) {
					Net_player[plr_index].m_player->stats.m_score = 0;
				}
			}			

			node = CDR(node);
		}
	}
}

void sexp_change_team_score(int node)
{
	int i, score, team;

	// since we only have a team score in TvT
	if ( !(MULTI_TEAM) ) {
		return;
	}

	score = eval_num(node); 
	team = eval_num(CDR(node)); 

	if (team == 0) {
		for (i = 0; i < MAX_TVT_TEAMS; i++) {
			Multi_team_score[i] += score;  
		}
	}
	else if (team > 0 && team <= MAX_TVT_TEAMS) {
		Multi_team_score[team - 1] += score;  
	}
	else {
		Warning(LOCATION, "Invalid team number. Team %d does not exist", team);
	}
}



void sexp_tech_add_ship(int node)
{
	int i;
	char *name;

	Assert(node >= 0);
	// this function doesn't mean anything when not in campaign mode
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	while (node >= 0) {
		name = CTEXT(node);
		i = ship_info_lookup(name);
		if (i >= 0)
			Ship_info[i].flags |= SIF_IN_TECH_DATABASE;
		else
			Error(LOCATION, "Ship class \"%s\" invalid", name);

		node = CDR(node);
	}
}

void sexp_tech_add_weapon(int node)
{
	int i;
	char *name;

	Assert(node >= 0);
	// this function doesn't mean anything when not in campaign mode
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	while (node >= 0) {
		name = CTEXT(node);
		i = weapon_info_lookup(name);
		if (i >= 0)
			Weapon_info[i].wi_flags |= WIF_IN_TECH_DATABASE;
		else
			Error(LOCATION, "Weapon class \"%s\" invalid", name);

		node = CDR(node);
	}
}

// Goober5000
void sexp_tech_add_intel(int node)
{
	int i;
	char *name;

	Assert(node >= 0);
	// this function doesn't mean anything when not in campaign mode
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	while (node >= 0) {
		name = CTEXT(node);
		i = intel_info_lookup(name);
		if (i >= 0)
			Intel_info[i].flags |= IIF_IN_TECH_DATABASE;
		else
			Error(LOCATION, "Intel name \"%s\" invalid", name);

		node = CDR(node);
	}
}

// Goober5000 - reset all the tech entries to their default states
void sexp_tech_reset_to_default()
{
	// this function doesn't mean anything when not in campaign mode
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	tech_reset_to_default();
}

/**
 * Set variables needed to grant a new ship/weapon to the player during the course
 * of a mission
 */
void sexp_allow_ship(int n)
{
	int idx;
	char name[NAME_LENGTH], temp[NAME_LENGTH];

	// this function doesn't mean anything when not in campaign mode
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	// get the base name of the ship
	strcpy_s(name, CTEXT(n));
	end_string_at_first_hash_symbol(name);

	// add that ship, as well as any # equivalents
	for (idx = 0; idx < Num_ship_classes; idx++)
	{
		strcpy_s(temp, Ship_info[idx].name);
		end_string_at_first_hash_symbol(temp);

		// we have a match, so allow this ship
		if (!strcmp(name, temp))
			mission_campaign_save_persistent(CAMPAIGN_PERSISTENT_SHIP, idx);
	}
}

void sexp_allow_weapon(int n)
{
	int idx;
	char name[NAME_LENGTH], temp[NAME_LENGTH];

	// this function doesn't mean anything when not in campaign mode
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	// get the base name of the weapon
	strcpy_s(name, CTEXT(n));
	end_string_at_first_hash_symbol(name);

	// add that weapon, as well as any # equivalents
	for (idx = 0; idx < Num_weapon_types; idx++)
	{
		strcpy_s(temp, Weapon_info[idx].name);
		end_string_at_first_hash_symbol(temp);

		// we have a match, so allow this weapon
		if (!strcmp(name, temp))
			mission_campaign_save_persistent(CAMPAIGN_PERSISTENT_WEAPON, idx);
	}
}

// Goober5000
// generic function for all those sexps that set flags
void sexp_deal_with_ship_flag(int node, bool process_subsequent_nodes, int object_flag, int object_flag2, int ship_flag, int ship_flag2, int p_object_flag, int p_object_flag2, bool set_it, bool send_multiplayer = false, bool include_players_in_ship_lookup = false)
{
	char *ship_name;
	int ship_index;
	int n = node;

	if (send_multiplayer && MULTIPLAYER_MASTER) {
		multi_start_callback(); 
		multi_send_int(object_flag); 
		/* Uncommenting this will break compatibility with earlier builds but it is pointless to send it until object_flag2
		is actually used by the engine 
		*/
		// multi_send_int(object_flag2); 
		multi_send_int(ship_flag); 
		multi_send_int(ship_flag2); 
		multi_send_int(p_object_flag); 
		multi_send_int(p_object_flag2); 
		multi_send_bool(set_it); 
	}

	// loop for all ships in the sexp
	// NB: if the flag is set, we will continue acting on nodes until we run out of them;
	//     if not, we will only act on the first one
	for (; n >= 0; process_subsequent_nodes ? n = CDR(n) : n = -1)
	{
		// get ship name
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if (mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL) || mission_log_get_time(LOG_SELF_DESTRUCTED, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL))
			continue;

		// see if ship exists in-mission
		ship_index = ship_name_lookup(ship_name, include_players_in_ship_lookup ? 1 : 0);

		// if ship is in-mission
		if (ship_index >= 0)
		{
			// see if we have an object flag to set
			if (object_flag)
			{
				// set or clear?
				if (set_it)
					Objects[Ships[ship_index].objnum].flags |= object_flag;
				else
					Objects[Ships[ship_index].objnum].flags &= ~object_flag;
			}

			// see if we have an object flag2 to set
			if (object_flag2)
			{
/*
				// set or clear?
				if (set_it)
					Objects[Ships[ship_index].objnum].flags2 |= object_flag2;
				else
					Objects[Ships[ship_index].objnum].flags2 &= ~object_flag2;
*/
			}

			// see if we have a ship flag to set
			if (ship_flag)
			{
				// set or clear?
				if (set_it)
					Ships[ship_index].flags |= ship_flag;
				else
					Ships[ship_index].flags &= ~ship_flag;
			}

			// see if we have a ship flag2 to set
			if (ship_flag2)
			{
				// set or clear?
				if (set_it)
					Ships[ship_index].flags2 |= ship_flag2;
				else
					Ships[ship_index].flags2 &= ~ship_flag2;
			}

			// the lock afterburner SEXP also needs to set a physics flag
			if (ship_flag2 == SF2_AFTERBURNER_LOCKED) {
				if (set_it) {
					afterburners_stop(&Objects[Ships[ship_index].objnum], 1);
				}
			}

			if (send_multiplayer && MULTIPLAYER_MASTER) {
				multi_send_bool(true); 
				multi_send_ship(ship_index); 
			}
		}
		// if it's not in-mission
		else
		{
			// grab it from the arrival list
			p_object *p_objp = mission_parse_get_arrival_ship(ship_name);
			
			// ships that have had ship-vanish used on them should be skipped
			if (!p_objp) {
				continue;
			}

			// see if we have a p_object flag to set
			if (p_object_flag)
			{
				// set or clear?
				if (set_it)
					p_objp->flags |= p_object_flag;
				else
					p_objp->flags &= ~p_object_flag;
			}

			// see if we have a p_object flag2 to set
			if (p_object_flag2)
			{
				// set or clear?
				if (set_it)
					p_objp->flags2 |= p_object_flag2;
				else
					p_objp->flags2 &= ~p_object_flag2;
			}

			if (send_multiplayer && MULTIPLAYER_MASTER) {
				multi_send_bool(false); 
				multi_send_parse_object(p_objp); 
			}
		}
	}

	if (send_multiplayer && MULTIPLAYER_MASTER) {
		multi_end_callback();
	}
}

void multi_sexp_deal_with_ship_flag() 
{
	int object_flag = 0;
	// int object_flag2 = 0; 
	int ship_flag = 0; 
	int ship_flag2 = 0;
	int p_object_flag = 0;
	int p_object_flag2 = 0;
	bool set_it = false;
	bool ship_arrived = true; 
	ship *shipp = NULL;
	p_object *pobjp = NULL;

	multi_get_int(object_flag); 
	// multi_get_int(object_flag2); 
	multi_get_int(ship_flag); 
	multi_get_int(ship_flag2); 
	multi_get_int(p_object_flag); 
	multi_get_int(p_object_flag2); 
	multi_get_bool(set_it);
 
	// if any of the above failed so will this loop
	while (multi_get_bool(ship_arrived)) 
	{

		if (ship_arrived) {
			multi_get_ship(shipp);
			if (shipp == NULL) {
				WarningEx(LOCATION, "Null ship pointer in multi_sexp_deal_with_ship_flag(), tell a coder.\n");
				return;
			}
			
			if (set_it) {
				Objects[shipp->objnum].flags |= object_flag;
				// Objects[shipp->objnum].flags2 |= object_flag2;
				shipp->flags |= ship_flag;
				shipp->flags2 |= ship_flag2;
			}
			else {
				Objects[shipp->objnum].flags &= ~object_flag;
				// Objects[shipp->objnum].flags2 &= ~object_flag2;
				shipp->flags &= ~ship_flag;
				shipp->flags2 &= ~ship_flag2;
			}

			// deal with side effects of these flags
			if (ship_flag2 == SF2_AFTERBURNER_LOCKED) {
				if (set_it) {
					afterburners_stop(&Objects[shipp->objnum], 1);
				}
			}

			if (ship_flag2 == SF2_STEALTH && !set_it) {
				if (shipp->flags & SF_ESCORT) {
					hud_add_ship_to_escort(shipp->objnum, 1);
				}			
			}
			if ((ship_flag2 == SF2_FRIENDLY_STEALTH_INVIS) || (ship_flag == SF_HIDDEN_FROM_SENSORS)) {
				if (set_it) {
					if (Player_ai->target_objnum == shipp->objnum) {
						hud_cease_targeting(); 
					}
				}
				else {
					if (shipp->flags & SF_ESCORT) {
					hud_add_ship_to_escort(shipp->objnum, 1);
					}		
				}
			}
		}
		else {
			multi_get_parse_object(pobjp); 
			if (pobjp != NULL) {
				if (set_it) {
					pobjp->flags |= p_object_flag;
					pobjp->flags2 |= p_object_flag2;
				}
				else {
					pobjp->flags &= ~p_object_flag;
					pobjp->flags2 &= ~p_object_flag2;
				}
			}
		}
	}
}
// modified by Goober5000; now it should work properly
// function to deal with breaking/fixing the warp engines on ships/wings.
// --repairable is true when we are breaking the warp drive (can be repaired)
// --damage_it is true when we are sabotaging it, one way or the other; false when fixing it
void sexp_deal_with_warp( int n, bool repairable, bool damage_it )
{
	int ship_flag, p_object_flag;

	if (repairable)
	{
		ship_flag = SF_WARP_BROKEN;
		p_object_flag = P_SF_WARP_BROKEN;
	}
	else
	{
		ship_flag = SF_WARP_NEVER;
		p_object_flag = P_SF_WARP_NEVER;
	}

	sexp_deal_with_ship_flag(n, true, 0, 0, ship_flag, 0, p_object_flag, 0, damage_it);
}

// Goober5000
void sexp_set_subspace_drive(int node)
{
	bool set_flag = !is_sexp_true(node);

	sexp_deal_with_ship_flag(CDR(node), true, 0, 0, 0, SF2_NO_SUBSPACE_DRIVE, 0, 0, set_flag);
}

/**
 * Tell the AI when it is okay to fire certain secondary weapons at other ships.
 */
void sexp_good_secondary_time(int n)
{
	char *team_name, *weapon_name, *ship_name;
	int num_weapons, weapon_index, team;

	team_name = CTEXT(n);
	num_weapons = eval_num(CDR(n));
	weapon_name = CTEXT(CDR(CDR(n)));
	ship_name = CTEXT(CDR(CDR(CDR(n))));

	weapon_index = weapon_info_lookup(weapon_name);
	if ( weapon_index == -1 ) {
		nprintf(("Warning", "couldn't find weapon %s for good-secondary-time\n", weapon_name));
		return;
	}

	// get the team type from the team_name
	team = iff_lookup(team_name);

	// see if the ship has departed or has been destroyed.  If so, then we don't need to set up the
	// AI stuff
	if ( mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
		return;

	ai_good_secondary_time( team, weapon_index, num_weapons, ship_name );
}

// Karajorma - Turns the built in messages for pilots and command on or off
void sexp_toggle_builtin_messages (int node, bool enable_messages)
{
	char *ship_name;
	int wingnum, shipnum, ship_index;

	// If no arguments were supplied then turn off all messages then bail
	if (node < 0)
	{
		if (enable_messages) 
		{
			The_mission.flags &= ~MISSION_FLAG_NO_BUILTIN_MSGS;	
			return;
		}
		else 
		{
			The_mission.flags |= MISSION_FLAG_NO_BUILTIN_MSGS;
			return;
		}
	}

	// iterate through all the nodes supplied
	while (node >= 0)
	{		
		ship_name = CTEXT(node);

		// check that this isn't a request to silence command. 
		if ((*ship_name == '#') && !stricmp(&ship_name[1], The_mission.command_sender)) 
		{
			// Either disable or enable messages from command
			if (enable_messages) 
			{
				The_mission.flags &= ~MISSION_FLAG_NO_BUILTIN_COMMAND;	
			}
			else 
			{
				The_mission.flags |= MISSION_FLAG_NO_BUILTIN_COMMAND;
			}
		}
		else if (!stricmp(ship_name, "<Any Wingman>"))
		{
			// Since trying to determine whose wingman in a stand alone multiplayer game opens a can of worms
			// Any Wingman silences all ships in wings regardless of whose side they're on. 
			for (wingnum = 0; wingnum < Num_wings; wingnum++ ) {
				for ( shipnum = 0; shipnum < Wings[wingnum].current_count; shipnum++ ) {
					ship_index = Wings[wingnum].ship_index[shipnum];
					Assert( ship_index != -1 );

					if (enable_messages) {
						Ships[ship_index].flags2 &= ~SF2_NO_BUILTIN_MESSAGES;
					}
					else {
						Ships[ship_index].flags2 |= SF2_NO_BUILTIN_MESSAGES;
					}
				}
			}
		}
		// If it isn't command then assume that we're dealing with a ship 
		else 
		{
			sexp_deal_with_ship_flag(node, false, 0, 0, 0, SF2_NO_BUILTIN_MESSAGES, 0, P2_SF2_NO_BUILTIN_MESSAGES, !enable_messages);
		}

		node = CDR(node);
	}
}

void sexp_set_persona (int node) 
{
	int persona_index = -1, sindex, i; 
	char *persona_name; 

	persona_name = CTEXT(node);

	for (i = 0 ; i < Num_personas ; i++) {
		if (!strcmp(persona_name, Personas[i].name) && (Personas[i].flags & PERSONA_FLAG_WINGMAN)) {
			persona_index = i;
			break;
		}
	}

	if (persona_index == -1) {
		Warning(LOCATION, "Unable to change to persona type: '%s'. Persona is not a wingman!", persona_name);
		return; 
	}

	node = CDR(node); 
	Assert (node >=0);

	if (MULTIPLAYER_MASTER) {
		multi_start_callback();
		multi_send_int(persona_index); 
	}

	// now loop through the list of ships
	for ( ; node >= 0; node = CDR(node) ) {
		sindex = ship_name_lookup( CTEXT(node) );

		if (sindex < 0) {
			continue;
		}

		if (Ships[sindex].objnum < 0) {
			continue;
		}

		Ships[sindex].persona_index = persona_index; 

		if (MULTIPLAYER_MASTER) {
			multi_send_ship(sindex); 
		}
	}

	if (MULTIPLAYER_MASTER) {
		multi_end_callback();
	}
}

void multi_sexp_set_persona()
{
	ship *shipp = NULL; 
	int persona_index = -1; 

	if (!multi_get_int(persona_index) ) {
		return; 
	}

	while (multi_get_ship(shipp)) {
		Assert(persona_index != -1);
		if (shipp != NULL) {
			shipp->persona_index = persona_index;
		}
	}
}

int sexp_weapon_fired_delay(int node, int op_num)
{
	ship *shipp;
	
	int requested_bank; 
	int delay;
	int last_fired = -1; 

	shipp = sexp_get_ship_from_node(node); 
	if (shipp == NULL) {
		return SEXP_FALSE;
	}

	// Get the bank to check
	node = CDR(node);
	requested_bank = eval_num(node);
	if (requested_bank < 0) {
		return SEXP_FALSE;
	}

	// get the delay
	node = CDR(node);
	delay = eval_num(node);
	if (delay <= 0 ) {
		return SEXP_FALSE; 
	}

	switch (op_num) {
		case OP_PRIMARY_FIRED_SINCE: 
			if (requested_bank >= shipp->weapons.num_primary_banks) {
				return SEXP_FALSE;
			}
			last_fired = shipp->weapons.last_primary_fire_stamp[requested_bank];
			break; 

		case OP_SECONDARY_FIRED_SINCE: 
			if (requested_bank >= shipp->weapons.num_secondary_banks) {
				return SEXP_FALSE;
			}
			last_fired = shipp->weapons.last_secondary_fire_stamp[requested_bank];
			break; 
	}

	if (last_fired < 0) {
		// weapon was never fired
		return SEXP_FALSE;
	}

	if (timestamp() - delay < last_fired) {
		return SEXP_TRUE; 
	}

	return SEXP_FALSE;
}

int sexp_has_weapon(int node, int op_num)
{
	ship *shipp;
	int i;
	int requested_bank;
	int weapon_index;
	int num_weapon_banks = 0;
	int *weapon_banks = NULL;

	shipp = sexp_get_ship_from_node(node); 
	if (shipp == NULL) {
		return SEXP_FALSE;
	}

	// Get the bank to check
	node = CDR(node);
	if (!strcmp(CTEXT(node), SEXP_ALL_BANKS_STRING)) {
		requested_bank = -1;
	}
	else {
		requested_bank = eval_num(node);
	}	
	node = CDR(node);
 
	switch (op_num) {
		case OP_HAS_PRIMARY_WEAPON:
			weapon_banks = shipp->weapons.primary_bank_weapons;
			num_weapon_banks = shipp->weapons.num_primary_banks;
			break;

		case OP_HAS_SECONDARY_WEAPON:
			weapon_banks = shipp->weapons.secondary_bank_weapons;
			num_weapon_banks = shipp->weapons.num_secondary_banks;
			break;

		default:
			Warning(LOCATION, "Unrecognised bank type used in has-x-weapon. Returning false");
			return SEXP_FALSE;
	}
	

	//loop through the weapons and test them
	while (node > -1) {
		weapon_index = weapon_info_lookup(CTEXT(node));
		Assertion (weapon_index >= 0, "Weapon name %s is unknown.", CTEXT(node));

		// if we're checking every bank
		if (requested_bank == -1) {
			for (i = 0; i < num_weapon_banks; i++) {
				if (weapon_index == weapon_banks[i]) {
					return SEXP_TRUE;
				}
			}
		}

		// if we're only checking one bank
		else {
			if (weapon_index == weapon_banks[requested_bank]) {
				return SEXP_TRUE;
			}
		}
	
		node = CDR(node);
	}

	return SEXP_FALSE;
}

/**
 * Gets status of goals for previous missions (in the current campaign).
 *
 * @param n Sexp node number
 * @param status tell this function if we are looking for a goal_satisfied, goal_failed, or goal incomplete event
 */
int sexp_previous_goal_status( int n, int status )
{
	int rval = 0;
	char *goal_name, *mission_name;
	int i, mission_num, default_value = 0, use_defaults = 1;

	mission_name = CTEXT(n);
	goal_name = CTEXT(CDR(n));

	// check for possible next optional argument
	n = CDR(CDR(n));
	if ( n != -1 ) {
		default_value = is_sexp_true(n);
	}

	// try to find the given mission name in the current list of missions in the campaign.
	if ( Game_mode & GM_CAMPAIGN_MODE ) {
		i = mission_campaign_find_mission( mission_name );

		if ( i == -1 ) {
			// if mission not found, assume that goal was false (so previous-goal-false returns true)
			nprintf(("General", "Couldn't find mission name %s in current campaign's list of missions.\nReturning %s for goal-status function.", mission_name, (status==GOAL_COMPLETE)?"false":"true"));
			if ( status == GOAL_COMPLETE )
				rval = SEXP_KNOWN_FALSE;
			else
				rval = SEXP_KNOWN_TRUE;

			use_defaults = 0;
		} else if (Campaign.missions[i].flags & CMISSION_FLAG_SKIPPED) {
			use_defaults = 1;
		} else {
			// now try and find the goal this mission
			mission_num = i;
			for (i = 0; i < Campaign.missions[mission_num].num_goals; i++) {
				if ( !stricmp(Campaign.missions[mission_num].goals[i].name, goal_name) )
					break;
			}

			if ( i == Campaign.missions[mission_num].num_goals ) {
				Warning(LOCATION, "Couldn't find goal name %s in mission %s.\nReturning %s for goal-true function.", goal_name, mission_name, (status==GOAL_COMPLETE)?"false":"true");
				if ( status == GOAL_COMPLETE )
					rval = SEXP_KNOWN_FALSE;
				else
					rval = SEXP_KNOWN_TRUE;

			} else {
				// now return KNOWN_TRUE or KNOWN_FALSE based on the status field in the goal structure
				if ( Campaign.missions[mission_num].goals[i].status == status )
					rval = SEXP_KNOWN_TRUE;
				else
					rval = SEXP_KNOWN_FALSE;
			}

			use_defaults = 0;
		}
	}

	if (use_defaults) {
		// when not in campaign mode, always return KNOWN_TRUE when looking for goal complete, and KNOWN_FALSE
		// otherwise
		if ( n != -1 ) {
			if ( default_value )
				rval = SEXP_KNOWN_TRUE;
			else
				rval = SEXP_KNOWN_FALSE;
		} else {
			if ( status == GOAL_COMPLETE )
				rval = SEXP_KNOWN_TRUE;
			else
				rval = SEXP_KNOWN_FALSE;
		}
	}

	return rval;
}

// sexpression which gets the status of an event from a previous mission.  Like the above function but
// dealing with events instead of goals.  Again, the status parameter tells the code if we are looking
// for an event_true, event_false, or event_incomplete status
int sexp_previous_event_status( int n, int status )
{
	int rval = 0;
	char *name, *mission_name;
	int i, mission_num, default_value = 0, use_defaults = 1;

	mission_name = CTEXT(n);
	name = CTEXT(CDR(n));

	// check for possible optional parameter
	n = CDR(CDR(n));
	if ( n != -1 ){
		default_value = is_sexp_true(n);
	}

	if ( Game_mode & GM_CAMPAIGN_MODE ) {
		// following function returns -1 when mission isn't found.
		i = mission_campaign_find_mission( mission_name );

		// if the mission name wasn't found -- make this return FALSE for the event status.
		if ( i == -1 ) {
			nprintf(("General", "Couldn't find mission name %s in current campaign's list of missions.\nReturning %s for event-status function.", mission_name, (status==EVENT_SATISFIED)?"false":"true"));
			if ( status == EVENT_SATISFIED ) {
				rval = SEXP_KNOWN_FALSE;
			} else {
				rval = SEXP_KNOWN_TRUE;
			}

			use_defaults = 0;
		} else if (Campaign.missions[i].flags & CMISSION_FLAG_SKIPPED) {
			use_defaults = 1;
		} else {
			// now try and find the goal this mission
			mission_num = i;
			for (i = 0; i < Campaign.missions[mission_num].num_events; i++) {
				if ( !stricmp(Campaign.missions[mission_num].events[i].name, name) )
					break;
			}

			if ( i == Campaign.missions[mission_num].num_events ) {
				Warning(LOCATION, "Couldn't find event name %s in mission %s.\nReturning %s for event_status function.", name, mission_name, (status==EVENT_SATISFIED)?"false":"true");
				if ( status == EVENT_SATISFIED )
					rval = SEXP_KNOWN_FALSE;
				else
					rval = SEXP_KNOWN_TRUE;

			} else {
				// now return KNOWN_TRUE or KNOWN_FALSE based on the status field in the goal structure
				if ( Campaign.missions[mission_num].events[i].status == status )
					rval = SEXP_KNOWN_TRUE;
				else
					rval = SEXP_KNOWN_FALSE;
			}

			use_defaults = 0;
		}
	} 

	if (use_defaults) {
		if ( n != -1 ) {
			if ( default_value )
				rval = SEXP_KNOWN_TRUE;
			else
				rval = SEXP_KNOWN_FALSE;
		} else {
			if ( status == EVENT_SATISFIED )
				rval = SEXP_KNOWN_TRUE;
			else
				rval = SEXP_KNOWN_FALSE;
		}
	}

	return rval;
}

/**
 * Return the status of an event in the current mission.  
 *
 * @param n Sexp node number
 * @param want_true indicates if we are checking whether the event is true or the event is false.
 */
int sexp_event_status( int n, int want_true )
{
	char *name;
	int i, result;

	name = CTEXT(n);

	if (name == NULL) {
		Int3();
		return SEXP_FALSE;
	}

	for (i = 0; i < Num_mission_events; i++ ) {
		// look for the event name, check it's status.  If formula is gone, we know the state won't ever change.
		if ( !stricmp(Mission_events[i].name, name) ) {
			result = Mission_events[i].result;
			if (Mission_events[i].formula < 0) {
				if ( (want_true && result) || (!want_true && !result) )
					return SEXP_KNOWN_TRUE;
				else
					return SEXP_KNOWN_FALSE;

			} else {
				if ( (want_true && result) || (!want_true && !result) )
					return SEXP_TRUE;
				else
					return SEXP_FALSE;
			}
		}
	}

	return SEXP_FALSE;
}

/**
 * Return the status of an event N seconds after the event is true or false.
 *
 * Similar to above function but waits N seconds before returning true
 */
int sexp_event_delay_status( int n, int want_true, bool use_msecs = false)
{
	char *name;
	int i, result;
	fix delay;
	int rval = SEXP_FALSE;
	int use_as_directive = 0;

	name = CTEXT(n);

	if (name == NULL) {
		Int3();
		return SEXP_FALSE;
	}

	uint64_t tempDelay = eval_num(CDR(n));

	if (use_msecs) {
		tempDelay = tempDelay << 16;
		tempDelay = tempDelay / 1000;

		delay = (fix) tempDelay;
	} else {
		delay = i2f(tempDelay);
	}

	for (i = 0; i < Num_mission_events; i++ ) {
		// look for the event name, check it's status.  If formula is gone, we know the state won't ever change.
		if ( !stricmp(Mission_events[i].name, name) ) {
			if ( (fix) Mission_events[i].timestamp + delay >= Missiontime ) {
				rval = SEXP_FALSE;
				break;
			}

			result = Mission_events[i].result;
			if (Mission_events[i].formula < 0) {
				if ( (want_true && result) || (!want_true && !result) ) {
					rval = SEXP_KNOWN_TRUE;
					break;
				} else {
					rval = SEXP_KNOWN_FALSE;
					break;
				}
			} else {
				if ( want_true && result ) {  //) || (!want_true && !result) )
					rval = SEXP_TRUE;
					break;
				} else {
					rval = SEXP_FALSE;
					break;
				}
			}
		}
	}

	// check for possible optional parameter
	n = CDDR(n);
	if (n != -1)
		use_as_directive = is_sexp_true(n);

	// zero out Sexp_useful_number if it's not true and we don't want this for specific directive use
	if ( !use_as_directive && (rval != SEXP_TRUE) && (rval != SEXP_KNOWN_TRUE) )
		Sexp_useful_number = 0;  // indicate sexp isn't current yet

	return rval;
}

/**
 * Returns true if the given event is still incomplete
 */
int sexp_event_incomplete(int n)
{
	char *name;
	int i;

	name = CTEXT(n);

	if (name == NULL) {
		Int3();
		return SEXP_FALSE;
	}
	
	for (i = 0; i < Num_mission_events; i++ ) {
		if ( !stricmp(Mission_events[i].name, name ) ) {
			// if the formula is still >= 0 (meaning it is still getting eval'ed), then
			// the event is incomplete
			if ( Mission_events[i].formula != -1 )
				return SEXP_TRUE;
			else
				return SEXP_KNOWN_FALSE;
		}
	}

	return SEXP_FALSE;
}

/**
 * Return the status of an goal N seconds after the goal is true or false.
 *
 * Similar to above function but operates on goals instead of events
 */
int sexp_goal_delay_status( int n, int want_true )
{
	char *name;
	fix delay, time;

	name = CTEXT(n);
	delay = i2f(eval_num(CDR(n)));
	
	if ( want_true ) {
		// if we are looking for a goal true entry and we find a false, then return known false here
		if ( mission_log_get_time(LOG_GOAL_FAILED, name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_GOAL_SATISFIED, name, NULL, &time) ) {
			if ( (Missiontime - time) >= delay )
				return SEXP_KNOWN_TRUE;
		}
	} else {
		// if we are looking for a goal false entry and we find a true, then return known false here
		if ( mission_log_get_time(LOG_GOAL_SATISFIED, name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_GOAL_FAILED, name, NULL, &time) ) {
			if ( (Missiontime - time) >= delay )
				return SEXP_KNOWN_TRUE;
		}
	}

	return SEXP_FALSE;
}

/**
 * Returns true if the given goal is still incomplete
 */
int sexp_goal_incomplete(int n)
{
	char *name;

	name = CTEXT(n);

	if ( mission_log_get_time( LOG_GOAL_SATISFIED, name, NULL, NULL) || mission_log_get_time( LOG_GOAL_FAILED, name, NULL, NULL) )
		return SEXP_KNOWN_FALSE;
	else
		return SEXP_TRUE;
}

/**
 * Protects/unprotects a ship.
 *
 * @param n Sexp node number
 * @param flag Whether or not the protect bit should be set (flag==true) or cleared (flag==false)
 */
void sexp_protect_ships(int n, bool flag)
{
	sexp_deal_with_ship_flag(n, true, OF_PROTECTED, 0, 0, 0, P_OF_PROTECTED, 0, flag);
}

/**
 * Protects/unprotects a ship from beams.
 *
 * @param n Sexp node number
 * @param flag Whether or not the protect bit should be set (flag==true) or cleared (flag==false)
 */
void sexp_beam_protect_ships(int n, bool flag)
{
	sexp_deal_with_ship_flag(n, true, OF_BEAM_PROTECTED, 0, 0, 0, P_OF_BEAM_PROTECTED, 0, flag);
}

/**
 * Protects/unprotects a ship from various turrets.
 *
 * @param n Sexp node number
 * @param flag Whether or not the protect bit should be set (flag==true) or cleared (flag==false)
 */
void sexp_turret_protect_ships(int n, bool flag)
{
	char *turret_type = CTEXT(n);
	n = CDR(n);

	if (!stricmp(turret_type, "beam"))
		sexp_deal_with_ship_flag(n, true, OF_BEAM_PROTECTED, 0, 0, 0, P_OF_BEAM_PROTECTED, 0, flag);
	else if (!stricmp(turret_type, "flak"))
		sexp_deal_with_ship_flag(n, true, OF_FLAK_PROTECTED, 0, 0, 0, P_OF_FLAK_PROTECTED, 0, flag);
	else if (!stricmp(turret_type, "laser"))
		sexp_deal_with_ship_flag(n, true, OF_LASER_PROTECTED, 0, 0, 0, P_OF_LASER_PROTECTED, 0, flag);
	else if (!stricmp(turret_type, "missile"))
		sexp_deal_with_ship_flag(n, true, OF_MISSILE_PROTECTED, 0, 0, 0, P_OF_MISSILE_PROTECTED, 0, flag);
	else
		Warning(LOCATION, "Invalid turret type '%s'!", turret_type);
}

// Goober5000 - sets the "dont collide invisible" flag on a list of ships
void sexp_dont_collide_invisible(int n, bool dont_collide)
{
	sexp_deal_with_ship_flag(n, true, 0, 0, 0, SF2_DONT_COLLIDE_INVIS, 0, P_SF2_DONT_COLLIDE_INVIS, dont_collide);
}

// Goober5000 - sets the "immobile" flag on a list of ships
void sexp_set_immobile(int n, bool immobile)
{
	sexp_deal_with_ship_flag(n, true, OF_IMMOBILE, 0, 0, 0, 0, P2_OF_IMMOBILE, immobile);
}

// Goober5000 - sets the "no-ets" flag on a list of ships
void sexp_disable_ets(int n, bool disable)
{
	sexp_deal_with_ship_flag(n, true, 0, 0, 0, SF2_NO_ETS, 0, P2_SF2_NO_ETS, disable);
}

// Goober5000 - sets the vaporize flag on a list of ships
void sexp_ships_vaporize(int n, bool vaporize)
{
	sexp_deal_with_ship_flag(n, true, 0, 0, SF_VAPORIZE, 0, P_SF_VAPORIZE, 0, vaporize);
}

/**
 * Make ships "visible" and "invisible" to sensors.
 *
 * @param n Sexp node number
 * @param visible Is true when making ships visible, false otherwise
 */
void sexp_ships_visible(int n, bool visible)
{
	sexp_deal_with_ship_flag(n, true, 0, 0, SF_HIDDEN_FROM_SENSORS, 0, P_SF_HIDDEN_FROM_SENSORS, 0, !visible, true);

	// we also have to add any escort ships that were made visible
	for (; n >= 0; n = CDR(n))
	{
		int shipnum = ship_name_lookup(CTEXT(n));
		if (shipnum < 0)
			continue;

		if (!visible && Player_ai->target_objnum == Ships[shipnum].objnum) {
			hud_cease_targeting(); 
		}

		else if (visible && (Ships[shipnum].flags & SF_ESCORT)) {
				hud_add_ship_to_escort(Ships[shipnum].objnum, 1);
		}		
	}
}

// Goober5000
void sexp_ships_stealthy(int n, bool stealthy)
{
	sexp_deal_with_ship_flag(n, true, 0, 0, 0, SF2_STEALTH, 0, P_SF2_STEALTH, stealthy, true);

	// we also have to add any escort ships that were made visible
	if (!stealthy)
	{
		for (; n >= 0; n = CDR(n))
		{
			int shipnum = ship_name_lookup(CTEXT(n));
			if (shipnum < 0)
				continue;

			if (Ships[shipnum].flags & SF_ESCORT)
				hud_add_ship_to_escort(Ships[shipnum].objnum, 1);
		}
	}
}

// Goober5000
void sexp_friendly_stealth_invisible(int n, bool invisible)
{
	sexp_deal_with_ship_flag(n, true, 0, 0, 0, SF2_FRIENDLY_STEALTH_INVIS, 0, P_SF2_FRIENDLY_STEALTH_INVIS, invisible, true);

	// we also have to add any escort ships that were made visible
	if (!invisible)
	{
		for (; n >= 0; n = CDR(n))
		{
			int shipnum = ship_name_lookup(CTEXT(n));
			if (shipnum < 0)
				continue;

			if (Ships[shipnum].flags & SF_ESCORT)
				hud_add_ship_to_escort(Ships[shipnum].objnum, 1);
		}
	}
}

//FUBAR
//generic function to deal with subsystem flag sexps.
//setit only passed for backward compatibility with older sexps.
void sexp_ship_deal_with_subsystem_flag(int node, int ss_flag, bool sendit = false, bool setit = false)
{	
	ship *shipp = NULL;
	ship_subsys *ss = NULL;	
	
	// get ship
	shipp = sexp_get_ship_from_node(node); 
	if (shipp == NULL) {
		return;
	}

	//replace or not
	// OP_SHIP_SUBSYS_TARGETABLE/UNTARGETABLE, OP_SHIP_SUBSYS_TARGETABLE and OP_TURRET_SUBSYS_TARGET_ENABLE/DISABLE 
	// will have already passed us this data we don't need to set it for them. 
	// backward compatibility hack for older sexps
	if (!((ss_flag == SSF_UNTARGETABLE) || (ss_flag == SSF_NO_SS_TARGETING)))
	{
		node = CDR(node);
		setit = (is_sexp_true(node) ? true : false);
	}
	
	//multiplayer packet start
	if (sendit)
	{
		multi_start_callback(); 
		multi_send_ship(shipp);
		multi_send_bool(setit);
	}

	//Process subsystems
	while(node != -1)
	{
		// deal with generic subsystem names
		int generic_type = get_generic_subsys(CTEXT(node));
		if (generic_type) {
			for (ss = GET_FIRST(&shipp->subsys_list); ss != END_OF_LIST(&shipp->subsys_list); ss = GET_NEXT(ss)) {
				if (generic_type == ss->system_info->type) {
					if (setit)
						ss->flags |= ss_flag;
					else
						ss->flags &= ~ss_flag;
				}
			}
		}
		else
		{
			// get the subsystem
			ss = ship_get_subsys(shipp, CTEXT(node));
			if(ss == NULL)
			{
				node = CDR(node);
				continue;
			}
 
			// set the flag
			if(setit)
				ss->flags |= ss_flag;
			else
				ss->flags &= ~ss_flag;
		}

		// multiplayer send subsystem name
		if (sendit)
			multi_send_string(CTEXT(node));

		// next
		node = CDR(node);
	}

	// mulitplayer end of packet
	if (sendit)
		multi_end_callback();
}
void multi_sexp_deal_with_subsys_flag(int ss_flag)
{
	bool setit = false;
	ship_subsys *ss = NULL;
    ship *shipp = NULL;
	char ss_name[MAX_NAME_LEN];

	multi_get_ship(shipp);
	multi_get_bool(setit);
 
	while (multi_get_string(ss_name)) 
	{
		// deal with generic subsystem names
		int generic_type = get_generic_subsys(ss_name);
		if (generic_type) {
			for (ss = GET_FIRST(&shipp->subsys_list); ss != END_OF_LIST(&shipp->subsys_list); ss = GET_NEXT(ss)) {
				if (generic_type == ss->system_info->type) {
					if (setit)
						ss->flags |= ss_flag;
					else
						ss->flags &= ~ss_flag;
				}
			}
		}
		else
		{
			ss = ship_get_subsys(shipp, ss_name);
			if(ss != NULL)
			{	
				// set the flag
				if(setit)
					ss->flags |= ss_flag;
				else
					ss->flags &= ~ss_flag;
			}
		}
	}
}
// Goober5000
void sexp_ship_tag( int n, int tag )
{
	int ship_num, tag_level, tag_time, ssm_index(0);
    int ssm_team = 0;

	// check to see if ship destroyed or departed.  In either case, do nothing.
	if ( mission_log_get_time(LOG_SHIP_DEPARTED, CTEXT(n), NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, CTEXT(n), NULL, NULL) )
		return;

	// get the ship num
	ship_num = ship_name_lookup(CTEXT(n));
	if ( ship_num < 0 )
		return;

	// if untag, then unset everything and leave
	if (!tag)
	{
		Ships[ship_num].tag_left = -1.0f;
		Ships[ship_num].level2_tag_left = -1.0f;
		return;
	}

	// get the tag level and time
	n = CDR(n);
	tag_level = eval_num(n);
	n = CDR(n);
	tag_time = eval_num(n);

	// get SSM info if needed
	vec3d start;
	if (tag_level == 3)
	{
		n = CDR(n);
		if (n < 0)
			return;
		ssm_index = ssm_info_lookup(CTEXT(n));
		if (ssm_index < 0)
			return;

		n = CDR(n);
		if (n < 0)
			return;
		start.xyz.x = (float)eval_num(n);

		n = CDR(n);
		if (n < 0)
			return;
		start.xyz.y = (float)eval_num(n);

		n = CDR(n);
		if (n < 0)
			return;
		start.xyz.z = (float)eval_num(n);

        n = CDR(n);

        if (n < 0)
        {
            ssm_team = 0;
        }
        else
        {
            ssm_team = iff_lookup(CTEXT(n));
        }
	}

	ship_apply_tag(ship_num, tag_level, (float)tag_time, &Objects[Ships[ship_num].objnum], &start, ssm_index, ssm_team);
}

// sexpression to toggle invulnerability flag of ships.
void sexp_ships_invulnerable( int n, bool invulnerable )
{
	sexp_deal_with_ship_flag(n, true, OF_INVULNERABLE, 0, 0, 0, P_OF_INVULNERABLE, 0, invulnerable);
}

void sexp_ships_bomb_targetable(int n, bool targetable)
{
	sexp_deal_with_ship_flag(n, true, OF_TARGETABLE_AS_BOMB, 0, 0, 0, 0, P2_OF_TARGETABLE_AS_BOMB, targetable, true);
}

// Goober5000
void sexp_ship_guardian_threshold(int node)
{
	char *ship_name;
	int ship_num, threshold;
	int n = -1;

	threshold = eval_num(node);

	n = CDR(node);

	// for all ships
	for ( ; n != -1; n = CDR(n) ) {
		// check to see if ship destroyed or departed.  In either case, do nothing.
		ship_name = CTEXT(n);

		if ( mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) ) {
			continue;
		}

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.
		ship_num = ship_name_lookup(ship_name);

		if (ship_num != -1) {
			Ships[ship_num].ship_guardian_threshold = threshold;
		}
	}
}

// Goober5000
void sexp_ship_subsys_guardian_threshold(int num)
{
	char *ship_name, *hull_name;
	int ship_num, threshold;
	ship_subsys *ss;
	int n = -1;

	threshold = eval_num(num);

	n = CDR(num);

	// check to see if ship destroyed or departed.  In either case, do nothing.
	ship_name = CTEXT(n);

	if ( mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) ) {
		return;
	}

	// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.
	ship_num = ship_name_lookup(ship_name);

	if (ship_num == -1) {
		return;
	}

	n = CDR(n);

	// for all subsystems
	for ( ; n != -1; n = CDR(n) ) {
		// check for HULL
		hull_name = CTEXT(n);

		if (hull_name != NULL) {
			int generic_type = get_generic_subsys(hull_name);
			if ( !strcmp(hull_name, SEXP_HULL_STRING) ) {
				Ships[ship_num].ship_guardian_threshold = threshold;
			}
			else if (generic_type) {
				// search through all subsystems
				for (ss = GET_FIRST(&Ships[ship_num].subsys_list); ss != END_OF_LIST(&Ships[ship_num].subsys_list); ss = GET_NEXT(ss)) {
					if (generic_type == ss->system_info->type) {
						ss->subsys_guardian_threshold = threshold;
					}
				}
			}				
			else {
				ss = ship_get_subsys(&Ships[ship_num], hull_name);
				if ( ss == NULL ) {
					if (ship_class_unchanged(ship_num)) {
						Warning(LOCATION, "Invalid subsystem passed to ship-subsys-guardian-threshold: %s does not have a %s subsystem", ship_name, hull_name);
					}
				} 
				else {
					ss->subsys_guardian_threshold = threshold;
				}
			}
		}
	}
}

// sexpression to toggle KEEP ALIVE flag of ship object
void sexp_ships_guardian( int n, int guardian )
{
	char *ship_name;
	int num;

	for ( ; n != -1; n = CDR(n) )
	{
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num != -1 ) {
			Ships[num].ship_guardian_threshold = guardian ? SHIP_GUARDIAN_THRESHOLD_DEFAULT : 0;
		} else {
			p_object *p_objp = mission_parse_get_arrival_ship(ship_name);
			if (p_objp)
			{
				if ( guardian )
					p_objp->flags |= P_SF_GUARDIAN;
				else
					p_objp->flags &= ~P_SF_GUARDIAN;
			}
		}
	}
}

void sexp_ship_create(int n)
{
	int new_ship_class = -1;
	char *new_ship_name;
	vec3d new_ship_pos = vmd_zero_vector;
	angles new_ship_ang = {0.0f, 0.0f, 0.0f};
	matrix new_ship_ori = vmd_identity_matrix;
	bool change_angles = false;

	Assert( n >= 0 );

	// get ship name - none means don't specify it
	// if ship with this name already exists, ship_create will respond appropriately
	if (!stricmp(CTEXT(n), SEXP_NONE_STRING))
	{
		new_ship_name = NULL;
	}
	else
	{
		new_ship_name = CTEXT(n);
	}
	
	//Get ship class
	n = CDR(n);
	new_ship_class = ship_info_lookup(CTEXT(n));

	if(new_ship_class == -1) {
		Warning(LOCATION, "Invalid ship class passed to ship-create; ship type '%s' does not exist", CTEXT(n));
		return;
	}

	n = CDR(n);
	new_ship_pos.xyz.x = (float) eval_num(n);
	n = CDR(n);
	new_ship_pos.xyz.y = (float) eval_num(n);
	n = CDR(n);
	new_ship_pos.xyz.z = (float) eval_num(n);

	n = CDR(n);
	if(n != -1) {
		new_ship_ang.p = fl_radians(eval_num(n) % 360);
		change_angles = true;
	}

	n = CDR(n);
	if(n != -1) {
		new_ship_ang.b = fl_radians(eval_num(n) % 360);
		change_angles = true;
	}

	n = CDR(n);
	if(n != -1) {
		new_ship_ang.h = fl_radians(eval_num(n) % 360);
		change_angles = true;
	}

	//This is a costly function, so only do it if needed
	if(change_angles) {
		vm_angles_2_matrix(&new_ship_ori, &new_ship_ang);
	}

	ship_create(&new_ship_ori, &new_ship_pos, new_ship_class, new_ship_name);
}

// Goober5000
void sexp_weapon_create(int n)
{
	int weapon_class, parent_objnum, target_objnum, weapon_objnum;
	ship_subsys *targeted_ss;

	vec3d weapon_pos = vmd_zero_vector;
	angles weapon_angles = {0.0f, 0.0f, 0.0f};
	matrix weapon_orient = vmd_identity_matrix;
	int is_locked;
	bool change_angles = false;

	Assert( n >= 0 );

	parent_objnum = -1;
	if (stricmp(CTEXT(n), SEXP_NONE_STRING))
	{
		int parent_ship = ship_name_lookup(CTEXT(n));

		if (parent_ship >= 0)
			parent_objnum = Ships[parent_ship].objnum;
	}
	n = CDR(n);
	
	weapon_class = weapon_info_lookup(CTEXT(n));
	n = CDR(n);
	if (weapon_class < 0)
	{
		Warning(LOCATION, "Invalid weapon class passed to weapon-create; weapon type '%s' does not exist", CTEXT(n));
		return;
	}

	weapon_pos.xyz.x = (float) eval_num(n);
	n = CDR(n);
	weapon_pos.xyz.y = (float) eval_num(n);
	n = CDR(n);
	weapon_pos.xyz.z = (float) eval_num(n);
	n = CDR(n);

	if (n >= 0)
	{
		weapon_angles.p = fl_radians(eval_num(n) % 360);
		n = CDR(n);
		change_angles = true;
	}

	if (n >= 0)
	{
		weapon_angles.b = fl_radians(eval_num(n) % 360);
		n = CDR(n);
		change_angles = true;
	}

	if (n >= 0)
	{
		weapon_angles.h = fl_radians(eval_num(n) % 360);
		n = CDR(n);
		change_angles = true;
	}

	// This is a costly function, so only do it if needed
	if (change_angles)
	{
		vm_angles_2_matrix(&weapon_orient, &weapon_angles);
	}

	target_objnum = -1;
	if (n >= 0)
	{
		int target_ship = ship_name_lookup(CTEXT(n));

		if (target_ship >= 0)
			target_objnum = Ships[target_ship].objnum;

		n = CDR(n);
	}

	targeted_ss = NULL;
	if (n >= 0)
	{
		if (target_objnum >= 0)
			targeted_ss = ship_get_subsys(&Ships[Objects[target_objnum].instance], CTEXT(n));

		n = CDR(n);
	}

	is_locked = (target_objnum >= 0) ? 1 : 0;	// assume full lock; this lets lasers track if people want them to

	// create the weapon
	weapon_objnum = weapon_create(&weapon_pos, &weapon_orient, weapon_class, parent_objnum, -1, is_locked);

	// maybe make the weapon track its target
	if (is_locked)
		weapon_set_tracking_info(weapon_objnum, parent_objnum, target_objnum, is_locked, targeted_ss);
}

// make ship vanish without a trace (and what its docked to)
void sexp_ship_vanish(int n)
{
	char *ship_name;
	int num;

	// if MULTIPLAYER bail
	if (Game_mode & GM_MULTIPLAYER) {
		return;
	}

	for ( ; n != -1; n = CDR(n) ) {
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive
		num = ship_name_lookup(ship_name);
		if ( num != -1 )
			ship_actually_depart(num, SHIP_VANISHED);
	}
}

void sexp_shields_off(int n, bool shields_off ) //-Sesquipedalian
{
	sexp_deal_with_ship_flag(n, true, OF_NO_SHIELDS, 0, 0, 0, P_OF_NO_SHIELDS, 0, shields_off);
}

// Goober5000
void sexp_ingame_ship_kamikaze(ship *shipp, int kdamage)
{
	Assert(shipp);

	ai_info *aip = &Ai_info[shipp->ai_index];

	if (kdamage > 0)
	{
		aip->ai_flags |= AIF_KAMIKAZE;
		aip->kamikaze_damage = kdamage;
	}
	else
	{
		aip->ai_flags &= ~AIF_KAMIKAZE;
		aip->kamikaze_damage = 0;
	}
}

// Goober5000
void sexp_parse_ship_kamikaze(p_object *parse_obj, int kdamage)
{
	Assert(parse_obj);

	if (kdamage > 0)
	{
		parse_obj->flags |= P_AIF_KAMIKAZE;
		parse_obj->kamikaze_damage = kdamage;
	}
	else
	{
		parse_obj->flags &= ~P_AIF_KAMIKAZE;
		parse_obj->kamikaze_damage = 0;
	}
}

// Goober5000 - redone, added wing stuff
void sexp_kamikaze(int n, int kamikaze)
{
	int kdamage;

	kdamage = 0;
	if (kamikaze)
	{
		kdamage = eval_num(n);
		n = CDR(n);
	}

	for ( ; n != -1; n = CDR(n) )
	{
		object_ship_wing_point_team oswpt;
		sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));

		switch (oswpt.type)
		{
			// change ingame ship
			case OSWPT_TYPE_SHIP:
			{
				sexp_ingame_ship_kamikaze(oswpt.shipp, kdamage);

				break;
			}

			// change ship yet to arrive
			case OSWPT_TYPE_PARSE_OBJECT:
			{
				sexp_parse_ship_kamikaze(oswpt.p_objp, kdamage);

				break;
			}

			// change wing (we must set the flags for all ships present as well as all ships yet to arrive)
			case OSWPT_TYPE_WING:
			case OSWPT_TYPE_WING_NOT_PRESENT:
			{
				// current ships
				for (int i = 0; i < oswpt.wingp->current_count; i++)
					sexp_ingame_ship_kamikaze(&Ships[oswpt.wingp->ship_index[i]], kdamage);

				// ships yet to arrive
				for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
				{
					if (p_objp->wingnum == WING_INDEX(oswpt.wingp))
						sexp_parse_ship_kamikaze(p_objp, kdamage);
				}
			}
		}
	}
}

// Goober5000
void sexp_ingame_ship_alt_name(ship *shipp, int alt_index)
{
	Assert((shipp != NULL) && (alt_index < Mission_alt_type_count));

	// we might be clearing it
	if (alt_index < 0)
	{
		shipp->alt_type_index = -1;
		return;
	}

	// see if this is actually the ship class
	if (!stricmp(Ship_info[shipp->ship_info_index].name, Mission_alt_types[alt_index]))
	{
		shipp->alt_type_index = -1;
		return;
	}

	shipp->alt_type_index = alt_index;
}

// Goober5000
void sexp_parse_ship_alt_name(p_object *parse_obj, int alt_index)
{
	Assert((parse_obj != NULL) && (alt_index < Mission_alt_type_count));

	// we might be clearing it
	if (alt_index < 0)
	{
		parse_obj->alt_type_index = -1;
		return;
	}

	// see if this is actually the ship class
	if (!stricmp(Ship_class_names[parse_obj->ship_class], Mission_alt_types[alt_index]))
	{
		parse_obj->alt_type_index = -1;
		return;
	}

	parse_obj->alt_type_index = alt_index;
}

// Goober5000
void sexp_ship_change_alt_name(int node)
{
	int n = node, new_alt_index;
	char *new_alt_name;

	// get the alt-name
	new_alt_name = CTEXT(n);
	n = CDR(n);

	// and its index
	if (!*new_alt_name || !stricmp(new_alt_name, SEXP_ANY_STRING))
	{
		new_alt_index = -1;
	}
	else
	{
		new_alt_index = mission_parse_lookup_alt(new_alt_name);
		if (new_alt_index < 0)
			new_alt_index = mission_parse_add_alt(new_alt_name);
	}

	for ( ; n != -1; n = CDR(n) )
	{
		object_ship_wing_point_team oswpt;
		sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));

		switch (oswpt.type)
		{
			// change ingame ship
			case OSWPT_TYPE_SHIP:
			{
				sexp_ingame_ship_alt_name(oswpt.shipp, new_alt_index);
				break;
			}

			// change ship yet to arrive
			case OSWPT_TYPE_PARSE_OBJECT:
			{
				sexp_parse_ship_alt_name(oswpt.p_objp, new_alt_index);
				break;
			}

			// change wing (we must set the flags for all ships present as well as all ships yet to arrive)
			case OSWPT_TYPE_WING:
			case OSWPT_TYPE_WING_NOT_PRESENT:
			{		
				// current ships
				for (int i = 0; i < oswpt.wingp->current_count; i++)
					sexp_ingame_ship_alt_name(&Ships[oswpt.wingp->ship_index[i]], new_alt_index);
	
				// ships yet to arrive
				for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
				{
					if (p_objp->wingnum == WING_INDEX(oswpt.wingp))
						sexp_parse_ship_alt_name(p_objp, new_alt_index);
				}
				break;
			}
		}
	}
}

// FUBAR
void sexp_ship_change_callsign(int node)
{
	char *new_callsign;
	int sindex, cindex;
	ship *shipp = NULL;

	// get the callsign
	new_callsign = CTEXT(node);
	node = CDR(node);

	// and its index
	if (!*new_callsign || !stricmp(new_callsign, SEXP_ANY_STRING))
	{
		cindex = -1;
	}
	else
	{
		cindex = mission_parse_lookup_callsign(new_callsign);
		if (cindex < 0)
			cindex = mission_parse_add_callsign(new_callsign);
	}

	// packets for multi
	multi_start_callback();
	multi_send_string(new_callsign); 

	while ( node >= 0 )
	{
		sindex = ship_name_lookup(CTEXT(node));
		if (sindex >= 0) 
		{
			shipp = &Ships[sindex];
			shipp->callsign_index = cindex;
			multi_send_ship(shipp);
		}
		node = CDR(node);
	}

	multi_end_callback();
}

void multi_sexp_ship_change_callsign()
{
	char new_callsign[TOKEN_LENGTH];
	int cindex;
	ship *shipp = NULL;

	multi_get_string(new_callsign);
	if (!new_callsign[0] || !stricmp(new_callsign, SEXP_ANY_STRING))
	{
		cindex = -1;
	}
	else
	{
		cindex = mission_parse_lookup_callsign(new_callsign);
		if (cindex < 0) 
		{
			cindex = mission_parse_add_callsign(new_callsign);
		}
	}

	while (multi_get_ship(shipp)) 
	{
		if (shipp != NULL) 
		{
			shipp->callsign_index = cindex;
		}
	}
}

// Goober5000
void sexp_set_death_message(int n)
{
	int i;

	// we'll suppose it's the string for now
	Player->death_message = CTEXT(n);

	// but use an actual message if one exists
	for (i=0; i<Num_messages; i++)
	{
		if (!stricmp(Messages[i].name, CTEXT(n)))
		{
			Player->death_message = Messages[i].message;
			break;
		}
	}

	// apply localization
	extern void lcl_replace_stuff(SCP_string &text);
	lcl_replace_stuff(Player->death_message);

	sexp_replace_variable_names_with_values(Player->death_message);
}

int sexp_key_pressed(int node)
{
	int z, t;

	Assert(node != -1);
	z = translate_key_to_index(CTEXT(node));
	if (z < 0){
		return SEXP_FALSE;
	}

	if (!Control_config[z].used){
		return SEXP_FALSE;
	}

	if (CDR(node) < 0){
		return SEXP_TRUE;
	}

	t = eval_num(CDR(node));
	return timestamp_has_time_elapsed(Control_config[z].used, t * 1000);
}

void sexp_key_reset(int node)
{
	int n, z;

	for (n = node; n != -1; n = CDR(n))
	{
		z = translate_key_to_index(CTEXT(n));
		if (z >= 0)
			Control_config[z].used = 0;
	}
}
				
void sexp_ignore_key(int node)
{
	int ignore_count;
	int ignored_key;


	ignore_count = eval_num(node);

	multi_start_callback();
	multi_send_int(ignore_count);

	node = CDR(node);
	while (node > -1) {
		// get the key
		ignored_key = translate_key_to_index(CTEXT(node));

		if (ignored_key > -1) {
			Ignored_keys[ignored_key] = ignore_count;
		}

		multi_send_int(ignored_key);

		node = CDR(node);
	}

	multi_end_callback();
}

void multi_sexp_ignore_key()
{
	int ignored_key, ignore_count;

	multi_get_int(ignore_count);
	
	while (multi_get_int(ignored_key)) {
		Ignored_keys[ignored_key] = ignore_count;
	}
}

int sexp_targeted(int node)
{
	int z;
	ship_subsys *ptr;

	z = ship_query_state(CTEXT(node));
	if (z == 1){
		return SEXP_KNOWN_FALSE;  // ship isn't around, nor will it ever be
	} else if (z == -1) {
		return SEXP_CANT_EVAL;
	}

	z = ship_name_lookup(CTEXT(node), 1);
	if ((z < 0) || !Player_ai || (Ships[z].objnum != Player_ai->target_objnum)){
		return SEXP_FALSE;
	}

	if (CDR(node) >= 0) {
		z = eval_num(CDR(node)) * 1000;
		if (!timestamp_has_time_elapsed(Players_target_timestamp, z)){
			return SEXP_FALSE;
		}

		if (CDR(CDR(node)) >= 0) {
			ptr = Player_ai->targeted_subsys;
			if (!ptr || subsystem_stricmp(ptr->system_info->subobj_name, CTEXT(CDR(CDR(node))))){
				return SEXP_FALSE;
			}
		}
	}

	return SEXP_TRUE;
}

int sexp_node_targeted(int node)
{
	int z;

	CJumpNode *jnp = jumpnode_get_by_name(CTEXT(node));

	if (jnp==NULL || !Player_ai || (jnp->GetSCPObjectNumber() != Player_ai->target_objnum)){
		return SEXP_FALSE;
	}

	if (CDR(node) >= 0) {
		z = eval_num(CDR(node)) * 1000;
		if (!timestamp_has_time_elapsed(Players_target_timestamp, z)){
			return SEXP_FALSE;
		}
	}

	return SEXP_TRUE;
}

int sexp_speed(int node)
{
	if (Training_context & TRAINING_CONTEXT_SPEED) {
		if (Training_context_speed_set){
			if (timestamp_has_time_elapsed(Training_context_speed_timestamp, eval_num(node) * 1000)){
				return SEXP_KNOWN_TRUE;
			}
		}
	}

	return SEXP_FALSE;
}

int sexp_get_throttle_speed(int node)
{
	player *the_player; 

	the_player = get_player_from_ship_node(node); 

	if (the_player != NULL) {
		float max_speed = Ship_info[Ships[Objects[the_player->objnum].instance].ship_info_index].max_speed;
		return (int)(the_player->ci.forward_cruise_percent / 100.0f * max_speed);
	}

	return 0;
}

// CommanderDJ
void sexp_set_player_throttle_speed(int node)
{
	//get and sanity check the player first
	player *the_player;
	the_player = get_player_from_ship_node(node); 

	if(the_player != NULL)
	{
		//now the throttle percentage
		node = CDR(node);
		int throttle_percent = eval_num(node);
		CLAMP(throttle_percent, 0, 100);
		
		//now actually set the throttle
		the_player->ci.forward_cruise_percent = (float) throttle_percent;
	}
}

// Goober5000
int sexp_primaries_depleted(int node)
{
	int sindex, num_banks, num_depleted_banks;
	ship *shipp;

	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if (sindex < 0) {
		return SEXP_FALSE;
	}

	shipp = &Ships[sindex];
	if (shipp->objnum < 0) {
		return SEXP_FALSE;
	}

	// see if ship has ballistic primary weapons   
	if (!(Ship_info[shipp->ship_info_index].flags & SIF_BALLISTIC_PRIMARIES))   
		return SEXP_FALSE; 

	// get num primary banks
	num_banks = shipp->weapons.num_primary_banks;
	num_depleted_banks = 0;

	// get number of depleted banks
	for (int idx=0; idx<num_banks; idx++)
	{
		// is this a ballistic bank?
		if (Weapon_info[shipp->weapons.primary_bank_weapons[idx]].wi_flags2 & WIF2_BALLISTIC)
		{
			// is this bank out of ammo?
			if (shipp->weapons.primary_bank_ammo[idx] == 0)
			{
				num_depleted_banks++;
			}
		}
	}

	// are they all depleted?
	return (num_depleted_banks == num_banks);
}

int sexp_secondaries_depleted(int node)
{
	int sindex, num_banks, num_depleted_banks;
	ship *shipp;

	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if (sindex < 0) {
		return SEXP_FALSE;
	}

	shipp = &Ships[sindex];
	if (shipp->objnum < 0) {
		return SEXP_FALSE;
	}

	// get num secondary banks
	num_banks = shipp->weapons.num_secondary_banks;
	num_depleted_banks = 0;

	// get number of depleted banks
	for (int idx=0; idx<num_banks; idx++) {
		if (shipp->weapons.secondary_bank_ammo[idx] == 0) {
			num_depleted_banks++;
		}
	}

	// are they all depleted?
	return (num_depleted_banks == num_banks);
}

int sexp_facing(int node)
{
	float a1, a2;
	vec3d v1, v2;

	if (!Player_obj) {
		return SEXP_FALSE;
	}

	ship *target_shipp = sexp_get_ship_from_node(node);
	if (target_shipp == NULL) {
		// hasn't arrived yet
		if (mission_parse_get_arrival_ship(CTEXT(node)) != NULL) {
			return SEXP_CANT_EVAL;
		}
		// not found and won't arrive: invalid
		return SEXP_KNOWN_FALSE;
	}
	double angle = atof(CTEXT(CDR(node)));

	v1 = Player_obj->orient.vec.fvec;
	vm_vec_normalize(&v1);

	vm_vec_sub(&v2, &Objects[target_shipp->objnum].pos, &Player_obj->pos);
	vm_vec_normalize(&v2);

	a1 = vm_vec_dotprod(&v1, &v2);
	a2 = (float) cos(ANG_TO_RAD(angle));
	if (a1 >= a2){
		return SEXP_TRUE;
	}

	return SEXP_FALSE;
}

int sexp_is_facing(int node)
{
	object *origin_objp, *target_objp;
	float a1, a2;
	vec3d v1, v2;
	
	ship *origin_shipp = sexp_get_ship_from_node(node);
	if (origin_shipp == NULL) {
		// hasn't arrived yet
		if (mission_parse_get_arrival_ship(CTEXT(node)) != NULL) {
			return SEXP_CANT_EVAL;
		}
		// not found and won't arrive: invalid
		return SEXP_KNOWN_FALSE;
	}
	node = CDR(node);

	ship *target_shipp = sexp_get_ship_from_node(node);
	if (target_shipp == NULL) {
		// hasn't arrived yet
		if (mission_parse_get_arrival_ship(CTEXT(node)) != NULL) {
			return SEXP_CANT_EVAL;
		}
		// not found and won't arrive: invalid
		return SEXP_KNOWN_FALSE;
	}
	node = CDR(node);

	double angle = atof(CTEXT(node));
	node = CDR(node);

	origin_objp = &Objects[origin_shipp->objnum];
	target_objp = &Objects[target_shipp->objnum];

	// check optional distance argument
	if (node > 0 && (sexp_distance3(origin_objp, target_objp) > eval_num(node))) {
		return SEXP_FALSE;
	}

	v1 = origin_objp->orient.vec.fvec;	
	vm_vec_normalize(&v1);

	vm_vec_sub(&v2, &target_objp->pos, &origin_objp->pos);
	vm_vec_normalize(&v2);

	a1 = vm_vec_dotprod(&v1, &v2);
	a2 = (float) cos(ANG_TO_RAD(angle));
	if (a1 >= a2){
		return SEXP_TRUE;
	}

	return SEXP_FALSE;
}

// is ship facing first waypoint in waypoint path
int sexp_facing2(int node)
{
	float a1, a2;
	vec3d v1, v2;

	// bail if Player_obj is not good
	if (!Player_obj) {
		return SEXP_CANT_EVAL;
	}

	// get player fvec
	v1 = Player_obj->orient.vec.fvec;
	vm_vec_normalize(&v1);

	// get waypoint name
	char *waypoint_name = CTEXT(node);

	// get position of first waypoint
	waypoint_list *wp_list = find_matching_waypoint_list(waypoint_name);

	if (wp_list == NULL) {
		return SEXP_CANT_EVAL;
	}

	vm_vec_sub(&v2, wp_list->get_waypoints().front().get_pos(), &Player_obj->pos);
	vm_vec_normalize(&v2);
	a1 = vm_vec_dotprod(&v1, &v2);
	a2 = (float) cos(ANG_TO_RAD(atof(CTEXT(CDR(node)))));
	if (a1 >= a2){
		return SEXP_TRUE;
	}

	return SEXP_FALSE;
}

int sexp_order(int n)
{
	char *order_to = CTEXT(n);
	char *order = CTEXT(CDR(n));
	char *target = NULL;

	//target
	n = CDDR(n); 
	if (n != -1)
		target = CTEXT(n);
	
	return hud_query_order_issued(order_to, order, target);
}

int sexp_query_orders (int n)
{
	char *order_to = CTEXT(n);
	char *order = CTEXT(CDR(n));
	char *target = NULL;
	char *order_from = NULL;
	char *special = NULL;
	int timestamp = 0;

	// delay
	n = CDDR(n); 
	if (n != -1) {
		timestamp = eval_sexp(n); 
		n = CDR(n); 
	}
	
	//target
	if (n != -1) {
		target = CTEXT(n);
		n = CDR(n); 
	}

	// player order comes from
	if (n != -1) {
		order_from = CTEXT(n);
		n = CDR(n); 
	}

	// optional special argument
	if (n != -1)
		special = CTEXT(n);

	return hud_query_order_issued(order_to, order, target, timestamp, order_from, special);
}

// Karajorma
void sexp_reset_orders (int n)
{
	Squadmsg_history.clear();
}

int sexp_waypoint_missed()
{
	if (Training_context & TRAINING_CONTEXT_FLY_PATH) {
		if (Training_context_at_waypoint > Training_context_goal_waypoint){
			return SEXP_TRUE;
		}
	}

	return SEXP_FALSE;
}

int sexp_waypoint_twice()
{
	if (Training_context & TRAINING_CONTEXT_FLY_PATH) {
		if (Training_context_at_waypoint < Training_context_goal_waypoint - 1){
			return SEXP_TRUE;
		}
	}

	return SEXP_FALSE;
}

int sexp_path_flown()
{
	if (Training_context & TRAINING_CONTEXT_FLY_PATH) {
		if ((uint) Training_context_goal_waypoint == Training_context_path->get_waypoints().size()){
			return SEXP_TRUE;
		}
	}

	return SEXP_FALSE;
}

void sexp_send_training_message(int node)
{
	int t = -1, delay = 0;

	if(physics_paused){
		return;
	}

	Assert(node >= 0);
	Assert(Event_index >= 0);

	if ((CDR(node) >= 0) && (CDR(CDR(node)) >= 0)) {
		delay = eval_num(CDR(CDR(node))) * 1000;
		t = CDR(CDR(CDR(node)));
		if (t >= 0){
			t = eval_num(t);
		}
	}

	multi_start_callback();
	multi_send_int(t);
	multi_send_int(delay);

	if ((Mission_events[Event_index].repeat_count > 1) || (CDR(node) < 0)){
		message_training_queue(CTEXT(node), timestamp(delay), t);
		multi_send_string(CTEXT(node));
	} else {
		message_training_queue(CTEXT(CDR(node)), timestamp(delay), t);
		multi_send_string(CTEXT(CDR(node)));
	}
	multi_end_callback();
}

void multi_sexp_send_training_message()
{	
	int t, delay;
	char message[TOKEN_LENGTH];

	multi_get_int(t);
	multi_get_int(delay);
	if (multi_get_string(message)) {
		message_training_queue(message, timestamp(delay), t); 
	}
	
}

int sexp_shield_recharge_pct(int node)
{
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}

	// shield recharge pct
	return (int)(100.0f * Energy_levels[Ships[sindex].shield_recharge_index]);
}

int sexp_engine_recharge_pct(int node)
{
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}

	// shield recharge pct
	return (int)(100.0f * Energy_levels[Ships[sindex].engine_recharge_index]);
}

int sexp_weapon_recharge_pct(int node)
{
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}

	// shield recharge pct
	return (int)(100.0f * Energy_levels[Ships[sindex].weapon_recharge_index]);
}

int sexp_shield_quad_low(int node)
{
	int sindex, idx;	
	float max_quad, check;
	ship_info *sip;
	object *objp;

	// get the ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return SEXP_FALSE;
	}
	if((Ships[sindex].objnum < 0) || (Ships[sindex].objnum >= MAX_OBJECTS)){
		return SEXP_FALSE;
	}
	if((Ships[sindex].ship_info_index < 0) || (Ships[sindex].ship_info_index >= Num_ship_classes)){
		return SEXP_FALSE;
	}
	objp = &Objects[Ships[sindex].objnum];
	sip = &Ship_info[Ships[sindex].ship_info_index];
	if(!(sip->flags & SIF_SMALL_SHIP)){
		return SEXP_FALSE;
	}
	max_quad = get_max_shield_quad(objp);	

	// shield pct
	check = (float)eval_num(CDR(node));

	// check his quadrants
	for(idx=0; idx<MAX_SHIELD_SECTIONS; idx++){
		if( ((objp->shield_quadrant[idx] / max_quad) * 100.0f) <= check ){
			return SEXP_TRUE;
		}
	}

	// all good
	return SEXP_FALSE;	
}

// Goober5000
int sexp_primary_ammo_pct(int node)
{
	ship *shipp;
	int sindex;
	int check, idx;
	int ret_sum[MAX_SHIP_PRIMARY_BANKS];
	int ret = 0;

	// get the ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0)
	{
		return 0;
	}
	if((Ships[sindex].objnum < 0) || (Ships[sindex].objnum >= MAX_OBJECTS))
	{
		return 0;
	}
	shipp = &Ships[sindex];
	
	// bank to check
	check = eval_num(CDR(node));

	// bogus check?
	if(check < 0){
		return 0;
	}

	// cumulative sum?
	if(check >= shipp->weapons.num_primary_banks)
	{
		for(idx=0; idx<shipp->weapons.num_primary_banks; idx++)
		{
			// ballistic
			if (Weapon_info[shipp->weapons.primary_bank_weapons[idx]].wi_flags2 & WIF2_BALLISTIC)
			{
				ret_sum[idx] = (int)(((float)shipp->weapons.primary_bank_ammo[idx] / (float)shipp->weapons.primary_bank_start_ammo[idx]) * 100.0f);
			}
			// non-ballistic
			else
			{
				ret_sum[idx] = 100;
			}
		}

		// add it up
		ret = 0;
		for(idx=0; idx<shipp->weapons.num_primary_banks; idx++)
		{
			ret += ret_sum[idx];
		}
		ret = (int)((float)ret / (float)shipp->weapons.num_primary_banks);
	}
	else
	{
		// ballistic
		if (Weapon_info[shipp->weapons.primary_bank_weapons[check]].wi_flags2 & WIF2_BALLISTIC)
		{
			ret = (int)(((float)shipp->weapons.primary_bank_ammo[check] / (float)shipp->weapons.primary_bank_start_ammo[check]) * 100.0f);
		}
		// non-ballistic
		else
		{
			ret = 100;
		}
	}	

	// return
	return ret;
}

int sexp_secondary_ammo_pct(int node)
{
	ship *shipp;
	int sindex;
	int check, idx;
	int ret_sum[MAX_SHIP_SECONDARY_BANKS];
	int ret = 0;

	// get the ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if((Ships[sindex].objnum < 0) || (Ships[sindex].objnum >= MAX_OBJECTS)){
		return 0;
	}
	shipp = &Ships[sindex];
	
	// bank to check
	check = eval_num(CDR(node));

	// bogus check?
	if(check < 0){
		return 0;
	}

	// cumulative sum?
	if(check >= shipp->weapons.num_secondary_banks){
		for(idx=0; idx<shipp->weapons.num_secondary_banks; idx++){
			ret_sum[idx] = (int)(((float)shipp->weapons.secondary_bank_ammo[idx] / (float)shipp->weapons.secondary_bank_start_ammo[idx]) * 100.0f);
		}

		// add it up
		ret = 0;
		for(idx=0; idx<shipp->weapons.num_secondary_banks; idx++){
			ret += ret_sum[idx];
		}
		ret = (int)((float)ret / (float)shipp->weapons.num_secondary_banks);
	} else {
		ret = (int)(((float)shipp->weapons.secondary_bank_ammo[check] / (float)shipp->weapons.secondary_bank_start_ammo[check]) * 100.0f);
	}	

	// return
	return ret;
}


// Karajorma - returns the number of bullets left in an ballistic ammo bank. Unlike primary_ammo_pct
// it does this as a numerical value rather than as a percentage of the maximum
int sexp_get_primary_ammo(int node)
{
	ship *shipp;
	int ammo_left = 0;
	int sindex;
	int check;

	// get the ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0)
	{
		return 0;
	}
	if((Ships[sindex].objnum < 0) || (Ships[sindex].objnum >= MAX_OBJECTS))
	{
		return 0;
	}
	shipp = &Ships[sindex];
	
	// bank to check
	check = eval_num(CDR(node));

	// bogus check?
	if(check < 0){
		return 0;
	}

	// cumulative sum?
	if(check >= shipp->weapons.num_primary_banks)
	{
		for(int bank=0; bank<shipp->weapons.num_primary_banks; bank++)
		{
			// Only ballistic weapons need to be counted
			if (Weapon_info[shipp->weapons.primary_bank_weapons[bank]].wi_flags2 & WIF2_BALLISTIC)
			{
				ammo_left += shipp->weapons.primary_bank_ammo[bank] ;
			}
		}
	}
	else
	{
		// Again only ballistic weapons need to be counted
		if (Weapon_info[shipp->weapons.primary_bank_weapons[check]].wi_flags2 & WIF2_BALLISTIC)
		{
			ammo_left = shipp->weapons.primary_bank_ammo[check] ;
		}
	}	

	// return
	return ammo_left;
}


// Karajorma - sets the amount of ammo in a certain ballistic weapon bank to the specified value
void sexp_set_primary_ammo (int node) 
{
	int sindex;
	int requested_bank ;
	int requested_weapons ;
	int rearm_limit = 0;

	// Check that a ship has been supplied
	sindex = ship_name_lookup(CTEXT(node));
	if (sindex < 0) 
	{
		return ;
	}

	// Get the bank to set the number on
	requested_bank = eval_num(CDR(node));
	if (requested_bank < 0)
	{
		return ;
	}

	//  Get the number of weapons requested
	node = CDR(node);
	requested_weapons = eval_num(CDR(node)); 
	if (requested_weapons < 0)
	{
		return ;
	}
	
	node = CDDR(node);	

	// If a rearm limit hasn't been specified simply change the ammo.Otherwise read in the rearm limit
	if (node < 0) {
		set_primary_ammo (sindex, requested_bank, requested_weapons);
	}
	else {
		rearm_limit = eval_num(node); 
		set_primary_ammo (sindex, requested_bank, requested_weapons, rearm_limit);
	}
}

//Karajorma - Helper function for the set-primary-ammo and weapon functions
void set_primary_ammo (int ship_index, int requested_bank, int requested_ammo, int rearm_limit, bool update)
{
	ship *shipp;
	int maximum_allowed ;

	// Check that it's valid
	if((Ships[ship_index].objnum < 0) || (Ships[ship_index].objnum >= MAX_OBJECTS)){
		return ;
	}
	shipp = &Ships[ship_index];
	
	// Can only set one bank at a time. Check that someone hasn't asked for the contents of all 
	// the banks or a non-existant bank
	if ((requested_bank > shipp->weapons.num_primary_banks) || requested_bank < 0)
	{
		return ;
	}

	// Check that this isn't a non-ballistic bank as it's pointless to set the amount of ammo for those
	if (!(Weapon_info[shipp->weapons.primary_bank_weapons[requested_bank]].wi_flags2 & WIF2_BALLISTIC))
	{
		return ;
	}

	//Check that a valid number of weapons have been specified. 
	if (requested_ammo < 0) 
	{
		return ;
	}

	// Is the number requested larger than the maximum allowed for that particular bank? 
	maximum_allowed = fl2i((Ship_info[shipp->ship_info_index].primary_bank_ammo_capacity[requested_bank] 
		/ Weapon_info[shipp->weapons.primary_bank_weapons[requested_bank]].cargo_size)+0.5);
	if (maximum_allowed < requested_ammo) 
	{
		requested_ammo = maximum_allowed ;
	}

	// Set the number of weapons
	shipp->weapons.primary_bank_ammo[requested_bank] = requested_ammo ;

	// If a rearm limit has been specified set it.
	if (rearm_limit >= 0)
	{
		// Don't allow more weapons than the bank can actually hold.
		if (rearm_limit > maximum_allowed) 
		{
			rearm_limit = maximum_allowed;
		}

		shipp->weapons.primary_bank_start_ammo[requested_bank] = rearm_limit;
	}

	// The change needs to be passed on if this is a multiplayer game
	if (MULTIPLAYER_MASTER & update)
	{
		send_weapon_or_ammo_changed_packet(ship_index, 0, requested_bank, requested_ammo, rearm_limit, -1);
	}
}

// Karajorma - returns the number of missiles left in an ammo bank. Unlike secondary_ammo_pct
// it does this as a numerical value rather than as a percentage of the maximum
int sexp_get_secondary_ammo (int node)
{
	int ammo_left = 0 ;
	ship *shipp ;
	int sindex ; 
	int check ;

	// Get the ship
	sindex = ship_name_lookup(CTEXT(node));
	if (sindex < 0) 
	{
		return 0;
	}
	if((Ships[sindex].objnum < 0) || (Ships[sindex].objnum >= MAX_OBJECTS)){
		return 0;
	}
	shipp = &Ships[sindex];
	
	// bank to check
	check = eval_num(CDR(node));

	// bogus check?
	if(check < 0){
		return 0;
	}

	// Are we looking at the number of secondaries in all banks? 
	if(check > shipp->weapons.num_secondary_banks)
	{
		for(int bank=0; bank<shipp->weapons.num_secondary_banks; bank++)
		{
			ammo_left += shipp->weapons.secondary_bank_ammo[bank] ;
		}
	}
	// If not return the value for the bank requested.
	else 
	{
		ammo_left = shipp->weapons.secondary_bank_ammo[check] ; 
	}

	return ammo_left ;
}

// Karajorma - sets the amount of ammo in a certain weapon bank to the specified value
void sexp_set_secondary_ammo (int node) 
{
	int sindex;
	int requested_bank;
	int requested_weapons;
	int rearm_limit;

	// Check that a ship has been supplied
	sindex = ship_name_lookup(CTEXT(node));
	if (sindex < 0) 
	{
		return ;
	}
	
	// Get the bank to set the number on
	requested_bank = eval_num(CDR(node));
	if (requested_bank < 0)
	{
		return ;
	}

	//  Get the number of weapons requested	
	node = CDR(node);
	requested_weapons = eval_num(CDR(node)); 
	if (requested_weapons < 0)
	{
		return ;
	}

	node = CDDR(node);	

	// If a rearm limit hasn't been specified simply change the ammo.Otherwise read in the rearm limit
	if (node < 0) {
		set_secondary_ammo(sindex, requested_bank, requested_weapons);
	}
	else {
		rearm_limit = eval_num(node); 
		set_secondary_ammo(sindex, requested_bank, requested_weapons, rearm_limit);
	}
}

//Karajorma - Helper function for the set-secondary-ammo and weapon functions
void set_secondary_ammo (int ship_index, int requested_bank, int requested_ammo, int rearm_limit, bool update)
{
	ship *shipp;
	int maximum_allowed;
	// Check that it's valid
	if((Ships[ship_index].objnum < 0) || (Ships[ship_index].objnum >= MAX_OBJECTS)){
		return ;
	}
	shipp = &Ships[ship_index];

	// Can only set one bank at a time. Check that someone hasn't asked for the contents of all 
	// the banks or a non-existant bank
	if ((requested_bank > shipp->weapons.num_secondary_banks) || requested_bank < 0)
	{
		return ;
	}
	
	if (requested_ammo < 0)
	{
		return ;
	}

	// Is the number requested larger than the maximum allowed for that particular bank? 
	maximum_allowed = fl2i(shipp->weapons.secondary_bank_capacity[requested_bank] 
		/ Weapon_info[shipp->weapons.secondary_bank_weapons[requested_bank]].cargo_size);
	if (maximum_allowed < requested_ammo) 
	{
		requested_ammo = maximum_allowed ;
	}

	// Set the number of weapons
	shipp->weapons.secondary_bank_ammo[requested_bank] = requested_ammo ;

	// If a rearm limit has been specified set it.
	if (rearm_limit >= 0)
	{
		// Don't allow more weapons than the bank can actually hold.
		if (rearm_limit > maximum_allowed) 
		{
			rearm_limit = maximum_allowed;
		}

		shipp->weapons.secondary_bank_start_ammo[requested_bank] = rearm_limit;
	}

	// The change needs to be passed on if this is a multiplayer game and we're not already updating the weapon
	if (MULTIPLAYER_MASTER && update)
	{
		send_weapon_or_ammo_changed_packet(ship_index, 1, requested_bank, requested_ammo, rearm_limit, -1);
	}
}

// Karajorma - Changes the weapon in the requested bank to the one supplied. Optionally sets the ammo and 
// rearm limit too. 
void sexp_set_weapon (int node, bool primary)
{
	ship *shipp;	
	int sindex, requested_bank, windex, requested_ammo=-1, rearm_limit=-1;

	Assert (node != -1);

	// Check that a ship has been supplied
	sindex = ship_name_lookup(CTEXT(node));
	if (sindex < 0) 
	{
		return ;
	}

	// Check that it's valid
	if((Ships[sindex].objnum < 0) || (Ships[sindex].objnum >= MAX_OBJECTS)){
		return ;
	}
	shipp = &Ships[sindex];

	// Get the bank to change the weapon of
	requested_bank = eval_num(CDR(node));

	// Skip to the node holding the weapon name
	node = CDDR(node);

	windex = weapon_info_lookup(CTEXT(node));
	if (windex < 0)
	{
		return;
	}

	if (primary)
	{
		// Change the weapon
		shipp->weapons.primary_bank_weapons[requested_bank] = windex ; 
	}
	else 
	{
		// Change the weapon
		shipp->weapons.secondary_bank_weapons[requested_bank] = windex ;
	}

	
	// Check to see if the optional ammo and rearm_limit settings were supplied
	node = CDR(node);
	if (node >= 0) {
		requested_ammo = eval_num(node); 
		
		if (requested_ammo < 0) {
			requested_ammo = 0; 
		}
		node = CDR(node);
		
		// If nothing was supplied then set the rearm limit to a negative value so that it is ignored
		if (node < 0) {
			rearm_limit = -1;
		}
		else {
			rearm_limit = eval_num(node); 
		}

		// Set the ammo but do not send an ammo changed update packet as we'll do that later from here
		if (primary) {
			set_primary_ammo (sindex, requested_bank, requested_ammo, rearm_limit, false);
		}
		else {
			set_secondary_ammo (sindex, requested_bank, requested_ammo, rearm_limit, false);
		}
	}
	else {
		// read the data 
		if (primary) {
			requested_ammo = shipp->weapons.primary_bank_ammo[requested_bank];
			rearm_limit = shipp->weapons.primary_bank_start_ammo[requested_bank];
		}
		else {
			requested_ammo = shipp->weapons.secondary_bank_ammo[requested_bank];
			rearm_limit = shipp->weapons.secondary_bank_start_ammo[requested_bank];
		}
	}

	// Now pass this info on to clients if need be.
	if (MULTIPLAYER_MASTER)
	{
		if (primary) {
			send_weapon_or_ammo_changed_packet(sindex, 0, requested_bank, requested_ammo, rearm_limit, windex);
		}
		else {
			send_weapon_or_ammo_changed_packet(sindex, 1, requested_bank, requested_ammo, rearm_limit, windex);
		}
	}
}

int sexp_get_countermeasures(int node) 
{
	ship *shipp;

	shipp = sexp_get_ship_from_node(node);

	if (shipp !=NULL) {
		return shipp->cmeasure_count;
	}
	else {
		return SEXP_NAN;
	}
}

void sexp_set_countermeasures(int node)
{
	ship *shipp;
	int num_cmeasures;

	shipp = sexp_get_ship_from_node(node);

	if (shipp == NULL) {
		return;
	}
	node = CDR(node);
	num_cmeasures = eval_num(node);
	if (num_cmeasures < 0) {
		num_cmeasures = 0;
	}
	else if (num_cmeasures > Ship_info[shipp->ship_info_index].cmeasure_max) {
		num_cmeasures = Ship_info[shipp->ship_info_index].cmeasure_max;
	}

	shipp->cmeasure_count = num_cmeasures;

	multi_start_callback();
	multi_send_ship(shipp);
	multi_send_int(num_cmeasures);
	multi_end_callback();
}

void multi_sexp_set_countermeasures()
{	
	int num_cmeasures = 0;
	ship *shipp; 

	multi_get_ship(shipp);
	if (shipp == NULL) {
		return;
	}
	if (multi_get_int(num_cmeasures)) {
		shipp->cmeasure_count = num_cmeasures;
	}
}

// KeldorKatarn - Locks or unlocks the afterburner on the requested ship
void sexp_deal_with_afterburner_lock (int node, bool lock)
{
	Assert (node != -1);
	sexp_deal_with_ship_flag(node, true, 0, 0, 0, SF2_AFTERBURNER_LOCKED, 0, 0, (lock ? 1:0), true);
}

// Karajorma - locks or unlocks primary weapons on the requested ship
void sexp_deal_with_primary_lock (int node, bool lock)
{
	Assert (node != -1);	
	sexp_deal_with_ship_flag(node, true, 0, 0, 0, SF2_PRIMARIES_LOCKED, 0, P2_SF2_PRIMARIES_LOCKED, (lock ? 1:0), true);

}

void sexp_deal_with_secondary_lock (int node, bool lock)
{
	Assert (node != -1);	
	sexp_deal_with_ship_flag(node, true, 0, 0, 0, SF2_SECONDARIES_LOCKED, 0, P2_SF2_SECONDARIES_LOCKED, (lock ? 1:0), true);

}

//Karajorma - Changes the subsystem name displayed on the HUD.  
void sexp_change_subsystem_name(int node) 
{
	ship *shipp;
	int ship_index;
	char *new_name;
	ship_subsys *subsystem_to_rename;

	Assert (node != -1);

	// Check that a ship has been supplied
	ship_index = ship_name_lookup(CTEXT(node));
	if (ship_index < 0) {
		return;
	}

	shipp = &Ships[ship_index];
	node = CDR(node);

	if (node < 0) {
		return;
	}

	new_name = CTEXT(node);
	node = CDR(node);
	
	if (MULTIPLAYER_MASTER) {
		multi_start_callback();
		multi_send_ship(shipp); 
		multi_send_string(new_name); 
	}

	// loop through all the subsystems the SEXP has provided
	while (node >= 0) {

		//Get the new subsystem name
		subsystem_to_rename = ship_get_subsys(&Ships[ship_index], CTEXT(node));
		if (subsystem_to_rename != NULL) {
			ship_subsys_set_name(subsystem_to_rename, new_name); 
	
			if (MULTIPLAYER_MASTER) {
				multi_send_string(CTEXT(node)); 
			}
		}

		node = CDR(node);
	}
	
	if (MULTIPLAYER_MASTER) {
		multi_end_callback();
	}
}

void multi_sexp_change_subsystem_name()
{
	ship *shipp = NULL;
	char new_name[TOKEN_LENGTH];
	char subsys_name[MAX_NAME_LEN];
	ship_subsys *subsystem_to_rename;

	multi_get_ship(shipp);
	multi_get_string(new_name);
	while (multi_get_string(subsys_name)) {
		subsystem_to_rename = ship_get_subsys(shipp, subsys_name);
		if (subsystem_to_rename != NULL) {
			ship_subsys_set_name(subsystem_to_rename, new_name);
		}
	}
}

// Goober5000
void sexp_change_ship_class(int n)
{
	int ship_num, class_num = ship_info_lookup(CTEXT(n));
	Assert(class_num != -1);

	n = CDR(n);

	if (MULTIPLAYER_MASTER) {
		multi_start_callback();
		multi_send_int(class_num);
	}

	// all ships in the sexp
	for ( ; n != -1; n = CDR(n))
	{
		ship_num = ship_name_lookup(CTEXT(n), 1);

		// If the ship hasn't arrived we still want the ability to change its class.
		if (ship_num == -1)
		{
			// Get the name of the ship that we are interested in
			char* ship_name = CTEXT(n); 
			p_object *pobj ;
			bool match_found = false;
			

			// Search the Ship_arrival_list to see if the ship is waiting to arrive
			for (pobj = GET_FIRST(&Ship_arrival_list); pobj != END_OF_LIST(&Ship_arrival_list); pobj = GET_NEXT(pobj))
			{
				if (!(strcmp(pobj->name, ship_name)))
				{
					match_found = true;
					break;
				}
			}

			if (match_found)
			{
				swap_parse_object(pobj, class_num);

				if (MULTIPLAYER_MASTER) {
					multi_send_bool(false); 
					multi_send_parse_object(pobj); 
				}
			}
		}
		// If the ship is already in the mission
		else 
		{
			// don't mess with a ship that's occupied
			if (!(Ships[ship_num].flags & (SF_DYING | SF_ARRIVING | SF_DEPARTING)))
			{
				change_ship_type(ship_num, class_num, 1);

				if (MULTIPLAYER_MASTER) {
					multi_send_bool(true); 
					multi_send_ship(ship_num); 
				}
			}
		}
	}

	if (MULTIPLAYER_MASTER) {
		multi_end_callback();
	}
}

void multi_sexp_change_ship_class()
{
	int class_num = -1;
	int ship_num = -1;
	bool ship_arrived;
	p_object *pobjp = NULL;

	multi_get_int(class_num);
	multi_get_bool(ship_arrived);

	if (ship_arrived) {
		multi_get_ship(ship_num);
		if ((class_num >= 0) && (ship_num >= 0)) {
			change_ship_type(ship_num, class_num, 1);
		}
	}
	else {
		multi_get_parse_object(pobjp); 
		if ((class_num >= 0) && (pobjp != NULL)) {
			swap_parse_object(pobjp, class_num);
		}
	}
}

// Goober5000
void ship_copy_damage(ship *target_shipp, ship *source_shipp)
{
	int i;
	object *target_objp = &Objects[target_shipp->objnum];
	object *source_objp = &Objects[source_shipp->objnum];
	ship_subsys *source_ss;
	ship_subsys *target_ss;

	if (target_shipp->ship_info_index != source_shipp->ship_info_index)
	{
		nprintf(("SEXP", "Copying damage of ship %s to ship %s which has a different ship class.  Strange results might occur.\n", source_shipp->ship_name, target_shipp->ship_name));
	}


	// copy hull...
	target_shipp->special_hitpoints = source_shipp->special_hitpoints;
	target_shipp->ship_max_hull_strength = source_shipp->ship_max_hull_strength;
	target_objp->hull_strength = source_objp->hull_strength;

	// ...and shields
	target_shipp->ship_max_shield_strength = source_shipp->ship_max_shield_strength;
	for (i = 0; i < MAX_SHIELD_SECTIONS; i++)
		target_objp->shield_quadrant[i] = source_objp->shield_quadrant[i];


	// search through all subsystems on source ship and map them onto target ship
	for (source_ss = GET_FIRST(&source_shipp->subsys_list); source_ss != GET_LAST(&source_shipp->subsys_list); source_ss = GET_NEXT(source_ss))
	{
		// find subsystem to configure
		target_ss = ship_get_subsys(target_shipp, source_ss->system_info->subobj_name);
		if (target_ss == NULL)
			continue;

		// copy
		target_ss->max_hits = source_ss->max_hits;
		target_ss->current_hits = source_ss->current_hits;
		target_ss->submodel_info_1.blown_off = source_ss->submodel_info_1.blown_off;
		target_ss->submodel_info_2.blown_off = source_ss->submodel_info_2.blown_off;
	}
}

int insert_subsys_status(p_object *pobjp);

// Goober5000
void parse_copy_damage(p_object *target_pobjp, ship *source_shipp)
{
	object *source_objp = &Objects[source_shipp->objnum];
	ship_subsys *source_ss;
	subsys_status *target_sssp;

	if (target_pobjp->ship_class != source_shipp->ship_info_index)
	{
		nprintf(("SEXP", "Copying damage of ship %s to ship %s which has a different ship class.  Strange results might occur.\n", source_shipp->ship_name, target_pobjp->name));
	}

	// copy hull...
	target_pobjp->special_hitpoints = source_shipp->special_hitpoints;
	target_pobjp->ship_max_hull_strength_multiplier = source_shipp->ship_max_hull_strength / Ship_info[source_shipp->ship_info_index].max_hull_strength;
	target_pobjp->initial_hull = fl2i(get_hull_pct(source_objp) * 100.0f);

	// ...and shields
	target_pobjp->ship_max_shield_strength_multiplier = source_shipp->ship_max_shield_strength / Ship_info[source_shipp->ship_info_index].max_shield_strength;
	target_pobjp->initial_shields = fl2i(get_shield_pct(source_objp) * 100.0f);


	// search through all subsystems on source ship and map them onto target ship
	for (source_ss = GET_FIRST(&source_shipp->subsys_list); source_ss != GET_LAST(&source_shipp->subsys_list); source_ss = GET_NEXT(source_ss))
	{
		// find subsystem to configure
		target_sssp = parse_get_subsys_status(target_pobjp, source_ss->system_info->subobj_name);

		// gak... none allocated; we need to allocate one!
		if (target_sssp == NULL)
		{
			// jam in the new subsystem at the end of the existing list
			int new_idx = insert_subsys_status(target_pobjp);
			target_sssp = &Subsys_status[new_idx];
			target_pobjp->subsys_count++;

			strcpy_s(target_sssp->name, source_ss->system_info->subobj_name);
		}

		// copy
		target_sssp->percent = 100.0f - (source_ss->current_hits / source_ss->max_hits) * 100.0f;
	}
}

// Goober5000
void sexp_ship_copy_damage(int node)
{
	int n, source_shipnum, target_shipnum;
	p_object *target_pobjp;

	// source ship must be present
	source_shipnum = ship_name_lookup(CTEXT(node));
	if (source_shipnum < 0)
		return;

	// loop through all subsequent arguments
	for (n = CDR(node); n != -1; n = CDR(n))
	{
		// maybe it's present in-mission
		target_shipnum = ship_name_lookup(CTEXT(n));
		if (target_shipnum >= 0)
		{
			ship_copy_damage(&Ships[target_shipnum], &Ships[source_shipnum]);
			continue;
		}

		// maybe it's on the arrival list
		target_pobjp = mission_parse_get_arrival_ship(CTEXT(n));
		if (target_pobjp != NULL)
		{
			parse_copy_damage(target_pobjp, &Ships[source_shipnum]);
			continue;
		}

		// must have departed or not even existed... do nothing
	}
}


//-Bobboau
void sexp_activate_deactivate_glow_points(int n, bool activate)
{
	int sindex;
	size_t i;

	for ( ; n != -1; n = CDR(n))
	{
		sindex = ship_name_lookup(CTEXT(n), 1);
		if (sindex >= 0)
		{
			for (i = 0; i < Ships[sindex].glow_point_bank_active.size(); i++)
				Ships[sindex].glow_point_bank_active[i] = activate;
		}
	}
}

//-Bobboau
void sexp_activate_deactivate_glow_point_bank(int n, bool activate)
{
	int sindex, num;

	sindex = ship_name_lookup(CTEXT(n), 1);
	if (sindex >= 0)
	{
		for ( ; n != -1; n = CDR(n))
		{
			num = eval_num(n);
			if (num >= 0 && num < (int)Ships[sindex].glow_point_bank_active.size())
			{
				Ships[sindex].glow_point_bank_active[num] = activate;
			}
		}
	}
}

//-Bobboau
void sexp_activate_deactivate_glow_maps(int n, int activate)
{
	int sindex;
	ship *shipp;

	for ( ; n != -1; n = CDR(n))
	{
		sindex = ship_name_lookup(CTEXT(n), 1);
		if (sindex >= 0)
		{
			shipp = &Ships[sindex];

			if (activate)
				shipp->flags2 &= ~SF2_GLOWMAPS_DISABLED;
			else
				shipp->flags2 |= SF2_GLOWMAPS_DISABLED;
		}
	}
}

void sexp_set_ambient_light(int node)
{
	int red, green, blue; 
	int level = 0;

	Assert(node > -1);
	red = eval_num(node); 
	if (red < 0 || red > 255) {
		red = 0;
	}

	node = CDR(node); 
	Assert(node > -1);
	green = eval_num(node); 
	if (green < 0 || green > 255) {
		green = 0;
	}

	node = CDR(node); 
	Assert(node > -1);
	blue = eval_num(node); 
	if (blue < 0 || blue > 255) {
		blue = 0;
	}

	level |= red; 
	level |= green << 8;
	level |= blue << 16;

	// setting the ambient light level for the mission won't actually do anything as it is parsed at mission 
	// start but it should be updated in case someone changes that later
	The_mission.ambient_light_level = level; 

	// call the graphics function to actually set the level
	gr_set_ambient_light(red, green, blue);

	// do the multiplayer callback
	if (MULTIPLAYER_MASTER) {
		multi_start_callback();
		multi_send_int(level);
		multi_end_callback();
	}
}

void multi_sexp_set_ambient_light()
{
	int level = 0;
	if (multi_get_int(level)) {
		The_mission.ambient_light_level = level;
		gr_set_ambient_light((level & 0xff),((level >> 8) & 0xff), ((level >> 16) & 0xff));
	}
}

void sexp_set_post_effect(int node)
{
	char *name = CTEXT(node);

	if (name == NULL) {
		return;
	}

	int amount = eval_num(CDR(node));

	if (amount < 0 || amount > 100)
		amount = 0;

	gr_post_process_set_effect(name, amount);
}

// Goober5000
void sexp_set_skybox_orientation(int n)
{
	matrix m;
	angles a;

	a.p = fl_radians(eval_num(n) % 360);
	n = CDR(n);

	a.b = fl_radians(eval_num(n) % 360);
	n = CDR(n);

	a.h = fl_radians(eval_num(n) % 360);
	n = CDR(n);

	vm_angles_2_matrix(&m, &a);
	stars_set_background_orientation(&m);
}

// taylor - load and set a skybox model
void sexp_set_skybox_model(int n)
{
	if ( !stricmp("default", CTEXT(n)) ) {
		stars_set_background_model( The_mission.skybox_model, NULL );
	} else {
		// stars_level_init() will set the actual mission skybox after this gets
		// evaluated during parse. by setting it now we get everything loaded so
		// there is less slowdown when it actually swaps out - taylor
		stars_set_background_model( CTEXT(n), NULL );
	}
}

// taylor - preload a skybox model.  this doesn't set anything as viewable, just loads it into memory
void sexp_set_skybox_model_preload(char *name)
{
	int i;

	if ( !stricmp("default", name) ) {
		// if there isn't a mission skybox model then don't load one
		if ( strlen(The_mission.skybox_model) ) {
			i = model_load( The_mission.skybox_model, 0, NULL );
			model_page_in_textures( i );
		}
	} else {
		i = model_load( name, 0, NULL );
		model_page_in_textures( i );
	}
}

void sexp_beam_fire(int node, bool at_coords)
{
	int sindex, n = node;
	beam_fire_info fire_info;		
	int idx;

	// zero stuff out
	memset(&fire_info, 0, sizeof(beam_fire_info));
	fire_info.accuracy = 0.000001f;							// this will guarantee a hit

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(n));
	n = CDR(n);
	if (sindex < 0) {
		return;
	}
	if (Ships[sindex].objnum < 0) {
		return;
	}
	fire_info.shooter = &Objects[Ships[sindex].objnum];

	// get the subsystem
	fire_info.turret = ship_get_subsys(&Ships[sindex], CTEXT(n));
	n = CDR(n);
	if (fire_info.turret == NULL) {
		return;
	}

	if (at_coords) {
		// get the target coordinates
		fire_info.target_pos1.xyz.x = fire_info.target_pos2.xyz.x = static_cast<float>(eval_num(n));
		n = CDR(n);
		fire_info.target_pos1.xyz.y = fire_info.target_pos2.xyz.y = static_cast<float>(eval_num(n));
		n = CDR(n);
		fire_info.target_pos1.xyz.z = fire_info.target_pos2.xyz.z = static_cast<float>(eval_num(n));
		n = CDR(n);
		fire_info.bfi_flags |= BFIF_TARGETING_COORDS;
		fire_info.target = NULL;
		fire_info.target_subsys = NULL;
	} else {
		// get the target
		sindex = ship_name_lookup(CTEXT(n));
		n = CDR(n);
		if (sindex < 0) {
			return;
		}
		if (Ships[sindex].objnum < 0) {
			return;
		}
		fire_info.target = &Objects[Ships[sindex].objnum];

		// see if the optional subsystem can be found	
		fire_info.target_subsys = NULL;
		if (n >= 0) {
			fire_info.target_subsys = ship_get_subsys(&Ships[sindex], CTEXT(n));
			n = CDR(n);
		}
	}

	// optionally force firing
	if (n >= 0 && is_sexp_true(n)) {
		fire_info.bfi_flags |= BFIF_FORCE_FIRING;
		n = CDR(n);
	}

	// get the second set of coordinates
	if (at_coords) {
		if (n >= 0) {
			fire_info.target_pos2.xyz.x = static_cast<float>(eval_num(n));
			n = CDR(n);
		}
		if (n >= 0) {
			fire_info.target_pos2.xyz.y = static_cast<float>(eval_num(n));
			n = CDR(n);
		}
		if (n >= 0) {
			fire_info.target_pos2.xyz.z = static_cast<float>(eval_num(n));
			n = CDR(n);
		}
	}

	// --- done getting arguments ---

	// if it has no primary weapons
	if (fire_info.turret->weapons.num_primary_banks <= 0) {
		Warning(LOCATION, "Couldn't fire turret on ship %s; subsystem %s has no primary weapons", CTEXT(node), CTEXT(CDR(node)));
		return;
	}

	// if the turret is destroyed
	if (!(fire_info.bfi_flags & BFIF_FORCE_FIRING) && fire_info.turret->current_hits <= 0.0f) {
		return;
	}

	// hmm, this could be wacky. Let's just simply select the first beam weapon in the turret
	fire_info.beam_info_index = -1;	
	for (idx=0; idx<fire_info.turret->weapons.num_primary_banks; idx++) {
		// store the weapon info index
		if (Weapon_info[fire_info.turret->weapons.primary_bank_weapons[idx]].wi_flags & WIF_BEAM) {
			fire_info.beam_info_index = fire_info.turret->weapons.primary_bank_weapons[idx];
		}
	}

	// fire the beam
	if (fire_info.beam_info_index != -1) {
		beam_fire(&fire_info);
	} else {
		// it would appear the turret doesn't have any beam weapons
		Warning(LOCATION, "Couldn't fire turret on ship %s; subsystem %s has no beam weapons", CTEXT(node), CTEXT(CDR(node)));
	}
}	

void sexp_beam_free(int node)
{
	int sindex;
	ship_subsys *turret = NULL;	

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	node = CDR(node);
	for ( ; node >= 0; node = CDR(node) ) {
		// get the subsystem
		turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(turret == NULL){
			continue;
		}

		// flag it as beam free :)
		if (!(turret->weapons.flags & SW_FLAG_BEAM_FREE))
		{
			turret->weapons.flags |= SW_FLAG_BEAM_FREE;
			turret->turret_next_fire_stamp = timestamp((int) frand_range(50.0f, 4000.0f));
		}
	}
}

void sexp_set_thrusters(int node) 
{
	bool activate = is_sexp_true(node) > 0;
	node = CDR(node);

	for(; node >= 0; node = CDR(node)) {
		int sindex = ship_name_lookup(CTEXT(node));
		
		if (sindex < 0) {
			continue;
		}

		if (Ships[sindex].objnum < 0) {
			continue;
		}

		if (activate)
			Ships[sindex].flags2 &= ~SF2_NO_THRUSTERS;
		else
			Ships[sindex].flags2 |= SF2_NO_THRUSTERS;
	}
}

void sexp_beam_free_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	for (int n = node; n >= 0; n = CDR(n)) {
		// get the firing ship
		sindex = ship_name_lookup( CTEXT(n) );

		if (sindex < 0) {
			continue;
		}

		if (Ships[sindex].objnum < 0) {
			continue;
		}

		// free all beam weapons
		subsys = GET_FIRST(&Ships[sindex].subsys_list);

		while ( subsys != END_OF_LIST(&Ships[sindex].subsys_list) ) {
			// just mark all turrets as beam free
			if ((subsys->system_info->type == SUBSYSTEM_TURRET) && (!(subsys->weapons.flags & SW_FLAG_BEAM_FREE)))
			{
				subsys->weapons.flags |= SW_FLAG_BEAM_FREE;
				subsys->turret_next_fire_stamp = timestamp((int) frand_range(50.0f, 4000.0f));
			}

			// next item
			subsys = GET_NEXT(subsys);
		}
	}
}

void sexp_beam_lock(int node)
{	
	int sindex;
	ship_subsys *turret = NULL;	

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	node = CDR(node);
	for ( ; node >= 0; node = CDR(node) ) {
		// get the subsystem
		turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(turret == NULL){
			continue;
		}

		// flag it as not beam free
		turret->weapons.flags &= ~(SW_FLAG_BEAM_FREE);
	}
}

void sexp_beam_lock_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	for (int n = node; n >= 0; n = CDR(n)) {
		// get the firing ship
		sindex = ship_name_lookup( CTEXT(n) );

		if (sindex < 0) {
			continue;
		}

		if (Ships[sindex].objnum < 0) {
			continue;
		}

		// lock all beam weapons
		subsys = GET_FIRST(&Ships[sindex].subsys_list);

		while ( subsys != END_OF_LIST(&Ships[sindex].subsys_list) ) {
			// just mark all turrets as not beam free
			if (subsys->system_info->type == SUBSYSTEM_TURRET) {
				subsys->weapons.flags &= ~(SW_FLAG_BEAM_FREE);
			}

			// next item
			subsys = GET_NEXT(subsys);
		}
	}
}

void sexp_turret_free(int node)
{
	int sindex;
	ship_subsys *turret = NULL;	

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	node = CDR(node);
	for ( ; node >= 0; node = CDR(node) ) {
		// get the subsystem
		turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(turret == NULL){
			continue;
		}

		// flag turret as no longer locked :)
		if (turret->weapons.flags & SW_FLAG_TURRET_LOCK) 
		{
			turret->weapons.flags &= (~SW_FLAG_TURRET_LOCK);
			turret->turret_next_fire_stamp = timestamp((int) frand_range(50.0f, 4000.0f));
		}
	}
}

void sexp_turret_free_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	for (int n = node; n >= 0; n = CDR(n)) {
		// get the firing ship
		sindex = ship_name_lookup( CTEXT(n) );

		if (sindex < 0) {
			continue;
		}

		if (Ships[sindex].objnum < 0) {
			continue;
		}

		// free all turrets
		subsys = GET_FIRST(&Ships[sindex].subsys_list);

		while ( subsys != END_OF_LIST(&Ships[sindex].subsys_list) ) {
			// just mark all turrets as free
			if ((subsys->system_info->type == SUBSYSTEM_TURRET) && (subsys->weapons.flags & SW_FLAG_TURRET_LOCK)) {
				subsys->weapons.flags &= (~SW_FLAG_TURRET_LOCK);
				subsys->turret_next_fire_stamp = timestamp((int) frand_range(50.0f, 4000.0f));
			}

			// next item
			subsys = GET_NEXT(subsys);
		}
	}
}

void sexp_turret_lock(int node)
{	
	int sindex;
	ship_subsys *turret = NULL;	

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	node = CDR(node);
	for ( ; node >= 0; node = CDR(node) ) {
		// get the subsystem
		turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(turret == NULL){
			continue;
		}

		// flag turret as locked
		turret->weapons.flags |= SW_FLAG_TURRET_LOCK;
	}
}

void sexp_turret_lock_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	for (int n = node; n >= 0; n = CDR(n)) {
		// get the firing ship
		sindex = ship_name_lookup( CTEXT(n) );

		if (sindex < 0) {
			continue;
		}

		if (Ships[sindex].objnum < 0) {
			continue;
		}

		// lock all turrets
		subsys = GET_FIRST(&Ships[sindex].subsys_list);

		while ( subsys != END_OF_LIST(&Ships[sindex].subsys_list) ) {
			// just mark all turrets as locked
			if (subsys->system_info->type == SUBSYSTEM_TURRET) {
				subsys->weapons.flags |= SW_FLAG_TURRET_LOCK;
			}

			// next item
			subsys = GET_NEXT(subsys);
		}
	}
}

void sexp_turret_tagged_only_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	// mark all turrets to only target tagged ships
	subsys = GET_FIRST(&Ships[sindex].subsys_list);
	while(subsys != END_OF_LIST(&Ships[sindex].subsys_list)){
		// just mark all turrets as locked
		if(subsys->system_info->type == SUBSYSTEM_TURRET){
			subsys->weapons.flags |= SW_FLAG_TAGGED_ONLY;
		}

		// next item
		subsys = GET_NEXT(subsys);
	}
}

void sexp_turret_tagged_clear_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	// mark all turrets so not restricted to only tagged ships
	subsys = GET_FIRST(&Ships[sindex].subsys_list);
	while(subsys != END_OF_LIST(&Ships[sindex].subsys_list)){
		// just mark all turrets as locked
		if(subsys->system_info->type == SUBSYSTEM_TURRET){
			subsys->weapons.flags &= (~SW_FLAG_TAGGED_ONLY);
		}

		// next item
		subsys = GET_NEXT(subsys);
	}
}

void sexp_turret_change_weapon(int node)
{	
	int sindex;
	int windex;	//hehe
	ship_subsys *turret = NULL;	
	ship_weapon *swp = NULL;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0 || Ships[sindex].objnum < 0){
		return;
	}

	//Get subsystem
	node = CDR(node);
	turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
	if(turret == NULL){
		return;
	}
	swp = &turret->weapons;

	node = CDR(node);
	windex = weapon_info_lookup(CTEXT(node));
	if(windex < 0) {
		return;
	}

	//Get the slot
	float capacity, size;
	int prim_slot = 0;
	int sec_slot = 0;

	node = CDR(node);
	prim_slot = eval_num(node);

	node = CDR(node);
	sec_slot = eval_num(node);

	if(prim_slot)
	{
		if(prim_slot > MAX_SHIP_PRIMARY_BANKS) {
			return;
		}

		if(prim_slot > swp->num_primary_banks) {
			swp->num_primary_banks++;
			prim_slot = swp->num_primary_banks;
		}

		//Get an index
		prim_slot--;

		//Get the max capacity
		capacity = (float) swp->primary_bank_capacity[prim_slot];
		size = (float) Weapon_info[windex].cargo_size;

		//Set various vars
		swp->primary_bank_start_ammo[prim_slot] = (int) (capacity / size);
		swp->primary_bank_ammo[prim_slot] = swp->primary_bank_start_ammo[prim_slot];
		swp->primary_bank_weapons[prim_slot] = windex;
		swp->primary_bank_rearm_time[prim_slot] = timestamp(0);
		swp->primary_bank_fof_cooldown[prim_slot] = 0.0f;
	}
	else if(sec_slot)
	{
		if(sec_slot > MAX_SHIP_PRIMARY_BANKS) {
			return;
		}

		if(sec_slot > swp->num_secondary_banks) {
			swp->num_secondary_banks++;
			sec_slot = swp->num_secondary_banks;
		}

		//Get an index
		sec_slot--;

		//Get the max capacity
		capacity = (float) swp->secondary_bank_capacity[sec_slot];
		size = (float) Weapon_info[windex].cargo_size;

		//Set various vars
		swp->secondary_bank_start_ammo[sec_slot] = (int) (capacity / size);
		swp->secondary_bank_ammo[sec_slot] = swp->secondary_bank_start_ammo[sec_slot];
		swp->secondary_bank_weapons[sec_slot] = windex;
		swp->secondary_bank_rearm_time[sec_slot] = timestamp(0);
	}
}

void sexp_set_armor_type(int node)
{	
	int sindex;
	int armor, rset;
	ship_subsys *ss = NULL;
	ship *shipp = NULL;
	ship_info *sip = NULL;

	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0) {
		return;
	}
	if(Ships[sindex].objnum < 0) {
		return;
	}
	shipp = &Ships[sindex];
	sip = &Ship_info[shipp->ship_info_index];

	// set or reset
	node = CDR(node);
	rset = is_sexp_true(node);

	// get armor
	node = CDR(node);
	if (!stricmp(SEXP_NONE_STRING, CTEXT(node))) {
		armor = -1;
	} else {
		armor = armor_type_get_idx(CTEXT(node));
	}
	
	//Set armor
	while(node != -1)
	{
		if (!stricmp(SEXP_HULL_STRING, CTEXT(node)))
		{
			// we are setting the ship itself
			if (!rset)
				shipp->armor_type_idx = sip->armor_type_idx;
			else
				shipp->armor_type_idx = armor;
		}
		else if (!stricmp(SEXP_SHIELD_STRING, CTEXT(node)))
		{
			// we are setting the ships shields
			if (!rset)
				shipp->shield_armor_type_idx = sip->shield_armor_type_idx;
			else
				shipp->shield_armor_type_idx = armor;
		}
		else 
		{
			// get the subsystem
			ss = ship_get_subsys(&Ships[sindex], CTEXT(node));
			if(ss == NULL){
				node = CDR(node);
				continue;
			}
		
			// set the range
			if (!rset)
				ss->armor_type_idx = ss->system_info->armor_type_idx;
			else
				ss->armor_type_idx = armor;
		}
		// next
		node = CDR(node);
	}
}

void sexp_weapon_set_damage_type(int node)
{	
	int windex, damage, swave, rset;
	size_t t;

	// weapon or shockwave
	swave = is_sexp_true(node);

	// get damage type
	node = CDR(node);
	if (!stricmp(SEXP_NONE_STRING, CTEXT(node)))
		damage = -1;
	else 
	{
		for(t = 0; t < Damage_types.size(); t++) 
		{
			if ( !stricmp(Damage_types[t].name, CTEXT(node)))  
				break;
		}
		if (t == Damage_types.size()) 
			return;
		damage = (int)t;
	}

	//Set or reset to defualt
	node = CDR(node);
	rset = is_sexp_true(node);
	
	//Set Damage
	node = CDR(node);
	while(node != -1)
	{
		// get the weapon
		windex = weapon_info_lookup(CTEXT(node));
		if(windex >= 0) 
		{
			// set the damage type
			if (swave)
				if (!rset)
					Weapon_info[windex].damage_type_idx = Weapon_info[windex].damage_type_idx_sav;
				else
					Weapon_info[windex].damage_type_idx = damage;
			else
				if (!rset)
					Weapon_info[windex].shockwave.damage_type_idx = Weapon_info[windex].shockwave.damage_type_idx_sav;
				else
					Weapon_info[windex].shockwave.damage_type_idx = damage;
		// next
		}
		node = CDR(node);
	}
}

void sexp_ship_set_damage_type(int node)
{	
	int sindex, damage, debris, rset;
	size_t t;
	ship *shipp = NULL;

	// collision or debris
	debris = is_sexp_true(node);

	// get damage type
	node = CDR(node);
	if (!stricmp(SEXP_NONE_STRING, CTEXT(node)))
		damage = -1;
	else 
	{
		for(t = 0; t < Damage_types.size(); t++) 
		{
			if ( !stricmp(Damage_types[t].name, CTEXT(node)))  
				break;
		}
		if (t == Damage_types.size()) 
			return;
		damage = (int)t;
	}

	//Set or reset to defualt
	node = CDR(node);
	rset = is_sexp_true(node);
	
	//Set Damage
	node = CDR(node);
	while(node != -1)
	{
		// get the ship
		sindex = ship_name_lookup(CTEXT(node));
		if(sindex >= 0) 
		{
			shipp = &Ships[sindex];
			// set the damage type
			if (debris)
			{
				if (!rset)
					shipp->collision_damage_type_idx = Ship_info[shipp->ship_info_index].collision_damage_type_idx;
				else
					shipp->collision_damage_type_idx = damage;
			}
			else 
			{
				if (!rset)
					shipp->debris_damage_type_idx = Ship_info[shipp->ship_info_index].debris_damage_type_idx;
				else
					shipp->debris_damage_type_idx = damage;
			}
			// next
		}
		node = CDR(node);
	}
}
void sexp_ship_shockwave_set_damage_type(int node)
{	
	int sindex, damage, rset;
	size_t t;

	// get damage type
	if (!stricmp(SEXP_NONE_STRING, CTEXT(node)))
		damage = -1;
	else 
	{
		for(t = 0; t < Damage_types.size(); t++) 
		{
			if ( !stricmp(Damage_types[t].name, CTEXT(node)))  
				break;
		}
		if (t == Damage_types.size()) 
			return;
		damage = (int)t;
	}

	//Set or reset to defualt
	node = CDR(node);
	rset = is_sexp_true(node);
	
	//Set Damage
	node = CDR(node);
	while(node != -1)
	{
		// get the ship
		sindex = ship_info_lookup(CTEXT(node));
		if(sindex >= 0) 
		{
			// set the damage type
			if (!rset)
				Ship_info[sindex].shockwave.damage_type_idx = Ship_info[sindex].shockwave.damage_type_idx_sav;
			else
				Ship_info[sindex].shockwave.damage_type_idx = damage;
			// next
		}
		node = CDR(node);
	}
}
void sexp_field_set_damage_type(int node)
{	
	int damage, rset;
	size_t t;

	// get damage type
	if (!stricmp(SEXP_NONE_STRING, CTEXT(node)))
		damage = -1;
	else 
	{
		for(t = 0; t < Damage_types.size(); t++) 
		{
			if ( !stricmp(Damage_types[t].name, CTEXT(node)))  
				break;
		}
		if (t == Damage_types.size()) 
			return;
		damage = (int)t;
	}

	//Set or reset to defualt
	node = CDR(node);
	rset = is_sexp_true(node);
	
	//Set Damage
	node = CDR(node);
	for(t = 0; t < Asteroid_info.size(); t++) 
	if (!rset)
		Asteroid_info[t].damage_type_idx = Asteroid_info[t].damage_type_idx_sav;
	else
		Asteroid_info[t].damage_type_idx = damage;
}
void sexp_turret_set_target_order(int node)
{	
	int sindex;
	ship_subsys *turret = NULL;	
	int oindex;
	int i;

	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	//Get turret subsys
	node = CDR(node);
	turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
	if(turret == NULL){
		return;
	}

	//Reset order
	for(i = 0; i < NUM_TURRET_ORDER_TYPES; i++) {
		turret->turret_targeting_order[i] = -1;
	}
	
	oindex = 0;
	node = CDR(node);
	while(node != -1)
	{
		if(oindex >= NUM_TURRET_ORDER_TYPES) {
			break;
		}

		for(i = 0; i < NUM_TURRET_ORDER_TYPES; i++) {
			if(!stricmp(Turret_target_order_names[i], CTEXT(node))) {
				turret->turret_targeting_order[oindex] = i;
			}
		}

		oindex++;
	}
}
void sexp_turret_set_direction_preference(int node)
{	
	int sindex;
	ship_subsys *turret = NULL;	
	
	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	//store direction preference
	int dirpref = eval_num(CDR(node));

	//Set range
	while(node != -1){
		// get the subsystem
		turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(turret == NULL){
			node = CDR(node);
			continue;
		}

		// set the range
		if(dirpref < 0)
			turret->optimum_range = turret->system_info->optimum_range;
		else
			if (dirpref == 0) {
				turret->favor_current_facing = 0.0f;
			} else {
				CAP(dirpref, 1, 100);
				turret->favor_current_facing = 1.0f + (((float) (100 - dirpref)) / 10.0f);
			}
		
		// next
		node = CDR(node);
	}
}

void sexp_turret_set_rate_of_fire(int node)
{	
	int sindex;
	ship_subsys *turret = NULL;	
	
	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	//store rof
	float rof = (float)eval_num(CDR(node));

	//Set rof
	while(node != -1){
		// get the subsystem
		turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(turret == NULL){
			node = CDR(node);
			continue;
		}

		// set the range
		if(rof < 0)
			turret->rof_scaler = turret->system_info->turret_rof_scaler ;
		else
			turret->rof_scaler = rof/100;
		
		// next
		node = CDR(node);
	}
}

void sexp_turret_set_optimum_range(int node)
{	
	int sindex;
	ship_subsys *turret = NULL;	
	
	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	//store range
	float range = (float)eval_num(CDR(node));

	//Set range
	while(node != -1){
		// get the subsystem
		turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(turret == NULL){
			node = CDR(node);
			continue;
		}

		// set the range
		if(range < 0)
			turret->optimum_range = turret->system_info->optimum_range;
		else
			turret->optimum_range = range;
		
		// next
		node = CDR(node);
	}
}

void sexp_turret_set_target_priorities(int node)
{	
	int sindex;
	ship_subsys *turret = NULL;	
	int i;
	int j;

	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	//Get turret subsys
	node = CDR(node);
	turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
	if(turret == NULL){
		return;
	}

	//Reset or new list
	node = CDR(node);   
	if(!(is_sexp_true(node))) {	//Reset
		turret->num_target_priorities = turret->system_info->num_target_priorities;
		for (j = 0; j < 32; j++) {
			turret->target_priority[j] = turret->system_info->target_priority[j];
		}
	}
	else {					//New List
		node = CDR(node);
		//clear the list
		turret->num_target_priorities = 0;
		for (i = 0; i < 32; i++) {
			turret->target_priority[i] = -1;
		}
		int num_groups = Ai_tp_list.size();
		// set the target priorities
		while(node != -1){
			if(turret->num_target_priorities < 32){
				for(j = 0; j < num_groups; j++) {
					if ( !stricmp(Ai_tp_list[j].name, CTEXT(node)))  {
							turret->target_priority[turret->num_target_priorities] = j;
							turret->num_target_priorities++;
					}
				}
			}
		// next
		node = CDR(node);
		}
	}
}

void sexp_ship_turret_target_order(int node)
{	
	int sindex;
	ship_subsys *turret = NULL;	
	int oindex;
	int i;
	int new_target_order[NUM_TURRET_ORDER_TYPES];

	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}
	
	oindex = 0;
	node = CDR(node);
	while(node != -1)
	{
		if(oindex >= NUM_TURRET_ORDER_TYPES) {
			break;
		}

		for(i = 0; i < NUM_TURRET_ORDER_TYPES; i++) {
			if(!stricmp(Turret_target_order_names[i], CTEXT(node))) {
				new_target_order[oindex] = i;
			}
		}

		oindex++;
	}

	turret = GET_FIRST(&Ships[sindex].subsys_list);
	while(turret != END_OF_LIST(&Ships[sindex].subsys_list))
	{
		//Reset order
		for(i = 0; i < NUM_TURRET_ORDER_TYPES; i++) {
			turret->turret_targeting_order[i] = -1;
		}

		memcpy(turret->turret_targeting_order, new_target_order, NUM_TURRET_ORDER_TYPES*sizeof(int));

		// next item
		turret = GET_NEXT(turret);
	}
}

// Goober5000
void sexp_set_subsys_rotation_lock_free(int node, int locked)
{
	int ship_num;		
	ship_subsys *rotate;

	// get the ship
	ship_num = ship_name_lookup(CTEXT(node));
	if (ship_num < 0)
		return;
	
	if (Ships[ship_num].objnum < 0)
		return;

	node = CDR(node);

	// loop for all specified subsystems
	for ( ; node >= 0; node = CDR(node) )
	{
		// get the rotating subsystem
		rotate = ship_get_subsys(&Ships[ship_num], CTEXT(node));
		if (rotate == NULL)
			continue;

		// set rotate or not, depending on flag
		if (locked)
		{
			rotate->flags &= ~SSF_ROTATES;
			if (rotate->subsys_snd_flags & SSSF_ROTATE)
			{
				obj_snd_delete_type(Ships[ship_num].objnum, rotate->system_info->rotation_snd, rotate);
				rotate->subsys_snd_flags &= ~SSSF_ROTATE;
			}
		}
		else
		{
			rotate->flags |= SSF_ROTATES;
			if (rotate->system_info->rotation_snd >= 0)
			{
				obj_snd_assign(Ships[ship_num].objnum, rotate->system_info->rotation_snd, &rotate->system_info->pnt, 0, OS_SUBSYS_ROTATION, rotate);
				rotate->subsys_snd_flags |= SSSF_ROTATE;
			}
		}
	}
}

// Goober5000
void sexp_reverse_rotating_subsystem(int node)
{
	int ship_num;
	ship_subsys *rotate;

	// get the ship
	ship_num = ship_name_lookup(CTEXT(node));
	if (ship_num < 0)
		return;
	
	if (Ships[ship_num].objnum < 0)
		return;

	node = CDR(node);

	// loop for all specified subsystems
	for ( ; node >= 0; node = CDR(node) )
	{
		// get the rotating subsystem
		rotate = ship_get_subsys(&Ships[ship_num], CTEXT(node));
		if (rotate == NULL)
			continue;

		// switch direction of rotation
		rotate->turn_rate *= -1.0f;
		rotate->submodel_info_1.cur_turn_rate *= -1.0f;
		rotate->submodel_info_1.desired_turn_rate *= -1.0f;
	}
}

// Goober5000
void sexp_rotating_subsys_set_turn_time(int node)
{
	int ship_num, n = node;
	float turn_time, turn_accel;
	ship_subsys *rotate;

	// get the ship
	ship_num = ship_name_lookup(CTEXT(n));
	if (ship_num < 0)
		return;
	if (Ships[ship_num].objnum < 0)
		return;
	n = CDR(n);

	// get the rotating subsystem
	rotate = ship_get_subsys(&Ships[ship_num], CTEXT(n));
	if (rotate == NULL)
		return;
	n = CDR(n);

	// get and set the turn time
	turn_time = eval_num(n) / 1000.0f;
	rotate->submodel_info_1.desired_turn_rate = PI2 / turn_time;
	n = CDR(n);

	// maybe get and set the turn accel
	if (n != -1)
	{
		turn_accel = eval_num(n) / 1000.0f;
		rotate->submodel_info_1.turn_accel = PI2 / turn_accel;
	}
	else
		rotate->submodel_info_1.cur_turn_rate = PI2 / turn_time;
}

void sexp_trigger_submodel_animation(int node)
{
	int ship_num, animation_type, animation_subtype, direction, n = node;
	bool instant;

	// get the ship
	ship_num = ship_name_lookup(CTEXT(n));
	if (ship_num < 0)
		return;
	if (Ships[ship_num].objnum < 0)
		return;
	n = CDR(n);

	// get the type
	animation_type = model_anim_match_type(CTEXT(n));
	if (animation_type == TRIGGER_TYPE_NONE)
	{
		Warning(LOCATION, "Unable to match animation type \"%s\"!", CTEXT(n));
		return;
	}
	n = CDR(n);

	// get the subtype
	animation_subtype = eval_num(n);
	n = CDR(n);

	// get the direction, 1 or -1
	direction = eval_num(n);
	if (direction != 1 && direction != -1)
	{
		Warning(LOCATION, "Direction is %d; it must be 1 or -1!", direction);
		return;
	}
	n = CDR(n);

	// instant or not
	if (n >= 0)
	{
		instant = (is_sexp_true(n) != 0);
		n = CDR(n);
	}
	else
		instant = false;

	// do we narrow it to a specific subsystem?
	if (n >= 0)
	{
		ship_subsys *ss = ship_get_subsys(&Ships[ship_num], CTEXT(n));
		if (ss == NULL)
		{
			Warning(LOCATION, "Subsystem \"%s\" not found on ship \"%s\"!", CTEXT(n), CTEXT(node));
			return;
		}
		model_anim_start_type(ss, animation_type, animation_subtype, direction, instant);
	}
	else
	{
		model_anim_start_type(&Ships[ship_num], animation_type, animation_subtype, direction, instant);
	}
}

void sexp_turret_tagged_specific(int node)
{
	ship_subsys *subsys;
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	node = CDR(node);
	for ( ; node >= 0; node = CDR(node) ) {
		// get the subsystem
		subsys = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(subsys == NULL){
			continue;
		}

		// flag turret as slaved to tag
		subsys->weapons.flags |= SW_FLAG_TAGGED_ONLY;
	}
}

void sexp_turret_tagged_clear_specific(int node)
{
	ship_subsys *subsys;
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	node = CDR(node);
	for ( ; node >= 0; node = CDR(node) ) {
		// get the subsystem
		subsys = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(subsys == NULL){
			continue;
		}

		// flag turret as slaved to tag
		subsys->weapons.flags &= (~SW_FLAG_TAGGED_ONLY);
	}
}

void sexp_add_remove_escort(int node)
{
	int sindex;
	int flag;
	char *whee;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	// determine whether to add or remove it
	whee = CTEXT(CDR(node));
	flag = eval_num(CDR(node));

	// add/remove
	if(flag){
		Ships[sindex].escort_priority = flag ;	
		hud_add_ship_to_escort(Ships[sindex].objnum, 1);
	} else {
		hud_remove_ship_from_escort(Ships[sindex].objnum);
	}

	multi_start_callback();
	multi_send_int(sindex);
	multi_send_int(flag); 
	multi_end_callback();
}

void multi_sexp_add_remove_escort()
{
	int sindex;
	int flag;

	multi_get_int(sindex); 
	if (!(multi_get_int(flag))) {
		return;
	}

	// add/remove
	if(flag){
		Ships[sindex].escort_priority = flag ;	
		hud_add_ship_to_escort(Ships[sindex].objnum, 1);
	} else {
		hud_remove_ship_from_escort(Ships[sindex].objnum);
	}
}

//given: two escort priorities and a list of ships
//do:    sets the most damaged one to the first priority and the rest to the second.
void sexp_damage_escort_list(int node)
{
	int n;
	int priority1;		//escort priority to set the most damaged ship
	int priority2;		//""         ""   to set the other ships

	ship* shipp;
	float smallest_hull_pct=1;		//smallest hull pct found
	int small_shipnum=-1;		//index in Ships[] of the above
	float current_hull_pct;			//hull pct of current ship we are evaluating
	int shipnum=-1;				//index in Ships[] of the above

	priority1=eval_num(node);
	priority2=eval_num(CDR(node));

	//go to start of ship list
	n=CDR(CDR(node));
	
	//loop through the ships
	for ( ; n != -1; n = CDR(n) )
	{
		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPARTED, CTEXT(n), NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, CTEXT(n), NULL, NULL) )
			continue;

		shipnum=ship_name_lookup(CTEXT(n));
		
		//it may be dead
		if (shipnum < 0)
			continue;

		if (Ships[shipnum].objnum < 0)
			continue;

		shipp=&Ships[shipnum];

		//calc hull integrity and compare
		current_hull_pct = get_hull_pct(&Objects[Ships[shipnum].objnum]);

		if (current_hull_pct < smallest_hull_pct)
		{
			Ships[small_shipnum].escort_priority=priority2;		//give the previous smallest the lower priority
			
			smallest_hull_pct=current_hull_pct;
			small_shipnum=shipnum;
	
			shipp->escort_priority=priority1;					//give the new smallest the higher priority
			hud_add_ship_to_escort(Ships[shipnum].objnum,1);
		}
		else														//if its bigger to begin with give it lower priority
		{
			shipp->escort_priority=priority2;
			hud_add_ship_to_escort(Ships[shipnum].objnum,1);
		}
	}
}

// Goober5000 - set stuff for mission support ship
void sexp_set_support_ship(int n)
{
	int i, temp_val;

	// get arrival location
	temp_val = -1;
	for (i=0; i<MAX_ARRIVAL_NAMES; i++)
	{
		if (!stricmp(CTEXT(n), Arrival_location_names[i]))
			temp_val = i;
	}
	if (temp_val < 0)
	{
		Warning(LOCATION, "Support ship arrival location '%s' not found.\n", CTEXT(n));
		return;
	}
	The_mission.support_ships.arrival_location = temp_val;

	// get arrival anchor
	n = CDR(n);
	if (!stricmp(CTEXT(n), "<no anchor>"))
	{
		// if no anchor, set arrival location to hyperspace
		The_mission.support_ships.arrival_location = 0;
	}
	else
	{
		// anchor must exist - look for it
		temp_val = -1;
		for (i=0; i<Num_parse_names; i++)
		{
			if (!stricmp(CTEXT(n), Parse_names[i]))
				temp_val = i;
		}
		// if not found, make a new entry
		if (temp_val < 0)
		{
			strcpy_s(Parse_names[Num_parse_names], CTEXT(n));
			temp_val = Num_parse_names;
			Num_parse_names++;
		}
		The_mission.support_ships.arrival_anchor = temp_val;
	}

	// get departure location
	n = CDR(n);
	temp_val = -1;
	for (i=0; i<MAX_DEPARTURE_NAMES; i++)
	{
		if (!stricmp(CTEXT(n), Departure_location_names[i]))
			temp_val = i;
	}
	if (temp_val < 0)
	{
		Warning(LOCATION, "Support ship departure location '%s' not found.\n", CTEXT(n));
		return;
	}
	The_mission.support_ships.departure_location = temp_val;

	// get departure anchor
	n = CDR(n);
	if (!stricmp(CTEXT(n), "<no anchor>"))
	{
		// if no anchor, set departure location to hyperspace
		The_mission.support_ships.departure_location = 0;
	}
	else
	{
		// anchor must exist - look for it
		temp_val = -1;
		for (i=0; i<Num_parse_names; i++)
		{
			if (!stricmp(CTEXT(n), Parse_names[i]))
				temp_val = i;
		}
		// if not found, make a new entry
		if (temp_val < 0)
		{
			strcpy_s(Parse_names[Num_parse_names], CTEXT(n));
			temp_val = Num_parse_names;
			Num_parse_names++;
		}
		The_mission.support_ships.departure_anchor = temp_val;
	}

	// get ship class
	n = CDR(n);
	temp_val = ship_info_lookup(CTEXT(n));
	if ((temp_val < 0) && ((stricmp(CTEXT(n), "<species support ship class>")) && (stricmp(CTEXT(n), "<any support ship class>"))) )
	{
		Warning(LOCATION, "Support ship class '%s' not found.\n", CTEXT(n));
		return;
	}
	if ((temp_val >= 0) && !(Ship_info[temp_val].flags & SIF_SUPPORT))
	{
		Warning(LOCATION, "Ship %s is not a support ship!", Ship_info[temp_val].name);
		return;
	}
	The_mission.support_ships.ship_class = temp_val;

	// get max number of ships allowed
	n = CDR(n);
	The_mission.support_ships.max_support_ships = eval_num(n);

	// get the number of concurrent ships allowed
	n = CDR(n);
	if ( n == -1 ) {
		// 7th arg not specified, set default
		The_mission.support_ships.max_concurrent_ships = 1;
	} else {
		The_mission.support_ships.max_concurrent_ships = eval_num(n);
	}
}

// Goober5000 - set stuff for arriving ships or wings
void sexp_set_arrival_info(int node)
{
	int i, arrival_location, arrival_anchor, arrival_mask, arrival_distance, n = node;
	bool show_warp;
	object_ship_wing_point_team oswpt;

	// get ship or wing
	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));
	n = CDR(n);

	// get arrival location
	arrival_location = -1;
	for (i=0; i<MAX_ARRIVAL_NAMES; i++)
	{
		if (!stricmp(CTEXT(n), Arrival_location_names[i]))
			arrival_location = i;
	}
	if (arrival_location < 0)
	{
		Warning(LOCATION, "Arrival location '%s' not found.\n", CTEXT(n));
		return;
	}
	n = CDR(n);

	// get arrival anchor
	arrival_anchor = -1;
	if ((n < 0) || !stricmp(CTEXT(n), "<no anchor>"))
	{
		// if no anchor, set arrival location to hyperspace
		arrival_location = 0;
	}
	else
	{
		// anchor must exist - look for it
		for (i=0; i<Num_parse_names; i++)
		{
			if (!stricmp(CTEXT(n), Parse_names[i]))
				arrival_anchor = i;
		}
		// if not found, make a new entry
		if (arrival_anchor < 0)
		{
			strcpy_s(Parse_names[Num_parse_names], CTEXT(n));
			arrival_anchor = Num_parse_names;
			Num_parse_names++;
		}
	}
	n = CDR(n);

	// get arrival path mask
	arrival_mask = 0;
	if (n >= 0)
		arrival_mask = eval_num(n);
	n = CDR(n);

	// get arrival distance
	arrival_distance = 0;
	if (n >= 0)
		arrival_distance = eval_num(n);
	n = CDR(n);

	// get warp effect
	show_warp = true;
	if (n >= 0)
		show_warp = (is_sexp_true(n) != 0);

	// now set all that information depending on the first argument
	if (oswpt.type == OSWPT_TYPE_SHIP)
	{
		oswpt.shipp->arrival_location = arrival_location;
		oswpt.shipp->arrival_anchor = arrival_anchor;
		oswpt.shipp->arrival_path_mask = arrival_mask;
		oswpt.shipp->arrival_distance = arrival_distance;

		if (show_warp)
			oswpt.shipp->flags &= ~SF_NO_ARRIVAL_WARP;
		else
			oswpt.shipp->flags |= SF_NO_ARRIVAL_WARP;
	}
	else if (oswpt.type == OSWPT_TYPE_WING || oswpt.type == OSWPT_TYPE_WING_NOT_PRESENT)
	{
		oswpt.wingp->arrival_location = arrival_location;
		oswpt.wingp->arrival_anchor = arrival_anchor;
		oswpt.wingp->arrival_path_mask = arrival_mask;
		oswpt.wingp->arrival_distance = arrival_distance;

		if (show_warp)
			oswpt.wingp->flags &= ~WF_NO_ARRIVAL_WARP;
		else
			oswpt.wingp->flags |= WF_NO_ARRIVAL_WARP;
	}
	else if (oswpt.type == OSWPT_TYPE_PARSE_OBJECT)
	{
		oswpt.p_objp->arrival_location = arrival_location;
		oswpt.p_objp->arrival_anchor = arrival_anchor;
		oswpt.p_objp->arrival_path_mask = arrival_mask;
		oswpt.p_objp->arrival_distance = arrival_distance;

		if (show_warp)
			oswpt.p_objp->flags &= ~P_SF_NO_ARRIVAL_WARP;
		else
			oswpt.p_objp->flags |= P_SF_NO_ARRIVAL_WARP;
	}
}

// Goober5000 - set stuff for departing ships or wings
void sexp_set_departure_info(int node)
{
	int i, departure_location, departure_anchor, departure_mask, n = node;
	bool show_warp;
	object_ship_wing_point_team oswpt;

	// get ship or wing
	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));
	n = CDR(n);

	// get departure location
	departure_location = -1;
	for (i=0; i<MAX_DEPARTURE_NAMES; i++)
	{
		if (!stricmp(CTEXT(n), Departure_location_names[i]))
			departure_location = i;
	}
	if (departure_location < 0)
	{
		Warning(LOCATION, "Departure location '%s' not found.\n", CTEXT(n));
		return;
	}
	n = CDR(n);

	// get departure anchor
	departure_anchor = -1;
	if ((n < 0) || !stricmp(CTEXT(n), "<no anchor>"))
	{
		// if no anchor, set departure location to hyperspace
		departure_location = 0;
	}
	else
	{
		// anchor must exist - look for it
		for (i=0; i<Num_parse_names; i++)
		{
			if (!stricmp(CTEXT(n), Parse_names[i]))
				departure_anchor = i;
		}
		// if not found, make a new entry
		if (departure_anchor < 0)
		{
			strcpy_s(Parse_names[Num_parse_names], CTEXT(n));
			departure_anchor = Num_parse_names;
			Num_parse_names++;
		}
	}
	n = CDR(n);

	// get departure path mask
	departure_mask = 0;
	if (n >= 0)
		departure_mask = eval_num(n);
	n = CDR(n);

	// get warp effect
	show_warp = true;
	if (n >= 0)
		show_warp = (is_sexp_true(n) != 0);

	// now set all that information depending on the first argument
	if (oswpt.type == OSWPT_TYPE_SHIP)
	{
		oswpt.shipp->departure_location = departure_location;
		oswpt.shipp->departure_anchor = departure_anchor;
		oswpt.shipp->departure_path_mask = departure_mask;

		if (show_warp)
			oswpt.shipp->flags &= ~SF_NO_DEPARTURE_WARP;
		else
			oswpt.shipp->flags |= SF_NO_DEPARTURE_WARP;
	}
	else if (oswpt.type == OSWPT_TYPE_WING || oswpt.type == OSWPT_TYPE_WING_NOT_PRESENT)
	{
		oswpt.wingp->departure_location = departure_location;
		oswpt.wingp->departure_anchor = departure_anchor;
		oswpt.wingp->departure_path_mask = departure_mask;

		if (show_warp)
			oswpt.wingp->flags &= ~WF_NO_DEPARTURE_WARP;
		else
			oswpt.wingp->flags |= WF_NO_DEPARTURE_WARP;
	}
	else if (oswpt.type == OSWPT_TYPE_PARSE_OBJECT)
	{
		oswpt.p_objp->departure_location = departure_location;
		oswpt.p_objp->departure_anchor = departure_anchor;
		oswpt.p_objp->departure_path_mask = departure_mask;

		if (show_warp)
			oswpt.p_objp->flags &= ~P_SF_NO_DEPARTURE_WARP;
		else
			oswpt.p_objp->flags |= P_SF_NO_DEPARTURE_WARP;
	}
}

// Goober5000
// set *all* the escort priorities of ships in escort list as follows: most damaged ship gets
// first priority in the argument list, next damaged gets next priority, etc.; if there are more
// ships than priorities, all remaining ships get the final priority on the list
// -- As indicated in the argument specification, there must be at least one argument but no more
// than MAX_COMPLETE_ESCORT_LIST arguments
void sexp_damage_escort_list_all(int n)
{
	typedef struct my_escort_ship
	{
		int index;
		float hull;
	} my_escort_ship;

	int priority[MAX_COMPLETE_ESCORT_LIST];
	my_escort_ship escort_ship[MAX_COMPLETE_ESCORT_LIST];
	int i, j, num_escort_ships, num_priorities, temp_i;
	float temp_f;
	ship *shipp;

	// build list of priorities
	num_priorities = 0;
	for ( ; n != -1; n = CDR(n) )
	{
		priority[num_priorities] = eval_num(n);
		num_priorities++;
	}

	// build custom list of escort ships
	num_escort_ships = 0;
	for (i = 0; i < MAX_SHIPS; i++)
	{
		shipp = &Ships[i];

		// make sure it exists
		if ( shipp->objnum < 0 )
			continue;

		// make sure it's on the escort list
		if ( !(shipp->flags & SF_ESCORT) )
			continue;

		// set index
		escort_ship[num_escort_ships].index = i;

		// calc and set hull integrity
		escort_ship[num_escort_ships].hull = get_hull_pct(&Objects[shipp->objnum]);

		num_escort_ships++;
	}

	// sort it bubbly, lowest hull to highest hull
	for (i = 0; i < num_escort_ships; i++)
	{
		for (j = 0; j < i; j++)
		{
			if (escort_ship[i].hull < escort_ship[j].hull)
			{
				// swap
				temp_i = escort_ship[i].index;
				temp_f = escort_ship[i].hull;
				escort_ship[i].index = escort_ship[j].index;
				escort_ship[i].hull = escort_ship[j].hull;
				escort_ship[j].index = temp_i;
				escort_ship[j].hull = temp_f;
			}
		}
	}

	// loop through and assign priorities
	for (i = 0; i < num_escort_ships; i++)
	{
		if (i >= num_priorities)
			Ships[escort_ship[i].index].escort_priority = priority[num_priorities - 1];
		else
			Ships[escort_ship[i].index].escort_priority = priority[i];
	}

	// redo the escort list
	hud_setup_escort_list();
}

void sexp_awacs_set_radius(int node)
{
	int sindex;		
	ship_subsys *awacs;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}	

	// get the awacs subsystem
	awacs = ship_get_subsys(&Ships[sindex], CTEXT(CDR(node)));
	if(awacs == NULL){
		return;
	}

	if (!(awacs->system_info->flags & MSS_FLAG_AWACS))
		return;

	// set the new awacs radius
	awacs->awacs_radius = (float)eval_num(CDR(CDR(node)));
}

// Goober5000
void sexp_primitive_sensors_set_range(int n)
{
	char *ship_name = CTEXT(n);
	int ship_num, range = eval_num(CDR(n));

	// check to see if ship destroyed or departed.  In either case, do nothing.
	if ( mission_log_get_time(LOG_SHIP_DEPARTED, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
		return;

	// get the ship
	ship_num = ship_name_lookup(ship_name);

	// ship not yet in mission? do nothing
	if (ship_num < 0)
		return;

	// set the new range
	Ships[ship_num].primitive_sensor_range = range;
}


//*************************************************************************************************
// Kazan
// AutoNav/AutoPilot system SEXPS

//text: set-nav-carry
//args: 1+, Ship/Wing name
void set_nav_carry_status(int node)
{
	int n=node, i;
	char *name;
	bool skip;

	while (n != -1)
	{
		name = CTEXT(n);
		skip = false;

		for (i = 0; i < MAX_WINGS; i++)
		{
			if (!stricmp(Wings[i].name, name))
			{
				Wings[i].flags |= WF_NAV_CARRY;
				skip = true;
				break;
			}
		}

		if (!skip)
		{
			for (i = 0; i < MAX_SHIPS; i++)
			{
				if (Ships[i].objnum != -1 && !stricmp(Ships[i].ship_name, name))
				{
					Ships[i].flags2 |= SF2_NAVPOINT_CARRY;
					break;
				}
			}
		}
		
		// move to next ship/wing in list
		n = CDR(n);
	}
}

//text: unset-nav-carry
//args: 1+, Ship/Wing name
void unset_nav_carry_status(int node)
{
	int n=node, i;
	char *name;
	bool skip;


	while (n != -1)
	{
		name = CTEXT(n);
		skip = false;

		for (i = 0; i < MAX_WINGS; i++)
		{
			if (!stricmp(Wings[i].name, name))
			{
				Wings[i].flags &= ~WF_NAV_CARRY;
				skip = true;
				break;

			}
		}

		if (!skip)
		{
			for (i = 0; i < MAX_SHIPS; i++)
			{
				if (Ships[i].objnum != -1 && !stricmp(Ships[i].ship_name, name))
				{
					Ships[i].flags2 &= ~SF2_NAVPOINT_CARRY;
					break;
				}
			}
		}
		
		// move to next ship/wing in list
		n = CDR(n);
	}
}


//text: set-nav-needslink
//args: 1+, Ship/Wing name
void set_nav_needslink(int node)
{
	int n=node, i;
	char *name;

	while (n != -1)
	{
		name = CTEXT(n);

		for (i = 0; i < MAX_SHIPS; i++)
		{
			if (Ships[i].objnum != -1 && !stricmp(Ships[i].ship_name, name))
			{
				Ships[i].flags2 &= ~SF2_NAVPOINT_CARRY;
				Ships[i].flags2 |= SF2_NAVPOINT_NEEDSLINK;
				break;
			}
		}
		// move to next ship/wing in list
		n = CDR(n);
	}
}

//text: unset-nav-needslink
//args: 1+, Ship/Wing name
void unset_nav_needslink(int node)
{
	int n=node, i;
	char *name;

	while (n != -1)
	{
		name = CTEXT(n);


		for (i = 0; i < MAX_SHIPS; i++)
		{
			if (Ships[i].objnum != -1 && !stricmp(Ships[i].ship_name, name))
			{
				Ships[i].flags2 &= ~SF2_NAVPOINT_NEEDSLINK;
				break;
			}
		}

		
		// move to next ship/wing in list
		n = CDR(n);
	}
}

void add_nav_waypoint(char *nav, char *WP_path, int vert, char *oswpt_name)
{	
	int i;
	object_ship_wing_point_team oswpt;
	bool add_for_this_player = true; 

	if (oswpt_name != NULL) {
		sexp_get_object_ship_wing_point_team(&oswpt, oswpt_name); 

		// we can't assume this nav should be visible to the player any more
		add_for_this_player = false;

		switch (oswpt.type)
		{
			case OSWPT_TYPE_TEAM:
				if (oswpt.team == Player_ship->team) {
					add_for_this_player = true; 
				}

			case OSWPT_TYPE_SHIP:
				if (oswpt.shipp == Player_ship) {
					add_for_this_player = true; 
				}

			case OSWPT_TYPE_WING:
				for ( i = 0; i < oswpt.wingp->current_count; i++) {
					if (Ships[oswpt.wingp->ship_index[i]].objnum == Player_ship->objnum) {
						add_for_this_player = true; 
					}
				}

			// for all other oswpt types we simply return
			default:
				return;
		}
	}

	if (add_for_this_player) {		
		AddNav_Waypoint(nav, WP_path, vert, 0);
	}
}


//text: add-nav-waypoint
//args: 4, Nav Name, Waypoint Path Name, Waypoint Path point, ShipWingTeam
void add_nav_waypoint(int node)
{
	char *nav_name = CTEXT(node);
	char *way_name = CTEXT(CDR(node));
	int  vert = eval_num(CDR(CDR(node)));	
	char *oswpt_name;

	node = CDR(CDR(CDR(node)));
	if (node >=0) {
		oswpt_name = CTEXT(node); 
	}
	else {
		oswpt_name = NULL;
	}

	add_nav_waypoint(nav_name, way_name, vert, oswpt_name);

	multi_start_callback();
	multi_send_string(nav_name);
	multi_send_string(way_name);
	multi_send_int(vert);

	if (oswpt_name != NULL) {
		multi_send_string(oswpt_name);
	}

	multi_end_callback();
}

void multi_sexp_add_nav_waypoint()
{
	char nav_name[TOKEN_LENGTH];
	char way_name[TOKEN_LENGTH];
	char oswpt_name[TOKEN_LENGTH];
	int vert;

	if (!multi_get_string(nav_name)) {
		return; 
	}
	
	if (!multi_get_string(way_name)) {
		return;
	}

	if (!multi_get_int(vert)) {
		return;
	}

	if (!multi_get_string(oswpt_name)) {
		add_nav_waypoint(nav_name, way_name, vert, NULL);		
	}
	else {
		add_nav_waypoint(nav_name, way_name, vert, oswpt_name);
	}

	AddNav_Waypoint(nav_name, way_name, vert, 0);
}


//text: add-nav-ship
//args: 2, Nav Name, Ship Name
void add_nav_ship(int node)
{
	char *nav_name = CTEXT(node);
	char *ship_name = CTEXT(CDR(node));
	AddNav_Ship(nav_name, ship_name, 0);

	multi_start_callback();
	multi_send_string(nav_name);
	multi_send_string(ship_name);
	multi_end_callback();
}

void multi_add_nav_ship()
{
	char nav_name[TOKEN_LENGTH];
	char ship_name[TOKEN_LENGTH];

	if (!multi_get_string(nav_name)) {
		return; 
	}
	
	if (!multi_get_string(ship_name)) {
		return;
	}

	AddNav_Ship(nav_name, ship_name, 0);
}


//text: del-nav
//args: 1, Nav Name
void del_nav(int node)
{
	char *nav_name = CTEXT(node);
	DelNavPoint(nav_name);

	multi_start_callback();
	multi_send_string(nav_name);
	multi_end_callback();

}

void multi_del_nav()
{
	char nav_name[TOKEN_LENGTH];

	if (!multi_get_string(nav_name)) {
		return; 
	}
	
	DelNavPoint(nav_name);
}

//text: use-nav-cinematics
//args: 1, boolean enable/disable
void set_use_ap_cinematics(int node)
{
	if (is_sexp_true(node))
	{
		The_mission.flags |= MISSION_FLAG_USE_AP_CINEMATICS;
	}
	else
	{
		The_mission.flags &= ~MISSION_FLAG_USE_AP_CINEMATICS;
	}
}

//text: use-autopilot
//args: 1, boolean enable/disable
void set_use_ap(int node)
{
	if (is_sexp_true(node))
	{
		The_mission.flags &= ~MISSION_FLAG_DEACTIVATE_AP;
	}
	else
	{
		The_mission.flags |= MISSION_FLAG_DEACTIVATE_AP;
	}
}

//text: hide-nav
//args: 1, Nav Name
void hide_nav(int node)
{
	char *nav_name = CTEXT(node);
	Nav_Set_Hidden(nav_name);
}

//text: restrict-nav
//args: 1, nav name
void restrict_nav(int node)
{
	char *nav_name = CTEXT(node);
	Nav_Set_NoAccess(nav_name);
}

//text: unhide-nav
//args: 1, Nav name
void unhide_nav(int node)
{
	char *nav_name = CTEXT(node);
	Nav_UnSet_Hidden(nav_name);
}

//text: unrestrict-nav
//args: 1, nav name
void unrestrict_nav(int node)
{
	char *nav_name = CTEXT(node);
	Nav_UnSet_NoAccess(nav_name);

}

//text: set-nav-visited
//args: 1, Nav Name
void set_nav_visited(int node)
{
	char *nav_name = CTEXT(node);
	Nav_Set_Visited(nav_name);
}

//text: unset-nav-visited
//args: 1, Nav Name
void unset_nav_visited(int node)
{
	char *nav_name = CTEXT(node);
	Nav_UnSet_Visited(nav_name);
}


//text: is-nav-visited
//args: 1, Nav Name
//rets: true/false
int is_nav_visited(int node)
{
	char *nav_name = CTEXT(node);
	return IsVisited(nav_name);
}


//text: is-nav_linked
//args: 1, Ship name
//rets: true/false
int is_nav_linked(int node)
{
	char *ship_name = CTEXT(node);
	for (int i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum != -1 && !stricmp(Ships[i].ship_name, ship_name))
		{
			return (Ships[i].flags2 & SF2_NAVPOINT_CARRY) != 0;
		}
	}
	return 0;
}

//text: distance-to-nav
//args: 1, Nav Name
//rets: distance to nav
int distance_to_nav(int node)
{
	char *nav_name = CTEXT(node);
	return DistanceTo(nav_name);
}


//*************************************************************************************************


int sexp_is_tagged(int node)
{
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return SEXP_FALSE;
	}
	if(Ships[sindex].objnum < 0){
		return SEXP_FALSE;
	}
	object *caller = &Objects[Ships[sindex].objnum];
	if(ship_is_tagged(caller)) { // This line and the one above were added.
		return SEXP_TRUE;
	}

	// not tagged
	return SEXP_FALSE;
}

// Joint effort of Sesquipedalian and Goober5000.  Sesq found the code, mucked around making
// sexps with it and learned things, Goober taught Sesq and made the sexp work properly. =D
// Returns true so long as the player has held a missile lock for the specified time.
// If the optional ship and/or ship's subsystem are specified, returns true when that
// has been locked onto, but otherwise returns as long as anything has been locked onto.
int sexp_missile_locked(int node)
{
	int z;

	// if we aren't targeting anything, it's false
	if ((Players_target == -1) || (Players_target == UNINITIALIZED))
		return SEXP_FALSE;

	// if we aren't locked on to anything, it's false
	if (!Players_mlocked)
		return SEXP_FALSE;

	// do we have a specific ship?
	if (CDR(node) != -1)
	{
		// if we're not targeting the specific ship, it's false
		if (stricmp(Ships[Objects[Players_target].instance].ship_name, CTEXT(CDR(node))))
			return SEXP_FALSE;

		// do we have a specific subsystem?
		if (CDR(CDR(node)) != -1)
		{
			// if we aren't targeting a subsystem at all, it's false
			if (!Player_ai->targeted_subsys)
				return SEXP_FALSE;

			// if we're not targeting the specific subsystem, it's false
			if (subsystem_stricmp(Player_ai->targeted_subsys->system_info->subobj_name, CTEXT(CDR(CDR(node)))))
				return SEXP_FALSE;
		}
	}

	// if we've gotten this far, we must have satisfied whatever conditions the sexp imposed
	// finally, test if we've locked for a certain period of time
	z = eval_num(node) * 1000;
	if (timestamp_has_time_elapsed(Players_mlocked_timestamp, z))
	{
		return SEXP_TRUE;
	}

	return SEXP_FALSE;
}

int sexp_is_player (int node) 
{
	int sindex, np_index;
	p_object *p_objp;

	int standard_check = is_sexp_true(node);

	if (!(Game_mode & GM_MULTIPLAYER)){	
		sindex = ship_name_lookup(CTEXT(CDR(node)));

		// There can only be one player ship in singleplayer so if more than one ship is specifed the sexp is false
		if (CDDR(node) < 0 ) {
			return SEXP_FALSE;
		}

		if(sindex >= 0){
			if(Player_obj == &Objects[Ships[sindex].objnum]){
				if (standard_check) {
					if (Player_use_ai) {
						return SEXP_FALSE;
					}
				}
				return SEXP_TRUE;
			}
		}
		return SEXP_FALSE;
	}

	// For multiplayer we need to decide what to do about respawning players
	else {		
		node = CDR(node);
		while (node >= 0) {
			// reset the netplayer index
			np_index = -1; 

			sindex = ship_name_lookup(CTEXT(node));
			if(sindex >= 0){
				if(Ships[sindex].objnum >= 0) {
					// try and find the player
					np_index = multi_find_player_by_object(&Objects[Ships[sindex].objnum]);
				}
			}
			
			if (standard_check && np_index < 0) {
				// Respawning ships don't have an objnum so we need to take a different approach 
				p_objp = mission_parse_get_arrival_ship(CTEXT(node));
				if (p_objp != NULL) {
					np_index = multi_find_player_by_parse_object(p_objp);
				}
			}
			
			// if we couldn't find a valid netplayer index then the ship isn't a player
			if((np_index < 0) || (np_index >= MAX_PLAYERS)){
				return SEXP_FALSE;
			}
			
			node = CDR(node);
		}

		// if we reached this far they all checked out
		return SEXP_TRUE;
	}
}

void sexp_set_respawns(int node)
{
	int num_respawns; 
	p_object *p_objp;

	// we're wasting our time if you can't respawn
	if (!(Game_mode & GM_MULTIPLAYER)) {
		return;
	}

	num_respawns = eval_num(node);

	node = CDR(node);

	// send the information to clients
	multi_start_callback();
	multi_send_int(num_respawns); 

	while (node != -1) {
		// get the parse object for the ship
		p_objp = mission_parse_get_arrival_ship(CTEXT(node));
		if (p_objp != NULL) {
			p_objp->respawn_count = num_respawns;
		}

		multi_send_string(CTEXT(node)); 

		node = CDR(node);
	}
	
	multi_end_callback();
}

void multi_sexp_set_respawns()
{
	p_object *p_objp;
	int num_respawns;
	char parse_name[NAME_LENGTH];

	multi_get_int(num_respawns); 
	while (multi_get_string(parse_name)) {
		// get the parse object for the ship
		p_objp = mission_parse_get_arrival_ship(parse_name);
		if (p_objp != NULL) {
			p_objp->respawn_count = num_respawns;
		}
	}
}

// helper function for the remove-weapons SEXP
void actually_remove_weapons(int weapon_info_index)
{
	int i; 

	for (i = 0; i<MAX_WEAPONS; i++) {
		// weapon doesn't match the optional weapon 
		if ((weapon_info_index > -1) && (Weapons[i].weapon_info_index != weapon_info_index)) {
			continue;
		}

		if (Weapons[i].objnum >= 0) {
			Objects[Weapons[i].objnum].flags |= OF_SHOULD_BE_DEAD;
		}		
	}
}

void sexp_remove_weapons(int node)
{ 
	int weapon_info_index = -1;

	// if we have the optional argument, read it in
	if (node >= 0) {
		weapon_info_index = weapon_info_lookup(CTEXT(node));
		if (weapon_info_index == -1) {
			char *buf = CTEXT(node); 
			mprintf(("Remove-weapons attempted to remove %s. Weapon not found. Remove-weapons will remove all weapons currently in the mission", buf)); 
		}
	}

	actually_remove_weapons(weapon_info_index); 
	
	// send the information to clients
	multi_start_callback();
	multi_send_int(weapon_info_index); 
	multi_end_callback();
}

void multi_sexp_remove_weapons()
{
	int weapon_info_index = -1; 

	multi_get_int(weapon_info_index);
	
	actually_remove_weapons(weapon_info_index); 
}


int sexp_return_player_data(int node, int type)
{
	int sindex, np_index = -1;
	player *p = NULL;
	p_object *p_objp;

	sindex = ship_name_lookup(CTEXT(node));

	if(Game_mode & GM_MULTIPLAYER){			
		if(sindex >= 0){
			if(Ships[sindex].objnum >= 0) {
				// try and find the player
				np_index = multi_find_player_by_object(&Objects[Ships[sindex].objnum]);
			}
		}
		
		if (np_index < 0) {
			// Respawning ships don't have an objnum so we need to take a different approach 
			p_objp = mission_parse_get_arrival_ship(CTEXT(node));
			np_index = multi_find_player_by_parse_object(p_objp);
		}
		
		if((np_index >= 0) && (np_index < MAX_PLAYERS)){
			p = Net_players[np_index].m_player;
		}
	}
	// if we're in single player, we're only concerned with ourself
	else {
		if(sindex < 0){
			return 0;
		}
		if(Ships[sindex].objnum < 0){
			return 0;
		}
		
		if(Player_obj == &Objects[Ships[sindex].objnum]){
			p = Player;
		}
	}

	// now, if we have a valid player, return his kills
	if(p != NULL) {
		switch (type) {
			case OP_NUM_KILLS:
				return p->stats.m_kill_count_ok;

			case OP_NUM_ASSISTS:
				return p->stats.m_assists;

			case OP_SHIP_SCORE: 
				return p->stats.m_score;

			case OP_SHIP_DEATHS: 
				return p->stats.m_player_deaths;

			case OP_RESPAWNS_LEFT:
				// Dogfight missions have no respawn limit
				if (MULTI_NOT_DOGFIGHT) {
					if ( Net_players[np_index].flags & NETINFO_FLAG_RESPAWNING) {						
						// since the player hasn't actually respawned yet he hasn't used up a number or spawns equal to his deaths
						// so add an extra life back. 
						return Netgame.respawn - p->stats.m_player_deaths + 1;
					}
					else {
						return Netgame.respawn - p->stats.m_player_deaths;
					}
				}
				break;

			default:
				Int3();
		}
	}	
	// AI ships also have a respawn count so we can return valid data for that at least
	else if ( (Game_mode & GM_MULTIPLAYER) && (type == OP_SHIP_DEATHS || type == OP_RESPAWNS_LEFT ) ) {
		p_objp = mission_parse_get_arrival_ship(CTEXT(node));
		if (p_objp == NULL) {
			return 0;
		}

		if (p_objp->flags & P_OF_PLAYER_START) { 
			switch (type) {				
				case OP_SHIP_DEATHS: 
					// when an AI ship is finally killed its respawn count won't be updated so get the number of deaths 
					// from the log instead
					return mission_log_get_count(LOG_SHIP_DESTROYED, CTEXT(node), NULL) + mission_log_get_count(LOG_SELF_DESTRUCTED, CTEXT(node), NULL);
					
				case OP_RESPAWNS_LEFT:
					return Netgame.respawn - p_objp->respawn_count; 

				default: 
					Int3();
			}
		}
	}

	// AI ships 
	return 0;
}

int sexp_num_type_kills(int node)
{
	int sindex, st_index;
	int idx, total;
	player *p = NULL;

	// get the ship we're interested in
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}
	
	int np_index;

	// in multiplayer, search through all players
	if(Game_mode & GM_MULTIPLAYER){
		// try and find the player
		np_index = multi_find_player_by_object(&Objects[Ships[sindex].objnum]);
		if((np_index >= 0) && (np_index < MAX_PLAYERS)){
			p = Net_players[np_index].m_player;
		}
	}
	// if we're in single player, we're only concerned with ourself
	else {
		// me
		if(Player_obj == &Objects[Ships[sindex].objnum]){
			p = Player;
		}
	}

	// bad
	if(p == NULL){
		return 0;
	}

	// lookup ship type name
	st_index = ship_type_name_lookup(CTEXT(CDR(node)));
	if(st_index < 0){
		return 0;
	}

	// look stuff up	
	total = 0;
	for(idx=0; idx<Num_ship_classes; idx++){
		if((p->stats.m_okKills[idx] > 0) && ship_class_query_general_type(idx)==st_index){
			total += p->stats.m_okKills[idx];
		}
	}

	// total
	return total;
}

int sexp_num_class_kills(int node)
{
	int sindex, si_index;
	player *p = NULL;

	// get the ship we're interested in
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}
	
	int np_index;

	// in multiplayer, search through all players
	if(Game_mode & GM_MULTIPLAYER){
		// try and find the player
		np_index = multi_find_player_by_object(&Objects[Ships[sindex].objnum]);
		if((np_index >= 0) && (np_index < MAX_PLAYERS)){
			p = Net_players[np_index].m_player;
		}
	}
	// if we're in single player, we're only concerned with ourself
	else {
		// me
		if(Player_obj == &Objects[Ships[sindex].objnum]){
			p = Player;
		}
	}

	// bad
	if(p == NULL){
		return 0;
	}

	// get the ship type we're looking for
	si_index = ship_info_lookup(CTEXT(CDR(node)));
	if((si_index < 0) || (si_index > Num_ship_classes)){
		return 0;
	}

	// return the count	
	return p->stats.m_okKills[si_index];
}

void sexp_subsys_set_random(int node)
{
	int sindex, low, high, n, idx, rand, exclusion_list[MAX_MODEL_SUBSYSTEMS];		
	ship_subsys *subsys;
	ship *shipp;

	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}
	shipp = &Ships[sindex];

	// get low
	low = eval_num(CDR(node));
	if (low < 0) {
		low = 0;
	}

	// get high
	high = eval_num(CDR(CDR(node)));
	if (high > 100) {
		high = 100;
	}

	if (low > high) {
		Int3();
		return;
	}

	n = CDR(CDR(CDR(node)));

	// init exclusion list
	memset(exclusion_list, 0, sizeof(int) * Ship_info[shipp->ship_info_index].n_subsystems);

	// get exclusion list
	while( n != -1) {
		int exclude_index = ship_get_subsys_index(shipp, CTEXT(n), 0);
		if (exclude_index >= 0) {
			exclusion_list[exclude_index] = 1;
		}

		n = CDR(n);
	}

	// apply to all others
	for (idx=0; idx<Ship_info[shipp->ship_info_index].n_subsystems; idx++) {
		if ( exclusion_list[idx] == 0 ) {
			// get non excluded subsystem
			subsys = ship_get_indexed_subsys(shipp, idx, NULL);
			if (subsys == NULL) {
				nprintf(("Warning", "Nonexistent subsystem for index %d on ship %s for sabotage subsystem\n", idx, shipp->ship_name));
				continue;
			}

			// randomize its hit points
			rand = rand_internal(low, high);
			subsys->current_hits = 0.01f * rand * subsys->max_hits;
		}
	}
}

void sexp_supernova_start(int node)
{
	supernova_start(eval_num(node));
}

void sexp_supernova_stop(int node)
{
	supernova_stop();
}

int sexp_is_secondary_selected(int node)
{
	int sindex;
	int bank;
	ship *shipp;

	// lookup ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return SEXP_FALSE;
	}
	if(Ships[sindex].objnum < 0){
		return SEXP_FALSE;
	}
	shipp = &Ships[sindex];

	// bogus value?
	bank = eval_num(CDR(node));
	if(bank >= shipp->weapons.num_secondary_banks){
		return SEXP_FALSE;
	}

	// is this the bank currently selected
	if( (bank == shipp->weapons.current_primary_bank) || ((shipp->flags & SF_PRIMARY_LINKED) && !(Weapon_info[shipp->weapons.primary_bank_weapons[bank]].wi_flags3 & WIF3_NOLINK)) ){
		return SEXP_TRUE;
	}

	// nope
	return SEXP_FALSE;
}

int sexp_is_primary_selected(int node)
{
	int sindex;
	int bank;
	ship *shipp;

	// lookup ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return SEXP_FALSE;
	}
	if(Ships[sindex].objnum < 0){
		return SEXP_FALSE;
	}
	shipp = &Ships[sindex];

	// bogus value?
	bank = eval_num(CDR(node));
	if(bank >= shipp->weapons.num_primary_banks){
		return SEXP_FALSE;
	}

	// is this the bank currently selected
	if( (bank == shipp->weapons.current_primary_bank) || (shipp->flags & SF_PRIMARY_LINKED) ){
		return SEXP_TRUE;
	}

	// nope
	return SEXP_FALSE;
}

#define	RIGHT_QUAD	0
#define	FRONT_QUAD	1
#define	LEFT_QUAD	3
#define	REAR_QUAD	2

//	Return SEXP_TRUE if quadrant quadnum is near max.
int shield_quad_near_max(int quadnum)
{
	float	remaining = 0.0f;
	for (int i=0; i<MAX_SHIELD_SECTIONS; i++) {
		if (i == quadnum){
			continue;
		}
		remaining += Player_obj->shield_quadrant[i];
	}

	if ((remaining < 2.0f) || (Player_obj->shield_quadrant[quadnum] > get_max_shield_quad(Player_obj) - 5.0f)) {
		return SEXP_TRUE;
	} else {
		return SEXP_FALSE;
	}
}

//	Return truth value for special SEXP.
//	Used in training#5, perhaps in other missions.
int process_special_sexps(int index)
{
	switch (index) {
	case 0:	//	Ship "Freighter 1" is aspect locked by player.
		if (Player_ai->target_objnum != -1) {
			if (!(stricmp(Ships[Objects[Player_ai->target_objnum].instance].ship_name, "Freighter 1"))) {
				if (Player_ai->current_target_is_locked)
					return SEXP_TRUE;
			}
		}
		return SEXP_FALSE;
		break;

	case 1:	//	Fired Interceptors
		object	*objp;
		for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (objp->type == OBJ_WEAPON) {
				if (!stricmp(Weapon_info[Weapons[objp->instance].weapon_info_index].name, "Interceptor#weak")) {
					int target = Weapons[objp->instance].target_num;
					if (target != -1) {
						if (Objects[target].type == OBJ_SHIP) {
							if (!(stricmp(Ships[Objects[target].instance].ship_name, "Freighter 1")))
								return SEXP_TRUE;
						}
					}
				}
			}
		}
		return SEXP_FALSE;

	case 2:	//	Ship "Freighter 1", subsystem "Weapons" is aspect locked by player.
		if (Player_ai->target_objnum != -1) {
			if (!(stricmp(Ships[Objects[Player_ai->target_objnum].instance].ship_name, "Freighter 1"))) {
				if (!(subsystem_stricmp(Player_ai->targeted_subsys->system_info->name, "Weapons"))) {
					if (Player_ai->current_target_is_locked){
						return SEXP_TRUE;
					}
				}
			}
		}
		return SEXP_FALSE;
		break;

	case 3:	//	Player ship suffering shield damage on front.
		apply_damage_to_shield(Player_obj, FRONT_QUAD, 10.0f);
		hud_shield_quadrant_hit(Player_obj, FRONT_QUAD);
		return SEXP_TRUE;
		break;

	case 4:	//	Player ship suffering much damage.
		nprintf(("AI", "Frame %i\n", Framecount));
		apply_damage_to_shield(Player_obj, FRONT_QUAD, 10.0f);
		hud_shield_quadrant_hit(Player_obj, FRONT_QUAD);
		if (Player_obj->shield_quadrant[FRONT_QUAD] < 2.0f)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
		break;

	case 5:	//	Player's shield is quick repaired
		nprintf(("AI", "Frame %i, recharged to %7.3f\n", Framecount, Player_obj->shield_quadrant[FRONT_QUAD]));

		apply_damage_to_shield(Player_obj, FRONT_QUAD, -flFrametime*200.0f);

		if (Player_obj->shield_quadrant[FRONT_QUAD] > get_max_shield_quad(Player_obj))
			Player_obj->shield_quadrant[FRONT_QUAD] = get_max_shield_quad(Player_obj);

		if (Player_obj->shield_quadrant[FRONT_QUAD] > Player_obj->shield_quadrant[(FRONT_QUAD+1)%MAX_SHIELD_SECTIONS] - 2.0f)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
		break;

	case 6:	//	3 of player's shield quadrants are reduced to 0.
		Player_obj->shield_quadrant[1] = 1.0f;
		Player_obj->shield_quadrant[2] = 1.0f;
		Player_obj->shield_quadrant[3] = 1.0f;
		hud_shield_quadrant_hit(Player_obj, FRONT_QUAD);
		return SEXP_TRUE;

	case 7:	//	Make sure front quadrant has been maximized, or close to it.
		if (shield_quad_near_max(FRONT_QUAD)) return SEXP_TRUE; else return SEXP_FALSE;
		break;

	case 8:	//	Make sure rear quadrant has been maximized, or close to it.
		if (shield_quad_near_max(REAR_QUAD)) return SEXP_TRUE; else return SEXP_FALSE;
		break;
	
	case 9:	//	Zero left and right quadrants in preparation for maximizing rear quadrant.
		Player_obj->shield_quadrant[LEFT_QUAD] = 0.0f;
		Player_obj->shield_quadrant[RIGHT_QUAD] = 0.0f;
		hud_shield_quadrant_hit(Player_obj, LEFT_QUAD);
		return SEXP_TRUE;
		break;

	case 10:	//	Return true if player is low on Interceptors.
		if (Player_ship->weapons.secondary_bank_ammo[0] + Player_ship->weapons.secondary_bank_ammo[1] < 8)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
		break;

	case 11:	//	Return true if player has plenty of Interceptors.
		if (Player_ship->weapons.secondary_bank_ammo[0] + Player_ship->weapons.secondary_bank_ammo[1] >= 8)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
		break;

	case 12:	//	Return true if player is low on Interceptors.
		if (Player_ship->weapons.secondary_bank_ammo[0] + Player_ship->weapons.secondary_bank_ammo[1] < 4)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
		break;

	case 13:	// Zero front shield quadrant.  Added for Jim Boone on August 26, 1999 by MK.
		Player_obj->shield_quadrant[FRONT_QUAD] = 0.0f;
		hud_shield_quadrant_hit(Player_obj, FRONT_QUAD);
		return SEXP_TRUE;
		break;

	case 100:	//	Return true if player is out of countermeasures.
		if (Player_ship->cmeasure_count <= 0)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;

	default:
		Int3();	//	Unsupported node type.
	}

	return SEXP_FALSE;
}

// Karajorma / Goober5000
int sexp_string_to_int(int n)
{
	bool first_ch = true;
	char *ch, *buf_ch, buf[TOKEN_LENGTH];
	Assert (n != -1);

	// copy all numeric characters to buf
	// also, copy a sign symbol if we haven't copied numbers yet
	buf_ch = buf;
	for (ch = CTEXT(n); *ch != 0; ch++)
	{
		if ((first_ch && (*ch == '-' || *ch == '+')) || strchr("0123456789", *ch))
		{
			*buf_ch = *ch;
			buf_ch++;

			first_ch = false;
		}

		// don't save the fractional parts of decimal numbers
		if (*ch == '.')
			break;
	}

	// terminate string
	*buf_ch = '\0';

	return atoi(buf);
}

// Goober5000
void sexp_int_to_string(int n)
{
	int i, sexp_variable_index;
	char new_text[TOKEN_LENGTH];

	// Only do single player of multi host
	if ( MULTIPLAYER_CLIENT )
		return;

	i = eval_num(n);
	n = CDR(n);

	// get sexp_variable index
	Assert(Sexp_nodes[n].first == -1);
	sexp_variable_index = atoi(Sexp_nodes[n].text);

	// verify variable set
	Assert(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_SET);

	// check variable type
	if (!(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING))
	{
		Warning(LOCATION, "Cannot assign a string to a non-string variable!");
		return;
	}

	// write string
	sprintf(new_text, "%d", i);

	// assign to variable
	sexp_modify_variable(new_text, sexp_variable_index);
}

// Goober5000
void sexp_string_concatenate(int n)
{
	int sexp_variable_index;
	char new_text[TOKEN_LENGTH * 2];

	// Only do single player of multi host
	if ( MULTIPLAYER_CLIENT )
		return;

	char *str1 = CTEXT(n);
	n = CDR(n);
	char *str2 = CTEXT(n);
	n = CDR(n);

	// get sexp_variable index
	Assert(Sexp_nodes[n].first == -1);
	sexp_variable_index = atoi(Sexp_nodes[n].text);

	// verify variable set
	Assert(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_SET);

	// check variable type
	if (!(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING))
	{
		Warning(LOCATION, "Cannot assign a string to a non-string variable!");
		return;
	}

	// concatenate strings
	strcpy_s(new_text, str1);
	strcat_s(new_text, str2);

	// check length
	if (strlen(new_text) >= TOKEN_LENGTH)
	{
		Warning(LOCATION, "Concatenated string is too long and will be truncated.");
		new_text[TOKEN_LENGTH] = 0;
	}

	// assign to variable
	sexp_modify_variable(new_text, sexp_variable_index);
}

// Goober5000
int sexp_string_get_length(int node)
{
	return strlen(CTEXT(node));
}

// Goober5000
void sexp_string_get_substring(int node)
{
	int n = node;
	int sexp_variable_index;
	char new_text[TOKEN_LENGTH];

	// Only do single player of multi host
	if ( MULTIPLAYER_CLIENT )
		return;

	char *parent = CTEXT(n);
	n = CDR(n);
	int pos = eval_num(n);
	n = CDR(n);
	int len = eval_num(n);
	n = CDR(n);

	// get sexp_variable index
	Assert(Sexp_nodes[n].first == -1);
	sexp_variable_index = atoi(Sexp_nodes[n].text);

	// verify variable set
	Assert(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_SET);

	// check variable type
	if (!(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING))
	{
		Warning(LOCATION, "Cannot assign a string to a non-string variable!");
		return;
	}

	int parent_len = strlen(parent);

	// sanity
	if (pos >= parent_len)
	{
		Warning(LOCATION, "( string-get-substring %s %d %d ) failed: starting position is larger than the string length!", parent, pos, len);
		return;
	}

	// sanity
	if (pos + len > parent_len)
		len = parent_len - pos;

	// copy substring
	memset(new_text, 0, TOKEN_LENGTH);
	strncpy(new_text, &parent[pos], len);

	// assign to variable
	sexp_modify_variable(new_text, sexp_variable_index);
}

// Goober5000
void sexp_string_set_substring(int node)
{
	int n = node;
	int sexp_variable_index;
	char new_text[TOKEN_LENGTH * 2];

	// Only do single player of multi host
	if ( MULTIPLAYER_CLIENT )
		return;

	char *parent = CTEXT(n);
	n = CDR(n);
	int pos = eval_num(n);
	n = CDR(n);
	int len = eval_num(n);
	n = CDR(n);
	char *new_substring = CTEXT(n);
	n = CDR(n);

	// get sexp_variable index
	Assert(Sexp_nodes[n].first == -1);
	sexp_variable_index = atoi(Sexp_nodes[n].text);

	// verify variable set
	Assert(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_SET);

	// check variable type
	if (!(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING))
	{
		Warning(LOCATION, "Cannot assign a string to a non-string variable!");
		return;
	}

	int parent_len = strlen(parent);
	int new_len = strlen(new_substring);

	// sanity
	if (pos >= parent_len)
	{
		Warning(LOCATION, "( string-set-substring %s %d %d %s ) failed: starting position is larger than the string length!", parent, pos, len, new_substring);
		return;
	}

	// make the common case fast
	if (len == 1 && new_len == 1)
	{
		strcpy_s(new_text, parent);
		new_text[pos] = new_substring[0];
	}
	else
	{
		// sanity
		if (pos + len > parent_len)
			len = parent_len - pos;

		// copy parent string up to the substring pos
		strncpy(new_text, parent, pos);

		// add new substring
		strcpy(&new_text[pos], new_substring);

		// add rest of parent string
		strcat_s(new_text, &parent[pos + len]);

		// check length
		if (strlen(new_text) >= TOKEN_LENGTH)
		{
			Warning(LOCATION, "Concatenated string is too long and will be truncated.");
			new_text[TOKEN_LENGTH] = 0;
		}
	}

	// assign to variable
	sexp_modify_variable(new_text, sexp_variable_index);
}

// custom sexp operator for handling misc training stuff
int sexp_special_training_check(int node)
{
	int num, rtn;

	num = eval_num(node);
	if (num == SPECIAL_CHECK_TRAINING_FAILURE)
		return Training_failure ? SEXP_TRUE : SEXP_FALSE;

	// To MK: do whatever you want with this number here.
	rtn = process_special_sexps(eval_num(node));

	return rtn;
}

// sexpression to flash a hud gauge.  gauge name is text valud of node
void sexp_flash_hud_gauge( int node )
{
	char *name;
	int i;

	name = CTEXT(node);
	for (i = 0; i < NUM_HUD_GAUGES; i++ ) {
		if ( !stricmp(HUD_gauge_text[i], name) ) {
			hud_gauge_start_flash(i);	// call HUD function to flash gauge

			multi_start_callback();
			multi_send_int(i);
			multi_end_callback();

			break;
		}
	}
}

void multi_sexp_flash_hud_gauge()
{
	int i; 

	if (multi_get_int(i)) {
		hud_gauge_start_flash(i);
	}
}

void sexp_set_training_context_fly_path(int node)
{
	waypoint_list *wp_list = find_matching_waypoint_list(CTEXT(node));
	if (wp_list == NULL)
		return;

	Training_context |= TRAINING_CONTEXT_FLY_PATH;
	Training_context_path = wp_list;
	Training_context_distance = (float) atof(CTEXT(CDR(node)));
	Training_context_goal_waypoint = 0;
	Training_context_at_waypoint = -1;
}

void sexp_set_training_context_speed(int node)
{
	Training_context |= TRAINING_CONTEXT_SPEED;
	Training_context_speed_min = eval_num(node);
	Training_context_speed_max = eval_num(CDR(node));
	Training_context_speed_set = 0;
}

bool Sexp_Messages_Scrambled = false;
void sexp_scramble_messages(bool scramble)
{
	Sexp_Messages_Scrambled = scramble;
}

void toggle_cutscene_bars(float delta_speed, int set) 
{
	//Do we want the bars?
	if (set) {
		Cutscene_bar_flags |= CUB_CUTSCENE;
	}
	else {
		Cutscene_bar_flags &= ~CUB_CUTSCENE;
	}

	if(delta_speed > 0.0f) {
		Cutscene_bars_progress = 0.0f;
		Cutscene_bar_flags |= CUB_GRADUAL;
		Cutscene_delta_time = delta_speed;
	}
	else {
		Cutscene_bar_flags &= ~CUB_GRADUAL;
	}
}

void sexp_toggle_cutscene_bars(int node, int set)
{
	float delta_speed = 0.0f;

	if(node != -1)
		delta_speed = eval_num(node)/1000.0f;

	toggle_cutscene_bars(delta_speed, set);

	multi_start_callback();
	multi_send_float(delta_speed);
	multi_end_callback();
}

void multi_sexp_toggle_cutscene_bars(int set)
{
	float delta_speed;

	if(multi_get_float(delta_speed) ) {
		toggle_cutscene_bars(delta_speed, set);
	}
}

void sexp_fade_in(int n)
{
	int duration = 0;

	if(n != -1)
		duration = eval_num(n);

	if (duration > 0) {
		Fade_start_timestamp = timestamp();
		Fade_end_timestamp = timestamp(duration);
		Fade_type = FI_FADEIN;
	} else {
		Fade_type = FI_NONE;
		gr_create_shader(&Viewer_shader, 0, 0, 0, 0);
	}

	// multiplayer callback
	multi_start_callback();
	multi_send_int(duration);
	multi_end_callback();
}

void multi_sexp_fade_in()
{
	int duration = 0;

	multi_get_int(duration);

	if (duration > 0) {
		Fade_start_timestamp = timestamp();
		Fade_end_timestamp = timestamp(duration);
		Fade_type = FI_FADEIN;
	} else {
		Fade_type = FI_NONE;
		gr_create_shader(&Viewer_shader, 0, 0, 0, 0);
	}
}

void sexp_fade_out(int duration, int fadeColor)
{
	ubyte R = 0;
	ubyte G = 0;
	ubyte B = 0;

	switch(fadeColor) {
		//White out
		case 1:
			gr_create_shader(&Viewer_shader, 255, 255, 255, Viewer_shader.c);
			break;
		//Red out
		case 2:
			gr_create_shader(&Viewer_shader, 255, 0, 0, Viewer_shader.c);
			break;
		//Black out
		default:
			gr_create_shader(&Viewer_shader, 0, 0, 0, Viewer_shader.c);
			break;
	}

	R = Viewer_shader.r;
	G = Viewer_shader.g;
	B = Viewer_shader.b;

	if (duration > 0) {
		Fade_start_timestamp = timestamp();
		Fade_end_timestamp = timestamp(duration);
		Fade_type = FI_FADEOUT;
	} else {
		Fade_type = FI_NONE;
		gr_create_shader(&Viewer_shader, R, G, B, 255);
	}
}

void sexp_fade_out(int n)
{
	int duration = 0;
	int fadeColor = 0;

	if (n != -1) {
		duration = eval_num(n);

		n = CDR(n);
		if (n != -1) {
			fadeColor = eval_num(n);
		}
	}

	sexp_fade_out(duration, fadeColor);

	multi_start_callback();
	multi_send_int(duration);
	multi_send_int(fadeColor);
	multi_end_callback();
}

void multi_sexp_fade_out()
{
	int duration = 0;
	int fadeColor = 0;

	multi_get_int(duration);
	if (!multi_get_int(fadeColor)){
		Int3();	// misformed packet
		return;
	}

	sexp_fade_out(duration, fadeColor);
}

camera* sexp_get_set_camera(bool reset = false)
{
	static camid sexp_camera;
	if(!reset)
	{
		if(Viewer_mode & VM_FREECAMERA)
		{
			camera *cam = cam_get_current().getCamera();
			if(cam != NULL)
				return cam;
		}
	}
	if(!sexp_camera.isValid())
	{
		sexp_camera = cam_create("SEXP camera");
	}

	cam_set_camera(sexp_camera);

	return sexp_camera.getCamera();
}

void sexp_set_camera(int node)
{
	if(node == -1)
	{
		sexp_get_set_camera(true);
		return;
	}

	char *cam_name = CTEXT(node);
	camid cid = cam_lookup(cam_name);
	if(!cid.isValid())
	{
		cid = cam_create(cam_name);
	}
	cam_set_camera(cid);
}

void sexp_set_camera_position(int n)
{
	camera *cam = sexp_get_set_camera();
	
	if(cam == NULL)
		return;

	vec3d camera_vec;
	float camera_time = 0.0f;
	float camera_acc_time = 0.0f;
	float camera_dec_time = 0.0f;

	camera_vec.xyz.x = i2fl(eval_num(n));
	n = CDR(n);
	camera_vec.xyz.y = i2fl(eval_num(n));
	n = CDR(n);
	camera_vec.xyz.z = i2fl(eval_num(n));
	n = CDR(n);

	if(n != -1)
	{
		camera_time = eval_num(n) / 1000.0f;
		n = CDR(n);
		if(n != -1)
		{
			camera_dec_time = camera_acc_time = eval_num(n) / 1000.0f;
			n = CDR(n);
			if(n != -1)
			{
				camera_dec_time = eval_num(n) / 1000.0f;
			}
		}
	}

	cam->set_position(&camera_vec, camera_time, camera_acc_time, camera_dec_time);

	//multiplayer callback
	multi_start_callback();
	multi_send_float(camera_vec.xyz.x);
	multi_send_float(camera_vec.xyz.y);
	multi_send_float(camera_vec.xyz.z);
	multi_send_float(camera_time);
	multi_send_float(camera_acc_time);
	multi_send_float(camera_dec_time);
	multi_end_callback();
}

//CommanderDJ
void multi_sexp_set_camera_position()
{
	camera *cam = sexp_get_set_camera();
	
	if(cam == NULL) {
		Int3();
		return;
	}

	vec3d camera_vec;
	float camera_time = 0.0f;
	float camera_acc_time = 0.0f;
	float camera_dec_time = 0.0f;

	multi_get_float(camera_vec.xyz.x);
	multi_get_float(camera_vec.xyz.y);
	multi_get_float(camera_vec.xyz.z);
	multi_get_float(camera_time);
	multi_get_float(camera_acc_time);
	multi_get_float(camera_dec_time);

	cam->set_position(&camera_vec, camera_time, camera_acc_time, camera_dec_time);
}

void sexp_set_camera_rotation(int n)
{
	camera *cam = sexp_get_set_camera();
	if(cam == NULL)
		return;

	angles rot_angles;
	float rot_time = 0.0f;
	float rot_acc_time = 0.0f;
	float rot_dec_time = 0.0f;

	//Angles are in degrees
	rot_angles.p = fl_radians(eval_num(n) % 360);
	n = CDR(n);
	rot_angles.b = fl_radians(eval_num(n) % 360);
	n = CDR(n);
	rot_angles.h = fl_radians(eval_num(n) % 360);
	n = CDR(n);
	if(n != -1)
	{
		rot_time = eval_num(n) / 1000.0f;
		n = CDR(n);
		if(n != -1)
		{
			rot_dec_time = rot_acc_time = eval_num(n) / 1000.0f;
			n = CDR(n);
			if(n != -1)
			{
				rot_dec_time = eval_num(n) / 1000.0f;
			}
		}
	}

	cam->set_rotation(&rot_angles, rot_time, rot_acc_time, rot_dec_time);

	multi_start_callback();
	multi_send_float(rot_angles.b);
	multi_send_float(rot_angles.h);
	multi_send_float(rot_angles.p);
	multi_send_float(rot_time);
	multi_send_float(rot_acc_time);
	multi_send_float(rot_dec_time);
	multi_end_callback();
}

void multi_sexp_set_camera_rotation()
{
	camera *cam = sexp_get_set_camera();
	if(cam == NULL)
		return;

	angles rot_angles;
	float rot_time = 0.0f;
	float rot_acc_time = 0.0f;
	float rot_dec_time = 0.0f;

	multi_get_float(rot_angles.b);
	multi_get_float(rot_angles.h);
	multi_get_float(rot_angles.p);
	multi_get_float(rot_time);
	multi_get_float(rot_acc_time);
	multi_get_float(rot_dec_time);

	cam->set_rotation(&rot_angles, rot_time, rot_acc_time, rot_dec_time);
}

void sexp_set_camera_facing(int n)
{
	camera *cam = sexp_get_set_camera();
	if(cam == NULL)
		return;

	vec3d location;
	float rot_time = 0.0f;
	float rot_acc_time = 0.0f;
	float rot_dec_time = 0.0f;

	location.xyz.x = i2fl(eval_num(n));
	n = CDR(n);
	location.xyz.y = i2fl(eval_num(n));
	n = CDR(n);
	location.xyz.z = i2fl(eval_num(n));
	n = CDR(n);
	if(n != -1)
	{
		rot_time = eval_num(n) / 1000.0f;
		n = CDR(n);
		if(n != -1)
		{
			rot_dec_time = rot_acc_time = eval_num(n) / 1000.0f;
			n = CDR(n);
			if(n != -1)
			{
				rot_dec_time = eval_num(n) / 1000.0f;
			}
		}
	}

	cam->set_rotation_facing(&location, rot_time, rot_acc_time, rot_dec_time);

	//multiplayer callback
	multi_start_callback();
	multi_send_float(location.xyz.x);
	multi_send_float(location.xyz.y);
	multi_send_float(location.xyz.z);
	multi_send_float(rot_time);
	multi_send_float(rot_acc_time);
	multi_send_float(rot_dec_time);
	multi_end_callback();
}

void multi_sexp_set_camera_facing()
{
	camera *cam = sexp_get_set_camera();
	if(cam == NULL)
		return;

	vec3d location;
	float rot_time = 0.0f;
	float rot_acc_time = 0.0f;
	float rot_dec_time = 0.0f;

	multi_get_float(location.xyz.x);
	multi_get_float(location.xyz.y);
	multi_get_float(location.xyz.z);
	multi_get_float(rot_time);
	multi_get_float(rot_acc_time);
	multi_get_float(rot_dec_time);
 
	cam->set_rotation_facing(&location, rot_time, rot_acc_time, rot_dec_time);
}

//CommanderDJ
/**
 * Helper function for set_camera_facing_object
 */
void actually_set_camera_facing_object(char *object_name, float rot_time, float rot_acc_time, float rot_dec_time)
{
	object_ship_wing_point_team oswpt;
	sexp_get_object_ship_wing_point_team(&oswpt, object_name);

	switch (oswpt.type)
	{
		case OSWPT_TYPE_EXITED:
		{
			Warning(LOCATION, "Camera tried to face destroyed/departed object %s", object_name);
			return;
		}

		case OSWPT_TYPE_TEAM:
		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WING:
		case OSWPT_TYPE_WAYPOINT:
		{
			camera *cam = sexp_get_set_camera();
			if(cam == NULL)
				return;
			cam->set_rotation_facing(&oswpt.objp->pos, rot_time, rot_acc_time, rot_dec_time);
			return;
		}
	}
}

void sexp_set_camera_facing_object(int n)
{
	char *object_name = CTEXT(n);
	float rot_time = 0.0f;
	float rot_acc_time = 0.0f;
	float rot_dec_time = 0.0f;

	//Now get the rotation time values
	n = CDR(n);
	if(n != -1)
	{
		rot_time = eval_num(n) / 1000.0f;
		n = CDR(n);
		if(n != -1)
		{
			rot_dec_time = rot_acc_time = eval_num(n) / 1000.0f;
			n = CDR(n);
			if(n != -1)
			{
				rot_dec_time = eval_num(n) / 1000.0f;
			}
		}
	}
	actually_set_camera_facing_object(object_name, rot_time, rot_acc_time, rot_dec_time);

	//multiplayer callback
	multi_start_callback();
	multi_send_string(object_name);
	multi_send_float(rot_time);
	multi_send_float(rot_acc_time);
	multi_send_float(rot_dec_time);
	multi_end_callback();
}

//CommanderDJ
void multi_sexp_set_camera_facing_object()
{
	char object_name[TOKEN_LENGTH];
	float rot_time = 0.0f;
	float rot_acc_time = 0.0f;
	float rot_dec_time = 0.0f;
	
	multi_get_string(object_name);
	multi_get_float(rot_time);
	multi_get_float(rot_acc_time);
	multi_get_float(rot_dec_time);
	
	actually_set_camera_facing_object(object_name, rot_time, rot_acc_time, rot_dec_time);
}

extern float VIEWER_ZOOM_DEFAULT;
void sexp_set_camera_fov(int n)
{
	camera *cam = sexp_get_set_camera();
	
	if(cam == NULL)
		return;

	float camera_time = 0.0f;
	float camera_acc_time = 0.0f;
	float camera_dec_time = 0.0f;

	float camera_fov = fl_radians(eval_num(n) % 360);
	n = CDR(n);

	if(n != -1)
	{
		camera_time = eval_num(n) / 1000.0f;
		n = CDR(n);
		if(n != -1)
		{
			camera_dec_time = camera_acc_time = eval_num(n) / 1000.0f;
			n = CDR(n);
			if(n != -1)
			{
				camera_dec_time = eval_num(n) / 1000.0f;
			}
		}
	}

	cam->set_fov(camera_fov, camera_time, camera_acc_time, camera_dec_time);


	multi_start_callback();
	multi_send_float(camera_fov);
	multi_send_float(camera_time);
	multi_send_float(camera_acc_time);
	multi_send_float(camera_dec_time);
	multi_end_callback();
}

//CommanderDJ
void multi_sexp_set_camera_fov()
{
	camera *cam = sexp_get_set_camera();
	
	if(cam == NULL)
		return;

	float camera_fov = VIEWER_ZOOM_DEFAULT;
	float camera_time = 0.0f;
	float camera_acc_time = 0.0f;
	float camera_dec_time = 0.0f;

	multi_get_float(camera_fov);
	multi_get_float(camera_time);
	multi_get_float(camera_acc_time);
	multi_get_float(camera_dec_time);

	cam->set_fov(camera_fov, camera_time, camera_acc_time, camera_dec_time);
}

//Internal helper function for set-target and set-host
object *sexp_camera_get_objsub(int node, int *o_submodel)
{
	//Get arguments
	int n = node;
	char *obj_name = NULL;
	char *sub_name = NULL;
	
	obj_name = CTEXT(n);
	n = CDR(n);
	if(n != -1)
		sub_name = CTEXT(n);
	
	//Important variables
	object *objp = NULL;
	int submodel = -1;
	
	//*****Process obj_name
	object_ship_wing_point_team oswpt;
	sexp_get_object_ship_wing_point_team(&oswpt, obj_name);
	
	switch (oswpt.type)
	{
		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WING:
		case OSWPT_TYPE_WAYPOINT:
		case OSWPT_TYPE_TEAM:
			objp = oswpt.objp;
			break;

		default:
			objp = NULL;
	}

	//*****Process submodel
	if(objp != NULL && sub_name != NULL && oswpt.type == OSWPT_TYPE_SHIP)
	{
		if(stricmp(sub_name, SEXP_NONE_STRING))
		{
			ship_subsys *ss = ship_get_subsys(&Ships[objp->instance], sub_name);
			if (ss != NULL)
			{
				submodel = ss->system_info->subobj_num;
			}
		}
	}
	
	if(o_submodel != NULL)
		*o_submodel = submodel;
	
	return objp;
}

void sexp_set_camera_host(int node)
{
	//Try to get current camera
	camera *cam = sexp_get_set_camera();
	
	if(cam == NULL)
		return;
	
	//*****Get variables
	int submodel = -1;
	object *objp = sexp_camera_get_objsub(node, &submodel);
	
	//*****Set
	cam->set_object_host(objp, submodel);
}

void sexp_set_camera_target(int node)
{
	//Try to get current camera
	camera *cam = sexp_get_set_camera();
	
	if(cam == NULL)
		return;
	
	//*****Get variables
	int submodel = -1;
	object *objp = sexp_camera_get_objsub(node, &submodel);
	
	//*****Set
	cam->set_object_target(objp, submodel);

	multi_start_callback();
	multi_send_object(objp);
	multi_send_int(submodel);
	multi_end_callback();
}

void multi_sexp_set_camera_target()
{
	int submodel;
	object *objp;
	
	//Try to get current camera
	camera *cam = sexp_get_set_camera();
	
	if(cam == NULL)
		return;

	multi_get_object(objp);
	multi_get_int(submodel);
	
	cam->set_object_target(objp, submodel);
}

void sexp_set_fov(int n)
{
	camera *cam = Main_camera.getCamera();
	if(cam == NULL) {
		game_render_frame_setup();
		cam = Main_camera.getCamera();
	}

	//Cap FOV to something reasonable.
	float new_fov = (float)(eval_num(n) % 360);
	Sexp_fov = fl_radians(new_fov);

	multi_start_callback();
	multi_send_float(new_fov);
	multi_end_callback();
}

void multi_sexp_set_fov()
{
	float new_fov;

	camera *cam = Main_camera.getCamera();
	if(cam == NULL) {
		game_render_frame_setup();
		cam = Main_camera.getCamera();
	}

	multi_get_float(new_fov);
	Sexp_fov = fl_radians(new_fov);
}

int sexp_get_fov()
{
	camera *cam = Main_camera.getCamera();
	if(cam == NULL)
		return -1;
	else if(Sexp_fov > 0.0f)
		// SEXP override has been set
		return (int) fl_degrees(Sexp_fov);
	else	
		return (int) fl_degrees(cam->get_fov());
}

/**
 * @todo Check VIEWER_ZOOM_DEFAULT
 */
void sexp_reset_fov()
{
	camera *cam = Main_camera.getCamera();
	if(cam == NULL)
		return;

	Sexp_fov = 0.0;
	//cam->set_fov(VIEWER_ZOOM_DEFAULT);

	multi_do_callback();
}

void multi_sexp_reset_fov()
{
	camera *cam = Main_camera.getCamera();
	if(cam == NULL)
		return;

	Sexp_fov = 0.0;
}

void sexp_reset_camera(int node)
{
	bool cam_reset = false; 
	camera *cam = cam_get_current().getCamera();
	if(cam != NULL)
	{
		if(is_sexp_true(node))
		{
			cam->reset();
			cam_reset = true;
		}
	}
	cam_reset_camera();
	multi_start_callback();
	multi_send_bool(cam_reset);
	multi_end_callback();
}

void multi_sexp_reset_camera()
{
	camera *cam = cam_get_current().getCamera();
	bool cam_reset = false;

	multi_get_bool(cam_reset);
	if((cam != NULL) && cam_reset) {
		cam->reset();
	}
	cam_reset_camera();
}

void sexp_show_subtitle(int node)
{
	//These should be set to the default if not required to be explicitly defined
	int x_pos, y_pos, width=0;
	char *text, *imageanim=NULL;
	float display_time, fade_time=0.0f;
	int r=255, g=255, b=255;
	bool center_x=false, center_y=false;
	int n = -1;
	bool post_shaded = false;

	x_pos = eval_num(node);
	if (gr_screen.max_w != 1024)
		x_pos = (int) ((x_pos / 1024.0f) * gr_screen.max_w);

	n = CDR(node);
	y_pos = eval_num(n);
	if (gr_screen.max_h != 768)
		y_pos = (int) ((y_pos / 768.0f) * gr_screen.max_h);

	n = CDR(n);
	text = CTEXT(n);

	n = CDR(n);
	display_time = eval_num(n)/1000.0f;	//is in ms

	n = CDR(n);
	if(n != -1)
	{
		imageanim = CTEXT(n);

		n = CDR(n);
		if(n != -1)
		{
			fade_time = eval_num(n)/1000.0f; //also in ms

			n = CDR(n);
			if(n != -1)
			{
				center_x = is_sexp_true(n) != 0;

				n = CDR(n);
				if(n != -1)
				{
					center_y = is_sexp_true(n) != 0;
						
					n = CDR(n);
					if(n != -1)
					{
						width = eval_num(n);

						n = CDR(n);
						if(n != -1)
						{
							r = eval_num(n);

							n = CDR(n);
							if(n != -1)
							{
								g = eval_num(n);

								n = CDR(n);
								if(n != -1)
								{
									b = eval_num(n);

									n = CDR(n);
									if ( n !=-1 )
									{
										post_shaded = is_sexp_true(n) != 0;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if(r > 255)
		r = 255;
	if(g > 255)
		g = 255;
	if(b > 255)
		b = 255;

	//FINALLY !!
	color new_color;
	gr_init_alphacolor(&new_color, r, g, b, 255);

	subtitle new_subtitle(x_pos, y_pos, text, imageanim, display_time, fade_time, &new_color, -1, center_x, center_y, width, 0, post_shaded);
	Subtitles.push_back(new_subtitle);
}

void sexp_clear_subtitles() 
{
	Subtitles.clear();

	multi_do_callback();
}

void multi_sexp_clear_subtitles() 
{
	Subtitles.clear();
}

void sexp_show_subtitle_text(int node)
{
	int n = node;

	char *text = CTEXT(n);
	n = CDR(n);

	int x_pct = eval_num(n);
	n = CDR(n);

	int y_pct = eval_num(n);
	n = CDR(n);

	bool center_x = is_sexp_true(n) != 0;
	n = CDR(n);

	bool center_y = is_sexp_true(n) != 0;
	n = CDR(n);

	float display_time = eval_num(n) / 1000.0f;
	n = CDR(n);

	float fade_time = 0.0f;
	if (n >= 0)
	{
		fade_time = eval_num(n) / 1000.0f;
		n = CDR(n);
	}

	int width_pct = 0;
	if (n >= 0)
	{
		width_pct = eval_num(n);
		n = CDR(n);
	}

	int red = 255;
	if (n >= 0)
	{
		red = eval_num(n);
		n = CDR(n);
	}

	int green = 255;
	if (n >= 0)
	{
		green = eval_num(n);
		n = CDR(n);
	}

	int blue = 255;
	if (n >= 0)
	{
		blue = eval_num(n);
		n = CDR(n);
	}

	int fontnum = -1;
	if (n >= 0)
	{
		char *font = CTEXT(n);
		n = CDR(n);

		// perform font lookup
		for (int i = 0; i < Num_fonts; i++)
		{
			if (!stricmp(font, Fonts[i].filename))
			{
				fontnum = i;
				break;
			}
		}
	}

	bool post_shaded = false;
	if (n >= 0)
	{
		post_shaded = is_sexp_true(n) != 0;
		n = CDR(n);
	}

	// check bounds
	if (x_pct < -100)
		x_pct = -100;
	if (x_pct > 100)
		x_pct = 100;
	if (y_pct < -100)
		y_pct = -100;
	if (y_pct > 100)
		y_pct = 100;
	if (width_pct > 100)
		width_pct = 100;
	if (red > 255)
		red = 255;
	if (green > 255)
		green = 255;
	if (blue > 255)
		blue = 255;

	color new_color;
	gr_init_alphacolor(&new_color, red, green, blue, 255);

	// calculate pixel positions
	int x_pos = (int) (gr_screen.max_w * (x_pct / 100.0f));
	int y_pos = (int) (gr_screen.max_h * (y_pct / 100.0f));
	int width = (int) (gr_screen.max_w * (width_pct / 100.0f));

	// add the subtitle
	subtitle new_subtitle(x_pos, y_pos, text, NULL, display_time, fade_time, &new_color, fontnum, center_x, center_y, width, 0, post_shaded);
	Subtitles.push_back(new_subtitle);

	multi_start_callback();
	multi_send_int(x_pos);
	multi_send_int(y_pos);
	multi_send_string(text);
	multi_send_float(display_time);
	multi_send_float(fade_time);
	multi_send_int(red);
	multi_send_int(green);
	multi_send_int(blue);
	multi_send_int(fontnum);
	multi_send_bool(center_x);
	multi_send_bool(center_y);
	multi_send_int(width);
	multi_send_bool(post_shaded);
	multi_end_callback();
}

void multi_sexp_show_subtitle_text()
{
	int x_pos, y_pos, width=0, fontnum;
	char text[TOKEN_LENGTH];
	float display_time, fade_time=0.0f;
	int red=255, green=255, blue=255;
	bool center_x=false, center_y=false;
	bool post_shaded = false;
	color new_color;

	multi_get_int(x_pos);
	multi_get_int(y_pos);
	multi_get_string(text);
	multi_get_float(display_time);
	multi_get_float(fade_time);
	multi_get_int(red);
	multi_get_int(green);
	multi_get_int(blue);
	multi_get_int(fontnum);
	multi_get_bool(center_x);
	multi_get_bool(center_y);
	multi_get_int(width);
	multi_get_bool(post_shaded);

	gr_init_alphacolor(&new_color, red, green, blue, 255);

	// add the subtitle
	subtitle new_subtitle(x_pos, y_pos, text, NULL, display_time, fade_time, &new_color, fontnum, center_x, center_y, width, 0, post_shaded);
	Subtitles.push_back(new_subtitle);	

}

void sexp_show_subtitle_image(int node)
{
	int n = node;

	char *image = CTEXT(n);
	n = CDR(n);

	int x_pct = eval_num(n);
	n = CDR(n);

	int y_pct = eval_num(n);
	n = CDR(n);

	bool center_x = is_sexp_true(n) != 0;
	n = CDR(n);

	bool center_y = is_sexp_true(n) != 0;
	n = CDR(n);

	int width_pct = eval_num(n);
	n = CDR(n);

	int height_pct = eval_num(n);
	n = CDR(n);

	float display_time = eval_num(n) / 1000.0f;
	n = CDR(n);

	float fade_time = 0.0f;
	if (n >= 0)
	{
		fade_time = eval_num(n) / 1000.0f;
		n = CDR(n);
	}

	bool post_shaded = false;
	if (n >= 0)
	{
		post_shaded = is_sexp_true(n) != 0;
		n = CDR(n);
	}

	// check bounds
	if (x_pct < -100)
		x_pct = -100;
	if (x_pct > 100)
		x_pct = 100;
	if (y_pct < -100)
		y_pct = -100;
	if (y_pct > 100)
		y_pct = 100;
	if (width_pct < 0)
		width_pct = 0;
	if (width_pct > 100)
		width_pct = 100;
	if (height_pct < 0)
		height_pct = 0;
	if (height_pct > 100)
		height_pct = 100;

	// calculate pixel positions
	int x_pos = (int)(gr_screen.max_w * (x_pct / 100.0f));
	int y_pos = (int)(gr_screen.max_h * (y_pct / 100.0f));
	int width = (int)(gr_screen.max_w * (width_pct / 100.0f));
	int height = (int)(gr_screen.max_h * (height_pct / 100.0f));

	// add the subtitle
	subtitle new_subtitle(x_pos, y_pos, NULL, image, display_time, fade_time, NULL, -1, center_x, center_y, width, height, post_shaded);
	Subtitles.push_back(new_subtitle);

	multi_start_callback();
	multi_send_int(x_pos);
	multi_send_int(y_pos);
	multi_send_string(image);
	multi_send_float(display_time);
	multi_send_float(fade_time);
	multi_send_bool(center_x);
	multi_send_bool(center_y);
	multi_send_int(width);
	multi_send_int(height);
	multi_send_bool(post_shaded);
	multi_end_callback();
}

void multi_sexp_show_subtitle_image()
{
	int x_pos, y_pos, width=0, height=0;
	char image[TOKEN_LENGTH];
	float display_time, fade_time=0.0f;
	bool center_x=false, center_y=false;
	bool post_shaded = false;

	multi_get_int(x_pos);
	multi_get_int(y_pos);
	multi_get_string(image);
	multi_get_float(display_time);
	multi_get_float(fade_time);
	multi_get_bool(center_x);
	multi_get_bool(center_y);
	multi_get_int(width);
	multi_get_int(height);
	multi_get_bool(post_shaded);

	// add the subtitle
	subtitle new_subtitle(x_pos, y_pos, NULL, image, display_time, fade_time, NULL, -1, center_x, center_y, width, height, post_shaded);
	Subtitles.push_back(new_subtitle);	

}

void sexp_set_time_compression(int n)
{
	float new_change_time = 0.0f;
	float new_multiplier = eval_num(n)/100.0f;			//percent->decimal

	//Time to change
	n = CDR(n);
	if(n != -1)
		new_change_time = eval_num(n)/1000.0f;			//ms->seconds

	//Override current time compression with this value
	n = CDR(n);
	if(n != -1)
		set_time_compression(eval_num(n)/100.0f);

	set_time_compression(new_multiplier, new_change_time);
	lock_time_compression(true);
}

void sexp_reset_time_compression()
{
	set_time_compression(1);
	lock_time_compression(false);
}

extern bool Perspective_locked;

void sexp_force_perspective(int n)
{
	Perspective_locked = (is_sexp_true(n) != 0);
	n=CDR(n);

	if(n != -1)
	{
		n = eval_num(n);
		switch(n)
		{
			case 0:
				Viewer_mode = 0;
				break;
			case 1:
				Viewer_mode = VM_CHASE;
				break;
			case 2:
				Viewer_mode = VM_EXTERNAL;
				break;
			case 3:
				Viewer_mode = VM_TOPDOWN;
				break;
		}
	}
}

void sexp_set_camera_shudder(int n)
{
	int time = eval_num(n); 
	float intensity = (float) eval_num(CDR(n)) * 0.01f; 

	game_shudder_apply(time, intensity);

	multi_start_callback();
	multi_send_int(time);
	multi_send_float(intensity); 
	multi_end_callback();
}

void multi_sexp_set_camera_shudder()
{
	int time;
	float intensity;

	multi_get_int(time);
	if (multi_get_float(intensity)) {
		game_shudder_apply(time, intensity);
	}
}

void sexp_set_jumpnode_name(int n) //CommanderDJ
{
	char *old_name = CTEXT(n); //for multi

	CJumpNode *jnp = jumpnode_get_by_name(old_name);

	if(jnp==NULL) {
		return;
	}

	n=CDR(n);
	char *new_name = CTEXT(n); //for multi
	jnp->SetName(new_name);

	//multiplayer callback
	multi_start_callback();
	multi_send_string(old_name);
	multi_send_string(new_name);
	multi_end_callback();
}

void multi_sexp_set_jumpnode_name() //CommanderDJ
{
	char old_name[TOKEN_LENGTH];
	char new_name[TOKEN_LENGTH];
	
	multi_get_string(old_name);
	multi_get_string(new_name);

	CJumpNode *jnp = jumpnode_get_by_name(old_name);

	if(jnp==NULL) 
		return;

	jnp->SetName(new_name);
}

void sexp_set_jumpnode_color(int n)
{
	CJumpNode *jnp = jumpnode_get_by_name(CTEXT(n));

	if(jnp==NULL)
		return;

	char* jumpnode_name = CTEXT(n); //for multi

	n=CDR(n);

	int red = eval_num(n);
	int green = eval_num(CDR(n));
	int blue = eval_num(CDR(CDR(n)));
	int alpha = eval_num(CDR(CDR(CDR(n))));

	jnp->SetAlphaColor(red, green, blue, alpha);

	multi_start_callback();
	multi_send_string(jumpnode_name);
	multi_send_int(red);
	multi_send_int(green);
	multi_send_int(blue);
	multi_send_int(alpha);
	multi_end_callback();
}

//CommanderDJ
void multi_sexp_set_jumpnode_color()
{
	char jumpnode_name[TOKEN_LENGTH];
	int red, blue, green, alpha;

	multi_get_string(jumpnode_name);
	CJumpNode *jnp = jumpnode_get_by_name(jumpnode_name);

	if(jnp==NULL) {
		multi_discard_remaining_callback_data();
		return;
	}

	multi_get_int(red);
	multi_get_int(green);
	multi_get_int(blue);
	multi_get_int(alpha);

	jnp->SetAlphaColor(red, green, blue, alpha);
}

void sexp_set_jumpnode_model(int n)
{
	char* jumpnode_name = CTEXT(n);
	CJumpNode *jnp = jumpnode_get_by_name(jumpnode_name);

	if(jnp==NULL)
		return;

	n=CDR(n);
	char* model_name = CTEXT(n);
	n=CDR(n);
	bool show_polys = (is_sexp_true(n) != 0);

	jnp->SetModel(model_name, show_polys);

	multi_start_callback();
	multi_send_string(jumpnode_name);
	multi_send_string(model_name);
	multi_send_bool(show_polys);
	multi_end_callback();
}

void multi_sexp_set_jumpnode_model()
{
	char jumpnode_name[TOKEN_LENGTH];
	char model_name[TOKEN_LENGTH];
	bool show_polys;

	multi_get_string(jumpnode_name);
	multi_get_string(model_name);

	CJumpNode *jnp = jumpnode_get_by_name(jumpnode_name);

	if(jnp==NULL) {
		multi_discard_remaining_callback_data();		
		return;
	}

	show_polys = multi_get_bool(show_polys);

	jnp->SetModel(model_name, show_polys);
}

void sexp_show_hide_jumpnode(int node, bool show)
{
	multi_start_callback();

	for (int n = node; n >= 0; n = CDR(n))
	{
		CJumpNode *jnp = jumpnode_get_by_name(CTEXT(n));
		if (jnp != NULL)
		{
			jnp->SetVisibility(show);
			multi_send_string(CTEXT(n));
		}
	}

	multi_end_callback();
}

void multi_sexp_show_hide_jumpnode(bool show)
{
	char jumpnode_name[TOKEN_LENGTH];

	while (multi_get_string(jumpnode_name))
	{
		CJumpNode *jnp = jumpnode_get_by_name(jumpnode_name);
		if (jnp != NULL)
			jnp->SetVisibility(show);
	}
}

//WMC - This is a bit of a hack, however, it's easier than
//coding in a whole new SCript_system function.
int sexp_script_eval(int node, int return_type)
{
	int n = node;
	char *s = CTEXT(n);
	bool success = false;

	int r = -1;

	switch(return_type)
	{
		case OPR_NUMBER:
			success = Script_system.EvalString(s, "|i", &r, s);
			break;
		case OPR_STRING:
			Error(LOCATION, "SEXP system does not support string return type; Goober must fix this before script-eval-string will work");
			break;
		case OPR_NULL:
			while(n != -1)
			{
				success = Script_system.EvalString(s, NULL, NULL, CTEXT(n));
				n = CDR(n);
			}
			break;
		default:
			Error(LOCATION, "Bad type passed to sexp_script_eval - get a coder");
			break;
	}

	if(!success)
		Warning(LOCATION, "sexp-script-eval failed to evaluate string \"%s\"; check your syntax", s);

	return r;
}

void sexp_force_glide(int node)
{
	ship *shipp;
	int sindex;

	// get ship
	sindex = ship_name_lookup(CTEXT(node));

	if (sindex < 0) {
		return;
	}

	shipp = &Ships[sindex];
	if (shipp->objnum < 0) {
		return;
	}

	//Can this ship glide?
	if (!Ship_info[shipp->ship_info_index].can_glide)
		return;

	int glide = is_sexp_true(CDR(node));

	object_set_gliding(&Objects[shipp->objnum], (glide > 0), true);

	return;
}

int sexp_is_in_box(int n)
{
	Assert(n >= 0);

	object_ship_wing_point_team oswpt;
	sexp_get_object_ship_wing_point_team(&oswpt, CTEXT(n));
	n = CDR(n);

	// Get box corners
	float x1 = (float) eval_num(n);
	n = CDR(n);
	float x2 = (float) eval_num(n);
	n = CDR(n);
	float y1 = (float) eval_num(n);
	n = CDR(n);
	float y2 = (float) eval_num(n);
	n = CDR(n);
	float z1 = (float) eval_num(n);
	n = CDR(n);
	float z2 = (float) eval_num(n);
	n = CDR(n);	
	vec3d box_corner_1;
	box_corner_1.xyz.x = MIN(x1, x2);
	box_corner_1.xyz.y = MIN(y1, y2);
	box_corner_1.xyz.z = MIN(z1, z2);
	vec3d box_corner_2;
	box_corner_2.xyz.x = MAX(x1, x2);
	box_corner_2.xyz.y = MAX(y1, y2);
	box_corner_2.xyz.z = MAX(z1, z2);

	// Ship to define reference frame is optional
	object* reference_ship_obj = NULL;
	if (n != -1)
	{
		int sindex = ship_name_lookup(CTEXT(n));

		if (sindex < 0 || Ships[sindex].objnum < 0)
			return SEXP_FALSE;

		reference_ship_obj = &Objects[Ships[sindex].objnum];
	}

	// Get position of test point
	vec3d test_point;
	switch (oswpt.type)
	{
		case OSWPT_TYPE_EXITED:
			return SEXP_KNOWN_FALSE;

		case OSWPT_TYPE_SHIP:
		case OSWPT_TYPE_WING:
		case OSWPT_TYPE_WAYPOINT:
		case OSWPT_TYPE_TEAM:
			test_point = oswpt.objp->pos;
			break;

		default:
			return SEXP_FALSE;
	}

	// If reference_ship is specified, rotate test_point into its reference frame
	if (reference_ship_obj != NULL) 
	{
		vec3d tempv;
		vm_vec_sub(&tempv, &test_point, &reference_ship_obj->pos);
		vm_vec_rotate(&test_point, &tempv, &reference_ship_obj->orient);
	}

	// Check to see if the test point is within the specified box
	if ((test_point.xyz.x >= box_corner_1.xyz.x && test_point.xyz.x <= box_corner_2.xyz.x) &&
		(test_point.xyz.y >= box_corner_1.xyz.y && test_point.xyz.y <= box_corner_2.xyz.y) &&
		(test_point.xyz.z >= box_corner_1.xyz.z && test_point.xyz.z <= box_corner_2.xyz.z))
	{
		return SEXP_TRUE;
	}
	else
	{
		return SEXP_FALSE;
	}
}

int sexp_is_in_mission(int node)
{
	for (int n = node; n != -1; n = CDR(n))
		if (ship_name_lookup(CTEXT(n)) < 0)
			return SEXP_FALSE;

	return SEXP_TRUE;
}

void sexp_manipulate_colgroup(int node, bool add_to_group) {
	object* objp;
	ship* shipp;
	int colgroup_id;

	shipp = sexp_get_ship_from_node(node);

	if (shipp == NULL)
		return;

	objp = &Objects[shipp->objnum];
	colgroup_id = objp->collision_group_id;

	node = CDR(node);

	while (node != -1) {

		int group = eval_num(node);
		
		if (group < 0 || group > 31) {
			WarningEx(LOCATION, "Invalid collision group id %d specified for object %s. Valid IDs range from 0 to 31.\n", group, shipp->ship_name); 
		} else {
			if (add_to_group) {
				colgroup_id |= (1<<group);
			} else {
				colgroup_id &= !(1<<group);
			}
		}

		node = CDR(node);
	}

	objp->collision_group_id = colgroup_id;
}

int sexp_get_colgroup(int node) {
	ship* shipp;

	shipp = sexp_get_ship_from_node(CDR(node));

	return Objects[shipp->objnum].collision_group_id;
}

int get_effect_from_name(char* name) {
	int i = 0;
	for (SCP_vector<ship_effect>::iterator sei = Ship_effects.begin(); sei != Ship_effects.end(); ++sei) {
		if (!stricmp(name, sei->name))
			return i;
		i++;
	}
	return -1;
}

void sexp_ship_effect(int n)
{
	char	*name;
	int ship_index, wing_index;
	
	Assert ( n != -1 );
	
	int effect_num = get_effect_from_name(CTEXT(n));
	if (effect_num == -1) {
		WarningEx(LOCATION, "Invalid effect name passed to ship-effect\n");
		return;
	}
	n = CDR(n);
	int effect_duration = eval_num(n);
	n = CDR(n);

	ship_index = -1;
	wing_index = -1;
	while (n != -1) {
		name = CTEXT(n);

		// check to see if this ship/wing has arrived yet.
		if (sexp_query_has_yet_to_arrive(name)) {
			n = CDR(n);
			continue;
		}

		// check to see if this ship/wing has departed.
		if ( mission_log_get_time (LOG_SHIP_DEPARTED, name, NULL, NULL) || mission_log_get_time (LOG_WING_DEPARTED, name, NULL, NULL) ) {
			n = CDR(n);
			continue;
		}

		// check to see if this ship/wing has been destroyed.
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, NULL) || mission_log_get_time(LOG_WING_DESTROYED, name, NULL, NULL) || mission_log_get_time(LOG_SELF_DESTRUCTED, name, NULL, NULL)) {
			n = CDR(n);
			continue;
		}

		ship *sp;
		if((wing_index = wing_name_lookup(name)) >= 0)
		{
			wing *wp = &Wings[wing_index];
			for(int i = 0; i < wp->current_count; i++)
			{
				if(wp->ship_index[i] >= 0)
				{
					sp = &Ships[wp->ship_index[i]];
					sp->shader_effect_active = true;
					sp->shader_effect_num = effect_num;
					sp->shader_effect_duration = effect_duration;
					sp->shader_effect_start_time = timer_get_milliseconds();
				}
			}
		}
		else
		{
			if((ship_index = ship_name_lookup(name)) >= 0)
			{
				sp = &Ships[ship_index];
				sp->shader_effect_active = true;
				sp->shader_effect_num = effect_num;
				sp->shader_effect_duration = effect_duration;
				sp->shader_effect_start_time = timer_get_milliseconds();
			}
			else
				mprintf(("Invalid Shipname in SEXP ship-effect\n"));
		}

		// move to next ship/wing in list
		n = CDR(n);
	}
}

/**
 * Returns the subsystem type if the name of a subsystem is actually a generic type (e.g <all engines> or <all turrets>
 */
int get_generic_subsys(char *subsys_name) 
{
	if (!strcmp(subsys_name, SEXP_ALL_ENGINES_STRING)) {
		return SUBSYSTEM_ENGINE;
	} else if (!strcmp(subsys_name, SEXP_ALL_TURRETS_STRING)) {
		return SUBSYSTEM_TURRET;
	}

	Assert(SUBSYSTEM_NONE == 0);
	return SUBSYSTEM_NONE;
}

// Karajorma - returns false if the ship class has changed since the mission was parsed in
// Changes can be from use of the change-ship-class SEXP, loadout or any future method
bool ship_class_unchanged(int ship_index) 
{	
	p_object *p_objp;

	ship *shipp = &Ships[ship_index];	
	p_objp = mission_parse_get_parse_object(shipp->ship_name);

	if ((p_objp != NULL) && (p_objp->ship_class == shipp->ship_info_index)) {
		return true;
	}

	return false;
}

// Goober5000 - needed because any nonzero integer value is "true"
int is_sexp_true(int cur_node, int referenced_node)
{
	int result = eval_sexp(cur_node, referenced_node);

	return ((result == SEXP_TRUE) || (result == SEXP_KNOWN_TRUE));
}

/**
 * High-level sexpression evaluator
 */
int eval_sexp(int cur_node, int referenced_node)
{
	int node, type, sexp_val = UNINITIALIZED;
	if (cur_node == -1)  // empty list, i.e. sexp: ( )
		return SEXP_FALSE;

	Assert(cur_node >= 0);			// we have special sexp nodes <= -1!!!  MWA
									// which should be intercepted before we get here.  HOFFOSS
	type = SEXP_NODE_TYPE(cur_node);
	Assert( (type == SEXP_LIST) || (type == SEXP_ATOM) );

	// trap known true and known false sexpressions.  We don't trap on SEXP_NAN sexpressions since
	// they may yet evaluate to true or false.

	if (Sexp_nodes[cur_node].value == SEXP_KNOWN_TRUE)
		return SEXP_TRUE;
	else if (Sexp_nodes[cur_node].value == SEXP_KNOWN_FALSE)
		return SEXP_FALSE;

	if (Sexp_nodes[cur_node].first != -1) {
		node = CAR(cur_node);
		sexp_val = eval_sexp(node);
		Sexp_nodes[cur_node].value = Sexp_nodes[node].value;	// higher level node gets node value
		return sexp_val;

	} else {
		int op_num;

		node = CDR(cur_node);		// makes reading the next bit of code a little easier.

		op_num = get_operator_const(cur_node);
		// add the op_num to the stack if it is an actual operator rather than a number
		if (op_num) {
			Current_sexp_operator.push_back(op_num); 
		}
		switch ( op_num ) {
		// arithmetic operators will always return just their value
			case OP_PLUS:
				sexp_val = add_sexps(node);
				break;

			case OP_MINUS:
				sexp_val = sub_sexps(node);
				break;

			case OP_MUL:
				sexp_val = mul_sexps(node);
				break;

			case OP_MOD:
				sexp_val = mod_sexps(node);
				break;

			case OP_DIV:
				sexp_val = div_sexps(node);
				break;

			case OP_RAND:
			case OP_RAND_MULTIPLE:
				sexp_val = rand_sexp( node, (op_num == OP_RAND_MULTIPLE) );
				break;

			case OP_ABS:
				sexp_val = abs_sexp(node);
				break;

			case OP_MIN:
				sexp_val = min_sexp(node);
				break;

			case OP_MAX:
				sexp_val = max_sexp(node);
				break;

			case OP_AVG:
				sexp_val = avg_sexp(node);
				break;

			case OP_POW:
				sexp_val = pow_sexp(node);
				break;

			case OP_SIGNUM:
				sexp_val = signum_sexp(node);
				break;

			case OP_SET_BIT:
			case OP_UNSET_BIT:
				sexp_val = sexp_set_bit(node, op_num == OP_SET_BIT);
				break;

			case OP_IS_BIT_SET:
				sexp_val = sexp_is_bit_set(node);
				break;

			case OP_BITWISE_AND:
				sexp_val = sexp_bitwise_and(node);
				break;

			case OP_BITWISE_OR:
				sexp_val = sexp_bitwise_or(node);
				break;

			case OP_BITWISE_NOT:
				sexp_val = sexp_bitwise_not(node);
				break;

			case OP_BITWISE_XOR:
				sexp_val = sexp_bitwise_xor(node);
				break;

			// boolean operators can have one of the special sexp values (known true, known false, unknown)
			case OP_TRUE:
				sexp_val = SEXP_KNOWN_TRUE;
				break;

			case OP_FALSE:
				sexp_val = SEXP_KNOWN_FALSE;
				break;

			case OP_OR:
				sexp_val = sexp_or(node);
				break;

			case OP_AND:
				sexp_val = sexp_and(node);
				break;

			case OP_AND_IN_SEQUENCE:
				sexp_val = sexp_and_in_sequence(node);
				break;

			case OP_XOR:
				sexp_val = sexp_xor(node);
				break;

			case OP_EQUALS:
			case OP_GREATER_THAN:
			case OP_LESS_THAN:
			case OP_NOT_EQUAL:
			case OP_GREATER_OR_EQUAL:
			case OP_LESS_OR_EQUAL:
				sexp_val = sexp_number_compare( node, op_num );
				break;

			case OP_STRING_GREATER_THAN:
			case OP_STRING_LESS_THAN:
			case OP_STRING_EQUALS:
				sexp_val = sexp_string_compare( node, op_num );
				break;

			case OP_IS_IFF:
				sexp_val = sexp_is_iff(node);
				break;

			case OP_NOT:
				sexp_val = sexp_not(node);
				break;

			case OP_PREVIOUS_GOAL_TRUE:
				sexp_val = sexp_previous_goal_status( node, GOAL_COMPLETE );
				break;

			case OP_PREVIOUS_GOAL_FALSE:
				sexp_val = sexp_previous_goal_status( node, GOAL_FAILED );
				break;

			case OP_PREVIOUS_GOAL_INCOMPLETE:
				sexp_val = sexp_previous_goal_status( node, GOAL_INCOMPLETE );
				break;

			case OP_PREVIOUS_EVENT_TRUE:
				sexp_val = sexp_previous_event_status( node, EVENT_SATISFIED );
				break;

			case OP_PREVIOUS_EVENT_FALSE:
				sexp_val = sexp_previous_event_status( node, EVENT_FAILED );
				break;

			case OP_PREVIOUS_EVENT_INCOMPLETE:
				sexp_val = sexp_previous_event_status( node, EVENT_INCOMPLETE );
				break;

			case OP_EVENT_TRUE:
			case OP_EVENT_FALSE:
				sexp_val = sexp_event_status( node, (op_num == OP_EVENT_TRUE?1:0) );
				if ((sexp_val != SEXP_TRUE) && (sexp_val != SEXP_KNOWN_TRUE))
					Sexp_useful_number = 0;  // indicate sexp isn't current yet
				break;

			case OP_EVENT_TRUE_DELAY:
			case OP_EVENT_FALSE_DELAY:
				sexp_val = sexp_event_delay_status( node, (op_num == OP_EVENT_TRUE_DELAY?1:0) );
				break;

			case OP_EVENT_TRUE_MSECS_DELAY:
			case OP_EVENT_FALSE_MSECS_DELAY:
				sexp_val = sexp_event_delay_status( node, (op_num == OP_EVENT_TRUE_MSECS_DELAY?1:0), true );
				break;

			case OP_GOAL_TRUE_DELAY:
			case OP_GOAL_FALSE_DELAY:
				sexp_val = sexp_goal_delay_status( node, (op_num == OP_GOAL_TRUE_DELAY?1:0) );
				break;

			case OP_EVENT_INCOMPLETE:
				sexp_val = sexp_event_incomplete(node);
				if ((sexp_val != SEXP_TRUE) && (sexp_val != SEXP_KNOWN_TRUE))
					Sexp_useful_number = 0;  // indicate sexp isn't current yet
				break;

			case OP_GOAL_INCOMPLETE:
				sexp_val = sexp_goal_incomplete(node);
				break;

			// destroy type sexpressions
			case OP_IS_DESTROYED:
				sexp_val = sexp_is_destroyed( node, NULL );
				break;

			case OP_IS_SUBSYSTEM_DESTROYED:
				sexp_val = sexp_is_subsystem_destroyed(node);
				break;

			case OP_HAS_DOCKED:
				sexp_val = sexp_has_docked(node);
				break;

			case OP_HAS_ARRIVED:
				sexp_val = sexp_has_arrived( node, NULL );
				break;

			case OP_HAS_DEPARTED:
				sexp_val = sexp_has_departed( node, NULL );
				break;

			case OP_HAS_UNDOCKED:
				sexp_val = sexp_has_undocked(node);
				break;

			case OP_IS_DISABLED:
				sexp_val = sexp_is_disabled( node, NULL );
				break;

			case OP_IS_DISARMED:
				sexp_val = sexp_is_disarmed( node, NULL );
				break;

			case OP_WAYPOINTS_DONE:
				sexp_val = sexp_are_waypoints_done(node);
				break;

			// objective operators that use a delay
			case OP_IS_DESTROYED_DELAY:
				sexp_val = sexp_is_destroyed_delay(node);
				break;

			case OP_IS_SUBSYSTEM_DESTROYED_DELAY:
				sexp_val = sexp_is_subsystem_destroyed_delay(node);
				break;

			case OP_HAS_DOCKED_DELAY:
				sexp_val = sexp_has_docked_delay(node);
				break;

			case OP_HAS_ARRIVED_DELAY:
				sexp_val = sexp_has_arrived_delay(node);
				break;

			case OP_HAS_DEPARTED_DELAY:
				sexp_val = sexp_has_departed_delay(node);
				break;

			case OP_HAS_UNDOCKED_DELAY:
				sexp_val = sexp_has_undocked_delay(node);
				break;

			case OP_IS_DISABLED_DELAY:
				sexp_val = sexp_is_disabled_delay(node);
				break;

			case OP_IS_DISARMED_DELAY:
				sexp_val = sexp_is_disarmed_delay(node);
				break;

			case OP_WAYPOINTS_DONE_DELAY:
				sexp_val = sexp_are_waypoints_done_delay(node);
				break;

			case OP_SHIP_TYPE_DESTROYED:
				sexp_val = sexp_ship_type_destroyed(node);
				break;

			// time based sexpressions
			case OP_HAS_TIME_ELAPSED:
				sexp_val = sexp_has_time_elapsed(node);
				break;

			case OP_MODIFY_VARIABLE:
				sexp_modify_variable(node);
				sexp_val = SEXP_TRUE;	// SEXP_TRUE means only do once.
				break;

			case OP_GET_VARIABLE_BY_INDEX:
				sexp_val = sexp_get_variable_by_index(node);
				break;

			case OP_SET_VARIABLE_BY_INDEX:
				sexp_set_variable_by_index(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_TIME_SHIP_DESTROYED:
				sexp_val = sexp_time_destroyed(node);
				break;
			
			case OP_TIME_WING_DESTROYED:
				sexp_val = sexp_time_wing_destroyed(node);
				break;
			
			case OP_TIME_SHIP_ARRIVED:
				sexp_val = sexp_time_ship_arrived(node);
				break;
			
			case OP_TIME_WING_ARRIVED:
				sexp_val = sexp_time_wing_arrived(node);
				break;
			
			case OP_TIME_SHIP_DEPARTED:
				sexp_val = sexp_time_ship_departed(node);
				break;
			
			case OP_TIME_WING_DEPARTED:
				sexp_val = sexp_time_wing_departed(node);
				break;

			case OP_MISSION_TIME:
				sexp_val = sexp_mission_time();
				break;

			case OP_MISSION_TIME_MSECS:
				sexp_val = sexp_mission_time_msecs();
				break;

			case OP_TIME_DOCKED:
				sexp_val = sexp_time_docked(node);
				break;

			case OP_TIME_UNDOCKED:
				sexp_val = sexp_time_undocked(node);
				break;

			case OP_AFTERBURNER_LEFT:
				sexp_val = sexp_get_energy_pct(node, op_num);
				break;

			case OP_WEAPON_ENERGY_LEFT:
				sexp_val = sexp_get_energy_pct(node, op_num);
				break;

			case OP_SHIELDS_LEFT:
				sexp_val = sexp_shields_left(node);
				break;

			case OP_HITS_LEFT:
				sexp_val = sexp_hits_left(node);
				break;

			case OP_HITS_LEFT_SUBSYSTEM:
				sexp_val = sexp_hits_left_subsystem(node);
				break;

			case OP_HITS_LEFT_SUBSYSTEM_GENERIC:
				sexp_val = sexp_hits_left_subsystem_generic(node);
				break;

			case OP_HITS_LEFT_SUBSYSTEM_SPECIFIC:
				sexp_val = sexp_hits_left_subsystem_specific(node);
				break;

			case OP_SIM_HITS_LEFT:
				sexp_val = sexp_sim_hits_left(node);
				break;

			case OP_SPECIAL_WARP_DISTANCE:
				sexp_val = sexp_special_warp_dist(node);
				break;

			case OP_DISTANCE:
				sexp_val = sexp_distance(node);
				break;

			case OP_DISTANCE_SUBSYSTEM:
				sexp_val = sexp_distance_subsystem(node);
				break;

			case OP_NUM_WITHIN_BOX:
				sexp_val = sexp_num_within_box(node);
				break;

			case OP_IS_IN_BOX:
				sexp_val = sexp_is_in_box(node);
				break;

			case OP_IS_IN_MISSION:
				sexp_val = sexp_is_in_mission(node);
				break;

			case OP_IS_SHIP_VISIBLE:
				sexp_val = sexp_is_ship_visible(node);
				break;

			case OP_IS_SHIP_STEALTHY:
				sexp_val = sexp_is_ship_stealthy(node);
				break;

			case OP_IS_FRIENDLY_STEALTH_VISIBLE:
				sexp_val = sexp_is_friendly_stealth_visible(node);
				break;

			case OP_TEAM_SCORE:
				sexp_val = sexp_team_score(node);
				break;

			case OP_LAST_ORDER_TIME:
				sexp_val = sexp_last_order_time(node);
				break;

			case OP_NUM_PLAYERS:
				sexp_val = sexp_num_players();
				break;

			case OP_SKILL_LEVEL_AT_LEAST:
				sexp_val = sexp_skill_level_at_least(node);
				break;

			case OP_IS_CARGO_KNOWN:
			case OP_CARGO_KNOWN_DELAY:
				sexp_val = sexp_is_cargo_known( node, (op_num==OP_IS_CARGO_KNOWN)?0:1 );
				break;

			case OP_HAS_BEEN_TAGGED_DELAY:
				sexp_val = sexp_has_been_tagged_delay(node);
				break;

			case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
				sexp_val = sexp_cap_subsys_cargo_known_delay(node);
				break;

			case OP_WAS_PROMOTION_GRANTED:
				sexp_val = sexp_was_promotion_granted(node);
				break;

			case OP_WAS_MEDAL_GRANTED:
				sexp_val = sexp_was_medal_granted(node);
				break;

			case OP_GET_DAMAGE_CAUSED:
				sexp_val = sexp_get_damage_caused(node);
				break;

			case OP_PERCENT_SHIPS_ARRIVED:
			case OP_PERCENT_SHIPS_DEPARTED:
			case OP_PERCENT_SHIPS_DESTROYED:
			case OP_PERCENT_SHIPS_DISARMED:
			case OP_PERCENT_SHIPS_DISABLED:
				sexp_val = sexp_percent_ships_arrive_depart_destroy_disarm_disable(node, op_num);
				break;

			case OP_DEPART_NODE_DELAY:
				sexp_val = sexp_depart_node_delay(node);
				break;

			case OP_DESTROYED_DEPARTED_DELAY:
				sexp_val = sexp_destroyed_departed_delay(node);
				break;

			// conditional sexpressions
			case OP_WHEN:
			case OP_WHEN_ARGUMENT:
			case OP_IF_THEN_ELSE:
			case OP_PERFORM_ACTIONS:
				sexp_val = eval_when( node, op_num );
				break;

			case OP_COND:
				sexp_val = eval_cond(node);
				break;

			// Goober5000: special case; evaluate like when, but flush the sexp tree
			// and return SEXP_NAN so this will always be re-evaluated
			case OP_EVERY_TIME:
			case OP_EVERY_TIME_ARGUMENT:
				eval_when( node, op_num );
				flush_sexp_tree(node);
				sexp_val = SEXP_NAN;
				break;

			// Goober5000
			case OP_ANY_OF:
				sexp_val = eval_any_of( cur_node, referenced_node );
				break;

			// Goober5000
			case OP_EVERY_OF:
				sexp_val = eval_every_of( cur_node, referenced_node );
				break;

			// Goober5000 and Karajorma
			case OP_RANDOM_OF:
			case OP_RANDOM_MULTIPLE_OF:
				sexp_val = eval_random_of( cur_node, referenced_node, (op_num == OP_RANDOM_MULTIPLE_OF) );
				break;

			// Goober5000
			case OP_NUMBER_OF:
				sexp_val = eval_number_of( cur_node, referenced_node );
				break;

			// Karajorma
			case OP_IN_SEQUENCE:
				sexp_val = eval_in_sequence( cur_node, referenced_node );
				break;

			// Goober5000
			case OP_FOR_COUNTER:
				sexp_val = eval_for_counter( cur_node, referenced_node );
				break;

			// Karajorma
			case OP_INVALIDATE_ALL_ARGUMENTS:
			case OP_VALIDATE_ALL_ARGUMENTS:
				sexp_change_all_argument_validity(cur_node, (op_num == OP_INVALIDATE_ALL_ARGUMENTS));
				sexp_val = SEXP_TRUE;
			break;

			// Goober5000
			case OP_INVALIDATE_ARGUMENT:
			case OP_VALIDATE_ARGUMENT:
				sexp_change_argument_validity(node, (op_num == OP_INVALIDATE_ARGUMENT));
				sexp_val = SEXP_TRUE;
				break;

			case OP_DO_FOR_VALID_ARGUMENTS:
				// do-for-valid-arguments should only ever be called within eval_when()
				Int3();
				break;

			case OP_NUM_VALID_ARGUMENTS:
				sexp_val = sexp_num_valid_arguments( cur_node );
				break;

			// sexpressions with side effects
			case OP_CHANGE_IFF:
				sexp_change_iff(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_ADD_SHIP_GOAL:
				sexp_add_ship_goal(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_ADD_WING_GOAL:
				sexp_add_wing_goal(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_ADD_GOAL:
				sexp_add_goal(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_REMOVE_GOAL:
				sexp_remove_goal(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_CLEAR_SHIP_GOALS:
				sexp_clear_ship_goals(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_CLEAR_WING_GOALS:
				sexp_clear_wing_goals(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_CLEAR_GOALS:
				sexp_clear_goals(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_PROTECT_SHIP:
			case OP_UNPROTECT_SHIP:
				sexp_protect_ships(node, (op_num == OP_PROTECT_SHIP));
				sexp_val = SEXP_TRUE;
				break;

			case OP_BEAM_PROTECT_SHIP:
			case OP_BEAM_UNPROTECT_SHIP:
				sexp_beam_protect_ships(node, (op_num == OP_BEAM_PROTECT_SHIP));
				sexp_val = SEXP_TRUE;
				break;

			case OP_TURRET_PROTECT_SHIP:
			case OP_TURRET_UNPROTECT_SHIP:
				sexp_turret_protect_ships(node, (op_num == OP_TURRET_PROTECT_SHIP));
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_STEALTHY:
			case OP_SHIP_UNSTEALTHY:
				sexp_ships_stealthy(node, (op_num == OP_SHIP_STEALTHY));
				sexp_val = SEXP_TRUE;
				break;

			case OP_FRIENDLY_STEALTH_INVISIBLE:
			case OP_FRIENDLY_STEALTH_VISIBLE:
				sexp_friendly_stealth_invisible(node, (op_num == OP_FRIENDLY_STEALTH_INVISIBLE));
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_INVISIBLE:
			case OP_SHIP_VISIBLE:
				sexp_ships_visible(node, (op_num == OP_SHIP_VISIBLE));
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_SHIP_TAG:
			case OP_SHIP_UNTAG:
				sexp_ship_tag(node, (op_num == OP_SHIP_TAG));
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_CHANGE_ALT_NAME:
				sexp_ship_change_alt_name(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_CHANGE_CALLSIGN:
				sexp_ship_change_callsign(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_DEATH_MESSAGE:
				sexp_set_death_message(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_VULNERABLE:
			case OP_SHIP_INVULNERABLE:
				sexp_ships_invulnerable(node, (op_num == OP_SHIP_INVULNERABLE));
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_BOMB_TARGETABLE:
			case OP_SHIP_BOMB_UNTARGETABLE:
				sexp_ships_bomb_targetable(node, (op_num == OP_SHIP_BOMB_TARGETABLE));
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_GUARDIAN:
			case OP_SHIP_NO_GUARDIAN:
				sexp_ships_guardian(node, (op_num == OP_SHIP_GUARDIAN));
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_GUARDIAN_THRESHOLD:
				sexp_ship_guardian_threshold(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_SUBSYS_GUARDIAN_THRESHOLD:
				sexp_ship_subsys_guardian_threshold(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_SUBSYS_TARGETABLE:
				sexp_ship_deal_with_subsystem_flag(node, SSF_UNTARGETABLE, true, false);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_SUBSYS_UNTARGETABLE:
				sexp_ship_deal_with_subsystem_flag(node, SSF_UNTARGETABLE, true, true);
				sexp_val = SEXP_TRUE;
				break;

			case OP_TURRET_SUBSYS_TARGET_DISABLE:
				sexp_val = SEXP_TRUE;
				sexp_ship_deal_with_subsystem_flag(node, SSF_NO_SS_TARGETING, false, true);
				break;

			case OP_TURRET_SUBSYS_TARGET_ENABLE:
				sexp_val = SEXP_TRUE;
				sexp_ship_deal_with_subsystem_flag(node, SSF_NO_SS_TARGETING, false, false);
				break;

			case OP_SHIP_SUBSYS_NO_REPLACE:
				sexp_ship_deal_with_subsystem_flag(node, SSF_NO_REPLACE, true);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_SUBSYS_NO_LIVE_DEBRIS:
				sexp_ship_deal_with_subsystem_flag(node, SSF_NO_LIVE_DEBRIS, true);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_SUBSYS_VANISHED:
				sexp_ship_deal_with_subsystem_flag(node, SSF_VANISHED, true);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_SUBSYS_IGNORE_IF_DEAD:
				sexp_ship_deal_with_subsystem_flag(node, SSF_MISSILES_IGNORE_IF_DEAD, false);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_CREATE:
				sexp_ship_create(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_WEAPON_CREATE:
				sexp_weapon_create(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_VANISH:
				sexp_ship_vanish(node);
				sexp_val = SEXP_TRUE;
				break;

			//-Sesquipedalian
			case OP_SHIELDS_ON: 
			case OP_SHIELDS_OFF:
				sexp_shields_off(node, (op_num == OP_SHIELDS_OFF));
				sexp_val = SEXP_TRUE;
				break;

			//-Sesquipedalian
			case OP_KAMIKAZE: 
				sexp_kamikaze(node, (op_num == OP_KAMIKAZE));
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_CARGO:
				sexp_set_cargo(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_IS_CARGO:
				sexp_val = sexp_is_cargo(node);
				break;

			case OP_CHANGE_AI_CLASS:
				sexp_change_ai_class(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_IS_AI_CLASS:
				sexp_val = sexp_is_ai_class(node);
				break;
				
			case OP_IS_SHIP_TYPE:
				sexp_val = sexp_is_ship_type(node);
				break;

			case OP_IS_SHIP_CLASS:
				sexp_val = sexp_is_ship_class(node);
				break;

			case OP_CHANGE_SOUNDTRACK:
				sexp_change_soundtrack(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_PLAY_SOUND_FROM_TABLE:
				sexp_play_sound_from_table(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_PLAY_SOUND_FROM_FILE:
				sexp_play_sound_from_file(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_CLOSE_SOUND_FROM_FILE:
				sexp_close_sound_from_file(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_SOUND_ENVIRONMENT:
				sexp_set_sound_environment(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_UPDATE_SOUND_ENVIRONMENT:
				sexp_update_sound_environment(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_ADJUST_AUDIO_VOLUME:
				sexp_adjust_audio_volume(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_HUD_DISABLE:
				sexp_hud_disable(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_HUD_DISABLE_EXCEPT_MESSAGES:
				sexp_hud_disable_except_messages(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_HUD_SET_TEXT:
				sexp_hud_set_text(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_HUD_SET_TEXT_NUM:
				sexp_hud_set_text_num(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_HUD_SET_COORDS:
				sexp_hud_set_coords(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_HUD_SET_FRAME:
				sexp_hud_set_frame(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_HUD_SET_COLOR:
				sexp_hud_set_color(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_HUD_SET_MAX_TARGETING_RANGE:
				sexp_hud_set_max_targeting_range(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_HUD_DISPLAY_GAUGE:
				sexp_hud_display_gauge(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_HUD_SET_MESSAGE:
				sexp_hud_set_message(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_PLAYER_USE_AI:
			case OP_PLAYER_NOT_USE_AI:
				sexp_player_use_ai(op_num == OP_PLAYER_USE_AI);
				sexp_val = SEXP_TRUE;
				break;

			//Karajorma
			case OP_ALLOW_TREASON:
				sexp_allow_treason(node);
				sexp_val = SEXP_TRUE;
				break; 

			//Karajorma
			case OP_SET_PLAYER_ORDERS:
				sexp_set_player_orders(node);
				sexp_val = SEXP_TRUE;
				break; 

			// Goober5000
			case OP_EXPLOSION_EFFECT:
				sexp_explosion_effect(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_WARP_EFFECT:
				sexp_warp_effect(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SEND_MESSAGE:
				sexp_send_message(node);
				sexp_val = SEXP_TRUE;
				break;

			// Karajorma
			case OP_ENABLE_BUILTIN_MESSAGES:
			case OP_DISABLE_BUILTIN_MESSAGES:
				sexp_toggle_builtin_messages (node, op_num == OP_ENABLE_BUILTIN_MESSAGES);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_PERSONA:
				sexp_set_persona (node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SEND_MESSAGE_LIST:
				sexp_send_message_list(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SEND_RANDOM_MESSAGE:
				sexp_send_random_message(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SELF_DESTRUCT:
				sexp_self_destruct(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_NEXT_MISSION:
				sexp_next_mission(node);
				sexp_val = SEXP_TRUE;
				break;
				
			case OP_END_OF_CAMPAIGN:
				sexp_end_of_campaign(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_END_CAMPAIGN:
				sexp_end_campaign(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SABOTAGE_SUBSYSTEM:
				sexp_sabotage_subsystem(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_REPAIR_SUBSYSTEM:
				sexp_repair_subsystem(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_SUBSYSTEM_STRNGTH:
				sexp_set_subsystem_strength(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_INVALIDATE_GOAL:
			case OP_VALIDATE_GOAL:
				sexp_change_goal_validity( node, (op_num==OP_INVALIDATE_GOAL?0:1) );
				sexp_val = SEXP_TRUE;
				break;

			case OP_TRANSFER_CARGO:
				sexp_transfer_cargo(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_EXCHANGE_CARGO:
				sexp_exchange_cargo(node);
				sexp_val = SEXP_TRUE;
				break;


			case OP_JETTISON_CARGO:
				sexp_jettison_cargo(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_DOCKED:
				sexp_set_docked(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_CARGO_NO_DEPLETE:
				sexp_cargo_no_deplete(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_SCANNED:	// Goober5000
			case OP_SET_UNSCANNED:
				sexp_set_scanned_unscanned(node, op_num == OP_SET_SCANNED);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_SPECIAL_WARPOUT_NAME:
				sexp_special_warpout_name(node);
				sexp_val = SEXP_TRUE;
				break;

			//-WMC
			case OP_MISSION_SET_NEBULA:
				sexp_mission_set_nebula(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_MISSION_SET_SUBSPACE:
				sexp_mission_set_subspace(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_ADD_BACKGROUND_BITMAP:
				sexp_add_background_bitmap(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_REMOVE_BACKGROUND_BITMAP:
				sexp_remove_background_bitmap(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_ADD_SUN_BITMAP:
				sexp_add_sun_bitmap(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_REMOVE_SUN_BITMAP:
				sexp_remove_sun_bitmap(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_NEBULA_CHANGE_STORM:
				sexp_nebula_change_storm(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_NEBULA_TOGGLE_POOF:
				sexp_nebula_toggle_poof(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_END_MISSION:
				sexp_end_mission(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_SET_DEBRIEFING_TOGGLED:
				sexp_set_debriefing_toggled(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_FORCE_JUMP:
				sexp_force_jump();
				sexp_val = SEXP_TRUE;
				break;

				// sexpressions for setting flag for good/bad time for someone to reasm
			case OP_GOOD_REARM_TIME:
				sexp_good_time_to_rearm(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_GRANT_PROMOTION:
				sexp_grant_promotion();
				sexp_val = SEXP_TRUE;
				break;

			case OP_GRANT_MEDAL:
				sexp_grant_medal(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_VAPORIZE:
			case OP_SHIP_NO_VAPORIZE:
				sexp_ships_vaporize( node, (op_num == OP_SHIP_VAPORIZE) );
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_EXPLOSION_OPTION:
				sexp_set_explosion_option(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_DONT_COLLIDE_INVISIBLE:
			case OP_COLLIDE_INVISIBLE:
				sexp_dont_collide_invisible( node, (op_num == OP_DONT_COLLIDE_INVISIBLE) );
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_MOBILE:
			case OP_SET_IMMOBILE:
				sexp_set_immobile(node, (op_num == OP_SET_IMMOBILE));
				sexp_val = SEXP_TRUE;
				break;

			case OP_IGNORE_KEY:
				sexp_ignore_key(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000 - sigh, was this messed up all along?
			case OP_WARP_BROKEN:
			case OP_WARP_NOT_BROKEN:
			case OP_WARP_NEVER:
			case OP_WARP_ALLOWED:
				sexp_deal_with_warp( node, (op_num==OP_WARP_BROKEN) || (op_num==OP_WARP_NOT_BROKEN),
					(op_num==OP_WARP_BROKEN) || (op_num==OP_WARP_NEVER) );
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_SET_SUBSPACE_DRIVE:
				sexp_set_subspace_drive(node);
				break;

			case OP_GOOD_SECONDARY_TIME:
				sexp_good_secondary_time(node);
				sexp_val = SEXP_TRUE;
				break;

			// sexpressions to allow shpis/weapons during the course of a mission
			case OP_ALLOW_SHIP:
				sexp_allow_ship(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_ALLOW_WEAPON:
				sexp_allow_weapon(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_TECH_ADD_SHIP:
				sexp_tech_add_ship(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_TECH_ADD_WEAPON:
				sexp_tech_add_weapon(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_TECH_ADD_INTEL:
				sexp_tech_add_intel(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_TECH_RESET_TO_DEFAULT:
				sexp_tech_reset_to_default();
				sexp_val = SEXP_TRUE;
				break;

			case OP_CHANGE_PLAYER_SCORE:
				sexp_change_player_score(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_CHANGE_TEAM_SCORE:
				sexp_change_team_score(node);
				sexp_val = SEXP_TRUE;
				break;

				// in the case of a red_alert mission, simply call the red alert function to close
				// the current campaign's mission and move forward to the next mission
			case OP_RED_ALERT:
				red_alert_start_mission();
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_OBJECT_SPEED_X:
			case OP_SET_OBJECT_SPEED_Y:
			case OP_SET_OBJECT_SPEED_Z:
				sexp_set_object_speed(node, op_num - OP_SET_OBJECT_SPEED_X);
				sexp_val = SEXP_TRUE;
				break;

			case OP_GET_OBJECT_SPEED_X:
			case OP_GET_OBJECT_SPEED_Y:
			case OP_GET_OBJECT_SPEED_Z:
				sexp_val = sexp_get_object_speed(node, op_num - OP_GET_OBJECT_SPEED_X);
				break;

			case OP_GET_OBJECT_X:
			case OP_GET_OBJECT_Y:
			case OP_GET_OBJECT_Z:
				sexp_val = sexp_get_object_coordinate(node, op_num - OP_GET_OBJECT_X);
				break;

			case OP_GET_OBJECT_PITCH:
			case OP_GET_OBJECT_BANK:
			case OP_GET_OBJECT_HEADING:
				sexp_val = sexp_get_object_angle(node, op_num - OP_GET_OBJECT_PITCH);
				break;

			case OP_SET_OBJECT_POSITION:
				sexp_set_object_position(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_OBJECT_ORIENTATION:
				sexp_set_object_orientation(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_OBJECT_FACING:
			case OP_SET_OBJECT_FACING_OBJECT:
				sexp_set_object_facing(node, op_num == OP_SET_OBJECT_FACING_OBJECT);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SHIP_MANEUVER:
			case OP_SHIP_ROT_MANEUVER:
			case OP_SHIP_LAT_MANEUVER:
				sexp_set_ship_maneuver(node, op_num);
				sexp_val = SEXP_TRUE;
				break;

			// training operators
			case OP_KEY_PRESSED:
				sexp_val = sexp_key_pressed(node);
				break;

			case OP_SPECIAL_CHECK:
				sexp_val = sexp_special_training_check(node);
				break;

			case OP_KEY_RESET:
				sexp_key_reset(node);
				sexp_val = SEXP_KNOWN_TRUE;  // only do it first time in repeating events.
				break;

			case OP_KEY_RESET_MULTIPLE:
				sexp_key_reset(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_MISSILE_LOCKED:
				sexp_val = sexp_missile_locked(node);
				break;

			case OP_TARGETED:
				sexp_val = sexp_targeted(node);
				break;

			case OP_NODE_TARGETED:
				sexp_val = sexp_node_targeted(node);
				break;

			case OP_SPEED:
				sexp_val = sexp_speed(node);
				break;

			case OP_GET_THROTTLE_SPEED:
				sexp_val = sexp_get_throttle_speed(node);
				break;

			case OP_SET_PLAYER_THROTTLE_SPEED:
				sexp_set_player_throttle_speed(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_PRIMARIES_DEPLETED:
				sexp_val = sexp_primaries_depleted(node);
				break;

			case OP_SECONDARIES_DEPLETED:
				sexp_val = sexp_secondaries_depleted(node);
				break;

			case OP_FACING:
				sexp_val = sexp_facing(node);
				break;

			case OP_IS_FACING:
				sexp_val = sexp_is_facing(node);
				break;

			case OP_FACING2:
				sexp_val = sexp_facing2(node);
				break;

			case OP_ORDER:
				sexp_val = sexp_order(node);
				break;

			case OP_QUERY_ORDERS:
				sexp_val = sexp_query_orders(node);
				break;

			// Karajorma
			case OP_RESET_ORDERS:
				sexp_reset_orders(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_WAYPOINT_MISSED:
				sexp_val = sexp_waypoint_missed();
				break;

			case OP_WAYPOINT_TWICE:
				sexp_val = sexp_waypoint_twice();
				break;

			case OP_PATH_FLOWN:
				sexp_val = sexp_path_flown();
				break;

			case OP_TRAINING_MSG:
				sexp_send_training_message(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_FLASH_HUD_GAUGE:
				sexp_flash_hud_gauge(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_TRAINING_CONTEXT_FLY_PATH:
				sexp_set_training_context_fly_path(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_TRAINING_CONTEXT_SPEED:
				sexp_set_training_context_speed(node);
				sexp_val = SEXP_TRUE;
				break;
			
			// Karajorma
			case OP_STRING_TO_INT:
				sexp_val = sexp_string_to_int(node);
				break;

			// Goober5000
			case OP_INT_TO_STRING:
				sexp_int_to_string(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_STRING_CONCATENATE:
				sexp_string_concatenate(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_STRING_GET_SUBSTRING:
				sexp_string_get_substring(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_STRING_SET_SUBSTRING:
				sexp_string_set_substring(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_STRING_GET_LENGTH:
				sexp_val = sexp_string_get_length(node);
				break;

			case 0: // zero represents a non-operator
				return eval_num(cur_node);

			case OP_NOP:
				sexp_val = SEXP_TRUE;
				break;

			case OP_BEAM_FIRE:
			case OP_BEAM_FIRE_COORDS:
				sexp_beam_fire(node, op_num == OP_BEAM_FIRE_COORDS);
				sexp_val = SEXP_TRUE;
				break;

			case OP_IS_TAGGED:
				sexp_val = sexp_is_tagged(node);
				break;

			case OP_IS_PLAYER:
				sexp_val = sexp_is_player(node);
				break;

			case OP_NUM_KILLS:
			case OP_NUM_ASSISTS:
			case OP_SHIP_SCORE: 
			case OP_SHIP_DEATHS: 
			case OP_RESPAWNS_LEFT:
				sexp_val = sexp_return_player_data(node, op_num);
				break;

			case OP_SET_RESPAWNS:
				sexp_set_respawns(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_REMOVE_WEAPONS:
				sexp_remove_weapons(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_NUM_TYPE_KILLS:
				sexp_val = sexp_num_type_kills(node);
				break;

			case OP_NUM_CLASS_KILLS:
				sexp_val = sexp_num_class_kills(node);
				break;

			case OP_BEAM_FREE:
				sexp_val = SEXP_TRUE;
				sexp_beam_free(node);
				break;

			case OP_BEAM_FREE_ALL:
				sexp_val = SEXP_TRUE;
				sexp_beam_free_all(node);
				break;

			case OP_BEAM_LOCK:
				sexp_val = SEXP_TRUE;
				sexp_beam_lock(node);
				break;

			case OP_BEAM_LOCK_ALL:
				sexp_val = SEXP_TRUE;
				sexp_beam_lock_all(node);
				break;

			case OP_TURRET_FREE:
				sexp_val = SEXP_TRUE;
				sexp_turret_free(node);
				break;

			case OP_TURRET_FREE_ALL:
				sexp_val = SEXP_TRUE;
				sexp_turret_free_all(node);
				break;

			case OP_TURRET_LOCK:
				sexp_val = SEXP_TRUE;
				sexp_turret_lock(node);
				break;

			case OP_TURRET_LOCK_ALL:
				sexp_val = SEXP_TRUE;
				sexp_turret_lock_all(node);
				break;

			case OP_TURRET_CHANGE_WEAPON:
				sexp_val = SEXP_TRUE;
				sexp_turret_change_weapon(node);
				break;

			case OP_TURRET_SET_DIRECTION_PREFERENCE:
				sexp_val = SEXP_TRUE;
				sexp_turret_set_direction_preference(node);
				break;

			case OP_TURRET_SET_RATE_OF_FIRE:
				sexp_val = SEXP_TRUE;
				sexp_turret_set_rate_of_fire(node);
				break;

			case OP_TURRET_SET_OPTIMUM_RANGE:
				sexp_val = SEXP_TRUE;
				sexp_turret_set_optimum_range(node);
				break;

			case OP_TURRET_SET_TARGET_PRIORITIES:
				sexp_val = SEXP_TRUE;
				sexp_turret_set_target_priorities(node);
				break;

			case OP_TURRET_SET_TARGET_ORDER:
				sexp_val = SEXP_TRUE;
				sexp_turret_set_target_order(node);
				break;

			case OP_SET_ARMOR_TYPE:
				sexp_val = SEXP_TRUE;
				sexp_set_armor_type(node);
				break;

			case OP_WEAPON_SET_DAMAGE_TYPE:
				sexp_val = SEXP_TRUE;
				sexp_weapon_set_damage_type(node);
				break;

			case OP_SHIP_SET_DAMAGE_TYPE:
				sexp_val = SEXP_TRUE;
				sexp_ship_set_damage_type(node);
				break;

			case OP_SHIP_SHOCKWAVE_SET_DAMAGE_TYPE:
				sexp_val = SEXP_TRUE;
				sexp_ship_shockwave_set_damage_type(node);
				break;

			case OP_FIELD_SET_DAMAGE_TYPE:
				sexp_val = SEXP_TRUE;
				sexp_field_set_damage_type(node);
				break;

			case OP_SHIP_TURRET_TARGET_ORDER:
				sexp_val = SEXP_TRUE;
				sexp_ship_turret_target_order(node);
				break;

			case OP_ADD_REMOVE_ESCORT:
				sexp_val = SEXP_TRUE;
				sexp_add_remove_escort(node);
				break;
			
			case OP_DAMAGED_ESCORT_LIST:
				sexp_val = SEXP_TRUE;
				sexp_damage_escort_list(node);
				break;

			case OP_DAMAGED_ESCORT_LIST_ALL:
				sexp_val = SEXP_TRUE;
				sexp_damage_escort_list_all(node);
				break;

			case OP_AWACS_SET_RADIUS:
				sexp_val = SEXP_TRUE;
				sexp_awacs_set_radius(node);
				break;

			case OP_PRIMITIVE_SENSORS_SET_RANGE:
				sexp_primitive_sensors_set_range(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_CAP_WAYPOINT_SPEED:
				sexp_val = SEXP_TRUE;
				sexp_cap_waypoint_speed(node);
				break;

			case OP_TURRET_TAGGED_ONLY_ALL:
				sexp_val = SEXP_TRUE;
				sexp_turret_tagged_only_all(node);
				break;

			case OP_TURRET_TAGGED_CLEAR_ALL:
				sexp_val = SEXP_TRUE;
				sexp_turret_tagged_clear_all(node);
				break;

			case OP_SUBSYS_SET_RANDOM:
				sexp_val = SEXP_TRUE;
				sexp_subsys_set_random(node);
				break;

			case OP_SUPERNOVA_START:
				sexp_val = SEXP_TRUE;
				sexp_supernova_start(node);
				break;

			case OP_SUPERNOVA_STOP:
				sexp_val = SEXP_TRUE;
				sexp_supernova_stop(node);
				break;

			case OP_SHIELD_RECHARGE_PCT:
				sexp_val = sexp_shield_recharge_pct(node);
				break;

			case OP_ENGINE_RECHARGE_PCT:
				sexp_val = sexp_engine_recharge_pct(node);
				break;

			case OP_WEAPON_RECHARGE_PCT:
				sexp_val = sexp_weapon_recharge_pct(node);
				break;

			case OP_SHIELD_QUAD_LOW:
				sexp_val = sexp_shield_quad_low(node);
				break;

			case OP_PRIMARY_AMMO_PCT:
				sexp_val = sexp_primary_ammo_pct(node);
				break;

			case OP_SECONDARY_AMMO_PCT:
				sexp_val = sexp_secondary_ammo_pct(node);
				break;

			// Karajorma
			case OP_GET_PRIMARY_AMMO:
				sexp_val = sexp_get_primary_ammo(node);
				break;

			// Karajorma
			case OP_GET_SECONDARY_AMMO:
				sexp_val = sexp_get_secondary_ammo(node);
				break;

			// Karajorma
			case OP_GET_NUM_COUNTERMEASURES:
				sexp_val = sexp_get_countermeasures(node);
				break;

			case OP_IS_SECONDARY_SELECTED:
				sexp_val = sexp_is_secondary_selected(node);
				break;

			case OP_IS_PRIMARY_SELECTED:
				sexp_val = sexp_is_primary_selected(node);
				break;

			// Goober5000
			case OP_SET_SUPPORT_SHIP:
				sexp_set_support_ship(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_SET_ARRIVAL_INFO:
				sexp_set_arrival_info(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_SET_DEPARTURE_INFO:
				sexp_set_departure_info(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_CHANGE_SHIP_CLASS:
				sexp_change_ship_class(node);
				sexp_val = SEXP_TRUE;
				break;

			// Goober5000
			case OP_SHIP_COPY_DAMAGE:
				sexp_ship_copy_damage(node);
				sexp_val = SEXP_TRUE;
				break;

			//-Bobboau
			case OP_ACTIVATE_GLOW_POINTS:
			case OP_DEACTIVATE_GLOW_POINTS:
				sexp_activate_deactivate_glow_points(node, op_num == OP_ACTIVATE_GLOW_POINTS);
				sexp_val = SEXP_TRUE;
				break;

			//-Bobboau
			case OP_ACTIVATE_GLOW_MAPS:
			case OP_DEACTIVATE_GLOW_MAPS:
				sexp_activate_deactivate_glow_maps(node, op_num == OP_ACTIVATE_GLOW_MAPS);
				sexp_val = SEXP_TRUE;
				break;

			//-Bobboau
			case OP_ACTIVATE_GLOW_POINT_BANK:
			case OP_DEACTIVATE_GLOW_POINT_BANK:
				sexp_activate_deactivate_glow_point_bank(node, op_num == OP_ACTIVATE_GLOW_POINT_BANK);
				sexp_val = SEXP_TRUE;
				break;

			// taylor
			case OP_SET_SKYBOX_MODEL:
				sexp_set_skybox_model(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_SKYBOX_ORIENT:
				sexp_set_skybox_orientation(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_TURRET_TAGGED_SPECIFIC:
				sexp_turret_tagged_specific(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_TURRET_TAGGED_CLEAR_SPECIFIC:
				sexp_turret_tagged_clear_specific(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_LOCK_ROTATING_SUBSYSTEM:
			case OP_FREE_ROTATING_SUBSYSTEM:
				sexp_set_subsys_rotation_lock_free(node, op_num == OP_LOCK_ROTATING_SUBSYSTEM);
				sexp_val = SEXP_TRUE;
				break;

			case OP_REVERSE_ROTATING_SUBSYSTEM:
				sexp_reverse_rotating_subsystem(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_ROTATING_SUBSYS_SET_TURN_TIME:
				sexp_rotating_subsys_set_turn_time(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_TRIGGER_SUBMODEL_ANIMATION:
				sexp_trigger_submodel_animation(node);
				sexp_val = SEXP_TRUE;
				break;
				
			// Karajorma
			case OP_SET_PRIMARY_AMMO:
				sexp_set_primary_ammo(node);
				sexp_val = SEXP_TRUE;
				break;
				
			// Karajorma
			case OP_SET_SECONDARY_AMMO:
				sexp_set_secondary_ammo(node);
				sexp_val = SEXP_TRUE;
				break;
				
			// Karajorma
			case OP_SET_PRIMARY_WEAPON:
			case OP_SET_SECONDARY_WEAPON:
				sexp_set_weapon(node, op_num == OP_SET_PRIMARY_WEAPON);
				sexp_val = SEXP_TRUE;
				break;

			// Karajorma
			case OP_SET_NUM_COUNTERMEASURES:
				sexp_set_countermeasures(node);
				sexp_val = SEXP_TRUE;
				break;

			// Karajorma
			case OP_LOCK_PRIMARY_WEAPON:
			case OP_UNLOCK_PRIMARY_WEAPON:
				sexp_deal_with_primary_lock(node, op_num == OP_LOCK_PRIMARY_WEAPON);
				sexp_val = SEXP_TRUE;
				break;

			case OP_LOCK_SECONDARY_WEAPON:
			case OP_UNLOCK_SECONDARY_WEAPON:
				sexp_deal_with_secondary_lock(node, op_num == OP_LOCK_SECONDARY_WEAPON);
				sexp_val = SEXP_TRUE;
				break;

			// KeldorKatarn
			case OP_LOCK_AFTERBURNER:
			case OP_UNLOCK_AFTERBURNER:
				sexp_deal_with_afterburner_lock(node, op_num == OP_LOCK_AFTERBURNER);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_AFTERBURNER_ENERGY: 
			case OP_SET_WEAPON_ENERGY:
			case OP_SET_SHIELD_ENERGY:
				sexp_set_energy_pct(node, op_num);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_AMBIENT_LIGHT: 
				sexp_set_ambient_light(node); 
				sexp_val = SEXP_TRUE;
				break;

			case OP_SET_POST_EFFECT:
				sexp_set_post_effect(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_PRIMARY_FIRED_SINCE:
			case OP_SECONDARY_FIRED_SINCE:
				sexp_val = sexp_weapon_fired_delay(node, op_num); 
				break;

			case OP_HAS_PRIMARY_WEAPON:
			case OP_HAS_SECONDARY_WEAPON:
				sexp_val = sexp_has_weapon(node, op_num);
				break;

			case OP_DIRECTIVE_VALUE:
				sexp_val = sexp_directive_value(node);
				break;

			case OP_CHANGE_SUBSYSTEM_NAME:
				sexp_change_subsystem_name(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_NUM_SHIPS_IN_BATTLE:	// phreak
				sexp_val=sexp_num_ships_in_battle(node);
				break;

			// Karajorma
			case OP_NUM_SHIPS_IN_WING:
				sexp_val=sexp_num_ships_in_wing(node);
				break;

			case OP_CURRENT_SPEED:
				sexp_val = sexp_current_speed(node);
				break;

			case OP_NAV_IS_VISITED: //kazan
				sexp_val = is_nav_visited(node);
				break;

			case OP_NAV_DISTANCE: //kazan
				sexp_val = distance_to_nav(node);
				break;

			case OP_NAV_ADD_WAYPOINT: //kazan
				sexp_val = SEXP_TRUE;
				add_nav_waypoint(node);
				break;

			case OP_NAV_ADD_SHIP: //kazan
				sexp_val = SEXP_TRUE;
				add_nav_ship(node);
				break;

			case OP_NAV_DEL: //kazan
				sexp_val = SEXP_TRUE;
				del_nav(node);
				break;

			case OP_NAV_HIDE: //kazan
				sexp_val = SEXP_TRUE;
				hide_nav(node);
				break;

			case OP_NAV_RESTRICT: //kazan
				sexp_val = SEXP_TRUE;
				restrict_nav(node);
				break;

			case OP_NAV_UNHIDE: //kazan
				sexp_val = SEXP_TRUE;
				unhide_nav(node);
				break;

			case OP_NAV_UNRESTRICT: //kazan
				sexp_val = SEXP_TRUE;
				unrestrict_nav(node);
				break;

			case OP_NAV_SET_VISITED: //kazan
				sexp_val = SEXP_TRUE;
				set_nav_visited(node);
				break;

			case OP_NAV_UNSET_VISITED: //kazan
				sexp_val = SEXP_TRUE;
				unset_nav_visited(node);
				break;

			case OP_NAV_SET_CARRY: //kazan
				sexp_val = SEXP_TRUE;
				set_nav_carry_status(node);
				break;

			case OP_NAV_UNSET_CARRY: //kazan
				sexp_val = SEXP_TRUE;
				unset_nav_carry_status(node);
				break;

			case OP_NAV_SET_NEEDSLINK:
				sexp_val = SEXP_TRUE;
				set_nav_needslink(node);
				break;

			case OP_NAV_UNSET_NEEDSLINK:
				sexp_val = SEXP_TRUE;
				unset_nav_needslink(node);
				break;

			case OP_NAV_ISLINKED:
				sexp_val = is_nav_linked(node);
				break;
			
			case OP_NAV_USEAP:
				sexp_val = SEXP_TRUE;
				set_use_ap(node);
				break;

			case OP_NAV_USECINEMATICS:
				sexp_val = SEXP_TRUE;
				set_use_ap_cinematics(node);
				break;

			case OP_SCRAMBLE_MESSAGES:
			case OP_UNSCRAMBLE_MESSAGES:
				sexp_scramble_messages(op_num == OP_SCRAMBLE_MESSAGES );
				sexp_val = SEXP_TRUE;
				break;

			case OP_CUTSCENES_SET_CUTSCENE_BARS:
			case OP_CUTSCENES_UNSET_CUTSCENE_BARS:
				sexp_val = SEXP_TRUE;
				sexp_toggle_cutscene_bars(node, op_num == OP_CUTSCENES_SET_CUTSCENE_BARS);
				break;

			case OP_CUTSCENES_FADE_IN:
				sexp_val = SEXP_TRUE;
				sexp_fade_in(node);
				break;
			case OP_CUTSCENES_FADE_OUT:
				sexp_val = SEXP_TRUE;
				sexp_fade_out(node);
				break;
			case OP_CUTSCENES_SET_CAMERA:
				sexp_val = SEXP_TRUE;
				sexp_set_camera(node);
				break;
			case OP_CUTSCENES_SET_CAMERA_FACING:
				sexp_val = SEXP_TRUE;
				sexp_set_camera_facing(node);
				break;
			case OP_CUTSCENES_SET_CAMERA_FACING_OBJECT:
				sexp_val = SEXP_TRUE;
				sexp_set_camera_facing_object(node);
				break;
			case OP_CUTSCENES_SET_CAMERA_FOV:
				sexp_val = SEXP_TRUE;
				sexp_set_camera_fov(node);
				break;
			case OP_CUTSCENES_SET_CAMERA_HOST:
				sexp_val = SEXP_TRUE;
				sexp_set_camera_host(node);
				break;
			case OP_CUTSCENES_SET_CAMERA_POSITION:
				sexp_val = SEXP_TRUE;
				sexp_set_camera_position(node);
				break;
			case OP_CUTSCENES_SET_CAMERA_ROTATION:
				sexp_val = SEXP_TRUE;
				sexp_set_camera_rotation(node);
				break;
			case OP_CUTSCENES_SET_CAMERA_TARGET:
				sexp_val = SEXP_TRUE;
				sexp_set_camera_target(node);
				break;
			case OP_CUTSCENES_SET_FOV:
				sexp_val = SEXP_TRUE;
				sexp_set_fov(node);
				break;
			case OP_CUTSCENES_GET_FOV:	
				sexp_val = sexp_get_fov();
				break;
			case OP_CUTSCENES_RESET_FOV:
				sexp_val = SEXP_TRUE;
				sexp_reset_fov();
				break;
			case OP_CUTSCENES_RESET_CAMERA:
				sexp_val = SEXP_TRUE;
				sexp_reset_camera(node);
				break;
			case OP_CUTSCENES_SHOW_SUBTITLE:
				sexp_val = SEXP_TRUE;
				sexp_show_subtitle(node);
				break;
			case OP_CUTSCENES_SHOW_SUBTITLE_TEXT:
				sexp_val = SEXP_TRUE;
				sexp_show_subtitle_text(node);
				break;
			case OP_CUTSCENES_SHOW_SUBTITLE_IMAGE:
				sexp_val = SEXP_TRUE;
				sexp_show_subtitle_image(node);
				break;
			case OP_CUTSCENES_SET_TIME_COMPRESSION:
				sexp_val = SEXP_TRUE;
				sexp_set_time_compression(node);
				break;
			case OP_CUTSCENES_RESET_TIME_COMPRESSION:
				sexp_val = SEXP_TRUE;
				sexp_reset_time_compression();
				break;
			case OP_CUTSCENES_FORCE_PERSPECTIVE:
				sexp_val = SEXP_TRUE;
				sexp_force_perspective(node);
				break;

			case OP_SET_CAMERA_SHUDDER:
				sexp_val = SEXP_TRUE;
				sexp_set_camera_shudder(node);
				break;

			case OP_JUMP_NODE_SET_JUMPNODE_NAME: //CommanderDJ
				sexp_val = SEXP_TRUE;
				sexp_set_jumpnode_name(node);
				break;

			case OP_JUMP_NODE_SET_JUMPNODE_COLOR:
				sexp_val = SEXP_TRUE;
				sexp_set_jumpnode_color(node);
				break;
			case OP_JUMP_NODE_SET_JUMPNODE_MODEL:
				sexp_val = SEXP_TRUE;
				sexp_set_jumpnode_model(node);
				break;
			case OP_JUMP_NODE_SHOW_JUMPNODE:
			case OP_JUMP_NODE_HIDE_JUMPNODE:
				sexp_show_hide_jumpnode(node, op_num == OP_JUMP_NODE_SHOW_JUMPNODE);
				sexp_val = SEXP_TRUE;
				break;

			case OP_SCRIPT_EVAL_NUM:
				sexp_val = sexp_script_eval(node, OPR_NUMBER);
				break;

			case OP_SCRIPT_EVAL_STRING:
				sexp_val = sexp_script_eval(node, OPR_STRING);
				break;

			case OP_SCRIPT_EVAL:
				sexp_val = sexp_script_eval(node, OPR_NULL);
				break;

			case OP_CHANGE_IFF_COLOR:
				sexp_change_iff_color(node);
				sexp_val = SEXP_TRUE;
				break;

			case OP_DISABLE_ETS:
			case OP_ENABLE_ETS:
				sexp_disable_ets(node, (op_num == OP_DISABLE_ETS));
				sexp_val = SEXP_TRUE;
				break;

			case OP_FORCE_GLIDE:
				sexp_val = SEXP_TRUE;
				sexp_force_glide(node);
				break;

			case OP_HUD_SET_DIRECTIVE:
				sexp_val = SEXP_TRUE;
				sexp_hud_set_directive(node);
				break;

			case OP_HUD_GAUGE_SET_ACTIVE:
				sexp_val = SEXP_TRUE;
				sexp_hud_gauge_set_active(node);
				break;

			case OP_HUD_CLEAR_MESSAGES:
				sexp_val = SEXP_TRUE;
				sexp_hud_clear_messages();
				break;

			case OP_HUD_ACTIVATE_GAUGE_TYPE:
				sexp_val = SEXP_TRUE;
				sexp_hud_activate_gauge_type(node);
				break;

			case OP_ADD_TO_COLGROUP:
				sexp_val = SEXP_TRUE;
				sexp_manipulate_colgroup(node, true);
				break;

			case OP_REMOVE_FROM_COLGROUP:
				sexp_val = SEXP_TRUE;
				sexp_manipulate_colgroup(node, false);
				break;

			case OP_GET_COLGROUP_ID:
				sexp_val = sexp_get_colgroup(node);
				break;

			case OP_SHIP_EFFECT:
				sexp_val = SEXP_TRUE;
				sexp_ship_effect(node);
				break;

			case OP_CLEAR_SUBTITLES:
				sexp_val = SEXP_TRUE;
				sexp_clear_subtitles();
				break;

			case OP_SET_THRUSTERS:
				sexp_val = SEXP_TRUE;
				sexp_set_thrusters(node);
				break;

			default:
				Error(LOCATION, "Looking for SEXP operator, found '%s'.\n", CTEXT(cur_node));
				break;
		}
		Assert(!Current_sexp_operator.empty()); 
		Current_sexp_operator.pop_back();

		Assert(sexp_val != UNINITIALIZED);		

		// if we haven't returned, check the sexp value of the sexpression evaluation.  A special
		// value of known true or known false means that we should set the sexp.value field for
		// short circuit eval (and return that special value as well).
		if (sexp_val == SEXP_KNOWN_TRUE) {
			Sexp_nodes[cur_node].value = SEXP_KNOWN_TRUE;
			return SEXP_TRUE;
		}

		if (sexp_val == SEXP_KNOWN_FALSE) {
			Sexp_nodes[cur_node].value = SEXP_KNOWN_FALSE;
			return SEXP_FALSE;
		}

		if ( sexp_val == SEXP_NAN ) {
			Sexp_nodes[cur_node].value = SEXP_NAN;			// not a number values are false I would suspect
			return SEXP_FALSE;
		}

		if ( sexp_val == SEXP_NAN_FOREVER ) {
			Sexp_nodes[cur_node].value = SEXP_NAN_FOREVER;
			return SEXP_FALSE;	// Goober5000 changed from sexp_val to SEXP_FALSE on 2/21/2006 in accordance with above comment
		}

		if ( sexp_val == SEXP_CANT_EVAL ) {
			Sexp_nodes[cur_node].value = SEXP_CANT_EVAL;
			Sexp_useful_number = 0;  // indicate sexp isn't current yet
			return SEXP_FALSE;
		}

		if ( Sexp_nodes[cur_node].value == SEXP_NAN ) {	// if we had a nan, but now don't, reset the value
			Sexp_nodes[cur_node].value = SEXP_UNKNOWN;
			return sexp_val;
		}

		// now, reconcile positive and negative - Goober5000
		if (sexp_val < 0)
		{
			int parent_node = find_parent_operator(cur_node);
			int arg_num = find_argnum(parent_node, cur_node);

			// make sure everything works okay
			if (arg_num == -1)
			{
				SCP_string sexp_text;
				convert_sexp_to_string(sexp_text, cur_node, SEXP_ERROR_CHECK_MODE);
				Error(LOCATION, "Error finding sexp argument.  Received value %d for sexp:\n%s", sexp_val, sexp_text.c_str());
			}

			// if we need a positive value, make it positive
			if (query_operator_argument_type(get_operator_index(parent_node), arg_num) == OPF_POSITIVE)
			{
				sexp_val *= -1;
			}
		}

		if ( sexp_val ){
			Sexp_nodes[cur_node].value = SEXP_TRUE;
		} else {
			Sexp_nodes[cur_node].value = SEXP_FALSE;
		}

		return sexp_val;
	}
}

/**
 * Only runs on the client machines not the server. Evaluates the contents of a SEXP packet and calls the relevent multi_sexp_x 
 * function(s). 
 */
void multi_sexp_eval()
{
	int op_num; 

	Assert (MULTIPLAYER_CLIENT); 

	while (Multi_sexp_bytes_left > 0) {
		op_num = multi_sexp_get_next_operator(); 

		Assert (Multi_sexp_bytes_left); 

		if (op_num < 0) {
			Warning(LOCATION, "Received invalid operator number from host in multi_sexp_eval(). Entire packet may be corrupt. Discarding packet"); 
			Int3(); 
			return; 	
		}

		switch(op_num) {

			case OP_CHANGE_SOUNDTRACK:
				multi_sexp_change_soundtrack();
				break; 

			case OP_SET_PERSONA:
				multi_sexp_set_persona();
				break; 
			
			case OP_CHANGE_SUBSYSTEM_NAME:
				multi_sexp_change_subsystem_name();
				break;

			case OP_SHIP_SUBSYS_NO_REPLACE:
				multi_sexp_deal_with_subsys_flag(SSF_NO_REPLACE);
				break;
			case OP_SHIP_SUBSYS_NO_LIVE_DEBRIS:
				multi_sexp_deal_with_subsys_flag(SSF_NO_LIVE_DEBRIS);
				break;
			case OP_SHIP_SUBSYS_VANISHED:
				multi_sexp_deal_with_subsys_flag(SSF_VANISHED);
				break;
			case OP_SHIP_SUBSYS_IGNORE_IF_DEAD:
				multi_sexp_deal_with_subsys_flag(SSF_MISSILES_IGNORE_IF_DEAD);
				break;
			case OP_SHIP_SUBSYS_TARGETABLE:
			case OP_SHIP_SUBSYS_UNTARGETABLE:
				multi_sexp_deal_with_subsys_flag(SSF_UNTARGETABLE);
				break;

			case OP_SHIP_CHANGE_CALLSIGN:
				multi_sexp_ship_change_callsign();
				break;

			case OP_SET_RESPAWNS:
				multi_sexp_set_respawns();
				break;

			case OP_REMOVE_WEAPONS:
				multi_sexp_remove_weapons();
				break;

			case OP_CHANGE_SHIP_CLASS:
				multi_sexp_change_ship_class();
				break;
					
			case OP_PLAY_SOUND_FROM_TABLE:
				multi_sexp_play_sound_from_table();
				break;
					
			case OP_PLAY_SOUND_FROM_FILE:
				multi_sexp_play_sound_from_file();
				break;

			case OP_CLOSE_SOUND_FROM_FILE:
				multi_sexp_close_sound_from_file();
				break;

			case OP_SHIP_BOMB_TARGETABLE:
			case OP_SHIP_BOMB_UNTARGETABLE:
			case OP_SHIP_INVISIBLE:
			case OP_SHIP_VISIBLE:
			case OP_SHIP_STEALTHY:
			case OP_SHIP_UNSTEALTHY:
			case OP_FRIENDLY_STEALTH_INVISIBLE:
			case OP_FRIENDLY_STEALTH_VISIBLE:
			case OP_LOCK_AFTERBURNER:
			case OP_UNLOCK_AFTERBURNER:
			case OP_LOCK_PRIMARY_WEAPON:
			case OP_UNLOCK_PRIMARY_WEAPON:
			case OP_LOCK_SECONDARY_WEAPON:
			case OP_UNLOCK_SECONDARY_WEAPON:
				multi_sexp_deal_with_ship_flag();
				break;

			case OP_SET_AFTERBURNER_ENERGY: 
				multi_sexp_set_energy_pct();
				break;

			case OP_SET_AMBIENT_LIGHT:
				multi_sexp_set_ambient_light();
				break;

			case OP_MODIFY_VARIABLE:
				multi_sexp_modify_variable();
				break;

			case OP_NAV_ADD_WAYPOINT:
				multi_sexp_add_nav_waypoint();
				break;

			case OP_CUTSCENES_FADE_IN:
				multi_sexp_fade_in();
				break;

			case OP_CUTSCENES_FADE_OUT:
				multi_sexp_fade_out();
				break;

			case OP_NAV_ADD_SHIP:
				multi_add_nav_ship();
				break;

			case OP_NAV_DEL:
				multi_del_nav();
				break;

			case OP_ADD_REMOVE_ESCORT:
				multi_sexp_add_remove_escort();
				break;

			case OP_CUTSCENES_SHOW_SUBTITLE_TEXT:
				 multi_sexp_show_subtitle_text();
				 break;

			case OP_CUTSCENES_SHOW_SUBTITLE_IMAGE:
				 multi_sexp_show_subtitle_image();
				 break;

			case OP_TRAINING_MSG:
				multi_sexp_send_training_message(); 
				break;

			case OP_HUD_DISABLE:
				multi_sexp_hud_disable();
				break;
			
			case OP_HUD_DISABLE_EXCEPT_MESSAGES:
				multi_sexp_hud_disable_except_messages();
				break;

			case OP_FLASH_HUD_GAUGE:
				multi_sexp_flash_hud_gauge();
				break;

			case OP_HUD_DISPLAY_GAUGE:
				multi_sexp_hud_display_gauge();
				break;

			case OP_CUTSCENES_SET_CUTSCENE_BARS:
			case OP_CUTSCENES_UNSET_CUTSCENE_BARS:
				multi_sexp_toggle_cutscene_bars(op_num == OP_CUTSCENES_SET_CUTSCENE_BARS );
				break;

			case OP_CUTSCENES_SET_CAMERA_FACING:
				multi_sexp_set_camera_facing();
				break;

			case OP_CUTSCENES_SET_CAMERA_FACING_OBJECT:
				multi_sexp_set_camera_facing_object();
				break;

			case OP_CUTSCENES_SET_CAMERA_TARGET :
				multi_sexp_set_camera_target();
				break;

			case OP_CUTSCENES_SET_CAMERA_ROTATION:
				multi_sexp_set_camera_rotation();
				break;

			case OP_CUTSCENES_SET_CAMERA_FOV:
				multi_sexp_set_camera_fov();
				break;

			case OP_CUTSCENES_SET_CAMERA_POSITION:
				multi_sexp_set_camera_position();
				break;

			case OP_SET_CAMERA_SHUDDER:
				multi_sexp_set_camera_shudder();
				break;

			case OP_CUTSCENES_RESET_CAMERA:
				multi_sexp_reset_camera();
				break;

			case OP_CUTSCENES_SET_FOV:
				multi_sexp_set_fov();
				break;

			case OP_CUTSCENES_RESET_FOV:
				multi_sexp_reset_fov();
				break;

			case OP_JUMP_NODE_SET_JUMPNODE_NAME:
				multi_sexp_set_jumpnode_name();
				break;

			case OP_IGNORE_KEY:
				multi_sexp_ignore_key();
				break;

			case OP_JUMP_NODE_SET_JUMPNODE_COLOR:
				multi_sexp_set_jumpnode_color();
				break;

			case OP_JUMP_NODE_SET_JUMPNODE_MODEL:
				multi_sexp_set_jumpnode_model();
				break;

			case OP_JUMP_NODE_SHOW_JUMPNODE:
			case OP_JUMP_NODE_HIDE_JUMPNODE:
				multi_sexp_show_hide_jumpnode(op_num == OP_JUMP_NODE_SHOW_JUMPNODE);
				break;

			case OP_CLEAR_SUBTITLES:
				multi_sexp_clear_subtitles();
				break;

			case OP_SET_OBJECT_SPEED_X:
			case OP_SET_OBJECT_SPEED_Y:
			case OP_SET_OBJECT_SPEED_Z:
				multi_sexp_set_object_speed();
				break;

			// bad sexp in the packet
			default: 
				// probably just a version error where the host supports a SEXP but a client does not
				if (multi_sexp_discard_operator()) {
					Warning(LOCATION, "Received invalid SEXP operator number from host. Operator number %d is not supported by this version.", op_num); 
				}
				// a more major problem
				else {
					Warning(LOCATION, "Received invalid SEXP packet from host. Function involving operator %d lacks termination. Entire packet may be corrupt. Discarding remaining packet", op_num); 
					Int3(); 
					return; 
				}			
		}

		multi_finished_callback();
	}
}

//	Still a debug-level system.
//	get_sexp_main reads and builds the internal representation for a
//	symbolic expression.
//	On entry:
//		Mp points at first character in expression.
//	The symbolic expression is built in Sexp_nodes beginning at node 0.
int get_sexp_main()
{
	int	start_node, op;
	char	token[TOKEN_LENGTH];
	char  *savep, ch;

	ignore_white_space();

	savep = Mp;
	if (!strncmp(Mp, "( )", 3))
		savep++;

	Assert(*Mp == '(');
	Mp++;
	start_node = get_sexp(token);
	// only need to check syntax if we have a operator
	if (Fred_running || (start_node == -1))
		return start_node;

	ch = *Mp;
	*Mp = '\0';

	op = get_operator_index(CTEXT(start_node));
	if (op == -1)
		Error (LOCATION, "Can't find operator %s in operator list\n.", CTEXT(start_node) );

	*Mp = ch;

	return start_node;
}

int run_sexp(const char* sexpression)
{
	char* oldMp = Mp;
	int n, i, sexp_val = UNINITIALIZED;
	char buf[8192];

	strncpy(buf, sexpression, 8192);

	// HACK: ! -> "
	for (i = 0; i < (int)strlen(buf); i++)
		if (buf[i] == '!')
			buf[i]='\"';

	Mp = buf;

	n = get_sexp_main();
	if (n != -1)
	{
		sexp_val = eval_sexp(n);
		free_sexp2(n);
	}
	Mp = oldMp;

	return sexp_val;
}

DCF(sexpc, "Always runs the given sexp command ")
{
	if ( Dc_command )       {
		if (Dc_command_line != NULL) {
			char buf[8192];
			snprintf(buf, 8191, "( when ( true ) ( %s ) )", Dc_command_line);

			int sexp_val = run_sexp( buf );
			dc_printf("SEXP '%s' run, sexp_val = %d\n", buf, sexp_val);
			do {
				dc_get_arg(ARG_ANY);
			} while (Dc_arg_type != ARG_NONE);
		}
	}
	if ( Dc_help )  {
		dc_printf( "Usage: sexpc sexpression\n. Always runs the given sexp as '( when ( true ) ( sexp ) )' .\n" );
	}
}


DCF(sexp,"Runs the given sexp")
{
	if ( Dc_command )       {
		if (Dc_command_line != NULL) {
			int sexp_val = run_sexp( Dc_command_line );
			dc_printf("SEXP '%s' run, sexp_val = %d\n", Dc_command_line, sexp_val);
			do {
				dc_get_arg(ARG_ANY);
			} while (Dc_arg_type != ARG_NONE);
		}
	}
	if ( Dc_help )  {
		dc_printf( "Usage: sexp 'sexpression'\n. Runs the given sexp.\n");
	}
}


void test_sexps()
{
	Mp = Mission_text;
	while (*Mp != '#') {
		get_sexp_main();
		diag_printf("\n----------------\n");
		ignore_white_space();
	}
	exit(0);
}

// returns the data type returned by an operator
int query_operator_return_type(int op)
{
	if (op < FIRST_OP)
	{
		Assert(op >= 0 && op < Num_operators);
		op = Operators[op].value;
	}

	switch (op)
	{
		case OP_TRUE:
		case OP_FALSE:
		case OP_AND:
		case OP_AND_IN_SEQUENCE:
		case OP_OR:
		case OP_NOT:
		case OP_XOR:
		case OP_EQUALS:
		case OP_GREATER_THAN:
		case OP_LESS_THAN:
		case OP_NOT_EQUAL:
		case OP_GREATER_OR_EQUAL:
		case OP_LESS_OR_EQUAL:
		case OP_STRING_EQUALS:
		case OP_STRING_GREATER_THAN:
		case OP_STRING_LESS_THAN:
		case OP_PERFORM_ACTIONS:
		case OP_IS_DESTROYED:
		case OP_IS_SUBSYSTEM_DESTROYED:
		case OP_IS_DISABLED:
		case OP_IS_DISARMED:
		case OP_HAS_DOCKED:
		case OP_HAS_UNDOCKED:
		case OP_HAS_ARRIVED:
		case OP_HAS_DEPARTED:
		case OP_IS_DESTROYED_DELAY:
		case OP_IS_SUBSYSTEM_DESTROYED_DELAY:
		case OP_IS_DISABLED_DELAY:
		case OP_IS_DISARMED_DELAY:
		case OP_HAS_DOCKED_DELAY:
		case OP_HAS_UNDOCKED_DELAY:
		case OP_HAS_ARRIVED_DELAY:
		case OP_HAS_DEPARTED_DELAY:
		case OP_IS_IFF:
		case OP_IS_AI_CLASS:
		case OP_IS_SHIP_TYPE:
		case OP_IS_SHIP_CLASS:
		case OP_HAS_TIME_ELAPSED:
		case OP_GOAL_INCOMPLETE:
		case OP_GOAL_TRUE_DELAY:
		case OP_GOAL_FALSE_DELAY:
		case OP_EVENT_INCOMPLETE:
		case OP_EVENT_TRUE_DELAY:
		case OP_EVENT_FALSE_MSECS_DELAY:
		case OP_EVENT_TRUE_MSECS_DELAY:
		case OP_EVENT_FALSE_DELAY:
		case OP_PREVIOUS_EVENT_TRUE:
		case OP_PREVIOUS_EVENT_FALSE:
		case OP_PREVIOUS_EVENT_INCOMPLETE:
		case OP_PREVIOUS_GOAL_TRUE:
		case OP_PREVIOUS_GOAL_FALSE:
		case OP_PREVIOUS_GOAL_INCOMPLETE:
		case OP_WAYPOINTS_DONE:
		case OP_WAYPOINTS_DONE_DELAY:
		case OP_SHIP_TYPE_DESTROYED:
		case OP_LAST_ORDER_TIME:
		case OP_KEY_PRESSED:
		case OP_TARGETED:
		case OP_NODE_TARGETED:
		case OP_SPEED:
		case OP_FACING:
		case OP_FACING2:
		case OP_ORDER:
		case OP_QUERY_ORDERS:
		case OP_WAYPOINT_MISSED:
		case OP_WAYPOINT_TWICE:
		case OP_PATH_FLOWN:
		case OP_EVENT_TRUE:
		case OP_EVENT_FALSE:
		case OP_SKILL_LEVEL_AT_LEAST:
		case OP_IS_CARGO_KNOWN:
		case OP_HAS_BEEN_TAGGED_DELAY:
		case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
		case OP_CARGO_KNOWN_DELAY:
		case OP_WAS_PROMOTION_GRANTED:
		case OP_WAS_MEDAL_GRANTED:
		case OP_PERCENT_SHIPS_DEPARTED:
		case OP_PERCENT_SHIPS_DESTROYED:
		case OP_PERCENT_SHIPS_DISARMED:
		case OP_PERCENT_SHIPS_DISABLED:
		case OP_PERCENT_SHIPS_ARRIVED:
		case OP_DEPART_NODE_DELAY:
		case OP_DESTROYED_DEPARTED_DELAY:
		case OP_SPECIAL_CHECK:
		case OP_IS_TAGGED:
		case OP_PRIMARIES_DEPLETED:
		case OP_SECONDARIES_DEPLETED:
		case OP_SHIELD_QUAD_LOW:
		case OP_IS_SECONDARY_SELECTED:
		case OP_IS_PRIMARY_SELECTED:
		case OP_IS_SHIP_STEALTHY:
		case OP_IS_FRIENDLY_STEALTH_VISIBLE:
		case OP_IS_CARGO:
		case OP_MISSILE_LOCKED:
		case OP_NAV_IS_VISITED:
		case OP_NAV_ISLINKED:
		case OP_IS_PLAYER:
		case OP_PRIMARY_FIRED_SINCE:
		case OP_SECONDARY_FIRED_SINCE:
		case OP_IS_FACING:
		case OP_HAS_PRIMARY_WEAPON:
		case OP_HAS_SECONDARY_WEAPON:
		case OP_IS_BIT_SET:
		case OP_DIRECTIVE_VALUE:
		case OP_IS_IN_BOX:
		case OP_IS_IN_MISSION:
			return OPR_BOOL;

		case OP_PLUS:
		case OP_MINUS:
		case OP_MOD:
		case OP_MUL:
		case OP_DIV:
		case OP_RAND:
		case OP_RAND_MULTIPLE:
		case OP_MIN:
		case OP_MAX:
		case OP_AVG:
		case OP_POW:
		case OP_SIGNUM:
		case OP_GET_OBJECT_X:
		case OP_GET_OBJECT_Y:
		case OP_GET_OBJECT_Z:
		case OP_GET_OBJECT_PITCH:
		case OP_GET_OBJECT_BANK:
		case OP_GET_OBJECT_HEADING:
		case OP_GET_OBJECT_SPEED_X:
		case OP_GET_OBJECT_SPEED_Y:
		case OP_GET_OBJECT_SPEED_Z:
		case OP_SCRIPT_EVAL_NUM:
		case OP_SCRIPT_EVAL_STRING:
		case OP_STRING_TO_INT:
		case OP_GET_THROTTLE_SPEED:
		case OP_GET_VARIABLE_BY_INDEX:
		case OP_GET_COLGROUP_ID:
			return OPR_NUMBER;

		case OP_ABS:
		case OP_SET_BIT:
		case OP_UNSET_BIT:
		case OP_BITWISE_AND:
		case OP_BITWISE_OR:
		case OP_BITWISE_NOT:
		case OP_BITWISE_XOR:
		case OP_TIME_SHIP_DESTROYED:
		case OP_TIME_SHIP_ARRIVED:
		case OP_TIME_SHIP_DEPARTED:
		case OP_TIME_WING_DESTROYED:
		case OP_TIME_WING_ARRIVED:
		case OP_TIME_WING_DEPARTED:
		case OP_MISSION_TIME:
		case OP_MISSION_TIME_MSECS:
		case OP_TIME_DOCKED:
		case OP_TIME_UNDOCKED:
		case OP_AFTERBURNER_LEFT:
		case OP_WEAPON_ENERGY_LEFT:
		case OP_SHIELDS_LEFT:
		case OP_HITS_LEFT:
		case OP_HITS_LEFT_SUBSYSTEM:
		case OP_HITS_LEFT_SUBSYSTEM_GENERIC:
		case OP_HITS_LEFT_SUBSYSTEM_SPECIFIC:
		case OP_SIM_HITS_LEFT:
		case OP_DISTANCE:
		case OP_DISTANCE_SUBSYSTEM:
		case OP_NUM_WITHIN_BOX:
		case OP_NUM_PLAYERS:
		case OP_NUM_KILLS:
		case OP_NUM_ASSISTS:
		case OP_SHIP_DEATHS: 
		case OP_RESPAWNS_LEFT:
		case OP_SHIP_SCORE:
		case OP_NUM_TYPE_KILLS:
		case OP_NUM_CLASS_KILLS:
		case OP_SHIELD_RECHARGE_PCT:
		case OP_ENGINE_RECHARGE_PCT:
		case OP_WEAPON_RECHARGE_PCT:
		case OP_PRIMARY_AMMO_PCT:
		case OP_SECONDARY_AMMO_PCT:
		case OP_GET_PRIMARY_AMMO:
		case OP_GET_SECONDARY_AMMO:
		case OP_GET_NUM_COUNTERMEASURES:
		case OP_SPECIAL_WARP_DISTANCE:
		case OP_IS_SHIP_VISIBLE:
		case OP_TEAM_SCORE:
		case OP_NUM_SHIPS_IN_BATTLE:
		case OP_NUM_SHIPS_IN_WING:
		case OP_CURRENT_SPEED:
		case OP_NAV_DISTANCE:
		case OP_GET_DAMAGE_CAUSED:
		case OP_CUTSCENES_GET_FOV:
		case OP_NUM_VALID_ARGUMENTS:
		case OP_STRING_GET_LENGTH:
			return OPR_POSITIVE;

		case OP_COND:
		case OP_WHEN:
		case OP_WHEN_ARGUMENT:
		case OP_EVERY_TIME:
		case OP_EVERY_TIME_ARGUMENT:
		case OP_IF_THEN_ELSE:
		case OP_INVALIDATE_ARGUMENT:
		case OP_VALIDATE_ARGUMENT:
		case OP_INVALIDATE_ALL_ARGUMENTS:
		case OP_VALIDATE_ALL_ARGUMENTS:
		case OP_DO_FOR_VALID_ARGUMENTS:
		case OP_CHANGE_IFF:
		case OP_CHANGE_AI_CLASS:
		case OP_CLEAR_SHIP_GOALS:
		case OP_CLEAR_WING_GOALS:
		case OP_CLEAR_GOALS:
		case OP_ADD_SHIP_GOAL:
		case OP_ADD_WING_GOAL:
		case OP_ADD_GOAL:
		case OP_REMOVE_GOAL:
		case OP_PROTECT_SHIP:
		case OP_UNPROTECT_SHIP:
		case OP_BEAM_PROTECT_SHIP:
		case OP_BEAM_UNPROTECT_SHIP:
		case OP_TURRET_PROTECT_SHIP:
		case OP_TURRET_UNPROTECT_SHIP:
		case OP_NOP:
		case OP_GOALS_ID:
		case OP_SEND_MESSAGE:
		case OP_SELF_DESTRUCT:
		case OP_NEXT_MISSION:
		case OP_END_CAMPAIGN:
		case OP_END_OF_CAMPAIGN:
		case OP_SABOTAGE_SUBSYSTEM:
		case OP_REPAIR_SUBSYSTEM:
		case OP_INVALIDATE_GOAL:
		case OP_VALIDATE_GOAL:
		case OP_SEND_RANDOM_MESSAGE:
		case OP_TRANSFER_CARGO:
		case OP_EXCHANGE_CARGO:
		case OP_SET_CARGO:
		case OP_JETTISON_CARGO:
		case OP_SET_DOCKED:
		case OP_CARGO_NO_DEPLETE:
		case OP_SET_SCANNED:
		case OP_SET_UNSCANNED:
		case OP_KEY_RESET:
		case OP_KEY_RESET_MULTIPLE:
		case OP_TRAINING_MSG:
		case OP_SET_TRAINING_CONTEXT_FLY_PATH:
		case OP_SET_TRAINING_CONTEXT_SPEED:
		case OP_END_MISSION:
		case OP_SET_DEBRIEFING_TOGGLED:
		case OP_FORCE_JUMP:
		case OP_SET_SUBSYSTEM_STRNGTH:
		case OP_GOOD_REARM_TIME:
		case OP_GRANT_PROMOTION:
		case OP_GRANT_MEDAL:
		case OP_ALLOW_SHIP:
		case OP_ALLOW_WEAPON:
		case OP_TECH_ADD_SHIP:
		case OP_TECH_ADD_WEAPON:
		case OP_TECH_ADD_INTEL:
		case OP_TECH_RESET_TO_DEFAULT:
		case OP_CHANGE_PLAYER_SCORE:
		case OP_CHANGE_TEAM_SCORE:
		case OP_WARP_BROKEN:
		case OP_WARP_NOT_BROKEN:
		case OP_WARP_NEVER:
		case OP_WARP_ALLOWED:
		case OP_SET_SUBSPACE_DRIVE:
		case OP_FLASH_HUD_GAUGE:
		case OP_GOOD_SECONDARY_TIME:
		case OP_SHIP_VISIBLE:
		case OP_SHIP_INVISIBLE:
		case OP_SHIP_TAG:
		case OP_SHIP_UNTAG:
		case OP_SHIP_VULNERABLE:
		case OP_SHIP_INVULNERABLE:
		case OP_SHIP_BOMB_TARGETABLE:
		case OP_SHIP_BOMB_UNTARGETABLE:
		case OP_SHIP_GUARDIAN:
		case OP_SHIP_NO_GUARDIAN:
		case OP_SHIP_GUARDIAN_THRESHOLD:
		case OP_SHIP_SUBSYS_GUARDIAN_THRESHOLD:
		case OP_SHIP_VANISH:
		case OP_SHIELDS_ON:
		case OP_SHIELDS_OFF:
		case OP_SHIP_STEALTHY:
		case OP_SHIP_UNSTEALTHY:
		case OP_FRIENDLY_STEALTH_INVISIBLE:
		case OP_FRIENDLY_STEALTH_VISIBLE:
		case OP_SHIP_SUBSYS_NO_REPLACE:
		case OP_SHIP_SUBSYS_NO_LIVE_DEBRIS:
		case OP_SHIP_SUBSYS_VANISHED:
		case OP_SHIP_SUBSYS_IGNORE_IF_DEAD:
		case OP_SHIP_SUBSYS_TARGETABLE:
		case OP_SHIP_SUBSYS_UNTARGETABLE:
		case OP_RED_ALERT:
		case OP_MODIFY_VARIABLE:
		case OP_SET_VARIABLE_BY_INDEX:
		case OP_BEAM_FIRE:
		case OP_BEAM_FIRE_COORDS:
		case OP_BEAM_FREE:
		case OP_BEAM_FREE_ALL:
		case OP_BEAM_LOCK:
		case OP_BEAM_LOCK_ALL:
		case OP_TURRET_FREE:
		case OP_TURRET_FREE_ALL:
		case OP_TURRET_LOCK:
		case OP_TURRET_LOCK_ALL:
		case OP_TURRET_CHANGE_WEAPON:
		case OP_TURRET_SET_DIRECTION_PREFERENCE:
		case OP_TURRET_SET_RATE_OF_FIRE:
		case OP_TURRET_SET_OPTIMUM_RANGE:
		case OP_TURRET_SET_TARGET_PRIORITIES:
		case OP_TURRET_SET_TARGET_ORDER:
		case OP_SET_ARMOR_TYPE:
		case OP_WEAPON_SET_DAMAGE_TYPE:
		case OP_SHIP_SET_DAMAGE_TYPE:
		case OP_SHIP_SHOCKWAVE_SET_DAMAGE_TYPE:
		case OP_FIELD_SET_DAMAGE_TYPE:
		case OP_SHIP_TURRET_TARGET_ORDER:
		case OP_TURRET_SUBSYS_TARGET_DISABLE:
		case OP_TURRET_SUBSYS_TARGET_ENABLE:
		case OP_ADD_REMOVE_ESCORT:
		case OP_DAMAGED_ESCORT_LIST:
		case OP_DAMAGED_ESCORT_LIST_ALL:
		case OP_AWACS_SET_RADIUS:
		case OP_PRIMITIVE_SENSORS_SET_RANGE:
		case OP_SEND_MESSAGE_LIST:
		case OP_CAP_WAYPOINT_SPEED:
		case OP_TURRET_TAGGED_ONLY_ALL:
		case OP_TURRET_TAGGED_CLEAR_ALL:
		case OP_SUBSYS_SET_RANDOM:
		case OP_SUPERNOVA_START:
		case OP_SUPERNOVA_STOP:
		case OP_SET_SPECIAL_WARPOUT_NAME:
		case OP_SHIP_VAPORIZE:
		case OP_SHIP_NO_VAPORIZE:
		case OP_SET_EXPLOSION_OPTION:
		case OP_DONT_COLLIDE_INVISIBLE:
		case OP_COLLIDE_INVISIBLE:
		case OP_SET_MOBILE:
		case OP_SET_IMMOBILE:
		case OP_IGNORE_KEY:
		case OP_CHANGE_SHIP_CLASS:
		case OP_SHIP_COPY_DAMAGE:
		case OP_DEACTIVATE_GLOW_POINTS:
		case OP_ACTIVATE_GLOW_POINTS:
		case OP_DEACTIVATE_GLOW_MAPS:
		case OP_ACTIVATE_GLOW_MAPS:
		case OP_DEACTIVATE_GLOW_POINT_BANK:
		case OP_ACTIVATE_GLOW_POINT_BANK:
		case OP_SET_SKYBOX_MODEL:
		case OP_SET_SKYBOX_ORIENT:
		case OP_SET_SUPPORT_SHIP:
		case OP_SET_ARRIVAL_INFO:
		case OP_SET_DEPARTURE_INFO:
		case OP_CHANGE_SOUNDTRACK:
		case OP_PLAY_SOUND_FROM_FILE:
		case OP_CLOSE_SOUND_FROM_FILE:
		case OP_PLAY_SOUND_FROM_TABLE:
		case OP_SET_SOUND_ENVIRONMENT:
		case OP_UPDATE_SOUND_ENVIRONMENT:
		case OP_ADJUST_AUDIO_VOLUME:
		case OP_EXPLOSION_EFFECT:
		case OP_WARP_EFFECT:
		case OP_SET_OBJECT_POSITION:
		case OP_SET_OBJECT_ORIENTATION:
		case OP_SET_OBJECT_FACING:
		case OP_SET_OBJECT_FACING_OBJECT:
		case OP_SHIP_MANEUVER:
		case OP_SHIP_ROT_MANEUVER:
		case OP_SHIP_LAT_MANEUVER:
		case OP_HUD_DISABLE:
		case OP_HUD_DISABLE_EXCEPT_MESSAGES:
		case OP_KAMIKAZE:
		case OP_TURRET_TAGGED_SPECIFIC:
		case OP_TURRET_TAGGED_CLEAR_SPECIFIC:
		case OP_LOCK_ROTATING_SUBSYSTEM:
		case OP_FREE_ROTATING_SUBSYSTEM:
		case OP_REVERSE_ROTATING_SUBSYSTEM:
		case OP_ROTATING_SUBSYS_SET_TURN_TIME:
		case OP_TRIGGER_SUBMODEL_ANIMATION:
		case OP_PLAYER_USE_AI:
		case OP_PLAYER_NOT_USE_AI:
		case OP_ALLOW_TREASON:
		case OP_SET_PLAYER_ORDERS:
		case OP_NAV_ADD_WAYPOINT:
		case OP_NAV_ADD_SHIP:
		case OP_NAV_DEL:
		case OP_NAV_HIDE:
		case OP_NAV_RESTRICT:
		case OP_NAV_UNHIDE:
		case OP_NAV_UNRESTRICT:
		case OP_NAV_SET_VISITED:
		case OP_NAV_UNSET_VISITED:
		case OP_NAV_SET_CARRY:
		case OP_NAV_UNSET_CARRY:
		case OP_NAV_SET_NEEDSLINK:
		case OP_NAV_UNSET_NEEDSLINK:
		case OP_NAV_USECINEMATICS:
		case OP_NAV_USEAP:
		case OP_HUD_SET_TEXT:
		case OP_HUD_SET_TEXT_NUM:
		case OP_HUD_SET_MESSAGE:
		case OP_HUD_SET_COORDS:
		case OP_HUD_SET_FRAME:
		case OP_HUD_SET_COLOR:
		case OP_HUD_SET_MAX_TARGETING_RANGE:
		case OP_HUD_CLEAR_MESSAGES:
		case OP_SHIP_CHANGE_ALT_NAME:
		case OP_SHIP_CHANGE_CALLSIGN:
		case OP_SET_DEATH_MESSAGE:
		case OP_SCRAMBLE_MESSAGES:
		case OP_UNSCRAMBLE_MESSAGES:
		case OP_CUTSCENES_SET_CUTSCENE_BARS:
		case OP_CUTSCENES_UNSET_CUTSCENE_BARS:
		case OP_CUTSCENES_FADE_IN:
		case OP_CUTSCENES_FADE_OUT:
		case OP_CUTSCENES_SET_CAMERA:
		case OP_CUTSCENES_SET_CAMERA_FACING:
		case OP_CUTSCENES_SET_CAMERA_FACING_OBJECT:
		case OP_CUTSCENES_SET_CAMERA_FOV:
		case OP_CUTSCENES_SET_CAMERA_HOST:
		case OP_CUTSCENES_SET_CAMERA_POSITION:
		case OP_CUTSCENES_SET_CAMERA_ROTATION:
		case OP_CUTSCENES_SET_CAMERA_TARGET:
		case OP_CUTSCENES_SET_FOV:
		case OP_CUTSCENES_RESET_FOV:
		case OP_CUTSCENES_RESET_CAMERA:
		case OP_CUTSCENES_SHOW_SUBTITLE:
		case OP_CUTSCENES_SHOW_SUBTITLE_TEXT:
		case OP_CUTSCENES_SHOW_SUBTITLE_IMAGE:
		case OP_CUTSCENES_SET_TIME_COMPRESSION:
		case OP_CUTSCENES_RESET_TIME_COMPRESSION:
		case OP_CUTSCENES_FORCE_PERSPECTIVE:
		case OP_SET_CAMERA_SHUDDER:
		case OP_JUMP_NODE_SET_JUMPNODE_NAME:
		case OP_JUMP_NODE_SET_JUMPNODE_COLOR:
		case OP_JUMP_NODE_SET_JUMPNODE_MODEL:
		case OP_JUMP_NODE_SHOW_JUMPNODE:
		case OP_JUMP_NODE_HIDE_JUMPNODE:
		case OP_SET_OBJECT_SPEED_X:
		case OP_SET_OBJECT_SPEED_Y:
		case OP_SET_OBJECT_SPEED_Z:
		case OP_SHIP_CREATE:
		case OP_WEAPON_CREATE:
		case OP_MISSION_SET_NEBULA:
		case OP_ADD_BACKGROUND_BITMAP:
		case OP_REMOVE_BACKGROUND_BITMAP:
		case OP_ADD_SUN_BITMAP:
		case OP_REMOVE_SUN_BITMAP:
		case OP_NEBULA_CHANGE_STORM:
		case OP_NEBULA_TOGGLE_POOF:
		case OP_SET_PRIMARY_AMMO:
		case OP_SET_SECONDARY_AMMO:
		case OP_SET_PRIMARY_WEAPON:
		case OP_SET_SECONDARY_WEAPON:
		case OP_SET_NUM_COUNTERMEASURES:
		case OP_SCRIPT_EVAL:
		case OP_ENABLE_BUILTIN_MESSAGES:
		case OP_DISABLE_BUILTIN_MESSAGES:
		case OP_LOCK_PRIMARY_WEAPON:
		case OP_UNLOCK_PRIMARY_WEAPON:
		case OP_LOCK_SECONDARY_WEAPON:
		case OP_UNLOCK_SECONDARY_WEAPON:
		case OP_LOCK_AFTERBURNER:
		case OP_UNLOCK_AFTERBURNER:
		case OP_RESET_ORDERS:
		case OP_SET_PERSONA:
		case OP_CHANGE_SUBSYSTEM_NAME:
		case OP_SET_RESPAWNS:
		case OP_SET_AFTERBURNER_ENERGY: 
		case OP_SET_WEAPON_ENERGY:
		case OP_SET_SHIELD_ENERGY:
		case OP_SET_AMBIENT_LIGHT:
		case OP_SET_POST_EFFECT:
		case OP_CHANGE_IFF_COLOR:
		case OP_REMOVE_WEAPONS:
		case OP_MISSION_SET_SUBSPACE:
		case OP_HUD_DISPLAY_GAUGE:
		case OP_FORCE_GLIDE:
		case OP_HUD_SET_DIRECTIVE:
		case OP_HUD_GAUGE_SET_ACTIVE:
		case OP_HUD_ACTIVATE_GAUGE_TYPE:
		case OP_STRING_CONCATENATE:
		case OP_INT_TO_STRING:
		case OP_DISABLE_ETS:
		case OP_ENABLE_ETS:
		case OP_STRING_GET_SUBSTRING:
		case OP_STRING_SET_SUBSTRING:
		case OP_ADD_TO_COLGROUP:
		case OP_REMOVE_FROM_COLGROUP:
		case OP_SHIP_EFFECT:
		case OP_CLEAR_SUBTITLES:
		case OP_SET_THRUSTERS:
		case OP_SET_PLAYER_THROTTLE_SPEED:
			return OPR_NULL;

		case OP_AI_CHASE:
		case OP_AI_DOCK:
		case OP_AI_UNDOCK:
		case OP_AI_WARP:						// this particular operator is obsolete
		case OP_AI_WARP_OUT:
		case OP_AI_WAYPOINTS:
		case OP_AI_WAYPOINTS_ONCE:
		case OP_AI_DESTROY_SUBSYS:
		case OP_AI_CHASE_WING:
		case OP_AI_DISABLE_SHIP:
		case OP_AI_DISARM_SHIP:
		case OP_AI_GUARD:
		case OP_AI_GUARD_WING:
		case OP_AI_CHASE_ANY:
		case OP_AI_EVADE_SHIP:
		case OP_AI_STAY_NEAR_SHIP:
		case OP_AI_KEEP_SAFE_DISTANCE:
		case OP_AI_IGNORE:
		case OP_AI_IGNORE_NEW:
		case OP_AI_STAY_STILL:
		case OP_AI_PLAY_DEAD:
		case OP_AI_FORM_ON_WING:
			return OPR_AI_GOAL;

		case OP_ANY_OF:
		case OP_EVERY_OF:
		case OP_RANDOM_OF:
		case OP_RANDOM_MULTIPLE_OF:
		case OP_NUMBER_OF:
		case OP_IN_SEQUENCE:
		case OP_FOR_COUNTER:
			return OPR_FLEXIBLE_ARGUMENT;

		default:
			Int3();
	}

	return 0;
}

/**
 * Return the data type of a specified argument to an operator.  
 *
 * @param op operator index
 * @param argnum is 0 indexed.
 */
int query_operator_argument_type(int op, int argnum)
{
	int index = op;

	if (op < FIRST_OP)
	{
		Assert(index >= 0 && index < Num_operators);
		op = Operators[index].value;

	} else {
		Warning(LOCATION, "Possible unnecessary search for operator index.  Trace out and see if this is necessary.\n");

		for (index=0; index<Num_operators; index++)
			if (Operators[index].value == op)
				break;

		Assert(index < Num_operators);
	}

	if (argnum >= Operators[index].max)
		return OPF_NONE;

	switch (op) {
		case OP_TRUE:
		case OP_FALSE:
		case OP_MISSION_TIME:
		case OP_MISSION_TIME_MSECS:
		case OP_NOP:
		case OP_WAYPOINT_MISSED:
		case OP_WAYPOINT_TWICE:
		case OP_PATH_FLOWN:
		case OP_GRANT_PROMOTION:
		case OP_WAS_PROMOTION_GRANTED:
		case OP_RED_ALERT:
		case OP_FORCE_JUMP:
		case OP_RESET_ORDERS:
		case OP_INVALIDATE_ALL_ARGUMENTS:
		case OP_VALIDATE_ALL_ARGUMENTS:
		case OP_NUM_VALID_ARGUMENTS:
		case OP_SUPERNOVA_STOP:
			return OPF_NONE;

		case OP_AND:
		case OP_AND_IN_SEQUENCE:
		case OP_OR:
		case OP_NOT:
		case OP_XOR:
			return OPF_BOOL;

		case OP_PLUS:
		case OP_MINUS:
		case OP_MOD:
		case OP_MUL:
		case OP_DIV:
		case OP_EQUALS:
		case OP_GREATER_THAN:
		case OP_LESS_THAN:
		case OP_NOT_EQUAL:
		case OP_GREATER_OR_EQUAL:
		case OP_LESS_OR_EQUAL:
		case OP_RAND:
		case OP_RAND_MULTIPLE:
		case OP_ABS:
		case OP_MIN:
		case OP_MAX:
		case OP_AVG:
			return OPF_NUMBER;

		case OP_POW:
			if (argnum == 0)
				return OPF_NUMBER;
			else
				return OPF_POSITIVE;

		case OP_SIGNUM:
			return OPF_NUMBER;

		case OP_STRING_EQUALS:
		case OP_STRING_GREATER_THAN:
		case OP_STRING_LESS_THAN:
		case OP_STRING_TO_INT:		// Karajorma
		case OP_STRING_GET_LENGTH:	// Goober5000
			return OPF_STRING;

		case OP_STRING_CONCATENATE:
			if (argnum == 0 || argnum == 1) {
				return OPF_STRING;
			} else if (argnum == 2) {
				return OPF_VARIABLE_NAME;
			}

		case OP_INT_TO_STRING:
			if (argnum == 0) {
				return OPF_NUMBER;
			} else if (argnum == 1) {
				return OPF_VARIABLE_NAME;
			}

		case OP_STRING_GET_SUBSTRING:
			if (argnum == 0) {
				return OPF_STRING;
			} else if (argnum == 1 || argnum == 2) {
				return OPF_POSITIVE;
			} else if (argnum == 3) {
				return OPF_VARIABLE_NAME;
			}

		case OP_STRING_SET_SUBSTRING:
			if (argnum == 0) {
				return OPF_STRING;
			} else if (argnum == 1 || argnum == 2) {
				return OPF_POSITIVE;
			} else if (argnum == 3) {
				return OPF_STRING;
			} else if (argnum == 4) {
				return OPF_VARIABLE_NAME;
			}

		case OP_HAS_TIME_ELAPSED:
		case OP_SPEED:
		case OP_SET_TRAINING_CONTEXT_SPEED:
		case OP_SPECIAL_CHECK:
		case OP_AI_WARP_OUT:
		case OP_TEAM_SCORE:
		case OP_HUD_SET_MAX_TARGETING_RANGE:
		case OP_MISSION_SET_NEBULA:	//WMC
		case OP_MISSION_SET_SUBSPACE:
		case OP_SET_BIT:
		case OP_UNSET_BIT:
		case OP_IS_BIT_SET:
		case OP_BITWISE_AND:
		case OP_BITWISE_OR:
		case OP_BITWISE_NOT:
		case OP_BITWISE_XOR:
			return OPF_POSITIVE;

		case OP_AI_WARP:								// this operator is obsolete
		case OP_SET_TRAINING_CONTEXT_FLY_PATH:
			if ( !argnum )
				return OPF_WAYPOINT_PATH;
			else
				return OPF_NUMBER;
		
		case OP_AI_WAYPOINTS:
		case OP_AI_WAYPOINTS_ONCE:
			if ( argnum == 0 )
				return OPF_WAYPOINT_PATH;
			else
				return OPF_POSITIVE;

		case OP_TURRET_PROTECT_SHIP:
		case OP_TURRET_UNPROTECT_SHIP:
			if (argnum == 0)
				return OPF_STRING;
			else
				return OPF_SHIP;

		case OP_IS_DISABLED:
		case OP_IS_DISARMED:
		case OP_TIME_SHIP_DESTROYED:
		case OP_TIME_SHIP_ARRIVED:
		case OP_TIME_SHIP_DEPARTED:
		case OP_AFTERBURNER_LEFT:
		case OP_WEAPON_ENERGY_LEFT:
		case OP_SHIELDS_LEFT:
		case OP_HITS_LEFT:
		case OP_SIM_HITS_LEFT:
		case OP_CLEAR_SHIP_GOALS:
		case OP_PROTECT_SHIP:
		case OP_UNPROTECT_SHIP:
		case OP_BEAM_PROTECT_SHIP:
		case OP_BEAM_UNPROTECT_SHIP:
		case OP_TRANSFER_CARGO:
		case OP_EXCHANGE_CARGO:
		case OP_SHIP_INVISIBLE:
		case OP_SHIP_VISIBLE:	
		case OP_SHIP_INVULNERABLE:
		case OP_SHIP_VULNERABLE:
		case OP_SHIP_BOMB_TARGETABLE:
		case OP_SHIP_BOMB_UNTARGETABLE:
		case OP_SHIP_GUARDIAN:
		case OP_SHIP_NO_GUARDIAN:
		case OP_SHIP_VANISH:
		case OP_SHIELDS_ON:
		case OP_SHIELDS_OFF:
		case OP_SHIP_STEALTHY:
		case OP_SHIP_UNSTEALTHY:
		case OP_FRIENDLY_STEALTH_INVISIBLE:
		case OP_FRIENDLY_STEALTH_VISIBLE:
		case OP_PRIMARIES_DEPLETED:
		case OP_SECONDARIES_DEPLETED:
		case OP_SPECIAL_WARP_DISTANCE:
		case OP_SET_SPECIAL_WARPOUT_NAME:
		case OP_IS_SHIP_VISIBLE:
		case OP_IS_SHIP_STEALTHY:
		case OP_IS_FRIENDLY_STEALTH_VISIBLE:
		case OP_GET_DAMAGE_CAUSED:
		case OP_GET_THROTTLE_SPEED:
			return OPF_SHIP;
		
		case OP_SET_PLAYER_THROTTLE_SPEED:
			if(argnum == 0)
				return OPF_SHIP;
			else
				return OPF_POSITIVE;

		case OP_SHIP_CREATE:
			if(argnum == 0)
				return OPF_STRING;
			else if(argnum == 1)
				return OPF_SHIP_CLASS_NAME;
			else
				return OPF_NUMBER;

		case OP_WEAPON_CREATE:
			if (argnum == 0)
				return OPF_SHIP_OR_NONE;
			else if (argnum == 1)
				return OPF_WEAPON_NAME;
			else if (argnum == 8)
				return OPF_SHIP;
			else if (argnum == 9)
				return OPF_SUBSYSTEM;
			else
				return OPF_NUMBER;

		case OP_REMOVE_WEAPONS:
			return OPF_WEAPON_NAME;

		case OP_SHIP_GUARDIAN_THRESHOLD:
			if (argnum == 0)
				return OPF_POSITIVE;
			else
				return OPF_SHIP;

		case OP_SHIP_SUBSYS_GUARDIAN_THRESHOLD:
			if (argnum == 0)
				return OPF_POSITIVE;
			else if (argnum == 1)
				return OPF_SHIP;
			else
				return OPF_SUBSYS_OR_GENERIC;

		case OP_SHIP_SUBSYS_TARGETABLE:
		case OP_SHIP_SUBSYS_UNTARGETABLE:
			if (argnum == 0)
				return OPF_SHIP;
			else
				return OPF_SUBSYS_OR_GENERIC;

		case OP_SHIP_SUBSYS_NO_REPLACE:
		case OP_SHIP_SUBSYS_NO_LIVE_DEBRIS:
		case OP_SHIP_SUBSYS_VANISHED:
		case OP_SHIP_SUBSYS_IGNORE_IF_DEAD:
			if (argnum == 0)
				return OPF_SHIP;
			else if (argnum == 1)
				return OPF_BOOL;
			else
				return OPF_SUBSYS_OR_GENERIC;

		case OP_IS_DESTROYED:
		case OP_HAS_ARRIVED:
		case OP_HAS_DEPARTED:
		case OP_CLEAR_GOALS:
			return OPF_SHIP_WING;

		case OP_IS_DISABLED_DELAY:
		case OP_IS_DISARMED_DELAY:
			if ( argnum == 0 )
				return OPF_POSITIVE;
			else
				return OPF_SHIP;

		case OP_SHIP_TAG:
			if (argnum == 0)
				return OPF_SHIP;
            else if (argnum == 7)
                return OPF_IFF;
			else
				return OPF_POSITIVE;

		case OP_SHIP_UNTAG:
			return OPF_SHIP;

		case OP_FACING:
			if (argnum == 0)
				return OPF_SHIP;
			else
				return OPF_POSITIVE;

		case OP_FACING2:
			if (argnum == 0) {
				return OPF_WAYPOINT_PATH;
			} else {
				return OPF_POSITIVE;
			}

		case OP_ORDER:
			if (argnum == 1)
				return OPF_AI_ORDER;
			else
				return OPF_SHIP_WING;	// arg 0 or 2

		case OP_QUERY_ORDERS:
			if (argnum == 0)
				return OPF_ORDER_RECIPIENT;
			if (argnum == 1)
				return OPF_AI_ORDER;
			if (argnum == 2)
				return OPF_POSITIVE;
			if (argnum == 5)
				return OPF_SUBSYSTEM;
			else
				return OPF_SHIP_WING;

		case OP_IS_DESTROYED_DELAY:
		case OP_HAS_ARRIVED_DELAY:
		case OP_HAS_DEPARTED_DELAY:
		case OP_LAST_ORDER_TIME:
			if ( argnum == 0 )
				return OPF_POSITIVE;
			else
				return OPF_SHIP_WING;

		case OP_SHIP_CHANGE_ALT_NAME:
			if (argnum == 0)
				return OPF_STRING;
			else
				return OPF_SHIP_WING;

		case OP_SHIP_CHANGE_CALLSIGN:
			if (argnum == 0)
				return OPF_STRING;
			else
				return OPF_SHIP;

		case OP_SET_DEATH_MESSAGE:
			return OPF_MESSAGE_OR_STRING;

		case OP_DISTANCE:
			return OPF_SHIP_WING_POINT;

		case OP_SET_OBJECT_SPEED_X:
		case OP_SET_OBJECT_SPEED_Y:
		case OP_SET_OBJECT_SPEED_Z:
			if (argnum == 0)
				return OPF_SHIP_WING_POINT;
			else if (argnum == 1)
				return OPF_NUMBER;
			else
				return OPF_BOOL;

		case OP_GET_OBJECT_SPEED_X:
		case OP_GET_OBJECT_SPEED_Y:
		case OP_GET_OBJECT_SPEED_Z:
			if (argnum == 0)
				return OPF_SHIP_WING_POINT;
			else
				return OPF_BOOL;

		case OP_GET_OBJECT_X:
		case OP_GET_OBJECT_Y:
		case OP_GET_OBJECT_Z:
			if (argnum == 0)
				return OPF_SHIP_WING_POINT;
			else if (argnum == 1)
				return OPF_SUBSYSTEM_OR_NONE;
			else
				return OPF_NUMBER;

		case OP_GET_OBJECT_PITCH:
		case OP_GET_OBJECT_BANK:
		case OP_GET_OBJECT_HEADING:
			return OPF_SHIP_WING_POINT;

		case OP_SET_OBJECT_POSITION:
			if(argnum == 0)
				return OPF_SHIP_WING_POINT;
			else
				return OPF_NUMBER;

		case OP_SET_OBJECT_ORIENTATION:
			if (argnum == 0)
				return OPF_SHIP_WING_POINT;
			else
				return OPF_NUMBER;

		case OP_SET_OBJECT_FACING:
			if (argnum == 0)
				return OPF_SHIP_WING_POINT;
			else if (argnum < 4)
				return OPF_NUMBER;
			else
				return OPF_POSITIVE;

		case OP_SET_OBJECT_FACING_OBJECT:
			if (argnum == 0 || argnum == 1)
				return OPF_SHIP_WING_POINT;
			else
				return OPF_POSITIVE;

		case OP_SHIP_MANEUVER:
			if (argnum == 0)
				return OPF_SHIP_WING_POINT;
			else if (argnum == 1)
				return OPF_POSITIVE;
			else if (argnum < 5)
				return OPF_NUMBER;
			else if (argnum == 5)
				return OPF_BOOL;
			else if (argnum < 9)
				return OPF_NUMBER;
			else
				return OPF_BOOL;

		case OP_SHIP_ROT_MANEUVER:
		case OP_SHIP_LAT_MANEUVER:
			if (argnum == 0)
				return OPF_SHIP_WING_POINT;
			else if (argnum == 1)
				return OPF_POSITIVE;
			else if (argnum < 5)
				return OPF_NUMBER;
			else
				return OPF_BOOL;

		case OP_MODIFY_VARIABLE:
			if (argnum == 0) {
				return OPF_VARIABLE_NAME;
			} else {
				return OPF_AMBIGUOUS; 
			}

		case OP_GET_VARIABLE_BY_INDEX:
			return OPF_POSITIVE;

		case OP_SET_VARIABLE_BY_INDEX:
			if (argnum == 0) {
				return OPF_POSITIVE;
			} else {
				return OPF_AMBIGUOUS;
			}

		case OP_HAS_DOCKED:
		case OP_HAS_UNDOCKED:
		case OP_HAS_DOCKED_DELAY:
		case OP_HAS_UNDOCKED_DELAY:
		case OP_TIME_DOCKED:
		case OP_TIME_UNDOCKED:
			if ( argnum < 2 )
				return OPF_SHIP;
			else
				return OPF_POSITIVE;

		case OP_TIME_WING_DESTROYED:
		case OP_TIME_WING_ARRIVED:
		case OP_TIME_WING_DEPARTED:
		case OP_CLEAR_WING_GOALS:
			return OPF_WING;

		case OP_SET_SCANNED:
		case OP_SET_UNSCANNED:
		case OP_IS_SUBSYSTEM_DESTROYED:
			if (!argnum)
				return OPF_SHIP;
			else
				return OPF_SUBSYSTEM;
			
		case OP_HITS_LEFT_SUBSYSTEM:
			if (argnum == 0)
				return OPF_SHIP;
			else if (argnum == 1) 
				return OPF_SUBSYSTEM;
			else 
				return OPF_BOOL;

		case OP_HITS_LEFT_SUBSYSTEM_GENERIC:
			if (argnum == 0)
				return OPF_SHIP;
			else
				return OPF_SUBSYSTEM_TYPE;

		case OP_HITS_LEFT_SUBSYSTEM_SPECIFIC:
			if (argnum == 0)
				return OPF_SHIP;
			else if (argnum == 1)
				return OPF_SUBSYSTEM;

		case OP_DISTANCE_SUBSYSTEM:
			if (argnum == 0)
				return OPF_SHIP_WING_POINT;
			else if (argnum == 1)
				return OPF_SHIP;
			else if (argnum == 2)
				return OPF_SUBSYSTEM;
			else
				Int3();		// shouldn't happen

		case OP_NUM_WITHIN_BOX:
			if(argnum < 3)
				return OPF_NUMBER;
			else if(argnum < 6)
				return OPF_POSITIVE;
			else
				return OPF_SHIP_WING;

		case OP_IS_IN_BOX:
			if (argnum == 0) // First arg is a ship/wing
				return OPF_SHIP_WING_POINT;
			else if (argnum <= 6) // Next 6 args are coordinates
				return OPF_NUMBER;
			else // Next arg is a ship
				return OPF_SHIP;

		case OP_IS_IN_MISSION:
			return OPF_STRING;

		// Sesquipedalian
		case OP_MISSILE_LOCKED:
			if (argnum == 0)
				return OPF_POSITIVE;
			else if (argnum == 1)
				return OPF_SHIP;
			else
				return OPF_SUBSYSTEM;

		case OP_TARGETED:
			if (!argnum)
				return OPF_SHIP;
			else if (argnum == 1)
				return OPF_POSITIVE;
			else
				return OPF_SUBSYSTEM;

		case OP_NODE_TARGETED:
			if (!argnum)
				return OPF_JUMP_NODE_NAME;
			else if (argnum == 1)
				return OPF_POSITIVE;

		case OP_IS_SUBSYSTEM_DESTROYED_DELAY:
			if ( argnum == 0 )
				return OPF_SHIP;
			else if ( argnum == 1 )
				return OPF_SUBSYSTEM;
			else
				return OPF_POSITIVE;

		case OP_IS_IFF:
		case OP_CHANGE_IFF:
			if (!argnum)
				return OPF_IFF;
			else
				return OPF_SHIP_WING;

		case OP_ADD_SHIP_GOAL:
			if (!argnum)
				return OPF_SHIP;
			else
				return OPF_AI_GOAL;

		case OP_ADD_WING_GOAL:
			if (!argnum)
				return OPF_WING;
			else
				return OPF_AI_GOAL;

		case OP_ADD_GOAL:
		case OP_REMOVE_GOAL:
			if ( argnum == 0 )
				return OPF_SHIP_WING;
			else
				return OPF_AI_GOAL;

		case OP_COND:
		case OP_WHEN:
		case OP_EVERY_TIME:
		case OP_IF_THEN_ELSE:
		case OP_PERFORM_ACTIONS:
			if (!argnum)
				return OPF_BOOL;
			else
				return OPF_NULL;

		case OP_WHEN_ARGUMENT:
		case OP_EVERY_TIME_ARGUMENT:
			if (argnum == 0)
				return OPF_FLEXIBLE_ARGUMENT;
			else if (argnum == 1)
				return OPF_BOOL;
			else
				return OPF_NULL;
			
		case OP_DO_FOR_VALID_ARGUMENTS:
			return OPF_NULL;

		case OP_ANY_OF:
		case OP_EVERY_OF:
		case OP_RANDOM_OF:
		case OP_RANDOM_MULTIPLE_OF:
		case OP_IN_SEQUENCE:
			return OPF_ANYTHING;

		case OP_NUMBER_OF:
			if (argnum == 0)
				return OPF_POSITIVE;
			else
				return OPF_ANYTHING;

		case OP_FOR_COUNTER:
			return OPF_NUMBER;

		case OP_INVALIDATE_ARGUMENT:
		case OP_VALIDATE_ARGUMENT:
			return OPF_ANYTHING;

		case OP_AI_DISABLE_SHIP:
		case OP_AI_DISARM_SHIP:
		case OP_AI_EVADE_SHIP:
		case OP_AI_STAY_NEAR_SHIP:
		case OP_AI_IGNORE:
		case OP_AI_IGNORE_NEW:
			if (!argnum)
				return OPF_SHIP;
			else
				return OPF_POSITIVE;

		case OP_AI_CHASE:
		case OP_AI_GUARD:
			if (!argnum)
				return OPF_SHIP_WING;
			else
				return OPF_POSITIVE;

		case OP_AI_KEEP_SAFE_DISTANCE:
			return OPF_POSITIVE;

		case OP_AI_DOCK:
			if (!argnum)
				return OPF_SHIP;
			else if (argnum == 1)
				return OPF_DOCKER_POINT;
			else if (argnum == 2)
				return OPF_DOCKEE_POINT;
			else
				return OPF_POSITIVE;

		case OP_AI_UNDOCK:
			if (argnum == 0)
				return OPF_POSITIVE;
			else
				return OPF_SHIP;

		case OP_AI_CHASE_WING:
		case OP_AI_GUARD_WING:
			if (!argnum)
				return OPF_WING;
			else
				return OPF_POSITIVE;

		case OP_AI_DESTROY_SUBSYS:
			if (!argnum)
				return OPF_SHIP;
			else if (argnum == 1)
				return OPF_SUBSYSTEM;
			else
				return OPF_POSITIVE;
			
		case OP_GOALS_ID:
			return OPF_AI_GOAL;

		case OP_SET_CARGO:
		case OP_IS_CARGO:
			if (argnum == 0)
				return OPF_CARGO;
			else if (argnum == 1)
				return OPF_SHIP;
			else
				return OPF_SUBSYSTEM;

		case OP_CHANGE_AI_CLASS:
		case OP_IS_AI_CLASS:
			if (argnum == 0)
				return OPF_AI_CLASS;
			else if (argnum == 1)
				return OPF_SHIP;
			else
				return OPF_SUBSYSTEM;

		case OP_IS_SHIP_TYPE:
			if (argnum == 0)
				return OPF_SHIP_TYPE;
			else
				return OPF_SHIP;

		case OP_IS_SHIP_CLASS:
			if (argnum == 0)
				return OPF_SHIP_CLASS_NAME;
			else
				return OPF_SHIP;

		case OP_CHANGE_SOUNDTRACK:
			return OPF_SOUNDTRACK_NAME;

		case OP_PLAY_SOUND_FROM_TABLE:
			return OPF_POSITIVE;

		case OP_PLAY_SOUND_FROM_FILE:
			if (argnum==0)
				return OPF_STRING;
			else
				return OPF_POSITIVE;

		case OP_CLOSE_SOUND_FROM_FILE:
		case OP_ALLOW_TREASON:
		case OP_END_MISSION:
		case OP_SET_DEBRIEFING_TOGGLED:
			return OPF_BOOL;

		case OP_SET_PLAYER_ORDERS:
			if (argnum==0)
				return OPF_SHIP_WING_TEAM;
			if (argnum==1)
				return OPF_BOOL;
			else 
				return OPF_AI_ORDER;

		case OP_SET_SOUND_ENVIRONMENT:
			if (argnum == 0)
				return OPF_SOUND_ENVIRONMENT;

			// fall through
			argnum--;

		case OP_UPDATE_SOUND_ENVIRONMENT:
		{
			// every two, the value repeats
			int a_mod = argnum % 2;

			if (a_mod == 0)
				return OPF_SOUND_ENVIRONMENT_OPTION;
			else
				return OPF_POSITIVE;
		}

		case OP_ADJUST_AUDIO_VOLUME:
		{
			if (argnum == 0)
				return OPF_AUDIO_VOLUME_OPTION;
			else
				return OPF_POSITIVE;
		}

		case OP_SET_EXPLOSION_OPTION:
		{
			// editing a ship
			if (argnum == 0)
				return OPF_SHIP;

			// every two, the value repeats
			int a_mod = (argnum - 1) % 2;

			if (a_mod == 0)
				return OPF_EXPLOSION_OPTION;
			else
				return OPF_POSITIVE;
		}

		case OP_HUD_DISABLE:
		case OP_HUD_DISABLE_EXCEPT_MESSAGES:
			return OPF_POSITIVE;

		case OP_HUD_SET_TEXT:
			return OPF_STRING;

		case OP_HUD_SET_MESSAGE:
			if(argnum == 0)
				return OPF_STRING;
			else
				return OPF_MESSAGE;

		case OP_HUD_SET_TEXT_NUM:
		case OP_HUD_SET_COORDS:
		case OP_HUD_SET_FRAME:
		case OP_HUD_SET_COLOR:
			if(argnum == 0)
				return OPF_STRING;
			else
				return OPF_POSITIVE;

		case OP_HUD_CLEAR_MESSAGES:
			return OPF_NONE;

		case OP_PLAYER_USE_AI:
		case OP_PLAYER_NOT_USE_AI:
			return OPF_NONE;

		case OP_EXPLOSION_EFFECT:
			if (argnum <= 2)
				return OPF_NUMBER;
			else
				return OPF_POSITIVE;

		case OP_WARP_EFFECT:
			if (argnum <= 5)
				return OPF_NUMBER;
			else
				return OPF_POSITIVE;

		case OP_SEND_MESSAGE:
		case OP_SEND_RANDOM_MESSAGE:
			if ( argnum == 0 )
				return OPF_WHO_FROM;
			else if ( argnum == 1 )
				return OPF_PRIORITY;
			else
				return OPF_MESSAGE;

		case OP_SEND_MESSAGE_LIST:
		{
			// every four, the value repeats
			int a_mod = argnum % 4;

			// who from
			if(a_mod == 0)
				return OPF_WHO_FROM;
			else if(a_mod == 1)
				return OPF_PRIORITY;
			else if(a_mod == 2)
				return OPF_MESSAGE;
			else if(a_mod == 3)
				return OPF_POSITIVE;
		}

		case OP_TRAINING_MSG:
			if (argnum < 2)
				return OPF_MESSAGE;
			else
				return OPF_POSITIVE;

		// Karajorma
		case OP_ENABLE_BUILTIN_MESSAGES:
		case OP_DISABLE_BUILTIN_MESSAGES:
				return OPF_WHO_FROM;

		case OP_SET_PERSONA:
			if (argnum == 0) 
				return OPF_PERSONA;
			else
				return OPF_SHIP;

		case OP_SELF_DESTRUCT:
			return OPF_SHIP;

		case OP_NEXT_MISSION:
			return OPF_MISSION_NAME;

		case OP_END_CAMPAIGN:
		case OP_END_OF_CAMPAIGN:
			return OPF_NONE;

		case OP_PREVIOUS_GOAL_TRUE:
		case OP_PREVIOUS_GOAL_FALSE:
			if ( argnum == 0 )
				return OPF_MISSION_NAME;
			else if (argnum == 1 )
				return OPF_GOAL_NAME;
			else
				return OPF_BOOL;

		case OP_PREVIOUS_GOAL_INCOMPLETE:
			return OPF_GOAL_NAME;

		case OP_PREVIOUS_EVENT_TRUE:
		case OP_PREVIOUS_EVENT_FALSE:
		case OP_PREVIOUS_EVENT_INCOMPLETE:
			if (!argnum)
				return OPF_MISSION_NAME;
			else if ( argnum == 1 )
				return OPF_EVENT_NAME;
			else
				return OPF_BOOL;

		case OP_SABOTAGE_SUBSYSTEM:
			if (!argnum)
				return OPF_SHIP;		// changed from OPF_SHIP_NOT_PLAYER by Goober5000: now it can be the player ship also
			else if (argnum == 1 )
				return OPF_SUBSYS_OR_GENERIC;
			else
				return OPF_POSITIVE;

		case OP_REPAIR_SUBSYSTEM:
		case OP_SET_SUBSYSTEM_STRNGTH:
			if (!argnum)
				return OPF_SHIP;		// changed from OPF_SHIP_NOT_PLAYER by Goober5000: now it can be the player ship also
			else if (argnum == 1 )
				return OPF_SUBSYS_OR_GENERIC;
			else if (argnum == 2)
				return OPF_POSITIVE;
			else 
				return OPF_BOOL;

		case OP_WAYPOINTS_DONE:
			if ( argnum == 0 )
				return OPF_SHIP_WING;
			else
				return OPF_WAYPOINT_PATH;

		case OP_WAYPOINTS_DONE_DELAY:
			if ( argnum == 0 )
				return OPF_SHIP_WING;
			else if ( argnum == 1 )
				return OPF_WAYPOINT_PATH;
			else
				return OPF_POSITIVE;

		case OP_INVALIDATE_GOAL:
		case OP_VALIDATE_GOAL:
			return OPF_GOAL_NAME;

		case OP_SHIP_TYPE_DESTROYED:
			if ( argnum == 0 )
				return OPF_POSITIVE;
			else
				return OPF_SHIP_TYPE;

		case OP_KEY_PRESSED:
			if (!argnum)
				return OPF_KEYPRESS;
			else
				return OPF_POSITIVE;

		case OP_KEY_RESET:
		case OP_KEY_RESET_MULTIPLE:
			return OPF_KEYPRESS;

		case OP_EVENT_TRUE:
		case OP_EVENT_FALSE:
			return OPF_EVENT_NAME;

		case OP_EVENT_INCOMPLETE:
		case OP_EVENT_TRUE_DELAY:
		case OP_EVENT_FALSE_DELAY:
		case OP_EVENT_TRUE_MSECS_DELAY:
		case OP_EVENT_FALSE_MSECS_DELAY:
			if (argnum == 0)
				return OPF_EVENT_NAME;
			else if (argnum == 1)
				return OPF_POSITIVE;
			else if (argnum == 2)
				return OPF_BOOL;

		case OP_GOAL_INCOMPLETE:
		case OP_GOAL_TRUE_DELAY:
		case OP_GOAL_FALSE_DELAY:
			if (!argnum)
				return OPF_GOAL_NAME;
			else
				return OPF_POSITIVE;

		case OP_AI_PLAY_DEAD:
		case OP_AI_CHASE_ANY:
			return OPF_POSITIVE;

		case OP_AI_STAY_STILL:
			if (!argnum)
				return OPF_SHIP_POINT;
			else
				return OPF_POSITIVE;

		case OP_AI_FORM_ON_WING:
			return OPF_SHIP;

		case OP_GOOD_REARM_TIME:
			if ( argnum == 0 )
				return OPF_IFF;
			else
				return OPF_POSITIVE;

		case OP_NUM_PLAYERS:
			return OPF_POSITIVE;

		case OP_SKILL_LEVEL_AT_LEAST:
			return OPF_SKILL_LEVEL;

		case OP_GRANT_MEDAL:
		case OP_WAS_MEDAL_GRANTED:
			return OPF_MEDAL_NAME;

		case OP_IS_CARGO_KNOWN:
			return OPF_SHIP;

		case OP_CARGO_KNOWN_DELAY:
			if ( argnum == 0 )
				return OPF_POSITIVE;
			else
				return OPF_SHIP;

		case OP_HAS_BEEN_TAGGED_DELAY:
			if ( argnum == 0 ) {
				return OPF_POSITIVE;
			} else {
				return OPF_SHIP;
			}

		case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
			if ( argnum == 0 ) {
				return OPF_POSITIVE;
			} else if ( argnum == 1 ) {
				return OPF_SHIP;
			} else {
				return OPF_SUBSYSTEM;
			}

		case OP_ALLOW_SHIP:
		case OP_TECH_ADD_SHIP:
			return OPF_SHIP_CLASS_NAME;

		case OP_ALLOW_WEAPON:
		case OP_TECH_ADD_WEAPON:
			return OPF_WEAPON_NAME;

		case OP_TECH_ADD_INTEL:
			return OPF_INTEL_NAME;

		case OP_TECH_RESET_TO_DEFAULT:
			return OPF_NONE;

		case OP_CHANGE_PLAYER_SCORE:
			if (argnum == 0)
				return OPF_NUMBER;
			else 
				return OPF_SHIP;

		case OP_CHANGE_TEAM_SCORE:
			return OPF_NUMBER;

		case OP_SHIP_VAPORIZE:
		case OP_SHIP_NO_VAPORIZE:
			return OPF_SHIP;

		case OP_DONT_COLLIDE_INVISIBLE:
		case OP_COLLIDE_INVISIBLE:
			return OPF_SHIP;

		case OP_SET_MOBILE:
		case OP_SET_IMMOBILE:
			return OPF_SHIP;

		case OP_IGNORE_KEY:
			if (argnum == 0) 
				return OPF_NUMBER;
			else 
				return OPF_KEYPRESS;


		case OP_WARP_BROKEN:
		case OP_WARP_NOT_BROKEN:
		case OP_WARP_NEVER:
		case OP_WARP_ALLOWED:
			return OPF_SHIP;

		case OP_SET_SUBSPACE_DRIVE:
			if (argnum == 0)
				return OPF_BOOL;
			else
				return OPF_SHIP;

		case OP_FLASH_HUD_GAUGE:
			return OPF_HUD_GAUGE_NAME;

		case OP_GOOD_SECONDARY_TIME:
			if ( argnum == 0 )
				return OPF_IFF;
			else if ( argnum == 1 )
				return OPF_POSITIVE;
			else if ( argnum == 2 )
				return OPF_HUGE_WEAPON;
			else
				return OPF_SHIP;

		case OP_PERCENT_SHIPS_ARRIVED:
		case OP_PERCENT_SHIPS_DEPARTED:
		case OP_PERCENT_SHIPS_DESTROYED:
			if ( argnum == 0 ){
				return OPF_POSITIVE;
			} else {
				return OPF_SHIP_WING;
			}
			break;

		case OP_PERCENT_SHIPS_DISARMED:
		case OP_PERCENT_SHIPS_DISABLED:
			if ( argnum == 0 ){
				return OPF_POSITIVE;
			} else {
				return OPF_SHIP;
			}
			break;

		case OP_DEPART_NODE_DELAY:	
			if ( argnum == 0 ){
				return OPF_POSITIVE;
			} else if ( argnum == 1 ){
				return OPF_JUMP_NODE_NAME;
			} else {
				return OPF_SHIP;
			}

		case OP_DESTROYED_DEPARTED_DELAY:
			if ( argnum == 0 ){
				return OPF_POSITIVE;
			} else {
				return OPF_SHIP_WING;
			}

		case OP_JETTISON_CARGO:
			if(argnum == 1){
				return OPF_POSITIVE;
			} else {
				return OPF_SHIP;
			}

		case OP_SET_DOCKED:
			if (argnum == 0) {
				return OPF_SHIP;
			} else if (argnum == 1) {
				return OPF_DOCKER_POINT;
			} else if (argnum == 2) {
				return OPF_SHIP;
			} else {
				return OPF_DOCKEE_POINT;
			}

		case OP_CARGO_NO_DEPLETE:
			if (argnum == 0) {
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}

		case OP_BEAM_FIRE:
			switch(argnum) {
				case 0:
					return OPF_SHIP;
				case 1:
					return OPF_SUBSYSTEM;
				case 2:
					return OPF_SHIP;
				case 3:
					return OPF_SUBSYSTEM;
				case 4:
					return OPF_BOOL;
			}

		case OP_BEAM_FIRE_COORDS:
			switch(argnum) {
				case 0:
					return OPF_SHIP;
				case 1:
					return OPF_SUBSYSTEM;
				case 5:
					return OPF_BOOL;
				default:
					return OPF_NUMBER;
			}

		case OP_IS_TAGGED:
			return OPF_SHIP;

		case OP_IS_PLAYER:
			if (argnum == 0) {
				return OPF_BOOL;
			} else {
				return OPF_SHIP;
			}

		case OP_NUM_KILLS:
		case OP_NUM_ASSISTS:
		case OP_SHIP_SCORE:
		case OP_SHIP_DEATHS: 
		case OP_RESPAWNS_LEFT:
			return OPF_SHIP;

		case OP_SET_RESPAWNS:
			if (argnum == 0 ) {
				return OPF_POSITIVE;
			}
			else {
				return OPF_SHIP;
			}

		case OP_NUM_TYPE_KILLS:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_SHIP_TYPE;
			}

		case OP_NUM_CLASS_KILLS:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_SHIP_CLASS_NAME;
			}

		case OP_BEAM_FREE:
		case OP_BEAM_LOCK:
		case OP_TURRET_FREE:
		case OP_TURRET_LOCK:
		case OP_TURRET_TAGGED_SPECIFIC:
		case OP_TURRET_TAGGED_CLEAR_SPECIFIC:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_SUBSYSTEM;
			}

		case OP_TURRET_SUBSYS_TARGET_DISABLE:
		case OP_TURRET_SUBSYS_TARGET_ENABLE:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_SUBSYS_OR_GENERIC;
			}
		
		case OP_TURRET_CHANGE_WEAPON:
			if(argnum == 0) {
				return OPF_SHIP;
			} else if(argnum == 1) {
				return OPF_SUBSYSTEM;
			} else if(argnum == 2) {
				return OPF_WEAPON_NAME;
			} else if(argnum > 2) {
				return OPF_POSITIVE;
			}

		case OP_TURRET_SET_DIRECTION_PREFERENCE:
			if(argnum == 0) {
				return OPF_SHIP;
			} else if(argnum == 1) {
				return OPF_NUMBER;
			} else {
				return OPF_SUBSYSTEM;
			}

		case OP_TURRET_SET_RATE_OF_FIRE:
			if(argnum == 0) {
				return OPF_SHIP;
			} else if(argnum == 1) {
				return OPF_NUMBER;
			} else {
				return OPF_SUBSYSTEM;
			}

		case OP_TURRET_SET_OPTIMUM_RANGE:
			if(argnum == 0) {
				return OPF_SHIP;
			} else if(argnum == 1) {
				return OPF_NUMBER;
			} else {
				return OPF_SUBSYSTEM;
			}

		case OP_TURRET_SET_TARGET_PRIORITIES:
			if(argnum == 0) {
				return OPF_SHIP;
			} else if(argnum == 1) {
				return OPF_SUBSYSTEM;
			} else if(argnum == 2) {
				return OPF_BOOL;
			} else {
				return OPF_TARGET_PRIORITIES;
			}

		case OP_SET_ARMOR_TYPE:
			if(argnum == 0) {
				return OPF_SHIP;
			} else if(argnum == 1) {
				return OPF_BOOL;
			} else if(argnum == 2) {
				return OPF_ARMOR_TYPE;
			} else {
				return OPF_SUBSYSTEM;
			}

		case OP_WEAPON_SET_DAMAGE_TYPE:
			if(argnum == 0) {
				return OPF_BOOL;
			} else if(argnum == 1) {
				return OPF_DAMAGE_TYPE;
			} else if(argnum == 2) {
				return OPF_BOOL;
			} else {
				return OPF_WEAPON_NAME;
			}

		case OP_SHIP_SET_DAMAGE_TYPE:
			if(argnum == 0) {
				return OPF_BOOL;
			} else if(argnum == 1) {
				return OPF_DAMAGE_TYPE;
			} else if(argnum == 2) {
				return OPF_BOOL;
			} else {
				return OPF_SHIP;
			}

		case OP_SHIP_SHOCKWAVE_SET_DAMAGE_TYPE:
			if(argnum == 0) {
				return OPF_DAMAGE_TYPE;
			} else if(argnum == 1) {
				return OPF_BOOL;
			} else {
				return OPF_SHIP_CLASS_NAME;
			}

		case OP_FIELD_SET_DAMAGE_TYPE:
			if(argnum == 0) {
				return OPF_DAMAGE_TYPE;
			} else {
				return OPF_BOOL;
			}

		case OP_TURRET_SET_TARGET_ORDER:
			if(argnum == 0) {
				return OPF_SHIP;
			} else if(argnum == 1) {
				return OPF_SUBSYSTEM;
			} else {
				return OPF_TURRET_TARGET_ORDER;
			}

		case OP_SHIP_TURRET_TARGET_ORDER:
			if(argnum == 0) {
				return OPF_SHIP;
			} else {
				return OPF_TURRET_TARGET_ORDER;
			}

		case OP_LOCK_ROTATING_SUBSYSTEM:
		case OP_FREE_ROTATING_SUBSYSTEM:
		case OP_REVERSE_ROTATING_SUBSYSTEM:
			if (argnum == 0)
				return OPF_SHIP;
			else
				return OPF_ROTATING_SUBSYSTEM;

		case OP_ROTATING_SUBSYS_SET_TURN_TIME:
			if (argnum == 0)
				return OPF_SHIP;
			else if (argnum == 1)
				return OPF_ROTATING_SUBSYSTEM;
			else if (argnum == 2)
				return OPF_NUMBER;
			else
				return OPF_POSITIVE;

		case OP_TRIGGER_SUBMODEL_ANIMATION:
			if (argnum == 0)
				return OPF_SHIP;
			else if (argnum == 1)
				return OPF_ANIMATION_TYPE;
			else if (argnum == 2 || argnum == 3)
				return OPF_NUMBER;
			else if (argnum == 4)
				return OPF_BOOL;
			else if (argnum == 5)
				return OPF_SUBSYSTEM;

		case OP_BEAM_FREE_ALL:
		case OP_BEAM_LOCK_ALL:
		case OP_TURRET_FREE_ALL:
		case OP_TURRET_LOCK_ALL:
		case OP_TURRET_TAGGED_ONLY_ALL:
		case OP_TURRET_TAGGED_CLEAR_ALL:
			return OPF_SHIP;

		case OP_ADD_REMOVE_ESCORT:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}

		case OP_AWACS_SET_RADIUS:
			if(argnum == 0){
				return OPF_SHIP;
			} else if(argnum == 1){
				return OPF_AWACS_SUBSYSTEM;
			} else {
				return OPF_NUMBER;
			}

		case OP_PRIMITIVE_SENSORS_SET_RANGE:
			if (!argnum)
				return OPF_SHIP;
			else
				return OPF_NUMBER;

		case OP_CAP_WAYPOINT_SPEED:
			if (argnum == 0) {
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}

		case OP_SUBSYS_SET_RANDOM:
			if (argnum == 0) {
				return OPF_SHIP;
			} else if (argnum == 1 || argnum == 2) {
				return OPF_NUMBER;
			} else {
				return OPF_SUBSYSTEM;
			}

		case OP_SUPERNOVA_START:
			return OPF_POSITIVE;

		case OP_SHIELD_RECHARGE_PCT:
		case OP_WEAPON_RECHARGE_PCT:
		case OP_ENGINE_RECHARGE_PCT:
			return OPF_SHIP;			

		case OP_SHIELD_QUAD_LOW:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}

		case OP_PRIMARY_AMMO_PCT:
		case OP_SECONDARY_AMMO_PCT:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}
			
		// Karajorma
		case OP_GET_PRIMARY_AMMO:
		case OP_SET_PRIMARY_AMMO:
		case OP_GET_SECONDARY_AMMO:
		case OP_SET_SECONDARY_AMMO:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}
		
		// Karajorma
		case OP_SET_PRIMARY_WEAPON:
		case OP_SET_SECONDARY_WEAPON:
			if(argnum == 0)
			{
				return OPF_SHIP;
			}
				
			else if (argnum == 2)
			{
				return OPF_WEAPON_NAME;
			}
			else 
			{
				return OPF_NUMBER;
			}

			
		case OP_GET_NUM_COUNTERMEASURES:
			return OPF_SHIP;

		case OP_SET_NUM_COUNTERMEASURES:
			if (argnum == 0)
				return OPF_SHIP;
			else 
				return OPF_POSITIVE;

		// Karajorma	
		case OP_LOCK_PRIMARY_WEAPON:
		case OP_UNLOCK_PRIMARY_WEAPON:
		case OP_LOCK_SECONDARY_WEAPON:
		case OP_UNLOCK_SECONDARY_WEAPON:
		// KeldorKatarn		
		case OP_LOCK_AFTERBURNER:
		case OP_UNLOCK_AFTERBURNER:
			return OPF_SHIP;

		
		case OP_SET_AFTERBURNER_ENERGY: 
		case OP_SET_WEAPON_ENERGY:
		case OP_SET_SHIELD_ENERGY:
			if (argnum == 0) {
				return OPF_POSITIVE;
			}
			else {
				return OPF_SHIP;
			}

		case OP_SET_AMBIENT_LIGHT:
			return OPF_POSITIVE;
			
		case OP_SET_POST_EFFECT:
			if (argnum == 0)
				return OPF_POST_EFFECT;
			else
				return OPF_POSITIVE;

		case OP_CHANGE_SUBSYSTEM_NAME:
			if (argnum == 0) {
				return OPF_SHIP;
			}
			else if (argnum == 1) {
				return OPF_STRING;
			}
			else {
				return OPF_SUBSYSTEM;
			}
		
		case OP_IS_SECONDARY_SELECTED:
		case OP_IS_PRIMARY_SELECTED:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}

		case OP_DAMAGED_ESCORT_LIST:
			if (argnum < 2)
			{
				return OPF_NUMBER;
			}
			else
			{
				return OPF_SHIP;
			}

		case OP_DAMAGED_ESCORT_LIST_ALL:
			return OPF_POSITIVE;

		case OP_CHANGE_SHIP_CLASS:
			if (!argnum)
				return OPF_SHIP_CLASS_NAME;
			else
				return OPF_SHIP;

		case OP_SHIP_COPY_DAMAGE:
			return OPF_SHIP;

		case OP_DEACTIVATE_GLOW_POINTS:	//-Bobboau
		case OP_ACTIVATE_GLOW_POINTS:	//-Bobboau
		case OP_DEACTIVATE_GLOW_MAPS:	//-Bobboau
		case OP_ACTIVATE_GLOW_MAPS:		//-Bobboau
			return OPF_SHIP;	//a list of ships that are to be activated/deactivated
		case OP_DEACTIVATE_GLOW_POINT_BANK:
		case OP_ACTIVATE_GLOW_POINT_BANK:
			if (!argnum)
				return OPF_SHIP;		//the ship
			else
				return OPF_POSITIVE;		//the glow bank to disable

		// taylor
		case OP_SET_SKYBOX_MODEL:
			return OPF_SKYBOX_MODEL_NAME;

		case OP_SET_SKYBOX_ORIENT:
			return OPF_NUMBER;

		// Goober5000 - this is complicated :)
		case OP_SET_SUPPORT_SHIP:
			if ((argnum == 0) || (argnum == 2))
				return OPF_DEPARTURE_LOCATION;	// use this for both because we don't want Near Ship or In Front of Ship
			if ((argnum == 1) || (argnum == 3))
				return OPF_SHIP_WITH_BAY;		// same - we don't want to anchor around anything without a docking bay
			if (argnum == 4)
				return OPF_SUPPORT_SHIP_CLASS;
			if (argnum == 5)
				return OPF_NUMBER;
			if (argnum == 6)
				return OPF_NUMBER;
			break;

		// Goober5000
		case OP_SET_ARRIVAL_INFO:
			if (argnum == 0)
				return OPF_SHIP_WING;
			else if (argnum == 1)
				return OPF_ARRIVAL_LOCATION;
			else if (argnum == 2)
				return OPF_ARRIVAL_ANCHOR_ALL;
			else if (argnum == 3)
				return OPF_NUMBER;
			else if (argnum == 4)
				return OPF_POSITIVE;
			else if (argnum == 5)
				return OPF_BOOL;
			break;

		// Goober5000
		case OP_SET_DEPARTURE_INFO:
			if (argnum == 0)
				return OPF_SHIP_WING;
			else if (argnum == 1)
				return OPF_DEPARTURE_LOCATION;
			else if (argnum == 2)
				return OPF_SHIP_WITH_BAY;
			else if (argnum == 3)
				return OPF_NUMBER;
			else if (argnum == 4)
				return OPF_BOOL;
			break;

		case OP_KAMIKAZE:
			if (argnum==0)
				return OPF_POSITIVE;
			else
				return OPF_SHIP_WING;

		case OP_NUM_SHIPS_IN_BATTLE:	//phreak modified by FUBAR
			return OPF_SHIP_WING_TEAM;

		case OP_NUM_SHIPS_IN_WING:	// Karajorma	
			return OPF_WING;

		case OP_CURRENT_SPEED:
			return OPF_SHIP_WING;
			
		case OP_PRIMARY_FIRED_SINCE:
		case OP_SECONDARY_FIRED_SINCE:
			if (argnum == 0) 
				return OPF_SHIP;
			else 
				return OPF_POSITIVE;

		case OP_HAS_PRIMARY_WEAPON:
		case OP_HAS_SECONDARY_WEAPON:
			if (argnum == 0)
				return OPF_SHIP;
			else if (argnum == 1) 
				return OPF_WEAPON_BANK_NUMBER;
			else 
				return OPF_WEAPON_NAME;
			
		case OP_DIRECTIVE_VALUE:
			if (argnum == 0)
				return OPF_NUMBER;
			else 
				return OPF_BOOL;			

		case OP_NAV_IS_VISITED:		//Kazan
		case OP_NAV_DISTANCE:		//kazan
		case OP_NAV_DEL:			//kazan
		case OP_NAV_HIDE:			//kazan
		case OP_NAV_RESTRICT:		//kazan
		case OP_NAV_UNHIDE:			//kazan
		case OP_NAV_UNRESTRICT:		//kazan
		case OP_NAV_SET_VISITED:	//kazan
		case OP_NAV_UNSET_VISITED:	//kazan
			return OPF_STRING;
		
		case OP_NAV_SET_CARRY:		//kazan
		case OP_NAV_UNSET_CARRY:	//kazan
			return OPF_SHIP_WING;

		case OP_NAV_SET_NEEDSLINK: // kazan
		case OP_NAV_UNSET_NEEDSLINK:
		case OP_NAV_ISLINKED:
				return OPF_SHIP;

		case OP_NAV_USECINEMATICS:
		case OP_NAV_USEAP:
			return OPF_BOOL;

		case OP_NAV_ADD_WAYPOINT:	//kazan
			if (argnum==0)
				return OPF_STRING;
			else if (argnum==1)
				return OPF_WAYPOINT_PATH;
			else if (argnum==2)
				return OPF_POSITIVE;
			else 
				return OPF_SHIP_WING_POINT;

		case OP_NAV_ADD_SHIP:		//kazan
			if (argnum==0)
				return OPF_STRING;
			else
				return OPF_SHIP;

		//<Cutscenes>
		case OP_SCRAMBLE_MESSAGES:
		case OP_UNSCRAMBLE_MESSAGES:
		case OP_CUTSCENES_GET_FOV:
			return OPF_NONE;

		case OP_CUTSCENES_SET_CUTSCENE_BARS:
		case OP_CUTSCENES_UNSET_CUTSCENE_BARS:
		case OP_CUTSCENES_FADE_IN:
		case OP_CUTSCENES_FADE_OUT:
		case OP_CUTSCENES_SET_TIME_COMPRESSION:
			return OPF_POSITIVE;

		case OP_CUTSCENES_SET_FOV:
			return OPF_NUMBER;

		case OP_CUTSCENES_SET_CAMERA:
			return OPF_STRING;
		
		case OP_CUTSCENES_SET_CAMERA_POSITION:
		case OP_CUTSCENES_SET_CAMERA_FACING:
		case OP_CUTSCENES_SET_CAMERA_ROTATION:
			if(argnum < 3)
				return OPF_NUMBER;
			else
				return OPF_POSITIVE;

		case OP_CUTSCENES_SET_CAMERA_FACING_OBJECT:
			if(argnum < 1)
				return OPF_SHIP_WING_POINT;
			else
				return OPF_POSITIVE;

		case OP_CUTSCENES_SET_CAMERA_FOV:
			return OPF_POSITIVE;
		
		case OP_CUTSCENES_SET_CAMERA_HOST:
		case OP_CUTSCENES_SET_CAMERA_TARGET:
			if(argnum < 1)
				return OPF_SHIP_WING_POINT_OR_NONE;
			else
				return OPF_SUBSYSTEM_OR_NONE;

		case OP_CUTSCENES_RESET_CAMERA:
			return OPF_BOOL;

		case OP_CUTSCENES_RESET_FOV:
		case OP_CUTSCENES_RESET_TIME_COMPRESSION:
			return OPF_NONE;

		case OP_CUTSCENES_FORCE_PERSPECTIVE:
			if(argnum==0)
				return OPF_BOOL;
			else
				return OPF_POSITIVE;

		case OP_SET_CAMERA_SHUDDER:
			return OPF_POSITIVE;

		case OP_CUTSCENES_SHOW_SUBTITLE:
			if(argnum < 2)
				return OPF_NUMBER;
			else if (argnum == 2)
				return OPF_STRING;
			else if (argnum == 3)
				return OPF_POSITIVE;
			else if (argnum == 4)
				return OPF_STRING;
			else if (argnum == 5)
				return OPF_POSITIVE;
			else if (argnum < 8)
				return OPF_BOOL;
			else if (argnum < 12)
				return OPF_POSITIVE;
			else if (argnum == 12 )
				return OPF_BOOL;

		case OP_CUTSCENES_SHOW_SUBTITLE_TEXT:
			if (argnum == 0)
				return OPF_STRING;
			else if (argnum == 1 || argnum == 2)
				return OPF_NUMBER;
			else if (argnum == 3 || argnum == 4)
				return OPF_BOOL;
			else if (argnum >= 5 && argnum <= 10)
				return OPF_POSITIVE;
			else if (argnum == 11)
				return OPF_FONT;
			else if (argnum == 12)
				return OPF_BOOL;

		case OP_CUTSCENES_SHOW_SUBTITLE_IMAGE:
			if (argnum == 0)
				return OPF_STRING;
			else if (argnum == 1 || argnum == 2)
				return OPF_NUMBER;
			else if (argnum == 3 || argnum == 4)
				return OPF_BOOL;
			else if (argnum >= 5 && argnum <= 8)
				return OPF_POSITIVE;
			else if (argnum == 9)
				return OPF_BOOL;

		//</Cutscenes>

		case OP_JUMP_NODE_SET_JUMPNODE_NAME: //CommanderDJ
			if(argnum==0)
				return OPF_JUMP_NODE_NAME;
			else if (argnum==1)
				return OPF_STRING;

		case OP_JUMP_NODE_SET_JUMPNODE_COLOR:
			if(argnum==0)
				return OPF_JUMP_NODE_NAME;
			else
				return OPF_POSITIVE;

		case OP_JUMP_NODE_SET_JUMPNODE_MODEL:
			if(argnum==0)
				return OPF_JUMP_NODE_NAME;
			else if (argnum == 1)
				return OPF_STRING;
			else
				return OPF_BOOL;

		case OP_JUMP_NODE_SHOW_JUMPNODE:
		case OP_JUMP_NODE_HIDE_JUMPNODE:
				return OPF_JUMP_NODE_NAME;

		case OP_ADD_BACKGROUND_BITMAP:
			if (argnum == 0)
				return OPF_BACKGROUND_BITMAP;
			else if (argnum == 8) return OPF_VARIABLE_NAME;
			else return OPF_POSITIVE;

		case OP_REMOVE_BACKGROUND_BITMAP:
			return OPF_POSITIVE;

		case OP_ADD_SUN_BITMAP:
			if (argnum == 0)
				return OPF_SUN_BITMAP;
			else if (argnum == 5) return OPF_VARIABLE_NAME;
			else return OPF_POSITIVE;

		case OP_REMOVE_SUN_BITMAP:
			return OPF_POSITIVE;

		case OP_NEBULA_CHANGE_STORM:
			return OPF_NEBULA_STORM_TYPE;

		case OP_NEBULA_TOGGLE_POOF:
			if (argnum == 1) return OPF_BOOL;
			else return OPF_NEBULA_POOF;

		case OP_SCRIPT_EVAL_NUM:
		case OP_SCRIPT_EVAL_STRING:
		case OP_SCRIPT_EVAL:
			return OPF_STRING;

		case OP_CHANGE_IFF_COLOR:
			if ((argnum == 0) || (argnum == 1))
				return OPF_IFF;
			else if ((argnum >= 2) && (argnum <=4))
				return OPF_POSITIVE;
			else return OPF_SHIP_WING;

		case OP_HUD_DISPLAY_GAUGE:
			if ( argnum == 0 ) {
				return OPF_POSITIVE;
			} else {
				return OPF_HUD_ELEMENT;
			}

		case OP_DISABLE_ETS:
		case OP_ENABLE_ETS:
				return OPF_SHIP;

		case OP_IS_FACING:
			if (argnum < 2)
				return OPF_SHIP;
			else
				return OPF_POSITIVE;

		case OP_FORCE_GLIDE:
			if (argnum == 0)
				return OPF_SHIP;
			else
				return OPF_BOOL;

		case OP_HUD_SET_DIRECTIVE:
			return OPF_STRING;

		case OP_HUD_GAUGE_SET_ACTIVE:
			if (argnum == 0)
				return OPF_STRING;
			else
				return OPF_BOOL;

		case OP_HUD_ACTIVATE_GAUGE_TYPE:
			if (argnum == 0)
				return OPF_HUD_GAUGE;
			else
				return OPF_BOOL;

		case OP_GET_COLGROUP_ID:
			return OPF_SHIP;

		case OP_ADD_TO_COLGROUP:
			if (argnum == 0)
				return OPF_SHIP;
			else
				return OPF_POSITIVE;

		case OP_REMOVE_FROM_COLGROUP:
			if (argnum == 0)
				return OPF_SHIP;
			else
				return OPF_POSITIVE;

		case OP_SHIP_EFFECT:
			if (argnum == 0)
				return OPF_SHIP_EFFECT;
			else if (argnum == 1)
				return OPF_NUMBER;
			else
				return OPF_SHIP_WING;

		case OP_CLEAR_SUBTITLES:
			return OPF_NONE;

		case OP_SET_THRUSTERS:
			if (argnum == 0)
				return OPF_BOOL;
			else
				return OPF_SHIP;

		default:
			Int3();
	}

	return 0;
}

// DA: 1/7/99  Used to rename ships and waypoints, not variables
// Strictly used in FRED
void update_sexp_references(const char *old_name, const char *new_name)
{
	int i;

	Assert(strlen(new_name) < TOKEN_LENGTH);
	for (i = 0; i < Num_sexp_nodes; i++)
	{
		if ((SEXP_NODE_TYPE(i) == SEXP_ATOM) && (Sexp_nodes[i].subtype == SEXP_ATOM_STRING))
			if (!stricmp(CTEXT(i), old_name))
				strcpy(CTEXT(i), new_name);
	}
}

// DA: 1/7/99  Used to rename event names, goal names, not variables
// Strictly used in FRED
void update_sexp_references(const char *old_name, const char *new_name, int format)
{
	int i;
	if (!strcmp(old_name, new_name)) {
		return;
	}

	Assert(strlen(new_name) < TOKEN_LENGTH);
	for (i = 0; i < Num_sexp_nodes; i++)
	{
		if (is_sexp_top_level(i))
			update_sexp_references(old_name, new_name, format, i);
	}
}

// DA: 1/7/99  Used to rename event names, goal names, not variables
// recursive function to update references to a certain type of data
void update_sexp_references(const char *old_name, const char *new_name, int format, int node)
{
	int i, n, op;

	if (node < 0)
		return;

	if ((SEXP_NODE_TYPE(node) == SEXP_LIST) && (Sexp_nodes[node].subtype == SEXP_ATOM_LIST))
	{
		if (Sexp_nodes[node].first)
			update_sexp_references(old_name, new_name, format, Sexp_nodes[node].first);

		if (Sexp_nodes[node].rest)
			update_sexp_references(old_name, new_name, format, Sexp_nodes[node].rest);

		return;
	}

	if (SEXP_NODE_TYPE(node) != SEXP_ATOM)
		return;

	if (Sexp_nodes[node].subtype != SEXP_ATOM_OPERATOR)
		return;

	op = get_operator_index(CTEXT(node));
	Assert(Sexp_nodes[node].first < 0);
	n = Sexp_nodes[node].rest;
	i = 0;
	while (n >= 0)
	{
		if (SEXP_NODE_TYPE(n) == SEXP_LIST)
		{
			update_sexp_references(old_name, new_name, format, Sexp_nodes[n].first);
		}
		else
		{
			Assert((SEXP_NODE_TYPE(n) == SEXP_ATOM) && ((Sexp_nodes[n].subtype == SEXP_ATOM_NUMBER) || (Sexp_nodes[n].subtype == SEXP_ATOM_STRING)));

			if (query_operator_argument_type(op, i) == format)
			{
				if (!stricmp(CTEXT(n), old_name))
					strcpy(CTEXT(n), new_name);
			}
		}

		n = Sexp_nodes[n].rest;
		i++;
	}
}

int query_referenced_in_sexp(int mode, char *name, int *node)
{
	int i, n, j;

	for (n=0; n<Num_sexp_nodes; n++){
		if ((SEXP_NODE_TYPE(n) == SEXP_ATOM) && (Sexp_nodes[n].subtype == SEXP_ATOM_STRING)){
			if (!stricmp(CTEXT(n), name)){
				break;
			}
		}
	}

	if (n == Num_sexp_nodes){
		return 0;
	}

	if (node){
		*node = n;
	}

	// so we know it's being used somewhere..  Time to find out where..
	for (i=0; i<MAX_SHIPS; i++)
		if (Ships[i].objnum >= 0) {
			if (query_node_in_sexp(n, Ships[i].arrival_cue)){
				return i | SRC_SHIP_ARRIVAL;
			}
			if (query_node_in_sexp(n, Ships[i].departure_cue)){
				return i | SRC_SHIP_DEPARTURE;
			}
		}

	for (i=0; i<MAX_WINGS; i++){
		if (Wings[i].wave_count) {
			if (query_node_in_sexp(n, Wings[i].arrival_cue)){
				return i | SRC_WING_ARRIVAL;
			}
			if (query_node_in_sexp(n, Wings[i].departure_cue)){
				return i | SRC_WING_DEPARTURE;
			}
		}
	}

	for (i=0; i<Num_mission_events; i++){
		if (query_node_in_sexp(n, Mission_events[i].formula)){
			return i | SRC_EVENT;
		}
	}

	for (i=0; i<Num_goals; i++){
		if (query_node_in_sexp(n, Mission_goals[i].formula)){
			return i | SRC_MISSION_GOAL;
		}
	}

	for (j=0; j<Num_teams; j++) {
		for (i=0; i<Debriefings[j].num_stages; i++) {
			if (query_node_in_sexp(n, Debriefings[j].stages[i].formula)){
				return i | SRC_DEBRIEFING;
			}
		}
	}

	for (j=0; j<Num_teams; j++) {
		for (i=0; i<Briefings[j].num_stages; i++) {
			if (query_node_in_sexp(n, Briefings[j].stages[i].formula)){
				return i | SRC_BRIEFING;
			}
		}
	}

	return SRC_UNKNOWN;
}

int verify_vector(char *text)
{
	char *str;
	int len = 0;

	if (text == NULL)
		return -1;

	len = strlen(text);
	if (text[0] != '(' || text[len - 1] != ')'){
		return -1;
	}

	str = &text[0];
	skip_white(&str);
	if (*str != '('){
		return -1;
	}

	str++;
	skip_white(&str);
	if (validate_float(&str)){
		return -1;
	}

	skip_white(&str);
	if (validate_float(&str)){
		return -1;
	}

	skip_white(&str);
	if (validate_float(&str)){
		return -1;
	}

	skip_white(&str);
	if (*str != ')'){
		return -1;
	}

	str++;
	skip_white(&str);
	if (*str){
		return -1;
	}

	return 0;
}

void skip_white(char **str)
{
	if ((**str == ' ') || (**str == '\t')){
		(*str)++;
	}
}

int validate_float(char **str)
{
	int count = 0, dot = 0;

	while (isdigit(**str) || **str == '.') {
		if (**str == '.') {
			if (dot){
				return -1;
			}

			dot = 1;
		}

		(*str)++;
		count++;
	}

	if (!count){
		return -1;
	}

	return 0;
}

/**
 * Check if operator return type opr is a valid match for operator argument type opf
 */
int sexp_query_type_match(int opf, int opr)
{
	switch (opf) {
		case OPF_NUMBER:
			return ((opr == OPR_NUMBER) || (opr == OPR_POSITIVE));

		case OPF_POSITIVE:
			// Goober5000's number hack
			return ((opr == OPR_POSITIVE) || (opr == OPR_NUMBER));

		case OPF_BOOL:
			return (opr == OPR_BOOL);

		case OPF_NULL:
			return (opr == OPR_NULL);

		// Goober5000
		case OPF_FLEXIBLE_ARGUMENT:
			return (opr == OPR_FLEXIBLE_ARGUMENT);

		// Goober5000
		case OPF_ANYTHING:
			// don't match any operators, only data
			return 0;

		case OPF_AI_GOAL:
			return (opr == OPR_AI_GOAL);
	}

	return 0;
}

char *sexp_error_message(int num)
{
	switch (num) {
		case SEXP_CHECK_NONOP_ARGS:
			return "Data shouldn't have arguments";

		case SEXP_CHECK_OP_EXPECTED:
			return "Operator expected instead of data";

		case SEXP_CHECK_UNKNOWN_OP:
			return "Unrecognized operator";

		case SEXP_CHECK_TYPE_MISMATCH:
			return "Argument type mismatch";

		case SEXP_CHECK_BAD_ARG_COUNT:
			return "Argument count is illegal";

		case SEXP_CHECK_UNKNOWN_TYPE:
			return "Unknown operator argument type";

		case SEXP_CHECK_INVALID_NUM:
			return "Not a number";

		case SEXP_CHECK_INVALID_SHIP:
			return "Invalid ship name";

		case SEXP_CHECK_INVALID_WING:
			return "Invalid wing name";

		case SEXP_CHECK_INVALID_SUBSYS:
			return "Invalid subsystem name";

		case SEXP_CHECK_INVALID_SUBSYS_TYPE:
			return "Invalid subsystem type";

		case SEXP_CHECK_INVALID_IFF:
			return "Invalid team name";

		case SEXP_CHECK_INVALID_AI_CLASS:
			return "Invalid AI class name";

		case SEXP_CHECK_INVALID_POINT:
			return "Invalid point";

		case SEXP_CHECK_NEGATIVE_NUM:
			return "Negative number not allowed";

		case SEXP_CHECK_INVALID_SHIP_WING:
			return "Invalid ship/wing name";

		case SEXP_CHECK_INVALID_SHIP_TYPE:
			return "Invalid ship type";

		case SEXP_CHECK_UNKNOWN_MESSAGE:
			return "Invalid message name";

		case SEXP_CHECK_INVALID_PRIORITY:
			return "Invalid priority";

		case SEXP_CHECK_INVALID_MISSION_NAME:
			return "Invalid mission filename";

		case SEXP_CHECK_INVALID_GOAL_NAME:
			return "Invalid goal name";

		case SEXP_CHECK_INVALID_LEVEL:
			return "Mission level too low in tree";

		case SEXP_CHECK_INVALID_MSG_SOURCE:
			return "Invalid message source";

		case SEXP_CHECK_INVALID_DOCKER_POINT:
			return "Invalid docker point";

		case SEXP_CHECK_INVALID_DOCKEE_POINT:
			return "Invalid dockee point";

		case SEXP_CHECK_ORDER_NOT_ALLOWED:
			return "Ship not allowed to have this order";

		case SEXP_CHECK_DOCKING_NOT_ALLOWED:
			return "Ship can't dock with target ship";

		case SEXP_CHECK_NUM_RANGE_INVALID:
			return "Number is out of range";

		case SEXP_CHECK_INVALID_EVENT_NAME:
			return "Event name is invalid (not known)";

		case SEXP_CHECK_INVALID_SKILL_LEVEL:
			return "Skill level name is invalid (not known)";

		case SEXP_CHECK_INVALID_MEDAL_NAME:
			return "Invalid medal name";

		case SEXP_CHECK_INVALID_WEAPON_NAME:
			return "Invalid weapon name";

		case SEXP_CHECK_INVALID_INTEL_NAME:
			return "Invalid intel name";

		case SEXP_CHECK_INVALID_SHIP_CLASS_NAME:
			return "Invalid ship class name";

		case SEXP_CHECK_INVALID_GAUGE_NAME:
			return "Invalid HUD gauges name";

		case SEXP_CHECK_INVALID_JUMP_NODE:
			return "Invalid Jump Node name";

		case SEXP_CHECK_UNKNOWN_ERROR:
			return "Unknown error";

		case SEXP_CHECK_INVALID_SUPPORT_SHIP_CLASS:
			return "Invalid support ship class";

		case SEXP_CHECK_INVALID_SHIP_WITH_BAY:
			return "Ship does not have a fighter bay";

		case SEXP_CHECK_INVALID_ARRIVAL_LOCATION:
			return "Invalid arrival location";

		case SEXP_CHECK_INVALID_DEPARTURE_LOCATION:
			return "Invalid departure location";

		case SEXP_CHECK_INVALID_ARRIVAL_ANCHOR_ALL:
			return "Invalid universal arrival anchor";

		case SEXP_CHECK_INVALID_SOUNDTRACK_NAME:
			return "Invalid soundtrack name";

		case SEXP_CHECK_INVALID_PERSONA_NAME:
			return "Invalid persona name";

		case SEXP_CHECK_INVALID_VARIABLE:
			return "Invalid variable name"; 

		case SEXP_CHECK_INVALID_VARIABLE_TYPE:
			return "Invalid variable type";

		case SEXP_CHECK_INVALID_FONT:
			return "Invalid font";

		case SEXP_CHECK_INVALID_HUD_ELEMENT:
			return "Invalid hud element magic name";

		case SEXP_CHECK_INVALID_SOUND_ENVIRONMENT:
			return "Invalid sound environment";

		case SEXP_CHECK_INVALID_SOUND_ENVIRONMENT_OPTION:
			return "Invalid sound environment option";

		case SEXP_CHECK_INVALID_AUDIO_VOLUME_OPTION:
			return "Invalid audio volume option";

		case SEXP_CHECK_INVALID_EXPLOSION_OPTION:
			return "Invalid explosion option";

		case SEXP_CHECK_INVALID_SHIP_EFFECT:
			return "Invalid ship effect name";

		case SEXP_CHECK_INVALID_TURRET_TARGET_ORDER:
			return "Invalid turret target order";

		case SEXP_CHECK_INVALID_ARMOR_TYPE:
			return "Invalid armor type";

		case SEXP_CHECK_INVALID_DAMAGE_TYPE:
			return "Invalid damage type";

		case SEXP_CHECK_INVALID_HUD_GAUGE:
			return "Invalid hud gauge";

		case SEXP_CHECK_INVALID_TARGET_PRIORITIES:
			return "Invalid target priorities";
			
		case SEXP_CHECK_INVALID_ANIMATION_TYPE:
			return "Invalid animation type";

		default:
			Warning(LOCATION, "Unhandled sexp error code %d!", num);
			return "Unhandled sexp error code!";
	}
}

int query_sexp_ai_goal_valid(int sexp_ai_goal, int ship)
{
	int i, op;

	for (op=0; op<Num_operators; op++)
		if (Operators[op].value == sexp_ai_goal)
			break;

	Assert(op < Num_operators);
	for (i=0; i<Num_sexp_ai_goal_links; i++)
		if (Sexp_ai_goal_links[i].op_code == sexp_ai_goal)
			break;

	Assert(i < Num_sexp_ai_goal_links);
	return ai_query_goal_valid(ship, Sexp_ai_goal_links[i].ai_goal);
}

// Takes an Sexp_node.text pointing to a variable (of form "Sexp_variables[xx]=string" or "Sexp_variables[xx]=number")
// and returns the index into the Sexp_variables array of the actual value 
int extract_sexp_variable_index(int node)
{
	char *text = Sexp_nodes[node].text;
	char char_index[8];
	char *start_index;
	int variable_index;

	// get past the '['
	start_index = text + 15;
	Assert(isdigit(*start_index));

	int len = 0;

	while ( *start_index != ']' ) {
		char_index[len++] = *(start_index++);
		Assert(len < 3);
	}

	Assert(len > 0);
	char_index[len] = 0;	// append null termination to string

	variable_index = atoi(char_index);
	Assert( (variable_index >= 0) && (variable_index < MAX_SEXP_VARIABLES) );

	return variable_index;
}


/**
 * Wrapper around Sexp_node[xx].text for normal and variable
 */
char *CTEXT(int n)
{
	int sexp_variable_index = -1;
	char variable_name[TOKEN_LENGTH];
	char *current_argument; 

	if ( n < 0 ) {
		Int3();
		return NULL;
	}

	// Goober5000 - MWAHAHAHAHAHAHAHA!  Thank you, Volition programmers!  Without
	// the CTEXT wrapper, when-argument would probably be infeasibly difficult to code.
	if (!strcmp(Sexp_nodes[n].text, SEXP_ARGUMENT_STRING))
	{
		if (Fred_running)
		{
			// CTEXT is used when writing sexps to savefiles, so don't translate the argument
			return Sexp_nodes[n].text;
		}
		else
		{
			// make sure we have an argument to replace it with
			if (Sexp_replacement_arguments.empty())
				return Sexp_nodes[n].text;
		}

		// if the replacement argument is a variable name, get the variable index
		sexp_variable_index = get_index_sexp_variable_name(Sexp_replacement_arguments.back());

		current_argument = Sexp_replacement_arguments.back();

		// if the replacement argument is a formatted variable name, get the variable index
		if (current_argument[0] == SEXP_VARIABLE_CHAR)
		{
			get_unformatted_sexp_variable_name(variable_name, current_argument);
			sexp_variable_index = get_index_sexp_variable_name(variable_name);
		}

		// if we have a variable, return the variable value, else return the regular argument
		if (sexp_variable_index != -1)
			return Sexp_variables[sexp_variable_index].text;
		else
			return Sexp_replacement_arguments.back();
	}

	// Goober5000 - if not special argument, proceed as normal
	if (Sexp_nodes[n].type & SEXP_FLAG_VARIABLE)
	{
		if (Fred_running)
		{
			sexp_variable_index = get_index_sexp_variable_name(Sexp_nodes[n].text);
			Assert(sexp_variable_index != -1);
		}
		else
		{
			sexp_variable_index = atoi(Sexp_nodes[n].text);
		}
		// Reference a Sexp_variable
		// string format -- "Sexp_variables[xx]=number" or "Sexp_variables[xx]=string", where xx is the index

		Assert( !(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_NOT_USED) );
		Assert(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_SET);

		return Sexp_variables[sexp_variable_index].text;
	}
	else
	{
		return Sexp_nodes[n].text;
	}
}


/**
 * Set all Sexp_variables to type uninitialized
 */
void init_sexp_vars()
{
	for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
		Sexp_variables[i].type = SEXP_VARIABLE_NOT_USED;
		Block_variables[i].type = SEXP_VARIABLE_NOT_USED;
	}
}

/**
 * Add a variable to the block variable array rather than the Sexp_variables array
 */
void add_block_variable(const char *text, const char *var_name, int type, int index)
{
	Assert( (index >= 0) && (index < MAX_SEXP_VARIABLES) );

	strcpy_s(Block_variables[index].text, text);
	strcpy_s(Block_variables[index].variable_name, var_name);
	Block_variables[index].type &= ~SEXP_VARIABLE_NOT_USED;
	Block_variables[index].type = (type | SEXP_VARIABLE_SET);
	
}

/**
 * Add a Sexp_variable to be used in a mission.
 *
 * This should be called from within mission parse.
 */
int sexp_add_variable(const char *text, const char *var_name, int type, int index)
{
	// if index == -1, find next open slot
	if (index == -1) {
		for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
			if (Sexp_variables[i].type == SEXP_VARIABLE_NOT_USED) {
				index = i;
				break;
			}
		}
	} else {
		Assert( (index >= 0) && (index < MAX_SEXP_VARIABLES) );
	}

	if (index >= 0) {
		strcpy_s(Sexp_variables[index].text, text);
		strcpy_s(Sexp_variables[index].variable_name, var_name);
		Sexp_variables[index].type &= ~SEXP_VARIABLE_NOT_USED;
		Sexp_variables[index].type = (type | SEXP_VARIABLE_SET);
	}

	return index;
}

// Goober5000 - minor variant of the above that is now required for variable arrays
void sexp_add_array_block_variable(int index, bool is_numeric)
{
	Assert(index >= 0 && index < MAX_SEXP_VARIABLES);

	strcpy_s(Sexp_variables[index].text, "");
	strcpy_s(Sexp_variables[index].variable_name, "variable array block");

	if (is_numeric)
		Sexp_variables[index].type = SEXP_VARIABLE_NUMBER | SEXP_VARIABLE_SET;
	else
		Sexp_variables[index].type = SEXP_VARIABLE_STRING | SEXP_VARIABLE_SET;
}

/**
 * Modify a Sexp_variable to be used in a mission
 *
 * This should be called in mission when an sexp_variable is to be modified
 */
void sexp_modify_variable(char *text, int index, bool sexp_callback)
{
	Assert(index >= 0 && index < MAX_SEXP_VARIABLES);
	Assert(Sexp_variables[index].type & SEXP_VARIABLE_SET);
	Assert( !MULTIPLAYER_CLIENT );

	if (strchr(text, '$') != NULL)
	{
		// we want to use the same variable substitution that's in messages etc.
		SCP_string temp_text = text;
		sexp_replace_variable_names_with_values(temp_text);

		// copy to original buffer
		int len = temp_text.copy(Sexp_variables[index].text, TOKEN_LENGTH);
		text[len] = 0;
	}
	else
	{
		// no variables, so no substitution
		strcpy_s(Sexp_variables[index].text, text);
	}
	Sexp_variables[index].type |= SEXP_VARIABLE_MODIFIED;

	// do multi_callback_here
	// if we're called from the sexp code send a SEXP packet (more efficient) 
	if( MULTIPLAYER_MASTER && (Sexp_variables[index].type & SEXP_VARIABLE_NETWORK) && sexp_callback) {
		multi_start_callback();
		multi_send_int(index);
		multi_send_string(Sexp_variables[index].text);
		multi_end_callback();
	}
	// otherwise send a SEXP variable packet
	else if ( (Game_mode & GM_MULTIPLAYER) && (Sexp_variables[index].type & SEXP_VARIABLE_NETWORK) ) {
		send_variable_update_packet(index, Sexp_variables[index].text);
	}
}

void multi_sexp_modify_variable()
{
	char value[TOKEN_LENGTH];
	int variable_index = -1;

	// get the data
	multi_get_int(variable_index);
	if (!multi_get_string(value)) {
		return;
	}

	// set the sexp_variable
	if ( (variable_index >= 0) && (variable_index < MAX_SEXP_VARIABLES) ) {
		// maybe create it first
		if (!(Sexp_variables[variable_index].type & SEXP_VARIABLE_SET)) {
			mprintf(("Warning; received multi packet for variable index which is not set!  Assuming this should be an array block variable..."));
			sexp_add_array_block_variable(variable_index, can_construe_as_integer(value));
		}

		strcpy_s(Sexp_variables[variable_index].text, value);
	}	
}

void sexp_modify_variable(int n)
{
	int sexp_variable_index;
	int new_number;
	char *new_text;
	char number_as_str[TOKEN_LENGTH];

	Assert(n >= 0);

	// Only do single player of multi host
	if ( MULTIPLAYER_CLIENT )
		return;

	// get sexp_variable index
	Assert(Sexp_nodes[n].first == -1);
	sexp_variable_index = atoi(Sexp_nodes[n].text);

	// verify variable set
	Assert(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_SET);

	if (Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_NUMBER)
	{
		// get new numerical value
		new_number = eval_sexp(CDR(n));
		sprintf(number_as_str, "%d", new_number);

		// assign to variable
		sexp_modify_variable(number_as_str, sexp_variable_index);
	}
	else if (Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING)
	{
		// get new string
		new_text = CTEXT(CDR(n));

		// assign to variable
		sexp_modify_variable(new_text, sexp_variable_index);
	}
	else
	{
		Error(LOCATION, "Invalid variable type.\n");
	}
}

bool is_sexp_node_numeric(int node)
{
	Assert(node >= 0);

	// make the common case fast: if the node has a CAR node, that means it uses an operator;
	// and operators cannot currently return strings
	if (Sexp_nodes[node].first >= 0)
		return true;
	
	// if the node text is numeric, the node is too
	if (can_construe_as_integer(Sexp_nodes[node].text))
		return true;

	// otherwise it's gotta be text
	return false;
}

// By Goober5000. Very similar to sexp_modify_variable(). Even uses the same multi code! :)	
void sexp_set_variable_by_index(int node)
{
	int sexp_variable_index;
	int new_number;
	char *new_text;
	char number_as_str[TOKEN_LENGTH];

	Assert(node >= 0);

	// Only do single player of multi host
	if ( MULTIPLAYER_CLIENT )
		return;

	// get sexp_variable index
	sexp_variable_index = eval_num(node);

	// check range
	if (sexp_variable_index < 0 || sexp_variable_index > MAX_SEXP_VARIABLES)
	{
		Warning(LOCATION, "variable-array-set: sexp variable index %d out of range!  min is 0; max is %d", sexp_variable_index, MAX_SEXP_VARIABLES - 1);
		return;
	}

	// verify variable set
	if (!(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_SET))
	{
		// well phooey.  go ahead and create it
		sexp_add_array_block_variable(sexp_variable_index, is_sexp_node_numeric(CDR(node)));
	}

	if (Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_NUMBER)
	{
		// get new numerical value
		new_number = eval_sexp(CDR(node));
		sprintf(number_as_str, "%d", new_number);

		// assign to variable
		sexp_modify_variable(number_as_str, sexp_variable_index);
	}
	else if (Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING)
	{
		// get new string
		new_text = CTEXT(CDR(node));

		// assign to variable
		sexp_modify_variable(new_text, sexp_variable_index);
	}
	else
	{
		Error(LOCATION, "Invalid variable type.\n");
	}
}

// Goober5000
int sexp_get_variable_by_index(int node)
{
	int sexp_variable_index;

	Assert(node >= 0);

	// get sexp_variable index
	sexp_variable_index = eval_num(node);

	// check range
	if (sexp_variable_index < 0 || sexp_variable_index > MAX_SEXP_VARIABLES)
	{
		Warning(LOCATION, "variable-array-get: sexp variable index %d out of range!  min is 0; max is %d", sexp_variable_index, MAX_SEXP_VARIABLES - 1);
		return 0;
	}

	if (Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_NOT_USED)
	{
		mprintf(("warning: retrieving a value from a sexp variable which is not in use!"));
	}
	else if (!(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_SET))
	{
		mprintf(("warning: retrieving a value from a sexp variable which is not set!"));
	}

	if (Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING)
	{
		mprintf(("warning: variable %d is a string but it is not possible to return a string value through a sexp!", sexp_variable_index));
		return SEXP_NAN_FOREVER;
	}

	return atoi(Sexp_variables[sexp_variable_index].text);
}

// Different type needed for Fred (1) allow modification of type (2) no callback required
void sexp_fred_modify_variable(const char *text, const char *var_name, int index, int type)
{
	Assert(index >= 0 && index < MAX_SEXP_VARIABLES);
	Assert(Sexp_variables[index].type & SEXP_VARIABLE_SET);
	Assert( (type & SEXP_VARIABLE_NUMBER) || (type & SEXP_VARIABLE_STRING) );

	strcpy_s(Sexp_variables[index].text, text);
	strcpy_s(Sexp_variables[index].variable_name, var_name);
	Sexp_variables[index].type = (SEXP_VARIABLE_SET | SEXP_VARIABLE_MODIFIED | type);
}

/**
 * Given a sexp node return the index of the variable at that node, -1 if not found
 */
int get_index_sexp_variable_from_node (int node)
{
	int var_index; 

	if (!(Sexp_nodes[node].type & SEXP_FLAG_VARIABLE)) {
		return -1;
	}

	if (Fred_running) {
		var_index = get_index_sexp_variable_name(Sexp_nodes[node].text);
	}
	else {
		var_index = atoi(Sexp_nodes[node].text);
	}

	return var_index; 
}

/**
 * Return index of sexp_variable_name, -1 if not found
 */
int get_index_sexp_variable_name(const char *text)
{
	for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_SET) {
			// check case sensitive
			if ( !strcmp(Sexp_variables[i].variable_name, text) ) {
				return i;
			}
		}
	}

	// not found
	return -1;
}

/**
 * Return index of sexp_variable_name, -1 if not found
 */
int get_index_sexp_variable_name(SCP_string &text)
{
	for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_SET) {
			// check case sensitive
			if ( text == Sexp_variables[i].variable_name ) {
				return i;
			}
		}
	}

	// not found
	return -1;
}

// Goober5000 - tests whether a variable name starts here
// return index of sexp_variable_name, -1 if not found
int get_index_sexp_variable_name_special(const char *startpos)
{
	for (int i = MAX_SEXP_VARIABLES - 1; i >= 0; i--) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_SET) {
			// check case sensitive
			// check number of chars in variable name
			if ( !strncmp(startpos, Sexp_variables[i].variable_name, strlen(Sexp_variables[i].variable_name)) ) {
				return i;
			}
		}
	}

	// not found
	return -1;
}

// Goober5000 - tests whether a variable name starts here
// return index of sexp_variable_name, -1 if not found
int get_index_sexp_variable_name_special(SCP_string &text, size_t startpos)
{
	for (int i = MAX_SEXP_VARIABLES - 1; i >= 0; i--) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_SET) {
			// check case sensitive
			// check that the variable name starts here, as opposed to farther down the string
			size_t pos = text.find(Sexp_variables[i].variable_name, startpos);
			if (pos != SCP_string::npos && pos == startpos) {
				return i;
			}
		}
	}

	// not found
	return -1;
}

// Goober5000
bool sexp_replace_variable_names_with_values(char *text, int max_len)
{
	Assert(text != NULL);
	Assert(max_len >= 0);

	bool replaced_anything = false;
	char *pos = text;
	do {
		// look for the meta-character
		pos = strchr(pos, '$');

		// found?
		if (pos != NULL)
		{
			// see if a variable starts at the next char
			int var_index = get_index_sexp_variable_name_special(pos+1);
			if (var_index >= 0)
			{
				// get the replacement string ($variable)
				char what_to_replace[TOKEN_LENGTH+1];
				memset(what_to_replace, 0, TOKEN_LENGTH+1);
				strncpy(what_to_replace, pos, strlen(Sexp_variables[var_index].variable_name) + 1);

				// replace it
				pos = text + replace_one(text, what_to_replace, Sexp_variables[var_index].text, max_len);
				replaced_anything = true;
			}
			// no match... so keep iterating along the string
			else
			{
				pos++;
			}
		}
	} while (pos != NULL);

	return replaced_anything;
}

// Goober5000
bool sexp_replace_variable_names_with_values(SCP_string &text)
{
	bool replaced_anything = false;

	size_t lookHere = 0;
	size_t foundHere;

	do {
		// look for the meta-character
		foundHere = text.find('$', lookHere);

		// found?
		if (foundHere != SCP_string::npos)
		{
			// see if a variable starts at the next char
			int var_index = get_index_sexp_variable_name_special(text, foundHere+1);
			if (var_index >= 0)
			{
				// replace $variable with the value
				text.replace(foundHere, strlen(Sexp_variables[var_index].variable_name)+1, Sexp_variables[var_index].text);
				replaced_anything = true;

				lookHere = foundHere + strlen(Sexp_variables[var_index].text);
			}
			// no match... so keep iterating along the string
			else
			{
				lookHere = foundHere + 1;
			}
		}
	} while (foundHere != SCP_string::npos);

	return replaced_anything;
}

// returns the index of the nth variable of the type given or -1 if there aren't that many
// NOTE : Not 0th order. If you want the 4th string variable call it as get_nth_variable_index(4, SEXP_VARIABLE_STRING)
int get_nth_variable_index(int nth, int variable_type)
{
	// Loop through Sexp_variables until we have found the one corresponding to the argument
	Assert ((nth > 0) && (nth < MAX_SEXP_VARIABLES));
	for (int i=0; i < MAX_SEXP_VARIABLES; i++)	{
		if ((Sexp_variables[i].type & variable_type)) {
			nth--; 
		}

		if (!nth) {
			return i; 
		}
	}
	return -1;
}

/**
 * Count number of sexp_variables that are set
 */
int sexp_variable_count()
{
	int count = 0;

	for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
		if ( Sexp_variables[i].type & SEXP_VARIABLE_SET) {
			count++;
		}
	}

	return count;
}

/**
 * Count number of campaign-persistent sexp_variables that are set
 */
int sexp_campaign_persistent_variable_count()
{
	int count = 0;

	for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
		if ( (Sexp_variables[i].type & SEXP_VARIABLE_SET) && (Sexp_variables[i].type & SEXP_VARIABLE_CAMPAIGN_PERSISTENT) ) {
			count++;
		}
	}

	return count;
}

/**
 * Given an index in Sexp_variables, returns the number variables of a type in the array until this point
 */
int sexp_variable_typed_count(int sexp_variables_index, int variable_type)
{
	Assert ((sexp_variables_index >= 0) && (sexp_variables_index < MAX_SEXP_VARIABLES));
	// Loop through Sexp_variables until we have found the one corresponding to the argument
	int count = 0;
	for (int i=0; i < MAX_SEXP_VARIABLES; i++)	{
		if (!(Sexp_variables[i].type & variable_type)) {
			continue; 
		}

		if (i == sexp_variables_index) {
			return count ; 
		}
		count++;
	}
	// shouldn't ever get here
	Int3();
	return -1;
}

/**
 * Delete sexp_variable from active
 */
void sexp_variable_delete(int index)
{
	Assert(Sexp_variables[index].type & SEXP_VARIABLE_SET);

	Sexp_variables[index].type = SEXP_VARIABLE_NOT_USED;
}

int sexp_var_compare(const void *var1, const void *var2)
{
	int set1, set2;
	sexp_variable *sexp_var1, *sexp_var2;

	sexp_var1 = (sexp_variable*) var1;
	sexp_var2 = (sexp_variable*) var2;

	set1 = sexp_var1->type & SEXP_VARIABLE_SET;
	set2 = sexp_var2->type & SEXP_VARIABLE_SET;

	if (!set1 && !set2) {
		return 0;
	} else if (set1 && !set2) {
		return -1;
	} else if (!set1 && set2) {
		return 1;
	} else {
		return stricmp( sexp_var1->variable_name, sexp_var2->variable_name);
	}
}

/**
 * Sort sexp_variable list lexigraphically, with set before unset
 */
void sexp_variable_sort()
{
	insertion_sort( (void *)Sexp_variables, (size_t)(MAX_SEXP_VARIABLES), sizeof(sexp_variable), sexp_var_compare );
}

/**
 * Evaluate number which may result from an operator or may be text
 */
int eval_num(int n)
{
	if (n < 0)
	{
		Int3();
		return 0;
	}

	if (CAR(n) != -1)				// if argument is a sexp
		return eval_sexp(CAR(n));
	else
		return atoi(CTEXT(n));		// otherwise, just get the number
}

// Goober5000
int get_sexp_id(char *sexp_name)
{
	for (int i = 0; i < Num_operators; i++)
	{
		if (!stricmp(sexp_name, Operators[i].text))
			return Operators[i].value;
	}
	return -1;
}

// Goober5000
int get_category(int sexp_id)
{
	int category = (sexp_id & OP_CATEGORY_MASK);

	// hack so that CHANGE and CHANGE2 show up in the same menu
	if (category == OP_CATEGORY_CHANGE2)
		category = OP_CATEGORY_CHANGE;

	return category;
}

// Goober5000
int category_of_subcategory(int subcategory_id)
{
	int category = (subcategory_id & OP_CATEGORY_MASK);

	// hack so that CHANGE and CHANGE2 show up in the same menu
	if (category == OP_CATEGORY_CHANGE2)
		category = OP_CATEGORY_CHANGE;

	return category;
}

// Goober5000 - for FRED2 menu subcategories
int get_subcategory(int sexp_id)
{
	switch(sexp_id)
	{
		case OP_SEND_MESSAGE_LIST:
		case OP_SEND_MESSAGE:
		case OP_SEND_RANDOM_MESSAGE:
		case OP_INVALIDATE_GOAL:
		case OP_VALIDATE_GOAL:
		case OP_SCRAMBLE_MESSAGES:
		case OP_UNSCRAMBLE_MESSAGES:
		case OP_ENABLE_BUILTIN_MESSAGES:
		case OP_DISABLE_BUILTIN_MESSAGES:
		case OP_SET_PERSONA:
			return CHANGE_SUBCATEGORY_MESSAGING_AND_MISSION_GOALS;
			
		case OP_ADD_GOAL:
		case OP_REMOVE_GOAL:
		case OP_CLEAR_GOALS:
		case OP_GOOD_REARM_TIME:
		case OP_GOOD_SECONDARY_TIME:
		case OP_CHANGE_IFF:
		case OP_CHANGE_AI_CLASS:
		case OP_PROTECT_SHIP:
		case OP_UNPROTECT_SHIP:
		case OP_BEAM_PROTECT_SHIP:
		case OP_BEAM_UNPROTECT_SHIP:
		case OP_TURRET_PROTECT_SHIP:
		case OP_TURRET_UNPROTECT_SHIP:
		case OP_KAMIKAZE:
		case OP_PLAYER_USE_AI:
		case OP_PLAYER_NOT_USE_AI:
		case OP_ALLOW_TREASON:
		case OP_SET_PLAYER_ORDERS:
		case OP_CHANGE_IFF_COLOR:
		case OP_FORCE_GLIDE:
		case OP_SET_PLAYER_THROTTLE_SPEED:
			return CHANGE_SUBCATEGORY_AI_AND_IFF;
			
		case OP_SABOTAGE_SUBSYSTEM:
		case OP_REPAIR_SUBSYSTEM:
		case OP_SET_SUBSYSTEM_STRNGTH:
		case OP_SUBSYS_SET_RANDOM:
		case OP_SELF_DESTRUCT:
		case OP_TRANSFER_CARGO:
		case OP_EXCHANGE_CARGO:
		case OP_SET_CARGO:
		case OP_JETTISON_CARGO:
		case OP_SET_DOCKED:
		case OP_CARGO_NO_DEPLETE:
		case OP_SET_SCANNED:
		case OP_SET_UNSCANNED:
		case OP_LOCK_ROTATING_SUBSYSTEM:
		case OP_FREE_ROTATING_SUBSYSTEM:
		case OP_REVERSE_ROTATING_SUBSYSTEM:
		case OP_ROTATING_SUBSYS_SET_TURN_TIME:
		case OP_TRIGGER_SUBMODEL_ANIMATION:
		case OP_SET_PRIMARY_AMMO:		// Karajorma
		case OP_SET_SECONDARY_AMMO:		// Karajorma
		case OP_SET_PRIMARY_WEAPON:		// Karajorma
		case OP_SET_SECONDARY_WEAPON:	// Karajorma
		case OP_SET_NUM_COUNTERMEASURES: // Karajorma
		case OP_LOCK_PRIMARY_WEAPON:
		case OP_UNLOCK_PRIMARY_WEAPON:
		case OP_LOCK_SECONDARY_WEAPON:
		case OP_UNLOCK_SECONDARY_WEAPON:
		case OP_CHANGE_SUBSYSTEM_NAME:
		case OP_LOCK_AFTERBURNER:	// KeldorKatarn
		case OP_UNLOCK_AFTERBURNER:	// KeldorKatarn
		case OP_SET_AFTERBURNER_ENERGY: 
		case OP_SET_WEAPON_ENERGY:
		case OP_SET_SHIELD_ENERGY:
		case OP_DISABLE_ETS:
		case OP_ENABLE_ETS:
			return CHANGE_SUBCATEGORY_SUBSYSTEMS_AND_CARGO;
			
		case OP_SHIP_INVULNERABLE:
		case OP_SHIP_VULNERABLE:
		case OP_SHIP_BOMB_TARGETABLE:
		case OP_SHIP_BOMB_UNTARGETABLE:
		case OP_SHIP_GUARDIAN:
		case OP_SHIP_NO_GUARDIAN:
		case OP_SHIP_GUARDIAN_THRESHOLD:
		case OP_SHIP_SUBSYS_GUARDIAN_THRESHOLD:
		case OP_SHIP_INVISIBLE:
		case OP_SHIP_VISIBLE:
		case OP_SHIP_STEALTHY:
		case OP_SHIP_UNSTEALTHY:
		case OP_FRIENDLY_STEALTH_INVISIBLE:
		case OP_FRIENDLY_STEALTH_VISIBLE:
		case OP_SHIP_SUBSYS_TARGETABLE:
		case OP_SHIP_SUBSYS_UNTARGETABLE:
		case OP_SHIP_SUBSYS_NO_REPLACE:
		case OP_SHIP_SUBSYS_NO_LIVE_DEBRIS:
		case OP_SHIP_SUBSYS_VANISHED:
		case OP_SHIP_SUBSYS_IGNORE_IF_DEAD:
		case OP_WARP_BROKEN:
		case OP_WARP_NOT_BROKEN:
		case OP_WARP_NEVER:
		case OP_WARP_ALLOWED:
		case OP_SET_SUBSPACE_DRIVE:
		case OP_SET_ARMOR_TYPE:
		case OP_ADD_TO_COLGROUP:
		case OP_REMOVE_FROM_COLGROUP:
		case OP_GET_COLGROUP_ID:
		case OP_SHIP_EFFECT:
			return CHANGE_SUBCATEGORY_SHIP_STATUS;
			
		case OP_BEAM_FIRE:
		case OP_BEAM_FIRE_COORDS:
		case OP_BEAM_FREE:
		case OP_BEAM_FREE_ALL:
		case OP_BEAM_LOCK:
		case OP_BEAM_LOCK_ALL:
		case OP_TURRET_FREE:
		case OP_TURRET_FREE_ALL:
		case OP_TURRET_LOCK:
		case OP_TURRET_LOCK_ALL:
		case OP_TURRET_TAGGED_ONLY_ALL:
		case OP_TURRET_TAGGED_CLEAR_ALL:
		case OP_TURRET_TAGGED_SPECIFIC:
		case OP_TURRET_TAGGED_CLEAR_SPECIFIC:
		case OP_TURRET_CHANGE_WEAPON:
		case OP_TURRET_SET_DIRECTION_PREFERENCE:
		case OP_TURRET_SET_RATE_OF_FIRE:
		case OP_TURRET_SET_OPTIMUM_RANGE:
		case OP_TURRET_SET_TARGET_PRIORITIES:
		case OP_TURRET_SET_TARGET_ORDER:
		case OP_SHIP_TURRET_TARGET_ORDER:
		case OP_TURRET_SUBSYS_TARGET_DISABLE:
		case OP_TURRET_SUBSYS_TARGET_ENABLE:
			return CHANGE_SUBCATEGORY_BEAMS_AND_TURRETS;

		case OP_RED_ALERT:
		case OP_END_MISSION:
		case OP_FORCE_JUMP:
		case OP_END_CAMPAIGN:
		case OP_SET_DEBRIEFING_TOGGLED:
		case OP_GRANT_PROMOTION:
		case OP_GRANT_MEDAL:
		case OP_ALLOW_SHIP:
		case OP_ALLOW_WEAPON:
		case OP_TECH_ADD_SHIP:
		case OP_TECH_ADD_WEAPON:
		case OP_TECH_ADD_INTEL:
		case OP_TECH_RESET_TO_DEFAULT:
		case OP_CHANGE_PLAYER_SCORE:
		case OP_CHANGE_TEAM_SCORE:
		case OP_SET_RESPAWNS:
			return CHANGE_SUBCATEGORY_MISSION_AND_CAMPAIGN;

		case OP_DONT_COLLIDE_INVISIBLE:
		case OP_COLLIDE_INVISIBLE:
		case OP_CHANGE_SHIP_CLASS:
		case OP_DEACTIVATE_GLOW_POINTS:
		case OP_ACTIVATE_GLOW_POINTS:
		case OP_DEACTIVATE_GLOW_MAPS:
		case OP_ACTIVATE_GLOW_MAPS:
		case OP_DEACTIVATE_GLOW_POINT_BANK:
		case OP_ACTIVATE_GLOW_POINT_BANK:
		case OP_SET_THRUSTERS:
			return CHANGE_SUBCATEGORY_MODELS_AND_TEXTURES;

		case OP_SET_OBJECT_POSITION:
		case OP_SET_OBJECT_ORIENTATION:
		case OP_SET_OBJECT_FACING:
		case OP_SET_OBJECT_FACING_OBJECT:
		case OP_SET_OBJECT_SPEED_X:
		case OP_SET_OBJECT_SPEED_Y:
		case OP_SET_OBJECT_SPEED_Z:
		case OP_SHIP_MANEUVER:
		case OP_SHIP_ROT_MANEUVER:
		case OP_SHIP_LAT_MANEUVER:
			return CHANGE_SUBCATEGORY_COORDINATE_MANIPULATION;

		case OP_CHANGE_SOUNDTRACK:
		case OP_PLAY_SOUND_FROM_TABLE:
		case OP_PLAY_SOUND_FROM_FILE:
		case OP_CLOSE_SOUND_FROM_FILE:
		case OP_SET_SOUND_ENVIRONMENT:
		case OP_UPDATE_SOUND_ENVIRONMENT:
		case OP_ADJUST_AUDIO_VOLUME:
			return CHANGE_SUBCATEGORY_MUSIC_AND_SOUND;

		case OP_ADD_REMOVE_ESCORT:
		case OP_AWACS_SET_RADIUS:
		case OP_PRIMITIVE_SENSORS_SET_RANGE:
		case OP_CAP_WAYPOINT_SPEED:
		case OP_SET_SPECIAL_WARPOUT_NAME:
		case OP_SHIP_CREATE:
		case OP_WEAPON_CREATE:
		case OP_SHIP_VANISH:
		case OP_SHIP_VAPORIZE:
		case OP_SHIP_NO_VAPORIZE:
		case OP_SET_EXPLOSION_OPTION:
		case OP_SHIELDS_ON:
		case OP_SHIELDS_OFF:
		case OP_SHIP_TAG:
		case OP_SHIP_UNTAG:
		case OP_DAMAGED_ESCORT_LIST:
		case OP_DAMAGED_ESCORT_LIST_ALL:
		case OP_SET_SUPPORT_SHIP:
		case OP_SET_ARRIVAL_INFO:
		case OP_SET_DEPARTURE_INFO:
		case OP_SHIP_CHANGE_ALT_NAME:
		case OP_SHIP_CHANGE_CALLSIGN:
		case OP_SET_DEATH_MESSAGE:
		case OP_EXPLOSION_EFFECT:
		case OP_WARP_EFFECT:
		case OP_SHIP_COPY_DAMAGE:
		case OP_REMOVE_WEAPONS:
		case OP_WEAPON_SET_DAMAGE_TYPE:
		case OP_SHIP_SET_DAMAGE_TYPE:
		case OP_SHIP_SHOCKWAVE_SET_DAMAGE_TYPE:
		case OP_FIELD_SET_DAMAGE_TYPE:
		case OP_SET_MOBILE:
		case OP_SET_IMMOBILE:
			return CHANGE_SUBCATEGORY_SPECIAL;

		case OP_SET_SKYBOX_MODEL:
		case OP_SET_SKYBOX_ORIENT:
		case OP_MISSION_SET_NEBULA:
		case OP_ADD_BACKGROUND_BITMAP:
		case OP_REMOVE_BACKGROUND_BITMAP:
		case OP_ADD_SUN_BITMAP:
		case OP_REMOVE_SUN_BITMAP:
		case OP_NEBULA_CHANGE_STORM:
		case OP_NEBULA_TOGGLE_POOF:
		case OP_SET_AMBIENT_LIGHT:
		case OP_SET_POST_EFFECT:
		case OP_MISSION_SET_SUBSPACE:
			return CHANGE_SUBCATEGORY_BACKGROUND_AND_NEBULA;

		case OP_HUD_DISABLE:
		case OP_HUD_DISABLE_EXCEPT_MESSAGES:
		case OP_HUD_SET_TEXT:
		case OP_HUD_SET_TEXT_NUM:
		case OP_HUD_SET_COORDS:
		case OP_HUD_SET_FRAME:
		case OP_HUD_SET_COLOR:
		case OP_HUD_SET_MAX_TARGETING_RANGE:
		case OP_HUD_DISPLAY_GAUGE:
		case OP_HUD_SET_MESSAGE:
		case OP_HUD_SET_DIRECTIVE:
		case OP_HUD_GAUGE_SET_ACTIVE:
		case OP_HUD_ACTIVATE_GAUGE_TYPE:
		case OP_HUD_CLEAR_MESSAGES:
			return CHANGE_SUBCATEGORY_HUD;

		case OP_CUTSCENES_SET_CUTSCENE_BARS:
		case OP_CUTSCENES_UNSET_CUTSCENE_BARS:
		case OP_CUTSCENES_FADE_IN:
		case OP_CUTSCENES_FADE_OUT:
		case OP_CUTSCENES_SET_CAMERA:
		case OP_CUTSCENES_SET_CAMERA_FACING:
		case OP_CUTSCENES_SET_CAMERA_FACING_OBJECT:
		case OP_CUTSCENES_SET_CAMERA_FOV:
		case OP_CUTSCENES_SET_CAMERA_HOST:
		case OP_CUTSCENES_SET_CAMERA_POSITION:
		case OP_CUTSCENES_SET_CAMERA_ROTATION:
		case OP_CUTSCENES_SET_CAMERA_TARGET:
		case OP_CUTSCENES_SET_FOV:
		case OP_CUTSCENES_GET_FOV:
		case OP_CUTSCENES_RESET_FOV:
		case OP_CUTSCENES_FORCE_PERSPECTIVE:
		case OP_CUTSCENES_RESET_CAMERA:
		case OP_CUTSCENES_SHOW_SUBTITLE:
		case OP_CUTSCENES_SHOW_SUBTITLE_TEXT:
		case OP_CUTSCENES_SHOW_SUBTITLE_IMAGE:
		case OP_CLEAR_SUBTITLES:
		case OP_CUTSCENES_SET_TIME_COMPRESSION:
		case OP_CUTSCENES_RESET_TIME_COMPRESSION:
		case OP_SET_CAMERA_SHUDDER:
		case OP_SUPERNOVA_START:
		case OP_SUPERNOVA_STOP:
			return CHANGE_SUBCATEGORY_CUTSCENES;

		case OP_JUMP_NODE_SET_JUMPNODE_NAME: //CommanderDJ
		case OP_JUMP_NODE_SET_JUMPNODE_COLOR:
		case OP_JUMP_NODE_SET_JUMPNODE_MODEL:
		case OP_JUMP_NODE_SHOW_JUMPNODE:
		case OP_JUMP_NODE_HIDE_JUMPNODE:
			return CHANGE_SUBCATEGORY_JUMP_NODES;

		case OP_NAV_ADD_WAYPOINT:
		case OP_NAV_ADD_SHIP:
		case OP_NAV_DEL:
		case OP_NAV_HIDE:
		case OP_NAV_RESTRICT:
		case OP_NAV_UNHIDE:
		case OP_NAV_UNRESTRICT:
		case OP_NAV_SET_VISITED:
		case OP_NAV_SET_CARRY:
		case OP_NAV_UNSET_CARRY:
		case OP_NAV_UNSET_VISITED:
		case OP_NAV_SET_NEEDSLINK:
		case OP_NAV_UNSET_NEEDSLINK:
		case OP_NAV_USECINEMATICS:
		case OP_NAV_USEAP:
			return CHANGE_SUBCATEGORY_NAV;

		case OP_NUM_PLAYERS:
		case OP_TEAM_SCORE:
		case OP_SHIP_DEATHS:
		case OP_RESPAWNS_LEFT:
		case OP_IS_PLAYER:
		case OP_NUM_SHIPS_IN_BATTLE:
		case OP_NUM_SHIPS_IN_WING:
		case OP_LAST_ORDER_TIME:
		case OP_DIRECTIVE_VALUE:
			return STATUS_SUBCATEGORY_MULTIPLAYER_AND_MISSION_CONFIG;

		case OP_SHIELD_RECHARGE_PCT:
		case OP_ENGINE_RECHARGE_PCT:
		case OP_WEAPON_RECHARGE_PCT:
		case OP_SHIELD_QUAD_LOW:
		case OP_PRIMARY_AMMO_PCT:
		case OP_SECONDARY_AMMO_PCT:
		case OP_IS_PRIMARY_SELECTED:
		case OP_IS_SECONDARY_SELECTED:
		case OP_GET_PRIMARY_AMMO:
		case OP_GET_SECONDARY_AMMO:
		case OP_GET_NUM_COUNTERMEASURES:
		case OP_AFTERBURNER_LEFT:
		case OP_WEAPON_ENERGY_LEFT:
		case OP_PRIMARY_FIRED_SINCE:
		case OP_SECONDARY_FIRED_SINCE:
		case OP_HAS_PRIMARY_WEAPON:
		case OP_HAS_SECONDARY_WEAPON:
			return STATUS_SUBCATEGORY_SHIELDS_ENGINES_AND_WEAPONS;
			
		case OP_CARGO_KNOWN_DELAY:
		case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
		case OP_IS_CARGO:
			return STATUS_SUBCATEGORY_CARGO;
			
		case OP_HAS_BEEN_TAGGED_DELAY:
		case OP_IS_TAGGED:
		case OP_IS_SHIP_VISIBLE:
		case OP_IS_SHIP_STEALTHY:
		case OP_IS_FRIENDLY_STEALTH_VISIBLE:
		case OP_IS_IFF:
		case OP_IS_AI_CLASS:
		case OP_IS_SHIP_CLASS:
		case OP_IS_SHIP_TYPE:
		case OP_CURRENT_SPEED:
		case OP_GET_THROTTLE_SPEED:
		case OP_IS_FACING:
		case OP_IS_IN_MISSION:
		case OP_NAV_ISLINKED:
			return STATUS_SUBCATEGORY_SHIP_STATUS;
			
		case OP_SHIELDS_LEFT:
		case OP_HITS_LEFT:
		case OP_HITS_LEFT_SUBSYSTEM:
		case OP_HITS_LEFT_SUBSYSTEM_GENERIC:
		case OP_HITS_LEFT_SUBSYSTEM_SPECIFIC:
		case OP_SIM_HITS_LEFT:
		case OP_GET_DAMAGE_CAUSED:
			return STATUS_SUBCATEGORY_DAMAGE;
		
		case OP_DISTANCE:
		case OP_DISTANCE_SUBSYSTEM:
		case OP_NAV_DISTANCE:
		case OP_GET_OBJECT_X:
		case OP_GET_OBJECT_Y:
		case OP_GET_OBJECT_Z:
		case OP_GET_OBJECT_PITCH:
		case OP_GET_OBJECT_BANK:
		case OP_GET_OBJECT_HEADING:
		case OP_GET_OBJECT_SPEED_X:
		case OP_GET_OBJECT_SPEED_Y:
		case OP_GET_OBJECT_SPEED_Z:
		case OP_NUM_WITHIN_BOX:
		case OP_SPECIAL_WARP_DISTANCE:
		case OP_IS_IN_BOX:
			return STATUS_SUBCATEGORY_DISTANCE_AND_COORDINATES;
			
		case OP_WAS_PROMOTION_GRANTED:
		case OP_WAS_MEDAL_GRANTED:
		case OP_NUM_KILLS:
		case OP_NUM_ASSISTS:
		case OP_NUM_TYPE_KILLS:
		case OP_NUM_CLASS_KILLS:
		case OP_SHIP_SCORE:
		case OP_SKILL_LEVEL_AT_LEAST:
			return STATUS_SUBCATEGORY_KILLS_AND_SCORING;

		default:
			return -1;		// sexp doesn't have a subcategory
	}
}

sexp_help_struct Sexp_help[] = {
	{ OP_PLUS, "Plus (Arithmetic operator)\r\n"
		"\tAdds numbers and returns results.\r\n\r\n"
		"Returns a number.  Takes 2 or more numeric arguments." },

	{ OP_MINUS, "Minus (Arithmetic operator)\r\n"
		"\tSubtracts numbers and returns results.\r\n\r\n"
		"Returns a number.  Takes 2 or more numeric arguments." },

	{ OP_MOD, "Mod (Arithmetic operator)\r\n"
		"\tDivides numbers and returns the remainer.\r\n\r\n"
		"Returns a number.  Takes 2 or more numeric arguments." },

	{ OP_MUL, "Multiply (Arithmetic operator)\r\n"
		"\tMultiplies numbers and returns results.\r\n\r\n"
		"Returns a number.  Takes 2 or more numeric arguments." },

	{ OP_DIV, "Divide (Arithmetic operator)\r\n"
		"\tDivides numbers and returns results.\r\n\r\n"
		"Returns a number.  Takes 2 or more numeric arguments." },

	{ OP_RAND, "Rand (Arithmetic operator)\r\n"
		"\tGets a random number.  This number will not change on successive calls to this sexp.\r\n\r\n"
		"Returns a number.  Takes 2 or 3 numeric arguments...\r\n"
		"\t1:\tLow range of random number.\r\n"
		"\t2:\tHigh range of random number.\r\n" 
		"\t3:\t(optional) A seed to use when generating numbers. (Setting this to 0 is the same as having no seed at all)" },

	// Goober5000
	{ OP_RAND_MULTIPLE, "Rand-multiple (Arithmetic operator)\r\n"
		"\tGets a random number.  This number can and will change between successive calls to this sexp.\r\n\r\n"
		"Returns a number.  Takes 2 or 3 numeric arguments...\r\n"
		"\t1:\tLow range of random number.\r\n"
		"\t2:\tHigh range of random number.\r\n" 
		"\t3:\t(optional) A seed to use when generating numbers. (Setting this to 0 is the same as having no seed at all)" },

	// -------------------------- Nav Points --- Kazan -------------------------- 
	{ OP_NAV_IS_VISITED, "Takes 1 argument: The Nav Point Name\r\n"
		"Returns whether that nav point has been visited (player within 1000 meters)" },

	{ OP_NAV_DISTANCE, "Takes 1 argument: The Nav point Name\r\n"
		"Returns the distance from the player ship to that nav point" },

	{ OP_NAV_ADD_WAYPOINT, "Takes 3 Arguments: NavPoint Name, Waypoint Path Name, Waypoint Path Point #\r\n"
		"IE Setting up 'Nav 1' to be on the first Waypoint on Wapoint Path 'Intercept' the arguments would be:\r\n"
		"'Nav 1', 'Intercept', '1'" },

	{ OP_NAV_ADD_SHIP, "Takes 2 Arguments: NavPoint Name, Ship Name\r\n"
		"Binds the named navpoint to the named ship - when the ship moves, the waypoint moves with it\r\n" },

	{ OP_NAV_DEL, "Takes 1 Argument: NavPoint Name, and deletes that NavPoint" },

	{ OP_NAV_HIDE, "Takes 1 Argument: NavPoint Name, it then 'hides' that Nav Point\n\r"
		"This causes the nav point to be unselectable, if in the future the Nav Map screen is implemented\r\n" 
		"then a hidden nav point will not be displayed on it\r\n" },

	{ OP_NAV_RESTRICT, "Takes 1 Argument: NavPoint Name, it then 'restricts' that Nav Point\n\r"
		"This causes the nav point to be unselectable, if in the future the Nav Map screen is implemented\r\n" 
		"then a restrict nav point will be displayed on it, but unselectable\r\n"  },

	{ OP_NAV_UNHIDE, "Takes 1 Argument: NavPoint Name, it then unhides it\r\n" },

	{ OP_NAV_UNRESTRICT, "Takes 1 Argument: NavPoint Name, it then unrestricts it\r\n" },

	{ OP_NAV_SET_VISITED, "Takes 1 Argument: NavPoint Name, it then sets its visited flag\r\n" },

	{ OP_NAV_UNSET_VISITED, "Takes 1 Argument: NavPoint Name, it then unsets its visited flag\r\n" },

	{ OP_NAV_SET_CARRY, "Takes at least 1 argument, but can take as many as you give it.\r\n"
		"It takes ship and/or wing names as sets their Nav Carry flag -- a ship with the nav carry flag\r\n" 
		"will autopilot with the player\r\n" },

	{ OP_NAV_UNSET_CARRY, "Takes atleast 1 argument, but can take as many as you give it.\r\n"
		"It takes ship and/or wing names and unsets their Nav Carry flag\r\n" },

	{ OP_NAV_SET_NEEDSLINK, "Takes atleast 1 argument, but can take as many as you give it.\r\n" 
		"It takes ships and marks them as needing AutoNav linkup (approach within link distance)"},

	{ OP_NAV_UNSET_NEEDSLINK, "Takes atleast 1 argument, but can take as many as you give it.\r\n" 
	    "It takes ships and unmarks them as needing AutoNav linkup"},

	{ OP_NAV_ISLINKED, "Takes 1 argument.\r\n"
		"Determines if a ship is linked for autopilot (\"set-nav-carry\" or \"set-nav-needslink\" + linked)"},

	{ OP_NAV_USECINEMATICS, "Takes 1 boolean argument.\r\n"
		"Set to true to enable automatic cinematics, set to false to disable automatic cinematics." },

	{ OP_NAV_USEAP, "Takes 1 boolean argument.\r\n"
		"Set to true to enable autopilot, set to false to disable autopilot." },

	// -------------------------- -------------------------- -------------------------- 

	// Goober5000
	{ OP_ABS, "Absolute value (Arithmetic operator)\r\n"
		"\tReturns the absolute value of a number.  Takes 1 numeric argument.\r\n" },

	// Goober5000
	{ OP_MIN, "Minimum value (Arithmetic operator)\r\n"
		"\tReturns the minimum of a set of numbers.  Takes 1 or more numeric arguments.\r\n" },

	// Goober5000
	{ OP_MAX, "Maximum value (Arithmetic operator)\r\n"
		"\tReturns the maximum of a set of numbers.  Takes 1 or more numeric arguments.\r\n" },

	// Goober5000
	{ OP_AVG, "Average value (Arithmetic operator)\r\n"
		"\tReturns the average (rounded to the nearest integer) of a set of numbers.  Takes 1 or more numeric arguments.\r\n" },

	// Goober5000
	{ OP_POW, "Power (Arithmetic operator)\r\n"
		"\tRaises one number to the power of the next number.  If the result will be larger than INT_MAX or smaller than INT_MIN, the appropriate limit will be returned.  Takes 2 numeric arguments.\r\n" },

	// Goober5000
	{ OP_SIGNUM, "Signum (Arithmetic operator)\r\n"
		"\tReturns the sign of a number: -1 if it is negative, 0 if it is 0, and 1 if it is positive.  Takes one argument.\r\n" },

	// Goober5000
	{ OP_SET_BIT, "set-bit\r\n"
		"\tTurns on (sets to 1) a certain bit in the provided number, returning the result.  This allows numbers to store up to 32 boolean flags, from 2^0 to 2^31.  Takes 2 numeric arguments...\r\n"
		"\t1: The number whose bit should be set\r\n"
		"\t2: The index of the bit to set.  Valid indexes are between 0 and 31, inclusive.\r\n" },

	// Goober5000
	{ OP_UNSET_BIT, "unset-bit\r\n"
		"\tTurns off (sets to 0) a certain bit in the provided number, returning the result.  This allows numbers to store up to 32 boolean flags, from 2^0 to 2^31.  Takes 2 numeric arguments...\r\n"
		"\t1: The number whose bit should be unset\r\n"
		"\t2: The index of the bit to unset.  Valid indexes are between 0 and 31, inclusive.\r\n" },

	// Goober5000
	{ OP_IS_BIT_SET, "is-bit-set\r\n"
		"\tReturns true if the specified bit is set (equal to 1) in the provided number.  Takes 2 numeric arguments...\r\n"
		"\t1: The number whose bit should be tested\r\n"
		"\t2: The index of the bit to test.  Valid indexes are between 0 and 31, inclusive.\r\n" },

	// Goober5000
	{ OP_BITWISE_AND, "bitwise-and\r\n"
		"\tPerforms the bitwise AND operator on its arguments.  This is the same as if the logical AND operator was performed on each successive bit.  Takes 2 or more numeric arguments.\r\n" },

	// Goober5000
	{ OP_BITWISE_OR, "bitwise-or\r\n"
		"\tPerforms the bitwise OR operator on its arguments.  This is the same as if the logical OR operator was performed on each successive bit.  Takes 2 or more numeric arguments.\r\n" },

	// Goober5000
	{ OP_BITWISE_NOT, "bitwise-not\r\n"
		"\tPerforms the bitwise NOT operator on its argument.  This is the same as if the logical NOT operator was performed on each successive bit.\r\n\r\n"
		"Note that the operation is performed as if on an unsigned integer whose maximum value is INT_MAX.  In other words, there is no need to worry about the sign bit.\r\n\r\n"
		"Takes only 1 argument.\r\n" },

	// Goober5000
	{ OP_BITWISE_XOR, "bitwise-xor\r\n"
		"\tPerforms the bitwise XOR operator on its arguments.  This is the same as if the logical XOR operator was performed on each successive bit.  Takes 2 or more numeric arguments.\r\n" },

	{ OP_SET_OBJECT_SPEED_X, "set-object-speed-x\r\n"
		"\tSets the X speed of a ship, wing, or waypoint."
		"Takes 2 or 3 arguments...\r\n"
		"\t1: The name of the object.\r\n"
		"\t2: The speed to set.\r\n"
		"\t3: Whether the speed on the axis should be set according to the universe grid (when false) or according to the object's facing (when true); You almost always want to set this to true; (optional; defaults to false).\r\n" },

	{ OP_SET_OBJECT_SPEED_Y, "set-object-speed-y\r\n"
		"\tSets the Y speed of a ship, wing, or waypoint."
		"Takes 2 or 3 arguments...\r\n"
		"\t1: The name of the object.\r\n"
		"\t2: The speed to set.\r\n"
		"\t3: Whether the speed on the axis should be set according to the universe grid (when false) or according to the object's facing (when true); You almost always want to set this to true; (optional; defaults to false).\r\n" },

	{ OP_SET_OBJECT_SPEED_Z, "set-object-speed-z\r\n"
		"\tSets the Z speed of a ship, wing, or waypoint."
		"Takes 2 or 3 arguments...\r\n"
		"\t1: The name of the object.\r\n"
		"\t2: The speed to set.\r\n"
		"\t3: Whether the speed on the axis should be set according to the universe grid (when false) or according to the object's facing (when true); You almost always want to set this to true; (optional; defaults to false).\r\n" },

	{ OP_GET_OBJECT_SPEED_X, "get-object-speed-x\r\n"
		"\tReturns the X speed of a ship, wing, or waypoint as an integer."
		"Takes 2 or 3 arguments...\r\n"
		"\t1: The name of the object.\r\n"
		"\t2: Whether the speed on the axis should be set according to the universe grid (when false) or according to the object's facing (when true); You almost always want to set this to true; (optional; defaults to false).\r\n" },

	{ OP_GET_OBJECT_SPEED_Y, "get-object-speed-y\r\n"
		"\tReturns the Y speed of a ship, wing, or waypoint as an integer."
		"Takes 2 or 3 arguments...\r\n"
		"\t1: The name of the object.\r\n"
		"\t2: Whether the speed on the axis should be set according to the universe grid (when false) or according to the object's facing (when true); You almost always want to set this to true; (optional; defaults to false).\r\n" },

	{ OP_GET_OBJECT_SPEED_Z, "get-object-speed-z\r\n"
		"\tReturns the Z speed of a ship, wing, or waypoint as an integer."
		"Takes 2 or 3 arguments...\r\n"
		"\t1: The name of the object.\r\n"
		"\t2: Whether the speed on the axis should be set according to the universe grid (when false) or according to the object's facing (when true); You almost always want to set this to true; (optional; defaults to false).\r\n" },

	// Goober5000
	{ OP_GET_OBJECT_X, "get-object-x\r\n"
		"\tReturns the absolute X coordinate of a set of coordinates relative to a particular object (or object's "
		"subsystem).  The input coordinates are the coordinates relative to the object's position and orientation.  "
		"If no input coordinates are specified, the coordinate returned is the coordinate of the object (or object's "
		"subsystem) itself.  Takes 1 to 5 arguments...\r\n"
		"\t1: The name of a ship, wing, or waypoint.\r\n"
		"\t2: A ship subsystem (or \""SEXP_NONE_STRING"\" if the first argument is not a ship - optional).\r\n"
		"\t3: The relative X coordinate (optional).\r\n"
		"\t4: The relative Y coordinate (optional).\r\n"
		"\t5: The relative Z coordinate (optional).\r\n" },

	// Goober5000
	{ OP_GET_OBJECT_Y, "get-object-y\r\n"
		"\tReturns the absolute Y coordinate of a set of coordinates relative to a particular object (or object's "
		"subsystem).  The input coordinates are the coordinates relative to the object's position and orientation.  "
		"If no input coordinates are specified, the coordinate returned is the coordinate of the object (or object's "
		"subsystem) itself.  Takes 1 to 5 arguments...\r\n"
		"\t1: The name of a ship, wing, or waypoint.\r\n"
		"\t2: A ship subsystem (or \""SEXP_NONE_STRING"\" if the first argument is not a ship - optional).\r\n"
		"\t3: The relative X coordinate (optional).\r\n"
		"\t4: The relative Y coordinate (optional).\r\n"
		"\t5: The relative Z coordinate (optional).\r\n" },

	// Goober5000
	{ OP_GET_OBJECT_Z, "get-object-z\r\n"
		"\tReturns the absolute Z coordinate of a set of coordinates relative to a particular object (or object's "
		"subsystem).  The input coordinates are the coordinates relative to the object's position and orientation.  "
		"If no input coordinates are specified, the coordinate returned is the coordinate of the object (or object's "
		"subsystem) itself.  Takes 1 to 5 arguments...\r\n"
		"\t1: The name of a ship, wing, or waypoint.\r\n"
		"\t2: A ship subsystem (or \""SEXP_NONE_STRING"\" if the first argument is not a ship - optional).\r\n"
		"\t3: The relative X coordinate (optional).\r\n"
		"\t4: The relative Y coordinate (optional).\r\n"
		"\t5: The relative Z coordinate (optional).\r\n" },

	// Goober5000
	{ OP_SET_OBJECT_POSITION, "set-object-position\r\n"
		"\tInstantaneously sets an object's spatial coordinates."
		"Takes 4 arguments...\r\n"
		"\t1: The name of a ship, wing, or waypoint.\r\n"
		"\t2: The new X coordinate.\r\n"
		"\t3: The new Y coordinate.\r\n"
		"\t4: The new Z coordinate." },

	// Goober5000
	{ OP_GET_OBJECT_PITCH, "get-object-pitch\r\n"
		"\tReturns the pitch angle, in degrees, of a particular object.  The returned value will be between 0 and 360.  Takes 1 argument...\r\n"
		"\t1: The name of a ship, wing, or waypoint.\r\n" },

	// Goober5000
	{ OP_GET_OBJECT_BANK, "get-object-bank\r\n"
		"\tReturns the bank angle, in degrees, of a particular object.  The returned value will be between 0 and 360.  Takes 1 argument...\r\n"
		"\t1: The name of a ship, wing, or waypoint.\r\n" },

	// Goober5000
	{ OP_GET_OBJECT_HEADING, "get-object-heading\r\n"
		"\tReturns the heading angle, in degrees, of a particular object.  The returned value will be between 0 and 360.  Takes 1 argument...\r\n"
		"\t1: The name of a ship, wing, or waypoint.\r\n" },

	// Goober5000
	{ OP_SET_OBJECT_ORIENTATION, "set-object-orientation\r\n"
		"\tInstantaneously sets an object's spatial orientation."
		"Takes 4 arguments...\r\n"
		"\t1: The name of a ship, wing, or waypoint.\r\n"
		"\t2: The new pitch angle, in degrees.  The angle can be any number; it does not have to be between 0 and 360.\r\n"
		"\t3: The new bank angle, in degrees.  The angle can be any can be any number; it does not have to be between 0 and 360.\r\n"
		"\t4: The new heading angle, in degrees.  The angle can be any number; it does not have to be between 0 and 360." },

	{ OP_TRUE, "True (Boolean operator)\r\n"
		"\tA true boolean state\r\n\r\n"
		"Returns a boolean value." },

	{ OP_FALSE, "False (Boolean operator)\r\n"
		"\tA false boolean state\r\n\r\n"
		"Returns a boolean value." },

	{ OP_AND, "And (Boolean operator)\r\n"
		"\tAnd is true if all of its arguments are true.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more boolean arguments." },

	{ OP_OR, "Or (Boolean operator)\r\n"
		"\tOr is true if any of its arguments are true.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more boolean arguments." },

	{ OP_XOR, "Xor (Boolean operator)\r\n"
		"\tXor is true if exactly one of its arguments is true.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more boolean arguments." },

	{ OP_EQUALS, "Equals (Boolean operator)\r\n"
		"\tIs true if all of its arguments are equal.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more numeric arguments." },

	{ OP_GREATER_THAN, "Greater Than (Boolean operator)\r\n"
		"\tTrue if the first argument is greater than the subsequent argument(s).\r\n\r\n"
		"Returns a boolean value.  Takes 2 numeric arguments." },

	{ OP_LESS_THAN, "Less Than (Boolean operator)\r\n"
		"\tTrue if the first argument is less than the subsequent argument(s).\r\n\r\n"
		"Returns a boolean value.  Takes 2 numeric arguments." },

	{ OP_NOT_EQUAL, "Not Equal To (Boolean operator)\r\n"
		"\tIs true if the first argument is not equal to any of the subsequent arguments.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more numeric arguments." },

	{ OP_GREATER_OR_EQUAL, "Greater Than Or Equal To (Boolean operator)\r\n"
		"\tTrue if the first argument is greater than or equal to the subsequent argument(s).\r\n\r\n"
		"Returns a boolean value.  Takes 2 numeric arguments." },

	{ OP_LESS_OR_EQUAL, "Less Than Or Equal To (Boolean operator)\r\n"
		"\tTrue if the first argument is less than or equal to the subsequent argument(s).\r\n\r\n"
		"Returns a boolean value.  Takes 2 numeric arguments." },

	// Goober5000
	{ OP_PERFORM_ACTIONS, "perform-actions\r\n"
		"\tThis sexp allows actions to be performed as part of a conditional test.  It is most useful for assigning variables or performing some sort of pre-test action within the conditional part of \"when\", etc.  "
		"It works well as the first branch of an \"and\" sexp, provided it returns true so as to evaluate the other \"and\" arguments.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments.\r\n"
		"\t1:\tA boolean value to return after all successive actions have been performed.\r\n"
		"\tRest:\tActions to perform, which would normally appear in a \"when\" sexp.\r\n" },

	// Goober5000
	{ OP_STRING_EQUALS, "String Equals (Boolean operator)\r\n"
		"\tIs true if all of its arguments are equal.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more string arguments." },

	// Goober5000
	{ OP_STRING_GREATER_THAN, "String Greater Than (Boolean operator)\r\n"
		"\tTrue if the first argument is greater than the second argument.\r\n\r\n"
		"Returns a boolean value.  Takes 2 string arguments." },

	// Goober5000
	{ OP_STRING_LESS_THAN, "String Less Than (Boolean operator)\r\n"
		"\tTrue if the first argument is less than the second argument.\r\n\r\n"
		"Returns a boolean value.  Takes 2 string arguments." },

	// Goober5000 - added wing capability
	{ OP_IS_IFF, "Is IFF (Boolean operator)\r\n"
		"\tTrue if ship(s) or wing(s) are all of the specified team.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments...\r\n"
		"\t1:\tTeam (\"friendly\", \"hostile\", \"neutral\", or \"unknown\").\r\n"
		"\tRest:\tName of ship or wing to check." },

	// Goober5000
	{ OP_IS_AI_CLASS, "Is AI Class (Boolean operator)\r\n"
		"\tTrue if ship or ship subsystem(s) is/are all of the specified AI class.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments...\r\n"
		"\t1:\tAI class (\"None\", \"Coward\", \"Lieutenant\", etc.)\r\n"
		"\t2:\tName of ship to check.\r\n"
		"\tRest:\tName of ship subsystem(s) to check (optional)" },

	// Goober5000
	{ OP_IS_SHIP_TYPE, "Is Ship Type (Boolean operator)\r\n"
		"\tTrue if ship or ships is/are all of the specified ship type.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments...\r\n"
		"\t1:\tShip type (\"fighter\", \"bomber\", etc.)\r\n"
		"\t2:\tName of ship to check.\r\n" },

	// Goober5000
	{ OP_IS_SHIP_CLASS, "Is Ship Class (Boolean operator)\r\n"
		"\tTrue if ship or ships is/are all of the specified ship class.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments...\r\n"
		"\t1:\tShip class\r\n"
		"\t2:\tName of ship to check.\r\n" },

	{ OP_HAS_TIME_ELAPSED, "Has time elapsed (Boolean operator)\r\n"
		"\tBecomes true when the specified amount of time has elapsed (Mission time "
		"becomes greater than the specified time).\r\n"
		"Returns a boolean value.  Takes 1 numeric argument...\r\n"
		"\t1:\tThe amount of time in seconds." },

	{ OP_NOT, "Not (Boolean operator)\r\n"
		"\tReturns opposite boolean value of argument (True becomes false, and "
		"false becomes true).\r\n\r\n"
		"Returns a boolean value.  Takes 1 boolean argument." },

	{ OP_PREVIOUS_GOAL_TRUE, "Previous Mission Goal True (Boolean operator)\r\n"
		"\tReturns true if the specified goal in the specified mission is true "
		"(or succeeded).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the mission.\r\n"
		"\t2:\tName of the goal in the mission.\r\n"
		"\t3:\t(Optional) True/False which signifies what this sexpession should return when "
		"this mission is played as a single mission." },

	{ OP_PREVIOUS_GOAL_FALSE, "Previous Mission Goal False (Boolean operator)\r\n"
		"\tReturns true if the specified goal in the specified mission "
		"is false (or failed).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the mission.\r\n"
		"\t2:\tName of the goal in the mission.\r\n"
		"\t3:\t(Optional) True/False which signifies what this sexpession should return when "
		"this mission is played as a single mission." },

	{ OP_PREVIOUS_GOAL_INCOMPLETE, "Previous Mission Goal Incomplete (Boolean operator)\r\n"
		"\tReturns true if the specified goal in the specified mission "
		"is incomplete (not true or false).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the mission.\r\n"
		"\t2:\tName of the goal in the mission.\r\n"
		"\t3:\t(Optional) True/False which signifies what this sexpession should return when "
		"this mission is played as a single mission." },

	{ OP_PREVIOUS_EVENT_TRUE, "Previous Mission Event True (Boolean operator)\r\n"
		"\tReturns true if the specified event in the specified mission is true "
		"(or succeeded).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the mission.\r\n"
		"\t2:\tName of the event in the mission.\r\n"
		"\t3:\t(Optional) True/False which signifies what this sexpession should return when "
		"this mission is played as a single mission." },

	{ OP_PREVIOUS_EVENT_FALSE, "Previous Mission Event False (Boolean operator)\r\n"
		"\tReturns true if the specified event in the specified mission "
		"is false (or failed).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the mission.\r\n"
		"\t2:\tName of the event in the mission.\r\n"
		"\t3:\t(Optional) True/False which signifies what this sexpession should return when "
		"this mission is played as a single mission." },

	{ OP_PREVIOUS_EVENT_INCOMPLETE, "Previous Mission Event Incomplete (Boolean operator)\r\n"
		"\tReturns true if the specified event in the specified mission "
		"is incomplete (not true or false).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the mission.\r\n"
		"\t2:\tName of the event in the mission.\r\n"
		"\t3:\t(Optional) True/False which signifies what this sexpession should return when "
		"this mission is played as a single mission." },

	{ OP_GOAL_TRUE_DELAY, "Mission Goal True (Boolean operator)\r\n"
		"\tReturns true N seconds after the specified goal in the this mission is true "
		"(or succeeded).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the event in the mission.\r\n"
		"\t2:\tNumber of seconds to delay before returning true."},

	{ OP_GOAL_FALSE_DELAY, "Mission Goal False (Boolean operator)\r\n"
		"\tReturns true N seconds after the specified goal in the this mission is false "
		"(or failed).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the event in the mission.\r\n"
		"\t2:\tNumber of seconds to delay before returning true."},

	{ OP_GOAL_INCOMPLETE, "Mission Goal Incomplete (Boolean operator)\r\n"
		"\tReturns true if the specified goal in the this mission is incomplete.  This "
		"sexpression will only be useful in conjunction with another sexpression like"
		"has-time-elapsed.  Used alone, it will return true upon misison startup."
		"Returns a boolean value.  Takes 1 argument...\r\n"
		"\t1:\tName of the event in the mission."},

	{ OP_EVENT_TRUE_DELAY, "Mission Event True (Boolean operator)\r\n"
		"\tReturns true N seconds after the specified event in the this mission is true "
		"(or succeeded).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the event in the mission.\r\n"
		"\t2:\tNumber of seconds to delay before returning true.\r\n"
		"\t3:\t(Optional) Defaults to False. When set to false, directives will only appear as soon as the specified event is true.\r\n"
		"\t\tWhen set to true, the event only affects whether the directive succeeds/fails, and has no effect on when it appears"},

	{ OP_EVENT_FALSE_DELAY, "Mission Event False (Boolean operator)\r\n"
		"\tReturns true N seconds after the specified event in the this mission is false "
		"(or failed).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the event in the mission.\r\n"
		"\t2:\tNumber of seconds to delay before returning true.\r\n"
		"\t3:\t(Optional) Defaults to False. When set to false, directives will only appear as soon as the specified event is true.\r\n"
		"\t\tWhen set to true, the event only affects whether the directive succeeds/fails, and has no effect on when it appears"},

	{ OP_EVENT_TRUE_MSECS_DELAY, "Mission Event True (Boolean operator)\r\n"
		"\tReturns true N milliseconds after the specified event in the this mission is true "
		"(or succeeded).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the event in the mission.\r\n"
		"\t2:\tNumber of milliseconds to delay before returning true.\r\n"
		"\t3:\t(Optional) Defaults to False. When set to false, directives will only appear as soon as the specified event is true.\r\n"
		"\t\tWhen set to true, the event only affects whether the directive succeeds/fails, and has no effect on when it appears"},

	{ OP_EVENT_FALSE_MSECS_DELAY, "Mission Event False (Boolean operator)\r\n"
		"\tReturns true N milliseconds after the specified event in the this mission is false "
		"(or failed).  It returns false otherwise.\r\n\r\n"
		"Returns a boolean value.  Takes 2 required arguments and 1 optional argument...\r\n"
		"\t1:\tName of the event in the mission.\r\n"
		"\t2:\tNumber of milliseconds to delay before returning true.\r\n"
		"\t3:\t(Optional) Defaults to False. When set to false, directives will only appear as soon as the specified event is true.\r\n"
		"\t\tWhen set to true, the event only affects whether the directive succeeds/fails, and has no effect on when it appears"},

	{ OP_EVENT_INCOMPLETE, "Mission Event Incomplete (Boolean operator)\r\n"
		"\tReturns true if the specified event in the this mission is incomplete.  This "
		"sexpression will only be useful in conjunction with another sexpression like"
		"has-time-elapsed.  Used alone, it will return true upon misison startup."
		"Returns a boolean value.  Takes 1 argument...\r\n"
		"\t1:\tName of the event in the mission."},

	{ OP_IS_DESTROYED_DELAY, "Is destroyed delay (Boolean operator)\r\n"
		"\tBecomes true <delay> seconds after all specified ships have been destroyed.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments...\r\n"
		"\t1:\tTime delay in seconds (see above).\r\n"
		"\tRest:\tName of ship (or wing) to check status of." },

	{ OP_IS_SUBSYSTEM_DESTROYED_DELAY, "Is subsystem destroyed delay (Boolean operator)\r\n"
		"\tBecomes true <delay> seconds after the specified subsystem of the specified "
		"ship is destroyed.\r\n\r\n"
		"Returns a boolean value.  Takes 3 arguments...\r\n"
		"\t1:\tName of ship the subsystem we are checking is on.\r\n"
		"\t2:\tThe name of the subsystem we are checking status of.\r\n"
		"\t3:\tTime delay in seconds (see above)." },

	{ OP_IS_DISABLED_DELAY, "Is disabled delay (Boolean operator)\r\n"
		"\tBecomes true <delay> seconds after the specified ship(s) are disabled.  A "
		"ship is disabled when all of its engine subsystems are destroyed.  All "
		"ships must be diabled for this function to return true.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments...\r\n"
		"\t1:\tTime delay is seconds (see above).\r\n"
		"\tRest:\tNames of ships to check disabled status of." },

	{ OP_IS_DISARMED_DELAY, "Is disarmed delay (Boolean operator)\r\n"
		"\tBecomes true <delay> seconds after the specified ship(s) are disarmed.  A "
		"ship is disarmed when all of its turret subsystems are destroyed.  All "
		"ships must be disarmed for this function to return true.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments...\r\n"
		"\t1:\tTime delay is seconds (see above).\r\n"
		"\tRest:\tNames of ships to check disarmed status of." },

	{ OP_HAS_DOCKED_DELAY, "Has docked delay (Boolean operator)\r\n"
		"\tBecomes true <delay> seconds after the specified ships have docked the "
		"specified number of times.\r\n\r\n"
		"Returns a boolean value.  Takes 4 arguments...\r\n"
		"\t1:\tThe name of the docker ship\r\n"
		"\t2:\tThe name of the dockee ship\r\n"
		"\t3:\tThe number of times they have to have docked\r\n"
		"\t4:\tTime delay in seconds (see above)." },

	{ OP_HAS_UNDOCKED_DELAY, "Has undocked delay (Boolean operator)\r\n"
		"\tBecomes true <delay> seconds after the specified ships have undocked the "
		"specified number of times.\r\n\r\n"
		"Returns a boolean value.  Takes 4 arguments...\r\n"
		"\t1:\tThe name of the docker ship\r\n"
		"\t2:\tThe name of the dockee ship\r\n"
		"\t3:\tThe number of times they have to have undocked\r\n"
		"\t4:\tTime delay in seconds (see above)." },

	{ OP_HAS_ARRIVED_DELAY, "Has arrived delay (Boolean operator)\r\n"
		"\tBecomes true <delay> seconds after the specified ship(s) have arrived into the mission\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments...\r\n"
		"\t1:\tTime delay in seconds (see above).\r\n"
		"\tRest:\tName of ship (or wing) we want to check has arrived." },

	{ OP_HAS_DEPARTED_DELAY, "Has departed delay (Boolean operator)\r\n"
		"\tBecomes true <delay> seconds after the specified ship(s) or wing(s) have departed "
		"from the mission by warping out.  If any ship was destroyed, this operator will "
		"never be true.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments...\r\n"
		"\t1:\tTime delay in seconds (see above).\r\n"
		"\tRest:\tName of ship (or wing) we want to check has departed." },

	{ OP_WAYPOINTS_DONE_DELAY, "Waypoints done delay (Boolean operator)\r\n"
		"\tBecomes true <delay> seconds after the specified ship has completed flying the "
		"specified waypoint path.\r\n\r\n"
		"Returns a boolean value.  Takes 3 or 4 arguments...\r\n"
		"\t1:\tName of ship we are checking.\r\n"
		"\t2:\tWaypoint path we want to check if ship has flown.\r\n"
		"\t3:\tTime delay in seconds (see above).\r\n"
		"\t4:\tHow many times the ship has completed the waypoint path (optional)." },

	{ OP_SHIP_TYPE_DESTROYED, "Ship Type Destroyed (Boolean operator)\r\n"
		"\tBecomes true when the specified percentage of ship types in this mission "
		"have been destroyed.  The ship type is a generic type such as fighter/bomber, "
		"transport, etc.  Fighters and bombers count as the same type.\r\n\r\n"
		"Returns a boolean value.  Takes 2 arguments...\r\n"
		"\t1:\tPercentage of ships that must be destroyed.\r\n"
		"\t2:\tShip type to check for." },

	{ OP_TIME_SHIP_DESTROYED, "Time ship destroyed (Time operator)\r\n"
		"\tReturns the time the specified ship was destroy.\r\n\r\n"
		"Returns a numeric value.  Takes 1 argument...\r\n"
		"\t1:\tName of ship we want to check." },

	{ OP_TIME_SHIP_ARRIVED, "Time ship arrived (Time operator)\r\n"
		"\tReturns the time the specified ship arrived into the mission.\r\n\r\n"
		"Returns a numeric value.  Takes 1 argument...\r\n"
		"\t1:\tName of ship we want to check." },

	{ OP_TIME_SHIP_DEPARTED, "Time ship departed (Time operator)\r\n"
		"\tReturns the time the specified ship departed the mission by warping out.  Being "
		"destroyed doesn't count departed.\r\n\r\n"
		"Returns a numeric value.  Takes 1 argument...\r\n"
		"\t1:\tName of ship we want to check." },

	{ OP_TIME_WING_DESTROYED, "Time wing destroyed (Time operator)\r\n"
		"\tReturns the time the specified wing was destroy.\r\n\r\n"
		"Returns a numeric value.  Takes 1 argument...\r\n"
		"\t1:\tName of wing we want to check." },

	{ OP_TIME_WING_ARRIVED, "Time wing arrived (Time operator)\r\n"
		"\tReturns the time the specified wing arrived into the mission.\r\n\r\n"
		"Returns a numeric value.  Takes 1 argument...\r\n"
		"\t1:\tName of wing we want to check." },

	{ OP_TIME_WING_DEPARTED, "Time wing departed (Time operator)\r\n"
		"\tReturns the time the specified wing departed the mission by warping out.  All "
		"ships in the wing have to have warped out.  If any are destroyed, the wing can "
		"never be considered departed.\r\n\r\n"
		"Returns a numeric value.  Takes 1 argument...\r\n"
		"\t1:\tName of ship we want to check." },

	{ OP_MISSION_TIME, "Mission time (Time operator)\r\n"
		"\tReturns the current time into the mission.\r\n\r\n"
		"Returns a numeric value.  Takes no arguments." },

	{ OP_MISSION_TIME_MSECS, "Mission time, in milliseconds (Time operator)\r\n"
		"\tReturns the current time into the mission, in milliseconds.  Useful for more fine-grained timing than possible with normal second-based sexps.  (Tip: when an event occurs, assign the result of mission-time-msecs to a variable.  Then, in another event, wait until mission-time-msecs is greater than the value of that variable plus some delay amount.  This second event should be chained or coupled with additional conditions so that it doesn't accidentally fire on an uninitialized variable!)\r\n\r\n"
		"Returns a numeric value.  Takes no arguments." },

	{ OP_TIME_DOCKED, "Time docked (Time operator)\r\n"
		"\tReturns the time the specified ships docked.\r\n\r\n"
		"Returns a numeric value.  Takes 3 arguments...\r\n"
		"\t1:\tThe name of the docker ship.\r\n"
		"\t2:\tThe name of the dockee ship.\r\n"
		"\t3:\tThe number of times they must have docked to be true." },

	{ OP_TIME_UNDOCKED, "Time undocked (Time operator)\r\n"
		"\tReturns the time the specified ships undocked.\r\n\r\n"
		"Returns a numeric value.  Takes 3 arguments...\r\n"
		"\t1:\tThe name of the docker ship.\r\n"
		"\t2:\tThe name of the dockee ship.\r\n"
		"\t3:\tThe number of times they must have undocked to be true." },

	{ OP_AFTERBURNER_LEFT, "Afterburner left\r\n"
		"\tReturns a ship's current engine energy as a percentage.\r\n"
		"\t1: Ship name\r\n" },

	{ OP_WEAPON_ENERGY_LEFT, "Weapon energy left\r\n"
		"\tReturns a ship's current weapon energy as a percentage.\r\n"
		"\t1: Ship name\r\n" },

	{ OP_SHIELDS_LEFT, "Shields left (Status operator)\r\n"
		"\tReturns the current level of the specified ship's shields as a percentage.\r\n\r\n"
		"Returns a numeric value.  Takes 1 argument...\r\n"
		"\t1:\tName of ship to check." },

	{ OP_HITS_LEFT, "Hits left (Status operator)\r\n"
		"\tReturns the current level of the specified ship's hull as a percentage.\r\n\r\n"
		"Returns a numeric value.  Takes 1 argument...\r\n"
		"\t1:\tName of ship to check." },

	{ OP_HITS_LEFT_SUBSYSTEM, "Hits left subsystem (status operator, deprecated)\r\n"
		"\tReturns the current level of the specified ship's subsystem integrity as a percentage of the damage done to *all "
		"subsystems of the same type*.  This operator provides the same functionality as the new hits-left-subsystem-generic "
		"operator, except that it gets the subsystem type in a very misleading way.  Common consensus among SCP programmers is "
		"that this operator was intended to behave like hits-left-subsystem-specific but was programmed incorrectly.  As such, "
		"this operator is deprecated.  Mission designers are strongly encouraged to use hits-left-subsystem-specific rather than "
		"the optional boolean parameter.\r\n\r\n"
		"Returns a numeric value.  Takes 2 or 3 arguments...\r\n"
		"\t1:\tName of ship to check.\r\n"
		"\t2:\tName of subsystem on ship to check.\r\n"
		"\t3:\t(Optional) True/False. When set to true only the subsystem supplied will be tested; when set to false (the default), "
		"all subsystems of that type will be tested." },

	{ OP_HITS_LEFT_SUBSYSTEM_GENERIC, "hits-left-subsystem-generic (status operator)\r\n"
		"\tReturns the current level of integrity of a generic subsystem type, as a percentage.  A \"generic subsystem type\" "
		"is a subsystem *category*, (for example, Engines), that includes one or more *individual* subsystems (for example, engine01, "
		"engine02, and engine03) on a ship.\r\n\r\nThis is the way FreeSpace tests certain subsystem thresholds internally; for "
		"example, if the integrity of all engine subsystems (that is, the combined strength of all engines divided by the maximum "
		"total strength of all engines) is less than 30%, the player cannot warp out.\r\n\r\n"
		"Returns a numeric value.  Takes 2 arguments...\r\n"
		"\t1:\tName of ship to check\r\n"
		"\t2:\tName of subsystem type to check\r\n" },

	{ OP_HITS_LEFT_SUBSYSTEM_SPECIFIC, "hits-left-subsystem-specific (status operator)\r\n"
		"\tReturns the current level of integrity of a specific subsystem, as a percentage.\r\n\r\n(If you were looking for the old "
		"hits-left-subsystem operator, this is almost certainly the operator you want.  The hits-left-subsystem operator "
		"suffers from a serious design flaw that causes it to behave like hits-left-subsystem-generic.  As such it has been deprecated "
		"and will not appear in the operator list; it can only be used if you type it in manually.  Old missions using hits-left-subsystem "
		"will still work, but mission designers are strongly encouraged to use the new operators instead.)\r\n\r\n"
		"Returns a numeric value.  Takes 2 arguments...\r\n"
		"\t1:\tName of ship to check\r\n"
		"\t2:\tName of subsystem to check\r\n" },

	{ OP_SIM_HITS_LEFT, "Simulated Hits left (Status operator)\r\n"
		"\tReturns the current level of the specified ship's simulated hull as a percentage.\r\n\r\n"
		"Returns a numeric value.  Takes 1 argument...\r\n"
		"\t1:\tName of ship to check." },

	{ OP_DISTANCE, "Distance (Status operator)\r\n"
		"\tReturns the distance between two objects.  These objects can be either a ship, "
		"a wing, or a waypoint.\r\n"
		"When a wing or team is given (for either argument) the answer will be the shortest distance. \r\n\r\n"
		"Returns a numeric value.  Takes 2 arguments...\r\n"
		"\t1:\tThe name of one of the objects.\r\n"
		"\t2:\tThe name of the other object." },

	{ OP_DISTANCE_SUBSYSTEM, "Distance from ship subsystem (Status operator)\r\n"
		"\tReturns the distance between an object and a ship subsystem.  The object can be either a ship, "
		"a wing, or a waypoint.\r\n\r\n"
		"Returns a numeric value.  Takes 3 arguments...\r\n"
		"\t1:\tThe name of the object.\r\n"
		"\t2:\tThe name of the ship which houses the subsystem.\r\n"
		"\t3:\tThe name of the subsystem." },

	{ OP_NUM_WITHIN_BOX, "Number of specified objects in the box specified\r\n"
		"\t1: Box center (X)\r\n"
		"\t2: Box center (Y)\r\n"
		"\t3: Box center (Z)\r\n"
		"\t4: Box width\r\n"
		"\t5: Box height\r\n"
		"\t6: Box depth\r\n"
		"\tRest:\tShips or wings to check" },

	{ OP_IS_IN_BOX, "Whether an object is in the box specified. If a second ship is specified, "
		"the box is relative to that ship's reference frame. \r\n"
		"\t1: Ships or wings to check\r\n"
		"\t2: Min X\r\n"
		"\t3: Max X\r\n"
		"\t4: Min Y\r\n"
		"\t5: Max Y\r\n"
		"\t6: Min Z\r\n"
		"\t7: Max Z\r\n"
		"\t8: Ship to use as reference frame (optional)." },

	{ OP_IS_IN_MISSION, "Checks whether a given ship is presently in the mission.  This sexp doesn't check the arrival list or exited status; it only tests to see if the "
		"ship is active.  This means that internally the sexp only returns SEXP_TRUE or SEXP_FALSE and does not use any of the special shortcut values.  This is useful "
		"for ships created with ship-create, as those ships will not have used the conventional ship arrival list.\r\n\r\n"
		"Takes 1 or more string arguments, which are checked against the ship list." },

	{ OP_GET_DAMAGE_CAUSED, "Get damage caused (Status operator)\r\n"
		"\tReturns the amount of damage one or more ships have done to a ship.\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tShip that has been damaged.\r\n"
		"\t2:\tName of ships that may have damaged it." },

	{ OP_LAST_ORDER_TIME, "Last order time (Status operator)\r\n"
		"\tReturns true if <count> seconds have elapsed since one or more ships have received "
		"a meaningful order from the player.  A meaningful order is currently any order that "
		"is not the warp out order.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments...\r\n"
		"\t1:\tTime in seconds that must elapse.\r\n"
		"\tRest:\tName of ship or wing to check for having received orders." },

	{ OP_WHEN, "When (Conditional operator)\r\n"
		"\tPerforms specified actions when a condition becomes true\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tBoolean expression that must be true for actions to take place.\r\n"
		"\tRest:\tActions to take when boolean expression becomes true." },

	{ OP_COND, "Blah" },

	// Goober5000
	{ OP_WHEN_ARGUMENT, "When-argument (Conditional operator)\r\n"
		"\tPerforms specified actions when a condition, given a set of arguments, becomes true.\r\n\r\n"
		"Takes 3 or more arguments...\r\n"
		"\t1:\tThe arguments to evaluate (see any-of, every-of, random-of, etc.).\r\n"
		"\t2:\tBoolean expression that must be true for actions to take place.\r\n"
		"\tRest:\tActions to take when the boolean expression becomes true." },

	// Goober5000
	{ OP_EVERY_TIME, "Every-time (Conditional operator)\r\n"
		"\tThis is a version of \"when\" that will always evaluate its arguments.  It's useful "
		"in situations where you need to repeatedly check things that may become true more than "
		"once.  Since this sexp will execute every time it's evaluated, you may need to use it as "
		"an argument to \"when\" if you want to impose restrictions on how it's called.\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tBoolean expression that must be true for actions to take place.\r\n"
		"\tRest:\tActions to take when boolean expression is true." },

	// Goober5000
	{ OP_EVERY_TIME_ARGUMENT, "Every-time-argument (Conditional operator)\r\n"
		"\tThis is a version of \"when-argument\" that will always evaluate its arguments.  It's useful "
		"in situations where you need to repeatedly check things that may become true more than "
		"once.  Since this sexp will execute every time it's evaluated, you may need to use it as "
		"an argument to \"when\" (not \"when-argument\") if you want to impose restrictions on how it's called.\r\n\r\n"
		"Takes 3 or more arguments...\r\n"
		"\t1:\tThe arguments to evaluate (see any-of, all-of, random-of, etc.).\r\n"
		"\t2:\tBoolean expression that must be true for actions to take place.\r\n"
		"\tRest:\tActions to take when the boolean expression becomes true." },

	// Goober5000
	{ OP_IF_THEN_ELSE, "If-then-else (Conditional operator)\r\n"
		"\tPerforms one action if a condition is true (like \"when\"), or another action (or set of actions) if the condition is false.  "
		"Note that this sexp only completes one of its branches once the condition has been determined; "
		"it does not come back later and evaluate the other branch if the condition happens to switch truth values.\r\n\r\n"
		"Takes 3 or more arguments...\r\n"
		"\t1:\tBoolean expression to evaluate.\r\n"
		"\t2:\tActions to take if that expression becomes true.\r\n"
		"\tRest:\tActions to take if that expression becomes false.\r\n" },

	// Karajorma
	{ OP_DO_FOR_VALID_ARGUMENTS, "Do-for-valid-arguments (Conditional operator)\r\n"
		"\tPerforms specified actions once for each valid " SEXP_ARGUMENT_STRING " in the parent conditional.\r\n"
		"\tMust not be used for any SEXP that actually contains " SEXP_ARGUMENT_STRING " as these are already being executed\r\n"
		"\tmultiple times without using Do-for-valid-arguments. Any use of "  SEXP_ARGUMENT_STRING " and will \r\n" 
		"\tprevent execution of the entire SEXP unless it is nested inside another when(or every-time)-argument SEXP.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tActions to take." },

	// Karajorma
	{ OP_NUM_VALID_ARGUMENTS, "num-valid-arguments (Conditional operator)\r\n"
		"\tReturns the number of valid arguments in the argument list.\r\n\r\n"
		"Takes no arguments...\r\n"},

	// Goober5000
	{ OP_ANY_OF, "Any-of (Conditional operator)\r\n"
		"\tSupplies arguments for the " SEXP_ARGUMENT_STRING " special data item.  Any of the supplied arguments can satisfy the expression(s) "
		"in which " SEXP_ARGUMENT_STRING " is used.\r\n\r\n"
		"In practice, this will behave like a standard \"for-each\" statement, evaluating the action operators for each argument that satisfies the condition.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tAnything that could be used in place of " SEXP_ARGUMENT_STRING "." },

	// Goober5000
	{ OP_EVERY_OF, "Every-of (Conditional operator)\r\n"
		"\tSupplies arguments for the " SEXP_ARGUMENT_STRING " special data item.  Every one of the supplied arguments will be evaluated to satisfy the expression(s) "
		"in which " SEXP_ARGUMENT_STRING " is used.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tAnything that could be used in place of " SEXP_ARGUMENT_STRING "." },

	// Goober5000
	{ OP_RANDOM_OF, "Random-of (Conditional operator)\r\n"
		"\tSupplies arguments for the " SEXP_ARGUMENT_STRING " special data item.  A random supplied argument will be selected to satisfy the expression(s) "
		"in which " SEXP_ARGUMENT_STRING " is used. The same argument will be returned by all subsequent calls\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tAnything that could be used in place of " SEXP_ARGUMENT_STRING "." },

	// Karajorma
	{ OP_RANDOM_MULTIPLE_OF, "Random-multiple-of (Conditional operator)\r\n"
		"\tSupplies arguments for the " SEXP_ARGUMENT_STRING " special data item.  A random supplied argument will be selected to satisfy the expression(s) "
		"in which " SEXP_ARGUMENT_STRING " is used.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tAnything that could be used in place of " SEXP_ARGUMENT_STRING "." },

	// Goober5000
	{ OP_NUMBER_OF, "Number-of (Conditional operator)\r\n"
		"\tSupplies arguments for the " SEXP_ARGUMENT_STRING " special data item.  Any [number] of the supplied arguments can satisfy the expression(s) "
		"in which " SEXP_ARGUMENT_STRING " is used.\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tNumber of arguments, as above\r\n"
		"\tRest:\tAnything that could be used in place of " SEXP_ARGUMENT_STRING "." },

	// Karajorma
	{ OP_IN_SEQUENCE, "In-Sequence (Conditional operator)\r\n"
		"\tSupplies arguments for the " SEXP_ARGUMENT_STRING " special data item.  The first argument in the list will be selected to satisfy the expression(s) "
		"in which " SEXP_ARGUMENT_STRING " is used. The same argument will be returned by all subsequent calls\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tAnything that could be used in place of " SEXP_ARGUMENT_STRING "." },

	// Goober5000
	{ OP_FOR_COUNTER, "For-Counter (Conditional operator)\r\n"
		"\tSupplies counter values for the " SEXP_ARGUMENT_STRING " special data item.  This sexp will count up from the start value to the stop value, and each value will be provided as an argument to the action operators.  "
		"The default increment is 1, but if the optional increment parameter is provided, the counter will increment by that number.  The stop value will be supplied if appropriate; e.g. counting from 0 to 10 by 2 will supply 0, 2, 4, 6, 8, and 10; "
		"but counting by 3 will supply 0, 3, 6, and 9.\r\n\r\n"
		"Note that the counter values are all treated as valid arguments, and it is impossible to invalidate a counter argument.  If you want to invalidate a counter value, use Any-of and list the values explicitly.\r\n\r\n"
		"This sexp will usually need to be accompanied by the string-to-int sexp, as the counter variables are provided in string format but are most useful in integer format.\r\n\r\n"
		"Takes 2 or 3 arguments...\r\n"
		"\t1:\tCounter start value\r\n"
		"\t2:\tCounter stop value\r\n"
		"\t3:\tCounter increment (optional)" },

	// Goober5000
	{ OP_INVALIDATE_ARGUMENT, "Invalidate-argument (Conditional operator)\r\n"
		"\tRemoves an argument from future consideration as a " SEXP_ARGUMENT_STRING " special data item.\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tThe argument to remove from the preceding argument list." },

	// Karajorma
	{ OP_VALIDATE_ARGUMENT, "Validate-argument (Conditional operator)\r\n"
		"\tRestores an argument for future consideration as a " SEXP_ARGUMENT_STRING " special data item.\r\n"
		"\tIf the argument hasn't been previously invalidated, it will do nothing.\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tThe argument to restore to the preceding argument list." },

	// Karajorma
	{ OP_INVALIDATE_ALL_ARGUMENTS, "Invalidate-all-arguments (Conditional operator)\r\n"
		"\tRemoves all argument from future consideration as " SEXP_ARGUMENT_STRING " special data items.\r\n"
		"Takes no arguments." },

	// Karajorma
	{ OP_VALIDATE_ALL_ARGUMENTS, "Validate-all-arguments (Conditional operator)\r\n"
		"\tRestores all arguments for future consideration as " SEXP_ARGUMENT_STRING " special data items.\r\n"
		"\tIf the argument hasn't been previously invalidated, it will do nothing.\r\n"
		"Takes no arguments." },

	// Goober5000 - added wing capability
	{ OP_CHANGE_IFF, "Change IFF (Action operator)\r\n"
		"\tSets the specified ship(s) or wing(s) to the specified team.\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tTeam to change to (\"friendly\", \"hostile\" or \"unknown\").\r\n"
		"\tRest:\tName of ship or wing to change team status of." },

	// Wanderer
	{ OP_CHANGE_IFF_COLOR, "Change IFF Color (Action operator)\r\n"
		"\tSets the specified ship(s) or wing(s) apparent color.\r\n"
		"Takes 6 or more arguments...\r\n"
		"\t1:\tName of the team from which target is observed from.\r\n"
		"\t2:\tName of the team of the observed target to receive the alternate color.\r\n"
		"\t3:\tRed color (value from 0 to 255).\r\n"
		"\t4:\tGreen color (value from 0 to 255).\r\n"
		"\t5:\tBlue color (value from 0 to 255).\r\n"
		"\tRest:\tName of ship or wing to change team status of." },

	// Goober5000
	{ OP_CHANGE_AI_CLASS, "Change AI Class (Action operator)\r\n"
		"\tSets the specified ship or ship subsystem(s) to the specified ai class.\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tAI Class to change to (\"None\", \"Coward\", \"Lieutenant\", etc.)\r\n"
		"\t2:\tName of ship to change AI class of\r\n"
		"\tRest:\tName of subsystem to change AI class of (optional)" },

	{ OP_MODIFY_VARIABLE, "Modify-variable (Misc. operator)\r\n"
		"\tModifies variable to specified value\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of Variable.\r\n"
		"\t2:\tValue to be set." },

	{ OP_GET_VARIABLE_BY_INDEX, "variable-array-get\r\n"
		"\tGets the value of the variable specified by the given index.  This is an alternate way "
		"to access variables rather than by their names, and it enables cool features such as "
		"arrays and pointers.\r\n\r\nPlease note that only numeric variables are supported.  Any "
		"attempt to access a string variable will result in a value of SEXP_NAN_FOREVER being returned.\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tIndex of variable, from 0 to 99." },

	{ OP_SET_VARIABLE_BY_INDEX, "variable-array-set\r\n"
		"\tSets the value of the variable specified by the given index.  This is an alternate way "
		"to modify variables rather than by their names, and it enables cool features such as "
		"arrays and pointers.\r\n\r\nIn contrast to variable-array-get, note that this sexp "
		"*does* allow the modification of string variables.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tIndex of variable, from 0 to 99.\r\n"
		"\t2:\tValue to be set." },

	{ OP_PROTECT_SHIP, "Protect ship (Action operator)\r\n"
		"\tProtects a ship from being attacked by any enemy ship.  Any ship "
		"that is protected will not come under enemy fire.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ship(s) to protect." },

	{ OP_UNPROTECT_SHIP, "Unprotect ship (Action operator)\r\n"
		"\tUnprotects a ship from being attacked by any enemy ship.  Any ship "
		"that is not protected can come under enemy fire.  This function is the opposite "
		"of protect-ship.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ship(s) to unprotect." },

	{ OP_BEAM_PROTECT_SHIP, "Beam Protect ship (Action operator)\r\n"
		"\tProtects a ship from being attacked with beam weapon.  Any ship "
		"that is beam protected will not come under enemy beam fire.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ship(s) to protect." },

	{ OP_BEAM_UNPROTECT_SHIP, "Beam Unprotect ship (Action operator)\r\n"
		"\tUnprotects a ship from being attacked with beam weapon.  Any ship "
		"that is not beam protected can come under enemy beam fire.  This function is the opposite "
		"of beam-protect-ship.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ship(s) to unprotect." },

	// Goober5000
	{ OP_TURRET_PROTECT_SHIP, "Turret Protect ship (Action operator)\r\n"
		"\tProtects a ship from being attacked with a turret weapon of a given type.  Any ship "
		"that is turret protected will not come under enemy fire from that type of turret, though it may come under fire by other turrets.\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tType of turret (currently supported types are \"beam\", \"flak\", \"laser\", and \"missile\")\r\n"
		"\tRest:\tName of ship(s) to protect." },

	// Goober5000
	{ OP_TURRET_UNPROTECT_SHIP, "Turret Unprotect ship (Action operator)\r\n"
		"\tUnprotects a ship from being attacked with a turret weapon of a given type.  Any ship "
		"that is not turret protected can come under enemy fire from that type of turret.  This function is the opposite "
		"of turret-protect-ship.\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tType of turret (currently supported types are \"beam\", \"flak\", \"laser\", and \"missile\")\r\n"
		"\tRest:\tName of ship(s) to unprotect." },

	{ OP_SEND_MESSAGE, "Send message (Action operator)\r\n"
		"\tSends a message to the player.  Can be send by a ship, wing, or special "
		"source.  To send it from a special source, make the first character of the first "
		"argument a \"#\".\r\n\r\n"
		"Takes 3 arguments...\r\n"
		"\t1:\tName of who the message is from.\r\n"
		"\t2:\tPriority of message (\"Low\", \"Normal\" or \"High\").\r\n"
		"\t3:\tName of message (from message editor)." },

	// Karajorma	
	{ OP_ENABLE_BUILTIN_MESSAGES, "Enable builtin messages (Action operator)\r\n"
		"\tTurns the built in messages sent by command or pilots on\r\n"
		"Takes 0 or more arguments...\r\n"
		"If no arguments are supplied any ships not given individual silence orders will be able\r\n"
		"to send built in messages. Command will also be unsilenced\r\n"
		"Using the Any Wingman option cancels radio silence for all ships in wings.\r\n"
		"\tAll:\tName of ship to allow to talk." },
		
	// Karajorma
	{ OP_DISABLE_BUILTIN_MESSAGES, "Disable builtin messages (Action operator)\r\n"
		"\tTurns the built in messages sent by command or pilots off\r\n"
		"Takes 0 or more arguments....\r\n"
		"If no arguments are supplied all built in messages are disabled.\r\n"
		"Using the Any Wingman option silences for all ships in wings.\r\n"
		"\tAll:\tName of ship to be silenced." },

	// Karajorma	
	{ OP_SET_PERSONA, "Set Persona (Action operator)\r\n"
		"\tSets the persona of the supplied ship to the persona supplied\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tPersona to use."
		"\tRest:\tName of the ship." },

	{ OP_SELF_DESTRUCT, "Self destruct (Action operator)\r\n"
		"\tCauses the specified ship(s) to self destruct.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ship to self destruct." },

	{ OP_NEXT_MISSION, "Next Mission (Action operator)\r\n"
		"\tThe next mission operator is used for campaign branching in the campaign editor.  "
		"It specifies which mission should played be next in the campaign.  This operator "
		"generally follows a 'when' or 'cond' statment in the campaign file.\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tName of mission (filename) to proceed to." },

	{ OP_CLEAR_GOALS, "Clear goals (Action operator)\r\n"
		"\tClears the goals for the specified ships and/or wings.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ship or wing." },

	{ OP_ADD_GOAL, "Add goal (Action operator)\r\n"
		"\tAdds a goal to a ship or wing.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of ship or wing to add goal to.\r\n"
		"\t2:\tGoal to add." },

	// Goober5000
	{ OP_REMOVE_GOAL, "Remove goal (Action operator)\r\n"
		"\tRemoves a goal from a ship or wing.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of ship or wing to remove goal from.\r\n"
		"\t2:\tGoal to remove." },

	{ OP_SABOTAGE_SUBSYSTEM, "Sabotage subystem (Action operator)\r\n"
		"\tReduces the specified subsystem integrity by the specified percentage."
		"If the percntage strength of the subsystem (after completion) is less than 0%,"
		"subsystem strength is set to 0%.\r\n\r\n"
		"Takes 3 arguments...\r\n"
		"\t1:\tName of ship subsystem is on.\r\n"
		"\t2:\tName of subsystem to sabotage.\r\n"
		"\t3:\tPercentage to reduce subsystem integrity by." },

	{ OP_REPAIR_SUBSYSTEM, "Repair Subystem (Action operator)\r\n"
		"\tIncreases the specified subsystem integrity by the specified percentage."
		"If the percntage strength of the subsystem (after completion) is greater than 100%,"
		"subsystem strength is set to 100%.\r\n\r\n"
		"Takes 4 arguments...\r\n"
		"\t1:\tName of ship subsystem is on.\r\n"
		"\t2:\tName of subsystem to repair.\r\n"
		"\t3:\tPercentage to increase subsystem integrity by.\r\n"
		"\t4:\tRepair turret submodel.  Optional argument that defaults to true."},

	{ OP_SET_SUBSYSTEM_STRNGTH, "Set Subsystem Strength (Action operator)\r\n"
		"\tSets the specified subsystem to the the specified percentage."
		"If the percentage specified is < 0, strength is set to 0.  If the percentage is "
		"> 100 % the subsystem strength is set to 100%.\r\n\r\n"
		"Takes 3 arguments...\r\n"
		"\t1:\tName of ship subsystem is on.\r\n"
		"\t2:\tName of subsystem to set strength.\r\n"
		"\t3:\tPercentage to set subsystem integrity to.\r\n" 
		"\t4:\tRepair turret submodel.  Optional argument that defaults to true."},

	{ OP_INVALIDATE_GOAL, "Invalidate goal (Action operator)\r\n"
		"\tMakes a mission goal invalid, which causes it to not show up on mission goals "
		"screen, or be evaluated.\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of mission goal to invalidate." },

	{ OP_VALIDATE_GOAL, "Validate goal (Action operator)\r\n"
		"\tMakes a mission goal valid again, so it shows up on mission goals screen.\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of mission goal to validate." },

	{ OP_SEND_RANDOM_MESSAGE, "Send random message (Action operator)\r\n"
		"\tSends a random message to the player from those supplied.  Can be send by a "
		"ship, wing, or special source.  To send it from a special source, make the first "
		"character of the first argument a \"#\".\r\n\r\n"
		"Takes 3 or more arguments...\r\n"
		"\t1:\tName of who the message is from.\r\n"
		"\t2:\tPriority of message (\"Low\", \"Normal\" or \"High\")."
		"\tRest:\tName of message (from message editor)." },

	{ OP_TRANSFER_CARGO, "Transfer Cargo (Action operator)\r\n"
		"\tTransfers the cargo from one ship to another ship.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of ship that cargo is being transferred from.\r\n"
		"\t2:\tName of ship that cargo is being transferred to." },

	{ OP_EXCHANGE_CARGO, "Exchange Cargo (Action operator)\r\n"
		"\tExchanges the cargos of two ships.  If one of the two ships contains no cargo, "
		"the cargo is transferred instead.\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of one of the ships.\r\n"
		"\t2:\tName of the other ship." },

	// Goober5000
	{ OP_SET_CARGO, "Set Cargo (Action operator)\r\n"
		"\tSets the cargo on a ship or ship subsystem.  The cargo-no-deplete flag status is carried through to the new cargo.\r\n"
		"Takes 2 or 3 arguments...\r\n"
		"\t1:\tName of the cargo\r\n"
		"\t2:\tName of the ship\r\n"
		"\t3:\tName of the ship subsystem (optional)" },

	// Goober5000
	{ OP_IS_CARGO, "Is Cargo (Status operator)\r\n"
		"\tChecks whether the specified ship or ship subsystem contains a particular cargo.\r\n"
		"Takes 2 or 3 arguments...\r\n"
		"\t1:\tName of the cargo\r\n"
		"\t2:\tName of the ship\r\n"
		"\t3:\tName of the ship subsystem (optional)" },

	// Goober5000
	{ OP_CHANGE_SOUNDTRACK, "change-soundtrack\r\n"
		"\tChanges the mission music.  Takes 1 argument...\r\n"
		"\t1: Name of the music selection (taken from music.tbl)" },

	// Goober5000
	{ OP_PLAY_SOUND_FROM_TABLE, "play-sound-from-table\r\n"
		"\tPlays a sound listed in the Game Sounds section of sounds.tbl.  Takes 4 arguments...\r\n"
		"\t1: Origin X\r\n"
		"\t2: Origin Y\r\n"
		"\t3: Origin Z\r\n"
		"\t4: Sound (index into sounds.tbl)" },

	// Goober5000
	{ OP_PLAY_SOUND_FROM_FILE, "play-sound-from-file\r\n"
		"\tPlays a sound, such as a music soundtrack, from a file.  Important: Only one sound at a time can be played with this sexp!\r\n"
		"Takes 1 to 3 arguments...\r\n"
		"\t1: Sound (file name)\r\n"
		"\t2: Enter a non-zero number to loop. default is off (optional).\r\n"
		"\t3: Enter a non-zero number to use environment effects. default is off (optional)."
	},

	// Goober5000
	{ OP_CLOSE_SOUND_FROM_FILE, "close-sound-from-file\r\n"
		"\tCloses the currently playing sound started by play-sound-from-file, if there is any.  Takes 1 argument...\r\n"
		"\t1: Fade (default is true)" },

	// Taylor
	{ OP_SET_SOUND_ENVIRONMENT, "set-sound-environment\r\n"
		"Sets the EAX environment for all sound effects.  Optionally sets one or more parameters specific to the environment.  Takes 1 or more arguments...\r\n"
		"\t1:\tSound environment name (a value of \"" SEXP_NONE_STRING "\" will disable the effects)\r\n"
		"\t2:\tEnvironment option (optional)\r\n"
		"\t3:\tEnvironment value x 1000, e.g. 10 is 0.01 (optional)\r\n"
		"Use Add-Data to specify additional environment options in repeating option-value pairs, just like Send-Message-List can have additional messages in source-priority-message-delay groups.\r\n\r\n"
		"IMPORTANT: each additional option in the list MUST HAVE two entries; any option without the two proper fields will be ignored, as will any successive options." },

	// Taylor
	{ OP_UPDATE_SOUND_ENVIRONMENT, "update-sound-environment\r\n"
		"Updates the current EAX environment with new values.  Takes 2 or more arguments...\r\n"
		"\t1:\tEnvironment option\r\n"
		"\t2:\tEnvironment value x 1000, e.g. 10 is 0.01\r\n"
		"Use Add-Data to specify additional environment options in repeating option-value pairs, just like Send-Message-List can have additional messages in source-priority-message-delay groups.\r\n\r\n"
		"IMPORTANT: Each additional option in the list MUST HAVE two entries; any option without the two proper fields will be ignored, as will any successive options." },
	
	//The E
	{ OP_ADJUST_AUDIO_VOLUME, "adjust-audio-volume\r\n"
		"Adjusts the relative volume of one sound type. Takes 1 to 3 arguments....\r\n"
		"\t1:\tSound Type to adjust, either Music, Voice or Effects. Will act as a reset for the given category, if no other argument present\r\n"
		"\t2:\tPercentage of the users' settings to adjust to (optional), 0 will be silence, 100 means the maximum volume as set by the user\r\n"
		"\t3:\tFade time (optional), time in milliseconds to adjust the volume"},

	// Goober5000
	{ OP_SET_EXPLOSION_OPTION, "set-explosion-option\r\n"
		"Sets an explosion option on a particular ship.  Takes 3 or more arguments...\r\n"
		"\t1:\tShip name\r\n"
		"\t2:\tExplosion option\r\n"
		"\t3:\tExplosion value (for shockwave speed, 0 will produce no shockwave; for death roll time, 0 will use the default time)\r\n"
		"Use Add-Data to specify additional explosion options in repeating option-value pairs, just like Send-Message-List can have additional messages in source-priority-message-delay groups.\r\n\r\n"
		"IMPORTANT: Each additional option in the list MUST HAVE two entries; any option without the two proper fields will be ignored, as will any successive options." },

	// Goober5000
	{ OP_EXPLOSION_EFFECT, "explosion-effect\r\n"
		"\tCauses an explosion at a given origin, with the given parameters.  "
		"Takes 11 or 13 arguments...\r\n"
		"\t1:  Origin X\r\n"
		"\t2:  Origin Y\r\n"
		"\t3:  Origin Z\r\n"
		"\t4:  Damage\r\n"
		"\t5:  Blast force\r\n"
		"\t6:  Size of explosion (if 0, explosion will not be visible)\r\n"
		"\t7:  Inner radius to apply damage (if 0, explosion will not be visible)\r\n"
		"\t8:  Outer radius to apply damage (if 0, explosion will not be visible)\r\n"
		"\t9:  Shockwave speed (if 0, there will be no shockwave)\r\n"
		"\t10: Type - For backward compatibility 0 = medium, 1 = large1 (4th in table), 2 = large2 (5th in table)\r\n"
		"           3 or greater link to respctive entry in fireball.tbl\r\n"
		"\t11: Sound (index into sounds.tbl)\r\n"
		"\t12: EMP intensity (optional)\r\n"
		"\t13: EMP duration in seconds (optional)" },

	// Goober5000
	{ OP_WARP_EFFECT, "warp-effect\r\n"
		"\tCauses a subspace warp effect at a given origin, facing toward a given location, with the given parameters.\r\n"
		"Takes 12 arguments...\r\n"
		"\t1:  Origin X\r\n"
		"\t2:  Origin Y\r\n"
		"\t3:  Origin Z\r\n"
		"\t4:  Location X\r\n"
		"\t5:  Location Y\r\n"
		"\t6:  Location Z\r\n"
		"\t7:  Radius\r\n"
		"\t8:  Duration in seconds (values smaller than 4 are ignored)\r\n"
		"\t9:  Warp opening sound (index into sounds.tbl)\r\n"
		"\t10: Warp closing sound (index into sounds.tbl)\r\n"
		"\t11: Type (0 for standard blue [default], 1 for Knossos green)\r\n"
		"\t12: Shape (0 for 2-D [default], 1 for 3-D)" },

	// Goober5000
	{ OP_SET_OBJECT_FACING, "set-object-facing\r\n"
		"\tSets an object's orientation to face the specified coordinates.  "
		"Takes 4 arguments...\r\n"
		"\t1: The name of an object.\r\n"
		"\t2: The X coordinate to face.\r\n"
		"\t3: The Y coordinate to face.\r\n"
		"\t4: The Z coordinate to face.\r\n"
		"\t5: Turn time in milliseconds (optional)\r\n"
		"\t6: Bank (optional). Enter a non-zero value to enable banking." },

	// Goober5000
	{ OP_SET_OBJECT_FACING_OBJECT, "set-object-facing-object\r\n"
		"\tSets an object's orientation to face the specified object.  "
		"Takes 2 arguments...\r\n"
		"\t1: The name of an object.\r\n"
		"\t2: The object to face.\r\n"
		"\t3: Turn time in milliseconds (optional)\r\n"
		"\t4: Bank (optional). Enter a non-zero value to enable banking." },

	// Wanderer
	{ OP_SHIP_MANEUVER, "ship-maneuver\r\n"
		"\tCombines the effects of the ship-rot-maneuver and ship-lat-maneuver sexps.  Takes 10 arguments:\r\n"
		"\t1: The name of a ship\r\n"
		"\t2: Duration of the maneuver, in milliseconds\r\n"
		"\t3: Heading movement velocity, as a percentage (-100 to 100) of the tabled maximum heading velocity, or 0 to not modify the ship's current value\r\n"
		"\t4: Pitch movement velocity, as a percentage (-100 to 100) of the tabled maximum pitch velocity, or 0 to not modify the ship's current value\r\n"
		"\t5: Bank movement velocity, as a percentage (-100 to 100) of the tabled maximum bank velocity, or 0 to not modify the ship's current value\r\n"
		"\t6: Whether to apply all of the rotational velocity values even if any of them are 0\r\n"
		"\t7: Vertical movement velocity, as a percentage (-100 to 100) of the tabled maximum vertical velocity, or 0 to not modify the ship's current value\r\n"
		"\t8: Sideways movement velocity, as a percentage (-100 to 100) of the tabled maximum sideways velocity, or 0 to not modify the ship's current value\r\n"
		"\t9: Forward movement velocity, as a percentage (-100 to 100) of the tabled maximum forward velocity, or 0 to not modify the ship's current value\r\n"
		"\t10: Whether to apply all of the lateral velocity values even if any of them are 0\r\n" },

	// Wanderer
	{ OP_SHIP_ROT_MANEUVER, "ship-rot-maneuver\r\n"
		"\tCauses a ship to move in a rotational direction.  For the purposes of this sexp, this means the ship rotates along its own heading, pitch, or bank axis (or a combination of axes) without regard to normal ship rotation rules.  "
		"You may find it necessary to disable the ship AI (e.g. by issuing a play-dead order) before running this sexp.\r\n\r\n"
		"Takes 6 arguments:\r\n"
		"\t1: The name of a ship\r\n"
		"\t2: Duration of the maneuver, in milliseconds\r\n"
		"\t3: Heading movement velocity, as a percentage (-100 to 100) of the tabled maximum heading velocity, or 0 to not modify the ship's current value\r\n"
		"\t4: Pitch movement velocity, as a percentage (-100 to 100) of the tabled maximum pitch velocity, or 0 to not modify the ship's current value\r\n"
		"\t5: Bank movement velocity, as a percentage (-100 to 100) of the tabled maximum bank velocity, or 0 to not modify the ship's current value\r\n"
		"\t6: Whether to apply all of the above velocity values even if any of them are 0\r\n" },
	
	// Wanderer
	{ OP_SHIP_LAT_MANEUVER, "ship-lat-maneuver\r\n"
		"\tCauses a ship to move in a lateral direction.  For the purposes of this sexp, this means the ship translates along its own X, Y, or Z axis (or a combination of axes) without regard to normal ship movement rules.  "
		"You may find it necessary to disable the ship AI (e.g. by issuing a play-dead order) before running this sexp.\r\n\r\n"
		"Takes 6 arguments:\r\n"
		"\t1: The name of a ship\r\n"
		"\t2: Duration of the maneuver, in milliseconds\r\n"
		"\t3: Vertical movement velocity, as a percentage (-100 to 100) of the tabled maximum vertical velocity, or 0 to not modify the ship's current value\r\n"
		"\t4: Sideways movement velocity, as a percentage (-100 to 100) of the tabled maximum sideways velocity, or 0 to not modify the ship's current value\r\n"
		"\t5: Forward movement velocity, as a percentage (-100 to 100) of the tabled maximum forward velocity, or 0 to not modify the ship's current value\r\n"
		"\t6: Whether to apply all of the above velocity values even if any of them are 0\r\n" },

	// Goober5000
	{ OP_SHIP_TAG, "ship-tag\r\n"
		"\tTags a ship.  Takes 3 or 8 arguments...\r\n"
		"\t1: The name of a ship.\r\n"
		"\t2: The tag level (currently 1, 2, or 3).\r\n"
		"\t3: The tag time (in seconds).\r\n"
		"\t4: A SSM missile (optional - used only for TAG-C).\r\n"
		"\t5: The X origin of the SSM missile (optional - used only for TAG-C).\r\n"
		"\t6: The Y origin of the SSM missile (optional - used only for TAG-C).\r\n"
		"\t7: The Z origin of the SSM missile (optional - used only for TAG-C).\r\n" 
        "\t8: The team the SSM missile belongs to (optional - used only for TAG-C).\r\n" },

/*	made obsolete by Goober5000 - it only works in debug builds anyway
	{ OP_INT3, "Error (Debug directive)\r\n"
		"Causes the game to halt with an error." },
*/

	{ OP_AI_CHASE, "Ai-chase (Ship goal)\r\n"
		"\tCauses the specified ship to chase and attack the specified target.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of ship to chase.\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_DOCK, "Ai-dock (Ship goal)\r\n"
		"\tCauses one ship to dock with another ship.\r\n\r\n"
		"Takes 4 arguments...\r\n"
		"\t1:\tName of dockee ship (The ship that \"docker\" will dock with).\r\n"
		"\t2:\tDocker's docking point - Which dock point docker uses to dock.\r\n"
		"\t3:\tDockee's docking point - Which dock point on dockee docker will move to.\r\n"
		"\t4:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_UNDOCK, "Ai-undock (Ship goal)\r\n"
		"\tCauses the specified ship to undock from who it is currently docked with.\r\n\r\n"
		"Takes 1 or 2 arguments...\r\n"
		"\t1:\tGoal priority (number between 0 and 89).\r\n"
		"\t2 (optional):\tShip to undock from.  If none is specified, the code will pick the first docked ship." },

	{ OP_AI_WARP_OUT, "Ai-warp-out (Ship/Wing Goal)\r\n"
		"\tCauses the specified ship/wing to warp out of the mission.  Currently, the ship will "
		"warp out at its current location.  This behavior will change.  Currently, the first "
		"argument means nothing.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of waypoint path to follow to warp out (not used).\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_WAYPOINTS, "Ai-waypoints (Ship goal)\r\n"
		"\tCauses the specified ship to fly a waypoint path continuously.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of waypoint path to fly.\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_WAYPOINTS_ONCE, "Ai-waypoints once (Ship goal)\r\n"
		"\tCauses the specified ship to fly a waypoint path.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of waypoint path to fly.\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_DESTROY_SUBSYS, "Ai-destroy subsys (Ship goal)\r\n"
		"\tCauses the specified ship to attack and try and destroy the specified subsystem "
		"on the specified ship.\r\n\r\n"
		"Takes 3 arguments...\r\n"
		"\t1:\tName of ship subsystem is on.\r\n"
		"\t2:\tName of subsystem on the ship to attack and destroy.\r\n"
		"\t3:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_CHASE_WING, "Ai-chase wing (Ship goal)\r\n"
		"\tCauses the specified ship to chase and attack the specified target.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of wing to chase.\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_DISABLE_SHIP, "Ai-disable-ship (Ship/wing goal)\r\n"
		"\tThis AI goal causes a ship/wing to destroy all of the engine subsystems on "
		"the specified ship.  This goal is different than ai-destroy-subsystem since a ship "
		"may have multiple engine subsystems requiring the use of > 1 ai-destroy-subsystem "
		"goals.\r\n"
		"Please note that this goal will implicitly call \"protect-ship\" on the target "
		"to prevent overzealous AI ships from destroying it in the process of disabling it.  "
		"If the ship must be destroyed later on, be sure to call an \"unprotect-ship\" sexp.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of ship whose engine subsystems should be destroyed\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_DISARM_SHIP, "Ai-disarm-ship (Ship/wing goal)\r\n"
		"\tThis AI goal causes a ship/wing to destroy all of the turret subsystems on "
		"the specified ship.  This goal is different than ai-destroy-subsystem since a ship "
		"may have multiple turret subsystems requiring the use of > 1 ai-destroy-subsystem "
		"goals.\r\n"
		"Please note that this goal will implicitly call \"protect-ship\" on the target "
		"to prevent overzealous AI ships from destroying it in the process of disarming it.  "
		"If the ship must be destroyed later on, be sure to call an \"unprotect-ship\" sexp.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of ship whose turret subsystems should be destroyed\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_GUARD, "Ai-guard (Ship goal)\r\n"
		"\tCauses the specified ship to guard a ship from other ships not on the same team.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of ship to guard.\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_CHASE_ANY, "Ai-chase-any (Ship goal)\r\n"
		"\tCauses the specified ship to chase and attack any ship on the opposite team.\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_GUARD_WING, "Ai-guard wing (Ship goal)\r\n"
		"\tCauses the specified ship to guard a wing of ships from other ships not on the "
		"same team.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of wing to guard.\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_NOP, "Do-nothing (Action operator)\r\n"
		"\tDoes nothing.  This is used as the default for any required action arguments "
		"of an operator." },

	{ OP_KEY_PRESSED, "Key-pressed (Boolean training operator)\r\n"
		"\tBecomes true when the specified default key has been pressed.  Default key "
		"refers to the what the key normally is when not remapped.  FreeSpace will "
		"automatically account for any keys that have been remapped.  If the optional "
		"delay is specified, becomes true that many seconds after the key has been pressed.\r\n\r\n"
		"Returns a boolean value.  Takes 1 or 2 arguments...\r\n"
		"\t1:\tDefault key to check for.\r\n"
		"\t2:\tDelay before operator registers as true (optional).\r\n" },

	{ OP_KEY_RESET, "Key-reset (Training operator)\r\n"
		"\tMarks the specified default key as having not been pressed, so key-pressed will be false "
		"until the player presses it again.  See key-pressed help for more information about "
		"what a default key is.\r\n\r\n"
		"\tNote that this sexp will not work properly in repeating events.  Use key-reset-multiple "
		"if this is to be called multiple times in one event.\r\n\r\n"
		"Returns a boolean value.  Takes 1 or more arguments...\r\n"
		"\tAll:\tDefault key to reset." },

	// Goober5000
	{ OP_KEY_RESET_MULTIPLE, "Key-reset-multiple (Training operator)\r\n"
		"\tMarks the specified default key as having not been pressed, so key-pressed will be false "
		"until the player presses it again.  See key-pressed help for more information about "
		"what a default key is.\r\n\r\n"
		"\tThis sexp, unlike key-reset, will work properly if called multiple times in one event.\r\n\r\n"
		"Returns a boolean value.  Takes 1 or more arguments...\r\n"
		"\tAll:\tDefault key to reset." },

	{ OP_TARGETED, "Targeted (Boolean training operator)\r\n"
		"\tIs true as long as the player has the specified ship (or ship's subsystem) targeted, "
		"or has been targeted for the specified amount of time.\r\n\r\n"
		"Returns a boolean value.  Takes 1 to 3 arguments (first required, rest optional):\r\n"
		"\t1:\tName of ship to check if targeted by player.\r\n"
		"\t2:\tLength of time target should have been kept for (optional).\r\n"
		"\t3:\tName of subsystem on ship to check if targeted (optional)." },

	{ OP_NODE_TARGETED, "Node-Targeted (Boolean training operator)\r\n"
		"\tIs true as long as the player has the specified jump node targeted, "
		"or has been targeted for the specified amount of time.\r\n\r\n"
		"Returns a boolean value.  Takes 1 to 2 arguments (first required, rest optional):\r\n"
		"\t1:\tName of Jump Node to check if targeted by player.\r\n"
		"\t2:\tLength of time target should have been kept for (optional)."},

	// Sesquipedalian
	{ OP_MISSILE_LOCKED, "Missile-locked (Boolean training operator)\r\n"
		"\tIs true as long as the player has had a missile lock for the specified amount of time. "
		"Optional arguments require lock to be maintained on a specified ship (or ship's subsystem).\r\n\r\n"
		"Returns a boolean value.  Takes 1 to 3 arguments (first required, rest optional):\r\n"
		"\t1:\tLength of time missile lock should have been kept for.\r\n"
		"\t2:\tName of ship to check if locked onto by player (optional).\r\n"
		"\t3:\tName of subsystem on ship to check if locked onto (optional)." },

	{ OP_SPEED, "Speed (Boolean training operator)\r\n"
		"\tBecomes true when the player has been within the specified speed range set by "
		"set-training-context-speed for the specified amount of time.\r\n\r\n"
		"Returns a boolean value.  Takes 1 argument...\r\n"
		"\t1:\tTime in seconds." },

	{ OP_GET_THROTTLE_SPEED, "Get-Throttle-Speed (Training operator)\r\n"
		"\tReturns the current throttle speed that the ship has been set to. Reverse speeds are returned as a negative value. "
		"Takes 1 argument...\r\n"
		"\t1:\tName of the player ship to check the throttle value for." },

	{ OP_FACING, "Facing (Boolean training operator)\r\n"
		"\tIs true as long as the specified ship is within the player's specified "
		"forward cone.  A forward cone is defined as any point that the angle between the "
		"vector of the point and the player, and the forward facing vector is within the "
		"given angle.\r\n\r\n"
		"Returns a boolean value.  Takes 2 argument...\r\n"
		"\t1:\tShip to check is withing forward cone.\r\n"
		"\t2:\tAngle in degrees of the forward cone." },

	{ OP_IS_FACING, "Is Facing (Boolean training operator)\r\n"
		"\tIs true as long as the second ship is within the first ship's specified "
		"forward cone.  A forward cone is defined as any point that the angle between the "
		"vector of the point and the player, and the forward facing vector is within the "
		"given angle. If the distance between the two ships is greather than "
		"the fourth parameter, this will return false.\r\n\r\n"
		"Returns a boolean value.  Takes 3 or 4 argument...\r\n"
		"\t1:\tShip to check from.\r\n"
		"\t2:\tShip to check is within forward cone.\r\n"
		"\t3:\tAngle in degrees of the forward cone.\r\n"
		"\t4:\tRange in meters (optional)."},

	{ OP_FACING2, "Facing Waypoint(Boolean training operator)\r\n"
		"\tIs true as long as the specified first waypoint is within the player's specified "
		"forward cone.  A forward cone is defined as any point that the angle between the "
		"vector of the point and the player, and the forward facing vector is within the "
		"given angle.\r\n\r\n"
		"Returns a boolean value.  Takes 2 argument...\r\n"
		"\t1:\tName of waypoint path whose first point is within forward cone.\r\n"
		"\t2:\tAngle in degrees of the forward cone." },

	// fixed by Goober5000 and then deprecated by Karajorma
	{ OP_ORDER, "Order (Boolean training operator)\r\n"
		"\tDeprecated - Use Query-Orders in any new mission.\r\n\r\n"
		"\tBecomes true when the player had given the specified ship or wing the specified order.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or 3 arguments...\r\n"
		"\t1:\tName of ship or wing to check if given order to.\r\n"
		"\t2:\tName of order to check if player has given.\r\n"
		"\t3:\tName of the target of the order (optional)." },

	{ OP_QUERY_ORDERS, "Query-Orders (Boolean training operator)\r\n"
		"\tBecomes true when the player had given the specified ship or wing the specified order.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more arguments...\r\n"
		"\t1:\tName of ship or wing to check if given order to.\r\n"
		"\t2:\tName of order to check if player has given.\r\n"
		"\t3:\tMaximum length of time since order was given. Use 0 for any time in the mission.\r\n"
		"\t4:\tName of the target of the order (optional).\r\n"
		"\t5:\tName of player ship giving the order(optional).\r\n"
		"\t6:\tName of the subsystem for Destroy Subsystem orders.(optional)" },

	// Karajorma
	{ OP_RESET_ORDERS, "Reset-Orders (Action training operator)\r\n"
		"\tResets the list of orders the player has given.\r\n"
		"Takes no arguments." },

	{ OP_WAYPOINT_MISSED, "Waypoint-missed (Boolean training operator)\r\n"
		"\tBecomes true when a waypoint is flown, but the waypoint is ahead of the one "
		"they are supposed to be flying.  The one they are supposed to be flying is the "
		"next one in sequence in the path after the last one they have hit.\r\n\r\n"
		"Returns a boolean value.  Takes no arguments." },

	{ OP_PATH_FLOWN, "Path-flown (Boolean training operator)\r\n"
		"\tBecomes true when all the waypoints in the path have been flown, in sequence.\r\n\r\n"
		"Returns a boolean value.  Takes no arguments." },

	{ OP_WAYPOINT_TWICE, "Waypoint-twice (Boolean training operator)\r\n"
		"\tBecomes true when a waypoint is hit that is before the last one hit, which "
		"indicates they have flown a waypoint twice.\r\n\r\n"
		"Returns a boolean value.  Takes no arguments." },

	{ OP_TRAINING_MSG, "Training-msg (Action training operator)\r\n"
		"\tSends the player a training message.  Uses the same messages as normal messages, "
		"only they get displayed differently using this operator.  If a secondary message "
		"is specified, it is sent the last time, while the primary message is sent all other "
		"times (event should have a repeat count greater than 1).\r\n\r\n"
		"Takes 1-3 arguments...\r\n"
		"\t1:\tName of primary message to send.\r\n"
		"\t2:\tName of secondary message to send (optional).\r\n"
		"\t3:\tDelay (in seconds) to wait before sending message. (optional)\r\n"
		"\t4:\tAmount of Time (in seconds) to display message (optional)." },

	{ OP_SET_TRAINING_CONTEXT_FLY_PATH, "Set-training-context-fly-path (Training context operator)\r\n"
		"\tTells FreeSpace that the player is expected to fly a waypoint path.  This must be "
		"executed before waypoint-missed, waypoint-twice and path-flown operators become valid.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of waypoint path player should fly.\r\n"
		"\t2:\tDistance away a player needs to be from a waypoint for it to be registered as flown." },

	{ OP_SET_TRAINING_CONTEXT_SPEED, "Set-training-context-speed (Training context operator)\r\n"
		"\tTells FreeSpace that the player is expected to fly within a certain speed range.  Once "
		"this operator has been executed, you can measure how long they have been within this "
		"speed range with the speed operator.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tMinimum speed of range player is to fly between.\r\n"
		"\t2:\tMaximum speed of range player is to fly between." },

	// Karajorma
	{ OP_STRING_TO_INT, "string-to-int\r\n"
		"\tConverts a string into an integer. The string must only contain numeric characters "
		"or zero is returned \r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tString to convert" },

	// Goober5000
	{ OP_STRING_GET_LENGTH, "string-get-length\r\n"
		"\tReturns the length of the specified string.  Takes 1 argument." },

	// Goober5000
	{ OP_INT_TO_STRING, "int-to-string\r\n"
		"\tConverts an integer into a string.  The destination must be a string variable.\r\n"
		"Takes 2 argument...\r\n"
		"\t1:\tInteger to convert\r\n"
		"\t2:\tString variable to contain the result\r\n" },

	// Goober5000
	{ OP_STRING_CONCATENATE, "string-concatenate\r\n"
		"\tConcatenates two strings, putting the result into a string variable.  If the length of the string will "
		"exceed the sexp variable token limit (currently 32), it will be truncated.\r\n\r\n"
		"Takes 3 arguments...\r\n"
		"\t1: First string\r\n"
		"\t2: Second string\r\n"
		"\t3: String variable to hold the result\r\n" },

	// Goober5000
	{ OP_STRING_GET_SUBSTRING, "string-get-substring\r\n"
		"\tExtracts a substring from a parent string, putting the result into a string variable.  If the length of the string will "
		"exceed the sexp variable token limit (currently 32), it will be truncated.\r\n\r\n"
		"Takes 3 arguments...\r\n"
		"\t1: Parent string\r\n"
		"\t2: Index at which the substring begins (0-based)\r\n"
		"\t3: Length of the substring\r\n"
		"\t4: String variable to hold the result\r\n" },

	// Goober5000
	{ OP_STRING_SET_SUBSTRING, "string-set-substring\r\n"
		"\tReplaces a substring from a parent string with a new string, putting the result into a string variable.  If the length of the string will "
		"exceed the sexp variable token limit (currently 32), it will be truncated.\r\n\r\n"
		"Takes 3 arguments...\r\n"
		"\t1: Parent string\r\n"
		"\t2: Index at which the substring begins (0-based)\r\n"
		"\t3: Length of the substring\r\n"
		"\t4: New substring (which can be a different length than the old substring)\r\n"
		"\t5: String variable to hold the result\r\n" },

	{ OP_GRANT_PROMOTION, "Grant promotion (Action operator)\r\n"
		"\tIn a single player game, this function grants a player an automatic promotion to the "
		"next rank which the player can obtain.  If he is already at the highest rank, this "
		"operator has no effect.  It takes no arguments." },

	{ OP_GRANT_MEDAL, "Grant medal (Action operator)\r\n"
		"\tIn single player missions, this function grants the given medal to the player.  "
		"Currently, only 1 medal will be allowed to be given per mission.\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tName of medal to grant to player." },

	{ OP_GOOD_SECONDARY_TIME, "Set prefered secondary weapons\r\n"
		"\tThis sexpression is used to inform the AI about prefered secondary weapons to "
		"fire during combat.  When this expression is evaulated, any AI ships of the given "
		"team prefer to fire the given weapon at the given ship. (Prefered over other "
		"secondary weapons)\r\n\r\n"
		"Takes 4 argument...\r\n"
		"\t1:\tTeam name which will prefer firing given weapon\r\n"
		"\t2:\tMaximum number of this type of weapon above team can fire.\r\n"
		"\t3:\tWeapon name (list includes only the valid weapons for this expression\r\n"
		"\t4:\tShip name at which the above named team should fire the above named weapon." },

	{ OP_AND_IN_SEQUENCE, "And in sequence (Boolean operator)\r\n"
		"\tReturns true if all of its arguments have become true in the order they are "
		"listed in.\r\n\r\n"
		"Returns a boolean value.  Takes 2 or more boolean arguments." },

	{ OP_SKILL_LEVEL_AT_LEAST, "Skill level at least (Boolean operator)\r\n"
		"\tReturns true if the player has selected the given skill level or higher.\r\n\r\n"
		"Returns a boolean value.  Takes 1 argument...\r\n"
		"\t1:\tName of the skill level to check." },

	{ OP_NUM_PLAYERS, "Num players (Status operator)\r\n"
		"\tReturns the current number of players (multiplayer) playing in the current mission.\r\n\r\n"
		"Returns a numeric value.  Takes no arguments." },

	{ OP_IS_CARGO_KNOWN, "Is cargo known (Boolean operator)\r\n"
		"\tReturns true if all of the specified objects' cargo is known by the player (i.e. they "
		"have scanned each one.\r\n\r\n"
		"Returns a boolean value.  Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ship to check if its cargo is known." },

	{ OP_HAS_BEEN_TAGGED_DELAY, "Has ship been tagged (delay) (Boolean operator)\r\n"
		"\tReturns true if all of the specified ships have been tagged.\r\n\r\n"
		"Returns a boolean value after <delay> seconds when all ships have been tagged.  Takes 2 or more arguments...\r\n"
		"\t1:\tDelay in seconds after which sexpression will return true when all cargo scanned."
		"\tRest:\tNames of ships to check if tagged.." },

	{ OP_CAP_SUBSYS_CARGO_KNOWN_DELAY, "Is capital ship subsystem cargo known (delay) (Boolean operator)\r\n"
		"\tReturns true if all of the specified subsystem cargo is known by the player.\r\n"
		"\tNote: Cargo must be explicitly named.\r\n\r\n"
		"Returns a boolean value after <delay> seconds when all cargo is known.  Takes 3 or more arguments...\r\n"
		"\t1:\tDelay in seconds after which sexpression will return true when all cargo scanned.\r\n"
		"\t2:\tName of capital ship\r\n"
		"\tRest:\tNames of subsystems to check for cargo known.." },

	{ OP_CARGO_KNOWN_DELAY, "Is cargo known (delay) (Boolean operator)\r\n"
		"\tReturns true if all of the specified objects' cargo is known by the player (i.e. they "
		"have scanned each one.\r\n\r\n"
		"Returns a boolean value after <delay> seconds when all cargo is known.  Takes 2 or more arguments...\r\n"
		"\t1:\tDelay in seconds after which sexpression will return true when all cargo scanned."
		"\tRest:\tNames of ships/cargo to check for cargo known.." },

	{ OP_WAS_PROMOTION_GRANTED, "Was promotion granted (Boolean operator)\r\n"
		"\tReturns true if a promotion was granted via the 'Grant promotion' operator in the mission.\r\n\r\n"
		"Returns a boolean value.  Takes no arguments." },

	{ OP_WAS_MEDAL_GRANTED, "Was medal granted (Boolean operator)\r\n"
		"\tReturns true if a medal was granted via via the 'Grant medal' operator in the mission.  "
		"If you provide the optional argument to this operator, then true is only returned if the "
		"specified medal was granted.\r\n\r\n"
		"Returns a boolean value.  Takes 0 or 1 arguments...\r\n"
		"\t1:\tName of medal to specifically check for (optional)." },

	{ OP_GOOD_REARM_TIME, "Good rearm time (Action operator)\r\n"
		"\tInforms the game logic that right now is a good time for a given team to attempt to "
		"rearm their ships.  The time parameter specified how long the \"good time\" will last.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tTeam Name\r\n"
		"\t2:\tTime in seconds rearm window should last" },

	{ OP_ALLOW_SHIP, "Allow ship (Action operator)\r\n"
		"\tThis operator makes the given ship type available to the Terran team.  Players will be "
		"able to have ships of this type in their starting wings in all future missions of this "
		"campaign.\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tName of ship type (or ship class) to allow." },

	{ OP_ALLOW_WEAPON, "Allow weapon (Action operator)\r\n"
		"\tThis operator makes the given weapon available to the Terran team.  Players will be "
		"able to equip ships with in all future missions of this campaign.\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tName of weapon (primary or secondary) to allow." },

	{ OP_TECH_ADD_SHIP, "Tech add ship (Action operator)\r\n"
		"\tThis operator makes the given ship type available in the techroom database.  Players will "
		"then be able to view this ship's specs there.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ship type (or ship class) to add." },

	{ OP_TECH_ADD_WEAPON, "Tech add weapon (Action operator)\r\n"
		"\tThis operator makes the given weapon available in the techroom database.  Players will "
		"then be able to view this weapon's specs there.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of weapon (primary or secondary) to add." },

	{ OP_TECH_ADD_INTEL, "Tech add intel (Action operator)\r\n"
		"\tThis operator makes the given intel entry available in the techroom database.  Players will "
		"then be able to view this intel entry there.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of intel entry to add." },

	{ OP_TECH_RESET_TO_DEFAULT, "Tech reset to default (Action operator)\r\n"
		"\tThis operator resets the tech room to the default represented in the tables.  This is "
		"useful for starting new campaigns, so that the player will not see tech entries carried over "
		"from previous campaigns.\r\n\r\n"
		"Takes no arguments." },

	{ OP_CHANGE_PLAYER_SCORE, "Change Player Score (Action operator)\r\n"
		"\tThis operator allows direct alteration of the player's score for this mission.\r\n\r\n"
		"Takes 2 or more arguments." 
		"\t1:\tAmount to alter the player's score by.\r\n"
		"\tRest:\tName of ship the player is flying."},

	{ OP_CHANGE_TEAM_SCORE, "Change Team Score (Action operator)\r\n"
		"\tThis operator allows direct alteration of the team's score for a TvT mission (Does nothing otherwise).\r\n\r\n"
		"Takes 2 arguments." 
		"\t1:\tAmount to alter the team's score by.\r\n"
		"\t2:\tThe team to alter the score for. (0 will add the score to all teams!)"},

	{ OP_AI_EVADE_SHIP, "Ai-evade ship (Ship goal)\r\n"
		"\tCauses the specified ship to go into evade mode and run away like the weak "
		"sally-boy it is.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of ship to evade from.\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_STAY_NEAR_SHIP, "Ai-stay near ship (Ship goal)\r\n"
		"\tCauses the specified ship to keep itself near the given ship and not stray too far "
		"away from it.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of ship to stay near.\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_KEEP_SAFE_DISTANCE, "Ai-keep safe distance (Ship goal)\r\n"
		"\tTells the specified ship to stay a safe distance away from any ship that isn't on the "
		"same team as it.\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_IGNORE, "Ai-ignore (Ship goal)\r\n"
		"\tTells all ships to ignore the given ship and not consider it as a valid "
		"target to attack.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of ship to ignore.\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	// Goober5000
	{ OP_AI_IGNORE_NEW, "Ai-ignore-new (Ship goal)\r\n"
		"\tTells the specified ship to ignore the given ship and not consider it as a valid "
		"target to attack.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of ship to ignore.\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_STAY_STILL, "Ai-stay still (Ship goal)\r\n"
		"\tCauses the specified ship to stay still.  The ship will do nothing until attacked at "
		"which time the ship will come to life and defend itself.\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tShip or waypoint the ship staying still will directly face (currently not implemented)\r\n"
		"\t2:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_PLAY_DEAD, "Ai-play dead (Ship goal)\r\n"
		"\tCauses the specified ship to pretend that it is dead and not do anything.  This "
		"expression should be used to indicate that a ship has no pilot and cannot respond "
		"to any enemy threats.  A ship playing dead will not respond to any attack.  This "
		"should really be named ai-is-dead\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tGoal priority (number between 0 and 89)." },

	{ OP_AI_FORM_ON_WING, "Ai-form-on-wing (Ship Goal)\r\n"
		"\tCauses the ship to form on the specified ship's wing. This works analogous to the "
		"player order, and will cause all other goals specified for the ship to be purged.\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tShip to form on." },

	{ OP_FLASH_HUD_GAUGE, "Ai-flash hud gauge (Training goal)\r\n"
		"\tCauses the specified hud gauge to flash to draw the player's attention to it.\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tName of hud gauge to flash." },

	{ OP_SHIP_VISIBLE, "ship-visible\r\n"
		"\tCauses the ships listed in this sexpression to be visible with player sensors.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\t1+:\tName of ships to make visible to sensors." },

	{ OP_SHIP_INVISIBLE, "ship-invisible\r\n"
		"\tCauses the ships listed in this sexpression to be invisible to player sensors.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\t1+:\tName of ships to make invisible to sensors." },

	{ OP_SHIP_VULNERABLE, "ship-vulnerable\r\n"
		"\tCauses the ship listed in this sexpression to be vulnerable to weapons.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\t1+:\tName of ships to make vulnerable to weapons." },

	{ OP_SHIP_INVULNERABLE, "ship-invulnerable\r\n"
		"\tCauses the ships listed in this sexpression to be invulnerable to weapons.  Use with caution!!!!\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\t1+:\tName of ships to make invulnerable to weapons." },

	{ OP_SHIP_BOMB_TARGETABLE, "ship-targetable-as-bomb\r\n"
		"\tCauses the ships listed in this sexpression to be targetable with bomb targetting key.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\t1+:\tName of ships to make targetable with bomb targeting key." },

	{ OP_SHIP_BOMB_UNTARGETABLE, "ship-untargetable-as-bomb\r\n"
		"\tCauses the ships listed in this sexpression to not be targetable with bomb targetting key.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\t1+:\tName of ships to make nontargetable with bomb targeting key." },

	{ OP_SHIELDS_ON, "shields-on\r\n" //-Sesquipedalian
		"\tCauses the ship listed in this sexpression to have their shields activated.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\t1+:\tName of ships to activate shields on." },

	{ OP_SHIELDS_OFF, "shields-off\r\n" //-Sesquipedalian
		"\tCauses the ships listed in this sexpression to have their shields deactivated.  \r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\t1+:\tName of ships to deactivate shields on." },

	{ OP_SHIP_GUARDIAN, "ship-guardian\r\n"
		"\tCauses the ships listed in this sexpression to not be killable by weapons.  Use with caution!!!!\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\t1+:\tName of ships to make unkillable." },

	{ OP_SHIP_NO_GUARDIAN, "ship-no-guardian\r\n"
		"\tCauses the ships listed in this sexpression to be killable by weapons, if not invulnerable.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\t1+:\tName of ships to make killable." },

	// Goober5000
 	{ OP_SHIP_GUARDIAN_THRESHOLD, "ship-guardian-threshold\r\n"
 		"\tSame as ship-guardian, except the lowest possible hull value is specified by the sexp rather than defaulting to 1.\r\n"
 		"Call with a threshold of 0 (or use ship-no-guardian) to deactivate.\r\n\r\n"
 		"Takes 2 or more arguments...\r\n"
 		"\t1:\tThreshold value.\r\n"
 		"\t2+:\tName of ships to make unkillable." },
 
 	// Goober5000
	{ OP_SHIP_SUBSYS_GUARDIAN_THRESHOLD, "ship-subsys-guardian-threshold\r\n"
		"\tSame as ship-guardian-threshold, but works on subsystems.\r\n"
		"Call with a threshold of 0 to deactivate.\r\n\r\n"
 		"Takes 3 or more arguments...\r\n"
		"\t1:\tThreshold value.\r\n"
		"\t2:\tShip housing the subsystem(s).\r\n"
		"\t3+:\tSubsystems to make unkillable." },

	// Goober5000
	{ OP_SHIP_STEALTHY, "ship-stealthy\r\n"
		"\tCauses the ships listed in this sexpression to become stealth ships (i.e. invisible to radar).\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ships to make stealthy." },

	// Goober5000
	{ OP_SHIP_UNSTEALTHY, "ship-unstealthy\r\n"
		"\tCauses the ships listed in this sexpression to become non-stealth ships (i.e. visible to radar).\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ships to make non-stealthy." },

	// Goober5000
	{ OP_FRIENDLY_STEALTH_INVISIBLE, "friendly-stealth-invisible\r\n"
		"\tCauses the friendly ships listed in this sexpression to be invisible to radar, just like hostile stealth ships."
		"It doesn't matter if the ship is friendly at the time this sexp executes: as long as it is a stealth ship, it will"
		"be invisible to radar both as hostile and as friendly.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ships" },

	// Goober5000
	{ OP_FRIENDLY_STEALTH_VISIBLE, "friendly-stealth-visible\r\n"
		"\tCauses the friendly ships listed in this sexpression to resume their normal behavior of being visible to radar as"
		"stealth friendlies.  Does not affect their visibility as stealth hostiles.\r\n\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tName of ships" },

	// Goober5000
	{ OP_SHIP_SUBSYS_TARGETABLE, "ship-subsys-targetable\r\n"
		"\tCauses the specified ship subsystem(s) to be targetable on radar.\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tName of a ship\r\n"
		"\tRest: Name of the ship's subsystem(s)" },

	// Goober5000
	{ OP_SHIP_SUBSYS_UNTARGETABLE, "ship-subsys-untargetable\r\n"
		"\tCauses the specified ship subsystem(s) to not be targetable on radar.\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tName of a ship\r\n"
		"\tRest: Name of the ship's subsystem(s)" },

	// FUBAR
	{ OP_SHIP_SUBSYS_NO_REPLACE, "ship-subsys-no-replace\r\n"
		"\tCauses the -destroyed version of specified ship subsystem(s) to not render when it's destroyed.\r\n"
		"Takes 3 or more arguments...\r\n"
		"\t1:\tName of a ship\r\n"
		"\t2:\tTrue = Do not render or False = render if exists\r\n"
		"\tRest: Name of the ship's subsystem(s)" 
		"\tNote: If subsystem is already dead it will vanish or reappear out of thin air" },


	// FUBAR
	{ OP_SHIP_SUBSYS_NO_LIVE_DEBRIS, "ship-subsys-no-live-debris\r\n"
		"\tCauses the specified ship subsystem(s) to not render live debris when it's destroyed.\r\n"
		"Takes 3 or more arguments...\r\n"
		"\t1:\tName of a ship\r\n"
		"\t2:\tTrue = Do not render or False = render if exists\r\n"
		"\tRest: Name of the ship's subsystem(s)" },

	// FUBAR
	{ OP_SHIP_SUBSYS_VANISHED, "ship-subsys-vanish\r\n"
		"\tCauses the subsystem to vanish without a trace - no fanfare, notification, or special effects.  See also ship-vanish.\r\n"
		"\tSingle Player Only!  Warning: This will cause subsystem destruction not to be logged, so 'has-departed', etc. will not work\r\n"
		"Takes 3 or more arguments...\r\n"
		"\t1:\tName of a ship\r\n"
		"\t2:\tTrue = vanish or False = don't vanish\r\n"
		"\tRest: Name of the ship's subsystem(s)" 
		"\tNote: Useful for replacing subsystems with actual docked models." },

	// FUBAR
	{ OP_SHIP_SUBSYS_IGNORE_IF_DEAD, "ship-subsys-ignore-if-dead\r\n"
		"\tCauses secondary weapons to ignore dead ship subsystem(s)and home on hull instead.\r\n"
		"Takes 3 or more arguments...\r\n"
		"\t1:\tName of a ship\r\n"
		"\t2:\tTrue = Ignore dead or False = don't ignore\r\n"
		"\tRest: Name of the ship's subsystem(s)" },

	// Goober5000
	{ OP_SHIP_CHANGE_ALT_NAME, "ship-change-alt-name\r\n"
		"\tChanges the alternate ship class name displayed in the HUD target window.  Takes 2 or more arguments...\r\n"
		"\t1:\tThe ship class name to display\r\n"
		"\tRest:\tThe ships to display the new class name" },

	// FUBAR
	{ OP_SHIP_CHANGE_CALLSIGN, "ship-change-callsign\r\n"
		"\tChanges the callsign of a ship.  Takes 2 or more arguments...\r\n"
		"\t1:\tThe callsign to display or empty to remove\r\n"
		"\tRest:\tThe ships to display the new callsign" },

	// Goober5000
	{ OP_SET_DEATH_MESSAGE, "set-death-message\r\n"
		"\tSets the message displayed when the specified players are killed.  Takes 1 or more arguments...\r\n"
		"\t1:\tThe message\r\n"
		"\tRest:\tThe players for whom this message is displayed (optional) (currently not implemented)" },

	{ OP_PERCENT_SHIPS_ARRIVED, "percent-ships-arrived\r\n"
		"\tBoolean function which returns true if the percentage of ships in the listed ships and wings "
		"which have arrived is greater or equal to the given percentage.  For wings, all ships of all waves "
		"are used for calculation for the total possible ships to arrive.\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tPercentge of arriving ships at which this function will return true.\r\n"
		"\t2+:\tList of ships/wings whose arrival status should be determined." },

	{ OP_PERCENT_SHIPS_DEPARTED, "percent-ships-departed\r\n"
		"\tBoolean function which returns true if the percentage of ships in the listed ships and wings "
		"which have departed is greater or equal to the given percentage.  For wings, all ships of all waves "
		"are used for calculation for the total possible ships to depart.\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tPercentge of departed ships at which this function will return true.\r\n"
		"\t2+:\tList of ships/wings whose departure status should be determined." },

	{ OP_PERCENT_SHIPS_DESTROYED, "percent-ships-destroyed\r\n"
		"\tBoolean function which returns true if the percentage of ships in the listed ships and wings "
		"which have been destroyed is greater or equal to the given percentage.  For wings, all ships of all waves "
		"are used for calculation for the total possible ships to be destroyed.\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tPercentge of destroyed ships at which this function will return true.\r\n"
		"\t2+:\tList of ships/wings whose destroyed status should be determined." },

	// Goober5000
	{ OP_PERCENT_SHIPS_DISARMED, "percent-ships-disarmed\r\n"
		"\tBoolean function which returns true if the percentage of ships in the listed ships "
		"which have been disarmed is greater or equal to the given percentage.\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tPercentge of disarmed ships at which this function will return true.\r\n"
		"\t2+:\tList of ships whose disarmed status should be determined." },

	// Goober5000
	{ OP_PERCENT_SHIPS_DISABLED, "percent-ships-disabled\r\n"
		"\tBoolean function which returns true if the percentage of ships in the listed ships "
		"which have been disabled is greater or equal to the given percentage.\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tPercentge of disabled ships at which this function will return true.\r\n"
		"\t2+:\tList of ships whose disabled status should be determined." },

	{ OP_RED_ALERT, "red-alert\r\n"
		"\tCauses Red Alert status in a mission.  This function ends the current mission, and moves to "
		"the next mission in the campaign under red alert status.  There should only be one branch from "
		"a mission that uses this expression\r\n\r\n"
		"Takes no arguments."},

	{ OP_MISSION_SET_NEBULA, "mission-set-nebula\r\n" 
		"\tTurns nebula on/off\r\n"
		"\tTakes1 argument...\r\n"
		"\t1:\t0 for nebula off, 1 for nebula on" },

	{ OP_MISSION_SET_SUBSPACE, "mission-set-subspace\r\n"
		"\tTurns subspace on/off\r\n"
		"\tTakes 1 argument...\r\n"
		"\r1:\t0 for subspace off, 1 for subspace on" },

	//-Sesquipedalian
	{ OP_END_MISSION, "end-mission\r\n" 
		"\tEnds the mission as if the player had engaged his subspace drive, but without him doing so.  Dumps the player back into a normal debriefing.  Does not invoke red-alert status.\r\n"
		"\t1:\tEnd Mission even if the player is dead (optional; defaults to true)\r\n"
		"\t2:\tBoot the player out into the main hall instead of going to the debriefing (optional; defaults to false; not supported in multi)"
	},

	// Goober5000
	{ OP_SET_DEBRIEFING_TOGGLED, "set-debriefing-toggled\r\n"
		"\tSets or clears the \"toggle debriefing\" mission flag.  If set, the mission will have its debriefing turned off, unless it is a multiplayer dogfight mission, in which case its debriefing will be turned on.  Takes 1 argument.\r\n"
	},

	// Goober5000
	{ OP_FORCE_JUMP, "force-jump\r\n"
		"\tForces activation of the player's subspace drive, thus ending the mission.  Takes no arguments."
	},

	// Goober5000
	{ OP_SET_SCANNED, "set-scanned\r\n"
		"\tSets the cargo on the specified ship or ship subsystem as known or scanned.  Takes 1 or more arguments...\r\n"
		"\t1:\tName of a ship\r\n"
		"\tRest:\tName of a subsystem on that ship (optional)\r\n" },

	// Goober5000
	{ OP_SET_UNSCANNED, "set-unscanned\r\n"
		"\tSets the cargo on the specified ship or ship subsystem as unknown or unscanned.  Takes 1 or more arguments...\r\n"
		"\t1:\tName of a ship\r\n"
		"\tRest:\tName of a subsystem on that ship (optional)\r\n" },

	{ OP_DEPART_NODE_DELAY, "depart-node-delay\r\n"
		"\tReturns true N seconds after the listed ships depart, if those ships depart within the "
		"radius of the given jump node.  The delay value is given in seconds.\r\n\r\n"
		"Takes 3 or more arguments...r\n"
		"\t1:\tDelay in seconds after the last ship listed departed before this expression can return true.\r\n"
		"\t2:\tName of a jump node\r\n"
		"\t3+:\tList of ships to check for departure within radius of the jump node." },

	{ OP_DESTROYED_DEPARTED_DELAY, "destroyed-or-departed-delay\r\n"
		"\tReturns true N seconds after all the listed ships or wings have been destroyed or have "
		"departed.\r\n\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tDelay in seconds after the last ship/wing is destroyed or departed before this expression can return true.\r\n"
		"\t2+:\tName of a ship or wing" },

	// description added by Goober5000
	{ OP_SPECIAL_CHECK, "Special-check\r\n"
		"\tMike K.'s special training sexp.  Returns a boolean value.  Takes 1 argument as follows:\r\n"
		"\t0:    Ship \"Freighter 1\" is aspect locked by the player\r\n"
		"\t1:    Player has fired Interceptor#Weak at Freighter 1\r\n"
		"\t2:    Ship \"Freighter 1\", subsystem \"Weapons\" is aspect locked by the player\r\n"
		"\t3:    Apply 10 points of damage to player's forward shields (action operator)\r\n"
		"\t4:    Player's front shields are nearly gone\r\n"
		"\t5:    Quickly recharge player's front shields (action operator)\r\n"
		"\t6:    Reduce all shield quadrants except front to 0 (action operator)\r\n"
		"\t7:    Front shield quadrant is near maximum strength\r\n"
		"\t8:    Rear shield quadrant is near maximum strength\r\n"
		"\t9:    Reduce left and right shield quadrants to 0 (action operator)\r\n"
		"\t10:   Player has fewer than 8 missiles left\r\n"
		"\t11:   Player has 8 or more missiles left\r\n"
		"\t12:   Player has fewer than 4 missiles left\r\n"
		"\t13:   Reduce front shield quadrant to 0 (action operator)\r\n"
		"\t100:  Player is out of countermeasures\r\n"
		"\t2000: Training failed"
	},

	{ OP_END_CAMPAIGN, "end-campaign\r\n"
		"\tEnds the builtin campaign.  Should only be used by the main FreeSpace campaign\r\n" },

	{ OP_SHIP_VAPORIZE, "ship-vaporize\r\n"
		"\tSets the ship to vaporize when it is destroyed.  Does not actually destroy the ship - use self-destruct for that.\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tList of ships on which to set the vaporize flag" },

	{ OP_SHIP_NO_VAPORIZE, "ship-no-vaporize\r\n"
		"\tSets the ship to not vaporize when it is destroyed.  Does not actually destroy the ship - use self-destruct for that.\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tList of ships on which to unset the vaporize flag" },

	{ OP_DONT_COLLIDE_INVISIBLE, "don't-collide-invisible\r\n"
		"\tSets the \"dont collide invisible\" flag on a list of ships.\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tList of ships on which to set the \"dont collide invisible\" flag" },

	{ OP_COLLIDE_INVISIBLE, "collide-invisible\r\n"
		"\tUnsets the \"dont collide invisible\" flag on a list of ships.\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tList of ships on which to unset the \"dont collide invisible\" flag" },

	{ OP_SET_MOBILE, "set-mobile\r\n"
		"\tAllows the specified ship(s) to move.  Opposite of set-immobile.\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tList of ships on which to unset the \"immobile\" flag" },

	{ OP_SET_IMMOBILE, "set-immobile\r\n"
		"\tPrevents the specified ship(s) from moving in any way.\r\n"
		"Takes 1 or more arguments...\r\n"
		"\tAll:\tList of ships on which to set the \"immobile\" flag" },

	{ OP_IGNORE_KEY, "ignore-key\r\n"
		"\tCauses the game to ignore (or stop ignoring) a certain key.\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1: Number of times to ignore this key (-1 = forever, 0 = stop ignoring). \r\n"
		"\tRest: Which key(s) to ignore.\r\n"
	},

	{ OP_WARP_BROKEN, "break-warp\r\n"
		"\tBreak the warp drive on the specified ship.  A broken warp drive can be repaired by "
		"a repair ship.  Takes 1 or more arguments...\r\n"
		"\tAll:\tList of ships to break the warp drive on" },

	{ OP_WARP_NOT_BROKEN, "fix-warp\r\n"
		"\tFixes a broken warp drive instantaneously.  This option applies to warp drives broken with "
		"the break-warp sepxression.  Takes 1 or more arguments...\r\n"
		"\tAll:\tList of ships whose warp drive should be fixed"},

	{ OP_WARP_NEVER, "never-warp\r\n"
		"\tNever allows a ship to warp out.  When this sexpression is used, the given ships will "
		"never be able to warp out.  The warp drive cannot be repaired.  Takes 1 or more arguments...\r\n"
		"\tAll:\tList of ships whose are not allowed to warp out under any condition"},

	{ OP_WARP_ALLOWED, "allow-warp\r\n"
		"\tAllows a ship which was previously not allowed to warp out to do so.  When this sexpression is "
		"used, the given ships will be able to warp out again.  Takes 1 or more arguments...\r\n"
		"\tAll:\tList of ships whose are allowed to warp out"},

	{ OP_SET_SUBSPACE_DRIVE, "set-subspace-drive\r\n"
		"\tTurns on or off the subspace edrive for the given ships.  A ship with no subspace drive will act "
		"as though it doesn't even occur to him to depart via subspace, and if ordered to do so, he will look "
		"for a friendly ship with a hangar bay.  If the ship is the player, pressing Alt-J will not not initiate "
		"a jump, nor give any indication that a jump failed.  Takes 2 or more arguments...\r\n"
		"\t1:\tTrue if the ship should have a drive; false otherwise\r\n"
		"\tRest:\tList of ships" },

	{ OP_JETTISON_CARGO, "jettison-cargo-delay\r\n"
		"\tCauses a cargo carrying ship to jettison its cargo without the undocking procedure. Takes 2 or more arguments...\r\n"
		"\t1: Ship to jettison cargo\r\n"
		"\t2: Delay after which to jettison cargo (note that this isn't actually used)\r\n"
		"\tRest (optional): Cargo to jettison.  If no optional arguments are specified, the ship jettisons all cargo.\r\n"
	},

	{ OP_SET_DOCKED, "set-docked\r\n"
		"\tCauses one ship to become instantly docked to another at the specified docking ports.Takes 4 arguments...\r\n"
		"\t1: Docker ship\r\n"
		"\t1: Docker point\r\n"
		"\t1: Dockee ship\r\n"
		"\t1: Dockee point\r\n"
	},

	{ OP_BEAM_FIRE, "fire-beam\r\n"
		"\tFire a beam weapon from a specified subsystem\r\n"
		"\t1:\tShip which will be firing\r\n"
		"\t2:\tTurret which will fire the beam (note, this turret must have at least 1 beam weapon on it)\r\n"
		"\t3:\tShip which will be targeted\r\n"
		"\t4:\tSubsystem to target (optional)\r\n"
		"\t5:\tWhether to force the beam to fire (disregarding FOV and subsystem status) (optional)\r\n" },

	{ OP_BEAM_FIRE_COORDS, "fire-beam-at-coordinates\r\n"
		"\tFire a beam weapon from a specified subsystem at a set of coordinates.  Not compatible with multiplayer.\r\n"
		"\t1:\tShip which will be firing\r\n"
		"\t2:\tTurret which will fire the beam (note, this turret must have at least 1 beam weapon on it)\r\n"
		"\t3:\tx coordinate to be targeted\r\n"
		"\t4:\ty coordinate to be targeted\r\n"
		"\t5:\tz coordinate to be targeted\r\n"
		"\t6:\tWhether to force the beam to fire (disregarding FOV and subsystem status) (optional)\r\n"
		"\t7:\tsecond x coordinate to be targeted (optional; only used for slash beams)\r\n"
		"\t8:\tsecond y coordinate to be targeted (optional; only used for slash beams)\r\n"
		"\t9:\tsecond z coordinate to be targeted (optional; only used for slash beams)\r\n" },

	{ OP_IS_TAGGED, "is-tagged\r\n"
		"\tReturns whether a given ship is tagged or not\r\n"},

	{ OP_IS_PLAYER, "is-player\r\n"
		"\tReturns true if all the ships specified are currently under control of a player.\r\n"
		"\t1 \t(SinglePlayer) When true ships under AI control return false even if they are the player ship\r\n"
		"\t \t(Multiplayer) When true ships that are respawning return true if the player is still connected\r\n"
		"\tRest \tList of ships to test"},

	{ OP_NUM_KILLS, "num-kills\r\n"
		"\tReturns the # of kills a player has. The ship specified in the first field should be the ship the player is in.\r\n"
		"\tSo, for single player, this would be Alpha 1. For multiplayer, it can be any ship with a player in it. If, at any\r\n"
		"\ttime there is no player in a given ship, this sexpression will return 0"},

	{ OP_NUM_ASSISTS, "num-assists\r\n"
		"\tReturns the # of assists a player has. The ship specified in the first field should be the ship the player is in.\r\n"
		"\tSo, for single player, this would be Alpha 1. For multiplayer, it can be any ship with a player in it. If, at any\r\n"
		"\ttime there is no player in a given ship, this sexpression will return 0"},

	{ OP_SHIP_SCORE, "ship-score\r\n"
		"\tReturns the score a player has. The ship specified in the first field should be the ship the player is in.\r\n"
		"\tSo, for single player, this would be Alpha 1. For multiplayer, it can be any ship with a player in it. If, at any\r\n"
		"\ttime there is no player in a given ship, this sexpression will return 0"},

	{ OP_SHIP_DEATHS, "ship-deaths\r\n"
		"\tReturns the # times a ship that is a player start has died.\r\n"
		"\tThe ship specified in the first field should be the ship that could have a player in.it\r\n"
		"\tOnly really useful for multiplayer."},

	{ OP_RESPAWNS_LEFT, "respawns-left\r\n"
		"\tReturns the # respawns a player (or AI that could have been a player) has remaining.\r\n"
		"\tThe ship specified in the first field should be the player start.\r\n"
		"\tOnly really useful for multiplayer."},

	{ OP_REMOVE_WEAPONS, "remove-weapons\r\n"
		"\tRemoves all live weapons currently in the game"
		"\t1: (Optional) Remove only this specific weapon\r\n"},

	{ OP_SET_RESPAWNS, "set-respawns\r\n"
		"\tSet the # respawns a player (or AI that could have been a player) has used.\r\n"
		"\t1: Number of respawns used up\r\n"
		"\tRest: The player start ship to operate on.\r\n"
		"\tOnly really useful for multiplayer."},

	{ OP_NUM_TYPE_KILLS, "num-type-kills\r\n"
		"\tReturns the # of kills a player has on a given ship type (fighter, bomber, cruiser, etc).\r\n"
		"The ship specified in the first field should be the ship the player is in.\r\n"
		"\tSo, for single player, this would be Alpha 1. For multiplayer, it can be any ship with a player in it. If, at any\r\n"
		"\ttime there is no player in a given ship, this sexpression will return 0"},

	{ OP_NUM_CLASS_KILLS, "num-class-kills\r\n"
		"\tReturns the # of kills a player has on a specific ship class (Ulysses, Hercules, etc).\r\n"
		"The ship specified in the first field should be the ship the player is in.\r\n"
		"\tSo, for single player, this would be Alpha 1. For multiplayer, it can be any ship with a player in it. If, at any\r\n"
		"\ttime there is no player in a given ship, this sexpression will return 0"},	

	{ OP_BEAM_FREE, "beam-free\r\n"
		"\tSets one or more beam weapons to allow firing for a given ship\r\n"
		"\t1: Ship to be operated on\r\n"
		"\t2, 3, etc : List of turrets to activate\r\n"},

	{ OP_BEAM_FREE_ALL, "beam-free-all\r\n"
		"\tSets all beam weapons on the specified ship to be active\r\n"},

	{ OP_BEAM_LOCK, "beam-lock\r\n"
		"\tSets one or more beam weapons to NOT allow firing for a given ship\r\n"
		"\t1: Ship to be operated on\r\n"
		"\t2, 3, etc : List of turrets to deactivate\r\n"},

	{ OP_BEAM_LOCK_ALL, "beam-lock-all\r\n"
		"\tSets all beam weapons on the specified ship to be deactivated\r\n"},

	{ OP_TURRET_FREE, "turret-free\r\n"
		"\tSets one or more turret weapons to allow firing for a given ship\r\n"
		"\t1: Ship to be operated on\r\n"
		"\t2, 3, etc : List of turrets to activate\r\n"},

	{ OP_TURRET_FREE_ALL, "turret-free-all\r\n"
		"\tSets all turret weapons on the specified ship to be active\r\n"},

	{ OP_TURRET_LOCK, "turret-lock\r\n"
		"\tSets one or more turret weapons to NOT allow firing for a given ship\r\n"
		"\t1: Ship to be operated on\r\n"
		"\t2, 3, etc : List of turrets to deactivate\r\n"},

	{ OP_TURRET_LOCK_ALL, "turret-lock-all\r\n"
		"\tSets all turret weapons on the specified ship to be deactivated\r\n"},

	{ OP_TURRET_CHANGE_WEAPON, "turret-change-weapon\r\n"
		"\tSets a given turret weapon slot to the specified weapon\r\n"
		"\t1: Ship turret is on\r\n"
		"\t2: Turret\r\n"
		"\t3: Weapon to set slot to\r\n"
		"\t4: Primary slot (or 0 to use secondary)\r\n"
		"\t5: Secondary slot (or 0 to use primary)"},

	{ OP_TURRET_SET_DIRECTION_PREFERENCE, "turret-set-direction-preference\r\n"
		"\tSets specified ship turrets direction preference to the specified value\r\n"
		"\t1: Ship turrets are on\r\n"
		"\t2: Preference to set, 0 to disable, or negative to reset to default\r\n"
		"\trest: Turrets to set\r\n"},

	{ OP_TURRET_SET_RATE_OF_FIRE, "turret-set-rate-of-fire\r\n"
		"\tSets specified ship turrets rate of fire to the specified value\r\n"
		"\t1: Ship turrets are on\r\n"
		"\t2: Rate to set in percentage format (200 = 2x, 50 = .5x), 0 to set to number of fire points, or negative to reset to default\r\n"
		"\trest: Turrets to set\r\n"},

	{ OP_TURRET_SET_OPTIMUM_RANGE, "turret-set-optimum-range\r\n"
		"\tSets specified ship turrets optimum range to the specified value\r\n"
		"\t1: Ship turrets are on\r\n"
		"\t2: Priority to set, 0 to disable, or negative to reset to default\r\n"
		"\trest: Turrets to set\r\n"},

	{ OP_TURRET_SET_TARGET_PRIORITIES, "turret-set-target-priorities\r\n"
		"\tSets target priorities for the specified ship turret\r\n"
		"\t1: Ship turret is on\r\n"
		"\t2: Turret to set\r\n"
		"\t3: True = Set new list, False = Reset to turret default\r\n"
		"\trest: Priorities to set (max 32) or blank for no priorities\r\n"},

	{ OP_SET_ARMOR_TYPE, "set-armor-type\r\n"
		"\tSets the armor type for a ship or subsystem\r\n"
		"\t1: Ship subsystem is on\r\n"
		"\t2: Set = true/Reset to defualt = false\r\n"
		"\t3: Armor type to set or <none>\r\n"
		"\trest: Subsystems to set (hull for ship, shield for shields)\r\n"},

	{ OP_WEAPON_SET_DAMAGE_TYPE, "weapon-set-damage-type\r\n"
		"\tSets the damage type for weapons or their shockwaves\r\n"
		"\t1: True = set weapon, False = set shockwave\r\n"
		"\t2: damage type to set or <none>\r\n"
		"\t3: Set = true/Reset to defualt = false\r\n"
		"\trest: Weapons to set\r\n"},

	{ OP_SHIP_SET_DAMAGE_TYPE, "ship-set-damage-type\r\n"
		"\tSets the damage type for ships collision or debris\r\n"
		"\t1: true = set collision, False = set debris\r\n"
		"\t2: Damage type to set or <none>\r\n"
		"\t3: Set = true/Reset to defualt = false\r\n"
		"\trest: Ships to set\r\n"},

	{ OP_SHIP_SHOCKWAVE_SET_DAMAGE_TYPE, "ship-set-shockwave-damage-type\r\n"
		"\tSets the shockwave damage type for a class of ship.  All ships of that class are changed.\r\n"
		"\t1: Damage type to set or <none>\r\n"
		"\t2: Set = true/Reset to defualt = false\r\n"
		"\trest: Ship classes to set\r\n"},

	{ OP_FIELD_SET_DAMAGE_TYPE, "field-set-damage-type\r\n"
		"\tSets the damage type for asteroid/debris fields\r\n"
		"\t1: Damage type to set or <none>\r\n"
		"\t2: Set = true/Reset to defualt = false\r\n"},

	{ OP_TURRET_SET_TARGET_ORDER, "turret-set-target-order\r\n"
		"\tSets targeting order of a given turret\r\n"
		"\t1: Ship turret is on\r\n"
		"\t2: Turret\r\n"
		"\trest: Target order type (Bombs,ships,asteroids)"},

	{ OP_SHIP_TURRET_TARGET_ORDER, "ship-turret-target-order\r\n"
		"\tSets targeting order of all turrets on a given ship\r\n"
		"\t1: Ship turrets are on\r\n"
		"\trest: Target order type (Bombs,ships,asteroids)"},

	{ OP_TURRET_SUBSYS_TARGET_DISABLE, "turret-subsys-target-disable\r\n"
		"\tPrevents turrets from targeting only the subsystems when targeting large targets\r\n"
		"\t1: Ship to be operated on\r\n"
		"\trest: List of turrets that are affected\r\n"},

	{ OP_TURRET_SUBSYS_TARGET_ENABLE, "turret-subsys-target-enable\r\n"
		"\tSets turret to target the subsystems when targeting large targets\r\n"
		"\t1: Ship to be operated on\r\n"
		"\trest: List of turrets that are affected\r\n"},

	{ OP_ADD_REMOVE_ESCORT, "add-remove-escort\r\n"
		"\tAdds or removes a ship from an escort list.\r\n"
		"\t1: Ship to be added or removed\r\n"
		"\t2: 0 to remove from the list, any positive value will be used as the escort priority\r\n"
		"NOTE : it _IS_ safe to add a ship which may already be on the list or remove\r\n"
		"a ship which is not on the list\r\n"},

	{ OP_AWACS_SET_RADIUS, "awacs-set-radius\r\n"
		"\tSets the awacs radius for a given ship subsystem. NOTE : does not work properly in multiplayer\r\n"
		"\t1: Ship which has the awacs subsystem\r\n"
		"\t2: Awacs subsystem\r\n"
		"\t3: New radius\r\n"},

	// Goober5000
	{ OP_PRIMITIVE_SENSORS_SET_RANGE, "primitive-sensors-set-range\r\n"
		"\tSets the range of the primitive sensors on a ship.  Ships outside of this range will not appear on "
		"sensors.  Has no effect on ships that do not have the \"primitive-sensors\" flag2 set.  Takes 2 arguments...\r\n"
		"\t1: Ship on which to set range\r\n"
		"\t2: Range, in meters\r\n" },

	{ OP_SEND_MESSAGE_LIST, "send-message-list\r\n"
		"\tSends a series of delayed messages. All times are accumulated.\r\n"
		"\t1:\tName of who the message is from.\r\n"
		"\t2:\tPriority of message (\"Low\", \"Normal\" or \"High\").\r\n"
		"\t3:\tName of message (from message editor).\r\n"
		"\t4:\tDelay from previous message in list (if any) in ms\r\n"
		"Use Add-Data for multiple messages.\r\n\r\n"
		"IMPORTANT: Each additional message in the list MUST HAVE four entries; "
		"any message without the four proper fields will be ignored, as will any "
		"successive messages."},

	{ OP_CAP_WAYPOINT_SPEED, "cap-waypoint-speed\r\n"
		"\tSets the maximum speed of a ship while flying waypoints.\r\n"
		"\t1: Ship name\r\n"
		"\t2: Maximum speed while flying waypoints\r\n"
		"\tNOTE: This will only work if the ship is already in the game\r\n"
		"\tNOTE: Set to -1 to reset\r\n"},

	{ OP_TURRET_TAGGED_ONLY_ALL, "turret-tagged-only\r\n"
		"\tMakes turrets target and hence fire strictly at tagged objects\r\n"
		"\t1: Ship name\r\n"
		"\tNOTE: Will not stop a turret already firing at an untagged ship\r\n"},

	{ OP_TURRET_TAGGED_CLEAR_ALL, "turret-tagged-clear\r\n"
		"\tRelaxes restriction on turrets targeting only tagged ships\r\n"
		"\t1: Ship name\r\n"},

	{ OP_PRIMARIES_DEPLETED, "primaries-depleted\r\n"
		"\tReturns true if ship is out of primary weapons\r\n"
		"\t1: Ship name\r\n"},

	{ OP_SECONDARIES_DEPLETED, "secondaries-depleted\r\n"
		"\tReturns true if ship is out of secondary weapons\r\n"
		"\t1: Ship name\r\n"},

	{ OP_SUBSYS_SET_RANDOM, "subsys-set-random\r\n"
		"\tSets ship subsystem strength in a given range\r\n"
		"\t1: Ship name\r\n"
		"\t2: Low range\r\n"
		"\t3: High range\r\n"
		"\t4: List of subsys names not to be randomized\r\n"},

	{ OP_SUPERNOVA_START, "supernova-start\r\n"
		"\t1: Time in seconds until the supernova shockwave hits the player\r\n"},

	{ OP_SUPERNOVA_STOP, "supernova-stop\r\n"
		"\t Stops a supernova in progress.\r\n"
		"\t Note this only works if the camera hasn't cut to the player's death animation yet.\r\n"},

	{ OP_WEAPON_RECHARGE_PCT, "weapon-recharge-pct\r\n"
		"\tReturns a percentage from 0 to 100\r\n"
		"\t1: Ship name\r\n" },

	{ OP_SHIELD_RECHARGE_PCT, "shield-recharge-pct\r\n"
		"\tReturns a percentage from 0 to 100\r\n"
		"\t1: Ship name\r\n" },

	{ OP_ENGINE_RECHARGE_PCT, "engine-recharge-pct\r\n"
		"\tReturns a percentage from 0 to 100\r\n"
		"\t1: Ship name\r\n" },

	{ OP_CARGO_NO_DEPLETE, "cargo-no-deplete\r\n"
		"\tCauses the named ship to have unlimited cargo.\r\n"
		"\tNote:  only applies to BIG or HUGE ships\r\n"
		"Takes 1 or more arguments...\r\n"
		"\t1:\tName of one of the ships.\r\n"
		"\t2:\toptional: 1 disallow depletion, 0 allow depletion." },

	{ OP_SHIELD_QUAD_LOW, "shield-quad-low\r\n"
		"\tReturns true if the specified ship has a shield quadrant below\r\n"
		"\tthe specified threshold percentage\r\n"
		"\t1: Ship name\r\n"
		"\t2: Percentage\r\n" },

	{ OP_PRIMARY_AMMO_PCT, "primary-ammo-pct\r\n"
		"\tReturns the percentage of ammo remaining in the specified ballistic primary bank (0 to 100).  Non-ballistic primary banks return as 100%.\r\n"
		"\t1: Ship name\r\n"
		"\t2: Bank to check (from 0 to N-1, where N is the number of primary banks in the ship; N or higher will return the cumulative average for all banks)" },

	// Karajorma
	{ OP_GET_PRIMARY_AMMO, "get-primary-ammo\r\n"
		"\tReturns the amount of ammo remaining in the specified bank\r\n"
		"\t1: Ship name\r\n"
		"\t2: Bank to check (from 0 to N-1, where N is the number of primary banks in the ship; N or higher will return the cumulative average for all banks)" },

	
	{ OP_SECONDARY_AMMO_PCT, "secondary-ammo-pct\r\n"
		"\tReturns the percentage of ammo remaining in the specified bank (0 to 100)\r\n"
		"\t1: Ship name\r\n"
		"\t2: Bank to check (from 0 to N-1, where N is the number of secondary banks in the ship; N or higher will return the cumulative average for all banks)" },

	// Karajorma
	{ OP_GET_SECONDARY_AMMO, "get-secondary-ammo\r\n"
		"\tReturns the amount of ammo remaining in the specified bank\r\n"
		"\t1: Ship name\r\n"
		"\t2: Bank to check (from 0 to N-1, where N is the number of secondary banks in the ship; N or higher will return the cumulative average for all banks)" },

	// Karajorma
	{ OP_GET_NUM_COUNTERMEASURES, "get-num-countermeasures\r\n"
		"\tReturns the amount of countermeasures remaining\r\n"
		"\t1: Ship name\r\n" },

	{ OP_IS_SECONDARY_SELECTED, "is-secondary-selected\r\n"
		"\tReturns true if the specified bank is selected (0 .. num_banks - 1)\r\n"
		"\t1: Ship name\r\n"
		"\t2: Bank to check (0 .. num_banks - 1)\r\n"},

	{ OP_IS_PRIMARY_SELECTED, "is-primary-selected\r\n"
		"\tReturns true if the specified bank is selected (0 .. num_banks - 1)\r\n"
		"\t1: Ship name\r\n"
		"\t2: Bank to check (0 .. num_banks - 1)\r\n"},

	{ OP_SPECIAL_WARP_DISTANCE, "special-warp-dist\r\n"
		"\tReturns distance to the plane of the knossos device in percent length of ship\r\n"
		"\t(ie, 100 means front of ship is 1 ship length from plane of knossos device)\r\n"
		"\t1: Ship name\r\n"}, 

	{ OP_SET_SPECIAL_WARPOUT_NAME, "special-warpout-name\r\n"
		"\tSets the name of the knossos device to be used for warpout\r\n"
		"\t1: Ship name to exit\r\n"
		"\t2: Name of knossos device\r\n"},

	{ OP_SHIP_VANISH, "ship-vanish\r\n"
		"\tMakes the named ship vanish (no log and vanish)\r\n"
		"\tSingle Player Only!  Warning: This will cause ship exit not to be logged, so 'has-departed', etc. will not work\r\n"
		"\t1: List of ship names to vanish\r\n"},

	{ OP_SHIP_CREATE, "ship-create\r\n"
		"\tCreates a new ship\r\n"
		"\tTakes 5 to 8 arguments...\r\n"
		"\t1: Name of new ship (use \"" SEXP_NONE_STRING "\" for a default name)\r\n"
		"\t2: Class of new ship\r\n"
		"\t3: X position\r\n"
		"\t4: Y position\r\n"
		"\t5: Z position\r\n"
		"\t6: Pitch (optional)\r\n"
		"\t7: Bank (optional)\r\n"
		"\t8: Heading (optional)\r\n"
	},

	// Goober5000
	{ OP_WEAPON_CREATE, "weapon-create\r\n"
		"\tCreates a new weapon\r\n"
		"\tTakes 5 to 10 arguments...\r\n"
		"\t 1: Name of parent ship (or \"" SEXP_NONE_STRING "\" for no parent)\r\n"
		"\t 2: Class of new weapon\r\n"
		"\t 3: X position\r\n"
		"\t 4: Y position\r\n"
		"\t 5: Z position\r\n"
		"\t 6: Pitch (optional)\r\n"
		"\t 7: Bank (optional)\r\n"
		"\t 8: Heading (optional)\r\n"
		"\t 9: Targeted ship (optional)\r\n"
		"\t10: Targeted subsystem (optional)\r\n"
	},

	{ OP_IS_SHIP_VISIBLE, "is-ship-visible\r\n"
		"\tCheck whether ship is visible on Player's radar\r\n"
		"\tSingle Player Only!  Returns 0 - not visible, 1 - partially visible, 2 - fully visible.\r\n"
		"\t1: Name of ship to check\r\n"},

	// Goober5000
	{ OP_IS_SHIP_STEALTHY, "is-ship-stealthy\r\n"
		"\tCheck whether ship is currently stealthy.\r\n"
		"\tTrue if stealth flag set, false otherwise.  Takes 1 argument...\r\n"
		"\t1: Name of ship to check\r\n"},

	// Goober5000
	{ OP_IS_FRIENDLY_STEALTH_VISIBLE, "is-friendly-stealth-visible\r\n"
		"\tCheck whether ship will be visible to radar as a stealth friendly.\r\n"
		"\tTakes 1 argument...\r\n"
		"\t1: Name of ship to check\r\n"},

	{ OP_TEAM_SCORE, "team-score\r\n"
		"\tGet the score of a multi team vs team game.\r\n"
		"\t1: Team index (1 for team 1 and 2 for team 2)\r\n"},

	// phreak
	{ OP_DAMAGED_ESCORT_LIST, "damaged-escort-list\r\n"
		"\tSets the most damaged ship in <ship list> to <priority1>, sets the others to <priority2>.  Don't use this sexp in the same mission as damaged-escort-list-all, or strange results might occur.\r\n"
		"\t1: Priority1\r\n"
		"\t2: Priority2\r\n"
		"\tRest: <ship_list>\r\n\r\n"
	},

	// Goober5000
	{ OP_DAMAGED_ESCORT_LIST_ALL, "damaged-escort-list-all\r\n"
		"\tSets the most damaged ship in the entire existing escort list (even what's not shown onscreen) to <priority1>, the next most damaged to <priority2>, and so on.  "
		"If there are more ships than priorities, the least most damaged ships are all set to the last priority in the list.  Don't use this sexp in the same mission as damaged-escort-list, or strange results might occur.\r\n"
		"\tTakes between 1 and MAX_COMPLETE_ESCORT_LIST (currently 20) arguments...\r\n"
		"\t1: Priority 1\r\n"
		"\tRest: Priorities 2 through 20 (optional)\r\n\r\n"
	},

	// Goober5000
	{ OP_CHANGE_SHIP_CLASS, "change-ship-class\r\n"
		"\tCauses the listed ships' classes to be changed to the specified ship class.  Takes 2 or more arguments...\r\n"
		"\t1: The name of the new ship class\r\n"
		"\tRest: The list of ships to change the classes of"
	},

	// Goober5000
	{ OP_SHIP_COPY_DAMAGE, "ship-copy-damage\r\n"
		"\tCopies the damage (hull, shields, and subsystems) from the first ship in the list to the rest.  The initial ship must be currently "
		"present in the mission, but the target ships may be either in-mission or on the arrival list.  Takes 2 or more arguments...\r\n"
		"\t1: The name of the ship that supplies the damage stats\r\n"
		"\tRest: The list of ships to be modified"
	},

	// Goober5000
	{ OP_SET_SUPPORT_SHIP, "set-support-ship\r\n"
		"\tSets information for all support ships in a mission.  Takes 6 or 7 arguments...\r\n"
		"\t1: Arrival location\r\n"
		"\t2: Arrival anchor\r\n"
		"\t3: Departure location\r\n"
		"\t4: Departure anchor\r\n"
		"\t5: Ship class\r\n"
		"\t6: Maximum number of support ships consecutively in this mission (use a negative number for infinity)\r\n"
		"\t7: Maximum number of support ships concurrently in this mission (optional, default 1)\r\n"
		"\r\n"
		"Note: The support ship will emerge from or depart into hyperspace if the location is set as hyperspace *or* the anchor is set as <no anchor>."
	},

	// Goober5000
	{ OP_SET_ARRIVAL_INFO, "set-arrival-info\r\n"
		"\tSets arrival information for a ship or wing.  Takes 2 to 6 arguments...\r\n"
		"\t1: Ship or wing name\r\n"
		"\t2: Arrival location\r\n"
		"\t3: Arrival anchor (optional; only required for certain locations)\r\n"
		"\t4: Arrival path mask (optional; defaults to 0; note that this is a bitfield)\r\n"
		"\t5: Arrival distance (optional; defaults to 0)\r\n"
		"\t6: Whether to show a jump effect if arriving from subspace (optional; defaults to true)\r\n"
	},

	// Goober5000
	{ OP_SET_DEPARTURE_INFO, "set-departure-info\r\n"
		"\tSets departure information for a ship or wing.  Takes 2 to 5 arguments...\r\n"
		"\t1: Ship or wing name\r\n"
		"\t2: Departure location\r\n"
		"\t3: Departure anchor (optional; only required for certain locations)\r\n"
		"\t4: Departure path mask (optional; defaults to 0; note that this is a bitfield)\r\n"
		"\t5: Whether to show a jump effect if departing to subspace (optional; defaults to true)\r\n"
	},

	// Bobboau
	{ OP_DEACTIVATE_GLOW_POINTS, "deactivate-glow-points\r\n"
		"\tDeactivates all glow points on a ship.  Takes 1 or more arguments...\r\n"
		"\tAll: Name of ship on which to deactivate glow points\r\n"
	},

	// Bobboau
	{ OP_ACTIVATE_GLOW_POINTS, "activate-glow-points\r\n"
		"\tActivates all glow points on a ship.  Takes 1 or more arguments...\r\n"
		"\tAll: Name of ship on which to activate glow points\r\n"
	},

	// Bobboau
	{ OP_DEACTIVATE_GLOW_MAPS, "deactivate-glow-maps\r\n"
		"\tDeactivates the glow maps for a ship.  Takes 1 or more arguments...\r\n"
		"\tAll: Name of ship on which to deactivate glow maps\r\n"
	},

	// Bobboau
	{ OP_ACTIVATE_GLOW_MAPS, "activate-glow-maps\r\n"
		"\tActivates the glow maps for a ship.  Takes 1 or more arguments...\r\n"
		"\tAll: Name of ship on which to activate glow maps\r\n"
	},

	// Bobboau
	{ OP_DEACTIVATE_GLOW_POINT_BANK, "deactivate-glow-point-bank\r\n"
		"\tDeactivates one or more glow point bank(s) on a ship.  Takes 2 or more arguments...\r\n"
		"\t1: Name of ship on which to deactivate glow point bank(s)\r\n"
		"\tRest: Name of glow point bank to deactivate\r\n"
	},

	// Bobboau
	{ OP_ACTIVATE_GLOW_POINT_BANK, "activate-glow-point-bank\r\n"
		"\tActivates one or more glow point bank(s) on a ship.  Takes 2 or more arguments...\r\n"
		"\t1: Name of ship on which to activate glow point bank(s)\r\n"
		"\tRest: Name of glow point bank to activate\r\n"
	},

	//-Sesquipedalian
	{ OP_KAMIKAZE, "kamikaze\r\n"
		"\tTells ships to perform a kamikaze on its current target. Takes 2 or more arguments...\r\n"
		"\t1: Damage dealt when kamikaze is done\r\n"
		"\tRest: Names of ships to perform kamikaze\r\n"
	},
	
	//phreak
	{ OP_TURRET_TAGGED_SPECIFIC, "turret-tagged-specific\r\n"
		"\tSpecific turrets on a ship only fire at tagged targets, as opposed to all turrets doing this using turret-tagged-only\r\n"
		"\tIt is safe to slave turrets already slaved\r\n"
		"\tTakes 2 or more arguments...\r\n"
		"\t1: Name of ship to slave some turrets to target only tagged ships\r\n"
		"\tRest: Turrets to slave\r\n"
	},

	//phreak
	{ OP_TURRET_TAGGED_CLEAR_SPECIFIC, "turret-tagged-clear-specific\r\n"
		"\tSpecific turrets on a ship are free to fire on untagged ships, as opposed to all turrets doing this using turret-tagged-clear\r\n"
		"\tIt is safe to unslave turrets already free\r\n"
		"\tTakes 2 or more arguments...\r\n"
		"\t1: Name of ship to unslave some turrets to target any hostile ship\r\n"
		"\tRest: Turrets to unslave\r\n"
	},

	// Goober5000
	{ OP_LOCK_ROTATING_SUBSYSTEM, "lock-rotating-subsystem\r\n"
		"\tInstantaneously locks a rotating subsystem so that it cannot rotate unless freed by free-rotating-subsystem.  "
		"Takes 2 or more arguments...\r\n"
		"\t1:\tName of the ship housing the subsystem\r\n"
		"\tRest:\tName of the rotating subsystem to lock"
	},

	// Goober5000
	{ OP_FREE_ROTATING_SUBSYSTEM, "free-rotating-subsystem\r\n"
		"\tInstantaneously frees a rotating subsystem previously locked by lock-rotating-subsystem.  "
		"Takes 2 or more arguments...\r\n"
		"\t1:\tName of the ship housing the subsystem\r\n"
		"\tRest:\tName of the rotating subsystem to free"
	},

	// Goober5000
	{ OP_REVERSE_ROTATING_SUBSYSTEM, "reverse-rotating-subsystem\r\n"
		"\tInstantaneously reverses the rotation direction of a rotating subsystem.  "
		"Takes 2 or more arguments...\r\n"
		"\t1:\tName of the ship housing the subsystem\r\n"
		"\tRest:\tName of the rotating subsystem to reverse"
	},

	// Goober5000
	{ OP_ROTATING_SUBSYS_SET_TURN_TIME, "rotating-subsys-set-turn-time\r\n"
		"\tSets the turn time of a rotating subsystem.  "
		"Takes 3 or 4 arguments...\r\n"
		"\t1:\tName of the ship housing the subsystem\r\n"
		"\t2:\tName of the rotating subsystem to configure\r\n"
		"\t3:\tThe time for one complete rotation, in milliseconds (positive is counterclockwise, negative is clockwise)\r\n"
		"\t4:\tThe acceleration (x1000, just as #3 is seconds x1000) to change from the current turn rate to the desired turn rate.  "
		"This is actually the time to complete one rotation that changes in one second, or the reciprocal of what you might expect, "
		"meaning that larger numbers cause slower acceleration.  (FS2 defaults to 2pi/0.5, or about 12.566, which would be 12566 in this sexp.)  "
		"The advantage of this method is so that this argument can be directly compared to the previous argument using a ratio, without worrying about pi.  "
		"Omit this argument if you want an instantaneous change."
	},

	// Goober5000
	{ OP_TRIGGER_SUBMODEL_ANIMATION, "trigger-submodel-animation\r\n"
		"\tActivates a submodel animation trigger for a given ship.  Takes 4 to 6 arguments...\r\n"
		"\t1: The ship on which the animation should run\r\n"
		"\t2: The type of animation (named as one would see them in ships.tbl)\r\n"
		"\t3: The subtype of animation, which is type-dependent.  For docking animations this is the dock index.\r\n"
		"\t4: The animation direction: 1 for forward, or -1 for reverse\r\n"
		"\t5: (Optional) Whether the animation should instantly snap to its final position\r\n"
		"\t6: (Optional) A subsystem, if the animation should trigger on only a specific subsystem as opposed to all applicable subsystems\r\n"
	},

	// Karajorma
	{ OP_SET_PRIMARY_AMMO, "set-primary-ammo\r\n"
		"\tSets the amount of ammo for the specified ballistic bank\r\n"
		"\t1: Ship name\r\n"
		"\t2: Bank to check (0, 1, and 2 are legal banks)\r\n" 
		"\t3: Number to set this bank to (If this is larger than the maximimum, bank will be set to maximum).\r\n"
		"\t4: Rearm Limit. Support ships will only supply this number of weapons (If this is larger than the maximimum, bank will be set to maximum)"
	},

	// Karajorma
	{ OP_SET_SECONDARY_AMMO, "set-secondary-ammo\r\n"
		"\tSets the amount of ammo for the specified bank\r\n"
		"\t1: Ship name\r\n"
		"\t2: Bank to check (0, 1, 2 and 3 are legal banks)\r\n" 
		"\t3: Number to set this bank to (If this is larger than the maximimum, bank will be set to maximum).\r\n"
		"\t4: Rearm Limit. Support ships will only supply this number of weapons (If this is larger than the maximimum, bank will be set to maximum)"
	},
	
	// Karajorma
	{ OP_SET_PRIMARY_WEAPON, "set-primary-weapon\r\n"
		"\tSets the weapon for the specified bank\r\n"
		"\t1: Ship name\r\n"
		"\t2: Bank to check (0, 1 and 2 are legal banks)\r\n" 
		"\t3: Name of the primary weapon \r\n"
		"\t4: Number to set this bank to (If this is larger than the maximimum, bank will be set to maximum)\r\n"
		"\t5: Rearm Limit. Support ships will only supply this number of weapons (If this is larger than the maximimum, bank will be set to maximum)"
	},
	
	// Karajorma
	{ OP_SET_SECONDARY_WEAPON, "set-secondary-weapon\r\n"
		"\tSets the weapon for the specified bank\r\n"
		"\t1: Ship name\r\n"
		"\t2: Bank to check (0, 1, 2 and 3 are legal banks)\r\n" 
		"\t3: Name of the secondary weapon \r\n"
		"\t4: Number to set this bank to (If this is larger than the maximimum, bank will be set to maximum)\r\n"
		"\t5: Rearm Limit. Support ships will only supply this number of weapons (If this is larger than the maximimum, bank will be set to maximum)"
	},


	
	// Karajorma
	{ OP_SET_NUM_COUNTERMEASURES, "set-num-countermeasures\r\n"
		"\tSets the number of countermeasures the ship has\r\n"
		"\tValues greater than the maximum a ship can carry are set to the maximum\r\n"
		"\t1: Ship name\r\n"
		"\t2: Number to set"
	},
	
	// Karajorma
	{ OP_LOCK_PRIMARY_WEAPON, "lock-primary-weapon\r\n"
		"\tLocks the primary banks for the specified ship(s)\r\n"
		"\tTakes 1 or more arguments\r\n"
		"\t(all): Name(s) of ship(s) to lock"
	},

	// Karajorma
	{ OP_UNLOCK_PRIMARY_WEAPON, "unlock-primary-weapon\r\n"
		"\tUnlocks the primary banks for the specified ship(s)\r\n"
		"\tTakes 1 or more arguments\r\n"
		"\t(all): Name(s) of ship(s) to lock"
	},
	
	// Karajorma
	{ OP_LOCK_SECONDARY_WEAPON, "lock-secondary-weapon\r\n"
		"\tLocks the secondary banks for the specified ship(s)\r\n"
		"\tTakes 1 or more arguments\r\n"
		"\t(all): Name(s) of ship(s) to lock"
	},

	// Karajorma
	{ OP_UNLOCK_SECONDARY_WEAPON, "unlock-secondary-weapon\r\n"
		"\tUnlocks the secondary banks for the specified ship(s)\r\n"
		"\tTakes 1 or more arguments\r\n"
		"\t(all): Name(s) of ship(s) to lock"
	},
	
	// KeldorKatarn
	{ OP_LOCK_AFTERBURNER, "lock-afterburner\r\n"
		"\tLocks the afterburners on the specified ship(s)\r\n"
		"\tTakes 1 or more arguments\r\n"
		"\t(all): Name(s) of ship(s) to lock"
	},

	// KeldorKatarn
	{ OP_UNLOCK_AFTERBURNER, "unlock-afterburner\r\n"
		"\tUnlocks the afterburners on the specified ship(s)\r\n"
		"\tTakes 1 or more arguments\r\n"
		"\t(all): Name(s) of ship(s) to lock"
	},

	// Karajorma
	{ OP_SET_AFTERBURNER_ENERGY, "set-afterburner-energy\r\n"
		"\tSets the afterburner energy for the specified ship(s)\r\n"
		"\tTakes 2 or more arguments\r\n"
		"\t1: percentage of maximum afterburner energy.\r\n"
		"\t(rest): Name(s) of ship(s)"
	},

	{ OP_SET_WEAPON_ENERGY, "set-weapon-energy\r\n"
		"\tSets the weapon energy for the specified ship(s)\r\n"
		"\tTakes 2 or more arguments\r\n"
		"\t1: percentage of maximum weapon energy.\r\n"
		"\t(rest): Name(s) of ship(s)"
	},

	{ OP_SET_SHIELD_ENERGY, "set-shield-energy\r\n"
		"\tSets the shield energy for the specified ship(s)\r\n"
		"\tTakes 2 or more arguments\r\n"
		"\t1: percentage of maximum shield energy.\r\n"
		"\t(rest): Name(s) of ship(s)"
	},

	{ OP_SET_AMBIENT_LIGHT, "set-ambient-light\r\n"
		"\tSets the ambient light level for the mission\r\n"
		"\tTakes 3 arguments\r\n"
		"\t1: Red (0 - 255).\r\n"
		"\t2: Green (0 - 255).\r\n"
		"\t3: Blue (0 - 255)."
	},

	{ OP_SET_POST_EFFECT, "set-post-effect\r\n"
		"\tConfigures post-processing effect\r\n"
		"\tTakes 2 arguments\r\n"
		"\t1: Effect type\r\n"
		"\t2: Effect intensity (0 - 100)."
	},

	{ OP_CHANGE_SUBSYSTEM_NAME, "change-subsystem-name\r\n"
		"\tChanges the name of the specified subsystem on the specified ship\r\n"
		"\tTakes 3 or more arguments\r\n"
		"\t1: Name of the ship.\r\n"
		"\t2: New name for the subsystem (names larger than the maximum display size will be truncated\r\n"
		"\t3: Name(s) of subsystem(s) to rename\r\n"
	},

	//phreak
	{ OP_NUM_SHIPS_IN_BATTLE, "num-ships-in-battle\r\n"
		"\tReturns the number of ships in battle or the number of ships in battle out of a list of teams, wings, and ships.  Takes 1 or more arguments...\r\n"
		"\t(all):\tTeams, Wings, and Ships to query (optional)"
	},

	// Karajorma
	{ OP_NUM_SHIPS_IN_WING, "num-ships-in-wing\r\n"
		"\tReturns the number of ships in battle which belong to a given wing.  Takes 1 or more arguments...\r\n"
		"\t(all):\tName of wing(s) to check"
	},

	// Goober5000
	{ OP_HUD_DISABLE, "hud-disable\r\n"
		"\tSets whether the hud is disabled.  Takes 1 argument...\r\n"
		"\t1: Flag (1 to disable, 0 to re-enable)"
	},
	
	// Goober5000
	{ OP_HUD_DISABLE_EXCEPT_MESSAGES, "hud-disable-except-messages\r\n"
		"\tSets whether the hud (except for messages) is disabled.  Takes 1 argument...\r\n"
		"\t1: Flag (1 to disable, 0 to re-enable)"
	},

	// Goober5000
	{ OP_HUD_SET_MAX_TARGETING_RANGE, "hud-set-max-targeting-range\r\n"
		"\tSets the farthest distance at which an object can be targeted.  Takes 1 argument...\r\n"
		"\1: Maximum targeting distance (0 for infinite)\r\n"
	},

	{ OP_HUD_DISPLAY_GAUGE, "hud-display-gauge <milliseconds> <gauge>\r\n"
		"\tCauses specified hud gauge to appear or disappear for so many milliseconds.  Takes 1 argument...\r\n"
		"\t1: Number of milliseconds that the warpout gauge should appear on the HUD."
		" 0 will immediately cause the gauge to disappear.\r\n"
		"\t2: Name of HUD element.  Must be one of:\r\n"
		"\t\t" SEXP_HUD_GAUGE_WARPOUT " - the \"Subspace drive active\" box that appears above the viewscreen.\r\n"
	},

	// Goober5000
	{ OP_PLAYER_USE_AI, "player-use-ai\r\n"
		"\tCauses the player's ship to be controlled by the FreeSpace AI.  Takes 0 arguments.\r\n"
	},

	// Goober5000
	{ OP_PLAYER_NOT_USE_AI, "player-not-use-ai\r\n"
		"\tCauses the player's ship to not be controlled by the FreeSpace AI.  Takes 0 arguments.\r\n"
	},

	// Karajorma
	{ OP_ALLOW_TREASON, "allow-treason\r\n"
		"\tTurns the Allow Traitor switch on or off in mission. Takes 0 arguments.\r\n"
		"\t1:\tTrue/False."
	},

	// Karajorma
	{ OP_SET_PLAYER_ORDERS, "set-player-orders\r\n"
		"\tChanges the orders friendly AI will accept. Takes 3 or more arguments.\r\n"
		"\t1:\tShip Name\r\n"
		"\t2:\tTrue/False as to whether this order is allowed or not\r\n"
		"\tRest:\tOrder"
	},

	//WMC
	{ OP_HUD_SET_TEXT, "hud-set-text\r\n"
		"\tSets the text value of a given HUD gauge. Works for custom and certain retail gauges. Takes 2 arguments...\r\n"
		"\t1:\tHUD gauge to be modified\r\n"
		"\t2:\tText to be set"
	},

	{ OP_HUD_SET_TEXT_NUM, "hud-set-text-num\r\n"
		"\tSets the text value of a given HUD gauge to a number. Works for custom and certain retail gauges. Takes 2 arguments...\r\n"
		"\t1:\tHUD gauge to be modified\r\n"
		"\t2:\tNumber to be set"
	},

	{ OP_HUD_SET_MESSAGE, "hud-set-message\r\n"
		"\tSets the text value of a given HUD gauge to a message from the mission's message list. Works for custom and certain retail gauges. Takes 2 arguments...\r\n"
		"\t1:\tHUD gauge to be modified\r\n"
		"\t2:\tMessage"
	},

	//WMC
	{ OP_HUD_SET_COORDS, "hud-set-coord\r\n"
		"\tSets the coordinates of a given HUD gauge. Works for custom and retail gauges. Takes 3 arguments...\r\n"
		"\t1:\tHUD gauge to be modified\r\n"
		"\t2:\tCoordinate X component\r\n"
		"\t2:\tCoordinate Y component"
	},

	//WMC
	{ OP_HUD_SET_FRAME, "hud-set-frame\r\n"
		"\tSets the frame of a given HUD gauge's associated image. Works for custom and certain retail gauges. Takes 2 arguments...\r\n"
		"\t1:\tHUD gauge to be modified\r\n"
		"\t2:\tFrame number to be changed to"
	},

	//WMC
	{ OP_HUD_SET_COLOR, "hud-set-color\r\n"
		"\tSets the color of a given HUD gauge. Works only for custom gauges Takes 4 arguments...\r\n"
		"\t1:\tHUD gauge to be modified\r\n"
		"\t2:\tRed component (0-255)\r\n"
		"\t3:\tGreen component (0-255)\r\n"
		"\t4:\tBlue component (0-255)"
	},

	//WMC
	{ OP_CURRENT_SPEED, "current-speed\r\n"
		"\tReturns the speed of the given object. Takes 1 argument...\r\n"
		"\t1:\tName of the object"
	},

	// Karajora
	{ OP_PRIMARY_FIRED_SINCE, "primary-fired-since\r\n"
		"\tReturns true if the primary weapon bank specified has been fired within the supplied window of time. Takes 3 arguments...\r\n\r\n"
		"\t1:\tShip name\r\n"
		"\t2:\tWeapon bank number\r\n"
		"\t3:\tTime period to check if the weapon was fired (in millieconds)\r\n"
	},

	// Karajora
	{ OP_SECONDARY_FIRED_SINCE, "secondary-fired-since\r\n"
		"\tReturns true if the secondary weapon bank specified has been fired within the supplied window of time. Takes 3 arguments...\r\n\r\n"
		"\t1:\tShip name\r\n"
		"\t2:\tWeapon bank number\r\n"
		"\t3:\tTime period to check if the weapon was fired (in millieconds)\r\n"
	},

	// Karajora
	{ OP_HAS_PRIMARY_WEAPON, "has-primary-weapon\r\n"
		"\tReturns true if the primary weapon bank specified has any of the weapons listed. Takes 3 or more arguments...\r\n\r\n"
		"\t1:\tShip name\r\n"
		"\t2:\tWeapon bank number\r\n"
		"\tRest:\tWeapon name\r\n"
	},

	// Karajora
	{ OP_HAS_SECONDARY_WEAPON, "has-secondary-weapon\r\n"
		"\tReturns true if the secondary weapon bank specified has any of the weapons listed. Takes 3 or more arguments...\r\n\r\n"
		"\t1:\tShip name\r\n"
		"\t2:\tWeapon bank number\r\n"
		"\tRest:\tWeapon name\r\n"
	},

	// Karajora
	{ OP_DIRECTIVE_VALUE, "directive-value\r\n"
		"\tCauses a value to appear in the directive count\r\n"
		"\tAlways returns true. Takes 1 or more arguments...\r\n\r\n"
		"\t1:\tValue\r\n"
		"\t2:\t(Optional) Ignore the directive count set by any earlier SEXPs in the event. If set to false it will add instead\r\n"
	},

	//phreak
	{ OP_SCRAMBLE_MESSAGES, "scramble-messages\r\n"
		"\tCauses messages to be sent as if the player has sustained communications subsystem or EMP damage.  Takes no arguments.\r\n"
		"\tThis effect can be reversed using unscramble-messages."
	},

	//phreak
	{ OP_UNSCRAMBLE_MESSAGES, "unscramble-messages\r\n"
		"\tUndoes the effects of scramble-messages, causing messages to be sent clearly.  Takes no arguments."
	},

	{ OP_CUTSCENES_SET_CUTSCENE_BARS, "set-cutscene-bars\r\n"
		"\tShows bars at the top and bottom of screen.  "
		"Takes 0 or 1 arguments...\r\n"
		"\t1:\tMilliseconds for bars to slide in\r\n"
	},

	{ OP_CUTSCENES_UNSET_CUTSCENE_BARS, "unset-cutscene-bars\r\n"
		"\tRemoves cutscene bars.  "
		"Takes 0 or 1 arguments...\r\n"
		"\t1:\tMilliseconds for bars to slide out\r\n"
	},

	{ OP_CUTSCENES_FADE_IN, "fade-in\r\n"
		"\tFades in.  "
		"Takes 0 or 1 arguments...\r\n"
		"\t1:\tTime to fade in (in milliseconds)\r\n"
	},

	{ OP_CUTSCENES_FADE_OUT, "fade-out\r\n"
		"\tFades out.  "
		"Takes 0 to 2 arguments...\r\n"
		"\t1:\tTime to fade in (in milliseconds)\r\n"
		"\t2:\tColor to fade to - 1 for white, 2 for red, default is black\r\n"
	},

	{ OP_CUTSCENES_SET_CAMERA, "set-camera\r\n"
		"\tSets SEXP camera, or another specified cutscene camera.  "
		"Takes 0 to 1 arguments...\r\n"
		"\t(optional)\r\n"
		"\t1:\tCamera name (created if nonexistent)\r\n"
	},

	{ OP_CUTSCENES_SET_CAMERA_FACING, "set-camera-facing\r\n"
		"\tMakes the camera face the given point.  "
		"Takes 3 to 6 arguments...\r\n"
		"\t1:\tX position to face\r\n"
		"\t2:\tY position to face\r\n"
		"\t3:\tZ position to face\r\n"
		"\t(optional)\r\n"
		"\t4:\tTotal turn time (milliseconds)\r\n"
		"\t5:\tTime to spend accelerating/decelerating (milliseconds)\r\n"
		"\t6:\tTime to spend decelerating (milliseconds)\r\n"
	},

	{ OP_CUTSCENES_SET_CAMERA_FACING_OBJECT, "set-camera-facing-object\r\n"
		"\tMakes the camera face the given object.  "
		"Takes 1 to 4 arguments...\r\n"
		"\t1:\tObject to face\r\n"
		"\t(optional)\r\n"
		"\t2:\tTotal turn time (milliseconds)\r\n"
		"\t3:\tTime to spend accelerating/decelerating (milliseconds)\r\n"
		"\t4:\tTime to spend decelerating (milliseconds)\r\n"
	},

	{ OP_CUTSCENES_SET_CAMERA_FOV, "set-camera-fov\r\n"
		"\tSets the camera field of view.  "
		"Takes 1 to 4 arguments...\r\n"
		"\t1:\tNew FOV (degrees)\r\n"
		"\t(optional)\r\n"
		"\t2:\tTotal zoom time (milliseconds)\r\n"
		"\t3:\tTime to spend accelerating/decelerating (milliseconds)\r\n"
		"\t4:\tTime to spend decelerating (milliseconds)\r\n"
	},
	
	{ OP_CUTSCENES_SET_CAMERA_HOST, "set-camera-host\r\n"
		"\tSets the object and subystem camera should view from. Camera position is offset from the host. "
		"Camera orientation is also offset from host, unless a valid camera target is set."
		"Takes 1 to 2 arguments...\r\n"
		"\t1:\tShip to mount camera on\r\n"
		"\t(optional)\r\n"
		"\t2:\tSubystem to mount camera on\r\n"
	},

	{ OP_CUTSCENES_SET_CAMERA_POSITION, "set-camera-position\r\n"
		"\tSets the camera position to a spot in mission space.  "
		"Takes 3 to 6 arguments...\r\n"
		"\t1:\tX position\r\n"
		"\t2:\tY position\r\n"
		"\t3:\tZ position\r\n"
		"\t(optional)\r\n"
		"\t4:\tTotal turn time (milliseconds)\r\n"
		"\t5:\tTime to spend accelerating/decelerating (milliseconds)\r\n"
		"\t6:\tTime to spend decelerating (milliseconds)\r\n"
	},

	{ OP_CUTSCENES_SET_CAMERA_ROTATION, "set-camera-rotation\r\n"
		"\tSets the camera rotation.  "
		"Takes 3 to 6 arguments...\r\n"
		"\t1:\tPitch (degrees)\r\n"
		"\t2:\tBank (degrees)\r\n"
		"\t3:\tHeading (degrees)\r\n"
		"\t(optional)\r\n"
		"\t4:\tTotal turn time (milliseconds)\r\n"
		"\t5:\tTime to spend accelerating/decelerating (milliseconds)\r\n"
		"\t6:\tTime to spend decelerating (milliseconds)\r\n"
	},

	{ OP_CUTSCENES_SET_CAMERA_TARGET, "set-camera-target\r\n"
		"\tSets the object and subystem camera should track. Camera orientation is offset from the target.  "
		"Takes 1 to 2 arguments...\r\n"
		"\t1:\tShip to track\r\n"
		"\t(optional)\r\n"
		"\t2:\tSubystem to track\r\n"
	},

	{ OP_CUTSCENES_SET_FOV, "set-fov\r\n"
		"\tSets the field of view - overrides all camera settings.  "
		"Takes 1 argument...\r\n"
		"\t1:\tNew FOV (degrees)\r\n"
	},

	// Echelon9
	{ OP_CUTSCENES_GET_FOV, "get-fov\r\n"
		"\tReturns the current field of view (in degrees) from the Main camera.\r\n\r\n"
		"Returns a numeric value.  Takes no arguments."
	},

	{ OP_CUTSCENES_RESET_FOV, "reset-fov\r\n"
		"\tResets the field of view.  "
	},

	{ OP_CUTSCENES_RESET_CAMERA, "reset-camera\r\n"
		"\tReleases cutscene camera control.  "
		"Takes 1 optional argument...\r\n"
		"\t(optional)\r\n"
		"\t1:\tReset camera data (Position, facing, FOV...) (default: false)"
	},

	{ OP_CUTSCENES_SHOW_SUBTITLE, "show-subtitle (deprecated)\r\n"
		"\tDisplays a subtitle, either an image or a string of text.  As this operator tries to combine two functions into one and does not adjust coordinates for screen formats, it has been deprecated.\r\n"
		"Takes 4 to 13 arguments...\r\n"
		"\t1:\tX position (negative value to be from right of screen)\r\n"
		"\t2:\tY position (negative value to be from bottom of screen)\r\n"
		"\t3:\tText to display\r\n"
		"\t4:\tTime to be displayed, not including fadein/out\r\n"
		"\t5:\tImage name (optional)\r\n"
		"\t6:\tFade in time (optional)\r\n"
		"\t7:\tCenter horizontally? (optional)\r\n"
		"\t8:\tCenter vertically? (optional)\r\n"
		"\t9:\tWidth (optional)\r\n"
		"\t10:\tText red component (0-255) (optional)\r\n"
		"\t11:\tText green component (0-255) (optional)\r\n"
		"\t12:\tText blue component (0-255) (optional)\r\n"
		"\t13:\tDrawn after shading? (optional)"
	},

	{ OP_CUTSCENES_SHOW_SUBTITLE_TEXT, "show-subtitle-text\r\n"
		"\tDisplays a subtitle in the form of text.  Note that because of the constraints of the SEXP type system, textual subtitles are currently limited to 31 characters or fewer.\r\n"
		"Takes 6 to 13 arguments...\r\n"
		"\t1:\tText to display\r\n"
		"\t2:\tX position, from 0 to 100% (positive measures from the left; negative measures from the right)\r\n"
		"\t3:\tY position, from 0 to 100% (positive measures from the top; negative measures from the bottom)\r\n"
		"\t4:\tCenter horizontally? (if true, overrides argument #2)\r\n"
		"\t5:\tCenter vertically? (if true, overrides argument #3)\r\n"
		"\t6:\tTime (in milliseconds) to be displayed, not including fade-in/fade-out\r\n"
		"\t7:\tFade time (in milliseconds) to be used for both fade-in and fade-out (optional)\r\n"
		"\t8:\tParagraph width, from 1 to 100% (optional; 0 uses default 200 pixels)\r\n"
		"\t9:\tText red component (0-255) (optional)\r\n"
		"\t10:\tText green component (0-255) (optional)\r\n"
		"\t11:\tText blue component (0-255) (optional)\r\n"
		"\t12:\tText font (optional)\r\n"
		"\t13:\tDrawn after shading? (optional)"
	},

	{ OP_CUTSCENES_SHOW_SUBTITLE_IMAGE, "show-subtitle-image\r\n"
		"\tDisplays a subtitle in the form of an image.\r\n"
		"Takes 8 to 10 arguments...\r\n"
		"\t1:\tImage to display\r\n"
		"\t2:\tX position, from 0 to 100% (positive measures from the left; negative measures from the right)\r\n"
		"\t3:\tY position, from 0 to 100% (positive measures from the top; negative measures from the bottom)\r\n"
		"\t4:\tCenter horizontally? (if true, overrides argument #2)\r\n"
		"\t5:\tCenter vertically? (if true, overrides argument #3)\r\n"
		"\t6:\tImage width, from 1 to 100% (0 uses original width)\r\n"
		"\t7:\tImage height, from 1 to 100% (0 uses original height)\r\n"
		"\t8:\tTime (in milliseconds) to be displayed, not including fade-in/fade-out\r\n"
		"\t9:\tFade time (in milliseconds) to be used for both fade-in and fade-out (optional)\r\n"
		"\t10:\tDrawn after shading? (optional)"
	},

	{ OP_CUTSCENES_SET_TIME_COMPRESSION, "set-time-compression\r\n"
		"\tSets the time compression and prevents it from being changed by the user.  "
		"Takes 1 to 3 arguments...\r\n"
		"\t1:\tNew time compression (% of 1x)\r\n"
		"\t2:\tTime in ms for change to take\r\n"
		"\t3:\tTime compression to start from\r\n"
	},

	{ OP_CUTSCENES_RESET_TIME_COMPRESSION, "reset-time-compression\r\n"
		"\tResets the time compression - that is to say, time compression is set back to 1x, "
		"and the time compression controls are unlocked.  Always call this when done with set-time-compression.  "
	},

	{ OP_CUTSCENES_FORCE_PERSPECTIVE, "lock-perspective\r\n"
		"\tPrevents or allows the player from changing the view mode.  "
		"Takes 1 or 2 arguments...\r\n"
		"\t1:\tTrue to lock the view mode, false to unlock it\r\n"
		"\t2:\tWhat view mode to lock; 0 for first-person, 1 for chase, 2 for external, 3 for top-down"
	},

	{ OP_SET_CAMERA_SHUDDER, "set-camera-shudder\r\n"
		"\tCauses the camera to shudder.  Currently this will only work if the camera is showing the player's viewpoint (i.e. the HUD).\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1: Time (in milliseconds)\r\n"
		"\t2: Intensity.  For comparison, the Maxim has an intensity of 1440."
	},

	{ OP_JUMP_NODE_SET_JUMPNODE_NAME, "set-jumpnode-name\r\n"
		"\tSets the name of a jump node. Takes 2 arguments...\r\n"
		"\t1: Name of jump node to change name for\r\n"
		"\t2: New name for jump node\r\n\r\n"
		"\tNote: SEXPs referencing the old name will not work after the name change.\r\n"
	},

	{ OP_JUMP_NODE_SET_JUMPNODE_COLOR, "set-jumpnode-color\r\n"
		"\tSets the color of a jump node.  "
		"Takes 5 arguments...\r\n"
		"\t1:\tJump node to change color for\r\n"
		"\t2:\tRed value\r\n"
		"\t3:\tGreen value\r\n"
		"\t4:\tBlue value\r\n"
		"\t5:\tAlpha value\r\n"
	},

	{ OP_JUMP_NODE_SET_JUMPNODE_MODEL, "set-jumpnode-model\r\n"
		"\tSets the model of a jump node.  "
		"Takes 3 arguments...\r\n"
		"\t1:\tJump node to change model for\r\n"
		"\t2:\tModel filename\r\n"
		"\t3:\tShow as normal model. When this is true, the jumpnode will be rendered like a normal model.\r\n"
	},

	{ OP_JUMP_NODE_SHOW_JUMPNODE, "show-jumpnode\r\n"
		"\tSets a jump node to display on the screen.\r\n"
		"\tAny:\tJump node to show\r\n"
	},

	{ OP_JUMP_NODE_HIDE_JUMPNODE, "hide-jumpnode\r\n"
		"\tSets a jump node to not display on the screen.\r\n"
		"\tAny:\tJump node to hide\r\n"
	},

	// taylor
	{ OP_SET_SKYBOX_MODEL, "set-skybox-model\r\n"
		"\tSets the current skybox model.  Takes 1 argument...\r\n"
		"\t1:\tModel filename (with .pof extension) to switch to\r\n\r\n"
		"Note: If the model filename is set to \"default\" with no extension then it will switch to the mission supplied default skybox."
	},

	// Goober5000
	{ OP_SET_SKYBOX_ORIENT, "set-skybox-orientation\r\n"
		"\tSets the current skybox orientation.  Takes 3 arguments...\r\n"
		"\t1:\tPitch\r\n"
		"\t2:\tBank\r\n"
		"\t3:\tHeading\r\n"
	},

	{ OP_ADD_BACKGROUND_BITMAP, "add-background-bitmap\r\n"
		"\tAdds a background bitmap to the sky.  Returns an integer that is stored in a variable so it can be deleted using remove-background-bitmap\r\n\r\n"
		"Takes 9 arguments...\r\n"
		"\t1:\tBackground bitmap name\r\n"
		"\t2:\tPitch\r\n"
		"\t3:\tBank\r\n"
		"\t4:\tHeading\r\n"
		"\t5:\tX scale (expressed as a percentage of the original size of the bitmap)\r\n"
		"\t6:\tY scale (expressed as a percentage of the original size of the bitmap)\r\n"
		"\t7:\tX divisions.\r\n"
		"\t8:\tY divisions.\r\n"
		"\t9:\tVariable in which to store result\r\n"
	},

	{ OP_REMOVE_BACKGROUND_BITMAP, "remove-background-bitmap\r\n"
		"\tRemoves the nth background bitmap from the mission\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tZero based bitmap index from the \'Bitmap\' box in the background editor\r\n"
		"\t\t\tYou can also use the result of a previous call to add-background-bitmap to remove that added bitmap\r\n"
	},

	{ OP_ADD_SUN_BITMAP, "add-sun-bitmap\r\n"
		"\tAdds a sun bitmap to the sky.  Returns an integer that is stored in a variable so it can be deleted using remove-sun-bitmap\r\n\r\n"
		"Takes 6 arguments...\r\n"
		"\t1:\tSun bitmap name\r\n"
		"\t2:\tPitch\r\n"
		"\t3:\tBank\r\n"
		"\t4:\tHeading\r\n"
		"\t5:\tScale (expressed as a percentage of the original size of the bitmap)\r\n"
		"\t6:\tVariable in which to store result\r\n"
	},

	{ OP_REMOVE_SUN_BITMAP, "remove-sun-bitmap\r\n"
		"\tRemoves the nth sun from the mission\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tZero based sun index from the \'Suns\' box in the background editor\r\n"
		"\t\t\tYou can also use the result of a previous call to add-sun-bitmap to remove that added sun\r\n"
	},

	{ OP_NEBULA_CHANGE_STORM, "nebula-change-storm\r\n"
		"\tChanges the current nebula storm\r\n\r\n"
		"Takes 1 argument...\r\n"
		"\t1:\tNebula storm to change to\r\n"
	},

	{ OP_NEBULA_TOGGLE_POOF, "nebula-toggle-poof\r\n"
		"\tToggles the state of a nebula poof\r\n\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tName of nebula poof to toggle\r\n"
		"\t2:\tA True boolean expression will toggle this poof on.  A false one will do the opposite."
	},

	{OP_SCRIPT_EVAL_NUM, "script-eval-num\r\n"
		"\tEvaluates script to return a number"
		"Takes 1 argument...\r\n"
		"\t1:\tScript\r\n"
	},

	{ OP_DISABLE_ETS, "disable-ets\r\n"
		"\tSwitches a ships' ETS system off\r\n\r\n"
		"Takes at least 1 argument...\r\n"
		"\tAll:\tList of ships this sexp applies to\r\n"
	},

	{ OP_ENABLE_ETS, "enable-ets\r\n"
		"\tSwitches a ships' ETS system on\r\n\r\n"
		"Takes at least 1 argument...\r\n"
		"\tAll:\tList of ships this sexp applies to\r\n"
	},

	{OP_SCRIPT_EVAL_STRING, "script-eval-string\r\n"
		"\tEvaluates script to return a string"
		"Takes 1 argument...\r\n"
		"\t1:\tScript\r\n"
	},

	{OP_SCRIPT_EVAL, "script-eval\r\n"
		"\tEvaluates script"
		"Takes at least 1 argument...\r\n"
		"\t1:\tScript to evaluate\r\n"
	},

	{OP_FORCE_GLIDE, "force-glide\r\n"
		"\tForces a given ship into glide mode, provided it is capable of gliding. Note that the player will not be able to leave glide mode on his own,, and that a ship in glide mode cannot warp out or enter autopilot."
		"Takes 2 Arguments...\r\n"
		"\t1:\tShip to force\r\n"
		"\t2:\tTrue to activate glide, False to deactivate\r\n"
	},

	{OP_HUD_SET_DIRECTIVE, "hud-set-directive\r\n"
		"\tSets the text of a given custom hud gauge to the provided text."
		"Takes 2 Arguments...\r\n"
		"\t1:\tHUD Gauge name\r\n"
		"\t2:\tText that will be displayed. This text will be treated as directive text, meaning that references to mapped keys will be replaced with the user's preferences.\r\n"
	},

	{OP_HUD_GAUGE_SET_ACTIVE, "hud-gauge-set-active\r\n"
		"\tActivates or deactivates a given custom gauge."
		"Takes 2 Arguments...\r\n"
		"\t1:\tHUD Gauge name\r\n"
		"\t2:\tBoolean, whether or not to display this gauge\r\n"
	},

	{OP_HUD_CLEAR_MESSAGES, "hud-clear-messages\r\n"
		"\tClears active messages displayed on the HUD."
		"Takes no arguments\r\n"
	},

	{OP_HUD_ACTIVATE_GAUGE_TYPE, "hud-activate-gauge-type\r\n"
		"\tActivates or deactivates all hud gauges of a given type."
		"Takes 2 Arguments...\r\n"
		"\t1:\tGauge Type\r\n"
		"\t2:\tBoolean, whether or not to display this gauge\r\n"
	},

	{OP_ADD_TO_COLGROUP, "add-to-collision-group\r\n"
		"\tAdds a ship to the specified collision group(s). Note that there are 32 collision groups,\r"
		"\tand that an object may be in several collision groups at the same time\r\n"
		"Takes 2 or more Arguments...\r\n"
		"\t1:\tObject to add\r\n"
		"\t2+:\tGroup IDs. Valid IDs are 0 through 31 inclusive.\r\n"
	},

	{OP_REMOVE_FROM_COLGROUP, "remove-from-collision-group\r\n"
		"\tRemoves a ship from the specified collision group(s). Note that there are 32 collision groups,\n"
		"\tand that an object may be in several collision groups at the same time\r\n"
		"Takes 2 or more Arguments...\r\n"
		"\t1:\tObject to add\r\n"
		"\t2+:\tGroup IDs. Valid IDs are 0 through 31 inclusive.\r\n"
	},

	{OP_GET_COLGROUP_ID, "get-collision-group\r\n"
		"\tReturns an objects' collision group ID. Note that this ID is a bitfield.\r\n"
		"Takes 1 Argument...\r\n"
		"\t1:\tObject name\r\n"
	},

	//Valathil
	{OP_SHIP_EFFECT, "ship-effect\r\n"
		"\tPlays an animated shader effect on the ship(s) or wing(s).\r\n"
		"Takes 3 or more arguments...\r\n"
		"\t1:\tEffect name (as defined in post_processing.tbl)\r\n"
		"\t2:\tHow long the effect should take in milliseconds\r\n"
		"\tRest:\tShip or wing name\r\n"
	},

	{OP_CLEAR_SUBTITLES, "clear-subtitles\r\n"
		"\tClears the subtitle queue completely.\r\n"
	},

	{OP_SET_THRUSTERS, "set-thrusters-status\r\n"
		"\tManipulates the thrusters on a ship.\r\n"
		"Takes 2 or more arguments...\r\n"
		"\t1:\tBoolean, true sets thrusters to visible, false deactivates them.\r\n"
		"\t2:\tRest: List of ships this sexp will work on.\r\n"
	},

	{OP_SET_PLAYER_THROTTLE_SPEED, "set-player-throttle-speed\r\n"
		"\tSets a player's throttle to a percentage of their maximum speed.\r\n"
		"\tThis SEXP has no effect if used on an AI ship, however it will still work on an AI-controlled player ship.\r\n"
		"Takes 2 arguments...\r\n"
		"\t1:\tThe player ship to set the throttle of.\r\n"
		"\t2:\tThe percentage of the player's maximum speed to set their throttle to.\r\n"
		"\t\tThis is capped to either 0 or 100 if outside the valid range."
	}
};



op_menu_struct op_menu[] =
{
	{ "Objectives",		OP_CATEGORY_OBJECTIVE },
	{ "Time",			OP_CATEGORY_TIME },
	{ "Logical",		OP_CATEGORY_LOGICAL },
	{ "Arithmetic",		OP_CATEGORY_ARITHMETIC },
	{ "Status",			OP_CATEGORY_STATUS },
	{ "Change",			OP_CATEGORY_CHANGE },
	{ "Conditionals",	OP_CATEGORY_CONDITIONAL },
	{ "Ai goals",		OP_CATEGORY_AI },
	{ "Event/Goals",	OP_CATEGORY_GOAL_EVENT },
	{ "Training",		OP_CATEGORY_TRAINING },
};

// Goober5000's subcategorization of the Change menu (and possibly other menus in the future,
// if people so choose - see sexp.h)
op_menu_struct op_submenu[] =
{
	{	"Messaging and Mission Goals",	CHANGE_SUBCATEGORY_MESSAGING_AND_MISSION_GOALS		},
	{	"AI and IFF",					CHANGE_SUBCATEGORY_AI_AND_IFF						},
	{	"Subsystems and Cargo",			CHANGE_SUBCATEGORY_SUBSYSTEMS_AND_CARGO				},
	{	"Ship Status",					CHANGE_SUBCATEGORY_SHIP_STATUS						},
	{	"Beams and Turrets",			CHANGE_SUBCATEGORY_BEAMS_AND_TURRETS				},
	{	"Mission and Campaign",			CHANGE_SUBCATEGORY_MISSION_AND_CAMPAIGN				},
	{	"Models and Textures",			CHANGE_SUBCATEGORY_MODELS_AND_TEXTURES				},
	{	"Coordinate Manipulation",		CHANGE_SUBCATEGORY_COORDINATE_MANIPULATION			},
	{	"Music and Sound",				CHANGE_SUBCATEGORY_MUSIC_AND_SOUND					},
	{	"Hud",							CHANGE_SUBCATEGORY_HUD								},
	{	"Nav",							CHANGE_SUBCATEGORY_NAV								},
	{	"Cutscenes",					CHANGE_SUBCATEGORY_CUTSCENES						},
	{	"Jump Nodes",					CHANGE_SUBCATEGORY_JUMP_NODES						},
	{	"Backgrounds and Nebula",		CHANGE_SUBCATEGORY_BACKGROUND_AND_NEBULA			},
	{	"Special",						CHANGE_SUBCATEGORY_SPECIAL							},
	{	"Multiplayer and Mission Config",	STATUS_SUBCATEGORY_MULTIPLAYER_AND_MISSION_CONFIG	},
	{	"Weapons, Shields, and Engines",	STATUS_SUBCATEGORY_SHIELDS_ENGINES_AND_WEAPONS	},
	{	"Cargo",						STATUS_SUBCATEGORY_CARGO							},
	{	"Ship Status",					STATUS_SUBCATEGORY_SHIP_STATUS						},
	{	"Damage",						STATUS_SUBCATEGORY_DAMAGE							},
	{	"Distance and Coordinates",		STATUS_SUBCATEGORY_DISTANCE_AND_COORDINATES			},
	{	"Kills and Scoring",			STATUS_SUBCATEGORY_KILLS_AND_SCORING				},
};
int Num_sexp_help = sizeof(Sexp_help) / sizeof(sexp_help_struct);
int Num_op_menus = sizeof(op_menu) / sizeof(op_menu_struct);
int Num_submenus = sizeof(op_submenu) / sizeof(op_menu_struct);

/**
 * Internal file used by output_sexps, should not be called from output_sexps
 */
static void output_sexp_html(int sexp_idx, FILE *fp)
{
	if(sexp_idx < 0 || sexp_idx > Num_operators)
		return;

	bool printed=false;

	for(int i = 0; i < Num_sexp_help; i++)
	{
		if(Sexp_help[i].id == Operators[sexp_idx].value)
		{
			char* new_buf = new char[2*strlen(Sexp_help[i].help)];
			char* dest_ptr = new_buf;
			char* curr_ptr = Sexp_help[i].help;
			char* end_ptr = curr_ptr + strlen(Sexp_help[i].help);
			while(curr_ptr < end_ptr)
			{
				if(*curr_ptr == '\n')
				{
					strcpy(dest_ptr, "\n<br>");
					dest_ptr+=5;
				}
				else
				{
					*dest_ptr++ = *curr_ptr;
				}
				curr_ptr++;
			}
			*dest_ptr = '\0';

			fprintf(fp, "<dt><b>%s</b></dt>\n<dd>%s</dd>\n", Operators[sexp_idx].text, new_buf);
			delete[] new_buf;

			printed = true;
		}
	}

	if(!printed)
		fprintf(fp, "<dt><b>%s</b></dt>\n<dd>Min arguments: %d, Max arguments: %d</dd>\n", Operators[sexp_idx].text, Operators[sexp_idx].min, Operators[sexp_idx].max);
}

/**
 * Output sexp.html file
 */
bool output_sexps(char *filepath)
{
	FILE *fp = fopen(filepath,"w");

	if(fp == NULL)
	{
		MessageBox(NULL,"Error creating SEXP operator list", "Error", MB_OK);
		return false; 
	}

	//Header
	if (FS_VERSION_BUILD == 0 && FS_VERSION_REVIS == 0) //-V547
	{
		fprintf(fp, "<html>\n<head>\n\t<title>SEXP Output - FSO v%i.%i</title>\n</head>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR);
		fputs("<body>", fp);
		fprintf(fp,"\t<h1>SEXP Output - FSO v%i.%i</h1>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR);
	}
	else if (FS_VERSION_REVIS == 0)
	{
		fprintf(fp, "<html>\n<head>\n\t<title>SEXP Output - FSO v%i.%i.%i</title>\n</head>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD);
		fputs("<body>", fp);
		fprintf(fp,"\t<h1>SEXP Output - FSO v%i.%i.%i</h1>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD);
	}
	else
	{
		fprintf(fp, "<html>\n<head>\n\t<title>SEXP Output - FSO v%i.%i.%i.%i</title>\n</head>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, FS_VERSION_REVIS);
		fputs("<body>", fp);
		fprintf(fp,"\t<h1>SEXP Output - FSO v%i.%i.%i.%i</h1>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, FS_VERSION_REVIS);
	}

	SCP_vector<int> done_sexp_ids;
	int x,y,z;

	//Output an overview
	fputs("<dl>", fp);
	for(x = 0; x < Num_op_menus; x++)
	{
		fprintf(fp, "<dt><a href=\"#%d\">%s</a></dt>", (op_menu[x].id & OP_CATEGORY_MASK), op_menu[x].name);
		for(y = 0; y < Num_submenus; y++)
		{
			if(((op_submenu[y].id & OP_CATEGORY_MASK) == op_menu[x].id))
			{
				fprintf(fp, "<dd><a href=\"#%d\">%s</a></dd>", op_submenu[y].id & (OP_CATEGORY_MASK | SUBCATEGORY_MASK), op_submenu[y].name);
			}
		}
	}
	fputs("</dl>", fp);

	//Output the full descriptions
	fputs("<dl>", fp);
	for(x = 0; x < Num_op_menus; x++)
	{
		fprintf(fp, "<dt id=\"%d\"><h2>%s</h2></dt>\n", (op_menu[x].id & OP_CATEGORY_MASK), op_menu[x].name);
		fputs("<dd>", fp);
		fputs("<dl>", fp);
		for(y = 0; y < Num_submenus; y++)
		{
			if(((op_submenu[y].id & OP_CATEGORY_MASK) == op_menu[x].id))
			{
				fprintf(fp, "<dt id=\"%d\"><h3>%s</h3></dt>\n", op_submenu[y].id & (OP_CATEGORY_MASK | SUBCATEGORY_MASK), op_submenu[y].name);
				fputs("<dd>", fp);
				fputs("<dl>", fp);
				for(z = 0; z < Num_operators; z++)
				{
					if((get_category(Operators[z].value) == op_menu[x].id)
						&& (get_subcategory(Operators[z].value) != -1)
						&& (get_subcategory(Operators[z].value) == op_submenu[y].id))
					{
						output_sexp_html(z, fp);
					}
				}
				fputs("</dl>", fp);
				fputs("</dd>", fp);
			}
		}
		for(z = 0; z < Num_operators; z++)
		{
			if((get_category(Operators[z].value) == op_menu[x].id)
				&& (get_subcategory(Operators[z].value) == -1))
			{
				output_sexp_html(z, fp);
			}
		}
		fputs("</dl>", fp);
		fputs("</dd>", fp);
	}
	for(z = 0; z < Num_operators; z++)
	{
		if(!get_category(Operators[z].value))
		{
			output_sexp_html(z, fp);
		}
	}
	fputs("</dl>", fp);
	fputs("</body>\n</html>\n", fp);

	fclose(fp);

	return true;
}
