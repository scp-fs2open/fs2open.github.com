/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_campaign.h $
 * $Revision: 2.1 $
 * $Date: 2004-03-05 09:02:02 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 7     5/21/98 12:14a Allender
 * fix ingame join problems
 * 
 * 6     5/19/98 8:35p Dave
 * Revamp PXO channel listing system. Send campaign goals/events to
 * clients for evaluation. Made lock button pressable on all screens. 
 * 
 * 5     5/05/98 5:02p Dave
 * Fix end-of-campaign sequencing to work right. Make all individual
 * missions of a campaign replayable.
 * 
 * 4     5/05/98 2:10p Dave
 * Verify campaign support for testing. More new tracker code.
 * 
 * 3     2/23/98 11:09p Dave
 * Finished up multiplayer campaign support. Seems bug-free.
 * 
 * 2     2/22/98 2:53p Dave
 * Put in groundwork for advanced multiplayer campaign  options.
 * 
 * 1     2/20/98 4:39p Dave
 * Split up mp campaign functionality into its own module.
 *  
 * $NoKeywords: $
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
