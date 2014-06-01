/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



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
