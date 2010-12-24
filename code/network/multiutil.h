/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef MULTI_UTIL_H
#define MULTI_UTIL_H

#include "network/psnet2.h"

// prototypes instead of headers :)
struct net_player;
struct net_addr;
struct player;
struct button_info;
struct join_request;
struct physics_info;
struct object;
struct active_game;
struct ship;
struct server_item;
struct ship_info;
struct p_object;

// two types of signatures that we can request,  permanent signatures are all below 1000.  non-permanent are above 1000
#define MULTI_SIG_SHIP					1
#define MULTI_SIG_ASTEROID				2
#define MULTI_SIG_NON_PERMANENT		3
#define MULTI_SIG_DEBRIS				4

extern ushort multi_assign_network_signature( int what_kind );
extern ushort multi_get_next_network_signature( int what_kind );
extern void multi_set_network_signature( ushort signature, int what_kind );

extern void stuff_netplayer_info( net_player *nplayer, net_addr *addr, int ship_class, player *pplayer );
extern int find_player(net_addr* addr);
extern int find_player_no_port(net_addr *addr);
extern int find_player_id(short player_id);
extern int find_player_socket(PSNET_SOCKET_RELIABLE sock);	// note this is only valid to do on a server!
extern int multi_find_player_by_object( object *obj );
extern int multi_find_player_by_signature( int signature );
extern int multi_find_player_by_callsign(char *callsign);
extern int multi_find_player_by_net_signature(ushort net_signature);
extern int multi_find_player_by_parse_object(p_object *p_objp );
extern int multi_find_player_by_ship_name(char *ship_name, bool inc_respawning = false);
extern int multi_create_player(int player_num, player *pl,char* name, net_addr* addr, int ship_class, short id);
extern int multi_find_open_netplayer_slot();
extern int multi_find_open_player_slot();
extern void delete_player(int player_num, int kicked_reason = -1);
extern int multi_get_player_ship(int np_index);

extern int multi_num_players();
extern int multi_num_observers();
extern int multi_num_connections();

extern char* multi_random_death_word();
extern char* multi_random_chat_start();

extern int multi_ship_class_lookup(char* ship_name);
extern ushort netmisc_calc_checksum( void * vptr, int len );
extern void fill_net_addr(net_addr* addr, ubyte* address, ubyte* net_id, ushort port);
extern char* get_text_address( char * text, ubyte * address );

extern object *multi_get_network_object( ushort net_signature );		// find a network object

void multi_find_ingame_join_pos(object *new_obj);

// return size of packed matrix
void multi_pack_orient_matrix(ubyte *data,matrix *m);

// return bytes processed
void multi_unpack_orient_matrix(ubyte *data,matrix *m);

// catchall to do any necessary client-side simulation processing or master side process for menu pauses, etc.
void multi_do_client_warp(float frame_time);

void multi_assign_player_ship( int net_player, object *objp, int ship_class );

// -------------------------------------------------------------------
// ship status change functions (used both client and server side)
int lookup_ship_status(net_player *p, int unique_id, int remove=0);        // auto-remove if remove == 1
void remove_ship_status_item(net_player *p, int id);     
void add_net_button_info(net_player *p, button_info *bi, int unique_id);

// called client-side every frame
void multi_maybe_send_ship_status();

// will be used server side _and_ client side. 
void multi_apply_ship_status(net_player *p,button_info *bi, int locally);

void multiplayer_match_target_speed(net_player *p);

void multi_subsys_update_all();

void server_verify_filesig(short player_id, ushort sum_sig, int length_sig);
int server_all_filesigs_ok();

void multi_untag_player_ships();

// broadcast alltime stats to everyone in the game
void multi_broadcast_stats(int stats_code);

int multi_netplayer_state_check(int state, int ignore_standalone = 0);
int multi_netplayer_state_check2(int state, int state2, int ignore_standalone = 0);
int multi_netplayer_state_check3(int state, int state2, int state3, int ignore_standalone = 0);
int multi_netplayer_flag_check(int flags, int ignore_standalone = 0);

void multi_eval_socket_error(PSNET_SOCKET sock, int error);

void multi_maybe_send_repair_info(object *dest_obj, object *source_objp, int code);

int multi_is_valid_unknown_packet(ubyte type);

// create a bogus object for the standalone
void multi_create_standalone_object();

// determine whether (as a server), you should be rebroadcasting certain messages to everyone in the game
int multi_message_should_broadcast(int type);

