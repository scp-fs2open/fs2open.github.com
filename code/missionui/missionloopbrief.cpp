/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "anim/animplay.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/eventmusic.h"
#include "gamesnd/gamesnd.h"
#include "graphics/generic.h"
#include "io/key.h"
#include "mission/missionbriefcommon.h"
#include "mission/missioncampaign.h"
#include "missionui/missionloopbrief.h"
#include "missionui/missionscreencommon.h"
#include "popup/popup.h"
#include "sound/audiostr.h"
#include "sound/fsspeech.h"

extern char default_loop_briefing_color;	// Doesn't seem worth including alphacolors.h for -MageKing17


// ---------------------------------------------------------------------------------------------------------------------------------------
// MISSION LOOP BRIEF DEFINES/VARS
//

char *Loop_brief_fname[GR_NUM_RESOLUTIONS] = {
	"LoopBrief",		// GR_640
	"2_LoopBrief",		// GR_1024
};

char *Loop_brief_mask[GR_NUM_RESOLUTIONS] = {
	"LoopBrief-m",		// GR_640
	"2_Loopbrief-m",	// GR_1024
};

#define NUM_LOOP_BRIEF_BUTTONS				2
#define LOOP_BRIEF_DECLINE						0
#define LOOP_BRIEF_ACCEPT						1
ui_button_info Loop_buttons[GR_NUM_RESOLUTIONS][NUM_LOOP_BRIEF_BUTTONS] = {
	{ // GR_640
		ui_button_info("LBB_00",		529,	437,	-1,	-1,	0),
		ui_button_info("LBB_01",		575,	433,	-1,	-1,	1),
	},
	{ // GR_1024
		ui_button_info("2_LBB_00",		846,	699,	-1,	-1,	0),
		ui_button_info("2_LBB_01",		920,	693,	-1,	-1,	1),
	}
};

#define NUM_LOOP_TEXT							2
UI_XSTR Loop_text[GR_NUM_RESOLUTIONS][NUM_LOOP_TEXT] = {
	{ // GR_640
		{ "Decline",		1467,	514,	413,	UI_XSTR_COLOR_PINK,	-1,	&Loop_buttons[0][LOOP_BRIEF_DECLINE].button },
		{ "Accept",			1035,	573,	413,	UI_XSTR_COLOR_PINK,	-1,	&Loop_buttons[0][LOOP_BRIEF_ACCEPT].button },
	},	
	{ // GR_1024
		{ "Decline",		1467,	855,	670,	UI_XSTR_COLOR_PINK,	-1,	&Loop_buttons[1][LOOP_BRIEF_DECLINE].button },
		{ "Accept",			1035,	928,	670,	UI_XSTR_COLOR_PINK,	-1,	&Loop_buttons[1][LOOP_BRIEF_ACCEPT].button },
	}
};

// Originally from missioncmdbrief.cpp
// center coordinates only
int Loop_brief_anim_center_coords[GR_NUM_RESOLUTIONS][2] =
{
	{
		242, 367				// GR_640
	},
	{
		392, 587				// GR_1024
	}
};

// text window coords
int Loop_brief_text_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		25,	107,	591,	143
	},
	{ // GR_1024
		40,	171,	945,	229
	}
};

UI_WINDOW Loop_brief_window;
int Loop_brief_bitmap;

generic_anim Loop_anim;

int Loop_sound;

// ---------------------------------------------------------------------------------------------------------------------------------------
// MISSION LOOP BRIEF FUNCTIONS
//

// button press
void loop_brief_button_pressed(int i)
{	
	switch(i){
	case LOOP_BRIEF_DECLINE:		
		gameseq_post_event(GS_EVENT_START_GAME);
		gamesnd_play_iface(SND_USER_SELECT);
		break;

	case LOOP_BRIEF_ACCEPT:
		// select the loop mission		
		Campaign.loop_enabled = 1;
		Campaign.loop_reentry = Campaign.next_mission;			// save reentry pt, so we can break out of loop
		Campaign.next_mission = Campaign.loop_mission;		

		gameseq_post_event(GS_EVENT_START_GAME);
		gamesnd_play_iface(SND_USER_SELECT);
		break;
	}
}

