/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/AiGoals.cpp $
 * $Revision: 2.21 $
 * $Date: 2005-01-18 00:14:36 $
 * $Author: Goober5000 $
 *
 * File to deal with manipulating AI goals, etc.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.20  2005/01/13 03:33:07  Goober5000
 * hmm, rolled back one of my clarifications since it caused things to get messed up
 * --Goober5000
 *
 * Revision 2.19  2005/01/13 01:10:33  Goober5000
 * forgot to remove a comment
 * --Goober5000
 *
 * Revision 2.18  2005/01/12 23:38:42  Goober5000
 * fixed another nasty bug
 * --Goober5000
 *
 * Revision 2.17  2005/01/11 21:38:48  Goober5000
 * multiple ship docking :)
 * don't tell anyone yet... check the SCP internal
 * --Goober500
 *
 * Revision 2.16  2004/12/14 14:46:12  Goober5000
 * allow different wing names than ABGDEZ
 * --Goober5000
 *
 * Revision 2.15  2004/07/26 20:47:51  Kazan
 * remove MCD complete
 *
 * Revision 2.14  2004/07/25 18:46:30  Kazan
 * -fred_no_warn has become -no_warn and applies to both fred and fs2
 * added new ai directive (last commit) and disabled afterburners while performing AIM_WAYPOINTS or AIM_FLY_TO_SHIP
 * fixed player ship speed bug w/ player-use-ai, now stays in formation correctly and manages speed
 * made -radar_reduce ignore itself if no parameter is given (ignoring launcher bug)
 *
 * Revision 2.13  2004/07/25 00:31:31  Kazan
 * i have absolutely nothing to say about that subject
 *
 * Revision 2.12  2004/07/12 16:33:05  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.11  2004/03/05 09:01:52  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.10  2004/02/04 09:21:36  Goober5000
 * more player ai stuff
 * --Goober5000
 *
 * Revision 2.9  2004/01/30 07:39:06  Goober5000
 * whew - I just went through all the code I ever added (or at least, that I could
 * find that I commented with a Goober5000 tag) and added a bunch of Asserts
 * and error-checking
 * --Goober5000
 *
 * Revision 2.8  2003/11/11 02:15:40  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.7  2003/09/05 04:25:27  Goober5000
 * well, let's see here...
 *
 * * persistent variables
 * * rotating gun barrels
 * * positive/negative numbers fixed
 * * sexps to trigger whether the player is controlled by AI
 * * sexp for force a subspace jump
 *
 * I think that's it :)
 * --Goober5000
 *
 * Revision 2.6  2003/01/19 22:20:22  Goober5000
 * fixed a bunch of bugs -- the support ship sexp, the "no-subspace-drive" flag,
 * and departure into hangars should now all work properly
 * --Goober5000
 *
 * Revision 2.5  2003/01/18 23:25:38  Goober5000
 * made "no-subspace-drive" applicable to all ships and fixed a really *STUPID*
 * bug that made FRED keep crashing (missing comma, bleagh!)
 * --Goober5000
 *
 * Revision 2.4  2003/01/18 09:25:40  Goober5000
 * fixed bug I inadvertently introduced by modifying SIF_ flags with sexps rather
 * than SF_ flags
 * --Goober5000
 *
 * Revision 2.3  2003/01/07 20:06:44  Goober5000
 * added ai-chase-any-except sexp
 * --Goober5000
 *
 * Revision 2.2  2003/01/06 22:57:23  Goober5000
 * implemented keep-safe-distance
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/13 21:09:29  mharris
 * I think the last of the networking code has ifndef NO_NETWORK...
 *
 * Revision 1.2  2002/05/07 03:00:17  mharris
 * make Goal_text static
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 12    7/19/99 12:02p Andsager
 * Allow AWACS on any ship subsystem. Fix sexp_set_subsystem_strength to
 * only blow up subsystem if its strength is > 0
 * 
 * 11    7/09/99 5:54p Dave
 * Seperated cruiser types into individual types. Added tons of new
 * briefing icons. Campaign screen.
 * 
 * 10    7/07/99 2:55p Andsager
 * fix ai_get_subsystem_type to recognize "radara" as an awacs type
 * subsystem as model_read.
 * 
 * 9     6/03/99 8:58p Andsager
 * Fix bug where player's target is lost.  Player wing had clear wing
 * goals, and this was applied to player.
 * 
 * 8     5/27/99 12:14p Andsager
 * Some fixes for live debris when more than one subsys on ship with live
 * debris.  Set subsys strength (when 0) blows off subsystem.
 * sexp_hits_left_subsystem works for SUBSYSTEM_UNKNOWN.
 * 
 * 7     4/23/99 12:01p Johnson
 * Added SIF_HUGE_SHIP
 * 
 * 6     3/26/99 4:49p Dave
 * Made cruisers able to dock with stuff. Made docking points and paths
 * visible in fred.
 * 
 * 5     1/07/99 1:52p Andsager
 * Initial check in of Sexp_variables
 * 
 * 4     12/23/98 2:53p Andsager
 * Added ship activation and gas collection subsystems, removed bridge
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 173   6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 172   6/01/98 11:43a John
 * JAS & MK:  Classified all strings for localization.
 * 
 * 171   5/21/98 9:57a Mike
 * Massively improve firing of bombs at big ships.  Improve willingness to
 * take incoming fire to deliver bomb.  Make a little easier to gain
 * aspect lock.
 * 
 * 170   5/12/98 10:06a Mike
 * Fix a jillion bugs preventing ships from firing bombs at Lucifer in
 * sm3-09.
 * 
 * 169   5/11/98 11:20a Sandeep
 * 
 * 168   5/10/98 12:02a Mike
 * Only player's wing will form on player's wing, not any wing in player's squadron.
 * 
 * 167   4/23/98 1:49a Allender
 * major rearm/repair fixes for multiplayer.  Fixed respawning of AI ships
 * to not respawn until 5 seconds after they die.  Send escort information
 * to ingame joiners
 * 
 * 166   4/23/98 12:31a Mike
 * Fix the sm1-04a Comet:Omega bug.  Was putting a goal on hold if a ship
 * was docked, but should only have done that if not docked with self.
 * 
 * 165   4/22/98 5:00p Allender
 * new multiplayer dead popup.  big changes to the comm menu system for  *
 * team vs. team.  Start of debriefing stuff for team vs. team  Make form
 * on my wing work with individual ships who have high priority 
 * orders
 * 
 * 164   4/18/98 9:53p Mike
 * Add debug code to track down Comet warpout problems in sm1-04a, then
 * comment it out for checkin.
 * 
 * 163   4/09/98 12:36p Allender
 * nasty repair problems fixed when ordering it to warp out while
 * repairing (and on way) and when aborting process while he's completing
 * the rearm
 * 
 * 162   3/31/98 5:19p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 161   3/19/98 4:43p Allender
 * allow player ships to follow certain orders
 * 
 * 160   3/19/98 2:55p Allender
 * when ship gets disabled, have him put dock goals on hold
 * 
 * 159   3/09/98 9:35p Mike
 * Suppress warning messages.
 * 
 * 158   3/09/98 5:12p Mike
 * Make sure Pl_objp uses are valid.
 * Throw asteroids at ships in asteroid field, not "Gal"
 * Support ships warp out if no goals after 30 seconds.
 * 
 * 157   3/07/98 3:50p Lawrance
 * Add lead indicator for asteroids
 * 
 * 156   3/06/98 2:58p Allender
 * moved assert to more appropriate place
 * 
 * 155   3/06/98 11:55a Lawrance
 * Don't process mission orders for player ships
 * 
 * 154   3/05/98 9:35a Allender
 * don't allow ships to be repaired when departing or dying
 * 
 * 153   2/26/98 10:08p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 152   2/23/98 8:59p Allender
 * fixed two docking bugs:  1) don't move cargo when ship it's docked with
 * is undocking.  2) aigoal code was clearing dock goals when it shouldn't
 * have been
 * 
 * 151   2/22/98 4:17p John
 * More string externalization classification... 190 left to go!
 * 
 * 150   2/19/98 12:04a Allender
 * more multiplayer rearm/dock stuff.  Fix bug in HUD code when finding
 * support ship reaming player.  show repair as order on Hud extended
 * target info
 * 
 * 149   2/18/98 10:34p Allender
 * repair/rearm system (for single and multi) about finished.
 * dock/undock and ai goals packets implemented for multiplayer
 * 
 * 148   2/16/98 10:13p Allender
 * initial work on being able to target weapons (bombs specifically).
 * Work on getting rearm/repair working under multiplayer
 * 
 * 147   2/16/98 4:07p Sandeep
 * fixed a bug with deleting waypoints
 * 
 * 146   2/16/98 3:43p Sandeep
 * Fixed bug with Ai_warp still expected to have an object reference in a
 * bit of the code.
 * 
 * 145   2/13/98 3:21p Lawrance
 * Set up allowed goals for escape pods.
 * 
 * 144   2/13/98 2:57p Allender
 * correct code using unitialized variable
 * 
 * 143   2/09/98 10:46a Allender
 * fixed bug with docking where multiple orders to dock with same object
 * caused strageness
 * 
 * 142   2/05/98 12:52p Jim
 * fix race conditions to get to subsystem index for attacking subsystem
 * goals
 * 
 * 141   2/02/98 4:02p Mike
 * Allender: Fix bug which caused ships to sometimes undock just as they
 * were docking.
 * 
 * 140   1/30/98 11:00a Allender
 * tidied up the code to get an index for a new goal.  Made all code (for
 * both ships and wings) go through this function.
 * 
 * 139   1/30/98 10:01a Allender
 * made large ships able to attack other ships.  Made goal code recognize
 * when ships removed from wings during ship select
 * 
 * 138   1/29/98 12:14p Lawrance
 * show 'waypoints' as a mission order.
 * 
 * 137   1/20/98 9:33a Allender
 * fixed bug with wing goals from players
 * 
 * 136   1/16/98 1:11p Sandeep
 * fix bug when trying to purge goals with no ship_name
 * 
 * 135   1/16/98 11:43a Allender
 * made it so certain orders (see PURGE_ORDERS) cause other ai goals to
 * get purged.  This step it to help the AI do the right thing.
 * 
 * 134   1/16/98 11:33a Mike
 * Fix bug in targeting subsystems on protected ships.
 * 
 * 133   1/13/98 5:36p Lawrance
 * Change Ai_goal_text[] to not return anything for waypoints.
 * 
 * 132   1/05/98 10:10p Mike
 * Comment out an obsolete function (discovered while searching for
 * something else.)
 * 
 * 131   1/02/98 1:55p Duncan
 * added back in case for undocking when checking for goal complete status
 * 
 * 130   12/30/97 4:48p Allender
 * work with ignore my target command.  Added new keyboard hotkey.  Made
 * it work globally
 * 
 * 129   12/26/97 12:15p Mike
 * Put in debug code for tracking down ships disabling, rather than
 * destroying, enemies.
 * 
 * 128   12/15/97 5:25p Allender
 * fix problem with docked ships receiving another dock goal.  They now
 * properly undock
 * 
 * 127   12/12/97 5:21p Lawrance
 * fix some problems with ai mode changing when arriving/departing from
 * docking bays
 * 
 * 126   12/04/97 12:24p Allender
 * made support ship orders other than rearm be super low priority.
 * Support ship orders now won't get removed incorrectly either
 * 
 * 125   11/26/97 9:55a Allender
 * minor changed to comm window for rearming.  Fixed some repair bugs --
 * player's repair was getting aborted by code when he was hit.  Repair
 * ship wasn't properly removing repair goals
 * 
 * 124   11/23/97 6:21p Lawrance
 * update goal text, used on HUD
 * 
 * 123   11/17/97 6:39p Lawrance
 * add AI_goal_text[] array, used by HUD code to show text description of
 * order
 * 
 * 122   11/09/97 2:21p Mike
 * Comment out Int3() and email Mark about it.
 * 
 * 121   11/05/97 9:30p Mike
 * Add play dead mode.
 * Enable undock to complete when dockee moves.
 * Make ships in waypoint mode react to enemy fire.
 * Support ships not form on player's wing.
 * 
 * 120   11/05/97 4:43p Allender
 * reworked medal/rank system to read all data from tables.  Made Fred
 * read medals.tbl.  Changed ai-warp to ai-warp-out which doesn't require
 * waypoint for activation
 * 
 * 119   10/31/97 4:03p Allender
 * test code to help find rearm/repair problems
 * 
 * 118   10/29/97 10:02p Allender
 * be sure that player starting wings that arrive late also form on the
 * players wing if there are no goals
 * 
 * 117   10/29/97 9:32p Allender
 * remove undock goal if we can't find docked sihp when docker trying to
 * undock.  Could be that docked ship was destroyed
 * 
 * 116   10/29/97 3:41p Sandeep
 * Allender put in dock debug code
 * 
 * 115   10/28/97 10:30a Allender
 * rest AIF_FORMATION_OBJECT bit when new order is given to a ship
 * 
 * 114   10/24/97 10:40a Allender
 * player wings with no initial orders now form on players wing by
 * default.
 * 
 * 113   10/23/97 4:41p Allender
 * lots of new rearm/repair code.  Rearm requests now queue as goals for
 * support ship.  Warp in of new support ships functional.  Support for
 * stay-still and play-dead.  
 * 
 * 112   10/23/97 4:12p Mike
 * Support for AIM_STAY_NEAR.
 * 
 * 111   10/22/97 1:46p Allender
 * get ship_name field for ai-stay-still
 * 
 * 110   10/22/97 1:44p Allender
 * more work on stay-still and play-dead
 * 
 * 109   10/22/97 11:07a Allender
 * Hooked in form on my wing into Mikes AI code
 * 
 * 108   10/22/97 1:03a Mike
 * ai-stay-still
 * form-on-my-wing
 * fix mysterious code in find_enemy that used goal_objnum instead of
 * target_objnum.
 * 
 * 107   10/12/97 11:23p Mike
 * About ten fixes/changes in the docking system.
 * Also, renamed SIF_REARM_REPAIR to SIF_SUPPORT.
 * 
 * 106   10/10/97 5:03p Allender
 * started work on ai-stay-still
 * 
 * 105   9/24/97 4:51p Allender
 * got rid pf default_player_ship_name variable.
 * 
 * 104   9/23/97 4:35p Allender
 * added two function to add "internal" goals to ships and wings.  Used by
 * AI when it needs to do something special
 * 
 * 103   9/09/97 2:42p Allender
 * fix bug where guarding a wing that hasn't arrived caused goal code to
 * crash
 * 
 * 102   9/09/97 11:28a Johnson
 * Fixed bug: Code is not Fred-aware.
 * 
 * 101   9/08/97 1:04p Allender
 * put code in ai_post_process_mission() to set orders for ships before
 * mission starts.  Prevents ships from following orders N seconds after
 * mission actually starts
 * 
 * 100   9/05/97 5:05p Lawrance
 * memset string to 0 during restore for safety
 * 
 * 99    8/18/97 1:16p Allender
 * added ignore goal into sexpressions
 * 
 * 98    8/16/97 3:09p Mike
 * If telling a ship to attack a protected ship, unprotect that ship.
 * 
 * 97    8/16/97 2:40p Mike
 * Move unmoving ship if it's attacking a large ship and it sees incoming
 * fire.
 * Also, don't set protected bit in a ship if someone has attacked a
 * subsystem.  Only do if someone was told to disarm or disable.
 * 
 * 96    8/15/97 11:50a Duncan
 * Fixed bug with ai functions assuming ai_goal_undock takes a ship
 * target, which it doesn't.
 * 
 * 95    8/15/97 10:05a Hoffoss
 * One more change to ai_query_goal_valid() to allow for AI_GOAL_NONE.
 * 
 * 94    8/15/97 9:59a Hoffoss
 * Changed ai_query_goal_valid() again.  I can't think of any time you
 * wouldn't find it easier just to pass in a ship instead of a ship_info
 * flag structure filtered of all flags that aren't ship type bits.  Since
 * I'll call this quite a few times, it will make my life easier.
 * 
 * 93    8/15/97 9:49a Hoffoss
 * Changed ai_query_goal_valid() slightly for Fred.  Fred can be more
 * detailed with error messages should these situations happen, and
 * recover from them better.
 * 
 * 92    8/14/97 10:00p Allender
 * made ai goals bitfields so that we can have set of orders that certain
 * ship types are allowed to receive.  Added function for Hoffoss to check
 * goal vs. ship type validity
 * 
 * 91    8/12/97 9:58a Hoffoss
 * Fixed ai_update_goal_references() and query_referended_in_ai_goals().
 * 
 * 90    8/12/97 8:01a Allender
 * when adding goals to wing structures, replace the oldest entry of the
 * goals if there are no empty entries
 * 
 * 89    8/12/97 1:55a Hoffoss
 * Made extensive changes to object reference checking and handling for
 * object deletion call.
 * 
 * 88    8/10/97 4:24p Hoffoss
 * Made initial orders revert back to none if it has invalid data
 * anywhere.
 * 
 * 87    8/08/97 1:30p Allender
 * added commands (and ai goals) for support ships.  stay near ship (which
 * could be player or current target), and keep safe distance
 * 
 * 86    8/07/97 11:11a Allender
 * fixed problem with wings completing waypoint goals
 * 
 * 85    8/06/97 9:41a Allender
 * renamed from ai_goal function.  made a function to remove a goal from a
 * wing (used strictly for waypoints now).  ai-chase-wing and
 * ai-guard-wing now appear to user as ai-chase and ai-guard.  Tidied up
 * function which deals with sexpression goals
 * 
 * 84    8/06/97 8:06a Lawrance
 * add more stuff to save/restore
 * 
 * 83    8/01/97 11:51a Dave
 * Changed calls to timer_get_fixed_seconds() to simply Missiontime.
 * Solves a lot of demo related problems.
 * 
 * 82    7/30/97 11:01p Mike
 * Set submode when setting mode from player order.
 * Increase distance to circle when ignoring player's target.
 * 
 * 81    7/28/97 10:28a Mike
 * Use forward_interpolate() to prevent weird banking behavior.
 * 
 * Suppress a couple annoying mprints and clarify another.
 * 
 * 80    7/25/97 12:10p Mike
 * Better default behavior.
 * 
 * 79    7/24/97 4:55p Allender
 * added ai-evade-ship to fred and to Freespace
 * 
 * 78    7/24/97 4:20p Mike
 * Add goal and Fred hook for evade behavior.
 * 
 * 77    7/24/97 2:17p Jasen
 * Fixed a bug with mission loading.
 * 
 * 76    7/24/97 12:10a Mike
 * Suppress problems when loadign pain.fsm.
 * 
 * 75    7/23/97 6:49p Hoffoss
 * Fixed bug in ai goal adding code.  It was using a pointer that was
 * temporary.
 * 
 * 74    7/23/97 11:25a Allender
 * more robust sexpression checking.  Fixed problem with some ai goals not
 * being named properly
 *
 * $NoKeywords: $
 */

