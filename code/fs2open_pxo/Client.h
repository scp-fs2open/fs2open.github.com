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
 * $Revision: 1.20 $
 * $Date: 2006-01-26 03:23:29 $
 * $Author: Goober5000 $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.19  2005/07/13 02:50:49  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.18  2005/03/02 21:18:18  taylor
 * better support for Inferno builds (in PreProcDefines.h now, no networking support)
 * make sure NO_NETWORK builds are as friendly on Windows as it is on Linux/OSX
 * revert a timeout in Client.h back to the original value before Linux merge
 *
 * Revision 1.17  2005/02/04 20:06:03  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 1.16  2004/08/11 05:06:23  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 1.15  2004/07/07 21:00:06  Kazan
 * FS2NetD: C2S Ping/Pong, C2S Ping/Pong, Global IP Banlist, Global Network Messages
 *
 * Revision 1.14  2004/03/31 05:42:26  Goober5000
 * got rid of all those nasty warnings from xlocale and so forth; also added comments
 * to indicate which warnings were being disabled
 * --Goober5000
 *
 * Revision 1.13  2004/03/09 00:02:16  Kazan
 * more fs2netd stuff
 *
 * Revision 1.12  2004/03/07 23:07:20  Kazan
 * [Incomplete] Readd of Software renderer so Standalone server works
 *
 * Revision 1.11  2004/03/05 09:01:56  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
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


#if !defined(__pxo_client_h_)
#define __pxo_client_h_


#include "globalincs/pstypes.h"

#pragma warning(disable:4018)	// signed/unsigned mismatch
#pragma warning(disable:4245)	// signed/unsigned mismatch in conversion of const value


#define MAX_SERVERS 512
#include "fs2open_pxo/protocol.h"

#if !defined(PXO_TCP)
#include "fs2open_pxo/udpsocket.h"
typedef UDP_Socket PXO_Socket;
#else
#include "fs2open_pxo/TCP_Socket.h"
typedef TCP_Socket PXO_Socket;
#endif

extern PXO_Socket FS2OpenPXO_Socket;

struct player;

struct net_server
{
      int pid; // 0x2 : serverlist reply (PCKT_SLIST_REPLY)
      char name[65];
	  char mission_name[65];
	  char title[65];
      short players;
      int flags;

	  char  ip[16]; // "255.255.255.255"
	  int port;
};

int SendPlayerData(int SID, const char* player_name, const char* user, player *pl, const char* masterserver, PXO_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=15);
int GetPlayerData(int SID, const char* player_name, player *pl, const char* masterserver, PXO_Socket &Socket, int port=FS2OPEN_PXO_PORT, bool CanCreate=false, int timeout=15);
int CheckSingleMission(const char* mission, unsigned int crc32, PXO_Socket &Socket, const char* masterserver, int port=FS2OPEN_PXO_PORT, int timeout=15);

net_server* GetServerList(const char* masterserver, int &numServersFound, PXO_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=15);
int Ping(const char* target, PXO_Socket &Socket);
void SendHeartBeat(const char* masterserver, int targetport, PXO_Socket &Socket, const char* myName, const char* MisName, const char* title, int flags, int port, int players);
int Fs2OpenPXO_Login(const char* username, const char* password, PXO_Socket &Socket, const char* masterserver, int port=FS2OPEN_PXO_PORT, int timeout=15);
int GetPingReply(PXO_Socket &Socket);
void SendPingReply(PXO_Socket &Socket, int tstamp);
fs2open_banmask* GetBanList(int &numBanMasks, PXO_Socket &Socket, int timeout=15);

// longer timeouts - mySQL operations
file_record* GetTablesList(int &numTables, const char *masterserver, PXO_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=30);
file_record* GetMissionsList(int &numMissions, const char *masterserver, PXO_Socket &Socket, int port=FS2OPEN_PXO_PORT, int timeout=30);


#endif
