
#include "globalincs/pstypes.h"
#include "pilotfile/pilotfile.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "stats/medals.h"
#include "menuui/techmenu.h"
#include "hud/hudconfig.h"
#include "cutscene/cutscenes.h"
#include "missionui/missionscreencommon.h"
#include "mission/missioncampaign.h"
#include "missionui/missionshipchoice.h"
#include "mission/missionload.h"
#include "sound/audiostr.h"
#include "io/joy.h"
#include "io/mouse.h"
#include "gamesnd/eventmusic.h"
#include "freespace2/freespace.h"

#define REDALERT_INTERNAL
#include "missionui/redalert.h"

#include <iostream>
#include <sstream>


static const unsigned int CSG_FILE_ID = 0x5f475343;	// "CSG_" in file

// NOTE: Version should be bumped only for adding/removing sections or section
//       content.  It should *NOT* be bumped for limit bumps or anything of
//       that sort!
//
//   0 - initial version
//   1 - re-add recent missions
static const ubyte CSG_VERSION = 1;


void pilotfile::csg_read_flags()
{
	// tips?
	p->tips = (int)cfread_ubyte(cfp);
}

void pilotfile::csg_write_flags()
{
	startSection(Section::Flags);

	// tips
	cfwrite_ubyte((ubyte)p->tips, cfp);

	endSection();
}

void pilotfile::csg_read_info()
{
	char t_string[NAME_LENGTH+1] = { '\0' };
	index_list_t ilist;
	int idx, list_size = 0;
	ubyte allowed = 0;

	if ( !m_have_flags ) {
		throw "Info before Flags!";
	}

	//
	// NOTE: lists may contain missing/invalid entries for current data
	//       this is not necessarily fatal
	//

	// ship list (NOTE: may contain more than MAX_SHIP_CLASSES)
	list_size = cfread_int(cfp);

	for (idx = 0; idx < list_size; idx++) {
		cfread_string_len(t_string, NAME_LENGTH, cfp);

		ilist.name = t_string;
		ilist.index = ship_info_lookup(t_string);

		ship_list.push_back(ilist);
	}

	// weapon list (NOTE: may contain more than MAX_WEAPON_TYPES)
	list_size = cfread_int(cfp);

	for (idx = 0; idx < list_size; idx++) {
		cfread_string_len(t_string, NAME_LENGTH, cfp);

		ilist.name = t_string;
		ilist.index = weapon_info_lookup(t_string);

		weapon_list.push_back(ilist);
	}

	// intel list (NOTE: may contain more than MAX_INTEL_ENTRIES)
	list_size = cfread_int(cfp);

	for (idx = 0; idx < list_size; idx++) {
		cfread_string_len(t_string, NAME_LENGTH, cfp);

		ilist.name = t_string;
		ilist.index = intel_info_lookup(t_string);

		intel_list.push_back(ilist);
	}

	// medals list (NOTE: may contain more than Num_medals)
	list_size = cfread_int(cfp);

	for (idx = 0; idx < list_size; idx++) {
		cfread_string_len(t_string, NAME_LENGTH, cfp);

		ilist.name = t_string;
		ilist.index = medals_info_lookup(t_string);

		medals_list.push_back(ilist);
	}

	// last ship flown (index into ship_list)
	idx = cfread_int(cfp);

	// check the idx is within bounds
	Assertion ((idx < (int)ship_list.size()), "Campaign file contains an incorrect value for the last flown ship class. No data in ship_list for ship number %d.", idx); 
	if (idx >= (int)ship_list.size())
		idx = -1;
	else if (idx != -1)
		p->last_ship_flown_si_index = ship_list[idx].index;
	else
		p->last_ship_flown_si_index = -1;

	// progression state
	Campaign.prev_mission = cfread_int(cfp);
	Campaign.next_mission = cfread_int(cfp);

	// loop state
	Campaign.loop_reentry = cfread_int(cfp);
	Campaign.loop_enabled = cfread_int(cfp);

	// missions completed
	Campaign.num_missions_completed = cfread_int(cfp);

	// allowed ships
	list_size = (int)ship_list.size();
	for (idx = 0; idx < list_size; idx++) {
		allowed = cfread_ubyte(cfp);

		if (allowed) {
			if (ship_list[idx].index >= 0) {
				Campaign.ships_allowed[ship_list[idx].index] = 1;
			} else {
				m_data_invalid = true;
			}
		}
	}

	// allowed weapons
	list_size = (int)weapon_list.size();
	for (idx = 0; idx < list_size; idx++) {
		allowed = cfread_ubyte(cfp);

		if (allowed) {
			if (weapon_list[idx].index >= 0) {
				Campaign.weapons_allowed[weapon_list[idx].index] = 1;
			} else {
				m_data_invalid = true;
			}
		}
	}

	// if anything we need/use was missing then it should be considered fatal
	if (m_data_invalid) {
		throw "Invalid data for CSG!";
	}
}

void pilotfile::csg_write_info()
{
	int idx;

	startSection(Section::Info);

	// ship list
	cfwrite_int(Num_ship_classes, cfp);

	for (idx = 0; idx < Num_ship_classes; idx++) {
		cfwrite_string_len(Ship_info[idx].name, cfp);
	}

	// weapon list
	cfwrite_int(Num_weapon_types, cfp);

	for (idx = 0; idx < Num_weapon_types; idx++) {
		cfwrite_string_len(Weapon_info[idx].name, cfp);
	}

	// intel list
	cfwrite_int(Intel_info_size, cfp);

	for (idx = 0; idx < Intel_info_size; idx++) {
		cfwrite_string_len(Intel_info[idx].name, cfp);
	}

	// medals list
	cfwrite_int((int)Medals.size(), cfp);

	for (idx = 0; idx < (int)Medals.size(); idx++) {
		cfwrite_string_len(Medals[idx].name, cfp);
	}

	// last ship flown
	cfwrite_int(p->last_ship_flown_si_index, cfp);

	// progression state
	cfwrite_int(Campaign.prev_mission, cfp);
	cfwrite_int(Campaign.next_mission, cfp);

	// loop state
	cfwrite_int(Campaign.loop_enabled, cfp);
	cfwrite_int(Campaign.loop_reentry, cfp);

	// missions completed
	cfwrite_int(Campaign.num_missions_completed, cfp);

	// allowed ships
	for (idx = 0; idx < Num_ship_classes; idx++) {
		cfwrite_ubyte(Campaign.ships_allowed[idx], cfp);
	}

	// allowed weapons
	for (idx = 0; idx < Num_weapon_types; idx++) {
		cfwrite_ubyte(Campaign.weapons_allowed[idx], cfp);
	}

	endSection();
}

