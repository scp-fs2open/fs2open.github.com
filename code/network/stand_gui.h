/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FREESPACE_STANDALONE_GUI_HEADER_FILE
#define _FREESPACE_STANDALONE_GUI_HEADER_FILE

#include "network/multi.h"
#include "network/multi_options.h"

#ifndef _WIN32
	void std_configLoaded(multi_global_options *options);
#endif

// ----------------------------------------------------------------------------------------
// external variables
//

struct net_player;

// ----------------------------------------------------------------------------------------
// generic dialog functions
//

// create the validate dialog 
void std_create_gen_dialog(char *title);

// kill the validate dialog();
void std_destroy_gen_dialog();

// set the text in the filename of the validate dialog
// valid values for field_num == 0 .. 2
void std_gen_set_text(char *str, int field_num);

// is the validate dialog active
int std_gen_is_active();


// ----------------------------------------------------------------------------------------
// connection page/tab functions
//

// set the text box indicating how many players are connected, returning the determined count
int std_connect_set_connect_count();

// set the connect status (connected or not) of the game host
void std_connect_set_host_connect_status();

// add an ip string to the connect page listbox
void std_connect_add_ip_string(char *string);

// remove an ip string from the connect page listbox
void std_connect_remove_ip_string(char *string);

// set an ip string on the connect page listbox
void std_connect_set_ip_string(char *lookup,char *string);

// kick a player (the one currently selected in the listbox)
void std_connect_kick_player();

// update the ping for this particular player
void std_connect_update_ping(net_player *p);

// clear all the controls for this page
void std_connect_clear_controls();

// set the game name for the standalone. passing NULL uses the default
void std_connect_set_gamename(char *name);

// the user has changed the text in the server name text box. handle this
void std_connect_handle_name_change();

// the user has changed the text in the host password text box
void std_connect_handle_passwd_change();


// ----------------------------------------------------------------------------------------
// multiplayer page/tab functions
//

// set the mission time in seconds
void std_multi_set_standalone_missiontime(float mission_time);

// set the mission name
void std_multi_set_standalone_mission_name(char *mission_name);

// initialize the goal tree for this mission 
void std_multi_setup_goal_tree();

// add all the goals from the current mission to the tree control
void std_multi_add_goals();

// update all the goals in the goal tree based upon the mission status
void std_multi_update_goals();

// set the framerate text box for this tab
void std_multi_set_framerate(float f);

// clear all the controls for this page
void std_multi_clear_controls();

// update the netgame information area controls with the current Netgame settings
void std_multi_update_netgame_info_controls();


// ---------------------------------------------------------------------------------------
// player info page/tab functions
//

// start displaying info for the passed player on this page
void std_pinfo_display_player_info(net_player *p);

// check to see if this player is the one being displayed, and if so, then update the display info
// return 1 if the player was updated
int std_pinfo_maybe_update_player_info(net_player *p);

// add a player to the list on the player info page
void std_pinfo_add_player_list_item(net_player *p);

// remove a player from the list on the player info page
void std_pinfo_remove_player_list_item(net_player *p);

// update the ping display for this player
void std_pinfo_update_ping(net_player *p);

// clear all the controls for this page
void std_pinfo_clear_controls();


// ---------------------------------------------------------------------------------------
// player god stuff page/tab functions
//

// add a player to the listbox on the godstuff page
void std_gs_add_player(net_player *p);

// remove a player from the listbox on the godstuff page
void std_gs_remove_player(net_player *p);

// send a message as if the standalone were a player
void std_gs_send_godstuff_message();

// set the framerate text box for this page
void std_gs_set_framerate(float f);

// clear all the controls for this page
void std_gs_clear_controls();


// ---------------------------------------------------------------------------------------
// debug page/tab functions
//

// set the text on the standalones state indicator box
void std_debug_set_standalone_state_string(char *str);

// clear all the controls for this page
void std_debug_clear_controls();


// ---------------------------------------------------------------------------------------
// general functions
// 

// add a player and take care of updating all gui/data details
void std_add_player(net_player *p);

// remove a player and take care of updateing all gui/data details
int std_remove_player(net_player *p);

// set any relevant controls which display the framerate of the standalone
void std_set_standalone_fps(float fps);

// update any relveant controls which display the ping for the given player
void std_update_player_ping(net_player *p);

// reset all gui stuff for the standalone
void std_reset_standalone_gui();

// reset all networking/gui stuff (calls reset_standalone_gui) for the standalone
void std_multi_standalone_reset_all();

// setup registry access, etc for the standalone
void std_init_os();

// close down the standalone
void std_deinit_standalone();

// initialize the standalone
void std_init_standalone();

// do any gui related issues on the standalone (like periodically updating player stats, etc...)
void std_do_gui_frame();

// notify the user that the standalone has failed to login to the tracker on startup
void std_tracker_notify_login_fail();

// attempt to log the standalone into the tracker
void std_tracker_login();

// reset all stand gui timestamps
void std_reset_timestamps();

// add a line of text chat to the standalone
void std_add_chat_text(const char *text,int player_index,int add_id);

// if the standalone is host password protected
int std_is_host_passwd();

// change the default property sheet interface into something more useful
void std_mutate_sheet();

// if the given callsign is banned from the server
int std_player_is_banned(const char *name);

// add a callsign to the ban list
void std_add_ban(const char *name);


#endif
