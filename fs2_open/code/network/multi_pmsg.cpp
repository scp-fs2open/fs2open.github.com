/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_pmsg.cpp $
 * $Revision: 2.2 $
 * $Date: 2002-07-22 01:22:25 $
 * $Author: penguin $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 7     3/10/99 6:50p Dave
 * Changed the way we buffer packets for all clients. Optimized turret
 * fired packets. Did some weapon firing optimizations.
 * 
 * 6     3/09/99 6:24p Dave
 * More work on object update revamping. Identified several sources of
 * unnecessary bandwidth.
 * 
 * 5     2/24/99 2:25p Dave
 * Fixed up chatbox bugs. Made squad war reporting better. Fixed a respawn
 * bug for dogfight more.
 * 
 * 4     11/19/98 8:03a Dave
 * Full support for D3-style reliable sockets. Revamped packet lag/loss
 * system, made it receiver side and at the lowest possible level.
 * 
 * 3     11/17/98 11:12a Dave
 * Removed player identification by address. Now assign explicit id #'s.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 18    6/13/98 9:32p Mike
 * Kill last character in file which caused "Find in Files" to report the
 * file as "not a text file."
 * 
 * 17    6/13/98 6:01p Hoffoss
 * Externalized all new (or forgot to be added) strings to all the code.
 * 
 * 16    6/13/98 3:19p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 15    5/13/98 6:54p Dave
 * More sophistication to PXO interface. Changed respawn checking so
 * there's no window for desynchronization between the server and the
 * clients.
 * 
 * 14    5/08/98 5:05p Dave
 * Go to the join game screen when quitting multiplayer. Fixed mission
 * text chat bugs. Put mission type symbols on the create game list.
 * Started updating standalone gui controls.
 * 
 * 13    5/02/98 5:38p Dave
 * Put in new tracker API code. Put in ship information on mp team select
 * screen. Make standalone server name permanent. Fixed standalone server
 * text messages.
 * 
 * 12    4/23/98 6:18p Dave
 * Store ETS values between respawns. Put kick feature in the text
 * messaging system. Fixed text messaging system so that it doesn't
 * process or trigger ship controls. Other UI fixes.
 * 
 * 11    4/22/98 4:59p Allender
 * new multiplayer dead popup.  big changes to the comm menu system for
 * team vs. team.  Start of debriefing stuff for team vs. team  Make form
 * on my wing work with individual ships who have high priority orders
 * 
 * 10    4/16/98 6:35p Dave
 * Display more informative prompt when typing in a text message.
 * 
 * 9     4/06/98 10:24p Dave
 * Fixed up Netgame.respawn for the standalone case.
 * 
 * 8     4/05/98 3:30p Dave
 * Print netplayer messages in brighter green on the hud, with
 * accompanying sound. Echo netplayer messages on sending machine. Fixed
 * standalone sequencing bug where host never get the "enter mission"
 * button.
 * 
 * 7     4/04/98 4:22p Dave
 * First rev of UDP reliable sockets is done. Seems to work well if not
 * overly burdened.
 * 
 * 6     4/03/98 1:03a Dave
 * First pass at unreliable guaranteed delivery packets.
 * 
 * 5     4/02/98 5:50p Dave
 * Put in support for standard comm messages to get sent to netplayers as
 * well as ai ships. Make critical button presses not get evaluated on the
 * observer.
 * 
 * 4     4/01/98 5:56p Dave
 * Fixed a messaging bug which caused msg_all mode in multiplayer not to
 * work. Compile out a host of multiplayer options not available in the
 * demo.
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

#include <ctype.h>
#include "pstypes.h"
#include "multi.h"
#include "multimsgs.h"
#include "multiutil.h"
#include "multi_pmsg.h"
#include "multi_kick.h"
#include "gamesnd.h"
#include "hud.h"
#include "hudmessage.h"
#include "hudsquadmsg.h"
#include "key.h"
#include "timer.h"
#include "ship.h"

// ----------------------------------------------------------------------------------
// MULTI MESSAGING DEFINES/VARS
//

// if a key is down less than this time, fire up the test messaging system, otherwise fire up the voice messaging system
#define MULTI_MSG_KEYDOWN_WAIT				325							// in ms

// sound to play before displaying incoming text messages in-mission
#define MULTI_MSG_TEXT_SOUND					SND_CUE_VOICE

