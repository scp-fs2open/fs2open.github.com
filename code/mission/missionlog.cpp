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



#define MAX_LOG_ENTRIES		700

// used for high water mark for culling out log entries
#define LOG_CULL_MARK				((int)(MAX_LOG_ENTRIES * 0.95f))
#define LOG_CULL_DOORDIE_MARK		((int)(MAX_LOG_ENTRIES * 0.99f))
#define LOG_LAST_DITCH_CULL_NUM	((int)(MAX_LOG_ENTRIES * 0.20f))
#define LOG_HALFWAY_REPORT_NUM	((int)(MAX_LOG_ENTRIES * 0.50f))

#define EMPTY_LOG_NAME		""

// defines for X position offsets of different items for mission log
#define TIME_X			10
#define OBJECT_X		75
#define ACTION_X		250

#define LOG_COLOR_NORMAL	0
#define LOG_COLOR_BRIGHT	1
#define LOG_COLOR_OTHER		2
#define NUM_LOG_COLORS		3

// defines for log flags
#define LOG_FLAG_GOAL_FAILED	(1<<0)
#define LOG_FLAG_GOAL_TRUE		(1<<1)

typedef struct log_text_seg {
	char	*text;				// the text
	int	color;				// color text should be displayed in
	int	x;						// x offset to display text at
	int	flags;				// used to possibly print special characters when displaying the log
} log_text_seg;

int Num_log_lines;
static int X, P_width;

struct log_line_complete {
	fix timestamp;
	log_text_seg objective;
	SCP_vector<log_text_seg> actions;
};

// used for displaying the mission log scrollback in a human readable format
SCP_vector<log_line_complete> Log_scrollback_vec;

std::array<log_entry, MAX_LOG_ENTRIES> log_entries;	// static array because John says....
int last_entry;

void mission_log_init()
{
	last_entry = 0;

	// zero out all the memory so we don't get bogus information when playing across missions!
	log_entries.fill({});
}

// function to clean up the mission log removing obsolete entries.  Entries might get marked obsolete
// in several ways -- having to recycle entries, a ship's subsystem destroyed entries when a ship is
// fully destroyed, etc.
void mission_log_cull_obsolete_entries()
{
	int i, index;

	nprintf(("missionlog", "culling obsolete entries.  starting last entry %d.\n", last_entry));
	// find the first obsolete entry
	for (i = 0; i < last_entry; i++ ) 
		if ( log_entries[i].flags & MLF_OBSOLETE )
			break;

	// nothing to do if next if statement is true
	if ( i == last_entry )
		return;

	// compact the log array, removing the obsolete entries.
	index = i;						// index is the first obsolete entry

	// 'index' should always point to the next element in the list
	// which is getting compacted.  'i' points to the next array
	// element to be replaced.
	do {
		// get to the next non-obsolete entry.  The obsolete entry must not be essential either!
		while ( (log_entries[index].flags & MLF_OBSOLETE) && !(log_entries[index].flags & MLF_ESSENTIAL) ) {
			index++;
			last_entry--;
		}

		log_entries[i++] = log_entries[index++];
	} while ( i < last_entry );

#ifndef NDEBUG
	nprintf(("missionlog", "Ending entry: %d.\n", last_entry));
#endif
}

