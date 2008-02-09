/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_data.cpp $
 * $Revision: 2.9 $
 * $Date: 2005-07-13 03:25:59 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.8  2005/03/02 21:18:19  taylor
 * better support for Inferno builds (in PreProcDefines.h now, no networking support)
 * make sure NO_NETWORK builds are as friendly on Windows as it is on Linux/OSX
 * revert a timeout in Client.h back to the original value before Linux merge
 *
 * Revision 2.7  2005/02/04 10:12:31  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.6  2004/07/26 20:47:42  Kazan
 * remove MCD complete
 *
 * Revision 2.5  2004/07/12 16:32:57  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.4  2004/03/05 09:02:02  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/22 01:22:25  penguin
 * Linux port -- added NO_STANDALONE ifdefs
 *
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
 * 8     6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 7     1/21/99 2:06p Dave
 * Final checkin for multiplayer testing.
 * 
 * 6     1/14/99 6:06p Dave
 * 100% full squad logo support for single player and multiplayer.
 * 
 * 5     12/18/98 12:31p Johnson
 * Fixed a bug where the server would not properly redistribute a data
 * file that he already had to other players.
 * 
 * 4     12/14/98 4:01p Dave
 * Got multi_data stuff working well with new xfer stuff. 
 * 
 * 3     12/14/98 12:13p Dave
 * Spiffed up xfer system a bit. Put in support for squad logo file xfer.
 * Need to test now.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 19    6/13/98 3:18p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 18    5/21/98 3:45a Sandeep
 * Make sure file xfer sender side uses correct directory type.
 * 
 * 17    5/17/98 6:32p Dave
 * Make sure clients/servers aren't kicked out of the debriefing when team
 * captains leave a game. Fixed chatbox off-by-one error. Fixed image
 * xfer/pilot info popup stuff.
 * 
 * 16    5/13/98 6:54p Dave
 * More sophistication to PXO interface. Changed respawn checking so
 * there's no window for desynchronization between the server and the
 * clients.
 * 
 * 15    4/30/98 4:53p John
 * Restructured and cleaned up cfile code.  Added capability to read off
 * of CD-ROM drive and out of multiple pack files.
 * 
 * 14    4/21/98 4:44p Dave
 * Implement Vasudan ships in multiplayer. Added a debug function to bash
 * player rank. Fixed a few rtvoice buffer overrun problems. Fixed ui
 * problem in options screen. 
 * 
 * 13    4/16/98 1:55p Dave
 * Removed unneeded Assert when processing chat packets. Fixed standalone
 * sequencing bugs. Laid groundwork for join screen server status
 * icons/text.
 * 
 * 12    4/14/98 12:19p Dave
 * Revised the pause system yet again. Seperated into its own module.
 * 
 * 11    4/08/98 2:51p Dave
 * Fixed pilot image xfer once again. Solidify team selection process in
 * pre-briefing multiplayer.
 * 
 * 10    4/04/98 4:22p Dave
 * First rev of UDP reliable sockets is done. Seems to work well if not
 * overly burdened.
 * 
 * 9     3/30/98 6:27p Dave
 * Put in a more official set of multiplayer options, including a system
 * for distributing netplayer and netgame settings.
 * 
 * 8     3/26/98 6:01p Dave
 * Put in file checksumming routine in cfile. Made pilot pic xferring more
 * robust. Cut header size of voice data packets in half. Put in
 * restricted game host query system.
 * 
 * 7     3/23/98 5:42p Dave
 * Put in automatic xfer of pilot pic files. Changed multi_xfer system so
 * that it can support multiplayer sends/received between client and
 * server simultaneously.
 * 
 * 6     3/21/98 7:14p Dave
 * Fixed up standalone player slot switching. Made training missions not
 * count towards player stats.
 * 
 * 5     3/15/98 4:17p Dave
 * Fixed oberver hud problems. Put in handy netplayer macros. Reduced size
 * of network orientation matrices.
 * 
 * 4     3/11/98 2:04p Dave
 * Removed optimized build warnings.
 * 
 * 3     2/20/98 4:43p Dave
 * Finished support for multiplayer player data files. Split off
 * multiplayer campaign functionality.
 * 
 * 2     2/19/98 6:44p Dave
 * Finished server getting data from players. Now need to rebroadcast the
 * data.
 * 
 * 1     2/19/98 6:21p Dave
 * Player data file xfer module.
 * 
 * $NoKeywords: $
 */


