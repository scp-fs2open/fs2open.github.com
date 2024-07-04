/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

//Pilot tracker client code

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
#include "network/ptrack.h"
#include "network/psnet2.h"
#include "network/multi_fstracker.h"


// check structs for size compatibility
SDL_COMPILE_TIME_ASSERT(udp_packet_header, sizeof(udp_packet_header) == 517);
SDL_COMPILE_TIME_ASSERT(vmt_fs2open_struct, sizeof(vmt_fs2open_struct) == 480);
SDL_COMPILE_TIME_ASSERT(validate_id_request, sizeof(validate_id_request) == 60);
SDL_COMPILE_TIME_ASSERT(squad_war_request, sizeof(squad_war_request) == 104);
SDL_COMPILE_TIME_ASSERT(squad_war_response, sizeof(squad_war_response) == 256);
SDL_COMPILE_TIME_ASSERT(squad_war_result, sizeof(squad_war_result) == 72);
SDL_COMPILE_TIME_ASSERT(pilot_request, sizeof(pilot_request) == 32);


//Variables

static SOCKADDR_STORAGE ptrackaddr;


static int		FSWriteState;
static int		FSReadState;
static unsigned int	FSLastSentWrite;
static unsigned int	FSFirstSentWrite;
static unsigned int	FSLastSent;
static unsigned int	FSFirstSent;

static int		SWWriteState;
static unsigned int SWLastSentWrite;
static unsigned int SWFirstSentWrite;


static udp_packet_header fs_pilot_req, fs_pilot_write, sw_res_write;
static pilot_request *fs_pr;
static vmt_stats_struct *ReadFSPilot;

// squad war response
static squad_war_response SquadWarWriteResponse;


static int SerializePilotPacket(const udp_packet_header *uph, ubyte *data)
{
	unsigned int packet_size = 0;
	int i;

	PXO_ADD_DATA(uph->type);
	PXO_ADD_USHORT(uph->len);
	PXO_ADD_UINT(uph->code);
	PXO_ADD_USHORT(uph->xcode);
	PXO_ADD_UINT(uph->sig);
	PXO_ADD_UINT(uph->security);

	switch (uph->type) {
		// no extra data for this
		case UNT_CONTROL:
			break;

		case UNT_PILOT_DATA_WRITE_NEW: {
			auto fsos = reinterpret_cast<const vmt_stats_struct *>(&uph->data);

			PXO_ADD_INT(fsos->tracker_id);
			PXO_ADD_DATA(fsos->pilot_name);

			PXO_ADD_INT(fsos->score);
			PXO_ADD_INT(fsos->rank);
			PXO_ADD_INT(fsos->assists);
			PXO_ADD_INT(fsos->kill_count);
			PXO_ADD_INT(fsos->kill_count_ok);
			PXO_ADD_UINT(fsos->p_shots_fired);
			PXO_ADD_UINT(fsos->s_shots_fired);

			PXO_ADD_UINT(fsos->p_shots_hit);
			PXO_ADD_UINT(fsos->s_shots_hit);

			PXO_ADD_UINT(fsos->p_bonehead_hits);
			PXO_ADD_UINT(fsos->s_bonehead_hits);
			PXO_ADD_INT(fsos->bonehead_kills);

			PXO_ADD_UINT(fsos->missions_flown);
			PXO_ADD_UINT(fsos->flight_time);
			PXO_ADD_UINT(fsos->last_flown);

			PXO_ADD_INT(fsos->security);
			PXO_ADD_UINT(fsos->checksum);

			PXO_ADD_DATA(fsos->virgin_pilot);

			// skip 1-byte pad

			PXO_ADD_DATA(fsos->num_medals);
			PXO_ADD_DATA(fsos->num_ships);

			const int count = std::min(static_cast<unsigned short>(fsos->num_medals + fsos->num_ships), MAX_FS2OPEN_COUNTS);

			for (i = 0; i < count; i++) {
				PXO_ADD_USHORT(fsos->counts[i]);
			}

			break;
		}

		case UNT_SW_RESULT_WRITE: {
			auto sw_result = reinterpret_cast<const squad_war_result *>(&uph->data);

			PXO_ADD_DATA(sw_result->match_code);
			PXO_ADD_DATA(sw_result->result);
			PXO_ADD_DATA(sw_result->squad_count1);
			PXO_ADD_DATA(sw_result->squad_count2);
			PXO_ADD_DATA(sw_result->pad);		// junk, for size/alignment

			for (i = 0; i < MAX_SQUAD_PLAYERS; i++) {
				PXO_ADD_INT(sw_result->squad_winners[i]);
			}

			for (i = 0; i < MAX_SQUAD_PLAYERS; i++) {
				PXO_ADD_INT(sw_result->squad_losers[i]);
			}

			break;
		}

		case UNT_PILOT_DATA_READ_NEW:
		case UNT_PILOT_DATA_READ: {
			auto pr = reinterpret_cast<const pilot_request *>(&uph->data);

			PXO_ADD_DATA(pr->pilot_name);
			PXO_ADD_DATA(pr->tracker_id);
			PXO_ADD_DATA(pr->pad);		// junk, for size/alignment

			break;
		}

		// we shouldn't be sending any other packet types
		default:
			Int3();
			break;
	}

	Assert(packet_size >= PACKED_HEADER_ONLY_SIZE);
	Assert(packet_size == uph->len);
	Assert(packet_size <= MAX_PACKET_SIZE);

	return static_cast<int>(packet_size);
}