// function to mark entries as obsolete.  Passed is the type of entry that is getting added
// to the log.  Some entries might get marked obsolete as a result of this type
void mission_log_obsolete_entries(LogType type, const char *pname)
{
	int i;
	log_entry *entry = NULL;

	// before adding this entry, check to see if the entry type is a ship destroyed or destructed entry.
	// If so, we can remove any subsystem destroyed entries from the log for this ship.  
	if ( type == LOG_SHIP_DESTROYED || type == LOG_SELF_DESTRUCTED ) {
		for (i = 0; i < last_entry; i++) {
			entry = &log_entries[i];

			// check to see if the type is a subsystem destroyed entry, and that it belongs to the
			// ship passed into this routine.  If it matches, mark as obsolete.  We'll clean up
			// the log when it starts to get full
			if ( !stricmp( pname, entry->pname ) ) {
				if ( (entry->type == LOG_SHIP_SUBSYS_DESTROYED) || (entry->type == LOG_SHIP_DISARMED) || (entry->type == LOG_SHIP_DISABLED) )
					entry->flags |= MLF_OBSOLETE;
			}
		}
	}

	// check to see if we are getting to about 80% of our log capacity.  If so, cull the log.
	if ( last_entry > LOG_CULL_MARK ) {
		mission_log_cull_obsolete_entries();

		// if we culled the entries, and we are still low on space, we need to take more drastic measures.
		// these include removing all non-essential entries from the log.  These entries are entries 
		// which has not been asked for by mission_log_get_time
		if ( last_entry > LOG_CULL_MARK ) {
			nprintf(("missionlog", "marking the first %d non-essential log entries as obsolete\n", LOG_LAST_DITCH_CULL_NUM));
			for (i = 0; i < LOG_LAST_DITCH_CULL_NUM; i++ ) {
				entry = &log_entries[i];
				if ( !(entry->flags & MLF_ESSENTIAL) ){
					entry->flags |= MLF_OBSOLETE;
				}
			}

			// cull the obsolete entries again
			mission_log_cull_obsolete_entries();

			// if we get to this point, and there are no entries left -- we are in big trouble.  We will simply
			// mark the first 20% of the log as obsolete and compress.  Don't do this unless we are *really*
			// in trouble
			if ( last_entry > LOG_CULL_DOORDIE_MARK ) {
				nprintf(("missionlog", "removing the first %d entries in the mission log!!!!\n", LOG_LAST_DITCH_CULL_NUM));
				for (i = 0; i < LOG_LAST_DITCH_CULL_NUM; i++ ){
					entry->flags |= MLF_OBSOLETE;
				}

				mission_log_cull_obsolete_entries();
			}
		}
	}
}

// following function adds an entry into the mission log.
// pass a type and a string which indicates the object
// that this event is for.  Don't add entries with this function for multiplayer
void mission_log_add_entry(LogType type, const char *pname, const char *sname, int info_index, int flags)
{
	int last_entry_save;
	log_entry *entry;	

	// multiplayer clients don't use this function to add log entries -- they will get
	// all their info from the host
	if ( MULTIPLAYER_CLIENT ){
		return;
	}

	last_entry_save = last_entry;

	// mark any entries as obsolete.  Part of the pruning is done based on the type (and name) passed
	// for a new entry
	mission_log_obsolete_entries(type, pname);

	entry = &log_entries[last_entry];

	if ( last_entry == MAX_LOG_ENTRIES ){
		return;
	}

	entry->type = type;
	if ( pname ) {
		Assert (strlen(pname) < NAME_LENGTH);
		strncpy_s(entry->pname, pname, NAME_LENGTH - 1);
	} else
		strcpy_s( entry->pname, EMPTY_LOG_NAME );

	if ( sname ) {
		Assert (strlen(sname) < NAME_LENGTH);
		strncpy_s(entry->sname, sname, NAME_LENGTH - 1);
	} else
		strcpy_s( entry->sname, EMPTY_LOG_NAME );

	entry->index = info_index;
	entry->flags = flags;
	entry->primary_team = -1;
	entry->secondary_team = -1;
	entry->pname_display = entry->pname;
	entry->sname_display = entry->sname;

	end_string_at_first_hash_symbol(entry->pname_display);
	end_string_at_first_hash_symbol(entry->sname_display);

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
		entry->primary_team = Ships[index].team;
		entry->pname_display = Ships[index].get_display_name();

		if (Ships[index].flags[Ship::Ship_Flags::Hide_mission_log]) {
			entry->flags |= MLF_HIDDEN;
		}

		// some of the entries have a secondary component.  Figure out what is up with them.
		if ( (type == LOG_SHIP_DOCKED) || (type == LOG_SHIP_UNDOCKED)) {
			if ( sname ) {
				index = ship_name_lookup( sname );
				Assert (index >= 0);
				entry->secondary_team = Ships[index].team;
				entry->sname_display = Ships[index].get_display_name();
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
						entry->secondary_team = Player_ship->team;
					}
					else
					{
						entry->secondary_team = Ships[Objects[Net_players[np_index].m_player->objnum].instance].team;
						entry->sname_display = Ships[Objects[Net_players[np_index].m_player->objnum].instance].get_display_name();
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
						entry->secondary_team = Ships_exited[index].team;
						entry->sname_display = Ships_exited[index].display_name;
					} else {
						entry->secondary_team = Ships[index].team;
						entry->sname_display = Ships[index].get_display_name();
					}
				}
 			} else {
				nprintf(("missionlog", "No secondary name for ship destroyed log entry!\n"));
			}
		} else if ( (type == LOG_SHIP_SUBSYS_DESTROYED) && (Ship_info[Ships[index].ship_info_index].is_small_ship()) ) {
			// make subsystem destroyed entries for small ships hidden
			entry->flags |= MLF_HIDDEN;
		} else if ( (type == LOG_SHIP_ARRIVED) && (Ships[index].wingnum != -1 ) ) {
			// arrival of ships in wings don't display
			entry->flags |= MLF_HIDDEN;
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
			entry->primary_team = Ships[si].team;
		} else {
			entry->primary_team = info_index;
		}

