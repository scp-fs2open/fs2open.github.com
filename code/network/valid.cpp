/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


//Validate tracker user class


#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cerrno>
#include <netdb.h>
#endif

#ifdef SCP_BSD
#include <sys/socket.h>
#endif

#include "network/multi.h"
#include "network/ptrack.h"
#include "network/valid.h"
#include "network/psnet2.h"
#include "network/multi_fstracker.h"
#include "io/timer.h"


// check structs for size compatibility
SDL_COMPILE_TIME_ASSERT(vmt_validate_mission_req_struct, sizeof(vmt_validate_mission_req_struct) == 104);


// Variables
static udp_packet_header PacketHeader;
static validate_id_request *ValidIDReq;

static int ValidState;

static SOCKADDR_STORAGE rtrackaddr;

static int ValidFirstSent;
static int ValidLastSent;

static char *Psztracker_id;

// mission validation
int MissionValidState;
static int MissionValidFirstSent;
static int MissionValidLastSent;

// squad war validation
int SquadWarValidState;
static int SquadWarFirstSent;
static int SquadWarLastSent;

// table validation
int TableValidState;
static int TableValidFirstSent;
static int TableValidLastSent;

// squad war response
static squad_war_response SquadWarValidateResponse;


static int SerializeValidatePacket(const udp_packet_header *uph, ubyte *data)
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

		case UNT_LOGIN_AUTH_REQUEST: {
			auto id_req = reinterpret_cast<const validate_id_request *>(&uph->data);

			PXO_ADD_DATA(id_req->login);
			PXO_ADD_DATA(id_req->password);
			PXO_ADD_DATA(id_req->tracker_id);	// junk here, just for size

			break;
		}

		case UNT_VALID_MSN_REQ:
		case UNT_VALID_TBL_REQ: {
			auto mis_req = reinterpret_cast<const vmt_validate_mission_req_struct *>(&uph->data);

			PXO_ADD_UINT(mis_req->checksum);

			memcpy(data+packet_size, mis_req->file_name, strlen(mis_req->file_name));
			packet_size += static_cast<unsigned int>(strlen(mis_req->file_name));

			data[packet_size] = '\0';
			packet_size++;

			break;
		}

		case UNT_VALID_SW_MSN_REQ: {
			auto sw_req = reinterpret_cast<const squad_war_request *>(&uph->data);

			for (i = 0; i < MAX_SQUAD_PLAYERS; i++) {
				PXO_ADD_INT(sw_req->squad_plr1[i]);
			}

			for (i = 0; i < MAX_SQUAD_PLAYERS; i++) {
				PXO_ADD_INT(sw_req->squad_plr2[i]);
			}

			PXO_ADD_DATA(sw_req->squad_count1);
			PXO_ADD_DATA(sw_req->squad_count2);

			PXO_ADD_DATA(sw_req->match_code);

			PXO_ADD_DATA(sw_req->mission_filename);
			PXO_ADD_INT(sw_req->mission_checksum);

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

static void DeserializeValidatePacket(const ubyte *data, const int data_size, udp_packet_header *uph)
{
	int offset = 0;

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
		case UNT_CONTROL:
		case UNT_CONTROL_VALIDATION:
		case UNT_LOGIN_NO_AUTH:
		case UNT_VALID_MSN_RSP:
			break;

		case UNT_VALID_TBL_RSP: {
			short game_id;

			PXO_GET_SHORT(game_id);

			memcpy(uph->data, &game_id, sizeof(short));
			SDL_strlcpy(reinterpret_cast<char *>(uph->data+sizeof(short)), reinterpret_cast<const char *>(data+offset), SDL_arraysize(uph->data) - sizeof(short));

			break;
		}

		case UNT_LOGIN_AUTHENTICATED: {
			SDL_strlcpy(reinterpret_cast<char *>(uph->data), reinterpret_cast<const char *>(data+offset), SDL_arraysize(uph->data));
			break;
		}

		case UNT_VALID_SW_MSN_RSP: {
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


int InitValidateClient()
{
	ValidFirstSent = 0;
	ValidLastSent = 0;
	ValidState = VALID_STATE_IDLE;
	
	MissionValidFirstSent = 0;
	MissionValidLastSent = 0;
	MissionValidState = VALID_STATE_IDLE;

	SquadWarFirstSent = 0;
	SquadWarLastSent = 0;
	SquadWarValidState = VALID_STATE_IDLE;

	TableValidFirstSent = 0;
	TableValidLastSent = 0;
	TableValidState = VALID_STATE_IDLE;

	psnet_get_addr(Multi_options_g.user_tracker_ip, REGPORT, &rtrackaddr);
	return 1;

}

//Call with a valid struct to validate a user
//Call with NULL to poll

//Return codes:
// -3	Still waiting (returned if we were waiting for a tracker response and ValidateUser was called with a non-NULL value
// -2 Timeout waiting for tracker to respond
// -1	User invalid
//  0	Still waiting for response from tracker/Idle
//  1	User valid
int ValidateUser(validate_id_request *valid_id, char *trackerid)
{
	ubyte packet_data[sizeof(udp_packet_header)];
	int packet_length = 0;

	ValidIdle();
	if(valid_id==nullptr)
	{
		switch(ValidState)
		{
		case VALID_STATE_IDLE:
			return 0;
		case VALID_STATE_WAITING:
			return 0;
		case VALID_STATE_VALID:
			ValidState = VALID_STATE_IDLE;
			return 1;
		case VALID_STATE_INVALID:
			ValidState = VALID_STATE_IDLE;
			return -1;
		case VALID_STATE_TIMEOUT:
			ValidState = VALID_STATE_IDLE;
			return -2;
		}
		return 0;
	}
	else
	{
		if(ValidState==VALID_STATE_IDLE)
		{
			//First, flush the input buffer for the socket
			fd_set read_fds;	           
			struct timeval timeout;
			
			timeout.tv_sec=0;            
			timeout.tv_usec=0;
			
			FD_ZERO(&read_fds);	// NOLINT
			FD_SET(Psnet_socket, &read_fds);

			while(SELECT(static_cast<int>(Psnet_socket+1),&read_fds,nullptr,nullptr,&timeout, PSNET_TYPE_VALIDATION))
			{
				int addrsize;
				SOCKADDR_STORAGE fromaddr;

				udp_packet_header inpacket;
				addrsize = sizeof(fromaddr);
				RECVFROM(Psnet_socket, reinterpret_cast<char *>(&inpacket), sizeof(udp_packet_header), 0,
						 reinterpret_cast<LPSOCKADDR>(&fromaddr), &addrsize, PSNET_TYPE_VALIDATION);
			}
			Psztracker_id = trackerid;

			//Build the request packet
			PacketHeader.type = UNT_LOGIN_AUTH_REQUEST;
			PacketHeader.xcode = static_cast<unsigned short>(Multi_fs_tracker_game_id);
			PacketHeader.len = PACKED_HEADER_ONLY_SIZE+sizeof(validate_id_request);
			ValidIDReq = reinterpret_cast<validate_id_request *>(&PacketHeader.data);
			SDL_strlcpy(ValidIDReq->login, valid_id->login, SDL_arraysize(ValidIDReq->login));
			SDL_strlcpy(ValidIDReq->password, valid_id->password, SDL_arraysize(ValidIDReq->password));

			packet_length = SerializeValidatePacket(&PacketHeader, packet_data);
			SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0, reinterpret_cast<LPSOCKADDR>(&rtrackaddr), sizeof(rtrackaddr), PSNET_TYPE_VALIDATION);
			ValidState = VALID_STATE_WAITING;
			ValidFirstSent = timer_get_milliseconds();
			ValidLastSent = timer_get_milliseconds();
			return 0;
		}
		else
		{
			return -3;
		}
	}
}


void ValidIdle()
{
	fd_set read_fds;	           
	struct timeval timeout;
	ubyte packet_data[sizeof(udp_packet_header)];
	int packet_length = 0;

	PSNET_TOP_LAYER_PROCESS();

	timeout.tv_sec=0;            
	timeout.tv_usec=0;
	
	FD_ZERO(&read_fds);	// NOLINT
	FD_SET(Psnet_socket, &read_fds);

	if(SELECT(static_cast<int>(Psnet_socket+1),&read_fds,nullptr,nullptr,&timeout, PSNET_TYPE_VALIDATION)){
		int bytesin;
		int addrsize;
		SOCKADDR_STORAGE fromaddr;

		udp_packet_header inpacket;

		SDL_zero(inpacket);
		addrsize = sizeof(fromaddr);

		bytesin = RECVFROM(Psnet_socket, reinterpret_cast<char *>(&packet_data), sizeof(udp_packet_header), 0,
						   reinterpret_cast<LPSOCKADDR>(&fromaddr), &addrsize, PSNET_TYPE_VALIDATION);

		if (bytesin > 0) {
			DeserializeValidatePacket(packet_data, bytesin, &inpacket);

			// decrease packet size by 1
			inpacket.len--;
#ifndef NDEBUG
		} else {
			int wserr=WSAGetLastError();
			mprintf(("recvfrom() failure. WSAGetLastError() returned %d\n",wserr));
#endif
		}

		FD_ZERO(&read_fds);	// NOLINT
		FD_SET(Psnet_socket, &read_fds);

		//Check to make sure the packets ok
		if ( (bytesin > 0) && (bytesin == inpacket.len) ) {
			switch(inpacket.type)
			{
				case UNT_LOGIN_NO_AUTH:
					if(ValidState == VALID_STATE_WAITING)
					{
						ValidState = VALID_STATE_INVALID;						
					}
					break;
				case UNT_LOGIN_AUTHENTICATED:
					if(ValidState == VALID_STATE_WAITING)
					{
						ValidState = VALID_STATE_VALID;
						SDL_strlcpy(Psztracker_id, reinterpret_cast<const char *>(&inpacket.data), TRACKER_ID_LEN);
					}
					break;

				// fs2 mission validation response
				case UNT_VALID_MSN_RSP:
					if(MissionValidState == VALID_STATE_WAITING){
						if(inpacket.code==2){
							MissionValidState = VALID_STATE_VALID;
						} else {
							MissionValidState = VALID_STATE_INVALID;
						}
					}
					break;

				// fs2 squad war validation response
				case UNT_VALID_SW_MSN_RSP:
					if(SquadWarValidState == VALID_STATE_WAITING){
						// copy the data
						Assert((static_cast<size_t>(bytesin) - PACKED_HEADER_ONLY_SIZE) == sizeof(squad_war_response));
						if((static_cast<size_t>(bytesin) - PACKED_HEADER_ONLY_SIZE) == sizeof(squad_war_response)){
							memset(&SquadWarValidateResponse, 0, sizeof(squad_war_response));
							memcpy(&SquadWarValidateResponse, inpacket.data, sizeof(squad_war_response));

							// now check to see if we're good
							if(SquadWarValidateResponse.accepted){
								SquadWarValidState = VALID_STATE_VALID;
							} else {
								SquadWarValidState = VALID_STATE_INVALID;
							}
						} else {
							SquadWarValidState = VALID_STATE_INVALID;
						}						
					}
					break;

				// table validation response
				case UNT_VALID_TBL_RSP:
					if (TableValidState == VALID_STATE_WAITING) {
						if (inpacket.code == 2) {
							TableValidState = VALID_STATE_VALID;

							if (static_cast<size_t>(bytesin) > PACKED_HEADER_ONLY_SIZE) {
								short game_id = -1;

								memcpy(&game_id, inpacket.data, sizeof(short));

								if (game_id > Multi_fs_tracker_game_id) {
									Multi_fs_tracker_game_id = game_id;
									Multi_fs_tracker_game_name = reinterpret_cast<const char *>(inpacket.data+sizeof(short));
								}
							}
						} else {
							TableValidState = VALID_STATE_INVALID;
						}
					}
					break;

				case UNT_CONTROL_VALIDATION:
					Int3();
					break;

				case UNT_CONTROL:
					Int3();
					break;
			}
			AckValidServer(inpacket.sig);
		}
	}

	if(ValidState == VALID_STATE_WAITING)
	{
		if((timer_get_milliseconds()-ValidFirstSent)>=PILOT_REQ_TIMEOUT)
		{
			ValidState = VALID_STATE_TIMEOUT;

		}		
		else if((timer_get_milliseconds()-ValidLastSent)>=PILOT_REQ_RESEND_TIME)
		{
			//Send 'da packet
			packet_length = SerializeValidatePacket(&PacketHeader, packet_data);
			SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0, reinterpret_cast<LPSOCKADDR>(&rtrackaddr), sizeof(rtrackaddr), PSNET_TYPE_VALIDATION);
			ValidLastSent = timer_get_milliseconds();
		}
	}

	if(MissionValidState == VALID_STATE_WAITING)
	{
		if((timer_get_milliseconds()-MissionValidFirstSent)>=PILOT_REQ_TIMEOUT)
		{
			MissionValidState = VALID_STATE_TIMEOUT;
		}
	}

	if(SquadWarValidState == VALID_STATE_WAITING)
	{
		if((timer_get_milliseconds()-SquadWarFirstSent)>=PILOT_REQ_TIMEOUT)
		{
			SquadWarValidState = VALID_STATE_TIMEOUT;
		}
	}

	if (TableValidState == VALID_STATE_WAITING) {
		if ( (timer_get_milliseconds() - TableValidFirstSent) >= PILOT_REQ_TIMEOUT ) {
			TableValidState = VALID_STATE_TIMEOUT;
		}
	}
}


//Send an ACK to the server
void AckValidServer(unsigned int sig)
{
	udp_packet_header ack_pack;
	ubyte packet_data[sizeof(udp_packet_header)];
	int packet_length = 0;

	ack_pack.type = UNT_CONTROL;
	ack_pack.sig = sig;
	ack_pack.code = CMD_CLIENT_RECEIVED;
	ack_pack.len = PACKED_HEADER_ONLY_SIZE;

	packet_length = SerializeValidatePacket(&ack_pack, packet_data);
	Assert(packet_length == PACKED_HEADER_ONLY_SIZE);
	SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0, reinterpret_cast<LPSOCKADDR>(&rtrackaddr), sizeof(rtrackaddr), PSNET_TYPE_VALIDATION);
}

