/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionMessage.h $
 * $Revision: 2.14 $
 * $Date: 2007-01-07 00:01:28 $
 * $Author: Goober5000 $
 *
 * Header file for mission messaging
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.13  2006/09/30 21:58:09  Goober5000
 * more flexible checking of generic messages
 *
 * Revision 2.12  2006/04/20 06:32:07  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.11  2005/08/25 22:40:03  taylor
 * basic cleaning, removing old/useless code, sanity stuff, etc:
 *  - very minor performance boost from not doing stupid things :)
 *  - minor change to 3d shockwave sizing to better approximate 2d effect movements
 *  - for shields, Gobal_tris was only holding half as many as the game can/will use, buffer is now set to full size to avoid possible rendering issues
 *  - removed extra tcache_set on OGL spec map code, not sure how that slipped in
 *
 * Revision 2.10  2005/07/13 03:25:59  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.9  2005/07/13 00:44:22  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.8  2004/12/10 17:21:00  taylor
 * dymanic allocation of Personas
 *
 * Revision 2.7  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.6  2004/06/15 21:05:30  wmcoolmon
 * Bumped MAX_MISSION_MESSAGES to 500
 *
 * Revision 2.5  2004/03/05 09:02:06  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.4  2004/02/12 22:29:55  phreak
 * fixed a bug where the inferno build would get lower limits for MAX_PERSONAS if INF_BUILD was defined
 *
 * Revision 2.3  2004/01/21 17:34:31  phreak
 * bumped MAX_PERSONAS to 25 if INF_BUILD is defined.
 *
 * Revision 2.2  2003/10/16 16:38:16  Kazan
 * couple more types in species_defs.cpp, also finished up "Da Species Upgrade"
 *
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 8     9/06/99 10:13a Jasons
 * Bump up MAX_PERSONAS from 10 to 13
 * 
 * 7     8/26/99 8:51p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 6     8/19/99 10:12a Alanl
 * preload mission-specific messages on machines greater than 48MB
 * 
 * 5     7/31/99 2:30p Dave
 * Added nifty mission message debug viewing keys.
 * 
 * 4     7/06/99 10:41a Andsager
 * Add AWACS need help messages
 * 
 * 3     6/16/99 10:20a Dave
 * Added send-message-list sexpression.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 46    4/25/98 11:49p Lawrance
 * Add Terran Command stray messages
 * 
 * 45    4/15/98 4:18p Allender
 * Make Terran Command who_from be just Command
 * 
 * 44    4/08/98 3:45p Allender
 * mission message overhaul.  Make message from any wingman mean any
 * wingman with that persona.  Terran command wave and ani's for dead
 * ships now play correctly.
 * 
 * 43    4/07/98 12:04a Mike
 * New system for instructor chastising player if he fires at instructor.
 * 
 * 42    4/02/98 10:07a Allender
 * wing arrival message for delta and epsilon wings
 * 
 * 41    4/01/98 10:47p Lawrance
 * Supporting builtin messages for rearm and repair requests
 * 
 * 40    2/20/98 8:33p Lawrance
 * Add the 'All Alone' message
 * 
 * 39    2/12/98 4:58p Lawrance
 * Add support for 'All Clear' radio message
 * 
 * 38    2/11/98 9:44p Allender
 * rearm repair code fixes.  hud support view shows abort status.  New
 * support ship killed message.  More network stats
 * 
 * 37    1/29/98 11:38a Allender
 * support for Vasudan personas
 * 
 * 36    1/28/98 11:38a Dan
 * AL: bump up MAX_MISSION_MESSAGES to 300 from 200.
 * 
 * 35    1/21/98 7:20p Lawrance
 * Make subsystem locking only work with line-of-sight, cleaned up locking
 * code, moved globals to player struct.
 * 
 * 34    1/18/98 9:51p Lawrance
 * Add support for 'Player Died' messages.
 * 
 * 33    1/14/98 9:49p Allender
 * removed 3'oclock and 9'oclock messages
 * 
 * 32    1/13/98 3:11p Allender
 * new messages for disable/disarm
 * 
 * 31    1/07/98 4:41p Allender
 * minor modification to special messages.  Fixed cargo_revealed problem
 * for multiplayer and problem with is-cargo-known sexpression
 * 
 * 30    12/01/97 5:10p Lawrance
 * 
 * 29    11/25/97 5:00p Allender
 * big changed in the messaging code -- implemented "timouts" -- windows
 * in which messages need to be played, or they are pitched
 * 
 * 28    11/21/97 4:09p Allender
 * added "Command" persona
 * 
 * 27    11/20/97 5:06p Allender
 * personas have types.  personas should now correctly get assigned to the
 * appropriate ship type.
 * 
 * 26    11/17/97 6:38p Lawrance
 * added message_anim_is_playing()
 * 
 * 25    11/17/97 4:57p Allender
 * persona support in FreeSpace
 * 
 * 24    11/14/97 3:52p Allender
 * removed '#' from TERRAN_COMMAND define
 * 
 * 23    11/11/97 4:57p Dave
 * Put in support for single vs. multiplayer pilots. Began work on
 * multiplayer campaign saving. Put in initial player select screen
 * 
 * 22    11/07/97 11:50a Allender
 * fixed ai_destroy_ship to know whether ship was actually destroyed or
 * just departed.  Fixed messages for making player hammer of light
 * 
 * 21    11/05/97 7:11p Hoffoss
 * Made changed to the hud message system.  Hud messages can now have
 * sources so they can be color coded.
 * 
 * 20    10/29/97 9:31p Allender
 * more rearm stuff.  Give player more apprioriate message when ship
 * already on way.  Don't count arriving ship as in the mission yet (for
 * find_support_ship() ).  
 * 
 * 19    10/23/97 9:40p Allender
 * more repari/rearm stuff.  Warped in support ships now go directly to
 * ship who they warped in for.  Player gets (hud only) message when
 * support ship warping in
 * 
 * 18    10/16/97 8:52p Duncan
 * Raised limit on messages to 100.
 * 
 * 17    10/13/97 7:40p Lawrance
 * have received messages get distorted if comm subsystem is severely
 * damaged
 * 
 * 16    10/10/97 5:02p Allender
 * started rudimentary work on personas
 * 
 * 15    10/09/97 4:44p Hoffoss
 * Dimmed training window glass and made it less transparent, added flags
 * to events, set he stage for detecting current events.
 * 
 * 14    10/02/97 9:53p Hoffoss
 * Added event evaluation analysis debug screen so we can determine the
 * state of events and their sexp trees to track down logic problems and
 * such.
 * 
 * 13    9/29/97 4:17p Duncan
 * Raised limit on number of waves and avis a single mission can utilize.
 * 
 * 12    9/22/97 4:55p Hoffoss
 * Added a training message window display thingy.
 * 
 * 11    8/04/97 11:17a Mike
 * Make rearm process abort if speed gets too high.
 * 
 * 10    5/15/97 11:46a Lawrance
 * play wingmen heads as anim's
 * 
 * 9     4/23/97 2:37p Lawrance
 * added message_is_playing() function
 * 
 * 8     4/08/97 10:55a Allender
 * draw purple brackets on ship sending a message
 * 
 * 7     3/13/97 10:57a Allender
 * new mission messages
 * 
 * 6     3/11/97 10:14a Allender
 * added unions for wave/avi files in messages to make things easier for
 * Fred.  FreeSpace will use the avi/wave indexing system.   Fred will use
 * only names
 * 
 * 5     3/10/97 4:16p Allender
 * new messaging system.  builtin5 fixed to support it.  made new sound
 * function to determine if sound is still playing.
 * 
 * 4     3/09/97 2:23p Allender
 * Major changes to player messaging system.  Added messages.tbl.  Made
 * all currently player messages go through new system.  Not done yet.
 * 
 * 3     1/08/97 11:45p Lawrance
 * moved some typedefs and defines to header file so visible for
 * save/restore
 * 
 * 2     12/17/96 1:02p Allender
 * rudimentary messaging working
 * 
 * 1     12/12/96 4:37p Allender
 *
 * $NoKeywords: $
 */

