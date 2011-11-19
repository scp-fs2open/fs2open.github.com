/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "network/multi_xfer.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/psnet2.h"
#include "io/timer.h"
#include "cfile/cfile.h"

#ifndef NDEBUG
#include "playerman/player.h"
#include "network/multiutil.h"
#include "network/multi_log.h"
#endif




// ------------------------------------------------------------------------------------------
// MULTI XFER DEFINES/VARS
//

#define MULTI_XFER_VERBOSE										// keep this defined for verbose debug output

#define MULTI_XFER_INVALID_HANDLE(handle) ( (handle < 0) || (handle > (MAX_XFER_ENTRIES-1)) || !(Multi_xfer_entry[handle].flags & MULTI_XFER_FLAG_USED) || (strlen(Multi_xfer_entry[handle].filename) <= 0) )

// packet codes
#define MULTI_XFER_CODE_ACK					0				// simple response to the last request
#define MULTI_XFER_CODE_NAK					1				// simple response to the last request
#define MULTI_XFER_CODE_HEADER				2				// file xfer header information follows, requires a HEADER_RESPONSE
#define MULTI_XFER_CODE_DATA					3				// data block follows, requires an ack
#define MULTI_XFER_CODE_FINAL					4				// indication from sender that xfer is complete, requires an ack

// entry flags
#define MULTI_XFER_FLAG_USED					(1<<0)		// this entry is in use	
#define MULTI_XFER_FLAG_SEND					(1<<1)		// this entry is sending a file
#define MULTI_XFER_FLAG_RECV					(1<<2)		// this entry is receiving a file
#define MULTI_XFER_FLAG_PENDING				(1<<3)		// this entry is ready to send a header and start the process
#define MULTI_XFER_FLAG_WAIT_ACK				(1<<4)		// waiting for an ack/nak
#define MULTI_XFER_FLAG_WAIT_DATA			(1<<5)		// waiting for another block of data 
#define MULTI_XFER_FLAG_UNKNOWN				(1<<6)		// xfer final has been sent, and we are waiting for a response
#define MULTI_XFER_FLAG_SUCCESS				(1<<7)		// finished xfer
#define MULTI_XFER_FLAG_FAIL					(1<<8)		// xfer failed
#define MULTI_XFER_FLAG_TIMEOUT				(1<<9)		// xfer has timed-out
#define MULTI_XFER_FLAG_QUEUE_CURRENT		(1<<10)		// for a set of XFER_FLAG_QUEUE'd files, this is the current one sending

// packet size for file xfer
#define MULTI_XFER_MAX_DATA_SIZE				490			// this will keep us within the MULTI_XFER_MAX_SIZE_LIMIT

// timeout for a given xfer operation
#define MULTI_XFER_TIMEOUT						10000		

//XSTR:OFF

// temp filename header for xferring files
#define MULTI_XFER_FNAME_PREFIX				"_fsx_"

//XSTR:ON

// xfer entries/events
#define MAX_XFER_ENTRIES						60				// the max allowed file xfer entries
typedef struct xfer_entry {
	int flags;														// status flags for this entry
	char filename[MAX_FILENAME_LEN+1];						// filename of the currently xferring file
	char ex_filename[MAX_FILENAME_LEN+10];					// filename with xfer prefix tacked on to the front
	CFILE *file;													// file handle of the current xferring file
	int file_size;													// total size of the file being xferred
	int file_ptr;													// total bytes we're received so far
	ushort file_chksum;											// used for checking successfully xferred files
	PSNET_SOCKET_RELIABLE file_socket;						// socket used to xfer the file	
	int xfer_stamp;												// timestamp for the current operation		
	int force_dir;													// force the file to go to this directory on receive (will override Multi_xfer_force_dir)	
	ushort sig;														// identifying sig - sender specifies this
} xfer_entry;
xfer_entry Multi_xfer_entry[MAX_XFER_ENTRIES];			// the file xfer entries themselves

// callback function pointer for when we start receiving a file
void (*Multi_xfer_recv_notify)(int handle);

// lock for the xfer system
int Multi_xfer_locked;

// force directory type for receives
int Multi_xfer_force_dir; 

// unique file signature - this along with a socket # is enough to identify all xfers
ushort Multi_xfer_sig = 0;


// ------------------------------------------------------------------------------------------
// MULTI XFER FORWARD DECLARATIONS
//

// evaluate the condition of the entry
void multi_xfer_eval_entry(xfer_entry *xe);

// set an entry to be "failed"
void multi_xfer_fail_entry(xfer_entry *xe);

// get a valid xfer entry handle
int multi_xfer_get_free_handle();

// process an ack for this entry
void multi_xfer_process_ack(xfer_entry *xe);

// process a nak for this entry
void multi_xfer_process_nak(xfer_entry *xe);
		
