/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionMessage.cpp $
 * $Revision: 2.12 $
 * $Date: 2004-03-06 23:28:23 $
 * $Author: bobboau $
 *
 * Controls messaging to player during the mission
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.11  2004/03/05 09:02:06  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.10  2004/02/20 04:29:55  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 2.9  2004/02/13 04:17:13  randomtiger
 * Turned off fog in OGL for Fred.
 * Simulated speech doesnt say tags marked by $ now.
 * The following are fixes to issues that came up testing TBP in fs2_open and fred2_open:
 * Changed vm_vec_mag and parse_tmap to fail gracefully on bad data.
 * Error now given on missing briefing icon and bad ship normal data.
 * Solved more species divide by zero error.
 * Fixed neb cube crash.
 *
 * Revision 2.8  2004/02/06 21:26:07  Goober5000
 * fixed a small compatibility bug
 * --Goober5000
 *
 * Revision 2.7  2004/02/05 14:29:33  Goober5000
 * fixed the talking head error
 * --Goober5000
 *
 * Revision 2.6  2004/01/14 21:12:24  Goober5000
 * I think this will fix the problem of the death head ani sometimes incorrectly playing
 * --Goober5000
 *
 * Revision 2.5  2003/11/11 02:15:45  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.4  2003/10/16 16:38:16  Kazan
 * couple more types in species_defs.cpp, also finished up "Da Species Upgrade"
 *
 * Revision 2.3  2003/09/07 18:14:53  randomtiger
 * Checked in new speech code and calls from relevent modules to make it play.
 * Should all work now if setup properly with version 2.4 of the launcher.
 * FS2_SPEECH can be used to make the speech code compile if you have SAPI 5.1 SDK installed.
 * Otherwise the compile flag should not be set and it should all compile OK.
 *
 * - RT
 *
 * Revision 2.2  2002/10/17 20:40:51  randomtiger
 * Added ability to remove HUD ingame on keypress shift O
 * So I've added a new key to the bind list and made use of already existing hud removal code.
 *
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.4  2002/05/13 15:11:03  mharris
 * More NO_NETWORK ifndefs added
 *
 * Revision 1.3  2002/05/10 20:42:44  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.2  2002/05/10 06:08:08  mharris
 * Porting... added ifndef NO_SOUND
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 32    9/12/99 8:09p Dave
 * Fixed problem where skip-training button would cause mission messages
 * not to get paged out for the current mission.
 * 
 * 31    9/01/99 2:52p Andsager
 * Add new heads to FRED and some debug code for playing heads
 * 
 * 30    8/28/99 7:29p Dave
 * Fixed wingmen persona messaging. Make sure locked turrets don't count
 * towards the # attacking a player.
 * 
 * 29    8/26/99 8:51p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 28    8/23/99 5:04p Jefff
 * Added new mission flag to disable built-in messages from playing.
 * Added fred support as well.
 * 
 * 27    8/19/99 10:12a Alanl
 * preload mission-specific messages on machines greater than 48MB
 * 
 * 26    8/18/99 12:09p Andsager
 * Add debug if message has no anim for message.  Make messages come from
 * wing leader.
 * 
 * 25    7/31/99 2:30p Dave
 * Added nifty mission message debug viewing keys.
 * 
 * 24    7/24/99 2:19p Dave
 * Fixed broken build.
 * 
 * 23    7/23/99 5:44p Andsager
 * make personas consistently choose same ship
 * 
 * 22    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 21    7/14/99 4:27p Andsager
 * Added multiple message debug check
 * 
 * 20    7/06/99 10:41a Andsager
 * Add AWACS need help messages
 * 
 * 19    7/02/99 11:16a Andsager
 * Removed mult message debug check.
 * 
 * 18    7/02/99 11:13a Andsager
 * max debug version
 * 
 * 17    6/16/99 10:20a Dave
 * Added send-message-list sexpression.
 * 
 * 16    6/14/99 5:53p Dave
 * Removed duplicate message check temporarily.
 * 
 * 15    6/10/99 3:43p Dave
 * Do a better job of syncing text colors to HUD gauges.
 * 
 * 14    6/09/99 2:56p Andsager
 * Check all messages for repeat.  Allow multiple versions of same message
 * if queued > 20 apart.
 * 
 * 13    6/07/99 11:33a Anoop
 * Get rid of erroneous Int3() in multiple message check.
 * 
 * 12    6/07/99 10:31a Andsager
 * Get rid of false multiplayer multiple messages catch.
 * 
 * 11    6/03/99 2:56p Andsager
 * DOH!!
 * 
 * 10    6/03/99 2:44p Andsager
 * Fix stupid bug in debug code.
 * 
 * 9     6/03/99 2:08p Andsager
 * Put in debug code to find multiple mission messages.
 * 
 * 8     3/29/99 6:17p Dave
 * More work on demo system. Got just about everything in except for
 * blowing ships up, secondary weapons and player death/warpout.
 * 
 * 7     1/28/99 12:19a Dave
 * Fixed a dumb debug build unhandled exception.
 * 
 * 6     1/07/99 10:08a Jasen
 * coords
 * 
 * 5     1/07/99 9:24a Dave
 * Put in hi-res coord support for head anim.
 * 
 * 4     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 3     10/23/98 3:51p Dave
 * Full support for tstrings.tbl and foreign languages. All that remains
 * is to make it active in Fred.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 127   8/25/98 1:48p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 126   6/01/98 11:43a John
 * JAS & MK:  Classified all strings for localization.
 * 
 * 125   5/24/98 12:55a Mike
 * Fix bug with scream from Installation.  Should also fix bug with double
 * screams from some ships.
 * 
 * 124   5/18/98 6:06p Lawrance
 * Don't play messages or auto-target on first frame
 * 
 * 123   5/15/98 8:36p Lawrance
 * Add 'target ship that last sent transmission' target key
 * 
 * 122   5/09/98 10:00p Allender
 * make vasudan persona for support use terran support persona
 * 
 * 121   5/08/98 11:21a Allender
 * fix ingame join trouble.  Small messaging fix.  Enable collisions for
 * friendlies again
 * 
 * 120   5/06/98 12:19p Lawrance
 * Fix typo for 'Stray Warning Final'
 * 
 * 119   5/05/98 9:12p Allender
 * fix large problem introduced last checkin when changiing Assert to if
 * 
 * 118   5/05/98 4:12p Chad
 * changed Assert info if statement when removing messages from queue when
 * too old
 * 
 * 117   5/01/98 12:34p John
 * Added code to force FreeSpace to run in the same dir as exe and made
 * all the parse error messages a little nicer.
 * 
 * 116   4/27/98 9:00p Allender
 * mission specific messages from #<someone> are now sourced to terran
 * command
 * 
 * 115   4/26/98 11:35a Allender
 * make traitor message play by iteself in all cases
 * 
 * 114   4/25/98 11:49p Lawrance
 * Add Terran Command stray messages
 * 
 * 113   4/22/98 9:17a Allender
 * be sure that builtin command messages play with the correct hud source.
 * Also be sure that messages which get converted to Terran command to the
 * same
 * 
 * 112   4/20/98 1:30a Lawrance
 * Don't load head animations if talking head gauge is disabled.
 * 
 * 111   4/17/98 11:03a Allender
 * some rearm message being played too often and sent with incorrect
 * 'who-from'
 * 
 * 110   4/13/98 5:06p Lawrance
 * Cut off talking head about 250ms before wave ends
 * 
 * 109   4/10/98 9:14a Lawrance
 * fix up persona code for the demo
 * 
 * 108   4/09/98 2:15p Allender
 * fixed compiler warnings
 * 
 * 107   4/09/98 12:36p Allender
 * don't allow the same ship to have messages overlapping.  Put in code to
 * check for ship's existence (wingman only) before actually playing
 * message
 * 
 * 106   4/09/98 12:32a Lawrance
 * Fix bugs related to multiple screams from same ship, builtin messages
 * playing after screams, or praising while severly damaged.
 * 
 * 105   4/08/98 3:45p Allender
 * mission message overhaul.  Make message from any wingman mean any
 * wingman with that persona.  Terran command wave and ani's for dead
 * ships now play correctly.
 * 
 * 104   4/07/98 8:09p Lawrance
 * don't play talking heads in the demo
 * 
 * 103   4/07/98 5:30p Lawrance
 * Player can't send/receive messages when comm is destroyed.  Garble
 * messages when comm is damaged.
 * 
 * 102   4/07/98 5:26p Allender
 * low priority mission specific messages won't interrupt anything.
 * 
 * 101   4/07/98 10:51a Allender
 * remove any allied from message senders.  Make heads for mission
 * specific messages play appropriately
 * 
 * 100   4/07/98 12:04a Mike
 * New system for instructor chastising player if he fires at instructor.
 * 
 * 99    4/03/98 11:39a Lawrance
 * only allow 1 wingman persona in demo
 * 
 * 98    4/02/98 1:09p Allender
 * don't process messages before player "enters" mission (i.e. due to
 * player entry delay)
 * 
 * 97    4/02/98 10:06a Allender
 * wing arrival message for delta and epsilon wings
 * 
 * 96    4/01/98 10:47p Lawrance
 * Supporting builtin messages for rearm and repair requests
 * 
 * 95    3/25/98 8:43p Hoffoss
 * Changed anim_play() to not be so complex when you try and call it.
 * 
 * 94    3/24/98 12:46p Allender
 * save shipnum before killing currently playing message in preparation
 * for playing death scream.
 * 
 * 93    3/22/98 3:54p Andsager
 * AL: Prevent -1 index into Ships[] array when playing a scream
 * 
 * 92    3/18/98 10:20p Allender
 * force wingman scream when he's talking and then dies
 * 
 * 91    3/18/98 12:03p John
 * Marked all the new strings as externalized or not.
 * 
 * 90    3/17/98 4:01p Hoffoss
 * Added HUD_SOURCE_TERRAN_CMD and changed code to utilize it when a
 * message is being sent from Terran Command.
 * 
 * 89    3/05/98 10:18p Lawrance
 * Play voice cue sound when there is no voice file present
 * 
 * 88    3/02/98 5:42p John
 * Removed WinAVI stuff from Freespace.  Made all HUD gauges wriggle from
 * afterburner.  Made gr_set_clip work good with negative x &y.  Made
 * model_caching be on by default.  Made each cached model have it's own
 * bitmap id.  Made asteroids not rotate when model_caching is on.  
 * 
 * 87    3/02/98 9:34a Allender
 * don't allow mission specific messages to timeout.  Assert when trying
 * to remove a mission specific messages from the queue.  Print out in the
 * log file if the voice didn't play.
 * 
 * 86    2/23/98 8:45a John
 * Externalized Strings
 * 
 * 85    2/20/98 8:33p Lawrance
 * Add the 'All Alone' message
 * 
 * 84    2/16/98 2:20p Allender
 * make death scream kill any other messages from that ship
 * 
 * 83    2/12/98 4:58p Lawrance
 * Add support for 'All Clear' radio message
 * 
 * 82    2/11/98 9:44p Allender
 * rearm repair code fixes.  hud support view shows abort status.  New
 * support ship killed message.  More network stats
 * 
 * 81    2/04/98 10:44p Allender
 * mark personas as not used between missions.   Don't ever randomly
 * choose a Vasudan persona
 * 
 * 80    1/29/98 11:38a Allender
 * support for Vasudan personas
 * 
 * 79    1/25/98 10:04p Lawrance
 * Fix nasty divide-by-zero bug in message_calc_anim_start_frame().
 * 
 * 78    1/24/98 4:46p Lawrance
 * add in support for new voice messasges
 * 
 * 77    1/22/98 5:13p Lawrance
 * pick useful starting frame when playing animation
 * 
 * 76    1/21/98 7:20p Lawrance
 * Make subsystem locking only work with line-of-sight, cleaned up locking
 * code, moved globals to player struct.
 * 
 * 75    1/21/98 11:54a Duncan
 * Commended out assert for the moment to allow Fred to run.  Mark can fix
 * this properly later, since he understands it.
 * 
 * 74    1/21/98 10:33a Allender
 * fixed up messaging code to play a random head when playing a builtin
 * message
 * 
 * 73    1/20/98 6:21p Lawrance
 * Stop animation from playing when voice clip ends early.
 * 
 * 72    1/20/98 3:43p Allender
 * don't queue messages when player becomes traitor
 * 
 * 71    1/20/98 12:52p Lawrance
 * Draw talking head as alpha-color bitmap, black out region behind
 * animation.
 * 
 * 70    1/20/98 10:20a Lawrance
 * Draw head animation as alpha-colored.
 * 
 * 69    1/18/98 9:51p Lawrance
 * Add support for 'Player Died' messages.
 * 
 * 68    1/14/98 9:49p Allender
 * removed 3'oclock and 9'oclock messages
 * 
 * 67    1/13/98 3:11p Allender
 * new messages for disable/disarm
 * 
 * 66    1/12/98 11:16p Lawrance
 * Wonderful HUD config.
 * 
 * 65    1/07/98 4:41p Allender
 * minor modification to special messages.  Fixed cargo_revealed problem
 * for multiplayer and problem with is-cargo-known sexpression
 * 
 * 64    12/15/97 12:14p Allender
 * implemented overlapping messages
 * 
 * 63    12/12/97 4:58p Allender
 * make messages interruptable.  Put in code to support multiple messages
 * at once, although this feature not fully implemented yet.
 * 
 * 62    12/04/97 9:37p Dave
 * Fixed a bunch of multiplayer messaging bugs.
 * 
 * 61    12/02/97 2:37p Allender
 * added asserts to be sure that an actual ship (i.e. not cargo or
 * otherwise) is the ship sending a message to the player
 *
 * $NoKeywords: $
 */

