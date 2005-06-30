/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Gamesnd/EventMusic.cpp $
 * $Revision: 2.21 $
 * $Date: 2005-06-30 01:48:52 $
 * $Author: Goober5000 $
 *
 * C module for high-level control of event driven music 
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.20  2005/04/25 00:22:34  wmcoolmon
 * Added parse_sound; replaced Assert() with an if() (The latter may not be a good idea, but it keeps missions from being un-debuggable)
 *
 * Revision 2.19  2005/03/27 12:28:32  Goober5000
 * clarified max hull/shield strength names and added ship guardian thresholds
 * --Goober5000
 *
 * Revision 2.18  2005/02/23 05:05:39  taylor
 * compiler warning fixes (for MSVC++ 6)
 * have the warp effect only load as many LODs as will get used
 * head off strange bug in release when corrupt soundtrack number gets used
 *    (will still Assert in debug)
 * don't ever try and save a campaign savefile in multi or standalone modes
 * first try at 32bit->16bit color conversion for TGA code (for TGA only ship textures)
 *
 * Revision 2.17  2005/01/18 01:14:17  wmcoolmon
 * OGG fixes, ship selection fixes
 *
 * Revision 2.16  2004/12/25 16:42:59  wmcoolmon
 * Fix to modular tables workaround with Fs2NetD
 *
 * Revision 2.15  2004/12/11 09:03:08  wmcoolmon
 * Changed music modular table ending to *-mus.tbm, in case a miscellaneous modular table is needed.
 *
 * Revision 2.14  2004/07/26 20:47:30  Kazan
 * remove MCD complete
 *
 * Revision 2.13  2004/07/17 09:26:00  taylor
 * add CF_SORT_REVERSE to real sort routine, makes CF_SORT_TIME work again
 *
 * Revision 2.12  2004/07/12 16:32:47  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.11  2004/06/18 04:59:54  wmcoolmon
 * Only used weapons paged in instead of all, fixed music box in FRED, sound quality settable with SoundSampleRate and SoundSampleBits registry values
 *
 * Revision 2.10  2004/05/28 23:37:30  wmcoolmon
 * Modular music tables
 *
 * Revision 2.9  2004/03/05 09:02:00  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.8  2004/02/08 03:35:08  Goober5000
 * fixed a small bug in FS1-compatible music
 * --Goober5000
 *
 * Revision 2.7  2004/01/30 07:39:06  Goober5000
 * whew - I just went through all the code I ever added (or at least, that I could
 * find that I commented with a Goober5000 tag) and added a bunch of Asserts
 * and error-checking
 * --Goober5000
 *
 * Revision 2.6  2003/10/14 16:47:34  Goober5000
 * tweaked a thingy with FS1-style music to make deciding the style less of a hack
 * --Goober5000
 *
 * Revision 2.5  2003/08/25 04:46:53  Goober5000
 * added extra code to enable playing of FS1 music tracks
 * --Goober5000
 *
 * Revision 2.4  2003/04/29 01:03:23  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.3  2003/03/22 06:06:22  Goober5000
 * changed event_sexp_change_music to event_change_sexp_soundtrack
 * --Goober5000
 *
 * Revision 2.2  2003/01/25 04:17:39  Goober5000
 * added change-music sexp and bumped MAX_SOUNDTRACKS from 10 to 25
 * --Ian Warfield
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 14    8/19/99 9:41a Alanl
 * don't play victory 2 music if there are no goals in the mission
 * 
 * 13    8/11/99 5:33p Jefff
 * added 3rd debrief music track
 * 
 * 12    8/01/99 2:11p Alanl
 * make hull value to play more intense battle music a constant
 * 
 * 11    8/01/99 2:06p Alanl
 * tweak battle music track switching
 * 
 * 10    7/25/99 9:57p Alanl
 * change battle music track selection after enemy arrivals to make battle
 * tracks less repetitive
 * 
 * 9     7/19/99 10:13p Andsager
 * Tie in hud_targeting to  hostile_ships_present() used to determine next
 * song.
 * 
 * 8     6/24/99 10:47p Alanl
 * loop battle between tracks 2 and 3 if hull integrity < 70%
 * 
 * 7     6/21/99 1:34p Alanl
 * event music tweaks
 * 
 * 6     6/20/99 12:06a Alanl
 * new event music changes
 * 
 * 5     11/23/98 11:55a Johnson
 * return -1 if a score isn't found
 * 
 * 4     11/20/98 4:08p Dave
 * Fixed flak effect in multiplayer.
 * 
 * 3     10/23/98 3:51p Dave
 * Full support for tstrings.tbl and foreign languages. All that remains
 * is to make it active in Fred.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 107   6/09/98 5:15p Lawrance
 * French/German localization
 * 
 * 106   6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 105   5/24/98 5:28p Dan
 * let event_music_level_init() over-ride Event_music_enabled
 * 
 * 104   5/24/98 4:42p Dan
 * AL: Fix several bugs related to pausing and enabling/disabling event
 * music
 * 
 * 103   5/23/98 3:17a Lawrance
 * Tweak how battle music gets restarted
 * 
 * 102   5/22/98 10:43a Lawrance
 * If mission doesn't have event music, don't choose random track
 * 
 * 101   5/21/98 6:56p Lawrance
 * Tweak how victory music plays
 * 
 * 100   5/21/98 2:47a Lawrance
 * Fix some problems with event music
 * 
 * 99    5/18/98 3:21p Mike
 * Don't play arrival and goals tracks in a training mission.
 * 
 * 98    5/04/98 3:15p Duncan
 * AL: remove bogus assert in event_music_player_respawn()
 * 
 * 97    5/03/98 1:54a Lawrance
 * Fix event music problems related to respawning
 * 
 * 96    4/03/98 12:56a Lawrance
 * Fix bug with music not starting in the first mission
 * 
 * 95    4/01/98 6:46p Lawrance
 * Lower default music volume 
 *
 * $NoKeywords: $
 */


#include "gamesnd/eventmusic.h"
#include "object/object.h"
#include "ship/ship.h"
#include "globalincs/linklist.h"
#include "mission/missionparse.h"
#include "io/timer.h"
#include "sound/audiostr.h"
#include "sound/sound.h"
#include "cmdline/cmdline.h"
#include "mission/missiongoals.h"
#include "localization/localize.h"
#include "parse/parselo.h"



#pragma optimize("", off)

#define DEFAULT_MASTER_EVENT_MUSIC_VOLUME	0.5f

#define HULL_VALUE_TO_PLAY_INTENSE_BATTLE_MUSIC 0.75f

////////////////////////////
// Globals
////////////////////////////
int Event_Music_battle_started = 0;
float Master_event_music_volume = DEFAULT_MASTER_EVENT_MUSIC_VOLUME;			// range is 0->1

// array that holds which soundtrack is used for which mission (data comes from music.tbl)
//int Mission_soundtracks[MAX_CAMPAIGN_MISSIONS];

// array that holds which briefing track to play for which mission.  Index into Spooled_music[][].
//int Mission_briefingmusic[MAX_CAMPAIGN_MISSIONS];

