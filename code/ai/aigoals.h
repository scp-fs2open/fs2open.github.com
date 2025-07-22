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

#include "ai/ai_flags.h"
#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "parse/sexp.h"
#include "scripting/lua/LuaTypes.h"
#include "scripting/lua/LuaValue.h"

struct wing;
struct ai_info;

// macros for goals which get set via sexpressions in the mission code

// types of ai goals -- these will help us to determine which goals should
// have priority over others (i.e. when a player issues a goal to a wing, then a seperate
// goal to a ship in that wing).  We would probably use this type in conjunction with
// goal priority to establish which goal to follow
enum class ai_goal_type
{
	INVALID = 0,
	EVENT_SHIP,		// from mission event direct to ship
	EVENT_WING,		// from mission event direct to wing
	PLAYER_SHIP,	// from player direct to ship
	PLAYER_WING,	// from player direct to wing
	DYNAMIC			// created on the fly
};

// IMPORTANT!  If you add a new AI_GOAL_x enum, be sure to update the functions
// ai_update_goal_references() and query_referenced_in_ai_goals() or else risk breaking
// Fred.  If the goal you add doesn't have a target (such as chase_any), then you don't have
// to worry about doing this.  Also add it to list in Fred\Management.cpp, and let Hoffoss know!
// WMC - Oh and add them to Ai_goal_names plz. TY! :)
// Goober5000 - As well as Ai_goal_text and Ai_goal_list, if appropriate
enum ai_goal_mode : uint8_t
{
	AI_GOAL_NONE = 0,
	AI_GOAL_SCHROEDINGER,		// used for FRED when multiple ships are selected with different orders

	AI_GOAL_CHASE,              // per the original #define list, AI_GOAL_CHASE started at 2 (1<<1)
	AI_GOAL_DOCK,               // used for undocking as well
	AI_GOAL_WAYPOINTS,
	AI_GOAL_WAYPOINTS_ONCE,
	AI_GOAL_WARP,
	AI_GOAL_DESTROY_SUBSYSTEM,
	AI_GOAL_FORM_ON_WING,
	AI_GOAL_UNDOCK,
	AI_GOAL_CHASE_WING,
	AI_GOAL_GUARD,
	AI_GOAL_DISABLE_SHIP,
	AI_GOAL_DISARM_SHIP,
	AI_GOAL_CHASE_ANY,
	AI_GOAL_IGNORE,
	AI_GOAL_GUARD_WING,
	AI_GOAL_EVADE_SHIP,

	// the next goals are for support ships only
	AI_GOAL_STAY_NEAR_SHIP,
	AI_GOAL_KEEP_SAFE_DISTANCE,
	AI_GOAL_REARM_REPAIR,

	// resume regular goals
	AI_GOAL_STAY_STILL,
	AI_GOAL_PLAY_DEAD,

	// added by SCP
	AI_GOAL_CHASE_WEAPON,
	AI_GOAL_FLY_TO_SHIP,
	AI_GOAL_IGNORE_NEW,
	AI_GOAL_CHASE_SHIP_CLASS,
	AI_GOAL_PLAY_DEAD_PERSISTENT,
	AI_GOAL_LUA,
	AI_GOAL_DISARM_SHIP_TACTICAL,
	AI_GOAL_DISABLE_SHIP_TACTICAL,

	AI_GOAL_NUM_VALUES
};

inline ai_goal_mode int_to_ai_goal_mode(int int_mode)
{
	if (int_mode >= 0 && int_mode < AI_GOAL_NUM_VALUES)
		return static_cast<ai_goal_mode>(int_mode);

	Warning(LOCATION, "ai_goal_mode %d out of range!  Setting to AI_GOAL_NONE.", int_mode);
	return AI_GOAL_NONE;
}

inline bool ai_goal_is_disable_or_disarm(ai_goal_mode ai_mode)
{
	return ai_mode == AI_GOAL_DISABLE_SHIP || ai_mode == AI_GOAL_DISABLE_SHIP_TACTICAL || ai_mode == AI_GOAL_DISARM_SHIP || ai_mode == AI_GOAL_DISARM_SHIP_TACTICAL;
}
inline bool ai_goal_is_specific_chase(ai_goal_mode ai_mode)
{
	return ai_mode == AI_GOAL_CHASE || ai_mode == AI_GOAL_CHASE_WING || ai_mode == AI_GOAL_CHASE_SHIP_CLASS;
}

enum class ai_achievability { ACHIEVABLE, NOT_ACHIEVABLE, NOT_KNOWN, SATISFIED };

struct ai_lua_parameters {
	object_ship_wing_point_team target;
	luacpp::LuaValueList arguments;
};

