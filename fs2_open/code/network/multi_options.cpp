/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_options.cpp $
 * $Revision: 2.7 $
 * $Date: 2005-03-02 21:18:19 $
 * $Author: taylor $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.6  2005/02/04 10:12:31  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.5  2004/07/26 20:47:42  Kazan
 * remove MCD complete
 *
 * Revision 2.4  2004/07/12 16:32:57  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.3  2004/03/05 09:02:02  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2002/08/01 01:41:08  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/22 01:22:25  penguin
 * Linux port -- added NO_STANDALONE ifdefs
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 22    8/27/99 12:32a Dave
 * Allow the user to specify a local port through the launcher.
 * 
 * 21    8/22/99 1:19p Dave
 * Fixed up http proxy code. Cleaned up scoring code. Reverse the order in
 * which d3d cards are detected.
 * 
 * 20    8/04/99 6:01p Dave
 * Oops. Make sure standalones log in.
 * 
 * 19    7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 18    5/03/99 8:32p Dave
 * New version of multi host options screen.
 * 
 * 17    4/25/99 7:43p Dave
 * Misc small bug fixes. Made sun draw properly.
 * 
 * 16    4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 15    3/10/99 6:50p Dave
 * Changed the way we buffer packets for all clients. Optimized turret
 * fired packets. Did some weapon firing optimizations.
 * 
 * 14    3/09/99 6:24p Dave
 * More work on object update revamping. Identified several sources of
 * unnecessary bandwidth.
 * 
 * 13    3/08/99 7:03p Dave
 * First run of new object update system. Looks very promising.
 * 
 * 12    2/21/99 6:01p Dave
 * Fixed standalone WSS packets. 
 * 
 * 11    2/19/99 2:55p Dave
 * Temporary checking to report the winner of a squad war match.
 * 
 * 10    2/12/99 6:16p Dave
 * Pre-mission Squad War code is 95% done.
 * 
 * 9     2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 8     11/20/98 11:16a Dave
 * Fixed up IPX support a bit. Making sure that switching modes and
 * loading/saving pilot files maintains proper state.
 * 
 * 7     11/19/98 4:19p Dave
 * Put IPX sockets back in psnet. Consolidated all multiplayer config
 * files into one.
 * 
 * 6     11/19/98 8:03a Dave
 * Full support for D3-style reliable sockets. Revamped packet lag/loss
 * system, made it receiver side and at the lowest possible level.
 * 
 * 5     11/17/98 11:12a Dave
 * Removed player identification by address. Now assign explicit id #'s.
 * 
 * 4     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 3     10/07/98 6:27p Dave
 * Globalized mission and campaign file extensions. Removed Silent Threat
 * special code. Moved \cache \players and \multidata into the \data
 * directory.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 20    9/10/98 1:17p Dave
 * Put in code to flag missions and campaigns as being MD or not in Fred
 * and Freespace. Put in multiplayer support for filtering out MD
 * missions. Put in multiplayer popups for warning of non-valid missions.
 * 
 * 19    7/07/98 2:49p Dave
 * UI bug fixes.
 * 
 * 18    6/04/98 11:04a Allender
 * object update level stuff.  Don't reset to high when becoming an
 * observer of any type.  default to low when guy is a dialup customer
 * 
 * 17    5/24/98 3:45a Dave
 * Minor object update fixes. Justify channel information on PXO. Add a
 * bunch of configuration stuff for the standalone.
 * 
 * 16    5/19/98 1:35a Dave
 * Tweaked pxo interface. Added rankings url to pxo.cfg. Make netplayer
 * local options update dynamically in netgames.
 * 
 * 15    5/08/98 5:05p Dave
 * Go to the join game screen when quitting multiplayer. Fixed mission
 * text chat bugs. Put mission type symbols on the create game list.
 * Started updating standalone gui controls.
 * 
 * 14    5/06/98 12:36p Dave
 * Make sure clients can leave the debrief screen easily at all times. Fix
 * respawn count problem.
 * 
 * 13    5/03/98 7:04p Dave
 * Make team vs. team work mores smoothly with standalone. Change how host
 * interacts with standalone for picking missions. Put in a time limit for
 * ingame join ship select. Fix ingame join ship select screen for Vasudan
 * ship icons.
 * 
 * 12    5/03/98 2:52p Dave
 * Removed multiplayer furball mode.
 * 
 * 11    4/23/98 6:18p Dave
 * Store ETS values between respawns. Put kick feature in the text
 * messaging system. Fixed text messaging system so that it doesn't
 * process or trigger ship controls. Other UI fixes.
 * 
 * 10    4/22/98 5:53p Dave
 * Large reworking of endgame sequencing. Updated multi host options
 * screen for new artwork. Put in checks for host or team captains leaving
 * midgame.
 * 
 * 9     4/16/98 11:39p Dave
 * Put in first run of new multiplayer options screen. Still need to
 * complete final tab.
 * 
 * 8     4/13/98 4:50p Dave
 * Maintain status of weapon bank/links through respawns. Put # players on
 * create game mission list. Make observer not have engine sounds. Make
 * oberver pivot point correct. Fixed respawn value getting reset every
 * time host options screen started.
 * 
 * 7     4/09/98 11:01p Dave
 * Put in new multi host options screen. Tweaked multiplayer options a
 * bit.
 * 
 * 6     4/09/98 5:43p Dave
 * Remove all command line processing from the demo. Began work fixing up
 * the new multi host options screen.
 * 
 * 5     4/06/98 6:37p Dave
 * Put in max_observers netgame server option. Make sure host is always
 * defaulted to alpha 1 or zeta 1. Changed create game so that MAX_PLAYERS
 * can always join but need to be kicked before commit can happen. Put in
 * support for server ending a game and notifying clients of a special
 * condition.
 * 
 * 4     4/04/98 4:22p Dave
 * First rev of UDP reliable sockets is done. Seems to work well if not
 * overly burdened.
 * 
 * 3     4/03/98 1:03a Dave
 * First pass at unreliable guaranteed delivery packets.
 * 
 * 2     3/31/98 4:51p Dave
 * Removed medals screen and multiplayer buttons from demo version. Put in
 * new pilot popup screen. Make ships in mp team vs. team have proper team
 * ids. Make mp respawns a permanent option saved in the player file.
 * 
 * 1     3/30/98 6:24p Dave
 *  
 * 
 * $NoKeywords: $
 */

