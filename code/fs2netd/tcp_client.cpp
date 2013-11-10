// TCP_Client.cpp
// TCP Client Functions for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################






#include "fs2netd/tcp_client.h"
#include "fs2netd/protocol.h"
#include "fs2netd/tcp_socket.h"
#include "fs2netd/fs2netd_client.h"
#include "network/multi_log.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "io/timer.h"
#include "globalincs/pstypes.h"

#include <iostream>
#include <string>
#include <limits.h>


extern SCP_vector<crc_valid_status> Table_valid_status;
 

int FS2NetD_CheckSingleMission(const char *m_name, uint crc32, bool do_send)
{
	int buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[100];

	if (do_send) {
		INIT_PACKET( PCKT_MISSION_CHECK );

		PXO_ADD_STRING( m_name );
		PXO_ADD_UINT( crc32 );

		DONE_PACKET();

		if ( FS2NetD_SendData(buffer, buffer_size) == -1 ) {
			return 3;
		}
	} else if ( FS2NetD_DataReady() ) {
		int rc;
		uint rc_total = 0;

		do {
			rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer)-rc_total);

			if (rc <= 0) {
				break;
			}

			rc_total += rc;

			Sleep(20);
		} while ( FS2NetD_DataReady() && (rc_total < (int)sizeof(buffer)) );

		if (rc < BASE_PACKET_SIZE) {
			return 0;
		}

		VRFY_PACKET2( PCKT_MCHECK_REPLY );

		if ( !my_packet ) {
			return 0;
		}

		ubyte status = 0;
		PXO_GET_DATA( status );
		Assert( (status == 0) || (status == 1) );

		// anything beyond 'true' is considered a failure of some kind
		if (status > 1) {
			status = 0;
		}

		return status+1;
	}

	return 0;
}

int FS2NetD_SendPlayerData(const char *player_name, player *pl, bool do_send)
{
	int buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[16384];

	if (do_send) {
		int i, num_type_kills = 0;

		// send packet...  (fs2open_pilot_update)
		INIT_PACKET( PCKT_PILOT_UPDATE );

		PXO_ADD_INT( Multi_tracker_id );

		PXO_ADD_STRING( player_name );				// name
		PXO_ADD_STRING( Multi_tracker_login );		// user

		PXO_ADD_INT( pl->stats.score );				// points
		PXO_ADD_UINT( pl->stats.missions_flown );	// missions
		PXO_ADD_UINT( pl->stats.flight_time );		// flighttime
		PXO_ADD_INT( pl->stats.last_flown );		// LastFlight
		PXO_ADD_INT( pl->stats.kill_count );		// Kills
		PXO_ADD_INT( pl->stats.kill_count_ok );		// NonFriendlyKills
		PXO_ADD_INT( pl->stats.assists );			// Assists
		PXO_ADD_UINT( pl->stats.p_shots_fired );	// PriShots
		PXO_ADD_UINT( pl->stats.p_shots_hit );		// PriHits
		PXO_ADD_UINT( pl->stats.p_bonehead_hits );	// PriFHits
		PXO_ADD_UINT( pl->stats.s_shots_fired );	// SecShots
		PXO_ADD_UINT( pl->stats.s_shots_hit );		// SecHits
		PXO_ADD_UINT( pl->stats.s_bonehead_hits );	// SecFHits
		PXO_ADD_INT( pl->stats.rank );				// rank

		for (i = 0; i < MAX_SHIP_CLASSES; i++) {
			if (pl->stats.kills[i] > 0) {
				Assert( (pl->stats.kills[i] >= 0) && (pl->stats.kills[i] < USHRT_MAX) );
				num_type_kills++;
			}
		}

		Assert( (num_type_kills >= 0) && (num_type_kills < USHRT_MAX) );

		PXO_ADD_USHORT( (ushort)num_type_kills );

		for (i = 0; i < MAX_SHIP_CLASSES; i++) {
			if (pl->stats.kills[i] > 0) {
				PXO_ADD_STRING( Ship_info[i].name );
				PXO_ADD_USHORT( (ushort)pl->stats.kills[i] );
			}
		}

		PXO_ADD_USHORT( (ushort)Num_medals );

		for (i = 0; i < Num_medals; i++) {
			PXO_ADD_INT( pl->stats.medal_counts[i] );
		}

		DONE_PACKET();

		if ( FS2NetD_SendData(buffer, buffer_size) == -1 ) {
			return -1;
		}
	} else if ( FS2NetD_DataReady() ) {
		int rc;
		uint rc_total = 0;

		do {
			rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer)-rc_total);

			if (rc <= 0) {
				break;
			}

			rc_total += rc;

			Sleep(20);
		} while ( FS2NetD_DataReady() && (rc_total < (int)sizeof(buffer)) );

		if (rc < BASE_PACKET_SIZE) {
			return -1;
		}

		VRFY_PACKET2( PCKT_PILOT_UREPLY );

		if ( !my_packet ) {
			return -1;
		}

		ubyte status;
		PXO_GET_DATA( status );
		Assert( (status == 0) || (status == 1) || (status == 2) );

		if (status > 2) {
			status = 2;
		}

		return (int)status;
	}

	return -1;
}

