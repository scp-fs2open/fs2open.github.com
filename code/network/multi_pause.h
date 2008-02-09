/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_pause.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:00 $
 * $Author: penguin $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *  
 * 
 * 3     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 2     5/07/98 6:26p Dave
 * Fix strange boundary conditions which arise when players die/respawn
 * while the game is being ended. Spiff up the chatbox doskey thing a bit.
 * 
 * 1     4/14/98 12:18p Dave
 *
 * 
 * $NoKeywords: $
 */


#ifndef _MULTI_PAUSE_HEADER_FILE
#define _MULTI_PAUSE_HEADER_FILE

// ----------------------------------------------------------------------------------
// PAUSE DEFINES/VARS
//

class UI_WINDOW;
struct net_player;

// state of the game (paused or not) on _my_ machine. Obviously this is important for the server
// call multi_pause_reset() to reinitialize
extern int Multi_pause_status;

// who paused the game
extern net_player *Multi_pause_pauser;


// ----------------------------------------------------------------------------------
// PAUSE FUNCTIONS
//

// re-initializes the pause system. call before entering the mission to reset
void multi_pause_reset();

// send a request to pause or unpause a game (all players should use this function)
void multi_pause_request(int pause);

// (client) call when receiving a packet indicating we should pause
void multi_pause_pause();

// (client) call when receiving a packet indicating we should unpause
void multi_pause_unpause();

// (server) evaluate a pause request from the given player (should call for himself as well)
void multi_pause_server_eval_request(net_player *pl, int pause);

// if we still want to eat keys 
int multi_pause_eat_keys();


// ----------------------------------------------------------------------------------
// PAUSE UI FUNCTIONS
//

// initialize multi pause screen
void multi_pause_init(UI_WINDOW *Ui_window);

// do frame for the multi pause screen
void multi_pause_do();

// close the multi pause screen
void multi_pause_close();


#endif
