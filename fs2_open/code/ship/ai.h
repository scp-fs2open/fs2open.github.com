/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/ai.h $
 * $Revision: 2.6 $
 * $Date: 2003-01-07 20:06:44 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2003/01/06 22:57:23  Goober5000
 * implemented keep-safe-distance
 * --Goober5000
 *
 * Revision 2.4  2003/01/03 21:58:07  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 2.3  2002/12/10 05:43:33  Goober5000
 * Full-fledged ballistic primary support added!  Try it and see! :)
 *
 * Revision 2.2  2002/10/19 19:29:28  bobboau
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam sheild hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 12    8/29/99 4:18p Andsager
 * New "burst" limit for friendly damage.  Also credit more damage done
 * against large friendly ships.
 * 
 * 11    8/26/99 5:14p Andsager
 * 
 * 10    7/26/99 12:14p Andsager
 * Apply cap to how much slower a transport flies with cargo.  Remove
 * limit on waypoint speed for training.  Enemy ai get stealth exact pos
 * when stealth fires
 * 
 * 9     7/02/99 10:56a Andsager
 * Put in big ship - big ship attack mode.  Modify stealth sweep ai.
 * 
 * 8     6/30/99 5:53p Dave
 * Put in new anti-camper code.
 * 
 * 7     6/23/99 5:51p Andsager
 * Add waypoint-cap-speed.  Checkin stealth ai - inactive.
 * 
 * 6     6/14/99 10:45a Dave
 * Made beam weapons specify accuracy by skill level in the weapons.tbl
 * 
 * 5     4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 4     4/20/99 3:40p Andsager
 * Changes to big ship ai.  Uses bounding box as limit where to fly to
 * when flying away.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 186   6/09/98 5:15p Lawrance
 * French/German localization
 * 
 * 185   5/23/98 2:41p Mike
 * Make Easy the default skill level and prevent old pilot's skill level
 * from carrying into new pilot.
 * 
 * 184   5/20/98 12:45a Mike
 * Fix bug in support ships aborting rearm.  Also, remove test code that
 * made any ship whose name began, "Kami" a kamikaze.
 * 
 * 183   5/18/98 1:58a Mike
 * Make Phoenix not be fired at fighters (but yes bombers).
 * Improve positioning of ships in guard mode.
 * Make turrets on player ship not fire near end of support ship docking.
 * 
 * 182   5/13/98 4:41p Mike
 * Make big ships try a tiny bit to avoid collision with each other when
 * attacking another big ship.  Make ships a little less likely to collide
 * into player when in formation, drop off if player flying wacky.
 * 
 * 181   5/12/98 4:53p Mike
 * Make ships recover from collisions with big ships, eventually flying
 * around.
 * 
 * 180   5/10/98 11:30p Mike
 * Better firing of bombs, less likely to go into strafe mode.
 * 
 * 179   4/12/98 2:02p Mike
 * Make small ships avoid big ships.
 * Turn on Collide_friendly flag.
 * 
 * 178   4/10/98 11:02p Mike
 * Make countermeasures less effective against aspect seekers than against
 * heat seekers.
 * Make AI ships match bank with each other when attacking a faraway
 * target.
 * Make ships not do silly loop-de-loop sometimes when attacking a faraway
 * target.
 * 
 * 177   4/08/98 8:38a Mike
 * Make ships avoid player when attacking or guarding a faraway object.
 * 
 * 176   4/06/98 12:46a Allender
 * bump max ai goals down to 5
 * 
 * 175   4/01/98 9:20a Mike
 * Reduce MAX_SHIPS, MAX_OBJECTS and make MAX_AI_INFO same as MAX_SHIPS
 * 
 * 174   3/21/98 3:36p Mike
 * Fix/optimize attacking of big ships.
 * 
 * 173   3/17/98 12:50a Mike
 * Improved kamikaze behavior.
 * 
 * 172   3/16/98 8:27p Allender
 * Fred support for two new AI flags -- kamikaze and no dynamic goals.
 * 
 * 171   3/16/98 12:03a Mike
 * Add support for kamikaze mode.  Add AIF_NO_DYNAMIC which means
 * relentlessly pursue current goal.
 * 
 * 170   3/15/98 9:44p Lawrance
 * Get secondary weapons for turrets working
 * 
 * 169   3/13/98 8:34a Mike
 * Intermediate checkin to resolve link errors.  Working on detecting
 * whether dock with player ship is obstructed by another ship
 * 
 * 168   3/09/98 5:12p Mike
 * Make sure Pl_objp uses are valid.
 * Throw asteroids at ships in asteroid field, not "Gal"
 * Support ships warp out if no goals after 30 seconds.
 * 
 * 167   2/26/98 10:07p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 166   2/22/98 4:17p John
 * More string externalization classification... 190 left to go!
 * 
 * 165   2/18/98 10:34p Allender
 * repair/rearm system (for single and multi) about finished.
 * dock/undock and ai goals packets implemented for multiplayer
 * 
 * 164   2/17/98 5:03p Allender
 * major cdhanges to rearm repair code.  All flag and variable setting
 * done in one function.  A little more work to do.  Fix bug in squad
 * messaging when hotkey was used on invalid target
 * 
 * 163   2/16/98 10:13p Allender
 * initial work on being able to target weapons (bombs specifically).
 * Work on getting rearm/repair working under multiplayer
 * 
 * $NoKeywords: $
 */