typedef struct tagSNDPATTERN {
	int default_next_pattern;	// Needed so the next_pattern member can be reset
	int next_pattern;				// Next pattern to play at loop time (can be same pattern)
	int default_loop_for;		// Needed so the loop_for variable can be reset
	int loop_for;					// Number of times to loop before switching to next pattern
	int handle;						// handle to open audio stream
	int force_pattern;			// flag to indicate that we want to not continue loop, but go to next_pattern
	int can_force;					// whether this pattern can be interrupted
	int samples_per_measure;		// number of bytes in a measure
	float num_measures;				// number of measures in wave file
}	SNDPATTERN;

SNDPATTERN	Patterns[MAX_PATTERNS];	// holds data on sections of a SoundTrack

// Holds filenames for the different sections of a soundtrack
SOUNDTRACK_INFO Soundtracks[MAX_SOUNDTRACKS];
int Num_soundtracks;
int Current_soundtrack_num;	// Active soundtrack for the current mission.. index into Soundtracks[]

#define PATTERN_DELAY	1000	// in ms
int Current_pattern = -1;		// currently playing part of track
int Pending_pattern = -1;
int Pattern_timer_id = 0;
int Current_nrml = SONG_NRML_1;

// File Globals
static int Num_enemy_arrivals;
static int Num_friendly_arrivals;

#define ARRIVAL_INTERVAL_TIMESTAMP  5000
#define BATTLE_CHECK_INTERVAL			15000
static int Battle_over_timestamp;
static int Mission_over_timestamp;
static int Victory2_music_played;
static int Next_arrival_timestamp;
static int Check_for_battle_music;
static int Current_soundtrack_fs1_style;

// stores the number of measures for the different patterns (data from music.tbl)
float	Pattern_num_measures[MAX_SOUNDTRACKS][MAX_PATTERNS];

// stores the number of bytes per measure (data from music.tbl)
int	Pattern_samples_per_measure[MAX_SOUNDTRACKS][MAX_PATTERNS];

char* Pattern_names[MAX_PATTERNS] =
{
//XSTR:OFF
	"NRML_1",	// Normal Song 1
	"AARV_1",	// Allied Arrival 1
	"EARV_1",	// Enemy Arrival 1
	"BTTL_1",	// Battle Song 1
	"BTTL_2",	// Battle Song 2
	"BTTL_3",	// Battle Song 3
	"AARV_2",	// Allied Arrival 2
	"EARV_2",	// Enemy Arrival 2
	"VICT_1",	// Victory Song 1
	"VICT_2",	// Victory Song 2
	"FAIL_1",	// Goal Failed 1
	"DEAD_1",	// Death Song 1
	"NRML_2",	// Normal Song 2
	"NRML_3"	// Normal Song 3
//XSTR:ON
};

char* Pattern_description[MAX_PATTERNS] =
{
//XSTR:OFF
	"normal 1",
	"friendly arrival 1",
	"enemy arrival 2",
	"battle 1",
	"battle 2",
	"battle 3",
	"friendly arrival 2",
	"enemey arrival 2",
	"victory 1",
	"victory 2",
	"goal failed 1",
	"death",
	"normal 2",
	"normal 3"
//XSTR:ON
};

int Pattern_loop_for[MAX_PATTERNS] =
{
	1,	// Normal Song 1
	1,	// Allied Arrival 1
	1,	// Enemy Arrival 1
	1,	// Battle Song 1
	1,	// Battle Song 2
	1,	// Battle Song 3
	1,	// Allied Arrival 2
	1,	// Enemy Arrival 2
	1,	// Victory Song 1
	1,	// Victory Song 2
	1,	// Goal Failed 1
	1, 	// Death Song 1
	1,	// Normal Song 2
	1	// Normal Song 3
};

int Pattern_default_next[MAX_PATTERNS] =
{
	SONG_NRML_1,	// NRML_1 progresses to NRML_1 by default
	SONG_NRML_1,	// AARV_1 progresses to NRML_1 by default
	SONG_BTTL_1,	// EARV_1 progresses to BTTL_1 by default
	SONG_BTTL_2,	// BTTL_1 progresses to BTTL_2 by default
	SONG_BTTL_3,	// BTTL_2 progresses to BTTL_3 by default
	SONG_BTTL_1,	// BTTL_3 progresses to BTTL_1 by default
	SONG_BTTL_2,	// AARV_2 progresses to BTTL_2 by default
	SONG_BTTL_3,	// EARV_2 progresses to BTTL_3 by default
	SONG_NRML_1,	// VICT_1 progresses to NRML_1 by default
	SONG_NRML_1,	// VICT_2 progresses to NRML_1 by default
	SONG_NRML_1,	//	FAIL_1 progresses to NRML_1 by default
	-1,					// no music plays after dead
	SONG_NRML_1,	// NRML_2 progresses to NRML_1 by default (but we intercept this)
	SONG_NRML_1		// NRML_3 progresses to NRML_1 by default (but we intercept this)
};


// Certain patterns can be interrupted (such as the long-playing NRML and BTTL tracks).  
// Other shorter tracks (such as arrivals) play their entire duration.
int Pattern_can_force[MAX_PATTERNS] =
{
	TRUE,		// NRML_1 
	FALSE,	// AARV_1 
	FALSE,	// EARV_1 
	TRUE,		// BTTL_1
	TRUE,		// BTTL_2 
	TRUE,		// BTTL_3
	FALSE,	// AARV_2 
	FALSE,	// EARV_2 
	FALSE,	// VICT_1 
	TRUE,		// VICT_2 
	FALSE,	// FAIL_1
	TRUE,		// DEAD_1
	TRUE,		// NRML_2
	TRUE		// NRML_3
};


int Event_music_enabled = TRUE;
static int Event_music_inited = FALSE;
static int Event_music_level_inited = FALSE;
static int Event_music_begun = FALSE;

// forward function declarations
int hostile_ships_present();
int hostile_ships_to_arrive();
extern int hud_target_invalid_awacs(object *objp);

// Holds file names of spooled music that is played at menus, briefings, credits etc.
// Indexed into by a #define enumeration of the different kinds of spooled music
menu_music Spooled_music[MAX_SPOOLED_MUSIC];
int Num_music_files;				// Number of spooled music files

// Array that holds indicies into Spooled_music[], these specify which music is played in breifing/debriefing
int Mission_music[NUM_SCORES];	

// Goober5000
int maybe_cycle_nrml(int pattern)
{
	if ((pattern == SONG_NRML_1) || (pattern == SONG_NRML_2) || (pattern == SONG_NRML_3))
	{
		Current_nrml = maybe_get_next_nrml_no_cycle(Current_nrml);
		pattern = Current_nrml;
	}

	return pattern;
}
// Goober5000
int maybe_get_next_nrml_no_cycle(int pattern)
{
	switch (pattern)
	{
		case SONG_NRML_1:
			if (Patterns[SONG_NRML_2].handle == -1)
				return SONG_NRML_1;
			else
				return SONG_NRML_2;

		case SONG_NRML_2:
			if (Patterns[SONG_NRML_3].handle == -1)
				return SONG_NRML_1;
			else
				return SONG_NRML_3;

		case SONG_NRML_3:
			return SONG_NRML_1;

		default:
			return pattern;
	}
}

