/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "globalincs/systemvars.h"
#include "hud/hud.h"
#include "network/multi.h"
#include "pilotfile/pilotfile.h"
#include "playerman/player.h"
#include "stats/stats.h"

static const scoring_struct *Player_stats;
static scoring_struct All_time_ever_stats;

void show_stats_init()
{
	if (Game_mode & GM_MULTIPLAYER) {				
		set_player_stats(MY_NET_PLAYER_NUM);
	} else {
		Player_stats = &Player->stats;
	}	

	// multi only has current stats and all-time-ever stats, so this is only needed in single-player
	Pilot.export_stats(&All_time_ever_stats);
}

// write out the label for each stat
void show_stats_label(StatsType type, int sx, int sy, int dy)
{
	char text[NAME_LENGTH];

	if (type == StatsType::MISSION_STATS)
		gr_printf_menu(sx, sy, "%s", XSTR("Mission Stats", 114));
	else
		gr_printf_menu(sx, sy, "%s", XSTR("All Time Stats", 128));
	sy += 2 * dy;


	gr_printf_menu(sx, sy, "%s", XSTR("Total kills", 115));
	sy += 2 * dy;


	gr_printf_menu(sx, sy, "%s", XSTR("Primary weapon shots", 116));
	sy += dy;

	gr_printf_menu(sx, sy, "%s", XSTR("Primary weapon hits", 117));
	sy += dy;

	gr_printf_menu(sx, sy, "%s", XSTR("Primary friendly hits", 118));
	sy += dy;

	strcpy_s(text, XSTR("Primary hit %", 119));
	replace_one(text, "%%", "%", NAME_LENGTH - 1);
	gr_printf_menu(sx, sy, "%s", text);
	sy += dy;

	strcpy_s(text, XSTR("Primary friendly hit %", 120));
	replace_one(text, "%%", "%", NAME_LENGTH - 1);
	gr_printf_menu(sx, sy, "%s", text);
	sy += 2 * dy;


	gr_printf_menu(sx, sy, "%s", XSTR("Secondary weapon shots", 121));
	sy += dy;

	gr_printf_menu(sx, sy, "%s", XSTR("Secondary weapon hits", 122));
	sy += dy;

	gr_printf_menu(sx, sy, "%s", XSTR("Secondary friendly hits", 123));
	sy += dy;

	strcpy_s(text, XSTR("Secondary hit %", 124));
	replace_one(text, "%%", "%", NAME_LENGTH - 1);
	gr_printf_menu(sx, sy, "%s", text);
	sy += dy;

	strcpy_s(text, XSTR("Secondary friendly hit %", 125));
	replace_one(text, "%%", "%", NAME_LENGTH - 1);
	gr_printf_menu(sx, sy, "%s", text);
	sy += 2 * dy;


	gr_printf_menu(sx, sy, "%s", XSTR("Assists", 126));
	sy += 2 * dy;


	if (Game_mode & GM_MULTIPLAYER)
	{
		if (type == StatsType::MISSION_STATS)
		{
			gr_printf_menu(sx, sy, "%s", XSTR("Player Deaths", 127));
			sy += 2 * dy;
		}

		if (type == StatsType::MISSION_STATS)
			gr_printf_menu(sx, sy, "%s", XSTR("Mission score", 1526));
		else
			gr_printf_menu(sx, sy, "%s", XSTR("Score", 1527));
	}
}

void show_stats_numbers(StatsType type, int sx, int sy, int dy)
{
	switch (type)
	{
		case StatsType::MISSION_STATS:
			show_stats_numbers(*Player_stats, true, sx, sy, dy);
			break;

		case StatsType::ALL_TIME_CAMPAIGN_STATS:
			Assertion(!(Game_mode & GM_MULTIPLAYER), "ALL_TIME_CAMPAIGN_STATS are not stored for multiplayer!");
			show_stats_numbers(*Player_stats, false, sx, sy, dy);
			break;

		case StatsType::ALL_TIME_EVER_STATS:
			if (Game_mode & GM_MULTIPLAYER)
				show_stats_numbers(*Player_stats, false, sx, sy, dy);		// in multiplayer, the all-time-ever stats are stored in the player
			else
				show_stats_numbers(All_time_ever_stats, false, sx, sy, dy);
			break;

		default:
			UNREACHABLE("Unhandled StatsType %d in show_stats_numbers()", static_cast<int>(type));
	}
}

