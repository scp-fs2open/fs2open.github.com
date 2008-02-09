// Client.cpp
// Client Functions for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################

/*
 * $Logfile: /Freespace2/code/fs2open_pxo/Client.cpp $
 * $Revision: 1.21 $
 * $Date: 2005-07-13 02:50:49 $
 * $Author: Goober5000 $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.20  2005/03/02 21:18:18  taylor
 * better support for Inferno builds (in PreProcDefines.h now, no networking support)
 * make sure NO_NETWORK builds are as friendly on Windows as it is on Linux/OSX
 * revert a timeout in Client.h back to the original value before Linux merge
 *
 * Revision 1.19  2005/02/23 13:17:04  taylor
 * few more compiler warning fixes (didn't mean to commit iostream.h change)
 * lower warning level to 3 to stop MSVC6 from complaining about C++ headers
 *
 * Revision 1.18  2005/02/23 05:05:37  taylor
 * compiler warning fixes (for MSVC++ 6)
 * have the warp effect only load as many LODs as will get used
 * head off strange bug in release when corrupt soundtrack number gets used
 *    (will still Assert in debug)
 * don't ever try and save a campaign savefile in multi or standalone modes
 * first try at 32bit->16bit color conversion for TGA code (for TGA only ship textures)
 *
 * Revision 1.17  2005/02/04 20:06:03  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 1.16  2004/11/21 16:07:22  Kazan
 * dumbbugfix
 *
 * Revision 1.15  2004/11/18 00:05:36  Goober5000
 * #pragma'd a bunch of warnings
 * --Goober5000
 *
 * Revision 1.14  2004/05/25 00:21:39  wmcoolmon
 * Updated to use <iostream> instead of <iostream.h>
 *
 * Revision 1.13  2004/03/07 23:07:20  Kazan
 * [Incomplete] Readd of Software renderer so Standalone server works
 *
 * Revision 1.12  2004/03/05 09:01:56  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 1.11  2004/02/21 00:59:43  Kazan
 * FS2NETD License Comments
 *
 * Revision 1.10  2004/02/04 09:02:42  Goober5000
 * got rid of unnecessary double semicolons
 * --Goober5000
 *
 * Revision 1.9  2003/11/11 02:15:42  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 1.8  2003/11/09 04:09:17  Goober5000
 * edited for language
 * --Goober5000
 *
 * Revision 1.7  2003/11/06 20:22:05  Kazan
 * slight change to .dsp - leave the release target as fs2_open_r.exe already
 * added myself to credit
 * killed some of the stupid warnings (including doing some casting and commenting out unused vars in the graphics modules)
 * Release builds should have warning level set no higher than 2 (default is 1)
 * Why are we getting warning's about function selected for inline expansion... (killing them with warning disables)
 * FS2_SPEECH was not defined (source file doesn't appear to capture preproc defines correctly either)
 *
 * Revision 1.6  2003/10/13 06:02:50  Kazan
 * Added Log Comment Thingy to these files
 *
 *
 *
 */


#ifndef NO_NETWORK

// 4711 = function selected for automatic inline expansion
#pragma warning(disable: 4711)

#include <iostream>
#include <time.h>


#include "fs2open_pxo/Client.h"
//#include "fs2open_pxo/protocol.h"
#include "fs2open_pxo/udpsocket.h"
#include "network/multi_log.h"
#include "playerman/player.h"
#include "ship/ship.h"

PXO_Socket FS2OpenPXO_Socket;

// NOTE: CLK_TCK is depreciated but available for compatibility, use CLOCKS_PER_SEC instead
#ifndef CLK_TCK
#define CLK_TCK CLOCKS_PER_SEC
#endif

#define PXO_ADDINT(n)	*((int *)cur) = (n); cur += sizeof(int);
#define PXO_ADDSTRING(x, y) memcpy(cur, x, y); cur += y;
// need to compile and link UDP_Server.cpp

//**************************************************************************************************************************************************

