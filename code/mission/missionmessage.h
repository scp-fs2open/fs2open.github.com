/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MISSIONMESSAGE_H
#define _MISSIONMESSAGE_H

#include "anim/packunpack.h"
#include "globalincs/globals.h"		// include so that we can gets defs for lengths of tokens
#include "graphics/generic.h"
#include "sound/sound.h"

class ship;

//Bumped to 500 from 300 06/15/2004 -C

// keep seperate lists of AVI's and wav files.  I suspect that many messages will have
// duplicate avi's for different messages.  Seperate list for wave files since some messages
// might not have wave file information.

typedef struct message_extra {
	char				name[MAX_FILENAME_LEN];
	sound_load_id num;
	generic_anim		anim_data;
	bool				exists;
} message_extra;

extern SCP_vector<message_extra> Message_avis;
extern SCP_vector<message_extra> Message_waves;

// defines for message priorities
#define MESSAGE_PRIORITY_LOW		1
#define MESSAGE_PRIORITY_NORMAL	2
#define MESSAGE_PRIORITY_HIGH		3

// defines for how quickly we should send a message
#define MESSAGE_TIME_IMMEDIATE	1
#define MESSAGE_TIME_SOON			2
#define MESSAGE_TIME_ANYTIME		3

// sources for messages
#define MESSAGE_SOURCE_SHIP		1
#define MESSAGE_SOURCE_WINGMAN	2
#define MESSAGE_SOURCE_COMMAND	3
#define MESSAGE_SOURCE_SPECIAL	4
#define MESSAGE_SOURCE_NONE		5

// define used for sender of a message when you want it to be Terran Command
#define DEFAULT_COMMAND			"Command"

extern SCP_vector<SCP_string> Builtin_moods;
extern int Current_mission_mood;

// Builtin messages

typedef struct builtin_message {
	const char* name;
	int         occurrence_chance;
	int         max_count;
	int         min_delay;
	int         priority;
	int         timing;
} builtin_message;

#define BUILTIN_MESSAGE_TYPES                                                     \
/* Orders */                                                                      \
X(ATTACK_TARGET,       "Attack Target",        100, -1,  0,     NORMAL, ANYTIME), \
X(DISABLE_TARGET,      "Disable Target",       100, -1,  0,     NORMAL, ANYTIME), \
X(DISARM_TARGET,       "Disarm Target",        100, -1,  0,     NORMAL, ANYTIME), \
X(IGNORE,              "Ignore Target",        100, -1,  0,     NORMAL, ANYTIME), \
X(ENGAGE,              "Engage",               100, -1,  0,     NORMAL, ANYTIME), \
X(WARP_OUT,            "Depart",               100, -1,  0,     NORMAL, ANYTIME), \
X(DOCK_YES,            "Docking Start",        100, -1,  0,     NORMAL, ANYTIME), \
X(YESSIR,              "Yes",                  100, -1,  0,     NORMAL, ANYTIME), \
X(NOSIR,               "No",                   100, -1,  0,     NORMAL, ANYTIME), \
X(NO_TARGET,           "No Target",            100, -1,  0,     NORMAL, ANYTIME), \
                                                                                  \
/* Friendly arrival */                                                            \
X(BETA_ARRIVED,        "Beta Arrived",         100, -1,  0,     LOW, SOON),       \
X(GAMMA_ARRIVED,       "Gamma Arrived",        100, -1,  0,     LOW, SOON),       \
X(DELTA_ARRIVED,       "Delta Arrived",        100, -1,  0,     LOW, SOON),       \
X(EPSILON_ARRIVED,     "Epsilon Arrived",      100, -1,  0,     LOW, SOON),       \
X(REINFORCEMENTS,      "Backup",               100, -1,  0,     LOW, SOON),       \
                                                                                  \
/* Player status */                                                               \
X(CHECK_6,             "Check 6",              100,  2,  6000,  HIGH, IMMEDIATE), \
X(PLAYER_DIED,         "Player Dead",          100, -1,  0,     HIGH, IMMEDIATE), \
X(PRAISE,              "Praise",               100, 10,  60000, HIGH, SOON),      \
X(HIGH_PRAISE,         "High Praise",          100, -1,  0,     HIGH, SOON),      \
                                                                                  \