void pilotfile::csg_read_missions()
{
	int i, j, idx, list_size;
	cmission *mission;

	if ( !m_have_info ) {
		throw "Missions before Info!";
	}

	for (i = 0; i < Campaign.num_missions_completed; i++) {
		idx = cfread_int(cfp);
		mission = &Campaign.missions[idx];

		mission->completed = 1;

		// flags
		mission->flags = cfread_int(cfp);

		// goals
		mission->num_goals = cfread_int(cfp);

		if (mission->num_goals > 0) {
			mission->goals = (mgoal *) vm_malloc( mission->num_goals * sizeof(mgoal) );
			Verify( mission->goals != NULL );

			memset( mission->goals, 0, mission->num_goals * sizeof(mgoal) );

			for (j = 0; j < mission->num_goals; j++) {
				cfread_string_len(mission->goals[j].name, NAME_LENGTH, cfp);
				mission->goals[j].status = cfread_char(cfp);
			}
		}

		// events
		mission->num_events = cfread_int(cfp);

		if (mission->num_events > 0) {
			mission->events = (mevent *) vm_malloc( mission->num_events * sizeof(mevent) );
			Verify( mission->events != NULL );

			memset( mission->events, 0, mission->num_events * sizeof(mevent) );

			for (j = 0; j < mission->num_events; j++) {
				cfread_string_len(mission->events[j].name, NAME_LENGTH, cfp);
				mission->events[j].status = cfread_char(cfp);
			}
		}

		// variables
		mission->num_variables = cfread_int(cfp);

		if (mission->num_variables > 0) {
			mission->variables = (sexp_variable *) vm_malloc( mission->num_variables * sizeof(sexp_variable) );
			Verify( mission->variables != NULL );

			memset( mission->variables, 0, mission->num_variables * sizeof(sexp_variable) );

			for (j = 0; j < mission->num_variables; j++) {
				mission->variables[j].type = cfread_int(cfp);
				cfread_string_len(mission->variables[j].text, TOKEN_LENGTH, cfp);
				cfread_string_len(mission->variables[j].variable_name, TOKEN_LENGTH, cfp);
			}
		}

		// scoring stats
		mission->stats.score = cfread_int(cfp);
		mission->stats.rank = cfread_int(cfp);
		mission->stats.assists = cfread_int(cfp);
		mission->stats.kill_count = cfread_int(cfp);
		mission->stats.kill_count_ok = cfread_int(cfp);
		mission->stats.bonehead_kills = cfread_int(cfp);

		mission->stats.p_shots_fired = cfread_uint(cfp);
		mission->stats.p_shots_hit = cfread_uint(cfp);
		mission->stats.p_bonehead_hits = cfread_uint(cfp);

		mission->stats.s_shots_fired = cfread_uint(cfp);
		mission->stats.s_shots_hit = cfread_uint(cfp);
		mission->stats.s_bonehead_hits = cfread_uint(cfp);

		// ship kills (scoring)
		list_size = (int)ship_list.size();
		for (j = 0; j < list_size; j++) {
			idx = cfread_int(cfp);

			if (ship_list[j].index >= 0) {
				mission->stats.kills[ship_list[j].index] = idx;
			}
		}

		// medals (scoring)
		list_size = (int)medals_list.size();
		for (j = 0; j < list_size; j++) {
			idx = cfread_int(cfp);

			if (medals_list[j].index >= 0) {
				mission->stats.medals[medals_list[j].index] = idx;
			}
		}
	}
}

void pilotfile::csg_write_missions()
{
	int idx, j;
	cmission *mission;

	startSection(Section::Missions);

	for (idx = 0; idx < MAX_CAMPAIGN_MISSIONS; idx++) {
		if (Campaign.missions[idx].completed) {
			mission = &Campaign.missions[idx];

			cfwrite_int(idx, cfp);

			// flags
			cfwrite_int(mission->flags, cfp);

			// goals
			cfwrite_int(mission->num_goals, cfp);

			for (j = 0; j < mission->num_goals; j++) {
				cfwrite_string_len(mission->goals[j].name, cfp);
				cfwrite_char(mission->goals[j].status, cfp);
			}

			// events
			cfwrite_int(mission->num_events, cfp);

			for (j = 0; j < mission->num_events; j++) {
				cfwrite_string_len(mission->events[j].name, cfp);
				cfwrite_char(mission->events[j].status, cfp);
			}

			// variables
			cfwrite_int(mission->num_variables, cfp);

			for (j = 0; j < mission->num_variables; j++) {
				cfwrite_int(mission->variables[j].type, cfp);
				cfwrite_string_len(mission->variables[j].text, cfp);
				cfwrite_string_len(mission->variables[j].variable_name, cfp);
			}

			// scoring stats
			cfwrite_int(mission->stats.score, cfp);
			cfwrite_int(mission->stats.rank, cfp);
			cfwrite_int(mission->stats.assists, cfp);
			cfwrite_int(mission->stats.kill_count, cfp);
			cfwrite_int(mission->stats.kill_count_ok, cfp);
			cfwrite_int(mission->stats.bonehead_kills, cfp);

			cfwrite_uint(mission->stats.p_shots_fired, cfp);
			cfwrite_uint(mission->stats.p_shots_hit, cfp);
			cfwrite_uint(mission->stats.p_bonehead_hits, cfp);

			cfwrite_uint(mission->stats.s_shots_fired, cfp);
			cfwrite_uint(mission->stats.s_shots_hit, cfp);
			cfwrite_uint(mission->stats.s_bonehead_hits, cfp);

			// ship kills (scoring)
			for (j = 0; j < Num_ship_classes; j++) {
				cfwrite_int(mission->stats.kills[j], cfp);
			}

			// medals earned (scoring)
			for (j = 0; j < (int)Medals.size(); j++) {
				cfwrite_int(mission->stats.medals[j], cfp);
			}
		}
	}

	endSection();
}

