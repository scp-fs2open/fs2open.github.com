/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "ai/aigoals.h"
#include "globalincs/linklist.h"
#include "mission/missionlog.h"
#include "mission/missionparse.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "object/object.h"
#include "object/objectdock.h"
#include "object/waypoint.h"
#include "parse/sexp.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "weapon/weapon.h"


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

#define MAX_GOAL_PRIORITY				200

// define for which goals cause other goals to get purged
// Goober5000 - okay, this seems really stupid.  If any ship in the mission is assigned a goal
// in PURGE_GOALS_ALL_SHIPS, *every* other ship will have certain goals purged.  So I added
// PURGE_GOALS_ONE_SHIP for goals which should only purge other goals in the one ship.
#define PURGE_GOALS_ALL_SHIPS		(AI_GOAL_IGNORE | AI_GOAL_DISABLE_SHIP | AI_GOAL_DISARM_SHIP)
#define PURGE_GOALS_ONE_SHIP		(AI_GOAL_IGNORE_NEW)

// goals given from the player to other ships in the game are also handled in this
// code


#define AI_GOAL_ACHIEVABLE			1
#define AI_GOAL_NOT_ACHIEVABLE		2
#define AI_GOAL_NOT_KNOWN			3
#define AI_GOAL_SATISFIED			4

int	Ai_goal_signature;
int	Num_ai_dock_names = 0;
char	Ai_dock_names[MAX_AI_DOCK_NAMES][NAME_LENGTH];

ai_goal_list Ai_goal_names[] =
{
	{ "Attack ship",			AI_GOAL_CHASE,			0 },
	{ "Dock",					AI_GOAL_DOCK,			0 },
	{ "Waypoints",				AI_GOAL_WAYPOINTS,		0 },
	{ "Waypoints once",			AI_GOAL_WAYPOINTS_ONCE,	0 },
	{ "Depart",					AI_GOAL_WARP,			0 },
	{ "Attack subsys",			AI_GOAL_DESTROY_SUBSYSTEM,	0 },
	{ "Form on wing",			AI_GOAL_FORM_ON_WING,	0 },
	{ "Undock",					AI_GOAL_UNDOCK,			0 },
	{ "Attack wing",			AI_GOAL_CHASE_WING,		0 },
	{ "Guard ship",				AI_GOAL_GUARD,			0 },
	{ "Disable ship",			AI_GOAL_DISABLE_SHIP,	0 },
	{ "Disarm ship",			AI_GOAL_DISARM_SHIP,	0 },
	{ "Attack any",				AI_GOAL_CHASE_ANY,		0 },
	{ "Ignore ship",			AI_GOAL_IGNORE,			0 },
	{ "Ignore ship (new)",		AI_GOAL_IGNORE_NEW,		0 },
	{ "Guard wing",				AI_GOAL_GUARD_WING,		0 },
	{ "Evade ship",				AI_GOAL_EVADE_SHIP,		0 },
	{ "Stay near ship",			AI_GOAL_STAY_NEAR_SHIP,	0 },
	{ "keep safe dist",			AI_GOAL_KEEP_SAFE_DISTANCE,	0 },
	{ "Rearm ship",				AI_GOAL_REARM_REPAIR,	0 },
	{ "Stay still",				AI_GOAL_STAY_STILL,		0 },
	{ "Play dead",				AI_GOAL_PLAY_DEAD,		0 },
	{ "Attack weapon",			AI_GOAL_CHASE_WEAPON,	0 },
	{ "Fly to ship",			AI_GOAL_FLY_TO_SHIP,	0 },
};

int Num_ai_goals = sizeof(Ai_goal_names) / sizeof(ai_goal_list);

// AL 11-17-97: A text description of the AI goals.  This is used for printing out on the
// HUD what a ship's current orders are.  If the AI goal doesn't correspond to something that
// ought to be printable, then NULL is used.
// JAS: Converted to a function in order to externalize the strings
const char *Ai_goal_text(int goal)
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
}

