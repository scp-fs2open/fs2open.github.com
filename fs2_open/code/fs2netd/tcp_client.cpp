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
#include "network/multi_log.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "io/timer.h"
#include "globalincs/pstypes.h"

#include <iostream>
#include <vector>

#define MAX_TIMEOUT		10
#define MIN_TIMEOUT		5

extern std::vector<crc_valid_status> Table_valid_status;


int FS2NetD_CheckSingleMission(const char *m_name, uint crc32, int timeout)
{
	int rc, buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[100];

	if (timeout == 0)
		goto Recieve_Only;


	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// send packet...  (fs2open_file_check_single)
	INIT_PACKET( PCKT_MISSION_CHECK );

	PXO_ADD_STRING( m_name );
	PXO_ADD_UINT( crc32 );

	DONE_PACKET();

	if ( FS2NetD_SendData(buffer, buffer_size) == -1 )
		return 3;


Recieve_Only:

	// and get the reply...  (fs2open_fcheck_reply)
	ubyte status = 0;

	if ( (rc = FS2NetD_GetData(buffer, sizeof(buffer))) != -1 ) {
		if (rc < BASE_PACKET_SIZE)
			return 0;

		VRFY_PACKET2( PCKT_MCHECK_REPLY );

		if (!my_packet)
			return 0;

		PXO_GET_DATA( status );
		Assert( (status == 0) || (status == 1) );

		// anything beyond 'true' is considered a failure of some kind
		if (status > 1)
			status = 0;

		return status+1;
	}

	return 0;
}

int FS2NetD_SendPlayerData(int SID, const char *player_name, const char *user, player *pl, int timeout)
{
	int rc, buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[16384]; // 16K should be enough i think..... I HOPE!
	int i, num_type_kills = 0;


	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// send packet...  (fs2open_pilot_update)
	INIT_PACKET( PCKT_PILOT_UPDATE );

	PXO_ADD_INT( SID );

	PXO_ADD_STRING( player_name );				// name
	PXO_ADD_STRING( user );						// user

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

	fs2open_ship_typekill *type_kills = NULL;
	type_kills = (fs2open_ship_typekill*) vm_malloc( sizeof(fs2open_ship_typekill) * MAX_SHIP_CLASSES );
	Verify( type_kills != NULL );

	for (i = 0; i < MAX_SHIP_CLASSES; i++) {
		if (pl->stats.kills[i] <= 0)
			continue;

		strcpy(type_kills[num_type_kills].name, Ship_info[i].name);

		Assert( (pl->stats.kills[i] >= 0) && (pl->stats.kills[i] < USHRT_MAX) );
		type_kills[num_type_kills].kills = (ushort)pl->stats.kills[i];

		num_type_kills++;
	}

	Assert( (num_type_kills >= 0) && (num_type_kills < USHRT_MAX) );
	
	PXO_ADD_USHORT( (ushort)num_type_kills );

	for (i = 0; i < num_type_kills; i++) {
		PXO_ADD_STRING( type_kills[i].name );
		PXO_ADD_USHORT( type_kills[i].kills );
	}

	PXO_ADD_USHORT( (ushort)MAX_MEDALS );

	for (i = 0; i < MAX_MEDALS; i++)
		PXO_ADD_INT( pl->stats.medals[i] );

	DONE_PACKET();

	if ( FS2NetD_SendData(buffer, buffer_size) == -1 )
		return -1;


	// get reply (fs2open_pilot_updatereply)
	fix end_time = timer_get_fixed_seconds() + (MIN_TIMEOUT * F1_0);
	ushort status;

	while ( timer_get_fixed_seconds() <= end_time ) {
		if ( (rc = FS2NetD_GetData(buffer, sizeof(buffer))) != -1 ) {
			if (rc < BASE_PACKET_SIZE)
				continue;

			VRFY_PACKET( PCKT_PILOT_UREPLY );

			if (!my_packet)
				continue;

			PXO_GET_DATA( status );
			Assert( (status == 0) || (status == 1) || (status == 2) );

			if (status > 2)
				status = 2;

			return (int)status;
		}
	}

	return -1;
}

