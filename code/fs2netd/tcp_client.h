// Client.h
// Client Functions for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################




#if !defined(__pxo_client_h_)
#define __pxo_client_h_


#include "globalincs/pstypes.h"

#include "fs2netd/protocol.h"
#include "fs2netd/tcp_socket.h"

#include <string>

struct player;
struct netgame_info;


int FS2NetD_CheckSingleMission(const char *m_name, uint crc32, bool do_send);

int FS2NetD_SendPlayerData(const char *player_name, player *pl, bool do_send);
int FS2NetD_GetPlayerData(const char *player_name, player *pl, bool can_create, bool do_send);

int FS2NetD_GetBanList(SCP_vector<SCP_string> &mask_list, bool do_send);

int FS2NetD_GetMissionsList(SCP_vector<file_record> &m_list, bool do_send);

int FS2NetD_ValidateTableList(bool do_send);

int FS2NetD_Login(const char *username, const char *password, bool do_send);
int FS2NetD_CheckValidID();
void FS2NetD_CheckDuplicateLogin();

void FS2NetD_SendServerStart();
void FS2NetD_SendServerUpdate();
void FS2NetD_SendServerDisconnect();

void FS2NetD_RequestServerList();

void FS2NetD_GameCountUpdate(char *chan_name);

void FS2NetD_Ping();
void FS2NetD_Pong(int tstamp);



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
