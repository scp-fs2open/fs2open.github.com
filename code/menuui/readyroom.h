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

extern int Sim_room_overlay_id;
extern int Campaign_room_overlay_id;

void sim_room_init();
void sim_room_close();
void sim_room_do_frame(float frametime);

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