#include "mission/missionmessage.h"
#include "mission/missiontraining.h"
#include "hud/hudmessage.h"
#include "hud/hudgauges.h"
#include "hud/hudtarget.h"
#include "io/timer.h"
#include "parse/parselo.h"
#include "gamesnd/gamesnd.h"
#include "gamesequence/gamesequence.h"
#include "anim/animplay.h"
#include "hud/hud.h"
#include "ship/ship.h"
#include "ship/subsysdamage.h"
#include "weapon/emp.h"
#include "localization/localize.h"
#include "demo/demo.h"
#include "hud/hudconfig.h"
#include "sound/fsspeech.h"
#include "species_defs/species_defs.h"

#ifndef NO_NETWORK
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#endif

// here is a text list of the builtin message names.  These names are used to match against
// names read in for builtin message radio bits to see what message to play.  These are
// generic names, meaning that there will be the same message type for a number of different
// personas
char *Builtin_message_types[MAX_BUILTIN_MESSAGE_TYPES] =
{
//XSTR:OFF
	"Arrive Enemy",
	"Attack Target",
	"Beta Arrived",
	"Check 6",
	"Engage",
	"Gamma Arrived",
	"Help",
	"Praise",
	"Backup",
	"Ignore Target",
	"No",
	"Oops 1",
	"Permission",			// AL: no code support yet
	"Stray",					// DA: no code support
	"Depart",
	"yes",
	"Rearm on Way",
	"On way",
	"Rearm warping in",
	"No Target",
	"Docking Start",		// AL: no message seems to exist for this
	"Repair Done",
	"Repair Aborted",
	"Traitor",
	"Rearm",
	"Disable Target",
	"Disarm Target",
	"Player Dead",
	"Death",
	"Support Killed",
	"All Clear",			// DA: no code support
	"All Alone",
	"Repair",
	"Delta Arrived",
	"Epsilon Arrived",
	"Instructor Hit",
	"Instructor Attack",
	"Stray Warning",
	"Stray Warning Final",
	"AWACS at 75",
	"AWACS at 25"
//XSTR:ON
};

MMessage Messages[MAX_MISSION_MESSAGES];
int Message_times[MAX_MISSION_MESSAGES];

int Num_messages, Num_message_avis, Num_message_waves;
int Num_builtin_messages, Num_builtin_avis, Num_builtin_waves;

int Message_debug_index = -1;

message_extra Message_avis[MAX_MESSAGE_AVIS];
message_extra Message_waves[MAX_MESSAGE_WAVES];

#define MAX_PLAYING_MESSAGES		2

#ifdef FS2_DEMO
	#define MAX_WINGMAN_HEADS			1
	#define MAX_COMMAND_HEADS			1
#else
#define MAX_WINGMAN_HEADS			2
#define MAX_COMMAND_HEADS			3
#endif

//XSTR:OFF
#define HEAD_PREFIX_STRING			"head-"
#define COMMAND_HEAD_PREFIX		"head-cm1"
#define COMMAND_WAVE_PREFIX		"TC_"
#define SUPPORT_NAME					"Support"
//XSTR:ON

// variables to keep track of messages that are currently playing
int Num_messages_playing;						// number of is a message currently playing?

typedef struct pmessage {
	anim_instance *anim;		// handle of anim currently playing
	int wave;					// handle of wave currently playing
	int id;						// id of message currently playing
	int priority;				// priority of message currently playing
	int shipnum;				// shipnum of ship sending this message,  -1 if from Terran command
	int builtin_type;			// if a builtin message, type of the message
} pmessage;

LOCAL pmessage Playing_messages[MAX_PLAYING_MESSAGES];

int Message_shipnum;						// ship number of who is sending message to player -- used outside this module

// variables to control message queuing.  All new messages to the player are queued.  The array
// will be ordered by priority, then time submitted.

#define MQF_CONVERT_TO_COMMAND		(1<<0)			// convert this queued message to terran command
#define MQF_CHECK_ALIVE					(1<<1)			// check for the existence of who_from before sending

typedef struct message_q {
	fix	time_added;					// time at which this entry was added
	int	window_timestamp;			// timestamp which will tell us how long we have to play the message
	int	priority;					// priority of the message
	int	message_num;				// index into the Messages[] array
	char	who_from[NAME_LENGTH];	// who this message is from
	int	source;						// who the source of the message is (HUD_SOURCE_* type)
	int	builtin_type;				// type of builtin message (-1 if mission message)
	int	flags;						// should this message entry be converted to Terran Command head/wave file
	int	min_delay_stamp;			// minimum delay before this message will start playing
	int	group;						// message is part of a group, don't time it out
} message_q;

#define MAX_MESSAGE_Q				30
#define MAX_MESSAGE_LIFE			F1_0*30		// After being queued for 30 seconds, don't play it
#define DEFAULT_MESSAGE_LENGTH	3000			// default number of milliseconds to display message indicator on hud
message_q	MessageQ[MAX_MESSAGE_Q];
int MessageQ_num;			// keeps track of number of entries on the queue.

#define MESSAGE_IMMEDIATE_TIMESTAMP		1000		// immediate messages must play within 1 second
#define MESSAGE_SOON_TIMESTAMP			5000		// "soon" messages must play within 5 seconds
#define MESSAGE_ANYTIME_TIMESTAMP		-1			// anytime timestamps are invalid