// -------------------------------------------------------------------------------------------------
// event_music_init() 
//
// Called once at game start-up to parse music.tbl and set some state variables
//
void event_music_init()
{
	if(!Fred_running)
	{
		if ( snd_is_inited() == FALSE ) {
			Event_music_enabled = FALSE;
			return;
		}

		if ( Cmdline_freespace_no_music ) {
			return;
		}

		if ( Event_music_inited == TRUE )
			return;
	}

	//MUST be called before parsing stuffzors.
	Num_music_files = 0;
	Num_soundtracks = 0;		// Global
	event_music_reset_choices();

	//Do teh parsing
	event_music_parse_musictbl("music.tbl", false);

	char tbl_file_arr[MAX_TBL_PARTS][MAX_FILENAME_LEN];
	char *tbl_file_names[MAX_TBL_PARTS];

	int num_files = cf_get_file_list_preallocated(MAX_TBL_PARTS, tbl_file_arr, tbl_file_names, CF_TYPE_TABLES, "*-mus.tbm", CF_SORT_REVERSE);
	for(int i = 0; i < num_files; i++)
	{
		//HACK HACK HACK
		modular_tables_loaded = true;
		strcat(tbl_file_names[i], ".tbm");
		event_music_parse_musictbl(tbl_file_names[i], true);
	}
	Event_music_inited = TRUE;
	Event_music_begun = FALSE;
}

// -------------------------------------------------------------------------------------------------
// event_music_close() 
//
// Called once at game end
//
void event_music_close()
{
	if ( Event_music_inited == FALSE )
		return;

	Event_music_inited = FALSE;
}

// -------------------------------------------------------------------------------------------------
// event_music_force_switch() 
//
// Performs a switch between patterns.  Sets the cutoff limit for the pattern that is being
// switch to.
//
void event_music_force_switch()
{
	if ( Event_music_enabled == FALSE )
		return;

	if ( Event_music_level_inited == FALSE )
		return;

	int new_pattern;
	Patterns[Current_pattern].loop_for = Patterns[Current_pattern].default_loop_for;

	new_pattern = Patterns[Current_pattern].next_pattern;

	// make sure we have a valid track to switch to
	if ( (new_pattern == SONG_BTTL_2) && (Patterns[SONG_BTTL_2].handle == -1) ) {
		new_pattern = SONG_BTTL_1;
	} else if ( (new_pattern == SONG_BTTL_3) && (Patterns[SONG_BTTL_3].handle == -1) ) {
		if ( (Current_pattern == SONG_BTTL_2) || (Patterns[SONG_BTTL_2].handle == -1) ) {
			new_pattern = SONG_BTTL_1;
		} else {
			new_pattern = SONG_BTTL_2;
		}
	} else {
		if (Current_pattern == SONG_BTTL_3 && new_pattern == SONG_BTTL_1) {
			// AL 06-24-99: maybe switch to battle 2 if hull is less than 70%
			if (Player_obj != NULL && Player_ship != NULL) {
				Assert(Player_ship->ship_info_index >= 0);
				Assert(Player_ship->ship_max_hull_strength != 0);
				float integrity = Player_obj->hull_strength / Player_ship->ship_max_hull_strength;
				if (integrity < HULL_VALUE_TO_PLAY_INTENSE_BATTLE_MUSIC) {
					new_pattern = SONG_BTTL_2;
				}
			}
		}
	}

	// set up switching
	new_pattern = maybe_cycle_nrml(new_pattern);

	if ( new_pattern == -1 ) {
		return;
	}

	if ( Patterns[new_pattern].num_measures == 0 )
		return;	// invalid pattern

	Assert(new_pattern >= 0 && new_pattern < MAX_PATTERNS);
	audiostream_play(Patterns[new_pattern].handle, Master_event_music_volume, 0);	// no looping
	audiostream_set_sample_cutoff(Patterns[new_pattern].handle, fl2i(Patterns[new_pattern].num_measures * Patterns[new_pattern].samples_per_measure) );
	Patterns[Current_pattern].next_pattern = Patterns[Current_pattern].default_next_pattern;
	Patterns[Current_pattern].force_pattern = FALSE;
	nprintf(("EVENTMUSIC", "EVENTMUSIC => switching to %s from %s\n", Pattern_names[new_pattern], Pattern_names[Current_pattern]));

	// actually switch the pattern
	Current_pattern = new_pattern;
}

