/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/AWACS.cpp $
 * $Revision: 2.9 $
 * $Date: 2004-07-12 16:33:05 $
 * $Author: Kazan $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.8  2004/03/05 09:01:52  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.7  2004/01/30 07:39:06  Goober5000
 * whew - I just went through all the code I ever added (or at least, that I could
 * find that I commented with a Goober5000 tag) and added a bunch of Asserts
 * and error-checking
 * --Goober5000
 *
 * Revision 2.6  2003/04/29 01:03:21  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.5  2003/01/18 09:25:40  Goober5000
 * fixed bug I inadvertently introduced by modifying SIF_ flags with sexps rather
 * than SF_ flags
 * --Goober5000
 *
 * Revision 2.4  2003/01/03 21:58:07  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 2.3  2002/12/27 02:57:50  Goober5000
 * removed the existing stealth sexps and replaced them with the following...
 * ship-stealthy
 * ship-unstealthy
 * is-ship-stealthy
 * friendly-stealth-invisible
 * friendly-stealth-visible
 * is-friendly-stealth-visible
 * --Goober5000
 *
 * Revision 2.2  2002/12/23 05:18:52  Goober5000
 * Squashed some Volition bugs! :O Some of the sexps for dealing with more than
 * one ship would return after only dealing with the first ship.
 *
 * Also added the following sexps:
 * is-ship-stealthed
 * ship-force-stealth
 * ship-force-nostealth
 * ship-remove-stealth-forcing
 *
 * They toggle the stealth flag on and off.  If a ship is forced stealthy, it won't even
 * show up for friendly ships.
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/13 21:09:29  mharris
 * I think the last of the networking code has ifndef NO_NETWORK...
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 13    8/25/99 10:50a Dave
 * Added music to the mainhall.tbl
 * 
 * 12    7/19/99 12:02p Andsager
 * Allow AWACS on any ship subsystem. Fix sexp_set_subsystem_strength to
 * only blow up subsystem if its strength is > 0
 * 
 * 11    7/06/99 10:45a Andsager
 * Modify engine wash to work on any ship that is not small.  Add AWACS
 * ask for help.
 * 
 * 10    6/17/99 1:41p Andsager
 * Added comments to team_visibility_update
 * 
 * 9     6/15/99 4:01p Jamesa
 * Don't count navbuoys and cargo containers towards team visibilitiy.
 * 
 * 8     6/02/99 12:52p Andsager
 * Added team-wide ship visibility.  Implemented for player.
 * 
 * 7     5/28/99 3:14p Andsager
 * Modify nebula scan range by species.
 * 
 * 6     5/28/99 9:39a Andsager
 * Modify awacs to take account for huge ships.
 * 
 * 5     5/12/99 2:55p Andsager
 * Implemented level 2 tag as priority in turret object selection
 * 
 * 4     1/25/99 2:49p Anoop
 * Put in more sanity checks when checking for AWACS sources.
 * 
 * 3     1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 2     1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 
 * $NoKeywords: $
 */

#include "ship/awacs.h"
#include "io/timer.h"
#include "ship/ship.h"
#include "globalincs/linklist.h"
#include "nebula/neb.h"
#include "mission/missionparse.h"
#include "network/multi.h"

// memory tracking - ALWAYS INCLUDE LAST
#include "mcd/mcd.h"

// ----------------------------------------------------------------------------------------------------
// AWACS DEFINES/VARS
//

// timestamp for updating AWACS stuff
#define AWACS_STAMP_TIME			1000
int Awacs_stamp = -1;

// total awacs levels for all teams
float Awacs_team[MAX_TEAMS];	// total AWACS capabilities for each team
float Awacs_level;				// Awacs_friendly - Awacs_hostile

// list of all AWACS sources
#define MAX_AWACS					30
typedef struct awacs_entry {
	int team;
	ship_subsys *subsys;
	object *objp;
} awacs_entry;
awacs_entry Awacs[MAX_AWACS];
int Awacs_count = 0;

// species dependent factor increasing scan range in nebula
#define AWACS_SHIVAN_MULT		1.50
#define AWACS_VASUDAN_MULT		1.25
#define AWACS_TERRAN_MULT		1.00

