/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on
 * the source.
 */


#ifndef WIN32	// Goober5000

#include "globalincs/pstypes.h"
#include "io/joy.h"
#include "math/fix.h"
#include "io/key.h"
#include "io/timer.h"
#include "osapi/osregistry.h"
#include "io/joy_ff.h"
#include "osapi/osapi.h"
#include "SDL.h"

static int Joy_inited = 0;
static int joy_num_sticks = 0;
static int joy_num_buttons = 0;
static int joy_num_axes = 0;
static int joy_num_hats = 0;
int Dead_zone_size = 10;
int Cur_joystick = -1;
int Joy_sensitivity = 9;

int joy_pollrate = 1000 / 18;  // poll at 18Hz

static int Joy_last_x_reading = 0;
static int Joy_last_y_reading = 0;

typedef struct joy_button_info {
	int     actual_state;           // Set if the button is physically down
	int     state;                          // Set when the button goes from up to down, cleared on down to up.  Different than actual_state after a flush.
	int     down_count;
	int     up_count;
	int     down_time;
	uint    last_down_check;        // timestamp in milliseconds of last 
} joy_button_info;

Joy_info joystick;

int JOYSTICKID1 = 0; 	// DDOI - temporary
static SDL_Joystick *sdljoy;

joy_button_info joy_buttons[JOY_TOTAL_BUTTONS];


void joy_close()
{
	if (!Joy_inited)
		return;

	Joy_inited = 0;
	joy_num_sticks = 0;
	
	if (sdljoy)
		SDL_JoystickClose(sdljoy);

	sdljoy = NULL;
	
	SDL_QuitSubSystem (SDL_INIT_JOYSTICK);
}

void joy_get_caps (int max)
{
	SDL_Joystick *joy;
	int j;

	for (j=0; j < JOY_NUM_AXES; j++)
		joystick.axis_valid[j] = 0;

	for (j=JOYSTICKID1; j<JOYSTICKID1+max; j++) {
		joy = SDL_JoystickOpen (j);
		if (joy)
		{
			nprintf (("JOYSTICK", "Joystick #%d: %s\n", j - JOYSTICKID1 + 1, SDL_JoystickName(j)));
			if (j == Cur_joystick) {
				for (int i = 0; i < SDL_JoystickNumAxes(joy); i++)
				{
					joystick.axis_valid[i] = 1;
				}
			}
			SDL_JoystickClose (joy);
		}
	}
}

int joy_down(int btn)
{
	int tmp;

	if ( joy_num_sticks < 1 ) return 0;
	if ( (btn < 0) || (btn >= JOY_TOTAL_BUTTONS )) return 0;

	tmp = joy_buttons[btn].state;

	return tmp;
}

int joy_down_count(int btn, int reset_count)
{
	int tmp;

	if ( joy_num_sticks < 1 ) return 0;
	if ( (btn < 0) || (btn >= JOY_TOTAL_BUTTONS)) return 0;

	tmp = joy_buttons[btn].down_count;

	if ( reset_count )
		joy_buttons[btn].down_count = 0;

	return tmp;
}

float joy_down_time(int btn)
{
	float                           rval;
	unsigned int    now;
	joy_button_info         *bi;

	if ( joy_num_sticks < 1 ) return 0.0f;
	if ( (btn < 0) || (btn >= JOY_TOTAL_BUTTONS)) return 0.0f;
	bi = &joy_buttons[btn];

	now = timer_get_milliseconds();

	if ( bi->down_time == 0 && joy_down(btn) ) {
		bi->down_time += joy_pollrate;
	}

	if ( (now - bi->last_down_check) > 0)
		rval = i2fl(bi->down_time) / (now - bi->last_down_check);
	else
		rval = 0.0f;

	bi->down_time = 0;
	bi->last_down_check = now;

	if (rval < 0)
		rval = 0.0f;
	if (rval > 1)
		rval = 1.0f;

	return rval;
}

void joy_flush()
{
	int                     i;
	joy_button_info *bi;

	if ( joy_num_sticks < 1 )
		return;

	for ( i = 0; i < JOY_TOTAL_BUTTONS; i++) {
		bi = &joy_buttons[i];
		bi->actual_state = 0;
		bi->state = 0;
		bi->down_count = 0;
		bi->up_count = 0;
		bi->down_time = 0;
		bi->last_down_check = timer_get_milliseconds();
	}
}