/* Wingmate status */                                                             \
X(HELP,                "Help",                 100, 10,  60000, HIGH, IMMEDIATE), \
X(WINGMAN_SCREAM,      "Death",                 50, 10,  60000, HIGH, IMMEDIATE), \
X(PRAISE_SELF,         "Praise Self",           10,  4,  60000, HIGH, SOON),      \
X(REARM_REQUEST,       "Rearm",                100, -1,  0,     NORMAL, SOON),    \
X(REPAIR_REQUEST,      "Repair",               100, -1,  0,     NORMAL, SOON),    \
X(PRIMARIES_LOW,       "Primaries Low",        100, -1,  0,     NORMAL, SOON),    \
X(REARM_PRIMARIES,     "Rearm Primaries",      100, -1,  0,     NORMAL, SOON),    \
                                                                                  \
/* Support status */                                                              \
X(REARM_WARP,          "Rearm Warping In",     100, -1,  0,     NORMAL, SOON),    \
X(ON_WAY,              "On Way",               100, -1,  0,     NORMAL, SOON),    \
X(REARM_ON_WAY,        "Rearm On Way",         100, -1,  0,     NORMAL, SOON),    \
X(REPAIR_DONE,         "Repair Done",          100, -1,  0,     LOW, SOON),       \
X(REPAIR_ABORTED,      "Repair Aborted",       100, -1,  0,     NORMAL, SOON),    \
X(SUPPORT_KILLED,      "Support Killed",       100, -1,  0,     HIGH, SOON),      \
                                                                                  \
/* Global status */                                                               \
X(ALL_ALONE,           "All Alone",            100, -1,  0,     HIGH, ANYTIME),   \
X(ARRIVE_ENEMY,        "Arrive Enemy",         100, -1,  0,     LOW, SOON),       \
X(OOPS,                "Oops 1",               100, -1,  0,     HIGH, ANYTIME),   \
X(HAMMER_SWINE,        "Traitor",              100, -1,  0,     HIGH, ANYTIME),   \
                                                                                  \
/* Misc */                                                                        \
X(AWACS_75,            "AWACS at 75",          100, -1,  0,     HIGH, IMMEDIATE), \
X(AWACS_25,            "AWACS at 25",          100, -1,  0,     HIGH, IMMEDIATE), \
X(STRAY_WARNING,       "Stray Warning",        100, -1,  0,     HIGH, SOON),      \
X(STRAY_WARNING_FINAL, "Stray Warning Final",  100, -1,  0,     HIGH, IMMEDIATE), \
X(INSTRUCTOR_HIT,      "Instructor Hit",       100, -1,  0,     HIGH, IMMEDIATE), \
X(INSTRUCTOR_ATTACK,   "Instructor Attack",    100, -1,  0,     HIGH, IMMEDIATE), \
                                                                                  \
/* Unused */                                                                      \
X(ALL_CLEAR,           "All Clear",            100, -1,  0,     LOW, SOON),       \
X(PERMISSION,          "Permission",           100, -1,  0,     LOW, SOON),       \
X(STRAY,               "Stray",                100, -1,  0,     LOW, SOON)

enum {
  #define X(NAME, ...) MESSAGE_ ## NAME
	NO_MESSAGE = -1, BUILTIN_MESSAGE_TYPES, MAX_BUILTIN_MESSAGE_TYPES
	#undef X
};

extern builtin_message Builtin_messages[MAX_BUILTIN_MESSAGE_TYPES];

typedef struct MissionMessage {
	char	name[NAME_LENGTH];					// used to identify this message
	char	message[MESSAGE_LENGTH];			// actual message
	int	persona_index;							// which persona says this message
	int	multi_team;								// multiplayer team filter (important for TvT only)
	int				mood;
	SCP_vector<int> excluded_moods;

	// unions for avi/wave information.  Because of issues with Fred, we are using
	// the union to specify either the index into the avi or wave arrays above,
	// or refernce the name directly.  The currently plan is to only have Fred reference
	// the name field!!!
	union {
		int	index;								// index of avi file to play
		char	*name;
	} avi_info;

	union {
		int	index;
		char	*name;
	} wave_info;

} MMessage;