#include "ship/aigoals.h"
#include "parse/sexp.h"
#include "mission/missionlog.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "globalincs/linklist.h"
#include "playerman/player.h"
#include "network/multimsgs.h"
#include "network/multi.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "object/objectdock.h"


// all ai goals dealt with in this code are goals that are specified through
// sexpressions in the mission file.  They are either specified as part of a
// ships goals in the #Object section of the mission file, or are created (and
// removed) dynamically using the #Events section.  Default goal behaviour and
// dynamic goals are not handled here.

// defines for player issued goal priorities
#define PLAYER_PRIORITY_MIN				90
#define PLAYER_PRIORITY_SHIP				100
#define PLAYER_PRIORITY_WING				95
#define PLAYER_PRIORITY_SUPPORT_LOW		10

// define for which goals cause other goals to get purged
#define PURGE_GOALS		(AI_GOAL_IGNORE | AI_GOAL_DISABLE_SHIP | AI_GOAL_DISARM_SHIP)

// goals given from the player to other ships in the game are also handled in this
// code


#define AI_GOAL_ACHIEVABLE			1
#define AI_GOAL_NOT_ACHIEVABLE	2
#define AI_GOAL_NOT_KNOWN			3
#define AI_GOAL_SATISFIED			4

int	Ai_goal_signature;
int	Num_ai_dock_names = 0;
char	Ai_dock_names[MAX_AI_DOCK_NAMES][NAME_LENGTH];

// AL 11-17-97: A text description of the AI goals.  This is used for printing out on the
// HUD what a ship's current orders are.  If the AI goal doesn't correspond to something that
// ought to be printable, then NULL is used.
// JAS: Converted to a function in order to externalize the strings
char *Ai_goal_text(int goal)
{
	switch(goal)	{
	case 1:
		return XSTR( "attack ", 474);
	case 2:
		return XSTR( "dock ", 475);
	case 3:
		return XSTR( "waypoints", 476);
	case 4:
		return XSTR( "waypoints", 476);
	case 6:
		return XSTR( "destroy ", 477);
	case 7:
		return XSTR( "form on ", 478);
	case 8:
		return XSTR( "undock ", 479);
	case 9:
		return XSTR( "attack ", 474);
	case 10:
		return XSTR( "guard ", 480);
	case 11:
		return XSTR( "disable ", 481);
	case 12:
		return XSTR( "disarm ", 482);
	case 15:
		return XSTR( "guard ", 480);
	case 16:
		return XSTR( "evade ", 483);
	case 19:
		return XSTR( "rearm ", 484);
	}

	// Avoid compiler warning
	return NULL;
};

// function to maybe add the form on my wing goal for a player's starting wing.  Called from below and when a
// player wing arrives.
void ai_maybe_add_form_goal( wing *wingp )
{
	int j;

	// iterate through the ship_index list of this wing and check for orders.  We will do
	// this for all ships in the wing instead of on a wing only basis in cases some ships
	// in the wing actually have different orders than others
	for ( j = 0; j < wingp->current_count; j++ ) {
		ai_info *aip;

		Assert( wingp->ship_index[j] != -1 );						// get Allender

		aip = &Ai_info[Ships[wingp->ship_index[j]].ai_index];
		// don't process Player_ship
		if ( aip == Player_ai )
			continue;
		
		// it is sufficient enough to check the first goal entry to see if it has a valid
		// goal
		if ( aip->goals[0].ai_mode == AI_GOAL_NONE ) {
			// need to add a form on my wing goal here.  Ships are always forming on the player's wing.
			ai_add_ship_goal_player( AIG_TYPE_PLAYER_SHIP, AI_GOAL_FORM_ON_WING, -1, Player_ship->ship_name, aip );
		}
	}
}

void ai_post_process_mission()
{
	object *objp;
	int i;

	// Check ships in player starting wings.  Those ships should follow these rules:
	// (1) if they have no orders, they should get a form on my wing order
	// (2) if they have an order, they are free to act on it.
	//
	// So basically, we are checking for (1)
	if ( !Fred_running ) {
		for ( i = 0; i < 1; i++ ) {	//	MK, 5/9/98: Used to iterate through MAX_STARTING_WINGS, but this was too many ships forming on player.
			if ( Starting_wings[i] != -1 ) {
				wing *wingp;

				wingp = &Wings[Starting_wings[i]];

				ai_maybe_add_form_goal( wingp );

			}
		}
	}

	// for every valid ship object, call process_mission_orders to be sure that ships start the
	// mission following the orders in the mission file right away instead of waiting N seconds
	// before following them.  Do both the created list and the object list for safety
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( objp->type != OBJ_SHIP )
			continue;
		ai_process_mission_orders( OBJ_INDEX(objp), &Ai_info[Ships[objp->instance].ai_index] );
	}
	for ( objp = GET_FIRST(&obj_create_list); objp != END_OF_LIST(&obj_create_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type != OBJ_SHIP) || Fred_running )
			continue;
		ai_process_mission_orders( OBJ_INDEX(objp), &Ai_info[Ships[objp->instance].ai_index] );
	}

	return;
}