#include "PreProcDefines.h"

#ifndef NO_NETWORK

#include "cmdline/cmdline.h"
#include "osapi/osregistry.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multi_oo.h"
#include "freespace2/freespace.h"
#include "network/stand_gui.h"
#include "network/multiutil.h"
#include "network/multi_voice.h"
#include "network/multi_options.h"
#include "network/multi_team.h"
#include "mission/missioncampaign.h"
#include "mission/missionparse.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "cfile/cfile.h"



// ----------------------------------------------------------------------------------
// MULTI OPTIONS DEFINES/VARS
//

// packet codes
#define MULTI_OPTION_SERVER						0				// server update follows
#define MULTI_OPTION_LOCAL							1				// local netplayer options follow
#define MULTI_OPTION_START_GAME					2				// host's start game options on the standalone server
#define MULTI_OPTION_MISSION						3				// host's mission selection stuff on a standalone server

// global options
#define MULTI_CFG_FILE								NOX("multi.cfg")
multi_global_options Multi_options_g;

char Multi_options_proxy[512] = "";
ushort Multi_options_proxy_port = 0;

// ----------------------------------------------------------------------------------
// MULTI OPTIONS FUNCTIONS
//

// load in the config file
#define NEXT_TOKEN()						do { tok = strtok(NULL, "\n"); if(tok != NULL){ drop_leading_white_space(tok); drop_trailing_white_space(tok); } } while(0);
#define SETTING(s)						( !stricmp(tok, s) )
void multi_options_read_config()
{
	CFILE *in;
	char str[512];
	char *tok = NULL;

	// set default value for the global multi options
	memset(&Multi_options_g, 0, sizeof(multi_global_options));
	Multi_options_g.protocol = NET_TCP;	

	// do we have a forced port via commandline or registry?
	ushort forced_port = (ushort)os_config_read_uint(NULL, "ForcePort", 0);	
	Multi_options_g.port = (Cmdline_network_port >= 0) ? (ushort)Cmdline_network_port : forced_port == 0 ? (ushort)DEFAULT_GAME_PORT : forced_port;

	Multi_options_g.log = (Cmdline_multi_log) ? 1 : 0;
	Multi_options_g.datarate_cap = OO_HIGH_RATE_DEFAULT;
	strcpy(Multi_options_g.user_tracker_ip, "");	
	strcpy(Multi_options_g.game_tracker_ip, "");	
	strcpy(Multi_options_g.pxo_ip, "");	
	strcpy(Multi_options_g.pxo_rank_url, "");	
	strcpy(Multi_options_g.pxo_create_url, "");	
	strcpy(Multi_options_g.pxo_verify_url, "");	
	strcpy(Multi_options_g.pxo_banner_url, "");

	// standalone values
	Multi_options_g.std_max_players = -1;
	Multi_options_g.std_datarate = OBJ_UPDATE_HIGH;
	Multi_options_g.std_voice = 1;
	memset(Multi_options_g.std_passwd, 0, STD_PASSWD_LEN);
	memset(Multi_options_g.std_pname, 0, STD_NAME_LEN);
	Multi_options_g.std_framecap = 30;
	
	// read in the config file
	in = cfopen(MULTI_CFG_FILE, "rt", CFILE_NORMAL, CF_TYPE_DATA);
	
	// if we failed to open the config file, user default settings
	if(in == NULL){
		nprintf(("Network","Failed to open network config file, using default settings\n"));		
		return;
	}

	while(!cfeof(in)){
		// read in the game info
		memset(str,0,512);
		cfgets(str,512,in);

		// parse the first line
		tok = strtok(str," \t");

		// check the token
		if(tok != NULL){
			drop_leading_white_space(tok);
			drop_trailing_white_space(tok);			
		} else {
			continue;
		}		

#ifndef NO_STANDALONE
		// all possible options
		// only standalone cares about the following options
		if(Is_standalone){
			if(SETTING("+pxo")){			
				// setup PXO mode
				NEXT_TOKEN();
				if(tok != NULL){
					// whee!
				}
			} else 
			if(SETTING("+name")){
				// set the standalone server's permanent name
				NEXT_TOKEN();
				if(tok != NULL){
					strncpy(Multi_options_g.std_pname, tok, STD_NAME_LEN);
				}
			} else 
			if(SETTING("+no_voice")){
				// standalone won't allow voice transmission
				Multi_options_g.std_voice = 0;
			} else
			if(SETTING("+max_players")){
				// set the max # of players on the standalone
				NEXT_TOKEN();
				if(tok != NULL){
					if(!((atoi(tok) < 1) || (atoi(tok) > MAX_PLAYERS))){
						Multi_options_g.std_max_players = atoi(tok);
					}
				}
			} else 
			if(SETTING("+ban")){
				// ban a player
				NEXT_TOKEN();
				if(tok != NULL){
					std_add_ban(tok);
				}
			} else 
			if(SETTING("+passwd")){
				// set the standalone host password
				NEXT_TOKEN();
				if(tok != NULL){
					strncpy(Multi_options_g.std_passwd, tok, STD_PASSWD_LEN);

#ifdef _WIN32
					// yuck
					extern HWND Multi_std_host_passwd;
					SetWindowText(Multi_std_host_passwd, Multi_options_g.std_passwd);
#else
					// TODO: get password ?
					// argh, gonna have to figure out how to do this - mharris 07/07/2002
#endif
				}
			} else 
			if(SETTING("+low_update")){
				// set standalone to low updates
				Multi_options_g.std_datarate = OBJ_UPDATE_LOW;
			} else
			if(SETTING("+med_update")){
				// set standalone to medium updates
				Multi_options_g.std_datarate = OBJ_UPDATE_MEDIUM;
			} else 
			if(SETTING("+high_update")){
				// set standalone to high updates
				Multi_options_g.std_datarate = OBJ_UPDATE_HIGH;
			} else 
			if(SETTING("+lan_update")){
				// set standalone to high updates
				Multi_options_g.std_datarate = OBJ_UPDATE_LAN;
			} 
		}
#endif

		// common to all modes
		if(SETTING("+user_server")){
			// ip addr of user tracker
			NEXT_TOKEN();
			if(tok != NULL){
				strcpy(Multi_options_g.user_tracker_ip, tok);
			}
		} else
		if(SETTING("+game_server")){
			// ip addr of game tracker
			NEXT_TOKEN();
			if(tok != NULL){
				strcpy(Multi_options_g.game_tracker_ip, tok);
			}
		} else
		if(SETTING("+chat_server")){
			// ip addr of pxo chat server
			NEXT_TOKEN();
			if(tok != NULL){
				strcpy(Multi_options_g.pxo_ip, tok);
			}
		} else
		if(SETTING("+rank_url")){
			// url of pilot rankings page
			NEXT_TOKEN();
			if(tok != NULL){
				strcpy(Multi_options_g.pxo_rank_url, tok);
			}
		} else
		if(SETTING("+create_url")){
			// url of pxo account create page
			NEXT_TOKEN();
			if(tok != NULL){
				strcpy(Multi_options_g.pxo_create_url, tok);
			}
		} else
		if(SETTING("+verify_url")){
			// url of pxo account verify page
			NEXT_TOKEN();
			if(tok != NULL){
				strcpy(Multi_options_g.pxo_verify_url, tok);
			}
		} else
		if(SETTING("+banner_url")){
			// url of pxo account verify page
			NEXT_TOKEN();
			if(tok != NULL){
				strcpy(Multi_options_g.pxo_banner_url, tok);
			}
		} else
		if(SETTING("+datarate")){
			// set the max datarate for high updates
			NEXT_TOKEN();
			if(tok != NULL){
				if(atoi(tok) >= 4000){
					Multi_options_g.datarate_cap = atoi(tok);
				}
			}			
		}
		if(SETTING("+http_proxy")){
			// get the proxy server
			NEXT_TOKEN();
			if(tok != NULL){				
				char *ip = strtok(tok, ":");
				if(ip != NULL){
					strcpy(Multi_options_proxy, ip);
				}
				ip = strtok(NULL, "");
				if(ip != NULL){
					Multi_options_proxy_port = (ushort)atoi(ip);
				} else {
					strcpy(Multi_options_proxy, "");
				}
			}
		}
	}

	// close the config file
	cfclose(in);
	in = NULL;
}

