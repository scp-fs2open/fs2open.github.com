/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _AIGOALS_H
#define _AIGOALS_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

struct wing;
struct ai_info;
struct ai_goal;

// macros for goals which get set via sexpressions in the mission code

// IMPORTANT!  If you add a new AI_GOAL_x define, be sure to update the functions
// ai_update_goal_references() and query_referenced_in_ai_goals() or else risk breaking
// Fred.  If the goal you add doesn't have a target (such as chase_any), then you don't have
// to worry about doing this.  Also add it to list in Fred\Management.cpp, and let Hoffoss know!
// WMC - Oh and add them to Ai_goal_names plz. TY! :)
#define AI_GOAL_CHASE					(1<<1)	// 0x00000002
#define AI_GOAL_DOCK					(1<<2)	// 0x00000004	// used for undocking as well
#define AI_GOAL_WAYPOINTS				(1<<3)	// 0x00000008
#define AI_GOAL_WAYPOINTS_ONCE			(1<<4)	// 0x00000010
#define AI_GOAL_WARP					(1<<5)	// 0x00000020
#define AI_GOAL_DESTROY_SUBSYSTEM		(1<<6)	// 0x00000040
#define AI_GOAL_FORM_ON_WING			(1<<7)	// 0x00000080
#define AI_GOAL_UNDOCK					(1<<8)	// 0x00000100
#define AI_GOAL_CHASE_WING				(1<<9)	// 0x00000200
#define AI_GOAL_GUARD					(1<<10)	// 0x00000400
#define AI_GOAL_DISABLE_SHIP			(1<<11)	// 0x00000800
#define AI_GOAL_DISARM_SHIP				(1<<12)	// 0x00001000
#define AI_GOAL_CHASE_ANY				(1<<13)	// 0x00002000
#define AI_GOAL_IGNORE					(1<<14)	// 0x00004000
#define AI_GOAL_GUARD_WING				(1<<15)	// 0x00008000
#define AI_GOAL_EVADE_SHIP				(1<<16)	// 0x00010000

// the next goals are for support ships only
#define AI_GOAL_STAY_NEAR_SHIP			(1<<17)	// 0x00020000
#define AI_GOAL_KEEP_SAFE_DISTANCE		(1<<18)	// 0x00040000
#define AI_GOAL_REARM_REPAIR			(1<<19)	// 0x00080000

// resume regular goals
#define AI_GOAL_STAY_STILL				(1<<20)	// 0x00100000
#define AI_GOAL_PLAY_DEAD				(1<<21)	// 0x00200000
#define AI_GOAL_CHASE_WEAPON			(1<<22)	// 0x00400000

#define AI_GOAL_FLY_TO_SHIP				(1<<23) // 0x00800000
#define AI_GOAL_IGNORE_NEW				(1<<24)	// 0x01000000

// now the masks for ship types

