/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cerrno>
#include <netdb.h>
#endif

#ifdef SCP_BSD
#include <sys/socket.h>
#endif

#include "globalincs/pstypes.h"
#include "io/timer.h"
#include "network/multi.h"
#include "network/multi_pxo.h"
#include "network/gtrack.h"
#include "network/ptrack.h"
#include "network/multi_fstracker.h"


// check structs for size compatibility
SDL_COMPILE_TIME_ASSERT(game_packet_header, sizeof(game_packet_header) == 529);
SDL_COMPILE_TIME_ASSERT(freespace2_net_game_data, sizeof(freespace2_net_game_data) == 120);
SDL_COMPILE_TIME_ASSERT(game_list_ip4, sizeof(game_list_ip4) == 384);
SDL_COMPILE_TIME_ASSERT(game_list_ip6, sizeof(game_list_ip6) == 504);
SDL_COMPILE_TIME_ASSERT(in6_addr, sizeof(in6_addr) == 16);
SDL_COMPILE_TIME_ASSERT(filter_game_list_struct, sizeof(filter_game_list_struct) == 40);



//Variables
static SOCKADDR_STORAGE gtrackaddr;

static game_list GameBuffer[MAX_GAME_BUFFERS];
static int GameType;//d3 or fs

static time_t LastTrackerUpdate;
static unsigned int LastSentToTracker;
static unsigned int TrackerAckdUs;
static unsigned int TrackerGameIsRunning;

static game_packet_header TrackerGameData;
static game_packet_header GameListReq;
static game_packet_header TrackAckPacket;
static game_packet_header GameOverPacket;

static freespace2_net_game_data	*FreeSpace2TrackerGameData;

//Start New 7-9-98
static unsigned int LastGameOverPacket;
static unsigned int FirstGameOverPacket;

static int SendingGameOver;
//End New 7-9-98


static int SerializeGamePacket(const game_packet_header *gph, ubyte *data)
{
	unsigned int packet_size = 0;

	PXO_ADD_UINT(gph->len);
	PXO_ADD_DATA(gph->game_type);

	//PXO_ADD_SHORT(gph->game_id);	// not used except with fs2open, part of 'junk'
	PXO_ADD_SHORT(Multi_fs_tracker_game_id);	// just set here instead of 15 other places

	PXO_ADD_DATA(gph->junk); // not used, basically just padding for compatibility
	PXO_ADD_INT(gph->type);
	PXO_ADD_UINT(gph->sig);

	switch (gph->type) {
		// these have no other data
		case GNT_CLIENT_ACK:
		case GNT_GAMEOVER:
			break;

		// this one may or may not have extra data
		case GNT_GAMELIST_REQ: {
			if (gph->len > GAME_HEADER_ONLY_SIZE) {
				Assert(gph->len == (GAME_HEADER_ONLY_SIZE+sizeof(filter_game_list_struct)));

				auto filter = reinterpret_cast<const filter_game_list_struct *>(&gph->data);

				PXO_ADD_INT(filter->rank);
				PXO_ADD_DATA(filter->channel);
				PXO_ADD_DATA(filter->pad);		// for sizing, so gph->len will match
			}

			break;
		}

		// these two packets are the same except for the type
		// the "STATUS" variant is only to tell PXO that we
		// want to know the connectivity probe status
		case GNT_GAMEUPDATE:
		case GNT_GAMEUPDATE_STATUS: {
			auto game_data = reinterpret_cast<const pxo_net_game_data *>(&gph->data);

			PXO_ADD_DATA(game_data->game_name);
			PXO_ADD_INT(game_data->difficulty);
			PXO_ADD_INT(game_data->type);
			PXO_ADD_INT(game_data->state);
			PXO_ADD_INT(game_data->max_players);
			PXO_ADD_INT(game_data->current_num_players);
			PXO_ADD_DATA(game_data->mission_name);

			PXO_ADD_DATA(game_data->channel);
			PXO_ADD_DATA(game_data->pad);		// for sizing, so gph->len will match

			break;
		}

		case GNT_GAME_COUNT_REQ: {
			Assert(gph->len == (GAME_HEADER_ONLY_SIZE+sizeof(filter_game_list_struct)));

			filter_game_list_struct filter;

			SDL_zero(filter);

			memcpy(filter.channel, gph->data, sizeof(filter.channel));

			PXO_ADD_DATA(filter.channel);

			// add in junk data (ignored on server) to make packet size match
			PXO_ADD_INT(filter.rank);
			PXO_ADD_DATA(filter.pad);

			break;
		}

		// we shouldn't be sending any other packet types
		default:
			Int3();
			break;
	}

	Assert(packet_size >= GAME_HEADER_ONLY_SIZE);
	Assert(packet_size == gph->len);

	return static_cast<int>(packet_size);
}

