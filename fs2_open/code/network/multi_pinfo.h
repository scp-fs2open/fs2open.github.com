/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_pinfo.h $
 * $Revision: 2.2 $
 * $Date: 2005-07-13 03:35:32 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:29  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *  
 * 
 * 3     1/30/99 1:29a Dave
 * Fixed nebula thumbnail problem. Full support for 1024x768 choose pilot
 * screen.  Fixed beam weapon death messages.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 2     4/20/98 6:04p Dave
 * Implement multidata cache flushing and xferring mission files to
 * multidata. Make sure observers can't change hud config. Fix pilot image
 * viewing in popup. Put in game status field. Tweaked multi options. 
 * 
 * 1     3/05/98 8:20p Dave
 * 
 * $NoKeywords: $
 */

#ifndef _MULTI_PLAYER_INFO_HEADER_FILE
#define _MULTI_PLAYER_INFO_HEADER_FILE

// ---------------------------------------------------------------------------------------
// MULTI PLAYER INFO DEFINES/VARS
//

// prototypes
struct net_player;


// ---------------------------------------------------------------------------------------
// MULTI PLAYER INFO FUNCTIONS
//

// fire up the player info popup
void multi_pinfo_popup(net_player *np);

// is the pilot info popup currently active?
int multi_pinfo_popup_active();

// kill the currently active popup (if any)
void multi_pinfo_popup_kill();

// notify the popup that a player has left
void multi_pinfo_notify_drop(net_player *np);

#endif
