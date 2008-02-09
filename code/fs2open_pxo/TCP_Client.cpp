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
 * $Revision: 1.35 $
 * $Date: 2006-01-26 03:23:29 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
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



#include "fs2open_pxo/Client.h"
#include "fs2open_pxo/protocol.h"
#include "fs2open_pxo/TCP_Socket.h"
#include "network/multi_log.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "io/timer.h"
#include "globalincs/pstypes.h"

#include <iostream>

#define PXO_ADDINT(n)	*((int *)cur) = (n); cur += sizeof(int);
#define PXO_ADDSTRING(x, y) memcpy(cur, x, y); cur += y;

// need to compile and link TCP_Socket.cpp

//**************************************************************************************************************************************************

int CheckSingleMission(const char* mission, unsigned int crc32, TCP_Socket &Socket, const char* masterserver, int port, int timeout)
{
	
	// Clear any old dead crap data
	Socket.IgnorePackets();

	fix end_time = timer_get_fixed_seconds() + (timeout * F1_0);

	std::string sender = masterserver;

	// send Packet
	fs2open_file_check_single checkpack;
	memset((char *) &checkpack, 0, sizeof(fs2open_file_check_single));
	checkpack.pid = PCKT_MISSION_CHECK;
	strncpy(checkpack.name, mission, 60);
	checkpack.crc32 = crc32;


		
	if (Socket.SendData((char *) &checkpack, sizeof(fs2open_file_check_single)) == -1)
		return -1;


	fs2open_fcheck_reply c_reply;

	while ( timer_get_fixed_seconds() <= end_time )
	{

		if (Socket.DataReady() && Socket.GetData((char *) &c_reply, sizeof(fs2open_fcheck_reply)) != -1)
		{

			if (c_reply.pid != PCKT_MCHECK_REPLY) // ignore packet
			{
				continue;
			}

			return c_reply.status;


			
		}


	}

	return -1;
}
//**************************************************************************************************************************************************

int SendPlayerData(int SID, const char* player_name, const char* user, player *pl, const char* masterserver, TCP_Socket &Socket, int port, int timeout)
{

	// Clear any old dead crap data
	Socket.IgnorePackets();


	fix end_time = timer_get_fixed_seconds() + (timeout * F1_0);


	std::string sender = masterserver;


	char PacketBuffer[16384]; // 16K should be enough i think..... I HOPE!
	memset(PacketBuffer,0, 16384);
//	char *cur = PacketBuffer;

	fs2open_pilot_update *p_update = (fs2open_pilot_update *) PacketBuffer;
	fs2open_ship_typekill *type_kills = (fs2open_ship_typekill *) (PacketBuffer + 204);
	int *medals = (int*)(PacketBuffer + 204 + (sizeof(fs2open_ship_typekill) * MAX_SHIP_CLASSES));
	

	p_update->pid = PCKT_PILOT_UPDATE2;
	p_update->sid = SID;

	strncpy(p_update->name, player_name, 65);
	strncpy(p_update->user, user, 65);

	p_update->points =			pl->stats.score;
	p_update->missions =		pl->stats.missions_flown;
	p_update->flighttime =		pl->stats.flight_time;
	p_update->LastFlight =		pl->stats.last_flown;
	p_update->Kills =			pl->stats.kill_count;
	p_update->FriendlyKills =	pl->stats.kill_count - pl->stats.kill_count_ok;
	p_update->Assists =			pl->stats.assists;
	p_update->PriShots =		pl->stats.p_shots_fired; 
	p_update->PriHits =			pl->stats.p_shots_hit; 
	p_update->PriFHits =		pl->stats.p_bonehead_hits; 
	p_update->SecShots =		pl->stats.s_shots_fired; 
	p_update->SecHits =			pl->stats.s_shots_hit; 
	p_update->SecFHits =		pl->stats.s_bonehead_hits;
	p_update->ship_types =		MAX_SHIP_CLASSES;
	p_update->num_medals =		MAX_MEDALS;
	p_update->rank	=			pl->stats.rank;

	int i;
	for (i = 0; i < MAX_SHIP_CLASSES; i++)
	{

		strncpy(type_kills[i].name, Ship_info[i].name, 32);
		type_kills[i].kills = pl->stats.kills[i];
	}
			
	
	for (i = 0; i < MAX_MEDALS; i++)
	{
		medals[i] = pl->stats.medals[i];
	}

	int packet_size = 204; // size of all the ints only
	packet_size += sizeof(fs2open_ship_typekill) * MAX_SHIP_CLASSES; // add the size of the ship_kills array
	packet_size += sizeof(int) * MAX_MEDALS; // add the size of the ship_kills array

	// send Packet
	if (Socket.SendData(PacketBuffer, packet_size) == -1)
		return -1;


	fs2open_pilot_updatereply u_reply;

	while ( timer_get_fixed_seconds() <= end_time )
	{

		if (Socket.DataReady() && (Socket.GetData((char *) &u_reply, sizeof(fs2open_pilot_updatereply)) != -1))
		{

			if (u_reply.pid != PCKT_PILOT_UREPLY) // ignore packet
			{
				continue;
			}

			return u_reply.replytype;


			
		}


	}

	return -1;
}

