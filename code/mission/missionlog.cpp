/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionLog.cpp $
 * $Revision: 2.15 $
 * $Date: 2006-05-20 02:03:01 $
 * $Author: Goober5000 $
 *
 * File to deal with Mission logs
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.14  2006/03/19 05:05:59  taylor
 * make sure the mission log doesn't modify stuff in Cargo_names[], since it shouldn't
 * have split_str_once() be sure to not split a word in half, it should end up on the second line instead
 *
 * Revision 2.13  2006/01/13 03:31:09  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 2.12  2005/10/30 20:03:39  taylor
 * add a bunch of Assert()'s and NULL checks to either help debug or avoid errors
 * fix Mantis bug #381
 * fix a small issue with the starfield bitmap removal sexp since it would read one past the array size
 *
 * Revision 2.11  2005/10/10 17:21:05  taylor
 * remove NO_NETWORK
 *
 * Revision 2.10  2005/07/22 10:18:39  Goober5000
 * CVS header tweaks
 * --Goober5000
 *
 * Revision 2.9  2005/07/13 03:25:59  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.8  2005/05/12 17:49:13  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.7  2005/03/02 21:24:45  taylor
 * more network/inferno goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.6  2005/02/04 10:12:31  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.5  2005/01/11 21:28:24  Goober5000
 * made a capitalization thing consistent
 * --Goober500
 *
 * Revision 2.4  2004/07/26 20:47:37  Kazan
 * remove MCD complete
 *
 * Revision 2.3  2004/07/12 16:32:54  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.2  2004/03/05 09:02:06  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/10 20:42:44  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 10    11/01/99 2:13p Jefff
 * some changes in how objective events are displayed in german
 * 
 * 9     10/13/99 3:22p Jefff
 * fixed unnumbered XSTRs
 * 
 * 8     9/08/99 4:05p Dave
 * Fixed string ID on self-destruxt message.
 * 
 * 7     9/06/99 8:18p Andsager
 * Make destroyed weapon subsystems display as turrets in event log.
 * 
 * 6     8/26/99 8:51p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 5     8/22/99 5:53p Dave
 * Scoring fixes. Added self destruct key. Put callsigns in the logfile
 * instead of ship designations for multiplayer players.
 * 
 * 4     2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 3     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 57    6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 56    5/07/98 1:11a Ed
 * AL: fix bogus null pointer test in message_log_add_seg()
 * 
 * 55    4/28/98 4:45p Allender
 * fix mission log problems on clients
 * 
 * 54    4/23/98 10:06a Allender
 * don't use the word "player" in event log for rearm event.  Send
 * shipname instead (players only)
 * 
 * 53    3/28/98 1:56p Allender
 * clarify the destroyed mission log entry.
 * 
 * 52    3/13/98 2:49p Allender
 * put fix into debug code so that it doesn't trip when no ships have
 * departed (i.e. wing was waxes before they actulaly left)
 * 
 * 51    3/12/98 5:52p Hoffoss
 * Moved stuff over so icons don't overlap frame.
 * 
 * 50    3/09/98 6:22p Hoffoss
 * Fixed text overrun problems in event log display.
 * 
 * 49    2/25/98 4:44p Allender
 * fix up mission log and wing/ship departed problems (hopefully).  There
 * was problem where wing was marked departed although all ships were
 * destroyed (I think).
 * 
 * 48    2/25/98 11:07a Allender
 * added debug code to help trap problems with wing departure and ship
 * departure weirdness
 * 
 * 47    2/23/98 8:55a John
 * More string externalization
 * 
 * 46    2/22/98 4:30p John
 * More string externalization classification
 * 
 * 45    2/10/98 12:47p Allender
 * clear log memeory before mission start to prevent entries from previous
 * mission from being used incorrectly
 * 
 * 44    2/05/98 10:33a Hoffoss
 * Changed refernce go "goal" to "objective" instead.
 * 
 * 43    1/30/98 11:16a Comet
 * DOH!  fix bug checking for exited ships
 * 
 * 42    1/08/98 1:59p Allender
 * fixed problem with some subsystems not getting recognized on
 * is-subsystem-destroyed sexpressions
 * 
 * 41    1/07/98 4:41p Allender
 * minor modification to special messages.  Fixed cargo_revealed problem
 * for multiplayer and problem with is-cargo-known sexpression
 * 
 * 40    11/21/97 10:31a Allender
 * minor cosmetic changes
 * 
 * 39    11/20/97 3:52p Hoffoss
 * Fixed timestamp color in mission log scrollback.
 * 
 * 38    11/19/97 2:30p Allender
 * hide waypoints done log entries
 * 
 * 37    11/13/97 4:25p Allender
 * hide certain mission log entries
 * 
 * 36    11/13/97 4:05p Hoffoss
 * Added hiding code for mission log entries.
 * 
 * 35    11/06/97 5:42p Hoffoss
 * Added support for fixed size timstamp rendering.
 * 
 * 34    11/03/97 10:12p Hoffoss
 * Finished up work on the hud message/mission log scrollback screen.
 * 
 * 33    10/09/97 5:21p Allender
 * try to check for bogus secondary names on ship destroy.
 * 
 * 32    10/07/97 3:09p Allender
 * fix typo
 * 
 * 31    9/26/97 3:47p Allender
 * add code to keep track of ships which have left mission (departed or
 * destroyed).  Log who killed who in mission log
 * 
 * 30    9/10/97 3:30p Allender
 * ignore waypoints_done entry when determining teams for log actions
 * 
 * 29    9/10/97 8:53a Allender
 * made the mission log prettier.  Added team info to log entries -- meant
 * reordering some code
 * 
 * 28    8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 27    8/20/97 3:53p Allender
 * minor fix to message log display
 * 
 * 26    8/07/97 11:37a Allender
 * made mission log cull non-essential entries when log starts getting
 * full.  More drastic action is taken as the log gets more full.
 * 
 * 25    7/24/97 4:13p Mike
 * Comment out check in mission_log_add_entry that prevented logging
 * events when player is dead.
 * 
 * 24    7/14/97 11:46a Lawrance
 * allow log entries when in the navmap
 * 
 * 23    7/10/97 1:40p Allender
 * make dock/undock log entries work with either name (docker/dockee)
 * coming first
 * 
 * 22    7/07/97 9:23a Allender
 * added code for valid/invalid goals.  Invalid goals are eval'ed,
 * counted, or otherwise used
 * 
 * 21    7/01/97 2:52p Allender
 * added packets for mission log stuff and for pregame chat stuff
 * 
 * 20    7/01/97 1:23p Allender
 * temp checkin
 * 