void pilotfile::csg_read_techroom()
{
	int idx, list_size = 0;
	ubyte visible;

	if ( !m_have_info ) {
		throw "Techroom before Info!";
	}

	// visible ships
	list_size = (int)ship_list.size();
	for (idx = 0; idx < list_size; idx++) {
		visible = cfread_ubyte(cfp);

		if (visible) {
			if (ship_list[idx].index >= 0) {
				Ship_info[ship_list[idx].index].flags |= SIF_IN_TECH_DATABASE;
			} else {
				m_data_invalid = true;
			}
		}
	}

	// visible weapons
	list_size = (int)weapon_list.size();
	for (idx = 0; idx < list_size; idx++) {
		visible = cfread_ubyte(cfp);

		if (visible) {
			if (weapon_list[idx].index >= 0) {
				Weapon_info[weapon_list[idx].index].wi_flags |= WIF_IN_TECH_DATABASE;
			} else {
				m_data_invalid = true;
			}
		}
	}

	// visible intel entries
	list_size = (int)intel_list.size();
	for (idx = 0; idx < list_size; idx++) {
		visible = cfread_ubyte(cfp);

		if (visible) {
			if (intel_list[idx].index >= 0) {
				Intel_info[intel_list[idx].index].flags |= IIF_IN_TECH_DATABASE;
			} else {
				m_data_invalid = true;
			}
		}
	}

	// if anything we need/use was missing then it should be considered fatal
	if (m_data_invalid) {
		throw "Invalid data for CSG!";
	}
}

void pilotfile::csg_write_techroom()
{
	int idx;
	ubyte visible;

	startSection(Section::Techroom);

	// visible ships
	for (idx = 0; idx < Num_ship_classes; idx++) {
		// only visible if not in techroom by default
		if ( (Ship_info[idx].flags & SIF_IN_TECH_DATABASE) && !(Ship_info[idx].flags2 & SIF2_DEFAULT_IN_TECH_DATABASE) ) {
			visible = 1;
		} else {
			visible = 0;
		}

		cfwrite_ubyte(visible, cfp);
	}

	// visible weapons
	for (idx = 0; idx < Num_weapon_types; idx++) {
		// only visible if not in techroom by default
		if ( (Weapon_info[idx].wi_flags & WIF_IN_TECH_DATABASE) && !(Weapon_info[idx].wi_flags2 & WIF2_DEFAULT_IN_TECH_DATABASE) ) {
			visible = 1;
		} else {
			visible = 0;
		}

		cfwrite_ubyte(visible, cfp);
	}

	// visible intel entries
	for (idx = 0; idx < Intel_info_size; idx++) {
		// only visible if not in techroom by default
		if ( (Intel_info[idx].flags & IIF_IN_TECH_DATABASE) && !(Intel_info[idx].flags & IIF_DEFAULT_IN_TECH_DATABASE) ) {
			visible = 1;
		} else {
			visible = 0;
		}

		cfwrite_ubyte(visible, cfp);
	}

	endSection();
}

void pilotfile::csg_read_loadout()
{
	int j;
	int count;
	uint idx, list_size = 0;

	if ( !m_have_info ) {
		throw "Loadout before Info!";
	}

	// base info
	cfread_string_len(Player_loadout.filename, MAX_FILENAME_LEN, cfp);
	cfread_string_len(Player_loadout.last_modified, DATE_TIME_LENGTH, cfp);

	// ship pool
	list_size = ship_list.size();
	for (idx = 0; idx < list_size; idx++) {
		count = cfread_int(cfp);

		if (ship_list[idx].index >= 0) {
			Player_loadout.ship_pool[ship_list[idx].index] = count;
		}
	}

	// weapon pool
	list_size = weapon_list.size();
	for (idx = 0; idx < list_size; idx++) {
		count = cfread_int(cfp);

		if (weapon_list[idx].index >= 0) {
			Player_loadout.weapon_pool[weapon_list[idx].index] = count;
		}
	}

	// player ship loadout
	list_size = (uint)cfread_ushort(cfp);
	for (uint i = 0; i < list_size; i++) {
		wss_unit *slot = NULL;

		if (i < MAX_WSS_SLOTS) {
			slot = &Player_loadout.unit_data[i];
		}

		// ship
		idx = cfread_int(cfp);

		if (idx > ship_list.size()) {
			//mprintf(("CSG => Parse Warning: Invalid value for ship index (%d), Clamping to valid range.\n", idx));	
			 idx = ship_list.size();						
		}

		if (slot) {
			slot->ship_class = ship_list[idx].index;
		}

		// primary weapons
		count = cfread_int(cfp);

		for (j = 0; j < count; j++) {
			idx = cfread_int(cfp);

			if (idx > weapon_list.size()) {
				//mprintf(("CSG => Parse Warning: Invalid value for weapon index (%d), Clamping to valid range.\n", idx));	
				idx = weapon_list.size();
			}


			if ( slot && (j < MAX_SHIP_PRIMARY_BANKS) ) {
				slot->wep[j] = weapon_list[idx].index;
			}

			idx = cfread_int(cfp);

			if ( slot && (j < MAX_SHIP_PRIMARY_BANKS) ) {
				slot->wep_count[j] = idx;
			}
		}

		// secondary weapons
		count = cfread_int(cfp);

		for (j = 0; j < count; j++) {
			idx = cfread_int(cfp);

			if (idx > weapon_list.size()) {
				//mprintf(("CSG => Parse Warning: Invalid value for weapon index (%d), Clamping to valid range.\n", idx));
				idx = weapon_list.size();		
			}

			if ( slot && (j < MAX_SHIP_SECONDARY_BANKS) ) {
				slot->wep[j+MAX_SHIP_PRIMARY_BANKS] = weapon_list[idx].index;
			}

			idx = cfread_int(cfp);

			if ( slot && (j < MAX_SHIP_SECONDARY_BANKS) ) {
				slot->wep_count[j+MAX_SHIP_PRIMARY_BANKS] = idx;
			}
		}
	}	
}