// call with a valid struct to validate a mission
// call with NULL to poll

// Return codes:
// -3	Still waiting (returned if we were waiting for a tracker response and ValidateMission was called with a non-NULL value
// -2 Timeout waiting for tracker to respond
// -1	User invalid
//  0	Still waiting for response from tracker/Idle
//  1	User valid
int ValidateMission(vmt_validate_mission_req_struct *valid_msn)
{
	ubyte packet_data[sizeof(udp_packet_header)];
	int packet_length = 0;

	ValidIdle();
	if(valid_msn==nullptr)
	{
		switch(MissionValidState)
		{
		case VALID_STATE_IDLE:
			return 0;

		case VALID_STATE_WAITING:
			return 0;

		case VALID_STATE_VALID:
			MissionValidState = VALID_STATE_IDLE;
			return 1;

		case VALID_STATE_INVALID:
			MissionValidState = VALID_STATE_IDLE;
			return -1;

		case VALID_STATE_TIMEOUT:
			MissionValidState = VALID_STATE_IDLE;
			return -2;
		}
		return 0;
	}
	else
	{
		if(MissionValidState==VALID_STATE_IDLE)
		{
			//First, flush the input buffer for the socket
			fd_set read_fds;	           
			struct timeval timeout;
			
			timeout.tv_sec=0;            
			timeout.tv_usec=0;

			while(SELECT(static_cast<int>(Psnet_socket+1),&read_fds,nullptr,nullptr,&timeout, PSNET_TYPE_VALIDATION))
			{
				int addrsize;
				SOCKADDR_STORAGE fromaddr;
				udp_packet_header inpacket;

				FD_ZERO(&read_fds);	// NOLINT
				FD_SET(Psnet_socket, &read_fds);

				addrsize = sizeof(fromaddr);
				RECVFROM(Psnet_socket, reinterpret_cast<char *>(&inpacket), sizeof(udp_packet_header), 0,
						 reinterpret_cast<LPSOCKADDR>(&fromaddr), &addrsize, PSNET_TYPE_VALIDATION);
			}
			//only send the header, the checksum and the string length plus the null
			PacketHeader.type = UNT_VALID_MSN_REQ;
			PacketHeader.xcode = static_cast<unsigned short>(Multi_fs_tracker_game_id);
			PacketHeader.len = static_cast<unsigned short>(PACKED_HEADER_ONLY_SIZE + sizeof(int)+1+strlen(valid_msn->file_name));
			memcpy(PacketHeader.data,valid_msn,PacketHeader.len-PACKED_HEADER_ONLY_SIZE);
			packet_length = SerializeValidatePacket(&PacketHeader, packet_data);
			SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0, reinterpret_cast<LPSOCKADDR>(&rtrackaddr), sizeof(rtrackaddr), PSNET_TYPE_VALIDATION);
			MissionValidState = VALID_STATE_WAITING;
			MissionValidFirstSent = timer_get_milliseconds();
			MissionValidLastSent = timer_get_milliseconds();
			return 0;
		}
		else
		{
			return -3;
		}
	}
}

