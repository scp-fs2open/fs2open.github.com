#ifndef __JOY_H__
#define __JOY_H__
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "globalincs/pstypes.h"
#include "SDL_joystick.h"

#include <climits>

#define JOY_NUM_BUTTONS     32
#define JOY_NUM_HAT_POS     4
#define JOY_TOTAL_BUTTONS   (JOY_NUM_BUTTONS + JOY_NUM_HAT_POS)
#define JOY_NUM_AXES        6

#define JOY_HATBACK     (JOY_NUM_BUTTONS)
#define JOY_HATFORWARD  (JOY_NUM_BUTTONS + 1)
#define JOY_HATLEFT     (JOY_NUM_BUTTONS + 2)
#define JOY_HATRIGHT    (JOY_NUM_BUTTONS + 3)

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

/**
 * @brief Inits the joystick
 *
 * @returns The number of joysticks available, if successful, or
 * @returns 0 otherwise
 */
int joy_init();

/**
 * @brief Shutdowns the joystick
 */
void joy_close();

/**
 * @brief Clears the state of the joystick to initial values
 */
void joy_flush();

/**
 * @brief Gets the values of the first 4 joystick axes.
 *
 * @param[out] x,y,z,r If successful, the values of the axes
 *
 * @returns 1 if successful,
 * @returns 0 if not
 *
 * @details Saves X and Y in Joy_last_x_reading and Joy_last_y_reading. If joystick doesn't support all axes, then the
 *  unsupported axes output values will be 0
 */
int joy_get_pos(int * x, int * y, int *z, int *r);

/**
 * @brief Is this button down?
 *
 * @param[in] btn Joystick button to check
 *
 * @returns nonzero if down, or
 * @returns 0 otherwise
 */
int joy_down(int btn);

/**
 * @brief How many times has this button been down?
 *
 * @param[in] btn          Joystick button to check
 * @param[in] reset_count  If 0, reset the down_count to 0
 *
 * @returns The number of times the button has been pressed
 */
int joy_down_count(int btn, int reset_count = 1);

/**
 * @brief How long has this button been down?
 *
 * @param[in] btn Joystick button to check
 *
 * @returns The time (in ms) this button has been down
 *
 * @details Actually returns a ratio of (n * T) / (t2 - t1) where:
 *   n = Number of joystick poll cycles this button has been down (in cycles),
 *   T = Period of the joystick poll (in ms/cycle),
 *   t1 = The last time the engine checked with joy_down_time() (in ms), and
 *   t2 = The current time the engine checked with joy_down_time() (in ms)
 *   The ratio is clamped to be within 0 and 1, and is unitless.
 */
float joy_down_time(int btn);

/**
 * @brief Sets the current joystick position as its center
 */
void joy_set_cen();

/**
 * @brief Reads the raw axis values from the latest joystick poll.
 *
 * @param[in]  num_axis Size of the output axis[]
 * @param[out] axis     The axis[] to output raw values to.
 *
 * @returns 1 if successful, or
 * @returns 0 otherwise
 */
int joystick_read_raw_axis(int num_axes, int * axis);

/**
 * @brief Get the scaled reading from the given raw value of the given axis
 *
 * @param[in] raw The raw value to scale.
 * @param[in] axn The index of the axis.
 *
 * @returns the scaled reading
 */
int joy_get_scaled_reading(int raw, int axn);

/**
 * @brief Gets the unscaled reading from the given raw value of the given axis
 *
 * @param[in] raw The scaled reading.
 * @param[in] axn The index of the axis.
 *
 * @returns the unscaled reading
 */
int joy_get_unscaled_reading(int raw, int axn);

/**
 * @brief SDL Handler for when the system joystick changes (somebody plugged in/unplugged a controller)
 *
 * @param[in] state  The SDL event type (such as SDL_JOYDEVICEADDED)
 * @param[in] device The SDL joystick device index.
 *
 * @details Currently only cares about the first joystick, if that is removed then there's no joystick available to FSO
 */
void joy_device_changed(int state, int device);

/**
 * @brief Gets the SDL_Joystick* of the joystick we're using
 */
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