static void DeserializePilotPacket(const ubyte *data, const int data_size, udp_packet_header *uph)
{
	int offset = 0;
	int i;

	memset(uph, 0, sizeof(udp_packet_header));

	// make sure we received a complete base packet
	if (data_size < static_cast<int>(PACKED_HEADER_ONLY_SIZE)) {
		uph->len = 0;
		uph->type = 0xff;

		return;
	}

	PXO_GET_DATA(uph->type);
	PXO_GET_USHORT(uph->len);
	PXO_GET_UINT(uph->code);
	PXO_GET_USHORT(uph->xcode);
	PXO_GET_UINT(uph->sig);
	PXO_GET_UINT(uph->security);

	// sanity check data size to make sure we reveived all of the expected packet
	// (the -1 is because psnet2 pops off one byte)
	if (static_cast<int>(uph->len)-1 > data_size) {
		uph->len = 0;
		uph->type = 0xff;

		return;
	}

	switch (uph->type) {
		// no extra data for these
		case UNT_PILOT_READ_FAILED:
		case UNT_PILOT_WRITE_SUCCESS:
		case UNT_PILOT_WRITE_FAILED:
			break;

		case UNT_PILOT_DATA_RESPONSE: {
			auto fsos = reinterpret_cast<vmt_stats_struct *>(&uph->data);

			PXO_GET_INT(fsos->tracker_id);
			PXO_GET_DATA(fsos->pilot_name);

			PXO_GET_INT(fsos->score);
			PXO_GET_INT(fsos->rank);
			PXO_GET_INT(fsos->assists);
			PXO_GET_INT(fsos->kill_count);
			PXO_GET_INT(fsos->kill_count_ok);
			PXO_GET_UINT(fsos->p_shots_fired);
			PXO_GET_UINT(fsos->s_shots_fired);

			PXO_GET_UINT(fsos->p_shots_hit);
			PXO_GET_UINT(fsos->s_shots_hit);

			PXO_GET_UINT(fsos->p_bonehead_hits);
			PXO_GET_UINT(fsos->s_bonehead_hits);
			PXO_GET_INT(fsos->bonehead_kills);

			PXO_GET_UINT(fsos->missions_flown);
			PXO_GET_UINT(fsos->flight_time);
			PXO_GET_UINT(fsos->last_flown);

			PXO_GET_INT(fsos->security);
			PXO_GET_UINT(fsos->checksum);

			PXO_GET_DATA(fsos->virgin_pilot);

			// skip 1-byte pad

			PXO_GET_DATA(fsos->num_medals);
			PXO_GET_DATA(fsos->num_ships);

			const int count = std::min(static_cast<unsigned short>(fsos->num_medals + fsos->num_ships), MAX_FS2OPEN_COUNTS);

			for (i = 0; i < count; i++) {
				PXO_GET_USHORT(fsos->counts[i]);
			}

			break;
		}

		case UNT_SW_RESULT_RESPONSE: {
			auto sw_resp = reinterpret_cast<squad_war_response *>(&uph->data);

			PXO_GET_DATA(sw_resp->reason);
			PXO_GET_DATA(sw_resp->accepted);

			break;
		}

		default:
			break;
	}

	//Assert(offset == data_size);
}