//**************************************************************************************************************************************************

int GetPlayerData(int SID, const char* player_name, player *pl, const char* masterserver, TCP_Socket &Socket, int port, bool CanCreate, int timeout)
{

	// Clear any old dead crap data
	Socket.IgnorePackets();

	fs2open_get_pilot prq_packet;

	memset((char *) &prq_packet, 0, sizeof(fs2open_get_pilot));
	prq_packet.pid = PCKT_PILOT_GET2;
	prq_packet.sid = SID;
	strncpy(prq_packet.pilotname, player_name, 64);
	prq_packet.create = CanCreate;

	std::string sender = masterserver;

	// send Packet
	if (Socket.SendData((char *)&prq_packet, sizeof(fs2open_get_pilot)) == -1)
		return -1;

	char PacketBuffer[16384]; // 16K should be enough i think..... I HOPE!
	fs2open_pilot_reply *p_reply = (fs2open_pilot_reply *) PacketBuffer;
	memset(PacketBuffer, 0, 16384);
	fs2open_ship_typekill *type_kills = (fs2open_ship_typekill *) (PacketBuffer + 72);
	int *medals = NULL; //(int*)(PacketBuffer+72+(sizeof(fs2open_ship_typekill) * pilotrep.ship_types));

	int CheckSize =  72, i;
	int recvsize = 0, rs2;
	Sleep(2); // give them a second

	fix end_time = timer_get_fixed_seconds() + (timeout * F1_0);

	while ( timer_get_fixed_seconds() <= end_time )
	{
		if (Socket.DataReady() && ((recvsize = Socket.GetData((char *) &PacketBuffer, 16384)) != -1))
		{

			if (p_reply->pid != PCKT_PILOT_REPLY) // ignore packet
			{
				continue;
			}

			if (p_reply->replytype == 0)
				CheckSize += (sizeof(fs2open_ship_typekill) * p_reply->ship_types) + (sizeof(int) * p_reply->num_medals);
			else
				CheckSize = 8;

			ml_printf("Precompletion Pilot Update: CheckSize = %d, recvsize = %d", CheckSize, recvsize);
			while (CheckSize - recvsize > 0 && timer_get_fixed_seconds() <= end_time)
			{
				if (Socket.DataReady())
				{
					rs2 = Socket.GetData((char *) &PacketBuffer+recvsize, 16384-recvsize);
					ml_printf("Tables Completetion: Got Additional %u bytes", rs2);
					recvsize += rs2;
				}
			}
			ml_printf("PostCompletion Tables Get: CheckSize = %u, recvsize = %u", CheckSize, recvsize);

			// if we timed out
			if ( timer_get_fixed_seconds() >= end_time )
			{
				ml_printf("Pilot Update completetion timed out");
				break;
			}

			
			medals = (int*)(PacketBuffer+72+(sizeof(fs2open_ship_typekill) * p_reply->ship_types));


			if (p_reply->replytype > 1)
				return p_reply->replytype; // don't want to try and access non-existant data

			pl->stats.score =			p_reply->points;
			pl->stats.missions_flown =	p_reply->missions;
			pl->stats.flight_time =		p_reply->flighttime;
			pl->stats.last_flown =		p_reply->LastFlight;
			pl->stats.kill_count =		p_reply->Kills;
			pl->stats.kill_count_ok =	p_reply->Kills - p_reply->FriendlyKills; 
				// p_reply.FriendlyKills = pl->stats.kill_count -> pl->stats.kill_count_ok;
			pl->stats.assists =			p_reply->Assists;
			pl->stats.p_shots_fired =	p_reply->PriShots; 
			pl->stats.p_shots_hit =		p_reply->PriHits; 
			pl->stats.p_bonehead_hits = p_reply->PriFHits; 
			pl->stats.s_shots_fired =	p_reply->SecShots; 
			pl->stats.s_shots_hit =		p_reply->SecHits; 
			pl->stats.s_bonehead_hits = p_reply->SecFHits;
			pl->stats.rank			  = p_reply->rank;

         
			if (p_reply->ship_types == 0)
				memset(pl->stats.kills, 0, sizeof(int) * MAX_SHIP_CLASSES);
			else
			{
				// i should really assert p_reply->ship_types == MAX_SHIP_CLASSES
				for (i = 0; i < MAX_SHIP_CLASSES && p_reply->ship_types; i++)
				{
					pl->stats.kills[i] = type_kills[i].kills;
				}
			}
         
			if (p_reply->num_medals == 0)
				memset(pl->stats.medals, 0, sizeof(int) * MAX_MEDALS);
			else
			{
				// i should really assert p_reply->ship_types == MAX_SHIP_CLASSES
				for (i = 0; i < MAX_MEDALS && p_reply->num_medals; i++)
				{
					pl->stats.medals[i] = medals[i];
				}
			}

			// ignored on load... fs2 doesn't need this 
			// type_kills[index].name[65] = Ship_info[index].name[32]
			


			return p_reply->replytype;
		}


	}

	return -1;
}
//**************************************************************************************************************************************************



