/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#ifndef MULTI_MSGS_H
#define MULTI_MSGS_H

#include "globalincs/pstypes.h"

#include <cstdint>

struct net_player;
struct net_addr;
class object;
class ship;
struct wing;
struct join_request;
struct button_info;
struct header;
struct beam_info;
class ship_subsys;

// macros for building up packets -- to save on time and typing.  Important to note that local variables
// must be named correctly
// there are two flavors of sending orientation matrices, 16 bit and 32 bit. Just #define ORIENT_16 to use
// 16 bits, otherwise 32 bits is the default

#define BUILD_HEADER(t) do { data[0]=t; packet_size = HEADER_LENGTH; } while(0)
#define ADD_DATA(d) do { Assert((packet_size + sizeof(d)) < MAX_PACKET_SIZE); memcpy(data+packet_size, &d, sizeof(d) ); packet_size += sizeof(d); } while (0)
#define ADD_SHORT(d) do { static_assert(sizeof(d) == sizeof(std::int16_t), "Size of short is not right!"); Assert((packet_size + sizeof(d)) < MAX_PACKET_SIZE); short swap = INTEL_SHORT(d); memcpy(data+packet_size, &swap, sizeof(d) ); packet_size += sizeof(d); } while (0)
#define ADD_USHORT(d) do { static_assert(sizeof(d) == sizeof(std::uint16_t), "Size of unsigned short is not right!"); Assert((packet_size + sizeof(d)) < MAX_PACKET_SIZE); ushort swap = INTEL_SHORT(d); memcpy(data+packet_size, &swap, sizeof(d) ); packet_size += sizeof(d); } while (0)
#define ADD_INT(d) do { static_assert(sizeof(d) == sizeof(std::int32_t), "Size of int is not right!"); Assert((packet_size + sizeof(d)) < MAX_PACKET_SIZE); int swap = INTEL_INT(d); memcpy(data+packet_size, &swap, sizeof(d) ); packet_size += sizeof(d); } while (0)
#define ADD_UINT(d) do { static_assert(sizeof(d) == sizeof(std::uint32_t), "Size of unsigned int is not right!"); Assert((packet_size + sizeof(d)) < MAX_PACKET_SIZE); uint swap = INTEL_INT(d); memcpy(data+packet_size, &swap, sizeof(d) ); packet_size += sizeof(d); } while (0)
#define ADD_FLOAT(d) do { Assert((packet_size + sizeof(d)) < MAX_PACKET_SIZE); float swap = INTEL_FLOAT(&d); memcpy(data+packet_size, &swap, sizeof(d) ); packet_size += sizeof(d); } while (0)
#define ADD_STRING(s) do { Assert((packet_size + strlen(s) + 4) < MAX_PACKET_SIZE);int len = strlen(s); int len_tmp = INTEL_INT(len); ADD_DATA(len_tmp); memcpy(data+packet_size, s, len ); packet_size += len; } while(0)
#define ADD_ORIENT(d) { Assert((packet_size + 17) < MAX_PACKET_SIZE); ubyte dt[17]; multi_pack_orient_matrix(dt,&d); memcpy(data+packet_size,dt,17); packet_size += 17; }
#define ADD_VECTOR(d) do { vec3d tmpvec = ZERO_VECTOR; tmpvec.xyz.x = INTEL_FLOAT(&d.xyz.x); tmpvec.xyz.y = INTEL_FLOAT(&d.xyz.y); tmpvec.xyz.z = INTEL_FLOAT(&d.xyz.z); ADD_DATA(tmpvec); } while(0)