extern SCP_vector<MMessage> Messages;

typedef struct pmessage {
	//anim_instance *anim;		// handle of anim currently playing
	generic_anim *anim_data;			// animation data to be used by the talking head HUD gauge handler
	int start_frame;			// the start frame needed to play the animation
	bool play_anim;			// used to tell HUD gauges if they should be playing or not
	sound_handle wave;      // handle of wave currently playing
	int id;						// id of message currently playing
	int priority;				// priority of message currently playing
	int shipnum;				// shipnum of ship sending this message,  -1 if from Terran command
	int builtin_type;			// if a builtin message, type of the message
} pmessage;

extern pmessage Playing_messages[2];

extern int Num_messages_playing;
extern int Num_messages;
extern int Num_builtin_messages;				// from messages.tbl -- index of message location to load mission specific messages into
extern int Message_shipnum;					// used to display info on hud when message is sent

extern SCP_vector<SCP_string> Generic_message_filenames;

// variable, etc for persona information
#define MAX_PERSONA_TYPES		4

// flags for personas.  the type flags must be sequential starting from 0, and must match
// the persona_type_names defined in missionmessage.cpp
#define PERSONA_FLAG_WINGMAN	(1<<0)
#define PERSONA_FLAG_SUPPORT	(1<<1)
#define PERSONA_FLAG_LARGE		(1<<2)		// for large ships
#define PERSONA_FLAG_COMMAND	(1<<3)		// for terran command
// be sure that MAX_PERSONA_TYPES is always 1 greater than the last type bitfield above!!!
// for non-type flags, add them in reverse order from the end

#define PERSONA_FLAG_NO_AUTOMATIC_ASSIGNMENT        (1<<29)     // when you don't want characters showing up unexpectedly
#define PERSONA_FLAG_SUBSTITUTE_MISSING_MESSAGES    (1<<30)
#define PERSONA_FLAG_USED                           (1<<31)

typedef struct persona_s {
	char	name[NAME_LENGTH];
	int	flags;
	int species_bitfield;
} Persona;

extern Persona *Personas;
extern int Num_personas;
extern int Default_command_persona;
extern int Praise_self_percentage;

// function to parse a message from either messages.tbl or the mission file.  Both files have the
// exact same format, so this function just gets reused in both instances.
void	message_parse(bool importing_from_fsm = false);
void	persona_parse();

void	messages_init();
void	message_mission_shutdown();
void	message_mission_close();
void	message_queue_process();
int	message_is_playing();
void	message_maybe_distort();
void	message_kill_all( int kill_all );

void	message_queue_message(int message_num, int priority, int timing, const char *who_from, int source, int group, int delay, int builtin_type, int event_num_to_cancel);

// functions which send messages to player -- called externally
void	message_send_unique_to_player(const char *id, const void *data, int source, int priority, int group, int delay, int event_num_to_cancel = -1);
void	message_send_builtin_to_player(int type, ship *shipp, int group, int delay, int multi_target, int multi_team_filter);

// functions to deal with personas
int message_persona_name_lookup(const char* name);

// preload mission messages (this is called by the level paging code when running with low memory)
void message_pagein_mission_messages();

// given a message id#, should it be filtered for me?
int message_filter_multi(int id);

// Goober5000
bool message_filename_is_generic(char *filename);

// m!m
void message_load_wave(int index, const char *filename);

// Kazan
// Use these functions with caution as everything else uses indexes... so make sure if you're going to
// use these there will be no remove_messages called before your message is displayed.

// these two are probably safe
// if change_message fails to find the message it'll fall through to add_message
bool add_message(const char* name, const char* message, int persona_index, int multi_team);
bool change_message(const char* name, const char* message, int persona_index, int multi_team);

#endif