// process a "final" packet	
void multi_xfer_process_final(xfer_entry *xe);		

// process a data packet
void multi_xfer_process_data(xfer_entry *xe, ubyte *data, int data_size);
	
// process a header
void multi_xfer_process_header(ubyte *data, PSNET_SOCKET_RELIABLE who, ushort sig, char *filename, int file_size, ushort file_checksum);		

// send the next block of outgoing data or a "final" packet if we're done
void multi_xfer_send_next(xfer_entry *xe);

// send an ack to the sender
void multi_xfer_send_ack(PSNET_SOCKET_RELIABLE socket, ushort sig);

// send a nak to the sender
void multi_xfer_send_nak(PSNET_SOCKET_RELIABLE socket, ushort sig);

// send a "final" packet
void multi_xfer_send_final(xfer_entry *xe);

// send the header to begin a file transfer
void multi_xfer_send_header(xfer_entry *xe);

// convert the filename into the prefixed ex_filename
void multi_xfer_conv_prefix(char *filename, char *ex_filename);

// get a new xfer sig
ushort multi_xfer_get_sig();

// ------------------------------------------------------------------------------------------
// MULTI XFER FUNCTIONS
//

// initialize all file xfer transaction stuff, call in multi_level_init()
void multi_xfer_init(void (*multi_xfer_recv_callback)(int handle))
{
	// blast all the entries
	memset(Multi_xfer_entry,0,sizeof(xfer_entry) * MAX_XFER_ENTRIES);

	// assign the receive callback function pointer
	Multi_xfer_recv_notify = multi_xfer_recv_callback;

	// unlocked
	Multi_xfer_locked = 0;

	// no forced directory
	Multi_xfer_force_dir = CF_TYPE_MULTI_CACHE;	
}

// do frame for all file xfers, call in multi_do_frame()
void multi_xfer_do()
{
	int idx;

	// process all valid xfer entries
	for(idx=0;idx<MAX_XFER_ENTRIES;idx++){
		// if this one is actually in use and has not finished for one reason or another
		if((Multi_xfer_entry[idx].flags & MULTI_XFER_FLAG_USED) && !(Multi_xfer_entry[idx].flags & (MULTI_XFER_FLAG_SUCCESS | MULTI_XFER_FLAG_FAIL | MULTI_XFER_FLAG_TIMEOUT))){
			// evaluate the condition of this entry (fail, timeout, etc)
			multi_xfer_eval_entry(&Multi_xfer_entry[idx]);			
		}
	}
}

// reset the xfer system, including shutting down/killing all active xfers
void multi_xfer_reset()
{
	int idx;

	// shut down all active xfers
	for(idx=0;idx<MAX_XFER_ENTRIES;idx++){
		if(Multi_xfer_entry[idx].flags & MULTI_XFER_FLAG_USED){
			multi_xfer_abort(idx);
		}
	}

	// blast all the memory clean
	memset(Multi_xfer_entry,0,sizeof(xfer_entry) * MAX_XFER_ENTRIES);
}

// send a file to the specified player, return a handle
int multi_xfer_send_file(PSNET_SOCKET_RELIABLE who, char *filename, int cfile_flags, int flags)
{
	xfer_entry temp_entry;	
	int handle;

	// if the system is locked, return -1
	if(Multi_xfer_locked){
		return -1;
	}

	// attempt to get a free handle
	handle = multi_xfer_get_free_handle();
	if(handle == -1){
		return -1;
	}

	// clear the temp entry
	memset(&temp_entry,0,sizeof(xfer_entry));

	// set the filename
	strcpy_s(temp_entry.filename,filename);	

	// attempt to open the file
	temp_entry.file = NULL;
	temp_entry.file = cfopen(filename,"rb",CFILE_NORMAL,cfile_flags);
	if(temp_entry.file == NULL){
#ifdef MULTI_XFER_VERBOSE
		nprintf(("Network","MULTI XFER : Could not open file %s on xfer send!\n",filename));
#endif

		return -1;
	}

	// set the file size
	temp_entry.file_size = -1;
	temp_entry.file_size = cfilelength(temp_entry.file);
	if(temp_entry.file_size == -1){
#ifdef MULTI_XFER_VERBOSE
		nprintf(("Network","MULTI XFER : Could not get file length for file %s on xfer send\n",filename));
#endif
		return -1;
	}
	temp_entry.file_ptr = 0;

	// get the file checksum
	if(!cf_chksum_short(temp_entry.file,&temp_entry.file_chksum)){
#ifdef MULTI_XFER_VERBOSE
		nprintf(("Network","MULTI XFER : Could not get file checksum for file %s on xfer send\n",filename));
#endif
		return -1;
	} 
#ifdef MULTI_XFER_VERBOSE
	nprintf(("Network","MULTI XFER : Got file %s checksum of %d\n",temp_entry.filename,(int)temp_entry.file_chksum));
#endif
	// rewind the file pointer to the beginning of the file
	cfseek(temp_entry.file,0,CF_SEEK_SET);

	// set the flags
	temp_entry.flags |= (MULTI_XFER_FLAG_USED | MULTI_XFER_FLAG_SEND | MULTI_XFER_FLAG_PENDING);
	temp_entry.flags |= flags;

	// set the socket	
	temp_entry.file_socket = who;		

	// set the signature
	temp_entry.sig = multi_xfer_get_sig();

	// copy to the global array
	memset(&Multi_xfer_entry[handle],0,sizeof(xfer_entry));
	memcpy(&Multi_xfer_entry[handle],&temp_entry,sizeof(xfer_entry));
	
	return handle;
}