#define GET_DATA(d) do { memcpy(&d, data+offset, sizeof(d) ); offset += sizeof(d); } while(0)
#define GET_SHORT(d) do { short swap; memcpy(&swap, data+offset, sizeof(d) ); d = INTEL_SHORT(swap); offset += sizeof(d); } while(0)
#define GET_USHORT(d) do { ushort swap; memcpy(&swap, data+offset, sizeof(d) ); d = INTEL_SHORT(swap); offset += sizeof(d); } while(0)
#define GET_INT(d) do { int swap; memcpy(&swap, data+offset, sizeof(d) ); d = INTEL_INT(swap); offset += sizeof(d); } while(0)
#define GET_UINT(d) do { uint swap; memcpy(&swap, data+offset, sizeof(d) ); d = INTEL_INT(swap); offset += sizeof(d); } while(0)
#define GET_FLOAT(d) do { float swap; memcpy(&swap, data+offset, sizeof(d) ); d = INTEL_FLOAT(&swap); offset += sizeof(d); } while(0)
#define GET_STRING(s) do { int len;  memcpy(&len, data+offset, sizeof(len)); len = INTEL_INT(len); offset += sizeof(len); memcpy(s, data+offset, len); offset += len; s[len] = '\0'; } while(0)
#define GET_ORIENT(d) { ubyte dt[17]; memcpy(dt,data+offset,17); offset+=17; multi_unpack_orient_matrix(dt,&d); }
#define GET_VECTOR(d) do { vec3d tmpvec = ZERO_VECTOR; GET_DATA(tmpvec); d.xyz.x = INTEL_FLOAT(&tmpvec.xyz.x); d.xyz.y = INTEL_FLOAT(&tmpvec.xyz.y); d.xyz.z = INTEL_FLOAT(&tmpvec.xyz.z); } while(0)

#define PACKET_SET_SIZE() do { hinfo->bytes_processed = offset; } while(0)

// defines for weapon status changes.
#define MULTI_PRIMARY_CHANGED		1
#define MULTI_SECONDARY_CHANGED	2

// data sending wrappers

// send the specified data packet to all players
void multi_io_send(net_player *pl, ubyte *data, int length);
void multi_io_send_to_all(ubyte *data, int length, net_player *ignore = NULL);
void multi_io_send_force(net_player *pl);

// send the data packet to all players via their reliable sockets
void multi_io_send_reliable(net_player *pl, ubyte *data, int length);
void multi_io_send_to_all_reliable(ubyte* data, int length, net_player *ignore = NULL);
void multi_io_send_reliable_force(net_player *pl);

// send all buffered packets
void multi_io_send_buffered_packets();


// packet handlers -------------------------------------------------------------------------------

// process an incoming join request packet
void process_join_packet( ubyte* data, header* hinfo );

// process an accept packet from the server
void process_accept_packet( ubyte* data, header* hinfo );

// process a notification for a new player who has joined the game
void process_new_player_packet(ubyte* data, header* hinfo);

// process an incoming hud message packet
void process_hud_message(ubyte* data, header* hinfo);

// process a notification the a player has left the game
void process_leave_game_packet(ubyte* data, header* hinfo);

// process information about an active game
void process_game_active_packet(ubyte* data, header* hinfo);

// process a query from a client looking for active freespace games
void process_game_query(ubyte* data, header* hinfo);

// process a general game chat packet, if we're the standalone we should rebroadcast
void process_game_chat_packet( ubyte *data, header *hinfo );

// process a game information update
void process_game_info_packet( ubyte *data, header *hinfo );

// process a packet indicating a secondary weapon was fired
void process_secondary_fired_packet(ubyte* data, header* hinfo, int flag);

// process a packet indicating a countermeasure was fired
void process_countermeasure_fired_packet( ubyte *data, header *hinfo );

// process information about the netgame sent from the server/host
void process_netgame_update_packet( ubyte *data, header *hinfo );

// process an incoming netgame description packet
void process_netgame_descript_packet( ubyte *data, header *hinfo );

// process an incoming netplayer state update. if we're the server, we should rebroadcast
void process_netplayer_update_packet( ubyte *data, header *hinfo );

void process_ship_status_packet(ubyte *data, header *hinfo);
void process_player_order_packet(ubyte *data, header *hinfo);

// process an object update packet.  See send_object_update for information on how
// this packet data should be interpreted.  We send different information depending on
// on several variables (no change in orienation, etc).
void process_object_update_packet( ubyte *data, header *hinfo );

