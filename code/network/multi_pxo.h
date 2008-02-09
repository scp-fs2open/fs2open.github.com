/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_pxo.h $
 * $Revision: 1.1.2.1 $
 * $Date: 2007-10-15 06:43:17 $
 * $Author: taylor $
 *
 * $Log: not supported by cvs2svn $
 * 
 * 5     11/02/99 2:32p Jefff
 * updated some URLs
 * 
 * 4     10/06/99 10:31a Jefff
 * url open fuction available to all
 * 
 * 3     4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 9     7/08/98 4:54p Dave
 * Join last used channel when returning from the games list.
 * 
 * 8     5/23/98 3:02a Dave
 * Pxo tweaks.
 * 
 * 7     5/21/98 9:45p Dave
 * Lengthened tracker polling times. Put in initial support for PXO
 * servers with channel filters. Fixed several small UI bugs.
 * 
 * 6     5/19/98 8:35p Dave
 * Revamp PXO channel listing system. Send campaign goals/events to
 * clients for evaluation. Made lock button pressable on all screens. 
 * 
 * 5     5/19/98 1:35a Dave
 * Tweaked pxo interface. Added rankings url to pxo.cfg. Make netplayer
 * local options update dynamically in netgames.
 * 
 * 4     5/18/98 9:15p Dave
 * Put in network config file support.
 * 
 * 3     5/15/98 12:09a Dave
 * New tracker api code. New game tracker code. Finished up first run of
 * the PXO screen. Fixed a few game server list exceptions.
 * 
 * 2     5/12/98 2:46a Dave
 * Rudimentary communication between Parallax Online and freespace. Can
 * get and store channel lists.
 * 
 * 1     5/11/98 11:47p Dave
 *  
 * 
 * $NoKeywords: $
 */

#ifndef _PARALLAX_ONLINE_HEADER_FILE
#define _PARALLAX_ONLINE_HEADER_FILE

// ----------------------------------------------------------------------------------------------------
// PXO DEFINES/VARS
//

// default url for PXO rankings
//#define MULTI_PXO_RANKINGS_URL				"http://www.volition-inc.com"
#define MULTI_PXO_RANKINGS_URL				"http://www.pxo.net/rankings/fs2full.cfm"


// default url for PXO account creation
//#define MULTI_PXO_CREATE_URL				"http://www.parallaxonline.com/register.html"
#define MULTI_PXO_CREATE_URL					"http://www.pxo.net/newaccount.cfm"

// default url for PXO account verification
//#define MULTI_PXO_VERIFY_URL				"http://www.parallaxonline.com/verify.html"
#define MULTI_PXO_VERIFY_URL					"http://www.pxo.net/verify.cfm"

// default url for PXO banners
#define MULTI_PXO_BANNER_URL					"http://www.pxo.net/files/banners"

// tracker and PXO addresses
#define MULTI_PXO_USER_TRACKER_IP		"ut.pxo.net"
#define MULTI_PXO_GAME_TRACKER_IP		"gt.pxo.com"
#define MULTI_PXO_CHAT_IP					"chat.pxo.net"

// ----------------------------------------------------------------------------------------------------
// PXO FUNCTIONS
//

// initialize the PXO screen
void multi_pxo_init(int use_last_channel);

// do frame for the PXO screen
void multi_pxo_do();

// close the PXO screen
void multi_pxo_close();


// initialize the PXO help screen
void multi_pxo_help_init();

// do frame for PXO help
void multi_pxo_help_do();

// close the pxo screen
void multi_pxo_help_close();

// open up a URL
void multi_pxo_url(char *url);

// called from the game tracker API - server count update for a channel
void multi_pxo_channel_count_update(char *name,int count);

#endif