// get the status of the current file xfer
int multi_xfer_get_status(int handle)
{
	// if this is an invalid or an unused handle, notify as such
	if((handle < 0) || (handle > (MAX_XFER_ENTRIES-1)) || !(Multi_xfer_entry[handle].flags & MULTI_XFER_FLAG_USED) ){
		return MULTI_XFER_NONE;
	}	
	
	// if the xfer has timed-out
	if(Multi_xfer_entry[handle].flags & MULTI_XFER_FLAG_TIMEOUT){
		return MULTI_XFER_TIMEDOUT;
	}

	// if the xfer has failed for one reason or another (not timeout)
	if(Multi_xfer_entry[handle].flags & MULTI_XFER_FLAG_FAIL){
		return MULTI_XFER_FAIL;
	}

	// if the xfer has succeeded
	if(Multi_xfer_entry[handle].flags & MULTI_XFER_FLAG_SUCCESS){
		return MULTI_XFER_SUCCESS;
	}

	// if the xfer is queued
	if((Multi_xfer_entry[handle].flags & MULTI_XFER_FLAG_QUEUE) && !(Multi_xfer_entry[handle].flags & MULTI_XFER_FLAG_QUEUE_CURRENT)){
		return MULTI_XFER_QUEUED;
	}

	// otherwise the xfer is still in progress
	return MULTI_XFER_IN_PROGRESS;
}

// abort a transferring file
void multi_xfer_abort(int handle)
{
	xfer_entry *xe;

	// don't do anything if this is an invalid handle
	if(MULTI_XFER_INVALID_HANDLE(handle)){
		return;
	}

	// get e handle to the entry
	xe = &Multi_xfer_entry[handle];

	// close any open file and delete it 
	if(xe->file != NULL){
		cfclose(xe->file);
		xe->file = NULL;

		// delete it if there isn't some problem with the filename
		if((xe->flags & MULTI_XFER_FLAG_RECV) && (xe->filename[0] != '\0')){
			cf_delete(xe->ex_filename, xe->force_dir);
		}
	}

	// zero the socket
	xe->file_socket = INVALID_SOCKET;

	// blast the entry
	memset(xe,0,sizeof(xfer_entry));
}

// release an xfer handle
void multi_xfer_release_handle(int handle)
{
	xfer_entry *xe;

	// don't do anything if this is an invalid handle
	if(MULTI_XFER_INVALID_HANDLE(handle)){
		return;
	}

	// get e handle to the entry
	xe = &Multi_xfer_entry[handle];

	// close any open file and delete it 
	if(xe->file != NULL){
		cfclose(xe->file);
		xe->file = NULL;

		// delete it if the file was not successfully received
		if(!(xe->flags & MULTI_XFER_FLAG_SUCCESS) && (xe->flags & MULTI_XFER_FLAG_RECV) && (xe->filename[0] != '\0')){
			cf_delete(xe->ex_filename,xe->force_dir);
		}
	}

	// zero the socket
	xe->file_socket = INVALID_SOCKET;	

	// blast the entry
	memset(xe,0,sizeof(xfer_entry));
}

// get the filename of the xfer for the given handle
char *multi_xfer_get_filename(int handle)
{
	// if this is an invalid handle, return NULL
	if(MULTI_XFER_INVALID_HANDLE(handle)){
		return NULL;
	}

	// otherwise return the string
	return Multi_xfer_entry[handle].filename;
}

// lock the xfer system (don't accept incoming files, don't allow outgoing files)
void multi_xfer_lock()
{
	Multi_xfer_locked = 1;
}

// unlock the xfer system
void multi_xfer_unlock()
{
	Multi_xfer_locked = 0;
}

// force all receives to go into the specified directory by cfile type
void multi_xfer_force_dir(int cf_type)
{
	Multi_xfer_force_dir = cf_type;
	Assert(Multi_xfer_force_dir > CF_TYPE_ANY);
}