// Persona information
int Num_personas;
Persona Personas[MAX_PERSONAS];

char *Persona_type_names[MAX_PERSONA_TYPES] = 
{
//XSTR:OFF
	"wingman",
	"support",
	"large", 
	"command",
//XSTR:ON
};

int Command_persona;

///////////////////////////////////////////////////////////////////
// used to distort incoming messages when comms are damaged
///////////////////////////////////////////////////////////////////
static int Message_wave_muted;
static int Message_wave_duration;
static int Next_mute_time;

#define MAX_DISTORT_PATTERNS	2
#define MAX_DISTORT_LEVELS		6
static float Distort_patterns[MAX_DISTORT_PATTERNS][MAX_DISTORT_LEVELS] = 
{
	{0.20f, 0.20f, 0.20f, 0.20f, 0.20f, 0.20f},
	{0.10f, 0.20f, 0.25f, 0.25f, 0.05f, 0.15f}
};

static int Distort_num;		// which distort pattern is being used
static int Distort_next;	// which section of distort pattern is next

int Head_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		7, 45
	},
	{ // GR_1024
		7, 66
	}
};

// forward declaration
void message_maybe_distort_text(char *text);

// following functions to parse messages.tbl -- code pretty much ripped from weapon/ship table parsing code

// functions to deal with parsing personas.  Personas are just a list of names that give someone
// sending a message an identity which spans the life of the mission
void persona_parse()
{
	int i;
	char type[NAME_LENGTH];

	Assert ( Num_personas < MAX_PERSONAS );

	Personas[Num_personas].flags = 0;
	required_string("$Persona:");
	stuff_string(Personas[Num_personas].name, F_NAME, NULL);

	// get the type name and set the appropriate flag
	required_string("$Type:");
	stuff_string( type, F_NAME, NULL );
	for ( i = 0; i < MAX_PERSONA_TYPES; i++ ) {
		if ( !stricmp( type, Persona_type_names[i]) ) {

			Personas[Num_personas].flags |= (1<<i);

			// save the Terran Command persona in a global
			if ( Personas[Num_personas].flags & PERSONA_FLAG_COMMAND ) {
//				Assert ( Command_persona == -1 );
				Command_persona = Num_personas;
			}

			break;
		}
	}

#if defined(MORE_SPECIES)

	char cstrtemp[SPECIES_NAME_MAXLEN+1];
	memset(cstrtemp, 0, SPECIES_NAME_MAXLEN+1);
	if ( optional_string("+") )
	{
		stuff_string(cstrtemp, F_NAME, NULL, SPECIES_NAME_MAXLEN);

		for (int j = 0; j < True_NumSpecies; j++)
		{
			if (!strcmp(cstrtemp, Species_names[j]))
			{
				Personas[Num_personas].species = j;
				break;
			}
		}
	}
#else
	if ( optional_string("+Vasudan") )
		Personas[Num_personas].flags |= PERSONA_FLAG_VASUDAN;
#endif
	if ( i == MAX_PERSONA_TYPES )
		Error(LOCATION, "Unknown persona type in messages.tbl -- %s\n", type );


	Num_personas++;
}

// two functions to add avi/wave names into a table
int add_avi( char *avi_name )
{
	int i;

	Assert ( Num_message_avis < MAX_MESSAGE_AVIS );
	Assert (strlen(avi_name) < MAX_FILENAME_LEN );

	// check to see if there is an existing avi being used here
	for ( i = 0; i < Num_message_avis; i++ ) {
		if ( !stricmp(Message_avis[i].name, avi_name) )
			return i;
	}

	// would have returned if a slot existed.
	strcpy( Message_avis[Num_message_avis].name, avi_name );
	Message_avis[Num_message_avis].num = -1;
	Num_message_avis++;
	return (Num_message_avis - 1);
}

int add_wave( char *wave_name )
{
	int i;

	Assert ( Num_message_waves < MAX_MESSAGE_WAVES );
	Assert (strlen(wave_name) < MAX_FILENAME_LEN );

	// check to see if there is an existing wave being used here
	for ( i = 0; i < Num_message_waves; i++ ) {
		if ( !stricmp(Message_waves[i].name, wave_name) )
			return i;
	}

	strcpy( Message_waves[Num_message_waves].name, wave_name );
	Message_waves[Num_message_waves].num = -1;
	Num_message_waves++;
	return (Num_message_waves - 1);
}

