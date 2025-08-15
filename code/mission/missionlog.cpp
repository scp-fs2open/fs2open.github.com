/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "globalincs/alphacolors.h"
#include "graphics/font.h"
#include "iff_defs/iff_defs.h"
#include "localization/localize.h"
#include "mission/missiongoals.h"
#include "mission/missionlog.h"
#include "mission/missionparse.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "ship/ship.h"


#define EMPTY_LOG_NAME		""

// defines for X position offsets of different items for mission log
#define TIME_X			10
#define OBJECT_X		75
#define ACTION_X		250

#define NUM_LOG_COLORS		3

static int X, P_width;

SCP_vector<log_line_complete> Log_scrollback_vec;
SCP_vector<log_entry> Log_entries;

void mission_log_init()
{
	// zero out all the memory so we don't get bogus information when playing across missions!
	Log_entries.clear();
}

// following function adds an entry into the mission log.
// pass a type and a string which indicates the object
// that this event is for.  Don't add entries with this function for multiplayer
void mission_log_add_entry(LogType type, const char *pname, const char *sname, int info_index, int flags)
{
	// multiplayer clients don't use this function to add log entries -- they will get
	// all their info from the host
	if ( MULTIPLAYER_CLIENT ){
		return;
	}

	Log_entries.emplace_back();
	auto &entry = Log_entries.back();

	entry.type = type;
	if ( pname ) {
		Assert (strlen(pname) < NAME_LENGTH);
		strncpy_s(entry.pname, pname, NAME_LENGTH - 1);
	} else
		strcpy_s( entry.pname, EMPTY_LOG_NAME );

	if ( sname ) {
		Assert (strlen(sname) < NAME_LENGTH);
		strncpy_s(entry.sname, sname, NAME_LENGTH - 1);
	} else
		strcpy_s( entry.sname, EMPTY_LOG_NAME );

	entry.index = info_index;
	entry.flags = flags;
	entry.primary_team = -1;
	entry.secondary_team = -1;
	entry.pname_display = entry.pname;
	entry.sname_display = entry.sname;

	end_string_at_first_hash_symbol(entry.pname_display);
	end_string_at_first_hash_symbol(entry.sname_display);

	// determine the contents of the flags member based on the type of entry we added.  We need to store things
	// like team for the primary and (possibly) secondary object for this entry.
	switch ( type ) {
	int index;

	case LOG_SHIP_DESTROYED:
	case LOG_SHIP_ARRIVED:
	case LOG_SHIP_DEPARTED:
	case LOG_SHIP_DOCKED:
	case LOG_SHIP_SUBSYS_DESTROYED:
	case LOG_SHIP_UNDOCKED:
	case LOG_SHIP_DISABLED:
	case LOG_SHIP_DISARMED:
	case LOG_SELF_DESTRUCTED:
		// multiplayer. callsign is passed in for ship destroyed and self destruct
		if((Game_mode & GM_MULTIPLAYER) && (multi_find_player_by_callsign(pname) >= 0)){
			int np_index = multi_find_player_by_callsign(pname);
			index = multi_get_player_ship( np_index );
		} else {
			index = ship_name_lookup( pname );
		}

		Assert (index >= 0);
		entry.primary_team = Ships[index].team;
		entry.pname_display = Ships[index].get_display_name();

		if (Ships[index].flags[Ship::Ship_Flags::Hide_mission_log]) {
			entry.flags |= MLF_HIDDEN;
		}

		// some of the entries have a secondary component.  Figure out what is up with them.
		if ( (type == LOG_SHIP_DOCKED) || (type == LOG_SHIP_UNDOCKED)) {
			if ( sname ) {
				index = ship_name_lookup( sname );
				Assert (index >= 0);
				entry.secondary_team = Ships[index].team;
				entry.sname_display = Ships[index].get_display_name();
			}
		} else if ( type == LOG_SHIP_DESTROYED ) {
			if ( sname ) {
				// multiplayer, player name will possibly be sent in
				if((Game_mode & GM_MULTIPLAYER) && (multi_find_player_by_callsign(sname) >= 0)) {
					// get the player's ship
					int np_index = multi_find_player_by_callsign(sname);
					int np_ship = multi_get_player_ship(np_index);

					if(np_ship < 0)
					{
						// argh. badness
						Int3();
						entry.secondary_team = Player_ship->team;
					}
					else
					{
						entry.secondary_team = Ships[Objects[Net_players[np_index].m_player->objnum].instance].team;
						entry.sname_display = Ships[Objects[Net_players[np_index].m_player->objnum].instance].get_display_name();
					}
				}
				else 
				{
					index = ship_name_lookup( sname );
					// no ship, then it probably exited -- check the exited 
					if ( index == -1 ) {
						index = ship_find_exited_ship_by_name( sname );
						if ( index == -1 ) {
							break;
						}
						entry.secondary_team = Ships_exited[index].team;
						entry.sname_display = Ships_exited[index].display_name;
					} else {
						entry.secondary_team = Ships[index].team;
						entry.sname_display = Ships[index].get_display_name();
					}
				}
 			} else {
				nprintf(("missionlog", "No secondary name for ship destroyed log entry!\n"));
			}
		} else if ( (type == LOG_SHIP_SUBSYS_DESTROYED) && (Ship_info[Ships[index].ship_info_index].is_small_ship()) ) {
			// make subsystem destroyed entries for small ships hidden
			entry.flags |= MLF_HIDDEN;
		} else if ( (type == LOG_SHIP_ARRIVED) && (Ships[index].wingnum != -1 ) ) {
			// arrival of ships in wings don't display
			entry.flags |= MLF_HIDDEN;
		}
		break;

	case LOG_WING_DESTROYED:
	case LOG_WING_DEPARTED:
	case LOG_WING_ARRIVED:
		index = wing_name_lookup(pname, 1);
		Assert(index != -1);
		Assert(info_index != -1);			// this is the team value

		// get the team value for this wing.  Departed or destroyed wings will pass the team
		// value in info_index parameter.  For arriving wings, get the team value from the
		// first ship in the list because the info_index contains the wave count
		if ( type == LOG_WING_ARRIVED ) {
			int i, si = -1;

			// Goober5000 - get the team value from any ship in the list, because
			// ships that arrive initially docked could be created in random order
			for (i = 0; i < MAX_SHIPS_PER_WING; ++i) {
				// get first valid ship
				si = Wings[index].ship_index[i];
				if (si >= 0) {
					break;
				}
			}
			Assert( si != -1 );
			entry.primary_team = Ships[si].team;
		} else {
			entry.primary_team = info_index;
		}

#ifndef NDEBUG
		// MWA 2/25/98.  debug code to try to find any ships in this wing that have departed.
		// scan through all log entries and find at least one ship_depart entry for a ship
		// that was in this wing.
		if ( type == LOG_WING_DEPARTED ) {
			// if all were destroyed, then don't do this debug code.
			if ( (Wings[index].total_destroyed + Wings[index].total_vanished) == Wings[index].total_arrived_count ){
				break;
			}

			if (std::find_if(Log_entries.begin(), Log_entries.end(), [index](const log_entry& le) {
				return (le.type == LOG_SHIP_DEPARTED) && (le.index == index);
			}) == Log_entries.end()) {
				UNREACHABLE("cannot find any departed ships from wing %s that supposedly departed.", Wings[index].name);	// get Allender
			}
		}
#endif
		break;

		// don't display waypoint done entries
	case LOG_WAYPOINTS_DONE:
		entry.flags |= MLF_HIDDEN;
		break;

	default:
		break;
	}

	entry.timestamp = Missiontime;
	entry.timer_padding = The_mission.HUD_timer_padding;

	// if in multiplayer and I am the master, send this log entry to everyone
	if ( MULTIPLAYER_MASTER ){
		send_mission_log_packet( &entry );
	}

#ifndef NDEBUG
	float mission_time = f2fl(Missiontime);
	int minutes = (int)(mission_time / 60);
	int seconds = (int)mission_time % 60;

	// record the entry to the debug log too
	switch (entry.type) {
		case LOG_SHIP_DESTROYED:
		case LOG_WING_DESTROYED:
			nprintf(("missionlog", "MISSION LOG: %s destroyed at %02d:%02d\n", entry.pname, minutes, seconds));
			break;
		case LOG_SHIP_ARRIVED:
		case LOG_WING_ARRIVED:
			nprintf(("missionlog", "MISSION LOG: %s arrived at %02d:%02d\n", entry.pname, minutes, seconds));
			break;
		case LOG_SHIP_DEPARTED:
		case LOG_WING_DEPARTED:
			nprintf(("missionlog", "MISSION LOG: %s departed at %02d:%02d\n", entry.pname, minutes, seconds));
			break;
		case LOG_SHIP_DOCKED:
		case LOG_SHIP_UNDOCKED:
			nprintf(("missionlog", "MISSION LOG: %s %sdocked with %s at %02d:%02d\n", entry.pname, entry.type == LOG_SHIP_UNDOCKED ? "un" : "", entry.sname, minutes, seconds));
			break;
		case LOG_SHIP_SUBSYS_DESTROYED:
			nprintf(("missionlog", "MISSION LOG: %s subsystem %s destroyed at %02d:%02d\n", entry.pname, entry.sname, minutes, seconds));
			break;
		case LOG_SHIP_DISABLED:
			nprintf(("missionlog", "MISSION LOG: %s disabled at %02d:%02d\n", entry.pname, minutes, seconds));
			break;
		case LOG_SHIP_DISARMED:
			nprintf(("missionlog", "MISSION LOG: %s disarmed at %02d:%02d\n", entry.pname, minutes, seconds));
			break;
		case LOG_PLAYER_CALLED_FOR_REARM:
			nprintf(("missionlog", "MISSION LOG: Player %s called for rearm at %02d:%02d\n", entry.pname, minutes, seconds));
			break;
		case LOG_PLAYER_ABORTED_REARM:
			nprintf(("missionlog", "MISSION LOG: Player %s aborted rearm at %02d:%02d\n", entry.pname, minutes, seconds));
			break;
		case LOG_PLAYER_CALLED_FOR_REINFORCEMENT:
			nprintf(("missionlog", "MISSION LOG: A player called for reinforcement %s at %02d:%02d\n", entry.pname, minutes, seconds));
			break;
		case LOG_GOAL_SATISFIED:
			nprintf(("missionlog", "MISSION LOG: Goal %s satisfied at %02d:%02d\n", entry.pname, minutes, seconds));
			break;
		case LOG_GOAL_FAILED:
			nprintf(("missionlog", "MISSION LOG: Goal %s failed at %02d:%02d\n", entry.pname, minutes, seconds));
			break;
		case LOG_WAYPOINTS_DONE:
			nprintf(("missionlog", "MISSION LOG: %s completed waypoint path %s at %02d:%02d\n", entry.pname, entry.sname, minutes, seconds));
			break;
		case LOG_CARGO_REVEALED:
			nprintf(("missionlog", "MISSION LOG: %s cargo %s revealed at %02d:%02d\n", entry.pname, Cargo_names[entry.index], minutes, seconds));
			break;
		case LOG_CAP_SUBSYS_CARGO_REVEALED:
			nprintf(("missionlog", "MISSION LOG: %s subsystem %s cargo %s revealed at %02d:%02d\n", entry.pname, entry.sname, Cargo_names[entry.index], minutes, seconds));
			break;
		case LOG_SELF_DESTRUCTED:
			nprintf(("missionlog", "MISSION LOG: %s self-destructed at %02d:%02d\n", entry.pname, minutes, seconds));
			break;
	}
#endif
}