static void DeserializeGamePacket(const ubyte *data, const int data_size, game_packet_header *gph)
{
	int offset = 0;
	int i;

	memset(gph, 0, sizeof(game_packet_header));

	// make sure we received a complete base packet
	if (data_size < static_cast<int>(GAME_HEADER_ONLY_SIZE)) {
		gph->len = 0;
		gph->type = 255;	// invalid = 0xff

		return;
	}

	PXO_GET_UINT(gph->len);
	PXO_GET_DATA(gph->game_type);

	PXO_GET_SHORT(gph->game_id);	// not used except with fs2open, part of 'junk'
	PXO_GET_DATA(gph->junk); // not used, basically just padding for compatibility
	PXO_GET_INT(gph->type);
	PXO_GET_UINT(gph->sig);

	// sanity check data size to make sure we reveived all of the expected packet
	// (the -1 is because psnet2 pops off one byte)
	if (static_cast<int>(gph->len)-1 > data_size) {
		gph->len = 0;
		gph->type = -1;

		return;
	}

	switch (gph->type) {
		case GNT_SERVER_ACK:
			break;

		case GNT_GAMELIST_DATA: {
			auto games = reinterpret_cast<game_list_ip4 *>(&gph->data);

			PXO_GET_DATA(games->game_type);

			for (i = 0; i < MAX_GAME_LISTS_PER_PACKET; i++) {
				PXO_GET_DATA(games->game_name[i]);
			}

			PXO_GET_DATA(games->pad);	// padded bytes for alignment

			for (i = 0; i < MAX_GAME_LISTS_PER_PACKET; i++) {
				PXO_GET_UINT(games->game_server[i]);
			}

			for (i = 0; i < MAX_GAME_LISTS_PER_PACKET; i++) {
				PXO_GET_USHORT(games->port[i]);
			}

			break;
		}

		case GNT_GAMELIST_DATA_NEW: {
			auto games = reinterpret_cast<game_list_ip6 *>(&gph->data);

			PXO_GET_DATA(games->game_type);

			for (i = 0; i < MAX_GAME_LISTS_PER_PACKET; i++) {
				PXO_GET_DATA(games->game_name[i]);
			}

			PXO_GET_DATA(games->pad);	// padded bytes for alignment

			for (i = 0; i < MAX_GAME_LISTS_PER_PACKET; i++) {
				PXO_GET_DATA(games->game_server[i]);
			}

			for (i = 0; i < MAX_GAME_LISTS_PER_PACKET; i++) {
				PXO_GET_USHORT(games->port[i]);
			}

			break;
		}

		case GNT_GAME_COUNT_DATA: {
			int n_users = 0;
			char channel[512];

			PXO_GET_INT(n_users);

			SDL_strlcpy(channel, reinterpret_cast<const char *>(data+offset), SDL_arraysize(channel));
			offset += static_cast<int>(strlen(channel) + 1);

			memcpy(gph->data, &n_users, sizeof(int));
			memcpy(gph->data+sizeof(int), channel, strlen(channel)+1);

			break;
		}

		case GNT_GAME_PROBE_STATUS: {
			int flags = 0;
			int next_try = 0;

			PXO_GET_INT(flags);
			PXO_GET_INT(next_try);

			memcpy(gph->data, &flags, sizeof(int));
			memcpy(gph->data+sizeof(int), &next_try, sizeof(int));

			break;
		}

		default:
			break;
	}

	Assert(offset == data_size);
}


