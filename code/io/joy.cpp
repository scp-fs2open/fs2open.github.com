/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <windows.h>
#include <windowsx.h>

#include "globalincs/pstypes.h"
#include "io/joy.h"
#include "math/fix.h"
#include "io/key.h"
#include "io/timer.h"
#include "osapi/osregistry.h"
#include "io/joy_ff.h"
#include "directx/vdinput.h"
#include "osapi/osapi.h"



#define PRECALIBRATED 1

static int Joy_inited = 0;
int joy_num_sticks = 0;
int Dead_zone_size = 10;
int Cur_joystick = -1;  // joystick used for input or -1
int Joy_sensitivity = 9;

CRITICAL_SECTION joy_lock;

HANDLE joy_thread = NULL;
DWORD joy_thread_id;
int joy_pollrate = 1000 / 18;  // poll at 18Hz

HANDLE Joy_tell_thread_to_end_event = NULL;
HANDLE Joy_thread_says_its_done_event = NULL;

static int Joy_last_x_reading = 0;
static int Joy_last_y_reading = 0;

int Joy_di_inited = 0;
static LPDIRECTINPUT				Di_joystick_obj = NULL;
static LPDIRECTINPUTDEVICE2	Di_joystick = NULL;

typedef struct joy_button_info {
	int	actual_state;		// Set if the button is physically down
	int	state;				// Set when the button goes from up to down, cleared on down to up.  Different than actual_state after a flush.
	int	down_count;
	int	up_count;
	int	down_time;
	uint	last_down_check;	// timestamp in milliseconds of last 
} joy_button_info;

Joy_info joystick;

joy_button_info joy_buttons[JOY_TOTAL_BUTTONS];

int joy_di_init();
int joy_di_shutdown();
int joystick_read_raw_axis_di(int num_axes, int *axis);

// --------------------------------------------------------------
//	joy_flush()
//
// Clear the state of the joystick.
//
void joy_flush()
{
	int			i;
	joy_button_info	*bi;

	if ( joy_num_sticks < 1 ) return;

	EnterCriticalSection(&joy_lock);
	for ( i = 0; i < JOY_TOTAL_BUTTONS; i++) {
		bi = &joy_buttons[i];
		bi->state		= 0;
		bi->down_count	= 0;
		bi->up_count	= 0;
		bi->down_time	= 0;
		bi->last_down_check = timer_get_milliseconds();
	}
	
	LeaveCriticalSection(&joy_lock);
}