// init
void loop_brief_init()
{
	int idx;
	ui_button_info *b;

	// load the background bitmap
	Loop_brief_bitmap = bm_load(Loop_brief_fname[gr_screen.res]);
	Assert(Loop_brief_bitmap != -1);

	// window
	Loop_brief_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Loop_brief_window.set_mask_bmap(Loop_brief_mask[gr_screen.res]);	

	// add the buttons
	for (idx=0; idx<NUM_LOOP_BRIEF_BUTTONS; idx++) {
		b = &Loop_buttons[gr_screen.res][idx];

		b->button.create(&Loop_brief_window, "", b->x, b->y, 60, 30, 0, 1);		
		b->button.set_highlight_action(common_play_highlight_sound);
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// add text
	for(idx=0; idx<NUM_LOOP_TEXT; idx++){
		Loop_brief_window.add_XSTR(&Loop_text[gr_screen.res][idx]);
	}

	const char* anim_name;
	// load animation if any
	if(Campaign.missions[Campaign.current_mission].mission_branch_brief_anim != NULL){
		anim_name = Campaign.missions[Campaign.current_mission].mission_branch_brief_anim;
	} else {
		anim_name = "CB_default";
	}

	int stream_result = generic_anim_init_and_stream(&Loop_anim, anim_name, bm_get_type(Loop_brief_bitmap), true);
	// we've failed to load any animation
	if (stream_result < 0) {
		// load an image and treat it like a 1 frame animation
		Loop_anim.first_frame = bm_load(anim_name);	//if we fail here, the value is still -1
		if(Loop_anim.first_frame != -1) {
			Loop_anim.num_frames = 1;
		}
	}

	// init brief text
	if(Campaign.missions[Campaign.current_mission].mission_branch_desc != NULL){
		brief_color_text_init(Campaign.missions[Campaign.current_mission].mission_branch_desc, Loop_brief_text_coords[gr_screen.res][2], default_loop_briefing_color);
	}

	bool sound_played = false;


	// open sound
	if(Campaign.missions[Campaign.current_mission].mission_branch_brief_sound != NULL){
		Loop_sound = audiostream_open(Campaign.missions[Campaign.current_mission].mission_branch_brief_sound, ASF_VOICE);

		if(Loop_sound != -1){
			audiostream_play(Loop_sound, Master_voice_volume, 0);
			sound_played = true;
		}
	}

	if(sound_played == false) {
		fsspeech_play(FSSPEECH_FROM_BRIEFING, 
			Campaign.missions[Campaign.current_mission].mission_branch_desc);

	}

	// music
	common_music_init(SCORE_BRIEFING);
}

// do
void loop_brief_do(float frametime)
{
	int k;
	int idx;

	// process keys
	k = Loop_brief_window.process();	

	switch (k) {
	case KEY_ESC:
		int do_loop = 0;

		// this popup should be straight forward, and also not allow you to get out
		// of it without actually picking one of the two options
		do_loop = popup(PF_USE_NEGATIVE_ICON | PF_USE_AFFIRMATIVE_ICON | PF_IGNORE_ESC | PF_BODY_BIG, 2, XSTR("Decline", 1467), XSTR("Accept", 1035), XSTR("You must either Accept or Decline before returning to the Main Hall", 1618));

		// if we accepted moving into loop then set it up for the next time the user plays
		if (do_loop == 1) {
			// select the loop mission		
			Campaign.loop_enabled = 1;
			Campaign.loop_reentry = Campaign.next_mission;			// save reentry pt, so we can break out of loop
			Campaign.next_mission = Campaign.loop_mission;
		}

		gameseq_post_event(GS_EVENT_MAIN_MENU);
		return;
	}

	// process button presses
	for (idx=0; idx<NUM_LOOP_BRIEF_BUTTONS; idx++){
		if (Loop_buttons[gr_screen.res][idx].button.pressed()){
			loop_brief_button_pressed(idx);
		}
	}
	
	common_music_do();

	// clear
	GR_MAYBE_CLEAR_RES(Loop_brief_bitmap);
	if (Loop_brief_bitmap >= 0) {
		gr_set_bitmap(Loop_brief_bitmap);
		gr_bitmap(0, 0, GR_RESIZE_MENU);
	} 
	
	// draw the window
	Loop_brief_window.draw();		

	// render the briefing text
	brief_render_text(0, Loop_brief_text_coords[gr_screen.res][0], Loop_brief_text_coords[gr_screen.res][1], Loop_brief_text_coords[gr_screen.res][3], flFrametime);

	if(Loop_anim.num_frames > 0) {
		int x;
		int y;

		bm_get_info((Loop_anim.streaming) ? Loop_anim.bitmap_id : Loop_anim.first_frame, &x, &y, NULL, NULL, NULL);
		x = Loop_brief_anim_center_coords[gr_screen.res][0] - x / 2;
		y = Loop_brief_anim_center_coords[gr_screen.res][1] - y / 2;
		generic_anim_render(&Loop_anim, frametime, x, y, true);
	}

	// render all anims
	anim_render_all(GS_STATE_LOOP_BRIEF, flFrametime);

	gr_flip();
}

// close
void loop_brief_close()
{
	// this makes sure that we're all cool no matter how the user decides to exit
	mission_campaign_mission_over();

	// free the bitmap
	if (Loop_brief_bitmap >= 0){
		bm_release(Loop_brief_bitmap);
	}		
	Loop_brief_bitmap = -1;

	// destroy the window
	Loop_brief_window.destroy();

	if (Loop_anim.num_frames > 0)
	{
		generic_anim_unload(&Loop_anim);
	}

	// stop voice
	if(Loop_sound != -1){
		audiostream_stop(Loop_sound, 1, 0);
		audiostream_close_file(Loop_sound, 1);
		Loop_sound = -1;
	}

	fsspeech_stop();

	// stop music
	common_music_close();
}
