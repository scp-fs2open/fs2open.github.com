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
 * $Revision: 1.10 $
 * $Date: 2004-03-05 09:01:56 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
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
 * Commented out the "warning null vector in vector normalize" crap since we don't give a rats arse
 * Added "beam no whack" flag for beams - said beams NEVER whack
 * Some reliability updates in FS2NetD
 *
 *
 *
 */


#pragma warning(disable: 4663 4018 4663 4245)
#include "Client.h"
#include "protocol.h"
#include "TCP_Socket.h"
#include <iostream.h>
#include "playerman/player.h"
#include "ship/ship.h"
#include <time.h>
#include "network/multi_log.h"


#define PXO_ADDINT(n)	*((int *)cur) = (n); cur += sizeof(int);
#define PXO_ADDSTRING(x, y) memcpy(cur, x, y); cur += y;

// need to compile and link TCP_Socket.cpp

//**************************************************************************************************************************************************

int CheckSingleMission(const char* mission, unsigned int crc32, TCP_Socket &Socket, const char* masterserver, int port, int timeout)
{
	
	// Clear any old dead crap data
	Socket.IgnorePackets();

	timeout = timeout * CLK_TCK;
	int starttime = clock();

	std::string sender = masterserver;

	// send Packet
	fs2open_file_check_single checkpack;
	memset((char *) &checkpack, 0, sizeof(fs2open_file_check_single));
	checkpack.pid = PCKT_MISSION_CHECK;
	strncpy(checkpack.name, mission, 60);
	checkpack.crc32 = crc32;


		
	if (Socket.SendData((char *) &checkpack, sizeof(fs2open_file_check_single)) == -1)
		return NULL;


	fs2open_fcheck_reply c_reply;

	while ((clock() - starttime) <= timeout)
	{

		if (Socket.MT_DataReady() && Socket.GetData((char *) &c_reply, sizeof(fs2open_fcheck_reply)) != -1)
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
	if (Socket.SendData(PacketBuffer, packet_size) == -1)
		return NULL;


	fs2open_pilot_updatereply u_reply;

	while ((clock() - starttime) <= timeout)
	{

		if (Socket.MT_DataReady() && Socket.GetData((char *) &u_reply, sizeof(fs2open_pilot_updatereply)) != -1)
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

	timeout = timeout * CLK_TCK;

	memset((char *) &prq_packet, 0, sizeof(fs2open_get_pilot));
	prq_packet.pid = PCKT_PILOT_GET;
	prq_packet.sid = SID;
	strncpy(prq_packet.pilotname, player_name, 64);
	prq_packet.create = CanCreate;

	std::string sender = masterserver;

	// send Packet
	if (Socket.SendData((char *)&prq_packet, sizeof(fs2open_get_pilot)) == -1)
		return NULL;

	char PacketBuffer[16384]; // 16K should be enough i think..... I HOPE!
	fs2open_pilot_reply *p_reply = (fs2open_pilot_reply *) PacketBuffer;
	memset(PacketBuffer, 0, 16384);
	fs2open_ship_typekill *type_kills = (fs2open_ship_typekill *) (PacketBuffer + sizeof(fs2open_pilot_reply) - sizeof(fs2open_ship_typekill *));

	int CheckSize =  sizeof(fs2open_pilot_reply) - sizeof(fs2open_ship_typekill *) - 2; //2 for the weird stinking msvc stuff
	int recvsize = 0, rs2;
	Sleep(2); // give them a second
	int starttime = clock();

	while ((clock() - starttime) <= timeout)
	{


		if (Socket.MT_DataReady() && (recvsize = Socket.GetData((char *) &PacketBuffer, 16384)) != -1)
		{

			if (p_reply->pid != PCKT_PILOT_REPLY) // ignore packet
			{
				continue;
			}

			CheckSize += sizeof(fs2open_ship_typekill) * p_reply->ship_types;

			ml_printf("Precompletion Pilot Update: CheckSize = %d, recvsize = %d", CheckSize, recvsize);
			while (CheckSize - recvsize > 0 && (clock() - starttime) <= timeout)
			{
				if (Socket.MT_DataReady())
				{
					rs2 = Socket.GetData((char *) &PacketBuffer+recvsize, 16384-recvsize);
					ml_printf("Tables Completetion: Got Additional %u bytes", rs2);
					recvsize += rs2;
				}
			}
			ml_printf("PostCompletion Tables Get: CheckSize = %u, recvsize = %u", CheckSize, recvsize);

			// if we timed out
			if ((clock() - starttime) >= timeout)
			{
				ml_printf("Pilot Update completetion timed out");
				break;
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



file_record* GetTablesList(int &numTables, const char *masterserver, TCP_Socket &Socket, int port, int timeout)
{
	// Clear any old dead crap data
	Socket.IgnorePackets();

	fs2open_file_check fc_packet;
	fc_packet.pid = PCKT_TABLES_RQST;

	timeout = timeout * CLK_TCK;

	std::string sender = masterserver;

	// send Packet
	if (Socket.SendData((char *)&fc_packet, sizeof(fs2open_file_check)) == -1)
		return NULL;

	char PacketBuffer[16384]; // 16K should be enough i think..... I HOPE!
	fs2open_pxo_missreply *misreply_ptr = (fs2open_pxo_missreply *) PacketBuffer;
	
	file_record *frecs = NULL;
	numTables = 0;
	Sleep(2); //lets give them a second

	int starttime = clock();

	int recvsize = 0, rs2;
	int CheckSize = sizeof(int) * 2;

	while ((clock() - starttime) <= timeout)
	{

		if (Socket.MT_DataReady() && (recvsize = Socket.GetData(PacketBuffer, 16384)) != -1)
		{
			if (misreply_ptr->pid != PCKT_TABLES_REPLY)
			{
				continue; // skip and ignore this packet
			}
			

			CheckSize += misreply_ptr->num_files * sizeof(file_record);

			ml_printf("Precompletion Table Get: CheckSize = %d, recvsize = %d", CheckSize, recvsize);
			while (CheckSize - recvsize > 0 && (clock() - starttime) <= timeout)
			{
				if (Socket.MT_DataReady())
				{
					rs2 = Socket.GetData(PacketBuffer+recvsize, 16384-recvsize);
					ml_printf("Tables Completetion: Got Additional %u bytes", rs2);
					recvsize += rs2;
				}
			}
			ml_printf("PostCompletion Tables Get: CheckSize = %u, recvsize = %u", CheckSize, recvsize);

			if ((clock() - starttime) >= timeout)
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

file_record* GetMissionsList(int &numMissions, const char *masterserver, TCP_Socket &Socket, int port, int timeout)
{
	// Clear any old dead crap data
	Socket.IgnorePackets();

	fs2open_file_check fc_packet;
	fc_packet.pid = PCKT_MISSIONS_RQST;

	timeout = timeout * CLK_TCK;

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
	int starttime = clock();

	int recvsize = 0, rs2;
	int CheckSize = sizeof(int) * 2;

	while ((clock() - starttime) <= timeout)
	{

		if (Socket.MT_DataReady() && (recvsize = Socket.GetData(PacketBuffer, 16384)) != -1)
		{

			if (misreply_ptr->pid != PCKT_MISSIONS_REPLY)
			{
				continue; // skip and ignore this packet
			}
			
			CheckSize += misreply_ptr->num_files * sizeof(file_record);

			ml_printf("Precompletion Missions Get: CheckSize = %d, recvsize = %d", CheckSize, recvsize);
			while (CheckSize - recvsize > 0 && (clock() - starttime) <= timeout)
			{
				if (Socket.MT_DataReady())
				{
					rs2 = Socket.GetData(PacketBuffer+recvsize, 16384-recvsize);
					ml_printf("Missions Completetion: Got Additional %u bytes", rs2);
					recvsize += rs2;
				}
			}
			ml_printf("PostCompletion Missions Get: CheckSize = %u, recvsize = %u", CheckSize, recvsize);

			if ((clock() - starttime) >= timeout)
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

	timeout = timeout * CLK_TCK;
	int starttime = clock();

	std::string sender = masterserver;

	// create and send login packet
	fs2open_pxo_login loginpckt;
	memset(&loginpckt, 0, sizeof(loginpckt));
	loginpckt.pid = PCKT_LOGIN_AUTH;
	strncpy(loginpckt.username, username, 64);
	strncpy(loginpckt.password, password, 64);


	if (Socket.SendData((char *) &loginpckt, sizeof(loginpckt)) == -1)
	{
		cout << "Error Sending Packet" << endl;
		return -1;
	}
	
	// await reply
	fs2open_pxo_lreply loginreply;
	while ((clock() - starttime) <= timeout)
	{

		if (Socket.MT_DataReady() && Socket.GetData((char *)&loginreply, sizeof(fs2open_pxo_lreply)) != -1)
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


void SendHeartBeat(const char* masterserver, int targetport, TCP_Socket &Socket, const char* myName, int myNetspeed, int myStatus, int myType, int numPlayers, int myPort)
{
	// Clear any old dead crap data
	Socket.IgnorePackets();

	// ---------- Prepair and send packet ------------	
	serverlist_hb_packet hbpack;
	memset(&hbpack, 0, sizeof(serverlist_hb_packet));

	hbpack.pid = PCKT_SLIST_HB;
	strncpy(hbpack.servername, myName, 64);
	hbpack.netspeed = myNetspeed;
	hbpack.status = myStatus; // forming
	hbpack.type = myType; // cooperative
	hbpack.players = (short) numPlayers;
	hbpack.port = myPort;


	Socket.SendData((char *) &hbpack, sizeof(serverlist_hb_packet));
}

//**************************************************************************************************************************************************


net_server* GetServerList(const char* masterserver, int &numServersFound, TCP_Socket &Socket, int port, int timeout)
{
	// Clear any old dead crap data
	Socket.IgnorePackets();

	numServersFound = 0;

	
	
	timeout = timeout * CLK_TCK;
	int starttime = clock();
	// ---------- Prepair and send request packet ------------	
	serverlist_request_packet rpack;
	rpack.pid = PCKT_SLIST_REQUEST;
	rpack.type = 0xFFFFFFFF;
	rpack.status = 0xFFFFFFFF;
	std::string sender = masterserver;	
	
	Socket.SendData((char *) &rpack, sizeof(serverlist_request_packet));
	

	// ---------- Receive Reply ------------

	serverlist_reply_packet NewServer;
	net_server *retlist, templist[MAX_SERVERS];

	while ((clock() - starttime) <= timeout)
	{
		if (Socket.MT_DataReady() && Socket.GetData((char *)&NewServer, sizeof(serverlist_reply_packet)) != -1)
		{

			if (!strcmp(NewServer.servername, "TERM") && NewServer.status == 0 && NewServer.players == 0 && NewServer.netspeed == 0 && NewServer.type == 0)
			{
				starttime = 0;
				break;
			}
		
			strncpy(templist[numServersFound].servername, NewServer.servername, 65);
			templist[numServersFound].netspeed = NewServer.netspeed;
			templist[numServersFound].status = NewServer.status;
			templist[numServersFound].players = NewServer.players;
			templist[numServersFound].type = NewServer.type;
			templist[numServersFound].port = NewServer.port;
			strncpy(templist[numServersFound].ip, NewServer.ip, 16);

			numServersFound++;
		}

		
	}


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
	fs2open_pingreply rping;
	ping.pid = PCKT_PING;
	ping.time = time(0);
	std::string rctp = target;

	Socket.SendData((char *)&ping, sizeof(fs2open_ping));

	Socket.GetData((char *)&rping, sizeof(fs2open_pingreply));

	return int( float(time(0) - rping.time)/2.0 );
}

