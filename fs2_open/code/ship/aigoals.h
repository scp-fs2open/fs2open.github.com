/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/AiGoals.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:02 $
 * $Author: penguin $
 *
 * <insert description of file here>
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     7/09/99 5:54p Dave
 * Seperated cruiser types into individual types. Added tons of new
 * briefing icons. Campaign screen.
 * 
 * 4     3/26/99 4:49p Dave
 * Made cruisers able to dock with stuff. Made docking points and paths
 * visible in fred.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 46    2/26/98 10:08p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 45    2/22/98 4:17p John
 * More string externalization classification... 190 left to go!
 * 
 * 44    2/16/98 10:13p Allender
 * initial work on being able to target weapons (bombs specifically).
 * Work on getting rearm/repair working under multiplayer
 * 
 * 43    2/13/98 3:21p Lawrance
 * Set up allowed goals for escape pods.
 * 
 * 42    1/30/98 10:01a Allender
 * made large ships able to attack other ships.  Made goal code recognize
 * when ships removed from wings during ship select
 * 
 * 41    12/15/97 5:25p Allender
 * fix problem with docked ships receiving another dock goal.  They now
 * properly undock
 * 
 * 40    11/17/97 6:39p Lawrance
 * add AI_goal_text[] array, used by HUD code to show text description of
 * order
 * 
 * 39    10/29/97 10:02p Allender
 * be sure that player starting wings that arrive late also form on the
 * players wing if there are no goals
 * 
 * 38    10/23/97 4:41p Allender
 * lots of new rearm/repair code.  Rearm requests now queue as goals for
 * support ship.  Warp in of new support ships functional.  Support for
 * stay-still and play-dead.  
 * 
 * 37    10/22/97 1:44p Allender
 * more work on stay-still and play-dead
 * 
 * 36    10/22/97 11:07a Allender
 * Hooked in form on my wing into Mikes AI code
 * 
 * 35    10/22/97 10:33a Hoffoss
 * Added AI_GOAL_PLAY_DEAD as a valid goal for all ships.
 * 
 * 34    10/22/97 9:54a Allender
 * added ai_goal_play_dead
 * 
 * 33    10/10/97 5:03p Allender
 * started work on ai-stay-still
 * 
 * 32    9/23/97 4:35p Allender
 * added two function to add "internal" goals to ships and wings.  Used by
 * AI when it needs to do something special
 * 
 * 31    8/15/97 11:07a Hoffoss
 * Added warning to help eliminate problems later on down the road.
 * 
 * 30    8/15/97 9:59a Hoffoss
 * Changed ai_query_goal_valid() again.  I can't think of any time you
 * wouldn't find it easier just to pass in a ship instead of a ship_info
 * flag structure filtered of all flags that aren't ship type bits.  Since
 * I'll call this quite a few times, it will make my life easier.
 * 
 * 29    8/14/97 10:00p Allender
 * made ai goals bitfields so that we can have set of orders that certain
 * ship types are allowed to receive.  Added function for Hoffoss to check
 * goal vs. ship type validity
 * 
 * 28    8/12/97 1:55a Hoffoss
 * Made extensive changes to object reference checking and handling for
 * object deletion call.
 * 
 * 27    8/08/97 1:30p Allender
 * added commands (and ai goals) for support ships.  stay near ship (which
 * could be player or current target), and keep safe distance
 * 
 * 26    8/06/97 9:41a Allender
 * renamed from ai_goal function.  made a function to remove a goal from a
 * wing (used strictly for waypoints now).  ai-chase-wing and
 * ai-guard-wing now appear to user as ai-chase and ai-guard.  Tidied up
 * function which deals with sexpression goals
 * 
 * 25    8/06/97 8:06a Lawrance
 * add more stuff to save/restore
 * 
 * 24    7/24/97 4:55p Allender
 * added ai-evade-ship to fred and to Freespace
 * 
 * 23    7/17/97 4:23p Allender
 * ai-guard-wing now in both Fred and Freespace
 * 
 * 22    7/15/97 11:57a Hoffoss
 * Added a general ai goal reference update mode.
 * 
 * 21    6/18/97 11:46a Hoffoss
 * Fixed initial order object reference updating and added briefing dialog
 * window tracking data.
 *
 * $NoKeywords: $
 */

#ifndef _AIGOALS_H
#define _AIGOALS_H

#include "ai.h"
#include "cfile.h"

// macros for goals which get set via sexpressions in the mission code

// IMPORTANT!  If you add a new AI_GOAL_x define, be sure to update the functions
// ai_update_goal_references() and query_referenced_in_ai_goals() or else risk breaking
// Fred.  If the goal you add doesn't have a target (such as chase_any), then you don't have
// to worry about doing this.  Also add it to list in Fred\Management.cpp, and let Hoffoss know!
#define AI_GOAL_CHASE					(1<<1)
#define AI_GOAL_DOCK						(1<<2)		// used for undocking as well
#define AI_GOAL_WAYPOINTS				(1<<3)
#define AI_GOAL_WAYPOINTS_ONCE		(1<<4)
#define AI_GOAL_WARP						(1<<5)
#define AI_GOAL_DESTROY_SUBSYSTEM	(1<<6)
#define AI_GOAL_FORM_ON_WING			(1<<7)
#define AI_GOAL_UNDOCK					(1<<8)
#define AI_GOAL_CHASE_WING				(1<<9)
#define AI_GOAL_GUARD					(1<<10)
#define AI_GOAL_DISABLE_SHIP			(1<<11)
#define AI_GOAL_DISARM_SHIP			(1<<12)
#define AI_GOAL_CHASE_ANY				(1<<13)
#define AI_GOAL_IGNORE					(1<<14)
#define AI_GOAL_GUARD_WING				(1<<15)
#define AI_GOAL_EVADE_SHIP				(1<<16)

