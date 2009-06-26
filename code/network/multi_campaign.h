/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _MULTIPLAYER_CAMPAIGN_HEADER_FILE
#define _MULTIPLAYER_CAMPAIGN_HEADER_FILE

#include "globalincs/pstypes.h"

// ------------------------------------------------------------------------------------
// MULTIPLAYER CAMPAIGN DEFINES/VARS
//
struct net_player;
struct header;

// ------------------------------------------------------------------------------------
// MULTIPLAYER CAMPAIGN FUNCTIONS
//

// load a new campaign file or notify the standalone if we're not the server
void multi_campaign_start(char *filename);

// client-side start of a campaign
void multi_campaign_client_start();

// move everything and eveyrone into the next mission state
void multi_campaign_next_mission();

// flush all important data between missions
void multi_campaign_flush_data();

// call in the debriefing stage to evaluate what we should be doing in regards to the campaign
// if player_status == 0, nothing should be done
//                  == 1, players want to continue to the next mission
//                  == 2, players want to repeat the previous mission
void multi_campaign_do_debrief(int player_status);

// display the done popup
void multi_campaign_done_popup();

// evaluate post mission goal stuff for the campaign and send all relevant stuff to clients
void multi_campaign_eval_debrief();


// ------------------------------------------------------------------------------------
// MULTIPLAYER CAMPAIGN PACKET HANDLERS
//

// process a campaign update packet
void multi_campaign_process_update(ubyte *data, header *hinfo);

// send a "campaign finished" packet
void multi_campaign_send_done();

// send a campaign pool status packet
void multi_campaign_send_pool_status();

// send a campaign debrief update packet
void multi_campaign_send_debrief_info();

// send a "start campaign" packet
void multi_campaign_send_start(net_player *pl = /*NULL*/ 0);

// campaign information for ingame joiners
void multi_campaign_send_ingame_start(net_player *pl);
void multi_campaign_process_ingame_start( ubyte *data, header *hinfo );

#endif
