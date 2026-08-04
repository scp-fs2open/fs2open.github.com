/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/



#ifndef __JOY_RUMBLE_H__
#define __JOY_RUMBLE_H__

#include "globalincs/pstypes.h"

bool joy_rumble_init();
void joy_rumble_shutdown();
void joy_rumble_set_gain(int gain);
void joy_rumble_stop_effects();
void joy_rumble_mission_init(vec3d *v);
void joy_rumble_play_vector_effect(vec3d *v, float scaler);
void joy_rumble_play_dir_effect(float x, float y);
void joy_rumble_play_primary_shoot(int gain);
void joy_rumble_play_secondary_shoot(int gain);
void joy_rumble_adjust_handling(int speed);
void joy_rumble_docked();
void joy_rumble_play_reload_effect();
void joy_rumble_afterburn_on();
void joy_rumble_afterburn_off();
void joy_rumble_explode();
void joy_rumble_fly_by(int mag);
void joy_rumble_deathroll();

#endif