// process a packet indicating that a ship has been killed
void process_ship_kill_packet( ubyte *data, header *hinfo );

// process a packet saying that a wing should be created
void process_wing_create_packet( ubyte *data, header *hinfo );

// process a packet indicating a ship should be created
void process_ship_create_packet( ubyte *data, header *hinfo );

// process a packet indicating a ship is departing
void process_ship_depart_packet( ubyte *data, header *hinfo );

// process a mission log item packet
void process_mission_log_packet( ubyte *data, header *hinfo );

// process a mission message packet
void process_mission_message_packet( ubyte *data, header *hinfo );

// just send them a pong back as fast as possible
void process_ping_packet(ubyte *data, header *hinfo);

// right now it just routes the pong through to the standalone gui, which is the only
// system which uses ping and pong right now.
void process_pong_packet(ubyte *data, header *hinfo);

// process a request for a list of missions
void process_mission_request_packet(ubyte *data, header *hinfo);

// process an individual mission file item
void process_mission_item_packet(ubyte *data, header *hinfo);

// process a pause update packet (pause, unpause, etc)
void process_multi_pause_packet(ubyte *data, header *hinfo);

// process an ingame nak packet
void process_ingame_nak(ubyte *data, header *hinfo);

void process_ingame_ships_packet(ubyte *data, header *hinfo);
void process_ingame_wings_packet(ubyte *data, header *hinfo);

// process a packet indicating we should end the current mission
void process_endgame_packet(ubyte *data, header *hinfo);

// process a packet indicating we should jump straight to the debrief screen
void process_force_end_mission_packet(ubyte *data, header *hinfo);

// process a position/orientation update from an observer
void process_observer_update_packet(ubyte *data, header *hinfo);

void process_netplayer_slot_packet(ubyte *data, header *hinfo);
void process_netplayer_class_packet(ubyte *data, header *hinfo);

void process_subsys_update_packet(ubyte *data, header *hinfo);

void process_ingame_ship_update_packet(ubyte *data, header *hinfo);

void process_file_sig_packet(ubyte *data, header *hinfo);
void process_file_sig_request(ubyte *data, header *hinfo);

void process_ingame_respawn_points_packet(ubyte *data, header *hinfo);

void process_subsystem_destroyed_packet( ubyte *data, header *hinfo );

void process_netplayer_load_packet(ubyte *data, header *hinfo);

void process_jump_into_mission_packet(ubyte *data, header *hinfo);

void process_repair_info_packet(ubyte *data, header *hinfo);

void process_mission_sync_packet(ubyte *data, header *hinfo);

void process_store_stats_packet(ubyte *data, header *hinfo);

void process_debris_update_packet(ubyte *data, header *hinfo);

void process_ship_weapon_state_packet(ubyte *data, header *hinfo );
void process_ship_weapon_change( ubyte *data, header *hinfo );

void process_firing_info_packet( ubyte *data, header *hinfo );

// process a cargo revealed packet
void process_cargo_revealed_packet( ubyte *data, header *hinfo );

void process_subsystem_cargo_revealed_packet( ubyte *data, header *hinfo );

void process_cargo_hidden_packet( ubyte *data, header *hinfo );

void process_subsystem_cargo_hidden_packet( ubyte *data, header *hinfo );

void process_mission_goal_info_packet( ubyte *data, header *hinfo );

void process_player_kick_packet(ubyte *data, header *hinfo);

void process_player_settings_packet(ubyte *data, header *hinfo);

void process_deny_packet(ubyte *data, header *hinfo);

void process_post_sync_data_packet(ubyte *data, header *hinfo);

void process_wss_slots_data_packet(ubyte *data, header *hinfo);

void process_shield_explosion_packet( ubyte *data, header *hinfo );

void process_player_stats_block_packet(ubyte *data, header *hinfo);

void process_host_restr_packet(ubyte *data, header *hinfo);

void process_netgame_end_error_packet(ubyte *data, header *hinfo);

void process_client_update_packet(ubyte *data, header *hinfo);

void process_countdown_packet(ubyte *data, header *hinfo);