// max length of a string we'll allow players to send
#define MULTI_MSG_MAX_LEN						75

// current message processing mode
int Multi_msg_mode = MULTI_MSG_NONE;

// timestamp for timing keydown
int Multi_msg_stamp = -1;

// flag indicating if there is _still_ a key down for the current message mode
int Multi_msg_repeat_flag = 0;

// timestamp set when we leave messaging mode, use to keep eating keys for a short period of time
int Multi_msg_eat_stamp = -1;

// text message input vars
int Multi_msg_text_enter = 0;
char Multi_msg_text[MULTI_MSG_MAX_TEXT_LEN+1];

// command defines - all these commands must be followed by a ":" so that we can easily tokenize and recognize
// it as a command instead of a word. they also must be immediately at the beginning of a text string
// SO : kick dave        would not work
//      kick: dave       would work
// Finally, if no command is found but there is a ":", it uses the text before the : as an expression to 
// lookup players to route the text to
#define MULTI_MSG_CMD_COUNT					1								// # of commands
#define MULTI_MSG_CMD_KICK						0								// kick command

//XSTR:OFF
char *Multi_msg_commands[MULTI_MSG_CMD_COUNT] = {						// commands themselves
	"kick"	
};
//XSTR:ON

// process an entered line of text and maybe perform a command on it (return 1 if an action was performed, 0 if not)
int multi_msg_check_command(char *str);

// perform the passed command (MULTI_MSG_CMD_* define) with the passed string argument
void multi_msg_perform_command(int command,char *param);


// ----------------------------------------------------------------------------------
// MULTI MESSAGING FUNCTIONS
//

// called when a messaging key has been detected as being pressed
void multi_msg_key_down(int mode)
{		
	// keep eating keys for a short period of time
	if((Multi_msg_eat_stamp != -1) && !timestamp_elapsed(Multi_msg_eat_stamp)){
		return;
	}	

	// if our player flags are marked as being in msg mode, don't do anything
	if(Player->flags & PLAYER_FLAGS_MSG_MODE){
		return;
	}	

	// if there already is a keydown
	if(Multi_msg_mode != MULTI_MSG_NONE){
		// if it is the same as the current mode, set the "still down" flag
		if((mode == Multi_msg_mode) && !Multi_msg_text_enter){			
			Multi_msg_repeat_flag = 1;
		}

		// return here
		return;
	}

	// otherwise set the message mode and set the timestamp
	Multi_msg_mode = mode;
	Multi_msg_repeat_flag = 1;
	Multi_msg_stamp = timestamp(MULTI_MSG_KEYDOWN_WAIT);
}

// returns true when messaging system has determined that we should be messaging with voice
int multi_msg_voice_record()
{
	return ((Multi_msg_mode != MULTI_MSG_NONE) && timestamp_elapsed(Multi_msg_stamp) && Multi_msg_repeat_flag && !Multi_msg_text_enter) ? 1 : 0;
}

// general processing function to do things like timing keydown, etc. call from multi_do_frame()
void multi_msg_process()
{
	// keep eating keys for a short period of time
	if((Multi_msg_eat_stamp != -1) && timestamp_elapsed(Multi_msg_eat_stamp)){
		Multi_msg_eat_stamp = -1;
		return;
	}	
	
	// if we don't currently have a valid mode set, don't do anything
	if(Multi_msg_mode == MULTI_MSG_NONE){
		return;
	}

	// if the key has been released
	if(!Multi_msg_repeat_flag && (Multi_msg_stamp != -1) && !Multi_msg_text_enter){
		// if the timestamp had not yet elapsed, fire up the text messaging system
		// this is the equivalent of a (TAP)
		if(!timestamp_elapsed(Multi_msg_stamp) && !Multi_msg_text_enter){
			// fire up text messaging system here
			Multi_msg_text_enter = 1;			
			memset(Multi_msg_text,0,MULTI_MSG_MAX_TEXT_LEN+1);
		} else {
			Multi_msg_mode = MULTI_MSG_NONE;
			Multi_msg_stamp = -1;		
		}		
	}
	
	// unset the repeat flag every frame
	Multi_msg_repeat_flag = 0;
}

// get the current messaging mode
int multi_msg_mode()
{
	return Multi_msg_mode;
}