int joy_get_unscaled_reading(int raw, int axn)
{
	int rng;

	// Make sure it's calibrated properly.
	if (joystick.axis_center[axn] - joystick.axis_min[axn] < 5)
		return 0;

	if (joystick.axis_max[axn] - joystick.axis_center[axn] < 5)
		return 0;

	rng = joystick.axis_max[axn] - joystick.axis_min[axn];
	raw -= joystick.axis_min[axn];  // adjust for linear range starting at 0
	
	// cap at limits
	if (raw < 0)
		raw = 0;
	if (raw > rng)
		raw = rng;

	return (int) ((uint) raw * (uint) F1_0 / (uint) rng);  // convert to 0 - F1_0 range.
}

// --------------------------------------------------------------
//	joy_get_scaled_reading()
//
//	input:	raw	=>	the raw value for an axis position
//				axn	=>	axis number, numbered starting at 0
//
// return:	joy_get_scaled_reading will return a value that represents
//				the joystick pos from -1 to +1 for the specified axis number 'axn', and
//				the raw value 'raw'
//
int joy_get_scaled_reading(int raw, int axn)
{
	int x, d, dead_zone, rng;
	float percent, sensitivity_percent, non_sensitivity_percent;

	// Make sure it's calibrated properly.
	if (joystick.axis_center[axn] - joystick.axis_min[axn] < 5)
		return 0;

	if (joystick.axis_max[axn] - joystick.axis_center[axn] < 5)
		return 0;

	raw -= joystick.axis_center[axn];

	dead_zone = (joystick.axis_max[axn] - joystick.axis_min[axn]) * Dead_zone_size / 100;

	if (raw < -dead_zone) {
		rng = joystick.axis_center[axn] - joystick.axis_min[axn] - dead_zone;
		d = -raw - dead_zone;

	} else if (raw > dead_zone) {
		rng = joystick.axis_max[axn] - joystick.axis_center[axn] - dead_zone;
		d = raw - dead_zone;

	} else
		return 0;

	if (d > rng)
		d = rng;

	Assert(Joy_sensitivity >= 0 && Joy_sensitivity <= 9);

	// compute percentages as a range between 0 and 1
	sensitivity_percent = (float) Joy_sensitivity / 9.0f;
	non_sensitivity_percent = (float) (9 - Joy_sensitivity) / 9.0f;

	// find percent of max axis is at
	percent = (float) d / (float) rng;

	// work sensitivity on axis value
	percent = (percent * sensitivity_percent + percent * percent * percent * percent * percent * non_sensitivity_percent);

	x = (int) ((float) F1_0 * percent);

	//nprintf(("AI", "d=%6i, sens=%3i, percent=%6.3f, val=%6i, ratio=%6.3f\n", d, Joy_sensitivity, percent, (raw<0) ? -x : x, (float) d/x));

	if (raw < 0)
		return -x;

	return x;
}

// --------------------------------------------------------------
//	joy_get_pos()
//
//	input:	x		=>		OUTPUT PARAMETER: x-axis position of stick (-1 to 1)
//				y		=>		OUTPUT PARAMETER: y-axis position of stick (-1 to 1)
//				z		=>		OUTPUT PARAMETER: z-axis (throttle) position of stick (-1 to 1)
//				r		=>		OUTPUT PARAMETER: rudder position of stick (-1 to 1)
//
//	return:	success	=> 1
//				failure	=> 0
//
int joy_get_pos(int *x, int *y, int *z, int *rx)
{
	int axis[JOY_NUM_AXES];

	if (x) *x = 0;
	if (y) *y = 0;
	if (z) *z = 0;
	if (rx) *rx = 0;

	if (joy_num_sticks < 1) return 0;

	joystick_read_raw_axis( 6, axis );

	//	joy_get_scaled_reading will return a value represents the joystick pos from -1 to +1
	if (x && joystick.axis_valid[0])
		*x = joy_get_scaled_reading(axis[0], 0);
	if (y && joystick.axis_valid[1]) 
		*y = joy_get_scaled_reading(axis[1], 1);
	if (z && joystick.axis_valid[2])
		*z = joy_get_unscaled_reading(axis[2], 2);
	if (rx && joystick.axis_valid[3])
		*rx = joy_get_scaled_reading(axis[3], 3);

	if (x)
		Joy_last_x_reading = *x;

	if (y)
		Joy_last_y_reading = *y;

	return 1;
}

