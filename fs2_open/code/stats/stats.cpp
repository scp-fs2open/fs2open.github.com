/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Stats/Stats.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-07-29 20:12:31 $
 * $Author: penguin $
 *
 * module for running the stats screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/13 21:09:29  mharris
 * I think the last of the networking code has ifndef NO_NETWORK...
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     10/15/99 3:56p Jefff
 * lifetime score to alltime stats printout (multi debrief screen)
 * 
 * 4     10/15/99 3:01p Jefff
 * mission score shows up in multiplayer debrief
 * 
 * 3     3/19/99 9:51a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 3     3/15/99 10:30a Neilk
 * Add hires support
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 38    6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 37    5/18/98 5:25p Chad
 * Removed old-ass stats display code.
 * 
 * 36    5/18/98 12:52a Allender
 * make initialization of multiplayer stats initialize all players, not
 * just connected players (fixed ingame join stats problems)
 * 
 * 35    5/15/98 4:12p Allender
 * removed redbook code.  Put back in ingame join timer.  Major fixups for
 * stats in multiplayer.  Pass correct score, medals, etc when leaving
 * game.  Be sure clients display medals, badges, etc.
 * 
 * 34    5/07/98 12:57a Dave
 * Fixed incorrect calls to free() from stats code. Put in new artwork for
 * debrief and host options screens. Another modification to scoring
 * system for secondary weapons.
 * 
 * 33    4/21/98 11:56p Dave
 * Put in player deaths statskeeping. Use arrow keys in the ingame join
 * ship select screen. Don't quit the game if in the debriefing and server
 * leaves.
 * 
 * 32    4/09/98 10:31a Dave
 * Don't display player deaths statistic under single player (doesn't make
 * sense).
 * 
 * 31    3/31/98 5:18p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 30    3/27/98 3:59p Hoffoss
 * Due to everyone using different assumptions about what a length define
 * means, changed code to account for safest assumption.
 * 
 * 29    3/17/98 12:29a Dave
 * Put in hud support for rtvoice. Several ui interface changes.
 * 
 * 28    3/15/98 4:17p Dave
 * Fixed oberver hud problems. Put in handy netplayer macros. Reduced size
 * of network orientation matrices.
 * 
 * 27    2/22/98 2:48p John
 * More String Externalization Classification
 * 
 * 26    2/12/98 5:00p Lawrance
 * underline kills in the stats section
 * 
 * 25    1/06/98 5:08p Dave
 * Put in stats display change to show alltime+mission stats during
 * debriefing. Fixed a object update sequencing bug.
 * 
 * 24    12/10/97 4:45p Dave
 * Added in more detailed support for multiplayer packet lag/loss. Fixed
 * some multiplayer stuff. Added some controls to the standalone.
 * 
 * 23    11/06/97 9:53a Dave
 * 
 * 22    10/25/97 7:23p Dave
 * Moved back to single set stats storing. Put in better respawning
 * locations system.
 * 
 * 21    10/24/97 6:19p Dave
 * More standalone testing/fixing. Added reliable endgame sequencing.
 * Added reliable ingame joining. Added reliable stats transfer (endgame).
 * Added support for dropping players in debriefing. Removed a lot of old
 * unused code.
 * 
 * 20    10/23/97 7:44p Hoffoss
 * Added 2 defines to replace hard-coded values in the code.
 * 
 * 19    10/21/97 5:21p Dave
 * Fixed pregame mission load/file verify debacle. Added single vs.
 * multiplayer stats system.
 * 
 * 18    10/14/97 5:37p Dave
 * Fixed a memory allocation bug
 * 
 * 17    10/08/97 4:47p Dave
 * Fixed bugs turned up in testing. Added some brief comments.
 * 
 * 16    9/30/97 8:48p Lawrance
 * generalize some functions for displaying stats info
 * 
 * 15    9/24/97 5:03p Dave
 * Beginning changes on how kills/assists are evaluated in both single and
 * multiplayer
 * 
 * 14    9/18/97 10:14p Dave
 * Updated the scoring system struct. Added rank.tbl. Modified how medal
 * names are displayed.
 * 
 * 13    9/17/97 5:12p John
 * Restructured collision routines.  Probably broke a lot of stuff.
 * 
 * 12    9/09/97 4:31p Dave
 * Put in multiplayer post-game stats stuff.
 * 
 * 11    8/26/97 4:59p Dave
 * Changed display from bonehead hits to friendly hits.
 * 
 * 10    8/23/97 11:31a Dave
 * Changed the definition of bonehead hit %
 * 
 * 9     8/20/97 12:31p Dave
 * Added a death count for auto ressurected players.
 * Made it easier to reorganize the stats page
 * 
 * 8     8/19/97 5:14p Dave
 * Added a sucker check. :)
 * 
 * 7     8/15/97 2:20p Allender
 * fix stats code to properly get connected players
 * 
 * 6     8/15/97 9:28a Dave
 * Fixed minor bug in multiplayer stats init.
 * 
 * 5     8/14/97 5:20p Dave
 * Made initialization/clearing of players stats more thorough.
 * 
 * 4     7/25/97 5:23p Dave
 * Made text displayed in antialiased font.
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