// return 0 or 1 if in text chat mode or not
int multi_msg_text_mode()
{
	return Multi_msg_text_enter;
}

// process a text string entered by the local player
void multi_msg_eval_text_msg()
{
	int player_index;
	
	// if its a 0 length string, don't do anything
	if(strlen(Multi_msg_text) <= 0){
		return;
	}

	// evaluate any special commands here
	if(multi_msg_check_command(Multi_msg_text)){
		return;
	}

	// get the player if in MSG_TARGET mode
	if(Multi_msg_mode == MULTI_MSG_TARGET){
		if(Player_ai->target_objnum != -1){			
			player_index = multi_find_player_by_object(&Objects[Player_ai->target_objnum]);
			if(player_index != -1){
				// send the chat packet
				send_game_chat_packet(Net_player, Multi_msg_text, Multi_msg_mode, &Net_players[player_index]);

				// echo the message locally
				multi_msg_display_mission_text(Multi_msg_text, MY_NET_PLAYER_NUM);
			}
		}
	}
	// all other modes
	else {
		// send the chat packet
		send_game_chat_packet(Net_player, Multi_msg_text, Multi_msg_mode, NULL);

		// echo the message locally
		multi_msg_display_mission_text(Multi_msg_text, MY_NET_PLAYER_NUM);
	}
}

// maybe process a keypress in text messaging mode, return true if the key was processed
int multi_msg_text_process(int k)
{
	char str[2];

	// keep eating keys for a short period of time
	if((Multi_msg_eat_stamp != -1) && !timestamp_elapsed(Multi_msg_eat_stamp)){
		return 1;
	}	
	
	// if we're not in text message mode, return 0
	if(!Multi_msg_text_enter){
		return 0;
	}

	switch(k){
	// cancel the message
	case KEY_ESC:	
		multi_msg_text_flush();				
		break;

	// send the message
	case KEY_ENTER:				
		multi_msg_eval_text_msg();		
		multi_msg_text_flush();						
		break;

	// backspace
	case KEY_BACKSP:
		if(strlen(Multi_msg_text) > 0){
			Multi_msg_text[strlen(Multi_msg_text)-1] = '\0';
		}
		break;

	// ignore these individual keys
	case KEY_LSHIFT + KEY_SHIFTED:
	case KEY_RSHIFT + KEY_SHIFTED:
	case KEY_LALT + KEY_SHIFTED:
	case KEY_RALT + KEY_SHIFTED:
	case KEY_LCTRL + KEY_SHIFTED:
	case KEY_RCTRL + KEY_SHIFTED:
		break;

	// stick other printable characters onto the text
	default :					
		// if we're not already at the maximum length
		if(strlen(Multi_msg_text) < MULTI_MSG_MAX_LEN){
			str[0] = (char)key_to_ascii(k);
			str[1] = '\0';
			strcat(Multi_msg_text,str);		
		}
		break;
	}

	return 1;
}

// return 0 or 1 if there is multi text to be rendered (filling in txt if necessary)
int multi_msg_message_text(char *txt)
{
	// if we're not in text message mode, return 0
	if(!Multi_msg_text_enter){
		return 0;
	}

	// put the target of the message at the front of the string
	switch(Multi_msg_mode){
	// messaging all players
	case MULTI_MSG_ALL:
		strcpy(txt,XSTR("ALL : ",694));
		break;

	// messaging friendly players
	case MULTI_MSG_FRIENDLY:
		strcpy(txt,XSTR("FRIENDLY : ",695));
		break;

	// messaging hostile players
	case MULTI_MSG_HOSTILE:
		strcpy(txt,XSTR("HOSTILE : ",696));
		break;

	// messaging targeted ship
	case MULTI_MSG_TARGET:
		strcpy(txt,XSTR("TARGET : ",697));
		break;	

	default :
		Int3();
	}	
	strcat(txt,Multi_msg_text);
	strcat(txt,"_");
	return 1;
}

// display ingame,inmission message text
void multi_msg_display_mission_text(char *msg,int player_index)
{
	// play a cue voice sound
	snd_play(&Snds[MULTI_MSG_TEXT_SOUND]);

	if(MULTI_STANDALONE(Net_players[player_index])){
		HUD_sourced_printf(HUD_SOURCE_NETPLAYER,"%s %s",XSTR("<SERVER>",698),msg);			
	} else {
		HUD_sourced_printf(HUD_SOURCE_NETPLAYER,"%s : %s",Net_players[player_index].player->callsign,msg);			
	}
}

