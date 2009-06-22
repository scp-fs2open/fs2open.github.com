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

#define JOY_NUM_BUTTONS		32
#define JOY_NUM_HAT_POS		4
#define JOY_TOTAL_BUTTONS	(JOY_NUM_BUTTONS + JOY_NUM_HAT_POS)
#define JOY_NUM_AXES			6

#define JOY_HATBACK			(JOY_NUM_BUTTONS)
#define JOY_HATFORWARD		(JOY_NUM_BUTTONS+1)
#define JOY_HATLEFT			(JOY_NUM_BUTTONS+2)
#define JOY_HATRIGHT			(JOY_NUM_BUTTONS+3)

#define JOY_AXIS_UNDEFINED		-10000

typedef struct Joy_info {
	int	axis_valid[JOY_NUM_AXES];
	int	axis_min[JOY_NUM_AXES];
	int	axis_center[JOY_NUM_AXES];
	int	axis_max[JOY_NUM_AXES];
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

#endif	/* __JOY_H__ */