// TEAM SHIP VISIBILITY
// team-wide shared visibility info
// at start of each frame (maybe timestamp), compute visibility 
ubyte Team_friendly_visibility[MAX_SHIPS];
ubyte Team_neutral_visibility[MAX_SHIPS];
ubyte Team_hostile_visibility[MAX_SHIPS];
// no shared team visibility info for TEAM_UNKNOWN or TEAM_TRAITOR

// ----------------------------------------------------------------------------------------------------
// AWACS FORWARD DECLARATIONS
//

// update the total awacs levels
void awacs_update_all_levels();

// update team visibility info
void team_visibility_update();


// ----------------------------------------------------------------------------------------------------
// AWACS FUNCTIONS
//

// call when initializing level, before parsing mission
void awacs_level_init()
{
	// set the update timestamp to -1 
	Awacs_stamp = -1;
}

// call every frame to process AWACS details
void awacs_process()
{
	// if we need to update total AWACS levels, do so now
	if((Awacs_stamp == -1) || timestamp_elapsed(Awacs_stamp)){
		// reset the timestamp
		Awacs_stamp = timestamp(AWACS_STAMP_TIME);

		// recalculate everything
		awacs_update_all_levels();

		// update team visibility
		team_visibility_update();
	}
}


// ----------------------------------------------------------------------------------------------------
// AWACS FORWARD DEFINITIONS
//

// update the total awacs levels
void awacs_update_all_levels()
{
	ship_obj *moveup;	
	ship *shipp;
	ship_subsys *ship_system;
	int idx;

	// zero all levels
	Awacs_level = 0.0f;
	for(idx=0; idx<MAX_TEAMS; idx++){
		Awacs_team[idx] = 0.0f;
	}

	Awacs_count = 0;

	// we need to traverse all subsystems on all ships	
	for (moveup = GET_FIRST(&Ship_obj_list); moveup != END_OF_LIST(&Ship_obj_list); moveup = GET_NEXT(moveup)) {
		// make sure its a valid ship
		if((Objects[moveup->objnum].type != OBJ_SHIP) || (Objects[moveup->objnum].instance < 0)){
			continue;
		}	
		
		// get a handle to the ship
		shipp = &Ships[Objects[moveup->objnum].instance];

		// ignore dying, departing, or arriving ships
		if((shipp->flags & SF_DYING) || (shipp->flags & SF_DEPARTING) || (shipp->flags & SF_ARRIVING)){
			continue;
		}

		// only look at ships that have awacs subsystems
		if ( !(Ship_info[shipp->ship_info_index].flags & SIF_HAS_AWACS) ) {
			continue;
		}

		// traverse all subsystems
		for(ship_system = GET_FIRST(&shipp->subsys_list); ship_system != END_OF_LIST(&shipp->subsys_list); ship_system = GET_NEXT(ship_system)){
			// if this is an AWACS subsystem
			if((ship_system->system_info != NULL) && (ship_system->system_info->flags & MSS_FLAG_AWACS)){
				// add the intensity to the team total
				Awacs_team[shipp->team] += ship_system->awacs_intensity * (ship_system->current_hits / ship_system->max_hits);

				// add an Awacs source
				if(Awacs_count < MAX_AWACS){
					Awacs[Awacs_count].subsys = ship_system;
					Awacs[Awacs_count].team = shipp->team;
					Awacs[Awacs_count].objp = &Objects[moveup->objnum];				
					Awacs_count++;
				}
			}
		}
	}

	// awacs level
	Awacs_level = Awacs_team[TEAM_FRIENDLY] - Awacs_team[TEAM_HOSTILE];

	// spew all the info
#ifndef NDEBUG 
	/*
	for(idx=0; idx<MAX_TEAMS; idx++){
		nprintf(("General", "Team %d AWACS == %f\n", idx, Awacs_team[idx]));
	}
	nprintf(("General", "AWACS level == %f\n", Awacs_level));
	*/
#endif
}