// set netgame defaults 
// NOTE : should be used when creating a newpilot
void multi_options_set_netgame_defaults(multi_server_options *options)
{
	// any player can do squadmate messaging
	options->squad_set = MSO_SQUAD_ANY;

	// only the host can end the game
	options->endgame_set = MSO_END_HOST;

	// allow ingame file xfer and custom pilot pix
	options->flags = (MSO_FLAG_INGAME_XFER | MSO_FLAG_ACCEPT_PIX);

	// set the default time limit to be -1 (no limit)
	options->mission_time_limit = fl2f(-1.0f);

	// set the default max kills for a mission
	options->kill_limit = 9999;

	// set the default # of respawns
	options->respawn = 2;

	// set the default # of max observers
	options->max_observers = 2;

	// set the default netgame qos
	options->voice_qos = 10;

	// set the default token timeout
	options->voice_token_wait = 2000;				// he must wait 2 seconds between voice gets

	// set the default max voice record time
	options->voice_record_time = 5000;
}

// set local netplayer defaults
// NOTE : should be used when creating a newpilot
void multi_options_set_local_defaults(multi_local_options *options)
{
	// accept pix by default and broadcast on the local subnet
	options->flags = (MLO_FLAG_ACCEPT_PIX | MLO_FLAG_LOCAL_BROADCAST);	

	// set the object update level based on the type of network connection specified by the user
	// at install (or launcher) time.
	if ( Psnet_connection == NETWORK_CONNECTION_DIALUP ) {
		options->obj_update_level = OBJ_UPDATE_LOW;
	} else {
		options->obj_update_level = OBJ_UPDATE_HIGH;
	}
}