#ifndef _AI_H
#define _AI_H

#include "globalincs/pstypes.h"
#include "object/object.h"
#include "parse/parselo.h"
#include "cfile/cfile.h"
#include "globalincs/systemvars.h"

#define	AI_DEFAULT_CLASS 3  // default AI class for new ships (Fred)

#define	AIF_FORMATION_WING					(1 << 0)		//	Fly in formation as part of wing.
#define	AIF_AWAITING_REPAIR					(1 << 1)		//	Awaiting a repair ship.
#define	AIF_BEING_REPAIRED					(1 << 2)		//	Currently docked with repair ship.
#define	AIF_REPAIRING							(1 << 3)		//	Repairing a ship (or going to repair a ship)
#define	AIF_DOCKED								(1 << 4)		// this object docked with something else
#define	AIF_SEEK_LOCK							(1 << 5)		//	set if should focus on gaining aspect lock, not hitting with lasers
#define	AIF_FORMATION_OBJECT					(1 << 6)		//	Fly in formation off a specific object.
#define	AIF_TEMPORARY_IGNORE					(1 << 7)		//	Means current ignore_objnum is only temporary, not an order from the player.
#define	AIF_USE_EXIT_PATH						(1	<< 8)		// Used by path code, to flag path as an exit path
#define	AIF_USE_STATIC_PATH					(1	<< 9)		// Used by path code, use fixed path, don't try to recreate
#define	AIF_TARGET_COLLISION					(1 << 10)	//	Collided with aip->target_objnum last frame.  Avoid that ship for half a second or so.
#define	AIF_UNLOAD_SECONDARIES				(1 << 11)	//	Fire secondaries as fast as possible!
#define	AIF_ON_SUBSYS_PATH					(1	<<	12)	// Current path leads to a subsystem
#define	AIF_AVOID_SHOCKWAVE_SHIP			(1 << 13)	//	Avoid an existing shockwave from a ship.
#define	AIF_AVOID_SHOCKWAVE_WEAPON			(1 << 14)	//	Avoid an expected shockwave from a weapon.  shockwave_object field contains object index.
#define	AIF_AVOID_SHOCKWAVE_STARTED		(1 << 15)	//	Already started avoiding shockwave, don't keep deciding whether to avoid.
#define	AIF_ATTACK_SLOWLY						(1 << 16)	//	Move slowly while attacking.
#define	AIF_REPAIR_OBSTRUCTED				(1 << 17)	//	Ship wants to be repaired, but path is obstructed.
#define	AIF_KAMIKAZE							(1 << 18)	//	Crash into target
#define	AIF_NO_DYNAMIC							(1 << 19)	//	Not allowed to get dynamic goals
#define	AIF_AVOIDING_SMALL_SHIP				(1 << 20)	//	Avoiding a player ship.
#define	AIF_AVOIDING_BIG_SHIP				(1 << 21)	//	Avoiding a large ship.
#define	AIF_BIG_SHIP_COLLIDE_RECOVER_1	(1 << 22)	//	Collided into a big ship.  Recovering by flying away.
#define	AIF_BIG_SHIP_COLLIDE_RECOVER_2	(1 << 23)	//	Collided into a big ship.  Fly towards big ship sphere perimeter.
#define	AIF_STEALTH_PURSUIT					(1 << 24)	// Ai is trying to fight stealth ship

// Goober5000
#define	AIF_UNLOAD_PRIMARIES				(1 << 25)	//	Fire primaries as fast as possible!

#define	AIF_AVOID_SHOCKWAVE	(AIF_AVOID_SHOCKWAVE_SHIP | AIF_AVOID_SHOCKWAVE_WEAPON)
#define	AIF_FORMATION			(AIF_FORMATION_WING | AIF_FORMATION_OBJECT)

//	dock_orient_and_approach() modes.
#define	DOA_APPROACH	1		//	Approach the current point on the path (aip->path_cur)
#define	DOA_DOCK			2		//	Dock with goal object.
#define	DOA_UNDOCK_1	3		//	Begin undocking with goal object.  Just move away.
#define	DOA_UNDOCK_2	4		//	Secondary undocking.  Move away.
#define	DOA_UNDOCK_3	5		//	Tertiary undocking.  Move away and orient away.
#define	DOA_DOCK_STAY	6		//	Rigidly maintain position in dock bay.

//	Type values for ai_dock_with_object() dock_type parameter.
#define	AIDO_DOCK		1		//	Set goal of docking with object.
#define	AIDO_DOCK_NOW	2		//	Immediately move into dock position.  For ships that start mission docked.
#define	AIDO_UNDOCK		3		//	Set goal of undocking with object.