void pilotfile::csg_write_loadout()
{
	int idx, j;

	startSection(Section::Loadout);

	// base info
	cfwrite_string_len(Player_loadout.filename, cfp);
	cfwrite_string_len(Player_loadout.last_modified, cfp);

	// ship pool
	for (idx = 0; idx < Num_ship_classes; idx++) {
		cfwrite_int(Player_loadout.ship_pool[idx], cfp);
	}

	// weapon pool
	for (idx = 0; idx < Num_weapon_types; idx++) {
		cfwrite_int(Player_loadout.weapon_pool[idx], cfp);
	}

	// play ship loadout
	cfwrite_ushort(MAX_WSS_SLOTS, cfp);

	for (idx = 0; idx < MAX_WSS_SLOTS; idx++) {
		wss_unit *slot = &Player_loadout.unit_data[idx];

		// ship
		cfwrite_int(slot->ship_class, cfp);

		// primary weapons
		cfwrite_int(MAX_SHIP_PRIMARY_BANKS, cfp);

		for (j = 0; j < MAX_SHIP_PRIMARY_BANKS; j++) {
			cfwrite_int(slot->wep[j], cfp);
			cfwrite_int(slot->wep_count[j], cfp);
		}

		// secondary weapons
		cfwrite_int(MAX_SHIP_SECONDARY_BANKS, cfp);

		for (j = 0; j < MAX_SHIP_SECONDARY_BANKS; j++) {
			cfwrite_int(slot->wep[j+MAX_SHIP_PRIMARY_BANKS], cfp);
			cfwrite_int(slot->wep_count[j+MAX_SHIP_PRIMARY_BANKS], cfp);
		}
	}

	endSection();
}

void pilotfile::csg_read_stats()
{
	int idx, list_size = 0;
	int count;

	if ( !m_have_info ) {
		throw "Stats before Info!";
	}

	// scoring stats
	p->stats.score = cfread_int(cfp);
	p->stats.rank = cfread_int(cfp);
	p->stats.assists = cfread_int(cfp);
	p->stats.kill_count = cfread_int(cfp);
	p->stats.kill_count_ok = cfread_int(cfp);
	p->stats.bonehead_kills = cfread_int(cfp);

	p->stats.p_shots_fired = cfread_uint(cfp);
	p->stats.p_shots_hit = cfread_uint(cfp);
	p->stats.p_bonehead_hits = cfread_uint(cfp);

	p->stats.s_shots_fired = cfread_uint(cfp);
	p->stats.s_shots_hit = cfread_uint(cfp);
	p->stats.s_bonehead_hits = cfread_uint(cfp);

	p->stats.flight_time = cfread_uint(cfp);
	p->stats.missions_flown = cfread_uint(cfp);
	p->stats.last_flown = (_fs_time_t)cfread_int(cfp);
	p->stats.last_backup = (_fs_time_t)cfread_int(cfp);

	// ship kills (scoring)
	list_size = (int)ship_list.size();
	for (idx = 0; idx < list_size; idx++) {
		count = cfread_int(cfp);

		if (ship_list[idx].index >= 0) {
			p->stats.kills[ship_list[idx].index] = count;
		}
	}

	// medals earned (scoring)
	list_size = (int)medals_list.size();
	for (idx = 0; idx < list_size; idx++) {
		count = cfread_int(cfp);

		if (medals_list[idx].index >= 0) {
			p->stats.medals[medals_list[idx].index] = count;
		}
	}
}

void pilotfile::csg_write_stats()
{
	int idx;

	startSection(Section::Scoring);

	// scoring stats
	cfwrite_int(p->stats.score, cfp);
	cfwrite_int(p->stats.rank, cfp);
	cfwrite_int(p->stats.assists, cfp);
	cfwrite_int(p->stats.kill_count, cfp);
	cfwrite_int(p->stats.kill_count_ok, cfp);
	cfwrite_int(p->stats.bonehead_kills, cfp);

	cfwrite_uint(p->stats.p_shots_fired, cfp);
	cfwrite_uint(p->stats.p_shots_hit, cfp);
	cfwrite_uint(p->stats.p_bonehead_hits, cfp);

	cfwrite_uint(p->stats.s_shots_fired, cfp);
	cfwrite_uint(p->stats.s_shots_hit, cfp);
	cfwrite_uint(p->stats.s_bonehead_hits, cfp);

	cfwrite_uint(p->stats.flight_time, cfp);
	cfwrite_uint(p->stats.missions_flown, cfp);
	cfwrite_int((int)p->stats.last_flown, cfp);
	cfwrite_int((int)p->stats.last_backup, cfp);

	// ship kills (scoring)
	for (idx = 0; idx < Num_ship_classes; idx++) {
		cfwrite_int(p->stats.kills[idx], cfp);
	}

	// medals earned (scoring)
	for (idx = 0; idx < (int)Medals.size(); idx++) {
		cfwrite_int(p->stats.medals[idx], cfp);
	}

	endSection();
}

