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

enum class StatsType { MISSION_STATS, ALL_TIME_CAMPAIGN_STATS, ALL_TIME_EVER_STATS };

void show_stats_init();
void show_stats_close();
void set_player_stats(int pid);
void init_multiplayer_stats( void );  // initializes all mission specific stats to be 0

class scoring_struct;

void show_stats_label(StatsType type, int sx, int sy, int dy = 10);
void show_stats_numbers(StatsType type, int sx, int sy, int dy = 10);
void show_stats_numbers(const scoring_struct &stats, bool use_m_stats, int sx, int sy, int dy = 10);
int stats_get_kills(StatsType type, int ship_class);
int stats_get_kills(const scoring_struct &stats, bool use_m_stats, int ship_class);

#endif