// the next goals are for support ships only
#define AI_GOAL_STAY_NEAR_SHIP		(1<<17)
#define AI_GOAL_KEEP_SAFE_DISTANCE	(1<<18)
#define AI_GOAL_REARM_REPAIR			(1<<19)

#define AI_GOAL_STAY_STILL				(1<<20)
#define AI_GOAL_PLAY_DEAD				(1<<21)
#define AI_GOAL_CHASE_WEAPON			(1<<22)

// now the masks for ship types

#define AI_GOAL_ACCEPT_FIGHTER		( AI_GOAL_CHASE | AI_GOAL_WAYPOINTS | AI_GOAL_WAYPOINTS_ONCE | AI_GOAL_WARP | AI_GOAL_DESTROY_SUBSYSTEM | AI_GOAL_CHASE_WING | AI_GOAL_GUARD | AI_GOAL_DISABLE_SHIP | AI_GOAL_DISARM_SHIP | AI_GOAL_CHASE_ANY | AI_GOAL_IGNORE | AI_GOAL_GUARD_WING | AI_GOAL_EVADE_SHIP | AI_GOAL_STAY_STILL | AI_GOAL_PLAY_DEAD)
#define AI_GOAL_ACCEPT_BOMBER			( AI_GOAL_ACCEPT_FIGHTER )
#define AI_GOAL_ACCEPT_STEALTH		( AI_GOAL_ACCEPT_FIGHTER )
#define AI_GOAL_ACCEPT_TRANSPORT		( AI_GOAL_CHASE | AI_GOAL_CHASE_WING | AI_GOAL_DOCK | AI_GOAL_WAYPOINTS | AI_GOAL_WAYPOINTS_ONCE | AI_GOAL_WARP | AI_GOAL_UNDOCK | AI_GOAL_STAY_STILL | AI_GOAL_PLAY_DEAD)
#define AI_GOAL_ACCEPT_FREIGHTER		( AI_GOAL_ACCEPT_TRANSPORT )
#define AI_GOAL_ACCEPT_CRUISER		( AI_GOAL_ACCEPT_FREIGHTER )
#define AI_GOAL_ACCEPT_CORVETTE		( AI_GOAL_ACCEPT_CRUISER )
#define AI_GOAL_ACCEPT_GAS_MINER		( AI_GOAL_ACCEPT_CRUISER )
#define AI_GOAL_ACCEPT_AWACS			( AI_GOAL_ACCEPT_CRUISER )
#define AI_GOAL_ACCEPT_CAPITAL		( AI_GOAL_ACCEPT_CRUISER & ~(AI_GOAL_DOCK | AI_GOAL_UNDOCK) )
#define AI_GOAL_ACCEPT_SUPERCAP		( AI_GOAL_ACCEPT_CAPITAL )
#define AI_GOAL_ACCEPT_SUPPORT		( AI_GOAL_DOCK | AI_GOAL_UNDOCK | AI_GOAL_WAYPOINTS | AI_GOAL_WAYPOINTS_ONCE | AI_GOAL_STAY_NEAR_SHIP | AI_GOAL_KEEP_SAFE_DISTANCE | AI_GOAL_STAY_STILL | AI_GOAL_PLAY_DEAD)
#define AI_GOAL_ACCEPT_ESCAPEPOD		( AI_GOAL_ACCEPT_TRANSPORT )

#define MAX_AI_DOCK_NAMES				25

extern int Num_ai_dock_names;
extern char Ai_dock_names[MAX_AI_DOCK_NAMES][NAME_LENGTH];

extern char *Ai_goal_text(int goal);

// extern function definitions
extern void ai_post_process_mission();
extern void ai_maybe_add_form_goal( wing *wingp );
extern void ai_process_mission_orders( int objnum, ai_info *aip );

// adds goals to ships/wing through sexpressions
extern void ai_add_ship_goal_sexp( int sexp, int type, ai_info *aip );
extern void ai_add_wing_goal_sexp( int sexp, int type, int wingnum );
extern void ai_add_goal_sub_sexp( int sexp, int type, ai_goal *aigp );

// adds goals to ships/sings through player orders
extern void ai_add_ship_goal_player( int type, int mode, int submode, char *shipname, ai_info *aip );
extern void ai_add_wing_goal_player( int type, int mode, int submode, char *shipname, int wingnum );

extern void ai_remove_ship_goal( ai_info *aip, int index );
extern void ai_clear_ship_goals( ai_info *aip );
extern void ai_clear_wing_goals( int wingnum );

extern void ai_copy_mission_wing_goal( ai_goal *aigp, ai_info *aip );

extern void ai_mission_goal_complete( ai_info *aip );
extern void ai_mission_wing_goal_complete( int wingnum, ai_goal *remove_goalp );

extern int ai_get_subsystem_type( char *subsystem );
extern char *ai_get_subsystem_type_name(int type);
extern void ai_update_goal_references(ai_goal *goals, int type, char *old_name, char *new_name);
extern int query_referenced_in_ai_goals(ai_goal *goals, int type, char *name);
extern char *ai_add_dock_name(char *str);

extern int ai_query_goal_valid( int ship, int ai_goal );

extern void ai_add_goal_ship_internal( ai_info *aip, int goal_type, char *name, int docker_point, int dockee_point, int immediate = 1 );
extern void ai_add_goal_wing_internal( wing *wingp, int goal_type, char *name, int immediate = 1 );

#endif