// query the usertracker to validate a squad war match
// call with a valid struct to validate a mission
// call with NULL to poll

// Return codes:
// -3	Still waiting (returned if we were waiting for a tracker response and ValidateSquadWae was called with a non-NULL value
// -2 Timeout waiting for tracker to respond
// -1	match invalid
//  0	Still waiting for response from tracker/Idle
//  1	match valid
int ValidateSquadWar(squad_war_request *sw_req, squad_war_response *sw_resp)
{
	ubyte packet_data[sizeof(udp_packet_header)];
	int packet_length = 0;

	ValidIdle();
	if(sw_req==nullptr){
		switch(SquadWarValidState){
		case VALID_STATE_IDLE:
			return 0;

		case VALID_STATE_WAITING:
			return 0;

		// fill in the response
		case VALID_STATE_VALID:
			SquadWarValidState = VALID_STATE_IDLE;
			if(sw_resp != nullptr){
				memcpy(sw_resp, &SquadWarValidateResponse, sizeof(squad_war_response));
			}
			return 1;

		// fill in the response
		case VALID_STATE_INVALID:
			SquadWarValidState = VALID_STATE_IDLE;
			if(sw_resp != nullptr){
				memcpy(sw_resp, &SquadWarValidateResponse, sizeof(squad_war_response));
			}
			return -1;

		case VALID_STATE_TIMEOUT:
			SquadWarValidState = VALID_STATE_IDLE;
			return -2;
		}
		return 0;
	} else {
		if(SquadWarValidState==VALID_STATE_IDLE){
			// First, flush the input buffer for the socket
			fd_set read_fds;	           
			struct timeval timeout;
			
			timeout.tv_sec=0;            
			timeout.tv_usec=0;

			while(SELECT(static_cast<int>(Psnet_socket+1),&read_fds,nullptr,nullptr,&timeout, PSNET_TYPE_VALIDATION)){
				int addrsize;
				SOCKADDR_STORAGE fromaddr;
				udp_packet_header inpacket;

				FD_ZERO(&read_fds);	// NOLINT
				FD_SET(Psnet_socket, &read_fds);

				addrsize = sizeof(fromaddr);
				RECVFROM(Psnet_socket, reinterpret_cast<char *>(&inpacket), sizeof(udp_packet_header), 0,
						 reinterpret_cast<LPSOCKADDR>(&fromaddr), &addrsize, PSNET_TYPE_VALIDATION);

			}
			// only send the header, the checksum and the string length plus the null
			PacketHeader.type = UNT_VALID_SW_MSN_REQ;
			PacketHeader.xcode = static_cast<unsigned short>(Multi_fs_tracker_game_id);
			PacketHeader.len = static_cast<unsigned short>(PACKED_HEADER_ONLY_SIZE + sizeof(squad_war_request));
			memcpy(PacketHeader.data, sw_req, PacketHeader.len-PACKED_HEADER_ONLY_SIZE);
			packet_length = SerializeValidatePacket(&PacketHeader, packet_data);
			SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0, reinterpret_cast<LPSOCKADDR>(&rtrackaddr), sizeof(rtrackaddr), PSNET_TYPE_VALIDATION);
			SquadWarValidState = VALID_STATE_WAITING;
			SquadWarFirstSent = timer_get_milliseconds();
			SquadWarLastSent = timer_get_milliseconds();
			return 0;
		} else {
			return -3;
		}
	}
}