#define MAX_AI_GOALS	5

//	Submodes for seeking safety.
#define	AISS_1	41				//	Pick a spot to fly to.
#define	AISS_2	42				//	Flying to spot.
#define	AISS_3	43				// Gotten near spot, fly about there.
#define	AISS_1a	44				//	Pick a new nearby spot because we are endangered, then go to AISS_2

// types of ai goals -- tyese types will help us to determination on which goals should
// have priority over others (i.e. when a player issues a goal to a wing, then a seperate
// goal to a ship in that wing).  We would probably use this type in conjunction with
// goal priority to establish which goal to follow
#define AIG_TYPE_EVENT_SHIP		1		// from mission event direct to ship
#define AIG_TYPE_EVENT_WING		2		// from mission event direct to wing
#define AIG_TYPE_PLAYER_SHIP		3		// from player direct to ship
#define AIG_TYPE_PLAYER_WING		4		// from player direct to wing
#define AIG_TYPE_DYNAMIC			5		// created on the fly

// flags for AI_GOALS
#define AIGF_DOCKER_NAME_VALID	(1<<0)	// when set, name field for docker is valid
#define AIGF_DOCKEE_NAME_VALID	(1<<1)	// when set, name field for dockee is valid
#define AIGF_GOAL_ON_HOLD			(1<<2)	// when set, this goal cannot currently be satisfied, although it could be in the future
#define AIGF_SUBSYS_NAME_VALID	(1<<3)	// when set, the subsystem name (for a destroy subsystem goal) is valid, and stored in docker.name field!!
#define AIGF_GOAL_OVERRIDE			(1<<4)	// paired with AIG_TYPE_DYNAMIC to mean this goal overrides any other goal
#define AIGF_PURGE					(1<<5)	// purge this goal next time we process
#define AIGF_GOALS_PURGED			(1<<6)	// this goal has already caused other goals to get purged

//	Flags to ai_turn_towards_vector().
#define	AITTV_FAST					(1<<0)	//	Turn fast, not slowed down based on skill level.

#define	KAMIKAZE_HULL_ON_DEATH	-1000.0f	//	Hull strength ship gets set to if it crash-dies.

// Goober5000, currently only used for the ai-chase-any-except behavior.  This can never go
// above 32, because 32 is the maximum number of bits possible in a unit data type.  If you
// need more, you're out of luck - you'll have to add a second special object array. :)
#define MAX_SPECIAL_OBJECTS	32

// structure for AI goals
typedef struct ai_goals {
	int	signature;			//	Unique identifier.  All goals ever created (per mission) have a unique signature.
	int	ai_mode;				// one of the AIM_* modes for this goal
	int	ai_submode;			// maybe need a submode
	int	type;					// one of the AIG_TYPE_* values above
	int	flags;				// one of the AIGF_* values above
	fix	time;					// time at which this goal was issued.
	int	priority;			// how important is this goal -- number 0 - 100
	char	*ship_name;			// name of the ship that this goal acts upon
	int	ship_name_index;	// index of ship_name in Goal_ship_names[][]
	int	wp_index;			// index into waypoints list of waypoints that this ship might fly.
	int	weapon_signature;	// signature of weapon this ship might be chasing.  Paired with above value to get target.

	// Goober5000
	char	*special_object[MAX_SPECIAL_OBJECTS];		// name of ship (or wing) that this goal acts upon
	int		special_object_num[MAX_SPECIAL_OBJECTS];	// index of ship or wing in Ship[] or Wing[]
	int		special_object_index[MAX_SPECIAL_OBJECTS];	// index of special_object in Goal_ship_names[][]
	uint	special_object_flags;
	int		num_special_objects;

	// unions for docking stuff.
	union {
		char	*name;
		int	index;
	} docker;
	
	union {
		char	*name;
		int	index;
	} dockee;

} ai_goal;

#include "ship/ship.h"  // ai_goal must be declared before including this.

#define	MAX_AI_CLASSES		10
#define	MAX_GOAL_SHIP_NAMES	100

#define	AIM_CHASE				0
#define	AIM_EVADE				1
#define	AIM_GET_BEHIND			2
#define	AIM_STAY_NEAR			3		//	Stay near another ship.
#define	AIM_STILL				4		//	Sit still.  Don't move.  Hold your breath.  Don't blink.
#define	AIM_GUARD				5		//	Guard an object
#define	AIM_AVOID				6		//	Avoid an object
#define	AIM_WAYPOINTS			7		//	Fly waypoints
#define	AIM_DOCK					8		//	Dock with ship.
#define	AIM_NONE					9		//	Uh, do nothing.
#define	AIM_BIGSHIP				10		//	Like a capital ship, doesn't focus on one ship.
#define	AIM_PATH					11		//	Follow path on ship
#define	AIM_BE_REARMED			12		//	Allow self to be rearmed
#define	AIM_SAFETY				13		//	Seek safety at periphery of battle
#define	AIM_EVADE_WEAPON		14		//	Evade a weapon.
#define	AIM_STRAFE				15		// attack a big ship by strafing it
#define	AIM_PLAY_DEAD			16		//	Play dead.  Get it?  Don't move, fire, etc.
#define	AIM_BAY_EMERGE			17		// Emerging from a fighter bay, following path to do so
#define	AIM_BAY_DEPART			18		// Departing to a fighter bay, following path to do so
#define	AIM_SENTRYGUN			19		// AI mode for sentry guns only (floating turrets)
#define	AIM_WARP_OUT			20		//	Commence warp out sequence.  Point in legal direction.  Then call John's code.

