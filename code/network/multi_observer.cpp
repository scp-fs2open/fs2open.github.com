/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_observer.cpp $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:00 $
 * $Author: penguin $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *  
 * 
 * 6     9/14/99 2:21p Dave
 * Fixed observer mode joining and ingame stuff.
 * 
 * 5     8/19/99 10:59a Dave
 * Packet loss detection.
 * 
 * 4     11/17/98 11:12a Dave
 * Removed player identification by address. Now assign explicit id #'s.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 13    6/13/98 6:01p Hoffoss
 * Externalized all new (or forgot to be added) strings to all the code.
 * 
 * 12    6/04/98 11:04a Allender
 * object update level stuff.  Don't reset to high when becoming an
 * observer of any type.  default to low when guy is a dialup customer
 * 
 * 11    5/22/98 9:49p Allender
 * make observers Player_ship hidden from sensors so that he cannot target
 * his "ship".
 * 
 * 10    5/19/98 11:36p Allender
 * fixed very nasty mask problem with ingame joiner marking player objects
 * incorrectly.  Named ingame joiner ship and observer ship unique names
 * 
 * 9     4/18/98 5:00p Dave
 * Put in observer zoom key. Made mission sync screen more informative.
 * 
 * 8     4/03/98 3:13p Dave
 * More work on udp reliable transport. Fixed observer hud offset problem.
 * Made observer join and ingame join work again. Put subnet broadcasting
 * in for TCP.
 * 
 * 7     3/30/98 6:27p Dave
 * Put in a more official set of multiplayer options, including a system
 * for distributing netplayer and netgame settings.
 * 
 * 6     3/24/98 5:00p Dave
 * Fixed several ui bugs. Put in pre and post voice stream playback sound
 * fx. Put in error specific popups for clients getting dropped from games
 * through actions other than their own.
 * 
 * 5     3/23/98 5:42p Dave
 * Put in automatic xfer of pilot pic files. Changed multi_xfer system so
 * that it can support multiplayer sends/received between client and
 * server simultaneously.
 * 
 * 4     3/15/98 4:17p Dave
 * Fixed oberver hud problems. Put in handy netplayer macros. Reduced size
 * of network orientation matrices.
 * 
 * 3     3/14/98 2:48p Dave
 * Cleaned up observer joining code. Put in support for file xfers to
 * ingame joiners (observers or not). Revamped and reinstalled pseudo
 * lag/loss system.
 * 
 * 2     3/13/98 2:51p Dave
 * Put in support for observers to join ingame.
 * 
 * 1     3/12/98 5:44p Dave
 *  
 * $NoKeywords: $
 */

#include "freespace.h"
#include "multi.h"
#include "multiutil.h"
#include "object.h"
#include "observer.h"
#include "hudconfig.h"
#include "hudobserver.h"
#include "managepilot.h"
#include "multi_observer.h"
#include "missionparse.h"
#include "timer.h"

// ---------------------------------------------------------------------------------------
// MULTI OBSERVER DEFINES/VARS
//


// ---------------------------------------------------------------------------------------
// MULTI OBSERVER FUNCTIONS
//

// create a _permanent_ observer player 
int multi_obs_create_player(int player_num,char *name,net_addr *addr,player *pl)
{	
	// blast the player struct
	memset(&Net_players[player_num],0,sizeof(net_player));
	
	// Net_players[player_num].flags |= (NETINFO_FLAG_CONNECTED | NETINFO_FLAG_OBSERVER);	
	// DOH!!! The lack of this caused many bugs. 
	Net_players[player_num].flags = (NETINFO_FLAG_DO_NETWORKING | NETINFO_FLAG_OBSERVER);
	// memcpy(&Net_players[player_num].p_info.addr, addr, sizeof(net_addr));	
	Net_players[player_num].player = pl;
	
	// 6/3/98 -- don't set observer to update high...let it be whatever player set it at.
	//Net_players[player_num].p_info.options.obj_update_level = OBJ_UPDATE_HIGH;
	// set up the net_player structure
	memset(pl, 0, sizeof(player));
	stuff_netplayer_info( &Net_players[player_num], addr, 0, pl );
	Net_players[player_num].last_heard_time = timer_get_fixed_seconds();
	Net_players[player_num].reliable_socket = INVALID_SOCKET;
	Net_players[player_num].s_info.kick_timestamp = -1;
	Net_players[player_num].s_info.voice_token_timestamp = -1;
	Net_players[player_num].s_info.tracker_security_last = -1;
	Net_players[player_num].s_info.target_objnum = -1;
	Net_players[player_num].s_info.accum_buttons = 0;

	// reset the ping for this player
	multi_ping_reset(&Net_players[player_num].s_info.ping);
	
	// timestamp his last_full_update_time
	Net_players[player_num].s_info.last_full_update_time = timestamp(0);			
	Net_players[player_num].player->objnum = -1;	

	// nil his file xfer handle
	Net_players[player_num].s_info.xfer_handle = -1;

	// zero out his object update and control info sequencing data
	Net_players[player_num].client_cinfo_seq = 0;
	Net_players[player_num].client_server_seq = 0;		

	// his kick timestamp
	Net_players[player_num].s_info.kick_timestamp = -1;

	// nil his data rate timestamp stuff
	Net_players[player_num].s_info.rate_stamp = -1;
	Net_players[player_num].s_info.rate_bytes = 0;

	// nil packet buffer stuff
	Net_players[player_num].s_info.unreliable_buffer_size = 0;
	Net_players[player_num].s_info.reliable_buffer_size = 0;

	// callsign and short callsign
	strcpy(pl->callsign,name);
	pilot_set_short_callsign(pl,SHORT_CALLSIGN_PIXEL_W);
	pl->flags |= PLAYER_FLAGS_STRUCTURE_IN_USE;	

	Net_players[player_num].sv_bytes_sent = 0;	
	Net_players[player_num].sv_last_pl = -1;	
	Net_players[player_num].cl_bytes_recvd = 0;	
	Net_players[player_num].cl_last_pl = -1;
	
	return 1;
}