// function, used in multiplayer only, which adds an entry sent by the host of the game, into
// the mission log.  The index of the log entry is passed as one of the parameters in addition to
// the normal parameters used for adding an entry to the log
void mission_log_add_entry_multi( LogType type, const char *pname, const char *sname, int index, fix timestamp, int flags )
{
	// we'd better be in multiplayer and not the master of the game
	Assert ( Game_mode & GM_MULTIPLAYER );
	Assert ( !(Net_player->flags & NETINFO_FLAG_AM_MASTER) );

	Log_entries.emplace_back();
	auto &entry = Log_entries.back();

	entry.type = type;
	if ( pname ) {
		Assert (strlen(pname) < NAME_LENGTH);
		strcpy_s(entry.pname, pname);
	}
	if ( sname ) {
		Assert (strlen(sname) < NAME_LENGTH);
		strcpy_s(entry.sname, sname);
	}
	entry.index = index;

	entry.flags = flags;
	entry.timestamp = timestamp;
	entry.timer_padding = The_mission.HUD_timer_padding;

	entry.pname_display = entry.pname;
	entry.sname_display = entry.sname;
}

// function to determine is the given event has taken place count number of times.

int mission_log_get_time_indexed( LogType type, const char *pname, const char *sname, int count, fix *time)
{
	Assertion(count > 0, "The count parameter is %d; it should be greater than 0!", count);

	for (const auto& entry: Log_entries) {
		bool found = false;

		if ( entry.type == type ) {
			// if we are looking for a dock/undock entry, then we don't care about the order in which the names
			// were passed into this function.  Count the entry as found if either name matches both in the other
			// set.
			if ( (type == LOG_SHIP_DOCKED) || (type == LOG_SHIP_UNDOCKED) ) {
				if (sname == NULL) {
					Int3();
					return 0;
				}

				if ( (!stricmp(entry.pname, pname) && !stricmp(entry.sname, sname)) || (!stricmp(entry.pname, sname) && !stricmp(entry.sname, pname)) ) {
					found = true;
				}
			} else {
				// for non dock/undock goals, then the names are important!
				if (pname == NULL) {
					Int3();
					return 0;
				}

				if ( stricmp(entry.pname, pname) != 0 ) {
					continue;
				}

				// if we are looking for a subsystem entry, the subsystem names must be compared
				if ((type == LOG_SHIP_SUBSYS_DESTROYED || type == LOG_CAP_SUBSYS_CARGO_REVEALED)) {
					if ( (sname == NULL) || !subsystem_stricmp(sname, entry.sname) ) {
						found = true;
					}
				} else {
					if ( (sname == NULL) || !stricmp(sname, entry.sname) ) {
						found = true;
					}
				}
			}

			if ( found ) {
				count--;

				if ( !count ) {
					if (time) {
						*time = entry.timestamp;
					}

					return 1;
				}
			}
		}
	}

	return 0;
}

