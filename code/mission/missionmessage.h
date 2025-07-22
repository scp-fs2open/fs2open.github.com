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
#define DEFAULT_HASHCOMMAND		"#" DEFAULT_COMMAND

extern SCP_vector<SCP_string> Builtin_moods;
extern int Current_mission_mood;
extern float Command_announces_enemy_arrival_chance;
extern bool Always_loop_head_anis;
extern bool Use_newer_head_ani_suffix;

// Builtin messages

typedef struct builtin_message {
	const char* name;
	int         occurrence_chance;
	int         max_count;
	int         min_delay;
	int         priority;
	int         timing;
	int         fallback;
	bool        used_strdup;

	builtin_message(const char* _name, int _occurrence_chance, int _max_count, int _min_delay, int _priority, int _timing, int _fallback, bool _used_strdup);
	// since we need a destructor, we need the other four special member functions as well
	~builtin_message();
	builtin_message(const builtin_message& other);
	builtin_message& operator=(const builtin_message& other);
	builtin_message(builtin_message&& other) noexcept = default;
	builtin_message& operator=(builtin_message&& other) noexcept = default;
} builtin_message;

// If these are changed or updated be sure to update the map in scripting/api/libs/mission.cpp and the connected lua enumerations!
#define BUILTIN_MESSAGE_TYPES                                                                    \
/* Orders */                                                                                     \
X(ATTACK_TARGET,       "Attack Target",        100, -1,  0,     NORMAL, ANYTIME, YESSIR),        \
X(DISABLE_TARGET,      "Disable Target",       100, -1,  0,     NORMAL, ANYTIME, YESSIR),        \
X(DISARM_TARGET,       "Disarm Target",        100, -1,  0,     NORMAL, ANYTIME, YESSIR),        \
X(ATTACK_SUBSYSTEM,    "Attack Subsystem",     100, -1,  0,     NORMAL, ANYTIME, ATTACK_TARGET), \
X(PROTECT_TARGET,      "Protect Target",       100, -1,  0,     NORMAL, ANYTIME, YESSIR),        \
X(FORM_ON_MY_WING,     "Form On My Wing",      100, -1,  0,     NORMAL, ANYTIME, YESSIR),        \
X(COVER_ME,            "Cover Me",             100, -1,  0,     NORMAL, ANYTIME, YESSIR),        \
X(IGNORE,              "Ignore Target",        100, -1,  0,     NORMAL, ANYTIME, YESSIR),        \
X(ENGAGE,              "Engage",               100, -1,  0,     NORMAL, ANYTIME, YESSIR),        \
X(WARP_OUT,            "Depart",               100, -1,  0,     NORMAL, ANYTIME, YESSIR),        \
X(DOCK_YES,            "Docking Start",        100, -1,  0,     NORMAL, ANYTIME, YESSIR),        \
X(YESSIR,              "Yes",                  100, -1,  0,     NORMAL, ANYTIME, NONE),          \
X(NOSIR,               "No",                   100, -1,  0,     NORMAL, ANYTIME, NONE),          \
X(NO_TARGET,           "No Target",            100, -1,  0,     NORMAL, ANYTIME, NOSIR),         \
                                                                                                 \
/* Player status */                                                                              \
X(CHECK_6,             "Check 6",              100,  2,  6000,  HIGH, IMMEDIATE, NONE),          \
X(PLAYER_DIED,         "Player Dead",           25, -1,  0,     HIGH, IMMEDIATE, NONE),          \
X(PRAISE,              "Praise",                50, 10,  60000, HIGH, SOON, NONE),               \
X(HIGH_PRAISE,         "High Praise",           50, -1,  0,     HIGH, SOON, PRAISE),             \
                                                                                                 \
/* Wingmate status */                                                                            \
X(BACKUP,              "Backup",               100, -1,   5000, LOW, SOON, NONE),                \
X(HELP,                "Help",                 100, 10,  60000, HIGH, IMMEDIATE, NONE),          \
X(WINGMAN_SCREAM,      "Death",                 50, 10,  60000, HIGH, IMMEDIATE, NONE),          \
X(PRAISE_SELF,         "Praise Self",           10,  4,  60000, HIGH, SOON, NONE),               \
X(REARM_REQUEST,       "Rearm",                 50, -1,  0,     NORMAL, SOON, NONE),             \
X(REPAIR_REQUEST,      "Repair",                50, -1,  0,     NORMAL, SOON, NONE),             \
X(PRIMARIES_LOW,       "Primaries Low",        100, -1,  0,     NORMAL, SOON, NONE),             \
X(REARM_PRIMARIES,     "Rearm Primaries",       50, -1,  0,     NORMAL, SOON, REARM_REQUEST),    \
                                                                                                 \