// forces the given xfer entry to the specified directory type (only valid when called from the recv_callback function)
void multi_xfer_handle_force_dir(int handle,int cf_type)
{
	// if this is an invalid handle, return NULL
	if(MULTI_XFER_INVALID_HANDLE(handle)){
		return;
	}

	// force to go to the given directory
	Multi_xfer_entry[handle].force_dir = cf_type;
	Assert(Multi_xfer_entry[handle].force_dir > CF_TYPE_ANY);
}

// or the flag on a given entry
void multi_xfer_xor_flags(int handle,int flags)
{
	// if this is an invalid handle, return NULL
	if(MULTI_XFER_INVALID_HANDLE(handle)){
		return;
	}

	// xor the flags
	Multi_xfer_entry[handle].flags ^= flags;
}

// get the flags for a given entry
int multi_xfer_get_flags(int handle)
{
	// if this is an invalid handle, return NULL
	if(MULTI_XFER_INVALID_HANDLE(handle)){
		return -1;
	}

	// return the flags
	return Multi_xfer_entry[handle].flags;
}

// if the passed filename is being xferred, return the xfer handle, otherwise return -1
int multi_xfer_lookup(char *filename)
{
	int idx;

	// if we have an invalid filename, do nothing
	if((filename == NULL) || (strlen(filename) <= 0)){
		return 0;
	}

	// otherwise, perform a lookup
	for(idx=0;idx<MAX_XFER_ENTRIES;idx++){
		// if we found a matching filename
		if((Multi_xfer_entry[idx].flags & MULTI_XFER_FLAG_USED) && !stricmp(filename,Multi_xfer_entry[idx].filename)){
			return idx;
		}
	}

	// did not find a match
	return -1;
}

// get the % of completion of the passed file handle, return < 0 if invalid
float multi_xfer_pct_complete(int handle)
{
	// if this is an invalid handle, return invalid
	if(MULTI_XFER_INVALID_HANDLE(handle)){
		return -1.0f;
	}

	// if the file size is 0, return invalid
	if(Multi_xfer_entry[handle].file_size == 0){
		return -1.0f;
	}

	// return the pct completion
	return (float)Multi_xfer_entry[handle].file_ptr / (float)Multi_xfer_entry[handle].file_size;
}

// get the socket of the file xfer (useful for identifying players)
uint multi_xfer_get_sock(int handle)
{
	// if this is an invalid handle, return NULL
	if(MULTI_XFER_INVALID_HANDLE(handle)){
		return INVALID_SOCKET;
	}

	return Multi_xfer_entry[handle].file_socket;
}

// get the CF_TYPE of the directory this file is going to
int multi_xfer_get_force_dir(int handle)
{
	// if this is an invalid handle, return NULL
	if(MULTI_XFER_INVALID_HANDLE(handle)){
		return INVALID_SOCKET;
	}

	return Multi_xfer_entry[handle].force_dir;
}


// ------------------------------------------------------------------------------------------
// MULTI XFER FORWARD DECLARATIONS
//

// evaluate the condition of the entry
void multi_xfer_eval_entry(xfer_entry *xe)
{	
	int idx;
	int found;
	xfer_entry *xe_c;

	// if the entry is marked as successful, then don't do anything
	if(xe->flags & MULTI_XFER_FLAG_SUCCESS){
		return;
	}

	// if the entry is queued
	if(xe->flags & MULTI_XFER_FLAG_QUEUE){
		// if the entry is not current
		if(!(xe->flags & MULTI_XFER_FLAG_QUEUE_CURRENT)){
			// see if there are any other queued up xfers to this target. if not, make me current and start sending
			found = 0;
			for(idx=0; idx<MAX_XFER_ENTRIES; idx++){
				xe_c = &Multi_xfer_entry[idx];

				// if this is a valid entry and is a queued entry and is going to the same target
				if((xe_c->flags & MULTI_XFER_FLAG_USED) && (xe_c->file_socket == xe->file_socket) && (xe_c->flags & MULTI_XFER_FLAG_SEND) && 
					(xe_c->flags & MULTI_XFER_FLAG_QUEUE) && (xe_c->flags & MULTI_XFER_FLAG_QUEUE_CURRENT)){
					
					found = 1;
					break;
				}				
			}

			// if we found no other entries, make this guy current and pending
			if(!found){
				xe->flags |= MULTI_XFER_FLAG_QUEUE_CURRENT;
				xe->flags |= MULTI_XFER_FLAG_PENDING;

#ifdef MULTI_XFER_VERBOSE
				nprintf(("Network","MULTI_XFER : Starting xfer send for queued entry %s\n", xe->filename));
#endif
			} 
			// otherwise, do nothing for him - he has to still wait
			else {
				return;
			}
		}
	}

	// if the entry is marked as pending - send out the header to get the ball rolling
	if(xe->flags & MULTI_XFER_FLAG_PENDING){		
		// send the header to begin the transfer
		multi_xfer_send_header(xe);

		// set the timestamp
		xe->xfer_stamp = timestamp(MULTI_XFER_TIMEOUT);	

		// unset the pending flag
		xe->flags &= ~(MULTI_XFER_FLAG_PENDING);

		// set the ack/wait flag
		xe->flags |= MULTI_XFER_FLAG_WAIT_ACK;
	}
	
	// see if the entry has timed-out for one reason or another
	if((xe->xfer_stamp != -1) && timestamp_elapsed(xe->xfer_stamp)){
		xe->flags |= MULTI_XFER_FLAG_TIMEOUT;			

		// if we should be auto-destroying this entry, do so
		if(xe->flags & MULTI_XFER_FLAG_AUTODESTROY){
			multi_xfer_fail_entry(xe);			
		}
	}		
}

