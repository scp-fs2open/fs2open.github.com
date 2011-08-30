/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// DumpStats.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "DumpStats.h"
#include "starfield/starfield.h"
#include "nebula/neb.h"
#include "cfile/cfile.h"
#include "globalincs/linklist.h"
#include "object/object.h"
#include "object/waypoint.h"
#include "jumpnode/jumpnode.h"
#include "mission/missiongoals.h"
#include "gamesnd/eventmusic.h"
#include "asteroid/asteroid.h"
#include "species_defs/species_defs.h"
#include "weapon/weapon.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DumpStats dialog


DumpStats::DumpStats(CWnd* pParent /*=NULL*/)
	: CDialog(DumpStats::IDD, pParent)
{
	//{{AFX_DATA_INIT(DumpStats)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void DumpStats::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DumpStats)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DumpStats, CDialog)
	//{{AFX_MSG_MAP(DumpStats)
	ON_BN_CLICKED(IDC_DUMP_TO_FILE, OnDumpToFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DumpStats message handlers

BOOL DumpStats::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CString buffer;
	int i;

	// get author, title, etc
	get_mission_stats(buffer);

	// get nebula, stars, etc.
	get_background_stats(buffer);

	// get number or ships, waypoints, start points, etc.
	get_object_stats(buffer);

	// get objectives / goals
	get_objectives_and_goals(buffer);

	// get ship selection for player wings
	get_ship_weapon_selection(buffer);

	// get messaging info
	get_messaging_info(buffer);

	// get species ship breakdown
	get_species_ship_breakdown(buffer);

	// get default loadouts
	get_default_ship_loadouts(buffer);

	int num_tab_stops = 5;
	int tab_stops[5];
	for (i=0; i<5; i++) {
		tab_stops[i] = (i+1) * 16;
	}

	((CEdit*) GetDlgItem(IDC_STATS_TEXT))->SetTabStops(num_tab_stops, tab_stops);
	((CEdit*) GetDlgItem(IDC_STATS_TEXT))->SetWindowText(buffer);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DumpStats::OnDumpToFile() 
{
	// TODO: Add your control notification handler code here

	// get dump from window
	CString buffer;
	((CEdit*) GetDlgItem(IDC_STATS_TEXT))->GetWindowText(buffer);

	CString dump_filename;
	dump_filename.Format("%s.dmp", Mission_filename);

	CFILE *fp;

	fp = cfopen((char *)LPCTSTR(dump_filename), "wt", CFILE_NORMAL, CF_TYPE_MISSIONS);
	cfputs((char *)LPCTSTR(buffer), fp);
	cfclose(fp);
}

void DumpStats::get_mission_stats(CString &buffer)
{
	CString temp;

	// Mission info
	buffer += "\t MISSION INFO\r\n";

	temp.Format("Title: %s\r\n", The_mission.name);
	buffer += temp;

	temp.Format("Filename: %s\r\n", Mission_filename);
	buffer += temp;

	temp.Format("Author: %s\r\n", The_mission.author);
	buffer += temp;

	temp.Format("Description: %s\r\n", The_mission.mission_desc);
	buffer += temp;

	temp.Format("Notes: %s\r\n", The_mission.notes);
	buffer += temp;

	if (The_mission.game_type & MISSION_TYPE_SINGLE) {
		temp.Format("Mission type: Single Player\r\n");
	} else if (The_mission.game_type & MISSION_TYPE_MULTI_COOP) {
		temp.Format("Mission type: Multi Coop\r\n");
	} else if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
		temp.Format("Mission type: Multi Team vs. Team\r\n");
	} else if (The_mission.game_type & MISSION_TYPE_MULTI_DOGFIGHT) {
		temp.Format("Mission type: Dogfight\r\n");
	}
	buffer += temp;

	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		temp.Format("\tNum respawns: %d\r\n", The_mission.num_respawns);
		buffer += temp;
	}

	if (Current_soundtrack_num >= 0) {
		temp.Format("\tMusic: %s\r\n", Soundtracks[Current_soundtrack_num].name);
		buffer += temp;
	}

	if (The_mission.flags & MISSION_FLAG_RED_ALERT) {
		buffer += "\tRed Alert\r\n";
	}

	if (The_mission.flags & MISSION_FLAG_SCRAMBLE) {
		buffer += "\tScramble\r\n";
	}

	if (The_mission.flags & MISSION_FLAG_NO_PROMOTION) {
		buffer += "\tNo Promotions\r\n";
	}

	if (The_mission.support_ships.max_support_ships == 0) {
		buffer += "\tNo Support ships\r\n";
	}

	temp.Format("Squadron: %s,  Squadron logo: %s\r\n", The_mission.squad_name, The_mission.squad_filename);
	buffer += temp;
}