#ifdef _WIN32
#include <io.h>
#include <winsock.h>
#endif

#include "freespace.h"
#include "gamesequence.h"
#include "key.h"
#include "2d.h"
#include "ui.h"
#include "timer.h"
#include "player.h"
#include "stats.h"
#include "hud.h"
#include "font.h"

#ifndef NO_NETWORK
#include "multi.h"
#endif

#define MISSION_STATS_START_Y 80
#define ALLTIME_STATS_START_Y 270
#define MULTIPLAYER_LIST_START 20

static int Mission_stats_start_y[GR_NUM_RESOLUTIONS] = {
	80,	// GR_640
	80		// GR_1024
};

static int Alltime_stats_start_y[GR_NUM_RESOLUTIONS] = {
	270,	// GR_640
	270	// GR_1024
};

static int Multiplayer_list_start[GR_NUM_RESOLUTIONS] = {
	20,	// GR_640
	20		// GR_1024
};

// static UI_WINDOW Player_stats_window;

player *Active_player;

void show_stats_init()
{
	// Player_stats_window.create( 0, 0, gr_screen.max_w, gr_screen.max_h, 0 );

#ifndef NO_NETWORK
	if (Game_mode & GM_MULTIPLAYER) {				
		set_player_stats(MY_NET_PLAYER_NUM);
	}
	else
#endif
	{
		Active_player = Player;		
	}	
}

// write out the label for each stat
void show_stats_label(int stage, int sx, int sy, int dy)
{
	switch ( stage ) {
		case MISSION_STATS:
			gr_printf(sx,sy,XSTR( "Mission Stats", 114));
			sy += 2*dy;
			gr_printf(sx,sy,XSTR( "Total kills", 115));
			sy += 2*dy;
			gr_printf(sx,sy,XSTR( "Primary weapon shots", 116));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Primary weapon hits", 117));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Primary friendly hits", 118));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Primary hit %%", 119));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Primary friendly hit %%", 120));
			sy += 2*dy;

			gr_printf(sx,sy,XSTR( "Secondary weapon shots", 121));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Secondary weapon hits", 122));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Secondary friendly hits", 123));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Secondary hit %%", 124));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Secondary friendly hit %%", 125));
			sy += 2*dy;

			gr_printf(sx,sy,XSTR( "Assists", 126));
			sy += 2*dy;

			if(Game_mode & GM_MULTIPLAYER){
				gr_printf(sx,sy,XSTR( "Player Deaths", 127));
				sy += 2*dy;

				gr_printf(sx,sy,XSTR( "Mission score", 1526));
			}


			break;

		case ALL_TIME_STATS:
			gr_printf(sx,sy,XSTR( "All Time Stats", 128));
			sy += 2*dy;			
			gr_printf(sx,sy,XSTR( "Total kills", 115));
			sy += 2*dy;
			gr_printf(sx,sy,XSTR( "Primary weapon shots", 116));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Primary weapon hits", 117));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Primary friendly hits", 118));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Primary hit %%", 119));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Primary friendly hit %%", 120));
			sy += 2*dy;

			gr_printf(sx,sy,XSTR( "Secondary weapon shots", 121));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Secondary weapon hits", 122));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Secondary friendly hits", 123));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Secondary hit %%", 124));
			sy += dy;
			gr_printf(sx,sy,XSTR( "Secondary friendly hit %%", 125));
			sy += 2*dy;			

			gr_printf(sx,sy,XSTR( "Assists", 126));
			sy += 2*dy;

			if(Game_mode & GM_MULTIPLAYER){
				gr_printf(sx,sy,XSTR( "Score", 1527));
			}
			break;
		} // end switch
}