// lookup a file xfer entry by player
xfer_entry *multi_xfer_find_entry(PSNET_SOCKET_RELIABLE who, ushort sig, int sender_side)
{
	int idx;

	// look through all valid xfer entries
	for(idx=0;idx<MAX_XFER_ENTRIES;idx++){
		// if we're looking for sending entries
		if(sender_side && !(Multi_xfer_entry[idx].flags & MULTI_XFER_FLAG_SEND)){
			continue;
		}
		// if we're looking for recv entries
		if(!sender_side && !(Multi_xfer_entry[idx].flags & MULTI_XFER_FLAG_RECV)){
			continue;
		}

		// if we found a match
		if((Multi_xfer_entry[idx].file_socket == who) && (Multi_xfer_entry[idx].sig == sig)){
			return &Multi_xfer_entry[idx];
		}
	}

	return NULL;
}

// set an entry to be "failed"
void multi_xfer_fail_entry(xfer_entry *xe)
{
	// set its flags appropriately
	xe->flags &= ~(MULTI_XFER_FLAG_WAIT_ACK | MULTI_XFER_FLAG_WAIT_DATA | MULTI_XFER_FLAG_UNKNOWN);
	xe->flags |= MULTI_XFER_FLAG_FAIL;

	// close the file pointer
	if(xe->file != NULL){
		cfclose(xe->file);
		xe->file = NULL;
	}

	// delete the file
	if((xe->flags & MULTI_XFER_FLAG_RECV) && (xe->filename[0] != '\0')){
		cf_delete(xe->ex_filename,xe->force_dir);
	}
		
	// null the timestamp
	xe->xfer_stamp = -1;

	// if we should be auto-destroying this entry, do so
	if(xe->flags & MULTI_XFER_FLAG_AUTODESTROY){
		multi_xfer_release_handle(xe - Multi_xfer_entry);
	}

	// blast the memory clean
	memset(xe,0,sizeof(xfer_entry));
}

// get a valid xfer entry handle
int multi_xfer_get_free_handle()
{
	int idx;

	// look for a free entry
	for(idx=0;idx<MAX_XFER_ENTRIES;idx++){
		if(!(Multi_xfer_entry[idx].flags & MULTI_XFER_FLAG_USED)){
			return idx;
		}
	}

	return -1;
}


// ------------------------------------------------------------------------------------------
// MULTI XFER PACKET HANDLERS
//