int FS2NetD_GetPlayerData(int SID, const char *player_name, player *pl, bool CanCreate, int timeout)
{
	int rc, buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[16384]; // 16K should be enough i think..... I HOPE!
	int i;
	ubyte create = (ubyte)CanCreate;


	if (timeout == 0)
		goto Recieve_Only;


	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// send packet...  (fs2open_get_pilot)
	INIT_PACKET( PCKT_PILOT_GET );

	PXO_ADD_INT( SID );
	PXO_ADD_STRING( player_name );
	PXO_ADD_DATA( create );

	DONE_PACKET();

	if ( FS2NetD_SendData(buffer, buffer_size) == -1 )
		return -1;


	Sleep(10); // give it a little time to process


Recieve_Only:

	// process the received pilot update data (fs2open_pilot_reply)
	fix end_time = timer_get_fixed_seconds() + (15 * F1_0);
	int si_index = 0;
	uint rc_total = 0;
	ubyte reply_type = 0;
	ushort bogus, num_type_kills = 0, num_medals = 0;
	char ship_name[NAME_LENGTH];

	if ( (rc = FS2NetD_GetData(buffer, sizeof(buffer))) != -1 ) {
		if (rc < BASE_PACKET_SIZE)
			return -1;

		VRFY_PACKET2( PCKT_PILOT_REPLY );

		if (!my_packet)
			return -1;

		if ( buffer_size > (int)sizeof(buffer) )
			ml_printf("FS2NetD WARNING: Pilot update data is larger than receive buffer!  Some data will be lost!");

		rc_total = rc;

		ml_printf("FS2NetD: Pre-completion pilot get: recvsize = %i, expecting = %i", rc, buffer_size);

		while ( (rc_total < (uint)buffer_size) && (timer_get_fixed_seconds() <= end_time) ) {
			if ( (rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer) - rc_total)) != -1 ) {
				rc_total += rc;
				ml_printf("FS2NetD: Pilot get completion: got an additional %i bytes!", rc);

				if ( rc_total >= sizeof(buffer) )
					break;
			}
		}

		ml_printf("FS2NetD: Post-completion pilot get: recvsize = %i, expected = %i", rc_total, buffer_size);

		if ( timer_get_fixed_seconds() >= end_time ) {
			ml_printf("FS2NetD: Pilot get transfer completetion timed out!");
			return -1;
		}

		PXO_GET_DATA( reply_type );

		// if we weren't retrieved or created then bail out now
		if (reply_type > 1)
			return (int)reply_type;
	
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

		for (i = 0; i < (int)num_type_kills; i++) {
			memset( ship_name, 0, sizeof(ship_name) );

			PXO_GET_STRING( ship_name );

			si_index = ship_info_lookup( ship_name );

			if (si_index == -1)
				PXO_GET_USHORT( bogus );
			else
				PXO_GET_USHORT( pl->stats.kills[si_index] );
		}

		PXO_GET_USHORT( num_medals );

		for (i = 0; (i < MAX_MEDALS) && (i < num_medals); i++)
			PXO_GET_INT( pl->stats.medals[i] );

		return (int)reply_type;
	}

	return -1;
}

fs2open_banmask *FS2NetD_GetBanList(int *numBanMasks, int timeout)
{
	int rc, buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[16384];	// 16K should be enough i think..... I HOPE!

	if (timeout == 0)
		goto Recieve_Only;

	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// send request packet (fs2open_file_check)
	INIT_PACKET( PCKT_BANLIST_RQST );
	DONE_PACKET();

	if ( FS2NetD_SendData(buffer, buffer_size) == -1 )
		return NULL;


	Sleep(5); // lets give it a second


Recieve_Only:

	// process received ban list (fs2open_banlist_reply)
	fix end_time = timer_get_fixed_seconds() + (MIN_TIMEOUT * F1_0);
	fs2open_banmask *masks = NULL;
	int i, num_files = 0;
	uint rc_total = 0;

	if ( (rc = FS2NetD_GetData(buffer, sizeof(buffer))) != -1 ) {
		if (rc <= BASE_PACKET_SIZE)
			return NULL;

		VRFY_PACKET2( PCKT_BANLIST_RPLY );

		if (!my_packet)
			return NULL;

		if ( buffer_size > (int)sizeof(buffer) )
			ml_printf("FS2NetD WARNING: Banned user list data is larger than receive buffer!  Some data will be lost!");

		PXO_GET_INT( num_files );

		rc_total = rc;

		ml_printf("FS2NetD: Pre-completion banlist get: recvsize = %i, num_ban_masks = %i...", rc, num_files);

		while ( (rc_total < (uint)buffer_size) && (timer_get_fixed_seconds() <= end_time) ) {
			if ( (rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer) - rc_total)) != -1 ) {
				rc_total += rc;
				ml_printf("FS2NetD: Banlist completion: got an additional %i bytes!", rc);

				if ( rc_total >= sizeof(buffer) )
					break;
			}
		}

		ml_printf("FS2NetD: Post-completion banlist get: recvsize = %i", rc_total);

		if ( timer_get_fixed_seconds() >= end_time ) {
			ml_printf("FS2NetD: Banlist transfer completetion timed out!");
			return NULL;
		}

		masks = new fs2open_banmask[num_files];

		fs2open_banmask *nrecord = &masks[0];

		for (i = 0; i < num_files; i++, nrecord++) {
			PXO_GET_STRING( nrecord->ip_mask );

		//	ml_printf("FS2NetD: Banlist[%i] = { \"%s\" }", i, nrecord->ip_mask);
		}

		*numBanMasks = num_files;
	
		return masks;
	}

	return NULL;
}

