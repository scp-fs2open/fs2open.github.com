// Client.h
// Client Functions for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################

/*
 * $Logfile: /Freespace2/code/fs2open_pxo/Client.h $
 * $Revision: 1.11 $
 * $Date: 2004-03-05 09:01:56 $
 * $Author: Goober5000 $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.10  2004/02/21 00:59:43  Kazan
 * FS2NETD License Comments
 *
 * Revision 1.9  2003/11/11 02:15:42  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 1.8  2003/11/06 20:22:05  Kazan
 * slight change to .dsp - leave the release target as fs2_open_r.exe already
 * added myself to credit
 * killed some of the stupid warnings (including doing some casting and commenting out unused vars in the graphics modules)
 * Release builds should have warning level set no higher than 2 (default is 1)
 * Why are we getting warning's about function selected for inline expansion... (killing them with warning disables)
 * FS2_SPEECH was not defined (source file doesn't appear to capture preproc defines correctly either)
 *
 * Revision 1.7  2003/10/13 06:02:50  Kazan
 * Added Log Comment Thingy to these files
 *
 *
 *
 */

#pragma warning(disable:4018)
#pragma warning(disable:4100)
#pragma warning(disable:4511)
#pragma warning(disable:4512)
#pragma warning(disable:4663)
#pragma warning(disable:4710)

#if !defined(__pxo_client_h_)
#define __pxo_client_h_

#define MAX_SERVERS 512
#include "protocol.h"

#if !defined(PXO_TCP)
#include "udpsocket.h"
#else
#include "TCP_Socket.h"
#endif

struct player;

struct net_server
{
      char servername[65];
      int netspeed;
      int status;
      short players;
      int type; // binary bitmask for type and dedicated server
      char  ip[16];
      int ping; // will be determined by client
	  int port;
};



#if !defined(PXO_TCP)
// ********************************************************************************************************
// UDP Version of PXO
// ********************************************************************************************************
// Variants of the above functions with persistant connections
int SendPlayerData(int SID, const char* player_name, const char* user, player *pl, const char* masterserver, UDP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=15);
int GetPlayerData(int SID, const char* player_name, player *pl, const char* masterserver, UDP_Socket &Socket, int port=FS2OPEN_PXO_PORT, bool CanCreate=false, int timeout=15);
int CheckSingleMission(const char* mission, unsigned int crc32, UDP_Socket &Socket, const char* masterserver, int port=FS2OPEN_PXO_PORT, int timeout=15);

net_server* GetServerList(const char* masterserver, int &numServersFound, UDP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=15);
int Ping(const char* target, UDP_Socket &Socket);
void SendHeartBeat(const char* masterserver, int targetport, UDP_Socket &Socket, const char* myName, int myNetspeed, int myStatus, int myType, int numPlayers, int myPort);
int Fs2OpenPXO_Login(const char* username, const char* password, UDP_Socket &Socket, const char* masterserver, int port=FS2OPEN_PXO_PORT, int timeout=15);


// longer timeouts - mySQL operations
file_record* GetTablesList(int &numTables, const char *masterserver, UDP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=30);
file_record* GetMissionsList(int &numMissions, const char *masterserver, UDP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=30);
#else
// ********************************************************************************************************
// TCP Version of PXO
// ********************************************************************************************************
int SendPlayerData(int SID, const char* player_name, const char* user, player *pl, const char* masterserver, TCP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=15);
int GetPlayerData(int SID, const char* player_name, player *pl, const char* masterserver, TCP_Socket &Socket, int port=FS2OPEN_PXO_PORT, bool CanCreate=false, int timeout=15);
int CheckSingleMission(const char* mission, unsigned int crc32, TCP_Socket &Socket, const char* masterserver, int port=FS2OPEN_PXO_PORT, int timeout=15);

net_server* GetServerList(const char* masterserver, int &numServersFound, TCP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=15);
int Ping(const char* target, TCP_Socket &Socket);
void SendHeartBeat(const char* masterserver, int targetport, TCP_Socket &Socket, const char* myName, int myNetspeed, int myStatus, int myType, int numPlayers, int myPort);
int Fs2OpenPXO_Login(const char* username, const char* password, TCP_Socket &Socket, const char* masterserver, int port=FS2OPEN_PXO_PORT, int timeout=15);


// longer timeouts - mySQL operations
file_record* GetTablesList(int &numTables, const char *masterserver, TCP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=30);
file_record* GetMissionsList(int &numMissions, const char *masterserver, TCP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=30);
#endif


#endif
