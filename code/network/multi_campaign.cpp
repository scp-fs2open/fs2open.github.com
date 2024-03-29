/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/





#include <climits>		// this is need even when not building debug!!

#include "gamesequence/gamesequence.h"
#include "network/multi.h"
#include "network/multiui.h"
#include "freespace.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "popup/popup.h"
#include "network/multi_campaign.h"
#include "network/multi_endgame.h"
#include "network/stand_gui.h"
#include "mission/missiongoals.h"
#include "mission/missioncampaign.h"
#include "mission/missionparse.h"
#include "ship/ship.h"	// Needed for Ship_info.size() -MageKing17
#include "weapon/weapon.h"	// Likewise, needed for Weapon_info.size() -Goober5000



// ------------------------------------------------------------------------------------
// MULTIPLAYER CAMPAIGN DEFINES/VARS
//

// packet codes
#define MC_CODE_POOL									0			// pool/weapons update
#define MC_CODE_DONE									1			// campaign is "done"
#define MC_CODE_DEBRIEF								2			// debrief info
#define MC_CODE_START								3			// start campaign information
#define MC_CODE_MISSION_NAMES						4			// mission names

// packet code for ingame joining
#define MC_JIP_INITIAL_PACKET						1			// initial data 
#define MC_JIP_GE_STATUS							2			// goal event status for all missions
#define MC_JIP_GOAL_NAMES							3			// goal names
#define MC_JIP_EVENT_NAMES							4			// event names
#define MC_JIP_MISSION_NAMES						5			// mission names
#define MC_JIP_END_DATA								6			// this is the end of the data


#define MC_INGAME_DATA_SLOP						50


// flags indicating the "accept" status of all players involved in a campaign
int Multi_campaign_accept_flags[MAX_PLAYERS];

// ------------------------------------------------------------------------------------
// MULTIPLAYER CAMPAIGN FUNCTIONS
//

// load a new campaign file or notify the standalone if we're not the server
void multi_campaign_start(char *filename)
{
	char str[255];
	
	// set the netgame mode
	Netgame.campaign_mode = MP_CAMPAIGN;		
	
	// set the campaign filename
	strcpy_s(Netgame.campaign_name,filename);

	// add the campaign mode flag
	Game_mode |= GM_CAMPAIGN_MODE;

	// if we're the server of the game we should also be loading the campaign up. otherwise, we let the standalone do it
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){	
		// start the campaign, passing 0 so we do _not_ load the savefile. this is only for starting
		// new campaigns
		mission_campaign_load(filename);
		mission_campaign_next_mission();
			
		// setup various filenames and mission names
		strcpy_s(Netgame.mission_name,Campaign.missions[Campaign.current_mission].name);
		strcpy_s(Netgame.campaign_name,filename);
		strcpy_s(Game_current_mission_filename,Netgame.mission_name);

		// if we're the standalone server, set the mission and campaign names
		if(Game_mode & GM_STANDALONE_SERVER){
			memset(str,0,255);
			strcpy_s(str,Netgame.mission_name);
			strcat_s(str," (");
			strcat_s(str,Netgame.campaign_name);
			strcat_s(str,")");

			// set the control on the stand_gui
			std_multi_set_standalone_mission_name(str);
		}

		// maybe override the Netgame.respawn setting			
		Netgame.respawn = The_mission.num_respawns;
		nprintf(("Network","MULTI CAMPAIGN : overriding respawn setting with mission max %d\n",The_mission.num_respawns));		

		// send a "start campaign" packet
		multi_campaign_send_start();
	}
}

// client-side start of a campaign
void multi_campaign_client_start()
{
	mission_campaign_clear();

	// set campaign mode. why not.
	Game_mode |= GM_CAMPAIGN_MODE;

	// also, set the campaign type correctly so that we can also rely on that info
	if (Netgame.type_flags & NG_TYPE_TVT) {
		Campaign.type = CAMPAIGN_TYPE_MULTI_TEAMS;
	} else {
		Campaign.type = CAMPAIGN_TYPE_MULTI_COOP;
	}
}