void joy_process(int time_delta)
{
	STUB_FUNCTION;

/*	int i;
	
	if (!Joy_inited)
		return;

	int hat = SDL_JoystickGetHat(sdljoy, 0);
	
	for (i=0; i < JOY_TOTAL_BUTTONS; i++) {
		int state = 0;
		
		if (i < JOY_NUM_BUTTONS) {
			if (i < joy_num_buttons) {
				state = SDL_JoystickGetButton(sdljoy, i);
			}
		} else { 
			switch (i) {
				case JOY_HATBACK:
					state = (hat & SDL_HAT_DOWN) ? 1 : 0;
					break;
				case JOY_HATFORWARD:
					state = (hat & SDL_HAT_UP) ? 1 : 0;
					break;
				case JOY_HATLEFT:
					state = (hat & SDL_HAT_LEFT) ? 1 : 0;
					break;
				case JOY_HATRIGHT:
					state = (hat & SDL_HAT_RIGHT) ? 1 : 0;
					break;
				default:
					break;
			}
		}
		
		if (state != joy_buttons[i].actual_state) {
			// Button position physically changed.
			joy_buttons[i].actual_state = state;
			
			if ( state )    {
				// went from up to down
				joy_buttons[i].down_count++;
				joy_buttons[i].down_time = 0;
				
				joy_buttons[i].state = 1;
			} else {
				// went from down to up
				if ( joy_buttons[i].state )     {
					joy_buttons[i].up_count++;
				}
				
				joy_buttons[i].state = 0;
			}
		} else {
			// Didn't move... increment time down if down.
			if (joy_buttons[i].state) {
				//joy_buttons[i].down_time += joy_pollrate;
				joy_buttons[i].down_time += time_delta;
			}
		}	
	} */
}


void joy_set_button_state(int button, int state)
{
	if (!Joy_inited)
		return;

	if (state != joy_buttons[button].actual_state) {
		// Button position physically changed.
		joy_buttons[button].actual_state = state;

		if ( state )    {
			// went from up to down
			joy_buttons[button].down_count++;
			joy_buttons[button].down_time = 0;

			joy_buttons[button].state = 1;
		} else {
			// went from down to up
			if ( joy_buttons[button].state )
				joy_buttons[button].up_count++;

			joy_buttons[button].state = 0;
		}
	} else {
		// Didn't move... increment time down if down.
		if (joy_buttons[button].state)
			joy_buttons[button].down_time += joy_pollrate;
	}
}

void joy_set_hat_state(int position)
{
	int state = 0;

	if (!Joy_inited)
		return;

	for (int i=JOY_NUM_BUTTONS-1; i<JOY_TOTAL_BUTTONS; i++) {
		switch (i) {
			case JOY_HATBACK:
				state = (position == SDL_HAT_DOWN) ? 1 : 0;
				break;
			case JOY_HATFORWARD:
				state = (position == SDL_HAT_UP) ? 1 : 0;
				break;
			case JOY_HATLEFT:
				state = (position == SDL_HAT_LEFT) ? 1 : 0;
				break;
			case JOY_HATRIGHT:
				state = (position == SDL_HAT_RIGHT) ? 1 : 0;
				break;
			default:
				break;
		}

		if (state != joy_buttons[i].actual_state) {
			// Button position physically changed.
			joy_buttons[i].actual_state = state;
			
			if ( state )    {
				// went from up to down
				joy_buttons[i].down_count++;
				joy_buttons[i].down_time = 0;
				
				joy_buttons[i].state = 1;
			} else {
				// went from down to up
				if ( joy_buttons[i].state )     {
					joy_buttons[i].up_count++;
				}
				
				joy_buttons[i].state = 0;
			}
		} else {
			// Didn't move... increment time down if down.
			if (joy_buttons[i].state) {
				joy_buttons[i].down_time += joy_pollrate;
			}
		}
	} 
}

void joy_poll()
{
	STUB_FUNCTION;

/*	if (!Joy_inited)
		return;

	uint lasttic = 0;
	uint curtic = SDL_GetTicks();
	uint delta = curtic - lasttic;

	while (delta >= (uint)joy_pollrate) {
		joy_process(delta);

		lasttic += joy_pollrate;

		delta = curtic - lasttic;
	} */
}

