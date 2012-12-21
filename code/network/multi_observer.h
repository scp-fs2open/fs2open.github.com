/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _MULTI_OBSERVER_HEADER_FILE
#define _MULTI_OBSERVER_HEADER_FILE

// ---------------------------------------------------------------------------------------
// MULTI OBSERVER DEFINES/VARS
//

struct net_addr;
struct player;
struct net_player; 

// ---------------------------------------------------------------------------------------
// MULTI OBSERVER FUNCTIONS
//

// create a _permanent_ observer player 
int multi_obs_create_player(int player_num,char *name,net_addr *addr,player *pl);

// create an explicit observer object and assign it to the passed player
void multi_obs_create_observer(net_player *pl);

// create observer object locally, and additionally, setup some other information
// ( client-side equivalent of multi_obs_create_observer() )
void multi_obs_create_observer_client();

// create objects for all known observers in the game at level start
// call this before entering a mission
// this implies for the local player in the case of a client or for _all_ players in the case of a server
void multi_obs_level_init();

// if i'm an observer, zoom to near my targted object (if any)
void multi_obs_zoom_to_target();

#endif