// this function determines if the given type of event on the specified
// object has taken place yet.  If not, it returns 0.  If it has, the
// timestamp that the event happened is returned in the time parameter
int mission_log_get_time( LogType type, const char *pname, const char *sname, fix *time )
{
	return mission_log_get_time_indexed( type, pname, sname, 1, time );
}

// determines the number of times the given type of event takes place

int mission_log_get_count( LogType type, const char *pname, const char *sname )
{
	int count = 0;

	for (const auto& entry : Log_entries) {
		if ( entry.type == type ) {
			// if we are looking for a dock/undock entry, then we don't care about the order in which the names
			// were passed into this function.  Count the entry as found if either name matches both in the other
			// set.
			if ( (type == LOG_SHIP_DOCKED) || (type == LOG_SHIP_UNDOCKED) ) {
				if (sname == NULL) {
					Int3();
					return 0;
				}

				if ( (!stricmp(entry.pname, pname) && !stricmp(entry.sname, sname)) || (!stricmp(entry.pname, sname) && !stricmp(entry.sname, pname)) ) {
					count++;
				}
			} else {
				// for non dock/undock goals, then the names are important!
				if (pname == NULL) {
					Int3();
					return 0;
				}

				if ( stricmp(entry.pname, pname) != 0 ) {
					continue;
				}

				if ( (sname == NULL) || !stricmp(sname, entry.sname) ) {
					count++;
				}
			}
		}
	}

	return count;
}


