/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_kick.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:26 $
 * $Author: penguin $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *  
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 3     9/11/98 4:14p Dave
 * Fixed file checksumming of < file_size. Put in more verbose kicking and
 * PXO stats store reporting.
 * 
 * 2     3/24/98 5:00p Dave
 * Fixed several ui bugs. Put in pre and post voice stream playback sound
 * fx. Put in error specific popups for clients getting dropped from games
 * through actions other than their own.
 * 
 * 1     2/12/98 4:38p Dave
 * Multiplayer kick functionality
 * 
 * $NoKeywords: $
 */
#ifndef _MULTIPLAYER_KICK_HEADER_FILE
#define _MULTIPLAYER_KICK_HEADER_FILE

// ----------------------------------------------------------------------------------
// KICK DEFINES/VARS
//

struct net_addr;
struct net_player;

// special reasons for kicking players
#define KICK_REASON_NORM				0				// plain old kick
#define KICK_REASON_BAD_XFER			1				// error xferring mission
#define KICK_REASON_CANT_XFER			2				// can't xfer a builtin mission
#define KICK_REASON_INGAME_ENDED		3				// kicked while ingame joining a mission about to end


// ----------------------------------------------------------------------------------
// KICK FUNCTIONS
//

// initialize all kicking details (ban lists, etc). it is safe to call this function at any time
void multi_kick_init();

// process all kick details (disconnecting players who have been kicked but haven't closed their socket)
void multi_kick_process();

// attempt to kick a player. return success or fail
void multi_kick_player(int player_index, int ban = 1, int reason = KICK_REASON_NORM);

// is this net address currently kicked and banded
int multi_kick_is_banned(net_addr *addr);

// debug console function called to determine which player to kick
void multi_dcf_kick();

// fill in the passed string with the appropriate "kicked" string
void multi_kick_get_text(net_player *pl, int reason, char *str);

#endif