// if the passed net_player's callsign matches the reg expression of the passed expr
int multi_msg_matches_expr(net_player *player,char *expr)
{
	char callsign[CALLSIGN_LEN+1];
	int len,idx;

	// some error checking
	if((player == NULL) || (expr == NULL) || (strlen(expr) <= 0)){
		return 0;
	}

	// get the completely lowercase callsign
	memset(callsign,0,CALLSIGN_LEN+1);
	len = strlen(player->player->callsign);
	for(idx=0;idx<len;idx++){
		callsign[idx] = (char)tolower(player->player->callsign[idx]);
	}

	// see if this guy's callsign matches the expr
	len = strlen(expr);
	for(idx=0;idx<len;idx++){
		// look for non-matching characters
		if(callsign[idx] != expr[idx]){
			return 0;
		}
	}
				
	// matches
	return 1;
}

// if text input mode is active, clear it
void multi_msg_text_flush()
{
	Multi_msg_text_enter = 0;
	Multi_msg_mode = MULTI_MSG_NONE;
	Multi_msg_stamp = -1;		

	// keep eating keys for a short period of time and unset any used control bits
	Multi_msg_eat_stamp = timestamp(350);
	control_config_clear_used_status();
	key_flush();
}


// -----------------------------------------------------------------------------------
// MULTI MESSAGE COMMAND FUNCTIONS
//

// process an entered line of text and maybe perform a command on it (return 1 if an action was performed, 0 if not)
int multi_msg_check_command(char *str)
{
	int idx;
	char *prefix,*predicate,param[MULTI_MSG_MAX_TEXT_LEN+1];

	// look for a colon
	if(strstr(str,":") == NULL){
		return 0;
	}

	// try and find a command prefix
	prefix = NULL;	
	prefix = strtok(str,":");
	if(prefix == NULL){
		return 0;
	}

	// get all the text after the message
	predicate = NULL;
	predicate = strtok(NULL, NOX("\n\0"));
	if(predicate == NULL){
		return 0;
	} 
	
	// store the text as the actual parameter
	strcpy(param,predicate);
	drop_leading_white_space(param);

	// go through all existing commands and see what we can do
	for(idx=0;idx<MULTI_MSG_CMD_COUNT;idx++){
		if(!stricmp(prefix,Multi_msg_commands[idx])){
			// perform the command
			multi_msg_perform_command(idx,param);

			// return true
			return 1;
		}
	}
	
	// apply the results as a general expression, if we're in message all mode
	if(Multi_msg_mode == MULTI_MSG_ALL){
		strcpy(Multi_msg_text,param);

		// send the chat packet
		send_game_chat_packet(Net_player, Multi_msg_text, MULTI_MSG_EXPR,NULL, prefix);

		// echo the message locally
		multi_msg_display_mission_text(Multi_msg_text, MY_NET_PLAYER_NUM);

		// return true
		return 1;
	}	
	
	// no commands performed
	return 0;
}

// perform the passed command (MULTI_MSG_CMD_* define) with the passed string argument
void multi_msg_perform_command(int command,char *param)
{	
	// we may eventually want to split each of these cases into its own function to make things neater
	switch(command){
	// kick a player
	case MULTI_MSG_CMD_KICK:
		int np_index = multi_find_player_by_callsign(param);
		if(np_index != -1){
			multi_kick_player(np_index);
		}
		break;
	}
}


// -----------------------------------------------------------------------------------
// MULTI SQUADMATE MESSAGING FUNCTIONS
//

//XSTR:OFF

char *Multi_msg_subsys_name[SUBSYSTEM_MAX] = {
	"None",
	"Engine",
	"Turret",
	"Bridge",
	"Radar",
	"Navigation",
	"Communication",
	"Weapons",
	"Sensors",
	"Solar Array",
	"Unknown"
};

//XSTR:ON

