/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __REDALERT_H__
#define __REDALERT_H__

#include "globalincs/pstypes.h"

struct CFILE;

void	red_alert_start_mission();

void	red_alert_init();
void	red_alert_close();
void	red_alert_do_frame(float frametime);
int	red_alert_mission();
void	red_alert_invalidate_timestamp();
int	red_alert_in_progress();
void red_alert_maybe_move_to_next_mission();

void red_alert_store_wingman_status();
void red_alert_bash_wingman_status();

void red_alert_voice_pause();
void red_alert_voice_unpause();

// should only ever be defined in redalert.cpp and the pilot file code!!
#ifdef REDALERT_INTERNAL
typedef struct red_alert_ship_status {
	SCP_string	name;
	float		hull;
	int			ship_class;
	SCP_vector<float>	subsys_current_hits;
	SCP_vector<float>	subsys_aggregate_current_hits;
	SCP_vector<wep_t>	primary_weapons;
	SCP_vector<wep_t>	secondary_weapons;
} red_alert_ship_status;

extern SCP_vector<red_alert_ship_status> Red_alert_wingman_status;
extern SCP_string Red_alert_precursor_mission;
#endif

#endif
