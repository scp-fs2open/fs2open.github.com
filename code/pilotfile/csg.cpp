
#include "controlconfig/controlsconfig.h"
#include "controlconfig/presets.h"
#include "cutscene/cutscenes.h"
#include "freespace.h"
#include "gamesnd/eventmusic.h"
#include "hud/hudconfig.h"
#include "io/joy.h"
#include "io/mouse.h"
#include "menuui/techmenu.h"
#include "mission/missioncampaign.h"
#include "mission/missionload.h"
#include "missionui/missionscreencommon.h"
#include "missionui/missionshipchoice.h"
#include "options/OptionsManager.h"
#include "parse/sexp_container.h"
#include "pilotfile/pilotfile.h"
#include "pilotfile/plr_hudprefs.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "sound/audiostr.h"
#include "stats/medals.h"
#include "utils/string_utils.h"
#include "weapon/weapon.h"

#define REDALERT_INTERNAL
#include "missionui/redalert.h"

#include <iostream>
#include <sstream>
#include <limits>

// this is kinda tricky
enum class TechroomState : ubyte
{
	DEFAULT = 0,
	ADDED = 1,
	REMOVED = 2
};

void pilotfile::csg_read_flags()
{
	// tips?
	p->tips = (int)cfread_ubyte(cfp);

	// avoid having to read everything to get the rank
	if (csg_ver >= 5) {
		p->stats.rank = cfread_int(cfp);
	}
}

