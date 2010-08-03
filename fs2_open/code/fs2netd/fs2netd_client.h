/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FS2NETD_CLIENT_H
#define _FS2NETD_CLIENT_H

#include "network/multi.h"

struct net_addr;
struct player;

// channel to associate when creating a server
extern char Multi_fs_tracker_channel[];

// channel to use when polling the tracker for games
extern char Multi_fs_tracker_filter[];

typedef struct tracker_game_data {
	char name[MAX_GAMENAME_LEN+1];
	char mission_name[MAX_GAMENAME_LEN+1];
	char title[NAME_LENGTH+1];
	char campaign_name[NAME_LENGTH+1];
	char chat_channel[MAX_PATH+1];

	ubyte campaign_mode;
	int flags;
	int type_flags;
	short players;
	short max_players;
	ubyte mode;
	ubyte rank_base;
	ubyte game_state;
	ubyte speed;
} tracker_game_data;

extern tracker_game_data Multi_tracker_game_data;


bool fs2netd_is_online();

void fs2netd_reset_connection();
void fs2netd_disconnect();

void fs2netd_close();

bool fs2netd_login();

void fs2netd_maybe_init();

void fs2netd_do_frame();

void fs2netd_gameserver_start();
void fs2netd_gameserver_update(bool force = false);
void fs2netd_gameserver_disconnect();

void fs2netd_send_game_request();

void fs2netd_store_stats();

bool fs2netd_player_banned(net_addr *addr);

void fs2netd_update_ban_list();

bool fs2netd_get_valid_missions();

int fs2netd_update_valid_tables();

int fs2netd_get_pilot_info(const char *callsign, player *out_plr, bool first_call);

void fs2netd_options_config_init();

void fs2netd_add_table_validation(char *tblname);

void fs2netd_update_game_count(char *chan_name);

#endif // _FS2NETD_CLIENT_H