// fill in the passed netgame options struct with the data from my player file data (only host/server should do this)
void multi_options_netgame_load(multi_server_options *options)
{
	if(options != NULL){
		memcpy(options,&Player->m_server_options,sizeof(multi_server_options));
	}	
}

// fill in the passed local options struct with the data from my player file data (all machines except standalone should do this)
void multi_options_local_load(multi_local_options *options, net_player *pxo_pl)
{
	if(options != NULL){
		memcpy(options,&Player->m_local_options,sizeof(multi_local_options));
	}

	// stuff pxo squad info
	if(pxo_pl != NULL){
		strcpy(pxo_pl->p_info.pxo_squad_name, Multi_tracker_squad_name);		
	}
}

// add data from a multi_server_options struct
void add_server_options(ubyte *data, int *size, multi_server_options *mso)
{
	int packet_size = *size;
	multi_server_options mso_tmp;

	memcpy(&mso_tmp, mso, sizeof(multi_server_options));

	mso_tmp.flags = INTEL_INT(mso->flags);
	mso_tmp.respawn = INTEL_INT(mso->respawn);
	mso_tmp.voice_token_wait = INTEL_INT(mso->voice_token_wait);
	mso_tmp.voice_record_time = INTEL_INT(mso->voice_record_time);
//	mso_tmp.mission_time_limit = INTEL_INT(mso->mission_time_limit);
	mso_tmp.kill_limit = INTEL_INT(mso->kill_limit);

	ADD_DATA(mso_tmp);

	*size = packet_size;
}

