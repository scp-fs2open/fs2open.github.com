/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __JOY_H__
#define __JOY_H__

#include "globalincs/pstypes.h"
#include "SDL_joystick.h"

#include <climits>

#define JOY_NUM_BUTTONS		32
#define JOY_NUM_HAT_POS		4
#define JOY_TOTAL_BUTTONS	(JOY_NUM_BUTTONS + JOY_NUM_HAT_POS)
#define JOY_NUM_AXES			6

#define JOY_HATBACK			(JOY_NUM_BUTTONS)
#define JOY_HATFORWARD		(JOY_NUM_BUTTONS+1)
#define JOY_HATLEFT			(JOY_NUM_BUTTONS+2)
#define JOY_HATRIGHT			(JOY_NUM_BUTTONS+3)

#define JOY_AXIS_UNDEFINED		-10000

const int JOY_AXIS_MIN = 0;
const int JOY_AXIS_CENTER = -SHRT_MIN;
const int JOY_AXIS_MAX = USHRT_MAX + 1; // 1 since this is always checked to be less. ex: if (x < JOY_AXIS_MAX) {du_stoof();}

typedef struct Joy_info {
	int axis_valid[JOY_NUM_AXES];   // (bool) 0 if invalid axis, 1 if valid
	int axis_min[JOY_NUM_AXES];     // minimum value of each axis (default is JOY_AXIS_MIN)
	int axis_center[JOY_NUM_AXES];  // center value of each axis (default is JOY_AXIS_CENTER)
	int axis_max[JOY_NUM_AXES];     // maximum value of each axis (default is JOY_AXIS_MAX)
} Joy_info;

extern int Joy_sensitivity;
extern int Dead_zone_size;  // percentage of range that is dead zone

int	joy_init();
void	joy_flush();
int	joy_get_pos(int * x, int * y, int *z, int *r);
int	joy_down_count(int btn, int reset_count = 1);
int	joy_down(int btn);
int	joy_up_count(int btn);
float	joy_down_time(int btn);
void	joy_get_cal_vals(int *axis_min, int *axis_center, int *axis_max);
void	joy_set_cal_vals(int *axis_min, int *axis_center, int *axis_max);
void	joy_set_ul();
void	joy_set_lr();
void	joy_set_cen(); 
void	joy_cheap_cal();
int	joystick_read_raw_axis( int num_axes, int * axis );
void joy_get_delta(int *dx, int *dy);
int joy_get_scaled_reading(int raw, int axn);
int joy_get_unscaled_reading(int raw, int axn);
void joy_close();
void joy_device_changed(int state, int device);
SDL_Joystick* joy_get_device();

/**
 * @brief Called asychronously when the OS detects a change on any monitored joystick axis
 *
 * @param[in] id      The joystick instance index
 * @param[in] axis_id The id of the axis that changed
 * @param[in] value   The new value of the axis
 */
void joy_event(SDL_JoystickID id, uint8_t axis_id, int16_t value);



#endif	/* __JOY_H__ */
