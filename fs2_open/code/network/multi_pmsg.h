/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_pmsg.h $
 * $Revision: 2.2 $
 * $Date: 2005-07-13 03:35:32 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:29  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
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
 * 6     5/08/98 5:05p Dave
 * Go to the join game screen when quitting multiplayer. Fixed mission
 * text chat bugs. Put mission type symbols on the create game list.
 * Started updating standalone gui controls.
 * 
 * 5     4/22/98 4:59p Allender
 * new multiplayer dead popup.  big changes to the comm menu system for
 * team vs. team.  Start of debriefing stuff for team vs. team  Make form
 * on my wing work with individual ships who have high priority orders
 * 
 * 4     4/02/98 5:50p Dave
 * Put in support for standard comm messages to get sent to netplayers as
 * well as ai ships. Make critical button presses not get evaluated on the
 * observer.
 * 
 * 3     3/27/98 11:57a Dave
 * Put in expression checking for text messages.
 * 
 * 2     3/25/98 2:16p Dave
 * Select random default image for newly created pilots. Fixed several
 * multi-pause messaging bugs. Begin work on online help for multiplayer
 * keys.
 * 
 * 1     3/19/98 5:04p Dave
 *  
 * 
 * $NoKeywords: $
 */

#ifndef _MULTIPLAYER_MESSAGING_HEADER_FILE
#define _MULTIPLAYER_MESSAGING_HEADER_FILE


// ----------------------------------------------------------------------------------
// MULTI MESSAGING DEFINES/VARS
//

struct net_player;
struct ai_info;
struct header;

// messaging modes
// netgame messaging settings
#define MULTI_MSG_NONE						-1							// not in messaging mode (send to no one)
#define MULTI_MSG_ALL						0							// all players in the game
#define MULTI_MSG_FRIENDLY					1							// friendly ships
#define MULTI_MSG_HOSTILE					2							// hostile ships
#define MULTI_MSG_TARGET					3							// to whatever is my targeted ship (if a player)
#define MULTI_MSG_EXPR						4							// send to all players whose callsigns match the expr

// max length for an entered text message
#define MULTI_MSG_MAX_TEXT_LEN			255						


// ----------------------------------------------------------------------------------
// MULTI MESSAGING FUNCTIONS
//

// called when a messaging key has been detected as being pressed
void multi_msg_key_down(int mode);

// returns true when messaging system has determined that we should be messaging with voice
int multi_msg_voice_record();

// general processing function to do things like timing keydown, etc. call from multi_do_frame()
void multi_msg_process();

// get the current messaging mode
int multi_msg_mode();

// maybe process a keypress in text messaging mode, return true if the key was processed
int multi_msg_text_process(int k);

// return 0 or 1 if in text chat mode or not
int multi_msg_text_mode();

// return 0 or 1 if there is multi text to be rendered (filling in txt if necessary)
int multi_msg_message_text(char *txt);

// display ingame,inmission message text
void multi_msg_display_mission_text(char *msg,int player_index);

// if the passed net_player's callsign matches the reg expression of the passed expr
int multi_msg_matches_expr(net_player *player,char *expr);

// if text input mode is active, clear it
void multi_msg_text_flush();


// -----------------------------------------------------------------------------------
// MULTI SQUADMATE MESSAGING FUNCTIONS
//

// evaluate if a wing SQUADMATE MESSAGE command should be sent to a player
// return 0 if at least one ai ship got the order, 1 if only players
int multi_msg_eval_wing_squadmsg(int wingnum,int command,ai_info *aif,int player_num);

// evaluate if a ship SQUADMATE MESSAGE command should be sent to a player
// return 0 if not sent to a netplayer, 1 if it was
int multi_msg_eval_ship_squadmsg(int shipnum,int command,ai_info *aif, int player_num);

// process incoming squadmate messaging info 
void multi_msg_process_squadmsg_packet(unsigned char *data, header *hinfo);

#endif