*/


#include "mission/missionlog.h"
#include "playerman/player.h"
#include "graphics/font.h"
#include "mission/missiongoals.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "mission/missionparse.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "iff_defs/iff_defs.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"



#define MAX_LOG_ENTRIES		700
#define MAX_LOG_LINES		1000

// used for high water mark for culling out log entries
#define LOG_CULL_MARK				((int)(MAX_LOG_ENTRIES * 0.95f))
#define LOG_CULL_DOORDIE_MARK		((int)(MAX_LOG_ENTRIES * 0.99f))
#define LOG_LAST_DITCH_CULL_NUM	((int)(MAX_LOG_ENTRIES * 0.20f))
#define LOG_HALFWAY_REPORT_NUM	((int)(MAX_LOG_ENTRIES * 0.50f))

#define ML_FLAG_PRIMARY		1
#define ML_FLAG_SECONDARY	2

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
	log_text_seg *next;		// linked list
	char	*text;				// the text
	int	color;				// color text should be displayed in
	int	x;						// x offset to display text at
	int	flags;				// used to possibly print special characters when displaying the log
} log_text_seg;

int Num_log_lines;
static int X, P_width;

// Log_lines is used for scrollback display purposes.
static log_text_seg *Log_lines[MAX_LOG_LINES];
static int Log_line_timestamps[MAX_LOG_LINES];

