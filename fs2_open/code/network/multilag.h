/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multilag.h $
 * $Revision: 2.4 $
 * $Date: 2005-07-13 03:35:33 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2005/01/31 10:34:38  taylor
 * merge with Linux/OSX tree - p0131
 *
 * Revision 2.2  2004/08/11 05:06:29  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2004/03/05 09:02:02  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/07 03:02:07  mharris
 * #ifndef WINDOWS should be #ifdef
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     6/07/99 9:51p Dave
 * Consolidated all multiplayer ports into one.
 * 
 * 4     11/19/98 8:03a Dave
 * Full support for D3-style reliable sockets. Revamped packet lag/loss
 * system, made it receiver side and at the lowest possible level.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 12    4/27/98 6:02p Dave
 * Modify how missile scoring works. Fixed a team select ui bug. Speed up
 * multi_lag system. Put in new main hall.
 * 
 * 11    4/04/98 8:42p Dave
 * Tested and debugged UDP reliable socket layer. Modified lag system to
 * take this into account. 
 * 
 * 10    4/01/98 11:19p Dave
 * Put in auto-loading of xferred pilot pic files. Grey out background
 * behind pinfo popup. Put a chatbox message in when players are kicked.
 * Moved mission title down in briefing. Other ui fixes.
 * 
 * 9     3/14/98 2:48p Dave
 * Cleaned up observer joining code. Put in support for file xfers to
 * ingame joiners (observers or not). Revamped and reinstalled pseudo
 * lag/loss system.
 * 
 * 8     1/15/98 6:12p Dave
 * Fixed weapons loadout bugs with multiplayer respawning. Added
 * multiplayer start screen. Fixed a few chatbox bugs.
 * 
 * 7     1/11/98 10:03p Allender
 * removed <winsock.h> from headers which included it.  Made psnet_socket
 * type which is defined just as SOCKET type is.
 * 
 * 6     12/29/97 5:21p Dave
 * Put in object update sequencing for multiplayer.
 * 
 * 5     12/16/97 6:17p Dave
 * Put in primary weapon support for multiplayer weapon select screen.
 * 
 * 4     12/10/97 4:46p Dave
 * Added in more detailed support for multiplayer packet lag/loss. Fixed
 * some multiplayer stuff. Added some controls to the standalone.
 * 
 * 3     12/01/97 4:59p Dave
 * Synchronized multiplayer debris objects. Put in pilot popup in main
 * hall. Optimized simulated multiplayer lag module. Fixed a potential
 * file_xfer bug.
 * 
 * 2     11/28/97 5:06p Dave
 * Put in facilities for simulating multiplayer lag.
 * 
 * 1     11/28/97 4:38p Dave
 * Initial Revision
 * 
 * $NoKeywords: $
 */

#ifndef _MULTI_LAG_HEADER_FILE
#define _MULTI_LAG_HEADER_FILE

#ifndef NDEBUG
	// #define MULTI_USE_LAG								
#endif

#include "globalincs/pstypes.h"

#ifdef _WIN32
struct fd_set;
#endif
struct timeval;

// initialize multiplayer lagloss. in non-debug situations, this call does nothing
void multi_lag_init();

// shutdown multiplayer lag
void multi_lag_close();

// select for multi_lag
int multi_lag_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *except_fds, const timeval *timeout);

// recvfrom for multilag
int multi_lag_recvfrom(uint s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen);

#endif