#ifndef NO_NETWORK

#include <time.h>
#include <ctype.h>

#include "network/multi.h"
#include "network/multi_data.h"
#include "network/multi_xfer.h"
#include "network/multiutil.h"
#include "playerman/player.h"
#include "cfile/cfile.h"
#include "globalincs/systemvars.h"



// -------------------------------------------------------------------------
// MULTI DATA DEFINES/VARS
//

#define MAX_MULTI_DATA					40

// player data struct
typedef struct np_data {
	char filename[MAX_FILENAME_LEN+1];			// filename
	ubyte status[MAX_PLAYERS];						// status for all players (0 == don't have it, 1 == have or sending, 2 == i sent it originally)
	ushort player_id;									// id of the player who sent the file
	ubyte used;											// in use or not
} np_data;

np_data Multi_data[MAX_MULTI_DATA];

// this doesn't travel off platform to don't _fs_time_t it
time_t Multi_data_time = 0;


// -------------------------------------------------------------------------
// MULTI DATA FORWARD DECLARATIONS
//

// is the given filename for a "multi data" file (pcx, wav, etc)
int multi_data_is_data(char *filename);

// get a free np_data slot
int multi_data_get_free();

// server side - add a new file
int multi_data_add_new(char *filename, int player_index);

// maybe try and reload player squad logo bitmaps
void multi_data_maybe_reload();


// -------------------------------------------------------------------------
// MULTI DATA FUNCTIONS
//

// reset the data xfer system
void multi_data_reset()
{		
	int idx;

	// clear out all used bits
	for(idx=0; idx<MAX_MULTI_DATA; idx++){
		Multi_data[idx].used = 0;
	}
}

// handle a player join (clear out lists of files, etc)
void multi_data_handle_join(int player_index)
{
	int idx;
	
	// clear out his status for all files		
	for(idx=0;idx<MAX_MULTI_DATA;idx++){
		Multi_data[idx].status[player_index] = 0;
	}
}

// handle a player drop (essentially the same as multi_data_handle_join)
void multi_data_handle_drop(int player_index)
{
	int idx;
		
	// mark all files he sent as being unused
	for(idx=0;idx<MAX_MULTI_DATA;idx++){
		if(Multi_data[idx].player_id == Net_players[player_index].player_id){
			Multi_data[idx].used = 0;
		}
	}
}

// do all sync related data stuff (server-side only)
void multi_data_do()
{		
	int idx, p_idx;

	// only do this once a second
	if((time(NULL) - Multi_data_time) < 1){		
		return;
	}

	// maybe try and reload player squad logo bitmaps
	multi_data_maybe_reload();

	// reset the time
	Multi_data_time = time(NULL);

	// if I'm the server
	if(Net_player && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		// for all valid files
		for(idx=0; idx<MAX_MULTI_DATA; idx++){
			// a valid file that isn't currently xferring (ie, anything we've got completely on our hard drive)
			if(Multi_data[idx].used && (multi_xfer_lookup(Multi_data[idx].filename) < 0)){
				// send it to all players who need it
				for(p_idx=0; p_idx<MAX_PLAYERS; p_idx++){
					// if he doesn't have it
					if((Net_player != &Net_players[p_idx]) && MULTI_CONNECTED(Net_players[p_idx]) && (Net_players[p_idx].reliable_socket != INVALID_SOCKET) && (Multi_data[idx].status[p_idx] == 0)){						
						// queue up the file to send to him, or at least try to
						if(multi_xfer_send_file(Net_players[p_idx].reliable_socket, Multi_data[idx].filename, CF_TYPE_ANY, MULTI_XFER_FLAG_AUTODESTROY | MULTI_XFER_FLAG_QUEUE) < 0){
							nprintf(("Network", "Failed to send data file! Trying again later...\n"));
						} else {
							// mark his status
							Multi_data[idx].status[p_idx] = 1;
						}
					}
				}
			}
		}
	}
}

