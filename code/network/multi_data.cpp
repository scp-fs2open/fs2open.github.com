/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




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
	if (cf_exists(fname, CF_TYPE_MULTI_CACHE)) {
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
			strcpy_s(Net_player->m_player->image_filename, with_ext);
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
			strcpy_s(Net_player->m_player->squad_filename,with_ext);
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
	strcpy_s(Multi_data[slot].filename, filename);									// copy the filename
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