void DumpStats::get_background_stats(CString &buffer)
{
	CString temp;
	int i;

	// Background
	buffer += "\r\n\tBACKGROUND INFO\r\n";

	// Num stars
	temp.Format("Num_stars: %d\r\n", Num_stars);
	buffer += temp;

	// Suns
	temp.Format("Num_suns: %d\r\n", stars_get_num_suns());
	buffer += temp;
	
	for (i=0; i<stars_get_num_suns(); i++) {
		temp.Format("\tSun%d bitmap name: %s\r\n", i, stars_get_sun_name(i));
		buffer += temp;
		//temp.Format("Sun%d glow name: %s\r\n", i, Suns[i].glow_filename);
		//buffer += temp;
	}

	// Starfield bitmaps
	temp.Format("Num_starfield_bitmaps: %d\r\n", stars_get_num_bitmaps());
	buffer += temp;

	for (i=0; i<stars_get_num_bitmaps(); i++) {
		temp.Format("\tStarfield%d bitmap name: %s\r\n", i, stars_get_bitmap_name(i));
		buffer += temp;
	}

	// Asteroids
	temp.Format("Num Field Debris Chunks: %d\r\n", Asteroid_field.num_initial_asteroids);
	buffer += temp;
	if (Asteroid_field.num_initial_asteroids > 0) {
		// active or passive
		if (Asteroid_field.field_type == FT_ACTIVE) {
			temp.Format("\tActive Field\r\n");
			buffer += temp;

			temp.Format("\tAsteroid Debris\r\n");
			buffer += temp;
		} else {
			// passive
			temp.Format("\tPassive Field\r\n");
			buffer += temp;

			if (Asteroid_field.debris_genre == DG_ASTEROID) {
				temp.Format("\tAsteroid Debris\r\n");
				buffer += temp;
			} else {
				temp.Format("\tShip Debris\r\n");
				buffer += temp;

				// species
				temp.Format("\t\tSpecies: ");
				for (i=0; i<(int)Species_info.size(); i++) {
					if (Asteroid_field.field_debris_type[i] >= 0) {
						temp += CString(Species_info[(Asteroid_field.field_debris_type[i] / NUM_DEBRIS_SIZES) - 1].species_name) + " ";
					}
				}

				temp += "\r\n";

				buffer += temp;
			}
		}
	}

	// Nebula mission
	int nebula_mission = (The_mission.flags & MISSION_FLAG_FULLNEB);
	temp = "Nebula mission:";
	if (nebula_mission) {
		temp += " Yes\r\n";
	} else {
		temp += " No\r\n";
	}
	buffer += temp;

	if (nebula_mission) {
		// range
		temp.Format("\tNebula awacs range: %.0f\r\n", Neb2_awacs);
		buffer += temp;

		// list of poofs
		for (i=0; i<MAX_NEB2_POOFS; i++) {
			if ( Neb2_poof_flags & (1<<i) ) {
				temp.Format("\tNebula poof: %s\r\n", Neb2_poof_filenames[i]);
				buffer += temp;
			}
		}

		// nebula texture
		if (strlen(Neb2_texture_name) > 0) {
			temp.Format("\tNebula texture: %s\r\n", Neb2_texture_name);
			buffer += temp;
		}
	} else {
		// FS! nebula pattern
		if (Nebula_index > 0) {
			temp.Format("\tOld style FS1 nebula filename: %s\r\n", Nebula_filenames[Nebula_index]);
			buffer += temp;
		}
	}

	// Subspace mission
	temp = "Subspace mission:";
	if (The_mission.flags & MISSION_FLAG_SUBSPACE) {
		temp += " Yes\r\n";
	} else {
		temp += " No\r\n";
	}
	buffer += temp;
}

