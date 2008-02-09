/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /freespace2/code/fs2netd/fs2netd_client.h $
 * $Revision: 1.1.2.1 $
 * $Date: 2007-10-15 06:43:10 $
 * $Author: taylor $
 *
 * FS2NetD client handler (header)
 *
 * $Log: not supported by cvs2svn $
 *
 * $NoKeywords: $
 */

#ifndef _FS2NETD_CLIENT_H
#define _FS2NETD_CLIENT_H

struct net_addr;
struct player;


void fs2netd_close();

bool fs2netd_login();

void fs2netd_maybe_init();

void fs2netd_do_frame();

int fs2netd_load_servers();

void fs2netd_debrief_init();

bool fs2netd_player_banned(net_addr *addr);

void fs2netd_update_ban_list();

bool fs2netd_get_valid_missions();

int fs2netd_update_valid_tables();

int fs2netd_get_pilot_info(const char *callsign, player *out_plr, bool first_call);

void fs2netd_options_config_init();

void fs2netd_add_table_validation(char *tblname);

#endif // _FS2NETD_CLIENT_H