// add data from a multi_local_options struct
void add_local_options(ubyte *data, int *size, multi_local_options *mlo)
{
	int packet_size = *size;
	multi_local_options mlo_tmp;

	memcpy(&mlo_tmp, mlo, sizeof(multi_local_options));

	mlo_tmp.flags = INTEL_INT(mlo->flags);
	mlo_tmp.obj_update_level = INTEL_INT(mlo->obj_update_level);

	ADD_DATA(mlo_tmp);

	*size = packet_size;
}

// get data from multi_server_options struct
void get_server_options(ubyte *data, int *size, multi_server_options *mso)
{
	int offset = *size;

	GET_DATA(*mso);

	mso->flags = INTEL_INT(mso->flags);
	mso->respawn = INTEL_INT(mso->respawn);
	mso->voice_token_wait = INTEL_INT(mso->voice_token_wait);
	mso->voice_record_time = INTEL_INT(mso->voice_record_time);
//	mso->mission_time_limit = INTEL_INT(mso->mission_time_limit);
	mso->kill_limit = INTEL_INT(mso->kill_limit);

	*size = offset;
}

// get data from multi_local_options struct
void get_local_options(ubyte *data, int *size, multi_local_options *mlo)
{
	int offset = *size;

	GET_DATA(*mlo);

	mlo->flags = INTEL_INT(mlo->flags);
	mlo->obj_update_level = INTEL_INT(mlo->obj_update_level);

	*size = offset;
}

// update everyone on the current netgame options
void multi_options_update_netgame()
{
	ubyte data[MAX_PACKET_SIZE],code;
	int packet_size = 0;
	
	Assert(Net_player->flags & NETINFO_FLAG_GAME_HOST);

	// build the header and add the opcode
	BUILD_HEADER(OPTIONS_UPDATE);
	code = MULTI_OPTION_SERVER;
	ADD_DATA(code);

	// add the netgame options
	add_server_options(data, &packet_size, &Netgame.options);

	// send the packet
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_io_send_to_all_reliable(data, packet_size);
	} else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

