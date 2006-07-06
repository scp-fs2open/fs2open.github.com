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
 * $Revision: 2.28 $
 * $Date: 2006-07-06 22:00:39 $
 * $Author: taylor $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.27  2006/06/27 05:05:18  taylor
 * make sure we only do ship related setup on actual ships
 *
 * Revision 2.26  2006/06/27 02:52:36  Goober5000
 * change back to ubyte
 * --Goober5000
 *
 * Revision 2.25  2006/06/24 04:48:02  Goober5000
 * cosmetics, plus revert an unnecessary if
 *
 * Revision 2.24  2006/06/07 04:47:43  wmcoolmon
 * Limbo flag support; removed unneeded muzzle flash flag
 *
 * Revision 2.23  2006/03/18 10:28:25  taylor
 * fix out-of-bounds problem in awacs checking (full neb missions may have been a bit freaky from this)
 *
 * Revision 2.22  2006/02/19 03:17:01  Goober5000
 * fix stealth
 * --Goober5000
 *
 * Revision 2.21  2006/01/13 03:30:59  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 2.20  2005/10/10 17:21:10  taylor
 * remove NO_NETWORK
 *
 * Revision 2.19  2005/10/09 08:03:21  wmcoolmon
 * New SEXP stuff
 *
 * Revision 2.18  2005/09/27 02:36:56  Goober5000
 * clarification
 * --Goober5000
 *
 * Revision 2.17  2005/09/25 05:13:04  Goober5000
 * hopefully complete species upgrade
 * --Goober5000
 *
 * Revision 2.16  2005/07/25 03:13:24  Goober5000
 * various code cleanups, tweaks, and fixes; most notably the MISSION_FLAG_USE_NEW_AI
 * should now be added to all places where it is needed (except the turret code, which I still
 * have to to review)
 * --Goober5000
 *
 * Revision 2.15  2005/07/22 10:18:35  Goober5000
 * CVS header tweaks
 * --Goober5000
 *
 * Revision 2.14  2005/07/13 03:35:30  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.13  2005/07/13 00:44:21  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.12  2005/04/05 05:53:24  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.11  2005/03/02 21:24:47  taylor
 * more network/inferno goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.10  2004/07/26 20:47:51  Kazan
 * remove MCD complete
 *
 * Revision 2.9  2004/07/12 16:33:05  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
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
		if ((shipp->flags & SF_DYING) || (shipp->flags & SF_DEPARTING) || (shipp->flags & SF_ARRIVING) || (shipp->flags & SF_LIMBO))
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
	int idx, friendly_invisible = 0;
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


	// if the viewer is me, and I'm a multiplayer observer, its always viewable
	if ((viewer == Player_ship) && (Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && MULTI_OBSERVER(Net_players[MY_NET_PLAYER_NUM]))
		return ALWAYS_TARGETABLE;

	if (target->type == OBJ_SHIP) {
		// if no valid target then bail as never viewable
		if (target->instance < 0)
			return UNTARGETABLE;

		// Goober5000
		shipp = &Ships[target->instance];
		sip = &Ship_info[shipp->ship_info_index];
		friendly_invisible = (shipp->flags2 & SF2_STEALTH) && (shipp->flags2 & SF2_FRIENDLY_STEALTH_INVIS);
	}
	
	int stealth_ship = (target->type == OBJ_SHIP) && (target->instance >= 0) && (shipp->flags2 & SF2_STEALTH);
	int nebula_enabled = (The_mission.flags & MISSION_FLAG_FULLNEB);
	int check_huge_ship = (target->type == OBJ_SHIP) && (target->instance >= 0) && (sip->flags & SIF_HUGE_SHIP);

	// ships on the same team are always viewable
	// not necessarily now! :) -- Goober5000
	if ((target->type == OBJ_SHIP) && (shipp->team == viewer->team) && (!friendly_invisible))
		return FULLY_TARGETABLE;

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

	// check for a tagged ship. TAG'd ships are _always_ visible
	if (target->type == OBJ_SHIP)
	{
		if (shipp->tag_left > 0.0f || shipp->level2_tag_left > 0.0f)
			return FULLY_TARGETABLE;
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

			// otherwise its targetable
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
				if (check_world_pt_in_expanded_ship_bbox(&Objects[viewer->objnum].pos, target, MARGINALLY_TARGETABLE * scan_nebula_range))
					return FULLY_TARGETABLE;

				return MARGINALLY_TARGETABLE;
			}
		} 
		// otherwise check straight up nebula numbers
		else
		{
			vm_vec_sub(&dist_vec, &target->pos, &Objects[viewer->objnum].pos);
			test = vm_vec_mag_quick(&dist_vec);

			if (test < (MARGINALLY_TARGETABLE * scan_nebula_range))
				return FULLY_TARGETABLE;
			else if (test < scan_nebula_range)
				return MARGINALLY_TARGETABLE;
		}

		// untargetable at longer range
		return UNTARGETABLE;	
	}		

	Int3();
	return FULLY_TARGETABLE;
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

	// Go through list of ships and mark those visible for given team
	for (moveup = GET_FIRST(&Ship_obj_list); moveup != END_OF_LIST(&Ship_obj_list); moveup = GET_NEXT(moveup))
	{
		// make sure its a valid ship
		if ((Objects[moveup->objnum].type != OBJ_SHIP) || (Objects[moveup->objnum].instance < 0))
			continue;
		
		// get a handle to the ship
		int ship_num = Objects[moveup->objnum].instance;
		shipp = &Ships[ship_num];

		// ignore dying, departing, or arriving ships
		if ((shipp->flags & SF_DYING) || (shipp->flags & SF_DEPARTING) || (shipp->flags & SF_ARRIVING) || (shipp->flags & SF_LIMBO))
			continue;

		// check if ship if flagged as invisible
		if (shipp->flags & SF_HIDDEN_FROM_SENSORS)
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
			if (en_team == cur_team)
				continue;

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
// Goober5000 - now accounts for friendly stealth invisible and primitive sensors
int ship_is_visible_by_team(object *target, ship *viewer)
{
	Assert(target);
	Assert(viewer);
	Assert(target->type == OBJ_SHIP);

	ship *target_shipp = &Ships[target->instance];

	// not visible if viewer has primitive sensors
	if (viewer->flags2 & SF2_PRIMITIVE_SENSORS)
		return 0;

	// friendly stealthed ships are not visible if they have the friendly-stealth-invisible flag set
	if (target_shipp->team == viewer->team)
	{
		if (target_shipp->flags2 & SF2_STEALTH)
		{
			if (target_shipp->flags2 & SF2_FRIENDLY_STEALTH_INVIS)
				return 0;
		}
	}

	// now evaluate this the old way
	int ship_num = target->instance;
	int team = viewer->team;

	return (int)Ship_visibility_by_team[team][ship_num];
}
