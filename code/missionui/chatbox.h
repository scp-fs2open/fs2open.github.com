/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MissionUI/Chatbox.h $
 * $Revision: 2.2 $
 * $Date: 2005-07-13 03:25:58 $
 * $Author: Goober5000 $
 *
 * Header file for chat box code
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     5/22/99 5:35p Dave
 * Debrief and chatbox screens. Fixed small hi-res HUD bug.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 14    9/11/98 5:08p Dave
 * More tweaks to kick notification system.
 * 
 * 13    9/11/98 4:14p Dave
 * Fixed file checksumming of < file_size. Put in more verbose kicking and
 * PXO stats store reporting.
 * 
 * 12    5/15/98 5:15p Dave
 * Fix a standalone resetting bug.Tweaked PXO interface. Display captaincy
 * status for team vs. team. Put in asserts to check for invalid team vs.
 * team situations.
 * 
 * 11    4/14/98 5:06p Dave
 * Don't load or send invalid pilot pics. Fixed chatbox graphic errors.
 * Made chatbox display team icons in a team vs. team game. Fixed up pause
 * and endgame sequencing issues.
 * 
 * 10    4/12/98 2:09p Dave
 * Make main hall door text less stupid. Make sure inputbox focus in the
 * multi host options screen is managed more intelligently.
 * 
 * 9     4/01/98 11:19p Dave
 * Put in auto-loading of xferred pilot pic files. Grey out background
 * behind pinfo popup. Put a chatbox message in when players are kicked.
 * Moved mission title down in briefing. Other ui fixes.
 * 
 * 8     3/29/98 1:24p Dave
 * Make chatbox not clear between multiplayer screens. Select player ship
 * as default in mp team select and weapons select screens. Made create
 * game mission list use 2 fixed size columns.
 * 
 * 7     2/13/98 3:46p Dave
 * Put in dynamic chatbox sizing. Made multiplayer file lookups use cfile
 * functions.
 * 
 * 6     1/16/98 2:34p Dave
 * Made pause screen work properly (multiplayer). Changed how chat packets
 * work.
 * 
 * 5     1/07/98 5:20p Dave
 * Put in support for multiplayer campaigns with the new interface
 * screens.
 * 
 * 4     12/18/97 8:59p Dave
 * Finished putting in basic support for weapon select and ship select in
 * multiplayer.
 * 
 * 3     10/01/97 4:47p Lawrance
 * move some #defines out of header file into .cpp file
 * 
 * 2     10/01/97 4:39p Lawrance
 * move chat code into Chatbox.cpp, simplify interface
 * 
 * 1     10/01/97 10:54a Lawrance
 *
 * $NoKeywords: $
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
void chatbox_add_line(char *msg,int pid,int add_id = 1);

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