// update everyone with my local settings
void multi_options_update_local()
{
	ubyte data[MAX_PACKET_SIZE],code;
	int packet_size = 0;
	
	// if i'm the server, don't do anything
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		return;
	}

	// build the header and add the opcode
	BUILD_HEADER(OPTIONS_UPDATE);
	code = MULTI_OPTION_LOCAL;
	ADD_DATA(code);

	// add the netgame options
	add_local_options(data, &packet_size, &Net_player->p_info.options);

	// send the packet		
	multi_io_send_reliable(Net_player, data, packet_size);
}

// update the standalone with the settings I have picked at the "start game" screen
void multi_options_update_start_game(netgame_info *ng)
{
	ubyte data[MAX_PACKET_SIZE],code;
	int packet_size = 0;

	// should be a host on a standalone
	Assert((Net_player->flags & NETINFO_FLAG_GAME_HOST) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER));

	// build the header
	BUILD_HEADER(OPTIONS_UPDATE);
	code = MULTI_OPTION_START_GAME;
	ADD_DATA(code);

	// add the start game options
	ADD_STRING(ng->name);
	ADD_INT(ng->mode);
	ADD_INT(ng->security);

	// add mode-specific data
	switch(ng->mode){
	case NG_MODE_PASSWORD:
		ADD_STRING(ng->passwd);
		break;
	case NG_MODE_RANK_ABOVE:
	case NG_MODE_RANK_BELOW:
		ADD_INT(ng->rank_base);
		break;
	}

	// send to the standalone server	
	multi_io_send_reliable(Net_player, data, packet_size);
}

// update the standalone with the mission settings I have picked (mission filename, etc)
void multi_options_update_mission(netgame_info *ng, int campaign_mode)
{
	ubyte data[MAX_PACKET_SIZE],code;
	int packet_size = 0;

	// should be a host on a standalone
	Assert((Net_player->flags & NETINFO_FLAG_GAME_HOST) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER));

	// build the header
	BUILD_HEADER(OPTIONS_UPDATE);
	code = MULTI_OPTION_MISSION;
	ADD_DATA(code);

	// type (coop or team vs. team)
	ADD_INT(ng->type_flags);

	// respawns
	ADD_UINT(ng->respawn);

	// add the mission/campaign filename
	code = (ubyte)campaign_mode;
	ADD_DATA(code);
	if(campaign_mode){
		ADD_STRING(ng->campaign_name);
	} else {
		ADD_STRING(ng->mission_name);
	}

	// send to the server	
	multi_io_send_reliable(Net_player, data, packet_size);
}


// ----------------------------------------------------------------------------------
// MULTI OPTIONS FUNCTIONS
//

