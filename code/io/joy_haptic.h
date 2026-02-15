/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/



#ifndef __JOY_HAPTIC_H__
#define __JOY_HAPTIC_H__

#include "globalincs/pstypes.h"

bool joy_haptic_init();
void joy_haptic_shutdown();
void joy_haptic_set_gain(int gain);
void joy_haptic_stop_effects();
void joy_haptic_mission_init(vec3d *v);
void joy_haptic_play_vector_effect(vec3d *v, float scaler);
void joy_haptic_play_dir_effect(float x, float y);
void joy_haptic_play_primary_shoot(int gain);
void joy_haptic_play_secondary_shoot(int gain);
void joy_haptic_adjust_handling(int speed);
void joy_haptic_docked();
void joy_haptic_play_reload_effect();
void joy_haptic_afterburn_on();
void joy_haptic_afterburn_off();
void joy_haptic_explode();
void joy_haptic_fly_by(int mag);
void joy_haptic_deathroll();

#endif

