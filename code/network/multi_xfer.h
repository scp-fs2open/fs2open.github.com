/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_xfer.h $
 * $Revision: 2.2 $
 * $Date: 2004-08-11 05:06:29 $
 * $Author: Kazan $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/03/05 09:02:02  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     12/14/98 4:01p Dave
 * Got multi_data stuff working well with new xfer stuff. 
 * 
 * 4     12/14/98 12:13p Dave
 * Spiffed up xfer system a bit. Put in support for squad logo file xfer.
 * Need to test now.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 21    5/21/98 3:45a Sandeep
 * Make sure file xfer sender side uses correct directory type.
 * 
 * 20    4/23/98 6:18p Dave
 * Store ETS values between respawns. Put kick feature in the text
 * messaging system. Fixed text messaging system so that it doesn't
 * process or trigger ship controls. Other UI fixes.
 * 
 * 19    4/01/98 11:19p Dave
 * Put in auto-loading of xferred pilot pic files. Grey out background
 * behind pinfo popup. Put a chatbox message in when players are kicked.
 * Moved mission title down in briefing. Other ui fixes.
 * 
 * 18    3/23/98 5:42p Dave
 * Put in automatic xfer of pilot pic files. Changed multi_xfer system so
 * that it can support multiplayer sends/received between client and
 * server simultaneously.
 * 
 * 17    3/21/98 7:14p Dave
 * Fixed up standalone player slot switching. Made training missions not
 * count towards player stats.
 * 
 * 16    2/22/98 2:53p Dave
 * Put in groundwork for advanced multiplayer campaign  options.
 * 
 * 15    2/20/98 4:43p Dave
 * Finished support for multiplayer player data files. Split off
 * multiplayer campaign functionality.
 * 
 * 14    2/19/98 6:26p Dave
 * Fixed a few file xfer bugs. Tweaked mp team select screen. Put in
 * initial support for player data uploading.
 * 
 * 13    2/18/98 10:21p Dave
 * Ripped out old file xfer system. Put in brand new xfer system.
 * 
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
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