// display a squadmsg order directed towards _me_
void multi_msg_show_squadmsg(net_player *source,int command,ushort target_sig,int subsys_type)
{
	char hud_string[255];
	char temp_string[100];			
	int should_display;
	object *target_obj;

	// clear the strings
	memset(hud_string,0,255);
	memset(temp_string,0,100);	

	// add the message header
	sprintf(hud_string,XSTR("ORDER FROM <%s> : ",699),source->player->callsign);

	// get the target obj if possible
	target_obj = NULL;
	target_obj = multi_get_network_object(target_sig);	

	should_display = 1;

	// add the command specific text
	switch(command){
	// attack my target
	case ATTACK_TARGET_ITEM :
		if((target_obj != NULL) && (target_obj->type == OBJ_SHIP)){
			sprintf(temp_string,XSTR("Attack %s",700),Ships[target_obj->instance].ship_name);
			strcat(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;

	// disable my target
	case DISABLE_TARGET_ITEM:
		if((target_obj != NULL) && (target_obj->type == OBJ_SHIP)){
			sprintf(temp_string,XSTR("Disable %s",701),Ships[target_obj->instance].ship_name);
			strcat(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;

	// protect my target
	case PROTECT_TARGET_ITEM:
		if((target_obj != NULL) && (target_obj->type == OBJ_SHIP)){
			sprintf(temp_string,XSTR("Protect %s",702),Ships[target_obj->instance].ship_name);
			strcat(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;

	// ignore my target
	case IGNORE_TARGET_ITEM:
		if((target_obj != NULL) && (target_obj->type == OBJ_SHIP)){
			sprintf(temp_string,XSTR("Ignore %s",703),Ships[target_obj->instance].ship_name);
			strcat(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;	

	// disarm my target
	case DISARM_TARGET_ITEM:
		if((target_obj != NULL) && (target_obj->type == OBJ_SHIP)){
			sprintf(temp_string,XSTR("Disarm %s",704),Ships[target_obj->instance].ship_name);
			strcat(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;	

	// disable subsystem on my target
	case DISABLE_SUBSYSTEM_ITEM:
		if((target_obj != NULL) && (target_obj->type == OBJ_SHIP) && (subsys_type != -1) && (subsys_type != 0)){
			sprintf(temp_string,XSTR("Disable subsystem %s on %s",705),Multi_msg_subsys_name[subsys_type],Ships[target_obj->instance].ship_name);
			strcat(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;

	// form on my wing
	case FORMATION_ITEM:		
		strcat(hud_string,XSTR("Form on my wing",706));		
		break;

	// cover me
	case COVER_ME_ITEM:
		strcat(hud_string,XSTR("Cover me",707));
		break;

	// engage enemy
	case ENGAGE_ENEMY_ITEM:
		strcat(hud_string,XSTR("Engage enemy!",708));
		break;

	default :
		should_display =0;
		break;
	}

	// print it out
	if(should_display){
		HUD_printf(hud_string);
	}
}

// evaluate if the given netplayer exists in the passed wingnum
int multi_msg_player_in_wing(int wingnum,net_player *pl)
{
	int idx;
	
	// if this guy doesn't have a valid ship, bail 
	if((pl->player->objnum == -1) || (Objects[pl->player->objnum].type != OBJ_SHIP)){
		return 0;
	}

	// look through all ships in the wing
	for(idx=0;idx<Wings[wingnum].current_count;idx++){
		// if we found a match
		if(Wings[wingnum].ship_index[idx] == Objects[pl->player->objnum].instance){
			return 1;
		}
	}

	return 0;
}

// evaluate if the given netplayer is flying the passed shipnum
int multi_msg_player_in_ship(int shipnum,net_player *pl)
{
	// if we found a matching ship
	if((pl->player->objnum != -1) && (Objects[pl->player->objnum].type == OBJ_SHIP) && (shipnum == Objects[pl->player->objnum].instance)){
		return 1;
	}

	// not a matching ship
	return 0;
}

// send a squadmsg packet to a player
void multi_msg_send_squadmsg_packet(net_player *target,net_player *source,int command,ushort net_sig,int subsys_type)
{
	ubyte data[100];		
	char s_val;
	int packet_size;

	Assert(source != NULL);
	Assert(target != NULL);
	if((source == NULL) || (target == NULL)){
		return;
	}

	// build the header
	BUILD_HEADER(SQUADMSG_PLAYER);

	// add the command and targeting data	
	ADD_DATA(command);

	// add the id of the guy sending the order
	ADD_DATA(source->player_id);

	// net signature
	ADD_DATA(net_sig);
	
	// targeted subsytem (or -1 if none)
	s_val = (char)subsys_type;
	ADD_DATA(s_val);	

	// send to the player	
	multi_io_send_reliable(target, data, packet_size);
}

// evaluate if a wing SQUADMATE MESSAGE command should be sent to a player
// return 0 if at least one ai ship got the order, 1 if only players
int multi_msg_eval_wing_squadmsg(int wingnum,int command,ai_info *aif, int player_num)
{
	int idx;
	ushort net_sig;
	int subsys_type;
	int sent_count;

	// get the index of the sender
	if(player_num == -1)
		player_num = MY_NET_PLAYER_NUM;

	// get the target information
	if(aif->target_objnum == -1){
		net_sig = 0;
	} else {
		net_sig = Objects[aif->target_objnum].net_signature;
	}
	subsys_type = -1;
	if((aif->targeted_subsys == NULL) || (aif->targeted_subsys->system_info == NULL)){
		subsys_type = -1;
	} else {
		subsys_type = aif->targeted_subsys->system_info->type;
	}

	// go through all netplayers and find all matched
	sent_count = Wings[wingnum].current_count;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){			
			// if he is in the wing, send him the message
			if(multi_msg_player_in_wing(wingnum,&Net_players[idx])){
				// if this was the sender himself, just decrement the count
				if(idx == player_num){
					sent_count--;
					continue;
				}

				// if its me, just display locally
				if(&Net_players[idx] == Net_player){
					multi_msg_show_squadmsg(&Net_players[player_num],command,net_sig,subsys_type);						
					sent_count--;
				}
				// otherwise send it to who is supposed to get it
				else {					
					multi_msg_send_squadmsg_packet(&Net_players[idx],&Net_players[player_num],command,net_sig,subsys_type);
					sent_count--;
				}
			}
		}
	}

	// if all the ships which got the message were players, return 1
	return !sent_count;
}

// evaluate if a ship SQUADMATE MESSAGE command should be sent to a player
// return 0 if not sent to a netplayer, 1 if it was
int multi_msg_eval_ship_squadmsg(int shipnum,int command,ai_info *aif, int player_num)
{
	int idx;
	ushort net_sig;
	int subsys_type;

	// get the index of the sender
	if ( player_num == -1 )
		player_num = MY_NET_PLAYER_NUM;

	// get the target information
	if(aif->target_objnum == -1){
		net_sig = 0;
	} else {
		net_sig = Objects[aif->target_objnum].net_signature;
	}
	subsys_type = -1;
	if((aif->targeted_subsys == NULL) || (aif->targeted_subsys->system_info == NULL)){
		subsys_type = -1;
	} else {
		subsys_type = aif->targeted_subsys->system_info->type;
	}

	// go through all netplayers and find all matched
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (idx != player_num)){
			// if he is in the ship, send him the message
			if(multi_msg_player_in_ship(shipnum,&Net_players[idx])){
				// if its me, just display locall
				if(&Net_players[idx] == Net_player){				
					multi_msg_show_squadmsg(&Net_players[player_num],command,net_sig,subsys_type);						

					return 1;
				}
				// otherwise send it to who is supposed to get it
				else {
					multi_msg_send_squadmsg_packet(&Net_players[idx],&Net_players[player_num],command,net_sig,subsys_type);
					return 1;
				}													
			}
		}
	}

	// this will let the messaging system show a response to the sender of the packet
	return 0;
}

// process incoming squadmate messaging info 
void multi_msg_process_squadmsg_packet(unsigned char *data, header *hinfo)
{	
	int command;
	ushort net_sig;
	short source_id;
	int source_index;
	char s_val;
	int offset = HEADER_LENGTH;

	// get all packet data
	GET_DATA(command);
	GET_DATA(source_id);
	GET_DATA(net_sig);
	GET_DATA(s_val);
	PACKET_SET_SIZE();

	// determine who the order is from
	source_index = find_player_id(source_id);
	if(source_index == -1){
		nprintf(("Network","Received squadmsg order packet from unknown player!!\n"));
		return;
	}

	// display the squadmessage somehow
	multi_msg_show_squadmsg(&Net_players[source_index],command,net_sig,(int)s_val);
}