void DumpStats::get_object_stats(CString &buffer)
{
	object *objp;
	int obj_type_count[MAX_OBJECT_TYPES];
	CString temp;
	int num_small_ships, num_big_ships, num_huge_ships;
	int i;

	memset(obj_type_count,0, sizeof(obj_type_count));
	num_small_ships = num_big_ships = num_huge_ships= 0;

	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {

		// inc big ship or small ship count
		if ( (objp->type == OBJ_SHIP) || (objp->type == OBJ_START) ) {
			if ( Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_SMALL_SHIP ) {
				num_small_ships++;
			} else if ( Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_BIG_SHIP ) {
				num_big_ships++;
			} else if ( Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_HUGE_SHIP ) {
				num_huge_ships++;
			}
		}

		obj_type_count[objp->type]++;
	}

	// Statistics
	buffer += "\r\n\tMISSION STATISTICS\r\n";

	// OBJ_START is also a OBJ_SHIP
	// not counting num_waves (for wings)
	obj_type_count[OBJ_SHIP] += obj_type_count[OBJ_START];

	for (i=0; i<MAX_OBJECT_TYPES; i++) {
		if (obj_type_count[i] > 0) {
			switch(i) {
			case OBJ_SHIP:
				temp.Format("Ship Count: %d\r\n", obj_type_count[i]);
				buffer += temp;
				break;

			case OBJ_START:
				temp.Format("Start Count: %d\r\n", obj_type_count[i]);
				buffer += temp;
				break;

			case OBJ_WAYPOINT:
				temp.Format("Waypoint Count: %d\r\n", obj_type_count[i]);
				buffer += temp;
				break;

			case OBJ_WING:
				temp.Format("Wing Count: %d\r\n", obj_type_count[i]);
				buffer += temp;
				break;

			case OBJ_JUMP_NODE:
				temp.Format("Jump Node Count: %d\r\n", obj_type_count[i]);
				buffer += temp;
				break;

			default:
				Int3();
				break;
			}
		}
	}

	buffer += "\r\nSHIPS\r\n";
	temp.Format("\tNum small ships: %d\r\n", num_small_ships);
	buffer += temp;

	temp.Format("\tNum big ships: %d\r\n", num_big_ships);
	buffer += temp;

	temp.Format("\tNum huge ships: %d\r\n", num_huge_ships);
	buffer += temp;

	// Waypoints
	int total_waypoints = 0;
	buffer += "\r\nWAYPOINTS\r\n";

	SCP_list<waypoint_list>::iterator ii;
	for (ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++ii) {
		temp.Format("\tWaypoint: %s, count: %d\r\n", ii->get_name(), ii->get_waypoints().size());
		buffer += temp;
		total_waypoints += ii->get_waypoints().size();
	}

	if (total_waypoints > 0) {
		temp.Format("\ttotal_waypoints: %d\r\n", total_waypoints);
		buffer += temp;
	}

	// Jumpnodes
	buffer += "\r\nJUMPNODES\r\n";
	for ( jump_node *jnp = (jump_node *)Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = (jump_node *)jnp->get_next() ) {
		temp.Format("\tJumpnode: %s\r\n", jnp->get_name_ptr());
		buffer += temp;
	}

	if (Jump_nodes.get_num_elements() > 0) {
		temp.Format("\ttotal_jumpnodes: %d\r\n", Jump_nodes.get_num_elements());
		buffer += temp;
	}


	// Wings
	int num_counted_wings = 0;
	buffer += "\r\nWINGS\r\n";
	for (i=0; i<MAX_WINGS; i++) {
		if (Wings[i].wave_count > 0) {
			temp.Format("\tWing Name: %s,  wave_count: %d,  num_waves: %d\r\n", Wings[i].name, Wings[i].wave_count, Wings[i].num_waves);
			buffer += temp;

			num_counted_wings++;
			if (num_counted_wings == Num_wings) {
				break;
			}
		}
	}

	// Escort
	buffer += "\r\nESCORT\r\n";
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type == OBJ_SHIP) || (objp->type == OBJ_START) ) {
			if (Ships[objp->instance].flags & SF_ESCORT) {
				temp.Format("\tShip name: %s, priority: %d\r\n", Ships[objp->instance].ship_name, Ships[objp->instance].escort_priority);
				buffer += temp;
			}
		}
	}

	// Hotkeys
	buffer += "\r\nHOTKEYS\r\n";

	// ship hotkeys
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type == OBJ_SHIP) || (objp->type == OBJ_START) ) {
			if (Ships[objp->instance].hotkey != -1) {
				temp.Format("\tShip name: %s, hotkey: F%d\r\n", Ships[objp->instance].ship_name, (Ships[objp->instance].hotkey + 5));
				buffer += temp;
			}
		}
	}

	// wing hotkeys
	for (i=0; i<MAX_WINGS; i++) {
		if (Wings[i].wave_count > 0) {
			if (Wings[i].hotkey != -1) {
				temp.Format("\tWing name: %s, hotkey: F%d\r\n", Wings[i].name, (Wings[i].hotkey + 5));
				buffer += temp;
			}
		}
	}

}