// function which determines is a goal is valid for a particular type of ship
int ai_query_goal_valid( int ship, int ai_goal )
{
	int accepted;

	if (ai_goal == AI_GOAL_NONE)
		return 1;  // anything can have no orders.

	accepted = 0;
	switch (Ship_info[Ships[ship].ship_info_index].flags & SIF_ALL_SHIP_TYPES) {
	case SIF_CARGO:
		if (!Fred_running)
			Int3();			// get Hoffoss or Allender -- cargo containers shouldn't have a goal!!!
		break;

	case SIF_FIGHTER:
		if ( ai_goal & AI_GOAL_ACCEPT_FIGHTER ){
			accepted = 1;
		}
		break;
	case SIF_BOMBER:
		if ( ai_goal & AI_GOAL_ACCEPT_BOMBER ){
			accepted = 1;
		}
		break;
	case SIF_CRUISER:
		if ( ai_goal & AI_GOAL_ACCEPT_CRUISER ){
			accepted = 1;
		}
		break;
	case SIF_FREIGHTER:
		if ( ai_goal & AI_GOAL_ACCEPT_FREIGHTER ){
			accepted = 1;
		}
		break;
	case SIF_CAPITAL:
		if ( ai_goal & AI_GOAL_ACCEPT_CAPITAL ){
			accepted = 1;
		}
		break;
	case SIF_TRANSPORT:
		if ( ai_goal & AI_GOAL_ACCEPT_TRANSPORT ){
			accepted = 1;
		}
		break;
	case SIF_SUPPORT:
		if ( ai_goal & AI_GOAL_ACCEPT_SUPPORT ){
			accepted = 1;
		}
		break;
	case SIF_ESCAPEPOD:
		if ( ai_goal & AI_GOAL_ACCEPT_ESCAPEPOD ){
			accepted = 1;
		}
		break;
	case SIF_SUPERCAP:
		if ( ai_goal & AI_GOAL_ACCEPT_SUPERCAP ){
			accepted = 1;
		}
		break;
	case SIF_SHIP_CLASS_STEALTH:
		if ( ai_goal & AI_GOAL_ACCEPT_STEALTH ){
			accepted = 1;
		}
		break;		
	case SIF_CORVETTE:
		if ( ai_goal & AI_GOAL_ACCEPT_CORVETTE ){
			accepted = 1;
		}
		break;
	case SIF_GAS_MINER:
		if ( ai_goal & AI_GOAL_ACCEPT_GAS_MINER ){
			accepted = 1;
		}
		break;
	case SIF_AWACS:
		if ( ai_goal & AI_GOAL_ACCEPT_AWACS ){
			accepted = 1;
		}
		break;
	case SIF_NO_SHIP_TYPE:
		if (!Fred_running){
			Int3();			// HUH?  doesn't make sense
		}
		break;
	default:
		if (!Fred_running){
			Int3();			// get allender or hoffos -- unknown ship type
		}
		break;
	}

	return accepted;
}

// remove an ai goal from it's list.  Uses the active_goal member as the goal to remove
void ai_remove_ship_goal( ai_info *aip, int index )
{
	// only need to set the ai_mode for the particular goal to AI_GOAL_NONE
	// reset ai mode to default behavior.  Might get changed next time through
	// ai goal code look
	Assert ( index >= 0 );			// must have a valid goal

	aip->goals[index].ai_mode = AI_GOAL_NONE;
	aip->goals[index].signature = -1;
	aip->goals[index].priority = -1;
	aip->goals[index].flags = 0;				// must reset the flags since not doing so will screw up goal sorting.
	if ( index == aip->active_goal )
		aip->active_goal = AI_GOAL_NONE;

	// mwa -- removed this line 8/5/97.  Just because we remove a goal doesn't mean to do the default
	// behavior.  We will make the call commented out below in a more reasonable location
	//ai_do_default_behavior( &Objects[Ships[aip->shipnum].objnum] );
}

void ai_clear_ship_goals( ai_info *aip )
{
	int i;

	for (i = 0; i < MAX_AI_GOALS; i++)
		ai_remove_ship_goal( aip, i );			// resets active_goal and default behavior

	aip->active_goal = AI_GOAL_NONE;					// for good measure

	// next line moved here on 8/5/97 by MWA
	// Dont reset player ai (and hence target)
	// Goober5000 - account for player ai
	//if ( !((Player_ship != NULL) && (&Ships[aip->shipnum] == Player_ship)) || Player_use_ai ) {
	if ( Player_ship == NULL || Player_use_ai || &Ships[aip->shipnum] != Player_ship)
	{
		ai_do_default_behavior( &Objects[Ships[aip->shipnum].objnum] );
	}
}

void ai_clear_wing_goals( int wingnum )
{
	int i;
	wing *wingp = &Wings[wingnum];
	//p_object *objp;

	// clear the goals for all ships in the wing
	for (i = 0; i < wingp->current_count; i++) {
		int num = wingp->ship_index[i];

		if ( num > -1 )
			ai_clear_ship_goals( &Ai_info[Ships[num].ai_index] );

	}

		// clear out the goals for the wing now
	for (i = 0; i < MAX_AI_GOALS; i++) {
		wingp->ai_goals[i].ai_mode = AI_GOAL_NONE;
		wingp->ai_goals[i].signature = -1;
		wingp->ai_goals[i].priority = -1;
	}


}

// routine which marks a wing goal as being complete.  We get the wingnum and a pointer to the goal
// structure of the goal to be removed.  This process is slightly tricky since some member of the wing
// might be pursuing a different goal.  We will have to compare based on mode, submode, priority,
// and name.. This routine is only currently called from waypoint code!!!
void ai_mission_wing_goal_complete( int wingnum, ai_goal *remove_goalp )
{
	int mode, submode, priority, i;
	char *name;
	ai_goal *aigp;
	wing *wingp;

	wingp = &Wings[wingnum];

	// set up locals for faster access.
	mode = remove_goalp->ai_mode;
	submode = remove_goalp->ai_submode;
	priority = remove_goalp->priority;
	name = remove_goalp->ship_name;

	Assert ( name );			// should not be NULL!!!!

	// remove the goal from all the ships currently in the wing
	for (i = 0; i < wingp->current_count; i++ ) {
		int num, j;
		ai_info *aip;

		num = wingp->ship_index[i];
		Assert ( num >= 0 );
		aip = &Ai_info[Ships[num].ai_index];
		for ( j = 0; j < MAX_AI_GOALS; j++ ) {
			aigp = &(aip->goals[j]);

			// don't need to worry about these types of goals since they can't possibly be a goal we are looking for.
			if ( (aigp->ai_mode == AI_GOAL_NONE) || !aigp->ship_name )
				continue;

			if ( (aigp->ai_mode == mode) && (aigp->ai_submode == submode) && (aigp->priority == priority) && !stricmp(name, aigp->ship_name) ) {
				ai_remove_ship_goal( aip, j );
				ai_do_default_behavior( &Objects[Ships[aip->shipnum].objnum] );		// do the default behavior
				break;			// we are all done
			}
		}
	}

	// now remove the goal from the wing
	for (i = 0; i < MAX_AI_GOALS; i++ ) {
		aigp = &(wingp->ai_goals[i]);
		if ( (aigp->ai_mode == AI_GOAL_NONE) || !aigp->ship_name )
			continue;

		if ( (aigp->ai_mode == mode) && (aigp->ai_submode == submode) && (aigp->priority == priority) && !stricmp(name, aigp->ship_name) ) {
			wingp->ai_goals[i].ai_mode = AI_GOAL_NONE;
			wingp->ai_goals[i].signature = -1;
			wingp->ai_goals[i].priority = -1;
			break;
		}
	}
			
}

// routine which is called with an ai object complete it's goal.  Do some action
// based on the goal what was just completed

void ai_mission_goal_complete( ai_info *aip )
{
	// if the active goal is dynamic or none, just return.  (AI_GOAL_NONE is probably an error, but
	// I don't think that this is a problem)
	if ( (aip->active_goal == AI_GOAL_NONE) || (aip->active_goal == AI_ACTIVE_GOAL_DYNAMIC) )
		return;

	ai_remove_ship_goal( aip, aip->active_goal );
	ai_do_default_behavior( &Objects[Ships[aip->shipnum].objnum] );		// do the default behavior

}

int ai_get_subsystem_type( char *subsystem )
{
	if ( strstr(subsystem, "engine") ) {
		return SUBSYSTEM_ENGINE;
	} else if ( strstr(subsystem, "radar") ) {
		return SUBSYSTEM_RADAR;
	} else if ( strstr(subsystem, "turret") ) {
		return SUBSYSTEM_TURRET;
	} else if ( strstr(subsystem, "navigation") ) {
		return SUBSYSTEM_NAVIGATION;
	} else if ( !strnicmp(subsystem, NOX("communication"), 13) ) {
		return SUBSYSTEM_COMMUNICATION;
	} else if ( !strnicmp(subsystem, NOX("weapons"), 7) )  {
		return SUBSYSTEM_WEAPONS;
	} else if ( !strnicmp(subsystem, NOX("sensors"), 7) )  {
		return SUBSYSTEM_SENSORS;
	} else if ( !strnicmp(subsystem, NOX("solar"), 5) )  {
		return SUBSYSTEM_SOLAR;
	} else if ( !strnicmp(subsystem, NOX("gas"), 3) )  {
		return SUBSYSTEM_GAS_COLLECT;
	} else if ( !strnicmp(subsystem, NOX("activator"), 9) )  {
		return SUBSYSTEM_ACTIVATION;
	} else {									// If unrecognized type, set to engine so artist can continue working...
		if (!Fred_running) {
//			Int3();							// illegal subsystem type -- find allender
		}

		return SUBSYSTEM_UNKNOWN;
	}
}

// function to prune out goals which are no longer valid, based on a goal pointer passed in.
// for instance, if we get passed a goal of "disable X", then any goals in the given goal array
// which are destroy, etc, should get removed.  goal list is the list of goals to purge.  It is
// always MAX_AI_GOALS in length.  This function will only get called when the goal which causes
// purging becomes valid.
void ai_goal_purge_invalid_goals( ai_goal *aigp, ai_goal *goal_list )
{
	int i;
	ai_goal *purge_goal;
	char *name;
	int mode, ship_index, wingnum;

	// get locals for easer access
	name = aigp->ship_name;
	mode = aigp->ai_mode;

	// these goals cannot be associated to wings, but can to a ship in a wing.  So, we should find out
	// if the ship is in a wing so we can purge goals which might operate on that wing
	ship_index = ship_name_lookup(name);
	if ( ship_index == -1 ) {
		Int3();						// get allender -- this is sort of odd
		return;
	}
	wingnum = Ships[ship_index].wingnum;

	purge_goal = goal_list;
	for ( i = 0; i < MAX_AI_GOALS; purge_goal++, i++ ) {
		int purge_ai_mode, purge_wing;

		purge_ai_mode = purge_goal->ai_mode;

		// don't need to process AI_GOAL_NONE
		if ( purge_ai_mode == AI_GOAL_NONE )
			continue;

		// goals must operate on something to be purged.
		if ( purge_goal->ship_name == NULL )
			continue;

		// determine if the purge goal is acting either on the ship or the ship's wing.
		purge_wing = wing_name_lookup( purge_goal->ship_name, 1 );

		// if the target of the purge goal is a ship (purge_wing will be -1), then if the names
		// don't match, we can continue;  if the wing is valid, don't process if the wing numbers
		// are different.
		if ( purge_wing == -1 ) {
			if ( stricmp(purge_goal->ship_name, name ) )
				continue;
		} else if ( purge_wing != wingnum )
			continue;

		switch( mode ) {
		// ignore goals should get rid of any kind of attack goal
		case AI_GOAL_IGNORE:
			if ( purge_ai_mode & (AI_GOAL_DISABLE_SHIP | AI_GOAL_DISARM_SHIP | AI_GOAL_CHASE | AI_GOAL_CHASE_WING | AI_GOAL_DESTROY_SUBSYSTEM) )
				purge_goal->flags |= AIGF_PURGE;
			break;

		// disarm/disable goals should remove any general attack
		case AI_GOAL_DISABLE_SHIP:
		case AI_GOAL_DISARM_SHIP:
			if ( purge_ai_mode & (AI_GOAL_CHASE | AI_GOAL_CHASE_WING) )
				purge_goal->flags |= AIGF_PURGE;
			break;
		}
	}
}

// function to purge the goals of all ships in the game based on the incoming goal structure
void ai_goal_purge_all_invalid_goals( ai_goal *aigp )
{
	int mode, i;
	ship_obj *sop;

	mode = aigp->ai_mode;

	// only purge goals if a new goal is one of the types in next statement
	if ( !(mode & PURGE_GOALS) )
		return;

	for ( sop = GET_FIRST(&Ship_obj_list); sop != END_OF_LIST(&Ship_obj_list); sop = GET_NEXT(sop) ) {
		ship *shipp;

		shipp = &Ships[Objects[sop->objnum].instance];
		ai_goal_purge_invalid_goals( aigp, Ai_info[shipp->ai_index].goals );
	}

	// we must do the same for the wing goals
	for (i = 0; i < num_wings; i++ )
		ai_goal_purge_invalid_goals( aigp, Wings[i].ai_goals );
}