#ifndef _MISSIONMESSAGE_H
#define _MISSIONMESSAGE_H

#include "globalincs/globals.h"		// include so that we can gets defs for lengths of tokens
#include "anim/packunpack.h"

struct ship;

#define MAX_MISSION_MESSAGES	500
//Bumped to 500 from 300 06/15/2004 -C

// keep seperate lists of AVI's and wav files.  I suspect that many messages will have
// duplicate avi's for different messages.  Seperate list for wave files since some messages
// might not have wave file information.

typedef struct message_avi {
	char				name[MAX_FILENAME_LEN];
	int				num;
	anim				*anim_data;
} message_extra;

#define MAX_MESSAGE_AVIS		MAX_MISSION_MESSAGES
extern int		Num_message_avis;
extern message_extra	Message_avis[MAX_MESSAGE_AVIS];

#define MAX_MESSAGE_WAVES		MAX_MISSION_MESSAGES
extern int		Num_message_waves;
extern message_extra		Message_waves[MAX_MESSAGE_WAVES];

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

// define used for sender of a message when you want it to be Terran Command
#define DEFAULT_COMMAND			"Command"

// defines for message id's used in FreeSpace code.  Callers to message_send_to_player() should
// probably use these defines.

// this number in this define should match the number of elements in the next array
#define MAX_BUILTIN_MESSAGE_TYPES	41

