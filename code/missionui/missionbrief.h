/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MISSIONBRIEF_H
#define _MISSIONBRIEF_H

#include "ui/ui.h"

// #defines to identify which screen we are on
#define ON_BRIEFING_SELECT			1
#define ON_SHIP_SELECT				2
#define ON_WEAPON_SELECT			3

// briefing buttons
#define BRIEF_BUTTON_LAST_STAGE		0
#define BRIEF_BUTTON_NEXT_STAGE		1
#define BRIEF_BUTTON_PREV_STAGE		2
#define BRIEF_BUTTON_FIRST_STAGE		3
#define BRIEF_BUTTON_SCROLL_UP		4
#define BRIEF_BUTTON_SCROLL_DOWN		5
#define BRIEF_BUTTON_SKIP_TRAINING	6
#define BRIEF_BUTTON_PAUSE				7
#define BRIEF_BUTTON_MULTI_LOCK		8
#define BRIEF_BUTTON_EXIT_LOOP		9


#define NUM_BRIEFING_REGIONS	(NUM_COMMON_REGIONS + 8)

extern int	Brief_multitext_bitmap;	// bitmap for multiplayer chat window
extern int	Brief_background_bitmap;
extern UI_INPUTBOX	Common_multi_text_inputbox[3];

// Sounds
#define		BRIEFING_MUSIC_DELAY	2500		// 650 ms delay before briefing music starts
extern int	Briefing_music_handle;
extern int	Briefing_music_begin_timestamp;

extern int Briefing_paused;	// for stopping audio and stage progression

struct brief_icon;

void brief_init();
void brief_close();
void brief_do_frame(float frametime);
void brief_unhide_buttons();
brief_icon *brief_get_closeup_icon();
void brief_turn_off_closeup_icon();

void briefing_stop_music();
void briefing_start_music();
void briefing_load_music(char* fname);
void brief_stop_voices();

void brief_pause();
void brief_unpause();

int brief_only_allow_briefing();

#endif // don't add anything past this line