#define	MAX_AI_BEHAVIORS		21	//	Number of AIM_xxxx types

#define	MAX_WAYPOINTS_PER_LIST	20
#define	MAX_WAYPOINT_LISTS		32
#define	MAX_ENEMY_DISTANCE	2500.0f			//	maximum distance from which a ship will pursue an enemy.

// waypoint list flags bitmasks.
#define WL_MARKED	0x01

typedef struct waypoint_list {
	char		name[NAME_LENGTH];
	int		count;
	char		flags[MAX_WAYPOINTS_PER_LIST];
	vector	waypoints[MAX_WAYPOINTS_PER_LIST];
} waypoint_list;

#define AI_GOAL_NONE				-1

#define	AI_ACTIVE_GOAL_DYNAMIC	999

typedef struct ai_class {
	char	name[NAME_LENGTH];
	float	ai_accuracy[NUM_SKILL_LEVELS];
	float	ai_evasion[NUM_SKILL_LEVELS];
	float	ai_courage[NUM_SKILL_LEVELS];
	float	ai_patience[NUM_SKILL_LEVELS];
} ai_class;

//	Submode definitions.
//	Note: These need to be renamed to be of the form: AIS_mode_xxxx
#define	SM_CONTINUOUS_TURN	1	// takes parm: vector_id {0..3 = right, -right, up, -up}
#define	SM_ATTACK				2
#define	SM_EVADE_SQUIGGLE		3
#define	SM_EVADE_BRAKE			4
#define	SM_EVADE					5
#define	SM_SUPER_ATTACK		6
#define	SM_AVOID					7
#define	SM_GET_BEHIND			8
#define	SM_GET_AWAY				9
#define	SM_EVADE_WEAPON		10		//	Evade incoming weapon
#define	SM_FLY_AWAY				11		//	Fly away from target_objnum
#define	SM_ATTACK_FOREVER		12		//	Engine subsystem destroyed, so attack, never evading, avoiding, etc.
#define	SM_STEALTH_FIND		13		// Stealth ship is "targeted", but not visible, so try to find based on predicted pos
#define	SM_STEALTH_SWEEP		14		// General sweep, looking for stealth after not visible for some time.
#define	SM_BIG_APPROACH		15		// Big ship approaches another
#define	SM_BIG_CIRCLE			16		// Big ship flies circle around other big ship to get good angle to go parallel
#define	SM_BIG_PARALLEL		17		// Big ship flies parallel to another

//	Submodes for docking behavior
#define	AIS_DOCK_0		21
#define	AIS_DOCK_1		22
#define	AIS_DOCK_2		23
#define	AIS_DOCK_3		24
#define	AIS_DOCK_3A		25
#define	AIS_DOCK_4		26			//	Only for rearm/repair.
#define	AIS_DOCK_4A		27			//	Only for not rearm/repair.  MK, 7/15/97
#define	AIS_UNDOCK_0	30
#define	AIS_UNDOCK_1	31
#define	AIS_UNDOCK_2	32
#define	AIS_UNDOCK_3	33
#define	AIS_UNDOCK_4	34

//	Submodes for Guard behavior
#define	AIS_GUARD_PATROL		101
#define	AIS_GUARD_ATTACK		102
#define	AIS_GUARD_2				103
#define	AIS_GUARD_STATIC		104					//	maintain current relative position to guard object, if possible

// Submodes for strafing big ships behavior (AIM_STRAFE)
#define	AIS_STRAFE_ATTACK		201	// fly towards target and attack
#define	AIS_STRAFE_AVOID		202	// fly evasive vector to avoid incoming fire
#define	AIS_STRAFE_RETREAT1	203	// fly away from attack point
#define	AIS_STRAFE_RETREAT2	204
#define	AIS_STRAFE_POSITION	205	// re-position to resume strafing attack

#define	WPF_REPEAT				(1 << 0)
#define	WPF_BACKTRACK			(1 << 1)

#define	PD_FORWARD				1
#define	PD_BACKWARD				-1

#define	MIN_TRACKABLE_ASPECT_DOT	0.992f		//	dot of fvec and vec_to_enemy to progress towards aspect lock