// process an incoming file xfer data packet, return bytes processed, guaranteed to process the entire
// packet regardless of error conditions
int multi_xfer_process_packet(unsigned char *data, PSNET_SOCKET_RELIABLE who)
{	
	ubyte val;	
	xfer_entry *xe;
	char filename[255];
	ushort data_size = 0;
	int file_size = -1;
	ushort file_checksum = 0;
	int offset = 0;
	ubyte xfer_data[600];
	ushort sig;
	int sender_side = 1;

	// read in all packet data
	GET_DATA(val);	
	GET_USHORT(sig);
	switch(val){
	// RECV side
	case MULTI_XFER_CODE_DATA:				
		GET_USHORT(data_size);		
		memcpy(xfer_data, data + offset, data_size);
		offset += data_size;
		sender_side = 0;
		break;

	// RECV side
	case MULTI_XFER_CODE_HEADER:		
		GET_STRING(filename);
		GET_INT(file_size);					
		GET_USHORT(file_checksum);
		sender_side = 0;
		break;

	// SEND side
	case MULTI_XFER_CODE_ACK:
	case MULTI_XFER_CODE_NAK:
		break;

	// RECV side
	case MULTI_XFER_CODE_FINAL:
		sender_side = 0;
		break;

	default:
		Int3();
	}			

	// at this point we've read all the data in the packet

	// at this point, we should process code-specific data	
	xe = NULL;
	if(val != MULTI_XFER_CODE_HEADER){		
		// if the code is not a request or a header, we need to look up the existing xfer_entry
		xe = NULL;
			
		xe = multi_xfer_find_entry(who, sig, sender_side);
		if(xe == NULL){						
#ifdef MULTI_XFER_VERBOSE
			nprintf(("Network","MULTI XFER : Could not find xfer entry for incoming data!\n"));

			// this is a rare case - I'm not overly concerned about it. But it _does_ happen. So blech
#ifndef NDEBUG
			int np_index = find_player_socket(who);
			ml_string("MULTI XFER : Could not find xfer entry for incoming data :");
			ml_printf(": sig == %d", sig);
			ml_printf(": xfer header == %d", val);
			if(np_index < 0){
				ml_string(": player == unknown");
			} else {
				ml_printf(": player == %s", Net_players[np_index].m_player->callsign);
			}
			if(sender_side){
				ml_string(": sending");
			} else {
				ml_string(": receiving");
			}
#endif
#endif
			return offset;
		}
	}

	switch((int)val){
	// process an ack for this entry
	case MULTI_XFER_CODE_ACK :
		Assert(xe != NULL);
		multi_xfer_process_ack(xe);
		break;
	
	// process a nak for this entry
	case MULTI_XFER_CODE_NAK :
		Assert(xe != NULL);
		multi_xfer_process_nak(xe);
		break;

	// process a "final" packet
	case MULTI_XFER_CODE_FINAL :
		Assert(xe != NULL);
		multi_xfer_process_final(xe);
		break;

	// process a data packet
	case MULTI_XFER_CODE_DATA :
		Assert(xe != NULL);
		multi_xfer_process_data(xe, xfer_data, data_size);
		break;
	
	// process a header
	case MULTI_XFER_CODE_HEADER :
		// send on my reliable socket
		multi_xfer_process_header(xfer_data, who, sig, filename, file_size, file_checksum);
		break;
	}		
	return offset;
}

// process an ack for this entry
void multi_xfer_process_ack(xfer_entry *xe)
{			
	// if we are a sender
	if(xe->flags & MULTI_XFER_FLAG_SEND){
		// if we are waiting on a final ack, then the transfer has completed successfully
		if(xe->flags & MULTI_XFER_FLAG_UNKNOWN){
			xe->flags &= ~(MULTI_XFER_FLAG_UNKNOWN);
			xe->flags |= MULTI_XFER_FLAG_SUCCESS;

#ifdef MULTI_XFER_VERBOSE
			nprintf(("Network", "MULTI XFER : Successfully sent file %s\n", xe->filename));
#endif

			// if we should be auto-destroying this entry, do so
			if(xe->flags & MULTI_XFER_FLAG_AUTODESTROY){
				multi_xfer_release_handle(xe - Multi_xfer_entry);
			}
		} 
		// otherwise if we're waiting for an ack, we should send the next chunk of data or a "final" packet if we're done
		else if(xe->flags & MULTI_XFER_FLAG_WAIT_ACK){
			multi_xfer_send_next(xe);
		}
	}
}

// process a nak for this entry
void multi_xfer_process_nak(xfer_entry *xe)
{		
	// if we get an ack at any time we should simply set the xfer to failed
	multi_xfer_fail_entry(xe);
}
		
// process a "final" packet	
void multi_xfer_process_final(xfer_entry *xe)
{	
	ushort chksum;

	// make sure we skip a line
	nprintf(("Network","\n"));
	
	// close the file
	if(xe->file != NULL){
		cflush(xe->file);
		cfclose(xe->file);
		xe->file = NULL;
	}	

	// check to make sure the file checksum is the same
	chksum = 0;
	if(!cf_chksum_short(xe->ex_filename, &chksum, -1, xe->force_dir) || (chksum != xe->file_chksum)){
		// mark as failed
		xe->flags |= MULTI_XFER_FLAG_FAIL;

#ifdef MULTI_XFER_VERBOSE
		nprintf(("Network","MULTI XFER : file %s failed checksum %d %d!\n",xe->ex_filename, (int)xe->file_chksum, (int)chksum));
#endif

		// abort the xfer
		multi_xfer_abort(xe - Multi_xfer_entry);
		return;
	}
	// checksums check out, so rename the file and be done with it
	else {
#ifdef MULTI_XFER_VERBOSE
		nprintf(("Network","MULTI XFER : renaming xferred file from %s to %s (chksum %d %d)\n", xe->ex_filename, xe->filename, (int)xe->file_chksum, (int)chksum));
#endif
		// rename the file properly
		if(cf_rename(xe->ex_filename,xe->filename, xe->force_dir) == CF_RENAME_SUCCESS){
			// mark the xfer as being successful
			xe->flags |= MULTI_XFER_FLAG_SUCCESS;	

			nprintf(("Network","MULTI XFER : SUCCESSFULLY TRANSFERRED FILE %s (%d bytes)\n", xe->filename, xe->file_size));		

			// send an ack to the sender
			multi_xfer_send_ack(xe->file_socket, xe->sig);
		} else {
			// mark it as failing
			xe->flags |= MULTI_XFER_FLAG_FAIL;
			nprintf(("Network","FAILED TO TRANSFER FILE (could not rename temp file %s)\n", xe->ex_filename));

			// delete the tempfile
			cf_delete(xe->ex_filename, xe->force_dir);

			// send an nak to the sender
			multi_xfer_send_nak(xe->file_socket, xe->sig);
		}

		// if we should be auto-destroying this entry, do so
		if(xe->flags & MULTI_XFER_FLAG_AUTODESTROY){
			multi_xfer_release_handle(xe - Multi_xfer_entry);
		}
	}
}