int InitGameTrackerClient(int gametype)
{
	GameType = gametype;
	LastTrackerUpdate = 0;
	switch(gametype)
	{
	case GT_FS2OPEN:
		TrackerGameData.len = GAME_HEADER_ONLY_SIZE+sizeof(freespace2_net_game_data);
		break;

	default:
		Int3();
		return 0;
	}
	TrackerGameData.game_type = static_cast<unsigned char>(gametype);	//1==freespace (GT_FREESPACE), 2==D3, 3==tuberacer, etc.
	TrackerGameData.type = GNT_GAMEUPDATE_STATUS;	//Used to specify what to do ie. Add a new net game (GNT_GAMESTARTED), remove a net game (game over), etc.

	FreeSpace2TrackerGameData = reinterpret_cast<freespace2_net_game_data *>(&TrackerGameData.data);
	
	GameListReq.game_type = static_cast<unsigned char>(gametype);
	GameListReq.type = GNT_GAMELIST_REQ;
	GameListReq.len = GAME_HEADER_ONLY_SIZE;

	TrackAckPacket.game_type = static_cast<unsigned char>(gametype);
	TrackAckPacket.len = GAME_HEADER_ONLY_SIZE;
	TrackAckPacket.type = GNT_CLIENT_ACK;

	GameOverPacket.game_type = static_cast<unsigned char>(gametype);
	GameOverPacket.len = GAME_HEADER_ONLY_SIZE;
	GameOverPacket.type = GNT_GAMEOVER;

	// This would be a good place to resolve the IP based on a domain name
	psnet_get_addr(Multi_options_g.game_tracker_ip, GAMEPORT, &gtrackaddr, ADDR_FLAG_PREFER_IPV4);

	SDL_zero(GameBuffer);

	//Start New 7-9-98
	SendingGameOver = 0;
	//End New 7-9-98

	return 1;
}

