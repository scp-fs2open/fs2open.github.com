/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#ifndef __EVENT_MUSIC_H__
#define __EVENT_MUSIC_H__

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

// Identifies songs in the Soundtrack_filenames[] structure.  The order matches up with
// what is in Pattern_info and music.tbl.  Do not modify without properly inputting to
//Pattern_info and New_pattern_order
#define SONG_NRML_1				0		// Normal Song 1
#define SONG_NRML_2				1		// Normal Song 2 - for FS1
#define SONG_NRML_3				2		// Normal Song 3 - for FS1
#define SONG_AARV_1				3		// Allied Arrival 1
#define SONG_AARV_2				4		// Allied Arrival 2
#define SONG_EARV_1				5		// Enemy Arrival 1
#define SONG_EARV_2				6		// Enemy Arrival 2
#define SONG_BTTL_1				7		// Battle Song 1
#define SONG_BTTL_2				8		// Battle Song 2
#define SONG_BTTL_3				9		// Battle Song 3
#define SONG_FAIL_1				10		// Goal Failed
#define SONG_VICT_1				11		// Victory Song 1
#define SONG_VICT_2				12		// Victory Song 2
#define SONG_DEAD_1				13		// Death Song 1

#define MAX_PATTERNS	14

// if player targets a hostile ship at less than this range, switch to battle track 
#define BATTLE_START_MIN_TARGET_DIST	500	

extern int Event_Music_battle_started;	// flag that will tell us if we've started a battle in the current mission
extern int Event_music_enabled;
extern float Master_event_music_volume;			// range is 0->1


/////////////////////////////////////////////////////////////////////////////
// Used to track what briefing and debriefing music is played for the mission
/////////////////////////////////////////////////////////////////////////////
#define NUM_SCORES						5
#define SCORE_BRIEFING					0
#define SCORE_DEBRIEF_SUCCESS			1
#define SCORE_DEBRIEF_AVERAGE			2
#define SCORE_DEBRIEF_FAIL				3
#define SCORE_FICTION_VIEWER			4
extern int Mission_music[NUM_SCORES];		// indicies into Spooled_music[]
/////////////////////////////////////////////////////////////////////////////

extern int Current_soundtrack_num;		// index into Soundtracks[]


// menu music storage
typedef struct menu_music {
	int flags;
	char name[NAME_LENGTH];				// name music is known by
	char filename[MAX_FILENAME_LEN];	// name music is stored on disk as
} menu_music;

#define MAX_SPOOLED_MUSIC	50			// max number of briefing/mainhall/credits tracks

// Goober5000 - spooled music flags
#define SMF_VALID						(1 << 0)

extern menu_music Spooled_music[MAX_SPOOLED_MUSIC];
extern int Num_music_files;


// event music soundtrack storage
typedef struct tagSOUNDTRACK_INFO {
	int flags;
	int	num_patterns;
	char	name[NAME_LENGTH];
	char	pattern_fnames[MAX_PATTERNS][MAX_FILENAME_LEN];
} SOUNDTRACK_INFO;

#define MAX_SOUNDTRACKS		30			// max number of battle tracks

// Goober5000 - event music flags
#define EMF_VALID						(1 << 0)
#define EMF_ALLIED_ARRIVAL_OVERLAY		(1 << 1)
#define EMF_CYCLE_FS1					(1 << 2)

extern SOUNDTRACK_INFO Soundtracks[MAX_SOUNDTRACKS];
extern int Num_soundtracks;


void	event_music_init();
void	event_music_close();
void	event_music_level_init(int force_soundtrack = -1);
void	event_music_level_close();
void	event_music_do_frame();
void	event_music_disable();
void	event_music_enable();
void	event_music_pause();
void	event_music_unpause();
void	event_music_set_volume_all(float volume);
void	event_music_parse_musictbl(char *filename);
void	event_music_change_pattern(int new_pattern);
int	event_music_return_current_pattern();
void	event_music_first_pattern();
int	event_music_battle_start();
int	event_music_enemy_arrival();
int	event_music_friendly_arrival();
void	event_music_arrival(int team);
int	event_music_primary_goals_met();
int	event_music_primary_goal_failed();
int	event_music_player_death();
void	event_music_start_default();
void	event_music_get_info(char *outbuf);
void	event_music_get_soundtrack_name(char *outbuf);
int	event_music_next_soundtrack(int delta);
void event_sexp_change_soundtrack(char *name);
void	event_music_set_soundtrack(char *name);
void	event_music_set_score(int score_index, char *name);
int event_music_get_soundtrack_index(char *name);
int	event_music_get_spooled_music_index(const char *name);
int	event_music_get_spooled_music_index(const SCP_string& name);
void	event_music_reset_choices();
int	event_music_player_respawn();
int	event_music_player_respawn_as_observer();
void event_music_hostile_ship_destroyed();

#endif /* __EVENT_MUSIC_H__  */