log_entry log_entries[MAX_LOG_ENTRIES];	// static array because John says....
int last_entry;

void mission_log_init()
{
	last_entry = 0;

	// zero out all the memory so we don't get bogus information when playing across missions!
	memset( log_entries, 0, sizeof(log_entries) );
}

// returns the number of entries in the mission log
int mission_log_query_scrollback_size()
{
	return last_entry;
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
void mission_log_obsolete_entries(int type, char *pname)
{
	int i;
	log_entry *entry = NULL;

	// before adding this entry, check to see if the entry type is a ship destroyed entry.
	// If so, we can remove any subsystem destroyed entries from the log for this ship.  
	if ( type == LOG_SHIP_DESTROYED ) {
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

// assigns flag values to an entry based on the team value passed in
void mission_log_flag_team( log_entry *entry, int which_entry, int team )
{
	if (which_entry == ML_FLAG_PRIMARY)
	{
		entry->primary_team = team;
	}
	else if (which_entry == ML_FLAG_SECONDARY)
	{
		entry->secondary_team = team;
	}
	else
	{
		// get allender -- impossible type
		Int3();
	}
}

// following function adds an entry into the mission log.
// pass a type and a string which indicates the object
// that this event is for.  Don't add entries with this function for multiplayer
void mission_log_add_entry(int type, char *pname, char *sname, int info_index)
{
	int last_entry_save;
	log_entry *entry;	

	// multiplayer clients don't use this function to add log entries -- they will get
	// all their info from the host
	if ( (Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER) ){
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
		strcpy(entry->pname, pname);
	} else
		strcpy( entry->pname, EMPTY_LOG_NAME );

	if ( sname ) {
		Assert (strlen(sname) < NAME_LENGTH);
		strcpy(entry->sname, sname);
	} else
		strcpy( entry->sname, EMPTY_LOG_NAME );

	entry->index = info_index;
	entry->flags = 0;

	// determine the contents of the flags member based on the type of entry we added.  We need to store things
	// like team for the primary and (possibly) secondary object for this entry.
	switch ( type ) {
	int index, si;

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

		Assert ( index != -1 );
		if(index < 0){
			mission_log_flag_team( entry, ML_FLAG_PRIMARY, Player_ship->team );		
		} else {
			mission_log_flag_team( entry, ML_FLAG_PRIMARY, Ships[index].team );		
		}

		// some of the entries have a secondary component.  Figure out what is up with them.
		if ( (type == LOG_SHIP_DOCKED) || (type == LOG_SHIP_UNDOCKED)) {
			if ( sname ) {
				index = ship_name_lookup( sname );
				Assert( index != -1 );
				mission_log_flag_team( entry, ML_FLAG_SECONDARY, Ships[index].team );
			}
		} else if ( type == LOG_SHIP_DESTROYED ) {
			if ( sname ) {
				int team;

				// multiplayer, player name will possibly be sent in
				if((Game_mode & GM_MULTIPLAYER) && (multi_find_player_by_callsign(sname) >= 0)) {
					// get the player's ship
					int np_index = multi_find_player_by_callsign(sname);
					int np_ship = multi_get_player_ship(np_index);

					if(np_ship < 0)
					{
						// argh. badness
						Int3();
						team = Player_ship->team;
					}
					else
					{
						team = Ships[Objects[Net_players[np_index].m_player->objnum].instance].team;
					}
				}
				else 
				{
					index = ship_name_lookup( sname );
					// no ship, then it probably exited -- check the exited 
					if ( index == -1 ) {
						index = ship_find_exited_ship_by_name( sname );
						if ( index == -1 ) {
						//	Int3();		// get allender.  name of object who killed ship appears to be bogus!!!
							break;
						}
						team = Ships_exited[index].team;
					} else {
						team = Ships[index].team;
					}
				}

				mission_log_flag_team( entry, ML_FLAG_SECONDARY, team );
 			} else {
				nprintf(("missionlog", "No secondary name for ship destroyed log entry!\n"));
			}
		} else if ( (type == LOG_SHIP_SUBSYS_DESTROYED) && (Ship_info[Ships[index].ship_info_index].flags & SIF_SMALL_SHIP) ) {
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
		index = wing_name_lookup( pname, 1 );
		Assert( index != -1 );
		Assert( info_index != -1 );			// this is the team value

		// get the team value for this wing.  Departed or destroyed wings will pass the team
		// value in info_index parameter.  For arriving wings, get the team value from the
		// first ship in the list
		if ( type == LOG_WING_ARRIVED ) {
			si = Wings[index].ship_index[0];
			Assert( si != -1 );
			mission_log_flag_team( entry, ML_FLAG_PRIMARY, Ships[si].team );
		} else {
			mission_log_flag_team( entry, ML_FLAG_PRIMARY, info_index );
		}

#ifndef NDEBUG
		// MWA 2/25/98.  debug code to try to find any ships in this wing that have departed.
		// scan through all log entries and find at least one ship_depart entry for a ship
		// that was in this wing.
		if ( type == LOG_WING_DEPARTED ) {
			int i;

			// if all were destroyed, then don't do this debug code.
			if ( Wings[index].total_destroyed == Wings[index].total_arrived_count ){
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
	if ( (Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) ){
		send_mission_log_packet( last_entry );
	}

	last_entry++;

#ifndef NDEBUG
	if ( !(last_entry % 10) ) {
		if ( (last_entry > LOG_HALFWAY_REPORT_NUM) && (last_entry > last_entry_save) ){
			nprintf(("missionlog", "new highwater point reached for mission log (%d entries).\n", last_entry));
		}
	}
#endif

}

// function, used in multiplayer only, which adds an entry sent by the host of the game, into
// the mission log.  The index of the log entry is passed as one of the parameters in addition to
// the normal parameters used for adding an entry to the log
void mission_log_add_entry_multi( int type, char *pname, char *sname, int index, fix timestamp, int flags )
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
		strcpy(entry->pname, pname);
	}
	if ( sname ) {
		Assert (strlen(sname) < NAME_LENGTH);
		strcpy(entry->sname, sname);
	}
	entry->index = index;

	entry->flags = flags;
	entry->timestamp = timestamp;
}

// function to determine is the given event has taken place count number of times.

int mission_log_get_time_indexed( int type, char *pname, char *sname, int count, fix *time)
{
	int i, found;
	log_entry *entry;

	entry = &log_entries[0];
	for (i = 0; i < last_entry; i++) {
		found = 0;
		if ( entry->type == type ) {
			// if we are looking for a dock/undock entry, then we don't care about the order in which the names
			// were passed into this function.  Count the entry as found if either name matches both in the other
			// set.
			if ( (type == LOG_SHIP_DOCKED) || (type == LOG_SHIP_UNDOCKED) ) {
				Assert ( sname );
				if ( (!stricmp(entry->pname, pname) && !stricmp(entry->sname, sname)) || (!stricmp(entry->pname, sname) && !stricmp(entry->sname, pname)) )
					found = 1;
			} else {
				// for non dock/undock goals, then the names are important!
				Assert( pname );
				if ( stricmp(entry->pname, pname) )
					goto next_entry;
				if ( !sname || !stricmp(sname, entry->sname) )
					found = 1;
			}

			if ( found ) {
				count--;
				if ( !count ) {
					entry->flags |= MLF_ESSENTIAL;				// since the goal code asked for this entry, mark it as essential
					if ( time )
						*time = entry->timestamp;
					return 1;
				}
			}
		}

next_entry:
		entry++;
	}

	return 0;
}

// this function determines if the given type of event on the specified
// object has taken place yet.  If not, it returns 0.  If it has, the
// timestamp that the event happened is returned in the time parameter
int mission_log_get_time( int type, char *pname, char *sname, fix *time )
{
	return mission_log_get_time_indexed( type, pname, sname, 1, time );
}

void message_log_add_seg(int n, int x, int color, char *text, int flags = 0)
{
	log_text_seg *seg, **parent;

	if ((n < 0) || (n >= MAX_LOG_LINES))
		return;

	parent = &Log_lines[n];
	while (*parent)
		parent = &((*parent)->next);

	seg = (log_text_seg *) vm_malloc(sizeof(log_text_seg));
	Assert(seg);
	seg->text = vm_strdup(text);
	seg->color = color;
	seg->x = x;
	seg->flags = flags;
	seg->next = NULL;
	*parent = seg;
}

void message_log_add_segs(char *text, int color, int flags = 0)
{
	char *ptr;
	int w;

	while (1) {
		if (X == ACTION_X) {
			while (is_white_space(*text))
				text++;
		}

		if (!text) {
			mprintf(("Why are you passing a NULL pointer to message_log_add_segs?\n"));
			return;
		}

		if ( !text[0] ) {
			return;
		}

		if (P_width - X < 1)
			ptr = text;
		else
			ptr = split_str_once(text, P_width - X);

		if (ptr != text)
			message_log_add_seg(Num_log_lines, X, color, text, flags);

		if (!ptr) {
			gr_get_string_size(&w, NULL, text);
			X += w;
			return;
		}

		Num_log_lines++;
		X = ACTION_X;
		text = ptr;
	}
}

void message_log_remove_segs(int n)
{
	log_text_seg *ptr, *ptr2;

	if ((n < 0) || (n >= MAX_LOG_LINES))
		return;

	ptr = Log_lines[n];
	while (ptr) {
		ptr2 = ptr->next;
		vm_free(ptr);
		ptr = ptr2;
	}

	Log_lines[n] = NULL;
}

int message_log_color_get_team(int color)
{
	return color - NUM_LOG_COLORS;
}

int message_log_team_get_color(int team)
{
	return NUM_LOG_COLORS + team;
}

// pw = total pixel width
void message_log_init_scrollback(int pw)
{
	char text[256];
	log_entry *entry;
	int i, c, kill, type;

	P_width = pw;
	mission_log_cull_obsolete_entries();  // compact array so we don't have gaps
	
	// initialize the log lines data
	Num_log_lines = 0;
	for (i=0; i<MAX_LOG_LINES; i++) {
		Log_lines[i] = NULL;
		Log_line_timestamps[i] = 0;
	}

	for (i=0; i<last_entry; i++) {
		entry = &log_entries[i];

		if (entry->flags & MLF_HIDDEN)
			continue;

		// track time of event (normal timestamp milliseconds format)
		Log_line_timestamps[Num_log_lines] = (int) ( f2fl(entry->timestamp) * 1000.0f );

		// Generate subject ship text for entry
		if ( (entry->type == LOG_GOAL_SATISFIED) || (entry->type == LOG_GOAL_FAILED) ){
			c = LOG_COLOR_BRIGHT;
		} else {
			c = message_log_team_get_color(entry->primary_team);
		}

		if ( (Lcl_gr) && ((entry->type == LOG_GOAL_FAILED) || (entry->type == LOG_GOAL_SATISFIED)) ) {
			// in german goal events, just say "objective" instead of objective name
			// this is cuz we can't translate objective names
			message_log_add_seg(Num_log_lines, OBJECT_X, c, "Einsatzziel");
		} else {
			message_log_add_seg(Num_log_lines, OBJECT_X, c, entry->pname);
		}

		// now on to the actual message itself
		X = ACTION_X;
		kill = 0;

		// Goober5000
		c = message_log_team_get_color(entry->secondary_team);

		switch (entry->type) {
			case LOG_SHIP_DESTROYED:
				message_log_add_segs(XSTR( "Destroyed", 404), LOG_COLOR_NORMAL);
				if (strlen(entry->sname)) {
					message_log_add_segs(XSTR( "  Kill: ", 405), LOG_COLOR_NORMAL);
					message_log_add_segs(entry->sname, c);
					if (entry->index >= 0) {
						sprintf(text, NOX(" (%d%%)"), entry->index);
						message_log_add_segs(text, LOG_COLOR_BRIGHT);
					}
				}
				break;

			case LOG_SELF_DESTRUCTED:
				message_log_add_segs(XSTR( "Self Destructed", 1476), LOG_COLOR_NORMAL);
				break;

			case LOG_WING_DESTROYED:
				message_log_add_segs(XSTR( "Destroyed", 404), LOG_COLOR_NORMAL);
				break;

			case LOG_SHIP_ARRIVED:
				message_log_add_segs(XSTR( "Arrived", 406), LOG_COLOR_NORMAL);
				break;

			case LOG_WING_ARRIVED:
				if (entry->index > 1){
					sprintf(text, XSTR( "Arrived (wave %d)", 407), entry->index);
				} else {
					strcpy(text, XSTR( "Arrived", 406));
				}
				message_log_add_segs(text, LOG_COLOR_NORMAL);
				break;

			case LOG_SHIP_DEPARTED:
				message_log_add_segs(XSTR( "Departed", 408), LOG_COLOR_NORMAL);
				break;

			case LOG_WING_DEPARTED:
				message_log_add_segs(XSTR( "Departed", 408), LOG_COLOR_NORMAL);
				break;

			case LOG_SHIP_DOCKED:
				message_log_add_segs(XSTR( "Docked with ", 409), LOG_COLOR_NORMAL);
				message_log_add_segs(entry->sname, c);
				break;

			case LOG_SHIP_SUBSYS_DESTROYED: {
				int si_index, model_index;

				si_index = (int)((entry->index >> 16) & 0xffff);
				model_index = (int)(entry->index & 0xffff);

				message_log_add_segs(XSTR( "Subsystem ", 410), LOG_COLOR_NORMAL);
				//message_log_add_segs(entry->sname, LOG_COLOR_BRIGHT);
				char *subsys_name = Ship_info[si_index].subsystems[model_index].name;
				if (Ship_info[si_index].subsystems[model_index].type == SUBSYSTEM_TURRET) {
					subsys_name = XSTR("Turret", 1487);
				}
				message_log_add_segs(subsys_name, LOG_COLOR_BRIGHT);
				message_log_add_segs(XSTR( " destroyed", 411), LOG_COLOR_NORMAL);
				break;
			}

			case LOG_SHIP_UNDOCKED:
				message_log_add_segs(XSTR( "Undocked with ", 412), LOG_COLOR_NORMAL);
				message_log_add_segs(entry->sname, c);
				break;

			case LOG_SHIP_DISABLED:
				message_log_add_segs(XSTR( "Disabled", 413), LOG_COLOR_NORMAL);
				break;

			case LOG_SHIP_DISARMED:
				message_log_add_segs(XSTR( "Disarmed", 414), LOG_COLOR_NORMAL);
				break;

			case LOG_PLAYER_CALLED_FOR_REARM:
				message_log_add_segs(XSTR( " called for rearm", 415), LOG_COLOR_NORMAL);
				break;

			case LOG_PLAYER_ABORTED_REARM:
				message_log_add_segs(XSTR( " aborted rearm", 416), LOG_COLOR_NORMAL);
				break;

			case LOG_PLAYER_CALLED_FOR_REINFORCEMENT:
				message_log_add_segs(XSTR( "Called in as reinforcement", 417), LOG_COLOR_NORMAL);
				break;

			case LOG_CARGO_REVEALED:
				Assert( entry->index != -1 );
				message_log_add_segs(XSTR( "Cargo revealed: ", 418), LOG_COLOR_NORMAL);
				strncpy(text, Cargo_names[entry->index], sizeof(text) - 1);
				message_log_add_segs( text, LOG_COLOR_BRIGHT );
				break;

			case LOG_CAP_SUBSYS_CARGO_REVEALED:
				Assert( entry->index != -1 );
				message_log_add_segs(entry->sname, LOG_COLOR_NORMAL);
				message_log_add_segs(XSTR( " subsystem cargo revealed: ", 1488), LOG_COLOR_NORMAL);
				strncpy(text, Cargo_names[entry->index], sizeof(text) - 1);
				message_log_add_segs( text, LOG_COLOR_BRIGHT );
				break;


			case LOG_GOAL_SATISFIED:
			case LOG_GOAL_FAILED: {
				type = Mission_goals[entry->index].type & GOAL_TYPE_MASK;

				// don't display failed bonus goals
				if ( (type == BONUS_GOAL) && (entry->type == LOG_GOAL_FAILED) ) {
					kill = 1;
					break;  // don't display this line
				}

				sprintf( text, XSTR( "%s objective ", 419), Goal_type_text(type) );
				if ( entry->type == LOG_GOAL_SATISFIED )
					strcat(text, XSTR( "satisfied.", 420));
				else
					strcat(text, XSTR( "failed.", 421));

				message_log_add_segs(text, LOG_COLOR_BRIGHT, (entry->type == LOG_GOAL_SATISFIED?LOG_FLAG_GOAL_TRUE:LOG_FLAG_GOAL_FAILED) );
				break;
			}	// matches case statement!
		}

		if (kill) {
			message_log_remove_segs(Num_log_lines);

		} else {
			if (Num_log_lines < MAX_LOG_LINES)
				Num_log_lines++;
		}
	}
}

void message_log_shutdown_scrollback()
{
	int i;

	for (i=0; i<MAX_LOG_LINES; i++)
		message_log_remove_segs(i);

	Num_log_lines = 0;
}

// message_log_scrollback displays the contents of the mesasge log currently much like the HUD
// message scrollback system.  I'm sure this system will be overhauled.		
void mission_log_scrollback(int line, int list_x, int list_y, int list_w, int list_h)
{
	char buf[256];
	int y;
	int font_h = gr_get_font_height();
	log_text_seg *seg;

	y = 0;
	while (y + font_h <= list_h) {
		if (line >= Num_log_lines)
			break;

		if (Log_line_timestamps[line]) {
			gr_set_color_fast(&Color_text_normal);
			gr_print_timestamp(list_x + TIME_X, list_y + y, Log_line_timestamps[line]);
		}

		seg = Log_lines[line];
		while (seg) {
			switch (seg->color) {
				case LOG_COLOR_BRIGHT:
					gr_set_color_fast(&Color_bright);
					break;

				case LOG_COLOR_OTHER:
					gr_set_color_fast(&Color_normal);
					break;

				default:
				{
					int team = message_log_color_get_team(seg->color);
					if (team < 0)
						gr_set_color_fast(&Color_text_normal);
					else
						gr_set_color_fast(iff_get_color_by_team(team, -1, 1));

					break;
				}
			}

			strcpy(buf, seg->text);
			if (seg->x < ACTION_X)
				gr_force_fit_string(buf, 256, ACTION_X - OBJECT_X - 8);
			else
				gr_force_fit_string(buf, 256, list_w - seg->x);

			gr_string(list_x + seg->x, list_y + y, buf);

			// possibly "print" some symbols for interesting log entries
			if ( (seg->flags & LOG_FLAG_GOAL_TRUE) || (seg->flags & LOG_FLAG_GOAL_FAILED) ) {
				int i;

				if ( seg->flags & LOG_FLAG_GOAL_FAILED )
					gr_set_color_fast(&Color_bright_red);
				else
					gr_set_color_fast(&Color_bright_green);

				i = list_y + y + font_h / 2 - 1;
				gr_circle(list_x + TIME_X - 6, i, 5);

				gr_set_color_fast(&Color_bright);
				gr_line(list_x + TIME_X - 10, i, list_x + TIME_X - 8, i);
				gr_line(list_x + TIME_X - 6, i - 4, list_x + TIME_X - 6, i - 2);
				gr_line(list_x + TIME_X - 4, i, list_x + TIME_X - 2, i);
				gr_line(list_x + TIME_X - 6, i + 2, list_x + TIME_X - 6, i + 4);
			}

			seg = seg->next;
		}

		y += font_h;
		line++;
	}
}