int FS2NetD_GetPlayerData(const char *player_name, player *pl, bool can_create, bool do_send)
{
	int buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[16384];

	if (do_send) {
		ubyte create = (ubyte)can_create;

		INIT_PACKET( PCKT_PILOT_GET );

		PXO_ADD_INT( (can_create) ? Multi_tracker_id : -2 );
		PXO_ADD_STRING( player_name );
		PXO_ADD_DATA( create );

		DONE_PACKET();

		if ( FS2NetD_SendData(buffer, buffer_size) == -1 ) {
			return -1;
		}
	} else if ( FS2NetD_DataReady() ) {
		const fix end_time = timer_get_fixed_seconds() + (15 * F1_0);
		int rc;
		uint rc_total = 0;
		ubyte reply_type = 0;
		int si_index = 0;
		ushort bogus, num_type_kills = 0, num_medals = 0;
		char ship_name[NAME_LENGTH];
		int idx;

		do {
			rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer)-rc_total);

			if (rc <= 0) {
				break;
			}

			rc_total += rc;

			Sleep(20);
		} while ( FS2NetD_DataReady() && (rc_total < (int)sizeof(buffer)) );

		if (rc < BASE_PACKET_SIZE) {
			return -1;
		}

		VRFY_PACKET2( PCKT_PILOT_REPLY );

		if ( !my_packet ) {
			return -1;
		}

		if ( buffer_size > (int)sizeof(buffer) ) {
			ml_printf("FS2NetD WARNING: Pilot update data is larger than receive buffer!  Some data will be lost!");
		}

		// make sure that we get the entire packet
		while ( (rc_total < (uint)buffer_size) && (rc_total <= sizeof(buffer)) && (timer_get_fixed_seconds() <= end_time) ) {
			if ( FS2NetD_DataReady() ) {
				rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer) - rc_total);

				if (rc <= 0) {
					continue;
				}

				rc_total += rc;
			}

			Sleep(20);
		}

		PXO_GET_DATA( reply_type );

		// if we weren't retrieved then bail out now
		if (reply_type != 0) {
			return (int)reply_type;
		}

		PXO_GET_INT( pl->stats.score );				// points
		PXO_GET_UINT( pl->stats.missions_flown );	// missions
		PXO_GET_UINT( pl->stats.flight_time );		// flighttime
		PXO_GET_INT( pl->stats.last_flown );		// LastFlight
		PXO_GET_INT( pl->stats.kill_count );		// Kills
		PXO_GET_INT( pl->stats.kill_count_ok  );	// NonFriendlyKills
		PXO_GET_INT( pl->stats.assists );			// Assists
		PXO_GET_UINT( pl->stats.p_shots_fired );	// PriShots
		PXO_GET_UINT( pl->stats.p_shots_hit );		// PriHits
		PXO_GET_UINT( pl->stats.p_bonehead_hits );	// PriFHits
		PXO_GET_UINT( pl->stats.s_shots_fired );	// SecShots
		PXO_GET_UINT( pl->stats.s_shots_hit );		// SecHits
		PXO_GET_UINT( pl->stats.s_bonehead_hits );	// SecFHits
		PXO_GET_INT( pl->stats.rank );				// rank

		PXO_GET_USHORT( num_type_kills );

		for (idx = 0; idx < (int)num_type_kills; idx++) {
			memset( ship_name, 0, sizeof(ship_name) );

			PXO_GET_STRING( ship_name );

			si_index = ship_info_lookup( ship_name );

			if (si_index == -1) {
				PXO_GET_USHORT( bogus );
			} else {
				PXO_GET_USHORT( pl->stats.kills[si_index] );
			}
		}

		PXO_GET_USHORT( num_medals );

		for (idx = 0; (idx < Num_medals) && (idx < num_medals); idx++) {
			PXO_GET_INT( pl->stats.medal_counts[idx] );
		}

		return (int)reply_type;
	}

	return -1;
}