void IdleGameTracker()
{
	fd_set read_fds;	           
	struct timeval timeout;
	ubyte packet_data[sizeof(game_packet_header)];
	int packet_length = 0;

	PSNET_TOP_LAYER_PROCESS();

	timeout.tv_sec=0;            
	timeout.tv_usec=0;
	if((TrackerGameIsRunning) && ((time(nullptr)-LastTrackerUpdate)>TRACKER_UPDATE_INTERVAL) && !SendingGameOver)
	{
		//Time to update the tracker again
		packet_length = SerializeGamePacket(&TrackerGameData, packet_data);
		SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
			   reinterpret_cast<LPSOCKADDR>(&gtrackaddr), sizeof(gtrackaddr), PSNET_TYPE_GAME_TRACKER);

		TrackerAckdUs = 0;
		LastTrackerUpdate = time(nullptr);
	}
	else if((TrackerGameIsRunning)&&(!TrackerAckdUs)&&((timer_get_milliseconds()-LastSentToTracker)>TRACKER_RESEND_TIME))
	{
		//We still haven't been acked by the last packet and it's time to resend.
		packet_length = SerializeGamePacket(&TrackerGameData, packet_data);
		SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
			   reinterpret_cast<LPSOCKADDR>(&gtrackaddr), sizeof(gtrackaddr), PSNET_TYPE_GAME_TRACKER);

		TrackerAckdUs = 0;
		LastTrackerUpdate = time(nullptr);
		LastSentToTracker = timer_get_milliseconds();
	}

	//Start New 7-9-98
	if(SendingGameOver){
		if((timer_get_milliseconds()-LastGameOverPacket)>TRACKER_RESEND_TIME){
			//resend
			packet_length = SerializeGamePacket(&GameOverPacket, packet_data);
			LastGameOverPacket = timer_get_milliseconds();
			SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
				   reinterpret_cast<LPSOCKADDR>(&gtrackaddr), sizeof(gtrackaddr), PSNET_TYPE_GAME_TRACKER);
		} 
		/*
		else if((timer_get_milliseconds()-FirstGameOverPacket)>NET_ACK_TIMEOUT) {
			//Giving up, it timed out.
			SendingGameOver = 2;
		}
		*/
	}
	//End New 7-9-98

	//Check for incoming
		
	FD_ZERO(&read_fds);	// NOLINT
	FD_SET(Psnet_socket, &read_fds);

	if(SELECT(static_cast<int>(Psnet_socket+1),&read_fds,nullptr,nullptr,&timeout, PSNET_TYPE_GAME_TRACKER))
	{
		int bytesin;
		int addrsize;
		SOCKADDR_STORAGE fromaddr;

		game_packet_header inpacket;

		SDL_zero(inpacket);
		addrsize = sizeof(fromaddr);

		bytesin = RECVFROM(Psnet_socket, reinterpret_cast<char *>(&packet_data), sizeof(game_packet_header), 0,
						   reinterpret_cast<LPSOCKADDR>(&fromaddr), &addrsize, PSNET_TYPE_GAME_TRACKER);

		if (bytesin > 0) {
			DeserializeGamePacket(packet_data, bytesin, &inpacket);

			// subtract one from the header
			inpacket.len--;
#ifndef NDEBUG
		} else {
			int wserr=WSAGetLastError();
			mprintf(("RECVFROM() failure. WSAGetLastError() returned %d\n",wserr));
#endif
		}

		//Check to make sure the packets ok
		if ( (bytesin > 0) && (bytesin == (int)inpacket.len) )
		{
			switch(inpacket.type)
			{
			case GNT_SERVER_ACK:
				//The server got our packet so we can stop sending now
				TrackerAckdUs = 1;				
				
				// 7/13/98 -- because of the FreeSpace iterative frame process -- set this value to 0, instead
				// of to 2 (as it originally was) since we call SendGameOver() only once.  Once we get the ack
				// from the server, we can assume that we are done.
				// need to mark this as 0
				SendingGameOver = 0;							
				break;
			case GNT_GAMELIST_DATA:
			case GNT_GAMELIST_DATA_NEW:
				int i;
				//Woohoo! Game data! put it in the buffer (if one's free)
				for(i=0;i<MAX_GAME_BUFFERS;i++)
				{
					if(GameBuffer[i].game_type==GT_UNUSED)
					{
						if (inpacket.type == GNT_GAMELIST_DATA)
						{
							// convert to ip6 struct
							auto gl4 = reinterpret_cast<game_list_ip4 *>(&inpacket.data);
							game_list_ip6 gl6;
							in_addr addr;

							gl6.game_type = gl4->game_type;
							memcpy(&gl6.game_name, &gl4->game_name, sizeof(gl6.game_name));
							memcpy(&gl6.port, &gl4->port, sizeof(gl6.port));

							for (int j = 0; j < MAX_GAME_LISTS_PER_PACKET; j++)
							{
								addr.s_addr = gl4->game_server[j];
								psnet_map4to6(&addr, &gl6.game_server[j]);
							}

							memcpy(&GameBuffer[i], &gl6, sizeof(game_list));
						}
						else
						{
							memcpy(&GameBuffer[i], &inpacket.data, sizeof(game_list));
						}
						i=MAX_GAME_BUFFERS+1;
					}
				}
				break;

			case GNT_GAME_COUNT_DATA:
				//Here, inpacket.data contains the following structure
				//struct {
				//	int numusers;
				//	char channel[];//Null terminated
				//	}
				//You can add whatever code, or callback, etc. you need to deal with this data

				// let the PXO screen know about this data
				int num_servers;
				char channel[512];

				// get the user count
				memcpy(&num_servers,inpacket.data,sizeof(int));

				// copy the channel name
				SDL_strlcpy(channel, inpacket.data+sizeof(int), SDL_arraysize(channel));

				// send it to the PXO screen				
				multi_pxo_channel_count_update(channel,num_servers);
				break;

			case GNT_GAME_PROBE_STATUS:
				int flags;
				int next_try;

				memcpy(&flags, inpacket.data, sizeof(int));
				memcpy(&next_try, inpacket.data+sizeof(int), sizeof(int));

				// tell user about it
				multi_fs_tracker_report_probe_status(flags, next_try);
				break;
			}
			AckPacket(inpacket.sig);			
		}
	}
}

void UpdateGameData(void *buffer)
{
	SendingGameOver = 0;

	switch(GameType){
	case GT_FS2OPEN:
		memcpy(FreeSpace2TrackerGameData,buffer,sizeof(freespace2_net_game_data));
		break;

	default:
		Int3();
		break;
	}

	// forces update
	LastTrackerUpdate = 0;
}

