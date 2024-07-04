/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef _valid_client_header
#define _valid_client_header

#include "network/ptrack.h"

//Validate User Header

//Function prototypes

extern int MissionValidState;
extern int SquadWarValidState;
extern int DataValidState;

//Call with a valid struct to validate a user
//Call with NULL to poll

//Return codes:
// -3	Still waiting (returned if we were waiting for a tracker response and ValidateUser was called with a non-NULL value
// -2 Timeout waiting for tracker to respond
// -1	User invalid
//  0	Still waiting for response from tracker/Idle
//  1	User valid
int ValidateUser(validate_id_request *valid_id, char *trackerid);
void AckValidServer(unsigned int sig);

int InitValidateClient();
void ValidIdle();


//Definitions


// #define PILOT_REQ_TIMEOUT			10000
// #define PILOT_REQ_RESEND_TIME		750

#define VALID_STATE_IDLE		1
#define VALID_STATE_WAITING	2
#define VALID_STATE_VALID		3
#define VALID_STATE_INVALID	4
#define VALID_STATE_TIMEOUT	5

typedef struct vmt_validate_mission_req_struct {
	unsigned int checksum;
	char file_name[100];
} vmt_validate_mission_req_struct;

// query the usertracker to validate a mission
int ValidateMission(vmt_validate_mission_req_struct *valid_msn);

// query the usertracker to validate a squad war match
int ValidateSquadWar(squad_war_request *sw_req, squad_war_response *sw_resp);

// query the usertracker to validate game data (tables, missions, scripts)
int ValidateData(const vmt_valid_data_req_struct *vreq);

// for batched validation, returns true if file at idx was valid or not
// (only when VDR_FLAG_STATUS is set)
bool IsDataIndexValid(const unsigned int idx);

#endif
