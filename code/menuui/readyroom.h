/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef _READYROOM_H
#define _READYROOM_H

#include "globalincs/pstypes.h"
#include "scripting/hook_api.h"

struct sim_mission {
	SCP_string name;					// the mission name
	SCP_string filename;  // the mission filename
	SCP_string mission_desc; // the mission description
	SCP_string author;               // the mission designer
	int visible;							// if the mission is visible by default
};

extern SCP_vector<sim_mission> Sim_Missions;
extern SCP_vector<sim_mission> Sim_CMissions;

extern int Sim_room_overlay_id;
extern int Campaign_room_overlay_id;

void sim_room_init();
void sim_room_close();
void sim_room_do_frame(float frametime);

void api_sim_room_build_mission_list(bool API_Access);

// called by main menu to continue on with current campaign (if there is one).
int readyroom_continue_campaign();

void campaign_room_init();
void campaign_room_close();
void campaign_room_do_frame(float frametime);

bool campaign_build_campaign_list();
void campaign_select_campaign(const SCP_string& campaign_file);
void campaign_reset(const SCP_string& campaign_file);

extern const std::shared_ptr<scripting::Hook> OnCampaignBeginHook;

#endif