// process an incoming multi options packet
void multi_options_process_packet(unsigned char *data, header *hinfo)
{
	ubyte code;	
	multi_local_options bogus;
	int idx,player_index;
	char str[255];
	int offset = HEADER_LENGTH;

	// find out who is sending this data	
	player_index = find_player_id(hinfo->id);

	// get the packet code
	GET_DATA(code);
	switch(code){
	// get the start game options
	case MULTI_OPTION_START_GAME:
		Assert(Game_mode & GM_STANDALONE_SERVER);

		// get the netgame name
		GET_STRING(Netgame.name);		

		// get the netgame mode
		GET_INT(Netgame.mode);

		// get the security #
		GET_INT(Netgame.security);

		// get mode specific data
		switch(Netgame.mode){
		case NG_MODE_PASSWORD:
			GET_STRING(Netgame.passwd);
			break;
		case NG_MODE_RANK_ABOVE:
		case NG_MODE_RANK_BELOW:
			GET_INT(Netgame.rank_base);
			break;
		}

#ifndef NO_STANDALONE
		// update standalone stuff
		std_connect_set_gamename(Netgame.name);
		std_multi_update_netgame_info_controls();
#endif
		break;

	// get mission choice options
	case MULTI_OPTION_MISSION:
		netgame_info ng;
		char title[NAME_LENGTH+1];
		int campaign_type,max_players;
		
		memset(&ng,0,sizeof(netgame_info));

		Assert(Game_mode & GM_STANDALONE_SERVER);

		// coop or team vs. team mode
		GET_INT(ng.type_flags);
		if((ng.type_flags & NG_TYPE_TEAM) && !(Netgame.type_flags & NG_TYPE_TEAM)){
			multi_team_reset();
		}
		// if squad war was switched on
		if((ng.type_flags & NG_TYPE_SW) && !(Netgame.type_flags & NG_TYPE_SW)){
			mprintf(("STANDALONE TURNED ON SQUAD WAR!!\n"));
		}
		Netgame.type_flags = ng.type_flags;

		// new respawn count
		GET_UINT(Netgame.respawn);

		// name string
		memset(str,255,0);

		GET_DATA(code);
		// campaign mode
		if(code){
			GET_STRING(ng.campaign_name);

			// set the netgame max players here if the filename has changed
			if(strcmp(Netgame.campaign_name,ng.campaign_name)){				
				memset(title,0,NAME_LENGTH+1);			
				if(!mission_campaign_get_info(ng.campaign_name,title,&campaign_type,&max_players)){
					Netgame.max_players = 0;
				} else {
					Netgame.max_players = max_players;
				}

				strcpy(Netgame.campaign_name,ng.campaign_name);
			}

			Netgame.campaign_mode = 1;

#ifndef NO_STANDALONE
			// put brackets around the campaign name
			if(Game_mode & GM_STANDALONE_SERVER){
				strcpy(str,"(");
				strcat(str,Netgame.campaign_name);
				strcat(str,")");
				std_multi_set_standalone_mission_name(str);
			}
#endif
		}
		// non-campaign mode
		else {
			GET_STRING(ng.mission_name);

			if(strcmp(Netgame.mission_name,ng.mission_name)){
				if(strlen(ng.mission_name)){
					Netgame.max_players = mission_parse_get_multi_mission_info( ng.mission_name );
				} else {
					// setting this to -1 will prevent us from being seen on the network
					Netgame.max_players = -1;				
				}
				strcpy(Netgame.mission_name,ng.mission_name);
				strcpy(Game_current_mission_filename,Netgame.mission_name);				
			}			

			Netgame.campaign_mode = 0;

#ifndef NO_STANDALONE
			// set the mission name
			if(Game_mode & GM_STANDALONE_SERVER){
				std_multi_set_standalone_mission_name(Netgame.mission_name);			
			}
#endif
		}
		
		send_netgame_update_packet();	   
		break;

	// get the netgame options
	case MULTI_OPTION_SERVER:		
		get_server_options(data, &offset, &Netgame.options);

		// if we're a standalone set for no sound, do so here
		if((Game_mode & GM_STANDALONE_SERVER) && !Multi_options_g.std_voice){
			Netgame.options.flags |= MSO_FLAG_NO_VOICE;
		} else {
			// maybe update the quality of sound
			multi_voice_maybe_update_vars(Netgame.options.voice_qos,Netgame.options.voice_record_time);
		}

		// set the skill level
		Game_skill_level = Netgame.options.skill_level;		

		if((Game_mode & GM_STANDALONE_SERVER) && !(Game_mode & GM_CAMPAIGN_MODE)){
			Netgame.respawn = Netgame.options.respawn;
		}

		// if we have the "temp closed" flag toggle
		if(Netgame.options.flags & MLO_FLAG_TEMP_CLOSED){
			Netgame.flags ^= NG_FLAG_TEMP_CLOSED;
		}
		Netgame.options.flags &= ~(MLO_FLAG_TEMP_CLOSED);

		// if i'm the standalone server, I should rebroadcast to all other players
		if(Game_mode & GM_STANDALONE_SERVER){
			for(idx=0;idx<MAX_PLAYERS;idx++){
				if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx]) && (&Net_players[idx] != &Net_players[player_index]) ){
					multi_io_send_reliable(&Net_players[idx], data, offset);
				}
			}

			send_netgame_update_packet();
		}
		break;
	
	// local netplayer options
	case MULTI_OPTION_LOCAL:
		if(player_index == -1){
			get_local_options(data, &offset, &bogus);
		} else {
			get_local_options(data, &offset, &Net_players[player_index].p_info.options);
		}		
		break;
	}
	PACKET_SET_SIZE();
}

#endif // !NO_NETWORK