// structure for AI goals
typedef struct ai_goal {
	int	signature;			//	Unique identifier.  All goals ever created (per mission) have a unique signature.
	ai_goal_mode ai_mode;	// one of the AI_GOAL_* modes for this goal
	int	ai_submode;			// maybe need a submode
	ai_goal_type type;		// one of the ai_goal_type (originally AIG_TYPE_*) values above
	flagset<AI::Goal_Flags>	flags;				// one of the AIGF_* values above
	fix	time;					// time at which this goal was issued.
	int	priority;			// how important is this goal -- number 0 - 100

	const char *target_name;	// name of the thing that this goal acts upon
	int		target_name_index;	// index of goal_target_name in Goal_target_names[][]
	int wp_list_index;			// waypoints that this ship might fly.
	int target_instance;		// instance of thing this ship might be chasing (currently only used for weapons; note, not the same as objnum!)
	int	target_signature;		// signature of object this ship might be chasing (currently only used for weapons; paired with above value to confirm target)

	// extra goal-specific data
	int int_data;
	float float_data;

	// unions for docking stuff.
	// (AIGF_DOCKER_INDEX_VALID and AIGF_DOCKEE_INDEX_VALID tell us to use indexes; otherwise we use names)
	// these are the dockpoints used on the docker and dockee ships, not the ships themselves
	union {
		const char *name;
		int	index;
	} docker;
	
	union {
		const char *name;
		int	index;
	} dockee;

	ai_lua_parameters lua_ai_target;

} ai_goal;

extern void ai_goal_reset(ai_goal *aigp, bool adding_goal = false, ai_goal_mode ai_mode = AI_GOAL_NONE, int ai_submode = -1, ai_goal_type type = ai_goal_type::INVALID);

// Reset all path points. Used in the ship lab. Missions clean up path points with the garbage collector in garbage_collect_path_points()
extern void reset_ai_path_points();

typedef flag_def_list_templated<ai_goal_mode> ai_goal_list;

extern ai_goal_list Ai_goal_names[];
extern int Num_ai_goals;

#define MAX_AI_DOCK_NAMES				25

extern int Num_ai_dock_names;
extern char Ai_dock_names[MAX_AI_DOCK_NAMES][NAME_LENGTH];

extern const char *Ai_goal_text(ai_goal_mode goal, int submode);

// every goal in a mission gets a unique signature
extern int Ai_goal_signature;

// extern function definitions
extern void ai_post_process_mission();
extern void ai_maybe_add_form_goal( wing *wingp );
extern void ai_process_mission_orders( int objnum, ai_info *aip );

extern int ai_goal_num(ai_goal *goals);

// adds goals to ships/wing through sexpressions
extern void ai_add_ship_goal_scripting(ai_goal_mode mode, int submode, int priority, const char *shipname, ai_info *aip, int int_data, float float_data);
extern void ai_add_ship_goal_sexp(int sexp, ai_goal_type type, ai_info *aip);
extern void ai_add_wing_goal_sexp(int sexp, ai_goal_type type, wing *wingp);
extern void ai_add_goal_sub_sexp(int sexp, ai_goal_type type, ai_info *aip, ai_goal *aigp, const char *actor_name);

extern int ai_remove_goal_sexp_sub( int sexp, ai_goal* aigp, bool &remove_more );
extern void ai_remove_wing_goal_sexp( int sexp, wing *wingp );

// adds goals to ships/sings through player orders
extern void ai_add_ship_goal_player(ai_goal_type type, ai_goal_mode mode, int submode, const char* shipname, ai_info* aip, int int_data = 0, float float_data = 0.0f, const ai_lua_parameters& lua_target = { object_ship_wing_point_team(), luacpp::LuaValueList{} });
extern void ai_add_wing_goal_player(ai_goal_type type, ai_goal_mode mode, int submode, const char* shipname, int wingnum, int int_data = 0, float float_data = 0.0f, const ai_lua_parameters& lua_target = { object_ship_wing_point_team(), luacpp::LuaValueList{} });

extern void ai_remove_ship_goal( ai_info *aip, int index );
extern void ai_clear_ship_goals( ai_info *aip );
extern void ai_clear_wing_goals( wing *wingp );

extern void ai_copy_mission_wing_goal( ai_goal *aigp, ai_info *aip );

extern void ai_mission_goal_complete( ai_info *aip );
extern void ai_mission_wing_goal_complete( int wingnum, ai_goal *remove_goalp );

extern void ai_update_goal_references(ai_goal *goals, sexp_ref_type type, const char *old_name, const char *new_name);
extern bool query_referenced_in_ai_goals(ai_goal *goals, sexp_ref_type type, const char *name);
extern char *ai_add_dock_name(const char *str);

extern int ai_query_goal_valid( int ship, ai_goal_mode ai_mode );

extern void ai_add_goal_ship_internal( ai_info *aip, int goal_type, char *name, int docker_point, int dockee_point, int immediate = 1 );

#endif