void pilotfile::csg_write_flags()
{
	startSection(Section::Flags);

	// tips
	cfwrite_ubyte((ubyte)p->tips, cfp);

	// avoid having to read everything to get the rank
	cfwrite_int(p->stats.rank, cfp);

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

	// check that the next mission won't be greater than the total number of missions
	// though ensure we only flag if campaign exists and has been loaded
	if (Campaign.num_missions > 0 && Campaign.next_mission >= Campaign.num_missions) {
		Campaign.next_mission = 0; // Prevent trying to load from invalid mission data downstream
		m_data_invalid = true; // Causes a warning popup to be displayed
	}

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
				mprintf(("Found invalid ship \"%s\" in campaign save file. Skipping...\n", ship_list[idx].name.c_str()));
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
				mprintf(("Found invalid weapon \"%s\" in campaign save file. Skipping...\n",
				         weapon_list[idx].name.c_str()));
			}
		}
	}

	if (csg_ver >= 2) {
		// single/campaign squad name & image
		cfread_string_len(p->s_squad_name, NAME_LENGTH, cfp);
		cfread_string_len(p->s_squad_filename, MAX_FILENAME_LEN, cfp);
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
	cfwrite_int(ship_info_size(), cfp);

	for (auto &si : Ship_info) {
		cfwrite_string_len(si.name, cfp);
	}

	// weapon list
	cfwrite_int(weapon_info_size(), cfp);

	for (auto &wi : Weapon_info) {
		cfwrite_string_len(wi.name, cfp);
	}

	// intel list
	cfwrite_int(intel_info_size(), cfp);

	for (auto &ii : Intel_info) {
		cfwrite_string_len(ii.name, cfp);
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
	cfwrite_int(Campaign.loop_reentry, cfp);
	cfwrite_int(Campaign.loop_enabled, cfp);

	// missions completed
	cfwrite_int(Campaign.num_missions_completed, cfp);

	// allowed ships
	for (idx = 0; idx < ship_info_size(); idx++) {
		cfwrite_ubyte(Campaign.ships_allowed[idx], cfp);
	}

	// allowed weapons
	for (idx = 0; idx < weapon_info_size(); idx++) {
		cfwrite_ubyte(Campaign.weapons_allowed[idx], cfp);
	}

	// single/campaign squad name & image
	cfwrite_string_len(p->s_squad_name, cfp);
	cfwrite_string_len(p->s_squad_filename, cfp);

	endSection();
}

void pilotfile::csg_read_missions()
{
	int i, j, idx, list_size;
	cmission *missionp;

	if ( !m_have_info ) {
		throw "Missions before Info!";
	}

	for (i = 0; i < Campaign.num_missions_completed; i++) {
		idx = cfread_int(cfp);
		missionp = &Campaign.missions[idx];

		missionp->completed = 1;

		// flags
		missionp->flags = cfread_int(cfp);

		// goals
		missionp->goals.clear();
		int num_goals = cfread_int(cfp);

		for (j = 0; j < num_goals; j++) {
			missionp->goals.emplace_back();
			auto& stored_goal = missionp->goals.back();

			cfread_string_len(stored_goal.name, NAME_LENGTH, cfp);
			stored_goal.status = cfread_char(cfp);
		}

		// events
		missionp->events.clear();
		int num_events = cfread_int(cfp);

		for (j = 0; j < num_events; j++) {
			missionp->events.emplace_back();
			auto& stored_event = missionp->events.back();

			cfread_string_len(stored_event.name, NAME_LENGTH, cfp);
			stored_event.status = cfread_char(cfp);
		}

		// variables
		missionp->variables.clear();
		int num_variables = cfread_int(cfp);

		for (j = 0; j < num_variables; j++) {
			missionp->variables.emplace_back();
			auto& stored_variable = missionp->variables.back();

			stored_variable.type = cfread_int(cfp);
			cfread_string_len(missionp->variables[j].text, TOKEN_LENGTH, cfp);
			cfread_string_len(missionp->variables[j].variable_name, TOKEN_LENGTH, cfp);
		}

		// scoring stats
		missionp->stats.score = cfread_int(cfp);
		missionp->stats.rank = cfread_int(cfp);
		missionp->stats.assists = cfread_int(cfp);
		missionp->stats.kill_count = cfread_int(cfp);
		missionp->stats.kill_count_ok = cfread_int(cfp);
		missionp->stats.bonehead_kills = cfread_int(cfp);

		missionp->stats.p_shots_fired = cfread_uint(cfp);
		missionp->stats.p_shots_hit = cfread_uint(cfp);
		missionp->stats.p_bonehead_hits = cfread_uint(cfp);

		missionp->stats.s_shots_fired = cfread_uint(cfp);
		missionp->stats.s_shots_hit = cfread_uint(cfp);
		missionp->stats.s_bonehead_hits = cfread_uint(cfp);

		// ship kills (scoring)
		list_size = (int)ship_list.size();
		for (j = 0; j < list_size; j++) {
			idx = cfread_int(cfp);

			if (ship_list[j].index >= 0) {
				missionp->stats.kills[ship_list[j].index] = idx;
			}
		}

		// medals (scoring)
		list_size = (int)medals_list.size();
		for (j = 0; j < list_size; j++) {
			idx = cfread_int(cfp);

			if (medals_list[j].index >= 0) {
				missionp->stats.medal_counts[medals_list[j].index] = idx;
			}
		}
	}
}

void pilotfile::csg_write_missions()
{
	int idx, j;
	cmission *missionp;

	startSection(Section::Missions);

	for (idx = 0; idx < MAX_CAMPAIGN_MISSIONS; idx++) {
		if (Campaign.missions[idx].completed) {
			missionp = &Campaign.missions[idx];

			cfwrite_int(idx, cfp);

			// flags
			cfwrite_int(missionp->flags, cfp);

			// goals
			cfwrite_int((int)missionp->goals.size(), cfp);

			for (j = 0; j < (int)missionp->goals.size(); j++) {
				cfwrite_string_len(missionp->goals[j].name, cfp);
				cfwrite_char(missionp->goals[j].status, cfp);
			}

			// events
			cfwrite_int((int)missionp->events.size(), cfp);

			for (j = 0; j < (int)missionp->events.size(); j++) {
				cfwrite_string_len(missionp->events[j].name, cfp);
				cfwrite_char(missionp->events[j].status, cfp);
			}

			// variables
			cfwrite_int((int)missionp->variables.size(), cfp);

			for (j = 0; j < (int)missionp->variables.size(); j++) {
				cfwrite_int(missionp->variables[j].type, cfp);
				cfwrite_string_len(missionp->variables[j].text, cfp);
				cfwrite_string_len(missionp->variables[j].variable_name, cfp);
			}

			// scoring stats
			cfwrite_int(missionp->stats.score, cfp);
			cfwrite_int(missionp->stats.rank, cfp);
			cfwrite_int(missionp->stats.assists, cfp);
			cfwrite_int(missionp->stats.kill_count, cfp);
			cfwrite_int(missionp->stats.kill_count_ok, cfp);
			cfwrite_int(missionp->stats.bonehead_kills, cfp);

			cfwrite_uint(missionp->stats.p_shots_fired, cfp);
			cfwrite_uint(missionp->stats.p_shots_hit, cfp);
			cfwrite_uint(missionp->stats.p_bonehead_hits, cfp);

			cfwrite_uint(missionp->stats.s_shots_fired, cfp);
			cfwrite_uint(missionp->stats.s_shots_hit, cfp);
			cfwrite_uint(missionp->stats.s_bonehead_hits, cfp);

			// ship kills (scoring)
			for (j = 0; j < ship_info_size(); j++) {
				cfwrite_int(missionp->stats.kills[j], cfp);
			}

			// medals earned (scoring)
			for (j = 0; j < (int)Medals.size(); j++) {
				cfwrite_int(missionp->stats.medal_counts[j], cfp);
			}
		}
	}

	endSection();
}

void pilotfile::csg_read_techroom()
{
	int idx, list_size = 0;
	TechroomState state;

	if ( !m_have_info ) {
		throw "Techroom before Info!";
	}

	// visible ships
	list_size = (int)ship_list.size();
	for (idx = 0; idx < list_size; idx++) {
		state = (TechroomState) cfread_ubyte(cfp);

		if (state != TechroomState::DEFAULT) {
			if (ship_list[idx].index >= 0) {
				if (state == TechroomState::ADDED) {
					Ship_info[ship_list[idx].index].flags.set(Ship::Info_Flags::In_tech_database);
				} else if (state == TechroomState::REMOVED) {
					Ship_info[ship_list[idx].index].flags.remove(Ship::Info_Flags::In_tech_database);
				} else {
					mprintf(("Unrecognized techroom state: %d\n", (int) state));
				}
			} else {
				mprintf(("Found invalid ship \"%s\" in campaign save file. "
				         "Skipping...\n",
				         ship_list[idx].name.c_str()));
			}
		}
	}

	// visible weapons
	list_size = (int)weapon_list.size();
	for (idx = 0; idx < list_size; idx++) {
		state = (TechroomState) cfread_ubyte(cfp);

		if (state != TechroomState::DEFAULT) {
			if (weapon_list[idx].index >= 0) {
				if (state == TechroomState::ADDED) {
					Weapon_info[weapon_list[idx].index].wi_flags.set(Weapon::Info_Flags::In_tech_database);
				} else if (state == TechroomState::REMOVED) {
					Weapon_info[weapon_list[idx].index].wi_flags.remove(Weapon::Info_Flags::In_tech_database);
				} else {
					mprintf(("Unrecognized techroom state: %d\n", (int) state));
				}
			} else {
				mprintf(("Found invalid weapon \"%s\" in campaign save file. Skipping...\n",
				         weapon_list[idx].name.c_str()));
			}
		}
	}

	// visible intel entries
	list_size = (int)intel_list.size();
	for (idx = 0; idx < list_size; idx++) {
		state = (TechroomState) cfread_ubyte(cfp);

		if (state != TechroomState::DEFAULT) {
			if (intel_list[idx].index >= 0) {
				if (state == TechroomState::ADDED) {
					Intel_info[intel_list[idx].index].flags |= IIF_IN_TECH_DATABASE;
				} else if (state == TechroomState::REMOVED) {
					Intel_info[intel_list[idx].index].flags &= ~IIF_IN_TECH_DATABASE;
				} else {
					mprintf(("Unrecognized techroom state: %d\n", (int) state));
				}
			} else {
				mprintf(("Found invalid intel entry \"%s\" in campaign save file. Skipping...\n",
				         intel_list[idx].name.c_str()));
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
	TechroomState state;

	startSection(Section::Techroom);

	// write whether it differs from the default for ships
	for (auto &si : Ship_info) {
		if (si.flags[Ship::Info_Flags::In_tech_database] == si.flags[Ship::Info_Flags::Default_in_tech_database]) {
			state = TechroomState::DEFAULT;
		} else if (si.flags[Ship::Info_Flags::In_tech_database]) {
			state = TechroomState::ADDED;
		} else {
			state = TechroomState::REMOVED;
		}

		cfwrite_ubyte((ubyte) state, cfp);
	}

	// and for weapons
	for (auto &wi : Weapon_info) {
		if (wi.wi_flags[Weapon::Info_Flags::In_tech_database] == wi.wi_flags[Weapon::Info_Flags::Default_in_tech_database]) {
			state = TechroomState::DEFAULT;
		} else if (wi.wi_flags[Weapon::Info_Flags::In_tech_database]) {
			state = TechroomState::ADDED;
		} else {
			state = TechroomState::REMOVED;
		}

		cfwrite_ubyte((ubyte) state, cfp);
	}

	// and for intel entries
	for (auto &ii : Intel_info) {
		if (((ii.flags & IIF_IN_TECH_DATABASE) != 0) == ((ii.flags & IIF_DEFAULT_IN_TECH_DATABASE) != 0)) {
			state = TechroomState::DEFAULT;
		} else if ((ii.flags & IIF_IN_TECH_DATABASE) != 0) {
			state = TechroomState::ADDED;
		} else {
			state = TechroomState::REMOVED;
		}

		cfwrite_ubyte((ubyte) state, cfp);
	}

	endSection();
}

void pilotfile::csg_read_loadout()
{
	int j, count, ship_idx = -1, wep_idx = -1;
	size_t idx, list_size = 0;

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
		ship_idx = cfread_int(cfp);

		if ( (ship_idx >= (int)ship_list.size()) || (ship_idx < -1) ) { // on the casts, assume that ship & weapon lists will never exceed ~2 billion
			mprintf(("CSG => Parse Warning: Invalid value for ship index (%d), emptying slot.\n", ship_idx));
			ship_idx = -1;
		}

		if (slot) {
			if (ship_idx == -1) { // -1 means no ship in this slot
				slot->ship_class = -1;
			} else {
				slot->ship_class = ship_list[ship_idx].index;
			}
		}

		// primary weapons
		count = cfread_int(cfp);

		for (j = 0; j < count; j++) {
			wep_idx = cfread_int(cfp);

			if ( (wep_idx >= (int)weapon_list.size()) || (wep_idx < -1) ) {
				mprintf(("CSG => Parse Warning: Invalid value for primary weapon index (%d), emptying slot.\n", wep_idx));
				wep_idx = -1;
			}


			if ( slot && (j < MAX_SHIP_PRIMARY_BANKS) ) {
				if (wep_idx == -1) { // -1 means no weapon in this slot
					slot->wep[j] = -1;
				} else {
					slot->wep[j] = weapon_list[wep_idx].index;
				}
			}

			int read_idx = cfread_int(cfp);

			if ( slot && (j < MAX_SHIP_PRIMARY_BANKS) ) {
				slot->wep_count[j] = read_idx;
			}
		}

		// secondary weapons
		count = cfread_int(cfp);

		for (j = 0; j < count; j++) {
			wep_idx = cfread_int(cfp);

			if ( (wep_idx >= (int)weapon_list.size()) || (wep_idx < -1) ) {
				mprintf(("CSG => Parse Warning: Invalid value for secondary weapon index (%d), emptying slot.\n", wep_idx));
				wep_idx = -1;
			}

			if ( slot && (j < MAX_SHIP_SECONDARY_BANKS) ) {
				if (wep_idx == -1) { // -1 means no weapon in this slot
					slot->wep[j+MAX_SHIP_PRIMARY_BANKS] = -1;
				} else {
					slot->wep[j+MAX_SHIP_PRIMARY_BANKS] = weapon_list[wep_idx].index;
				}
			}

			int read_idx = cfread_int(cfp);

			if ( slot && (j < MAX_SHIP_SECONDARY_BANKS) ) {
				slot->wep_count[j+MAX_SHIP_PRIMARY_BANKS] = read_idx;
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
	for (idx = 0; idx < ship_info_size(); idx++) {
		cfwrite_int(Player_loadout.ship_pool[idx], cfp);
	}

	// weapon pool
	for (idx = 0; idx < weapon_info_size(); idx++) {
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
			p->stats.medal_counts[medals_list[idx].index] = count;
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
	for (idx = 0; idx < ship_info_size(); idx++) {
		cfwrite_int(p->stats.kills[idx], cfp);
	}

	// medals earned (scoring)
	for (idx = 0; idx < (int)Medals.size(); idx++) {
		cfwrite_int(p->stats.medal_counts[idx], cfp);
	}

	endSection();
}

void pilotfile::csg_read_redalert()
{
	int idx, i, j, ship_list_size = 0, wing_list_size = 0;
	int count;
	char t_string[MAX_FILENAME_LEN+NAME_LENGTH+1] = { '\0' };
	float hit;
	wep_t weapons;

	if ( !m_have_info ) {
		throw "RedAlert before Info!";
	}

	ship_list_size = cfread_int(cfp);

	if (ship_list_size > 0) {
		cfread_string_len(t_string, MAX_FILENAME_LEN, cfp);

		Red_alert_precursor_mission = t_string;

		for (idx = 0; idx < ship_list_size; idx++) {
			red_alert_ship_status ras;

			cfread_string_len(t_string, NAME_LENGTH, cfp);
			ras.name = t_string;

			ras.hull = cfread_float(cfp);

			// ship class, index into ship_list[]
			i = cfread_int(cfp);
			if ( (i >= (int)ship_list.size()) || (i < RED_ALERT_LOWEST_VALID_SHIP_CLASS) ) {
				mprintf(("CSG => Parse Warning: Invalid value for red alert ship index (%d), emptying slot.\n", i));
				ras.ship_class = RED_ALERT_DESTROYED_SHIP_CLASS;
			} else if ( (i < 0 ) && (i >= RED_ALERT_LOWEST_VALID_SHIP_CLASS) ) {  // ship destroyed/exited
				ras.ship_class = i;
			} else {
				ras.ship_class = ship_list[i].index;
			}

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

			// this is quite likely a *bad* thing if it doesn't happen
			if (ras.ship_class >= RED_ALERT_LOWEST_VALID_SHIP_CLASS) {
				Red_alert_ship_status.push_back( ras );
			}
		}
	}


	// old versions of CSG files do not store wing status
	if (csg_ver < 8) {
		return;
	}


	wing_list_size = cfread_int(cfp);

	if (wing_list_size > 0) {
		for (idx = 0; idx < wing_list_size; idx++) {
			red_alert_wing_status rws;

			cfread_string_len(t_string, NAME_LENGTH, cfp);
			rws.name = t_string;

			rws.latest_wave = cfread_int(cfp);

			rws.wave_count = cfread_int(cfp);
			rws.total_arrived_count = cfread_int(cfp);
			rws.total_departed = cfread_int(cfp);
			rws.total_destroyed = cfread_int(cfp);
			rws.total_vanished = cfread_int(cfp);

			Red_alert_wing_status.push_back(rws);
		}
	}

}

void pilotfile::csg_write_redalert()
{
	int idx, j, ship_list_size = 0, wing_list_size = 0;
	int count;

	startSection(Section::RedAlert);

	ship_list_size = (int)Red_alert_ship_status.size();

	cfwrite_int(ship_list_size, cfp);

	if (ship_list_size) {
		cfwrite_string_len(Red_alert_precursor_mission.c_str(), cfp);

		for (idx = 0; idx < ship_list_size; idx++) {
			auto ras = &Red_alert_ship_status[idx];

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

	wing_list_size = (int)Red_alert_wing_status.size();

	cfwrite_int(wing_list_size, cfp);

	if (wing_list_size) {
		for (idx = 0; idx < wing_list_size; idx++) {
			auto rws = &Red_alert_wing_status[idx];

			cfwrite_string_len(rws->name.c_str(), cfp);

			cfwrite_int(rws->latest_wave, cfp);

			cfwrite_int(rws->wave_count, cfp);
			cfwrite_int(rws->total_arrived_count, cfp);
			cfwrite_int(rws->total_departed, cfp);
			cfwrite_int(rws->total_destroyed, cfp);
			cfwrite_int(rws->total_vanished, cfp);
		}
	}

	endSection();
}

void pilotfile::csg_read_hud()
{
	const HC_gauge_mappings& gauge_map = HC_gauge_mappings::get_instance();
	
	int strikes = 0;

	// flags
	int show_flags = cfread_int(cfp);
	int show_flags2 = cfread_int(cfp);

	int popup_flags = cfread_int(cfp);
	int popup_flags2 = cfread_int(cfp);

	// Convert show_flags (0-31) and show_flags2 (32-63)
	for (int i = 0; i < 64; i++) {
		bool is_set = (i < 32) ? (show_flags & (1 << i)) : (show_flags2 & (1 << (i - 32)));
		SCP_string gauge_id = gauge_map.get_string_id_from_numeric_id(i);

		if (!gauge_id.empty()) {
			HUD_config.set_gauge_visibility(gauge_id, is_set);
		}
	}

	// Convert popup_flags (0-31) and popup_flags2 (32-63)
	for (int i = 0; i < 64; i++) {
		bool is_set = (i < 32) ? (popup_flags & (1 << i)) : (popup_flags2 & (1 << (i - 32)));
		SCP_string gauge_id = gauge_map.get_string_id_from_numeric_id(i);

		if (!gauge_id.empty()) {
			HUD_config.set_gauge_popup(gauge_id, is_set);
		}
	}

	// settings
	SCP_UNUSED(cfread_ubyte(cfp));// Deprecated but still read for file compatibility 3/7/2025
	SCP_UNUSED(cfread_int(cfp));// Deprecated but still read for file compatibility 3/7/2025

	HUD_config.rp_dist = cfread_int(cfp);
	if (HUD_config.rp_dist < 0 || HUD_config.rp_dist >= RR_MAX_RANGES) {
		ReleaseWarning(LOCATION, "Campaign file has invalid radar range %d, setting to default.\n", HUD_config.rp_dist);
		HUD_config.rp_dist = RR_INFINITY;
		strikes++;
	}

	// basic colors
	HUD_config.main_color = cfread_int(cfp);
	if (HUD_config.main_color < 0 || HUD_config.main_color >= NUM_HUD_COLOR_PRESETS) {
		ReleaseWarning(LOCATION, "Campaign file has invalid main color selection %i, setting to default.\n", HUD_config.main_color);
		HUD_config.main_color = HUD_COLOR_PRESET_1;
		strikes++;
	}

	HUD_color_alpha = cfread_int(cfp);
	if (HUD_color_alpha < HUD_COLOR_ALPHA_USER_MIN || HUD_color_alpha > HUD_COLOR_ALPHA_USER_MAX) {
		ReleaseWarning(LOCATION, "Campaign file has invalid alpha color %i, setting to default.\n", HUD_color_alpha);
		HUD_color_alpha = HUD_COLOR_ALPHA_DEFAULT;
		strikes++;
	}

	if (strikes == 3) {
		ReleaseWarning(LOCATION, "Campaign file has too many hud config errors, and is likely corrupted. Please verify and save your settings in the hud config menu.");
	}

	hud_config_record_color(HUD_config.main_color);

	// gauge-specific colors
	int num_gauges = cfread_int(cfp);

	for (int idx = 0; idx < num_gauges; idx++) {
		ubyte red = cfread_ubyte(cfp);
		ubyte green = cfread_ubyte(cfp);
		ubyte blue = cfread_ubyte(cfp);
		ubyte alpha = cfread_ubyte(cfp);

		if (idx >= NUM_HUD_GAUGES) {
			continue;
		}

		SCP_string gauge_id = gauge_map.get_string_id_from_numeric_id(idx);
		if (!gauge_id.empty()) {
			color clr;
			gr_init_alphacolor(&clr, red, green, blue, alpha);
			HUD_config.set_gauge_color(gauge_id, clr);
		}
	}
}

void pilotfile::csg_write_hud()
{
	startSection(Section::HUD);

	// Get gauge mappings instance
	const HC_gauge_mappings& gauge_map = HC_gauge_mappings::get_instance();

	// Initialize bitfields
	int show_flags = 0, show_flags2 = 0;
	int popup_flags = 0, popup_flags2 = 0;

	// Convert show_flags_map to bitfield
	for (int i = 0; i < 64; i++) {
		SCP_string gauge_id = gauge_map.get_string_id_from_numeric_id(i);
		if (!gauge_id.empty() && HUD_config.is_gauge_visible(gauge_id)) {
			if (i < 32) {
				show_flags |= (1 << i);
			} else {
				show_flags2 |= (1 << (i - 32));
			}
		}
	}

	// Convert popup_flags_map to bitfield
	for (int i = 0; i < 64; i++) {
		SCP_string gauge_id = gauge_map.get_string_id_from_numeric_id(i);
		if (!gauge_id.empty() && HUD_config.is_gauge_popup(gauge_id)) {
			if (i < 32) {
				popup_flags |= (1 << i);
			} else {
				popup_flags2 |= (1 << (i - 32));
			}
		}
	}

	// flags
	cfwrite_int(show_flags, cfp);
	cfwrite_int(show_flags2, cfp);

	cfwrite_int(popup_flags, cfp);
	cfwrite_int(popup_flags2, cfp);

	// settings
	cfwrite_ubyte(0, cfp);// Deprecated but still written for file compatibility 3/7/2025
	cfwrite_int(0, cfp);// Deprecated but still written for file compatibility 3/7/2025

	cfwrite_int(HUD_config.rp_dist, cfp);

	// basic colors
	cfwrite_int(HUD_config.main_color, cfp);
	cfwrite_int(HUD_color_alpha, cfp);

	// gauge-specific colors
	cfwrite_int(NUM_HUD_GAUGES, cfp);

	for (int idx = 0; idx < NUM_HUD_GAUGES; idx++) {
		// Get the gauge string ID from numeric ID
		SCP_string gauge_id = gauge_map.get_string_id_from_numeric_id(idx);
		color clr = HUD_config.get_gauge_color(gauge_id);

		cfwrite_ubyte(clr.red, cfp);
		cfwrite_ubyte(clr.green, cfp);
		cfwrite_ubyte(clr.blue, cfp);
		cfwrite_ubyte(clr.alpha, cfp);
	}

	endSection();
}

void pilotfile::csg_read_variables()
{
	int idx;

	int num_variables = cfread_int(cfp);

	if (num_variables > 0) {

		for (idx = 0; idx < num_variables; idx++) {
			sexp_variable temp_var;
			memset(&temp_var, 0, sizeof(sexp_variable));
			temp_var.type = cfread_int(cfp);
			cfread_string_len(temp_var.text, TOKEN_LENGTH, cfp);
			cfread_string_len(temp_var.variable_name, TOKEN_LENGTH, cfp);
			Campaign.persistent_variables.push_back(temp_var);
		}
	}

	Campaign.red_alert_variables.clear();
	if (csg_ver < 4) { // CSG files before version 4 don't have a Red Alert set of CPVs to load, so just copy the regular set.
		for (auto& current_pv : Campaign.persistent_variables) {
			Campaign.red_alert_variables.push_back(current_pv);
		}
	} else {
		int redalert_num_variables = cfread_int(cfp);

		if (redalert_num_variables > 0) {
			for (idx = 0; idx < redalert_num_variables; idx++) {
				sexp_variable temp_var;
				memset(&temp_var, 0, sizeof(sexp_variable));
				temp_var.type = cfread_int(cfp);
				cfread_string_len(temp_var.text, TOKEN_LENGTH, cfp);
				cfread_string_len(temp_var.variable_name, TOKEN_LENGTH, cfp);
				Campaign.red_alert_variables.push_back(temp_var);
			}
		}
	}
}

void pilotfile::csg_write_variables()
{
	int idx;

	startSection(Section::Variables);

	cfwrite_int((int)Campaign.persistent_variables.size(), cfp);

	for (idx = 0; idx < (int)Campaign.persistent_variables.size(); idx++) {
		if (!(Campaign.persistent_variables[idx].type & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE)) {
			cfwrite_int(Campaign.persistent_variables[idx].type, cfp);
			cfwrite_string_len(Campaign.persistent_variables[idx].text, cfp);
			cfwrite_string_len(Campaign.persistent_variables[idx].variable_name, cfp);
		}
	}

	cfwrite_int((int)Campaign.red_alert_variables.size(), cfp);

	for (idx = 0; idx < (int)Campaign.red_alert_variables.size(); idx++) {
		cfwrite_int(Campaign.red_alert_variables[idx].type, cfp);
		cfwrite_string_len(Campaign.red_alert_variables[idx].text, cfp);
		cfwrite_string_len(Campaign.red_alert_variables[idx].variable_name, cfp);
	}

	endSection();
}

void pilotfile::csg_read_settings()
{
	clamped_range_warnings.clear();

	// sound/voice/music
	float temp_volume = cfread_float(cfp);
	clamp_value_with_warn(&temp_volume, 0.f, 1.f, "Effects Volume");
	snd_set_effects_volume(temp_volume);
	options::OptionsManager::instance()->set_ingame_range_option("Audio.Effects", Master_sound_volume);

	temp_volume = cfread_float(cfp);
	clamp_value_with_warn(&temp_volume, 0.f, 1.f, "Music Volume");
	event_music_set_volume(temp_volume);
	options::OptionsManager::instance()->set_ingame_range_option("Audio.Music", Master_event_music_volume);

	temp_volume = cfread_float(cfp);
	clamp_value_with_warn(&temp_volume, 0.f, 1.f, "Voice Volume");
	snd_set_voice_volume(temp_volume);
	options::OptionsManager::instance()->set_ingame_range_option("Audio.Voice", Master_voice_volume);

	Briefing_voice_enabled = cfread_int(cfp) != 0;
	options::OptionsManager::instance()->set_ingame_binary_option("Audio.BriefingVoice", Briefing_voice_enabled);


	// skill level
	Game_skill_level = cfread_int(cfp);
	clamp_value_with_warn(&Game_skill_level, 0, 4, "Game Skill Level");
	options::OptionsManager::instance()->set_ingame_range_option("Game.SkillLevel", Game_skill_level);

	// input options
	Use_mouse_to_fly   = cfread_int(cfp) != 0;
	options::OptionsManager::instance()->set_ingame_binary_option("Input.UseMouse", Use_mouse_to_fly);

	Mouse_sensitivity  = cfread_int(cfp);
	clamp_value_with_warn(&Mouse_sensitivity, 0, 9, "Mouse Sensitivity");
	options::OptionsManager::instance()->set_ingame_range_option("Input.MouseSensitivity", Mouse_sensitivity);

	Joy_sensitivity    = cfread_int(cfp);
	clamp_value_with_warn(&Joy_sensitivity, 0, 9, "Joystick Sensitivity");
	options::OptionsManager::instance()->set_ingame_range_option("Input.JoystickSensitivity", Joy_sensitivity);

	Joy_dead_zone_size = cfread_int(cfp);
	clamp_value_with_warn(&Joy_dead_zone_size, 0, 45, "Joystick Deadzone");
	options::OptionsManager::instance()->set_ingame_range_option("Input.JoystickDeadZone", Joy_dead_zone_size);

	if (csg_ver < 3) {
		// detail
		int dummy  __UNUSED = cfread_int(cfp);
		dummy = cfread_int(cfp);
		dummy = cfread_int(cfp);
		dummy = cfread_int(cfp);
		dummy = cfread_int(cfp);
		dummy = cfread_int(cfp);
		dummy = cfread_int(cfp);
		dummy = cfread_int(cfp);
		dummy = cfread_int(cfp);
		dummy = cfread_int(cfp);
		dummy = cfread_int(cfp);
		dummy = cfread_int(cfp);
	}
	if (!clamped_range_warnings.empty()) {
		ReleaseWarning(LOCATION, "The following values in the campaign save file were out of bounds and were automatically reset:\n%s\nPlease check your settings!\n", clamped_range_warnings.c_str());
		clamped_range_warnings.clear();
	}
}

void pilotfile::csg_write_settings()
{
	startSection(Section::Settings);
	clamped_range_warnings.clear();

	// sound/voice/music
	clamp_value_with_warn(&Master_sound_volume, 0.f, 1.f, "Effects Volume");
	cfwrite_float(Master_sound_volume, cfp);
	clamp_value_with_warn(&Master_event_music_volume, 0.f, 1.f, "Music Volume");
	cfwrite_float(Master_event_music_volume, cfp);
	clamp_value_with_warn(&Master_voice_volume, 0.f, 1.f, "Voice Volume");
	cfwrite_float(Master_voice_volume, cfp);

	cfwrite_int(Briefing_voice_enabled ? 1 : 0, cfp);

	// skill level
	clamp_value_with_warn(&Game_skill_level, 0, 4, "Game Skill Level");
	cfwrite_int(Game_skill_level, cfp);

	// input options
	cfwrite_int(Use_mouse_to_fly, cfp);
	clamp_value_with_warn(&Mouse_sensitivity, 0, 9, "Mouse Sensitivity");
	cfwrite_int(Mouse_sensitivity, cfp);
	clamp_value_with_warn(&Joy_sensitivity, 0, 9, "Joystick Sensitivity");
	cfwrite_int(Joy_sensitivity, cfp);
	clamp_value_with_warn(&Joy_dead_zone_size, 0, 45, "Joystick Deadzone");
	cfwrite_int(Joy_dead_zone_size, cfp);

	if (!clamped_range_warnings.empty()) {
		ReleaseWarning(LOCATION, "The following values were out of bounds when saving the campaign file and were automatically reset.\n%s\nThis shouldn't be possible, please contact the FreeSpace 2 Open Source Code Project!\n", clamped_range_warnings.c_str());
		clamped_range_warnings.clear();
	}
	endSection();
}

void pilotfile::csg_read_controls()
{
	if (csg_ver < 7) {
		// Pre CSG-7 compatibility
		int idx, list_size;
		short id1, id2, id3 __UNUSED;

		list_size = (int)cfread_ushort(cfp);

		for (idx = 0; idx < list_size; idx++) {
			id1 = cfread_short(cfp);
			id2 = cfread_short(cfp);
			id3 = cfread_short(cfp);	// unused, at the moment

			if (idx < CCFG_MAX) {
				Control_config[idx].take(CC_bind(CID_KEYBOARD, id1), 0);
				Control_config[idx].take(CC_bind(CID_JOY0, id2), 1);
			}
		}

		// Check that these bindings are in a preset.
		// CSG doesn't save invert flags, so use agnostic equals
		auto it = control_config_get_current_preset(true);
		if (it == Control_config_presets.end()) {
			// Not a preset, create one and its file
			CC_preset preset;
			preset.name = filename;

			// strip off extension
			auto n = preset.name.find_last_of('.');
			preset.name.resize(n);

			std::copy(Control_config.begin(), Control_config.end(), std::back_inserter(preset.bindings));
			Control_config_presets.push_back(preset);
			save_preset_file(preset, true);
		}
		return;
	
	} else {
		// >= CSG-7
		char buf[MAX_FILENAME_LEN];
		memset(buf, '\0', sizeof(buf));
		cfread_string(buf, sizeof(buf), cfp);

		auto it = std::find_if(Control_config_presets.begin(), Control_config_presets.end(),
		                       [&buf](const CC_preset& preset) { return preset.name == buf; });

		if (it == Control_config_presets.end()) {
			Assertion(!Control_config_presets.empty(), "[CSG] Error reading CSG! Control_config_presets empty; Get a coder!");
			// Couldn't find the preset, use defaults
			it = Control_config_presets.begin();
		}

		control_config_use_preset(*it);
		return;
	}
}

void pilotfile::csg_write_controls()
{
	auto it = control_config_get_current_preset();

	if (it == Control_config_presets.end()) {
		// Normally shouldn't happen, may be a new, blank player
		// First assert that the presets have been initialized and have at least the defaults preset
		Assertion(!Control_config_presets.empty(), "[CSG] Error saving CSG! Control_config_presets empty; Get a coder!");

		// Next, just bash the current preset to be defaults
		it = Control_config_presets.begin();
	}

	startSection(Section::Controls);

	cfwrite_string(it->name.c_str(), cfp);

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
	SCP_vector<cutscene_info>::iterator cut;

	startSection(Section::Cutscenes);

	size_t viewableScenes = 0;
	for (cut = Cutscenes.begin(); cut != Cutscenes.end(); ++cut) {
		if (cut->flags[Cutscene::Cutscene_Flags::Viewable]) {
			viewableScenes++;
		}
	}

	// Check for possible overflow because we can only write 32 bit integers
	Assertion(viewableScenes <= std::numeric_limits<uint>::max(), "Too many viewable cutscenes! Maximum is %ud!", std::numeric_limits<uint>::max());

	cfwrite_uint((uint)viewableScenes, cfp);

	for (cut = Cutscenes.begin(); cut != Cutscenes.end(); ++cut) {
		if (cut->flags[Cutscene::Cutscene_Flags::Viewable]) {
			cfwrite_string_len(cut->filename, cfp);
		}
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

void pilotfile::csg_read_containers()
{
	const int num_containers = cfread_int(cfp);

	for (int idx = 0; idx < num_containers; idx++) {
		Campaign.persistent_containers.emplace_back();
		auto& container = Campaign.persistent_containers.back();
		csg_read_container(container);
	}

	Campaign.red_alert_containers.clear();

	const int redalert_num_containers = cfread_int(cfp);

	for (int idx = 0; idx < redalert_num_containers; idx++) {
		Campaign.red_alert_containers.emplace_back();
		auto& ra_container = Campaign.red_alert_containers.back();
		csg_read_container(ra_container);
	}
}

void pilotfile::csg_read_container(sexp_container& container)
{
	char temp_buf[NAME_LENGTH];
	memset(temp_buf, 0, sizeof(temp_buf));

	cfread_string_len(temp_buf, sizeof(temp_buf), cfp);
	container.container_name = temp_buf;

	container.type = (ContainerType)cfread_int(cfp);
	container.opf_type = cfread_int(cfp);

	const int size = cfread_int(cfp);

	if (container.is_list()) {
		for (int i = 0; i < size; ++i) {
			cfread_string_len(temp_buf, sizeof(temp_buf), cfp);
			container.list_data.emplace_back(temp_buf);
		}
	} else if (container.is_map()) {
		char temp_key[NAME_LENGTH];
		memset(temp_key, 0, sizeof(temp_key));

		for (int i = 0; i < size; ++i) {
			cfread_string_len(temp_key, sizeof(temp_key), cfp);
			cfread_string_len(temp_buf, sizeof(temp_buf), cfp);
			container.map_data.emplace(temp_key, temp_buf);
		}
	} else {
		UNREACHABLE("Unknown container type %d", (int)container.type);
	}
}

void pilotfile::csg_write_containers()
{
	startSection(Section::Containers);

	cfwrite_int((int)Campaign.persistent_containers.size(), cfp);

	for (const auto& container : Campaign.persistent_containers) {
		Assert(!container.is_eternal()); // eternal containers should be written to player file
		csg_write_container(container);
	}

	cfwrite_int((int)Campaign.red_alert_containers.size(), cfp);

	for (const auto& ra_container : Campaign.red_alert_containers) {
		csg_write_container(ra_container);
	}

	endSection();
}

void pilotfile::csg_write_container(const sexp_container &container)
{
	cfwrite_string_len(container.container_name.c_str(), cfp);
	cfwrite_int((int)container.type, cfp);
	cfwrite_int(container.opf_type, cfp);

	cfwrite_int(container.size(), cfp);
	if (container.is_list()) {
		for (const auto& data : container.list_data) {
			cfwrite_string_len(data.c_str(), cfp);
		}
	} else if (container.is_map()) {
		for (const auto& key_data : container.map_data) {
			cfwrite_string_len(key_data.first.c_str(), cfp);
			cfwrite_string_len(key_data.second.c_str(), cfp);
		}
	} else {
		UNREACHABLE("Unknown container type %d", (int)container.type);
	}
}

void pilotfile::csg_reset_data(bool reset_ships_and_weapons)
{
	int idx;
	cmission *missionp;

	// internals
	m_have_flags = false;
	m_have_info = false;

	m_data_invalid = false;

	// init stats
	p->stats.init();

	// zero out allowed ships/weapons
	if (reset_ships_and_weapons) {
		memset(Campaign.ships_allowed, 0, sizeof(Campaign.ships_allowed));
		memset(Campaign.weapons_allowed, 0, sizeof(Campaign.weapons_allowed));
	}

	// reset campaign status
	Campaign.prev_mission = -1;
	Campaign.next_mission = 0;
	Campaign.num_missions_completed = 0;

	// techroom reset
	tech_reset_to_default();

	// clear out variables
	Campaign.persistent_variables.clear();
	Campaign.red_alert_variables.clear();

	// clear out containers
	Campaign.persistent_containers.clear();
	Campaign.red_alert_containers.clear();

	// clear red alert data
	Red_alert_ship_status.clear();
	Red_alert_wing_status.clear();

	// clear out mission stuff
	for (idx = 0; idx < MAX_CAMPAIGN_MISSIONS; idx++) {
		missionp = &Campaign.missions[idx];

		missionp->goals.clear();
		missionp->events.clear();
		missionp->variables.clear();

		missionp->stats.init();
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

	csg_ver = PLR_VERSION_INVALID;
}

bool pilotfile::load_savefile(player *_p, const char *campaign)
{
	SCP_string campaign_filename;

	if (Game_mode & GM_MULTIPLAYER) {
		return false;
	}

	if ( (campaign == NULL) || !strlen(campaign) ) {
		return false;
	}

	// set player ptr first thing
	Assert( (Player_num >= 0) && (Player_num < MAX_PLAYERS) );
	p = _p;

	auto base = util::get_file_part(campaign);
	// do a sanity check, but don't arbitrarily drop any extension in case the filename contains a period
	Assertion(!stristr(base, FS_CAMPAIGN_FILE_EXT), "The campaign should not have an extension at this point!");

	// build up filename for the savefile...
	sprintf(filename, NOX("%s.%s.csg"), p->callsign, base);

	// if campaign file doesn't exist, abort so we don't load irrelevant data
	campaign_filename = base;
	campaign_filename += FS_CAMPAIGN_FILE_EXT;
	if ( !cf_exists_full(campaign_filename.c_str(), CF_TYPE_MISSIONS) ) {
		mprintf(("CSG => Unable to find campaign file '%s'!\n", campaign_filename.c_str()));
		return false;
	}

	// we need to reset this early, in case open fails and we need to create
	m_data_invalid = false;

	// open it, hopefully...
	cfp = cfopen(filename.c_str(), "rb", CF_TYPE_PLAYERS, false,
	             CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

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

	// version, now used
	csg_ver = cfread_ubyte(cfp);

	mprintf(("CSG => Loading '%s' with version %d...\n", filename.c_str(), (int)csg_ver));

	csg_reset_data(true);

	// the point of all this: read in the CSG contents
	while ( !cfeof(cfp) ) {
		Section section_id = static_cast<Section>(cfread_ushort(cfp));
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

				case Section::Containers:
					mprintf(("CSG => Parsing:  Containers...\n"));
					csg_read_containers();
					break;

				default:
					mprintf(("CSG => Skipping unknown section 0x%04x!\n", (uint32_t)section_id));
					break;
			}
		} catch (cfile::max_read_length &msg) {
			// read to max section size, move to next section, discarding
			// extra/unknown data
			mprintf(("CSG => Warning: (0x%04x) %s\n", (uint32_t)section_id, msg.what()));
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
			mprintf(("CSG => Warning: (0x%04x) Short read, information may have been lost!\n", (uint32_t)section_id));
			cfseek(cfp, (int)offset_pos, CF_SEEK_CUR);
		}
	}

	mprintf(("HUDPREFS => Loading extended player HUD preferences...\n"));
	hud_config_load_player_prefs(p->callsign); 

	// Probably don't need to persist these to disk but it'll make sure on next boot we start with these campaign options set
	// The github tests don't know what to do with the ini file so I guess we'll skip this for now
	//options::OptionsManager::instance()->persistChanges();

	// if the campaign (for whatever reason) doesn't have a squad image, use the multi one
	if (p->s_squad_filename[0] == '\0') {
		strcpy_s(p->s_squad_filename, p->m_squad_filename);
	}
	player_set_squad_bitmap(p, p->s_squad_filename, false);

	mprintf(("CSG => Loading complete!\n"));

	// cleanup and return
	csg_close();

	return true;
}

bool pilotfile::save_savefile()
{
	if (Game_mode & GM_MULTIPLAYER) {
		return false;
	}

	// set player ptr first thing
	Assert( (Player_num >= 0) && (Player_num < MAX_PLAYERS) );
	p = &Players[Player_num];

	if ( !strlen(Campaign.filename) ) {
		return false;
	}

	auto base = util::get_file_part(Campaign.filename);
	// do a sanity check, but don't arbitrarily drop any extension in case the filename contains a period
	Assertion(!stristr(base, FS_CAMPAIGN_FILE_EXT), "The campaign should not have an extension at this point!");

	// build up filename for the savefile...
	sprintf(filename, NOX("%s.%s.csg"), p->callsign, base);

	// make sure that we can actually save this safely
	if (m_data_invalid) {
		mprintf(("CSG => Skipping save of '%s' due to invalid data check!\n", filename.c_str()));
		return false;
	}

	// validate the number of red alert entries
	// assertion before writing so that we don't corrupt the .csg by asserting halfway through writing
	// assertion should also prevent loss of major campaign progress
	// i.e. lose one mission, not several missions worth (in theory)
	Assertion(Red_alert_ship_status.size() <= MAX_SHIPS, "Invalid number of Red_alert_ship_status entries: " SIZE_T_ARG "\n", Red_alert_ship_status.size());
	Assertion(Red_alert_wing_status.size() <= MAX_WINGS, "Invalid number of Red_alert_wing_status entries: " SIZE_T_ARG "\n", Red_alert_wing_status.size());

	// open it, hopefully...
	cfp = cfopen(filename.c_str(), "wb", CF_TYPE_PLAYERS, false,
	             CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

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
	mprintf(("CSG => Saving:  Containers...\n"));
	csg_write_containers();

	mprintf(("HUDPREFS => Saving player HUD preferences (testing)...\n"));
	hud_config_save_player_prefs(p->callsign);

	// Done!
	mprintf(("CSG => Saving complete!\n"));

	csg_close();

	return true;
}

void pilotfile::clear_savefile(bool reset_ships_and_weapons)
{
	if (Game_mode & GM_MULTIPLAYER) {
		return;
	}

	// set player ptr first thing
	Assert((Player_num >= 0) && (Player_num < MAX_PLAYERS));
	p = &Players[Player_num];

	csg_reset_data(reset_ships_and_weapons);
}

/*
 * get_csg_rank: this function is called from plr.cpp & is
 * tightly linked with pilotfile::verify()
 */
bool pilotfile::get_csg_rank(int *rank)
{
	player t_csg;

	// set player ptr first thing
	p = &t_csg;

	// filename has already been set
	cfp = cfopen(filename.c_str(), "rb", CF_TYPE_PLAYERS, false,
	             CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	if ( !cfp ) {
		mprintf(("CSG => Unable to open '%s'!\n", filename.c_str()));
		return false;
	}

	unsigned int csg_id = cfread_uint(cfp);

	if (csg_id != CSG_FILE_ID) {
		mprintf(("CSG => Invalid header id for '%s'!\n", filename.c_str()));
		csg_close();
		return false;
	}

	// version, now used
	csg_ver = cfread_ubyte(cfp);

	mprintf(("CSG => Get Rank from '%s' with version %d...\n", filename.c_str(), (int)csg_ver));

	// the point of all this: read in the CSG contents
	while ( !m_have_flags && !cfeof(cfp) ) {
		Section section_id = static_cast<Section>(cfread_ushort(cfp));
		uint section_size = cfread_uint(cfp);

		size_t start_pos = cftell(cfp);
		size_t offset_pos;

		// safety, to help protect against long reads
		cf_set_max_read_len(cfp, section_size);

		try {
			switch (section_id) {
				case Section::Flags:
					mprintf(("CSG => Parsing:  Flags...\n"));
					m_have_flags = true;
					csg_read_flags();
					break;

				default:
					break;
			}
		} catch (cfile::max_read_length &msg) {
			// read to max section size, move to next section, discarding
			// extra/unknown data
			mprintf(("CSG => (0x%04x) %s\n", (uint32_t)section_id, msg.what()));
		} catch (const char *err) {
			mprintf(("CSG => ERROR: %s\n", err));
			csg_close();
			return false;
		}

		// reset safety catch
		cf_set_max_read_len(cfp, 0);

		// skip to next section (if not already there)
		offset_pos = (start_pos + section_size) - cftell(cfp);

		if (offset_pos) {
			mprintf(("CSG => Warning: (0x%04x) Short read, information may have been lost!\n", (uint32_t)section_id));
			cfseek(cfp, (int)offset_pos, CF_SEEK_CUR);
		}
	}

	// this is what we came for...
	*rank = p->stats.rank;

	mprintf(("CSG => Get Rank complete!\n"));

	// cleanup & return
	csg_close();

	return true;
}