//	Submodes for warping out.
#define	AIS_WARP_1				300	//	Make sure there is no obstruction to warping out.
#define	AIS_WARP_2				301
#define	AIS_WARP_3				302
#define	AIS_WARP_4				303
#define	AIS_WARP_5				304

//	A node on a path.
//	Contains global location of point.
//	Contains hooks back to original path information.
//	This hook is used to extract information on the point such as whether it is
//	protected by turrets.
typedef struct pnode {
	vector	pos;
	int		path_num;			//	path number from polymodel, ie in polymodel, paths[path_num]
	int		path_index;			//	index in original model path of point, ie in model_path, use verts[path_index]
} pnode;

#define	MAX_PATH_POINTS	1000
extern pnode	Path_points[MAX_PATH_POINTS];
extern pnode	*Ppfp;			//	Free pointer in path points.

typedef struct ai_info {
	int		ai_flags;				//	Special flags for AI behavior.
	int		shipnum;					// Ship using this slot, -1 means none.
	int		type;						//	
	int		wing;						//	Member of what wing? -1 means none. 

	int		behavior;				//	AI Class.  Doesn't change after initial setting.
	int		mode;
	int		previous_mode;
	int		mode_time;				//	timestamp at which current mode elapses.
	int		target_objnum;			//	object index of current target.
	int		target_signature;		//	Signature of current target.
	int		previous_target_objnum;	//	On 5/19/97, only used for player.

	int		stealth_last_cheat_visible_stamp;	// when within 100m, always update pos and velocity, with error increasing for increasing time from last legal visible
	int		stealth_last_visible_stamp;
	float		stealth_sweep_box_size;
	vector	stealth_last_pos;
	vector	stealth_velocity;

	float		previous_dot_to_enemy;	//	dot(fvec, vec_to_enemy) last frame
	float		target_time;			//	Amount of time continuously targeting this ship.

	int		enemy_wing;				//	When picking an enemy wing, only allow to be in enemy_wing, unless == -1, in which case don't care.
	int		attacker_objnum;
	int		goal_objnum;			//	mode specific goal.  In DOCK, ship to dock with.
	int		goal_signature;

	int		guard_objnum;			//	Ship to guard.
	int		guard_signature;		//	Signature of ship to guard.
	int		guard_wingnum;			//	Wing to guard.  guard_objnum set to leader.

	int		ignore_objnum;			//	ship to be ignored, based on player order.  UNUSED_OBJNUM if none.  -(wing_num+1) if ignoring wing.
	int		ignore_signature;		//	signature of ship to be ignored

	int		ai_class;				//	Class.  Might be override of default.

	//	Probably become obsolete, to be replaced by path_start, path_cur, etc.
	int		wp_list;					// waypoint list index
	int		wp_index;				// waypoint index in list
	int		wp_flags;				//	waypoint flags, see WPF_xxxx
	int		wp_dir;					//	1 or -1, amount to add to get to next waypoint index.
	int		waypoint_speed_cap;	// -1 no cap, otherwise cap - changed to int by Goober5000

	//	Path following information
	int		path_start;				//	Index into global array, start of path.
	int		path_cur;				//	Index into global array, current location in path.
	int		path_length;			//	Number of links in this path.
	int		path_dir;				//	PD_FORWARD, PD_BACKWARD
	int		path_flags;				//	loop, backtrack, whatever else.
	int		path_objnum;			//	Object of interest.  It's model contains the path.
	int		path_goal_obj_hash;	//	Hash value of goal object when global path created.
	fix		path_next_create_time;	//	Next time at which we'll create a global path.
	vector	path_create_pos;		//	Object's position at time of global path creation.
	matrix	path_create_orient;	//	Object's orientation at time of global path creation.
	int		mp_index;				//	Model path index.  Index in polymodel:model_paths
	fix		path_next_check_time;	//	Last time checked to see if would collide with model.
	int		path_goal_dist;		// minimum distance to first path point to consider path reached
	int		path_subsystem_next_check;	// timestamp to next check if subsystem is still visible

	int		submode;
	int		previous_submode;		// previous submode, get it?
	float		best_dot_to_enemy;	//	best dot product to enemy in last BEST_DOT_TIME seconds
	float		best_dot_from_enemy;	// best dot product for enemy to player in last BEST_DOT_TIME seconds
	fix		best_dot_to_time;		// time at which best dot occurred
	fix		best_dot_from_time;	// time at which best dot occurred
	fix		submode_start_time;	// time at which we entered the current submode
	int		submode_parm0;			//	parameter specific to current submode
	fix		next_predict_pos_time;			//	Next time to predict position.

	ai_goal	goals[MAX_AI_GOALS];
	int		active_goal;			//	index of active goal, -1 if none, AI_ACTIVE_GOAL_DYNAMIC if dynamic (runtime-created) goal
	int		goal_check_time;		// timer used for processing goals for this ai object

	vector	last_predicted_enemy_pos;		//	Where he thought enemy was last time.
	float		time_enemy_in_range;				//	Amount of time enemy continuously in "sight", near crosshair.
	fix		last_attack_time;					//	Missiontime of last time this ship attacked its enemy.
	fix		last_hit_time;						//	Missiontime of last time this ship was hit by anyone.
	int		last_hit_quadrant;				//	Shield section of last hit.
	fix		last_hit_target_time;			//	Missiontime of last time this ship successfully hit target.
	int		hitter_objnum;						//	Object index of ship that hit this ship last time.
	int		hitter_signature;					//	Signature of hitter.  Prevents stupidity if hitter gets killed.
	fix		resume_goal_time;					//	Time at which to resume interrupted goal, if nothing else intervenes.
	float		prev_accel;							//	Acceleration last frame.
	float		prev_dot_to_goal;					//	dot of fvec to goal last frame, used to see if making progress towards goal.
	vector	goal_point;							//	Used in AIM_SAFETY, AIM_STILL and in circling.
	vector	prev_goal_point;					//	Previous location of goal point, used at least for evading.
	float		ai_accuracy, ai_evasion, ai_courage, ai_patience;
	union {
	float		lead_scale;							//	Amount to lead current opponent by.
	float		stay_near_distance;				//	Distance to stay within for AIM_STAY_NEAR mode.
	};

	ship_subsys*	targeted_subsys;			// Targeted subobject on current target.  NULL if none;
	ship_subsys*	last_subsys_target;		// last known subsystem target
	int				targeted_subsys_parent;	//	Parent objnum of subobject, not necessarily targeted

	float		aspect_locked_time;				//	Time towards acquiring lock for current_target

//	ship_subsys	*targeted_subobject;			//	subsystem to attack
//	int		attack_subsystem_parent;		//	objnum of the object containing the attack_subsystem
	int		dock_index;							// index of docking point to use when docking.
	int		dockee_index;						//	index of dock point on other ship.
	int		dock_path_index;					// index of docking path to use when docking.
	int		dock_objnum;						// objnum of ship we are docked with.
	int		dock_signature;					//	Signature of repair object.
	int		danger_weapon_objnum;			//	Closest objnum of weapon fired at this ship.
	int		danger_weapon_signature;		//	Signature of object danger_weapon_objnum.

	vector	guard_vec;							//	vector to object being guarded, only used in AIS_GUARD_STATIC submode
	int		nearest_locked_object;			//	Nearest locked object.
	float		nearest_locked_distance;		//	Distance to nearest locked object.

	float		current_target_distance;		// Distance of current target from player
	int		current_target_is_locked;		// Flag to indicate whether the current target is locked for missile fire
	int		current_target_dist_trend;		// Tracks whether distance to target is increasing or decreasing
	int		current_target_speed_trend;	// Tracks whether speed of target is increasing or decreasing

	float		last_dist;							// last frame's distance between player and target
	float		last_speed;							// last frame's target speed
	int		last_secondary_index;			// needed for secondary weapon change check
	int		last_target;

	int		rearm_first_missile;				// flag to show that reloading of missilies hasn't begun yet
	int		rearm_first_ballistic_primary;		// flag to show that reloading of ballistic primaries hasn't begun yet
	int		rearm_release_delay;				// timestamp used to delay separation of ships after rearm complete

	fix		afterburner_stop_time;			//	Missiontime to turn off afterburner
	int		last_objsig_hit;					// The object number signature of the ship last hit by this ship
	int		ignore_expire_timestamp;		//	Timestamp at which temporary ignore (AIF_TEMPORARY_IGNORE) expires.
	int		warp_out_timestamp;				//	Timestamp at which this ship is to warp out.
	int		next_rearm_request_timestamp;	//	Timestamp at which ship might next request rearm.
	int		primary_select_timestamp;		//	When to next select a primary weapon.
	int		secondary_select_timestamp;	//	When to next select a secondary weapon.

	int		scan_for_enemy_timestamp;		// When to next look for enemy fighters if sitting still while pounding
														// on a bigship.   SCAN_FIGHTERS_INTERVAL is defined in AiBig.h
	int		choose_enemy_timestamp;			//	Time at which it is next legal to choose a new enemy (does not apply 
														// to special situations, like getting hit by a weapon)
	int		force_warp_time;					//	time at which to give up avoiding a ship and just warp out

	int		shockwave_object;					//	Object index of missile that will generate a shockwave.  We will try to avoid.

	int		shield_manage_timestamp;		//	Time at which to next manage shield.
	int		self_destruct_timestamp;		//	Time at which to self-destruct, probably due to being disabled.
	int		ok_to_target_timestamp;			//	Time at which this ship can dynamically target.

	float		kamikaze_damage;					// some damage value used to produce a shockwave from a kamikaze ship
	vector	big_attack_point;					//	Global point this ship is attacking on a big ship.
	vector	big_attack_surface_normal;		// Surface normal at ship at big_attack_point;
	int		pick_big_attack_point_timestamp; //	timestamp at which to pick a new point to attack on a big ship.

	//	Note: These three avoid_XX terms are shared between the code that avoids small (only player now) and large ships
	//	The bits in ai_flags determine which is occurring.  AIF_AVOID_SMALL_SHIP, AIF_AVOID_BIG_SHIP
	int		avoid_ship_num;					//	object index of small ship to avoid
	vector	avoid_goal_point;					//	point to aim at when avoiding a ship
	fix		avoid_check_timestamp;			//	timestamp at which to next check for having to avoid ship

	vector	big_collision_normal;			// Global normal of collision with big ship.  Helps find direction to fly away from big ship.  Set for each collision.
	vector	big_recover_pos_1;				//	Global point to fly towards when recovering from collision with a big ship, stage 1.
	vector	big_recover_pos_2;				//	Global point to fly towards when recovering from collision with a big ship, stage 2.
	int		big_recover_timestamp;			//	timestamp at which it's OK to re-enter stage 1.

	int		abort_rearm_timestamp;			//	time at which this rearm should be aborted in a multiplayer game.

	// artillery targeting info
	int		artillery_objnum;					// object currently being targeted for artillery lock/attack
	int		artillery_sig;						// artillery object signature
	float		artillery_lock_time;				// how long we've been locked onto this guy
	vector	artillery_lock_pos;				// base position of the lock point on (in model's frame of reference)
	float		lethality;							// measure of how dangerous ship is to enemy BIG|HUGE ships (likelyhood of targeting)
} ai_info;

