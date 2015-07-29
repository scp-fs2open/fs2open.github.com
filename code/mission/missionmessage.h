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

#include "globalincs/globals.h"		// include so that we can gets defs for lengths of tokens
#include "anim/packunpack.h"
#include "graphics/generic.h"

class ship;

//Bumped to 500 from 300 06/15/2004 -C

// keep seperate lists of AVI's and wav files.  I suspect that many messages will have
// duplicate avi's for different messages.  Seperate list for wave files since some messages
// might not have wave file information.

typedef struct message_extra {
	char				name[MAX_FILENAME_LEN];
	int				num;
	generic_anim		anim_data;
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

// defines for message id's used in FreeSpace code.  Callers to message_send_to_player() should
// probably use these defines.

typedef struct builtin_message {
	char			*name;
	int				occurrence_chance;
	int				max_count;
	int				min_delay;
} builtin_message;

extern SCP_vector<SCP_string> Builtin_moods;
extern int Current_mission_mood;

// this number in this define should match the number of elements in the next array
#define MAX_BUILTIN_MESSAGE_TYPES	45

extern builtin_message Builtin_messages[];

#define MESSAGE_ARRIVE_ENEMY		0
#define MESSAGE_ATTACK_TARGET		1
#define MESSAGE_BETA_ARRIVED		2
#define MESSAGE_CHECK_6				3
#define MESSAGE_ENGAGE				4
#define MESSAGE_GAMMA_ARRIVED		5
#define MESSAGE_HELP				6
#define MESSAGE_PRAISE				7
#define MESSAGE_REINFORCEMENTS		8
#define MESSAGE_IGNORE				9
#define MESSAGE_NOSIR				10
#define MESSAGE_OOPS				11
#define MESSAGE_PERMISSION			12
#define MESSAGE_STRAY				13
#define MESSAGE_WARP_OUT			14
#define MESSAGE_YESSIR				15
#define MESSAGE_REARM_ON_WAY		16
#define MESSAGE_ON_WAY				17
#define MESSAGE_REARM_WARP			18
#define MESSAGE_NO_TARGET			19
#define MESSAGE_DOCK_YES			20
#define MESSAGE_REPAIR_DONE			21
#define MESSAGE_REPAIR_ABORTED		22
#define MESSAGE_HAMMER_SWINE		23
#define MESSAGE_REARM_REQUEST		24		// wingman messages player when he calls a support ship
#define MESSAGE_DISABLE_TARGET		25
#define MESSAGE_DISARM_TARGET		26
#define MESSAGE_PLAYER_DIED			27		// message sent when player starts death roll
#define MESSAGE_WINGMAN_SCREAM		28
#define MESSAGE_SUPPORT_KILLED		29
#define MESSAGE_ALL_CLEAR			30
#define MESSAGE_ALL_ALONE			31		// message sent when player is last ship left and primary objectives still exist
#define MESSAGE_REPAIR_REQUEST		32
#define MESSAGE_DELTA_ARRIVED		33
#define MESSAGE_EPSILON_ARRIVED		34
#define MESSAGE_INSTRUCTOR_HIT		35
#define MESSAGE_INSTRUCTOR_ATTACK	36
#define MESSAGE_STRAY_WARNING		37
#define MESSAGE_STRAY_WARNING_FINAL		38
#define MESSAGE_AWACS_75			39
#define MESSAGE_AWACS_25			40
#define MESSAGE_PRAISE_SELF			41
#define MESSAGE_HIGH_PRAISE			42
#define MESSAGE_REARM_PRIMARIES		43
#define MESSAGE_PRIMARIES_LOW		44

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
	int wave;					// handle of wave currently playing
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

// variable, etc for persona information
#define MAX_PERSONA_TYPES		4

// flags for personas.  the type flags must be sequential starting from 0, and must match
// the persona_type_names defined in missionmessage.cpp
#define PERSONA_FLAG_WINGMAN	(1<<0)
#define PERSONA_FLAG_SUPPORT	(1<<1)
#define PERSONA_FLAG_LARGE		(1<<2)		// for large ships
#define PERSONA_FLAG_COMMAND	(1<<3)		// for terran command
// be sure that MAX_PERSONA_TYPES is always 1 greater than the last type bitfield above!!!

#define PERSONA_FLAG_VASUDAN	(1<<30)
#define PERSONA_FLAG_USED		(1<<31)

typedef struct persona_s {
	char	name[NAME_LENGTH];
	int	flags;
	int species;
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
int	message_anim_is_playing();
void	message_kill_all( int kill_all );

void	message_queue_message( int message_num, int priority, int timing, char *who_from, int source, int group, int delay, int builtin_type=-1 );

// functions which send messages to player -- called externally
void	message_send_unique_to_player( char *id, void *data, int source, int priority, int group, int delay);
void	message_send_builtin_to_player( int type, ship *shipp, int priority, int timing, int group, int delay, int multi_target, int multi_team_filter );

// functions to deal with personas
int	message_persona_name_lookup( char *name );

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
bool add_message(char *name, char *message, int persona_index, int multi_team);
bool change_message(char *name, char *message, int persona_index, int multi_team);

#endif