// call with a valid struct to validate a table
// call with NULL to poll

// Return codes:
// -3	Still waiting (returned if we were waiting for a tracker response and ValidateTable was called with a non-NULL value
// -2   Timeout waiting for tracker to respond
// -1	table invalid
//  0	Still waiting for response from tracker/Idle
//  1	table valid
int ValidateTable(vmt_validate_mission_req_struct *valid_tbl)
{
	ubyte packet_data[sizeof(udp_packet_header)];
	int packet_length = 0;

	ValidIdle();

	if (valid_tbl == nullptr) {
		switch (TableValidState) {
			case VALID_STATE_IDLE:
				return 0;
			case VALID_STATE_WAITING:
				return 0;
			case VALID_STATE_VALID:
				TableValidState = VALID_STATE_IDLE;
				return 1;
			case VALID_STATE_INVALID:
				TableValidState = VALID_STATE_IDLE;
				return -1;
			case VALID_STATE_TIMEOUT:
				TableValidState = VALID_STATE_IDLE;
				return -2;
		}

		return 0;
	} else {
		if (TableValidState == VALID_STATE_IDLE) {
			fd_set read_fds;
			struct timeval timeout;

			timeout.tv_sec = 0;
			timeout.tv_usec = 0;

			while ( SELECT(static_cast<int>(Psnet_socket+1), &read_fds, nullptr, nullptr, &timeout, PSNET_TYPE_VALIDATION) ) {
				int addrsize;
				SOCKADDR_STORAGE fromaddr;
				udp_packet_header inpacket;

				FD_ZERO(&read_fds);	// NOLINT
				FD_SET(Psnet_socket, &read_fds);

				addrsize = sizeof(fromaddr);
				RECVFROM(Psnet_socket, reinterpret_cast<char *>(&inpacket), sizeof(udp_packet_header), 0,
						 reinterpret_cast<LPSOCKADDR>(&fromaddr), &addrsize, PSNET_TYPE_VALIDATION);
			}

			//only send the header, the checksum and the string length plus the null
			PacketHeader.type = UNT_VALID_TBL_REQ;
			PacketHeader.xcode = static_cast<unsigned short>(Multi_fs_tracker_game_id);
			PacketHeader.len = static_cast<unsigned short>(PACKED_HEADER_ONLY_SIZE + sizeof(int)+1+strlen(valid_tbl->file_name));
			memcpy(PacketHeader.data, valid_tbl, PacketHeader.len-PACKED_HEADER_ONLY_SIZE);
			packet_length = SerializeValidatePacket(&PacketHeader, packet_data);
			SENDTO(Psnet_socket, reinterpret_cast<char *>(&packet_data), packet_length, 0,reinterpret_cast<LPSOCKADDR>(&rtrackaddr), sizeof(rtrackaddr), PSNET_TYPE_VALIDATION);
			TableValidState = VALID_STATE_WAITING;
			TableValidFirstSent = timer_get_milliseconds();
			TableValidLastSent = timer_get_milliseconds();

			return 0;
		} else {
			return -1;
		}
	}
}