// move everything and eveyrone into the next mission state
void multi_campaign_next_mission()
{
	char str[255];

	// flush the important data
	multi_campaign_flush_data();

	// call the campaign over function
	mission_campaign_mission_over();		
	
	// now we should be sequencing through the next stage (mission load, etc)
	// this will eventually be replaced with the real filename of the next mission
	if(Campaign.current_mission != -1){
		strcpy_s(Game_current_mission_filename, Campaign.missions[Campaign.current_mission].name);
		strcpy_s(Netgame.mission_name,Game_current_mission_filename);			

		// if we're the standalone server, set the mission and campaign names
		if(Game_mode & GM_STANDALONE_SERVER){
			memset(str,0,255);
			strcpy_s(str,Netgame.mission_name);
			strcat_s(str," (");
			strcat_s(str,Netgame.campaign_name);
			strcat_s(str,")");

			// set the control on the stand_gui
			std_multi_set_standalone_mission_name(str);
		}
	}
}

// flush all important data between missions
void multi_campaign_flush_data()
{	
	// blast the accept flags
	memset(Multi_campaign_accept_flags,0,sizeof(int) * MAX_PLAYERS);

	// flush mission stuff
	multi_flush_mission_stuff();
}

// call in the debriefing stage to evaluate what we should be doing in regards to the campaign
// if player_status == 0, nothing should be done
//                  == 1, players want to continue to the next mission
//                  == 2, players want to repeat the previous mission
void multi_campaign_do_debrief(int player_status)
{
	// the server (standalone or no)
	if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Campaign.current_mission != -1) && player_status){				
		// if players want to go to the next mission
		if(player_status == 1){
			// move the multiplayer campaign along
			multi_campaign_next_mission();			

			// if we're at the end of the campaign
			if(Campaign.current_mission == -1){
				// set the netgame state to be forming and continue
				Netgame.game_state = NETGAME_STATE_FORMING;
				send_netgame_update_packet();

				if(Game_mode & GM_STANDALONE_SERVER){
					gameseq_post_event(GS_EVENT_STANDALONE_MAIN);
				} else {
					gameseq_post_event(GS_EVENT_MULTI_HOST_SETUP);
				}
				multi_reset_timestamps();
			}
			// if we're still in the campaign
			else {
				Netgame.game_state = NETGAME_STATE_MISSION_SYNC;
				send_netgame_update_packet();

				Multi_sync_mode = MULTI_SYNC_PRE_BRIEFING;
				gameseq_post_event(GS_EVENT_MULTI_MISSION_SYNC);

				multi_reset_timestamps();
			}
		}
		// if the players want to replay the current mission
		else if(player_status == 2){
			Netgame.game_state = NETGAME_STATE_MISSION_SYNC;
			send_netgame_update_packet();

			Multi_sync_mode = MULTI_SYNC_PRE_BRIEFING;
			gameseq_post_event(GS_EVENT_MULTI_MISSION_SYNC);

			multi_reset_timestamps();
		} else {
			Int3();
		}								
	}			
}

// display the done popup
void multi_campaign_done_popup()
{
	popup(PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("The Campaign Is Complete. Thank You For Playing",643));
	multi_quit_game(PROMPT_NONE);
}

// evaluate post mission goal stuff for the campaign and send all relevant stuff to clients
void multi_campaign_eval_debrief()
{
	// evaluate mission stuff (fills in structures, etc).
	// DKA 12/12/98 already done 
	// mission_campaign_eval_next_mission();

	// send the campaign debriefing packet
	multi_campaign_send_debrief_info();
}