// Goober5000
int ai_goal_find_dockpoint(int shipnum, int dock_type)
{
	int dock_index = -1;
	int loop_count = 0;

	ship *shipp = &Ships[shipnum];
	object *objp = &Objects[shipp->objnum];

	// only check 100 points for sanity's sake
	while (loop_count < 100)
	{
		dock_index = model_find_dock_index(shipp->modelnum, dock_type, dock_index+1);

		// not found?
		if (dock_index == -1)
		{
			if (loop_count == 0)
			{
				// first time around... there are no slots fitting this description
				return -1;
			}
			else
			{
				// every slot is full
				break;
			}
		}

		// we've found something... check if it's occupied
		if (dock_find_object_at_dockpoint(objp, dock_index) == NULL)
		{
			// not occupied... yay, we've found an index
			return dock_index;
		}

		// keep track
		loop_count++;
	}

	// insanity?
	if (loop_count >= 100)
		Warning(LOCATION, "Too many iterations while looking for a dockpoint on %s.  Either there was a bug or this is an übership.\n", shipp->ship_name);

	// if we're here, just return the first dockpoint
	return model_find_dock_index(shipp->modelnum, dock_type);
}

// function to fix up dock point references for objects.
// passed are the pointer to goal we are working with.  aip if the ai_info pointer
// of the ship with the order.  aigp is a pointer to the goal (of aip) of which we are
// fixing up the docking points
void ai_goal_fixup_dockpoints(ai_info *aip, ai_goal *aigp)
{
	int shipnum, docker_index, dockee_index;

	Assert ( aip->shipnum != -1 );
	shipnum = ship_name_lookup( aigp->ship_name );
	docker_index = -1;
	dockee_index = -1;

	// look for docking points of the appriopriate type.  Use cargo docks for cargo ships.
	if (Ship_info[Ships[shipnum].ship_info_index].flags & SIF_CARGO) {
		docker_index = ai_goal_find_dockpoint(aip->shipnum, DOCK_TYPE_CARGO);
		dockee_index = ai_goal_find_dockpoint(shipnum, DOCK_TYPE_CARGO);
	} else if (Ship_info[Ships[aip->shipnum].ship_info_index].flags & SIF_SUPPORT) {
		docker_index = ai_goal_find_dockpoint(aip->shipnum, DOCK_TYPE_REARM);
		dockee_index = ai_goal_find_dockpoint(shipnum, DOCK_TYPE_REARM);
	}

	// if we didn't find dockpoints above, then we should just look for generic docking points
	if ( docker_index == -1 )
		docker_index = ai_goal_find_dockpoint(aip->shipnum, DOCK_TYPE_GENERIC);
	if ( dockee_index == -1 )
		dockee_index = ai_goal_find_dockpoint(shipnum, DOCK_TYPE_GENERIC);
		
	aigp->docker.index = docker_index;
	aigp->dockee.index = dockee_index;
	aigp->flags |= AIGF_DOCK_INDEXES_VALID;
}

// these functions deal with adding goals sent from the player.  They are slightly different
// from the mission goals (i.e. those goals which come from events) in that we don't
// use sexpressions for goals from the player...so we enumerate all the parameters

void ai_add_goal_sub_player(int type, int mode, int submode, char *shipname, ai_goal *aigp )
{
	Assert ( (type == AIG_TYPE_PLAYER_WING) || (type == AIG_TYPE_PLAYER_SHIP) );

	aigp->time = Missiontime;
	aigp->type = type;											// from player for sure -- could be to ship or to wing
	aigp->ai_mode = mode;										// major mode for this goal
	aigp->ai_submode = submode;								// could mean different things depending on mode

	if ( mode == AI_GOAL_WARP )
		aigp->wp_index = submode;

	if ( mode == AI_GOAL_CHASE_WEAPON ) {
		aigp->wp_index = submode;								// submode contains the instance of the weapon
		aigp->weapon_signature = Objects[Weapons[submode].objnum].signature;
	}

	if ( shipname != NULL )
		aigp->ship_name = ai_get_goal_ship_name( shipname, &aigp->ship_name_index );
	else
		aigp->ship_name = NULL;

	// special case certain orders from player so that ships continue to do the right thing

	// make priority for these two support ship orders low so that they will prefer repairing
	// a ship over staying near a ship.
	if ( (mode == AI_GOAL_STAY_NEAR_SHIP) || (mode == AI_GOAL_KEEP_SAFE_DISTANCE) )
		aigp->priority = PLAYER_PRIORITY_SUPPORT_LOW;

	else if ( aigp->type == AIG_TYPE_PLAYER_WING )
		aigp->priority = PLAYER_PRIORITY_WING;			// player wing goals not as high as ship goals
	else
		aigp->priority = PLAYER_PRIORITY_SHIP;
}

int ai_goal_find_empty_slot( ai_goal *goals )
{
	int gindex, empty_index, oldest_index;

	empty_index = -1;
	oldest_index = 0;
	for ( gindex = 0; gindex < MAX_AI_GOALS; gindex++ ) {
		if ( goals[gindex].time < goals[oldest_index].time )
			oldest_index = gindex;

		if ( (empty_index == -1) && (goals[gindex].ai_mode == AI_GOAL_NONE) )			// get the index for this goal
			empty_index = gindex;
	}

	// if we didn't find an empty slot, find the oldest goal and use it's slot
	if ( empty_index == -1 )
		empty_index = oldest_index;

 	Assert ( empty_index < MAX_AI_GOALS );

	return empty_index;
}

// adds a goal from a player to the given ship's ai_info structure.  'type' tells us if goal
// is issued to ship or wing (from player),  mode is AI_GOAL_*. submode is the submode the
// ship should go into.  shipname is the object of the action.  aip is the ai_info pointer
// of the ship receiving the order
void ai_add_ship_goal_player( int type, int mode, int submode, char *shipname, ai_info *aip )
{
	int empty_index;
	ai_goal *aigp;

	empty_index = ai_goal_find_empty_slot( aip->goals );

	// get a pointer to the goal structure
	aigp = &aip->goals[empty_index];
	ai_add_goal_sub_player( type, mode, submode, shipname, aigp );

	// if the goal is to dock, then we must determine which dock points on the two ships to use.
	// If the target of the dock is a cargo type container, then we should use DOCK_TYPE_CARGO
	// on both ships.  Code is here instead of in ai_add_goal_sub_player() since a dock goal
	// should only occur to a specific ship.

	if ( (mode == AI_GOAL_REARM_REPAIR) || ((mode == AI_GOAL_DOCK) && (submode == AIS_DOCK_0)) ) {
		ai_goal_fixup_dockpoints( aip, aigp );
	}

	aigp->signature = Ai_goal_signature++;

}

// adds a goal from the player to the given wing (which in turn will add it to the proper
// ships in the wing
void ai_add_wing_goal_player( int type, int mode, int submode, char *shipname, int wingnum )
{
	int i, empty_index;
	wing *wingp = &Wings[wingnum];

	// add the ai goal for any ship that is currently arrived in the game.
	if ( !Fred_running ) {										// only add goals to ships if fred isn't running
		for (i = 0; i < wingp->current_count; i++) {
			int num = wingp->ship_index[i];
			if ( num == -1 )			// ship must have been destroyed or departed
				continue;
			ai_add_ship_goal_player( type, mode, submode, shipname, &Ai_info[Ships[num].ai_index] );
		}
	}

	// add the sexpression index into the wing's list of goal sexpressions if
	// there are more waves to come.  We use the same method here as when adding a goal to
	// a ship -- find the first empty entry.  If none exists, take the oldest entry and replace it.
	empty_index = ai_goal_find_empty_slot( wingp->ai_goals );
	ai_add_goal_sub_player( type, mode, submode, shipname, &wingp->ai_goals[empty_index] );
}


// common routine to add a sexpression mission goal to the appropriate goal structure.
void ai_add_goal_sub_sexp( int sexp, int type, ai_goal *aigp )
{
	int n, node, dummy, op;
	char *text;

	Assert ( Sexp_nodes[sexp].first != -1 );
	node = Sexp_nodes[sexp].first;
	text = CTEXT(node);

	aigp->signature = Ai_goal_signature++;

	aigp->time = Missiontime;
	aigp->type = type;
	aigp->flags = 0;

	op = get_operator_const( text );

	switch (op) {

	case OP_AI_WAYPOINTS_ONCE:
	case OP_AI_WAYPOINTS: {
		int ref_type;

		ref_type = Sexp_nodes[CDR(node)].subtype;
		if (ref_type == SEXP_ATOM_STRING) {  // referenced by name
			// save the waypoint path name -- the index will get resolved when the goal is checked
			// for acheivability.
			aigp->ship_name = ai_get_goal_ship_name(CTEXT(CDR(node)), &aigp->ship_name_index);  // waypoint path name;
			aigp->wp_index = -1;

		} else
			Int3();

		aigp->priority = atoi( CTEXT(CDR(CDR(node))) );
		aigp->ai_mode = AI_GOAL_WAYPOINTS;
		if ( op == OP_AI_WAYPOINTS_ONCE )
			aigp->ai_mode = AI_GOAL_WAYPOINTS_ONCE;
		break;
	}

	case OP_AI_DESTROY_SUBSYS:
		aigp->ai_mode = AI_GOAL_DESTROY_SUBSYSTEM;
		aigp->ship_name = ai_get_goal_ship_name( CTEXT(CDR(node)), &aigp->ship_name_index );
		// store the name of the subsystem in the docker.name field for now -- this field must get
		// fixed up when the goal is valid since we need to locate the subsystem on the ship's model
		aigp->docker.name = ai_get_goal_ship_name(CTEXT(CDR(CDR(node))), &dummy);
		aigp->flags |= AIGF_SUBSYS_NEEDS_FIXUP;
		aigp->priority = atoi( CTEXT(CDR(CDR(CDR(node)))) );
		break;

	case OP_AI_DISABLE_SHIP:
		aigp->ai_mode = AI_GOAL_DISABLE_SHIP;
		aigp->ship_name = ai_get_goal_ship_name( CTEXT(CDR(node)), &aigp->ship_name_index );
		aigp->ai_submode = -SUBSYSTEM_ENGINE;
		aigp->priority = atoi( CTEXT(CDR(CDR(node))) );
		break;

	case OP_AI_DISARM_SHIP:
		aigp->ai_mode = AI_GOAL_DISARM_SHIP;
		aigp->ship_name = ai_get_goal_ship_name( CTEXT(CDR(node)), &aigp->ship_name_index );
		aigp->ai_submode = -SUBSYSTEM_TURRET;
		aigp->priority = atoi( CTEXT(CDR(CDR(node))) );
		break;

	case OP_AI_WARP_OUT:
		aigp->ai_mode = AI_GOAL_WARP;
		aigp->priority = atoi( CTEXT(CDR(node)) );
		break;

		// the following goal is obsolete, but here for compatibility
	case OP_AI_WARP:
		aigp->ai_mode = AI_GOAL_WARP;
		aigp->ship_name = ai_get_goal_ship_name(CTEXT(CDR(node)), &aigp->ship_name_index);  // waypoint path name;
		//aigp->wp_index = atoi( CTEXT(CDR(node)) );		// this is the index into the warp points
		aigp->wp_index = -1;
		aigp->priority = atoi( CTEXT(CDR(CDR(node))) );
		break;

	case OP_AI_UNDOCK:
		aigp->priority = atoi( CTEXT(CDR(node)) );

		// Goober5000 - optional undock with something
		if (CDR(CDR(node)) != -1)
			aigp->ship_name = ai_get_goal_ship_name( CTEXT(CDR(CDR(node))), &aigp->ship_name_index );
		else
			aigp->ship_name = NULL;

		aigp->ai_mode = AI_GOAL_UNDOCK;
		aigp->ai_submode = AIS_UNDOCK_0;
		break;

	case OP_AI_STAY_STILL:
		aigp->ai_mode = AI_GOAL_STAY_STILL;
		aigp->ship_name = ai_get_goal_ship_name(CTEXT(CDR(node)), &aigp->ship_name_index);  // waypoint path name;
		aigp->priority = atoi( CTEXT(CDR(CDR(node))) );
		break;

	case OP_AI_DOCK:
		aigp->ship_name = ai_get_goal_ship_name( CTEXT(CDR(node)), &aigp->ship_name_index );
		aigp->docker.name = ai_add_dock_name(CTEXT(CDR(CDR(node))));
		aigp->dockee.name = ai_add_dock_name(CTEXT(CDR(CDR(CDR(node)))));
		aigp->flags &= ~AIGF_DOCK_INDEXES_VALID;
		aigp->priority = atoi( CTEXT(CDR(CDR(CDR(CDR(node))))) );

		aigp->ai_mode = AI_GOAL_DOCK;
		aigp->ai_submode = AIS_DOCK_0;		// be sure to set the submode
		break;

	case OP_AI_CHASE_ANY:
		aigp->priority = atoi( CTEXT(CDR(node)) );
		aigp->ai_mode = AI_GOAL_CHASE_ANY;
		break;

	// Goober5000
	case OP_AI_CHASE_ANY_EXCEPT:
		aigp->priority = atoi( CTEXT(CDR(node)) );
		aigp->ai_mode = AI_GOAL_CHASE_ANY_EXCEPT;

		// reset goal stuff
		aigp->num_special_objects = 0;
		aigp->special_object_flags = 0;

		// get list of exceptions
		n = CDR(CDR(node));
		for ( ; n != -1; n = CDR(n) )
		{
			aigp->special_object[aigp->num_special_objects] = ai_get_goal_ship_name( CTEXT(n), &aigp->special_object_index[aigp->num_special_objects] );
			aigp->special_object_num[aigp->num_special_objects] = -1;

			// do we have a ship?
			if ( (dummy = ship_name_lookup(aigp->special_object[aigp->num_special_objects], 1)) != -1 )
			{
				aigp->special_object_num[aigp->num_special_objects] = dummy;
			}

			// do we have a wing?
			if ( (dummy = wing_name_lookup(aigp->special_object[aigp->num_special_objects], 1)) != -1 )
			{
				aigp->special_object_num[aigp->num_special_objects] = dummy;
				aigp->special_object_flags |= (1 << (aigp->num_special_objects));	// set wing flag
			}

			// make sure we got something assigned
			Assert(aigp->special_object_num[aigp->num_special_objects] != -1);

			aigp->num_special_objects++;
		}

		if (!aigp->num_special_objects)	// found any?
		{
			// just make it ai-chase-any
			aigp->ai_mode = AI_GOAL_CHASE_ANY;
		}

		break;

	case OP_AI_PLAY_DEAD:
		aigp->priority = atoi( CTEXT(CDR(node)) );
		aigp->ai_mode = AI_GOAL_PLAY_DEAD;
		break;

	case OP_AI_KEEP_SAFE_DISTANCE:
		aigp->priority = atoi( CTEXT(CDR(node)) );
		aigp->ai_mode = AI_GOAL_KEEP_SAFE_DISTANCE;
		break;

	case OP_AI_CHASE:
	case OP_AI_GUARD:
	case OP_AI_GUARD_WING:
	case OP_AI_CHASE_WING:
	case OP_AI_EVADE_SHIP:
	case OP_AI_STAY_NEAR_SHIP:
	case OP_AI_IGNORE:
		aigp->ship_name = ai_get_goal_ship_name( CTEXT(CDR(node)), &aigp->ship_name_index );
		aigp->priority = atoi( CTEXT(CDR(CDR(node))) );

		if ( op == OP_AI_CHASE ) {
			aigp->ai_mode = AI_GOAL_CHASE;

			// in the case of ai_chase (and ai_guard) we must do a wing_name_lookup on the name
			// passed here to see if we could be chasing a wing.  Hoffoss and I have consolidated
			// sexpression operators which makes this step necessary
			if ( wing_name_lookup(aigp->ship_name, 1) != -1 )
				aigp->ai_mode = AI_GOAL_CHASE_WING;

		} else if ( op == OP_AI_GUARD ) {
			aigp->ai_mode = AI_GOAL_GUARD;
			if ( wing_name_lookup(aigp->ship_name, 1) != -1 )
				aigp->ai_mode = AI_GOAL_GUARD_WING;

		} else if ( op == OP_AI_EVADE_SHIP ) {
			aigp->ai_mode = AI_GOAL_EVADE_SHIP;

		} else if ( op == OP_AI_GUARD_WING ) {
			aigp->ai_mode = AI_GOAL_GUARD_WING;
		} else if ( op == OP_AI_CHASE_WING ) {
			aigp->ai_mode = AI_GOAL_CHASE_WING;
		} else if ( op == OP_AI_STAY_NEAR_SHIP ) {
			aigp->ai_mode = AI_GOAL_STAY_NEAR_SHIP;
		} else if ( op == OP_AI_IGNORE ) {
			aigp->ai_mode = AI_GOAL_IGNORE;
		} else
			Int3();		// this is impossible

		break;

	default:
		Int3();			// get ALLENDER -- invalid ai-goal specified for ai object!!!!
	}

	if ( aigp->priority >= PLAYER_PRIORITY_MIN ) {
		nprintf (("AI", "bashing sexpression priority of goal %s from %d to %d.\n", text, aigp->priority, PLAYER_PRIORITY_MIN-1));
		aigp->priority = PLAYER_PRIORITY_MIN-1;
	}
}