int FS2NetD_GetBanList(SCP_vector<SCP_string> &mask_list, bool do_send)
{
	int buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[16384];

	if (do_send) {
		INIT_PACKET( PCKT_BANLIST_RQST );
		DONE_PACKET();

		if ( FS2NetD_SendData(buffer, buffer_size) == -1 ) {
			return -1;
		}
	} else if ( FS2NetD_DataReady() ) {
		const fix end_time = timer_get_fixed_seconds() + (15 * F1_0);
		int rc;
		uint rc_total = 0;
		int num_files = 0;
		char ip_mask[32];
		int idx;

		do {
			rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer)-rc_total);

			if (rc <= 0) {
				break;
			}

			rc_total += rc;

			Sleep(20);
		} while ( FS2NetD_DataReady() && (rc_total < (int)sizeof(buffer)) );

		if (rc < BASE_PACKET_SIZE) {
			return -1;
		}

		VRFY_PACKET2( PCKT_BANLIST_RPLY );

		if ( !my_packet ) {
			return 0;
		}

		if ( buffer_size > (int)sizeof(buffer) ) {
			ml_printf("FS2NetD WARNING: Banned user list data is larger than receive buffer!  Some data will be lost!");
		}

		// make sure that we get the entire packet
		while ( (rc_total < (uint)buffer_size) && (rc_total <= sizeof(buffer)) && (timer_get_fixed_seconds() <= end_time) ) {
			if ( FS2NetD_DataReady() ) {
				rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer) - rc_total);

				if (rc <= 0) {
					continue;
				}

				rc_total += rc;
			}

			Sleep(20);
		}

		PXO_GET_INT( num_files );

		for (idx = 0; idx < num_files; idx++) {
			PXO_GET_STRING( ip_mask );
			mask_list.push_back( ip_mask );
		}
	
		return 1;
	}

	return 0;
}

int FS2NetD_GetMissionsList(SCP_vector<file_record> &m_list, bool do_send)
{
	int buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[16384];

	if (do_send) {
		INIT_PACKET( PCKT_MISSIONS_RQST );
		DONE_PACKET();

		if ( FS2NetD_SendData(buffer, buffer_size) == -1 ) {
			return -1;
		}
	} else if ( FS2NetD_DataReady() ) {
		const fix end_time = timer_get_fixed_seconds() + (15 * F1_0);
		int rc;
		uint rc_total = 0;
		int i, num_files = 0;
		file_record nrec;

		do {
			rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer)-rc_total);

			if (rc <= 0) {
				break;
			}

			rc_total += rc;

			Sleep(20);
		} while ( FS2NetD_DataReady() && (rc_total < (int)sizeof(buffer)) );

		if (rc < BASE_PACKET_SIZE) {
			return 0;
		}

		VRFY_PACKET2( PCKT_MISSIONS_REPLY );

		if ( !my_packet ) {
			return 0;
		}

		if ( buffer_size > (int)sizeof(buffer) ) {
			ml_printf("FS2NetD WARNING: Mission list data is larger than receive buffer!  Some data will be lost!");
		}

		// make sure that we get the entire packet
		while ( (rc_total < (uint)buffer_size) && (rc_total <= sizeof(buffer)) && (timer_get_fixed_seconds() <= end_time) ) {
			if ( FS2NetD_DataReady() ) {
				rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer) - rc_total);

				if (rc <= 0) {
					continue;
				}

				rc_total += rc;
			}

			Sleep(20);
		}

		PXO_GET_INT( num_files );

		for (i = 0; i < num_files; i++) {
			memset(&nrec, 0, sizeof(file_record));

			PXO_GET_STRING( nrec.name );
			PXO_GET_UINT( nrec.crc32 );

			m_list.push_back( nrec );
		}

		return 1;
	}

	return 0;
}