void DumpStats::get_objectives_and_goals(CString &buffer)
{
	CString temp;
	int i;

	buffer += "\r\nOBJECTIVES AND GOALS\r\n";

	// objectives
	for (i=0; i<Num_mission_events; i++) {
		// name, objective_text, objective_key_text
		if ( Mission_events[i].objective_text == NULL ) {
			continue;
		}
		temp.Format("\tObjective: %s,  text: %s,  key_text: %s\r\n", Mission_events[i].name, Mission_events[i].objective_text, Mission_events[i].objective_key_text);
		buffer += temp;
	}

	buffer += "\r\n";

	// goals
	for (i=0; i<Num_goals; i++) {
		temp.Format("\tGoal: %s, text: ", Mission_goals[i].name, Mission_goals[i].message);
		buffer += temp;

		switch(Mission_goals[i].type & GOAL_TYPE_MASK) {
		case PRIMARY_GOAL:
			buffer += ",  type: primary\r\n";
			break;

		case SECONDARY_GOAL:
			buffer += ",  type: secondary\r\n";
			break;

		case BONUS_GOAL:
			buffer += ",  type: bonus\r\n";
			break;

		default:
			Int3();
			break;
		}
	}

}

void DumpStats::get_ship_weapon_selection(CString &buffer)
{
	CString temp;
	int i,j;

	buffer += "\r\nSHIP WEAPON/SELECTION\r\n";
	buffer += "Reported numbers are in addition to assigned ships and their default weapons\r\n";

	for (i=0; i<Num_teams; i++) {
		temp.Format("Team %d\r\n", i);
		buffer += temp;

		// ships
		for (j=0; j<Team_data[i].num_ship_choices; j++) {
			temp.Format("\tShip name: %s, count %d", Ship_info[Ships[Team_data[i].ship_list[j]].ship_info_index].name, Team_data[i].ship_count[j]);
			buffer += temp;

			if (Team_data[i].ship_list[j] == Team_data[i].default_ship) {
				temp = "  DEFAULT SHIP\r\n";
			} else {
				temp = "\r\n";
			}
			buffer += temp;
		}

		buffer += "\r\n";

		// weapons
		for (j=0; j<Team_data[i].num_weapon_choices; j++) {
			//if (Team_data[i].weaponry_pool[j] > 0) 
			temp.Format("\tWeapon name: %s, count %d\r\n", Weapon_info[Team_data[i].weaponry_pool[j]].name, Team_data[i].weaponry_count[j]);
			buffer += temp;
			
		}
	}

}

void DumpStats::get_messaging_info(CString &buffer)
{
	CString temp;
	object *objp;
	ship *shipp;

	buffer += "\r\nSHIP ACCEPTED ORDERS\r\n";

	// go through all ships and check (.orders_accepted against default_orders)
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if (objp->type == OBJ_START || objp->type == OBJ_SHIP) {
			shipp = &Ships[objp->instance];

			if (shipp->orders_accepted != ship_get_default_orders_accepted(&Ship_info[shipp->ship_info_index])) {
				temp.Format("\tShip: %s with nonstandard accepted orders\r\n", shipp->ship_name);
				buffer += temp;
			}
		}
	}
}

