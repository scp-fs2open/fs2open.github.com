// Client.h
// Client Functions for FS2Open PXO
// Derek Meek
// 2-14-2003

#if !defined(__pxo_client_h_)
#define __pxo_client_h_

#define MAX_SERVERS 512
#include "protocol.h"
#include "udpsocket.h"

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
};



// Variants of the above functions with persistant connections
int SendPlayerData(int SID, const char* player_name, const char* user, player *pl, const char* masterserver, UDP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=15);
int GetPlayerData(int SID, const char* player_name, player *pl, const char* masterserver, UDP_Socket &Socket, int port=FS2OPEN_PXO_PORT, bool CanCreate=false, int timeout=15);
int CheckSingleMission(const char* mission, unsigned int crc32, UDP_Socket &Socket, const char* masterserver, int port=FS2OPEN_PXO_PORT, int timeout=15);

net_server* GetServerList(const char* masterserver, int &numServersFound, UDP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=15);
int Ping(const char* target, UDP_Socket &Socket);
void SendHeartBeat(const char* masterserver, int targetport, UDP_Socket &Socket, const char* myName, int myNetspeed, int myStatus, int myType, int numPlayers);
int Fs2OpenPXO_Login(const char* username, const char* password, UDP_Socket &Socket, const char* masterserver, int port=FS2OPEN_PXO_PORT, int timeout=15);


// longer timeouts - mySQL operations
file_record* GetTablesList(int &numTables, const char *masterserver, UDP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=30);
file_record* GetMissionsList(int &numMissions, const char *masterserver, UDP_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=30);

#endif