void pilotfile::csg_read_redalert()
{
	int idx, i, j, list_size = 0;
	int count;
	char t_string[MAX_FILENAME_LEN+NAME_LENGTH+1] = { '\0' };
	float hit;
	wep_t weapons;

	if ( !m_have_info ) {
		throw "RedAlert before Info!";
	}

	list_size = cfread_int(cfp);

	if (list_size <= 0) {
		return;
	}

	// about to read new redalert data so flush any existing data
	// otherwise wingman entries will multiply like rabbits
	Red_alert_wingman_status.clear();

	cfread_string_len(t_string, MAX_FILENAME_LEN, cfp);

	Red_alert_precursor_mission = t_string;

	for (idx = 0; idx < list_size; idx++) {
		red_alert_ship_status ras;

		cfread_string_len(t_string, NAME_LENGTH, cfp);
		ras.name = t_string;

		ras.hull = cfread_float(cfp);

		// ship class, index into ship_list[]
		i = cfread_int(cfp);
		ras.ship_class = ship_list[i].index;

		// subsystem hits
		count = cfread_int(cfp);

		for (j = 0; j < count; j++) {
			hit = cfread_float(cfp);
			ras.subsys_current_hits.push_back( hit );
		}

		// subsystem aggregate hits
		count = cfread_int(cfp);

		for (j = 0; j < count; j++) {
			hit = cfread_float(cfp);
			ras.subsys_aggregate_current_hits.push_back( hit );
		}

		// primary weapon loadout and status
		count = cfread_int(cfp);

		for (j = 0; j < count; j++) {
			i = cfread_int(cfp);
			weapons.index = weapon_list[i].index;
			weapons.count = cfread_int(cfp);

			// triggering this means something is really fubar
			if (weapons.index < 0) {
				continue;
			}

			ras.primary_weapons.push_back( weapons );
		}

		// secondary weapon loadout and status
		count = cfread_int(cfp);

		for (j = 0; j < count; j++) {
			i = cfread_int(cfp);
			weapons.index = weapon_list[i].index;
			weapons.count = cfread_int(cfp);

			// triggering this means something is really fubar
			if (weapons.index < 0) {
				continue;
			}

			ras.secondary_weapons.push_back( weapons );
		}

		// this is quite likely a *bad* thing if it happens
		if (ras.ship_class >= 0) {
			Red_alert_wingman_status.push_back( ras );
		}
	}
}

void pilotfile::csg_write_redalert()
{
	int idx, j, list_size = 0;
	int count;
	red_alert_ship_status *ras;

	startSection(Section::RedAlert);

	list_size = (int)Red_alert_wingman_status.size();

	cfwrite_int(list_size, cfp);

	if (list_size) {
		cfwrite_string_len(Red_alert_precursor_mission.c_str(), cfp);

		for (idx = 0; idx < list_size; idx++) {
			ras = &Red_alert_wingman_status[idx];

			cfwrite_string_len(ras->name.c_str(), cfp);

			cfwrite_float(ras->hull, cfp);

			// ship class, should be index into ship_list[] on load
			cfwrite_int(ras->ship_class, cfp);

			// subsystem hits
			count = (int)ras->subsys_current_hits.size();
			cfwrite_int(count, cfp);

			for (j = 0; j < count; j++) {
				cfwrite_float(ras->subsys_current_hits[j], cfp);
			}

			// subsystem aggregate hits
			count = (int)ras->subsys_aggregate_current_hits.size();
			cfwrite_int(count, cfp);

			for (j = 0; j < count; j++) {
				cfwrite_float(ras->subsys_aggregate_current_hits[j], cfp);
			}

			// primary weapon loadout and status
			count = (int)ras->primary_weapons.size();
			cfwrite_int(count, cfp);

			for (j = 0; j < count; j++) {
				cfwrite_int(ras->primary_weapons[j].index, cfp);
				cfwrite_int(ras->primary_weapons[j].count, cfp);
			}

			// secondary weapon loadout and status
			count = (int)ras->secondary_weapons.size();
			cfwrite_int(count, cfp);

			for (j = 0; j < count; j++) {
				cfwrite_int(ras->secondary_weapons[j].index, cfp);
				cfwrite_int(ras->secondary_weapons[j].count, cfp);
			}
		}
	}

	endSection();
}

void pilotfile::csg_read_hud()
{
	int idx;

	// flags
	HUD_config.show_flags = cfread_int(cfp);
	HUD_config.show_flags2 = cfread_int(cfp);

	HUD_config.popup_flags = cfread_int(cfp);
	HUD_config.popup_flags2 = cfread_int(cfp);

	// settings
	HUD_config.num_msg_window_lines = cfread_ubyte(cfp);

	HUD_config.rp_flags = cfread_int(cfp);
	HUD_config.rp_dist = cfread_int(cfp);

	// basic colors
	HUD_config.main_color = cfread_int(cfp);
	HUD_color_alpha = cfread_int(cfp);

	if (HUD_color_alpha < HUD_COLOR_ALPHA_USER_MIN) {
		HUD_color_alpha = HUD_COLOR_ALPHA_DEFAULT;
	}

	hud_config_record_color(HUD_config.main_color);

	// gauge-specific colors
	int num_gauges = cfread_int(cfp);

	for (idx = 0; idx < num_gauges; idx++) {
		ubyte red = cfread_ubyte(cfp);
		ubyte green = cfread_ubyte(cfp);
		ubyte blue = cfread_ubyte(cfp);
		ubyte alpha = cfread_ubyte(cfp);

		if (idx >= NUM_HUD_GAUGES) {
			continue;
		}

		HUD_config.clr[idx].red = red;
		HUD_config.clr[idx].green = green;
		HUD_config.clr[idx].blue = blue;
		HUD_config.clr[idx].alpha = alpha;
	}
}

