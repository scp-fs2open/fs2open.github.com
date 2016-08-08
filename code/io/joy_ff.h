#ifndef __JOY_FF_H__
#define __JOY_FF_H__
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "globalincs/pstypes.h"

/**
 * @brief Inits haptic feedback
 *
 * @returns  0 if successful, or
 * @returns  0 if Haptic Feedback is disabled (and can't init), or
 * @returns -1 if an error occurred
 */
int joy_ff_init();

/**
 * @brief De-init's haptic feedback
 */
void joy_ff_shutdown();

/**
 * @brief Stops any currently playing haptic effects
 */
void joy_ff_stop_effects();

/**
 * @brief Pre-mission init
 */
void joy_ff_mission_init(vec3d v);

/**
 * @brief Unimplemented (TODO)
 */
void joy_reacquire_ff();

/**
 * @brief Unimplmented (TODO)
 */
void joy_unacquire_ff();

/**
 * @brief Play a vector effect with the given scaler
 *
 * @param[in] v Vector in
 * @param[in] scaler Scale up or down the effect
 */
void joy_ff_play_vector_effect(vec3d *v, float scaler);

/**
 * @brief Play an effect with the given x, y as its origin (left/right, up/down)
 */
void joy_ff_play_dir_effect(float x, float y);

/**
 * @brief Play an effect of a primary weapon shooting
 *
 * @param[in] gain Strength of the buzz
 */
void joy_ff_play_primary_shoot(int gain);

/**
 * @brief Play an effect of a secondary weapon shooting
 *
 * @param[in] gain Strength of the buzz
 */
void joy_ff_play_secondary_shoot(int gain);

/**
 * @brief Adjust the handling force with the given speed
 *
 * @param[in] speed The current speed (non-negative scalar of velocity) of the craft.
 *   Higher speeds generally mean tighter handling
 */
void joy_ff_adjust_handling(int speed);

/**
 * @brief Play a "docked" haptic effect
 */
void joy_ff_docked();

/**
 * @brief Play a "reload" (from the support ship) haptic effect
 */
void joy_ff_play_reload_effect();

/**
 * @brief Start playing an "afterburner" haptic effect
 */
void joy_ff_afterburn_on();

/**
 * @brief Stop playing afterburner effects
 */
void joy_ff_afterburn_off();

/**
 * @brief Play the "explode" haptic effect
 */
void joy_ff_explode();

/**
 * @brief Play the "Fly By" haptic effect
 *
 * @param[in] mag Strength of the effect
 */
void joy_ff_fly_by(int mag);

/**
 * @brief Play the "deathroll" haptic effect
 */
void joy_ff_deathroll();

#endif
