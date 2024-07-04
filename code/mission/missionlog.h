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

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"

// defined for different mission log entries

enum LogType {
	LOG_SHIP_DESTROYED                  = 1,
	LOG_WING_DESTROYED                  = 2,
	LOG_SHIP_ARRIVED                    = 3,
	LOG_WING_ARRIVED                    = 4,
	LOG_SHIP_DEPARTED                   = 5,
	LOG_WING_DEPARTED                   = 6,
	LOG_SHIP_DOCKED                     = 7,
	LOG_SHIP_SUBSYS_DESTROYED           = 8,
	LOG_SHIP_UNDOCKED                   = 9,
	LOG_SHIP_DISABLED                   = 10,
	LOG_SHIP_DISARMED                   = 11,
	LOG_PLAYER_CALLED_FOR_REARM         = 12,
	LOG_PLAYER_CALLED_FOR_REINFORCEMENT = 13,
	LOG_GOAL_SATISFIED                  = 14,
	LOG_GOAL_FAILED                     = 15,
	LOG_PLAYER_ABORTED_REARM            = 16,
	LOG_WAYPOINTS_DONE                  = 17,
	LOG_CARGO_REVEALED                  = 18,
	LOG_CAP_SUBSYS_CARGO_REVEALED       = 19,
	LOG_SELF_DESTRUCTED                 = 20,
};

// structure definition for log entries

#define MLF_HIDDEN							(1 << 0)	// entry doesn't show up in displayed log.

// defines for log flags
#define LOG_FLAG_GOAL_FAILED (1 << 0)
#define LOG_FLAG_GOAL_TRUE (1 << 1)

// defines for log colors
#define LOG_COLOR_NORMAL 0
#define LOG_COLOR_BRIGHT 1
#define LOG_COLOR_OTHER 2

struct log_entry {
	LogType type;            // one of the log #defines in MissionLog.h
	int flags;               // flags used for status of this log entry
	fix timestamp;           // time in fixed seconds when entry was made from beginning of mission
	int timer_padding;		 // the mission timer padding, in seconds, when the entry was created
	char pname[NAME_LENGTH]; // name of primary object of this action
	char sname[NAME_LENGTH]; // name of secondary object of this action
	int index;               // a generic entry which can contain things like wave # (for wing arrivals), goal #, etc

	// Goober5000
	int primary_team;
	int secondary_team;

	SCP_string pname_display;
	SCP_string sname_display;
};

struct log_text_seg {
	SCP_vm_unique_ptr<char> text; // the text
	int color;                    // color text should be displayed in
	int x;                        // x offset to display text at
	int flags;                    // used to possibly print special characters when displaying the log
};

struct log_line_complete {
	fix timestamp;
	int timer_padding;
	log_text_seg objective;
	SCP_vector<log_text_seg> segments;
};

// function prototypes

// to be called before each mission starts
extern void mission_log_init();

// adds an entry to the mission log.  The name is a string identifier that is the object
// of the event.  The multiplayer version of this takes the actual entry number to modify.
extern void mission_log_add_entry(LogType type, const char *pname, const char *sname, int index = -1, int flags = 0);
extern void mission_log_add_entry_multi(LogType type, const char *pname, const char *sname, int index, fix timestamp,
                                        int flags);

// function to determine if event happened and what time it happened
extern int mission_log_get_time(LogType type, const char *name, const char *sname, fix *time);

// function to determine if event happend count times and return time that the count event
// happened
extern int mission_log_get_time_indexed(LogType type, const char *name, const char *sname, int count, fix *time);

// get the number of times an event happened
extern int mission_log_get_count(LogType type, const char *pname, const char *sname);

// get the team for a log item
extern int mission_log_color_get_team(int msg_color);

// get the actual color for a line item
extern const color *log_line_get_color(int tag);

extern void mission_log_init_scrollback(int pw, bool split_string = true);
extern void mission_log_shutdown_scrollback();
extern void mission_log_scrollback(int line_offset, int list_x, int list_y, int list_w, int list_h);

extern int mission_log_scrollback_num_lines();
extern const log_line_complete* mission_log_scrollback_get_line(int index);

#endif