file_record *FS2NetD_GetMissionsList(int *num_missions, int timeout)
{
	int rc, buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[16384];	// 16K should be enough i think..... I HOPE!

	if (timeout == 0)
		goto Recieve_Only;

	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// send request packet (fs2open_file_check)
	INIT_PACKET( PCKT_MISSIONS_RQST );
	DONE_PACKET();

	if ( FS2NetD_SendData(buffer, buffer_size) == -1 )
		return NULL;


	Sleep(10); // lets give it a second


Recieve_Only:

	// process received mission data (fs2open_pxo_missreply)
	fix end_time = timer_get_fixed_seconds() + (MAX_TIMEOUT * F1_0);
	file_record *frecs = NULL;
	int i, num_files = 0;
	uint rc_total = 0;

	if ( (rc = FS2NetD_GetData(buffer, sizeof(buffer))) != -1 ) {
		if (rc < BASE_PACKET_SIZE)
			return NULL;

		VRFY_PACKET2( PCKT_MISSIONS_REPLY );

		if (!my_packet)
			return NULL;

		if ( buffer_size > (int)sizeof(buffer) )
			ml_printf("FS2NetD WARNING: Mission list data is larger than receive buffer!  Some data will be lost!");

		PXO_GET_INT( num_files );

		rc_total = rc;

		ml_printf("FS2NetD: Pre-completion missions get: recvsize = %i, num_missions = %i...", rc, num_files);

		while ( (rc_total < (uint)buffer_size) && (timer_get_fixed_seconds() <= end_time) ) {
			if ( (rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer) - rc_total)) != -1) {
				rc_total += rc;
				ml_printf("FS2NetD: Missions completion: got an additional %i bytes!", rc);

				if ( rc_total >= sizeof(buffer) )
					break;
			}
		}

		ml_printf("FS2NetD: Post-completion missions get: recvsize = %i", rc_total);

		if ( timer_get_fixed_seconds() >= end_time ) {
			ml_printf("FS2NetD: Missions transfer completetion timed out!");
			return NULL;
		}

		frecs = new file_record[num_files];

		file_record *nrecord = &frecs[0];

		for (i = 0; i < num_files; i++, nrecord++) {
			PXO_GET_STRING( nrecord->name );
			PXO_GET_UINT( nrecord->crc32 );

		//	ml_printf("FS2NetD: Missions[%i] = { \"%s\", 0x%08x }", i, nrecord->name, nrecord->crc32);
		}

		*num_missions = num_files;

		return frecs;
	}

	return NULL;
}