void message_log_add_seg(log_text_seg* entry, int x, int msg_color, const char* text, int flags = 0)
{
	// set the vector
	entry->text.reset(vm_strdup(text));
	entry->color = msg_color;
	entry->x = x;
	entry->flags = flags;
}

void message_log_add_segs(const char* source_string, int msg_color, int flags = 0, SCP_vector<log_text_seg> *entry = nullptr, bool split_string = false)
{
	if (!source_string) {
		mprintf(("Why are you passing a NULL pointer to message_log_add_segs?\n"));
		return;
	}
	if (!*source_string) {
		return;
	}
        
	int w;

	// duplicate the string so that we can split it without modifying the source
	char *dup_string = vm_strdup(source_string);
	char *str = dup_string;
	char *split = NULL;

	if (split_string) {
		while (true) {
			if (X == ACTION_X) {
				while (is_white_space(*str))
					str++;
			}

			if (P_width - X < 1)
				split = str;
			else
				split = split_str_once(str, P_width - X);

			if (split != str) {
				log_text_seg new_seg;
				message_log_add_seg(&new_seg, X, msg_color, str, flags);
				entry->push_back(std::move(new_seg));
			}

			if (!split) {
				gr_get_string_size(&w, NULL, str);
				X += w;
				break;
			}

			X = ACTION_X;
			str = split;
		}
	} else {
		log_text_seg new_seg;
		message_log_add_seg(&new_seg, X, msg_color, str, flags);
		entry->push_back(std::move(new_seg));
	}

	// free the buffer
	vm_free(dup_string);
}