// handle an incoming xfer request from the xfer system
void multi_data_handle_incoming(int handle)
{	
	int player_index = -1;
	PSNET_SOCKET_RELIABLE sock = INVALID_SOCKET;	
	char *fname;		

	// get the player who is sending us this file	
	sock = multi_xfer_get_sock(handle);
	player_index = find_player_socket(sock);

	// get the filename of the file
	fname = multi_xfer_get_filename(handle);
	if(fname == NULL){
		nprintf(("Network", "Could not get file xfer filename! wacky...!\n"));

		// kill the stream
		multi_xfer_xor_flags(handle, MULTI_XFER_FLAG_REJECT);
		return;
	}

	// if this is not a valid data file
	if(!multi_data_is_data(fname)){
		nprintf(("Network", "Not accepting xfer request because its not a valid data file!\n"));
		
		// kill the stream		
		multi_xfer_xor_flags(handle, MULTI_XFER_FLAG_REJECT);	
		return;
	}	

	// if we already have a copy of this file, abort the xfer		
	// Does file exist in \multidata?
	if( cf_exist(fname, CF_TYPE_MULTI_CACHE) ){			
		nprintf(("Network", "Not accepting file xfer because a duplicate exists!\n"));			
	
		// kill the stream		
		multi_xfer_xor_flags(handle, MULTI_XFER_FLAG_REJECT);	

		// if we're the server, we still need to add it to the list though
		if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && (player_index >= 0)){
			multi_data_add_new(fname, player_index);
		}
		return;
	}	
	
	// if I'm the server of the game, do stuff a little differently
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		if(player_index == -1){
			nprintf(("Network", "Could not find player for incoming player data stream!\n"));

			// kill the stream
			multi_xfer_xor_flags(handle, MULTI_XFER_FLAG_REJECT);
			return;
		}	

		// attempt to add the file
		if(!multi_data_add_new(fname, player_index)){
			// kill the stream
			multi_xfer_xor_flags(handle, MULTI_XFER_FLAG_REJECT);
			return;
		} else {
			// force the file to go into the multi cache directory
			multi_xfer_handle_force_dir(handle, CF_TYPE_MULTI_CACHE);
			
			// mark it as autodestroy			
			multi_xfer_xor_flags(handle, MULTI_XFER_FLAG_AUTODESTROY);
		}
	}
	// if i'm a client, this is an incoming file from the server
	else {
		// if i'm not accepting pilot pics, abort
		if(!(Net_player->p_info.options.flags & MLO_FLAG_ACCEPT_PIX)){
			nprintf(("Network", "Client not accepting files because we don't want 'em!\n"));

			// kill the stream		
			multi_xfer_xor_flags(handle, MULTI_XFER_FLAG_REJECT);	
			return;
		}

		// set the xfer handle to autodestroy itself since we don't really want to have to manage all incoming pics
		multi_xfer_xor_flags(handle, MULTI_XFER_FLAG_AUTODESTROY);
		
		// force the file to go into the multi cache directory
		multi_xfer_handle_force_dir(handle, CF_TYPE_MULTI_CACHE);

		// begin receiving the file
		nprintf(("Network", "Client receiving xfer handle %d\n",handle));
	}		
}

// send all my files as necessary
void multi_data_send_my_junk()
{		
	char *with_ext;	
	int bmap, w, h;
	int ok_to_send = 1;

	// pilot pic --------------------------------------------------------------

	// verify that my pilot pic is valid
	if(strlen(Net_player->m_player->image_filename)){
		bmap = bm_load(Net_player->m_player->image_filename);
		if(bmap == -1){			
			ok_to_send = 0;
		}

		// verify image dimensions
		if(ok_to_send){
			w = -1;
			h = -1;
			bm_get_info(bmap,&w,&h);

			// release the bitmap
			bm_release(bmap);
			bmap = -1;

			// if the dimensions are invalid, kill the filename
			if((w != PLAYER_PILOT_PIC_W) || (h != PLAYER_PILOT_PIC_H)){
				ok_to_send = 0;
			}
		}
	} else {		
		ok_to_send = 0;
	}

	if(ok_to_send){
		with_ext = cf_add_ext(Net_player->m_player->image_filename, NOX(".pcx"));
		if(with_ext != NULL){
			strcpy(Net_player->m_player->image_filename, with_ext);
		}

		// host should put his own pic file in the list now
		if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && !(Game_mode & GM_STANDALONE_SERVER) && (strlen(Net_player->m_player->image_filename) > 0)){
			multi_data_add_new(Net_player->m_player->image_filename, MY_NET_PLAYER_NUM);
		}
		// otherwise clients should just queue up a send
		else {
			// add a file extension if necessary			
			multi_xfer_send_file(Net_player->reliable_socket, Net_player->m_player->image_filename, CF_TYPE_ANY, MULTI_XFER_FLAG_AUTODESTROY | MULTI_XFER_FLAG_QUEUE);
		}		
	}


	// squad logo --------------------------------------------------------------

	// verify that my pilot pic is valid
	ok_to_send = 1;
	if(strlen(Net_player->m_player->squad_filename)){
		bmap = bm_load(Net_player->m_player->squad_filename);
		if(bmap == -1){			
			ok_to_send = 0;
		}

		if(ok_to_send){
			// verify image dimensions
			w = -1;
			h = -1;
			bm_get_info(bmap,&w,&h);

			// release the bitmap
			bm_release(bmap);
			bmap = -1;

			// if the dimensions are invalid, kill the filename
			if((w != PLAYER_SQUAD_PIC_W) || (h != PLAYER_SQUAD_PIC_H)){
				ok_to_send = 0;
			}
		}
	} else {		
		ok_to_send = 0;
	}

	if(ok_to_send){
		with_ext = cf_add_ext(Net_player->m_player->squad_filename, NOX(".pcx"));
		if(with_ext != NULL){
			strcpy(Net_player->m_player->squad_filename,with_ext);
		}

		// host should put his own pic file in the list now
		if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && !(Game_mode & GM_STANDALONE_SERVER) && (strlen(Net_player->m_player->squad_filename) > 0)){
			multi_data_add_new(Net_player->m_player->squad_filename, MY_NET_PLAYER_NUM);
		}
		// otherwise clients should just queue up a send
		else {
			// add a file extension if necessary			
			multi_xfer_send_file(Net_player->reliable_socket, Net_player->m_player->squad_filename, CF_TYPE_ANY, MULTI_XFER_FLAG_AUTODESTROY | MULTI_XFER_FLAG_QUEUE);
		}		
	}
}