// the active game list manager functions
active_game *multi_new_active_game( void );
active_game *multi_update_active_games(active_game *ag);
void multi_free_active_games();

server_item *multi_new_server_item( void );
void multi_free_server_list();

// netgame options evaluation stuff
int multi_can_message(net_player *p);
int multi_can_end_mission(net_player *p);

int multi_eval_join_request(join_request *jr,net_addr *addr);

// called by any machine (client, host, server, standalone, etc), to begin warping out all player objects
void multi_warpout_all_players();

// determine the highest rank of any of the players in the game
int multi_get_highest_rank();

// called on the machine of the player who hit alt+j
void multi_handle_end_mission_request();

// called to handle any special cases where a player is in some submenu when he needs to get pushed into some other state
void multi_handle_state_special();

// called by the file xfer subsytem when we start receiving a file
void multi_file_xfer_notify(int handle);

// return the lag/disconnected status of the game
int multi_query_lag_status();

// process a valid join request
void multi_process_valid_join_request(join_request *jr, net_addr *who_from, int ingame_join_team = -1);

// if a player is trying to join a restricted game, evaluate the keypress (accept or not, etc)
int multi_process_restricted_keys(int k);

// determine the status of available player ships (use team_0 for non team vs. team situations)
void multi_player_ships_available(int *team_0, int *team_1);

// server should update the player's bank/link status with the data in the passed ship
void multi_server_update_player_weapons(net_player *pl, ship *shipp);

// flush the multidata cache directory
void multi_flush_multidata_cache();

// flush all data from a previous mission before starting the next
void multi_flush_mission_stuff();

// should we ignore all controls and keypresses because of some multiplayer 
int multi_ignore_controls(int key = -1);

// if the kill limit has been reached by any given player
int multi_kill_limit_reached();

// display a chat message (write to the correct spot - hud, standalone gui, chatbox, etc)
void multi_display_chat_msg(char *msg, int player_index, int add_id);

// fill in Current_file_checksum and Current_file_length
void multi_get_mission_checksum(char *filename);

// Packs/unpacks an object position.
// Returns number of bytes read or written.
#define OO_POS_RET_SIZE							9
int multi_pack_unpack_position(int write, ubyte *data, vec3d *pos);

// Packs/unpacks an orientation matrix.
// Returns number of bytes read or written.
#define OO_ORIENT_RET_SIZE						6
int multi_pack_unpack_orient(int write, ubyte *data, matrix *orient);

// Packs/unpacks velocity
// Returns number of bytes read or written.
#define OO_VEL_RET_SIZE							4
int multi_pack_unpack_vel(int write, ubyte *data, matrix *orient, vec3d *pos, physics_info *pi);

// Packs/unpacks desired_velocity
// Returns number of bytes read or written.
#define OO_DESIRED_VEL_RET_SIZE				3
int multi_pack_unpack_desired_vel(int write, ubyte *data, matrix *orient, vec3d *pos, physics_info *pi, ship_info *sip);

// Packs/unpacks rotational velocity
// Returns number of bytes read or written.
#define OO_ROTVEL_RET_SIZE						4
int multi_pack_unpack_rotvel(int write, ubyte *data, matrix *orient, vec3d *pos, physics_info *pi);

// Packs/unpacks desired rotvel
// Returns number of bytes read or written.
#define OO_DESIRED_ROTVEL_RET_SIZE			3
int multi_pack_unpack_desired_rotvel(int write, ubyte *data, matrix *orient, vec3d *pos, physics_info *pi, ship_info *sip);

char multi_unit_to_char(float unit);
float multi_char_to_unit(float val);

// if we should render our ping time to the server in a multiplayer game
int multi_show_ingame_ping();

// if Game_current_mission_filename is a builtin multiplayer mission
int multi_is_builtin_mission();

int multi_get_connection_speed();

// if we're in tracker mode, do a validation update on all known missions
void multi_update_valid_missions();

// get a new id# for a player
short multi_get_new_id();

// Karajorma - sends the player to the correct debrief for this game type
void send_debrief_event();

// Karajorma - Performs any cleanup needed by missions which don't end with a warpout.
void multi_handle_sudden_mission_end();

// make a bunch of fake players - don't rely on this to be very safe - its mostly used for interface testing
#ifndef NDEBUG
void multi_make_fake_players(int count);
#endif

#endif