#ifndef NDEBUG
		// MWA 2/25/98.  debug code to try to find any ships in this wing that have departed.
		// scan through all log entries and find at least one ship_depart entry for a ship
		// that was in this wing.
		if ( type == LOG_WING_DEPARTED ) {
			int i;

			// if all were destroyed, then don't do this debug code.
			if ( (Wings[index].total_destroyed + Wings[index].total_vanished) == Wings[index].total_arrived_count ){
				break;
			}

			for ( i = 0; i < last_entry; i++ ) {
				if ( log_entries[i].type != LOG_SHIP_DEPARTED ){
					continue;
				}
				if( log_entries[i].index == index ){
					break;
				}
			}
			if ( i == last_entry ){
				Int3();						// get Allender -- cannot find any departed ships from wing that supposedly departed.
			}
		}
#endif

		break;

		// don't display waypoint done entries
	case LOG_WAYPOINTS_DONE:
		entry->flags |= MLF_HIDDEN;
		break;

	default:
		break;
	}

	entry->timestamp = Missiontime;

	// if in multiplayer and I am the master, send this log entry to everyone
	if ( MULTIPLAYER_MASTER ){
		send_mission_log_packet( &log_entries[last_entry] );
	}

	last_entry++;

#ifndef NDEBUG
	if ( !(last_entry % 10) ) {
		if ( (last_entry > LOG_HALFWAY_REPORT_NUM) && (last_entry > last_entry_save) ){
			nprintf(("missionlog", "new highwater point reached for mission log (%d entries).\n", last_entry));
		}
	}

	float mission_time = f2fl(Missiontime);
	int minutes = (int)(mission_time / 60);
	int seconds = (int)mission_time % 60;

	// record the entry to the debug log too
	switch (entry->type) {
		case LOG_SHIP_DESTROYED:
		case LOG_WING_DESTROYED:
			nprintf(("missionlog", "MISSION LOG: %s destroyed at %02d:%02d\n", entry->pname, minutes, seconds));
			break;
		case LOG_SHIP_ARRIVED:
		case LOG_WING_ARRIVED:
			nprintf(("missionlog", "MISSION LOG: %s arrived at %02d:%02d\n", entry->pname, minutes, seconds));
			break;
		case LOG_SHIP_DEPARTED:
		case LOG_WING_DEPARTED:
			nprintf(("missionlog", "MISSION LOG: %s departed at %02d:%02d\n", entry->pname, minutes, seconds));
			break;
		case LOG_SHIP_DOCKED:
		case LOG_SHIP_UNDOCKED:
			nprintf(("missionlog", "MISSION LOG: %s %sdocked with %s at %02d:%02d\n", entry->pname, entry->type == LOG_SHIP_UNDOCKED ? "un" : "", entry->sname, minutes, seconds));
			break;
		case LOG_SHIP_SUBSYS_DESTROYED:
			nprintf(("missionlog", "MISSION LOG: %s subsystem %s destroyed at %02d:%02d\n", entry->pname, entry->sname, minutes, seconds));
			break;
		case LOG_SHIP_DISABLED:
			nprintf(("missionlog", "MISSION LOG: %s disabled at %02d:%02d\n", entry->pname, minutes, seconds));
			break;
		case LOG_SHIP_DISARMED:
			nprintf(("missionlog", "MISSION LOG: %s disarmed at %02d:%02d\n", entry->pname, minutes, seconds));
			break;
		case LOG_PLAYER_CALLED_FOR_REARM:
			nprintf(("missionlog", "MISSION LOG: Player %s called for rearm at %02d:%02d\n", entry->pname, minutes, seconds));
			break;
		case LOG_PLAYER_ABORTED_REARM:
			nprintf(("missionlog", "MISSION LOG: Player %s aborted rearm at %02d:%02d\n", entry->pname, minutes, seconds));
			break;
		case LOG_PLAYER_CALLED_FOR_REINFORCEMENT:
			nprintf(("missionlog", "MISSION LOG: A player called for reinforcement %s at %02d:%02d\n", entry->pname, minutes, seconds));
			break;
		case LOG_GOAL_SATISFIED:
			nprintf(("missionlog", "MISSION LOG: Goal %s satisfied at %02d:%02d\n", entry->pname, minutes, seconds));
			break;
		case LOG_GOAL_FAILED:
			nprintf(("missionlog", "MISSION LOG: Goal %s failed at %02d:%02d\n", entry->pname, minutes, seconds));
			break;
		case LOG_WAYPOINTS_DONE:
			nprintf(("missionlog", "MISSION LOG: %s completed waypoint path %s at %02d:%02d\n", entry->pname, entry->sname, minutes, seconds));
			break;
		case LOG_CARGO_REVEALED:
			nprintf(("missionlog", "MISSION LOG: %s cargo %s revealed at %02d:%02d\n", entry->pname, Cargo_names[entry->index], minutes, seconds));
			break;
		case LOG_CAP_SUBSYS_CARGO_REVEALED:
			nprintf(("missionlog", "MISSION LOG: %s subsystem %s cargo %s revealed at %02d:%02d\n", entry->pname, entry->sname, Cargo_names[entry->index], minutes, seconds));
			break;
		case LOG_SELF_DESTRUCTED:
			nprintf(("missionlog", "MISSION LOG: %s self-destructed at %02d:%02d\n", entry->pname, minutes, seconds));
			break;
	}
