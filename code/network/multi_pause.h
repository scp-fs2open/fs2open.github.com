/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
void multi_pause_init();

// do frame for the multi pause screen
void multi_pause_do();

// close the multi pause screen
void multi_pause_close(int end_mission);


#endif
