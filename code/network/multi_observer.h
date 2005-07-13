/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_observer.h $
 * $Revision: 2.2 $
 * $Date: 2005-07-13 03:25:59 $
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
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 3     4/18/98 5:00p Dave
 * Put in observer zoom key. Made mission sync screen more informative.
 * 
 * 2     3/13/98 2:51p Dave
 * Put in support for observers to join ingame.
 * 
 * 1     3/12/98 5:44p Dave
 *  
 * $NoKeywords: $
 */

#ifndef _MULTI_OBSERVER_HEADER_FILE
#define _MULTI_OBSERVER_HEADER_FILE

// ---------------------------------------------------------------------------------------
// MULTI OBSERVER DEFINES/VARS
//

struct net_addr;
struct player;
struct net_player; 

// ---------------------------------------------------------------------------------------
// MULTI OBSERVER FUNCTIONS
//

// create a _permanent_ observer player 
int multi_obs_create_player(int player_num,char *name,net_addr *addr,player *pl);

// create an explicit observer object and assign it to the passed player
void multi_obs_create_observer(net_player *pl);

// create observer object locally, and additionally, setup some other information
// ( client-side equivalent of multi_obs_create_observer() )
void multi_obs_create_observer_client();

// create objects for all known observers in the game at level start
// call this before entering a mission
// this implies for the local player in the case of a client or for _all_ players in the case of a server
void multi_obs_level_init();

// if i'm an observer, zoom to near my targted object (if any)
void multi_obs_zoom_to_target();

#endif