int FS2NetD_Login(const char *username, const char *password, bool do_send)
{
	int buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[150];

	if (do_send) {
		INIT_PACKET( PCKT_LOGIN_AUTH );

		PXO_ADD_STRING( username );
		PXO_ADD_STRING( password );
		PXO_ADD_USHORT( Multi_options_g.port );

		DONE_PACKET();

		if (FS2NetD_SendData(buffer, buffer_size) == -1) {
			return -1;
		}
	} else if ( FS2NetD_DataReady() ) {
		int rc;
		uint rc_total = 0;
		ubyte login_status = 0;
		int sid;
		short pilots;

		do {
			rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer)-rc_total);

			if (rc <= 0) {
				break;
			}

			rc_total += rc;

			Sleep(20);
		} while ( FS2NetD_DataReady() && (rc_total < (int)sizeof(buffer)) );

		if (rc < BASE_PACKET_SIZE) {
			return -1;
		}

		VRFY_PACKET2( PCKT_LOGIN_REPLY );

		if ( !my_packet ) {
			return -1;
		}
			
		PXO_GET_DATA( login_status );

		if ( !login_status ) {
			return -2;
		}

		PXO_GET_INT( sid );

		PXO_GET_SHORT( pilots );

		return sid;
	}

	return -1;
}