int FS2NetD_Login(const char *username, const char *password, int timeout)
{
	int rc, buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[150];

	if ( timeout == 0 )
		goto Recieve_Only;

	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// create and send login packet (fs2open_pxo_login)
	INIT_PACKET( PCKT_LOGIN_AUTH );

	PXO_ADD_STRING( username );
	PXO_ADD_STRING( password );
	PXO_ADD_USHORT( Multi_options_g.port );

	DONE_PACKET();

	if (FS2NetD_SendData(buffer, buffer_size) == -1)
		return -1;

	
Recieve_Only:

	// await reply (fs2open_pxo_lreply)
	ubyte login_status = 0;
	int sid;
	short pilots;

	if ( (rc = FS2NetD_GetData(buffer, sizeof(buffer))) != -1 ) {
		if (rc < BASE_PACKET_SIZE)
			return -1;

		VRFY_PACKET2( PCKT_LOGIN_REPLY );

		if (!my_packet)
			return -1;
			
		PXO_GET_DATA( login_status );

		if (!login_status)
			return -2;

		PXO_GET_INT( sid );

		PXO_GET_SHORT( pilots );

		return sid;
	}

	return -1;
}

void FS2NetD_SendHeartBeat()
{
	int buffer_size;
	char buffer[550];
	ubyte tvar;

	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// create and send hb packet  (serverlist_hb_packet)
	INIT_PACKET( PCKT_SLIST_HB_2 );

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

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

void FS2NetD_SendServerDisconnect(ushort port)
{
	int buffer_size;
	char buffer[100];

	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// create and send hb packet  (serverlist_hb_packet)
	INIT_PACKET( PCKT_SLIST_DISCONNECT );

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

int FS2NetD_GetServerList(int timeout)
{
	int rc, buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[5000]; // packet is 30 bytes max, and we need space for 150 servers
	int all = 0xFFFFFFFF;

	if (timeout == 0)
		goto Recieve_Only;

	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// send request packet (serverlist_request_packet)
	INIT_PACKET( PCKT_SLIST_REQUEST );

	PXO_ADD_INT( all );	// type
	PXO_ADD_INT( all );	// status

	DONE_PACKET();

	if ( FS2NetD_SendData(buffer, buffer_size) == -1 )
		return 2;
	

Recieve_Only:

	// receive reply (serverlist_reply_packet)
	fix end_time = timer_get_fixed_seconds() + (MAX_TIMEOUT * F1_0);
	net_server templist[MAX_SERVERS];
	int i, numServers = 0;
	uint rc_total = 0;
	net_addr addr;
	server_item *item = NULL;

	memset( templist, 0, MAX_SERVERS * sizeof(net_server) );

	if ( (rc = FS2NetD_GetData(buffer, sizeof(buffer))) != -1 ) {
		if (rc < BASE_PACKET_SIZE)
			return 0;

		VRFY_PACKET2( PCKT_SLIST_REPLY );

		if (!my_packet)
			return 0;

		if ( buffer_size > (int)sizeof(buffer) )
			ml_printf("FS2NetD WARNING: Server list data is larger than receive buffer!  Some data will be lost!");

		PXO_GET_USHORT( numServers );

		if (!numServers)
			return 1;

		rc_total = rc;

		ml_printf("FS2NetD: Pre-completion servers get: recvsize = %i, num servers = %i...", rc, numServers);

		while ( (rc_total < (uint)buffer_size) && (timer_get_fixed_seconds() <= end_time) ) {
			if ( (rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer) - rc_total)) != -1 ) {
				rc_total += rc;
				ml_printf("FS2NetD: Server list completion: got an additional %i bytes!", rc);

				if ( rc_total >= sizeof(buffer) )
					break;
			}
		}
	
		ml_printf("FS2NetD: Post-completion servers get: recvsize = %i", rc_total);

		if ( timer_get_fixed_seconds() >= end_time ) {
			ml_printf("FS2NetD: Server list transfer completetion timed out!");
			return 1;
		}

		if (numServers > MAX_SERVERS) {
			ml_printf("FS2NetD WARNING: Server list contains %i server, but can only handle %i at a time!  Some servers will not be listed!", numServers, MAX_SERVERS);
			numServers = MAX_SERVERS;
		}

		for (i = 0; i < numServers; i++) {
			PXO_GET_INT( templist[i].flags );
			PXO_GET_USHORT( templist[i].port );
			PXO_GET_STRING( templist[i].ip );

			if ( !psnet_is_valid_ip_string(templist[i].ip) ) {
				nprintf(("Network", "Invalid ip string (%s)\n", templist[i].ip));
			} else {	
				memset( &addr, 0, sizeof(net_addr) );
				addr.type = NET_TCP;
				psnet_string_to_addr(&addr, templist[i].ip);
				addr.port = (short) templist[i].port;

				if (addr.port == 0)
					addr.port = DEFAULT_GAME_PORT;

				// create a new server item on the list
				item = multi_new_server_item();

				if (item != NULL)
					memcpy( &item->server_addr, &addr, sizeof(net_addr) );
			}
		}

		return 1;
	}

	return 0;
}