// send a join packet request to the specified address (should be a server)
void send_join_packet(net_addr* addr,join_request *jr);

// send an accept packet to a client in response to a request to join the game
void send_accept_packet(int new_player_num, int code, int ingame_join_team = -1);

// send a general game chat packet (if msg_mode == MULTI_MSG_TARGET, need to pass in "to", if == MULTI_MSG_EXPR, need to pass in expr)
void send_game_chat_packet(net_player *from, const char *msg, int msg_mode, net_player *to = NULL, const char *expr = NULL,int server_msg = 0);

// send a game information update
void send_game_info_packet( void );

// send a notice that the player at net_addr is leaving (if target is NULL, the broadcast the packet)
void send_leave_game_packet(short player_id = -1,int kicked_reason = -1,net_player *target = NULL);

// send a packet indicating a secondary weapon was fired
void send_secondary_fired_packet( ship *shipp, ushort starting_sig, int starting_count, int num_fired, int allow_swarm );

// send a packet indicating a countermeasure was fired
void send_countermeasure_fired_packet( object *objp, int cmeasure_count, int rand_val );



// send_game_update_packet sends an updated Netgame structure to all players currently connected.  The update
// is used to change the current mission, current state, etc.
void send_netgame_update_packet(net_player *pl = NULL);

// sends information about netplayers in the game. if called on the server, broadcasts information about _all_ players
void send_netplayer_update_packet( net_player *pl = NULL );

void send_ship_status_packet(net_player *pl, button_info *bi, int id);
void send_player_order_packet(int type, int index, int command);

// send a request or a reply for mission description, if code == 0, request, if code == 1, reply
void send_netgame_descript_packet(net_addr *addr, int code);

// send object update packet sends object updates for all objects in the game.  This function will be smart
// about sending only certain objects to certain players based on the players distance from an object, whether
// the object is behind the player, etc.
void send_object_update_packet(int force_all = 0);

// send a packet indicating a ship has been killed
void send_ship_kill_packet( object *ship_obj, object *other_objp, float percent_killed, int self_destruct );

// send a packet indicating a wing of ships should be created
void send_wing_create_packet( wing *wingp, int num_to_create, int pre_create_count );

// send a packet indicating a ship should be created
void send_ship_create_packet( object *objp, int is_support = 0 );

// packet indicating a ship is departing
void send_ship_depart_packet( object *objp, int method = -1 );

// send a mission log item packet
void send_mission_log_packet( int entry );

// send a mission message packet
void send_mission_message_packet(int id, char *who_from, int priority, int timing, int source, int builtin_type, int multi_target, int multi_team_filter, int delay = 0);

// broadcast a query for active games. IPX will use net broadcast and TCP will either request from the MT or from the specified list
void broadcast_game_query();

// send an individual query to an address to see if there is an active game
void send_server_query(net_addr *addr);

// broadcast a hud message to all players
void send_hud_msg_to_all( char* msg );
void send_heartbeat();

// send a ping packet
void send_ping(net_addr *addr);

// send a pong packet
void send_pong(net_addr *addr);

// sent from host to master. give me the list of missions you have.
// this will be used only in a standalone mode
void send_mission_list_request( int what );

// send an individual mission file item
void send_mission_item(net_player *pl,char *file_name,char *mission_name);

// send a request to the server to pause or unpause the game
void send_multi_pause_packet(int pause);

// send an ack packet
void send_ingame_ack(int state,net_player *p);

// send an ingame nak packet
void send_ingame_nak(int state,net_player *p);

void send_ingame_ships_packet(net_player *pl);
void send_ingame_wings_packet(net_player *pl);

// send a notification that a new player has joined the game (if target != NULL, broadcast the packet)
void send_new_player_packet(int new_player_num,net_player *target);

// send a packet telling players to end the mission
void send_endgame_packet(net_player *pl = NULL);

// send a skip to debrief item packet
void send_force_end_mission_packet();

// send a position/orientation update for myself (if I'm an observer)
void send_observer_update_packet();

void send_netplayer_slot_packet();