int CheckSingleMission(const char* mission, unsigned int crc32, UDP_Socket &Socket, const char* masterserver, int port, int timeout)
{
	
	timeout = timeout * CLK_TCK;
	int starttime = clock();

	std::string sender = masterserver;

	// send Packet
	fs2open_file_check_single checkpack;
	memset((char *) &checkpack, 0, sizeof(fs2open_file_check_single));
	checkpack.pid = PCKT_MISSION_CHECK;
	strncpy(checkpack.name, mission, 60);
	checkpack.crc32 = crc32;


	if (Socket.SendPacket((char *) &checkpack, sizeof(fs2open_file_check_single), sender, port) == -1)
		return -1;


	fs2open_fcheck_reply c_reply;

	while ((clock() - starttime) <= timeout)
	{

		if (Socket.GetPacket((char *) &c_reply, sizeof(fs2open_fcheck_reply), sender) != -1)
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

int SendPlayerData(int SID, const char* player_name, const char* user, player *pl, const char* masterserver, UDP_Socket &Socket, int port, int timeout)
{

	

	timeout = timeout * CLK_TCK;
	int starttime = clock();


	std::string sender = masterserver;



	char PacketBuffer[16384]; // 16K should be enough i think..... I HOPE!
	memset(PacketBuffer,0, 16384);
//	char *cur = PacketBuffer;

	fs2open_pilot_update *p_update = (fs2open_pilot_update *) PacketBuffer;
	fs2open_ship_typekill *type_kills = (fs2open_ship_typekill *) (PacketBuffer + (sizeof(fs2open_pilot_update) - sizeof(fs2open_ship_typekill *)));
	

	p_update->pid = PCKT_PILOT_UPDATE;
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
	p_update->ship_types =		MAX_SHIP_TYPES;

	for (int i = 0; i < MAX_SHIP_TYPES; i++)
	{

		strncpy(type_kills[i].name, Ship_info[i].name, 32);
		type_kills[i].kills = pl->stats.kills[i];
	}
			

	int packet_size = 194; // size of all the ints only
	packet_size += sizeof(fs2open_ship_typekill) * MAX_SHIP_TYPES; // add the size of the ship_kills array

	// send Packet
	if (Socket.SendPacket(PacketBuffer, packet_size, sender, port) == -1)
		return -1;


	fs2open_pilot_updatereply u_reply;

	while ((clock() - starttime) <= timeout)
	{

		if (Socket.GetPacket((char *) &u_reply, sizeof(fs2open_pilot_updatereply), sender) != -1)
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

int GetPlayerData(int SID, const char* player_name, player *pl, const char* masterserver, UDP_Socket &Socket, int port, bool CanCreate, int timeout)
{

	fs2open_get_pilot prq_packet;

	timeout = timeout * CLK_TCK;
	int starttime = clock();

	memset((char *) &prq_packet, 0, sizeof(fs2open_get_pilot));
	prq_packet.pid = PCKT_PILOT_GET;
	prq_packet.sid = SID;
	strncpy(prq_packet.pilotname, player_name, 64);
	prq_packet.create = CanCreate;

	std::string sender = masterserver;

	// send Packet
	if (Socket.SendPacket((char *)&prq_packet, sizeof(fs2open_get_pilot), sender, port) == -1)
		return -1;

	char PacketBuffer[16384]; // 16K should be enough i think..... I HOPE!
	fs2open_pilot_reply *p_reply = (fs2open_pilot_reply *) PacketBuffer;
	memset(PacketBuffer, 0, 16384);
	fs2open_ship_typekill *type_kills = (fs2open_ship_typekill *) (PacketBuffer + sizeof(fs2open_pilot_reply) - sizeof(fs2open_ship_typekill *));

	while ((clock() - starttime) <= timeout)
	{

		if (Socket.GetPacket((char *) &PacketBuffer, 16384, sender) != -1)
		{

			if (p_reply->pid != PCKT_PILOT_REPLY) // ignore packet
			{
				continue;
			}

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

         
			if (p_reply->ship_types == 0)
				memset(pl->stats.kills, 0, sizeof(int) * MAX_SHIP_TYPES);
			else
			{
				// i should really assert p_reply->ship_types == MAX_SHIP_TYPES
				for (int i = 0; i < MAX_SHIP_TYPES && p_reply->ship_types; i++)
				{
					pl->stats.kills[i] = type_kills[i].kills;
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



file_record* GetTablesList(int &numTables, const char *masterserver, UDP_Socket &Socket, int port, int timeout)
{
	fs2open_file_check fc_packet;
	fc_packet.pid = PCKT_TABLES_RQST;

	timeout = timeout * CLK_TCK;
	int starttime = clock();

	std::string sender = masterserver;

	// send Packet
	if (Socket.SendPacket((char *)&fc_packet, sizeof(fs2open_file_check), sender, port) == -1)
		return NULL;

	char PacketBuffer[16384]; // 16K should be enough i think..... I HOPE!
	fs2open_pxo_missreply *misreply_ptr = (fs2open_pxo_missreply *) PacketBuffer;
	
	file_record *frecs = NULL;
	numTables = 0;

	while ((clock() - starttime) <= timeout)
	{

		if (Socket.GetPacket(PacketBuffer, 16384, sender) != -1)
		{
			if (misreply_ptr->pid != PCKT_TABLES_REPLY)
			{
				continue; // skip and ignore this packet
			}
			

			if ((misreply_ptr->num_files * sizeof(file_record)) + (sizeof(int) * 2) > 16384)
			{
				// WE'RE IN TROUBLE!

				ml_printf("Network (FS2OpenPXO): PCKT_TABLES_REPLY was larger than 16k!!!\n");
				return NULL;
			}
			frecs = new file_record[misreply_ptr->num_files];
			memcpy(frecs, PacketBuffer + 8, sizeof(file_record) * misreply_ptr->num_files); // packet buffer will be two ints then the array;
			numTables = misreply_ptr->num_files;
			return frecs;

		}


	}

	return NULL;

}

//**************************************************************************************************************************************************

file_record* GetMissionsList(int &numMissions, const char *masterserver, UDP_Socket &Socket, int port, int timeout)
{
	fs2open_file_check fc_packet;
	fc_packet.pid = PCKT_MISSIONS_RQST;

	timeout = timeout * CLK_TCK;
	int starttime = clock();

	std::string sender = masterserver;

	// send Packet
	if (Socket.SendPacket((char *)&fc_packet, sizeof(fs2open_file_check), sender, port) == -1)
		return NULL;

	char PacketBuffer[16384]; // 16K should be enough i think..... I HOPE!
	fs2open_pxo_missreply *misreply_ptr = (fs2open_pxo_missreply *) PacketBuffer;
	
	file_record *frecs = NULL;
	numMissions = 0;

	while ((clock() - starttime) <= timeout)
	{

		if (Socket.GetPacket(PacketBuffer, 16384, sender) != -1)
		{
			if (misreply_ptr->pid != PCKT_MISSIONS_REPLY)
			{
				continue; // skip and ignore this packet
			}
			

			if ((misreply_ptr->num_files * sizeof(file_record)) + (sizeof(int) * 2) > 16384)
			{
				// WE'RE IN TROUBLE!

				ml_printf("Network (FS2OpenPXO): PCKT_MISSIONS_REPLY was larger than 16k!!!\n");
				return NULL;
			}
			frecs = new file_record[misreply_ptr->num_files];
			memcpy(frecs, PacketBuffer + 8, sizeof(file_record) * misreply_ptr->num_files); // packet buffer will be two ints then the array;
			numMissions = misreply_ptr->num_files;
			return frecs;

		}


	}

	return NULL;

}
//**************************************************************************************************************************************************


int Fs2OpenPXO_Login(const char* username, const char* password, UDP_Socket &Socket, const char* masterserver, int port, int timeout)
{
	timeout = timeout * CLK_TCK;
	int starttime = clock();

	std::string sender = masterserver;

	// create and send login packet
	fs2open_pxo_login loginpckt;
	memset(&loginpckt, 0, sizeof(loginpckt));
	loginpckt.pid = PCKT_LOGIN_AUTH;
	strncpy(loginpckt.username, username, 64);
	strncpy(loginpckt.password, password, 64);


	if (Socket.SendPacket((char *) &loginpckt, sizeof(loginpckt), sender, port) == -1)
	{
		std::cout << "Error Sending Packet" << std::endl;
	}
	
	// await reply
	fs2open_pxo_lreply loginreply;
	while ((clock() - starttime) <= timeout)
	{

		if (Socket.GetPacket((char *)&loginreply, sizeof(fs2open_pxo_lreply), sender) != -1)
		{
			if (loginreply.pid != PCKT_LOGIN_REPLY)
			{
				continue; // skip and ignore this packet
			}

			if (loginreply.login_status == false)
				return -1;
			else
				return loginreply.sid;
		}


	}



	return -1;
}


//**************************************************************************************************************************************************

void SendHeartBeat(const char* masterserver, int targetport, UDP_Socket &Socket, const char* myName, const char* MisName, const char* title, int flags, int port, int players)
{
	// Clear any old dead crap data
	//Socket.IgnorePackets();

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

	std::string ms = masterserver;
	Socket.SendPacket((char *) &hbpack, (int)sizeof(serverlist_hb_packet), ms, targetport);
}


//**************************************************************************************************************************************************


net_server* GetServerList(const char* masterserver, int &numServersFound, UDP_Socket &Socket, int port, int timeout)
{
	numServersFound = 0;

	
	
	timeout = timeout * CLK_TCK;
	int starttime = clock();
	// ---------- Prepair and send request packet ------------	
	serverlist_request_packet rpack;
	rpack.pid = PCKT_SLIST_REQUEST;
	rpack.type = 0xFFFFFFFF;
	rpack.status = 0xFFFFFFFF;
	std::string sender = masterserver;	
	
	Socket.SendPacket((char *) &rpack, sizeof(serverlist_request_packet), sender, port);
	

	// ---------- Receive Reply ------------

	serverlist_reply_packet NewServer;
	net_server *retlist, templist[MAX_SERVERS];

	while ((clock() - starttime) <= timeout)
	{
		if (Socket.GetPacket((char *)&NewServer, sizeof(serverlist_reply_packet), sender) != -1)
		{

			if (!strcmp(NewServer.name, "TERM") && NewServer.flags == 0 && NewServer.players == 0)
			{
				starttime = 0;
				break;
			}
		
			memcpy((char *)&templist[numServersFound], (char *)&NewServer, sizeof(serverlist_reply_packet));

			numServersFound++;
		}

		
	}


	// ---------- Prepair and return list ------------
	retlist = new net_server[numServersFound];
	memcpy(retlist, templist, sizeof(net_server) * numServersFound);

	return retlist;
}

//**************************************************************************************************************************************************


int Ping(const char* target, UDP_Socket &Socket)
{
	fs2open_ping ping;
	fs2open_pingreply rping;
	ping.pid = PCKT_PING;
	ping.time = time(0);
	std::string rctp = target;

	Socket.SendPacket((char *)&ping, sizeof(fs2open_ping), rctp, FS2OPEN_PXO_PORT);

	Socket.GetPacket((char *)&rping, sizeof(fs2open_pingreply), rctp);

	return int( float(time(0) - rping.time)/2.0 );
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

int Ping(const char* target)
{
	// ---------- Open and Bind Socket ------------
	UDP_Socket Socket(FS2OPEN_CLIENT_PORT);
	if (!Socket.InitSocket())
	{
		std::cout << "Initialization Error!" << std::endl;
		exit(1);
	}


	fs2open_ping ping;
	fs2open_pingreply rping;
	ping.pid = PCKT_PING;
	ping.time = time(0);
	std::string rctp = target;

	Socket.SendPacket((char *)&ping, sizeof(fs2open_ping), rctp, FS2OPEN_PXO_PORT);

	Socket.GetPacket((char *)&rping, sizeof(fs2open_pingreply), rctp);

	return int( float(time(0) - rping.time)/2.0 );
}

#endif // !NO_NETWORK
