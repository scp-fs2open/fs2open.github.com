/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FS_STATISTICS_STATE_HEADER
#define _FS_STATISTICS_STATE_HEADER

#define MISSION_STATS	0
#define ALL_TIME_STATS	1

void show_stats_init();
void show_stats_close();
void set_player_stats(int pid);
void init_multiplayer_stats( void );  // initializes all mission specific stats to be 0

void show_stats_numbers(int stage, int sx, int sy, int dy=10,int add_mission = 0);
void show_stats_label(int stage, int sx, int sy, int dy=10);

#endif