void pilotfile::csg_write_hud()
{
	int idx;

	startSection(Section::HUD);

	// flags
	cfwrite_int(HUD_config.show_flags, cfp);
	cfwrite_int(HUD_config.show_flags2, cfp);

	cfwrite_int(HUD_config.popup_flags, cfp);
	cfwrite_int(HUD_config.popup_flags2, cfp);

	// settings
	cfwrite_ubyte(HUD_config.num_msg_window_lines, cfp);

	cfwrite_int(HUD_config.rp_flags, cfp);
	cfwrite_int(HUD_config.rp_dist, cfp);

	// basic colors
	cfwrite_int(HUD_config.main_color, cfp);
	cfwrite_int(HUD_color_alpha, cfp);

	// gauge-specific colors
	cfwrite_int(NUM_HUD_GAUGES, cfp);

	for (idx = 0; idx < NUM_HUD_GAUGES; idx++) {
		cfwrite_ubyte(HUD_config.clr[idx].red, cfp);
		cfwrite_ubyte(HUD_config.clr[idx].green, cfp);
		cfwrite_ubyte(HUD_config.clr[idx].blue, cfp);
		cfwrite_ubyte(HUD_config.clr[idx].alpha, cfp);
	}

	endSection();
}

void pilotfile::csg_read_variables()
{
	int idx;

	Campaign.num_variables = cfread_int(cfp);

	if (Campaign.num_variables > 0) {
		Campaign.variables = (sexp_variable *) vm_malloc( Campaign.num_variables * sizeof(sexp_variable) );
		Verify( Campaign.variables != NULL );

		memset( Campaign.variables, 0, Campaign.num_variables * sizeof(sexp_variable) );

		for (idx = 0; idx < Campaign.num_variables; idx++) {
			Campaign.variables[idx].type = cfread_int(cfp);
			cfread_string_len(Campaign.variables[idx].text, TOKEN_LENGTH, cfp);
			cfread_string_len(Campaign.variables[idx].variable_name, TOKEN_LENGTH, cfp);
		}
	}
}

void pilotfile::csg_write_variables()
{
	int idx;

	startSection(Section::Variables);

	cfwrite_int(Campaign.num_variables, cfp);

	for (idx = 0; idx < Campaign.num_variables; idx++) {
		cfwrite_int(Campaign.variables[idx].type, cfp);
		cfwrite_string_len(Campaign.variables[idx].text, cfp);
		cfwrite_string_len(Campaign.variables[idx].variable_name, cfp);
	}

	endSection();
}

void pilotfile::csg_read_settings()
{
	// sound/voice/music
	Master_sound_volume = cfread_float(cfp);
	Master_event_music_volume = cfread_float(cfp);
	Master_voice_volume = cfread_float(cfp);

	audiostream_set_volume_all(Master_voice_volume, ASF_VOICE);
	audiostream_set_volume_all(Master_event_music_volume, ASF_EVENTMUSIC);

	if (Master_event_music_volume > 0.0f) {
		Event_music_enabled = 1;
	} else {
		Event_music_enabled = 0;
	}

	Briefing_voice_enabled = cfread_int(cfp);

	// skill level
	Game_skill_level = cfread_int(cfp);

	// input options
	Use_mouse_to_fly = cfread_int(cfp);
	Mouse_sensitivity = cfread_int(cfp);
	Joy_sensitivity = cfread_int(cfp);
	Dead_zone_size = cfread_int(cfp);

	// detail
	Detail.setting = cfread_int(cfp);
	Detail.nebula_detail = cfread_int(cfp);
	Detail.detail_distance = cfread_int(cfp);
	Detail.hardware_textures = cfread_int(cfp);
	Detail.num_small_debris = cfread_int(cfp);
	Detail.num_particles = cfread_int(cfp);
	Detail.num_stars = cfread_int(cfp);
	Detail.shield_effects = cfread_int(cfp);
	Detail.lighting = cfread_int(cfp);
	Detail.targetview_model = cfread_int(cfp);
	Detail.planets_suns = cfread_int(cfp);
	Detail.weapon_extras = cfread_int(cfp);
}

void pilotfile::csg_write_settings()
{
	startSection(Section::Settings);

	// sound/voice/music
	cfwrite_float(Master_sound_volume, cfp);
	cfwrite_float(Master_event_music_volume, cfp);
	cfwrite_float(Master_voice_volume, cfp);

	cfwrite_int(Briefing_voice_enabled, cfp);

	// skill level
	cfwrite_int(Game_skill_level, cfp);

	// input options
	cfwrite_int(Use_mouse_to_fly, cfp);
	cfwrite_int(Mouse_sensitivity, cfp);
	cfwrite_int(Joy_sensitivity, cfp);
	cfwrite_int(Dead_zone_size, cfp);

	// detail
	cfwrite_int(Detail.setting, cfp);
	cfwrite_int(Detail.nebula_detail, cfp);
	cfwrite_int(Detail.detail_distance, cfp);
	cfwrite_int(Detail.hardware_textures, cfp);
	cfwrite_int(Detail.num_small_debris, cfp);
	cfwrite_int(Detail.num_particles, cfp);
	cfwrite_int(Detail.num_stars, cfp);
	cfwrite_int(Detail.shield_effects, cfp);
	cfwrite_int(Detail.lighting, cfp);
	cfwrite_int(Detail.targetview_model, cfp);
	cfwrite_int(Detail.planets_suns, cfp);
	cfwrite_int(Detail.weapon_extras, cfp);

	endSection();
}

void pilotfile::csg_read_controls()
{
	int idx, list_size;
	short id1, id2, id3;

	list_size = (int)cfread_ushort(cfp);

	for (idx = 0; idx < list_size; idx++) {
		id1 = cfread_short(cfp);
		id2 = cfread_short(cfp);
		id3 = cfread_short(cfp);	// unused, at the moment

		if (idx < CCFG_MAX) {
			Control_config[idx].key_id = id1;
			Control_config[idx].joy_id = id2;
		}
	}
}