fs2open_banmask* GetBanList(int &numBanMasks, TCP_Socket &Socket, int timeout)
{
	// Clear any old dead crap data
	Socket.IgnorePackets();

	fs2open_banlist_request br_packet;
	br_packet.pid = PCKT_BANLIST_RQST;
	br_packet.reserved = 0;


	//std::string sender = masterserver;

	// send Packet
	if (Socket.SendData((char *)&br_packet, sizeof(fs2open_banlist_request)) == -1)
		return NULL;

	char PacketBuffer[16384]; // 16K should be enough i think..... I HOPE!
	fs2open_banlist_reply *banreply_ptr = (fs2open_banlist_reply *) PacketBuffer;
	
	fs2open_banmask *masks = NULL;
	numBanMasks = 0;
	Sleep(1); //lets give them a second

	fix end_time = timer_get_fixed_seconds() + (timeout * F1_0);

	int recvsize = 0, rs2;
	int CheckSize = sizeof(int) * 2;


	while ( timer_get_fixed_seconds() <= end_time )
	{

		if (Socket.DataReady() && ((recvsize = Socket.GetData(PacketBuffer, 16384)) != -1))
		{
			if (banreply_ptr->pid != PCKT_BANLIST_RPLY)
			{
				continue; // skip and ignore this packet
			}
			

			CheckSize += banreply_ptr->num_ban_masks * sizeof(fs2open_banmask);

			ml_printf("Precompletion Table Get: CheckSize = %d, recvsize = %d", CheckSize, recvsize);
			while (CheckSize - recvsize > 0 && timer_get_fixed_seconds() <= end_time)
			{
				if (Socket.DataReady())
				{
					rs2 = Socket.GetData(PacketBuffer+recvsize, 16384-recvsize);
					ml_printf("Banlist Completetion: Got Additional %u bytes", rs2);
					recvsize += rs2;
				}
			}
			ml_printf("PostCompletion Banlist Get: CheckSize = %u, recvsize = %u", CheckSize, recvsize);

			if ( timer_get_fixed_seconds() >= end_time )
			{
				ml_printf("Banlist Transfer completetion timed out");
				break;
			}

			masks = new fs2open_banmask[banreply_ptr->num_ban_masks];
			memcpy(masks, PacketBuffer + 8, sizeof(fs2open_banmask) * banreply_ptr->num_ban_masks); // packet buffer will be two ints then the array;
			numBanMasks = banreply_ptr->num_ban_masks;
			

			return masks;

		}


	}

	return NULL;

}
//**************************************************************************************************************************************************



