/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 
#include "mission/missionbriefcommon.h"


#ifndef __MISSIONDEBRIEF_H__
#define __MISSIONDEBRIEF_H__


extern int Debrief_overlay_id;

extern int Debrief_multi_stages_loaded;

extern int Must_replay_mission;
extern int Turned_traitor;
extern int Promoted;
extern debriefing Traitor_debriefing;
extern debrief_stage Promotion_stage, Badge_stage;

extern bool API_Access;

void debrief_init();
void debrief_do_frame(float frametime);
void debrief_close();

// useful so that the server can reset the list and ship slots if a player drops
void debrief_rebuild_player_list();
void debrief_handle_player_drop();

void debrief_maybe_auto_accept();
void debrief_disable_accept();
void debrief_assemble_optional_mission_popup_text(char *buffer, char *mission_loop_desc);

int debrief_select_music();
void debrief_choose_medal_variant(char* buf, int medal_earned, int zero_based_version_index);
void debrief_accept(int ok_to_post_start_game_event = 1);

// multiplayer call to set up the client side debriefings
void debrief_multi_server_stuff();
void debrief_set_multi_clients( int stage_count, int active_stages[] );

void debrief_pause();
void debrief_unpause();

#endif /* __MISSIONDEBRIEF_H__ */