void pilotfile::csg_write_controls()
{
	int idx;

	startSection(Section::Controls);

	cfwrite_ushort(CCFG_MAX, cfp);

	for (idx = 0; idx < CCFG_MAX; idx++) {
		cfwrite_short(Control_config[idx].key_id, cfp);
		cfwrite_short(Control_config[idx].joy_id, cfp);
		// placeholder? for future mouse_id?
		cfwrite_short(-1, cfp);
	}

	endSection();
}

void pilotfile::csg_read_cutscenes() {
	size_t list_size = cfread_uint(cfp);

	for(size_t i = 0; i < list_size; i++) {
		char tempFilename[MAX_FILENAME_LEN];

		cfread_string_len(tempFilename, MAX_FILENAME_LEN, cfp);
		cutscene_mark_viewable(tempFilename);
	}
}

void pilotfile::csg_write_cutscenes() {
	startSection(Section::Cutscenes);

	size_t viewableScenes = 0;
	for(SCP_vector<cutscene_info>::iterator cut = Cutscenes.begin(); cut != Cutscenes.end(); ++cut) {
		if(cut->viewable)
			viewableScenes ++;
	}
	cfwrite_uint(viewableScenes, cfp);

	for(SCP_vector<cutscene_info>::iterator cut = Cutscenes.begin(); cut != Cutscenes.end(); ++cut) {
		if(cut->viewable)
			cfwrite_string_len(cut->filename, cfp);
	}

	endSection();
}

/*
 * Only used for quick start missions
 */
void pilotfile::csg_read_lastmissions()
{
	int i;

	// restore list of most recently played missions
	Num_recent_missions = cfread_int( cfp );
	Assert(Num_recent_missions <= MAX_RECENT_MISSIONS);
	for ( i = 0; i < Num_recent_missions; i++ ) {
		char *cp;

		cfread_string_len( Recent_missions[i], MAX_FILENAME_LEN, cfp);
		// Remove the extension (safety check: shouldn't exist anyway)
		cp = strchr(Recent_missions[i], '.');
			if (cp)
				*cp = 0;
	}
}

/*
 * Only used for quick start missions
 */

void pilotfile::csg_write_lastmissions()
{
	int i;

	startSection(Section::LastMissions);

	// store list of most recently played missions
	cfwrite_int(Num_recent_missions, cfp);
	for (i=0; i<Num_recent_missions; i++) {
		cfwrite_string_len(Recent_missions[i], cfp);
	}

	endSection();
}

void pilotfile::csg_reset_data()
{
	int idx;
	cmission *mission;

	// internals
	m_have_flags = false;
	m_have_info = false;

	m_data_invalid = false;

	// init stats
	init_scoring_element(&p->stats);

	// zero out allowed ships/weapons
	memset(Campaign.ships_allowed, 0, sizeof(Campaign.ships_allowed));
	memset(Campaign.weapons_allowed, 0, sizeof(Campaign.weapons_allowed));

	// reset campaign status
	Campaign.prev_mission = -1;
	Campaign.next_mission = -1;
	Campaign.num_missions_completed = 0;

	// techroom reset
	tech_reset_to_default();

	// clear out variables
	if (Campaign.variables) {
		Campaign.num_variables = 0;
		vm_free(Campaign.variables);
		Campaign.variables = NULL;
	}

	// clear out mission stuff
	for (idx = 0; idx < MAX_CAMPAIGN_MISSIONS; idx++) {
		mission = &Campaign.missions[idx];

		if (mission->goals) {
			mission->num_goals = 0;
			vm_free(mission->goals);
			mission->goals = NULL;
		}

		if (mission->events) {
			mission->num_events = 0;
			vm_free(mission->events);
			mission->events = NULL;
		}

		if (mission->variables) {
			mission->num_variables = 0;
			vm_free(mission->variables);
			mission->variables = NULL;
		}

		init_scoring_element(&mission->stats);
	}
}

void pilotfile::csg_close()
{
	if (cfp) {
		cfclose(cfp);
		cfp = NULL;
	}

	p = NULL;
	filename = "";

	ship_list.clear();
	weapon_list.clear();
	intel_list.clear();
	medals_list.clear();

	m_have_flags = false;
	m_have_info = false;
}