extern char *Builtin_message_types[MAX_BUILTIN_MESSAGE_TYPES];

#define MESSAGE_ARRIVE_ENEMY		0
#define MESSAGE_ATTACK_TARGET		1
#define MESSAGE_BETA_ARRIVED		2
#define MESSAGE_CHECK_6				3
#define MESSAGE_ENGAGE				4
#define MESSAGE_GAMMA_ARRIVED		5
#define MESSAGE_HELP					6
#define MESSAGE_PRAISE				7
#define MESSAGE_REINFORCEMENTS	8
#define MESSAGE_IGNORE				9
#define MESSAGE_NOSIR				10
#define MESSAGE_OOPS					11
#define MESSAGE_PERMISSION			12
#define MESSAGE_STRAY				13
#define MESSAGE_WARP_OUT			14
#define MESSAGE_YESSIR				15
#define MESSAGE_REARM_ON_WAY		16
#define MESSAGE_ON_WAY				17
#define MESSAGE_REARM_WARP			18
#define MESSAGE_NO_TARGET			19
#define MESSAGE_DOCK_YES			20
#define MESSAGE_REPAIR_DONE		21
#define MESSAGE_REPAIR_ABORTED	22
#define MESSAGE_HAMMER_SWINE		23
#define MESSAGE_REARM_REQUEST		24		// wingman messages player when he calls a support ship
#define MESSAGE_DISABLE_TARGET	25
#define MESSAGE_DISARM_TARGET		26
#define MESSAGE_PLAYED_DIED		27		// message sent when player starts death roll
#define MESSAGE_WINGMAN_SCREAM	28
#define MESSAGE_SUPPORT_KILLED	29
#define MESSAGE_ALL_CLEAR			30
#define MESSAGE_ALL_ALONE			31		// message sent when player is last ship left and primary objectives still exist
#define MESSAGE_REPAIR_REQUEST	32
#define MESSAGE_DELTA_ARRIVED		33
#define MESSAGE_EPSILON_ARRIVED	34
#define MESSAGE_INSTRUCTOR_HIT	35
#define MESSAGE_INSTRUCTOR_ATTACK 36
#define MESSAGE_STRAY_WARNING				37
#define MESSAGE_STRAY_WARNING_FINAL		38
#define MESSAGE_AWACS_75			39
#define MESSAGE_AWACS_25			40

typedef struct MissionMessage {
	char	name[NAME_LENGTH];					// used to identify this message
	char	message[MESSAGE_LENGTH];			// actual message
	int	persona_index;							// which persona says this message
	int	multi_team;								// multiplayer team filter (important for TvT only)

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

extern MMessage Messages[MAX_MISSION_MESSAGES];

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

// function to parse a message from either messages.tbl or the mission file.  Both files have the
// exact same format, so this function just gets reused in both instances.
void	message_parse();
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

#endif
