/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/PlayerMenu.h $
 * $Revision: 1.1 $Date: 2002/05/02 18:03:09 $
 * $Author: penguin $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     8/02/99 9:13p Dave
 * Added popup tips.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 12    2/12/98 4:40p Dave
 * Seperated multiplayer kick functionality into its own module. Tweaked
 * some main hall values. Added default help overlay when adding new
 * pilots.
 * 
 * 11    1/26/98 5:23p Andsager
 * Fixed a compile bug caused by Dave B.
 * 
 * 10    1/26/98 4:44p Dave
 * Changed how multiplayer messaging, endgame, and pausing are handled.
 * Tidied up player select dialog thingie.
 * 
 * 9     11/12/97 4:40p Dave
 * Put in multiplayer campaign support parsing, loading and saving. Made
 * command-line variables better named. Changed some things on the initial
 * pilot select screen.
 * 
 * 8     11/11/97 4:57p Dave
 * Put in support for single vs. multiplayer pilots. Began work on
 * multiplayer campaign saving. Put in initial player select screen
 *
 * $NoKeywords: $
 *
 */

#ifndef _PLAYER_SELECT_MENU_HEADER_FILE
#define _PLAYER_SELECT_MENU_HEADER_FILE

// general defines
#define PLAYER_SELECT_MODE_SINGLE	0							// looking through single player pilots
#define PLAYER_SELECT_MODE_MULTI    1							// looking through multi player pilots

// flag indicating if this is the absolute first pilot created and selected. Used to determine
// if the main hall should display the help overlay screen
extern int Player_select_very_first_pilot;			

// functions for selecting single/multiplayer pilots at the very beginning of Freespace
void player_select_init();
void player_select_do();
void player_select_close();

// function to check whether we found a "last pilot". loads this pilot in if possible and returns true, or false otherwise
int player_select_get_last_pilot();

// tooltips
void player_tips_init();
void player_tips_popup();

#endif