// parses an individual message
void message_parse( )
{
	MissionMessage *msgp;
	char persona_name[NAME_LENGTH];

	Assert ( Num_messages < MAX_MISSION_MESSAGES );
	msgp = &Messages[Num_messages];

	required_string("$Name:");
	stuff_string(msgp->name, F_NAME, NULL);

	// team
	msgp->multi_team = -1;
	if(optional_string("$Team:")){
		int mt;
		stuff_int(&mt);

		// keep it real
		if((mt < 0) || (mt >= 2)){
			mt = -1;
		}

		// only bother with filters if multiplayer and TvT
#ifndef NO_NETWORK
		if(Fred_running || ((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM)) ){
#else
		if (Fred_running) {
#endif
			msgp->multi_team = mt;
		}
	}

	// backwards compatibility for old fred missions - all new ones should use $MessageNew
	if(optional_string("$Message:")){
		stuff_string(msgp->message, F_MESSAGE, NULL);
	} else {
		required_string("$MessageNew:");
		stuff_string(msgp->message, F_MULTITEXT, NULL);
	}

	msgp->persona_index = -1;
	if ( optional_string("+Persona:") ) {
		stuff_string(persona_name, F_NAME, NULL);
		msgp->persona_index = message_persona_name_lookup( persona_name );
	}

	if ( !Fred_running)
		msgp->avi_info.index = -1;
	else
		msgp->avi_info.name = NULL;

	if ( optional_string("+AVI Name:") ) {
		char avi_name[MAX_FILENAME_LEN];

		stuff_string(avi_name, F_NAME, NULL);
		if ( !Fred_running ) {
			msgp->avi_info.index = add_avi(avi_name);
		} else {
			msgp->avi_info.name = strdup(avi_name);
		}
	}

	if ( !Fred_running )
		msgp->wave_info.index = -1;
	else
		msgp->wave_info.name = NULL;

	if ( optional_string("+Wave Name:") ) {
		char wave_name[MAX_FILENAME_LEN];

		stuff_string(wave_name, F_NAME, NULL);
		if ( !Fred_running ) {
			msgp->wave_info.index = add_wave(wave_name);
		} else {
			msgp->wave_info.name = strdup(wave_name);
		}
	}

	Num_messages++;
}

void parse_msgtbl()
{
	// open localization
	lcl_ext_open();

	read_file_text("messages.tbl");
	reset_parse();
	Num_messages = 0;
	Num_personas = 0;

	required_string("#Personas");
	while ( required_string_either("#Messages", "$Persona:")){
		persona_parse();
	}

	required_string("#Messages");
	while (required_string_either("#End", "$Name:")){
		message_parse();
	}

	required_string("#End");

	// save the number of builting message things -- make initing between missions easier
	Num_builtin_messages = Num_messages;
	Num_builtin_avis = Num_message_avis;
	Num_builtin_waves = Num_message_waves;

	// close localization
	lcl_ext_close();
}

// this is called at the start of each level
void messages_init()
{
	int rval, i;
	static int table_read = 0;

	if ( !table_read ) {
		Command_persona = -1;
		if ((rval = setjmp(parse_abort)) != 0) {
			Error(LOCATION, "Error parsing '%s'\r\nError code = %i.\r\n", "messages.tbl", rval);

		} else {			
			parse_msgtbl();
			table_read = 1;
		}
	}

	// reset the number of messages that we have for this mission
	Num_messages = Num_builtin_messages;
	Message_debug_index = Num_builtin_messages - 1;
	Num_message_avis = Num_builtin_avis;
	Num_message_waves = Num_builtin_waves;

	// initialize the stuff for the linked lists of messages
	MessageQ_num = 0;
	for (i = 0; i < MAX_MESSAGE_Q; i++) {
		MessageQ[i].priority = -1;
		MessageQ[i].time_added = -1;
		MessageQ[i].message_num = -1;
		MessageQ[i].builtin_type = -1;
		MessageQ[i].min_delay_stamp = -1;
		MessageQ[i].group = 0;
	}
	
	// this forces a reload of the AVI's and waves for builtin messages.  Needed because the flic and
	// sound system also get reset between missions!
	for (i = 0; i < Num_builtin_avis; i++ ) {
		Message_avis[i].anim_data = NULL;
	}

	for (i = 0; i < Num_builtin_waves; i++ ){
		Message_waves[i].num = -1;
	}

	Message_shipnum = -1;
	Num_messages_playing = 0;
	for ( i = 0; i < MAX_PLAYING_MESSAGES; i++ ) {
		Playing_messages[i].anim = NULL;
		Playing_messages[i].wave = -1;
		Playing_messages[i].id = -1;
		Playing_messages[i].priority = -1;
		Playing_messages[i].shipnum = -1;
		Playing_messages[i].builtin_type = -1;
	}

	// reinitialize the personas.  mark them all as not used
	for ( i = 0; i < Num_personas; i++ ){
		Personas[i].flags &= ~PERSONA_FLAG_USED;
	}

	Message_wave_muted = 0;
	Next_mute_time = 1;

	memset(Message_times, 0, sizeof(int)*MAX_MISSION_MESSAGES);

}

// called to do cleanup when leaving a mission
void message_mission_shutdown()
{
	int i, j;

	mprintf(("Unloading in mission messages\n"));

	training_mission_shutdown();

	// remove the wave sounds from memory
	for (i = 0; i < Num_message_waves; i++ ) {
		if ( Message_waves[i].num != -1 ){
			snd_unload( Message_waves[i].num );
		}
	}

	fsspeech_stop();
	//Taylor from icculus found and fixed this -Bobboau
    // free up remaining anim data 
    for (i=0; i<Num_message_avis; i++) { 
            if (Message_avis[i].anim_data != NULL) { 
                    for (j=0; j<Message_avis[i].anim_data->ref_count; j++) { 
                            anim_free(Message_avis[i].anim_data); 
                    } 
            } 
            Message_avis[i].anim_data = NULL; 
    } 
}

// functions to deal with queuing messages to the message system.

//	Compare function for system qsort() for sorting message queue entries based on priority.
//	Return values set to sort array in _decreasing_ order.  If priorities equal, sort based
// on time added into queue
int message_queue_priority_compare(const void *a, const void *b)
{
	message_q *ma, *mb;

	ma = (message_q *) a;
	mb = (message_q *) b;

	if (ma->priority > mb->priority) {
		return -1;
	} else if (ma->priority < mb->priority) {
		return 1;
	} else if (ma->time_added < mb->time_added) {
		return -1;
	} else if (ma->time_added > mb->time_added) {
		return 1;
	} else {
		return 0;
	}
}

// function to kill all currently playing messages.  kill_all parameter tells us to
// kill only the animations that are playing, or wave files too
void message_kill_all( int kill_all )
{
	int i;

	Assert( Num_messages_playing );

	// kill sounds for all voices currently playing
	for ( i = 0; i < Num_messages_playing; i++ ) {
		if ( (Playing_messages[i].anim != NULL) && anim_playing(Playing_messages[i].anim) ) {
			anim_stop_playing( Playing_messages[i].anim );
			Playing_messages[i].anim=NULL;
		}

		if ( kill_all ) {
			if ( (Playing_messages[i].wave != -1 ) && snd_is_playing(Playing_messages[i].wave) ){
				snd_stop( Playing_messages[i].wave );
			}

			Playing_messages[i].shipnum = -1;
		}
	}

	if ( kill_all ) {
		Num_messages_playing = 0;
	}

	fsspeech_stop();
}

// function to kill nth playing message
void message_kill_playing( int message_num )
{
	Assert( message_num < Num_messages_playing );

	if ( (Playing_messages[message_num].anim != NULL) && anim_playing(Playing_messages[message_num].anim) ) {
		anim_stop_playing( Playing_messages[message_num].anim );
		Playing_messages[message_num].anim=NULL;
	}
	if ( (Playing_messages[message_num].wave != -1 ) && snd_is_playing(Playing_messages[message_num].wave) )
		snd_stop( Playing_messages[message_num].wave );

	Playing_messages[message_num].shipnum = -1;

	fsspeech_stop();
}


// returns true if all messages currently playing are builtin messages
int message_playing_builtin()
{
	int i;

	for ( i = 0; i < Num_messages_playing; i++ ) {
		if ( Playing_messages[i].id >= Num_builtin_messages ){
			break;
		}
	}

	// if we got through the list without breaking, all playing messages are builtin messages
	if ( i == Num_messages_playing ){
		return 1;
	} else {
		return 0;
	}
}

// returns true in any playing message is of the specific builtin type
int message_playing_specific_builtin( int builtin_type )
{
	int i;

	for (i = 0; i < Num_messages_playing; i++ ) {
		if ( (Playing_messages[i].id < Num_builtin_messages) && (Playing_messages[i].builtin_type == builtin_type) ){
			return 1;
		}
	}

	return 0;
}

// returns true if all messages current playing are unique messages
int message_playing_unique()
{
	int i;

	for ( i = 0; i < Num_messages_playing; i++ ) {
		if ( Playing_messages[i].id < Num_builtin_messages ){
			break;
		}
	}

	// if we got through the list without breaking, all playing messages are builtin messages
	if ( i == Num_messages_playing ){
		return 1;
	} else {
		return 0;
	}
}


// returns the highest priority of the currently playing messages
#define MESSAGE_GET_HIGHEST		1
#define MESSAGE_GET_LOWEST			2
int message_get_priority(int which)
{
	int i;
	int priority;

	if ( which == MESSAGE_GET_HIGHEST ){
		priority = MESSAGE_PRIORITY_LOW;
	} else {
		priority = MESSAGE_PRIORITY_HIGH;
	}

	for ( i = 0; i < Num_messages_playing; i++ ) {
		if ( (which == MESSAGE_GET_HIGHEST) && (Playing_messages[i].priority > priority) ){
			priority = Playing_messages[i].priority;
		} else if ( (which == MESSAGE_GET_LOWEST) && (Playing_messages[i].priority < priority) ){
			priority = Playing_messages[i].priority;
		}
	}

	return priority;
}


// removes current message from the queue
void message_remove_from_queue(message_q *q)
{	
	// quick out if nothing to do.
	if ( MessageQ_num <= 0 ) {
		return;
	}	

	MessageQ_num--;
	q->priority = -1;
	q->time_added = -1;
	q->message_num = -1;
	q->builtin_type = -1;
	q->min_delay_stamp = -1;
	q->group = 0;	

	if ( MessageQ_num > 0 ) {
		qsort(MessageQ, MAX_MESSAGE_Q, sizeof(message_q), message_queue_priority_compare);
	}
}

// Load in the sound data for a message.
//
// index - index into the Message_waves[] array
//
void message_load_wave(int index, const char *filename)
{
	if (index == -1) {
		Int3();
		return;
	}

	if ( Message_waves[index].num >= 0) {
		return;
	}

	game_snd tmp_gs;
	memset(&tmp_gs, 0, sizeof(game_snd));
	strcpy( tmp_gs.filename, filename );
	Message_waves[index].num = snd_load( &tmp_gs, 0 );
	if ( Message_waves[index].num == -1 ) {
		nprintf (("messaging", "Cannot load message wave: %s.  Will not play\n", Message_waves[index].name ));
	}
}

// Play wave file associated with message
// input: m		=>		pointer to message description
//
// note: changes Messave_wave_duration, Playing_messages[].wave, and Message_waves[].num
bool message_play_wave( message_q *q )
{
	int index;
	MissionMessage *m;
	char filename[MAX_FILENAME_LEN];

	// check for multiple messages playing.  don't check builtin messages.
	if (q->message_num >= Num_builtin_messages) {
		if ( (f2fl(Missiontime - Message_times[q->message_num]) < 10) && (f2fl(Missiontime) > 10) ) {
			// Int3();  // Get Andsager
		}
		Message_times[q->message_num] = Missiontime;
	}

	m = &Messages[q->message_num];

	if ( m->wave_info.index != -1 ) {
		index = m->wave_info.index;

		// sanity check
		Assert( index != -1 );
		if ( index == -1 ){
			return false;
		}

		// if we need to bash the wave name because of "conversion" to terran command, do it here
		strcpy( filename, Message_waves[index].name );
		if ( q->flags & MQF_CONVERT_TO_COMMAND ) {
			char *p, new_filename[MAX_FILENAME_LEN];

			Message_waves[index].num = -1;					// forces us to reload the message

			// bash the filename here. Look for "[1-6]_" at the front of the message.  If found, then
			// convert to TC_*
			p = strchr(filename, '_' );
			if ( p == NULL ) {
				mprintf(("Cannot convert %s to terran command wave -- find Sandeep or Allender\n", Message_waves[index].name));
				return false;
			}

			// prepend the command name, and then the rest of the filename.
			p++;
			strcpy( new_filename, COMMAND_WAVE_PREFIX );
			strcat( new_filename, p );
			strcpy( filename, new_filename );
		}

		// load the sound file into memory
		message_load_wave(index, filename);
		if ( Message_waves[index].num == -1 ) {
			m->wave_info.index = -1;
		}

		if ( m->wave_info.index >= 0 ) {
			// this call relies on the fact that snd_play returns -1 if the sound cannot be played
			Message_wave_duration = snd_get_duration(Message_waves[index].num);
			Playing_messages[Num_messages_playing].wave = snd_play_raw( Message_waves[index].num, 0.0f );

			return (Playing_messages[Num_messages_playing].wave != -1);
		}
	}

	return false;
}

// Determine the starting frame for the animation
// input:	time	=>		time of voice clip, in ms
//				ani	=>		pointer to anim data
//				reverse	=>	flag to indicate that the start should be time ms from the end (used for death screams)
int message_calc_anim_start_frame(int time, anim *ani, int reverse)
{
	float	wave_time, anim_time;
	int	start_frame;

	start_frame=0;

	// If no voice clip exists, start from beginning of anim
	if ( time <= 0 ) {
		return start_frame;
	}

	// convert time to seconds
	wave_time = time/1000.0f;
	anim_time = ani->time;

	// If voice clip is longer than anim, start from beginning of anim
	if ( wave_time >= (anim_time) ) {
		return start_frame;
	}

	if ( reverse ) {
		start_frame = (ani->total_frames-1) - fl2i(ani->fps * wave_time + 0.5f);
	} else {
		int num_frames_extra;
		num_frames_extra = fl2i(ani->fps * (anim_time - wave_time) + 0.5f);
		if ( num_frames_extra > 0 ) {
			start_frame=rand()%num_frames_extra;
		}
	}

	if ( start_frame < 0 ) {
		Int3();
		start_frame=0;
	}

	return start_frame;
}

// Play animation associated with message
// input:	m		=>		pointer to message description
//				q		=>		message queue data
//
// note: changes Messave_wave_duration, Playing_messages[].wave, and Message_waves[].num
void message_play_anim( message_q *q )
{
	message_extra	*anim_info;
	int				is_death_scream=0, persona_index=-1, rand_index=0;
	char				ani_name[MAX_FILENAME_LEN], *p;
	MissionMessage	*m;

	m = &Messages[q->message_num];

	// check to see if the avi_index is valid -- try and load/play the avi if so.
	if ( m->avi_info.index < 0 ) {
		return;
	}

	anim_info = &Message_avis[m->avi_info.index];

	// get the filename.  Strip off the extension since we won't need it anyway
	strcpy(ani_name, anim_info->name);
	p = strchr(ani_name, '.');			// gets us to the extension
	if ( p ) {
		*p = '\0';
	}

	// builtin messages are given a base ani which we should add a suffix on before trying
	// to load the animation.  See if this message is a builtin message which has a persona
	// attached to it.  Deal with munging the name

	// support ships use a wingman head.
	// terran command uses it's own set of heads.
	int subhead_selected = FALSE;
	if ( (q->message_num < Num_builtin_messages) || !(_strnicmp(HEAD_PREFIX_STRING, ani_name, strlen(HEAD_PREFIX_STRING)-1)) ) {
		persona_index = m->persona_index;
		
		// if this ani should be converted to a terran command, set the persona to the command persona
		// so the correct head plays.
		if ( q->flags & MQF_CONVERT_TO_COMMAND ) {
			persona_index = Command_persona;
			strcpy( ani_name, COMMAND_HEAD_PREFIX );
		}

		// Goober5000 - guard against negative array indexing; this way, if no persona was
		// assigned, the logic will drop down below like it's supposed to
		if (persona_index >= 0)
		{
			if ( Personas[persona_index].flags & (PERSONA_FLAG_WINGMAN | PERSONA_FLAG_SUPPORT) ) {
				// get a random head
				if ( q->builtin_type == MESSAGE_WINGMAN_SCREAM ) {
					rand_index = MAX_WINGMAN_HEADS;		// [0,MAX) are regular heads; MAX is always death head
					is_death_scream = 1;
				} else {
					rand_index = ((int) Missiontime % MAX_WINGMAN_HEADS);
				}
				sprintf(ani_name, "%s%c", ani_name, 'a'+rand_index);
				subhead_selected = TRUE;
			} else if ( Personas[persona_index].flags & (PERSONA_FLAG_COMMAND | PERSONA_FLAG_LARGE) ) {
				// get a random head
				// Goober5000 - *sigh*... if mission designers assign a command persona
				// to a wingman head, they risk having the death ani play
				Assert(strlen(ani_name) >= 7);
				if (!strnicmp(ani_name+5,"CM",2) || !strnicmp(ani_name+5,"BS",2))	// Head-CM* or Head-BSH
					rand_index = ((int) Missiontime % MAX_COMMAND_HEADS);
				else
					rand_index = ((int) Missiontime % MAX_WINGMAN_HEADS);
				sprintf(ani_name, "%s%c", ani_name, 'a'+rand_index);
				subhead_selected = TRUE;
			}
		}

		if (!subhead_selected) {
			// choose between a and b
			rand_index = ((int) Missiontime % MAX_WINGMAN_HEADS);
			sprintf(ani_name, "%s%c", ani_name, 'a'+rand_index);
			mprintf(("message '%s' with invalid head.  Fix by assigning persona to the message.\n", m->name));
		}
		nprintf(("Messaging", "playing head %s for %s\n", ani_name, q->who_from));
	}

	// check to see if the avi has been loaded.  If not, then load the AVI.  On an error loading
	// the avi, set the top level index to -1 to avoid multiple tries at loading the flick.
	if ( hud_gauge_active(HUD_TALKING_HEAD) ) {
 
 	//Taylor from icculus found and fixed this -Bobboau
           // if there is something already here that's not this same file then go ahead a let go of it 
           if ( (anim_info->anim_data != NULL) && stricmp(ani_name, anim_info->anim_data->name) ) 
                   anim_free(anim_info->anim_data); 

		anim_info->anim_data = anim_load( ani_name, 0 );
	} else {
		return;
	}

	if ( anim_info->anim_data == NULL ) {
		nprintf (("messaging", "Cannot load message avi %s.  Will not play.\n", ani_name));
		m->avi_info.index = -1;			// if cannot load the avi -- set this index to -1 to avoid trying to load multiple times
	}

	if ( m->avi_info.index >= 0 ) {
		// This call relies on the fact that AVI_play will return -1 if the AVI cannot be played
		// if any messages are already playing, kill off any head anims that are currently playing.  We will
		// only play a head anim of the newest messages being played
		if ( Num_messages_playing > 0 ) {
			nprintf(("messaging", "killing off any currently playing head animations\n"));
			message_kill_all( 0 );
		}

		if ( hud_disabled() ) {
			return;
		}

		if ( hud_gauge_active(HUD_TALKING_HEAD) ) {
			int anim_start_frame;
			anim_play_struct aps;

			// figure out anim start frame
			anim_start_frame = message_calc_anim_start_frame(Message_wave_duration, anim_info->anim_data, is_death_scream);
			anim_play_init(&aps, anim_info->anim_data, Head_coords[gr_screen.res][0], Head_coords[gr_screen.res][1]);
			aps.start_at = anim_start_frame;
			
			// aps.color = &HUD_color_defaults[HUD_color_alpha];
			aps.color = &HUD_config.clr[HUD_TALKING_HEAD];

			Playing_messages[Num_messages_playing].anim = anim_play(&aps);
		}
	}
}

// process the message queue -- called once a frame
void message_queue_process()
{	
	char	buf[4096];
	message_q *q;
	int i;
	MissionMessage *m;

	// Don't play messages until first frame has been rendered
	if ( Framecount < 2 ) {
		return;
	}

	// determine if all playing messages (if any) are done playing.  If any are done, remove their
	// entries collapsing the Playing_messages array if necessary
	if ( Num_messages_playing > 0 ) {

		// for each message playing, determine if it is done.
		i = 0;
		while ( i < Num_messages_playing ) {
			int ani_done, wave_done, j;

			ani_done = 1;
			if ( (Playing_messages[i].anim != NULL) && anim_playing(Playing_messages[i].anim) )
				ani_done = 0;

			wave_done = 1;

//			if ( (Playing_messages[i].wave != -1) && snd_is_playing(Playing_messages[i].wave) )
			if ( (Playing_messages[i].wave != -1) && (snd_time_remaining(Playing_messages[i].wave) > 250) )
				wave_done = 0;

			// AL 1-20-98: If voice message is done, kill the animation early
			if ( (Playing_messages[i].wave != -1) && wave_done ) {
				if ( !ani_done ) {
					anim_stop_playing( Playing_messages[i].anim );
				}
			}

			// see if the ship sending this message is dying.  If do, kill wave and anim
			if ( Playing_messages[i].shipnum != -1 ) {
				if ( (Ships[Playing_messages[i].shipnum].flags & SF_DYING) && (Playing_messages[i].builtin_type != MESSAGE_WINGMAN_SCREAM) ) {
					int shipnum;

					shipnum = Playing_messages[i].shipnum;
					message_kill_playing( i );
					// force this guy to scream
					// AL 22-2-98: Ensure don't use -1 to index into ships array.  Mark, something is incorrect 
					//             here, since message_kill_playing() seems to always set Playing_messages[i].shipnum to -1
					// MWA 3/24/98 -- save shipnum before killing message
					// 
					Assert( shipnum >= 0 );
					if ( !(Ships[shipnum].flags & SF_SHIP_HAS_SCREAMED) ) {
						ship_scream( &Ships[shipnum] );
					}
					continue;							// this should keep us in the while() loop with same value of i.														
				}											// we should enter the next 'if' statement during next pass
			}

			// if both ani and wave are done, mark internal variable so we can do next message on queue, and
			// global variable to clear voice brackets on hud
			if ( wave_done && ani_done ) {
				nprintf(("messaging", "Message %d is done playing\n", i));
				Message_shipnum = -1;
				Num_messages_playing--;
				if ( Num_messages_playing == 0 )
					break;

				// there is still another message playing.  Collapse the playing_message array
				nprintf(("messaging", "Collapsing playing message stack\n"));
				for ( j = i+1; j < Num_messages_playing + 1; j++ ) {
					Playing_messages[j-1] = Playing_messages[j];
				}
			} else {
				// messages is not done playing -- move to next message
				i++;
			}
		}
	}

	// preprocess message queue and remove anything on the queue that is too old.  If next message on
	// the queue can be played, then break out of the loop.  Otherwise, loop until nothing on the queue
	while ( MessageQ_num > 0 ) {
		q = &MessageQ[0];		
		if ( timestamp_valid(q->window_timestamp) && timestamp_elapsed(q->window_timestamp) && !q->group) {
			// remove message from queue and see if more to remove
			nprintf(("messaging", "Message %s didn't play because it didn't fit into time window.\n", Messages[q->message_num].name));
			if ( q->message_num < Num_builtin_messages ){			// we should only ever remove builtin messages this way
				message_remove_from_queue(q);
			} else {
				break;
			}
		} else {
			break;
		}
	}

	// no need to process anything if there isn't anything on the queue
	if ( MessageQ_num <= 0 ){
		return;
	}

	// get a pointer to an item on the queue
	int found = -1;
	int idx = 0;
	while((found == -1) && (idx < MessageQ_num)){
		// if this guy has no min delay timestamp, or it has expired, select him
		if((MessageQ[idx].min_delay_stamp == -1) || timestamp_elapsed(MessageQ[idx].min_delay_stamp)){
			found = idx;
			break;
		}

		// next
		idx++;
	}

	// if we didn't find anything, bail
	if(found == -1){
		return;
	}
	// if this is not the first item on the queue, make it the first item
	if(found != 0){
		message_q temp;

		// store the entry
		memcpy(&temp, &MessageQ[found], sizeof(message_q));

		// move all other entries up
		for(idx=found; idx>0; idx--){
			memcpy(&MessageQ[idx], &MessageQ[idx-1], sizeof(message_q));
		}

		// plop the entry down as being first
		memcpy(&MessageQ[0], &temp, sizeof(message_q));
	}

	q = &MessageQ[0];
	Assert ( q->message_num != -1 );
	Assert ( q->priority != -1 );
	Assert ( q->time_added != -1 );

	if ( Num_messages_playing ) {
		// peek at the first message on the queue to see if it should interrupt, or overlap a currently
		// playing message.  Mission specific messages will always interrupt builtin messages.  They
		// will never interrupt other mission specific messages.
		//
		//  Builtin message might interrupt other builtin messages, or overlap them, all depending on
		// message priority.

		if ( q->builtin_type == MESSAGE_HAMMER_SWINE ) {
			message_kill_all(1);
		} else if ( message_playing_specific_builtin(MESSAGE_HAMMER_SWINE) ) {
			MessageQ_num = 0;
			return;
		} else if ( message_playing_builtin() && ( q->message_num >= Num_builtin_messages) && (q->priority > MESSAGE_PRIORITY_LOW) ) {
			// builtin is playing and we have a unique message to play.  Kill currently playing message
			// so unique can play uninterrupted.  Only unique messages higher than low priority will interrupt
			// other messages.
			message_kill_all(1);
			nprintf(("messaging", "Killing all currently playing messages to play unique message\n"));
		} else if ( message_playing_builtin() && (q->message_num < Num_builtin_messages) ) {
			// when a builtin message is queued, we might either overlap or interrupt the currently
			// playing message.
			//
			// we have to check for num_messages_playing (again), since code for death scream might
			// kill all messages.
			if ( Num_messages_playing ) {
				if ( message_get_priority(MESSAGE_GET_HIGHEST) < q->priority ) {
					// lower priority message playing -- kill it.
					message_kill_all(1);
					nprintf(("messaging", "Killing all currently playing messages to play high priority builtin\n"));
				} else if ( message_get_priority(MESSAGE_GET_LOWEST) > q->priority ) {
					// queued message is a lower priority, so wait it out
					return;
				} else {
					// if we get here, then queued messages is a builtin message with the same priority
					// as the currently playing messages.  This state will cause messages to overlap.
					nprintf(("messaging", "playing builtin message (overlap) because priorities match\n"));
				}
			}
		} else if ( message_playing_unique() && (q->message_num < Num_builtin_messages) ) {
			// code messages can kill any low priority mission specific messages
			if ( Num_messages_playing ) {
				if ( message_get_priority(MESSAGE_GET_HIGHEST) == MESSAGE_PRIORITY_LOW ) {
					message_kill_all(1);
					nprintf(("messaging", "Killing low priority unique messages to play code message\n"));
				} else {
					return;			// do nothing.
				}
			}
		} else {
			return;
		}
	}

	// if we are playing the maximum number of voices, then return.  Make the check here since the above
	// code might kill of currently playing messages
	if ( Num_messages_playing == MAX_PLAYING_MESSAGES )
		return;

	Message_shipnum = ship_name_lookup( q->who_from );

	// see if we need to check if sending ship is alive
	if ( q->flags & MQF_CHECK_ALIVE ) {
		if ( Message_shipnum == -1 ) {
			goto all_done;
		}
	}

	// if this is a ship, then don't play anything if this ship is already talking
	if ( Message_shipnum != -1 ) {
		for ( i = 0; i < Num_messages_playing; i++ ) {
			if ( (Playing_messages[i].shipnum != -1) && (Playing_messages[i].shipnum == Message_shipnum) ){
				return;
			}
		}
	}

	// set up module globals for this message
	m = &Messages[q->message_num];
	Playing_messages[Num_messages_playing].anim = NULL;
	Playing_messages[Num_messages_playing].wave  = -1;
	Playing_messages[Num_messages_playing].id  = q->message_num;
	Playing_messages[Num_messages_playing].priority = q->priority;
	Playing_messages[Num_messages_playing].shipnum = Message_shipnum;
	Playing_messages[Num_messages_playing].builtin_type = q->builtin_type;

	Message_wave_duration = 0;

	// translate tokens in message to the real things
	message_translate_tokens(buf, m->message);

	// AL: added 07/14/97.. only play avi/sound if in gameplay
	if ( gameseq_get_state() != GS_STATE_GAME_PLAY )
		goto all_done;

	// AL 4-7-98: Can't receive messages if comm is destroyed
	if ( hud_communications_state(Player_ship) == COMM_DESTROYED ) {
		goto all_done;
	}

	//	Don't play death scream unless a small ship.
	if ( q->builtin_type == MESSAGE_WINGMAN_SCREAM ) {
		int t = Ship_info[Ships[Message_shipnum].ship_info_index].flags;
		int t2 = SIF_SMALL_SHIP;
		int t3 = t & t2;
		if (!t3) {
			goto all_done;
		}
	}

	// play wave first, since need to know duration for picking anim start frame
	if(message_play_wave(q) == false) {
		fsspeech_play(FSSPEECH_FROM_INGAME, buf);
	}

	// play animation for head
	#ifndef DEMO // do we want this for FS2_DEMO
		message_play_anim(q);
	#endif
	
	// distort the message if comms system is damaged
	message_maybe_distort_text(buf);

#ifndef NDEBUG
	// debug only -- if the message is a builtin message, put in parens whether or not the voice played
	if ( Playing_messages[Num_messages_playing].wave == -1 ) {
		strcat( buf, NOX("..(no wavefile for voice)"));
		snd_play(&Snds[SND_CUE_VOICE]);
	}
#endif

	HUD_sourced_printf( q->source, NOX("%s: %s"), q->who_from, buf );

	if ( Message_shipnum >= 0 ) {
		hud_target_last_transmit_add(Message_shipnum);
	}

all_done:
	Num_messages_playing++;
	message_remove_from_queue( q );
}

// queues up a message to display to the player
void message_queue_message( int message_num, int priority, int timing, char *who_from, int source, int group, int delay, int builtin_type )
{
	int i, m_persona;

	if ( message_num < 0 ) return;

	// some messages can get queued quickly.  Try to filter out certain types of messages before
	// they get queued if there are other messages of the same type already queued
	if ( (builtin_type == MESSAGE_REARM_ON_WAY) || (builtin_type == MESSAGE_OOPS) ) {
		// if it is already playing, then don't play it
		if ( message_playing_specific_builtin(builtin_type) ) 
			return;

		for ( i = 0; i < MessageQ_num; i++ ) {
			// if one of these messages is already queued, the don't play
			if ( (MessageQ[i].message_num == message_num) && (MessageQ[i].builtin_type == builtin_type) )
				return;

		}
	}

	// check to be sure that we haven't reached our max limit on these messages yet.
	if ( MessageQ_num == MAX_MESSAGE_Q ) {
		Int3();											
		return;
	}

	// if player is a traitor, no messages for him!!!
	if ( Player_ship->team == TEAM_TRAITOR ) {
		return;
	}

	m_persona = Messages[message_num].persona_index;

	// put the message into a slot
	i = MessageQ_num;
	MessageQ[i].time_added = Missiontime;
	MessageQ[i].priority = priority;
	MessageQ[i].message_num = message_num;
	MessageQ[i].source = source;
	MessageQ[i].builtin_type = builtin_type;
	MessageQ[i].min_delay_stamp = timestamp(delay);
	MessageQ[i].group = group;
	strcpy(MessageQ[i].who_from, who_from);

	// SPECIAL HACK -- if the who_from is terran command, and there is a wingman persona attached
	// to this message, then set a bit to tell the wave/anim playing code to play the command version
	// of the wave and head
	MessageQ[i].flags = 0;
	if ( !stricmp(who_from, TERRAN_COMMAND) && (m_persona != -1) && (Personas[m_persona].flags & PERSONA_FLAG_WINGMAN) ) {
		MessageQ[i].flags |= MQF_CONVERT_TO_COMMAND;
		MessageQ[i].source = HUD_SOURCE_TERRAN_CMD;
	}

	if ( (m_persona != -1) && (Personas[m_persona].flags & PERSONA_FLAG_WINGMAN) ) {
		if ( !strstr(who_from, ".wav") ) {
			MessageQ[i].flags |= MQF_CHECK_ALIVE;
		}
	}

	// set the timestamp of when to play this message based on the 'timing' value
	if ( timing == MESSAGE_TIME_IMMEDIATE )
		MessageQ[i].window_timestamp = timestamp(MESSAGE_IMMEDIATE_TIMESTAMP);
	else if ( timing == MESSAGE_TIME_SOON )
		MessageQ[i].window_timestamp = timestamp(MESSAGE_SOON_TIMESTAMP);
	else
		MessageQ[i].window_timestamp = timestamp(MESSAGE_ANYTIME_TIMESTAMP);		// make invalid

	MessageQ_num++;
	qsort(MessageQ, MAX_MESSAGE_Q, sizeof(message_q), message_queue_priority_compare);

	// Try to start it!
	// MWA -- called every frame from game loop
	//message_queue_process();
}

// function to return the persona index of the given ship.  If it isn't assigned, it will be
// in this function.  persona_type could be a wingman, Terran Command, or other generic ship
// type personas.  ship is the ship we should assign a persona to
int message_get_persona( ship *shipp )
{
	int i, ship_type, slist[MAX_PERSONAS], count;

	if ( shipp != NULL ) {
		// see if this ship has a persona
		if ( shipp->persona_index != -1 )
			return shipp->persona_index;

		// get the type of ship (i.e. support, fighter/bomber, etc)
		ship_type = Ship_info[shipp->ship_info_index].flags;

#if defined(MORE_SPECIES)
		int persona_needed;
		count = 0;

		if ( ship_type & (SIF_FIGHTER|SIF_BOMBER) )
		{
			persona_needed = PERSONA_FLAG_WINGMAN;
		} else if ( ship_type & SIF_SUPPORT ) 
		{
			persona_needed = PERSONA_FLAG_SUPPORT;
		}
		else 
		{
			persona_needed = PERSONA_FLAG_LARGE;
		}

		// first try to go for an unused persona
		for (i = 0; i < Num_personas; i++)
		{
			// this Persona is not our species - skip it
			if (Personas[i].species != Ship_info[shipp->ship_info_index].species)
				continue;

			// check the ship types, and don't try to assign those which don't type match
			if ( Personas[i].flags & persona_needed)
			{
				if (!(Personas[i].flags & PERSONA_FLAG_USED))
				{
					// if it hasn't been used - USE IT!
					Personas[i].flags |= PERSONA_FLAG_USED;
					return i;
				}
				else
				{
					// otherwise add it to our list of valid options to randomly select from
					slist[count] = i;
					count++;
				}
			}
		}

		// we didn't find an unused one - so we randomly select one
		if(count != 0)
		{
			i = (rand() % count);
			i = slist[i];
		}
		// RT Protect against count being zero
		else
			i = slist[0];

		return i;

#else
		//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
		// Old Volition code for personnas
		//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 


		// shorcut for Vasudan personas.  All vasudan fighters/bombers use the same persona.  All Vasudan
		// large ships will use the same persona
		if ( Ship_info[shipp->ship_info_index].species == SPECIES_VASUDAN ) {
			int persona_needed;

			if ( ship_type & (SIF_FIGHTER|SIF_BOMBER) ) {
				persona_needed = PERSONA_FLAG_WINGMAN;
			} else if ( ship_type & SIF_SUPPORT ) {
				persona_needed = PERSONA_FLAG_SUPPORT;
			} else {
				persona_needed = PERSONA_FLAG_LARGE;
			}

			// iternate through the persona list finding the one that we need
			for ( i = 0; i < Num_personas; i++ ) {
				if ( (Personas[i].flags & persona_needed) && (Personas[i].flags & PERSONA_FLAG_VASUDAN) ) {
					nprintf(("messaging", "assigning vasudan persona %s to %s\n", Personas[i].name, shipp->ship_name));
					return i;
				}
			}

			// make support personas use the terran persona by not returning here when looking for 
			// Vasudan persona
			if ( persona_needed != PERSONA_FLAG_SUPPORT )
				return -1;			// shouldn't get here eventually, but return -1 for now to deal with missing persona
		}

		// iterate through the persona list looking for one not used.  Look at the type of persona
		// and try to determine appropriate personas to use.
		for ( i = 0; i < Num_personas; i++ ) {

			// if this is a vasudan persona -- skip it
			if ( Personas[i].flags & PERSONA_FLAG_VASUDAN )
				continue;

			// check the ship types, and don't try to assign those which don't type match
			if ( (ship_type & SIF_SUPPORT) && !(Personas[i].flags & PERSONA_FLAG_SUPPORT) )
				continue;
			else if ( (ship_type & (SIF_FIGHTER|SIF_BOMBER)) && !(Personas[i].flags & PERSONA_FLAG_WINGMAN) )
				continue;
			else if ( !(ship_type & (SIF_FIGHTER|SIF_BOMBER|SIF_SUPPORT)) && !(Personas[i].flags & PERSONA_FLAG_LARGE) )
				continue;

			if ( !(Personas[i].flags & PERSONA_FLAG_USED) ) {
				nprintf(("messaging", "assigning persona %s to %s\n", Personas[i].name, shipp->ship_name));
				Personas[i].flags |= PERSONA_FLAG_USED;
				return i;
			}
		}

		// grab a random one, and reuse it (staying within type specifications)
		count = 0;
		for ( i = 0; i < Num_personas; i++ ) {

			// see if ship meets our criterea
			if ( (ship_type & SIF_SUPPORT) && !(Personas[i].flags & PERSONA_FLAG_SUPPORT) )
				continue;
			else if ( (ship_type & (SIF_FIGHTER|SIF_BOMBER)) && !(Personas[i].flags & PERSONA_FLAG_WINGMAN) )
				continue;
			else if ( !(ship_type & (SIF_FIGHTER|SIF_BOMBER|SIF_SUPPORT)) && !(Personas[i].flags & PERSONA_FLAG_LARGE) )
				continue;
			else if ( Personas[i].flags & PERSONA_FLAG_VASUDAN )		// don't use any vasudan persona
				continue;

			slist[count] = i;
			count++;
		}

		// couldn't find appropriate persona type
		if ( count == 0 )
			return -1;

		// now get a random one from the list
		i = (rand() % count);
		i = slist[i];
			
		nprintf(("messaging", "Couldn't find a new persona for ship %s, reusing persona %s\n", shipp->ship_name, Personas[i].name));

		return i;

		// +-+-+-+-+-+ End old V Code +-+-+-+-+-+ 
#endif
	}

	// for now -- we don't support other types of personas (non-wingman personas)
	Int3();
	return 0;
}

#ifndef NO_NETWORK
// given a message id#, should it be filtered for me?
int message_filter_multi(int id)
{
	// not multiplayer
	if(!(Game_mode & GM_MULTIPLAYER)){
		return 0;
	}

	// bogus
	if((id < 0) || (id >= Num_messages)){
		mprintf(("Filtering bogus mission message!\n"));
		return 1;
	}

	// builtin messages
	if(id < Num_builtin_messages){
	}
	// mission-specific messages
	else {
		// not team filtered
		if(Messages[id].multi_team < 0){
			return 0;
		}

		// not TvT
		if(!(Netgame.type_flags & NG_TYPE_TEAM)){
			return 0;
		}

		// is this for my team?
		if((Net_player != NULL) && (Net_player->p_info.team != Messages[id].multi_team)){
			mprintf(("Filtering team-based mission message!\n"));
			return 1;
		}
	}		
	
	return 0;
}
#endif  // ifndef NO_NETWORK

// send_unique_to_player sends a mission unique (specific) message to the player (possibly a multiplayer
// person).  These messages are *not* the builtin messages
void message_send_unique_to_player( char *id, void *data, int m_source, int priority, int group, int delay )
{
	int i, source;
	char *who_from;

	source = 0;
	who_from = NULL;
	for (i=0; i<Num_messages; i++) {
		// find the message
		if ( !stricmp(id, Messages[i].name) ) {

			// if the ship pointer and special_who are both NULL then this is from generic "Terran Command"
			// if the ship is NULL and special_who is not NULL, then this is from special_who
			// otherwise, message is from ship.
			if ( m_source == MESSAGE_SOURCE_COMMAND ) {
				who_from = TERRAN_COMMAND;
				source = HUD_SOURCE_TERRAN_CMD;
			} else if ( m_source == MESSAGE_SOURCE_SPECIAL ) {
				who_from = (char *)data;
				source = HUD_SOURCE_TERRAN_CMD;
			} else if ( m_source == MESSAGE_SOURCE_WINGMAN ) {
				int m_persona, ship_index;

				// find a wingman with the same persona as this message.  If the message's persona doesn't
				// exist, we will use Terran command
				m_persona = Messages[i].persona_index;
				if ( m_persona == -1 ) {
					mprintf(("Warning:  Message %d has no persona assigned.\n", i));
				}

				// get a ship						
				ship_index = ship_get_random_player_wing_ship( SHIP_GET_NO_PLAYERS, 0.0f, m_persona, 1, Messages[i].multi_team);

				// if the ship_index is -1, then make the message come from Terran command
				if ( ship_index == -1 ) {
					who_from = TERRAN_COMMAND;
					source = HUD_SOURCE_TERRAN_CMD;
				} else {
					who_from = Ships[ship_index].ship_name;
					source = HUD_get_team_source(Ships[ship_index].team);
				}

			} else if ( m_source == MESSAGE_SOURCE_SHIP ) {
				ship *shipp;

				shipp = (ship *)data;
				who_from = shipp->ship_name;
				source = HUD_get_team_source(shipp->team);

				// be sure that this ship can actually send a message!!! (i.e. not-not-flyable -- get it!)
				Assert( !(Ship_info[shipp->ship_info_index].flags & SIF_NOT_FLYABLE) );		// get allender or alan
			}

			// not multiplayer or this message is for me, then queue it
			// if ( !(Game_mode & GM_MULTIPLAYER) || ((multi_target == -1) || (multi_target == MY_NET_PLAYER_NUM)) ){

			// maybe filter it out altogether
#ifndef NO_NETWORK
			if(!message_filter_multi(i))
#endif
			{
				message_queue_message( i, priority, MESSAGE_TIME_ANYTIME, who_from, source, group, delay );
			}

			// record to the demo if necessary
			if(Game_mode & GM_DEMO_RECORD){
				demo_POST_unique_message(id, who_from, m_source, priority);
			}
			// }

#ifndef NO_NETWORK
			// send a message packet to a player if destined for everyone or only a specific person
			if ( MULTIPLAYER_MASTER ){
				send_mission_message_packet( i, who_from, priority, MESSAGE_TIME_SOON, source, -1, -1, -1);
			}			
#endif

			return;		// all done with displaying		
		}
	}
	nprintf (("messaging", "Couldn't find message id %s to send to player!\n", id ));
}

// send builtin_to_player sends a message (from messages.tbl) to the player.  These messages are
// the generic infomrational type messages.  The have priorities like misison specific messages,
// and use a timing to tell how long we should wait before playing this message
void message_send_builtin_to_player( int type, ship *shipp, int priority, int timing, int group, int delay, int multi_target, int multi_team_filter )
{
	int i, persona_index;
	int source;	

	// if we aren't showing builtin msgs, bail
	if (The_mission.flags & MISSION_FLAG_NO_BUILTIN_MSGS) {
		return;
	}

	// see if there is a persona assigned to this ship.  If not, then try to assign one!!!
	if ( shipp ) {
		if ( shipp->persona_index == -1 ){
			shipp->persona_index = message_get_persona( shipp );
		}

		persona_index = shipp->persona_index;
		if ( persona_index == -1 ) {
			nprintf(("messaging", "Couldn't find persona for %s\n", shipp->ship_name ));
		}		

		// be sure that this ship can actually send a message!!! (i.e. not-not-flyable -- get it!)
		Assert( !(Ship_info[shipp->ship_info_index].flags & SIF_NOT_FLYABLE) );		// get allender or alan
	} else {
		persona_index = Command_persona;				// use the terran command persona
	}

	// try to find a builtin message with the given type for the given persona
	// make a loop out of this routne since we may try to play a message in the wrong
	// persona if we can't find the right message for the given persona
	do {
		for ( i = 0; i < Num_builtin_messages; i++ ) {
			char *name, *who_from;

			name = Builtin_message_types[type];

			// see if the have the type of message
			if ( stricmp(Messages[i].name, name) ){
				continue;
			}

			// must have the correct persona.  persona_index of -1 means find the first
			// message possibly of the correct type
			if ( (persona_index != -1 ) && (Messages[i].persona_index != persona_index) ){
				continue;
			}

			// get who this message is from -- kind of a hack since we assume Terran Command in the
			// absense of a ship.  This will be fixed later
			if ( shipp ) {
				source = HUD_get_team_source( shipp->team );
				who_from = shipp->ship_name;
			} else {
				source = HUD_SOURCE_TERRAN_CMD;
				who_from = TERRAN_COMMAND;
			}

			// maybe change the who from here for special rearm cases (always seems like that is the case :-) )
			if ( !stricmp(who_from, TERRAN_COMMAND) && (type == MESSAGE_REARM_ON_WAY) ){
				who_from = SUPPORT_NAME;
			}

			// determine what we should actually do with this dang message.  In multiplayer, we must
			// deal with the fact that this message might not get played on my machine if I am a server

#ifndef NO_NETWORK
			// not multiplayer or this message is for me, then queue it
			if ( !(Game_mode & GM_MULTIPLAYER) || ((multi_target == -1) || (multi_target == MY_NET_PLAYER_NUM)) ){

				// if this filter matches mine
				if( (multi_team_filter < 0) || !(Netgame.type_flags & NG_TYPE_TEAM) || ((Net_player != NULL) && (Net_player->p_info.team == multi_team_filter)) ){
#endif
					message_queue_message( i, priority, timing, who_from, source, group, delay, type );

					// post a builtin message
					if(Game_mode & GM_DEMO_RECORD){
						demo_POST_builtin_message(type, shipp, priority, timing);
					}
#ifndef NO_NETWORK
				}
			}
#endif

#ifndef NO_NETWORK
			// send a message packet to a player if destined for everyone or only a specific person
			if ( MULTIPLAYER_MASTER ) {
				// only send a message if it is of a particular type
				if(multi_target == -1){
					if(multi_message_should_broadcast(type)){				
						send_mission_message_packet( i, who_from, priority, timing, source, type, -1, multi_team_filter );
					}
				} else {
					send_mission_message_packet( i, who_from, priority, timing, source, type, multi_target, multi_team_filter );
				}
			}
#endif

			return;		// all done with displaying
		}

		if ( persona_index >= 0 ) {
			nprintf(("messaging", "Couldn't find builtin message %s for persona %d\n", Builtin_message_types[type], persona_index ));
			nprintf(("messaging", "looking for message for any persona\n"));
			persona_index = -1;
		} else {
			persona_index = -999;		// used here and the next line only -- hard code bad, but I'm lazy
		}
	} while ( persona_index != -999 );
}

// message_is_playing()
//
// Return the Message_playing flag.  Message_playing is local to MissionMessage.cpp, but
// this info is needed by code in HUDsquadmsg.cpp
//
int message_is_playing()
{
	return Num_messages_playing?1:0;
}

// Functions below pertain only to personas!!!!

// given a character string, try to find the persona index
int message_persona_name_lookup( char *name )
{
	int i;

	for (i = 0; i < Num_personas; i++ ) {
		if ( !stricmp(Personas[i].name, name) )
			return i;
	}

	return -1;
}


// Blank out portions of the audio playback for the sound identified by Message_wave
// This works by using the same Distort_pattern[][] that was used to distort the associated text
void message_maybe_distort()
{
	int i;
	int was_muted;

	if ( Num_messages_playing == 0 )
		return;
	
	for ( i = 0; i < Num_messages_playing; i++ ) {
		if ( !snd_is_playing(Playing_messages[i].wave) )
			return;
	}

	// distort the number of voices currently playing
	for ( i = 0; i < Num_messages_playing; i++ ) {
		Assert(Playing_messages[i].wave >= 0 );

		was_muted = 0;

		// added check to see if EMP effect was active
		// 8/24/98 - DB
		if ( (hud_communications_state(Player_ship) != COMM_OK) || emp_active_local() ) {
			was_muted = Message_wave_muted;
			if ( timestamp_elapsed(Next_mute_time) ) {
				Next_mute_time = fl2i(Distort_patterns[Distort_num][Distort_next++] * Message_wave_duration);
				if ( Distort_next >= MAX_DISTORT_LEVELS )
					Distort_next = 0;

				Message_wave_muted ^= 1;
			}
		
			if ( Message_wave_muted ) {
				if ( !was_muted )
					snd_set_volume(Playing_messages[i].wave, 0.0f);
			} else {
				if ( was_muted )
					snd_set_volume(Playing_messages[i].wave, Master_sound_volume);
			}
		}
	}
}


// if the player communications systems are heavily damaged, distort incoming messages.
//
// first case: Message_wave_duration == 0 (this occurs when there is no associated voice playback)
//					Blank out random runs of characters in the message
//
// second case: Message_wave_duration > 0 (occurs when voice playback accompainies message)
//					 Blank out portions of the sound based on Distort_num, this this is that same
//					 data that will be used to blank out portions of the audio playback
//
void message_maybe_distort_text(char *text)
{
	int i, j, len, run, curr_offset, voice_duration, next_distort;

	if ( (hud_communications_state(Player_ship) == COMM_OK) && !emp_active_local() ) { 
		return;
	}

	len = strlen(text);
	if ( Message_wave_duration == 0 ) {
		next_distort = 5+myrand()%5;
		for ( i = 0; i < len; i++ ) {
			if ( i == next_distort ) {
				run = 3+myrand()%5;
				if ( i+run > len )
					run = len-i;
				for ( j = 0; j < run; j++) {
					text[i++] = '-';
					if ( i >= len )
						break;
				}
				next_distort = i + (5+myrand()%5);
			}
		}
		return;
	}

	voice_duration = Message_wave_duration;

	// distort text
	Distort_num = myrand()%MAX_DISTORT_PATTERNS;
	Distort_next = 0;
	curr_offset = 0;
	while (voice_duration > 0) {
		run = fl2i(Distort_patterns[Distort_num][Distort_next] * len);
		if (Distort_next & 1) {
			for ( i = curr_offset; i < min(len, curr_offset+run); i++ ) {
				if ( text[i] != ' ' ) 
					text[i] = '-';
			}
			curr_offset = i;
			if ( i >= len )
				break;
		} else {
			curr_offset += run;
		}

		voice_duration -= fl2i(Distort_patterns[Distort_num][Distort_next]*Message_wave_duration);
		Distort_next++;
		if ( Distort_next >= MAX_DISTORT_LEVELS )
			Distort_next = 0;
	};
	
	Distort_next = 0;
}

// return 1 if a talking head animation is playing, otherwise return 0
int message_anim_is_playing()
{
	int i;

	for (i = 0; i < Num_messages_playing; i++ ) {
		if ( (Playing_messages[i].anim != NULL) && anim_playing(Playing_messages[i].anim) )
			return 1;
	}

	return 0;
}

// Load mission messages (this is called by the level paging code when running with low memory)
void message_pagein_mission_messages()
{
	int i;
	
	mprintf(("Paging in mission messages\n"));

	if (Num_messages <= Num_builtin_messages) {
		return;
	}

	char *sound_filename;

	for (i=Num_builtin_messages; i<Num_messages; i++) {
		if (Messages[i].wave_info.index != -1) {
			sound_filename = Message_waves[Messages[i].wave_info.index].name;
			message_load_wave(Messages[i].wave_info.index, sound_filename);
		}
	}
}
