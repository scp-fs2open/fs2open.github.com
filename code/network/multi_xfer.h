/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FREESPACE_FILE_TRANSFER_HEADER
#define _FREESPACE_FILE_TRANSFER_HEADER

// ------------------------------------------------------------------------------------------
// MULTI XFER DEFINES/VARS
//
#include "globalincs/pstypes.h"

typedef uint PSNET_SOCKET_RELIABLE;

// status codes for transfers
#define MULTI_XFER_NONE						-1							// nothing is happening - this is an invalid handle
#define MULTI_XFER_SUCCESS					0							// the xfer has successfully transferred
#define MULTI_XFER_FAIL						1							// the xfer has failed for one reason or another
#define MULTI_XFER_UNKNOWN					2							// the xfer has finished but its unknown if it was successful - wait a while longer
#define MULTI_XFER_TIMEDOUT				3							// the xfer has timed-out during some stage of the process
#define MULTI_XFER_IN_PROGRESS			4							// the xfer is in progress
#define MULTI_XFER_QUEUED					5							// queued up - hasn't started yet

#define MULTI_XFER_FLAG_AUTODESTROY		(1<<15)					// automatically clear and free an xfer handle that is done
#define MULTI_XFER_FLAG_REJECT			(1<<16)					// set by the receive callback function if we want to disallow xfer of this file
// if this flag is set, the system will only xfer one file at a time to a given destination. 
// so, suppose you start sending 3 files to one target, all which have this flag set. Only the first file will send.
// Once it is complete, the second one will go. Then the third. This is extremely useful for files where you don't 
// _really_ care if it arrives or not (eg - sending multiple pilot pics or sounds or squad logos, etc). If you _do_
// care about the file (eg - mission files), you probably shouldn't be using this flag
#define MULTI_XFER_FLAG_QUEUE				(1<<17)					

// the xfer system is guaranteed never to spew data larger than this
#define MULTI_XFER_MAX_SIZE				500

// ------------------------------------------------------------------------------------------
// MULTI XFER FUNCTIONS
//

// initialize all file xfer transaction stuff, call in multi_level_init()
void multi_xfer_init(void (*multi_xfer_recv_callback)(int handle));

// do frame for all file xfers, call in multi_do_frame()
void multi_xfer_do();

// close down the file xfer system
void multi_xfer_close();

// reset the xfer system, including shutting down/killing all active xfers
void multi_xfer_reset();

// send a file to the specified player, return a handle
int multi_xfer_send_file(PSNET_SOCKET_RELIABLE who, char *filename, int cfile_flags, int flags = 0);

// get the status of the current file xfer
int multi_xfer_get_status(int handle);

// abort a transferring file
void multi_xfer_abort(int handle);

// release an xfer handle
void multi_xfer_release_handle(int handle);

// get the filename of the xfer for the given handle
char *multi_xfer_get_filename(int handle);

// lock the xfer system (don't accept incoming files, don't allow outgoing files)
void multi_xfer_lock();

// unlock the xfer system
void multi_xfer_unlock();

// force all receives to go into the specified directory by cfile type
void multi_xfer_force_dir(int cf_type);

// forces the given xfer entry to the specified directory type (only valid when called from the recv_callback function)
void multi_xfer_handle_force_dir(int handle,int cf_type);

// xor the flag on a given entry
void multi_xfer_xor_flags(int handle,int flags);

// get the flags for a given entry
int multi_xfer_get_flags(int handle);

// if the passed filename is being xferred, return the xfer handle, otherwise return -1
int multi_xfer_lookup(char *filename);

// get the % of completion of the passed file handle, return < 0 if invalid
float multi_xfer_pct_complete(int handle);

// get the socket of the file xfer (useful for identifying players)
uint multi_xfer_get_sock(int handle);

// get the CF_TYPE of the directory this file is going to
int multi_xfer_get_force_dir(int handle);

// ------------------------------------------------------------------------------------------
// MULTI XFER PACKET HANDLERS
//

// process an incoming file xfer data packet, return bytes processed, guaranteed to process the entire
// packet regardless of error conditions
int multi_xfer_process_packet(unsigned char *data, PSNET_SOCKET_RELIABLE who);

#endif