// function to maybe add the form on my wing goal for a player's starting wing.  Called when a player wing arrives.
void ai_maybe_add_form_goal( wing *wingp )
{
	int j;

	// iterate through the ship_index list of this wing and check for orders.  We will do
	// this for all ships in the wing instead of on a wing only basis in case some ships
	// in the wing actually have different orders than others
	for ( j = 0; j < wingp->current_count; j++ ) {
		ai_info *aip;

		Assert( wingp->ship_index[j] != -1 );						// get Allender

		aip = &Ai_info[Ships[wingp->ship_index[j]].ai_index];
		// don't process Player_ship
		if ( aip == Player_ai )
			continue;
		
		// it is sufficient enough to check the first goal entry to see if it has a valid goal
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
		//	MK, 5/9/98: Used to iterate through MAX_STARTING_WINGS, but this was too many ships forming on player.
		// Goober5000 - MK originally iterated on only the first wing; now we iterate on only the player wing
		// because the player wing may not be first
		for ( i = 0; i < MAX_STARTING_WINGS; i++ ) {	
			if (Starting_wings[i] >= 0 && Starting_wings[i] == Player_ship->wingnum) {
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

/**
 * Determines if a goal is valid for a particular type of ship
 *
 * @param ship Ship type to test
 * @param ai_goal_type Goal type to test
 */
int ai_query_goal_valid( int ship, int ai_goal_type )
{
	int accepted;

	if (ai_goal_type == AI_GOAL_NONE)
		return 1;  // anything can have no orders.

	accepted = 0;

	//WMC - This is much simpler with shiptypes.tbl
	//Except you have to add the orders into it by hand.
	int ship_type = Ship_info[Ships[ship].ship_info_index].class_type;
	if(ship_type > -1)
	{
		if(ai_goal_type & Ship_types[ship_type].ai_valid_goals) {
			accepted = 1;
		}
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
	// Don't reset player ai (and hence target)
	// Goober5000 - account for player ai
	//if ( !((Player_ship != NULL) && (&Ships[aip->shipnum] == Player_ship)) || Player_use_ai ) {
	if ( (Player_ship == NULL) || (&Ships[aip->shipnum] != Player_ship) || Player_use_ai )
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
		wingp->ai_goals[i].flags = 0;
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
	name = remove_goalp->target_name;

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
			if ( (aigp->ai_mode == AI_GOAL_NONE) || !aigp->target_name )
				continue;

			if ( (aigp->ai_mode == mode) && (aigp->ai_submode == submode) && (aigp->priority == priority) && !stricmp(name, aigp->target_name) ) {
				ai_remove_ship_goal( aip, j );
				ai_do_default_behavior( &Objects[Ships[aip->shipnum].objnum] );		// do the default behavior
				break;			// we are all done
			}
		}
	}

	// now remove the goal from the wing
	for (i = 0; i < MAX_AI_GOALS; i++ ) {
		aigp = &(wingp->ai_goals[i]);
		if ( (aigp->ai_mode == AI_GOAL_NONE) || !aigp->target_name )
			continue;

		if ( (aigp->ai_mode == mode) && (aigp->ai_submode == submode) && (aigp->priority == priority) && !stricmp(name, aigp->target_name) ) {
			wingp->ai_goals[i].ai_mode = AI_GOAL_NONE;
			wingp->ai_goals[i].signature = -1;
			wingp->ai_goals[i].priority = -1;
			wingp->ai_goals[i].flags = 0;
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

// function to prune out goals which are no longer valid, based on a goal pointer passed in.
// for instance, if we get passed a goal of "disable X", then any goals in the given goal array
// which are destroy, etc, should get removed.  goal list is the list of goals to purge.  It is
// always MAX_AI_GOALS in length.  This function will only get called when the goal which causes
// purging becomes valid.
void ai_goal_purge_invalid_goals( ai_goal *aigp, ai_goal *goal_list, ai_info *aip, int ai_wingnum )
{
	int i, j;
	ai_goal *purge_goal;
	char *name;
	int mode, ship_index, wingnum;

	// get locals for easer access
	name = aigp->target_name;
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
		if ( purge_goal->target_name == NULL )
			continue;

		// determine if the purge goal is acting either on the ship or the ship's wing.
		purge_wing = wing_name_lookup( purge_goal->target_name, 1 );

		// if the target of the purge goal is a ship (purge_wing will be -1), then if the names
		// don't match, we can continue;  if the wing is valid, don't process if the wing numbers
		// are different.
		if ( purge_wing == -1 ) {
			if ( stricmp(purge_goal->target_name, name ) )
				continue;
		} else if ( purge_wing != wingnum )
			continue;

		switch (mode)
		{
			// ignore goals should get rid of any kind of attack goal
			case AI_GOAL_IGNORE:
			case AI_GOAL_IGNORE_NEW:
				if ( purge_ai_mode & (AI_GOAL_DISABLE_SHIP | AI_GOAL_DISARM_SHIP | AI_GOAL_CHASE | AI_GOAL_CHASE_WING | AI_GOAL_DESTROY_SUBSYSTEM) )
					purge_goal->flags |= AIGF_PURGE;
				break;

			// disarm/disable goals should remove attacks from certain ships types
			case AI_GOAL_DISARM_SHIP:
			case AI_GOAL_DISABLE_SHIP:
				if ( purge_ai_mode & (AI_GOAL_CHASE | AI_GOAL_CHASE_WING) ) {
					int ai_ship_type;

					// for wings we grab the ship type of the wing leader
					if (ai_wingnum >= 0) {
						ai_ship_type = Ship_info[Ships[Wings[ai_wingnum].special_ship].ship_info_index].class_type;
					}
					// otherwise we simply grab it from the ship itself
					else {
						Assert(aip != NULL);
						ai_ship_type = Ship_info[Ships[aip->shipnum].ship_info_index].class_type;
					}

					// grab the ship type of the ship that is being disarmed/disabled
					ship_type_info *crippled_ships_type = &Ship_types[Ship_info[Ships[ship_index].ship_info_index].class_type];

					// work through all the ship types which to see if the class matching our ai ship must ignore the ship 
					// being disarmed/disabled
					for ( j=0 ; j < (int)crippled_ships_type->ai_cripple_ignores.size(); j++) {
						if (crippled_ships_type->ai_cripple_ignores[j] == ai_ship_type) {
								purge_goal->flags |= AIGF_PURGE;
						}
					}
				}	
				break;
		}
	}
}

// function to purge the goals of *all* ships in the game based on the incoming goal structure
void ai_goal_purge_all_invalid_goals(ai_goal *aigp)
{
	int i;
	ship_obj *sop;

	// only purge goals if a new goal is one of the types in next statement
	if (!(aigp->ai_mode & PURGE_GOALS_ALL_SHIPS))
		return;
	
	for (sop = GET_FIRST(&Ship_obj_list); sop != END_OF_LIST(&Ship_obj_list); sop = GET_NEXT(sop))
	{
		ship *shipp = &Ships[Objects[sop->objnum].instance];
		ai_goal_purge_invalid_goals(aigp, Ai_info[shipp->ai_index].goals, &Ai_info[shipp->ai_index], -1);
	}

	// we must do the same for the wing goals
	for (i = 0; i < Num_wings; i++)
	{
		ai_goal_purge_invalid_goals(aigp, Wings[i].ai_goals, NULL, i);
	}
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
		dock_index = model_find_dock_index(Ship_info[shipp->ship_info_index].model_num, dock_type, dock_index+1);

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
		Warning(LOCATION, "Too many iterations while looking for a dockpoint on %s.\n", shipp->ship_name);

	// if we're here, just return the first dockpoint
	return model_find_dock_index(Ship_info[shipp->ship_info_index].model_num, dock_type);
}

// function to fix up dock point references for objects.
// passed are the pointer to goal we are working with.  aip is the ai_info pointer
// of the ship with the order.  aigp is a pointer to the goal (of aip) of which we are
// fixing up the docking points
void ai_goal_fixup_dockpoints(ai_info *aip, ai_goal *aigp)
{
	int shipnum, docker_index, dockee_index;

	Assert ( aip->shipnum != -1 );
	shipnum = ship_name_lookup( aigp->target_name );
	Assertion ( shipnum != -1, "Couldn't find ai goal's target_name (%s); get a coder!\n", aigp->target_name );
	docker_index = -1;
	dockee_index = -1;

	//WMC - This gets a bit complex with shiptypes.tbl
	//Basically this finds the common dockpoint.
	//For this, the common flags for aip's active point (ie point it wants to dock with)
	//and aigp's passive point (point it wants to be docked with) are combined
	//and the common ones are used, in order of precedence.
	//Yes, it does sound vaguely like a double-entree.

	int aip_type_dock = Ship_info[Ships[aip->shipnum].ship_info_index].class_type;
	int aigp_type_dock = Ship_info[Ships[shipnum].ship_info_index].class_type;

	int common_docks = 0;

	if(aip_type_dock > -1) {
		aip_type_dock = Ship_types[aip_type_dock].ai_active_dock;
	} else {
		aip_type_dock = 0;
	}

	if(aigp_type_dock > -1) {
		aigp_type_dock = Ship_types[aigp_type_dock].ai_passive_dock;
	} else {
		aigp_type_dock = 0;
	}

	common_docks = aip_type_dock & aigp_type_dock;

	//Now iterate through types.
	for(int i = 0; i < Num_dock_type_names; i++)
	{
		if(common_docks & Dock_type_names[i].def) {
			docker_index = ai_goal_find_dockpoint(aip->shipnum, Dock_type_names[i].def);
			dockee_index = ai_goal_find_dockpoint(shipnum, Dock_type_names[i].def);
			break;
		}
	}

	// look for docking points of the appriopriate type.  Use cargo docks for cargo ships.
	/*
	if (Ship_info[Ships[shipnum].ship_info_index].flags & SIF_CARGO) {
		docker_index = ai_goal_find_dockpoint(aip->shipnum, DOCK_TYPE_CARGO);
		dockee_index = ai_goal_find_dockpoint(shipnum, DOCK_TYPE_CARGO);
	} else if (Ship_info[Ships[aip->shipnum].ship_info_index].flags & SIF_SUPPORT) {
		docker_index = ai_goal_find_dockpoint(aip->shipnum, DOCK_TYPE_REARM);
		dockee_index = ai_goal_find_dockpoint(shipnum, DOCK_TYPE_REARM);
	}
	*/

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

void ai_add_goal_sub_player(int type, int mode, int submode, char *target_name, ai_goal *aigp )
{
	Assert ( (type == AIG_TYPE_PLAYER_WING) || (type == AIG_TYPE_PLAYER_SHIP) );

	aigp->time = Missiontime;
	aigp->type = type;										// from player for sure -- could be to ship or to wing
	aigp->ai_mode = mode;									// major mode for this goal
	aigp->ai_submode = submode;								// could mean different things depending on mode

	if ( mode == AI_GOAL_WARP ) {
		if (submode >= 0) {
			aigp->wp_list = find_waypoint_list_at_index(submode);
			Assert(aigp->wp_list != NULL);
		} else {
			aigp->wp_list = NULL;
		}
	}

	if ( mode == AI_GOAL_CHASE_WEAPON ) {
		aigp->target_instance = submode;				// submode contains the instance of the weapon
		aigp->target_signature = Objects[Weapons[submode].objnum].signature;
	} else {
		aigp->target_instance = -1;
		aigp->target_signature = -1;
	}

	if ( target_name != NULL )
		aigp->target_name = ai_get_goal_target_name( target_name, &aigp->target_name_index );
	else
		aigp->target_name = NULL;

	// special case certain orders from player so that ships continue to do the right thing

	// make priority for these two support ship orders low so that they will prefer repairing
	// a ship over staying near a ship.
	if ( (mode == AI_GOAL_STAY_NEAR_SHIP) || (mode == AI_GOAL_KEEP_SAFE_DISTANCE) )
		aigp->priority = PLAYER_PRIORITY_SUPPORT_LOW;

	// Goober5000 - same with form-on-wing, since it's a type of staying near
	else if ( mode == AI_GOAL_FORM_ON_WING )
		aigp->priority = PLAYER_PRIORITY_SUPPORT_LOW;

	else if ( aigp->type == AIG_TYPE_PLAYER_WING )
		aigp->priority = PLAYER_PRIORITY_WING;			// player wing goals not as high as ship goals
	else
		aigp->priority = PLAYER_PRIORITY_SHIP;
}

// Goober5000 - Modified this function for clarity and to avoid returning the active goal's index
// as the empty slot.  This avoids overwriting the active goal while it's being executed.  So far
// the only time I've noticed it being a problem is during a rare situation where more than five
// friendlies want to rearm at the same time.  The support ship forgets what it's doing and flies
// off to repair somebody while still docked.  I reproduced this with retail, so it's not a bug in
// my new docking code. :)
int ai_goal_find_empty_slot( ai_goal *goals, int active_goal )
{
	int gindex, oldest_index;

	oldest_index = -1;
	for ( gindex = 0; gindex < MAX_AI_GOALS; gindex++ )
	{
		// get the index for the first unused goal
		if (goals[gindex].ai_mode == AI_GOAL_NONE)
			return gindex;

		// if this is the active goal, don't consider it for pre-emption!!
		if (gindex == active_goal)
			continue;

		// store the index of the oldest goal
		if (oldest_index < 0)
			oldest_index = gindex;
		else if (goals[gindex].time < goals[oldest_index].time)
			oldest_index = gindex;
	}

	// if we didn't find an empty slot, use the oldest goal's slot
	return oldest_index;
}

int ai_goal_num(ai_goal *goals)
{
	int gindex = 0;
	int num_goals = 0;
	for(gindex = 0; gindex < MAX_AI_GOALS; gindex++)
	{
		if(goals[gindex].ai_mode != AI_GOAL_NONE)
			num_goals++;
	}

	return num_goals;
}


void ai_add_goal_sub_scripting(int type, int mode, int submode, int priority, char *target_name, ai_goal *aigp )
{
	Assert ( (type == AIG_TYPE_PLAYER_WING) || (type == AIG_TYPE_PLAYER_SHIP) );

	aigp->time = Missiontime;
	aigp->type = type;											// from player for sure -- could be to ship or to wing
	aigp->ai_mode = mode;										// major mode for this goal
	aigp->ai_submode = submode;								// could mean different things depending on mode

	if ( mode == AI_GOAL_WARP )
		aigp->wp_list = NULL;

	if ( mode == AI_GOAL_CHASE_WEAPON ) {
		aigp->target_instance = submode;								// submode contains the instance of the weapon
		aigp->target_signature = Objects[Weapons[submode].objnum].signature;
	}

	if ( target_name != NULL )
		aigp->target_name = ai_get_goal_target_name( target_name, &aigp->target_name_index );
	else
		aigp->target_name = NULL;

	aigp->priority = priority;
}

void ai_add_ship_goal_scripting(int mode, int submode, int priority, char *shipname, ai_info *aip)
{
	int empty_index;
	ai_goal *aigp;

	empty_index = ai_goal_find_empty_slot(aip->goals, aip->active_goal);
	aigp = &aip->goals[empty_index];

	//WMC - hack to get docking setup correctly
	if ( mode == AI_GOAL_DOCK ) {
		aigp->docker.name = Ships[aip->shipnum].ship_name;
		aigp->dockee.name = shipname;
		aigp->flags &= ~AIGF_DOCK_INDEXES_VALID;
	}
	ai_add_goal_sub_scripting(AIG_TYPE_PLAYER_SHIP, mode, submode, priority, shipname, aigp);

	if ( (mode == AI_GOAL_REARM_REPAIR) || ((mode == AI_GOAL_DOCK) && (submode == AIS_DOCK_0)) ) {
		ai_goal_fixup_dockpoints( aip, aigp );
	}

	aigp->signature = Ai_goal_signature++;
}

// adds a goal from a player to the given ship's ai_info structure.  'type' tells us if goal
// is issued to ship or wing (from player),  mode is AI_GOAL_*. submode is the submode the
// ship should go into.  shipname is the object of the action.  aip is the ai_info pointer
// of the ship receiving the order
void ai_add_ship_goal_player( int type, int mode, int submode, char *shipname, ai_info *aip )
{
	int empty_index;
	ai_goal *aigp;

	empty_index = ai_goal_find_empty_slot( aip->goals, aip->active_goal );

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
	empty_index = ai_goal_find_empty_slot( wingp->ai_goals, -1 );
	ai_add_goal_sub_player( type, mode, submode, shipname, &wingp->ai_goals[empty_index] );
}


// common routine to add a sexpression mission goal to the appropriate goal structure.
void ai_add_goal_sub_sexp( int sexp, int type, ai_goal *aigp, char *actor_name )
{
	int node, dummy, op;
	char *text;

	Assert ( Sexp_nodes[sexp].first != -1 );
	node = Sexp_nodes[sexp].first;
	text = CTEXT(node);

	aigp->signature = Ai_goal_signature++;

	aigp->target_name = NULL;
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
			// save the waypoint path name -- the list will get resolved when the goal is checked
			// for achievability.
			aigp->target_name = ai_get_goal_target_name(CTEXT(CDR(node)), &aigp->target_name_index);  // waypoint path name;
			aigp->wp_list = NULL;

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
		aigp->target_name = ai_get_goal_target_name( CTEXT(CDR(node)), &aigp->target_name_index );
		// store the name of the subsystem in the docker.name field for now -- this field must get
		// fixed up when the goal is valid since we need to locate the subsystem on the ship's model
		aigp->docker.name = ai_get_goal_target_name(CTEXT(CDR(CDR(node))), &dummy);
		aigp->flags |= AIGF_SUBSYS_NEEDS_FIXUP;
		aigp->priority = atoi( CTEXT(CDR(CDR(CDR(node)))) );
		break;

	case OP_AI_DISABLE_SHIP:
		aigp->ai_mode = AI_GOAL_DISABLE_SHIP;
		aigp->target_name = ai_get_goal_target_name( CTEXT(CDR(node)), &aigp->target_name_index );
		aigp->ai_submode = -SUBSYSTEM_ENGINE;
		aigp->priority = atoi( CTEXT(CDR(CDR(node))) );
		break;

	case OP_AI_DISARM_SHIP:
		aigp->ai_mode = AI_GOAL_DISARM_SHIP;
		aigp->target_name = ai_get_goal_target_name( CTEXT(CDR(node)), &aigp->target_name_index );
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
		aigp->target_name = ai_get_goal_target_name(CTEXT(CDR(node)), &aigp->target_name_index);  // waypoint path name;
		aigp->wp_list = NULL;
		aigp->priority = atoi( CTEXT(CDR(CDR(node))) );
		break;

	case OP_AI_UNDOCK:
		aigp->priority = atoi( CTEXT(CDR(node)) );

		// Goober5000 - optional undock with something
		if (CDR(CDR(node)) != -1)
			aigp->target_name = ai_get_goal_target_name( CTEXT(CDR(CDR(node))), &aigp->target_name_index );
		else
			aigp->target_name = NULL;

		aigp->ai_mode = AI_GOAL_UNDOCK;
		aigp->ai_submode = AIS_UNDOCK_0;
		break;

	case OP_AI_STAY_STILL:
		aigp->ai_mode = AI_GOAL_STAY_STILL;
		aigp->target_name = ai_get_goal_target_name(CTEXT(CDR(node)), &aigp->target_name_index);  // waypoint path name;
		aigp->priority = atoi( CTEXT(CDR(CDR(node))) );
		break;

	case OP_AI_DOCK:
		aigp->target_name = ai_get_goal_target_name( CTEXT(CDR(node)), &aigp->target_name_index );
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

	case OP_AI_PLAY_DEAD:
		aigp->priority = atoi( CTEXT(CDR(node)) );
		aigp->ai_mode = AI_GOAL_PLAY_DEAD;
		break;

	case OP_AI_KEEP_SAFE_DISTANCE:
		aigp->priority = atoi( CTEXT(CDR(node)) );
		aigp->ai_mode = AI_GOAL_KEEP_SAFE_DISTANCE;
		break;

	case OP_AI_FORM_ON_WING:
		aigp->priority = 99;
		aigp->target_name = ai_get_goal_target_name(CTEXT(CDR(node)), &aigp->target_name_index);
		aigp->ai_mode = AI_GOAL_FORM_ON_WING;
		break;

	case OP_AI_CHASE:
	case OP_AI_GUARD:
	case OP_AI_GUARD_WING:
	case OP_AI_CHASE_WING:
	case OP_AI_EVADE_SHIP:
	case OP_AI_STAY_NEAR_SHIP:
	case OP_AI_IGNORE:
	case OP_AI_IGNORE_NEW:
		aigp->target_name = ai_get_goal_target_name( CTEXT(CDR(node)), &aigp->target_name_index );
		aigp->priority = atoi( CTEXT(CDR(CDR(node))) );

		if ( op == OP_AI_CHASE ) {
			aigp->ai_mode = AI_GOAL_CHASE;

			// in the case of ai_chase (and ai_guard) we must do a wing_name_lookup on the name
			// passed here to see if we could be chasing a wing.  Hoffoss and I have consolidated
			// sexpression operators which makes this step necessary
			if ( wing_name_lookup(aigp->target_name, 1) != -1 )
				aigp->ai_mode = AI_GOAL_CHASE_WING;

		} else if ( op == OP_AI_GUARD ) {
			aigp->ai_mode = AI_GOAL_GUARD;
			if ( wing_name_lookup(aigp->target_name, 1) != -1 )
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
		} else if ( op == OP_AI_IGNORE_NEW ) {
			aigp->ai_mode = AI_GOAL_IGNORE_NEW;
		} else
			Int3();		// this is impossible

		break;

	default:
		Int3();			// get ALLENDER -- invalid ai-goal specified for ai object!!!!
	}

	if ( aigp->priority > MAX_GOAL_PRIORITY ) {
		nprintf (("AI", "bashing sexpression priority of goal %s from %d to %d.\n", text, aigp->priority, MAX_GOAL_PRIORITY));
		aigp->priority = MAX_GOAL_PRIORITY;
	}

	// Goober5000 - we now have an extra optional chase argument to allow chasing our own team
	if ( op == OP_AI_CHASE || op == OP_AI_CHASE_WING || op == OP_AI_DISABLE_SHIP || op == OP_AI_DISARM_SHIP ) {
		if ((CDDDR(node) != -1) && is_sexp_true(CDDDR(node)))
			aigp->flags |= AIGF_TARGET_OWN_TEAM;
	}
	if ( op == OP_AI_DESTROY_SUBSYS ) {
		if ((CDDDDR(node) != -1) && is_sexp_true(CDDDDR(node)))
			aigp->flags |= AIGF_TARGET_OWN_TEAM;
	}

	// Goober5000 - since none of the goals act on the actor,
	// don't assign the goal if the actor's goal target is itself
	if (aigp->target_name != NULL && !strcmp(aigp->target_name, actor_name))
	{
		// based on ai_remove_ship_goal, these seem to be the only statements
		// necessary to cause this goal to "never have been assigned"
		aigp->ai_mode = AI_GOAL_NONE;
		aigp->signature = -1;
		aigp->priority = -1;
		aigp->flags = 0;
	}
}

/* Find the index of the goal in the passed ai_goal array
 * Call something like ai_find_goal_index( aiip->goals, AIM_* );
 * Pass -1 in submode to ignore ai_submode when searching
 * Pass -1 in priority to ignore priority when searching
 * Returns -1 if not found, or [0, MAX_AI_GOALS)
 */
int ai_find_goal_index( ai_goal* aigp, int mode, int submode, int priority )
{
	Assert( aigp != NULL );
	for ( int i = 0; i < MAX_AI_GOALS; i++ )
	{
		if ( aigp[ i ].ai_mode == mode &&
			 ( submode == -1 || aigp[ i ].ai_submode == submode ) &&
			 ( priority == -1 || aigp[ i ].priority == priority ) )
		{
			return i;
		}
	}

	return -1;
}

/* Remove a goal from the given goals structure
 * Returns the index of the goal that it clears out.
 * This is importnat so that if active_goal == index you can set AI_GOAL_NONE
 */
int ai_remove_goal_sexp_sub( int sexp, ai_goal* aigp )
{
	/* Sanity check */
	Assert( Sexp_nodes[ sexp ].first != -1 );

	/* The bits we're searching for in the goals list */
	int priority = -1;

	int goalmode = -1;
	int goalsubmode = -1;

	/* Sexp node */
	int node = -1;
	/* The operator to use */
	char* op_text = NULL;
	int op = -1;

	node = Sexp_nodes[ sexp ].first;
	op_text = CTEXT( node );
	op = get_operator_const( op_text );

	/* We now need to determine what the mode and submode values are*/
	switch( op )
	{
	case OP_AI_WAYPOINTS_ONCE:
		goalmode = AI_GOAL_WAYPOINTS_ONCE;
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		break;
	case OP_AI_WAYPOINTS:
		goalmode = AI_GOAL_WAYPOINTS;
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		break;
	case OP_AI_DESTROY_SUBSYS:
		goalmode = AI_GOAL_DESTROY_SUBSYSTEM;
		priority = ( CDR( CDR( CDR(node) ) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( CDR( node ) ) ) ) ) : -1;
		break;
	case OP_AI_DISABLE_SHIP:
		goalmode = AI_GOAL_DISABLE_SHIP;
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		break;
	case OP_AI_DISARM_SHIP:
		goalmode = AI_GOAL_DISABLE_SHIP;
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		break;
	case OP_AI_WARP_OUT:
		goalmode = AI_GOAL_WARP;
		priority = ( CDR(node) >= 0 ) ? atoi( CTEXT( CDR( node ) ) ) : -1;
		break;
	case OP_AI_WARP:
		goalmode = AI_GOAL_WARP;
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		break;
	case OP_AI_UNDOCK:
		goalmode = AI_GOAL_UNDOCK;
		goalsubmode = AIS_UNDOCK_0;
		priority = ( CDR(node) >= 0 ) ? atoi( CTEXT( CDR( node ) ) ) : -1;
		break;
	case OP_AI_STAY_STILL:
		goalmode = AI_GOAL_STAY_STILL;
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		break;
	case OP_AI_DOCK:
		goalmode = AI_GOAL_DOCK;
		goalsubmode = AIS_DOCK_0;
		priority = ( CDR( CDR( CDR( CDR(node) ) ) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( CDR( CDR( node ) ) ) ) ) ) : -1;
		break;
	case OP_AI_CHASE_ANY:
		goalmode = AI_GOAL_CHASE_ANY;
		priority = ( CDR(node) >= 0 ) ? atoi( CTEXT( CDR( node ) ) ) : -1;
		break;
	case OP_AI_PLAY_DEAD:
		goalmode = AI_GOAL_PLAY_DEAD;
		priority = ( CDR(node) >= 0 ) ? atoi( CTEXT( CDR( node ) ) ) : -1;
		break;
	case OP_AI_KEEP_SAFE_DISTANCE:
		priority = ( CDR(node) >= 0 ) ? atoi( CTEXT( CDR( node ) ) ) : -1;
		goalmode = AI_GOAL_KEEP_SAFE_DISTANCE;
		break;
	case OP_AI_CHASE:
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		if ( wing_name_lookup( CTEXT( CDR( node ) ), 1 ) != -1 )
			goalmode = AI_GOAL_CHASE_WING;
		else
			goalmode = AI_GOAL_CHASE;
		break;
	case OP_AI_GUARD:
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		if ( wing_name_lookup( CTEXT( CDR( node ) ), 1 ) != -1 )
			goalmode = AI_GOAL_GUARD_WING;
		else
			goalmode = AI_GOAL_GUARD;
		break;
	case OP_AI_GUARD_WING:
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		goalmode = AI_GOAL_GUARD_WING;
		break;
	case OP_AI_CHASE_WING:
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		goalmode = AI_GOAL_CHASE_WING;
		break;
	case OP_AI_EVADE_SHIP:
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		goalmode = AI_GOAL_EVADE_SHIP;
		break;
	case OP_AI_STAY_NEAR_SHIP:
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		goalmode = AI_GOAL_STAY_NEAR_SHIP;
		break;
	case OP_AI_IGNORE:
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		goalmode = AI_GOAL_IGNORE;
		break;
	case OP_AI_IGNORE_NEW:
		priority = ( CDR( CDR(node) ) >= 0 ) ? atoi( CTEXT( CDR( CDR( node ) ) ) ) : -1;
		goalmode = AI_GOAL_IGNORE_NEW;
		break;
	default:
		Int3( );
		break;
	};
	
	/* Attempt to find the goal */
	int goalindex = ai_find_goal_index( aigp, goalmode, goalsubmode, priority );

	if ( goalindex == -1 )
		return -1; /* no more to do; */

	/* Clear out the contents of the goal. We can't use ai_remove_ship_goal since it needs ai_info and
	 * we've only got ai_goals */
	aigp[goalindex].ai_mode = AI_GOAL_NONE;
	aigp[goalindex].signature = -1;
	aigp[goalindex].priority = -1;
	aigp[goalindex].flags = 0;				// must reset the flags since not doing so will screw up goal sorting.

	return goalindex;
}

// code to add ai goals to wings.
void ai_remove_wing_goal_sexp(int sexp, int wingnum)
{
	int i;
	int goalindex = -1;
	wing *wingp = &Wings[wingnum];

	// add the ai goal for any ship that is currently arrived in the game (only if fred isn't running)
	if ( !Fred_running ) {
		for (i = 0; i < wingp->current_count; i++) {
			int num = wingp->ship_index[i];
			if ( num == -1 )			// ship must have been destroyed or departed
				continue;
			goalindex = ai_remove_goal_sexp_sub( sexp, Ai_info[Ships[num].ai_index].goals );
			if ( Ai_info[Ships[num].ai_index].active_goal == goalindex )
				Ai_info[Ships[num].ai_index].active_goal = AI_GOAL_NONE;
		}
	}

	if ((wingp->num_waves - wingp->current_wave > 0) || Fred_running) 
	{
		ai_remove_goal_sexp_sub( sexp, wingp->ai_goals );
	}
}

// adds an ai goal for an individual ship
// type determines who has issues this ship a goal (i.e. the player/mission event/etc)
void ai_add_ship_goal_sexp( int sexp, int type, ai_info *aip )
{
	int gindex;

	gindex = ai_goal_find_empty_slot( aip->goals, aip->active_goal );
	ai_add_goal_sub_sexp( sexp, type, &aip->goals[gindex], Ships[aip->shipnum].ship_name );
}

// code to add ai goals to wings.
void ai_add_wing_goal_sexp(int sexp, int type, int wingnum)
{
	int i;
	wing *wingp = &Wings[wingnum];

	// add the ai goal for any ship that is currently arrived in the game (only if fred isn't running)
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

		gindex = ai_goal_find_empty_slot( wingp->ai_goals, -1 );
		ai_add_goal_sub_sexp( sexp, type, &wingp->ai_goals[gindex], wingp->name );
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

#ifndef NDEBUG
	// Goober5000 - none of the goals act on the actor, as in ai_add_goal_sub_sexp
	if (!strcmp(name, Ships[aip->shipnum].ship_name))
	{
		// not good
		Int3();
		return;
	}
#endif

	// find an empty slot to put this goal in.
	gindex = ai_goal_find_empty_slot( aip->goals, aip->active_goal );
	aigp = &(aip->goals[gindex]);

	aigp->signature = Ai_goal_signature++;

	aigp->time = Missiontime;
	aigp->type = AIG_TYPE_DYNAMIC;
	aigp->flags = AIGF_GOAL_OVERRIDE;

	switch ( goal_type ) {

/* Goober5000 - this seems to not be used
	case AI_GOAL_DOCK:
		aigp->ship_name = name;
		aigp->docker.index = docker_point;
		aigp->dockee.index = dockee_point;
		aigp->priority = 100;

		aigp->ai_mode = AI_GOAL_DOCK;
		aigp->ai_submode = AIS_DOCK_0;		// be sure to set the submode
		break;
*/

	case AI_GOAL_UNDOCK:
		aigp->target_name = name;
		aigp->priority = MAX_GOAL_PRIORITY;
		aigp->ai_mode = AI_GOAL_UNDOCK;
		aigp->ai_submode = AIS_UNDOCK_0;
		break;

/* Goober5000 - this seems to not be used
	case AI_GOAL_GUARD:
		if ( wing_name_lookup(name, 1) != -1 )
			aigp->ai_mode = AI_GOAL_GUARD_WING;
		else
			aigp->ai_mode = AI_GOAL_GUARD;
		aigp->priority = PLAYER_PRIORITY_MIN-1;		// make the priority always less than what the player's is
		break;
*/

	case AI_GOAL_REARM_REPAIR:
		aigp->ai_mode = AI_GOAL_REARM_REPAIR;
		aigp->ai_submode = 0;
		aigp->target_name = name;
		aigp->priority = PLAYER_PRIORITY_MIN-1;		// make the priority always less than what the player's is
		aigp->flags &= ~AIGF_GOAL_OVERRIDE;				// don't override this goal.  rearm repair requests should happen in order
		ai_goal_fixup_dockpoints( aip, aigp );
		break;

	default:
		Int3();		// unsupported internal goal -- see Mike K or Mark A.
		return;
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
	Assert ( (goal_type != AI_GOAL_DOCK) && (goal_type != AI_GOAL_UNDOCK) );

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
		if ( aip->goals[j].ai_mode == AI_GOAL_NONE ) {
			aip->goals[j] = *aigp;
			break;
		}
	}

	if (j >= MAX_AI_GOALS) {
		mprintf(("Unable to assign wing goal to ship %s; the ship goals are already filled to capacity", Ships[aip->shipnum].ship_name));
	}
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
	int return_val;
	object *objp;
	ai_info *aip;
	int index = -1, sindex = -1;
	int modelnum = -1;

	objp = &Objects[objnum];
	Assert( objp->instance != -1 );
	aip = &Ai_info[Ships[objp->instance].ai_index];

	//  these orders are always achievable.
	if ( (aigp->ai_mode == AI_GOAL_KEEP_SAFE_DISTANCE)
		|| (aigp->ai_mode == AI_GOAL_CHASE_ANY) || (aigp->ai_mode == AI_GOAL_STAY_STILL)
		|| (aigp->ai_mode == AI_GOAL_PLAY_DEAD) )
		return AI_GOAL_ACHIEVABLE;

	// warp (depart) only achievable if there's somewhere to depart to
	if (aigp->ai_mode == AI_GOAL_WARP)
	{
		ship *shipp = &Ships[objp->instance];

		// always valid if has subspace drive
		if (!(shipp->flags2 & SF2_NO_SUBSPACE_DRIVE))
			return AI_GOAL_ACHIEVABLE;

		// if no subspace drive, only valid if our mothership is present

		// check that we have a mothership and that we can depart to it
		if (shipp->departure_location == DEPART_AT_DOCK_BAY)
		{
			int anchor_shipnum = ship_name_lookup(Parse_names[shipp->departure_anchor]);
			if (anchor_shipnum >= 0 && ship_useful_for_departure(anchor_shipnum, shipp->departure_path_mask))
				return AI_GOAL_ACHIEVABLE;
		}

		return AI_GOAL_NOT_KNOWN;
	}


	// form on wing is always achievable if we are forming on Player, but it's up for grabs otherwise
	// if the wing target is valid then be sure to set the override bit so that it always
	// gets executed next
	if ( aigp->ai_mode == AI_GOAL_FORM_ON_WING ) {
		sindex = ship_name_lookup( aigp->target_name );

		if (sindex < 0)
			return AI_GOAL_NOT_ACHIEVABLE;

		aigp->flags |= AIGF_GOAL_OVERRIDE;
		return AI_GOAL_ACHIEVABLE;
	}

	// check to see if we have a valid list.  If not, then try to set one up.  If that
	// fails, then we must pitch this order
	if ( (aigp->ai_mode == AI_GOAL_WAYPOINTS_ONCE) || (aigp->ai_mode == AI_GOAL_WAYPOINTS) ) {
		if ( aigp->wp_list == NULL ) {
			aigp->wp_list = find_matching_waypoint_list(aigp->target_name);

			if ( aigp->wp_list == NULL ) {
				Warning(LOCATION, "Unknown waypoint list %s - not found in mission file.  Killing ai goal", aigp->target_name );
				return AI_GOAL_NOT_ACHIEVABLE;
			}
		}
		return AI_GOAL_ACHIEVABLE;
	}


	return_val = AI_GOAL_SATISFIED;

	// next, determine if the goal has been completed successfully
	switch ( aigp->ai_mode )
	{
		case AI_GOAL_DOCK:
		case AI_GOAL_UNDOCK:
			//MWA 3/20/97 -- cannot short circuit a dock or undock goal already succeeded -- we must
			// rely on the goal removal code to just remove this goal.  This is because docking/undock
			// can happen > 1 time per mission per pair of ships.  The above checks will find only
			// if the ships docked or undocked at all, which is not what we want.
			status = 0;
			break;

		case AI_GOAL_DESTROY_SUBSYSTEM:
		{
			ship_subsys *ssp;

			// shipnum could be -1 depending on if the ship hasn't arrived or died.  only look for subsystem
			// destroyed when shipnum is valid
			sindex = ship_name_lookup( aigp->target_name );

			// can't determine the status of this goal if ship not valid
			// or we haven't found a valid subsystem index yet
			if ( (sindex == -1) || (aigp->flags & AIGF_SUBSYS_NEEDS_FIXUP) ) {
				status = 0;
				break;
			}

			// if the ship is not in the mission or the subsystem name is still being stored, mark the status
			// as 0 so we can continue.  (The subsystem name must be turned into an index into the ship's subsystems
			// for this goal to be valid).
			Assert ( aigp->ai_submode >= 0 );
			ssp = ship_get_indexed_subsys( &Ships[sindex], aigp->ai_submode );
			if (ssp != NULL) {
				status = mission_log_get_time( LOG_SHIP_SUBSYS_DESTROYED, aigp->target_name, ssp->system_info->subobj_name, NULL );
			} else {
				// not supposed to ever happen, but could if there is a mismatch between the table and model subsystems
				nprintf(("AI", "Couldn't find subsystem %d for ship %s\n", aigp->ai_submode, Ships[sindex].ship_name));
				status = 0;
			}
			break;
		}

		case AI_GOAL_DISABLE_SHIP:
			status = mission_log_get_time( LOG_SHIP_DISABLED, aigp->target_name, NULL, NULL );
			break;

		case AI_GOAL_DISARM_SHIP:
			status = mission_log_get_time( LOG_SHIP_DISARMED, aigp->target_name, NULL, NULL );
			break;

		// to guard or ignore a ship, the goal cannot continue if the ship being guarded is either destroyed
		// or has departed.
		case AI_GOAL_CHASE:
		case AI_GOAL_GUARD:
		case AI_GOAL_IGNORE:
		case AI_GOAL_IGNORE_NEW:
		case AI_GOAL_EVADE_SHIP:
		case AI_GOAL_STAY_NEAR_SHIP:
		case AI_GOAL_FLY_TO_SHIP:
		case AI_GOAL_REARM_REPAIR:
		{
			// MWA -- 4/22/98.  Check for the ship actually being in the mission before
			// checking departure and destroyed.  In multiplayer, since ships can respawn,
			// they get log entries for being destroyed even though they have respawned.
			sindex = ship_name_lookup( aigp->target_name );
			if ( sindex < 0 ) {
				status = mission_log_get_time( LOG_SHIP_DEPARTED, aigp->target_name, NULL, NULL);
				if ( !status ) {
					status = mission_log_get_time( LOG_SHIP_DESTROYED, aigp->target_name, NULL, NULL);
					if ( !status ) {
						status = mission_log_get_time( LOG_SELF_DESTRUCTED, aigp->target_name, NULL, NULL);
					}
				}

				if ( status )
					return_val = AI_GOAL_NOT_ACHIEVABLE;
			} else {
				status = 0;
			}

			break;
		}

		case AI_GOAL_CHASE_WING:
		case AI_GOAL_GUARD_WING:
		{
			status = mission_log_get_time( LOG_WING_DEPARTED, aigp->target_name, NULL, NULL );
			if ( !status ) {
				status = mission_log_get_time( LOG_WING_DESTROYED, aigp->target_name, NULL, NULL);
				if ( status )
					return_val = AI_GOAL_NOT_ACHIEVABLE;
			}

			break;
		}

		// the following case statement returns control to caller on all paths!!!!
		case AI_GOAL_CHASE_WEAPON:
		{
			// for chase weapon, we simply need to look at the weapon instance that we are trying to
			// attack and see if the object still exists, and has the same signature that we expect.
			Assert( aigp->target_instance >= 0 );

			if ( Weapons[aigp->target_instance].objnum == -1 )
				return AI_GOAL_NOT_ACHIEVABLE;

			// if the signatures don't match, then goal isn't achievable.
			if ( Objects[Weapons[aigp->target_instance].objnum].signature != aigp->target_signature )
				return AI_GOAL_NOT_ACHIEVABLE;

			// otherwise, we should be good to go
			return AI_GOAL_ACHIEVABLE;

			break;
		}

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
		sindex = wing_name_lookup( aigp->target_name );

		if (sindex < 0)
			return AI_GOAL_NOT_KNOWN;

		wing *wingp = &Wings[sindex];

		if ( wingp->flags & WF_WING_GONE )
			return AI_GOAL_NOT_ACHIEVABLE;
		else if ( wingp->total_arrived_count == 0 )
			return AI_GOAL_NOT_KNOWN;
		else
			return AI_GOAL_ACHIEVABLE;
	}
	// Goober5000 - undocking from an unspecified object is always achievable;
	// undocking from a specified object is handled below
	else if ( (aigp->ai_mode == AI_GOAL_UNDOCK) && (aigp->target_name == NULL) )
	{
			return AI_GOAL_ACHIEVABLE;
	}
	else
	{
		// goal ship is currently in mission
		if ( ship_name_lookup( aigp->target_name ) != -1 )
		{
			status = SHIP_STATUS_ARRIVED;
		}
		// goal ship is still on the arrival list
		else if ( mission_parse_get_arrival_ship(aigp->target_name) )
		{
			status = SHIP_STATUS_NOT_ARRIVED;
		}
		// goal ship has left the area
		else if ( ship_find_exited_ship_by_name(aigp->target_name) != -1 )
		{
			status = SHIP_STATUS_GONE;
		}
		else
		{
			Int3();		// get ALLENDER
			status = SHIP_STATUS_UNKNOWN;
		}
	}

	// Goober5000 - before doing anything else, check if this is a disarm goal for an arrived ship...
	if ((status == SHIP_STATUS_ARRIVED) && (aigp->ai_mode == AI_GOAL_DISARM_SHIP))
	{
		// if the ship has no turrets, we can't disarm it!
		if (Ships[ship_name_lookup(aigp->target_name)].subsys_info[SUBSYSTEM_TURRET].type_count == 0)
			return AI_GOAL_NOT_ACHIEVABLE;
	}

	// if the goal is an ignore/disable/disarm goal, then 
	// Goober5000 - see note at PURGE_GOALS_ALL_SHIPS... this is bizarre
	if ((status == SHIP_STATUS_ARRIVED) && !(aigp->flags & AIGF_GOALS_PURGED))
	{
		if (aigp->ai_mode & PURGE_GOALS_ALL_SHIPS) {
			ai_goal_purge_all_invalid_goals(aigp);
			aigp->flags |= AIGF_GOALS_PURGED;
		}
		else if (aigp->ai_mode & PURGE_GOALS_ONE_SHIP) {
			ai_goal_purge_invalid_goals(aigp, aip->goals, aip, -1);
			aigp->flags |= AIGF_GOALS_PURGED;
		}
	}	

	// if we are docking, validate the docking indices on both ships.  We might have to change names to indices.
	// only enter this calculation if the ship we are docking with has arrived.  If the ship is gone, then
	// this goal will get removed.
	if ( (aigp->ai_mode == AI_GOAL_DOCK) && (status == SHIP_STATUS_ARRIVED) ) {
		if (!(aigp->flags & AIGF_DOCKER_INDEX_VALID)) {
			modelnum = Ship_info[Ships[objp->instance].ship_info_index].model_num;
			Assert( modelnum >= 0 );
			index = model_find_dock_name_index(modelnum, aigp->docker.name);
			aigp->docker.index = index;
			aigp->flags |= AIGF_DOCKER_INDEX_VALID;
		}

		if (!(aigp->flags & AIGF_DOCKEE_INDEX_VALID)) {
			sindex = ship_name_lookup(aigp->target_name);
			if ( sindex != -1 ) {
				modelnum = Ship_info[Ships[sindex].ship_info_index].model_num;
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
		sindex = ship_name_lookup(aigp->target_name);
		Assert( sindex >= 0 );
		object *goal_objp = &Objects[Ships[sindex].objnum];

		// if the ship that I am supposed to dock with is docked with something else, then I need to put my goal on hold
		//	[MK, 4/23/98: With Mark, we believe this fixes the problem of Comet refusing to warp out after docking with Omega.
		//	This bug occurred only when mission goals were validated in the frame in which Comet docked, which happened about
		// once in 10-20 tries.]
		if ( object_is_docked(goal_objp) )
		{
			// if the dockpoint I need to dock to is occupied by someone other than me
			object *obstacle_objp = dock_find_object_at_dockpoint(goal_objp, aigp->dockee.index);
			if (obstacle_objp == NULL)
			{
				// nobody in the way... we're good
			}
			else if (obstacle_objp != objp)
			{
				// return NOT_KNOWN which will place the goal on hold until the dockpoint is clear
				return AI_GOAL_NOT_KNOWN;
			}
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
				// if this goal isn't on hold yet, then issue the undock goal
				if ( !(aigp->flags & AIGF_GOAL_ON_HOLD) )
					ai_add_goal_ship_internal( aip, AI_GOAL_UNDOCK, Ships[obstacle_objp->instance].ship_name, -1, -1, 0 );

				// return NOT_KNOWN which will place the goal on hold until the undocking is complete.
				return AI_GOAL_NOT_KNOWN;
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
			if (aip->goal_objnum != Ships[ship_name_lookup(aigp->target_name)].objnum)
				return AI_GOAL_NOT_KNOWN;
		}

	} else if ( (aigp->ai_mode == AI_GOAL_DESTROY_SUBSYSTEM) && (status == SHIP_STATUS_ARRIVED) ) {
		// if the ship has arrived, and the goal is destroy subsystem, then check to see that we
		// have fixed up the subsystem name (of the subsystem to destroy) into an index into
		// the ship's subsystem list
		if ( aigp->flags & AIGF_SUBSYS_NEEDS_FIXUP ) {
			sindex = ship_name_lookup( aigp->target_name );

			if ( sindex != -1 ) {
				aigp->ai_submode = ship_get_subsys_index( &Ships[sindex], aigp->docker.name );
				aigp->flags &= ~AIGF_SUBSYS_NEEDS_FIXUP;
			} else {
				Int3();
				return AI_GOAL_NOT_ACHIEVABLE;			// force this goal to be invalid
			}
		}
	} else if ( ((aigp->ai_mode == AI_GOAL_IGNORE) || (aigp->ai_mode == AI_GOAL_IGNORE_NEW)) && (status == SHIP_STATUS_ARRIVED) ) {
		object *ignored;

		// for ignoring a ship, call the ai_ignore object function, then declare the goal satisfied
		sindex = ship_name_lookup( aigp->target_name );
		Assert( sindex != -1 );		// should be true because of above status
		ignored = &Objects[Ships[sindex].objnum];

		ai_ignore_object(objp, ignored, (aigp->ai_mode == AI_GOAL_IGNORE_NEW));

		return AI_GOAL_SATISFIED;
	}

	switch ( aigp->ai_mode )
	{
		case AI_GOAL_CHASE:
		case AI_GOAL_CHASE_WING:
		case AI_GOAL_DOCK:
		case AI_GOAL_UNDOCK:
		case AI_GOAL_GUARD:
		case AI_GOAL_GUARD_WING:
		case AI_GOAL_DISABLE_SHIP:
		case AI_GOAL_DISARM_SHIP:
		case AI_GOAL_DESTROY_SUBSYSTEM:
		case AI_GOAL_IGNORE:
		case AI_GOAL_IGNORE_NEW:
		case AI_GOAL_EVADE_SHIP:
		case AI_GOAL_STAY_NEAR_SHIP:
		case AI_GOAL_FLY_TO_SHIP:
		{
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
		}

		// for rearm repair ships, a goal is only achievable if the support ship isn't repairing anything
		// else at the time, or is set to repair the ship for this goal.  All other goals should be placed
		// on hold by returning GOAL_NOT_KNOWN.
		case AI_GOAL_REARM_REPAIR:
		{
			// short circuit a couple of cases.  Ship not arrived shouldn't happen.  Ship gone means
			// we mark the goal as not achievable.
			if ( status == SHIP_STATUS_NOT_ARRIVED ) {
				Int3();										// get Allender.  this shouldn't happen!!!
				return AI_GOAL_NOT_ACHIEVABLE;
			}

			if ( status == SHIP_STATUS_GONE )
				return AI_GOAL_NOT_ACHIEVABLE;

			sindex = ship_name_lookup( aigp->target_name );

			if ( sindex < 0 ) {
				Int3();
				return AI_GOAL_NOT_ACHIEVABLE;
			}

			// if desitnation currently being repaired, then goal is stil active
			if ( Ai_info[Ships[sindex].ai_index].ai_flags & AIF_BEING_REPAIRED )
				return AI_GOAL_ACHIEVABLE;

			// if the destination ship is dying or departing (but not completed yet), the mark goal as
			// not achievable.
			if ( Ships[sindex].flags & (SF_DYING | SF_DEPARTING) )
				return AI_GOAL_NOT_ACHIEVABLE;

			// if the destination object is no longer awaiting repair, then remove the item
			if ( !(Ai_info[Ships[sindex].ai_index].ai_flags & AIF_AWAITING_REPAIR) )
				return AI_GOAL_NOT_ACHIEVABLE;

			// not repairing anything means that he can do this goal!!!
			if ( !(aip->ai_flags  & AIF_REPAIRING) )
				return AI_GOAL_ACHIEVABLE;

			// test code!!!
			if ( aip->goal_objnum == -1 ) {
				return AI_GOAL_ACHIEVABLE;
			}

			// if he is repairing something, he can satisfy his repair goal (his goal_objnum)
			// return GOAL_NOT_KNOWN which is kind of a hack which puts the goal on hold until it can be
			// satisfied.  
			if ( aip->goal_objnum != Ships[sindex].objnum )
				return AI_GOAL_NOT_KNOWN;

			return AI_GOAL_ACHIEVABLE;
		}

		default:
			Int3();			// invalid case in switch:
	}

	return AI_GOAL_NOT_KNOWN;
}

//	Compare function for sorting ai_goals based on priority.
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

	// check based on time goal was issued

	if ( ga->time > gb->time )
		return -1;
	// V had this check commented out and would always return 1 here, that messes up where multiple goals 
	// get assigned at the same time though, when the priorities are also the same (Enif station bug) - taylor
	else if ( ga->time < gb->time )
		return 1;

	// the two are equal
	return 0;
}

//	Prioritize goal list.
//	First sort on priority.
//	Then sort on time for goals of equivalent priority.
//	*aip		The AI info to act upon.  Goals are stored at aip->goals
void prioritize_goals(ai_info *aip)
{
	//	First sort based on priority field.
	insertion_sort(aip->goals, MAX_AI_GOALS, sizeof(ai_goal), ai_goal_priority_compare);
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
/*
static char *Goal_text[5] = {
"EVENT_SHIP",
"EVENT_WING",
"PLAYER_SHIP",
"PLAYER_WING",
"DYNAMIC",
};
*/
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
	prioritize_goals(aip);

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

	if ( MULTIPLAYER_MASTER ){
		send_ai_info_update_packet( objp, AI_UPDATE_ORDERS );
	}

	// if this object was flying in formation off of another object, remove the flag that tells him
	// to do this.  The form-on-my-wing command is removed from the goal list as soon as it is called, so
	// we are safe removing this bit here.
	aip->ai_flags &= ~AIF_FORMATION_OBJECT;

	// Goober5000 - we may want to use AI for the player
	// AL 3-7-98: If this is a player ship, and the goal is not a formation goal, then do a quick out
	if ( !(Player_use_ai) && (objp->flags & OF_PLAYER_SHIP) && (current_goal->ai_mode != AI_GOAL_FORM_ON_WING) )
	{
		return;
	}	



	switch ( current_goal->ai_mode ) {

	case AI_GOAL_CHASE:
		if ( current_goal->target_name ) {
			shipnum = ship_name_lookup( current_goal->target_name );
			Assert (shipnum != -1 );			// shouldn't get here if this is false!!!!
			other_obj = &Objects[Ships[shipnum].objnum];
		} else
			other_obj = NULL;						// we get this case when we tell ship to engage enemy!

		//	Mike -- debug code!
		//	If a ship has a subobject on it, attack that instead of the main ship!
		ai_attack_object( objp, other_obj, NULL);
		break;

	case AI_GOAL_CHASE_WEAPON:
		Assert( Weapons[current_goal->target_instance].objnum >= 0 );
		other_obj = &Objects[Weapons[current_goal->target_instance].objnum];
		Assert( other_obj->signature == current_goal->target_signature );
		ai_attack_object( objp, other_obj, NULL );
		break;

	case AI_GOAL_GUARD:
		shipnum = ship_name_lookup( current_goal->target_name );
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
		wingnum = wing_name_lookup( current_goal->target_name );
		Assert (wingnum != -1 );			// shouldn't get here if this is false!!!!
		ai_set_guard_wing(objp, wingnum);
		aip->submode_start_time = Missiontime;
		break;

	case AI_GOAL_WAYPOINTS:				// do nothing for waypoints
	case AI_GOAL_WAYPOINTS_ONCE: {
		int flags = 0;

		if ( current_goal->ai_mode == AI_GOAL_WAYPOINTS)
			flags |= WPF_REPEAT;
		ai_start_waypoints(objp, current_goal->wp_list, flags);
		break;
	}

	case AI_GOAL_FLY_TO_SHIP:
		shipnum = ship_name_lookup( current_goal->target_name );
		Assert (shipnum != -1 );			// shouldn't get here if this is false!!!!
		ai_start_fly_to_ship(objp, shipnum);
		break;

	case AI_GOAL_DOCK: {
		shipnum = ship_name_lookup( current_goal->target_name );
		Assert (shipnum != -1 );			// shouldn't get here if this is false!!!!
		other_obj = &Objects[Ships[shipnum].objnum];

		// be sure that we have indices for docking points here!  If we ever had names, they should
		// get fixed up in goal_achievable so that the points can be checked there for validity
		Assert (current_goal->flags & AIGF_DOCK_INDEXES_VALID);
		ai_dock_with_object( objp, current_goal->docker.index, other_obj, current_goal->dockee.index, AIDO_DOCK );
		break;
	}

	case AI_GOAL_UNDOCK:
		// try to find the object which which this object is docked with.  Use that object as the
		// "other object" for the undocking proceedure.  If "other object" isn't found, then the undock
		// goal cannot continue.  Spit out a warning and remove the goal.

		// Goober5000 - do we have a specific ship to undock from?
		if ( current_goal->target_name != NULL )
		{
			shipnum = ship_name_lookup( current_goal->target_name );

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
				current_goal->target_name = Ships[other_obj->instance].ship_name;
			}
			// hmm, nobody exists that we can undock from
			else
			{
				other_obj = NULL;
			}
		}

		if ( other_obj == NULL ) {
			// assume that the guy he was docked with doesn't exist anymore.  (i.e. a cargo containuer
			// can get destroyed while docked with a freighter.)  We should just remove this goal and
			// let this ship pick up it's next goal.
			ai_mission_goal_complete( aip );		// mark as complete, so we can remove it and move on!!!
			break;
		}

		// Goober5000 - Sometimes a ship will be assigned a new goal before it can finish undocking.  Later,
		// when the ship returns to this goal, it will try to resume undocking from a ship it's not attached
		// to.  If this happens, remove the goal as above.
		if (!dock_check_find_direct_docked_object(objp, other_obj))
		{
			ai_mission_goal_complete( aip );
			break;
		}

		// passing 0, 0 is okay because the undock code will figure out where to undock from
		ai_dock_with_object( objp, 0, other_obj, 0, AIDO_UNDOCK );
		break;


		// when destroying a subsystem, we can destroy a specific instance of a subsystem
		// or all instances of a type of subsystem (i.e. a specific engine or all engines).
		// the ai_submode value is > 0 for a specific instance of subsystem and < 0 for all
		// instances of a specific type
	case AI_GOAL_DESTROY_SUBSYSTEM:
	case AI_GOAL_DISABLE_SHIP:
	case AI_GOAL_DISARM_SHIP: {
		shipnum = ship_name_lookup( current_goal->target_name );
		Assert( shipnum >= 0 );
		other_obj = &Objects[Ships[shipnum].objnum];
		ai_attack_object( objp, other_obj, NULL);
		ai_set_attack_subsystem( objp, current_goal->ai_submode );		// submode stored the subsystem type
		if (current_goal->ai_mode != AI_GOAL_DESTROY_SUBSYSTEM) {
			if (aip->target_objnum != -1) {
				//	Only protect if _not_ a capital ship.  We don't want the Lucifer accidentally getting protected.
				if (Ship_types[Ship_info[Ships[shipnum].ship_info_index].class_type].ai_bools & STI_AI_PROTECTED_ON_CRIPPLE)
					Objects[aip->target_objnum].flags |= OF_PROTECTED;
			}
		} else	//	Just in case this ship had been protected, unprotect it.
			if (aip->target_objnum != -1)
				Objects[aip->target_objnum].flags &= ~OF_PROTECTED;

		break;
	}

	case AI_GOAL_CHASE_WING:
		wingnum = wing_name_lookup( current_goal->target_name );
		Assert( wingnum >= 0 );
		ai_attack_wing(objp, wingnum);
		break;

	case AI_GOAL_CHASE_ANY:
		ai_attack_object( objp, NULL, NULL );
		break;

	case AI_GOAL_WARP: {
		mission_do_departure( objp, true );
		break;
	}

	case AI_GOAL_EVADE_SHIP:
		shipnum = ship_name_lookup( current_goal->target_name );
		Assert( shipnum >= 0 );
		other_obj = &Objects[Ships[shipnum].objnum];
		ai_evade_object( objp, other_obj);
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
		aip->submode_start_time = Missiontime;
		break;

	case AI_GOAL_FORM_ON_WING:
		// for form on wing, we need to clear out all goals for this ship, and then call the form
		// on wing AI code
		// clearing out goals is okay here since we are now what mode to set this AI object to.
		ai_clear_ship_goals( aip );
		shipnum = ship_name_lookup( current_goal->target_name );
		Assert( shipnum >= 0 );
		other_obj = &Objects[Ships[shipnum].objnum];
		ai_form_on_wing( objp, other_obj );
		break;

// labels for support ship commands

	case AI_GOAL_STAY_NEAR_SHIP: {
		shipnum = ship_name_lookup( current_goal->target_name );
		Assert( shipnum >= 0 );
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
		shipnum = ship_name_lookup( current_goal->target_name );
		Assert( shipnum >= 0 );
		other_obj = &Objects[Ships[shipnum].objnum];
		ai_rearm_repair( objp, current_goal->docker.index, other_obj, current_goal->dockee.index );
		break;

	default:
		Int3();
		break;
	}

}

void ai_update_goal_references(ai_goal *goals, int type, const char *old_name, const char *new_name)
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
					case AI_GOAL_IGNORE_NEW:
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
			if (!stricmp(goals[i].target_name, old_name)) {
				if (*new_name == '<')  // target was just deleted..
					goals[i].ai_mode = AI_GOAL_NONE;
				else
					goals[i].target_name = ai_get_goal_target_name(new_name, &dummy);
			}
	}
}

int query_referenced_in_ai_goals(ai_goal *goals, int type, const char *name)
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
					case AI_GOAL_IGNORE_NEW:
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
			if (!stricmp(goals[i].target_name, name))
				return 1;
		}
	}

	return 0;
}

char *ai_add_dock_name(const char *str)
{
	char *ptr;
	int i;

	Assert(strlen(str) <= NAME_LENGTH - 1);
	for (i=0; i<Num_ai_dock_names; i++)
		if (!stricmp(Ai_dock_names[i], str))
			return Ai_dock_names[i];

	Assert(Num_ai_dock_names < MAX_AI_DOCK_NAMES);
	ptr = Ai_dock_names[Num_ai_dock_names++];
	strcpy(ptr, str);
	return ptr;
}
