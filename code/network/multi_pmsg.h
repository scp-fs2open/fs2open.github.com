/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