int InitPilotTrackerClient()
{
	FSWriteState = STATE_IDLE;
	FSReadState = STATE_IDLE;	
	SWWriteState = STATE_IDLE;
	
	ReadFSPilot = nullptr;

	fs_pr = reinterpret_cast<pilot_request *>(&fs_pilot_req.data);

	psnet_get_addr(Multi_options_g.user_tracker_ip, REGPORT, &ptrackaddr);

	return 1;
}

// Returns:
// -3	Error -- Called with NULL, but no request is waiting
// -2	Error -- Already sending data (hasn't timed out yet)
// -1	Timeout trying to send pilot data
// 0	Sending
// 1	Data succesfully sent
// 2	Send Cancelled (data may still have been written already, we just haven't been ACK'd yet)
// 3	Pilot not written (for some reason)
  
// Call with NULL to poll 
// Call with -1 to cancel send
// Call with valid pointer to a vmt_descent3_struct to initiate send
int SendFSPilotData(vmt_stats_struct *fs_pilot)
{
	//First check the network
	PollPTrackNet();

	if(fs_pilot == nullptr)
	{
		if(FSWriteState == STATE_IDLE)
		{
			return -3;
		}
		if(FSWriteState == STATE_SENDING_PILOT)
		{
			return 0;
		}
		if(FSWriteState == STATE_WROTE_PILOT)
		{
			//We wrote this pilot, and now we are about to inform the app, so back to idle
			FSWriteState = STATE_IDLE;
			return 1;
		}
		if(FSWriteState == STATE_TIMED_OUT)
		{
			//We gave up on sending this pilot, and now we are about to inform the app, so back to idle
			FSWriteState = STATE_IDLE;

			return -1;
		}
		if(FSWriteState == STATE_WRITE_PILOT_FAILED)
		{
			//The tracker said this dude couldn't be written
			FSWriteState = STATE_IDLE;

			return 3;
		}

	}
	else if(fs_pilot == reinterpret_cast<vmt_stats_struct *>(static_cast<uintptr_t>(0xffffffff)))
	{
		if(FSWriteState == STATE_IDLE)
		{
			return -3;
		}
		else
		{
			//Cancel this baby
			FSWriteState = STATE_IDLE;

			return 2;
		}

	}
	else if(FSWriteState == STATE_IDLE)
	{
		//New request, send out the req, and go for it.
		
		FSWriteState = STATE_SENDING_PILOT;
		
		FSLastSentWrite = 0;
		FSFirstSentWrite = timer_get_milliseconds();

		// deal with variable length of vmt_fs2open_struct
		const unsigned short size_offset = (MAX_FS2OPEN_COUNTS - (fs_pilot->num_ships + fs_pilot->num_medals)) * sizeof(unsigned short);

		fs_pilot_write.type = UNT_PILOT_DATA_WRITE_NEW;
		fs_pilot_write.code = CMD_GAME_FS2OPEN;
		fs_pilot_write.xcode = static_cast<unsigned short>(Multi_fs_tracker_game_id);
		fs_pilot_write.len = PACKED_HEADER_ONLY_SIZE+STATS_BLOCK_SIZE-size_offset;
		memcpy(&fs_pilot_write.data,fs_pilot,sizeof(vmt_stats_struct));

		return 0;	
	}
	return -2;
}

// Returns:
// -3	Error -- Called with NULL, but no request is waiting
// -2	Error -- Already sending data (hasn't timed out yet)
// -1	Timeout trying to send pilot data
// 0	Sending
// 1	Data succesfully sent
// 2	Send Cancelled (data may still have been written already, we just haven't been ACK'd yet)
// 3	Pilot not written (for some reason)
  