#define	MAX_AI_INFO	 MAX_SHIPS

// SUBSYS_PATH_DIST is used as the distance that a subsystem path should terminate from the actual
// subsystem.  We don't want to rely on the model path points for this, since we may need to be 
// changed.
#define SUBSYS_PATH_DIST	500.0f

// Friendly damage defines
#define MAX_BURST_DAMAGE	20		// max damage that can be done in BURST_DURATION
#define BURST_DURATION		500	// decay time over which Player->damage_this_burst falls from MAX_BURST_DAMAGE to 0

extern int Mission_all_attack;	//	!0 means all teams attack all teams.
extern int Total_goal_ship_names;
extern char Goal_ship_names[MAX_GOAL_SHIP_NAMES][NAME_LENGTH];

extern void update_ai_info_for_hit(int hitter_obj, int hit_obj);
extern void ai_frame_all(void);

extern int find_guard_obj(void);

extern ai_info Ai_info[];
extern ai_info *Player_ai;

extern int Waypoints_created;	// externed since needed for save/restore

extern ai_class Ai_classes[];
extern char *Ai_class_names[];

extern int Num_ai_classes;
extern int Ai_firing_enabled;

extern char	*Skill_level_names(int skill_level, int translate = 1);
extern int	Skill_level_max_attackers[NUM_SKILL_LEVELS];
extern int Ai_goal_signature;

