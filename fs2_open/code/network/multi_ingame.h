/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_ingame.h $
 * $Revision: 2.2 $
 * $Date: 2004-08-11 05:06:29 $
 * $Author: Kazan $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/03/05 09:02:02  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 12    5/20/98 3:25p Allender
 * ingame join changes (which probably won't make the final version).
 * Added RAS code into psnet
 * 
 * 11    3/24/98 5:12p Allender
 * ingame join packet sequencing
 * 
 * 10    3/14/98 2:48p Dave
 * Cleaned up observer joining code. Put in support for file xfers to
 * ingame joiners (observers or not). Revamped and reinstalled pseudo
 * lag/loss system.
 * 
 * 9     3/13/98 2:51p Dave
 * Put in support for observers to join ingame.
 * 
 * 8     3/11/98 11:42p Allender
 * more ingame join stuff.  Fix to networking code to possibly
 * reinitialize reliable socket when entering join screen
 * 
 * 7     3/06/98 9:33a Allender
 * more ingame join stuff.  Wing packets are done.
 * 
 * 6     3/03/98 8:23p Allender
 * first pass of getting ingame join to a better state.  Started rewriting
 * the ship/wing list code
 * 
 * 5     2/05/98 11:10a Dave
 * Fixed an ingame join bug. Fixed a read-only file problem with
 * multiplayer file xfer.
 * 
 * 4     1/29/98 5:24p Dave
 * Made ingame join handle bad packets gracefully
 * 
 * 3     1/22/98 5:26p Dave
 * Modified some pregame sequencing packets. Starting to repair broken
 * standalone stuff.
 * 
 * 2     1/21/98 5:58p Dave
 * Finished ingame join. Coded in multiplayer interface artwork changes.
 * 
 * 1     1/20/98 5:41p Dave
 * Seperated ingame join functionality into its own module.
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _MULTI_INGAME_JOIN_HEADER_FILE
#define _MULTI_INGAME_JOIN_HEADER_FILE

#include "globalincs/pstypes.h"

struct ship;
struct wing;
struct net_player;
struct header;

// --------------------------------------------------------------------------------------------------
// DAVE's BIGASS INGAME JOIN WARNING/DISCLAIMER
//
// Ingame joining is another delicate system. Although not as delicate as server transfer, it will
// help to take as many precautions as possible when handling ingame joins. Please be sure to follow
// all the same rules as explained in multi_strans.h
//
// --------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------
// INGAME JOIN DESCRIPTION
//
// 1.) Joiner sends a JOIN packet to the server
// 2.) If the server accepts him, he receives an ACCEPT packet in return
// 3.) The client then moves into the INGAME_SYNC state to begin receiving data from the server
// 4.) The first thing he does on this screen is send his filesig packet to the server. At which 
//     point the server will either let him in or deny him. There are no file transfers ingame.
// 5.) The server calls multi_handle_ingame_joiners() once per frame, through multi_do_frame()
// 6.) After verifiying or kicking the player because of his file signature, the server tells the
//     player to load the mission
// 7.) When the mission is loaded, the server, sends a netgame update to the client
// 8.) Without waiting, the server then begins sending data ship packets to the player
// 9.) Upon confirmation of receiving these packets, the server sends wing data packets
// 10.) Upon completion of this, the server sends respawn point packets
// 11.) Upon completion of this, the server sends a post briefing data block packet containing ship class and 
//      weapon information
// 12.) After this, the server sends a player settings packet (to all players for good measure)
// 13.) At this point, the server sends a jump into mission packet
// 14.) Upon receipt of this packet, the client moves into the ingame ship select state
// 15.) The first thing the client does in this state is load the mission data (textures, etc)
// 16.) The player is presented with a list of ships he can choose from. He selects one and sends
//      an INGAME_SHIP_REQUEST to the server. 
// 17.) The server checks to see if this request is acceptable and sends an INGAME_SHIP_REQUEST back
//      with the appropriate data.
// 18.) If the client received an affirmative, he selects the ship and jumps into the mission, otherwise
//      he removes it from the list and tries for another ship
// --------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------
// INGAME JOIN DEFINES
//

// ingame join defines - NOTE : it is important to keep these flags so that they appear
// numerically in the order in which the events they represent are done
#define INGAME_JOIN_FLAG_SENDING_SETS		(1<<0)	// sending player settings to him - it is important that this is done _first_
#define INGAME_JOIN_FLAG_CAMPAIGN_INFO		(1<<1)	// sending player settings to him - it is important that this is done _first_
#define INGAME_JOIN_FLAG_LOADING_MISSION	(1<<2)	// player has finished loading the mission
#define INGAME_JOIN_FLAG_SENDING_SHIPS		(1<<3)	// sending ships to an ingame joiner
#define INGAME_JOIN_FLAG_SENDING_WINGS		(1<<4)	// sending wings to an ingame joiner
#define INGAME_JOIN_FLAG_SENDING_RPTS		(1<<5)	// sending respawn points to a player
#define INGAME_JOIN_FLAG_SENDING_POST		(1<<6)	// sending standard post briefing data block
#define INGAME_JOIN_FLAG_SENDING_WSS		(1<<7)	// sending wss slots info
#define INGAME_JOIN_FLAG_PICK_SHIP			(1<<8)	// player is in the "pick" ship screen
#define INGAME_JOIN_FLAG_FILE_XFER			(1<<9)	// player is in the process of downloading the mission file

#define INGAME_SHIP_UPDATE_TIME				1500		// update time information for all ships the ingame joiner can see

#define INGAME_SHIP_NEXT						0			// another ship to follow
#define INGAME_SHIP_WARP_SUPPORT				1			// support ship warping in
#define INGAME_SHIP_LIST_EOP					2			// end of packet
#define INGAME_SHIP_LIST_EOL					3			// end of list

#define INGAME_WING_NEXT						0			// another wing to follow
#define INGAME_WING_LIST_EOP					1			// end of packet
#define INGAME_WING_LIST_EOL					2			// end of list

// defines used for the ingame wings packet
#define INGAME_WING_NOT_ARRIVED				1			// wing not yet present
#define INGAME_WING_DEPARTED					2			// wing is gone -- never to be seen again
#define INGAME_WING_PRESENT					3			// wing is in mission


// --------------------------------------------------------------------------------------------------
// INGAME JOIN SERVER FUNCTIONS
//

// called on the server to process ingame joiners and move them through the motions of ingame joining
void multi_handle_ingame_joiners();  

// pack the ship into the data string and return bytes processed
int multi_deconstruct_ship(ubyte *data, ship *s);

// pack the wing into the data string and return bytes processed
int multi_deconstruct_wing(ubyte *data, wing *w);

// --------------------------------------------------------------------------------------------------
// INGAME JOIN CLIENT FUNCTIONS
//

// unpack a ship from the data string and return bytes processed
int multi_reconstruct_ship(ubyte *data);

// unpack a wing from the data string and return bytes processed
int multi_reconstruct_wing(ubyte *data);   

// the final step for an ingame joining observer - create my observer object, unflag myself as joining and jump into mission
void multi_ingame_observer_finish();


// --------------------------------------------------------------------------------------------------
// INGAME DATA SYNC SCREEN 
//

// mission sync screen init function for ingame joining
void multi_ingame_sync_init();

// mission sync screen do function for ingame joining
void multi_ingame_sync_do();

// mission sync screen do function for ingame joining
void multi_ingame_sync_close();


// --------------------------------------------------------------------------------------------------
// INGAME SHIP SELECT SCREEN
//

// ingame join ship selection screen init
void multi_ingame_select_init();

// ingame join ship selection screen do
void multi_ingame_select_do();

// ingame join ship selection screen close
void multi_ingame_select_close();


// --------------------------------------------------------------------------------------------------
// PACKET HANDLER functions
// these are also defined in multimsgs.h, but the implementations are in the module for the sake of convenience
//

// send ship information for the mission to the ingame joiner
void send_ingame_ships_packet(net_player *pl);

// process ship information for the mission
void process_ingame_ships_packet(ubyte *data, header *hinfo);

// send wing information for the mission to the ingame joiner
void send_ingame_wings_packet(net_player *pl);

// process wing information for the mission
void process_ingame_wings_packet(ubyte *data, header *hinfo);

// send respawn points information to the ingame joiner
void send_ingame_respawn_points_packet(net_player *pl = /*NULL*/ 0);

// process respawn points information for the mission
void process_ingame_respawn_points_packet(ubyte *data, header *hinfo);

// send Wss_slots data to an ingame joiner
void send_ingame_slots_packet(net_player *p);

// process Wss_slots data for the mission
void process_ingame_slots_packet(ubyte *data, header *hinfo);

// send a request or a reply regarding ingame join ship choice
void send_ingame_ship_request_packet(int code,int rdata,net_player *pl = NULL);

// process an ingame ship request packet
void process_ingame_ship_request_packet(ubyte *data, header *hinfo);

// for extra mission information
void multi_ingame_process_mission_stuff( ubyte *data, header *hinfo );

#endif