int joy_init()
{
	int i, n;

	if (Joy_inited)
		return 0;

	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
		mprintf(("Could not initialize joystick\n"));
		return 0;
	}

	// enable event processing of the joystick
	if ( (SDL_JoystickEventState(SDL_ENABLE)) != SDL_ENABLE ) {
		mprintf(("ERROR: Unable to initialize joystick event processing!\n"));
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		return 0;
	}

	n = SDL_NumJoysticks();

	if (n < 1) {
		mprintf(("No joysticks found\n"));
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		return 0;
	}

	joy_get_caps(n);

	Cur_joystick = os_config_read_uint(NULL, "CurrentJoystick", JOYSTICKID1);

	sdljoy = SDL_JoystickOpen(Cur_joystick);

	if (sdljoy == NULL) {
		mprintf(("Unable to init joystick %d\n", Cur_joystick));
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		return 0;
	}

	joy_flush();

	joy_num_sticks = n;

	joy_num_buttons = SDL_JoystickNumButtons(sdljoy);
	joy_num_axes = SDL_JoystickNumAxes(sdljoy);
	joy_num_hats = SDL_JoystickNumHats(sdljoy);

	mprintf(( "\nJoystick INITTED!\n\n" ));
	mprintf(( "Using '%s' as the primary joystick:\n", SDL_JoystickName(Cur_joystick) ));
	mprintf(( "  Number of axes: %i\n", joy_num_axes ));
	mprintf(( "  Number of buttons: %i\n", joy_num_buttons ));
	mprintf(( "  Number of hats: %i\n", joy_num_hats ));
	mprintf(( "  Number of trackballs: %i\n\n", SDL_JoystickNumBalls(sdljoy) ));

	// Fake a calibration
	if (joy_num_sticks > 0) {
		for (i=0; i<JOY_NUM_AXES; i++) {
			joystick.axis_center[i] = 32768;
			joystick.axis_min[i] = 0;
			joystick.axis_max[i] = 65536;
		}
	}

	// we poll for axis type motion so be sure to ignore that during normal event state polling
	SDL_EventState( SDL_JOYAXISMOTION, SDL_IGNORE );
	SDL_EventState( SDL_JOYBALLMOTION, SDL_IGNORE );

	// we do want to make sure that hat/button presses go through event polling though
	// (should be on by default already, just here as a reminder)
	SDL_EventState( SDL_JOYBUTTONDOWN, SDL_ENABLE );
	SDL_EventState( SDL_JOYBUTTONUP, SDL_ENABLE );
	SDL_EventState( SDL_JOYHATMOTION, SDL_ENABLE );

	Joy_inited = 1;

	return joy_num_sticks;
}

void joy_set_cen()
{
	joystick_read_raw_axis( 2, joystick.axis_center );
}

int joystick_read_raw_axis(int num_axes, int *axis)
{
	int i;
	
	if (!Joy_inited) {
		// fake a return value so that controlconfig doesn't get freaky with no joystick
		for (i = 0; i < num_axes; i++) {
			axis[i] = 32768;
		}

		return 0;
	}

	Assert( num_axes <= JOY_NUM_AXES );

	for (i = 0; i < num_axes; i++) {
		if (i < joy_num_axes) {
			axis[i] = SDL_JoystickGetAxis(sdljoy, i) + 32768;
		} else {
			axis[i] = 32768;
		}
	}
	
	return 1;
}

void joy_ff_adjust_handling(int speed)
{
//	STUB_FUNCTION;
}

void joy_ff_afterburn_off()
{
//	STUB_FUNCTION;
}

void joy_ff_afterburn_on()
{
//	STUB_FUNCTION;
}

void joy_ff_deathroll()
{
//	STUB_FUNCTION;
}

void joy_ff_docked()
{
//	STUB_FUNCTION;
}

void joy_ff_explode()
{
//	STUB_FUNCTION;
}

void joy_ff_fly_by(int mag)
{
//	STUB_FUNCTION;
}

void joy_ff_mission_init(vec3d v)
{
//	STUB_FUNCTION;
}

void joy_ff_play_dir_effect(float x, float y)
{
//	STUB_FUNCTION;
}

void joy_ff_play_primary_shoot(int gain)
{
//	STUB_FUNCTION;
}

void joy_ff_play_reload_effect()
{
//	STUB_FUNCTION;
}

void joy_ff_play_secondary_shoot(int gain)
{
//	STUB_FUNCTION;
}

void joy_ff_play_vector_effect(vec3d *v, float scaler)
{
//	STUB_FUNCTION;
}

void joy_ff_stop_effects()
{
	joy_ff_afterburn_off();
}

#endif		// Goober5000 - #ifndef WIN32