// --------------------------------------------------------------
//	joy_process()
//
// Runs as a separate thread, and updates the state of the joystick
//
DWORD joy_process(DWORD lparam)
{
	MMRESULT		rs;
	JOYINFOEX	ji;
	int			i,state;
	joy_button_info	*bi;
	EXECUTION_STATE last_exectution_state = 0;
	uint last_ssav_time = 0;


	// power management stuff, we need to handle this manually for joysticks
	{
		// give notification that we need both display and system (for multi) resources to remain available
		// NOTE: we'll have to restore the previous execution state before exiting this thread
		last_exectution_state = SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);

		// turn of screen saver, but save the current timeout so that we can restore it later
		SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &last_ssav_time, 0);
		SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 0, NULL, 0);
	}


	for ( i = 0; i < JOY_TOTAL_BUTTONS; i++) {
		bi = &joy_buttons[i];
		bi->actual_state = 0;		// Don't set in flush code!
		bi->state		= 0;
		bi->down_count	= 0;
		bi->up_count	= 0;
		bi->down_time	= 0;
		bi->last_down_check = timer_get_milliseconds();
	}

	while (1) {
		// Wait for the thread to be signaled to end or 1/18th of a second to pass...
		if ( WaitForSingleObject( Joy_tell_thread_to_end_event, joy_pollrate )==WAIT_OBJECT_0)	{
			break;
		}

		memset(&ji, 0, sizeof(ji));
		ji.dwSize = sizeof(ji);
//		ji.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNRAWDATA;
		ji.dwFlags = JOY_RETURNALL;

		EnterCriticalSection(&joy_lock);

		uint joy_state = 0;
		if (Cur_joystick >= 0) {
			rs = joyGetPosEx(Cur_joystick, &ji);
			// If there's an error, assume all buttons down.
			if (rs == JOYERR_NOERROR) {
				joy_state = ji.dwButtons;
			}
		}

		// Process ji.dwButtons
		for (i=0; i<JOY_TOTAL_BUTTONS; i++) {
			state = 0;
			if (i < JOY_NUM_BUTTONS) {
				state = joy_state & (1<<i);

			} else {
				// check for hat presses, which act like buttons
				switch (i) {
					case JOY_HATBACK:
						if (ji.dwPOV == JOY_POVBACKWARD)
							state = 1;
						break;

					case JOY_HATFORWARD:
						if (ji.dwPOV == JOY_POVFORWARD)
							state = 1;
						break;

					case JOY_HATLEFT:
						if (ji.dwPOV == JOY_POVLEFT)
							state = 1;
						break;

					case JOY_HATRIGHT:
						if (ji.dwPOV == JOY_POVRIGHT)
							state = 1;
						break;

					default:
						Int3();	// should never happen
						break;

				}	// end switch
			}	// end if


			if (state != joy_buttons[i].actual_state) {
				// Button position physically changed.
				joy_buttons[i].actual_state = state;

				if ( state )	{
					// went from up to down
					joy_buttons[i].down_count++;
					joy_buttons[i].down_time = 0;

					joy_buttons[i].state = 1;

////////////////////////////
/// SOMETHING TERRIBLE IS ABOUT TO HAPPEN.  I FEEL THIS IS NECESSARY FOR THE DEMO, SINCE
/// I DON'T WANT TO CALL CRITICAL SECTION CODE EACH FRAME TO CHECK ALL THE JOYSTICK BUTTONS.
/// PLEASE SEE ALAN FOR MORE INFORMATION.
////////////////////////////
#ifdef FS2_DEMO
					{
					extern void demo_reset_trailer_timer();
					demo_reset_trailer_timer();
					}
#endif
////////////////////////////
/// IT'S OVER.  SEE, IT WASN'T SO BAD RIGHT?  IT'S IS VERY UGLY LOOKING, I KNOW.
////////////////////////////


				} else {
					// went from down to up
					if ( joy_buttons[i].state )	{
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

		}  // end for

		LeaveCriticalSection(&joy_lock);
	}

	// power management stuff, we need to handle this manually for joysticks
	{
		// restore the original execution state
		last_exectution_state = SetThreadExecutionState(last_exectution_state);

		// restore the original screensaver timeout value
		SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, last_ssav_time, NULL, 0);
	}

	SetEvent(Joy_thread_says_its_done_event);	

	return 0;
}

// --------------------------------------------------------------
//	joy_close()
//
// Close the joystick system.  Should be called at game exit.
//
void joy_close()
{
	if (!Joy_inited)
		return;

	// joy_di_shutdown();

	Joy_inited = 0;
	joy_num_sticks = 0;

	// Tell joystick polling thread to end
	SetEvent(Joy_tell_thread_to_end_event);

	// Wait for it to end
	if ( WaitForSingleObject( Joy_thread_says_its_done_event, 5000 )==WAIT_TIMEOUT)	{			//INFINITE );
		mprintf(( "Joy end thread wait timeout!\n" ));
	}
	CloseHandle(Joy_tell_thread_to_end_event);
	CloseHandle(Joy_thread_says_its_done_event);

	// It is now safe to release any resources use by the polling thread.
	DeleteCriticalSection( &joy_lock );
	if (joy_thread)	{
		CloseHandle(joy_thread);
		joy_thread = NULL;
	}

	joy_ff_shutdown();
}

// --------------------------------------------------------------
//	joy_get_caps()
//
// Determine the capabilities of the attached joysticks.
//
void joy_get_caps(int max)
{
	JOYCAPS JoyCaps;
	int j;

	for (j=0; j<JOY_NUM_AXES; j++)
		joystick.axis_valid[j] = 0;

	for (j=JOYSTICKID1; j<JOYSTICKID1+max; j++) {
		if (JOYERR_NOERROR == joyGetDevCaps (j, &JoyCaps, sizeof(JoyCaps))) {
			nprintf(("JOYSTICK", "Joystick #%d: %s\n", j - JOYSTICKID1 + 1, JoyCaps.szPname));

			if (j == Cur_joystick) {
				joystick.axis_valid[0] = joystick.axis_valid[1] = 1;
				if (JoyCaps.wCaps & JOYCAPS_HASZ)
					joystick.axis_valid[2] = 1;
				if (JoyCaps.wCaps & JOYCAPS_HASR)
					joystick.axis_valid[3] = 1;
				if (JoyCaps.wCaps & JOYCAPS_HASU)
					joystick.axis_valid[4] = 1;
				if (JoyCaps.wCaps & JOYCAPS_HASV)
					joystick.axis_valid[5] = 1;
			}
		}
	}
}

int joy_get_scaled_reading(int raw, int axn);
int joy_get_unscaled_reading(int raw, int axn);

DCF(joytest, "Test joystick")
{
	if (Dc_command) {
		while (!keyd_pressed[KEY_ESC]) {
			int x, y, axis[JOY_NUM_AXES];

			if (joy_num_sticks < 1)
				return;

			joystick_read_raw_axis(JOY_NUM_AXES, axis);

			x = joy_get_scaled_reading(axis[0], 0);
			y = joy_get_scaled_reading(axis[1], 1);

			mprintf(("X=%5d Y=%5d  Calibrated X=%6d Y=%6d\n", axis[0], axis[1], x, y));
			Sleep(100);
		}
	}
}

DCF(joytest2, "Test joystick (extended)")
{
	if (Dc_command) {
		while (!keyd_pressed[KEY_ESC]) {
			int x, y, z, r, axis[JOY_NUM_AXES];

			if (joy_num_sticks < 1)
				return;

			joystick_read_raw_axis(JOY_NUM_AXES, axis);

			x = joy_get_scaled_reading(axis[0], 0);
			y = joy_get_scaled_reading(axis[1], 1);
			z = joy_get_unscaled_reading(axis[2], 2);
			r = joy_get_scaled_reading(axis[3], 3);

			mprintf(("X=%5d Y=%5d Z=%5d Rx=%5d Ry=%5d Rz=%5d Cal X=%6d Y=%6d Z=%6d R=%6d\n", axis[0], axis[1], axis[2], axis[3], axis[4], axis[5], x, y, z, r));
			Sleep(100);
		}
	}
}

// --------------------------------------------------------------
//	joy_init()
//
// Initialize the joystick system.  This is called once at game startup.
//

int joy_init()
{
	int i, n, count;
	MMRESULT rs;
	JOYINFOEX ji;

	if (Joy_inited)
		return 0;

	Joy_inited = 1;
	n = joyGetNumDevs();
	Cur_joystick = os_config_read_uint(NULL, "CurrentJoystick", JOYSTICKID1);

	joy_get_caps(n);

	if (n < 1) {
		mprintf(("No joystick driver detected\n"));
		return 0;
	}

	InitializeCriticalSection(&joy_lock);
	atexit(joy_close);

	joy_flush();

	joy_num_sticks = 0;
	memset(&ji, 0, sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = JOY_RETURNALL;

	if (Cur_joystick >= 0) {
		// AL: test code to try and find out why this call fails the first time
		rs = 0;
		for (count=0; count<20; count++) {
			rs = joyGetPosEx(Cur_joystick, &ji);
			if (rs == JOYERR_NOERROR)
				break;
		}
			
		if (rs == JOYERR_NOERROR) {
			joy_num_sticks++;

			Joy_tell_thread_to_end_event = CreateEvent( NULL, FALSE, FALSE, NULL );
			Joy_thread_says_its_done_event = CreateEvent( NULL, FALSE, FALSE, NULL );

			joy_thread = CreateThread(NULL, 
											1024 * 32,
											(LPTHREAD_START_ROUTINE) joy_process,
											NULL,
											0,
											&joy_thread_id);

		//		SetThreadPriority(joy_thread, THREAD_PRIORITY_TIME_CRITICAL - 1);
			SetThreadPriority(joy_thread, THREAD_PRIORITY_HIGHEST);
		}
	}

	mprintf(("Windows reported %d joysticks, we found %d\n", n, joy_num_sticks));

#ifdef PRECALIBRATED
	// Fake a calibration
	if (joy_num_sticks > 0) {
		for (i=0; i<4; i++) {
			joystick.axis_min[i] = 0;
			joystick.axis_center[i] = 32768;
			joystick.axis_max[i] = 65536;
		}
	}
#else
	// Fake a calibration
	if (joy_num_sticks > 0) {
		joy_set_cen();
		for (i=0; i<4; i++) {
			joystick.axis_min[i] = 0;
			joystick.axis_max[i] = joystick.axis_center[i]*2;
		}
	}
#endif

	joy_ff_init();
	// joy_di_init();

	return joy_num_sticks;
}

// --------------------------------------------------------------
//	joy_cheap_cal()
//
//	Manual calibrate joystick routine
//
void joy_cheap_cal()
{
	if ( joy_num_sticks < 1 ) return;

	while(1)	{
		Sleep(50);
		if ( key_inkey()) break;
		if (joy_down_count(0)) break;
		mprintf(( "Move stick to upper-left and hit button\n" ));
	}
	joy_set_ul();

	while(1)	{
		Sleep(50);
		if ( key_inkey()) break;
		if (joy_down_count(0)) break;
		mprintf(( "Move stick to lower-right and hit button\n" ));
	}
	joy_set_lr();

	while(1)	{
		Sleep(50);
		if ( key_inkey()) break;
		if (joy_down_count(0)) break;
		mprintf(( "Move stick to center and hit button\n" ));
	}
	joy_set_cen();
}

// --------------------------------------------------------------
//	joy_get_pos_old()
//
//	Get the position of the joystick axes
//
int joy_get_pos_old(int * x, int * y )
{
	MMRESULT rs;
	JOYINFOEX ji;

	if ( joy_num_sticks < 1 ) {
		if (x) *x = 0;
		if (y) *y = 0;
		return 0;
	}

	memset(&ji, 0, sizeof(ji));
	ji.dwSize = sizeof(ji);

#ifdef PRECALIBRATED
//JOY_USEDEADZONE|
//JOY_RETURNCENTERED|
//JOY_RETURNX|JOY_RETURNY|JOY_RETURNRAWDATA;
	ji.dwFlags = JOY_RETURNALL;
#else
	ji.dwFlags = JOY_CAL_READXYONLY;
#endif

	if (Cur_joystick >= 0) {
		EnterCriticalSection(&joy_lock);
		rs = joyGetPosEx(Cur_joystick, &ji);
		LeaveCriticalSection(&joy_lock);

		if (rs == JOYERR_NOERROR) {
#if 1
			if (x)
				*x = ji.dwXpos;
			if (y)
				*y = ji.dwYpos;
#else
			if (x) {	
				*x = (ji.dwXpos - 32768) * 2;
				if (*x < -65536)
					*x = -65536;
				else if (*x > 65536)
					*x = 65536;
			}

			if (y) {
				*y = (ji.dwYpos - 32768) * 2;
				if (*y < -65536)
					*y = -65536;
				else if (*y > 65536)
					*y = 65536;
			}
#endif
			return 1;
		}
	}

	if (x)
		*x = 0;
	if (y)
		*y = 0;

	return 0;
}


// --------------------------------------------------------------
//	joy_down_count()
//
//	Return the number of times the joystick button has gone down since
// joy_down_count() was last called
//
//	input:		btn			=>		button number to check
//					reset_count	=>		(default 1): if true reset down_count
//
//	returns:		number of times button 'btn' has gone down since last call
//
int joy_down_count(int btn, int reset_count)
{
	int tmp;

	if ( joy_num_sticks < 1 ) return 0;
	if ( (btn < 0) || (btn >= JOY_TOTAL_BUTTONS)) return 0;

	EnterCriticalSection(&joy_lock);
	tmp = joy_buttons[btn].down_count;
	if ( reset_count ) {
		joy_buttons[btn].down_count = 0;
	}
	LeaveCriticalSection(&joy_lock);

	return tmp;
}


// --------------------------------------------------------------
//	joy_down()
//
//	Return the state of button number 'btn'
//
//	input:		btn	=>		button number to check
//
//	returns:		0		=>		not pressed
//					1		=>		pressed
//
int joy_down(int btn)
{
	int tmp;

	if ( joy_num_sticks < 1 ) return 0;
	if ( (btn < 0) || (btn >= JOY_TOTAL_BUTTONS )) return 0;

	EnterCriticalSection(&joy_lock);
	tmp = joy_buttons[btn].state;
	LeaveCriticalSection(&joy_lock);

	return tmp;
}

// --------------------------------------------------------------
//	joy_up_count()
//
//	Return the number of times the joystick button has gone up since
// joy_up_count() was last called
//
//	input:		btn	=>		button number to check
//
//	returns:		number of times button 'btn' has gone up since last call
//
int joy_up_count(int btn)
{
	int tmp;

	if ( joy_num_sticks < 1 ) return 0;
	if ( (btn < 0) || (btn >= JOY_TOTAL_BUTTONS)) return 0;

	EnterCriticalSection(&joy_lock);
	tmp = joy_buttons[btn].up_count;
	joy_buttons[btn].up_count = 0;
	LeaveCriticalSection(&joy_lock);

	return tmp;
}

// --------------------------------------------------------------
//	joy_down_time()
//
//	Return a number between 0 and 1.  This number represents the percentage
// time that the joystick button has been down since it was last checked
//
//	input:		btn	=>		button number to check
//	returns:		value between 0 and 1
//
float joy_down_time(int btn)
{
	float				rval;
	unsigned int	now;
	joy_button_info		*bi;

	if ( joy_num_sticks < 1 ) return 0.0f;
	if ( (btn < 0) || (btn >= JOY_TOTAL_BUTTONS)) return 0.0f;
	bi = &joy_buttons[btn];
	EnterCriticalSection(&joy_lock);
	
	now = timer_get_milliseconds();

	if ( bi->down_time == 0 && joy_down(btn) ) {
		bi->down_time += joy_pollrate;
	}

	if (now > bi->last_down_check)
		rval = i2fl(bi->down_time) / (now - bi->last_down_check);
	else
		rval = 0.0f;

	bi->down_time = 0;
	bi->last_down_check = now;
	
	LeaveCriticalSection(&joy_lock);

	if (rval < 0)
		rval = 0.0f;
	if (rval > 1)
		rval = 1.0f;

	return rval;
}

// --------------------------------------------------------------
//	joy_get_cal_vals()
//
//	Get the calibrated min, center, and max for all axes
//
//	input:	axis_min		=>		OUTPUT PARAMETER:	array of at least 4 ints to hold min axis values
//				axis_center	=>		OUTPUT PARAMETER:	array of at least 4 ints to hold center axis values
//				axis_min		=>		OUTPUT PARAMETER:	array of at least 4 ints to hold max axis values
//
void joy_get_cal_vals(int *axis_min, int *axis_center, int *axis_max)
{
	int i;

	for ( i = 0; i < 4; i++)		{
		axis_min[i] = joystick.axis_min[i];
		axis_center[i] = joystick.axis_center[i];
		axis_max[i] = joystick.axis_max[i];
	}
}

// --------------------------------------------------------------
//	joy_set_cal_vals()
//
//	Get the calibrated min, center, and max for all axes
//
//	input:	axis_min		=>		array of at 4 ints that hold min axis values
//				axis_center	=>		array of at 4 ints that hold center axis values
//				axis_min		=>		array of at 4 ints that hold max axis values
//
void joy_set_cal_vals(int *axis_min, int *axis_center, int *axis_max)
{
	int i;

	for (i=0; i<4; i++)		{
		joystick.axis_min[i] = axis_min[i];
		joystick.axis_center[i] = axis_center[i];
		joystick.axis_max[i] = axis_max[i];
	}
}

// --------------------------------------------------------------
//	joystick_read_raw_axis()
//
//	Read the raw axis information for a specified number of axes.
//
//	input:	num_axes	=>		number of axes to read.  Note the axes go in the following order:
//									dwXpos
//									dwYpos 
//									dwZpos (throttle)
//									dwRpos (rudder)
//									dwUpos (5th axis)
//									dwVpos (6th axis) 
//
//				axis		=>		an array of at least 4 ints to hold axis data
//
int joystick_read_raw_axis(int num_axes, int *axis)
{
	MMRESULT		rs;
	JOYINFOEX	ji;
	int			i;

	Assert(num_axes <= JOY_NUM_AXES);
	for (i=0; i<num_axes; i++)
		axis[i] = 32768;

	// first, try reading with DirectInput, if we can
	if (Joy_di_inited)
		return joystick_read_raw_axis_di(num_axes, axis);

	// No DirectInput joystick, fall back on older stuff
	if (joy_num_sticks < 1)
		return 0;

	memset(&ji, 0, sizeof(ji));
	ji.dwSize = sizeof(ji);

#ifdef PRECALIBRATED
	ji.dwFlags = JOY_RETURNALL;
#else
	ji.dwFlags = JOY_RETURNRAWDATA;
#endif

	switch (num_axes) {
		case	6:
			ji.dwFlags |= JOY_RETURNV;
		case	5:
			ji.dwFlags |= JOY_RETURNU;
		case	4:
			ji.dwFlags |= JOY_RETURNR;
		case	3:
			ji.dwFlags |= JOY_RETURNZ;
		case	2:
			ji.dwFlags |= JOY_RETURNY;
		case	1:
			ji.dwFlags |= JOY_RETURNX;
			break;

		default:
			Int3();
			break;
	}

	if (Cur_joystick >= 0) {
		EnterCriticalSection(&joy_lock);
		rs = joyGetPosEx(Cur_joystick, &ji);
		LeaveCriticalSection(&joy_lock);

	} else
		return 0;

	if (rs != JOYERR_NOERROR)
		return 0;

	switch (num_axes) {
		case 6:
			if (joystick.axis_valid[5])
				axis[5] = ji.dwVpos;

		case 5:
			if (joystick.axis_valid[4])
				axis[4] = ji.dwUpos;

		case 4:
			if (joystick.axis_valid[3])
				axis[3] = ji.dwRpos;

		case 3:
			if (joystick.axis_valid[2])
				axis[2] = ji.dwZpos;

		case 2:
			if (joystick.axis_valid[1])
				axis[1] = ji.dwYpos;

		case 1:
			if (joystick.axis_valid[0])
				axis[0] = ji.dwXpos;

			break;

		default:
			Int3();
			break;
	}

	return 1;
}

// --------------------------------------------------------------
//	joy_set_ul()
//
//	Get the minimum axis information (namely, joystick in upper left).
// This is called by a manual calibration routine.
//
//	NOTE:	sets the values in joystick.axis_min[]
//
void joy_set_ul()	
{
	joystick_read_raw_axis( 2, joystick.axis_min );
}

// --------------------------------------------------------------
//	joy_set_lr()
//
//	Get the maximum axis information (namely, joystick in lower right).
// This is called by a manual calibration routine.
//
//	NOTE:	sets the values in joystick.axis_max[]
//
void joy_set_lr()	
{
	joystick_read_raw_axis( 2, joystick.axis_max );
}

// --------------------------------------------------------------
//	joy_set_cen()
//
//	Get the center axis information (namely, joystick in dead zone).
// This is called by a manual calibration routine.
//
//	NOTE:	sets the values in joystick.axis_center[]
//
void joy_set_cen() 
{
	joystick_read_raw_axis( 2, joystick.axis_center );
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

	return (int) ((unsigned int) raw * (unsigned int) F1_0 / (unsigned int) rng);  // convert to 0 - F1_0 range.
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
	float percent;

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

	// find percent of max axis is at
	percent = (float) d / (float) rng;

	// work sensitivity on axis value
	percent = pow(percent, (3.0f - ((float) Joy_sensitivity / 4.5f)));

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
		Joy_last_x_reading = *y;

	return 1;
}

// change in joy position since last call
void joy_get_delta(int *dx, int *dy)
{
	static int old_joy_x = 0;
	static int old_joy_y = 0;

	if ( !Joy_inited ) {
		*dx = *dy = 0;
		return;
	}

	*dx = Joy_last_x_reading - old_joy_x;
	*dy = Joy_last_y_reading - old_joy_y;

	old_joy_x = Joy_last_x_reading;
	old_joy_y = Joy_last_y_reading;
}

////  This is the DirectInput joystick stuff

GUID Di_joy_guid;
int Di_joy_guid_valid = 0;

BOOL CALLBACK joy_di_enum(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	char buf[64];   

	nprintf(("Joystick", "Joystick detected: %s (%s)\n", lpddi->tszInstanceName, lpddi->tszProductName));
	sprintf(buf, "Joystick %d", Cur_joystick + 1);
	if (!stricmp(buf, lpddi->tszInstanceName)) {
		Di_joy_guid = lpddi->guidInstance;
		Di_joy_guid_valid = 1;
		nprintf(("Joystick", "   (Selected joystick)\n"));
	}

	return DIENUM_CONTINUE;
}
/*
BOOL FAR PASCAL InitJoystickInput(LPCDIDEVICEINSTANCE pdinst, LPVOID pvRef)
{
   LPDIRECTINPUT pdi = pvRef;
   LPDIRECTINPUTDEVICE pdev;
   DIPROPRANGE diprg;

   // create the DirectInput joystick device
   if(pdi->lpVtbl->CreateDevice(pdi, &pdinst->guidInstance, &pdev, NULL) != DI_OK)
   {
      OutputDebugString("IDirectInput::CreateDevice FAILED\n");
      return DIENUM_CONTINUE;
   }

   // set joystick data format
   if (pdev->lpVtbl->SetDataFormat(pdev, &c_dfDIJoystick) != DI_OK)
   {
      OutputDebugString("IDirectInputDevice::SetDataFormat FAILED\n");
      pdev->lpVtbl->Release(pdev);
      return DIENUM_CONTINUE;
   }

   // set the cooperative level
   if (pdev->lpVtbl->SetCooperativeLevel(pdev, hWndMain,
      DISCL_NONEXCLUSIVE | DISCL_FOREGROUND) != DI_OK)
   {
      OutputDebugString("IDirectInputDevice::SetCooperativeLevel FAILED\n");
      pdev->lpVtbl->Release(pdev);
      return DIENUM_CONTINUE;
   }

   // set X-axis range to (-1000 ... +1000)
   // This lets us test against 0 to see which way the stick is pointed.

   diprg.diph.dwSize       = sizeof(diprg);
   diprg.diph.dwHeaderSize = sizeof(diprg.diph);
   diprg.diph.dwObj        = DIJOFS_X;
   diprg.diph.dwHow        = DIPH_BYOFFSET;
   diprg.lMin              = -1000;
   diprg.lMax              = +1000;

   if (pdev->lpVtbl->SetProperty(pdev, DIPROP_RANGE, &diprg.diph) != DI_OK)
   {
      OutputDebugString("IDirectInputDevice::SetProperty(DIPH_RANGE) FAILED\n");
      pdev->lpVtbl->Release(pdev);
      return FALSE;
   }

   //
   // And again for Y-axis range
   //
   diprg.diph.dwObj        = DIJOFS_Y;

   if (pdev->lpVtbl->SetProperty(pdev, DIPROP_RANGE, &diprg.diph) != DI_OK)
   {
      OutputDebugString("IDirectInputDevice::SetProperty(DIPH_RANGE) FAILED\n");
      pdev->lpVtbl->Release(pdev);
      return FALSE;
   }

   // set X axis dead zone to 50% (to avoid accidental turning)
   // Units are ten thousandths, so 50% = 5000/10000.
   if (SetDIDwordProperty(pdev, DIPROP_DEADZONE, DIJOFS_X, DIPH_BYOFFSET, 5000) != DI_OK)
   {
      OutputDebugString("IDirectInputDevice::SetProperty(DIPH_DEADZONE) FAILED\n");
      pdev->lpVtbl->Release(pdev);
      return FALSE;
   }


   // set Y axis dead zone to 50% (to avoid accidental thrust)
   // Units are ten thousandths, so 50% = 5000/10000.
   if (SetDIDwordProperty(pdev, DIPROP_DEADZONE, DIJOFS_Y, DIPH_BYOFFSET, 5000) != DI_OK)
   {
      OutputDebugString("IDirectInputDevice::SetProperty(DIPH_DEADZONE) FAILED\n");
      pdev->lpVtbl->Release(pdev);
      return FALSE;
   }


   // Add it to our list of devices.  If AddInputDevice succeeds,
   // he will do an AddRef.
   AddInputDevice(pdev, pdinst);
        hRes = pdev->lpVtbl->QueryInterface(
                    pdev, &IID_IDirectInputDevice2,
                    (LPVOID *)&g_rgpdevFound[g_cpdevFound]);

   pdev->lpVtbl->Release(pdev);

   return DIENUM_CONTINUE;
}
*/
int joy_di_init()
{
	HRESULT hr;
   LPDIRECTINPUTDEVICE pdev;

	Joy_di_inited = 0;
	hr = DirectInputCreate(GetModuleHandle(NULL), 0x500, &Di_joystick_obj, NULL);
	if (FAILED(hr)) {
		mprintf(( "DirectInputCreate() failed!\n" ));
		return -1;
	}

	Di_joy_guid_valid = 0;
	hr = Di_joystick_obj->EnumDevices(DIDEVTYPE_JOYSTICK, joy_di_enum, Di_joystick_obj, DIEDFL_ATTACHEDONLY);
	if (FAILED(hr)) {
		mprintf(( "EnumDevice() failed!\n" ));
		return -1;
	}

	if (!Di_joy_guid_valid) {
		mprintf(( "Correct joystick not found.\n" ));
		return -1;
	}

	hr = Di_joystick_obj->CreateDevice(Di_joy_guid, &pdev, NULL);
	if (FAILED(hr)) {
		mprintf(( "CreateDevice() failed!\n" ));
		return -1;
	}

	hr = pdev->SetDataFormat(&c_dfDIJoystick);
	if (FAILED(hr)) {
		mprintf(( "SetDataFormat() failed!\n" ));
		if (hr == DIERR_ACQUIRED)
			mprintf(( "   (reason: DIERR_ACQUIRED)\n" ));

		if (hr == DIERR_INVALIDPARAM)
			mprintf(( "   (reason: DIERR_INVALIDPARAM)\n" ));

		if (hr == DIERR_NOTINITIALIZED)
			mprintf(( "   (reason: DIERR_NOTINITIALIZED)\n" ));

		pdev->Release();
		return -1;
	}

	hr = pdev->SetCooperativeLevel((HWND) os_get_window(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr)) {
		mprintf(( "SetCooperativeLevel() failed!\n" ));
		if (hr == DIERR_ACQUIRED)
			mprintf(( "   (reason: DIERR_ACQUIRED)\n" ));

		if (hr == DIERR_INVALIDPARAM)
			mprintf(( "   (reason: DIERR_INVALIDPARAM)\n" ));

		if (hr == DIERR_NOTINITIALIZED)
			mprintf(( "   (reason: DIERR_NOTINITIALIZED)\n" ));

		pdev->Release();
		return -1;
	}

	hr = pdev->QueryInterface(IID_IDirectInputDevice2, (LPVOID *) &Di_joystick);
	if (FAILED(hr)) {
		pdev->Release();
		return -1;
	}

	Di_joystick->Acquire();

	pdev->Release();
	Joy_di_inited = 1;
	nprintf(("Joystick", "DirectInput initialization of joystick succeeded\n"));
	return 0;
}

int joy_di_shutdown()
{
	// Destroy any lingering IDirectInputDevice object.
	if (Di_joystick) {
		// Unacquire the device one last time just in case we got really confused
		// and tried to exit while the device is still acquired.
		Di_joystick->Unacquire();

		Di_joystick->Release();
		Di_joystick = NULL;
	}

	// Destroy any lingering IDirectInput object.
	if (Di_joystick_obj) {
		Di_joystick_obj->Release();
		Di_joystick_obj = NULL;
	}

	Joy_di_inited = 0;
	return 0;
}

int joystick_read_raw_axis_di(int num_axes, int *axis)
{
	int repeat = 1;
	HRESULT hr = 0;
	DIJOYSTATE joy_state;

	if (!Joy_di_inited)
		return 0;

	repeat = 1;
	while (repeat) {
		repeat = 0;

	   hr = Di_joystick->Poll();
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED)) {
			// DirectInput is telling us that the input stream has
			// been interrupted.  We aren't tracking any state
			// between polls, so we don't have any special reset
			// that needs to be done.  We just re-acquire and
			// try again.
			Sleep(1000);		// Pause a second...
			hr = Di_joystick->Acquire();
			if (SUCCEEDED(hr))
				repeat = 1;
		}
	}

	repeat = 1;
	memset(&joy_state, 0, sizeof(joy_state));
	while (repeat) {
		repeat = 0;

		hr = Di_joystick->GetDeviceState(sizeof(joy_state), &joy_state);
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED)) {
			// DirectInput is telling us that the input stream has
			// been interrupted.  We aren't tracking any state
			// between polls, so we don't have any special reset
			// that needs to be done.  We just re-acquire and
			// try again.
			Sleep(1000);		// Pause a second...
			hr = Di_joystick->Acquire();
			if (SUCCEEDED(hr))
				repeat = 1;
		}
	}

	if (SUCCEEDED(hr)) {
		switch (num_axes) {
			case 6:
				if (joystick.axis_valid[5])
					axis[5] = joy_state.lRy;

			case 5:
				if (joystick.axis_valid[4])
					axis[4] = joy_state.lRx;

			case 4:
				if (joystick.axis_valid[3])
					axis[3] = joy_state.lRz;

			case 3:
				if (joystick.axis_valid[2])
					axis[2] = joy_state.lZ;

			case 2:
				if (joystick.axis_valid[1])
					axis[1] = joy_state.lY;

			case 1:
				if (joystick.axis_valid[0])
					axis[0] = joy_state.lX;

				break;
		}
	}

	return 1;
}
