/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_endgame.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:00 $
 * $Author: penguin $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 3     8/22/99 1:55p Dave
 * Cleaned up host/team-captain leaving code.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 8     9/14/98 3:40p Allender
 * better error checking for invalid number of waves for player wings in a
 * multiplayer game.  Better popup message in FreeSpace side.
 * 
 * 7     9/11/98 4:14p Dave
 * Fixed file checksumming of < file_size. Put in more verbose kicking and
 * PXO stats store reporting.
 * 
 * 6     7/24/98 9:27a Dave
 * Tidied up endgame sequencing by removing several old flags and
 * standardizing _all_ endgame stuff with a single function call.
 * 
 * 5     7/13/98 5:19p Dave
 * 
 * 4     5/17/98 6:32p Dave
 * Make sure clients/servers aren't kicked out of the debriefing when team
 * captains leave a game. Fixed chatbox off-by-one error. Fixed image
 * xfer/pilot info popup stuff.
 * 
 * 3     5/03/98 7:04p Dave
 * Make team vs. team work mores smoothly with standalone. Change how host
 * interacts with standalone for picking missions. Put in a time limit for
 * ingame join ship select. Fix ingame join ship select screen for Vasudan
 * ship icons.
 * 
 * 2     4/30/98 5:12p Dave
 * Fixed game polling code for joining clients. Reworked some file xfer
 * stuff.
 * 
 * 1     4/22/98 5:50p Dave
 *  
 * 
 * $NoKeywords: $
 */

#ifndef _MULTI_ENDGAME_HEADER_FILE
#define _MULTI_ENDGAME_HEADER_FILE

// ----------------------------------------------------------------------------------------------------------
// Put all functions/data related to leaving a netgame, handling players leaving, handling the server leaving,
// and notifying the user of all of these actions, here.
//


// ----------------------------------------------------------------------------------------------------------
// MULTI ENDGAME DEFINES/VARS
//

// defines for calling multi_quit_game(...)
#define PROMPT_NONE									0				// don't prompt anyone when quitting (multi_quit_game)
#define PROMPT_HOST									1				// prompt the host when quitting (multi_quit_game)
#define PROMPT_CLIENT								2				// prompt the client when quitting (multi_quit_game)
#define PROMPT_ALL									3				// prompt any players when quitting (multi_quit_game)

// notification defines for calling multi_quit_game(...)
#define MULTI_END_NOTIFY_NONE						(-1)			// no notification code
#define MULTI_END_NOTIFY_KICKED					1				// player was kicked
#define MULTI_END_NOTIFY_SERVER_LEFT			2				// server has left the game
#define MULTI_END_NOTIFY_FILE_REJECTED			3				// mission file was rejected by the server
#define MULTI_END_NOTIFY_EARLY_END				4				// game ended while the ingame joiner was joining
#define MULTI_END_NOTIFY_INGAME_TIMEOUT		5				// waited too long in the ship select screen
#define MULTI_END_NOTIFY_KICKED_BAD_XFER		6				// kicked because file xfer failed
#define MULTI_END_NOTIFY_KICKED_CANT_XFER		7				// kicked because can't xfer a builtin mission
#define MULTI_END_NOTIFY_KICKED_INGAME_ENDED	8				// kicked because was ingame joining in an ending game

// error defines for calling multi_quit_game(...)
#define MULTI_END_ERROR_NONE						(-1)			// no error code
#define MULTI_END_ERROR_CONTACT_LOST			1				// contact with the server has been lost
#define MULTI_END_ERROR_CONNECT_FAIL			2				// failed to connect to the server
#define MULTI_END_ERROR_LOAD_FAIL				3				// failed to load the mission properly
#define MULTI_END_ERROR_INGAME_SHIP				4				// unable to create ingame join player ship
#define MULTI_END_ERROR_INGAME_BOGUS			5				// received bogus data on ingame join
#define MULTI_END_ERROR_STRANS_FAIL				6				// server transfer failed (obsolete)
#define MULTI_END_ERROR_SHIP_ASSIGN				7				// server had problems assigning players to ships
#define MULTI_END_ERROR_HOST_LEFT				8				// host has left a standalone game
#define MULTI_END_ERROR_XFER_FAIL				9				// mission file xfer failed on the client
#define MULTI_END_ERROR_WAVE_COUNT				10				// illegal data found in mission when parsing
#define MULTI_END_ERROR_TEAM0_EMPTY				11				// all of team 0 has left
#define MULTI_END_ERROR_TEAM1_EMPTY				12				// all of team 1 has left
#define MULTI_END_ERROR_CAPTAIN_LEFT			13				// captain of a team has left while not ingame

// ----------------------------------------------------------------------------------------------------------
// MULTI ENDGAME FUNCTIONS
//

// initialize the endgame processor (call when joining/starting a new netgame)
void multi_endgame_init();

// process all endgame related events
void multi_endgame_process();

// if the game has been flagged as ended (ie, its going to be reset)
int multi_endgame_ending();

// general quit function, with optional notification, error, and winsock error codes
// return 0 if the act was cancelled, 1 if it was accepted
int multi_quit_game(int prompt,int notify_code = MULTI_END_NOTIFY_NONE,int err_code = MULTI_END_ERROR_NONE,int wsa_error = -1);


#endif