// adds an ai goal for an individual ship
// type determines who has issues this ship a goal (i.e. the player/mission event/etc)
void ai_add_ship_goal_sexp( int sexp, int type, ai_info *aip )
{
	int gindex;

	gindex = ai_goal_find_empty_slot( aip->goals );
	ai_add_goal_sub_sexp( sexp, type, &aip->goals[gindex] );
}

// code to add ai goals to wings.
void ai_add_wing_goal_sexp(int sexp, int type, int wingnum)
{
	int i;
	wing *wingp = &Wings[wingnum];

	// add the ai goal for any ship that is currently arrived in the game (only if fred isn't running
	if ( !Fred_running ) {
		for (i = 0; i < wingp->current_count; i++) {
			int num = wingp->ship_index[i];
			if ( num == -1 )			// ship must have been destroyed or departed
				continue;
			ai_add_ship_goal_sexp( sexp, type, &Ai_info[Ships[num].ai_index] );
		}
	}

	// add the sexpression index into the wing's list of goal sexpressions if
	// there are more waves to come
	if ((wingp->num_waves - wingp->current_wave > 0) || Fred_running) {
		int gindex;

		gindex = ai_goal_find_empty_slot( wingp->ai_goals );
		ai_add_goal_sub_sexp( sexp, type, &wingp->ai_goals[gindex] );
	}
}

// function for internal code to add a goal to a ship.  Needed when the AI finds itself in a situation
// that it must get out of by issuing itself an order.
//
// objp is the object getting the goal
// goal_type is one of AI_GOAL_*
// other_name is a character string objp might act on (for docking, this is a shipname, for guarding
// this name can be a shipname or a wingname)
// docker_point and dockee_point are used for the AI_GOAL_DOCK command to tell two ships where to dock
// immediate means to process this order right away
void ai_add_goal_ship_internal( ai_info *aip, int goal_type, char *name, int docker_point, int dockee_point, int immediate )
{
	int gindex;
	ai_goal *aigp;

	// find an empty slot to put this goal in.
	gindex = ai_goal_find_empty_slot( aip->goals );

	aigp = &(aip->goals[gindex]);

	aigp->signature = Ai_goal_signature++;

	aigp->time = Missiontime;
	aigp->type = AIG_TYPE_DYNAMIC;
	aigp->flags = AIGF_GOAL_OVERRIDE;

	switch ( goal_type ) {
	case AI_GOAL_DOCK:
		aigp->ship_name = name;
		aigp->docker.index = docker_point;
		aigp->dockee.index = dockee_point;
		aigp->priority = 100;

		aigp->ai_mode = AI_GOAL_DOCK;
		aigp->ai_submode = AIS_DOCK_0;		// be sure to set the submode
		break;

	case AI_GOAL_UNDOCK:
		aigp->ship_name = name;
		aigp->priority = 100;
		aigp->ai_mode = AI_GOAL_UNDOCK;
		aigp->ai_submode = AIS_UNDOCK_0;
		break;

	case AI_GOAL_GUARD:
		aigp->ai_mode = AI_GOAL_GUARD;
		if ( wing_name_lookup(name, 1) != -1 )
			aigp->ai_mode = AI_GOAL_GUARD_WING;
		break;

	case AI_GOAL_REARM_REPAIR:
		aigp->ai_mode = AI_GOAL_REARM_REPAIR;
		aigp->ai_submode = 0;
		aigp->ship_name = name;
		aigp->priority = PLAYER_PRIORITY_MIN-1;		// make the priority always less than what the player's is
		aigp->flags &= ~AIGF_GOAL_OVERRIDE;				// don't override this goal.  rearm repair requests should happen in order
		ai_goal_fixup_dockpoints( aip, aigp );
		break;

	default:
		Int3();		// unsupported internal goal -- see Mike K or Mark A.
		break;
	}


	// process the orders immediately so that these goals take effect right away
	if ( immediate )
		ai_process_mission_orders( Ships[aip->shipnum].objnum, aip );
}

// function to add an internal goal to a wing.  Mike K says that the goal doesn't need to persist
// across waves of the wing so we merely need to add the goal to each ship in the wing.  Certain
// goal are simply not valid for wings (like dock, undock).  Immediate parameter gets passed to add_ship_goal
// to say whether or not we should process this goal right away
void ai_add_goal_wing_internal( wing *wingp, int goal_type, char *name, int immediate )
{
	int i;

	// be sure we are not trying to issue dock or undock goals to wings
	Assert ( (goal_type != AI_GOAL_DOCK) || (goal_type != AI_GOAL_UNDOCK) );

	for (i = 0; i < wingp->current_count; i++) {
		int num = wingp->ship_index[i];
		if ( num == -1 )			// ship must have been destroyed or departed
			continue;
		ai_add_goal_ship_internal(  &Ai_info[Ships[num].ai_index], goal_type, name, -1, -1, immediate);
	}
}

// this function copies goals from a wing to an ai_info * from a ship.
void ai_copy_mission_wing_goal( ai_goal *aigp, ai_info *aip )
{
	int j;

	for ( j = 0; j < MAX_AI_GOALS; j++ ) {
		if ( aip->goals[j].ai_mode == AI_GOAL_NONE )
			break;
	}
	Assert ( j < MAX_AI_GOALS );
	aip->goals[j] = *aigp;
}


#define SHIP_STATUS_GONE				1
#define SHIP_STATUS_NOT_ARRIVED		2
#define SHIP_STATUS_ARRIVED			3
#define SHIP_STATUS_UNKNOWN			4

// function to determine if an ai goal is achieveable or not.  Will return
// one of the AI_GOAL_* values.  Also determines is a goal was successful.

