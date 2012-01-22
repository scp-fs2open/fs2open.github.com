/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "ship/awacs.h"
#include "io/timer.h"
#include "ship/ship.h"
#include "globalincs/linklist.h"
#include "nebula/neb.h"
#include "mission/missionparse.h"
#include "network/multi.h"
#include "species_defs/species_defs.h"
#include "iff_defs/iff_defs.h"


// ----------------------------------------------------------------------------------------------------
// AWACS DEFINES/VARS
//

// timestamp for updating AWACS stuff
#define AWACS_STAMP_TIME			1000
int Awacs_stamp = -1;

// total awacs levels for all teams
float Awacs_team[MAX_IFFS];	// total AWACS capabilities for each team
float Awacs_level;					// Awacs_friendly - Awacs_hostile

// list of all AWACS sources
#define MAX_AWACS					30
typedef struct awacs_entry {
	int team;
	ship_subsys *subsys;
	object *objp;
} awacs_entry;
awacs_entry Awacs[MAX_AWACS];
int Awacs_count = 0;

// TEAM SHIP VISIBILITY
// team-wide shared visibility info
// at start of each frame (maybe timestamp), compute visibility 
ubyte Ship_visibility_by_team[MAX_IFFS][MAX_SHIPS];

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
	if ((Awacs_stamp == -1) || timestamp_elapsed(Awacs_stamp))
	{
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
	for (idx=0; idx<MAX_IFFS; idx++)
		Awacs_team[idx] = 0.0f;

	Awacs_count = 0;

	// we need to traverse all subsystems on all ships	
	for (moveup = GET_FIRST(&Ship_obj_list); moveup != END_OF_LIST(&Ship_obj_list); moveup = GET_NEXT(moveup))
	{
		// make sure its a valid ship
		if ((Objects[moveup->objnum].type != OBJ_SHIP) || (Objects[moveup->objnum].instance < 0))
			continue;
		
		// get a handle to the ship
		shipp = &Ships[Objects[moveup->objnum].instance];

		// ignore dying, departing, or arriving ships
		if ((shipp->flags & SF_DYING) || (shipp->flags & SF_DEPARTING) || (shipp->flags & SF_ARRIVING))
			continue;

		// only look at ships that have awacs subsystems
		if (!(Ship_info[shipp->ship_info_index].flags & SIF_HAS_AWACS))
			continue;

		// traverse all subsystems
		for (ship_system = GET_FIRST(&shipp->subsys_list); ship_system != END_OF_LIST(&shipp->subsys_list); ship_system = GET_NEXT(ship_system))
		{
			// if this is an AWACS subsystem
			if ((ship_system->system_info != NULL) && (ship_system->system_info->flags & MSS_FLAG_AWACS))
			{
				// add the intensity to the team total
				Awacs_team[shipp->team] += ship_system->awacs_intensity * (ship_system->current_hits / ship_system->max_hits);

				// add an Awacs source
				if (Awacs_count < MAX_AWACS)
				{
					Awacs[Awacs_count].subsys = ship_system;
					Awacs[Awacs_count].team = shipp->team;
					Awacs[Awacs_count].objp = &Objects[moveup->objnum];				
					Awacs_count++;
				}
			}
		}
	}

	// Goober5000 - Awacs_level isn't used anywhere that I can see
	// awacs level
	//Awacs_level = Awacs_team[TEAM_FRIENDLY] - Awacs_team[TEAM_HOSTILE];

	// spew all the info
#ifndef NDEBUG 
	/*
	for (idx=0; idx<MAX_TVT_TEAMS; idx++){
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

	vec3d dist_vec, subsys_pos;
	float closest = 0.0f;
	float test;
	int closest_index = -1;
	int idx, stealth_ship = 0, check_huge_ship = 0, friendly_stealth_invisible = 0;
	ship *shipp = NULL;
	ship_info *sip = NULL;

	int viewer_has_primitive_sensors = (viewer->flags2 & SF2_PRIMITIVE_SENSORS);

	// calc distance from viewer to target
	vm_vec_sub(&dist_vec, &target->pos, &Objects[viewer->objnum].pos);
	int distance = (int) vm_vec_mag_quick(&dist_vec);

// redone by Goober5000
#define ALWAYS_TARGETABLE		1.5f
#define MARGINALLY_TARGETABLE	0.5f
#define UNTARGETABLE			-1.0f
#define FULLY_TARGETABLE		(viewer_has_primitive_sensors ? ((distance < viewer->primitive_sensor_range) ? MARGINALLY_TARGETABLE : UNTARGETABLE) : ALWAYS_TARGETABLE)

	// if the viewer is me, and I'm a multiplayer observer, its always viewable
	if ((viewer == Player_ship) && (Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && MULTI_OBSERVER(Net_players[MY_NET_PLAYER_NUM]))
		return ALWAYS_TARGETABLE;

	// check the targeting threshold
	if ((Hud_max_targeting_range > 0) && (distance > Hud_max_targeting_range)) {
		return UNTARGETABLE;
	}

	if (target->type == OBJ_SHIP) {
		// if no valid target then bail as never viewable
		if (target->instance < 0)
			return UNTARGETABLE;

		// Goober5000
		shipp = &Ships[target->instance];
		sip = &Ship_info[shipp->ship_info_index];
		stealth_ship = (shipp->flags2 & SF2_STEALTH);
		friendly_stealth_invisible = (shipp->flags2 & SF2_FRIENDLY_STEALTH_INVIS);

		check_huge_ship = (sip->flags & SIF_HUGE_SHIP);
	}
	
	int nebula_enabled = (The_mission.flags & MISSION_FLAG_FULLNEB);

	// ships on the same team are always viewable
	if ((target->type == OBJ_SHIP) && (shipp->team == viewer->team))
	{
		// not necessarily now! -- Goober5000
		if ( !(stealth_ship && friendly_stealth_invisible) )
			return FULLY_TARGETABLE;
	}

	// check for a tagged ship. TAG'd ships are _always_ visible
	if (target->type == OBJ_SHIP)
	{
		Assert( shipp != NULL );
		if (shipp->tag_left > 0.0f || shipp->level2_tag_left > 0.0f)
			return FULLY_TARGETABLE;
	}
	
	// only check for Awacs if stealth ship or Nebula mission
	// determine the closest awacs on our team
	if ((stealth_ship || nebula_enabled) && use_awacs)
	{
		for (idx=0; idx<Awacs_count; idx++)
		{
			// if not on the same team as the viewer
			if (Awacs[idx].team != viewer->team)
				continue;

			// if this awacs source has somehow become invalid
			if (Awacs[idx].objp->type != OBJ_SHIP)
				continue;

			// get the subsystem position
			if (!get_subsystem_pos(&subsys_pos, Awacs[idx].objp, Awacs[idx].subsys))
				continue;

			// determine if its the closest
			// special case for HUGE_SHIPS
			if (check_huge_ship)
			{
				// check if inside bbox expanded by awacs_radius
				if (check_world_pt_in_expanded_ship_bbox(&subsys_pos, target, Awacs[idx].subsys->awacs_radius))
				{
					closest_index = idx;
					break;
				}
			}
			// not a huge ship
			else
			{
				// get distance from Subsys to target
				vm_vec_sub(&dist_vec, &subsys_pos, &target->pos);
				test = vm_vec_mag_quick(&dist_vec);

				if (test > Awacs[idx].subsys->awacs_radius)
					continue;

				if ((closest_index == -1) || (test < closest))
				{
					closest = test;
					closest_index = idx;
					break;
				}
			}
		}
	}

	// if this is a stealth ship
	if (stealth_ship)
	{
		// if the ship is within range of an awacs
		if (closest_index >= 0)
		{
			// if the nebula effect is active, stealth ships are only partially targetable
			if (nebula_enabled)
				return MARGINALLY_TARGETABLE;

			// otherwise it's targetable
			return FULLY_TARGETABLE;
		} 
		// otherwise its completely hidden
		else
		{
			return UNTARGETABLE;
		}
	}
	// all other ships
	else
	{
		// if this is not a nebula mission, its always targetable
		if (!nebula_enabled)
			return FULLY_TARGETABLE;

		// if the ship is within range of an awacs, its fully targetable
		if (closest_index >= 0)
			return FULLY_TARGETABLE;


		// fully targetable at half the nebula value

		// modify distance by species
		float scan_nebula_range = Neb2_awacs * Species_info[Ship_info[viewer->ship_info_index].species].awacs_multiplier;

		// special case for huge ship - check inside expanded bounding boxes
		if (check_huge_ship)
		{
			if (check_world_pt_in_expanded_ship_bbox(&Objects[viewer->objnum].pos, target, scan_nebula_range))
			{
				if (check_world_pt_in_expanded_ship_bbox(&Objects[viewer->objnum].pos, target, 0.5f * scan_nebula_range))
					return FULLY_TARGETABLE;

				return MARGINALLY_TARGETABLE;
			}
		} 
		// otherwise check straight up nebula numbers
		else
		{
			vm_vec_sub(&dist_vec, &target->pos, &Objects[viewer->objnum].pos);
			test = vm_vec_mag_quick(&dist_vec);

			if (test < (0.5f * scan_nebula_range))
				return FULLY_TARGETABLE;
			else if (test < scan_nebula_range)
				return MARGINALLY_TARGETABLE;
		}

		// untargetable at longer range
		return UNTARGETABLE;	
	}		
}


// update team visibility
void team_visibility_update()
{
	int team_count[MAX_IFFS];
	int team_ships[MAX_IFFS][MAX_SHIPS];

	ship_obj *moveup;
	ship *shipp;

	// zero out stuff for each team
	memset(team_count, 0, MAX_IFFS * sizeof(int));
	memset(Ship_visibility_by_team, 0, MAX_IFFS * MAX_SHIPS * sizeof(ubyte));

	// Go through list of ships and mark those visible for their own team
	for (moveup = GET_FIRST(&Ship_obj_list); moveup != END_OF_LIST(&Ship_obj_list); moveup = GET_NEXT(moveup))
	{
		// make sure its a valid ship
		if ((Objects[moveup->objnum].type != OBJ_SHIP) || (Objects[moveup->objnum].instance < 0))
			continue;
		
		// get a handle to the ship
		int ship_num = Objects[moveup->objnum].instance;
		shipp = &Ships[ship_num];

		// ignore dying, departing, or arriving ships
		if ((shipp->flags & SF_DYING) || (shipp->flags & SF_DEPARTING) || (shipp->flags & SF_ARRIVING))
			continue;

		// check if ship is flagged as invisible
		if (shipp->flags & SF_HIDDEN_FROM_SENSORS)
			continue;

		// check if ship is stealthed and friendly-invisible
		if ((shipp->flags2 & SF2_STEALTH) && (shipp->flags2 & SF2_FRIENDLY_STEALTH_INVIS))
			continue;

		Ship_visibility_by_team[shipp->team][ship_num] = 1;
		team_ships[shipp->team][team_count[shipp->team]] = ship_num;
		team_count[shipp->team]++;
	}

	int idx, en_idx, cur_count, en_count;
	int *cur_team_ships, *en_team_ships;

	// Do for all teams that cooperate with visibility
	for (int cur_team = 0; cur_team < MAX_IFFS; cur_team++)
	{
		// set up current team
		cur_count = team_count[cur_team];
		cur_team_ships = team_ships[cur_team];

		// short circuit if team has no presence
		if (cur_count == 0)
			continue;	// Goober5000 10/06/2005 changed from break; probably a bug


		// check against all enemy teams
		for (int en_team = 0; en_team < MAX_IFFS; en_team++)
		{
			// NOTE: we no longer skip our own team because we must adjust visibility for friendly-stealth-invisible ships
			// if (en_team == cur_team)
			//	continue;

			// set up enemy team
			en_count = team_count[en_team];
			en_team_ships = team_ships[en_team];

			// check if current team can see enemy team's ships
			for (en_idx = 0; en_idx < en_count; en_idx++)
			{
				// for each ship on other team
				for (idx = 0; idx < cur_count; idx++)
				{
					// ignore nav buoys and cargo containers
					if (Ship_info[Ships[cur_team_ships[idx]].ship_info_index].flags & (SIF_CARGO | SIF_NAVBUOY))
					{
						continue;
					}

					// check against each ship on my team (and AWACS only once)
					if (awacs_get_level(&Objects[Ships[en_team_ships[en_idx]].objnum], &Ships[cur_team_ships[idx]], (idx == 0)) > 1.0f)
					{
						Ship_visibility_by_team[cur_team][en_team_ships[en_idx]] = 1;
						break;
					}
				}
			}
		}
	}
}


// Determine is ship is visible by team
// Goober5000 - now accounts for primitive sensors
int ship_is_visible_by_team(object *target, ship *viewer)
{
	Assert(target);
	Assert(viewer);
	Assert(target->type == OBJ_SHIP);

	// not visible if viewer has primitive sensors
	if (viewer->flags2 & SF2_PRIMITIVE_SENSORS)
		return 0;

	// not visible if out of range
	if ((Hud_max_targeting_range > 0) && (vm_vec_dist_quick(&target->pos, &Objects[viewer->objnum].pos) > Hud_max_targeting_range))
		return 0;

	// now evaluate this the old way
	int ship_num = target->instance;
	int team = viewer->team;

	return (int)Ship_visibility_by_team[team][ship_num];
}
