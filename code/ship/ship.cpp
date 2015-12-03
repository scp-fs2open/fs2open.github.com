
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include <setjmp.h>

#include "ai/aigoals.h"
#include "asteroid/asteroid.h"
#include "autopilot/autopilot.h"
#include "cmdline/cmdline.h"
#include "cmeasure/cmeasure.h"
#include "debugconsole/console.h"
#include "fireball/fireballs.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/eventmusic.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "globalincs/def_files.h"
#include "globalincs/linklist.h"
#include "graphics/gropenglshader.h"
#include "hud/hud.h"
#include "hud/hudartillery.h"
#include "hud/hudets.h"
#include "hud/hudmessage.h"
#include "hud/hudshield.h"
#include "hud/hudsquadmsg.h"
#include "hud/hudtargetbox.h"
#include "hud/hudwingmanstatus.h"
#include "iff_defs/iff_defs.h"
#include "io/joy_ff.h"
#include "io/timer.h"
#include "jumpnode/jumpnode.h"
#include "lighting/lighting.h"
#include "localization/localize.h"
#include "math/fvi.h"
#include "math/staticrand.h"
#include "math/vecmat.h"
#include "mission/missioncampaign.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h"
#include "missionui/missionshipchoice.h"
#include "missionui/redalert.h"
#include "mod_table/mod_table.h"
#include "model/model.h"
#include "model/modelrender.h"
#include "nebula/neb.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "object/deadobjectdock.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "object/objectdock.h"
#include "object/objectshield.h"
#include "object/objectsnd.h"
#include "object/waypoint.h"
#include "parse/parselo.h"
#include "parse/scripting.h"
#include "particle/particle.h"
#include "playerman/player.h"
#include "radar/radar.h"
#include "radar/radarsetup.h"
#include "render/3d.h"
#include "ship/afterburner.h"
#include "ship/ship.h"
#include "ship/shipcontrails.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"
#include "ship/subsysdamage.h"
#include "species_defs/species_defs.h"
#include "weapon/beam.h"
#include "weapon/corkscrew.h"
#include "weapon/emp.h"
#include "weapon/flak.h"								//phreak addded 11/05/02 for flak primaries
#include "weapon/shockwave.h"
#include "weapon/swarm.h"
#include "weapon/weapon.h"

#define NUM_SHIP_SUBSYSTEM_SETS			20		// number of subobject sets to use (because of the fact that it's a linked list,
												//     we can't easily go fully dynamic)

#define NUM_SHIP_SUBSYSTEMS_PER_SET		200 	// Reduced from 1000 to 400 by MK on 4/1/98.  DTP; bumped from 700 to 2100
												// Reduced to 200 by taylor on 3/13/07  --  it's managed in dynamically allocated sets now
												//    Highest I saw was 164 in sm2-03a which Sandeep says has a lot of ships.
												//    JAS: sm3-01 needs 460.   You cannot know this number until *all* ships
												//    have warped in.   So I put code in the paging code which knows all ships
												//    that will warp in.

static int Num_ship_subsystems = 0;
static int Num_ship_subsystems_allocated = 0;

static ship_subsys *Ship_subsystems[NUM_SHIP_SUBSYSTEM_SETS] = { NULL };
ship_subsys ship_subsys_free_list;

extern bool splodeing;
extern float splode_level;
extern int splodeingtexture;

extern int Cmdline_nohtl;

extern void fs2netd_add_table_validation(const char *tblname);

#define SHIP_REPAIR_SUBSYSTEM_RATE	0.01f

int	Ai_render_debug_flag=0;
#ifndef NDEBUG
int	Ship_sphere_check = 0;
int	Ship_auto_repair = 1;		// flag to indicate auto-repair of subsystem should occur
extern void render_path_points(object *objp);
#endif

int	Num_wings = 0;
int	Num_reinforcements = 0;
ship	Ships[MAX_SHIPS];

ship	*Player_ship;
int		*Player_cockpit_textures;
SCP_vector<cockpit_display> Player_displays;

wing	Wings[MAX_WINGS];
int	ships_inited = 0;
int armor_inited = 0;

int	Starting_wings[MAX_STARTING_WINGS];  // wings player starts a mission with (-1 = none)

// Goober5000
int Squadron_wings[MAX_SQUADRON_WINGS];
int TVT_wings[MAX_TVT_WINGS];

// Goober5000
char Starting_wing_names[MAX_STARTING_WINGS][NAME_LENGTH];
char Squadron_wing_names[MAX_SQUADRON_WINGS][NAME_LENGTH];
char TVT_wing_names[MAX_TVT_WINGS][NAME_LENGTH];

SCP_vector<engine_wash_info> Engine_wash_info;
engine_wash_info *get_engine_wash_pointer(char* engine_wash_name);

void ship_reset_disabled_physics(object *objp, int ship_class);

// forward declaring for parse_ship()
int ship_info_lookup_sub(const char *token);

// information for ships which have exited the game
SCP_vector<exited_ship> Ships_exited;

int	Num_engine_wash_types;
int	Num_ship_subobj_types;
int	Num_ship_subobjects;
int	Player_ship_class;	// needs to be player specific, move to player structure	

#define		SHIP_OBJ_USED	(1<<0)				// flag used in ship_obj struct
#define		MAX_SHIP_OBJS	MAX_SHIPS			// max number of ships tracked in ship list
ship_obj		Ship_objs[MAX_SHIP_OBJS];		// array used to store ship object indexes
ship_obj		Ship_obj_list;							// head of linked list of ship_obj structs

SCP_vector<ship_info>	Ship_info;
reinforcements	Reinforcements[MAX_REINFORCEMENTS];
SCP_vector<ship_info>	Ship_templates;

SCP_vector<ship_type_info> Ship_types;

SCP_vector<ArmorType> Armor_types;
SCP_vector<DamageTypeStruct>	Damage_types;

flag_def_list Armor_flags[] = {
	{ "ignore subsystem armor",		SAF_IGNORE_SS_ARMOR,	0 }
};

const int Num_armor_flags = sizeof(Armor_flags)/sizeof(flag_def_list);

flag_def_list Man_types[] = {
	{ "Bank right",		MT_BANK_RIGHT,	0 },
	{ "Bank left",		MT_BANK_LEFT,	0 },
	{ "Pitch up",		MT_PITCH_UP,	0 },
	{ "Pitch down",		MT_PITCH_DOWN,	0 },
	{ "Roll right",		MT_ROLL_RIGHT,	0 },
	{ "Roll left",		MT_ROLL_LEFT,	0 },
	{ "Slide right",	MT_SLIDE_RIGHT,	0 },
	{ "Slide left",		MT_SLIDE_LEFT,	0 },
	{ "Slide up",		MT_SLIDE_UP,	0 },
	{ "Slide down",		MT_SLIDE_DOWN,	0 },
	{ "Forward",		MT_FORWARD,		0 },
	{ "Reverse",		MT_REVERSE,		0 }
};

const int Num_man_types = sizeof(Man_types)/sizeof(flag_def_list);

// Goober5000 - I figured we should keep this separate
// from Comm_orders, considering how I redid it :p
// (and also because we may want to change either
// the order text or the flag text in the future)
flag_def_list Player_orders[] = {
	// common stuff
	{ "attack ship",		ATTACK_TARGET_ITEM,		0 },
	{ "disable ship",		DISABLE_TARGET_ITEM,	0 },
	{ "disarm ship",		DISARM_TARGET_ITEM,		0 },
	{ "disable subsys",		DISABLE_SUBSYSTEM_ITEM,	0 },
	{ "guard ship",			PROTECT_TARGET_ITEM,	0 },
	{ "ignore ship",		IGNORE_TARGET_ITEM,		0 },
	{ "form on wing",		FORMATION_ITEM,			0 },
	{ "cover me",			COVER_ME_ITEM,			0 },
	{ "attack any",			ENGAGE_ENEMY_ITEM,		0 },

	// transports mostly
	{ "dock",				CAPTURE_TARGET_ITEM,	0 },

	// support ships
	{ "rearm me",			REARM_REPAIR_ME_ITEM,	0 },
	{ "abort rearm",		ABORT_REARM_REPAIR_ITEM,	0 },

	// all ships
	{ "depart",				DEPART_ITEM,			0 },

	// extra stuff for support
	{ "stay near me",		STAY_NEAR_ME_ITEM,		0 },
	{ "stay near ship",		STAY_NEAR_TARGET_ITEM,	0 },
	{ "keep safe dist",		KEEP_SAFE_DIST_ITEM,	0 }
};

const int Num_player_orders = sizeof(Player_orders)/sizeof(flag_def_list);

// Use the last parameter here to tell the parser whether to stuff the flag into flags or flags2
flag_def_list Subsystem_flags[] = {
	{ "untargetable",			MSS_FLAG_UNTARGETABLE,		0 },
	{ "carry no damage",		MSS_FLAG_CARRY_NO_DAMAGE,	0 },
	{ "use multiple guns",		MSS_FLAG_USE_MULTIPLE_GUNS,	0 },
	{ "fire down normals",		MSS_FLAG_FIRE_ON_NORMAL,	0 },
	{ "check hull",				MSS_FLAG_TURRET_HULL_CHECK,	0 },
	{ "fixed firingpoints",		MSS_FLAG_TURRET_FIXED_FP,	0 },
	{ "salvo mode",				MSS_FLAG_TURRET_SALVO,		0 },
	{ "no subsystem targeting",	MSS_FLAG_NO_SS_TARGETING,	0 },
	{ "fire on target",			MSS_FLAG_FIRE_ON_TARGET,	0 },
	{ "reset when idle",		MSS_FLAG_TURRET_RESET_IDLE,	0 },
	{ "carry shockwave",		MSS_FLAG_CARRY_SHOCKWAVE,	0 },
	{ "allow landing",			MSS_FLAG_ALLOW_LANDING,		0 },
	{ "target requires fov",	MSS_FLAG_FOV_REQUIRED,		0 },
	{ "fov edge checks",		MSS_FLAG_FOV_EDGE_CHECK,	0 },
	{ "no replace",				MSS_FLAG_NO_REPLACE,		0 },
	{ "no live debris",			MSS_FLAG_NO_LIVE_DEBRIS,	0 },
	{ "ignore if dead",			MSS_FLAG_IGNORE_IF_DEAD,	0 },
	{ "allow vanishing",		MSS_FLAG_ALLOW_VANISHING,	0 },
	{ "damage as hull",			MSS_FLAG_DAMAGE_AS_HULL,	0 },
	{ "starts locked",          MSS_FLAG_TURRET_LOCKED,     0 },
	{ "no aggregate",			MSS_FLAG_NO_AGGREGATE,		0 },
	{ "wait for animation",     MSS_FLAG_TURRET_ANIM_WAIT,  0 },
	{ "play fire sound for player", MSS_FLAG2_PLAYER_TURRET_SOUND, 1},
	{ "only target if can fire",    MSS_FLAG2_TURRET_ONLY_TARGET_IF_CAN_FIRE, 1},
	{ "no disappear",			MSS_FLAG2_NO_DISAPPEAR, 1},
	{ "collide submodel",		MSS_FLAG2_COLLIDE_SUBMODEL, 1},
	{ "allow destroyed rotation",	MSS_FLAG2_DESTROYED_ROTATION, 1},
	{ "turret use ammo",		MSS_FLAG2_TURRET_USE_AMMO, 1},
	{ "autorepair if disabled",	MSS_FLAG2_AUTOREPAIR_IF_DISABLED, 1},
	{ "don't autorepair if disabled", MSS_FLAG2_NO_AUTOREPAIR_IF_DISABLED, 1},
};

const int Num_subsystem_flags = sizeof(Subsystem_flags)/sizeof(flag_def_list);


// NOTE: a var of:
//         "0"    means that it's a SIF_* flag
//         "1"    means that it's a SIF2_* flag
//         "255"  means that the option is obsolete and a warning should be generated
flag_def_list Ship_flags[] = {
	{ "no_collide",					SIF_NO_COLLIDE,				0 },
	{ "player_ship",				SIF_PLAYER_SHIP,			0 },
	{ "default_player_ship",		SIF_DEFAULT_PLAYER_SHIP,	0 },
	{ "repair_rearm",				SIF_SUPPORT,				0 },
	{ "cargo",						SIF_CARGO,					0 },
	{ "fighter",					SIF_FIGHTER,				0 },
	{ "bomber",						SIF_BOMBER,					0 },
	{ "transport",					SIF_TRANSPORT,				0 },
	{ "freighter",					SIF_FREIGHTER,				0 },
	{ "capital",					SIF_CAPITAL,				0 },
	{ "supercap",					SIF_SUPERCAP,				0 },
	{ "drydock",					SIF_DRYDOCK,				0 },
	{ "cruiser",					SIF_CRUISER,				0 },
	{ "navbuoy",					SIF_NAVBUOY,				0 },
	{ "sentrygun",					SIF_SENTRYGUN,				0 },
	{ "escapepod",					SIF_ESCAPEPOD,				0 },
	{ "stealth",					SIF_STEALTH,				0 },
	{ "no type",					SIF_NO_SHIP_TYPE,			0 },
	{ "ship copy",					SIF_SHIP_COPY,				0 },
	{ "in tech database",			SIF_IN_TECH_DATABASE | SIF_IN_TECH_DATABASE_M,	0 },
	{ "in tech database multi",		SIF_IN_TECH_DATABASE_M,		0 },
	{ "dont collide invisible",		SIF_SHIP_CLASS_DONT_COLLIDE_INVIS,	0 },
	{ "big damage",					SIF_BIG_DAMAGE,				0 },
	{ "corvette",					SIF_CORVETTE,				0 },
	{ "gas miner",					SIF_GAS_MINER,				0 },
	{ "awacs",						SIF_AWACS,					0 },
	{ "knossos",					SIF_KNOSSOS_DEVICE,			0 },
	{ "no_fred",					SIF_NO_FRED,				0 },
	{ "flash",						SIF2_FLASH,					1 },
	{ "surface shields",			SIF2_SURFACE_SHIELDS,		1 },
	{ "show ship",					SIF2_SHOW_SHIP_MODEL,		1 },
	{ "generate icon",				SIF2_GENERATE_HUD_ICON,		1 },
	{ "no weapon damage scaling",	SIF2_DISABLE_WEAPON_DAMAGE_SCALING,	1 },
	{ "gun convergence",			SIF2_GUN_CONVERGENCE,		1 },
	{ "no thruster geometry noise", SIF2_NO_THRUSTER_GEO_NOISE,	1 },
	{ "intrinsic no shields",		SIF2_INTRINSIC_NO_SHIELDS,	1 },
	{ "dynamic primary linking",    SIF2_DYN_PRIMARY_LINKING,	1 },
	{ "no primary linking",			SIF2_NO_PRIMARY_LINKING,	1 },
	{ "no pain flash",				SIF2_NO_PAIN_FLASH,			1 },
	{ "no ets",						SIF2_NO_ETS,				1 },
	{ "no lighting",				SIF2_NO_LIGHTING,			1 },
	{ "auto spread shields",		SIF2_AUTO_SPREAD_SHIELDS,	1 },
	{ "model point shields",		SIF2_MODEL_POINT_SHIELDS,	1 },
	{ "repair disabled subsystems", SIF2_SUBSYS_REPAIR_WHEN_DISABLED, 1},

	// to keep things clean, obsolete options go last
	{ "ballistic primaries",		-1,		255 }
};

const int Num_ship_flags = sizeof(Ship_flags) / sizeof(flag_def_list);

/*
++Here be dragons.. err.. begins the section for the ai targeting revision
++  First flag_def_list (& its size) for object types (ship/asteroid/weapon)
++  List of reasonable object flags (from object.h)
++  List of potentially useful ship class flags
++  List of potentially useful weapon class flags
*/
flag_def_list ai_tgt_objects[] = {
	{ "ship",		OBJ_SHIP,		0 },
	{ "asteroid",	OBJ_ASTEROID,	0 },
	{ "weapon",		OBJ_WEAPON,		0 }
};

const int num_ai_tgt_objects = sizeof(ai_tgt_objects) / sizeof(flag_def_list);

flag_def_list ai_tgt_obj_flags[] = {
	{ "no shields",			OF_NO_SHIELDS,			0 },
	{ "targetable as bomb",	OF_TARGETABLE_AS_BOMB,	0 }
};

const int num_ai_tgt_obj_flags = sizeof(ai_tgt_obj_flags) / sizeof(flag_def_list);

flag_def_list ai_tgt_ship_flags[] = {
	{ "afterburners",	SIF_AFTERBURNER,	0 },
	{ "big damage",		SIF_BIG_DAMAGE,		0 },
	{ "has awacs",		SIF_HAS_AWACS,		0 }
};

const int num_ai_tgt_ship_flags = sizeof(ai_tgt_ship_flags) / sizeof(flag_def_list);

flag_def_list ai_tgt_weapon_flags[] = {
	{ "bomb",				WIF_BOMB,				0 },
	{ "huge damage",		WIF_HUGE,				0 },
	{ "supercap damage",	WIF_SUPERCAP,			0 },
	{ "bomber+",			WIF_BOMBER_PLUS,		0 },
	{ "electronics",		WIF_ELECTRONICS,		0 },
	{ "puncture",			WIF_PUNCTURE,			0 },
	{ "emp",				WIF_EMP,				0 },
	{ "heat seeking",		WIF_HOMING_HEAT,		0 },
	{ "aspect seeking",		WIF_HOMING_ASPECT,		0 },
	{ "engine seeking",		WIF_HOMING_JAVELIN,		0 },
	{ "pierce shields",		WIF2_PIERCE_SHIELDS,	1 },
	{ "local ssm",			WIF2_LOCAL_SSM,			1 },
	{ "capital+",			WIF2_CAPITAL_PLUS,		1 }
};

//	Constant for flag,				Name of flag,				In flags or flags2
//  When adding new flags remember to bump MAX_SHIP_FLAG_NAMES in ship.h
ship_flag_name Ship_flag_names[] = {
	{SF_VAPORIZE,					"vaporize",						1,	},
	{SF_WARP_BROKEN,				"break-warp",					1,	},
	{SF_WARP_NEVER,					"never-warp",					1,	},
	{SF_SCANNABLE,					"scannable",					1,	},
	{SF_CARGO_REVEALED,				"cargo-known",					1,	},
	{SF_HIDDEN_FROM_SENSORS,		"hidden-from-sensors",			1,	},
	{SF2_STEALTH,					"stealth",						2,	},
	{SF2_FRIENDLY_STEALTH_INVIS,	"friendly-stealth-invisible",	2,	},
	{SF2_HIDE_SHIP_NAME,			"hide-ship-name",				2,	},
	{SF2_PRIMITIVE_SENSORS,			"primitive-sensors",			2,	},
	{SF2_AFTERBURNER_LOCKED,		"afterburners-locked",			2,	},
	{SF2_PRIMARIES_LOCKED,			"primaries-locked",				2,	},
	{SF2_SECONDARIES_LOCKED,		"secondaries-locked",			2,	},
	{SF2_NO_SUBSPACE_DRIVE,			"no-subspace-drive",			2,	},
	{SF2_DONT_COLLIDE_INVIS,		"don't-collide-invisible",		2,	},
	{SF2_NO_ETS,					"no-ets",						2,	},
	{SF2_TOGGLE_SUBSYSTEM_SCANNING,	"toggle-subsystem-scanning",	2,	},
	{SF2_NO_SECONDARY_LOCKON,		"no-secondary-lock-on",			2,	},
};

const int num_ai_tgt_weapon_flags = sizeof(ai_tgt_weapon_flags) / sizeof(flag_def_list);

SCP_vector <ai_target_priority> Ai_tp_list;

static int Laser_energy_out_snd_timer;	// timer so we play out of laser sound effect periodically
static int Missile_out_snd_timer;	// timer so we play out of laser sound effect periodically

SCP_vector<ship_counts>	Ship_type_counts;

// I don't want to do an AI cargo check every frame, so I made a global timer to limit check to
// every SHIP_CARGO_CHECK_INTERVAL ms.  Didn't want to make a timer in each ship struct.  Ensure
// inited to 1 at mission start.
static int Ship_cargo_check_timer;

static int Thrust_anim_inited = 0;

int ship_get_subobj_model_num(ship_info* sip, char* subobj_name);

SCP_vector<ship_effect> Ship_effects;

int ship_render_mode = MODEL_RENDER_ALL;
/**
 * Set the ship_obj struct fields to default values
 */
void ship_obj_list_reset_slot(int index)
{
	Ship_objs[index].flags = 0;
	Ship_objs[index].next = NULL;
	Ship_objs[index].prev = (ship_obj*)-1;
}

/**
 * If the given ship is in my squadron wings
 */
int ship_in_my_squadron(ship *shipp)
{
	int i;

	for (i=0; i<MAX_STARTING_WINGS; i++)
	{
		if (shipp->wingnum == Starting_wings[i])
			return 1;
	}

	for (i=0; i<MAX_TVT_WINGS; i++)
	{
		if (shipp->wingnum == TVT_wings[i])
			return 1;
	}

	// not in
	return 0;
}

/**
 * Initialise Ship_obj_list
 */
void ship_obj_list_init()
{
	int i;

	list_init(&Ship_obj_list);
	for ( i = 0; i < MAX_SHIP_OBJS; i++ ) {
		ship_obj_list_reset_slot(i);
	}
}

/**
 * Function to add a node to the Ship_obj_list.  Only
 * called from ::ship_create()
 */
int ship_obj_list_add(int objnum)
{
	int i;

	for ( i = 0; i < MAX_SHIP_OBJS; i++ ) {
		if ( !(Ship_objs[i].flags & SHIP_OBJ_USED) )
			break;
	}
	if ( i == MAX_SHIP_OBJS ) {
		Error(LOCATION, "Fatal Error: Ran out of ship object nodes\n");
		return -1;
	}
	
	Ship_objs[i].flags = 0;
	Ship_objs[i].objnum = objnum;
	list_append(&Ship_obj_list, &Ship_objs[i]);
	Ship_objs[i].flags |= SHIP_OBJ_USED;

	return i;
}

/**
 * Function to remove a node from the Ship_obj_list.  Only
 * called from ::ship_delete()
 */
void ship_obj_list_remove(int index)
{
	Assert(index >= 0 && index < MAX_SHIP_OBJS);
	list_remove( Ship_obj_list, &Ship_objs[index]);	
	ship_obj_list_reset_slot(index);
}

/**
 * Called from the save/restore code to re-create the Ship_obj_list
 */
void ship_obj_list_rebuild()
{
	object *objp;

	ship_obj_list_init();

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( objp->type == OBJ_SHIP ) {
			Ships[objp->instance].ship_list_index = ship_obj_list_add(OBJ_INDEX(objp));
		}
	}
}

ship_obj *get_ship_obj_ptr_from_index(int index)
{
	Assert(index >= 0 && index < MAX_SHIP_OBJS);
	return &Ship_objs[index];
}

/**
 * Return number of ships in the game.
 */
int ship_get_num_ships()
{
	int count;
	ship_obj *so;

	count = 0;
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) )
		count++;

	return count;
}

void engine_wash_info_init(engine_wash_info *ewi)
{
	ewi->name[0] = '\0';
	ewi->angle = PI / 10.0f;
	ewi->radius_mult = 1.0f;
	ewi->length = 500.0f;
	ewi->intensity = 1.0f;
}

/**
 * Parse an engine wash info record
 */
void parse_engine_wash(bool replace)
{
	engine_wash_info ewt;
	engine_wash_info_init(&ewt);

	engine_wash_info *ewp;
	bool create_if_not_found  = true;

	// name of engine wash info
	required_string("$Name:");
	stuff_string(ewt.name, F_NAME, NAME_LENGTH);

	if(optional_string("+nocreate")) {
		if(!replace) {
			Warning(LOCATION, "+nocreate flag used for engine wash in non-modular table");
		}
		create_if_not_found = false;
	}

	//Does this engine wash exist already?
	//If so, load this new info into it
	//Otherwise, increment Num_engine_wash_types
	ewp = get_engine_wash_pointer(ewt.name);
	if(ewp != NULL)
	{
		if(replace)
		{
			nprintf(("Warning", "More than one version of engine wash %s exists; using newer version.", ewt.name));
		}
		else
		{
			Error(LOCATION, "Error:  Engine wash %s already exists.  All engine wash names must be unique.", ewt.name);
		}
	}
	else
	{
		//Don't create engine wash if it has +nocreate and is in a modular table.
		if(!create_if_not_found && replace)
		{
			if ( !skip_to_start_of_string_either("$Name:", "#End")) {
				Int3();
			}
			return;
		}
		Engine_wash_info.push_back(ewt);
		ewp = &Engine_wash_info[Num_engine_wash_types++];
	}


	// half angle of cone of wash from thruster
	if(optional_string("$Angle:"))
	{
		stuff_float(&ewp->angle);
		ewp->angle *= (PI / 180.0f);
	}

	// radius multiplier for hemisphere around thruster pt
	if(optional_string("$Radius Mult:")) {
		stuff_float(&ewp->radius_mult);
	}

	// length of cone
	if(optional_string("$Length:")) {
		stuff_float(&ewp->length);
	}

	// intensity inside hemisphere (or at 0 distance from frustated cone)
	if(optional_string("$Intensity:")) {
		stuff_float(&ewp->intensity);
	}
}

char *Warp_types[] = {
	"Default",
	"Knossos",
	"Babylon5",
	"Galactica",
	"Homeworld",
	"Hyperspace",
};

int Num_warp_types = sizeof(Warp_types)/sizeof(char*);

int warptype_match(char *p)
{
	int i;
	for(i = 0; i < Num_warp_types; i++)
	{
		if(!stricmp(Warp_types[i], p))
			return i;
	}

	return -1;
}

char *Lightning_types[] = {
	"None",
	"Default",
};

int Num_lightning_types = sizeof(Lightning_types)/sizeof(char*);

int lightningtype_match(char *p)
{
	int i;
	for(i = 0; i < Num_lightning_types; i++)
	{
		if(!stricmp(Lightning_types[i], p))
			return i;
	}

	return -1;
}

// Kazan -- Volition had this set to 1500, Set it to 4K for WC Saga
#define SHIP_MULTITEXT_LENGTH 4096
#define DEFAULT_DELTA_BANK_CONST	0.5f

#define CHECK_THEN_COPY(attribute) \
do {\
	if (other.attribute != NULL)\
		attribute = vm_strdup( other.attribute );\
} while(false)

void ship_info::clone(const ship_info& other)
{
	strcpy_s(name, other.name);
	strcpy_s(alt_name, other.alt_name);
	strcpy_s(short_name, other.short_name);
	species = other.species;
	class_type = other.class_type;

	CHECK_THEN_COPY(type_str);
	CHECK_THEN_COPY(maneuverability_str);
	CHECK_THEN_COPY(armor_str);
	CHECK_THEN_COPY(manufacturer_str);
	CHECK_THEN_COPY(desc);
	CHECK_THEN_COPY(tech_desc);

	strcpy_s(tech_title, other.tech_title);

	CHECK_THEN_COPY(ship_length);
	CHECK_THEN_COPY(gun_mounts);
	CHECK_THEN_COPY(missile_banks);

	strcpy_s(cockpit_pof_file, other.cockpit_pof_file);
	cockpit_offset = other.cockpit_offset;
	strcpy_s(pof_file, other.pof_file);
	strcpy_s(pof_file_hud, other.pof_file_hud);
	num_detail_levels = other.num_detail_levels;
	memcpy(detail_distance, other.detail_distance, sizeof(int) * MAX_SHIP_DETAIL_LEVELS);
	collision_lod = other.collision_lod;

	// I'm not sure if these three are A) a good idea or B) relevant at all. -MageKing17
	cockpit_model_num = other.cockpit_model_num;
	model_num = other.model_num;
	model_num_hud = other.model_num_hud;

	hud_target_lod = other.hud_target_lod;
	density = other.density;
	damp = other.damp;
	rotdamp = other.rotdamp;
	delta_bank_const = other.delta_bank_const;
	max_vel = other.max_vel;
	max_rotvel = other.max_rotvel;
	rotation_time = other.rotation_time;
	srotation_time = other.srotation_time;
	max_rear_vel = other.max_rear_vel;
	forward_accel = other.forward_accel;
	forward_decel = other.forward_decel;
	slide_accel = other.slide_accel;
	slide_decel = other.slide_decel;

	strcpy_s(warpin_anim, other.warpin_anim);
	warpin_radius = other.warpin_radius;
	warpin_snd_start = other.warpin_snd_start;
	warpin_snd_end = other.warpin_snd_end;
	warpin_speed = other.warpin_speed;
	warpin_time = other.warpin_time;
	warpin_decel_exp = other.warpin_decel_exp;
	warpin_type = other.warpin_type;

	strcpy_s(warpout_anim, other.warpout_anim);
	warpout_radius = other.warpout_radius;
	warpout_snd_start = other.warpout_snd_start;
	warpout_snd_end = other.warpout_snd_end;
	warpout_engage_time = other.warpout_engage_time;
	warpout_speed = other.warpout_speed;
	warpout_time = other.warpout_time;
	warpout_accel_exp = other.warpout_accel_exp;
	warpout_type = other.warpout_type;

	warpout_player_speed = other.warpout_player_speed;

	flags = other.flags;
	flags2 = other.flags2;
	ai_class = other.ai_class;
	max_speed = other.max_speed;
	min_speed = other.min_speed;
	max_accel = other.max_accel;

	collision_damage_type_idx = other.collision_damage_type_idx;
	collision_physics = other.collision_physics;

	memcpy(&shockwave, &other.shockwave, sizeof(shockwave_create_info));
	explosion_propagates = other.explosion_propagates;
	big_exp_visual_rad = other.big_exp_visual_rad;
	prop_exp_rad_mult = other.prop_exp_rad_mult;
	death_roll_r_mult = other.death_roll_r_mult;
	death_fx_r_mult = other.death_fx_r_mult;
	death_roll_time_mult = other.death_roll_time_mult;
	death_roll_base_time = other.death_roll_base_time;
	death_fx_count = other.death_fx_count;
	shockwave_count = other.shockwave_count;
	explosion_bitmap_anims = other.explosion_bitmap_anims;
	vaporize_chance = other.vaporize_chance;

	impact_spew = other.impact_spew;
	damage_spew = other.damage_spew;
	split_particles = other.split_particles;
	knossos_end_particles = other.knossos_end_particles;
	regular_end_particles = other.regular_end_particles;

	debris_min_lifetime = other.debris_min_lifetime;
	debris_max_lifetime = other.debris_max_lifetime;
	debris_min_speed = other.debris_min_speed;
	debris_max_speed = other.debris_max_speed;
	debris_min_rotspeed = other.debris_min_rotspeed;
	debris_max_rotspeed = other.debris_max_rotspeed;
	debris_damage_type_idx = other.debris_damage_type_idx;
	debris_min_hitpoints = other.debris_min_hitpoints;
	debris_max_hitpoints = other.debris_max_hitpoints;
	debris_damage_mult = other.debris_damage_mult;
	debris_arc_percent = other.debris_arc_percent;

	if ( other.n_subsystems > 0 ) {
		if( n_subsystems < 1 ) {
			subsystems = (model_subsystem *)vm_malloc(sizeof(model_subsystem) * other.n_subsystems );
		} else {
			for ( int i = 0; i < n_subsystems; i++ ) {
				for ( int j = 0; j < subsystems[i].n_triggers; j++ ) {
					if ( i < other.n_subsystems ) {
						subsystems[i].triggers = (queued_animation*)vm_realloc(subsystems[i].triggers, sizeof(queued_animation) * (other.subsystems[i].n_triggers));
					} else {
						vm_free(subsystems[i].triggers);
					}
				}
			}
			subsystems = (model_subsystem *)vm_realloc(subsystems, sizeof(model_subsystem) * other.n_subsystems);
		}
		Assert( subsystems != NULL );
		for ( int i = n_subsystems; i < other.n_subsystems; i++ ) {
			subsystems[i].triggers = (queued_animation*) vm_malloc(sizeof(queued_animation) * (other.subsystems[i].n_triggers));
		}

		for ( int i = 0; i < other.n_subsystems; i++ ) {
			queued_animation* triggers = subsystems[i].triggers;
			subsystems[i] = other.subsystems[i];
			subsystems[i].triggers = triggers;
			memcpy(subsystems[i].triggers, other.subsystems[i].triggers, sizeof(queued_animation) * (subsystems[i].n_triggers));
		}
	}
	n_subsystems = other.n_subsystems;

	power_output = other.power_output;
	max_overclocked_speed = other.max_overclocked_speed;
	max_weapon_reserve = other.max_weapon_reserve;
	max_shield_regen_per_second = other.max_shield_regen_per_second;
	max_weapon_regen_per_second = other.max_weapon_regen_per_second;

	afterburner_max_vel = other.afterburner_max_vel;
	afterburner_forward_accel = other.afterburner_forward_accel;
	afterburner_fuel_capacity = other.afterburner_fuel_capacity;
	afterburner_burn_rate = other.afterburner_burn_rate;
	afterburner_recover_rate = other.afterburner_recover_rate;
	afterburner_max_reverse_vel = other.afterburner_max_reverse_vel;
	afterburner_reverse_accel = other.afterburner_reverse_accel;

	cmeasure_type = other.cmeasure_type;
	cmeasure_max = other.cmeasure_max;

	num_primary_banks = other.num_primary_banks;
	memcpy(primary_bank_weapons, other.primary_bank_weapons, sizeof(int) * MAX_SHIP_PRIMARY_BANKS);
	memcpy(primary_bank_ammo_capacity, other.primary_bank_ammo_capacity, sizeof(int) * MAX_SHIP_PRIMARY_BANKS);

	num_secondary_banks = other.num_secondary_banks;
	memcpy(secondary_bank_weapons, other.secondary_bank_weapons, sizeof(int) * MAX_SHIP_SECONDARY_BANKS);
	memcpy(secondary_bank_ammo_capacity, other.secondary_bank_ammo_capacity, sizeof(int) * MAX_SHIP_SECONDARY_BANKS);

	memcpy(draw_primary_models, other.draw_primary_models, sizeof(bool) * MAX_SHIP_PRIMARY_BANKS);
	memcpy(draw_secondary_models, other.draw_secondary_models, sizeof(bool) * MAX_SHIP_SECONDARY_BANKS);
	weapon_model_draw_distance = other.weapon_model_draw_distance;

	max_hull_strength = other.max_hull_strength;
	max_shield_strength = other.max_shield_strength;
	max_shield_recharge = other.max_shield_recharge;
	auto_shield_spread = other.auto_shield_spread;
	auto_shield_spread_bypass = other.auto_shield_spread_bypass;
	auto_shield_spread_from_lod = other.auto_shield_spread_from_lod;
	auto_shield_spread_min_span = other.auto_shield_spread_min_span;

	// ...Hmm. A memcpy() seems slightly overkill here, but I've settled into the pattern of "array gets memcpy'd", so... -MageKing17
	memcpy(shield_point_augment_ctrls, other.shield_point_augment_ctrls, sizeof(int) * 4);

	hull_repair_rate = other.hull_repair_rate;
	subsys_repair_rate = other.subsys_repair_rate;

	sup_hull_repair_rate = other.sup_hull_repair_rate;
	sup_shield_repair_rate = other.sup_shield_repair_rate;
	sup_subsys_repair_rate = other.sup_subsys_repair_rate;

	closeup_pos = other.closeup_pos;
	closeup_zoom = other.closeup_zoom;

	memcpy(allowed_weapons, other.allowed_weapons, sizeof(int) * MAX_WEAPON_TYPES);

	memcpy(restricted_loadout_flag, other.restricted_loadout_flag, sizeof(int) * MAX_SHIP_WEAPONS);
	memcpy(allowed_bank_restricted_weapons, other.allowed_bank_restricted_weapons, sizeof(int) * MAX_SHIP_WEAPONS * MAX_WEAPON_TYPES);

	shield_icon_index = other.shield_icon_index;
	strcpy_s(icon_filename, other.icon_filename);
	model_icon_angles = other.model_icon_angles;
	strcpy_s(anim_filename, other.anim_filename);
	strcpy_s(overhead_filename, other.overhead_filename);
	selection_effect = other.selection_effect;

	bii_index_ship = other.bii_index_ship;
	bii_index_ship_with_cargo = other.bii_index_ship_with_cargo;
	bii_index_wing = other.bii_index_wing;
	bii_index_wing_with_cargo = other.bii_index_wing_with_cargo;

	score = other.score;

	scan_time = other.scan_time;

	memcpy(ct_info, other.ct_info, sizeof(trail_info) * MAX_SHIP_CONTRAILS);
	ct_count = other.ct_count;

	num_nondark_colors = other.num_nondark_colors;
	memcpy(nondark_colors, other.nondark_colors, sizeof(ubyte) * MAX_NONDARK_COLORS * 3);

	memcpy(shield_color, other.shield_color, sizeof(ubyte) * 3);

	uses_team_colors = other.uses_team_colors;
	default_team_name = other.default_team_name;

	afterburner_trail = other.afterburner_trail;
	afterburner_trail_width_factor = other.afterburner_trail_width_factor;
	afterburner_trail_alpha_factor = other.afterburner_trail_alpha_factor;
	afterburner_trail_life = other.afterburner_trail_life;
	afterburner_trail_faded_out_sections = other.afterburner_trail_faded_out_sections;

	normal_thruster_particles = other.normal_thruster_particles;
	afterburner_thruster_particles = other.afterburner_thruster_particles;

	memcpy(&thruster_flame_info, &other.thruster_flame_info, sizeof(thrust_pair));
	memcpy(&thruster_glow_info, &other.thruster_glow_info, sizeof(thrust_pair));
	memcpy(&thruster_secondary_glow_info, &other.thruster_secondary_glow_info, sizeof(thrust_pair_bitmap));
	memcpy(&thruster_tertiary_glow_info, &other.thruster_tertiary_glow_info, sizeof(thrust_pair_bitmap));
	memcpy(&thruster_distortion_info, &other.thruster_distortion_info, sizeof(thrust_pair_bitmap));

	thruster01_glow_rad_factor = other.thruster01_glow_rad_factor;
	thruster02_glow_rad_factor = other.thruster02_glow_rad_factor;
	thruster03_glow_rad_factor = other.thruster03_glow_rad_factor;
	thruster02_glow_len_factor = other.thruster02_glow_len_factor;
	thruster_dist_rad_factor = other.thruster_dist_rad_factor;
	thruster_dist_len_factor = other.thruster_dist_len_factor;
	thruster_glow_noise_mult = other.thruster_glow_noise_mult;

	draw_distortion = other.draw_distortion;

	splodeing_texture = other.splodeing_texture;
	strcpy_s(splodeing_texture_name, other.splodeing_texture_name);

	replacement_textures = other.replacement_textures;

	armor_type_idx = other.armor_type_idx;
	shield_armor_type_idx = other.shield_armor_type_idx;
	
	can_glide = other.can_glide;
	glide_cap = other.glide_cap;
	glide_dynamic_cap = other.glide_dynamic_cap;
	glide_accel_mult = other.glide_accel_mult;
	use_newtonian_damp = other.use_newtonian_damp;
	newtonian_damp_override = other.newtonian_damp_override;

	autoaim_fov = other.autoaim_fov;

	topdown_offset_def = other.topdown_offset_def;
	topdown_offset = other.topdown_offset;

	engine_snd = other.engine_snd;
	min_engine_vol = other.min_engine_vol;
	glide_start_snd = other.glide_start_snd;
	glide_end_snd = other.glide_end_snd;

	ship_sounds = other.ship_sounds;

	num_maneuvering = other.num_maneuvering;
	memcpy(maneuvering, other.maneuvering, sizeof(man_thruster) * MAX_MAN_THRUSTERS);

	radar_image_2d_idx = other.radar_image_2d_idx;
	radar_color_image_2d_idx = other.radar_color_image_2d_idx;
	radar_image_size = other.radar_image_size;
	radar_projection_size_mult = other.radar_projection_size_mult;

	memcpy(ship_iff_info, other.ship_iff_info, sizeof(int) * MAX_IFFS * MAX_IFFS);

	aiming_flags = other.aiming_flags;
	minimum_convergence_distance = other.minimum_convergence_distance;
	convergence_distance = other.convergence_distance;
	convergence_offset = other.convergence_offset;

	emp_resistance_mod = other.emp_resistance_mod;

	piercing_damage_draw_limit = other.piercing_damage_draw_limit;

	damage_lightning_type = other.damage_lightning_type;

	shield_impact_explosion_anim = other.shield_impact_explosion_anim;
	hud_gauges = other.hud_gauges;
	hud_enabled = other.hud_enabled;
	hud_retail = other.hud_retail;

	displays = other.displays;

	pathMetadata = other.pathMetadata;

	glowpoint_bank_override_map = other.glowpoint_bank_override_map;
}

void ship_info::move(ship_info&& other)
{
	std::swap(name, other.name);
	std::swap(alt_name, other.alt_name);
	std::swap(short_name, other.short_name);
	species = other.species;
	class_type = other.class_type;

	std::swap(type_str, other.type_str);
	std::swap(maneuverability_str, other.maneuverability_str);
	std::swap(armor_str, other.armor_str);
	std::swap(manufacturer_str, other.manufacturer_str);
	std::swap(desc, other.desc);
	std::swap(tech_desc, other.tech_desc);

	std::swap(tech_title, other.tech_title);

	std::swap(ship_length, other.ship_length);
	std::swap(gun_mounts, other.gun_mounts);
	std::swap(missile_banks, other.missile_banks);

	std::swap(cockpit_pof_file, other.cockpit_pof_file);
	std::swap(cockpit_offset, other.cockpit_offset);
	std::swap(pof_file, other.pof_file);
	std::swap(pof_file_hud, other.pof_file_hud);
	num_detail_levels = other.num_detail_levels;
	std::swap(detail_distance, other.detail_distance);
	collision_lod = other.collision_lod;

	cockpit_model_num = other.cockpit_model_num;
	model_num = other.model_num;
	model_num_hud = other.model_num_hud;

	hud_target_lod = other.hud_target_lod;
	density = other.density;
	damp = other.damp;
	rotdamp = other.rotdamp;
	delta_bank_const = other.delta_bank_const;
	std::swap(max_vel, other.max_vel);
	std::swap(max_rotvel, other.max_rotvel);
	std::swap(rotation_time, other.rotation_time);
	srotation_time = other.srotation_time;
	max_rear_vel = other.max_rear_vel;
	forward_accel = other.forward_accel;
	forward_decel = other.forward_decel;
	slide_accel = other.slide_accel;
	slide_decel = other.slide_decel;

	std::swap(warpin_anim, other.warpin_anim);
	warpin_radius = other.warpin_radius;
	warpin_snd_start = other.warpin_snd_start;
	warpin_snd_end = other.warpin_snd_end;
	warpin_speed = other.warpin_speed;
	warpin_time = other.warpin_time;
	warpin_decel_exp = other.warpin_decel_exp;
	warpin_type = other.warpin_type;

	std::swap(warpout_anim, other.warpout_anim);
	warpout_radius = other.warpout_radius;
	warpout_snd_start = other.warpout_snd_start;
	warpout_snd_end = other.warpout_snd_end;
	warpout_engage_time = other.warpout_engage_time;
	warpout_speed = other.warpout_speed;
	warpout_time = other.warpout_time;
	warpout_accel_exp = other.warpout_accel_exp;
	warpout_type = other.warpout_type;

	warpout_player_speed = other.warpout_player_speed;

	flags = other.flags;
	flags2 = other.flags2;
	ai_class = other.ai_class;
	max_speed = other.max_speed;
	min_speed = other.min_speed;
	max_accel = other.max_accel;

	collision_damage_type_idx = other.collision_damage_type_idx;
	std::swap(collision_physics, other.collision_physics);

	std::swap(shockwave, other.shockwave);
	explosion_propagates = other.explosion_propagates;
	big_exp_visual_rad = other.big_exp_visual_rad;
	prop_exp_rad_mult = other.prop_exp_rad_mult;
	death_roll_r_mult = other.death_roll_r_mult;
	death_fx_r_mult = other.death_fx_r_mult;
	death_roll_time_mult = other.death_roll_time_mult;
	death_roll_base_time = other.death_roll_base_time;
	death_fx_count = other.death_fx_count;
	shockwave_count = other.shockwave_count;
	std::swap(explosion_bitmap_anims, other.explosion_bitmap_anims);
	vaporize_chance = other.vaporize_chance;

	std::swap(impact_spew, other.impact_spew);
	std::swap(damage_spew, other.damage_spew);
	std::swap(split_particles, other.split_particles);
	std::swap(knossos_end_particles, other.knossos_end_particles);
	std::swap(regular_end_particles, other.regular_end_particles);

	debris_min_lifetime = other.debris_min_lifetime;
	debris_max_lifetime = other.debris_max_lifetime;
	debris_min_speed = other.debris_min_speed;
	debris_max_speed = other.debris_max_speed;
	debris_min_rotspeed = other.debris_min_rotspeed;
	debris_max_rotspeed = other.debris_max_rotspeed;
	debris_damage_type_idx = other.debris_damage_type_idx;
	debris_min_hitpoints = other.debris_min_hitpoints;
	debris_max_hitpoints = other.debris_max_hitpoints;
	debris_damage_mult = other.debris_damage_mult;
	debris_arc_percent = other.debris_arc_percent;

	std::swap(subsystems, other.subsystems);
	std::swap(n_subsystems, other.n_subsystems);

	power_output = other.power_output;
	max_overclocked_speed = other.max_overclocked_speed;
	max_weapon_reserve = other.max_weapon_reserve;
	max_shield_regen_per_second = other.max_shield_regen_per_second;
	max_weapon_regen_per_second = other.max_weapon_regen_per_second;

	std::swap(afterburner_max_vel, other.afterburner_max_vel);
	afterburner_forward_accel = other.afterburner_forward_accel;
	afterburner_fuel_capacity = other.afterburner_fuel_capacity;
	afterburner_burn_rate = other.afterburner_burn_rate;
	afterburner_recover_rate = other.afterburner_recover_rate;
	afterburner_max_reverse_vel = other.afterburner_max_reverse_vel;
	afterburner_reverse_accel = other.afterburner_reverse_accel;

	cmeasure_type = other.cmeasure_type;
	cmeasure_max = other.cmeasure_max;

	num_primary_banks = other.num_primary_banks;
	std::swap(primary_bank_weapons, other.primary_bank_weapons);
	std::swap(primary_bank_ammo_capacity, other.primary_bank_ammo_capacity);

	num_secondary_banks = other.num_secondary_banks;
	std::swap(secondary_bank_weapons, other.secondary_bank_weapons);
	std::swap(secondary_bank_ammo_capacity, other.secondary_bank_ammo_capacity);

	std::swap(draw_primary_models, other.draw_primary_models);
	std::swap(draw_secondary_models, other.draw_secondary_models);
	weapon_model_draw_distance = other.weapon_model_draw_distance;

	max_hull_strength = other.max_hull_strength;
	max_shield_strength = other.max_shield_strength;
	max_shield_recharge = other.max_shield_recharge;
	auto_shield_spread = other.auto_shield_spread;
	auto_shield_spread_bypass = other.auto_shield_spread_bypass;
	auto_shield_spread_from_lod = other.auto_shield_spread_from_lod;
	auto_shield_spread_min_span = other.auto_shield_spread_min_span;

	std::swap(shield_point_augment_ctrls, other.shield_point_augment_ctrls);

	hull_repair_rate = other.hull_repair_rate;
	subsys_repair_rate = other.subsys_repair_rate;

	sup_hull_repair_rate = other.sup_hull_repair_rate;
	sup_shield_repair_rate = other.sup_shield_repair_rate;
	sup_subsys_repair_rate = other.sup_subsys_repair_rate;

	std::swap(closeup_pos, other.closeup_pos);
	closeup_zoom = other.closeup_zoom;

	std::swap(allowed_weapons, other.allowed_weapons);

	std::swap(restricted_loadout_flag, other.restricted_loadout_flag);
	memcpy(allowed_bank_restricted_weapons, other.allowed_bank_restricted_weapons, sizeof(int) * MAX_SHIP_WEAPONS * MAX_WEAPON_TYPES);

	shield_icon_index = other.shield_icon_index;
	std::swap(icon_filename, other.icon_filename);
	model_icon_angles = other.model_icon_angles;
	std::swap(anim_filename, other.anim_filename);
	std::swap(overhead_filename, other.overhead_filename);
	selection_effect = other.selection_effect;

	bii_index_ship = other.bii_index_ship;
	bii_index_ship_with_cargo = other.bii_index_ship_with_cargo;
	bii_index_wing = other.bii_index_wing;
	bii_index_wing_with_cargo = other.bii_index_wing_with_cargo;

	score = other.score;

	scan_time = other.scan_time;

	std::swap(ct_info, other.ct_info);
	ct_count = other.ct_count;

	num_nondark_colors = other.num_nondark_colors;
	memcpy(nondark_colors, other.nondark_colors, sizeof(ubyte) * MAX_NONDARK_COLORS * 3);

	std::swap(shield_color, other.shield_color);

	uses_team_colors = other.uses_team_colors;
	std::swap(default_team_name, other.default_team_name);

	std::swap(afterburner_trail, other.afterburner_trail);
	afterburner_trail_width_factor = other.afterburner_trail_width_factor;
	afterburner_trail_alpha_factor = other.afterburner_trail_alpha_factor;
	afterburner_trail_life = other.afterburner_trail_life;
	afterburner_trail_faded_out_sections = other.afterburner_trail_faded_out_sections;

	std::swap(normal_thruster_particles, other.normal_thruster_particles);
	std::swap(afterburner_thruster_particles, other.afterburner_thruster_particles);

	std::swap(thruster_flame_info, other.thruster_flame_info);
	std::swap(thruster_glow_info, other.thruster_glow_info);
	std::swap(thruster_secondary_glow_info, other.thruster_secondary_glow_info);
	std::swap(thruster_tertiary_glow_info, other.thruster_tertiary_glow_info);
	std::swap(thruster_distortion_info, other.thruster_distortion_info);

	thruster01_glow_rad_factor = other.thruster01_glow_rad_factor;
	thruster02_glow_rad_factor = other.thruster02_glow_rad_factor;
	thruster03_glow_rad_factor = other.thruster03_glow_rad_factor;
	thruster02_glow_len_factor = other.thruster02_glow_len_factor;
	thruster_dist_rad_factor = other.thruster_dist_rad_factor;
	thruster_dist_len_factor = other.thruster_dist_len_factor;
	thruster_glow_noise_mult = other.thruster_glow_noise_mult;

	draw_distortion = other.draw_distortion;

	splodeing_texture = other.splodeing_texture;
	std::swap(splodeing_texture_name, other.splodeing_texture_name);

	std::swap(replacement_textures, other.replacement_textures);

	armor_type_idx = other.armor_type_idx;
	shield_armor_type_idx = other.shield_armor_type_idx;
	
	can_glide = other.can_glide;
	glide_cap = other.glide_cap;
	glide_dynamic_cap = other.glide_dynamic_cap;
	glide_accel_mult = other.glide_accel_mult;
	use_newtonian_damp = other.use_newtonian_damp;
	newtonian_damp_override = other.newtonian_damp_override;

	autoaim_fov = other.autoaim_fov;

	topdown_offset_def = other.topdown_offset_def;
	std::swap(topdown_offset, other.topdown_offset);

	engine_snd = other.engine_snd;
	min_engine_vol = other.min_engine_vol;
	glide_start_snd = other.glide_start_snd;
	glide_end_snd = other.glide_end_snd;

	std::swap(ship_sounds, other.ship_sounds);

	num_maneuvering = other.num_maneuvering;
	std::swap(maneuvering, other.maneuvering);

	radar_image_2d_idx = other.radar_image_2d_idx;
	radar_color_image_2d_idx = other.radar_color_image_2d_idx;
	radar_image_size = other.radar_image_size;
	radar_projection_size_mult = other.radar_projection_size_mult;

	memcpy(ship_iff_info, other.ship_iff_info, sizeof(int) * MAX_IFFS * MAX_IFFS);

	aiming_flags = other.aiming_flags;
	minimum_convergence_distance = other.minimum_convergence_distance;
	convergence_distance = other.convergence_distance;
	std::swap(convergence_offset, other.convergence_offset);

	emp_resistance_mod = other.emp_resistance_mod;

	piercing_damage_draw_limit = other.piercing_damage_draw_limit;

	damage_lightning_type = other.damage_lightning_type;

	shield_impact_explosion_anim = other.shield_impact_explosion_anim;
	std::swap(hud_gauges, other.hud_gauges);
	hud_enabled = other.hud_enabled;
	hud_retail = other.hud_retail;

	std::swap(displays, other.displays);

	std::swap(pathMetadata, other.pathMetadata);

	std::swap(glowpoint_bank_override_map, other.glowpoint_bank_override_map);
}

#define CHECK_THEN_FREE(attribute) \
do {\
	if (attribute != NULL) {\
		vm_free(attribute);\
		attribute = NULL;\
	}\
} while(false)

void ship_info::free_strings()
{
	CHECK_THEN_FREE(type_str);
	CHECK_THEN_FREE(maneuverability_str);
	CHECK_THEN_FREE(armor_str);
	CHECK_THEN_FREE(manufacturer_str);
	CHECK_THEN_FREE(desc);
	CHECK_THEN_FREE(tech_desc);
	CHECK_THEN_FREE(ship_length);
	CHECK_THEN_FREE(gun_mounts);
	CHECK_THEN_FREE(missile_banks);
}

ship_info &ship_info::operator= (ship_info&& other) NOEXCEPT
{
	if (this != &other) {
		move(std::move(other));
	}
	return *this;
}

ship_info::ship_info(ship_info&& other) NOEXCEPT
{
	// MageKing17 - Initialize these pointers to NULL because otherwise move() will leave them uninitialized.
	type_str = NULL;
	maneuverability_str = NULL;
	armor_str = NULL;
	manufacturer_str = NULL;
	desc = NULL;
	tech_desc = NULL;
	ship_length = NULL;
	gun_mounts = NULL;
	missile_banks = NULL;
	subsystems = NULL;
	n_subsystems = 0;

	// Then we swap everything (well, everything that matters to the deconstructor).
	move(std::move(other));
}

ship_info::ship_info()
{
	int i,j;

	name[0] = '\0';
	alt_name[0] = '\0';
	sprintf(short_name, "ShipClass%d", static_cast<int>(Ship_info.size()));
	species = 0;
	class_type = -1;

	type_str = maneuverability_str = armor_str = manufacturer_str = NULL;
	desc = tech_desc = NULL;
	tech_title[0] = 0;

	ship_length = NULL;
	gun_mounts = NULL;
	missile_banks = NULL;

	cockpit_pof_file[0] = '\0';
	vm_vec_zero(&cockpit_offset);
	pof_file[0] = '\0';
	pof_file_hud[0] = '\0';
	num_detail_levels = 1;
	detail_distance[0] = 0;
	collision_lod = -1;
	cockpit_model_num = -1;
	model_num = -1;
	model_num_hud = -1;
	hud_target_lod = -1;

	density = 1.0f;
	damp = 0.0f;
	rotdamp = 0.0f;
	delta_bank_const = DEFAULT_DELTA_BANK_CONST;
	vm_vec_zero(&max_vel);
	vm_vec_zero(&max_rotvel);
	vm_vec_zero(&rotation_time);
	srotation_time = 0.0f;
	max_rear_vel = 0.0f;
	forward_accel = 0.0f;
	forward_decel = 0.0f;
	slide_accel = 0.0f;
	slide_decel = 0.0f;

	warpin_anim[0] = '\0';
	warpin_radius = 0.0f;
	warpin_snd_start = -1;
	warpin_snd_end = -1;
	warpin_speed = 0.0f;
	warpin_time = 0;
	warpin_decel_exp = 1;
	warpin_type = WT_DEFAULT;

	warpout_anim[0] = '\0';
	warpout_radius = 0.0f;
	warpout_snd_start = -1;
	warpout_snd_end = -1;
	warpout_engage_time = -1;
	warpout_speed = 0.0f;
	warpout_time = 0;
	warpout_accel_exp = 1;
	warpout_type = WT_DEFAULT;

	warpout_player_speed = 0.0f;

	flags = SIF_DEFAULT_VALUE;
	flags2 = SIF2_DEFAULT_VALUE;
	ai_class = 0;
	max_speed = 0.0f;
	min_speed = 0.0f;
	max_accel = 0.0f;

	collision_damage_type_idx = -1;
	// Retail default collision physics and default landing parameters
	memset(&collision_physics, 0, sizeof(ship_collision_physics));
	collision_physics.both_small_bounce = 5.0;
	collision_physics.bounce = 5.0;
	collision_physics.friction = COLLISION_FRICTION_FACTOR;
	collision_physics.rotation_factor = COLLISION_ROTATION_FACTOR;
	collision_physics.reorient_mult = 1.0f;
	collision_physics.landing_sound_idx = -1;

	shockwave_create_info_init(&shockwave);
	explosion_propagates = 0;
	big_exp_visual_rad = -1.0f;
	prop_exp_rad_mult = 1.0f;
	death_roll_r_mult = 1.0f;
	death_fx_r_mult = 1.0f;
	death_roll_time_mult = 1.0f;
	death_roll_base_time = 3000;
	death_fx_count = 6;
	shockwave_count = 1;
	explosion_bitmap_anims.clear();
	vaporize_chance = 0;

	// default values from shipfx.cpp
	impact_spew.n_high = 30;
	impact_spew.n_low = 25;
	impact_spew.max_rad = 0.5f;
	impact_spew.min_rad = 0.2f;
	impact_spew.max_life = 0.55f;
	impact_spew.min_life = 0.05f;
	impact_spew.max_vel = 12.0f;
	impact_spew.min_vel = 2.0f;
	impact_spew.variance = 1.0f;

	// default values from shipfx.cpp
	damage_spew.n_high = 1;						// 1 is used here to trigger retail behaviour
	damage_spew.n_low = 0;
	damage_spew.max_rad = 1.3f;
	damage_spew.min_rad = 0.7f;
	damage_spew.max_life = 0.0f;
	damage_spew.min_life = 0.0f;
	damage_spew.max_vel = 12.0f;
	damage_spew.min_vel = 3.0f;
	damage_spew.variance = 0.0f;

	split_particles.n_high = 80;
	split_particles.n_low = 40;
	split_particles.max_rad = 0.0f;
	split_particles.min_rad = 0.0f;
	split_particles.max_life = 0.0f;
	split_particles.min_life = 0.0f;
	split_particles.max_vel = 0.0f;
	split_particles.min_vel = 0.0f;
	split_particles.variance = 2.0f;

	knossos_end_particles.n_high = 30;
	knossos_end_particles.n_low = 15;
	knossos_end_particles.max_rad = 100.0f;
	knossos_end_particles.min_rad = 30.0f;
	knossos_end_particles.max_life = 12.0f;
	knossos_end_particles.min_life = 2.0f;
	knossos_end_particles.max_vel = 350.0f;
	knossos_end_particles.min_vel = 50.0f;
	knossos_end_particles.variance = 2.0f;

	regular_end_particles.n_high = 100;
	regular_end_particles.n_low = 50;
	regular_end_particles.max_rad = 1.5f;
	regular_end_particles.min_rad = 0.1f;
	regular_end_particles.max_life = 4.0f;
	regular_end_particles.min_life = 0.5f;
	regular_end_particles.max_vel = 20.0f;
	regular_end_particles.min_vel = 0.0f;
	regular_end_particles.variance = 2.0f;

	debris_min_lifetime = -1.0f;
	debris_max_lifetime = -1.0f;
	debris_min_speed = -1.0f;
	debris_max_speed = -1.0f;
	debris_min_rotspeed = -1.0f;
	debris_max_rotspeed = -1.0f;
	debris_damage_type_idx = -1;
	debris_min_hitpoints = -1.0f;
	debris_max_hitpoints = -1.0f;
	debris_damage_mult = 1.0f;
	debris_arc_percent = 0.5f;

	n_subsystems = 0;
	subsystems = NULL;

	power_output = 0.0f;
	max_overclocked_speed = 0.0f;
	max_weapon_reserve = 0.0f;
	max_shield_regen_per_second = 0.0f;
	max_weapon_regen_per_second = 0.0f;

	vm_vec_zero(&afterburner_max_vel);
	afterburner_forward_accel = 0.0f;
	afterburner_fuel_capacity = 0.0f;
	afterburner_burn_rate = 0.0f;
	afterburner_recover_rate = 0.0f;
	afterburner_max_reverse_vel = 0.0f;
	afterburner_reverse_accel = 0.0f;

	cmeasure_type = Default_cmeasure_index;
	cmeasure_max = 0;

	num_primary_banks = 0;
	for ( i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++ )
	{
		primary_bank_weapons[i] = -1;
		draw_primary_models[i] = false;
		primary_bank_ammo_capacity[i] = 0;
	}
	
	num_secondary_banks = 0;
	for ( i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++ )
	{
		secondary_bank_weapons[i] = -1;
		draw_secondary_models[i] = false;
		secondary_bank_ammo_capacity[i] = 0;
	}

	weapon_model_draw_distance = 200.0f;

	ship_recoil_modifier = 1.0f;

	max_hull_strength = 100.0f;
	max_shield_strength = 0.0f;

	max_shield_recharge = 1.0f;

	auto_shield_spread = 0.0f;
	auto_shield_spread_bypass = false;
	auto_shield_spread_from_lod = -1;
	auto_shield_spread_min_span = -1.0f;

	for (i = 0; i < 4; i++)
	{
		shield_point_augment_ctrls[i] = -1;
	}

	hull_repair_rate = 0.0f;
	//-2 represents not set, in which case the default is used for the ship (if it is small)
	subsys_repair_rate = -2.0f;

	sup_hull_repair_rate = 0.15f;
	sup_shield_repair_rate = 0.20f;
	sup_subsys_repair_rate = 0.15f;

	vm_vec_zero(&closeup_pos);
	closeup_zoom = 0.5f;

	memset(allowed_weapons, 0, sizeof(int) * MAX_WEAPON_TYPES);

	memset(restricted_loadout_flag, 0, sizeof(int) * MAX_SHIP_WEAPONS);
	memset(allowed_bank_restricted_weapons, 0, sizeof(int) * MAX_SHIP_WEAPONS * MAX_WEAPON_TYPES);

	shield_icon_index = 255;		// stored as ubyte
	icon_filename[0] = '\0';
	memset(&model_icon_angles, 0, sizeof(angles));
	anim_filename[0] = '\0';
	overhead_filename[0] = '\0';

	selection_effect = Default_ship_select_effect;

	bii_index_ship = -1;
	bii_index_ship_with_cargo = -1;
	bii_index_wing = -1;
	bii_index_wing_with_cargo = -1;

	score = 0;

	scan_time = 2000;

	memset(&ct_info, 0, sizeof(trail_info) * MAX_SHIP_CONTRAILS);
	ct_count = 0;

	num_nondark_colors = 0;
	memset(nondark_colors, 0, sizeof(ubyte) * MAX_NONDARK_COLORS * 3);

	shield_color[0] = 255;
	shield_color[1] = 255;
	shield_color[2] = 255;

	// Team colors
	uses_team_colors = false;
	default_team_name = "";

	generic_bitmap_init(&afterburner_trail, NULL);
	afterburner_trail_width_factor = 1.0f;
	afterburner_trail_alpha_factor = 1.0f;
	afterburner_trail_life = 5.0f;
	afterburner_trail_faded_out_sections = 0;

	normal_thruster_particles.clear();
	afterburner_thruster_particles.clear();

	// Bobboau's thruster stuff
	generic_anim_init( &thruster_flame_info.normal );
	generic_anim_init( &thruster_flame_info.afterburn );
	generic_anim_init( &thruster_glow_info.normal );
	generic_anim_init( &thruster_glow_info.afterburn );
	generic_bitmap_init( &thruster_secondary_glow_info.normal );
	generic_bitmap_init( &thruster_secondary_glow_info.afterburn );
	generic_bitmap_init( &thruster_tertiary_glow_info.normal );
	generic_bitmap_init( &thruster_tertiary_glow_info.afterburn );
	generic_bitmap_init( &thruster_distortion_info.normal );
	generic_bitmap_init( &thruster_distortion_info.afterburn );

	// Bobboau's thruster stuff
	thruster01_glow_rad_factor = 1.0f;
	thruster02_glow_rad_factor = 1.0f;
	thruster03_glow_rad_factor = 1.0f;
	thruster02_glow_len_factor = 1.0f;
	thruster_dist_rad_factor = 2.0f;
	thruster_dist_len_factor = 2.0f;
	thruster_glow_noise_mult = 1.0f;

	draw_distortion = true;

	splodeing_texture = -1;
	strcpy_s(splodeing_texture_name, "boom");

	replacement_textures.clear();

	armor_type_idx = -1;
	shield_armor_type_idx = -1;

	can_glide = false;
	glide_cap = 0.0f;
	glide_dynamic_cap = false;
	glide_accel_mult = 0.0f;
	use_newtonian_damp = false;
	newtonian_damp_override = false;

	autoaim_fov = 0.0f;

	topdown_offset_def = false;
	vm_vec_zero(&topdown_offset);

	engine_snd = -1;
	min_engine_vol = -1.0f;
	glide_start_snd = -1;
	glide_end_snd = -1;

	ship_sounds.clear();

	num_maneuvering = 0;
	memset(maneuvering, 0, MAX_MAN_THRUSTERS * sizeof(man_thruster));
	for (i = 0; i < MAX_MAN_THRUSTERS; i++)
	{
		maneuvering[i].start_snd = -1;
		maneuvering[i].loop_snd = -1;
		maneuvering[i].stop_snd = -1;
		maneuvering[i].tex_id = -1;
	}

	radar_image_2d_idx = -1;
	radar_color_image_2d_idx = -1;
	radar_image_size = -1;
	radar_projection_size_mult = 1.0f;

	for (i=0;i<MAX_IFFS;i++)
	{
		for (j=0;j<MAX_IFFS;j++)
			ship_iff_info[i][j] = -1;
	}

	aiming_flags = 0;
	minimum_convergence_distance = 0.0f;
	convergence_distance = 100.0f;
	vm_vec_zero(&convergence_offset);

	emp_resistance_mod = 0.0f;

	piercing_damage_draw_limit = 0.10f;

	damage_lightning_type = SLT_DEFAULT;

	shield_impact_explosion_anim = -1;
	hud_gauges.clear();
	hud_enabled = false;
	hud_retail = false;

	displays.clear();

	pathMetadata.clear();

	glowpoint_bank_override_map.clear();
}

ship_info::~ship_info()
{
	if ( subsystems != NULL ) {
		for(int n = 0; n < n_subsystems; n++) {
			if (subsystems[n].triggers != NULL) {
				vm_free(subsystems[n].triggers);
				subsystems[n].triggers = NULL;
			}
		}

		vm_free(subsystems);
		subsystems = NULL;
	}

	free_strings();
}

/**
 * Parse the information for a specific ship type.
 */
int parse_ship(const char *filename, bool replace)
{
	char buf[SHIP_MULTITEXT_LENGTH];
	ship_info *sip;
	bool create_if_not_found  = true;
	int rtn = 0;

	required_string("$Name:");
	stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);

	if(optional_string("+nocreate")) {
		if(!replace) {
			Warning(LOCATION, "+nocreate flag used for ship in non-modular table");
		}
		create_if_not_found = false;
	}

#ifdef NDEBUG
	if (get_pointer_to_first_hash_symbol(buf) && Fred_running)
		rtn = 1;
#endif

	//Remove @ symbol
	//these used to be used to denote weapons that would
	//only be parsed in demo builds
	if ( buf[0] == '@' ) {
		backspace(buf);
	}

	diag_printf ("Ship name -- %s\n", buf);
	//Check if ship exists already
	int ship_id;
	bool first_time = false;
	ship_id = ship_info_lookup_sub( buf );
	
	if(ship_id != -1)
	{
		sip = &Ship_info[ship_id];
		if(!replace)
		{
			Warning(LOCATION, "Error:  Ship name %s already exists in %s.  All ship class names must be unique.", sip->name, filename);
			if ( !skip_to_start_of_string_either("$Name:", "#End")) {
				Int3();
			}
			return -1;
		}
	}
	else
	{
		//Don't create ship if it has +nocreate and is in a modular table.
		if(!create_if_not_found && replace)
		{
			if ( !skip_to_start_of_string_either("$Name:", "#End")) {
				Int3();
			}

			return -1;
		}
		
		//Check if there are too many ship classes
		if(Ship_info.size() >= MAX_SHIP_CLASSES) {
			Error(LOCATION, "Too many ship classes before '%s'; maximum is %d.\n", buf, MAX_SHIP_CLASSES);
		}

		//Init vars
		Ship_info.push_back(ship_info());
		sip = &Ship_info.back();
		first_time = true;

		strcpy_s(sip->name, buf);
	}

	// Use a template for this ship.
	if( optional_string( "+Use Template:" ) ) {
		if ( !create_if_not_found ) {
			Warning(LOCATION, "Both '+nocreate' and '+Use Template:' were specified for ship class '%s', ignoring '+Use Template:'\n", buf);
		}
		else {
			char template_name[SHIP_MULTITEXT_LENGTH];
			stuff_string(template_name, F_NAME, SHIP_MULTITEXT_LENGTH);
			int template_id = ship_template_lookup(template_name);
			if ( template_id != -1 ) {
				first_time = false;
				sip->clone(Ship_templates[template_id]);
				strcpy_s(sip->name, buf);
			}
			else {
				Warning(LOCATION, "Unable to find ship template '%s' requested by ship class '%s', ignoring template request...", template_name, buf);
			}
		}
	}

	rtn = parse_ship_values(sip, false, first_time, replace);

	return rtn;	//0 for success
}

/**
 * Parse the information for a specific ship type template.
 */
int parse_ship_template()
{
	char buf[SHIP_MULTITEXT_LENGTH];
	ship_info *sip;
	int rtn = 0;
	bool first_time = false;

	required_string("$Template:");
	stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
	
	if( optional_string("+nocreate") ) {
		Warning(LOCATION, "+nocreate flag used on ship template. Ship templates can not be modified. Ignoring +nocreate.");
	}
	
	diag_printf ("Ship template name -- %s\n", buf);
	//Check if the template exists already
	int template_id;
	template_id = ship_template_lookup( buf );
	
	if( template_id != -1 ) {
		sip = &Ship_templates[template_id];
		Warning(LOCATION, "WARNING: Ship template %s already exists. All ship template names must be unique.", sip->name);
		if ( !skip_to_start_of_string_either("$Template:", "#End")) {
			error_display(1, "Missing [#End] or [$Template] after duplicate entry for %s", sip->name);
		}
		return -1;
	}
	else {
		
		Ship_templates.push_back(ship_info());
		sip = &Ship_templates.back();
		
		strcpy_s(sip->name, buf);
		//Use another template for this template. This allows for template hierarchies. - Turey
		if( optional_string("+Use Template:") ) {
			char template_name[SHIP_MULTITEXT_LENGTH];
			stuff_string(template_name, F_NAME, SHIP_MULTITEXT_LENGTH);
			template_id = ship_template_lookup( template_name);
			
			if ( template_id != -1 ) {
				first_time = false;
				sip->clone(Ship_templates[template_id]);
				strcpy_s(sip->name, buf);
			}
			else {
				Warning(LOCATION, "Unable to find ship template '%s' requested by ship template '%s', ignoring template request...", template_name, buf);
			}
		}
	}
	
	rtn = parse_ship_values( sip, true, first_time, false );
	
	return rtn;
}

void parse_ship_sound(char *name, GameSoundsIndex id, ship_info *sip)
{
	Assert( name != NULL );

	int temp_index = -1;

	parse_sound(name, &temp_index, sip->name);

	if (temp_index >= 0)
		sip->ship_sounds.insert(std::pair<GameSoundsIndex, int>(id, temp_index));
}

void parse_ship_sounds(ship_info *sip)
{
	parse_ship_sound("$CockpitEngineSnd:",                SND_ENGINE, sip);
	parse_ship_sound("$FullThrottleSnd:",                 SND_FULL_THROTTLE, sip);
	parse_ship_sound("$ZeroThrottleSnd:",                 SND_ZERO_THROTTLE, sip);
	parse_ship_sound("$ThrottleUpSnd:",                   SND_THROTTLE_UP, sip);
	parse_ship_sound("$ThrottleDownSnd:",                 SND_THROTTLE_DOWN, sip);
	parse_ship_sound("$AfterburnerSnd:",                  SND_ABURN_LOOP, sip);
	parse_ship_sound("$AfterburnerEngageSnd:",            SND_ABURN_ENGAGE, sip);
	parse_ship_sound("$AfterburnerFailedSnd:",            SND_ABURN_FAIL, sip);
	parse_ship_sound("$MissileTrackingSnd:",              SND_MISSILE_TRACKING, sip);
	parse_ship_sound("$MissileLockedSnd:",                SND_MISSILE_LOCK, sip);
	parse_ship_sound("$PrimaryCycleSnd:",                 SND_PRIMARY_CYCLE, sip);
	parse_ship_sound("$SecondaryCycleSnd:",               SND_SECONDARY_CYCLE, sip);
	parse_ship_sound("$TargetAcquiredSnd:",               SND_TARGET_ACQUIRE, sip);
	parse_ship_sound("$PrimaryFireFailedSnd:",            SND_OUT_OF_WEAPON_ENERGY, sip);
	parse_ship_sound("$SecondaryFireFailedSnd:",          SND_OUT_OF_MISSLES, sip);
	parse_ship_sound("$HeatSeekerLaunchWarningSnd:",      SND_HEATLOCK_WARN, sip);
	parse_ship_sound("$AspectSeekerLaunchWarningSnd:",    SND_ASPECTLOCK_WARN, sip);
	parse_ship_sound("$MissileLockWarningSnd:",           SND_THREAT_FLASH, sip);
	parse_ship_sound("$HeatSeekerProximityWarningSnd:",   SND_PROXIMITY_WARNING, sip);
	parse_ship_sound("$AspectSeekerProximityWarningSnd:", SND_PROXIMITY_ASPECT_WARNING, sip);
	parse_ship_sound("$MissileEvadedSnd:",                SND_MISSILE_EVADED_POPUP, sip);
	parse_ship_sound("$CargoScanningSnd:",                SND_CARGO_SCAN, sip);

	// Use SND_SHIP_EXPLODE_1 for custom explosion sounds
	parse_ship_sound("$ExplosionSnd:",                    SND_SHIP_EXPLODE_1, sip);
} 

void parse_ship_particle_effect(ship_info* sip, particle_effect* pe, char *id_string)
{
	float tempf;
	int temp;
	if(optional_string("+Max particles:"))
	{
		stuff_int(&temp);
		if (temp < 0) {
			Warning(LOCATION,"Bad value %i, defined as %s particle number (max) in ship '%s'.\nValue should be a non-negative integer.\n", temp, id_string, sip->name);
		} else {
			pe->n_high = temp;
			if (pe->n_high == 0) {
				// notification for disabling the particles
				mprintf(("Particle effect for %s disabled on ship '%s'.\n", id_string, sip->name));
			}
		}
	}
	if(optional_string("+Min particles:"))
	{
		stuff_int(&temp);
		if (temp < 0) {
			Warning(LOCATION,"Bad value %i, defined as %s particle number (min) in ship '%s'.\nValue should be a non-negative integer.\n", temp, id_string, sip->name);
		} else {
			pe->n_low = temp;
		}
	}
	if (pe->n_low > pe->n_high)
		pe->n_low = pe->n_high;

	if(optional_string("+Max Radius:"))
	{
		stuff_float(&tempf);
		if (tempf <= 0.0f) {
			Warning(LOCATION,"Bad value %f, defined as %s particle radius (max) in ship '%s'.\nValue should be a positive float.\n", tempf, id_string, sip->name);
		} else {
			pe->max_rad = tempf;
		}
	}
	if(optional_string("+Min Radius:"))
	{
		stuff_float(&tempf);
		if (tempf < 0.0f) {
			Warning(LOCATION,"Bad value %f, defined as %s particle radius (min) in ship '%s'.\nValue should be a non-negative float.\n", tempf, id_string, sip->name);
		} else {
			pe->min_rad = tempf;
		}
	}
	if (pe->min_rad > pe->max_rad)
		pe->min_rad = pe->max_rad;

	if(optional_string("+Max Lifetime:"))
	{
		stuff_float(&tempf);
		if (tempf <= 0.0f) {
			Warning(LOCATION,"Bad value %f, defined as %s particle lifetime (max) in ship '%s'.\nValue should be a positive float.\n", tempf, id_string, sip->name);
		} else {
			pe->max_life = tempf;
		}
	}
	if(optional_string("+Min Lifetime:"))
	{
		stuff_float(&tempf);
		if (tempf < 0.0f) {
			Warning(LOCATION,"Bad value %f, defined as %s particle lifetime (min) in ship '%s'.\nValue should be a non-negative float.\n", tempf, id_string, sip->name);
		} else {
			pe->min_life = tempf;
		}
	}
	if (pe->min_life > pe->max_life)
		pe->min_life = pe->max_life;

	if(optional_string("+Max Velocity:"))
	{
		stuff_float(&tempf);
		if (tempf < 0.0f) {
			Warning(LOCATION,"Bad value %f, defined as %s particle velocity (max) in ship '%s'.\nValue should be a non-negative float.\n", tempf, id_string, sip->name);
		} else {
			pe->max_vel = tempf;
		}
	}
	if(optional_string("+Min Velocity:"))
	{
		stuff_float(&tempf);
		if (tempf < 0.0f) {
			Warning(LOCATION,"Bad value %f, defined as %s particle velocity (min) in ship '%s'.\nValue should be a non-negative float.\n", tempf, id_string, sip->name);
		} else {
			pe->min_vel = tempf;
		}
	}
	if (pe->min_vel > pe->max_vel)
		pe->min_vel = pe->max_vel;

	if(optional_string("+Normal Variance:"))
	{
		stuff_float(&tempf);
		if ((tempf >= 0.0f) && (tempf <= 2.0f)) {
			pe->variance = tempf;
		} else {
			Warning(LOCATION,"Bad value %f, defined as %s particle normal variance in ship '%s'.\nValue should be a float from 0.0 to 2.0.\n", tempf, id_string, sip->name);
		}
	}
}

void parse_allowed_weapons(ship_info *sip, bool is_primary, bool is_dogfight)
{
	int i, num_allowed;
	int allowed_weapons[MAX_WEAPON_TYPES];
	const int max_banks = (is_primary ? MAX_SHIP_PRIMARY_BANKS : MAX_SHIP_SECONDARY_BANKS);
	const int weapon_type = (is_dogfight ? DOGFIGHT_WEAPON : REGULAR_WEAPON);
	const int offset = (is_primary ? 0 : MAX_SHIP_PRIMARY_BANKS);
	const char *allowed_banks_str = is_primary ? (is_dogfight ? "$Allowed Dogfight PBanks:" : "$Allowed PBanks:")
		: (is_dogfight ? "$Allowed Dogfight SBanks:" : "$Allowed SBanks:");
	const char *bank_type_str = is_primary ? "primary" : "secondary";

	// Goober5000 - fixed Bobboau's implementation of restricted banks
	int bank;

	// Set the weapons filter used in weapons loadout (for primary weapons)
	if (optional_string(allowed_banks_str))
	{
		bank = -1;

		while (check_for_string("("))
		{
			bank++;

			// make sure we don't specify more than we have banks for
			if (bank >= max_banks)
			{
				Warning(LOCATION, "%s bank-specific loadout for %s exceeds permissible number of %s banks.  Ignoring the rest...", allowed_banks_str, sip->name, bank_type_str);
				bank--;
				break;
			}

			num_allowed = stuff_int_list(allowed_weapons, MAX_WEAPON_TYPES, WEAPON_LIST_TYPE);

			// actually say which weapons are allowed
			for ( i = 0; i < num_allowed; i++ )
			{
				if ( allowed_weapons[i] >= 0 )		// MK, Bug fix, 9/6/99.  Used to be "allowed_weapons" not "allowed_weapons[i]".
				{
					sip->allowed_bank_restricted_weapons[offset+bank][allowed_weapons[i]] |= weapon_type;
				}
			}
		}

		// set flags if need be
		if (bank > 0)	// meaning there was a restricted bank table entry
		{
			for (i=0; i<=bank; i++)
			{
				sip->restricted_loadout_flag[offset+i] |= weapon_type;
			}
		}
	}
}

/**
 * Common method for parsing ship/subsystem primary/secondary weapons so that the parser doesn't flip out in the event of a problem.
 *
 */
void parse_weapon_bank(ship_info *sip, bool is_primary, int *num_banks, int *bank_default_weapons, int *bank_capacities)
{
	Assert(sip != NULL);
	Assert(bank_default_weapons != NULL);
	Assert(bank_capacities != NULL);
	const int max_banks = is_primary ? MAX_SHIP_PRIMARY_BANKS : MAX_SHIP_SECONDARY_BANKS;
	const char *default_banks_str = is_primary ? "$Default PBanks:" : "$Default SBanks:";
	const char *bank_capacities_str = is_primary ? "$PBank Capacity:" : "$SBank Capacity:";

	// we initialize to the previous parse, which presumably worked
	int num_bank_capacities = num_banks != NULL ? *num_banks : 0;

	if (optional_string(default_banks_str))
	{
		// get weapon list
		if (num_banks != NULL)
			*num_banks = stuff_int_list(bank_default_weapons, max_banks, WEAPON_LIST_TYPE);
		else
			stuff_int_list(bank_default_weapons, max_banks, WEAPON_LIST_TYPE);
	}

	if (optional_string(bank_capacities_str))
	{
		// get capacity list
		num_bank_capacities = stuff_int_list(bank_capacities, max_banks, RAW_INTEGER_TYPE);
	}

	// num_banks can be null if we're parsing weapons for a turret
	if ((num_banks != NULL) && (*num_banks != num_bank_capacities))
	{
		// okay for a ship to have 0 primary capacities, since it won't be ammo-enabled
		if (is_primary && num_bank_capacities != 0)
		{
			Warning(LOCATION, "Ship class '%s' has %d primary banks, but %d primary capacities... fix this!!", sip->name, *num_banks, num_bank_capacities);
		}

		// secondaries have no excuse!
		if (!is_primary)
		{
			Warning(LOCATION, "Ship class '%s' has %d secondary banks, but %d secondary capacities... fix this!!", sip->name, *num_banks, num_bank_capacities);
		}
	}
}

/**
 * Common method for parsing briefing icon info.
 */
int parse_and_add_briefing_icon_info()
{
	int bii_index = -1;
	size_t icon;
	char regular_temp[MAX_FILENAME_LEN];
	char fade_temp[MAX_FILENAME_LEN];
	char highlight_temp[MAX_FILENAME_LEN];

	required_string("+Regular:");
	stuff_string(regular_temp, F_NAME, MAX_FILENAME_LEN);
	required_string("+Fade:");
	stuff_string(fade_temp, F_NAME, MAX_FILENAME_LEN);
	required_string("+Highlight:");
	stuff_string(highlight_temp, F_NAME, MAX_FILENAME_LEN);

	// search among our existing icons
	for (icon = 0; icon < Briefing_icon_info.size(); icon++)
	{
		if (   !stricmp(regular_temp, Briefing_icon_info[icon].regular.filename)
			&& !stricmp(fade_temp, Briefing_icon_info[icon].fade.filename)
			&& !stricmp(highlight_temp, Briefing_icon_info[icon].highlight.filename) )
		{
			bii_index = (int) icon;
			break;
		}
	}

	// icon not found: create new one
	if (bii_index < 0)
	{
		briefing_icon_info bii;
		generic_anim_init(&bii.regular, regular_temp);
		hud_anim_init(&bii.fade, 0, 0, fade_temp);
		hud_anim_init(&bii.highlight, 0, 0, highlight_temp);

		bii_index = (int) Briefing_icon_info.size();
		Briefing_icon_info.push_back(bii);			
	}

	return bii_index;
}

/**
 * Puts values into a ship_info.
 */
int parse_ship_values(ship_info* sip, const bool is_template, const bool first_time, const bool replace)
{
	char buf[SHIP_MULTITEXT_LENGTH];
	char* info_type_name;
	char* type_name;
	int i, j;
	int rtn = 0;
	char name_tmp[NAME_LENGTH];

	if ( ! is_template ) {
		info_type_name = "Ship Class";
		type_name = "$Name";
	}
	else {
		info_type_name = "Ship Template";
		type_name = "$Template";
	}
	
	if(optional_string("$Alt name:"))
		stuff_string(sip->alt_name, F_NAME, NAME_LENGTH);

	if(optional_string("$Short name:"))
		stuff_string(sip->short_name, F_NAME, NAME_LENGTH);
	else if (first_time)
	{
		char *srcpos, *srcend, *destpos;
		srcpos = sip->name;
		destpos = sip->short_name;
		srcend = srcpos + strlen(sip->name);
		while(srcpos <= srcend)
		{
			if(*srcpos != ' ')
				*destpos++ = *srcpos++;
			else
				srcpos++;
		}
	}
	diag_printf ("Ship short name -- %s\n", sip->short_name);

	if (optional_string("$Species:")) {
		char temp[NAME_LENGTH];
		stuff_string(temp, F_NAME, NAME_LENGTH);
		int i_species = 0;
		
		bool found = false;
		for (SCP_vector<species_info>::iterator sii = Species_info.begin(); sii != Species_info.end(); ++sii, ++i_species) {
			if (!stricmp(temp, sii->species_name)) {
				sip->species = i_species;
				found = true;
				break;
			}
		}

		if (!found) {
			Error(LOCATION, "Invalid Species %s defined in table entry for ship %s.\n", temp, sip->name);
		}
	}

	diag_printf ("Ship species -- %s\n", Species_info[sip->species].species_name);

	if (optional_string("+Type:")) {
		stuff_malloc_string(&sip->type_str, F_MESSAGE);
	}

	if (optional_string("+Maneuverability:")) {
		stuff_malloc_string(&sip->maneuverability_str, F_MESSAGE);
	}

	if (optional_string("+Armor:")) {
		stuff_malloc_string(&sip->armor_str, F_MESSAGE);
	}

	if (optional_string("+Manufacturer:")) {
		stuff_malloc_string(&sip->manufacturer_str, F_MESSAGE);
	}

	if (optional_string("+Description:")) {
		stuff_malloc_string(&sip->desc, F_MULTITEXT, NULL);
	}
	
	if (optional_string("+Tech Title:")) {
		stuff_string(sip->tech_title, F_NAME, NAME_LENGTH);
	}

	if (optional_string("+Tech Description:")) {
		stuff_malloc_string(&sip->tech_desc, F_MULTITEXT, NULL);
	}

	if (optional_string("+Length:")) {
		stuff_malloc_string(&sip->ship_length, F_MESSAGE);
	}
	
	if (optional_string("+Gun Mounts:")) {
		stuff_malloc_string(&sip->gun_mounts, F_MESSAGE);
	}
	
	if (optional_string("+Missile Banks:")) {
		stuff_malloc_string(&sip->missile_banks, F_MESSAGE);
	}

	// Ship fadein effect, used when no ani is specified or ship_select_3d is active
	sip->selection_effect = Default_ship_select_effect; //By default, use the FS2 effect
	if(optional_string("$Selection Effect:")) {
		char effect[NAME_LENGTH];
		stuff_string(effect, F_NAME, NAME_LENGTH);
		if (!stricmp(effect, "FS2"))
			sip->selection_effect = 2;
		else if (!stricmp(effect, "FS1"))
			sip->selection_effect = 1;
		else if (!stricmp(effect, "off"))
			sip->selection_effect = 0;
	}

	if(optional_string( "$Cockpit POF file:" ))
	{
		char temp[MAX_FILENAME_LEN];
		stuff_string(temp, F_NAME, MAX_FILENAME_LEN);

		// assume we're using this file name
		bool valid = true;

		// Goober5000 - if this is a modular table, and we're replacing an existing file name, and the file doesn't exist, don't replace it
		if (replace)
			if (sip->cockpit_pof_file[0] != '\0')
				if (!cf_exists_full(temp, CF_TYPE_MODELS))
					valid = false;

		if (valid)
			strcpy_s(sip->cockpit_pof_file, temp);
		else
			WarningEx(LOCATION, "Ship %s\nCockpit POF file \"%s\" invalid!", sip->name, temp);
	}
	if(optional_string( "+Cockpit offset:" ))
	{
		stuff_vec3d(&sip->cockpit_offset);
	}
	while(optional_string( "$Cockpit Display:" )) 
	{
		cockpit_display_info display;

		display.bg_filename[0] = 0;
		display.fg_filename[0] = 0;
		display.filename[0] = 0;
		display.name[0] = 0;
		display.offset[0] = 0;
		display.offset[1] = 0;

		required_string("+Texture:");
		stuff_string(display.filename, F_NAME, MAX_FILENAME_LEN);

		if ( optional_string("+Offsets:") ) {
			stuff_int_list(display.offset, 2);
		}
		
		required_string("+Size:");
		stuff_int_list(display.size, 2);
		
		if ( optional_string("+Background:") ) {
			stuff_string(display.bg_filename, F_NAME, MAX_FILENAME_LEN);
		}
		if ( optional_string("+Foreground:") ) {
			stuff_string(display.fg_filename, F_NAME, MAX_FILENAME_LEN);
		}
		
		required_string("+Display Name:");
		stuff_string(display.name, F_NAME, MAX_FILENAME_LEN);

		if ( display.offset[0] < 0 || display.offset[1] < 0 ) {
			Warning(LOCATION, "Negative display offsets given for cockpit display on %s, skipping entry", sip->name);
			continue;
		}

		if( display.size[0] <= 0 || display.size[1] <= 0 ) {
			Warning(LOCATION, "Negative or zero display size given for cockpit display on %s, skipping entry", sip->name);
			continue;
		}

		sip->displays.push_back(display);
	}

	if(optional_string( "$POF file:" ))
	{
		char temp[MAX_FILENAME_LEN];
		stuff_string(temp, F_NAME, MAX_FILENAME_LEN);

		// assume we're using this file name
		bool valid = true;

		// Goober5000 - if this is a modular table, and we're replacing an existing file name, and the file doesn't exist, don't replace it
		if (replace)
			if (sip->pof_file[0] != '\0')
				if (!cf_exists_full(temp, CF_TYPE_MODELS))
					valid = false;

		if (valid)
			strcpy_s(sip->pof_file, temp);
		else
			WarningEx(LOCATION, "Ship %s\nPOF file \"%s\" invalid!", sip->name, temp);
	}

	// ship class texture replacement - Goober5000
	// don't clear the vector because we could be parsing a TBM
	if (optional_string("$Texture Replace:"))
	{
		texture_replace tr;
		char *p;

		while (optional_string("+old:"))
		{
			strcpy_s(tr.ship_name, sip->name);
			tr.new_texture_id = -1;

			stuff_string(tr.old_texture, F_NAME, MAX_FILENAME_LEN);
			required_string("+new:");
			stuff_string(tr.new_texture, F_NAME, MAX_FILENAME_LEN);

			// get rid of extensions
			p = strchr(tr.old_texture, '.');
			if (p)
			{
				mprintf(("Extraneous extension found on replacement texture %s!\n", tr.old_texture));
				*p = 0;
			}
			p = strchr(tr.new_texture, '.');
			if (p)
			{
				mprintf(("Extraneous extension found on replacement texture %s!\n", tr.new_texture));
				*p = 0;
			}

			// add it if we aren't over the limit
			if (sip->replacement_textures.size() < MAX_MODEL_TEXTURES)
				sip->replacement_textures.push_back(tr);
			else
				mprintf(("Too many replacement textures specified for ship '%s'!\n", sip->name));
		}
	}

	// optional hud targeting model
	if(optional_string( "$POF target file:"))
	{
		char temp[MAX_FILENAME_LEN];
		stuff_string(temp, F_NAME, MAX_FILENAME_LEN);

		// assume we're using this file name
		bool valid = true;

		// Goober5000 - if this is a modular table, and we're replacing an existing file name, and the file doesn't exist, don't replace it
		if (replace)
			if (sip->pof_file_hud[0] != '\0')
				if (!cf_exists_full(temp, CF_TYPE_MODELS))
					valid = false;

		if (valid)
			strcpy_s(sip->pof_file_hud, temp);
		else
			WarningEx(LOCATION, "Ship \"%s\" POF target file \"%s\" invalid!", sip->name, temp);
	}

	// optional hud target LOD if not using special hud model
	if (optional_string( "$POF target LOD:" )) {
		stuff_int(&sip->hud_target_lod);
	}

	if(optional_string("$Detail distance:")) {
		sip->num_detail_levels = stuff_int_list(sip->detail_distance, MAX_SHIP_DETAIL_LEVELS, RAW_INTEGER_TYPE);
	}

	if(optional_string("$Collision LOD:")) {
		stuff_int(&sip->collision_lod);

		// Cap to sane values, just in case
		sip->collision_lod = MAX(0, MIN(sip->collision_lod, sip->num_detail_levels));
	}

	// check for optional pixel colors
	while(optional_string("$ND:")){		
		ubyte nr, ng, nb;
		stuff_ubyte(&nr);
		stuff_ubyte(&ng);
		stuff_ubyte(&nb);

		if(sip->num_nondark_colors < MAX_NONDARK_COLORS){
			sip->nondark_colors[sip->num_nondark_colors][0] = nr;
			sip->nondark_colors[sip->num_nondark_colors][1] = ng;
			sip->nondark_colors[sip->num_nondark_colors++][2] = nb;
		}
	}

	if (optional_string("$Enable Team Colors:")) {
		stuff_boolean(&sip->uses_team_colors);
		sip->default_team_name = "None";
	}

	if (optional_string("$Default Team:")) {
		char temp[NAME_LENGTH];
		stuff_string(temp, F_NAME, NAME_LENGTH);
		SCP_string name = temp;
		if (!stricmp(temp, "none")) {
			sip->uses_team_colors = true;
		} else {
			if (Team_Colors.find(name) != Team_Colors.end()) {
				sip->default_team_name = name;
				sip->uses_team_colors = true;
			} else {
				Warning(LOCATION, "Team name %s is invalid. Teams must be defined in colors.tbl.\n", temp);
			}
		}
	}

	if(optional_string("$Show damage:"))
	{
		int bogus_bool;
		stuff_boolean(&bogus_bool);
	}

	if(optional_string("$Damage Lightning Type:"))
	{
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		j = lightningtype_match(buf);
		if(j >= 0) {
			sip->damage_lightning_type = j;
		} else {
			Warning(LOCATION, "Invalid lightning type '%s' specified for ship '%s'", buf, sip->name);
			sip->damage_lightning_type = SLT_DEFAULT;
		}
	}

	if(optional_string("$Impact:"))
	{
		if(optional_string("+Damage Type:"))
		{
			stuff_string(buf, F_NAME, NAME_LENGTH);
			sip->collision_damage_type_idx = damage_type_add(buf);
		}
	}

	//HACK -
	//This should really be reworked so that all particle fields
	//are settable, but erg, just not happening right now -C
	if(optional_string("$Impact Spew:"))
	{
		parse_ship_particle_effect(sip, &sip->impact_spew, "impact spew");
	}
	if(optional_string("$Damage Spew:"))
	{
		parse_ship_particle_effect(sip, &sip->damage_spew, "damage spew");
	}

	if(optional_string("$Collision Physics:"))
	{
		if(optional_string("+Bounce:"))	{
			stuff_float(&sip->collision_physics.bounce);
		}
		if(optional_string("+Both Small Bounce:")) {
			stuff_float(&sip->collision_physics.both_small_bounce);
		}
		if(optional_string("+Friction:")) {
			stuff_float(&sip->collision_physics.friction);
		}
		if(optional_string("+Rotation Factor:")) {
			stuff_float(&sip->collision_physics.friction);
		}
		if(optional_string("+Landing Max Forward Vel:")) {
			stuff_float(&sip->collision_physics.landing_max_z);
		}
		if(optional_string("+Landing Min Forward Vel:")) {
			stuff_float(&sip->collision_physics.landing_min_z);
		}
		if(optional_string("+Landing Max Descent Vel:")) {
			stuff_float(&sip->collision_physics.landing_min_y);
			sip->collision_physics.landing_min_y *= -1;
		}
		if(optional_string("+Landing Max Horizontal Vel:")) {
			stuff_float(&sip->collision_physics.landing_max_x);
		}
		if(optional_string("+Landing Max Angle:")) {
			float degrees;
			stuff_float(&degrees);
			sip->collision_physics.landing_max_angle = cos(ANG_TO_RAD(90 - degrees));
		}
		if(optional_string("+Landing Min Angle:")) {
			float degrees;
			stuff_float(&degrees);
			sip->collision_physics.landing_min_angle = cos(ANG_TO_RAD(90 - degrees));
		}
		if(optional_string("+Landing Max Rotate Angle:")) {
			float degrees;
			stuff_float(&degrees);
			sip->collision_physics.landing_max_rot_angle = cos(ANG_TO_RAD(90 - degrees));
		}
		if(optional_string("+Reorient Max Forward Vel:")) {
			stuff_float(&sip->collision_physics.reorient_max_z);
		}
		if(optional_string("+Reorient Min Forward Vel:")) {
			stuff_float(&sip->collision_physics.reorient_min_z);
		}
		if(optional_string("+Reorient Max Descent Vel:")) {
			stuff_float(&sip->collision_physics.reorient_min_y);
			sip->collision_physics.reorient_min_y *= -1;
		}
		if(optional_string("+Reorient Max Horizontal Vel:")) {
			stuff_float(&sip->collision_physics.reorient_max_x);
		}
		if(optional_string("+Reorient Max Angle:")) {
			float degrees;
			stuff_float(&degrees);
			sip->collision_physics.reorient_max_angle = cos(ANG_TO_RAD(90 - degrees));
		}
		if(optional_string("+Reorient Min Angle:")) {
			float degrees;
			stuff_float(&degrees);
			sip->collision_physics.reorient_min_angle = cos(ANG_TO_RAD(90 - degrees));
		}
		if(optional_string("+Reorient Max Rotate Angle:")) {
			float degrees;
			stuff_float(&degrees);
			sip->collision_physics.reorient_max_rot_angle = cos(ANG_TO_RAD(90 - degrees));
		}
		if(optional_string("+Reorient Speed Mult:")) {
			stuff_float(&sip->collision_physics.reorient_mult);
		}
		if(optional_string("+Landing Rest Angle:")) {
			float degrees;
			stuff_float(&degrees);
			sip->collision_physics.landing_rest_angle = cos(ANG_TO_RAD(90 - degrees));
		}
		parse_sound("+Landing Sound:", &sip->collision_physics.landing_sound_idx, sip->name);
	}


	if(optional_string("$Debris:"))
	{
		if(optional_string("+Min Lifetime:"))	{
			stuff_float(&sip->debris_min_lifetime);
			if(sip->debris_min_lifetime < 0.0f)
				Warning(LOCATION, "Debris min lifetime on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Max Lifetime:"))	{
			stuff_float(&sip->debris_max_lifetime);
			if(sip->debris_max_lifetime < 0.0f)
				Warning(LOCATION, "Debris max lifetime on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Min Speed:"))	{
			stuff_float(&sip->debris_min_speed);
			if(sip->debris_min_speed < 0.0f)
				Warning(LOCATION, "Debris min speed on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Max Speed:"))	{
			stuff_float(&sip->debris_max_speed);
			if(sip->debris_max_speed < 0.0f)
				Warning(LOCATION, "Debris max speed on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Min Rotation speed:"))	{
			stuff_float(&sip->debris_min_rotspeed);
			if(sip->debris_min_rotspeed < 0.0f)
				Warning(LOCATION, "Debris min speed on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Max Rotation speed:"))	{
			stuff_float(&sip->debris_max_rotspeed);
			if(sip->debris_max_rotspeed < 0.0f)
				Warning(LOCATION, "Debris max speed on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Damage Type:")) {
			stuff_string(buf, F_NAME, NAME_LENGTH);
			sip->debris_damage_type_idx = damage_type_add(buf);
		}
		if(optional_string("+Min Hitpoints:")) {
			stuff_float(&sip->debris_min_hitpoints);
			if(sip->debris_min_hitpoints < 0.0f)
				Warning(LOCATION, "Debris min hitpoints on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Max Hitpoints:")) {
			stuff_float(&sip->debris_max_hitpoints);
			if(sip->debris_max_hitpoints < 0.0f)
				Warning(LOCATION, "Debris max hitpoints on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Damage Multiplier:")) {
			stuff_float(&sip->debris_damage_mult);
			if(sip->debris_damage_mult < 0.0f)
				Warning(LOCATION, "Debris damage multiplier on %s '%s' is below 0 and will be ignored", info_type_name, sip->name);
		}
		if(optional_string("+Lightning Arc Percent:")) {
			stuff_float(&sip->debris_arc_percent);
			if(sip->debris_arc_percent < 0.0f || sip->debris_arc_percent > 100.0f) {
				Warning(LOCATION, "Lightning Arc Percent on %s '%s' should be between 0 and 100.0 (read %f). Entry will be ignored.", info_type_name, sip->name, sip->debris_arc_percent);
				sip->debris_arc_percent = 50.0;
			}
			//Percent is nice for modders, but here in the code we want it betwwen 0 and 1.0
			sip->debris_arc_percent /= 100.0;
		}
		
	}
	//WMC - sanity checking
	if(sip->debris_min_speed > sip->debris_max_speed && sip->debris_max_speed >= 0.0f) {
		Warning(LOCATION, "Debris min speed (%f) on %s '%s' is greater than debris max speed (%f), and will be set to debris max speed.", sip->debris_min_speed, info_type_name, sip->name, sip->debris_max_speed);
		sip->debris_min_speed = sip->debris_max_speed;
	}
	if(sip->debris_min_rotspeed > sip->debris_max_rotspeed && sip->debris_max_rotspeed >= 0.0f) {
		Warning(LOCATION, "Debris min rotation speed (%f) on %s '%s' is greater than debris max rotation speed (%f), and will be set to debris max rotation speed.", sip->debris_min_rotspeed, info_type_name, sip->name, sip->debris_max_rotspeed);
		sip->debris_min_rotspeed = sip->debris_max_rotspeed;
	}
	if(sip->debris_min_lifetime > sip->debris_max_lifetime && sip->debris_max_lifetime >= 0.0f) {
		Warning(LOCATION, "Debris min lifetime (%f) on %s '%s' is greater than debris max lifetime (%f), and will be set to debris max lifetime.", sip->debris_min_lifetime, info_type_name, sip->name, sip->debris_max_lifetime);
		sip->debris_min_lifetime = sip->debris_max_lifetime;
	}
	if(sip->debris_min_hitpoints > sip->debris_max_hitpoints && sip->debris_max_hitpoints >= 0.0f) {
		Warning(LOCATION, "Debris min hitpoints (%f) on %s '%s' is greater than debris max hitpoints (%f), and will be set to debris max hitpoints.", sip->debris_min_hitpoints, info_type_name, sip->name, sip->debris_max_hitpoints);
		sip->debris_min_hitpoints = sip->debris_max_hitpoints;
	}

	if(optional_string("$Density:"))
		stuff_float( &(sip->density) );
	diag_printf ("Ship density -- %7.3f\n", sip->density);

	if(optional_string("$Damp:"))
		stuff_float( &(sip->damp) );
	diag_printf ("Ship damp -- %7.3f\n", sip->damp);

	if(optional_string("$Rotdamp:"))
		stuff_float( &(sip->rotdamp) );
	diag_printf ("Ship rotdamp -- %7.3f\n", sip->rotdamp);

	if(optional_string("$Banking Constant:"))
		stuff_float( &(sip->delta_bank_const) );
	diag_printf ("%s '%s' delta_bank_const -- %7.3f\n", info_type_name, sip->name, sip->delta_bank_const);

	if(optional_string("$Max Velocity:"))
	{
		stuff_vec3d(&sip->max_vel);
		sip->max_accel = sip->max_vel.xyz.z;
	}

	// calculate the max speed from max_velocity
	sip->max_speed = sip->max_vel.xyz.z;

	if(optional_string("$Rotation Time:"))
	{
		stuff_vec3d(&sip->rotation_time);

		// div/0 safety check.
		if ((sip->rotation_time.xyz.x == 0) || (sip->rotation_time.xyz.y == 0) || (sip->rotation_time.xyz.z == 0))
			Warning(LOCATION, "Rotation time must have non-zero values in each of the three variables.\nFix this in %s %s\n", info_type_name, sip->name);

		sip->srotation_time = (sip->rotation_time.xyz.x + sip->rotation_time.xyz.y)/2.0f;

		sip->max_rotvel.xyz.x = (2 * PI) / sip->rotation_time.xyz.x;
		sip->max_rotvel.xyz.y = (2 * PI) / sip->rotation_time.xyz.y;
		sip->max_rotvel.xyz.z = (2 * PI) / sip->rotation_time.xyz.z;
	}

	// get the backwards velocity;
	if(optional_string("$Rear Velocity:"))
	{
		stuff_float(&sip->max_rear_vel);
		sip->min_speed = -sip->max_rear_vel;
	}

	// get the accelerations
	if(optional_string("$Forward accel:"))
		stuff_float(&sip->forward_accel );

	if(optional_string("$Forward decel:"))
		stuff_float(&sip->forward_decel );

	if(optional_string("$Slide accel:"))
		stuff_float(&sip->slide_accel );

	if(optional_string("$Slide decel:"))
		stuff_float(&sip->slide_decel );
		
	if(optional_string("$Glide:"))
	{
		stuff_boolean(&sip->can_glide);
	}

	if(sip->can_glide == true)
	{
		if(optional_string("+Dynamic Glide Cap:"))
			stuff_boolean(&sip->glide_dynamic_cap);
		if(optional_string("+Max Glide Speed:"))
			stuff_float(&sip->glide_cap );
		if(optional_string("+Glide Accel Mult:"))
			stuff_float(&sip->glide_accel_mult);
	}

	if(optional_string("$Use Newtonian Dampening:")) {
			sip->newtonian_damp_override = true;
			stuff_boolean(&sip->use_newtonian_damp);
	}

	if(optional_string("$Autoaim FOV:"))
	{
		float fov_temp;
		stuff_float(&fov_temp);

		// Make sure it is a reasonable value
		if (fov_temp < 0.0f)
			fov_temp = 0.0f;

		if (fov_temp > 180.0f)
			fov_temp = 180.0f;

		sip->aiming_flags |= AIM_FLAG_AUTOAIM;
		sip->autoaim_fov = fov_temp * PI / 180.0f;

		if(optional_string("+Converging Autoaim"))
			sip->aiming_flags |= AIM_FLAG_AUTOAIM_CONVERGENCE;

		if(optional_string("+Minimum Distance:"))
			stuff_float(&sip->minimum_convergence_distance);
	}

	if(optional_string("$Convergence:"))
	{
		if(optional_string("+Automatic"))
		{
			sip->aiming_flags |= AIM_FLAG_AUTO_CONVERGENCE;
			if(optional_string("+Minimum Distance:"))
				stuff_float(&sip->minimum_convergence_distance);
		}
		if(optional_string("+Standard"))
		{
			sip->aiming_flags |= AIM_FLAG_STD_CONVERGENCE;
			if(required_string("+Distance:"))
				stuff_float(&sip->convergence_distance);
		}
		if(optional_string("+Offset:")) {
			stuff_vec3d(&sip->convergence_offset);

			if (IS_VEC_NULL(&sip->convergence_offset))
				sip->aiming_flags &= ~AIM_FLAG_CONVERGENCE_OFFSET;
			else
				sip->aiming_flags |= AIM_FLAG_CONVERGENCE_OFFSET;				
		}
	}

	if(optional_string("$Warpin type:"))
	{
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		j = warptype_match(buf);
		if(j >= 0) {
			sip->warpin_type = j;
		} else {
			Warning(LOCATION, "Invalid warpin type '%s' specified for %s '%s'", buf, info_type_name, sip->name);
			sip->warpin_type = WT_DEFAULT;
		}
	}

	parse_sound("$Warpin Start Sound:", &sip->warpin_snd_start, sip->name);
	parse_sound("$Warpin End Sound:", &sip->warpin_snd_end, sip->name);

	if(optional_string("$Warpin speed:"))
	{
		stuff_float(&sip->warpin_speed);
	}

	if(optional_string("$Warpin time:"))
	{
		float t_time;
		stuff_float(&t_time);
		sip->warpin_time = fl2i(t_time*1000.0f);
		if(sip->warpin_time <= 0) {
			Warning(LOCATION, "Warp-in time specified as 0 or less on %s '%s'; value ignored", info_type_name, sip->name);
		}
	}

	if(optional_string("$Warpin decel exp:"))
	{
		stuff_float(&sip->warpin_decel_exp);
		if (sip->warpin_decel_exp < 0.0f) {
			Warning(LOCATION, "Warp-in deceleration exponent specified as less than 0 on %s '%s'; value ignored", info_type_name, sip->name);
			sip->warpin_decel_exp = 1.0f;
		}
	}

	if(optional_string("$Warpin radius:"))
	{
		stuff_float(&sip->warpin_radius);
		if(sip->warpin_radius <= 0.0f) {
			Warning(LOCATION, "Warp-in radius specified as 0 or less on %s '%s'; value ignored", info_type_name, sip->name);
		}
	}

	if(optional_string("$Warpin animation:"))
	{
		stuff_string(sip->warpin_anim, F_NAME, MAX_FILENAME_LEN);
	}

	if(optional_string("$Warpout type:"))
	{
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		j = warptype_match(buf);
		if(j >= 0) {
			sip->warpout_type = j;
		} else {
			Warning(LOCATION, "Invalid warpout type '%s' specified for %s '%s'", buf, info_type_name, sip->name);
			sip->warpout_type = WT_DEFAULT;
		}
	}

	parse_sound("$Warpout Start Sound:", &sip->warpout_snd_start, sip->name);
	parse_sound("$Warpout End Sound:", &sip->warpout_snd_end, sip->name);

	if(optional_string("$Warpout engage time:"))
	{
		float t_time;
		stuff_float(&t_time);
		if (t_time >= 0)
			sip->warpout_engage_time = fl2i(t_time*1000.0f);
		else
			Warning(LOCATION, "Warp-out engage time specified as 0 or less on %s '%s'; value ignored", info_type_name, sip->name);
	} else {
		sip->warpout_engage_time = -1;
	}

	if(optional_string("$Warpout speed:"))
	{
		stuff_float(&sip->warpout_speed);
	}

	if(optional_string("$Warpout time:"))
	{
		float t_time;
		stuff_float(&t_time);
		sip->warpout_time = fl2i(t_time*1000.0f);
		if(sip->warpout_time <= 0) {
			Warning(LOCATION, "Warp-out time specified as 0 or less on %s '%s'; value ignored", info_type_name, sip->name);
		}
	}

	if(optional_string("$Warpout accel exp:"))
	{
		stuff_float(&sip->warpout_accel_exp);
		if (sip->warpout_accel_exp < 0.0f) {
			Warning(LOCATION, "Warp-out acceleration exponent specified as less than 0 on %s '%s'; value ignored", info_type_name, sip->name);
			sip->warpout_accel_exp = 1.0f;
		}
	}

	if(optional_string("$Warpout radius:"))
	{
		stuff_float(&sip->warpout_radius);
		if(sip->warpout_radius <= 0.0f) {
			Warning(LOCATION, "Warp-out radius specified as 0 or less on %s '%s'; value ignored", info_type_name, sip->name);
		}
	}

	if(optional_string("$Warpout animation:"))
	{
		stuff_string(sip->warpout_anim, F_NAME, MAX_FILENAME_LEN);
	}


	if(optional_string("$Player warpout speed:"))
	{
		stuff_float(&sip->warpout_player_speed);
		if(sip->warpout_player_speed == 0.0f) {
			Warning(LOCATION, "Player warp-out speed cannot be 0; value ignored.");
		}
	}

	// get ship explosion info
	shockwave_create_info *sci = &sip->shockwave;
	if(optional_string("$Expl inner rad:")){
		stuff_float(&sci->inner_rad);
	}

	if(optional_string("$Expl outer rad:")){
		stuff_float(&sci->outer_rad);
	}

	if(optional_string("$Expl damage:")){
		stuff_float(&sci->damage);
	}

	if(optional_string("$Expl blast:")){
		stuff_float(&sci->blast);
	}

	if(optional_string("$Expl Propagates:")){
		stuff_boolean(&sip->explosion_propagates);
	}

	if(optional_string("$Propagating Expl Radius Multiplier:")){
		stuff_float(&sip->prop_exp_rad_mult);
		if(sip->prop_exp_rad_mult <= 0) {
			// on invalid value return to default setting
			Warning(LOCATION, "Propagating explosion radius multiplier was set to non-positive value.\nDefaulting multiplier to 1.0 on %s '%s'.\n", info_type_name, sip->name);
			sip->prop_exp_rad_mult = 1.0f;
		}
	}

	if(optional_string("$Expl Visual Rad:")){
		stuff_float(&sip->big_exp_visual_rad);
	}

	if(optional_string("$Base Death-Roll Time:")){
		stuff_int(&sip->death_roll_base_time);
		if (sip->death_roll_base_time < 2)
			sip->death_roll_base_time = 2;
	}

	if(optional_string("$Death-Roll Explosion Radius Mult:")){
		stuff_float(&sip->death_roll_r_mult);
		if (sip->death_roll_r_mult < 0)
			sip->death_roll_r_mult = 0;
	}

	if(optional_string("$Death-Roll Explosion Intensity Mult:")){
		stuff_float(&sip->death_roll_time_mult);
		if (sip->death_roll_time_mult <= 0)
			sip->death_roll_time_mult = 1.0f;
	}

	if(optional_string("$Death FX Explosion Radius Mult:")){
		stuff_float(&sip->death_fx_r_mult);
		if (sip->death_fx_r_mult < 0)
			sip->death_fx_r_mult = 0;
	}

	if(optional_string("$Death FX Explosion Count:")){
		stuff_int(&sip->death_fx_count);
		if (sip->death_fx_count < 0)
			sip->death_fx_count = 0;
	}

	if(optional_string("$Ship Splitting Particles:"))
	{
		parse_ship_particle_effect(sip, &sip->split_particles, "ship split spew");
	}

	if(optional_string("$Ship Death Particles:"))
	{
		parse_ship_particle_effect(sip, &sip->regular_end_particles, "normal death spew");
	}

	if(optional_string("$Alternate Death Particles:"))
	{
		parse_ship_particle_effect(sip, &sip->knossos_end_particles, "knossos death spew");
	}

	if(optional_string("$Vaporize Percent Chance:")){
		stuff_float(&sip->vaporize_chance);
		if (sip->vaporize_chance < 0.0f || sip->vaporize_chance > 100.0f) {
			sip->vaporize_chance = 0.0f;
			Warning(LOCATION, "$Vaporize Percent Chance should be between 0 and 100.0 (read %f). Setting to 0.", sip->vaporize_chance);
		}
		//Percent is nice for modders, but here in the code we want it betwwen 0 and 1.0
		sip->vaporize_chance /= 100.0;
	}

	if(optional_string("$Shockwave Damage Type:")) {
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		sci->damage_type_idx_sav = damage_type_add(buf);
		sci->damage_type_idx = sci->damage_type_idx_sav;
	}

	if(optional_string("$Shockwave Speed:")){
		stuff_float( &sci->speed );
	}

	if(optional_string("$Shockwave Count:")){
		stuff_int(&sip->shockwave_count);
	}

	if(optional_string("$Shockwave model:")){
		stuff_string( sci->pof_name, F_NAME, MAX_FILENAME_LEN);
	}
	
	if(optional_string("$Shockwave name:")) {
		stuff_string( sci->name, F_NAME, NAME_LENGTH);
	}

	if(optional_string("$Explosion Animations:")){
		int temp[MAX_FIREBALL_TYPES];
		int parsed_ints = stuff_int_list(temp, MAX_FIREBALL_TYPES, RAW_INTEGER_TYPE);
		sip->explosion_bitmap_anims.clear();
		sip->explosion_bitmap_anims.insert(sip->explosion_bitmap_anims.begin(), temp, temp+parsed_ints);
	}

	if (optional_string("$Weapon Model Draw Distance:")) {
		stuff_float( &sip->weapon_model_draw_distance );
	}

	// Set the weapons filter used in weapons loadout (for primary weapons)
	parse_allowed_weapons(sip, true, false);
	parse_allowed_weapons(sip, true, true);

	// Get primary bank weapons
	parse_weapon_bank(sip, true, &sip->num_primary_banks, sip->primary_bank_weapons, sip->primary_bank_ammo_capacity);

	if(optional_string("$Show Primary Models:"))
	{
		sip->flags2 |= SIF2_DRAW_WEAPON_MODELS;
		stuff_bool_list(sip->draw_primary_models, sip->num_primary_banks);
	}

	// Set the weapons filter used in weapons loadout (for secondary weapons)
	parse_allowed_weapons(sip, false, false);
	parse_allowed_weapons(sip, false, true);

	// Get secondary bank weapons
	parse_weapon_bank(sip, false, &sip->num_secondary_banks, sip->secondary_bank_weapons, sip->secondary_bank_ammo_capacity);
    
	if(optional_string("$Show Secondary Models:"))
	{
		sip->flags2 |= SIF2_DRAW_WEAPON_MODELS;
		stuff_bool_list(sip->draw_secondary_models, sip->num_secondary_banks);
	}

	if (optional_string("$Ship Recoil Modifier:")){
		stuff_float(&sip->ship_recoil_modifier);
	}

	if(optional_string("$Shields:")) {
		stuff_float(&sip->max_shield_strength);

		if(optional_string("+Auto Spread:")) {
			stuff_float(&sip->auto_shield_spread);
		}
		if(optional_string("+Minimum Weapon Span:")) {
			stuff_float(&sip->auto_shield_spread_min_span);
		}
		if(optional_string("+Allow Bypass:")) {
			stuff_boolean(&sip->auto_shield_spread_bypass);
		}
		if(optional_string("+Spread From LOD:")) {
			int temp;
			stuff_int(&temp);

			if (temp > sip->num_detail_levels)
				Warning(LOCATION, "+Spread From LOD for %s was %i whereas ship only has %i detail levels, ignoring...", sip->name, temp, sip->num_detail_levels);
			else
				sip->auto_shield_spread_from_lod = temp;
		}
	}

	if(optional_string("$Model Point Shield Controls:")) {
		SCP_vector<SCP_string> ctrl_strings;
		int num_strings = stuff_string_list(ctrl_strings);

		// Init all to -1 in case some aren't supplied...
		sip->shield_point_augment_ctrls[FRONT_QUAD] = -1;
		sip->shield_point_augment_ctrls[REAR_QUAD] = -1;
		sip->shield_point_augment_ctrls[LEFT_QUAD] = -1;
		sip->shield_point_augment_ctrls[RIGHT_QUAD] = -1;

		for (i = 0; i < num_strings; i++) {
			const char *str = ctrl_strings[i].c_str();

			if (!stricmp(str, "front"))
				sip->shield_point_augment_ctrls[FRONT_QUAD] = i;
			else if (!stricmp(str, "rear"))
				sip->shield_point_augment_ctrls[REAR_QUAD] = i;
			else if (!stricmp(str, "left"))
				sip->shield_point_augment_ctrls[LEFT_QUAD] = i;
			else if (!stricmp(str, "right"))
				sip->shield_point_augment_ctrls[RIGHT_QUAD] = i;
			else if (!stricmp(str, "none"))
				;
			else
				Warning(LOCATION, "Unrecognized value \"%s\" passed to $Model Point Shield Controls, ignoring...", str);
		}
	}

	// optional shield color
	if(optional_string("$Shield Color:")){
		stuff_ubyte(&sip->shield_color[0]);
		stuff_ubyte(&sip->shield_color[1]);
		stuff_ubyte(&sip->shield_color[2]);
	}

	if(optional_string("$Shield Impact Explosion:")) {
		char fname[MAX_NAME_LEN];
		stuff_string(fname, F_NAME, NAME_LENGTH);

		if ( VALID_FNAME(fname) )
			sip->shield_impact_explosion_anim = Weapon_explosions.Load(fname);
	}

	if(optional_string("$Max Shield Recharge:")){
		stuff_float(&sip->max_shield_recharge);
		CLAMP(sip->max_shield_recharge, 0.0f, 1.0f);
	}

	// The next five fields are used for the ETS
	if (optional_string("$Power Output:"))
		stuff_float(&sip->power_output);

	// Goober5000
	if (optional_string("$Shield Regeneration Rate:"))
		stuff_float(&sip->max_shield_regen_per_second);
	else if (first_time)
		sip->max_shield_regen_per_second = 0.02f;

	// Support ship hull shield rate - if allowed
	if(optional_string("$Support Shield Repair Rate:"))
	{
		stuff_float(&sip->sup_shield_repair_rate);
		sip->sup_shield_repair_rate *= 0.01f;
		CLAMP(sip->sup_shield_repair_rate, 0.0f, 1.0f);
	}

	// Goober5000
	if (optional_string("$Weapon Regeneration Rate:"))
		stuff_float(&sip->max_weapon_regen_per_second);
	else if (first_time)
		sip->max_weapon_regen_per_second = 0.04f;

	if (optional_string("$Max Oclk Speed:") || optional_string("$Max Overclock Speed:"))
		stuff_float(&sip->max_overclocked_speed);
	else if (first_time)
		sip->max_overclocked_speed = sip->max_vel.xyz.z * 1.5f;

	if (optional_string("$Max Weapon Eng:") || optional_string("$Max Weapon Energy:"))
		stuff_float(&sip->max_weapon_reserve);

	if(optional_string("$Hitpoints:"))
	{
		stuff_float(&sip->max_hull_strength);
		if (sip->max_hull_strength < 0.0f)
		{
			Warning(LOCATION, "Max hull strength on %s '%s' cannot be less than 0.  Defaulting to 100.\n", info_type_name, sip->name);
			sip->max_hull_strength = 100.0f;
		}
	}

	//Hull rep rate
	
	if(optional_string("$Hull Repair Rate:"))
	{
		stuff_float(&sip->hull_repair_rate);
		sip->hull_repair_rate *= 0.01f;
		
		//Sanity checking
		if(sip->hull_repair_rate > 1.0f)
			sip->hull_repair_rate = 1.0f;
		else if(sip->hull_repair_rate < -1.0f)
			sip->hull_repair_rate = -1.0f;
	}

	// Support ship hull repair rate - if allowed
	if(optional_string("$Support Hull Repair Rate:"))
	{
		stuff_float(&sip->sup_hull_repair_rate);
		sip->sup_hull_repair_rate *= 0.01f;
		CLAMP(sip->sup_hull_repair_rate, 0.0f, 1.0f);
	}

	//Subsys rep rate
	if(optional_string("$Subsystem Repair Rate:"))
	{
		stuff_float(&sip->subsys_repair_rate);
		sip->subsys_repair_rate *= 0.01f;
		
		//Sanity checking
		if(sip->subsys_repair_rate > 1.0f)
			sip->subsys_repair_rate = 1.0f;
		else if(sip->subsys_repair_rate < -1.0f)
			sip->subsys_repair_rate = -1.0f;
	}

	// Support ship hull repair rate
	if(optional_string("$Support Subsystem Repair Rate:"))
	{
		stuff_float(&sip->sup_subsys_repair_rate);
		sip->sup_subsys_repair_rate *= 0.01f;
		CLAMP(sip->sup_subsys_repair_rate, 0.0f, 1.0f);
	}

	if(optional_string("$Armor Type:"))
	{
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		sip->armor_type_idx = armor_type_get_idx(buf);

		if(sip->armor_type_idx == -1)
			Warning(LOCATION,"Invalid armor name %s specified for hull in %s '%s'", buf, info_type_name, sip->name);
	}

	if(optional_string("$Shield Armor Type:"))
	{
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		sip->shield_armor_type_idx = armor_type_get_idx(buf);

		if(sip->shield_armor_type_idx == -1)
			Warning(LOCATION,"Invalid armor name %s specified for shield in %s '%s'", buf, info_type_name, sip->name);
	}

	if (optional_string("$Flags:"))
	{
		// we'll assume the list will contain no more than 20 distinct tokens
		char ship_strings[20][NAME_LENGTH];
		int num_strings = stuff_string_list(ship_strings, 20);

		int ship_type_index = -1;

		if (!optional_string("+noreplace")) {
			// clear flags since we might have a modular table
			// clear only those which are actually set in the flags
			sip->flags = (sip->flags & SIF_MASK);
			sip->flags2 = (sip->flags2 & SIF2_MASK);
		}

		for (i = 0; i < num_strings; i++)
		{
			// get ship type from ship flags
			char *ship_type = ship_strings[i];
			bool flag_found = false;

			// Goober5000 - in retail FreeSpace, some ship classes were specified differently
			// in ships.tbl and the ship type array; this patches those differences so that
			// the ship type lookup will work properly
			if (!stricmp(ship_type, "sentrygun"))
				ship_type = "sentry gun";
			else if (!stricmp(ship_type, "escapepod"))
				ship_type = "escape pod";
			else if (!stricmp(ship_type, "repair_rearm"))
				ship_type = "support";
			else if (!stricmp(ship_type, "supercap"))
				ship_type = "super cap";
			else if (!stricmp(ship_type, "knossos"))
				ship_type = "knossos device";

			// look it up in the object types table
			ship_type_index = ship_type_name_lookup(ship_type);

			// set ship class type
			if ((ship_type_index >= 0) && (sip->class_type < 0))
				sip->class_type = ship_type_index;

			// check various ship flags
			for (int idx = 0; idx < Num_ship_flags; idx++) {
				if ( !stricmp(Ship_flags[idx].name, ship_strings[i]) ) {
					flag_found = true;

					if (Ship_flags[idx].var == 255)
						Warning(LOCATION, "Use of '%s' flag for %s '%s' - this flag is no longer needed.", Ship_flags[idx].name, info_type_name, sip->name);
					else if (Ship_flags[idx].var == 0)
						sip->flags |= Ship_flags[idx].def;
					else if (Ship_flags[idx].var == 1)
						sip->flags2 |= Ship_flags[idx].def;
				}
			}

			if ( !flag_found && (ship_type_index < 0) )
				Warning(LOCATION, "Bogus string in ship flags: %s\n", ship_strings[i]);
		}

		// set original status of tech database flags - Goober5000
		if (sip->flags & SIF_IN_TECH_DATABASE)
			sip->flags2 |= SIF2_DEFAULT_IN_TECH_DATABASE;
		if (sip->flags & SIF_IN_TECH_DATABASE_M)
			sip->flags2 |= SIF2_DEFAULT_IN_TECH_DATABASE_M;
	}

	// Goober5000 - ensure number of banks checks out
	if (sip->num_primary_banks > MAX_SHIP_PRIMARY_BANKS)
	{
		Error(LOCATION, "%s '%s' has too many primary banks (%d).  Maximum for ships is currently %d.\n", info_type_name, sip->name, sip->num_primary_banks, MAX_SHIP_PRIMARY_BANKS);
	}

	// copy to regular allowed_weapons array
	for (i=0; i<MAX_SHIP_WEAPONS; i++)
	{
		for (j=0; j<MAX_WEAPON_TYPES; j++)
		{
			if (sip->allowed_bank_restricted_weapons[i][j] & REGULAR_WEAPON)
				sip->allowed_weapons[j] |= REGULAR_WEAPON;

			if (sip->allowed_bank_restricted_weapons[i][j] & DOGFIGHT_WEAPON)
				sip->allowed_weapons[j] |= DOGFIGHT_WEAPON;
		}
	}

	//Set ship ballistic flag if necessary
	for (i=0; i<MAX_SHIP_PRIMARY_BANKS; i++)
	{
		for (j=0; j<MAX_WEAPON_TYPES; j++)
		{
			if(sip->allowed_bank_restricted_weapons[i][j] && (Weapon_info[j].wi_flags2 & WIF2_BALLISTIC))
			{
				sip->flags |= SIF_BALLISTIC_PRIMARIES;
				break;
			}
		}
	}

	find_and_stuff_optional("$AI Class:", &sip->ai_class, F_NAME, Ai_class_names, Num_ai_classes, "AI class names");

	// Get Afterburner information
	// Be aware that if $Afterburner is not 1, the other Afterburner fields are not read in
	int has_afterburner = 0;

	if(optional_string("$Afterburner:"))
		stuff_boolean(&has_afterburner);

	if ( has_afterburner == 1 )
	{
		sip->flags |= SIF_AFTERBURNER;

		if(optional_string("+Aburn Max Vel:")) {
			stuff_vec3d(&sip->afterburner_max_vel);
		}

		if(optional_string("+Aburn For accel:")) {
			stuff_float(&sip->afterburner_forward_accel);
		}

		// SparK: added reverse burner capability
		if(optional_string("+Aburn Max Reverse Vel:")) {
			stuff_float(&sip->afterburner_max_reverse_vel);
		}
		if(optional_string("+Aburn Rev accel:")) {
			stuff_float(&sip->afterburner_reverse_accel);
		}

		if(optional_string("+Aburn Fuel:")) {
			stuff_float(&sip->afterburner_fuel_capacity);
		}

		if(optional_string("+Aburn Burn Rate:")) {
			stuff_float(&sip->afterburner_burn_rate);
		}

		if(optional_string("+Aburn Rec Rate:")) {
			stuff_float(&sip->afterburner_recover_rate);
		}

		if (!(sip->afterburner_fuel_capacity) ) {
			Warning(LOCATION, "%s '%s' has an afterburner but has no afterburner fuel. Setting fuel to 1", info_type_name, sip->name);
			sip->afterburner_fuel_capacity = 1.0f;
		}
	}
	
	if ( optional_string("$Trails:") ) {
		bool trails_warning = true;

		if (optional_string("+Bitmap:") ) {
			trails_warning = false;
			generic_bitmap_init(&sip->afterburner_trail, NULL);
			stuff_string(sip->afterburner_trail.filename, F_NAME, MAX_FILENAME_LEN);
		}
		
		if ( optional_string("+Width:") ) {
			trails_warning = false;
			stuff_float(&sip->afterburner_trail_width_factor);
		}
			
		if ( optional_string("+Alpha:") ) {
			trails_warning = false;
			stuff_float(&sip->afterburner_trail_alpha_factor);
		}
			
		if ( optional_string("+Life:") ) {
			trails_warning = false;
			stuff_float(&sip->afterburner_trail_life);
		}

		if ( optional_string("+Faded Out Sections:") ) {
			trails_warning = false;
			stuff_int(&sip->afterburner_trail_faded_out_sections);
		}

		if (trails_warning)
			Warning(LOCATION, "%s %s entry has $Trails field specified, but no properties given.", info_type_name, sip->name);
	}

	if (optional_string("$Countermeasure type:")) {
		stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
		int res = weapon_info_lookup(buf);
		if (res < 0) {
			Warning(LOCATION, "Could not find weapon type '%s' to use as countermeasure on %s '%s'", buf, info_type_name, sip->name);
		} else if (Weapon_info[res].wi_flags & WIF_BEAM) {
			Warning(LOCATION, "Attempt made to set a beam weapon as a countermeasure on %s '%s'", info_type_name, sip->name);
		} else {
			sip->cmeasure_type = res;
		}
	} else if (Species_info[sip->species].cmeasure_index >= 0) {
		sip->cmeasure_type = Species_info[sip->species].cmeasure_index;
	}

	if(optional_string("$Countermeasures:"))
		stuff_int(&sip->cmeasure_max);

	if(optional_string("$Scan time:"))
		stuff_int(&sip->scan_time);

	//Parse the engine sound
	parse_sound("$EngineSnd:", &sip->engine_snd, sip->name);

	if(optional_string("$Minimum Engine Volume:"))
		stuff_float(&sip->min_engine_vol);

	//Parse optional sound to be used for beginning of a glide
	parse_sound("$GlideStartSnd:", &sip->glide_start_snd, sip->name);

	//Parse optional sound to be used for end of a glide
	parse_sound("$GlideEndSnd:", &sip->glide_end_snd, sip->name);

	parse_ship_sounds(sip);
	
	if(optional_string("$Closeup_pos:"))
	{
		stuff_vec3d(&sip->closeup_pos);
	}
	else if (first_time && strlen(sip->pof_file))
	{
		//Calculate from the model file. This is inefficient, but whatever
		int model_idx = model_load(sip->pof_file, 0, NULL);
		polymodel *pm = model_get(model_idx);

		//Go through, find best
		sip->closeup_pos.xyz.z = fabsf(pm->maxs.xyz.z);

		float temp = fabsf(pm->mins.xyz.z);
		if(temp > sip->closeup_pos.xyz.z)
			sip->closeup_pos.xyz.z = temp;

		//Now multiply by 2
		sip->closeup_pos.xyz.z *= -2.0f;

		//We're done with the model.
		model_unload(model_idx);
	}

	if (optional_string("$Closeup_zoom:")) {
		stuff_float(&sip->closeup_zoom);

		if (sip->closeup_zoom <= 0.0f) {
			mprintf(("Warning!  Ship '%s' has a $Closeup_zoom value that is less than or equal to 0 (%f). Setting to default value.\n", sip->name, sip->closeup_zoom));
			sip->closeup_zoom = 0.5f;
		}
	}
		
	if(optional_string("$Topdown offset:")) {
		sip->topdown_offset_def = true;
		stuff_vec3d(&sip->topdown_offset);
	}

	if (optional_string("$Shield_icon:")) {
		stuff_string(name_tmp, F_NAME, sizeof(name_tmp));
		hud_shield_assign_info(sip, name_tmp);
	}

	// read in filename for icon that is used in ship selection
	if ( optional_string("$Ship_icon:") ) {
		stuff_string(sip->icon_filename, F_NAME, MAX_FILENAME_LEN);
	}

	if ( optional_string("$Model Icon Direction:") ) {
		char str[NAME_LENGTH];
		stuff_string(str, F_NAME, NAME_LENGTH);

		angles model_icon_angles = {0.0f,0.0f,0.0f};

		if (!stricmp(str, "top")) {
			model_icon_angles.p = -PI_2;
		} else if (!stricmp(str, "bottom")) {
			model_icon_angles.p = -PI_2;
			model_icon_angles.b = 2 * PI_2;
		} else if (!stricmp(str, "front")) {
			model_icon_angles.h = 2 * PI_2;
		} else if (!stricmp(str, "back")) {
			model_icon_angles.h = 4 * PI_2;
		} else if (!stricmp(str, "left")) {
			model_icon_angles.h = -PI_2;
		} else if (!stricmp(str, "right")) {
			model_icon_angles.h = PI_2;
		} else {
			Warning(LOCATION, "Unrecognized value \"%s\" passed to $Model Icon Direction, ignoring...", str);
		}

		sip->model_icon_angles = model_icon_angles;
	}

	// read in filename for animation that is used in ship selection
	if ( optional_string("$Ship_anim:") ) {
		stuff_string(sip->anim_filename, F_NAME, MAX_FILENAME_LEN);
	}

	// read in filename for animation that is used in ship selection
	if ( optional_string("$Ship_overhead:") ) {
		stuff_string(sip->overhead_filename, F_NAME, MAX_FILENAME_LEN);
	}

	// read in briefing stuff
	if ( optional_string("$Briefing icon:") )
		sip->bii_index_ship = parse_and_add_briefing_icon_info();
	if ( optional_string("$Briefing icon with cargo:") )
		sip->bii_index_ship_with_cargo = parse_and_add_briefing_icon_info();
	if ( optional_string("$Briefing wing icon:") )
		sip->bii_index_wing = parse_and_add_briefing_icon_info();
	if ( optional_string("$Briefing wing icon with cargo:") )
		sip->bii_index_wing_with_cargo = parse_and_add_briefing_icon_info();

	// check for inconsistencies
	if ((sip->bii_index_wing_with_cargo >= 0) && (sip->bii_index_wing < 0 || sip->bii_index_ship_with_cargo < 0))
		Warning(LOCATION, "%s '%s' has a wing-with-cargo briefing icon but is missing a wing briefing icon or a ship-with-cargo briefing icon!", info_type_name, sip->name);
	if ((sip->bii_index_wing_with_cargo < 0) && (sip->bii_index_wing >= 0) && (sip->bii_index_ship_with_cargo >= 0))
		Warning(LOCATION, "%s '%s' has both a wing briefing icon and a ship-with-cargo briefing icon but does not have a wing-with-cargo briefing icon!", info_type_name, sip->name);

	if ( optional_string("$Score:") ){
		stuff_int( &sip->score );
	}

	if (first_time)
	{
		species_info *species = &Species_info[sip->species];

		sip->thruster_flame_info = species->thruster_info.flames;
		sip->thruster_glow_info = species->thruster_info.glow;
		sip->thruster_secondary_glow_info = species->thruster_secondary_glow_info;
		sip->thruster_tertiary_glow_info = species->thruster_tertiary_glow_info;
		sip->thruster_distortion_info = species->thruster_distortion_info;
	}

	if ( optional_string("$Thruster Normal Flame:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );
	
		if ( VALID_FNAME(name_tmp) )
			generic_anim_init( &sip->thruster_flame_info.normal, name_tmp );
	}

	if ( optional_string("$Thruster Afterburner Flame:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_anim_init( &sip->thruster_flame_info.afterburn, name_tmp );
	}

	if ( optional_string("$Thruster Bitmap 1:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );
	
		if ( VALID_FNAME(name_tmp) ) {
			strcpy_s(sip->thruster_glow_info.normal.filename, name_tmp);
			thruster_glow_anim_load( &sip->thruster_glow_info.normal );
		}
	}

	if ( optional_string("$Thruster Bitmap 1a:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) ) {
			strcpy_s(sip->thruster_glow_info.afterburn.filename, name_tmp);
			thruster_glow_anim_load( &sip->thruster_glow_info.afterburn );
		}
	}

	if ( optional_string("$Thruster01 Radius factor:") ) {
		stuff_float(&sip->thruster01_glow_rad_factor);
	}

	if ( optional_string("$Thruster Bitmap 2:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_bitmap_init( &sip->thruster_secondary_glow_info.normal, name_tmp );
	}

	if ( optional_string("$Thruster Bitmap 2a:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_bitmap_init( &sip->thruster_secondary_glow_info.afterburn, name_tmp );
	}

	if ( optional_string("$Thruster02 Radius factor:") ) {
		stuff_float(&sip->thruster02_glow_rad_factor);
	}

	if ( optional_string("$Thruster01 Length factor:") ) {
		stuff_float(&sip->thruster02_glow_len_factor);
		Warning(LOCATION, "Deprecated spelling: \"$Thruster01 Length factor:\".  Use \"$Thruster02 Length factor:\" instead.");
	}

	if ( optional_string("$Thruster02 Length factor:") ) {
		stuff_float(&sip->thruster02_glow_len_factor);
	}

	if ( optional_string("$Thruster Bitmap 3:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_bitmap_init( &sip->thruster_tertiary_glow_info.normal, name_tmp );
	}

	if ( optional_string("$Thruster Bitmap 3a:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_bitmap_init( &sip->thruster_tertiary_glow_info.afterburn, name_tmp );
	}

	if ( optional_string("$Thruster03 Radius factor:") ) {
		stuff_float(&sip->thruster03_glow_rad_factor);
	}

	// Valathil - Custom Thruster Distortion
	if ( optional_string("$Thruster Bitmap Distortion:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_bitmap_init( &sip->thruster_distortion_info.normal, name_tmp );
	}

	if ( optional_string("$Thruster Bitmap Distortion a:") ) {
		stuff_string( name_tmp, F_NAME, sizeof(name_tmp) );

		if ( VALID_FNAME(name_tmp) )
			generic_bitmap_init( &sip->thruster_distortion_info.afterburn, name_tmp );
	}

	if ( optional_string("$Thruster Distortion Radius factor:") ) {
		stuff_float(&sip->thruster_dist_rad_factor);
	}

	if ( optional_string("$Thruster Distortion Length factor:") ) {
		stuff_float(&sip->thruster_dist_len_factor);
	}

	if ( optional_string("$Thruster Distortion:") ) {
		stuff_boolean(&sip->draw_distortion);
	}

	if ( optional_string("$Thruster Glow Noise Mult:") ) {
		stuff_float(&sip->thruster_glow_noise_mult);
	}

	while ( optional_string("$Thruster Particles:") ) {
		bool afterburner = false;
		thruster_particles tpart;

		if ( optional_string("$Thruster Particle Bitmap:") )
			afterburner = false;
		else if ( optional_string("$Afterburner Particle Bitmap:") )
			afterburner = true;
		else
			Error( LOCATION, "formatting error in the thruster's particle section for %s '%s'\n", info_type_name, sip->name );

		generic_anim_init(&tpart.thruster_bitmap, NULL);
		stuff_string(tpart.thruster_bitmap.filename, F_NAME, MAX_FILENAME_LEN);

		required_string("$Min Radius:");
		stuff_float(&tpart.min_rad);
		
		required_string("$Max Radius:");
		stuff_float(&tpart.max_rad);
		
		required_string("$Min created:");
		stuff_int(&tpart.n_low);
		
		required_string("$Max created:");
		stuff_int(&tpart.n_high);
		
		required_string("$Variance:");
		stuff_float(&tpart.variance);

		if (afterburner) {
			sip->afterburner_thruster_particles.push_back( tpart );
		} else {
			sip->normal_thruster_particles.push_back( tpart );
		}
	}

	// if the ship is a stealth ship
	if ( optional_string("$Stealth:") ) {
		sip->flags |= SIF_STEALTH;
	}
	
	else if ( optional_string("$Stealth") ) {
		Warning(LOCATION, "%s '%s' is missing the colon after \"$Stealth\". Note that you may also use the ship flag \"stealth\".", info_type_name, sip->name);
		sip->flags |= SIF_STEALTH;
	}

	if ( optional_string("$max decals:") ){
		int bogus;
		stuff_int(&bogus);
		WarningEx(LOCATION, "The decal system has been deactivated in FSO builds. Entries will be discarded.\n");
		mprintf(("WARNING: The decal system has been deactivated in FSO builds. Entries will be discarded.\n"));
		//Do nothing, left in for compatibility.
	}

	// parse contrail info
	while ( optional_string("$Trail:") ) {
		// setting '+ClearAll' resets the trails
		if ( optional_string("+ClearAll")) {
			memset(&sip->ct_info, 0, sizeof(trail_info) * MAX_SHIP_CONTRAILS);
			sip->ct_count = 0;
		}

		// this means you've reached the max # of contrails for a ship
		if (sip->ct_count >= MAX_SHIP_CONTRAILS) {
			Warning(LOCATION, "%s '%s' has more contrails than the max of %d", info_type_name, sip->name, MAX_SHIP_CONTRAILS);
			break;
		}

		trail_info *ci = &sip->ct_info[sip->ct_count++];
		
		required_string("+Offset:");
		stuff_vec3d(&ci->pt);
		
		required_string("+Start Width:");
		stuff_float(&ci->w_start);
		
		required_string("+End Width:");
		stuff_float(&ci->w_end);
		
		required_string("+Start Alpha:");
		stuff_float(&ci->a_start);
		
		required_string("+End Alpha:");
		stuff_float(&ci->a_end);

		required_string("+Max Life:");
		stuff_float(&ci->max_life);
		
		required_string("+Spew Time:");
		stuff_int(&ci->stamp);		

		required_string("+Bitmap:");
		stuff_string(name_tmp, F_NAME, NAME_LENGTH);
		generic_bitmap_init(&ci->texture, name_tmp);
		generic_bitmap_load(&ci->texture);

		if (optional_string("+Faded Out Sections:") ) {
			stuff_int(&ci->n_fade_out_sections);
		}
	}

	man_thruster *mtp = NULL;
	man_thruster manwich;
	while(optional_string("$Thruster:"))
	{
		int idx = -1;
		if(optional_string("+index:")) {
			stuff_int(&idx);
		}

		if(idx >= 0 && idx < sip->num_maneuvering) {
			mtp = &sip->maneuvering[idx];
		} else if(idx < 0) {
			if(sip->num_maneuvering < MAX_MAN_THRUSTERS) {
				mtp = &sip->maneuvering[sip->num_maneuvering++];
			} else {
				Warning(LOCATION, "Too many maneuvering thrusters on %s '%s'; maximum is %d", info_type_name, sip->name, MAX_MAN_THRUSTERS);
			}
		} else {
			mtp = &manwich;
			Warning(LOCATION, "Invalid index (%d) specified for maneuvering thruster on %s '%s'", idx, info_type_name, sip->name);
		}

		if(optional_string("+Used for:")) {
			parse_string_flag_list(&mtp->use_flags, Man_types, Num_man_types);
		}

		if(optional_string("+Position:")) {
			stuff_float_list(mtp->pos.a1d, 3);
		}

		if(optional_string("+Normal:")) {
			stuff_float_list(mtp->norm.a1d, 3);
		}

		if(optional_string("+Texture:"))
		{
			stuff_string(name_tmp, F_NAME, sizeof(name_tmp));
			int tex_fps=0, tex_nframes=0, tex_id=-1;;
			tex_id = bm_load_animation(name_tmp, &tex_nframes, &tex_fps, NULL, 1);
			if(tex_id < 0)
				tex_id = bm_load(name_tmp);
			if(tex_id >= 0)
			{
				if(mtp->tex_id >= 0) {
					bm_unload(mtp->tex_id);
				}

				mtp->tex_id = tex_id;
				mtp->tex_fps = tex_fps;
				mtp->tex_nframes = tex_nframes;
			}
		}

		if(optional_string("+Radius:")) {
			stuff_float(&mtp->radius);
		}

		if(optional_string("+Length:")) {
			stuff_float(&mtp->length);
		}

		parse_sound("+StartSnd:", &mtp->start_snd, sip->name);
		parse_sound("+LoopSnd:", &mtp->loop_snd, sip->name);
		parse_sound("+StopSnd:", &mtp->stop_snd, sip->name);
	}

	if (optional_string("$Glowpoint overrides:")) {
		SCP_vector<SCP_string> tokens;
		tokens.clear();
		stuff_string_list(tokens);
		for(SCP_vector<SCP_string>::iterator token = tokens.begin(); token != tokens.end(); ++token) {
			SCP_string name, banks;
			size_t seppos;
			seppos = token->find_first_of(':');
			if(seppos == SCP_string::npos) {
				Warning(LOCATION, "Couldn't find ':' seperator in Glowpoint override for ship %s ignoring token", sip->name);
				continue;
			}
			name = token->substr(0, seppos);
			banks = token->substr(seppos+1);
			SCP_vector<glow_point_bank_override>::iterator gpo = get_glowpoint_bank_override_by_name(name.data());
			if(gpo == glowpoint_bank_overrides.end()){
				Warning(LOCATION, "Couldn't find preset %s in glowpoints.tbl when parsing ship: %s", name.data(), sip->name);
				continue;
			}
			if(banks == "*") {
				sip->glowpoint_bank_override_map[-1] = (void*)(&(*gpo));
				continue;
			}
			SCP_string banktoken;
			int start = -1;
			int end = -1;
			do {
				end = banks.find_first_of(',', ++start);
				banktoken = banks.substr(start, end);
				start = end;
				
				size_t fromtopos;
				fromtopos = banktoken.find_first_of('-');
				if(fromtopos != SCP_string::npos) {
					SCP_string from, to;
					int ifrom, ito;
					from = banktoken.substr(0, fromtopos);
					to = banktoken.substr(fromtopos+1);
					ifrom = atoi(from.data()) - 1;
					ito = atoi(to.data()) - 1;
					for(int bank = ifrom; bank <= ito; ++bank) {
						sip->glowpoint_bank_override_map[bank] = (void*)(&(*gpo));
					}
				} else {
					int bank = atoi(banktoken.data()) - 1;
					sip->glowpoint_bank_override_map[bank] = (void*)(&(*gpo));
				}
			} while(start!=-1);
		}
	}

	if (optional_string("$Radar Image 2D:"))
	{
		stuff_string(name_tmp, F_NAME, NAME_LENGTH);
		sip->radar_image_2d_idx = bm_load(name_tmp);

		if ( optional_string("$Radar Color Image 2D:") ) {
			stuff_string(name_tmp, F_NAME, NAME_LENGTH);
			sip->radar_color_image_2d_idx = bm_load(name_tmp);
		}

		if (optional_string("$Radar Image Size:"))
			stuff_int(&sip->radar_image_size);

		if (optional_string("$3D Radar Blip Size Multiplier:"))
			stuff_float(&sip->radar_projection_size_mult);
	}

	// Alternate - per ship class - IFF colors
	while((optional_string("$Ship IFF Colors:")) || (optional_string("$Ship IFF Colours:")))
	{
		char iff_1[NAME_LENGTH];
		char iff_2[NAME_LENGTH];
		int iff_color_data[3];
		int iff_data[2];
		
		// Get the iff strings and get the iff indexes
		required_string("+Seen By:");
		stuff_string(iff_1, F_NAME, NAME_LENGTH);
		
		required_string("+When IFF Is:");
		stuff_string(iff_2, F_NAME, NAME_LENGTH);
		iff_data[0] = iff_lookup(iff_1);
		iff_data[1] = iff_lookup(iff_2);

		if (iff_data[0] == -1)
			WarningEx(LOCATION, "%s '%s'\nIFF colour seen by \"%s\" invalid!", info_type_name, sip->name, iff_1);

		if (iff_data[1] == -1)
			WarningEx(LOCATION, "%s '%s'\nIFF colour when IFF is \"%s\" invalid!", info_type_name, sip->name, iff_2);

		// Set the color
		required_string("+As Color:");
		stuff_int_list(iff_color_data, 3, RAW_INTEGER_TYPE);
		sip->ship_iff_info[iff_data[0]][iff_data[1]] = iff_init_color(iff_color_data[0],iff_color_data[1],iff_color_data[2]);
	}

	if (optional_string("$Target Priority Groups:") ) {
		SCP_vector<SCP_string> target_group_strings;
		int num_strings = stuff_string_list(target_group_strings);
		int num_groups = Ai_tp_list.size();
		bool override_strings = false;

		if (optional_string("+Override")) {
			override_strings = true;
		}

		for(j = 0; j < num_strings; j++) {
			for(i = 0; i < num_groups; i++) {
				if ( !stricmp(target_group_strings[j].c_str(), Ai_tp_list[i].name) ) {
					//so now the string from the list above as well as the ai priority group name match
					//clear it if override has been set
					if (override_strings) {
						Ai_tp_list[i].ship_class.clear();
						override_strings = false;
					}
					for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
						//find the index number of the current ship info type
						if (it->name == sip->name) {
							Ai_tp_list[i].ship_class.push_back(std::distance(Ship_info.cbegin(), it));
							break;
						}
					}
					// found something, try next string
					break;
				}
			}
			if (i == num_groups) {
				Warning(LOCATION,"Unidentified priority group '%s' set for %s '%s'\n", target_group_strings[j].c_str(), info_type_name, sip->name);
			}
		}
	}

	if (optional_string("$EMP Resistance Modifier:")) {
		stuff_float(&sip->emp_resistance_mod);
	}
	
	if (optional_string("$Piercing Damage Draw Limit:")) {
		float tempf;
		stuff_float(&tempf);
		sip->piercing_damage_draw_limit = tempf / 100.0f;
	}

	while(optional_string("$Path Metadata:")) 
	{
		char path_name[64];
		stuff_string(path_name, F_NAME, sizeof(path_name));

		path_metadata metadata;
		init_path_metadata(metadata);

		//Get +departure rvec and store on the path_metadata object
		if (optional_string("+departure rvec:"))
		{
			stuff_vec3d(&metadata.departure_rvec);
		}

		if (optional_string("+arrive speed multiplier:"))
		{
			stuff_float(&metadata.arrive_speed_mult);
		}
		if (optional_string("+depart speed multiplier:"))
		{
			stuff_float(&metadata.depart_speed_mult);
		}

		//Add the new path_metadata to sip->pathMetadata keyed by path name
		SCP_string pathName(path_name);
		sip->pathMetadata[pathName] = metadata;
	}

	int n_subsystems = 0;
	int cont_flag = 1;
	model_subsystem subsystems[MAX_MODEL_SUBSYSTEMS];		// see model.h for max_model_subsystems
	for (i=0; i<MAX_MODEL_SUBSYSTEMS; i++) {
		subsystems[i].stepped_rotation = NULL;
	}
	
	float	hull_percentage_of_hits = 100.0f;
	//If the ship already has subsystem entries (ie this is a modular table)
	//make sure hull_percentage_of_hits is set properly
	for(i=0; i < sip->n_subsystems; i++) {
		hull_percentage_of_hits -= sip->subsystems[i].max_subsys_strength / sip->max_hull_strength;
	}

	while (cont_flag) {
		int r = required_string_one_of(3, "#End", "$Subsystem:", type_name);
		switch (r) {
		case 0:
			cont_flag = 0;
			break;
		case 1:
		{
			float	turning_rate;
			float	percentage_of_hits;
			bool turret_has_base_fov = false;
			model_subsystem *sp = NULL;			// to append on the ships list of subsystems
			
			int sfo_return;
			required_string("$Subsystem:");
			stuff_string(name_tmp, F_NAME, sizeof(name_tmp), ",");
			Mp++;
			for(i = 0;i < sip->n_subsystems; i++)
			{
				if(!subsystem_stricmp(sip->subsystems[i].subobj_name, name_tmp))
					sp = &sip->subsystems[i];
			}

			if(sp == NULL)
			{
				if( sip->n_subsystems + n_subsystems >= MAX_MODEL_SUBSYSTEMS )
				{
					Warning(LOCATION, "Number of subsystems for %s '%s' (%d) exceeds max of %d; only the first %d will be used", info_type_name, sip->name, sip->n_subsystems, n_subsystems, MAX_MODEL_SUBSYSTEMS);
					break;
				}
				sp = &subsystems[n_subsystems++];			// subsystems a local -- when done, we will malloc and copy
				strcpy_s(sp->subobj_name, name_tmp);
				
				//Init blank values
				sp->max_subsys_strength = 0.0f;
				sp->turret_turning_rate = 0.0f;
				sp->weapon_rotation_pbank = -1;

				memset(sp->alt_sub_name, 0, sizeof(sp->alt_sub_name) );
				memset(sp->alt_dmg_sub_name, 0, sizeof(sp->alt_dmg_sub_name) );

				for (i=0; i<MAX_SHIP_PRIMARY_BANKS; i++) {
					sp->primary_banks[i] = -1;
					sp->primary_bank_capacity[i] = 0;
				}

				for (i=0; i<MAX_SHIP_SECONDARY_BANKS; i++) {
					sp->secondary_banks[i] = -1;
					sp->secondary_bank_capacity[i] = 0;
				}

				sp->engine_wash_pointer = NULL;
				
				sp->alive_snd = -1;
				sp->dead_snd = -1;
				sp->rotation_snd = -1;
				sp->turret_gun_rotation_snd = -1;
				sp->turret_gun_rotation_snd_mult = 1.0f;
				sp->turret_base_rotation_snd = -1;
				sp->turret_base_rotation_snd_mult = 1.0f;
				
				sp->flags = 0;
				sp->flags2 = 0;
				
				sp->n_triggers = 0;
				sp->triggers = NULL;
				
				sp->model_num = -1;		// init value for later sanity checking!!
				sp->armor_type_idx = -1;
				sp->path_num = -1;
				sp->turret_max_fov = 1.0f;

				sp->turret_reset_delay = 2000;

				sp->num_target_priorities = 0;
				for (i = 0; i < 32; i++) {
					sp->target_priority[i] = -1;
				}
				sp->optimum_range = 0.0f;
				sp->favor_current_facing = 0.0f;

				sp->turret_rof_scaler = 1.0f;

				sp->turret_max_bomb_ownage = -1;
				sp->turret_max_target_ownage = -1;
			}
			sfo_return = stuff_float_optional(&percentage_of_hits);
			if(sfo_return==2)
			{
				hull_percentage_of_hits -= percentage_of_hits;
				sp->max_subsys_strength = sip->max_hull_strength * (percentage_of_hits / 100.0f);
				sp->type = SUBSYSTEM_UNKNOWN;
			}
			if(sfo_return > 0)
			{
				if(stuff_float_optional(&turning_rate)==2)
				{
					// specified as how long to turn 360 degrees in ships.tbl
					if ( turning_rate > 0.0f ){
						sp->turret_turning_rate = PI2 / turning_rate;		
					} else {
						sp->turret_turning_rate = 0.0f;		
					}
				}
				else
				{
					Error(LOCATION, "Malformed $Subsystem entry '%s' in %s '%s'.\n\n"
						"Specify a turning rate or remove the trailing comma.",
						sp->subobj_name, info_type_name, sip->name);
				}
			}

			if(optional_string("$Alt Subsystem Name:")) {
				stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
				strcpy_s(sp->alt_sub_name, buf);
			}

			if(optional_string("$Alt Damage Popup Subsystem Name:")) {
				stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
				strcpy_s(sp->alt_dmg_sub_name, buf);
			}

			if(optional_string("$Armor Type:")) {
				stuff_string(buf, F_NAME, SHIP_MULTITEXT_LENGTH);
				sp->armor_type_idx = armor_type_get_idx(buf);

				if (sp->armor_type_idx == -1)
					WarningEx(LOCATION, "%s '%s', subsystem %s\nInvalid armor type %s!", info_type_name, sip->name, sp->subobj_name, buf);
			}

			//	Get primary bank weapons
			parse_weapon_bank(sip, true, NULL, sp->primary_banks, sp->primary_bank_capacity);

			//	Get secondary bank weapons
			parse_weapon_bank(sip, false, NULL, sp->secondary_banks, sp->secondary_bank_capacity);

			// Get optional engine wake info
			if (optional_string("$Engine Wash:")) {
				stuff_string(name_tmp, F_NAME, sizeof(name_tmp));
				// get and set index
				sp->engine_wash_pointer = get_engine_wash_pointer(name_tmp);

				if(sp->engine_wash_pointer == NULL)
					WarningEx(LOCATION,"Invalid engine wash name %s specified for subsystem %s in %s '%s'", name_tmp, sp->subobj_name, info_type_name, sip->name);
			}

			parse_sound("$AliveSnd:", &sp->alive_snd, sp->subobj_name);
			parse_sound("$DeadSnd:", &sp->dead_snd, sp->subobj_name);
			parse_sound("$RotationSnd:", &sp->rotation_snd, sp->subobj_name);
			parse_sound("$Turret Base RotationSnd:", &sp->turret_base_rotation_snd, sp->subobj_name);
			parse_sound("$Turret Gun RotationSnd:", &sp->turret_gun_rotation_snd, sp->subobj_name);

			if (optional_string("$Turret BaseSnd Volume:"))
				stuff_float(&sp->turret_base_rotation_snd_mult);

			if (optional_string("$Turret GunSnd Volume:"))
				stuff_float(&sp->turret_gun_rotation_snd_mult);
				
			// Get any AWACS info
			sp->awacs_intensity = 0.0f;
			if(optional_string("$AWACS:")){
				sfo_return = stuff_float_optional(&sp->awacs_intensity);
				if(sfo_return > 0)
					stuff_float_optional(&sp->awacs_radius);
				sip->flags |= SIF_HAS_AWACS;
			}

			if(optional_string("$Maximum Barrel Elevation:")){
				int value;
				stuff_int(&value);
				CAP(value, 0, 90);
				float angle = ANG_TO_RAD((float) (90 - value));
				sp->turret_max_fov = (float)cos(angle);
			}

			if(optional_string("$Turret Base FOV:")) {
				int value;
				stuff_int(&value);
				CAP(value, 0, 359);
				float angle = ANG_TO_RAD((float) value)/2.0f;
				sp->turret_y_fov = (float)cos(angle);
				turret_has_base_fov = true;
			}

			if (optional_string("$Turret Reset Delay:"))
				stuff_int(&sp->turret_reset_delay);

			if (optional_string("$Turret Optimum Range:"))
				stuff_float(&sp->optimum_range);

			if (optional_string("$Turret Direction Preference:")) {
				int temp;
				stuff_int(&temp);
				if (temp == 0) {
					sp->favor_current_facing = 0.0f;
				} else {
					CAP(temp, 1, 100);
					sp->favor_current_facing = 1.0f + (((float) (100 - temp)) / 10.0f);
				}
			}

			if (optional_string("$Target Priority:")) {
				SCP_vector <SCP_string> tgt_priorities;
				int num_strings = stuff_string_list(tgt_priorities);
				sp->num_target_priorities = 0;

				if (num_strings > 32)
					num_strings = 32;

				int num_groups = Ai_tp_list.size();

				for(i = 0; i < num_strings; i++) {
					for(j = 0; j < num_groups; j++) {
						if ( !stricmp(Ai_tp_list[j].name, tgt_priorities[i].c_str()))  {
							sp->target_priority[i] = j;
							sp->num_target_priorities++;
							break;
						}
					}
					if (j == num_groups) {
						Warning(LOCATION, "Unidentified target priority '%s' set for\nsubsystem '%s' in %s '%s'.", tgt_priorities[i].c_str(), sp->subobj_name, info_type_name, sip->name);
					}
				}
			}

			if (optional_string("$Max Turrets per Bomb:")) {
				stuff_int(&sp->turret_max_bomb_ownage);
			}

			if (optional_string("$Max Turrets per Target:")) {
				stuff_int(&sp->turret_max_target_ownage);
			}

			if (optional_string("$ROF:")) {

				if (optional_string("+Use firingpoints")) {
					sp->turret_rof_scaler = 0;
				} else {
					if (optional_string("+Multiplier:")) {
						float tempf;
						stuff_float(&tempf);

						if (tempf < 0) {
							mprintf(("RoF multiplier clamped to 0 for subsystem '%s' in %s '%s'.\n", sp->subobj_name, info_type_name, sip->name));
							sp->turret_rof_scaler = 0;
						} else {
							sp->turret_rof_scaler = tempf;
						}
					} else {
						Warning(LOCATION, "RoF multiplier not set for subsystem\n'%s' in %s '%s'.", sp->subobj_name, info_type_name, sip->name);
					}
				}
			}

			if (optional_string("$Flags:")) {
				char flag_strings[Num_subsystem_flags][NAME_LENGTH];
				int num_strings = stuff_string_list(flag_strings, NUM_SUBSYSTEM_FLAGS);

				if (!optional_string("+noreplace")) {
					// clear flags since we might have a modular table
					// clear only those which are actually set in the flags
					sp->flags = 0;
					sp->flags2 = 0;
				}

				for (i = 0; i < num_strings; i++)
				{

					bool flag_found = false;
					// check various subsystem flags
					for (int idx = 0; idx < Num_subsystem_flags; idx++) {
						if ( !stricmp(Subsystem_flags[idx].name, flag_strings[i]) ) {
							flag_found = true;
							
							if (Subsystem_flags[idx].var == 0)
								sp->flags |= Subsystem_flags[idx].def;
							else if (Subsystem_flags[idx].var == 1)
								sp->flags2 |= Subsystem_flags[idx].def;
						}
					}

					if ( !flag_found )
						Warning(LOCATION, "Bogus string in subsystem flags: %s\n", flag_strings[i]);
				}

				//If we've set any subsystem as landable, set a ship-info flag as a shortcut for later
				if (sp->flags & MSS_FLAG_ALLOW_LANDING)
					sip->flags2 |= SIF2_ALLOW_LANDINGS;
			}

			if (turret_has_base_fov)
				sp->flags |= MSS_FLAG_TURRET_ALT_MATH;

			if (optional_string("+non-targetable")) {
				Warning(LOCATION, "Grammar error in table file.  Please change \"+non-targetable\" to \"+untargetable\".");
				sp->flags |= MSS_FLAG_UNTARGETABLE;
			}

			bool old_flags = false;
			if (optional_string("+untargetable")) {
				sp->flags |= MSS_FLAG_UNTARGETABLE;
				old_flags = true;
			}

			if (optional_string("+carry-no-damage")) {
				sp->flags |= MSS_FLAG_CARRY_NO_DAMAGE;
				old_flags = true;
			}

			if (optional_string("+use-multiple-guns")) {
				sp->flags |= MSS_FLAG_USE_MULTIPLE_GUNS;
				old_flags = true;
			}

			if (optional_string("+fire-down-normals")) {
				sp->flags |= MSS_FLAG_FIRE_ON_NORMAL;
				old_flags = true;
			}

			if ((sp->flags & MSS_FLAG_TURRET_FIXED_FP) && !(sp->flags & MSS_FLAG_USE_MULTIPLE_GUNS)) {
				Warning(LOCATION, "\"fixed firingpoints\" flag used without \"use multiple guns\" flag on a subsystem on %s '%s'.\n\"use multiple guns\" flags added by default\n", info_type_name, sip->name);
				sp->flags |= MSS_FLAG_USE_MULTIPLE_GUNS;
			}

			if ((sp->flags2 & MSS_FLAG2_AUTOREPAIR_IF_DISABLED) && (sp->flags2 & MSS_FLAG2_NO_AUTOREPAIR_IF_DISABLED)) {
				Warning(LOCATION, "\"autorepair if disabled\" flag used with \"don't autorepair if disabled\" flag on a subsystem on %s '%s'.\nWhichever flag would be default behavior anyway for this ship has been removed.\n", info_type_name, sip->name);
				if (sip->flags2 & SIF2_SUBSYS_REPAIR_WHEN_DISABLED){
					sp->flags2 &= ~MSS_FLAG2_AUTOREPAIR_IF_DISABLED;
				} else {
					sp->flags2 &= ~MSS_FLAG2_NO_AUTOREPAIR_IF_DISABLED;
				}
			}

			if (old_flags) {
				mprintf(("Use of deprecated subsystem syntax.  Please use the $Flags: field for subsystem flags.\n\n" \
				"At least one of the following tags was used on %s '%s', subsystem %s:\n" \
				"\t+untargetable\n" \
				"\t+carry-no-damage\n" \
				"\t+use-multiple-guns\n" \
				"\t+fire-down-normals\n", info_type_name, sip->name, sp->subobj_name));
			}

			while(optional_string("$animation:"))
			{
				stuff_string(name_tmp, F_NAME, sizeof(name_tmp));
				if(!stricmp(name_tmp, "triggered"))
				{
					queued_animation *current_trigger;

					sp->triggers = (queued_animation*)vm_realloc(sp->triggers, sizeof(queued_animation) * (sp->n_triggers + 1));
					Verify(sp->triggers != NULL);
					
					//add a new trigger
					current_trigger = &sp->triggers[sp->n_triggers];
					queued_animation_init(current_trigger);
					sp->n_triggers++;

					required_string("$type:");
					char atype[NAME_LENGTH];
					stuff_string(atype, F_NAME, NAME_LENGTH);
					current_trigger->type = model_anim_match_type(atype);

					if(optional_string("+sub_type:")){
						stuff_int(&current_trigger->subtype);
					}else{
						current_trigger->subtype = ANIMATION_SUBTYPE_ALL;
					}

					if(optional_string("+sub_name:")) {
						stuff_string(current_trigger->sub_name, F_NAME, NAME_LENGTH);
					} else {
						strcpy_s(current_trigger->sub_name, "<none>");
					}


					if(current_trigger->type == TRIGGER_TYPE_INITIAL){
						//the only thing initial animation type needs is the angle, 
						//so to save space lets just make everything optional in this case

						if(optional_string("+delay:"))
							stuff_int(&current_trigger->start); 
						else
							current_trigger->start = 0;

						if ( optional_string("+reverse_delay:") )
							stuff_int(&current_trigger->reverse_start);
						else
							current_trigger->reverse_start = 0;

						if(optional_string("+absolute_angle:")){
							current_trigger->absolute = true;
							stuff_vec3d(&current_trigger->angle );
		
							current_trigger->angle.xyz.x = fl_radians(current_trigger->angle.xyz.x);
							current_trigger->angle.xyz.y = fl_radians(current_trigger->angle.xyz.y);
							current_trigger->angle.xyz.z = fl_radians(current_trigger->angle.xyz.z);
						}else{
							current_trigger->absolute = false;
							if(!optional_string("+relative_angle:"))
								required_string("+relative_angle:");

							stuff_vec3d(&current_trigger->angle );
		
							current_trigger->angle.xyz.x = fl_radians(current_trigger->angle.xyz.x);
							current_trigger->angle.xyz.y = fl_radians(current_trigger->angle.xyz.y);
							current_trigger->angle.xyz.z = fl_radians(current_trigger->angle.xyz.z);
						}
		
						if(optional_string("+velocity:")){
							stuff_vec3d(&current_trigger->vel );
							current_trigger->vel.xyz.x = fl_radians(current_trigger->vel.xyz.x);
							current_trigger->vel.xyz.y = fl_radians(current_trigger->vel.xyz.y);
							current_trigger->vel.xyz.z = fl_radians(current_trigger->vel.xyz.z);
						}
		
						if(optional_string("+acceleration:")){
							stuff_vec3d(&current_trigger->accel );
							current_trigger->accel.xyz.x = fl_radians(current_trigger->accel.xyz.x);
							current_trigger->accel.xyz.y = fl_radians(current_trigger->accel.xyz.y);
							current_trigger->accel.xyz.z = fl_radians(current_trigger->accel.xyz.z);
						}

						if(optional_string("+time:"))
							stuff_int(&current_trigger->end );
						else
							current_trigger->end = 0;
					}else{

						if(optional_string("+delay:"))
							stuff_int(&current_trigger->start); 
						else
							current_trigger->start = 0;

						if ( optional_string("+reverse_delay:") )
							stuff_int(&current_trigger->reverse_start);
						else
							current_trigger->reverse_start = -1; //have some code figure this out for us
		
						if(optional_string("+absolute_angle:")){
							current_trigger->absolute = true;
							stuff_vec3d(&current_trigger->angle );
		
							current_trigger->angle.xyz.x = fl_radians(current_trigger->angle.xyz.x);
							current_trigger->angle.xyz.y = fl_radians(current_trigger->angle.xyz.y);
							current_trigger->angle.xyz.z = fl_radians(current_trigger->angle.xyz.z);
						}else{
							current_trigger->absolute = false;
							required_string("+relative_angle:");
							stuff_vec3d(&current_trigger->angle );
		
							current_trigger->angle.xyz.x = fl_radians(current_trigger->angle.xyz.x);
							current_trigger->angle.xyz.y = fl_radians(current_trigger->angle.xyz.y);
							current_trigger->angle.xyz.z = fl_radians(current_trigger->angle.xyz.z);
						}
		
						required_string("+velocity:");
						stuff_vec3d(&current_trigger->vel );
						current_trigger->vel.xyz.x = fl_radians(current_trigger->vel.xyz.x);
						current_trigger->vel.xyz.y = fl_radians(current_trigger->vel.xyz.y);
						current_trigger->vel.xyz.z = fl_radians(current_trigger->vel.xyz.z);
		
						if (optional_string("+acceleration:")){
							stuff_vec3d(&current_trigger->accel );
							current_trigger->accel.xyz.x = fl_radians(current_trigger->accel.xyz.x);
							current_trigger->accel.xyz.y = fl_radians(current_trigger->accel.xyz.y);
							current_trigger->accel.xyz.z = fl_radians(current_trigger->accel.xyz.z);
						} else {
							current_trigger->accel.xyz.x = 0.0f;
							current_trigger->accel.xyz.y = 0.0f;
							current_trigger->accel.xyz.z = 0.0f;
						}

						if(optional_string("+time:"))
							stuff_int(&current_trigger->end );
						else
							current_trigger->end = 0;

						if(optional_string("$Sound:")){
							parse_sound("+Start:", &current_trigger->start_sound, sip->name);

							parse_sound("+Loop:", &current_trigger->loop_sound, sip->name);

							parse_sound("+End:", &current_trigger->end_sound, sip->name);

							required_string("+Radius:");
							stuff_float(&current_trigger->snd_rad );
						}else{
							current_trigger->start_sound = -1;
							current_trigger->loop_sound = -1;
							current_trigger->end_sound = -1;
							current_trigger->snd_rad = 0;
						}
					}

					//make sure that the amount of time it takes to accelerate up and down doesn't make it go farther than the angle
					queued_animation_correct(current_trigger);
				}
				else if(!stricmp(name_tmp, "linked"))
				{
					mprintf(("TODO: set up linked animation\n"));
				}
			}

		}
		break;
		case 2:
			cont_flag = 0;
			break;
		case -1:	// Possible return value if -noparseerrors is used
			break;
		default:
			Assertion(false, "This should never happen.\n");	// Impossible return value from required_string_one_of.
		}
	}	

	// must be > 0//no it doesn't :P -Bobboau
	// yes it does! - Goober5000
	// (we don't want a div-0 error)
	if (hull_percentage_of_hits <= 0.0f )
	{
		//Warning(LOCATION, "The subsystems defined for the %s can take more (or the same) combined damage than the ship itself. Adjust the tables so that the percentages add up to less than 100", sip->name);
	}
	// when done reading subsystems, malloc and copy the subsystem data to the ship info structure
	int orig_n_subsystems = sip->n_subsystems;
	if ( n_subsystems > 0 ) {
		if(sip->n_subsystems < 1) {
			sip->n_subsystems = n_subsystems;
			sip->subsystems = (model_subsystem *)vm_malloc(sizeof(model_subsystem) * sip->n_subsystems );
		} else {
			sip->n_subsystems += n_subsystems;
			sip->subsystems = (model_subsystem *)vm_realloc(sip->subsystems, sizeof(model_subsystem) * sip->n_subsystems);
		} 
		Assert( sip->subsystems != NULL );
		
		for ( i = 0; i < n_subsystems; i++ ){
			sip->subsystems[orig_n_subsystems+i] = subsystems[i];
		}
	}

	model_anim_fix_reverse_times(sip);

	return rtn;	//0 for success
}

engine_wash_info *get_engine_wash_pointer(char *engine_wash_name)
{
	for(int i = 0; i < Num_engine_wash_types; i++)
	{
		if(!stricmp(engine_wash_name, Engine_wash_info[i].name))
		{
			return &Engine_wash_info[i];
		}
	}

	//Didn't find anything.
	return NULL;
}

void parse_ship_type()
{
	char name_buf[NAME_LENGTH];
	bool nocreate = false;
	ship_type_info stp_buf, *stp = NULL;

	required_string("$Name:");
	stuff_string(name_buf, F_NAME, NAME_LENGTH);

	if(optional_string("+nocreate")) {
		nocreate = true;
	}

	int idx = ship_type_name_lookup(name_buf);
	if (idx >= 0)
	{
		stp = &Ship_types[idx];
	}
	else
	{
		stp = &stp_buf;
		strcpy_s(stp->name, name_buf);
	}

	char *ship_type = NULL;
	if (!stricmp(stp->name, "sentrygun")) {
		ship_type = "sentry gun";
	} else if (!stricmp(stp->name, "escapepod")) {
		ship_type = "escape pod";
	} else if (!stricmp(stp->name, "repair_rearm")) {
		ship_type = "support";
	} else if (!stricmp(stp->name, "supercap")) {
		ship_type = "super cap";
	} else if (!stricmp(stp->name, "knossos")) {
		ship_type = "knossos device";
	}

	if (ship_type != NULL) {
		Warning(LOCATION, "Bad ship type name in objecttypes.tbl\n\nUsed ship type is redirected to another ship type.\nReplace \"%s\" with \"%s\"\nin objecttypes.tbl to fix this.\n", stp->name, ship_type);
	}

	//Okay, now we should have the values to parse
	//But they aren't here!! :O
	//Now they are!! Whee fogging!!

	//AI turret targeting priority setup
	if (optional_string("$Target Priority Groups:") ) {
		SCP_vector <SCP_string> target_group_strings;
		int num_strings = stuff_string_list(target_group_strings);
		int num_groups = Ai_tp_list.size();
		int i, j;
		bool override_strings = false;

		if (optional_string("+Override")) {
			override_strings = true;
		}

		for(j = 0; j < num_strings; j++) {
			for(i = 0; i < num_groups; i++) {
				if ( !stricmp(target_group_strings[j].c_str(), Ai_tp_list[i].name) ) {
					//so now the string from the list above as well as the ai priority group name match
					//clear it if override has been set
					if (override_strings) {
						Ai_tp_list[i].ship_type.clear();
						override_strings = false;
					}
					//find the index number of the current ship info type
					Ai_tp_list[i].ship_type.push_back(ship_type_name_lookup(name_buf));
					break;
				}
			}
			if (i == num_groups) {
				Warning(LOCATION,"Unidentified priority group '%s' set for objecttype '%s'\n", target_group_strings[j].c_str(), stp->name);
			}
		}
	}

	if(optional_string("$Counts for Alone:")) {
		stuff_boolean_flag(&stp->message_bools, STI_MSG_COUNTS_FOR_ALONE);
	}

	if(optional_string("$Praise Destruction:")) {
		stuff_boolean_flag(&stp->message_bools, STI_MSG_PRAISE_DESTRUCTION);
	}

	if(optional_string("$On Hotkey list:")) {
		stuff_boolean_flag(&stp->hud_bools, STI_HUD_HOTKEY_ON_LIST);
	}

	if(optional_string("$Target as Threat:")) {
		stuff_boolean_flag(&stp->hud_bools, STI_HUD_TARGET_AS_THREAT);
	}

	if(optional_string("$Show Attack Direction:")) {
		stuff_boolean_flag(&stp->hud_bools, STI_HUD_SHOW_ATTACK_DIRECTION);
	}

	if(optional_string("$Scannable:")) {
		stuff_boolean_flag(&stp->ship_bools, STI_SHIP_SCANNABLE);
	}

	if(optional_string("$Warp Pushes:")) {
		stuff_boolean_flag(&stp->ship_bools, STI_SHIP_WARP_PUSHES);
	}

	if(optional_string("$Warp Pushable:")) {
		stuff_boolean_flag(&stp->ship_bools, STI_SHIP_WARP_PUSHABLE);
	}

	if(optional_string("$Turrets prioritize ship target:")) {
		stuff_boolean_flag(&stp->ship_bools, STI_TURRET_TGT_SHIP_TGT);
	}

	if(optional_string("$Max Debris Speed:")) {
		stuff_float(&stp->debris_max_speed);
	}

	if(optional_string("$FF Multiplier:")) {
		stuff_float(&stp->ff_multiplier);
	}

	if(optional_string("$EMP Multiplier:")) {
		stuff_float(&stp->emp_multiplier);
	}

	if(optional_string("$Beams Easily Hit:")) {
		stuff_boolean_flag(&stp->weapon_bools, STI_WEAP_BEAMS_EASILY_HIT);
	}

	if(optional_string("$Protected on cripple:")) {
		stuff_boolean_flag(&stp->ai_bools, STI_AI_PROTECTED_ON_CRIPPLE);
	}

	if(optional_string("$No Huge Beam Impact Effects:")) {
		stuff_boolean_flag(&stp->weapon_bools, STI_WEAP_NO_HUGE_IMPACT_EFF);
	}

	if(optional_string("$Don't display class in briefing:")) {
		stuff_boolean_flag(&stp->hud_bools, STI_HUD_NO_CLASS_DISPLAY);
	}

	if(optional_string("$Fog:"))
	{
		if(optional_string("+Start dist:")) {
			stuff_float(&stp->fog_start_dist);
		}

		if(optional_string("+Compl dist:")) {
			stuff_float(&stp->fog_complete_dist);
		}
	}

	if(optional_string("$AI:"))
	{
		if(optional_string("+Valid goals:")) {
			parse_string_flag_list(&stp->ai_valid_goals, Ai_goal_names, Num_ai_goals);
		}

		if(optional_string("+Accept Player Orders:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_ACCEPT_PLAYER_ORDERS);
		}

		if(optional_string("+Player Orders:")) {
			parse_string_flag_list(&stp->ai_player_orders, Player_orders, Num_player_orders);
		}

		if(optional_string("+Auto attacks:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_AUTO_ATTACKS);
		}

		if(optional_string("+Attempt broadside:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_ATTEMPT_BROADSIDE);
		}

		if(optional_string("+Actively Pursues:")) {
			stuff_string_list(stp->ai_actively_pursues_temp);
		}

		if(optional_string("+Guards attack this:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_GUARDS_ATTACK);
		}

		if(optional_string("+Turrets attack this:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_TURRETS_ATTACK);
		}

		if(optional_string("+Can form wing:")) {
			stuff_boolean_flag(&stp->ai_bools, STI_AI_CAN_FORM_WING);
		}

		if(optional_string("+Active docks:")) {
			parse_string_flag_list(&stp->ai_active_dock, Dock_type_names, Num_dock_type_names);
		}

		if(optional_string("+Passive docks:")) {
			parse_string_flag_list(&stp->ai_passive_dock, Dock_type_names, Num_dock_type_names);
		}

		if(optional_string("+Ignored on cripple by:")) {
			stuff_string_list(stp->ai_cripple_ignores_temp); 
		}
	}

	if(optional_string("$Explosion Animations:"))
	{
		int temp[MAX_FIREBALL_TYPES];
		int parsed_ints = stuff_int_list(temp, MAX_FIREBALL_TYPES, RAW_INTEGER_TYPE);
		stp->explosion_bitmap_anims.clear();
		stp->explosion_bitmap_anims.insert(stp->explosion_bitmap_anims.begin(), temp, temp+parsed_ints);
	}

	if(optional_string("$Vaporize Percent Chance:")) {
		stuff_float(&stp->vaporize_chance);
		if (stp->vaporize_chance < 0.0f || stp->vaporize_chance > 100.0f) {
			stp->vaporize_chance = 0.0f;
			Warning(LOCATION, "$Vaporize Percent Chance should be between 0 and 100.0 (read %f). Setting to 0.", stp->vaporize_chance);
		}
		//Percent is nice for modders, but here in the code we want it betwwen 0 and 1.0
		stp->vaporize_chance /= 100.0;
	}

	if (!nocreate)
		Ship_types.push_back(stp_buf);
}

void parse_shiptype_tbl(const char *filename)
{
	try
	{
		if (filename != NULL)
			read_file_text(filename, CF_TYPE_TABLES);
		else
			read_file_text_from_array(defaults_get_file("objecttypes.tbl"));

		reset_parse();

		if (optional_string("#Target Priorities"))
		{
			while (required_string_either("#End", "$Name:"))
				parse_ai_target_priorities();

			required_string("#End");
		}

		if (optional_string("#Weapon Targeting Priorities"))
		{
			while (required_string_either("#End", "$Name:"))
				parse_weapon_targeting_priorities();

			required_string("#End");
		}

		if (optional_string("#Ship Types"))
		{
			while (required_string_either("#End", "$Name:"))
				parse_ship_type();

			required_string("#End");
		}

		// add tbl/tbm to multiplayer validation list
		fs2netd_add_table_validation(filename);
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

// The E - Simple lookup function for FRED.
int get_default_player_ship_index() 
{
	if (strlen(default_player_ship)) 
	{
		for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) 
		{
			if (stricmp(default_player_ship, it->name) == 0)
				return std::distance(Ship_info.cbegin(), it);
		}
		return 0;
	} else
		return 0;
}

// Goober5000 - this works better in its own function
void ship_set_default_player_ship()
{
	// already have one
	if(strlen(default_player_ship))
		return;

	// find the first with the default flag
	for(auto it = Ship_info.cbegin(); it != Ship_info.end(); ++it)
	{
		if(it->flags & SIF_DEFAULT_PLAYER_SHIP)
		{
			strcpy_s(default_player_ship, it->name);
			return;
		}
	}

	// find the first player ship
	for(auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it)
	{
		if(it->flags & SIF_PLAYER_SHIP)
		{
			strcpy_s(default_player_ship, it->name);
			return;
		}
	}

	// find the first ship
	if(!Ship_info.empty())
	{
		strcpy_s(default_player_ship, Ship_info[0].name);
	}
}

void parse_shiptbl(const char *filename)
{
	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		// parse default ship
		//Override default player ship
		if (optional_string("#Default Player Ship"))
		{
			required_string("$Name:");
			stuff_string(default_player_ship, F_NAME, sizeof(default_player_ship));
			required_string("#End");
		}
		//Add engine washes
		//This will override if they already exist
		if (optional_string("#Engine Wash Info"))
		{
			while (required_string_either("#End", "$Name:"))
			{
				parse_engine_wash(Parsing_modular_table);
			}

			required_string("#End");
		}

		if ( optional_string("#Ship Templates") ) {

			while ( required_string_either("#End", "$Template:") ) {
				if ( parse_ship_template() ) {
					continue;
				}
			}

			required_string("#End");
		}

		//Add ship classes
		if(optional_string("#Ship Classes"))
		{

			while (required_string_either("#End", "$Name:"))
			{
				if (parse_ship(filename, Parsing_modular_table)) {
					continue;
				}
			}

			required_string("#End");
		}

		//Set default player ship
		ship_set_default_player_ship();

		// add tbl/tbm to multiplayer validation list
		fs2netd_add_table_validation(filename);
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

int ship_show_velocity_dot = 0;


DCF_BOOL( show_velocity_dot, ship_show_velocity_dot )

/**
 * Clean up ship entries, making sure various flags and settings are correct
 */
void ship_parse_post_cleanup()
{
	int i, j;
	char name_tmp[NAME_LENGTH];

	for (auto sip = Ship_info.begin(); sip != Ship_info.end(); ++sip)
	{
		// ballistic primary fixage...
		{
			bool pbank_capacity_specified = false;

			// determine whether this ship had primary capacities specified for it
			for (j = 0; j < sip->num_primary_banks; j++) {
				if (sip->primary_bank_ammo_capacity[j] > 0) {
					pbank_capacity_specified = true;
					break;
				}
			}

			// be friendly; ensure ballistic flags check out
			if (pbank_capacity_specified) {
				if ( !(sip->flags & SIF_BALLISTIC_PRIMARIES) ) {
					Warning(LOCATION, "Pbank capacity specified for non-ballistic-primary-enabled ship %s.\nResetting capacities to 0.\nTo fix this, add a ballistic primary to the list of allowed primaries.\n", sip->name);

					for (j = 0; j < MAX_SHIP_PRIMARY_BANKS; j++) {
						sip->primary_bank_ammo_capacity[j] = 0;
					}
				}
			} else {
				if (sip->flags & SIF_BALLISTIC_PRIMARIES) {
					Warning(LOCATION, "Pbank capacity not specified for ballistic-primary-enabled ship %s.\nDefaulting to capacity of 1 per bank.\n", sip->name);

					for (j = 0; j < MAX_SHIP_PRIMARY_BANKS; j++) {
						sip->primary_bank_ammo_capacity[j] = 1;
					}
				}
			}
		}

		// ultra stupid compatbility handling for the once broken "generate hud" flag.
		// it was previously testing the afterburner flag, so that's what we check for that
		if ( (sip->shield_icon_index == 255) && (sip->flags & SIF_AFTERBURNER)
				&& !(sip->flags2 & SIF2_GENERATE_HUD_ICON) && (sip->flags & SIF_PLAYER_SHIP) )
		{
			Warning(LOCATION, "Compatibility warning:\nNo shield icon specified for '%s' but the \"generate icon\" flag is not specified.\nEnabling flag by default.\n", sip->name);
			sip->flags2 |= SIF2_GENERATE_HUD_ICON;
		}

		// if we have a ship copy, then check to be sure that our base ship exists
		if (sip->flags & SIF_SHIP_COPY)
		{
			strcpy_s(name_tmp, sip->name);

			if (end_string_at_first_hash_symbol(name_tmp))
			{
				if (ship_info_lookup(name_tmp) < 0)
				{
					Warning(LOCATION, "Ship %s is a copy, but base ship %s couldn't be found.", sip->name, name_tmp);
					sip->flags &= ~SIF_SHIP_COPY;
				}
			}
			else
			{
				Warning(LOCATION, "Ship %s is defined as a copy (ship flag 'ship copy' is set), but is not named like one (no '#').\n", sip->name);
				sip->flags &= ~SIF_SHIP_COPY;
			}
		}

		// very low rotational velocity values disable rotational collisions
		// warn early rather than ambush the modder @ runtime (unless the ship is also no-collide!)
		// the 2nd part of this check is duplicated from collideshipship.cpp:ship_ship_check_collision()
		if (!(sip->flags & SIF_NO_COLLIDE) && (vm_vec_mag_squared( &sip->max_rotvel ) * .04) >= (PI*PI/4))
		{
			Warning(LOCATION, "$Rotation time: too low; this will disable rotational collisions. All three variables should be >= 1.39.\nFix this in ship '%s'\n", sip->name);
		}
	}

	// check also target groups here
	int n_tgt_groups = Ai_tp_list.size();

	if (n_tgt_groups > 0) {
		for(i = 0; i < n_tgt_groups; i++) {
			if (!(Ai_tp_list[i].obj_flags || Ai_tp_list[i].sif_flags || Ai_tp_list[i].sif2_flags || Ai_tp_list[i].wif2_flags || Ai_tp_list[i].wif_flags)) {
				//had none of these, check next
				if (Ai_tp_list[i].obj_type == -1) {
					//didn't have this one
					if (!(Ai_tp_list[i].ship_class.size() || Ai_tp_list[i].ship_type.size() || Ai_tp_list[i].weapon_class.size())) {
						// had nothing - time to issue a warning
						Warning(LOCATION, "Target priority group '%s' had no targeting rules issued for it.\n", Ai_tp_list[i].name);
					}
				}
			}
		}
	}

	// Clear out ship templates, since they're no longer needed. -MageKing17
	Ship_templates.clear();
}

/**
 * Called once at the beginning of the game to parse ships.tbl and stuff the ::Ship_info
 * vector
 */
void ship_init()
{
	if ( !ships_inited )
	{
		//Parse main TBL first
		if (cf_exists_full("objecttypes.tbl", CF_TYPE_TABLES))
			parse_shiptype_tbl("objecttypes.tbl");
		else
			parse_shiptype_tbl(NULL);

		//Then other ones
		parse_modular_table(NOX("*-obt.tbm"), parse_shiptype_tbl);

		// DO ALL THE STUFF WE NEED TO DO AFTER LOADING Ship_types
		ship_type_info *stp;

		uint i,j;
		int idx;
		for(i = 0; i < Ship_types.size(); i++)
		{
			stp = &Ship_types[i];

			//Handle active pursuit
			for(j = 0; j < stp->ai_actively_pursues_temp.size(); j++)
			{
				idx = ship_type_name_lookup((char*)stp->ai_actively_pursues_temp[j].c_str());
				if(idx >= 0) {
					stp->ai_actively_pursues.push_back(idx);
				}
			}
			stp->ai_actively_pursues_temp.clear();

			//Handle disabled/disarmed behaviour
			for(j = 0; j < stp->ai_cripple_ignores_temp.size(); j++) {
				idx = ship_type_name_lookup((char*)stp->ai_cripple_ignores_temp[j].c_str());
				if(idx >= 0) {
					stp->ai_cripple_ignores.push_back(idx);
				}
			}
			stp->ai_cripple_ignores_temp.clear();
		}

		//ships.tbl
		{			
			Num_engine_wash_types = 0;
			strcpy_s(default_player_ship, "");

			//Parse main TBL first
			parse_shiptbl("ships.tbl");

			//Then other ones
			parse_modular_table(NOX("*-shp.tbm"), parse_shiptbl);

			ship_parse_post_cleanup();

			ships_inited = 1;
		}

		// NULL out "dynamic" subsystem ptr's
		for (i = 0; i < NUM_SHIP_SUBSYSTEM_SETS; i++)
			Ship_subsystems[i] = NULL;
	}

	ship_level_init();	// needed for FRED
}

int Man_thruster_reset_timestamp = 0;

static void ship_clear_subsystems()
{
	int i;

	for (i = 0; i < NUM_SHIP_SUBSYSTEM_SETS; i++) {
		if (Ship_subsystems[i] != NULL) {
			delete[] Ship_subsystems[i];
			Ship_subsystems[i] = NULL;
		}
	}

	Num_ship_subsystems = 0;
	Num_ship_subsystems_allocated = 0;

	Triggered_rotations.clear();
}

static int ship_allocate_subsystems(int num_so, bool page_in = false)
{
	int idx, i;
	int num_subsystems_save = 0;

	// "0" itself is safe
	if (num_so < 0) {
		Int3();
		return 0;
	}

	// allow a page-in thingy, so that we can grab as much as possible before mission
	// start, but without messing up our count for future things
	if (page_in)
		num_subsystems_save = Num_ship_subsystems;

	Num_ship_subsystems += num_so;

	// bail if we don't actually need any more
	if ( Num_ship_subsystems < Num_ship_subsystems_allocated )
		return 1;

	mprintf(("Allocating space for at least %i new ship subsystems ... ", num_so));

	// we might need more than one set worth of new subsystems, so make as many as required
	do {
		for (idx = 0; idx < NUM_SHIP_SUBSYSTEM_SETS; idx++) {
			if (Ship_subsystems[idx] == NULL)
				break;
		}

		// safety check, but even if we have this here it will fubar something else later, so we're screwed either way
		if (idx == NUM_SHIP_SUBSYSTEM_SETS) {
			return 0;
		}

		Ship_subsystems[idx] = new ship_subsys[NUM_SHIP_SUBSYSTEMS_PER_SET];

		// append the new set to our free list
		for (i = 0; i < NUM_SHIP_SUBSYSTEMS_PER_SET; i++)
			list_append( &ship_subsys_free_list, &Ship_subsystems[idx][i] );

		Num_ship_subsystems_allocated += NUM_SHIP_SUBSYSTEMS_PER_SET;
	} while ( (Num_ship_subsystems - Num_ship_subsystems_allocated) > 0 );

	if (page_in)
		Num_ship_subsystems = num_subsystems_save;

	mprintf((" a total of %i is now available (%i in-use).\n", Num_ship_subsystems_allocated, Num_ship_subsystems));
	return 1;
}

/**
 * This will get called at the start of each level.
 */
void ship_level_init()
{
	int i;

	// Reset everything between levels
	Ships_exited.clear(); 
	Ships_exited.reserve(100);
	for (i=0; i<MAX_SHIPS; i++ )
	{
		Ships[i].ship_name[0] = '\0';
		Ships[i].objnum = -1;
	}

	Num_wings = 0;
	for (i = 0; i < MAX_WINGS; i++ )
	{
		Wings[i].num_waves = -1;
		Wings[i].wing_squad_filename[0] = '\0';
		Wings[i].wing_insignia_texture = -1;	// Goober5000 - default to no wing insignia
												// don't worry about releasing textures because
												// they are released automatically when the model
												// is unloaded (because they are part of the model)
	}

	for (i=0; i<MAX_STARTING_WINGS; i++)
		Starting_wings[i] = -1;

	for (i=0; i<MAX_SQUADRON_WINGS; i++)
		Squadron_wings[i] = -1;

	for (i=0; i<MAX_TVT_WINGS; i++)
		TVT_wings[i] = -1;

	// Goober5000

	// set starting wing names to default
	strcpy_s(Starting_wing_names[0], "Alpha");
	strcpy_s(Starting_wing_names[1], "Beta");
	strcpy_s(Starting_wing_names[2], "Gamma");

	// set squadron wing names to default
	strcpy_s(Squadron_wing_names[0], "Alpha");
	strcpy_s(Squadron_wing_names[1], "Beta");
	strcpy_s(Squadron_wing_names[2], "Gamma");
	strcpy_s(Squadron_wing_names[3], "Delta");
	strcpy_s(Squadron_wing_names[4], "Epsilon");

	// set tvt wing names to default
	strcpy_s(TVT_wing_names[0], "Alpha");
	strcpy_s(TVT_wing_names[1], "Zeta");

	// Empty the subsys list
	ship_clear_subsystems();
	list_init( &ship_subsys_free_list );

	Laser_energy_out_snd_timer = 1;
	Missile_out_snd_timer		= 1;

	ship_obj_list_init();

	Ship_cargo_check_timer = 1;

	shipfx_large_blowup_level_init();

	Man_thruster_reset_timestamp = timestamp(0);
}

/**
 * Add a ship onto the exited ships list.
 *
 * The reason parameter tells us why the ship left the mission (i.e. departed or destroyed)
 */
void ship_add_exited_ship( ship *sp, int reason )
{
	exited_ship entry; 

	strcpy_s(entry.ship_name, sp->ship_name );
	entry.obj_signature = Objects[sp->objnum].signature;
	entry.ship_class = sp->ship_info_index;
	entry.team = sp->team;
	entry.flags = reason;
	// if ship is red alert, flag as such
	if (sp->flags & SF_RED_ALERT_STORE_STATUS) {
		entry.flags |= SEF_RED_ALERT_CARRY;
	}
	entry.time = Missiontime;
	entry.hull_strength = int(Objects[sp->objnum].hull_strength);

	entry.cargo1 = sp->cargo1;

	entry.time_cargo_revealed = (fix)0;
	if ( sp->flags & SF_CARGO_REVEALED )
	{
		entry.flags |= SEF_CARGO_KNOWN;
		entry.time_cargo_revealed = sp->time_cargo_revealed;
	}

	if ( sp->time_first_tagged > 0 )
		entry.flags |= SEF_BEEN_TAGGED;
	
	//copy across the damage_ship arrays
	for (int i = 0; i < MAX_DAMAGE_SLOTS ; i++) {
		entry.damage_ship_id[i] = sp->damage_ship_id[i] ;
		entry.damage_ship[i] = sp->damage_ship[i] ;
	}
	
	Ships_exited.push_back(entry);
}

/**
 * Attempt to find information about an exited ship based on shipname
 */
int ship_find_exited_ship_by_name( char *name )
{
	int i;

	for (i = 0; i < (int)Ships_exited.size(); i++) {
		if ( !stricmp(name, Ships_exited[i].ship_name) )
			return i;
	}

	return -1;
}

/**
 * Attempt to find information about an exited ship based on signature
 */
int ship_find_exited_ship_by_signature( int signature )
{
	int i;

	for (i = 0; i < (int)Ships_exited.size(); i++) {
		if ( signature == Ships_exited[i].obj_signature )
			return i;
	}

	return -1;
}


void physics_ship_init(object *objp)
{
	ship_info	*sinfo = &Ship_info[Ships[objp->instance].ship_info_index];
	physics_info	*pi = &objp->phys_info;
	polymodel *pm = model_get(sinfo->model_num);

	// use mass and I_body_inv from POF read into polymodel
	physics_init(pi);

	pi->mass = pm->mass * sinfo->density;
	if (pi->mass==0.0f)
	{
		vec3d size;
		vm_vec_sub(&size,&pm->maxs,&pm->mins);
		float vmass=size.xyz.x*size.xyz.y*size.xyz.z;
		float amass=4.65f*(float)pow(vmass,(2.0f/3.0f));

		nprintf(("Physics", "pi->mass==0.0f. setting to %f\n",amass));
		Warning(LOCATION, "%s (%s) has no mass! setting to %f", sinfo->name, sinfo->pof_file, amass);
		pm->mass=amass;
		pi->mass=amass*sinfo->density;
	}

	// ack!
	// if pm's MOI is invalid, compensate
	if ( IS_VEC_NULL(&pm->moment_of_inertia.vec.rvec)
		&& IS_VEC_NULL(&pm->moment_of_inertia.vec.uvec)
		&& IS_VEC_NULL(&pm->moment_of_inertia.vec.fvec) )
	{
		nprintf(("Physics", "pm->moment_of_inertia is invalid for %s!\n", pm->filename));
		Warning(LOCATION, "%s (%s) has a null moment of inertia!", sinfo->name, sinfo->pof_file);

		// TODO: generate MOI properly
		pi->I_body_inv = pm->moment_of_inertia;
	}
	// it's valid, so we can use it
	else
		pi->I_body_inv = pm->moment_of_inertia;

	// scale pm->I_body_inv value by density
	vm_vec_scale( &pi->I_body_inv.vec.rvec, sinfo->density );
	vm_vec_scale( &pi->I_body_inv.vec.uvec, sinfo->density );
	vm_vec_scale( &pi->I_body_inv.vec.fvec, sinfo->density );

	pi->center_of_mass = pm->center_of_mass;
	pi->side_slip_time_const = sinfo->damp;
	pi->delta_bank_const = sinfo->delta_bank_const;
	pi->rotdamp = sinfo->rotdamp;
	pi->max_vel = sinfo->max_vel;
	pi->afterburner_max_vel = sinfo->afterburner_max_vel;
	pi->max_rotvel = sinfo->max_rotvel;
	pi->max_rear_vel = sinfo->max_rear_vel;
	pi->flags |= PF_ACCELERATES;	
	pi->flags &= ~PF_GLIDING; //Turn off glide
	pi->flags &= ~PF_FORCE_GLIDE;

	pi->forward_accel_time_const=sinfo->forward_accel;
	pi->afterburner_forward_accel_time_const=sinfo->afterburner_forward_accel;
	pi->forward_decel_time_const=sinfo->forward_decel;
	pi->slide_accel_time_const=sinfo->slide_accel;
	pi->slide_decel_time_const=sinfo->slide_decel;

	if ( (pi->max_vel.xyz.x > 0.000001f) || (pi->max_vel.xyz.y > 0.000001f) )
		pi->flags |= PF_SLIDE_ENABLED;

	pi->cur_glide_cap = pi->max_vel.xyz.z; //Init dynamic glide cap stuff to the max vel.
	if ( sinfo->glide_cap > 0.000001f || sinfo->glide_cap < -0.000001f )		//Backslash
		pi->glide_cap = sinfo->glide_cap;
	else
		pi->glide_cap = MAX(MAX(pi->max_vel.xyz.z, sinfo->max_overclocked_speed), pi->afterburner_max_vel.xyz.z);
	// If there's not a value for +Max Glide Speed set in the table, we want this cap to default to the fastest speed the ship can go.
	// However, a negative value means we want no cap, thus allowing nearly infinite maximum gliding speeds.

	//SUSHI: If we are using dynamic glide capping, force the glide cap to 0 (understood by physics.cpp to mean the cap should be dynamic)
	if (sinfo->glide_dynamic_cap)
		pi->glide_cap = 0;

	pi->glide_accel_mult = sinfo->glide_accel_mult;

	//SUSHI: This defaults to the AI_Profile value, and is only optionally overridden
	pi->use_newtonian_damp = ((The_mission.ai_profile->flags & AIPF_USE_NEWTONIAN_DAMPENING) != 0);
	if (sinfo->newtonian_damp_override)
		pi->use_newtonian_damp = sinfo->use_newtonian_damp;

	vm_vec_zero(&pi->vel);
	vm_vec_zero(&pi->rotvel);
	pi->speed = 0.0f;
	pi->heading = 0.0f;
	vm_set_identity(&pi->last_rotmat);

	//SparK: setting the reverse burners
	pi->afterburner_max_reverse_vel = sinfo->afterburner_max_reverse_vel;
	pi->afterburner_reverse_accel = sinfo->afterburner_reverse_accel;
}

/**
 * Get the type of the given ship as a string
 */
int ship_get_type(char* output, ship_info *sip)
{
	if(sip->class_type < 0) {
		strcpy(output, "Unknown");
		return 0;
	}

	strcpy(output, Ship_types[sip->class_type].name);
	return 1;
}

/**
 * Set the orders allowed for a ship -- based on ship type.  
 *
 * This value might get overridden by a value in the mission file.
 */
int ship_get_default_orders_accepted( ship_info *sip )
{
	if(sip->class_type >= 0) {
		return Ship_types[sip->class_type].ai_player_orders;
	} else {
		return 0;
	}
}

vec3d get_submodel_offset(int model, int submodel){
	polymodel*pm = model_get(model);
	if(pm->submodel[submodel].parent == -1)
		return pm->submodel[submodel].offset;
	vec3d ret = pm->submodel[submodel].offset;
	vec3d v = get_submodel_offset(model,pm->submodel[submodel].parent);
	vm_vec_add2(&ret, &v);
	return ret;

}

void ship_set_warp_effects(object *objp, ship_info *sip)
{
	ship *shipp = &Ships[objp->instance];

	if(shipp->warpin_effect != NULL)
		delete shipp->warpin_effect;

	switch(sip->warpin_type)
	{
		case WT_DEFAULT:
		case WT_KNOSSOS:
		case WT_DEFAULT_THEN_KNOSSOS:
			shipp->warpin_effect = new WE_Default(objp, WD_WARP_IN);
			break;
		case WT_IN_PLACE_ANIM:
			shipp->warpin_effect = new WE_BSG(objp, WD_WARP_IN);
			break;
		case WT_SWEEPER:
			shipp->warpin_effect = new WE_Homeworld(objp, WD_WARP_IN);
			break;
		case WT_HYPERSPACE:
			shipp->warpin_effect = new WE_Hyperspace(objp, WD_WARP_IN);
			break;
		default:
			shipp->warpin_effect = new WarpEffect();
	}

	if(shipp->warpout_effect != NULL)
		delete shipp->warpout_effect;

	switch(sip->warpout_type)
	{
		case WT_DEFAULT:
		case WT_KNOSSOS:
		case WT_DEFAULT_THEN_KNOSSOS:
			shipp->warpout_effect = new WE_Default(objp, WD_WARP_OUT);
			break;
		case WT_IN_PLACE_ANIM:
			shipp->warpout_effect = new WE_BSG(objp, WD_WARP_OUT);
			break;
		case WT_SWEEPER:
			shipp->warpout_effect = new WE_Homeworld(objp, WD_WARP_OUT);
			break;
		case WT_HYPERSPACE:
			shipp->warpout_effect = new WE_Hyperspace(objp, WD_WARP_OUT);
			break;
		default:
			shipp->warpout_effect = new WarpEffect();
	}
}

// Reset all ship values to empty/unused.
void ship::clear()
{
	int i, j;

	objnum = -1;
	ai_index = -1;
	ship_info_index = -1;
	hotkey = -1;
	escort_priority = 0;
	score = 0;
	assist_score_pct = 0.0f;
	respawn_priority = 0;

	pre_death_explosion_happened = 0;
	wash_killed = 0;	// serenity lies
	cargo1 = 0;							// "Nothing"

	wing_status_wing_index = -1;
	wing_status_wing_pos = -1;

	alt_type_index = -1;
	callsign_index = -1;

	targeting_laser_bank = -1;
	targeting_laser_objnum = -1;

	num_corkscrew_to_fire = 0;
	corkscrew_missile_bank = -1;
	next_corkscrew_fire = timestamp(0);

	final_death_time = timestamp(-1);
	death_time = timestamp(-1);
	end_death_time = timestamp(-1);
	really_final_death_time = timestamp(-1);
	deathroll_rotvel = vmd_zero_vector;

	if (warpin_effect != NULL)
		delete warpin_effect;
	if (warpout_effect != NULL)
		delete warpout_effect;
	warpin_effect = NULL;
	warpout_effect = NULL;

	next_fireball = timestamp(-1);
	next_hit_spark = timestamp(-1);
	num_hits = 0;
	memset(sparks, 0, MAX_SHIP_HITS * sizeof(ship_spark));

	use_special_explosion = false;
	special_exp_damage = 0;
	special_exp_blast = 0;
	special_exp_inner = 0;
	special_exp_outer = 0;
	use_shockwave = false;
	special_exp_shockwave_speed = 0;
	special_exp_deathroll_time = 0;

	special_hitpoints = 0;
	special_shield = -1;

	shield_points.clear();

	ship_max_shield_strength = 0.0f;
	ship_max_hull_strength = 0.0f;

	ship_guardian_threshold = 0;

	ship_name[0] = 0;
	team = 0;

	time_cargo_revealed = 0;

	arrival_location = 0;
	arrival_distance = 0;
	arrival_anchor = -1;
	arrival_path_mask = 0;
	arrival_cue = -1;
	arrival_delay = 0;

	departure_location = 0;
	departure_anchor = -1;
	departure_path_mask = 0;
	departure_cue = -1;
	departure_delay = 0;

	wingnum = -1;
	orders_accepted = 0;

	subsys_list.clear();
	// since these aren't cleared by clear()
	subsys_list.next = NULL;
	subsys_list.prev = NULL;

	memset(&subsys_info, 0, SUBSYSTEM_MAX * sizeof(ship_subsys_info));

	memset(last_targeted_subobject, 0, MAX_PLAYERS * sizeof(ship_subsys *));

	shield_integrity = NULL;

	shield_recharge_index = INTIAL_SHIELD_RECHARGE_INDEX;
	weapon_recharge_index = INTIAL_WEAPON_RECHARGE_INDEX;
	engine_recharge_index = INTIAL_ENGINE_RECHARGE_INDEX;
	weapon_energy = 0;
	current_max_speed = 0.0f;
	next_manage_ets = timestamp(0);

	flags = 0;
	flags2 = 0;
	reinforcement_index = -1;
	
	afterburner_fuel = 0.0f;

	cmeasure_count = 0;
	current_cmeasure = -1;

	cmeasure_fire_stamp = timestamp(0);

	target_shields_delta = 0.0f;
	target_weapon_energy_delta = 0.0f;

	memset(&weapons, 0, sizeof(ship_weapon));

	// ---------- special weapons init that isn't setting things to 0
	for (i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++)
	{
		weapons.primary_bank_weapons[i] = -1;

		weapons.next_primary_fire_stamp[i] = timestamp(0);
		weapons.last_primary_fire_stamp[i] = timestamp(-1);
		weapons.last_primary_fire_sound_stamp[i] = timestamp(0);

		weapons.primary_bank_slot_count[i] = 1;

		weapons.primary_bank_rearm_time[i] = timestamp(0);
		weapons.primary_animation_done_time[i] = timestamp(0);

		// not part of weapons!
		primary_rotate_rate[i] = 0.0f;
		primary_rotate_ang[i] = 0.0f;
		last_fired_point[i] = 0;
		// for fighter beams
		was_firing_last_frame[i] = 0;
	}

	for (i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++)
	{
		weapons.secondary_bank_weapons[i] = -1;

		weapons.next_secondary_fire_stamp[i] = timestamp(0);
		weapons.last_secondary_fire_stamp[i] = timestamp(-1);

		weapons.secondary_bank_rearm_time[i] = timestamp(0);
		weapons.secondary_animation_done_time[i] = timestamp(0);

		for (j = 0; j < MAX_SLOTS; j++)
			secondary_point_reload_pct[i][j] = 1.0f;
	}

	weapons.next_tertiary_fire_stamp = timestamp(0);

	weapons.last_fired_weapon_index = -1;
	weapons.last_fired_weapon_signature = -1;
	// ---------- done with weapons init

	shield_hits = 0;

	wash_intensity = 0.0f;
	wash_rot_axis = vmd_zero_vector;
	wash_timestamp = timestamp(0);

	num_swarm_missiles_to_fire = 0;
	next_swarm_fire = timestamp(0);
	next_swarm_path = 0;
	num_turret_swarm_info = 0;
	swarm_missile_bank = -1;

	group = -1;
	death_roll_snd = -1;
	ship_list_index = -1;

	thruster_bitmap = -1;
	thruster_frame = 0.0f;

	thruster_glow_bitmap = -1;
	thruster_glow_frame = 0.0f;
	thruster_glow_noise = 1.0f;

	thruster_secondary_glow_bitmap = -1;
	thruster_tertiary_glow_bitmap = -1;
	thruster_distortion_bitmap = -1;

	next_engine_stutter = timestamp(0);

	base_texture_anim_frametime = 0;

	total_damage_received = 0.0f;
	memset(&damage_ship, 0, MAX_DAMAGE_SLOTS * sizeof(float));
	for(i = 0; i < MAX_DAMAGE_SLOTS; i++)
		damage_ship_id[i] = -1;

	persona_index = -1;

	subsys_disrupted_flags = 0;
	subsys_disrupted_check_timestamp = timestamp(0);

	create_time = 0;

	ts_index = -1;

	large_ship_blowup_index = -1;
	for (i = 0; i < NUM_SUB_EXPL_HANDLES; i++)
		sub_expl_sound_handle[i] = -1;

	memset(&arc_pts, 0, MAX_SHIP_ARCS * 2 * sizeof(vec3d));
	for (i = 0; i < MAX_SHIP_ARCS; i++)
		arc_timestamp[i] = timestamp(-1);
	memset(&arc_type, 0, MAX_SHIP_ARCS * sizeof(ubyte));
	arc_next_time = timestamp(-1);

	emp_intensity = -1.0f;
	emp_decr = 0.0f;

	memset(trail_ptr, 0, MAX_SHIP_CONTRAILS * sizeof(trail *));

	tag_total = 0.0f;
	tag_left = -1.0f;
	time_first_tagged = 0;
	level2_tag_total = 0.0f;
	level2_tag_left = -1.0f;

	for (i = 0; i < MAX_PLAYERS; i++ )
	{
		np_updates[i].seq = 0;
		np_updates[i].update_stamp = -1;
		np_updates[i].status_update_stamp = -1;
		np_updates[i].subsys_update_stamp = -1;
		np_updates[i].pos_chksum = 0;
		np_updates[i].orient_chksum = 0;
	}

	lightning_stamp = timestamp(-1);

	// set awacs warning flags so awacs ship only asks for help once at each level
	awacs_warning_flag = AWACS_WARN_NONE;

	special_warpin_objnum = -1;
	special_warpout_objnum = -1;

	fighter_beam_turret_data.clear();
	memset(&beam_sys_info, 0, sizeof(model_subsystem));

	primitive_sensor_range = DEFAULT_SHIP_PRIMITIVE_SENSOR_RANGE;

	ship_replacement_textures = NULL;

	current_viewpoint = -1;

	for (i = 0; i < MAX_SHIP_CONTRAILS; i++)
		ABtrail_ptr[i] = NULL;
	memset(&ab_info, 0, MAX_SHIP_CONTRAILS * sizeof(trail_info));
	ab_count = 0;

	glow_point_bank_active.clear();

	shader_effect_num = 0;
	shader_effect_duration = 0;
	shader_effect_start_time = 0;
	shader_effect_active = false;

	last_fired_turret = NULL;

	bay_doors_anim_done_time = 0;
	bay_doors_status = MA_POS_NOT_SET;
	bay_doors_wanting_open = 0;
	bay_doors_launched_from = 0;
	bay_doors_need_open = false;
	bay_doors_parent_shipnum = -1;

	for(i = 0; i < MAX_MAN_THRUSTERS; i++)
	{
		thrusters_start[i] = timestamp(-1);
		thrusters_sounds[i] = -1;
	}

	s_alt_classes.clear();

	for(i=0;i<MAX_IFFS;i++)
		for(j=0;j<MAX_IFFS;j++)
			ship_iff_color[i][j] = -1;

	ammo_low_complaint_count = 0;

	armor_type_idx = -1;
	shield_armor_type_idx = -1;
	collision_damage_type_idx = -1;
	debris_damage_type_idx = -1;
	debris_net_sig = 0;

	model_instance_num = -1;

	time_created = 0;

	radar_visible_since = -1;
	radar_last_contact = -1;

	radar_last_status = VISIBLE;
	radar_current_status = VISIBLE;

	team_name = "";
	secondary_team_name = "";
	team_change_timestamp = timestamp(-1);
	team_change_time = 0;

	autoaim_fov = 0.0f;
}

// NOTE: Now that the clear() member function exists, this function only sets the stuff associated with the object and ship class.
void ship_set(int ship_index, int objnum, int ship_type)
{
	int i;
	ship		*shipp = &Ships[ship_index];
	object		*objp = &Objects[objnum];
	ship_info	*sip = &(Ship_info[ship_type]);
	ship_weapon	*swp = &shipp->weapons;
	polymodel *pm = model_get(sip->model_num);

	extern int oo_arrive_time_count[MAX_SHIPS];		
	extern int oo_interp_count[MAX_SHIPS];	
	oo_arrive_time_count[shipp - Ships] = 0;
	oo_interp_count[shipp - Ships] = 0;

	Assert(strlen(shipp->ship_name) <= NAME_LENGTH - 1);
	shipp->ship_info_index = ship_type;
	shipp->objnum = objnum;
	shipp->score = sip->score;

	ai_object_init(objp, shipp->ai_index);
	physics_ship_init(objp);

	if ( !Fred_running ) {
		ship_set_warp_effects(objp, sip);
	}

	if (Fred_running){
		shipp->ship_max_hull_strength = 100.0f;
	} else {
		shipp->ship_max_hull_strength = sip->max_hull_strength;
	}
	objp->hull_strength = shipp->ship_max_hull_strength;

	shipp->max_shield_recharge = sip->max_shield_recharge;

	if (Fred_running) {
		shipp->ship_max_shield_strength = 100.0f;
		objp->shield_quadrant[0] = 100.0f;
	} else {
		shipp->ship_max_shield_strength = sip->max_shield_strength;
		shield_set_strength(objp, shipp->ship_max_shield_strength * shipp->max_shield_recharge);
	}

	if (sip->flags2 & SIF2_MODEL_POINT_SHIELDS) {
		objp->n_quadrants = pm->shield_points.size();
		shipp->shield_points = pm->shield_points;
		objp->shield_quadrant.resize(objp->n_quadrants);
	}

	shipp->orders_accepted = ship_get_default_orders_accepted( sip );

	if (!subsys_set(objnum))
	{
		char err_msg[512]; 
		sprintf (err_msg, "Unable to allocate ship subsystems. Maximum is %d. No subsystems have been assigned to %s.", (NUM_SHIP_SUBSYSTEM_SETS* NUM_SHIP_SUBSYSTEMS_PER_SET), shipp->ship_name);

		if (Fred_running) 
			MessageBox(NULL, err_msg, "Error", MB_OK);
		else
			Error(LOCATION, "%s", err_msg);
	}

	ets_init_ship(objp);	// init ship fields that are used for the ETS

	shipp->current_max_speed = Ship_info[ship_type].max_speed;

	shipp->flags |= SF_ENGINES_ON;

	// set certain flags that used to be in ship_info - Goober5000
	if (sip->flags & SIF_STEALTH)
		shipp->flags2 |= SF2_STEALTH;
	if (sip->flags & SIF_SHIP_CLASS_DONT_COLLIDE_INVIS)
		shipp->flags2 |= SF2_DONT_COLLIDE_INVIS;

	if (sip->flags & SIF_NO_COLLIDE)
		obj_set_flags(objp, objp->flags & ~OF_COLLIDES);
	else
		obj_set_flags(objp, objp->flags | OF_COLLIDES);

	if (sip->flags2 & SIF2_NO_ETS)
		shipp->flags2 |= SF2_NO_ETS;

	shipp->afterburner_fuel = sip->afterburner_fuel_capacity;

	shipp->cmeasure_count = sip->cmeasure_max;
	shipp->current_cmeasure = sip->cmeasure_type;

	if (sip->num_primary_banks == 0 || swp->primary_bank_weapons[0] < 0) {
		swp->current_primary_bank = -1;
		swp->previous_primary_bank = -1;
	}
	if (sip->num_secondary_banks == 0 || swp->secondary_bank_weapons[0] < 0) {
		swp->current_secondary_bank = -1;
		swp->previous_secondary_bank = -1;
	}
	swp->current_tertiary_bank = -1;

	swp->ai_class = Ai_info[shipp->ai_index].ai_class;

	// handle ballistic primaries - kinda hackish; is this actually necessary?
	// because I think it's not needed - when I accidentally left this unreachable
	// it didn't cause any problems - Goober5000
	for ( i = 0; i < sip->num_primary_banks; i++ )
	{
		float weapon_size = Weapon_info[sip->primary_bank_weapons[i]].cargo_size;

		if ( weapon_size > 0.0f )
		{
			if (Fred_running)
				swp->primary_bank_ammo[i] = 100;
			else
				swp->primary_bank_ammo[i] = fl2i(sip->primary_bank_ammo_capacity[i] / weapon_size + 0.5f );
		}
	}

	for ( i = 0; i < sip->num_secondary_banks; i++ )
	{
		float weapon_size = Weapon_info[sip->secondary_bank_weapons[i]].cargo_size;
		Assertion( weapon_size > 0.0f, "Cargo size for secondary weapon %s is invalid, must be greater than 0.\n", Weapon_info[sip->secondary_bank_weapons[i]].name );

		if (Fred_running)
			swp->secondary_bank_ammo[i] = 100;
		else
			swp->secondary_bank_ammo[i] = fl2i(sip->secondary_bank_ammo_capacity[i] / weapon_size + 0.5f );
	}

	shipp->armor_type_idx = sip->armor_type_idx;
	shipp->shield_armor_type_idx = sip->shield_armor_type_idx;
	shipp->collision_damage_type_idx =  sip->collision_damage_type_idx;
	shipp->debris_damage_type_idx = sip->debris_damage_type_idx;

	if(pm != NULL && pm->n_view_positions > 0)
		ship_set_eye(objp, 0);
	else
		ship_set_eye(objp, -1);

	sip->shockwave.damage_type_idx = sip->shockwave.damage_type_idx_sav;

	// Team colors
	shipp->team_name.assign( sip->default_team_name);
	shipp->secondary_team_name = "none";

	shipp->autoaim_fov = sip->autoaim_fov;
}

/**
 * Recalculates the overall strength of subsystems.
 *
 * Needed because several places in FreeSpace change subsystem strength and all 
 * this data needs to be kept up to date.
 */
void ship_recalc_subsys_strength( ship *shipp )
{
	int i;
	ship_subsys *ship_system;

	// fill in the subsys_info fields for all particular types of subsystems
	// make the current strength be 1.0.  If there are initial conditions on the ship, then
	// the mission parse code should take care of setting that.
	for (i = 0; i < SUBSYSTEM_MAX; i++) {
		shipp->subsys_info[i].type_count = 0;
		shipp->subsys_info[i].aggregate_max_hits = 0.0f;
		shipp->subsys_info[i].aggregate_current_hits = 0.0f;
	}

	// count all of the subsystems of a particular type.  For each generic type of subsystem, we store the
	// total count of hits.  (i.e. for 3 engines, we store the sum of the max_hits for each engine)
	for ( ship_system = GET_FIRST(&shipp->subsys_list); ship_system != END_OF_LIST(&shipp->subsys_list); ship_system = GET_NEXT(ship_system) ) {

		if (!(ship_system->flags & SSF_NO_AGGREGATE)) {
			int type = ship_system->system_info->type;
			Assert ( (type >= 0) && (type < SUBSYSTEM_MAX) );

			shipp->subsys_info[type].type_count++;
			shipp->subsys_info[type].aggregate_max_hits += ship_system->max_hits;
			shipp->subsys_info[type].aggregate_current_hits += ship_system->current_hits;
		}

		//Get rid of any persistent sounds on the subsystem
		//This is inefficient + sloppy but there's not really an easy way to handle things
		//if a subsystem is brought back from the dead, other than this
		if(ship_system->current_hits > 0.0f)
		{
			if(ship_system->subsys_snd_flags & SSSF_DEAD)
			{
				obj_snd_delete_type(shipp->objnum, ship_system->system_info->dead_snd, ship_system);
				ship_system->subsys_snd_flags &= ~SSSF_DEAD;
			}
			if((ship_system->system_info->alive_snd != -1) && !(ship_system->subsys_snd_flags & SSSF_ALIVE))
			{
				obj_snd_assign(shipp->objnum, ship_system->system_info->alive_snd, &ship_system->system_info->pnt, 0, OS_SUBSYS_ALIVE, ship_system);
				ship_system->subsys_snd_flags |= SSSF_ALIVE;
			}
			if(!(ship_system->subsys_snd_flags & SSSF_TURRET_ROTATION))
			{
				if(ship_system->system_info->turret_base_rotation_snd != -1)
				{
					obj_snd_assign(shipp->objnum, ship_system->system_info->turret_base_rotation_snd, &ship_system->system_info->pnt, 0, OS_TURRET_BASE_ROTATION, ship_system);
					ship_system->subsys_snd_flags |= SSSF_TURRET_ROTATION;
				}
				if(ship_system->system_info->turret_gun_rotation_snd != -1)
				{
					obj_snd_assign(shipp->objnum, ship_system->system_info->turret_gun_rotation_snd, &ship_system->system_info->pnt, 0, OS_TURRET_GUN_ROTATION, ship_system);
					ship_system->subsys_snd_flags |= SSSF_TURRET_ROTATION;
				}
			}
			if((ship_system->flags & SSF_ROTATES) && (ship_system->system_info->rotation_snd != -1) && !(ship_system->subsys_snd_flags & SSSF_ROTATE))
			{
				obj_snd_assign(shipp->objnum, ship_system->system_info->rotation_snd, &ship_system->system_info->pnt, 0, OS_SUBSYS_ROTATION, ship_system);
				ship_system->subsys_snd_flags |= SSSF_ROTATE;
			}
		}
		else
		{
			if(ship_system->subsys_snd_flags & SSSF_ALIVE)
			{
				obj_snd_delete_type(shipp->objnum, ship_system->system_info->alive_snd, ship_system);
				ship_system->subsys_snd_flags &= ~SSSF_ALIVE;
			}
			if(ship_system->subsys_snd_flags & SSSF_TURRET_ROTATION)
			{
				obj_snd_delete_type(shipp->objnum, ship_system->system_info->turret_base_rotation_snd, ship_system);
				obj_snd_delete_type(shipp->objnum, ship_system->system_info->turret_gun_rotation_snd, ship_system);
				ship_system->subsys_snd_flags &= ~SSSF_TURRET_ROTATION;
			}
			if(ship_system->subsys_snd_flags & SSSF_ROTATE)
			{
				obj_snd_delete_type(shipp->objnum, ship_system->system_info->rotation_snd, ship_system);
				ship_system->subsys_snd_flags &= ~SSSF_ROTATE;
			}
			if((ship_system->system_info->dead_snd != -1) && !(ship_system->subsys_snd_flags & SSSF_DEAD))
			{
				obj_snd_assign(shipp->objnum, ship_system->system_info->dead_snd, &ship_system->system_info->pnt, 0, OS_SUBSYS_DEAD, ship_system);
				ship_system->subsys_snd_flags |= SSSF_DEAD;
			}
		}
	}

	// set any ship flags which should be set.  unset the flags since we might be repairing a subsystem
	// through sexpressions.
	if ( (shipp->subsys_info[SUBSYSTEM_ENGINE].type_count > 0) && (shipp->subsys_info[SUBSYSTEM_ENGINE].aggregate_current_hits <= 0.0f) ) {
		shipp->flags |= SF_DISABLED;
	} else {
		shipp->flags &= ~SF_DISABLED;
		ship_reset_disabled_physics( &Objects[shipp->objnum], shipp->ship_info_index );
	}
}

/**
 * Fixup the model subsystem information for this ship pointer.
 * Needed when ships share the same model.
 */
void ship_copy_subsystem_fixup(ship_info *sip)
{
	int model_num;

	model_num = sip->model_num;

	// no point copying the subsystem data if the ship in question has none...
	// mark that the ship (cargo container) has the path fixup done.
	if (sip->n_subsystems == 0) {
		sip->flags |= SIF_PATH_FIXUP;
		return;
	}

	// if we need to get information for all our subsystems, we need to find another ship with the same model
	// number as our own and that has the model information
	for ( auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it ) {
		model_subsystem *source_msp, *dest_msp;

		if ( (it->model_num != model_num) || (&(*it) == sip) ){
			continue;
		}

		// see if this ship has subsystems and a model for the subsystems.  We only need check the first
		// subsystem since previous error checking would have trapped its loading as an error.
		Assert( it->n_subsystems == sip->n_subsystems );

		source_msp = &it->subsystems[0];
		dest_msp = &(sip->subsystems[0]);
		if (source_msp->model_num != -1) {
			model_copy_subsystems( sip->n_subsystems, dest_msp, source_msp );
		} else if (dest_msp->model_num != -1) {
			model_copy_subsystems( sip->n_subsystems, source_msp, dest_msp );
		} else {
			// if none were found try finding a another ship to copy the data from
			continue;
		}
		sip->flags |= SIF_PATH_FIXUP;
		break;
	}

}

// as with object, don't set next and prev to NULL because they keep the object on the free and used lists
void ship_subsys::clear()
{
	int i;

	system_info = NULL;

	parent_objnum = -1;

	sub_name[0] = 0;
	current_hits = max_hits = 0.0f;

	flags = 0;

	subsys_guardian_threshold = 0;
	armor_type_idx = -1;

	turret_best_weapon = -1;
	turret_last_fire_direction = vmd_zero_vector;
	turret_next_enemy_check_stamp = timestamp(0);
	turret_next_fire_stamp = timestamp(0);
	turret_enemy_objnum = -1;
	turret_enemy_sig = 0;
	turret_next_fire_pos = 0;
	turret_time_enemy_in_range = 0.0f;

	for (i = 0; i < NUM_TURRET_ORDER_TYPES; i++)
		turret_targeting_order[i] = -1;
	optimum_range = 0.0f;
	favor_current_facing = 0.0f;
	targeted_subsys = NULL;
	scripting_target_override = false;
	last_fired_weapon_info_index = -1;

	turret_pick_big_attack_point_timestamp = timestamp(0);
	turret_big_attack_point = vmd_zero_vector;

	turret_animation_position = MA_POS_NOT_SET;
	turret_animation_done_time = 0;

	for (i = 0; i < MAX_TFP; i++)
		turret_swarm_info_index[i] = -1;
	turret_swarm_num = 0;

	awacs_intensity = 0.0f;
	awacs_radius = 0.0f;

	memset(&weapons, 0, sizeof(ship_weapon));

	memset(&submodel_info_1, 0, sizeof(submodel_instance_info));
	memset(&submodel_info_2, 0, sizeof(submodel_instance_info));

	disruption_timestamp = timestamp(0);

	subsys_cargo_name = 0;
	time_subsys_cargo_revealed = 0;

	triggered_rotation_index = -1;

	points_to_target = 0.0f;
	base_rotation_rate_pct = 0.0f;
	gun_rotation_rate_pct = 0.0f;

	subsys_snd_flags = 0;

	rotation_timestamp = timestamp(0);

	world_to_turret_matrix = vmd_identity_matrix;

	for (i = 0; i < 32; i++)
		target_priority[i] = -1;
	num_target_priorities = 0;

	next_aim_pos_time = 0;
	last_aim_enemy_pos = vmd_zero_vector;
	last_aim_enemy_vel = vmd_zero_vector;

	rof_scaler = 1.0f;
	turn_rate = 0.0f;

	turret_max_bomb_ownage = -1; 
	turret_max_target_ownage = -1;
}

/**
 * Set subsystem
 *
 * @param objnum				Object number (used as index into Objects[])
 * @param ignore_subsys_info	Default parameter with value of 0.  This is only set to 1 by the save/restore code
 */
int subsys_set(int objnum, int ignore_subsys_info)
{	
	ship	*shipp = &Ships[Objects[objnum].instance];
	ship_info	*sinfo = &Ship_info[Ships[Objects[objnum].instance].ship_info_index];
	model_subsystem *model_system;
	ship_subsys *ship_system;
	int i, j, k;

	// set up the subsystems for this ship.  walk through list of subsystems in the ship-info array.
	// for each subsystem, get a new ship_subsys instance and set up the pointers and other values
	list_init ( &shipp->subsys_list );								// initialize the ship's list of subsystems

	// make sure to have allocated the number of subsystems we require
	if (!ship_allocate_subsystems( sinfo->n_subsystems )) {
		return 0;
	}

	for ( i = 0; i < sinfo->n_subsystems; i++ )
	{
		model_system = &(sinfo->subsystems[i]);
		if (model_system->model_num < 0) {
			Warning (LOCATION, "Invalid subobj_num or model_num in subsystem '%s' on ship type '%s'.\nNot linking into ship!\n\n(This warning means that a subsystem was present in the table entry and not present in the model\nit should probably be removed from the table or added to the model.)\n", model_system->subobj_name, sinfo->name );
			continue;
		}

		// set up the linked list
		ship_system = GET_FIRST( &ship_subsys_free_list );		// get a new element from the ship_subsystem array
		Assert ( ship_system != &ship_subsys_free_list );		// shouldn't have the dummy element
		list_remove( ship_subsys_free_list, ship_system );		// remove the element from the array
		list_append( &shipp->subsys_list, ship_system );		// link the element into the ship
		ship_system->clear();									// initialize it to a known blank slate

		ship_system->system_info = model_system;				// set the system_info pointer to point to the data read in from the model

		ship_system->parent_objnum = objnum;

		// if the table has set an name copy it
		if (ship_system->system_info->alt_sub_name[0] != '\0') {
			strcpy_s(ship_system->sub_name, ship_system->system_info->alt_sub_name);
		}
		else {
			memset(ship_system->sub_name, '\0', sizeof(ship_system->sub_name));
		}

		// copy subsystem target priorities stuff
		ship_system->num_target_priorities = ship_system->system_info->num_target_priorities;
		for (j = 0; j < 32; j++) {
			ship_system->target_priority[j] = ship_system->system_info->target_priority[j];
		}

		ship_system->rof_scaler = ship_system->system_info->turret_rof_scaler;

		// zero flags
		ship_system->flags = 0;
		ship_system->weapons.flags = 0;
		ship_system->subsys_snd_flags = 0;

		// Goober5000
		if (model_system->flags & MSS_FLAG_UNTARGETABLE)
			ship_system->flags |= SSF_UNTARGETABLE;
		// Wanderer
		if (model_system->flags & MSS_FLAG_NO_SS_TARGETING)
			ship_system->flags |= SSF_NO_SS_TARGETING;
		if ((The_mission.ai_profile->flags2 & AIPF2_ADVANCED_TURRET_FOV_EDGE_CHECKS) || (model_system->flags & MSS_FLAG_FOV_EDGE_CHECK))
			ship_system->flags |= SSF_FOV_EDGE_CHECK;
		if ((The_mission.ai_profile->flags2 & AIPF2_REQUIRE_TURRET_TO_HAVE_TARGET_IN_FOV) || (model_system->flags & MSS_FLAG_FOV_REQUIRED))
			ship_system->flags |= SSF_FOV_REQUIRED;

		if (model_system->flags & MSS_FLAG_NO_REPLACE)
			ship_system->flags |= SSF_NO_REPLACE;
		if (model_system->flags & MSS_FLAG_NO_LIVE_DEBRIS)
			ship_system->flags |= SSF_NO_LIVE_DEBRIS;
		if (model_system->flags & MSS_FLAG_IGNORE_IF_DEAD)
			ship_system->flags |= SSF_MISSILES_IGNORE_IF_DEAD;
		if (model_system->flags & MSS_FLAG_ALLOW_VANISHING)
			ship_system->flags |= SSF_VANISHED;
		if (model_system->flags & MSS_FLAG_DAMAGE_AS_HULL)
			ship_system->flags |= SSF_DAMAGE_AS_HULL;
		if (model_system->flags & MSS_FLAG_NO_AGGREGATE)
			ship_system->flags |= SSF_NO_AGGREGATE;
		if (model_system->flags & MSS_FLAG_ROTATES)
			ship_system->flags |= SSF_ROTATES;
		if (model_system->flags2 & MSS_FLAG2_PLAYER_TURRET_SOUND)
			ship_system->flags |= SSF_PLAY_SOUND_FOR_PLAYER;
		if (model_system->flags2 & MSS_FLAG2_NO_DISAPPEAR)
			ship_system->flags |= SSF_NO_DISAPPEAR;
		if (model_system->flags2 & MSS_FLAG2_AUTOREPAIR_IF_DISABLED)
			ship_system->flags |= SSF_AUTOREPAIR_IF_DISABLED;
		if (model_system->flags2 & MSS_FLAG2_NO_AUTOREPAIR_IF_DISABLED)
			ship_system->flags |= SSF_NO_AUTOREPAIR_IF_DISABLED;

		ship_system->turn_rate = model_system->turn_rate;

		// Goober5000 - this has to be moved outside back to parse_create_object, because
		// a lot of the ship creation code is duplicated in several points and overwrites
		// previous things... ugh.
		ship_system->max_hits = model_system->max_subsys_strength;	// * shipp->ship_max_hull_strength / sinfo->max_hull_strength;

		if ( !Fred_running ) {
			ship_system->current_hits = ship_system->max_hits;		// set the current hits
		} else {
			ship_system->current_hits = 0.0f;				// Jason wants this to be 0 in Fred.
		}

		ship_system->subsys_guardian_threshold = 0;
		ship_system->armor_type_idx = model_system->armor_type_idx;
		ship_system->turret_next_fire_stamp = timestamp(0);
		ship_system->turret_next_enemy_check_stamp = timestamp(0);
		ship_system->turret_enemy_objnum = -1;
		ship_system->turret_next_fire_stamp = timestamp((int) frand_range(1.0f, 500.0f));	// next time this turret can fire
		ship_system->turret_last_fire_direction = model_system->turret_norm;
		ship_system->turret_next_fire_pos = 0;
		ship_system->turret_time_enemy_in_range = 0.0f;
		ship_system->disruption_timestamp=timestamp(0);
		ship_system->turret_pick_big_attack_point_timestamp = timestamp(0);
		ship_system->scripting_target_override = false;
		vm_vec_zero(&ship_system->turret_big_attack_point);
		for(j = 0; j < NUM_TURRET_ORDER_TYPES; j++)
		{
			//WMC - Set targeting order to default.
			ship_system->turret_targeting_order[j] = j;
		}
		ship_system->optimum_range = model_system->optimum_range;
		ship_system->favor_current_facing = model_system->favor_current_facing;
		ship_system->subsys_cargo_name = 0;
		ship_system->time_subsys_cargo_revealed = 0;
		
		j = 0;
		int number_of_weapons = 0;

		for (k=0; k<MAX_SHIP_PRIMARY_BANKS; k++){
			if (model_system->primary_banks[k] != -1) {
				ship_system->weapons.primary_bank_weapons[j] = model_system->primary_banks[k];
				ship_system->weapons.primary_bank_capacity[j] = model_system->primary_bank_capacity[k];	// added by Goober5000
				ship_system->weapons.next_primary_fire_stamp[j] = timestamp(0);
				ship_system->weapons.last_primary_fire_stamp[j++] = -1;
			}
			ship_system->weapons.burst_counter[k] = 0;
		}

		ship_system->weapons.num_primary_banks = j;
		number_of_weapons += j;

		j = 0;
		for (k=0; k<MAX_SHIP_SECONDARY_BANKS; k++){
			if (model_system->secondary_banks[k] != -1) {
				ship_system->weapons.secondary_bank_weapons[j] = model_system->secondary_banks[k];
				ship_system->weapons.secondary_bank_capacity[j] = model_system->secondary_bank_capacity[k];
				ship_system->weapons.next_secondary_fire_stamp[j] = timestamp(0);
				ship_system->weapons.last_secondary_fire_stamp[j++] = -1;
			}
			ship_system->weapons.burst_counter[k + MAX_SHIP_PRIMARY_BANKS] = 0;
		}

		ship_system->weapons.num_secondary_banks = j;
		number_of_weapons += j;
		ship_system->weapons.current_primary_bank = -1;
		ship_system->weapons.current_secondary_bank = -1;
		
		ship_system->next_aim_pos_time = 0;

		ship_system->turret_max_bomb_ownage = model_system->turret_max_bomb_ownage;
		ship_system->turret_max_target_ownage = model_system->turret_max_target_ownage;

		// Make turret flag checks and warnings
		if ((ship_system->system_info->flags & MSS_FLAG_TURRET_SALVO) && (ship_system->system_info->flags & MSS_FLAG_TURRET_FIXED_FP))
		{
			Warning (LOCATION, "\"salvo mode\" flag used with \"fixed firingpoints\" flag\nsubsystem '%s' on ship type '%s'.\n\"salvo mode\" flag is ignored\n", model_system->subobj_name, sinfo->name );
			ship_system->system_info->flags &= (~MSS_FLAG_TURRET_SALVO);
		}

		if ((ship_system->system_info->flags & MSS_FLAG_TURRET_SALVO) && (model_system->turret_num_firing_points < 2))
		{
			Warning (LOCATION, "\"salvo mode\" flag used with turret which has less than two firingpoints\nsubsystem '%s' on ship type '%s'.\n\"salvo mode\" flag is ignored\n", model_system->subobj_name, sinfo->name );
			ship_system->system_info->flags &= (~MSS_FLAG_TURRET_SALVO);
		}

		if ((ship_system->system_info->flags & MSS_FLAG_TURRET_FIXED_FP) && (model_system->turret_num_firing_points < 2))
		{
			Warning (LOCATION, "\"fixed firingpoints\" flag used with turret which has less than two firingpoints\nsubsystem '%s' on ship type '%s'.\n\"fixed firingpoints\" flag is ignored\n", model_system->subobj_name, sinfo->name );
			ship_system->system_info->flags &= (~MSS_FLAG_TURRET_FIXED_FP);
		}

		if ((ship_system->system_info->flags & MSS_FLAG_TURRET_SALVO) && (ship_system->system_info->flags & MSS_FLAG_USE_MULTIPLE_GUNS))
		{
			Warning (LOCATION, "\"salvo mode\" flag used with \"use multiple guns\" flag\nsubsystem '%s' on ship type '%s'.\n\"use multiple guns\" flag is ignored\n", model_system->subobj_name, sinfo->name );
			ship_system->system_info->flags &= (~MSS_FLAG_USE_MULTIPLE_GUNS);
		}

		if ((ship_system->system_info->flags & MSS_FLAG_TURRET_FIXED_FP) && !(ship_system->system_info->flags & MSS_FLAG_USE_MULTIPLE_GUNS))
		{
			Warning (LOCATION, "\"fixed firingpoints\" flag used without \"use multiple guns\" flag\nsubsystem '%s' on ship type '%s'.\n\"use multiple guns\" guns added by default\n", model_system->subobj_name, sinfo->name );
			ship_system->system_info->flags |= MSS_FLAG_USE_MULTIPLE_GUNS;
		}

		if ((ship_system->system_info->flags & MSS_FLAG_TURRET_SALVO) && (number_of_weapons > 1))
		{
			Warning (LOCATION, "\"salvo mode\" flag used with turret which has more than one weapon defined for it\nsubsystem '%s' on ship type '%s'.\nonly single weapon will be used\n", model_system->subobj_name, sinfo->name );
		}

		if ((ship_system->system_info->flags & MSS_FLAG_TURRET_FIXED_FP) && (number_of_weapons > model_system->turret_num_firing_points))
		{
			Warning (LOCATION, "\"fixed firingpoint\" flag used with turret which has more weapons defined for it than it has firingpoints\nsubsystem '%s' on ship type '%s'.\nweapons will share firingpoints\n", model_system->subobj_name, sinfo->name );
		}

		if ((ship_system->system_info->flags & MSS_FLAG_TURRET_FIXED_FP) && (number_of_weapons < model_system->turret_num_firing_points))
		{
			Warning (LOCATION, "\"fixed firingpoint\" flag used with turret which has less weapons defined for it than it has firingpoints\nsubsystem '%s' on ship type '%s'.\nsome of the firingpoints will be left unused\n", model_system->subobj_name, sinfo->name );
		}


		for (k=0; k<ship_system->weapons.num_secondary_banks; k++) {
			float weapon_size = Weapon_info[ship_system->weapons.secondary_bank_weapons[k]].cargo_size;
			Assertion( weapon_size > 0.0f, "Cargo size for secondary weapon %s is invalid, must be greater than 0.\n", Weapon_info[ship_system->weapons.secondary_bank_weapons[k]].name );
			ship_system->weapons.secondary_bank_ammo[k] = (Fred_running ? 100 : fl2i(ship_system->weapons.secondary_bank_capacity[k] / weapon_size + 0.5f));

			ship_system->weapons.secondary_next_slot[k] = 0;
		}

		// Goober5000
		for (k=0; k<ship_system->weapons.num_primary_banks; k++)
		{
			float weapon_size = Weapon_info[ship_system->weapons.primary_bank_weapons[k]].cargo_size;

			if (weapon_size > 0.0f) {	// Non-ballistic primaries are supposed to have a cargo_size of 0
				ship_system->weapons.primary_bank_ammo[k] = (Fred_running ? 100 : fl2i(ship_system->weapons.primary_bank_capacity[k] / weapon_size + 0.5f));
			}
		}

		ship_system->weapons.last_fired_weapon_index = -1;
		ship_system->weapons.last_fired_weapon_signature = -1;
		ship_system->weapons.detonate_weapon_time = -1;
		ship_system->weapons.ai_class = sinfo->ai_class;  // assume ai class of ship for turret

		// rapid fire (swarm) stuff
		for (k = 0; k < MAX_TFP; k++)
			ship_system->turret_swarm_info_index[k] = -1;

		ship_system->turret_swarm_num = 0;

		// AWACS stuff
		ship_system->awacs_intensity = model_system->awacs_intensity;
		ship_system->awacs_radius = model_system->awacs_radius;
		if (ship_system->awacs_intensity > 0) {
			ship_system->system_info->flags |= MSS_FLAG_AWACS;
		}

		// turn_rate, turn_accel
		float turn_accel = 0.5f;
		model_set_instance_info(&ship_system->submodel_info_1, model_system->turn_rate, turn_accel);

		model_clear_instance_info( &ship_system->submodel_info_2 );

		// Clear this flag here so we correctly rebuild the turret matrix on mission load
		model_system->flags &= ~MSS_FLAG_TURRET_MATRIX;

		// Allocate a triggered rotation instance if we need it
		if (model_system->flags & MSS_FLAG_TRIGGERED) {
			ship_system->triggered_rotation_index = Triggered_rotations.size();
			triggered_rotation tr;
			Triggered_rotations.push_back(tr);
		}
	}

	if ( !ignore_subsys_info ) {
		ship_recalc_subsys_strength( shipp );
	}

	// Fix up animation code references
	for (i = 0; i < sinfo->n_subsystems; i++) {
		for (j = 0; j < sinfo->subsystems[i].n_triggers; j++) {
			if (subsystem_stricmp(sinfo->subsystems[i].triggers[j].sub_name, "<none>")) {
				int idx = ship_get_subobj_model_num(sinfo, sinfo->subsystems[i].triggers[j].sub_name);
				if (idx != -1) {
					sinfo->subsystems[i].triggers[j].subtype = idx;
				} else {
					WarningEx(LOCATION, "Could not find subobject %s in ship class %s. Animation triggers will not work correctly.\n", sinfo->subsystems[i].triggers[j].sub_name, sinfo->name);
				}
			}
		}
	}

	return 1;
}

/**
 * Modify the matrix orient by the slew angles a.
 */
void compute_slew_matrix(matrix *orient, angles *a)
{
	matrix	tmp, tmp2;
	angles	t1, t2;

	t1 = t2 = *a;
	t1.h = 0.0f;	t1.b = 0.0f;
	t2.p = 0.0f;	t2.b = 0.0f;

	// put in p & b like normal
	vm_angles_2_matrix(&tmp, &t2 ); // Changed the order of axis rotations. First pitch, then yaw (Swifty)
	vm_matrix_x_matrix( &tmp2, orient, &tmp);

	// Put in heading separately
	vm_angles_2_matrix(&tmp, &t1 );
	vm_matrix_x_matrix( orient, &tmp2, &tmp );

	vm_orthogonalize_matrix(orient);
}


#ifndef NDEBUG
/**
 * Render docking information, NOT while in object's reference frame.
 */
void render_dock_bays(object *objp)
{
	polymodel	*pm;
	dock_bay		*db;

	pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);

	if (pm->docking_bays == NULL)
		return;

	if (pm->docking_bays[0].num_slots != 2)
		return;

	db = &pm->docking_bays[0];

	vertex	v0, v1;
	vec3d	p0, p1, p2, p3, nr;

	vm_vec_unrotate(&p0, &db->pnt[0], &objp->orient);
	vm_vec_add2(&p0, &objp->pos);
	g3_rotate_vertex(&v0, &p0);

	vm_vec_unrotate(&p1, &db->pnt[1], &objp->orient);
	vm_vec_add2(&p1, &objp->pos);
	g3_rotate_vertex(&v1, &p1);

	gr_set_color(255, 0, 0);
	g3_draw_line(&v0, &v1);

	vm_vec_avg(&p2, &p0, &p1);

	vm_vec_unrotate(&nr, &db->norm[0], &objp->orient);
	vm_vec_scale_add(&p3, &p2, &nr, 10.0f);

	g3_rotate_vertex(&v0, &p2);
	g3_rotate_vertex(&v1, &p3);
	gr_set_color(255, 255, 0);
	g3_draw_line(&v0, &v1);
	g3_draw_sphere(&v1, 1.25f);

}
#endif

int Ship_shadows = 0;

DCF_BOOL( ship_shadows, Ship_shadows )

MONITOR( NumShipsRend )

int Show_shield_hits = 0;
DCF_BOOL( show_shield_hits, Show_shield_hits )

int Show_tnorms = 0;
DCF_BOOL( show_tnorms, Show_tnorms )

int Show_paths = 0;
DCF_BOOL( show_paths, Show_paths )

int Show_fpaths = 0;
DCF_BOOL( show_fpaths, Show_fpaths )

void ship_find_warping_ship_helper(object *objp, dock_function_info *infop)
{
	// only check ships
	if (objp->type != OBJ_SHIP)
		return;

	// am I arriving or departing by warp?
	if ( Ships[objp->instance].flags & (SF_ARRIVING|SF_DEPART_WARP) )
	{
#ifndef NDEBUG
		// in debug builds, make sure only one of the docked objects has these flags set
		if (infop->maintained_variables.bool_value)
		{
			//WMC - This is annoying and triggered in sm2-10
			//Warning(LOCATION, "Ship %s and its docked ship %s are arriving or departing at the same time.\n",
			//Ships[infop->maintained_variables.objp_value->instance].ship_name, Ships[objp->instance].ship_name);
		}
#endif
		// we found someone
		infop->maintained_variables.bool_value = true;
		infop->maintained_variables.objp_value = objp;

#ifdef NDEBUG
		// return early in release builds
		infop->early_return_condition = true;
#endif
	}
}

SCP_vector<man_thruster_renderer> Man_thrusters;

/**
 * Batch renders all maneuvering thrusters in the array.
 *
 * It also clears the array every 10 seconds to keep mem usage down.
 */
void batch_render_man_thrusters()
{
	man_thruster_renderer *mtr;
	size_t mant_size = Man_thrusters.size();

	if (mant_size == 0)
		return;

	for(size_t i = 0; i < mant_size; i++)
	{
		mtr = &Man_thrusters[i];
		gr_set_bitmap(mtr->bmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);

		mtr->man_batcher.render(TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT);
		mtr->bmap_id = -1;	//Mark as free
	}

	//WMC - clear maneuvering thruster render queue every 10 seconds
	if(timestamp() - Man_thruster_reset_timestamp > 10000)
	{
		Man_thrusters.clear();
		Man_thruster_reset_timestamp = timestamp();
	}
}

/**
 * Looks for a free slot in the man_thruster batch
 * rendering array. Or, it returns a slot with the same bitmap
 * ID as the maneuvering thruster.
 *
 * You could actually batch render anything that uses a simple bitmap
 * on a single poly with this system...just plug the bitmap into bmap_frame
 * and use as a normal batcher.
 *
 * Once calling this function, use man_batcher.allocate_add() to allocate or it will crash later.
 * Then call man_batcher.draw*()
 */
man_thruster_renderer *man_thruster_get_slot(int bmap_frame)
{
	man_thruster_renderer *mtr;
	size_t mant_size = Man_thrusters.size();

	for(size_t mi = 0; mi < mant_size; mi++)
	{
		mtr = &Man_thrusters[mi];
		if(mtr->bmap_id == bmap_frame)
			return mtr;
	}
	for(size_t mj = 0; mj < mant_size; mj++)
	{
		mtr = &Man_thrusters[mj];
		if(mtr->bmap_id == -1)
		{
			mtr->bmap_id = bmap_frame;
			return mtr;
		}
	}

	Man_thrusters.push_back(man_thruster_renderer(bmap_frame));
	return &Man_thrusters[Man_thrusters.size()-1];
}

//WMC - used for FTL and maneuvering thrusters
geometry_batcher fx_batcher;
extern bool Rendering_to_shadow_map;
void ship_render_DEPRECATED(object * obj)
{
	int num = obj->instance;
	Assert( num >= 0);
	ship *shipp = &Ships[num];
	ship *warp_shipp = NULL;
	ship_info *sip = &Ship_info[Ships[num].ship_info_index];
	bool is_first_stage_arrival = false;
	bool show_thrusters = ((shipp->flags2 & SF2_NO_THRUSTERS) == 0) && !Rendering_to_shadow_map;
	dock_function_info dfi;


#if 0
	// show target when attacking big ship
	vec3d temp, target;
	ai_info *aip = &Ai_info[Ships[obj->instance].ai_index];
	if ( (aip->target_objnum >= 0)  && (Ship_info[Ships[Objects[aip->target_objnum].instance].ship_info_index].flags & (SIF_SUPERCAP|SIF_CAPITAL|SIF_CRUISER)) ) {
		vm_vec_unrotate(&temp, &aip->big_attack_point, &Objects[aip->target_objnum].orient);
		vm_vec_add(&target, &temp, &Objects[aip->target_objnum].pos);

		vertex v0, v1;
		gr_set_color(128,0,0);
		g3_rotate_vertex( &v0, &obj->pos );
		g3_rotate_vertex( &v1, &target );

		g3_draw_line(&v0, &v1);

		g3_draw_sphere(&v1, 5.0f);
	}
#endif


	if ( obj == Viewer_obj && !Rendering_to_shadow_map)
	{
		if (ship_show_velocity_dot && (obj==Player_obj) )
		{
			vec3d p0,v;
			vertex v0;

			vm_vec_scale_add( &v, &obj->phys_info.vel, &obj->orient.vec.fvec, 3.0f );
			vm_vec_normalize( &v );
			
					
			vm_vec_scale_add( &p0, &obj->pos, &v, 20.0f);

			g3_rotate_vertex( &v0, &p0 );
			
			gr_set_color(0,128,0);
			g3_draw_sphere( &v0, 0.1f );
		}

		// Show the shield hit effect for the viewer.
		if ( Show_shield_hits )
		{
			shipp = &Ships[num];
			if (shipp->shield_hits)
			{
				create_shield_explosion_all(obj);
				shipp->shield_hits = 0;
			}
		}		

		if (!(Viewer_mode & VM_TOPDOWN))
		{
			return;
		}
	}

	MONITOR_INC( NumShipsRend, 1 );

	// look for a warping ship, whether for me or for anybody I'm docked with
	dock_evaluate_all_docked_objects(obj, &dfi, ship_find_warping_ship_helper);

	// if any docked objects are set to stage 1 arrival then set bool
	if (dfi.maintained_variables.bool_value) {
		warp_shipp = &Ships[dfi.maintained_variables.objp_value->instance];

		is_first_stage_arrival = ((warp_shipp->flags & SF_ARRIVING_STAGE_1) > 0);

		// This is a hack to make ships using the hyperspace warpin type to
		// render even in stage 1, which is used for collision detection
		// purposes -zookeeper
		if (Ship_info[warp_shipp->ship_info_index].warpin_type == WT_HYPERSPACE) {
			warp_shipp = NULL;
			is_first_stage_arrival = false;
		}
	}


	// Make ships that are warping in not render during stage 1
	if ( !(is_first_stage_arrival) )
	{
		if ( Ship_shadows && shipfx_in_shadow( obj ) )	{
			light_set_shadow(1);
		} else {
			light_set_shadow(0);
		}

		ship_model_start(obj);

		uint render_flags = MR_DEPRECATED_NORMAL;
	#ifndef NDEBUG
// 		if(Show_paths || Show_fpaths){
// 			render_flags |= MR_DEPRECATED_BAY_PATHS;
// 		}
	#endif

		// Only render electrical arcs if within 500m of the eye (for a 10m piece)
		if ( vm_vec_dist_quick( &obj->pos, &Eye_position ) < obj->radius*50.0f && !Rendering_to_shadow_map)	{
			int i;
			for (i=0; i<MAX_SHIP_ARCS; i++ )	{
				if ( timestamp_valid( shipp->arc_timestamp[i] ) )	{
					model_add_arc(sip->model_num, -1, &shipp->arc_pts[i][0], &shipp->arc_pts[i][1], shipp->arc_type[i]);
				}
			}
		}

		if ( shipp->large_ship_blowup_index >= 0 )	{
			shipfx_large_blowup_render(shipp);
		} else {
			//WMC - I suppose this is a bit hackish.
			if(!Rendering_to_shadow_map)
			{
				physics_info *pi = &Objects[shipp->objnum].phys_info;
				float render_amount;
				fx_batcher.allocate(sip->num_maneuvering);	//Act as if all thrusters are going.

				for(int i = 0; i < sip->num_maneuvering; i++)
				{
					man_thruster *mtp = &sip->maneuvering[i];

					render_amount = 0.0f;

					//WMC - get us a steady value
					vec3d des_vel;
					vm_vec_rotate(&des_vel, &pi->desired_vel, &obj->orient);

					if(pi->desired_rotvel.xyz.x < 0 && (mtp->use_flags & MT_PITCH_UP)) {
						render_amount = fl_abs(pi->desired_rotvel.xyz.x) / pi->max_rotvel.xyz.x;
					} else if(pi->desired_rotvel.xyz.x > 0 && (mtp->use_flags & MT_PITCH_DOWN)) {
						render_amount = fl_abs(pi->desired_rotvel.xyz.x) / pi->max_rotvel.xyz.x;
					} else if(pi->desired_rotvel.xyz.y < 0 && (mtp->use_flags & MT_ROLL_RIGHT)) {
						render_amount = fl_abs(pi->desired_rotvel.xyz.y) / pi->max_rotvel.xyz.y;
					} else if(pi->desired_rotvel.xyz.y > 0 && (mtp->use_flags & MT_ROLL_LEFT)) {
						render_amount = fl_abs(pi->desired_rotvel.xyz.y) / pi->max_rotvel.xyz.y;
					} else if(pi->desired_rotvel.xyz.z < 0 && (mtp->use_flags & MT_BANK_RIGHT)) {
						render_amount = fl_abs(pi->desired_rotvel.xyz.z) / pi->max_rotvel.xyz.z;
					} else if(pi->desired_rotvel.xyz.z > 0 && (mtp->use_flags & MT_BANK_LEFT)) {
						render_amount = fl_abs(pi->desired_rotvel.xyz.z) / pi->max_rotvel.xyz.z;
					}
				
					//Backslash - show thrusters according to thrust amount, not speed
					if(pi->side_thrust > 0 && (mtp->use_flags & MT_SLIDE_RIGHT)) {
						render_amount = pi->side_thrust;
					} else if(pi->side_thrust < 0 && (mtp->use_flags & MT_SLIDE_LEFT)) {
						render_amount = -pi->side_thrust;
					} else if(pi->vert_thrust > 0 && (mtp->use_flags & MT_SLIDE_UP)) {
						render_amount = pi->vert_thrust;
					} else if(pi->vert_thrust < 0 && (mtp->use_flags & MT_SLIDE_DOWN)) {
						render_amount = -pi->vert_thrust;
					} else if(pi->forward_thrust > 0 && (mtp->use_flags & MT_FORWARD)) {
						render_amount = pi->forward_thrust;
					} else if(pi->forward_thrust < 0 && (mtp->use_flags & MT_REVERSE)) {
						render_amount = -pi->forward_thrust;
					}

					//Don't render small faraway thrusters (more than 10k * radius away)
					if (vm_vec_dist(&Eye_position, &obj->pos) > (10000.0f * mtp->radius))
						render_amount = 0.0f;

					if(render_amount > 0.0f)
					{
						//Handle sounds and stuff
						if(shipp->thrusters_start[i] <= 0)
						{
							shipp->thrusters_start[i] = timestamp();
							if(mtp->start_snd >= 0)
								snd_play_3d( &Snds[mtp->start_snd], &mtp->pos, &Eye_position, 0.0f, &obj->phys_info.vel );
						}

						//Only assign looping sound if
						//it is specified
						//it isn't assigned already
						//start sound doesn't exist or has finished
						if (!Cmdline_freespace_no_sound)
							if(mtp->loop_snd >= 0
								&& shipp->thrusters_sounds[i] < 0
								&& (mtp->start_snd < 0 || (snd_get_duration(mtp->start_snd) < timestamp() - shipp->thrusters_start[i])) 
								)
							{
								shipp->thrusters_sounds[i] = obj_snd_assign(OBJ_INDEX(obj), mtp->loop_snd, &mtp->pos, 1);
							}

						//Draw graphics
						//Skip invalid ones
						if(mtp->tex_id >= 0)
						{
							float rad = mtp->radius;
							if(rad <= 0.0f)
								rad = 1.0f;

							float len = mtp->length;
							if(len == 0.0f)
								len = rad;

							vec3d start, tmpend, end;
							//Start
							vm_vec_unrotate(&start, &mtp->pos, &obj->orient);
							vm_vec_add2(&start, &obj->pos);

							//End
							vm_vec_scale_add(&tmpend, &mtp->pos, &mtp->norm, len * render_amount);
							vm_vec_unrotate(&end, &tmpend, &obj->orient);
							vm_vec_add2(&end, &obj->pos);

							//Draw
							fx_batcher.draw_beam(&start, &end, rad, 1.0f);

							int bmap_frame = mtp->tex_id;
							if(mtp->tex_nframes > 0)
								bmap_frame += (int)(((float)(timestamp() - shipp->thrusters_start[i]) / 1000.0f) * (float)mtp->tex_fps) % mtp->tex_nframes;

							man_thruster_renderer *mtr = man_thruster_get_slot(bmap_frame);
							mtr->man_batcher.add_allocate(1);
							mtr->man_batcher.draw_beam(&start, &end, rad, 1.0f);
						}

					}
					//We've stopped firing a thruster
					else if(shipp->thrusters_start[i] > 0)
					{
						shipp->thrusters_start[i] = 0;
						if(shipp->thrusters_sounds[i] >= 0)
						{
							obj_snd_delete(OBJ_INDEX(obj), shipp->thrusters_sounds[i]);
							shipp->thrusters_sounds[i] = -1;
						}

						if(mtp->stop_snd >= 0)
						{
							//Get world pos
							vec3d start;
							vm_vec_unrotate(&start, &mtp->pos, &obj->orient);
							vm_vec_add2(&start, &obj->pos);

							snd_play_3d( &Snds[mtp->stop_snd], &mtp->pos, &Eye_position, 0.0f, &obj->phys_info.vel );
						}
					}
				}
			}

			if ( !(shipp->flags & SF_DISABLED) && !ship_subsys_disrupted(shipp, SUBSYSTEM_ENGINE) && show_thrusters) {
				mst_info mst;

				mst.length.xyz.z = obj->phys_info.forward_thrust;
				mst.length.xyz.x = obj->phys_info.side_thrust;
				mst.length.xyz.y = obj->phys_info.vert_thrust;

				//	Maybe add noise to thruster geometry.
				if (!(sip->flags2 & SIF2_NO_THRUSTER_GEO_NOISE)) {
					mst.length.xyz.z *= (1.0f + frand()/5.0f - 0.1f);
					mst.length.xyz.y *= (1.0f + frand()/5.0f - 0.1f);
					mst.length.xyz.x *= (1.0f + frand()/5.0f - 0.1f);
				}

				CLAMP(mst.length.xyz.z, -1.0f, 1.0f);
				CLAMP(mst.length.xyz.y, -1.0f, 1.0f);
				CLAMP(mst.length.xyz.x, -1.0f, 1.0f);

				mst.primary_bitmap = shipp->thruster_bitmap;
				mst.primary_glow_bitmap = shipp->thruster_glow_bitmap;
				mst.secondary_glow_bitmap = shipp->thruster_secondary_glow_bitmap;
				mst.tertiary_glow_bitmap = shipp->thruster_tertiary_glow_bitmap;
				mst.distortion_bitmap = shipp->thruster_distortion_bitmap;

				mst.use_ab = (obj->phys_info.flags & PF_AFTERBURNER_ON) || (obj->phys_info.flags & PF_BOOSTER_ON);
				mst.glow_noise = shipp->thruster_glow_noise * sip->thruster_glow_noise_mult;
				mst.rotvel = Objects[shipp->objnum].phys_info.rotvel;

				mst.glow_rad_factor = sip->thruster01_glow_rad_factor;
				mst.secondary_glow_rad_factor = sip->thruster02_glow_rad_factor;
				mst.tertiary_glow_rad_factor = sip->thruster03_glow_rad_factor;
				mst.glow_length_factor = sip->thruster02_glow_len_factor;
				mst.distortion_length_factor = sip->thruster_dist_len_factor;
				mst.distortion_rad_factor = sip->thruster_dist_rad_factor;

				mst.draw_distortion = sip->draw_distortion;

				model_set_thrust(sip->model_num, &mst);

				render_flags |= MR_DEPRECATED_SHOW_THRUSTERS;
			}

			// fill the model flash lighting values in
			shipfx_flash_light_model( obj, sip->model_num );

			
			// If the ship is going "through" the warp effect, then
			// set up the model renderer to only draw the polygons in front
			// of the warp in effect
			int clip_started = 0;

			// Warp_shipp points to the ship that is going through a
			// warp... either this ship or the ship it is docked with.
			if ( warp_shipp != NULL )
			{
				if(warp_shipp->flags & SF_ARRIVING)
					clip_started = warp_shipp->warpin_effect->warpShipClip();
				else if(warp_shipp->flags & SF_DEPART_WARP)
					clip_started = warp_shipp->warpout_effect->warpShipClip();
			}

			// maybe set squad logo bitmap
			model_set_insignia_bitmap(-1);
			if(!Rendering_to_shadow_map)
			{
				if(Game_mode & GM_MULTIPLAYER){
					// if its any player's object
					int np_index = multi_find_player_by_object( obj );
					if((np_index >= 0) && (np_index < MAX_PLAYERS) && MULTI_CONNECTED(Net_players[np_index]) && (Net_players[np_index].m_player != NULL)){
						model_set_insignia_bitmap(Net_players[np_index].m_player->insignia_texture);
					}
				}
				// in single player, we want to render model insignias on all ships in alpha beta and gamma
				// Goober5000 - and also on wings that have their logos set
				else {
					// if its an object in my squadron
					if(ship_in_my_squadron(shipp)) {
						model_set_insignia_bitmap(Player->insignia_texture);
					}

					// maybe it has a wing squad logo - Goober5000
					if (shipp->wingnum >= 0)
					{
						// don't override the player's wing
						if (shipp->wingnum != Player_ship->wingnum)
						{
							// if we have a logo texture
							if (Wings[shipp->wingnum].wing_insignia_texture >= 0)
							{
								model_set_insignia_bitmap(Wings[shipp->wingnum].wing_insignia_texture);
							}
						}
					}
				}
			

				// nebula		
				if(The_mission.flags & MISSION_FLAG_FULLNEB){		
					extern void model_set_fog_level(float l);
					model_set_fog_level(neb2_get_fog_intensity(obj));
				}

				// Valathil - maybe do a scripting hook here to do some scriptable effects?
				if(shipp->shader_effect_active && Use_GLSL > 1)
				{
					float timer;
					render_flags |= (MR_DEPRECATED_ANIMATED_SHADER);

					ship_effect* sep = &Ship_effects[shipp->shader_effect_num];
					
					if (sep->invert_timer) {
						timer = 1.0f - ((timer_get_milliseconds() - shipp->shader_effect_start_time) / (float)shipp->shader_effect_duration);
						timer = MAX(timer,0.0f);
					} else {
						timer = ((timer_get_milliseconds() - shipp->shader_effect_start_time) / (float)shipp->shader_effect_duration);
					}

					model_interp_set_animated_effect_and_timer(sep->shader_effect, timer);

					if (sep->disables_rendering && (timer_get_milliseconds() > shipp->shader_effect_start_time + shipp->shader_effect_duration) ) {
						shipp->flags2 |= SF2_CLOAKED;
						shipp->shader_effect_active = false;
					} else {
						shipp->flags2 &= ~SF2_CLOAKED;
						if (timer_get_milliseconds() > shipp->shader_effect_start_time + shipp->shader_effect_duration)
							shipp->shader_effect_active = false;
					}
				}

				if (sip->uses_team_colors) {
					model_interp_set_team_color(shipp->team_name, shipp->secondary_team_name, shipp->team_change_timestamp, shipp->team_change_time);
				}

				if(sip->flags2 & SIF2_NO_LIGHTING)
					render_flags |= MR_DEPRECATED_NO_LIGHTING;
			}
			if(Rendering_to_shadow_map)
				render_flags = MR_DEPRECATED_NO_TEXTURING | MR_DEPRECATED_NO_LIGHTING;

			//draw weapon models
			if ((sip->flags2 & SIF2_DRAW_WEAPON_MODELS) && !(shipp->flags2 & SF2_CLOAKED)) {
				int i,k;
				ship_weapon *swp = &shipp->weapons;
				g3_start_instance_matrix(&obj->pos, &obj->orient, true);
			
				int save_flags = render_flags;
		
				render_flags &= ~MR_DEPRECATED_SHOW_THRUSTERS;
				render_flags |= MR_DEPRECATED_ATTACHED_MODEL;

				//primary weapons
				for (i = 0; i < swp->num_primary_banks; i++) {
					if (Weapon_info[swp->primary_bank_weapons[i]].external_model_num == -1 || !sip->draw_primary_models[i])
						continue;

					w_bank *bank = &model_get(sip->model_num)->gun_banks[i];
					for(k = 0; k < bank->num_slots; k++) {	
						polymodel* pm = model_get(Weapon_info[swp->primary_bank_weapons[i]].external_model_num);
						pm->gun_submodel_rotation = shipp->primary_rotate_ang[i];
						model_render_DEPRECATED(Weapon_info[swp->primary_bank_weapons[i]].external_model_num, &vmd_identity_matrix, &bank->pnt[k], render_flags, shipp->objnum);
						pm->gun_submodel_rotation = 0.0f;
					}
				}

				//secondary weapons
		        int num_secondaries_rendered = 0;
                vec3d secondary_weapon_pos;
                w_bank* bank;

				for (i = 0; i < swp->num_secondary_banks; i++) {
					if (Weapon_info[swp->secondary_bank_weapons[i]].external_model_num == -1 || !sip->draw_secondary_models[i])
						continue;

					bank = &(model_get(sip->model_num))->missile_banks[i];
					
					if (Weapon_info[swp->secondary_bank_weapons[i]].wi_flags2 & WIF2_EXTERNAL_WEAPON_LNCH) {
						for(k = 0; k < bank->num_slots; k++) {
							model_render_DEPRECATED(Weapon_info[swp->secondary_bank_weapons[i]].external_model_num, &vmd_identity_matrix, &bank->pnt[k], render_flags, shipp->objnum);
						}
					} else {
						num_secondaries_rendered = 0;
						
						for(k = 0; k < bank->num_slots; k++)
						{
							secondary_weapon_pos = bank->pnt[k];

							if (num_secondaries_rendered >= shipp->weapons.secondary_bank_ammo[i])
								break;

							if(shipp->secondary_point_reload_pct[i][k] <= 0.0)
								continue;
							
							num_secondaries_rendered++;
			
							vm_vec_scale_add2(&secondary_weapon_pos, &vmd_z_vector, -(1.0f-shipp->secondary_point_reload_pct[i][k]) * model_get(Weapon_info[swp->secondary_bank_weapons[i]].external_model_num)->rad);

							model_render_DEPRECATED(Weapon_info[swp->secondary_bank_weapons[i]].external_model_num, &vmd_identity_matrix, &secondary_weapon_pos, render_flags, shipp->objnum);
						}
					}
				}
				g3_done_instance(true);
				render_flags = save_flags;
			}

			// small ships
			if (!(shipp->flags2 & SF2_CLOAKED)) {
				if ((The_mission.flags & MISSION_FLAG_FULLNEB) && (sip->flags & SIF_SMALL_SHIP)) {			
					// force detail levels
 					float fog_val = neb2_get_fog_intensity(obj);
					if(fog_val >= 0.6f){
						model_set_detail_level(2);
						model_render_DEPRECATED( sip->model_num, &obj->orient, &obj->pos, render_flags, OBJ_INDEX(obj), -1, shipp->ship_replacement_textures, ship_render_mode );
					} else {
						model_render_DEPRECATED( sip->model_num, &obj->orient, &obj->pos, render_flags, OBJ_INDEX(obj), -1, shipp->ship_replacement_textures, ship_render_mode );
					}
				} else {
					model_render_DEPRECATED( sip->model_num, &obj->orient, &obj->pos, render_flags, OBJ_INDEX(obj), -1, shipp->ship_replacement_textures, ship_render_mode );
				}
			}

			// always turn off fog after rendering a ship
			gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
			light_set_shadow(0);

			#ifndef NDEBUG
			if (Show_shield_mesh)
				ship_draw_shield( obj);		//	Render the shield.
			#endif

			if ( clip_started )	{
				g3_stop_user_clip_plane();
			}
		} 
		
		ship_model_stop(obj);

		if (shipp->shield_hits && !Rendering_to_shadow_map) {
			create_shield_explosion_all(obj);
			shipp->shield_hits = 0;
		}

	#ifndef NDEBUG
		if (Ai_render_debug_flag || Show_paths) {
			if ( shipp->ai_index != -1 ){
				render_path_points(obj);
			}

			render_dock_bays(obj);
		}
	#endif
		
	#ifndef NDEBUG
		if(Show_tnorms){
			ship_subsys *systemp;
			vec3d tpos, tnorm, temp;
			vec3d v1, v2;
			vertex l1, l2;

			gr_set_color(0, 0, 255);
			systemp = GET_FIRST( &shipp->subsys_list );		
			while ( systemp != END_OF_LIST(&shipp->subsys_list) ) {
				ship_get_global_turret_gun_info(obj, systemp, &tpos, &tnorm, 1, &temp);
				
				v1 = tpos;
				vm_vec_scale_add(&v2, &v1, &tnorm, 20.0f);

				g3_rotate_vertex(&l1, &v1);
				g3_rotate_vertex(&l2, &v2);

				g3_draw_sphere(&l1, 2.0f);
				g3_draw_line(&l1, &l2);

				systemp = GET_NEXT(systemp);
			}
		}
	#endif
	}

	//WMC - Draw animated warp effect (ie BSG thingy)
	//WMC - based on Bobb's secondary thruster stuff
	//which was in turn based on the beam code.
	//I'm gonna need some serious acid to neutralize this base.
	if(shipp->flags & SF_ARRIVING)
		shipp->warpin_effect->warpShipRender();
	else if(shipp->flags & SF_DEPART_WARP)
		shipp->warpout_effect->warpShipRender();
}

void ship_render_cockpit(object *objp)
{
	if(objp->type != OBJ_SHIP || objp->instance < 0)
		return;

	ship *shipp = &Ships[objp->instance];
	ship_info *sip = &Ship_info[shipp->ship_info_index];

	if(sip->cockpit_model_num < 0)
		return;

	polymodel *pm = model_get(sip->cockpit_model_num);
	Assert(pm != NULL);

	//Setup
	gr_reset_clip();
	hud_save_restore_camera_data(1);

	matrix eye_ori = vmd_identity_matrix;
	vec3d eye_pos = vmd_zero_vector;
	ship_get_eye(&eye_pos, &eye_ori, objp, false, true);

	vec3d pos = vmd_zero_vector;

	vm_vec_unrotate(&pos, &sip->cockpit_offset, &eye_ori);
	if (!Cmdline_nohtl)
	{
		gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, 0.02f, 10.0f*pm->rad);
		gr_set_view_matrix(&vmd_zero_vector, &Eye_matrix);
	}

	//Deal with the model
	model_clear_instance(sip->cockpit_model_num);

	model_render_params render_info;
	render_info.set_detail_level_lock(0);
	render_info.set_flags(MR_NO_FOGGING);
	render_info.set_replacement_textures(Player_cockpit_textures);

	model_render_immediate(&render_info, sip->cockpit_model_num, &eye_ori, &pos);

	if (!Cmdline_nohtl) 
	{
		gr_end_view_matrix();
		gr_end_proj_matrix();
	}

	hud_save_restore_camera_data(0);
}

void ship_render_show_ship_cockpit(object *objp)
{
	vec3d cockpit_eye_pos;
	matrix dummy;
	ship_get_eye(&cockpit_eye_pos, &dummy, objp, true, true); //Get cockpit eye position
	
	gr_end_view_matrix();
	gr_end_proj_matrix();
	gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, 0.05f, Max_draw_distance);
	gr_set_view_matrix(&cockpit_eye_pos, &Eye_matrix); // Set Camera to cockpit eye position
	
	Glowpoint_override = true; // Turn off glowpoints so they dont get rendered fixed at origin

	model_render_params render_info;

	render_info.set_object_number(OBJ_INDEX(objp));
	render_info.set_detail_level_lock(0);

	model_render_immediate(&render_info, Ship_info[Ships[objp->instance].ship_info_index].model_num, &objp->orient, &vmd_zero_vector); // Render ship model with fixed detail level 0 so its not switching LOD when moving away from origin
	Glowpoint_override = false;

	gr_end_view_matrix();
	gr_end_proj_matrix();
	gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix); // Reset Camera to normal
}

void ship_init_cockpit_displays(ship *shipp)
{
	ship_info *sip = &Ship_info[shipp->ship_info_index];

	int cockpit_model_num = sip->cockpit_model_num;

	// don't bother creating cockpit texture replacements if this ship has no cockpit
	if ( cockpit_model_num < 0 ) {
		return;
	}

	// check if we even have cockpit displays
	if ( sip->displays.size() <= 0 ) {
		return;
	}

	if ( Player_cockpit_textures != NULL) {
		return;
	}

	// ship's cockpit texture replacements haven't been setup yet, so do it.
	Player_cockpit_textures = (int *) vm_malloc(MAX_REPLACEMENT_TEXTURES * sizeof(int));

	int i;

	for ( i = 0; i < MAX_REPLACEMENT_TEXTURES; i++ ) {
		Player_cockpit_textures[i] = -1;
	}

	for ( i = 0; i < (int)sip->displays.size(); i++ ) {
		ship_add_cockpit_display(&sip->displays[i], cockpit_model_num);
	}

	ship_set_hud_cockpit_targets();
}

void ship_clear_cockpit_displays()
{
	for ( int i = 0; i < (int)Player_displays.size(); i++ ) {
		if ( Player_displays[i].background >= 0 ) {
			bm_release(Player_displays[i].background);
		}

		if ( Player_displays[i].foreground >= 0 ) {
			bm_release(Player_displays[i].foreground);
		}

		if ( Player_displays[i].target >= 0 ) {
			bm_release(Player_displays[i].target);
		}
	}

	Player_displays.clear();

	if ( Player_cockpit_textures != NULL ) {
		vm_free(Player_cockpit_textures);
		Player_cockpit_textures = NULL;
	}
}

void ship_add_cockpit_display(cockpit_display_info *display, int cockpit_model_num)
{
	if ( strlen(display->filename) <= 0 ) {
		return;
	}

	if( cockpit_model_num < 0 ) { 
		return;
	}

	int i, tm_num, glow_target = -1, glow_handle = -1, diffuse_handle = -1;
	int w, h;
	cockpit_display new_display;

	// if no texture target has been found yet, find one.
	polymodel *pm = model_get(cockpit_model_num);

	for ( i = 0; i < pm->n_textures; i++ )
	{
		tm_num = pm->maps[i].FindTexture(display->filename);
		if ( tm_num >= 0 ) {
			glow_target = i*TM_NUM_TYPES+TM_GLOW_TYPE;

			diffuse_handle = pm->maps[i].textures[TM_BASE_TYPE].GetTexture();
			glow_handle = pm->maps[i].textures[TM_GLOW_TYPE].GetTexture();
			break;
		}
	}

	// create a render target for this cockpit texture
	if ( Player_cockpit_textures[glow_target] < 0) {

		bm_get_info(diffuse_handle, &w, &h);
		Player_cockpit_textures[glow_target] = bm_make_render_target(w, h, BMP_FLAG_RENDER_TARGET_DYNAMIC);

		// if no render target was made, bail
		if ( Player_cockpit_textures[glow_target] < 0 ) {
			return;
		}
	}

	new_display.background = -1;
	if ( display->bg_filename[0] != '\0' ) {
		new_display.background = bm_load(display->bg_filename);

		if ( new_display.background < 0 ) {
			Warning(LOCATION, "Unable to load background %s for cockpit display %s", display->bg_filename, display->name);
		}
	}

	new_display.foreground = -1;
	if ( display->fg_filename[0] != '\0' ) {
		new_display.foreground = bm_load(display->fg_filename);

		if ( new_display.foreground < 0 ) {
			Warning(LOCATION, "Unable to load background %s for cockpit display %s", display->fg_filename, display->name);
		}
	}
	
	strcpy_s(new_display.name, display->name);
	new_display.offset[0] = display->offset[0];
	new_display.offset[1] = display->offset[1];
	new_display.size[0] = display->size[0];
	new_display.size[1] = display->size[1];
	new_display.source = glow_handle;
	new_display.target = Player_cockpit_textures[glow_target];

	Player_displays.push_back(new_display);
}

void ship_set_hud_cockpit_targets()
{
	if ( !Ship_info[Player_ship->ship_info_index].hud_enabled ) {
		return;
	}

	SCP_vector<HudGauge*> &hud = Ship_info[Player_ship->ship_info_index].hud_gauges;

	for ( int i = 0; i < (int)hud.size(); i++ ) {
		for ( int j = 0; j < (int)Player_displays.size(); j++ ) {
			hud[i]->setCockpitTarget(&Player_displays[j]);
		}
	}
}

int ship_start_render_cockpit_display(int cockpit_display_num)
{
	// make sure this thing even has a cockpit
	if ( Ship_info[Player_ship->ship_info_index].cockpit_model_num < 0 ) {
		return -1;
	}

	if ( Player_cockpit_textures == NULL ) {
		return -1;
	}

	// check sanity of the cockpit display handle
	if ( cockpit_display_num >= (int)Player_displays.size() || cockpit_display_num < 0 ) {
		return -1;
	}

	cockpit_display* display = &Player_displays[cockpit_display_num];

	if ( display->target < 0 ) {
		return -1;
	}

	if ( !bm_set_render_target(display->target) ) {
		return -1;
	}
	
	int cull = gr_set_cull(0);

	gr_clear();
	
	if ( display->source >= 0 ) {
		gr_set_bitmap(display->source);
		gr_bitmap(0, 0, GR_RESIZE_NONE);
	}

	if ( display->background >= 0 ) {
		gr_set_bitmap(display->background);
		gr_bitmap_ex(display->offset[0], display->offset[1], display->size[0], display->size[1], 0, 0, GR_RESIZE_NONE);
	}

	gr_set_cull(cull);

	return display->target;
}

void ship_end_render_cockpit_display(int cockpit_display_num)
{
	// make sure this thing even has a cockpit
	if ( Ship_info[Player_ship->ship_info_index].cockpit_model_num < 0 ) {
		return;
	}

	if ( Player_cockpit_textures == NULL ) {
		return;
	}

	// check sanity of the cockpit display handle
	if ( cockpit_display_num >= (int)Player_displays.size() || cockpit_display_num < 0 ) {
		return;
	}

	cockpit_display* display = &Player_displays[cockpit_display_num];

	int cull = gr_set_cull(0);
	if ( display->foreground >= 0 ) {
		gr_reset_clip();
		gr_set_bitmap(display->foreground);
		gr_bitmap_ex(display->offset[0], display->offset[1], display->size[0], display->size[1], 0, 0, GR_RESIZE_NONE);
	}

	gr_set_cull(cull);
	bm_set_render_target(-1);
}

void ship_subsystems_delete(ship *shipp)
{
	if ( NOT_EMPTY(&shipp->subsys_list) )
	{
		ship_subsys *systemp, *temp;

		systemp = GET_FIRST( &shipp->subsys_list );
		while ( systemp != END_OF_LIST(&shipp->subsys_list) ) {
			temp = GET_NEXT( systemp );								// use temporary since pointers will get screwed with next operation
			list_remove( shipp->subsys_list, systemp );			// remove the element
			list_append( &ship_subsys_free_list, systemp );		// and place back onto free list
			Num_ship_subsystems--;								// subtract from our in-use total
			systemp = temp;												// use the temp variable to move right along
		}
	}
}

void ship_delete( object * obj )
{
	ship	*shipp;
	int	num, objnum __UNUSED;

	num = obj->instance;
	Assert( num >= 0);

	objnum = OBJ_INDEX(obj);
	Assert( Ships[num].objnum == objnum );

	shipp = &Ships[num];

	if (shipp->ai_index != -1){
		ai_free_slot(shipp->ai_index);
	}	

	// free up the list of subsystems of this ship.  walk through list and move remaining subsystems
	// on ship back to the free list for other ships to use.
	ship_subsystems_delete(&Ships[num]);
	shipp->objnum = -1;

	if (shipp->shield_integrity != NULL) {
		vm_free(shipp->shield_integrity);
		shipp->shield_integrity = NULL;
	}

	if (shipp->ship_replacement_textures != NULL) {
		vm_free(shipp->ship_replacement_textures);
		shipp->ship_replacement_textures = NULL;
	}

	// glow point banks
	shipp->glow_point_bank_active.clear();

	if ( shipp->ship_list_index != -1 ) {
		ship_obj_list_remove(shipp->ship_list_index);
		shipp->ship_list_index = -1;
	}

	free_sexp2(shipp->arrival_cue);
	free_sexp2(shipp->departure_cue);

	// call the contrail system
	ct_ship_delete(shipp);
	
	model_delete_instance(shipp->model_instance_num);
}

/**
 * Used by ::ship_cleanup which is called if the ship is in a wing.
 *
 * This function updates the ship_index list (i.e. removes its entry in the list)
 * and packs the array accordingly.
 */
void ship_wing_cleanup( int shipnum, wing *wingp )
{
	int i, index = -1, team = Ships[shipnum].team;

	// find this ship's position within its wing
	for (i = 0; i < wingp->current_count; i++)
	{
		if (wingp->ship_index[i] == shipnum)
		{
			index = i;
			break;
		}
	}

	// this can happen in multiplayer (dogfight, ingame join specifically)
	if (index == -1)
		return;

	// compress the ship_index array and mark the last entry with a -1
	for (i = index; i < wingp->current_count - 1; i++)
		wingp->ship_index[i] = wingp->ship_index[i+1];

	wingp->current_count--;
	Assert ( wingp->current_count >= 0 );
	wingp->ship_index[wingp->current_count] = -1;

	// if the current count is 0, check to see if the wing departed or was destroyed.
	if (wingp->current_count == 0)
	{
		// if this wing was ordered to depart by the player, set the current_wave equal to the total
		// waves so we can mark the wing as gone and no other ships arrive
		// Goober5000 - also if it's departing... this is sort of, but not exactly, what :V: did;
		// but it seems to be consistent with how it should behave
		if (wingp->flags & (WF_WING_DEPARTING | WF_DEPARTURE_ORDERED))
			wingp->current_wave = wingp->num_waves;

		// Goober5000 - some changes for clarity and closing holes
		// make sure to flag the wing as gone if all of its member ships are gone and no more can arrive
		if ((wingp->current_wave == wingp->num_waves) && (wingp->total_destroyed + wingp->total_departed + wingp->total_vanished == wingp->total_arrived_count))
		{
			// mark the wing as gone
			wingp->flags |= WF_WING_GONE;
			wingp->time_gone = Missiontime;

			// if all ships were destroyed, log it as destroyed
			if (wingp->total_destroyed == wingp->total_arrived_count)
			{
				// first, be sure to mark a wing destroyed event if all members of wing were destroyed and on
				// the last wave.  This circumvents a problem where the wing could be marked as departed and
				// destroyed if the last ships were destroyed after the wing's departure cue became true.
				mission_log_add_entry(LOG_WING_DESTROYED, wingp->name, NULL, team);
			}
			// if some ships escaped, log it as departed
			else if (wingp->total_vanished != wingp->total_arrived_count)
			{
				// if the wing wasn't destroyed, and it is departing, then mark it as departed -- in this
				// case, there had better be ships in this wing with departure entries in the log file.  The
				// logfile code checks for this case.  
				mission_log_add_entry(LOG_WING_DEPARTED, wingp->name, NULL, team);
			}

#ifndef NDEBUG
			//WMC - Ships can depart too, besides being destroyed :P
            if ((wingp->total_destroyed + wingp->total_departed + wingp->total_vanished) != wingp->total_arrived_count)
			{
				// apparently, there have been reports of ships still present in the mission when this log
				// entry if written.  Do a sanity check here to find out for sure.
				for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
				{
					// skip the player -- stupid special case.
					if (&Objects[so->objnum] == Player_obj)
						continue;
	
					if ((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_INGAME_JOIN))
						continue;
	
					if ((Ships[Objects[so->objnum].instance].wingnum == WING_INDEX(wingp)) && !(Ships[Objects[so->objnum].instance].flags & (SF_DEPARTING|SF_DYING)))
					{
						// TODO: I think this Int3() is triggered when a wing whose ships are all docked to ships of another
						// wing departs.  It can be reliably seen in TVWP chapter 1 mission 7, when Torino and Iota wing depart.
						// Not sure how to fix this. -- Goober5000
						Int3();
					}
				}
			}
#endif
		}
	}
}

// functions to do management, like log entries and wing cleanup after a ship has been destroyed

// Goober5000
void ship_actually_depart_helper(object *objp, dock_function_info *infop)
{
	// do standard departure stuff first
	objp->flags |= OF_SHOULD_BE_DEAD;	
	if (objp->type == OBJ_SHIP)
		ship_cleanup(objp->instance, infop->parameter_variables.bool_value ? SHIP_VANISHED : SHIP_DEPARTED);

	// do the end-mission stuff if it's the player ship
	if (objp == Player_obj)
		gameseq_post_event(GS_EVENT_PLAYER_WARPOUT_DONE);
}

/**
 * Used to actually remove a ship, plus all the ships it's docked to, from the mission
 */
void ship_actually_depart(int shipnum, int method)
{
	dock_function_info dfi;
	dfi.parameter_variables.bool_value = (method == SHIP_VANISHED ? true:false);
	dock_evaluate_all_docked_objects(&Objects[Ships[shipnum].objnum], &dfi, ship_actually_depart_helper);

	// in a couple of cases we'll need to send a packet to update clients 
	if (MULTIPLAYER_MASTER && ((method == SHIP_DEPARTED_BAY) || (method == SHIP_VANISHED)) ) {
		send_ship_depart_packet(&Objects[Ships[shipnum].objnum], method); 
	}
}

// no destruction effects, not for player destruction and multiplayer, only self-destruction
void ship_destroy_instantly(object *ship_objp, int shipnum)
{
	Assert(ship_objp->type == OBJ_SHIP);
	Assert(!(ship_objp == Player_obj));
	Assert(!(Game_mode & GM_MULTIPLAYER));

	// undocking and death preparation
	ship_stop_fire_primary(ship_objp);
	ai_deathroll_start(ship_objp);

	mission_log_add_entry(LOG_SELF_DESTRUCTED, Ships[ship_objp->instance].ship_name, NULL );
	
	// scripting stuff
	Script_system.SetHookObject("Self", ship_objp);
	Script_system.RunCondition(CHA_DEATH, 0, NULL, ship_objp);
	Script_system.RemHookVar("Self");

	ship_objp->flags |= OF_SHOULD_BE_DEAD;
	ship_cleanup(shipnum,SHIP_DESTROYED);
}


/**
 * Merge ship_destroyed and ship_departed and ship_vanished
 */
void ship_cleanup(int shipnum, int cleanup_mode)
{
	Assert(shipnum >= 0 && shipnum < MAX_SHIPS);
	Assert(cleanup_mode == SHIP_DESTROYED || cleanup_mode == SHIP_DEPARTED || cleanup_mode == SHIP_VANISHED);
	Assert(Objects[Ships[shipnum].objnum].type == OBJ_SHIP);
	Assert(Objects[Ships[shipnum].objnum].flags & OF_SHOULD_BE_DEAD);

	ship *shipp = &Ships[shipnum];
	object *objp = &Objects[shipp->objnum];

	// add the information to the exited ship list
	if (cleanup_mode == SHIP_DESTROYED) {
		ship_add_exited_ship(shipp, SEF_DESTROYED);
	} else {
		ship_add_exited_ship(shipp, SEF_DEPARTED);
	}

	// record kill?
	if (cleanup_mode == SHIP_DESTROYED) {
		// determine if we need to count this ship as a kill in counting number of kills per ship type
		// look at the ignore flag for the ship (if not in a wing), or the ignore flag for the wing
		// (if the ship is in a wing), and add to the kill count if the flags are not set
		if ( !(shipp->flags & SF_IGNORE_COUNT) || ((shipp->wingnum != -1) && !(Wings[shipp->wingnum].flags & WF_IGNORE_COUNT)) )
			ship_add_ship_type_kill_count( shipp->ship_info_index );

		// let the event music system know an enemy was destroyed (important for deciding when to transition from battle to normal music)
		if (Player_ship != NULL && iff_x_attacks_y(Player_ship->team, shipp->team))
			event_music_hostile_ship_destroyed();
	}

	// add mission log entry?
	// (vanished ships have no log, and destroyed ships are logged in ship_hit_kill)
	if (cleanup_mode == SHIP_DEPARTED) {
		// see if this ship departed within the radius of a jump node -- if so, put the node name into
		// the secondary mission log field
		CJumpNode *jnp = jumpnode_get_which_in(objp);
		if(jnp==NULL)
			mission_log_add_entry(LOG_SHIP_DEPARTED, shipp->ship_name, NULL, shipp->wingnum);
		else
			mission_log_add_entry(LOG_SHIP_DEPARTED, shipp->ship_name, jnp->GetName(), shipp->wingnum);
	}

#ifndef NDEBUG
	// add a debug log entry
	if (cleanup_mode == SHIP_DESTROYED) {
		nprintf(("Alan", "SHIP DESTROYED: %s\n", shipp->ship_name));
	} else if (cleanup_mode == SHIP_DEPARTED) {
		nprintf(("Alan", "SHIP DEPARTED: %s\n", shipp->ship_name));
	} else {
		nprintf(("Alan", "SHIP VANISHED: %s\n", shipp->ship_name));
	}
#endif

	// update wingman status gauge
	if ( (shipp->wing_status_wing_index >= 0) && (shipp->wing_status_wing_pos >= 0) ) {
		if (cleanup_mode == SHIP_DESTROYED) {
			hud_set_wingman_status_dead(shipp->wing_status_wing_index, shipp->wing_status_wing_pos);
		} else if (cleanup_mode == SHIP_DEPARTED) {
			hud_set_wingman_status_departed(shipp->wing_status_wing_index, shipp->wing_status_wing_pos);
		} else {
			hud_set_wingman_status_none(shipp->wing_status_wing_index, shipp->wing_status_wing_pos);
		}
	}

	// if ship belongs to a wing, do the wing cleanup
	if ( shipp->wingnum != -1 ) {
		wing *wingp = &Wings[shipp->wingnum];

		if (cleanup_mode == SHIP_DESTROYED) {
			wingp->total_destroyed++;
		} else if (cleanup_mode == SHIP_DEPARTED) {
			wingp->total_departed++;
		} else {
			wingp->total_vanished++;
		}

		ship_wing_cleanup(shipnum, wingp);
	}

	// Note, this call to ai_ship_destroy must come after ship_wing_cleanup for guarded wings to
	// properly note the destruction of a ship in their wing.
	if (cleanup_mode == SHIP_DESTROYED) {
		ai_ship_destroy(shipnum, SEF_DESTROYED);	// Do AI stuff for destruction of ship.
	} else {
		ai_ship_destroy(shipnum, SEF_DEPARTED);		// should still do AI cleanup after ship has departed
	}

	// Goober5000 - lastly, clear out the dead-docked list, per Mantis #2294
	// (for exploding ships, this list should have already been cleared by now, via
	// do_dying_undock_physics, except in the case of the destroy-instantly sexp)
	while (object_is_dead_docked(objp))
	{
		object *docked_objp = dock_get_first_dead_docked_object(objp);
		dock_dead_undock_objects(objp, docked_objp);
	}
}

/**
 * Calculates the blast and damage applied to a ship from another ship blowing up.
 * 
 * @param pos1			ship explosion position
 * @param pos2			other ship position
 * @param inner_rad		distance from ship center for which full damage is applied
 * @param outer_rad		distance from ship center for which no damage is applied
 * @param max_damage	maximum damage applied
 * @param max_blast		maximum impulse applied from blast
 * @param damage		damage applied
 * @param blast			impulse applied from blast
 */
int ship_explode_area_calc_damage( vec3d *pos1, vec3d *pos2, float inner_rad, float outer_rad, float max_damage, float max_blast, float *damage, float *blast )
{
	float dist;

	dist = vm_vec_dist_quick( pos1, pos2 );

	// check outside outer radius
	if ( dist > outer_rad )
		return -1;

	if ( dist < inner_rad ) {
	// check insider inner radius
		*damage = max_damage;
		*blast = max_blast;
	} else {
	// between inner and outer
		float fraction = 1.0f - (dist - inner_rad) / (outer_rad - inner_rad);
		*damage  = fraction * max_damage;
		*blast   = fraction * max_blast;
	}

	return 1;
}

static const float MAX_SHOCK_ANGLE_RANGE = 1.99f * PI;

/**
 * Applies damage to ship close to others when a ship dies and blows up
 *
 * @param exp_objp			ship object pointers
 */
void ship_blow_up_area_apply_blast( object *exp_objp)
{
	ship *shipp;
	ship_info *sip;
	float	inner_rad, outer_rad, max_damage, max_blast, shockwave_speed;

	//	No area explosion in training missions.
	if (The_mission.game_type & MISSION_TYPE_TRAINING){
		return;
	}

	Assert( exp_objp != NULL );
	Assert( exp_objp->type == OBJ_SHIP );
	Assert( exp_objp->instance >= 0 );

	shipp = &Ships[exp_objp->instance];
	sip = &Ship_info[shipp->ship_info_index];

	Assert( (shipp != NULL) && (sip != NULL) );

	if ((exp_objp->hull_strength <= KAMIKAZE_HULL_ON_DEATH) && (Ai_info[Ships[exp_objp->instance].ai_index].ai_flags & AIF_KAMIKAZE) && (shipp->special_exp_damage == -1)) {
		int override = Ai_info[shipp->ai_index].kamikaze_damage;

		inner_rad = exp_objp->radius*2.0f;
		outer_rad = exp_objp->radius*4.0f; // + (override * 0.3f);
		max_damage = i2fl(override);
		max_blast = override * 5.0f;
		shockwave_speed = 100.0f;
	} else {
		if (shipp->use_special_explosion) {
			inner_rad = i2fl(shipp->special_exp_inner);
			outer_rad = i2fl(shipp->special_exp_outer);
			max_damage = i2fl(shipp->special_exp_damage);
			max_blast = i2fl(shipp->special_exp_blast);
			shockwave_speed = i2fl(shipp->special_exp_shockwave_speed);
		} else {
			inner_rad = sip->shockwave.inner_rad;
			outer_rad = sip->shockwave.outer_rad;
			max_damage = sip->shockwave.damage;
			max_blast  = sip->shockwave.blast;
			shockwave_speed = sip->shockwave.speed;
		}
	}

	// account for ships that give no damage when they blow up.
	if ( (max_damage < 0.1f) && (max_blast < 0.1f) ){
		return;
	}

	if ( shockwave_speed > 0 ) {
		shockwave_create_info sci = sip->shockwave;

		sci.inner_rad = inner_rad;
		sci.outer_rad = outer_rad;
		sci.blast = max_blast;
		sci.damage = max_damage;
		sci.speed = shockwave_speed;
		sci.rot_angles.p = frand_range(0.0f, MAX_SHOCK_ANGLE_RANGE);
		sci.rot_angles.b = frand_range(0.0f, MAX_SHOCK_ANGLE_RANGE);
		sci.rot_angles.h = frand_range(0.0f, MAX_SHOCK_ANGLE_RANGE);
		shipfx_do_shockwave_stuff(shipp, &sci);
	} else {
		object *objp;
		float blast = 0.0f;
		float damage = 0.0f;
		for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID) ) {
				continue;
			}
		
			if ( objp == exp_objp ){
				continue;
			}

			// don't blast navbuoys
			if ( objp->type == OBJ_SHIP ) {
				if ( ship_get_SIF(objp->instance) & SIF_NAVBUOY ) {
					continue;
				}
			}

			if ( ship_explode_area_calc_damage( &exp_objp->pos, &objp->pos, inner_rad, outer_rad, max_damage, max_blast, &damage, &blast ) == -1 ){
				continue;
			}

			switch ( objp->type ) {
			case OBJ_SHIP:
				ship_apply_global_damage( objp, exp_objp, &exp_objp->pos, damage );
				vec3d force, vec_ship_to_impact;
				vm_vec_sub( &vec_ship_to_impact, &objp->pos, &exp_objp->pos );
				vm_vec_copy_normalize( &force, &vec_ship_to_impact );
				vm_vec_scale( &force, blast );
				ship_apply_whack( &force, &vec_ship_to_impact, objp );
				break;
			case OBJ_ASTEROID:
				asteroid_hit(objp, NULL, NULL, damage);
				break;
			default:
				Int3();
				break;
			}
		}	// end for
	}
}

/**
 * Only ever called once for any ship that dies
 *
 * This function relies on the "dead dock" list, which replaces the dock_objnum_when_dead
 * used in retail.
 */
void do_dying_undock_physics(object *dying_objp, ship *dying_shipp) 
{
	// this function should only be called for an object that was docked...
	// no harm in calling it if it wasn't, but we want to enforce this
	Assert(object_is_dead_docked(dying_objp));

	object *docked_objp;

	float damage;
	float impulse_mag;

	vec3d impulse_norm, impulse_vec, pos;

	// damage applied to each docked object
	damage = 0.2f * dying_shipp->ship_max_hull_strength;

	// Goober5000 - as with ai_deathroll_start, we can't simply iterate through the dock list while we're
	// unlinking things.  So just repeatedly unlink the first object.
	while (object_is_dead_docked(dying_objp))
	{
		docked_objp = dock_get_first_dead_docked_object(dying_objp);
		ship *docked_shipp = &Ships[docked_objp->instance];
		int dockee_index = dock_find_dead_dockpoint_used_by_object(docked_objp, dying_objp);

		// undo all the docking animations for the docked ship only
		model_anim_start_type(docked_shipp, TRIGGER_TYPE_DOCKED, dockee_index, -1);
		model_anim_start_type(docked_shipp, TRIGGER_TYPE_DOCKING_STAGE_3, dockee_index, -1);
		model_anim_start_type(docked_shipp, TRIGGER_TYPE_DOCKING_STAGE_2, dockee_index, -1);
		model_anim_start_type(docked_shipp, TRIGGER_TYPE_DOCKING_STAGE_1, dockee_index, -1);

		// only consider the mass of these two objects, not the whole assembly
		// (this is inaccurate, but the alternative is a huge mess of extra code for a very small gain in realism)
		float docked_mass = dying_objp->phys_info.mass + docked_objp->phys_info.mass;

		// damage this docked object
		ship_apply_global_damage(docked_objp, dying_objp, &dying_objp->pos, damage);

		// do physics
		vm_vec_sub(&impulse_norm, &docked_objp->pos, &dying_objp->pos);
		vm_vec_normalize(&impulse_norm);
		// set for relative separation velocity of ~30
		impulse_mag = 50.f * docked_objp->phys_info.mass * dying_objp->phys_info.mass / docked_mass;
		vm_vec_copy_scale(&impulse_vec, &impulse_norm, impulse_mag);
		vm_vec_rand_vec_quick(&pos);
		vm_vec_scale(&pos, docked_objp->radius);
		// apply whack to docked object
		physics_apply_whack(&impulse_vec, &pos, &docked_objp->phys_info, &docked_objp->orient, docked_objp->phys_info.mass);
		// enhance rotation of the docked object
		vm_vec_scale(&docked_objp->phys_info.rotvel, 2.0f);

		// apply whack to dying object
		vm_vec_negate(&impulse_vec);
		vm_vec_rand_vec_quick(&pos);
		vm_vec_scale(&pos, dying_objp->radius);
		physics_apply_whack(&impulse_vec, &pos, &dying_objp->phys_info, &dying_objp->orient, dying_objp->phys_info.mass);

		// unlink the two objects, since dying_objp has blown up
		dock_dead_undock_objects(dying_objp, docked_objp);
	}
}

/**
 * Do the stuff we do in a frame for a ship that's in its death throes.
 */
void ship_dying_frame(object *objp, int ship_num)
{
	ship *shipp = &Ships[ship_num];

	if ( shipp->flags & SF_DYING )	{
		ship_info *sip = &Ship_info[shipp->ship_info_index];
		int knossos_ship = (sip->flags & SIF_KNOSSOS_DEVICE);

		// bash hull value toward 0 (from self destruct)
		if (objp->hull_strength > 0) {
			int time_left = timestamp_until(shipp->final_death_time);
			float hits_left = objp->hull_strength;

			objp->hull_strength -= hits_left * (1000.0f * flFrametime) / time_left;
		}

		// special case of VAPORIZE
		if (shipp->flags & SF_VAPORIZE) {
			if (timestamp_elapsed(shipp->final_death_time)) {
				// play death sound
				snd_play_3d( &Snds[SND_VAPORIZED], &objp->pos, &View_position, objp->radius, NULL, 0, 1.0f, SND_PRIORITY_MUST_PLAY  );

				// do joystick effect
				if (objp == Player_obj) {
					joy_ff_explode();
				}

				// if dying ship is docked, do damage to docked and physics
				if (object_is_dead_docked(objp))  {
					do_dying_undock_physics(objp, shipp);
				}			

				// do all accounting for respawning client and server side here.
				if (objp == Player_obj) {				
					gameseq_post_event(GS_EVENT_DEATH_BLEW_UP);
				}

				// mark object as dead
				objp->flags |= OF_SHOULD_BE_DEAD;

				// Don't blow up model.  Only use debris shards.
				// call ship function to clean up after the ship is destroyed.
				ship_cleanup(ship_num, SHIP_DESTROYED);
				return;
			} else {
				return;
			}
		}

		// bash the desired rotvel
		objp->phys_info.desired_rotvel = shipp->deathroll_rotvel;

		// Do fireballs for Big ship with propagating explostion, but not Kamikaze
		if (!(Ai_info[shipp->ai_index].ai_flags & AIF_KAMIKAZE) && ship_get_exp_propagates(shipp) && (sip->death_roll_r_mult > 0.0f)) {
			if ( timestamp_elapsed(shipp->next_fireball))	{
				vec3d outpnt, pnt1, pnt2;
				polymodel *pm = model_get(sip->model_num);

				// Gets two random points on the surface of a submodel
				if ( Cmdline_old_collision_sys ) {
					submodel_get_two_random_points(pm->id, pm->detail[0], &pnt1, &pnt2 );
				} else {
					submodel_get_two_random_points_better(pm->id, pm->detail[0], &pnt1, &pnt2 );
				}

				model_find_world_point(&outpnt, &pnt1, sip->model_num, pm->detail[0], &objp->orient, &objp->pos );

				float rad = objp->radius*0.1f;
				
				if (sip->death_roll_r_mult != 1.0f)
					rad *= sip->death_roll_r_mult;

				int fireball_type = fireball_ship_explosion_type(sip);
				if(fireball_type < 0) {
					fireball_type = FIREBALL_EXPLOSION_LARGE1 + rand()%FIREBALL_NUM_LARGE_EXPLOSIONS;
				}
				fireball_create( &outpnt, fireball_type, FIREBALL_LARGE_EXPLOSION, OBJ_INDEX(objp), rad, 0, &objp->phys_info.vel );
				// start the next fireball up in the next 50 - 200 ms (2-3 per frame)
				int min_time = 333;
				int max_time = 500;

				if (sip->death_roll_time_mult != 1.0f) {
					min_time = (int) (min_time / sip->death_roll_time_mult);
					max_time = (int) (max_time / sip->death_roll_time_mult);
				}

				shipp->next_fireball = timestamp_rand(min_time,max_time);

				// do sound - maybe start a random sound, if it has played far enough.
				do_sub_expl_sound(objp->radius, &outpnt, shipp->sub_expl_sound_handle);
			}
		}

		// create little fireballs for knossos as it dies
		if (knossos_ship) {
			if ( timestamp_elapsed(shipp->next_fireball)) {
				vec3d rand_vec, outpnt; // [0-.7 rad] in plane
				vm_vec_rand_vec_quick(&rand_vec);
				float scale = -vm_vec_dot(&objp->orient.vec.fvec, &rand_vec) * (0.9f + 0.2f * frand());
				vm_vec_scale_add2(&rand_vec, &objp->orient.vec.fvec, scale);
				vm_vec_normalize_quick(&rand_vec);
				scale = objp->radius * frand() * 0.717f;
				vm_vec_scale(&rand_vec, scale);
				vm_vec_add(&outpnt, &objp->pos, &rand_vec);

				float rad = objp->radius*0.2f;

				int fireball_type = fireball_ship_explosion_type(sip);
				if(fireball_type < 0) {
					fireball_type = FIREBALL_EXPLOSION_LARGE1 + rand()%FIREBALL_NUM_LARGE_EXPLOSIONS;
				}
				fireball_create( &outpnt, fireball_type, FIREBALL_LARGE_EXPLOSION, OBJ_INDEX(objp), rad, 0, &objp->phys_info.vel );
				// start the next fireball up in the next 50 - 200 ms (2-3 per frame)
				shipp->next_fireball = timestamp_rand(333,500);

				// emit particles
				particle_emitter	pe;
				particle_effect		pef = sip->knossos_end_particles;
				
				pe.num_low = pef.n_low;					// Lowest number of particles to create
				pe.num_high = pef.n_high;				// Highest number of particles to create
				pe.pos = outpnt;				// Where the particles emit from
				pe.vel = objp->phys_info.vel;	// Initial velocity of all the particles
				pe.min_life = pef.min_life;	// How long the particles live
				pe.max_life = pef.max_life;	// How long the particles live
				pe.normal = objp->orient.vec.uvec;	// What normal the particle emit around
				pe.normal_variance = pef.variance;		//	How close they stick to that normal 0=on normal, 1=180, 2=360 degree
				pe.min_vel = pef.min_vel;
				pe.max_vel = pef.max_vel;
				pe.min_rad = pef.min_rad;	// * objp->radius;
				pe.max_rad = pef.max_rad; // * objp->radius;

				if (pe.num_high > 0) {
					particle_emit( &pe, PARTICLE_SMOKE2, 0, 50 );
				}

				// do sound - maybe start a random sound, if it has played far enough.
				do_sub_expl_sound(objp->radius, &outpnt, shipp->sub_expl_sound_handle);
			}
		}

		int time_until_minor_explosions = timestamp_until(shipp->final_death_time);

		// Wait until just before death and set off some explosions
		// If it is less than 1/2 second until large explosion, but there is
		// at least 1/10th of a second left, then create 5 small explosions
		if ( (time_until_minor_explosions < 500) && (time_until_minor_explosions > 100) && (!shipp->pre_death_explosion_happened) ) {
			shipp->next_fireball = timestamp(-1);	// never time out again
			shipp->pre_death_explosion_happened=1;		// Mark this event as having occurred

			polymodel *pm = model_get(sip->model_num);

			// Start shockwave for ship with propagating explosion, do now for timing
			if ( ship_get_exp_propagates(shipp) ) {
				ship_blow_up_area_apply_blast( objp );
			}

			int zz_max = sip->death_fx_count;

			for (int zz=0; zz<zz_max; zz++ ) {
				// don't make sequence of fireballs for knossos
				if (knossos_ship) {
					break;
				}

				if (sip->death_fx_r_mult <= 0.0f) {
					break;
				}
				// Find two random vertices on the model, then average them
				// and make the piece start there.
				vec3d tmp, outpnt, pnt1, pnt2;

				// Gets two random points on the surface of a submodel [KNOSSOS]
				if ( Cmdline_old_collision_sys ) {
					submodel_get_two_random_points(pm->id, pm->detail[0], &pnt1, &pnt2 );
				} else {
					submodel_get_two_random_points_better(pm->id, pm->detail[0], &pnt1, &pnt2 );
				}

				vm_vec_avg( &tmp, &pnt1, &pnt2 );
				model_find_world_point(&outpnt, &tmp, pm->id, pm->detail[0], &objp->orient, &objp->pos );

				float rad = objp->radius*0.40f;

				rad *= sip->death_fx_r_mult;

				int fireball_type = fireball_ship_explosion_type(sip);
				if(fireball_type < 0) {
					fireball_type = FIREBALL_EXPLOSION_MEDIUM;
				}
				fireball_create( &outpnt, fireball_type, FIREBALL_MEDIUM_EXPLOSION, OBJ_INDEX(objp), rad, 0, &objp->phys_info.vel );
			}
		}

		if ( timestamp_elapsed(shipp->final_death_time))	{
			shipp->death_time = shipp->final_death_time;
			shipp->final_death_time = timestamp(-1);	// never time out again
			
			// play ship explosion sound effect, pick appropriate explosion sound
			int sound_index;

			if (ship_has_sound(objp, SND_SHIP_EXPLODE_1))
			{
				sound_index = ship_get_sound(objp, SND_SHIP_EXPLODE_1);
			}
			else
			{
				if ( sip->flags & (SIF_CAPITAL | SIF_KNOSSOS_DEVICE) ) {
					sound_index=SND_CAPSHIP_EXPLODE;
				} else {
					 if ( OBJ_INDEX(objp) & 1 ) {
						sound_index=SND_SHIP_EXPLODE_1;
					} else {
						sound_index=SND_SHIP_EXPLODE_2;
					}
				}
			}

			snd_play_3d( &Snds[sound_index], &objp->pos, &View_position, objp->radius, NULL, 0, 1.0f, SND_PRIORITY_MUST_PLAY  );
			if (objp == Player_obj)
				joy_ff_explode();

			if ( shipp->death_roll_snd != -1 ) {
				snd_stop(shipp->death_roll_snd);
				shipp->death_roll_snd = -1;
			}

			// if dying ship is docked, do damage to docked and physics
			if (object_is_dead_docked(objp))  {
				do_dying_undock_physics(objp, shipp);
			}			

			// play a random explosion
			particle_emitter	pe;
			particle_effect		pef = sip->regular_end_particles;

			pe.num_low = pef.n_low;					// Lowest number of particles to create
			pe.num_high = pef.n_high;				// Highest number of particles to create
			pe.pos = objp->pos;				// Where the particles emit from
			pe.vel = objp->phys_info.vel;	// Initial velocity of all the particles
			pe.min_life = pef.min_life;				// How long the particles live
			pe.max_life = pef.max_life;				// How long the particles live
			pe.normal = objp->orient.vec.uvec;	// What normal the particle emit around
			pe.normal_variance = pef.variance;		//	How close they stick to that normal 0=on normal, 1=180, 2=360 degree
			pe.min_vel = pef.min_vel;				// How fast the slowest particle can move
			pe.max_vel = pef.max_vel;				// How fast the fastest particle can move
			pe.min_rad = pef.min_rad;				// Min radius
			pe.max_rad = pef.max_rad;				// Max radius

			if ((!knossos_ship) && (pe.num_high > 0)) {
				particle_emit( &pe, PARTICLE_SMOKE2, 0 );
			}

			// If this is a large ship with a propagating explosion, set it to blow up.
			if ( ship_get_exp_propagates(shipp) )	{
				if (Ai_info[shipp->ai_index].ai_flags & AIF_KAMIKAZE) {
					ship_blow_up_area_apply_blast( objp );
				}
				shipfx_large_blowup_init(shipp);
				// need to timeout immediately to keep physics in sync
				shipp->really_final_death_time = timestamp(0);
				polymodel *pm = model_get(sip->model_num);
				shipp->end_death_time = timestamp((int) pm->core_radius);
			} else {
				// only do big fireball if not big ship
				float big_rad;
				int fireball_objnum, fireball_type, default_fireball_type;
				float explosion_life;
				big_rad = objp->radius*1.75f;

				default_fireball_type = FIREBALL_EXPLOSION_LARGE1 + rand()%FIREBALL_NUM_LARGE_EXPLOSIONS;
				if (knossos_ship) {
					big_rad = objp->radius * 1.2f;
					default_fireball_type = FIREBALL_EXPLOSION_LARGE1;
				}
				//SUSHI: Option to override radius of big fireball
				if (Ship_info[shipp->ship_info_index].big_exp_visual_rad >= 0)
					big_rad = Ship_info[shipp->ship_info_index].big_exp_visual_rad;

				fireball_type = fireball_ship_explosion_type(sip);
				if(fireball_type < 0) {
					fireball_type = default_fireball_type;
				}
				fireball_objnum = fireball_create( &objp->pos, fireball_type, FIREBALL_LARGE_EXPLOSION, OBJ_INDEX(objp), big_rad, 0, &objp->phys_info.vel );
				if ( fireball_objnum >= 0 )	{
					explosion_life = fireball_lifeleft(&Objects[fireball_objnum]);
				} else {
					explosion_life = 0.0f;
				}

				// JAS:  I put in all this code because of an item on my todo list that
				// said that the ship destroyed debris shouldn't pop in until the
				// big explosion is 30% done.  I did this on Oct24 and me & Adam 
				// thought it looked dumb since the explosion didn't move with the
				// ship, so instead of just taking this code out, since we might need
				// it in the future, I disabled it.   You can reenable it by changing
				// the commenting on the following two lines.
				shipp->end_death_time = shipp->really_final_death_time = timestamp( fl2i(explosion_life*1000.0f)/5 );	// Wait till 30% of vclip time before breaking the ship up.
			}

			shipp->flags |= SF_EXPLODED;

			if ( !(ship_get_exp_propagates(shipp)) ) {
				// apply area of effect blast damage from ship explosion
				ship_blow_up_area_apply_blast( objp );
			}
		}

		if ( timestamp_elapsed(shipp->really_final_death_time))	{
			// Copied from lock all turrets sexp
			// Locks all turrets on ship that is about to split.
			ship_subsys *subsys;
			subsys = GET_FIRST(&shipp->subsys_list);
			while ( subsys != END_OF_LIST(&shipp->subsys_list) ) 
			{
			// just mark all turrets as locked
				if (subsys->system_info->type == SUBSYSTEM_TURRET) {
					subsys->weapons.flags |= SW_FLAG_TURRET_LOCK;
				}
				subsys = GET_NEXT(subsys);
			}

			// do large_ship_split and explosion
			if ( shipp->large_ship_blowup_index >= 0 ) {
				if ( shipfx_large_blowup_do_frame(shipp, flFrametime) )	{
					// do all accounting for respawning client and server side here.
					if(objp == Player_obj) {				
						gameseq_post_event(GS_EVENT_DEATH_BLEW_UP);
					}

					objp->flags |= OF_SHOULD_BE_DEAD;									
					
					ship_cleanup(ship_num, SHIP_DESTROYED);		// call ship function to clean up after the ship is destroyed.
				}
				return;
			} 

			shipfx_blow_up_model(objp, sip->model_num, 0, 20, &objp->pos );

			// do all accounting for respawning client and server side here.
			if(objp == Player_obj) {				
				gameseq_post_event(GS_EVENT_DEATH_BLEW_UP);
			}

			objp->flags |= OF_SHOULD_BE_DEAD;
								
			ship_cleanup(ship_num, SHIP_DESTROYED);		// call ship function to clean up after the ship is destroyed.
			shipp->really_final_death_time = timestamp( -1 );	// Never time out again!
		}

		// If a ship is dying (and not a capital or big ship) then stutter the engine sound
		if ( timestamp_elapsed(shipp->next_engine_stutter) ) {
			if ( !(sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
				shipp->flags ^= SF_ENGINES_ON;			// toggle state of engines
				shipp->next_engine_stutter = timestamp_rand(50, 250);
			}
		}
	}
}

void ship_chase_shield_energy_targets(ship *shipp, object *obj, float frametime)
{
	float delta;
	ship_info *sip;

	if (shipp->flags & SF_DYING)
		return;

	sip = &Ship_info[shipp->ship_info_index];

	delta = frametime * ETS_RECHARGE_RATE * shipp->ship_max_shield_strength / 100.0f;

	//	Chase target_shields and target_weapon_energy
	if (shipp->target_shields_delta > 0.0f) {
		if (delta > shipp->target_shields_delta)
			delta = shipp->target_shields_delta;

		shield_add_strength(obj, delta);
		shipp->target_shields_delta -= delta;
	} else if (shipp->target_shields_delta < 0.0f) {
		if (delta < -shipp->target_shields_delta)
			delta = -shipp->target_shields_delta;

		shield_add_strength(obj, -delta);
		shipp->target_shields_delta += delta;
	}

	delta = frametime * ETS_RECHARGE_RATE * sip->max_weapon_reserve / 100.0f;

	if (shipp->target_weapon_energy_delta > 0.0f) {
		if (delta > shipp->target_weapon_energy_delta)
			delta = shipp->target_weapon_energy_delta;

		shipp->weapon_energy += delta;
		shipp->target_weapon_energy_delta -= delta;
	} else if (shipp->target_weapon_energy_delta < 0.0f) {
		if (delta < -shipp->target_weapon_energy_delta)
			delta = -shipp->target_weapon_energy_delta;

		shipp->weapon_energy -= delta;
		shipp->target_weapon_energy_delta += delta;
	}

}

int thruster_glow_anim_load(generic_anim *ga)
{
	if ( !VALID_FNAME(ga->filename) )
		return -1;

	int fps = 15;

	ga->first_frame = bm_load(ga->filename);
	if (ga->first_frame < 0)
	{
		Warning(LOCATION, "Couldn't load thruster glow animation '%s'\nPrimary glow type effect does not accept .EFF or .ANI effects", ga->filename);
		return -1;
	}
	ga->num_frames = NOISE_NUM_FRAMES;

	Assert(fps != 0);
	ga->total_time = i2fl(ga->num_frames)/fps;

	return 0;
}

/**
 * Loads the animations for ship's afterburners
 */
void ship_init_thrusters()
{
	if ( Thrust_anim_inited == 1 )
		return;

	for (size_t i = 0; i < Species_info.size(); i++)
	{
		species_info *species = &Species_info[i];

		generic_anim_load(&species->thruster_info.flames.normal);
		generic_anim_load(&species->thruster_info.flames.afterburn);

		// Bobboau's extra thruster stuff
		{
			generic_bitmap_load(&species->thruster_secondary_glow_info.normal);
			generic_bitmap_load(&species->thruster_secondary_glow_info.afterburn);
			generic_bitmap_load(&species->thruster_tertiary_glow_info.normal);
			generic_bitmap_load(&species->thruster_tertiary_glow_info.afterburn);
			generic_bitmap_load(&species->thruster_distortion_info.normal);
			generic_bitmap_load(&species->thruster_distortion_info.afterburn);
		}

		// glows are handled a bit strangely
		thruster_glow_anim_load(&species->thruster_info.glow.normal);
		thruster_glow_anim_load(&species->thruster_info.glow.afterburn);
	}

	Thrust_anim_inited = 1;
}


/**
 * Figure out which thruster bitmap will get rendered next time around.  
 *
 * ::ship_render() needs to have shipp->thruster_bitmap set to
 * a valid bitmap number, or -1 if we shouldn't render thrusters.
 */
void ship_do_thruster_frame( ship *shipp, object *objp, float frametime )
{
	float rate;
	int framenum;
	int secondary_glow_bitmap, tertiary_glow_bitmap, distortion_bitmap;
	generic_anim *flame_anim, *glow_anim;
	ship_info	*sinfo = &Ship_info[shipp->ship_info_index];

	if ( !Thrust_anim_inited ) {
		ship_init_thrusters();
	}

	if (objp->phys_info.flags & PF_AFTERBURNER_ON) {
		flame_anim = &sinfo->thruster_flame_info.afterburn;		// select afterburner flame
		glow_anim = &sinfo->thruster_glow_info.afterburn;			// select afterburner glow
		secondary_glow_bitmap = sinfo->thruster_secondary_glow_info.afterburn.bitmap_id;
		tertiary_glow_bitmap = sinfo->thruster_tertiary_glow_info.afterburn.bitmap_id;
		distortion_bitmap = sinfo->thruster_distortion_info.afterburn.bitmap_id;

		rate = 1.5f;		// go at 1.5x faster when afterburners on
	} else if (objp->phys_info.flags & PF_BOOSTER_ON) {
		flame_anim = &sinfo->thruster_flame_info.afterburn;		// select afterburner flame
		glow_anim = &sinfo->thruster_glow_info.afterburn;			// select afterburner glow
		secondary_glow_bitmap = sinfo->thruster_secondary_glow_info.afterburn.bitmap_id;
		tertiary_glow_bitmap = sinfo->thruster_tertiary_glow_info.afterburn.bitmap_id;
		distortion_bitmap = sinfo->thruster_distortion_info.afterburn.bitmap_id;

		rate = 2.5f;		// go at 2.5x faster when boosters on
	} else {
		flame_anim = &sinfo->thruster_flame_info.normal;			// select normal flame
		glow_anim = &sinfo->thruster_glow_info.normal;				// select normal glow
		secondary_glow_bitmap = sinfo->thruster_secondary_glow_info.normal.bitmap_id;
		tertiary_glow_bitmap = sinfo->thruster_tertiary_glow_info.normal.bitmap_id;
		distortion_bitmap = sinfo->thruster_distortion_info.normal.bitmap_id;

		// If thrust at 0, go at half as fast, full thrust; full framerate
		// so set rate from 0.67 to 1.67, depending on thrust from 0 to 1
		rate = 0.67f * (1.0f + objp->phys_info.forward_thrust);
	}

	Assert( frametime > 0.0f );

	// add primary thruster effects ...

	if (flame_anim->first_frame >= 0) {
		shipp->thruster_frame += frametime * rate;

		// Sanity checks
		if (shipp->thruster_frame < 0.0f) {
			shipp->thruster_frame = 0.0f;
		} else if (shipp->thruster_frame > 100.0f) {
			shipp->thruster_frame = 0.0f;
		}

		while (shipp->thruster_frame > flame_anim->total_time) {
			shipp->thruster_frame -= flame_anim->total_time;
		}

		framenum = fl2i( (shipp->thruster_frame * flame_anim->num_frames) / flame_anim->total_time );
		CLAMP(framenum, 0, (flame_anim->num_frames - 1));

		// Get the bitmap for this frame
		shipp->thruster_bitmap = flame_anim->first_frame + framenum;
	} else {
		shipp->thruster_frame = 0.0f;
		shipp->thruster_bitmap = -1;
	}

	// primary glows ...
	if (glow_anim->first_frame >= 0) {
		shipp->thruster_glow_frame += frametime * rate;

		// Sanity checks
		if (shipp->thruster_glow_frame < 0.0f) {
			shipp->thruster_glow_frame = 0.0f;
		} else if (shipp->thruster_glow_frame > 100.0f) {
			shipp->thruster_glow_frame = 0.0f;
		}

		while (shipp->thruster_glow_frame > glow_anim->total_time) {
			shipp->thruster_glow_frame -= glow_anim->total_time;
		}

		framenum = fl2i( (shipp->thruster_glow_frame*glow_anim->num_frames) / glow_anim->total_time );
		CLAMP(framenum, 0, (glow_anim->num_frames - 1));

		// Get the bitmap for this frame
		shipp->thruster_glow_bitmap = glow_anim->first_frame;	// + framenum;
		shipp->thruster_glow_noise = Noise[framenum];
	} else {
		shipp->thruster_glow_frame = 0.0f;
		shipp->thruster_glow_bitmap = -1;
		shipp->thruster_glow_noise = 1.0f;
	}

	// add extra thruster effects
	shipp->thruster_secondary_glow_bitmap = secondary_glow_bitmap;
	shipp->thruster_tertiary_glow_bitmap = tertiary_glow_bitmap;
	shipp->thruster_distortion_bitmap = distortion_bitmap;
}


/**
 * Figure out which thruster bitmap will get rendered next time around.  
 *
 * ship_render needs to have shipp->thruster_bitmap set to
 * a valid bitmap number, or -1 if we shouldn't render thrusters.
 *
 * This does basically the same thing as ship_do_thruster_frame, except it
 * operates on a weapon. This is in the ship code because it needs
 * the same thruster animation info as the ship stuff, and I would
 * rather extern this one function than all the thruster animation stuff.
 */
void ship_do_weapon_thruster_frame( weapon *weaponp, object *objp, float frametime )
{
	float rate;
	int framenum;
	generic_anim *flame_anim, *glow_anim;

	if (!Thrust_anim_inited)
		ship_init_thrusters();

	species_info *species = &Species_info[weaponp->species];
	weapon_info *wip = &Weapon_info[weaponp->weapon_info_index];

	// If thrust at 0, go at half as fast, full thrust; full framerate
	// so set rate from 0.67 to 1.67, depending on thrust from 0 to 1
	rate = 0.67f * (1.0f + objp->phys_info.forward_thrust);

	if (wip->thruster_flame.first_frame >= 0)
		flame_anim = &wip->thruster_flame;
	else
		flame_anim = &species->thruster_info.flames.normal;

	if (wip->thruster_glow.first_frame >= 0)
		glow_anim = &wip->thruster_glow;
	else
		glow_anim  = &species->thruster_info.glow.normal;

	Assert( frametime > 0.0f );

	if (flame_anim->first_frame >= 0) {
		weaponp->thruster_frame += frametime * rate;

		// Sanity checks
		if ( weaponp->thruster_frame < 0.0f )	weaponp->thruster_frame = 0.0f;
		if ( weaponp->thruster_frame > 100.0f ) weaponp->thruster_frame = 0.0f;

		while ( weaponp->thruster_frame > flame_anim->total_time )	{
			weaponp->thruster_frame -= flame_anim->total_time;
		}
		framenum = fl2i( (weaponp->thruster_frame*flame_anim->num_frames) / flame_anim->total_time );
		if ( framenum < 0 ) framenum = 0;
		if ( framenum >= flame_anim->num_frames ) framenum = flame_anim->num_frames-1;
	
		// Get the bitmap for this frame
		weaponp->thruster_bitmap = flame_anim->first_frame + framenum;
	} else {
		weaponp->thruster_frame = 0.0f;
		weaponp->thruster_bitmap = -1;
	}

	// Do it for glow bitmaps
	if (glow_anim->first_frame >= 0) {
		weaponp->thruster_glow_frame += frametime * rate;

		// Sanity checks
		if ( weaponp->thruster_glow_frame < 0.0f )	weaponp->thruster_glow_frame = 0.0f;
		if ( weaponp->thruster_glow_frame > 100.0f ) weaponp->thruster_glow_frame = 0.0f;

		while ( weaponp->thruster_glow_frame > glow_anim->total_time )	{
			weaponp->thruster_glow_frame -= glow_anim->total_time;
		}
		framenum = fl2i( (weaponp->thruster_glow_frame*glow_anim->num_frames) / glow_anim->total_time );
		if ( framenum < 0 ) framenum = 0;
		if ( framenum >= glow_anim->num_frames ) framenum = glow_anim->num_frames-1;
	
		// Get the bitmap for this frame
		weaponp->thruster_glow_bitmap = glow_anim->first_frame;	// + framenum;
		weaponp->thruster_glow_noise = Noise[framenum];
	} else {
		weaponp->thruster_glow_frame = 0.0f;
		weaponp->thruster_glow_bitmap = -1;
		weaponp->thruster_glow_noise = 1.0f;
	}
}



// Repair damaged subsystems for a ship, called for each ship once per frame.
// TODO: optimize by only calling ever N seconds and keeping track of elapsed time
//
// NOTE: need to update current_hits in the sp->subsys_list element, and the sp->subsys_info[]
// element.
#define SHIP_REPAIR_SUBSYSTEM_RATE	0.01f	// percent repair per second for a subsystem
#define SUBSYS_REPAIR_THRESHOLD		0.1	// only repair subsystems that have > 10% strength
void ship_auto_repair_frame(int shipnum, float frametime)
{
	ship_subsys		*ssp;
	ship_subsys_info	*ssip;
	ship			*sp;
	ship_info		*sip;
	object			*objp;
	float			real_repair_rate;

	#ifndef NDEBUG
	if ( !Ship_auto_repair )	// only repair subsystems if Ship_auto_repair flag is set
		return;
	#endif

	Assert( shipnum >= 0 && shipnum < MAX_SHIPS);
	sp = &Ships[shipnum];
	sip = &Ship_info[sp->ship_info_index];
	objp = &Objects[sp->objnum];

	//Repair the hull...or maybe unrepair?
	if(sip->hull_repair_rate != 0.0f)
	{
		objp->hull_strength += sp->ship_max_hull_strength * sip->hull_repair_rate * frametime;

		if(objp->hull_strength > sp->ship_max_hull_strength)
		{
			objp->hull_strength = sp->ship_max_hull_strength;
		}
	}

	// only allow for the auto-repair of subsystems on small ships
	//...NOT. Check if var has been changed from def -C
	if ( !(sip->flags & SIF_SMALL_SHIP) && sip->subsys_repair_rate == -2.0f)
		return;
	
	if(sip->subsys_repair_rate == -2.0f)
		real_repair_rate = SHIP_REPAIR_SUBSYSTEM_RATE;
	else
		real_repair_rate = sip->subsys_repair_rate;

	// AL 3-14-98: only allow auto-repair if power output not zero
	if (sip->power_output <= 0)
		return;
	
	// iterate through subsystems, repair as needed based on elapsed frametime
	for ( ssp = GET_FIRST(&sp->subsys_list); ssp != END_OF_LIST(&sp->subsys_list); ssp = GET_NEXT(ssp) ) {
		Assert(ssp->system_info->type >= 0 && ssp->system_info->type < SUBSYSTEM_MAX);
		ssip = &sp->subsys_info[ssp->system_info->type];

		if ( ssp->current_hits < ssp->max_hits ) {

			// only repair those subsystems which are not destroyed
			if ( ssp->max_hits <= 0 )
				continue;

			if ( ssp->current_hits <= 0 ) {
				if (sip->flags2 & SIF2_SUBSYS_REPAIR_WHEN_DISABLED) {
					if (ssp->flags & SSF_NO_AUTOREPAIR_IF_DISABLED) {
						continue;
					}
				} else if (!(ssp->flags & SSF_AUTOREPAIR_IF_DISABLED)) {
					continue;
				}
			}

			// do incremental repair on the subsystem
			// check for overflow of current_hits
			ssp->current_hits += ssp->max_hits * real_repair_rate * frametime;
			if ( ssp->current_hits > ssp->max_hits ) {
				ssp->current_hits = ssp->max_hits;
			}

			// aggregate repair
			if (!(ssp->flags & SSF_NO_AGGREGATE)) {
				ssip->aggregate_current_hits += ssip->aggregate_max_hits * real_repair_rate * frametime;
				if ( ssip->aggregate_current_hits > ssip->aggregate_max_hits ) {
					ssip->aggregate_current_hits = ssip->aggregate_max_hits;
				}
			}

			// check to see if this subsystem was totally non functional before -- if so, then
			// reset the flags
			if ( (ssp->system_info->type == SUBSYSTEM_ENGINE) && (sp->flags & SF_DISABLED) ) {
				sp->flags &= ~SF_DISABLED;
				ship_reset_disabled_physics(objp, sp->ship_info_index);
			}
		}
	}	// end for
}

// this function checks to see how far the player has strayed from his starting location (should be
// single player only).  Issues a warning at some distance.  Makes mission end if he keeps flying away
// 3 strikes and you're out or too far away
#define PLAYER_MAX_DIST_WARNING			700000			// distance in KM at which player gets warning to return to battle
#define PLAYER_DISTANCE_MAX_WARNINGS	3				// maximum number of warnings player can receive before mission ends
#define PLAYER_MAX_DIST_END				750000			// distance from starting loc at which we end mission
#define PLAYER_WARN_DELTA_TIME			10000			//ms
#define PLAYER_DEATH_DELTA_TIME			5000			//ms

void ship_check_player_distance_sub(player *p, int multi_target=-1)
{
	// only check distance for ships
	if ( p->control_mode != PCM_NORMAL )	{
		// already warping out... don't bother checking anymore
		return;
	}

	float dist = vm_vec_dist_quick(&Objects[p->objnum].pos, &vmd_zero_vector);

	int give_warning_to_player = 0;
	if ( dist > PLAYER_MAX_DIST_WARNING ) {
		if (p->distance_warning_count == 0) {
			give_warning_to_player = 1;
		} else {
			if (timestamp_until(p->distance_warning_time) < 0) {
				give_warning_to_player = 1;
			}
		}
	}

	if ( give_warning_to_player ) {
		// increase warning count
		p->distance_warning_count++;
		// set timestamp unless player PLAYER_FLAGS_DIST_TO_BE_KILLED flag is set
		if ( !(p->flags & PLAYER_FLAGS_DIST_TO_BE_KILLED) ) {
			p->distance_warning_time = timestamp(PLAYER_WARN_DELTA_TIME);
		}
		// issue up to max warnings
		if (p->distance_warning_count <= PLAYER_DISTANCE_MAX_WARNINGS) {
			message_send_builtin_to_player( MESSAGE_STRAY_WARNING, NULL, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_SOON, 0, 0, multi_target, -1 );
		}

		if (p->distance_warning_count > PLAYER_DISTANCE_MAX_WARNINGS) {
			p->flags |= PLAYER_FLAGS_DIST_WARNING;
		}
	}

	if ( !(p->flags & PLAYER_FLAGS_FORCE_MISSION_OVER) && ((p->distance_warning_count > PLAYER_DISTANCE_MAX_WARNINGS) || (dist > PLAYER_MAX_DIST_END)) ) {
//		DKA 5/17/99 - DON'T force warpout.  Won't work multiplayer.  Blow up ship.
		if ( !(p->flags & PLAYER_FLAGS_DIST_TO_BE_KILLED) ) {
			message_send_builtin_to_player( MESSAGE_STRAY_WARNING_FINAL, NULL, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, multi_target, -1 );
			p->flags |= PLAYER_FLAGS_DIST_TO_BE_KILLED;
			p->distance_warning_time = timestamp(PLAYER_DEATH_DELTA_TIME);
		}

		// get hull strength and blow up
		if ( (p->flags & PLAYER_FLAGS_DIST_TO_BE_KILLED) && (timestamp_until(p->distance_warning_time) < 0) ) {
			p->flags |= PLAYER_FLAGS_FORCE_MISSION_OVER;
			float damage = 10.0f * Objects[p->objnum].hull_strength;
			ship_apply_global_damage(&Objects[p->objnum], &Objects[p->objnum], NULL, damage);
		}
	}

	// see if player has moved back into "bounds"
	if ( (dist < PLAYER_MAX_DIST_WARNING) && (p->flags & PLAYER_FLAGS_DIST_WARNING) && !(p->flags & PLAYER_FLAGS_DIST_TO_BE_KILLED) ) {
		p->flags &= ~PLAYER_FLAGS_DIST_WARNING;
		p->distance_warning_count = 1;
	}
}

void ship_check_player_distance()
{
	int idx;

	// multiplayer
	if (Game_mode & GM_MULTIPLAYER) {
		// if I'm the server, check all non-observer players including myself
		if (MULTIPLAYER_MASTER) {
			// warn all players
			for (idx=0; idx<MAX_PLAYERS; idx++) {
				if (MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_OBSERVER(Net_players[idx]) && (Objects[Net_players[idx].m_player->objnum].type != OBJ_GHOST) ) {
					// if bad, blow him up
					ship_check_player_distance_sub(Net_players[idx].m_player, idx);
				}
			}
		}
	}
	// single player
	else {
		// maybe blow him up
		ship_check_player_distance_sub(Player);
	}		
}

void observer_process_post(object *objp)
{
	Assert(objp != NULL);

	if (objp == NULL)
		return;

	Assert(objp->type == OBJ_OBSERVER);

	if (Game_mode & GM_MULTIPLAYER) {
		// if I'm just an observer
		if (MULTI_OBSERVER(Net_players[MY_NET_PLAYER_NUM])) {
			float dist = vm_vec_dist_quick(&Player_obj->pos, &vmd_zero_vector);
			// if beyond max dist, reset to 0
			if (dist > PLAYER_MAX_DIST_END) {
				// set me to zero
				if ((Player_obj != NULL) && (Player_obj->type != OBJ_GHOST)) {
					Player_obj->pos = vmd_zero_vector;
				}
			}
		}
	}
}

/**
 * Reset some physics info when ship's engines goes from disabled->enabled
 */
void ship_reset_disabled_physics(object *objp, int ship_class)
{
	Assert(objp != NULL);

	if (objp == NULL)
		return;

	objp->phys_info.flags &= ~(PF_REDUCED_DAMP | PF_DEAD_DAMP);
	objp->phys_info.side_slip_time_const = Ship_info[ship_class].damp;
}

/**
 * Clear/set the subsystem disrupted flags
 */
void ship_subsys_disrupted_check(ship *sp)
{
	ship_subsys *ss;
	int engines_disabled=0;
	
	if ( sp->subsys_disrupted_flags & (1<<SUBSYSTEM_ENGINE) ) {
		engines_disabled=1;
	}

	sp->subsys_disrupted_flags=0;

	ss = GET_FIRST(&sp->subsys_list);
	while ( ss != END_OF_LIST( &sp->subsys_list ) ) {
		if ( !timestamp_elapsed(ss->disruption_timestamp) ) {
			sp->subsys_disrupted_flags |= (1<<ss->system_info->type);
		}
		ss = GET_NEXT( ss );
	}

	if ( engines_disabled ) {
		if ( !(sp->subsys_disrupted_flags & (1<<SUBSYSTEM_ENGINE)) ) {
			if ( !(sp->flags & SF_DISABLED) ) {
				ship_reset_disabled_physics(&Objects[sp->objnum], sp->ship_info_index);
			}
		}
	}
}

/**
 * Maybe check ship subsystems for disruption, and set/clear flags
 */
void ship_subsys_disrupted_maybe_check(ship *shipp)
{
	if ( timestamp_elapsed(shipp->subsys_disrupted_check_timestamp) ) {
		ship_subsys_disrupted_check(shipp);
		shipp->subsys_disrupted_check_timestamp=timestamp(250);
	}
}

/**
 * Determine if a given subsystem is disrupted (ie inoperable)
 *
 * @param ss	pointer to ship subsystem
 * @return		1 if subsystem is disrupted, 0 if subsystem is not disrupted
 */
int ship_subsys_disrupted(ship_subsys *ss)
{
	if ( !ss ) {
		Int3();		// should never happen, get Alan if it does.
		return 0;
	}

	if ( timestamp_elapsed(ss->disruption_timestamp) ) {
		return 0;
	} else {
		return 1;
	}
}

/**
 * Disrupt a subsystem (ie make it inoperable for a time)
 *
 * @param ss	pointer to ship subsystem to be disrupted
 * @param time	time in ms that subsystem should be disrupted
 */
void ship_subsys_set_disrupted(ship_subsys *ss, int time)
{
	int time_left=0;

	if ( !ss ) {
		Int3();		// should never happen, get Alan if it does.
		return;
	}

	time_left=timestamp_until(ss->disruption_timestamp);
	if ( time_left < 0 ) {
		time_left=0;
	}

	ss->disruption_timestamp = timestamp(time+time_left);
}

/**
 * Determine if a given type of subsystem is disrupted (i.e. inoperable)
 * 
 * @param sp	pointer to ship containing subsystem
 * @param type	type of subsystem (SUBSYSTEM_*)
 * @return		1 if subsystem is disrupted, 0 if subsystem is not disrupted
 */
int ship_subsys_disrupted(ship *sp, int type)
{
	Assert ( sp != NULL );
	Assert ( type >= 0 && type < SUBSYSTEM_MAX );
    
	// Bogus pointer to ship to check for disrupted subsystem
	if (sp == NULL)
		return 0;
    
	if ( sp->subsys_disrupted_flags & (1<<type) ) {
		return 1;
	} else {
		return 0;
	}
}

float Decay_rate = 1.0f / 120.0f;
DCF(lethality_decay, "Sets ship lethality_decay, or the time in sec to go from 100 to 0 health (default is 1/120)")
{
	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Decay rate is currently %f\n", Decay_rate);
		return;
	}
	
	dc_stuff_float(&Decay_rate);
}

float min_lethality = 0.0f;

void lethality_decay(ai_info *aip)
{
	float decay_rate = Decay_rate;
	aip->lethality -= 100.0f * decay_rate * flFrametime;
	aip->lethality = MAX(-10.0f, aip->lethality);

#ifndef NDEBUG
	if (Objects[Ships[aip->shipnum].objnum].flags & OF_PLAYER_SHIP) {
		if (Framecount % 10 == 0) {
			int num_turrets = 0;
			if ((aip->target_objnum != -1) && (Objects[aip->target_objnum].type == OBJ_SHIP)) {
				//TODO: put this where it belongs, this would involve recompiling *everything* right now
				//-WMC
				int num_turrets_attacking(object *turret_parent, int target_objnum);
				num_turrets = num_turrets_attacking(&Objects[aip->target_objnum], Ships[aip->shipnum].objnum);
			}
			nprintf(("lethality", "Player lethality: %.1f, num turrets targeting player: %d\n", aip->lethality, num_turrets));
		}
	}
#endif
}

void ship_process_pre(object *objp, float frametime)
{
	if ( (objp == NULL) || !frametime )
		return;
}

MONITOR( NumShips )

void ship_radar_process( object * obj, ship * shipp, ship_info * sip ) 
{
	Assert( obj != NULL);
	Assert( shipp != NULL );
	Assert( sip != NULL);

	shipp->radar_last_status = shipp->radar_current_status;

	RadarVisibility visibility = radar_is_visible(obj);

	if (visibility == NOT_VISIBLE)
	{
		if (shipp->radar_last_contact < 0 && shipp->radar_visible_since < 0)
		{
			shipp->radar_visible_since = -1;
			shipp->radar_last_contact = -1;
		}
		else
		{
			shipp->radar_visible_since = -1;
			shipp->radar_last_contact = Missiontime;
		}
	}
	else if (visibility == VISIBLE || visibility == DISTORTED)
	{
		if (shipp->radar_visible_since < 0)
		{
			shipp->radar_visible_since = Missiontime;
		}

		shipp->radar_last_contact = Missiontime;
	}

	shipp->radar_current_status = visibility;
}


/**
 * Player ship uses this code, but does a quick out after doing a few things.
 * 
 * When adding code to this function, decide whether or not a client in a multiplayer game
 * needs to execute the code you are adding.  Code which moves things, creates things, etc
 * probably doesn't need to be called.  If you don't know -- find Allender!!!
 */
void ship_process_post(object * obj, float frametime)
{
	int	num;
	ship	*shipp;
	ship_info *sip;

	if(obj->type != OBJ_SHIP){
		nprintf(("Network","Ignoring non-ship object in ship_process_post()\n"));
		return;
	}

	MONITOR_INC( NumShips, 1 );	

	num = obj->instance;
	Assert( num >= 0 && num < MAX_SHIPS);
	Assert( obj->type == OBJ_SHIP );
	Assert( Ships[num].objnum == OBJ_INDEX(obj));	

	shipp = &Ships[num];

	sip = &Ship_info[shipp->ship_info_index];

	shipp->shield_hits = 0;

	update_ets(obj, frametime);

	afterburners_update(obj, frametime);

	ship_subsys_disrupted_maybe_check(shipp);

	ship_dying_frame(obj, num);

	ship_chase_shield_energy_targets(shipp, obj, frametime);

	// AL 1-6-98: record the initial ammo counts for ships, which is used as the max limit for rearming
	// Goober5000 - added ballistics support
	if ( !(shipp->flags & SF_AMMO_COUNT_RECORDED) )
	{
		int max_missiles;
		for ( int i=0; i<MAX_SHIP_SECONDARY_BANKS; i++ ) {
			if ( red_alert_mission() )
			{
				max_missiles = get_max_ammo_count_for_bank(shipp->ship_info_index, i, shipp->weapons.secondary_bank_weapons[i]);
				shipp->weapons.secondary_bank_start_ammo[i] = max_missiles;
			}
			else
			{
				shipp->weapons.secondary_bank_start_ammo[i] = shipp->weapons.secondary_bank_ammo[i];
			}
		}

		if ( sip->flags & SIF_BALLISTIC_PRIMARIES )
		{
			for ( int i=0; i<MAX_SHIP_PRIMARY_BANKS; i++ )
			{
				if ( red_alert_mission() )
				{
					max_missiles = get_max_ammo_count_for_primary_bank(shipp->ship_info_index, i, shipp->weapons.primary_bank_weapons[i]);
					shipp->weapons.primary_bank_start_ammo[i] = max_missiles;
				}
				else
				{
					shipp->weapons.primary_bank_start_ammo[i] = shipp->weapons.primary_bank_ammo[i];
				}
			}
		}
		
		shipp->flags |= SF_AMMO_COUNT_RECORDED;
	}

	if(!(Game_mode & GM_STANDALONE_SERVER)) {
		// Plot ship on the radar.  What about multiplayer ships?
		if ( obj != Player_obj )			// don't plot myself.
			radar_plot_object( obj );

		// MWA -- move the spark code to before the check for multiplayer master
		//	Do ship sparks.  Don't do sparks on my ship (since I cannot see it).  This
		// code will do sparks on other ships in multiplayer though.
		// JAS: Actually in external view, you can see sparks, so I don't do sparks
		// on the Viewer_obj, not Player_obj.
		if ( (obj != Viewer_obj) && timestamp_elapsed(Ships[num].next_hit_spark) )	{
			shipfx_emit_spark(num,-1);	// -1 means choose random spark location
		}

		if ( obj != Viewer_obj )	{
			shipfx_do_damaged_arcs_frame( shipp );
		}

		// JAS - flicker the thruster bitmaps
		ship_do_thruster_frame(shipp,obj,frametime);		
	}

	ship_auto_repair_frame(num, frametime);

	shipfx_do_lightning_frame(shipp);

	// if the ship has an EMP effect active, process it
	emp_process_ship(shipp);	

	// call the contrail system
	ct_ship_process(shipp);

	// process engine wash
	void engine_wash_ship_process(ship *shipp);
	engine_wash_ship_process(shipp);

	// update TAG info
	if(shipp->tag_left > 0.0f){
		shipp->tag_left -= flFrametime;
		if(shipp->tag_left <= 0.000001f){
			shipp->tag_left = -1.0f;

			mprintf(("Killing TAG for %s\n", shipp->ship_name));
		}
	}
	
	// update level 2 TAG info
	if(shipp->level2_tag_left > 0.0f){
		shipp->level2_tag_left -= flFrametime;
		if(shipp->level2_tag_left <= 0.000001f){
			shipp->level2_tag_left = -1.0f;

			mprintf(("Killing level 2 TAG for %s\n", shipp->ship_name));
		}
	}
	
	if ( shipp->flags & SF_ARRIVING && Ai_info[shipp->ai_index].mode != AIM_BAY_EMERGE )	{
		// JAS -- if the ship is warping in, just move it forward at a speed
		// fast enough to move 2x its radius in SHIP_WARP_TIME seconds.
		shipfx_warpin_frame( obj, frametime );
	} else if ( shipp->flags & SF_DEPART_WARP ) {
		// JAS -- if the ship is warping out, just move it forward at a speed
		// fast enough to move 2x its radius in SHIP_WARP_TIME seconds.
		shipfx_warpout_frame( obj, frametime );
	} 

	// update radar status of the ship
	ship_radar_process(obj, shipp, sip);

	if ( (!(shipp->flags & SF_ARRIVING) || (Ai_info[shipp->ai_index].mode == AIM_BAY_EMERGE)
		|| ((sip->warpin_type == WT_IN_PLACE_ANIM) && (shipp->flags & SF_ARRIVING_STAGE_2)) )
		&&	!(shipp->flags & SF_DEPART_WARP))
	{
		//	Do AI.

		// for multiplayer people.  return here if in multiplay and not the host
		if ( MULTIPLAYER_CLIENT ) {
			model_anim_handle_multiplayer( &Ships[num] );
			return;
		}

		// MWA -- moved the code to maybe fire swarm missiles to after the check for
		// multiplayer master.  Only single player and multi server needs to do this code
		// this code might call ship_fire_secondary which will send the fire packets
		swarm_maybe_fire_missile(num);

		// maybe fire turret swarm missiles
		void turret_swarm_maybe_fire_missile(int num);
		turret_swarm_maybe_fire_missile(num);

		// maybe fire a corkscrew missile (just like swarmers)
		cscrew_maybe_fire_missile(num);

		//rotate player subobjects since its processed by the ai functions
		// AL 2-19-98: Fire turret for player if it exists
		//WMC - changed this to call process_subobjects
		if ( (obj->flags & OF_PLAYER_SHIP) && !Player_use_ai )
		{
			ai_info *aip = &Ai_info[Ships[obj->instance].ai_index];
			if (aip->ai_flags & (AIF_AWAITING_REPAIR | AIF_BEING_REPAIRED))
			{
				if (aip->support_ship_objnum >= 0)
				{
					if (vm_vec_dist_quick(&obj->pos, &Objects[aip->support_ship_objnum].pos) < (obj->radius + Objects[aip->support_ship_objnum].radius) * 1.25f)
						return;
				}
			}
			process_subobjects(OBJ_INDEX(obj));
		}

		if (obj == Player_obj) {
			ship_check_player_distance();
		}

		// update ship lethality
		if ( Ships[num].ai_index >= 0 ){
			if (!physics_paused && !ai_paused){
				lethality_decay(&Ai_info[Ships[num].ai_index]);
			}
		}

		// if the ship is an observer ship don't need to do AI
		if ( obj->type == OBJ_OBSERVER)  {
			return;
		}

		// Goober5000 - player may want to use AI
		if ( (Ships[num].ai_index >= 0) && (!(obj->flags & OF_PLAYER_SHIP) || Player_use_ai) ){
			if (!physics_paused && !ai_paused){
				ai_process( obj, Ships[num].ai_index, frametime );
			}
		}
	}			
}


/**
 * Set the ship level weapons based on the information contained in the ship info.
 * 
 * Weapon assignments are checked against the model to ensure the models
 * and the ship info weapon data are in synch.
 */
void ship_set_default_weapons(ship *shipp, ship_info *sip)
{
	int			i, j;
	polymodel	*pm;
	ship_weapon *swp = &shipp->weapons;
	weapon_info *wip;

	//	Copy primary and secondary weapons from ship_info to ship.
	//	Later, this will happen in the weapon loadout screen.
	for (i=0; i < MAX_SHIP_PRIMARY_BANKS; i++){
		swp->primary_bank_weapons[i] = sip->primary_bank_weapons[i];
		swp->primary_bank_slot_count[i] = 1; // RSAXVC DYN LINK CODE
	}

	for (i=0; i < MAX_SHIP_SECONDARY_BANKS; i++){
		swp->secondary_bank_weapons[i] = sip->secondary_bank_weapons[i];
	}

	// Copy the number of primary and secondary banks to ship, and verify that
	// model is in synch
	pm = model_get( sip->model_num );

	// Primary banks
	if ( pm->n_guns > sip->num_primary_banks ) {
		Assert(pm->n_guns <= MAX_SHIP_PRIMARY_BANKS);
		Error(LOCATION, "There are %d primary banks in the model file,\nbut only %d primary banks specified for %s.\nThis must be fixed, as it will cause crashes.\n", pm->n_guns, sip->num_primary_banks, sip->name);
		for ( i = sip->num_primary_banks; i < pm->n_guns; i++ ) {
			// Make unspecified weapon for bank be a laser
			for ( j = 0; j < Num_player_weapon_precedence; j++ ) {
				Assertion((Player_weapon_precedence[j] > 0), "Error reading player weapon precedence list. Check weapons.tbl for $Player Weapon Precedence entry, and correct as necessary.\n");
				int weapon_id = Player_weapon_precedence[j];
				if ( (Weapon_info[weapon_id].subtype == WP_LASER) || (Weapon_info[weapon_id].subtype == WP_BEAM) ) {
					swp->primary_bank_weapons[i] = weapon_id;
					break;
				}
			}
			Assert(swp->primary_bank_weapons[i] >= 0);
		}
		sip->num_primary_banks = pm->n_guns;
	}
	else if ( pm->n_guns < sip->num_primary_banks ) {
		Warning(LOCATION, "There are %d primary banks specified for %s\nbut only %d primary banks in the model\n", sip->num_primary_banks, sip->name, pm->n_guns);
		sip->num_primary_banks = pm->n_guns;
	}

	// Secondary banks
	if ( pm->n_missiles > sip->num_secondary_banks ) {
		Assert(pm->n_missiles <= MAX_SHIP_SECONDARY_BANKS);
		Error(LOCATION, "There are %d secondary banks in the model file,\nbut only %d secondary banks specified for %s.\nThis must be fixed, as it will cause crashes.\n", pm->n_missiles, sip->num_secondary_banks, sip->name);
		for ( i = sip->num_secondary_banks; i < pm->n_missiles; i++ ) {
			// Make unspecified weapon for bank be a missile
			for ( j = 0; j < Num_player_weapon_precedence; j++ ) {
				Assertion((Player_weapon_precedence[j] > 0), "Error reading player weapon precedence list. Check weapons.tbl for $Player Weapon Precedence entry, and correct as necessary.\n");
				int weapon_id = Player_weapon_precedence[j];
				if (Weapon_info[weapon_id].subtype == WP_MISSILE) {
					swp->secondary_bank_weapons[i] = weapon_id;
					break;
				}
			}
			Assert(swp->secondary_bank_weapons[i] >= 0);
		}
		sip->num_secondary_banks = pm->n_missiles;
	}
	else if ( pm->n_missiles < sip->num_secondary_banks ) {
		Warning(LOCATION, "There are %d secondary banks specified for %s,\n but only %d secondary banks in the model.\n", sip->num_secondary_banks, sip->name, pm->n_missiles);
		sip->num_secondary_banks = pm->n_missiles;
	}

	// added ballistic primary support - Goober5000
	swp->num_primary_banks = sip->num_primary_banks;
	for ( i = 0; i < swp->num_primary_banks; i++ )
	{
		wip = &Weapon_info[swp->primary_bank_weapons[i]];

		if ( wip->wi_flags2 & WIF2_BALLISTIC )
		{
			if (Fred_running){
				swp->primary_bank_ammo[i] = 100;
			}
			else
			{
				float capacity, size;
				capacity = (float) sip->primary_bank_ammo_capacity[i];
				size = (float) wip->cargo_size;
				swp->primary_bank_ammo[i] = fl2i((capacity / size)+0.5f);
				swp->primary_bank_start_ammo[i] = swp->primary_bank_ammo[i];
			}

			swp->primary_bank_capacity[i] = sip->primary_bank_ammo_capacity[i];
		}
	}

	swp->num_secondary_banks = sip->num_secondary_banks;
	for ( i = 0; i < swp->num_secondary_banks; i++ ) {
		if (Fred_running){
			swp->secondary_bank_ammo[i] = 100;
		} else {
			wip = &Weapon_info[swp->secondary_bank_weapons[i]];
			float size = (float) wip->cargo_size;
			swp->secondary_bank_ammo[i] = fl2i(sip->secondary_bank_ammo_capacity[i]/size);
			// Karajorma - Support ships will use the wrong values if we don't set this. 
			swp->secondary_bank_start_ammo[i] = swp->secondary_bank_ammo[i];
		}

		swp->secondary_bank_capacity[i] = sip->secondary_bank_ammo_capacity[i];
	}

	for ( i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++ ){
		swp->next_primary_fire_stamp[i] = timestamp(0);
		swp->last_primary_fire_stamp[i] = -1;
		swp->burst_counter[i] = 0;
		swp->last_primary_fire_sound_stamp[i] = timestamp(0);
	}

	for ( i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++ ){
		swp->next_secondary_fire_stamp[i] = timestamp(0);
		swp->last_secondary_fire_stamp[i] = -1;
		swp->burst_counter[i + MAX_SHIP_PRIMARY_BANKS] = 0;
	}

	//Countermeasures
	shipp->current_cmeasure = sip->cmeasure_type;
}


/**
 * Faster version of ship_check_collision that does not do checking at the polygon
 * level.  Just checks to see if a vector will intersect a sphere.
 */
int ship_check_collision_fast( object * obj, object * other_obj, vec3d * hitpos)
{
	int num;
	mc_info mc;

	Assert( obj->type == OBJ_SHIP );
	Assert( obj->instance >= 0 );

	num = obj->instance;

	mc_info_init(&mc);
	mc.model_instance_num = Ships[num].model_instance_num;
	mc.model_num = Ship_info[Ships[num].ship_info_index].model_num;	// Fill in the model to check
	mc.orient = &obj->orient;					// The object's orient
	mc.pos = &obj->pos;							// The object's position
	mc.p0 = &other_obj->last_pos;			// Point 1 of ray to check
	mc.p1 = &other_obj->pos;					// Point 2 of ray to check
	mc.flags = MC_ONLY_SPHERE;				// flags

	model_collide(&mc);
	if (mc.num_hits)
		*hitpos = mc.hit_point_world;

	return mc.num_hits;
}

/**
 * Ensure create time for ship is unique
 */
void ship_make_create_time_unique(ship *shipp)
{
	static int last_smctu_initial_time = -1;
	static int last_smctu_final_time = -1;
	int		sanity_counter = 0, collision;
	ship		*compare_shipp;
	ship_obj	*so;
	uint		new_create_time;

	new_create_time = shipp->create_time;

	while (1) {

		collision = 0;

		for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
			compare_shipp = &Ships[Objects[so->objnum].instance];

			if ( compare_shipp == shipp ) {
				continue;
			}

			if ( compare_shipp->create_time == new_create_time )
			{
				if((unsigned int)sanity_counter == 0 && (unsigned int)last_smctu_initial_time == shipp->create_time)
				{
					//WMC: If we're creating a whole bunch of ships at once, we can
					//shortcut this process by looking at the last call to this function
					//This fixes a bug when more than 50 ships are created at once.
					new_create_time = last_smctu_final_time + 1;
				}
				else
				{
					new_create_time++;
				}
				collision = 1;
				break;
			}
		}

		if ( !collision ) {
			last_smctu_initial_time = shipp->create_time;
			last_smctu_final_time = new_create_time;
			shipp->create_time = new_create_time;
			break;
		}

		if ( sanity_counter++ > MAX_SHIPS ) {
			Int3();
			break;
		}
	}
}

int	Ship_subsys_hwm = 0;

void show_ship_subsys_count()
{
	object	*objp;
	int		count = 0;	
	int		o_type = 0;

	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		o_type = (int)objp->type;
		if (o_type == OBJ_SHIP) {
			count += Ship_info[Ships[objp->instance].ship_info_index].n_subsystems;
		}
	}

	if (count > Ship_subsys_hwm) {
		Ship_subsys_hwm = count;
	}
}

void ship_init_afterburners(ship *shipp)
{
	Assert( shipp );

	shipp->ab_count = 0;

	if (shipp->ship_info_index < 0) {
		Int3();
		return;
	}

	ship_info *sip = &Ship_info[shipp->ship_info_index];
	Assert( sip->model_num >= 0 );
	polymodel *pm = model_get(sip->model_num);
	Assert( pm != NULL );

	if ( !(sip->flags & SIF_AFTERBURNER) ) {
		return;
	}

	if (sip->afterburner_trail.bitmap_id < 0) {
		return;
	}

	for (int i = 0; i < pm->n_thrusters; i++) {
		thruster_bank *bank = &pm->thrusters[i];

		for (int j = 0; j < bank->num_points; j++) {
			// this means you've reached the max # of AB trails for a ship
			if (shipp->ab_count >= MAX_SHIP_CONTRAILS) {
				Int3();
				break;
			}

			trail_info *ci = &shipp->ab_info[shipp->ab_count];

			if (bank->points[j].norm.xyz.z > -0.5f) {
				continue; // only make ab trails for thrusters that are pointing backwards
			}

			ci->pt = bank->points[j].pnt; //offset

			ci->w_start = bank->points[j].radius * sip->afterburner_trail_width_factor;	// width * table loaded width factor
			ci->w_end = 0.05f; //end width

			ci->a_start = 1.0f * sip->afterburner_trail_alpha_factor; // start alpha  * table loaded alpha factor
			ci->a_end = 0.0f; //end alpha

			ci->max_life = sip->afterburner_trail_life;	// table loaded max life
			ci->stamp = 60;	//spew time???	

			ci->n_fade_out_sections = sip->afterburner_trail_faded_out_sections; // initial fade out

			ci->texture.bitmap_id = sip->afterburner_trail.bitmap_id; // table loaded bitmap used on this ships burner trails

			nprintf(("AB TRAIL", "AB trail point #%d made for '%s'\n", shipp->ab_count, shipp->ship_name));

			shipp->ab_count++;
		}
	}
}

/**
 * Returns object index of ship.
 * @return -1 means failed.
 */
int ship_create(matrix *orient, vec3d *pos, int ship_type, char *ship_name)
{
	int			i, n, objnum, j, k, t;
	ship_info	*sip;
	ship			*shipp;

	t = ship_get_num_ships();
	
	// The following check caps the number of ships that can be created.  Because Fred needs
	// to create all the ships, regardless of when they arrive/depart, it needs a higher
	// limit than FreeSpace.  On release, however, we will reduce it, thus FreeSpace needs
	// to check against what this limit will be, otherwise testing the missions before
	// release could work fine, yet not work anymore once a release build is made.
	if (Fred_running) {
		if (t >= MAX_SHIPS)
			return -1;

	} else {
		if (t >= SHIPS_LIMIT) {
			Error(LOCATION, XSTR("There is a limit of %d ships in the mission at once.  Please be sure that you do not have more than %d ships present in the mission at the same time.", 1495), SHIPS_LIMIT, SHIPS_LIMIT );
			return -1;
		}
	}

	for (n=0; n<MAX_SHIPS; n++){
		if (Ships[n].objnum == -1){
			break;
		}
	}

	if (n == MAX_SHIPS){
		return -1;
	}

	Assert((ship_type >= 0) && (ship_type < static_cast<int>(Ship_info.size())));
	sip = &(Ship_info[ship_type]);
	shipp = &Ships[n];
	shipp->clear();

	sip->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);		// use the highest detail level
	if(strlen(sip->cockpit_pof_file))
	{
		sip->cockpit_model_num = model_load(sip->cockpit_pof_file, 0, NULL);
	}

	// maybe load an optional hud target model
	if(strlen(sip->pof_file_hud)){
		// check to see if a "real" ship uses this model. if so, load it up for him so that subsystems are setup properly
		for(auto it = Ship_info.begin(); it != Ship_info.end(); ++it){
			if(!stricmp(it->pof_file, sip->pof_file_hud)){
				it->model_num = model_load(it->pof_file, it->n_subsystems, &it->subsystems[0]);
			}
		}

		// mow load it for me with no subsystems
		sip->model_num_hud = model_load(sip->pof_file_hud, 0, NULL);
	}

	polymodel *pm = model_get(sip->model_num);

	ship_copy_subsystem_fixup(sip);
	show_ship_subsys_count();

	if ( sip->num_detail_levels != pm->n_detail_levels )
	{
		if ( !Is_standalone )
		{
			// just log to file for standalone servers
			Warning(LOCATION, "For ship '%s', detail level\nmismatch. Table has %d,\nPOF has %d.", sip->name, sip->num_detail_levels, pm->n_detail_levels );
		}
		else
		{
			nprintf(("Warning",  "For ship '%s', detail level mismatch. Table has %d, POF has %d.", sip->name, sip->num_detail_levels, pm->n_detail_levels ));
		}	
	}		
	for ( i=0; i<pm->n_detail_levels; i++ )
		pm->detail_depth[i] = (i < sip->num_detail_levels) ? i2fl(sip->detail_distance[i]) : 0.0f;

	// JAS: Nav buoys don't need to do collisions!
	// G5K: Corrected to apply specifically for ships with the no-collide flag.  (In retail, navbuoys already have this flag, so this doesn't break anything.)
	if ( sip->flags & SIF_NO_COLLIDE )	{
		objnum = obj_create(OBJ_SHIP, -1, n, orient, pos, model_get_radius(sip->model_num), OF_RENDERS | OF_PHYSICS );
	} else {
		objnum = obj_create(OBJ_SHIP, -1, n, orient, pos, model_get_radius(sip->model_num), OF_RENDERS | OF_COLLIDES | OF_PHYSICS );
	}
	Assert( objnum >= 0 );

	shipp->ai_index = ai_get_slot(n);
	Assert( shipp->ai_index >= 0 );

	// Goober5000 - if no ship name specified, or if specified ship already exists,
	// or if specified ship has exited, use a default name
	if ((ship_name == NULL) || (ship_name_lookup(ship_name) >= 0) || (ship_find_exited_ship_by_name(ship_name) >= 0)) {
		char suffix[NAME_LENGTH];
		sprintf(suffix, NOX(" %d"), n);

		// ensure complete ship name doesn't overflow the buffer
		int name_len = MIN(NAME_LENGTH - strlen(suffix) - 1, strlen(Ship_info[ship_type].name));
		Assert(name_len > 0);

		strncpy(shipp->ship_name, Ship_info[ship_type].name, name_len);
		strcpy(shipp->ship_name + name_len, suffix);
	} else {
		strcpy_s(shipp->ship_name, ship_name);
	}

	ship_set_default_weapons(shipp, sip);	//	Moved up here because ship_set requires that weapon info be valid.  MK, 4/28/98
	ship_set(n, objnum, ship_type);

	init_ai_object(objnum);
	ai_clear_ship_goals( &Ai_info[Ships[n].ai_index] );		// only do this one here.  Can't do it in init_ai because it might wipe out goals in mission file

	//	Allocate shield and initialize it.
	if (pm->shield.ntris) {
		shipp->shield_integrity = (float *) vm_malloc(sizeof(float) * pm->shield.ntris);
		for (i=0; i<pm->shield.ntris; i++)
			shipp->shield_integrity[i] = 1.0f;
	} else
		shipp->shield_integrity = NULL;

	// Bump the object radius to ensure that collision detection works right
	// even when spread shields extend outside the model's natural radius
	if (sip->flags2 & SIF2_AUTO_SPREAD_SHIELDS) {
		Objects[objnum].radius += sip->auto_shield_spread;
	}

	// allocate memory for keeping glow point bank status (enabled/disabled)
	{
		bool val = true; // default value, enabled

		if (pm->n_glow_point_banks)
			shipp->glow_point_bank_active.resize( pm->n_glow_point_banks, val );
	}

	// fix up references into paths for this ship's model to point to a ship_subsys entry instead
	// of a submodel index.  The ship_subsys entry should be the same for *all* instances of the
	// same ship.
	if (!(sip->flags & SIF_PATH_FIXUP))
	{
		for ( i = 0; i < pm->n_paths; i++ )
		{
			for ( j = 0; j < pm->paths[i].nverts; j++ )
			{
				for ( k = 0; k < pm->paths[i].verts[j].nturrets; k++ )
				{
					int ptindex = pm->paths[i].verts[j].turret_ids[k];		// this index is a submodel number (ala bspgen)
					int index;
					ship_subsys *ss;

					// iterate through the ship_subsystems looking for an id that matches
					index = 0;
					ss = GET_FIRST(&Ships[n].subsys_list);
					while ( ss != END_OF_LIST( &Ships[n].subsys_list ) ) {
						if ( ss->system_info->subobj_num == ptindex ) {			// when these are equal, fix up the ref
							pm->paths[i].verts[j].turret_ids[k] = index;				// in path structure to index a ship_subsys
							break;											
						}
						index++;
						ss = GET_NEXT( ss );
					}

					if ( ss == END_OF_LIST(&Ships[n].subsys_list) )
						Warning(LOCATION, "Couldn't fix up turret indices in spline path\n\nModel: %s\nPath: %s\nVertex: %d\nTurret model id:%d\n\nThis probably means that the turret was not specified in the ship table(s).", sip->pof_file, pm->paths[i].name, j, ptindex );
				}
			}
		}
		sip->flags |= SIF_PATH_FIXUP;
	}

	// Add this ship to Ship_obj_list
	shipp->ship_list_index = ship_obj_list_add(objnum);

	// Set time when ship is created
	shipp->create_time = timer_get_milliseconds();

	ship_make_create_time_unique(shipp);

	// first try at ABtrails -Bobboau
	ship_init_afterburners(shipp);

	// call the contrail system
	ct_ship_create(shipp);

	model_anim_set_initial_states(shipp);

	shipp->model_instance_num = model_create_instance(sip->model_num);

	shipp->time_created = Missiontime;

	return objnum;
}

/**
 * Change the ship model for a ship to that for ship class 'ship_type'
 *
 * @param n			index of ship in ::Ships[] array
 * @param ship_type	ship class (index into ::Ship_info vector)
 */
void ship_model_change(int n, int ship_type)
{
	int			i;
	ship_info	*sip;
	ship			*sp;
	polymodel * pm;
	object *objp;

	Assert( n >= 0 && n < MAX_SHIPS );
	sp = &Ships[n];
	sip = &(Ship_info[ship_type]);
	objp = &Objects[sp->objnum];

	// get new model
	if (sip->model_num == -1) {
		sip->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
	}

	if ( sip->cockpit_model_num == -1 ) {
		if ( strlen(sip->cockpit_pof_file) ) {
			sip->cockpit_model_num = model_load(sip->cockpit_pof_file, 0, NULL);
		}
	}

	pm = model_get(sip->model_num);
	Objects[sp->objnum].radius = model_get_radius(pm->id);

	// page in nondims in game
	if ( !Fred_running )
		model_page_in_textures(sip->model_num, ship_type);

	// allocate memory for keeping glow point bank status (enabled/disabled)
	{
		bool val = true; // default value, enabled

		// clear out any old gpb's first, then add new ones if needed
		sp->glow_point_bank_active.clear();

		if (pm->n_glow_point_banks)
			sp->glow_point_bank_active.resize( pm->n_glow_point_banks, val );
	}

	ship_copy_subsystem_fixup(sip);

	if ( sip->num_detail_levels != pm->n_detail_levels )
	{
		if ( !Is_standalone )
		{
			// just log to file for standalone servers
			Warning(LOCATION, "For ship '%s', detail level\nmismatch. Table has %d,\nPOF has %d.", sip->name, sip->num_detail_levels, pm->n_detail_levels );
		}
		else
		{
			nprintf(("Warning",  "For ship '%s', detail level mismatch. Table has %d, POF has %d.", sip->name, sip->num_detail_levels, pm->n_detail_levels ));
		}
	}	
	for ( i=0; i<pm->n_detail_levels; i++ )
		pm->detail_depth[i] = (i < sip->num_detail_levels) ? i2fl(sip->detail_distance[i]) : 0.0f;

	if (sip->flags2 & SIF2_MODEL_POINT_SHIELDS) {
		objp->n_quadrants = pm->shield_points.size();
		sp->shield_points = pm->shield_points;
	} else {
		objp->n_quadrants = DEFAULT_SHIELD_SECTIONS;
	}
	objp->shield_quadrant.resize(objp->n_quadrants);

	if (sp->shield_integrity != NULL) {
		vm_free(sp->shield_integrity);
		sp->shield_integrity = NULL;
	}

	//	Allocate shield and initialize it.
	if (pm->shield.ntris) {
		sp->shield_integrity = (float *) vm_malloc(sizeof(float) * pm->shield.ntris);

		for (i = 0; i < pm->shield.ntris; i++) {
			sp->shield_integrity[i] = 1.0f;
		}
	} else {
		sp->shield_integrity = NULL;
	}

	// reset texture animations
	sp->base_texture_anim_frametime = game_get_overall_frametime();
}

/**
 * Change the ship class on a ship, and changing all required information
 * for consistency (ie textures, subsystems, weapons, physics)
 *
 * @param n			index of ship in ::Ships[] array
 * @param ship_type	ship class (index into ::Ship_info vector)
 * @param by_sexp	SEXP reference
 */
void change_ship_type(int n, int ship_type, int by_sexp)
{
	int i;
	ship_info	*sip;
	ship_info	*sip_orig;
	ship			*sp;
	ship_weapon *swp;
	ship_subsys *ss;
	object		*objp;
	p_object	*p_objp;
	float hull_pct, shield_pct;
	physics_info ph_inf;

	Assert( n >= 0 && n < MAX_SHIPS );
	sp = &Ships[n];

	// do a quick out if we're already using the new ship class
	if (sp->ship_info_index == ship_type)
		return;

	swp = &sp->weapons;
	sip = &(Ship_info[ship_type]);
	sip_orig = &Ship_info[sp->ship_info_index];
	objp = &Objects[sp->objnum];
	p_objp = mission_parse_get_parse_object(sp->ship_name);
	ph_inf = objp->phys_info;


	// Goober5000 - we can't copy the ship object because the tree of structs contains at least
	// one class without a copy constructor.  So let's just save the information we need.

	// these are wiped by ets_init_ship
	int orig_wep_rechg_idx = sp->weapon_recharge_index;
	int orig_shd_rechg_idx = sp->shield_recharge_index;
	int orig_eng_rechg_idx = sp->engine_recharge_index;


	// Goober5000 - maintain the original hull, shield, and subsystem percentages... gah
	// ...except when in FRED, because this stuff is handled in the missionparse/missionsave part. The E

	if (!Fred_running) {
		// hull
		if (sp->special_hitpoints) {
			hull_pct = objp->hull_strength / sp->ship_max_hull_strength; 
		} else {
			Assert( Ship_info[sp->ship_info_index].max_hull_strength > 0.0f );
			hull_pct = objp->hull_strength / Ship_info[sp->ship_info_index].max_hull_strength;
		}

		// extra check
		Assert(hull_pct > 0.0f && hull_pct <= 1.0f);
		CLAMP(hull_pct, 0.1f, 1.0f);

		// shield
		if (sp->special_shield > 0) {
			shield_pct = shield_get_strength(objp) / sp->ship_max_shield_strength;
		} else if (Ship_info[sp->ship_info_index].max_shield_strength > 0.0f) {
			shield_pct = shield_get_strength(objp) / Ship_info[sp->ship_info_index].max_shield_strength;
		} else {
			shield_pct = 0.0f;
		}

		// extra check
		Assert(shield_pct >= 0.0f && shield_pct <= 1.0f);
		CLAMP(shield_pct, 0.0f, 1.0f);
	} else {
		shield_pct = hull_pct = 1.0f;
	}

	// subsystems
	int num_saved_subsystems = 0;
	char **subsys_names = new char *[sip_orig->n_subsystems];
	float *subsys_pcts = new float[sip_orig->n_subsystems];

	ss = GET_FIRST(&sp->subsys_list);
	while ( ss != END_OF_LIST(&sp->subsys_list) )
	{
		if (num_saved_subsystems == sip_orig->n_subsystems)
		{
			Error(LOCATION, "Subsystem mismatch while changing ship class from '%s' to '%s'!", sip_orig->name, sip->name);
			break;
		}

		// save subsys information
		subsys_names[num_saved_subsystems] = new char[NAME_LENGTH];
		strcpy(subsys_names[num_saved_subsystems], ss->system_info->subobj_name);

		if (ss->max_hits > 0.0f)
			subsys_pcts[num_saved_subsystems] = ss->current_hits / ss->max_hits;
		else
			subsys_pcts[num_saved_subsystems] = ss->max_hits;

		// extra check
		Assert(subsys_pcts[num_saved_subsystems] >= 0.0f && subsys_pcts[num_saved_subsystems] <= 1.0f);
        CLAMP(subsys_pcts[num_saved_subsystems], 0.0f, 1.0f);

		num_saved_subsystems++;
		ss = GET_NEXT(ss);
	}

	// point to new ship data
	ship_model_change(n, ship_type);
	sp->ship_info_index = ship_type;

	if (!Fred_running) {
		//WMC - set warp effects
		ship_set_warp_effects(objp, sip);
	}

	// set the correct hull strength
	if (Fred_running) {
		sp->ship_max_hull_strength = 100.0f;
		objp->hull_strength = 100.0f;
	} else {
		if (sp->special_hitpoints > 0) {
			sp->ship_max_hull_strength = (float)sp->special_hitpoints;
		} else {
			sp->ship_max_hull_strength = sip->max_hull_strength;
		}

		objp->hull_strength = hull_pct * sp->ship_max_hull_strength;
	}

	sp->max_shield_recharge = sip->max_shield_recharge;

	// set the correct shield strength
	if (Fred_running) {
		if (sp->ship_max_shield_strength)
			sp->ship_max_shield_strength = 100.0f;
		objp->shield_quadrant[0] = 100.0f;
	} else {
		if (sp->special_shield >= 0) {
			sp->ship_max_shield_strength = (float)sp->special_shield;
		} else {
			sp->ship_max_shield_strength = sip->max_shield_strength;
		}

		shield_set_strength(objp, shield_pct * sp->ship_max_shield_strength);
	}

	// Goober5000: div-0 checks
	Assert(sp->ship_max_hull_strength > 0.0f);
	Assert(objp->hull_strength > 0.0f);

	// Mantis 2763: moved down to have access to the right ship_max_shield_strength value
	// make sure that shields are disabled/enabled if they need to be - Chief1983
	if (!Fred_running) {
		if ((p_objp->flags2 & P2_OF_FORCE_SHIELDS_ON) && (sp->ship_max_shield_strength > 0.0f)) {
			objp->flags &= ~OF_NO_SHIELDS;
		} else if ((p_objp->flags & P_OF_NO_SHIELDS) || (sp->ship_max_shield_strength == 0.0f)) {
			objp->flags |= OF_NO_SHIELDS;
		// Since there's not a mission flag set to be adjusting this, see if there was a change from a ship that normally has shields to one that doesn't, and vice versa
		} else if (!(sip_orig->flags2 & SIF2_INTRINSIC_NO_SHIELDS) && (sip->flags2 & SIF2_INTRINSIC_NO_SHIELDS)) {
			objp->flags |= OF_NO_SHIELDS;
		} else if ((sip_orig->flags2 & SIF2_INTRINSIC_NO_SHIELDS) && !(sip->flags2 & SIF2_INTRINSIC_NO_SHIELDS) && (sp->ship_max_shield_strength > 0.0f)) {
			objp->flags &= ~OF_NO_SHIELDS;
		}
	}

	// niffiwan: set new armor types
	sp->armor_type_idx = sip->armor_type_idx;
	sp->shield_armor_type_idx = sip->shield_armor_type_idx;
	sp->collision_damage_type_idx = sip->collision_damage_type_idx;
	sp->debris_damage_type_idx = sip->debris_damage_type_idx;

	// subsys stuff done only after hull stuff is set
	// if the subsystem list is not currently empty, then we need to clear it out first.
	ship_subsystems_delete(sp);

	// fix up the subsystems
	subsys_set( sp->objnum );

	// Goober5000 - restore the subsystem percentages
	ss = GET_FIRST(&sp->subsys_list);
	while ( ss != END_OF_LIST(&sp->subsys_list) )
	{
		for (i = 0; i < num_saved_subsystems; i++)
		{
			if (!subsystem_stricmp(ss->system_info->subobj_name, subsys_names[i]))
			{
				ss->current_hits = ss->max_hits * subsys_pcts[i];
				break;
			}
		}

		ss = GET_NEXT(ss);
	}
	ship_recalc_subsys_strength(sp);

	// now free the memory
	for (i = 0; i < sip_orig->n_subsystems; i++)
		delete[] subsys_names[i];
	
	delete [] subsys_names;
	delete [] subsys_pcts;

	sp->afterburner_fuel = MAX(0, sip->afterburner_fuel_capacity - (sip_orig->afterburner_fuel_capacity - sp->afterburner_fuel));
	sp->cmeasure_count = MAX(0, sip->cmeasure_max - (sip_orig->cmeasure_max - sp->cmeasure_count));

	// avoid cases where either of these are 0
	if (sp->current_max_speed != 0 && sip_orig->max_speed != 0) {
		sp->current_max_speed = sip->max_speed * (sp->current_max_speed / sip_orig->max_speed);
	}
	else {
		sp->current_max_speed = sip->max_speed;
	}

	ship_set_default_weapons(sp, sip);
	physics_ship_init(&Objects[sp->objnum]);
	ets_init_ship(&Objects[sp->objnum]);

	// Reset physics to previous values
	if (by_sexp) {
		Objects[sp->objnum].phys_info.desired_rotvel = ph_inf.desired_rotvel;
		Objects[sp->objnum].phys_info.desired_vel = ph_inf.desired_vel;
		Objects[sp->objnum].phys_info.forward_thrust = ph_inf.forward_thrust;
		Objects[sp->objnum].phys_info.fspeed = ph_inf.fspeed;
		Objects[sp->objnum].phys_info.heading = ph_inf.heading;
		Objects[sp->objnum].phys_info.last_rotmat = ph_inf.last_rotmat;
		Objects[sp->objnum].phys_info.prev_fvec = ph_inf.prev_fvec;
		Objects[sp->objnum].phys_info.prev_ramp_vel = ph_inf.prev_ramp_vel;
		Objects[sp->objnum].phys_info.reduced_damp_decay = ph_inf.reduced_damp_decay;
		Objects[sp->objnum].phys_info.rotvel = ph_inf.rotvel;
		Objects[sp->objnum].phys_info.shockwave_decay = ph_inf.shockwave_decay;
		Objects[sp->objnum].phys_info.shockwave_shake_amp = ph_inf.shockwave_shake_amp;
		Objects[sp->objnum].phys_info.side_thrust = ph_inf.side_thrust;
		Objects[sp->objnum].phys_info.speed = ph_inf.speed;
		Objects[sp->objnum].phys_info.vel = ph_inf.vel;
		Objects[sp->objnum].phys_info.vert_thrust = ph_inf.vert_thrust;
	}

	ship_set_new_ai_class(n, sip->ai_class);

	//======================================================

	// Bobboau's thruster stuff again
	if (sip->afterburner_trail.bitmap_id < 0)
		generic_bitmap_load(&sip->afterburner_trail);

	sp->ab_count = 0;
	if (sip->flags & SIF_AFTERBURNER)
	{
		polymodel *pm = model_get(sip->model_num);

		for (int h = 0; h < pm->n_thrusters; h++)
		{
			for (int j = 0; j < pm->thrusters[h].num_points; j++)
			{
				// this means you've reached the max # of AB trails for a ship
				Assert(sip->ct_count <= MAX_SHIP_CONTRAILS);
	
				trail_info *ci = &sp->ab_info[sp->ab_count];

				// only make ab trails for thrusters that are pointing backwards
				if (pm->thrusters[h].points[j].norm.xyz.z > -0.5)
					continue;

				ci->pt = pm->thrusters[h].points[j].pnt;	//offset
				ci->w_start = pm->thrusters[h].points[j].radius * sip->afterburner_trail_width_factor;	// width * table loaded width factor
	
				ci->w_end = 0.05f;//end width
	
				ci->a_start = 1.0f * sip->afterburner_trail_alpha_factor;	// start alpha  * table loaded alpha factor
	
				ci->a_end = 0.0f;//end alpha
	
				ci->max_life = sip->afterburner_trail_life;	// table loaded max life
	
				ci->stamp = 60;	//spew time???	

				ci->n_fade_out_sections = sip->afterburner_trail_faded_out_sections; // table loaded n sections to be faded out

				ci->texture.bitmap_id = sip->afterburner_trail.bitmap_id; // table loaded bitmap used on this ships burner trails
				nprintf(("AB TRAIL", "AB trail point #%d made for '%s'\n", sp->ab_count, sp->ship_name));
				sp->ab_count++;
				Assert(MAX_SHIP_CONTRAILS > sp->ab_count);
			}
		}
	}//end AB trails -Bobboau

	// Goober5000 - check other class-specific flags too

	if (sip->flags & SIF_STEALTH)			// changing TO a stealthy ship class
		sp->flags2 |= SF2_STEALTH;
	else if (sip_orig->flags & SIF_STEALTH)	// changing FROM a stealthy ship class
		sp->flags2 &= ~SF2_STEALTH;

	if (sip->flags & SIF_SHIP_CLASS_DONT_COLLIDE_INVIS)				// changing TO a don't-collide-invisible ship class
		sp->flags2 |= SF2_DONT_COLLIDE_INVIS;
	else if (sip_orig->flags & SIF_SHIP_CLASS_DONT_COLLIDE_INVIS)	// changing FROM a don't-collide-invisible ship class
		sp->flags2 &= ~SF2_DONT_COLLIDE_INVIS;

	if (sip->flags & SIF_NO_COLLIDE)								// changing TO a no-collision ship class
		obj_set_flags(objp, objp->flags & ~OF_COLLIDES);
	else if (sip_orig->flags & SIF_NO_COLLIDE)						// changing FROM a no-collision ship class
		obj_set_flags(objp, objp->flags | OF_COLLIDES);

	if (sip->flags2 & SIF2_NO_ETS)
		sp->flags2 |= SF2_NO_ETS;
	else if (sip_orig->flags2 & SIF2_NO_ETS)
		sp->flags2 &= ~SF2_NO_ETS;


	// Chief1983: Make sure that when changing to a new ship with secondaries, you switch to bank 0.  They still won't 
	// fire if the SF2_SECONDARIES_LOCKED flag is on as this should have carried over.
	if ( swp->num_secondary_banks > 0 && swp->current_secondary_bank == -1 ){
		swp->current_secondary_bank = 0;
	}

	// Bobboau's animation fixup
	for( i = 0; i<MAX_SHIP_PRIMARY_BANKS;i++){
			swp->primary_animation_position[i] = MA_POS_NOT_SET;
	}
	for( i = 0; i<MAX_SHIP_SECONDARY_BANKS;i++){
			swp->secondary_animation_position[i] = MA_POS_NOT_SET;
	}
	model_anim_set_initial_states(sp);

	//Reassign sound stuff
	ship_assign_sound(sp);
	
	// create new model instance data
	sp->model_instance_num = model_create_instance(sip->model_num);

	// Valathil - Reinitialize collision checks
	if ( Cmdline_old_collision_sys ) {
		obj_remove_pairs(objp);
		obj_add_pairs(OBJ_INDEX(objp));
	} else {
		obj_remove_collider(OBJ_INDEX(objp));
		obj_add_collider(OBJ_INDEX(objp));
	}

	// The E - If we're switching during gameplay, make sure we get valid primary/secondary selections
	if ( by_sexp ) {
		if (sip_orig->num_primary_banks > sip->num_primary_banks) {
			sp->weapons.current_primary_bank = 0;
		}

		if (sip_orig->num_secondary_banks > sip->num_secondary_banks) {
			sp->weapons.current_secondary_bank = 0;
		}

		// While we're at it, let's copy over the ETS settings too
		sp->weapon_recharge_index = orig_wep_rechg_idx;
		sp->shield_recharge_index = orig_shd_rechg_idx;
		sp->engine_recharge_index = orig_eng_rechg_idx;
	}

	// zookeeper - If we're switching in the loadout screen, make sure we retain initial velocity set in FRED
	if (!(Game_mode & GM_IN_MISSION) && !(Fred_running)) {
		Objects[sp->objnum].phys_info.speed = (float) p_objp->initial_velocity * sip->max_speed / 100.0f;
		Objects[sp->objnum].phys_info.vel.xyz.z = Objects[sp->objnum].phys_info.speed;
		Objects[sp->objnum].phys_info.prev_ramp_vel = Objects[sp->objnum].phys_info.vel;
		Objects[sp->objnum].phys_info.desired_vel = Objects[sp->objnum].phys_info.vel;
	}

	// Goober5000 - if we're changing to a ship class that has a different default set of orders, update the orders
	// (this avoids wiping the orders if we're e.g. changing between fighter classes)
	if (Fred_running)
	{
		int old_defaults = ship_get_default_orders_accepted(sip_orig);
		int new_defaults = ship_get_default_orders_accepted(sip);

		if (old_defaults != new_defaults)
			sp->orders_accepted = new_defaults;
	}

	// Goober5000 - deal with texture replacement by re-applying the same code we used during parsing
	if (sp->ship_replacement_textures != NULL)
	{
		// clear them out because the new positions may be different
		for (i = 0; i < MAX_REPLACEMENT_TEXTURES; i++)
			sp->ship_replacement_textures[i] = -1;

		// now fill them in according to texture name
		for (SCP_vector<texture_replace>::iterator tr = p_objp->replacement_textures.begin(); tr != p_objp->replacement_textures.end(); ++tr)
		{
			int j;
			polymodel *pm = model_get(sip->model_num);

			// look for textures
			for (j = 0; j < pm->n_textures; j++)
			{
				texture_map *tmap = &pm->maps[j];

				int tnum = tmap->FindTexture(tr->old_texture);
				if(tnum > -1)
					sp->ship_replacement_textures[j * TM_NUM_TYPES + tnum] = tr->new_texture_id;
			}
		}
	}

	if (sip->uses_team_colors)
	{
		sp->team_name = sip->default_team_name;
	}
}

#ifndef NDEBUG
/**
 * Fire the debug laser
 */
int ship_fire_primary_debug(object *objp)
{
	int	i;
	ship	*shipp = &Ships[objp->instance];
	vec3d wpos;

	if ( !timestamp_elapsed(shipp->weapons.next_primary_fire_stamp[0]) )
		return 0;

	// do timestamp stuff for next firing time
	shipp->weapons.next_primary_fire_stamp[0] = timestamp(250);
	shipp->weapons.last_primary_fire_stamp[0] = timestamp();

	//	Debug code!  Make the single laser fire only one bolt and from the object center!
	for (i=0; i<MAX_WEAPON_TYPES; i++)
		if (!stricmp(Weapon_info[i].name, NOX("Debug Laser")))
			break;
	
	vm_vec_add(&wpos, &objp->pos, &(objp->orient.vec.fvec) );
	if (i != MAX_WEAPONS) {
		int weapon_objnum;
		weapon_objnum = weapon_create( &wpos, &objp->orient, i, OBJ_INDEX(objp) );
		weapon_set_tracking_info(weapon_objnum, OBJ_INDEX(objp), Ai_info[shipp->ai_index].target_objnum);
		return 1;
	} else
		return 0;
}
#endif

/**
 * Launch countermeasures from object *objp.
 * 
 * @param objp object from which to launch countermeasure
 * @param rand_val is used in multiplayer to ensure that all clients in the game fire countermeasure the same way
 */
int ship_launch_countermeasure(object *objp, int rand_val)
{
	if(!Countermeasures_enabled) {
		return 0;
	}

	int	check_count, cmeasure_count;
	int cobjnum=-1;
	vec3d	pos;
	ship	*shipp;
	ship_info *sip;

	shipp = &Ships[objp->instance];
	sip = &Ship_info[shipp->ship_info_index];

	int arand;
	if(rand_val < 0) {
		arand = myrand();
	} else {
		arand = rand_val;
	}

	// in the case where the server is an observer, he can launch countermeasures unless we do this.
	if( objp->type == OBJ_OBSERVER){
		return 0;
	}

	if ( !timestamp_elapsed(shipp->cmeasure_fire_stamp) ){
		return 0;
	}

	shipp->cmeasure_fire_stamp = timestamp(CMEASURE_WAIT);	//	Can launch every half second.
#ifndef NDEBUG
	if (Weapon_energy_cheat) {
		shipp->cmeasure_count++;
	}
#endif

	// we might check the count of countermeasures left depending on game state.  Multiplayer clients
	// do not need to check any objects other than themselves for the count
	check_count = 1;

	if ( MULTIPLAYER_CLIENT && (objp != Player_obj) ){
		check_count = 0;
	}

	if ( check_count && ((shipp->cmeasure_count <= 0) || (sip->cmeasure_type < 0)) ) {
		if ( objp == Player_obj ) {
			if(sip->cmeasure_max < 1 || sip->cmeasure_type < 0) {
				//TODO: multi-lingual support
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Not equipped with countermeasures", 1633));
			} else if(shipp->current_cmeasure < 0) {
				//TODO: multi-lingual support
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "No countermeasures selected", 1634));
			} else if(shipp->cmeasure_count <= 0) {
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "No more countermeasure charges.", 485));
			}
			snd_play( &Snds[ship_get_sound(Player_obj, SND_OUT_OF_MISSLES)], 0.0f );
		}

		// if we have a player ship, then send the fired packet anyway so that the player
		// who fired will get his 'out of countermeasures' sound
		cmeasure_count = 0;
		if (objp->flags & OF_PLAYER_SHIP){
			// the new way of doing things
			if (Game_mode & GM_MULTIPLAYER){
				send_NEW_countermeasure_fired_packet(objp, cmeasure_count, -1);
			}
			return 0;
		}

		return 0;
	}

	cmeasure_count = shipp->cmeasure_count;
	shipp->cmeasure_count--;

	vm_vec_scale_add(&pos, &objp->pos, &objp->orient.vec.fvec, -objp->radius/2.0f);

	cobjnum = weapon_create(&pos, &objp->orient, shipp->current_cmeasure, OBJ_INDEX(objp));
	if (cobjnum >= 0)
	{
		cmeasure_set_ship_launch_vel(&Objects[cobjnum], objp, arand);
		nprintf(("Network", "Cmeasure created by %s\n", shipp->ship_name));

		// Play sound effect for counter measure launch
		Assert(shipp->current_cmeasure < Num_weapon_types);
		if ( Weapon_info[shipp->current_cmeasure].launch_snd >= 0 ) {
			snd_play_3d( &Snds[Weapon_info[shipp->current_cmeasure].launch_snd], &pos, &View_position );
		}

		// the new way of doing things
		if(Game_mode & GM_MULTIPLAYER){
			send_NEW_countermeasure_fired_packet(objp, cmeasure_count, Objects[cobjnum].net_signature);
		}
	}

	return (cobjnum >= 0);		// return 0 if not fired, 1 otherwise
}

/**
 * See if enough time has elapsed to play fail sound again
 */
void ship_maybe_play_primary_fail_sound()
{
	ship_weapon *swp = &Player_ship->weapons;
	int stampval;

	hud_start_flash_weapon(swp->current_primary_bank);

	if ( timestamp_elapsed(Laser_energy_out_snd_timer) )
	{
		// check timestamp according to ballistics
		if (Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]].wi_flags2 & WIF2_BALLISTIC)
		{
			stampval = 500;
		}
		else
		{
			stampval = 50;
		}
		Laser_energy_out_snd_timer = timestamp(stampval);
		snd_play( &Snds[ship_get_sound(Player_obj, SND_OUT_OF_WEAPON_ENERGY)]);
	}
}

/**
 * See if enough time has elapsed to play secondary fail sound again
 */
int ship_maybe_play_secondary_fail_sound(weapon_info *wip)
{
	hud_start_flash_weapon(Player_ship->weapons.num_primary_banks + Player_ship->weapons.current_secondary_bank);

	if ( timestamp_elapsed(Missile_out_snd_timer) ) {
		
		if ( wip->wi_flags & WIF_SWARM ) {
			Missile_out_snd_timer = timestamp(500);
		} else {
			Missile_out_snd_timer = timestamp(50);
		}
		snd_play( &Snds[ship_get_sound(Player_obj, SND_OUT_OF_MISSLES)] );
		return 1;
	}
	return 0;
}

/**
 * See if weapon for ship can fire based on weapons subystem strength.
 *
 * @return 1 if weapon failed to fire, 0 if weapon can fire
 */
int ship_weapon_maybe_fail(ship *sp)
{
	int	rval;
	float	weapons_subsys_str;

	// If playing on lowest skill level, weapons will not fail due to subsystem damage
	if ( Game_skill_level == 0 ){
		return 0;
	}

	rval = 0;
	weapons_subsys_str = ship_get_subsystem_strength( sp, SUBSYSTEM_WEAPONS );
	if ( weapons_subsys_str < SUBSYS_WEAPONS_STR_FIRE_FAIL ) {
		rval = 1;
	}
	else if ( weapons_subsys_str < SUBSYS_WEAPONS_STR_FIRE_OK ) {
		// chance to fire depends on weapons subsystem strength
		if ( (frand()-0.2f) > weapons_subsys_str )		
			rval = 1;
	}

	if (!rval) {
		// is subsystem disrupted?
		if ( ship_subsys_disrupted(sp, SUBSYSTEM_WEAPONS) ) {
			rval=1;
		}
	}
		
	return rval;
}

// create a moving tracer based upon a weapon which just fired
float t_rad = 0.5f;
float t_len = 10.0f;
float t_vel = 0.2f;
float t_min = 150.0f;
float t_max = 300.0f;
DCF(t_rad, "Sets weapon tracer radius")
{
	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("t_rad : %f\n", t_rad);
		return;
	}
	
	dc_stuff_float(&t_rad);
}
DCF(t_len, "Sets weapon tracer length")
{
	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("t_len : %f\n", t_len);
		return;
	}

	dc_stuff_float(&t_len);
}
DCF(t_vel, "Sets weapon tracer velocity")
{
	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("t_vel : %f\n", t_vel);
		return;
	}

	dc_stuff_float(&t_vel);
}
/*
 TODO: These two DCF's (and variables) are unused
DCF(t_min, "")
{
	dc_stuff_float(&t_min);
}
DCF(t_max, "")
{
	dc_stuff_float(&t_max);
}
*/
void ship_fire_tracer(int weapon_objnum)
{
	particle_info pinfo;
	object *objp = &Objects[weapon_objnum];
	weapon_info *wip = &Weapon_info[Weapons[Objects[weapon_objnum].instance].weapon_info_index];

	// setup particle info
	memset(&pinfo, 0, sizeof(particle_info));
	pinfo.pos = objp->pos;
	pinfo.vel = objp->phys_info.vel;
	vm_vec_scale(&pinfo.vel, t_vel);
	pinfo.lifetime = wip->lifetime;
	pinfo.rad = t_rad;
	pinfo.type = PARTICLE_BITMAP;
	pinfo.optional_data = wip->laser_bitmap.first_frame;
	pinfo.tracer_length = t_len;
	pinfo.reverse = 0;
	pinfo.attached_objnum = -1;
	pinfo.attached_sig = 0;

	// create the particle
	particle_create(&pinfo);
}

/**
 * Stops a single primary bank
 */
int ship_stop_fire_primary_bank(object * obj, int bank_to_stop)
{
	ship			*shipp;
	ship_weapon	*swp;

	if(obj == NULL){
		return 0;
	}

	if(obj->type != OBJ_SHIP){
		return 0;
	}

	shipp = &Ships[obj->instance];
	swp = &shipp->weapons;
	
	if(Ship_info[shipp->ship_info_index].draw_primary_models[bank_to_stop]){
		if(shipp->primary_rotate_rate[bank_to_stop] > 0.0f)
			shipp->primary_rotate_rate[bank_to_stop] -= Weapon_info[swp->primary_bank_weapons[bank_to_stop]].weapon_submodel_rotate_accell*flFrametime;
		if(shipp->primary_rotate_rate[bank_to_stop] < 0.0f)shipp->primary_rotate_rate[bank_to_stop] = 0.0f;
		shipp->primary_rotate_ang[bank_to_stop] += shipp->primary_rotate_rate[bank_to_stop]*flFrametime;
		if(shipp->primary_rotate_ang[bank_to_stop] > PI2)shipp->primary_rotate_ang[bank_to_stop] -= PI2;
		if(shipp->primary_rotate_ang[bank_to_stop] < 0.0f)shipp->primary_rotate_ang[bank_to_stop] += PI2;
	}
	
	if(shipp->was_firing_last_frame[bank_to_stop] == 0)
		return 0;

	shipp->was_firing_last_frame[bank_to_stop] = 0;

	return 1;
}


/**
 * Stuff to do when the ship has stoped fireing all primary weapons
 */
int ship_stop_fire_primary(object * obj)
{
	int i, num_primary_banks = 0, bank_to_stop = 0;
	ship			*shipp;
	ship_weapon	*swp;

	if(obj == NULL){
		return 0;
	}

	if(obj->type != OBJ_SHIP){
		return 0;
	}

	shipp = &Ships[obj->instance];
	swp = &shipp->weapons;

	bank_to_stop = swp->current_primary_bank;

	if ( shipp->flags & SF_PRIMARY_LINKED ) {
		num_primary_banks = swp->num_primary_banks;
	} else {
		num_primary_banks = MIN(1, swp->num_primary_banks);
	}

	for ( i = 0; i < num_primary_banks; i++ ) {	
		// Goober5000 - allow more than two banks
		bank_to_stop = (swp->current_primary_bank+i) % swp->num_primary_banks;
		//only stop if it was fireing last frame
		ship_stop_fire_primary_bank(obj, bank_to_stop);
	}
	for(i = 0; i<swp->num_primary_banks+num_primary_banks;i++)
		ship_stop_fire_primary_bank(obj, i%swp->num_primary_banks);

	return 1;
}



int tracers[MAX_SHIPS][4][4];

float ship_get_subsystem_strength( ship *shipp, int type );

// fires a primary weapon for the given object.  It also handles multiplayer cases.
// in multiplayer, the starting network signature, and number of banks fired are sent
// to all the clients in the game. All the info is passed to send_primary at the end of
// the function.  The check_energy parameter (defaults to 1) tells us whether or not
// we should check the energy.  It will be 0 when a multiplayer client is firing an AI
// primary.
int ship_fire_primary(object * obj, int stream_weapons, int force)
{
	vec3d		gun_point, pnt, firing_pos, target_position, target_velocity_vec;
	int			n = obj->instance;
	ship			*shipp;
	ship_weapon	*swp;
	ship_info	*sip;
	ai_info		*aip;
	int			weapon_idx, i, j, w, v, weapon_objnum;
	int			bank_to_fire, num_fired = 0;	
	int			banks_fired;				// used for multiplayer to help determine whether or not to send packet
	banks_fired = 0;			// used in multiplayer -- bitfield of banks that were fired
	bool has_fired = false;		// used to determine whether we should fire the scripting hook
	bool has_autoaim, has_converging_autoaim, needs_target_pos;	// used to flag weapon/ship as having autoaim
	float autoaim_fov = 0;			// autoaim limit
	float dist_to_target = 0;		// distance to target, for autoaim & automatic convergence

	int			sound_played;	// used to track what sound is played.  If the player is firing two banks
										// of the same laser, we only want to play one sound
	Assert( obj != NULL );

	if(obj == NULL){
		return 0;
	}

	// in the case where the server is an observer, he can fire (which) would be bad - unless we do this.
	if( obj->type == OBJ_OBSERVER){
		return 0;
	}

	Assert( obj->type == OBJ_SHIP );
	Assert( n >= 0 );
	Assert( Ships[n].objnum == OBJ_INDEX(obj));
	if((obj->type != OBJ_SHIP) || (n < 0) || (n >= MAX_SHIPS) || (Ships[n].objnum != OBJ_INDEX(obj))){
		return 0;
	}
	
	shipp = &Ships[n];
	swp = &shipp->weapons;

	// bogus 
	if((shipp->ship_info_index < 0) || (shipp->ship_info_index >= static_cast<int>(Ship_info.size()))){
		return 0;
	}
	if((shipp->ai_index < 0) || (shipp->ai_index >= MAX_AI_INFO)){
		return 0;
	}
	sip = &Ship_info[shipp->ship_info_index];
	aip = &Ai_info[shipp->ai_index];

	if ( swp->num_primary_banks <= 0 ) {
		return 0;
	}

	if ( swp->current_primary_bank < 0 ){
		return 0;
	}	

	// If the primaries have been locked, bail
	if (shipp->flags2 & SF2_PRIMARIES_LOCKED)
	{
		return 0;
	}

	sound_played = -1;

	// Fire the correct primary bank.  If primaries are linked (SF_PRIMARY_LINKED set), then fire 
	// both primary banks.
	int	num_primary_banks;

	if ( shipp->flags & SF_PRIMARY_LINKED ) {
		num_primary_banks = swp->num_primary_banks;
	} else {
		num_primary_banks = MIN(1, swp->num_primary_banks);
	}

	Assert(num_primary_banks > 0);
	if (num_primary_banks < 1){
		return 0;
	}

	// if we're firing stream weapons, but the trigger is not down, do nothing
	if(stream_weapons && !(shipp->flags & SF_TRIGGER_DOWN)){
		return 0;
	}

	if(num_primary_banks == 1)
		for(i = 0; i<swp->num_primary_banks; i++){
			if(i!=swp->current_primary_bank)ship_stop_fire_primary_bank(obj, i);
		}

	// lets start gun convergence / autoaim code from here - Wanderer
	has_converging_autoaim = ((sip->aiming_flags & AIM_FLAG_AUTOAIM_CONVERGENCE || (The_mission.ai_profile->player_autoaim_fov[Game_skill_level] > 0.0f && !( Game_mode & GM_MULTIPLAYER ))) && aip->target_objnum != -1);
	has_autoaim = ((has_converging_autoaim || (sip->aiming_flags & AIM_FLAG_AUTOAIM)) && aip->target_objnum != -1);
	needs_target_pos = ((has_autoaim || (sip->aiming_flags & AIM_FLAG_AUTO_CONVERGENCE)) && aip->target_objnum != -1);
	
	if (needs_target_pos) {
		if (has_autoaim) {
			autoaim_fov = MAX(shipp->autoaim_fov, The_mission.ai_profile->player_autoaim_fov[Game_skill_level]);
		}

		// If a subsystem is targeted, fire in that direction instead
		if (aip->targeted_subsys != NULL)
		{
			get_subsystem_world_pos(&Objects[aip->target_objnum], aip->targeted_subsys, &target_position);
		}
		else
		{
			target_position = Objects[aip->target_objnum].pos;
		}

		dist_to_target = vm_vec_dist_quick(&target_position, &obj->pos);
	}

	for ( i = 0; i < num_primary_banks; i++ ) {		
		// Goober5000 - allow more than two banks
		bank_to_fire = (swp->current_primary_bank+i) % swp->num_primary_banks;

		
		weapon_idx = swp->primary_bank_weapons[bank_to_fire];
		Assert( weapon_idx >= 0 && weapon_idx < MAX_WEAPON_TYPES );
		if ( (weapon_idx < 0) || (weapon_idx >= MAX_WEAPON_TYPES) ) {
			Int3();		// why would a ship try to fire a weapon that doesn't exist?
			continue;
		}		

		if (swp->primary_animation_position[bank_to_fire] == MA_POS_SET) {
			if ( timestamp_elapsed(swp->primary_animation_done_time[bank_to_fire]) )
				swp->primary_animation_position[bank_to_fire] = MA_POS_READY;
			else
				continue;
		}

		weapon_info* winfo_p = &Weapon_info[weapon_idx];

		if (needs_target_pos) {
			target_velocity_vec = Objects[aip->target_objnum].phys_info.vel;
			if (The_mission.ai_profile->flags & AIPF_USE_ADDITIVE_WEAPON_VELOCITY)
				vm_vec_scale_sub2(&target_velocity_vec, &obj->phys_info.vel, winfo_p->vel_inherit_amount);
		}

		if(sip->draw_primary_models[bank_to_fire]){
			if(shipp->primary_rotate_rate[bank_to_fire] < winfo_p->weapon_submodel_rotate_vel)
				shipp->primary_rotate_rate[bank_to_fire] += winfo_p->weapon_submodel_rotate_accell*flFrametime;
			if(shipp->primary_rotate_rate[bank_to_fire] > winfo_p->weapon_submodel_rotate_vel)shipp->primary_rotate_rate[bank_to_fire] = winfo_p->weapon_submodel_rotate_vel;
			shipp->primary_rotate_ang[bank_to_fire] += shipp->primary_rotate_rate[bank_to_fire]*flFrametime;
			if(shipp->primary_rotate_ang[bank_to_fire] > PI2)shipp->primary_rotate_ang[bank_to_fire] -= PI2;
			if(shipp->primary_rotate_ang[bank_to_fire] < 0.0f)shipp->primary_rotate_ang[bank_to_fire] += PI2;

			if(shipp->primary_rotate_rate[bank_to_fire] < winfo_p->weapon_submodel_rotate_vel)continue;
		}
		// if this is a targeting laser, start it up   ///- only targeting laser if it is tag-c, otherwise it's a fighter beam -Bobboau
		if((winfo_p->wi_flags & WIF_BEAM) && (winfo_p->tag_level == 3) && (shipp->flags & SF_TRIGGER_DOWN) && (winfo_p->b_info.beam_type == BEAM_TYPE_C) ){
			ship_start_targeting_laser(shipp);
			continue;
		}

		// if we're firing stream weapons and this is a non stream weapon, skip it
		if(stream_weapons && !(winfo_p->wi_flags & WIF_STREAM)){
			continue;
		}
		// if we're firing non stream weapons and this is a stream weapon, skip it
		if(!stream_weapons && (winfo_p->wi_flags & WIF_STREAM)){
			continue;
		}

		// only non-multiplayer clients (single, multi-host) need to do timestamp checking
		if ( !timestamp_elapsed(swp->next_primary_fire_stamp[bank_to_fire]) ) {
			continue;
		}

		// if weapons are linked and this is a nolink weapon, skip it
		if (shipp->flags & SF_PRIMARY_LINKED && winfo_p->wi_flags3 & WIF3_NOLINK) {
			continue;
		}

		// do timestamp stuff for next firing time
		float next_fire_delay;
		bool fast_firing = false;
		if (winfo_p->burst_shots > swp->burst_counter[bank_to_fire]) {
			next_fire_delay = winfo_p->burst_delay * 1000.0f;
			swp->burst_counter[bank_to_fire]++;
			if (winfo_p->burst_flags & WBF_FAST_FIRING)
				fast_firing = true;
		} else if (winfo_p->max_delay != 0.0f && winfo_p->min_delay != 0.0f) {			// Random fire delay (DahBlount)
			next_fire_delay = (((float)rand()) / (((float)RAND_MAX + 1.0f) * (winfo_p->max_delay - winfo_p->min_delay + 1.0f) + winfo_p->min_delay)) * 1000.0f;
		} else {
			next_fire_delay = winfo_p->fire_wait * 1000.0f;
			swp->burst_counter[bank_to_fire] = 0;
		}
		if (!((obj->flags & OF_PLAYER_SHIP) || (fast_firing))) {
			if (shipp->team == Ships[Player_obj->instance].team){
				next_fire_delay *= aip->ai_ship_fire_delay_scale_friendly;
			} else {
				next_fire_delay *= aip->ai_ship_fire_delay_scale_hostile;
			}
		}

		polymodel *pm = model_get( sip->model_num );
		
		// Goober5000 (thanks to _argv[-1] for the original idea)
		if ( (num_primary_banks > 1) &&  !(winfo_p->wi_flags3 & WIF3_NO_LINKED_PENALTY) && !(The_mission.ai_profile->flags & AIPF_DISABLE_LINKED_FIRE_PENALTY) )
		{
			int effective_primary_banks = 0;
			for (int it = 0; it < num_primary_banks; it++)
			{
				if ((it == bank_to_fire) || !(Weapon_info[swp->primary_bank_weapons[it]].wi_flags3 & (WIF3_NOLINK | WIF3_NO_LINKED_PENALTY)))
					effective_primary_banks++;
			}
			Assert(effective_primary_banks >= 1);

			next_fire_delay *= 1.0f + (effective_primary_banks - 1) * 0.5f;		//	50% time penalty if banks linked
		}

		if (winfo_p->fof_spread_rate > 0.0f)
		{
			//Adjust the primary_bank_fof_cooldown based on how long it's been since the last shot. 
			float reset_amount = (timestamp_until(swp->last_primary_fire_stamp[bank_to_fire]) / 1000.0f) * winfo_p->fof_reset_rate;
			swp->primary_bank_fof_cooldown[bank_to_fire] += winfo_p->fof_spread_rate + reset_amount;
			CLAMP(swp->primary_bank_fof_cooldown[bank_to_fire], 0.0f, 1.0f);
		}

		//	MK, 2/4/98: Since you probably were allowed to fire earlier, but couldn't fire until your frame interval
		//	rolled around, subtract out up to half the previous frametime.
		//	Note, unless we track whether the fire button has been held down, and not tapped, it's hard to
		//	know how much time to subtract off.  It could be this fire is "late" because the user didn't want to fire.
		if ((next_fire_delay > 0.0f)) {
			if (obj->flags & OF_PLAYER_SHIP) {
				int	t = timestamp_until(swp->next_primary_fire_stamp[bank_to_fire]);
				if (t < 0) {
					float	tx;

					tx = (float) t/-1000.0f;
					if (tx > flFrametime/2.0f){
						tx = 1000.0f * flFrametime * 0.7f;
					}
					next_fire_delay -= tx;
				}
				
				if ((int) next_fire_delay < 1){
					next_fire_delay = 1.0f;
				}
			}

			swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int)(next_fire_delay));
			swp->last_primary_fire_stamp[bank_to_fire] = timestamp();
		}

		if (sip->flags2 & SIF2_DYN_PRIMARY_LINKING ) {
			Assert(pm->gun_banks[bank_to_fire].num_slots != 0);
			swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int)(next_fire_delay * ( swp->primary_bank_slot_count[ bank_to_fire ] ) / pm->gun_banks[bank_to_fire].num_slots ) );
			swp->last_primary_fire_stamp[bank_to_fire] = timestamp();
		} else if (winfo_p->wi_flags2 & WIF2_CYCLE) {
			Assert(pm->gun_banks[bank_to_fire].num_slots != 0);
			swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int)(next_fire_delay / pm->gun_banks[bank_to_fire].num_slots));
			swp->last_primary_fire_stamp[bank_to_fire] = timestamp();
			//to maintain balance of fighters with more fire points they will fire faster than ships with fewer points
		}else{
			swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int)(next_fire_delay));
			swp->last_primary_fire_stamp[bank_to_fire] = timestamp();
		}
		// Here is where we check if weapons subsystem is capable of firing the weapon.
		// Note that we can have partial bank firing, if the weapons subsystem is partially
		// functional, which should be cool.  		
		if ( ship_weapon_maybe_fail(shipp) && !force) {
			if ( obj == Player_obj ) {
				ship_maybe_play_primary_fail_sound();
			}
			ship_stop_fire_primary_bank(obj, bank_to_fire);
			continue;
		}		
		

		if ( pm->n_guns > 0 ) {
			int num_slots = pm->gun_banks[bank_to_fire].num_slots;
			vec3d predicted_target_pos, plr_to_target_vec;
			vec3d player_forward_vec = obj->orient.vec.fvec;
			bool in_automatic_aim_fov = false;
			float dist_to_aim = 0;

			// more autoaim stuff here - Wanderer
			// needs weapon speed
			if (needs_target_pos) {
				float time_to_target, angle_to_target;
				vec3d last_delta_vec;

				time_to_target = 0.0f;

				if (winfo_p->max_speed != 0)
				{
					time_to_target = dist_to_target / winfo_p->max_speed;
				}

				vm_vec_scale_add(&predicted_target_pos, &target_position, &target_velocity_vec, time_to_target);
				polish_predicted_target_pos(winfo_p, &Objects[aip->target_objnum], &target_position, &predicted_target_pos, dist_to_target, &last_delta_vec, 1);
				vm_vec_sub(&plr_to_target_vec, &predicted_target_pos, &obj->pos);

				if (has_autoaim) {
					angle_to_target = vm_vec_delta_ang(&player_forward_vec, &plr_to_target_vec, NULL);
					if (angle_to_target < autoaim_fov)
						in_automatic_aim_fov = true;
				}

				dist_to_aim = vm_vec_mag_quick(&plr_to_target_vec);

				// minimum convergence distance
				if (sip->minimum_convergence_distance > dist_to_aim) {
					float dist_mult;
					dist_mult = sip->minimum_convergence_distance / dist_to_aim;
					vm_vec_scale_add(&predicted_target_pos, &obj->pos, &plr_to_target_vec, dist_mult);
				}
			}
			
			if(winfo_p->wi_flags & WIF_BEAM){		// the big change I made for fighter beams, if there beams fill out the Fire_Info for a targeting laser then fire it, for each point in the weapon bank -Bobboau
				float t;
				if (winfo_p->burst_shots > swp->burst_counter[bank_to_fire]) {
					t = winfo_p->burst_delay;
					swp->burst_counter[bank_to_fire]++;
				} else {
					t = winfo_p->fire_wait;//doing that time scale thing on enemy fighter is just ugly with beams, especaly ones that have careful timeing
					swp->burst_counter[bank_to_fire] = 0;
				}
				swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int) (t * 1000.0f));
				swp->last_primary_fire_stamp[bank_to_fire] = timestamp();
				beam_fire_info fbfire_info;				

				int points;
				if (winfo_p->b_info.beam_shots){
					if (winfo_p->b_info.beam_shots > num_slots){
						points = num_slots;
					}else{
						points = winfo_p->b_info.beam_shots;
					}
				}else{
					points = num_slots;
				}

				if ( shipp->weapon_energy < points*winfo_p->energy_consumed*flFrametime)
				{
					swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int)(next_fire_delay));
					if ( obj == Player_obj )
					{
						ship_maybe_play_primary_fail_sound();
					}
					ship_stop_fire_primary_bank(obj, bank_to_fire);
					continue;
				}			
				
				shipp->beam_sys_info.turret_norm.xyz.x = 0.0f;
				shipp->beam_sys_info.turret_norm.xyz.y = 0.0f;
				shipp->beam_sys_info.turret_norm.xyz.z = 1.0f;
				shipp->beam_sys_info.model_num = sip->model_num;
				shipp->beam_sys_info.turret_gun_sobj = pm->detail[0];
				shipp->beam_sys_info.turret_num_firing_points = 1;  // dummy turret info is used per firepoint
				shipp->beam_sys_info.turret_fov = (float)cos((winfo_p->field_of_fire != 0.0f)?winfo_p->field_of_fire:180);

				shipp->fighter_beam_turret_data.disruption_timestamp = timestamp(0);
				shipp->fighter_beam_turret_data.turret_next_fire_pos = 0;
				shipp->fighter_beam_turret_data.current_hits = 1.0;
				shipp->fighter_beam_turret_data.system_info = &shipp->beam_sys_info;
				
				fbfire_info.target_subsys = Ai_info[shipp->ai_index].targeted_subsys;
				fbfire_info.beam_info_index = shipp->weapons.primary_bank_weapons[bank_to_fire];
				fbfire_info.beam_info_override = NULL;
				fbfire_info.shooter = &Objects[shipp->objnum];
				
				if (aip->target_objnum >= 0) {
					fbfire_info.target = &Objects[aip->target_objnum];
				} else {
					fbfire_info.target = NULL;
				}
				fbfire_info.turret = &shipp->fighter_beam_turret_data;
				fbfire_info.bfi_flags = BFIF_IS_FIGHTER_BEAM;
				fbfire_info.bank = bank_to_fire;

				for ( v = 0; v < points; v++ ){
					if(winfo_p->b_info.beam_shots){
						j = (shipp->last_fired_point[bank_to_fire]+1)%num_slots;
						shipp->last_fired_point[bank_to_fire] = j;
					}else{
						j=v;
					}

					fbfire_info.targeting_laser_offset = pm->gun_banks[bank_to_fire].pnt[j];
					shipp->beam_sys_info.pnt = pm->gun_banks[bank_to_fire].pnt[j];
					shipp->beam_sys_info.turret_firing_point[0] = pm->gun_banks[bank_to_fire].pnt[j];

					fbfire_info.point = j;

					beam_fire(&fbfire_info);
					has_fired = true;
					num_fired++;
				}
			}
			else	//if this isn't a fighter beam, do it normally -Bobboau
			{
				int points = 0, numtimes = 1;

				// ok if this is a cycling weapon use shots as the number of points to fire from at a time
				// otherwise shots is the number of times all points will be fired (used mostly for the 'shotgun' effect)
				if ( sip->flags2 & SIF2_DYN_PRIMARY_LINKING ) {
					numtimes = 1;
					points = MIN( num_slots, swp->primary_bank_slot_count[ bank_to_fire ] );
				} else if ( winfo_p->wi_flags2 & WIF2_CYCLE ) {
					numtimes = 1;
					points = MIN(num_slots, winfo_p->shots);
				} else {
					numtimes = winfo_p->shots;
					points = num_slots;
				}

				// The energy-consumption code executes even for ballistic primaries, because
				// there may be a reason why you want to have ballistics consume energy.  Perhaps
				// you can't fire too many too quickly or they'll overheat.  If not, just set
				// the weapon's energy_consumed to 0 and it'll work just fine. - Goober5000

				// fail unless we're forcing (energy based primaries)
				if ( (shipp->weapon_energy < points*numtimes * winfo_p->energy_consumed)			//was num_slots
				 && !force ) {

					swp->next_primary_fire_stamp[bank_to_fire] = timestamp((int)(next_fire_delay));
					if ( obj == Player_obj )
					{
						ship_maybe_play_primary_fail_sound();
					}
					ship_stop_fire_primary_bank(obj, bank_to_fire);
					continue;
				}
				// moved the above to here to use points instead of num_slots for energy consumption check

				// ballistics support for primaries - Goober5000
				if ( winfo_p->wi_flags2 & WIF2_BALLISTIC )
				{
					// Make sure this ship is set up for ballistics.
					// If you get this error, add the ballistic primaries tags to ships.tbl.
					Assert ( sip->flags & SIF_BALLISTIC_PRIMARIES );

					// If ship is being repaired/rearmed, it cannot fire ballistics
					if ( aip->ai_flags & AIF_BEING_REPAIRED )
					{
						continue;
					}

					// duplicated from the secondaries firing routine...
					// determine if there is enough ammo left to fire weapons on this bank.  As with primary
					// weapons, we might or might not check ammo counts depending on game mode, who is firing,
					// and if I am a client in multiplayer
					int check_ammo = 1;

					if ( MULTIPLAYER_CLIENT && (obj != Player_obj) )
					{
						check_ammo = 0;
					}

					// not enough ammo
					if ( check_ammo && ( swp->primary_bank_ammo[bank_to_fire] <= 0) )
					{
						if ( obj == Player_obj )
						{
							ship_maybe_play_primary_fail_sound();
						}
						else
						{
							// TODO:  AI switch primary weapon / re-arm?
						}
						continue;
					}
					
					// deplete ammo
					if ( Weapon_energy_cheat == 0 )
					{
						swp->primary_bank_ammo[bank_to_fire] -= points*numtimes;

						// make sure we don't go below zero; any such error is excusable
						// because it only happens when the bank is depleted in one shot
						if (swp->primary_bank_ammo[bank_to_fire] < 0)
						{
							swp->primary_bank_ammo[bank_to_fire] = 0;
						}
					}
				}

				// now handle the energy as usual
				// deplete the weapon reserve energy by the amount of energy used to fire the weapon	
				// Only subtract the energy amount required for equipment operation once
				shipp->weapon_energy -= points*numtimes * winfo_p->energy_consumed;
				// note for later: option for fuel!
				
				// Mark all these weapons as in the same group
				int new_group_id = weapon_create_group_id();

				vec3d total_impulse;
				vec3d *firepoint_list;
				size_t current_firepoint = 0;

				if (winfo_p->wi_flags3 & WIF3_APPLY_RECOIL){
					firepoint_list = new vec3d[numtimes * points];
					vm_vec_zero(&total_impulse);
				} else {
					firepoint_list = nullptr;
				}

				for ( w = 0; w < numtimes; w++ ) {
					polymodel *weapon_model = NULL;
					if(winfo_p->external_model_num >= 0) 
						weapon_model = model_get(winfo_p->external_model_num);

					if (weapon_model)
						if ((weapon_model->n_guns <= swp->external_model_fp_counter[bank_to_fire]) || (swp->external_model_fp_counter[bank_to_fire] < 0))
							swp->external_model_fp_counter[bank_to_fire] = 0;

					for ( j = 0; j < points; j++ ) {
						int pt; //point
						if ( (winfo_p->wi_flags2 & WIF2_CYCLE) || (sip->flags2 & SIF2_DYN_PRIMARY_LINKING) ){
							pt = (shipp->last_fired_point[bank_to_fire]+1)%num_slots;
						}else{
							pt = j;
						}

						int sub_shots = 1;
						// Use 0 instead of bank_to_fire as index when checking the number of external weapon model firingpoints
						if (weapon_model && weapon_model->n_guns)
							if (!(winfo_p->wi_flags2 & WIF2_EXTERNAL_WEAPON_FP))
								sub_shots = weapon_model->gun_banks[0].num_slots;

						for(int s = 0; s<sub_shots; s++){
							pnt = pm->gun_banks[bank_to_fire].pnt[pt];
							// Use 0 instead of bank_to_fire as index to external weapon model firingpoints 
							if (weapon_model && weapon_model->n_guns) {
								if (winfo_p->wi_flags2 & WIF2_EXTERNAL_WEAPON_FP) {
									vm_vec_add2(&pnt, &weapon_model->gun_banks[0].pnt[swp->external_model_fp_counter[bank_to_fire]]);
								} else {
									vm_vec_add2(&pnt, &weapon_model->gun_banks[0].pnt[s]);
								}
							}

							vm_vec_unrotate(&gun_point, &pnt, &obj->orient);
							vm_vec_add(&firing_pos, &gun_point, &obj->pos);

							matrix firing_orient;
							
							/*	I AIM autoaim convergence
								II AIM autoaim
								III AIM auto convergence
								IV AIM std convergence
								V SIF convergence
								no convergence or autoaim
							*/
							if (has_autoaim && in_automatic_aim_fov) {
								vec3d firing_vec;

								if (has_converging_autoaim) {
									// converging autoaim
									vm_vec_sub(&firing_vec, &predicted_target_pos, &firing_pos);
								} else {
									// autoaim
									vm_vec_sub(&firing_vec, &predicted_target_pos, &obj->pos);
								}

								vm_vector_2_matrix(&firing_orient, &firing_vec, NULL, NULL);
							} else if ((sip->aiming_flags & AIM_FLAG_STD_CONVERGENCE) || ((sip->aiming_flags & AIM_FLAG_AUTO_CONVERGENCE) && (aip->target_objnum != -1))) {
								// std & auto convergence
								vec3d target_vec, firing_vec, convergence_offset;
								
								// make sure vector is of the set length
								vm_vec_copy_normalize(&target_vec, &player_forward_vec);
								if ((sip->aiming_flags & AIM_FLAG_AUTO_CONVERGENCE) && (aip->target_objnum != -1)) {
									// auto convergence
									vm_vec_scale(&target_vec, dist_to_aim);
								} else {
									// std convergence
									vm_vec_scale(&target_vec, sip->convergence_distance);
								}
								
								// if there is convergence offset then make use of it)
								if (sip->aiming_flags & AIM_FLAG_CONVERGENCE_OFFSET) {
									vm_vec_unrotate(&convergence_offset, &sip->convergence_offset, &obj->orient);
									vm_vec_add2(&target_vec, &convergence_offset);
								}

								vm_vec_add2(&target_vec, &obj->pos);
								vm_vec_sub(&firing_vec, &target_vec, &firing_pos);

								// set orientation
								vm_vector_2_matrix(&firing_orient, &firing_vec, NULL, NULL);
							} else if (sip->aiming_flags & AIM_FLAG_STD_CONVERGENCE) {
								// fixed distance convergence
								vec3d target_vec, firing_vec, convergence_offset;
																
								// make sure vector is of the set length
								vm_vec_copy_normalize(&target_vec, &player_forward_vec);
								vm_vec_scale(&target_vec, sip->convergence_distance);
								
								// if there is convergence offset then make use of it)
								if (sip->aiming_flags & AIM_FLAG_CONVERGENCE_OFFSET) {
									vm_vec_unrotate(&convergence_offset, &sip->convergence_offset, &obj->orient);
									vm_vec_add2(&target_vec, &convergence_offset);
								}

								vm_vec_add2(&target_vec, &obj->pos);
								vm_vec_sub(&firing_vec, &target_vec, &firing_pos);

								// set orientation
								vm_vector_2_matrix(&firing_orient, &firing_vec, NULL, NULL);
							} else if (sip->flags2 & SIF2_GUN_CONVERGENCE) {
								// model file defined convergence
								vec3d firing_vec;
								vm_vec_unrotate(&firing_vec, &pm->gun_banks[bank_to_fire].norm[pt], &obj->orient);
								vm_vector_2_matrix(&firing_orient, &firing_vec, NULL, NULL);
							} else {
								// no autoaim or convergence
								firing_orient = obj->orient;
							}
							
							if (winfo_p->wi_flags3 & WIF3_APPLY_RECOIL){	// Function to add recoil functionality - DahBlount
								vec3d local_impulse = firing_orient.vec.fvec;
								
								float recoil_force = (winfo_p->mass * winfo_p->max_speed * winfo_p->recoil_modifier * sip->ship_recoil_modifier);

								firepoint_list[current_firepoint++] = firing_pos;

								vm_vec_scale(&local_impulse, (-1 * recoil_force));
								vm_vec_add2(&total_impulse, &local_impulse);
							}

							// create the weapon -- the network signature for multiplayer is created inside
							// of weapon_create							
							weapon_objnum = weapon_create( &firing_pos, &firing_orient, weapon_idx, OBJ_INDEX(obj), new_group_id,
								0, 0, swp->primary_bank_fof_cooldown[bank_to_fire] );
							winfo_p = &Weapon_info[Weapons[Objects[weapon_objnum].instance].weapon_info_index];
							has_fired = true;

							weapon_set_tracking_info(weapon_objnum, OBJ_INDEX(obj), aip->target_objnum, aip->current_target_is_locked, aip->targeted_subsys);				

							if (winfo_p->wi_flags & WIF_FLAK)
							{
								object *target;
								vec3d predicted_pos;
								float flak_range=(winfo_p->lifetime)*(winfo_p->max_speed);
								float range_to_target = flak_range;
								float wepstr=ship_get_subsystem_strength(shipp, SUBSYSTEM_WEAPONS);

								if (aip->target_objnum != -1) {
									target = &Objects[aip->target_objnum];
								} else {
									target = NULL;
								}

								if (target != NULL) {
									set_predicted_enemy_pos(&predicted_pos, obj, &target->pos, &target->phys_info.vel, aip);
									range_to_target=vm_vec_dist(&predicted_pos, &obj->pos);
								}

								//if we have a target and its in range
								if ( (target != NULL) && (range_to_target < flak_range) )
								{
									//set flak range to range of ship
									flak_pick_range(&Objects[weapon_objnum], &firing_pos, &predicted_pos,wepstr);
								}
								else
								{
									flak_set_range(&Objects[weapon_objnum], flak_range - winfo_p->untargeted_flak_range_penalty);
								}

								if ((winfo_p->muzzle_flash>=0) && (((shipp==Player_ship) && (vm_vec_mag(&Player_obj->phys_info.vel)>=45)) || (shipp!=Player_ship)))
								{
									flak_muzzle_flash(&firing_pos,&obj->orient.vec.fvec, &obj->phys_info, swp->primary_bank_weapons[bank_to_fire]);
								}
							}
							// create the muzzle flash effect
							if ( (obj != Player_obj) || (sip->flags2 & SIF2_SHOW_SHIP_MODEL) || (Viewer_mode) ) {
								// show the flash only if in not cockpit view, or if "show ship" flag is set
								shipfx_flash_create( obj, sip->model_num, &pnt, &obj->orient.vec.fvec, 1, weapon_idx );
							}

							// maybe shudder the ship - if its me
							if((winfo_p->wi_flags & WIF_SHUDDER) && (obj == Player_obj) && !(Game_mode & GM_STANDALONE_SERVER)){
								// calculate some arbitrary value between 100
								// (mass * velocity) / 10
								game_shudder_apply(500, (winfo_p->mass * winfo_p->max_speed) * 0.1f);
							}

							num_fired++;
							shipp->last_fired_point[bank_to_fire] = (shipp->last_fired_point[bank_to_fire] + 1) % num_slots;
						}
					}
					swp->external_model_fp_counter[bank_to_fire]++;
				}
				if (winfo_p->wi_flags3 & WIF3_APPLY_RECOIL){
					vec3d avg_firepoint;

					vm_vec_avg_n(&avg_firepoint, current_firepoint, firepoint_list);

					ship_apply_whack(&total_impulse, &avg_firepoint, obj);
					delete[] firepoint_list;
				}
			}

			CLAMP(shipp->weapon_energy, 0.0f, sip->max_weapon_reserve);

			banks_fired |= (1<<bank_to_fire);				// mark this bank as fired.
		}

		
		// Only play the weapon fired sound if it hasn't been played yet.  This is to 
		// avoid playing the same sound multiple times when banks are linked with the
		// same weapon.

		if (!(winfo_p->wi_flags & WIF_BEAM)){	// not a beam weapon?
			if ( sound_played != winfo_p->launch_snd ) {
				sound_played = winfo_p->launch_snd;
				if ( obj == Player_obj ) {
					if ( winfo_p->launch_snd != -1 ) {
						weapon_info *wip;
						ship_weapon *sw_pl;

						//Update the last timestamp until continous fire is over, so we have the timestamp of the cease-fire.
						if (shipp->was_firing_last_frame[bank_to_fire] == 1) {
							swp->last_primary_fire_sound_stamp[bank_to_fire] = timestamp();
						}

						//Check for pre-launch sound and play if relevant
						if( (winfo_p->pre_launch_snd != -1)									//If this weapon type has a pre-fire sound
							&& ((timestamp() - swp->last_primary_fire_sound_stamp[bank_to_fire]) >= winfo_p->pre_launch_snd_min_interval)	//and if we're past our minimum delay from the last cease-fire
							&& (shipp->was_firing_last_frame[bank_to_fire] == 0)				//and if we are at the beginning of a firing stream
						){ 
							snd_play( &Snds[winfo_p->pre_launch_snd], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY); //play it 
						} else { //Otherwise, play normal firing sounds
							// HACK
							if(winfo_p->launch_snd == SND_AUTOCANNON_SHOT){
								snd_play( &Snds[winfo_p->launch_snd], 0.0f, 1.0f, SND_PRIORITY_TRIPLE_INSTANCE );
							} else {
								snd_play( &Snds[winfo_p->launch_snd], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY );
							}
						}
	
						sw_pl = &Player_ship->weapons;
						if (sw_pl->current_primary_bank >= 0)
						{
							wip = &Weapon_info[sw_pl->primary_bank_weapons[sw_pl->current_primary_bank]];
							int force_level = (int) ((wip->armor_factor + wip->shield_factor * 0.2f) * (wip->damage * wip->damage - 7.5f) * 0.45f + 0.6f) * 10 + 2000;

							// modify force feedback for ballistics: make it stronger
							if (wip->wi_flags2 & WIF2_BALLISTIC)
								joy_ff_play_primary_shoot(force_level * 2);
							// no ballistics
							else
								joy_ff_play_primary_shoot(force_level);
						}
					}
				}else {
					if ( winfo_p->launch_snd != -1 ) {
						snd_play_3d( &Snds[winfo_p->launch_snd], &obj->pos, &View_position );
					}	
				}
			}	
		}

		shipp->was_firing_last_frame[bank_to_fire] = 1;
	}	// end for (go to next primary bank)
	
	// if multiplayer and we're client-side firing, send the packet
	if(Game_mode & GM_MULTIPLAYER){
		// if i'm a client, and this is not me, don't send
		if(!(MULTIPLAYER_CLIENT && (shipp != Player_ship))){
			send_NEW_primary_fired_packet( shipp, banks_fired );
		}
	}

   // STATS
   if (obj->flags & OF_PLAYER_SHIP) {
		// in multiplayer -- only the server needs to keep track of the stats.  Call the cool
		// function to find the player given the object *.  It had better return a valid player
		// or our internal structure as messed up.
		if( Game_mode & GM_MULTIPLAYER ) {
			if ( Net_player->flags & NETINFO_FLAG_AM_MASTER ) {
				int player_num;

				player_num = multi_find_player_by_object ( obj );
				Assert ( player_num != -1 );

				Net_players[player_num].m_player->stats.mp_shots_fired += num_fired;
			}
		} else {
			Player->stats.mp_shots_fired += num_fired;
		}
	}

	if (has_fired) {
		object *objp = &Objects[shipp->objnum];
		object* target;
		if (Ai_info[shipp->ai_index].target_objnum != -1)
			target = &Objects[Ai_info[shipp->ai_index].target_objnum];
		else
			target = NULL;
		if (objp == Player_obj && Player_ai->target_objnum != -1)
			target = &Objects[Player_ai->target_objnum]; 

		Script_system.SetHookObjects(2, "User", objp, "Target", target);
		Script_system.RunCondition(CHA_ONWPFIRED, 0, NULL, objp, 1);

		Script_system.RunCondition(CHA_PRIMARYFIRE, 0, NULL, objp);
		Script_system.RemHookVars(2, "User", "Target");
	}

	return num_fired;
}

void ship_start_targeting_laser(ship *shipp)
{	
	int bank0_laser = 0;
	int bank1_laser = 0;

	// determine if either of our banks have a targeting laser
	if((shipp->weapons.primary_bank_weapons[0] >= 0) && (Weapon_info[shipp->weapons.primary_bank_weapons[0]].wi_flags & WIF_BEAM) && (Weapon_info[shipp->weapons.primary_bank_weapons[0]].b_info.beam_type == BEAM_TYPE_C)){
		bank0_laser = 1;
	}
	if((shipp->weapons.primary_bank_weapons[1] >= 0) && (Weapon_info[shipp->weapons.primary_bank_weapons[1]].wi_flags & WIF_BEAM) && (Weapon_info[shipp->weapons.primary_bank_weapons[1]].b_info.beam_type == BEAM_TYPE_C)){
		bank1_laser = 1;
	}

	// if primary banks are linked
	if(shipp->flags & SF_PRIMARY_LINKED){
		if(bank0_laser){
			shipp->targeting_laser_bank = 0;
			return;
		} 
		if(bank1_laser){
			shipp->targeting_laser_bank = 1;
			return;
		}
	}
	// if we only have 1 bank selected
	else {
		if(bank0_laser && (shipp->weapons.current_primary_bank == 0)){
			shipp->targeting_laser_bank = 0;
			return;
		}
		if(bank1_laser && (shipp->weapons.current_primary_bank == 1)){
			shipp->targeting_laser_bank = 1;
			return;
		}
	}
}

void ship_stop_targeting_laser(ship *shipp)
{
	shipp->targeting_laser_bank = -1;
	shipp->targeting_laser_objnum = -1; // erase old laser obj num if it has any -Bobboau
}

void ship_process_targeting_lasers()
{
	fighter_beam_fire_info fire_info;
	ship_obj *so;
	ship *shipp;	
	polymodel *m;

	// interate over all ships
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		// sanity checks
		if(so->objnum < 0){
			continue;
		}
		if(Objects[so->objnum].type != OBJ_SHIP){
			continue;
		}
		if(Objects[so->objnum].instance < 0){
			continue;
		}
		shipp = &Ships[Objects[so->objnum].instance];

		// if our trigger is no longer down, switch it off
		if(!(shipp->flags & SF_TRIGGER_DOWN)){
			ship_stop_targeting_laser(shipp);
			continue;
		}		

		// if we have a bank to fire - fire it
		if((shipp->targeting_laser_bank >= 0) && (shipp->targeting_laser_bank < 2)){
			// try and get the model
			m = model_get(Ship_info[shipp->ship_info_index].model_num);
			if(m == NULL){
				continue;
			}

			// fire a targeting laser
			fire_info.life_left = 0.0;					//for fighter beams
			fire_info.life_total = 0.0f;					//for fighter beams
			fire_info.warmdown_stamp = -1;				//for fighter beams
			fire_info.warmup_stamp = -1;				//for fighter beams
			fire_info.accuracy = 0.0f;
			fire_info.beam_info_index = shipp->weapons.primary_bank_weapons[(int)shipp->targeting_laser_bank];
			fire_info.beam_info_override = NULL;
			fire_info.shooter = &Objects[shipp->objnum];
			fire_info.target = NULL;
			fire_info.target_subsys = NULL;
			fire_info.turret = NULL;
			fire_info.targeting_laser_offset = m->gun_banks[shipp->targeting_laser_bank].pnt[0];			
			shipp->targeting_laser_objnum = beam_fire_targeting(&fire_info);			

			// hmm, why didn't it fire?
			if(shipp->targeting_laser_objnum < 0){
				Int3();
				ship_stop_targeting_laser(shipp);
			}
		}
	}}

/**
 * Attempt to detonate weapon last fired by *src.
 * Only used for weapons that support remote detonation.
 * 
 * @param swp	Ship weapon
 * @param src	Source of weapon
 * @return true if detonated, else return false.
 * 
 *	Calls ::weapon_hit(), indirectly via ::weapon_detonate(), to detonate weapon.
 *	If it's a weapon that spawns particles, those will be released.
 */
int maybe_detonate_weapon(ship_weapon *swp, object *src)
{
	int			objnum = swp->last_fired_weapon_index;
	object		*objp;
	weapon_info	*wip;

	if ((objnum < 0) || (objnum >= MAX_OBJECTS)) {
		return 0;
	}
    
	objp = &Objects[objnum];

	if (objp->type != OBJ_WEAPON){
		return 0;
	}

	if ((objp->instance < 0) || (objp->instance >= MAX_WEAPONS)){
		return 0;
	}

	// check to make sure that the weapon to detonate still exists
	if ( swp->last_fired_weapon_signature != objp->signature ){
		return 0;
	}

	Assert(Weapons[objp->instance].weapon_info_index != -1);
	wip = &Weapon_info[Weapons[objp->instance].weapon_info_index];

	if (wip->wi_flags & WIF_REMOTE) {

		int	weapon_sig;

		weapon_sig = objp->signature;

		if (swp->last_fired_weapon_signature == weapon_sig) {
			weapon_detonate(objp);
			swp->last_fired_weapon_index = -1;

			return 1;
		}
	}

	return 0;
}

/**
 * Maybe detonate secondary weapon that's already out.
 * @return Return true if we detonate it, false if not.
 */
int ship_fire_secondary_detonate(object *obj, ship_weapon *swp)
{
	if (swp->last_fired_weapon_index != -1)
		if (timestamp_elapsed(swp->detonate_weapon_time)) {
			object	*first_objp = &Objects[swp->last_fired_weapon_index];
			if (maybe_detonate_weapon(swp, obj)) {
				//	If dual fire was set, there could be another weapon to detonate.  Scan all weapons.
				missile_obj	*mo;

				// check for currently locked missiles (highest precedence)
				for ( mo = GET_FIRST(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
					object	*mobjp;
					Assert(mo->objnum >= 0 && mo->objnum < MAX_OBJECTS);
					mobjp = &Objects[mo->objnum];
					if ((mobjp != first_objp) && (mobjp->parent_sig == obj->parent_sig)) {
						if (Weapon_info[Weapons[mobjp->instance].weapon_info_index].wi_flags & WIF_REMOTE) {
							weapon_detonate(mobjp);
						}
					}
				}
				
				return 1;
			}
		}

	return 0;
}

/**
 * Try to switch to a secondary bank that has ammo
 *
 * @note: not currently used - mark for removal?
 */
int ship_select_next_valid_secondary_bank(ship_weapon *swp)
{
	int cycled=0;

	int ns = swp->num_secondary_banks;

	if ( ns > 1 ) {
		int i,j=swp->current_secondary_bank+1;
		for (i=0; i<ns; i++) {
			if ( j >= ns ) {
				j=0;
			}

			if ( swp->secondary_bank_ammo[j] > 0 ) {
				swp->current_secondary_bank=j;
				cycled = 1;
				break;
			}

			j++;
		}
	}

	return cycled;
}


extern void ai_maybe_announce_shockwave_weapon(object *firing_objp, int weapon_index);

//	Object *obj fires its secondary weapon, if it can.
//	If its most recently fired weapon is a remotely detonatable weapon, detonate it.
//	Returns number of weapons fired.  Note, for swarmers, returns 1 if it is allowed
//	to fire the missiles when allow_swarm is NOT set.  They don't actually get fired on a call here unless allow_swarm is set.
//	When you want to fire swarmers, you call this function with allow_swarm NOT set and frame interval
//	code comes aruond and fires it.
// allow_swarm -> default value is 0... since swarm missiles are fired over several frames,
//                need to avoid firing when normally called
int ship_fire_secondary( object *obj, int allow_swarm )
{
	int			n, weapon_idx, j, bank, bank_adjusted, starting_bank_count = -1, num_fired;
	ushort		starting_sig = 0;
	ship			*shipp;
	ship_weapon *swp;
	ship_info	*sip;
	weapon_info	*wip;
	ai_info		*aip;
	polymodel	*pm;
	vec3d		missile_point, pnt, firing_pos;
	bool has_fired = false;		// Used to determine whether to fire the scripting hook

	Assert( obj != NULL );

	// in the case where the server is an observer, he can fire (which would be bad) - unless we do this.
	if( obj->type == OBJ_OBSERVER ){
		return 0;
	}

	// in the case where the object is a ghost (a delayed fire packet from right before he died, for instance)
	if( (obj->type == OBJ_GHOST) || (obj->type == OBJ_NONE) ){
		return 0;
	}

	Assert( obj->type == OBJ_SHIP );
	if(obj->type != OBJ_SHIP){
		return 0;
	}
	n = obj->instance;
	Assert( n >= 0 && n < MAX_SHIPS );
	if((n < 0) || (n >= MAX_SHIPS)){
		return 0;
	}
	Assert( Ships[n].objnum == OBJ_INDEX(obj));
	if(Ships[n].objnum != OBJ_INDEX(obj)){
		return 0;
	}
	
	shipp = &Ships[n];
	swp = &shipp->weapons;
	sip = &Ship_info[shipp->ship_info_index];
	aip = &Ai_info[shipp->ai_index];

	// if no secondary weapons are present on ship, return
	if ( swp->num_secondary_banks <= 0 ){
		return 0;
	}

	// If the secondaries have been locked, bail
	if (shipp->flags2 & SF2_SECONDARIES_LOCKED)
	{
		return 0;
	}

	// If ship is being repaired/rearmed, it cannot fire missiles
	if ( aip->ai_flags & AIF_BEING_REPAIRED ) {
		return 0;
	}

	num_fired = 0;		// tracks how many missiles actually fired

	// niffiwan: allow swarm/corkscrew bank to keep firing if current bank changes
	if (shipp->swarm_missile_bank != -1 && allow_swarm) {
		bank = shipp->swarm_missile_bank;
	} else if (shipp->corkscrew_missile_bank != -1 && allow_swarm) {
		bank = shipp->corkscrew_missile_bank;
	} else {
		bank = swp->current_secondary_bank;
	}

	if ( bank < 0 || bank >= sip->num_secondary_banks ) {
		return 0;
	}
	bank_adjusted = MAX_SHIP_PRIMARY_BANKS + bank;

	if (swp->secondary_animation_position[bank] == MA_POS_SET) {
		if ( timestamp_elapsed(swp->secondary_animation_done_time[bank]) )
			swp->secondary_animation_position[bank] = MA_POS_READY;
		else
			return 0;
	}

	weapon_idx = swp->secondary_bank_weapons[bank];
	Assert( (swp->secondary_bank_weapons[bank] >= 0) && (swp->secondary_bank_weapons[bank] < MAX_WEAPON_TYPES) );
	if((swp->secondary_bank_weapons[bank] < 0) || (swp->secondary_bank_weapons[bank] >= MAX_WEAPON_TYPES)){
		return 0;
	}
	wip = &Weapon_info[weapon_idx];

	if ( MULTIPLAYER_MASTER ) {
		starting_sig = multi_get_next_network_signature( MULTI_SIG_NON_PERMANENT );
		starting_bank_count = swp->secondary_bank_ammo[bank];
	}

	if (ship_fire_secondary_detonate(obj, swp)) {
		// in multiplayer, master sends a secondary fired packet with starting signature of -1 -- indicates
		// to client code to set the detonate timer to 0.
		if ( MULTIPLAYER_MASTER ) {
			send_secondary_fired_packet( shipp, 0, starting_bank_count, 1, allow_swarm );
		}
	
		//	For all banks, if ok to fire a weapon, make it wait a bit.
		//	Solves problem of fire button likely being down next frame and
		//	firing weapon despite fire causing detonation of existing weapon.
		if (swp->current_secondary_bank >= 0) {
			if (timestamp_elapsed(swp->next_secondary_fire_stamp[bank])){
				swp->next_secondary_fire_stamp[bank] = timestamp(MAX((int) flFrametime*3000, 250));
			}
		}
		return 0;
	}

	// niffiwan: 04/03/12: duplicate of a check approx 100 lines above - not needed?
	if ( bank < 0 ){
		return 0;
	}

	if ( !timestamp_elapsed(swp->next_secondary_fire_stamp[bank]) && !allow_swarm) {
		if (timestamp_until(swp->next_secondary_fire_stamp[bank]) > 60000){
			swp->next_secondary_fire_stamp[bank] = timestamp(1000);
		}
		goto done_secondary;
	}

	// Ensure if this is a "require-lock" missile, that a lock actually exists
	if ( wip->wi_flags & WIF_NO_DUMBFIRE ) {
		if ( aip->current_target_is_locked <= 0 ) {
			if ( obj == Player_obj ) {			
				if ( !Weapon_energy_cheat ) {
					float max_dist;

					max_dist = wip->lifetime * wip->max_speed;
					if (wip->wi_flags2 & WIF2_LOCAL_SSM){
						max_dist= wip->lssm_lock_range;
					}

					if ((aip->target_objnum != -1) && (vm_vec_dist_quick(&obj->pos, &Objects[aip->target_objnum].pos) > max_dist)) {
						HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Too far from target to acquire lock", 487));
					} else {
						char missile_name[NAME_LENGTH];
						strcpy_s(missile_name, wip->name);
						end_string_at_first_hash_symbol(missile_name);
						HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Cannot fire %s without a lock", 488), missile_name);
					}

					snd_play( &Snds[ship_get_sound(Player_obj, SND_OUT_OF_MISSLES)] );
					swp->next_secondary_fire_stamp[bank] = timestamp(800);	// to avoid repeating messages
					return 0;
				}
			} else {
				// multiplayer clients should always fire the weapon here, so return only if not
				// a multiplayer client.
				if ( !MULTIPLAYER_CLIENT ) {
					return 0;
				}
			}
		}
	}

	if (wip->wi_flags2 & WIF2_TAGGED_ONLY)
	{
		if (!ship_is_tagged(&Objects[aip->target_objnum]))
		{
			if (obj==Player_obj)
			{
				if ( !Weapon_energy_cheat )
				{
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, NOX("Cannot fire %s if target is not tagged"),wip->name);
					snd_play( &Snds[ship_get_sound(Player_obj, SND_OUT_OF_MISSLES)] );
					swp->next_secondary_fire_stamp[bank] = timestamp(800);	// to avoid repeating messages
					return 0;
				}
			}
			else
			{
				if ( !MULTIPLAYER_CLIENT )
				{
					return 0;
				}
			}
		}
	}



	// if trying to fire a swarm missile, make sure being called from right place
	if ( (wip->wi_flags & WIF_SWARM) && !allow_swarm ) {
		Assert(wip->swarm_count > 0);
		if(wip->swarm_count <= 0){
			shipp->num_swarm_missiles_to_fire = SWARM_DEFAULT_NUM_MISSILES_FIRED;
		} else {
			shipp->num_swarm_missiles_to_fire = wip->swarm_count;
		}
		shipp->swarm_missile_bank = bank;
		return 1;		//	Note: Missiles didn't get fired, but the frame interval code will fire them.
	}

	// if trying to fire a corkscrew missile, make sure being called from right place	
	if ( (wip->wi_flags & WIF_CORKSCREW) && !allow_swarm ) {
		//phreak 11-9-02 
		//changed this from 4 to custom number defined in tables
		shipp->num_corkscrew_to_fire = (ubyte)(shipp->num_corkscrew_to_fire + (ubyte)wip->cs_num_fired);
		shipp->corkscrew_missile_bank = bank;
		return 1;		//	Note: Missiles didn't get fired, but the frame interval code will fire them.
	}	

	float t;

	if (Weapon_info[weapon_idx].burst_shots > swp->burst_counter[bank_adjusted]) {
		t = Weapon_info[weapon_idx].burst_delay;
		swp->burst_counter[bank_adjusted]++;
	} else {
		t = Weapon_info[weapon_idx].fire_wait;	// They can fire 5 times a second
		swp->burst_counter[bank_adjusted] = 0;
	}
	swp->next_secondary_fire_stamp[bank] = timestamp((int) (t * 1000.0f));
	swp->last_secondary_fire_stamp[bank] = timestamp();

	// Here is where we check if weapons subsystem is capable of firing the weapon.
	// do only in single player or if I am the server of a multiplayer game
	if ( !(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER ) {
		if ( ship_weapon_maybe_fail(shipp) ) {
			if ( obj == Player_obj ) 
				if ( ship_maybe_play_secondary_fail_sound(wip) ) {
					char missile_name[NAME_LENGTH];
					strcpy_s(missile_name, Weapon_info[weapon_idx].name);
					end_string_at_first_hash_symbol(missile_name);
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Cannot fire %s due to weapons system damage", 489), missile_name);
				}
			goto done_secondary;
		}
	}

	pm = model_get( sip->model_num );
	if ( pm->n_missiles > 0 ) {
		int check_ammo;		// used to tell if we should check ammo counts or not
		int num_slots;

		if ( bank > pm->n_missiles ) {
			nprintf(("WARNING","WARNING ==> Tried to fire bank %d, but ship has only %d banks\n", bank+1, pm->n_missiles));
			return 0;		// we can make a quick out here!!!
		}

		num_slots = pm->missile_banks[bank].num_slots;

		// determine if there is enough ammo left to fire weapons on this bank.  As with primary
		// weapons, we might or might not check ammo counts depending on game mode, who is firing,
		// and if I am a client in multiplayer
		check_ammo = 1;

		if ( MULTIPLAYER_CLIENT && (obj != Player_obj) ){
			check_ammo = 0;
		}

		if ( check_ammo && ( swp->secondary_bank_ammo[bank] <= 0) ) {
			if ( shipp->objnum == OBJ_INDEX(Player_obj) ) {
				if ( ship_maybe_play_secondary_fail_sound(wip) ) {
//					HUD_sourced_printf(HUD_SOURCE_HIDDEN, "No %s missiles left in bank", Weapon_info[swp->secondary_bank_weapons[bank]].name);
				}
			}
			else {
				// TODO:  AI switch secondary weapon / re-arm?
			}
			goto done_secondary;
		}

		int start_slot, end_slot;

		if ( shipp->flags & SF_SECONDARY_DUAL_FIRE && num_slots > 1) {
			start_slot = swp->secondary_next_slot[bank];
			// AL 11-19-97: Ensure enough ammo remains when firing linked secondary weapons
			if ( check_ammo && (swp->secondary_bank_ammo[bank] < 2) ) {
				end_slot = start_slot;
			} else {
				end_slot = start_slot+1;
			}
		} else {
			// de-set the flag just in case dual-fire was set but couldn't be used
			// because there's less than two firepoints
			shipp->flags &= ~SF_SECONDARY_DUAL_FIRE;
			start_slot = swp->secondary_next_slot[bank];
			end_slot = start_slot;
		}

		int pnt_index=start_slot;
		//If this is a tertiary weapon, only subtract one piece of ammo
		for ( j = start_slot; j <= end_slot; j++ ) {
			int	weapon_num;

			swp->secondary_next_slot[bank]++;
			if ( swp->secondary_next_slot[bank] > (num_slots-1) ){
				swp->secondary_next_slot[bank] = 0;
			}

			if ( pnt_index >= num_slots ){
				pnt_index = 0;
			}
			shipp->secondary_point_reload_pct[bank][pnt_index] = 0.0f;
			pnt = pm->missile_banks[bank].pnt[pnt_index++];

			polymodel *weapon_model = NULL;
			if(wip->external_model_num >= 0){
				weapon_model = model_get(wip->external_model_num);
			}

			if (weapon_model && weapon_model->n_guns) {
				int external_bank = bank + MAX_SHIP_PRIMARY_BANKS;
				if (wip->wi_flags2 & WIF2_EXTERNAL_WEAPON_FP) {
					if ((weapon_model->n_guns <= swp->external_model_fp_counter[external_bank]) || (swp->external_model_fp_counter[external_bank] < 0))
						swp->external_model_fp_counter[external_bank] = 0;
					vm_vec_add2(&pnt, &weapon_model->gun_banks[0].pnt[swp->external_model_fp_counter[external_bank]]);
					swp->external_model_fp_counter[external_bank]++;
				} else {
					// make it use the 0 index slot
					vm_vec_add2(&pnt, &weapon_model->gun_banks[0].pnt[0]);
				}
			}
			vm_vec_unrotate(&missile_point, &pnt, &obj->orient);
			vm_vec_add(&firing_pos, &missile_point, &obj->pos);

			if ( Game_mode & GM_MULTIPLAYER ) {
				Assert( Weapon_info[weapon_idx].subtype == WP_MISSILE );
			}

			matrix firing_orient;
			if(!(sip->flags2 & SIF2_GUN_CONVERGENCE))
			{
				firing_orient = obj->orient;
			}
			else
			{
				vec3d firing_vec;
				vm_vec_unrotate(&firing_vec, &pm->missile_banks[bank].norm[pnt_index-1], &obj->orient);
				vm_vector_2_matrix(&firing_orient, &firing_vec, NULL, NULL);
			}

			// create the weapon -- for multiplayer, the net_signature is assigned inside
			// of weapon_create
			weapon_num = weapon_create( &firing_pos, &firing_orient, weapon_idx, OBJ_INDEX(obj), -1, aip->current_target_is_locked);

			if (weapon_num >= 0) {
				weapon_idx = Weapons[Objects[weapon_num].instance].weapon_info_index;
				weapon_set_tracking_info(weapon_num, OBJ_INDEX(obj), aip->target_objnum, aip->current_target_is_locked, aip->targeted_subsys);
				has_fired = true;

				// create the muzzle flash effect
				if ( (obj != Player_obj) || (sip->flags2 & SIF2_SHOW_SHIP_MODEL) || (Viewer_mode) ) {
					// show the flash only if in not cockpit view, or if "show ship" flag is set
					shipfx_flash_create(obj, sip->model_num, &pnt, &obj->orient.vec.fvec, 0, weapon_idx);
				}

				if((wip->wi_flags & WIF_SHUDDER) && (obj == Player_obj) && !(Game_mode & GM_STANDALONE_SERVER)){
					// calculate some arbitrary value between 100
					// (mass * velocity) / 10
					game_shudder_apply(500, (wip->mass * wip->max_speed) * 0.1f);
				}

				num_fired++;
				swp->last_fired_weapon_index = weapon_num;
				swp->detonate_weapon_time = timestamp(500);		//	Can detonate 1/2 second later.
				swp->last_fired_weapon_signature = Objects[weapon_num].signature;

				// subtract the number of missiles fired
				if ( Weapon_energy_cheat == 0 ){
					swp->secondary_bank_ammo[bank]--;
				}
			}
		}
	}

	if ( obj == Player_obj ) {
		if ( Weapon_info[weapon_idx].launch_snd != -1 ) {
			snd_play( &Snds[Weapon_info[weapon_idx].launch_snd], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY );
			swp = &Player_ship->weapons;
			if (bank >= 0) {
				wip = &Weapon_info[swp->secondary_bank_weapons[bank]];
				if (Player_ship->flags & SF_SECONDARY_DUAL_FIRE){
					joy_ff_play_secondary_shoot((int) (wip->cargo_size * 2.0f));
				} else {
					joy_ff_play_secondary_shoot((int) wip->cargo_size);
				}
			}
		}

	} else {
		if ( Weapon_info[weapon_idx].launch_snd != -1 ) {
			snd_play_3d( &Snds[Weapon_info[weapon_idx].launch_snd], &obj->pos, &View_position );
		}
	}

done_secondary:

	if(num_fired > 0){
		// if I am the master of a multiplayer game, send a secondary fired packet along with the
		// first network signatures for the newly created weapons.  if nothing got fired, send a failed
		// packet if 
		if ( MULTIPLAYER_MASTER ) {			
			Assert(starting_sig != 0);
			send_secondary_fired_packet( shipp, starting_sig, starting_bank_count, num_fired, allow_swarm );			
		}

		// STATS
		if (obj->flags & OF_PLAYER_SHIP) {
			// in multiplayer -- only the server needs to keep track of the stats.  Call the cool
			// function to find the player given the object *.  It had better return a valid player
			// or our internal structure as messed up.
			if( Game_mode & GM_MULTIPLAYER ) {
				if ( Net_player->flags & NETINFO_FLAG_AM_MASTER ) {
					int player_num;

					player_num = multi_find_player_by_object ( obj );
					Assert ( player_num != -1 );

					Net_players[player_num].m_player->stats.ms_shots_fired += num_fired;
				}				
			} else {
				Player->stats.ms_shots_fired += num_fired;
			}
		}
	
		// maybe announce a shockwave weapon
		ai_maybe_announce_shockwave_weapon(obj, weapon_idx);
	}

	// if we are out of ammo in this bank then don't carry over firing swarm/corkscrew
	// missiles to a new bank
	if (swp->secondary_bank_ammo[bank] <= 0) {
		// NOTE: these are set to 1 since they will get reduced by 1 in the
		//       swarm/corkscrew code once this function returns

		if (shipp->num_swarm_missiles_to_fire > 1) {
			shipp->num_swarm_missiles_to_fire = 1;
			shipp->swarm_missile_bank = -1;
		}

		if (shipp->num_corkscrew_to_fire > 1) {
			shipp->num_corkscrew_to_fire = 1;
			shipp->corkscrew_missile_bank = -1;
		}
	}

	// AL 3-7-98: Move to next valid secondary bank if out of ammo
	//

	//21-07-02 01:24 DTP; COMMENTED OUT some of the mistakes
	//this bug was made by AL, when he assumed he had to take the next fire_wait time remaining and add 250 ms of delay to it, 
	//and put it in the next valid bank. for the player to have a 250 ms of penalty
	//
	//what that caused was that the next valid bank inherited the current valid banks FULL fire delay. since he used the Weapon_info struct that has
	// no information / member that stores the next valids banks remaning fire_wait delay.
	//
	//what he should have done was to check of the next valid bank had any fire delay that had elapsed, if it had elapsed, 
	//then it would have no firedelay. and then add 250 ms of delay. in effect, this way there is no penalty if there is any firedelay remaning in
	//the next valid bank. the delay is there to prevent things like Trible/Quad Fire Trebuchets.
	//
	// niffiwan: only try to switch banks if object has multiple banks, and firing bank is the current bank
	if ( (obj->flags & OF_PLAYER_SHIP) && (swp->secondary_bank_ammo[bank] <= 0) && (swp->num_secondary_banks >= 2) && (bank == swp->current_secondary_bank) ) {
		// niffiwan: call ship_select_next_secondary instead of ship_select_next_valid_secondary_bank
		// ensures all "extras" are dealt with, like animations, scripting hooks, etc
		if (ship_select_next_secondary(obj) ) {			//DTP here we switch to the next valid bank, but we can't call weapon_info on next fire_wait

			if ( timestamp_elapsed(shipp->weapons.next_secondary_fire_stamp[shipp->weapons.current_secondary_bank]) ) {	//DTP, this is simply a copy of the manual cycle functions
				shipp->weapons.next_secondary_fire_stamp[shipp->weapons.current_secondary_bank] = timestamp(1000);	//Bumped from 250 to 1000 because some people seem to be to triggerhappy :).
				shipp->weapons.last_secondary_fire_stamp[shipp->weapons.current_secondary_bank] = timestamp();
			}
						
			if ( obj == Player_obj ) {
				snd_play( &Snds[ship_get_sound(Player_obj, SND_SECONDARY_CYCLE)] );		
			}
		}
	}	

	if (has_fired) {
		object *objp = &Objects[shipp->objnum];
		object* target;
		if (Ai_info[shipp->ai_index].target_objnum != -1)
			target = &Objects[Ai_info[shipp->ai_index].target_objnum];
		else
			target = NULL;
		if (objp == Player_obj && Player_ai->target_objnum != -1)
			target = &Objects[Player_ai->target_objnum]; 
		Script_system.SetHookObjects(2, "User", objp, "Target", target);
		Script_system.RunCondition(CHA_ONWPFIRED, 0, NULL, objp);
		Script_system.RunCondition(CHA_SECONDARYFIRE, 0, NULL, objp);
		Script_system.RemHookVars(2, "User", "Target");
	}

	return num_fired;
}

// Goober5000
int primary_out_of_ammo(ship_weapon *swp, int bank)
{
	// true if both ballistic and ammo <= 0,
	// false if not ballistic or if ballistic and ammo > 0
			
	if ( Weapon_info[swp->primary_bank_weapons[bank]].wi_flags2 & WIF2_BALLISTIC )
	{
		if (swp->primary_bank_ammo[bank] <= 0)
		{
			return 1;
		}
	}

	// note: never out of ammo if not ballistic
	return 0;
}

/**
 * Return true if a new index gets selected.
 * 
 * @param objp      pointer to object for ship cycling primary
 * @param direction forward == CYCLE_PRIMARY_NEXT, backward == CYCLE_PRIMARY_PREV
 *
 * NOTE: This code can be called for any arbitrary ship.  HUD messages and sounds are only used
 *       for the player ship.
 */
int ship_select_next_primary(object *objp, int direction)
{
	ship	*shipp;
	ship_info *sip;
	ship_weapon *swp;
	int new_bank;
	int original_bank;
	unsigned int original_link_flag;
	int i;

	Assert(objp != NULL);
	Assert(objp->type == OBJ_SHIP);
	Assert(objp->instance >= 0 && objp->instance < MAX_SHIPS);

	shipp = &Ships[objp->instance];
	sip = &Ship_info[shipp->ship_info_index];
	swp = &shipp->weapons;

	Assert(direction == CYCLE_PRIMARY_NEXT || direction == CYCLE_PRIMARY_PREV);

	original_bank = swp->current_primary_bank;
	original_link_flag = shipp->flags & SF_PRIMARY_LINKED;

	// redid case structure to possibly add more primaries in the future - Goober5000
	if ( swp->num_primary_banks == 0 )
	{
		if ( objp == Player_obj )
		{
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "This ship has no primary weapons", 490));
			gamesnd_play_error_beep();
		}
		return 0;
	}
	else if ( swp->num_primary_banks == 1 )
	{
		if ( objp == Player_obj )
		{
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "This ship has only one primary weapon: %s", 491),Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]].name, swp->current_primary_bank + 1);
			gamesnd_play_error_beep();
		}
		return 0;
	}
	else if ( swp->num_primary_banks > MAX_SHIP_PRIMARY_BANKS )
	{
		Int3();
		return 0;
	}
	else
	{
		Assert((swp->current_primary_bank >= 0) && (swp->current_primary_bank < swp->num_primary_banks));

		// first check if linked
		if ( shipp->flags2 & SF2_SHIP_SELECTIVE_LINKING )
		{
			printf("npb:%i\n", swp->num_primary_banks );
		}

		if ( shipp->flags & SF_PRIMARY_LINKED )
		{
			shipp->flags &= ~SF_PRIMARY_LINKED;
			if ( direction == CYCLE_PRIMARY_NEXT )
			{
				swp->current_primary_bank = 0;
			}
			else
			{
				swp->current_primary_bank = swp->num_primary_banks - 1;
			}
		}
		// now handle when not linked: cycle and constrain
		else
		{
			if ( direction == CYCLE_PRIMARY_NEXT )
			{
				if ( swp->current_primary_bank < swp->num_primary_banks - 1 )
				{
					swp->current_primary_bank++;
				}
				else if( sip->flags2 & SIF2_NO_PRIMARY_LINKING )
				{
					swp->current_primary_bank = 0;
				}
				else
				{
					shipp->flags |= SF_PRIMARY_LINKED;
				}
			}
			else
			{
				if ( swp->current_primary_bank > 0 )
				{
					swp->current_primary_bank--;
				}
				else if( sip->flags2 & SIF2_NO_PRIMARY_LINKING )
				{
					swp->current_primary_bank = swp->num_primary_banks - 1;
				}
				else
				{
					shipp->flags |= SF_PRIMARY_LINKED;
				}
			}
		}
	}

	// test for ballistics - Goober5000
	if ( sip->flags & SIF_BALLISTIC_PRIMARIES )
	{
		// if we can't link, disengage primary linking and change to next available bank
		if (shipp->flags & SF_PRIMARY_LINKED)
		{
			for (i = 0; i < swp->num_primary_banks; i++)
			{
				if (primary_out_of_ammo(swp, i))
				{
					shipp->flags &= ~SF_PRIMARY_LINKED;
					
					if (direction == CYCLE_PRIMARY_NEXT)
					{
						swp->current_primary_bank = 0;
					}
					else
					{
						swp->current_primary_bank = shipp->weapons.num_primary_banks-1;
					}
					break;
				}
			}
		}

		// check to see if we keep cycling...we have to if we're out of ammo in the current bank
		if ( primary_out_of_ammo(swp, swp->current_primary_bank) )
		{
			// cycle around until we find ammunition...
			// we land on the original bank if all banks fail
			Assert(swp->current_primary_bank < swp->num_primary_banks);
			new_bank = swp->current_primary_bank;

			for (i = 1; i < swp->num_primary_banks; i++)
			{
				// cycle in the proper direction
				if ( direction == CYCLE_PRIMARY_NEXT )
				{
					new_bank = (swp->current_primary_bank + i) % swp->num_primary_banks;
				}
				else
				{
					new_bank = (swp->current_primary_bank + swp->num_primary_banks - i) % swp->num_primary_banks;
				}

				// check to see if this is a valid bank
				if (!primary_out_of_ammo(swp, new_bank))
				{
					break;
				}
			}
			// set the new bank; defaults to resetting to the old bank if we completed a full iteration
			swp->current_primary_bank = new_bank;
		}
		
		// make sure we're okay
		Assert((swp->current_primary_bank >= 0) && (swp->current_primary_bank < swp->num_primary_banks));

		if(swp->current_primary_bank != original_bank)
			swp->previous_primary_bank = original_bank;
		else
			swp->previous_primary_bank = swp->current_primary_bank;

		// if this ship is ballistics-equipped, and we cycled, then we had to verify some stuff,
		// so we should check if we actually changed banks
		if ( (swp->current_primary_bank != original_bank) || ((shipp->flags & SF_PRIMARY_LINKED) != original_link_flag) )
		{
			if ( objp == Player_obj )
			{
				snd_play( &Snds[ship_get_sound(objp, SND_PRIMARY_CYCLE)], 0.0f );
			}
			ship_primary_changed(shipp);
			objp = &Objects[shipp->objnum];
			object* target;
			if (Ai_info[shipp->ai_index].target_objnum != -1)
				target = &Objects[Ai_info[shipp->ai_index].target_objnum];
			else
				target = NULL;
			if (objp == Player_obj && Player_ai->target_objnum != -1)
				target = &Objects[Player_ai->target_objnum]; 
			Script_system.SetHookObjects(2, "User", objp, "Target", target);
			Script_system.RunCondition(CHA_ONWPSELECTED, 0, NULL, objp);
			Script_system.SetHookObjects(2, "User", objp, "Target", target);
			Script_system.RunCondition(CHA_ONWPDESELECTED, 0, NULL, objp);
			Script_system.RemHookVars(2, "User", "Target");
			return 1;
		}

		// could not select new weapon:
		if ( objp == Player_obj )
		{
			gamesnd_play_error_beep();
		}
		return 0;
	}	// end of ballistics implementation

	if ( objp == Player_obj )
	{
		snd_play( &Snds[ship_get_sound(objp, SND_PRIMARY_CYCLE)], 0.0f );
	}

	ship_primary_changed(shipp);
	object* target;
	if (Ai_info[shipp->ai_index].target_objnum != -1)
		target = &Objects[Ai_info[shipp->ai_index].target_objnum];
	else
		target = NULL;
	if (objp == Player_obj && Player_ai->target_objnum != -1)
		target = &Objects[Player_ai->target_objnum]; 
	Script_system.SetHookObjects(2, "User", objp, "Target", target);
	Script_system.RunCondition(CHA_ONWPSELECTED, 0, NULL, objp);
	Script_system.SetHookObjects(2, "User", objp, "Target", target);
	Script_system.RunCondition(CHA_ONWPDESELECTED, 0, NULL, objp);
	Script_system.RemHookVars(2, "User", "Target");

	return 1;
}

// ------------------------------------------------------------------------------
// ship_select_next_secondary() selects the next secondary bank with missles
//
//	returns:		1	=> The secondary bank was switched
//					0	=> The secondary bank stayed the same
//
// If a secondary bank has no missles left, it is skipped.
//
// NOTE: This can be called for an arbitrary ship.  HUD messages and sounds are only used
//			for the player ship.
int ship_select_next_secondary(object *objp)
{
	Assert(objp != NULL);
	Assert(objp->type == OBJ_SHIP);
	Assert(objp->instance >= 0 && objp->instance < MAX_SHIPS);

	int	original_bank, new_bank, i;
	ship	*shipp;
	ship_weapon *swp;

	shipp = &Ships[objp->instance];
	swp = &shipp->weapons;

	// redid the switch structure to allow additional seconary banks if added later - Goober5000
	if ( swp->num_secondary_banks == 0 )
	{
		if ( objp == Player_obj )
		{
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "This ship has no secondary weapons", 492));
			gamesnd_play_error_beep();
		}
		return 0;
	}
	else if ( swp->num_secondary_banks == 1 )
	{
		if ( objp == Player_obj )
		{
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "This ship has only one secondary weapon: %s", 493), Weapon_info[swp->secondary_bank_weapons[swp->current_secondary_bank]].name, swp->current_secondary_bank + 1);
			gamesnd_play_error_beep();
		}
		return 0;
	}
	else if ( swp->num_secondary_banks > MAX_SHIP_SECONDARY_BANKS )
	{
		Int3();
		return 0;
	}
	else
	{
		Assert((swp->current_secondary_bank >= 0) && (swp->current_secondary_bank < swp->num_secondary_banks));

		original_bank = swp->current_secondary_bank;

		for ( i = 1; i < swp->num_secondary_banks; i++ ) {
			new_bank = (swp->current_secondary_bank+i) % swp->num_secondary_banks;
			if ( swp->secondary_bank_ammo[new_bank] <= 0 )
				continue;
			swp->current_secondary_bank = new_bank;
			break;
		}

		if ( swp->current_secondary_bank != original_bank )
		{
			if(swp->current_primary_bank != original_bank)
				swp->previous_primary_bank = original_bank;
			else
				swp->previous_primary_bank = swp->current_primary_bank;
			if ( objp == Player_obj )
			{
				snd_play( &Snds[ship_get_sound(Player_obj, SND_SECONDARY_CYCLE)], 0.0f );
			}
			ship_secondary_changed(shipp);

			objp = &Objects[shipp->objnum];
			object* target;
			if (Ai_info[shipp->ai_index].target_objnum != -1)
				target = &Objects[Ai_info[shipp->ai_index].target_objnum];
			else
				target = NULL;
			if (objp == Player_obj && Player_ai->target_objnum != -1)
				target = &Objects[Player_ai->target_objnum]; 
			Script_system.SetHookObjects(2, "User", objp, "Target", target);
			Script_system.RunCondition(CHA_ONWPSELECTED, 0, NULL, objp);
			Script_system.SetHookObjects(2, "User", objp, "Target", target);
			Script_system.RunCondition(CHA_ONWPDESELECTED, 0, NULL, objp);
			Script_system.RemHookVars(2, "User", "Target");
			return 1;
		}
	} // end if

	// If we've reached this point, must have failed
	if ( objp == Player_obj )
	{
		gamesnd_play_error_beep();
	}
	return 0;
}

// Goober5000 - copied from secondary routine
//	Stuff list of weapon indices for object *objp in list *outlist.
//	Return number of weapons in list.
int get_available_primary_weapons(object *objp, int *outlist, int *outbanklist)
{
	int	count = 0;
	int	i;
	ship	*shipp;

	Assert(objp->type == OBJ_SHIP);
	Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));
	shipp = &Ships[objp->instance];

	for (i=0; i<shipp->weapons.num_primary_banks; i++)
	{
		if (!primary_out_of_ammo(&(shipp->weapons), i))
		{
			outbanklist[count] = i;
			outlist[count++] = shipp->weapons.primary_bank_weapons[i];
		}
	}

	return count;
}

/**
 * Stuff list of weapon indices for object *objp in list *outlist.
 * @return number of weapons in list.
 */
int get_available_secondary_weapons(object *objp, int *outlist, int *outbanklist)
{
	int	count = 0;
	int	i;
	ship	*shipp;

	Assert(objp->type == OBJ_SHIP);
	Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));
	shipp = &Ships[objp->instance];

	for (i=0; i<shipp->weapons.num_secondary_banks; i++)
		if (shipp->weapons.secondary_bank_ammo[i]) {
			outbanklist[count] = i;
			outlist[count++] = shipp->weapons.secondary_bank_weapons[i];
		}

	return count;
}

void wing_bash_ship_name(char *ship_name, const char *wing_name, int index)
{
	// if wing name has a hash symbol, create the ship name a particular way
	// (but don't do this for names that have the hash as the first or last character)
	const char *p = get_pointer_to_first_hash_symbol(wing_name);
	if ((p != NULL) && (p != wing_name) && (*(p+1) != '\0'))
	{
		size_t len = (p - wing_name);
		strncpy(ship_name, wing_name, len);
		sprintf(ship_name + len, NOX(" %d"), index);
		strcat(ship_name, p);
	}
	// most of the time we should create the name the standard retail way
	else
		sprintf(ship_name, NOX("%s %d"), wing_name, index);
}

/**
 * Return the object index of the ship with name *name.
 */
int wing_name_lookup(const char *name, int ignore_count)
{
	int i, wing_limit;

	if (name == NULL)
		return -1;

	if ( Fred_running )
		wing_limit = MAX_WINGS;
	else
		wing_limit = Num_wings;

	if (Fred_running || ignore_count ) {  // current_count not used for Fred..
		for (i=0; i<wing_limit; i++)
			if (Wings[i].wave_count && !stricmp(Wings[i].name, name))
				return i;

	} else {
		for (i=0; i<wing_limit; i++)
			if (Wings[i].current_count && !stricmp(Wings[i].name, name))
				return i;
	}

	return -1;
}

/**
 * Needed in addition to wing_name_lookup because it does a straight lookup without
 * caring about how many ships are in the wing, etc.
 */
int wing_lookup(const char *name)
{
   int idx;
	for(idx=0;idx<Num_wings;idx++)
		if(stricmp(Wings[idx].name,name)==0)
		   return idx;

	return -1;
}

/**
 * Return the index of Ship_info[].name that is *token.
 */
int ship_info_lookup_sub(const char *token)
{
	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it)
		if (!stricmp(token, it->name))
			return std::distance(Ship_info.cbegin(), it);

	return -1;
}

/**
 * Return the index of Ship_templates[].name that is *token.
 */
int ship_template_lookup(const char *token)
{
	for ( auto it = Ship_templates.cbegin(); it != Ship_templates.cend(); ++it ) {
		if ( !stricmp(token, it->name) ) {
			return std::distance(Ship_templates.cbegin(), it);
		}
	}
	return -1;
}

// Goober5000
int ship_info_lookup(const char *token)
{
	int idx;
	const char *p;
	char name[NAME_LENGTH], temp1[NAME_LENGTH], temp2[NAME_LENGTH];

	// bogus
	if (token == NULL)
		return -1;

	// first try a straightforward lookup
	idx = ship_info_lookup_sub(token);
	if (idx >= 0)
		return idx;

	// ship copy types might be mismatched
	p = get_pointer_to_first_hash_symbol(token);
	if (p == NULL)
		return -1;

	// conversion from FS1 missions
	if (!stricmp(token, "GTD Orion#1 (Galatea)"))
	{
		idx = ship_info_lookup_sub("GTD Orion#Galatea");
		if (idx >= 0)
			return idx;

		idx = ship_info_lookup_sub("GTD Orion (Galatea)");
		if (idx >= 0)
			return idx;

		return -1;
	}
	else if (!stricmp(token, "GTD Orion#2 (Bastion)"))
	{
		idx = ship_info_lookup_sub("GTD Orion#Bastion");
		if (idx >= 0)
			return idx;

		idx = ship_info_lookup_sub("GTD Orion (Bastion)");
		if (idx >= 0)
			return idx;

		return -1;
	}
	else if (!stricmp(token, "SF Dragon#2 (weakened)"))
	{
		idx = ship_info_lookup_sub("SF Dragon#weakened");
		if (idx >= 0)
			return idx;

		idx = ship_info_lookup_sub("SF Dragon (weakened)");
		if (idx >= 0)
			return idx;

		return -1;
	}
	else if (!stricmp(token, "SF Dragon#3 (Player)"))
	{
		idx = ship_info_lookup_sub("SF Dragon#Terrans");
		if (idx >= 0)
			return idx;

		idx = ship_info_lookup_sub("SF Dragon (Terrans)");
		if (idx >= 0)
			return idx;

		return -1;
	}
	else if (!stricmp(token, "GTSC Faustus#2 (big blast)"))
	{
		idx = ship_info_lookup_sub("GTSC Faustus#bigblast");
		if (idx >= 0)
			return idx;

		return -1;
	}
	else if (!stricmp(token, "GTF Loki (stealth)"))
	{
		idx = ship_info_lookup_sub("GTF Loki#stealth");
		if (idx >= 0)
			return idx;

		return -1;
	}

	// get first part of new string
	strcpy_s(temp1, token);
	end_string_at_first_hash_symbol(temp1);

	// get second part
	strcpy_s(temp2, p + 1);

	// found a hash
	if (*p == '#')
	{
		if (strlen(token) > NAME_LENGTH-3) {
			// If the below sprintf would exceed NAME_LENGTH (taking \0 terminator into account), give a warning and return.
			Warning(LOCATION, "Token [%s] is too long to be parenthesized by ship_info_lookup()!\n", token);
			return -1;
		}
		// assemble using parentheses
		sprintf(name, "%s (%s)", temp1, temp2);
	}
	// found a parenthesis
	else if (*p == '(')
	{
		// chop off right parenthesis (it exists because otherwise the left wouldn't have been flagged)
		char *p2 = strchr(temp2, ')');
		*p2 = '\0';

		// assemble using hash
		sprintf(name, "%s#%s", temp1, temp2);
	}
	// oops
	else
	{
		Warning(LOCATION, "Unrecognized hash symbol.  Contact a programmer!");
		return -1;
	}

	// finally check the new name
	return ship_info_lookup_sub(name);
}

/**
 * Return the ship index of the ship with name *name.
 */
int ship_name_lookup(const char *name, int inc_players)
{
	int	i;

	// bogus
	if(name == NULL){
		return -1;
	}

	for (i=0; i<MAX_SHIPS; i++){
		if (Ships[i].objnum >= 0){
			if (Objects[Ships[i].objnum].type == OBJ_SHIP || (Objects[Ships[i].objnum].type == OBJ_START && inc_players)){
				if (!stricmp(name, Ships[i].ship_name)){
					return i;
				}
			}
		}
	}
	
	// couldn't find it
	return -1;
}

int ship_type_name_lookup(const char *name)
{
	// bogus
	if(name == NULL || !strlen(name)){
		return -1;
	}

	//Look through Ship_types array
	uint max_size = Ship_types.size();
	for(uint idx=0; idx < max_size; idx++){
		if(!stricmp(name, Ship_types[idx].name)){
			return idx;
		}
	}
	// couldn't find it
	return -1;
}

// checks the (arrival & departure) state of a ship.  Return values:
// -1: has yet to arrive in mission
//  0: is currently in mission
//  1: has been destroyed, departed, or never existed
int ship_query_state(char *name)
{
	int i;

	// bogus
	Assert(name != NULL);
	if(name == NULL){
		return -1;
	}

	for (i=0; i<MAX_SHIPS; i++){
		if (Ships[i].objnum >= 0){
			if ((Objects[Ships[i].objnum].type == OBJ_SHIP) || (Objects[Ships[i].objnum].type == OBJ_START)){
				if (!stricmp(name, Ships[i].ship_name)){
					return 0;
				}
			}
		}
	}

	if (mission_parse_get_arrival_ship(name))
		return -1;

	return 1;
}

// Finds the world position of a subsystem.
// Return true/false for subsystem found/not found.
// Stuff vector *pos with absolute position.
// subsysp is a pointer to the subsystem.
int get_subsystem_pos(vec3d *pos, object *objp, ship_subsys *subsysp)
{
	if (subsysp == NULL) {
		*pos = objp->pos;
		return 0;
	}

	model_subsystem *mss = subsysp->system_info;

	if (mss->subobj_num == -1) {
		// If it's a special point subsys, we can use its offset directly

		vm_vec_unrotate(pos, &subsysp->system_info->pnt, &objp->orient);
		vm_vec_add2(pos, &objp->pos);
	} else {
		// Submodel subsystems may require a more complicated calculation

		find_submodel_instance_world_point(pos, objp, mss->subobj_num);
	}

	return 1;
}

//=================================================
// Takes all the angle info from the ship structure and stuffs it
// into the model data so that the model code has all the correct
// angles and stuff that it needs.    This is a poorly designed 
// system that should be re-engineered so that all the model functions
// accept a list of angles and everyone passes them through, but
// that would require some major code revision.
// So, anytime you are using a model that has rotating parts, you
// need to do a ship_model_start before any model_ functions are
// called and a ship_model_stop after you're done.   Even for 
// collision detection and stuff, not just rendering.
// See John for details.

void ship_model_start(object *objp)
{
	model_subsystem	*psub;
	ship		*shipp;
	ship_subsys	*pss;
	int model_num;

	shipp = &Ships[objp->instance];
	model_num = Ship_info[shipp->ship_info_index].model_num;

	// First clear all the angles in the model to zero
	model_clear_instance(model_num);

	// Go through all subsystems and bash the model angles for all 
	// the subsystems that need it.
	for ( pss = GET_FIRST(&shipp->subsys_list); pss != END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;
		switch (psub->type) {
			case SUBSYSTEM_RADAR:
			case SUBSYSTEM_NAVIGATION:
			case SUBSYSTEM_COMMUNICATION:
			case SUBSYSTEM_UNKNOWN:
			case SUBSYSTEM_ENGINE:
			case SUBSYSTEM_SENSORS:
			case SUBSYSTEM_WEAPONS:
			case SUBSYSTEM_SOLAR:
			case SUBSYSTEM_GAS_COLLECT:
			case SUBSYSTEM_ACTIVATION:
			case SUBSYSTEM_TURRET:
				break;
			default:
				Error(LOCATION, "Illegal subsystem type.\n");
		}

		if ( psub->subobj_num >= 0 )	{
			model_set_instance(model_num, psub->subobj_num, &pss->submodel_info_1, pss->flags );
		}

		if ( (psub->subobj_num != psub->turret_gun_sobj) && (psub->turret_gun_sobj >= 0) )		{
			model_set_instance(model_num, psub->turret_gun_sobj, &pss->submodel_info_2, pss->flags );
		}
	}
	model_do_dumb_rotation(model_num);
}

/**
 * Clears all the instance specific stuff out of the model info
 */
void ship_model_stop(object *objp)
{
	Assert(objp != NULL);
	Assert(objp->instance >= 0);
	Assert(objp->type == OBJ_SHIP);

	// Then, clear all the angles in the model to zero
	model_clear_instance(Ship_info[Ships[objp->instance].ship_info_index].model_num);
}

/**
 * Like ship_model_start but fills submodel instances instead of the submodels themselves
 */
void ship_model_update_instance(object *objp)
{
	model_subsystem	*psub;
	ship		*shipp;
	ship_subsys	*pss;
	int model_instance_num;

	Assert(objp != NULL);
	Assert(objp->instance >= 0);
	Assert(objp->type == OBJ_SHIP);

	shipp = &Ships[objp->instance];
	model_instance_num = shipp->model_instance_num;

	// Then, clear all the angles in the model to zero
	model_clear_submodel_instances(model_instance_num);

	for ( pss = GET_FIRST(&shipp->subsys_list); pss != END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;
		switch (psub->type) {
			case SUBSYSTEM_RADAR:
			case SUBSYSTEM_NAVIGATION:
			case SUBSYSTEM_COMMUNICATION:
			case SUBSYSTEM_UNKNOWN:
			case SUBSYSTEM_ENGINE:
			case SUBSYSTEM_SENSORS:
			case SUBSYSTEM_WEAPONS:
			case SUBSYSTEM_SOLAR:
			case SUBSYSTEM_GAS_COLLECT:
			case SUBSYSTEM_ACTIVATION:
			case SUBSYSTEM_TURRET:
				break;
			default:
				Error(LOCATION, "Illegal subsystem type.\n");
		}

		if ( psub->subobj_num >= 0 )	{
			model_update_instance(model_instance_num, psub->subobj_num, &pss->submodel_info_1, pss->flags );
		}

		if ( (psub->subobj_num != psub->turret_gun_sobj) && (psub->turret_gun_sobj >= 0) )		{
			model_update_instance(model_instance_num, psub->turret_gun_sobj, &pss->submodel_info_2, pss->flags );
		}
	}

	model_instance_dumb_rotation(model_instance_num);


	// preprocess subobject orientations for collision detection
	model_collide_preprocess(&objp->orient, model_instance_num);
}

/**
 * Finds the number of crew points in a ship
 */
int ship_find_num_crewpoints(object *objp)
{
	int n = 0;
	model_subsystem	*psub;
	ship		*shipp;
	ship_subsys	*pss;

	shipp = &Ships[objp->instance];

	// Go through all subsystems and record the model angles for all 
	// the subsystems that need it.
	for ( pss = GET_FIRST(&shipp->subsys_list); pss != END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;
		switch (psub->type) {
		case SUBSYSTEM_TURRET:
			if ( psub->flags & MSS_FLAG_CREWPOINT )
				n++; // fall through

		case SUBSYSTEM_RADAR:
		case SUBSYSTEM_NAVIGATION:
		case SUBSYSTEM_COMMUNICATION:
		case SUBSYSTEM_UNKNOWN:
		case SUBSYSTEM_ENGINE:
		case SUBSYSTEM_GAS_COLLECT:
		case SUBSYSTEM_ACTIVATION:
			break;
		default:
			Error(LOCATION, "Illegal subsystem type.\n");
		}
	}
	return n;
}

/**
 * Finds the number of turrets in a ship
 */
int ship_find_num_turrets(object *objp)
{
	int n = 0;
	model_subsystem	*psub;
	ship		*shipp;
	ship_subsys	*pss;

	shipp = &Ships[objp->instance];

	// Go through all subsystems and record the model angles for all 
	// the subsystems that need it.
	for ( pss = GET_FIRST(&shipp->subsys_list); pss != END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;
		switch (psub->type) {
		case SUBSYSTEM_TURRET:
			n++; // drop through

		case SUBSYSTEM_RADAR:
		case SUBSYSTEM_NAVIGATION:
		case SUBSYSTEM_COMMUNICATION:
		case SUBSYSTEM_UNKNOWN:
		case SUBSYSTEM_ENGINE:
		case SUBSYSTEM_GAS_COLLECT:
		case SUBSYSTEM_ACTIVATION:
			break;
		default:
			Error(LOCATION, "Illegal subsystem type.\n");
		}
	}
	return n;
}

//WMC
void ship_set_eye( object *obj, int eye_index)
{
	if(obj->type != OBJ_SHIP)
		return;

	ship *shipp = &Ships[obj->instance];

	if(eye_index < 0)
	{
		shipp->current_viewpoint = -1;
		return;
	}

	ship_info *sip = &Ship_info[shipp->ship_info_index];
	if(sip->model_num < 0)
		return;

	polymodel *pm = model_get(sip->model_num);

	if(pm == NULL || eye_index > pm->n_view_positions)
		return;

	shipp->current_viewpoint = eye_index;
}

// calculates the eye position for this ship in the global reference frame.  Uses the
// view_positions array in the model.  The 0th element is the normal viewing position.
// the vector of the eye is returned in the parameter 'eye'.  The orientation of the
// eye is returned in orient.  (NOTE: this is kind of bogus for now since non 0th element
// eyes have no defined up vector)
void ship_get_eye( vec3d *eye_pos, matrix *eye_orient, object *obj, bool do_slew , bool from_origin)
{
	ship *shipp = &Ships[obj->instance];
	polymodel *pm = model_get(Ship_info[shipp->ship_info_index].model_num);

	// check to be sure that we have a view eye to look at.....spit out nasty debug message
	if ( shipp->current_viewpoint < 0 || pm->n_view_positions == 0 || shipp->current_viewpoint > pm->n_view_positions) {
		*eye_pos = obj->pos;
		*eye_orient = obj->orient;
		return;
	}

	// eye points are stored in an array -- the normal viewing position for a ship is the current_eye_index
	// element.
	eye *ep = &(pm->view_positions[Ships[obj->instance].current_viewpoint]);

	if (ep->parent >= 0 && pm->submodel[ep->parent].can_move) {
		find_submodel_instance_point_orient(eye_pos, eye_orient, obj, ep->parent, &ep->pnt, &vmd_identity_matrix);
		vec3d tvec = *eye_pos;
		vm_vec_unrotate(eye_pos, &tvec, &obj->orient);
		vm_vec_add2(eye_pos, &obj->pos);

		matrix tempmat = *eye_orient;
		vm_matrix_x_matrix(eye_orient, &obj->orient, &tempmat);
	} else {
		model_find_world_point( eye_pos, &ep->pnt, pm->id, ep->parent, &obj->orient, from_origin ? &vmd_zero_vector : &obj->pos );
		*eye_orient = obj->orient;
	}

	//	Modify the orientation based on head orientation.
	if ( Viewer_obj == obj && do_slew) {
		// Add the cockpit leaning translation offset
		vm_vec_add2(eye_pos,&leaning_position);
		compute_slew_matrix(eye_orient, &Viewer_slew_angles);
	}
}

// of attackers to make this decision.
//
// NOTE: This function takes into account how many ships are attacking a subsystem, and will 
//			prefer an ignored subsystem over a subsystem that is in line of sight, if the in-sight
//			subsystem is attacked by more than MAX_SUBSYS_ATTACKERS
// input:
//				sp					=>		ship pointer to parent of subsystem
//				subsys_type		=>		what kind of subsystem this is
//				attacker_pos	=>		the world coords of the attacker of this subsystem
//
// returns: pointer to subsystem if one found, NULL otherwise
#define MAX_SUBSYS_ATTACKERS 3
ship_subsys *ship_get_best_subsys_to_attack(ship *sp, int subsys_type, vec3d *attacker_pos)
{
	ship_subsys	*ss;
	ship_subsys *best_in_sight_subsys, *lowest_attacker_subsys, *ss_return;
	int			lowest_num_attackers, lowest_in_sight_attackers, num_attackers;
	vec3d		gsubpos;
	ship_obj		*sop;

	lowest_in_sight_attackers = lowest_num_attackers = 1000;
	ss_return = best_in_sight_subsys = lowest_attacker_subsys = NULL;

	for (ss = GET_FIRST(&sp->subsys_list); ss != END_OF_LIST(&sp->subsys_list); ss = GET_NEXT(ss) ) {
		if ( (ss->system_info->type == subsys_type) && (ss->current_hits > 0) ) {

			// get world pos of subsystem
			vm_vec_unrotate(&gsubpos, &ss->system_info->pnt, &Objects[sp->objnum].orient);
			vm_vec_add2(&gsubpos, &Objects[sp->objnum].pos);
			
			// now find the number of ships attacking this subsystem by iterating through the ships list,
			// and checking if aip->targeted_subsys matches the subsystem we're checking
			num_attackers = 0;
			sop = GET_FIRST(&Ship_obj_list);
			while(sop != END_OF_LIST(&Ship_obj_list)){
				if ( Ai_info[Ships[Objects[sop->objnum].instance].ai_index].targeted_subsys == ss ) {
					num_attackers++;
				}
				sop = GET_NEXT(sop);
			}

			if ( num_attackers < lowest_num_attackers ) {
				lowest_num_attackers = num_attackers;
				lowest_attacker_subsys = ss;
			}

			if ( ship_subsystem_in_sight(&Objects[sp->objnum], ss, attacker_pos, &gsubpos) ) {
				if ( num_attackers < lowest_in_sight_attackers ) {
					lowest_in_sight_attackers = num_attackers;
					best_in_sight_subsys = ss;
				}
			}
		}
	}

	if ( best_in_sight_subsys == NULL ) {
		// no subsystems are in sight, so return the subsystem with the lowest # of attackers
		ss_return =  lowest_attacker_subsys;
	} else {
		if ( lowest_in_sight_attackers > MAX_SUBSYS_ATTACKERS ) {
			ss_return = lowest_attacker_subsys;
		} else {
			ss_return =  best_in_sight_subsys;
		}
	}

	return ss_return;
}

// function to return a pointer to the 'nth' ship_subsys structure in a ship's linked list
// of ship_subsys'.
// attacker_pos	=>	world pos of attacker (default value NULL).  If value is non-NULL, try
//							to select the best subsystem to attack of that type (using line-of-sight)
//							and based on the number of ships already attacking the subsystem
ship_subsys *ship_get_indexed_subsys( ship *sp, int index, vec3d *attacker_pos )
{
	int count;
	ship_subsys *ss;

	// first, special code to see if the index < 0.  If so, we are looking for one of several possible
	// engines or one of several possible turrets.  If we enter this if statement, we will always return
	// something.
	if ( index < 0 ) {
		int subsys_type;
		
		subsys_type = -index;
		if ( sp->subsys_info[subsys_type].aggregate_current_hits <= 0.0f )		// if there are no hits, no subsystem to attack.
			return NULL;

		if ( attacker_pos != NULL ) {
			ss = ship_get_best_subsys_to_attack(sp, subsys_type, attacker_pos);
			return ss;
		} else {
			// next, scan the list of subsystems and search for the first subsystem of the particular
			// type which has > 0 hits remaining.
			for (ss = GET_FIRST(&sp->subsys_list); ss != END_OF_LIST(&sp->subsys_list); ss = GET_NEXT(ss) ) {
				if ( (ss->system_info->type == subsys_type) && (ss->current_hits > 0) )
					return ss;
			}
		}
		
		// maybe we shouldn't get here, but with possible floating point rounding, I suppose we could
		Warning(LOCATION, "Unable to get a nonspecific subsystem of index %d on ship %s!", index, sp->ship_name);
		return NULL;
	}


	count = 0;
	ss = GET_FIRST(&sp->subsys_list);
	while ( ss != END_OF_LIST( &sp->subsys_list ) ) {
		if ( count == index )
			return ss;
		count++;
		ss = GET_NEXT( ss );
	}

	// get allender -- turret ref didn't fixup correctly!!!!
	Warning(LOCATION, "In ship_get_indexed_subsys, unable to get a subsystem of index %d on ship %s, due to a broken subsystem reference!  This is most likely due to a table/model mismatch.", index, sp->ship_name);	
	return NULL;
}

/**
 * Given a pointer to a subsystem and an associated object, return the index.
 */
int ship_get_index_from_subsys(ship_subsys *ssp, int objnum, int error_bypass)
{
	if (ssp == NULL)
		return -1;
	else {
		int	count;
		ship	*shipp;
		ship_subsys	*ss;

		Assert(objnum >= 0);
		Assert(Objects[objnum].instance >= 0);

		shipp = &Ships[Objects[objnum].instance];

		count = 0;
		ss = GET_FIRST(&shipp->subsys_list);
		while ( ss != END_OF_LIST( &shipp->subsys_list ) ) {
			if ( ss == ssp)
				return count;
			count++;
			ss = GET_NEXT( ss );
		}
		if ( !error_bypass )
			Int3();			// get allender -- turret ref didn't fixup correctly!!!!
		return -1;
	}
}

/**
 * Returns the index number of the ship_subsys parameter
 */
int ship_get_subsys_index(ship *sp, char *ss_name, int error_bypass)
{
	int count;
	ship_subsys *ss;

	count = 0;
	ss = GET_FIRST(&sp->subsys_list);
	while ( ss != END_OF_LIST( &sp->subsys_list ) ) {
		if ( !subsystem_stricmp(ss->system_info->subobj_name, ss_name) )
			return count;
		count++;
		ss = GET_NEXT( ss );
	}

	if (!error_bypass)
		Int3();

	return -1;
}

// routine to return the strength of a subsystem.  We keep a total hit tally for all subsystems
// which are similar (i.e. a total for all engines).  These routines will return a number between
// 0.0 and 1.0 which is the relative combined strength of the given subsystem type.  The number
// calculated for the engines is slightly different.  Once an engine reaches < 15% of its hits, its
// output drops to that %.  A dead engine has no output.
float ship_get_subsystem_strength( ship *shipp, int type )
{
	float strength;
	ship_subsys *ssp;

	Assert ( (type >= 0) && (type < SUBSYSTEM_MAX) );

	//	For a dying ship, all subsystem strengths are zero.
	if (Objects[shipp->objnum].hull_strength <= 0.0f)
		return 0.0f;

	// short circuit 1
	if (shipp->subsys_info[type].aggregate_max_hits <= 0.0f)
		return 1.0f;

	// short circuit 0
	if (shipp->subsys_info[type].aggregate_current_hits <= 0.0f)
		return 0.0f;

	strength = shipp->subsys_info[type].aggregate_current_hits / shipp->subsys_info[type].aggregate_max_hits;
	Assert( strength != 0.0f );

	if ( (type == SUBSYSTEM_ENGINE) && (strength < 1.0f) ) {
		float percent;

		percent = 0.0f;
		ssp = GET_FIRST(&shipp->subsys_list);
		while ( ssp != END_OF_LIST( &shipp->subsys_list ) ) {

			if ( ssp->system_info->type == SUBSYSTEM_ENGINE ) {
				float ratio;

				ratio = ssp->current_hits / ssp->max_hits;
				if ( ratio < ENGINE_MIN_STR )
					ratio = ENGINE_MIN_STR;

				percent += ratio;
			}
			ssp = GET_NEXT( ssp );
		}
		strength = percent / (float)shipp->subsys_info[type].type_count;
	}

	return strength;
}

/**
 * Set the strength of a subsystem on a given ship.
 *
 * The strength passed as a parameter is between 0.0 and 1.0
 *
 * NOTE: this function was made to be called by the debug function dcf_set_subsys().  If
 * you want to use this, be sure that you test it for all cases.
 */
void ship_set_subsystem_strength( ship *shipp, int type, float strength )
{
	float total_current_hits, diff;
	ship_subsys *ssp;

	Assert ( (type >= 0) && (type < SUBSYSTEM_MAX) );
	if ( shipp->subsys_info[type].aggregate_max_hits <= 0.0f )
		return;

	total_current_hits = 0.0f;
	ssp = GET_FIRST(&shipp->subsys_list);
	while ( ssp != END_OF_LIST( &shipp->subsys_list ) ) {

		if ( (ssp->system_info->type == type) && !(ssp->flags & SSF_NO_AGGREGATE) ) {
			ssp->current_hits = strength * ssp->max_hits;
			total_current_hits += ssp->current_hits;
		}
		ssp = GET_NEXT( ssp );
	}

	// update the objects integrity, needed since we've bashed the strength of a subsysem
	diff = total_current_hits - shipp->subsys_info[type].aggregate_current_hits;
	Objects[shipp->objnum].hull_strength += diff;
	// fix up the shipp->subsys_info[type] aggregate_current_hits value
	shipp->subsys_info[type].aggregate_current_hits = total_current_hits;
}

#define		SHIELD_REPAIR_RATE	0.20f			//	Percent of shield repaired per second.
#define		HULL_REPAIR_RATE		0.15f			//	Percent of hull repaired per second.
#define		SUBSYS_REPAIR_RATE	0.10f			// Percent of subsystems repaired per second.

#define REARM_NUM_MISSILES_PER_BATCH 4		// how many missiles are dropped in per load sound
#define REARM_NUM_BALLISTIC_PRIMARIES_PER_BATCH	100	// how many bullets are dropped in per load sound

/**
 * Calculates approximate time in seconds it would take to rearm and repair object.
 */
float ship_calculate_rearm_duration( object *objp )
{
	ship* sp;
	ship_info* sip;
	ship_subsys* ssp;
	ship_weapon* swp;
	weapon_info* wip;

	float shield_rep_time = 0;
	float subsys_rep_time = 0;
	float hull_rep_time = 0;
	float prim_rearm_time = 0;
	float sec_rearm_time = 0;

	float max_hull_repair;
	float max_subsys_repair;

	int i;
	int num_reloads;

	bool found_first_empty;
	
	Assert(objp->type == OBJ_SHIP);

	sp = &Ships[objp->instance];
	swp = &sp->weapons;
	sip = &Ship_info[sp->ship_info_index];

	//find out time to repair shields
	if(sip->sup_shield_repair_rate > 0.0f)
		shield_rep_time = ((sp->ship_max_shield_strength * sp->max_shield_recharge) - shield_get_strength(objp)) / (sp->max_shield_recharge * sp->ship_max_shield_strength * sip->sup_shield_repair_rate);
	
	max_hull_repair = sp->ship_max_hull_strength * (The_mission.support_ships.max_hull_repair_val * 0.01f);
	if ((The_mission.flags & MISSION_FLAG_SUPPORT_REPAIRS_HULL) && (max_hull_repair > objp->hull_strength) && (sip->sup_hull_repair_rate > 0.0f))
	{
		hull_rep_time = (max_hull_repair - objp->hull_strength) / (sp->ship_max_hull_strength * sip->sup_hull_repair_rate);
	}

	//caluclate subsystem repair time
	ssp = GET_FIRST(&sp->subsys_list);
	while (ssp != END_OF_LIST(&sp->subsys_list))
	{
		max_subsys_repair = ssp->max_hits * (The_mission.support_ships.max_subsys_repair_val * 0.01f);
		if ((max_subsys_repair > ssp->current_hits) && (sip->sup_subsys_repair_rate > 0.0f))
		{
			subsys_rep_time += (max_subsys_repair - ssp->current_hits) / (ssp->max_hits * sip->sup_subsys_repair_rate);
		}

		ssp = GET_NEXT( ssp );
	}

	//now do the primary rearm time
	found_first_empty = false;
	if (sip->flags & SIF_BALLISTIC_PRIMARIES)
	{
		for (i = 0; i < swp->num_primary_banks; i++)
		{
			wip = &Weapon_info[swp->primary_bank_weapons[i]];
			if (wip->wi_flags2 & WIF2_BALLISTIC)
			{
				//check how many full reloads we need
				num_reloads = (swp->primary_bank_start_ammo[i] - swp->primary_bank_ammo[i])/REARM_NUM_BALLISTIC_PRIMARIES_PER_BATCH;

				//take into account a fractional reload
				if ((swp->primary_bank_start_ammo[i] - swp->primary_bank_ammo[i]) % REARM_NUM_BALLISTIC_PRIMARIES_PER_BATCH != 0)
				{
					num_reloads++;
				}

				//don't factor in the time it takes for the first reload, since that is loaded instantly
				num_reloads--;

				if (num_reloads < 0) continue;

				if (!found_first_empty && (swp->primary_bank_start_ammo[i] - swp->primary_bank_ammo[i]))
				{
					found_first_empty = true;
					prim_rearm_time += (float)snd_get_duration(Snds[SND_MISSILE_START_LOAD].id) / 1000.0f;
				}

				prim_rearm_time += num_reloads * wip->rearm_rate;
			}
		}
	}

	//and on to secondary rearm time
	found_first_empty = false;
	for (i = 0; i < swp->num_secondary_banks; i++)
	{
			wip = &Weapon_info[swp->secondary_bank_weapons[i]];
	
			//check how many full reloads we need
			num_reloads = (swp->secondary_bank_start_ammo[i] - swp->secondary_bank_ammo[i])/REARM_NUM_MISSILES_PER_BATCH;

			//take into account a fractional reload
			if ((swp->secondary_bank_start_ammo[i] - swp->secondary_bank_ammo[i]) % REARM_NUM_MISSILES_PER_BATCH != 0)
			{
				num_reloads++;
			}

			//don't factor in the time it takes for the first reload, since that is loaded instantly
			num_reloads--;

			if (num_reloads < 0) continue;

			if (!found_first_empty && (swp->secondary_bank_start_ammo[i] - swp->secondary_bank_ammo[i]))
			{
				found_first_empty = true;
				sec_rearm_time += (float)snd_get_duration(Snds[SND_MISSILE_START_LOAD].id) / 1000.0f;
			}

			sec_rearm_time += num_reloads * wip->rearm_rate;
	}

	//sum them up and you've got an estimated rearm time.
	//add 1.2 to compensate for release delay
	return shield_rep_time + hull_rep_time + subsys_rep_time + prim_rearm_time + sec_rearm_time + 1.2f;
}



// ==================================================================================
// ship_do_rearm_frame()
//
// function to rearm a ship.  This function gets called from the ai code ai_do_rearm_frame (or
// some function of a similar name).  Returns 1 when ship is fully repaired and rearmed, 0 otherwise
//
int ship_do_rearm_frame( object *objp, float frametime )
{
	int			i, banks_full, primary_banks_full, subsys_type, subsys_all_ok, last_ballistic_idx = 0;
	float			shield_str, repair_delta, repair_allocated, max_hull_repair, max_subsys_repair;
	ship			*shipp;
	ship_weapon	*swp;
	ship_info	*sip;
	ship_subsys	*ssp;
	ai_info		*aip;

	shipp = &Ships[objp->instance];
	swp = &shipp->weapons;
	sip = &Ship_info[shipp->ship_info_index];
	aip = &Ai_info[shipp->ai_index];

	// AL 10-31-97: Add missing primary weapons to the ship.  This is required since designers
	//              want to have ships that start with no primaries, but can get them through
	//					 rearm/repair
	if ( swp->num_primary_banks < sip->num_primary_banks ) {
		for ( i = swp->num_primary_banks; i < sip->num_primary_banks; i++ ) {
			swp->primary_bank_weapons[i] = sip->primary_bank_weapons[i];
		}
		swp->num_primary_banks = sip->num_primary_banks;
	}
	// AL 12-30-97: Repair broken warp drive
	if ( shipp->flags & SF_WARP_BROKEN ) {
		// TODO: maybe do something here like informing player warp is fixed?
		// like this? -- Goober5000
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Subspace drive repaired.", 1635));
		shipp->flags &= ~SF_WARP_BROKEN;
	}

	// AL 1-16-97: Replenish countermeasures
	shipp->cmeasure_count = sip->cmeasure_max;

	// Do shield repair here
	if ( !(objp->flags & OF_NO_SHIELDS) )
	{
		shield_str = shield_get_strength(objp);
		if ( shield_str < (shipp->ship_max_shield_strength * shipp->max_shield_recharge) ) {
			if ( objp == Player_obj ) {
				player_maybe_start_repair_sound();
			}
			shield_str += shipp->ship_max_shield_strength * frametime * sip->sup_shield_repair_rate;
			if ( shield_str > shipp->ship_max_shield_strength ) {
				 shield_str = shipp->ship_max_shield_strength;
			}
			shield_set_strength(objp, shield_str);
		}
	}

	// Repair the ship integrity (subsystems + hull).  This works by applying the repair points
	// to the subsystems.  Ships integrity is stored is objp->hull_strength, so that always is 
	// incremented by repair_allocated
	repair_allocated = shipp->ship_max_hull_strength * frametime * sip->sup_hull_repair_rate;

//	AL 11-24-97: remove increase to hull integrity
//	Comments removed by PhReAk; Note that this is toggled on/off with a mission flag

	//Figure out how much of the ship's hull we can repair
	max_hull_repair = shipp->ship_max_hull_strength * (The_mission.support_ships.max_hull_repair_val * 0.01f);

	//Don't "reverse-repair" the hull if it's already above the max repair threshold
	if (objp->hull_strength > max_hull_repair)
	{
		max_hull_repair = objp->hull_strength;
	}
	
	if(The_mission.flags & MISSION_FLAG_SUPPORT_REPAIRS_HULL)
	{
		objp->hull_strength += repair_allocated;
		if ( objp->hull_strength > max_hull_repair ) {
			objp->hull_strength = max_hull_repair;
		}

		if ( objp->hull_strength > shipp->ship_max_hull_strength )
		{
			objp->hull_strength = shipp->ship_max_hull_strength;
			repair_allocated -= ( shipp->ship_max_hull_strength - objp->hull_strength);
		}
	}

	// figure out repairs for subsystems
	if(sip->sup_subsys_repair_rate == 0.0f)
		repair_allocated = 0.0f;
	else if(sip->sup_hull_repair_rate == 0.0f)
		repair_allocated = shipp->ship_max_hull_strength * frametime * sip->sup_subsys_repair_rate;
	else if(!(sip->sup_hull_repair_rate == sip->sup_subsys_repair_rate))
		repair_allocated = repair_allocated * sip->sup_subsys_repair_rate / sip->sup_hull_repair_rate;

	// check the subsystems of the ship.
	subsys_all_ok = 1;
	ssp = GET_FIRST(&shipp->subsys_list);
	while ( ssp != END_OF_LIST( &shipp->subsys_list ) ) {
		//Figure out how much we *can* repair the current subsystem -C
		max_subsys_repair = ssp->max_hits * (The_mission.support_ships.max_subsys_repair_val * 0.01f);

		if ( ssp->current_hits < max_subsys_repair && repair_allocated > 0 ) {
			subsys_all_ok = 0;
			subsys_type = ssp->system_info->type;

			if ( objp == Player_obj ) {
				player_maybe_start_repair_sound();
			}
			
			repair_delta = max_subsys_repair - ssp->current_hits;
			if ( repair_delta > repair_allocated ) {
				repair_delta = repair_allocated;
			}
			repair_allocated -= repair_delta;
			Assert(repair_allocated >= 0.0f);

			// add repair to current strength of single subsystem
			ssp->current_hits += repair_delta;
			if ( ssp->current_hits > max_subsys_repair ) {
				ssp->current_hits = max_subsys_repair;
			}

			// add repair to aggregate strength of subsystems of that type
			if (!(ssp->flags & SSF_NO_AGGREGATE)) {
				shipp->subsys_info[subsys_type].aggregate_current_hits += repair_delta;
				if ( shipp->subsys_info[subsys_type].aggregate_current_hits > shipp->subsys_info[subsys_type].aggregate_max_hits )
					shipp->subsys_info[subsys_type].aggregate_current_hits = shipp->subsys_info[subsys_type].aggregate_max_hits;
			}

			// check to see if this subsystem was totally non functional before -- if so, then
			// reset the flags
			if ( (ssp->system_info->type == SUBSYSTEM_ENGINE) && (shipp->flags & SF_DISABLED) ) {
				shipp->flags &= ~SF_DISABLED;
				ship_reset_disabled_physics(objp, shipp->ship_info_index);
			}
			break;
		}
		ssp = GET_NEXT( ssp );
	}

	// now deal with rearming the player.  All secondary weapons have a certain rate at which
	// they can be rearmed.  We can rearm multiple banks at once.
	banks_full = 0;
	primary_banks_full = 0;
	if ( subsys_all_ok )
	{
		for (i = 0; i < swp->num_secondary_banks; i++ )
		{
			// Actual loading of missiles is preceded by a sound effect which is the missile
			// loading equipment moving into place
			if ( aip->rearm_first_missile == TRUE )
			{
				swp->secondary_bank_rearm_time[i] = timestamp(snd_get_duration(Snds[SND_MISSILE_START_LOAD].id));			

				if (i == swp->num_secondary_banks - 1) 
					aip->rearm_first_missile = FALSE;
			}
			
			if ( swp->secondary_bank_ammo[i] < swp->secondary_bank_start_ammo[i] )
			{
				float rearm_time;

				if ( objp == Player_obj )
				{
					hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
				}

				if ( timestamp_elapsed(swp->secondary_bank_rearm_time[i]) )
				{
					rearm_time = Weapon_info[swp->secondary_bank_weapons[i]].rearm_rate;
					swp->secondary_bank_rearm_time[i] = timestamp((int)(rearm_time * 1000.0f));
					
					snd_play_3d( &Snds[SND_MISSILE_LOAD], &objp->pos, &View_position );
					if (objp == Player_obj)
						joy_ff_play_reload_effect();

					swp->secondary_bank_ammo[i] += REARM_NUM_MISSILES_PER_BATCH;
					if ( swp->secondary_bank_ammo[i] > swp->secondary_bank_start_ammo[i] ) 
					{
						swp->secondary_bank_ammo[i] = swp->secondary_bank_start_ammo[i]; 
					}
				}
				else
				{
				}
			} 
			else
			{
				banks_full++;
			}

			if ((aip->rearm_first_missile == TRUE) && (i == swp->num_secondary_banks - 1) && (banks_full != swp->num_secondary_banks))
					snd_play_3d( &Snds[SND_MISSILE_START_LOAD], &objp->pos, &View_position );
		}	// end for

		// rearm ballistic primaries - Goober5000
		if (sip->flags & SIF_BALLISTIC_PRIMARIES)
		{
			if ( aip->rearm_first_ballistic_primary == TRUE)
			{
				for (i = 1; i < swp->num_primary_banks; i++ )
				{
					if ( Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & WIF2_BALLISTIC )
						last_ballistic_idx = i;
				}
			}

			for (i = 0; i < swp->num_primary_banks; i++ )
			{
				if ( Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & WIF2_BALLISTIC )
				{
					// Actual loading of bullets is preceded by a sound effect which is the bullet
					// loading equipment moving into place
					if ( aip->rearm_first_ballistic_primary == TRUE )
					{
						// Goober5000
						int sound_index;
						if (Snds[SND_BALLISTIC_START_LOAD].id >= 0)
							sound_index = SND_BALLISTIC_START_LOAD;
						else
							sound_index = SND_MISSILE_START_LOAD;

						swp->primary_bank_rearm_time[i] = timestamp(snd_get_duration(Snds[sound_index].id));			

						if (i == last_ballistic_idx) 
							aip->rearm_first_ballistic_primary = FALSE;
					}

					if ( swp->primary_bank_ammo[i] < swp->primary_bank_start_ammo[i] )
					{
						float rearm_time;
	
						if ( objp == Player_obj )
						{
							hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
						}

						if ( timestamp_elapsed(swp->primary_bank_rearm_time[i]) )
						{
							rearm_time = Weapon_info[swp->primary_bank_weapons[i]].rearm_rate;
							swp->primary_bank_rearm_time[i] = timestamp( (int)(rearm_time * 1000.f) );
	
							// Goober5000
							int sound_index;
							if (Snds[SND_BALLISTIC_LOAD].id >= 0)
								sound_index = SND_BALLISTIC_LOAD;
							else
								sound_index = SND_MISSILE_LOAD;

							snd_play_3d( &Snds[sound_index], &objp->pos, &View_position );
	
							swp->primary_bank_ammo[i] += REARM_NUM_BALLISTIC_PRIMARIES_PER_BATCH;
							if ( swp->primary_bank_ammo[i] > swp->primary_bank_start_ammo[i] )
							{
								swp->primary_bank_ammo[i] = swp->primary_bank_start_ammo[i]; 
							}
						}
					}
					else
					{
						primary_banks_full++;
					}
				}
				// if the bank is not a ballistic
				else
				{
					primary_banks_full++;
				}

				if ((aip->rearm_first_ballistic_primary == TRUE) && (i == swp->num_primary_banks - 1) && (primary_banks_full != swp->num_primary_banks))
				{
					// Goober5000
					int sound_index;
					if (Snds[SND_BALLISTIC_START_LOAD].id >= 0)
						sound_index = SND_BALLISTIC_START_LOAD;
					else
						sound_index = SND_MISSILE_START_LOAD;

					snd_play_3d( &Snds[sound_index], &objp->pos, &View_position );
				}
			}	// end for
		}	// end if - rearm ballistic primaries
	} // end if (subsys_all_ok)

	if ( banks_full == swp->num_secondary_banks )
	{
		aip->rearm_first_missile = TRUE;
	}

	if ( primary_banks_full == swp->num_primary_banks )
	{
		aip->rearm_first_ballistic_primary = TRUE;
	}

	int shields_full = 0;
	if ( (objp->flags & OF_NO_SHIELDS) ) {
		shields_full = 1;
	} else {
		if ( shield_get_strength(objp) >= shipp->ship_max_shield_strength ) 
			shields_full = 1;
		if (sip->sup_shield_repair_rate == 0.0f)
			shields_full = 1;
	}

	int hull_ok = 0;
	if(objp->hull_strength >= max_hull_repair)
		hull_ok = 1;

	if(sip->sup_hull_repair_rate == 0.0f) {
		subsys_all_ok = 1;
		hull_ok = 1;
	}

	// return 1 if at end of subsystem list, hull damage at 0, and shields full and all secondary banks full.
	if ( (subsys_all_ok && shields_full && (The_mission.flags & MISSION_FLAG_SUPPORT_REPAIRS_HULL) && hull_ok ) || (subsys_all_ok && shields_full && !(The_mission.flags & MISSION_FLAG_SUPPORT_REPAIRS_HULL) ) )
	{
		if ( objp == Player_obj ) {
			player_stop_repair_sound();
		}

		if (!aip->rearm_release_delay)
			aip->rearm_release_delay = timestamp(1200);

		// check both primary and secondary banks are full
		if ( (banks_full == swp->num_secondary_banks) &&
			( !(sip->flags & SIF_BALLISTIC_PRIMARIES) || ((sip->flags & SIF_BALLISTIC_PRIMARIES) && (primary_banks_full == swp->num_primary_banks)) )	)
		{
			if ( timestamp_elapsed(aip->rearm_release_delay) )
				return 1;
		}
		else
		{
			aip->rearm_release_delay = timestamp(1200);
		}
	}

	if (objp == Player_obj) Player_rearm_eta -= frametime;

	return 0;
}

// Goober5000 - modified the logic to clarify the various states
// function which is used to find a repair ship to repair requester_obj.  the way repair ships will work is:
// if no ships in the mission at all, return 0
// if a ship can immediately satisfy a repair request, return 1 and fill in the pointer
// if no ships can satisfy a request, but we haven't reached either the concurrent or cumulative limit, return 2
// if no ships can satisfy a request, and we've reached the limits, but a request can be queued, return 3 and fill in the pointer
// if no ships can satisfy a request, we've reached the limits, and we can't queue anything, we're out of luck -- return 4
int ship_find_repair_ship( object *requester_obj, object **ship_we_found )
{
	object *objp;
	int num_support_ships = 0;
	float min_dist = -1.0f;
	object *nearest_support_ship = NULL;
	float min_time_till_available = -1.0f;
	object *soonest_available_support_ship = NULL;

	Assertion(requester_obj->type == OBJ_SHIP, "requester_obj not a ship. Has type of %08x", requester_obj->type);
	Assertion((requester_obj->instance >= 0) && (requester_obj->instance < MAX_SHIPS),
		"requester_obj does not have a valid pointer to a ship. Pointer is %d, which is smaller than 0 or bigger than %d",
		requester_obj->instance, MAX_SHIPS);

	ship *requester_ship = &Ships[requester_obj->instance];
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
	{
		if ((objp->type == OBJ_SHIP) && !(objp->flags & OF_SHOULD_BE_DEAD))
		{
			ship *shipp;
			ship_info *sip;
			float dist;

			Assertion((objp->instance >= 0) && (objp->instance < MAX_SHIPS),
				"objp does not have a valid pointer to a ship. Pointer is %d, which is smaller than 0 or bigger than %d",
				objp->instance, MAX_SHIPS);

			shipp = &Ships[objp->instance];

			if ( shipp->team != requester_ship->team ) {
				continue;
			}

			Assertion((shipp->ship_info_index >= 0) && (shipp->ship_info_index < static_cast<int>(Ship_info.size())),
				"Ship '%s' does not have a valid pointer to a ship class. Pointer is %d, which is smaller than 0 or bigger than %d",
				shipp->ship_name, shipp->ship_info_index, static_cast<int>(Ship_info.size()));

			sip = &Ship_info[shipp->ship_info_index];

			if ( !(sip->flags & SIF_SUPPORT) ) {
				continue;
			}

			// tally how many support ships actually exist
			num_support_ships++;

			// don't deal with dying or departing support ships
			if ( shipp->flags & (SF_DYING | SF_DEPARTING) ) {
				continue;
			}

			// Ship has been ordered to warpout but has not had a chance to process the order.
			Assertion( (shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO),
				"Ship '%s' doesn't have a valid ai pointer. Pointer is %d, which is smaller than 0 or larger than %d",
				shipp->ship_name, shipp->ai_index, MAX_AI_INFO);
			ai_info* aip = &(Ai_info[shipp->ai_index]);
			if ( ai_find_goal_index( aip->goals, AI_GOAL_WARP ) != -1 ) {
				continue;
			}

			dist = vm_vec_dist_quick(&objp->pos, &requester_obj->pos);

			if (((aip->ai_flags & (AIF_REPAIRING|AIF_AWAITING_REPAIR|AIF_BEING_REPAIRED)) > 0))
			{
				// support ship is already busy, track the one that will be
				// done soonest by estimating how many seconds it will take for the support ship
				// to reach the requester.
				// The estimate is calculated by calculating how many seconds it will take the
				// support ship to travel from its current location to the requester at max velocity
				// We assume that every leg of the support ships journey will take the amount of time
				// for the support ship to fly from its current location to the requester.  This is
				// a bit hacky but it penalizes further away support ships, so a futher support ship
				// will only be called if the closer ones are really busy.  This is just a craps shoot
				// anyway because everything is moving around.
				float howlong = 0;
				for( int i = 0; i < MAX_AI_GOALS; i++ ) {
					if ( aip->goals[i].ai_mode == AI_GOAL_REARM_REPAIR ) {
						howlong += dist * objp->phys_info.max_vel.xyz.z;
					}
				}
				if ( min_time_till_available < 0.0f || howlong < min_time_till_available ) {
					min_time_till_available = howlong;
					soonest_available_support_ship = objp;
				}
			}
			else
			{
				// support ship not already busy, find the closest
				if ( min_dist < 0.0f || dist < min_dist )
				{
					min_dist = dist;
					nearest_support_ship = objp;
				}
			}
		}
	}

	// no ships present?
	// (be advised we may have an Arriving_support_ship in this case)
	if (num_support_ships == 0) {
		return 0;
	}
	// ship available?
	else if (nearest_support_ship != NULL) {
		// the nearest non-busy support ship is to service request
		if (ship_we_found != NULL)
			*ship_we_found = nearest_support_ship;
		return 1;
	}
	// no ships available; are we below the limits?  (can we bring another ship in? -- and btw an Arriving_support_ship counts as being able to bring one in)
	else if ((num_support_ships < The_mission.support_ships.max_concurrent_ships)
		&& (	(Arriving_support_ship == NULL && The_mission.support_ships.tally < The_mission.support_ships.max_support_ships)
			 || (Arriving_support_ship != NULL && The_mission.support_ships.tally <= The_mission.support_ships.max_support_ships) ))
	{
		// We are allowed more support ships in the mission; request another ship
		// to service this request.
		return 2;
	}
	// we're at the limit, but maybe a ship will become available
	else if (soonest_available_support_ship != NULL) {
		// found more support ships than should be in mission, so I can't ask for more,
		// instead I will give the player the ship that will be done soonest
		if (ship_we_found != NULL)
			*ship_we_found = soonest_available_support_ship;
		return 3;
	}
	// none of the above; we're out of luck
	else {
		return 4;
	}
}

/**
 * Called in game_shutdown() to free malloced memory
 *
 * NOTE: do not call this function.  It is only called from ::game_shutdown()
 */
int CLOAKMAP=-1;
void ship_close()
{
	int i;

	for (i=0; i<MAX_SHIPS; i++ )	{
		ship *shipp = &Ships[i];

		if (shipp->shield_integrity != NULL) {
			vm_free(shipp->shield_integrity);
			shipp->shield_integrity = NULL;
		}

		if (shipp->ship_replacement_textures != NULL) {
			vm_free(shipp->ship_replacement_textures);
			shipp->ship_replacement_textures = NULL;
		}

		if(shipp->warpin_effect != NULL)
			delete shipp->warpin_effect;
		shipp->warpin_effect = NULL;

		if(shipp->warpout_effect != NULL)
			delete shipp->warpout_effect;
		shipp->warpout_effect = NULL;
	}

	// free this too! -- Goober5000
	ship_clear_subsystems();

	// free info from parsed table data
	Ship_info.clear();

	for (i = 0; i < (int)Ship_types.size(); i++) {
		Ship_types[i].ai_actively_pursues.clear();
		Ship_types[i].ai_actively_pursues_temp.clear();
	}
	Ship_types.clear();
	
	if(CLOAKMAP != -1)
		bm_release(CLOAKMAP);
}	

/**
 * Assign object-linked sound to a particular ship
 */
void ship_assign_sound(ship *sp)
{
	ship_info	*sip;	
	object *objp;
	vec3d engine_pos;
	ship_subsys *moveup;

	Assert( sp->objnum >= 0 );
	if(sp->objnum < 0){
		return;
	}

	objp = &Objects[sp->objnum];
	sip = &Ship_info[sp->ship_info_index];

	if ( sip->engine_snd != -1 ) {
		vm_vec_copy_scale(&engine_pos, &objp->orient.vec.fvec, -objp->radius/2.0f);		
		
		obj_snd_assign(sp->objnum, sip->engine_snd, &engine_pos, 1);
	}

	// Do subsystem sounds	
	moveup = GET_FIRST(&sp->subsys_list);
	while(moveup != END_OF_LIST(&sp->subsys_list)){
		// Check for any engine sounds		
		if(strstr(moveup->system_info->name, "enginelarge")){
			obj_snd_assign(sp->objnum, SND_ENGINE_LOOP_LARGE, &moveup->system_info->pnt, 0);
		} else if(strstr(moveup->system_info->name, "enginehuge")){
			obj_snd_assign(sp->objnum, SND_ENGINE_LOOP_HUGE, &moveup->system_info->pnt, 0);
		}

		//Do any normal subsystem sounds
		if(moveup->current_hits > 0.0f)
		{
			if(moveup->system_info->alive_snd != -1)
			{
				obj_snd_assign(sp->objnum, moveup->system_info->alive_snd, &moveup->system_info->pnt, 0, OS_SUBSYS_ALIVE, moveup);
				moveup->subsys_snd_flags |= SSSF_ALIVE;
			}
			if(moveup->system_info->turret_base_rotation_snd != -1)
			{
				obj_snd_assign(sp->objnum, moveup->system_info->turret_base_rotation_snd, &moveup->system_info->pnt, 0, OS_TURRET_BASE_ROTATION, moveup);
				moveup->subsys_snd_flags |= SSSF_TURRET_ROTATION;
			}
			if(moveup->system_info->turret_gun_rotation_snd != -1)
			{
				obj_snd_assign(sp->objnum, moveup->system_info->turret_gun_rotation_snd, &moveup->system_info->pnt, 0, OS_TURRET_GUN_ROTATION, moveup);
				moveup->subsys_snd_flags |= SSSF_TURRET_ROTATION;
			}
			if((moveup->system_info->rotation_snd != -1) && (moveup->flags & SSF_ROTATES))
			{
				obj_snd_assign(sp->objnum, moveup->system_info->rotation_snd, &moveup->system_info->pnt, 0, OS_SUBSYS_ROTATION, moveup);
				moveup->subsys_snd_flags |= SSSF_ROTATE;
			}
		} 
		else 
		{
			if(moveup->system_info->dead_snd != -1)
			{
				obj_snd_assign(sp->objnum, moveup->system_info->dead_snd, &moveup->system_info->pnt, 0, OS_SUBSYS_DEAD, moveup);
				moveup->subsys_snd_flags |= SSSF_DEAD;
			}
		}

		// next
		moveup = GET_NEXT(moveup);
	}	
}

/**
 * Assign object-linked sounds to all ships currently in the obj_used_list
 */
void ship_assign_sound_all()
{
	object *objp;
	size_t idx;
	int has_sounds;

	if ( !Sound_enabled )
		return;

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {		
		if ( objp->type == OBJ_SHIP && Player_obj != objp) {
			has_sounds = 0;

			// check to make sure this guy hasn't got sounds already assigned to him
			for(idx=0; idx<objp->objsnd_num.size(); idx++){
				if(objp->objsnd_num[idx] != -1){
					// skip
					has_sounds = 1;
					break;
				}
			}

			// actually assign the sound
			if(!has_sounds){
				ship_assign_sound(&Ships[objp->instance]);
			}
		}
	}
}


/**
 * Debug console function to set the shield for the player ship
 */
DCF(set_shield,"Change player ship shield strength")
{
	float value;

	if (dc_optional_string_either("help", "--help")) {
		dc_printf ("Usage: set_shield [num]\n");
		dc_printf ("[num] --  shield percentage 0.0 -> 1.0 of max\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf( "Shields are currently %.2f", shield_get_strength(Player_obj) );
		return;
	}

	dc_stuff_float(&value);

	CLAMP(value, 0.0f, 1.0f);

	shield_set_strength(Player_obj, value * Player_ship->ship_max_shield_strength);
	dc_printf("Shields set to %.2f\n", shield_get_strength(Player_obj) );
}

/**
 * Debug console function to set the hull for the player ship
 */
DCF(set_hull, "Change player ship hull strength")
{
	float value;
	
	if (dc_optional_string_either("help", "--help")) {
		dc_printf ("Usage: set_hull [num]\n");
		dc_printf ("[num] --  hull percentage 0.0 -> 1.0 of max\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf( "Hull is currently %.2f", Player_obj->hull_strength );
		return;
	}

	dc_stuff_float(&value);

	CLAMP(value, 0.0f, 1.0f);
	Player_obj->hull_strength = value * Player_ship->ship_max_hull_strength;
	dc_printf("Hull set to %.2f\n", Player_obj->hull_strength );
}

/**
 * Debug console function to set the strength of a particular subsystem
 */
//XSTR:OFF
DCF(set_subsys, "Set the strength of a particular subsystem on player ship" )
{
	SCP_string arg;
	int subsystem = SUBSYSTEM_NONE;
	float val_f;
	
	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: set_subsys <type> [--status] <strength>\n");
		dc_printf("<type> is any of the following:\n");
		dc_printf("\tweapons\n");
		dc_printf("\tengine\n");
		dc_printf("\tsensors\n");
		dc_printf("\tcommunication\n");
		dc_printf("\tnavigation\n");
		dc_printf("\tradar\n\n");

		dc_printf("[--status] will display status of that subsystem\n\n");
		
		dc_printf("<strength> is any value between 0 and 1.0\n");
		return;
	}

	dc_stuff_string_white(arg);

	if (arg == "weapons") {
		subsystem = SUBSYSTEM_WEAPONS;
	
	} else if (arg == "engine") {
		subsystem = SUBSYSTEM_ENGINE;	
	
	} else if (arg == "sensors") {
		subsystem = SUBSYSTEM_SENSORS;

	} else if (arg == "communication") {
		subsystem = SUBSYSTEM_COMMUNICATION;

	} else if (arg == "navigation") {
		subsystem = SUBSYSTEM_NAVIGATION;

	} else if (arg == "radar") {
		subsystem = SUBSYSTEM_RADAR;

	} else if ((arg == "status") || (arg == "--status") || (arg == "?") || (arg == "--?")) {
		dc_printf("Error: Must specify a subsystem.\n");
		return;

	} else {
		dc_printf("Error: Unknown argument '%s'\n", arg.c_str());
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Subsystem '%s' is at %f strength\n", arg.c_str(), ship_get_subsystem_strength(Player_ship, subsystem));

	} else {
		// Set the subsystem strength
		dc_stuff_float(&val_f);

		CLAMP(val_f, 0.0, 1.0);
		ship_set_subsystem_strength( Player_ship, subsystem, val_f );
		
		if (subsystem == SUBSYSTEM_ENGINE) {
			// If subsystem is an engine, set/clear the disabled flag
			(val_f < ENGINE_MIN_STR) ? (Player_ship->flags |= SF_DISABLED) : (Player_ship->flags &= (~SF_DISABLED));
		}
	}
}
//XSTR:ON

// console function to toggle whether auto-repair for subsystems is active
#ifndef NDEBUG
DCF_BOOL( auto_repair, Ship_auto_repair )
#endif

// two functions to keep track of counting ships of particular types.  Maybe we should be rolling this
// thing into the stats section??  The first function adds a ship of a particular type to the overall
// count of ships of that type (called from MissionParse.cpp).  The second function adds to the kill total
// of ships of a particular type.  Note that we use the ship_info flags structure member to determine
// what is happening.

//WMC - ALERT!!!!!!!!!!!
//These two functions did something weird with fighters/bombers. I don't
//think that not doing this will break anything, but it might.
//If it does, get me. OR someone smart.
//G5K - Someone smart to the rescue!  Fixed the functions so they don't
//accidentally overwrite all the information.

void ship_clear_ship_type_counts()
{
	// resize if we need to
	Ship_type_counts.resize(Ship_types.size());

	// clear all the stats
	for (size_t i = 0; i < Ship_type_counts.size(); i++)
	{
		Ship_type_counts[i].killed = 0;
		Ship_type_counts[i].total = 0;
	}
}

void ship_add_ship_type_count( int ship_info_index, int num )
{
	int type = ship_class_query_general_type(ship_info_index);

	//Ship has no type or something
	if(type < 0) {
		return;
	}

	//Add it
	Ship_type_counts[type].total += num;
}

void ship_add_ship_type_kill_count( int ship_info_index )
{
	int type = ship_class_query_general_type(ship_info_index);

	//Ship has no type or something
	if(type < 0) {
		return;
	}

	//Add it
	Ship_type_counts[type].killed++;
}

int ship_query_general_type(int ship)
{
	return ship_query_general_type(&Ships[ship]);
}

int ship_query_general_type(ship *shipp)
{
	return ship_class_query_general_type(shipp->ship_info_index);
}

int ship_class_query_general_type(int ship_class)
{
	//This is quick
	return Ship_info[ship_class].class_type;
}

/**
 * Returns true
 */
int ship_docking_valid(int docker, int dockee)
{
	// Goober5000
	// So many people have asked for this function to be extended that it's making less
	// and less sense to keep it around.  We should probably just let any ship type
	// dock with any other ship type and assume the mission designer is smart enough not to
	// mess things up.
	return 1;
}

// function to return a random ship in a starting player wing.  Returns -1 if a suitable
// one cannot be found
// input:	max_dist	=>	OPTIONAL PARAMETER (default value 0.0f) max range ship can be from player
// input:   persona  => OPTIONAL PARAMETER (default to -1) which persona to get
int ship_get_random_player_wing_ship( int flags, float max_dist, int persona_index, int get_first, int multi_team )
{
	const int MAX_SIZE = MAX_SHIPS_PER_WING * MAX_SQUADRON_WINGS;

	int i, j, ship_index, count;
	int slist[MAX_SIZE], which_one;

	// iterate through starting wings of player.  Add ship indices of ships which meet
	// given criteria
	count = 0;
	for (i = 0; i < Num_wings; i++ ) {
		if (count >= MAX_SIZE)
			break;

		int wingnum = -1;

		// multi-team?
		if(multi_team >= 0){
			if( i == TVT_wings[multi_team] ) {
				wingnum = i;
			} else {
				continue;
			}
		} else {
			// first check for a player starting wing
			for ( j = 0; j < MAX_STARTING_WINGS; j++ ) {
				if ( i == Starting_wings[j] ) {
					wingnum = i;
					break;
				}
			}

			// if not found, then check all squad wings (Goober5000)
			if ( wingnum == -1 ) {
				for ( j = 0; j < MAX_SQUADRON_WINGS; j++ ) {
					if ( i == Squadron_wings[j] ) {
						wingnum = i;
						break;
					}
				}
			}

			if ( wingnum == -1 ){
				continue;
			}
		}

		for ( j = 0; j < Wings[wingnum].current_count; j++ ) {
			if (count >= MAX_SIZE)
				break;

			ship_index = Wings[wingnum].ship_index[j];
			Assert( ship_index != -1 );

			if ( Ships[ship_index].flags & SF_DYING ) {
				continue;
			}
			// see if ship meets our criterea
			if ( (flags == SHIP_GET_NO_PLAYERS || flags == SHIP_GET_UNSILENCED) && (Objects[Ships[ship_index].objnum].flags & OF_PLAYER_SHIP) ){
				continue;
			}
			
			if ( (flags == SHIP_GET_UNSILENCED) && (Ships[ship_index].flags2 & SF2_NO_BUILTIN_MESSAGES) )
			{
				continue;
			}

			// don't process ships on a different team
			if(multi_team < 0){
				if ( Player_ship->team != Ships[ship_index].team ){
					continue;
				}
			}

			// see if ship is within max_dist units
			if ( (max_dist > 1.0f) && (multi_team < 0) ) {
				float dist;
				dist = vm_vec_dist_quick(&Objects[Ships[ship_index].objnum].pos, &Player_obj->pos);
				if ( dist > max_dist ) {
					continue;
				}
			}

			// if we should be checking persona's, then don't add ships that don't have the proper persona
			if ( persona_index != -1 ) {
				if ( Ships[ship_index].persona_index != persona_index ){
					continue;
				}
			}

			// return the first ship with correct persona
			if (get_first) {
				return ship_index;
			}

			slist[count] = ship_index;
			count++;
		}
	}

	if ( count == 0 ){
		return -1;
	}

	// now get a random one from the list
	which_one = (rand() % count);
	ship_index = slist[which_one];

	Assert ( Ships[ship_index].objnum != -1 );

	return ship_index;
}

// like above function, but returns a random ship in the given wing -- no restrictions
// input:	max_dist	=>	OPTIONAL PARAMETER (default value 0.0f) max range ship can be from player
int ship_get_random_ship_in_wing(int wingnum, int flags, float max_dist, int get_first)
{
	int i, ship_index, slist[MAX_SHIPS_PER_WING], count, which_one;

	count = 0;
	for ( i = 0; i < Wings[wingnum].current_count; i++ ) {
		ship_index = Wings[wingnum].ship_index[i];
		Assert( ship_index != -1 );

		if ( Ships[ship_index].flags & SF_DYING ) {
			continue;
		}

		// see if ship meets our criterea
		if ( (flags == SHIP_GET_NO_PLAYERS || flags == SHIP_GET_UNSILENCED) && (Objects[Ships[ship_index].objnum].flags & OF_PLAYER_SHIP) )
			continue;

		if ( (flags == SHIP_GET_UNSILENCED) && (Ships[ship_index].flags2 & SF2_NO_BUILTIN_MESSAGES) )
		{
			continue;
		}

		// see if ship is within max_dist units
		if ( max_dist > 0 ) {
			float dist;
			dist = vm_vec_dist_quick(&Objects[Ships[ship_index].objnum].pos, &Player_obj->pos);
			if ( dist > max_dist ) {
				continue;
			}
		}

		// return the first ship in wing
		if (get_first) {
			return ship_index;
		}

		slist[count] = ship_index;
		count++;
	}

	if ( count == 0 ) {
		return -1;
	}

	// now get a random one from the list
	which_one = (rand() % count);
	ship_index = slist[which_one];

	Assert ( Ships[ship_index].objnum != -1 );

	return ship_index;
}


// this function returns a random index into the Ship array of a ship of the given team
// cargo containers are not counted as ships for the purposes of this function.  Why???
// because now it is only used for getting a random ship for a message and cargo containers
// can't send mesages.  This function is an example of kind of bad coding :-(
// input:	max_dist	=>	OPTIONAL PARAMETER (default value 0.0f) max range ship can be from player
int ship_get_random_team_ship(int team_mask, int flags, float max_dist )
{
	int num, which_one;
	object *objp, *obj_list[MAX_SHIPS];

	// for any allied, go through the ships list and find all of the ships on that team
	num = 0;
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( objp->type != OBJ_SHIP )
			continue;

		// series of conditionals one per line for easy reading
		// don't process ships on wrong team
		// don't process cargo's or navbuoys
		// don't process player ships if flags are set
		if (!iff_matches_mask(Ships[objp->instance].team, team_mask))
			continue;
		else if ( Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_NOT_FLYABLE )
			continue;
		else if ( (flags == SHIP_GET_NO_PLAYERS) && (objp->flags & OF_PLAYER_SHIP) )
			continue;
		else if ( (flags == SHIP_GET_ONLY_PLAYERS) && !(objp->flags & OF_PLAYER_SHIP) )
			continue;

		if ( Ships[objp->instance].flags & SF_DYING ) {
			continue;
		}

		// see if ship is within max_dist units
		if ( max_dist > 0 ) {
			float dist;
			dist = vm_vec_dist_quick(&objp->pos, &Player_obj->pos);
			if ( dist > max_dist ) {
				continue;
			}
		}

		obj_list[num] = objp;
		num++;
	}

	if ( num == 0 )
		return -1;

	which_one = (rand() % num);
	objp = obj_list[which_one];

	Assert ( objp->instance != -1 );

	return objp->instance;
}

// -----------------------------------------------------------------------
// ship_secondary_bank_has_ammo()
//
// check if currently selected secondary bank has ammo
//
// input:	shipnum	=>	index into Ships[] array for ship to check
//
int ship_secondary_bank_has_ammo(int shipnum)
{
	ship_weapon	*swp;

	Assert(shipnum >= 0 && shipnum < MAX_SHIPS);
	swp = &Ships[shipnum].weapons;
	
	if ( swp->current_secondary_bank == -1 )
		return 0;

	Assert(swp->current_secondary_bank >= 0 && swp->current_secondary_bank < MAX_SHIP_SECONDARY_BANKS );
	if ( swp->secondary_bank_ammo[swp->current_secondary_bank] <= 0 )
		return 0;

	return 1;
}

// ship_primary_bank_has_ammo()
//
// check if currently selected primary bank has ammo
//
// input:	shipnum	=>	index into Ships[] array for ship to check
//
int ship_primary_bank_has_ammo(int shipnum)
{
	ship_weapon	*swp;

	Assert(shipnum >= 0 && shipnum < MAX_SHIPS);
	swp = &Ships[shipnum].weapons;
	
	if ( swp->current_primary_bank == -1 )
	{
		return 0;
	}

	Assert(swp->current_primary_bank >= 0 && swp->current_primary_bank < MAX_SHIP_PRIMARY_BANKS );
	
	return ( primary_out_of_ammo(swp, swp->current_primary_bank) == 0 );
}

// see if there is enough engine power to allow the ship to warp
// returns 1 if ship is able to warp, otherwise return 0
int ship_engine_ok_to_warp(ship *sp)
{
	// disabled ships can't warp
	if (sp->flags & SF_DISABLED)
		return 0;

	if (sp->flags & SF_WARP_BROKEN || sp->flags & SF_WARP_NEVER)
		return 0;

	float engine_strength = ship_get_subsystem_strength(sp, SUBSYSTEM_ENGINE);

	// if at 0% strength, can't warp
	if (engine_strength <= 0.0f)
		return 0;

	// player ships playing above Very Easy can't warp when below a threshold
	if ((sp == Player_ship) && (Game_skill_level > 0) && (engine_strength < SHIP_MIN_ENGINES_TO_WARP))
		return 0;

	// otherwise, warp is allowed
	return 1;
}

// Goober5000
// see if there is enough navigation power to allow the ship to warp
// returns 1 if ship is able to warp, otherwise return 0
int ship_navigation_ok_to_warp(ship *sp)
{
	// if not using the special flag, warp is always allowed
	if (!(The_mission.ai_profile->flags & AIPF_NAVIGATION_SUBSYS_GOVERNS_WARP))
		return 1;

	float navigation_strength = ship_get_subsystem_strength(sp, SUBSYSTEM_NAVIGATION);

	// if at 0% strength, can't warp
	if (navigation_strength <= 0.0f)
		return 0;

	// player ships playing above Very Easy can't warp when below a threshold
	if ((sp == Player_ship) && (Game_skill_level > 0) && (navigation_strength < SHIP_MIN_NAV_TO_WARP))
		return 0;

	// otherwise, warp is allowed
	return 1;
}

// Calculate the normal vector from a subsystem position and its first path point
// input:	sp	=>	pointer to ship that is parent of subsystem
//				ss =>	pointer to subsystem of interest
//				norm	=> output parameter... vector from subsys to first path point
//
//	exit:		0	=>	a valid vector was placed in norm
//				!0	=> an path normal could not be calculated
//				
int ship_return_subsys_path_normal(ship *shipp, ship_subsys *ss, vec3d *gsubpos, vec3d *norm)
{
	if ( ss->system_info->path_num >= 0 ) {
		polymodel	*pm = NULL;
		model_path	*mp;
		vec3d		*path_point;
		vec3d		gpath_point;
		pm = model_get(Ship_info[shipp->ship_info_index].model_num);
		Assert( pm != NULL );

		// possibly a bad model?
		Assertion(ss->system_info->path_num <= pm->n_paths, "Too many paths in '%s'!  Max is %i and the requested path was %i for subsystem '%s'!\n", pm->filename, pm->n_paths, ss->system_info->path_num, ss->system_info->subobj_name);
		if (ss->system_info->path_num > pm->n_paths) 
			return 1;

		mp = &pm->paths[ss->system_info->path_num];
		if ( mp->nverts >= 2 ) {
			path_point = &mp->verts[0].pos;
			// get path point in world coords
			vm_vec_unrotate(&gpath_point, path_point, &Objects[shipp->objnum].orient);
			vm_vec_add2(&gpath_point, &Objects[shipp->objnum].pos);
			// get unit vector pointing from subsys pos to first path point
			vm_vec_normalized_dir(norm, &gpath_point, gsubpos);
			return 0;
		}
	}
	return 1;
}


//	Determine if the subsystem can be viewed from eye_pos.  The method is to check where the
// vector from eye_pos to the subsystem hits the ship.  If distance from the hit position and
// the center of the subsystem is within a range (currently the subsystem radius) it is considered
// in view (return true).  If not in view, return false.
//
// input:	objp		=>		object that is the ship with the subsystem on it
//				subsys	=>		pointer to the subsystem of interest
//				eye_pos	=>		world coord for the eye looking at the subsystem
//				subsys_pos			=>	world coord for the center of the subsystem of interest
//				do_facing_check	=>	OPTIONAL PARAMETER (default value is 1), do a dot product check to see if subsystem fvec is facing
//											towards the eye position	
//				dot_out	=>		OPTIONAL PARAMETER, output parameter, will return dot between subsys fvec and subsys_to_eye_vec
//									(only filled in if do_facing_check is true)
//				vec_out	=>		OPTIONAL PARAMETER, vector from eye_pos to absolute subsys_pos.  (only filled in if do_facing_check is true)
int ship_subsystem_in_sight(object* objp, ship_subsys* subsys, vec3d *eye_pos, vec3d* subsys_pos, int do_facing_check, float *dot_out, vec3d *vec_out)
{
	float		dist, dot;
	mc_info	mc;
	vec3d	terminus, eye_to_pos, subsys_fvec, subsys_to_eye_vec;

	if (objp->type != OBJ_SHIP)
		return 0;

	// See if we are at least facing the subsystem
	if ( do_facing_check ) {
		if ( ship_return_subsys_path_normal(&Ships[objp->instance], subsys, subsys_pos, &subsys_fvec) ) {
			// non-zero return value means that we couldn't generate a normal from path info... so use inaccurate method
			vm_vec_normalized_dir(&subsys_fvec, subsys_pos, &objp->pos);
		}

		vm_vec_normalized_dir(&subsys_to_eye_vec, eye_pos, subsys_pos);
		dot = vm_vec_dot(&subsys_fvec, &subsys_to_eye_vec);
		if ( dot_out ) {
			*dot_out = dot;
		}

		if (vec_out) {
			*vec_out = subsys_to_eye_vec;
			vm_vec_negate(vec_out);
		}

		if ( dot < 0 )
			return 0;
	}

	// See if ray from eye to subsystem actually hits close enough to the subsystem position
	vm_vec_normalized_dir(&eye_to_pos, subsys_pos, eye_pos);
	vm_vec_scale_add(&terminus, eye_pos, &eye_to_pos, 100000.0f);

	mc_info_init(&mc);
	mc.model_instance_num = Ships[objp->instance].model_instance_num;
	mc.model_num = Ship_info[Ships[objp->instance].ship_info_index].model_num;			// Fill in the model to check
	mc.orient = &objp->orient;										// The object's orientation
	mc.pos = &objp->pos;												// The object's position
	mc.p0 = eye_pos;													// Point 1 of ray to check
	mc.p1 = &terminus;												// Point 2 of ray to check
	mc.flags = MC_CHECK_MODEL;	

	model_collide(&mc);

	if ( !mc.num_hits ) {
		return 0;
	}	

	// determine if hitpos is close enough to subsystem
	dist = vm_vec_dist(&mc.hit_point_world, subsys_pos);

	if ( dist <= subsys->system_info->radius ) {
		return 1;
	}
	
	return 0;
}

/**
 * Find a subsystem matching 'type' inside the ship, and that is not destroyed.  
 * @return If cannot find one, return NULL.
 */
ship_subsys *ship_return_next_subsys(ship *shipp, int type, vec3d *attacker_pos)
{
	ship_subsys	*ssp;

	Assert ( type >= 0 && type < SUBSYSTEM_MAX );

	// If aggregate total is 0, that means no subsystem is alive of that type
	if ( shipp->subsys_info[type].aggregate_max_hits <= 0.0f )
		return NULL;

	// loop through all the subsystems, if we find a match that has some strength, return it
	ssp = ship_get_best_subsys_to_attack(shipp, type, attacker_pos);

	return ssp;
}

// Returns the closest subsystem of specified type that is in line of sight.
// Returns null if all subsystems of that type are destroyed or none is in sight.
ship_subsys *ship_get_closest_subsys_in_sight(ship *sp, int subsys_type, vec3d *attacker_pos)
{
	Assert ( subsys_type >= 0 && subsys_type < SUBSYSTEM_MAX );

	// If aggregate total is 0, that means no subsystem is alive of that type
	if ( sp->subsys_info[subsys_type].aggregate_max_hits <= 0.0f )
		return NULL;

	ship_subsys	*closest_in_sight_subsys;
	ship_subsys	*ss;
	vec3d		gsubpos;
	float		closest_dist;
	float		ss_dist;

	closest_in_sight_subsys = NULL;
	closest_dist = FLT_MAX;

	for (ss = GET_FIRST(&sp->subsys_list); ss != END_OF_LIST(&sp->subsys_list); ss = GET_NEXT(ss) ) {
		if ( (ss->system_info->type == subsys_type) && (ss->current_hits > 0) ) {

			// get world pos of subsystem
			vm_vec_unrotate(&gsubpos, &ss->system_info->pnt, &Objects[sp->objnum].orient);
			vm_vec_add2(&gsubpos, &Objects[sp->objnum].pos);
			
			if ( ship_subsystem_in_sight(&Objects[sp->objnum], ss, attacker_pos, &gsubpos) ) {
				ss_dist = vm_vec_dist_squared(attacker_pos, &gsubpos);

				if ( ss_dist < closest_dist ) {
					closest_dist = ss_dist;
					closest_in_sight_subsys = ss;
				}
			}
		}
	}

	return closest_in_sight_subsys;
}

char *ship_subsys_get_name(ship_subsys *ss)
{
	if( ss->sub_name[0] != '\0' )
		return ss->sub_name;
	else
		return ss->system_info->name;
}

bool ship_subsys_has_instance_name(ship_subsys *ss)
{
	if( ss->sub_name[0] != '\0' )
		return true;
	else
		return false;
}

void ship_subsys_set_name(ship_subsys *ss, char* n_name)
{
	strncpy(ss->sub_name, n_name, NAME_LENGTH-1);
}

// Return the shield strength in the quadrant hit on hit_objp, based on global hitpos
//
// input:	hit_objp	=>	object pointer to ship getting hit
//				hitpos	=> global position of impact
//
// exit:		strength of shields in the quadrant that was hit as a percentage, between 0 and 1.0
//
// Assumes: that hitpos is a valid global hit position
float ship_quadrant_shield_strength(object *hit_objp, vec3d *hitpos)
{
	int			quadrant_num, i;
	float			max_quadrant;
	vec3d		tmpv1, tmpv2;

	// If ship doesn't have shield mesh, then return
	if ( hit_objp->flags & OF_NO_SHIELDS ) {
		return 0.0f;
	}

	// Check if all the shield quadrants are all already 0, if so return 0
	for ( i = 0; i < 4; i++ ) {
		if ( hit_objp->shield_quadrant[i] > 0 )
			break;
	}

	if ( i == 4 ) {
		return 0.0f;
	}

	// convert hitpos to position in model coordinates
	vm_vec_sub(&tmpv1, hitpos, &hit_objp->pos);
	vm_vec_rotate(&tmpv2, &tmpv1, &hit_objp->orient);
	quadrant_num = get_quadrant(&tmpv2, hit_objp);

	if ( quadrant_num < 0 )
		quadrant_num = 0;

	max_quadrant = get_max_shield_quad(hit_objp);
	if ( max_quadrant <= 0 ) {
		return 0.0f;
	}

	if(hit_objp->shield_quadrant[quadrant_num] > max_quadrant)
		mprintf(("Warning: \"%s\" has shield quadrant strength of %f out of %f\n",
				Ships[hit_objp->instance].ship_name, hit_objp->shield_quadrant[quadrant_num], max_quadrant));

	return hit_objp->shield_quadrant[quadrant_num]/max_quadrant;
}

// Determine if a ship is threatened by any dumbfire projectiles (laser or missile)
// input:	sp	=>	pointer to ship that might be threatened
// exit:		0 =>	no dumbfire threats
//				1 =>	at least one dumbfire threat
//
// NOTE: Currently this function is only called periodically from the HUD code for the 
//       player ship.
int ship_dumbfire_threat(ship *sp)
{
	if ( (Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER) ) {
		return 0;
	}

	if (ai_endangered_by_weapon(&Ai_info[sp->ai_index]) > 0) {
		return 1;
	} 

	return 0;
}

// Return !0 if there is a missile in the air homing on shipp
int ship_has_homing_missile_locked(ship *shipp)
{
	object		*locked_objp, *A;
	weapon		*wp;
	weapon_info	*wip;
	missile_obj	*mo;

	Assert(shipp->objnum >= 0 && shipp->objnum < MAX_OBJECTS);
	locked_objp = &Objects[shipp->objnum];

	// check for currently locked missiles (highest precedence)
	for ( mo = GET_NEXT(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
		Assert(mo->objnum >= 0 && mo->objnum < MAX_OBJECTS);
		A = &Objects[mo->objnum];

		if (A->type != OBJ_WEAPON)
			continue;

		Assert((A->instance >= 0) && (A->instance < MAX_WEAPONS));
		wp = &Weapons[A->instance];
		wip = &Weapon_info[wp->weapon_info_index];

		if ( wip->subtype != WP_MISSILE )
			continue;

		if ( !(wip->wi_flags & WIF_HOMING ) )
			continue;

		if (wp->homing_object == locked_objp) {
			return 1;
		}
	}	// end for 

	return 0;
}

// Return !0 if there is some ship attempting to lock onto shipp
int ship_is_getting_locked(ship *shipp)
{
	ship_obj	*so;
	object	*objp;
	ai_info	*aip;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];
		aip = &Ai_info[Ships[objp->instance].ai_index];

		if ( aip->target_objnum == shipp->objnum ) {
			if ( aip->aspect_locked_time > 0.1f ) {
				float dist, wep_range;
				dist = vm_vec_dist_quick(&objp->pos, &Objects[shipp->objnum].pos);
				wep_range = ship_get_secondary_weapon_range(&Ships[objp->instance]);
				if ( wep_range > dist ) {
					nprintf(("Alan","AI ship is seeking lock\n"));
					return 1;
				}
			}
		}
	}

	return 0;
}

// Determine if a ship is threatened by attempted lock or actual lock
// input:	sp	=>	pointer to ship that might be threatened
// exit:		0 =>	no lock threats of any kind
//				1 =>	at least one attempting lock (no actual locks)
//				2 =>	at least one lock (possible other attempting locks)
//
// NOTE: Currently this function is only called periodically from the HUD code for the 
//       player ship.
int ship_lock_threat(ship *sp)
{
	if ( ship_has_homing_missile_locked(sp) ) {
		return 2;
	}

	if ( ship_is_getting_locked(sp) ) {
		return 1;
	}

	return 0;
}

// converts a bitmask, such as 0x08, into the bit number this would be (3 in this case)
// NOTE: Should move file to something like Math_utils.
int bitmask_2_bitnum(int num)
{
	int i;

	for (i=0; i<32; i++)
		if (num & (1 << i))
			return i;

	return -1;
}

// Get a text description of a ships orders. 
//
//	input:	outbuf	=>		buffer to hold orders string
//				sp			=>		ship pointer to extract orders from
//
// exit:		NULL		=>		printable orders are not applicable
//				non-NULL	=>		pointer to string that was passed in originally
//
// This function is called from HUD code to get a text description
// of what a ship's orders are.  Feel free to use this function if 
// it suits your needs for something.
//
char *ship_return_orders(char *outbuf, ship *sp)
{
	ai_info	*aip;
	ai_goal	*aigp;
	const char		*order_text;
	char ship_name[NAME_LENGTH];
	
	Assert(sp->ai_index >= 0);
	aip = &Ai_info[sp->ai_index];

	// The active goal is always in the first element of aip->goals[]
	aigp = &aip->goals[0];

	if ( aigp->ai_mode < 0 ) 
		return NULL;

	order_text = Ai_goal_text(bitmask_2_bitnum(aigp->ai_mode));
	if ( order_text == NULL )
		return NULL;

	strcpy(outbuf, order_text);

	if ( aigp->target_name ) {
		strcpy_s(ship_name, aigp->target_name);
		end_string_at_first_hash_symbol(ship_name);
	}
	switch (aigp->ai_mode ) {

		case AI_GOAL_FORM_ON_WING:
		case AI_GOAL_GUARD_WING:
		case AI_GOAL_CHASE_WING:
			if ( aigp->target_name ) {
				strcat(outbuf, ship_name);
				strcat(outbuf, XSTR( "'s Wing", 494));
			} else {
				strcpy(outbuf, XSTR( "no orders", 495));
			}
			break;
	
		case AI_GOAL_CHASE:
		case AI_GOAL_DOCK:
		case AI_GOAL_UNDOCK:
		case AI_GOAL_GUARD:
		case AI_GOAL_DISABLE_SHIP:
		case AI_GOAL_DISARM_SHIP:
		case AI_GOAL_EVADE_SHIP:
		case AI_GOAL_REARM_REPAIR:
			if ( aigp->target_name ) {
				strcat(outbuf, ship_name);
			} else {
				strcpy(outbuf, XSTR( "no orders", 495));
			}
			break;

		case AI_GOAL_DESTROY_SUBSYSTEM: {
			if ( aip->targeted_subsys != NULL ) {
				char subsys_name[NAME_LENGTH];
				strcpy_s(subsys_name, aip->targeted_subsys->system_info->subobj_name);
				hud_targetbox_truncate_subsys_name(subsys_name);
				sprintf(outbuf, XSTR( "atk %s %s", 496), ship_name, subsys_name);
			} else {
				strcpy(outbuf, XSTR( "no orders", 495) );
			}
			break;
		}

		case AI_GOAL_WAYPOINTS:
		case AI_GOAL_WAYPOINTS_ONCE:
			// don't do anything, all info is in order_text
			break;

		case AI_GOAL_FLY_TO_SHIP:
			strcpy(outbuf, "Flying to ship");
			break;


		default:
			return NULL;
	}

	return outbuf;
}

// return the amount of time until ship reaches its goal (in MM:SS format)
//	input:	outbuf	=>		buffer to hold orders string
//				sp			=>		ship pointer to extract orders from
//
// exit:		NULL		=>		printable orders are not applicable
//				non-NULL	=>		pointer to string that was passed in originally
//
// This function is called from HUD code to get a text description
// of what a ship's orders are.  Feel free to use this function if 
// it suits your needs for something.
char *ship_return_time_to_goal(char *outbuf, ship *sp)
{
	ai_info	*aip;
	int		time, seconds, minutes;
	float		dist = 0.0f;
	object	*objp;	
	float		min_speed, max_speed;

	objp = &Objects[sp->objnum];
	aip = &Ai_info[sp->ai_index];

	min_speed = objp->phys_info.speed;

	// Goober5000 - handle cap
	if (aip->waypoint_speed_cap >= 0)
		max_speed = MIN(sp->current_max_speed, aip->waypoint_speed_cap);
	else
		max_speed = sp->current_max_speed;

	if ( aip->mode == AIM_WAYPOINTS ) {
		min_speed = 0.9f * max_speed;
		if (aip->wp_list != NULL) {
			Assert(aip->wp_index != INVALID_WAYPOINT_POSITION);
			dist += vm_vec_dist_quick(&objp->pos, aip->wp_list->get_waypoints()[aip->wp_index].get_pos());

			SCP_vector<waypoint>::iterator ii;
			vec3d *prev_vec = NULL;
			for (ii = (aip->wp_list->get_waypoints().begin() + aip->wp_index); ii != aip->wp_list->get_waypoints().end(); ++ii) {
				if (prev_vec != NULL) {
					dist += vm_vec_dist_quick(ii->get_pos(), prev_vec);
				}
				prev_vec = ii->get_pos();
			}
		}

		if ( dist < 1.0f) {
			return NULL;
		}	

		if ( (Objects[sp->objnum].phys_info.speed <= 0) || (max_speed <= 0.0f) ) {
			time = -1;
		} else {
			float	speed;

			speed = objp->phys_info.speed;

			if (speed < min_speed)
				speed = min_speed;
			time = fl2i(dist/speed);
		}

	} else if ( (aip->mode == AIM_DOCK) && (aip->submode < AIS_DOCK_4) ) {
		time = hud_get_dock_time( objp );
	} else {
		// don't return anytime for time to except for waypoints and actual docking.
		return NULL;
	}

	if ( time >= 0 ) {
		minutes = time/60;
		seconds = time%60;
		if ( minutes > 99 ) {
			minutes = 99;
			seconds = 99;
		}
		sprintf(outbuf, NOX("%02d:%02d"), minutes, seconds);
	} else {
		strcpy( outbuf, XSTR( "Unknown", 497) );
	}

	return outbuf;
}

/* Karajorma - V decided not to use this function so I've commented it out so it isn't confused with code
+that is actually in use. Someone might want to get it working using AI_Profiles at some point so I didn't
+simply delete it.

// Called to check if any AI ships might reveal the cargo of any cargo containers.
//
// This is called once a frame, but a global timer 'Ship_cargo_check_timer' will limit this
// function to being called every SHIP_CARGO_CHECK_INTERVAL ms.  I think that should be sufficient.
//
// NOTE: This function uses CARGO_REVEAL_DISTANCE from the HUD code... which is a multiple of
//       the ship radius that is used to determine when cargo is detected.  AI ships do not 
//       have to have the ship targeted to reveal cargo.  The player is ignored in this function.
#define SHIP_CARGO_CHECK_INTERVAL	1000
void ship_check_cargo_all()
{
	object	*cargo_objp;
	ship_obj	*cargo_so, *ship_so;
	ship		*cargo_sp, *ship_sp;
	float		dist_squared, limit_squared;

	// I don't want to do this check every frame, so I made a global timer to limit check to
	// every SHIP_CARGO_CHECK_INTERVAL ms.
	if ( !timestamp_elapsed(Ship_cargo_check_timer) ) {
		return;
	} else {
		Ship_cargo_check_timer = timestamp(SHIP_CARGO_CHECK_INTERVAL);
	}

	// Check all friendly fighter/bombers against all non-friendly cargo containers that don't have
	// cargo revealed

	// for now just locate a capital ship on the same team:
	cargo_so = GET_FIRST(&Ship_obj_list);
	while(cargo_so != END_OF_LIST(&Ship_obj_list)){
		cargo_sp = &Ships[Objects[cargo_so->objnum].instance];
		if ( (Ship_info[cargo_sp->ship_info_index].flags & SIF_CARGO) && (cargo_sp->team != Player_ship->team) ) {
			
			// If the cargo is revealed, continue on to next hostile cargo
			if ( cargo_sp->flags & SF_CARGO_REVEALED ) {
				goto next_cargo;
			}

			// check against friendly fighter/bombers + cruiser/freighter/transport
			// IDEA: could cull down to fighter/bomber if we want this to run a bit quicker
			for ( ship_so=GET_FIRST(&Ship_obj_list); ship_so != END_OF_LIST(&Ship_obj_list); ship_so=GET_NEXT(ship_so) )
			{
				ship_sp = &Ships[Objects[ship_so->objnum].instance];
				// only consider friendly ships
				if (ship_sp->team != Player_ship->team) {
					continue;
				}

				// ignore the player
				if ( ship_so->objnum == OBJ_INDEX(Player_obj) ) {
					continue;
				}

				// if this ship is a small or big ship
				if ( Ship_info[ship_sp->ship_info_index].flags & (SIF_SMALL_SHIP|SIF_BIG_SHIP) ) {
					cargo_objp = &Objects[cargo_sp->objnum];
					// use square of distance, faster than getting real distance (which will use sqrt)
					dist_squared = vm_vec_dist_squared(&cargo_objp->pos, &Objects[ship_sp->objnum].pos);
					limit_squared = (cargo_objp->radius+CARGO_RADIUS_DELTA)*(cargo_objp->radius+CARGO_RADIUS_DELTA);
					if ( dist_squared <= MAX(limit_squared, CARGO_REVEAL_MIN_DIST*CARGO_REVEAL_MIN_DIST) ) {
						ship_do_cargo_revealed( cargo_sp );
						break;	// break out of for loop, move on to next hostile cargo
					}
				}
			} // end for
		}
next_cargo:
		cargo_so = GET_NEXT(cargo_so);
	} // end while
}
*/


// Maybe warn player about this attacking ship.  This is called once per frame, and the
// information about the closest attacking ship comes for free, since this function is called
// from HUD code which has already determined the closest enemy attacker and the distance.
//
// input:	enemy_sp	=>	ship pointer to the TEAM_ENEMY ship attacking the player
//				dist		=>	the distance of the enemy to the player
//
// NOTE: there are no filters on enemy_sp, so it could be any ship type
//
#define PLAYER_CHECK_WARN_INTERVAL		300		// how often we check for warnings
#define PLAYER_MIN_WARN_DIST				100		// minimum distance attacking ship can be from player and still allow warning
#define PLAYER_MAX_WARN_DIST				1000		// maximum distance attacking ship can be from plyaer and still allow warning

void ship_maybe_warn_player(ship *enemy_sp, float dist)
{
	float		fdot; //, rdot, udot;
	vec3d	vec_to_target;
	int		msg_type; //, on_right;

	// First check if the player has reached the maximum number of warnings for a mission
	if ((Builtin_messages[MESSAGE_CHECK_6].max_count > -1) && ( Player->warn_count >= Builtin_messages[MESSAGE_CHECK_6].max_count )) {
		return;
	}

	// Check if enough time has elapsed since last warning, if not - leave
	if ( !timestamp_elapsed(Player->allow_warn_timestamp) ) {
		return;
	}

	// Check to see if check timer has elapsed.  Necessary, since we don't want to check each frame
	if ( !timestamp_elapsed(Player->check_warn_timestamp ) ) {
		return;
	}
	Player->check_warn_timestamp = timestamp(PLAYER_CHECK_WARN_INTERVAL);

	// only allow warnings if within a certain distance range
	if ( dist < PLAYER_MIN_WARN_DIST || dist > PLAYER_MAX_WARN_DIST ) {
		return;
	}

	// only warn if a fighter or bomber is attacking the player
	if ( !(Ship_info[enemy_sp->ship_info_index].flags & SIF_SMALL_SHIP) ) {
		return;
	}

	// get vector from player to target
	vm_vec_normalized_dir(&vec_to_target, &Objects[enemy_sp->objnum].pos, &Eye_position);

	// ensure that enemy fighter is oriented towards player
	fdot = vm_vec_dot(&Objects[enemy_sp->objnum].orient.vec.fvec, &vec_to_target);
	if ( fdot > -0.7 ) {
		return;
	}

	fdot = vm_vec_dot(&Player_obj->orient.vec.fvec, &vec_to_target);

	msg_type = -1;

	// check if attacking ship is on six.  return if not far enough behind player.
	if ( fdot > -0.7 )
		return;

	msg_type = MESSAGE_CHECK_6;

	if ( msg_type != -1 ) {
		int ship_index;

		// multiplayer tvt - this is client side.
		if(MULTI_TEAM && (Net_player != NULL)){
			ship_index = ship_get_random_player_wing_ship( SHIP_GET_UNSILENCED, 0.0f, -1, 0, Net_player->p_info.team );
		} else {
			ship_index = ship_get_random_player_wing_ship( SHIP_GET_UNSILENCED );
		}

		if ( ship_index >= 0 ) {
			// multiplayer - make sure I just send to myself
			if(Game_mode & GM_MULTIPLAYER){
				message_send_builtin_to_player(msg_type, &Ships[ship_index], MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, MY_NET_PLAYER_NUM, -1);
			} else {
				message_send_builtin_to_player(msg_type, &Ships[ship_index], MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, -1);
			}
			Player->allow_warn_timestamp = timestamp(Builtin_messages[MESSAGE_CHECK_6].min_delay);
			Player->warn_count++;
		}
	}
}

// player has just killed a ship, maybe offer send a 'good job' message
void ship_maybe_praise_player(ship *deader_sp)
{
	if ( myrand()&1 ) {
		return;
	}

	// First check if the player has reached the maximum number of praises for a mission
	if ((Builtin_messages[MESSAGE_PRAISE].max_count > -1) && (Player->praise_count >= Builtin_messages[MESSAGE_PRAISE].max_count )) {
		return;
	}

	// Check if enough time has elapsed since last praise, if not - leave
	if ( !timestamp_elapsed(Player->allow_praise_timestamp) ) {
		return;
	}

	// make sure player is not a traitor
	if (Player_ship->team == Iff_traitor) {
		return;
	}

	// only praise if killing an enemy!
	if ( deader_sp->team == Player_ship->team ) {
		return;
	}

	// don't praise the destruction of navbuoys, cargo or other non-flyable ship types
	if ( (Ship_info[deader_sp->ship_info_index].class_type > 0) && !(Ship_types[Ship_info[deader_sp->ship_info_index].class_type].message_bools & STI_MSG_PRAISE_DESTRUCTION) ) {
		return;
	}

	// There is already a praise pending
	if ( Player->praise_delay_timestamp ) {
		return;
	}

	// We don't want to praise the player right away.. it is more realistic to wait a moment
	Player->praise_delay_timestamp = timestamp_rand(1000, 2000);
}

void ship_maybe_praise_self(ship *deader_sp, ship *killer_sp)
{
	int j; 
	bool wingman = false;

	if ( (int)(frand()*100) > Builtin_messages[MESSAGE_PRAISE_SELF].occurrence_chance ) {
		return;
	}

	if (Game_mode & GM_MULTIPLAYER) {
		return;
	}

	if ((Builtin_messages[MESSAGE_PRAISE_SELF].max_count > -1) && (Player->praise_self_count >= Builtin_messages[MESSAGE_PRAISE_SELF].max_count)) {
		return;
	}

	if (!timestamp_elapsed(Player->praise_self_timestamp)) {
		return;
	}

	// only praise if killing an enemy so check they both attack each other!
	if (!((iff_x_attacks_y(deader_sp->team, killer_sp->team)) && (iff_x_attacks_y(killer_sp->team, deader_sp->team ))) ) {
		return;
	}

	
	// only send messages from the player's wingmen
	if (killer_sp->wingnum == -1) {
		return; 
	}
	for ( j = 0; j < MAX_STARTING_WINGS; j++ ) {
		if ( Starting_wings[j] == killer_sp->wingnum) {
			wingman = true; 
			break;
		}
	}

	if (!wingman) {
		return;
	}

	// don't praise the destruction of navbuoys, cargo or other non-flyable ship types
	if ( (Ship_info[deader_sp->ship_info_index].class_type > 0) && !(Ship_types[Ship_info[deader_sp->ship_info_index].class_type].message_bools & STI_MSG_PRAISE_DESTRUCTION) ) {
		return;
	}

	// ensure the ship isn't silenced
	if ( killer_sp->flags2 & SF2_NO_BUILTIN_MESSAGES ) {
		return; 
	}

	message_send_builtin_to_player(MESSAGE_PRAISE_SELF, killer_sp, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_SOON, 0, 0, -1, -1);
	Player->praise_self_timestamp = timestamp(Builtin_messages[MESSAGE_PRAISE_SELF].min_delay);
	Player->praise_self_count++;			
}

#define ASK_HELP_SHIELD_PERCENT			0.1		// percent shields at which ship will ask for help
#define ASK_HELP_HULL_PERCENT				0.3		// percent hull at which ship will ask for help
#define AWACS_HELP_HULL_HI					0.75		// percent hull at which ship will ask for help
#define AWACS_HELP_HULL_LOW				0.25		// percent hull at which ship will ask for help

// -----------------------------------------------------------------------------
void awacs_maybe_ask_for_help(ship *sp, int multi_team_filter)
{
	// Goober5000 - bail if not in main fs2 campaign
	// (stupid coders... it's the FREDder's responsibility to add this message)
	if (stricmp(Campaign.filename, "freespace2") || !(Game_mode & GM_CAMPAIGN_MODE))
		return;

	object *objp;
	int message = -1;
	objp = &Objects[sp->objnum];

	if ( objp->hull_strength < ( (AWACS_HELP_HULL_LOW + 0.01f *(static_rand(objp-Objects) & 5)) * sp->ship_max_hull_strength) ) {
		// awacs ship below 25 + (0-4) %
		if (!(sp->awacs_warning_flag & AWACS_WARN_25)) {
			message = MESSAGE_AWACS_25;
			sp->awacs_warning_flag |=  AWACS_WARN_25;
		}
	} else if ( objp->hull_strength < ( (AWACS_HELP_HULL_HI + 0.01f*(static_rand(objp-Objects) & 5)) * sp->ship_max_hull_strength) ) {
		// awacs ship below 75 + (0-4) %
		if (!(sp->awacs_warning_flag & AWACS_WARN_75)) {
			message = MESSAGE_AWACS_75;
			sp->awacs_warning_flag |=  AWACS_WARN_75;
		}
	}

	if (message >= 0) {
		message_send_builtin_to_player(message, sp, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, multi_team_filter);
		Player->allow_ask_help_timestamp = timestamp(Builtin_messages[MESSAGE_HELP].min_delay);
		Player->ask_help_count++;
	}
}

// -----------------------------------------------------------------------------
void ship_maybe_ask_for_help(ship *sp)
{
	object *objp;
	int multi_team_filter = -1;

	// First check if the player has reached the maximum number of ask_help's for a mission
	if ((Builtin_messages[MESSAGE_HELP].max_count > -1) && (Player->ask_help_count >= Builtin_messages[MESSAGE_HELP].max_count))
		return;

	// Check if enough time has elapsed since last help request, if not - leave
	if (!timestamp_elapsed(Player->allow_ask_help_timestamp))
		return;

	// make sure player is on their team and not a traitor
	if ((Player_ship->team != sp->team) || (Player_ship->team == Iff_traitor))
		return;

	objp = &Objects[sp->objnum];

	// don't let the player ask for help!
	if (objp->flags & OF_PLAYER_SHIP)
		return;

	// determine team filter if TvT
	if(MULTI_TEAM)
		multi_team_filter = sp->team;

	// handle awacs ship as a special case
	if (Ship_info[sp->ship_info_index].flags & SIF_HAS_AWACS)
	{
		awacs_maybe_ask_for_help(sp, multi_team_filter);
		return;
	}

	// for now, only have wingman ships request help
	if (!(sp->flags & SF_FROM_PLAYER_WING))
		return;

	// first check if hull is at a critical level
	if (objp->hull_strength < ASK_HELP_HULL_PERCENT * sp->ship_max_hull_strength)
		goto play_ask_help;

	// check if shields are near critical level
	if (objp->flags & OF_NO_SHIELDS)
		return;	// no shields on ship, no don't check shield levels

	if (shield_get_strength(objp) > (ASK_HELP_SHIELD_PERCENT * sp->ship_max_shield_strength))
		return;

play_ask_help:

	if (!(Ship_info[sp->ship_info_index].flags & (SIF_FIGHTER|SIF_BOMBER))) //If we're still here, only continue if we're a fighter or bomber.
		return;

	if (!(sp->flags2 & SF2_NO_BUILTIN_MESSAGES)) // Karajorma - Only unsilenced ships should ask for help
	{
	message_send_builtin_to_player(MESSAGE_HELP, sp, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, multi_team_filter);
	Player->allow_ask_help_timestamp = timestamp(Builtin_messages[MESSAGE_HELP].min_delay);

	// prevent overlap with death message
	if (timestamp_until(Player->allow_scream_timestamp) < 15000)
		Player->allow_scream_timestamp = timestamp(15000);

	Player->ask_help_count++;
	}
}

/**
 * The player has just entered death roll, maybe have wingman mourn the loss of the player
 */
void ship_maybe_lament()
{
	int ship_index;

	// no. because in multiplayer, its funny
	if (Game_mode & GM_MULTIPLAYER)
		return;

	if (rand() % 4 == 0)
	{
		ship_index = ship_get_random_player_wing_ship(SHIP_GET_UNSILENCED);
		if (ship_index >= 0)
			message_send_builtin_to_player(MESSAGE_PLAYER_DIED, &Ships[ship_index], MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, -1);
	}
}

/**
 * Play a death scream for a ship
 */
void ship_scream(ship *sp)
{
	int multi_team_filter = -1;

	// bogus
	if (sp == NULL)
		return;

	// multiplayer tvt
	if (MULTI_TEAM)
		multi_team_filter = sp->team;

	// Bail if the ship is silenced
	if (sp->flags2 & SF2_NO_BUILTIN_MESSAGES)
	{
		return;
	}

	message_send_builtin_to_player(MESSAGE_WINGMAN_SCREAM, sp, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, multi_team_filter);
	Player->allow_scream_timestamp = timestamp(Builtin_messages[MESSAGE_WINGMAN_SCREAM].min_delay);
	Player->scream_count++;

	sp->flags |= SF_SHIP_HAS_SCREAMED;

	// prevent overlap with help messages
	if (timestamp_until(Player->allow_ask_help_timestamp) < 15000)
		Player->allow_ask_help_timestamp = timestamp(15000);
}

/**
 * Ship has just died, maybe play a scream.
 */
void ship_maybe_scream(ship *sp)
{
	// bail if screaming is disabled
	if (sp->flags2 & SF2_NO_DEATH_SCREAM)
		return;

	// if screaming is enabled, skip all checks
	if (!(sp->flags2 & SF2_ALWAYS_DEATH_SCREAM))
	{
		// only scream x% of the time 
		if ( (int)(frand()*100) > Builtin_messages[MESSAGE_WINGMAN_SCREAM].occurrence_chance ) {
			return;
		}

		// check if enough time has elapsed since last scream; if not, leave
		if (!timestamp_elapsed(Player->allow_scream_timestamp))
			return;

		// for WCSaga, only do a subset of the checks
		if (!(The_mission.ai_profile->flags2 & AIPF2_PERFORM_FEWER_SCREAM_CHECKS))
		{
			// bail if this ship isn't from the player wing
			if (!(sp->flags & SF_FROM_PLAYER_WING))
				return;

			// first check if the player has reached the maximum number of screams for a mission
			if ((Builtin_messages[MESSAGE_WINGMAN_SCREAM].max_count > -1) && (Player->scream_count >= Builtin_messages[MESSAGE_WINGMAN_SCREAM].max_count)) {
				return;
			}

			// if on different teams (i.e. team v. team games in multiplayer), no scream
			if (Player_ship->team != sp->team)
				return;
		}
	}

	ship_scream(sp);
}

// maybe tell player that we're running low on ammo
#define PLAYER_LOW_AMMO_MSG_INTERVAL		250000
#define PLAYER_REQUEST_REPAIR_MSG_INTERVAL	240000
#define PLAYER_MAX_LOW_AMMO_MSGS			5

/**
 * This function is only for notifying the player that the ship's ammo is low, without necessarily requesting support (e.g. if support
 * is disallowed for this mission).  It is not called if support was successfully requested in maybe_request_support.
 */
void ship_maybe_tell_about_low_ammo(ship *sp)
{
	weapon_info *wip;
	int i;
	ship_weapon *swp;
	int multi_team_filter = -1;

	// we don't want a ship complaining about low ammo after it has just complained about needing support
	if (!timestamp_elapsed(Player->request_repair_timestamp))
		return;

	if (!timestamp_elapsed(Player->allow_ammo_timestamp))
		return;

	if (Player_ship->team == Iff_traitor)
		return;

	// Silent ships should remain just that
	if (sp->flags2 & SF2_NO_BUILTIN_MESSAGES) {
		return;
	}

	// don't mention low ammo if we're docked, for the same reason as in maybe_request_support
	if (object_is_docked(&Objects[sp->objnum]))
		return;

	// for now, each ship can only complain about low ammo once a mission to stop it getting repetitive
	if (sp->ammo_low_complaint_count) {
		return;
	}

	if (Player->low_ammo_complaint_count >= PLAYER_MAX_LOW_AMMO_MSGS) {
		return;
	}

	swp = &sp->weapons;
	
	// stole the code for this from ship_maybe_tell_about_rearm()
	if (Ship_info[sp->ship_info_index].flags & SIF_BALLISTIC_PRIMARIES)
	{
		for (i = 0; i < swp->num_primary_banks; i++)
		{
			wip = &Weapon_info[swp->primary_bank_weapons[i]];

			if (wip->wi_flags2 & WIF2_BALLISTIC)
			{
				if (swp->primary_bank_start_ammo[i] > 0)
				{
					if ((float)swp->primary_bank_ammo[i] / (float)swp->primary_bank_start_ammo[i] < 0.3f)
					{
						// multiplayer tvt
						if(MULTI_TEAM) {
							multi_team_filter = sp->team;
						}

						message_send_builtin_to_player(MESSAGE_PRIMARIES_LOW, sp, MESSAGE_PRIORITY_NORMAL, MESSAGE_TIME_SOON, 0, 0, -1, multi_team_filter);

						Player->allow_ammo_timestamp = timestamp(PLAYER_LOW_AMMO_MSG_INTERVAL);

						// better reset this one too
						Player->request_repair_timestamp = timestamp(PLAYER_REQUEST_REPAIR_MSG_INTERVAL);

						Player->low_ammo_complaint_count++;
						sp->ammo_low_complaint_count++;
						break;
					}
				}
			}
		}
	}
}


/**
 * Tell player that we've requested a support ship
 */
void ship_maybe_tell_about_rearm(ship *sp)
{
	weapon_info *wip;

	if (!timestamp_elapsed(Player->request_repair_timestamp))
		return;

	if (Player_ship->team == Iff_traitor)
		return;

	// Silent ships should remain just that
	if (sp->flags2 & SF2_NO_BUILTIN_MESSAGES)
		return;

	// AL 1-4-98:	If ship integrity is low, tell player you want to get repaired.  Otherwise, tell
	// the player you want to get re-armed.

	int message_type = -1;
	int heavily_damaged = (get_hull_pct(&Objects[sp->objnum]) < 0.4);

	if (heavily_damaged || (sp->flags & SF_DISABLED))
	{
		message_type = MESSAGE_REPAIR_REQUEST;
	}
	else
	{
		int i;
		ship_weapon *swp;

		swp = &sp->weapons;
		for (i = 0; i < swp->num_secondary_banks; i++)
		{
			if (swp->secondary_bank_start_ammo[i] > 0)
			{
				if ((float)swp->secondary_bank_ammo[i] / (float)swp->secondary_bank_start_ammo[i] < 0.5f)
				{
					message_type = MESSAGE_REARM_REQUEST;
					break;
				}
			}
		}

		// also check ballistic primaries - Goober5000
		if (Ship_info[sp->ship_info_index].flags & SIF_BALLISTIC_PRIMARIES)
		{
			for (i = 0; i < swp->num_primary_banks; i++)
			{
				wip = &Weapon_info[swp->primary_bank_weapons[i]];

				if (wip->wi_flags2 & WIF2_BALLISTIC)
				{
					if (swp->primary_bank_start_ammo[i] > 0)
					{
						if ((float)swp->primary_bank_ammo[i] / (float)swp->primary_bank_start_ammo[i] < 0.3f)
						{
							message_type = MESSAGE_REARM_PRIMARIES;
							break;
						}
					}
				}
			}
		}
	}

	int multi_team_filter = -1;

	// multiplayer tvt
	if(MULTI_TEAM)
		multi_team_filter = sp->team;


	if (message_type >= 0)
	{
		if (rand() & 1)
			message_send_builtin_to_player(message_type, sp, MESSAGE_PRIORITY_NORMAL, MESSAGE_TIME_SOON, 0, 0, -1, multi_team_filter);

		Player->request_repair_timestamp = timestamp(PLAYER_REQUEST_REPAIR_MSG_INTERVAL);
	}
}

// The current primary weapon or link status for a ship has changed.. notify clients if multiplayer
//
// input:	sp			=>	pointer to ship that modified primaries
void ship_primary_changed(ship *sp)
{
	int i;
	ship_weapon	*swp;
	swp = &sp->weapons;


	if (sp->flags & SF_PRIMARY_LINKED) {
		// if we are linked now find any body who is down and flip them up
		for (i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
			if (swp->primary_animation_position[i] == MA_POS_NOT_SET) {
				if ( model_anim_start_type(sp, TRIGGER_TYPE_PRIMARY_BANK, i, 1) ) {
					swp->primary_animation_done_time[i] = model_anim_get_time_type(sp, TRIGGER_TYPE_PRIMARY_BANK, i);
					swp->primary_animation_position[i] = MA_POS_SET;
				} else {
					swp->primary_animation_position[i] = MA_POS_READY;
				}
			}
		}
	} else {
		// find anything that is up that shouldn't be
		for (i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
			if (i == swp->current_primary_bank) {
				// if the current bank is down raise it up
				if (swp->primary_animation_position[i] == MA_POS_NOT_SET) {
					if ( model_anim_start_type(sp, TRIGGER_TYPE_PRIMARY_BANK, i, 1) ) {
						swp->primary_animation_done_time[i] = model_anim_get_time_type(sp, TRIGGER_TYPE_PRIMARY_BANK, i);
						swp->primary_animation_position[i] = MA_POS_SET;
					} else {
						swp->primary_animation_position[i] = MA_POS_READY;
					}
				}
			} else {
				// everyone else should be down, if they are not make them so
				if (swp->primary_animation_position[i] != MA_POS_NOT_SET) {
					model_anim_start_type(sp, TRIGGER_TYPE_PRIMARY_BANK, i, -1);
					swp->primary_animation_position[i] = MA_POS_NOT_SET;
				}
			}
		}
	}

#if 0
	// we only need to deal with multiplayer issues for now, so bail it not multiplayer
	if ( !(Game_mode & GM_MULTIPLAYER) )
		return;

	Assert(sp);

	if ( MULTIPLAYER_MASTER )
		send_ship_weapon_change( sp, MULTI_PRIMARY_CHANGED, swp->current_primary_bank, (sp->flags & SF_PRIMARY_LINKED)?1:0 );
#endif
}

// The current secondary weapon or dual-fire status for a ship has changed.. notify clients if multiplayer
//
// input:	sp					=>	pointer to ship that modified secondaries
void ship_secondary_changed(ship *sp)
{
	Assert( sp != NULL );

	int i;
	ship_weapon	*swp = &sp->weapons;

	// find anything that is up that shouldn't be
	if (timestamp() > 10) {
		for (i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
			if (i == swp->current_secondary_bank) {
				// if the current bank is down raise it up
				if (swp->secondary_animation_position[i] == MA_POS_NOT_SET) {
					if ( model_anim_start_type(sp, TRIGGER_TYPE_SECONDARY_BANK, i, 1) ) {
						swp->secondary_animation_done_time[i] = model_anim_get_time_type(sp, TRIGGER_TYPE_SECONDARY_BANK, i);
						swp->secondary_animation_position[i] = MA_POS_SET;
					} else {
						swp->secondary_animation_position[i] = MA_POS_READY;
					}
				}
			} else {
				// everyone else should be down, if they are not make them so
				if (swp->secondary_animation_position[i] != MA_POS_NOT_SET) {
					model_anim_start_type(sp, TRIGGER_TYPE_SECONDARY_BANK, i, -1);
					swp->secondary_animation_position[i] = MA_POS_NOT_SET;
				}
			}
		}
	}

#if 0
	// we only need to deal with multiplayer issues for now, so bail it not multiplayer
	if ( !(Game_mode & GM_MULTIPLAYER) ){
		return;
	}

	Assert(sp);

	if ( MULTIPLAYER_MASTER )
		send_ship_weapon_change( sp, MULTI_SECONDARY_CHANGED, swp->current_secondary_bank, (sp->flags & SF_SECONDARY_DUAL_FIRE)?1:0 );
#endif
}

int ship_get_SIF(ship *shipp)
{
	return Ship_info[shipp->ship_info_index].flags;
}

int ship_get_SIF(int sh)
{
	return Ship_info[Ships[sh].ship_info_index].flags;
}

int ship_get_by_signature(int signature)
{
	ship_obj *so;
		
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {		
		// if we found a matching ship object signature
		if((Objects[so->objnum].signature == signature) && (Objects[so->objnum].type == OBJ_SHIP)){
			return Objects[so->objnum].instance;
		}
	}

	// couldn't find the ship
	return -1;
}

ship_type_info *ship_get_type_info(object *objp)
{
	Assert(objp != NULL);
	Assert(objp->type == OBJ_SHIP);
	Assert(objp->instance > -1);
	Assert(Ships[objp->instance].ship_info_index > -1);
	Assert(Ship_info[Ships[objp->instance].ship_info_index].class_type > -1);

	return &Ship_types[Ship_info[Ships[objp->instance].ship_info_index].class_type];
}

/**
 * Called when the cargo of a ship is revealed.
 *
 * Happens at two different locations (at least when this function was written), one for the player, and one for AI ships.
 * Need to send stuff to clients in multiplayer game.
 */
void ship_do_cargo_revealed( ship *shipp, int from_network )
{
	// don't do anything if we already know the cargo
	if ( shipp->flags & SF_CARGO_REVEALED ){
		return;
	}
	
	nprintf(("Network", "Revealing cargo for %s\n", shipp->ship_name));

	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		send_cargo_revealed_packet( shipp );		
	}

	shipp->flags |= SF_CARGO_REVEALED;
	shipp->time_cargo_revealed = Missiontime;	

	// if the cargo is something other than "nothing", then make a log entry
	if ( stricmp(Cargo_names[shipp->cargo1 & CARGO_INDEX_MASK], NOX("nothing")) ){
		mission_log_add_entry(LOG_CARGO_REVEALED, shipp->ship_name, NULL, (shipp->cargo1 & CARGO_INDEX_MASK) );
	}	
}

void ship_do_cap_subsys_cargo_revealed( ship *shipp, ship_subsys *subsys, int from_network )
{
	// don't do anything if we already know the cargo
	if (subsys->flags & SSF_CARGO_REVEALED) {
		return;
	}

	nprintf(("Network", "Revealing cap ship subsys cargo for %s\n", shipp->ship_name));

	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		int subsystem_index = ship_get_index_from_subsys(subsys, shipp->objnum);
		send_subsystem_cargo_revealed_packet( shipp, subsystem_index );		
	}

	subsys->flags |= SSF_CARGO_REVEALED;
	subsys->time_subsys_cargo_revealed = Missiontime;

	// if the cargo is something other than "nothing", then make a log entry
	if ( stricmp(Cargo_names[subsys->subsys_cargo_name & CARGO_INDEX_MASK], NOX("nothing")) ){
		mission_log_add_entry(LOG_CAP_SUBSYS_CARGO_REVEALED, shipp->ship_name, subsys->system_info->subobj_name, (subsys->subsys_cargo_name & CARGO_INDEX_MASK) );
	}	
}

/**
 * alled when the cargo of a ship is hidden by the sexp.  
 *
 * Need to send stuff to clients in multiplayer game.
 */
void ship_do_cargo_hidden( ship *shipp, int from_network )
{
	// don't do anything if the cargo is already hidden
	if ( !(shipp->flags & SF_CARGO_REVEALED) )
	{
		return;
	}
	
	nprintf(("Network", "Hiding cargo for %s\n", shipp->ship_name));

	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		send_cargo_hidden_packet( shipp );		
	}

	shipp->flags &= ~SF_CARGO_REVEALED;

	// don't log that the cargo was hidden and don't reset the time cargo revealed
}

void ship_do_cap_subsys_cargo_hidden( ship *shipp, ship_subsys *subsys, int from_network )
{
	// don't do anything if the cargo is already hidden
	if (!(subsys->flags & SSF_CARGO_REVEALED))
	{
		return;
	}

	nprintf(("Network", "Hiding cap ship subsys cargo for %s\n", shipp->ship_name));

	// send the packet if needed
	if ( (Game_mode & GM_MULTIPLAYER) && !from_network ){
		int subsystem_index = ship_get_index_from_subsys(subsys, shipp->objnum);
		send_subsystem_cargo_hidden_packet( shipp, subsystem_index );		
	}

	subsys->flags &= ~SSF_CARGO_REVEALED;

	// don't log that the cargo was hidden and don't reset the time cargo revealed
}

/**
 * Return the range of the currently selected secondary weapon
 *
 * NOTE: If there is no missiles left in the current bank, range returned is 0
 *
 * @param shipp Pointer to ship from which currently selected secondary weapon will be ranged
 */
float ship_get_secondary_weapon_range(ship *shipp)
{
	float srange=0.0f;

	ship_weapon	*swp;
	swp = &shipp->weapons;
	if ( swp->current_secondary_bank >= 0 ) {
		weapon_info	*wip;
		int bank=swp->current_secondary_bank;
		if (swp->secondary_bank_weapons[bank] >= 0) {
			wip = &Weapon_info[swp->secondary_bank_weapons[bank]];
			if ( swp->secondary_bank_ammo[bank] > 0 ) {
				srange = wip->max_speed * wip->lifetime;
			}
		}
	}

	return srange;
}

// Goober5000 - added for ballistic primaries
/**
 * Determine the number of primary ammo units allowed max for a ship
 */
int get_max_ammo_count_for_primary_bank(int ship_class, int bank, int ammo_type)
{
	float capacity, size;
	
	if (!(Ship_info[ship_class].flags & SIF_BALLISTIC_PRIMARIES))
	{
		return 0;
	}

	capacity = (float) Ship_info[ship_class].primary_bank_ammo_capacity[bank];
	size = (float) Weapon_info[ammo_type].cargo_size;
	return  fl2i((capacity / size)+0.5f);
}

/**
 * Determine the number of secondary ammo units (missile/bomb) allowed max for a ship
 */
int get_max_ammo_count_for_bank(int ship_class, int bank, int ammo_type)
{
	float capacity, size;

	Assertion(ship_class < static_cast<int>(Ship_info.size()), "Invalid ship_class of %d is >= Ship_info.size() (%d); get a coder!\n", ship_class, static_cast<int>(Ship_info.size()));
	Assertion(bank < MAX_SHIP_SECONDARY_BANKS, "Invalid secondary bank of %d (max is %d); get a coder!\n", bank, MAX_SHIP_SECONDARY_BANKS - 1);
	Assertion(ammo_type < Num_weapon_types, "Invalid ammo_type of %d is >= Num_weapon_types (%d); get a coder!\n", ammo_type, Num_weapon_types);

	if (ship_class < 0 || bank < 0 || ammo_type < 0) {
		return 0;
	} else {
		capacity = (float) Ship_info[ship_class].secondary_bank_ammo_capacity[bank];
		size = (float) Weapon_info[ammo_type].cargo_size;
		return (int) (capacity / size);
	}
}

/**
 * The same as above, but for a specific turret's bank.
 */
int get_max_ammo_count_for_turret_bank(ship_weapon *swp, int bank, int ammo_type)
{
	float capacity, size;

	Assertion(bank < MAX_SHIP_SECONDARY_BANKS, "Invalid secondary bank of %d (max is %d); get a coder!\n", bank, MAX_SHIP_SECONDARY_BANKS - 1);
	Assertion(ammo_type < Num_weapon_types, "Invalid ammo_type of %d is >= Num_weapon_types (%d); get a coder!\n", ammo_type, Num_weapon_types);

	if (!swp || bank < 0 || ammo_type < 0) {
		return 0;
	} else {
		capacity = (float) swp->secondary_bank_capacity[bank];
		size = (float) Weapon_info[ammo_type].cargo_size;
		return (int) (capacity / size);
	}
}

/**
 * Page in bitmaps for all the ships in this level
 */
void ship_page_in()
{
	int i, j, k;
	int num_subsystems_needed = 0;

	int *ship_class_used = NULL;

	ship_class_used = new int[Ship_info.size()];

	Verify( ship_class_used != NULL );

	// Mark all ship classes as not used
	memset( ship_class_used, 0, Ship_info.size() * sizeof(int) );

	// Mark any support ship types as used
	for (auto sip = Ship_info.begin(); sip != Ship_info.end(); ++sip) {
		if (sip->flags & SIF_SUPPORT) {
			nprintf(( "Paging", "Found support ship '%s'\n", sip->name ));
			i = std::distance(Ship_info.begin(), sip);
			ship_class_used[i]++;

			num_subsystems_needed += sip->n_subsystems;

			// load the darn model and page in textures
			sip->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);

			if (sip->model_num >= 0) {
				model_page_in_textures(sip->model_num, i);
			}
		}
	}
	
	// Mark any ships in the mission as used
	for (i = 0; i < MAX_SHIPS; i++)	{
		if (Ships[i].objnum < 0)
			continue;
	
		nprintf(( "Paging","Found ship '%s'\n", Ships[i].ship_name ));
		ship_class_used[Ships[i].ship_info_index]++;

		// check if we are going to use a Knossos device and make sure the special warp ani gets pre-loaded
		if ( Ship_info[Ships[i].ship_info_index].flags & SIF_KNOSSOS_DEVICE )
			Knossos_warp_ani_used = 1;

		// mark any weapons as being used, saves memory and time if we don't load them all
		ship_weapon *swp = &Ships[i].weapons;

		for (j = 0; j < swp->num_primary_banks; j++)
			weapon_mark_as_used(swp->primary_bank_weapons[j]);

		for (j = 0; j < swp->num_secondary_banks; j++)
			weapon_mark_as_used(swp->secondary_bank_weapons[j]);

		// get weapons for all capship subsystems (turrets)
		ship_subsys *ptr = GET_FIRST(&Ships[i].subsys_list);

		while (ptr != END_OF_LIST(&Ships[i].subsys_list)) {
			for (k = 0; k < MAX_SHIP_PRIMARY_BANKS; k++)
				weapon_mark_as_used(ptr->weapons.primary_bank_weapons[k]);

			for (k = 0; k < MAX_SHIP_SECONDARY_BANKS; k++)
				weapon_mark_as_used(ptr->weapons.secondary_bank_weapons[k]);

			ptr = GET_NEXT(ptr);
		}

		ship_info *sip = &Ship_info[Ships[i].ship_info_index];

		// page in all of the textures if the model is already loaded
		if (sip->model_num >= 0) {
			nprintf(( "Paging", "Paging in textures for ship '%s'\n", Ships[i].ship_name ));
			model_page_in_textures(sip->model_num, Ships[i].ship_info_index);
			// need to make sure and do this again, after we are sure that all of the textures are ready
			ship_init_afterburners( &Ships[i] );
		}

		//WMC - Since this is already in-mission, ignore the warpin effect.
		Ships[i].warpout_effect->pageIn();

		// don't need this one anymore, it's already been accounted for
	//	num_subsystems_needed += Ship_info[Ships[i].ship_info_index].n_subsystems;
	}

	// Mark any ships that might warp in in the future as used
	for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp)) {
		nprintf(( "Paging", "Found future arrival ship '%s'\n", p_objp->name ));
		ship_class_used[p_objp->ship_class]++;

		// This will go through Subsys_index[] and grab all weapons: primary, secondary, and turrets
		for (i = p_objp->subsys_index; i < (p_objp->subsys_index + p_objp->subsys_count); i++) {
			for (j = 0; j < MAX_SHIP_PRIMARY_BANKS; j++) {
				if (Subsys_status[i].primary_banks[j] >= 0)
					weapon_mark_as_used(Subsys_status[i].primary_banks[j]);
			}

			for (j = 0; j < MAX_SHIP_SECONDARY_BANKS; j++) {
				if (Subsys_status[i].secondary_banks[j] >= 0)
					weapon_mark_as_used(Subsys_status[i].secondary_banks[j]);
			}
		}

		// page in any replacement textures
		if (Ship_info[p_objp->ship_class].model_num >= 0) {
			nprintf(( "Paging", "Paging in textures for future arrival ship '%s'\n", p_objp->name ));
			model_page_in_textures(Ship_info[p_objp->ship_class].model_num, p_objp->ship_class);
		}

		num_subsystems_needed += Ship_info[p_objp->ship_class].n_subsystems;
	}

	// pre-allocate the subsystems, this really only needs to happen for ships
	// which don't exist yet (ie, ships NOT in Ships[])
	if (!ship_allocate_subsystems(num_subsystems_needed, true)) {
		Error(LOCATION, "Attempt to page in new subsystems subsystems failed because mission file contains more than %d subsystems", (NUM_SHIP_SUBSYSTEM_SETS* NUM_SHIP_SUBSYSTEMS_PER_SET)); 
	}

	mprintf(("About to page in ships!\n"));

	// Page in all the ship classes that are used on this level
	int num_ship_types_used = 0;
	int test_id __UNUSED = -1;

	memset( fireball_used, 0, sizeof(int) * MAX_FIREBALL_TYPES );

	i = 0;
	for (auto sip = Ship_info.begin(); sip != Ship_info.end(); i++, ++sip) {
		if ( !ship_class_used[i] )
			continue;

		int model_previously_loaded = -1;
		int ship_previously_loaded = -1;

		num_ship_types_used++;

		// Page in the small hud icons for each ship
		hud_ship_icon_page_in(&(*sip));

		// See if this model was previously loaded by another ship
		for (auto it = Ship_info.begin(); it != Ship_info.end(); ++it) {
			if ( (it->model_num > -1) && !stricmp(sip->pof_file, it->pof_file) ) {
				// Model already loaded
				model_previously_loaded = it->model_num;

				if ((sip->n_subsystems > 0) && (sip->subsystems[0].model_num > -1)) {
					ship_previously_loaded = std::distance(Ship_info.begin(), it);

					// It is possible in some cases for sip->model_num to change, and for subsystems->model_num
					// to still point to the old model index; this makes sure it doesn't happen. -zookeeper
					for (k = 0; k < sip->n_subsystems; k++) {
						if (sip->model_num != sip->subsystems[k].model_num) {
							mprintf(("Ship %s has model_num %i but its subsystem %s has model_num %i, fixing...\n", sip->name, sip->model_num, sip->subsystems[k].name, sip->subsystems[k].model_num));
							sip->subsystems[k].model_num = sip->model_num;
						}
					}
				}

				// the model should already be loaded so this wouldn't take long, but
				// we need to make sure that the load count for the model is correct
				test_id = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
				Assert( test_id == model_previously_loaded );

				break;
			}
		}

		// If the model is previously loaded...
		if (model_previously_loaded >= 0) {
			// If previously loaded model isn't the same ship class...)
			if (ship_previously_loaded != i) {
				// update the model number.
				sip->model_num = model_previously_loaded;

				for (j = 0; j < sip->n_subsystems; j++)
					sip->subsystems[j].model_num = -1;

				ship_copy_subsystem_fixup(&(*sip));

#ifndef NDEBUG
				for (j = 0; j < sip->n_subsystems; j++) {
					if (sip->subsystems[j].model_num != sip->model_num) {
						polymodel *sip_pm = (sip->model_num >= 0) ? model_get(sip->model_num) : NULL;
						polymodel *subsys_pm = (sip->subsystems[j].model_num >= 0) ? model_get(sip->subsystems[j].model_num) : NULL;
						Warning(LOCATION, "After ship_copy_subsystem_fixup, ship '%s' does not have subsystem '%s' linked into the model file, '%s'.\n\n(Ship_info model is '%s' and subsystem model is '%s'.)", sip->name, sip->subsystems[j].subobj_name, sip->pof_file, (sip_pm != NULL) ? sip_pm->filename : "NULL", (subsys_pm != NULL) ? subsys_pm->filename : "NULL");
					}
				}
#endif
			} else {
				// Just to be safe (I mean to check that my code works...)
				Assert( sip->model_num >= 0 );
				Assert( sip->model_num == model_previously_loaded );

#ifndef NDEBUG
				for (j = 0; j < sip->n_subsystems; j++) {
					if (sip->subsystems[j].model_num != sip->model_num) {
						polymodel *sip_pm = (sip->model_num >= 0) ? model_get(sip->model_num) : NULL;
						polymodel *subsys_pm = (sip->subsystems[j].model_num >= 0) ? model_get(sip->subsystems[j].model_num) : NULL;
						Warning(LOCATION, "Without ship_copy_subsystem_fixup, ship '%s' does not have subsystem '%s' linked into the model file, '%s'.\n\n(Ship_info model is '%s' and subsystem model is '%s'.)", sip->name, sip->subsystems[j].subobj_name, sip->pof_file, (sip_pm != NULL) ? sip_pm->filename : "NULL", (subsys_pm != NULL) ? subsys_pm->filename : "NULL");
					}
				}
#endif
			}
		} else {
			// Model not loaded, so load it
			sip->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);

			Assert( sip->model_num >= 0 );

#ifndef NDEBUG
			// Verify that all the subsystem model numbers are updated
			for (j = 0; j < sip->n_subsystems; j++)
				Assertion( sip->subsystems[j].model_num == sip->model_num, "Model reference for subsystem %s (model num: %d) on model %s (model num: %d) is invalid.\n", sip->subsystems[j].name, sip->subsystems[j].model_num, sip->pof_file, sip->model_num );	// JAS
#endif
		}

		// more weapon marking, the weapon info in Ship_info[] is the default
		// loadout which isn't specified by missionparse unless it's different
		for (j = 0; j < sip->num_primary_banks; j++)
			weapon_mark_as_used(sip->primary_bank_weapons[j]);

		for (j = 0; j < sip->num_secondary_banks; j++)
			weapon_mark_as_used(sip->secondary_bank_weapons[j]);

		weapon_mark_as_used(sip->cmeasure_type);

		for (j = 0; j < sip->n_subsystems; j++) {
			model_subsystem *msp = &sip->subsystems[j];

			for (k = 0; k < MAX_SHIP_PRIMARY_BANKS; k++)
				weapon_mark_as_used(msp->primary_banks[k]);

			for (k = 0; k < MAX_SHIP_SECONDARY_BANKS; k++)
				weapon_mark_as_used(msp->secondary_banks[k]);
		}

		// Page in the shockwave stuff. -C
		shockwave_create_info_load(&sip->shockwave);
		if(sip->explosion_bitmap_anims.size() > 0) {
			int num_fireballs = sip->explosion_bitmap_anims.size();
			for(j = 0; j < num_fireballs; j++){
				fireball_used[sip->explosion_bitmap_anims[j]] = 1;
			}
		} else if(sip->class_type >= 0 && Ship_types[sip->class_type].explosion_bitmap_anims.size() > 0) { 
			int num_fireballs = Ship_types[sip->class_type].explosion_bitmap_anims.size();
			for(j = 0; j < num_fireballs; j++){
				fireball_used[Ship_types[sip->class_type].explosion_bitmap_anims[j]] = 1;
			}
		}
	}

	nprintf(( "Paging", "There are %d ship classes used in this mission.\n", num_ship_types_used ));


	// Page in the thruster effects
	// Make sure thrusters are loaded
	if (!Thrust_anim_inited)
		ship_init_thrusters();

	thrust_info *thruster;
	for (i = 0; i < (int)Species_info.size(); i++) {
		thruster = &Species_info[i].thruster_info;

		bm_page_in_texture( thruster->flames.normal.first_frame, thruster->flames.normal.num_frames );
		bm_page_in_texture( thruster->flames.afterburn.first_frame, thruster->flames.afterburn.num_frames );

		// glows are really not anims
		bm_page_in_texture( thruster->glow.normal.first_frame );
		bm_page_in_texture( thruster->glow.afterburn.first_frame );
	}

	// page in insignia bitmaps
	if(Game_mode & GM_MULTIPLAYER){
		for(i=0; i<MAX_PLAYERS; i++){
			if(MULTI_CONNECTED(Net_players[i]) && (Net_players[i].m_player != NULL) && (Net_players[i].m_player->insignia_texture >= 0)){
				bm_page_in_xparent_texture(Net_players[i].m_player->insignia_texture);
			}
		}
	} else {
		if((Player != NULL) && (Player->insignia_texture >= 0)){
			bm_page_in_xparent_texture(Player->insignia_texture);
		}
	}

	// page in wing insignia bitmaps - Goober5000
	for (i = 0; i < MAX_WINGS; i++)
	{
		if (Wings[i].wing_insignia_texture >= 0)
			bm_page_in_xparent_texture(Wings[i].wing_insignia_texture);
	}

	// page in replacement textures - Goober5000
	for (i = 0; i < MAX_SHIPS; i++)
	{
		// is this a valid ship?
		if (Ships[i].objnum >= 0)
		{
			// do we have any textures?
			if (Ships[i].ship_replacement_textures != NULL)
			{
				// page in replacement textures
				for (j=0; j<MAX_REPLACEMENT_TEXTURES; j++)
				{
					if (Ships[i].ship_replacement_textures[j] > -1)
					{
						bm_page_in_texture( Ships[i].ship_replacement_textures[j] );
					}
				}
			}
		}
	}

	// should never be NULL, this entire function wouldn't work
	delete[] ship_class_used;
	ship_class_used = NULL;

}

// Goober5000 - called from ship_page_in()
void ship_page_in_textures(int ship_index)
{
	int i;
	ship_info *sip;

	if ( (ship_index < 0) || (ship_index >= static_cast<int>(Ship_info.size())) )
		return;


	sip = &Ship_info[ship_index];

	// afterburner
	if ( !generic_bitmap_load(&sip->afterburner_trail) )
		bm_page_in_texture(sip->afterburner_trail.bitmap_id);

	// Wanderer - just copying over Bobboau's code...
	if ( !generic_anim_load(&sip->thruster_flame_info.normal) )
		bm_page_in_texture(sip->thruster_flame_info.normal.first_frame);

	if ( !generic_anim_load(&sip->thruster_flame_info.afterburn) )
		bm_page_in_texture(sip->thruster_flame_info.afterburn.first_frame);

	// Bobboau's thruster bitmaps
	// the first set has to be loaded a special way
	if ( !thruster_glow_anim_load(&sip->thruster_glow_info.normal) )
		bm_page_in_texture(sip->thruster_glow_info.normal.first_frame);

	if ( !thruster_glow_anim_load(&sip->thruster_glow_info.afterburn) )
		bm_page_in_texture(sip->thruster_glow_info.afterburn.first_frame);

	// everything else is loaded normally
	if ( !generic_bitmap_load(&sip->thruster_secondary_glow_info.normal) )
		bm_page_in_texture(sip->thruster_secondary_glow_info.normal.bitmap_id);

	if ( !generic_bitmap_load(&sip->thruster_secondary_glow_info.afterburn) )
		bm_page_in_texture(sip->thruster_secondary_glow_info.afterburn.bitmap_id);

	if ( !generic_bitmap_load(&sip->thruster_tertiary_glow_info.normal) )
		bm_page_in_texture(sip->thruster_tertiary_glow_info.normal.bitmap_id);

	if ( !generic_bitmap_load(&sip->thruster_tertiary_glow_info.afterburn) )
		bm_page_in_texture(sip->thruster_tertiary_glow_info.afterburn.bitmap_id);
 
	// splodeing bitmap
	if ( VALID_FNAME(sip->splodeing_texture_name) ) {
		sip->splodeing_texture = bm_load(sip->splodeing_texture_name);
		bm_page_in_texture(sip->splodeing_texture);
	}

	// thruster/particle bitmaps
	for (i = 0; i < (int)sip->normal_thruster_particles.size(); i++) {
		generic_anim_load(&sip->normal_thruster_particles[i].thruster_bitmap);
		bm_page_in_texture(sip->normal_thruster_particles[i].thruster_bitmap.first_frame);
	}

	for (i = 0; i < (int)sip->afterburner_thruster_particles.size(); i++) {
		generic_anim_load(&sip->afterburner_thruster_particles[i].thruster_bitmap);
		bm_page_in_texture(sip->afterburner_thruster_particles[i].thruster_bitmap.first_frame);
	}
}


#define PAGE_OUT_TEXTURE(x) {	\
	if ( (x) >= 0 ) {	\
		if (release) {	\
			bm_release( (x) );	\
			(x) = -1;	\
		} else {	\
			bm_unload( (x) );	\
		}	\
	}	\
}

/**
 * Unload all textures for a given ship
 */
void ship_page_out_textures(int ship_index, bool release)
{
	int i;
	ship_info *sip;

	if ( (ship_index < 0) || (ship_index >= static_cast<int>(Ship_info.size())) )
		return;


	sip = &Ship_info[ship_index];

	// afterburner
	PAGE_OUT_TEXTURE(sip->afterburner_trail.bitmap_id);

	// thruster bitmaps
	PAGE_OUT_TEXTURE(sip->thruster_flame_info.normal.first_frame);
	PAGE_OUT_TEXTURE(sip->thruster_flame_info.afterburn.first_frame);
	PAGE_OUT_TEXTURE(sip->thruster_glow_info.normal.first_frame);
	PAGE_OUT_TEXTURE(sip->thruster_glow_info.afterburn.first_frame);
	PAGE_OUT_TEXTURE(sip->thruster_secondary_glow_info.normal.bitmap_id);
	PAGE_OUT_TEXTURE(sip->thruster_secondary_glow_info.afterburn.bitmap_id);
	PAGE_OUT_TEXTURE(sip->thruster_tertiary_glow_info.normal.bitmap_id);
	PAGE_OUT_TEXTURE(sip->thruster_tertiary_glow_info.afterburn.bitmap_id);

	// slodeing bitmap
	PAGE_OUT_TEXTURE(sip->splodeing_texture);

	// thruster/particle bitmaps
	for (i = 0; i < (int)sip->normal_thruster_particles.size(); i++)
		PAGE_OUT_TEXTURE(sip->normal_thruster_particles[i].thruster_bitmap.first_frame);

	for (i = 0; i < (int)sip->afterburner_thruster_particles.size(); i++)
		PAGE_OUT_TEXTURE(sip->afterburner_thruster_particles[i].thruster_bitmap.first_frame);
}

// function to return true if support ships are allowed in the mission for the given object.
//	In single player, must be friendly and not Shivan. (Goober5000 - Shivans can now have support)
//	In multiplayer -- to be coded by Mark Allender after 5/4/98 -- MK, 5/4/98
int is_support_allowed(object *objp, bool do_simple_check)
{
	int result = -1;

	// check updated mission conditions to allow support

	// If running under autopilot support is not allowed
	if ( AutoPilotEngaged )
		return 0;

	// none allowed
	if (The_mission.support_ships.max_support_ships == 0)
		return 0;

	// ship_find_repair_ship is a little expensive, so let's not do it every frame
	if (!do_simple_check)
	{
		// check if all support ships are departing or dying
		result = ship_find_repair_ship(objp);
		if (result == 4) {
			return 0;
		}

		// restricted number allowed
		if (The_mission.support_ships.max_support_ships > 0)
		{
			// if all the allowed ships have been used up, can't rearm unless something's available in-mission or arriving
			if ((The_mission.support_ships.tally >= The_mission.support_ships.max_support_ships))
			{
				// this shouldn't happen because we've reached one of the limits
				Assert(result != 2);

				// nothing arriving and no ships available in mission
				if ((Arriving_support_ship == NULL) && (result == 0 || result == 3))
					return 0;
			}
		}
	}

	ship *shipp = &Ships[objp->instance];

	// this also looks a little more expensive
	if (!do_simple_check)
	{
		// make sure, if exiting from bay, that parent ship is in the mission!
		if ((result == 0 || result == 2) && (The_mission.support_ships.arrival_location == ARRIVE_FROM_DOCK_BAY))
		{
			Assert(The_mission.support_ships.arrival_anchor != -1);

			// ensure it's in-mission
			int temp = ship_name_lookup(Parse_names[The_mission.support_ships.arrival_anchor]);
			if (temp < 0)
			{
				return 0;
			}

			// make sure it's not leaving or blowing up
			if (Ships[temp].flags & (SF_DYING | SF_DEPARTING))
			{
				return 0;
			}

			// also make sure that parent ship's fighterbay hasn't been destroyed
			if (ship_fighterbays_all_destroyed(&Ships[temp]))
			{
				return 0;
			}
		}
	}

	if (Game_mode & GM_NORMAL)
	{
		if ( !(Iff_info[shipp->team].flags & IFFF_SUPPORT_ALLOWED) )
		{
			return 0;
		}
	}
	else
	{
		// multiplayer version behaves differently.  Depending on mode:
		// 1) coop mode -- only available to friendly
		// 2) team v team mode -- availble to either side
		// 3) dogfight -- never

		if(Netgame.type_flags & NG_TYPE_DOGFIGHT)
		{
			return 0;
		}

		if (IS_MISSION_MULTI_COOP)
		{
			if ( !(Iff_info[shipp->team].flags & IFFF_SUPPORT_ALLOWED) )
			{
				return 0;
			}
		}
	}

	// Goober5000 - extra check for existence of support ship
	if ( (The_mission.support_ships.ship_class < 0) &&
		!(The_mission.support_ships.support_available_for_species & (1 << Ship_info[shipp->ship_info_index].species)) )
	{
		return 0;
	}

	// this is also somewhat expensive
	if (!do_simple_check)
	{
		// Goober5000 - extra check to make sure this guy has a rearming dockpoint
		if (model_find_dock_index(Ship_info[shipp->ship_info_index].model_num, DOCK_TYPE_REARM) < 0)
		{
			static bool warned_about_rearm_dockpoint = false;
			if (!warned_about_rearm_dockpoint)
			{
				Warning(LOCATION, "Support not allowed for %s because its model lacks a rearming dockpoint!", shipp->ship_name);
				warned_about_rearm_dockpoint = true;
			}
			return 0;
		}
	}

	// Goober5000 - if we got this far, we can request support
	return 1;
}

// returns random index of a visible ship
// if no visible ships are generated in num_ships iterations, it returns -1
int ship_get_random_targetable_ship()
{
	int rand_ship;
	int idx = 0, target_list[MAX_SHIPS];
	ship_obj *so;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		// make sure the instance is valid
		if ( (Objects[so->objnum].instance < 0) || (Objects[so->objnum].instance >= MAX_SHIPS) )
			continue;

		// skip if we aren't supposed to target it
		if ( Ships[Objects[so->objnum].instance].flags & TARGET_SHIP_IGNORE_FLAGS )
			continue;

		if (idx >= MAX_SHIPS) {
			idx = MAX_SHIPS;
			break;
		}

		target_list[idx] = Objects[so->objnum].instance;
		idx++;
	}

	if (idx == 0)
		return -1;

	rand_ship = (rand() % idx);

	return target_list[rand_ship];
}

/**
 * Forcible jettison cargo from a ship
 */
void object_jettison_cargo(object *objp, object *cargo_objp)
{
	// make sure we are docked
	Assert((objp != NULL) && (cargo_objp != NULL));
	Assert(dock_check_find_direct_docked_object(objp, cargo_objp));

	vec3d impulse, pos;
	ship *shipp = &Ships[objp->instance];
	ship *cargo_shipp = &Ships[cargo_objp->instance];
	int docker_index = dock_find_dockpoint_used_by_object(objp, cargo_objp);
	int dockee_index = dock_find_dockpoint_used_by_object(cargo_objp, objp);

	// undo all the docking animations
	model_anim_start_type(shipp, TRIGGER_TYPE_DOCKED, docker_index, -1);
	model_anim_start_type(shipp, TRIGGER_TYPE_DOCKING_STAGE_3, docker_index, -1);
	model_anim_start_type(shipp, TRIGGER_TYPE_DOCKING_STAGE_2, docker_index, -1);
	model_anim_start_type(shipp, TRIGGER_TYPE_DOCKING_STAGE_1, docker_index, -1);
	model_anim_start_type(cargo_shipp, TRIGGER_TYPE_DOCKED, dockee_index, -1);
	model_anim_start_type(cargo_shipp, TRIGGER_TYPE_DOCKING_STAGE_3, dockee_index, -1);
	model_anim_start_type(cargo_shipp, TRIGGER_TYPE_DOCKING_STAGE_2, dockee_index, -1);
	model_anim_start_type(cargo_shipp, TRIGGER_TYPE_DOCKING_STAGE_1, dockee_index, -1);

	// undock the objects
	ai_do_objects_undocked_stuff(objp, cargo_objp);

	// Goober5000 - add log
	mission_log_add_entry(LOG_SHIP_UNDOCKED, shipp->ship_name, cargo_shipp->ship_name);

	// physics stuff
	vm_vec_sub(&pos, &cargo_objp->pos, &objp->pos);
	impulse = pos;
	vm_vec_scale(&impulse, 100.0f);
	vm_vec_normalize(&pos);

	// whack the ship
	physics_apply_whack(&impulse, &pos, &cargo_objp->phys_info, &cargo_objp->orient, cargo_objp->phys_info.mass);
}

float ship_get_exp_damage(object* objp)
{
	Assert(objp->type == OBJ_SHIP);
	float damage; 

	ship *shipp = &Ships[objp->instance];

	if (shipp->special_exp_damage >= 0) {
		damage = i2fl(shipp->special_exp_damage);
	} else {
		damage = Ship_info[shipp->ship_info_index].shockwave.damage;
	}

	return damage;
}

int ship_get_exp_propagates(ship *sp)
{
	return Ship_info[sp->ship_info_index].explosion_propagates;
}

float ship_get_exp_outer_rad(object *ship_objp)
{
	float outer_rad;
	Assert(ship_objp->type == OBJ_SHIP);

	if (Ships[ship_objp->instance].special_exp_outer == -1) {
		outer_rad = Ship_info[Ships[ship_objp->instance].ship_info_index].shockwave.outer_rad;
	} else {
		outer_rad = (float)Ships[ship_objp->instance].special_exp_outer;
	}

	return outer_rad;
}

int valid_cap_subsys_cargo_list(char *subsys)
{
	// Return 1 for all subsystems now, due to Mantis #2524.
	return 1;
	/*
	if (stristr(subsys, "nav")
		|| stristr(subsys, "comm")
		|| stristr(subsys, "engine")
		|| stristr(subsys, "fighter")	// fighter bays
		|| stristr(subsys, "sensors")
		|| stristr(subsys, "weapons")) {

		return 1;
	}

	return 0;
	*/
}

/**
 * Determine turret status of a given subsystem
 *
 * @return 0 for no turret, 1 for "fixed turret", 2 for "rotating" turret
 */
int ship_get_turret_type(ship_subsys *subsys)
{
	// not a turret at all
	if(subsys->system_info->type != SUBSYSTEM_TURRET){
		return 0;
	}

	// if it rotates
	if(subsys->system_info->turret_turning_rate > 0.0f){
		return 2;
	}

	// if its fixed
	return 1;
}

ship_subsys *ship_get_subsys(ship *shipp, char *subsys_name)
{
	// sanity checks
	if ((shipp == NULL) || (subsys_name == NULL)) {
		return NULL;
	}

	ship_subsys *ss = GET_FIRST(&shipp->subsys_list);
	while (ss != END_OF_LIST(&shipp->subsys_list)) {
		// check subsystem name
		if (!subsystem_stricmp(ss->system_info->subobj_name, subsys_name)) {
			return ss;
		}

		// next
		ss = GET_NEXT(ss);
	}

	// didn't find it
	return NULL;
}

int ship_get_num_subsys(ship *shipp)
{
	Assert(shipp != NULL);

	return Ship_info[shipp->ship_info_index].n_subsystems;
}

// returns 0 if no conflict, 1 if conflict, -1 on some kind of error with wing struct
int wing_has_conflicting_teams(int wing_index)
{
	int first_team, idx;

	// sanity checks
	Assert((wing_index >= 0) && (wing_index < Num_wings) && (Wings[wing_index].wave_count > 0));
	if((wing_index < 0) || (wing_index >= Num_wings) || (Wings[wing_index].wave_count <= 0)){
		return -1;
	}

	// check teams
	Assert(Wings[wing_index].ship_index[0] >= 0);
	if(Wings[wing_index].ship_index[0] < 0){
		return -1;
	}
	first_team = Ships[Wings[wing_index].ship_index[0]].team;
	for(idx=1; idx<Wings[wing_index].wave_count; idx++){
		// more sanity checks
		Assert(Wings[wing_index].ship_index[idx] >= 0);
		if(Wings[wing_index].ship_index[idx] < 0){
			return -1;
		}

		// if we've got a team conflict
		if(first_team != Ships[Wings[wing_index].ship_index[idx]].team){
			return 1;
		}
	}

	// no conflict
	return 0;
}

/**
 * Get the team of a reinforcement item
 */
int ship_get_reinforcement_team(int r_index)
{
	int wing_index;
	p_object *p_objp;

	// sanity checks
	Assert((r_index >= 0) && (r_index < Num_reinforcements));
	if ((r_index < 0) || (r_index >= Num_reinforcements))
		return -1;

	// if the reinforcement is a ship	
	p_objp = mission_parse_get_arrival_ship(Reinforcements[r_index].name);
	if (p_objp != NULL)
		return p_objp->team;

	// if the reinforcement is a ship
	wing_index = wing_lookup(Reinforcements[r_index].name);
	if (wing_index >= 0)
	{		
		// go through the ship arrival list and find any ship in this wing
		for (p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
		{
			// check by wingnum			
			if (p_objp->wingnum == wing_index)
				return p_objp->team;
		}
	}

	// no team ?
	return -1;
}

/**
 * Determine if the given texture is used by a ship type. return ship info index, or -1 if not used by a ship
 */
int ship_get_texture(int bitmap)
{
	// check all ship types
	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
		if ((it->model_num >= 0) && model_find_texture(it->model_num, bitmap) == 1) {
			return std::distance(Ship_info.cbegin(), it);
		}
	}

	// couldn't find the texture
	return -1;
}

// update artillery lock info
#define CLEAR_ARTILLERY_AND_CONTINUE()	{ if(aip != NULL){ aip->artillery_objnum = -1; aip->artillery_sig = -1;	aip->artillery_lock_time = 0.0f;} continue; } 
float artillery_dist = 10.0f;
DCF(art, "Sets artillery disance")
{
	dc_stuff_float(&artillery_dist);
}

void ship_update_artillery_lock()
{
	ai_info *aip = NULL;
	mc_info *cinfo = NULL;
	int c_objnum;
	vec3d temp, local_hit;
	ship *shipp;
	ship_obj *so;

	// update all ships
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ){
		// get the ship
		if((so->objnum >= 0) && (Objects[so->objnum].type == OBJ_SHIP) && (Objects[so->objnum].instance >= 0)){
			shipp = &Ships[Objects[so->objnum].instance];
		} else {
			continue;
		}		

		// get ai info
		if(shipp->ai_index >= 0){
			aip = &Ai_info[shipp->ai_index];
		}

		// if the ship has no targeting laser firing
		if((shipp->targeting_laser_objnum < 0) || (shipp->targeting_laser_bank < 0)){
			CLEAR_ARTILLERY_AND_CONTINUE();
		}

		// if he didn't hit any objects this frame
		if(beam_get_num_collisions(shipp->targeting_laser_objnum) <= 0){
			CLEAR_ARTILLERY_AND_CONTINUE();
		}

		// get weapon info for the targeting laser he's firing
		Assert((shipp->weapons.current_primary_bank >= 0) && (shipp->weapons.current_primary_bank < 2));
		if((shipp->weapons.current_primary_bank < 0) || (shipp->weapons.current_primary_bank >= 2)){
			continue;
		}
		Assert(shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank] >= 0);
		if(shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank] < 0){
			continue;
		}
		Assert((Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].wi_flags & WIF_BEAM) && (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].b_info.beam_type == BEAM_TYPE_C));
		if(!(Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].wi_flags & WIF_BEAM) || (Weapon_info[shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank]].b_info.beam_type != BEAM_TYPE_C)){
			continue;
		}	

		// get collision info
		if(!beam_get_collision(shipp->targeting_laser_objnum, 0, &c_objnum, &cinfo)){
			CLEAR_ARTILLERY_AND_CONTINUE();
		}
		if((c_objnum < 0) || (cinfo == NULL)){
			CLEAR_ARTILLERY_AND_CONTINUE();
		}

		// get the position we hit this guy with in his local coords
		vm_vec_sub(&temp, &cinfo->hit_point_world, &Objects[c_objnum].pos);
		vm_vec_rotate(&local_hit, &temp, &Objects[c_objnum].orient);

		// if we are hitting a different guy now, reset the lock
		if((c_objnum != aip->artillery_objnum) || (Objects[c_objnum].signature != aip->artillery_sig)){
			aip->artillery_objnum = c_objnum;
			aip->artillery_sig = Objects[c_objnum].signature;
			aip->artillery_lock_time = 0.0f;
			aip->artillery_lock_pos = local_hit;

			// done
			continue;
		}	

		// otherwise we're hitting the same guy. check to see if we've strayed too far
		if(vm_vec_dist_quick(&local_hit, &aip->artillery_lock_pos) > artillery_dist){
			// hmmm. reset lock time, but don't reset the lock itself
			aip->artillery_lock_time = 0.0f;
			continue;
		}

		// finally - just increment the lock time
		aip->artillery_lock_time += flFrametime;

		// TEST CODE
		if(aip->artillery_objnum >= 0 && aip->artillery_lock_time >= 2.0f){
			ssm_create(&Objects[aip->artillery_objnum], &cinfo->hit_point_world, 0, NULL, shipp->team);				

			// reset the artillery			
			aip->artillery_lock_time = 0.0f;			
		}
	}
}

/**
 * Checks if a world point is inside the extended bounding box of a ship
 *
 * May not work if delta box is large and negative (ie, adjusted box crosses over on itself - min > max)
 */
int check_world_pt_in_expanded_ship_bbox(vec3d *world_pt, object *objp, float delta_box)
{
	Assert(objp->type == OBJ_SHIP);

	vec3d temp, ship_pt;
	polymodel *pm;
	vm_vec_sub(&temp, world_pt, &objp->pos);
	vm_vec_rotate(&ship_pt, &temp, &objp->orient);

	pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);

	return (
			(ship_pt.xyz.x > pm->mins.xyz.x - delta_box) && (ship_pt.xyz.x < pm->maxs.xyz.x + delta_box)
		&& (ship_pt.xyz.y > pm->mins.xyz.y - delta_box) && (ship_pt.xyz.y < pm->maxs.xyz.y + delta_box)
		&& (ship_pt.xyz.z > pm->mins.xyz.z - delta_box) && (ship_pt.xyz.z < pm->maxs.xyz.z + delta_box)
	);
}


/**
 * Returns true when objp is ship and is tagged
 */
int ship_is_tagged(object *objp)
{
	ship *shipp;
	if (objp->type == OBJ_SHIP) {
		shipp = &Ships[objp->instance];
		if ( (shipp->tag_left > 0) || (shipp->level2_tag_left > 0) ) {
			return 1;
		}
	}

	return 0;
}

/**
 * Get maximum ship speed (when not warping in or out)
 */
float ship_get_max_speed(ship *shipp)
{
	float max_speed;
	ship_info *sip = &Ship_info[shipp->ship_info_index];

	// Goober5000 - maybe we're using cap-waypoint-speed
	ai_info *aip = &Ai_info[shipp->ai_index];
	if ((aip->mode == AIM_WAYPOINTS || aip->mode == AIM_FLY_TO_SHIP) && aip->waypoint_speed_cap > 0)
		return i2fl(aip->waypoint_speed_cap);

	// max overclock
	max_speed = sip->max_overclocked_speed;

	// normal max speed
	max_speed = MAX(max_speed, sip->max_vel.xyz.z);

	// afterburn if not locked
	if (!(shipp->flags2 & SF2_AFTERBURNER_LOCKED)) {
		max_speed = MAX(max_speed, sip->afterburner_max_vel.xyz.z);
	}

	return max_speed;
}

/**
 * Determine warpout speed of ship
 */
float ship_get_warpout_speed(object *objp)
{
	Assert(objp->type == OBJ_SHIP);

	ship_info *sip = &Ship_info[Ships[objp->instance].ship_info_index];
	//WMC - Any speed is good for in place anims (aka BSG FTL effect)
	if(sip->warpout_type == WT_IN_PLACE_ANIM && sip->warpout_speed <= 0.0f)
	{
		return objp->phys_info.speed;
	}
	else if(sip->warpout_type == WT_SWEEPER || sip->warpout_type == WT_IN_PLACE_ANIM)
	{
		return sip->warpout_speed;
	}
	else if(sip->warpout_type == WT_HYPERSPACE)
	{
		if (objp->phys_info.speed > sip->warpout_speed)
			return objp->phys_info.speed;
		else
			return sip->warpout_speed;
	}

	return shipfx_calculate_warp_dist(objp) / shipfx_calculate_warp_time(objp, WD_WARP_OUT);
}

/**
 * Returns true if ship is beginning to speed up in warpout
 */
int ship_is_beginning_warpout_speedup(object *objp)
{
	Assert(objp->type == OBJ_SHIP);

	ai_info *aip;

	aip = &Ai_info[Ships[objp->instance].ai_index];

	if (aip->mode == AIM_WARP_OUT) {
		if ( (aip->submode == AIS_WARP_3) || (aip->submode == AIS_WARP_4) || (aip->submode == AIS_WARP_5) ) {
			return 1;
		}
	}

	return 0;
}

/**
 * Return the length of a ship
 */
float ship_class_get_length(ship_info *sip)
{
	Assert(sip->model_num >= 0);
	polymodel *pm = model_get(sip->model_num);
	return (pm->maxs.xyz.z - pm->mins.xyz.z);
}

// Goober5000
void ship_set_new_ai_class(int ship_num, int new_ai_class)
{
	Assert(ship_num >= 0 && ship_num < MAX_SHIPS);
	Assert(new_ai_class >= 0);

	ai_info *aip = &Ai_info[Ships[ship_num].ai_index];

	// we hafta change a bunch of stuff here...
	aip->ai_class = new_ai_class;
	aip->behavior = AIM_NONE;
	init_aip_from_class_and_profile(aip, &Ai_classes[new_ai_class], The_mission.ai_profile);

	Ships[ship_num].weapons.ai_class = new_ai_class;

	// I think that's everything!
}

// Goober5000
void ship_subsystem_set_new_ai_class(int ship_num, char *subsystem, int new_ai_class)
{
	Assert(ship_num >= 0 && ship_num < MAX_SHIPS);
	Assert(subsystem);
	Assert(new_ai_class >= 0);

	ship_subsys *ss;

	// find the ship subsystem by searching ship's subsys_list
	ss = GET_FIRST( &Ships[ship_num].subsys_list );
	while ( ss != END_OF_LIST( &Ships[ship_num].subsys_list ) )
	{
		// if we found the subsystem
		if ( !subsystem_stricmp(ss->system_info->subobj_name, subsystem))
		{
			// set ai class
			ss->weapons.ai_class = new_ai_class;
			return;
		}

		ss = GET_NEXT( ss );
	}
}

// Goober5000 - will attempt to load an insignia bitmap and set it as active for the wing
// copied more or less from managepilot.cpp
void wing_load_squad_bitmap(wing *w)
{
	// sanity check
	if(w == NULL)
	{
		return;
	}

	// make sure one is not already set?!?
	Assert (w->wing_insignia_texture == -1);

	// try and set the new one
	if( w->wing_squad_filename[0] != '\0' )
	{
		// load duplicate because it might be the same as the player's squad,
		// and we don't want to overlap and breed nasty errors when we unload
		w->wing_insignia_texture = bm_load_duplicate(w->wing_squad_filename);
		
		// lock is as a transparent texture
		if(w->wing_insignia_texture != -1)
		{
			bm_lock(w->wing_insignia_texture, 16, BMP_TEX_XPARENT);
			bm_unlock(w->wing_insignia_texture);
		}
	}
}

// Goober5000 - needed by new hangar depart code
// check whether this ship has a docking bay
bool ship_has_dock_bay(int shipnum)
{
	Assert(shipnum >= 0 && shipnum < MAX_SHIPS);

	polymodel *pm;
				
	pm = model_get( Ship_info[Ships[shipnum].ship_info_index].model_num );
	Assert( pm );

	return ( pm->ship_bay && (pm->ship_bay->num_paths > 0) );
}

// Goober5000
bool ship_useful_for_departure(int shipnum, int path_mask)
{
	Assert( shipnum >= 0 && shipnum < MAX_SHIPS );

	// not valid if dying or departing
	if (Ships[shipnum].flags & (SF_DYING | SF_DEPARTING))
		return false;

	// no dockbay, can't depart to it
	if (!ship_has_dock_bay(shipnum))
		return false;

	// make sure that the bays are not all destroyed
	if (ship_fighterbays_all_destroyed(&Ships[shipnum]))
		return false;

	// in the future, we might want to check bay paths against destroyed fighterbays,
	// but that capability doesn't currently exist
	// (note, a mask of 0 indicates all paths are valid)
	//if (path_mask != 0)
	//{
	//}

	// ship is valid
	return true;
}

// Goober5000 - needed by new hangar depart code
// get first ship in ship list with docking bay
int ship_get_ship_for_departure(int team)
{
	for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
	{
		Assert(so->objnum >= 0);
		int shipnum = Objects[so->objnum].instance;
		Assert(shipnum >= 0);

		if ( (Ships[shipnum].team == team) && ship_useful_for_departure(shipnum) )
			return shipnum;
	}

	// we didn't find anything
	return -1;
}

// Goober5000 - check if all fighterbays on a ship have been destroyed
bool ship_fighterbays_all_destroyed(ship *shipp)
{
	Assert(shipp);
	ship_subsys *subsys;
	int num_fighterbay_subsystems = 0;

	// check all fighterbay systems
	subsys = GET_FIRST(&shipp->subsys_list);
	while(subsys != END_OF_LIST(&shipp->subsys_list))
	{
		// look for fighterbays
		if (ship_subsys_is_fighterbay(subsys))
		{
			num_fighterbay_subsystems++;

			// if fighterbay doesn't take damage, we're good
			if (!ship_subsys_takes_damage(subsys))
				return false;

			// if fighterbay isn't destroyed, we're good
			if (subsys->current_hits > 0)
				return false;
		}

		// next item
		subsys = GET_NEXT(subsys);
	}

	// if the ship has no fighterbay subsystems at all, it must be an unusual case,
	// like the Faustus, so pretend it's okay...
	if (num_fighterbay_subsystems == 0)
		return false;

	// if we got this far, the ship has at least one fighterbay subsystem,
	// and all the ones it has are destroyed
	return true;
}

// moved here by Goober5000
bool ship_subsys_is_fighterbay(ship_subsys *ss)
{
	Assert(ss);

	if ( !strnicmp(NOX("fighter"), ss->system_info->name, 7) ) {
		return true;
	}

	return false;
}

// Goober5000
bool ship_subsys_takes_damage(ship_subsys *ss)
{
	Assert(ss);

	return (ss->max_hits > SUBSYS_MAX_HITS_THRESHOLD);
}

// Goober5000
void ship_do_submodel_rotation(ship *shipp, model_subsystem *psub, ship_subsys *pss)
{
	Assert(shipp);
	Assert(psub);
	Assert(pss);

	// check if we actually can rotate
	if ( !(pss->flags & SSF_ROTATES) ){
		return;
	}

	if (psub->flags & MSS_FLAG_TRIGGERED && pss->triggered_rotation_index >= 0) {
		Triggered_rotations[pss->triggered_rotation_index].process_queue();
		model_anim_submodel_trigger_rotate(psub, pss );
		return;
	
	}

	// check for rotating artillery
	if ( psub->flags & MSS_FLAG_ARTILLERY )
	{
		ship_weapon *swp = &shipp->weapons;

		// rotate only if trigger is down
		if ( !(shipp->flags & SF_TRIGGER_DOWN) )
			return;

		// check linked
		if ( shipp->flags & SF_PRIMARY_LINKED )
		{
			int i, ammo_tally = 0;

			// calculate ammo
			for (i=0; i<swp->num_primary_banks; i++)
				ammo_tally += swp->primary_bank_ammo[i];

			// do not rotate if out of ammo
			if (ammo_tally <= 0)
				return;
		}
		// check unlinked
		else
		{
			// do not rotate if this is not the firing bank or if we have no ammo in this bank
			if ((psub->weapon_rotation_pbank != swp->current_primary_bank) || (swp->primary_bank_ammo[swp->current_primary_bank] <= 0))
				return;
		}
	}

	// if we got this far, we can rotate - so choose which method to use
	if (psub->flags & MSS_FLAG_STEPPED_ROTATE	) {
		submodel_stepped_rotate(psub, &pss->submodel_info_1);
	} else {
		submodel_rotate(psub, &pss->submodel_info_1 );
	}
}

// Goober5000
int ship_has_energy_weapons(ship *shipp)
{
	// (to avoid round-off errors, weapon reserve is not tested for zero)
	return (Ship_info[shipp->ship_info_index].max_weapon_reserve > WEAPON_RESERVE_THRESHOLD);
}

// Goober5000
int ship_has_engine_power(ship *shipp)
{
	return (Ship_info[shipp->ship_info_index].max_speed > 0 );
}

// Goober5000
int ship_starting_wing_lookup(const char *wing_name)
{
	for (int i = 0; i < MAX_STARTING_WINGS; i++)
	{
		if (!stricmp(Starting_wing_names[i], wing_name))
			return i;
	}

	return -1;
}

// Goober5000
int ship_squadron_wing_lookup(const char *wing_name)
{
	// TvT uses a different set of wing names from everything else
	if (MULTI_TEAM)
	{
		for (int i = 0; i < MAX_TVT_WINGS; i++)
		{
			if (!stricmp(TVT_wing_names[i], wing_name))
				return i;
		}
	}
	else 
	{
		for (int i = 0; i < MAX_SQUADRON_WINGS; i++)
		{
			if (!stricmp(Squadron_wing_names[i], wing_name))
				return i;
		}
	}

	return -1;
}

// Goober5000
int ship_tvt_wing_lookup(const char *wing_name)
{
	for (int i = 0; i < MAX_TVT_WINGS; i++)
	{
		if (!stricmp(TVT_wing_names[i], wing_name))
			return i;
	}

	return -1;
}

// Goober5000
int ship_class_get_priority(int ship_class)
{
	ship_info *sip = &Ship_info[ship_class];

	// biggest to smallest
	if (sip->flags & SIF_KNOSSOS_DEVICE)
		return 1;
	else if (sip->flags & SIF_SUPERCAP)
		return 2;
	else if (sip->flags & SIF_DRYDOCK)
		return 3;
	else if (sip->flags & SIF_CAPITAL)
		return 4;
	else if (sip->flags & SIF_CORVETTE)
		return 5;
	else if (sip->flags & SIF_CRUISER)
		return 6;
	else if (sip->flags & SIF_GAS_MINER)
		return 7;
	else if (sip->flags & SIF_AWACS)
		return 8;
	else if (sip->flags & SIF_FREIGHTER)
		return 9;
	else if (sip->flags & SIF_TRANSPORT)
		return 10;
	else if (sip->flags & SIF_BOMBER)
		return 11;
	else if (sip->flags & (SIF_FIGHTER | SIF_STEALTH))
		return 12;
	else if (sip->flags & SIF_ESCAPEPOD)
		return 13;
	else if (sip->flags & SIF_SENTRYGUN)
		return 14;
	else if (sip->flags & SIF_CARGO)
		return 15;
	else if (sip->flags & SIF_NAVBUOY)
		return 16;

	Warning(LOCATION, "Unknown priority for ship class '%s'!", sip->name);
	return 17 + ship_class;
}

// Goober5000
int ship_class_compare(int ship_class_1, int ship_class_2)
{
	// grab priorities
	int priority1 = ship_class_get_priority(ship_class_1);
	int priority2 = ship_class_get_priority(ship_class_2);

	// standard compare
	if (priority1 < priority2)
		return -1;
	else if (priority1 > priority2)
		return 1;
	else
		return 0;
}

/**
 * Gives the index into the Damage_types[] vector of a specified damage type name
 * @return -1 if not found
 */
int damage_type_get_idx(char *name)
{
	//This should never be bigger than INT_MAX anyway
	for(int i = 0; i < (int)Damage_types.size(); i++)
	{
		if(!stricmp(name, Damage_types[i].name))
			return i;
	}

	return -1;
}

/**
 * Either loads a new damage type, or returns the index of one with the same name as given
 */
int damage_type_add(char *name)
{
	int i = damage_type_get_idx(name);
	if(i != -1)
		return i;

	DamageTypeStruct dts;

	strncpy(dts.name, name, NAME_LENGTH-1);

	if(strlen(name) > NAME_LENGTH - 1)
	{
		Warning(LOCATION, "Damage type name '%s' is too long and has been truncated to '%s'", name, dts.name);
	}

	Damage_types.push_back(dts);
	return Damage_types.size()-1;
}

void ArmorDamageType::clear()
{
	DamageTypeIndex = -1;

	Calculations.clear();
	Arguments.clear();
	altArguments.clear();  // Nuke: don't forget to delete it
}

//************
// Wanderer - beam piercing type
//************

flag_def_list	PiercingTypes[] = {
	{	"none",		SADTF_PIERCING_NONE,		0},
	{	"default",	SADTF_PIERCING_DEFAULT,		0},
	{	"retail",	SADTF_PIERCING_RETAIL,		0},
};

const int Num_piercing_effect_types = sizeof(PiercingTypes)/sizeof(flag_def_list);

int piercing_type_get(char *str)
{
	int i;
	for(i = 0; i < Num_piercing_effect_types; i++)
	{
		if(!stricmp(PiercingTypes[i].name, str))
			return PiercingTypes[i].def;
	}

	// default to retail
	return SADTF_PIERCING_RETAIL;
}

// Nuke: handle difficulty scaling type
flag_def_list	DifficultyScaleTypes[] = {
	{	"first",	ADT_DIFF_SCALE_FIRST,	0},
	{	"last",		ADT_DIFF_SCALE_LAST,	0},
	{	"manual",	ADT_DIFF_SCALE_MANUAL,	0},
};

const int Num_difficulty_scale_types = sizeof(DifficultyScaleTypes)/sizeof(flag_def_list);

int difficulty_scale_type_get(char *str) {
	int i;
	for(i = 0; i < Num_difficulty_scale_types; i++){
		if (!stricmp(DifficultyScaleTypes[i].name, str))
			return DifficultyScaleTypes[i].def;
	}

	// indicate error
	return ADT_DIFF_SCALE_BAD_VAL;
}

// Nuke: flag list for +constant: values
flag_def_list	ArmorTypeConstants[] = {
	{	"base damage",			AT_CONSTANT_BASE_DMG,		0},
	{	"current damage",		AT_CONSTANT_CURRENT_DMG,	0},
	{	"difficulty factor",	AT_CONSTANT_DIFF_FACTOR,	0},
	{	"random",				AT_CONSTANT_RANDOM,			0},
	{	"pi",					AT_CONSTANT_PI,				0},
};

const int Num_armor_type_constants = sizeof(ArmorTypeConstants)/sizeof(flag_def_list);

int armor_type_constants_get(char *str){
	int i;
	for (i = 0; i < Num_armor_type_constants; i++){
		if (!stricmp(ArmorTypeConstants[i].name, str))
			return ArmorTypeConstants[i].def;
	}
	// this shouldnt happen, but if it does theirs a define for that
	return AT_CONSTANT_BAD_VAL;
}


//**************************************************************
//WMC - All the extra armor crap

//****************************Calculation type addition
//4 steps to add a new one

//Armor types
//STEP 1: Add a define
#define AT_TYPE_ADDITIVE				0
#define AT_TYPE_MULTIPLICATIVE			1
#define AT_TYPE_EXPONENTIAL				2
#define AT_TYPE_EXPONENTIAL_BASE		3
#define AT_TYPE_CUTOFF					4
#define AT_TYPE_REVERSE_CUTOFF			5
#define AT_TYPE_INSTANT_CUTOFF			6
#define AT_TYPE_INSTANT_REVERSE_CUTOFF	7
// Added by Nuke
#define AT_TYPE_CAP						8
#define AT_TYPE_INSTANT_CAP				9
#define AT_TYPE_SET						10
#define AT_TYPE_STORE					11
#define AT_TYPE_LOAD					12
#define AT_TYPE_RANDOM					13

// Nuke: this is the number of storage locations load/store calculations are allowed to use
#define AT_NUM_STORAGE_LOCATIONS		8

//STEP 2: Add the name string to the array
char *TypeNames[] = {
	"additive",
	"multiplicative",
	"exponential",
	"exponential base",
	"cutoff",
	"reverse cutoff",
	"instant cutoff",
	"instant reverse cutoff",
	// Added by Nuke
	"cap",
	"instant cap",
	"set",
	"load",
	"store",
	"random"
};

//STEP 3: Add the default value
float TypeDefaultValues[] = {
	0.0f,	//additive
	1.0f,	//multiplicatve
	1.0f,	//exp
	1.0f, 	//exp base - Damage will always be one (No mathematical way to do better)
	0.0f,	//cutoff
	0.0f,	//reverse cutoff
	0.0f,	//instant cutoff
	0.0f,	//rev instant cutoff
	// Added by Nuke
	0.0f,	// cap - caps are the same as reverse cutoffs, but sets damage to value instead of 0
	0.0f,	// instant cap
	0.0f,	// set - set the damage to value
	0.0f,	// data storage index - load and store calculations allow you to dump and retrieve the current damage in one of a few memory locations (these only persist for the duration of the computation)
	0.0f,	// data storage index
	0.0f	// random min/max
};

const int Num_armor_calculation_types = sizeof(TypeNames)/sizeof(char*);

int calculation_type_get(char *str)
{
	for(int i = 0; i < Num_armor_calculation_types; i++)
	{
		if(!stricmp(TypeNames[i], str))
			return i;
	}

	return -1;
}

//STEP 4: Add the calculation to the switch statement.
float ArmorType::GetDamage(float damage_applied, int in_damage_type_idx, float diff_dmg_scale)
{
	// Nuke: If the weapon has no damage type, just return damage
	if (in_damage_type_idx < 0) {
		// multiply by difficulty scaler now, since it is no longer done where this is called
		return (damage_applied * diff_dmg_scale);
	}

	//Initialize vars
	uint i,num;
	ArmorDamageType *adtp = NULL;

	//Find the entry in the weapon that corresponds to the given weapon damage type
	num = DamageTypes.size();
	for(i = 0; i < num; i++)
	{
		if(DamageTypes[i].DamageTypeIndex == in_damage_type_idx)
		{
			adtp = &DamageTypes[i];
			break;
		}
	}

	//curr_arg is the current calculation type value
	float curr_arg;

	//Make sure that we _have_ an armor entry for this damage type
	if(adtp != NULL)
	{
		//How many calculations do we have to do?
		num = adtp->Calculations.size();

		// Used for instant cutoffs/cap, to instantly end the loop
		bool end_now = false;
		// used for load/store operations
		float storage[AT_NUM_STORAGE_LOCATIONS];
		int storage_idx;
		bool using_storage = false;
		// constant related stuff
		float constant_val;
		float base_damage;
		bool using_constant = false;

		// set storage locations to zero
		for (i = 0; i < AT_NUM_STORAGE_LOCATIONS; i++) {
			storage[i]=0.0f;
		}

		// check to see if we need to difficulty scale damage first
		if (adtp->difficulty_scale_type == ADT_DIFF_SCALE_FIRST) {
			damage_applied *= diff_dmg_scale;
		}

		// user may want to use base damage as a constant
		base_damage = damage_applied;
		// LOOP!
		for (i = 0; i < num; i++) {
			storage_idx = adtp->altArguments[i];
			//Set curr_arg
			// use storage index at +Stored Value:
			if ( (storage_idx >= 0) && (storage_idx < AT_NUM_STORAGE_LOCATIONS) ) {
				curr_arg = storage[storage_idx];
				using_storage = true;
			// using +value: (or error cases caught at parse, where this holda a 0.0f)
			} else if (storage_idx == AT_CONSTANT_NOT_USED) { // save time checking all possible constants when most of the time you will be using +value:
				curr_arg = adtp->Arguments[i];
			// maybe handle constants
			} else if (storage_idx == AT_CONSTANT_BASE_DMG) {
				curr_arg = base_damage;
				using_constant = true;
			} else if (storage_idx == AT_CONSTANT_CURRENT_DMG) {
				curr_arg = damage_applied;
				using_constant = true;
			} else if (storage_idx == AT_CONSTANT_DIFF_FACTOR) {
				curr_arg = diff_dmg_scale;
				using_constant = true;
			} else if (storage_idx == AT_CONSTANT_RANDOM) {
				constant_val = frand();
				curr_arg = constant_val;
				using_constant = true;
			} else if (storage_idx == AT_CONSTANT_PI) {
				constant_val = PI;
				curr_arg = constant_val;
				using_constant = true;
			} else { // fail
				constant_val = 0.0f;
				curr_arg = constant_val;
			}
			// new calcs go here
			switch(adtp->Calculations[i])
			{
				case AT_TYPE_ADDITIVE:
					damage_applied += curr_arg;
					break;
				case AT_TYPE_MULTIPLICATIVE:
					damage_applied *= curr_arg;
					break;
				case AT_TYPE_EXPONENTIAL:
					damage_applied = powf(damage_applied, curr_arg);
					break;
				case AT_TYPE_EXPONENTIAL_BASE:
					damage_applied = powf(curr_arg, damage_applied);
					break;
				case AT_TYPE_CUTOFF:
					if(damage_applied < curr_arg)
						damage_applied = 0;
					break;
				case AT_TYPE_REVERSE_CUTOFF:
					if(damage_applied > curr_arg)
						damage_applied = 0;
					break;
				case AT_TYPE_INSTANT_CUTOFF:
					if(damage_applied < curr_arg)
					{
						damage_applied = 0;
						end_now = true;
					}
					break;
				case AT_TYPE_INSTANT_REVERSE_CUTOFF:
					if(damage_applied > curr_arg)
					{
						damage_applied = 0;
						end_now = true;
					}
					break;
				case AT_TYPE_CAP:
					if (damage_applied > curr_arg)
						damage_applied = curr_arg;
					break;
				case AT_TYPE_INSTANT_CAP:
					if (damage_applied > curr_arg) {
						damage_applied = curr_arg;
						end_now = true;
					}
					break;
				case AT_TYPE_SET:
					damage_applied = curr_arg;
					break;
				case AT_TYPE_STORE:
					if (using_storage || using_constant) {
						Warning(LOCATION, "Cannot use +Stored Value: or +Constant: with +Store:, that would be bad. Skipping calculation.");
					} else {
						storage_idx =  int(floorf(curr_arg));
						// Nuke: idiotproof this, no segfault 4 u
						if ( (storage_idx < 0) || (storage_idx >= AT_NUM_STORAGE_LOCATIONS) ) {
							Warning(LOCATION, "+Value: for +Store: calculation out of range. Should be between 0 and %i. Read: %i, Skipping calculation.", AT_NUM_STORAGE_LOCATIONS, storage_idx);
							storage_idx = 0;
						} else {
							storage[storage_idx] = damage_applied;
						}
					}
					break;
				case AT_TYPE_LOAD:
					if (using_storage || using_constant) {
						Warning(LOCATION, "Cannot use +Stored Value: or +Constant: with +Load:, that would be bad. Skipping calculation.");
					} else {
						storage_idx =  int(floorf(curr_arg));
						// Nuke: idiotproof this, no segfault 4 u
						if ( (storage_idx < 0) || (storage_idx >= AT_NUM_STORAGE_LOCATIONS) ) {
							Warning(LOCATION, "+Value: for +Load: calculation out of range. Should be between 0 and %i. Read: %i, Skipping calculation.", AT_NUM_STORAGE_LOCATIONS, storage_idx);
							storage_idx = 0;
						} else {
							damage_applied = storage[storage_idx];
						}
					}
					break;
				case AT_TYPE_RANDOM:  // Nuke: get a random number between damage_applied and +value:
					if (damage_applied > curr_arg) {
						damage_applied = frand_range( curr_arg, damage_applied );
					} else {
						damage_applied = frand_range( damage_applied, curr_arg );
					}
				break;
			}
			
			if(end_now)
				break;
		}
		// Nuke: check to see if we need to difficulty scale damage last
		if (adtp->difficulty_scale_type == ADT_DIFF_SCALE_LAST)
			damage_applied *= diff_dmg_scale;
	
		return damage_applied;
	}
	// fail return is fail
	return (damage_applied * diff_dmg_scale);
}

float ArmorType::GetShieldPiercePCT(int damage_type_idx)
{
	if(damage_type_idx < 0)
		return 0.0f;

	//Initialize vars
	uint i,num;
	ArmorDamageType *adtp = NULL;

	//Find the entry in the weapon that corresponds to the given weapon damage type
	num = DamageTypes.size();
	for(i = 0; i < num; i++)
	{
		if(DamageTypes[i].DamageTypeIndex == damage_type_idx)
		{
			adtp = &DamageTypes[i];
			break;
		}
	}
	if(adtp != NULL){
		return adtp->shieldpierce_pct;
	}

	return 0.0f;
}

int ArmorType::GetPiercingType(int damage_type_idx)
{
	if(damage_type_idx < 0)
		return 0;

	//Initialize vars
	uint i,num;
	ArmorDamageType *adtp = NULL;

	//Find the entry in the weapon that corresponds to the given weapon damage type
	num = DamageTypes.size();
	for(i = 0; i < num; i++)
	{
		if(DamageTypes[i].DamageTypeIndex == damage_type_idx)
		{
			adtp = &DamageTypes[i];
			break;
		}
	}
	if(adtp != NULL){
		return adtp->piercing_type;
	}

	return 0;
}

float ArmorType::GetPiercingLimit(int damage_type_idx)
{
	if(damage_type_idx < 0)
		return 0.0f;

	//Initialize vars
	uint i,num;
	ArmorDamageType *adtp = NULL;

	//Find the entry in the weapon that corresponds to the given weapon damage type
	num = DamageTypes.size();
	for(i = 0; i < num; i++)
	{
		if(DamageTypes[i].DamageTypeIndex == damage_type_idx)
		{
			adtp = &DamageTypes[i];
			break;
		}
	}
	if(adtp != NULL){
		return adtp->piercing_start_pct;
	}

	return 0.0f;
}

//***********************************Member functions

ArmorType::ArmorType(char* in_name)
{
	uint len = strlen(in_name);
	if(len >= NAME_LENGTH) {
		Warning(LOCATION, "Armor name %s is %d characters too long, and will be truncated", in_name, len - NAME_LENGTH);
	}
	
	strncpy(Name, in_name, NAME_LENGTH-1);
}

void ArmorType::ParseData()
{
	ArmorDamageType adt;
	char buf[NAME_LENGTH];
	float temp_float;
	int temp_int;
	int calc_type = -1;

	//Get the damage types
	required_string("$Damage Type:");
	do
	{
		//Get damage type name
		stuff_string(buf, F_NAME, NAME_LENGTH);
		
		//Clear the struct and set the index
		adt.clear();
		adt.DamageTypeIndex = damage_type_add(buf);
		bool no_content = true;

		//Get calculation and argument
		while (optional_string("+Calculation:")) 
		{
			//+Calculation
			stuff_string(buf, F_NAME, NAME_LENGTH);

			calc_type = calculation_type_get(buf);

			//Make sure we have a valid calculation type
			if(calc_type == -1)
			{
				Warning(LOCATION, "Armor '%s': Armor calculation type '%s' is invalid, and has been skipped", Name, buf);
				// Nuke: guess we need to add this here too
				if (optional_string("+Stored Value:")) {
					stuff_int(&temp_int);
				} else if (optional_string("+Constant:")) {
					stuff_string(buf, F_NAME, NAME_LENGTH);
				} else {
					required_string("+Value:");
					stuff_float(&temp_float);
				}
			}
			else
			{
				adt.Calculations.push_back(calc_type);
				// Nuke: maybe were using a stored location
				if (optional_string("+Stored Value:")) {
					stuff_int(&temp_int);
					// Nuke: idiot-proof
					if ( (temp_int < 0) || (temp_int >= AT_NUM_STORAGE_LOCATIONS) ) {
						Error(LOCATION, "+Stored Value: is out of range. Should be between 0 and %i. Read: %i, Using value 0.", AT_NUM_STORAGE_LOCATIONS-1, temp_int);
						temp_int = AT_CONSTANT_NOT_USED;
					}
					adt.altArguments.push_back(temp_int);
					adt.Arguments.push_back(0.0f); // this isnt used in this case, just take up space so the indices lign up, also a fallback value in case of bad altArguments
				} else if (optional_string("+Constant:")) { // use one of the pre-defined constants
					stuff_string(buf, F_NAME, NAME_LENGTH);
					temp_int = armor_type_constants_get(buf);
					// Nuke: idiot proof some more
					if (temp_int == AT_CONSTANT_BAD_VAL) {
						Error(LOCATION, "Invalid +Constant: name, '%s'. Using value 0.", buf);
						temp_int = AT_CONSTANT_NOT_USED;
					}
					adt.altArguments.push_back(temp_int);
					adt.Arguments.push_back(0.0f); // this isnt used in this case, just take up space so the indices lign up, also a fallback value in case of bad altArguments
				} else { // Nuke: +Value, only required if storage location or constant is not used -nuke
					required_string("+Value:");
					stuff_float(&temp_float);
					adt.altArguments.push_back(AT_CONSTANT_NOT_USED); // set this to AT_CONSTANT_NOT_USED so we know to just use the value from adt.Arguments instead of constants/storage locations
					adt.Arguments.push_back(temp_float);
				}
				no_content = false;
			}
		}

		adt.shieldpierce_pct = 0.0f;

		if(optional_string("+Shield Piercing Percentage:")) {
			stuff_float(&temp_float);
			CLAMP(temp_float, 0.0f, 1.0f);
			adt.shieldpierce_pct = temp_float;
			no_content = false;
		}

		adt.piercing_start_pct = 0.1f;
		adt.piercing_type = -1;

		if(optional_string("+Weapon Piercing Effect Start Limit:")) {
			stuff_float(&temp_float);
			CLAMP(temp_float, 0.0f, 100.0f); 
			temp_float /= 100.0f;
			adt.piercing_start_pct = temp_float;
			no_content = false;
		}

		if(optional_string("+Weapon Piercing Type:")) {
			stuff_string(buf, F_NAME, NAME_LENGTH);
			adt.piercing_type = piercing_type_get(buf);
			no_content = false;
		}

		// Nuke: don't forget to init things
		adt.difficulty_scale_type = ADT_DIFF_SCALE_FIRST;

		if (optional_string("+Difficulty Scale Type:")) {
			stuff_string(buf, F_NAME, NAME_LENGTH);
			temp_int = difficulty_scale_type_get(buf);
			if (temp_int == ADT_DIFF_SCALE_BAD_VAL) {
				Error(LOCATION, "Invalid +Difficulty Scale Type: name: '%s'. Reverting to default behavior.", buf);
				adt.difficulty_scale_type = ADT_DIFF_SCALE_FIRST;
			} else {
				adt.difficulty_scale_type = temp_int;
			}
			no_content = false;
		}

		//If we have calculations in this damage type, add it
		if(!no_content)
		{
			if(adt.Calculations.size() != adt.Arguments.size())
			{
				Warning(LOCATION, "Armor '%s', damage type " SIZE_T_ARG ": Armor has a different number of calculation types than arguments (" SIZE_T_ARG ", " SIZE_T_ARG ")",
						Name, DamageTypes.size(), adt.Calculations.size(), adt.Arguments.size());
			}
			DamageTypes.push_back(adt);
		}
	} while(optional_string("$Damage Type:"));
}

//********************************Global functions

int armor_type_get_idx(char* name)
{
	int i, num;
	num = Armor_types.size();
	for(i = 0; i < num; i++)
	{
		if(Armor_types[i].IsName(name))
			return i;
	}
	
	//Didn't find anything.
	return -1;
}

void parse_armor_type()
{
	char name_buf[NAME_LENGTH];
	ArmorType tat("");
	
	required_string("$Name:");
	stuff_string(name_buf, F_NAME, NAME_LENGTH);
	
	tat = ArmorType(name_buf);
	
	//now parse the actual table (damage type/armor type pairs)
	tat.ParseData();

	//rest of the parse data
	if (optional_string("$Flags:"))
		parse_string_flag_list((int*)&tat.flags, Armor_flags, Num_armor_flags);
	
	//Add it to global armor types
	Armor_types.push_back(tat);
}

void armor_parse_table(const char *filename)
{
	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		//Enumerate through all the armor types and add them.
		while (optional_string("#Armor Type")) {
			while (required_string_either("#End", "$Name:")) {
				parse_armor_type();
				continue;
			}

			required_string("#End");
		}

		// add tbl/tbm to multiplayer validation list
		fs2netd_add_table_validation(filename);
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

void armor_init()
{
	if (!armor_inited) {
		armor_parse_table("armor.tbl");

		parse_modular_table(NOX("*-amr.tbm"), armor_parse_table);

		armor_inited = 1;
	}
}

//**************************************************************
// AI targeting priority functions
//**************************************************************
void parse_ai_target_priorities()
{
	int i, j, num_strings;
	int n_entries = Ai_tp_list.size();
	SCP_vector <SCP_string> temp_strings;

	bool first_time = false;
	int already_exists = -1;

	if (n_entries == 0)
		first_time = true;

	required_string("$Name:");
	ai_target_priority temp_priority = init_ai_target_priorities();

	stuff_string(temp_priority.name, F_NAME, NAME_LENGTH);
	if (first_time == false) {
		for (i = 0; i < n_entries; i++) {
			if (!strnicmp(temp_priority.name, Ai_tp_list[i].name, NAME_LENGTH)) {
				already_exists = i;
			}
		}
	}

	if (optional_string("+Object Type:") ) {
		char tempname[NAME_LENGTH];
		stuff_string(tempname, F_NAME, NAME_LENGTH);

		for (j = 0; j < num_ai_tgt_objects; j++) {
			if ( !stricmp(ai_tgt_objects[j].name, tempname) ) {
				temp_priority.obj_type = ai_tgt_objects[j].def;
			}
		}
	}

	if (optional_string("+Weapon Class:") ) {
		temp_strings.clear();
		num_strings = stuff_string_list(temp_strings);

		for(i = 0; i < num_strings; i++) {
			for(j = 0; j < MAX_WEAPON_TYPES ; j++) {
				if ( !stricmp(Weapon_info[j].name, temp_strings[i].c_str()) ) {
					temp_priority.weapon_class.push_back(j);
					break;
				}
			}
			if (j == MAX_WEAPON_TYPES) {
				Warning(LOCATION, "Unidentified weapon class '%s' set for target priority group '%s'\n", temp_strings[i].c_str(), temp_priority.name);
			}
		}
	}

	if (optional_string("+Object Flags:") ) {
		temp_strings.clear();
		num_strings = stuff_string_list(temp_strings);

		for (i = 0; i < num_strings; i++) {
			for (j = 0; j < num_ai_tgt_obj_flags; j++) {
				if ( !stricmp(ai_tgt_obj_flags[j].name, temp_strings[i].c_str()) ) {
					temp_priority.obj_flags |= ai_tgt_obj_flags[j].def;
					break;
				}
			}
			if (j == num_ai_tgt_obj_flags) {
				Warning(LOCATION, "Unidentified object flag '%s' set for target priority group '%s'\n", temp_strings[i].c_str(), temp_priority.name);
			}
		}
	}

	if (optional_string("+Ship Class Flags:") ) {
		temp_strings.clear();
		num_strings = stuff_string_list(temp_strings);

		for (i = 0; i < num_strings; i++) {
			for (j = 0; j < num_ai_tgt_ship_flags; j++) {
				if ( !stricmp(ai_tgt_ship_flags[j].name, temp_strings[i].c_str()) ) {
					if (ai_tgt_ship_flags[j].var == 0) {
						temp_priority.sif_flags |= ai_tgt_ship_flags[j].def;
					} else {
						temp_priority.sif2_flags |= ai_tgt_ship_flags[j].def;
					}
					break;
				}
			}
			if (j == num_ai_tgt_ship_flags) {
				Warning(LOCATION, "Unidentified ship class flag '%s' set for target priority group '%s'\n", temp_strings[i].c_str(), temp_priority.name);
			}
		}
	}

	if (optional_string("+Weapon Class Flags:") ) {
		temp_strings.clear();
		num_strings = stuff_string_list(temp_strings);

		for (i = 0; i < num_strings; i++) {
			for (j = 0; j < num_ai_tgt_weapon_flags; j++) {
				if ( !stricmp(ai_tgt_weapon_flags[j].name, temp_strings[i].c_str()) ) {
					if (ai_tgt_weapon_flags[j].var == 0) {
						temp_priority.wif_flags |= ai_tgt_weapon_flags[j].def;
					} else {
						temp_priority.wif2_flags |= ai_tgt_weapon_flags[j].def;
					}
					break;
				}
			}
			if (j == num_ai_tgt_weapon_flags) {
				Warning(LOCATION, "Unidentified weapon class flag '%s' set for target priority group '%s'\n", temp_strings[i].c_str(), temp_priority.name);
			}
		}
	}

	temp_strings.clear();

	if (already_exists == -1) {
		Ai_tp_list.push_back(temp_priority);
	} else {
		Ai_tp_list[already_exists] = temp_priority;
	}
}

ai_target_priority init_ai_target_priorities()
{
	ai_target_priority temp_priority;

	//initialize the entries
	temp_priority.obj_flags = 0;
	temp_priority.obj_type = -1;
	temp_priority.ship_class.clear();
	temp_priority.ship_type.clear();
	temp_priority.sif_flags = 0;
	temp_priority.sif2_flags = 0;
	temp_priority.weapon_class.clear();
	temp_priority.wif2_flags = 0;
	temp_priority.wif_flags = 0;
	temp_priority.name[0] = '\0';

	//return the initialized
	return temp_priority;
}

void parse_weapon_targeting_priorities()
{
	char tempname[NAME_LENGTH];
	int i = 0;
	int j = 0;
	int k = 0;

	if (optional_string("$Name:")) {
		stuff_string(tempname, F_NAME, NAME_LENGTH);
		
		for(k = 0; k < MAX_WEAPON_TYPES ; k++) {
			if ( !stricmp(Weapon_info[k].name, tempname) ) {
				// found weapon, yay!
				// reset the list

				weapon_info *wip = &Weapon_info[k];
				
				wip->num_targeting_priorities = 0;

				if (optional_string("+Target Priority:")) {
					SCP_vector <SCP_string> tgt_priorities;
					int num_strings = stuff_string_list(tgt_priorities);

					if (num_strings > 32)
						num_strings = 32;

					int num_groups = Ai_tp_list.size();

					for(i = 0; i < num_strings; i++) {
						for(j = 0; j < num_groups; j++) {
							if ( !stricmp(Ai_tp_list[j].name, tgt_priorities[i].c_str()))  {
								wip->targeting_priorities[i] = j;
								wip->num_targeting_priorities++;
								break;
							}
						}
						if(j == num_groups)
							Warning(LOCATION, "Unrecognized string '%s' found when setting weapon targeting priorities.\n", tgt_priorities[i].c_str());
					}
				}
				// no need to keep searching for more
				break;
			}
		}
		if(k == MAX_WEAPON_TYPES)
			Warning(LOCATION, "Unrecognized weapon '%s' found when setting weapon targeting priorities.\n", tempname);
	}
}

int ship_get_subobj_model_num(ship_info* sip, char* subobj_name) 
{
	for (int i = 0; i < sip->n_subsystems; i++) {
		if (!subsystem_stricmp(sip->subsystems[i].subobj_name, subobj_name))
			return sip->subsystems[i].subobj_num;
	}

	return -1;
}

void init_path_metadata(path_metadata& metadata)
{
	vm_vec_zero(&metadata.departure_rvec);
	metadata.arrive_speed_mult = FLT_MIN;
	metadata.depart_speed_mult = FLT_MIN;
}

int ship_get_sound(object *objp, GameSoundsIndex id)
{
	Assert( objp != NULL );
	Assert( id >= 0 && id < (int) Snds.size() );

	// ugh, it's possible that we're an observer at this point
	if (objp->type == OBJ_OBSERVER)
		return id;

	Assert( objp->type == OBJ_SHIP );

	ship *shipp = &Ships[objp->instance];
	ship_info *sip = &Ship_info[shipp->ship_info_index];

	SCP_map<GameSoundsIndex, int>::iterator element = sip->ship_sounds.find(id);

	if (element == sip->ship_sounds.end())
		return id;
	else
		return (*element).second;
}

bool ship_has_sound(object *objp, GameSoundsIndex id)
{
	Assert( objp != NULL );
	Assert( id >= 0 && id < (int) Snds.size() );

	Assert( objp->type == OBJ_SHIP );

	ship *shipp = &Ships[objp->instance];
	ship_info *sip = &Ship_info[shipp->ship_info_index];

	SCP_map<GameSoundsIndex, int>::iterator element = sip->ship_sounds.find(id);

	if (element == sip->ship_sounds.end())
		return false;
	else
		return true;
}

/**
 * Given a ship with bounding box and a point, find the closest point on the bbox
 *
 * @param ship_objp Object that has the bounding box (should be a ship)
 * @param start World position of the point being compared
 * @param box_pt OUTPUT PARAMETER: closest point on the bbox to start
 *
 * @return point is inside bbox, TRUE/1
 * @return point is outside bbox, FALSE/0
 */
int get_nearest_bbox_point(object *ship_objp, vec3d *start, vec3d *box_pt)
{
	vec3d temp, rf_start;
	polymodel *pm;
	pm = model_get(Ship_info[Ships[ship_objp->instance].ship_info_index].model_num);

	// get start in ship rf
	vm_vec_sub(&temp, start, &ship_objp->pos);
	vm_vec_rotate(&rf_start, &temp, &ship_objp->orient);

	// find box_pt
	int inside = project_point_onto_bbox(&pm->mins, &pm->maxs, &rf_start, &temp);

	// get box_pt in world rf
	vm_vec_unrotate(box_pt, &temp, &ship_objp->orient);
	vm_vec_add2(box_pt, &ship_objp->pos);

	return inside;
}

void ship_set_thruster_info(mst_info *mst, object *obj, ship *shipp, ship_info *sip)
{
	mst->length.xyz.z = obj->phys_info.forward_thrust;
	mst->length.xyz.x = obj->phys_info.side_thrust;
	mst->length.xyz.y = obj->phys_info.vert_thrust;

	//	Maybe add noise to thruster geometry.
	if (!(sip->flags2 & SIF2_NO_THRUSTER_GEO_NOISE)) {
		mst->length.xyz.z *= (1.0f + frand()/5.0f - 0.1f);
		mst->length.xyz.y *= (1.0f + frand()/5.0f - 0.1f);
		mst->length.xyz.x *= (1.0f + frand()/5.0f - 0.1f);
	}

	CLAMP(mst->length.xyz.z, -1.0f, 1.0f);
	CLAMP(mst->length.xyz.y, -1.0f, 1.0f);
	CLAMP(mst->length.xyz.x, -1.0f, 1.0f);

	mst->primary_bitmap = shipp->thruster_bitmap;
	mst->primary_glow_bitmap = shipp->thruster_glow_bitmap;
	mst->secondary_glow_bitmap = shipp->thruster_secondary_glow_bitmap;
	mst->tertiary_glow_bitmap = shipp->thruster_tertiary_glow_bitmap;
	mst->distortion_bitmap = shipp->thruster_distortion_bitmap;

	mst->use_ab = (obj->phys_info.flags & PF_AFTERBURNER_ON) || (obj->phys_info.flags & PF_BOOSTER_ON);
	mst->glow_noise = shipp->thruster_glow_noise;
	mst->rotvel = Objects[shipp->objnum].phys_info.rotvel;

	mst->glow_rad_factor = sip->thruster01_glow_rad_factor;
	mst->secondary_glow_rad_factor = sip->thruster02_glow_rad_factor;
	mst->tertiary_glow_rad_factor = sip->thruster03_glow_rad_factor;
	mst->glow_length_factor = sip->thruster02_glow_len_factor;
	mst->distortion_length_factor = sip->thruster_dist_len_factor;
	mst->distortion_rad_factor = sip->thruster_dist_rad_factor;

	mst->draw_distortion = sip->draw_distortion;
}

void ship_render_batch_thrusters(object *obj)
{
	int num = obj->instance;
	ship *shipp = &Ships[num];
	ship_info *sip = &Ship_info[Ships[num].ship_info_index];

	if ( Rendering_to_shadow_map ) return;

	physics_info *pi = &Objects[shipp->objnum].phys_info;
	float render_amount;
	fx_batcher.allocate(sip->num_maneuvering);	//Act as if all thrusters are going.

	for ( int i = 0; i < sip->num_maneuvering; i++ ) {
		man_thruster *mtp = &sip->maneuvering[i];

		render_amount = 0.0f;

		//WMC - get us a steady value
		vec3d des_vel;
		vm_vec_rotate(&des_vel, &pi->desired_vel, &obj->orient);

		if(pi->desired_rotvel.xyz.x < 0 && (mtp->use_flags & MT_PITCH_UP)) {
			render_amount = fl_abs(pi->desired_rotvel.xyz.x) / pi->max_rotvel.xyz.x;
		} else if(pi->desired_rotvel.xyz.x > 0 && (mtp->use_flags & MT_PITCH_DOWN)) {
			render_amount = fl_abs(pi->desired_rotvel.xyz.x) / pi->max_rotvel.xyz.x;
		} else if(pi->desired_rotvel.xyz.y < 0 && (mtp->use_flags & MT_ROLL_RIGHT)) {
			render_amount = fl_abs(pi->desired_rotvel.xyz.y) / pi->max_rotvel.xyz.y;
		} else if(pi->desired_rotvel.xyz.y > 0 && (mtp->use_flags & MT_ROLL_LEFT)) {
			render_amount = fl_abs(pi->desired_rotvel.xyz.y) / pi->max_rotvel.xyz.y;
		} else if(pi->desired_rotvel.xyz.z < 0 && (mtp->use_flags & MT_BANK_RIGHT)) {
			render_amount = fl_abs(pi->desired_rotvel.xyz.z) / pi->max_rotvel.xyz.z;
		} else if(pi->desired_rotvel.xyz.z > 0 && (mtp->use_flags & MT_BANK_LEFT)) {
			render_amount = fl_abs(pi->desired_rotvel.xyz.z) / pi->max_rotvel.xyz.z;
		}

		//Backslash - show thrusters according to thrust amount, not speed
		if(pi->side_thrust > 0 && (mtp->use_flags & MT_SLIDE_RIGHT)) {
			render_amount = pi->side_thrust;
		} else if(pi->side_thrust < 0 && (mtp->use_flags & MT_SLIDE_LEFT)) {
			render_amount = -pi->side_thrust;
		} else if(pi->vert_thrust > 0 && (mtp->use_flags & MT_SLIDE_UP)) {
			render_amount = pi->vert_thrust;
		} else if(pi->vert_thrust < 0 && (mtp->use_flags & MT_SLIDE_DOWN)) {
			render_amount = -pi->vert_thrust;
		} else if(pi->forward_thrust > 0 && (mtp->use_flags & MT_FORWARD)) {
			render_amount = pi->forward_thrust;
		} else if(pi->forward_thrust < 0 && (mtp->use_flags & MT_REVERSE)) {
			render_amount = -pi->forward_thrust;
		}

		//Don't render small faraway thrusters (more than 10k * radius away)
		if ( vm_vec_dist(&Eye_position, &obj->pos) > (10000.0f * mtp->radius) ) {
			render_amount = 0.0f;
		}

		if ( render_amount > 0.0f ) {
			//Handle sounds and stuff
			if ( shipp->thrusters_start[i] <= 0 ) {
				shipp->thrusters_start[i] = timestamp();
				if(mtp->start_snd >= 0)
					snd_play_3d( &Snds[mtp->start_snd], &mtp->pos, &Eye_position, 0.0f, &obj->phys_info.vel );
			}

			//Only assign looping sound if
			//it is specified
			//it isn't assigned already
			//start sound doesn't exist or has finished
			if (!Cmdline_freespace_no_sound) {
				if(mtp->loop_snd >= 0
					&& shipp->thrusters_sounds[i] < 0
					&& (mtp->start_snd < 0 || (snd_get_duration(mtp->start_snd) < timestamp() - shipp->thrusters_start[i])) 
					)
				{
					shipp->thrusters_sounds[i] = obj_snd_assign(OBJ_INDEX(obj), mtp->loop_snd, &mtp->pos, 1);
				}
			}

			//Draw graphics
			//Skip invalid ones
			if ( mtp->tex_id >= 0 ) {
				float rad = mtp->radius;
				if(rad <= 0.0f)
					rad = 1.0f;

				float len = mtp->length;
				if(len == 0.0f)
					len = rad;

				vec3d start, tmpend, end;
				//Start
				vm_vec_unrotate(&start, &mtp->pos, &obj->orient);
				vm_vec_add2(&start, &obj->pos);

				//End
				vm_vec_scale_add(&tmpend, &mtp->pos, &mtp->norm, len * render_amount);
				vm_vec_unrotate(&end, &tmpend, &obj->orient);
				vm_vec_add2(&end, &obj->pos);

				//Draw
				fx_batcher.draw_beam(&start, &end, rad, 1.0f);

				int bmap_frame = mtp->tex_id;
				if(mtp->tex_nframes > 0)
					bmap_frame += (int)(((float)(timestamp() - shipp->thrusters_start[i]) / 1000.0f) * (float)mtp->tex_fps) % mtp->tex_nframes;

				man_thruster_renderer *mtr = man_thruster_get_slot(bmap_frame);
				mtr->man_batcher.add_allocate(1);
				mtr->man_batcher.draw_beam(&start, &end, rad, 1.0f);
			}
		} else if ( shipp->thrusters_start[i] > 0 ) { 
			// We've stopped firing a thruster
			
			shipp->thrusters_start[i] = 0;
			if(shipp->thrusters_sounds[i] >= 0)
			{
				obj_snd_delete(OBJ_INDEX(obj), shipp->thrusters_sounds[i]);
				shipp->thrusters_sounds[i] = -1;
			}

			if ( mtp->stop_snd >= 0 ) {
				//Get world pos
				vec3d start;
				vm_vec_unrotate(&start, &mtp->pos, &obj->orient);
				vm_vec_add2(&start, &obj->pos);

				snd_play_3d( &Snds[mtp->stop_snd], &mtp->pos, &Eye_position, 0.0f, &obj->phys_info.vel );
			}
		}
	}
}

void ship_render_weapon_models(model_render_params *ship_render_info, draw_list *scene, object *obj, int render_flags)
{
	int num = obj->instance;
	ship *shipp = &Ships[num];
	ship_info *sip = &Ship_info[Ships[num].ship_info_index];

	if ( !(sip->flags2 & SIF2_DRAW_WEAPON_MODELS) || (shipp->flags2 & SF2_CLOAKED) ) {
		return;
	}

	int i,k;
	ship_weapon *swp = &shipp->weapons;

	scene->push_transform(&obj->pos, &obj->orient);

	model_render_params render_info = *ship_render_info;

	render_flags &= ~MR_SHOW_THRUSTERS;

	//primary weapons
	for ( i = 0; i < swp->num_primary_banks; i++ ) {
		if ( Weapon_info[swp->primary_bank_weapons[i]].external_model_num == -1 || !sip->draw_primary_models[i] ) {
			continue;
		}

		w_bank *bank = &model_get(sip->model_num)->gun_banks[i];
		for ( k = 0; k < bank->num_slots; k++ ) {
			polymodel* pm = model_get(Weapon_info[swp->primary_bank_weapons[i]].external_model_num);

			pm->gun_submodel_rotation = shipp->primary_rotate_ang[i];

			render_info.set_flags(render_flags);

			model_render_queue(&render_info, scene, Weapon_info[swp->primary_bank_weapons[i]].external_model_num, &vmd_identity_matrix, &bank->pnt[k]);

			pm->gun_submodel_rotation = 0.0f;
		}
	}

	//secondary weapons
	int num_secondaries_rendered = 0;
	vec3d secondary_weapon_pos;
	w_bank* bank;

	for (i = 0; i < swp->num_secondary_banks; i++) {
		if ( Weapon_info[swp->secondary_bank_weapons[i]].external_model_num == -1 || !sip->draw_secondary_models[i] ) {
			continue;
		}

		bank = &(model_get(sip->model_num))->missile_banks[i];

		if (Weapon_info[swp->secondary_bank_weapons[i]].wi_flags2 & WIF2_EXTERNAL_WEAPON_LNCH) {
			for(k = 0; k < bank->num_slots; k++) {
				render_info.set_flags(render_flags);

				model_render_queue(&render_info, scene, Weapon_info[swp->secondary_bank_weapons[i]].external_model_num, &vmd_identity_matrix, &bank->pnt[k]);
			}
		} else {
			num_secondaries_rendered = 0;

			for ( k = 0; k < bank->num_slots; k++ ) {
				secondary_weapon_pos = bank->pnt[k];

				if ( num_secondaries_rendered >= shipp->weapons.secondary_bank_ammo[i] ) {
					break;
				}

				if ( shipp->secondary_point_reload_pct[i][k] <= 0.0 ) {
					continue;
				}

				num_secondaries_rendered++;

				vm_vec_scale_add2(&secondary_weapon_pos, &vmd_z_vector, -(1.0f-shipp->secondary_point_reload_pct[i][k]) * model_get(Weapon_info[swp->secondary_bank_weapons[i]].external_model_num)->rad);

				render_info.set_flags(render_flags);

				model_render_queue(&render_info, scene, Weapon_info[swp->secondary_bank_weapons[i]].external_model_num, &vmd_identity_matrix, &secondary_weapon_pos);
			}
		}
	}

	scene->pop_transform();
}

int ship_render_get_insignia(object* obj, ship* shipp)
{
	if ( Rendering_to_shadow_map ) {
		return -1;
	}

	if ( Game_mode & GM_MULTIPLAYER ) {
		// if its any player's object
		int np_index = multi_find_player_by_object( obj );
		if ( (np_index >= 0) && (np_index < MAX_PLAYERS) && MULTI_CONNECTED(Net_players[np_index]) && (Net_players[np_index].m_player != NULL) ) {
			return Net_players[np_index].m_player->insignia_texture;
		}
	} 

	// in single player, we want to render model insignias on all ships in alpha beta and gamma
	// Goober5000 - and also on wings that have their logos set

	// if its an object in my squadron
	if ( ship_in_my_squadron(shipp) ) {
		return Player->insignia_texture;
	}

	// maybe it has a wing squad logo - Goober5000
	if ( shipp->wingnum >= 0 ) {
		// don't override the player's wing
		if ( shipp->wingnum != Player_ship->wingnum ) {
			// if we have a logo texture
			if ( Wings[shipp->wingnum].wing_insignia_texture >= 0 ) {
				return Wings[shipp->wingnum].wing_insignia_texture;
			}
		}
	}

	return -1;
}

void ship_render_set_animated_effect(model_render_params *render_info, ship *shipp, uint *render_flags)
{
	if ( !shipp->shader_effect_active || Use_GLSL <= 1 || Rendering_to_shadow_map ) {
		return;
	}

	float timer;

	ship_effect* sep = &Ship_effects[shipp->shader_effect_num];
	
	if ( sep->invert_timer ) {
		timer = 1.0f - ((timer_get_milliseconds() - shipp->shader_effect_start_time) / (float)shipp->shader_effect_duration);
		timer = MAX(timer,0.0f);
	} else {
		timer = ((timer_get_milliseconds() - shipp->shader_effect_start_time) / (float)shipp->shader_effect_duration);
	}

	render_info->set_animated_effect(sep->shader_effect, timer);

	if ( sep->disables_rendering && (timer_get_milliseconds() > shipp->shader_effect_start_time + shipp->shader_effect_duration) ) {
		shipp->flags2 |= SF2_CLOAKED;
		shipp->shader_effect_active = false;
	} else {
		shipp->flags2 &= ~SF2_CLOAKED;
		if (timer_get_milliseconds() > shipp->shader_effect_start_time + shipp->shader_effect_duration) {
			shipp->shader_effect_active = false;
		}
	}
}

void ship_render(object* obj, draw_list* scene)
{
	int num = obj->instance;
	ship *shipp = &Ships[num];
	ship_info *sip = &Ship_info[Ships[num].ship_info_index];
	ship *warp_shipp = NULL;
	bool is_first_stage_arrival = false;
	bool show_thrusters = ((shipp->flags2 & SF2_NO_THRUSTERS) == 0) && !Rendering_to_shadow_map;
	dock_function_info dfi;

	MONITOR_INC( NumShipsRend, 1 );

	// look for a warping ship, whether for me or for anybody I'm docked with
	dock_evaluate_all_docked_objects(obj, &dfi, ship_find_warping_ship_helper);

	// if any docked objects are set to stage 1 arrival then set bool
	if ( dfi.maintained_variables.bool_value ) {
		warp_shipp = &Ships[dfi.maintained_variables.objp_value->instance];

		is_first_stage_arrival = ((warp_shipp->flags & SF_ARRIVING_STAGE_1) > 0);

		// This is a hack to make ships using the hyperspace warpin type to
		// render even in stage 1, which is used for collision detection
		// purposes -zookeeper
		if ( Ship_info[warp_shipp->ship_info_index].warpin_type == WT_HYPERSPACE ) {
			warp_shipp = NULL;
			is_first_stage_arrival = false;
		}
	}

	if ( is_first_stage_arrival ) {
		//WMC - Draw animated warp effect (ie BSG thingy)
		//WMC - based on Bobb's secondary thruster stuff
		//which was in turn based on the beam code.
		//I'm gonna need some serious acid to neutralize this base.
		if(shipp->flags & SF_ARRIVING) {
			shipp->warpin_effect->warpShipRender();
		} else if(shipp->flags & SF_DEPART_WARP) {
			shipp->warpout_effect->warpShipRender();
		}

		return;
	}

	if ( obj == Viewer_obj && !Rendering_to_shadow_map ) {
		if (!(Viewer_mode & VM_TOPDOWN))
		{
			return;
		}
	}

	// Only render electrical arcs if within 500m of the eye (for a 10m piece)
	if ( vm_vec_dist_quick( &obj->pos, &Eye_position ) < obj->radius*50.0f && !Rendering_to_shadow_map ) {
		for ( int i = 0; i < MAX_SHIP_ARCS; i++ )	{
			if ( timestamp_valid(shipp->arc_timestamp[i]) ) {
				model_add_arc(sip->model_num, -1, &shipp->arc_pts[i][0], &shipp->arc_pts[i][1], shipp->arc_type[i]);
			}
		}
	}

	uint render_flags = MR_NORMAL;

	if ( shipp->large_ship_blowup_index >= 0 )	{
		shipfx_large_blowup_queue_render(scene, shipp);

		//WMC - Draw animated warp effect (ie BSG thingy)
		//WMC - based on Bobb's secondary thruster stuff
		//which was in turn based on the beam code.
		//I'm gonna need some serious acid to neutralize this base.
		if(shipp->flags & SF_ARRIVING) {
			shipp->warpin_effect->warpShipRender();
		} else if(shipp->flags & SF_DEPART_WARP) {
			shipp->warpout_effect->warpShipRender();
		}

		return;
	}
		
	ship_render_batch_thrusters(obj);

	model_render_params render_info;
	
	if ( !(shipp->flags & SF_DISABLED) && !ship_subsys_disrupted(shipp, SUBSYSTEM_ENGINE) && show_thrusters) {
		mst_info mst;

		ship_set_thruster_info(&mst, obj, shipp, sip);

		render_info.set_thruster_info(mst);

		render_flags |= MR_SHOW_THRUSTERS;
	}

	// Warp_shipp points to the ship that is going through a
	// warp... either this ship or the ship it is docked with.
	if ( warp_shipp != NULL ) {
		if ( warp_shipp->flags & SF_ARRIVING ) {
			warp_shipp->warpin_effect->warpShipClip(&render_info);
		} else if ( warp_shipp->flags & SF_DEPART_WARP ) {
			warp_shipp->warpout_effect->warpShipClip(&render_info);
		}
	}

	// maybe set squad logo bitmap
	render_info.set_insignia_bitmap(ship_render_get_insignia(obj, shipp));

	// Valathil - maybe do a scripting hook here to do some scriptable effects?
	ship_render_set_animated_effect(&render_info, shipp, &render_flags);

	if ( sip->uses_team_colors ) {
		team_color model_team_color;

		bool team_color_set = model_get_team_color(&model_team_color, shipp->team_name, shipp->secondary_team_name, shipp->team_change_timestamp, shipp->team_change_time);

		if ( team_color_set ) {
			render_info.set_team_color(model_team_color);
		}
	}

	if ( Rendering_to_shadow_map ) {
		render_flags = MR_NO_TEXTURING | MR_NO_LIGHTING;
	}

	if (shipp->flags2 & SF2_GLOWMAPS_DISABLED) {
		render_flags |= MR_NO_GLOWMAPS;
	}

	render_info.set_flags(render_flags);

	//draw weapon models
	ship_render_weapon_models(&render_info, scene, obj, render_flags);

	render_info.set_object_number(OBJ_INDEX(obj));
	render_info.set_replacement_textures(shipp->ship_replacement_textures);

	// small ships
	if ( !( shipp->flags2 & SF2_CLOAKED ) ) {
		if ( ( The_mission.flags & MISSION_FLAG_FULLNEB ) && ( sip->flags & SIF_SMALL_SHIP ) ) {			
			// force detail levels
			float fog_val = neb2_get_fog_intensity(obj);
			if ( fog_val >= 0.6f ) {
				render_info.set_detail_level_lock(2);
			}
		}

		model_render_queue(&render_info, scene, sip->model_num, &obj->orient, &obj->pos);
	}

	if (shipp->shield_hits && !Rendering_to_shadow_map) {
		create_shield_explosion_all(obj);
		shipp->shield_hits = 0;
	}

	//WMC - Draw animated warp effect (ie BSG thingy)
	//WMC - based on Bobb's secondary thruster stuff
	//which was in turn based on the beam code.
	//I'm gonna need some serious acid to neutralize this base.
	if(shipp->flags & SF_ARRIVING) {
		shipp->warpin_effect->warpShipRender();
	} else if(shipp->flags & SF_DEPART_WARP) {
		shipp->warpout_effect->warpShipRender();
	}
}