game_list * GetGameList()
{
	static game_list gl;

	for (auto &game : GameBuffer) {
		if (game.game_type != GT_UNUSED) {
			memcpy(&gl, &game, sizeof(game_list));
			game.game_type = GT_UNUSED;
			return &gl;
		}
	}

	return nullptr;
}

void RequestGameList()
{
	ubyte packet_data[sizeof(game_packet_header)];
	int packet_length = 0;

	GameListReq.len = GAME_HEADER_ONLY_SIZE;

	packet_length = SerializeGamePacket(&GameListReq, packet_data);
	SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
		   reinterpret_cast<LPSOCKADDR>(&gtrackaddr), sizeof(gtrackaddr), PSNET_TYPE_GAME_TRACKER);
}

void RequestGameListWithFilter(void *filter)
{
	ubyte packet_data[sizeof(game_packet_header)];
	int packet_length = 0;

	memcpy(&GameListReq.data,filter,sizeof(filter_game_list_struct));
	GameListReq.len = GAME_HEADER_ONLY_SIZE+sizeof(filter_game_list_struct);

	packet_length = SerializeGamePacket(&GameListReq, packet_data);
	SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
		   reinterpret_cast<LPSOCKADDR>(&gtrackaddr), sizeof(gtrackaddr), PSNET_TYPE_GAME_TRACKER);
}


/* REPLACED BELOW
void SendGameOver()
{
	TrackerGameIsRunning = 0;
	sendto(gamesock,(const char *)&GameOverPacket,GameOverPacket.len,0,(SOCKADDR *)&gtrackaddr,sizeof(SOCKADDR_IN));
}
*/

//Start New 7-9-98
int SendGameOver()
{
	ubyte packet_data[sizeof(game_packet_header)];
	int packet_length = 0;

	if(SendingGameOver==2) 
	{
		SendingGameOver = 0;	
		return 1;
	}
	if(SendingGameOver==1) 
	{
		//Wait until it's sent.
		IdleGameTracker();
		return 0;
	}
	if(SendingGameOver==0)
	{
		LastGameOverPacket = timer_get_milliseconds();
		FirstGameOverPacket = timer_get_milliseconds();
		SendingGameOver = 1;
		TrackerGameIsRunning = 0;

		packet_length = SerializeGamePacket(&GameOverPacket, packet_data);
		SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
			   reinterpret_cast<LPSOCKADDR>(&gtrackaddr), sizeof(gtrackaddr), PSNET_TYPE_GAME_TRACKER);

		return 0;
	}
	return 0;
}
//End New 7-9-98

void AckPacket(int sig)
{
	ubyte packet_data[sizeof(game_packet_header)];
	int packet_length = 0;

	TrackAckPacket.sig = sig;

	packet_length = SerializeGamePacket(&TrackAckPacket, packet_data);
	SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
		   reinterpret_cast<LPSOCKADDR>(&gtrackaddr), sizeof(gtrackaddr), PSNET_TYPE_GAME_TRACKER);
}

void StartTrackerGame(void *buffer)
{
	SendingGameOver = 0;

	switch(GameType){
	case GT_FS2OPEN:
		memcpy(FreeSpace2TrackerGameData,buffer,sizeof(freespace2_net_game_data));
		break;

	default:
		Int3();
		break;
	}
	TrackerGameIsRunning = 1;
	LastTrackerUpdate = 0;	
}

//A new function
void RequestGameCountWithFilter(void *filter) 
{
	game_packet_header GameCountReq;
	ubyte packet_data[sizeof(game_packet_header)];
	int packet_length = 0;

	GameCountReq.game_type = GT_FS2OPEN;
	GameCountReq.type = GNT_GAME_COUNT_REQ;
	GameCountReq.len = GAME_HEADER_ONLY_SIZE+sizeof(filter_game_list_struct);
	memcpy(&GameCountReq.data, reinterpret_cast<const filter_game_list_struct *>(filter)->channel, CHANNEL_LEN);

	packet_length = SerializeGamePacket(&GameCountReq, packet_data);
	SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
		   reinterpret_cast<LPSOCKADDR>(&gtrackaddr), sizeof(gtrackaddr), PSNET_TYPE_GAME_TRACKER);
}