// ------------------------------------------------------------------------------------
// MULTIPLAYER CAMPAIGN PACKET HANDLERS
//
// process a campaign update packet
void multi_campaign_process_update(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	char fname[255];
	ubyte val;
	ushort s_val, spool_size, wpool_size;
	ubyte code;
	ubyte cur_mission,next_mission;
	ubyte item_count, starting_num;
	ubyte num_goals;
	ushort num_events;
	int idx;

	// get the packet code
	GET_DATA(code);	

	switch(code){
	case MC_CODE_DONE:
		// display the done popup
		multi_campaign_done_popup();
		break;

	case MC_CODE_POOL:
		// get the campaign status byte
		GET_DATA(val);

		// if we're not in campaign mode, bash all weapons and ships to be "allowed"
		if(!val){
			// all ships
			for(idx = 0; idx < ship_info_size(); idx++) {
				Campaign.ships_allowed[idx] = 1;
			}

			// all weapons
			for(idx = 0; idx < weapon_info_size(); idx++) {
				Campaign.weapons_allowed[idx] = 1;
			}
		} else {
			// clear the ships and weapons allowed arrays
			memset(Campaign.ships_allowed,0,MAX_SHIP_CLASSES);
			memset(Campaign.weapons_allowed,0,MAX_WEAPON_TYPES);

			// get all ship classes
			GET_USHORT(spool_size);
			for(idx=0;idx<spool_size;idx++){
				GET_USHORT(s_val);

				if (s_val < MAX_SHIP_CLASSES) {
					Campaign.ships_allowed[s_val] = 1;
				}
			} 
	
			// get all weapon classes
			GET_USHORT(wpool_size);
			for(idx=0;idx<wpool_size;idx++){
				GET_USHORT(s_val);

				if (s_val < MAX_WEAPON_TYPES) {
					Campaign.weapons_allowed[s_val] = 1;
				}
			}
		}	

		// ack the server
		Net_player->state = NETPLAYER_STATE_CPOOL_ACK;
		send_netplayer_update_packet();
		break;

	case MC_CODE_DEBRIEF:
		GET_DATA(cur_mission);
		GET_DATA(next_mission);		

		// add the filename		
		GET_STRING(fname);
		Campaign.missions[cur_mission].name = vm_strdup(fname);
	
		// add the # of goals and events
		GET_DATA(val);
		num_goals = val;
		GET_USHORT(s_val);
		num_events = s_val;

		// add the goals
		Campaign.missions[cur_mission].goals.clear();
		for (idx = 0; idx < num_goals; idx++) {
			GET_DATA(val);

			Campaign.missions[cur_mission].goals.emplace_back();
			auto& stored_goal = Campaign.missions[cur_mission].goals.back();

			strncpy_s(stored_goal.name, Mission_goals[idx].name.c_str(), NAME_LENGTH - 1);
			stored_goal.status = static_cast<char>(val);
		}

		// add the events
		Campaign.missions[cur_mission].events.clear();
		for (idx = 0; idx < num_events; idx++) {
			GET_DATA(val);

			Campaign.missions[cur_mission].events.emplace_back();
			auto& stored_event = Campaign.missions[cur_mission].events.back();

			strncpy_s(stored_event.name, Mission_events[idx].name.c_str(), NAME_LENGTH - 1);
			stored_event.status = static_cast<char>(val);
		}

		// now set the "next mission to be the "current mission"
		Campaign.prev_mission = cur_mission;
		Campaign.current_mission = next_mission;
		break;

	case MC_CODE_START:
		// clear the campaign
		multi_campaign_client_start();

		// read in the # of missions
		GET_DATA(val);
		Campaign.num_missions = val;
		break;

	case MC_CODE_MISSION_NAMES:
		GET_DATA(item_count);
		GET_DATA(starting_num);
		Assert(starting_num + item_count <= Campaign.num_missions);
		for (idx = starting_num; idx < (starting_num + item_count); ++idx) {
			GET_STRING(fname);
			Campaign.missions[idx].name = vm_strdup(fname);
		}
		break;
	}

	PACKET_SET_SIZE();
}

// send a "campaign finished" packet
void multi_campaign_send_done()
{
	ubyte data[MAX_PACKET_SIZE], val;
	int packet_size = 0;

	// build the header
	BUILD_HEADER(CAMPAIGN_UPDATE);

	// add the code
	val = MC_CODE_DONE;
	ADD_DATA(val);

	// broadcast the packet
	multi_io_send_to_all_reliable(data, packet_size);
}