void send_subsys_update_packet(net_player *p);

void send_ingame_ship_update_packet(net_player *p,ship *sp);

void send_ingame_final_packet(int net_sig);

void send_file_sig_packet(ushort sum_sig,int length_sig);
void send_file_sig_request(char *file_name);

void send_subsystem_destroyed_packet( ship *shipp, int index, vec3d worldpos );

void send_netplayer_load_packet(net_player *pl);

void send_jump_into_mission_packet(net_player *pl = NULL);

void send_repair_info_packet(object *repaired_objp, object *repair_objp, int code );

void send_mission_sync_packet(int mode,int start_campaign = 0);

void send_store_stats_packet(int accept);

void send_debris_create_packet(object *objp, ushort net_signature, int model_num, vec3d exp_center );
void send_debris_update_packet(object *objp,int code);

void send_ship_weapon_change( ship *shipp, int what, int new_bank, int link_status );

// ALAN BEGIN

// send a request from the client to the host of the game (which is not necessarily the server in the case of the standalone) 
// mode == WSS_WEAPON_SELECT  or  WSS_SHIP_SELECT
void send_wss_request_packet(short player_id, int from_slot,int from_index, int to_slot, int to_index, int wl_ship_slot, int ship_class, int mode,net_player *p = NULL);
void process_wss_request_packet(ubyte *data, header *hinfo);

// send the update from the host to the clients
// wss_data is the pointer to a block of data returned by store_wss_stuff(...)
// 
// I would reccomend :
// int store_wss_data(ubyte *block);		// which returns bytes processed
// 
// so you would say :
// 
// ubyte block[MAX_PACKET_SIZE - 10 or so];
// int processed = store_wss_data(block);
// send_wss_update_packet(block,processed);
// 
// also :
// I would reccomend :
// int restore_wss_data(ubyte *block);		// which returns bytes processed
// 
// so I would say in the process_wss_update_packet() :
//
// int processed = restore_wss_data(block);
//	do_other_lowlevel_packet_related_stuff_here();
//
void send_wss_update_packet(int team_num,ubyte *wss_data,int size);
void process_wss_update_packet(ubyte *data, header *hinfo);
// ALAN END

void send_firing_info_packet(void);

void send_sh_transfer_complete_packet(int code);

// packet to tell clients cargo of a ship was revealed to all
void send_cargo_revealed_packet(ship *shipp);

void send_subsystem_cargo_revealed_packet(ship *shipp, int index);

void send_cargo_hidden_packet(ship *shipp);

void send_subsystem_cargo_hidden_packet(ship *shipp, int index);

void send_mission_goal_info_packet(int goal_num, int new_status, int valid);

void send_player_settings_packet(net_player *p = NULL);

void send_deny_packet(net_addr *addr, int code);

void send_post_sync_data_packet(net_player *p = NULL, int std_request = 1);

void send_wss_slots_data_packet(int team_num, int final, net_player *p = NULL, int std_request = 1);

void send_shield_explosion_packet(int objnum, int tri_num, vec3d hit_pos);

void send_player_stats_block_packet(net_player *pl, int stats_type, net_player *target = NULL, short offset = 0);

void send_host_restr_packet(char *callsign, int code, int mode);

void send_netgame_end_error_packet(int notify_code, int err_code);

void send_client_update_packet(net_player *pl);

// send information about this currently active game to the specified address
void send_game_active_packet(net_addr* addr);

void send_ai_info_update_packet(object *objp, char what, object * other_objp = NULL);
void process_ai_info_update_packet(ubyte *data, header *hinfo);

void send_asteroid_create(object *new_objp, object *parent_objp, int asteroid_type, vec3d *relvec);
void send_asteroid_throw(object *objp);
void send_asteroid_hit(object *objp, object *other_objp, vec3d *hitpos, float damage);
void process_asteroid_info(ubyte *data, header *hinfo);

void send_countermeasure_success_packet(int objnum);
void process_countermeasure_success_packet(ubyte *data, header *hinfo);

