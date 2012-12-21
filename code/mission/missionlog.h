/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MISSIONLOG_H
#define _MISSIONLOG_H

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"

// defined for different mission log entries

#define LOG_SHIP_DESTROYED					1
#define LOG_WING_DESTROYED					2
#define LOG_SHIP_ARRIVED					3
#define LOG_WING_ARRIVED					4
#define LOG_SHIP_DEPARTED					5
#define LOG_WING_DEPARTED					6
#define LOG_SHIP_DOCKED						7
#define LOG_SHIP_SUBSYS_DESTROYED			8
#define LOG_SHIP_UNDOCKED					9
#define LOG_SHIP_DISABLED					10
#define LOG_SHIP_DISARMED					11
#define LOG_PLAYER_CALLED_FOR_REARM			12
#define LOG_PLAYER_CALLED_FOR_REINFORCEMENT	13
#define LOG_GOAL_SATISFIED					14
#define LOG_GOAL_FAILED						15
#define LOG_PLAYER_ABORTED_REARM			16
#define LOG_WAYPOINTS_DONE					17
#define LOG_CARGO_REVEALED					18
#define LOG_CAP_SUBSYS_CARGO_REVEALED		19
#define LOG_SELF_DESTRUCTED					20

// structure definition for log entries

#define MLF_ESSENTIAL						(1 << 0)	// this entry is essential for goal checking code
#define MLF_OBSOLETE						(1 << 1)	// this entry is obsolete and will be removed
#define MLF_HIDDEN							(1 << 2)	// entry doesn't show up in displayed log.

typedef struct {
	int		type;										// one of the log #defines in MissionLog.h
	int		flags;										// flags used for status of this log entry
	fix		timestamp;									// time in fixed seconds when entry was made from beginning of mission
	char	pname[NAME_LENGTH];							// name of primary object of this action
	char	sname[NAME_LENGTH];							// name of secondary object of this action
	int		index;										// a generic entry which can contain things like wave # (for wing arrivals), goal #, etc

	// Goober5000
	int primary_team;
	int secondary_team;
} log_entry;

extern log_entry log_entries[];
extern int last_entry;
extern int Num_log_lines;

// function prototypes

// to be called before each mission starts
extern void mission_log_init();

// adds an entry to the mission log.  The name is a string identifier that is the object
// of the event.  The multiplayer version of this takes the actual entry number to modify.
extern void mission_log_add_entry(int type, char *pname, char *sname, int index = -1);
extern void mission_log_add_entry_multi( int type, char *pname, char *sname, int index, fix timestamp, int flags);

// function to determine if event happened and what time it happened
extern int mission_log_get_time( int type, char *name, char *sname, fix *time);

// function to determine if event happend count times and return time that the count event
// happened
extern int mission_log_get_time_indexed( int type, char *name, char *sname, int count, fix *time);

// get the number of times an event happened
extern int mission_log_get_count( int type, char *pname, char *sname ); 

// function to show all message log entries during or after mission
// (code stolen liberally from Alan!)
extern void mission_log_scrollback(float frametime);

void message_log_init_scrollback(int pw);
void message_log_shutdown_scrollback();
void mission_log_scrollback(int scroll_offset, int list_x, int list_y, int list_w, int list_h);

#endif