// get the total AWACS level for target to viewer
// < 0.0f		: untargetable
// 0.0 - 1.0f	: marginally targetable
// >= 1.0f			: fully targetable as normal
float awacs_get_level(object *target, ship *viewer, int use_awacs)
{
	Assert(target);	// Goober5000
	Assert(viewer);	// Goober5000

	vector dist_vec, subsys_pos;
	float closest = 0.0f;
	float test;
	int closest_index = -1;
	int idx, friendly_invisible;
	ship *shipp;
	ship_info *sip;

	int viewer_has_primitive_sensors = (viewer->flags2 & SF2_PRIMITIVE_SENSORS);

	// calc distance from viewer to target
	vm_vec_sub(&dist_vec, &target->pos, &Objects[viewer->objnum].pos);
	int distance = (int) vm_vec_mag_quick(&dist_vec);

// redone by Goober5000
#define ALWAYS_TARGETABLE		1.5f
#define MARGINALLY_TARGETABLE	0.5f
#define UNTARGETABLE			-1.0f
#define FULLY_TARGETABLE		(viewer_has_primitive_sensors ? ((distance < viewer->primitive_sensor_range) ? MARGINALLY_TARGETABLE : UNTARGETABLE) : ALWAYS_TARGETABLE)


#ifndef NO_NETWORK
	// if the viewer is me, and I'm a multiplayer observer, its always viewable
	if((viewer == Player_ship) && (Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && MULTI_OBSERVER(Net_players[MY_NET_PLAYER_NUM])){
		return ALWAYS_TARGETABLE;
	}
#endif

	// Goober5000
	shipp = &Ships[target->instance];
	sip = &Ship_info[shipp->ship_info_index];
	friendly_invisible = (shipp->flags2 & SF2_STEALTH) && (shipp->flags2 & SF2_FRIENDLY_STEALTH_INVIS);
	
	int stealth_ship = (target->type == OBJ_SHIP) && (target->instance >= 0) && (shipp->flags2 & SF2_STEALTH);
	int nebula_mission = (The_mission.flags & MISSION_FLAG_FULLNEB);
	int check_huge_ship = (target->type == OBJ_SHIP) && (target->instance >= 0) && (sip->flags & SIF_HUGE_SHIP);

	// ships on the same team are always viewable
	// not necessarily now! :) -- Goober5000
	if((target->type == OBJ_SHIP) && (shipp->team == viewer->team) && (!friendly_invisible))
	{
		return FULLY_TARGETABLE;
	}

	// only check for Awacs if stealth ship or Nebula mission
	// determine the closest friendly awacs
	if ((stealth_ship || nebula_mission) && use_awacs) {
		for (idx=0; idx<Awacs_count; idx++) {
			// if not on the same team as the viewer
			if(Awacs[idx].team != viewer->team){
				continue;
			}

			// if this awacs source has somehow become invalid
			if(Awacs[idx].objp->type != OBJ_SHIP){
				continue;
			}

			// get the subsystem position
			if(!get_subsystem_pos(&subsys_pos, Awacs[idx].objp, Awacs[idx].subsys)){
				continue;
			}

			// determine if its the closest
			// special case for HUGE_SHIPS
			if ( check_huge_ship ) {
				// check if inside bbox expanded by awacs_radius
				if (check_world_pt_in_expanded_ship_bbox(&subsys_pos, target, Awacs[idx].subsys->awacs_radius)) {
					closest_index = idx;
					break;
				}

			// not a huge ship
			} else {
				// get distance from Subsys to target
				vm_vec_sub(&dist_vec, &subsys_pos, &target->pos);
				test = vm_vec_mag_quick(&dist_vec);

				if (test > Awacs[idx].subsys->awacs_radius) {
					continue;
				}
				if ((closest_index == -1) || (test < closest)) {
					closest = test;
					closest_index = idx;
					break;
				}
			}
		}
	}

	// check for a tagged ship. TAG'd ships are _always_ visible
	if(target->type == OBJ_SHIP){
		if(shipp->tag_left > 0.0f || shipp->level2_tag_left > 0.0f){
			return FULLY_TARGETABLE;
		}
	}
	
	// if this is a stealth ship
	if( stealth_ship ){
		// if the ship is within range of an awacs
		if(closest_index != -1){
			// if the nebula effect is active, stealth ships are only partially targetable
			if ( nebula_mission ) {
				return MARGINALLY_TARGETABLE;
			}

			// otherwise its targetable
			return FULLY_TARGETABLE;
		} 
		// otherwise its completely hidden
		else {
			return UNTARGETABLE;
		}
	}
	// all other ships
	else {
		// if this is not a nebula mission, its always targetable
		if( !nebula_mission ){
			return FULLY_TARGETABLE;
		}

		// if the ship is within range of an awacs, its fully targetable
		if(closest_index != -1){
			return FULLY_TARGETABLE;
		}

		// fully targetable at half the nebula value
		// modify distance by species
		float scan_nebula_range = Neb2_awacs;
		switch (Ship_info[viewer->ship_info_index].species) {
		case SPECIES_SHIVAN:
			scan_nebula_range *= float(AWACS_SHIVAN_MULT);
			break;

		case SPECIES_VASUDAN:
			scan_nebula_range *= float(AWACS_VASUDAN_MULT);
			break;

		case SPECIES_TERRAN:
			scan_nebula_range *= float(AWACS_TERRAN_MULT);
			break;

		default:
			Int3();
		}

		// special case for huge ship - check inside expanded bounding boxes
		if ( check_huge_ship ) {
			if (check_world_pt_in_expanded_ship_bbox(&Objects[viewer->objnum].pos, target, scan_nebula_range)) {
				if (check_world_pt_in_expanded_ship_bbox(&Objects[viewer->objnum].pos, target, MARGINALLY_TARGETABLE*scan_nebula_range)) {
					return FULLY_TARGETABLE;
				}
				return MARGINALLY_TARGETABLE;
			}
		} 
		// otherwise check straight up nebula numbers
		else {
			vm_vec_sub(&dist_vec, &target->pos, &Objects[viewer->objnum].pos);
			test = vm_vec_mag_quick(&dist_vec);
			if(test < (MARGINALLY_TARGETABLE * scan_nebula_range)){
				return FULLY_TARGETABLE;
			} else if(test < scan_nebula_range){
				return MARGINALLY_TARGETABLE;
			}
		}

		// untargetable at longer range
		return UNTARGETABLE;	
	}		

	Int3();
	return FULLY_TARGETABLE;
}


// update team visibility
#define NUM_TEAMS	5
void team_visibility_update()
{
	int friendly_count, hostile_count, neutral_count, unknown_count, traitor_count, num_stealth;
	int friendly_ships[MAX_SHIPS];
	int hostile_ships[MAX_SHIPS];
	int neutral_ships[MAX_SHIPS];
	int unknown_ships[MAX_SHIPS];
	int traitor_ships[MAX_SHIPS];
	ship_obj *moveup;
	ship *shipp;

	friendly_count = hostile_count = neutral_count = unknown_count = traitor_count = num_stealth = 0;

	// zero out visibility for each team
	memset(Team_friendly_visibility, 0, sizeof(Team_friendly_visibility));
	memset(Team_hostile_visibility,  0, sizeof(Team_hostile_visibility));
	memset(Team_neutral_visibility,  0, sizeof(Team_neutral_visibility));

	// Go through list of ships and mark those visible for given team
	for (moveup = GET_FIRST(&Ship_obj_list); moveup != END_OF_LIST(&Ship_obj_list); moveup = GET_NEXT(moveup)) {
		// make sure its a valid ship
		if ((Objects[moveup->objnum].type != OBJ_SHIP) || (Objects[moveup->objnum].instance < 0)) {
			continue;
		}	
		
		// get a handle to the ship
		shipp = &Ships[Objects[moveup->objnum].instance];

		// ignore dying, departing, or arriving ships
		if ((shipp->flags & SF_DYING) || (shipp->flags & SF_DEPARTING) || (shipp->flags & SF_ARRIVING)) {
			continue;
		}

		// check if ship if flagged as invisible
		if (shipp->flags & SF_HIDDEN_FROM_SENSORS) {
			continue;
		}

		int ship_num = shipp - Ships;
		Assert((ship_num >= 0) && (ship_num < MAX_SHIPS));

		switch (shipp->team) {
		case TEAM_FRIENDLY:	
			Team_friendly_visibility[ship_num] = 1;
			friendly_ships[friendly_count++] = ship_num;
			break;

		case TEAM_HOSTILE:	
			Team_hostile_visibility[ship_num] = 1;
			hostile_ships[hostile_count++] = ship_num;
			break;

		case TEAM_NEUTRAL:	
			Team_neutral_visibility[ship_num] = 1;
			neutral_ships[neutral_count++] = ship_num;
			break;

		case TEAM_UNKNOWN:
			unknown_ships[unknown_count++] = ship_num;
			break;

		case TEAM_TRAITOR:
			traitor_ships[traitor_count++] = ship_num;
			break;

		default:
			Int3();
		}
	}

	int team_count[NUM_TEAMS];
	int *team_ships[NUM_TEAMS];
	ubyte *team_visibility[NUM_TEAMS];
	int *cur_team_ships, *en_team_ships;

	// set up variable for loop
	// team count is number of ship of each team type
	team_count[0] = friendly_count;
	team_count[1] = hostile_count;
	team_count[2] = neutral_count;
	team_count[3] = unknown_count;
	team_count[4] = traitor_count;

	// team ships is array of ship_nums of ships from each team
	team_ships[0] = friendly_ships;
	team_ships[1] = hostile_ships;
	team_ships[2] = neutral_ships;
	team_ships[3] = unknown_ships;
	team_ships[4] = traitor_ships;

	// this is the result, visiblity for each team indexed by ship_num
	team_visibility[0] = Team_friendly_visibility;
	team_visibility[1] = Team_hostile_visibility;
	team_visibility[2] = Team_neutral_visibility;
	team_visibility[3] = NULL;
	team_visibility[4] = NULL;

	int idx, en_idx, cur_count, en_count, en_team;

	// Do for friendly, neutral and hostile (only teams that cooperate with visibility)
	for (int cur_team=0; cur_team<3; cur_team++) {
		// set up current team
		cur_count = team_count[cur_team];
		cur_team_ships = team_ships[cur_team];

		// short circuit if team has no presence
		if (cur_count == 0) {
			break;
		}

		// check agains both enemy teams
		for (int en_team_inc=1; en_team_inc<NUM_TEAMS; en_team_inc++) {
			// set up enemy team
			en_team = (cur_team + en_team_inc) % NUM_TEAMS;		// enemy_team (index) is cur_team + (1-4), wrapped back to range 0-4
			en_count = team_count[en_team];
			en_team_ships = team_ships[en_team];

			// check if current team can see enemy team's ships
			for (en_idx=0; en_idx<en_count; en_idx++) {
				// for each ship on other team
				for (idx=0; idx<cur_count; idx++) {
					// ignore nav buoys and cargo containers
					if(Ship_info[Ships[cur_team_ships[idx]].ship_info_index].flags & (SIF_CARGO | SIF_NAVBUOY)){
						continue;
					}

					// check against each ship on my team(and AWACS only once)
					if ( awacs_get_level(&Objects[Ships[en_team_ships[en_idx]].objnum], &Ships[cur_team_ships[idx]], idx==0) > 1) {
						team_visibility[cur_team][en_team_ships[en_idx]] = 1;
						break;
					}
				} //end cur_count (ship on current team looking at en_team)
			} // end en_count
		} // end en_team_inc
	} // end cur_team
}


// Determine is ship is visible by team
int ship_is_visible_by_team(int ship_num, int team)
{
	Assert((ship_num >= 0) && (ship_num < MAX_SHIPS));

	switch (team) {
	case TEAM_FRIENDLY:
		return Team_friendly_visibility[ship_num];
		break;

	case TEAM_HOSTILE:
		return Team_hostile_visibility[ship_num];
		break;

	case TEAM_NEUTRAL:
		return Team_neutral_visibility[ship_num];
		break;

	case TEAM_UNKNOWN:
		return 0;
		break;

	case TEAM_TRAITOR:
		return 0;
		break;

	default:
		Int3();
		return 0;
	}
}

// New version of determine is ship is visible by team - Goober5000
// accounts for friendly stealth invisible and primitive sensors
int ship_is_visible_by_team_new(object *target, ship *viewer)
{
	Assert(target);
	Assert(viewer);
	Assert(target->type == OBJ_SHIP);

	ship *target_shipp = &Ships[target->instance];

	// not visible if viewer has primitive sensors
	if (viewer->flags2 & SF2_PRIMITIVE_SENSORS)
	{
		return 0;
	}

	// friendly stealthed ships are not visible if they have the friendly-stealth-invisible
	// flag set
	if (target_shipp->team == viewer->team)
	{
		if (target_shipp->flags2 & SF2_STEALTH)
		{
			if (target_shipp->flags2 & SF2_FRIENDLY_STEALTH_INVIS)
			{
				return 0;
			}
		}
	}

	// now evaluate this the old way
	return ship_is_visible_by_team(target->instance, viewer->team);
}


		