void FS2NetD_Ping()
{
	int buffer_size;
	char buffer[15];
	
	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// (fs2open_ping)
	INIT_PACKET( PCKT_PING );

	int time = timer_get_milliseconds();
	PXO_ADD_INT( time );

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

void FS2NetD_Pong(int tstamp)
{
	int buffer_size;
	char buffer[15];

	INIT_PACKET( PCKT_PONG );

	PXO_ADD_INT( tstamp );

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

int FS2NetD_CheckValidSID(int SID)
{
	int buffer_size;
	char buffer[150];

	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// create and send request packet
	INIT_PACKET( PCKT_VALID_SID_RQST );

	PXO_ADD_INT( SID );

	DONE_PACKET();

	if (FS2NetD_SendData(buffer, buffer_size) == -1)
		return -1;

	return 0;
}

int FS2NetD_ValidateTableList(int timeout)
{
	int rc, buffer_size, buffer_offset;
	bool my_packet = false;
	char buffer[1024];
	uint i;
	ushort num_tables = 0;

	if (timeout == 0)
		goto Recieve_Only;

	// clear any old dead crap data
	FS2NetD_IgnorePackets();

	// create and send the request packet
	INIT_PACKET( PCKT_TABLES_RQST );

	num_tables = (ushort)Table_valid_status.size();

	PXO_ADD_USHORT( num_tables );

	for (i = 0; i < Table_valid_status.size(); i++) {
		PXO_ADD_STRING( Table_valid_status[i].name );
		PXO_ADD_UINT( Table_valid_status[i].crc32 );
	}

	DONE_PACKET();

	if (FS2NetD_SendData(buffer, buffer_size) == -1)
		return -1;

	
Recieve_Only:

	ubyte tbl_valid_status = 0;
	uint rc_total = 0;
	fix end_time = timer_get_fixed_seconds() + (MAX_TIMEOUT * F1_0);

	if ( (rc = FS2NetD_GetData(buffer, sizeof(buffer))) != -1 ) {
		if (rc < BASE_PACKET_SIZE)
			return -1;

		VRFY_PACKET2( PCKT_TABLES_REPLY );

		if (!my_packet)
			return -1;

		PXO_GET_USHORT( num_tables );

		if (!num_tables)
			return -1;

		rc_total = rc;

		ml_printf("FS2NetD: Pre-completion tables get: recvsize = %i, num servers = %i...", rc, num_tables);

		while ( (rc_total < (uint)buffer_size) && (timer_get_fixed_seconds() <= end_time) ) {
			if ( (rc = FS2NetD_GetData(buffer+rc_total, sizeof(buffer) - rc_total)) != -1 ) {
				rc_total += rc;
				ml_printf("FS2NetD: Table list completion: got an additional %i bytes!", rc);

				if ( rc_total >= sizeof(buffer) )
					break;
			}
		}

		ml_printf("FS2NetD: Post-completion tables get: recvsize = %i", rc_total);

		if ( timer_get_fixed_seconds() >= end_time ) {
			ml_printf("FS2NetD: Table list transfer completetion timed out!");
			return 1;
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

void FS2NetD_ChatChannelUpdate(char *chan_name)
{
	int buffer_size;
	char buffer[MAX_PATH+BASE_PACKET_SIZE+15];

	Assert( chan_name );
	Assert( strlen(chan_name) && (strlen(chan_name) < MAX_PATH) );

	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// (fs2open_ping)
	INIT_PACKET( PCKT_CHAT_CHANNEL_UPD );

	PXO_ADD_STRING( chan_name );

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

void FS2NetD_GameCountUpdate(char *chan_name)
{
	int buffer_size;
	char buffer[MAX_PATH+BASE_PACKET_SIZE+10];

	// Clear any old dead crap data
	FS2NetD_IgnorePackets();

	// create and send request packet
	INIT_PACKET( PCKT_CHAT_CHAN_COUNT_RQST );

	PXO_ADD_STRING( chan_name );

	DONE_PACKET();

	FS2NetD_SendData(buffer, buffer_size);
}