// send a campaign debrief update packet
void multi_campaign_send_debrief_info()
{
	ubyte data[MAX_PACKET_SIZE],val;
	int idx;
	int packet_size = 0;

	// build the header
	BUILD_HEADER(CAMPAIGN_UPDATE);

	// add the code
	val = MC_CODE_DEBRIEF;
	ADD_DATA(val);

	// add the mission # we're including
	val = static_cast<ubyte>(Campaign.current_mission);
	ADD_DATA(val);

	// add the next mission
	val = static_cast<ubyte>(Campaign.next_mission);
	ADD_DATA(val);

	// add the filename
	Assert(Campaign.missions[Campaign.current_mission].name != NULL);
	ADD_STRING(Campaign.missions[Campaign.current_mission].name);

	// add the # of goals and events
	val = static_cast<ubyte>(Campaign.missions[Campaign.current_mission].goals.size());
	ADD_DATA(val);
	ADD_USHORT(static_cast<ushort>(Campaign.missions[Campaign.current_mission].events.size()));

#ifndef NDEBUG
	const size_t goal_size = Campaign.missions[Campaign.current_mission].goals.size() * sizeof(ubyte);
	const size_t event_size = Campaign.missions[Campaign.current_mission].events.size() * sizeof(ubyte);

	Assertion(packet_size + goal_size + event_size < MAX_PACKET_SIZE, "Campaign debrief info packet is too large!");
#endif

	// add the goals
	for(idx=0;idx<(int)Campaign.missions[Campaign.current_mission].goals.size();idx++){
		val = static_cast<ubyte>(Campaign.missions[Campaign.current_mission].goals[idx].status);
		ADD_DATA(val);
	}	

	// add the events
	for(idx=0;idx<(int)Campaign.missions[Campaign.current_mission].events.size();idx++){
		val = static_cast<ubyte>(Campaign.missions[Campaign.current_mission].events[idx].status);
		ADD_DATA(val);
	}	

	// send to all players
	multi_io_send_to_all_reliable(data, packet_size);
}

// send a campaign pool status packet
void multi_campaign_send_pool_status()
{
	ubyte data[MAX_PACKET_SIZE],val;
	int idx;
	int spool_size;
	int wpool_size;
	int packet_size = 0;

	// build the header
	BUILD_HEADER(CAMPAIGN_UPDATE);

	// add the code
	val = MC_CODE_POOL;
	ADD_DATA(val);

	// if we're not in campaign mode, send a single byte saying "allow all ships and weapons"
	if(!(Game_mode & GM_CAMPAIGN_MODE)){
		val = 0x0;
		ADD_DATA(val);
	}
	// otherwise add all relevant ship/weapon pool data
	else {
		val = 0x1;
		ADD_DATA(val);

		// determine how many ship types we're going to add
		spool_size = 0;
		for(idx = 0; idx < ship_info_size(); idx++) {
			if(Campaign.ships_allowed[idx]){
				spool_size++;
			}
		}
		
		// determine how many weapon types we're going to add
		wpool_size = 0;
		for(idx = 0; idx < weapon_info_size(); idx++){
			if(Campaign.weapons_allowed[idx]){
				wpool_size++;
			}
		}

#ifndef NDEBUG
		// make sure it'll all fit into this packet
		const auto p_size = static_cast<size_t>(wpool_size + spool_size) * sizeof(ushort);

		Assertion((static_cast<size_t>(packet_size) + p_size + (sizeof(ushort) * 2)) < MAX_PACKET_SIZE,
				  "Allowed ship/weapon pool is too large!");
#endif

		// add all ship types
		ADD_USHORT(static_cast<ushort>(spool_size));
		for(idx = 0; idx < ship_info_size(); idx++) {
			if(Campaign.ships_allowed[idx]){
				ADD_USHORT(static_cast<ushort>(idx));
			}
		}

		// add all weapon types
		ADD_USHORT(static_cast<ushort>(wpool_size));
		for(idx = 0; idx < weapon_info_size(); idx++){
			if(Campaign.weapons_allowed[idx]){
				ADD_USHORT(static_cast<ushort>(idx));
			}
		}
	}

	// send to all players
	Assert(Net_player->flags & NETINFO_FLAG_AM_MASTER);	
	multi_io_send_to_all_reliable(data, packet_size);

	// notification message
	multi_common_add_text(XSTR("Campaign ship/weapon pool\n",644),1);
}