void DumpStats::get_species_ship_breakdown(CString &buffer)
{
	CString temp;
	int i;
	int species;
	object *objp;
	ship *shipp;

	buffer += "\r\nSHIP SPECIES BREAKDOWN\r\n";


	for (species=0; (uint) species < Species_info.size(); species++) {
		buffer += Species_info[species].species_name;
		buffer += "\r\n";

		// fighter wings
		buffer += "\tFighter wings:\r\n";
		for (i=0; i<MAX_WINGS; i++) {
			if (Wings[i].wave_count > 0) {
				int wing_leader_shipnum = Wings[i].ship_index[Wings[i].special_ship];
				if (Ship_info[Ships[wing_leader_shipnum].ship_info_index].species == species) {
					if (Ship_info[Ships[wing_leader_shipnum].ship_info_index].flags & SIF_FIGHTER) {
						temp.Format("\t\tWing: %s, count: %d, waves: %d, type: %s\r\n", Wings[i].name, Wings[i].wave_count, Wings[i].num_waves, Ship_info[Ships[wing_leader_shipnum].ship_info_index].name);
						buffer += temp;
					}
				}
			}
		}

		// bomber wings
		buffer += "\tBomber wings:\r\n";
		for (i=0; i<MAX_WINGS; i++) {
			if (Wings[i].wave_count > 0) {
				int wing_leader_shipnum = Wings[i].ship_index[Wings[i].special_ship];
				if (Ship_info[Ships[wing_leader_shipnum].ship_info_index].species == species) {
					if (Ship_info[Ships[wing_leader_shipnum].ship_info_index].flags & SIF_BOMBER) {
						temp.Format("\t\tWing: %s, count: %d, waves: %d, type: %s\r\n", Wings[i].name, Wings[i].wave_count, Wings[i].num_waves, Ship_info[Ships[wing_leader_shipnum].ship_info_index].name);
						buffer += temp;
					}
				}
			}
		}

		buffer += "\tFreighters, Cargo, Transports:\r\n";
		// freighters and transports (cargo)
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (objp->type == OBJ_START || objp->type == OBJ_SHIP) {
				shipp = &Ships[objp->instance];

				if (Ship_info[shipp->ship_info_index].species == species) {
					//if (shipp->wingnum == -1)
					//if (shipp->cargo1 > 0)
					if (Ship_info[shipp->ship_info_index].flags & (SIF_FREIGHTER | SIF_TRANSPORT | SIF_CARGO)) {
						temp.Format("\t\tName: %s Type: %s, Cargo: %s\r\n", shipp->ship_name, Ship_info[shipp->ship_info_index].name, Cargo_names[shipp->cargo1]);
						buffer += temp;
					}
				}
			}
		}

		buffer += "\tNav buoy, Escape pod, Sentry gun:\r\n";
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (objp->type == OBJ_START || objp->type == OBJ_SHIP) {
				shipp = &Ships[objp->instance];

				if (Ship_info[shipp->ship_info_index].species == species) {
					//if (shipp->wingnum == -1)
					//if (shipp->cargo1 > 0)
					if (Ship_info[shipp->ship_info_index].flags & (SIF_NAVBUOY | SIF_ESCAPEPOD | SIF_SENTRYGUN)) {
						temp.Format("\t\tName: %s, Type: %s Cargo: %s\r\n", shipp->ship_name, Ship_info[shipp->ship_info_index].name, Cargo_names[shipp->cargo1]);
						buffer += temp;
					}
				}
			}
		}


		// cruiser
		buffer += "\tCruiser:\r\n";
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (objp->type == OBJ_START || objp->type == OBJ_SHIP) {
				shipp = &Ships[objp->instance];

				if (Ship_info[shipp->ship_info_index].species == species) {
					//if (shipp->wingnum == -1)
					//if (shipp->cargo1 > 0)
					if (Ship_info[shipp->ship_info_index].flags & (SIF_CRUISER)) {
						temp.Format("\t\tName: %s, Type: %s, Cargo: %s\r\n", shipp->ship_name, Ship_info[shipp->ship_info_index].name, Cargo_names[shipp->cargo1]);
						buffer += temp;
					}
				}
			}
		}

		// dry dock, cap, super cap
		buffer += "\tDry dock, Capital, Supercap:\r\n";
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (objp->type == OBJ_START || objp->type == OBJ_SHIP) {
				shipp = &Ships[objp->instance];

				if (Ship_info[shipp->ship_info_index].species == species) {
					//if (shipp->wingnum == -1)
					//if (shipp->cargo1 > 0)
					if (Ship_info[shipp->ship_info_index].flags & (SIF_DRYDOCK|SIF_CAPITAL|SIF_SUPERCAP)) {
						temp.Format("\t\tName: %s, Type: %s, Cargo: %s\r\n", shipp->ship_name, Ship_info[shipp->ship_info_index].name, Cargo_names[shipp->cargo1]);
						buffer += temp;
					}
				}
			}
		}

		buffer += "\r\n";
	}
}

