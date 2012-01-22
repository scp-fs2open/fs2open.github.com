/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#ifdef _WIN32
#include <io.h>
#include <winsock.h>
#endif

#include "globalincs/systemvars.h"
#include "playerman/player.h"
#include "stats/stats.h"
#include "hud/hud.h"
#include "network/multi.h"


#define MISSION_STATS_START_Y 80
#define ALLTIME_STATS_START_Y 270
#define MULTIPLAYER_LIST_START 20

/*
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
*/

// static UI_WINDOW Player_stats_window;

player *Active_player;

void show_stats_init()
{
	// Player_stats_window.create( 0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0 );
	if (Game_mode & GM_MULTIPLAYER) {				
		set_player_stats(MY_NET_PLAYER_NUM);
	} else {
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
			sprintf(text,"%u",Active_player->stats.mp_shots_fired);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%u",Active_player->stats.mp_shots_hit);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%u",Active_player->stats.mp_bonehead_hits);
			gr_printf(sx,sy,text);
			sy += dy;
			if(Active_player->stats.mp_shots_fired>0)
				pct=(float)100.0*((float)Active_player->stats.mp_shots_hit/(float)Active_player->stats.mp_shots_fired);
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat_s(text," %%");
			gr_printf(sx,sy,text);
			sy += dy;
			if(Active_player->stats.mp_bonehead_hits>0)
				pct=(float)100.0*((float)Active_player->stats.mp_bonehead_hits/(float)Active_player->stats.mp_shots_fired);
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat_s(text," %%");
			gr_printf(sx,sy,text);
			sy += 2*dy;

			// mission secondary weapon stats
			sprintf(text,"%u",Active_player->stats.ms_shots_fired);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%u",Active_player->stats.ms_shots_hit);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%u",Active_player->stats.ms_bonehead_hits);
			gr_printf(sx,sy,text);
			sy += dy;
			if(Active_player->stats.ms_shots_fired>0)
				pct=(float)100.0*((float)Active_player->stats.ms_shots_hit/(float)Active_player->stats.ms_shots_fired);
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat_s(text," %%");
			gr_printf(sx,sy,text);
			sy += dy;
			if(Active_player->stats.ms_bonehead_hits>0)
				pct=(float)100.0*((float)Active_player->stats.ms_bonehead_hits/(float)Active_player->stats.ms_shots_fired);
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat_s(text," %%");
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
			sprintf(text,"%u",Active_player->stats.p_shots_fired + add.p_shots_fired);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%u",Active_player->stats.p_shots_hit + add.p_shots_hit);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%u",Active_player->stats.p_bonehead_hits + add.p_bonehead_hits);
			gr_printf(sx,sy,text);
			sy += dy;
			if((Active_player->stats.p_shots_fired + add.p_shots_fired)>0)
				pct=(float)100.0*((float)(Active_player->stats.p_shots_hit+add.p_shots_hit)/(float)(Active_player->stats.p_shots_fired + add.p_shots_fired));
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat_s(text," %%");
			gr_printf(sx,sy,text);
			sy += dy;
			if((Active_player->stats.p_bonehead_hits + add.p_bonehead_hits)>0)
				pct=(float)100.0*((float)(Active_player->stats.p_bonehead_hits+add.p_bonehead_hits)/(float)(Active_player->stats.p_shots_fired + add.p_shots_fired));
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat_s(text," %%");
			gr_printf(sx,sy,text);
			sy += 2*dy;

			// alltime secondary weapon stats
			sprintf(text,"%u",Active_player->stats.s_shots_fired + add.s_shots_fired);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%u",Active_player->stats.s_shots_hit + add.s_shots_hit);
			gr_printf(sx,sy,text);
			sy += dy;
			sprintf(text,"%u",Active_player->stats.s_bonehead_hits + add.s_bonehead_hits);
			gr_printf(sx,sy,text);
			sy += dy;
			if((Active_player->stats.s_shots_fired+add.s_shots_fired)>0)
				pct=(float)100.0*((float)(Active_player->stats.s_shots_hit + add.s_shots_hit)/(float)(Active_player->stats.s_shots_fired + add.s_shots_fired));
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat_s(text," %%");
			gr_printf(sx,sy,text);
			sy += dy;
			if((Active_player->stats.s_bonehead_hits + add.s_bonehead_hits)>0)
				pct=(float)100.0*((float)(Active_player->stats.s_bonehead_hits+add.s_bonehead_hits)/(float)(Active_player->stats.s_shots_fired+add.s_shots_fired));
			else pct=(float)0.0;
			sprintf(text,"%d",(int)pct); strcat_s(text," %%");
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


void show_stats_close()
{	
}
 
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
   Active_player = Net_players[pid].m_player;
}
