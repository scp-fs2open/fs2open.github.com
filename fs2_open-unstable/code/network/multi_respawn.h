/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_respawn.h $
 * $Revision: 2.3 $
 * $Date: 2005-07-13 03:35:33 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
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
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 4     8/06/99 2:44a Dave
 * Make sure dead players who leave respawn AI.
 * 
 * 3     3/01/99 7:39p Dave
 * Added prioritizing ship respawns. Also fixed respawns in TvT so teams
 * don't mix respawn points.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 3     4/23/98 1:49a Allender
 * major rearm/repair fixes for multiplayer.  Fixed respawning of AI ships
 * to not respawn until 5 seconds after they die.  Send escort information
 * to ingame joiners
 * 
 * 2     3/11/98 10:22p Dave
 * Laid groundwork for new observer HUD. Split up multi respawning into
 * its own module.
 * 
 * 1     3/11/98 5:07p Dave
 *  
 * $NoKeywords: $
 */

#ifndef _MULTIPLAYER_RESPAWN_HEADER_FILE
#define _MULTIPLAYER_RESPAWN_HEADER_FILE

// ---------------------------------------------------------------------------------------
// MULTI RESPAWN DEFINES/VARS
//
#include "globalincs/pstypes.h"

struct object;
struct header;
struct net_player;


// ---------------------------------------------------------------------------------------
// MULTI RESPAWN FUNCTIONS
//

// check to see if a net player needs to be respawned
void multi_respawn_check(object *objp);

// respawn normally
void multi_respawn_normal();

// respawn as an observer
void multi_respawn_observer();

// server should check to see if any respawned players have run out of their invulnerability
void multi_respawn_handle_invul_players();

// build a list of base respawn points on the server, for this level
void multi_respawn_build_points();

void multi_respawn_init();
void multi_respawn_check_ai();

// notify of a player leaving
void multi_respawn_player_leave(net_player *pl);

// ---------------------------------------------------------------------------------------
// MULTI RESPAWN PACKET HANDLERS
//

void multi_respawn_process_packet(ubyte *data, header *hinfo);

#endif