// send a "start campaign" packet
void multi_campaign_send_start(net_player *pl)
{
	ubyte data[MAX_PACKET_SIZE], val;
	int idx;
	int packet_size = 0;
	ubyte item_count, starting_num, *ptr;

	// first send "start" to init campaign structs
	BUILD_HEADER(CAMPAIGN_UPDATE);

	val = MC_CODE_START;
	ADD_DATA(val);

	val = static_cast<ubyte>(Campaign.num_missions);
	ADD_DATA(val);

	if (pl != nullptr) {
		multi_io_send_reliable(pl, data, packet_size);
	} else {
		multi_io_send_to_all_reliable(data, packet_size);
	}

	// now send list of mission names
	BUILD_HEADER(CAMPAIGN_UPDATE);

	val = MC_CODE_MISSION_NAMES;
	ADD_DATA(val);

	ptr = &data[packet_size];

	item_count = 0;
	ADD_DATA(item_count);

	starting_num = 0;
	ADD_DATA(starting_num);

	for (idx = 0; idx < Campaign.num_missions; idx++) {
		Assert(Campaign.missions[idx].name != NULL);
		ADD_STRING(Campaign.missions[idx].name);
		++item_count;

		if ( (packet_size + MC_INGAME_DATA_SLOP) > MAX_PACKET_SIZE ) {
			*ptr = item_count;

			if (pl != nullptr) {
				multi_io_send_reliable(pl, data, packet_size);
			} else {
				multi_io_send_to_all_reliable(data, packet_size);
			}

			BUILD_HEADER(CAMPAIGN_UPDATE);
			val = MC_CODE_MISSION_NAMES;
			ADD_DATA(val);

			ptr = &data[packet_size];

			item_count = 0;
			ADD_DATA(item_count);

			starting_num = static_cast<ubyte>(idx);
			ADD_DATA(starting_num);
		}
	}

	*ptr = item_count;

	// if we're targeting a specific player
	if (pl != nullptr) {
		multi_io_send_reliable(pl, data, packet_size);
	}
	// send to all players
	else {	
		multi_io_send_to_all_reliable(data, packet_size);
	}
}