// need access to following data in AiBig.cpp
extern object	*Pl_objp;
extern object	*En_objp;
extern float	AI_frametime;


// Return index of free AI slot.
// Return 0 if no free slot.
int ai_get_slot(int shipnum);

// Releases an AI slot to be used by someone else.
void ai_free_slot(int ai_index);

// call to init one ai object.. you can pass all sorts of
// stuff by adding new paramters.
void ai_object_init(object * obj, int ai_index);

// Called once a frame
void ai_process( object * obj, int ai_index, float frametime );

int get_wingnum(int objnum);

void set_wingnum(int objnum, int wingnum);
char *ai_get_goal_ship_name(char *name, int *index);

extern waypoint_list Waypoint_lists[MAX_WAYPOINT_LISTS];
extern int	Num_waypoint_lists;

extern void init_ai_system(void);
extern void ai_attack_object(object *attacker, object *attacked, int priority, ship_subsys *ssp, int except = 0);
extern void ai_evade_object(object *evader, object *evaded, int priority);
extern void ai_ignore_object(object *ignorer, object *ignored, int priority);
extern void ai_ignore_wing(object *ignorer, int wingnum, int priority);
extern void ai_dock_with_object(object *docker, object *dockee, int priority, int dock_type, int docker_index, int dockee_index);
extern void ai_stay_still(object *still_objp, vector *view_pos);
extern void ai_set_default_behavior(object *obj, int classnum);
extern void ai_do_default_behavior(object *obj);
extern void ai_start_waypoints(object *objp, int waypoint_list_index, int wp_flags);
extern void ai_ship_hit(object *objp_ship, object *hit_objp, vector *hitpos, int shield_quadrant, vector *hit_normal);
extern void ai_ship_destroy(int shipnum, int method);
extern void ai_turn_towards_vector(vector *dest, object *objp, float frametime, float turn_time, vector *slide_vec, vector *rel_pos, float bank_override, int flags, vector *rvec = NULL);
extern void init_ai_object(int objnum);
extern void ai_init(void);				//	Call this one to parse ai.tbl.
extern void ai_level_init(void);		//	Call before each level to reset AI

