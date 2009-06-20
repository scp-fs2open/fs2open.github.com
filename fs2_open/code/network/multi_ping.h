/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _MULTIPLAYER_PING_HEADER_FILE
#define _MULTIPLAYER_PING_HEADER_FILE

// ------------------------------------------------------------------------------------
// MULTIPLAYER PING DEFINES/VARS
//

struct header;
struct net_addr;
struct net_player;

// the max ping we'll store to calculate the average
#define MAX_PINGS					10

typedef struct ping_struct {
	float ping_start;										// time the current ping was sent out, or -1 if none
	float ping_times[MAX_PINGS];						// ping times for calculating the average
	int num_pings;											// # of pings in the ping_times array
	int ping_add;											// where to add the next ping

	int ping_avg;											// in ms, this is the only thing we should be concerned with
} ping_struct;


// ------------------------------------------------------------------------------------
// MULTIPLAYER PING FUNCTIONS
//

// initialize all player ping times
void multi_ping_reset_players();

// initialize the given ping struct
void multi_ping_reset(ping_struct *ps);

// start a ping - call this when sending a ping packet
void multi_ping_start(ping_struct *ps);

// evaluate a pong return on the given struct
void multi_ping_eval_pong(ping_struct *ps);

// send a ping to a specific player
void multi_ping_send(net_player *p);

// send a ping to the specified address
void multi_ping_send(net_addr *addr,ping_struct *ps);

// send a ping to all players
void multi_ping_send_all();

// get the lowest existing ping in the ping struct, returning -1 if no pings
int multi_ping_get_lowest(ping_struct *ps);

// (average ping + lowest ping)/2
int multi_ping_lowest_avg(ping_struct *ps);

#endif