bool pilotfile::load_savefile(const char *campaign)
{
	char base[_MAX_FNAME] = { '\0' };
	std::ostringstream buf;

	if (Game_mode & GM_MULTIPLAYER) {
		return false;
	}

	if ( (campaign == NULL) || !strlen(campaign) ) {
		return false;
	}

	// set player ptr first thing
	Assert( (Player_num >= 0) && (Player_num < MAX_PLAYERS) );
	p = &Players[Player_num];

	// build up filename for the savefile...
	_splitpath((char*)campaign, NULL, NULL, base, NULL);

	buf << p->callsign << "." << base << ".csg";

	filename = buf.str().c_str();


	// we need to reset this early, in case open fails and we need to create
	m_data_invalid = false;

	// open it, hopefully...
	cfp = cfopen((char*)filename.c_str(), "rb", CFILE_NORMAL, CF_TYPE_PLAYERS);

	if ( !cfp ) {
		mprintf(("CSG => Unable to open '%s' for reading!\n", filename.c_str()));
		return false;
	}

	unsigned int csg_id = cfread_uint(cfp);

	if (csg_id != CSG_FILE_ID) {
		mprintf(("CSG => Invalid header id for '%s'!\n", filename.c_str()));
		csg_close();
		return false;
	}

	// version, should be able to just ignore it
	ubyte csg_ver = cfread_ubyte(cfp);

	mprintf(("CSG => Loading '%s' with version %d...\n", filename.c_str(), (int)csg_ver));

	csg_reset_data();

	// the point of all this: read in the CSG contents
	while ( !cfeof(cfp) ) {
		ushort section_id = cfread_ushort(cfp);
		uint section_size = cfread_uint(cfp);

		size_t start_pos = cftell(cfp);

		// safety, to help protect against long reads
		cf_set_max_read_len(cfp, section_size);

		try {
			switch (section_id) {
				case Section::Flags:
					mprintf(("CSG => Parsing:  Flags...\n"));
					m_have_flags = true;
					csg_read_flags();
					break;

				case Section::Info:
					mprintf(("CSG => Parsing:  Info...\n"));
					m_have_info = true;
					csg_read_info();
					break;

				case Section::Variables:
					mprintf(("CSG => Parsing:  Variables...\n"));
					csg_read_variables();
					break;

				case Section::HUD:
					mprintf(("CSG => Parsing:  HUD...\n"));
					csg_read_hud();
					break;

				case Section::RedAlert:
					mprintf(("CSG => Parsing:  RedAlert...\n"));
					csg_read_redalert();
					break;

				case Section::Scoring:
					mprintf(("CSG => Parsing:  Scoring...\n"));
					csg_read_stats();
					break;

				case Section::Loadout:
					mprintf(("CSG => Parsing:  Loadout...\n"));
					csg_read_loadout();
					break;

				case Section::Techroom:
					mprintf(("CSG => Parsing:  Techroom...\n"));
					csg_read_techroom();
					break;

				case Section::Missions:
					mprintf(("CSG => Parsing:  Missions...\n"));
					csg_read_missions();
					break;

				case Section::Settings:
					mprintf(("CSG => Parsing:  Settings...\n"));
					csg_read_settings();
					break;

				case Section::Controls:
					mprintf(("CSG => Parsing:  Controls...\n"));
					csg_read_controls();
					break;

				case Section::Cutscenes:
					mprintf(("CSG => Parsing:  Cutscenes...\n"));
					csg_read_cutscenes();
					break;

				case Section::LastMissions:
					mprintf(("CSG => Parsing:  Last Missions...\n"));
					csg_read_lastmissions();
					break;

				default:
					mprintf(("CSG => Skipping unknown section 0x%04x!\n", section_id));
					break;
			}
		} catch (cfile::max_read_length &msg) {
			// read to max section size, move to next section, discarding
			// extra/unknown data
			mprintf(("CSG => Warning: (0x%04x) %s\n", section_id, msg.what()));
		} catch (const char *err) {
			mprintf(("CSG => ERROR: %s\n", err));
			csg_close();
			return false;
		}

		// reset safety catch
		cf_set_max_read_len(cfp, 0);

		// skip to next section (if not already there)
		size_t offset_pos = (start_pos + section_size) - cftell(cfp);

		if (offset_pos) {
			mprintf(("CSG => Warning: (0x%04x) Short read, information may have been lost!\n", section_id));
			cfseek(cfp, offset_pos, CF_SEEK_CUR);
		}
	}

	mprintf(("CSG => Loading complete!\n"));

	// cleanup and return
	csg_close();

	return true;
}

bool pilotfile::save_savefile()
{
	char base[_MAX_FNAME] = { '\0' };
	std::ostringstream buf;

	if (Game_mode & GM_MULTIPLAYER) {
		return false;
	}

	// set player ptr first thing
	Assert( (Player_num >= 0) && (Player_num < MAX_PLAYERS) );
	p = &Players[Player_num];

	if ( !strlen(Campaign.filename) ) {
		return false;
	}

	// build up filename for the savefile...
	_splitpath(Campaign.filename, NULL, NULL, base, NULL);

	buf << p->callsign << "." << base << ".csg";

	filename = buf.str().c_str();

	// make sure that we can actually save this safely
	if (m_data_invalid) {
		mprintf(("CSG => Skipping save of '%s' due to invalid data check!\n", filename.c_str()));
		return false;
	}

	// validate the number of red alert entries
	// assertion before writing so that we don't corrupt the .csg by asserting halfway through writing
	// assertion should also prevent loss of major campaign progress
	// i.e. lose one mission, not several missions worth (in theory)
	Assertion(Red_alert_wingman_status.size() >= 0 && Red_alert_wingman_status.size() <= MAX_SHIPS, "Invalid number of Red_alert_wingman_status entries: %u\n", Red_alert_wingman_status.size());

	// open it, hopefully...
	cfp = cfopen((char*)filename.c_str(), "wb", CFILE_NORMAL, CF_TYPE_PLAYERS);

	if ( !cfp ) {
		mprintf(("CSG => Unable to open '%s' for saving!\n", filename.c_str()));
		return false;
	}

	// header and version
	cfwrite_int(CSG_FILE_ID, cfp);
	cfwrite_ubyte(CSG_VERSION, cfp);

	mprintf(("CSG => Saving '%s' with version %d...\n", filename.c_str(), (int)CSG_VERSION));

	// flags and info sections go first
	mprintf(("CSG => Saving:  Flags...\n"));
	csg_write_flags();
	mprintf(("CSG => Saving:  Info...\n"));
	csg_write_info();

	// everything else is next, not order specific
	mprintf(("CSG => Saving:  Missions...\n"));
	csg_write_missions();
	mprintf(("CSG => Saving:  Techroom...\n"));
	csg_write_techroom();
	mprintf(("CSG => Saving:  Loadout...\n"));
	csg_write_loadout();
	mprintf(("CSG => Saving:  Scoring...\n"));
	csg_write_stats();
	mprintf(("CSG => Saving:  RedAlert...\n"));
	csg_write_redalert();
	mprintf(("CSG => Saving:  HUD...\n"));
	csg_write_hud();
	mprintf(("CSG => Saving:  Variables...\n"));
	csg_write_variables();
	mprintf(("CSG => Saving:  Settings...\n"));
	csg_write_settings();
	mprintf(("CSG => Saving:  Controls...\n"));
	csg_write_controls();
	mprintf(("CSG => Saving:  Cutscenes...\n"));
	csg_write_cutscenes();
	mprintf(("CSG => Saving:  Last Missions...\n"));
	csg_write_lastmissions();

	// Done!
	mprintf(("CSG => Saving complete!\n"));

	csg_close();

	return true;
}