void FS2NetD_SendServerStart()
{
	int buffer_size;
	char buffer[550];
	ubyte tvar;

	INIT_PACKET( PCKT_SERVER_START );

	PXO_ADD_STRING( Netgame.name );
	PXO_ADD_STRING( Netgame.mission_name );
	PXO_ADD_STRING( Netgame.title );
	PXO_ADD_STRING( Netgame.campaign_name );

	tvar = (ubyte)Netgame.campaign_mode;
	PXO_ADD_DATA( tvar );

	PXO_ADD_INT( Netgame.flags );
	PXO_ADD_INT( Netgame.type_flags );

	PXO_ADD_SHORT( (short)multi_num_players() );
	PXO_ADD_SHORT( (short)Netgame.max_players );

	tvar = (ubyte)Netgame.mode;
	PXO_ADD_DATA( tvar );

	tvar = (ubyte)Netgame.rank_base;
	PXO_ADD_DATA( tvar );

	tvar = (ubyte)Netgame.game_state;
	PXO_ADD_DATA( tvar );

	tvar = (ubyte)multi_get_connection_speed();
	PXO_ADD_DATA( tvar );

	PXO_ADD_STRING(Multi_fs_tracker_channel);

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

void FS2NetD_SendServerUpdate()
{
	int buffer_size;
	char buffer[550];
	ubyte tvar;

	INIT_PACKET( PCKT_SERVER_UPDATE );

	PXO_ADD_STRING( Netgame.mission_name );
	PXO_ADD_STRING( Netgame.title );
	PXO_ADD_STRING( Netgame.campaign_name );

	tvar = (ubyte)Netgame.campaign_mode;
	PXO_ADD_DATA( tvar );

	PXO_ADD_SHORT( (short)multi_num_players() );

	tvar = (ubyte)Netgame.game_state;
	PXO_ADD_DATA( tvar );

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

void FS2NetD_SendServerDisconnect()
{
	int buffer_size;
	char buffer[BASE_PACKET_SIZE];

	INIT_PACKET( PCKT_SERVER_DISCONNECT );

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

void FS2NetD_RequestServerList()
{
	int buffer_size;
	char buffer[BASE_PACKET_SIZE+sizeof(int)+sizeof(int)+sizeof(int)+MAX_PATH];
	int all = 0xFFFFFFFF;
	bool filtered = false;

	if ( strlen(Multi_fs_tracker_filter) ) {
		filtered = true;
	}

	// send request packet
	INIT_PACKET( (filtered) ? PCKT_SLIST_REQUEST_FILTER : PCKT_SLIST_REQUEST );

	PXO_ADD_INT( all );	// type
	PXO_ADD_INT( all );	// status

	if (filtered) {
		PXO_ADD_STRING(Multi_fs_tracker_filter);
	}

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

void FS2NetD_Ping()
{
	int buffer_size;
	char buffer[BASE_PACKET_SIZE+sizeof(int)];

	INIT_PACKET( PCKT_PING );

	int time = timer_get_milliseconds();
	PXO_ADD_INT( time );

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

void FS2NetD_Pong(int tstamp)
{
	int buffer_size;
	char buffer[BASE_PACKET_SIZE+sizeof(int)];

	INIT_PACKET( PCKT_PONG );

	PXO_ADD_INT( tstamp );

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

int FS2NetD_CheckValidID()
{
	int buffer_size;
	char buffer[BASE_PACKET_SIZE+sizeof(int)];

	// create and send request packet
	INIT_PACKET( PCKT_VALID_SID_RQST );

	PXO_ADD_INT( Multi_tracker_id );

	DONE_PACKET();

	if (FS2NetD_SendData(buffer, buffer_size) == -1) {
		return -1;
	}

	return 0;
}

int FS2NetD_ValidateTableList(bool do_send)
{
	int buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[4096];
	ushort num_tables = 0;

	if (do_send) {
		// create and send the request packet
		INIT_PACKET( PCKT_TABLES_RQST );

		num_tables = (ushort)Table_valid_status.size();

		PXO_ADD_USHORT( num_tables );

		for (SCP_vector<crc_valid_status>::iterator tvs = Table_valid_status.begin(); tvs != Table_valid_status.end(); ++tvs) {
			PXO_ADD_STRING(tvs->name );
			PXO_ADD_UINT( tvs->crc32 );
		}

		DONE_PACKET();

		if (FS2NetD_SendData(buffer, buffer_size) == -1) {
			return -1;
		}
	} else if ( FS2NetD_DataReady() ) {
		int rc;
		const fix end_time = timer_get_fixed_seconds() + (15 * F1_0);
		ubyte tbl_valid_status = 0;
		uint rc_total = 0;

		do {
			rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer)-rc_total);

			if (rc <= 0) {
				break;
			}

			rc_total += rc;

			Sleep(20);
		} while ( FS2NetD_DataReady() && (rc_total < (int)sizeof(buffer)) );

		if (rc < BASE_PACKET_SIZE) {
			return -1;
		}

		VRFY_PACKET2( PCKT_TABLES_REPLY );

		if ( !my_packet ) {
			return -1;
		}

		// make sure that we get the entire packet
		while ( (rc_total < (uint)buffer_size) && (rc_total < sizeof(buffer)) && (timer_get_fixed_seconds() <= end_time) ) {
			if ( FS2NetD_DataReady() ) {
				rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer) - rc_total);

				if (rc <= 0) {
					continue;
				}

				rc_total += rc;
			}

			Sleep(20);
		}

		PXO_GET_USHORT( num_tables );

		if ( !num_tables ) {
			return -1;
		}

		if ( num_tables > (int)Table_valid_status.size() ) {
			ml_printf("FS2NetD WARNING: Table list contains %i tables, but we only requested %i!  Invalid data!", num_tables, Table_valid_status.size());
			return -1;
		}

		for (SCP_vector<crc_valid_status>::iterator tvs = Table_valid_status.begin(); tvs != Table_valid_status.end(); ++tvs) {
			PXO_GET_DATA( tbl_valid_status );
			Assert( (tbl_valid_status == 0) || (tbl_valid_status == 1) );

			tvs->valid = tbl_valid_status;
		}

		return 2;
	}

	return 0;
}

void FS2NetD_GameCountUpdate(const char *chan_name)
{
	int buffer_size;
	char buffer[MAX_PATH+BASE_PACKET_SIZE+10];

	INIT_PACKET( PCKT_CHAT_CHAN_COUNT_RQST );

	PXO_ADD_STRING( chan_name );

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

void FS2NetD_CheckDuplicateLogin()
{
	int buffer_size;
	char buffer[BASE_PACKET_SIZE + sizeof(int) + sizeof(ubyte) + (MAX_PLAYERS * sizeof(int)) + 10];
	int ids_count = 0;
	int *ids = new int[MAX_PLAYERS];
	int idx;

	if ( !ids ) {
		return;
	}

	for (idx = 0; idx < MAX_PLAYERS; idx++) {
		if ( MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) ) {      
			if ( (Net_players[idx].tracker_player_id >= 0) && (Net_players[idx].tracker_player_id != Multi_tracker_id) ) {
				ids[ids_count] = Net_players[idx].tracker_player_id;
				ids_count++;
			}
		}
	}

	if ( !ids_count ) {
		delete [] ids;
		return;
	}

	INIT_PACKET( PCKT_DUP_LOGIN_RQST );

	PXO_ADD_INT( Multi_tracker_id );

	Assert( MAX_PLAYERS <= 255 );

	ubyte tvar = (ubyte)ids_count;
	PXO_ADD_DATA( tvar );

	for (idx = 0; idx < ids_count; idx++) {
		PXO_ADD_INT( ids[idx] );
	}

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);

	delete [] ids;
}