// Goober5000: added AI_GOAL_STAY_NEAR_SHIP and AI_GOAL_KEEP_SAFE_DISTANCE as valid for fighters
//WMC - Don't need these anymore. Whee!
/*
#define AI_GOAL_ACCEPT_FIGHTER		( AI_GOAL_FLY_TO_SHIP | AI_GOAL_CHASE | AI_GOAL_WAYPOINTS | AI_GOAL_WAYPOINTS_ONCE | AI_GOAL_WARP | AI_GOAL_DESTROY_SUBSYSTEM | AI_GOAL_CHASE_WING | AI_GOAL_GUARD | AI_GOAL_DISABLE_SHIP | AI_GOAL_DISARM_SHIP | AI_GOAL_CHASE_ANY | AI_GOAL_IGNORE | AI_GOAL_IGNORE_NEW | AI_GOAL_GUARD_WING | AI_GOAL_EVADE_SHIP | AI_GOAL_STAY_STILL | AI_GOAL_PLAY_DEAD | AI_GOAL_STAY_NEAR_SHIP | AI_GOAL_KEEP_SAFE_DISTANCE )
#define AI_GOAL_ACCEPT_BOMBER			( AI_GOAL_FLY_TO_SHIP | AI_GOAL_ACCEPT_FIGHTER | AI_GOAL_STAY_NEAR_SHIP )
#define AI_GOAL_ACCEPT_STEALTH		( AI_GOAL_FLY_TO_SHIP | AI_GOAL_ACCEPT_FIGHTER | AI_GOAL_STAY_NEAR_SHIP )
#define AI_GOAL_ACCEPT_TRANSPORT		( AI_GOAL_FLY_TO_SHIP | AI_GOAL_CHASE | AI_GOAL_CHASE_WING | AI_GOAL_DOCK | AI_GOAL_WAYPOINTS | AI_GOAL_WAYPOINTS_ONCE | AI_GOAL_WARP | AI_GOAL_UNDOCK | AI_GOAL_STAY_STILL | AI_GOAL_PLAY_DEAD| AI_GOAL_STAY_NEAR_SHIP )
#define AI_GOAL_ACCEPT_FREIGHTER		( AI_GOAL_FLY_TO_SHIP | AI_GOAL_ACCEPT_TRANSPORT | AI_GOAL_STAY_NEAR_SHIP )
#define AI_GOAL_ACCEPT_CRUISER		( AI_GOAL_FLY_TO_SHIP | AI_GOAL_ACCEPT_FREIGHTER | AI_GOAL_STAY_NEAR_SHIP )
#define AI_GOAL_ACCEPT_CORVETTE		( AI_GOAL_FLY_TO_SHIP | AI_GOAL_ACCEPT_CRUISER | AI_GOAL_STAY_NEAR_SHIP )
#define AI_GOAL_ACCEPT_GAS_MINER		( AI_GOAL_FLY_TO_SHIP | AI_GOAL_ACCEPT_CRUISER | AI_GOAL_STAY_NEAR_SHIP )
#define AI_GOAL_ACCEPT_AWACS			( AI_GOAL_FLY_TO_SHIP | AI_GOAL_ACCEPT_CRUISER | AI_GOAL_STAY_NEAR_SHIP )
#define AI_GOAL_ACCEPT_CAPITAL		( AI_GOAL_FLY_TO_SHIP | AI_GOAL_ACCEPT_CRUISER & ~(AI_GOAL_DOCK | AI_GOAL_UNDOCK) | AI_GOAL_STAY_NEAR_SHIP )
#define AI_GOAL_ACCEPT_SUPERCAP		( AI_GOAL_FLY_TO_SHIP | AI_GOAL_ACCEPT_CAPITAL | AI_GOAL_STAY_NEAR_SHIP )
#define AI_GOAL_ACCEPT_SUPPORT		( AI_GOAL_FLY_TO_SHIP | AI_GOAL_DOCK | AI_GOAL_UNDOCK | AI_GOAL_WAYPOINTS | AI_GOAL_WAYPOINTS_ONCE | AI_GOAL_STAY_NEAR_SHIP | AI_GOAL_KEEP_SAFE_DISTANCE | AI_GOAL_STAY_STILL | AI_GOAL_PLAY_DEAD)
#define AI_GOAL_ACCEPT_ESCAPEPOD		( AI_GOAL_FLY_TO_SHIP | AI_GOAL_ACCEPT_TRANSPORT| AI_GOAL_STAY_NEAR_SHIP  )
*/

#define MAX_AI_DOCK_NAMES				25

typedef flag_def_list ai_goal_list;

extern ai_goal_list Ai_goal_names[];
extern int Num_ai_goals;

extern int Num_ai_dock_names;
extern char Ai_dock_names[MAX_AI_DOCK_NAMES][NAME_LENGTH];

extern char *Ai_goal_text(int goal);

// extern function definitions
extern void ai_post_process_mission();
extern void ai_maybe_add_form_goal( wing *wingp );
extern void ai_process_mission_orders( int objnum, ai_info *aip );

extern int ai_goal_num(ai_goal *goals);

// adds goals to ships/wing through sexpressions
extern void ai_add_ship_goal_scripting(int mode, int submode, int priority, char *shipname, ai_info *aip);
extern void ai_add_ship_goal_sexp( int sexp, int type, ai_info *aip );
extern void ai_add_wing_goal_sexp( int sexp, int type, int wingnum );
extern void ai_add_goal_sub_sexp( int sexp, int type, ai_goal *aigp );

extern int ai_remove_goal_sexp_sub( int sexp, ai_goal* aigp );
extern void ai_remove_wing_goal_sexp(int sexp, int wingnum);

// adds goals to ships/sings through player orders
extern void ai_add_ship_goal_player( int type, int mode, int submode, char *shipname, ai_info *aip );
extern void ai_add_wing_goal_player( int type, int mode, int submode, char *shipname, int wingnum );

extern void ai_remove_ship_goal( ai_info *aip, int index );
extern void ai_clear_ship_goals( ai_info *aip );
extern void ai_clear_wing_goals( int wingnum );

extern void ai_copy_mission_wing_goal( ai_goal *aigp, ai_info *aip );

extern void ai_mission_goal_complete( ai_info *aip );
extern void ai_mission_wing_goal_complete( int wingnum, ai_goal *remove_goalp );

extern void ai_update_goal_references(ai_goal *goals, int type, const char *old_name, const char *new_name);
extern int query_referenced_in_ai_goals(ai_goal *goals, int type, char *name);
extern char *ai_add_dock_name(char *str);

extern int ai_query_goal_valid( int ship, int ai_goal );

extern void ai_add_goal_ship_internal( ai_info *aip, int goal_type, char *name, int docker_point, int dockee_point, int immediate = 1 );
extern void ai_add_goal_wing_internal( wing *wingp, int goal_type, char *name, int immediate = 1 );

#endif