// campaign start packet for ingame joiners.  Sends filename and goal/event name and status
void multi_campaign_send_ingame_start( net_player *pl )
{
	ubyte data[MAX_PACKET_SIZE], packet_type, *ptr, val, num_goals;
	ushort num_events, *s_ptr;
	int packet_size, i, j;
	ubyte item_count, starting_num;

	Assert( pl != NULL );
	packet_size = 0;

	if ( Game_mode & GM_CAMPAIGN_MODE ) {

		// first -- init campaign structs
		// add the # of missions
		BUILD_HEADER(CAMPAIGN_UPDATE_INGAME);
		packet_type = MC_JIP_INITIAL_PACKET;
		ADD_DATA(packet_type);

		val = static_cast<ubyte>(Campaign.num_missions);
		ADD_DATA(val);

		multi_io_send_reliable(pl, data, packet_size);

		// now send mission names
		BUILD_HEADER(CAMPAIGN_UPDATE_INGAME);
		packet_type = MC_JIP_MISSION_NAMES;
		ADD_DATA(packet_type);

		ptr = &data[packet_size];

		item_count = 0;
		ADD_DATA(item_count);

		starting_num = 0;
		ADD_DATA(starting_num);

		for (i = 0; i < Campaign.num_missions; i++) {
			Assert(Campaign.missions[i].name != NULL);
			ADD_STRING(Campaign.missions[i].name);
			++item_count;

			if ( (packet_size + MC_INGAME_DATA_SLOP) > MAX_PACKET_SIZE ) {
				*ptr = item_count;
				multi_io_send_reliable(pl, data, packet_size);

				BUILD_HEADER(CAMPAIGN_UPDATE_INGAME);
				packet_type = MC_JIP_MISSION_NAMES;
				ADD_DATA(packet_type);

				ptr = &data[packet_size];

				item_count = 0;
				ADD_DATA(item_count);

				starting_num = static_cast<ubyte>(i);
				ADD_DATA(starting_num);
			}
		}

		*ptr = item_count;

		multi_io_send_reliable(pl, data, packet_size);

		// send the number and status of all goals event for all previous missions
		for (i = 0; i < Campaign.num_missions; i++ ) {
			ubyte status;

			// don't do anything if mission hasn't been completed
			if ( !Campaign.missions[i].completed )
				continue;

			// don't send data for the current mission being played, or if both goals and events are 0
			num_goals = static_cast<ubyte>(Campaign.missions[i].goals.size());
			num_events = static_cast<ushort>(Campaign.missions[i].events.size());

			// add the mission number and the goal/event status
			BUILD_HEADER( CAMPAIGN_UPDATE_INGAME );

			packet_type = MC_JIP_GE_STATUS;
			ADD_DATA( packet_type );
			val = static_cast<ubyte>(i);
			ADD_DATA(val);

			ADD_DATA(num_goals);
			for ( j = 0; j < num_goals; j++ ) {
				status = static_cast<ubyte>(Campaign.missions[i].goals[j].status);
				ADD_DATA(status);
			}

			// now the events
			ADD_USHORT(num_events);
			for ( j = 0; j < num_events; j++ ) {
				status = static_cast<ubyte>(Campaign.missions[i].events[j].status);
				ADD_DATA(status);
			}
			
			multi_io_send_reliable(pl, data, packet_size);
		}	

		// send the goal/event names.
		for ( i = 0; i < Campaign.num_missions; i++ ) {
			ubyte goal_count, starting_goal_num;

			// first the goal names
			num_goals = static_cast<ubyte>(Campaign.missions[i].goals.size());

			// don't do anything if mission hasn't been completed
			if ( !Campaign.missions[i].completed ){
				continue;
			}

			BUILD_HEADER( CAMPAIGN_UPDATE_INGAME );
			packet_type = MC_JIP_GOAL_NAMES;
			ADD_DATA(packet_type);
			val = static_cast<ubyte>(i);
			ADD_DATA(val);

			// save a pointer so we can put the number of goals written here.
			ptr = &data[packet_size];

			goal_count = 0;
			ADD_DATA( goal_count );

			starting_goal_num = 0;
			ADD_DATA( starting_goal_num );

			for ( j = 0; j < num_goals; j++ ) {
				ADD_STRING( Campaign.missions[i].goals[j].name );
				goal_count++;

				// if packet will get too big, send it off.
				if ( (packet_size + MC_INGAME_DATA_SLOP) > MAX_PACKET_SIZE ) {
					*ptr = goal_count;					
					multi_io_send_reliable(pl, data, packet_size);
					BUILD_HEADER(CAMPAIGN_UPDATE_INGAME);
					packet_type = MC_JIP_GOAL_NAMES;
					ADD_DATA( packet_type );
					val = static_cast<ubyte>(i);
					ADD_DATA(val);
					ptr = &data[packet_size];
					goal_count = 0;
					ADD_DATA( goal_count );
					starting_goal_num = (ubyte)j;
					ADD_DATA( starting_goal_num );
				}
			}

			*ptr = goal_count;			
			multi_io_send_reliable(pl, data, packet_size);
		}

		// send the goal/event names.
		for ( i = 0; i < Campaign.num_missions; i++ ) {
			ushort event_count, starting_event_num;

			// first the goal names
			Assert( Campaign.missions[i].events.size() < UCHAR_MAX );
			num_events = static_cast<ushort>(Campaign.missions[i].events.size());

			// don't do anything if mission hasn't been completed
			if ( !Campaign.missions[i].completed )
				continue;

			BUILD_HEADER(CAMPAIGN_UPDATE_INGAME);
			packet_type = MC_JIP_EVENT_NAMES;
			ADD_DATA(packet_type);
			val = static_cast<ubyte>(i);
			ADD_DATA(val);

			// save a pointer so we can put the number of goals written here.
			s_ptr = reinterpret_cast<ushort *>(&data[packet_size]);

			event_count = 0;
			ADD_USHORT( event_count );

			starting_event_num = 0;
			ADD_USHORT( starting_event_num );

			for ( j = 0; j < num_events; j++ ) {
				ADD_STRING( Campaign.missions[i].events[j].name );
				event_count++;

				// if packet will get too big, send it off.
				if ( (packet_size + MC_INGAME_DATA_SLOP) > MAX_PACKET_SIZE ) {
					*s_ptr = INTEL_SHORT(event_count);
					multi_io_send_reliable(pl, data, packet_size);
					BUILD_HEADER(CAMPAIGN_UPDATE_INGAME);
					packet_type = MC_JIP_EVENT_NAMES;
					ADD_DATA( packet_type );
					val = static_cast<ubyte>(i);
					ADD_DATA(val);
					s_ptr = reinterpret_cast<ushort *>(&data[packet_size]);
					event_count = 0;
					ADD_USHORT( event_count );
					starting_event_num = static_cast<ushort>(j);
					ADD_USHORT( starting_event_num );
				}
			}

			*s_ptr = event_count;
			multi_io_send_reliable(pl, data, packet_size);
		}
	}

	// add the stop byte
	BUILD_HEADER(CAMPAIGN_UPDATE_INGAME);
	packet_type = MC_JIP_END_DATA;
	ADD_DATA(packet_type);	
	multi_io_send_reliable(pl, data, packet_size);
}