void dump_loadout(ship *shipp, CString &loadout)
{
	CString temp;
	char *weapon_name;

	loadout = "";

//	PRIMARY
	int pri_idx, sec_idx;

	for (pri_idx=0; pri_idx < shipp->weapons.num_primary_banks; pri_idx++) {
		if (shipp->weapons.primary_bank_weapons[pri_idx] == -1) {
			weapon_name = "none";
		} else {
			weapon_name = Weapon_info[shipp->weapons.primary_bank_weapons[pri_idx]].name;
		} 
		temp.Format("\t\t\tPrimary[%d]: %s\r\n", pri_idx+1, weapon_name);
		loadout += temp;
	}

// SECONDARY
	for (sec_idx=0; sec_idx < shipp->weapons.num_secondary_banks; sec_idx++) {
		if (shipp->weapons.secondary_bank_weapons[sec_idx] == -1) {
			weapon_name = "none";
		} else {
			weapon_name = Weapon_info[shipp->weapons.secondary_bank_weapons[sec_idx]].name;
		} 
		temp.Format("\t\t\tSecondary[%d]: %s\r\n", sec_idx+1, weapon_name);
		loadout += temp;
	}

// TURRET
	ship_subsys *ss;
	for (ss = GET_FIRST(&shipp->subsys_list); ss != END_OF_LIST(&shipp->subsys_list); ss = GET_NEXT(ss) ) {
		if ( (ss->system_info->type == SUBSYSTEM_TURRET) ) {
//			ss->weapons.num_primary_banks, ss->weapons.num_secondary_banks, ss->weapons.primary_bank_weapons[3], ss->weapons.secondary_bank_weapons[2]
//			ss->system_info->primary_banks, ss->system_info->secondary_banks
			temp.Format("\t\t\tTurret: %s\r\n", ss->system_info->subobj_name);
			loadout += temp;

			// PRIMARY
			for (pri_idx=0; pri_idx < ss->weapons.num_primary_banks; pri_idx++) {
				if (ss->weapons.primary_bank_weapons[pri_idx] == -1) {
					weapon_name = "none";
				} else {
					weapon_name = Weapon_info[ss->weapons.primary_bank_weapons[pri_idx]].name;
				} 
				temp.Format("\t\t\t\tPrimary[%d]: %s\r\n", pri_idx+1, weapon_name);
				loadout += temp;
			}

			// SECONDARY
			for (sec_idx=0; sec_idx < ss->weapons.num_secondary_banks; sec_idx++) {
				if (ss->weapons.secondary_bank_weapons[sec_idx] == -1) {
					weapon_name = "none";
				} else {
					weapon_name = Weapon_info[ss->weapons.secondary_bank_weapons[sec_idx]].name;
				} 
				temp.Format("\t\t\t\tSecondary[%d]: %s\r\n", sec_idx+1, weapon_name);
				loadout += temp;
			}
		}
	}

}

