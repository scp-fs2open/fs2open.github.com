/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __FREESPACE_CHATBOX_H__
#define __FREESPACE_CHATBOX_H__

// prototype
struct net_player;

#define CHATBOX_MAX_LEN						125			// max length of the actual text string

// chatbox flags for creation/switching between modes
#define CHATBOX_FLAG_SMALL					 (1<<0)		// small chatbox
#define CHATBOX_FLAG_BIG					 (1<<1)		// big chatbox
#define CHATBOX_FLAG_MULTI_PAUSED		 (1<<2)		// chatbox in the multiplayer paused screen
#define CHATBOX_FLAG_DRAW_BOX				 (1<<3)		// should be drawn by the chatbox code
#define CHATBOX_FLAG_BUTTONS				 (1<<4)		// the chatbox should be drawing/checking its own buttons
// NOTE : CHATBOX_FLAG_BUTTONS requires that CHATBOX_FLAG_DRAW_BOX is also set!

// initialize all chatbox details with the given mode flags
int chatbox_create(int mode_flags = (CHATBOX_FLAG_SMALL | CHATBOX_FLAG_DRAW_BOX | CHATBOX_FLAG_BUTTONS));

// process this frame for the chatbox
int chatbox_process(int key_in=-1);

// shutdown all chatbox functionality
void chatbox_close();

// render the chatbox for this frame
void chatbox_render();

// try and scroll the chatbox up. return 0 or 1 on fail or success
int chatbox_scroll_up();

// try and scroll the chatbox down, return 0 or 1 on fail or success
int chatbox_scroll_down();

// clear the contents of the chatbox
void chatbox_clear();

// add a line of text (from the player identified by pid) to the chatbox
void chatbox_add_line(const char *msg, int pid, int add_id = 1);

// force the chatbox to go into small mode (if its in large mode) - will not wotk if in multi paused chatbox mode
void chatbox_force_small();

// force the chatbox to go into big mode (if its in small mode) - will not work if in multi paused chatbox mode
void chatbox_force_big();

// "lose" the focus on the chatbox inputbox
void chatbox_lose_focus();

// return if the inputbox for the chatbox currently has focus
int chatbox_has_focus();

// grab the focus for the chatbox inputbox
void chatbox_set_focus();

// return if the inputbox was pressed - "clicked on"
int chatbox_pressed();

// reset all timestamps associated with the chatbox
void chatbox_reset_timestamps();

#endif