int ai_mission_goal_achievable( int objnum, ai_goal *aigp )
{
	int status;
	char *ai_shipname;
	int return_val;
	object *objp;
	ai_info *aip;

	//  these orders are always achievable.
	if ( (aigp->ai_mode == AI_GOAL_KEEP_SAFE_DISTANCE)
		|| (aigp->ai_mode == AI_GOAL_CHASE_ANY) || (aigp->ai_mode == AI_GOAL_STAY_STILL)
		|| (aigp->ai_mode == AI_GOAL_PLAY_DEAD) || (aigp->ai_mode == AI_GOAL_CHASE_ANY_EXCEPT) )
		return AI_GOAL_ACHIEVABLE;

	// warp (depart) only achievable if there's somewhere to depart to
	if (aigp->ai_mode == AI_GOAL_WARP)
	{
		ship *shipp = &Ships[Objects[objnum].instance];

		// always valid if has subspace drive
		if (!(shipp->flags2 & SF2_NO_SUBSPACE_DRIVE))
			return AI_GOAL_ACHIEVABLE;

		// if no subspace drive, only valid if there's somewhere to depart to

		// locate a capital ship on the same team:
		if (ship_get_ship_with_dock_bay(shipp->team) >= 0)
		{
			return AI_GOAL_ACHIEVABLE;
		}
		else
		{
			return AI_GOAL_NOT_ACHIEVABLE;
		}
	}


	// form on my wing is always achievable, but need to set the override bit so that it
	// always gets executed next
	if ( aigp->ai_mode == AI_GOAL_FORM_ON_WING ) {
		aigp->flags |= AIGF_GOAL_OVERRIDE;
		return AI_GOAL_ACHIEVABLE;
	}

	// check to see if we have a valid index.  If not, then try to set one up.  If that
	// fails, then we must pitch this order
	if ( (aigp->ai_mode == AI_GOAL_WAYPOINTS_ONCE) || (aigp->ai_mode == AI_GOAL_WAYPOINTS) ) {
		if ( aigp->wp_index == -1 ) {
			int i;

			for (i = 0; i < Num_waypoint_lists; i++) {
				if (!stricmp(aigp->ship_name, Waypoint_lists[i].name)) {
					aigp->wp_index = i;
					break;
				}
			}
			if ( i == Num_waypoint_lists ) {
				Warning(LOCATION, "Unknown waypoint %s.  not found in mission file.  Killing ai goal", aigp->ship_name );
				return AI_GOAL_NOT_ACHIEVABLE;
			}
		}
		return AI_GOAL_ACHIEVABLE;
	}

	objp = &Objects[objnum];
	Assert( objp->instance != -1 );
	ai_shipname = Ships[objp->instance].ship_name;
	aip = &Ai_info[Ships[objp->instance].ai_index];

	return_val = AI_GOAL_SATISFIED;

	// next, determine if the goal has been completed successfully
	switch ( aigp->ai_mode ) {

	case AI_GOAL_DOCK:
	case AI_GOAL_CHASE_WING:
	case AI_GOAL_UNDOCK:
		//status = mission_log_get_time( LOG_SHIP_DOCK, ai_shipname, aigp->ship_name, NULL);
		//status = mission_log_get_time( LOG_SHIP_UNDOCK, ai_shipname, aigp->ship_name, NULL );
		//MWA 3/20/97 -- cannot short circuit a dock or undock goal already succeeded -- we must
		// rely on the goal removal code to just remove this goal.  This is because docking/undock
		// can happen > 1 time per mission per pair of ships.  The above checks will find only
		// if the ships docked or undocked at all, which is not what we want.
		status = 0;
		break;
	case AI_GOAL_DESTROY_SUBSYSTEM: {
		int shipnum;
		ship_subsys *ssp;

		// shipnum could be -1 depending on if the ship hasn't arrived or died.  only look for subsystem
		// destroyed when shipnum is valid
		shipnum = ship_name_lookup( aigp->ship_name );

		// can't determine the status of this goal if ship not valid
		// or we haven't found a valid subsystem index yet
		if ( (shipnum == -1) || (aigp->flags & AIGF_SUBSYS_NEEDS_FIXUP) ) {
			status = 0;
			break;
		}

		// if the ship is not in the mission or the subsystem name is still being stored, mark the status
		// as 0 so we can continue.  (The subsystem name must be turned into an index into the ship's subsystems
		// for this goal to be valid).
		Assert ( aigp->ai_submode >= 0 );
		ssp = ship_get_indexed_subsys( &Ships[shipnum], aigp->ai_submode );
		status = mission_log_get_time( LOG_SHIP_SUBSYS_DESTROYED, aigp->ship_name, ssp->system_info->name, NULL );

		break;
	}
	case AI_GOAL_DISABLE_SHIP:
		status = mission_log_get_time( LOG_SHIP_DISABLED, aigp->ship_name, NULL, NULL );
		break;
	case AI_GOAL_DISARM_SHIP:
		status = mission_log_get_time( LOG_SHIP_DISARMED, aigp->ship_name, NULL, NULL );
		break;

		// to guard or ignore a ship, the goal cannot continue if the ship being guarded is either destroyed
		// or has departed.
	case AI_GOAL_GUARD:
	case AI_GOAL_IGNORE:
	case AI_GOAL_EVADE_SHIP:
	case AI_GOAL_CHASE:
	case AI_GOAL_STAY_NEAR_SHIP:
	case AI_GOAL_REARM_REPAIR:
	case AI_GOAL_FLY_TO_SHIP: {
		int shipnum;

		// MWA -- 4/22/98.  Check for the ship actually being in the mission before
		// checking departure and destroyed.  In multiplayer, since ships can respawn,
		// they get log entries for being destroyed even though they have respawned.
		shipnum = ship_name_lookup( aigp->ship_name );
		if ( shipnum == -1 ) {
			status = mission_log_get_time( LOG_SHIP_DEPART, aigp->ship_name, NULL, NULL);
			if ( !status ) {
				status = mission_log_get_time( LOG_SHIP_DESTROYED, aigp->ship_name, NULL, NULL);
				if ( status )
					return_val = AI_GOAL_NOT_ACHIEVABLE;
			}
		} else {
			status = 0;
		}
		break;
	}

	case AI_GOAL_GUARD_WING:
		status = mission_log_get_time( LOG_WING_DEPART, aigp->ship_name, NULL, NULL );
		if ( !status ) {
			status = mission_log_get_time( LOG_WING_DESTROYED, aigp->ship_name, NULL, NULL);
			if ( status )
				return_val = AI_GOAL_NOT_ACHIEVABLE;
		}
		break;

		// the following case statement returns control to caller on all paths!!!!
	case AI_GOAL_CHASE_WEAPON:
		// for chase weapon, we simply need to look at the weapon instance that we are trying to
		// attack and see if the object still exists, and has the same signature that we expect.
		if ( Weapons[aigp->wp_index].objnum == -1 )
			return AI_GOAL_NOT_ACHIEVABLE;

		// if the signatures don't match, then goal isn't achievable.
		if ( Objects[Weapons[aigp->wp_index].objnum].signature != aigp->weapon_signature )
			return AI_GOAL_NOT_ACHIEVABLE;

		// otherwise, we should be good to go
		return AI_GOAL_ACHIEVABLE;

		break;

	default:
		Int3();
		status = 0;
		break;
	}

	// if status is true, then the mission log event was found and the goal was satisfied.  return
	// AI_GOAL_SATISFIED which should allow this ai object to move onto the next order
	if ( status )
		return return_val;

	// determine the status of the shipname that this object is acting on.  There are a couple of
	// special cases to deal with.  Both the chase wing and undock commands will return from within
	// the if statement.
	if ( (aigp->ai_mode == AI_GOAL_CHASE_WING) || (aigp->ai_mode == AI_GOAL_GUARD_WING) )
	{
		int num = wing_name_lookup( aigp->ship_name );
		wing *wingp = &Wings[num];

		if ( wingp->flags & WF_WING_GONE )
			return AI_GOAL_NOT_ACHIEVABLE;
		else if ( wingp->total_arrived_count == 0 )
			return AI_GOAL_NOT_KNOWN;
		else
			return AI_GOAL_ACHIEVABLE;
	}
	// Goober5000 - undocking from an unspecified object is always achievable;
	// undocking from a specified object is handled below
	else if ( (aigp->ai_mode == AI_GOAL_UNDOCK) && (aigp->ship_name == NULL) )
	{
			return AI_GOAL_ACHIEVABLE;
	}
	else
	{
		// goal ship is currently in mission
		if ( ship_name_lookup( aigp->ship_name ) != -1 )
		{
			status = SHIP_STATUS_ARRIVED;
		}
		// goal ship is still on the arrival list
		else if ( !mission_parse_ship_arrived(aigp->ship_name) )
		{
			status = SHIP_STATUS_NOT_ARRIVED;
		}
		// goal ship has left the area
		else if ( ship_find_exited_ship_by_name(aigp->ship_name) != -1 )
		{
			status = SHIP_STATUS_GONE;
		}
		else
		{
			Int3();		// get ALLENDER
			status = SHIP_STATUS_UNKNOWN;
		}
	}

	// if the goal is an ignore/disable/disarm goal, then 
	if ( (status == SHIP_STATUS_ARRIVED) && (aigp->ai_mode & PURGE_GOALS) && !(aigp->flags & AIGF_GOALS_PURGED) ) {
		ai_goal_purge_all_invalid_goals( aigp );
		aigp->flags |= AIGF_GOALS_PURGED;
	}
		

	// if we are docking, validate the docking indices on both ships.  We might have to change names to indices.
	// only enter this calculation if the ship we are docking with has arrived.  If the ship is gone, then
	// this goal will get removed.
	if ( (aigp->ai_mode == AI_GOAL_DOCK) && (status == SHIP_STATUS_ARRIVED) ) {
		int index, modelnum, shipnum;

		if (!(aigp->flags & AIGF_DOCKER_INDEX_VALID)) {
			modelnum = Ships[objp->instance].modelnum;
			index = model_find_dock_name_index(modelnum, aigp->docker.name);
			aigp->docker.index = index;
			aigp->flags |= AIGF_DOCKER_INDEX_VALID;
		}
		if (!(aigp->flags & AIGF_DOCKEE_INDEX_VALID)) {
			shipnum = ship_name_lookup(aigp->ship_name);
			if ( shipnum != -1 ) {
				modelnum = Ships[shipnum].modelnum;
				index = model_find_dock_name_index(modelnum, aigp->dockee.name);
				aigp->dockee.index = index;
				aigp->flags |= AIGF_DOCKEE_INDEX_VALID;
			} else
				aigp->dockee.index = -1;		// this will force code into if statement below making goal not achievable.
		}
		if ( (aigp->dockee.index == -1) || (aigp->docker.index == -1) ) {
			Int3();			// for now, allender wants to know about these things!!!!
			return AI_GOAL_NOT_ACHIEVABLE;
		}

		// if ship is disabled, don't know if it can dock or not
		if ( Ships[objp->instance].flags & SF_DISABLED )
			return AI_GOAL_NOT_KNOWN;

		// we must also determine if we're prevented from docking for any reason
		shipnum = ship_name_lookup(aigp->ship_name);
		Assert( shipnum != -1 );
		object *goal_objp = &Objects[Ships[shipnum].objnum];

		// if the ship that I am supposed to dock with is docked with something else, then I need to put my goal on hold
		//	[MK, 4/23/98: With Mark, we believe this fixes the problem of Comet refusing to warp out after docking with Omega.
		//	This bug occurred only when mission goals were validated in the frame in which Comet docked, which happened about
		// once in 10-20 tries.]
		if ( object_is_docked(goal_objp) )
		{
			// if the dockpoint I need to dock to is occupied by someone other than me
			if (dock_find_object_at_dockpoint(goal_objp, aigp->dockee.index) != objp)
				return AI_GOAL_NOT_KNOWN;
		}

		// if this ship is docked and needs to get docked with something else, then undock this ship
		if ( object_is_docked(objp) )
		{
			// if the dockpoint I need to dock with is occupied by someone other than the guy I need to dock to
			object *obstacle_objp = dock_find_object_at_dockpoint(objp, aigp->docker.index);
			if (obstacle_objp == NULL)
			{
				// nobody in the way... we're good
			}
			else if (obstacle_objp != goal_objp)
			{
				// if this goal isn't on hold yet, then issue the undock goal and return NOT_KNOWN
				// which will then place the goal on hold until the undocking is complete.
				if ( !(aigp->flags & AIGF_GOAL_ON_HOLD) )
					ai_add_goal_ship_internal( aip, AI_GOAL_UNDOCK, Ships[obstacle_objp->instance].ship_name, -1, -1, 0 );

				return AI_GOAL_NOT_KNOWN;
			}
			else
			{
				// if this ship is already docked with the guy this order tells him to dock with,
				// then mark the goal as satisfied.
				// MWA 2/23/98 -- don't return anything.  Since this item is a goal, the ai_dock code
				// should remove the goal!!!!
				//return AI_GOAL_SATISFIED;
			}
		}

	// Goober5000 - necessitated by the multiple ship docking
	} else if ( (aigp->ai_mode == AI_GOAL_UNDOCK) && (status == SHIP_STATUS_ARRIVED) ) {
		// Put this goal on hold if we're already undocking.  Otherwise the new goal will pre-empt
		// the current goal and strange things might happen.  One is that the object movement code
		// forgets the previous undocking and "re-docks" the previous goal's ship.  Other problems
		// might happen too, so err on the safe side.  (Yay for emergent paragraph justification!)
		if ((aip->mode == AIM_DOCK) && (aip->submode >= AIS_UNDOCK_0))
		{
			// only put it on hold if it's someone other than the guy we're undocking from right now!!
			if (aip->goal_objnum != Ships[ship_name_lookup(aigp->ship_name)].objnum)
				return AI_GOAL_NOT_KNOWN;
		}

	} else if ( (aigp->ai_mode == AI_GOAL_DESTROY_SUBSYSTEM) && (status == SHIP_STATUS_ARRIVED) ) {
		// if the ship has arrived, and the goal is destroy subsystem, then check to see that we
		// have fixed up the subsystem name (of the subsystem to destroy) into an index into
		// the ship's subsystem list
		if ( aigp->flags & AIGF_SUBSYS_NEEDS_FIXUP ) {
			int shipnum;			

			shipnum = ship_name_lookup( aigp->ship_name );
			if ( shipnum != -1 ) {
				aigp->ai_submode = ship_get_subsys_index( &Ships[shipnum], aigp->docker.name );
				aigp->flags &= ~AIGF_SUBSYS_NEEDS_FIXUP;
			} else {
				Int3();
				return AI_GOAL_NOT_ACHIEVABLE;			// force this goal to be invalid
			}
		}
	} else if ( (aigp->ai_mode == AI_GOAL_IGNORE) && (status == SHIP_STATUS_ARRIVED) ) {
		int shipnum;
		object *ignored;

		// for ignoring a ship, call the ai_ignore object function, then declare the goal satisfied
		shipnum = ship_name_lookup( aigp->ship_name );
		Assert( shipnum != -1 );		// should be true because of above status
		ignored = &Objects[Ships[shipnum].objnum];
		ai_ignore_object(objp, ignored, 100);
		return AI_GOAL_SATISFIED;
	}

	switch ( aigp->ai_mode ) {

	case AI_GOAL_CHASE:
	case AI_GOAL_DOCK:
	case AI_GOAL_DESTROY_SUBSYSTEM:
	case AI_GOAL_UNDOCK:
	case AI_GOAL_GUARD:
	case AI_GOAL_GUARD_WING:
	case AI_GOAL_DISABLE_SHIP:
	case AI_GOAL_DISARM_SHIP:
	case AI_GOAL_IGNORE:
	case AI_GOAL_EVADE_SHIP:
	case AI_GOAL_STAY_NEAR_SHIP:
	case AI_GOAL_FLY_TO_SHIP:
		if ( status == SHIP_STATUS_ARRIVED )
			return AI_GOAL_ACHIEVABLE;
		else if ( status == SHIP_STATUS_NOT_ARRIVED )
			return AI_GOAL_NOT_KNOWN;
		else if ( status == SHIP_STATUS_GONE )
			return AI_GOAL_NOT_ACHIEVABLE;
		else if ( status == SHIP_STATUS_UNKNOWN )
			return AI_GOAL_NOT_KNOWN;
			Int3();		// get allender -- bad logic
		break;

	// for rearm repair ships, a goal is only achievable if the support ship isn't repairing anything
	// else at the time, or is set to repair the ship for this goal.  All other goals should be placed
	// on hold by returning GOAL_NOT_KNOWN.
	case AI_GOAL_REARM_REPAIR: {
		int shipnum;

		// short circuit a couple of cases.  Ship not arrived shouldn't happen.  Ship gone means
		// we mark the goal as not achievable.
		if ( status == SHIP_STATUS_NOT_ARRIVED ) {
			Int3();										// get Allender.  this shouldn't happen!!!
			return AI_GOAL_NOT_ACHIEVABLE;
		}

		if ( status == SHIP_STATUS_GONE )
			return AI_GOAL_NOT_ACHIEVABLE;

		Assert( aigp->ship_name );
		shipnum = ship_name_lookup( aigp->ship_name );

		// if desitnation currently being repaired, then goal is stil active
		if ( Ai_info[Ships[shipnum].ai_index].ai_flags & AIF_BEING_REPAIRED )
			return AI_GOAL_ACHIEVABLE;

		// if the destination ship is dying or departing (but not completed yet), the mark goal as
		// not achievable.
		if ( Ships[shipnum].flags & (SF_DYING | SF_DEPARTING) )
			return AI_GOAL_NOT_ACHIEVABLE;

		// if the destination object is no longer awaiting repair, then remove the item
		if ( !(Ai_info[Ships[shipnum].ai_index].ai_flags & AIF_AWAITING_REPAIR) )
			return AI_GOAL_NOT_ACHIEVABLE;

		// not repairing anything means that he can do this goal!!!
		if ( !(aip->ai_flags  & AIF_REPAIRING) )
			return AI_GOAL_ACHIEVABLE;

		// test code!!!
		if ( aip->goal_objnum == -1 ) {
			// -- MK, 11/9/97 -- I was always hitting this: Int3();
			return AI_GOAL_ACHIEVABLE;
		}

		// if he is repairing something, he can satisfy his repair goal (his goal_objnum)
		// return GOAL_NOT_KNOWN which is kind of a hack which puts the goal on hold until it can be
		// satisfied.  
		if ( aip->goal_objnum != Ships[shipnum].objnum )
			return AI_GOAL_NOT_KNOWN;

		return AI_GOAL_ACHIEVABLE;
	}

	default:
		Int3();			// invalid case in switch:
	}

	return AI_GOAL_NOT_KNOWN;
}