file_record* GetTablesList(int &numTables, const char *masterserver, TCP_Socket &Socket, int port, int timeout)
{
	// Clear any old dead crap data
	Socket.IgnorePackets();

	fs2open_file_check fc_packet;
	fc_packet.pid = PCKT_TABLES_RQST;

	std::string sender = masterserver;

	// send Packet
	if (Socket.SendData((char *)&fc_packet, sizeof(fs2open_file_check)) == -1)
		return NULL;

	char PacketBuffer[16384]; // 16K should be enough i think..... I HOPE!
	fs2open_pxo_missreply *misreply_ptr = (fs2open_pxo_missreply *) PacketBuffer;
	
	file_record *frecs = NULL;
	numTables = 0;
	Sleep(2); //lets give them a second

	fix end_time = timer_get_fixed_seconds() + (timeout * F1_0);

	int recvsize = 0, rs2;
	int CheckSize = sizeof(int) * 2;

	while ( timer_get_fixed_seconds() <= end_time )
	{
		if (Socket.DataReady() && ((recvsize = Socket.GetData(PacketBuffer, 16384)) != -1))
		{
			if (misreply_ptr->pid != PCKT_TABLES_REPLY)
			{
				continue; // skip and ignore this packet
			}
			

			CheckSize += misreply_ptr->num_files * sizeof(file_record);

			ml_printf("Precompletion Table Get: CheckSize = %d, recvsize = %d", CheckSize, recvsize);
			while (CheckSize - recvsize > 0 && timer_get_fixed_seconds() <= end_time)
			{
				if (Socket.DataReady())
				{
					rs2 = Socket.GetData(PacketBuffer+recvsize, 16384-recvsize);
					ml_printf("Tables Completetion: Got Additional %u bytes", rs2);
					recvsize += rs2;
				}
			}
			ml_printf("PostCompletion Tables Get: CheckSize = %u, recvsize = %u", CheckSize, recvsize);

			if ( timer_get_fixed_seconds() >= end_time )
			{
				ml_printf("Tables Transfer completetion timed out");
				break;
			}

			/*
			if ((misreply_ptr->num_files * sizeof(file_record)) + (sizeof(int) * 2) > 16384)
			{
				// WE'RE IN TROUBLE!

				ml_printf("Network (FS2OpenPXO): PCKT_TABLES_REPLY was larger than 16k!!!\n");
				return NULL;
			}*/

			frecs = new file_record[misreply_ptr->num_files];
			memcpy(frecs, PacketBuffer + 8, sizeof(file_record) * misreply_ptr->num_files); // packet buffer will be two ints then the array;
			numTables = misreply_ptr->num_files;
			
			/*for (int i = 0; i < misreply_ptr->num_files; i++)
			{
				ml_printf("Tables[%u] = { \"%s\", %u}", i, frecs[i].name, frecs[i].crc32);
			}*/
			return frecs;

		}


	}

	return NULL;

}

//**************************************************************************************************************************************************

