/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



/**
 * @file
 * Contains the actual AI code that does interesting stuff to objects.
 *
 * The code in Ai.cpp is just for bookkeeping, allocating AI slots and linking them to ships.
 */


#include "ai/ai.h"
#include "globalincs/linklist.h"
#include "object/object.h"
#include "physics/physics.h"
#include "ship/ship.h"
#include "model/model.h"
#include "render/3d.h"
#include "playerman/player.h"
#include "freespace2/freespace.h"
#include "mission/missiongoals.h"
#include "mission/missionlog.h"
#include "io/timer.h"
#include "ai/aigoals.h"
#include "gamesnd/gamesnd.h"
#include "mission/missionmessage.h"
#include "cmeasure/cmeasure.h"
#include "math/staticrand.h"
#include "ship/afterburner.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"
#include "ai/aibig.h"
#include "hud/hud.h"
#include "object/objcollide.h"
#include "object/objectshield.h"
#include "asteroid/asteroid.h"
#include "hud/hudlock.h"
#include "mission/missiontraining.h"
#include "gamesequence/gamesequence.h"
#include "io/joy_ff.h"
#include "localization/localize.h"
#include "weapon/weapon.h"
#include "weapon/flak.h"
#include "weapon/beam.h"
#include "weapon/swarm.h"
#include "ship/awacs.h"
#include "math/fvi.h"
#include "parse/parselo.h"
#include "object/objectdock.h"
#include "object/deadobjectdock.h"
#include "object/waypoint.h"
#include "ai/aiinternal.h"
#include "iff_defs/iff_defs.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multi.h"
#include "ai/ai_profiles.h"
#include "autopilot/autopilot.h"
#include <map>
#include <limits.h>


#pragma optimize("", off)
#pragma auto_inline(off)

#define	UNINITIALIZED_VALUE	-99999.9f

#define INSTRUCTOR_SHIP_NAME NOX("instructor")

#define AICODE_SMALL_MAGNITUDE	0.001f		// cosider a vector NULL if mag is less than this

#define NEXT_REARM_TIMESTAMP (60*1000)			//	Ships will re-request rearm, typically, after this long.

#define CIRCLE_STRAFE_MAX_DIST 300.0f	//Maximum distance for circle strafe behavior.

// AIM_CHASE submode defines
// SM_STEALTH_FIND
#define	SM_SF_AHEAD		0
#define	SM_SF_BEHIND	1
#define	SM_SF_BAIL		2

// SM_STEALTH_SWEEP
#define	SM_SS_SET_GOAL	-1
#define	SM_SS_BOX0		0
#define	SM_SS_LR			1
#define	SM_SS_UL			2
#define	SM_SS_BOX1		3
#define	SM_SS_UR			4
#define	SM_SS_LL			5
#define	SM_SS_BOX2		6
#define	SM_SS_DONE		7

//XSTR:OFF

char *Mode_text[MAX_AI_BEHAVIORS] = {
	"CHASE",
	"EVADE",
	"GET_BEHIND",
	"CHASE_LONG",
	"SQUIGGLE",
	"GUARD",
	"AVOID",
	"WAYPOINTS",
	"DOCK",
	"NONE",
	"BIGSHIP",
	"PATH",
	"BE_REARMED",
	"SAFETY",
	"EV_WEAPON",
	"STRAFE",
	"PLAY_DEAD",
	"BAY_EMERGE",
	"BAY_DEPART",
	"SENTRYGUN",
	"WARP_OUT",
};

//	Submode text is only valid for CHASE mode.
char *Submode_text[] = {
"undefined",
"CONT_TURN",
"ATTACK   ",
"E_SQUIG  ",
"E_BRAKE  ",
"EVADE    ",
"SUP_ATTAK",
"AVOID    ",
"BEHIND   ",
"GET_AWAY ",
"E_WEAPON ",
"FLY_AWAY ",
"ATK_4EVER",
"STLTH_FND",
"STLTH_SWP",
"BIG_APPR",
"BIG_CIRC",
"BIG_PARL"
};

char *Strafe_submode_text[5] = {
"ATTACK",
"AVOID",
"RETREAT1",
"RETREAT2",
"POSITION"
};
//XSTR:ON

// few forward decs i needed - kazan
object * get_wing_leader(int wingnum);
int get_wing_index(object *objp, int wingnum);

control_info	AI_ci;

object *Pl_objp;
object *En_objp;

#define	REARM_SOUND_DELAY		(3*F1_0)		//	Amount of time to delay rearm/repair after mode start
#define	REARM_BREAKOFF_DELAY	(3*F1_0)		//	Amount of time to wait after fully rearmed to breakoff.

#define	MIN_DIST_TO_WAYPOINT_GOAL	5.0f
#define	MAX_GUARD_DIST					250.0f
#define	BIG_GUARD_RADIUS				500.0f

#define	MAX_EVADE_TIME			(15 * 1000)	//	Max time to evade a weapon.

// defines for repair ship stuff.
#define	MAX_REPAIR_SPEED			25.0f
#define	MAX_UNDOCK_ABORT_SPEED	2.0f

// defines for EMP effect stuff
#define	MAX_EMP_INACCURACY		50.0f

// defines for stealth
#define	MAX_STEALTH_INACCURACY	50.0f		// at max view dist
#define	STEALTH_MAX_VIEW_DIST	400		// dist at which 1) stealth no longer visible 2) firing inaccuracy is greatest
#define	STEALTH_VIEW_CONE_DOT	0.707		// (half angle of 45 degrees)

ai_class *Ai_classes = NULL;
int	Ai_firing_enabled = 1;
int	Num_ai_classes;
int Num_alloced_ai_classes;

int	AI_FrameCount = 0;
int	AI_watch_object = 0; // Debugging, object to spew debug info for.
int	Mission_all_attack = 0;					//	!0 means all teams attack all teams.

char *Skill_level_names(int level, int translate)
{
	char *str = NULL;

	#if NUM_SKILL_LEVELS != 5
	#error Number of skill levels is wrong!
	#endif

	if(translate){
		switch( level )	{
		case 0:
			str = XSTR("Very Easy", 469);
			break;
		case 1:
			str = XSTR("Easy", 470);
			break;
		case 2:
			str = XSTR("Medium", 471);
			break;
		case 3:
			str = XSTR("Hard", 472);
			break;
		case 4:
			str = XSTR("Insane", 473);
			break;
		default:	
			Int3();
		}
	} else {
		switch( level )	{
		case 0:
			str = NOX("Very Easy");
			break;
		case 1:
			str = NOX("Easy");
			break;
		case 2:
			str = NOX("Medium");
			break;
		case 3:
			str = NOX("Hard");
			break;
		case 4:
			str = NOX("Insane");
			break;
		default:	
			Int3();
		}
	}

	return str;
}

#define	DELAY_TARGET_TIME	(12*1000)		//	time in milliseconds until a ship can target a new enemy after an order.

pnode		Path_points[MAX_PATH_POINTS];
pnode		*Ppfp;			//	Free pointer in path points.

float	AI_frametime;

char** Ai_class_names = NULL;

typedef struct {
	int team;
	int weapon_index;
	int max_fire_count;
	char	*shipname;
} huge_fire_info;

SCP_vector<huge_fire_info> Ai_huge_fire_info;

int Ai_last_arrive_path;	// index of ship_bay path used by last arrival from a fighter bay

// forward declarations
int	ai_return_path_num_from_dockbay(object *dockee_objp, int dockbay_index);
void create_model_exit_path(object *pl_objp, object *mobjp, int path_num, int count=1);
void copy_xlate_model_path_points(object *objp, model_path *mp, int dir, int count, int path_num, pnode *pnp, int randomize_pnt=-1);
void ai_cleanup_rearm_mode(object *objp);
void ai_cleanup_dock_mode_subjective(object *objp);
void ai_cleanup_dock_mode_objective(object *objp);

// The object that is declared to be the leader of the group formation for
// the "autopilot"
object *Autopilot_flight_leader = NULL;

/**
 * Sets the timestamp used to tell is it is a good time for this team to rearm.  
 * Once the timestamp is no longer valid, then rearming is not a "good time"
 *
 * @param team Team (friendly, hostile, neutral) 
 * @param time Time to rearm
 */
void ai_set_rearm_status(int team, int time)
{
	Assert( time >= 0 );

	Iff_info[team].ai_rearm_timestamp = timestamp(time * 1000);
}

/**
 * "safe" is currently defined by the mission designer using the good/bad time to rearm sexpressions.  This status is currently team based.
 *
 * @return true(1) or false(0) if it is "safe" for the given object to rearm.  
 * @todo This function could be easily expended to further the definition of "safe"
 */
int ai_good_time_to_rearm(object *objp)
{
	int team;

	Assert(objp->type == OBJ_SHIP);
	team = Ships[objp->instance].team;
	
	return timestamp_valid(Iff_info[team].ai_rearm_timestamp);
}

/**
 * Entry point from sexpression code to set internal data for use by AI code.
 */
void ai_good_secondary_time( int team, int weapon_index, int max_fire_count, char *shipname )
{
	Assert(shipname != NULL);
	int index;
	huge_fire_info new_info; 

	new_info.weapon_index = weapon_index;
	new_info.team = team;
	new_info.max_fire_count = max_fire_count;

	new_info.shipname = ai_get_goal_target_name( shipname, &index );

	Ai_huge_fire_info.push_back(new_info);
}

/**
 * Called internally to the AI code to tell whether or not weapon_num can be fired
 * from firer_objp at target_objp.  This function will resolve the team for the firer.
 *
 * @return -1 when conditions don't allow firer to fire weapon_num on target_objp
 * @return >=0 when conditions allow firer to fire.  Return value is max number of weapon_nums which can be fired on target_objp
 */
int is_preferred_weapon(int weapon_num, object *firer_objp, object *target_objp)
{
	int firer_team, target_signature;
	ship *firer_ship;
	SCP_vector<huge_fire_info>::iterator hfi;

	Assert( firer_objp->type == OBJ_SHIP );
	firer_ship = &Ships[firer_objp->instance];
	firer_team = firer_ship->team;

	// get target object's signature and try to find it in the list.
	target_signature = target_objp->signature;
	for (hfi = Ai_huge_fire_info.begin();hfi != Ai_huge_fire_info.end(); ++hfi ) {
		int ship_index, signature;

		if ( hfi->weapon_index == -1 )
			continue;

		ship_index = ship_name_lookup( hfi->shipname );
		if ( ship_index == -1 )
			continue;

		signature = Objects[Ships[ship_index].objnum].signature;

		// sigatures, weapon_index, and team must match
		if ( (signature == target_signature) && (hfi->weapon_index == weapon_num) && (hfi->team == firer_team) )
			break;
	}

	// return -1 if not found
	if ( hfi == Ai_huge_fire_info.end() )
		return -1;

	// otherwise, we can return the max number of weapons we can fire against target_objps
	return hfi->max_fire_count;
}

/** 
 * Clear out secondary firing information between levels
 */
void ai_init_secondary_info()
{
	Ai_huge_fire_info.clear();
}

/**
 * Garbage collect the ::Path_points buffer.
 *
 * Scans all objects, looking for used Path_points records.
 *	Compresses Path_points buffer, updating aip->path_start and aip->path_cur indices.
 *	Updates Ppfp to point to first free record.
 *	This function is fairly fast.  Its worst-case running time is proportional to
 *	3*MAX_PATH_POINTS + MAX_OBJECTS
 *
 * @todo Things to do to optimize this function:
 *		1. if (t != 0) xlt++; can be replaced by xlt += t; assuming t can only be 0 or 1.
 *		2. When pp_xlate is getting stuffed the first time, note highest index and use that 
 *			instead of MAX_PATH_POINTS in following two for loops.
 */
void garbage_collect_path_points()
{
	int	i;
	int	pp_xlate[MAX_PATH_POINTS];
	object	*A;
	ship_obj	*so;

	//	Scan all objects and create Path_points xlate table.
	for (i=0; i<MAX_PATH_POINTS; i++)
		pp_xlate[i] = 0;

	//	in pp_xlate, mark all used Path_point records
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		A = &Objects[so->objnum];
		ship	*shipp = &Ships[A->instance];
		if (shipp->ai_index != -1) {
			ai_info	*aip = &Ai_info[shipp->ai_index];

			if ((aip->path_length > 0) && (aip->path_start > -1)) {

				for (i=aip->path_start; i<aip->path_start + aip->path_length; i++) {
					Assert(pp_xlate[i] == 0);	//	If this is not 0, then two paths use this point!
					pp_xlate[i] = 1;
				}
			}
		}
	}

	//	Now, stuff xlate index in pp_xlate.  This is the number to translate any path_start
	//	or path_cur index to.
	int	xlt = 0;
	for (i=0; i<MAX_PATH_POINTS; i++) {
		int	t = pp_xlate[i];

		pp_xlate[i] = xlt;
		if (t != 0)
			xlt++;
	}
	
	//	Update global Path_points free pointer.
	Ppfp = &Path_points[xlt];

	//	Now, using pp_xlate, fixup all aip->path_cur and aip->path_start indices
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		A = &Objects[so->objnum];
		ship	*shipp = &Ships[A->instance];
		if (shipp->ai_index != -1) {
			ai_info	*aip = &Ai_info[shipp->ai_index];

			if ((aip->path_length > 0) && (aip->path_start > -1)) {
				Assert(aip->path_start < MAX_PATH_POINTS);
				aip->path_start = pp_xlate[aip->path_start];

				Assert((aip->path_cur >= 0) && (aip->path_cur < MAX_PATH_POINTS));
				aip->path_cur = pp_xlate[aip->path_cur];
			}
		}
	}

	//	Now, compress the buffer.
	for (i=0; i<MAX_PATH_POINTS; i++)
		if (i != pp_xlate[i])
			Path_points[pp_xlate[i]] = Path_points[i];

}

/**
 * Hash two values together, return result.
 *	Hash function: curval shifted right circular by one, newval xored in.
 */
int hash(unsigned int curval, int newval)
{
	int	addval = curval & 1;

	curval >>= 1;
	if (addval)
		curval |= 0x80000000;
	curval ^= newval;

	return curval;
}

/**
 * Hash some information in an object together.
 *	On 2/20/97, the information is position and orientation.
 */
int create_object_hash(object *objp)
{
	int	*ip;
	unsigned int	hashval = 0;
	int	i;

	ip = (int *) &objp->orient;

	for (i=0; i<9; i++) {
		hashval = hash(hashval, *ip);
		ip++;
	}

	ip = (int *) &objp->pos;

	for (i=0; i<3; i++) {
		hashval = hash(hashval, *ip);
		ip++;
	}

	return hashval;
}

void free_ai_stuff()
{
	if(Ai_classes != NULL)
		vm_free(Ai_classes);
	
	if(Ai_class_names != NULL)
		vm_free(Ai_class_names);
}

/**
 * Initialize an AI class's nonrequired values to defaults
 * Boolean overrides are unset, others initialized to FLT_MIN or INT_MIN (used as the "not set" state)
 */
void init_ai_class(ai_class *aicp)
{
	int i;
	for (i = 0; i < NUM_SKILL_LEVELS; i++)
	{
		aicp->ai_cmeasure_fire_chance[i] = FLT_MIN;
		aicp->ai_in_range_time[i] = FLT_MIN;
		aicp->ai_link_ammo_levels_maybe[i] = FLT_MIN;
		aicp->ai_link_ammo_levels_always[i] = FLT_MIN;
		aicp->ai_primary_ammo_burst_mult[i] = FLT_MIN;
		aicp->ai_link_energy_levels_maybe[i] = FLT_MIN;
		aicp->ai_link_energy_levels_always[i] = FLT_MIN;
		aicp->ai_predict_position_delay[i] = INT_MIN;
		aicp->ai_shield_manage_delay[i] = FLT_MIN;
		aicp->ai_ship_fire_delay_scale_friendly[i] = FLT_MIN;
		aicp->ai_ship_fire_delay_scale_hostile[i] = FLT_MIN;
		aicp->ai_ship_fire_secondary_delay_scale_friendly[i] = FLT_MIN;
		aicp->ai_ship_fire_secondary_delay_scale_hostile[i] = FLT_MIN;
		aicp->ai_turn_time_scale[i] = FLT_MIN;
		aicp->ai_glide_attack_percent[i] = FLT_MIN;
		aicp->ai_circle_strafe_percent[i] = FLT_MIN;
		aicp->ai_glide_strafe_percent[i] = FLT_MIN;
		aicp->ai_random_sidethrust_percent[i] = FLT_MIN;
		aicp->ai_stalemate_time_thresh[i] = FLT_MIN;
		aicp->ai_stalemate_dist_thresh[i] = FLT_MIN;
		aicp->ai_chance_to_use_missiles_on_plr[i] = INT_MIN;
		aicp->ai_max_aim_update_delay[i] = FLT_MIN;
		aicp->ai_turret_max_aim_update_delay[i] = FLT_MIN;
	}
	aicp->ai_profile_flags = 0;
	aicp->ai_profile_flags_set = 0;
	aicp->ai_profile_flags2 = 0;
	aicp->ai_profile_flags2_set = 0;

	//AI Class autoscale overrides
	//INT_MIN and FLT_MIN represent the "not set" state
	for (i = 0; i < NUM_SKILL_LEVELS; i++)
	{
		aicp->ai_aburn_use_factor[i] = INT_MIN;
		aicp->ai_shockwave_evade_chance[i] = FLT_MIN;
		aicp->ai_get_away_chance[i] = FLT_MIN;
		aicp->ai_secondary_range_mult[i] = FLT_MIN;
	}
	aicp->ai_class_autoscale = true;	//Retail behavior is to do the stupid autoscaling
}

void set_aic_flag(ai_class *aicp, char *name, int flag, int type)
{
	int* flags = (type == AIP_FLAG) ? &(aicp->ai_profile_flags) : &(aicp->ai_profile_flags2);
	int* set = (type == AIP_FLAG) ? &(aicp->ai_profile_flags_set) : &(aicp->ai_profile_flags2_set);

	if (optional_string(name))
	{
		bool val;
		stuff_boolean(&val);

		if (val)
			*flags |= flag;
		else
			*flags &= ~flag;

		*set |= flag;
	}
	else
		*set &= ~flag;
}

void parse_ai_class()
{
	ai_class	*aicp = &Ai_classes[Num_ai_classes];

	init_ai_class(aicp);

	required_string("$Name:");
	stuff_string(aicp->name, F_NAME, NAME_LENGTH);

	Ai_class_names[Num_ai_classes] = aicp->name;

	required_string("$accuracy:");
	parse_float_list(aicp->ai_accuracy, NUM_SKILL_LEVELS);

	required_string("$evasion:");
	parse_float_list(aicp->ai_evasion, NUM_SKILL_LEVELS);

	required_string("$courage:");
	parse_float_list(aicp->ai_courage, NUM_SKILL_LEVELS);

	required_string("$patience:");
	parse_float_list(aicp->ai_patience, NUM_SKILL_LEVELS);

	if (optional_string("$Afterburner Use Factor:"))
		parse_int_list(aicp->ai_aburn_use_factor, NUM_SKILL_LEVELS);

	if (optional_string("$Shockwave Evade Chances Per Second:"))
		parse_float_list(aicp->ai_shockwave_evade_chance, NUM_SKILL_LEVELS);

	if (optional_string("$Get Away Chance:"))
		parse_float_list(aicp->ai_get_away_chance, NUM_SKILL_LEVELS);

	if (optional_string("$Secondary Range Multiplier:"))
		parse_float_list(aicp->ai_secondary_range_mult, NUM_SKILL_LEVELS);

	if (optional_string("$Autoscale by AI Class Index:"))
		stuff_boolean(&aicp->ai_class_autoscale);

	//Parse optional values for stuff imported from ai_profiles
	if (optional_string("$AI Countermeasure Firing Chance:"))
		parse_float_list(aicp->ai_cmeasure_fire_chance, NUM_SKILL_LEVELS);

	if (optional_string("$AI In Range Time:"))
		parse_float_list(aicp->ai_in_range_time, NUM_SKILL_LEVELS);

	if (optional_string("$AI Always Links Ammo Weapons:"))
		parse_float_list(aicp->ai_link_ammo_levels_always, NUM_SKILL_LEVELS);

	if (optional_string("$AI Maybe Links Ammo Weapons:"))
		parse_float_list(aicp->ai_link_ammo_levels_maybe, NUM_SKILL_LEVELS);

	if (optional_string("$Primary Ammo Burst Multiplier:"))
		parse_float_list(aicp->ai_primary_ammo_burst_mult, NUM_SKILL_LEVELS);

	if (optional_string("$AI Always Links Energy Weapons:"))
		parse_float_list(aicp->ai_link_energy_levels_always, NUM_SKILL_LEVELS);

	if (optional_string("$AI Maybe Links Energy Weapons:"))
		parse_float_list(aicp->ai_link_energy_levels_maybe, NUM_SKILL_LEVELS);

	// represented in fractions of F1_0
	if (optional_string("$Predict Position Delay:")) {
		int iLoop;
		float temp_list[NUM_SKILL_LEVELS];
		parse_float_list(temp_list, NUM_SKILL_LEVELS);
		for (iLoop = 0; iLoop < NUM_SKILL_LEVELS; iLoop++)
			aicp->ai_predict_position_delay[iLoop] = fl2f(temp_list[iLoop]);
	}

	if (optional_string("$AI Shield Manage Delay:") || optional_string("$AI Shield Manage Delays:"))
		parse_float_list(aicp->ai_shield_manage_delay, NUM_SKILL_LEVELS);

	if (optional_string("$Friendly AI Fire Delay Scale:"))
		parse_float_list(aicp->ai_ship_fire_delay_scale_friendly, NUM_SKILL_LEVELS);

	if (optional_string("$Hostile AI Fire Delay Scale:"))
		parse_float_list(aicp->ai_ship_fire_delay_scale_hostile, NUM_SKILL_LEVELS);

	if (optional_string("$Friendly AI Secondary Fire Delay Scale:"))
		parse_float_list(aicp->ai_ship_fire_secondary_delay_scale_friendly, NUM_SKILL_LEVELS);

	if (optional_string("$Hostile AI Secondary Fire Delay Scale:"))
		parse_float_list(aicp->ai_ship_fire_secondary_delay_scale_hostile, NUM_SKILL_LEVELS);

	if (optional_string("$AI Turn Time Scale:"))
		parse_float_list(aicp->ai_turn_time_scale, NUM_SKILL_LEVELS);

	if (optional_string("$Glide Attack Percent:")) {
		parse_float_list(aicp->ai_glide_attack_percent, NUM_SKILL_LEVELS);
		for (int i = 0; i < NUM_SKILL_LEVELS; i++) {
			if (aicp->ai_glide_attack_percent[i] < 0.0f || aicp->ai_glide_attack_percent[i] > 100.0f) {
				aicp->ai_glide_attack_percent[i] = 0.0f;
				Warning(LOCATION, "$Glide Attack Percent should be between 0 and 100.0 (read %f). Setting to 0.", aicp->ai_glide_attack_percent[i]);
			}
			aicp->ai_glide_attack_percent[i] /= 100.0;
		}

	}

	if (optional_string("$Circle Strafe Percent:")) {
		parse_float_list(aicp->ai_circle_strafe_percent, NUM_SKILL_LEVELS);
		for (int i = 0; i < NUM_SKILL_LEVELS; i++) {
			if (aicp->ai_circle_strafe_percent[i] < 0.0f || aicp->ai_circle_strafe_percent[i] > 100.0f) {
				aicp->ai_circle_strafe_percent[i] = 0.0f;
				Warning(LOCATION, "$Circle Strafe Percent should be between 0 and 100.0 (read %f). Setting to 0.", aicp->ai_circle_strafe_percent[i]);
			}
			aicp->ai_circle_strafe_percent[i] /= 100.0;
		}
	}

	if (optional_string("$Glide Strafe Percent:")) {
		parse_float_list(aicp->ai_glide_strafe_percent, NUM_SKILL_LEVELS);
		for (int i = 0; i < NUM_SKILL_LEVELS; i++) {
			if (aicp->ai_glide_strafe_percent[i] < 0.0f || aicp->ai_glide_strafe_percent[i] > 100.0f) {
				aicp->ai_glide_strafe_percent[i] = 0.0f;
				Warning(LOCATION, "$Glide Strafe Percent should be between 0 and 100.0 (read %f). Setting to 0.", aicp->ai_glide_strafe_percent[i]);
			}
			aicp->ai_glide_strafe_percent[i] /= 100.0;
		}
	}

	if (optional_string("$Random Sidethrust Percent:")) {
		parse_float_list(aicp->ai_random_sidethrust_percent, NUM_SKILL_LEVELS);
		for (int i = 0; i < NUM_SKILL_LEVELS; i++) {
			if (aicp->ai_random_sidethrust_percent[i] < 0.0f || aicp->ai_random_sidethrust_percent[i] > 100.0f) {
				aicp->ai_random_sidethrust_percent[i] = 0.0f;
				Warning(LOCATION, "$Random Sidethrust Percent should be between 0 and 100.0 (read %f). Setting to 0.", aicp->ai_random_sidethrust_percent[i]);
			}
			aicp->ai_random_sidethrust_percent[i] /= 100.0;
		}
	}

	if (optional_string("$Stalemate Time Threshold:"))
		parse_float_list(aicp->ai_stalemate_time_thresh, NUM_SKILL_LEVELS);

	if (optional_string("$Stalemate Distance Threshold:"))
		parse_float_list(aicp->ai_stalemate_dist_thresh, NUM_SKILL_LEVELS);

	if (optional_string("$Chance AI Has to Fire Missiles at Player:"))
		parse_int_list(aicp->ai_chance_to_use_missiles_on_plr, NUM_SKILL_LEVELS);

	if (optional_string("$Max Aim Update Delay:"))
		parse_float_list(aicp->ai_max_aim_update_delay, NUM_SKILL_LEVELS);

	if (optional_string("$Turret Max Aim Update Delay:"))
		parse_float_list(aicp->ai_turret_max_aim_update_delay, NUM_SKILL_LEVELS);

	set_aic_flag(aicp, "$big ships can attack beam turrets on untargeted ships:", AIPF_BIG_SHIPS_CAN_ATTACK_BEAM_TURRETS_ON_UNTARGETED_SHIPS, AIP_FLAG);

	set_aic_flag(aicp, "$smart primary weapon selection:", AIPF_SMART_PRIMARY_WEAPON_SELECTION, AIP_FLAG);

	set_aic_flag(aicp, "$smart secondary weapon selection:", AIPF_SMART_SECONDARY_WEAPON_SELECTION, AIP_FLAG);

	set_aic_flag(aicp, "$smart shield management:", AIPF_SMART_SHIELD_MANAGEMENT, AIP_FLAG);

	set_aic_flag(aicp, "$smart afterburner management:", AIPF_SMART_AFTERBURNER_MANAGEMENT, AIP_FLAG);

	set_aic_flag(aicp, "$allow rapid secondary dumbfire:", AIPF_ALLOW_RAPID_SECONDARY_DUMBFIRE, AIP_FLAG);
	
	set_aic_flag(aicp, "$huge turret weapons ignore bombs:", AIPF_HUGE_TURRET_WEAPONS_IGNORE_BOMBS, AIP_FLAG);

	set_aic_flag(aicp, "$don't insert random turret fire delay:", AIPF_DONT_INSERT_RANDOM_TURRET_FIRE_DELAY, AIP_FLAG);

	set_aic_flag(aicp, "$prevent turrets targeting too distant bombs:", AIPF_PREVENT_TARGETING_BOMBS_BEYOND_RANGE, AIP_FLAG);

	set_aic_flag(aicp, "$smart subsystem targeting for turrets:", AIPF_SMART_SUBSYSTEM_TARGETING_FOR_TURRETS, AIP_FLAG);

	set_aic_flag(aicp, "$allow turrets target weapons freely:", AIPF_ALLOW_TURRETS_TARGET_WEAPONS_FREELY, AIP_FLAG);

	set_aic_flag(aicp, "$allow vertical dodge:", AIPF_ALLOW_VERTICAL_DODGE, AIP_FLAG);

	set_aic_flag(aicp, "$no extra collision avoidance vs player:", AIPF2_NO_SPECIAL_PLAYER_AVOID, AIP_FLAG2);

	set_aic_flag(aicp, "$all ships manage shields:", AIPF2_ALL_SHIPS_MANAGE_SHIELDS, AIP_FLAG2);
}

void reset_ai_class_names()
{
	ai_class *aicp;

	for (int i = 0; i < Num_ai_classes; i++) {
		aicp = &Ai_classes[i];

		Ai_class_names[i] = aicp->name;
	}
}

#define AI_CLASS_INCREMENT		10
void parse_aitbl()
{
	// open localization
	lcl_ext_open();

	read_file_text("ai.tbl", CF_TYPE_TABLES);
	reset_parse();

	//Just in case parse_aitbl is called twice
	free_ai_stuff();
	
	Num_ai_classes = 0;
	Num_alloced_ai_classes = AI_CLASS_INCREMENT;
	Ai_classes = (ai_class*) vm_malloc(Num_alloced_ai_classes * sizeof(ai_class));
	Ai_class_names = (char**) vm_malloc(Num_alloced_ai_classes * sizeof(char*));
	
	required_string("#AI Classes");

	while (required_string_either("#End", "$Name:")) {

		parse_ai_class();

		Num_ai_classes++;

		if(Num_ai_classes >= Num_alloced_ai_classes)
		{
			Num_alloced_ai_classes += AI_CLASS_INCREMENT;
			Ai_classes = (ai_class*) vm_realloc(Ai_classes, Num_alloced_ai_classes * sizeof(ai_class));

			// Ai_class_names doesn't realloc all that well so we have to do it the hard way.
			// Luckily, it's contents can be easily replaced so we don't have to save anything.
			vm_free(Ai_class_names);
			Ai_class_names = (char **) vm_malloc(Num_alloced_ai_classes * sizeof(char*));
			reset_ai_class_names();
		}
	}

	// close localization
	lcl_ext_close();
	
	atexit(free_ai_stuff);
}

LOCAL int ai_inited = 0;

//========================= BOOK-KEEPING FUNCTIONS =======================

/**
 * Called once at game start-up
 */
void ai_init()
{
	if ( !ai_inited )	{
		// Do the first time initialization stuff here
		int	rval;

		if ((rval = setjmp(parse_abort)) != 0) {
			mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "ai.tbl", rval));
			lcl_ext_close();
		} else {			
			parse_aitbl();			
		}

		ai_inited = 1;
	}

	init_semirand();
	
	ai_level_init();
}

/**
 * This inits the AI.  You should be able to call this between
 * levels to reset everything.
 */
void ai_level_init()
{
	int i;
 
	// Do the stuff to reset all ai stuff here
	for (i=0; i<MAX_AI_INFO ; i++) {
		Ai_info[i].shipnum = -1;
	}

	Ai_goal_signature = 0;

	for (i = 0; i < Num_iffs; i++)
		Iff_info[i].ai_rearm_timestamp = timestamp(-1);

	// clear out the stuff needed for AI firing powerful secondary weapons
	ai_init_secondary_info();

	Ai_last_arrive_path=0;
}

// BEGIN STEALTH
// -----------------------------------------------------------------------------

/**
 * Check if object is a stealth ship
 */
bool is_object_stealth_ship(object* objp)
{
	if (objp->type == OBJ_SHIP) {
		if (Ships[objp->instance].flags2 & SF2_STEALTH) {
			return true;
		}
	}

	// not stealth ship
	return false;
}

/**
 * Init necessary AI info for new stealth target
 */
void init_ai_stealth_info(ai_info *aip, object *stealth_objp)
{
	Assert(is_object_stealth_ship(stealth_objp));

	// set necessary ai info for new stealth target
	aip->stealth_last_pos = stealth_objp->pos;
	aip->stealth_velocity = stealth_objp->phys_info.vel;
	aip->stealth_last_visible_stamp = timestamp();
}

// -----------------------------------------------------------------------------
// Check whether Pl_objp can see a stealth ship object
#define STEALTH_INVISIBLE			0
#define STEALTH_VISIBLE				1
#define STEALTH_FULLY_TARGETABLE	2

/**
 * Return dist scaler based on skill level
 */
float get_skill_stealth_dist_scaler()
{
	switch (Game_skill_level) {
	case 0: // very easy
		return 0.65f;

	case 1:	// easy
		return 0.9f;

	case 2:	// medium
		return 1.0f;

	case 3:	// hard
		return 1.1f;

	case 4:	// insane
		return 1.3f;

	default:
		Int3();
	}

	return 1.0f;
}

/**
 * Return multiplier on dot based on skill level
 */
float get_skill_stealth_dot_scaler()
{
	switch (Game_skill_level) {
	case 0: // very easy
		return 1.3f;

	case 1:	// easy
		return 1.1f;

	case 2:	// medium
		return 1.0f;

	case 3:	// hard
		return 0.9f;

	case 4:	// insane
		return 0.7f;

	default:
		Int3();
	}

	return 1.0f;
}

int ai_is_stealth_visible(object *viewer_objp, object *stealth_objp)
{
	ship *shipp;
	vec3d vec_to_stealth;
	float dot_to_stealth, dist_to_stealth, max_stealth_dist;

	Assert(stealth_objp->type == OBJ_SHIP);
	shipp = &Ships[stealth_objp->instance];
	Assert(viewer_objp->type == OBJ_SHIP);

	// check if stealth ship
	Assert(shipp->flags2 & SF2_STEALTH);

	// check if in neb and below awac level for visible
	if ( !ship_is_visible_by_team(stealth_objp, &Ships[viewer_objp->instance]) ) {
		vm_vec_sub(&vec_to_stealth, &stealth_objp->pos, &viewer_objp->pos);
		dist_to_stealth = vm_vec_mag_quick(&vec_to_stealth);
		dot_to_stealth = vm_vec_dotprod(&viewer_objp->orient.vec.fvec, &vec_to_stealth) / dist_to_stealth;

		// get max dist at which stealth is visible
		max_stealth_dist = get_skill_stealth_dist_scaler() * STEALTH_MAX_VIEW_DIST;

		// now check if within view frustrum
		float needed_dot_to_stealth;
		if (dist_to_stealth < 100) {
			needed_dot_to_stealth = 0.0f;
		} else {
			needed_dot_to_stealth = get_skill_stealth_dot_scaler() * float(STEALTH_VIEW_CONE_DOT) * (dist_to_stealth / max_stealth_dist);
		}
		if (dot_to_stealth > needed_dot_to_stealth) {
			if (dist_to_stealth < max_stealth_dist) {
				return STEALTH_VISIBLE;
			}
		}

		// not within frustrum
		return STEALTH_INVISIBLE;
	}

	// visible by awacs level
	return STEALTH_FULLY_TARGETABLE;
}

// END STEALTH

/**
 * Compute dot product of direction vector and forward vector.
 * Direction vector is vector from one object to other object.
 * Forward vector is the forward vector of the ship.
 * If from_dot == NULL, don't fill it in.
 */
float compute_dots(object *objp, object *other_objp, float *to_dot, float *from_dot)
{
	vec3d	v2o;
	float		dist;

	dist = vm_vec_normalized_dir(&v2o, &other_objp->pos, &objp->pos);

	*to_dot = vm_vec_dot(&objp->orient.vec.fvec, &v2o);

	if (from_dot != NULL)
		*from_dot = - vm_vec_dot(&other_objp->orient.vec.fvec, &v2o);

	return dist;
}

/**
 * Update estimated stealth info
 * 
 * This is a "cheat" update. Error increases with time not seen, true distance away, dot to enemy
 * this is done only if we can not see the stealth target
 * need to infer its position either by weapon fire pos or last know pos
 */
void update_ai_stealth_info_with_error(ai_info *aip)
{
	object *ship;
	object *stealth_objp;

	// make sure I am targeting a stealth ship
	Assert( is_object_stealth_ship(&Objects[aip->target_objnum]) );
	stealth_objp = &Objects[aip->target_objnum];

	// my_ship
	ship = &Objects[Ships[aip->shipnum].objnum];

	// if update is due to weapon fire, get exact stealth position
	aip->stealth_last_pos = stealth_objp->pos;
	aip->stealth_velocity = stealth_objp->phys_info.vel;
	aip->stealth_last_visible_stamp = timestamp();
}

/**
 * Update danger_weapon_objnum and signature in ai_info to say this weapon is to be avoided.
 */
void ai_update_danger_weapon(int attacked_objnum, int weapon_objnum)
{
	object	*objp, *weapon_objp;
	ai_info	*aip;
	float		old_dist, new_dist;
	float		old_dot, new_dot;
	object	*old_weapon_objp = NULL;

	if ((attacked_objnum == -1) || (weapon_objnum == -1)) {
		return;
	}

	objp = &Objects[attacked_objnum];

	// AL 2-24-98: If this isn't a ship, we don't need to worry about updating weapon_objnum (ie it would be
	//					an asteroid or bomb).
	if ( objp->type != OBJ_SHIP ) {
		return;
	}

	weapon_objp = &Objects[weapon_objnum];

	aip = &Ai_info[Ships[objp->instance].ai_index];

	// if my target is a stealth ship and is not visible
	if (aip->target_objnum >= 0) {
		if ( is_object_stealth_ship(&Objects[aip->target_objnum]) ) {
			if ( ai_is_stealth_visible(objp, &Objects[aip->target_objnum]) == STEALTH_INVISIBLE ) {
				// and the weapon is coming from that stealth ship
				if (weapon_objp->parent == aip->target_objnum) {
					// update my position estimate for stealth ship
					update_ai_stealth_info_with_error(aip);
				}
			}
		}
	}

	if (aip->danger_weapon_objnum != -1) {
		old_weapon_objp = &Objects[aip->danger_weapon_objnum];
		if ((old_weapon_objp->type == OBJ_WEAPON) && (old_weapon_objp->signature == aip->danger_weapon_signature)) {
			;
		} else {
			aip->danger_weapon_objnum = -1;
		}
	}

	new_dist = compute_dots(weapon_objp, objp, &new_dot, NULL);

	if (aip->danger_weapon_objnum == -1) {
		if (new_dist < 1500.0f) {
			if (new_dot > 0.5f) {
				aip->danger_weapon_objnum = weapon_objnum;
				aip->danger_weapon_signature = weapon_objp->signature;
			}
		}
	} else {
		Assert(old_weapon_objp != NULL);
		old_dist = compute_dots(old_weapon_objp, objp, &old_dot, NULL);
	
		if (old_dot < 0.5f) {
			aip->danger_weapon_objnum = -1;
			old_dist = 9999.9f;
		}

		if ((new_dot > 0.5f) && (new_dot > old_dot-0.01f)) {
			if (new_dist < old_dist) {
				aip->danger_weapon_objnum = weapon_objnum;
				aip->danger_weapon_signature = weapon_objp->signature;
			}
		}
	}
}

//	If rvec != NULL, use it to match bank by calling vm_matrix_interpolate.
//	(rvec defaults to NULL)
void ai_turn_towards_vector(vec3d *dest, object *objp, float frametime, float turn_time, vec3d *slide_vec, vec3d *rel_pos, float bank_override, int flags, vec3d *rvec, int sexp_flags)
{
	matrix	curr_orient;
	vec3d	vel_in, vel_out, desired_fvec, src;
	float		delta_time;
	physics_info	*pip;
	vec3d	vel_limit, acc_limit;
	float		delta_bank;

	//	Don't allow a ship to turn if it has no engine strength.
	// AL 3-12-98: objp may not always be a ship!
	if ( (objp->type == OBJ_SHIP) && !(sexp_flags & AITTV_VIA_SEXP) ) {
		if (ship_get_subsystem_strength(&Ships[objp->instance], SUBSYSTEM_ENGINE) <= 0.0f)
			return;
	}

	pip = &objp->phys_info;

	vel_in = pip->rotvel;
	curr_orient = objp->orient;
	delta_time = flFrametime;

	if(turn_time > 0.0f)
	{
		//	Scale turn_time based on skill level and team.
		if (!(flags & AITTV_FAST) && !(sexp_flags & AITTV_VIA_SEXP) ){
			if (objp->type == OBJ_SHIP){
				if (iff_x_attacks_y(Player_ship->team, Ships[objp->instance].team)) {
					turn_time *= Ai_info[Ships[objp->instance].ai_index].ai_turn_time_scale;
				}
			}
		}

		//	Set max turn rate.
		vel_limit.xyz.x = PI2/turn_time;
		vel_limit.xyz.y = PI2/turn_time;
		vel_limit.xyz.z = PI2/turn_time;
	}
	else
	{
		vm_vec_zero(&vel_limit);
	}

	//	Set rate at which ship can accelerate to its rotational velocity.
	//	For now, weapons just go much faster.
	acc_limit = vel_limit;
	if (objp->type == OBJ_WEAPON)
		vm_vec_scale(&acc_limit, 8.0f);

	src = objp->pos;

	if (rel_pos != NULL) {
		vec3d	gun_point;
		vm_vec_unrotate(&gun_point, rel_pos, &objp->orient);
		vm_vec_add2(&src, &gun_point);
	}

	vm_vec_normalized_dir(&desired_fvec, dest, &src);

	//	Since ship isn't necessarily moving in the direction it's pointing, sometimes it's better
	//	to be moving towards goal rather than just pointing.  So, if slide_vec is !NULL, try to
	//	make ship move towards goal, not point at goal.
	if (slide_vec != NULL) {
		vm_vec_add2(&desired_fvec, slide_vec);
		vm_vec_normalize(&desired_fvec);
	}

	//	Should be more general case here.  Currently, anything that is not a weapon will bank when it turns.
	// Goober5000 - don't bank if sexp says not to
	if ( (objp->type == OBJ_WEAPON) || (sexp_flags & AITTV_IGNORE_BANK ) )
		delta_bank = 0.0f;
	else if ((bank_override) && (iff_x_attacks_y(Ships[objp->instance].team, Player_ship->team))) {	//	Theoretically, this will only happen for Shivans.
		delta_bank = bank_override;
	} else {
		delta_bank = vm_vec_dot(&curr_orient.vec.rvec, &objp->last_orient.vec.rvec);
		delta_bank = 100.0f * (1.0f - delta_bank);
		if (vm_vec_dot(&objp->last_orient.vec.fvec, &objp->orient.vec.rvec) < 0.0f)
			delta_bank = -delta_bank;
	}

	//	Dave Andsager: The non-indented lines here are debug code to help you track down the problem in the physics
	//	that is causing ships to inexplicably rotate very far.  If you hit the Int3(), set the next statement to be
	//	the one marked "HERE".  (Do this clicking the cursor there, then right clicking.  Choose the right option.)
	//	This will allow you to rerun vm_forward_interpolate() with the values that caused the error.
	//	Note, you'll need to enable the Int3() about ten lines below.
#ifndef NDEBUG
vec3d tvec = objp->orient.vec.fvec;
vec3d	vel_in_copy;
matrix	objp_orient_copy;

vel_in_copy = vel_in;
objp_orient_copy = objp->orient;

vel_in = vel_in_copy;	//	HERE //-V587
objp->orient = objp_orient_copy; //-V587
#endif
	
	if (rvec != NULL) {
		matrix	out_orient, goal_orient;

		vm_vector_2_matrix(&goal_orient, &desired_fvec, NULL, rvec);
		vm_matrix_interpolate(&goal_orient, &curr_orient, &vel_in, delta_time, &out_orient, &vel_out, &vel_limit, &acc_limit);
		objp->orient = out_orient;
	} else {
		vm_forward_interpolate(&desired_fvec, &curr_orient, &vel_in, delta_time, delta_bank, &objp->orient, &vel_out, &vel_limit, &acc_limit);
	}
	
#ifndef NDEBUG
if (!((objp->type == OBJ_WEAPON) && (Weapon_info[Weapons[objp->instance].weapon_info_index].subtype == WP_MISSILE))) {
	Assertion(!(delta_time < 0.25f && vm_vec_dot(&objp->orient.vec.fvec, &tvec) < 0.1f), "A ship rotated too far. Offending vessel is %s, please investigate.\n", Ships[objp->instance].ship_name);
}
#endif

	pip->rotvel = vel_out;
}

//	Set aip->target_objnum to objnum
//	Update aip->previous_target_objnum.
//	If new target (objnum) is different than old target, reset target_time.
int set_target_objnum(ai_info *aip, int objnum)
{
	if ((aip != Player_ai) && (!timestamp_elapsed(aip->ok_to_target_timestamp))) {
		return aip->target_objnum;
	}

	if (aip->target_objnum == objnum) {
		aip->previous_target_objnum = aip->target_objnum;
	} else {
		aip->previous_target_objnum = aip->target_objnum;

		// ignore this assert if a multiplayer observer
		if((Game_mode & GM_MULTIPLAYER) && (aip == Player_ai) && (Player_obj->type == OBJ_OBSERVER)){
		} else {
			Assert(objnum != Ships[aip->shipnum].objnum);	//	make sure not targeting self
		}

		// if stealth target, init ai_info for stealth
		if ( (objnum >= 0) && is_object_stealth_ship(&Objects[objnum]) ) {
			init_ai_stealth_info(aip, &Objects[objnum]);
		}

		aip->target_objnum = objnum;
		aip->target_time = 0.0f;
		aip->time_enemy_near = 0.0f;				//SUSHI: Reset the "time near" counter whenever the target is changed
		aip->last_hit_target_time = Missiontime;	//Also, the "last hit target" time should be reset when the target changes
		aip->target_signature = (objnum >= 0) ? Objects[objnum].signature : -1;
		// clear targeted subsystem
		set_targeted_subsys(aip, NULL, -1);
	}
	
	return aip->target_objnum;
}

int ai_select_primary_weapon(object *objp, object *other_objp, int flags);

/**
 * Make new_subsys the targeted subsystem of ship *aip.
 */
ship_subsys *set_targeted_subsys(ai_info *aip, ship_subsys *new_subsys, int parent_objnum)
{
	Assert(aip != NULL);

	aip->last_subsys_target = aip->targeted_subsys;
	aip->targeted_subsys = new_subsys;
	aip->targeted_subsys_parent = parent_objnum;

	if ( new_subsys ) {
		// Make new_subsys target
		if (new_subsys->system_info->type == SUBSYSTEM_ENGINE) {
			if ( aip != Player_ai ) {
				Assert( aip->shipnum >= 0 );
				ai_select_primary_weapon(&Objects[Ships[aip->shipnum].objnum], &Objects[parent_objnum], WIF_PUNCTURE);
				ship_primary_changed(&Ships[aip->shipnum]);	// AL: maybe send multiplayer information when AI ship changes primaries
			}
		}

		if ( aip == Player_ai ) {
			hud_lock_reset(0.5f);
		}

	} else {
		// Cleanup any subsys path information if it exists
		ai_big_subsys_path_cleanup(aip);
	}
	
	return aip->targeted_subsys;
}											  

/**
 * Called to init the data for single ai object.  
 *
 * At this point, the ship and the object and the ai_info are are correctly
 * linked together. Ai_info[ai_index].shipnum is the only valid field 
 * in ai_info.
 *
 *	This is called right when the object is parsed, so you can't assume much
 *	has been initialized.  For example, wings, waypoints, goals are probably
 *	not yet loaded. --MK, 10/8/96
 */
void ai_object_init(object * obj, int ai_index)
{
	ai_info	*aip;
	Assert(ai_index >= 0 && ai_index < MAX_AI_INFO);

	aip = &Ai_info[ai_index];

	aip->type = 0;		//	0 means not in use.
	aip->wing = -1;		//	Member of what wing? -1 means none.
	aip->ai_class = Ship_info[Ships[obj->instance].ship_info_index].ai_class;
	aip->behavior = AIM_NONE;
}

/**
 * If *aip is docked, set max acceleration to A->mass/(A->mass + B->mass) where A is *aip and B is dock object
 */
void adjust_accel_for_docking(ai_info *aip)
{
	object *objp = &Objects[Ships[aip->shipnum].objnum];

	if (object_is_docked(objp))
	{
		float ratio = objp->phys_info.mass / dock_calc_total_docked_mass(objp);

		// put cap on how much ship can slow down
		if ( (ratio < 0.8f) && !(The_mission.ai_profile->flags & AIPF_NO_MIN_DOCK_SPEED_CAP) ) {
			ratio = 0.8f;
		}

		// make sure we at least some velocity
		if (ratio < 0.1f) {
			ratio = 0.1f;
		}

		if (AI_ci.forward > ratio) {
			AI_ci.forward = ratio;
		}
	}
}

void accelerate_ship(ai_info *aip, float accel)
{
	aip->prev_accel = accel;
	AI_ci.forward = accel;
	adjust_accel_for_docking(aip);
}

void change_acceleration(ai_info *aip, float delta_accel)
{
	float	new_accel;

	if (delta_accel < 0.0f) {
		if (aip->prev_accel > 0.0f)
			aip->prev_accel = 0.0f;
	} else if (aip->prev_accel < 0.0f)
		aip->prev_accel = 0.0f;

	new_accel = aip->prev_accel + delta_accel * flFrametime;

	if (new_accel > 1.0f)
		new_accel = 1.0f;
	else if (new_accel < -1.0f)
		new_accel = -1.0f;
	
	aip->prev_accel = new_accel;

	AI_ci.forward = new_accel;
	adjust_accel_for_docking(aip);
}

void set_accel_for_target_speed(object *objp, float tspeed)
{
	float	max_speed;
	ai_info	*aip;

	aip = &Ai_info[Ships[objp->instance].ai_index];

	max_speed = Ships[objp->instance].current_max_speed;

	if (max_speed > 0.0f) {
		AI_ci.forward = tspeed/max_speed;
	} else {
		AI_ci.forward = 0.0f;
	}

	aip->prev_accel = AI_ci.forward;

	adjust_accel_for_docking(aip);
}

/**
 * Stuff perim_point with a point on the perimeter of the sphere defined by object *objp
 * on the vector from the center of *objp through the point *vp.
 */
void project_point_to_perimeter(vec3d *perim_point, vec3d *pos, float radius, vec3d *vp)
{
	vec3d	v1;
	float		mag;

	vm_vec_sub(&v1, vp, pos);
	mag = vm_vec_mag(&v1);

	if (mag == 0.0f) {
		Warning(LOCATION, "projectable point is at center of sphere.");
		vm_vec_make(&v1, 0.0f, radius, 0.0f);
	} else {
		vm_vec_normalize(&v1);
		vm_vec_scale(&v1, 1.1f * radius + 10.0f);
	}

	vm_vec_add2(&v1, pos);
	*perim_point = v1;
}

/**
 * Stuff tan1 with tangent point on sphere.
 *
 * @param tan1		is point nearer to *p1
 * @param *p0		is point through which tangents pass.
 * @param *centerp	is center of sphere.
 * @param *p1		is another point in space to define the plane in which tan1, tan2 reside.
 * @param radius	is the radius of the sphere.
 *
 * Note, this is a very approximate function just for AI.
 * Note also: On 12/26/96, p1 is used to define the plane perpendicular to that which
 * contains the tangent point.
 */
void get_tangent_point(vec3d *tan1, vec3d *p0, vec3d *centerp, vec3d *p1, float radius)
{
	vec3d	dest_vec, v2c, perp_vec, temp_vec, v2;
	float		dist, ratio;

	//	Detect condition of point inside sphere.
	if (vm_vec_dist(p0, centerp) < radius)
		project_point_to_perimeter(tan1, centerp, radius, p0);
	else {
		vm_vec_normalized_dir(&v2c, centerp, p0);

		//	Compute perpendicular vector using p0, centerp, p1
		vm_vec_normal(&temp_vec, p0, centerp, p1);
		vm_vec_sub(&v2, centerp, p0);
		vm_vec_cross(&perp_vec, &temp_vec, &v2);

		vm_vec_normalize(&perp_vec);

		dist = vm_vec_dist_quick(p0, centerp);
		ratio = dist / radius;

		if (ratio < 2.0f)
			vm_vec_scale_add(&dest_vec, &perp_vec, &v2c, ratio-1.0f);
		else
			vm_vec_scale_add(&dest_vec, &v2c, &perp_vec, (1.0f + 1.0f/ratio));

		vm_vec_scale_add(tan1, p0, &dest_vec, dist + radius);
	}
}

/**
 * Given an object and a point, turn towards the point, resulting in
 * approach behavior.
 */
void turn_towards_point(object *objp, vec3d *point, vec3d *slide_vec, float bank_override)
{
	ai_info	*aip;
	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];
	
	// check if in formation and if not leader, don't change rotvel.z (bank to match leader elsewhere)
	if (aip->ai_flags & AIF_FORMATION) {
		if (&Objects[aip->goal_objnum] != objp) {
			float rotvel_z = objp->phys_info.rotvel.xyz.z;
			ai_turn_towards_vector(point, objp, flFrametime, Ship_info[Ships[objp->instance].ship_info_index].srotation_time, slide_vec, NULL, bank_override, 0);
			objp->phys_info.rotvel.xyz.z = rotvel_z;
		}
	} else {
		// normal turn
		ai_turn_towards_vector(point, objp, flFrametime, Ship_info[Ships[objp->instance].ship_info_index].srotation_time, slide_vec, NULL, bank_override, 0);
	}
}

/**
 * Given an object and a point, turn away from the point, resulting in avoidance behavior.
 * Note: Turn away at full speed, not scaled down by skill level.
 */
void turn_away_from_point(object *objp, vec3d *point, float bank_override)
{
	vec3d	opposite_point;

	vm_vec_sub(&opposite_point, &objp->pos, point);
	vm_vec_add2(&opposite_point, &objp->pos);

	ai_turn_towards_vector(&opposite_point, objp, flFrametime, Ship_info[Ships[objp->instance].ship_info_index].srotation_time, NULL, NULL, bank_override, AITTV_FAST);
}


//	--------------------------------------------------------------------------
//	Given an object and a point, turn tangent to the point, resulting in
// a circling behavior.
//	Make object *objp turn around the point *point with a radius of radius.
//	Note that this isn't the same as following a circle of radius radius with
//	center *point, but it should be adequate.
//	Note that if you want to circle an object without hitting it, you should use
//	about twice that object's radius for radius, else you'll certainly bump into it.
//	Return dot product to goal point.
float turn_towards_tangent(object *objp, vec3d *point, float radius)
{
	vec3d	vec_to_point;
	vec3d	goal_point;
	vec3d	perp_point;				//	point radius away from *point on vector to objp->pos
	vec3d	up_vec, perp_vec;
	vec3d	v2g;

	vm_vec_normalized_dir(&vec_to_point, point, &objp->pos);
	vm_vec_crossprod(&up_vec, &vec_to_point, &objp->orient.vec.fvec);
	vm_vec_crossprod(&perp_vec, &vec_to_point, &up_vec);

	vm_vec_scale_add(&perp_point, point, &vec_to_point, -radius);
	if (vm_vec_dot(&objp->orient.vec.fvec, &perp_vec) > 0.0f) {
		vm_vec_scale_add(&goal_point, &perp_point, &perp_vec, radius);
	} else {
		vm_vec_scale_add(&goal_point, &perp_point, &perp_vec, -radius);
	}

	turn_towards_point(objp, &goal_point, NULL, 0.0f);

	vm_vec_normalized_dir(&v2g, &goal_point, &objp->pos);
	return vm_vec_dot(&objp->orient.vec.fvec, &v2g);
}

float turn_toward_tangent_with_axis(object *objp, object *center_objp, float radius)
{
	vec3d r_vec, theta_vec;
	vec3d center_vec, vec_on_cylinder, sph_r_vec;
	float center_obj_z;

	// find closest z of center objp
	vm_vec_sub(&sph_r_vec, &objp->pos, &center_objp->pos);
	center_obj_z = vm_vec_dotprod(&sph_r_vec, &center_objp->orient.vec.fvec);

	// find pt on axis with closest z
	vm_vec_scale_add(&center_vec, &center_objp->pos, &center_objp->orient.vec.fvec, center_obj_z);

	// get r_vec
	vm_vec_sub(&r_vec, &objp->pos, &center_vec);

	Assert( (vm_vec_dotprod(&r_vec, &center_objp->orient.vec.fvec) < 0.0001));

	// get theta vec - perp to r_vec and z_vec
	vm_vec_crossprod(&theta_vec, &center_objp->orient.vec.fvec, &r_vec);

#ifndef NDEBUG
	float mag;
	mag = vm_vec_normalize(&theta_vec);
	Assert(mag > 0.9999 && mag < 1.0001);
#endif

	vec3d temp;
	vm_vec_crossprod(&temp, &r_vec, &theta_vec);

#ifndef NDEBUG
	float dot;
	dot = vm_vec_dotprod(&temp, &center_objp->orient.vec.fvec);
	Assert( dot >0.9999 && dot < 1.0001);
#endif

	// find pt on clylinder with closest z
	vm_vec_scale_add(&vec_on_cylinder, &center_vec, &r_vec, radius);

	vec3d goal_pt, v2g;
	vm_vec_scale_add(&goal_pt, &vec_on_cylinder, &theta_vec, radius);

	turn_towards_point(objp, &goal_pt, NULL, 0.0f);

	vm_vec_normalized_dir(&v2g, &goal_pt, &objp->pos);
	return vm_vec_dot(&objp->orient.vec.fvec, &v2g);
}

/**
 * Returns a point radius units away from *point that *objp should turn towards to orbit *point
 */
void get_tangent_point(vec3d *goal_point, object *objp, vec3d *point, float radius)
{
	vec3d	vec_to_point;
	vec3d	perp_point;				//	point radius away from *point on vector to objp->pos
	vec3d	up_vec, perp_vec;

	vm_vec_normalized_dir(&vec_to_point, point, &objp->pos);
	vm_vec_crossprod(&up_vec, &vec_to_point, &objp->orient.vec.fvec);
	
	while (IS_VEC_NULL(&up_vec)) {
		vec3d	rnd_vec;

		vm_vec_rand_vec_quick(&rnd_vec);
		vm_vec_add2(&rnd_vec, &objp->orient.vec.fvec);
		vm_vec_crossprod(&up_vec, &vec_to_point, &rnd_vec);
	}

	vm_vec_crossprod(&perp_vec, &vec_to_point, &up_vec);
	vm_vec_normalize(&perp_vec);

	vm_vec_scale_add(&perp_point, point, &vec_to_point, -radius);

	if (vm_vec_dot(&objp->orient.vec.fvec, &perp_vec) > 0.0f) {
		vm_vec_scale_add(goal_point, &perp_point, &perp_vec, radius);
	} else {
		vm_vec_scale_add(goal_point, &perp_point, &perp_vec, -radius);
	}
}

int	Player_attacking_enabled = 1;

/**
 * Determine whether an object is targetable within a nebula
 */
int object_is_targetable(object *target, ship *viewer)
{
	int stealth_ship = 0;

	// if target is ship, check if visible by team
	if (target->type == OBJ_SHIP)
	{
		stealth_ship = (Ships[target->instance].flags2 & SF2_STEALTH);

		if ( ship_is_visible_by_team(target, viewer) == 1)
		{
			return 1;
		}
	}

	// for AI partially targetable works as fully targetable, except for stealth ship
	if (stealth_ship) {
		// if not team targetable, check if within frustrum
		if ( ai_is_stealth_visible(&Objects[viewer->objnum], target) == STEALTH_VISIBLE ) {
			return 1;
		} else {
			return 0;
		}
	}

	// if not fully targetable by team, check awacs level with viewer
	// allow targeting even if only only partially targetable to player
	float radar_return = awacs_get_level(target, viewer);
	if ( radar_return > 0.4 ) {
		return 1;
	} else {
		return 0;
	}
}

/**
 * Return number of enemies attacking object objnum
 */
int num_enemies_attacking(int objnum)
{
	object		*objp;
	ship			*sp;
	ship_subsys	*ssp;
	ship_obj		*so;
	int			count;

	count = 0;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];
		Assert(objp->instance != -1);
		sp = &Ships[objp->instance];

		if (Ai_info[sp->ai_index].target_objnum == objnum)
			count++;

		// consider turrets that may be attacking objnum (but only turrets on SIF_BIG_SHIP ships)
		if ( Ship_info[sp->ship_info_index].flags & SIF_BIG_SHIP ) {

			// loop through all the subsystems, check if turret has objnum as a target
			ssp = GET_FIRST(&sp->subsys_list);
			while ( ssp != END_OF_LIST( &sp->subsys_list ) ) {

				if ( ssp->system_info->type == SUBSYSTEM_TURRET ) {
					if ( (ssp->turret_enemy_objnum == objnum) && (ssp->current_hits > 0) ) {
						count++;
					}
				}
				ssp = GET_NEXT( ssp );
			} // end while
		}
	}

	return count;
}

/**
 * Scan all the ships in *objp's wing. Return the lowest maximum speed of a ship in the wing.
 *
 * Current maximum speed (based on energy settings) is shipp->current_max_speed
 */
float get_wing_lowest_max_speed(object *objp)
{
	ship		*shipp;
	ai_info	*aip;
	float		lowest_max_speed;
	int		wingnum;
	object	*o;
	ship_obj	*so;

	Assert(objp->type == OBJ_SHIP);
	Assert((objp->instance >= 0) && (objp->instance < MAX_OBJECTS));
	shipp = &Ships[objp->instance];
	Assert((shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO));
	aip = &Ai_info[shipp->ai_index];

	wingnum = aip->wing;

	lowest_max_speed = shipp->current_max_speed;

	if ( wingnum == -1 )
		return lowest_max_speed;

	Assert(wingnum >= 0);

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		o = &Objects[so->objnum];
		ship	*oshipp = &Ships[o->instance];
		ai_info	*oaip = &Ai_info[oshipp->ai_index];

		if ((oaip->mode == AIM_WAYPOINTS) && (oaip->wing == wingnum)) {
			//	Note: If a ship in the wing has a super low max speed, probably its engines are disabled.  So, fly along and
			//	ignore the poor guy.
			float	cur_max = oshipp->current_max_speed;

			if (object_is_docked(o)) {
				cur_max *= o->phys_info.mass / dock_calc_total_docked_mass(o);
			}
							
			if ((oshipp->current_max_speed > 5.0f) && (cur_max < lowest_max_speed)) {
				lowest_max_speed = cur_max;
			}
		}
	}

	return lowest_max_speed;
}


/**
 * Determine if object objnum is supposed to be ignored by object with ai_info *aip.
 * @return TRUE		if objnum is aip->ignore_objnum (and signatures match) or objnum is in ignore wing
 * @return FALSE	otherwise
 */
int is_ignore_object_sub(int *ignore_objnum, int *ignore_signature, int objnum)
{
	// Not ignoring anything.
	if (*ignore_objnum == UNUSED_OBJNUM)
	{
		return 0;									
	}
	// This means it's ignoring an object, not a wing.
	else if (*ignore_objnum >= 0)
	{
		// see if this object became invalid
		if (Objects[*ignore_objnum].signature != *ignore_signature)
		{
			// reset
			*ignore_objnum = UNUSED_OBJNUM;
		}
		// objects and signatures match
		else if (*ignore_objnum == objnum)
		{
			// found it
			return 1;
		}

		return 0;
	}
	// Ignoring a wing.
	else
	{
		Int3(); // Should never happen.  I thought I removed this behavior! -- MK, 5/17/98
		return 0;
	}
}

// Goober5000
int find_ignore_new_object_index(ai_info *aip, int objnum)
{
	int i;

	for (i = 0; i < MAX_IGNORE_NEW_OBJECTS; i++)
	{
		if (is_ignore_object_sub(&aip->ignore_new_objnums[i], &aip->ignore_new_signatures[i], objnum))
			return i;
	}

	return -1;
}

// Goober5000
int is_ignore_object(ai_info *aip, int objnum, int just_the_original = 0)
{
	// check original (retail) ignore
	if (is_ignore_object_sub(&aip->ignore_objnum, &aip->ignore_signature, objnum))
		return 1;

	// check new ignore
	if (!just_the_original)
	{
		if (find_ignore_new_object_index(aip, objnum) >= 0)
			return 1;
	}

	return 0;
}

/**
 * Given a ship with bounding box and a point, find the closest point on the bbox
 */
int get_nearest_bbox_point(object *ship_obj, vec3d *start, vec3d *box_pt)
{
	vec3d temp, rf_start;
	polymodel *pm;
	pm = model_get(Ship_info[Ships[ship_obj->instance].ship_info_index].model_num);

	// get start in ship rf
	vm_vec_sub(&temp, start, &ship_obj->pos);
	vm_vec_rotate(&rf_start, &temp, &ship_obj->orient);

	// find box_pt
	int inside = project_point_onto_bbox(&pm->mins, &pm->maxs, &rf_start, &temp);

	// get box_pt in world rf
	vm_vec_unrotate(box_pt, &temp, &ship_obj->orient);
	vm_vec_add2(box_pt, &ship_obj->pos);

	return inside;
}


typedef struct eval_nearest_objnum {
	int	objnum;
	object *trial_objp;
	int	enemy_team_mask;
	int	enemy_wing;
	float	range;
	int	max_attackers;
	int	nearest_objnum;
	float	nearest_dist;
	int	check_danger_weapon_objnum;
} eval_nearest_objnum;


void evaluate_object_as_nearest_objnum(eval_nearest_objnum *eno)
{
	ai_info	*aip;
	ship_subsys	*attacking_subsystem;
	ship *shipp = &Ships[eno->trial_objp->instance];

	aip = &Ai_info[Ships[Objects[eno->objnum].instance].ai_index];

	attacking_subsystem = aip->targeted_subsys;

	if ((attacking_subsystem != NULL) || !(eno->trial_objp->flags & OF_PROTECTED)) {
		if ( OBJ_INDEX(eno->trial_objp) != eno->objnum ) {
#ifndef NDEBUG
			if (!Player_attacking_enabled && (eno->trial_objp == Player_obj))
				return;
#endif
			//	If only supposed to attack ship in a specific wing, don't attack other ships.
			if ((eno->enemy_wing != -1) && (shipp->wingnum != eno->enemy_wing))
				return;

			//	Don't keep firing at a ship that is in its death throes.
			if (shipp->flags & SF_DYING)
				return;

			if (is_ignore_object(aip, OBJ_INDEX(eno->trial_objp)))
				return;

			if (eno->trial_objp->flags & OF_PROTECTED)
				return;

			if (shipp->flags & SF_ARRIVING)
				return;

			ship_info *sip = &Ship_info[shipp->ship_info_index];

			if (sip->flags & (SIF_NO_SHIP_TYPE | SIF_NAVBUOY))
				return;

			if (iff_matches_mask(shipp->team, eno->enemy_team_mask)) {
				float	dist;
				int	num_attacking;

				// Allow targeting of stealth in nebula by his firing at me
				// This is done for a specific ship, not generally.
				if ( !eno->check_danger_weapon_objnum ) {
					// check if can be targeted if inside nebula
					if ( !object_is_targetable(eno->trial_objp, &Ships[Objects[eno->objnum].instance]) ) {
						// check if stealth ship is visible, but not "targetable"
						if ( !((shipp->flags2 & SF2_STEALTH) && ai_is_stealth_visible(&Objects[eno->objnum], eno->trial_objp)) ) {
							return;
						}
					}
				}

				// if objnum is BIG or HUGE, find distance to bbox
				if (sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {
					vec3d box_pt;
					// check if inside bbox
					int inside = get_nearest_bbox_point(eno->trial_objp, &Objects[eno->objnum].pos, &box_pt);
					if (inside) {
						dist = 10.0f;
						// on the box
					} else {
						dist = vm_vec_dist_quick(&Objects[eno->objnum].pos, &box_pt);
					}
				} else {
					dist = vm_vec_dist_quick(&Objects[eno->objnum].pos, &eno->trial_objp->pos);
				}
				
				//	Make it more likely that fighters (or bombers) will be picked as an enemy by scaling up distance for other types.
				if ((Ship_info[shipp->ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER))) {
					dist = dist * 0.5f;
				}

				num_attacking = num_enemies_attacking(eno->trial_objp-Objects);
				if ((sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) || (num_attacking < eno->max_attackers)) {
					if (!(sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))){
						dist *= (float) (num_attacking+2)/2.0f;				//	prevents lots of ships from attacking same target
					}

					if (eno->trial_objp->flags & OF_PLAYER_SHIP){
						// Goober5000: oh dear, it looks like this was originally meant to scale
						// the distance according to skill, but as it is, the effect appears negligible.
						// I could fix it with parentheses, but we all know how quickly AI pilots
						// die usually, so the overall effect would probably be worse.  So I left
						// this unchanged.
						dist *= 1.0f + (NUM_SKILL_LEVELS - Game_skill_level - 1)/NUM_SKILL_LEVELS;	//	Favor attacking non-players based on skill level.
					}

					if (dist < eno->nearest_dist) {
						eno->nearest_dist = dist;
						eno->nearest_objnum = eno->trial_objp-Objects;
					}
				}
			}
		}
	}

}


/**
 * Given an object and an enemy team, return the index of the nearest enemy object.
 * Unless aip->targeted_subsys != NULL, don't allow to attack objects with OF_PROTECTED bit set.
 *
 * @param objnum			Object number
 * @param enemy_team_mask	Mask to apply to enemy team
 * @param enemy_wing		Enemy wing chosen
 * @param range				Ship must be within range "range".
 * @param max_attackers		Don't attack a ship that already has at least max_attackers attacking it.
 */
int get_nearest_objnum(int objnum, int enemy_team_mask, int enemy_wing, float range, int max_attackers)
{
	object	*danger_weapon_objp;
	ai_info	*aip;
	ship_obj	*so;

	// initialize eno struct
	eval_nearest_objnum eno;
	eno.enemy_team_mask = enemy_team_mask;
	eno.enemy_wing = enemy_wing;
	eno.max_attackers = max_attackers;
	eno.objnum = objnum;
	eno.range = range;
	eno.nearest_dist = range;
	eno.nearest_objnum = -1;
	eno.check_danger_weapon_objnum = 0;

	// go through the list of all ships and evaluate as potential targets
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		eno.trial_objp = &Objects[so->objnum];
		evaluate_object_as_nearest_objnum(&eno);
	}

	// check if danger_weapon_objnum has will show a stealth ship
	aip = &Ai_info[Ships[Objects[objnum].instance].ai_index];
	if (aip->danger_weapon_objnum >= 0) {
		danger_weapon_objp = &Objects[aip->danger_weapon_objnum];
		// validate weapon
		if (danger_weapon_objp->signature == aip->danger_weapon_signature) {
			Assert(danger_weapon_objp->type == OBJ_WEAPON);
			// check if parent is a ship
			if (danger_weapon_objp->parent >= 0) {
				if ( is_object_stealth_ship(&Objects[danger_weapon_objp->parent]) ) {
					// check if stealthy
					if ( ai_is_stealth_visible(&Objects[objnum], &Objects[danger_weapon_objp->parent]) != STEALTH_FULLY_TARGETABLE ) {
						// check if weapon is laser
						if (Weapon_info[Weapons[danger_weapon_objp->instance].weapon_info_index].subtype == WP_LASER) {
							// check stealth ship by its laser fire
							eno.check_danger_weapon_objnum = 1;
							eno.trial_objp = &Objects[danger_weapon_objp->parent];
							evaluate_object_as_nearest_objnum(&eno);
						}
					}
				}
			}
		}
	}

	//	If only looking for target in certain wing and couldn't find anything in
	//	that wing, look for any object.
	if ((eno.nearest_objnum == -1) && (enemy_wing != -1)) {
		return get_nearest_objnum(objnum, enemy_team_mask, -1, range, max_attackers);
	}

	return eno.nearest_objnum;
}

/**
 * Given an object and an enemy team, return the index of the nearest enemy object.
 *
 * Unlike find_enemy or find_nearest_objnum, this doesn't care about things like the protected flag or number of enemies attacking.
 * It is used to find the nearest enemy to determine things like whether to rearm.
 */
int find_nearby_threat(int objnum, int enemy_team_mask, float range, int *count)
{
	int		nearest_objnum;
	float		nearest_dist;
	object	*objp;
	ai_info	*aip;
	ship_obj	*so;

	nearest_objnum = -1;
	nearest_dist = range;

	aip = &Ai_info[Ships[Objects[objnum].instance].ai_index];

	*count = 0;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];

		if ( OBJ_INDEX(objp) != objnum ) {
			if (Ships[objp->instance].flags & SF_DYING)
				continue;

			if (Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_NO_SHIP_TYPE | SIF_NAVBUOY))
				continue;

			if (iff_matches_mask(Ships[objp->instance].team, enemy_team_mask)) {
				float	dist;

				dist = vm_vec_dist_quick(&Objects[objnum].pos, &objp->pos) - objp->radius*0.75f;
				
				if (dist < range) {
					(*count)++;

					if (dist < nearest_dist) {
						nearest_dist = dist;
						nearest_objnum = objp-Objects;
					}
				}
			}
		}
	}

	return nearest_objnum;
}

/**
 * Number of live turrets with target_objnum 
 */
int num_turrets_attacking(object *turret_parent, int target_objnum) 
{
	ship_subsys *ss;
	ship *shipp;
	int count = 0;
	shipp = &Ships[turret_parent->instance];

	Assert(turret_parent->type == OBJ_SHIP);

	for (ss=GET_FIRST(&shipp->subsys_list); ss!=END_OF_LIST(&shipp->subsys_list); ss=GET_NEXT(ss)) {
		// check if subsys is alive
		if (ss->current_hits <= 0.0f) {
			continue;
		}

		// check if it's a turret
		if (ss->system_info->type != SUBSYSTEM_TURRET) {
			continue;
		}

		// if the turret is locked
		if(ss->weapons.flags & SW_FLAG_TURRET_LOCK){
			continue;
		}		

		// check if turret is targeting target_objnum
		if (ss->turret_enemy_objnum == target_objnum) {
			count++;
		}
	}

	return count;
}

/**
 * Return timestamp until a ship can find an enemy.
 */
int get_enemy_timestamp()
{
	return (NUM_SKILL_LEVELS - Game_skill_level) * ( (myrand() % 500) + 500);
}

/**
 * Return objnum if enemy found, else return -1;
 *
 * @param objnum		Object number
 * @param range			Range within which to look
 * @param max_attackers Don't attack a ship that already has at least max_attackers attacking it.
 */
int find_enemy(int objnum, float range, int max_attackers)
{
	int	enemy_team_mask;

	if (objnum < 0)
		return -1;

	enemy_team_mask = iff_get_attackee_mask(obj_team(&Objects[objnum]));

	//	if target_objnum != -1, use that as goal.
	ai_info	*aip = &Ai_info[Ships[Objects[objnum].instance].ai_index];
	if (timestamp_elapsed(aip->choose_enemy_timestamp)) {
		aip->choose_enemy_timestamp = timestamp(get_enemy_timestamp());
		if (aip->target_objnum != -1) {
			int	target_objnum = aip->target_objnum;

			// DKA don't undo object as target in nebula missions.
			// This could cause attack on ship on fringe on nebula to stop if attackee moves our of nebula range.  (BAD)
			if ( (Objects[target_objnum].signature == aip->target_signature) ) {
				if (iff_matches_mask(Ships[Objects[target_objnum].instance].team, enemy_team_mask)) {
					if (!(Objects[target_objnum].flags & OF_PROTECTED)) {
						return target_objnum;
					}
				}
			} else {
				aip->target_objnum = -1;
				aip->target_signature = -1;
			}
		}
		
		return get_nearest_objnum(objnum, enemy_team_mask, aip->enemy_wing, range, max_attackers);
		
	} else {
		aip->target_objnum = -1;
		aip->target_signature = -1;
		return -1;
	}
}

/**
 * If issued an order to a ship that's awaiting repair, abort that process.
 * However, do not abort process for an object that is currently being repaired -- let it finish.
 */
void ai_set_goal_maybe_abort_dock(object *objp, ai_info *aip)
{
	if (aip->ai_flags & AIF_AWAITING_REPAIR) {
		object	*repair_obj;

		if (aip->support_ship_objnum == -1) {
			repair_obj = NULL;
		} else {
			repair_obj = &Objects[aip->support_ship_objnum];
		}
		ai_do_objects_repairing_stuff( objp, repair_obj, REPAIR_INFO_ABORT );
	}
	aip->next_rearm_request_timestamp = timestamp(NEXT_REARM_TIMESTAMP);	//	Might request again after 30 seconds.
}

void force_avoid_player_check(object *objp, ai_info *aip)
{
	if (Ships[objp->instance].team == Player_ship->team){
		aip->avoid_check_timestamp = timestamp(0);		//	Force a check for collision next frame.
	}
}

/**
 * Set *attacked as object to attack for object *attacker
 *
 * If attacked == NULL, then attack any enemy object.
 * Attack point *rel_pos on object.  This is for supporting attacking subsystems.
 */
void ai_attack_object(object *attacker, object *attacked, ship_subsys *ssp)
{
	int temp;
	ai_info	*aip;

	Assert(attacker != NULL);
	Assert(attacker->instance != -1);
	Assert(Ships[attacker->instance].ai_index != -1);

	aip = &Ai_info[Ships[attacker->instance].ai_index];
	force_avoid_player_check(attacker, aip);

	aip->ok_to_target_timestamp = timestamp(0);		//	Guarantee we can target.

	if (attacker == attacked) {
		Int3();		//	Bogus!  Who tried to get me to attack myself!  Trace out and fix!
		return;
	}

	//	Only set to chase if a fighter or bomber, otherwise just return.
	if (!(Ship_info[Ships[attacker->instance].ship_info_index].flags & SIF_SMALL_SHIP) && (attacked != NULL)) {
		nprintf(("AI", "AI ship %s is large ship ordered to attack %s\n", Ships[attacker->instance].ship_name, Ships[attacked->instance].ship_name));
	}

	//	This is how "engage enemy" gets processed
	if (attacked == NULL) {
		aip->choose_enemy_timestamp = timestamp(0);
		// nebula safe
		set_target_objnum(aip, find_enemy(attacker-Objects, 99999.9f, 4));
	} else {
		// check if we can see atacked in nebula
		if (aip->target_objnum != attacked - Objects) {
			aip->aspect_locked_time = 0.0f;
		}
		set_target_objnum(aip, attacked - Objects);
	}

	ai_set_goal_maybe_abort_dock(attacker, aip);
	aip->ok_to_target_timestamp = timestamp(DELAY_TARGET_TIME);	//	No dynamic targeting for 7 seconds.

	// Goober5000
	if ((temp = find_ignore_new_object_index(aip, aip->target_objnum)) >= 0)
	{
		aip->ignore_new_objnums[temp] = UNUSED_OBJNUM;
	}
	else if (is_ignore_object(aip, aip->target_objnum, 1))
	{
		aip->ignore_objnum = UNUSED_OBJNUM;
	}

	aip->mode = AIM_CHASE;
	aip->submode = SM_ATTACK;				// AL 12-15-97: need to set submode?  I got an assert() where submode was bogus
	aip->submode_start_time = Missiontime;	// for AIM_CHASE... it may have been not set correctly here

	if (ssp == NULL) {
		set_targeted_subsys(aip, NULL, -1);
		if (aip->target_objnum != -1) {
			Objects[aip->target_objnum].flags &= ~OF_PROTECTED;	//	If ship had been protected, unprotect it.
		}
	} else {
		Int3();	//	Not supported yet!
	}
}

/**
 * Set *attacked as object to attack for object *attacker
 * Attack point *rel_pos on object.  This is for supporting attacking subsystems.
 */
void ai_attack_wing(object *attacker, int wingnum)
{
	ai_info	*aip;

	Assert(attacker != NULL);
	Assert(attacker->instance != -1);
	Assert(Ships[attacker->instance].ai_index != -1);

	aip = &Ai_info[Ships[attacker->instance].ai_index];

	aip->enemy_wing = wingnum;
	aip->mode = AIM_CHASE;
	aip->submode = SM_ATTACK;				// AL 12-15-97: need to set submode?  I got an assert() where submode was bogus
	aip->submode_start_time = Missiontime;	// for AIM_CHASE... it may have been not set correctly here

	aip->ok_to_target_timestamp = timestamp(0);		//	Guarantee we can target.

	int count = Wings[wingnum].current_count;
	if (count > 0) {
		int	index;

		index = (int) (frand() * count);

		if (index >= count)
			index = 0;

		set_target_objnum(aip, Ships[Wings[wingnum].ship_index[index]].objnum);

		ai_set_goal_maybe_abort_dock(attacker, aip);
		aip->ok_to_target_timestamp = timestamp(DELAY_TARGET_TIME);	//	No dynamic targeting for 7 seconds.
	}
}

/**
 * Set *evaded as object for *evader to evade.
 */
void ai_evade_object(object *evader, object *evaded)
{
	ai_info	*aip;

	Assert(evader != NULL);
	Assert(evaded != NULL);
	Assert(evader->instance != -1);
	Assert(Ships[evader->instance].ai_index != -1);

	if (evaded == evader) {
		Int3();	//	Bogus!  Who tried to get me to evade myself!  Trace out and fix!
		return;
	}

	aip = &Ai_info[Ships[evader->instance].ai_index];

	set_target_objnum(aip, evaded - Objects);
	aip->mode = AIM_EVADE;

}

/**
 * Returns total number of ignored objects.
 *
 * @param aip		AI info
 * @param force		Means we forget the oldest object
 */
int compact_ignore_new_objects(ai_info *aip, int force = 0)
{
	if (force)
		aip->ignore_new_objnums[0] = UNUSED_OBJNUM;

	for (int current_index = 0; current_index < MAX_IGNORE_NEW_OBJECTS; current_index++)
	{
		int next_occupied_index = -1;

		// skip occupied slots
		if (aip->ignore_new_objnums[current_index] != UNUSED_OBJNUM)
		{
			// prune invalid objects
			if (Objects[aip->ignore_new_objnums[current_index]].signature != aip->ignore_new_signatures[current_index])
				aip->ignore_new_objnums[current_index] = UNUSED_OBJNUM;
			else
				continue;
		}

		// find an index to move downward
		for (int i = current_index + 1; i < MAX_IGNORE_NEW_OBJECTS; i++)
		{
			// skip empty slots
			if (aip->ignore_new_objnums[i] == UNUSED_OBJNUM)
				continue;

			// found one
			next_occupied_index = i;
			break;
		}

		// are all higher slots empty?
		if (next_occupied_index < 0)
			return current_index;

		// move the occupied slot down to this one
		aip->ignore_new_objnums[current_index] = aip->ignore_new_objnums[next_occupied_index];
		aip->ignore_new_signatures[current_index] = aip->ignore_new_signatures[next_occupied_index];

		// empty the occupied slot
		aip->ignore_new_objnums[next_occupied_index] = UNUSED_OBJNUM;
		aip->ignore_new_signatures[next_occupied_index] = -1;
	}

	// all slots are occupied
	return MAX_IGNORE_NEW_OBJECTS;
}

/**
 * Ignore some object without changing mode.
 */
void ai_ignore_object(object *ignorer, object *ignored, int ignore_new)
{
	ai_info	*aip;

	Assert(ignorer != NULL);
	Assert(ignored != NULL);
	Assert(ignorer->instance != -1);
	Assert(Ships[ignorer->instance].ai_index != -1);
	Assert(ignorer != ignored);

	aip = &Ai_info[Ships[ignorer->instance].ai_index];

	// Goober5000 - new behavior
	if (ignore_new)
	{
		int num_objects;

		// compact the array
		num_objects = compact_ignore_new_objects(aip);

		// make sure we're not adding a duplicate
		if (find_ignore_new_object_index(aip, OBJ_INDEX(ignored)) >= 0)
			return;

		// if we can't add a new one; "forget" one
		if (num_objects >= MAX_IGNORE_NEW_OBJECTS)
			num_objects = compact_ignore_new_objects(aip, 1);

		// add it
		aip->ignore_new_objnums[num_objects] = OBJ_INDEX(ignored);
		aip->ignore_new_signatures[num_objects] = ignored->signature;
	}
	// retail behavior
	else
	{
		aip->ignore_objnum = OBJ_INDEX(ignored);
		aip->ignore_signature = ignored->signature;
		aip->ai_flags &= ~AIF_TEMPORARY_IGNORE;
		ignored->flags |= OF_PROTECTED;					// set protected bit of ignored ship.
	}
}

/**
 * Ignore some object without changing mode.
 */
void ai_ignore_wing(object *ignorer, int wingnum, int priority)
{
	ai_info	*aip;

	Assert(ignorer != NULL);
	Assert(ignorer->instance != -1);
	Assert(Ships[ignorer->instance].ai_index != -1);
	Assert((wingnum >= 0) && (wingnum < MAX_WINGS));

	aip = &Ai_info[Ships[ignorer->instance].ai_index];

	aip->ignore_objnum = -(wingnum +1);
	aip->ai_flags &= ~AIF_TEMPORARY_IGNORE;
}


/**
 * Add a path point in the global buffer Path_points.
 *
 * If modify_index == -1, then create a new point.
 * If a new point is created (ie, modify_index == -1), then Ppfp is updated.
 *
 * @param pos			Position in vector space
 * @param path_num		Path numbers
 * @param path_index	Index into path
 * @param modify_index	Index in Path_points at which to store path point.
 */
void add_path_point(vec3d *pos, int path_num, int path_index, int modify_index)
{
	pnode	*pnp;

	if (modify_index == -1) {
		Assert(Ppfp-Path_points < MAX_PATH_POINTS-1);
		pnp = Ppfp;
		Ppfp++;
	} else {
		Assert((modify_index >= 0) && (modify_index < MAX_PATH_POINTS-1));
		pnp = &Path_points[modify_index];
	}

	pnp->pos = *pos;
	pnp->path_num = path_num;
	pnp->path_index = path_index;
}

/**
 * Given two points on a sphere, the center of the sphere and the radius, return a
 * point on the vector through the midpoint of the chord on the sphere.
 */
void bisect_chord(vec3d *p0, vec3d *p1, vec3d *centerp, float radius)
{
	vec3d	tvec;
	vec3d	new_pnt;

	vm_vec_add(&tvec, p0, p1);
	vm_vec_sub2(&tvec, centerp);
	vm_vec_sub2(&tvec, centerp);
	if (vm_vec_mag_quick(&tvec) < 0.1f) {
		vm_vec_sub(&tvec, p0, p1);
		if (fl_abs(tvec.xyz.x) <= fl_abs(tvec.xyz.z)){
			tvec.xyz.x = -tvec.xyz.z;
		} else {
			tvec.xyz.y = -tvec.xyz.x;
		}
	}

	vm_vec_normalize(&tvec);
	vm_vec_scale(&tvec, radius);
	vm_vec_add(&new_pnt, centerp, &tvec);

	add_path_point(&new_pnt, -1, -1, -1);
}
			
/**
 * Create a path from the current position to a goal position.
 *
 * The current position is in the current object and the goal position is in the goal object.
 * It is ok to intersect the current object, but not the goal object.
 * This function is useful for creating a path to an initial point near a large object.
 *
 * @param curpos		Current position in vector space
 * @param goalpos		Goal position in vector space
 * @param curobjp		Current object pointer
 * @param goalobjp		Goal object pointer
 * @param subsys_path	Optional param (default 0), indicates this is a path to a subsystem
 */
void create_path_to_point(vec3d *curpos, vec3d *goalpos, object *curobjp, object *goalobjp, int subsys_path)
{
	//	If can't cast vector to goalpos, then create an intermediate point.
	if (pp_collide(curpos, goalpos, goalobjp, curobjp->radius)) {
		vec3d	tan1;
		float		radius;

		// If this is a path to a subsystem, use SUBSYS_PATH_DIST as the radius for the object you are
		// trying to avoid.  This is needed since subsystem paths extend out to SUBSYS_PATH_DIST, and we
		// want ships to reach their path destination without flying to points that sit on the radius of
		// a small ship
		radius = goalobjp->radius;
		if (subsys_path) {
			if ( SUBSYS_PATH_DIST > goalobjp->radius ) {
				radius = SUBSYS_PATH_DIST;
			}
		}

		//	The intermediate point is at the intersection of:
		//		tangent to *goalobjp sphere at point *goalpos
		//		tangent to *goalobjp sphere through point *curpos in plane defined by *curpos, *goalpos, goalobjp->pos
		//	Note, there are two tangents through *curpos, unless *curpos is on the
		//	sphere.  The tangent that causes the nearer intersection (to *goalpos) is chosen.
		get_tangent_point(&tan1, curpos, &goalobjp->pos, goalpos, radius);

		//	If we can't reach tan1 from curpos, insert a new point.
		if (pp_collide(&tan1, curpos, goalobjp, curobjp->radius))
			bisect_chord(curpos, &tan1, &goalobjp->pos, radius);

		add_path_point(&tan1, -1, -1, -1);

		//	If we can't reach goalpos from tan1, insert a new point.
		if (pp_collide(goalpos, &tan1, goalobjp, curobjp->radius))
			bisect_chord(goalpos, &tan1, &goalobjp->pos, radius);
	}

}

/**
 * Given an object and a model path, globalize the points on the model and copy into the global path list.
 * If pnp != NULL, then modify, in place, the path points.  This is used to create new globalized points when the base object has moved.
 *
 * @param objp			Object pointer
 * @param mp			Model path
 * @param dir			Directory type
 * @param count			Count of items
 * @param path_num		Path number
 * @param pnp			Node on a path
 * @param randomize_pnt	Optional parameter (default value -1), add random vector in sphere to this path point
 */
void copy_xlate_model_path_points(object *objp, model_path *mp, int dir, int count, int path_num, pnode *pnp, int randomize_pnt)
{
	polymodel *pm;
	int		i, modelnum;
	vec3d	v1;
	int		pp_index;		//	index in Path_points at which to store point, if this is a modify-in-place (pnp ! NULL)
	int		start_index, finish_index;
	vec3d submodel_offset, local_vert;
	bool rotating_submodel;
	
	//	Initialize pp_index.
	//	If pnp == NULL, that means we're creating new points.  If not NULL, then modify in place.
	if (pnp == NULL)
		pp_index = -1;			//	This tells add_path_point to create a new point.
	else
		pp_index = 0;			//	pp_index will get assigned to index in Path_points to reuse.

	if (dir == 1) {
		start_index = 0;
		finish_index = MIN(count, mp->nverts);
	} else {
		Assert(dir == -1);	//	direction must be up by 1 or down by 1 and it's neither!
		start_index = mp->nverts-1;
		finish_index = MAX(-1, mp->nverts-1-count);
	}

	// Goober5000 - check for rotating submodels
	modelnum = Ship_info[Ships[objp->instance].ship_info_index].model_num;
	pm = model_get(modelnum);
	if ((mp->parent_submodel >= 0) && (pm->submodel[mp->parent_submodel].movement_type >= 0))
	{
		rotating_submodel = true;

		// start submodel calculation
		ship_model_start(objp);

		model_find_submodel_offset(&submodel_offset, modelnum, mp->parent_submodel);
	}
	else
	{
		rotating_submodel = false;
	}

	int offset = 0;
	for (i=start_index; i != finish_index; i += dir)
	{
		//	Globalize the point.
		// Goober5000 - handle rotating submodels
		if (rotating_submodel)
		{
			// movement... find location of point like with docking code and spark generation
			vm_vec_sub(&local_vert, &mp->verts[i].pos, &submodel_offset);
			model_find_world_point(&v1, &local_vert, modelnum, mp->parent_submodel, &objp->orient, &objp->pos);			
		}
		else
		{
			// no movement... calculate as in original code
			vm_vec_unrotate(&v1, &mp->verts[i].pos, &objp->orient);
			vm_vec_add2(&v1, &objp->pos);
		}

		if ( randomize_pnt == i ) {
			vec3d v_rand;
			static_randvec(OBJ_INDEX(objp), &v_rand);
			vm_vec_scale(&v_rand, 30.0f);
			vm_vec_add2(&v1, &v_rand);
		}

		if (pp_index != -1)
			pp_index = pnp-Path_points + offset;

		add_path_point(&v1, path_num, i, pp_index);
		offset++;
	}

	// stop submodel calculation
	if (!rotating_submodel)
	{
		ship_model_stop(objp);
	}
}


/**
 * For pl_objp, create a path along path path_num into mobjp.
 * The tricky part of this problem is creating the entry to the first point on the predefined path.  
 * The points on this entry path are based on the location of Pl_objp relative to the start of the path.
 *
 * @param pl_objp		Player object
 * @param mobjp			Model object
 * @param path_num		Number of path
 * @param subsys_path	Optional param (default 0), indicating this is a path to a subsystem
 */
void create_model_path(object *pl_objp, object *mobjp, int path_num, int subsys_path)
{	
	ship			*shipp = &Ships[pl_objp->instance];
	ai_info		*aip = &Ai_info[shipp->ai_index];

	ship_info	*osip = &Ship_info[Ships[mobjp->instance].ship_info_index];
	polymodel	*pm = model_get(Ship_info[Ships[mobjp->instance].ship_info_index].model_num);
	int			num_points;
	model_path	*mp;
	pnode			*ppfp_start = Ppfp;
	vec3d		gp0;

	Assert(path_num >= 0);

	//	Do garbage collection if necessary.
	if (Ppfp-Path_points + 64 > MAX_PATH_POINTS) {
		garbage_collect_path_points();
		ppfp_start = Ppfp;
	}

	aip->path_start = Ppfp - Path_points;
	Assert(path_num < pm->n_paths);
	
	mp = &pm->paths[path_num];
	num_points = mp->nverts;

	Assert(Ppfp-Path_points + num_points + 4 < MAX_PATH_POINTS);

	vm_vec_unrotate(&gp0, &mp->verts[0].pos, &mobjp->orient);
	vm_vec_add2(&gp0, &mobjp->pos);

	if (pp_collide(&pl_objp->pos, &gp0, mobjp, pl_objp->radius)) {
		vec3d	perim_point1;
		vec3d	perim_point2;

		perim_point2 = pl_objp->pos;
		
		//	If object that wants to dock is inside bounding sphere of object it wants to dock with, make it fly out.
		//	Assume it can fly "straight" out to the bounding sphere.
		if (vm_vec_dist_quick(&pl_objp->pos, &mobjp->pos) < mobjp->radius) {
			project_point_to_perimeter(&perim_point2, &mobjp->pos, mobjp->radius, &pl_objp->pos);
			add_path_point(&perim_point2, path_num, -1, -1);
		}

		//	If last point on pre-defined path is inside bounding sphere, create a new point on the surface of the sphere.
		if (vm_vec_dist_quick(&mobjp->pos, &gp0) < mobjp->radius) {
			project_point_to_perimeter(&perim_point1, &mobjp->pos, mobjp->radius, &gp0);
			create_path_to_point(&perim_point2, &perim_point1, pl_objp, mobjp, subsys_path);
			add_path_point(&perim_point1, path_num, -1, -1);
		} else {		//	The predefined path extends outside the sphere.  Create path to that point.
			create_path_to_point(&perim_point2, &gp0, pl_objp, mobjp, subsys_path);
		}
	}

	// AL 12-31-97: If following a subsystem path, add random vector to second last path point
	if ( subsys_path ) {
		copy_xlate_model_path_points(mobjp, mp, 1, mp->nverts, path_num, NULL, mp->nverts-2);
	} else {
		copy_xlate_model_path_points(mobjp, mp, 1, mp->nverts, path_num, NULL);
	}

	aip->path_cur = aip->path_start;
	aip->path_dir = PD_FORWARD;
	aip->path_objnum = mobjp-Objects;
	aip->mp_index = path_num;
	aip->path_length = Ppfp - ppfp_start;
	aip->path_next_check_time = timestamp(1);

	aip->path_goal_obj_hash = create_object_hash(&Objects[aip->path_objnum]);

	aip->path_next_create_time = timestamp(1000);	//	OK to try to create one second later
	aip->path_create_pos = pl_objp->pos;
	aip->path_create_orient = pl_objp->orient;
	
	//Get path departure orientation from ships.tbl if it exists, otherwise zero it
	SCP_string pathName(mp->name);
	if (osip->pathMetadata.find(pathName) != osip->pathMetadata.end() && !IS_VEC_NULL(&osip->pathMetadata[pathName].departure_rvec))
	{
		vm_vec_copy_normalize(&aip->path_depart_orient, &osip->pathMetadata[pathName].departure_rvec);
	}
	else
	{
		vm_vec_zero(&aip->path_depart_orient);
	}

	aip->ai_flags &= ~AIF_USE_EXIT_PATH;			// ensure this flag is cleared
}

/**
 * For pl_objp, create a path along path path_num into mobjp.
 * 
 * The tricky part of this problem is creating the entry to the first point on the predefined path.  
 * The points on this entry path are based on the location of pl_objp relative to the start of the path.
 */
void create_model_exit_path(object *pl_objp, object *mobjp, int path_num, int count)
{	
	ship			*shipp = &Ships[pl_objp->instance];
	ai_info		*aip = &Ai_info[shipp->ai_index];

	polymodel	*pm = model_get(Ship_info[Ships[mobjp->instance].ship_info_index].model_num);
	int			num_points;
	model_path	*mp;
	pnode			*ppfp_start = Ppfp;

	Assert(path_num >= 0);

	//	Do garbage collection if necessary.
	if (Ppfp-Path_points + 64 > MAX_PATH_POINTS) {
		garbage_collect_path_points();
		ppfp_start = Ppfp;
	}

	aip->path_start = Ppfp - Path_points;
	Assert(path_num < pm->n_paths);
	
	mp = &pm->paths[path_num];
	num_points = mp->nverts;

	Assert(Ppfp-Path_points + num_points + 4 < MAX_PATH_POINTS);

	copy_xlate_model_path_points(mobjp, mp, -1, count, path_num, NULL);

	aip->path_cur = aip->path_start;
	aip->path_dir = PD_FORWARD;
	aip->path_objnum = mobjp-Objects;
	aip->mp_index = path_num;
	aip->path_length = Ppfp - ppfp_start;
	aip->path_next_check_time = timestamp(1);

	aip->ai_flags |= AIF_USE_EXIT_PATH;		// mark as exit path, referenced in maybe
}

/**
 * Return true if the vector from curpos to goalpos intersects with any ship other than the ignore objects.
 * Calls pp_collide()
 */
int pp_collide_any(vec3d *curpos, vec3d *goalpos, float radius, object *ignore_objp1, object *ignore_objp2, int big_only_flag)
{
	ship_obj	*so;	

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		object *objp = &Objects[so->objnum];

		if (big_only_flag) {
			if (!(Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)))
				continue;
		}

		if ((objp != ignore_objp1) && (objp != ignore_objp2)) {
			if (pp_collide(curpos, goalpos, objp, radius))
				return OBJ_INDEX(objp);
		}
	}

	return -1;
}

//	Used to create docking paths and other pre-defined paths through ships.
//	Creates a path in absolute space.
//	Create a path into the object objnum.
//
// input:
//	pl_objp:			object that will use the path
//	objnum:			Object to find path to.
//	path_num:		model path index to use
//	exit_flag:		true means this is an exit path in the model
// subsys_path:	optional param (default 0) that indicates this is a path to a subsystem
//	Exit:
//	ai_info struct in Pl_objp gets stuffed with information to enable Pl_objp to fly the path.
void ai_find_path(object *pl_objp, int objnum, int path_num, int exit_flag, int subsys_path)
{
	ai_info	*aip = &Ai_info[Ships[pl_objp->instance].ai_index];

	Assert(path_num >= 0);

	//	This is test code, find an object with paths.
	if (objnum != -1) {
		object	*objp = &Objects[objnum];

		if (objp->type == OBJ_SHIP) {
			polymodel *pm;

			ship	*shipp = &Ships[objp->instance];
			pm = model_get(Ship_info[shipp->ship_info_index].model_num);
			if(pm->n_paths <= path_num)
				Error(LOCATION,"ai_find_path tring to find a path (%d) that doesn't exist, on ship %s", path_num, shipp->ship_name);

			aip->goal_objnum = objnum;
			aip->goal_signature = objp->signature;
			if (exit_flag)
				create_model_exit_path(pl_objp, objp, path_num);
			else
				create_model_path(pl_objp, objp, path_num, subsys_path);
			return;
		}

	}
}

extern int vector_object_collision(vec3d *start_pos, vec3d *end_pos, object *objp, float radius_scale);

//	Maybe make *objp avoid a player object.
//	For now, 4/6/98, only check Player_obj.
//	If player collision would occur, set AIF_AVOIDING_SMALL_SHIP bit in ai_flags.
//	Set aip->avoid_goal_point
int maybe_avoid_player(object *objp, vec3d *goal_pos)
{
	ai_info	*aip;
	vec3d	cur_pos, new_goal_pos;
	object	*player_objp;
	vec3d	n_vec_to_goal, n_vec_to_player;

	aip = &Ai_info[Ships[objp->instance].ai_index];

	if (!timestamp_elapsed(aip->avoid_check_timestamp))
		return 0;

	player_objp = Player_obj;

	float	speed_time;

	//	How far two ships could be apart and still collide within one second.
	speed_time = player_objp->phys_info.speed + objp->phys_info.speed;

	float	obj_obj_dist;

	obj_obj_dist = vm_vec_dist_quick(&player_objp->pos, &objp->pos);

	if (obj_obj_dist > speed_time*2.0f)
		return 0;

	cur_pos = objp->pos;

	new_goal_pos = *goal_pos;

	float dist = vm_vec_normalized_dir(&n_vec_to_goal, goal_pos, &objp->pos);
	vm_vec_normalized_dir(&n_vec_to_player, &player_objp->pos, &objp->pos);

	if (dist > speed_time*2.0f) {
		vm_vec_scale_add(&new_goal_pos, &objp->pos, &n_vec_to_goal, 200.0f);
	}

	if (vector_object_collision(&objp->pos, &new_goal_pos, player_objp, 1.5f)) {
		aip->ai_flags |= AIF_AVOIDING_SMALL_SHIP;

		vec3d	avoid_vec;

		vm_vec_sub(&avoid_vec, &n_vec_to_goal, &n_vec_to_player);
		if (vm_vec_mag_quick(&avoid_vec) < 0.01f) {
			vm_vec_copy_scale(&avoid_vec, &objp->orient.vec.rvec, frand()-0.5f);
			vm_vec_scale_add2(&avoid_vec, &objp->orient.vec.uvec, frand()-0.5f);
			vm_vec_normalize(&avoid_vec);
		} else {
			vec3d	tvec1;
			vm_vec_normalize(&avoid_vec);
			vm_vec_crossprod(&tvec1, &n_vec_to_goal, &avoid_vec);
			vm_vec_crossprod(&avoid_vec, &tvec1, &n_vec_to_player);
		}

		//	Now, avoid_vec is a vector perpendicular to the vector to the player and the direction *objp
		//	should fly in to avoid the player while still approaching its goal.
		vm_vec_scale_add(&aip->avoid_goal_point, &player_objp->pos, &avoid_vec, 400.0f);

		aip->avoid_check_timestamp = timestamp(1000);

		return 1;
	} else {
		aip->ai_flags &= ~AIF_AVOIDING_SMALL_SHIP;
		aip->avoid_check_timestamp = timestamp((int) (obj_obj_dist/200.0f) + 500);

		return 0;
	}
}

//	Make object *still_objp enter AIM_STILL mode.
//	Make it point at view_pos.
void ai_stay_still(object *still_objp, vec3d *view_pos)
{
	ship	*shipp;
	ai_info	*aip;

	Assert(still_objp->type == OBJ_SHIP);
	Assert((still_objp->instance >= 0) && (still_objp->instance < MAX_OBJECTS));

	shipp = &Ships[still_objp->instance];
	Assert((shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO));

	aip = &Ai_info[shipp->ai_index];

	aip->mode = AIM_STILL;

	//	If view_pos not NULL, point at that point.  Else, point at a point directly in front of ship.  Ie, don't turn.
	if (view_pos != NULL)
		aip->goal_point = *view_pos;
	else
		vm_vec_scale_add(&aip->goal_point, &still_objp->pos, &still_objp->orient.vec.fvec, 100.0f);
}

// code which is called from ai_dock_with_object and ai_dock to set flags and apprioriate variable
// when two objects have completed docking.  used because we can dock object initially at misison load
// time (meaning that ai_dock() might never get called).  docker has docked with dockee (i.e. docker
// would be a freighter and dockee would be a cargo).
void ai_do_objects_docked_stuff(object *docker, int docker_point, object *dockee, int dockee_point, bool update_clients)
{
	Assert((docker != NULL) && (dockee != NULL));

	// make sure they're not already docked!
	if (dock_check_find_direct_docked_object(docker, dockee))
	{
		Warning(LOCATION, "Call to ai_do_objects_docked_stuff when objects are already docked!  Trace out and fix!\n");
		return;
	}

	// link the two objects
	dock_dock_objects(docker, docker_point, dockee, dockee_point);

	if (docker->type == OBJ_SHIP && dockee->type == OBJ_SHIP)
	{
		// maybe set support ship info
		if ((Ship_info[Ships[docker->instance].ship_info_index].flags & SIF_SUPPORT)
			|| (Ship_info[Ships[dockee->instance].ship_info_index].flags & SIF_SUPPORT))
		{
			ai_info *docker_aip = &Ai_info[Ships[docker->instance].ai_index];
			ai_info *dockee_aip = &Ai_info[Ships[dockee->instance].ai_index];

#ifndef NDEBUG
			// support ship can only dock with one thing at a time
			if (Ship_info[Ships[docker->instance].ship_info_index].flags & SIF_SUPPORT)
				Assert(docker->dock_list->next == NULL);
			else
				Assert(dockee->dock_list->next == NULL);
#endif

			// set stuff for both objects
			docker_aip->support_ship_objnum = OBJ_INDEX(dockee);
			dockee_aip->support_ship_objnum = OBJ_INDEX(docker);
			docker_aip->support_ship_signature = dockee->signature;
			dockee_aip->support_ship_signature = docker->signature;
		}
	}

	// add multiplayer hook here to deal with docked objects.  
	if ( MULTIPLAYER_MASTER && update_clients)
		send_ai_info_update_packet( docker, AI_UPDATE_DOCK, dockee );
}

// code which is called when objects become undocked. Equivalent of above function.
// Goober5000 - dockee must always be non-NULL
void ai_do_objects_undocked_stuff( object *docker, object *dockee )
{
	Assert((docker != NULL) && (dockee != NULL));

	// make sure they're not already undocked!
	if (!dock_check_find_direct_docked_object(docker, dockee))
	{
		Warning(LOCATION, "Call to ai_do_objects_undocked_stuff when objects are already undocked!  Trace out and fix!\n");
		return;
	}

	// add multiplayer hook here to deal with undocked objects.  Do it before we
	// do anything else.  We don't need to send info for both objects, since multi
	// only supports one docked object
	if ( MULTIPLAYER_MASTER )
		send_ai_info_update_packet( docker, AI_UPDATE_UNDOCK, dockee );

	if (docker->type == OBJ_SHIP && dockee->type == OBJ_SHIP)
	{
		ai_info *docker_aip = &Ai_info[Ships[docker->instance].ai_index];
		ai_info *dockee_aip = &Ai_info[Ships[dockee->instance].ai_index];

		// clear stuff for both objects
		docker_aip->ai_flags &= ~AIF_BEING_REPAIRED;
		dockee_aip->ai_flags &= ~AIF_BEING_REPAIRED;
		docker_aip->support_ship_objnum = -1;
		dockee_aip->support_ship_objnum = -1;
		docker_aip->support_ship_signature = -1;
		dockee_aip->support_ship_signature = -1;
	}

	// unlink the two objects
	dock_undock_objects(docker, dockee);
}


//	--------------------------------------------------------------------------
//	Interface from goals code to AI.
//	Cause *docker to dock with *dockee.
//	priority is priority of goal from goals code.
//	dock_type is:
//		AIDO_DOCK		set goal of docking
//		AIDO_DOCK_NOW	immediately dock, used for ships that need to be docked at mission start
//		AIDO_UNDOCK		set goal of undocking
void ai_dock_with_object(object *docker, int docker_index, object *dockee, int dockee_index, int dock_type)
{
	Assert(docker != NULL);
	Assert(dockee != NULL);
	Assert(docker->instance != -1);
	Assert(dockee->instance != -1);
	Assert(Ships[docker->instance].ai_index != -1);
	Assert(Ships[dockee->instance].ai_index != -1);
	Assert(docker_index != -1);
	Assert(dockee_index != -1);

	ai_info *aip = &Ai_info[Ships[docker->instance].ai_index];

	aip->goal_objnum = OBJ_INDEX(dockee);
	aip->goal_signature = dockee->signature;

	aip->mode = AIM_DOCK;

	switch (dock_type) {
	case AIDO_DOCK:
		aip->submode = AIS_DOCK_0;
		aip->submode_start_time = Missiontime;
		break;
	case AIDO_DOCK_NOW:
		aip->submode = AIS_DOCK_4A;
		aip->submode_start_time = Missiontime;
		break;
	case AIDO_UNDOCK:
		aip->submode = AIS_UNDOCK_0;
		aip->submode_start_time = Missiontime;
		break;
	default:
		Int3();		//	Bogus dock_type.
	}

	// Goober5000 - we no longer need to set dock_path_index because it's easier to grab the path from the dockpoint
	// a debug check would be a good thing here, though
#ifndef NDEBUG
	if (dock_type == AIDO_UNDOCK)
	{
		polymodel	*pm = model_get(Ship_info[Ships[dockee->instance].ship_info_index].model_num);
		Assert( pm->docking_bays[dockee_index].num_spline_paths > 0 );
	}
#endif

	// dock instantly
	if (dock_type == AIDO_DOCK_NOW)
	{
		dock_orient_and_approach(docker, docker_index, dockee, dockee_index, DOA_DOCK_STAY);
		ai_do_objects_docked_stuff( docker, docker_index, dockee, dockee_index, false );
	}
	// pick a path to use to start docking
	else
	{
		int path_num = ai_return_path_num_from_dockbay(dockee, dockee_index);

		// make sure we have a path
		if (path_num < 0)
		{
			Error(LOCATION, "Cannot find a dock path for ship %s, dock index %d.  Aborting dock.\n", Ships[dockee->instance].ship_name, dockee_index);
			ai_mission_goal_complete(aip);
			return;
		}

		ai_find_path(docker, OBJ_INDEX(dockee), path_num, 0);
	}
}


/*
 * Cause a ship to fly toward a ship.
 */
void ai_start_fly_to_ship(object *objp, int shipnum)
{
	ai_info	*aip;

	aip = &Ai_info[Ships[objp->instance].ai_index];

	if (The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS && AutoPilotEngaged)
	{
		aip->ai_flags &= ~AIF_FORMATION_WING;
		aip->ai_flags &= ~AIF_FORMATION_OBJECT;
	}
	else
	{
		aip->ai_flags |= AIF_FORMATION_WING;
		aip->ai_flags &= ~AIF_FORMATION_OBJECT;
	}

	aip->mode = AIM_FLY_TO_SHIP;
	aip->submode_start_time = Missiontime;

	aip->target_objnum = Ships[shipnum].objnum;	

	Assert(aip->active_goal != AI_ACTIVE_GOAL_DYNAMIC);
}


//	Cause a ship to fly its waypoints.
//	flags tells:
//		WPF_REPEAT	Set -> repeat waypoints.
void ai_start_waypoints(object *objp, waypoint_list *wp_list, int wp_flags)
{
	ai_info	*aip;
	Assert(wp_list != NULL);

	aip = &Ai_info[Ships[objp->instance].ai_index];

	if ( (aip->mode == AIM_WAYPOINTS) && (aip->wp_list == wp_list) )
	{
		if (aip->wp_index == INVALID_WAYPOINT_POSITION)
		{
			Warning(LOCATION, "aip->wp_index should have been assigned already!");
			aip->wp_index = aip->wp_list->get_waypoints().begin();
		}
		return;
	}

	if (The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS && AutoPilotEngaged)
	{
		aip->ai_flags &= ~AIF_FORMATION_WING;
		aip->ai_flags &= ~AIF_FORMATION_OBJECT;
	}
	else
	{
		aip->ai_flags |= AIF_FORMATION_WING;
		aip->ai_flags &= ~AIF_FORMATION_OBJECT;
	}

	aip->wp_list = wp_list;
	aip->wp_index = wp_list->get_waypoints().begin();
	aip->wp_flags = wp_flags;
	aip->mode = AIM_WAYPOINTS;

	Assert(aip->active_goal != AI_ACTIVE_GOAL_DYNAMIC);
}

/**
 * Make *objp stay within dist units of *other_objp
 */
void ai_do_stay_near(object *objp, object *other_objp, float dist)
{
	ai_info	*aip;

	Assert(objp != other_objp);		//	Bogus!  Told to stay near self.
	Assert(objp->type == OBJ_SHIP);
	Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

	aip = &Ai_info[Ships[objp->instance].ai_index];

	aip->mode = AIM_STAY_NEAR;
	aip->submode = -1;
	aip->submode_start_time = Missiontime;
	aip->stay_near_distance = dist;
	aip->goal_objnum = other_objp-Objects;
	aip->goal_signature = other_objp->signature;
}

//	Goober5000 - enter safety mode (used by support ships, now possibly by fighters too)
void ai_do_safety(object *objp)
{
	ai_info	*aip;

	Assert(objp->type == OBJ_SHIP);
	Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

	aip = &Ai_info[Ships[objp->instance].ai_index];

	aip->mode = AIM_SAFETY;
	aip->submode = AISS_1;
	aip->submode_start_time = Missiontime;
}

/**
 * Make object *objp form on wing of object *goal_objp
 */
void ai_form_on_wing(object *objp, object *goal_objp)
{
	ai_info	*aip;
	ship			*shipp;
	ship_info	*sip;

	// objp == goal_objp sometimes in multiplayer when someone leaves a game -- make a simple
	// out for this case.
	if ( Game_mode & GM_MULTIPLAYER ) {
		if ( objp == goal_objp ) {
			return;
		}
	}

	Assert(objp != goal_objp);		//	Bogus!  Told to form on own's wing!

	shipp = &Ships[objp->instance];
	sip = &Ship_info[shipp->ship_info_index];

	//	Only fighters or bombers allowed to form on wing.
	if (!(sip->flags & (SIF_FIGHTER | SIF_BOMBER))) {
		nprintf(("AI", "Warning: Ship %s tried to form on player's wing, but not fighter or bomber.\n", shipp->ship_name));
		return;
	}


	aip = &Ai_info[Ships[objp->instance].ai_index];

	aip->ai_flags &= ~AIF_FORMATION_WING;
	aip->ai_flags |= AIF_FORMATION_OBJECT;

	aip->goal_objnum = goal_objp-Objects;
	ai_set_goal_maybe_abort_dock(objp, aip);
	aip->ok_to_target_timestamp = timestamp(DELAY_TARGET_TIME*4);		//	Super extra long time until can target another ship.

}

/**
 * Given an object and an object on whose wing to form, return slot to use.
 *
 * @todo This function is called per object in formation per frame.  Should store slot in ::ai_info struct.
 */
int ai_formation_object_get_slotnum(int objnum, object *objp)
{
	int	slotnum = 1;			//	Note: Slot #0 means leader, which isn't someone who was told to form-on-wing.

	object *o;
	for ( o = GET_FIRST(&obj_used_list); o != END_OF_LIST(&obj_used_list); o = GET_NEXT(o) ) {
		if (objp == o)
			break;
		else if (o->type == OBJ_SHIP)
			if (Ai_info[Ships[o->instance].ai_index].ai_flags & AIF_FORMATION_OBJECT)
				if (Ai_info[Ships[o->instance].ai_index].goal_objnum == objnum)
					slotnum++;
	}

	Assert(o != END_OF_LIST(&obj_used_list));	//	Didn't find objp in list of used ships.  Impossible!

	return slotnum;
}

#define	BIGNUM	100000.0f

int Debug_k = 0;

/**
 * Given an attacker's position and a target's position and velocity, compute the time of
 * intersection of a weapon fired by the attacker with speed weapon_speed.
 *
 * Return this value.  Return value of 0.0f means no collision is possible.
 */
float compute_collision_time(vec3d *targpos, vec3d *targvel, vec3d *attackpos, float weapon_speed)
{
	vec3d	vec_to_target;
	float		pos_dot_vel;
	float		vel_sqr;
	float		discrim;

	vm_vec_sub(&vec_to_target, targpos, attackpos);
	pos_dot_vel = vm_vec_dot(&vec_to_target, targvel);
	vel_sqr = vm_vec_dot(targvel, targvel) - weapon_speed*weapon_speed;
	discrim = pos_dot_vel*pos_dot_vel - vel_sqr*vm_vec_dot(&vec_to_target, &vec_to_target);

	if (discrim > 0.0f) {
		float	t1, t2, t_solve;

		t1 = (-pos_dot_vel + fl_sqrt(discrim)) / vel_sqr;
		t2 = (-pos_dot_vel - fl_sqrt(discrim)) / vel_sqr;

		t_solve = BIGNUM;

		if (t1 > 0.0f)
			t_solve = t1;
		if ((t2 > 0.0f) && (t2 < t_solve))
			t_solve = t2;

		if (t_solve < BIGNUM-1.0f) {
			return t_solve + Debug_k * flFrametime;
		}
	}

	return 0.0f;
}


//	--------------------------------------------------------------------------
//	If far away, use player's speed.
//	If in between, lerp between player and laser speed
//	If close, use laser speed.
// Want to know how much time it will take to get to the enemy.
// This function doesn't account for the fact that by the time the player
// (or his laser) gets to the current enemy position, the enemy will have moved.
// This is dealt with in polish_predicted_enemy_pos.
float compute_time_to_enemy(float dist_to_enemy, object *pobjp, object *eobjp)
{
	float	time_to_enemy;
	float	pl_speed = pobjp->phys_info.speed;
	float	max_laser_distance, max_laser_speed;
	int	bank_num, weapon_num;
	ship	*shipp = &Ships[pobjp->instance];

	bank_num = shipp->weapons.current_primary_bank;
	weapon_num = shipp->weapons.primary_bank_weapons[bank_num];
	max_laser_speed = Weapon_info[weapon_num].max_speed;
	max_laser_distance = MIN((max_laser_speed * Weapon_info[weapon_num].lifetime), Weapon_info[weapon_num].weapon_range);

	//	If pretty far away, use player's speed to predict position, else
	//	use laser's speed because when close, we care more about hitting
	//	with a laser than about causing ship:ship rendezvous.
	if (dist_to_enemy > 1.5 * max_laser_distance) {
		if (pl_speed > 0.0f)
			time_to_enemy = dist_to_enemy/pl_speed;
		else
			time_to_enemy = 1.0f;
	} else if (dist_to_enemy > 1.1*max_laser_distance) {
		if (pl_speed > 0.1f) {
			float	scale;

			scale = (float) ((dist_to_enemy - max_laser_distance) / max_laser_distance);
		
			time_to_enemy = (float) (dist_to_enemy/(pl_speed * scale + max_laser_speed * (1.0f - scale)));
		} else
			time_to_enemy = 2.0f;
	} else
		time_to_enemy = (float) (dist_to_enemy/max_laser_speed);

	return time_to_enemy + flFrametime;
}

//	Stuff *dot and *tts.
//	*dot is always computed.  If dot is less than zero, the magnitude is
//	incorrect, not having been divided by distance.
//	If *dot is > 0.0f, then tts is computed.  This is the time it will take object
//	*objp to get to *pos, assuming it moves right at it.
void fds_aux(float *dot, float *tts, vec3d *pos, float dtime, object *objp)
{
	vec3d	v2s;

	vm_vec_sub(&v2s, pos, &objp->pos);
	*dot = vm_vec_dot(&v2s, &objp->orient.vec.fvec);

	if (*dot > 0.0f) {
		float	dist;

		dist = vm_vec_dist(&objp->pos, pos);

		if (dist > 0.1f)
			*dot /= dist;
		else
			*dot = 1.0f;

		if (objp->phys_info.speed > 0.1f)
			*tts = dist / objp->phys_info.speed;
		else
			*tts = dist * 100.0f;
	}
}

//	--------------------------------------------------------------------------
void ai_set_positions(object *pl_objp, object *en_objp, ai_info *aip, vec3d *player_pos, vec3d *enemy_pos)
{
	*player_pos = pl_objp->pos;

	if (aip->next_predict_pos_time > Missiontime) {
		*enemy_pos = aip->last_predicted_enemy_pos;
	} else {
		*enemy_pos = en_objp->pos;

		aip->next_predict_pos_time = Missiontime + aip->ai_predict_position_delay;
		aip->last_predicted_enemy_pos = *enemy_pos;
	}


}

//	--------------------------------------------------------------------------
void ai_update_aim(ai_info *aip, object* En_Objp)
{
	if (Missiontime >= aip->next_aim_pos_time)
	{
		aip->last_aim_enemy_pos = En_objp->pos;
		aip->last_aim_enemy_vel = En_objp->phys_info.vel;
		aip->next_aim_pos_time = Missiontime + fl2f(frand_range(0.0f, aip->ai_max_aim_update_delay));
	}
	else
	{
		//Update the position based on the velocity (assume no velocity vector change)
		vm_vec_scale_add2(&aip->last_aim_enemy_pos, &aip->last_aim_enemy_vel, flFrametime);
	}
}

//	Given an ai_info struct, by reading current goal and path information,
//	extract base path information and return in pmp and pmpv.
//	Return true if found, else return false.
//	false means the current point is not on the original path.
int get_base_path_info(int path_cur, int goal_objnum, model_path **pmp, mp_vert **pmpv)
{
	pnode			*pn = &Path_points[path_cur];
	ship *shipp = &Ships[Objects[goal_objnum].instance];
	polymodel	*pm = model_get(Ship_info[shipp->ship_info_index].model_num);
	
	*pmpv = NULL;
	*pmp = NULL;

	if (pn->path_num != -1) {
		*pmp = &pm->paths[pn->path_num];
		if (pn->path_index != -1)
			*pmpv = &(*pmp)->verts[pn->path_index];
		else
			return 0;
	} else
		return 0;

	return 1;
}

//	Modify, in place, the points in a global model path.
//	Only modify those points that are defined in the model path.  Don't modify the
//	leadin points, such as those that are necessary to get the model on the path.
void modify_model_path_points(object *objp)
{	
	ai_info		*aip = &Ai_info[Ships[objp->instance].ai_index];
	object		*mobjp = &Objects[aip->path_objnum];
	polymodel	*pm = model_get(Ship_info[Ships[mobjp->instance].ship_info_index].model_num);
	pnode			*pnp;
	int			path_num, dir;

	Assert((aip->path_start >= 0) && (aip->path_start < MAX_PATH_POINTS));

	pnp = &Path_points[aip->path_start];
	while ((pnp->path_index == -1) && (pnp-Path_points - aip->path_start < aip->path_length))
		pnp++;

	path_num = pnp->path_num;
	Assert((path_num >= 0) && (path_num < pm->n_paths));
	
	Assert(pnp->path_index != -1);	//	If this is -1, that means we never found the model path points

	dir = 1;
	if ( aip->ai_flags & AIF_USE_EXIT_PATH ) {
		dir = -1;
	}

	copy_xlate_model_path_points(mobjp, &pm->paths[path_num], dir, pm->paths[path_num].nverts, path_num, pnp);
}

//	Return an indication of the distance between two matrices.
//	This is the sum of the distances of their dot products from 1.0f.
float ai_matrix_dist(matrix *mat1, matrix *mat2)
{
	float	t;

	t =  1.0f - vm_vec_dot(&mat1->vec.fvec, &mat2->vec.fvec);
	t += 1.0f - vm_vec_dot(&mat1->vec.uvec, &mat2->vec.uvec);
	t += 1.0f - vm_vec_dot(&mat1->vec.rvec, &mat2->vec.rvec);

	return t;
}


//	Paths are created in absolute space, so a moving object needs to have model paths within it recreated.
//	This uses the hash functions which means the slightest movement will cause a recreate, though the timestamp
//	prevents this from happening too often.
//	force_recreate_flag TRUE means to recreate regardless of timestamp.
//	Returns TRUE if path recreated.
float maybe_recreate_path(object *objp, ai_info *aip, int force_recreate_flag, int override_hash = 0)
{
	int	hashval;

	Assert(&Ai_info[Ships[objp->instance].ai_index] == aip);

	if ((aip->mode == AIM_BAY_EMERGE) || (aip->mode == AIM_BAY_DEPART))
		if ((OBJ_INDEX(objp) % 4) == (Framecount % 4))
			force_recreate_flag = 1;

	//	If no path, that means we don't need one.
	if (aip->path_start == -1)
		return 0.0f;

	// AL 11-12-97: If AIF_USE_STATIC_PATH is set, don't try to recreate.  This is needed when ships
	//				    emerge from fighter bays.  We don't need to recreate the path.. and in case the 
	//              parent ship dies, we still want to be able to continue on the path
	if ( aip->ai_flags & AIF_USE_STATIC_PATH ) 
		return 0.0f;

	if (force_recreate_flag || timestamp_elapsed(aip->path_next_create_time)) {
		object	*path_objp;

		path_objp = &Objects[aip->path_objnum];
		hashval = create_object_hash(path_objp);

		if (override_hash || (hashval != aip->path_goal_obj_hash)) {
			float dist;
			
			dist = vm_vec_dist_quick(&path_objp->pos, &aip->path_create_pos);
			dist += ai_matrix_dist(&path_objp->orient, &aip->path_create_orient) * 25.0f;

			if (force_recreate_flag || (dist > 2.0f)) {
				aip->path_next_create_time = timestamp(1000);	//	Update again in as little as 1000 milliseconds, ie 1 second.
				aip->path_goal_obj_hash = hashval;
				modify_model_path_points(objp);

				aip->path_create_pos = path_objp->pos;
				aip->path_create_orient = path_objp->orient;
		
				//Get path departure orientation from ships.tbl if it exists, otherwise zero it
				ship_info *osip = &Ship_info[Ships[path_objp->instance].ship_info_index];
				model_path	*mp = &model_get(osip->model_num)->paths[aip->mp_index];
				SCP_string pathName(mp->name);
				if (osip->pathMetadata.find(pathName) != osip->pathMetadata.end() && !IS_VEC_NULL(&osip->pathMetadata[pathName].departure_rvec))
				{
					vm_vec_copy_normalize(&aip->path_depart_orient, &osip->pathMetadata[pathName].departure_rvec);
				}
				else
				{
					vm_vec_zero(&aip->path_depart_orient);
				}

				return dist;
			}
		}
	}

	return 0.0f;
}

/**
 * Set acceleration for ai_dock().
 */
void set_accel_for_docking(object *objp, ai_info *aip, float dot, float dot_to_next, float dist_to_next, float dist_to_goal, ship_info *sip, float max_allowed_speed)
{
	float prev_dot_to_goal = aip->prev_dot_to_goal;
	
	aip->prev_dot_to_goal = dot;

	if (objp->phys_info.speed < 0.0f) {
		accelerate_ship(aip, 1.0f/32.0f);
	} else if ((prev_dot_to_goal-dot) > 0.01) {
		if (prev_dot_to_goal > dot + 0.05f) {
			accelerate_ship(aip, 0.0f);
		} else {
			change_acceleration(aip, -1.0f);	//	-1.0f means subtract off flFrametime from acceleration value in 0.0..1.0
		}
	} else {
		// If max_allowed_speed isn't set, use the ship max speed
		if (max_allowed_speed <= 0)
			max_allowed_speed = sip->max_speed;

		if ((aip->mode == AIM_DOCK) && (dist_to_next < 150.0f) && (aip->path_start + aip->path_length - 2 == aip->path_cur)) {
			set_accel_for_target_speed(objp, max_allowed_speed * MAX(dist_to_next/500.0f, 1.0f));
		} else if ((dot_to_next >= dot * .9) || (dist_to_next > 100.0f)) {
			if (dist_to_goal > 200.0f) {
				set_accel_for_target_speed(objp, max_allowed_speed * (dot + 1.0f) / 2.0f);
			}
			else {
				float	xdot;

				xdot = (dot_to_next + dot)/2.0f;
				if (xdot < 0.0f)
					xdot = 0.0f;

				// AL: if following a path not in dock mode, move full speed
				if (( aip->mode != AIM_DOCK ) && (dot > 0.9f)) {
					set_accel_for_target_speed(objp, max_allowed_speed*dot*dot*dot);
				} else {
					if ((aip->path_cur - aip->path_start < aip->path_length-2) && (dist_to_goal < 2*objp->radius)) {
						set_accel_for_target_speed(objp, dist_to_goal/8.0f + 2.0f);
					} else {
						set_accel_for_target_speed(objp, max_allowed_speed * (2*xdot + 0.25f)/4.0f);
					}
				}
			}
		} else {
			float	xdot;

			xdot = MAX(dot_to_next, 0.1f);
			if ( aip->mode != AIM_DOCK ) {
				set_accel_for_target_speed(objp, max_allowed_speed);
			} else {
				float	speed;
				if ((aip->path_cur - aip->path_start < aip->path_length-2) && (dist_to_goal < 2*objp->radius)) {
					speed = dist_to_goal/8.0f + 2.0f;
				} else if (dist_to_goal < 4*objp->radius + 50.0f) {
					speed = dist_to_goal/4.0f + 4.0f;
				} else {
					speed = max_allowed_speed * (3*xdot + 1.0f)/4.0f;
				}
				if (aip->mode == AIM_DOCK) {
					speed = speed * 2.0f + 1.0f;
					if (aip->goal_objnum != -1) {
						speed += Objects[aip->goal_objnum].phys_info.speed;
					}
				}

				set_accel_for_target_speed(objp, speed);
			}
		}
	}
}

//	--------------------------------------------------------------------------
//	Follow a path associated with a large object, such as a capital ship.
//	The points defined on the path are in the object's reference frame.
//	The object of interest is goal_objnum.
//	The paths are defined in the model.  The path of interest is wp_list.
//	The next goal point in the path is wp_index.
//	wp_flags contain special information specific to the path.

// The path vertices are defined by model_path structs:
//		typedef struct model_path {
//			char		name[MAX_NAME_LEN];					// name of the subsystem.  Probably displayed on HUD
//			int		nverts;
//			vec3d	*verts;
//		} model_path;

//	The polymodel struct for the object contains the following:
//		int			n_paths;
//		model_path	*paths;

//	Returns distance to goal point.
float ai_path_0()
{
	polymodel	*pm;
	int		num_paths, num_points;
	float		dot, dist_to_goal, dist_to_next, speed, dot_to_next;
	ship		*shipp = &Ships[Pl_objp->instance];
	ship_info	*sip = &Ship_info[shipp->ship_info_index];
	ai_info	*aip;
	vec3d	nvel_vec;
	float		mag, prev_dot_to_goal;
	vec3d	temp_vec, *slop_vec;
	object	*gobjp;
	ship		*gshipp;
	vec3d	*cvp, *nvp, next_vec, gcvp, gnvp;		//	current and next vertices in global coordinates.

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	Assert(aip->goal_objnum != -1);
	Assert(Objects[aip->goal_objnum].type == OBJ_SHIP);

	gobjp = &Objects[aip->goal_objnum];
	gshipp = &Ships[gobjp->instance];

	pm = model_get(Ship_info[gshipp->ship_info_index].model_num);
	num_paths = pm->n_paths;
	Assert(num_paths > 0);

	if (aip->path_start == -1) {
		Assert(aip->goal_objnum >= 0 && aip->goal_objnum < MAX_OBJECTS);
		int path_num;
		Assert(aip->active_goal >= 0);
		ai_goal *aigp = &aip->goals[aip->active_goal];
		Assert(aigp->flags & AIGF_DOCK_INDEXES_VALID);

		path_num = ai_return_path_num_from_dockbay(&Objects[aip->goal_objnum], aigp->dockee.index);
		ai_find_path(Pl_objp, aip->goal_objnum, path_num, 0);
	}

	maybe_recreate_path(Pl_objp, aip, 0);

	num_points = aip->path_length;

	//	Set cvp and nvp as pointers to current and next vertices of interest on path.
	cvp = &Path_points[aip->path_cur].pos;
	if ((aip->path_cur + aip->path_dir - aip->path_start < num_points) || (aip->path_cur + aip->path_dir < aip->path_start))
		nvp = &Path_points[aip->path_cur + aip->path_dir].pos;
	else {
		//	If this is 0, then path length must be 1 which means we have no direction!
		Assert((aip->path_cur - aip->path_dir >= aip->path_start) && (aip->path_cur - aip->path_dir - aip->path_start < num_points));
		//	Cleanup for above Assert() which we hit too near release. -- MK, 5/24/98.
		if (aip->path_cur - aip->path_dir - aip->path_start >= num_points) {
			if (aip->path_dir == 1)
				aip->path_cur = aip->path_start;
			else
				aip->path_cur = aip->path_start + num_points - 1;
		}

		vec3d	delvec;
		vm_vec_sub(&delvec, cvp, &Path_points[aip->path_cur - aip->path_dir].pos);
		vm_vec_normalize(&delvec);
		vm_vec_scale_add(&next_vec, cvp, &delvec, 10.0f);
		nvp = &next_vec;
	}

	//	See if can reach next point (as opposed to current point)
	//	However, don't do this if docking and next point is last point.
	//	That is, we don't want to pursue the last point under control of the
	//	path code.  In docking, this is a special hack.
	if ((aip->mode != AIM_DOCK) || ((aip->path_cur-aip->path_start) < num_points - 2)) {
		if ((aip->path_cur + aip->path_dir > aip->path_start) && (aip->path_cur + aip->path_dir < aip->path_start + num_points-2)) {
			if ( timestamp_elapsed(aip->path_next_check_time)) {
				aip->path_next_check_time = timestamp( 3000 );
				if (!pp_collide(&Pl_objp->pos, nvp, gobjp, 1.1f * Pl_objp->radius)) {
					cvp = nvp;
					aip->path_cur += aip->path_dir;
					nvp = &Path_points[aip->path_cur].pos;
				}
			}
		}
	}

	gcvp = *cvp;
	gnvp = *nvp;

	speed = Pl_objp->phys_info.speed;

	dist_to_goal = vm_vec_dist_quick(&Pl_objp->pos, &gcvp);
	dist_to_next = vm_vec_dist_quick(&Pl_objp->pos, &gnvp);
	//	Can't use fvec, need to use velocity vector because we aren't necessarily
	//	moving in the direction we're facing.

	if ( vm_vec_mag_quick(&Pl_objp->phys_info.vel) < AICODE_SMALL_MAGNITUDE ) {
		mag = 0.0f;
		vm_vec_zero(&nvel_vec);
	} else
		mag = vm_vec_copy_normalize(&nvel_vec, &Pl_objp->phys_info.vel);

	//	If moving not-very-slowly and sliding, then try to slide at goal, rather than
	//	point at goal.
	slop_vec = NULL;
	if (mag < 1.0f)
		nvel_vec = Pl_objp->orient.vec.fvec;
	else if (mag > 5.0f) {
		float	nv_dot;
		nv_dot = vm_vec_dot(&Pl_objp->orient.vec.fvec, &nvel_vec);
		if ((nv_dot > 0.5f) && (nv_dot < 0.97f)) {
			slop_vec = &temp_vec;
			vm_vec_sub(slop_vec, &nvel_vec, &Pl_objp->orient.vec.fvec);
		}
	}

	if (dist_to_goal > 0.1f)
		ai_turn_towards_vector(&gcvp, Pl_objp, flFrametime, sip->srotation_time, slop_vec, NULL, 0.0f, 0);

	//	Code to control speed is MUCH less forgiving in path following than in waypoint
	//	following.  Must be very close to path or might hit objects.
	prev_dot_to_goal = aip->prev_dot_to_goal;
	dot = vm_vec_dot_to_point(&nvel_vec, &Pl_objp->pos, &gcvp);
	dot_to_next = vm_vec_dot_to_point(&nvel_vec, &Pl_objp->pos, &gnvp);

	set_accel_for_docking(Pl_objp, aip, dot, dot_to_next, dist_to_next, dist_to_goal, sip, 0);
	aip->prev_dot_to_goal = dot;

	//	If moving at a non-tiny velocity, detect attaining path point by its being close to
	//	line between previous and current object location.
	if ((dist_to_goal < MIN_DIST_TO_WAYPOINT_GOAL) || (vm_vec_dist_quick(&Pl_objp->last_pos, &Pl_objp->pos) > 0.1f)) {
		vec3d	nearest_point;
		float		r, min_dist_to_goal;

		r = find_nearest_point_on_line(&nearest_point, &Pl_objp->last_pos, &Pl_objp->pos, &gcvp);

		//	Set min_dist_to_goal = how close must be to waypoint to pick next one.
		//	If docking and this is the second last waypoint, must be very close.
		if ((aip->mode == AIM_DOCK) && (aip->path_cur >= aip->path_length-2))
			min_dist_to_goal = MIN_DIST_TO_WAYPOINT_GOAL;
		else
			min_dist_to_goal = MIN_DIST_TO_WAYPOINT_GOAL + Pl_objp->radius;

		if ( (vm_vec_dist_quick(&Pl_objp->pos, &gcvp) < min_dist_to_goal) ||
			((r >= 0.0f) && (r <= 1.0f)) && (vm_vec_dist_quick(&nearest_point, &gcvp) < (MIN_DIST_TO_WAYPOINT_GOAL + Pl_objp->radius))) {
			aip->path_cur += aip->path_dir;
			if (((aip->path_cur - aip->path_start) > (num_points+1)) || (aip->path_cur < aip->path_start)) {
				Assert(aip->mode != AIM_DOCK);		//	If docking, should never get this far, getting to last point handled outside ai_path()
				aip->path_dir = -aip->path_dir;
			}
		}
	}

	return dist_to_goal;
}

//	--------------------------------------------------------------------------
//	Alternate version of ai_path
//  1. 
float ai_path_1()
{
	polymodel	*pm;
	int		num_paths, num_points;
	float		dot, dist_to_goal, dist_to_next, speed, dot_to_next;
	ship		*shipp = &Ships[Pl_objp->instance];
	ship_info	*sip = &Ship_info[shipp->ship_info_index];
	ai_info	*aip;
	vec3d	nvel_vec;
	float		mag, prev_dot_to_goal;
	object	*gobjp;
	ship		*gshipp;
	vec3d	*pvp, *cvp, *nvp, next_vec, gpvp, gcvp, gnvp;		//	previous, current and next vertices in global coordinates.

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	Assert(aip->goal_objnum != -1);
	Assert(Objects[aip->goal_objnum].type == OBJ_SHIP);

	gobjp = &Objects[aip->goal_objnum];
	gshipp = &Ships[gobjp->instance];

	// Get path data
	pm = model_get(Ship_info[gshipp->ship_info_index].model_num);
	num_paths = pm->n_paths;
	Assert(num_paths > 0);

	if (aip->path_start == -1) {
		Assert(aip->goal_objnum >= 0 && aip->goal_objnum < MAX_OBJECTS);
		int path_num;
		Assert(aip->active_goal >= 0);
		ai_goal *aigp = &aip->goals[aip->active_goal];
		Assert(aigp->flags & AIGF_DOCK_INDEXES_VALID);

		path_num = ai_return_path_num_from_dockbay(&Objects[aip->goal_objnum], aigp->dockee.index);
		ai_find_path(Pl_objp, aip->goal_objnum, path_num, 0);
	}

	maybe_recreate_path(Pl_objp, aip, 0);

	num_points = aip->path_length;

	//	Set cvp and nvp as pointers to current and next vertices of interest on path.
	cvp = &Path_points[aip->path_cur].pos;
	if ((aip->path_cur + aip->path_dir - aip->path_start < num_points) || (aip->path_cur + aip->path_dir < aip->path_start))
		nvp = &Path_points[aip->path_cur + aip->path_dir].pos;
	else {
		//	If this is 0, then path length must be 1 which means we have no direction!
		Assert((aip->path_cur - aip->path_dir >= aip->path_start) && (aip->path_cur - aip->path_dir - aip->path_start < num_points));
		//	Cleanup for above Assert() which we hit too near release. -- MK, 5/24/98.
		if (aip->path_cur - aip->path_dir - aip->path_start >= num_points) {
			if (aip->path_dir == 1)
				aip->path_cur = aip->path_start;
			else
				aip->path_cur = aip->path_start + num_points - 1;
		}

		vec3d	delvec;
		vm_vec_sub(&delvec, cvp, &Path_points[aip->path_cur - aip->path_dir].pos);
		vm_vec_normalize(&delvec);
		vm_vec_scale_add(&next_vec, cvp, &delvec, 10.0f);
		nvp = &next_vec;
	}
	// Set pvp to the previous vertex of interest on the path (if there is one)
	int prev_point = aip->path_cur - aip->path_dir;
	if (prev_point >= aip->path_start && prev_point <= aip->path_start + num_points)
		pvp = &Path_points[prev_point].pos;
	else
		pvp = NULL;

	gpvp = (pvp != NULL) ? *pvp : *cvp;
	gcvp = *cvp;
	gnvp = *nvp;

	speed = Pl_objp->phys_info.speed;

	dist_to_goal = vm_vec_dist_quick(&Pl_objp->pos, &gcvp);
	dist_to_next = vm_vec_dist_quick(&Pl_objp->pos, &gnvp);

	//	Can't use fvec, need to use velocity vector because we aren't necessarily
	//	moving in the direction we're facing.
	if ( vm_vec_mag_quick(&Pl_objp->phys_info.vel) < AICODE_SMALL_MAGNITUDE ) {
		mag = 0.0f;
		vm_vec_zero(&nvel_vec);
	} else
		mag = vm_vec_copy_normalize(&nvel_vec, &Pl_objp->phys_info.vel);

	if (mag < 1.0f)
		nvel_vec = Pl_objp->orient.vec.fvec;

	// For departures, optionally rotate so the ship is pointing the correct direction
	// (so you can always have "wheels-down" landings)
	vec3d rvec;
	vec3d *prvec = NULL;
	if ( aip->mode == AIM_BAY_DEPART && !IS_VEC_NULL(&aip->path_depart_orient) ) {
		vm_vec_unrotate(&rvec, &aip->path_depart_orient, &Objects[aip->path_objnum].orient);
		prvec = &rvec;
	}

	// Turn toward a point between where we are on the path and the goal. This helps keep the ship flying
	// "down the line"
	if (dist_to_goal > 0.1f) {
		//Get nearest point on line between current and previous path points
		vec3d closest_point;
		float r = find_nearest_point_on_line(&closest_point, &gpvp, &gcvp, &Pl_objp->pos);
 
		//Turn towards the 1/3 point between the closest point on the line and the current goal point 
		//(unless we are already past the current goal)
		if (r <= 1.0f) {
			vec3d midpoint;
			vm_vec_avg3(&midpoint, &gcvp, &closest_point, &closest_point);
			ai_turn_towards_vector(&midpoint, Pl_objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0, prvec);
		}
		else {
			ai_turn_towards_vector(&gcvp, Pl_objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0, prvec);
		}
	}

	//	Code to control speed is MUCH less forgiving in path following than in waypoint
	//	following.  Must be very close to path or might hit objects.
	prev_dot_to_goal = aip->prev_dot_to_goal;
	dot = vm_vec_dot_to_point(&nvel_vec, &Pl_objp->pos, &gcvp);
	dot_to_next = vm_vec_dot_to_point(&nvel_vec, &Pl_objp->pos, &gnvp);

	// This path mode respects the "cap-waypoint-speed" SEXP
	float max_allowed_speed = 0;
	if ( aip->waypoint_speed_cap > 0) {
		max_allowed_speed = (float) aip->waypoint_speed_cap;
	}

	set_accel_for_docking(Pl_objp, aip, dot, dot_to_next, dist_to_next, dist_to_goal, sip, max_allowed_speed);
	aip->prev_dot_to_goal = dot;

	//	If moving at a non-tiny velocity, detect attaining path point by its being close to
	//	line between previous and current object location.
	if ((dist_to_goal < MIN_DIST_TO_WAYPOINT_GOAL) || (vm_vec_dist_quick(&Pl_objp->last_pos, &Pl_objp->pos) > 0.1f)) {
		vec3d	nearest_point;
		float		r, min_dist_to_goal;

		r = find_nearest_point_on_line(&nearest_point, &Pl_objp->last_pos, &Pl_objp->pos, &gcvp);

		//	Set min_dist_to_goal = how close must be to waypoint to pick next one.
		//	If docking and this is the second last waypoint, must be very close.
		if ((aip->mode == AIM_DOCK) && (aip->path_cur - aip->path_start >= aip->path_length-1)) {
			min_dist_to_goal = MIN_DIST_TO_WAYPOINT_GOAL;
		}
		//  If this is not the last waypoint, include a term to compensate for inertia
		else if (aip->path_cur - aip->path_start < aip->path_length - (aip->mode == AIM_DOCK ? 0 : 1)) {
			vec3d cp_rel;
			vm_vec_sub(&cp_rel, &gcvp, &Pl_objp->pos);
			float goal_mag = 0.0f;
			if (vm_vec_mag(&cp_rel) > 0.0f) {
				vm_vec_normalize(&cp_rel);
				goal_mag = vm_vec_dot(&Pl_objp->phys_info.vel, &cp_rel);
			}
			min_dist_to_goal = MIN_DIST_TO_WAYPOINT_GOAL + Pl_objp->radius + (goal_mag * sip->damp);
		}
		else {
			min_dist_to_goal = MIN_DIST_TO_WAYPOINT_GOAL + Pl_objp->radius;
		}

		if ( (vm_vec_dist_quick(&Pl_objp->pos, &gcvp) < min_dist_to_goal) ||
			((r >= 0.0f) && (r <= 1.0f)) && (vm_vec_dist_quick(&nearest_point, &gcvp) < (MIN_DIST_TO_WAYPOINT_GOAL + Pl_objp->radius))) {
			aip->path_cur += aip->path_dir;
			if (((aip->path_cur - aip->path_start) > (num_points+1)) || (aip->path_cur < aip->path_start)) {
				Assert(aip->mode != AIM_DOCK);		//	If docking, should never get this far, getting to last point handled outside ai_path()
				aip->path_dir = -aip->path_dir;
			}
		}
	}

	return dist_to_goal;
}

float ai_path()
{
	switch (The_mission.ai_profile->ai_path_mode)
	{
	case AI_PATH_MODE_NORMAL:
		return ai_path_0();
		break;
	case AI_PATH_MODE_ALT1:
		return ai_path_1();
		break;
	default:
		Error(LOCATION, "Invalid path mode found: %d\n", The_mission.ai_profile->ai_path_mode);
		return ai_path_0();
	}
}

// Only needed because CLAMP() doesn't handle pointers
void update_min_max(float val, float *min, float *max)
{
	if (val < *min)
		*min = val;
	else if (val > *max)
		*max = val;
}

//	Stuff bounding box of all enemy objects within "range" units of object *my_objp.
//	Stuff ni min_vec and max_vec.
//	Return value: Number of enemy objects in bounding box.
int get_enemy_team_range(object *my_objp, float range, int enemy_team_mask, vec3d *min_vec, vec3d *max_vec)
{
	object	*objp;
	ship_obj	*so;
	int		count = 0;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];
		if (iff_matches_mask(Ships[objp->instance].team, enemy_team_mask)) {
			if (Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER | SIF_CRUISER | SIF_CAPITAL | SIF_SUPERCAP | SIF_DRYDOCK | SIF_CORVETTE | SIF_AWACS | SIF_GAS_MINER))
				if (vm_vec_dist_quick(&my_objp->pos, &objp->pos) < range) {
					if (count == 0) {
						*min_vec = objp->pos;
						*max_vec = objp->pos;
						count++;
					} else {
						update_min_max(objp->pos.xyz.x, &min_vec->xyz.x, &max_vec->xyz.x);
						update_min_max(objp->pos.xyz.y, &min_vec->xyz.y, &max_vec->xyz.y);
						update_min_max(objp->pos.xyz.z, &min_vec->xyz.z, &max_vec->xyz.z);
					}
				}

		}
	}

	return count;
}

//	Pick a relatively safe spot for objp to fly to.
//	Problem:
//		Finds a spot away from any enemy within a bounding box.
//		Doesn't verify that "safe spot" is not near some other enemy.
void ai_safety_pick_spot(object *objp)
{
	int		objnum;
	int		enemy_team_mask;
	vec3d	min_vec, max_vec;
	vec3d	vec_to_center, center;
	vec3d	goal_pos;

	objnum = OBJ_INDEX(objp);

	enemy_team_mask = iff_get_attacker_mask(obj_team(&Objects[objnum]));

	if (get_enemy_team_range(objp, 1000.0f, enemy_team_mask, &min_vec, &max_vec)) {
		vm_vec_avg(&center, &min_vec, &max_vec);
		vm_vec_normalized_dir(&vec_to_center, &center, &objp->pos);

		vm_vec_scale_add(&goal_pos, &center, &vec_to_center, 2000.0f);
	} else
		vm_vec_scale_add(&goal_pos, &objp->pos, &objp->orient.vec.fvec, 100.0f);

	Ai_info[Ships[objp->instance].ai_index].goal_point = goal_pos;
}

//	Fly to desired safe point.
// Returns distance to that point.
float ai_safety_goto_spot(object *objp)
{
	float	dot, dist;
	ai_info	*aip;
	vec3d	vec_to_goal;
	ship_info	*sip;
	float	dot_val;

	sip = &Ship_info[Ships[objp->instance].ship_info_index];

	aip = &Ai_info[Ships[objp->instance].ai_index];
	dist = vm_vec_normalized_dir(&vec_to_goal, &aip->goal_point, &objp->pos);
	dot = vm_vec_dot(&vec_to_goal, &objp->orient.vec.fvec);

	dot_val = (1.1f + dot) / 2.0f;
	if (dist > 200.0f) {
		set_accel_for_target_speed(objp, sip->max_speed * dot_val);
	} else
		set_accel_for_target_speed(objp, sip->max_speed * dot_val * (dist/200.0f + 0.2f));

	return dist;
}

void ai_safety_circle_spot(object *objp)
{
	vec3d	goal_point;
	ship_info	*sip;
	float		dot;

	sip = &Ship_info[Ships[objp->instance].ship_info_index];

	goal_point = Ai_info[Ships[objp->instance].ai_index].goal_point;
	dot = turn_towards_tangent(objp, &goal_point, 250.0f);	//	Increased from 50 to 250 to make circling not look so wacky.

	set_accel_for_target_speed(objp, 0.5f * (1.0f + dot) * sip->max_speed/4.0f);
}

//	--------------------------------------------------------------------------
void ai_safety()
{
	ai_info	*aip;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	switch (aip->submode) {
	case AISS_1:
		ai_safety_pick_spot(Pl_objp);
		aip->submode = AISS_2;
		aip->submode_start_time = Missiontime;
		break;
	case AISS_1a:	//	Pick a safe point because we just got whacked!
		Int3();
		break;
	case AISS_2:
		if (ai_safety_goto_spot(Pl_objp) < 25.0f) {
			aip->submode = AISS_3;
			aip->submode_start_time = Missiontime;
		}
		break;
	case AISS_3:
		ai_safety_circle_spot(Pl_objp);
		break;
	default:
		Int3();		//	Illegal submode for ai_safety();
		break;
	}
}


//	--------------------------------------------------------------------------
// make Pl_objp fly to its target's position
// optionally returns true if Pl_objp (pl_done_p) has reached the target position (can be NULL)
// optionally returns true if Pl_objp's (pl_treat_as_ship_p) fly to order was issued to him directly,
// or if the order was issued to his wing (returns false) (can be NULL, and result is only
// valid if Pl_done_p was not NULL and was set true by function.
void ai_fly_to_target_position(vec3d* target_pos, bool* pl_done_p=NULL, bool* pl_treat_as_ship_p=NULL)
{
	float		dot, dist_to_goal, speed;
	ship		*shipp = &Ships[Pl_objp->instance];
	ship_info	*sip = &Ship_info[shipp->ship_info_index];
	ai_info	*aip;
	vec3d	nvel_vec;
	float		mag;
	float		prev_dot_to_goal;
	vec3d	temp_vec;
	vec3d	*slop_vec;
	int j;

	Assert( target_pos != NULL );

	if ( pl_done_p != NULL ) {
		// initialize this to false now incase we leave early so that that
		// the caller of this function doesn't get any funny ideas about
		// the status of the waypoint
		*pl_done_p = false;
	}

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	/* I shouldn't be flying to position for what ever called me any more.
	Set mode to none so that default dynamic behaviour gets started up again. */
	if ( (aip->active_goal == AI_GOAL_NONE) || (aip->active_goal == AI_ACTIVE_GOAL_DYNAMIC) ) {
		aip->mode = AIM_NONE;
	}

	speed = Pl_objp->phys_info.speed;

	dist_to_goal = vm_vec_dist_quick(&Pl_objp->pos, target_pos);

	//	Can't use fvec, need to use velocity vector because we aren't necessarily
	//	moving in the direction we're facing.
	// AL 23-3-98: Account for very small velocities by checking result of vm_vec_mag().
	//					If we don't vm_vec_copy_normalize() will think it is normalizing a null vector.
	if ( vm_vec_mag_quick(&Pl_objp->phys_info.vel) < AICODE_SMALL_MAGNITUDE ) {
		mag = 0.0f;
		vm_vec_zero(&nvel_vec);
	} else {
		mag = vm_vec_copy_normalize(&nvel_vec, &Pl_objp->phys_info.vel);
	}

	//	If moving not-very-slowly and sliding, then try to slide at goal, rather than
	//	point at goal.
	slop_vec = NULL;
	if (mag < 1.0f) {
		nvel_vec = Pl_objp->orient.vec.fvec;
	} else if (mag > 5.0f) {
		float	nv_dot;
		nv_dot = vm_vec_dot(&Pl_objp->orient.vec.fvec, &nvel_vec);
		if ((nv_dot > 0.5f) && (nv_dot < 0.97f)) {
			slop_vec = &temp_vec;
			vm_vec_sub(slop_vec, &nvel_vec, &Pl_objp->orient.vec.fvec);
		}
	}

	//	If a wing leader, take turns more slowly, based on size of wing.
	int	scale;

	if (aip->wing >= 0) {
		scale = Wings[aip->wing].current_count;
		scale = (int) ((scale+1)/2);
	} else {
		scale = 1;
	}

	// ----------------------------------------------
	// if in autopilot mode make sure to not collide
	// and "keep reasonable distance" 
	// this needs to be done for ALL SHIPS not just capships STOP CHANGING THIS
	// ----------------------------------------------

	vec3d perp, goal_point;

	bool carry_flag = ((shipp->flags2 & SF2_NAVPOINT_CARRY) || ((shipp->wingnum >= 0) && (Wings[shipp->wingnum].flags & WF_NAV_CARRY)));

	if (AutoPilotEngaged
		&& timestamp_elapsed(LockAPConv)
		&& carry_flag
		&& ((The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS) || (Pl_objp != Autopilot_flight_leader)) )
	{
		Assertion( Autopilot_flight_leader != NULL, "When under autopilot there must be a flight leader" );
		// snap wings into formation them into formation
		if (The_mission.flags & MISSION_FLAG_USE_AP_CINEMATICS) {
			if (aip->wing != -1) {
				int wing_index = get_wing_index(Pl_objp, aip->wing);
				object *wing_leader = get_wing_leader(aip->wing);

				if (wing_leader != Pl_objp) {
					// not wing leader.. get my position relative to wing leader
					get_absolute_wing_pos_autopilot(&goal_point, wing_leader, wing_index, aip->ai_flags & AIF_FORMATION_OBJECT);
				} else {
					// Am wing leader.. get the wings position relative to the flight leader
					j = 1+int( (float)floor(double(autopilot_wings[aip->wing]-1)/2.0) );

					switch (autopilot_wings[aip->wing] % 2) {
						case 1: // back-left
							vm_vec_copy_normalize(&perp, &Autopilot_flight_leader->orient.vec.rvec);
							vm_vec_scale(&perp, -166.0f*j); // 166m is supposedly the optimal range according to tolwyn
							vm_vec_add(&goal_point, &Autopilot_flight_leader->pos, &perp);
							break;

						default: //back-right
						case 0:
							vm_vec_copy_normalize(&perp, &Autopilot_flight_leader->orient.vec.rvec);
							vm_vec_scale(&perp, 166.0f*j);
							vm_vec_add(&goal_point, &Autopilot_flight_leader->pos, &perp);
							break;
					}

				}

				Pl_objp->pos = goal_point;
			}

			vm_vec_sub(&perp, Navs[CurrentNav].GetPosition(), &Autopilot_flight_leader->pos);
			vm_vector_2_matrix(&Pl_objp->orient, &perp, NULL, NULL);
		} else {
			vm_vec_scale_add(&perp, &Pl_objp->pos, &Autopilot_flight_leader->phys_info.vel, 1000.0f);
			ai_turn_towards_vector(&perp, Pl_objp, flFrametime, sip->srotation_time*3.0f*scale, slop_vec, NULL, 0.0f, 0);
		}
	} else {
		if (dist_to_goal > 0.1f) {
			ai_turn_towards_vector(target_pos, Pl_objp, flFrametime, sip->srotation_time*3.0f*scale, slop_vec, NULL, 0.0f, 0);
		}
	}

	// ----------------------------------------------

	prev_dot_to_goal = aip->prev_dot_to_goal;
	dot = vm_vec_dot_to_point(&nvel_vec, &Pl_objp->pos, target_pos);
	aip->prev_dot_to_goal = dot;

	if (Pl_objp->phys_info.speed < 0.0f) {
		accelerate_ship(aip, 1.0f/32);
	} else if (prev_dot_to_goal > dot+0.01f) {
		//	We are further from pointing at our goal this frame than last frame, so slow down.
		set_accel_for_target_speed(Pl_objp, Pl_objp->phys_info.speed * 0.95f);
	} else if (dist_to_goal < 100.0f) {
		float slew_dot = vm_vec_dot(&Pl_objp->orient.vec.fvec, &nvel_vec);
		if (fl_abs(slew_dot) < 0.9f) {
			accelerate_ship(aip, 0.0f);
		} else if (dot < 0.88f + 0.1f*(100.0f - dist_to_goal)/100.0f) {
			accelerate_ship(aip, 0.0f);
		} else {
			accelerate_ship(aip, 0.5f * dot * dot);
		}
	} else {
		float	dot1;
		if (dist_to_goal < 250.0f) {
			dot1 = dot*dot*dot;				//	Very important to be pointing towards goal when nearby.  Note, cubing preserves sign.
		} else {
			if (dot > 0.0f) {
				dot1 = dot*dot;
			} else {
				dot1 = dot;
			}
		}

		if (dist_to_goal > 100.0f + Pl_objp->radius * 2) {
			if (dot < 0.2f) {
				dot1 = 0.2f;
			}
		}

		if (sip->flags & SIF_SMALL_SHIP) {
			set_accel_for_target_speed(Pl_objp, dot1 * dist_to_goal/5.0f);
		} else {
			set_accel_for_target_speed(Pl_objp, dot1 * dist_to_goal/10.0f);
		}
	}

	//	Make sure not travelling too fast for someone to keep up.
	float	max_allowed_speed = 9999.9f;

	if (shipp->wingnum != -1) {
		max_allowed_speed = 0.9f * get_wing_lowest_max_speed(Pl_objp);
	}

	// check if waypoint speed cap is set and adjust max speed
	if (aip->waypoint_speed_cap > 0) {
		max_allowed_speed = (float) aip->waypoint_speed_cap;
	}

	if (aip->prev_accel * shipp->current_max_speed > max_allowed_speed) {
		accelerate_ship(aip, max_allowed_speed / shipp->current_max_speed);
	}

	if ( (dist_to_goal < MIN_DIST_TO_WAYPOINT_GOAL) || (vm_vec_dist_quick(&Pl_objp->last_pos, &Pl_objp->pos) > 0.1f) ) {
		vec3d	nearest_point;
		float		r;

		r = find_nearest_point_on_line(&nearest_point, &Pl_objp->last_pos, &Pl_objp->pos, target_pos);

		if ( (dist_to_goal < (MIN_DIST_TO_WAYPOINT_GOAL + fl_sqrt(Pl_objp->radius) + vm_vec_dist_quick(&Pl_objp->pos, &Pl_objp->last_pos)))
			|| (((r >= 0.0f) && (r <= 1.0f)) && (vm_vec_dist_quick(&nearest_point, target_pos) < (MIN_DIST_TO_WAYPOINT_GOAL + fl_sqrt(Pl_objp->radius)))))
		{
				int treat_as_ship;

				// we must be careful when dealing with wings.  A ship in a wing might be completing
				// a waypoint for for the entire wing, or it might be completing a goal for itself.  If
				// for itself and in a wing, treat the completion as we would a ship
				treat_as_ship = 1;
				if ( Ships[Pl_objp->instance].wingnum != -1 ) {
					int type;

					type = aip->goals[aip->active_goal].type;
					if ( (type == AIG_TYPE_EVENT_WING) || (type == AIG_TYPE_PLAYER_WING) ) {
						treat_as_ship = 0;
					} else {
						treat_as_ship = 1;
					}
				}
				// setup out parameters
				if ( pl_done_p != NULL ) {
					*pl_done_p = true;
					if ( pl_treat_as_ship_p != NULL ) {
						if ( treat_as_ship ) {
							*pl_treat_as_ship_p = true;
						} else {
							*pl_treat_as_ship_p = false;
						}
					}
				} else {
					Assertion( pl_treat_as_ship_p != NULL,
						"pl_done_p cannot be NULL while pl_treat_as_ship_p is not NULL" );
				}
		}
	}
}

//	--------------------------------------------------------------------------
//	make Pl_objp fly waypoints.
void ai_waypoints()
{
	ai_info	*aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	// sanity checking for stuff that should never happen
	if (aip->wp_index == INVALID_WAYPOINT_POSITION) {
		if (aip->wp_list == NULL) {
			Warning(LOCATION, "Waypoints should have been assigned already!");
			ai_start_waypoints(Pl_objp, &Waypoint_lists.front(), WPF_REPEAT);
		} else {
			Warning(LOCATION, "Waypoints should have been started already!");
			ai_start_waypoints(Pl_objp, aip->wp_list, WPF_REPEAT);
		}
	}
	
	Assert(!aip->wp_list->get_waypoints().empty());	// What? Is this zero? Probably never got initialized!

	bool done, treat_as_ship;
	ai_fly_to_target_position(aip->wp_index->get_pos(), &done, &treat_as_ship);

	if ( done ) {
		// go on to next waypoint in path
		++aip->wp_index;

		if ( aip->wp_index == aip->wp_list->get_waypoints().end() ) {
			// have reached the last waypoint.  Do I repeat?
			if ( aip->wp_flags & WPF_REPEAT ) {
				 // go back to the start.
				aip->wp_index = aip->wp_list->get_waypoints().begin();
			} else {
				// stay on the last waypoint.
				--aip->wp_index;

				// Log a message that the wing or ship reached his waypoint and
				// remove the goal from the AI goals of the ship pr wing, respectively.
				// Whether we should treat this as a ship or a wing is determined by
				// ai_fly_to_target_position when it marks the AI directive as complete
				if ( treat_as_ship ) {
					ai_mission_goal_complete( aip );					// this call should reset the AI mode
					mission_log_add_entry( LOG_WAYPOINTS_DONE, Ships[Pl_objp->instance].ship_name, aip->wp_list->get_name(), -1 );
				} else {
					ai_mission_wing_goal_complete( Ships[Pl_objp->instance].wingnum, &(aip->goals[aip->active_goal]) );
					mission_log_add_entry( LOG_WAYPOINTS_DONE, Wings[Ships[Pl_objp->instance].wingnum].name, aip->wp_list->get_name(), -1 );
				}
			}
		}
	}
}

//	--------------------------------------------------------------------------
//	make Pl_objp fly toward a ship
void ai_fly_to_ship()
{
	ai_info	*aip;
	object* target_p = NULL;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	if ( aip->mode != AIM_FLY_TO_SHIP ) {
		Warning(LOCATION,
			"ai_fly_to_ship called for '%s' when ai_info.mode not equal to AIM_FLY_TO_SHIP. Is actually '%d'",
			Ships[Pl_objp->instance].ship_name,
			aip->mode);
		aip->mode = AIM_NONE;
		return;
	}
	if ( aip->active_goal < 0 || aip->active_goal >= MAX_AI_GOALS ) {
		Warning(LOCATION,
			"'%s' is trying to fly-to a ship without an active AI_GOAL\n\n"
			"Active ai mode is '%d'",
			Ships[Pl_objp->instance].ship_name,
			aip->active_goal);
		aip->mode = AIM_NONE;
		return;
	}
	Assert( aip->goals[aip->active_goal].target_name != NULL );
	if ( aip->goals[aip->active_goal].target_name[0] == '\0' ) {
		Warning(LOCATION, "'%s' is trying to fly-to-ship without a name for the ship", Ships[Pl_objp->instance].ship_name);
		aip->mode = AIM_NONE;
		ai_remove_ship_goal( aip, aip->active_goal ); // function sets aip->active_goal to NONE for me
		return;
	}

	for (int j = 0; j < MAX_SHIPS; j++)
	{
		if (Ships[j].objnum != -1 && !stricmp(aip->goals[aip->active_goal].target_name, Ships[j].ship_name))
		{
			target_p = &Objects[Ships[j].objnum];
			break;
		}
	}

	if ( target_p == NULL ) {
		#ifndef NDEBUG
		for (int i = 0; i < MAX_AI_GOALS; i++)
		{
			if (aip->mode == AIM_FLY_TO_SHIP || aip->goals[i].ai_mode == AI_GOAL_FLY_TO_SHIP)
			{
				mprintf(("Ship '%s' told to fly to target ship '%s'",
					Ships[Pl_objp->instance].ship_name,
					aip->goals[i].target_name));
			}
		}
		#endif
		Warning(LOCATION, "Ship '%s' told to fly to a ship but none of the ships it was told to fly to exist.\n"
			"See log before this message for list of ships set as fly-to tagets",
			Ships[Pl_objp->instance].ship_name);
		aip->mode = AIM_NONE;
		ai_remove_ship_goal( aip, aip->active_goal ); // function sets aip->active_goal to NONE for me
		return;
	} else {
		bool done, treat_as_ship;
		ai_fly_to_target_position(&(target_p->pos), &done, &treat_as_ship);

		if ( done ) {
			// remove the goal from the AI goals of the ship pr wing, respectively.
			// Wether or not we should treat this as a ship or a wing is determined by
			// ai_fly_to_target when it marks the AI directive as complete
			if ( treat_as_ship ) {
				ai_mission_goal_complete( aip );					// this call should reset the AI mode
			} else {
				ai_mission_wing_goal_complete( Ships[Pl_objp->instance].wingnum, &(aip->goals[aip->active_goal]) );
			}
		}
	}
}

//	Make Pl_objp avoid En_objp
//	Not like evading.  This is for avoiding a collision!
//	Note, use sliding if available.
void avoid_ship()
{
	//	To avoid an object, turn towards right or left vector until facing away from object.
	//	To choose right vs. left, pick one that is further from center of avoid object.
	//	Keep turning away from until pointing away from ship.
	//	Stay in avoid mode until at least 3 enemy ship radii away.

	//	Speed setting:
	//	If inside sphere, zero speed and turn towards outside.
	//	If outside sphere, inside 2x sphere, set speed percent of max to:
	//		max(away_dot, (dist-rad)/rad)
	//	where away_dot is dot(Pl_objp->fvec, vec_En_objp_to_Pl_objp)

	vec3d	vec_to_enemy;
	float		away_dot;
	float		dist;
	ship		*shipp = &Ships[Pl_objp->instance];
	ship_info	*sip = &Ship_info[shipp->ship_info_index];
	ai_info	*aip = &Ai_info[shipp->ai_index];
	vec3d	player_pos, enemy_pos;

	// if we're avoiding a stealth ship, then we know where he is, update with no error
	if ( is_object_stealth_ship(En_objp) ) {
		update_ai_stealth_info_with_error(aip/*, 1*/);
	}

	ai_set_positions(Pl_objp, En_objp, aip, &player_pos, &enemy_pos);
	vm_vec_sub(&vec_to_enemy, &enemy_pos, &Pl_objp->pos);

	dist = vm_vec_normalize(&vec_to_enemy);
	away_dot = -vm_vec_dot(&Pl_objp->orient.vec.fvec, &vec_to_enemy);
	
	if ((sip->max_vel.xyz.x > 0.0f) || (sip->max_vel.xyz.y > 0.0f)) {
		if (vm_vec_dot(&Pl_objp->orient.vec.rvec, &vec_to_enemy) > 0.0f) {
			AI_ci.sideways = -1.0f;
		} else {
			AI_ci.sideways = 1.0f;
		}
		if (vm_vec_dot(&Pl_objp->orient.vec.uvec, &vec_to_enemy) > 0.0f) {
			AI_ci.vertical = -1.0f;
		} else {
			AI_ci.vertical = 1.0f;
		}
	}		

	//	If in front of enemy, turn away from it.
	//	If behind enemy, try to get fully behind it.
	if (away_dot < 0.0f) {
		turn_away_from_point(Pl_objp, &enemy_pos, Pl_objp->phys_info.speed);
	} else {
		vec3d	goal_pos;

		vm_vec_scale_add(&goal_pos, &En_objp->pos, &En_objp->orient.vec.fvec, -100.0f);
		turn_towards_point(Pl_objp, &goal_pos, NULL, Pl_objp->phys_info.speed);
	}

	//	Set speed.
	float	radsum = Pl_objp->radius + En_objp->radius;

	if (dist < radsum)
		accelerate_ship(aip, MAX(away_dot, 0.2f));
	else if (dist < 2*radsum)
		accelerate_ship(aip, MAX(away_dot, (dist - radsum) / radsum)+0.2f);
	else
		accelerate_ship(aip, 1.0f);

}

//	Maybe it's time to resume the previous AI mode in aip->previous_mode.
//	Each type of previous_mode has its own criteria on when to resume.
//	Return true if previous mode was resumed.
int maybe_resume_previous_mode(object *objp, ai_info *aip)
{
	//	Only (maybe) resume previous goal if current goal is dynamic.
	if (aip->active_goal != AI_ACTIVE_GOAL_DYNAMIC)
		return 0;

	if (aip->mode == AIM_EVADE_WEAPON) {
		if (timestamp_elapsed(aip->mode_time) || (((aip->nearest_locked_object == -1) || (Objects[aip->nearest_locked_object].type != OBJ_WEAPON)) && (aip->danger_weapon_objnum == -1))) {
			Assert(aip->previous_mode != AIM_EVADE_WEAPON);
			aip->mode = aip->previous_mode;
			aip->submode = aip->previous_submode;
			aip->submode_start_time = Missiontime;
			aip->active_goal = AI_GOAL_NONE;
			aip->mode_time = -1;			//	Means do forever.
			return 1;
		}
	} else if ( aip->previous_mode == AIM_GUARD) {
		if ((aip->guard_objnum != -1) && (aip->guard_signature == Objects[aip->guard_objnum].signature)) {
			object	*guard_objp;
			float	dist;

			guard_objp = &Objects[aip->guard_objnum];
			dist = vm_vec_dist_quick(&guard_objp->pos, &objp->pos);

			//	If guarding ship is far away from guardee and enemy is far away from guardee,
			//	then stop chasing and resume guarding.
			if (dist > (MAX_GUARD_DIST + guard_objp->radius) * 6) {
				if ((En_objp != NULL) && (En_objp->type == OBJ_SHIP)) {
					if (vm_vec_dist_quick(&guard_objp->pos, &En_objp->pos) > (MAX_GUARD_DIST + guard_objp->radius) * 6) {
						Assert(aip->previous_mode == AIM_GUARD);
						aip->mode = aip->previous_mode;
						aip->submode = AIS_GUARD_PATROL;
						aip->submode_start_time = Missiontime;
						aip->active_goal = AI_GOAL_NONE;
						return 1;
					}
				}
			}
		}
	}

	return 0;

}

//	Call this function if you want something to happen on average every N quarters of a second.
//	The truth value returned by this function will be the same for any given quarter second interval.
//	The value "num" is only passed in to get asynchronous behavior for different objects.
//	modulus == 1 will always return true.
//	modulus == 2 will return true half the time.
//	modulus == 16 will return true for one quarter second interval every four seconds.
int static_rand_timed(int num, int modulus)
{
	if (modulus < 2)
		return 1;
	else {
		int	t;

		t = Missiontime >> 18;		//	Get time in quarters of a second (SUSHI: this comment is wrong! It's actually every 4 seconds!)
		t += num;

		return !(t % modulus);
	}
}

/**
 * Maybe fire afterburner based on AI class
 */
int ai_maybe_fire_afterburner(object *objp, ai_info *aip)
{
	// bail if the ship doesn't even have an afterburner
	if (!(Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_AFTERBURNER)) {
		return 0;
	}
	if (aip->ai_aburn_use_factor == INT_MIN && aip->ai_class == 0) {
		return 0;		//	Lowest level never aburners away (unless ai_aburn_use_factor is specified)
	} 
	else {
		//	Maybe don't afterburner because of a potential collision with the player.
		//	If not multiplayer, near player and player in front, probably don't afterburner.
		if (!(Game_mode & GM_MULTIPLAYER)) {
			if (Ships[objp->instance].team == Player_ship->team) {
				float	dist;

				dist = vm_vec_dist_quick(&objp->pos, &Player_obj->pos) - Player_obj->radius - objp->radius;
				if (dist < 150.0f) {
					vec3d	v2p;
					float		dot;

					vm_vec_normalized_dir(&v2p, &Player_obj->pos, &objp->pos);
					dot = vm_vec_dot(&v2p, &objp->orient.vec.fvec);

					if (dot > 0.0f) {
						if (dot * dist > 50.0f)
							return 0;
					}
				}
			}
		}

		if (aip->ai_aburn_use_factor == INT_MIN && aip->ai_class >= Num_ai_classes-2)
			return 1;		//	Highest two levels always aburner away (unless ai_aburn_use_factor is specified).
		else {
			//If ai_aburn_use_factor is not specified, calculate a number based on the AI class. Otherwise, use that value.
			if (aip->ai_aburn_use_factor == INT_MIN)
				return static_rand_timed(objp-Objects, Num_ai_classes - aip->ai_class);
			else
				return static_rand_timed(objp-Objects, aip->ai_aburn_use_factor);
		}
	}
}

/**
 * Maybe engage afterburner after being hit by an object.
 */
void maybe_afterburner_after_ship_hit(object *objp, ai_info *aip, object *en_objp)
{
	//	Only do if facing a little away.
	if (en_objp != NULL) {
		vec3d	v2e;

		vm_vec_normalized_dir(&v2e, &en_objp->pos, &objp->pos);
		if (vm_vec_dot(&v2e, &objp->orient.vec.fvec) > -0.5f)
			return;
	}

	if (!( objp->phys_info.flags & PF_AFTERBURNER_ON )) {
		if (ai_maybe_fire_afterburner(objp, aip)) {
			afterburners_start(objp);
			aip->afterburner_stop_time = Missiontime + F1_0/2;
		}
	}
}

/**
 * Is an instructor if name begins INSTRUCTOR_SHIP_NAME else not.
 * @return true if object *objp is an instructor.
 */
int is_instructor(object *objp)
{
	return !strnicmp(Ships[objp->instance].ship_name, INSTRUCTOR_SHIP_NAME, strlen(INSTRUCTOR_SHIP_NAME));
}

//	Evade the weapon aip->danger_weapon_objnum
//	If it's not valid, do a quick out.
//	Evade by accelerating hard.
//	If necessary, turn hard left or hard right.
void evade_weapon()
{
	object	*weapon_objp = NULL;
	object	*unlocked_weapon_objp = NULL, *locked_weapon_objp = NULL;
	vec3d	weapon_pos, player_pos, goal_point;
	vec3d	vec_from_enemy;
	float		dot_from_enemy, dot_to_enemy;
	float		dist;
	ship		*shipp = &Ships[Pl_objp->instance];
	ai_info	*aip = &Ai_info[shipp->ai_index];

	if (is_instructor(Pl_objp))
		return;

	//	Make sure we're actually being attacked.
	//	Favor locked objects.
	if (aip->nearest_locked_object != -1) {
		if (Objects[aip->nearest_locked_object].type == OBJ_WEAPON)
			locked_weapon_objp = &Objects[aip->nearest_locked_object];
	}
	
	if (aip->danger_weapon_objnum != -1)
		if (Objects[aip->danger_weapon_objnum].signature == aip->danger_weapon_signature)
			unlocked_weapon_objp = &Objects[aip->danger_weapon_objnum];
		else
			aip->danger_weapon_objnum = -1;		//	Signatures don't match, so no longer endangered.

	if (locked_weapon_objp != NULL) {
		if (unlocked_weapon_objp != NULL) {
			if (vm_vec_dist_quick(&locked_weapon_objp->pos, &Pl_objp->pos) < 1.5f * vm_vec_dist_quick(&unlocked_weapon_objp->pos, &Pl_objp->pos))
				weapon_objp = locked_weapon_objp;
			else
				weapon_objp = unlocked_weapon_objp;
		} else
			weapon_objp = locked_weapon_objp;
	} else if (unlocked_weapon_objp != NULL)
		weapon_objp = unlocked_weapon_objp;
	else {
		if (aip->mode == AIM_EVADE_WEAPON)
			maybe_resume_previous_mode(Pl_objp, aip);
		return;
	}

	Assert(weapon_objp != NULL);

	if (weapon_objp->type != OBJ_WEAPON) {
		if (aip->mode == AIM_EVADE_WEAPON)
			maybe_resume_previous_mode(Pl_objp, aip);
		return;
	}
	
	weapon_pos = weapon_objp->pos;
	player_pos = Pl_objp->pos;

	//	Make speed based on skill level, varying at highest skill level, which is harder to hit.
	accelerate_ship(aip, 1.0f);

	dist = vm_vec_normalized_dir(&vec_from_enemy, &player_pos, &weapon_pos);

	dot_to_enemy = -vm_vec_dot(&Pl_objp->orient.vec.fvec, &vec_from_enemy);
	dot_from_enemy = vm_vec_dot(&weapon_objp->orient.vec.fvec, &vec_from_enemy);

	//	If shot is incoming...
	if (dot_from_enemy < 0.3f) {
		if (weapon_objp == unlocked_weapon_objp)
			aip->danger_weapon_objnum = -1;
		return;
	} else if (dot_from_enemy > 0.7f) {
		if (dist < 200.0f) {
			if (!( Pl_objp->phys_info.flags & PF_AFTERBURNER_ON )) {
				if (ai_maybe_fire_afterburner(Pl_objp, aip)) {
					afterburners_start(Pl_objp);
					aip->afterburner_stop_time = Missiontime + F1_0/2;
				}
			}
		}

		//	If we're sort of pointing towards it...
		if ((dot_to_enemy < -0.5f) || (dot_to_enemy > 0.5f)) {
			float rdot;
			float udot;

			//	Turn hard left or right, depending on which gets out of way quicker.
			//SUSHI: Also possibly turn up or down. 
			rdot = vm_vec_dot(&Pl_objp->orient.vec.rvec, &vec_from_enemy);
			udot = vm_vec_dot(&Pl_objp->orient.vec.uvec, &vec_from_enemy);

			if (aip->ai_profile_flags & AIPF_ALLOW_VERTICAL_DODGE && abs(udot) > abs(rdot))
			{
				if ((udot < -0.5f) || (udot > 0.5f))
					vm_vec_scale_add(&goal_point, &Pl_objp->pos, &Pl_objp->orient.vec.uvec, -200.0f);
				else
					vm_vec_scale_add(&goal_point, &Pl_objp->pos, &Pl_objp->orient.vec.uvec, 200.0f);
			}
			else
			{
				if ((rdot < -0.5f) || (rdot > 0.5f))
					vm_vec_scale_add(&goal_point, &Pl_objp->pos, &Pl_objp->orient.vec.rvec, -200.0f);
				else
					vm_vec_scale_add(&goal_point, &Pl_objp->pos, &Pl_objp->orient.vec.rvec, 200.0f);
			}

			turn_towards_point(Pl_objp, &goal_point, NULL, 0.0f);
		}
	}

}

//	Use sliding and backwards moving to face enemy.
//	(Coded 2/20/98.  Works fine, but it's hard to see how to integrate it into the AI system.
//	 Typically ships are moving so fast that a little sliding isn't enough to gain an advantage.
//	 It's currently used to avoid collisions and could be used to evade weapon fire, but the latter
//	 would be frustrating, I think.
//	 This function is currently not called.)
void slide_face_ship()
{
	ship_info	*sip;

	sip = &Ship_info[Ships[Pl_objp->instance].ship_info_index];

	//	If can't slide, return.
	if ((sip->max_vel.xyz.x == 0.0f) && (sip->max_vel.xyz.y == 0.0f))
		return;

	vec3d	goal_pos;
	float		dot_from_enemy, dot_to_enemy;
	vec3d	vec_from_enemy, vec_to_goal;
	float		dist;
	float		up, right;
	ai_info		*aip;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	dist = vm_vec_normalized_dir(&vec_from_enemy, &Pl_objp->pos, &En_objp->pos);

	ai_turn_towards_vector(&En_objp->pos, Pl_objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0);

	dot_from_enemy = vm_vec_dot(&vec_from_enemy, &En_objp->orient.vec.fvec);
	dot_to_enemy = -vm_vec_dot(&vec_from_enemy, &Pl_objp->orient.vec.fvec);

	if (vm_vec_dot(&vec_from_enemy, &En_objp->orient.vec.rvec) > 0.0f)
		right = 1.0f;
	else
		right = -1.0f;

	if (vm_vec_dot(&vec_from_enemy, &En_objp->orient.vec.uvec) > 0.0f)
		up = 1.0f;
	else
		up = -1.0f;

	vm_vec_scale_add(&goal_pos, &En_objp->pos, &En_objp->orient.vec.rvec, right * 200.0f);
	vm_vec_scale_add(&goal_pos, &En_objp->pos, &En_objp->orient.vec.uvec, up * 200.0f);

	vm_vec_normalized_dir(&vec_to_goal, &goal_pos, &Pl_objp->pos);

	if (vm_vec_dot(&vec_to_goal, &Pl_objp->orient.vec.rvec) > 0.0f)
		AI_ci.sideways = 1.0f;
	else
		AI_ci.sideways = -1.0f;

	if (vm_vec_dot(&vec_to_goal, &Pl_objp->orient.vec.uvec) > 0.0f)
		AI_ci.vertical = 1.0f;
	else
		AI_ci.vertical = -1.0f;

	if (dist < 200.0f) {
		if (dot_from_enemy < 0.7f)
			accelerate_ship(aip, -1.0f);
		else
			accelerate_ship(aip, dot_from_enemy + 0.5f);
	} else {
		if (dot_from_enemy < 0.7f) {
			accelerate_ship(aip, 0.2f);
		} else {
			accelerate_ship(aip, 1.0f);
		}
	}
}

//	General code for handling one ship evading another.
//	Problem: This code is also used for avoiding an impending collision.
//	In such a case, it is not good to go to max speed, which is often good
//	for a certain kind of evasion.
void evade_ship()
{
	vec3d	player_pos, enemy_pos, goal_point;
	vec3d	vec_from_enemy;
	float		dot_from_enemy;
	float		dist;
	ship		*shipp = &Ships[Pl_objp->instance];
	ship_info	*sip = &Ship_info[shipp->ship_info_index];
	ai_info	*aip = &Ai_info[shipp->ai_index];
	float		bank_override = 0.0f;

	ai_set_positions(Pl_objp, En_objp, aip, &player_pos, &enemy_pos);

	//	Make speed based on skill level, varying at highest skill level, which is harder to hit.
	if (Game_skill_level == NUM_SKILL_LEVELS-1) {
		int	rand_int;
		float	accel_val;

		rand_int = static_rand(Pl_objp-Objects);
		accel_val = (float) (((Missiontime^rand_int) >> 14) & 0x0f)/32.0f + 0.5f;
		accelerate_ship(aip, accel_val);
	} else
		accelerate_ship(aip, (float) (Game_skill_level+2) / (NUM_SKILL_LEVELS+1));

	if ((Missiontime - aip->submode_start_time > F1_0/2) && (sip->afterburner_fuel_capacity > 0.0f)) {
		float percent_left = 100.0f * shipp->afterburner_fuel / sip->afterburner_fuel_capacity;
		if (percent_left > 30.0f + ((Pl_objp-Objects) & 0x0f)) {
			afterburners_start(Pl_objp);
		
			if (aip->ai_profile_flags & AIPF_SMART_AFTERBURNER_MANAGEMENT) {
				aip->afterburner_stop_time = (fix) (Missiontime + F1_0 + static_randf(Pl_objp-Objects) * F1_0 / 4);
			} else {				
				aip->afterburner_stop_time = Missiontime + F1_0 + static_rand(Pl_objp-Objects)/4;
			}
		}
	}

	vm_vec_sub(&vec_from_enemy, &player_pos, &enemy_pos);

	dist = vm_vec_normalize(&vec_from_enemy);
	dot_from_enemy = vm_vec_dot(&En_objp->orient.vec.fvec, &vec_from_enemy);

	if (dist > 250.0f) {
		vec3d	gp1, gp2;
		//	If far away from enemy, circle, going to nearer of point far off left or right wing
		vm_vec_scale_add(&gp1, &enemy_pos, &En_objp->orient.vec.rvec, 250.0f);
		vm_vec_scale_add(&gp2, &enemy_pos, &En_objp->orient.vec.rvec, -250.0f);
		if (vm_vec_dist_quick(&gp1, &Pl_objp->pos) < vm_vec_dist_quick(&gp2, &Pl_objp->pos))
			goal_point = gp1;
		else
			goal_point = gp2;
	} else if (dot_from_enemy < 0.1f) {
		//	If already close to behind, goal is to get completely behind.
		vm_vec_scale_add(&goal_point, &enemy_pos, &En_objp->orient.vec.fvec, -1000.0f);
	} else if (dot_from_enemy > 0.9f) {
		//	If enemy pointing almost right at self, and self pointing close to enemy, turn away from
		vec3d	vec_to_enemy;
		float		dot_to_enemy;

		vm_vec_sub(&vec_to_enemy, &enemy_pos, &player_pos);

		vm_vec_normalize(&vec_to_enemy);
		dot_to_enemy = vm_vec_dot(&Pl_objp->orient.vec.fvec, &vec_to_enemy);
		if (dot_to_enemy > 0.75f) {
			//	Used to go to En_objp's right vector, but due to banking while turning, that
			//	caused flying in an odd spiral.
			vm_vec_scale_add(&goal_point, &enemy_pos, &Pl_objp->orient.vec.rvec, 1000.0f);
			if (dist < 100.0f)
				bank_override = Pl_objp->phys_info.speed; 
		} else {
			bank_override = Pl_objp->phys_info.speed;			//	In enemy's sights, not pointing at him, twirl away.
			goto evade_ship_l1;
		}
	} else {
evade_ship_l1: ;
		if (aip->ai_evasion > myrand()*100.0f/32767.0f) {
			int	temp;
			float	scale;
			float	psrandval;	//	some value close to zero to choose whether to turn right or left.

			psrandval = (float) (((Missiontime >> 14) & 0x0f) - 8);	//	Value between -8 and 7
			psrandval = psrandval/16.0f;							//	Value between -1/2 and 1/2 (approx)

			//	If not close to behind, turn towards his right or left vector, whichever won't cross his path.
			if (vm_vec_dot(&vec_from_enemy, &En_objp->orient.vec.rvec) > psrandval) {
				scale = 1000.0f;
			} else {
				scale = -1000.0f;
			}

			vm_vec_scale_add(&goal_point, &enemy_pos, &En_objp->orient.vec.rvec, scale);

			temp = ((Missiontime >> 16) & 0x07);
			temp = ((temp * (temp+1)) % 16)/2 - 4;
			if ((psrandval == 0) && (temp == 0))
				temp = 3;

			scale = 200.0f * temp;

			vm_vec_scale_add2(&goal_point, &En_objp->orient.vec.uvec, scale);
		} else {
			//	No evasion this frame, but continue with previous turn.
			//	Reason: If you don't, you lose rotational momentum.  Turning every other frame,
			//	and not in between results in a very slow turn because of loss of momentum.
			if ((aip->prev_goal_point.xyz.x != 0.0f) || (aip->prev_goal_point.xyz.y != 0.0f) || (aip->prev_goal_point.xyz.z != 0.0f))
				goal_point = aip->prev_goal_point;
			else
				vm_vec_scale_add(&goal_point, &enemy_pos, &En_objp->orient.vec.rvec, 100.0f);
		}
	}

	turn_towards_point(Pl_objp, &goal_point, NULL, bank_override);

	aip->prev_goal_point = goal_point;
}

//	--------------------------------------------------------------------------
//	Fly in a manner making it difficult for opponent to attack.
void ai_evade()
{
	evade_ship();
}


float	G_collision_time;
vec3d	G_predicted_pos, G_fire_pos;


//old version of this fuction, this will be useful for playing old missions and not having the new primary
//selection code throw off the balance of the mission.
//	If:
//		flags & WIF_PUNCTURE
//	Then Select a Puncture weapon.
//	Else
//		Select Any ol' weapon.
//	Returns primary_bank index.
int ai_select_primary_weapon_OLD(object *objp, object *other_objp, int flags)
{
	ship	*shipp = &Ships[objp->instance];
	ship_weapon *swp = &shipp->weapons;
	ship_info *sip;

	Assert( shipp->ship_info_index >= 0 && shipp->ship_info_index < Num_ship_classes);

	sip = &Ship_info[shipp->ship_info_index];

	if (flags & WIF_PUNCTURE) {
		if (swp->current_primary_bank >= 0) {
			int	bank_index;

			bank_index = swp->current_primary_bank;

			if (Weapon_info[swp->primary_bank_weapons[bank_index]].wi_flags & WIF_PUNCTURE) {
				return swp->current_primary_bank;
			}
		}
		for (int i=0; i<swp->num_primary_banks; i++) {
			int	weapon_info_index;

			weapon_info_index = swp->primary_bank_weapons[i];

			if (weapon_info_index > -1){
				if (Weapon_info[weapon_info_index].wi_flags & WIF_PUNCTURE) {
					swp->current_primary_bank = i;
					return i;
				}
			}
		}
		
		// AL 26-3-98: If we couldn't find a puncture weapon, pick first available weapon if one isn't active
		if ( swp->current_primary_bank < 0 ) {
			if ( swp->num_primary_banks > 0 ) {
				swp->current_primary_bank = 0;
			}
		}

	} else {		//	Don't need to be using a puncture weapon.
		if (swp->current_primary_bank >= 0) {
			if (!(Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]].wi_flags & WIF_PUNCTURE)){
				return swp->current_primary_bank;
			}
		}
		for (int i=0; i<swp->num_primary_banks; i++) {
			if (swp->primary_bank_weapons[i] > -1) {
				if (!(Weapon_info[swp->primary_bank_weapons[i]].wi_flags & WIF_PUNCTURE)) {
					swp->current_primary_bank = i;
					nprintf(("AI", "%i: Ship %s selecting weapon %s\n", Framecount, Ships[objp->instance].ship_name, Weapon_info[swp->primary_bank_weapons[i]].name));
					return i;
				}
			}
		}
		//	Wasn't able to find a non-puncture weapon.  Stick with what we have.
	}

	Assert( swp->current_primary_bank != -1 );		// get Alan or Allender

	return swp->current_primary_bank;
}

//	If:
//		flags & WIF_PUNCTURE
//	Then Select a Puncture weapon.
//	Else
//		Select Any ol' weapon.
//	Returns primary_bank index.
/**
 * Etc. Etc. This is like the 4th rewrite of the code here. Special thanks to Bobboau
 * for finding the get_shield_strength function.
 * 
 * The AI will now intelligently choose the best weapon to use based on the overall shield
 * status of the target.
 */
int ai_select_primary_weapon(object *objp, object *other_objp, int flags)
{
	// Pointer Set Up
	ship	*shipp = &Ships[objp->instance];
	ship_weapon *swp = &shipp->weapons;
	ship_info *sip;

	// Debugging
	if (other_objp==NULL)
	{
		// this can be NULL in the case of a target death and attacker weapon
		// change.  using notification message instead of a fault
		mprintf(("'other_objpp == NULL' in ai_select_primary_weapon()\n"));
		return -1;
	}

	//not using the new AI, use the old version of this function instead.
	if (!(Ai_info[shipp->ai_index].ai_profile_flags & AIPF_SMART_PRIMARY_WEAPON_SELECTION))
	{
		return ai_select_primary_weapon_OLD(objp, other_objp, flags);
	}

	Assert( shipp->ship_info_index >= 0 && shipp->ship_info_index < Num_ship_classes);
	
	sip = &Ship_info[shipp->ship_info_index];
	//made it so it only selects puncture weapons if the active goal is to disable something -Bobboau
	if ((flags & WIF_PUNCTURE) && (Ai_info[shipp->ai_index].goals[0].ai_mode & (AI_GOAL_DISARM_SHIP | AI_GOAL_DISABLE_SHIP))) 
	{
		if (swp->current_primary_bank >= 0) 
		{
			int	bank_index;

			bank_index = swp->current_primary_bank;

			if (Weapon_info[swp->primary_bank_weapons[bank_index]].wi_flags & WIF_PUNCTURE) 
			{
				return swp->current_primary_bank;
			}
		}
		for (int i=0; i<swp->num_primary_banks; i++) 
		{
			int	weapon_info_index;

			weapon_info_index = swp->primary_bank_weapons[i];

			if (weapon_info_index > -1)
			{
				if (Weapon_info[weapon_info_index].wi_flags & WIF_PUNCTURE) 
				{
					swp->current_primary_bank = i;
					return i;
				}
			}
		}
	}

	if ( (other_objp->type == OBJ_SHIP) && (Ship_info[Ships[other_objp->instance].ship_info_index].flags & (SIF_BIG_SHIP|SIF_HUGE_SHIP) ) ) 
	{
		//Check if we have a capital+ weapon on board
		for (int i = 0; i < swp->num_primary_banks; i++)
		{
			if (swp->primary_bank_weapons[i] > -1)		// Make sure there is a weapon in the bank
			{
				if (Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & WIF2_CAPITAL_PLUS)
				{
					swp->current_primary_bank = i;
					nprintf(("AI", "%i: Ship %s selecting weapon %s\n", Framecount, Ships[objp->instance].ship_name, Weapon_info[swp->primary_bank_weapons[i]].name));
					return i;
				}
			}
		}
	}

	float enemy_remaining_shield = get_shield_pct(other_objp);

	// Is the target shielded by say only 5%?
	if (enemy_remaining_shield <= 0.05f)	
	{
		// Then it is safe to start using a heavy hull damage weapon such as the maxim
		int i;
		float i_hullfactor_prev = 0;		// Previous weapon bank hull factor (this is the safe way to do it)
		int i_hullfactor_prev_bank = -1;	// Bank which gave us this hull factor

		// Find the weapon with the highest hull * damage / fire delay factor
		for (i = 0; i < swp->num_primary_banks; i++)
		{
			if (swp->primary_bank_weapons[i] > -1)		// Make sure there is a weapon in the bank
			{
				if ((((Weapon_info[swp->primary_bank_weapons[i]].armor_factor) * (Weapon_info[swp->primary_bank_weapons[i]].damage)) / Weapon_info[swp->primary_bank_weapons[i]].fire_wait) > i_hullfactor_prev)
				{
					if ( !(Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & WIF2_CAPITAL_PLUS) )
					{
						// This weapon is the new candidate
						i_hullfactor_prev = ( ((Weapon_info[swp->primary_bank_weapons[i]].armor_factor) * (Weapon_info[swp->primary_bank_weapons[i]].damage)) / Weapon_info[swp->primary_bank_weapons[i]].fire_wait );
						i_hullfactor_prev_bank = i;
					}
				}
			}
		}
		if (i_hullfactor_prev_bank == -1)		// In the unlikely instance we don't find at least 1 candidate weapon
		{
			i_hullfactor_prev_bank = 0;		// Just switch to the first one
		}
		swp->current_primary_bank = i_hullfactor_prev_bank;		// Select the best weapon
		return i_hullfactor_prev_bank;							// Return
	}

	//if the shields are above lets say 10% definitely use a pierceing weapon if there are any-Bobboau
	if (enemy_remaining_shield >= 0.10f)
	{
		if (swp->current_primary_bank >= 0) 
		{
			int	bank_index;

			bank_index = swp->current_primary_bank;

			if ((Weapon_info[swp->primary_bank_weapons[bank_index]].wi_flags2 & WIF2_PIERCE_SHIELDS) && !(Weapon_info[swp->primary_bank_weapons[bank_index]].wi_flags2 & WIF2_CAPITAL_PLUS))
			{
				return swp->current_primary_bank;
			}
		}
		for (int i=0; i<swp->num_primary_banks; i++) 
		{
			int	weapon_info_index;

			weapon_info_index = swp->primary_bank_weapons[i];

			if (weapon_info_index > -1)
			{
				if ((Weapon_info[weapon_info_index].wi_flags2 & WIF2_PIERCE_SHIELDS) && !(Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & WIF2_CAPITAL_PLUS)) 
				{
					swp->current_primary_bank = i;
					return i;
				}
			}
		}
	}

	// Is the target shielded by less then 50%?
	if (enemy_remaining_shield <= 0.50f)	
	{
		// Should be using best balanced shield gun
		int i;
		float i_hullfactor_prev = 0;		// Previous weapon bank hull factor (this is the safe way to do it)
		int i_hullfactor_prev_bank = -1;	// Bank which gave us this hull factor
		// Find the weapon with the highest average hull and shield * damage / fire delay factor
		for (i = 0; i < swp->num_primary_banks; i++)
		{
			if (swp->primary_bank_weapons[i] > -1)		// Make sure there is a weapon in the bank
			{
				if ((((Weapon_info[swp->primary_bank_weapons[i]].armor_factor + Weapon_info[swp->primary_bank_weapons[i]].shield_factor) * Weapon_info[swp->primary_bank_weapons[i]].damage) / Weapon_info[swp->primary_bank_weapons[i]].fire_wait) > i_hullfactor_prev)
				{
					if ( !(Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & WIF2_CAPITAL_PLUS) )
					{
						// This weapon is the new candidate
						i_hullfactor_prev = ((((Weapon_info[swp->primary_bank_weapons[i]].armor_factor + Weapon_info[swp->primary_bank_weapons[i]].shield_factor) * Weapon_info[swp->primary_bank_weapons[i]].damage) / Weapon_info[swp->primary_bank_weapons[i]].fire_wait));
						i_hullfactor_prev_bank = i;
					}
				}
			}
		}
		if (i_hullfactor_prev_bank == -1)		// In the unlikely instance we don't find at least 1 candidate weapon
		{
			i_hullfactor_prev_bank = 0;		// Just switch to the first one
		}
		swp->current_primary_bank = i_hullfactor_prev_bank;		// Select the best weapon
		return i_hullfactor_prev_bank;							// Return
	}
	else
	{
		// Should be using best shield destroying gun
		int i;
		float i_hullfactor_prev = 0;		// Previous weapon bank hull factor (this is the safe way to do it)
		int i_hullfactor_prev_bank = -1;	// Bank which gave us this hull factor
		// Find the weapon with the highest average hull and shield * damage / fire delay factor
		for (i = 0; i < swp->num_primary_banks; i++)
		{
			if (swp->primary_bank_weapons[i] > -1)		// Make sure there is a weapon in the bank
			{
				if ((((Weapon_info[swp->primary_bank_weapons[i]].shield_factor) * Weapon_info[swp->primary_bank_weapons[i]].damage) / Weapon_info[swp->primary_bank_weapons[i]].fire_wait) > i_hullfactor_prev)
				{
					if ( !(Weapon_info[swp->primary_bank_weapons[i]].wi_flags2 & WIF2_CAPITAL_PLUS) )
					{
						// This weapon is the new candidate
						i_hullfactor_prev = ( ((Weapon_info[swp->primary_bank_weapons[i]].shield_factor) * (Weapon_info[swp->primary_bank_weapons[i]].damage)) / Weapon_info[swp->primary_bank_weapons[i]].fire_wait );
						i_hullfactor_prev_bank = i;
					}
				}
			}
		}
		if (i_hullfactor_prev_bank == -1)		// In the unlikely instance we don't find at least 1 candidate weapon
		{
			i_hullfactor_prev_bank = 0;		// Just switch to the first one
		}
		swp->current_primary_bank = i_hullfactor_prev_bank;		// Select the best weapon
		return i_hullfactor_prev_bank;							// Return
	}
}

/**
 * Maybe link primary weapons.
 */
void set_primary_weapon_linkage(object *objp)
{
	ship		*shipp;
	ship_info *sip;
	ai_info	*aip;
	ship_weapon	*swp;
	weapon_info *wip;
	
	int total_ammo;
	int current_ammo;
	float ammo_pct;
	int i;

	shipp = &Ships[objp->instance];
	sip = &Ship_info[shipp->ship_info_index];
	aip	= &Ai_info[shipp->ai_index];
	swp = &shipp->weapons;

	shipp->flags &= ~SF_PRIMARY_LINKED;

	// AL: ensure target is a ship!
	if ( (aip->target_objnum != -1) && (Objects[aip->target_objnum].type == OBJ_SHIP) ) {
		// If trying to destroy a big ship (i.e., not disable/disarm), always unleash all weapons
		if ( ship_get_SIF(&Ships[Objects[aip->target_objnum].instance]) & SIF_BIG_SHIP) {
			if ( aip->targeted_subsys == NULL ) {
				shipp->flags |= SF_PRIMARY_LINKED;
				shipp->flags |= SF_SECONDARY_DUAL_FIRE;
				return;
			}
		}
	}

	if (Num_weapons > (int) (MAX_WEAPONS * 0.75f) || sip->flags2 & SIF2_NO_PRIMARY_LINKING) {
		if (shipp->flags & SF_PRIMARY_LINKED)
			nprintf(("AI", "Frame %i, ship %s: Unlinking primaries.\n", Framecount, shipp->ship_name));
		shipp->flags &= ~SF_PRIMARY_LINKED;
		return;		//	If low on slots or primary linking disallowed, don't link.
	}

	// AL 2-11-98: If ship has a disarm or disable goal, don't link unless both weapons are
	//					puncture weapons
	if ( (aip->active_goal != AI_GOAL_NONE) && (aip->active_goal != AI_ACTIVE_GOAL_DYNAMIC) )
	{
		if ( aip->goals[aip->active_goal].ai_mode & (AI_GOAL_DISABLE_SHIP|AI_GOAL_DISARM_SHIP) )
		{
			// only continue if both primaries are puncture weapons
			if ( swp->num_primary_banks == 2 ) {
				if ( !(Weapon_info[swp->primary_bank_weapons[0]].wi_flags & WIF_PUNCTURE) ) 
					return;
				if ( !(Weapon_info[swp->primary_bank_weapons[1]].wi_flags & WIF_PUNCTURE) ) 
					return;
			}
		}
	}

	//	Don't want all ships always linking weapons at start, so asynchronize.
	if (The_mission.ai_profile->flags2 & AIPF2_ALLOW_PRIMARY_LINK_DELAY)
	{
		if (Missiontime < i2f(30))
			return;
		else if (Missiontime < i2f(120))
		{
			int r = static_rand((Missiontime >> 17) ^ OBJ_INDEX(objp));
			if ( (r&3) != 0)
				return;
		}
	}

	// get energy level
	float energy;
	if (The_mission.ai_profile->flags & AIPF_FIX_LINKED_PRIMARY_BUG) {
		energy = shipp->weapon_energy / sip->max_weapon_reserve * 100.0f;
	} else {
		energy = shipp->weapon_energy;
	}

	// make linking decision based on weapon energy
	if (energy > aip->ai_link_energy_levels_always) {
		shipp->flags |= SF_PRIMARY_LINKED;
	} else if (energy > aip->ai_link_ammo_levels_maybe) {
		if (objp->hull_strength < shipp->ship_max_hull_strength/3.0f) {
			shipp->flags |= SF_PRIMARY_LINKED;
		}
	}

	// also check ballistics - Goober5000
	if (sip->flags & SIF_BALLISTIC_PRIMARIES)
	{
		total_ammo = 0;
		current_ammo = 0;

		// count ammo, and do not continue unless all weapons are ballistic
		for (i = 0; i < swp->num_primary_banks; i++)
		{
			wip = &Weapon_info[swp->primary_bank_weapons[i]];

			if (wip->wi_flags2 & WIF2_BALLISTIC)
			{
				total_ammo += swp->primary_bank_start_ammo[i];
				current_ammo += swp->primary_bank_ammo[i];
			}
			else
			{
				return;
			}
		}

		Assert(total_ammo);	// Goober5000: div-0 check
		ammo_pct = float (current_ammo) / float (total_ammo) * 100.0f;

		// link according to defined levels
		if (ammo_pct > aip->ai_link_ammo_levels_always)
		{
			shipp->flags |= SF_PRIMARY_LINKED;
		}
		else if (ammo_pct > aip->ai_link_ammo_levels_maybe)
		{
			if (objp->hull_strength < shipp->ship_max_hull_strength/3.0f)
			{
				shipp->flags |= SF_PRIMARY_LINKED;
			}
		}
	}
}

//	--------------------------------------------------------------------------
//	Fire the current primary weapon.
//	*objp is the object to fire from.
//I changed this to return a true false flag on weather 
//it did or did not fire the weapon sorry if this screws you up-Bobboau
int ai_fire_primary_weapon(object *objp)
{
	ship		*shipp = &Ships[objp->instance];
	ship_weapon	*swp = &shipp->weapons;
	ship_info	*sip, *enemy_sip;
	ai_info		*aip;
	object		*enemy_objp;

	Assert( shipp->ship_info_index >= 0 && shipp->ship_info_index < Num_ship_classes);
	sip = &Ship_info[shipp->ship_info_index];

	aip = &Ai_info[shipp->ai_index];

	//	If low on slots, fire a little less often.
	if (Num_weapons > (int) (0.9f * MAX_WEAPONS)) {
		if (frand() > 0.5f) {
			nprintf(("AI", "Frame %i, %s not fire.\n", Framecount, shipp->ship_name));
			return 0;
		}
	}

	if (!Ai_firing_enabled){
		return 0;
	}

	if (aip->target_objnum != -1){
		enemy_objp = &Objects[aip->target_objnum];
		enemy_sip = (enemy_objp->type == OBJ_SHIP) ? &Ship_info[Ships[enemy_objp->instance].ship_info_index] : NULL;
	} else {
		enemy_objp = NULL;
		enemy_sip = NULL;
	}

	if ( (swp->current_primary_bank < 0) || (swp->current_primary_bank >= swp->num_primary_banks) || timestamp_elapsed(aip->primary_select_timestamp)) {
		int	flags = 0;
		if ( aip->targeted_subsys != NULL ) {
			flags = WIF_PUNCTURE;
		}
		ai_select_primary_weapon(objp, enemy_objp, flags);
		ship_primary_changed(shipp);	// AL: maybe send multiplayer information when AI ship changes primaries
		aip->primary_select_timestamp = timestamp(5 * 1000);	//	Maybe change primary weapon five seconds from now.
	}

	//	If pointing nearly at predicted collision point of target, bash orientation to be perfectly pointing.
	float	dot;
	vec3d	v2t;

	if (!( vm_vec_mag_quick(&G_predicted_pos) < AICODE_SMALL_MAGNITUDE )) {
		if ( vm_vec_cmp(&G_predicted_pos, &G_fire_pos) ) {
			vm_vec_normalized_dir(&v2t, &G_predicted_pos, &G_fire_pos);
			dot = vm_vec_dot(&v2t, &objp->orient.vec.fvec);
			if (dot > .998629534f){	//	if within 3.0 degrees of desired heading, bash
				vm_vector_2_matrix(&objp->orient, &v2t, &objp->orient.vec.uvec, NULL);
			}
		}
	}

	//SUSHI: Burst-fire for ballistic primaries.
	if (The_mission.ai_profile->primary_ammo_burst_mult[Game_skill_level] > 0 &&						//Make sure we are using burst fire
		enemy_objp != NULL && enemy_sip != NULL	&& 														//We need a target, obviously
		(enemy_objp->phys_info.speed >= 1.0f) &&														//Only burst for moving ships
		(enemy_sip->flags & (SIF_SMALL_SHIP | SIF_TRANSPORT)) && 										//Only burst for small ships (transports count)
		swp->primary_bank_start_ammo[swp->current_primary_bank] > 0 &&									//Prevent div by 0
		Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]].wi_flags2 & WIF2_BALLISTIC)	//Current weapon must be ballistic
	{
		float percentAmmoLeft = ((float)swp->primary_bank_ammo[swp->current_primary_bank] / (float)swp->primary_bank_start_ammo[swp->current_primary_bank]);
		float distToTarget = vm_vec_dist(&enemy_objp->pos, &objp->pos);
		float weaponRange = Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]].weapon_range;
		float distanceFactor = 1.0f - distToTarget/weaponRange;
		vec3d vecToTarget;
		vm_vec_normalized_dir(&vecToTarget, &enemy_objp->pos, &objp->pos);
		float dotToTarget = vm_vec_dot(&vecToTarget, &objp->orient.vec.fvec);
		dotToTarget = pow(dotToTarget, 4);	//This makes the dot a tiny bit more impactful (otherwise nearly always over 0.98 or so)
		
		//Combine factors
		float burstFireProb = ((0.6f * percentAmmoLeft) + (0.4f * distanceFactor)) * dotToTarget * aip->ai_primary_ammo_burst_mult;

		//Possibly change values every half-second
		if (static_randf((Missiontime + static_rand(aip->shipnum)) >> 15) > burstFireProb)
			return 0;
	}

	//	Make sure not firing at a protected ship unless firing at a live subsystem.
	//	Note: This happens every time the ship tries to fire, perhaps every frame.
	//	Should be wrapped in a timestamp, same one that enables it to fire, but that is complicated
	//	by multiple banks it can fire from.
	if (aip->target_objnum != -1) {
		object	*tobjp = &Objects[aip->target_objnum];
		if (tobjp->flags & OF_PROTECTED) {
			if (aip->targeted_subsys != NULL) {
				int	type;

				type = aip->targeted_subsys->system_info->type;
				if (ship_get_subsystem_strength(&Ships[tobjp->instance], type) == 0.0f) {
					aip->target_objnum = -1;
					return 0;
				}
			} else {
				aip->target_objnum = -1;
				return 0;
			}
		}
	}

	//	If enemy is protected, not firing a puncture weapon and enemy's hull is low, don't fire.
	if ((enemy_objp != NULL) && (enemy_objp->flags & OF_PROTECTED)) {
		// AL: 3-6-98: Check if current_primary_bank is valid
		if ((enemy_objp->hull_strength < 750.0f) && 
			((aip->targeted_subsys == NULL) || (enemy_objp->hull_strength < aip->targeted_subsys->current_hits + 50.0f)) &&
			(swp->current_primary_bank >= 0) ) {
			if (!(Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]].wi_flags & WIF_PUNCTURE)) {
				swp->next_primary_fire_stamp[swp->current_primary_bank] = timestamp(1000);
				return 0;
			}
		}
	}

	set_primary_weapon_linkage(objp);
	
	// I think this will properly solve the problem
	// fire non-streaming weapons
	ship_fire_primary(objp, 0);
	
	// fire streaming weapons
	shipp->flags |= SF_TRIGGER_DOWN;
	ship_fire_primary(objp, 1);
	shipp->flags &= ~SF_TRIGGER_DOWN;
	return 1;//if it got down to here then it tryed to fire
}

//	--------------------------------------------------------------------------
//	Return number of nearby enemy fighters.
//	threshold is the distance within which a ship is considered near.
//
// input:	enemy_team_mask	=>	teams that are considered as an enemy
//				pos					=>	world position to measure ship distances from
//				threshold			=>	max distance from pos to be considered "near"
//
// exit:		number of ships within threshold units of pos
int num_nearby_fighters(int enemy_team_mask, vec3d *pos, float threshold)
{
	ship_obj	*so;
	object	*ship_objp;
	int		count = 0;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {

		ship_objp = &Objects[so->objnum];

		if (iff_matches_mask(Ships[ship_objp->instance].team, enemy_team_mask)) {
			if (Ship_info[Ships[ship_objp->instance].ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER)) {
				if (vm_vec_dist_quick(pos, &ship_objp->pos) < threshold)
					count++;
			}
		}
	}

	return count;
}

//	--------------------------------------------------------------------------
//	Select secondary weapon to fire.
//	Currently, 1/16/98:
//		If 0 secondary weapons available, return -1
//		If 1 available, use it.
//		If 2 or more, if the current weapon is one of them, stick with it, otherwise choose a random one.
//	priority1 and priority2 are Weapon_info[] bitmasks such as WIF_HOMING_ASPECT.  If any weapon has any bit in priority1
//	set, that weapon will be selected.  If not, apply to priority2.  If neither, return -1, meaning no weapon selected.
//	Note, priorityX have default values of -1, meaning if not set, they will match any weapon.
//	Return value:
//		bank index
//	Should do this:
//		Favor aspect seekers when attacking small ships faraway.
//		Favor rapid fire dumbfire when attacking a large ship.
//		Ignore heat seekers because we're not sure how they'll work.
void ai_select_secondary_weapon(object *objp, ship_weapon *swp, int priority1 = -1, int priority2 = -1, int wif2_priority1 = -1, int wif2_priority2 = -1)
{
	int	num_weapon_types;
	int	weapon_id_list[MAX_WEAPON_TYPES], weapon_bank_list[MAX_WEAPON_TYPES];
	int	i;
	int	ignore_mask, ignore_mask_without_huge;
	int	initial_bank;
	ai_info	*aip = &Ai_info[Ships[objp->instance].ai_index];

	initial_bank = swp->current_secondary_bank;

	// set up ignore masks
	ignore_mask = 0;
	ignore_mask_without_huge = 0;

	// Ignore bombs unless one of the priorities asks for them to be selected.
	if (!(WIF_HUGE & (priority1 | priority2))) {
		ignore_mask |= WIF_HUGE;
	}

	// Ignore capital+ unless one of the priorities asks for it.
	if (!(WIF2_CAPITAL_PLUS & (wif2_priority1 | wif2_priority2))) {
		ignore_mask |= WIF2_CAPITAL_PLUS;
	}

	// Ignore bomber+ unless one of the priorities asks for them to be selected
	if (!(WIF_BOMBER_PLUS & (priority1 | priority2))) {
		ignore_mask |= WIF_BOMBER_PLUS;
		ignore_mask_without_huge |= WIF_BOMBER_PLUS;
	}

#ifndef NDEBUG
	for (i=0; i<MAX_WEAPON_TYPES; i++) {
		weapon_id_list[i] = -1;
		weapon_bank_list[i] = -1;
	}
#endif

	//	Stuff weapon_bank_list with bank index of available weapons.
	num_weapon_types = get_available_secondary_weapons(objp, weapon_id_list, weapon_bank_list);

	// Ignore homing weapons if we didn't specify a flag - for priority 1
	if ((aip->ai_profile_flags & AIPF_SMART_SECONDARY_WEAPON_SELECTION) && (priority1 == 0)) {
		ignore_mask |= WIF_HOMING;
		ignore_mask_without_huge |= WIF_HOMING;
	}

	int	priority2_index = -1;

	for (i=0; i<num_weapon_types; i++) {
		int wi_flags = Weapon_info[swp->secondary_bank_weapons[weapon_bank_list[i]]].wi_flags;
		int wi_flags2 = Weapon_info[swp->secondary_bank_weapons[weapon_bank_list[i]]].wi_flags2;
		int ignore_mask_to_use = ((aip->ai_profile_flags & AIPF_SMART_SECONDARY_WEAPON_SELECTION) && (wi_flags & WIF_BOMBER_PLUS)) ? ignore_mask_without_huge : ignore_mask;

		if (!(wi_flags & ignore_mask_to_use)) {					//	Maybe bombs are illegal.
			if (wi_flags & priority1) {
				swp->current_secondary_bank = weapon_bank_list[i];				//	Found first priority, return it.
				break;
			} else if (wi_flags & priority2)
				priority2_index = weapon_bank_list[i];	//	Found second priority, but might still find first priority.
		}
		if (!(wi_flags2 & ignore_mask_to_use)) {					//	Maybe bombs are illegal.
			if (wi_flags2 & wif2_priority1) {
				swp->current_secondary_bank = weapon_bank_list[i];				//	Found first priority, return it.
				break;
			} else if (wi_flags2 & wif2_priority2)
				priority2_index = weapon_bank_list[i];	//	Found second priority, but might still find first priority.
		}
	}

	// Ignore homing weapons if we didn't specify a flag - for priority 2
	if ((aip->ai_profile_flags & AIPF_SMART_SECONDARY_WEAPON_SELECTION) && (priority2 == 0)) {
		ignore_mask |= WIF_HOMING;
		ignore_mask_without_huge |= WIF_HOMING;
	}

	//	If didn't find anything above, then pick any secondary weapon.
	if (i == num_weapon_types) {
		swp->current_secondary_bank = priority2_index;	//	Assume we won't find anything.
		if (priority2_index == -1) {
			for (i=0; i<num_weapon_types; i++) {
				int wi_flags = Weapon_info[swp->secondary_bank_weapons[weapon_bank_list[i]]].wi_flags;
				int ignore_mask_to_use = ((aip->ai_profile_flags & AIPF_SMART_SECONDARY_WEAPON_SELECTION) && (wi_flags & WIF_BOMBER_PLUS)) ? ignore_mask_without_huge : ignore_mask;

				if (!(wi_flags & ignore_mask_to_use)) {					//	Maybe bombs are illegal.
					if (swp->secondary_bank_ammo[i] > 0) {
						swp->current_secondary_bank = i;
						break;
					}
				}
			}
		}
	}


	//	If switched banks, force reacquisition of aspect lock.
	if (swp->current_secondary_bank != initial_bank) {
		aip->aspect_locked_time = 0.0f;
		aip->current_target_is_locked = 0;
	}

	if (swp->current_secondary_bank >= 0 && swp->current_secondary_bank < MAX_SHIP_SECONDARY_BANKS) 
	{
		weapon_info *wip=&Weapon_info[swp->secondary_bank_weapons[swp->current_secondary_bank]];
	
		// phreak -- rapid dumbfire? let it rip!
		if ((aip->ai_profile_flags & AIPF_ALLOW_RAPID_SECONDARY_DUMBFIRE) && !(wip->wi_flags & WIF_HOMING) && (wip->fire_wait < .5f))
		{	
			aip->ai_flags |= AIF_UNLOAD_SECONDARIES;
		}
	}

	ship_secondary_changed(&Ships[objp->instance]);	// AL: let multiplayer know if secondary bank has changed
}

/**
 * @return number of objects homing on object *target_objp
 */
int compute_num_homing_objects(object *target_objp)
{
	object	*objp;
	int		count = 0;

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if (objp->type == OBJ_WEAPON) {
			if (Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_HOMING) {
				if (Weapons[objp->instance].homing_object == target_objp) {
					count++;
				}
			}
		}
	}

	return count;
}

//	Object *firing_objp just fired weapon weapon_index (index in Weapon_info).
//	If it's a shockwave weapon, tell your team about it!
void ai_maybe_announce_shockwave_weapon(object *firing_objp, int weapon_index)
{
	if ((firing_objp->type == OBJ_SHIP) && (Weapon_info[weapon_index].shockwave.speed > 0.0f)) {
		ship_obj	*so;
		int		firing_ship_team;

		firing_ship_team = Ships[firing_objp->instance].team;

		for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
			object	*A = &Objects[so->objnum];
			Assert(A->type == OBJ_SHIP);

			if (Ships[A->instance].team == firing_ship_team) {
				ai_info	*aip = &Ai_info[Ships[A->instance].ai_index];

				// AL 1-5-98: only avoid shockwave if not docked or repairing
				if ( !object_is_docked(A) && !(aip->ai_flags & (AIF_REPAIRING|AIF_BEING_REPAIRED)) ) {
					aip->ai_flags |= AIF_AVOID_SHOCKWAVE_WEAPON;
				}
			}
		}
	}
}

/**
 * Return total payload of all incoming missiles.
 */
float compute_incoming_payload(object *target_objp)
{
	missile_obj	*mo;
	float			payload = 0.0f;

	for ( mo = GET_NEXT(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
		object	*objp;

		objp = &Objects[mo->objnum];
		Assert(objp->type == OBJ_WEAPON);
		if (Weapons[objp->instance].homing_object == target_objp) {
			payload += Weapon_info[Weapons[objp->instance].weapon_info_index].damage;
		}
	}

	return payload;
}

//	--------------------------------------------------------------------------
//	Return true if OK for *aip to fire its current weapon at its current target.
//	Only reason this function returns false is:
//		weapon is a homer
//		targeted at player
//			OR:	player has too many homers targeted at him
//					Missiontime in that dead zone in which can't fire at this player
//	Note: If player is attacking a ship, that ship is allowed to fire at player.  Otherwise, we get in a situation in which
//	player is attacking a large ship, but that large ship is not defending itself with missiles.
int check_ok_to_fire(int objnum, int target_objnum, weapon_info *wip)
{
	int	num_homers = 0;

	if (target_objnum >= 0) {
		object	*tobjp = &Objects[target_objnum];

		// AL 3-4-98: Ensure objp target is a ship first 
		if ( tobjp->type == OBJ_SHIP ) {
			if (Ship_info[Ships[tobjp->instance].ship_info_index].flags & SIF_SMALL_SHIP) {
				num_homers = compute_num_homing_objects(&Objects[target_objnum]);
			}
		}

		//	If player, maybe fire based on Skill_level and number of incoming weapons.
		//	If non-player, maybe fire based on payload of incoming weapons.
		if (wip->wi_flags & WIF_HOMING) {
			if ((target_objnum > -1) && (tobjp->flags & OF_PLAYER_SHIP)) {
				if (Ai_info[Ships[tobjp->instance].ai_index].target_objnum != objnum) {
					//	Don't allow AI ships to fire at player for fixed periods of time based on skill level.
					//	With 5 skill levels, at Very Easy, they fire in 1/7 of every 10 second interval.
					//	At Easy, 2/7...at Expert, 5/7
					int t = ((Missiontime /(65536*10)) ^ target_objnum ^ 0x01) % (NUM_SKILL_LEVELS+2);
					if (t > Ai_info[Ships[tobjp->instance].ai_index].ai_chance_to_use_missiles_on_plr) {
						return 0;
					}
				}
				int	swarmers = 0;
				if (wip->wi_flags & WIF_SWARM)
					swarmers = 2;	//	Note, always want to be able to fire swarmers if no currently incident homers.
				if (The_mission.ai_profile->max_allowed_player_homers[Game_skill_level] < num_homers + swarmers) {
					return 0;
				}
			} else if (num_homers > 3) {
				float	incoming_payload;

				incoming_payload = compute_incoming_payload(&Objects[target_objnum]);

				if (incoming_payload > tobjp->hull_strength) {
					return 0;
				}
			}
		}
	}

	return 1;
}

//	--------------------------------------------------------------------------
//	Fire a secondary weapon.
//	Maybe choose to fire a different one.
//	priority1 and priority2 are optional parameters with defaults = -1
int ai_fire_secondary_weapon(object *objp, int priority1, int priority2)
{
	ship_weapon *swp;
	ship	*shipp;
	ship_info *sip;
	int		current_bank;
	int		rval = 0;

#ifndef NDEBUG
	if (!Ai_firing_enabled)
		return rval;
#endif

	Assert( objp != NULL );
	Assert(objp->type == OBJ_SHIP);
	shipp = &Ships[objp->instance];
	swp = &shipp->weapons;

	Assert( shipp->ship_info_index >= 0 && shipp->ship_info_index < Num_ship_classes);
	sip = &Ship_info[shipp->ship_info_index];

	//	Select secondary weapon.
	current_bank = swp->current_secondary_bank; //ai_select_secondary_weapon(objp, swp, priority1, priority2);

	if (current_bank == -1) {
		return rval;
	}

	Assert(current_bank < shipp->weapons.num_secondary_banks);

	weapon_info	*wip = &Weapon_info[shipp->weapons.secondary_bank_weapons[current_bank]];

	if ((wip->wi_flags & WIF_LOCKED_HOMING) && (!Ai_info[shipp->ai_index].current_target_is_locked)) {
		swp->next_secondary_fire_stamp[current_bank] = timestamp(250);
	} else if ((wip->wi_flags & WIF_BOMB) || (vm_vec_dist_quick(&objp->pos, &En_objp->pos) > 50.0f)) {
		//	This might look dumb, firing a bomb even if closer than 50 meters, but the reason is, if you're carrying
		//	bombs, delivering them is probably more important than surviving.
		ai_info	*aip;

		aip = &Ai_info[shipp->ai_index];
		
		//	Note, maybe don't fire if firing at player and any homers yet fired.
		//	Decreasing chance to fire the more homers are incoming on player.
		if (check_ok_to_fire(OBJ_INDEX(objp), aip->target_objnum, wip)) {
			if (ship_fire_secondary(objp)) {
				rval = 1;
				swp->next_secondary_fire_stamp[current_bank] = timestamp(500);
			}

		} else {
			swp->next_secondary_fire_stamp[current_bank] = timestamp(500);
		}
	}

	return rval;
}

/**
 * Return true if it looks like obj1, if continuing to move along current vector, will collide with obj2.
 */
int might_collide_with_ship(object *obj1, object *obj2, float dot_to_enemy, float dist_to_enemy, float duration)
{
	if (obj1->phys_info.speed * duration + 2*(obj1->radius + obj2->radius) > dist_to_enemy)
		if (dot_to_enemy > 0.8f - 2*(obj1->radius + obj2->radius)/dist_to_enemy)
			return objects_will_collide(obj1, obj2, duration, 2.0f);
	
	return 0;
}

/**
 * Return true if ship *objp firing a laser believes it will hit a teammate.
 */
int might_hit_teammate(object *firing_objp)
{
	int		team;
	object	*objp;
	ship_obj	*so;

	team = Ships[firing_objp->instance].team;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];
		if (Ships[objp->instance].team == team) {
			float		dist, dot;
			vec3d	vec_to_objp;

			vm_vec_sub(&vec_to_objp, &firing_objp->pos, &objp->pos);
			dist = vm_vec_mag_quick(&vec_to_objp);
			dot = vm_vec_dot(&firing_objp->orient.vec.fvec, &vec_to_objp)/dist;
			if (might_collide_with_ship(firing_objp, objp, dot, dist, 2.0f))
				return 1;
		}
	}

	return 0;
}

void render_all_ship_bay_paths(object *objp)
{
	int		i,j,color;
	polymodel	*pm;
	model_path	*mp;

	pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);
	vec3d	global_path_point;
	vertex	v, prev_vertex;

	if ( pm->ship_bay == NULL )
		return;

	for ( i = 0; i < pm->ship_bay->num_paths; i++ ) {
		mp = &pm->paths[pm->ship_bay->path_indexes[i]];

		for ( j = 0; j < mp->nverts; j++ ) {
			vm_vec_unrotate(&global_path_point, &mp->verts[j].pos, &objp->orient);
			vm_vec_add2(&global_path_point, &objp->pos);
			g3_rotate_vertex(&v, &global_path_point);
			color = 255 - j*50;
			if ( color < 50 ) 
				color = 100;
			gr_set_color(0, color, 0);

			if ( j == mp->nverts-1 ) {
				gr_set_color(255, 0, 0);
			}

			g3_draw_sphere( &v, 1.5f);

			if ( j > 0 )
				g3_draw_line(&v, &prev_vertex);

			prev_vertex = v;
	
		}
	}
}

/**
 * Debug function to show all path points associated with an object
 */
void render_all_subsys_paths(object *objp)
{
	int		i,j,color;
	polymodel	*pm;
	model_path	*mp;

	pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);
	vec3d	global_path_point;
	vertex	v, prev_vertex;

	if ( pm->ship_bay == NULL )
		return;

	for ( i = 0; i < pm->n_paths; i++ ) {
		mp = &pm->paths[i];
		for ( j = 0; j < mp->nverts; j++ ) {
			vm_vec_unrotate(&global_path_point, &mp->verts[j].pos, &objp->orient);
			vm_vec_add2(&global_path_point, &objp->pos);
			g3_rotate_vertex(&v, &global_path_point);
			color = 255 - j*50;
			if ( color < 50 ) 
				color = 100;
			gr_set_color(0, color, 0);

			if ( j == mp->nverts-1 ) {
				gr_set_color(255, 0, 0);
			}

			g3_draw_sphere( &v, 1.5f);

			if ( j > 0 )
				g3_draw_line(&v, &prev_vertex);

			prev_vertex = v;
		}
	}
}

void render_path_points(object *objp)
{
	ship		*shipp = &Ships[objp->instance];
	ai_info	*aip = &Ai_info[shipp->ai_index];
	object	*dobjp;
	polymodel	*pm;

	render_all_subsys_paths(objp);
	render_all_ship_bay_paths(objp);

	if (aip->goal_objnum < 0)
		return;

	dobjp = &Objects[aip->goal_objnum];
	pm = model_get(Ship_info[Ships[dobjp->instance].ship_info_index].model_num);
	vec3d	dock_point, global_dock_point;
	vertex	v;

	ship_model_start(&Objects[aip->goal_objnum]);
	if (pm->n_docks) {
		dock_point = pm->docking_bays[0].pnt[0];
		model_find_world_point(&global_dock_point, &dock_point, pm->id, 0, &dobjp->orient, &dobjp->pos );
		g3_rotate_vertex(&v, &global_dock_point);
		gr_set_color(255, 255, 255);
		g3_draw_sphere( &v, 1.5f);
	}

	if (aip->path_start != -1) {
		vertex		prev_vertex;
		pnode			*pp = &Path_points[aip->path_start];
		int			num_points = aip->path_length;
		int			i;

		for (i=0; i<num_points; i++) {
			vertex	v0;

			g3_rotate_vertex( &v0, &pp->pos );

			gr_set_color(0, 128, 96);
			if (i != 0)
				g3_draw_line(&v0, &prev_vertex);

			if (pp-Path_points == aip->path_cur)
				gr_set_color(255,255,0);
			
			g3_draw_sphere( &v0, 4.5f);

			prev_vertex = v0;

			pp++;
		}
	}

	ship_model_stop(&Objects[aip->goal_objnum]);
}

/**
 * Return the distance that the current AI weapon will travel
 */
float ai_get_weapon_dist(ship_weapon *swp)
{
	int	bank_num, weapon_num;

	bank_num = swp->current_primary_bank;
	weapon_num = swp->primary_bank_weapons[bank_num];

	//	If weapon_num is illegal, return a reasonable value.  A valid weapon
	//	will get selected when this ship tries to fire.
	if (weapon_num == -1) {
		return 1000.0f;
	}

	return MIN((Weapon_info[weapon_num].max_speed * Weapon_info[weapon_num].lifetime), Weapon_info[weapon_num].weapon_range);
}

float ai_get_weapon_speed(ship_weapon *swp)
{
	int	bank_num, weapon_num;

	bank_num = swp->current_primary_bank;
	if (bank_num < 0)
		return 100.0f;

	weapon_num = swp->primary_bank_weapons[bank_num];

	if (weapon_num == -1) {
		return 100.0f;
	}

	return Weapon_info[weapon_num].max_speed;
}

//	Compute the predicted position of a ship to be fired upon from a turret.
//	This is based on position of firing gun, enemy object, weapon speed and skill level constraints.
//	Return value in *predicted_enemy_pos.
//	Also, stuff globals G_predicted_pos, G_collision_time and G_fire_pos.
//	*pobjp		object firing the weapon
//	*eobjp		object being fired upon
void set_predicted_enemy_pos_turret(vec3d *predicted_enemy_pos, vec3d *gun_pos, object *pobjp, vec3d *enemy_pos, vec3d *enemy_vel, float weapon_speed, float time_enemy_in_range)
{
	ship	*shipp = &Ships[pobjp->instance];
	float	range_time;

	if (weapon_speed < 1.0f)
		weapon_speed = 1.0f;

	range_time = 2.0f;

	//	Make it take longer for enemies to get player's allies in range based on skill level.
	if (iff_x_attacks_y(Ships[pobjp->instance].team, Player_ship->team))
		range_time += Ai_info[shipp->ai_index].ai_in_range_time;

	if (time_enemy_in_range < range_time) {
		float	dist;

		dist = vm_vec_dist_quick(&pobjp->pos, enemy_pos);
		vm_vec_scale_add(predicted_enemy_pos, enemy_pos, enemy_vel, time_enemy_in_range * dist/weapon_speed);
	} else {
		float	collision_time, scale;
		vec3d	rand_vec;
		ai_info	*aip = &Ai_info[shipp->ai_index];

		collision_time = compute_collision_time(enemy_pos, enemy_vel, gun_pos, weapon_speed);

		if (collision_time == 0.0f){
			collision_time = 100.0f;
		}

		vm_vec_scale_add(predicted_enemy_pos, enemy_pos, enemy_vel, collision_time);
		if (time_enemy_in_range > 2*range_time){
			scale = (1.0f - aip->ai_accuracy) * 4.0f;
		} else {
			scale = (1.0f - aip->ai_accuracy) * 4.0f * (1.0f + 4.0f * (1.0f - time_enemy_in_range/(2*range_time)));
		}		

		static_randvec(((pobjp-Objects) ^ (Missiontime >> 16)) & 7, &rand_vec);

		vm_vec_scale_add2(predicted_enemy_pos, &rand_vec, scale);
		G_collision_time = collision_time;
		G_fire_pos = *gun_pos;
	}

	G_predicted_pos = *predicted_enemy_pos;
}

//	Compute the predicted position of a ship to be fired upon.
//	This is based on current position of firing object, enemy object, relative position of gun on firing object,
//	weapon speed and skill level constraints.
//	Return value in *predicted_enemy_pos.
//	Also, stuff globals G_predicted_pos, G_collision_time and G_fire_pos.
//SUSHI: Modified to take in a position and accel value instead of reading it directly from the enemy object
void set_predicted_enemy_pos(vec3d *predicted_enemy_pos, object *pobjp, vec3d *enemy_pos, vec3d *enemy_vel, ai_info *aip)
{
	float	weapon_speed, range_time;
	ship	*shipp = &Ships[pobjp->instance];
	vec3d	target_moving_direction;

	Assert( enemy_pos != NULL );
	Assert( enemy_vel != NULL );

	target_moving_direction = *enemy_vel;

	if (The_mission.ai_profile->flags & AIPF_USE_ADDITIVE_WEAPON_VELOCITY)
		vm_vec_sub2(&target_moving_direction, &pobjp->phys_info.vel);

	weapon_speed = ai_get_weapon_speed(&shipp->weapons);
	weapon_speed = MAX(weapon_speed, 1.0f);		// set not less than 1

	range_time = 2.0f;

	//	Make it take longer for enemies to get player's allies in range based on skill level.
	// but don't bias team v. team missions
	if ( !(MULTI_TEAM) )
	{
		if (iff_x_attacks_y(shipp->team, Player_ship->team))
			range_time += aip->ai_in_range_time;
	}

	if (aip->time_enemy_in_range < range_time) {
		float	dist;

		dist = vm_vec_dist_quick(&pobjp->pos, enemy_pos);
		vm_vec_scale_add(predicted_enemy_pos, enemy_pos, &target_moving_direction, aip->time_enemy_in_range * dist/weapon_speed);
	} else {
		float	collision_time;
		vec3d	gun_pos, pnt;
		polymodel *pm = model_get(Ship_info[shipp->ship_info_index].model_num);

		//	Compute position of gun in absolute space and use that as fire position 
		//  ...unless we want to just use the ship center
		if(pm->gun_banks != NULL && !(The_mission.ai_profile->flags2 & AIPF2_AI_AIMS_FROM_SHIP_CENTER)){
			pnt = pm->gun_banks[0].pnt[0];
		} else {
			//Use the convergence offset, if there is one
			vm_vec_copy_scale(&pnt, &Ship_info[shipp->ship_info_index].convergence_offset, 1.0f);
		}
		vm_vec_unrotate(&gun_pos, &pnt, &pobjp->orient);
		vm_vec_add2(&gun_pos, &pobjp->pos);

		collision_time = compute_collision_time(enemy_pos, &target_moving_direction, &gun_pos, weapon_speed);

		if (collision_time == 0.0f) {
			collision_time = 100.0f;
		}

		vm_vec_scale_add(predicted_enemy_pos, enemy_pos, &target_moving_direction, collision_time);

		// set globals
		G_collision_time = collision_time;
		G_fire_pos = gun_pos;
	}

	// Now add error terms (1) regular aim (2) EMP (3) stealth
	float scale = 0.0f;
	vec3d rand_vec;

	// regular skill level error in aim
	if (aip->time_enemy_in_range > 2*range_time) {
		scale = (1.0f - aip->ai_accuracy) * 4.0f;
	} else {
		scale = (1.0f - aip->ai_accuracy) * 4.0f * (1.0f + 4.0f * (1.0f - aip->time_enemy_in_range/(2*range_time)));
	}

	// if this ship is under the effect of an EMP blast, throw his aim off a bit
	if (shipp->emp_intensity > 0.0f) {
		// never go lower than 1/2 of the EMP effect max, otherwise things aren't noticeable
		scale += (MAX_EMP_INACCURACY * (shipp->emp_intensity < 0.5f ? 0.5f : shipp->emp_intensity));
		mprintf(("AI miss scale factor (EMP) %f\n",scale));
	}

	// if stealthy ship, throw his aim off, more when farther away and when dot is small
	if ( aip->ai_flags & AIF_STEALTH_PURSUIT ) {
		float dist = vm_vec_dist_quick(&pobjp->pos, enemy_pos);
		vec3d temp;
		vm_vec_sub(&temp, enemy_pos, &pobjp->pos);
		vm_vec_normalize_quick(&temp);
		float dot = vm_vec_dotprod(&temp, &pobjp->orient.vec.fvec);
		float st_err = 3.0f * (1.4f - dot) * (1.0f + dist / (get_skill_stealth_dist_scaler() * STEALTH_MAX_VIEW_DIST)) * (1 - aip->ai_accuracy);
		scale += st_err;
	}

	// get a random vector that changes slowly over time (1x / sec)
	static_randvec(((pobjp-Objects) ^ (Missiontime >> 16)) & 7, &rand_vec);

	vm_vec_scale_add2(predicted_enemy_pos, &rand_vec, scale);

	// set global
	G_predicted_pos = *predicted_enemy_pos;
}

/**
 * Handler of submode for Chase.  Go into a continuous turn for awhile.
 */
void ai_chase_ct()
{
	vec3d		tvec;
	ship_info	*sip;
	ai_info		*aip;

	Assert(Ships[Pl_objp->instance].ship_info_index >= 0);
	sip = &Ship_info[Ships[Pl_objp->instance].ship_info_index];
	Assert(Ships[Pl_objp->instance].ai_index >= 0);
	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	//	Make a continuous turn towards any combination of possibly negated
	// up and right vectors.
	tvec = Pl_objp->pos;

	if (aip->submode_parm0 & 0x01)
		vm_vec_add2(&tvec, &Pl_objp->orient.vec.rvec);
	if (aip->submode_parm0 & 0x02)
		vm_vec_sub2(&tvec, &Pl_objp->orient.vec.rvec);
	if (aip->submode_parm0 & 0x04)
		vm_vec_add2(&tvec, &Pl_objp->orient.vec.uvec);
	if (aip->submode_parm0 & 0x08)
		vm_vec_sub2(&tvec, &Pl_objp->orient.vec.uvec);

	//	Detect degenerate cases that cause tvec to be same as player pos.
	if (vm_vec_dist_quick(&tvec, &Pl_objp->pos) < 0.1f) {
		aip->submode_parm0 &= 0x05;
		if (aip->submode_parm0 == 0)
			aip->submode_parm0 = 1;
		vm_vec_add2(&tvec, &Pl_objp->orient.vec.rvec);
	}

	ai_turn_towards_vector(&tvec, Pl_objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0);
	accelerate_ship(aip, 1.0f);
}

/**
 * ATTACK submode handler for chase mode.
 */
void ai_chase_eb(ai_info *aip, ship_info *sip, vec3d *predicted_enemy_pos, float dist_to_enemy)
{
	vec3d	_pep;
	float		dot_to_enemy, dot_from_enemy;

	compute_dots(Pl_objp, En_objp, &dot_to_enemy, &dot_from_enemy);

	//	If we're trying to slow down to get behind, then point to turn towards is different.
	_pep = *predicted_enemy_pos;
	if ((dot_to_enemy > dot_from_enemy + 0.1f) || (dot_to_enemy > 0.9f))
		vm_vec_scale_add(&_pep, &Pl_objp->pos, &En_objp->orient.vec.fvec, 100.0f);

	ai_turn_towards_vector(&_pep, Pl_objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0);

	accelerate_ship(aip, 0.0f);
}

//	Return time until weapon_objp might hit ship_objp.
//	Assumes ship_objp is not moving.
//	Returns negative time if not going to hit.
//	This is a very approximate function, but is pretty fast.
float ai_endangered_time(object *ship_objp, object *weapon_objp)
{
	float		to_dot, from_dot, dist;

	dist = compute_dots(ship_objp, weapon_objp, &to_dot, &from_dot);

	//	Note, this is bogus.  It assumes only the weapon is moving.
	//	Only proceed if weapon sort of pointing at object and object pointing towards or away from weapon
	//	(Ie, if object moving at right angle to weapon, just continue for now...)
	if (weapon_objp->phys_info.speed < 1.0f)
		return dist + 1.0f;
	else if ((from_dot > 0.1f) && (dist/(from_dot*from_dot) < 48*ship_objp->radius)) //: don't require them to see it, they have instruments!: && (fl_abs(to_dot) > 0.5f))
		return dist / weapon_objp->phys_info.speed;
	else
		return -1.0f;
}

//	Return time until danger weapon could hit this ai object.
//	Return negative time if not endangered.
float ai_endangered_by_weapon(ai_info *aip)
{
	object	*weapon_objp;

	if (aip->danger_weapon_objnum == -1) {
		return -1.0f;
	}

	weapon_objp = &Objects[aip->danger_weapon_objnum];

	if (weapon_objp->signature != aip->danger_weapon_signature) {
		aip->danger_weapon_objnum = -1;
		return -1.0f;
	}

	return ai_endangered_time(&Objects[Ships[aip->shipnum].objnum], weapon_objp);
}

//	Return true if this ship is near full strength.
// Goober5000: simplified and accounted for shields being 0
int ai_near_full_strength(object *objp)
{	
	return (get_hull_pct(objp) > 0.9f) || (get_shield_pct(objp) > 0.8f);
}

void do_random_sidethrust(ai_info *aip, ship_info *sip)
{
	//Sidethrust vector is initially based on the velocity vector representing the ship's current sideways motion.
	vec2d side_vec;
	//Length to hold circle strafe is random (1-3) + slide accel time, changes every 4 seconds
	int strafeHoldDirAmount = (int)(sip->slide_accel) + static_rand_range((Missiontime + static_rand(aip->shipnum)) >> 18, 1, 3);
	//Get a random float using some of the more significant chunks of the missiontime as a seed (>>16 means it changes every second)
	//This means that we get the same random values for a little bit.
	//Using static_rand(shipnum) as a crude hash function to make sure that the seed is different for each ship and direction
	//The *2 ensures that y and x stay separate.
	if (strafeHoldDirAmount > 0) { //This may look unnecessary, but we're apparently still getting div by zero errors on Macs here.
		side_vec.x = static_randf_range((((Missiontime + static_rand(aip->shipnum)) >> 16) / strafeHoldDirAmount) , -1.0f, 1.0f);
		side_vec.y = static_randf_range((((Missiontime + static_rand(aip->shipnum)) >> 16) / strafeHoldDirAmount) * 2, -1.0f, 1.0f);
	} else {
		Warning(LOCATION, "Division by zero in do_random_sidethrust averted. Please tell a coder.\n");
		side_vec.x = 1.0f;
		side_vec.y = 1.0f;
	}
	//Scale it up so that the longest dimension is length 1.0. This ensures we are always getting as much use out of sidethrust as possible.
	vm_vec_boxscale(&side_vec, 1.0f);

	AI_ci.sideways = side_vec.x;
	AI_ci.vertical = side_vec.y;
}
				
/**
 * Set acceleration while in attack mode.
 */
void attack_set_accel(ai_info *aip, ship_info *sip, float dist_to_enemy, float dot_to_enemy, float dot_from_enemy)
{
	float	speed_ratio;

	if (En_objp->phys_info.speed > 1.0f)
		speed_ratio = Pl_objp->phys_info.speed/En_objp->phys_info.speed;
	else
		speed_ratio = 5.0f;

	//	Sometimes, told to attack slowly.  Allows to get in more hits.
	if (aip->ai_flags & AIF_ATTACK_SLOWLY) {
		if ((dist_to_enemy > 200.0f) && (dist_to_enemy < 800.0f)) {
			if ((dot_from_enemy < 0.9f) || ai_near_full_strength(Pl_objp)) {
				accelerate_ship(aip, MAX(1.0f - (dist_to_enemy-200.0f)/600.0f, 0.1f));
				return;
			}
		} else
			aip->ai_flags &= ~AIF_ATTACK_SLOWLY;
	}

	//Glide attack: we turn on glide to maintain current vector while aiming at the enemy
	//The aiming part should already be taken care of. 
	if (aip->submode == AIS_CHASE_GLIDEATTACK) {
		Pl_objp->phys_info.flags |= PF_GLIDING;
		accelerate_ship(aip, 0.0f);
		return;
	}

	//Circle Strafe: We try to maintain a constant distance from the target while using sidethrust to move in a circle
	//around the target
	if (aip->submode == AIS_CHASE_CIRCLESTRAFE) {
		//If glide is available, use it (smooths things out a bit)
		if (sip->can_glide == true)
			Pl_objp->phys_info.flags |= PF_GLIDING;

		//Try to maintain a distance between 50% and 75% of maximum circle strafe distance
		if (dist_to_enemy <= CIRCLE_STRAFE_MAX_DIST * .5)
			accelerate_ship(aip, -1.0f); 
		else if (dist_to_enemy >= CIRCLE_STRAFE_MAX_DIST * 0.75)
			accelerate_ship(aip, 1.0f); 
		else
			accelerate_ship(aip, 0.0f); 

		do_random_sidethrust(aip, sip);
		return;
	}


	if (dist_to_enemy > 200.0f + vm_vec_mag_quick(&En_objp->phys_info.vel) * dot_from_enemy + Pl_objp->phys_info.speed * speed_ratio) {
		if (ai_maybe_fire_afterburner(Pl_objp, aip)) {
			if (dist_to_enemy > 800.0f) {
				if (!( Pl_objp->phys_info.flags & PF_AFTERBURNER_ON )) {
					float percent_left;
					ship	*shipp;
					ship_info *sip_local;

					shipp = &Ships[Pl_objp->instance];
					sip_local = &Ship_info[shipp->ship_info_index];

					if (sip_local->afterburner_fuel_capacity > 0.0f) {
						percent_left = 100.0f * shipp->afterburner_fuel / sip_local->afterburner_fuel_capacity;
						if (percent_left > 30.0f + ((Pl_objp-Objects) & 0x0f)) {
							afterburners_start(Pl_objp);							
							if (aip->ai_profile_flags & AIPF_SMART_AFTERBURNER_MANAGEMENT) {
								float max_ab_vel;
								float time_to_exhaust_25pct_fuel;
								float time_to_fly_75pct_of_distance;
								float ab_time;

								// Max afterburner speed - make sure we don't devide by 0 later
								max_ab_vel = sip_local->afterburner_max_vel.xyz.z > 0.0f ? sip_local->afterburner_max_vel.xyz.z : sip_local->max_vel.xyz.z;
								max_ab_vel = max_ab_vel > 0.0f ? max_ab_vel : 0.0001f;

								// Time to exhaust 25% of the remaining fuel
								time_to_exhaust_25pct_fuel = shipp->afterburner_fuel * 0.25f / sip_local->afterburner_burn_rate;

								// Time to fly 75% of the distance to the target
								time_to_fly_75pct_of_distance = dist_to_enemy * 0.75f / max_ab_vel;

								// Get minimum
								ab_time = MIN(time_to_exhaust_25pct_fuel, time_to_fly_75pct_of_distance);								
								
								aip->afterburner_stop_time = (fix) (Missiontime + F1_0 * ab_time);
							} else {				
								aip->afterburner_stop_time = Missiontime + F1_0 + static_rand(Pl_objp-Objects)/4;
							}
						}
					}
				}
			}
		}

		accelerate_ship(aip, 1.0f);
	} else if ((Missiontime - aip->last_hit_time > F1_0*7)
		&& (En_objp->phys_info.speed < 10.0f) 
		&& (dist_to_enemy > 25.0f) 
		&& (dot_to_enemy > 0.8f)
		&& (dot_from_enemy < 0.8f)) {
		accelerate_ship(aip, 0.0f);		//	No one attacking us, so don't need to move.
	} else if ((dot_from_enemy < 0.25f) && (dot_to_enemy > 0.5f)) {
		set_accel_for_target_speed(Pl_objp, En_objp->phys_info.speed);
	} else if (Pl_objp->phys_info.speed < 15.0f) {
		accelerate_ship(aip, 1.0f);
	} else if (Pl_objp->phys_info.speed > En_objp->phys_info.speed - 1.0f) {
		if (dot_from_enemy > 0.75f)
			accelerate_ship(aip, 1.0f);
		else
			set_accel_for_target_speed(Pl_objp, En_objp->phys_info.speed*0.75f + 3.0f);
	} else {
		change_acceleration(aip, 0.5f);
	}
}

//	Pl_objp (aip) tries to get behind En_objp.
//	New on 2/21/98: If this ship can move backwards and slide, maybe do that to get behind.
void get_behind_ship(ai_info *aip, ship_info *sip, float dist_to_enemy)
{
	vec3d	new_pos;
	float		dot;
	vec3d	vec_from_enemy;
	float		dist;

	dist = vm_vec_normalized_dir(&vec_from_enemy, &Pl_objp->pos, &En_objp->pos);

	vm_vec_scale_add(&new_pos, &En_objp->pos, &En_objp->orient.vec.fvec, -100.0f);		//	Pick point 100 units behind.
	ai_turn_towards_vector(&new_pos, Pl_objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0);

	dot = vm_vec_dot(&vec_from_enemy, &En_objp->orient.vec.fvec);

	if (dot > 0.25f) {
		accelerate_ship(aip, 1.0f);
	} else {
		accelerate_ship(aip, (dot + 1.0f)/2.0f);
	}
}

int avoid_player(object *objp, vec3d *goal_pos)
{
	maybe_avoid_player(Pl_objp, goal_pos);
	ai_info	*aip = &Ai_info[Ships[objp->instance].ai_index];

	if (aip->ai_flags & AIF_AVOIDING_SMALL_SHIP) {
		ship_info *sip = &Ship_info[Ships[objp->instance].ship_info_index];

		if (aip->ai_flags & AIF_AVOIDING_SMALL_SHIP) {
			ai_turn_towards_vector(&aip->avoid_goal_point, objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0);
			accelerate_ship(aip, 0.5f);
			return 1;
		}
	}

	return 0;
}

//	Determine if a cylinder of width radius from p0 to p1 will collide with big_objp.
//	If so, stuff *collision_point.
int will_collide_pp(vec3d *p0, vec3d *p1, float radius, object *big_objp, vec3d *collision_point)
{
	mc_info	mc;
	polymodel *pm = model_get(Ship_info[Ships[big_objp->instance].ship_info_index].model_num);

	mc.model_instance_num = -1;
	mc.model_num = pm->id;				// Fill in the model to check
	mc.orient = &big_objp->orient;			// The object's orient
	mc.pos = &big_objp->pos;					// The object's position
	mc.p0 = p0;										// Point 1 of ray to check
	mc.p1 = p1;
	mc.flags = MC_CHECK_MODEL | MC_CHECK_SPHERELINE | MC_SUBMODEL;					// flags

	mc.radius = radius;

	// Only check the 2nd lowest hull object
	mc.submodel_num = pm->detail[0];
	model_collide(&mc);

	if (mc.num_hits)
		*collision_point = mc.hit_point_world;

	return mc.num_hits;
}

//	Return true/false if *objp will collide with *big_objp
//	Stuff distance in *distance to collision point if *objp will collide with *big_objp within delta_time seconds.
//	Global collision point stuffed in *collision_point
int will_collide_with_big_ship(object *objp, vec3d *goal_point, object *big_objp, vec3d *collision_point, float delta_time)
{
	float		radius;
	vec3d	end_pos;

	radius = big_objp->radius + delta_time * objp->phys_info.speed;

	if (vm_vec_dist_quick(&big_objp->pos, &objp->pos) > radius) {
		return 0;
	}

	if (goal_point == NULL) {
		vm_vec_scale_add(&end_pos, &objp->pos, &objp->phys_info.vel, delta_time);					// Point 2 of ray to check
	} else {
		end_pos = *goal_point;
	}

	return will_collide_pp(&objp->pos, &end_pos, objp->radius, big_objp, collision_point);
}

//	Return true if *objp is expected to collide with a large ship.
//	Stuff global collision point in *collision_point.
//	If *goal_point is not NULL, use that as the point towards which *objp will be flying.  Don't use *objp velocity
//	*ignore_objp will typically be the target this ship is pursuing, either to attack or guard.  We don't want to avoid it.
int will_collide_with_big_ship_all(object *objp, object *ignore_objp, vec3d *goal_point, vec3d *collision_point, float *distance, float delta_time)
{
	ship_obj	*so;
	object	*big_objp;
	int		collision_obj_index = -1;
	float		min_dist = 999999.9f;
	float		collision_time = -1.0f;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		float	time = 0.0f;
		big_objp = &Objects[so->objnum];

		if (big_objp == ignore_objp)
			continue;

		if (Ship_info[Ships[big_objp->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {
			vec3d	cur_collision_point;
			float		cur_dist;

			if (will_collide_with_big_ship(objp, goal_point, big_objp, &cur_collision_point, delta_time)) {

				cur_dist = vm_vec_dist(&cur_collision_point, &objp->pos);

				if (cur_dist < min_dist) {
					min_dist = cur_dist;
					*collision_point = cur_collision_point;
					collision_time = time;
					collision_obj_index = OBJ_INDEX(big_objp);
				}
			}
		}
	}

	*distance = min_dist;
	return collision_obj_index;

}

typedef struct {
	float		dist;
	int		collide;
	vec3d	pos;
} sgoal;

//	Pick a point for *objp to fly towards to avoid a collision with *big_objp at *collision_point
//	Return result in *avoid_pos
void mabs_pick_goal_point(object *objp, object *big_objp, vec3d *collision_point, vec3d *avoid_pos)
{
	matrix	mat1;
	sgoal		goals[4];
	vec3d	v2b;

	vm_vec_normalized_dir(&v2b, collision_point, &objp->pos);
	vm_vector_2_matrix(&mat1, &v2b, NULL, NULL);

	int	found = 0;

	//	Try various scales, in 0.5f, 0.75f, 1.0f, 1.25f.
	//	First try 0.5f to see if we can find a point that near the center of the target ship, which presumably
	//	means less of a turn.
	//	Try going as far as 1.25f * radius.
	float	s;
	for (s=0.5f; s<1.3f; s += 0.25f) {
		int	i;
		for (i=0; i<4; i++) {
			vec3d p = big_objp->pos;
			float ku = big_objp->radius*s + objp->radius * (OBJ_INDEX(objp) % 4)/4;		//	This objp->radius stuff to prevent ships from glomming together at one point
			float kr = big_objp->radius*s + objp->radius * ((OBJ_INDEX(objp) % 4) ^ 2)/4;
			if (i&1)
				ku = -ku;
			if (i&2)
				kr = -kr;
			vm_vec_scale_add2(&p, &mat1.vec.uvec, ku);
			vm_vec_scale_add2(&p, &mat1.vec.rvec, kr);
			goals[i].pos = p;
			goals[i].dist = vm_vec_dist_quick(&objp->pos, &p);
			goals[i].collide = will_collide_pp(&objp->pos, &p, objp->radius, big_objp, collision_point);
			if (!goals[i].collide)
				found = 1;
		}

		//	If we found a point that doesn't collide, find the nearest one and make that the *avoid_pos.
		if (found) {
			float	min_dist = 9999999.9f;
			int	min_index = -1;

			for (i=0; i<4; i++) {
				if (!goals[i].collide && (goals[i].dist < min_dist)) {
					min_dist = goals[i].dist;
					min_index = i;
				}
			}

			Assert(i != -1);
			if (i != -1) {
				*avoid_pos = goals[min_index].pos;
				return;
			}
		}
	}

	//	Drat.  We tried and tried and could not find a point that did not cause a collision.
	//	Get this dump pilot far away from the problem ship.
	vec3d	away_vec;
	vm_vec_normalized_dir(&away_vec, &objp->pos, collision_point);
	vm_vec_scale_add(avoid_pos, &objp->pos, &away_vec, big_objp->radius*1.5f);

}

/**
 * Return true if a large ship is being ignored.
 */
int maybe_avoid_big_ship(object *objp, object *ignore_objp, ai_info *aip, vec3d *goal_point, float delta_time)
{
	if (timestamp_elapsed(aip->avoid_check_timestamp)) {
		float		distance;
		vec3d	collision_point;
		int		ship_num;
		if ((ship_num = will_collide_with_big_ship_all(Pl_objp, ignore_objp, goal_point, &collision_point, &distance, delta_time)) != -1) {
			aip->ai_flags |= AIF_AVOIDING_BIG_SHIP;
			mabs_pick_goal_point(objp, &Objects[ship_num], &collision_point, &aip->avoid_goal_point);
			float dist = vm_vec_dist_quick(&aip->avoid_goal_point, &objp->pos);
			aip->avoid_check_timestamp = timestamp(2000 + MIN(1000, (int) (dist * 2.0f)));	//	Delay until check again is based on distance to avoid point.
			aip->avoid_ship_num = ship_num;
		} else {
			aip->ai_flags &= ~AIF_AVOIDING_BIG_SHIP;
			aip->ai_flags &= ~AIF_AVOIDING_SMALL_SHIP;
			aip->avoid_ship_num = -1;
			aip->avoid_check_timestamp = timestamp(1500);
		}
	}
	
	if (aip->ai_flags & AIF_AVOIDING_BIG_SHIP) {
		ship_info *sip = &Ship_info[Ships[Pl_objp->instance].ship_info_index];

		vec3d	v2g;

		ai_turn_towards_vector(&aip->avoid_goal_point, Pl_objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0);
		vm_vec_normalized_dir(&v2g, &aip->avoid_goal_point, &Pl_objp->pos);
		float dot = vm_vec_dot(&objp->orient.vec.fvec, &v2g);
		float d2 = (1.0f + dot) * (1.0f + dot);
		accelerate_ship(aip, d2/4.0f);
		return 1;
	}

	return 0;
}

/**
 * Set desired right vector for ships flying towards another ship.
 * Since this is governed only by vector to target, it causes ships to align bank and look less chaotic.
 */
void compute_desired_rvec(vec3d *rvec, vec3d *goal_pos, vec3d *cur_pos)
{
	vec3d	v2e;

	vm_vec_normalized_dir(&v2e, goal_pos, cur_pos);
	rvec->xyz.x = v2e.xyz.z;
	rvec->xyz.y = 0.0f;
	rvec->xyz.z = -v2e.xyz.x;
	if (vm_vec_mag_squared(rvec) < 0.001f)
		rvec->xyz.y = 1.0f;
}

/**
 * Handler for stealth find submode of Chase.
 */
void ai_stealth_find()
{
	ai_info		*aip;
	ship_info	*sip;

	vec3d new_pos, vec_to_enemy;
	float dist_to_enemy, dot_to_enemy, dot_from_enemy;

	Assert(Ships[Pl_objp->instance].ship_info_index >= 0);
	sip = &Ship_info[Ships[Pl_objp->instance].ship_info_index];
	Assert(Ships[Pl_objp->instance].ai_index >= 0);
	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	// get time since last seen
	int delta_time = (timestamp() - aip->stealth_last_visible_stamp);

	// if delta_time is really big, i'm real confused, start sweep
	if (delta_time > 10000) {
		aip->submode_parm0 = SM_SF_BAIL;
	}

	// guestimate new position
	vm_vec_scale_add(&new_pos, &aip->stealth_last_pos, &aip->stealth_velocity, (delta_time * 0.001f));

	// if I think he's behind me, go to the goal point
	if ( aip->submode_parm0 == SM_SF_BEHIND ) {
		new_pos = aip->goal_point;
	}

	// check for collision with big ships
	if (maybe_avoid_big_ship(Pl_objp, En_objp, aip, &new_pos, 10.0f)) {
		// reset ai submode to chase
		return;
	}

	// if dist is near max and dot is close to 1, accel, afterburn
	vm_vec_sub(&vec_to_enemy, &new_pos, &Pl_objp->pos);
	dist_to_enemy = vm_vec_normalize_quick(&vec_to_enemy);
	dot_to_enemy = vm_vec_dotprod(&vec_to_enemy, &Pl_objp->orient.vec.fvec);

	// if i think i should see him ahead and i don't, set goal pos and turn around, but only if I haven't seen him for a while
	if ( (delta_time > 800) && (aip->submode_parm0 == SM_SF_AHEAD) && (dot_to_enemy > .94) && (dist_to_enemy < get_skill_stealth_dist_scaler()*STEALTH_MAX_VIEW_DIST + 50) ) {
		// do turn around)
		vm_vec_scale_add(&aip->goal_point, &Pl_objp->pos, &Pl_objp->orient.vec.fvec, -300.0f);
		aip->submode_parm0 = SM_SF_BEHIND;
		vm_vec_sub(&vec_to_enemy, &new_pos, &Pl_objp->pos);
		dist_to_enemy = vm_vec_normalize_quick(&vec_to_enemy);
		dot_to_enemy = vm_vec_dotprod(&vec_to_enemy, &Pl_objp->orient.vec.fvec);
	}

	if ( (dist_to_enemy > get_skill_stealth_dist_scaler()*STEALTH_MAX_VIEW_DIST) && (dot_to_enemy > 0.94f) ) {		// 20 degree half angle
		// accelerate ship
		accelerate_ship(aip, 1.0f);

		// engage afterburner
		if (!( Pl_objp->phys_info.flags & PF_AFTERBURNER_ON )) {
			if (ai_maybe_fire_afterburner(Pl_objp, aip)) {
				afterburners_start(Pl_objp);
				aip->afterburner_stop_time = Missiontime + 3*F1_0/2;
			}
		}

		ai_turn_towards_vector(&new_pos, Pl_objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0);
		return;
	}

	//	If enemy more than 500 meters away, all ships flying there will tend to match bank.
	//	They do this by using their vector to their target to compute their right vector and causing ai_turn_towards_vector
	//	to interpolate a matrix rather than just a vector.
	if (dist_to_enemy > 500.0f) {
		vec3d	rvec;
		compute_desired_rvec(&rvec, &new_pos, &Pl_objp->pos);
		ai_turn_towards_vector(&new_pos, Pl_objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0, &rvec);
	} else {
		ai_turn_towards_vector(&new_pos, Pl_objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0);
	}

	dot_from_enemy = -vm_vec_dotprod(&vec_to_enemy, &En_objp->orient.vec.fvec);

	attack_set_accel(aip, sip, dist_to_enemy, dot_to_enemy, dot_from_enemy);
}

/**
 * Try to find stealth ship by sweeping an area
 */
void ai_stealth_sweep()
{
	ai_info		*aip;
	ship_info	*sip;

	Assert(Ships[Pl_objp->instance].ship_info_index >= 0);
	sip = &Ship_info[Ships[Pl_objp->instance].ship_info_index];
	Assert(Ships[Pl_objp->instance].ai_index >= 0);
	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	vec3d goal_pt;
	vec3d forward, right, up;
	int lost_time;

	// time since stealth last seen
	lost_time = (timestamp() - aip->stealth_last_visible_stamp);

	// determine which pt to fly to in sweep by keeping track of parm0
	if (aip->submode_parm0 == SM_SS_SET_GOAL) {

		// don't make goal pt more than 2k from current pos
		vm_vec_scale_add(&goal_pt, &aip->stealth_last_pos, &aip->stealth_velocity, (0.001f * lost_time));

		// make box size based on speed of stealth and expected time to intercept (keep box in range 200-500)
		float box_size = vm_vec_mag_quick(&aip->stealth_velocity) * (0.001f * lost_time);
		box_size = MIN(200.0f, box_size);
		box_size = MAX(500.0f, box_size);
		aip->stealth_sweep_box_size = box_size;

		aip->goal_point = goal_pt;
		aip->submode_parm0 = SM_SS_BOX0;
	}

	// GET UP, RIGHT, FORWARD FOR BOX based on stealth ship's velocity
	// if velocity changes in stealth mode, then ship is *seen*, and falls out of sweep mode
	// if stealth has no velocity make a velocity
	if ( vm_vec_mag_quick(&aip->stealth_velocity) < 1 ) {
		vm_vec_rand_vec_quick(&aip->stealth_velocity);
	}

	// get "right" vector for box
	vm_vec_crossprod(&right, &aip->stealth_velocity, &vmd_y_vector);

	if ( vm_vec_mag_quick(&right) < 0.01 ) {
		vm_vec_crossprod(&right, &aip->stealth_velocity, &vmd_z_vector);
	}

	vm_vec_normalize_quick(&right);

	// get "forward" for box
	vm_vec_copy_normalize_quick(&forward, &aip->stealth_velocity);

	// get "up" for box
	vm_vec_crossprod(&up, &forward, &right);
	
	// lost far away ahead (do box)
	switch(aip->submode_parm0) {
	case SM_SS_BOX0:
		goal_pt = aip->goal_point;
		break;

	// pt1 -U +R
	case SM_SS_LR:
		vm_vec_scale_add(&goal_pt, &aip->goal_point, &up, -aip->stealth_sweep_box_size);
		vm_vec_scale_add2(&goal_pt, &right, aip->stealth_sweep_box_size);
		vm_vec_scale_add2(&goal_pt, &forward, 0.5f*aip->stealth_sweep_box_size);
		break;

	// pt2 +U -R
	case SM_SS_UL:
		vm_vec_scale_add(&goal_pt, &aip->goal_point, &up, aip->stealth_sweep_box_size);
		vm_vec_scale_add2(&goal_pt, &right, -aip->stealth_sweep_box_size);
		vm_vec_scale_add2(&goal_pt, &forward, 0.5f*aip->stealth_sweep_box_size);
		break;

	// pt3 back
	case SM_SS_BOX1:
		goal_pt = aip->goal_point;
		break;

	// pt4 +U +R
	case SM_SS_UR:
		vm_vec_scale_add(&goal_pt, &aip->goal_point, &up, aip->stealth_sweep_box_size);
		vm_vec_scale_add2(&goal_pt, &right, aip->stealth_sweep_box_size);
		vm_vec_scale_add2(&goal_pt, &forward, 0.5f*aip->stealth_sweep_box_size);
		break;

	// pt5 -U -R
	case SM_SS_LL:
		vm_vec_scale_add(&goal_pt, &aip->goal_point, &up, -aip->stealth_sweep_box_size);
		vm_vec_scale_add2(&goal_pt, &right, -aip->stealth_sweep_box_size);
		vm_vec_scale_add2(&goal_pt, &forward, 0.5f*aip->stealth_sweep_box_size);
		break;

	// pt6 back
	case SM_SS_BOX2:
		goal_pt = aip->goal_point;
		break;

	default:
		Int3();

	}

	// when close to goal_pt, update next goal pt
	float dist_to_goal = vm_vec_dist(&goal_pt, &Pl_objp->pos);
	if (dist_to_goal < 15) {
		aip->submode_parm0++;
	}

	// check for collision with big ship
	if (maybe_avoid_big_ship(Pl_objp, En_objp, aip, &goal_pt, 10.0f)) {
		// skip to the next pt on box
		aip->submode_parm0++;
		return;
	}

	ai_turn_towards_vector(&goal_pt, Pl_objp, flFrametime, sip->srotation_time, NULL, NULL, 0.0f, 0);

	float dot = 1.0f;
	if (dist_to_goal < 100) {
		vec3d vec_to_goal;
		vm_vec_normalized_dir(&vec_to_goal, &goal_pt, &Pl_objp->pos);
		dot = vm_vec_dotprod(&vec_to_goal, &Pl_objp->orient.vec.fvec);
	}

	accelerate_ship(aip, 0.8f*dot);
}

/**
 * ATTACK submode handler for chase mode.
 */
void ai_chase_attack(ai_info *aip, ship_info *sip, vec3d *predicted_enemy_pos, float dist_to_enemy, int modelnum)
{
	int		start_bank;
	float		dot_to_enemy, dot_from_enemy;
	float		bank_override = 0.0f;

	if (!(aip->ai_profile_flags2 & AIPF2_NO_SPECIAL_PLAYER_AVOID) && avoid_player(Pl_objp, predicted_enemy_pos))
		return;

	compute_dots(Pl_objp, En_objp, &dot_to_enemy, &dot_from_enemy);

	polymodel *po = model_get( modelnum );

	vec3d	*rel_pos;
	float		scale;
	vec3d	randvec;
	vec3d	new_pos;

	start_bank = Ships[aip->shipnum].weapons.current_primary_bank;
	if (po->n_guns && start_bank != -1 ) {
		rel_pos = &po->gun_banks[start_bank].pnt[0];
	} else
		rel_pos = NULL;

	//	If ship moving slowly relative to its size, then don't attack its center point.
	//	How far from center we attack is based on speed, size and distance to enemy
	if (En_objp->radius > En_objp->phys_info.speed) {
		static_randvec(Pl_objp-Objects, &randvec);
		scale = dist_to_enemy/(dist_to_enemy + En_objp->radius) * En_objp->radius;
		scale *= 0.5f * En_objp->radius/(En_objp->phys_info.speed + En_objp->radius);	// scale downward by 1/2 to 1/4
		vm_vec_scale_add(&new_pos, predicted_enemy_pos, &randvec, scale);
	} else
		new_pos = *predicted_enemy_pos;

	//SUSHI: Don't change bank while circle strafing or glide attacking
	if (dist_to_enemy < 250.0f && dot_from_enemy > 0.7f && aip->submode != AIS_CHASE_CIRCLESTRAFE && aip->submode != AIS_CHASE_GLIDEATTACK) {
		bank_override = Pl_objp->phys_info.speed;
	}

	//	If enemy more than 500 meters away, all ships flying there will tend to match bank.
	//	They do this by using their vector to their target to compute their right vector and causing ai_turn_towards_vector
	//	to interpolate a matrix rather than just a vector.
	if (dist_to_enemy > 500.0f) {
		vec3d	rvec;
		compute_desired_rvec(&rvec, predicted_enemy_pos, &Pl_objp->pos);
		ai_turn_towards_vector(&new_pos, Pl_objp, flFrametime, sip->srotation_time, NULL, rel_pos, bank_override, 0, &rvec);
	} else {
		ai_turn_towards_vector(&new_pos, Pl_objp, flFrametime, sip->srotation_time, NULL, rel_pos, bank_override, 0);
	}

	attack_set_accel(aip, sip, dist_to_enemy, dot_to_enemy, dot_from_enemy);
}

//	EVADE_SQUIGGLE submode handler for chase mode.
//	Changed by MK on 5/5/97.
//	Used to evade towards a point off the right or up vector.
//	Now, evade straight away to try to get far away.
//	The squiggling should protect against laser fire.
void ai_chase_es(ai_info *aip, ship_info *sip)
{
	vec3d	tvec;
	fix		timeslice;
	fix		scale;

	tvec = Pl_objp->pos;

	timeslice = (Missiontime >> 16) & 0x0f;
	scale = ((Missiontime >> 16) & 0x0f) << 14;

	if (timeslice & 0x01)
		vm_vec_scale_add2(&tvec, &Pl_objp->orient.vec.rvec, f2fl(scale ^ 0x10000));
	if (timeslice & 0x02)
		vm_vec_scale_sub2(&tvec, &Pl_objp->orient.vec.rvec, f2fl(scale));
	if (timeslice & 0x04)
		vm_vec_scale_add2(&tvec, &Pl_objp->orient.vec.uvec, f2fl(scale ^ 0x10000));
	if (timeslice & 0x08)
		vm_vec_scale_sub2(&tvec, &Pl_objp->orient.vec.uvec, f2fl(scale));

	while (vm_vec_dist_quick(&tvec, &Pl_objp->pos) < 0.1f) {
		tvec.xyz.x += frand();
		tvec.xyz.y += frand();
	}

	float bank_override = Pl_objp->phys_info.speed;

	ai_turn_towards_vector(&tvec, Pl_objp, flFrametime/2, sip->srotation_time, NULL, NULL, bank_override, 0);
	accelerate_ship(aip, 1.0f);
}

/**
 * Trying to get away from opponent.
 */
void ai_chase_ga(ai_info *aip, ship_info *sip)
{
	//	If not near end of this submode, evade squiggly.  If near end, just fly straight for a bit
	vec3d	tvec;
	float		bank_override;
	vec3d	vec_from_enemy;

	if (En_objp != NULL) {
		vm_vec_normalized_dir(&vec_from_enemy, &Pl_objp->pos, &En_objp->pos);
	} else
		vec_from_enemy = Pl_objp->orient.vec.fvec;

	static_randvec(Missiontime >> 15, &tvec);
	vm_vec_scale(&tvec, 100.0f);
	vm_vec_scale_add2(&tvec, &vec_from_enemy, 300.0f);
	vm_vec_add2(&tvec, &Pl_objp->pos);

	bank_override = Pl_objp->phys_info.speed;

	ai_turn_towards_vector(&tvec, Pl_objp, flFrametime/2, sip->srotation_time, NULL, NULL, bank_override, 0);

	accelerate_ship(aip, 2.0f);

	if (ai_maybe_fire_afterburner(Pl_objp, aip)) {
		if (!(Pl_objp->phys_info.flags & PF_AFTERBURNER_ON )) {
			float percent_left = 100.0f * Ships[Pl_objp->instance].afterburner_fuel / sip->afterburner_fuel_capacity;
			if (percent_left > 30.0f + ((Pl_objp-Objects) & 0x0f)) {
				afterburners_start(Pl_objp);
				aip->afterburner_stop_time = Missiontime + 3*F1_0/2;
			}
			afterburners_start(Pl_objp);
			aip->afterburner_stop_time = Missiontime + 3*F1_0/2;
		}
	}

}

//	Make object *objp attack subsystem with ID = subnum.
//	Return true if found a subsystem to attack, else return false.
//	Note, can fail if subsystem exists, but has no hits.
int ai_set_attack_subsystem(object *objp, int subnum)
{
	int temp;
	ship			*shipp, *attacker_shipp;
	ai_info		*aip;
	ship_subsys	*ssp;
	object		*attacked_objp;

	Assert(objp->type == OBJ_SHIP);
	Assert(objp->instance >= 0);

	attacker_shipp = &Ships[objp->instance];
	Assert(attacker_shipp->ai_index >= 0);

	aip = &Ai_info[attacker_shipp->ai_index];

	// MWA -- 2/27/98.  Due to AL's changes, target_objnum is now not always valid (at least sometimes
	// in terms of goals).  So, bail if we don't have a valid target.
	if ( aip->target_objnum == -1 )
		return 0;

	attacked_objp = &Objects[aip->target_objnum];
	shipp = &Ships[attacked_objp->instance];		//  need to get our target's ship pointer!!!

	ssp = ship_get_indexed_subsys(shipp, subnum, &objp->pos);
	if (ssp == NULL)
		return 0;

	set_targeted_subsys(aip, ssp, aip->target_objnum);
	
	if (aip->ignore_objnum == aip->target_objnum)
		aip->ignore_objnum = UNUSED_OBJNUM;

	// Goober5000
	if ((temp = find_ignore_new_object_index(aip, aip->target_objnum)) >= 0)
	{
		aip->ignore_new_objnums[temp] = UNUSED_OBJNUM;
	}
	else if (is_ignore_object(aip, aip->target_objnum, 1))
	{
		aip->ignore_objnum = UNUSED_OBJNUM;
	}

	// -- Done at caller in ai_process_mission_orders -- attacked_objp->flags |= OF_PROTECTED;

	ai_set_goal_maybe_abort_dock(objp, aip);
	aip->ok_to_target_timestamp = timestamp(DELAY_TARGET_TIME);

	return 1;
}

void ai_set_guard_vec(object *objp, object *guard_objp)
{
	ai_info *aip;
	float	radius;

	aip = &Ai_info[Ships[objp->instance].ai_index];

	//	Handle case of bogus call in which ship is told to guard self.
	Assert(objp != guard_objp);
	if (objp == guard_objp) {
		vm_vec_rand_vec_quick(&aip->guard_vec);
		vm_vec_scale(&aip->guard_vec, 100.0f);
		return;
	}

	// check if guard_objp is BIG
	radius = 5.0f * (objp->radius + guard_objp->radius) + 50.0f;
	if (radius > 300.0f) {
		radius = guard_objp->radius * 1.25f;
	}

	vm_vec_sub(&aip->guard_vec, &objp->pos, &guard_objp->pos);

	if (vm_vec_mag(&aip->guard_vec) > 3.0f*radius) {
		//	Far away, don't just use vector to object, causes clustering of guard ships.
		vec3d	tvec, rvec;
		float	mag;
		mag = vm_vec_copy_normalize(&tvec, &aip->guard_vec);
		vm_vec_rand_vec_quick(&rvec);			
		vm_vec_scale_add2(&tvec, &rvec, 0.5f);
		vm_vec_copy_scale(&aip->guard_vec, &tvec, mag);
	}

	vm_vec_normalize_quick(&aip->guard_vec);
	vm_vec_scale(&aip->guard_vec, radius);
}

//	Make object *objp guard object *other_objp.
//	To be called from the goals code.
void ai_set_guard_wing(object *objp, int wingnum)
{
	ship		*shipp;
	ai_info	*aip;
	int		leader_objnum, leader_shipnum;

	Assert(wingnum >= 0);

	Assert(objp->type == OBJ_SHIP);
	Assert(objp->instance >= 0);

	// shouldn't set the ai mode for the player
	if ( objp == Player_obj ) {
		return;
	}

	shipp = &Ships[objp->instance];

	Assert(shipp->ai_index >= 0);

	aip = &Ai_info[shipp->ai_index];
	force_avoid_player_check(objp, aip);

	ai_set_goal_maybe_abort_dock(objp, aip);
	aip->ok_to_target_timestamp = timestamp(DELAY_TARGET_TIME);

	//	This function is called whenever a guarded ship is destroyed, so this code
	//	prevents a ship from trying to guard a non-existent wing.
	if (Wings[wingnum].current_count < 1) {
		aip->guard_objnum = -1;
		aip->guard_wingnum = -1;
		aip->mode = AIM_NONE;
	} else {
		leader_shipnum = Wings[wingnum].ship_index[0];
		leader_objnum = Ships[leader_shipnum].objnum;

		Assert((leader_objnum >= 0) && (leader_objnum < MAX_OBJECTS));
		
		if (leader_objnum == OBJ_INDEX(objp)) {
			return;
		}

		aip->guard_wingnum = wingnum;
		aip->guard_objnum = leader_objnum;
		aip->guard_signature = Objects[leader_objnum].signature;
		aip->mode = AIM_GUARD;
		aip->submode = AIS_GUARD_STATIC;
		aip->submode_start_time = Missiontime;

		ai_set_guard_vec(objp, &Objects[leader_objnum]);
	}
}

//	Make object *objp guard object *other_objp.
//	To be called from the goals code.
void ai_set_evade_object(object *objp, object *other_objp)
{
	ship		*shipp;
	ai_info	*aip;
	int		other_objnum;

	Assert(objp->type == OBJ_SHIP);
	Assert(objp->instance >= 0);

	shipp = &Ships[objp->instance];

	Assert(shipp->ai_index >= 0);

	aip = &Ai_info[shipp->ai_index];

	other_objnum = OBJ_INDEX(other_objp);
	Assert(other_objnum >= 0);

	Assert(other_objnum != Ships[aip->shipnum].objnum);	//	make sure not targeting self
	aip->target_objnum = other_objnum;

	aip->mode = AIM_EVADE;
}

//	Make objp guard other_objp
//	If other_objp is a member of a wing, objp will guard that whole wing
//	UNLESS objp is also a member of the wing!
void ai_set_guard_object(object *objp, object *other_objp)
{
	ship		*shipp;
	ai_info	*aip;
	int		other_objnum;

	Assert(objp->type == OBJ_SHIP);
	Assert(objp->instance >= 0);
	Assert(objp != other_objp);

	shipp = &Ships[objp->instance];

	Assert(shipp->ai_index >= 0);

	aip = &Ai_info[shipp->ai_index];
	aip->avoid_check_timestamp = timestamp(1);

	//	If ship to guard is in a wing, guard that whole wing.
	ai_info	*other_aip = &Ai_info[Ships[other_objp->instance].ai_index];
	if ((other_aip->wing != -1) && (other_aip->wing != aip->wing)) {
		ai_set_guard_wing(objp, Ai_info[Ships[other_objp->instance].ai_index].wing);
	} else {

		other_objnum = other_objp-Objects;

		aip->guard_objnum = other_objnum;
		aip->guard_signature = other_objp->signature;
		aip->guard_wingnum = -1;

		aip->mode = AIM_GUARD;
		aip->submode = AIS_GUARD_STATIC;
		aip->submode_start_time = Missiontime;

		Assert(other_objnum >= 0);	//	Hmm, bogus object and we need its position for guard_vec.

		ai_set_guard_vec(objp, &Objects[other_objnum]);

		ai_set_goal_maybe_abort_dock(objp, aip);
		aip->ok_to_target_timestamp = timestamp(DELAY_TARGET_TIME);
	}
}

//	Update the aspect_locked_time field based on whether enemy is in view cone.
//	Also set/clear AIF_SEEK_LOCK.
void update_aspect_lock_information(ai_info *aip, vec3d *vec_to_enemy, float dist_to_enemy, float enemy_radius)
{
	float	dot_to_enemy;
	int	num_weapon_types;
	int	weapon_id_list[MAX_WEAPON_TYPES], weapon_bank_list[MAX_WEAPON_TYPES];
	ship	*shipp;
	ship	*tshpp;
	ship_weapon	*swp;
	weapon_info	*wip;
	object *tobjp = &Objects[aip->target_objnum];
	
	shipp = &Ships[aip->shipnum];
	tshpp = NULL;
	swp = &shipp->weapons;

	object *aiobjp = &Objects[shipp->objnum];

	// AL 3-7-98: This probably should never happen, but check to ensure that current_secondary_bank is valid
	if ( (swp->current_secondary_bank < 0) || (swp->current_secondary_bank > swp->num_secondary_banks) ) {
		return;
	}

	if (tobjp->type == OBJ_SHIP) {
		tshpp = &Ships[tobjp->instance];
	}

	num_weapon_types = get_available_secondary_weapons(Pl_objp, weapon_id_list, weapon_bank_list);

	wip = &Weapon_info[swp->secondary_bank_weapons[swp->current_secondary_bank]];

	if (num_weapon_types && (wip->wi_flags & WIF_LOCKED_HOMING)) {
		if (dist_to_enemy > 300.0f - MIN(enemy_radius, 100.0f))
			aip->ai_flags |= AIF_SEEK_LOCK;
		else
			aip->ai_flags &= ~AIF_SEEK_LOCK;

		//	Update locking information for aspect seeking missiles.
		aip->current_target_is_locked = 0;
		dot_to_enemy = vm_vec_dot(vec_to_enemy, &aiobjp->orient.vec.fvec);

		float	needed_dot = 0.9f - 0.5f * enemy_radius/(dist_to_enemy + enemy_radius);	//	Replaced MIN_TRACKABLE_DOT with 0.9f
		if (dot_to_enemy > needed_dot &&
			(wip->wi_flags & WIF_HOMING_ASPECT ||
			wip->wi_flags & WIF_HOMING_JAVELIN &&
			(tshpp == NULL ||
			ship_get_closest_subsys_in_sight(tshpp, SUBSYSTEM_ENGINE, &aiobjp->pos)))) {
				aip->aspect_locked_time += flFrametime;
				if (aip->aspect_locked_time >= wip->min_lock_time) {
					aip->aspect_locked_time = wip->min_lock_time;
					aip->current_target_is_locked = 1;
				}
		} else {
			aip->aspect_locked_time -= flFrametime*2;
			if (aip->aspect_locked_time < 0.0f)
				aip->aspect_locked_time = 0.0f;
		}
	
	} else {
		aip->current_target_is_locked = 0;
		aip->aspect_locked_time = 0.0f; // Used to be this, why?: wip->min_lock_time;
		aip->ai_flags &= ~AIF_SEEK_LOCK;
	}

}

//	We're in chase mode and we've recently collided with our target.
//	Fly away from it!
void ai_chase_fly_away(object *objp, ai_info *aip)
{
	int	abort_flag = 0;

	if (aip->ai_flags & AIF_TARGET_COLLISION) {
		aip->ai_flags &= ~AIF_TARGET_COLLISION;	//	Don't process this hit again next frame.
		aip->submode = SM_FLY_AWAY;					//	Focus on avoiding target
		aip->submode_start_time = Missiontime;
	}

	if ((aip->target_objnum == -1) || (Objects[aip->target_objnum].signature != aip->target_signature)) {
		abort_flag = 1;
	}

	if (abort_flag || (Missiontime > aip->submode_start_time + F1_0)) {
		aip->last_attack_time = Missiontime;
		aip->submode = SM_ATTACK;
		aip->submode_start_time = Missiontime;
	} else {
		vec3d	v2e;
		float		dot;

		vm_vec_normalized_dir(&v2e, &Objects[aip->target_objnum].pos, &objp->pos);

		dot = vm_vec_dot(&objp->orient.vec.fvec, &v2e);
		if (dot < 0.0f)
			accelerate_ship(aip, 1.0f);
		else
			accelerate_ship(aip, 1.0f - dot);
		turn_away_from_point(objp, &Objects[aip->target_objnum].pos, 0.0f);
	}
}

//	Return bank index of favored secondary weapon.
//	Return -1 if nothing favored.
//	"favored" means SEXPs have specified the weapon as being good to fire at en_objp.
int has_preferred_secondary(object *objp, object *en_objp, ship_weapon *swp)
{
	int	i;

	for (i=0; i<swp->num_secondary_banks; i++) {
		if (swp->secondary_bank_capacity[i] > 0) {
			if (swp->secondary_bank_ammo[i] > 0) {
				if (is_preferred_weapon(swp->secondary_bank_weapons[i], objp, en_objp) != -1){
					return i;
				}
			}
		}
	}

	return -1;
}

//	Choose which secondary weapon to fire.
//	Note, this is not like ai_select_secondary_weapon().  "choose" means make a choice.
//	"select" means execute an order.  Get it?
//	This function calls ai_select_secondary_weapon() with the characteristics it should search for.
void ai_choose_secondary_weapon(object *objp, ai_info *aip, object *en_objp)
{
	float			subsystem_strength = 0.0f;
	int			is_big_ship, wif_priority1, wif_priority2, wif2_priority1, wif2_priority2;
	ship_weapon	*swp;
	ship_info	*esip;

	if ( en_objp->type == OBJ_SHIP ) {
		esip = &Ship_info[Ships[en_objp->instance].ship_info_index];
	} else {
		esip = NULL;
	}

	swp = &Ships[objp->instance].weapons;

	// AL 3-5-98: do a quick out if the ship has no secondaries
	if ( swp->num_secondary_banks <= 0 ) {
		swp->current_secondary_bank = -1;
		return;
	}

	int preferred_secondary = has_preferred_secondary(objp, en_objp, swp);

	if (preferred_secondary != -1) {
		if (swp->current_secondary_bank != preferred_secondary) {
			aip->current_target_is_locked = 0;
			aip->aspect_locked_time = 0.0f;
			swp->current_secondary_bank = preferred_secondary;
		}
		aip->ai_flags |= AIF_UNLOAD_SECONDARIES;
	} else {
		aip->ai_flags &= ~AIF_UNLOAD_SECONDARIES;
		if (aip->targeted_subsys) {
			subsystem_strength = aip->targeted_subsys->current_hits;
		}

		if ( esip ) {
			is_big_ship = esip->flags & (SIF_HUGE_SHIP | SIF_BIG_SHIP);
		} else {
			is_big_ship=0;
		}

		if (is_big_ship)
		{
			wif_priority1 = WIF_HUGE;
			wif_priority2 = (aip->ai_profile_flags & AIPF_SMART_SECONDARY_WEAPON_SELECTION) ? WIF_BOMBER_PLUS : WIF_HOMING;
			wif2_priority1 = WIF2_CAPITAL_PLUS;
			wif2_priority2 = 0;
		} 
		else if ( (esip != NULL) && (esip->flags & SIF_BOMBER) )
		{
			wif_priority1 = WIF_BOMBER_PLUS;
			wif_priority2 = WIF_HOMING;
			wif2_priority1 = 0;
			wif2_priority2 = 0;
		} 
		else if (subsystem_strength > 100.0f)
		{
			wif_priority1 = WIF_PUNCTURE;
			wif_priority2 = WIF_HOMING;
			wif2_priority1 = 0;
			wif2_priority2 = 0;
		}
		else if ((aip->ai_profile_flags & AIPF_SMART_SECONDARY_WEAPON_SELECTION) && (en_objp->type == OBJ_ASTEROID))	//prefer dumbfires if its an asteroid	
		{	
			wif_priority1 = 0;								
			wif_priority2 = 0;
			wif2_priority1 = 0;
			wif2_priority2 = 0;
		} 
		else
		{
			wif_priority1 = WIF_HOMING;
			wif_priority2 = 0;
			wif2_priority1 = 0;
			wif2_priority2 = 0;
		}
		
		ai_select_secondary_weapon(objp, swp, wif_priority1, wif_priority2, wif2_priority1, wif2_priority2);
	}
}

/**
 * Return time, in seconds, at which this ship can next fire its current secondary weapon.
 */
float set_secondary_fire_delay(ai_info *aip, ship *shipp, weapon_info *swip, bool burst)
{
	float t;
	if (burst) {
		t = swip->burst_delay;
	} else {
		t = swip->fire_wait;		//	Base delay for this weapon.
	}
	if (shipp->team == Player_ship->team) {
		t *= aip->ai_ship_fire_secondary_delay_scale_friendly;
	} else {
		t *= aip->ai_ship_fire_secondary_delay_scale_hostile;
	}

	if (aip->ai_class_autoscale)
		t += (Num_ai_classes - aip->ai_class + 1) * 0.5f;

	t *= frand_range(0.8f, 1.2f);

	//	For the missiles that fire fairly quickly, occasionally add an additional substantial delay.
	if (t < 5.0f)
		if (frand() < 0.5f)
			t = t * 2.0f + 2.0f;

	return t;
}


void ai_chase_big_approach_set_goal(vec3d *goal_pos, object *attack_objp, object *target_objp, float *accel)
{
	float dist_to_goal;

	// head straight toward him and maybe circle later
	vm_vec_avg(goal_pos, &attack_objp->pos, &target_objp->pos);

	// get distance to goal
	dist_to_goal = vm_vec_dist(goal_pos, &attack_objp->pos);
	
	// set accel
	if (dist_to_goal > 400.0f) {
		*accel = 1.0f;
	} else {
		*accel = dist_to_goal/400.0f;
	}
}

void ai_chase_big_circle_set_goal(vec3d *goal_pos, object *attack_objp, object *target_objp, float *accel)
{
	get_tangent_point(goal_pos, attack_objp, &target_objp->pos, attack_objp->radius + target_objp->radius + 100.0f);

	*accel = 1.0f;
}

/**
 * Get the current and desired horizontal separations between target
 */
void ai_chase_big_get_separations(object *attack_objp, object *target_objp, vec3d *horz_vec_to_target, float *desired_separation, float *cur_separation)
{
	float temp, r_target, r_attacker, h_attacker, h_target;
	float perp_dist;
	vec3d vec_to_target;

	// get parameters of ships (as cylinders - radius and height)
	polymodel *pm = model_get(Ship_info[Ships[attack_objp->instance].ship_info_index].model_num);

	// get radius of attacker (for rotations about forward)
	temp = MAX(pm->maxs.xyz.x, pm->maxs.xyz.y);
	r_attacker = MAX(-pm->mins.xyz.x, -pm->mins.xyz.y);
	r_attacker = MAX(temp, r_attacker);
	h_attacker = MAX(-pm->mins.xyz.z, pm->maxs.xyz.z);

	// get radius of target (for rotations about forward)
	temp = MAX(pm->maxs.xyz.x, pm->maxs.xyz.y);
	r_target = MAX(-pm->mins.xyz.x, -pm->mins.xyz.y);
	r_target = MAX(temp, r_target);
	h_target = MAX(-pm->mins.xyz.z, pm->maxs.xyz.z);

	// find separation between cylinders [if parallel]
	vm_vec_sub(&vec_to_target, &attack_objp->pos, &target_objp->pos);

	// find the distance between centers along forward direction of ships
	perp_dist = vm_vec_dotprod(&vec_to_target, &target_objp->orient.vec.fvec);

	// subtract off perp component to get "horizontal" separation vector between cylinders [ASSUMING parallel]
	vm_vec_scale_add(horz_vec_to_target, &vec_to_target, &target_objp->orient.vec.fvec, -perp_dist);
	*cur_separation = vm_vec_mag_quick(horz_vec_to_target);

	// choose "optimal" separation of 1000 + r_target + r_attacker
	*desired_separation = 1000 + r_target + r_attacker;
}

void ai_chase_big_parallel_set_goal(vec3d *goal_pos, object *attack_objp, object *target_objp, float *accel)
{
	int opposing;
	float temp, r_target, r_attacker, h_attacker, h_target;
	float separation, optimal_separation;
	vec3d  horz_vec_to_target;

	// get parameters of ships (as cylinders - radius and height)
	polymodel *pm = model_get(Ship_info[Ships[attack_objp->instance].ship_info_index].model_num);

	// get radius of attacker (for rotations about forward)
	temp = MAX(pm->maxs.xyz.x, pm->maxs.xyz.y);
	r_attacker = MAX(-pm->mins.xyz.x, -pm->mins.xyz.y);
	r_attacker = MAX(temp, r_attacker);
	h_attacker = MAX(-pm->mins.xyz.z, pm->maxs.xyz.z);

	// get radius of target (for rotations about forward)
	temp = MAX(pm->maxs.xyz.x, pm->maxs.xyz.y);
	r_target = MAX(-pm->mins.xyz.x, -pm->mins.xyz.y);
	r_target = MAX(temp, r_target);
	h_target = MAX(-pm->mins.xyz.z, pm->maxs.xyz.z);

	// are we opposing (only when other ship is not moving)
	opposing = ( vm_vec_dotprod(&attack_objp->orient.vec.fvec, &target_objp->orient.vec.fvec) < 0 );

	ai_chase_big_get_separations(attack_objp, target_objp, &horz_vec_to_target, &optimal_separation, &separation);

	// choose dist (2000) so that we don't bash
	float dist = 2000;
	if (opposing) {
		dist = - dist;
	}

	// set the goal pos as dist forward from target along target forward
	vm_vec_scale_add(goal_pos, &target_objp->pos, &target_objp->orient.vec.fvec, dist);
	// then add horizontal separation
	vm_vec_scale_add2(goal_pos, &horz_vec_to_target, optimal_separation/separation);

	// find the distance between centers along forward direction of ships
	vec3d vec_to_target;
	vm_vec_sub(&vec_to_target, &target_objp->pos, &attack_objp->pos);
	float perp_dist = vm_vec_dotprod(&vec_to_target, &target_objp->orient.vec.fvec);

	float match_accel = 0.0f;
	float length_scale = attack_objp->radius;

	if (Ship_info[Ships[attack_objp->instance].ship_info_index].max_vel.xyz.z != 0.0f) {
		match_accel = target_objp->phys_info.vel.xyz.z / Ship_info[Ships[attack_objp->instance].ship_info_index].max_vel.xyz.z;
	}

	// if we're heading toward enemy ship, we want to keep going if we're ahead
	if (opposing) {
		perp_dist = -perp_dist;
	}

	if (perp_dist > 0) {
		// falling behind, so speed up
		*accel = match_accel + (1.0f - match_accel) / length_scale * (perp_dist);
	} else {
		// up in front, so slow down
		*accel = match_accel  - match_accel / length_scale * -perp_dist;
		*accel = MAX(0.0f, *accel);
	}

}

//	Return *goal_pos for one cruiser to attack another (big ship).
//	Choose point fairly nearby that is not occupied by another cruiser.
void ai_cruiser_chase_set_goal_pos(vec3d *goal_pos, object *pl_objp, object *en_objp)
{
	ai_info *aip;

	aip = &Ai_info[Ships[pl_objp->instance].ai_index];
	float accel;

	switch (aip->submode) {
	case SM_BIG_APPROACH:
		// do approach stuff;
		ai_chase_big_approach_set_goal(goal_pos, pl_objp, en_objp, &accel);
		break;

	case SM_BIG_CIRCLE:
		// do circle stuff
		ai_chase_big_circle_set_goal(goal_pos, pl_objp, en_objp, &accel);
		break;

	case SM_BIG_PARALLEL:
		// do parallel stuff
		ai_chase_big_parallel_set_goal(goal_pos, pl_objp, en_objp, &accel);
		break;
	}
}

int maybe_hack_cruiser_chase_abort()
{
	ship			*shipp = &Ships[Pl_objp->instance];	
	ship			*eshipp = &Ships[En_objp->instance];
	ai_info		*aip = &Ai_info[shipp->ai_index];

	// mission sm3-08, sathanos chasing collosus
	if ( stricmp(Mission_filename, "sm3-08.fs2") == 0 ) {
		if (( stricmp(eshipp->ship_name, "colossus") == 0 ) || ( stricmp(shipp->ship_name, "colossus") == 0 )) {
			// Changed so all big ships attacking the Colossus will not do the chase code.
			// Did this so Beast wouldn't swerve away from Colossus. -- MK, 9/14/99
			ai_clear_ship_goals( aip );
			aip->mode = AIM_NONE;
			return 1;
		}
	}

	return 0;
}

//	Make a big ship pursue another big ship.
//	(Note, called "ai_cruiser_chase" because we already have ai_chase_big() which means fighter chases big ship.
void ai_cruiser_chase()
{
	ship			*shipp = &Ships[Pl_objp->instance];	
	ai_info		*aip = &Ai_info[shipp->ai_index];

	if (En_objp->type != OBJ_SHIP) {
		Int3();
		return;
	}

	if (En_objp->instance < 0) {
		Int3();
		return;
	}

	ship			*eshipp;
	ship_info	*esip;

	eshipp = &Ships[En_objp->instance];
	esip = &Ship_info[eshipp->ship_info_index];

	vec3d	goal_pos;
	float turn_time = Ship_info[Ships[Pl_objp->instance].ship_info_index].srotation_time;

	// kamikaze - ram and explode
	if (aip->ai_flags & AIF_KAMIKAZE) {
		ai_turn_towards_vector(&En_objp->pos, Pl_objp, flFrametime, turn_time, NULL, NULL, 0.0f, 0);
		accelerate_ship(aip, 1.0f);
	} 
	
	// really track down and chase
	else {
		// check valid submode
		Assert( (aip->submode == SM_ATTACK) || (aip->submode == SM_BIG_APPROACH) || (aip->submode == SM_BIG_CIRCLE) || (aip->submode == SM_BIG_PARALLEL) );

		// just entering, approach enemy ship
		if (aip->submode == SM_ATTACK) {
			aip->submode = SM_BIG_APPROACH;
			aip->submode_start_time = Missiontime;
		}

		// desired accel
		float accel = 0.0f;
		vec3d *rvecp = NULL;

		switch (aip->submode) {
		case SM_BIG_APPROACH:
			// do approach stuff;
			ai_chase_big_approach_set_goal(&goal_pos, Pl_objp, En_objp, &accel);
			// maybe set rvec
			break;

		case SM_BIG_CIRCLE:
			// do circle stuff
			ai_chase_big_circle_set_goal(&goal_pos, Pl_objp, En_objp, &accel);
			// maybe set rvec
			break;

		case SM_BIG_PARALLEL:
			// do parallel stuff
			ai_chase_big_parallel_set_goal(&goal_pos, Pl_objp, En_objp, &accel);
			//maybe set rvec
			break;
		}

		// now move as desired
		ai_turn_towards_vector(&goal_pos, Pl_objp, flFrametime, turn_time, NULL, NULL, 0.0f, 0, rvecp);
		accelerate_ship(aip, accel);

		// maybe switch to new mode
		vec3d vec_to_enemy;
		float dist_to_enemy;
		int moving = (En_objp->phys_info.vel.xyz.z > 0.5f);
		vm_vec_sub(&vec_to_enemy, &En_objp->pos, &Pl_objp->pos);
		dist_to_enemy = vm_vec_mag_quick(&vec_to_enemy);

		switch (aip->submode) {
		case SM_BIG_APPROACH:
			if ( dist_to_enemy < (Pl_objp->radius + En_objp->radius)*1.25f + 200.0f ) {
				// moving
				if (moving) {
					// if within 90 degrees of en forward, go into parallel, otherwise circle
					if ( vm_vec_dotprod(&En_objp->orient.vec.fvec, &Pl_objp->orient.vec.fvec) > 0 ) {
						aip->submode = SM_BIG_PARALLEL;
						aip->submode_start_time = Missiontime;
					}
				}

				// otherwise cirle
				if ( !maybe_hack_cruiser_chase_abort() ) {
					aip->submode = SM_BIG_CIRCLE;
					aip->submode_start_time = Missiontime;
				}
			}
			break;

		case SM_BIG_CIRCLE:
			// moving
			if (moving) {
				vec3d temp;
				float desired_sep, cur_sep;
				// we're behind the enemy ship
				if (vm_vec_dotprod(&vec_to_enemy, &En_objp->orient.vec.fvec) > 0) {
					// and we're turning toward the enemy
					if (vm_vec_dotprod(&En_objp->orient.vec.fvec, &Pl_objp->orient.vec.fvec) > 0) {
						// get separation
						ai_chase_big_get_separations(Pl_objp, En_objp, &temp, &desired_sep, &cur_sep);
						// and the separation is > 0.9 desired
						if (cur_sep > (0.9f * desired_sep)) {
							aip->submode = SM_BIG_PARALLEL;
							aip->submode_start_time = Missiontime;
						}
					}
				}
			} else {
				// still
				vec3d temp;
				float desired_sep, cur_sep;
				// we're behind the enemy ship
				if (vm_vec_dotprod(&vec_to_enemy, &En_objp->orient.vec.fvec) > 0) {
					// and we're turning toward the enemy
					if (vm_vec_dotprod(&En_objp->orient.vec.fvec, &Pl_objp->orient.vec.fvec) > 0) {
						// get separation
						ai_chase_big_get_separations(Pl_objp, En_objp, &temp, &desired_sep, &cur_sep);
						// and the separation is > 0.9 desired
						if (cur_sep > (0.9f * desired_sep)) {
							aip->submode = SM_BIG_PARALLEL;
							aip->submode_start_time = Missiontime;
						}
					}
				}
				// in front of ship
				else {
					// and we're turning toward the enemy
					if (vm_vec_dotprod(&En_objp->orient.vec.fvec, &Pl_objp->orient.vec.fvec) < 0) {
						// get separation
						ai_chase_big_get_separations(Pl_objp, En_objp, &temp, &desired_sep, &cur_sep);
						// and the separation is > 0.9 desired
						if (cur_sep > (0.9f * desired_sep)) {
							aip->submode = SM_BIG_PARALLEL;
							aip->submode_start_time = Missiontime;
						}
					}
				}
			}
			break;

		case SM_BIG_PARALLEL:
			// we're opposing
			if ( vm_vec_dotprod(&Pl_objp->orient.vec.fvec, &En_objp->orient.vec.fvec) < 0 ) {
				// and the other ship is moving
				if (moving) {
					// and we no longer overlap
					if ( dist_to_enemy > (0.75 * (En_objp->radius + Pl_objp->radius)) ) {
						aip->submode = SM_BIG_APPROACH;
						aip->submode_start_time = Missiontime;
					}
				}
			}
			break;
		}
	}
}

/**
 * Make object Pl_objp chase object En_objp
 */
void ai_chase()
{
	float		dist_to_enemy, time_to_enemy;
	float		dot_to_enemy, dot_from_enemy, real_dot_to_enemy;
	vec3d		player_pos, enemy_pos, predicted_enemy_pos, real_vec_to_enemy, predicted_vec_to_enemy;
	ship		*shipp = &Ships[Pl_objp->instance];
	ship_info	*sip = &Ship_info[shipp->ship_info_index];
	ship_weapon	*swp = &shipp->weapons;
	ai_info		*aip = &Ai_info[shipp->ai_index];
	int			enemy_sip_flags, enemy_shipp_flags2;
	int has_fired = -1;

	if (aip->mode != AIM_CHASE) {
		Int3();
	}

	// by default we try to chase anything
	bool go_after_it = true;

	if ( (sip->class_type > -1) && (En_objp->type == OBJ_SHIP) )
	{
		// default to not chasing for ships
		go_after_it = false;
		ship_info *esip = &Ship_info[Ships[En_objp->instance].ship_info_index];
		if (esip->class_type > -1)
		{
			ship_type_info *stp = &Ship_types[sip->class_type];
			size_t ap_size = stp->ai_actively_pursues.size();
			for(size_t i = 0; i < ap_size; i++)
			{
				if(stp->ai_actively_pursues[i] == esip->class_type) {
					go_after_it = true;
					break;
				}
			}
		}
		else
		{
			// if there was no class type then assume we can go after it ...
			go_after_it = true;
			// ... but also log this in debug so it doesn't go unchecked (NOTE that this can completely flood a debug log!)
			mprintf(("AI-WARNING: No class_type specified for '%s', assuming that it's ok to chase!\n", esip->name));
		}
	}

	//WMC - Guess we do need this
	if (!go_after_it) {
		aip->mode = AIM_NONE;
		return;
	}

	if (sip->class_type > -1 && (Ship_types[sip->class_type].ai_bools & STI_AI_ATTEMPT_BROADSIDE)) {
		ai_cruiser_chase();
		return;
	}

	Assert( En_objp != NULL );

	if ( En_objp->type == OBJ_SHIP ) {
		enemy_sip_flags = Ship_info[Ships[En_objp->instance].ship_info_index].flags;
		enemy_shipp_flags2 = Ships[En_objp->instance].flags2;
	} else {
		enemy_sip_flags = 0;
		enemy_shipp_flags2 = 0;
	}

	if ( enemy_sip_flags > 0 ) {
		if (enemy_sip_flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {
			ai_big_chase();
			return;
		}
	}

	//	If collided with target_objnum last frame, avoid that ship.
	//	This should prevent the embarrassing behavior of ships getting stuck on each other
	//	as if they were magnetically attracted. -- MK, 11/13/97.
	if ((aip->ai_flags & AIF_TARGET_COLLISION) || (aip->submode == SM_FLY_AWAY)) {
		ai_chase_fly_away(Pl_objp, aip);
		return;
	}

	ai_set_positions(Pl_objp, En_objp, aip, &player_pos, &enemy_pos);
	dist_to_enemy = vm_vec_dist_quick(&player_pos, &enemy_pos);
	time_to_enemy = compute_time_to_enemy(dist_to_enemy, Pl_objp, En_objp);
	vm_vec_sub(&real_vec_to_enemy, &enemy_pos, &player_pos);

	//Enemy position for the purpose of aiming is already calculated differently, do it explicitly here
	ai_update_aim(aip, En_objp);

	vm_vec_normalize(&real_vec_to_enemy);

	real_dot_to_enemy = vm_vec_dot(&real_vec_to_enemy, &Pl_objp->orient.vec.fvec);

	int is_stealthy_ship = 0;
	if ( (enemy_sip_flags > 0) && (enemy_shipp_flags2 & SF2_STEALTH) ) {
		if ( ai_is_stealth_visible(Pl_objp, En_objp) != STEALTH_FULLY_TARGETABLE ) {
			is_stealthy_ship = 1;
		}
	}

	// Can only acquire lock on a target that isn't hidden from sensors
	if ( !(Ships[En_objp->instance].flags & SF_HIDDEN_FROM_SENSORS) && !is_stealthy_ship ) {
		update_aspect_lock_information(aip, &real_vec_to_enemy, dist_to_enemy, En_objp->radius);
	} else {
		aip->current_target_is_locked = 0;
		aip->ai_flags &= ~AIF_SEEK_LOCK;
	}

	//	If seeking lock, try to point directly at ship, else predict position so lasers can hit it.
	//	If just acquired target, or target is not in reasonable cone, don't refine believed enemy position.
	if ((real_dot_to_enemy < 0.25f) || (aip->target_time < 1.0f) || (aip->ai_flags & AIF_SEEK_LOCK)) {
		predicted_enemy_pos = enemy_pos;
	} else {
		//	Set predicted_enemy_pos.
		//	See if attacking a subsystem.
		if (aip->targeted_subsys != NULL) {
			Assert(En_objp->type == OBJ_SHIP);
			if (get_shield_pct(En_objp) < HULL_DAMAGE_THRESHOLD_PERCENT) {
				if (aip->targeted_subsys != NULL) {
					get_subsystem_pos(&enemy_pos, En_objp, aip->targeted_subsys);
					predicted_enemy_pos = enemy_pos;
					predicted_vec_to_enemy = real_vec_to_enemy;
				} else {
					set_predicted_enemy_pos(&predicted_enemy_pos, Pl_objp, &aip->last_aim_enemy_pos, &aip->last_aim_enemy_vel, aip);
					set_target_objnum(aip, -1);
				}

			} else {
				set_predicted_enemy_pos(&predicted_enemy_pos, Pl_objp, &aip->last_aim_enemy_pos, &aip->last_aim_enemy_vel, aip);
			}
		} else {
			set_predicted_enemy_pos(&predicted_enemy_pos, Pl_objp, &aip->last_aim_enemy_pos, &aip->last_aim_enemy_vel, aip);
		}
	}

	vm_vec_sub(&predicted_vec_to_enemy, &predicted_enemy_pos, &player_pos);

	vm_vec_normalize(&predicted_vec_to_enemy);

	dot_to_enemy = vm_vec_dot(&Pl_objp->orient.vec.fvec, &predicted_vec_to_enemy);
	dot_from_enemy= - vm_vec_dot(&En_objp->orient.vec.fvec, &real_vec_to_enemy);

	//	Set turn and acceleration based on submode.
	switch (aip->submode) {
	case SM_CONTINUOUS_TURN:
		ai_chase_ct();
		break;

	case SM_STEALTH_FIND:
		ai_stealth_find();
		break;

	case SM_STEALTH_SWEEP:
		ai_stealth_sweep();
		break;

	case SM_ATTACK:
	case SM_SUPER_ATTACK:
	case SM_ATTACK_FOREVER:
	case AIS_CHASE_GLIDEATTACK:
	case AIS_CHASE_CIRCLESTRAFE:
		if (vm_vec_dist_quick(&Pl_objp->pos, &predicted_enemy_pos) > 100.0f + En_objp->radius * 2.0f) {
			if (maybe_avoid_big_ship(Pl_objp, En_objp, aip, &predicted_enemy_pos, 10.0f))
				return;
		}

		ai_chase_attack(aip, sip, &predicted_enemy_pos, dist_to_enemy, sip->model_num);
		break;

	case SM_EVADE_SQUIGGLE:
		ai_chase_es(aip, sip);
		break;

	case SM_EVADE_BRAKE:
		ai_chase_eb(aip, sip, &predicted_enemy_pos, dist_to_enemy);
		break;

	case SM_EVADE:
		evade_ship();
		break;

	case SM_AVOID:
		avoid_ship();
		break;

	case SM_GET_BEHIND:
		get_behind_ship(aip, sip, dist_to_enemy);
		break;

	case SM_GET_AWAY:		//	Used to get away from opponent to prevent endless circling.
		ai_chase_ga(aip, sip);
		break;

	case SM_EVADE_WEAPON:
		evade_weapon();
		break;

	default:
		aip->last_attack_time = Missiontime;
		aip->submode = SM_ATTACK;
		aip->submode_start_time = Missiontime;
	}

	//Maybe apply random sidethrust, depending on the current submode
	//The following are valid targets for random sidethrust (circle strafe uses it too, but that is handled separately)
	if (aip->submode == SM_ATTACK ||
		aip->submode == SM_SUPER_ATTACK ||
		aip->submode == SM_EVADE_SQUIGGLE ||
		aip->submode == SM_EVADE ||
		aip->submode == SM_GET_AWAY ||
		aip->submode == AIS_CHASE_GLIDEATTACK)
	{
		//Re-roll for random sidethrust every 2 seconds
		//Also, only do this if we've recently been hit or are in current primary weapon range of target
		float current_weapon_range = Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]].weapon_range;
		if (static_randf((Missiontime + static_rand(aip->shipnum)) >> 17) < aip->ai_random_sidethrust_percent &&
			((Missiontime - aip->last_hit_time < i2f(5) && aip->hitter_objnum >= 0) || dist_to_enemy < current_weapon_range)) 
		{
			do_random_sidethrust(aip, sip);
		}
	}

	//	Maybe choose a new submode.
	if ( (aip->submode != SM_AVOID) && (aip->submode != SM_ATTACK_FOREVER) ) {
		//	If a very long time since attacked, attack no matter what!
		if (Missiontime - aip->last_attack_time > i2f(6)) {
			if ( (aip->submode != SM_SUPER_ATTACK) && (aip->submode != SM_GET_AWAY) && !(aip->ai_flags & AIF_STEALTH_PURSUIT) ) {
				aip->submode = SM_SUPER_ATTACK;
				aip->submode_start_time = Missiontime;
				aip->last_attack_time = Missiontime;
			}
		}

		//SUSHI: Alternate stalemate dection method: if nobody has hit each other for a while
		//(and we've been near that whole time), shake things up somehow
		//Only do this if stalemate time threshold > 0
		if (aip->ai_stalemate_time_thresh > 0.0f &&
				Missiontime - aip->last_hit_target_time > fl2f(aip->ai_stalemate_time_thresh) && 
				Missiontime - aip->last_hit_time > fl2f(aip->ai_stalemate_time_thresh) &&
				aip->time_enemy_near > aip->ai_stalemate_time_thresh &&
				(dot_to_enemy < 0.95f - 0.5f * En_objp->radius/MAX(1.0f, En_objp->radius + dist_to_enemy)))
		{
			//Every second, evaluate whether or not to break stalemate. The more patient the ship, the less likely this is.
			if (static_randf((Missiontime + static_rand(aip->shipnum)) >> 16) > (aip->ai_patience * .01))
			{
				if ((sip->can_glide == true) && (frand() < aip->ai_glide_attack_percent)) {
					//Maybe use glide attack
					aip->submode = AIS_CHASE_GLIDEATTACK;
					aip->submode_start_time = Missiontime;
					aip->last_hit_target_time = Missiontime;
					aip->time_enemy_near = 0.0f;
				} else {
					//Otherwise, try to get away
					aip->submode = SM_GET_AWAY;
					aip->submode_start_time = Missiontime;
					aip->last_hit_target_time = Missiontime;
					aip->time_enemy_near = 0.0f;
				}
			}
		}

		//	If a collision is expected, pull out!
		//	If enemy is pointing away and moving a bit, don't worry about collision detection.
		if ((dot_from_enemy > 0.5f) || (En_objp->phys_info.speed < 10.0f)) {
			//If we're in circle strafe mode, don't worry about colliding with the target
			if (aip->submode != AIS_CHASE_CIRCLESTRAFE && might_collide_with_ship(Pl_objp, En_objp, dot_to_enemy, dist_to_enemy, 4.0f)) {
				if ((Missiontime - aip->last_hit_time > F1_0*4) && (dist_to_enemy < Pl_objp->radius*2 + En_objp->radius*2)) {
					accelerate_ship(aip, -1.0f);
				} else {
					aip->submode = SM_AVOID;
					aip->submode_start_time = Missiontime;
				}
			}
		}
	}

	switch (aip->submode) {
	case SM_CONTINUOUS_TURN:
		if (Missiontime - aip->submode_start_time > i2f(3)) {
			aip->last_attack_time = Missiontime;
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
		}
		break;

	case SM_ATTACK:
		// if target is stealth and stealth not visible, then enter stealth find mode
		if ( (aip->ai_flags & AIF_STEALTH_PURSUIT) && (ai_is_stealth_visible(Pl_objp, En_objp) == STEALTH_INVISIBLE) ) {
			aip->submode = SM_STEALTH_FIND;
			aip->submode_start_time = Missiontime;
			aip->submode_parm0 = SM_SF_AHEAD;
		} else if (dist_to_enemy < CIRCLE_STRAFE_MAX_DIST + En_objp->radius &&
			(En_objp->phys_info.speed < MAX(sip->max_vel.xyz.x, sip->max_vel.xyz.y) * 1.5f) &&
			(static_randf((Missiontime + static_rand(aip->shipnum)) >> 19) < aip->ai_circle_strafe_percent)) {
			aip->submode = AIS_CHASE_CIRCLESTRAFE;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;		
		} else if (ai_near_full_strength(Pl_objp) && (Missiontime - aip->last_hit_target_time > i2f(3)) && (dist_to_enemy < 500.0f) && (dot_to_enemy < 0.5f)) {
			aip->submode = SM_SUPER_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
		} else if ((Missiontime - aip->last_hit_target_time > i2f(6)) &&
			(dist_to_enemy < 500.0f) && (dot_to_enemy < 0.2f) &&
			(frand() < (float) Game_skill_level/NUM_SKILL_LEVELS)) {
			aip->submode = SM_GET_AWAY;
			aip->submode_start_time = Missiontime;
			aip->last_hit_target_time = Missiontime;
		} else if ((enemy_sip_flags & SIF_SMALL_SHIP)
			&& (dot_to_enemy < dot_from_enemy)
			&& (En_objp->phys_info.speed > 15.0f) 
			&& (dist_to_enemy < 200.0f) 
			&& (dist_to_enemy > 50.0f)
			&& (dot_to_enemy < 0.1f)
			&& (Missiontime - aip->submode_start_time > i2f(2))) {
			aip->submode = SM_EVADE_BRAKE;
			aip->submode_start_time = Missiontime;
		} else if ((dot_to_enemy > 0.2f) && (dot_from_enemy > -0.2f) && (dot_from_enemy < 0.1f)) {
			aip->submode = SM_GET_BEHIND;
			aip->submode_start_time = Missiontime;
		} else if ((enemy_sip_flags & SIF_SMALL_SHIP) && (dist_to_enemy < 150.0f) && (dot_from_enemy > dot_to_enemy + 0.5f + aip->ai_courage*.002)) {
			float get_away_chance = (aip->ai_get_away_chance == FLT_MIN)
				? (float)(aip->ai_class + Game_skill_level)/(Num_ai_classes + NUM_SKILL_LEVELS)
				: aip->ai_get_away_chance;
			if ((Missiontime - aip->last_hit_target_time > i2f(5)) && (frand() < get_away_chance)) {
				aip->submode = SM_GET_AWAY;
				aip->submode_start_time = Missiontime;
				aip->last_hit_target_time = Missiontime;
			} else {
				aip->submode = SM_EVADE_SQUIGGLE;
				aip->submode_start_time = Missiontime;
			}
		} else if ((enemy_sip_flags & SIF_SMALL_SHIP) && (Missiontime - aip->submode_start_time > F1_0*2)) {
			if ((dot_to_enemy < 0.8f) && (dot_from_enemy > dot_to_enemy)) {
				if (frand() > 0.5f) {
					aip->submode = SM_CONTINUOUS_TURN;
					aip->submode_start_time = Missiontime;
					aip->submode_parm0 = myrand() & 0x0f;
				} else {
					aip->submode = SM_EVADE;
					aip->submode_start_time = Missiontime;
				}
			} else {
				aip->submode_start_time = Missiontime;
			}
		}

		aip->last_attack_time = Missiontime;

		break;
		
	case SM_EVADE_SQUIGGLE:
		if ((Missiontime - aip->submode_start_time > i2f(5)) || (dist_to_enemy > 300.0f)) {
			if ((dist_to_enemy < 100.0f) && (dot_to_enemy < 0.0f) && (dot_from_enemy > 0.5f)) {
				aip->submode = SM_EVADE_BRAKE;
			} else if ((Pl_objp->phys_info.speed >= Pl_objp->phys_info.max_vel.xyz.z / 2.0f) && (sip->can_glide == true) && (frand() < aip->ai_glide_attack_percent)) {
				aip->last_attack_time = Missiontime;
				aip->submode = AIS_CHASE_GLIDEATTACK;
			} else {
				aip->last_attack_time = Missiontime;
				aip->submode = SM_ATTACK;
			}
			aip->submode_start_time = Missiontime;
		}
		break;
	
	case SM_EVADE_BRAKE:
		if ((dist_to_enemy < 15.0f) || (En_objp->phys_info.speed < 10.0f)) {
			aip->submode = SM_AVOID;
			aip->submode_start_time = Missiontime;
		} else if ((dot_to_enemy > 0.9f) || ((dot_from_enemy > 0.9f) && (Missiontime - aip->submode_start_time > i2f(1)))) {
			aip->last_attack_time = Missiontime;
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
		} else if (Missiontime - aip->submode_start_time > i2f(4)) {
			aip->last_attack_time = Missiontime;
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
		}
		break;

	case SM_EVADE:
		//	Modified by MK on 5/5/97 to keep trying to regain attack mode.  It's what a human would do.
		if ((dot_to_enemy < 0.2f) && (dot_from_enemy < 0.8f) && (dist_to_enemy < 100.0f) && (En_objp->phys_info.speed > 15.0f)) {
			aip->last_attack_time = Missiontime;
			aip->submode = SM_EVADE_BRAKE;
			aip->submode_start_time = Missiontime;
		} else if (((dot_to_enemy > dot_from_enemy - 0.1f)
			&& (Missiontime > aip->submode_start_time + i2f(1)))
			|| (dist_to_enemy > 150.0f + 2*(Pl_objp->radius + En_objp->radius))) {
			aip->last_attack_time = Missiontime;
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
		} else if (Missiontime - aip->submode_start_time > i2f(2))
			if (dot_from_enemy > 0.8f) {
				aip->submode = SM_EVADE_SQUIGGLE;
				aip->submode_start_time = Missiontime;
			}

		break;

	case SM_SUPER_ATTACK:
		// if stealth and invisible, enter stealth find mode
		if ( (aip->ai_flags & AIF_STEALTH_PURSUIT) && (ai_is_stealth_visible(Pl_objp, En_objp) == STEALTH_INVISIBLE) ) {
			aip->submode = SM_STEALTH_FIND;
			aip->submode_start_time = Missiontime;
			aip->submode_parm0 = SM_SF_AHEAD;
		} else if (dist_to_enemy < CIRCLE_STRAFE_MAX_DIST + En_objp->radius &&
			(En_objp->phys_info.speed < MAX(sip->max_vel.xyz.x, sip->max_vel.xyz.y) * 1.5f) &&
			(static_randf((Missiontime + static_rand(aip->shipnum)) >> 19) < aip->ai_circle_strafe_percent)) {
			aip->submode = AIS_CHASE_CIRCLESTRAFE;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;		
		} else if ((dist_to_enemy < 100.0f) && (dot_to_enemy < 0.8f) && (enemy_sip_flags & SIF_SMALL_SHIP) && (Missiontime - aip->submode_start_time > i2f(5) )) {
			aip->ai_flags &= ~AIF_ATTACK_SLOWLY;	//	Just in case, clear here.

			float get_away_chance = (aip->ai_get_away_chance == FLT_MIN)
				? (float)(aip->ai_class + Game_skill_level)/(Num_ai_classes + NUM_SKILL_LEVELS)
				: aip->ai_get_away_chance;

			switch (myrand() % 5) {
			case 0:
				aip->submode = SM_CONTINUOUS_TURN;
				aip->submode_start_time = Missiontime;
				break;
			case 1:
				aip->submode_start_time = Missiontime;	//	Stay in super attack mode
				break;
			case 2:
			case 3:
				if (frand() < (float) 0.5f * get_away_chance) {
					aip->submode = SM_GET_AWAY;
					aip->submode_start_time = Missiontime;
				} else {
					aip->submode = SM_EVADE;
					aip->submode_start_time = Missiontime;
				}
				break;
			case 4:
				if (dot_from_enemy + (NUM_SKILL_LEVELS - Game_skill_level) * 0.1f > dot_to_enemy) {	//	Less likely to GET_AWAY at lower skill levels.
					aip->submode = SM_EVADE;
					aip->submode_start_time = Missiontime;
				} else {
					aip->submode = SM_GET_AWAY;
					aip->submode_start_time = Missiontime;
				}
				break;
			default:
				Int3();	//	Impossible!
			}
		}

		aip->last_attack_time = Missiontime;

		break;

	case SM_AVOID:
		if ((dot_to_enemy > -0.2f) && (dist_to_enemy / (dot_to_enemy + 0.3f) < 100.0f)) {
			aip->submode_start_time = Missiontime;
		} else if (Missiontime - aip->submode_start_time > i2f(1)/2)
			if (might_collide_with_ship(Pl_objp, En_objp, dot_to_enemy, dist_to_enemy, 3.0f)) {
				aip->submode_start_time = Missiontime;
			} else {
				aip->submode = SM_GET_BEHIND;
				aip->submode_start_time = Missiontime;
			}

		break;

	case SM_GET_BEHIND:
		if ((dot_from_enemy < -0.7f) || (Missiontime - aip->submode_start_time > i2f(2))) {
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
		}
		break;

	case SM_GET_AWAY:
		if (Missiontime - aip->submode_start_time > i2f(2)) {
			float	rand_dist;

			rand_dist = ((Missiontime >> 17) & 0x03) * 100.0f + 200.0f;	//	Some value in 200..500
			if ((Missiontime - aip->submode_start_time > i2f(5)) || (dist_to_enemy > rand_dist) || (dot_from_enemy < 0.4f)) {
				//Sometimes use a glide attack instead (if we can)
				if ((sip->can_glide == true) && (frand() < aip->ai_glide_attack_percent)) {
					aip->submode = AIS_CHASE_GLIDEATTACK;
				}
				else {
					aip->ai_flags |= AIF_ATTACK_SLOWLY;
					aip->submode = SM_ATTACK;
				}

				aip->submode_start_time = Missiontime;
				aip->time_enemy_in_range = 2.0f;		//	Cheat.  Presumably if they were running away from you, they were monitoring you!
				aip->last_attack_time = Missiontime;
			}
		}
		break;

	case SM_EVADE_WEAPON:
		if (aip->danger_weapon_objnum == -1) {
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
		}
		break;

	// Either change to SM_ATTACK or AIM_FIND_STEALTH
	case SM_STEALTH_FIND:
		// if time > 5 sec change mode to sweep
		if ( !(aip->ai_flags & AIF_STEALTH_PURSUIT) || (ai_is_stealth_visible(Pl_objp, En_objp) == STEALTH_VISIBLE) ) {
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
			// sweep if I can't find in 5 sec or bail from find
		} else if ( ((Missiontime - aip->submode_start_time) > i2f(5)) || (aip->submode_parm0 == SM_SF_BAIL) ) {
			// begin sweep mode
			aip->submode = SM_STEALTH_SWEEP;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
			aip->submode_parm0 = SM_SS_SET_GOAL;
		}
		break;

	case SM_STEALTH_SWEEP:
		if ( !(aip->ai_flags & AIF_STEALTH_PURSUIT) || (ai_is_stealth_visible(Pl_objp, En_objp) == STEALTH_VISIBLE) ) {
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
		} else if ( (timestamp() - aip->stealth_last_visible_stamp) < 5000 ) {
			// go back to find mode
			aip->submode = SM_STEALTH_FIND;
			aip->submode_start_time = Missiontime;
			aip->submode_parm0 = SM_SF_AHEAD;
		} else if ( aip->submode_parm0 == SM_SS_DONE ) {
			// set target objnum = -1
			set_target_objnum(aip, -1);

			// set submode to attack
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
		}
		break;

	case SM_ATTACK_FOREVER:	//	Engines blown, just attack.
		break;

	case AIS_CHASE_GLIDEATTACK:
		//Glide attack lasts at least as long as it takes to turn around and fire for a couple seconds. 
		if (Missiontime - aip->submode_start_time > i2f((int)(sip->rotation_time.xyz.x) + 4 + static_rand_range(Missiontime >> 19, 0, 3))) {
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
		}
		aip->last_attack_time = Missiontime;
		break;

	case AIS_CHASE_CIRCLESTRAFE:
		//Break out of circle strafe if the target is too far, moving too fast, or we've been doing this for a while
		if ((dist_to_enemy > CIRCLE_STRAFE_MAX_DIST + En_objp->radius) || 
			(Missiontime - aip->submode_start_time > i2f(8)) || 
			(En_objp->phys_info.speed > MAX(sip->max_vel.xyz.x, sip->max_vel.xyz.y) * 1.5)) {
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
		}
		aip->last_attack_time = Missiontime;
		break;

	default:
		aip->submode = SM_ATTACK;
		aip->submode_start_time = Missiontime;
		aip->last_attack_time = Missiontime;
	}


	//Update time enemy near
	//Ignore for stealth ships that can't be seen
	//If we're trying to get away, recent counter
	//Only do this if stalemate distance threshold > 0
	if (aip->ai_stalemate_dist_thresh > 0.0f &&
			dist_to_enemy < aip->ai_stalemate_dist_thresh && 
			aip->submode != SM_GET_AWAY && aip->submode != AIS_CHASE_GLIDEATTACK && aip->submode != SM_FLY_AWAY && 
			(!(aip->ai_flags & AIF_STEALTH_PURSUIT) || (ai_is_stealth_visible(Pl_objp, En_objp) == STEALTH_VISIBLE)))
	{
		aip->time_enemy_near += flFrametime;
	}
	else
	{
		aip->time_enemy_near *= (1.0f - flFrametime);
		if (aip->time_enemy_near < 0.0f)
			aip->time_enemy_near = 0.0f;
	}

	//	Maybe fire primary weapon and update time_enemy_in_range
	if (aip->mode != AIM_EVADE) {
		if (dot_to_enemy > 0.95f - 0.5f * En_objp->radius/MAX(1.0f, En_objp->radius + dist_to_enemy)) {
			aip->time_enemy_in_range += flFrametime;
			
			//	Chance of hitting ship is based on dot product of firing ship's forward vector with vector to ship
			//	and also the size of the target relative to distance to target.
			if (dot_to_enemy > MAX(0.5f, 0.90f + aip->ai_accuracy/10.0f - En_objp->radius/MAX(1.0f,dist_to_enemy))) {

				ship *temp_shipp;
				ship_weapon *tswp;

				temp_shipp = &Ships[Pl_objp->instance];
				tswp = &temp_shipp->weapons;
				if ( tswp->num_primary_banks > 0 ) {
					float	scale;
					Assert(tswp->current_primary_bank < tswp->num_primary_banks);
					weapon_info	*pwip = &Weapon_info[tswp->primary_bank_weapons[tswp->current_primary_bank]];

					//	Less likely to fire if far away and moving.
					scale = pwip->max_speed/(En_objp->phys_info.speed + pwip->max_speed);
					if (scale > 0.6f)
						scale = (scale - 0.6f) * 1.5f;
					else
						scale = 0.0f;
					if (dist_to_enemy < pwip->max_speed * (1.0f + scale)) {
						if(ai_fire_primary_weapon(Pl_objp) == 1){
							has_fired = 1;
						}else{
							has_fired = -1;
						}
					}
				}

				if ( tswp->num_secondary_banks > 0) {

					//	Don't fire secondaries at a protected ship.
					if (!(En_objp->flags & OF_PROTECTED)) {
						ai_choose_secondary_weapon(Pl_objp, aip, En_objp);
						int current_bank = tswp->current_secondary_bank;
						weapon_info	*swip = &Weapon_info[tswp->secondary_bank_weapons[tswp->current_secondary_bank]];

						if (current_bank > -1) {
							if (aip->ai_flags & AIF_UNLOAD_SECONDARIES) {
								if (timestamp_until(swp->next_secondary_fire_stamp[current_bank]) > swip->fire_wait*1000.0f) {
									swp->next_secondary_fire_stamp[current_bank] = timestamp((int) (swip->fire_wait*1000.0f));
								}
							}

							if (timestamp_elapsed(swp->next_secondary_fire_stamp[current_bank])) {
								if (tswp->current_secondary_bank >= 0) {
									float firing_range;
									
									if (swip->wi_flags2 & WIF2_LOCAL_SSM)
										firing_range=swip->lssm_lock_range;		//that should be enough
									else if (swip->wi_flags & WIF_BOMB)
										firing_range = MIN((swip->max_speed * swip->lifetime * 0.75f), swip->weapon_range);
									else
									{
										float secondary_range_mult = (aip->ai_secondary_range_mult == FLT_MIN)
											? (float)(Game_skill_level + 1 + (3 * aip->ai_class/(Num_ai_classes - 1)))/NUM_SKILL_LEVELS
											: aip->ai_secondary_range_mult;

										firing_range = MIN((swip->max_speed * swip->lifetime * secondary_range_mult), swip->weapon_range);
									}

									
									// reduce firing range in nebula
									extern int Nebula_sec_range;
									if ((The_mission.flags & MISSION_FLAG_FULLNEB) && Nebula_sec_range) {
										firing_range *= 0.8f;
									}

									//	If firing a spawn weapon, distance doesn't matter.
									int	spawn_fire = 0;

									if (swip->wi_flags & WIF_SPAWN) {
										int	count;

										count = num_nearby_fighters(iff_get_attackee_mask(obj_team(Pl_objp)), &Pl_objp->pos, 1000.0f);

										if (count > 3)
											spawn_fire = 1;
										else if (count >= 1) {
											float hull_percent = Pl_objp->hull_strength/temp_shipp->ship_max_hull_strength;

											if (hull_percent < 0.01f)
												hull_percent = 0.01f;

											if (frand() < 0.25f/(30.0f*hull_percent) * count)	//	With timestamp below, this means could fire in 30 seconds if one enemy.
												spawn_fire = 1;
										}
									}

									if (spawn_fire || (dist_to_enemy < firing_range)) {
										if (ai_fire_secondary_weapon(Pl_objp)) {
											//	Only if weapon was fired do we specify time until next fire.  If not fired, done in ai_fire_secondary...
											float t;
											
											if ((aip->ai_flags & AIF_UNLOAD_SECONDARIES) || (swip->burst_flags & WBF_FAST_FIRING)) {
												if (swip->burst_shots > swp->burst_counter[current_bank]) {
													t = swip->burst_delay;
													swp->burst_counter[current_bank]++;
												} else {
													t = swip->fire_wait;
													if ((swip->burst_shots > 0) && (swip->burst_flags & WBF_RANDOM_LENGTH)) {
														swp->burst_counter[current_bank] = myrand() % swip->burst_shots;
													} else {
														swp->burst_counter[current_bank] = 0;
													}
												}
											} else {
												if (swip->burst_shots > swp->burst_counter[current_bank]) {
													t = set_secondary_fire_delay(aip, temp_shipp, swip, true);
													swp->burst_counter[current_bank]++;
												} else {
													t = set_secondary_fire_delay(aip, temp_shipp, swip, false);
													if ((swip->burst_shots > 0) && (swip->burst_flags & WBF_RANDOM_LENGTH)) {
														swp->burst_counter[current_bank] = myrand() % swip->burst_shots;
													} else {
														swp->burst_counter[current_bank] = 0;
													}
												}
											}
											swp->next_secondary_fire_stamp[current_bank] = timestamp((int) (t*1000.0f));
										}
									} else {
										swp->next_secondary_fire_stamp[current_bank] = timestamp(250);
									}
								}
							}
						}
					}
				}
			}
		} else {
			if (flFrametime < 1.0f)
				aip->time_enemy_in_range *= (1.0f - flFrametime);
			else
				aip->time_enemy_in_range = 0;
		}
	} else {
		if (flFrametime < 1.0f)
			aip->time_enemy_in_range *= (1.0f - flFrametime);
		else
			aip->time_enemy_in_range = 0;
	}

	if(has_fired == -1){
		ship_stop_fire_primary(Pl_objp);
	}

}

/**
 * Make the object *objp move so that the point *start moves towards the point *finish.
 * @return distance.
 */
float dock_move_towards_point(object *objp, vec3d *start, vec3d *finish, float speed_scale, float other_obj_speed = 0.0f, rotating_dockpoint_info *rdinfo = NULL)
{
	physics_info	*pi = &objp->phys_info;
	float			dist;			//	dist to goal
	vec3d			v2g;			//	vector to goal

	dist = vm_vec_dist_quick(start, finish);
	if (dist > 0.0f) {
		float	speed;

		dist = vm_vec_normalized_dir(&v2g, finish, start);
		speed = fl_sqrt(dist) * speed_scale;

		// Goober5000 - if we're on a rotating submodel and we're rotating with it, adjust velocity for rotation
		if (rdinfo && (rdinfo->submodel >= 0))
		{
			speed *= 1.25f;

			switch (rdinfo->dock_mode)
			{
				case DOA_APPROACH:
					// ramp submodel's linear velocity according to distance
					speed += (rdinfo->submodel_r * rdinfo->submodel_w) / (dist);
					break;

				case DOA_DOCK:
				case DOA_UNDOCK_1:
					// use docker's linear velocity and don't ramp
					speed += (vm_vec_dist(&rdinfo->submodel_pos, &objp->pos) * rdinfo->submodel_w);
					break;
			}
		}

		if (other_obj_speed < MAX_REPAIR_SPEED*0.75f)
			speed += other_obj_speed;
		else
			speed += MAX_REPAIR_SPEED*0.75f;

		vm_vec_copy_scale(&pi->desired_vel, &v2g, speed);
	} else
		vm_vec_zero(&pi->desired_vel);

	return dist;
}

//	Set the orientation in the global reference frame for an object to attain
//	to dock with another object.  Resultant global matrix returned in dom.
// Revised by Goober5000
void set_goal_dock_orient(matrix *dom, vec3d *docker_p0, vec3d *docker_p1, vec3d *docker_p0_norm, matrix *docker_orient, vec3d *dockee_p0, vec3d *dockee_p1, vec3d *dockee_p0_norm, matrix *dockee_orient)
{
	vec3d	fvec, uvec, temp;
	matrix	m1, m2, m3;

	//	Compute the global orientation of the dockee's docking bay.

	// get the rotated (local) fvec
	vm_vec_rotate(&fvec, dockee_p0_norm, dockee_orient);
	vm_vec_negate(&fvec);

	// get the rotated (local) uvec
	vm_vec_normalized_dir(&temp, dockee_p1, dockee_p0);
	vm_vec_rotate(&uvec, &temp, dockee_orient);

	// create a rotation matrix
	vm_vector_2_matrix(&m1, &fvec, &uvec, NULL);

	// get the global orientation
	vm_matrix_x_matrix(&m3, dockee_orient, &m1);

	//	Compute the matrix given by the docker's docking bay.

	// get the rotated (local) fvec
	vm_vec_rotate(&fvec, docker_p0_norm, docker_orient);

	// get the rotated (local) uvec
	vm_vec_normalized_dir(&temp, docker_p1, docker_p0);
	vm_vec_rotate(&uvec, &temp, docker_orient);

	// create a rotation matrix
	vm_vector_2_matrix(&m2, &fvec, &uvec, NULL);

	//	Pre-multiply the orientation of the source object (docker_orient) by the transpose
	//	of the docking bay's orientation, ie unrotate the source object's matrix.
	vm_transpose_matrix(&m2);
	vm_matrix_x_matrix(dom, &m3, &m2);
}

// Goober5000
// Return the rotating submodel on which is mounted the specified dockpoint, or -1 for none.
int find_parent_rotating_submodel(polymodel *pm, int dock_index)
{
	int path_num, submodel;

	// make sure we have a spline path to check against before going any further
	if (pm->docking_bays[dock_index].num_spline_paths <= 0)
	{
		return -1;
	}

	// find a path for this dockpoint (c.f. ai_return_path_num_from_dockbay)
	path_num = pm->docking_bays[dock_index].splines[0];

	// path must exist
	if ((path_num >= 0) && (path_num < pm->n_paths))
	{
		// find the submodel for the path for this dockpoint
		submodel = pm->paths[path_num].parent_submodel;

		// submodel must exist and must move
		if ((submodel >= 0) && (submodel < pm->n_models) && (pm->submodel[submodel].movement_type >= 0))
		{
			return submodel;
		}
	}

	// if path doesn't exist or the submodel doesn't exist or the submodel doesn't move
	return -1;
}

// Goober5000
void find_adjusted_dockpoint_info(vec3d *global_p0, vec3d *global_p1, vec3d *global_p0_norm, object *objp, polymodel *pm, int modelnum, int submodel, int dock_index)
{
	// are we basing this off a rotating submodel?
	if (submodel >= 0)
	{
		vec3d submodel_offset;
		vec3d local_p0, local_p1;

		ship_model_start(objp);

		// calculate the dockpoint locations relative to the unrotated submodel
		model_find_submodel_offset(&submodel_offset, modelnum, submodel);
		vm_vec_sub(&local_p0, &pm->docking_bays[dock_index].pnt[0], &submodel_offset);
		vm_vec_sub(&local_p1, &pm->docking_bays[dock_index].pnt[1], &submodel_offset);

		// find the dynamic positions of the dockpoints
		model_find_world_point(global_p0, &local_p0, modelnum, submodel, &objp->orient, &objp->pos);
		model_find_world_point(global_p1, &local_p1, modelnum, submodel, &objp->orient, &objp->pos);

		// find the normal of the first dockpoint
		model_find_world_dir(global_p0_norm, &pm->docking_bays[dock_index].norm[0], modelnum, submodel, &objp->orient, &objp->pos);

		ship_model_stop(objp);
	}
	// use the static dockpoints
	else
	{
		vm_vec_unrotate(global_p0, &pm->docking_bays[dock_index].pnt[0], &objp->orient);
		vm_vec_add2(global_p0, &objp->pos);

		vm_vec_unrotate(global_p1, &pm->docking_bays[dock_index].pnt[1], &objp->orient);
		vm_vec_add2(global_p1, &objp->pos);

		vm_vec_unrotate(global_p0_norm, &pm->docking_bays[dock_index].norm[0], &objp->orient);
	}
}

#define	DOCK_BACKUP_RETURN_VAL	99999.9f

//	Make docker_objp dock with dockee_objp
//	Returns distance to goal, defined as distance between corresponding dock points, plus 10.0f * rotational velocity vector (DOA_DOCK only)
//	DOA_APPROACH	means	approach point aip->path_cur
//	DOA_DOCK			means dock
//	DOA_UNDOCK_1	means undock, moving to point nearest dock bay
//	DOA_UNDOCK_2	means undock, moving to point nearest dock bay and facing away from ship
//	DOA_UNDOCK_3	means undock, moving directly away from ship
//	DOA_DOCK_STAY	means rigidly maintain position in dock bay.
float dock_orient_and_approach(object *docker_objp, int docker_index, object *dockee_objp, int dockee_index, int dock_mode, rotating_dockpoint_info *rdinfo)
{
	ship_info	*sip0, *sip1;
	polymodel	*pm0, *pm1;
	ai_info		*aip;
	matrix		dom, nm;
	vec3d docker_p0, docker_p1, docker_p0_norm;
	vec3d dockee_p0, dockee_p1, dockee_p0_norm;
	vec3d docker_point, dockee_point;
	float			fdist = UNINITIALIZED_VALUE;


	docker_objp->phys_info.forward_thrust = 0.0f;		//	Kill thrust so we don't have a sputtering thruster.

	// Goober5000 - moved out here to save calculations
	if (dock_mode != DOA_DOCK_STAY)
		if (ship_get_subsystem_strength(&Ships[docker_objp->instance], SUBSYSTEM_ENGINE) <= 0.0f)
			return 9999.9f;

	aip = &Ai_info[Ships[docker_objp->instance].ai_index];

	sip0 = &Ship_info[Ships[docker_objp->instance].ship_info_index];
	sip1 = &Ship_info[Ships[dockee_objp->instance].ship_info_index];
	pm0 = model_get( sip0->model_num );
	pm1 = model_get( sip1->model_num );

	Assert( docker_index >= 0 );
	Assert( dockee_index >= 0 );

	Assert(pm0->docking_bays[docker_index].num_slots == 2);
	Assert(pm1->docking_bays[dockee_index].num_slots == 2);


	// Goober5000 - check if we're attached to a rotating submodel
	int dockee_rotating_submodel = find_parent_rotating_submodel(pm1, dockee_index);

	// Goober5000 - move docking points with submodels if necessary, for both docker and dockee
	find_adjusted_dockpoint_info(&docker_p0, &docker_p1, &docker_p0_norm, docker_objp, pm0, sip0->model_num, -1, docker_index);
	find_adjusted_dockpoint_info(&dockee_p0, &dockee_p1, &dockee_p0_norm, dockee_objp, pm1, sip1->model_num, dockee_rotating_submodel, dockee_index);

	// Goober5000 - find average of point
	vm_vec_avg(&docker_point, &docker_p0, &docker_p1);
	vm_vec_avg(&dockee_point, &dockee_p0, &dockee_p1);

	// Goober5000
	vec3d submodel_pos = ZERO_VECTOR;
	float submodel_radius = 0.0f;
	float submodel_omega = 0.0f;
	if ((dockee_rotating_submodel >= 0) && (dock_mode != DOA_DOCK_STAY))
	{
		vec3d submodel_offset;
		vec3d dockpoint_temp;

		ship_model_start(dockee_objp);

		// get submodel center
		model_find_submodel_offset(&submodel_offset, sip1->model_num, dockee_rotating_submodel);
		vm_vec_add(&submodel_pos, &dockee_objp->pos, &submodel_offset);

		// get angular velocity of dockpoint
		//WMC - hack(?) to fix bug where sii might not exist
		if(pm1->submodel[dockee_rotating_submodel].sii != NULL) {
			submodel_omega = pm1->submodel[dockee_rotating_submodel].sii->cur_turn_rate;
		}

		// get radius to dockpoint
		vm_vec_avg(&dockpoint_temp, &dockee_p0, &dockee_p1);
		submodel_radius = vm_vec_dist(&submodel_pos, &dockpoint_temp);

		ship_model_stop(dockee_objp);
	}

	// Goober5000
	rotating_dockpoint_info rdinfo_buf;
	if (rdinfo == NULL)
		rdinfo = &rdinfo_buf;

	rdinfo->docker_point = docker_point;
	rdinfo->dockee_point = dockee_point;
	rdinfo->dock_mode = dock_mode;
	rdinfo->submodel = dockee_rotating_submodel;
	rdinfo->submodel_pos = submodel_pos;
	rdinfo->submodel_r = submodel_radius;
	rdinfo->submodel_w = submodel_omega;


	//	If dockee has moved much, then path will be recreated.
	//	Might need to change state if moved too far.
	if ((dock_mode != DOA_DOCK_STAY) && (dock_mode != DOA_DOCK)) {
		// Goober5000 - maybe force recreate
		int force_recreate = (dockee_rotating_submodel >= 0) && ((dock_mode == DOA_APPROACH) || (dock_mode == DOA_UNDOCK_1));
	}


	float speed_scale = 1.0f;
	if (sip0->flags & SIF_SUPPORT) {
		speed_scale = 3.0f;
	}

	switch (dock_mode) {
	case DOA_APPROACH:
	{
		vec3d *goal_point;

		//	Compute the desired global orientation matrix for the docker's station.
		//	That is, the normal vector of the docking station must be the same as the
		//	forward vector and the vector between its two points must be the uvec.
		set_goal_dock_orient(&dom, &docker_p0, &docker_p1, &docker_p0_norm, &docker_objp->orient, &dockee_p0, &dockee_p1, &dockee_p0_norm, &dockee_objp->orient);

		//	Compute new orientation matrix and update rotational velocity.
		vec3d	omega_in, omega_out, vel_limit, acc_limit;
		float		tdist, mdist, ss1;

		omega_in = docker_objp->phys_info.rotvel;
		vel_limit = docker_objp->phys_info.max_rotvel;
		vm_vec_copy_scale(&acc_limit, &vel_limit, 0.3f);
		
		if (sip0->flags & SIF_SUPPORT)
			vm_vec_scale(&acc_limit, 2.0f);

		// 1 at end of line prevent overshoot
		vm_matrix_interpolate(&dom, &docker_objp->orient, &omega_in, flFrametime, &nm, &omega_out, &vel_limit, &acc_limit, 1);
		docker_objp->phys_info.rotvel = omega_out;
		docker_objp->orient = nm;

		//	Translate towards goal and note distance to goal.
		goal_point = &Path_points[aip->path_cur].pos;
		mdist = ai_matrix_dist(&docker_objp->orient, &dom);
		tdist = vm_vec_dist_quick(&docker_objp->pos, goal_point);

		//	If translation is badly lagging rotation, speed up translation.
		if (mdist > 0.1f) {
			ss1 = tdist/(10.0f * mdist);
			if (ss1 > 2.0f)
				ss1 = 2.0f;
		} else
			ss1 = 2.0f;

		// if we're docking to a rotating submodel, speed up translation
		if (dockee_rotating_submodel >= 0)
			ss1 = 2.0f;

		speed_scale *= 1.0f + ss1;

		fdist = dock_move_towards_point(docker_objp, &docker_objp->pos, goal_point, speed_scale, dockee_objp->phys_info.speed, rdinfo);

		//	Note, we're interested in distance from goal, so if we're still turning, bash that into return value.
		fdist += 2.0f * mdist;

		break;
	}
	case DOA_DOCK:
	case DOA_DOCK_STAY:
	{
		//	Compute the desired global orientation matrix for the docker's station.
		//	That is, the normal vector of the docking station must be the same as the
		//	forward vector and the vector between its two points must be the uvec.
		set_goal_dock_orient(&dom, &docker_p0, &docker_p1, &docker_p0_norm, &docker_objp->orient, &dockee_p0, &dockee_p1, &dockee_p0_norm, &dockee_objp->orient);

		//	Compute distance between dock bay points.
		if (dock_mode == DOA_DOCK) {
			vec3d	omega_in, omega_out, vel_limit, acc_limit;

			//	Compute new orientation matrix and update rotational velocity.
			omega_in = docker_objp->phys_info.rotvel;
			vel_limit = docker_objp->phys_info.max_rotvel;
			vm_vec_copy_scale(&acc_limit, &vel_limit, 0.3f);

			if (sip0->flags & SIF_SUPPORT)
				vm_vec_scale(&acc_limit, 2.0f);

			vm_matrix_interpolate(&dom, &docker_objp->orient, &omega_in, flFrametime, &nm, &omega_out, &vel_limit, &acc_limit);
			docker_objp->phys_info.rotvel = omega_out;
			docker_objp->orient = nm;

			fdist = dock_move_towards_point(docker_objp, &docker_point, &dockee_point, speed_scale, dockee_objp->phys_info.speed, rdinfo);
		
			//	Note, we're interested in distance from goal, so if we're still turning, bash that into return value.
			fdist += 10.0f * vm_vec_mag_quick(&omega_out);
		} else {
			vec3d offset;

			Assert(dock_mode == DOA_DOCK_STAY);
			docker_objp->orient = dom;

			vm_vec_sub(&offset, &dockee_point, &docker_point);
			vm_vec_add2(&docker_objp->pos, &offset);
		}

		break;
	}
	case DOA_UNDOCK_1:
	{
		//	Undocking.
		//	Move to point on dock path nearest to dock station.
		Assert(aip->path_length >= 2);
		fdist = dock_move_towards_point(docker_objp, &docker_objp->pos, &Path_points[aip->path_start + aip->path_length-2].pos, speed_scale, 0.0f, rdinfo);

		break;
	}
	case DOA_UNDOCK_2:
	{
		//	Undocking.
		//	Move to point on dock path nearest to dock station and orient away from big ship.
		int		desired_index;

		Assert(aip->path_length >= 2);
		
		desired_index = aip->path_length-2;
		fdist = dock_move_towards_point(docker_objp, &docker_objp->pos, &Path_points[aip->path_start + desired_index].pos, speed_scale, 0.0f, rdinfo);

		break;
	}
	case DOA_UNDOCK_3:
	{
		vec3d	goal_point;
		float		dist, goal_dist;
		vec3d	away_vec;

		goal_dist = docker_objp->radius + dockee_objp->radius + 25.0f;

		dist = vm_vec_normalized_dir(&away_vec, &docker_objp->pos, &dockee_objp->pos);
		vm_vec_scale_add(&goal_point, &dockee_objp->pos, &away_vec, goal_dist);
		if (vm_vec_dist_quick(&goal_point, &dockee_objp->pos) < vm_vec_dist_quick(&docker_objp->pos, &dockee_objp->pos))
			fdist = 0.0f;
		else
		{
			float	dot, accel;
			float turn_time = Ship_info[Ships[docker_objp->instance].ship_info_index].srotation_time;
			ai_turn_towards_vector(&goal_point, docker_objp, flFrametime, turn_time, NULL, NULL, 0.0f, 0);

			dot = vm_vec_dot(&docker_objp->orient.vec.fvec, &away_vec);
			accel = 0.1f;
			if (dot > accel)
				accel = dot;
			if (dist > goal_dist/2)
				accel *= 1.2f - 0.5f*goal_dist/dist;

			accelerate_ship(aip, accel);
			fdist = vm_vec_dist_quick(&docker_objp->pos, &goal_point);
		}

		break;
	}
	}

	return fdist;
}

void debug_find_guard_object()
{
	ship			*shipp = &Ships[Pl_objp->instance];	
	object		*objp;

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ((Pl_objp != objp) && (objp->type == OBJ_SHIP)) {
			if (objp->instance != -1) {
				if (Ships[objp->instance].team == shipp->team)	{
					ai_set_guard_object(Pl_objp, objp);
				}
			}
		}
	}

}

/**
 * Given an object number, return the number of ships attacking it.
 */
int num_ships_attacking(int target_objnum)
{
	object	*attacking_objp;
	ai_info	*attacking_aip;
	ship_obj	*so;
	int		count = 0;
	int target_team = Ships[Objects[target_objnum].instance].team;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) )
	{
		attacking_objp = &Objects[so->objnum];
		if (attacking_objp->instance < 0)
			continue;

		attacking_aip = &Ai_info[Ships[attacking_objp->instance].ai_index];

		// don't count instructor
		int is_training_mission();
		if (is_training_mission() && is_instructor(attacking_objp))
			continue;	// Goober5000 10/06/2005 changed from break

		if (iff_x_attacks_y(Ships[attacking_objp->instance].team, target_team))
		{
			if (attacking_aip->target_objnum == target_objnum)
			{
				if ( ((Game_mode & GM_MULTIPLAYER) && (attacking_objp->flags & OF_PLAYER_SHIP))
					|| (attacking_aip->mode == AIM_CHASE) )
				{
					count++;
				}
			}
		}
	}

	return count;
}

//	For all objects attacking object #objnum, remove the one that is farthest away.
//	Do this by resuming previous behavior, if any.  If not, set target_objnum to -1.
void remove_farthest_attacker(int objnum)
{
	object	*objp, *objp2, *farthest_objp;
	ship_obj	*so;
	float		farthest_dist;

	objp2 = &Objects[objnum];

	farthest_dist = 9999999.9f;
	farthest_objp = NULL;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];
		if ( !(objp->flags & OF_PLAYER_SHIP)) {
			if (objp->instance != -1) {
				ai_info	*aip2;

				aip2 = &Ai_info[Ships[objp->instance].ai_index];

				if ((aip2->mode == AIM_CHASE) && (aip2->target_objnum == objnum)) {
					if (iff_x_attacks_y(Ships[objp->instance].team, Ships[objp2->instance].team)) {
						float	dist;

						dist = vm_vec_dist_quick(&objp->pos, &objp2->pos);
						if (dist < farthest_dist) {
							farthest_dist = dist;
							farthest_objp = objp;
						}
					}
				}
			}
		}
	}

	if (farthest_objp != NULL) {
		ai_info	*aip;
		Assert(farthest_objp->type == OBJ_SHIP);
		Assert((farthest_objp->instance > -1) && (farthest_objp->instance < MAX_SHIPS));
		Assert(Ships[farthest_objp->instance].ai_index > -1);

		aip = &Ai_info[Ships[farthest_objp->instance].ai_index];

		if (!maybe_resume_previous_mode(Pl_objp, aip))
		{
			//	If already ignoring something under player's orders, don't ignore current target.
			if ((aip->ignore_objnum == UNUSED_OBJNUM) || (aip->ai_flags & AIF_TEMPORARY_IGNORE))
			{
				aip->ignore_objnum = aip->target_objnum;
				aip->ignore_signature = Objects[aip->target_objnum].signature;
				aip->ai_flags |= AIF_TEMPORARY_IGNORE;
				aip->ignore_expire_timestamp = timestamp(((myrand() % 10) + 20) * 1000);	//	OK to attack again in 20 to 24 seconds.
			}
			aip->target_objnum = -1;
			ai_do_default_behavior(farthest_objp);
		}
	}
}

// Maybe limit the number of attackers on attack_objnum.  For now, only limit attackers
// in attacked_objnum is the player
// input:	attacked_objnum	=>		object index for ship we want to limit attacks on
//
//	exit:			1	=>	num attackers exceeds maximum, abort
//					0	=>	removed the farthest attacker
//					-1	=>	nothing was done
int ai_maybe_limit_attackers(int attacked_objnum)
{
	int rval=-1;

	if ( Objects[attacked_objnum].flags & OF_PLAYER_SHIP) {
		int num_attacking;
		num_attacking = num_ships_attacking(attacked_objnum);

		if (num_attacking == The_mission.ai_profile->max_attackers[Game_skill_level]) {
			remove_farthest_attacker(attacked_objnum);
			rval=0;
		} else if (num_attacking > The_mission.ai_profile->max_attackers[Game_skill_level]) {
			rval=1;
		}
	}

	return rval;
}

/**
 * Object being guarded by object *guard_objp was hit by object *hitter_objp
 */
void guard_object_was_hit(object *guard_objp, object *hitter_objp)
{
	int		hitter_objnum;
	ai_info	*aip;

	aip = &Ai_info[Ships[guard_objp->instance].ai_index];

	if (guard_objp == hitter_objp) {
		return;
	}

	if (guard_objp->type == OBJ_GHOST || hitter_objp->type == OBJ_GHOST)
		return;

	if (aip->ai_flags & AIF_NO_DYNAMIC)	//	Not allowed to pursue dynamic goals.  So, why are we guarding?
		return;

	Assert( (hitter_objp->type == OBJ_SHIP) || (hitter_objp->type == OBJ_ASTEROID) || (hitter_objp->type == OBJ_WEAPON) );

	hitter_objnum = OBJ_INDEX(hitter_objp);

	if ( hitter_objp->type == OBJ_SHIP ) {
		//	If the hitter object is the ignore object, don't attack it.
		if (is_ignore_object(aip, OBJ_INDEX(hitter_objp)))
			return;

		//	If hitter is on same team as me, don't attack him.
		if (Ships[guard_objp->instance].team == Ships[hitter_objp->instance].team)
			return;

		// limit the number of ships attacking hitter_objnum (for now, only if hitter_objnum is player)
		if ( ai_maybe_limit_attackers(hitter_objnum) == 1 ) {
			return;
		}

		// don't attack if you can't see him
		if ( awacs_get_level(hitter_objp, &Ships[aip->shipnum], 1) < 1 ) {
			// if he's a stealth and visible, but not targetable, ok to attack.
			if ( is_object_stealth_ship(hitter_objp) ) {
				if ( ai_is_stealth_visible(guard_objp, hitter_objp) != STEALTH_VISIBLE ) {
					return;
				}
			}
		}
	}

	if (aip->target_objnum == -1) {
		aip->ok_to_target_timestamp = timestamp(0);
	}

	if ((aip->submode == AIS_GUARD_PATROL) || (aip->submode == AIS_GUARD_STATIC)) {

		if ( hitter_objp->type == OBJ_SHIP ) {
			if (!(Ship_info[Ships[guard_objp->instance].ship_info_index].flags & SIF_SMALL_SHIP)) {
				return;
			}

			// limit the number of ships attacking hitter_objnum (for now, only if hitter_objnum is player)
			if ( ai_maybe_limit_attackers(hitter_objnum) == 1 ) {
				return;
			}
		}

		if (aip->target_objnum != hitter_objnum) {
			aip->aspect_locked_time = 0.0f;
		}

		aip->ok_to_target_timestamp = timestamp(0);

		set_target_objnum(aip, hitter_objnum);
		aip->previous_mode = AIM_GUARD;
		aip->previous_submode = aip->submode;
		aip->mode = AIM_CHASE;
		aip->submode = SM_ATTACK;
		aip->submode_start_time = Missiontime;
		aip->active_goal = AI_ACTIVE_GOAL_DYNAMIC;
	} else if (aip->previous_mode == AIM_GUARD) {
		if (aip->target_objnum == -1) {

			if ( hitter_objp->type == OBJ_SHIP ) {
				// limit the number of ships attacking hitter_objnum (for now, only if hitter_objnum is player)
				if ( ai_maybe_limit_attackers(hitter_objnum) == 1 ) {
					return;
				}
			}

			set_target_objnum(aip, hitter_objnum);
			aip->mode = AIM_CHASE;
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->active_goal = AI_ACTIVE_GOAL_DYNAMIC;
		} else {
			int	num_attacking_cur, num_attacking_new;

			num_attacking_cur = num_ships_attacking(aip->target_objnum);
			if (num_attacking_cur > 1) {
				num_attacking_new = num_ships_attacking(hitter_objnum);

				if (num_attacking_new < num_attacking_cur) {

					if ( hitter_objp->type == OBJ_SHIP ) {
						// limit the number of ships attacking hitter_objnum (for now, only if hitter_objnum is player)
						if ( ai_maybe_limit_attackers(hitter_objnum) == 1 ) {
							return;
						}
					}
					set_target_objnum(aip, hitter_objp-Objects);
					aip->mode = AIM_CHASE;
					aip->submode = SM_ATTACK;
					aip->submode_start_time = Missiontime;
					aip->active_goal = AI_ACTIVE_GOAL_DYNAMIC;
				}
			}
		}
	}
}

//	Ship object *hit_objp was hit by ship object *hitter_objp.
//	See if anyone is guarding hit_objp and, if so, do something useful.
void maybe_update_guard_object(object *hit_objp, object *hitter_objp)
{
	object	*objp;
	ship_obj	*so;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];
		if (objp->instance != -1) {
			ai_info	*aip;
			aip = &Ai_info[Ships[objp->instance].ai_index];

			if ((aip->mode == AIM_GUARD) || (aip->active_goal == AI_ACTIVE_GOAL_DYNAMIC)) {
				if (aip->guard_objnum == hit_objp-Objects) {
					guard_object_was_hit(objp, hitter_objp);
				} else if ((aip->guard_wingnum != -1) && (aip->guard_wingnum == Ai_info[Ships[hit_objp->instance].ai_index].wing)) {
					guard_object_was_hit(objp, hitter_objp);
				}
			}
		}
	}
}

// Scan missile list looking for bombs homing on guarded_objp
// return 1 if bomb is found (and targeted by guarding_objp), otherwise return 0
int ai_guard_find_nearby_bomb(object *guarding_objp, object *guarded_objp)
{	
	missile_obj	*mo;
	object		*bomb_objp, *closest_bomb_objp=NULL;
	float			dist, dist_to_guarding_obj,closest_dist_to_guarding_obj=999999.0f;
	weapon		*wp;
	weapon_info	*wip;

	for ( mo = GET_NEXT(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
		Assert(mo->objnum >= 0 && mo->objnum < MAX_OBJECTS);
		bomb_objp = &Objects[mo->objnum];

		wp = &Weapons[bomb_objp->instance];
		wip = &Weapon_info[wp->weapon_info_index];

		if ( !(wip->wi_flags & WIF_BOMB) ) {
			continue;
		}

		if ( wp->homing_object != guarded_objp ) {
			continue;
		}

		if (wp->lssm_stage==3){
			continue;
		}

		dist = vm_vec_dist_quick(&bomb_objp->pos, &guarded_objp->pos);

		if (dist < (MAX_GUARD_DIST + guarded_objp->radius)*3) {
			dist_to_guarding_obj = vm_vec_dist_quick(&bomb_objp->pos, &guarding_objp->pos);
			if ( dist_to_guarding_obj < closest_dist_to_guarding_obj ) {
				closest_dist_to_guarding_obj = dist_to_guarding_obj;
				closest_bomb_objp = bomb_objp;
			}
		}
	}

	if ( closest_bomb_objp ) {
		guard_object_was_hit(guarding_objp, closest_bomb_objp);
		return 1;
	}

	return 0;
}

/**
 * Scan enemy ships and see if one is near enough to guard object to be pursued.
 */
void ai_guard_find_nearby_ship(object *guarding_objp, object *guarded_objp)
{
	ship *guarding_shipp = &Ships[guarding_objp->instance];
	ai_info	*guarding_aip = &Ai_info[guarding_shipp->ai_index];
	ship_obj *so;
	object *enemy_objp;
	float dist;

	for (so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
	{
		enemy_objp = &Objects[so->objnum];

		if (enemy_objp->instance < 0)
		{
			continue;
		}

		ship *eshipp = &Ships[enemy_objp->instance];

		if (iff_x_attacks_y(guarding_shipp->team, eshipp->team))
		{
			//	Don't attack a cargo container or other harmless ships
			if (Ship_info[eshipp->ship_info_index].class_type >= 0 && (Ship_types[Ship_info[eshipp->ship_info_index].class_type].ai_bools & STI_AI_GUARDS_ATTACK))
			{
				dist = vm_vec_dist_quick(&enemy_objp->pos, &guarded_objp->pos);
				if (dist < (MAX_GUARD_DIST + guarded_objp->radius)*3)
				{
					guard_object_was_hit(guarding_objp, enemy_objp);
				}
				else if ((dist < 3000.0f) && (Ai_info[eshipp->ai_index].target_objnum == guarding_aip->guard_objnum))
				{
					guard_object_was_hit(guarding_objp, enemy_objp);
				}
			}
		}
	}
}

// Scan for nearby asteroids.  Favor asteroids which have their collide_objnum set to that of the
// guarded ship.  Also, favor asteroids that are closer to the guarding ship, since it looks cooler
// when a ship blows up an asteroid then goes after the pieces that break off.
void ai_guard_find_nearby_asteroid(object *guarding_objp, object *guarded_objp)
{	
	float		dist;

	object	*closest_asteroid_objp=NULL, *danger_asteroid_objp=NULL, *asteroid_objp;
	float		dist_to_self, closest_danger_asteroid_dist=999999.0f, closest_asteroid_dist=999999.0f;

	for ( asteroid_objp = GET_FIRST(&obj_used_list); asteroid_objp != END_OF_LIST(&obj_used_list); asteroid_objp = GET_NEXT(asteroid_objp) ) {
		if ( asteroid_objp->type == OBJ_ASTEROID ) {
			// Attack asteroid if near guarded ship
			dist = vm_vec_dist_quick(&asteroid_objp->pos, &guarded_objp->pos);
			if ( dist < (MAX_GUARD_DIST + guarded_objp->radius)*2) {
				dist_to_self = vm_vec_dist_quick(&asteroid_objp->pos, &guarding_objp->pos);
				if ( OBJ_INDEX(guarded_objp) == asteroid_collide_objnum(asteroid_objp) ) {
					if( dist_to_self < closest_danger_asteroid_dist ) {
						danger_asteroid_objp=asteroid_objp;
						closest_danger_asteroid_dist=dist_to_self;
					}
				} 
				if ( dist_to_self < closest_asteroid_dist ) {
					// only attack if moving slower than own max speed
					if ( vm_vec_mag_quick(&asteroid_objp->phys_info.vel) < guarding_objp->phys_info.max_vel.xyz.z ) {
						closest_asteroid_dist = dist_to_self;
						closest_asteroid_objp = asteroid_objp;
					}
				}
			}
		}
	}

	if ( danger_asteroid_objp ) {
		guard_object_was_hit(guarding_objp, danger_asteroid_objp);
	} else if ( closest_asteroid_objp ) {
		guard_object_was_hit(guarding_objp, closest_asteroid_objp);
	}
}

/**
 * Scan potential harmful objects and see if one is near enough to guard object to be pursued.
 */
void ai_guard_find_nearby_object()
{
	ship			*shipp = &Ships[Pl_objp->instance];
	ai_info		*aip = &Ai_info[shipp->ai_index];
	object		*guardobjp;
	int			bomb_found=0;

	guardobjp = &Objects[aip->guard_objnum];
	
	// highest priority is a bomb fired on guarded ship
	bomb_found = ai_guard_find_nearby_bomb(Pl_objp, guardobjp);

	if ( !bomb_found ) {
		// check for ships if there are no bombs fired at guarded ship
		ai_guard_find_nearby_ship(Pl_objp, guardobjp);

		// if not attacking anything, go for asteroid close to guarded ship
		if ( (aip->target_objnum == -1) && asteroid_count() ) {
			ai_guard_find_nearby_asteroid(Pl_objp, guardobjp);
		}
	}
}

/**
 * Gets closest point on extended axis of cylinder, r_vec, and radius of cylinder
 * @return z of axis_point in cyl_objp reference frame
 */
float get_cylinder_points(object *other_objp, object *cyl_objp, vec3d *axis_pt, vec3d *r_vec, float *radius)
{
	Assert(other_objp->type == OBJ_SHIP);
	Assert(cyl_objp->type == OBJ_SHIP);

	// get radius of cylinder
	polymodel *pm = model_get(Ship_info[Ships[cyl_objp->instance].ship_info_index].model_num);
	float tempx, tempy;
	tempx = MAX(-pm->mins.xyz.x, pm->maxs.xyz.x);
	tempy = MAX(-pm->mins.xyz.y, pm->maxs.xyz.y);
	*radius = MAX(tempx, tempy);

	// get vec from cylinder to other_obj
	vec3d r_sph;
	vm_vec_sub(&r_sph, &other_objp->pos, &cyl_objp->pos);

	// get point on axis and on cylinder
	// extended_cylinder_z is along extended cylinder
	// cylinder_z is capped within cylinder
	float extended_cylinder_z = vm_vec_dotprod(&r_sph, &cyl_objp->orient.vec.fvec);

	// get pt on axis of extended cylinder
	vm_vec_scale_add(axis_pt, &cyl_objp->pos, &cyl_objp->orient.vec.fvec, extended_cylinder_z);

	// get r_vec (pos - axis_pt) normalized
	vm_vec_normalized_dir(r_vec, &other_objp->pos, axis_pt);

	return extended_cylinder_z;
}

// handler for guard behavior when guarding BIG ships
//	When someone has attacked guarded ship, then attack that ship.
// To attack another ship, switch out of guard mode into chase mode.
void ai_big_guard()
{
	
	ship			*shipp = &Ships[Pl_objp->instance];
	ai_info		*aip = &Ai_info[shipp->ai_index];
	object		*guard_objp;

	// sanity checks already done in ai_guard()
	guard_objp = &Objects[aip->guard_objnum];

	switch (aip->submode) {
	case AIS_GUARD_STATIC:
	case AIS_GUARD_PATROL:
		{
		vec3d axis_pt, r_vec, theta_vec;
		float radius, extended_z;

		// get random [0 to 1] based on OBJNUM
		float objval = static_randf(Pl_objp-Objects);

		// get position relative to cylinder of guard_objp		
		extended_z = get_cylinder_points(Pl_objp, guard_objp, &axis_pt, &r_vec, &radius);
		vm_vec_crossprod(&theta_vec, &guard_objp->orient.vec.fvec, &r_vec);

		// half ships circle each way
		if (objval > 0.5f) {
			vm_vec_negate(&theta_vec);
		}

		float min_guard_dist = radius + Pl_objp->radius + 50.0f;
		float desired_guard_dist = min_guard_dist + 0.5f * ((1.0f + objval) * MAX_GUARD_DIST);
		float max_guard_dist =     min_guard_dist + 1.0f * ((1.0f + objval) * MAX_GUARD_DIST);

		// get z extents
		float min_z, max_z, length;
		polymodel *pm = model_get(Ship_info[Ships[guard_objp->instance].ship_info_index].model_num);
		min_z = pm->mins.xyz.z;
		max_z = pm->maxs.xyz.z;
		length = max_z - min_z;

		// get desired z
		// how often to choose new desired_z
		// 1*(64) sec < 2000, 2*(64) < 2-4000 3*(64) > 4-8000, etc (Missiontime >> 22 is 64 sec intervals)
		int time_choose = int(floor(log(length * 0.001f) / log(2.0f)));
		float desired_z = min_z + length * static_randf( Pl_objp-Objects ^ (Missiontime >> (22 + time_choose)) );

		// get r from guard_ship
		float cur_guard_rad = vm_vec_dist(&Pl_objp->pos, &axis_pt);

		// is ship within extents of cylinder of ship it is guarding
		int inside = (extended_z > min_z) && (extended_z < min_z + length);

		vec3d goal_pt;
		// maybe go into orbit mode
		if (cur_guard_rad < max_guard_dist) {
			if ( cur_guard_rad > min_guard_dist ) {
				if (inside) {
					// orbit
					vm_vec_scale_add(&goal_pt, &axis_pt, &r_vec, desired_guard_dist);
					vm_vec_scale_add2(&goal_pt, &theta_vec, desired_guard_dist);
				} else {
					// move to where I can orbit
					if (extended_z < min_z) {
						vm_vec_scale_add(&goal_pt, &guard_objp->pos, &guard_objp->orient.vec.fvec, min_z);
					} else {
						vm_vec_scale_add(&goal_pt, &guard_objp->pos, &guard_objp->orient.vec.fvec, max_z);
					}
					vm_vec_scale_add2(&goal_pt, &r_vec, desired_guard_dist);
					vm_vec_scale_add2(&goal_pt, &theta_vec, desired_guard_dist);
				}
			} else {
				// too close for orbit mode
				if (inside) {
					// inside (fly straight out and return circle)
					vm_vec_scale_add(&goal_pt, &axis_pt, &r_vec, max_guard_dist);
				} else {
					// outside (fly to edge and circle)
					if (extended_z < min_z) {
						vm_vec_scale_add(&goal_pt, &guard_objp->pos, &guard_objp->orient.vec.fvec, min_z);
					} else {
						vm_vec_scale_add(&goal_pt, &guard_objp->pos, &guard_objp->orient.vec.fvec, max_z);
					}
					vm_vec_scale_add2(&goal_pt, &r_vec, max_guard_dist);
					vm_vec_scale_add2(&goal_pt, &theta_vec, desired_guard_dist);
				}
			}

			// make sure we have a forward velocity worth calculating (values too low can produce NaN in goal_pt) - taylor
			if (Pl_objp->phys_info.fspeed > 0.0001f) {
				// modify goal_pt to take account moving guard objp
				float dist = vm_vec_dist_quick(&Pl_objp->pos, &goal_pt);
				float time = dist / Pl_objp->phys_info.fspeed;
				vm_vec_scale_add2(&goal_pt, &guard_objp->phys_info.vel, time);

				// now modify to move to desired z (at a max of 20 m/s)
				float delta_z = desired_z - extended_z;
				float v_z = delta_z * 0.2f;
				if (v_z < -20) {
					v_z = -20.0f;
				} else if (v_z > 20) {
					v_z = 20.0f;
				}

				vm_vec_scale_add2(&goal_pt, &guard_objp->orient.vec.fvec, v_z*time);
			}

		} else {
			// cast vector to center of guard_ship adjusted by desired_z
			float delta_z = desired_z - extended_z;
			vm_vec_scale_add(&goal_pt, &guard_objp->pos, &guard_objp->orient.vec.fvec, delta_z);
		}

		// try not to bump into things along the way
		if ( (cur_guard_rad > max_guard_dist) || (extended_z < min_z) || (extended_z > max_z) ) {
			if (maybe_avoid_big_ship(Pl_objp, guard_objp, aip, &goal_pt, 5.0f)) {
				return;
			}

			if (avoid_player(Pl_objp, &goal_pt)) {
				return;
			}
		} else {
			if (maybe_avoid_big_ship(Pl_objp, guard_objp, aip, &goal_pt, 5.0f)) {
				return;
			}
		}

		// got the point, now let's go there
		ai_turn_towards_vector(&goal_pt, Pl_objp, flFrametime, Ship_info[Ships[Pl_objp->instance].ship_info_index].srotation_time, NULL, NULL, 0.0f, 0);
		accelerate_ship(aip, 1.0f);

		//	Periodically, scan for a nearby ship to attack.
		if (((AI_FrameCount ^ (Pl_objp-Objects)) & 0x07) == 0) {
			ai_guard_find_nearby_object();
		}
		}
		break;

	case AIS_GUARD_ATTACK:
		//	The guarded ship has been attacked.  Do something useful!
		ai_chase();
		break;

	default:
		Int3();	//	Illegal submode for Guard mode.
		// AL 06/03/97 comment out Int3() to allow milestone to get out the door
		aip->submode = AIS_GUARD_PATROL;
		aip->submode_start_time = Missiontime;
		break;
	}
}

//	Main handler for guard behavior.
//	When someone has attacked guarded ship, then attack that ship.
// To attack another ship, switch out of guard mode into chase mode.
void ai_guard()
{
	ship			*shipp = &Ships[Pl_objp->instance];
	ai_info		*aip = &Ai_info[shipp->ai_index];
	object		*guard_objp;	
	ship			*gshipp;
	float			dist_to_guardobj, dot_to_guardobj;
	vec3d		vec_to_guardobj;

	if (aip->guard_objnum == -1) {
		aip->mode = AIM_NONE;
		return;
	}

	Assert(aip->guard_objnum != -1);

	guard_objp = &Objects[aip->guard_objnum];

	if (guard_objp == Pl_objp) {
		Int3();		//	This seems illegal.  Why is a ship guarding itself?
		aip->guard_objnum = -1;
		return;
	}

	// check that I have someone to guard
	if (guard_objp->instance == -1) {
		return;
	}

	//	Not sure whether this should be impossible, or a reasonable cleanup condition.
	//	For now (3/31/97), it's getting trapped by an Assert, so clean it up.
	if (guard_objp->type != OBJ_SHIP) {
		aip->guard_objnum = -1;
		return;
	}

	// handler for gurad object with BIG radius
	if (guard_objp->radius > BIG_GUARD_RADIUS) {
		ai_big_guard();
		return;
	}

	gshipp = &Ships[guard_objp->instance];

	float			objval;
	vec3d		goal_point;
	vec3d		rel_vec;
	float			dist_to_goal_point, dot_to_goal_point, accel_scale;
	vec3d		v2g, rvec;

	// get random [0 to 1] based on OBJNUM
	objval = static_randf(Pl_objp-Objects);

	switch (aip->submode) {
	case AIS_GUARD_STATIC:
	case AIS_GUARD_PATROL:
		//	Stay near ship
		dist_to_guardobj = vm_vec_normalized_dir(&vec_to_guardobj, &guard_objp->pos, &Pl_objp->pos);
		dot_to_guardobj = vm_vec_dot(&Pl_objp->orient.vec.fvec, &vec_to_guardobj);

		rel_vec = aip->guard_vec;
		vm_vec_add(&goal_point, &guard_objp->pos, &rel_vec);

		vm_vec_normalized_dir(&v2g, &goal_point, &Pl_objp->pos);
		dist_to_goal_point = vm_vec_dist_quick(&goal_point, &Pl_objp->pos);
		dot_to_goal_point = vm_vec_dot(&v2g, &Pl_objp->orient.vec.fvec);
		accel_scale = (1.0f + dot_to_goal_point)/2.0f;

		//	If far away, get closer
		if (dist_to_goal_point > MAX_GUARD_DIST + 1.5 * (Pl_objp->radius + guard_objp->radius)) {
			if (maybe_avoid_big_ship(Pl_objp, guard_objp, aip, &goal_point, 5.0f)) {
				return;
			}

			if (avoid_player(Pl_objp, &goal_point)) {
				return;
			}

			// quite far away, so try to go straight to 
			compute_desired_rvec(&rvec, &goal_point, &Pl_objp->pos);
			ai_turn_towards_vector(&goal_point, Pl_objp, flFrametime, Ship_info[shipp->ship_info_index].srotation_time, NULL, NULL, 0.0f, 0, &rvec);

			accelerate_ship(aip, accel_scale * (0.25f + dist_to_goal_point/700.0f));
		} else {
			if (maybe_avoid_big_ship(Pl_objp, guard_objp, aip, &goal_point, 2.0f)) {
				return;
			}

			// get max of guard_objp (1) normal speed (2) dock speed
			float speed = guard_objp->phys_info.speed;

			if (guard_objp->type == OBJ_SHIP) {
				if (object_is_docked(guard_objp)) {
					speed = dock_calc_docked_speed(guard_objp);
				}
			}
			
			//	Deal with guarding a small object.
			//	If going to guard_vec might cause a collision with guarded object, pick a new guard point.
			if (vm_vec_dot(&v2g, &vec_to_guardobj) > 0.8f) {
				if (dist_to_guardobj < dist_to_goal_point) {
					ai_set_guard_vec(Pl_objp, guard_objp);	//	OK to return here.
					return;
				}
			} 

			if (speed > 10.0f) {
				//	If goal ship is moving more than a tiny bit, don't orbit it, get near it.
				if (vm_vec_dist_quick(&goal_point, &Pl_objp->pos) > 40.0f) {
					if (vm_vec_dot(&Pl_objp->orient.vec.fvec, &v2g) < 0.0f) {
						//	Just slow down, don't turn.
						set_accel_for_target_speed(Pl_objp, guard_objp->phys_info.speed - dist_to_goal_point/10.0f);
					} else {
						//	Goal point is in front.
						//	If close to goal point, don't change direction, just change speed.
						if (dist_to_goal_point > Pl_objp->radius + 10.0f) {
							turn_towards_point(Pl_objp, &goal_point, NULL, 0.0f);
						}
						
						set_accel_for_target_speed(Pl_objp, guard_objp->phys_info.speed + (dist_to_goal_point-40.0f)/20.0f);
					}
				} else {
					if (dot_to_goal_point > 0.8f) {
						turn_towards_point(Pl_objp, &goal_point, NULL, 0.0f);
						set_accel_for_target_speed(Pl_objp, guard_objp->phys_info.speed + dist_to_goal_point*0.1f);
					} else {
						set_accel_for_target_speed(Pl_objp, guard_objp->phys_info.speed - dist_to_goal_point*0.1f - 1.0f);
					}
				}
			// consider guard object STILL
			} else if (guard_objp->radius < 50.0f) {
				if (dist_to_goal_point > 15.0f) {
					turn_towards_point(Pl_objp, &goal_point, NULL, 0.0f);
					set_accel_for_target_speed(Pl_objp, (dist_to_goal_point-10.0f)/2.0f);
				} else if (Pl_objp->phys_info.speed < 1.0f) {
					turn_away_from_point(Pl_objp, &guard_objp->pos, 0.0f);
				}
				//	It's a big ship
			} else if (dist_to_guardobj > MAX_GUARD_DIST + Pl_objp->radius + guard_objp->radius) {
				//	Orbiting ship, too far away
				float dot = turn_towards_tangent(Pl_objp, &guard_objp->pos, (1.0f + objval/2) * guard_objp->radius);
				accelerate_ship(aip, (1.0f + dot)/2.0f);
			} else if (dist_to_guardobj < Pl_objp->radius + guard_objp->radius) {
				//	Orbiting ship, got too close
				turn_away_from_point(Pl_objp, &guard_objp->pos, 0.0f);
				if ((dist_to_guardobj > guard_objp->radius + Pl_objp->radius + 50.0f) && (guard_objp->phys_info.speed > Pl_objp->phys_info.speed - 1.0f))
					change_acceleration(aip, 0.25f);
				else
					accelerate_ship(aip, 0.5f + objval/4.0f);
			} else {
				//	Orbiting ship, about the right distance away.
				float dot = turn_towards_tangent(Pl_objp, &guard_objp->pos, (1.5f + objval/2.0f)*guard_objp->radius);
				if ((dist_to_guardobj > guard_objp->radius + Pl_objp->radius + 50.0f) && (guard_objp->phys_info.speed > Pl_objp->phys_info.speed - 1.0f))
					set_accel_for_target_speed(Pl_objp, (0.5f * (1.0f + dot)) * (guard_objp->phys_info.speed + (dist_to_guardobj - guard_objp->radius - Pl_objp->radius)/10.0f));
				else
					accelerate_ship(aip, 0.5f * (1.0f + dot) * (0.3f + objval/3.0f));
			}
		}

		//	Periodically, scan for a nearby ship to attack.
		if (((AI_FrameCount ^ (Pl_objp-Objects)) & 0x07) == 0) {
			ai_guard_find_nearby_object();
		}
		break;

	case AIS_GUARD_ATTACK:
		//	The guarded ship has been attacked.  Do something useful!
		ai_chase();

		break;
	default:
		Int3();	//	Illegal submode for Guard mode.
		// AL 06/03/97 comment out Int3() to allow milestone to get out the door
		aip->submode = AIS_GUARD_PATROL;
		aip->submode_start_time = Missiontime;
		break;
	}

}

// function to clean up ai flags, variables, and other interesting information
// for a ship that was getting repaired.  The how parameter is useful for multiplayer
// only in that it tells us why the repaired ship is being cleaned up.
void ai_do_objects_repairing_stuff( object *repaired_objp, object *repair_objp, int how )
{
	ai_info *aip, *repair_aip;
	int	stamp = -1;

	int p_index;
	int p_team;

	p_index = -1;
	p_team = -1;

	// repaired_objp should not be null, but repair_objp will be null when a support ship is just warping in
	Assert(repaired_objp != NULL);

	Assert( repaired_objp->type == OBJ_SHIP);
	aip = &Ai_info[Ships[repaired_objp->instance].ai_index];

	if(Game_mode & GM_MULTIPLAYER){
		p_index = multi_find_player_by_object(repaired_objp);
		p_team = Net_players[p_index].p_info.team;
	} else {		
		if(repaired_objp == Player_obj){
			p_index = Player_num;
		}
	}

	switch( how ) {
	case REPAIR_INFO_BEGIN:
		aip->ai_flags |= AIF_BEING_REPAIRED;
		aip->ai_flags &= ~AIF_AWAITING_REPAIR;
		stamp = timestamp(-1);

		// if this is a player ship, then subtract the repair penalty from this player's score
		if ( repaired_objp->flags & OF_PLAYER_SHIP ) {
			if ( !(Game_mode & GM_MULTIPLAYER) ) {
				Player->stats.m_score -= The_mission.ai_profile->repair_penalty[Game_skill_level];			// subtract the penalty
			}
		}
		break;

	case REPAIR_INFO_BROKEN:
		aip->ai_flags &= ~AIF_BEING_REPAIRED;
		aip->ai_flags |= AIF_AWAITING_REPAIR;
		stamp = timestamp((int) ((30 + 10*frand()) * 1000));
		break;

	case REPAIR_INFO_END:
		// when only awaiting repair, and the repair is ended, then set support to -1.
		if ( aip->ai_flags & AIF_AWAITING_REPAIR ){
			aip->support_ship_objnum = -1;
		}
		aip->ai_flags &= ~(AIF_AWAITING_REPAIR | AIF_BEING_REPAIRED );
		stamp = timestamp((int) ((30 + 10*frand()) * 1000));
		break;

	case REPAIR_INFO_QUEUE:
		aip->ai_flags |= AIF_AWAITING_REPAIR;
		if ( aip == Player_ai ){
			hud_support_view_start();
		}
		stamp = timestamp(-1);
		break;

	case REPAIR_INFO_ABORT:
	case REPAIR_INFO_KILLED:
		// undock if necessary (we may be just waiting for a repair in which case we aren't docked)
		if ((repair_objp != NULL) && dock_check_find_direct_docked_object(repair_objp, repaired_objp))
		{
			ai_do_objects_undocked_stuff(repair_objp, repaired_objp);
		}

		// 5/4/98 -- MWA -- Need to set support objnum to -1 to let code know this guy who was getting
		// repaired (or queued for repair), isn't really going to be docked with anyone anymore.
		aip->support_ship_objnum = -1;
		aip->ai_flags &= ~(AIF_AWAITING_REPAIR | AIF_BEING_REPAIRED );

		if (repair_objp != NULL) {
			repair_aip = &Ai_info[Ships[repair_objp->instance].ai_index];
			repair_aip->ai_flags &= ~(AIF_AWAITING_REPAIR | AIF_BEING_REPAIRED );
		}		

		if ( p_index >= 0 ) {
			hud_support_view_abort();

			// send appropriate messages here
			if (Game_mode & GM_NORMAL || MULTIPLAYER_MASTER) {
				if ( how == REPAIR_INFO_KILLED ){
					message_send_builtin_to_player( MESSAGE_SUPPORT_KILLED, NULL, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_SOON, 0, 0, p_index, p_team );
				} else {
					if ( repair_objp ){
						message_send_builtin_to_player( MESSAGE_REPAIR_ABORTED, &Ships[repair_objp->instance], MESSAGE_PRIORITY_NORMAL, MESSAGE_TIME_SOON, 0, 0, p_index, p_team );
					}
				}
			}
		}

		// add log entry if this is a player
		if ( repaired_objp->flags & OF_PLAYER_SHIP ){
			mission_log_add_entry(LOG_PLAYER_ABORTED_REARM, Ships[repaired_objp->instance].ship_name, NULL);
		}

		stamp = timestamp((int) ((30 + 10*frand()) * 1000));
		break;

	case REPAIR_INFO_COMPLETE:
		// clear the being repaired flag -- and 
		if ( p_index >= 0 ) {
			Assert( repair_objp );
			
			hud_support_view_stop();			
			
			if (Game_mode & GM_NORMAL || MULTIPLAYER_MASTER) {
				message_send_builtin_to_player(MESSAGE_REPAIR_DONE, &Ships[repair_objp->instance], MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, p_index, p_team);
			}
		}
		stamp = timestamp((int) ((30 + 10*frand()) * 1000));
		break;

	case REPAIR_INFO_ONWAY:
		// need to set the signature so that clients in multiplayer games rearm correctly
		Assert( repair_objp );
		aip->support_ship_signature = repair_objp->signature;
		aip->support_ship_objnum = OBJ_INDEX(repair_objp);
		stamp = timestamp(-1);
		break;

	default:
		Int3();			// bogus type of repair info
	}

	if (repair_objp){
		Ai_info[Ships[repair_objp->instance].ai_index].warp_out_timestamp = stamp;
	}

	// repair_objp might be NULL is we are cleaning up this mode because of the support ship
	// getting killed.
	if ( repair_objp ) {
		aip = &Ai_info[Ships[repair_objp->instance].ai_index];
		switch ( how ) {
		case REPAIR_INFO_ONWAY:
			Assert( repaired_objp != NULL );
			aip->goal_objnum = OBJ_INDEX(repaired_objp);
			aip->ai_flags |= AIF_REPAIRING;
			break;

		case REPAIR_INFO_BROKEN:
			break;

		case REPAIR_INFO_END:
		case REPAIR_INFO_ABORT:
		case REPAIR_INFO_KILLED:
			if ( how == REPAIR_INFO_ABORT )
				aip->goal_objnum = -1;

			aip->ai_flags &= ~AIF_REPAIRING;
			break;
			
		case REPAIR_INFO_QUEUE:
			ai_add_rearm_goal( repaired_objp, repair_objp );
			break;

		case REPAIR_INFO_BEGIN:
		case REPAIR_INFO_COMPLETE:
			break;

		default:
			Int3();		// bogus type of repair info
		}
	}

	multi_maybe_send_repair_info( repaired_objp, repair_objp, how );
}

// Goober5000 - clean up my own dock mode
void ai_cleanup_dock_mode_subjective(object *objp)
{
	// get ai of object
	ai_info *aip = &Ai_info[Ships[objp->instance].ai_index];

	// if the object is in dock mode, force them to near last stage
	if ( (aip->mode == AIM_DOCK) && (aip->submode < AIS_UNDOCK_3) )
	{
		aip->submode = AIS_UNDOCK_3;
		aip->submode_start_time = Missiontime;
	}
}

// Goober5000 - clean up the dock mode of everybody around me
// (This function should ONLY need to be called from a ship doing a deathroll.
// It ensures that any ship docking or undocking with it will finish gracefully.)
void ai_cleanup_dock_mode_objective(object *objp)
{
	// process all directly docked objects
	for (dock_instance *ptr = objp->dock_list; ptr != NULL; ptr = ptr->next)
		ai_cleanup_dock_mode_subjective(ptr->docked_objp);
}

// Goober5000
// (This function should ONLY need to be called from a ship doing a deathroll.
// It ensures that any support ship stuff will be wrapped up gracefully.)
void ai_cleanup_rearm_mode(object *objp)
{
	ai_info *aip = &Ai_info[Ships[objp->instance].ai_index];

	if (aip->ai_flags & AIF_REPAIRING) {
		Assert( aip->goal_objnum != -1 );
		ai_do_objects_repairing_stuff( &Objects[aip->goal_objnum], objp, REPAIR_INFO_KILLED );
	} else if ( aip->ai_flags & AIF_BEING_REPAIRED ) {
		// MWA/Goober5000 -- note that we have to use support object here instead of goal_objnum.
		Assert( aip->support_ship_objnum != -1 );
		ai_do_objects_repairing_stuff( objp, &Objects[aip->support_ship_objnum], REPAIR_INFO_ABORT );
	} else if ( aip->ai_flags & AIF_AWAITING_REPAIR ) {
		// MWA/Goober5000 -- note that we have to use support object here instead of goal_objnum.
		// MWA -- 3/38/98  Check to see if this guy is queued for a support ship, or there is already
		// one in the mission
		if ( mission_is_repair_scheduled(objp) ) {
			mission_remove_scheduled_repair( objp );			// this function will notify multiplayer clients.
		} else {
			if ( aip->support_ship_objnum != -1 )
				ai_do_objects_repairing_stuff( objp, &Objects[aip->support_ship_objnum], REPAIR_INFO_ABORT );
			else
				ai_do_objects_repairing_stuff( objp, NULL, REPAIR_INFO_ABORT );
		}
	}
}

//	Make Pl_objp point at aip->goal_point.
void ai_still()
{
	ship	*shipp;
	ai_info	*aip;

	Assert(Pl_objp->type == OBJ_SHIP);
	Assert((Pl_objp->instance >= 0) && (Pl_objp->instance < MAX_OBJECTS));

	shipp = &Ships[Pl_objp->instance];
	Assert((shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO));

	aip = &Ai_info[shipp->ai_index];

	turn_towards_point(Pl_objp, &aip->goal_point, NULL, 0.0f);
}

//	Make *Pl_objp stay near another ship.
void ai_stay_near()
{
	ai_info	*aip;
	int		goal_objnum;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	goal_objnum = aip->goal_objnum;

	if ((goal_objnum < 0) || (Objects[goal_objnum].type != OBJ_SHIP) || (Objects[goal_objnum].signature != aip->goal_signature)) {
		aip->mode = AIM_NONE;
	} else {
		float		dist, max_dist, scale;
		vec3d	rand_vec, goal_pos, vec_to_goal;
		object	*goal_objp;

		goal_objp = &Objects[goal_objnum];

		//	Make not all ships pursue same point.
		static_randvec(Pl_objp-Objects, &rand_vec);

		//	Make sure point is in front hemisphere (relative to Pl_objp's position.
		vm_vec_sub(&vec_to_goal, &goal_objp->pos, &Pl_objp->pos);
		if (vm_vec_dot(&rand_vec, &vec_to_goal) > 1.0f) {
			vm_vec_negate(&rand_vec);
		}

		//	Scale the random vector by an amount proportional to the distance from Pl_objp to the true goal.
		dist = vm_vec_dist_quick(&goal_objp->pos, &Pl_objp->pos);
		max_dist = aip->stay_near_distance;
		scale = dist - max_dist/2;
		if (scale < 0.0f)
			scale = 0.0f;

		vm_vec_scale_add(&goal_pos, &goal_objp->pos, &rand_vec, scale);

		if (max_dist < Pl_objp->radius + goal_objp->radius + 25.0f)
			max_dist = Pl_objp->radius + goal_objp->radius + 25.0f;

		if (dist > max_dist) {
			turn_towards_point(Pl_objp, &goal_pos, NULL, 0.0f);
			accelerate_ship(aip, dist / max_dist - 0.8f);
		}
	
	}

}

//	Warn player if dock path is obstructed.
int maybe_dock_obstructed(object *cur_objp, object *goal_objp, int big_only_flag)
{
	vec3d	*goalpos, *curpos;
	float		radius;
	ai_info	*aip;
	int		collide_objnum;

	aip = &Ai_info[Ships[cur_objp->instance].ai_index];

	Ai_info[Ships[goal_objp->instance].ai_index].ai_flags &= ~AIF_REPAIR_OBSTRUCTED;

	if (goal_objp != Player_obj)
		return -1;

	curpos = &cur_objp->pos;
	radius = cur_objp->radius;
	goalpos = &Path_points[aip->path_cur].pos;
	collide_objnum = pp_collide_any(curpos, goalpos, radius, cur_objp, goal_objp, big_only_flag);

	if (collide_objnum != -1)
		Ai_info[Ships[goal_objp->instance].ai_index].ai_flags |= AIF_REPAIR_OBSTRUCTED;

	return collide_objnum;
}


//	Docking behavior.
//	Approach a ship, follow path to docking platform, approach platform, after awhile,
//	undock.
void ai_dock()
{
	ship		*shipp = &Ships[Pl_objp->instance];
	ai_info		*aip = &Ai_info[shipp->ai_index];

	// Make sure we still have a dock goal.
	// Make sure the object we're supposed to dock with or undock from still exists.
	if ( ((aip->active_goal < 0) && (aip->submode != AIS_DOCK_4A))
		|| (aip->goal_objnum == -1)	|| (Objects[aip->goal_objnum].signature != aip->goal_signature) )
	{
		ai_cleanup_dock_mode_subjective(Pl_objp);
	}

	ship_info	*sip = &Ship_info[shipp->ship_info_index];
	int docker_index, dockee_index;

	// get the active goal
	ai_goal *aigp;
	if (aip->active_goal >= 0)
		aigp = &aip->goals[aip->active_goal];
	else
		aigp = NULL;

	// get the object being acted upon
	object		*goal_objp;
	if (aip->goal_objnum >= 0)
		goal_objp = &Objects[aip->goal_objnum];
	else
		goal_objp = NULL;

	// get the indexes
	if ((aip->submode == AIS_DOCK_2) || (aip->submode == AIS_DOCK_3) || (aip->submode == AIS_DOCK_4))
	{
		// get them from the active goal
		Assert(aigp != NULL);
		Assert(aigp->flags & AIGF_DOCK_INDEXES_VALID);
		docker_index = aigp->docker.index;
		dockee_index = aigp->dockee.index;
	}
	else if ((aip->submode == AIS_UNDOCK_0) || (aip->submode == AIS_UNDOCK_1) || (aip->submode == AIS_UNDOCK_2))
	{
		// get them from the guy I'm docked to
		Assert(goal_objp != NULL);
		docker_index = dock_find_dockpoint_used_by_object(Pl_objp, goal_objp);
		dockee_index = dock_find_dockpoint_used_by_object(goal_objp, Pl_objp);
	}
	else
	{
		// indexes aren't needed or (in case of AIS_UNDOCK_3) aren't actually used
		docker_index = 0;
		dockee_index = 0;
	}

	// For docking submodes (ie, not undocking), follow path.  Once at second last
	// point on path (point just before point on dock platform), orient into position.
	//
	// For undocking, first mode pushes docked ship straight back from docking point
	// second mode turns ship and moves to point on docking radius
	switch (aip->submode)
	{

	//	This mode means to find the path to the docking point.
	case AIS_DOCK_0:
	{
		ai_path();
		if (aip->path_length < 4)
		{
			Assert(goal_objp != NULL);
			ship_info *goal_sip = &Ship_info[Ships[goal_objp->instance].ship_info_index];
			char *goal_ship_class_name = goal_sip->name;
			char *goal_dock_path_name = model_get(goal_sip->model_num)->paths[aip->mp_index].name;

			Warning(LOCATION, "Ship class %s has only %i points on dock path \"%s\".  Recommended minimum number of points is 4.  "\
				"Docking along that path will look strange.  You may wish to edit the model.", goal_ship_class_name, aip->path_length, goal_dock_path_name);
		}

		aip->submode = AIS_DOCK_1;
		aip->submode_start_time = Missiontime;
		aip->path_start = -1;
		break;
	}

	//	This mode means to follow the path until just before the end.
	case AIS_DOCK_1:
	{
		float	dist;
		int	r;

		if ((r = maybe_dock_obstructed(Pl_objp, goal_objp, 1)) != -1) {
			int	r1;
			if ((r1 = maybe_avoid_big_ship(Pl_objp, goal_objp, aip, &goal_objp->pos, 7.0f)) != 0) {
				nprintf(("AI", "Support ship %s avoiding large ship %s\n", Ships[Pl_objp->instance].ship_name, Ships[Objects[r1].instance].ship_name));
				break;
			}
		} //else {
		{
			dist = ai_path();

			if (aip->path_cur-aip->path_start >= aip->path_length-1) {		//	If got this far, advance no matter what.
				aip->submode = AIS_DOCK_2;
				aip->submode_start_time = Missiontime;
				aip->path_cur--;
				Assert(aip->path_cur-aip->path_start >= 0);
			} else if (aip->path_cur-aip->path_start >= aip->path_length-2) {
				if (Pl_objp->phys_info.speed > goal_objp->phys_info.speed + 1.5f) {
					set_accel_for_target_speed(Pl_objp, goal_objp->phys_info.speed);
				} else {
					aip->submode = AIS_DOCK_2;
					aip->submode_start_time = Missiontime;
				}
			}
		}
		break;
	}

	//	This mode means to drag oneself right to the second last point on the path.
	//	Path code allows it to overshoot.
	case AIS_DOCK_2:
	{
		float		dist;
		int	r;

		if ((r = maybe_dock_obstructed(Pl_objp, goal_objp,0)) != -1) {
			nprintf(("AI", "Dock 2: Obstructed by %s\n", Ships[Objects[r].instance].ship_name));
			accelerate_ship(aip, 0.0f);
			aip->submode = AIS_DOCK_1;
			aip->submode_start_time = Missiontime;
		} else {
			dist = dock_orient_and_approach(Pl_objp, docker_index, goal_objp, dockee_index, DOA_APPROACH);
			Assert(dist != UNINITIALIZED_VALUE);

			if (dist == DOCK_BACKUP_RETURN_VAL) {
				Assert(aip->goal_objnum >= 0 && aip->goal_objnum < MAX_OBJECTS);
				int path_num;

				aip->submode = AIS_DOCK_1;
				aip->submode_start_time = Missiontime;
				path_num = ai_return_path_num_from_dockbay(goal_objp, dockee_index);
				ai_find_path(Pl_objp, aip->goal_objnum, path_num, 0);
				break;
			}

			float	tolerance;
			if (goal_objp->flags & OF_PLAYER_SHIP)
				tolerance = 6*flFrametime + 1.0f;
			else
				tolerance = 4*flFrametime + 0.5f;

			if ( dist < tolerance) {
				aip->submode = AIS_DOCK_3;
				aip->submode_start_time = Missiontime;
				aip->path_cur++;
			}
		}
		break;
	}

	case AIS_DOCK_3:
	{
		Assert(aip->goal_objnum != -1);
		int	r;

		if ((r = maybe_dock_obstructed(Pl_objp, goal_objp,0)) != -1) {
			nprintf(("AI", "Dock 1: Obstructed by %s\n", Ships[Objects[r].instance].ship_name));
			accelerate_ship(aip, 0.0f);
			aip->submode = AIS_DOCK_2;
			aip->submode_start_time = Missiontime;
		} else {
			rotating_dockpoint_info rdinfo;

			float dist = dock_orient_and_approach(Pl_objp, docker_index, goal_objp, dockee_index, DOA_DOCK, &rdinfo);
			Assert(dist != UNINITIALIZED_VALUE);

			if (dist == DOCK_BACKUP_RETURN_VAL) {
				aip->submode = AIS_DOCK_2;
				aip->submode_start_time = Missiontime;
				break;
			}

			float tolerance = 2*flFrametime * (1.0f + fl_sqrt(goal_objp->phys_info.speed));

			// Goober5000
			if (rdinfo.submodel >= 0)
			{
				tolerance += 4*flFrametime * (rdinfo.submodel_r * rdinfo.submodel_w);
			}

			if (dist < tolerance)
			{
				// - Removed by MK on 11/7/97, causes errors for ships docked at mission start: maybe_recreate_path(Pl_objp, aip, 1);
				dist = dock_orient_and_approach(Pl_objp, docker_index, goal_objp, dockee_index, DOA_DOCK);
				Assert(dist != UNINITIALIZED_VALUE);

				physics_ship_init(Pl_objp);

				ai_do_objects_docked_stuff( Pl_objp, docker_index, goal_objp, dockee_index );

				if (aip->submode == AIS_DOCK_3) {
					snd_play_3d( &Snds[SND_DOCK_ATTACH], &Pl_objp->pos, &View_position );

					if ((Pl_objp == Player_obj) || (goal_objp == Player_obj))
						joy_ff_docked();  // shake player's joystick a little
				}

				//	If this ship is repairing another ship...
				if (aip->ai_flags & AIF_REPAIRING) {
					aip->submode = AIS_DOCK_4;			//	Special rearming only dock mode.
					aip->submode_start_time = Missiontime;
				} else {
					aip->submode = AIS_DOCK_4A;
					aip->submode_start_time = Missiontime;
				}
			}
		}
		break;
	}

	//	Yes, we just sit here.  We wait for further orders.  No, it's not a bug.
	case AIS_DOCK_4A:
	{
		if (aigp == NULL) {	//	Can happen for initially docked ships.
			ai_do_default_behavior( &Objects[Ships[aip->shipnum].objnum] );		// do the default behavior
		} else {
			mission_log_add_entry(LOG_SHIP_DOCKED, Ships[Pl_objp->instance].ship_name, Ships[goal_objp->instance].ship_name);

			if (aigp->ai_mode == AI_GOAL_DOCK) {
				ai_mission_goal_complete( aip );					// Note, this calls ai_do_default_behavior().
			} 
		}
		
		break;
	}

	case AIS_DOCK_4:
	{
		//	This mode is only for rearming/repairing.
		//	The ship that is performing the rearm enters this mode after it docks.
		Assert((aip->goal_objnum >= -1) && (aip->goal_objnum < MAX_OBJECTS));

		float dist = dock_orient_and_approach(Pl_objp, docker_index, goal_objp, dockee_index, DOA_DOCK);
		Assert(dist != UNINITIALIZED_VALUE);

		Assert(goal_objp->type == OBJ_SHIP);
		ship		*goal_shipp = &Ships[goal_objp->instance];		
		ai_info		*goal_aip = &Ai_info[goal_shipp->ai_index];

		// Goober5000 - moved from call_doa
		// Abort if the ship being repaired exceeds my max speed
		if (goal_objp->phys_info.speed > MAX_REPAIR_SPEED)
		{
			// call the ai_abort rearm request code
			ai_abort_rearm_request( Pl_objp );
		}
		//	Make sure repair has not broken off.
		else if (dist > 5.0f)	//	Oops, too far away!
		{
			if ( goal_aip->ai_flags & AIF_BEING_REPAIRED )
				ai_do_objects_repairing_stuff( goal_objp, Pl_objp, REPAIR_INFO_BROKEN);

			if (dist > Pl_objp->radius*2 + goal_objp->radius*2) {
				//	Got real far away from goal, so move back a couple modes and try again.
				aip->submode = AIS_DOCK_2;
				aip->submode_start_time = Missiontime;
			}
		}
		else
		{
			if ( goal_aip->ai_flags & AIF_AWAITING_REPAIR )
				ai_do_objects_repairing_stuff( goal_objp, Pl_objp, REPAIR_INFO_BEGIN );
		}

		break;
	}

	case AIS_UNDOCK_0:
	{
		int path_num;
		//	First stage of undocking.

		// set up the path points for the undocking procedure
		path_num = ai_return_path_num_from_dockbay(goal_objp, dockee_index);
		Assert(path_num >= 0);
		ai_find_path(Pl_objp, OBJ_INDEX(goal_objp), path_num, 0);

		// Play a ship docking detach sound
		snd_play_3d( &Snds[SND_DOCK_DETACH], &Pl_objp->pos, &View_position );

		aip->submode = AIS_UNDOCK_1;
		aip->submode_start_time = Missiontime;

		break;
	}

	case AIS_UNDOCK_1:
	{
		//	Using thrusters, exit from dock station to nearest next dock path point.
		float	dist;
		rotating_dockpoint_info rdinfo;

		if (Missiontime - aip->submode_start_time < REARM_BREAKOFF_DELAY)
		{
			break;		//	Waiting for one second to elapse to let detach sound effect play out.
		}
		else if ( !(aigp->flags & AIGF_DOCK_SOUND_PLAYED))
		{
			snd_play_3d( &Snds[SND_DOCK_DEPART], &Pl_objp->pos, &View_position );
			aigp->flags |= AIGF_DOCK_SOUND_PLAYED;
		}

		dist = dock_orient_and_approach(Pl_objp, docker_index, goal_objp, dockee_index, DOA_UNDOCK_1, &rdinfo);
		Assert(dist != UNINITIALIZED_VALUE);

		float dist_to_dock;

		// Goober5000 - if via submodel, calc distance to point, not center
		if (rdinfo.submodel >= 0)
			dist_to_dock = vm_vec_dist_quick(&Pl_objp->pos, &rdinfo.dockee_point);
		else
			dist_to_dock = vm_vec_dist_quick(&Pl_objp->pos, &goal_objp->pos);

		//	Move to within 0.1 units of second last point on path before orienting, or just plain far away from docked-to ship.
		//	This allows undock to complete if first ship flies away.
		if ((dist < 2*flFrametime) || (dist_to_dock > 2*Pl_objp->radius)) {
			aip->submode = AIS_UNDOCK_2;
			aip->submode_start_time = Missiontime;
		}
		break;
	}

	case AIS_UNDOCK_2:
	{
		float dist;
		ai_info *other_aip;

		// get pointer to docked object's aip to reset flags, etc
		Assert( aip->goal_objnum != -1 );
		other_aip = &Ai_info[Ships[goal_objp->instance].ai_index];

		//	Second stage of undocking.
		dist = dock_orient_and_approach(Pl_objp, docker_index, goal_objp, dockee_index, DOA_UNDOCK_2);
		Assert(dist != UNINITIALIZED_VALUE);
		
		// If at goal point, or quite far away from dock object
		// NOTE: the speed check has an etra 5 thousandths added on to account for some floating point error
		if ((dist < 2.0f) || (vm_vec_dist_quick(&Pl_objp->pos, &goal_objp->pos) > (Pl_objp->radius + goal_objp->radius)*2) || ((goal_objp->phys_info.speed + 0.005f) > MAX_UNDOCK_ABORT_SPEED) ) {
			// reset the dock flags.  If rearm/repair, reset rearm repair flags for those ships as well.
			if ( sip->flags & SIF_SUPPORT ) {
				ai_do_objects_repairing_stuff( &Objects[aip->support_ship_objnum], Pl_objp, REPAIR_INFO_END );
			}

			// clear out dock stuff for both objects.
			ai_do_objects_undocked_stuff( Pl_objp, goal_objp );
			physics_ship_init(Pl_objp);
			aip->submode = AIS_UNDOCK_3;				//	The do-nothing mode, until another order is issued
			aip->submode_start_time = Missiontime;

			// don't add undock log entries for support ships.
			if ( !(sip->flags & SIF_SUPPORT) ) {
				mission_log_add_entry(LOG_SHIP_UNDOCKED, Ships[Pl_objp->instance].ship_name, Ships[goal_objp->instance].ship_name);
			}
		}
		break;
	}

	case AIS_UNDOCK_3:
	{
		if (goal_objp == NULL)
		{
			// this might happen when a goal is cancelled before docking has finished
			aip->submode = AIS_UNDOCK_4;
			aip->submode_start_time = Missiontime;
		}
		else
		{
			float dist = dock_orient_and_approach(Pl_objp, docker_index, goal_objp, dockee_index, DOA_UNDOCK_3);
			Assert(dist != UNINITIALIZED_VALUE);

			if (dist < Pl_objp->radius/2 + 5.0f) {
				aip->submode = AIS_UNDOCK_4;
				aip->submode_start_time = Missiontime;
			}

			// possible that this flag hasn't been cleared yet.  When aborting a rearm, this submode might
			// be entered directly.
			if ( (sip->flags & SIF_SUPPORT) && (aip->ai_flags & AIF_REPAIRING) ) {
				ai_do_objects_repairing_stuff( goal_objp, Pl_objp, REPAIR_INFO_ABORT );
			}
		}

		break;
	}

	case AIS_UNDOCK_4:
	{

		aip->mode = AIM_NONE;

		// only call mission goal complete if this was indeed an undock goal
		if ( aip->active_goal >= 0 ) {
			if ( aigp->ai_mode == AI_GOAL_UNDOCK )
				ai_mission_goal_complete( aip );			// this call should reset the AI mode
		}

		break;
	}

	default:
	{
		Int3();	//	Error, bogus submode
	}

	}	// end of switch statement
}

#ifndef NDEBUG

#define	MAX_AI_DEBUG_RENDER_STUFF	100
typedef struct ai_render_stuff {
	ship_subsys	*ss;
	int			parent_objnum;
} ai_render_stuff;

ai_render_stuff AI_debug_render_stuff[MAX_AI_DEBUG_RENDER_STUFF];

int	Num_AI_debug_render_stuff = 0;
int	Msg_count_4996 = 0;

void ai_debug_render_stuff()
{

	vertex	vert1, vert2;
	vec3d	gpos2;
	int		i;

	for (i=0; i<Num_AI_debug_render_stuff; i++) {
		ship_subsys	*ss;
		int	parent_objnum;
		vec3d	gpos, gvec;
		model_subsystem	*tp;

		ss = AI_debug_render_stuff[i].ss;
		tp = ss->system_info;

		parent_objnum = AI_debug_render_stuff[i].parent_objnum;

		ship_get_global_turret_info(&Objects[parent_objnum], tp, &gpos, &gvec);
		g3_rotate_vertex(&vert1, &gpos);
		vm_vec_scale_add(&gpos2, &gpos, &gvec, 20.0f);
		g3_rotate_vertex(&vert2, &gpos2);
		gr_set_color(0, 0, 255);
		g3_draw_sphere(&vert1, 2.0f);
		gr_set_color(255, 0, 255);
		g3_draw_sphere(&vert2, 2.0f);
		g3_draw_line(&vert1, &vert2);
	}

	Num_AI_debug_render_stuff = 0;
}

#endif



//	--------------------------------------------------------------------------
// Process subobjects of object objnum.
//	Deal with engines disabled.
void process_subobjects(int objnum)
{
	ship_subsys	*pss;
	object	*objp = &Objects[objnum];
	ship		*shipp = &Ships[objp->instance];
	ai_info	*aip = &Ai_info[shipp->ai_index];
	ship_info	*sip = &Ship_info[shipp->ship_info_index];

	//Look for enemies. If none are present, we don't have to move turrets
	int enemies_present = -1;

	model_subsystem	*psub;
	for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;

		// Don't process destroyed objects (but allow subobjects with hitpoints disabled -nuke)
		if (pss->max_hits > 0 && pss->current_hits <= 0.0f ) 
			continue;

		switch (psub->type) {
		case SUBSYSTEM_TURRET:
			// handle ending animations
			if ( (pss->turret_animation_position == MA_POS_READY) && timestamp_elapsed(pss->turret_animation_done_time) ) {
				if ( model_anim_start_type(shipp, TRIGGER_TYPE_TURRET_FIRING, pss->system_info->subobj_num, -1) ) {
					pss->turret_animation_position = MA_POS_NOT_SET;
				}
			}

			if ( psub->turret_num_firing_points > 0 )
			{
				if(enemies_present == -1)
				{
					enemies_present = 0;
					for(unsigned int i = 0; i < MAX_OBJECTS; i++)
					{
						objp = &Objects[i];
						switch(objp->type)
						{
							case OBJ_SHIP:
							case OBJ_DEBRIS:
							case OBJ_WEAPON:
								if(obj_team(objp) != shipp->team)
									enemies_present = 1;
								break;
							case OBJ_ASTEROID:
								enemies_present = 1;
								break;
						}

						if(enemies_present==1)
							break;
					}
					//Reset objp
					objp = &Objects[objnum];
				}
				//Only move turrets if enemies are present
				if(enemies_present == 1 || pss->turret_enemy_objnum >= 0)
					ai_fire_from_turret(shipp, pss, objnum);
			} else {
#ifndef NDEBUG
				if (!Msg_count_4996) {
					Warning( LOCATION, "Ship '%s' has turrets with no guns!\nProbably a model problem, so get an artist!", shipp->ship_name );
					Msg_count_4996++;
				}
#endif
				}
			break;

		case SUBSYSTEM_ENGINE:
		case SUBSYSTEM_NAVIGATION:
		case SUBSYSTEM_COMMUNICATION:
		case SUBSYSTEM_WEAPONS:
		case SUBSYSTEM_SENSORS:
		case SUBSYSTEM_UNKNOWN:
			break;

		// next set of subsystems may rotation
		case SUBSYSTEM_RADAR:
		case SUBSYSTEM_SOLAR:
		case SUBSYSTEM_GAS_COLLECT:
		case SUBSYSTEM_ACTIVATION:
			break;
		default:
			Error(LOCATION, "Illegal subsystem type.\n");
		}

		// do solar/radar/gas/activator rotation here
		ship_do_submodel_rotation(shipp, psub, pss);
	}

	//	Deal with a ship with blown out engines.
	if (ship_get_subsystem_strength(shipp, SUBSYSTEM_ENGINE) == 0.0f) {
		// Karajorma - if Player_use_ai is ever fixed to work on multiplayer it should be checked that any player ships 
		// aren't under AI control here
		if ( (!(objp->flags & OF_PLAYER_SHIP) ) && (sip->flags & (SIF_FIGHTER | SIF_BOMBER)) && !(shipp->flags & SF_DYING) ) {
			// AL: Only attack forever if not trying to depart to a docking bay.  Need to have this in, since
			//     a ship may get repaired... and it should still try to depart.  Since docking bay departures
			//     are not handled as goals, we don't want to leave the AIM_BAY_DEPART mode.
			if ( aip->mode != AIM_BAY_DEPART ) {
				ai_attack_object(objp, NULL, NULL);		//	Regardless of current mode, enter attack mode.
				aip->submode = SM_ATTACK_FOREVER;				//	Never leave attack submode, don't avoid, evade, etc.
				aip->submode_start_time = Missiontime;
			}
		}
	}
}

//	Given an object and the wing it's in, return its index in the wing list.
//	This defines its location in the wing formation.
//	If the object can't be found in the wing, return -1.
//	*objp		object of interest
//	wingnum	the wing *objp is in
int get_wing_index(object *objp, int wingnum)
{
	wing	*wingp;
	int	i;

	Assert((wingnum >= 0) && (wingnum < MAX_WINGS));

	wingp = &Wings[wingnum];

	for (i=wingp->current_count-1; i>=0; i--)
		if ( objp->instance == wingp->ship_index[i] )
			break;

	return i;		//	Note, returns -1 if string not found.
}

//	Given a wing, return a pointer to the object of its leader.
//	Asserts if object not found.
//	Currently, the wing leader is defined as the first object in the wing.
//	wingnum		Wing number in Wings array.
//	If wing leader is disabled, swap it with another ship.
object *get_wing_leader(int wingnum)
{
	wing *wingp;
	int ship_num = 0;

	if (wingnum < 0) {
		return NULL;
	}

	Assert( wingnum < MAX_WINGS );

	wingp = &Wings[wingnum];

	Assert(wingp->current_count != 0);			//	Make sure there is a leader

	ship_num = wingp->ship_index[0];

	//	If this ship is disabled, try another ship in the wing.
	int n = 0;
	while (ship_get_subsystem_strength(&Ships[ship_num], SUBSYSTEM_ENGINE) == 0.0f) {
		n++;

		if (n >= wingp->current_count) {
			break;
		}

		ship_num = wingp->ship_index[n];
	}

	if ( (n != 0) && (n != wingp->current_count) ) {
		int t = wingp->ship_index[0];
		wingp->ship_index[0] = wingp->ship_index[n];
		wingp->ship_index[n] = t;
	}

	return &Objects[Ships[ship_num].objnum];
}

#define	DEFAULT_WING_X_DELTA		1.0f
#define	DEFAULT_WING_Y_DELTA		0.25f
#define	DEFAULT_WING_Z_DELTA		0.75f
#define	DEFAULT_WING_MAG		(fl_sqrt(DEFAULT_WING_X_DELTA*DEFAULT_WING_X_DELTA + DEFAULT_WING_Y_DELTA*DEFAULT_WING_Y_DELTA + DEFAULT_WING_Z_DELTA*DEFAULT_WING_Z_DELTA))
// next constant is higher that MAX_SHIPS_IN_WINGS to deal with forming on player's wing
#define	MAX_FORMATION_ROWS		4

//	Given a position in a wing, return the desired location of the ship relative to the leader
//	*_delta_vec		OUTPUT.  delta vector based on wing_index
//	wing_index		position in wing.
void get_wing_delta(vec3d *_delta_vec, int wing_index)
{
	int	wi0;

	Assert(wing_index >= 0);

	int	k, row, column;

	int bank = wing_index / (MAX_FORMATION_ROWS*(MAX_FORMATION_ROWS+1)/2);
	wi0 = wing_index % (MAX_FORMATION_ROWS * (MAX_FORMATION_ROWS+1)/2);

	k = 0;
	for (row=1; row<MAX_FORMATION_ROWS+1; row++) {
		k += row;
		if (wi0 < k)
			break;
	}

	row--;
	column = wi0 - k + row + 1;

	_delta_vec->xyz.x = ((float) column - (float) row/2.0f) * DEFAULT_WING_X_DELTA/DEFAULT_WING_MAG;
	_delta_vec->xyz.y = ((float)row + (float)bank*2.25f) * DEFAULT_WING_Y_DELTA/DEFAULT_WING_MAG;
	_delta_vec->xyz.z = - ((float)row + 0.5f * (float) bank) * DEFAULT_WING_Z_DELTA/DEFAULT_WING_MAG;
	
}

/**
 * Compute the largest radius of a ship in a *objp's wing.
 */
float gwlr_1(object *objp, ai_info *aip)
{
	int		wingnum = aip->wing;
	float		max_radius;
	object	*o;
	ship_obj	*so;

	Assert(wingnum >= 0);

	max_radius = objp->radius;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		o = &Objects[so->objnum];
		if (Ai_info[Ships[o->instance].ai_index].wing == wingnum)
			if (o->radius > max_radius)
				max_radius = o->radius;
	}

	return max_radius;
}

/**
 * Compute the largest radius of a ship forming on *objp's wing.
 */
float gwlr_object_1(object *objp, ai_info *aip)
{
	float		max_radius;
	object	*o;
	ship_obj	*so;

	max_radius = objp->radius;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		o = &Objects[so->objnum];
		if (Ai_info[Ships[o->instance].ai_index].goal_objnum == OBJ_INDEX(objp))
			if (o->radius > max_radius)
				max_radius = o->radius;
	}

	return max_radius;
}

/**
 * For the wing that *objp is part of, return the largest ship radius in that wing.
 */
float get_wing_largest_radius(object *objp, int formation_object_flag)
{
	ship		*shipp;
	ai_info	*aip;

	Assert(objp->type == OBJ_SHIP);
	Assert((objp->instance >= 0) && (objp->instance < MAX_OBJECTS));
	shipp = &Ships[objp->instance];
	Assert((shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO));
	aip = &Ai_info[shipp->ai_index];

	if (formation_object_flag) {
		return gwlr_object_1(objp, aip);
	} else {
		return gwlr_1(objp, aip);
	}

}

float Wing_y_scale = 2.0f;
float Wing_scale = 1.0f;
DCF(wing_y_scale, "")
{
	dc_get_arg(ARG_FLOAT);
	Wing_y_scale = Dc_arg_float;
}

DCF(wing_scale, "")
{
	dc_get_arg(ARG_FLOAT);
	Wing_scale = Dc_arg_float;
}

/**
 * Given a wing leader and a position in the wing formation, return the desired absolute location to fly to.
 * @return result in *result_pos.
 */
void get_absolute_wing_pos(vec3d *result_pos, object *leader_objp, int wing_index, int formation_object_flag)
{
	vec3d	wing_delta, rotated_wing_delta;
	float		wing_spread_size;

	get_wing_delta(&wing_delta, wing_index);		//	Desired location in leader's reference frame

	wing_spread_size = MAX(50.0f, 3.0f * get_wing_largest_radius(leader_objp, formation_object_flag) + 15.0f);

	// for player obj (1) move ships up 20% (2) scale formation up 20%
	if (leader_objp->flags & OF_PLAYER_SHIP) {
		wing_delta.xyz.y *= Wing_y_scale;
		wing_spread_size *= Wing_scale;
	}

	vm_vec_scale(&wing_delta, wing_spread_size * (1.0f + leader_objp->phys_info.speed/70.0f));

	vm_vec_unrotate(&rotated_wing_delta, &wing_delta, &leader_objp->orient);	//	Rotate into leader's reference.

	vm_vec_add(result_pos, &leader_objp->pos, &rotated_wing_delta);	//	goal_point is absolute 3-space point.
}

// autopilot variant.. removes some scaling crap
void get_absolute_wing_pos_autopilot(vec3d *result_pos, object *leader_objp, int wing_index, int formation_object_flag)
{
	vec3d	wing_delta, rotated_wing_delta;
	float		wing_spread_size;

	get_wing_delta(&wing_delta, wing_index);		//	Desired location in leader's reference frame
	wing_spread_size = MAX(50.0f, 3.0f * get_wing_largest_radius(leader_objp, formation_object_flag) + 15.0f);

	vm_vec_scale(&wing_delta, wing_spread_size * 1.5f);
	vm_vec_unrotate(&rotated_wing_delta, &wing_delta, &leader_objp->orient);	//	Rotate into leader's reference.
	vm_vec_add(result_pos, &leader_objp->pos, &rotated_wing_delta);	//	goal_point is absolute 3-space point.
}

#ifndef NDEBUG
int Debug_render_wing_phantoms;

void render_wing_phantoms(object *objp)
{
	int		i;
	ship		*shipp;
	ai_info	*aip;
	int		wingnum;
	int		wing_index;		//	Index in wing struct, defines 3-space location in wing.
	vec3d	goal_point;
	
	Assert(objp->type == OBJ_SHIP);
	Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

	shipp = &Ships[objp->instance];
	Assert((shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO));

	aip = &Ai_info[shipp->ai_index];

	wingnum = aip->wing;

	if (wingnum == -1)
		return;

	wing_index = get_wing_index(objp, wingnum);

	//	If this ship is NOT the leader, abort.
	if (wing_index != 0)
		return;

	for (i=0; i<32; i++)
		if (Debug_render_wing_phantoms & (1 << i)) {
			get_absolute_wing_pos(&goal_point, objp, i, 0);
	
			vertex	vert;
			gr_set_color(255, 0, 128);
			g3_rotate_vertex(&vert, &goal_point);
			g3_draw_sphere(&vert, 2.0f);
		}

	Debug_render_wing_phantoms = 0;

}

void render_wing_phantoms_all()
{
	object	*objp;
	ship_obj	*so;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		ship		*shipp;
		ai_info	*aip;
		int		wingnum;
		int		wing_index;		//	Index in wing struct, defines 3-space location in wing.

		objp = &Objects[so->objnum];
		
		Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));
		shipp = &Ships[objp->instance];
		Assert((shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO));

		aip = &Ai_info[shipp->ai_index];

		wingnum = aip->wing;

		if (wingnum == -1)
			continue;

		wing_index = get_wing_index(objp, wingnum);

		//	If this ship is NOT the leader, abort.
		if (wing_index != 0)
			continue;
		
		render_wing_phantoms(objp);

		return;
	}
}

#endif

//	Hook from goals code to AI.
//	Force a wing to fly in formation.
//	Sets AIF_FORMATION bit in ai_flags.
//	wingnum		Wing to force to fly in formation
void ai_fly_in_formation(int wingnum)
{
	object	*objp;
	ship		*shipp;
	ship_obj	*so;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];
		Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

		shipp = &Ships[objp->instance];
		Assert((shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO));

		if (Ai_info[shipp->ai_index].wing == wingnum) {
			Ai_info[shipp->ai_index].ai_flags |= AIF_FORMATION_WING;
			Ai_info[shipp->ai_index].ai_flags &= ~AIF_FORMATION_OBJECT;
		}
	}
}

//	Hook from goals code to AI.
//	Force a wing to abandon formation flying.
//	Clears AIF_FORMATION bit in ai_flags.
//	wingnum		Wing to force to fly in formation
void ai_disband_formation(int wingnum)
{
	object	*objp;
	ship		*shipp;
	ship_obj	*so;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];
		Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

		shipp = &Ships[objp->instance];
		Assert((shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO));

		if (Ai_info[shipp->ai_index].wing == wingnum) {
			Ai_info[shipp->ai_index].ai_flags &= ~AIF_FORMATION_WING;
		}
	}
}

float	Leader_chaos = 0.0f;
int Chaos_frame = -1;

//	Return true if objp is flying in an erratic manner
//	Only true if objp is a player
int formation_is_leader_chaotic(object *objp)
{
	if (Game_mode & GM_MULTIPLAYER)
		return 0;

	if (objp != Player_obj)
		return 0;

	if (Framecount != Chaos_frame) {
		float	speed_scale;
		float	fdot, udot;

		speed_scale = 3.0f + objp->phys_info.speed * 0.1f;

		fdot = 5.0f * (1.0f - vm_vec_dot(&objp->orient.vec.fvec, &objp->last_orient.vec.fvec)) * flFrametime;
		udot = 8.0f * (1.0f - vm_vec_dot(&objp->orient.vec.uvec, &objp->last_orient.vec.uvec)) * flFrametime;

		Leader_chaos += fdot * speed_scale + udot * speed_scale;

		Leader_chaos *= (1.0f - flFrametime*0.2f);

		if (Leader_chaos < 0.0f)
			Leader_chaos = 0.0f;
		else if (Leader_chaos > 1.7f)
			Leader_chaos = 1.7f;

		Chaos_frame = Framecount;
	}

	return (Leader_chaos > 1.0f);
}

void ai_most_massive_object_of_its_wing_of_all_docked_objects_helper(object *objp, dock_function_info *infop)
{
	// check that I am a ship
	if (objp->type == OBJ_SHIP)
	{
		// check that wings match
		if (Ai_info[Ships[objp->instance].ai_index].wing == infop->parameter_variables.int_value)
		{
			// if this guy has a higher mass, he is now the most massive object
			if (objp->phys_info.mass > infop->maintained_variables.objp_value->phys_info.mass)
			{
				infop->maintained_variables.objp_value = objp;
			}
			// if masses are equal, then check if this guy has a higher signature - if so, he is now the most massive object
			else if (objp->phys_info.mass == infop->maintained_variables.objp_value->phys_info.mass)
			{
				if (objp->signature > infop->maintained_variables.objp_value->signature)
				{
					infop->maintained_variables.objp_value = objp;
				}
			}
		}
	}
}

// Fly in formation.
//	Make Pl_objp assume its proper place in formation.
//	If the leader of the wing is doing something stupid, like fighting a battle,
//	then the poor sap wingmates will be in for a "world of hurt"
//	Return TRUE if we need to process this object's normal mode
int ai_formation()
{
	object	*leader_objp;
	ship		*shipp;
	ai_info	*aip, *laip;
	int		wingnum;
	int		wing_index;		//	Index in wing struct, defines 3-space location in wing.
	vec3d	goal_point, future_goal_point_5, future_goal_point_2, future_goal_point_x, future_goal_point_1000x, vec_to_goal, dir_to_goal;
	float		dot_to_goal, dist_to_goal, leader_speed;

	Assert(Pl_objp->type == OBJ_SHIP);
	Assert((Pl_objp->instance >= 0) && (Pl_objp->instance < MAX_SHIPS));

	shipp = &Ships[Pl_objp->instance];

	Assert((shipp->ai_index >= 0) && (shipp->ai_index < MAX_AI_INFO));

	aip = &Ai_info[shipp->ai_index];

	Assert((aip->ai_flags & AIF_FORMATION) != AIF_FORMATION);	//	Make sure not both types of formation flying in effect.

	//	Determine which kind of formation flying.
	//	If tracking an object, not in waypoint mode:
	if (aip->ai_flags & AIF_FORMATION_OBJECT) {
		if ((aip->goal_objnum < 0) || (aip->goal_objnum >= MAX_OBJECTS) || (aip->mode == AIM_BAY_DEPART)) {
			aip->ai_flags &= ~AIF_FORMATION_OBJECT;
			return 1;
		}
		
		wing_index = ai_formation_object_get_slotnum(aip->goal_objnum, Pl_objp);
		leader_objp = &Objects[aip->goal_objnum];
	} else {	//	Formation flying in waypoint mode.
		Assert(aip->ai_flags & AIF_FORMATION_WING);

		if ( (aip->mode != AIM_WAYPOINTS) && (aip->mode != AIM_FLY_TO_SHIP) ) {
			aip->ai_flags &= ~AIF_FORMATION_WING;
			return 1;
		}

		wingnum = aip->wing;

		if (wingnum == -1) {
			return 1;
		}

		// disable formation flying for any ship in the players wing
		// ... except when using auto-pilot
		if ( !AutoPilotEngaged ) {
			if (wingnum == Ships[Player_obj->instance].wingnum) {
				return 1;
			}
		}

		wing_index = get_wing_index(Pl_objp, wingnum);

		leader_objp = get_wing_leader(wingnum);
	}

	// if Pl_objp is docked with a ship in his own wing, only the most massive one
	// in the whole assembly actually flies in formation
	// Goober5000 - this is really stupid code
	if ( object_is_docked(Pl_objp) ) {
		// assume I am the most massive
		dock_function_info dfi;
		dfi.parameter_variables.int_value = aip->wing;
		dfi.maintained_variables.objp_value = Pl_objp;
		
		// check docked objects
		dock_evaluate_all_docked_objects(Pl_objp, &dfi, ai_most_massive_object_of_its_wing_of_all_docked_objects_helper);

		// if I am not the most massive, return
		if (dfi.maintained_variables.objp_value != Pl_objp)
		{
			return 0;
		}
	}

	Assert(leader_objp != NULL);
	laip = &Ai_info[Ships[leader_objp->instance].ai_index];

	//	Make sure we're really in this wing.
	if (wing_index == -1) {
		return 1;
	}

	// skip ourselves, if we happen to be the leader
	if (leader_objp == Pl_objp) {
		return 1;
	}
	
	if (aip->mode == AIM_WAYPOINTS) {
		aip->wp_list = laip->wp_list;
		aip->wp_index = laip->wp_index;
		aip->wp_flags = laip->wp_flags;

		if ((aip->wp_list != NULL) && (aip->wp_index == aip->wp_list->get_waypoints().end()))
			--aip->wp_index;
	}

	#ifndef NDEBUG
	Debug_render_wing_phantoms |= (1 << wing_index);
	#endif

	leader_speed = leader_objp->phys_info.speed;
	vec3d leader_vec = leader_objp->phys_info.vel;

	get_absolute_wing_pos(&goal_point, leader_objp, wing_index, aip->ai_flags & AIF_FORMATION_OBJECT);
	vm_vec_scale_add(&future_goal_point_5, &goal_point, &leader_vec, 10.0f);
	vm_vec_scale_add(&future_goal_point_2, &goal_point, &leader_vec, 5.0f);
	vm_vec_scale_add(&future_goal_point_x, &goal_point, &leader_objp->orient.vec.fvec, 10.0f);	//	used when very close to destination
	vm_vec_scale_add(&future_goal_point_1000x, &goal_point, &leader_objp->orient.vec.fvec, 1000.0f);	//	used when very close to destination

	//	Now, get information telling this object how to turn and accelerate to get to its
	//	desired location.
	vm_vec_sub(&vec_to_goal, &goal_point, &Pl_objp->pos);
	if ( vm_vec_mag_quick(&vec_to_goal) < AICODE_SMALL_MAGNITUDE )
		vec_to_goal.xyz.x += 0.1f;

	vm_vec_copy_normalize(&dir_to_goal, &vec_to_goal);
	dot_to_goal = vm_vec_dot(&dir_to_goal, &Pl_objp->orient.vec.fvec);
	dist_to_goal = vm_vec_dist_quick(&Pl_objp->pos, &goal_point);
	float	dist_to_goal_2 = vm_vec_dist_quick(&Pl_objp->pos, &future_goal_point_2);

	int	chaotic_leader = 0;

	chaotic_leader = formation_is_leader_chaotic(leader_objp);	//	Set to 1 if leader is player and flying erratically.  Causes ships to not aggressively pursue formation location.

	if (dist_to_goal > 500.0f) {
		turn_towards_point(Pl_objp, &goal_point, NULL, 0.0f);
		accelerate_ship(aip, 1.0f);
	} else if (dist_to_goal > 200.0f) {
		if (dot_to_goal > -0.5f) {
			turn_towards_point(Pl_objp, &goal_point, NULL, 0.0f);
			float range_speed = shipp->current_max_speed - leader_speed;
			if (range_speed > 0.0f)
				set_accel_for_target_speed(Pl_objp, leader_speed + range_speed * (dist_to_goal+100.0f)/500.0f);
			else
				set_accel_for_target_speed(Pl_objp, shipp->current_max_speed);
		} else {
			turn_towards_point(Pl_objp, &future_goal_point_5, NULL, 0.0f);
			if (leader_speed > 10.0f)
				set_accel_for_target_speed(Pl_objp, leader_speed *(1.0f + dot_to_goal));
			else
				set_accel_for_target_speed(Pl_objp, 10.0f);
		}
	} else {
		vec3d	v2f2;
		float	dot_to_f2;
		float	dist_to_f2;

		dist_to_f2 = vm_vec_normalized_dir(&v2f2, &future_goal_point_2, &Pl_objp->pos);
		dot_to_f2 = vm_vec_dot(&v2f2, &Pl_objp->orient.vec.fvec);

		//	Leader flying like a maniac.  Don't try hard to form on wing.
		if (chaotic_leader) {
			turn_towards_point(Pl_objp, &future_goal_point_2, NULL, 0.0f);
			set_accel_for_target_speed(Pl_objp, MIN(leader_speed*0.8f, 20.0f));
		} else if (dist_to_goal > 75.0f) {
			turn_towards_point(Pl_objp, &future_goal_point_2, NULL, 0.0f);
			float	delta_speed;
			float range_speed = shipp->current_max_speed - leader_speed;
			if (range_speed > 0.0f)
				delta_speed = dist_to_goal_2/500.0f * range_speed;
			else
				delta_speed = shipp->current_max_speed - leader_speed;
			if (dot_to_goal < 0.0f) {
				delta_speed = -delta_speed;
				if (-delta_speed > leader_speed/2)
					delta_speed = -leader_speed/2;
			}

			if (leader_speed < 5.0f)
				if (delta_speed < 5.0f)
					delta_speed = 5.0f;

			float scale = dot_to_f2;
			if (scale < 0.1f)
				scale = 0.0f;
			else
				scale *= scale;

			set_accel_for_target_speed(Pl_objp, scale * (leader_speed + delta_speed));
		} else {

			if (leader_speed < 5.0f) {
				//	Leader very slow.  If not close to goal point, get very close.  Note, keep trying to get close unless
				//	moving very slowly, else momentum can carry far away from goal.

				if ((dist_to_goal > 10.0f) || ((Pl_objp->phys_info.speed > leader_speed + 2.5f) && (dot_to_goal > 0.5f))) {
					turn_towards_point(Pl_objp, &goal_point, NULL, 0.0f);
					set_accel_for_target_speed(Pl_objp, leader_speed + dist_to_goal/10.0f);
				} else {
					if (Pl_objp->phys_info.speed < 0.5f) {
						turn_towards_point(Pl_objp, &future_goal_point_1000x, NULL, 0.0f);
					}
					set_accel_for_target_speed(Pl_objp, leader_speed);
				}
			} else if (dist_to_goal > 10.0f) {
				float	dv;

				turn_towards_point(Pl_objp, &future_goal_point_2, NULL, 0.0f);

				if (dist_to_goal > 25.0f) {
					if (dot_to_goal < 0.3f)
						dv = -0.1f;
					else
						dv = dot_to_goal - 0.2f;

					set_accel_for_target_speed(Pl_objp, leader_speed + dist_to_goal/5.0f * dv);
				} else {
					set_accel_for_target_speed(Pl_objp, leader_speed + 1.5f * dot_to_goal - 1.0f);
				}
			} else {
				if (Pl_objp->phys_info.speed < 0.1f)
					turn_towards_point(Pl_objp, &future_goal_point_1000x, NULL, 0.0f);
				else
					turn_towards_point(Pl_objp, &future_goal_point_x, NULL, 0.0f);

				// Goober5000 7/5/2006 changed to leader_speed from 0.0f
				set_accel_for_target_speed(Pl_objp, leader_speed);
			}
		}

	}

	//	See how different this ship's bank is relative to wing leader
	float	up_dot = vm_vec_dot(&leader_objp->orient.vec.uvec, &Pl_objp->orient.vec.uvec);
	if (up_dot < 0.996f) {
		vec3d	w_out;
		matrix	new_orient;
		vec3d	angular_accel;

		vm_vec_copy_scale(&angular_accel, &Pl_objp->phys_info.max_rotvel, 0.2f);
		vm_matrix_interpolate(&leader_objp->orient, &Pl_objp->orient, &Pl_objp->phys_info.rotvel, flFrametime, &new_orient, &w_out, &Pl_objp->phys_info.max_rotvel, &angular_accel, 1);

		Pl_objp->orient = new_orient;
		Pl_objp->phys_info.rotvel = w_out;
	} else {
		Pl_objp->phys_info.rotvel.xyz.z = 0.0f;
	}

	return 0;
}

/**
 * If object *objp is being repaired, deal with it!
 */
void ai_do_repair_frame(object *objp, ai_info *aip, float frametime)
{
	static bool rearm_eta_found=false;

	if (Ships[objp->instance].team == Iff_traitor) {
		ai_abort_rearm_request(objp);
		return;
	}

	if (aip->ai_flags & (AIF_BEING_REPAIRED | AIF_AWAITING_REPAIR)) {
		int	support_objnum;
		
		ai_info	*repair_aip;

		support_objnum = aip->support_ship_objnum;

		if (support_objnum == -1)
			return;

		//	Curious -- object numbers match, but signatures do not.
		//	Must mean original repair ship died and was replaced by current ship.
		Assert(Objects[support_objnum].signature == aip->support_ship_signature);
	
		repair_aip = &Ai_info[Ships[Objects[support_objnum].instance].ai_index];

		if (aip->ai_flags & AIF_BEING_REPAIRED) {

			//	Wait awhile into the mode to synchronize with sound effect.
			if (Missiontime - repair_aip->submode_start_time > REARM_SOUND_DELAY) {
				int repaired;

				if (!rearm_eta_found && objp==Player_obj && Player_rearm_eta <= 0) 
				{
					rearm_eta_found = true;
					Player_rearm_eta = ship_calculate_rearm_duration(objp);
				}

				repaired = ship_do_rearm_frame( objp, frametime );		// hook to do missile rearming

				//	See if fully repaired.  If so, cause process to stop.
				if ( repaired && (repair_aip->submode == AIS_DOCK_4)) {

					repair_aip->submode = AIS_UNDOCK_0;
					repair_aip->submode_start_time = Missiontime;

					// if repairing player object -- tell him done with repair
					if ( !MULTIPLAYER_CLIENT ) {
						ai_do_objects_repairing_stuff( objp, &Objects[support_objnum], REPAIR_INFO_COMPLETE );
					}
				}
			}
		} else if (aip->ai_flags & AIF_AWAITING_REPAIR) {
			//	If this ship has been awaiting repair for 90+ seconds, abort.
			if ( !MULTIPLAYER_CLIENT ) {
				if ((Game_mode & GM_MULTIPLAYER) || (objp != Player_obj)) {
					if ((repair_aip->goal_objnum == OBJ_INDEX(objp)) && (timestamp_elapsed(aip->abort_rearm_timestamp))) {
						ai_abort_rearm_request(objp);
						aip->next_rearm_request_timestamp = timestamp(NEXT_REARM_TIMESTAMP);
					}
				}
			}
		}
	} else {
		// AL 11-24-97: If this is the player ship, ensure the repair sound isn't playing.  We need to
		//              do this check, since this is a looping sound, and may continue on if rearm/repair
		//              finishes abnormally once sound begins looping.
		if ( objp == Player_obj ) {
			Player_rearm_eta = 0;
			rearm_eta_found = false;
			player_stop_repair_sound();
		}
	}
}

//	Shell around dock_orient_and_approach to detect whether dock process should be aborted.
//	Goober5000 - The child should always keep up with the parent.
void call_doa(object *child, object *parent)
{
	Assert(dock_check_find_direct_docked_object(parent, child));

	// get indexes
	int parent_index = dock_find_dockpoint_used_by_object(parent, child);
	int child_index = dock_find_dockpoint_used_by_object(child, parent);

	// child keeps up with parent
	dock_orient_and_approach(child, child_index, parent, parent_index, DOA_DOCK_STAY);
}

//	Maybe launch a countermeasure.
//	Also, detect a supposed homing missile that no longer exists.
void ai_maybe_launch_cmeasure(object *objp, ai_info *aip)
{
	float			dist;
	ship_info	*sip;
	ship			*shipp;

	shipp = &Ships[objp->instance];
	sip = &Ship_info[shipp->ship_info_index];

	if (!(sip->flags & (SIF_SMALL_SHIP | SIF_TRANSPORT)))
		return;

	if (!shipp->cmeasure_count)
		return;

	if ( !timestamp_elapsed(shipp->cmeasure_fire_stamp) )
		return;

	//	If not on player's team and Skill_level + ai_class is low, never fire a countermeasure.  The ship is too dumb.
	if (iff_x_attacks_y(Player_ship->team, shipp->team)) {
		//SUSHI: Only bail if autoscale is on...
		if (aip->ai_class_autoscale && Game_skill_level + aip->ai_class < 4){
			return;
		}
	}

	if ((aip->nearest_locked_object != -1) && (Objects[aip->nearest_locked_object].type == OBJ_WEAPON)) {
		object	*weapon_objp;
		weapon	*weaponp;
		weapon_info	*wip;

		weapon_objp = &Objects[aip->nearest_locked_object];
		weaponp = &Weapons[weapon_objp->instance];
		wip = &Weapon_info[weaponp->weapon_info_index];

		if ((dist = vm_vec_dist_quick(&objp->pos, &weapon_objp->pos)) < weapon_objp->phys_info.speed*2.0f) {
	
			aip->nearest_locked_distance = dist;
			//	Verify that this object is really homing on us.

			float	fire_chance;

			//	For ships on player's team, have constant, average chance to fire.
			//	For enemies, increasing chance with higher skill level.
			if (shipp->team == Player_ship->team)
				fire_chance = The_mission.ai_profile->cmeasure_fire_chance[NUM_SKILL_LEVELS/2];
			else
				fire_chance = aip->ai_cmeasure_fire_chance;

			//	Decrease chance to fire at lower ai class (SUSHI: Only if autoscale is on)
			if (aip->ai_class_autoscale)
				fire_chance *= (float) aip->ai_class/Num_ai_classes;

			float r = frand();
			if (fire_chance < r) {
				shipp->cmeasure_fire_stamp = timestamp(CMEASURE_WAIT + (int) (fire_chance*2000));		//	Wait 1/2 second (CMEASURE_WAIT) + additional delay to decrease chance of firing very soon.
				return;
			}

			if (weapon_objp->type == OBJ_WEAPON) {
				if (weapon_objp->instance >= 0) {
					ship_launch_countermeasure(objp);
					shipp->cmeasure_fire_stamp = timestamp(2*CMEASURE_WAIT);
					return;
				}
			}
	
		}
	}

	return;
}

//	--------------------------------------------------------------------------
void ai_preprocess_ignore_objnum(object *objp, ai_info *aip)
{
	if (aip->ai_flags & AIF_TEMPORARY_IGNORE)
	{
		if (timestamp_elapsed(aip->ignore_expire_timestamp))
			aip->ignore_objnum = UNUSED_OBJNUM;
	}

	if (is_ignore_object(aip, aip->goal_objnum))
	{
		aip->goal_objnum = -1;

		// AL 12-11-97: If in STRAFE mode, we need to ensure that target_objnum is also
		//              set to -1
		if (aip->mode == AIM_STRAFE)
			aip->target_objnum = -1;
	}

	if (is_ignore_object(aip, aip->target_objnum))
		aip->target_objnum = -1;
}

#define	CHASE_CIRCLE_DIST		100.0f

void ai_chase_circle(object *objp)
{
	float		dist_to_goal;
	float		target_speed;
	vec3d	goal_point;
	ship_info	*sip;
	ai_info		*aip;

	sip = &Ship_info[Ships[Pl_objp->instance].ship_info_index];

	target_speed = sip->max_speed/4.0f;
	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	Assert(vm_vec_mag(&aip->goal_point) >= 0.0f);		//	Supposedly detects bogus vector

	goal_point = aip->goal_point;

	if (aip->ignore_objnum == UNUSED_OBJNUM) {
		dist_to_goal = vm_vec_dist_quick(&aip->goal_point, &objp->pos);

		if (dist_to_goal > 2*CHASE_CIRCLE_DIST) {
			vec3d	vec_to_goal;
			//	Too far from circle goal, create a new goal point.
			vm_vec_normalized_dir(&vec_to_goal, &aip->goal_point, &objp->pos);
			vm_vec_scale_add(&aip->goal_point, &objp->pos, &vec_to_goal, CHASE_CIRCLE_DIST);
		}

		goal_point = aip->goal_point;
	} else if (is_ignore_object(aip, aip->ignore_objnum)) {
		object	*ignore_objp = &Objects[aip->ignore_objnum];

		vec3d	tvec1;
		float		dist;

		dist = vm_vec_normalized_dir(&tvec1, &Pl_objp->pos, &ignore_objp->pos);

		if (dist < ignore_objp->radius*2 + 1500.0f) {
			vm_vec_scale_add(&goal_point, &Pl_objp->pos, &tvec1, ignore_objp->radius*2 + 1400.0f);
			if (dist < ignore_objp->radius*2 + 1300.0f)
				target_speed = sip->max_speed * (1.25f - dist/(ignore_objp->radius*2 + 1500.0f));
		}
	}

	Assert(vm_vec_mag(&aip->goal_point) >= 0.0f);		//	Supposedly detects bogus vector

	turn_towards_tangent(Pl_objp, &goal_point, 10*objp->radius + 200.0f);

	set_accel_for_target_speed(Pl_objp, target_speed);

}

#define	SHIELD_BALANCE_RATE	0.2f		//	0.1f -> takes 10 seconds to equalize shield.

/**
 * Transfer shield energy to most recently hit section from others.
 */
void ai_transfer_shield(object *objp, int quadrant_num)
{
	int	i;
	float	transfer_amount;
	float	transfer_delta;
	float	max_quadrant_strength;

	max_quadrant_strength = get_max_shield_quad(objp);

	transfer_amount = 0.0f;
	transfer_delta = (SHIELD_BALANCE_RATE/2) * max_quadrant_strength;

	if (objp->shield_quadrant[quadrant_num] + (MAX_SHIELD_SECTIONS-1)*transfer_delta > max_quadrant_strength)
		transfer_delta = (max_quadrant_strength - objp->shield_quadrant[quadrant_num])/(MAX_SHIELD_SECTIONS-1);

	for (i=0; i<MAX_SHIELD_SECTIONS; i++)
		if (i != quadrant_num) {
			if (objp->shield_quadrant[i] >= transfer_delta) {
				objp->shield_quadrant[i] -= transfer_delta;
				transfer_amount += transfer_delta;
			} else {
				transfer_amount += objp->shield_quadrant[i];
				objp->shield_quadrant[i] = 0.0f;
			}
		}

	objp->shield_quadrant[quadrant_num] += transfer_amount;
}

void ai_balance_shield(object *objp)
{
	int	i;
	float	shield_strength_avg;
	float	delta;

	// if we are already at the max shield strength for all quads then just bail now
	if ( Ships[objp->instance].ship_max_shield_strength == shield_get_strength(objp) )
		return;


	shield_strength_avg = shield_get_strength(objp)/MAX_SHIELD_SECTIONS;

	delta = SHIELD_BALANCE_RATE * shield_strength_avg;

	for (i=0; i<MAX_SHIELD_SECTIONS; i++) {
		if (objp->shield_quadrant[i] < shield_strength_avg) {
			// only do it the retail way if using smart shields (since that's a bigger thing) - taylor
			if (Ai_info[Ships[objp->instance].ai_index].ai_profile_flags & AIPF_SMART_SHIELD_MANAGEMENT)
				shield_add_strength(objp, delta);
			else
				objp->shield_quadrant[i] += delta/MAX_SHIELD_SECTIONS;

			if (objp->shield_quadrant[i] > shield_strength_avg)
				objp->shield_quadrant[i] = shield_strength_avg;

		} else {
			// only do it the retail way if using smart shields (since that's a bigger thing) - taylor
			if (Ai_info[Ships[objp->instance].ai_index].ai_profile_flags & AIPF_SMART_SHIELD_MANAGEMENT)
				shield_add_strength(objp, -delta);
			else
				objp->shield_quadrant[i] -= delta/MAX_SHIELD_SECTIONS;

			if (objp->shield_quadrant[i] < shield_strength_avg)
				objp->shield_quadrant[i] = shield_strength_avg;
		}
	}
}

//	Manage the shield for this ship.
//	Try to max out the side that was most recently hit.
void ai_manage_shield(object *objp, ai_info *aip)
{
	ship_info *sip;

	sip = &Ship_info[Ships[objp->instance].ship_info_index];

	if (timestamp_elapsed(aip->shield_manage_timestamp)) {
		float delay;

		//	Scale time until next manage shield based on Skill_level.
		//	Ships on player's team are treated as if Skill_level is average.
		if (iff_x_attacks_y(Player_ship->team, Ships[objp->instance].team))
		{
			delay = aip->ai_shield_manage_delay;
		} 
		else 
		{
			delay = The_mission.ai_profile->shield_manage_delay[NUM_SKILL_LEVELS/2];
		}

		//	Scale between 1x and 3x based on ai_class (SUSHI: only if autoscale is on)
		if (aip->ai_class_autoscale)
			delay = delay + delay * (float) (3*(Num_ai_classes - aip->ai_class - 1) / (Num_ai_classes - 1));

		// set timestamp
		aip->shield_manage_timestamp = timestamp((int) (delay * 1000.0f));

		if (sip->flags & SIF_SMALL_SHIP || (aip->ai_profile_flags2 & AIPF2_ALL_SHIPS_MANAGE_SHIELDS)) {
			if (Missiontime - aip->last_hit_time < F1_0*10)
				ai_transfer_shield(objp, aip->last_hit_quadrant);
			else
				ai_balance_shield(objp);
		}
	}
}

//	See if object *objp should evade an incoming missile.
//	*aip is the ai_info pointer within *objp.
void ai_maybe_evade_locked_missile(object *objp, ai_info *aip)
{
	ship			*shipp;
	ship_info	*sip;

	shipp = &Ships[objp->instance];
	sip = &Ship_info[shipp->ship_info_index];

	//	Only small ships evade an incoming missile.  Why would a capital ship try to swerve?
	if (!(Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_SMALL_SHIP)) {
		return;
	}

	if (aip->ai_flags & (AIF_NO_DYNAMIC | AIF_KAMIKAZE)) {	//	If not allowed to pursue dynamic objectives, don't evade.  Dumb?  Maybe change. -- MK, 3/15/98
		return;
	}

	if (aip->nearest_locked_object != -1) {
		object	*missile_objp;

		missile_objp = &Objects[aip->nearest_locked_object];

		if (Weapons[missile_objp->instance].homing_object != objp) {
			aip->nearest_locked_object = -1;
			return;
		}

		if ((missile_objp->type == OBJ_WEAPON) && (Weapon_info[Weapons[missile_objp->instance].weapon_info_index].wi_flags & WIF_HOMING)) {
			float dist = vm_vec_dist_quick(&missile_objp->pos, &objp->pos);
			float dist2 = 4.0f  * vm_vec_mag_quick(&missile_objp->phys_info.vel);			
			if (dist < dist2) {
				switch (aip->mode) {
				//	If in AIM_STRAFE mode, don't evade if parent of weapon is targeted ship.
				case AIM_STRAFE:
					if ((missile_objp->parent != -1) && (missile_objp->parent == aip->target_objnum)) {
						;
					} else {
						;		//	Alan -- If you want to handle incoming weapons from someone other than the ship
								//	the strafing ship is attacking, do it here.
					}
					break;
				case AIM_CHASE:
					//	Don't always go into evade weapon mode.  Usually, a countermeasure gets launched.
					// If low on countermeasures, more likely to try to evade.  If 8+, never evade due to low cmeasures.
					if (((((Missiontime >> 18) ^ OBJ_INDEX(objp)) & 3) == 0) || 
						(objp->phys_info.speed < 40.0f) ||
						(frand() < 1.0f - (float) shipp->cmeasure_count/8.0f)) {
						if (aip->submode != SM_ATTACK_FOREVER) {	//	SM_ATTACK_FOREVER means engines blown.
							aip->submode = SM_EVADE_WEAPON;
							aip->submode_start_time = Missiontime;
						}
					}
					break;
				case AIM_DOCK:	//	Ships in dock mode can evade iif they are not currently repairing or docked.
					if (object_is_docked(objp) || (aip->ai_flags & (AIF_REPAIRING|AIF_BEING_REPAIRED)))
						break;
				case AIM_GUARD:
					//	If in guard mode and far away from guard object, don't pursue guy that hit me.
					if ((aip->guard_objnum != -1) && (aip->guard_signature == Objects[aip->guard_objnum].signature)) {
						if (vm_vec_dist_quick(&objp->pos, &Objects[aip->guard_objnum].pos) > 500.0f) {
							return;
						}
					}
				case AIM_EVADE:
				case AIM_GET_BEHIND:
				case AIM_STAY_NEAR:
				case AIM_STILL:
 				case AIM_AVOID:
				case AIM_WAYPOINTS:
				case AIM_NONE:
				case AIM_BIGSHIP:
				case AIM_PATH:
				case AIM_BE_REARMED:
				case AIM_SAFETY:
				case AIM_BAY_EMERGE:
				case AIM_FLY_TO_SHIP:
					aip->active_goal = AI_ACTIVE_GOAL_DYNAMIC;
					aip->previous_mode = aip->mode;
					aip->previous_submode = aip->submode;
					aip->mode = AIM_EVADE_WEAPON;
					aip->submode = -1;
					aip->submode_start_time = Missiontime;
					aip->mode_time = timestamp(MAX_EVADE_TIME);	//	Max time to evade.
					break;
				case AIM_EVADE_WEAPON:		//	Note: We don't want to change mode on another evasion, or previous_mode will get bashed.
				case AIM_PLAY_DEAD:
				case AIM_BAY_DEPART:
				case AIM_SENTRYGUN:
					break;
				case AIM_WARP_OUT:
					break;
				default:
					Int3();			//	Hey, what mode is it?
					break;
				}
			}
		} else {
			aip->nearest_locked_object = -1;
		}
	}
}

//	Maybe evade a dumbfire weapon that was fired when Pl_objp was targeted.
//	Have an 80% chance of evading in a second
void maybe_evade_dumbfire_weapon(ai_info *aip)
{
	//	Only small ships evade an incoming missile.  Why would a capital ship try to swerve?
	if (!(Ship_info[Ships[Pl_objp->instance].ship_info_index].flags & SIF_SMALL_SHIP)) {
		return;
	}

	//	Make sure in a mode in which we evade dumbfire weapons.
	switch (aip->mode) {
	case AIM_CHASE:
		if (aip->submode == SM_ATTACK_FOREVER) {
			return;
		}
	case AIM_GUARD:
		//	If in guard mode and far away from guard object, don't pursue guy that hit me.
		if ((aip->guard_objnum != -1) && (aip->guard_signature == Objects[aip->guard_objnum].signature)) {
			if (vm_vec_dist_quick(&Objects[Ships[aip->shipnum].objnum].pos, &Objects[aip->guard_objnum].pos) > 500.0f) {
				return;
			}
		}
 	case AIM_STILL:
	case AIM_STAY_NEAR:
	case AIM_EVADE:
	case AIM_GET_BEHIND:
	case AIM_AVOID:
	case AIM_PATH:
	case AIM_NONE:
	case AIM_WAYPOINTS:
	case AIM_SAFETY:
		break;
	case AIM_STRAFE:
	case AIM_BIGSHIP:
	case AIM_DOCK:
	case AIM_PLAY_DEAD:
	case AIM_EVADE_WEAPON:
	case AIM_BAY_EMERGE:
	case AIM_BAY_DEPART:
	case AIM_SENTRYGUN:
	case AIM_WARP_OUT:
	case AIM_FLY_TO_SHIP:
		return;
	default:
		Int3();	//	Bogus mode!
		return;
	}

	if (is_instructor(&Objects[Ships[aip->shipnum].objnum]))
		return;	//	Instructor doesn't evade.

	float t = ai_endangered_by_weapon(aip);
	if ((t > 0.0f) && (t < 1.0f)) {
	// Check if this weapon is from a large ship Pl_objp is attacking... if so, enter strafe mode
		if ( !(aip->ai_flags & AIF_NO_DYNAMIC) ) {
			if ( ai_big_maybe_enter_strafe_mode(Pl_objp, aip->danger_weapon_objnum) ) {
				return;
			}
		}

		switch (aip->mode) {
		case AIM_CHASE:
			switch (aip->submode) {
			case SM_EVADE:
			case SM_ATTACK_FOREVER:
			case SM_AVOID:
			case SM_GET_AWAY:
			case SM_EVADE_WEAPON:
				break;
			default:
				if (ai_near_full_strength(Pl_objp)) {
					aip->submode = SM_SUPER_ATTACK;
					aip->submode_start_time = Missiontime;
					aip->last_attack_time = Missiontime;
				} else {
					aip->submode = SM_EVADE_WEAPON;
					aip->submode_start_time = Missiontime;
				}
				break;
			}
			break;
		case AIM_GUARD:
		case AIM_STILL:
		case AIM_STAY_NEAR:
		case AIM_EVADE:
		case AIM_GET_BEHIND:
		case AIM_AVOID:
		case AIM_PATH:
		case AIM_NONE:
		case AIM_WAYPOINTS:	
		case AIM_FLY_TO_SHIP:
		case AIM_SAFETY:
			if (!(aip->ai_flags & (AIF_NO_DYNAMIC | AIF_KAMIKAZE)) && (Ship_info[Ships[aip->shipnum].ship_info_index].flags & SIF_SMALL_SHIP)) {
				aip->active_goal = AI_ACTIVE_GOAL_DYNAMIC;
				aip->previous_mode = aip->mode;
				aip->previous_submode = aip->submode;
				aip->mode = AIM_EVADE_WEAPON;
				aip->submode = -1;
				aip->submode_start_time = Missiontime;
				aip->mode_time = timestamp(MAX_EVADE_TIME);	//	Evade for up to five seconds.
			}
			break;
		case AIM_STRAFE:
		case AIM_BIGSHIP:
		case AIM_DOCK:
		case AIM_PLAY_DEAD:
		case AIM_EVADE_WEAPON:
		case AIM_BAY_EMERGE:
		case AIM_BAY_DEPART:
		case AIM_SENTRYGUN:
			break;
		default:
			Int3();	//	Bogus mode!
		}
	}
}

// manage the opening and closing of fighter bay related subobject animations
// input:	pl_objp  =>   the object which is/has entered/exited the fighter bay
//          aip      =>   ai info for said object
//          done     =>   true if the object has is finished with the path, or is dead
void ai_manage_bay_doors(object *pl_objp, ai_info *aip, bool done)
{
	Assert( pl_objp );
	Assert( pl_objp->instance >= 0 );
	Assert( aip );

	ship *shipp = &Ships[pl_objp->instance];

	if (done && !shipp->bay_doors_need_open)
		return;

	if (done)
		shipp->bay_doors_need_open = false;

	// if this happens then the parent was probably destroyed/departed, so just skip the rest
	if (shipp->bay_doors_parent_shipnum < 0)
		return;

	ship *parent_ship = &Ships[shipp->bay_doors_parent_shipnum];

	if (done)
		parent_ship->bay_doors_wanting_open--;

	// trigger an open if we need it
	if ( (parent_ship->bay_doors_status == MA_POS_NOT_SET) && (parent_ship->bay_doors_wanting_open > 0) ) {
		if ( model_anim_start_type(parent_ship, TRIGGER_TYPE_DOCK_BAY_DOOR, shipp->bay_doors_launched_from, 1) ) {
			parent_ship->bay_doors_status = MA_POS_SET;
			parent_ship->bay_doors_anim_done_time = model_anim_get_time_type(parent_ship, TRIGGER_TYPE_DOCK_BAY_DOOR, shipp->bay_doors_launched_from);
		} else {
			parent_ship->bay_doors_status = MA_POS_READY;
		}
	}

	// if we are already open, and no longer need to be, then close the doors
	if ( (parent_ship->bay_doors_status == MA_POS_READY) && (parent_ship->bay_doors_wanting_open <= 0) ) {
		model_anim_start_type(parent_ship, TRIGGER_TYPE_DOCK_BAY_DOOR, shipp->bay_doors_launched_from, -1);
		parent_ship->bay_doors_status = MA_POS_NOT_SET;
	}

	if ( (parent_ship->bay_doors_status == MA_POS_SET) && timestamp_elapsed(parent_ship->bay_doors_anim_done_time) ) {
		parent_ship->bay_doors_status = MA_POS_READY;
	}
}

// determine what path to use when emerging from a fighter bay
// input:	pl_objp	=>	pointer to object for ship that is arriving
//				pos		=>	output parameter, it is the starting world pos for path choosen
//				fvec		=>	output parameter, this is the forward vector that ship has when arriving
//
// exit:		-1		=>	path could not be located
//				 0		=> success
int ai_acquire_emerge_path(object *pl_objp, int parent_objnum, int allowed_path_mask, vec3d *pos, vec3d *fvec)
{
	int			path_index, bay_path;
	pnode		*pnp;
	vec3d		*next_point;

	ship *shipp = &Ships[pl_objp->instance];
	ai_info *aip = &Ai_info[shipp->ai_index];

	if ( parent_objnum == -1 ) {
		Int3();
		return -1;
	}

	object *parent_objp = &Objects[parent_objnum];
	ship *parent_shipp = &Ships[parent_objp->instance];

	polymodel *pm = model_get( Ship_info[parent_shipp->ship_info_index].model_num );
	ship_bay *bay = pm->ship_bay;

	if ( bay == NULL ) {
		WarningEx(LOCATION, "Ship %s was set to arrive from fighter bay on object %s, but no fighter bay exists on that ships' model (%s).\n", shipp->ship_name, parent_shipp->ship_name, pm->filename);
		return -1;
	}

	if ( bay->num_paths <= 0 ) {
		WarningEx(LOCATION, "Ship %s was set to arrive from fighter bay on object %s, but no fighter bay paths exist on that ships' model (%s).\n", shipp->ship_name, parent_shipp->ship_name, pm->filename);
		return -1;
	}

	// try to find a bay path that is not taken
	// Goober5000 - choose from among allowed paths
	if (allowed_path_mask != 0)
	{
		int i, num_allowed_paths = 0, allowed_bay_paths[MAX_SHIP_BAY_PATHS];

		for (i = 0; i < bay->num_paths; i++)
		{
			if (allowed_path_mask & (1 << i))
			{
				allowed_bay_paths[num_allowed_paths] = i;
				num_allowed_paths++;
			}
		}

		// cycle through the allowed paths
		bay_path = allowed_bay_paths[Ai_last_arrive_path % num_allowed_paths];
	}
	// choose from among all paths
	else
	{
		bay_path = Ai_last_arrive_path % bay->num_paths;
	}

	Ai_last_arrive_path++;

	path_index = bay->path_indexes[bay_path];

	if ( path_index == -1 ) 
		return -1;

	// notify parent that I'll want to leave
	parent_shipp->bay_doors_wanting_open++;

	// keep track of my own status as well
	shipp->bay_doors_need_open = true;
	shipp->bay_doors_launched_from = (ubyte)bay_path;
	shipp->bay_doors_parent_shipnum = parent_objp->instance;

	// create the path for pl_objp to follow
	create_model_exit_path(pl_objp, parent_objp, path_index, pm->paths[path_index].nverts);

	// now return to the caller what the starting world pos and starting fvec for the ship will be
	Assert((aip->path_start >= 0) && (aip->path_start < MAX_PATH_POINTS));
	pnp = &Path_points[aip->path_start];
	*pos = pnp->pos;

	// calc the forward vector using the starting two points of the path
	pnp = &Path_points[aip->path_start+1];
	next_point = &pnp->pos;
	vm_vec_normalized_dir(fvec, next_point, pos);

	// record the parent objnum, since we'll need it once we're done with following the path
	aip->goal_objnum = parent_objnum;
	aip->goal_signature = parent_objp->signature;
	aip->mode = AIM_BAY_EMERGE;
	aip->submode_start_time = Missiontime;

	//due to the door animations the ship has to remain still until the doors are open
	vec3d vel = ZERO_VECTOR;
	pl_objp->phys_info.vel = vel;
	pl_objp->phys_info.desired_vel = vel;
	pl_objp->phys_info.prev_ramp_vel.xyz.x = 0.0f;
	pl_objp->phys_info.prev_ramp_vel.xyz.y = 0.0f;
	pl_objp->phys_info.prev_ramp_vel.xyz.z = 0.0f;
	pl_objp->phys_info.forward_thrust = 0.0f;		// How much the forward thruster is applied.  0-1.

	return 0;	
}

/**
 * Clean up path data used for emerging from a fighter bay
 */
void ai_emerge_bay_path_cleanup(ai_info *aip)
{
	aip->path_start = -1;
	aip->path_cur = -1;
	aip->path_length = 0;
	aip->mode = AIM_NONE;
}

/**
 * Handler for AIM_BAY_EMERGE
 */
void ai_bay_emerge()
{
	ai_info	*aip;
	ship *shipp;
	int		parent_died=0;

	Assert( Pl_objp != NULL );
	Assert( Pl_objp->instance >= 0 );

	shipp = &Ships[Pl_objp->instance];
	aip = &Ai_info[shipp->ai_index];

	// if no path to follow, leave this mode
	if ( aip->path_start < 0 ) {
		ai_manage_bay_doors(Pl_objp, aip, true);
		aip->mode = AIM_NONE;
		return;
	}

	// ensure parent ship is still alive
	if (aip->goal_objnum < 0) {
		parent_died = 1;
	}

	if ( !parent_died ) {
		if ( Objects[aip->goal_objnum].signature != aip->goal_signature ) {
			parent_died = 1;
		}
	}

	if ( !parent_died ) {
		Assert(Objects[aip->goal_objnum].type == OBJ_SHIP);
		if ( Ships[Objects[aip->goal_objnum].instance].flags & SF_DYING ) {
			parent_died = 1;
		}
	}

	if ( parent_died ) {
		ai_emerge_bay_path_cleanup(aip);
		return;
	}

	ai_manage_bay_doors(Pl_objp, aip, false);

	if ( Ships[Objects[aip->goal_objnum].instance].bay_doors_status != MA_POS_READY )
		return;

	// follow the path to the final point
	ai_path();

	// New test: must have been in AI_EMERGE mode for at least 10 seconds, and be a minimum distance from the start point
	if ( ( (Missiontime - aip->submode_start_time) > 10*F1_0 ) && (vm_vec_dist_quick(&Pl_objp->pos, &Objects[aip->goal_objnum].pos) > 0.75f * Objects[aip->goal_objnum].radius)) {
		// erase path
		ai_emerge_bay_path_cleanup(aip);
		// deal with model animations
		ai_manage_bay_doors(Pl_objp, aip, true);
	}

	// 2-25-99: Need this check to fix an assert for supercap ships... maybe we'll only do this check for supercaps	
	if (aip->path_cur > (aip->path_start+aip->path_length-1)) {
		ai_emerge_bay_path_cleanup(aip);
		// deal with model animations
		ai_manage_bay_doors(Pl_objp, aip, true);
	}	
}

// Select the closest depart path
//
//	input:	aip	=>		ai info pointer to ship seeking to depart
//				pm		=>		pointer to polymodel for the ship contining the ship bay/depart paths
//
// exit:		>=0	=>		ship bay path index for depart path (ie index into sb->paths[])
//				-1		=>		no path could be found
//
// NOTE: this function should only be used for calculating closest depart paths for ai mode
//			AI_BAY_DEPART.  It tries to find the closest path that isn't already in use
int ai_find_closest_depart_path(ai_info *aip, polymodel *pm, int allowed_path_mask)
{
	int			i, j, best_path, best_free_path;
	float		dist, min_dist, min_free_dist;
	vec3d		*source;
	model_path	*mp;
	ship_bay	*bay;

	bay = pm->ship_bay;

	best_free_path = best_path = -1;
	min_free_dist = min_dist = 1e20f;
	Assert(aip->shipnum >= 0);
	source = &Objects[Ships[aip->shipnum].objnum].pos;

	for ( i = 0; i < bay->num_paths; i++ )
	{
		// Goober5000 - maybe skip this path
		if ((allowed_path_mask != 0) && !(allowed_path_mask & (1 << i)))
			continue;

		mp = &pm->paths[bay->path_indexes[i]];
		for ( j = 0; j < mp->nverts; j++ )
		{
			dist = vm_vec_dist_squared(source, &mp->verts[j].pos);

			if (dist < min_dist)
			{
				min_dist = dist;
				best_path = i;
			}

			// If this is a free path
			if (!(bay->depart_flags & (1 << i)))
			{
				if (dist < min_free_dist)
				{
					min_free_dist = dist;
					best_free_path = i;
				}
			}
		}
	}

	if (best_free_path >= 0)
	{
		return best_free_path;		
	}

	return best_path;
}

// determine what path to use when trying to depart to a fighter bay
// NOTE: this should be called when AIM_BAY_DEPART mode is set
//
// input:	pl_objp	=>	pointer to object for ship that is departing
//
// exit:		-1	=>	could not find depart path
//				0	=> found depart path
int ai_acquire_depart_path(object *pl_objp, int parent_objnum, int allowed_path_mask)
{
	int path_index = -1;
	int ship_bay_path = -1;

	ship *shipp = &Ships[pl_objp->instance];
	ai_info *aip = &Ai_info[shipp->ai_index];

	if ( parent_objnum < 0 )
	{
		// try to locate a capital ship on the same team:
		int shipnum = ship_get_ship_with_dock_bay(shipp->team);

		if (shipnum >= 0)
			parent_objnum = Ships[shipnum].objnum;
	}

	aip->path_start = -1;

	if ( parent_objnum < 0 )
		return -1;

	object *parent_objp = &Objects[parent_objnum];
	polymodel *pm = model_get(Ship_info[Ships[parent_objp->instance].ship_info_index].model_num );
	ship_bay *bay = pm->ship_bay;

	if ( bay == NULL ) 
		return -1;
	if ( bay->num_paths <= 0 ) 
		return -1;

	// take the closest path we can find
	ship_bay_path = ai_find_closest_depart_path(aip, pm, allowed_path_mask);
	path_index = bay->path_indexes[ship_bay_path];
	aip->submode_parm0 = ship_bay_path;
	bay->depart_flags |= (1<<ship_bay_path);

	if ( path_index == -1 ) {
		return -1;
	}

	// notify parent that I'll want to enter bay
	Ships[parent_objp->instance].bay_doors_wanting_open++;

	// keep track of my own status as well
	shipp->bay_doors_need_open = true;
	shipp->bay_doors_launched_from = (ubyte)ship_bay_path;
	shipp->bay_doors_parent_shipnum = parent_objp->instance;

	Assert(pm->n_paths > path_index);
	ai_find_path(pl_objp, parent_objnum, path_index, 0);

	// Set this flag, so we don't bother recreating the path... we won't need to update the path
	// that has just been created.
	aip->ai_flags &= ~AIF_USE_STATIC_PATH;

	aip->goal_objnum = parent_objnum;
	aip->goal_signature = parent_objp->signature;
	aip->mode = AIM_BAY_DEPART;

	shipp->flags |= SF_DEPART_DOCKBAY;
	return 0;
}

/**
 * Handler for AIM_BAY_DEPART
 */
void ai_bay_depart()
{
	ai_info	*aip;
	int anchor_shipnum;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	// if no path to follow, leave this mode
	if ( aip->path_start < 0 ) {
		aip->mode = AIM_NONE;
		return;
	}

	// check if parent ship valid; if not, abort depart
	anchor_shipnum = ship_name_lookup(Parse_names[Ships[Pl_objp->instance].departure_anchor]);
	if (anchor_shipnum >= 0)
	{
		// make sure not dying or departing
		if ( Ships[anchor_shipnum].flags & (SF_DYING | SF_DEPARTING))
		{
			anchor_shipnum = -1;
		}
		// make sure fighterbays not destroyed
		else if ( ship_fighterbays_all_destroyed(&Ships[anchor_shipnum]) )
		{
			anchor_shipnum = -1;
		}
	}

	if (anchor_shipnum < 0)
	{
		aip->mode = AIM_NONE;
		
		Ships[Pl_objp->instance].flags &= ~SF_DEPART_DOCKBAY;
		return;
	}

	ai_manage_bay_doors(Pl_objp, aip, false);

	if ( Ships[anchor_shipnum].bay_doors_status != MA_POS_READY )
		return;

	// follow the path to the final point
	ai_path();

	//if we are on the last segment of the path and the bay doors are not open, open them
	//if we are on any other segment of the path, close them

	// if the final point is reached, let default AI take over
	if ( aip->path_cur >= (aip->path_start+aip->path_length) ) {
		ai_manage_bay_doors(Pl_objp, aip, true);

		// Volition bay code
		polymodel	*pm;
		ship_bay	*bay;

		pm = model_get(Ship_info[Ships[Objects[aip->goal_objnum].instance].ship_info_index].model_num);
		bay = pm->ship_bay;
		if ( bay != NULL ) {
			bay->depart_flags &= ~(1<<aip->submode_parm0);
		}

		// make ship disappear
		ship_actually_depart(Pl_objp->instance, SHIP_DEPARTED_BAY);

		// clean up path stuff
		aip->path_start = -1;
		aip->path_cur = -1;
		aip->path_length = 0;
		aip->mode = AIM_NONE;
	}
}

/**
 * Handler for AIM_SENTRYGUN.  This AI mode is for sentry guns only (ie floating turrets).
 */
void ai_sentrygun()
{
	// Nothing to do here.  Turret firing is handled via process_subobjects().
	// If you want the sentry guns to do anything beyond firing their turrets at enemies, add it here!
}

/**
 * Execute behavior given by aip->mode.
 * @todo Complete the AIM_GET_BEHIND option
 */
void ai_execute_behavior(ai_info *aip)
{
	switch (aip->mode) {
	case AIM_CHASE:
		if (En_objp) {
			ai_chase();
		} else if (aip->submode == SM_EVADE_WEAPON) {
			evade_weapon();
			// maybe reset submode
			if (aip->danger_weapon_objnum == -1) {
				aip->submode = SM_ATTACK;
				aip->submode_start_time = Missiontime;
				aip->last_attack_time = Missiontime;
			}
		} else {
			//	Don't circle if this is the instructor.
			ship	*shipp = &Ships[aip->shipnum];
			ship_info	*sip = &Ship_info[shipp->ship_info_index];

			if (strnicmp(shipp->ship_name, INSTRUCTOR_SHIP_NAME, strlen(INSTRUCTOR_SHIP_NAME))) {
				if (sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {
					aip->mode = AIM_NONE;
				} else {
					ai_chase_circle(Pl_objp);
				}
			}
		}
		break;
	case AIM_EVADE:
		if (En_objp) {
			ai_evade();
		} else {
			vec3d	tvec;
			vm_vec_scale_add(&tvec, &Pl_objp->pos, &Pl_objp->orient.vec.rvec, 100.0f);
			turn_towards_point(Pl_objp, &tvec, NULL, 0.0f);
			accelerate_ship(aip, 0.5f);
		}
		break;
	case AIM_STILL:
		ai_still();
		break;
	case AIM_STAY_NEAR:
		ai_stay_near();
		break;
	case AIM_GUARD:
		ai_guard();
		break;
	case AIM_WAYPOINTS:
		ai_waypoints();
		break;
	case AIM_FLY_TO_SHIP:
		ai_fly_to_ship();
		break;
	case AIM_DOCK:
		ai_dock();
		break;
	case AIM_NONE:
		break;
	case AIM_BIGSHIP:
		ai_big_ship(Pl_objp);
		break;
	case AIM_PATH: {
		int path_num;
		path_num = ai_return_path_num_from_dockbay(&Objects[aip->goal_objnum], 0);
		ai_find_path(Pl_objp, aip->goal_objnum, path_num, 0);
		ai_path();
		break;
	}
	case AIM_SAFETY:
		ai_safety();
		break;
	case AIM_EVADE_WEAPON:
		evade_weapon();
		break;
	case AIM_STRAFE:
		if (En_objp) {
			Assert(En_objp->type == OBJ_SHIP);
			ai_big_strafe();	// strafe a big ship
		} else {
			aip->mode = AIM_NONE;
		}
		break;
	case AIM_BAY_EMERGE:
		ai_bay_emerge();
		break;
	case AIM_BAY_DEPART:
		ai_bay_depart();
		break;
	case AIM_SENTRYGUN:
		ai_sentrygun();
		break;
	case AIM_WARP_OUT:
		break;		//	Note, handled directly from ai_frame().
	case AIM_GET_BEHIND:
		// FIXME: got this from TBP and added it here to skip the Int3() but don't really want to handle it
		// properly until after 3.6.7 just to avoid delaying release or breaking something - taylor
		break;
	default:
		Int3();		//	This should never happen -- MK, 5/12/97	
		break;
	}

	if ( !(ship_get_SIF(aip->shipnum) & SIF_NOT_FLYABLE) ) {
		maybe_evade_dumbfire_weapon(aip);
	}
}

//	Auxiliary function for maybe_request_support.
//	Return 1 if subsystem "type" is worthy of repair, else return 0.
//	Since subsystems cannot be repaired if they are at 0 strength, don't return 1 if subsystem is dead.
int mrs_subsystem(ship *shipp, int type)
{
	float	t;

	t = ship_get_subsystem_strength(shipp, type);

	if (t > 0.0f) {
		return (int) ((1.0f - t) * 3);
	} else {
		return 3;
	}
}

/**
 * Return number of ships on *objp's team that are currently rearming.
 */
int num_allies_rearming(object *objp)
{
	ship_obj	*so;
	int		team;
	int		count = 0;

	team = Ships[objp->instance].team;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		object	*A;
		
		Assert (so->objnum != -1);
		A = &Objects[so->objnum];

		if (Ships[A->instance].team == team) {
			if (Ai_info[Ships[A->instance].ai_index].ai_flags & (AIF_REPAIRING | AIF_AWAITING_REPAIR)) {
				count++;
			}
		}
	}

	return count;
}


//	Maybe ship *objp should request support (rearm/repair).
//	If it does, return TRUE, else return FALSE.
int maybe_request_support(object *objp)
{
	ship_info	*sip;
	ship			*shipp;
	ai_info		*aip;
	weapon_info *wip;
	int			desire;
	int i;
	float r;

	Assert(objp->type == OBJ_SHIP);
	shipp = &Ships[objp->instance];
	aip = &Ai_info[shipp->ai_index];
	sip = &Ship_info[shipp->ship_info_index];

	if (!timestamp_elapsed(aip->next_rearm_request_timestamp))
		return 0;

	//	Only fighters and bombers request support.
	if (!(sip->flags & (SIF_FIGHTER | SIF_BOMBER)))
		return 0;

	//	A ship that is currently awaiting does not need support!
	if (aip->ai_flags & (AIF_AWAITING_REPAIR | AIF_BEING_REPAIRED))
		return 0;

	if (!is_support_allowed(objp))
		return 0;

	//	Compute a desire value.
	//	Desire of 0 means no reason to request support.
	//	1 is slight, 2 more, etc.  Maximum is around 20.  Anything larger than 3 is pretty strong.
	desire = 0;

	//	Set desire based on hull strength.
	//	Note: We no longer repair hull, so this would cause repeated repair requests.
	// Added back in upon mission flag condition - Goober5000
	if (The_mission.flags & MISSION_FLAG_SUPPORT_REPAIRS_HULL)
	{
		desire += 6 - (int) (get_hull_pct(objp) * 6.0f);
	}

	//	Set desire based on key subsystems.
	desire += 2*mrs_subsystem(shipp, SUBSYSTEM_ENGINE);	//	Note, disabled engine forces repair request, regardless of nearby enemies.
	desire += mrs_subsystem(shipp, SUBSYSTEM_COMMUNICATION);
	desire += mrs_subsystem(shipp, SUBSYSTEM_WEAPONS);
	desire += mrs_subsystem(shipp, SUBSYSTEM_SENSORS);


	//	Set desire based on percentage of secondary weapons.
	ship_weapon *swp = &shipp->weapons;

	for ( i = 0; i < swp->num_secondary_banks; i++ ) {
		if (swp->secondary_bank_start_ammo[i] > 0) {
			r = (float) swp->secondary_bank_ammo[i]/swp->secondary_bank_start_ammo[i];
			desire += (int) ((1.0f - r) * 3.0f);
		}
	}

	// Set desire based on ballistic weapons - Goober5000
	if (sip->flags & SIF_BALLISTIC_PRIMARIES)
	{
		for (i = 0; i < swp->num_primary_banks; i++)
		{
			wip = &Weapon_info[swp->primary_bank_weapons[i]];

			if (wip->wi_flags2 & WIF2_BALLISTIC)
			{
				r = (float) swp->primary_bank_ammo[i] / swp->primary_bank_start_ammo[i];

				// cube ammo level for better behavior, and adjust for number of banks
				desire += (int) ((1.0f - r)*(1.0f - r)*(1.0f - r) * (5.0f / swp->num_primary_banks));
			}
		}
	}


	//	If no reason to repair, don't bother to see if it's safe to repair.
	if (desire == 0){
		return 0;
	}

	//	Compute danger threshold.
	//	Balance this with desire and maybe request support.
	if (ai_good_time_to_rearm( objp )) {
		ai_issue_rearm_request(objp);
		return 1;
	} else if (num_allies_rearming(objp) < 2) {
		if (desire >= 8) {	//	guarantees disabled will cause repair request
			ai_issue_rearm_request(objp);
		} else if (desire >= 3) {		//	>= 3 means having a single subsystem fully blown will cause repair.
			int	count;
			int objnum = find_nearby_threat(OBJ_INDEX(objp), iff_get_attacker_mask(obj_team(objp)), 2000.0f, &count);

			if ((objnum == -1) || (count < 2) || (vm_vec_dist_quick(&objp->pos, &Objects[objnum].pos) > 3000.0f*count/desire)) {
				ai_issue_rearm_request(objp);
				return 1;
			}
		}
	}

	return 0;
}

void ai_set_mode_warp_out(object *objp, ai_info *aip)
{
	ai_abort_rearm_request(objp);
	if (aip->mode != AIM_WARP_OUT) {
		aip->mode = AIM_WARP_OUT;
		aip->submode = AIS_WARP_1;
		aip->submode_start_time = Missiontime;
	}
}

//	Maybe make ship depart (Goober5000 - changed from always warp ship out)
//	Shivan and HoL fighter/bomber warp out if their weapons subsystems have been destroyed.
void ai_maybe_depart(object *objp)
{
	ship	*shipp;
	ship_info *sip;

	// don't do anything if in a training mission.
	if ( The_mission.game_type & MISSION_TYPE_TRAINING )
		return;

	Assert(objp->type == OBJ_SHIP);

	shipp = &Ships[objp->instance];
	ai_info	*aip = &Ai_info[shipp->ai_index];
	sip = &Ship_info[shipp->ship_info_index];

	if (aip->mode == AIM_WARP_OUT || aip->mode == AIM_BAY_DEPART)
		return;

	//	If a support ship with no goals and low hull, depart.  Be sure that there are no pending goals
	// in the support ships ai_goal array.  Just process this ships goals.
	if (sip->flags & SIF_SUPPORT) {
		if ( timestamp_elapsed(aip->warp_out_timestamp) ) {
			ai_process_mission_orders( OBJ_INDEX(objp), aip );
			if ( (aip->support_ship_objnum == -1) && (get_hull_pct(objp) < 0.25f) ) {
				if (!(shipp->flags & SF_DEPARTING))
					mission_do_departure(objp);
			}
		}
	}

	// some iffs don't warp out, they'll eventually request support.
	if (Iff_info[shipp->team].flags & IFFF_SUPPORT_ALLOWED)
		return;

	if (!(shipp->flags & SF_DEPARTING)) {
		if (sip->flags & (SIF_FIGHTER | SIF_BOMBER)) {
			if (aip->warp_out_timestamp == 0) {
				//if (ship_get_subsystem_strength(shipp, SUBSYSTEM_WEAPONS) == 0.0f) {
				//	aip->warp_out_timestamp = timestamp(((myrand() % 10) + 10) * 1000);
				//}
			} else if (timestamp_elapsed(aip->warp_out_timestamp)) {
				mission_do_departure(objp);
			}
		}
	}
}

/**
 * Warp this ship out.
 */
void ai_warp_out(object *objp)
{
	ship *shipp = &Ships[objp->instance];
	ai_info	*aip = &Ai_info[shipp->ai_index];

	// if dying, don't warp out.
	if (shipp->flags & SF_DYING)
		return;

	// Goober5000 - check for engine or navigation failure
	if (!ship_engine_ok_to_warp(shipp) || !ship_navigation_ok_to_warp(shipp))
	{
		// you shouldn't hit this... if you do, then I need to add a check for it
		// in whatever function initiates a warpout
		Assert (!(shipp->flags2 & SF2_NO_SUBSPACE_DRIVE));

		// flag us as trying to warp so that this function keeps getting called
		// (in other words, if we can't warp just yet, we want to warp at the first
		// opportunity)
		if (aip->mode != AIM_WARP_OUT) 
			aip->mode = AIM_WARP_OUT;
		aip->submode = AIS_WARP_1;
		aip->ai_flags |= AIF_TRYING_UNSUCCESSFULLY_TO_WARP;

		return;
	}

	// Goober5000 - make sure the flag is clear (if it was previously set)
	aip->ai_flags &= ~AIF_TRYING_UNSUCCESSFULLY_TO_WARP;

	switch (aip->submode) {
	case AIS_WARP_1:
		aip->force_warp_time = timestamp(10*1000);	//	Try to avoid a collision for up to ten seconds.
		aip->submode = AIS_WARP_2;
		aip->submode_start_time = Missiontime;
		break;
	case AIS_WARP_2:			//	Make sure won't collide with any object.
		if (timestamp_elapsed(aip->force_warp_time)
			|| (!collide_predict_large_ship(objp, objp->radius*2.0f + 100.0f)
			|| (Ship_info[shipp->ship_info_index].warpout_type == WT_HYPERSPACE
				&& !collide_predict_large_ship(objp, 100000.0f)))) {
			aip->submode = AIS_WARP_3;
			aip->submode_start_time = Missiontime;

			// maybe recalculate collision pairs.
			if (ship_get_warpout_speed(objp) > ship_get_max_speed(shipp)) {
				// recalculate collision pairs
				OBJ_RECALC_PAIRS(objp);	
			}

			aip->force_warp_time = timestamp(4*1000);		//	Try to attain target speed for up to 4 seconds.
		} else {
			vec3d	goal_point;
			vm_vec_scale_add(&goal_point, &objp->pos, &objp->orient.vec.uvec, 100.0f);
			turn_towards_point(objp, &goal_point, NULL, 0.0f);
			accelerate_ship(aip, 0.0f);
		}
		break;
	case AIS_WARP_3:
		//	Rampup desired_vel in here from current to desired velocity and set PF_USE_VEL. (not sure this is the right flag)
		//	See shipfx#572 for sample code.
		float	speed, goal_speed;
		goal_speed = ship_get_warpout_speed(objp);

		// HUGE ships go immediately to AIS_WARP_4
		if (Ship_info[shipp->ship_info_index].flags & SIF_HUGE_SHIP) {
			aip->submode = AIS_WARP_4;
			aip->submode_start_time = Missiontime;
			break;
		}

		speed = goal_speed * flFrametime + objp->phys_info.speed * (1.0f - flFrametime);
		vm_vec_copy_scale(&objp->phys_info.vel, &objp->orient.vec.fvec, speed);
		objp->phys_info.desired_vel = objp->phys_info.vel;
		
		if (timestamp_elapsed(aip->force_warp_time) || (fl_abs(objp->phys_info.speed - goal_speed) < 2.0f)) {
			aip->submode = AIS_WARP_4;
			aip->submode_start_time = Missiontime;
		}
		break;
	case AIS_WARP_4: {
		// Only lets the ship warp after waiting for the warpout engage time
		if ( (Missiontime / 100) >= (aip->submode_start_time / 100 + Ship_info[shipp->ship_info_index].warpout_engage_time) ) {
			shipfx_warpout_start(objp);
			aip->submode = AIS_WARP_5;
			aip->submode_start_time = Missiontime;
		}
		break;
	}
	case AIS_WARP_5:
		break;
	default:
		Int3();		//	Illegal submode for warping out.
	}
}

//	Return object index of weapon that could produce a shockwave that should be known about to *objp.
//	Return nearest one.
int ai_find_shockwave_weapon(object *objp, ai_info *aip)
{
	missile_obj	*mo;
	float	nearest_dist = 999999.9f;
	int	nearest_index = -1;

	for ( mo = GET_NEXT(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
		object		*A;
		weapon		*wp;
		weapon_info	*wip;
	
		Assert(mo->objnum >= 0 && mo->objnum < MAX_OBJECTS);
		A = &Objects[mo->objnum];

		Assert(A->type == OBJ_WEAPON);
		Assert((A->instance >= 0) && (A->instance < MAX_WEAPONS));
		wp = &Weapons[A->instance];
		wip = &Weapon_info[wp->weapon_info_index];
		Assert( wip->subtype == WP_MISSILE );

		if (wip->shockwave.speed > 0.0f) {
			float	dist;

			dist = vm_vec_dist_quick(&objp->pos, &A->pos);
			if (dist < nearest_dist) {
				nearest_dist = dist;
				nearest_index = mo->objnum;
			}
		}
	}

	return nearest_index;

}

#define	EVADE_SHOCKWAVE_DAMAGE_THRESHOLD		100.0f

//	Tell all ships to avoid a big ship that is blowing up.
//	Only avoid if shockwave is fairly large.
//	OK to tell everyone to avoid.  If they're too far away, that gets cleaned up in the frame interval.
void ai_announce_ship_dying(object *dying_objp)
{
	float damage = ship_get_exp_damage(dying_objp);
	if (damage >= EVADE_SHOCKWAVE_DAMAGE_THRESHOLD) {
		ship_obj	*so;

		for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
			object	*A = &Objects[so->objnum];
			Assert(A->type == OBJ_SHIP);

			if (Ship_info[Ships[A->instance].ship_info_index].flags & (SIF_SMALL_SHIP | SIF_FREIGHTER | SIF_TRANSPORT)) {
				ai_info	*aip = &Ai_info[Ships[A->instance].ai_index];

				// AL 1-5-98: only avoid shockwave if not docked or repairing
				if ( !object_is_docked(A) && !(aip->ai_flags & (AIF_REPAIRING|AIF_BEING_REPAIRED)) ) {
					aip->ai_flags |= AIF_AVOID_SHOCKWAVE_SHIP;
				}
			}
		}
	}
}


//	Return object index of weapon that could produce a shockwave that should be known about to *objp.
//	Return nearest one.
int ai_find_shockwave_ship(object *objp, ai_info *aip)
{
	ship_obj	*so;
	float	nearest_dist = 999999.9f;
	int	nearest_index = -1;

	for ( so = GET_NEXT(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		object		*A;
		ship			*shipp;
	
		Assert(so->objnum >= 0 && so->objnum < MAX_OBJECTS);
		A = &Objects[so->objnum];

		Assert(A->type == OBJ_SHIP);
		Assert((A->instance >= 0) && (A->instance < MAX_SHIPS));
		shipp = &Ships[A->instance];
		//	Only look at objects in the process of dying.
		if (shipp->flags & SF_DYING) {
			float damage = ship_get_exp_damage(objp);

			if (damage >= EVADE_SHOCKWAVE_DAMAGE_THRESHOLD) {		//	Only evade quite large blasts
				float	dist;

				dist = vm_vec_dist_quick(&objp->pos, &A->pos);
				if (dist < nearest_dist) {
					nearest_dist = dist;
					nearest_index = so->objnum;
				}
			}
		}
	}

	return nearest_index;

}

int aas_1(object *objp, ai_info *aip, vec3d *safe_pos)
{
	// MAKE SURE safe_pos DOES NOT TAKE US TOWARD THE A SHIP WE'RE ATTACKING.
	if (aip->ai_flags & AIF_AVOID_SHOCKWAVE_WEAPON) {
		//	If we don't currently know of a weapon to avoid, try to find one.
		//	If we can't find one, then clear the bit so we don't keep coming here.
		if (aip->shockwave_object == -1) {
			int shockwave_weapon = ai_find_shockwave_weapon(objp, aip);
			if (shockwave_weapon == -1) {
				aip->ai_flags &= ~AIF_AVOID_SHOCKWAVE_WEAPON;
				return 0;
			} else {
				aip->shockwave_object = shockwave_weapon;
			}
		}

		//	OK, we have reason to believe we should avoid aip->shockwave_object.
		Assert(aip->shockwave_object > -1);
		object	*weapon_objp = &Objects[aip->shockwave_object];
		if (weapon_objp->type != OBJ_WEAPON) {
			aip->ai_flags &= ~AIF_AVOID_SHOCKWAVE_WEAPON;
			aip->shockwave_object = -1;
			return 0;
		}

		weapon	*weaponp = &Weapons[weapon_objp->instance];
		weapon_info	*wip = &Weapon_info[weaponp->weapon_info_index];
		object *target_ship_obj = NULL;

		if (wip->shockwave.speed == 0.0f) {
			aip->ai_flags &= ~AIF_AVOID_SHOCKWAVE_WEAPON;
			aip->shockwave_object = -1;
			return 0;
		}

		float	danger_dist;
		vec3d	expected_pos;		//	Position at which we expect the weapon to detonate.
		int		pos_set = 0;

		danger_dist = wip->shockwave.outer_rad;
		//	Set predicted position of detonation.
		//	If an aspect locked missile, assume it will detonate at the homing position.
		//	If not, which is not possible in a default FreeSpace weapon, then predict it will detonate at some
		//	time in the future, this time based on max lifetime and life left.
		if (wip->wi_flags & WIF_LOCKED_HOMING) {
			expected_pos = weaponp->homing_pos;
			if (weaponp->homing_object && weaponp->homing_object->type == OBJ_SHIP) {
				target_ship_obj = weaponp->homing_object;
			}
			pos_set = 1;
			if (IS_VEC_NULL(&weaponp->homing_pos)) {
				pos_set = 0;
				if (weaponp->target_num != -1) {
					if (Objects[weaponp->target_num].type == OBJ_SHIP) {
						target_ship_obj = &Objects[weaponp->target_num];
						expected_pos = target_ship_obj->pos;
						pos_set = 1;
					}
				}
			}
		}

		if (!pos_set) {
			float	time_scale;

			if (wip->lifetime - weaponp->lifeleft > 5.0f) {
				time_scale = 1.0f;
			} else {
				time_scale = weaponp->lifeleft/2.0f;
			}

			vm_vec_scale_add(&expected_pos, &weapon_objp->pos, &weapon_objp->orient.vec.fvec, time_scale);
		}

		//	See if too far away to care about shockwave.
		if (vm_vec_dist_quick(&objp->pos, &expected_pos) > danger_dist*2.0f) {
			//aip->ai_flags &= ~AIF_AVOID_SHOCKWAVE_WEAPON;
			return 0;
		} else {
			// try to find a safe position
			vec3d vec_from_exp;
			float dir = 1.0f;
			vm_vec_sub(&vec_from_exp, &objp->pos, &expected_pos);
			float dot = vm_vec_dotprod(&vec_from_exp, &weapon_objp->orient.vec.fvec);
			if (dot > -30) {
				// if we're already on the other side of the explosion, don't try to fly behind it
				dir = -1.0f;
			}

			//	Fly towards a point behind the weapon.
			vm_vec_scale_add(safe_pos, &weapon_objp->pos, &weapon_objp->orient.vec.fvec, -50000.0f*dir);

			return 1;
		}
	} else if (aip->ai_flags & AIF_AVOID_SHOCKWAVE_SHIP) {
		if (aip->shockwave_object == -1) {
			int shockwave_ship = ai_find_shockwave_ship(objp, aip);
			if (shockwave_ship == -1) {
				aip->ai_flags &= ~AIF_AVOID_SHOCKWAVE_SHIP;
				return 0;
			} else {
				aip->shockwave_object = shockwave_ship;
			}
		}

		Assert(aip->shockwave_object > -1);
		object	*ship_objp = &Objects[aip->shockwave_object];
		if (ship_objp == objp) {
			aip->shockwave_object = -1;
			return 0;
		}

		if (ship_objp->type != OBJ_SHIP) {
			aip->ai_flags &= ~AIF_AVOID_SHOCKWAVE_SHIP;
			return 0;
		}

		//	Optimize note! Don't really have to normalize.  We only need a point away from the blowing-up ship.
		vec3d safe_vec;

		vm_vec_normalized_dir(&safe_vec, &objp->pos, &ship_objp->pos);
		vm_vec_scale_add(safe_pos, &ship_objp->pos, &safe_vec, 50000.0f);	//	Fly away from the ship.

		float outer_rad = ship_get_exp_outer_rad(ship_objp);

		if (vm_vec_dist_quick(&objp->pos, &ship_objp->pos) > outer_rad*1.5f) {
			aip->ai_flags &= ~AIF_AVOID_SHOCKWAVE_WEAPON;
			return 0;
		}

		return 1;

	} else {
		Int3();	//	Illegal -- supposedly avoiding a shockwave, but neither ship nor weapon.  What is it!?
	}

	return 0;
}

//	--------------------------------------------------------------------------
//	Make object *objp avoid the nearest dangerous shockwave-producing weapon.
//	If it looks like there is no valid shockwave-producing weapon then clear the AIF_AVOID_SHOCKWAVE_WEAPON bit in ai_flags and return.
//	Return 1 if avoiding a shockwave, else return 0.
int ai_avoid_shockwave(object *objp, ai_info *aip)
{
	vec3d	safe_pos;

	// BIG|HUGE do not respond to shockwaves
	if (Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_BIG_SHIP|SIF_HUGE_SHIP)) {
		// don't come here again
		aip->ai_flags &= ~AIF_AVOID_SHOCKWAVE;
		return 0;
	}

	//	Don't all react right away.
	if (!(aip->ai_flags & AIF_AVOID_SHOCKWAVE_STARTED)) {
		float evadeChance = (aip->ai_shockwave_evade_chance == FLT_MIN) 
			? ((float) aip->ai_class/4.0f + 0.25f)
			: aip->ai_shockwave_evade_chance;
		if (!rand_chance(flFrametime, evadeChance))	//	Chance to avoid in 1 second is 0.25 + ai_class/4
			return 0;
	}

	if (!aas_1(objp, aip, &safe_pos)) {
		aip->ai_flags |= AIF_AVOID_SHOCKWAVE_STARTED;
		return 0;
	}

	aip->ai_flags |= AIF_AVOID_SHOCKWAVE_STARTED;

	//	OK, evade the shockwave!
	turn_towards_point(objp, &safe_pos, NULL, 0.0f);
	vec3d	vec_to_safe_pos;
	float		dot_to_goal;

	vm_vec_normalized_dir(&vec_to_safe_pos, &safe_pos, &objp->pos);

	dot_to_goal = vm_vec_dot(&objp->orient.vec.fvec, &vec_to_safe_pos);
	if (dot_to_goal < -0.5f)
		accelerate_ship(aip, 0.3f);
	else {
		accelerate_ship(aip, 1.0f + dot_to_goal);
		if (dot_to_goal > 0.2f) {
			if (!(objp->phys_info.flags & PF_AFTERBURNER_ON )) {
				afterburners_start(objp);
				aip->afterburner_stop_time = Missiontime + 2*F1_0;
			}
		}
	}

	return 1;
}

//	Awaiting repair.  Be useful.
//	Probably fly towards incoming repair ship.
//	Return true if this ship is close to being repaired, else return false.
int ai_await_repair_frame(object *objp, ai_info *aip)
{
	if (!(aip->ai_flags & (AIF_AWAITING_REPAIR | AIF_BEING_REPAIRED)))
		return 0;

	if (aip->support_ship_objnum == -1)
		return 0;

	ship	*shipp;
	ship_info	*sip;

	shipp = &Ships[Objects[aip->support_ship_objnum].instance];
	sip = &Ship_info[shipp->ship_info_index];

	aip->ai_flags &= ~AIF_FORMATION_OBJECT;	//	Prevents endless rotation.

	if (!(sip->flags & SIF_SUPPORT))
		return 0;

	vec3d	goal_point;
	object	*repair_objp;

	repair_objp = &Objects[aip->support_ship_objnum];

	if (Ships[repair_objp->instance].team == Iff_traitor) {
		ai_abort_rearm_request(repair_objp);
		return 0;
	}

	vm_vec_scale_add(&goal_point, &repair_objp->pos, &repair_objp->orient.vec.uvec, -50.0f);	//	Fly towards point below repair ship.

	vec3d	vtr;
	float dist = vm_vec_normalized_dir(&vtr, &goal_point, &objp->pos);
	float dot = vm_vec_dot(&vtr, &objp->orient.vec.fvec);

	if (dist > 200.0f) {
		accelerate_ship(aip, (0.9f + dot) * dist/1500.0f);
		turn_towards_point(objp, &goal_point, NULL, 0.0f);
	} else {
		accelerate_ship(aip, 0.0f);
	}

	return 1;
}

//	Maybe cause this ship to self-destruct.
//	Currently, any small ship (SIF_SMALL_SHIP) that has been disabled will self-destruct after awhile.
//	Maybe should only do this if they are preventing their wing from re-entering.
void ai_maybe_self_destruct(object *objp, ai_info *aip)
{
	//	Some IFFs can be repaired, so no self-destruct.
	//	In multiplayer, just don't self-destruct.  I figured there would be a problem. -- MK, 3/19/98.
	if ((Iff_info[Ships[objp->instance].team].flags & IFFF_SUPPORT_ALLOWED) || (Game_mode & GM_MULTIPLAYER))
		return;

	//	Small ships in a wing blow themselves up after awhile if engine or weapons system has been destroyed.
	//	Reason: Don't want them to prevent a re-emergence of the wing.
	//	Note: Don't blow up if not in a wing for two reasons: One, won't affect re-emergence of waves and (1) disable the Dragon
	//	mission would be broken.
	if ((Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_SMALL_SHIP) && (Ships[objp->instance].wingnum != -1)) {
		if ((ship_get_subsystem_strength(&Ships[objp->instance], SUBSYSTEM_ENGINE) <= 0.0f) ||
			(ship_get_subsystem_strength(&Ships[objp->instance], SUBSYSTEM_WEAPONS) <= 0.0f)) {
			if (aip->self_destruct_timestamp < 0)
				aip->self_destruct_timestamp = timestamp(90 * 1000);	//	seconds until self-destruct
		} else {
			aip->self_destruct_timestamp = -1;
		}

		if (aip->self_destruct_timestamp < 0) {
			return;
		}

		if (timestamp_elapsed(aip->self_destruct_timestamp)) {
			ship_apply_local_damage( objp, objp, &objp->pos, objp->hull_strength*flFrametime + 1.0f, MISS_SHIELDS);
		}
	}
}

/**
 * Determine if pl_objp needs a new target, called from ai_frame()
 */
int ai_need_new_target(object *pl_objp, int target_objnum)
{
	object *objp;

	if ( target_objnum < 0 ) {
		return 1;
	}

	objp = &Objects[target_objnum];

	if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID) && (objp->type != OBJ_WEAPON) ) {
		return 1;
	}

	if ( objp->type == OBJ_SHIP ) {
		if ( Ships[objp->instance].flags & SF_DYING ) {
			return 1;
		} else if (Ships[objp->instance].team == Ships[pl_objp->instance].team)
			return 1;
	}

	return 0;
}

/**
 * If *objp is recovering from a collision with a big ship, handle it.
 * @return true if recovering.
 */
int maybe_big_ship_collide_recover_frame(object *objp, ai_info *aip)
{
	float	dot, dist;
	vec3d	v2g;
	
	if (aip->ai_flags & AIF_BIG_SHIP_COLLIDE_RECOVER_1) {
		ai_turn_towards_vector(&aip->big_recover_pos_1, objp, flFrametime, Ship_info[Ships[objp->instance].ship_info_index].srotation_time, NULL, NULL, 0.0f, 0, NULL);
		dist = vm_vec_normalized_dir(&v2g, &aip->big_recover_pos_1, &objp->pos);
		dot = vm_vec_dot(&objp->orient.vec.fvec, &v2g);
		accelerate_ship(aip, dot);

		//	If close to desired point, or 15+ seconds since entered this mode, continue to next mode.
		if ((timestamp_until(aip->big_recover_timestamp) < -15*1000) || (dist < (0.5f + flFrametime) * objp->phys_info.speed)) {
			aip->ai_flags &= ~AIF_BIG_SHIP_COLLIDE_RECOVER_1;
			aip->ai_flags |= AIF_BIG_SHIP_COLLIDE_RECOVER_2;
		}

		return 1;

	} else if (aip->ai_flags & AIF_BIG_SHIP_COLLIDE_RECOVER_2) {
		ai_turn_towards_vector(&aip->big_recover_pos_2, objp, flFrametime, Ship_info[Ships[objp->instance].ship_info_index].srotation_time, NULL, NULL, 0.0f, 0, NULL);
		dist = vm_vec_normalized_dir(&v2g, &aip->big_recover_pos_2, &objp->pos);
		dot = vm_vec_dot(&objp->orient.vec.fvec, &v2g);
		accelerate_ship(aip, dot);

		//	If close to desired point, or 30+ seconds since started avoiding collision, done avoiding.
		if ((timestamp_until(aip->big_recover_timestamp) < -30*1000) || (dist < (0.5f + flFrametime) * objp->phys_info.speed)) {
			aip->ai_flags &= ~AIF_BIG_SHIP_COLLIDE_RECOVER_2;
			aip->ai_flags &= ~AIF_TARGET_COLLISION;
		}

		return 1;
	}

	if (aip->ai_flags & AIF_TARGET_COLLISION) {
		aip->ai_flags &= ~AIF_TARGET_COLLISION;
	}
	return 0;
}

void validate_mode_submode(ai_info *aip)
{
	switch (aip->mode) {
	case AIM_CHASE:
		// check valid submode
		switch (aip->submode) {
		case SM_CONTINUOUS_TURN:
		case SM_ATTACK:
		case SM_EVADE_SQUIGGLE:
		case SM_EVADE_BRAKE:	
		case SM_EVADE:		
		case SM_SUPER_ATTACK:
		case SM_AVOID:	
		case SM_GET_BEHIND:
		case SM_GET_AWAY:		
		case SM_EVADE_WEAPON:
		case SM_FLY_AWAY:	
		case SM_ATTACK_FOREVER:
			break;
		default:
			Int3();
		}
		break;

	case AIM_STRAFE:
		// check valid submode
		switch(aip->submode) {
		case AIS_STRAFE_ATTACK:
		case AIS_STRAFE_AVOID:
		case AIS_STRAFE_RETREAT1:
		case AIS_STRAFE_RETREAT2:
		case AIS_STRAFE_POSITION:
			break;
		default:
			Int3();
		}
		break;
	}
}

/**
 * Process AI object "objnum".
 */
void ai_frame(int objnum)
{
	ship		*shipp = &Ships[Objects[objnum].instance];
	ai_info	*aip = &Ai_info[shipp->ai_index];
	int		target_objnum;

	Assert((aip->mode != AIM_WAYPOINTS) || (aip->active_goal != AI_ACTIVE_GOAL_DYNAMIC));

	// Set globals defining the current object and its enemy object.
	Pl_objp = &Objects[objnum];

	//Default to glide OFF
	Pl_objp->phys_info.flags &= ~PF_GLIDING;

	// warping out?
	if ((aip->mode == AIM_WARP_OUT) || (aip->ai_flags & AIF_TRYING_UNSUCCESSFULLY_TO_WARP))
	{
		ai_warp_out(Pl_objp);

		// Goober5000 - either we were never trying unsuccessfully, or we were but now
		// we're successful... in either case, since we're actually warping we simply return
		if (!(aip->ai_flags & AIF_TRYING_UNSUCCESSFULLY_TO_WARP))
			return;
	}

	ai_maybe_self_destruct(Pl_objp, aip);
	ai_process_mission_orders( objnum, aip );

	//	Avoid a shockwave, if necessary.  If a shockwave and rearming, stop rearming.
	if (aip->ai_flags & AIF_AVOID_SHOCKWAVE) {
		if (ai_avoid_shockwave(Pl_objp, aip)) {
			aip->ai_flags &= ~(AIF_BIG_SHIP_COLLIDE_RECOVER_1 | AIF_BIG_SHIP_COLLIDE_RECOVER_2);
			if (aip->ai_flags & (AIF_AWAITING_REPAIR | AIF_BEING_REPAIRED))
				ai_abort_rearm_request(Pl_objp);
			return;
		}
	} else {
		aip->ai_flags &= ~AIF_AVOID_SHOCKWAVE_STARTED;
	}

	// moved call to ai_do_repair frame here from below because of the subsequent if statment returning
	// if the ship is getting repaired
	//	If waiting to be repaired, just stop and sit.
	ai_do_repair_frame(Pl_objp, aip, flFrametime);
	if ((aip->ai_flags & AIF_AWAITING_REPAIR) || (aip->ai_flags & AIF_BEING_REPAIRED)) {
		if (ai_await_repair_frame(Pl_objp, aip))
			return;
	}

	if (aip->mode == AIM_PLAY_DEAD)
		return;

	//	If recovering from a collision with a big ship, don't continue.
	if (maybe_big_ship_collide_recover_frame(Pl_objp, aip))
		return;

	ai_preprocess_ignore_objnum(Pl_objp, aip);
	target_objnum = set_target_objnum(aip, aip->target_objnum);

	Assert(objnum != target_objnum);

	ai_manage_shield(Pl_objp, aip);
	
	if ( maybe_request_support(Pl_objp) ) {
		if ( Ships[Pl_objp->instance].flags & SF_FROM_PLAYER_WING ) {
			ship_maybe_tell_about_rearm(shipp);
		}
	}
	else {
		ship_maybe_tell_about_low_ammo(shipp);
	}

	ai_maybe_depart(Pl_objp);

	//	Find an enemy if don't already have one.
	En_objp = NULL;
	if ( ai_need_new_target(Pl_objp, target_objnum) ) {
		if ((aip->mode != AIM_EVADE_WEAPON) && (aip->active_goal == AI_ACTIVE_GOAL_DYNAMIC)) {
			aip->resume_goal_time = -1;
			aip->active_goal = AI_GOAL_NONE;
		} else if (aip->resume_goal_time == -1) {
			// AL 12-9-97: Don't allow cargo and navbuoys to set their aip->target_objnum
			if ( Ship_info[shipp->ship_info_index].class_type > -1 && (Ship_types[Ship_info[shipp->ship_info_index].class_type].ai_bools & STI_AI_AUTO_ATTACKS) ) {
				target_objnum = find_enemy(objnum, MAX_ENEMY_DISTANCE, The_mission.ai_profile->max_attackers[Game_skill_level]);		//	Attack up to 25K units away.
				if (target_objnum != -1) {
					if (aip->target_objnum != target_objnum)
						aip->aspect_locked_time = 0.0f;
					set_target_objnum(aip, target_objnum);
					En_objp = &Objects[target_objnum];
				}
			}
		}
	} else if (target_objnum >= 0) {
		En_objp = &Objects[target_objnum];
	}

	// set base stealth info each frame
	aip->ai_flags &= ~AIF_STEALTH_PURSUIT;
	if (En_objp && En_objp->type == OBJ_SHIP) {
		if (Ships[En_objp->instance].flags2 & SF2_STEALTH) {
			int stealth_state = ai_is_stealth_visible(Pl_objp, En_objp);
			float dist = vm_vec_dist_quick(&En_objp->pos, &Pl_objp->pos);

			if (stealth_state != STEALTH_FULLY_TARGETABLE) {
				aip->ai_flags |= AIF_STEALTH_PURSUIT;
			}

			if ( (stealth_state == STEALTH_FULLY_TARGETABLE) || (stealth_state == STEALTH_VISIBLE) ) {
				aip->stealth_last_visible_stamp = timestamp();
				aip->stealth_last_cheat_visible_stamp = aip->stealth_last_visible_stamp;
				aip->stealth_last_pos = En_objp->pos;
				aip->stealth_velocity = En_objp->phys_info.vel;
			} else if (dist < 100) {
				// get cheat timestamp
				aip->stealth_last_cheat_visible_stamp = timestamp();

				// set approximate pos and vel, with increasing error as time from last_visible_stamp increases
				update_ai_stealth_info_with_error(aip/*, 0*/);
			}
		}
	}

	// AL 12-10-97: ensure that cargo and navbuoys aip->target_objnum is always -1.
	if ( Ship_info[shipp->ship_info_index].class_type > -1 && !(Ship_types[Ship_info[shipp->ship_info_index].class_type].ai_bools & STI_AI_AUTO_ATTACKS)) {
		aip->target_objnum = -1;
	}

	if ((En_objp != NULL) && (En_objp->pos.xyz.x == Pl_objp->pos.xyz.x) && (En_objp->pos.xyz.y == Pl_objp->pos.xyz.y) && (En_objp->pos.xyz.z == Pl_objp->pos.xyz.z)) {
		mprintf(("Warning: Object and its enemy have same position.  Object #%i\n", Pl_objp-Objects));
		En_objp = NULL;
	}

	if (aip->mode == AIM_CHASE) {
		if (En_objp == NULL) {
			aip->active_goal = -1;
		}
	}

	//	If there is a goal to resume and enough time has elapsed, resume the goal.
	if ((aip->resume_goal_time > 0) && (aip->resume_goal_time < Missiontime)) {
		aip->active_goal = AI_GOAL_NONE;
		aip->resume_goal_time = -1;
		target_objnum = find_enemy(objnum, 2000.0f, The_mission.ai_profile->max_attackers[Game_skill_level]);
		if (target_objnum != -1) {
			if (aip->target_objnum != target_objnum) {
				aip->aspect_locked_time = 0.0f;
			}
			set_target_objnum(aip, target_objnum);
		}
	}

	// check if targeted subsystem has been destroyed, if so, move onto another subsystem
	// if trying to disable or disarm the target
	if ((En_objp != NULL) && ( aip->targeted_subsys != NULL )) {
		Assert(En_objp->type == OBJ_SHIP);
		if ( aip->targeted_subsys->current_hits <= 0.0f ) {
			int subsys_type;

			if ( aip->goals[0].ai_mode == AI_GOAL_DISABLE_SHIP ) {
				subsys_type = SUBSYSTEM_ENGINE;
			} else if ( aip->goals[0].ai_mode == AI_GOAL_DISARM_SHIP ) {
				subsys_type = SUBSYSTEM_TURRET;
			} else {
				subsys_type = -1;
			}

			if ( subsys_type != -1 ) {
				ship_subsys *new_subsys;
				new_subsys = ship_return_next_subsys(&Ships[En_objp->instance], subsys_type, &Pl_objp->pos);
				if ( new_subsys != NULL ) {
					set_targeted_subsys(aip, new_subsys, aip->target_objnum);
				} else {
					// AL 12-16-97: no more subsystems to attack... reset targeting info
					aip->target_objnum = -1;
					set_targeted_subsys(aip, NULL, -1);
				}
			} else {
				// targeted subsys is destroyed, so stop attacking it
				set_targeted_subsys(aip, NULL, -1);
			}
		}
	}

	ai_maybe_launch_cmeasure(Pl_objp, aip);
	ai_maybe_evade_locked_missile(Pl_objp, aip);

	aip->target_time += flFrametime;

	int in_formation = 0;
	if (aip->ai_flags & AIF_FORMATION) {
		in_formation = !ai_formation();
	}

	if ( !in_formation ) {
		ai_execute_behavior(aip);
	}

	process_subobjects(objnum);
	maybe_resume_previous_mode(Pl_objp, aip);
	
	if (Pl_objp->phys_info.flags & PF_AFTERBURNER_ON ) {
		if (Missiontime > aip->afterburner_stop_time) {
			afterburners_stop(Pl_objp);
		}
	}
}

void ai_control_info_check( object *obj, ai_info *aip )
{
	if(aip->ai_override_flags == 0)
		return;

	if(timestamp_elapsed(aip->ai_override_timestamp)) {
		aip->ai_override_flags = 0;
	} else {
		if(aip->ai_override_flags & AIORF_FULL)
		{
			AI_ci.pitch = aip->ai_override_ci.pitch;
			AI_ci.heading = aip->ai_override_ci.heading;
			AI_ci.bank = aip->ai_override_ci.bank;
		} else {
			if(aip->ai_override_flags & AIORF_PITCH)
			{
				AI_ci.pitch = aip->ai_override_ci.pitch;
			}
			if(aip->ai_override_flags & AIORF_HEADING)
			{
				AI_ci.heading = aip->ai_override_ci.heading;
			}
			if(aip->ai_override_flags & AIORF_ROLL)
			{
				AI_ci.bank = aip->ai_override_ci.bank;
			}
		}
		if(aip->ai_override_flags & AIORF_FULL_LAT)
		{
			AI_ci.vertical = aip->ai_override_ci.vertical;
			AI_ci.sideways = aip->ai_override_ci.sideways;
			AI_ci.forward = aip->ai_override_ci.forward;
		} else {
			if(aip->ai_override_flags & AIORF_UP)
			{
				AI_ci.vertical = aip->ai_override_ci.vertical;
			}
			if(aip->ai_override_flags & AIORF_SIDEWAYS)
			{
				AI_ci.sideways = aip->ai_override_ci.sideways;
			}
			if(aip->ai_override_flags & AIORF_FORWARD)
			{
				AI_ci.forward = aip->ai_override_ci.forward;
			}
		}
	}
}

int Last_ai_obj = -1;

void ai_process( object * obj, int ai_index, float frametime )
{
	if (obj->flags & OF_SHOULD_BE_DEAD)
		return;

	// return if ship is dead, unless it's a big ship...then its turrets still fire, like I was quoted in a magazine.  -- MK, 5/15/98.
	if ((Ships[obj->instance].flags & SF_DYING ) && !(Ship_info[Ships[obj->instance].ship_info_index].flags & (SIF_HUGE_SHIP | SIF_BIG_SHIP))){
		return;
	}

	int rfc = 1;		//	Assume will be Reading Flying Controls.

	Assert( obj->type == OBJ_SHIP );
	Assert( ai_index >= 0 );

	AI_frametime = frametime;
	if (obj-Objects <= Last_ai_obj) {
		AI_FrameCount++;
	}

	memset( &AI_ci, 0, sizeof(AI_ci) );

	ai_frame(OBJ_INDEX(obj));

	AI_ci.pitch = 0.0f;
	AI_ci.bank = 0.0f;
	AI_ci.heading = 0.0f;

	// the ships maximum velocity now depends on the energy flowing to engines
	obj->phys_info.max_vel.xyz.z = Ships[obj->instance].current_max_speed;
	ai_info	*aip = &Ai_info[Ships[obj->instance].ai_index];

	//	In certain circumstances, the AI says don't fly in the normal way.
	//	One circumstance is in docking and undocking, when the ship is moving
	//	under thruster control.
	switch (aip->mode) {
	case AIM_DOCK:
		if ((aip->submode >= AIS_DOCK_2) && (aip->submode != AIS_UNDOCK_3))
			rfc = 0;
		break;
	case AIM_WARP_OUT:
		if (aip->submode >= AIS_WARP_3)
			rfc = 0;
		break;
	default:
		break;
	}

	if (rfc == 1) {
		// Wanderer - sexp based override goes here - only if rfc is valid though
		ai_control_info_check(obj, aip);
		vec3d copy_desired_rotvel = obj->phys_info.rotvel;
		physics_read_flying_controls( &obj->orient, &obj->phys_info, &AI_ci, frametime);
		// if obj is in formation and not flight leader, don't update rotvel
		if (aip->ai_flags & AIF_FORMATION) {
			if (&Objects[aip->goal_objnum] != obj) {
				obj->phys_info.desired_rotvel = copy_desired_rotvel;
				obj->phys_info.rotvel = copy_desired_rotvel;
			}
		}
	}

	Last_ai_obj = obj-Objects;
}

/**
 * Initialize ::ai_info struct of object objnum.
 */
void init_ai_object(int objnum)
{
	int	i, ship_index, ai_index, ship_type;
	ai_info	*aip;
	object	*objp;
	vec3d	near_vec;			//	A vector nearby and mainly in front of this object.

	objp = &Objects[objnum];
	ship_index = objp->instance;
	ai_index = Ships[ship_index].ai_index;
	Assert((ai_index >= 0) && (ai_index < MAX_AI_INFO));

	aip = &Ai_info[ai_index];

	ship_type = Ships[ship_index].ship_info_index;

	vm_vec_scale_add(&near_vec, &objp->pos, &objp->orient.vec.fvec, 100.0f);
	vm_vec_scale_add2(&near_vec, &objp->orient.vec.rvec, 10.0f);

	// Things that shouldn't have to get initialized, but initialize them just in case!
	aip->ai_flags = 0;
	aip->previous_mode = AIM_NONE;
	aip->mode_time = -1;
	aip->target_objnum = -1;
	aip->target_signature = -1;
	aip->previous_target_objnum = -1;
	aip->target_time = 0.0f;
	aip->enemy_wing = -1;
	aip->attacker_objnum = -1;
	aip->goal_objnum = -1;
	aip->goal_signature = -1;
	aip->guard_objnum = -1;
	aip->guard_signature = -1;
	aip->guard_wingnum = -1;
	aip->submode = 0;
	aip->previous_submode = 0;
	aip->best_dot_to_enemy = -1.0f;
	aip->best_dot_from_enemy = -1.0f;
	aip->best_dot_to_time = 0;
	aip->best_dot_from_time = 0;
	aip->submode_start_time = 0;
	aip->submode_parm0 = 0;
	aip->active_goal = -1;
	aip->goal_check_time = timestamp(0);
	aip->last_predicted_enemy_pos = near_vec;
	aip->prev_goal_point = near_vec;
	aip->goal_point = near_vec;
	aip->time_enemy_in_range = 0.0f;
	aip->time_enemy_near = 0.0f;
	aip->last_attack_time = 0;
	aip->last_hit_time = 0;
	aip->last_hit_quadrant = 0;
	aip->hitter_objnum = -1;
	aip->hitter_signature = -1;
	aip->resume_goal_time = -1;
	aip->prev_accel = 0.0f;
	aip->prev_dot_to_goal = 0.0f;

	aip->ignore_objnum = UNUSED_OBJNUM;
	aip->ignore_signature = -1;

	// Goober5000
	for (i = 0; i < MAX_IGNORE_NEW_OBJECTS; i++)
	{
		aip->ignore_new_objnums[i] = UNUSED_OBJNUM;
		aip->ignore_new_signatures[i] = -1;
	}

	//Init stuff from AI class and AI profiles
	init_aip_from_class_and_profile(aip, &Ai_classes[Ship_info[ship_type].ai_class], The_mission.ai_profile);

	aip->wp_list = NULL;
	aip->wp_index = INVALID_WAYPOINT_POSITION;

	aip->attacker_objnum = -1;
	aip->goal_signature = -1;

	Objects[objnum].phys_info.prev_fvec = Objects[objnum].orient.vec.fvec;

	aip->last_predicted_enemy_pos.xyz.x = 0.0f;	//	Says this value needs to be recomputed!
	aip->time_enemy_in_range = 0.0f;
	aip->time_enemy_near = 0.0f;

	aip->resume_goal_time = -1;					//	Say there is no goal to resume.

	aip->active_goal = -1;
	aip->path_start = -1;
	aip->path_goal_dist = -1;
	aip->path_length = 0;
	aip->path_subsystem_next_check = 1;

	aip->support_ship_objnum = -1;
	aip->support_ship_signature = -1;

	aip->danger_weapon_objnum = -1;
	aip->danger_weapon_signature = -1;

	aip->lead_scale = 0.0f;
	aip->last_hit_target_time = Missiontime;
	aip->last_hit_time = Missiontime;

	aip->nearest_locked_object = -1;
	aip->nearest_locked_distance = 99999.0f;

	aip->targeted_subsys = NULL;
	aip->last_subsys_target = NULL;
	aip->targeted_subsys_parent = -1;

	// The next two fields are used to time the rearming to allow useful sound effects for missile rearming
	aip->rearm_first_missile = TRUE;		//	flag to indicate that next missile to load is the first missile
	aip->rearm_first_ballistic_primary = TRUE;	// flag to indicate that next ballistic to load is the first ballistic
	aip->rearm_release_delay = 0;			//	timestamp to delay the separation of docked ships after rearm

	aip->next_predict_pos_time = 0;
	aip->next_aim_pos_time = 0;

	aip->afterburner_stop_time = 0;
	aip->last_objsig_hit = -1;				// object signature of the ship most recently hit by aip

	aip->path_next_create_time = timestamp(1);
	aip->path_create_pos = Objects[objnum].pos;
	aip->path_create_orient = Objects[objnum].orient;
	vm_vec_zero(&aip->path_depart_orient);

	aip->ignore_expire_timestamp = timestamp(1);
	aip->warp_out_timestamp = 0;
	aip->next_rearm_request_timestamp = timestamp(1);
	aip->primary_select_timestamp = timestamp(1);
	aip->secondary_select_timestamp = timestamp(1);
	aip->scan_for_enemy_timestamp = timestamp(1);

	aip->choose_enemy_timestamp = timestamp(3*(NUM_SKILL_LEVELS-Game_skill_level) * ((static_rand_alt() % 500) + 500));

	aip->shockwave_object = -1;
	aip->shield_manage_timestamp = timestamp(1);
	aip->self_destruct_timestamp = -1;	//	This is a flag that we have not yet set this.
	aip->ok_to_target_timestamp = timestamp(1);
	aip->pick_big_attack_point_timestamp = timestamp(1);
	vm_vec_zero(&aip->big_attack_point);

	aip->avoid_check_timestamp = timestamp(1);

	aip->abort_rearm_timestamp = -1;

	// artillery stuff
	aip->artillery_objnum = -1;
	aip->artillery_sig = -1;	

	// waypoint speed cap
	aip->waypoint_speed_cap = -1;

	// set lethality to enemy team
	aip->lethality = 0.0f;
	aip->ai_override_flags = 0;
	memset(&aip->ai_override_ci,0,sizeof(control_info));
}

void init_ai_objects()
{
	int	i;

	for (i=0; i<Num_objects; i++){
		if (Objects[i].type == OBJ_SHIP){
			init_ai_object(i);
		}
	}
}

void init_ai_system()
{
	Ppfp = Path_points;
}

int combine_flags(int base_flags, int override_flags, int override_set)
{
	int result = 0;
	//Scan through every bit in the flag int
	for (int i = 0; i < 31; i++)
	{
		int flag = (1 << i);
		//If this flag is marked in the override as set, copy it from the override
		if (override_set & flag)
		{
			if (override_flags & flag)
				result |= flag;
		}
		else	//Otherwise, copy it from the base flag
		{
			if (base_flags & flag)
				result |= flag;
		}
	}
	return result;
}

//Sets the ai_info stuff based on what is in the ai class and the current ai profile
//Stuff in the ai class will override what is in the ai profile, but only if it is set.
//Unset per-difficulty-level values are marked with FLT_MIN or INT_MIN
//Which flags are set is handled by using two flag ints: one with the flag values (TRUE/FALSE), one that
//just says which flags are set.
void init_aip_from_class_and_profile(ai_info *aip, ai_class *aicp, ai_profile_t *profile)
{
	//ai_class-only stuff
	aip->ai_courage = aicp->ai_courage[Game_skill_level];
	aip->ai_patience = aicp->ai_patience[Game_skill_level];
	aip->ai_evasion = aicp->ai_evasion[Game_skill_level];
	aip->ai_accuracy = aicp->ai_accuracy[Game_skill_level];

	aip->ai_aburn_use_factor = aicp->ai_aburn_use_factor[Game_skill_level];		
	aip->ai_shockwave_evade_chance = aicp->ai_shockwave_evade_chance[Game_skill_level];	
	aip->ai_get_away_chance = aicp->ai_get_away_chance[Game_skill_level];	
	aip->ai_secondary_range_mult = aicp->ai_secondary_range_mult[Game_skill_level];
	aip->ai_class_autoscale = aicp->ai_class_autoscale;

	//Apply overrides from ai class to ai profiles values
	//Only override values which were explicitly set in the AI class
	aip->ai_cmeasure_fire_chance = (aicp->ai_cmeasure_fire_chance[Game_skill_level] == FLT_MIN) ? 
		profile->cmeasure_fire_chance[Game_skill_level] : aicp->ai_cmeasure_fire_chance[Game_skill_level];
	aip->ai_in_range_time = (aicp->ai_in_range_time[Game_skill_level] == FLT_MIN) ? 
		profile->in_range_time[Game_skill_level] : aicp->ai_in_range_time[Game_skill_level];
	aip->ai_link_ammo_levels_maybe = (aicp->ai_link_ammo_levels_maybe[Game_skill_level] == FLT_MIN) ? 
		profile->link_ammo_levels_maybe[Game_skill_level] : aicp->ai_link_ammo_levels_maybe[Game_skill_level];
	aip->ai_link_ammo_levels_always = (aicp->ai_link_ammo_levels_always[Game_skill_level] == FLT_MIN) ? 
		profile->link_ammo_levels_always[Game_skill_level] : aicp->ai_link_ammo_levels_always[Game_skill_level];
	aip->ai_primary_ammo_burst_mult = (aicp->ai_primary_ammo_burst_mult[Game_skill_level] == FLT_MIN) ? 
		profile->primary_ammo_burst_mult[Game_skill_level] : aicp->ai_primary_ammo_burst_mult[Game_skill_level];
	aip->ai_link_energy_levels_maybe = (aicp->ai_link_energy_levels_maybe[Game_skill_level] == FLT_MIN) ? 
		profile->link_energy_levels_maybe[Game_skill_level] : aicp->ai_link_energy_levels_maybe[Game_skill_level];
	aip->ai_link_energy_levels_always = (aicp->ai_link_energy_levels_always[Game_skill_level] == FLT_MIN) ? 
		profile->link_energy_levels_always[Game_skill_level] : aicp->ai_link_energy_levels_always[Game_skill_level];
	aip->ai_predict_position_delay = (aicp->ai_predict_position_delay[Game_skill_level] == INT_MIN) ? 
		profile->predict_position_delay[Game_skill_level] : aicp->ai_predict_position_delay[Game_skill_level];
	aip->ai_shield_manage_delay = (aicp->ai_shield_manage_delay[Game_skill_level] == FLT_MIN) ? 
		profile->shield_manage_delay[Game_skill_level] : aicp->ai_shield_manage_delay[Game_skill_level];
	aip->ai_ship_fire_delay_scale_friendly = (aicp->ai_ship_fire_delay_scale_friendly[Game_skill_level] == FLT_MIN) ? 
		profile->ship_fire_delay_scale_friendly[Game_skill_level] : aicp->ai_ship_fire_delay_scale_friendly[Game_skill_level];
	aip->ai_ship_fire_delay_scale_hostile = (aicp->ai_ship_fire_delay_scale_hostile[Game_skill_level] == FLT_MIN) ? 
		profile->ship_fire_delay_scale_hostile[Game_skill_level] : aicp->ai_ship_fire_delay_scale_hostile[Game_skill_level];
	aip->ai_ship_fire_secondary_delay_scale_friendly = (aicp->ai_ship_fire_secondary_delay_scale_friendly[Game_skill_level] == FLT_MIN) ? 
		profile->ship_fire_secondary_delay_scale_friendly[Game_skill_level] : aicp->ai_ship_fire_secondary_delay_scale_friendly[Game_skill_level];
	aip->ai_ship_fire_secondary_delay_scale_hostile = (aicp->ai_ship_fire_secondary_delay_scale_hostile[Game_skill_level] == FLT_MIN) ? 
		profile->ship_fire_secondary_delay_scale_hostile[Game_skill_level] : aicp->ai_ship_fire_secondary_delay_scale_hostile[Game_skill_level];
	aip->ai_turn_time_scale = (aicp->ai_turn_time_scale[Game_skill_level] == FLT_MIN) ? 
		profile->turn_time_scale[Game_skill_level] : aicp->ai_turn_time_scale[Game_skill_level];
	aip->ai_glide_attack_percent = (aicp->ai_glide_attack_percent[Game_skill_level] == FLT_MIN) ? 
		profile->glide_attack_percent[Game_skill_level] : aicp->ai_glide_attack_percent[Game_skill_level];
	aip->ai_circle_strafe_percent = (aicp->ai_circle_strafe_percent[Game_skill_level] == FLT_MIN) ? 
		profile->circle_strafe_percent[Game_skill_level] : aicp->ai_circle_strafe_percent[Game_skill_level];
	aip->ai_glide_strafe_percent = (aicp->ai_glide_strafe_percent[Game_skill_level] == FLT_MIN) ? 
		profile->glide_strafe_percent[Game_skill_level] : aicp->ai_glide_strafe_percent[Game_skill_level];
	aip->ai_random_sidethrust_percent = (aicp->ai_random_sidethrust_percent[Game_skill_level] == FLT_MIN) ? 
		profile->random_sidethrust_percent[Game_skill_level] : aicp->ai_random_sidethrust_percent[Game_skill_level];
	aip->ai_stalemate_time_thresh = (aicp->ai_stalemate_time_thresh[Game_skill_level] == FLT_MIN) ? 
		profile->stalemate_time_thresh[Game_skill_level] : aicp->ai_stalemate_time_thresh[Game_skill_level];
	aip->ai_stalemate_dist_thresh = (aicp->ai_stalemate_dist_thresh[Game_skill_level] == FLT_MIN) ? 
		profile->stalemate_dist_thresh[Game_skill_level] : aicp->ai_stalemate_dist_thresh[Game_skill_level];
	aip->ai_chance_to_use_missiles_on_plr = (aicp->ai_chance_to_use_missiles_on_plr[Game_skill_level] == INT_MIN) ? 
		profile->chance_to_use_missiles_on_plr[Game_skill_level] : aicp->ai_chance_to_use_missiles_on_plr[Game_skill_level];
	aip->ai_max_aim_update_delay = (aicp->ai_max_aim_update_delay[Game_skill_level] == FLT_MIN) ? 
		profile->max_aim_update_delay[Game_skill_level] : aicp->ai_max_aim_update_delay[Game_skill_level];
	aip->ai_turret_max_aim_update_delay = (aicp->ai_turret_max_aim_update_delay[Game_skill_level] == FLT_MIN) ? 
		profile->turret_max_aim_update_delay[Game_skill_level] : aicp->ai_turret_max_aim_update_delay[Game_skill_level];

	//Combine AI profile and AI class flags
	aip->ai_profile_flags = combine_flags(profile->flags, aicp->ai_profile_flags, aicp->ai_profile_flags_set);
	aip->ai_profile_flags2 = combine_flags(profile->flags2, aicp->ai_profile_flags2, aicp->ai_profile_flags2_set);
}

void ai_set_default_behavior(object *obj, int classnum)
{
	ai_info	*aip;

	Assert(obj != NULL);
	Assert(obj->instance != -1);
	Assert(Ships[obj->instance].ai_index != -1);

	aip = &Ai_info[Ships[obj->instance].ai_index];

	aip->behavior = AIM_NONE;

}

void ai_do_default_behavior(object *obj)
{
	ai_info	*aip;
	int		ship_flags;

	Assert(obj != NULL);
	Assert(obj->instance != -1);
	Assert(Ships[obj->instance].ai_index != -1);

	aip = &Ai_info[Ships[obj->instance].ai_index];

	ship_flags = Ship_info[Ships[obj->instance].ship_info_index].flags;
	if (!is_instructor(obj) && (ship_flags & (SIF_FIGHTER | SIF_BOMBER)))
	{
		int enemy_objnum = find_enemy(OBJ_INDEX(obj), 1000.0f, The_mission.ai_profile->max_attackers[Game_skill_level]);
		set_target_objnum(aip, enemy_objnum);
		aip->mode = AIM_CHASE;
		aip->submode = SM_ATTACK;
		aip->submode_start_time = Missiontime;
	}
	else if (ship_flags & (SIF_SUPPORT))
	{
		aip->mode = AIM_SAFETY;
		aip->submode = AISS_1;
		aip->submode_start_time = Missiontime;
		aip->ai_flags &= ~(AIF_REPAIRING);
	}
	else if ( ship_flags & SIF_SENTRYGUN )
	{
		aip->mode = AIM_SENTRYGUN;
	}
	else
	{
		aip->mode = AIM_NONE;
	}
	
	aip->submode_start_time = Missiontime;
	aip->active_goal = AI_GOAL_NONE;
}

#define	FRIENDLY_DAMAGE_THRESHOLD	50.0f		//	Display a message at this threshold.  Note, this gets scaled by Skill_level

// send the given message from objp.  called from the maybe_process_friendly_hit
// code below when a message must get send to the player when he fires on friendlies
void process_friendly_hit_message( int message, object *objp )
{
	int index;

	// no traitor in multiplayer
	if(Game_mode & GM_MULTIPLAYER){
		return;
	}

	// don't send this message if a player ship was hit.
	if ( objp->flags & OF_PLAYER_SHIP ){
		return;
	}

	// check if objp is a fighter/bomber -- if not, then find a new ship to send the message
	index = objp->instance;
	if ( !(Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_FIGHTER|SIF_BOMBER)) ){
		index = -1;
	}

	// If the ship can't send messages pick someone else
	if (Ships[objp->instance].flags2 & SF2_NO_BUILTIN_MESSAGES) {
		index = -1;
	}

	// Karajorma - pick a random ship to send Command messages if command is silenced. 
	if (index < 0 && (The_mission.flags & MISSION_FLAG_NO_BUILTIN_COMMAND) ) {
		index = ship_get_random_player_wing_ship( SHIP_GET_UNSILENCED );
	}

	if ( index >= 0 ) 
	{
		message_send_builtin_to_player( message, &Ships[index], MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_ANYTIME, 0, 0, -1, -1 );
	} else {
		message_send_builtin_to_player( message, NULL, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_ANYTIME, 0, 0, -1, -1 );
	}
}

extern	void ship_set_subsystem_strength( ship *shipp, int type, float strength );

/**
 * Object *objp_weapon, fired by *objp_hitter, hit object *objp_ship.
 */
void maybe_process_friendly_hit(object *objp_hitter, object *objp_hit, object *objp_weapon)
{
	// no turning traitor in multiplayer
	if ( Game_mode & GM_MULTIPLAYER ) {
		return;
	}

	// ditto if mission says no traitors allowed
	if (The_mission.flags & MISSION_FLAG_NO_TRAITOR) {
		return;
	}

	if ((obj_team(objp_hitter) == obj_team(objp_hit)) && (objp_hitter == Player_obj)) {

		// AL 12-4-97: It is possible the Player is a OBJ_GHOST at this point.  If so, bail out.
		if ( objp_hitter->type != OBJ_SHIP ) {
			return;
		}

		Assert(objp_hitter->type == OBJ_SHIP);
		Assert(objp_hit->type == OBJ_SHIP);
		Assert((objp_weapon->type == OBJ_WEAPON) || (objp_weapon->type == OBJ_BEAM));  //beam added for traitor detection - FUBAR

		ship	*shipp_hitter = &Ships[objp_hitter->instance];
		ship	*shipp_hit = &Ships[objp_hit->instance];

		if (shipp_hitter->team != shipp_hit->team) {
			return;
		}

		// get the player
		player *pp = &Players[Player_num];

		// wacky stuff here
		if (pp->friendly_hits != 0) {
			float	time_since_last_hit = f2fl(Missiontime - pp->friendly_last_hit_time);
			if ((time_since_last_hit >= 0.0f) && (time_since_last_hit < 10000.0f)) {
				if (time_since_last_hit > 60.0f) {
					pp->friendly_hits = 0;
					pp->friendly_damage = 0.0f;
				} else if (time_since_last_hit > 2.0f) {
					pp->friendly_hits -= (int) time_since_last_hit/2;
					pp->friendly_damage -= time_since_last_hit;
				}

				if (pp->friendly_damage < 0.0f) {
					pp->friendly_damage = 0.0f;
				}

				if (pp->friendly_hits < 0) {
					pp->friendly_hits = 0;
				}
			}
		}

		float	damage;		//	Damage done by weapon.  Gets scaled down based on size of ship.

		if (objp_weapon->type == OBJ_BEAM)  // added beam for traitor detection -FUBAR
			damage = beam_get_ship_damage(&Beams[objp_weapon->instance], objp_hit);
		else  
			damage = Weapon_info[Weapons[objp_weapon->instance].weapon_info_index].damage;
		
		// wacky stuff here
		ship_info *sip = &Ship_info[Ships[objp_hit->instance].ship_info_index];
		if (shipp_hit->ship_max_hull_strength > 1000.0f) {
			float factor = shipp_hit->ship_max_hull_strength / 1000.0f;
			factor = MIN(100.0f, factor);
			damage /= factor;
		}

		//	Don't penalize much at all for hitting cargo
		if (sip->class_type > -1) {
			damage *= Ship_types[sip->class_type].ff_multiplier;
		}

		//	Hit ship, but not targeting it, so it's not so heinous, maybe an accident.
		if (Ai_info[shipp_hitter->ai_index].target_objnum != OBJ_INDEX(objp_hit)) {
			damage /= 5.0f;
		}

		pp->friendly_last_hit_time = Missiontime;
		pp->friendly_hits++;

		// cap damage and number of hits done this frame
		float accredited_damage = MIN(MAX_BURST_DAMAGE, pp->damage_this_burst + damage) - pp->damage_this_burst;
		pp->friendly_damage += accredited_damage;
		pp->damage_this_burst += accredited_damage;

		// Done with adjustments to damage.  Evaluate based on current friendly_damage
		nprintf(("AI", "Friendly damage: %.1f, threshold: %.1f, inc damage: %.1f, max burst: %d\n", pp->friendly_damage, FRIENDLY_DAMAGE_THRESHOLD * (1.0f + (float) (NUM_SKILL_LEVELS + 1 - Game_skill_level)/3.0f), pp->damage_this_burst, MAX_BURST_DAMAGE ));
		
		if (is_instructor(objp_hit)) {
			// it's not nice to hit your instructor
			if (pp->friendly_damage > FRIENDLY_DAMAGE_THRESHOLD) {
				message_send_builtin_to_player( MESSAGE_INSTRUCTOR_ATTACK, NULL, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, -1);
				pp->last_warning_message_time = Missiontime;
				ship_set_subsystem_strength( Player_ship, SUBSYSTEM_WEAPONS, 0.0f);

				training_fail();

				//	Instructor leaves.
				mission_do_departure(objp_hit);
				gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_START_FORCED );	//	Force player to warp out.
			} else if (Missiontime - pp->last_warning_message_time > F1_0*4) {
				// warning every 4 sec
				// use NULL as the message sender here since it is the Terran Command persona
				message_send_builtin_to_player( MESSAGE_INSTRUCTOR_HIT, NULL, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_IMMEDIATE, 0, 0, -1, -1);
				pp->last_warning_message_time = Missiontime;
			}

		// not nice to hit your friends
		} else if (pp->friendly_damage > FRIENDLY_DAMAGE_THRESHOLD * (1.0f + (float) (NUM_SKILL_LEVELS + 1 - Game_skill_level)/3.0f)) {
			process_friendly_hit_message( MESSAGE_HAMMER_SWINE, objp_hit );
			mission_goal_fail_all();
			ai_abort_rearm_request( Player_obj );

			Player_ship->team = Iff_traitor;

		} else if ((damage > frand()) && (Missiontime - pp->last_warning_message_time > F1_0*4) && (pp->friendly_damage > FRIENDLY_DAMAGE_THRESHOLD)) {
			// no closer than 4 sec intervals
			//	Note: (damage > frand()) added on 12/9/97 by MK.  Since damage is now scaled down for big ships, we could get too
			//	many warnings.  Kind of tedious.  frand() returns a value in 0..1, so this won't affect legit hits.
			process_friendly_hit_message( MESSAGE_OOPS, objp_hit );
			pp->last_warning_message_time = Missiontime;
		}
	}
}

/**
 * Maybe make ship with ai_info *aip attack hitter_objnum as a dynamic goal
 */
void maybe_set_dynamic_chase(ai_info *aip, int hitter_objnum)
{
	Assert(Ship_info[Ships[aip->shipnum].ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER));

	// limit the number of ships attacking hitter_objnum (for now, only if hitter_objnum is player)
	if ( ai_maybe_limit_attackers(hitter_objnum) == 1 ) {
		return;
	}

	// only set as target if can be targeted.
	if (awacs_get_level(&Objects[hitter_objnum], &Ships[aip->shipnum], 1) < 1) {
		return;
	}

	if (aip->target_objnum != hitter_objnum)
		aip->aspect_locked_time = 0.0f;
	set_target_objnum(aip, hitter_objnum);
	aip->resume_goal_time = Missiontime + i2f(20);	//	Only chase up to 20 seconds.
	aip->active_goal = AI_ACTIVE_GOAL_DYNAMIC;

	set_targeted_subsys(aip, NULL, -1);		//	Say not attacking any particular subsystem.

	aip->previous_submode = aip->mode;
	aip->mode = AIM_CHASE;
	aip->submode = SM_ATTACK;
	aip->submode_start_time = Missiontime;
}


//	Return true if *objp has armed an aspect seeking bomb.
//	This function written so a ship with an important bomb to fire will willingly take hits in the face to fire its bomb.
int firing_aspect_seeking_bomb(object *objp)
{
	ship	*shipp;
	int	bank_index;
	ship_weapon	*swp;

	shipp = &Ships[objp->instance];

	swp = &shipp->weapons;

	bank_index = swp->current_secondary_bank;

	if (bank_index != -1)
		if (swp->secondary_bank_ammo[bank_index] > 0) {
			if (Weapon_info[swp->secondary_bank_weapons[bank_index]].wi_flags & WIF_BOMB) {
				if (Weapon_info[swp->secondary_bank_weapons[bank_index]].wi_flags & WIF_HOMING_ASPECT) {
					return 1;
				}
			}
		}

	return 0;
}

//	*objp collided with big ship *big_objp at global point *collide_pos
//	Make it fly away from the collision point.
// collision_normal is NULL, when a collision is imminent and we just want to bug out.
void big_ship_collide_recover_start(object *objp, object *big_objp, vec3d *collide_pos, vec3d *collision_normal)
{
	ai_info	*aip;

	Assert(objp->type == OBJ_SHIP);

	aip = &Ai_info[Ships[objp->instance].ai_index];

	if (!timestamp_elapsed(aip->big_recover_timestamp) && (aip->ai_flags & AIF_BIG_SHIP_COLLIDE_RECOVER_1))
		return;

	if (collision_normal) {
		aip->big_recover_timestamp = timestamp(2000);
		aip->big_collision_normal = *collision_normal;
	} else {
		aip->big_recover_timestamp = timestamp(500);
	}

	aip->ai_flags &= ~AIF_BIG_SHIP_COLLIDE_RECOVER_2;
	aip->ai_flags |= AIF_BIG_SHIP_COLLIDE_RECOVER_1;

	// big_recover_pos_1 is 100 m out along normal
	vec3d direction;
	if (collision_normal) {
		direction = *collision_normal;
	} else {
		vm_vec_copy_scale(&direction, &objp->orient.vec.fvec, -1.0f);
	}
	vm_vec_scale_add(&aip->big_recover_pos_1, &objp->pos, &direction, 100.0f);

	// go out 200 m from box closest box point
	get_world_closest_box_point_with_delta(&aip->big_recover_pos_2, big_objp, &aip->big_recover_pos_1, NULL, 300.0f);

	accelerate_ship(aip, 0.0f);
}

float max_lethality = 0.0f;

void ai_update_lethality(object *ship_obj, object *other_obj, float damage)
{
	// Goober5000 - stop any trickle-down errors from ship_do_damage
	Assert(other_obj);
	if (!other_obj)
	{
		return;
	}

	Assert(ship_obj);	// Goober5000
	Assert(ship_obj->type == OBJ_SHIP);
	Assert(other_obj->type == OBJ_WEAPON || other_obj->type == OBJ_SHOCKWAVE);
	int dont_count = FALSE;

	int parent = other_obj->parent;
	if (Objects[parent].type == OBJ_SHIP) {
		if (Objects[parent].signature == other_obj->parent_sig) {

			// check damage done to enemy team
			if (iff_x_attacks_y(Ships[ship_obj->instance].team, Ships[Objects[parent].instance].team)) {

				// other is weapon
				if (other_obj->type == OBJ_WEAPON) {
					weapon *wp = &Weapons[other_obj->instance];
					weapon_info *wif = &Weapon_info[wp->weapon_info_index];

					// if parent is BIG|HUGE, don't count beam
					if (Ship_info[Ships[Objects[parent].instance].ship_info_index].flags & (SIF_BIG_SHIP|SIF_HUGE_SHIP)) {
						if (wif->wi_flags & WIF_BEAM) {
							dont_count = TRUE;
						}
					}
				}

				if (!dont_count) {
					float lethality = 0.025f * damage;	// 2 cyclops (@2000) put you at 100 lethality

					// increase lethality weapon's parent ship
					ai_info *aip = &Ai_info[Ships[Objects[parent].instance].ai_index];
					aip->lethality += lethality;
					aip->lethality = MIN(110.0f, aip->lethality);
					// if you hit, don't be less than 0
					aip->lethality = MAX(0.0f, aip->lethality);
				}
			}
		}
	}
}


/**
 * Object *objp_ship was hit by either weapon *objp_weapon or collided into by ship hit_objp at point *hitpos.
 */
void ai_ship_hit(object *objp_ship, object *hit_objp, vec3d *hitpos, int shield_quadrant, vec3d *hit_normal)
{
	int		hitter_objnum = -2;
	object	*objp_hitter = NULL;
	ship		*shipp;
	ai_info	*aip, *hitter_aip;

	shipp = &Ships[objp_ship->instance];
	aip = &Ai_info[shipp->ai_index];

	if (objp_ship->flags & OF_PLAYER_SHIP) {
		//SUSHI: So that hitting a player ship actually resets the last_hit_target_time counter for whoever hit the player.
		//This is all copypasted from code below
		// Added OBJ_BEAM for traitor detection - FUBAR
		if ((hit_objp->type == OBJ_WEAPON) || (hit_objp->type == OBJ_BEAM)) {
			hitter_objnum = hit_objp->parent;
			Assert((hitter_objnum < MAX_OBJECTS));
			if (hitter_objnum == -1) {
				return; // Possible SSM, bail while we still can.
			}
			objp_hitter = &Objects[hitter_objnum];
		} else if (hit_objp->type == OBJ_SHIP) {
			objp_hitter = hit_objp;
		} else {
			Int3();	// Should never happen.
			return;
		}
		Assert(objp_hitter != NULL);
		hitter_aip = &Ai_info[Ships[objp_hitter->instance].ai_index];
		hitter_aip->last_hit_target_time = Missiontime;
		return;
	}

	if ((aip->mode == AIM_WARP_OUT) || (aip->mode == AIM_PLAY_DEAD))
		return;

	if (hit_objp->type == OBJ_SHIP) {
		//	If the object that this ship collided with is a big ship
		if (Ship_info[Ships[hit_objp->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {
			//	And the current object is _not_ a big ship
			if (!(Ship_info[Ships[objp_ship->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))) {
				//	Recover from hitting a big ship.  Note, if two big ships collide, they just pound away at each other.  Oh well.  Recovery looks dumb and it's very late.
				big_ship_collide_recover_start(objp_ship, hit_objp, hitpos, hit_normal);
			}
		}
	}

	// Added OBJ_BEAM for traitor detection - FUBAR
	if ((hit_objp->type == OBJ_WEAPON) || (hit_objp->type == OBJ_BEAM)) {
		if(hit_objp->parent < 0){
			return;
		}
		if ( hit_objp->parent_sig != Objects[hit_objp->parent].signature ){
			return;
		}
		
		hitter_objnum = hit_objp->parent;
		Assert((hitter_objnum >= 0) && (hitter_objnum < MAX_OBJECTS));
		objp_hitter = &Objects[hitter_objnum];

		// lets not check hits by ghosts any further either
		if(objp_hitter->type == OBJ_GHOST)
			return;
		
		//	Hit by a protected ship, don't attack it.
		if (objp_hitter->flags & OF_PROTECTED) {
			if ((Ship_info[shipp->ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER)) && (aip->target_objnum == -1)) {
				if (aip->mode == AIM_CHASE) {
					if (aip->submode != SM_EVADE_WEAPON) {
						aip->mode = AIM_CHASE;
						aip->submode = SM_EVADE_WEAPON;
						aip->submode_start_time = Missiontime;
					}
				} else if (aip->mode != AIM_EVADE_WEAPON) {
					aip->active_goal = AI_ACTIVE_GOAL_DYNAMIC;
					aip->previous_mode = aip->mode;
					aip->previous_submode = aip->submode;
					aip->mode = AIM_EVADE_WEAPON;
					aip->submode = -1;
					aip->submode_start_time = Missiontime;
					aip->mode_time = timestamp(MAX_EVADE_TIME);	//	Evade for up to five seconds.
				}

			}
			return;
		}

		maybe_process_friendly_hit(objp_hitter, objp_ship, hit_objp);		//	Deal with player's friendly fire.

		ship_maybe_ask_for_help(shipp);
	} else if (hit_objp->type == OBJ_SHIP) {
		if (shipp->team == Ships[hit_objp->instance].team)		//	Don't have AI react to collisions between teammates.
			return;
		objp_hitter = hit_objp;
		hitter_objnum = hit_objp-Objects;
	} else {
		Int3();	//	Hmm, what kind of object hit this if not weapon or ship?  Get MikeK.
		return;
	}

	// traitor detection was the only reason for OBJ_BEAM to make it this far.
	// so bail if it wasn't fired by a player.  
	if ((hit_objp->type == OBJ_BEAM) && (!(objp_hitter->flags & OF_PLAYER_SHIP))) 
		return;						

	//	Collided into a protected ship, don't attack it.
	if (hit_objp->flags & OF_PROTECTED)
		return;

	Assert(objp_hitter != NULL);
	hitter_aip = &Ai_info[Ships[objp_hitter->instance].ai_index];
	hitter_aip->last_hit_target_time = Missiontime;
	
	// store the object signature of objp_ship into ai_info, since we want to track the last ship hit by 'hitter_objnum'
	hitter_aip->last_objsig_hit = objp_ship->signature; 

	aip->last_hit_time = Missiontime;

	if (aip->ai_flags & (AIF_NO_DYNAMIC | AIF_KAMIKAZE))	//	If not allowed to pursue dynamic objectives, don't evade.  Dumb?  Maybe change. -- MK, 3/15/98
		return;

	//	If this ship is awaiting repair, abort!
	if (aip->ai_flags & (AIF_AWAITING_REPAIR | AIF_BEING_REPAIRED)) {
		if (get_hull_pct(objp_ship) < 0.3f) {
			//	Note, only abort if hull below a certain level.
			aip->next_rearm_request_timestamp = timestamp(NEXT_REARM_TIMESTAMP/2);	//	Might request again after 15 seconds.
			if ( !(objp_ship->flags & OF_PLAYER_SHIP) )						// mwa -- don't abort rearm for a player
				ai_abort_rearm_request(objp_ship);
		}
	}

	//	If firing a bomb, ignore enemy fire so we can gain lock drop the bomb.
	//	Only ignore fire if aspect_locked_time > 0.5f, as this means we're in range.
	if (firing_aspect_seeking_bomb(objp_ship)) {
		if ((aip->ai_flags & AIF_SEEK_LOCK) && (aip->aspect_locked_time > 0.1f))
			return;
	}

	//	If in AIM_STRAFE mode and got hit by target, maybe attack turret if appropriate
	if (aip->mode == AIM_STRAFE) {
		Assert(hitter_objnum != -2);
		if (aip->target_objnum == hitter_objnum) {
			if ( hit_objp->type == OBJ_WEAPON ) {
				ai_big_strafe_maybe_attack_turret(objp_ship, hit_objp);
			}
			return;
		}
	}

	if ((objp_ship == Player_obj) && !Player_use_ai)	// Goober5000 - allow for exception
		return;		//	We don't do AI for the player.

	maybe_update_guard_object(objp_ship, objp_hitter);

	//	Big ships don't go any further.
	if (!(Ship_info[shipp->ship_info_index].flags & SIF_SMALL_SHIP))
		return;

	//	If the hitter object is the ignore object, don't attack it.
	ship_info	*sip = &Ship_info[shipp->ship_info_index];
	if ((is_ignore_object(aip, objp_hitter-Objects)) && (sip->flags & (SIF_BOMBER | SIF_FIGHTER))) {
		if (aip->mode == AIM_NONE) {
			aip->mode = AIM_CHASE;	//	This will cause the ship to move, if not attack.
			aip->submode = SM_EVADE;
			aip->submode_start_time = Missiontime;
		}
		return;
	}

	//	Maybe abort based on mode.
	switch (aip->mode) {
	case AIM_CHASE:
		if (aip->submode == SM_ATTACK_FOREVER)
			return;

		if ( (hit_objp->type == OBJ_WEAPON) && !(aip->ai_flags & AIF_NO_DYNAMIC) ) {
			if ( ai_big_maybe_enter_strafe_mode(objp_ship, OBJ_INDEX(hit_objp), 1) )
				return;
		}

	case AIM_GUARD:
		//	If in guard mode and far away from guard object, don't pursue guy that hit me.
			if ((aip->guard_objnum != -1) && (aip->guard_signature == Objects[aip->guard_objnum].signature)) {
				if (vm_vec_dist_quick(&objp_ship->pos, &Objects[aip->guard_objnum].pos) > 500.0f) {
					return;
				}
			}
	case AIM_STILL:
	case AIM_STAY_NEAR:
		// Note: Dealt with above, at very top.  case AIM_PLAY_DEAD:
	case AIM_STRAFE:
		break;
	case AIM_EVADE_WEAPON:
	case AIM_EVADE:
	case AIM_GET_BEHIND:
	case AIM_AVOID:
	case AIM_DOCK:
	case AIM_BIGSHIP:
	case AIM_PATH:
	case AIM_NONE:
	case AIM_BAY_DEPART:
	case AIM_SENTRYGUN:
		return;
	case AIM_BAY_EMERGE:
		// If just leaving the docking bay, don't react to enemy fire... just keep flying away from docking bay
		if ( (Missiontime - aip->submode_start_time) < 5*F1_0 ) {
			return;
		}
		break;
	case AIM_WAYPOINTS:
		if (sip->flags & (SIF_FIGHTER | SIF_BOMBER))
			break;
		else
			return;
		break;
	case AIM_SAFETY:
		if ((aip->submode != AISS_1) || (Missiontime - aip->submode_start_time > i2f(1))) {
			aip->submode = AISS_1;
			aip->submode_start_time = Missiontime;
		}
		return;
		break;
	case AIM_WARP_OUT:
		return;
		break;
	default:
		Int3();	//	Bogus mode!
	}

	if (timestamp_elapsed(aip->ok_to_target_timestamp))
		aip->ai_flags &= ~AIF_FORMATION;			//	If flying in formation, bug out!

	aip->hitter_objnum = hitter_objnum;
	aip->hitter_signature = Objects[hitter_objnum].signature;

	//	If the hitter is not on the same team as the hittee, do some stuff.
	if (iff_x_attacks_y(shipp->team, Ships[objp_hitter->instance].team)) {

		if ((hitter_objnum != aip->target_objnum) && (sip->flags & (SIF_FIGHTER | SIF_BOMBER))) {
			maybe_set_dynamic_chase(aip, hitter_objnum);
			maybe_afterburner_after_ship_hit(objp_ship, aip, &Objects[hitter_objnum]);
		} else {
			if ((aip->mode == AIM_CHASE) && ai_near_full_strength(objp_ship)) {
				switch (aip->submode) {
				case SM_ATTACK:
				case SM_SUPER_ATTACK:
				case SM_GET_AWAY:
					break;
				default:
					if (sip->flags & (SIF_FIGHTER | SIF_BOMBER)) {
						maybe_set_dynamic_chase(aip, hitter_objnum);
					}
					maybe_afterburner_after_ship_hit(objp_ship, aip, &Objects[hitter_objnum]);
					break;
				}
			} else if (aip->mode == AIM_CHASE) {
				switch (aip->submode) {
				case SM_ATTACK:
					aip->submode = SM_EVADE;
					aip->submode_start_time = Missiontime;
					break;
				case SM_SUPER_ATTACK:
					if (Missiontime - aip->submode_start_time > i2f(1)) {
						aip->submode = SM_EVADE;
						aip->submode_start_time = Missiontime;
					}
					break;
				case SM_EVADE_BRAKE:
					break;
				case SM_EVADE_SQUIGGLE:
					aip->submode = SM_EVADE;
					aip->submode_start_time = Missiontime;
					break;
				default:
					if (sip->flags & (SIF_BOMBER | SIF_FIGHTER)) {
						maybe_set_dynamic_chase(aip, hitter_objnum);
						maybe_afterburner_after_ship_hit(objp_ship, aip, &Objects[hitter_objnum]);
					}

					break;
				}
			} else {
				// AL 3-15-98: Prevent escape pods from entering chase mode
				if ( (sip->flags & (SIF_BOMBER | SIF_FIGHTER)) ) {
					maybe_set_dynamic_chase(aip, hitter_objnum);
				}
				maybe_afterburner_after_ship_hit(objp_ship, aip, &Objects[hitter_objnum]);
			}
		}
	}
}

//	Ship shipnum has been destroyed.
//	Cleanup.
// the parameter 'method' is used to tell is this ship was destroyed or it departed normally.
// This function will get called in either case, and there are things that should be done if
// the ship actually gets destroyed which shouldn't get done if it departed.
void ai_ship_destroy(int shipnum, int method)
{
	int		objnum;
	object	*other_objp;
	ship		*shipp;
	ship_obj	*so;
	ai_info	*dead_aip;

	Assert((shipnum >= 0) && (shipnum < MAX_SHIPS));
	Assert((Ships[shipnum].ai_index >= 0) && (Ships[shipnum].ai_index < MAX_AI_INFO));
	objnum = Ships[shipnum].objnum;
	dead_aip = &Ai_info[Ships[shipnum].ai_index];

	// if I was getting repaired, or awaiting repair, then cleanup the repair mode.  When awaiting repair, the support objnum
	// is -1.  When the support ship is on the way, the suppoort objnum >= 0 (points to support ship).
	if ( dead_aip->ai_flags & (AIF_AWAITING_REPAIR | AIF_BEING_REPAIRED) ) {
		if ( dead_aip->support_ship_objnum >= 0 )
			ai_do_objects_repairing_stuff( &Objects[objnum], &Objects[dead_aip->support_ship_objnum], REPAIR_INFO_END);
		else
			ai_do_objects_repairing_stuff( &Objects[objnum], NULL, REPAIR_INFO_END );
	}

	// clear bay door animations
	ai_manage_bay_doors(&Objects[objnum], dead_aip, true);

	//	For all objects that had this ship as a target, wipe it out, forcing find of a new enemy.
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		other_objp = &Objects[so->objnum];
		Assert(other_objp->instance != -1);

		shipp = &Ships[other_objp->instance];
		Assert(shipp->ai_index != -1);

		ai_info	*aip = &Ai_info[shipp->ai_index];

		if (aip->target_objnum == objnum) {
			set_target_objnum(aip, -1);
			//	If this ship had a dynamic goal of chasing the dead ship, clear the dynamic goal.
			if (aip->resume_goal_time != -1)
				aip->active_goal = AI_GOAL_NONE;
		}

		if (aip->goal_objnum == objnum) {
			aip->goal_objnum = -1;
			aip->goal_signature = -1;
		}

		if (aip->guard_objnum == objnum) {
			aip->guard_objnum = -1;
			aip->guard_signature = -1;
		}

		if ((aip->guard_wingnum != -1) && (aip->guard_wingnum == Ai_info[Ships[Objects[objnum].instance].ai_index].wing)) {
			if (aip->guard_wingnum != aip->wing)
				ai_set_guard_wing(other_objp, aip->guard_wingnum);
		}

		if (aip->hitter_objnum == objnum)
			aip->hitter_objnum = -1;
	}
}

/**
 * Do stuff at start of deathroll.
 */
void ai_deathroll_start(object *dying_objp)
{
	// make sure this is a ship
	Assert(dying_objp->type == OBJ_SHIP);

	// mark objects we are docked with so we can do damage and separate during death roll
	for (dock_instance *ptr = dying_objp->dock_list; ptr != NULL; ptr = ptr->next)
	{
		object *docked_objp = ptr->docked_objp;
		int docker_index = ptr->dockpoint_used;
		int dockee_index = dock_find_dockpoint_used_by_object(docked_objp, dying_objp);

		dock_dead_dock_objects(dying_objp, docker_index, docked_objp, dockee_index);
	}

	// clean up any rearm-related stuff
	ai_cleanup_rearm_mode(dying_objp);

	// clean up anybody docking or undocking to me
	ai_cleanup_dock_mode_objective(dying_objp);

	// Undock from every object directly docked to dying_objp.  We can't just iterate through the list because
	// we're undocking the objects while we iterate over them and the pointers get seriously messed up.
	// So we just repeatedly remove the first object until the dying object is no longer docked to anything.
	while (object_is_docked(dying_objp))
	{
		object *docked_objp = dock_get_first_docked_object(dying_objp);

		// undock these objects
		ai_do_objects_undocked_stuff(dying_objp, docked_objp);
	}

	// clear my ai mode
	Ai_info[Ships[dying_objp->instance].ai_index].mode = AIM_NONE;
}

//	Object *requester_objp tells rearm ship to abort rearm.
//	Returns true if it succeeded, else false.
//	To succeed means you were previously rearming.
int ai_abort_rearm_request(object *requester_objp)
{
	ship		*requester_shipp;
	ai_info	*requester_aip;

	Assert(requester_objp->type == OBJ_SHIP);
	if(requester_objp->type != OBJ_SHIP){
		return 0;
	}
	Assert((requester_objp->instance >= 0) && (requester_objp->instance < MAX_SHIPS));	
	if((requester_objp->instance < 0) || (requester_objp->instance >= MAX_SHIPS)){
		return 0;
	}
	requester_shipp = &Ships[requester_objp->instance];
	Assert((requester_shipp->ai_index >= 0) && (requester_shipp->ai_index < MAX_AI_INFO));		
	if((requester_shipp->ai_index < 0) || (requester_shipp->ai_index >= MAX_AI_INFO)){
		return 0;
	}	
	requester_aip = &Ai_info[requester_shipp->ai_index];
	
	if (requester_aip->ai_flags & (AIF_AWAITING_REPAIR | AIF_BEING_REPAIRED)){

		// support objnum is always valid once a rearm repair has been requested.  It points to the
		// ship that is coming to repair me.
		if (requester_aip->support_ship_objnum != -1) {
			object	*repair_objp;
			ai_info	*repair_aip;

			repair_objp = &Objects[requester_aip->support_ship_objnum];
			repair_aip = &Ai_info[Ships[repair_objp->instance].ai_index];

			//	Make sure signatures match.  This prevents nasty bugs in which an object
			//	that was repairing another is destroyed and is replaced by another ship
			//	before this code comes around.
			if (repair_objp->signature == requester_aip->support_ship_signature) {

				Assert( repair_objp->type == OBJ_SHIP );

				// if support ship is in the process of undocking, don't do anything.
				if ( repair_aip->submode < AIS_UNDOCK_0 ) {
					ai_do_objects_repairing_stuff( requester_objp, repair_objp, REPAIR_INFO_ABORT );

					if ( repair_aip->submode == AIS_DOCK_4 )
					{
						repair_aip->submode = AIS_UNDOCK_0;
						repair_aip->submode_start_time = Missiontime;
					}
					else
					{
						repair_aip->submode = AIS_UNDOCK_3;
						repair_aip->submode_start_time = Missiontime;
					}
				} else {
					nprintf(("AI", "Not aborting rearm since already undocking\n"));
				}
			}
		} else {
			// setting these flags is the safe things to do.  There may not be a corresponding repair
			// ship for this guys since a repair ship may be currently repairing someone else.
			ai_do_objects_repairing_stuff( requester_objp, NULL, REPAIR_INFO_ABORT );

			// try and remove this guy from an arriving support ship.
			mission_remove_scheduled_repair(requester_objp);
		}

		return 1;
	} else if ( requester_aip->ai_flags & AIF_REPAIRING ) {
		// a support ship can request to abort when he is told to do something else (like warp out).
		// see if this support ships goal_objnum is valid.  If so, then issue this ai_abort comment
		// for the ship that he is enroute to repair
		if ( requester_aip->goal_objnum != -1 ) {
			int val;

			val = ai_abort_rearm_request( &Objects[requester_aip->goal_objnum] );
			return val;
		}
	}

	return 0;
}

// function which gets called from ai-issue_rearm_request and from code in missionparse.cpp
// to actually issue the rearm goal (support_obj to rearm requester_obj);
void ai_add_rearm_goal( object *requester_objp, object *support_objp )
{
	ship *support_shipp, *requester_shipp;
	ai_info *support_aip, *requester_aip;

	support_shipp = &Ships[support_objp->instance];
	requester_shipp = &Ships[requester_objp->instance];
	requester_aip = &Ai_info[requester_shipp->ai_index];

	Assert( support_shipp->ai_index != -1 );
	support_aip = &Ai_info[support_shipp->ai_index];

	// if the requester is a player object, issue the order as the squadmate messaging code does.  Doing so
	// ensures that the player get a higher priority!
	requester_aip->ai_flags |= AIF_AWAITING_REPAIR;	//	Tell that I'm awaiting repair.
	if ( requester_objp->flags & OF_PLAYER_SHIP )
		ai_add_ship_goal_player( AIG_TYPE_PLAYER_SHIP, AI_GOAL_REARM_REPAIR, -1, requester_shipp->ship_name, support_aip );
	else
		ai_add_goal_ship_internal( support_aip, AI_GOAL_REARM_REPAIR, requester_shipp->ship_name, -1, -1 );

}

//	Object *requester_objp requests rearming.
//	Returns objnum of ship coming to repair requester on success
//	Success means you found someone to rearm you and you weren't previously rearming.
int ai_issue_rearm_request(object *requester_objp)
{
	object	*objp;
	ship		*requester_shipp;
	ai_info	*requester_aip;

	Assert(requester_objp->type == OBJ_SHIP);
	Assert((requester_objp->instance >= 0) && (requester_objp->instance < MAX_SHIPS));
	requester_shipp = &Ships[requester_objp->instance];
	Assert((requester_shipp->ai_index >= 0) && (requester_shipp->ai_index < MAX_AI_INFO));
	requester_aip = &Ai_info[requester_shipp->ai_index];
	
	//	Make sure not already awaiting repair.
	if (requester_aip->ai_flags & AIF_AWAITING_REPAIR) {
		nprintf(("AI", "Ship %s already awaiting rearm by ship %s.\n", requester_shipp->ship_name, &Ships[Objects[requester_aip->support_ship_objnum].instance].ship_name));	
		return -1;
	}

	if ( !is_support_allowed(requester_objp) )
		return -1;

	requester_aip->next_rearm_request_timestamp = timestamp(NEXT_REARM_TIMESTAMP);	//	Might request again after this much time.

	// call ship_find_repair_ship to get a support ship.  If none is found, then we will warp one in.  This
	// function will return the next available ship which can repair requester
	objp = ship_find_repair_ship( requester_objp );
	ai_do_objects_repairing_stuff( requester_objp, objp, REPAIR_INFO_QUEUE );
	if ( objp ) {
		return OBJ_INDEX(objp);
	} else {
		// call to warp in repair ship!!!!  for now, warp in any number of ships needed.  Should cap it to
		// some reasonable max (or let support ships warp out).  We should assume here that ship_find_repair_ship()
		// would have returned a valid object if there are too many support ships already in the mission
		mission_bring_in_support_ship( requester_objp );

		return -1;
	}

}

/**
 * Make objp rearm and repair goal_objp
 */
void ai_rearm_repair( object *objp, int docker_index, object *goal_objp, int dockee_index )
{
	ai_info *aip, *goal_aip;

	aip = &Ai_info[Ships[objp->instance].ai_index];
	aip->goal_objnum = OBJ_INDEX(goal_objp);

	ai_dock_with_object(objp, docker_index, goal_objp, dockee_index, AIDO_DOCK);
	aip->ai_flags |= AIF_REPAIRING;						//	Tell that repair guy is busy trying to repair someone.

	goal_aip = &Ai_info[Ships[goal_objp->instance].ai_index];

	goal_aip->support_ship_objnum = OBJ_INDEX(objp);		//	Tell which object is coming to repair.
	goal_aip->support_ship_signature = objp->signature;

	ai_do_objects_repairing_stuff( goal_objp, objp, REPAIR_INFO_ONWAY );

	goal_aip->abort_rearm_timestamp = timestamp(NEXT_REARM_TIMESTAMP*3/2);
}

// Given a dockee object and the index of the dockbay for that object (ie the dockbay index
// into polymodel->dockbays[] for the model associated with the object), return the index
// of a path_num associated with than dockbay (this is an index into polymodel->paths[])
int ai_return_path_num_from_dockbay(object *dockee_objp, int dockbay_index)
{
	if ( dockbay_index < 0 || dockee_objp == NULL ) {
		Int3();		// should never happen
		return -1;
	}

	if ( dockee_objp->type == OBJ_SHIP ) {
		int			path_num;
		polymodel	*pm;

		pm = model_get(Ship_info[Ships[dockee_objp->instance].ship_info_index].model_num );

		Assert(pm->n_docks > dockbay_index);
		Assert(pm->docking_bays[dockbay_index].num_spline_paths > 0);
		Assert(pm->docking_bays[dockbay_index].splines != NULL);
		
		if(pm->n_docks <= dockbay_index){
			return -1;
		}
		if(pm->docking_bays[dockbay_index].num_spline_paths <= 0){
			return -1;
		}
		if(pm->docking_bays[dockbay_index].splines == NULL){
			return -1;
		}

		// We only need to return one path for the dockbay, so return the first
		path_num = pm->docking_bays[dockbay_index].splines[0];
		return path_num;
	} else {
		return -1;
	}
}

/**
 * Actually go ahead and fire the synaptics.
 */
void cheat_fire_synaptic(object *objp, ship *shipp, ai_info *aip)
{
	ship_weapon	*swp;
	swp = &shipp->weapons;
	int	current_bank = swp->current_secondary_bank;

	ai_select_secondary_weapon(objp, swp, WIF_SPAWN, 0);
	if (timestamp_elapsed(swp->next_secondary_fire_stamp[current_bank])) {
		if (ship_fire_secondary(objp)) {
			nprintf(("AI", "ship %s cheat fired synaptic!\n", shipp->ship_name));
			swp->next_secondary_fire_stamp[current_bank] = timestamp(2500);
		}
	}
}

//	For the subspace mission (sm3-09a)
//		for delta wing
//			if they're sufficiently far into the mission
//				if they're near one or more enemies
//					every so often
//						fire a synaptic if they have one.
void maybe_cheat_fire_synaptic(object *objp, ai_info *aip)
{
	//	Only do in subspace missions.
	if (!(The_mission.flags & MISSION_FLAG_SUBSPACE))
		return;

	//	Only do in sm3-09a
	if (!stricmp(Game_current_mission_filename, "sm3-09a"))
	{
		ship	*shipp;
		int	wing_index, time;

		shipp = &Ships[objp->instance];

		if (!(strnicmp(shipp->ship_name, NOX("delta"), 5)))
		{
			wing_index = shipp->ship_name[6] - '1';

			if ((wing_index >= 0) && (wing_index < MAX_SHIPS_PER_WING))
			{
				time = Missiontime >> 16;	//	Convert to seconds.
				time -= 2*60;				//	Subtract off two minutes.

				if (time > 0)
				{
					int modulus = 17 + wing_index*3;

					if ((time % modulus) < 2)
					{
						int count = num_nearby_fighters(iff_get_attackee_mask(obj_team(objp)), &objp->pos, 1500.0f);

						if (count > 0)
							cheat_fire_synaptic(objp, shipp, aip);
					}
				}
			}
		}
	}
}