//	Compare function for system qsort() for sorting ai_goals based on priority.
//	Return values set to sort array in _decreasing_ order.
int ai_goal_priority_compare(const void *a, const void *b)
{
	ai_goal	*ga, *gb;

	ga = (ai_goal *) a;
	gb = (ai_goal *) b;

	// first, sort based on whether or not the ON_HOLD flag is set for the goal.
	// If the flag is set, don't push the goal higher in the list even if priority
	// is higher since goal cannot currently be achieved.

	if ( (ga->flags & AIGF_GOAL_ON_HOLD) && !(gb->flags & AIGF_GOAL_ON_HOLD) )
		return 1;
	else if ( !(ga->flags & AIGF_GOAL_ON_HOLD) && (gb->flags & AIGF_GOAL_ON_HOLD) )
		return -1;

	// check whether or not the goal override flag is set.  If it is set, then push this goal higher
	// in the list

	else if ( (ga->flags & AIGF_GOAL_OVERRIDE) && !(gb->flags & AIGF_GOAL_OVERRIDE) )
		return -1;
	else if ( !(ga->flags & AIGF_GOAL_OVERRIDE) && (gb->flags & AIGF_GOAL_OVERRIDE) )
		return 1;

	// now normal priority processing

	if (ga->priority > gb->priority)
		return -1;
	else if ( ga->priority < gb->priority )
		return 1;
	else {
		if ( ga->time > gb->time )
			return -1;
		else // if ( ga->time < gb->time )			// this way prevents element swapping if times happen to be equal (which they should not)
			return 1;
	}
}

//	Prioritize goal list.
//	First sort on priority.
//	Then sort on time for goals of equivalent priority.
//	objnum	The object number to act upon.  Redundant with *aip.
//	*aip		The AI info to act upon.  Goals are stored at aip->goals
void prioritize_goals(int objnum, ai_info *aip)
{

	//	First sort based on priority field.
	qsort(aip->goals, MAX_AI_GOALS, sizeof(ai_goal), ai_goal_priority_compare);

}

//	Scan the list of goals at aip->goals.
//	Remove obsolete goals.
//	objnum	Object of interest.  Redundant with *aip.
//	*aip		contains goals at aip->goals.
void validate_mission_goals(int objnum, ai_info *aip)
{
	int	i;
	
	// loop through all of the goals to determine which goal should be followed.
	// This determination will be based on priority, and the time at which it was issued.
	for ( i = 0; i < MAX_AI_GOALS; i++ ) {
		int		state;
		ai_goal	*aigp;

		aigp = &aip->goals[i];

		// quick check to see if this goal is valid or not, or if we are trying to process the
		// current goal
		if (aigp->ai_mode == AI_GOAL_NONE)
			continue;

		// purge any goals which should get purged
		if ( aigp->flags & AIGF_PURGE ) {
			ai_remove_ship_goal( aip, i );
			continue;
		}

		state = ai_mission_goal_achievable( objnum, aigp );

		// if this order is no longer a valid one, remove it
		if ( (state == AI_GOAL_NOT_ACHIEVABLE) || (state == AI_GOAL_SATISFIED) ) {
			ai_remove_ship_goal( aip, i );
			continue;
		}

		// if the status is achievable, and the on_hold flag is set, clear the flagb
		if ( (state == AI_GOAL_ACHIEVABLE) && (aigp->flags & AIGF_GOAL_ON_HOLD) )
			aigp->flags &= ~AIGF_GOAL_ON_HOLD;

		// if the goal is not known, then set the ON_HOLD flag so that it doesn't get counted as
		// a goal to be pursued
		if ( state == AI_GOAL_NOT_KNOWN )
			aigp->flags |= AIGF_GOAL_ON_HOLD;		// put this goal on hold until it becomes true
	}

	// if we had an active goal, and that goal is now in hold, make the mode AIM_NONE.  If a new valid
	// goal is produced after prioritizing, then the mode will get reset immediately.  Otherwise, setting
	// the mode to none will force ship to do default behavior.
	if ( (aip->goals[0].ai_mode != AI_GOAL_NONE) && (aip->goals[0].flags & AIGF_GOAL_ON_HOLD) )
		aip->mode = AIM_NONE;

	// if the active goal is a rearm/repair goal, the put all other valid goals (which are not repair goals)
	// on hold
	if ( (aip->goals[0].ai_mode == AI_GOAL_REARM_REPAIR) && object_is_docked(&Objects[objnum]) ) {
		for ( i = 1; i < MAX_AI_GOALS; i++ ) {
			if ( (aip->goals[i].ai_mode == AI_GOAL_NONE) || (aip->goals[i].ai_mode == AI_GOAL_REARM_REPAIR) )
				continue;
			aip->goals[i].flags |= AIGF_GOAL_ON_HOLD;
		}
	}
}

//XSTR:OFF
static char *Goal_text[5] = {
"EVENT_SHIP",
"EVENT_WING",
"PLAYER_SHIP",
"PLAYER_WING",
"DYNAMIC",
};
//XSTR:ON

extern char *Mode_text[MAX_AI_BEHAVIORS];