// Call with NULL to poll 
// Call with -1 to cancel send
// Call with valid pointer to a vmt_descent3_struct to initiate send
int SendSWData(squad_war_result *sw_res, squad_war_response *sw_resp)
{
	//First check the network
	PollPTrackNet();

	if(sw_res == nullptr){
		if(SWWriteState == STATE_IDLE){
			return -3;
		}
		if(SWWriteState == STATE_SENDING_PILOT){
			return 0;
		}

		// fill in the response
		if(SWWriteState == STATE_WROTE_PILOT){
			// We wrote this pilot, and now we are about to inform the app, so back to idle
			SWWriteState = STATE_IDLE;
			
			if(sw_resp != nullptr){
				memcpy(sw_resp, &SquadWarWriteResponse, sizeof(squad_war_response));
			}
			return 1;
		}
		// fill in the response
		if(SWWriteState == STATE_WRITE_PILOT_FAILED){
			// The tracker said this dude couldn't be written		
			SWWriteState = STATE_IDLE;

			if(sw_resp != nullptr){
				memcpy(sw_resp, &SquadWarWriteResponse, sizeof(squad_war_response));
			}
			return 3;
		}

		if(SWWriteState == STATE_TIMED_OUT){
			// We gave up on sending this pilot, and now we are about to inform the app, so back to idle
			SWWriteState = STATE_IDLE;

			return -1;
		}		
	} else if(sw_res == reinterpret_cast<squad_war_result *>(static_cast<uintptr_t>(0xffffffff))){
		if(SWWriteState == STATE_IDLE){
			return -3;
		} else {
			// Cancel this baby
			SWWriteState = STATE_IDLE;

			return 2;
		}
	} else if(SWWriteState == STATE_IDLE) {
		//New request, send out the req, and go for it.
		
		SWWriteState = STATE_SENDING_PILOT;
		
		SWLastSentWrite = 0;
		SWFirstSentWrite = timer_get_milliseconds();

		sw_res_write.len = PACKED_HEADER_ONLY_SIZE+sizeof(squad_war_result);
		sw_res_write.type = UNT_SW_RESULT_WRITE;
		sw_res_write.code = CMD_GAME_FS2OPEN;
		memcpy(&sw_res_write.data, sw_res, sizeof(squad_war_result));

		return 0;	
	}
	return -2;
}


// Returns:
// -3	Error -- Called with NULL, but no request is waiting
// -2	Error -- Already waiting on data (hasn't timed out yet)
// -1	Timeout waiting for pilot data
// 0	Waiting for data
// 1	Data received
// 2	Get Cancelled
// 3	Pilot not found
	
// Call with NULL to poll 
// Call with -1 to cancel wait
// Call with valid pointer to a vmt_descent3_struct to get a response
int GetFSPilotData(vmt_stats_struct *fs_pilot, const char *pilot_name, const char *tracker_id, int get_security)
{
	//First check the network
	PollPTrackNet();

	if(fs_pilot == nullptr)
	{
		if(FSReadState == STATE_IDLE)
		{
			return -3;
		}
		if(FSReadState == STATE_READING_PILOT)
		{
			return 0;
		}
		if(FSReadState == STATE_RECEIVED_PILOT)
		{
			// We got this pilot, and now we are about to inform the app, so back to idle
			FSReadState = STATE_IDLE;
			ReadFSPilot = nullptr;
			return 1;
		}
		if(FSReadState == STATE_TIMED_OUT)
		{
			// We gave up on this pilot, and now we are about to inform the app, so back to idle
			FSReadState = STATE_IDLE;
			ReadFSPilot = nullptr;
			return -1;
		}
		if(FSReadState == STATE_PILOT_NOT_FOUND)
		{
			//The tracker said this dude is not found.
			FSReadState = STATE_IDLE;
			ReadFSPilot = nullptr;
			return 3;
		}

	}
	else if(fs_pilot == reinterpret_cast<vmt_stats_struct *>(static_cast<uintptr_t>(0xffffffff)))
	{
		if(FSReadState == STATE_IDLE)
		{
			return -3;
		}
		else
		{
			//Cancel this baby
			FSReadState = STATE_IDLE;
			ReadFSPilot = nullptr;
			return 2;
		}

	}
	else if(FSReadState == STATE_IDLE)
	{
		//New request, send out the req, and go for it.
		
		FSReadState = STATE_READING_PILOT;
		ReadFSPilot = fs_pilot;
		FSLastSent = 0;
		FSFirstSent = timer_get_milliseconds();

		fs_pilot_req.xcode = static_cast<unsigned short>(Multi_fs_tracker_game_id);
		fs_pilot_req.len = PACKED_HEADER_ONLY_SIZE+sizeof(pilot_request);

		if(get_security){
			fs_pilot_req.type = UNT_PILOT_DATA_READ_NEW;
		} else {
			fs_pilot_req.type = UNT_PILOT_DATA_READ;			
		}

		fs_pilot_req.code = CMD_GAME_FS2OPEN;
		SDL_strlcpy(fs_pr->pilot_name, pilot_name, SDL_arraysize(fs_pr->pilot_name));
		SDL_strlcpy(fs_pr->tracker_id, tracker_id, SDL_arraysize(fs_pr->tracker_id));

		return 0;	
	}
	return -2;

}