void multi_campaign_process_ingame_start( ubyte *data, header *hinfo )
{
	int offset, i;
	ubyte packet_type, num_goals, status, starting_num, mission_num, num_missions;
	ushort num_events, event_starting_num;
	char fname[255];

	offset = HEADER_LENGTH;

	GET_DATA( packet_type );
	switch( packet_type ) {
	case MC_JIP_INITIAL_PACKET:

		// clear out the names of the missions
		mission_campaign_clear();		// should free all data structures which need to be freed

		// get the number of missions
		GET_DATA(mission_num);
		Campaign.num_missions = mission_num;
		break;

	case MC_JIP_GE_STATUS:
		
		GET_DATA( mission_num );
		GET_DATA( num_goals );
		// need to fill out the data
		Assert( Campaign.missions[mission_num].goals.empty() );
		Campaign.missions[mission_num].goals.clear();
		for (i = 0; i < num_goals; i++) {
			Campaign.missions[mission_num].goals.emplace_back();
			auto& stored_goal = Campaign.missions[mission_num].goals.back();
			GET_DATA(status);
			stored_goal.status = static_cast<char>(status);
		}

		// now the events
		GET_USHORT( num_events );
		// need to fill out the data
		Assert( Campaign.missions[mission_num].events.empty() );
		Campaign.missions[mission_num].events.clear();
		for (i = 0; i < num_events; i++) {
			Campaign.missions[mission_num].events.emplace_back();
			auto& stored_event = Campaign.missions[mission_num].events.back();
			GET_DATA(status);
			stored_event.status = static_cast<char>(status);
		}
		break;

	case MC_JIP_GOAL_NAMES:
		GET_DATA( mission_num );
		GET_DATA( num_goals );
		GET_DATA( starting_num );
		for ( i = starting_num; i < (starting_num + num_goals); i++ ) {
			GET_STRING(Campaign.missions[mission_num].goals[i].name);
		}
		break;

	case MC_JIP_EVENT_NAMES:
		GET_DATA( mission_num );
		GET_USHORT( num_events );
		GET_USHORT( event_starting_num );
		for ( i = event_starting_num; i < (event_starting_num + num_events); i++ ) {
			GET_STRING(Campaign.missions[mission_num].events[i].name);
		}
		break;

	case MC_JIP_MISSION_NAMES:
		GET_DATA(num_missions);
		GET_DATA(starting_num);
		Assert(starting_num + num_missions <= Campaign.num_missions);
		for (i = starting_num; i < (starting_num + num_missions); ++i) {
			GET_STRING(fname);
			Campaign.missions[i].name = vm_strdup(fname);
		}
		break;

	case MC_JIP_END_DATA:
		Net_player->state = NETPLAYER_STATE_INGAME_CINFO;
		send_netplayer_update_packet();
		break;
	}

	PACKET_SET_SIZE();
}