file_record* GetMissionsList(int &numMissions, const char *masterserver, TCP_Socket &Socket, int port, int timeout)
{
	// Clear any old dead crap data
	Socket.IgnorePackets();

	fs2open_file_check fc_packet;
	fc_packet.pid = PCKT_MISSIONS_RQST;


	std::string sender = masterserver;

	// send Packet
	if (Socket.SendData((char *)&fc_packet, sizeof(fs2open_file_check)) == -1)
		return NULL;

	char PacketBuffer[16384]; // 16K should be enough i think..... I HOPE!
	memset(PacketBuffer, 0, 16384);

	fs2open_pxo_missreply *misreply_ptr = (fs2open_pxo_missreply *) PacketBuffer;
	
	file_record *frecs = NULL;
	numMissions = 0;
	//int depth = 0;
	Sleep(2); // lets give it a second

	fix end_time = timer_get_fixed_seconds() + (timeout * F1_0);

	int recvsize = 0, rs2;
	int CheckSize = sizeof(int) * 2;

	while ( timer_get_fixed_seconds() <= end_time )
	{

		if (Socket.DataReady() && ((recvsize = Socket.GetData(PacketBuffer, 16384)) != -1))
		{

			if (misreply_ptr->pid != PCKT_MISSIONS_REPLY)
			{
				continue; // skip and ignore this packet
			}
			
			CheckSize += misreply_ptr->num_files * sizeof(file_record);

			ml_printf("Precompletion Missions Get: CheckSize = %d, recvsize = %d", CheckSize, recvsize);
			while (CheckSize - recvsize > 0 && timer_get_fixed_seconds() <= end_time)
			{
				if (Socket.DataReady())
				{
					rs2 = Socket.GetData(PacketBuffer+recvsize, 16384-recvsize);
					ml_printf("Missions Completetion: Got Additional %u bytes", rs2);
					recvsize += rs2;
				}
			}
			ml_printf("PostCompletion Missions Get: CheckSize = %u, recvsize = %u", CheckSize, recvsize);

			if ( timer_get_fixed_seconds() >= end_time )
			{
				ml_printf("Missions Transfer completetion timed out");
				break;
			}
			/*
			if ((misreply_ptr->num_files * sizeof(file_record)) + (sizeof(int) * 2) > 16384)
			{
				// WE'RE IN TROUBLE!

				ml_printf("Network (FS2OpenPXO): PCKT_MISSIONS_REPLY was larger than 16k!!!\n");
				return NULL;
			}*/

			frecs = new file_record[misreply_ptr->num_files];
			memcpy(frecs, PacketBuffer + 8, sizeof(file_record) * misreply_ptr->num_files); // packet buffer will be two ints then the array;
			numMissions = misreply_ptr->num_files;

			for (int i = 0; i < misreply_ptr->num_files; i++)
			{
				ml_printf("Tables[%u] = { \"%s\", %u}", i, frecs[i].name, frecs[i].crc32);
			}
			return frecs;

		}


	}

	return NULL;

}
//**************************************************************************************************************************************************


int Fs2OpenPXO_Login(const char* username, const char* password, TCP_Socket &Socket, const char* masterserver, int port, int timeout)
{
	// Clear any old dead crap data
	Socket.IgnorePackets();

	std::string sender = masterserver;

	// create and send login packet
	fs2open_pxo_login loginpckt;
	memset(&loginpckt, 0, sizeof(loginpckt));
	loginpckt.pid = PCKT_LOGIN_AUTH;
	strncpy(loginpckt.username, username, 64);
	strncpy(loginpckt.password, password, 64);


	if (Socket.SendData((char *) &loginpckt, sizeof(loginpckt)) == -1)
	{
		std::cout << "Error Sending Packet" << std::endl;
		return -1;
	}

	fix end_time = timer_get_fixed_seconds() + (timeout * F1_0);

	// await reply
	fs2open_pxo_lreply loginreply;

	while ( timer_get_fixed_seconds() <= end_time )
	{

		if (Socket.DataReady() && Socket.GetData((char *)&loginreply, sizeof(fs2open_pxo_lreply)) != -1)
		{
			if (loginreply.pid != PCKT_LOGIN_REPLY)
			{
				continue; // skip and ignore this packet
			}

			if (loginreply.login_status == false)
			{
				return -1;
			}
			else
				return loginreply.sid;
		}


	}



	return -1;
}