// Send an ACK to the server
void AckServer(unsigned int sig)
{
	udp_packet_header ack_pack;
	ubyte packet_data[sizeof(udp_packet_header)];
	int packet_length = 0;

	ack_pack.type = UNT_CONTROL;
	ack_pack.sig = sig;
	ack_pack.code = CMD_CLIENT_RECEIVED;
	ack_pack.len = PACKED_HEADER_ONLY_SIZE;	

	packet_length = SerializePilotPacket(&ack_pack, packet_data);
	Assert(packet_length == PACKED_HEADER_ONLY_SIZE);
	SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
		   reinterpret_cast<LPSOCKADDR>(&ptrackaddr), sizeof(ptrackaddr), PSNET_TYPE_USER_TRACKER);
}

void IdlePTrack()
{
	ubyte packet_data[sizeof(udp_packet_header)];
	int packet_length = 0;

	PSNET_TOP_LAYER_PROCESS();

	// reading pilot data
	if(FSReadState == STATE_READING_PILOT){
		if((timer_get_milliseconds()-FSFirstSent)>=PILOT_REQ_TIMEOUT){
			FSReadState = STATE_TIMED_OUT;
		} else if((timer_get_milliseconds()-FSLastSent)>=PILOT_REQ_RESEND_TIME){
			//Send 'da packet
			packet_length = SerializePilotPacket(&fs_pilot_req, packet_data);
			SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
				   reinterpret_cast<LPSOCKADDR>(&ptrackaddr), sizeof(ptrackaddr), PSNET_TYPE_USER_TRACKER);

			FSLastSent = timer_get_milliseconds();
		}
	}

	// writing pilot data
	if(FSWriteState == STATE_SENDING_PILOT){
		if((timer_get_milliseconds()-FSFirstSentWrite)>=PILOT_REQ_TIMEOUT){
			FSWriteState = STATE_TIMED_OUT;

		} else if((timer_get_milliseconds()-FSLastSentWrite)>=PILOT_REQ_RESEND_TIME){
			// Send 'da packet
			packet_length = SerializePilotPacket(&fs_pilot_write, packet_data);
			SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
				   reinterpret_cast<LPSOCKADDR>(&ptrackaddr), sizeof(ptrackaddr), PSNET_TYPE_USER_TRACKER);

			FSLastSentWrite = timer_get_milliseconds();
		}
	}

	// writing squad war results
	if(SWWriteState == STATE_SENDING_PILOT){
		if((timer_get_milliseconds()-SWFirstSentWrite) >= PILOT_REQ_TIMEOUT){
			SWWriteState = STATE_TIMED_OUT;
		} else if((timer_get_milliseconds()-SWLastSentWrite) >= PILOT_REQ_RESEND_TIME){
			// Send 'da packet
			packet_length = SerializePilotPacket(&sw_res_write, packet_data);
			SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,
				   reinterpret_cast<LPSOCKADDR>(&ptrackaddr), sizeof(ptrackaddr), PSNET_TYPE_USER_TRACKER);

			SWLastSentWrite = timer_get_milliseconds();
		}
	}
}