/* Support status */                                                                             \
X(REARM_WARP,          "Rearm Warping In",     100, -1,  0,     NORMAL, SOON, NONE),             \
X(ON_WAY,              "On Way",               100, -1,  0,     NORMAL, SOON, NONE),             \
X(ALREADY_ON_WAY,      "Rearm On Way",         100, -1,  0,     NORMAL, SOON, ON_WAY),           \
X(REPAIR_DONE,         "Repair Done",          100, -1,  0,     LOW, SOON, NONE),                \
X(REPAIR_ABORTED,      "Repair Aborted",       100, -1,  0,     NORMAL, SOON, NONE),             \
X(SUPPORT_KILLED,      "Support Killed",       100, -1,  0,     HIGH, SOON, NONE),               \
                                                                                                 \
/* Global status */                                                                              \
X(ALL_ALONE,           "All Alone",             50, -1,  0,     HIGH, ANYTIME, NONE),            \
X(ARRIVE_ENEMY,        "Arrive Enemy",         100, -1,  30000, LOW, SOON, NONE),                \
X(OOPS,                "Oops 1",               100, -1,  0,     HIGH, ANYTIME, NONE),            \
X(HAMMER_SWINE,        "Traitor",              100, -1,  0,     HIGH, ANYTIME, NONE),            \
                                                                                                 \
/* Misc */                                                                                       \
X(AWACS_75,            "AWACS at 75",          100, -1,  0,     HIGH, IMMEDIATE, NONE),          \
X(AWACS_25,            "AWACS at 25",          100, -1,  0,     HIGH, IMMEDIATE, NONE),          \
X(STRAY_WARNING,       "Stray Warning",        100, -1,  0,     HIGH, SOON, NONE),               \
X(STRAY_WARNING_FINAL, "Stray Warning Final",  100, -1,  0,     HIGH, IMMEDIATE, NONE),          \
X(INSTRUCTOR_HIT,      "Instructor Hit",       100, -1,  0,     HIGH, IMMEDIATE, NONE),          \
X(INSTRUCTOR_ATTACK,   "Instructor Attack",    100, -1,  0,     HIGH, IMMEDIATE, NONE),          \
                                                                                                 \
/* Unused */                                                                                     \
X(ALL_CLEAR,           "All Clear",            100, -1,  0,     LOW, SOON, NONE),                \
X(PERMISSION,          "Permission",           100, -1,  0,     LOW, SOON, NONE),                \
X(STRAY,               "Stray",                100, -1,  0,     LOW, SOON, NONE)

enum {
  #define X(NAME, ...) MESSAGE_ ## NAME
	MESSAGE_NONE = -1, BUILTIN_MESSAGE_TYPES,
	#undef X
	Num_Message_Types
};

extern SCP_vector<builtin_message> Builtin_messages;

int get_builtin_message_type(const char* name);

typedef struct MessageFilter {
	SCP_vector<SCP_string> ship_name;
	SCP_vector<SCP_string> callsign;
	SCP_vector<SCP_string> class_name;
	SCP_vector<SCP_string> wing_name;
	int                    species_bitfield;
	int                    type_bitfield;
	int                    team_bitfield;
} MessageFilter;

typedef struct MissionMessage {
	char    name[NAME_LENGTH];					// used to identify this message
	char    message[MESSAGE_LENGTH];			// actual message
	int     persona_index;							// which persona says this message
	int     multi_team;								// multiplayer team filter (important for TvT only)
	int     mood;
	SCP_string note;
	SCP_vector<int> excluded_moods;

	MessageFilter sender_filter;
	MessageFilter subject_filter;
	MessageFilter outer_filter;
	int outer_filter_radius;
	int boost_level;

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

struct Persona {
	char name[NAME_LENGTH];
	int flags;
	int species_bitfield;
};

extern SCP_vector<Persona> Personas;

extern int Default_command_persona;
extern int Praise_self_percentage;

enum class MessageFormat { FS1_MISSION, FS2_MISSION, TABLED };

void	message_parse(MessageFormat format);
void	persona_parse();

void	messages_init();
void	message_types_init();
void	message_mission_shutdown();
void	message_queue_process();
int	message_is_playing();
void	message_maybe_distort();
void	message_kill_all( bool kill_all );

void	message_pause_all();
void	message_resume_all();

void	message_queue_message(int message_num, int priority, int timing, const char *who_from, int source, int group, int delay, int builtin_type, int event_num_to_cancel);

// functions which send messages to player -- called externally
void	message_send_unique(const char *id, const void *data, int source, int priority, int group, int delay, int event_num_to_cancel = -1);

bool	message_send_builtin(int type, ship* sender, ship* subject, int multi_target = -1, int multi_team_filter = -1);

// functions to deal with personas
int message_persona_name_lookup(const char* name);

// preload mission messages (this is called by the level paging code when running with low memory)
void message_pagein_mission_messages();

// given a message id#, should it be filtered for me?
int message_filter_multi(int id);

// Goober5000
bool message_filename_is_generic(const char *filename);

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