// code to process ai "orders".  Orders include those determined from the mission file and those
// given by the player to a ship that is under his control.  This function gets called for every
// AI object every N seconds through the ai loop.
void ai_process_mission_orders( int objnum, ai_info *aip )
{
	object	*objp = &Objects[objnum];
	object	*other_obj;
	ai_goal	*current_goal;
	int		wingnum, shipnum;
	int		original_signature;

/*	if (!stricmp(Ships[objp->instance].ship_name, "gtt comet")) {
		for (int i=0; i<MAX_AI_GOALS; i++) {
			if (aip->goals[i].signature != -1) {
				nprintf(("AI", "%6.1f: mode=%s, type=%s, ship=%s\n", f2fl(Missiontime), Mode_text[aip->goals[i].ai_mode], Goal_text[aip->goals[i].type], aip->goals[i].ship_name));
			}
		}
		nprintf(("AI", "\n"));
	}
*/

	// AL 12-12-97: If a ship is entering/leaving a docking bay, wait until path
	//					 following is finished before pursuing goals.
	if ( aip->mode == AIM_BAY_EMERGE || aip->mode == AIM_BAY_DEPART ) {
		return;
	}

	//	Goal #0 is always the active goal, as we maintain a sorted list.
	//	Get the signature to see if sorting it again changes it.
	original_signature = aip->goals[0].signature;

	validate_mission_goals(objnum, aip);

	//	Sort the goal array by priority and other factors.
	prioritize_goals(objnum, aip);

	//	Make sure there's a goal to pursue, else return.
	if (aip->goals[0].signature == -1) {
		if (aip->mode == AIM_NONE)
			ai_do_default_behavior(objp);
		return;
	}

	//	If goal didn't change, return.
	if ((aip->active_goal != -1) && (aip->goals[0].signature == original_signature))
		return;

	// if the first goal in the list has the ON_HOLD flag, set, there is no current valid goal
	// to pursue.
	if ( aip->goals[0].flags & AIGF_GOAL_ON_HOLD )
		return;

	//	Kind of a hack for now.  active_goal means the goal currently being pursued.
	//	It will always be #0 since the list is prioritized.
	aip->active_goal = 0;

	//nprintf(("AI", "New goal for %s = %i\n", Ships[objp->instance].ship_name, aip->goals[0].ai_mode));

	current_goal = &aip->goals[0];

#ifndef NO_NETWORK
	if ( MULTIPLAYER_MASTER ){
		send_ai_info_update_packet( objp, AI_UPDATE_ORDERS );
	}
#endif

	// if this object was flying in formation off of another object, remove the flag that tells him
	// to do this.  The form-on-my-wing command is removed from the goal list as soon as it is called, so
	// we are safe removing this bit here.
	aip->ai_flags &= ~AIF_FORMATION_OBJECT;

	// Goober5000 - we may want to use AI for the player
	// AL 3-7-98: If this is a player ship, and the goal is not a formation goal, then do a quick out
	//if ( !(Player_use_ai) && (objp->flags & OF_PLAYER_SHIP) && (current_goal->ai_mode != AI_GOAL_FORM_ON_WING) ) {
	if ( !Player_use_ai && objp->flags & OF_PLAYER_SHIP)
	{
		return;
	}	



	switch ( current_goal->ai_mode ) {

	case AI_GOAL_CHASE:
		if ( current_goal->ship_name ) {
			shipnum = ship_name_lookup( current_goal->ship_name );
			Assert (shipnum != -1 );			// shouldn't get here if this is false!!!!
			other_obj = &Objects[Ships[shipnum].objnum];
		} else
			other_obj = NULL;						// we get this case when we tell ship to engage enemy!

		//	Mike -- debug code!
		//	If a ship has a subobject on it, attack that instead of the main ship!
		ai_attack_object( objp, other_obj, current_goal->priority, NULL);
		break;

	case AI_GOAL_CHASE_WEAPON:
		Assert( Weapons[current_goal->wp_index].objnum != -1 );
		other_obj = &Objects[Weapons[current_goal->wp_index].objnum];
		ai_attack_object( objp, other_obj, current_goal->priority, NULL );
		break;

	case AI_GOAL_GUARD:
		shipnum = ship_name_lookup( current_goal->ship_name );
		Assert (shipnum != -1 );			// shouldn't get here if this is false!!!!
		other_obj = &Objects[Ships[shipnum].objnum];
		// shipnum and other_obj are the shipnumber and object pointer of the object that you should
		// guard.
		if (objp != other_obj) {
			ai_set_guard_object(objp, other_obj);
			aip->submode_start_time = Missiontime;
		} else {
			mprintf(("Warning: Ship %s told to guard itself.  Goal ignored.\n", Ships[objp->instance].ship_name));
		}
		// -- What is this doing here?? -- MK, 7/30/97 -- ai_do_default_behavior( objp );
		break;

	case AI_GOAL_GUARD_WING:
		wingnum = wing_name_lookup( current_goal->ship_name );
		Assert (wingnum != -1 );			// shouldn't get here if this is false!!!!
		ai_set_guard_wing(objp, wingnum);
		aip->submode_start_time = Missiontime;
		break;

	case AI_GOAL_WAYPOINTS:				// do nothing for waypoints
	case AI_GOAL_WAYPOINTS_ONCE: {
		int flags = 0;

		if ( current_goal->ai_mode == AI_GOAL_WAYPOINTS)
			flags |= WPF_REPEAT;
		ai_start_waypoints(objp, current_goal->wp_index, flags);
		break;
	}

	case AI_GOAL_FLY_TO_SHIP:
		ai_start_fly_to_ship(objp, current_goal->ship_name);
		break;

	case AI_GOAL_DOCK: {
		shipnum = ship_name_lookup( current_goal->ship_name );
		Assert (shipnum != -1 );			// shouldn't get here if this is false!!!!
		other_obj = &Objects[Ships[shipnum].objnum];

		// be sure that we have indices for docking points here!  If we ever had names, they should
		// get fixed up in goal_achievable so that the points can be checked there for validity
		Assert (current_goal->flags & AIGF_DOCK_INDEXES_VALID);
		ai_dock_with_object( objp, current_goal->docker.index, other_obj, current_goal->dockee.index, current_goal->priority, AIDO_DOCK );
		aip->submode_start_time = Missiontime;
		break;
	}

	case AI_GOAL_UNDOCK:
		// try to find the object which which this object is docked with.  Use that object as the
		// "other object" for the undocking proceedure.  If "other object" isn't found, then the undock
		// goal cannot continue.  Spit out a warning and remove the goal.

		// Goober5000 - do we have a specific ship to undock from?
		if ( current_goal->ship_name != NULL )
		{
			shipnum = ship_name_lookup( current_goal->ship_name );

			// hmm, perhaps he was destroyed
			if (shipnum == -1)
			{
				other_obj = NULL;
			}
			// he exists... let's undock from him
			else
			{
				other_obj = &Objects[Ships[shipnum].objnum];
			}
		}
		// no specific ship
		else
		{
			// are we docked?
			if (object_is_docked(objp))
			{
				// just pick the first guy we're docked to
				other_obj = dock_get_first_docked_object( objp );

				// and add the ship name so it displays on the HUD
				current_goal->ship_name = Ships[other_obj->instance].ship_name;
			}
			// hmm, nobody exists that we can undock from
			else
			{
				other_obj = NULL;
			}
		}

		if ( other_obj == NULL ) {
			//Int3();
			// assume that the guy he was docked with doesn't exist anymore.  (i.e. a cargo containuer
			// can get destroyed while docked with a freighter.)  We should just remove this goal and
			// let this ship pick up it's next goal.
			ai_mission_goal_complete( aip );		// mark as complete, so we can remove it and move on!!!
			break;
		}

		// passing 0, 0 is okay because the undock code will figure out where to undock from
		ai_dock_with_object( objp, 0, other_obj, 0, current_goal->priority, AIDO_UNDOCK );
		aip->submode_start_time = Missiontime;
		break;


		// when destroying a subsystem, we can destroy a specific instance of a subsystem
		// or all instances of a type of subsystem (i.e. a specific engine or all engines).
		// the ai_submode value is > 0 for a specific instance of subsystem and < 0 for all
		// instances of a specific type
	case AI_GOAL_DESTROY_SUBSYSTEM:
	case AI_GOAL_DISABLE_SHIP:
	case AI_GOAL_DISARM_SHIP: {
		shipnum = ship_name_lookup( current_goal->ship_name );
		other_obj = &Objects[Ships[shipnum].objnum];
		ai_attack_object( objp, other_obj, current_goal->priority, NULL);
		ai_set_attack_subsystem( objp, current_goal->ai_submode );		// submode stored the subsystem type
		if (current_goal->ai_mode != AI_GOAL_DESTROY_SUBSYSTEM) {
			if (aip->target_objnum != -1) {
				//	Only protect if _not_ a capital ship.  We don't want the Lucifer accidentally getting protected.
				if (!(Ship_info[Ships[shipnum].ship_info_index].flags & SIF_HUGE_SHIP))
					Objects[aip->target_objnum].flags |= OF_PROTECTED;
			}
		} else	//	Just in case this ship had been protected, unprotect it.
			if (aip->target_objnum != -1)
				Objects[aip->target_objnum].flags &= ~OF_PROTECTED;

		break;
									  }

	case AI_GOAL_CHASE_WING:
		wingnum = wing_name_lookup( current_goal->ship_name );
		ai_attack_wing(objp, wingnum, current_goal->priority);
		break;

	case AI_GOAL_CHASE_ANY:
		ai_attack_object( objp, NULL, current_goal->priority, NULL );
		break;

	// Goober5000
	case AI_GOAL_CHASE_ANY_EXCEPT:
		aip->target_objnum = -1;	// force reacquisition of target in case we're attacking an exception
		aip->enemy_wing = -1;		// same with any current enemy wing
		ai_attack_object( objp, NULL, current_goal->priority, NULL, 1 );
		break;

	case AI_GOAL_WARP: {
		int index;

		index = current_goal->wp_index;
		mission_do_departure( objp );
		break;
	}

	case AI_GOAL_EVADE_SHIP:
		shipnum = ship_name_lookup( current_goal->ship_name );
		other_obj = &Objects[Ships[shipnum].objnum];
		ai_evade_object( objp, other_obj, current_goal->priority );
		break;

	case AI_GOAL_STAY_STILL:
		// for now, ignore any other parameters!!!!
		// clear out the object's goals.  Seems to me that if a ship is staying still for a purpose
		// then we need to clear everything out since there is not a real way to get rid of this goal
		// clearing out goals is okay here since we are now what mode to set this AI object to.
		ai_clear_ship_goals( aip );
		ai_stay_still( objp, NULL );
		break;

	case AI_GOAL_PLAY_DEAD:
		// if a ship is playing dead, MWA says that it shouldn't try to do anything else.
		// clearing out goals is okay here since we are now what mode to set this AI object to.
		ai_clear_ship_goals( aip );
		aip->mode = AIM_PLAY_DEAD;
		aip->submode = -1;
		break;

	case AI_GOAL_FORM_ON_WING:
		// for form on wing, we need to clear out all goals for this ship, and then call the form
		// on wing AI code
		// clearing out goals is okay here since we are now what mode to set this AI object to.
		ai_clear_ship_goals( aip );
		shipnum = ship_name_lookup( current_goal->ship_name );
		other_obj = &Objects[Ships[shipnum].objnum];
		ai_form_on_wing( objp, other_obj );
		break;

// labels for support ship commands

	case AI_GOAL_STAY_NEAR_SHIP: {
		shipnum = ship_name_lookup( current_goal->ship_name );
		other_obj = &Objects[Ships[shipnum].objnum];
		// todo MK:  hook to keep support ship near other_obj -- other_obj could be the player object!!!
		float	dist = 300.0f;		//	How far away to stay from ship.  Should be set in SEXP?
		ai_do_stay_near(objp, other_obj, dist);
		break;
										  }

	case AI_GOAL_KEEP_SAFE_DISTANCE:
		// todo MK: hook to keep support ship at a safe distance

		// Goober5000 - hmm, never implemented - let's see about that
		ai_do_safety(objp);
		break;

	case AI_GOAL_REARM_REPAIR:
		shipnum = ship_name_lookup( current_goal->ship_name );
		other_obj = &Objects[Ships[shipnum].objnum];
		ai_rearm_repair( objp, current_goal->docker.index, other_obj, current_goal->dockee.index, current_goal->priority );
		break;

	default:
		Int3();
		break;
	}

}

void ai_update_goal_references(ai_goal *goals, int type, char *old_name, char *new_name)
{
	int i, mode, flag, dummy;

	for (i=0; i<MAX_AI_GOALS; i++)  // loop through all the goals in the Ai_info entry
	{
		mode = goals[i].ai_mode;
		flag = 0;
		switch (type)
		{
			case REF_TYPE_SHIP:
			case REF_TYPE_PLAYER:
				switch (mode)
				{
					case AI_GOAL_CHASE:
					case AI_GOAL_DOCK:
					case AI_GOAL_DESTROY_SUBSYSTEM:
					case AI_GOAL_GUARD:
					case AI_GOAL_DISABLE_SHIP:
					case AI_GOAL_DISARM_SHIP:
					case AI_GOAL_IGNORE:
					case AI_GOAL_EVADE_SHIP:
					case AI_GOAL_STAY_NEAR_SHIP:
						flag = 1;
				}
				break;

			case REF_TYPE_WING:
				switch (mode)
				{
					case AI_GOAL_CHASE_WING:
					case AI_GOAL_GUARD_WING:
						flag = 1;
				}
				break;

			case REF_TYPE_WAYPOINT:
				switch (mode)
				{
					case AI_GOAL_WAYPOINTS:
					case AI_GOAL_WAYPOINTS_ONCE:
						flag = 1;
				}
				break;

			case REF_TYPE_PATH:
				switch (mode)
				{
					case AI_GOAL_WAYPOINTS:
					case AI_GOAL_WAYPOINTS_ONCE:
					
						flag = 1;
				}
				break;
		}

		if (flag)  // is this a valid goal to parse for this conversion?
			if (!stricmp(goals[i].ship_name, old_name)) {
				if (*new_name == '<')  // target was just deleted..
					goals[i].ai_mode = AI_GOAL_NONE;
				else
					goals[i].ship_name = ai_get_goal_ship_name(new_name, &dummy);
			}
	}
}

int query_referenced_in_ai_goals(ai_goal *goals, int type, char *name)
{
	int i, mode, flag;

	for (i=0; i<MAX_AI_GOALS; i++)  // loop through all the goals in the Ai_info entry
	{
		mode = goals[i].ai_mode;
		flag = 0;
		switch (type)
		{
			case REF_TYPE_SHIP:
				switch (mode)
				{
					case AI_GOAL_CHASE:
					case AI_GOAL_DOCK:
					case AI_GOAL_DESTROY_SUBSYSTEM:
					case AI_GOAL_GUARD:
					case AI_GOAL_DISABLE_SHIP:
					case AI_GOAL_DISARM_SHIP:
					case AI_GOAL_IGNORE:
					case AI_GOAL_EVADE_SHIP:
					case AI_GOAL_STAY_NEAR_SHIP:
						flag = 1;
				}
				break;

			case REF_TYPE_WING:
				switch (mode)
				{
					case AI_GOAL_CHASE_WING:
					case AI_GOAL_GUARD_WING:
						flag = 1;
				}
				break;

			case REF_TYPE_WAYPOINT:
				switch (mode)
				{
					case AI_GOAL_WAYPOINTS:
					case AI_GOAL_WAYPOINTS_ONCE:
						flag = 1;
				}
				break;

			case REF_TYPE_PATH:
				switch (mode)
				{
					case AI_GOAL_WAYPOINTS:
					case AI_GOAL_WAYPOINTS_ONCE:
						flag = 1;
				}
				break;
		}

		if (flag)  // is this a valid goal to parse for this conversion?
		{
			if (!stricmp(goals[i].ship_name, name))
				return 1;
		}
	}

	return 0;
}

char *ai_add_dock_name(char *str)
{
	char *ptr;
	int i;

	Assert(strlen(str) < NAME_LENGTH - 1);
	for (i=0; i<Num_ai_dock_names; i++)
		if (!stricmp(Ai_dock_names[i], str))
			return Ai_dock_names[i];

	Assert(Num_ai_dock_names < MAX_AI_DOCK_NAMES);
	ptr = Ai_dock_names[Num_ai_dock_names++];
	strcpy(ptr, str);
	return ptr;
}
