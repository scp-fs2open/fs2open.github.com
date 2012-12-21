/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "network/multi.h"
#include "network/multi_ping.h"
#include "network/multimsgs.h"
#include "io/timer.h"



// ------------------------------------------------------------------------------------
// MULTIPLAYER PING DEFINES/VARS
//


// ------------------------------------------------------------------------------------
// MULTIPLAYER PING FUNCTIONS
//

// initialize all player ping times
void multi_ping_reset_players()
{
	int idx;

	// reset the pings for all players
	for(idx=0;idx<MAX_PLAYERS;idx++){
		multi_ping_reset(&Net_players[idx].s_info.ping);
	}
}

// initialize the given ping struct
void multi_ping_reset(ping_struct *ps)
{
	// blast the struct clear
	memset(ps,0,sizeof(ping_struct));

	ps->ping_add = 0;

	// set the ping start to be -1
	ps->ping_start = -1.0f;

	ps->ping_avg = -1;

	ps->num_pings = 0;
}

// evaluate a pong return on the given struct
void multi_ping_eval_pong(ping_struct *ps)
{	
	int idx;
	float ping_sum;
	
	// if the ping technically hasn't started,
	if(ps->ping_start < 0.0f){
		nprintf(("Network","Processing pong for ping which hasn't started yet!\n"));
		return;
	}
	
	// if we still have room to add a ping
	if(ps->num_pings < MAX_PINGS){
		ps->ping_times[ps->ping_add++] = f2fl(timer_get_fixed_seconds()) - ps->ping_start;		
		ps->num_pings++;
	} 
	// otherwise if we've wrapped around
	else {		
		// increment the place to add the ping time
		if(ps->ping_add >= MAX_PINGS - 1){
			ps->ping_add = 0;
		} else {
			ps->ping_add++;
		}

		ps->ping_times[ps->ping_add] = f2fl(timer_get_fixed_seconds()) - ps->ping_start;
	}

	// calculate the average ping time
	ping_sum = 0.0f;
	for(idx=0;idx<ps->num_pings;idx++){
		ping_sum += ps->ping_times[idx];
	}
	ps->ping_avg = (int)(1000.0f * (ping_sum / (float)ps->num_pings));	
}

// start a ping - call this when sending a ping packet
void multi_ping_start(ping_struct *ps)
{
	ps->ping_start = f2fl(timer_get_fixed_seconds());
}

// send a ping to a specific player
void multi_ping_send(net_player *p)
{
	multi_ping_start(&p->s_info.ping);
	send_ping(&p->p_info.addr);
}

// send a ping to the specified address
void multi_ping_send(net_addr *addr,ping_struct *ps)
{
	multi_ping_start(ps);
	send_ping(addr);
}

// send a ping to all players
void multi_ping_send_all()
{
	int idx;
	for(idx = 0;idx < MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != NULL) && (Net_player->player_id != Net_players[idx].player_id)){
			// start the ping
			multi_ping_start(&Net_players[idx].s_info.ping);			

			// send the ping packet
			send_ping(&Net_players[idx].p_info.addr);
		}
	}
}