int stats_get_kills(StatsType type, int ship_class)
{
	switch (type)
	{
		case StatsType::MISSION_STATS:
			return stats_get_kills(*Player_stats, true, ship_class);

		case StatsType::ALL_TIME_CAMPAIGN_STATS:
			return stats_get_kills(*Player_stats, false, ship_class);

		case StatsType::ALL_TIME_EVER_STATS:
			if (Game_mode & GM_MULTIPLAYER)
				return stats_get_kills(*Player_stats, false, ship_class);		// in multiplayer, the all-time-ever stats are stored in the player
			else
				return stats_get_kills(All_time_ever_stats, false, ship_class);

		default:
			UNREACHABLE("Unhandled StatsType %d in stats_get_kills()", static_cast<int>(type));
			return 0;
	}
}

void show_stats_numbers(const scoring_struct &stats, bool use_m_stats, int sx, int sy, int dy)
{
	float		pct;
	int kill_count_ok, p_shots_fired, p_shots_hit, p_bonehead_hits, s_shots_fired, s_shots_hit, s_bonehead_hits, assists, score;

	if (use_m_stats)
	{
		kill_count_ok = stats.m_kill_count_ok;
		p_shots_fired = stats.mp_shots_fired;
		p_shots_hit = stats.mp_shots_hit;
		p_bonehead_hits = stats.mp_bonehead_hits;
		s_shots_fired = stats.ms_shots_fired;
		s_shots_hit = stats.ms_shots_hit;
		s_bonehead_hits = stats.ms_bonehead_hits;
		assists = stats.m_assists;
		score = stats.m_score;
	}
	else
	{
		kill_count_ok = stats.kill_count_ok;
		p_shots_fired = stats.p_shots_fired;
		p_shots_hit = stats.p_shots_hit;
		p_bonehead_hits = stats.p_bonehead_hits;
		s_shots_fired = stats.s_shots_fired;
		s_shots_hit = stats.s_shots_hit;
		s_bonehead_hits = stats.s_bonehead_hits;
		assists = stats.assists;
		score = stats.score;
	}

	sy += 2 * dy;

	// kills stats
	gr_printf_menu(sx, sy, "%d", kill_count_ok);
	sy += 2 * dy;


	// primary weapon stats
	gr_printf_menu(sx, sy, "%u", p_shots_fired);
	sy += dy;

	gr_printf_menu(sx, sy, "%u", p_shots_hit);
	sy += dy;

	gr_printf_menu(sx, sy, "%u", p_bonehead_hits);
	sy += dy;

	if (p_shots_fired > 0)
		pct = 100.0f * i2fl(p_shots_hit) / i2fl(p_shots_fired);
	else
		pct = 0.0f;
	gr_printf_menu(sx, sy, "%d%%", fl2i(pct));
	sy += dy;

	if (p_shots_fired > 0)
		pct = 100.0f * i2fl(p_bonehead_hits) / i2fl(p_shots_fired);
	else
		pct = 0.0f;
	gr_printf_menu(sx, sy, "%d%%", fl2i(pct));
	sy += 2 * dy;


	// secondary weapon stats
	gr_printf_menu(sx, sy, "%u", s_shots_fired);
	sy += dy;

	gr_printf_menu(sx, sy, "%u", s_shots_hit);
	sy += dy;

	gr_printf_menu(sx, sy, "%u", s_bonehead_hits);
	sy += dy;

	if (s_shots_fired > 0)
		pct = 100.0f * i2fl(s_shots_hit) / i2fl(s_shots_fired);
	else
		pct = 0.0f;
	gr_printf_menu(sx, sy, "%d%%", fl2i(pct));
	sy += dy;

	if (s_shots_fired > 0)
		pct = 100.0f * i2fl(s_bonehead_hits) / i2fl(s_shots_fired);
	else
		pct = 0.0f;
	gr_printf_menu(sx, sy, "%d%%", fl2i(pct));
	sy += 2 * dy;


	// assists and player rescues (respawns)
	gr_printf_menu(sx, sy, "%d", assists);
	sy += 2 * dy;


	if (Game_mode & GM_MULTIPLAYER)
	{
		if (use_m_stats)
		{
			gr_printf_menu(sx, sy, "%d", stats.m_player_deaths);
			sy += 2 * dy;
		}

		// score
		gr_printf_menu(sx, sy, "%d", score);
	}
}

int stats_get_kills(const scoring_struct &stats, bool use_m_stats, int ship_class)
{
	Assertion(ship_class >= 0 && ship_class < MAX_SHIP_CLASSES, "ship_class is out of bounds!");

	if (use_m_stats)
		return stats.m_okKills[ship_class];
	else
		return stats.kills[ship_class];
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
   Player_stats = &Net_players[pid].m_player->stats;
}