void DumpStats::get_default_ship_loadouts(CString &buffer)
{
	int i;
	int species;
	object *objp;
	ship *shipp;
	CString temp, loadout;

	buffer += "\r\nSHIP SPECIES BREAKDOWN\r\n";


	for (species=0; (uint) species < Species_info.size(); species++) {
		buffer += Species_info[species].species_name;
		buffer += "\r\n";

		// fighter wings
		buffer += "\tFighter wings:\r\n";
		for (i=0; i<MAX_WINGS; i++) {
			if (Wings[i].wave_count > 0) {
				int wing_leader_shipnum = Wings[i].ship_index[Wings[i].special_ship];
				if (Ship_info[Ships[wing_leader_shipnum].ship_info_index].species == species) {
					if (Ship_info[Ships[wing_leader_shipnum].ship_info_index].flags & SIF_FIGHTER) {
						temp.Format("\t\tWing: %s\r\n", Wings[i].name);
						buffer += temp;
						dump_loadout(&Ships[wing_leader_shipnum], loadout);
						buffer += loadout;
					}
				}
			}
		}

		// bomber wings
		buffer += "\tBomber wings:\r\n";
		for (i=0; i<MAX_WINGS; i++) {
			if (Wings[i].wave_count > 0) {
				int wing_leader_shipnum = Wings[i].ship_index[Wings[i].special_ship];
				if (Ship_info[Ships[wing_leader_shipnum].ship_info_index].species == species) {
					if (Ship_info[Ships[wing_leader_shipnum].ship_info_index].flags & SIF_BOMBER) {
						temp.Format("\t\tWing: %s\r\n", Wings[i].name);
						buffer += temp;
						dump_loadout(&Ships[wing_leader_shipnum], loadout);
						buffer += loadout;
					}
				}
			}
		}

		buffer += "\tFreighters, Cargo, Transports:\r\n";
		// freighters and transports (cargo)
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (objp->type == OBJ_START || objp->type == OBJ_SHIP) {
				shipp = &Ships[objp->instance];

				if (Ship_info[shipp->ship_info_index].species == species) {
					//if (shipp->wingnum == -1)
					//if (shipp->cargo1 > 0)
					if (Ship_info[shipp->ship_info_index].flags & (SIF_FREIGHTER | SIF_TRANSPORT)) {
						temp.Format("\t\tName: %s\r\n", shipp->ship_name);
						buffer += temp;
						dump_loadout(shipp, loadout);
						buffer += loadout;
					}
				}
			}
		}

		buffer += "\tEscape pod, Sentry gun:\r\n";
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (objp->type == OBJ_START || objp->type == OBJ_SHIP) {
				shipp = &Ships[objp->instance];

				if (Ship_info[shipp->ship_info_index].species == species) {
					//if (shipp->wingnum == -1)
					//if (shipp->cargo1 > 0)
					if (Ship_info[shipp->ship_info_index].flags & (SIF_ESCAPEPOD | SIF_SENTRYGUN)) {
						temp.Format("\t\tName: %s\r\n", shipp->ship_name);
						buffer += temp;
						dump_loadout(shipp, loadout);
						buffer += loadout;
					}
				}
			}
		}


		// cruiser
		buffer += "\tCruiser:\r\n";
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (objp->type == OBJ_START || objp->type == OBJ_SHIP) {
				shipp = &Ships[objp->instance];

				if (Ship_info[shipp->ship_info_index].species == species) {
					//if (shipp->wingnum == -1)
					//if (shipp->cargo1 > 0)
					if (Ship_info[shipp->ship_info_index].flags & (SIF_CRUISER)) {
						temp.Format("\t\tName: %s\r\n", shipp->ship_name);
						buffer += temp;
						dump_loadout(shipp, loadout);
						buffer += loadout;
					}
				}
			}
		}

		// dry dock, cap, super cap
		buffer += "\tCapital, Supercap:\r\n";
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (objp->type == OBJ_START || objp->type == OBJ_SHIP) {
				shipp = &Ships[objp->instance];

				if (Ship_info[shipp->ship_info_index].species == species) {
					//if (shipp->wingnum == -1)
					//if (shipp->cargo1 > 0)
					if (Ship_info[shipp->ship_info_index].flags & (SIF_CAPITAL|SIF_SUPERCAP)) {
						temp.Format("\t\tName: %s\r\n", shipp->ship_name);
						buffer += temp;
						dump_loadout(shipp, loadout);
						buffer += loadout;
					}
				}
			}
		}

		buffer += "\r\n";
	}
	// go through all wings

	// go through all ships not in wings and FLYABLE

	// print primary, secondary, and BIG turrets
}