int mission_log_color_get_team(int msg_color)
{
	return msg_color - NUM_LOG_COLORS;
}

int mission_log_team_get_color(int team)
{
	return NUM_LOG_COLORS + team;
}

// pw = total pixel width
void mission_log_init_scrollback(int pw, bool split_string)
{
	P_width = pw;

	Log_scrollback_vec.clear();

	for (const auto& entry : Log_entries) {
		if (entry.flags & MLF_HIDDEN)
			continue;

		// don't display failed bonus goals
		if (entry.type == LOG_GOAL_FAILED) {
			int type = Mission_goals[entry.index].type & GOAL_TYPE_MASK;

			if (type == BONUS_GOAL) {
				continue;
			}
		}

		log_line_complete thisEntry;

		// track time of event (normal Missiontime format)
		thisEntry.timestamp = entry.timestamp;
		thisEntry.timer_padding = entry.timer_padding;

		// keep track of base color for the entry
		int thisColor;

		// Goober5000
		if ((entry.type == LOG_GOAL_SATISFIED) || (entry.type == LOG_GOAL_FAILED))
			thisColor = LOG_COLOR_BRIGHT;
		else if (entry.primary_team >= 0)
			thisColor = mission_log_team_get_color(entry.primary_team);
		else
			thisColor = LOG_COLOR_OTHER;

		if ( (Lcl_gr) && ((entry.type == LOG_GOAL_FAILED) || (entry.type == LOG_GOAL_SATISFIED)) ) {
			// in german goal events, just say "objective" instead of objective name
			// this is cuz we can't translate objective names
			message_log_add_seg(&thisEntry.objective, OBJECT_X, thisColor, "Einsatzziel");
		} else if ( (Lcl_pl) && ((entry.type == LOG_GOAL_FAILED) || (entry.type == LOG_GOAL_SATISFIED)) ) {
			// same thing for polish
			message_log_add_seg(&thisEntry.objective, OBJECT_X, thisColor, "Cel misji");
		} else {
			message_log_add_seg(&thisEntry.objective, OBJECT_X, thisColor, entry.pname_display.c_str());
		}

		//Set the flags for objectives
		if (entry.type == LOG_GOAL_SATISFIED) {
			thisEntry.objective.flags = LOG_FLAG_GOAL_TRUE;
		} else if (entry.type == LOG_GOAL_FAILED) {
			thisEntry.objective.flags = LOG_FLAG_GOAL_FAILED;
		} else {
			thisEntry.objective.flags = 0;
		}

		// now on to the actual message itself
		X = ACTION_X;

		// Goober5000
		if (entry.secondary_team >= 0)
			thisColor = mission_log_team_get_color(entry.secondary_team);
		else
			thisColor = LOG_COLOR_NORMAL;

		char text[256];

		switch (entry.type) {
			case LOG_SHIP_DESTROYED:
				message_log_add_segs(XSTR( "Destroyed", 404), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				if (!entry.sname_display.empty()) {
					message_log_add_segs(XSTR("  Kill: ", 405), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
					message_log_add_segs(entry.sname_display.c_str(), thisColor, 0, &thisEntry.segments, split_string);
					if (entry.index >= 0) {
						sprintf(text, NOX(" (%d%%)"), entry.index);
						message_log_add_segs(text, LOG_COLOR_BRIGHT, 0, &thisEntry.segments, split_string);
					}
				}
				break;

			case LOG_SELF_DESTRUCTED:
				message_log_add_segs(XSTR("Self destructed", 1476), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;

			case LOG_WING_DESTROYED:
				message_log_add_segs(XSTR("Destroyed", 404), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;

			case LOG_SHIP_ARRIVED:
				message_log_add_segs(XSTR("Arrived", 406), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;

			case LOG_WING_ARRIVED:
				if (entry.index > 1){
					sprintf(text, XSTR( "Arrived (wave %d)", 407), entry.index);
				} else {
					strcpy_s(text, XSTR( "Arrived", 406));
				}
				message_log_add_segs(text, LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;

			case LOG_SHIP_DEPARTED:
				message_log_add_segs(XSTR("Departed", 408), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;

			case LOG_WING_DEPARTED:
				message_log_add_segs(XSTR("Departed", 408), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;

			case LOG_SHIP_DOCKED:
				message_log_add_segs(XSTR("Docked with ", 409), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				message_log_add_segs(entry.sname_display.c_str(), thisColor, 0, &thisEntry.segments, split_string);
				break;

			case LOG_SHIP_SUBSYS_DESTROYED: {
				ship_info* sip = &Ship_info[(int)((entry.index >> 16) & 0xffff)];
				int model_index = (int)(entry.index & 0xffff);
				const char* subsys_name;
				if (strlen(sip->subsystems[model_index].alt_sub_name)) {
					subsys_name = sip->subsystems[model_index].alt_sub_name;
				} else {
					subsys_name = sip->subsystems[model_index].name;
				}

				message_log_add_segs(XSTR("Subsystem ", 410), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				message_log_add_segs(subsys_name, LOG_COLOR_BRIGHT, 0, &thisEntry.segments, split_string);
				message_log_add_segs(XSTR(" destroyed", 411), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;
			}

			case LOG_SHIP_UNDOCKED:
				message_log_add_segs(XSTR("Undocked with ", 412), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				message_log_add_segs(entry.sname_display.c_str(), thisColor, 0, &thisEntry.segments, split_string);
				break;

			case LOG_SHIP_DISABLED:
				message_log_add_segs(XSTR("Disabled", 413), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;

			case LOG_SHIP_DISARMED:
				message_log_add_segs(XSTR("Disarmed", 414), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;

			case LOG_PLAYER_CALLED_FOR_REARM:
				message_log_add_segs(XSTR(" called for rearm", 415), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;

			case LOG_PLAYER_ABORTED_REARM:
				message_log_add_segs(XSTR(" aborted rearm", 416), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;

			case LOG_PLAYER_CALLED_FOR_REINFORCEMENT:
				message_log_add_segs(XSTR("Called in as reinforcement", 417), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				break;

			case LOG_CARGO_REVEALED:
				Assert( entry.index >= 0 );
				Assert(!(entry.index & CARGO_NO_DEPLETE));

				message_log_add_segs(XSTR("Cargo revealed: ", 418), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				strncpy(text, Cargo_names[entry.index], sizeof(text) - 1);
				message_log_add_segs(text, LOG_COLOR_BRIGHT, 0, &thisEntry.segments, split_string);
				break;

			case LOG_CAP_SUBSYS_CARGO_REVEALED:
				Assert( entry.index >= 0 );
				Assert(!(entry.index & CARGO_NO_DEPLETE));

				message_log_add_segs(entry.sname_display.c_str(), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				message_log_add_segs(XSTR( " subsystem cargo revealed: ", 1488), LOG_COLOR_NORMAL, 0, &thisEntry.segments, split_string);
				strncpy(text, Cargo_names[entry.index], sizeof(text) - 1);
				message_log_add_segs(text, LOG_COLOR_BRIGHT, 0, &thisEntry.segments, split_string);
				break;


			case LOG_GOAL_SATISFIED:
			case LOG_GOAL_FAILED: {
				int type = Mission_goals[entry.index].type & GOAL_TYPE_MASK;
				sprintf( text, XSTR( "%s objective ", 419), Goal_type_text(type) );
				if ( entry.type == LOG_GOAL_SATISFIED )
					strcat_s(text, XSTR( "satisfied.", 420));
				else
					strcat_s(text, XSTR( "failed.", 421));

				message_log_add_segs(text, LOG_COLOR_BRIGHT, 0, &thisEntry.segments, split_string);
				break;
			}	// matches case statement!
			default:
				UNREACHABLE("Unhandled enum value!");
				break;
		}

		Log_scrollback_vec.push_back(std::move(thisEntry));
	}
}

void mission_log_shutdown_scrollback()
{
	Log_scrollback_vec.clear();
}

int mission_log_scrollback_num_lines()
{
	return static_cast<int>(Log_scrollback_vec.size());
}

const log_line_complete* mission_log_scrollback_get_line(int index)
{
	if (!SCP_vector_inbounds(Log_scrollback_vec, index))
		return nullptr;
	return &Log_scrollback_vec[index];
}

const color *log_line_get_color(int tag)
{
	switch (tag) {
		case LOG_COLOR_BRIGHT:
			return &Color_bright;

		case LOG_COLOR_OTHER:
			return &Color_normal;

		default: {
			int team = mission_log_color_get_team(tag);
			if (team < 0)
				return &Color_text_normal;
			else
				return iff_get_color_by_team(team, -1, 1);
		}
	}
}

// message_log_scrollback displays the contents of the mesasge log currently much like the HUD
// message scrollback system.  I'm sure this system will be overhauled.		
void mission_log_scrollback(int line_offset, int list_x, int list_y, int list_w, int list_h)
{
	int font_h = gr_get_font_height();

	int y = 0;

	for (int i = line_offset; i < mission_log_scrollback_num_lines(); i++) {

		// if we're beyond the printable area stop printing!
		if (y + font_h > list_h)
			break;

		bool printSymbols = false;
		int symbolFlag = 0;

		// print the timestamp
		gr_set_color_fast(&Color_text_normal);
		gr_print_timestamp(list_x + TIME_X, list_y + y, Log_scrollback_vec[i].timestamp, GR_RESIZE_MENU);

		// print the objective
		auto obj_color = log_line_get_color(Log_scrollback_vec[i].objective.color);
		gr_set_color_fast(obj_color);

		// check the flags
		if ((Log_scrollback_vec[i].objective.flags & LOG_FLAG_GOAL_TRUE) || (Log_scrollback_vec[i].objective.flags & LOG_FLAG_GOAL_FAILED)) {
			printSymbols = true;
			symbolFlag = Log_scrollback_vec[i].objective.flags;
		}
			
		char buf[256];
		strcpy_s(buf, Log_scrollback_vec[i].objective.text.get());
		font::force_fit_string(buf, 256, ACTION_X - OBJECT_X - 8);
		gr_string(list_x + Log_scrollback_vec[i].objective.x, list_y + y, buf, GR_RESIZE_MENU);

		// print the segments
		for (int j = 0; j < (int)Log_scrollback_vec[i].segments.size(); j++) {

			const auto& thisSeg = Log_scrollback_vec[i].segments[j];

			auto this_color = log_line_get_color(Log_scrollback_vec[i].segments[j].color);
			gr_set_color_fast(this_color);

			strcpy_s(buf, thisSeg.text.get());
			font::force_fit_string(buf, 256, list_w - thisSeg.x);
			gr_string(list_x + thisSeg.x, list_y + y, buf, GR_RESIZE_MENU);

		}

		if (printSymbols) {
			if (symbolFlag & LOG_FLAG_GOAL_FAILED)
				gr_set_color_fast(&Color_bright_red);
			else
				gr_set_color_fast(&Color_bright_green);

			int loc_y = list_y + y + font_h / 2 - 1;
			gr_circle(list_x + TIME_X - 6, loc_y, 5, GR_RESIZE_MENU);

			gr_set_color_fast(&Color_bright);
			gr_line(list_x + TIME_X - 10, loc_y, list_x + TIME_X - 8, loc_y, GR_RESIZE_MENU);
			gr_line(list_x + TIME_X - 6, loc_y - 4, list_x + TIME_X - 6, loc_y - 2, GR_RESIZE_MENU);
			gr_line(list_x + TIME_X - 4, loc_y, list_x + TIME_X - 2, loc_y, GR_RESIZE_MENU);
			gr_line(list_x + TIME_X - 6, loc_y + 2, list_x + TIME_X - 6, loc_y + 4, GR_RESIZE_MENU);
		}

		y += font_h;

	}
}
