// Client.h
// Client Functions for FS2Open PXO
// Derek Meek
// 2-14-2003

/*
 * $Logfile: /Freespace2/code/fs2open_pxo/Client.h $
 * $Revision: 1.7 $
 * $Date: 2003-10-13 06:02:50 $
 * $Author: Kazan $
 *
 *
 * $Log: not supported by cvs2svn $
 *
 *
 */

#pragma warning(disable:4100)
#pragma warning(disable:4511)
#pragma warning(disable:4512)
#pragma warning(disable:4663)
#pragma warning(disable:4018)

#if !defined(__pxo_client_h_)
#define __pxo_client_h_

#define MAX_SERVERS 512
#include "protocol.h"

#if !defined(PXO_TCP)
#include "udpsocket.h"
#else
#include "TCP_Socket.h"
#endif

// Getting linked into FS2 Here
#include "Playerman/Player.h"



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