// process a data packet
void multi_xfer_process_data(xfer_entry *xe, ubyte *data, int data_size)	
{			
	// print out a crude progress indicator
	nprintf(("Network","."));		

	// attempt to write the rest of the data string to the file
	if((xe->file == NULL) || !cfwrite(data, data_size, 1, xe->file)){
		// inform the sender we had a problem
		multi_xfer_send_nak(xe->file_socket, xe->sig);

		// fail this entry
		multi_xfer_fail_entry(xe);

		xe->file_ptr += data_size;		
		return;
	}

	// increment the file pointer
	xe->file_ptr += data_size;

	// send an ack to the sender
	multi_xfer_send_ack(xe->file_socket, xe->sig);

	// set the timestmp
	xe->xfer_stamp = timestamp(MULTI_XFER_TIMEOUT);	
}
	
// process a header, return bytes processed
void multi_xfer_process_header(ubyte *data, PSNET_SOCKET_RELIABLE who, ushort sig, char *filename, int file_size, ushort file_checksum)
{		
	xfer_entry *xe;		
	int handle;	

	// if the xfer system is locked, send a nak
	if(Multi_xfer_locked){		
		multi_xfer_send_nak(who, sig);
		return;
	}

	// try and get a free xfer handle
	handle = multi_xfer_get_free_handle();
	if(handle == -1){		
		multi_xfer_send_nak(who, sig);
		return;
	} else {
		xe = &Multi_xfer_entry[handle];
		memset(xe,0,sizeof(xfer_entry));
	}		

	// set the recv and used flags
	xe->flags |= (MULTI_XFER_FLAG_USED | MULTI_XFER_FLAG_RECV);

	// get the header data	
	xe->file_size = file_size;

	// get the file chksum
	xe->file_chksum = file_checksum;	

	// set the socket
	xe->file_socket = who;	

	// set the timeout timestamp
	xe->xfer_stamp = timestamp(MULTI_XFER_TIMEOUT);

	// set the sig
	xe->sig = sig;

	// copy the filename and get the prefixed xfer filename
	strcpy_s(xe->filename, filename);
	multi_xfer_conv_prefix(xe->filename, xe->ex_filename);
#ifdef MULTI_XFER_VERBOSE
	nprintf(("Network","MULTI XFER : converted filename %s to %s\n",xe->filename, xe->ex_filename));
#endif

	// determine what directory to place the file in
	// individual xfer entries take precedence over the global multi xfer force entry	
	xe->force_dir = Multi_xfer_force_dir;	

	// call the callback function
	Multi_xfer_recv_notify(handle);	

	// if the notify function invalidated this xfer handle, then cancel the whole thing
	if(xe->flags & MULTI_XFER_FLAG_REJECT){		
		multi_xfer_send_nak(who, sig);
		
		// clear the data
		memset(xe, 0, sizeof(xfer_entry));
		return;
	}			

	// delete the old file (if it exists)
	cf_delete( xe->filename, CF_TYPE_MULTI_CACHE );
	cf_delete( xe->filename, CF_TYPE_MISSIONS );

	// attempt to open the file (using the prefixed filename)
	xe->file = NULL;
	xe->file = cfopen(xe->ex_filename, "wb", CFILE_NORMAL, xe->force_dir);
	if(xe->file == NULL){		
		multi_xfer_send_nak(who, sig);		

		// clear the data
		memset(xe, 0, sizeof(xfer_entry));
		return;
	}
	
	// set the waiting for data flag
	xe->flags |= MULTI_XFER_FLAG_WAIT_DATA;		

	// send an ack to the server		
	multi_xfer_send_ack(who, sig);	

#ifdef MULTI_XFER_VERBOSE
	nprintf(("Network","MULTI XFER : AFTER HEADER %s\n",xe->filename));
#endif	
}