// -------------------------------------------------------------------------------------------------
// event_music_do_frame() 
//
// Called once per game frame, to check for transitions of patterns (and to start the music off at
// the beginning).
//
void event_music_do_frame()
{
	if ( Event_music_level_inited == FALSE ) {
		return;
	}

	if ( Event_music_enabled == FALSE ) {
		return;
	}

	// start off the music delayed
	if ( timestamp_elapsed(Pattern_timer_id) ) {
		Pattern_timer_id = 0;
		Event_music_begun = TRUE;
		if ( Current_pattern != -1  && Patterns[Current_pattern].handle >= 0) {
			//WMC - removed in favor of if
			//Assert(Patterns[Current_pattern].handle >= 0 );
			audiostream_play(Patterns[Current_pattern].handle, Master_event_music_volume, 0);	// no looping
			audiostream_set_sample_cutoff(Patterns[Current_pattern].handle, fl2i(Patterns[Current_pattern].num_measures * Patterns[Current_pattern].samples_per_measure) );
		}
	}

	if ( Event_music_begun == FALSE ) {
		return;
	}

	if ( Current_pattern != -1 ) {
		SNDPATTERN *pat;
		pat = &Patterns[Current_pattern];

		// First case: switching to a different track since first track is almost at end
		if ( audiostream_done_reading(pat->handle) ) {
			event_music_force_switch();	
		}
		// Second case: looping back to start of same track since at the end
		else if ( !audiostream_is_playing(pat->handle) && !audiostream_is_paused(pat->handle) ) {
			audiostream_stop(pat->handle);	// stop current and rewind
			pat->loop_for--;
			if ( pat->loop_for > 0 ) {
				audiostream_play(pat->handle, Master_event_music_volume, 0);	// no looping
				audiostream_set_sample_cutoff(Patterns[Current_pattern].handle, fl2i(Patterns[Current_pattern].num_measures * Patterns[Current_pattern].samples_per_measure) );
			}
			else {
				event_music_force_switch();
			}
		}
		// Third case: switching to a different track by interruption
		else if ( (pat->force_pattern == TRUE && pat->can_force == TRUE) ) {
			int samples_streamed = audiostream_get_samples_committed(pat->handle);
			int measures_played = samples_streamed / pat->samples_per_measure;
			if ( measures_played < pat->num_measures ) {
				audiostream_set_sample_cutoff(pat->handle, pat->samples_per_measure * (measures_played+1) );
				pat->force_pattern = FALSE;
				pat->loop_for = 0;
			}
		}

		// We want to go back to NRML track music if all the hostiles have been 
		// destroyed, and we are still playing the battle music
		if ( Current_pattern == SONG_BTTL_1 || Current_pattern == SONG_BTTL_2 || Current_pattern == SONG_BTTL_3 ) {
			if ( timestamp_elapsed(Battle_over_timestamp) && Event_Music_battle_started == 1) {
				//Battle_over_timestamp = timestamp(BATTLE_CHECK_INTERVAL);
				if ( hostile_ships_present() == FALSE ) {
					if ( Patterns[Current_pattern].next_pattern != SONG_VICT_2 ) {
						Patterns[Current_pattern].next_pattern = SONG_NRML_1;
						Patterns[Current_pattern].force_pattern = TRUE;
						Event_Music_battle_started = 0;
					}
				}
			}
		}

		if (Event_Music_battle_started == 0) {
			if (Current_pattern == SONG_NRML_1 || Current_pattern == SONG_NRML_2 || Current_pattern == SONG_NRML_3 ) {
				if (timestamp_elapsed(Check_for_battle_music)) {
					Check_for_battle_music = timestamp(1000);
					if (hostile_ships_present() == TRUE) {
						Patterns[Current_pattern].next_pattern = SONG_BTTL_1;
						Patterns[Current_pattern].force_pattern = TRUE;
					}
				}
			}
		}

		if ( !Victory2_music_played ) {
			if ( timestamp_elapsed(Mission_over_timestamp) ) {
				Mission_over_timestamp = timestamp(BATTLE_CHECK_INTERVAL);
				if ( mission_goals_met() && (!hostile_ships_present()) ) {
					Patterns[Current_pattern].next_pattern = SONG_VICT_2;
					Patterns[Current_pattern].force_pattern = TRUE;
					Victory2_music_played = 1;
				}
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------
// event_music_level_init() 
//
// Called at the start of a mission (level).  Sets up the pattern data, and kicks off the
// first track to play().
//
// input:	force_soundtrack	=>		OPTIONAL parameter (default value -1)
//												forces the soundtrack to ignore the music.tbl assignment
//
void event_music_level_init(int force_soundtrack)
{
	int					i;
	SOUNDTRACK_INFO	*strack;

	if ( Cmdline_freespace_no_music ) {
		return;
	}

	if ( !audiostream_is_inited() )
		return;

	if ( Event_music_level_inited == TRUE )
		return;

	Current_pattern = -1;
	Current_nrml = SONG_NRML_1;

	if ( Event_music_inited == FALSE )
		return;


	if ( force_soundtrack != -1 ) {
		Current_soundtrack_num = force_soundtrack;
	}

	if ( Current_soundtrack_num < 0 ) {
		return;
/*
		// okay, assign a random soundtrack if one exists
		if ( Num_soundtracks > 0 ) {
			Current_soundtrack_num = rand()%Num_soundtracks;
			nprintf(("EVENTMUSIC", "EVENTMUSIC ==> Picking random event music soundtrack: %s\n", Soundtracks[Current_soundtrack_num].name));
		} else {
			return;
		}
*/
	}

	Assert(Current_soundtrack_num >= 0 && Current_soundtrack_num < Num_soundtracks);

	if (Current_soundtrack_num < 0 || Current_soundtrack_num > Num_soundtracks)
		return;

	strack = &Soundtracks[Current_soundtrack_num];

	// open the pattern files, and get ready to play them
	// Goober5000 - changed from strack->num_patterns to MAX_PATTERNS so that *all*
	// patterns would be checked; the behavior is the same because they were previously
	// set to none.wav in event_music_parse_musictbl, but this change was needed because
	// the NRML_2 and NRML_3 at the end of the pattern array kept getting spurious music
	// tracks because their patterns weren't -1
	for ( i = 0; i < MAX_PATTERNS; i++ ) {
		if ( !strnicmp(strack->pattern_fnames[i], NOX("none.wav"), 4) ) {
			Patterns[i].handle = -1;	
			continue;
		}

		Patterns[i].handle = audiostream_open( strack->pattern_fnames[i], ASF_EVENTMUSIC );

		if ( Patterns[i].handle >= 0 ) {
			Event_music_level_inited = TRUE;
			Event_music_enabled = TRUE;
		}

		Patterns[i].next_pattern = Pattern_default_next[i];
		Patterns[i].default_next_pattern = Pattern_default_next[i];
		Patterns[i].loop_for = Pattern_loop_for[i];
		Patterns[i].default_loop_for = Pattern_loop_for[i];
		Patterns[i].force_pattern = FALSE;
		Patterns[i].can_force = Pattern_can_force[i];
		Patterns[i].samples_per_measure = Pattern_samples_per_measure[Current_soundtrack_num][i];
		Patterns[i].num_measures = Pattern_num_measures[Current_soundtrack_num][i];
	}

	Num_enemy_arrivals = 0;
	Num_friendly_arrivals = 0;
	Battle_over_timestamp = timestamp(BATTLE_CHECK_INTERVAL);
	Mission_over_timestamp = timestamp(BATTLE_CHECK_INTERVAL);
	Next_arrival_timestamp = timestamp(1);
	Victory2_music_played = 0;
	Check_for_battle_music = 0;

	// Goober5000 - is this FS1 style?
	Current_soundtrack_fs1_style = stristr(Soundtracks[Current_soundtrack_num].name, "fs1") ? 1 : 0;

	if ( Event_music_level_inited ) {
		if ( force_soundtrack != -1 )  {
			event_music_first_pattern();
		}
	}
}

// -------------------------------------------------------------------------------------------------
// event_music_first_pattern() 
//
// Picks the first pattern to play, based on whether the battle has started.  Delay start
// by PATTERN_DELAY
//
void event_music_first_pattern()
{
	if ( Event_music_inited == FALSE ) {
		return;
	}

	if ( Event_music_enabled == FALSE ) {
		return;
	}

	if ( Event_music_level_inited == FALSE ) {
		event_music_level_init();
	}

	if ( Event_music_level_inited == FALSE ) {
		return;
	}

	if ( Event_music_begun == TRUE ) {
		return;
	}

	if ( Current_pattern != -1 ) {
		if (  audiostream_is_playing(Patterns[Current_pattern].handle) )
			audiostream_stop( Patterns[Current_pattern].handle );
	}

	Pattern_timer_id = 2000;	// start music delay
	
	Event_music_begun = FALSE;
	if ( Event_Music_battle_started == TRUE ) {
		Current_pattern = SONG_BTTL_1;
	}
	else {
		Current_pattern = SONG_NRML_1;
	}
}

// -------------------------------------------------------------------------------------------------
// event_music_level_close() 
//
// Called at the end of each mission (level).  Stops any playing patterns by fading them out.
//
void event_music_level_close()
{
	int i;

	if ( Event_music_level_inited == FALSE )
		return;

	if ( Current_soundtrack_num >= 0 && Current_soundtrack_num < MAX_SOUNDTRACKS ) {
		SOUNDTRACK_INFO *strack;
   
		Assert( Current_soundtrack_num >= 0 && Current_soundtrack_num < MAX_SOUNDTRACKS );
		strack = &Soundtracks[Current_soundtrack_num];

		// close the pattern files
		for ( i = 0; i < strack->num_patterns; i++ ) {
			if ( i == Current_pattern ) {
				if (  audiostream_is_playing(Patterns[Current_pattern].handle) )
					audiostream_close_file( Patterns[i].handle );
				else
					audiostream_close_file( Patterns[i].handle, 0 );
			}
			else
				audiostream_close_file( Patterns[i].handle, 0 );
		}
	} else {
		// close em all down then
		audiostream_close_all(0);
	}

	Current_pattern = -1;
	Event_music_level_inited = FALSE;
	Event_Music_battle_started = FALSE;
	Event_music_enabled = 0;
	Event_music_begun = FALSE;
}

// -------------------------------------------------------------------------------------------------
// event_music_battle_start() 
//
// Start the battle music.  If the music is already started before, do nothing.
//
int event_music_battle_start()
{	
	if ( !hostile_ships_present() ) {
		return 0;
	}

	//	No special tracks in training.
	if ( The_mission.game_type & MISSION_TYPE_TRAINING )
		return -1;

	// Check to see if we've already started off the battle song
	if ( Event_Music_battle_started == 1 ) {
		return 0;
	}

	if ( Event_music_enabled == FALSE )
		return -1;

	if ( Event_music_level_inited == FALSE )
		return -1;

	if ( Current_pattern == SONG_BTTL_1 )
		return 0;	// already playing

	if ( Current_pattern == SONG_DEAD_1 )
		return 0;	// death is the last song to play

	if ( Current_pattern != -1 ) {
		Patterns[Current_pattern].next_pattern = SONG_BTTL_1;
		Patterns[Current_pattern].force_pattern = TRUE;
	}

	Event_Music_battle_started = 1;	// keep track of this state though, need on restore
	Battle_over_timestamp = timestamp(BATTLE_CHECK_INTERVAL);

	return 0;
}

// -------------------------------------------------------------------------------------------------
// event_music_enemy_arrival() 
//
// An enemy has arrived, play an enemy arrival pattern.
//
int event_music_enemy_arrival()
{
	if ( Event_music_enabled == FALSE ) {
		return -1;
	}

	if ( Event_music_level_inited == FALSE ) {
		return -1;
	}

	int next_pattern;
	if ( Event_Music_battle_started == TRUE ) {
		next_pattern = SONG_EARV_2;
	}
	else {
		next_pattern = SONG_EARV_1;
	}

	if ( Current_pattern == maybe_get_next_nrml_no_cycle(next_pattern) )
		return 0;	// already playing

	if ( Current_pattern == SONG_DEAD_1 )
		return 0;	// death is the last song to play

	if ( (Current_pattern == SONG_VICT_1) || (Current_pattern == SONG_VICT_1) )
		return 0;

	if ( Patterns[Current_pattern].next_pattern != Patterns[Current_pattern].default_next_pattern )
		return 0;	// don't squash a pending pattern

	Num_enemy_arrivals++;

	// AL 7-25-99: If hull is less than 70% then switch to battle 2 or 3, otherwise switch to 1 or 2
	bool play_intense_battle_music = false;
	if (Player_obj != NULL && Player_ship != NULL) {
		Assert(Player_ship->ship_info_index >= 0);
		Assert(Player_ship->ship_max_hull_strength != 0);
		float integrity = Player_obj->hull_strength / Player_ship->ship_max_hull_strength;
		if (integrity < HULL_VALUE_TO_PLAY_INTENSE_BATTLE_MUSIC) {
			play_intense_battle_music = true;
		}
	}

	if (play_intense_battle_music == true) {
		if (Current_pattern == SONG_BTTL_2) {
			Patterns[next_pattern].next_pattern = SONG_BTTL_3;
		} else {
			Patterns[next_pattern].next_pattern = SONG_BTTL_2;
		}
	} else {
		if (Current_pattern == SONG_BTTL_1) {
			Patterns[next_pattern].next_pattern = SONG_BTTL_2;
		} else if (Current_pattern == SONG_BTTL_2) {
			Patterns[next_pattern].next_pattern = SONG_BTTL_3;
		} else {
			Patterns[next_pattern].next_pattern = SONG_BTTL_1;
		}
	}

	/*
	// AL 11-03-97:
	// Alternate between BTTL_2 and BTTL_3 following enemy arrivals
	if ( Num_enemy_arrivals & 1 ) {
		Patterns[next_pattern].next_pattern = SONG_BTTL_2;
	} else {
		if ( Patterns[SONG_BTTL_3].handle != -1 ) {
			Patterns[next_pattern].next_pattern = SONG_BTTL_3;
		} else {
			Patterns[next_pattern].next_pattern = SONG_BTTL_2;
		}
	}
	*/

	if ( Current_pattern != -1 ) {
		Patterns[Current_pattern].next_pattern = next_pattern;
		Patterns[Current_pattern].force_pattern = TRUE;
	}

	Battle_over_timestamp = timestamp(BATTLE_CHECK_INTERVAL);

	return 0;
}

// -------------------------------------------------------------------------------------------------
// event_music_friendly_arrival() 
//
// An friendly has arrived, play a friendly arrival pattern.
//
int event_music_friendly_arrival()
{
	if ( Event_music_enabled == FALSE )
		return -1;

	if ( Event_music_level_inited == FALSE )
		return -1;

	if (timestamp_elapsed(Next_arrival_timestamp) == false) {
		return 0;
	}

	int next_pattern;
	if ( Event_Music_battle_started == TRUE ) {
		next_pattern = SONG_AARV_2;
	}
	else {
		next_pattern = SONG_AARV_1;
	}

	if ( Current_pattern == maybe_get_next_nrml_no_cycle(next_pattern) )
		return 0;	// already playing

	if ( Current_pattern == SONG_DEAD_1 )
		return 0;	// death is the last song to play

	if ( (Current_pattern == SONG_VICT_1) || (Current_pattern == SONG_VICT_1) )
		return 0;

	// Goober5000 - to avoid array out-of-bounds
	Assert(Current_pattern >= 0 && Current_pattern < MAX_PATTERNS);

	if ( Patterns[Current_pattern].next_pattern != Patterns[Current_pattern].default_next_pattern )
		return 0;	// don't squash a pending pattern

	// After the second friendly arrival, default to SONG_BTTL_3
	Num_friendly_arrivals++;

	if ( Current_pattern != -1 ) {
		// AL 06-24-99: always overlay allied arrivals
		// Goober5000 - bah, this screws up the FS1 music... so this is a hack to *not* overlay
		// *if* the soundtrack title includes "FS1" in it (as set in event_music_level_init)

		// don't overlay
		if (Current_soundtrack_fs1_style) {
			Patterns[Current_pattern].next_pattern = next_pattern;
			Patterns[Current_pattern].force_pattern = TRUE;
		// overlay
		} else {
			// Goober5000 - I didn't touch this part... for some reason, FS2 only has one
			// arrival music pattern, and this is it
			Assert(Patterns[SONG_AARV_1].handle >= 0 );
			audiostream_play(Patterns[SONG_AARV_1].handle, Master_event_music_volume, 0);	// no looping
			audiostream_set_sample_cutoff(Patterns[SONG_AARV_1].handle, fl2i(Patterns[SONG_AARV_1].num_measures * Patterns[SONG_AARV_1].samples_per_measure) );
		}
	}

	Next_arrival_timestamp = timestamp(ARRIVAL_INTERVAL_TIMESTAMP);

	Battle_over_timestamp = timestamp(BATTLE_CHECK_INTERVAL);

	return 0;
}

//	Play arrival music keyed to team "team".
void event_music_arrival(int team)
{
	//	No friendly arrival music in a training mission.
	if ( The_mission.game_type & MISSION_TYPE_TRAINING )
		return;

	if ( Player_ship->team == team ) {
		event_music_friendly_arrival();
	} else {
		event_music_enemy_arrival();
	}
}

// -------------------------------------------------------------------------------------------------
// event_music_primary_goals_met() 
//
// A primary goal has failed
//
int event_music_primary_goal_failed()
{
	int next_pattern;

	//	No special tracks in training.
	if ( The_mission.game_type & MISSION_TYPE_TRAINING )
		return -1;

	if ( Event_music_enabled == FALSE )
		return -1;

	if ( Event_music_level_inited == FALSE )
		return -1;

	if ( Current_pattern == SONG_DEAD_1 )
		return 0;	// death is the last song to play

	if ( Patterns[SONG_FAIL_1].handle < 0 )	// can't play if music file doesn't exist
		return 0;

	if ( hostile_ships_present() ) {
		next_pattern = SONG_BTTL_1;
	}
	else {
		next_pattern = SONG_NRML_1;
		Event_Music_battle_started = 0;
	}

	if ( Current_pattern != -1 ) {
		Patterns[Current_pattern].next_pattern = next_pattern;
		Patterns[Current_pattern].force_pattern = TRUE;
	}

	return 0;
}

// -------------------------------------------------------------------------------------------------
// event_music_primary_goals_met() 
//
// A goal has been achieved, play the appropriate victory music.
//
int event_music_primary_goals_met()
{
	int next_pattern = SONG_VICT_1;

	//	No special tracks in training.
	if ( The_mission.game_type & MISSION_TYPE_TRAINING )
		return -1;

	if ( Event_music_enabled == FALSE )
		return -1;

	if ( Event_music_level_inited == FALSE )
		return -1;

	if ( (Current_pattern == SONG_VICT_1) || (Current_pattern == SONG_VICT_2) )
		return 0;	// already playing

	if ( Current_pattern == SONG_DEAD_1 )
		return 0;	// death is the last song to play

	if ( hostile_ships_present() ) {
		Patterns[SONG_VICT_1].next_pattern = SONG_BTTL_1;
	}
	else {
		Patterns[SONG_VICT_1].next_pattern = SONG_VICT_2;
		Event_Music_battle_started = 0;

		// If the mission goals aren't met (or there are no goals), or if victory 2 music has already played, then go
		// to the next default track
		if ( !mission_goals_met() || Victory2_music_played || (Num_goals == 0)) {
			Patterns[next_pattern].next_pattern = Patterns[next_pattern].default_next_pattern;
		} else {
			Victory2_music_played = 1;
		}
	}

	if ( Current_pattern != -1 ) {
		Patterns[Current_pattern].next_pattern = next_pattern;
		Patterns[Current_pattern].force_pattern = TRUE;
	}

	return 0;
}

// -------------------------------------------------------------------------------------------------
// event_music_player_death() 
//
// The player has died, play death pattern.
//
int event_music_player_death()
{
	if ( Event_music_enabled == FALSE )
		return -1;

	if ( Event_music_level_inited == FALSE )
		return -1;

	if ( Current_pattern == SONG_DEAD_1 )
		return 0;	// already playing

	if ( Current_pattern != -1 ) {
		Patterns[Current_pattern].next_pattern = SONG_DEAD_1;
		Patterns[Current_pattern].force_pattern = TRUE;
	}

	return 0;
}

// -------------------------------------------------------------------------------------------------
// event_music_player_respawn() 
//
// Player has respawned (multiplayer only)
//
int event_music_player_respawn()
{
	if ( Event_music_enabled == FALSE )
		return -1;

	if ( Event_music_level_inited == FALSE )
		return -1;

//	Assert(Current_pattern == SONG_DEAD_1);

	Event_Music_battle_started = 0;
	Patterns[Current_pattern].next_pattern = SONG_NRML_1;
	Patterns[Current_pattern].force_pattern = TRUE;

	return 0;
}

// -------------------------------------------------------------------------------------------------
// event_music_player_respawn_as_observer() 
//
// Player has respawned (multiplayer only)
//
int event_music_player_respawn_as_observer()
{
	if ( Event_music_enabled == FALSE )
		return -1;

	if ( Event_music_level_inited == FALSE )
		return -1;

	if ( Current_pattern >= 0 ) {
		if ( audiostream_is_playing(Patterns[Current_pattern].handle) ) {
			audiostream_stop(Patterns[Current_pattern].handle);
			Current_pattern = -1;
		}
	}

	return 0;
}

void parse_soundtrack()
{
	char fname[MAX_FILENAME_LEN];
	char line_buf[128];
	int num_patterns = 0;

	required_string("#Soundtrack Start");
	required_string("$SoundTrack Name:");
	stuff_string(Soundtracks[Num_soundtracks].name, F_NAME, NULL);
	while (required_string_either("#SoundTrack End","$Name:")) {
		Assert( num_patterns < MAX_PATTERNS );
		required_string("$Name:");
		stuff_string(line_buf, F_NAME, NULL);

		// line_buf holds 3 fields:  filename, num measures, bytes per measure

		char *token;
		int count = 0;
		token = strtok( line_buf, NOX(" ,\t"));
		strcpy(fname, token);
		while ( token != NULL ) {
			token = strtok( NULL, NOX(" ,\t") );
			if ( token == NULL ) {
				Assert(count == 2 );
				break;
			}

			if ( count == 0 ) {
				Pattern_num_measures[Num_soundtracks][num_patterns] = (float)atof(token);

			} else {
				Pattern_samples_per_measure[Num_soundtracks][num_patterns] = atoi(token);
			}

			count++;
		}	// end while

		// convert from samples per measure to bytes per measure
		strcpy(Soundtracks[Num_soundtracks].pattern_fnames[num_patterns], fname);
		num_patterns++;
		}
		required_string("#SoundTrack End");
		Soundtracks[Num_soundtracks].num_patterns = num_patterns;
		Num_soundtracks++;
}
void parse_menumusic()
{
	char fname[MAX_FILENAME_LEN];
	required_string("$Name:");
	stuff_string(fname, F_PATHNAME, NULL);
	Assert( strlen(fname) < (NAME_LENGTH-1) );
	strcpy( Spooled_music[Num_music_files].name, fname );

	required_string("$Filename:");
	stuff_string(fname, F_PATHNAME, NULL);
	if ( strnicmp(fname, NOX("none.wav"), 4)  ) {
		Assert( strlen(fname) < (MAX_FILENAME_LEN-1) );
		strcpy( Spooled_music[Num_music_files].filename, fname );
	}

	Num_music_files++;	
}

// -------------------------------------------------------------------------------------------------
// event_music_parse_musictbl() will parse the music.tbl file, and set up the Mission_songs[]
// array
//
void event_music_parse_musictbl(char* longname, bool is_chunk)
{
	int rval, i, j;

	// Goober5000 - clear all the filenames, so we're compatible with the extra NRMLs in FS1 music
	for (i = 0; i < MAX_SOUNDTRACKS; i++)
	{
		for (j = 0; j < MAX_PATTERNS; j++)
		{
			strcpy(Soundtracks[i].pattern_fnames[j], NOX("none.wav"));
		}
	}

	if ((rval = setjmp(parse_abort)) != 0) {
		Error(LOCATION, "Unable to parse %sl!  Code = %i.\n", longname, rval);

	} else {
		// open localization
		lcl_ext_open();

		read_file_text(longname);
		reset_parse();		

		if(!is_chunk)
		{
			// Loop through all the sound-tracks
			while (required_string_either("#Menu Music Start","#SoundTrack Start")) {
				Assert(Num_soundtracks < MAX_SOUNDTRACKS);
				parse_soundtrack();
			}

			// Parse the menu music section
			required_string("#Menu Music Start");
			while (required_string_either("#Menu Music End","$Name:")) {
				Assert( Num_music_files < MAX_SPOOLED_MUSIC );
				parse_menumusic();
			}
			required_string("#Menu Music End");
		}
		else
		{
			// Loop through all the sound-tracks
			while (required_string_either("#Menu Music Start","#SoundTrack Start")) {
				Assert(Num_soundtracks < MAX_SOUNDTRACKS);
				parse_soundtrack();
			}

			// Parse the menu music section
			if(optional_string("#Menu Music Start"))
			{
				while (required_string_either("#Menu Music End","$Name:")) {
					Assert( Num_music_files < MAX_SPOOLED_MUSIC );
					parse_menumusic();
				}
				required_string("#Menu Music End");
			}
		}

		// close localization
		lcl_ext_close();
	}
}

// -------------------------------------------------------------------------------------------------
// event_music_change_pattern()
//
// Force a particular pattern to play.  This is used for debugging purposes.
//
void event_music_change_pattern(int new_pattern)
{
	if ( Event_music_enabled == FALSE ) {
		nprintf(("EVENTMUSIC", "EVENTMUSIC ==> Requested a song switch when event music is not enabled\n"));
		return;
	}

	if ( Event_music_level_inited == FALSE ) {
		nprintf(("EVENTMUSIC", "EVENTMUSIC ==> Event music is not enabled\n"));
		return;
	}

	if ( Current_pattern == maybe_get_next_nrml_no_cycle(new_pattern) );
		return;	// already playing

	if ( Current_pattern != -1 ) {
		Patterns[Current_pattern].next_pattern = new_pattern;
		Patterns[Current_pattern].force_pattern = TRUE;
	}
}

// -------------------------------------------------------------------------------------------------
// event_music_return_current_pattern()
//
// Simply return what the current pattern being played is.  Don't want to make global.
//
int event_music_return_current_pattern()
{
	return Current_pattern;
}

// -------------------------------------------------------------------------------------------------
// event_music_disable()
//
// Stop any patterns that are playing, and prevent any further patterns from playing (ie
// set Event_music_enabled = FALSE).  We don't uninit event music, since it might be toggled
// back on this level.
//
void event_music_disable()
{
	if ( Event_music_level_inited == FALSE )
		return;

	if ( Event_music_enabled == FALSE )
		return;

	if (Current_pattern == -1)
		return;

	Assert( Current_pattern >= 0 && Current_pattern < MAX_PATTERNS );
	if (  audiostream_is_playing(Patterns[Current_pattern].handle) ) {
			audiostream_stop(Patterns[Current_pattern].handle);	// stop current and rewind
	}

	Event_music_begun = FALSE;
	Event_music_enabled = FALSE;
	Current_pattern = -1;
}

// -------------------------------------------------------------------------------------------------
// event_music_enable()
//
// Init the event music (ie load the patterns) if required.  Set up the first song to play, and
// set Event_music_enabled = TRUE to allow patterns to play.
//
void event_music_enable()
{
	if ( Event_music_enabled == TRUE )
		return;

	Event_music_enabled = TRUE;

	if ( Event_music_level_inited == FALSE ) {
		event_music_level_init();
		// start the first pattern to play (delayed)
		if ( Game_mode & GM_IN_MISSION ) {
			event_music_first_pattern();
		}
	}
	else {
		// start a new pattern
		Event_music_begun = FALSE;
		Pattern_timer_id = timestamp(150);
		event_music_start_default();
	}
}

// -------------------------------------------------------------------------------------------------
// event_music_start_default()
//
//	Start playing a default track, based on how far the mission has progressed
//
void event_music_start_default()
{	
	int next_pattern;

	if ( Event_Music_battle_started == TRUE ) {
		if ( hostile_ships_present() ) {
			next_pattern = SONG_BTTL_1;
		}
		else {
			Event_Music_battle_started = FALSE;
			next_pattern = SONG_NRML_1;
		}
	}
	else
	{
		next_pattern = SONG_NRML_1;	
	}

	// switch now
	if ( Current_pattern == -1 ) {
		Current_pattern = maybe_cycle_nrml(next_pattern);
	}
	// switch later
	else {
		Patterns[Current_pattern].next_pattern = next_pattern;
		Patterns[Current_pattern].force_pattern = TRUE;
	}

}

// -------------------------------------------------------------------------------------------------
// event_music_pause()
//
//	Stop any playing pattern, but don't rewind.
//
void event_music_pause()
{
	if ( Event_music_enabled == FALSE ) {
		nprintf(("EVENTMUSIC", "EVENTMUSIC ==> Requested a song switch when event music is not enabled\n"));
		return;
	}

	if ( Event_music_level_inited == FALSE ) {
		nprintf(("EVENTMUSIC", "EVENTMUSIC ==> Event music is not enabled\n"));
		return;
	}

	if (Current_pattern == -1)
		return;

	Assert( Current_pattern >= 0 && Current_pattern < MAX_PATTERNS );
	if (  audiostream_is_playing(Patterns[Current_pattern].handle) ) {
			audiostream_stop(Patterns[Current_pattern].handle, 0);	// stop current and don't rewind
	}
}

// -------------------------------------------------------------------------------------------------
// event_music_unpause()
//
//	Start the Current_pattern if it is paused.
//
void event_music_unpause()
{
	if ( Event_music_enabled == FALSE ) {
		nprintf(("EVENTMUSIC", "EVENTMUSIC ==> Requested a song switch when event music is not enabled\n"));
		return;
	}

	if ( Event_music_level_inited == FALSE ) {
		nprintf(("EVENTMUSIC", "EVENTMUSIC ==> Event music is not enabled\n"));
		return;
	}

	if (Current_pattern == -1)
		return;

	Assert( Current_pattern >= 0 && Current_pattern < MAX_PATTERNS );
	if ( audiostream_is_paused(Patterns[Current_pattern].handle) == TRUE ) {
		audiostream_play(Patterns[Current_pattern].handle, Master_event_music_volume, 0);	// no looping
		audiostream_set_sample_cutoff(Patterns[Current_pattern].handle, fl2i(Patterns[Current_pattern].num_measures * Patterns[Current_pattern].samples_per_measure) );
	}
}

// -------------------------------------------------------------------------------------------------
// event_music_set_volume_all()
//
//	Set the volume of the event driven music.  Used when using the game-wide music volume is changed
// by the user.
//
void event_music_set_volume_all(float volume)
{
	audiostream_set_volume_all(volume, ASF_EVENTMUSIC);
}

// ----------------------------------------------------------------------
// hostile_ships_present()
//
// Determine if there are any non-friendly ships in existance
//
// returns: 1 =>	there are non-friendly ships in existance
//				0 =>  any ships in existance are friendly
int hostile_ships_present()
{
	ship		*shipp;
	ship_obj *so;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		shipp = &Ships[Objects[so->objnum].instance];

		// check if ship if enemy ship
		if ( (shipp->team == Player_ship->team) && (Player_ship->team != TEAM_TRAITOR)  )
			continue;

		// check if ship is threatening
		if ( (shipp->flags & (SF_DISABLED|SF_DYING|SF_DEPARTING)) || (ship_subsys_disrupted(shipp, SUBSYSTEM_ENGINE)) ) 
			continue;
	
		// check if ship is flyable
		if ( Ship_info[shipp->ship_info_index].flags & SIF_NOT_FLYABLE ) {
			continue;
		}

		// check if ship is visible by player's team
		if ( hud_target_invalid_awacs(&Objects[so->objnum]) == 1 ) {
			continue;
		}

		return 1;
	}
	return 0;
}

// ----------------------------------------------------------------------
// hostile_ships_to_arrive()
//
// Determine if there are any non-friendly ships yet to arrive
//
// NOTE: neutral ships are considered hostile for the purpose of event music
//
int hostile_ships_to_arrive()
{
	p_object *p_objp;

	p_objp = GET_FIRST(&ship_arrival_list);
	while( p_objp != END_OF_LIST(&ship_arrival_list) )	{
		if ( (p_objp->team != Player_ship->team) && !(p_objp->flags & P_SF_CANNOT_ARRIVE) ) {
			return 1;
		}
		p_objp = GET_NEXT(p_objp);
	}
	return 0;
}

// ----------------------------------------------------------------
// event_music_get_info()
//
// Return information about the event music in the buffer outbuf
// NOTE: callers to this function are advised to allocate a 256 byte buffer
void event_music_get_info(char *outbuf)
{
	if ( Event_music_enabled == FALSE || Event_music_level_inited == FALSE || Current_pattern == -1 ) {
		sprintf(outbuf,XSTR( "Event music is not playing", 213));
	}
	else {	
		sprintf(outbuf,XSTR( "soundtrack: %s [%s]", 214), Soundtracks[Current_soundtrack_num].name, Pattern_description[Current_pattern]);
	}
}

// ----------------------------------------------------------------
// event_music_next_soundtrack()
//
// input:	delta		=>		1 or -1, depending if you want to go to next or previous song
//
// returns: New soundtrack number if successfully changed, otherwise return -1
//
int event_music_next_soundtrack(int delta)
{
	int new_soundtrack;

	if ( Event_music_enabled == FALSE || Event_music_level_inited == FALSE ) {
		return -1;
	}
		
	new_soundtrack = Current_soundtrack_num + delta;
	if ( new_soundtrack >= Num_soundtracks )
		new_soundtrack = 0;

	event_music_level_close();
	event_music_level_init(new_soundtrack);

	return Current_soundtrack_num;
}

// Goober5000 - along the same lines; this is for the sexp
void event_sexp_change_soundtrack(char *name)
{
	Assert(name);

	int i, new_soundtrack = -1;

	if ( Event_music_enabled == FALSE || Event_music_level_inited == FALSE )
	{
		return;
	}

	for (i=0; i<Num_soundtracks; i++)
	{
		if (!stricmp(name, Soundtracks[i].name))
		{
			new_soundtrack = i;
		}
	}

	event_music_level_close();
	event_music_level_init(new_soundtrack);
}

// ----------------------------------------------------------------
// event_music_get_soundtrack_name()
//
// Return information about the event music in the buffer outbuf
// NOTE: callers to this function are advised to allocate a NAME_LENGTH buffer
void event_music_get_soundtrack_name(char *outbuf)
{
	if ( Event_music_enabled == FALSE || Event_music_level_inited == FALSE ) {
		strcpy(outbuf, XSTR( "Event music is not playing", 213));
	}
	else {
		sprintf(outbuf, Soundtracks[Current_soundtrack_num].name);
	}
}

// set the current soundtrack based on name
void event_music_set_soundtrack(char *name)
{
	int i;

	// find the correct index for the event music
	for ( i = 0; i < Num_soundtracks; i++ ) {
		if ( !stricmp(name, Soundtracks[i].name) ) {
			Current_soundtrack_num = i;
			break;
		}
	}

	if ( i == Num_soundtracks ) {
		Current_soundtrack_num = -1;
		mprintf(("Current soundtrack set to -1 in event_music_set_soundtrack\n"));
	}
}

int event_music_get_spooled_music_index(char *name)
{
	// find the correct index for the event music
	for ( int i = 0; i < Num_music_files; i++ ) {
		if ( !stricmp(name, Spooled_music[i].name) ) {
			return i;
		}
	}

	return -1;
}

// set a score based on name
void event_music_set_score(int score_index, char *name)
{
	Assert(score_index < NUM_SCORES);

	// find the correct index for the event music
	Mission_music[score_index] = event_music_get_spooled_music_index(name);
}

// reset what sort of music is to be used for this mission
void event_music_reset_choices()
{
	Current_soundtrack_num = -1;
	mprintf(("Current soundtrack set to -1 in event_music_reset_choices\n"));
	Mission_music[SCORE_BRIEFING] = -1;
	event_music_set_score(SCORE_DEBRIEF_SUCCESS, "Success");
	event_music_set_score(SCORE_DEBRIEF_AVERAGE, "Average");
	event_music_set_score(SCORE_DEBRIEF_FAIL, "Failure");
	//Mission_music[SCORE_DEBRIEF_SUCCESS] = MUSIC_DEBRIEF_SUCCESS_1;
	//Mission_music[SCORE_DEBRIEF_FAIL] = MUSIC_DEBRIEF_FAIL_1;
}

void event_music_hostile_ship_destroyed()
{
	Battle_over_timestamp = timestamp(BATTLE_CHECK_INTERVAL);
}


#pragma optimize("", on)