#endif
}

// function, used in multiplayer only, which adds an entry sent by the host of the game, into
// the mission log.  The index of the log entry is passed as one of the parameters in addition to
// the normal parameters used for adding an entry to the log
void mission_log_add_entry_multi( LogType type, const char *pname, const char *sname, int index, fix timestamp, int flags )
{
	log_entry *entry;

	// we'd better be in multiplayer and not the master of the game
	Assert ( Game_mode & GM_MULTIPLAYER );
	Assert ( !(Net_player->flags & NETINFO_FLAG_AM_MASTER) );

	// mark any entries as obsolete.  Part of the pruning is done based on the type (and name) passed
	// for a new entry
	mission_log_obsolete_entries(type, pname);

	entry = &log_entries[last_entry];

	if ( last_entry == MAX_LOG_ENTRIES ){
		return;
	}

	last_entry++;

	entry->type = type;
	if ( pname ) {
		Assert (strlen(pname) < NAME_LENGTH);
		strcpy_s(entry->pname, pname);
	}
	if ( sname ) {
		Assert (strlen(sname) < NAME_LENGTH);
		strcpy_s(entry->sname, sname);
	}
	entry->index = index;

	entry->flags = flags;
	entry->timestamp = timestamp;

	entry->pname_display = entry->pname;
	entry->sname_display = entry->sname;
}

// function to determine is the given event has taken place count number of times.