// send the next block of outgoing data or a "final" packet if we're done
void multi_xfer_send_next(xfer_entry *xe)
{
	ubyte data[MAX_PACKET_SIZE],code;
	ushort data_size;
	int flen;
	int packet_size = 0;	

	// print out a crude progress indicator
	nprintf(("Network", "+"));		

	// if we've sent all the data, then we should send a "final" packet
	if(xe->file_ptr >= xe->file_size){
		// mark the entry as unknown 
		xe->flags |= MULTI_XFER_FLAG_UNKNOWN;

		// set the timestmp
		xe->xfer_stamp = timestamp(MULTI_XFER_TIMEOUT);

		// send the packet
		multi_xfer_send_final(xe);		

		return;
	}

	// build the header 
	BUILD_HEADER(XFER_PACKET);	

	// length of the added string
	flen = strlen(xe->filename) + 4;

	// determine how much data we are going to send with this packet and add it in
	if((xe->file_size - xe->file_ptr) >= (MULTI_XFER_MAX_DATA_SIZE - flen)){
		data_size = (ushort)(MULTI_XFER_MAX_DATA_SIZE - flen);
	} else {
		data_size = (unsigned short)(xe->file_size - xe->file_ptr);
	}
	// increment the file pointer
	xe->file_ptr += data_size;	

	// add the opcode
	code = MULTI_XFER_CODE_DATA;
	ADD_DATA(code);

	// add the sig
	ADD_USHORT(xe->sig);

	// add in the size of the rest of the packet	
	ADD_USHORT(data_size);
	
	// copy in the data
	if(cfread(data+packet_size,1,(int)data_size,xe->file) == 0){
		// send a nack to the receiver
		multi_xfer_send_nak(xe->file_socket, xe->sig);

		// fail this send
		multi_xfer_fail_entry(xe);		
		return;
	}

	// increment the packet size
	packet_size += (int)data_size;

	// set the timestmp
	xe->xfer_stamp = timestamp(MULTI_XFER_TIMEOUT);

	// otherwise send the data	
	psnet_rel_send(xe->file_socket, data, packet_size);
}

// send an ack to the sender
void multi_xfer_send_ack(PSNET_SOCKET_RELIABLE socket, ushort sig)
{
	ubyte data[MAX_PACKET_SIZE],code;	
	int packet_size = 0;

	// build the header and add 
	BUILD_HEADER(XFER_PACKET);	

	// add the opcode
	code = MULTI_XFER_CODE_ACK;
	ADD_DATA(code);

	// add the sig
	ADD_USHORT(sig);
	
	// send the data	
	psnet_rel_send(socket, data, packet_size);
}

// send a nak to the sender
void multi_xfer_send_nak(PSNET_SOCKET_RELIABLE socket, ushort sig)
{
	ubyte data[MAX_PACKET_SIZE],code;	
	int packet_size = 0;

	// build the header and add the code
	BUILD_HEADER(XFER_PACKET);	

	// add the opcode
	code = MULTI_XFER_CODE_NAK;
	ADD_DATA(code);

	// add the sig
	ADD_USHORT(sig);

	// send the data	
	psnet_rel_send(socket, data, packet_size);
}

// send a "final" packet
void multi_xfer_send_final(xfer_entry *xe)
{
	ubyte data[MAX_PACKET_SIZE],code;	
	int packet_size = 0;

	// build the header
	BUILD_HEADER(XFER_PACKET);	

	// add the code
	code = MULTI_XFER_CODE_FINAL;
	ADD_DATA(code);

	// add the sig
	ADD_USHORT(xe->sig);

	// send the data	
	psnet_rel_send(xe->file_socket, data, packet_size);
}

// send the header to begin a file transfer
void multi_xfer_send_header(xfer_entry *xe)
{
	ubyte data[MAX_PACKET_SIZE],code;	
	int packet_size = 0;

	// build the header and add the opcode
	BUILD_HEADER(XFER_PACKET);	
	code = MULTI_XFER_CODE_HEADER;
	ADD_DATA(code);

	// add the sig
	ADD_USHORT(xe->sig);

	// add the filename
	ADD_STRING(xe->filename);
		
	// add the id #
	ADD_INT(xe->file_size);

	// add the file checksum
	ADD_USHORT(xe->file_chksum);

	// send the packet	
	psnet_rel_send(xe->file_socket, data, packet_size);
}

// convert the filename into the prefixed ex_filename
void multi_xfer_conv_prefix(char *filename,char *ex_filename)
{
	char temp[MAX_FILENAME_LEN+50];
	
	// blast the memory clean
	memset(temp, 0, MAX_FILENAME_LEN+50);

	// copy in the prefix
	strcpy_s(temp, MULTI_XFER_FNAME_PREFIX);

	// stick on the original name
	strcat_s(temp, filename);

	// copy the whole thing to the outgoing filename
	strcpy(ex_filename, temp);
}

// get a new xfer sig
ushort multi_xfer_get_sig()
{
	ushort ret = Multi_xfer_sig;

	// new one
	if(Multi_xfer_sig == 0xffff){
		Multi_xfer_sig = 0;
	} else {
		Multi_xfer_sig++;
	}

	return ret;
}
