/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _FREESPACE_SPECIFIC_MASTER_TRACKER_HEADER
#define _FREESPACE_SPECIFIC_MASTER_TRACKER_HEADER

#include "network/ptrack.h"

// -----------------------------------------------------------------------------------
// FREESPACE MASTER TRACKER DEFINES/VARS
//

// tracker mission validation status
#define MVALID_STATUS_UNKNOWN					-1
#define MVALID_STATUS_VALID					0
#define MVALID_STATUS_INVALID					1

// tracker squad war validation status
#define MSW_STATUS_UNKNOWN						-1
#define MSW_STATUS_VALID						0
#define MSW_STATUS_INVALID						1

// tracker table validation status
#define TVALID_STATUS_UNKNOWN						-1
#define TVALID_STATUS_VALID						0
#define TVALID_STATUS_INVALID						1

//struct vmt_freespace2_struct;
class scoring_struct;
struct squad_war_request;
struct squad_war_result;

// channel to associate when creating a server
extern char Multi_fs_tracker_channel[MAX_PATH];

// channel to use when polling the tracker for games
extern char Multi_fs_tracker_filter[MAX_PATH];

// used for mod detection
extern short Multi_fs_tracker_game_id;
extern SCP_string Multi_fs_tracker_game_name;

// -----------------------------------------------------------------------------------
// FREESPACE MASTER TRACKER DECLARATIONS
//

// give some processor time to the tracker API
void multi_fs_tracker_process();

// initialize the master tracker API for Freespace
void multi_fs_tracker_init();

// validate the current player with the master tracker (will create the pilot on the MT if necessary)
int multi_fs_tracker_validate(int show_error);

// attempt to log the current game server in with the master tracker
void multi_fs_tracker_login_freespace();

// attempt to update all player statistics and scores on the tracker
int multi_fs_tracker_store_stats();

// attempt to update all player statistics (standalone mode)
int multi_fs_std_tracker_store_stats();

// log freespace out of the tracker
void multi_fs_tracker_logout();

// send a request for a list of games
void multi_fs_tracker_send_game_request();

// if the API has successfully been initialized and is running
int multi_fs_tracker_inited();

// update our settings on the tracker regarding the current netgame stuff
void multi_fs_tracker_update_game(netgame_info *ng);

// if we're currently busy performing some tracker operation (ie, you should wait or not)
int multi_fs_tracker_busy();

// copy a freespace stats struct to a tracker-freespace stats struct
void multi_stats_fs_to_tracker(scoring_struct *fs, vmt_stats_struct *vmt, player *pl, int tracker_id);

// copy a tracker-freespace stats struct to a freespace stats struct
void multi_stats_tracker_to_fs(vmt_stats_struct *vmt, scoring_struct *fs);

// return an MVALID_STATUS_* value, or -2 if the user has "cancelled"
int multi_fs_tracker_validate_mission(char *filename);

// return an MSW_STATUS_* value
int multi_fs_tracker_validate_sw(squad_war_request *sw_req, char *bad_reply, const size_t max_reply_len);

// store the results of a squad war mission on PXO, return 1 on success
int multi_fs_tracker_store_sw(squad_war_result *sw_res, char *bad_reply, const size_t max_reply_len);

// check all tables with tracker
// this is hacked data check as well as mod ident
int multi_fs_tracker_validate_game_data();

// verify and possibly update Multi_options_g with sane PXO values
void multi_fs_tracker_verify_options();

// report the status of PXO game probe (firewall check)
void multi_fs_tracker_report_probe_status(int flags, int next_try);

#endif
