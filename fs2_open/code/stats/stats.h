/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Stats/Stats.h $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:10 $
 * $Author: penguin $
 *
 * module for running the stats screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 13    5/18/98 5:25p Chad
 * Removed old-ass stats display code.
 * 
 * 12    1/06/98 5:08p Dave
 * Put in stats display change to show alltime+mission stats during
 * debriefing. Fixed a object update sequencing bug.
 * 
 * 11    10/24/97 6:19p Dave
 * More standalone testing/fixing. Added reliable endgame sequencing.
 * Added reliable ingame joining. Added reliable stats transfer (endgame).
 * Added support for dropping players in debriefing. Removed a lot of old
 * unused code.
 * 
 * 10    10/23/97 7:44p Hoffoss
 * Added 2 defines to replace hard-coded values in the code.
 * 
 * 9     10/08/97 4:47p Dave
 * Fixed bugs turned up in testing. Added some brief comments.
 * 
 * 8     9/30/97 8:48p Lawrance
 * generalize some functions for displaying stats info
 * 
 * 7     9/09/97 4:31p Dave
 * Put in multiplayer post-game stats stuff.
 * 
 * 6     8/15/97 2:20p Allender
 * fix stats code to properly get connected players
 * 
 * 5     8/15/97 9:28a Dave
 * Fixed minor bug in multiplayer stats init.
 * 
 * 4     8/14/97 5:20p Dave
 * Made initialization/clearing of players stats more thorough.
 * 
 * 3     7/25/97 4:33p Dave
 * Spiffed up statistics screen considerably. Currently reports all
 * statistics in the game (except mission/campaign related scores)
 * 
 * 2     7/24/97 2:31p Dave
 * Added basic screen, no interaction yet.
 * 
 * 1     7/24/97 1:50p Dave
 * 
 * $NoKeywords: $
 */

#ifndef _FS_STATISTICS_STATE_HEADER
#define _FS_STATISTICS_STATE_HEADER

#define MISSION_STATS	0
#define ALL_TIME_STATS	1

#include "stats/scoring.h"

void show_stats_init();
void show_stats_close();
void set_player_stats(int pid);
void init_multiplayer_stats( void );  // initializes all mission specific stats to be 0

void show_stats_numbers(int stage, int sx, int sy, int dy=10,int add_mission = 0);
void show_stats_label(int stage, int sx, int sy, int dy=10);

#endif