// host sends a -1 to the server to begin the countdown. server sends an int "seconds until start"
void send_countdown_packet(int time);

void send_debrief_info(int stage_count[], int *stage_active[]);
void process_debrief_info(ubyte *data, header *hinfo);

void send_accept_player_data(net_player *npp, int is_ingame);
void process_accept_player_data(ubyte *data, header *hinfo);

void send_homing_weapon_info(int num);
void process_homing_weapon_info(ubyte *data, header *hinfo);

// emp effect stuff
void send_emp_effect(ushort net_sig, float intensity, float time);
void process_emp_effect(ubyte *data, header *hinfo);

// for reinforcements
void send_reinforcement_avail( int rnum );
void process_reinforcement_avail( ubyte *data, header *hinfo );

// change iff stuff
void send_change_iff_packet(ushort net_signature, int new_team);
void process_change_iff_packet( ubyte *data, header *hinfo );

// change iff color stuff
void send_change_iff_color_packet(ushort net_signature, int observer_team, int observed_team, int alternate_iff_color);
void process_change_iff_color_packet( ubyte *data, header *hinfo );

// change ai class stuff
void send_change_ai_class_packet(ushort net_signature, char *subsystem, int new_ai_class);
void process_change_ai_class_packet( ubyte *data, header *hinfo );

// new primary fired info
void send_NEW_primary_fired_packet(ship *shipp, int banks_fired);
void process_NEW_primary_fired_packet(ubyte *data, header *hinfo);

// new countermeasure fired info
void send_NEW_countermeasure_fired_packet(object *objp, int cmeasure_count, int rand_val);
void process_NEW_countermeasure_fired_packet(ubyte *data, header *hinfo);

// beam weapon packet
void send_beam_fired_packet(object *shooter, ship_subsys *turret, object *target, int beam_info_index, beam_info *override, int bfi_flags, int bank_point);
void process_beam_fired_packet(ubyte *data, header *hinfo);

// sw std query packet
void send_sw_query_packet(ubyte code, char *txt);
void process_sw_query_packet(ubyte *data, header *hinfo);

// event update packet
void send_event_update_packet(int event);
void process_event_update_packet(ubyte *data, header *hinfo);

// variable update packet
void send_variable_update_packet(int variable_index, char *value);
void process_variable_update_packet( ubyte *data, header *hinfo);

// weapons or ammo changed packet
void send_weapon_or_ammo_changed_packet (int ship_index, int bank_type, int bank_number, int ammo_left, int rearm_limit, int new_weapon_index);
void process_weapon_or_ammo_changed_packet( ubyte *data, header *hinfo);

// weapon detonate packet
void send_weapon_detonate_packet(object *objp);
void process_weapon_detonate_packet(ubyte *data, header *hinfo);

// turret fired packet
void send_turret_fired_packet( int objnum, int subsys_index, int weapon_objnum );
void process_turret_fired_packet( ubyte *data, header *hinfo );

// flak fired packet
void send_flak_fired_packet(int ship_objnum, int subsys_index, int weapon_objnum, float flak_range);
void process_flak_fired_packet(ubyte *data, header *hinfo);

// player pain packet
void send_player_pain_packet(net_player *pl, int weapon_info_index, float damage, vec3d *force, vec3d *hitpos, int quadrant_num);
void process_player_pain_packet(ubyte *data, header *hinfo);

// lightning packet
void send_lightning_packet(int bolt_type_internal, vec3d *start, vec3d *strike);
void process_lightning_packet(ubyte *data, header *hinfo);

// bytes sent
void send_bytes_recvd_packet(net_player *pl);
void process_bytes_recvd_packet(ubyte *data, header *hinfo);

// host transfer
void send_host_captain_change_packet(short player_id, int captain_change);
void process_host_captain_change_packet(ubyte *data, header *hinfo);

// self destruct
void send_self_destruct_packet();
void process_self_destruct_packet(ubyte *data, header *hinfo);

void send_sexp_packet(ubyte *sexp_packet, int num_ubytes);
void process_sexp_packet(ubyte *data, header *hinfo);

#endif
