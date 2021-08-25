/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _gtrack_header
#define _gtrack_header

#ifdef _WIN32
#include <in6addr.h>
#else
#include <netinet/in.h>
#endif

//Game Tracker client code header

#define GAMEPORT	9202

#define MAX_NET_RETRIES 30
#define NET_ACK_TIMEOUT 2500
#define NET_GAME_TIMEOUT 300			//time in seconds

#define MAX_GAME_DATA_SIZE	1500		// NOTE: this is larger than MAX_PACKET_SIZE but
										//       is set here to match server code

#define MAX_GENERIC_GAME_NAME_LEN	32

#define MAX_GAME_LISTS_PER_PACKET	10

#define CHANNEL_LEN	33

#define MAX_FREESPACE_PLAYERS	16
#define MAX_FREESPACE_PLAYER_NAME_LEN	32
#define MAX_FREESPACE_MISSION_NAME_LEN	32
#define MAX_FREESPACE_PLAYERS	16

#define	GNT_SERVER_ACK			0
#define	GNT_CLIENT_ACK			1
#define	GNT_GAMESTARTED		2
#define	GNT_GAMEOVER			3
#define	GNT_GAMEUPDATE			4
#define	GNT_GAMELIST_REQ		5
#define	GNT_GAMELIST_DATA		6
#define	GNT_GAME_COUNT_REQ	7
#define	GNT_GAME_COUNT_DATA	8
#define	GNT_GAMELIST_DATA_NEW	9
#define	GNT_GAME_PROBE_STATUS	10
#define	GNT_GAMEUPDATE_STATUS	11

#define	GT_FREESPACE			1
#define	GT_DESCENT3				2
#define	GT_TUBERACER			3
#define	GT_FREESPACE2			4
#define GT_FS2OPEN				5
#define	GT_UNUSED				0

#define GAME_HEADER_ONLY_SIZE		(sizeof(game_packet_header)-MAX_GAME_DATA_SIZE)

#pragma pack(push, 1)
	typedef struct {
		unsigned int len;				//Length of entire packet;
		unsigned char game_type;	//1==freespace (GT_FREESPACE), 2==D3, 3==tuberacer, etc.
		short game_id;				// only used for fs2open, otherwise is part of 16-byte junk space
		char junk[14];				// not used but need constant size for compatibility (SOCKADDR_IN	addr); (originally 16-bytes!!)
		int	type;	//Used to specify what to do ie. Add a new net game (GNT_GAMESTARTED), remove a net game (game over), etc.
		unsigned int	sig;	//Unique identifier for client ACKs (The server always fills this in, the client responds)

		char data[MAX_GAME_DATA_SIZE];
	} game_packet_header;
#pragma pack(pop)

typedef struct {
	char	game_name[MAX_GENERIC_GAME_NAME_LEN];
	int	difficulty;
	int	type;			//game type;
	int	state;
	int	max_players;
	int	current_num_players;
	char	mission_name[MAX_FREESPACE_MISSION_NAME_LEN];
	char	channel[CHANNEL_LEN];
	char	pad[3];		// 3-byte padding for size/alignment
} freespace2_net_game_data;

#define pxo_net_game_data freespace2_net_game_data

typedef struct {
	unsigned char game_type;
	char game_name[MAX_GAME_LISTS_PER_PACKET][MAX_GENERIC_GAME_NAME_LEN];
	char pad[3];	// ..needs 3-byte padding here for alignment..
	unsigned int	game_server[MAX_GAME_LISTS_PER_PACKET];
	unsigned short port[MAX_GAME_LISTS_PER_PACKET];
} game_list_ip4;

typedef struct {
	unsigned char game_type;
	char game_name[MAX_GAME_LISTS_PER_PACKET][MAX_GENERIC_GAME_NAME_LEN];
	char pad[3];			// ..needs 3-byte padding here for alignment..
	in6_addr game_server[MAX_GAME_LISTS_PER_PACKET];
	unsigned short port[MAX_GAME_LISTS_PER_PACKET];
} game_list_ip6;

#define game_list game_list_ip6

typedef struct {
	int	rank;								// Try to find opponents with a rank similar to this
	char channel[CHANNEL_LEN];			// only give us games in this channel	
	char pad[3];					// 3-bytes padding for size/alignment
} filter_game_list_struct;


//Function prototypes

int InitGameTrackerClient(int gametype);
void IdleGameTracker();
void UpdateGameData(void *buffer);
game_list * GetGameList();
void RequestGameList();
void RequestGameListWithFilter(void *filter);
void RequestGameCountWithFilter(void *filter);

// void SendGameOver();
//Start New 7-9-98
int SendGameOver();
//End New 7-9-98

void StartTrackerGame(void *buffer);

void AckPacket(int sig);

//Definitions
#define MAX_GAME_BUFFERS	20					//Thats a lot considering 20 games per game_list struct

#define TRACKER_UPDATE_INTERVAL			240		//300 seconds
#define TRACKER_RESEND_TIME				2000		//2000ms


#endif
