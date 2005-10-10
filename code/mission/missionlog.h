/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionLog.h $
 * $Revision: 2.4 $
 * $Date: 2005-10-10 17:21:05 $
 * $Author: taylor $
 *
 * Header file to deal with Mission logs
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2005/07/13 03:25:59  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.2  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2004/03/05 09:02:06  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
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
 * 4     8/22/99 5:53p Dave
 * Scoring fixes. Added self destruct key. Put callsigns in the logfile
 * instead of ship designations for multiplayer players.
 * 
 * 3     2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 21    4/28/98 4:45p Allender
 * fix mission log problems on clients
 * 
 * 20    1/07/98 4:41p Allender
 * minor modification to special messages.  Fixed cargo_revealed problem
 * for multiplayer and problem with is-cargo-known sexpression
 * 
 * 19    11/13/97 1:21p Hoffoss
 * Added a hidden flag to entries can be hidden from being displayed to
 * the user (but kept in log for checking things).
 * 
 * 18    11/03/97 10:12p Hoffoss
 * Finished up work on the hud message/mission log scrollback screen.
 * 
 * 17    9/10/97 8:53a Allender
 * made the mission log prettier.  Added team info to log entries -- meant
 * reordering some code
 * 
 * 16    8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 15    8/07/97 11:37a Allender
 * made mission log cull non-essential entries when log starts getting
 * full.  More drastic action is taken as the log gets more full.
 * 
 * 14    7/02/97 10:49p Allender
 * added waypoints-done sexpressions
 * 
 * 13    7/01/97 2:52p Allender
 * added packets for mission log stuff and for pregame chat stuff
 * 
 * 12    3/04/97 1:21p Mike
 * Repair/Rearm aborting, code cleanup.
 * 
 * 11    3/03/97 1:21p Allender
 * mission log stuff -- display the log during/after game.  Enhanced
 * structure
 * 
 * 10    2/20/97 5:05p Allender
 * support for docking and undocking multiple times -- also able to
 * specify dock points through sexpression
 * 
 * 9     2/17/97 3:50p Allender
 * change to allow ability to destroy single subsystem (through orders) or
 * all engines (disable) or all turrets (disarm)
 * 
 * 8     1/06/97 10:44p Lawrance
 * Changes to make save/restore functional
 * 
 * 7     1/06/97 11:29a Allender
 * added define for logging a ship that undocks
 * 
 * 6     1/01/97 4:18p Allender
 * added new field in mission log entry -- secondary name.  Used when
 * there are two objects for a log entry (i.e. docking)
 * 
 * 5     12/17/96 1:04p Allender
 * added subtype field for mission messages -- not used yet and may be
 * removed
 * 
 * 4     11/08/96 9:06a Allender
 * added LOG_SHIP_DOCK
 * 
 * 3     10/24/96 8:36a Allender
 * added a couple of new event types
 * 
 * 2     10/23/96 12:48p Allender
 * increased mission log functionality -- function to get the timestamp
 * that a recorded event happened at
 * 
 * 1     10/22/96 4:49p Allender
 * Mission log files
 * 
*/

#ifndef _MISSIONLOG_H
#define _MISSIONLOG_H

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"

// defined for different mission log entries

#define LOG_SHIP_DESTROYED				1
#define LOG_WING_DESTROYED				2
#define LOG_SHIP_ARRIVE					3
#define LOG_WING_ARRIVE					4
#define LOG_SHIP_DEPART					5
#define LOG_WING_DEPART					6
#define LOG_SHIP_DOCK					7
#define LOG_SHIP_SUBSYS_DESTROYED	8
#define LOG_SHIP_UNDOCK					9
#define LOG_SHIP_DISABLED				10
#define LOG_SHIP_DISARMED				11
#define LOG_PLAYER_REARM				12
#define LOG_PLAYER_REINFORCEMENT		13
#define LOG_GOAL_SATISFIED				14
#define LOG_GOAL_FAILED					15
#define LOG_PLAYER_REARM_ABORT		16
#define LOG_WAYPOINTS_DONE				17
#define LOG_CARGO_REVEALED				18
#define LOG_CAP_SUBSYS_CARGO_REVEALED 19
#define LOG_SELF_DESTRUCT				20

// structure definition for log entries

#define MLF_ESSENTIAL				(1<<0)		// this entry is essential for goal checking code
#define MLF_OBSOLETE					(1<<1)		// this entry is obsolete and will be removed
#define MLF_PRIMARY_FRIENDLY		(1<<2)		// primary object in this entry is friendly
#define MLF_PRIMARY_HOSTILE		(1<<3)		// primary object in this entry is hostile
#define MLF_SECONDARY_FRIENDLY	(1<<4)		// secondary object is friendly
#define MLF_SECONDARY_HOSTILE		(1<<5)		// secondary object is hostile
#define MLF_HIDDEN					(1<<6)		// entry doesn't show up in displayed log.

typedef struct {
	int		type;									// one of the log #defines in MissionLog.h
	int		flags;								// flags used for status of this log entry
	fix		timestamp;							// time in fixed seconds when entry was made from beginning of mission
	char		pname[NAME_LENGTH];				// name of primary object of this action
	char		sname[NAME_LENGTH];				// name of secondary object of this action
	int		index;								// a generic entry which can contain things like wave # (for wing arrivals), goal #, etc
} log_entry;

extern log_entry log_entries[];
extern int last_entry;
extern int Num_log_lines;

// function prototypes

// to be called before each mission starts
extern void mission_log_init();

// adds an entry to the mission log.  The name is a string identifier that is the object
// of the event.  The multiplayer version of this takes the actual entry number to modify.
extern void mission_log_add_entry(int type, char *pname, char *sname, int index = -1 );
extern void mission_log_add_entry_multi( int type, char *pname, char *sname, int index, fix timestamp, int flags );

// function to determine if event happened and what time it happened
extern int mission_log_get_time( int type, char *name, char *sname, fix *time);

// function to determine if event happend count times and return time that the count event
// happened
extern int mission_log_get_time_indexed( int type, char *name, char *sname, int count, fix *time);

// function to show all message log entries during or after mission
// (code stolen liberally from Alan!)
extern void mission_log_scrollback(float frametime);

void message_log_init_scrollback(int pw);
void message_log_shutdown_scrollback();
void mission_log_scrollback(int scroll_offset, int list_x, int list_y, int list_w, int list_h);

#endif
