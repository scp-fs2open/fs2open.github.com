/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include <ctype.h>

#include "network/multi_pmsg.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multi_kick.h"
#include "gamesnd/gamesnd.h"
#include "hud/hudmessage.h"
#include "hud/hudsquadmsg.h"
#include "io/key.h"
#include "io/timer.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "object/object.h"
#include "parse/parselo.h"
#include "sound/fsspeech.h"



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
		if(Multi_msg_text[0] != '\0'){
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
			strcat_s(Multi_msg_text,str);		
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
		strcpy(txt, XSTR("ALL : ",694));
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
void multi_msg_display_mission_text(const char *msg, int player_index)
{
	// play a cue voice sound and text to speech if not from this player
	if(Net_players[player_index].player_id != MY_NET_PLAYER_NUM) {
		snd_play(&Snds[MULTI_MSG_TEXT_SOUND]);
		fsspeech_play(FSSPEECH_FROM_MULTI, msg);
	}

	if(MULTI_STANDALONE(Net_players[player_index])){
		HUD_sourced_printf(HUD_SOURCE_NETPLAYER,"%s %s",XSTR("<SERVER>", 698), msg);			
	} else {
		HUD_sourced_printf(HUD_SOURCE_NETPLAYER,"%s: %s", Net_players[player_index].m_player->callsign, msg);			
	}
}

// if the passed net_player's callsign matches the reg expression of the passed expr
int multi_msg_matches_expr(net_player *np, const char *expr)
{
	// some error checking
	if((np == NULL) || (expr == NULL) || (strlen(expr) <= 0)){
		return 0;
	}

	return stricmp(expr, np->m_player->callsign) ? 0 : 1 ; 
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
	char *prefix = NULL, *predicate = NULL, param[MULTI_MSG_MAX_TEXT_LEN+1];
	char temp_str[MULTI_MSG_MAX_TEXT_LEN+1];

	// look for a colon
	if(strstr(str,":") == NULL){
		return 0;
	}

	// we don't want to modify the original string, which strtok() does
	strcpy_s(temp_str, str);

	// try and find a command prefix
	prefix = strtok(temp_str, ":");
	if (prefix == NULL)
		return 0;

	// get all the text after the message
	predicate = strtok(NULL, NOX("\n\0"));
	if (predicate == NULL)
		return 0;
	
	// store the text as the actual parameter
	strcpy_s(param, predicate);
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
		strcpy_s(Multi_msg_text,param);

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
	sprintf(hud_string,XSTR("ORDER FROM <%s> : ",699),source->m_player->callsign);

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
			strcat_s(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;

	// disable my target
	case DISABLE_TARGET_ITEM:
		if((target_obj != NULL) && (target_obj->type == OBJ_SHIP)){
			sprintf(temp_string,XSTR("Disable %s",701),Ships[target_obj->instance].ship_name);
			strcat_s(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;

	// protect my target
	case PROTECT_TARGET_ITEM:
		if((target_obj != NULL) && (target_obj->type == OBJ_SHIP)){
			sprintf(temp_string,XSTR("Protect %s",702),Ships[target_obj->instance].ship_name);
			strcat_s(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;

	// ignore my target
	case IGNORE_TARGET_ITEM:
		if((target_obj != NULL) && (target_obj->type == OBJ_SHIP)){
			sprintf(temp_string,XSTR("Ignore %s",703),Ships[target_obj->instance].ship_name);
			strcat_s(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;	

	// disarm my target
	case DISARM_TARGET_ITEM:
		if((target_obj != NULL) && (target_obj->type == OBJ_SHIP)){
			sprintf(temp_string,XSTR("Disarm %s",704),Ships[target_obj->instance].ship_name);
			strcat_s(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;	

	// disable subsystem on my target
	case DISABLE_SUBSYSTEM_ITEM:
		if((target_obj != NULL) && (target_obj->type == OBJ_SHIP) && (subsys_type != -1) && (subsys_type != 0)){
			sprintf(temp_string,XSTR("Disable subsystem %s on %s",705),Multi_msg_subsys_name[subsys_type],Ships[target_obj->instance].ship_name);
			strcat_s(hud_string,temp_string);
		} else {
			should_display = 0;
		}
		break;

	// form on my wing
	case FORMATION_ITEM:		
		strcat_s(hud_string,XSTR("Form on my wing",706));		
		break;

	// cover me
	case COVER_ME_ITEM:
		strcat_s(hud_string,XSTR("Cover me",707));
		break;

	// engage enemy
	case ENGAGE_ENEMY_ITEM:
		strcat_s(hud_string,XSTR("Engage enemy!",708));
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
	if((pl->m_player->objnum == -1) || (Objects[pl->m_player->objnum].type != OBJ_SHIP)){
		return 0;
	}

	// look through all ships in the wing
	for(idx=0;idx<Wings[wingnum].current_count;idx++){
		// if we found a match
		if(Wings[wingnum].ship_index[idx] == Objects[pl->m_player->objnum].instance){
			return 1;
		}
	}

	return 0;
}

// evaluate if the given netplayer is flying the passed shipnum
int multi_msg_player_in_ship(int shipnum,net_player *pl)
{
	// if we found a matching ship
	if((pl->m_player->objnum != -1) && (Objects[pl->m_player->objnum].type == OBJ_SHIP) && (shipnum == Objects[pl->m_player->objnum].instance)){
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
	ADD_INT(command);

	// add the id of the guy sending the order
	ADD_SHORT(source->player_id);

	// net signature
	ADD_USHORT(net_sig);
	
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
	GET_INT(command);
	GET_SHORT(source_id);
	GET_USHORT(net_sig);
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
