// TCP_Client.cpp
// TCP Client Functions for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################


/*
 * $Logfile: /Freespace2/code/fs2open_pxo/TCP_Client.cpp $
 * $Revision: 1.1.2.1 $
 * $Date: 2007-10-15 06:43:10 $
 * $Author: taylor $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.35  2006/01/26 03:23:29  Goober5000
 * pare down the pragmas some more
 * --Goober5000
 *
 * Revision 1.34  2006/01/20 07:10:33  Goober5000
 * reordered #include files to quash Microsoft warnings
 * --Goober5000
 *
 * Revision 1.33  2005/12/29 08:08:33  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 1.32  2005/10/10 17:21:04  taylor
 * remove NO_NETWORK
 *
 * Revision 1.31  2005/07/13 02:50:49  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.30  2005/06/29 18:49:37  taylor
 * various FS2NetD fixes:
 *  - replace timer stuff with something that more accurately works cross-platform and without being affected by load
 *  - better sanity checking for the server list
 *  - working Linux compatibility that's not dog slow
 *  - when calling DataReady() make sure that the data is properly valid
 *  - fix messed up cvs merge cleanup from the Linux merge which did nasty things
 *
 * Revision 1.29  2005/06/21 00:13:47  taylor
 * add some better error checking/handling for when GetServerList messes up
 *
 * Revision 1.28  2005/05/08 20:28:57  wmcoolmon
 * Dynamically allocated medals
 *
 * Revision 1.27  2005/03/08 03:50:25  Goober5000
 * edited for language ;)
 * --Goober5000
 *
 * Revision 1.26  2005/03/02 21:18:18  taylor
 * better support for Inferno builds (in PreProcDefines.h now, no networking support)
 * make sure NO_NETWORK builds are as friendly on Windows as it is on Linux/OSX
 * revert a timeout in Client.h back to the original value before Linux merge
 *
 * Revision 1.25  2005/02/23 13:17:04  taylor
 * few more compiler warning fixes (didn't mean to commit iostream.h change)
 * lower warning level to 3 to stop MSVC6 from complaining about C++ headers
 *
 * Revision 1.24  2005/02/23 05:05:37  taylor
 * compiler warning fixes (for MSVC++ 6)
 * have the warp effect only load as many LODs as will get used
 * head off strange bug in release when corrupt soundtrack number gets used
 *    (will still Assert in debug)
 * don't ever try and save a campaign savefile in multi or standalone modes
 * first try at 32bit->16bit color conversion for TGA code (for TGA only ship textures)
 *
 * Revision 1.23  2005/02/04 20:06:03  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 1.22  2004/11/18 00:05:36  Goober5000
 * #pragma'd a bunch of warnings
 * --Goober5000
 *
 * Revision 1.21  2004/07/26 20:47:29  Kazan
 * remove MCD complete
 *
 * Revision 1.20  2004/07/12 16:32:46  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.19  2004/07/09 22:05:32  Kazan
 * fs2netd 1.0 RC5 full support - Rank and Medal updates
 *
 * Revision 1.18  2004/07/07 21:00:06  Kazan
 * FS2NetD: C2S Ping/Pong, C2S Ping/Pong, Global IP Banlist, Global Network Messages
 *
 * Revision 1.17  2004/07/06 23:45:34  Kazan
 * minor multi fix
 *
 * Revision 1.16  2004/05/25 00:21:39  wmcoolmon
 * Updated to use <iostream> instead of <iostream.h>
 *
 * Revision 1.15  2004/03/31 05:42:26  Goober5000
 * got rid of all those nasty warnings from xlocale and so forth; also added comments
 * to indicate which warnings were being disabled
 * --Goober5000
 *
 * Revision 1.14  2004/03/09 17:59:01  Kazan
 * Disabled multithreaded TCP_Socket in favor of safer single threaded
 * FS2NetD doesn't kill the game on connection failure now - just gives warning message and effectively dsiables itself until they try to connect again
 *
 * Revision 1.13  2004/03/08 15:06:23  Kazan
 * Did, undo
 *
 * Revision 1.12  2004/03/07 23:07:20  Kazan
 * [Incomplete] Readd of Software renderer so Standalone server works
 *
 * Revision 1.11  2004/03/05 21:19:39  Kazan
 * Fixed mission validation (was returning false positives)
 *
 * Revision 1.10  2004/03/05 09:01:56  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 1.9  2004/02/21 00:59:43  Kazan
 * FS2NETD License Comments
 *
 * Revision 1.8  2004/02/04 09:02:42  Goober5000
 * got rid of unnecessary double semicolons
 * --Goober5000
 *
 * Revision 1.7  2003/11/13 03:59:52  Kazan
 * PXO_SID changed from unsigned to signed
 *
 * Revision 1.6  2003/11/11 02:15:42  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 1.5  2003/11/09 04:09:17  Goober5000
 * edited for language
 * --Goober5000
 *
 * Revision 1.4  2003/11/06 20:22:05  Kazan
 * slight change to .dsp - leave the release target as fs2_open_r.exe already
 * added myself to credit
 * killed some of the stupid warnings (including doing some casting and commenting out unused vars in the graphics modules)
 * Release builds should have warning level set no higher than 2 (default is 1)
 * Why are we getting warning's about function selected for inline expansion... (killing them with warning disables)
 * FS2_SPEECH was not defined (source file doesn't appear to capture preproc defines correctly either)
 *
 * Revision 1.3  2003/10/13 05:57:47  Kazan
 * Removed a bunch of Useless *_printf()s in the rendering pipeline that were just slowing stuff down
 * Commented out the "warning null vector in vector normalize" crap
 * Added "beam no whack" flag for beams - said beams NEVER whack
 * Some reliability updates in FS2NetD
 *
 *
 *
 */



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
#include <vector>
#include <string>
#include <limits.h>