int mission_log_get_time_indexed( LogType type, const char *pname, const char *sname, int count, fix *time)
{
	int i, found;
	log_entry *entry;
	Assertion(count > 0, "The count parameter is %d; it should be greater than 0!", count);

	entry = &log_entries[0];

	for (i = 0; i < last_entry; i++, entry++) {
		found = 0;

		if ( entry->type == type ) {
			// if we are looking for a dock/undock entry, then we don't care about the order in which the names
			// were passed into this function.  Count the entry as found if either name matches both in the other
			// set.
			if ( (type == LOG_SHIP_DOCKED) || (type == LOG_SHIP_UNDOCKED) ) {
				if (sname == NULL) {
					Int3();
					return 0;
				}

				if ( (!stricmp(entry->pname, pname) && !stricmp(entry->sname, sname)) || (!stricmp(entry->pname, sname) && !stricmp(entry->sname, pname)) ) {
					found = 1;
				}
			} else {
				// for non dock/undock goals, then the names are important!
				if (pname == NULL) {
					Int3();
					return 0;
				}

				if ( stricmp(entry->pname, pname) != 0 ) {
					continue;
				}

				// if we are looking for a subsystem entry, the subsystem names must be compared
				if ((type == LOG_SHIP_SUBSYS_DESTROYED || type == LOG_CAP_SUBSYS_CARGO_REVEALED)) {
					if ( (sname == NULL) || !subsystem_stricmp(sname, entry->sname) ) {
						found = 1;
					}
				} else {
					if ( (sname == NULL) || !stricmp(sname, entry->sname) ) {
						found = 1;
					}
				}
			}

			if ( found ) {
				count--;

				if ( !count ) {
					entry->flags |= MLF_ESSENTIAL;				// since the goal code asked for this entry, mark it as essential

					if (time) {
						*time = entry->timestamp;
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
	int i;
	log_entry *entry;
	int count = 0;  

	entry = &log_entries[0];

	for (i = 0; i < last_entry; i++, entry++) {

		if ( entry->type == type ) {
			// if we are looking for a dock/undock entry, then we don't care about the order in which the names
			// were passed into this function.  Count the entry as found if either name matches both in the other
			// set.
			if ( (type == LOG_SHIP_DOCKED) || (type == LOG_SHIP_UNDOCKED) ) {
				if (sname == NULL) {
					Int3();
					return 0;
				}

				if ( (!stricmp(entry->pname, pname) && !stricmp(entry->sname, sname)) || (!stricmp(entry->pname, sname) && !stricmp(entry->sname, pname)) ) {
					count++;
				}
			} else {
				// for non dock/undock goals, then the names are important!
				if (pname == NULL) {
					Int3();
					return 0;
				}

				if ( stricmp(entry->pname, pname) != 0 ) {
					continue;
				}

				if ( (sname == NULL) || !stricmp(sname, entry->sname) ) {
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
	entry->text = vm_strdup(text);
	entry->color = msg_color;
	entry->x = x;
	entry->flags = flags;
}

void message_log_add_segs(const char* source_string, int msg_color, int flags = 0, SCP_vector<log_text_seg> *entry = nullptr)
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
			entry->push_back(new_seg);
		}

		if (!split) {
			gr_get_string_size(&w, NULL, str);
			X += w;
			break;
		}

		X = ACTION_X;
		str = split;
	}

	// free the buffer
	vm_free(dup_string);
}

int message_log_color_get_team(int msg_color)
{
	return msg_color - NUM_LOG_COLORS;
}

int message_log_team_get_color(int team)
{
	return NUM_LOG_COLORS + team;
}

// pw = total pixel width
void message_log_init_scrollback(int pw)
{
	P_width = pw;
	mission_log_cull_obsolete_entries();  // compact array so we don't have gaps

	Log_scrollback_vec.clear();
	
	Num_log_lines = 0;

	log_entry* entry;
	for (int i=0; i<last_entry; i++) {
		entry = &log_entries[i];

		if (entry->flags & MLF_HIDDEN)
			continue;

		// don't display failed bonus goals
		if (entry->type == LOG_GOAL_FAILED) {
			int type = Mission_goals[entry->index].type & GOAL_TYPE_MASK;

			if (type == BONUS_GOAL) {
				continue;
			}
		}

		log_line_complete thisEntry;

		// track time of event (normal timestamp milliseconds format)
		thisEntry.timestamp = entry->timestamp;

		// keep track of base color for the entry
		int thisColor;

		// Goober5000
		if ((entry->type == LOG_GOAL_SATISFIED) || (entry->type == LOG_GOAL_FAILED))
			thisColor = LOG_COLOR_BRIGHT;
		else if (entry->primary_team >= 0)
			thisColor = message_log_team_get_color(entry->primary_team);
		else
			thisColor = LOG_COLOR_OTHER;

		if ( (Lcl_gr) && ((entry->type == LOG_GOAL_FAILED) || (entry->type == LOG_GOAL_SATISFIED)) ) {
			// in german goal events, just say "objective" instead of objective name
			// this is cuz we can't translate objective names
			message_log_add_seg(&thisEntry.objective, OBJECT_X, thisColor, "Einsatzziel");
		} else if ( (Lcl_pl) && ((entry->type == LOG_GOAL_FAILED) || (entry->type == LOG_GOAL_SATISFIED)) ) {
			// same thing for polish
			message_log_add_seg(&thisEntry.objective, OBJECT_X, thisColor, "Cel misji");
		} else {
			message_log_add_seg(&thisEntry.objective, OBJECT_X, thisColor, entry->pname_display.c_str());
		}

		// now on to the actual message itself
		X = ACTION_X;

		// Goober5000
		if (entry->secondary_team >= 0)
			thisColor = message_log_team_get_color(entry->secondary_team);
		else
			thisColor = LOG_COLOR_NORMAL;

		char text[256];

		switch (entry->type) {
			case LOG_SHIP_DESTROYED:
				message_log_add_segs(XSTR( "Destroyed", 404), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				if (!entry->sname_display.empty()) {
					message_log_add_segs(XSTR("  Kill: ", 405), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
					message_log_add_segs(entry->sname_display.c_str(), thisColor, 0, &thisEntry.actions);
					if (entry->index >= 0) {
						sprintf(text, NOX(" (%d%%)"), entry->index);
						message_log_add_segs(text, LOG_COLOR_BRIGHT, 0, &thisEntry.actions);
					}
				}
				break;

			case LOG_SELF_DESTRUCTED:
				message_log_add_segs(XSTR("Self destructed", 1476), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;

			case LOG_WING_DESTROYED:
				message_log_add_segs(XSTR("Destroyed", 404), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;

			case LOG_SHIP_ARRIVED:
				message_log_add_segs(XSTR("Arrived", 406), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;

			case LOG_WING_ARRIVED:
				if (entry->index > 1){
					sprintf(text, XSTR( "Arrived (wave %d)", 407), entry->index);
				} else {
					strcpy_s(text, XSTR( "Arrived", 406));
				}
				message_log_add_segs(text, LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;

			case LOG_SHIP_DEPARTED:
				message_log_add_segs(XSTR("Departed", 408), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;

			case LOG_WING_DEPARTED:
				message_log_add_segs(XSTR("Departed", 408), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;

			case LOG_SHIP_DOCKED:
				message_log_add_segs(XSTR("Docked with ", 409), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				message_log_add_segs(entry->sname_display.c_str(), thisColor, 0, &thisEntry.actions);
				break;

			case LOG_SHIP_SUBSYS_DESTROYED: {
				int si_index, model_index;

				si_index = (int)((entry->index >> 16) & 0xffff);
				model_index = (int)(entry->index & 0xffff);

				message_log_add_segs(XSTR("Subsystem ", 410), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				//message_log_add_segs(entry->sname, LOG_COLOR_BRIGHT);
				const char *subsys_name = Ship_info[si_index].subsystems[model_index].subobj_name;
				if (Ship_info[si_index].subsystems[model_index].type == SUBSYSTEM_TURRET) {
					subsys_name = XSTR("Turret", 1487);
				}
				message_log_add_segs(subsys_name, LOG_COLOR_BRIGHT, 0, &thisEntry.actions);
				message_log_add_segs(XSTR(" destroyed", 411), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;
			}

			case LOG_SHIP_UNDOCKED:
				message_log_add_segs(XSTR("Undocked with ", 412), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				message_log_add_segs(entry->sname_display.c_str(), thisColor, 0, &thisEntry.actions);
				break;

			case LOG_SHIP_DISABLED:
				message_log_add_segs(XSTR("Disabled", 413), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;

			case LOG_SHIP_DISARMED:
				message_log_add_segs(XSTR("Disarmed", 414), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;

			case LOG_PLAYER_CALLED_FOR_REARM:
				message_log_add_segs(XSTR(" called for rearm", 415), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;

			case LOG_PLAYER_ABORTED_REARM:
				message_log_add_segs(XSTR(" aborted rearm", 416), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;

			case LOG_PLAYER_CALLED_FOR_REINFORCEMENT:
				message_log_add_segs(XSTR("Called in as reinforcement", 417), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				break;

			case LOG_CARGO_REVEALED:
				Assert( entry->index >= 0 );
				Assert(!(entry->index & CARGO_NO_DEPLETE));

				message_log_add_segs(XSTR("Cargo revealed: ", 418), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				strncpy(text, Cargo_names[entry->index], sizeof(text) - 1);
				message_log_add_segs(text, LOG_COLOR_BRIGHT, 0, &thisEntry.actions);
				break;

			case LOG_CAP_SUBSYS_CARGO_REVEALED:
				Assert( entry->index >= 0 );
				Assert(!(entry->index & CARGO_NO_DEPLETE));

				message_log_add_segs(entry->sname_display.c_str(), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				message_log_add_segs(XSTR( " subsystem cargo revealed: ", 1488), LOG_COLOR_NORMAL, 0, &thisEntry.actions);
				strncpy(text, Cargo_names[entry->index], sizeof(text) - 1);
				message_log_add_segs(text, LOG_COLOR_BRIGHT, 0, &thisEntry.actions);
				break;


			case LOG_GOAL_SATISFIED:
			case LOG_GOAL_FAILED: {
				int type = Mission_goals[entry->index].type & GOAL_TYPE_MASK;
				sprintf( text, XSTR( "%s objective ", 419), Goal_type_text(type) );
				if ( entry->type == LOG_GOAL_SATISFIED )
					strcat_s(text, XSTR( "satisfied.", 420));
				else
					strcat_s(text, XSTR( "failed.", 421));

				message_log_add_segs(text, LOG_COLOR_BRIGHT, (entry->type == LOG_GOAL_SATISFIED?LOG_FLAG_GOAL_TRUE:LOG_FLAG_GOAL_FAILED), &thisEntry.actions );
				break;
			}	// matches case statement!
			default:
				UNREACHABLE("Unhandled enum value!");
				break;
		}

		Log_scrollback_vec.push_back(thisEntry);

		Num_log_lines++;

	}
}

void message_log_shutdown_scrollback()
{
	Log_scrollback_vec.clear();

	Num_log_lines = 0;
}

// message_log_scrollback displays the contents of the mesasge log currently much like the HUD
// message scrollback system.  I'm sure this system will be overhauled.		
void mission_log_scrollback(int line_offset, int list_x, int list_y, int list_w, int list_h)
{
	int font_h = gr_get_font_height();

	int y = 0;

	for (int i = line_offset; i < (int)Log_scrollback_vec.size(); i++) {

		// if we're beyond the printable area stop printing!
		if (y + font_h > list_h)
			break;

		bool printSymbols = false;
		int symbolFlag = 0;

		// print the timestamp
		gr_set_color_fast(&Color_text_normal);
		gr_print_timestamp(list_x + TIME_X, list_y + y, Log_scrollback_vec[i].timestamp, GR_RESIZE_MENU);

		// print the objective
		switch (Log_scrollback_vec[i].objective.color) {
			case LOG_COLOR_BRIGHT:
				gr_set_color_fast(&Color_bright);
				break;

			case LOG_COLOR_OTHER:
				gr_set_color_fast(&Color_normal);
				break;

			default: {
				int team = message_log_color_get_team(Log_scrollback_vec[i].objective.color);
				if (team < 0)
					gr_set_color_fast(&Color_text_normal);
				else
					gr_set_color_fast(iff_get_color_by_team(team, -1, 1));

				break;
			}
		}
			
		char buf[256];
		strcpy_s(buf, Log_scrollback_vec[i].objective.text);
		font::force_fit_string(buf, 256, ACTION_X - OBJECT_X - 8);
		gr_string(list_x + Log_scrollback_vec[i].objective.x, list_y + y, buf, GR_RESIZE_MENU);

		// print the actions
		for (int j = 0; j < (int)Log_scrollback_vec[i].actions.size(); j++) {

			log_text_seg thisSeg = Log_scrollback_vec[i].actions[j];

			switch (thisSeg.color) {
				case LOG_COLOR_BRIGHT:
					gr_set_color_fast(&Color_bright);
					break;

				case LOG_COLOR_OTHER:
					gr_set_color_fast(&Color_normal);
					break;

				default: {
					int team = message_log_color_get_team(thisSeg.color);
					if (team < 0)
						gr_set_color_fast(&Color_text_normal);
					else
						gr_set_color_fast(iff_get_color_by_team(team, -1, 1));

					break;
				}
			}

			strcpy_s(buf, thisSeg.text);
			font::force_fit_string(buf, 256, ACTION_X - OBJECT_X - 8);
			gr_string(list_x + thisSeg.x, list_y + y, buf, GR_RESIZE_MENU);

			if ((thisSeg.flags & LOG_FLAG_GOAL_TRUE) || (thisSeg.flags & LOG_FLAG_GOAL_FAILED)) {
				printSymbols = true;
				symbolFlag = thisSeg.flags;
			}
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
