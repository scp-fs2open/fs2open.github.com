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
 * $Revision: 1.1.2.1 $
 * $Date: 2007-10-15 06:43:10 $
 * $Author: taylor $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.20  2006/01/26 03:23:29  Goober5000
 * pare down the pragmas some more
 * --Goober5000
 *
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

//#pragma warning(disable:4018)	// signed/unsigned mismatch
//#pragma warning(disable:4245)	// signed/unsigned mismatch in conversion of const value


#include "fs2netd/protocol.h"
#include "fs2netd/tcp_socket.h"

struct player;
struct netgame_info;

int FS2NetD_SendPlayerData(int SID, const char* player_name, const char* user, player *pl, int timeout = 15);
int FS2NetD_GetPlayerData(int SID, const char* player_name, player *pl, bool CanCreate = false, int timeout = 15);
int FS2NetD_CheckSingleMission(const char *m_name, uint crc32, int timeout = 15);
int FS2NetD_CheckValidSID(int SID);
int FS2NetD_GetServerList(int timeout = 15);
void FS2NetD_Ping();
void FS2NetD_SendHeartBeat();
void FS2NetD_SendServerDisconnect(ushort port);
int FS2NetD_Login(const char *username, const char *password, int timeout = 15);
void FS2NetD_Pong(int tstamp);
int FS2NetD_ValidateTableList(int timeout = 30);
void FS2NetD_ChatChannelUpdate(char *chan_name);
void FS2NetD_GameCountUpdate(char *chan_name);
void FS2NetD_CheckDuplicateLogin(int SID);
fs2open_banmask *FS2NetD_GetBanList(int *numBanMasks, int timeout = 15);

// longer timeouts - mySQL operations
//file_record *FS2NetD_GetTablesList(int *numTables, int timeout = 30);
file_record *FS2NetD_GetMissionsList(int *numMissions, int timeout = 30);



// Some easy to use macros for handling the packet data
#define BASE_PACKET_SIZE	(int)(sizeof(ubyte) + sizeof(int))

#define PXO_ADD_DATA(d) do { Assert(buffer_size+sizeof(d) <= sizeof(buffer)); memcpy(buffer+buffer_size, &d, sizeof(d)); buffer_size += sizeof(d); } while (0)
#define PXO_ADD_SHORT(d) do { Assert(buffer_size+sizeof(short) <= sizeof(buffer)); short swap = INTEL_SHORT(d); memcpy(buffer+buffer_size, &swap, sizeof(short)); buffer_size += sizeof(short); } while (0)
#define PXO_ADD_USHORT(d) do { Assert(buffer_size+sizeof(ushort) <= sizeof(buffer)); ushort swap = INTEL_SHORT(d); memcpy(buffer+buffer_size, &swap, sizeof(ushort)); buffer_size += sizeof(ushort); } while (0)
#define PXO_ADD_INT(d) do { Assert(buffer_size+sizeof(int) <= sizeof(buffer)); int swap = INTEL_INT(d); memcpy(buffer+buffer_size, &swap, sizeof(int)); buffer_size += sizeof(int); } while (0)
#define PXO_ADD_UINT(d) do { Assert(buffer_size+sizeof(uint) <= sizeof(buffer)); uint swap = INTEL_INT(d); memcpy(buffer+buffer_size, &swap, sizeof(uint)); buffer_size += sizeof(uint); } while (0)
#define PXO_ADD_STRING(s) do { Assert(buffer_size+strlen(s)+sizeof(int) <= sizeof(buffer)); int len = strlen(s); PXO_ADD_INT(len); if (len > 0) { memcpy(buffer+buffer_size, s, len ); buffer_size += len; } } while(0)

#define PXO_GET_DATA(d) do { Assert(buffer_offset+sizeof(d) <= sizeof(buffer)); memcpy(&d, buffer+buffer_offset, sizeof(d) ); buffer_offset += sizeof(d); } while(0)
#define PXO_GET_SHORT(d) do { Assert(buffer_offset+sizeof(short) <= sizeof(buffer)); short swap; memcpy(&swap, buffer+buffer_offset, sizeof(short) ); d = INTEL_SHORT(swap); buffer_offset += sizeof(short); } while(0)
#define PXO_GET_USHORT(d) do { Assert(buffer_offset+sizeof(ushort) <= sizeof(buffer)); ushort swap; memcpy(&swap, buffer+buffer_offset, sizeof(ushort) ); d = INTEL_SHORT(swap); buffer_offset += sizeof(ushort); } while(0)
#define PXO_GET_INT(d) do { Assert(buffer_offset+sizeof(int) <= sizeof(buffer)); int swap; memcpy(&swap, buffer+buffer_offset, sizeof(int) ); d = INTEL_INT(swap); buffer_offset += sizeof(int); } while(0)
#define PXO_GET_UINT(d) do { Assert(buffer_offset+sizeof(uint) <= sizeof(buffer)); uint swap; memcpy(&swap, buffer+buffer_offset, sizeof(uint) ); d = INTEL_INT(swap); buffer_offset += sizeof(uint); } while(0)
#define PXO_GET_STRING(s) do { Assert(buffer_offset+sizeof(int) <= sizeof(buffer)); s[0] = '\0'; int len; memcpy(&len, buffer+buffer_offset, sizeof(int)); len = INTEL_INT(len); buffer_offset += sizeof(int); if (len > 0) { memcpy(s, buffer+buffer_offset, len); buffer_offset += len; s[len] = '\0'; } } while(0)

// initialize a packet
#define INIT_PACKET(x) { memset(buffer, 0, sizeof(buffer)); buffer_size = 0; ubyte pckt = (x); PXO_ADD_DATA(pckt); PXO_ADD_INT(buffer_size); }
// we are done with a new packet, so update the final packet size
#define DONE_PACKET() { int swap = INTEL_INT(buffer_size); memcpy(buffer+sizeof(ubyte), &swap, sizeof(buffer_size)); }
// verify received packet
#define VRFY_PACKET(x) { buffer_offset = 0; ubyte pckt; PXO_GET_DATA(pckt); if (pckt != (x)) break; my_packet = true; PXO_GET_INT(buffer_size); }
#define VRFY_PACKET2(x) { buffer_offset = 0; ubyte pckt; PXO_GET_DATA(pckt); if (pckt == (x)) my_packet = true; PXO_GET_INT(buffer_size); }


#endif