// create an explicit observer object and assign it to the passed player
void multi_obs_create_observer(net_player *pl)
{
	int objnum;
	
	// create the basic observer object
	objnum = observer_create( &vmd_identity_matrix, &vmd_zero_vector);	
	Assert(objnum != -1);
	Objects[objnum].flags |= OF_PLAYER_SHIP;	
	Objects[objnum].net_signature = 0;

	// put it a 1,1,1
	Objects[objnum].pos.x = 1.0f;
	Objects[objnum].pos.y = 1.0f;
	Objects[objnum].pos.z = 1.0f;

	// assign this object to the player
	pl->player->objnum = objnum;				
}

// create observer object locally, and additionally, setup some other information
// ( client-side equivalent of multi_obs_create_observer() )
void multi_obs_create_observer_client()
{
	int pobj_num;
	
	Assert(!(Net_player->flags & NETINFO_FLAG_OBS_PLAYER));					

	// make me an observer object
	multi_obs_create_observer(Net_player);	
					
	// set my object to be the observer object	
	Player_obj = &Objects[Net_player->player->objnum];
	
	// create the default player ship object and use that as my default virtual "ship", and make it "invisible"
	pobj_num = parse_create_object(&Player_start_pobject);
	Assert(pobj_num != -1);
	obj_set_flags(&Objects[pobj_num],OF_PLAYER_SHIP);
	Player_ship = &Ships[Objects[pobj_num].instance];

	// make ship hidden from sensors so that this observer cannot target it.  Observers really have two ships
	// one observer, and one "Player_ship".  Observer needs to ignore the Player_ship.
	Player_ship->flags |= SF_HIDDEN_FROM_SENSORS;
	strcpy(Player_ship->ship_name, XSTR("Observer Ship",688));
	Player_ai = &Ai_info[Ships[Objects[pobj_num].instance].ai_index];		

	// configure the hud to be in "observer" mode
	hud_config_as_observer(Player_ship,Player_ai);	
		
	// set some flags for myself
	Net_player->flags |= NETINFO_FLAG_OBSERVER;	
	
	// reset the control info structure
	memset(&Player->ci,0,sizeof(control_info));	
}

// create objects for all known observers in the game at level start
// call this before entering a mission
// this implies for the local player in the case of a client or for _all_ players in the case of a server
void multi_obs_level_init()
{
	int idx;	

	// unset the OBS_PLAYER flag here for all net players
	for(idx=0;idx<MAX_PLAYERS;idx++){
		Net_players[idx].flags &= ~(NETINFO_FLAG_OBS_PLAYER);
	}

	// if i'm a client and I'm an observer, create an object for myself
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Net_player->flags & NETINFO_FLAG_OBSERVER)){		
		// create my own observer object and setup other misc. data
		multi_obs_create_observer_client();
	}
	// otherwise create stuff for all (permanent) observers in the game
	else {
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && MULTI_OBSERVER(Net_players[idx])){
				// make an observer object for the guy
				multi_obs_create_observer(&Net_players[idx]);
			}
		}
	}
}

// if i'm an observer, zoom to near my targted object (if any)
void multi_obs_zoom_to_target()
{
	vector direct;		
	float dist;	
	
	// if i'm not an observer, do nothing
	if(!(Game_mode & GM_MULTIPLAYER) || (Net_player == NULL) || !(Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type != OBJ_OBSERVER)){		
		return;
	}

	// if I have no targeted object, do nothing
	if(Player_ai->target_objnum == -1){		
		return;
	}

	// get the normalized direction vector between the observer and the targeted object
	vm_vec_sub(&direct,&Objects[Player_ai->target_objnum].pos,&Player_obj->pos);
	dist = vm_vec_mag(&direct);
	vm_vec_normalize(&direct);	

	// orient the guy correctly
	vm_vec_ang_2_matrix(&Player_obj->orient,&direct,0.0f);

	// keep about 3 object radii away when moving
	dist -= (Objects[Player_ai->target_objnum].radius * 3.0f);

	// get the movement vector
	vm_vec_scale(&direct,dist);

	// move
	vm_vec_add2(&Player_obj->pos,&direct);
}
	

	