// -------------------------------------------------------------------------
// MULTI DATA FORWARD DECLARATIONS
//

// is the give file xfer handle for a "multi data" file (pcx, wav, etc)
int multi_data_is_data(char *filename)
{		
	int len,idx;

	Assert(filename != NULL);

	// some kind of error
	if(filename == NULL){
		return 0;
	}

	// convert to lowercase
	len = strlen(filename);
	for(idx=0;idx<len;idx++){
		filename[idx] = (char)tolower(filename[idx]);
	}

	// check to see if the extension is .pcx
	if(strstr(filename, NOX(".pcx"))){
		return 1;
	}

	// not a data file
	return 0;
}

// get a free np_data slot
int multi_data_get_free()
{
	int idx;

	// find a free slot
	for(idx=0; idx<MAX_MULTI_DATA; idx++){
		if(!Multi_data[idx].used){
			return idx;
		}
	}

	// couldn't find one
	return -1;
}

// server side - add a new file. return 1 on success
int multi_data_add_new(char *filename, int player_index)
{	
	int slot;
		
	// try and get a free slot
	slot = multi_data_get_free();
	if(slot < 0){
		nprintf(("Network", "Could not get free np_data slot! yikes!\n"));		
		return 0;
	}

	// if the netgame isn't flagged as accepting data files
	if(!(Netgame.options.flags & MSO_FLAG_ACCEPT_PIX)){
		nprintf(("Network", "Server not accepting pilot pic because we don't want 'em!\n"));	
		return 0;
	}			

	// assign the data
	memset(&Multi_data[slot], 0, sizeof(np_data));								// clear the slot out
	strcpy(Multi_data[slot].filename, filename);									// copy the filename
	Multi_data[slot].used = 1;															// set it as being in use
	Multi_data[slot].player_id = Net_players[player_index].player_id;		// player id of who's sending the file
	Multi_data[slot].status[player_index] = 2;									// mark his status appropriately

	// success
	return 1;
}

// maybe try and reload player squad logo bitmaps
void multi_data_maybe_reload()
{
	int idx;

	// go through all connected and try to reload their images if necessary
	for(idx=0; idx<MAX_PLAYERS; idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && strlen(Net_players[idx].m_player->squad_filename) && (Net_players[idx].m_player->insignia_texture == -1)){
			Net_players[idx].m_player->insignia_texture = bm_load_duplicate(Net_players[idx].m_player->squad_filename);

			// if the bitmap loaded properly, lock it as a transparent texture
			if(Net_players[idx].m_player->insignia_texture != -1){
				bm_lock(Net_players[idx].m_player->insignia_texture, 16, BMP_TEX_XPARENT);
				bm_unlock(Net_players[idx].m_player->insignia_texture);
			}
		}
	}	
}

#endif // !NO_NETWORK