void stats_underline_text(int sx, int sy, char *text)
{
	int w,h,fh;

	gr_get_string_size(&w,&h,text);
	fh=gr_get_font_height();
	gr_line(sx-1, sy+fh, sx+w+1, sy+fh);
}

void show_stats_numbers(int stage, int sx, int sy, int dy,int add_mission)
{
   char		text[30];
	float		pct;

	sy += 2*dy;
	switch ( stage ) {
		case MISSION_STATS:
         // mission kills stats
			sprintf(text,"%d",Active_player->stats.m_kill_count_ok);
			gr_printf(sx,sy,text);
			// stats_underline_text(sx,sy,text);
			sy += 2*dy;
         // mission primary weapon stats
			sprintf(text,"%d",Active_player->stats.mp_shots_fired);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%d",Active_player->stats.mp_shots_hit);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%d",Active_player->stats.mp_bonehead_hits);
			gr_printf(sx,sy,text);
			sy += dy;
			if(Active_player->stats.mp_shots_fired>0)
				pct=(float)100.0*((float)Active_player->stats.mp_shots_hit/(float)Active_player->stats.mp_shots_fired);
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat(text," %%");
			gr_printf(sx,sy,text);
			sy += dy;
			if(Active_player->stats.mp_bonehead_hits>0)
				pct=(float)100.0*((float)Active_player->stats.mp_bonehead_hits/(float)Active_player->stats.mp_shots_fired);
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat(text," %%");
			gr_printf(sx,sy,text);
			sy += 2*dy;

			// mission secondary weapon stats
			sprintf(text,"%d",Active_player->stats.ms_shots_fired);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%d",Active_player->stats.ms_shots_hit);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%d",Active_player->stats.ms_bonehead_hits);
			gr_printf(sx,sy,text);
			sy += dy;
			if(Active_player->stats.ms_shots_fired>0)
				pct=(float)100.0*((float)Active_player->stats.ms_shots_hit/(float)Active_player->stats.ms_shots_fired);
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat(text," %%");
			gr_printf(sx,sy,text);
			sy += dy;
			if(Active_player->stats.ms_bonehead_hits>0)
				pct=(float)100.0*((float)Active_player->stats.ms_bonehead_hits/(float)Active_player->stats.ms_shots_fired);
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat(text," %%");
			gr_printf(sx,sy,text);
			sy += 2*dy;

			// mission assists and player rescues (respawns)
			sprintf(text,"%d",(int)Active_player->stats.m_assists);
			gr_printf(sx,sy,text);
			sy += 2*dy;

			if(Game_mode & GM_MULTIPLAYER){
				sprintf(text,"%d",(int)Active_player->stats.m_player_deaths);
				gr_printf(sx,sy,text);
				sy += 2*dy;

				// mission score
				gr_printf(sx, sy, "%d", (int)Active_player->stats.m_score);
			}


			break;

		case ALL_TIME_STATS:
			 scoring_struct add;
			
			// if we are passed mission_add (the stats for the current mission), copy it to "add", otherwise,
			// zero it out
			memset(&add,0,sizeof(scoring_struct));				
			if(add_mission){
				add.kill_count_ok = Active_player->stats.m_kill_count_ok;
				add.p_shots_fired  = Active_player->stats.mp_shots_fired;
				add.p_shots_hit = Active_player->stats.mp_shots_hit;
				add.p_bonehead_hits = Active_player->stats.mp_bonehead_hits;				
				
				add.s_shots_fired = Active_player->stats.ms_shots_fired;
				add.s_shots_hit = Active_player->stats.ms_shots_hit;
				add.s_bonehead_hits = Active_player->stats.ms_bonehead_hits;				
			}			

         // mission kills stats
			sprintf(text,"%d",Active_player->stats.kill_count_ok + add.kill_count_ok);
			hud_num_make_mono(text);
			gr_printf(sx,sy,text);
			// stats_underline_text(sx,sy,text);
			sy += 2*dy;
         // alltime primary weapon stats
			sprintf(text,"%d",Active_player->stats.p_shots_fired + add.p_shots_fired);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%d",Active_player->stats.p_shots_hit + add.p_shots_hit);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%d",Active_player->stats.p_bonehead_hits + add.p_bonehead_hits);
			gr_printf(sx,sy,text);
			sy += dy;
			if((Active_player->stats.p_shots_fired + add.p_shots_fired)>0)
				pct=(float)100.0*((float)(Active_player->stats.p_shots_hit+add.p_shots_hit)/(float)(Active_player->stats.p_shots_fired + add.p_shots_fired));
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat(text," %%");
			gr_printf(sx,sy,text);
			sy += dy;
			if((Active_player->stats.p_bonehead_hits + add.p_bonehead_hits)>0)
				pct=(float)100.0*((float)(Active_player->stats.p_bonehead_hits+add.p_bonehead_hits)/(float)(Active_player->stats.p_shots_fired + add.p_shots_fired));
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat(text," %%");
			gr_printf(sx,sy,text);
			sy += 2*dy;

			// alltime secondary weapon stats
			sprintf(text,"%d",Active_player->stats.s_shots_fired + add.s_shots_fired);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%d",Active_player->stats.s_shots_hit + add.s_shots_hit);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%d",Active_player->stats.s_bonehead_hits + add.s_bonehead_hits);
			gr_printf(sx,sy,text);
			sy += dy;
			if((Active_player->stats.s_shots_fired+add.s_shots_fired)>0)
				pct=(float)100.0*((float)(Active_player->stats.s_shots_hit + add.s_shots_hit)/(float)(Active_player->stats.s_shots_fired + add.s_shots_fired));
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat(text," %%");
			gr_printf(sx,sy,text);
			sy += dy;
			if((Active_player->stats.s_bonehead_hits + add.s_bonehead_hits)>0)
				pct=(float)100.0*((float)(Active_player->stats.s_bonehead_hits+add.s_bonehead_hits)/(float)(Active_player->stats.s_shots_fired+add.s_shots_fired));
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat(text," %%");
			gr_printf(sx,sy,text);
			sy += 2*dy;

			// alltime assists
			sprintf(text,"%d",(int)Active_player->stats.assists + add.assists);
			gr_printf(sx,sy,text);
			sy += 2*dy;

			if (Game_mode & GM_MULTIPLAYER) {
				gr_printf(sx, sy, "%d", (int)Active_player->stats.score);
			}
			break;
	} // end switch
}

#ifndef NO_NETWORK
int find_netplayer_n(int n)
{
	int idx;
	int target;
   target = n;
	n=0;
   for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx])){
			n++;
			if(n == target)
				return idx;
		}
	}
	return -1;
}
#endif


void show_stats_close()
{	
}
 
#ifndef NO_NETWORK
// initialize the statistics portion of the player structure for multiplayer.  Only the host of
// a netgame needs to be doing this (and if fact, only he *should* be doing this)
void init_multiplayer_stats( )
{
	scoring_struct *ptr;

	for (int idx=0; idx < MAX_PLAYERS; idx++) {
		ptr = &Players[idx].stats;
		scoring_level_init( ptr );
	}
}

void set_player_stats(int pid)
{
   Active_player = Net_players[pid].player;
}
#endif