extern std::vector<crc_valid_status> Table_valid_status;
 

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

		PXO_ADD_USHORT( (ushort)MAX_MEDALS );

		for (i = 0; i < MAX_MEDALS; i++) {
			PXO_ADD_INT( pl->stats.medals[i] );
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

		// if we weren't retrieved or created then bail out now
		if (reply_type > 1) {
			return (int)reply_type;
		}
	
		// initialize the stats to default values
		init_scoring_element( &pl->stats );

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

		for (idx = 0; (idx < MAX_MEDALS) && (idx < num_medals); idx++) {
			PXO_GET_INT( pl->stats.medals[idx] );
		}

		return (int)reply_type;
	}

	return -1;
}

int FS2NetD_GetBanList(std::vector<std::string> &mask_list, bool do_send)
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

int FS2NetD_GetMissionsList(std::vector<file_record> &m_list, bool do_send)
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
	PXO_ADD_SHORT( Netgame.max_players );

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
	char buffer[1024];
	uint i;
	ushort num_tables = 0;

	if (do_send) {
		// create and send the request packet
		INIT_PACKET( PCKT_TABLES_RQST );

		num_tables = (ushort)Table_valid_status.size();

		PXO_ADD_USHORT( num_tables );

		for (i = 0; i < Table_valid_status.size(); i++) {
			PXO_ADD_STRING( Table_valid_status[i].name );
			PXO_ADD_UINT( Table_valid_status[i].crc32 );
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

		PXO_GET_USHORT( num_tables );

		if ( !num_tables ) {
			return -1;
		}

		if ( num_tables > (int)Table_valid_status.size() ) {
			ml_printf("FS2NetD WARNING: Table list contains %i tables, but we only requested %i!  Invalid data!", num_tables, Table_valid_status.size());
			return -1;
		}

		for (i = 0; i < Table_valid_status.size(); i++) {
			PXO_GET_DATA( tbl_valid_status );
			Assert( (tbl_valid_status == 0) || (tbl_valid_status == 1) );

			Table_valid_status[i].valid = tbl_valid_status;
		}

		return 2;
	}

	return 0;
}

void FS2NetD_GameCountUpdate(char *chan_name)
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