extern int ai_set_attack_subsystem(object *objp, int subnum);
extern int ai_issue_rearm_request(object *requester_objp);		//	Object requests rearm/repair.
extern int ai_abort_rearm_request(object *requester_objp);		//	Object aborts rearm/repair.
extern void ai_do_repair_frame(object *objp, ai_info *aip, float frametime);		//	Repair a ship object, player or AI.
extern float dock_orient_and_approach(object *objp, object *dobjp, int dock_mode);	//	Move to a position relative to a dock bay using thrusters.
extern void ai_update_danger_weapon(int objnum, int weapon_objnum);

// called externally from MissionParse.cpp to position ships in wings upon arrival into the
// mission.
extern void get_absolute_wing_pos( vector *result_pos, object *leader_objp, int wing_index, int formation_object_flag);


//	Interface from goals code to AI.  Set ship to guard.  *objp guards *other_objp
extern void ai_set_guard_object(object *objp, object *other_objp);
extern void ai_set_evade_object(object *objp, object *other_objp);
extern void ai_set_guard_wing(object *objp, int wingnum);
extern void ai_warp_out(object *objp, vector *vp);
extern void ai_attack_wing(object *attacker, int wingnum, int priority);
extern void ai_deathroll_start(object *ship_obj);
extern void ai_fly_in_formation(int wing_num);		//	Force wing to fly in formation.
extern void ai_disband_formation(int wing_num);		//	Force wing to disband formation flying.
extern object *ai_find_docked_object( object *objp );	// returns object that objp is docked to
extern int set_target_objnum(ai_info *aip, int objnum);
extern void ai_form_on_wing(object *objp, object *goal_objp);
extern void ai_do_stay_near(object *objp, object *other_obj, float dist);
extern ship_subsys *set_targeted_subsys(ai_info *aip, ship_subsys *new_subsys, int parent_objnum);
extern void ai_rearm_repair( object *objp, object *goal_objp, int priority, int docker_index, int dockee_index );
extern void ai_add_rearm_goal( object *requester_objp, object *support_objp );
extern void create_model_path(object *pl_objp, object *mobjp, int path_num, int subsys_path=0);

// Goober5000
extern void ai_do_safety(object *objp);

// used to get path info for fighter bay emerging and departing
int ai_acquire_emerge_path(object *pl_objp, int parent_objnum, vector *pos, vector *fvec);
int ai_acquire_depart_path(object *pl_objp, int parent_objnum);

// used by AiBig.cpp
extern void ai_set_positions(object *pl_objp, object *en_objp, ai_info *aip, vector *player_pos, vector *enemy_pos);
extern void accelerate_ship(ai_info *aip, float accel);
extern void turn_away_from_point(object *objp, vector *point, float bank_override);
extern float ai_endangered_by_weapon(ai_info *aip);
extern void update_aspect_lock_information(ai_info *aip, vector *vec_to_enemy, float dist_to_enemy, float enemy_radius);
extern void ai_chase_ct();
extern void ai_find_path(object *pl_objp, int objnum, int path_num, int exit_flag, int subsys_path=0);
extern float ai_path();
extern void evade_weapon();
extern int might_collide_with_ship(object *obj1, object *obj2, float dot_to_enemy, float dist_to_enemy, float duration);
extern int ai_fire_primary_weapon(object *objp);	//changed to return weather it fired-Bobboau
extern int ai_fire_secondary_weapon(object *objp, int priority1 = -1, int priority2 = -1);
extern float ai_get_weapon_dist(ship_weapon *swp);
extern void turn_towards_point(object *objp, vector *point, vector *slide_vec, float bank_override);
extern int ai_maybe_fire_afterburner(object *objp, ai_info *aip);
extern void set_predicted_enemy_pos(vector *predicted_enemy_pos, object *pobjp, object *eobjp, ai_info *aip);

extern int is_instructor(object *objp);
extern int find_enemy(int objnum, float range, int max_attackers, int except = 0);

float ai_get_weapon_speed(ship_weapon *swp);
void set_predicted_enemy_pos_turret(vector *predicted_enemy_pos, vector *gun_pos, object *pobjp, vector *enemy_pos, vector *enemy_vel, float weapon_speed, float time_enemy_in_range);
void ai_turret_select_default_weapon(ship_subsys *turret);

// function to change rearm status for ai ships (called from sexpression code)
extern void ai_set_rearm_status( int team, int new_status );
extern void ai_good_secondary_time( int team, int weapon_index, int num_weapons, char *shipname );

extern void ai_do_objects_docked_stuff( object *docker, object *dockee );
extern void ai_do_objects_undocked_stuff( object *docker, object *dockee );
extern void ai_do_objects_repairing_stuff( object *repaired_obj, object *repair_obj, int how );

extern int find_danger_weapon(object *sobjp, float dtime, float *atime, float dot_threshhold);

void ai_set_mode_warp_out(object *objp, ai_info *aip);

// prototyped by Goober5000
int get_nearest_objnum(int objnum, int enemy_team_mask, int enemy_wing, float range, int max_attackers, int except = 0);

#endif