void PollPTrackNet()
{
	fd_set read_fds;	           
	struct timeval timeout;
	ubyte packet_data[sizeof(udp_packet_header)];

	IdlePTrack();
	
	timeout.tv_sec=0;            
	timeout.tv_usec=0;
	
	FD_ZERO(&read_fds);	// NOLINT
	FD_SET(Psnet_socket, &read_fds);

	if(SELECT(static_cast<int>(Psnet_socket+1), &read_fds,nullptr,nullptr,&timeout, PSNET_TYPE_USER_TRACKER)){
		int bytesin;
		int addrsize;
		SOCKADDR_STORAGE fromaddr;

		udp_packet_header inpacket;

		SDL_zero(inpacket);
		addrsize = sizeof(fromaddr);

		bytesin = RECVFROM(Psnet_socket, reinterpret_cast<char *>(&packet_data), sizeof(udp_packet_header), 0,
						   reinterpret_cast<LPSOCKADDR>(&fromaddr), &addrsize, PSNET_TYPE_USER_TRACKER);

		if (bytesin > 0) {
			DeserializePilotPacket(packet_data, bytesin, &inpacket);

			// decrease packet size by 1
			inpacket.len--;
#ifndef NDEBUG
		} else {
			int wserr=WSAGetLastError();
			mprintf(("recvfrom() failure. WSAGetLastError() returned %d\n",wserr));
#endif
		}

		//Check to make sure the packets ok
		if ( (bytesin > 0) && (bytesin == inpacket.len) ) {
			switch(inpacket.type){
			case UNT_PILOT_DATA_RESPONSE:
				if(inpacket.code == CMD_GAME_FS2OPEN){
					if(FSReadState == STATE_READING_PILOT){
						vmt_stats_struct *stats;
						
						// 9/17/98 MWA.  Compare the tracker id of this packet with the tracker id of
						// what we are expecting.  Only set our state to something different when
						// the tracker id's match.  This fixes possible multiple packets for a single pilto
						// accidentally getting set for the wrong pilot

						stats = reinterpret_cast<vmt_stats_struct *>(&inpacket.data);
						if ( stats->tracker_id == SDL_atoi(fs_pr->tracker_id) ) {
							//Copy the data
							memcpy(ReadFSPilot,&inpacket.data,sizeof(vmt_stats_struct));
							//Set the state 
							FSReadState = STATE_RECEIVED_PILOT;
						}
					}
				} else {
					Int3();
				}
				break;

			case UNT_PILOT_READ_FAILED:
				if(inpacket.code == CMD_GAME_FS2OPEN){
					if(FSReadState == STATE_READING_PILOT){
						FSReadState = STATE_PILOT_NOT_FOUND;
					}
				} else {
					Int3();
				}
				break;

			case UNT_PILOT_WRITE_SUCCESS:
				if(inpacket.code == CMD_GAME_FS2OPEN){
					if(FSWriteState == STATE_SENDING_PILOT){
						FSWriteState = STATE_WROTE_PILOT;
					}
				} else {
					Int3();
				}
				break;

			case UNT_PILOT_WRITE_FAILED:
				if(inpacket.code == CMD_GAME_FS2OPEN){
					if(FSWriteState == STATE_SENDING_PILOT){
						FSWriteState = STATE_WRITE_PILOT_FAILED;
					}
				} else {
					Int3();
				}
				break;

			case UNT_SW_RESULT_RESPONSE:
				if(SWWriteState == STATE_SENDING_PILOT){					
					// copy the data
					Assert((bytesin - PACKED_HEADER_ONLY_SIZE) == sizeof(squad_war_response));
					if((bytesin - PACKED_HEADER_ONLY_SIZE) == sizeof(squad_war_response)){
						memset(&SquadWarWriteResponse, 0, sizeof(squad_war_response));
						memcpy(&SquadWarWriteResponse, inpacket.data, sizeof(squad_war_response));

						// now check to see if we're good
						if(SquadWarWriteResponse.accepted){
							SWWriteState = STATE_WROTE_PILOT;
						} else {
							SWWriteState = STATE_WRITE_PILOT_FAILED;
						}
					} else {
						SWWriteState = STATE_WRITE_PILOT_FAILED;
					}	
				}
				break;

			case UNT_CONTROL:
				Int3();
				break;

			case UNT_CONTROL_VALIDATION:
				Int3();
				break;

			default:
				break;
			}
			AckServer(inpacket.sig);
		}
	}
}
