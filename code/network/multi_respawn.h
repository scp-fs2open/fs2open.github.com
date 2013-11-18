/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _MULTIPLAYER_RESPAWN_HEADER_FILE
#define _MULTIPLAYER_RESPAWN_HEADER_FILE

// ---------------------------------------------------------------------------------------
// MULTI RESPAWN DEFINES/VARS
//
#include "globalincs/pstypes.h"

class object;
struct header;
struct net_player;


// ---------------------------------------------------------------------------------------
// MULTI RESPAWN FUNCTIONS
//

// check to see if a net player needs to be respawned
void multi_respawn_check(object *objp);

// respawn normally
void multi_respawn_normal();

// respawn as an observer
void multi_respawn_observer();

// server should check to see if any respawned players have run out of their invulnerability
void multi_respawn_handle_invul_players();

// build a list of base respawn points on the server, for this level
void multi_respawn_build_points();

void multi_respawn_init();
void multi_respawn_check_ai();

// notify of a player leaving
void multi_respawn_player_leave(net_player *pl);

// ---------------------------------------------------------------------------------------
// MULTI RESPAWN PACKET HANDLERS
//

void multi_respawn_process_packet(ubyte *data, header *hinfo);

#endif