//**************************************************************************************************************************************************



void SendHeartBeat(const char* masterserver, int targetport, TCP_Socket &Socket, const char* myName, const char* MisName, const char* title, int flags, int port, int players)
{
	// Clear any old dead crap data
	Socket.IgnorePackets();

	// ---------- Prepair and send packet ------------	
	serverlist_hb_packet hbpack;
	memset(&hbpack, 0, sizeof(serverlist_hb_packet));

	hbpack.pid = PCKT_SLIST_HB;
	strncpy(hbpack.name, myName, 64);
	strncpy(hbpack.mission_name, MisName, 64);
	strncpy(hbpack.title, title, 64);
	hbpack.flags = flags; // cooperative
	hbpack.players = (short) players;
	hbpack.port = port;


	Socket.SendData((char *) &hbpack, sizeof(serverlist_hb_packet));
}

//**************************************************************************************************************************************************



net_server* GetServerList(const char* masterserver, int &numServersFound, TCP_Socket &Socket, int port, int timeout)
{
	// Clear any old dead crap data
	Socket.IgnorePackets();

	numServersFound = 0;


	// ---------- Prepair and send request packet ------------	
	serverlist_request_packet rpack;
	rpack.pid = PCKT_SLIST_REQUEST;
	rpack.type = 0xFFFFFFFF;
	rpack.status = 0xFFFFFFFF;
	std::string sender = masterserver;	
	
	Socket.SendData((char *) &rpack, sizeof(serverlist_request_packet));
	

	// ---------- Receive Reply ------------

	serverlist_reply_packet NewServer;
	net_server *retlist = NULL, templist[MAX_SERVERS];

	fix end_time = timer_get_fixed_seconds() + (timeout * F1_0);

	while ( timer_get_fixed_seconds() <= end_time )
	{
		if (Socket.DataReady() && Socket.GetData((char *)&NewServer, sizeof(serverlist_reply_packet)) == sizeof(serverlist_reply_packet))
		{
			if (!strcmp(NewServer.name, "TERM") && NewServer.flags == 0 && NewServer.players == 0)
			{
				break;
			}

			memcpy((char *)&templist[numServersFound], (char *)&NewServer, sizeof(serverlist_reply_packet));

			numServersFound++;

			if (numServersFound >= MAX_SERVERS) {
				numServersFound = MAX_SERVERS;
				break;
			}
		}
	}

	if (!numServersFound)
		return NULL;

	// ---------- Prepair and return list ------------
	retlist = new net_server[numServersFound];
	memcpy(retlist, templist, sizeof(net_server) * numServersFound);

	return retlist;
}

//**************************************************************************************************************************************************


int Ping(const char* target, TCP_Socket &Socket)
{
	// Clear any old dead crap data
	Socket.IgnorePackets();

	fs2open_ping ping;
	//fs2open_pingreply rping;
	ping.pid = PCKT_PING;
	ping.time = clock();
	//std::string rctp = target;

	Socket.SendData((char *)&ping, sizeof(fs2open_ping));

	//Socket.GetData((char *)&rping, sizeof(fs2open_pingreply));

	//return int( float(time(0) - rping.time)/2.0 );
	return 0;
}

//**************************************************************************************************************************************************

/*
struct fs2open_pingreply
{
	int pid; // 0xE (PCKT_PINGREPLY
	int time;
};
*/

int GetPingReply(TCP_Socket &Socket)
{
	fs2open_pingreply rping;
	
	Socket.GetData((char *)&rping, sizeof(fs2open_pingreply));

	return int( float(clock() - rping.time)/2.0 );
}


//**************************************************************************************************************************************************

void SendPingReply(TCP_Socket &Socket, int tstamp)
{
	
	fs2open_pingreply rping;
	rping.pid = PCKT_PINGREPLY;
	rping.time = tstamp;

	Socket.SendData((char*)&rping, sizeof(rping));
}
