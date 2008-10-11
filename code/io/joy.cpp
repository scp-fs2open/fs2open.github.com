/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Io/Joy.cpp $
 * $Revision: 2.3.2.1 $
 * $Date: 2006-09-20 05:00:47 $
 * $Author: taylor $
 *
 * Code to read the joystick
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/07/26 20:47:33  Kazan
 * remove MCD complete
 *
 * Revision 2.2  2004/07/12 16:32:51  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 4     6/02/99 6:18p Dave
 * Fixed TNT lockup problems! Wheeeee!
 * 
 * 3     10/09/98 2:57p Dave
 * Starting splitting up OS stuff.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 61    5/24/98 12:56a Mike
 * Put in debug code, but comment out, to check joystick sensitivity.
 * 
 * 60    5/19/98 6:54p Lawrance
 * Set default joystick sensitivity to max
 * 
 * 59    5/13/98 7:14p Hoffoss
 * Made invalid axis return center position value.
 * 
 * 58    5/13/98 1:17a Hoffoss
 * Added joystick axes configurability.
 * 
 * 57    5/07/98 3:15p Hoffoss
 * Fixed typo.
 * 
 * 56    5/07/98 12:41p Hoffoss
 * Changed code to make joystick sensitivity default to center of range.
 * 
 * 55    5/06/98 12:02a Hoffoss
 * Fixed throttle problems with joysticks.
 * 
 * 54    5/05/98 8:38p Hoffoss
 * Added sensitivity adjustment to options menu and made it save to pilot
 * file.
 * 
 * 53    5/04/98 11:08p Hoffoss
 * Expanded on Force Feedback code, and moved it all into Joy_ff.cpp.
 * Updated references everywhere to it.
 * 
 * 52    5/03/98 6:02p Hoffoss
 * Added DirectInput support for joystick (only works with v5.0, so only
 * Win95).
 * 
 * 51    5/01/98 5:59p Hoffoss
 * Checking in code so I can switch back to NT to work on this instead
 * (Win95, bah!)
 * 
 * 50    4/30/98 12:12p Hoffoss
 * Changed code to explicitly use calibrated values from Windows for
 * joystick axis.
 * 
 * 49    4/29/98 12:13a Lawrance
 * Add function for reading down count without resetting internal count.
 * Add hook to reset demo trailer timer.
 * 
 * 48    4/25/98 12:02p Lawrance
 * take out crit sec code around joystick read in joy_process
 * 
 * 47    4/22/98 9:03p John
 * Put critical section around all the joygetposex functions.   Had to set
 * up events to signal the polling thread to end at the end of the
 * program.
 * 
 * 46    4/17/98 3:07p Jim
 * added debug code to test joystick
 * 
 * 45    4/13/98 10:16a John
 * Switched gettime back to timer_get_milliseconds, which is now thread
 * safe.
 * 
 * 44    4/12/98 11:08p Lawrance
 * switch back to using gettime() in separate threads
 * 
 * 43    4/12/98 5:31p Lawrance
 * use timer_get_milliseconds() instead of gettime()
 * 
 * 42    3/21/98 11:29a John
 * Made joy_flush work when a button is held down
 * 
 * 41    3/12/98 10:44a Hoffoss
 * Whoops, should probably use the actual define rather than 0.
 * 
 * 40    3/12/98 10:38a Hoffoss
 * Changed joystick default to first slot in never set up in FSLaunch
 * 
 * 39    3/11/98 5:27p Hoffoss
 * Added to FreeSpace usage of joystick specified through FSLaunch.
 * 
 * 38    3/09/98 4:44p Sandeep
 * 
 * 37    3/07/98 5:23p Sandeep
 * 
 * 35    3/06/98 11:12a Hoffoss
 * Fixed joystick deadzone bug.
 * 
 * 33    3/06/98 10:02a Hoffoss
 * Made dead zone adjustable, and defaulted it to 10% instead of 5%.
 * 
 * 36    3/07/98 4:50p John
 * Added code to allow toggling force feedback on/off in setup
 * 
 * 35    3/06/98 11:12a Hoffoss
 * Fixed joystick deadzone bug.
 * 
 * 33    3/06/98 10:02a Hoffoss
 * Made dead zone adjustable, and defaulted it to 10% instead of 5%.
 * 
 * 32    2/11/98 9:56p Jim
 * allender: from sandeep on Jim's machine -- some force feedback stuff
 * 
 * 31    1/29/98 11:04a Sandeep
 * 
 * 30    1/27/98 8:40p Sandeep
 * 
 * 29    1/19/98 6:15p John
 * Fixed all my Optimized Build compiler warnings
 * 
 * 28    1/08/98 6:35p Hoffoss
 * Fixed joystick undefined detection.
 * 
 * 27    1/08/98 3:48p Dan
 * Fixed bug with joystick axis reading thinking it's undefined at
 * extremes.
 * 
 * 26    10/16/97 5:37p Lawrance
 * change thread priority from THREAD_PRIORITY_TIME_CRITICAL to 
 * THREAD_PRIORITY_HIGHEST
 * 
 * 25    10/09/97 10:15a Johnson
 * try to init several times if joystick init fails
 * 
 * 24    10/07/97 10:42a Johnson
 * zero out JOYINFOEX struct before setting dwSize
 * 
 * 23    10/06/97 5:54p Johnson
 * ALAN: fix nasty bug where dwSize member of JOYINFOEX was not being set,
 * resulting in random failure
 * 
 * 22    9/15/97 11:42p Lawrance
 * change button_info to joy_button_info to avoid name conflict
 * 
 * 21    8/07/97 11:26p Lawrance
 * add support for 4th axis (rudder)
 * 
 * 20    7/29/97 5:30p Lawrance
 * move gettime() from keyboard module to timer module
 * 
 * 19    7/11/97 11:43a Lawrance
 * fix bug with joy_up_count
 * 
 * 18    7/10/97 12:29a Lawrance
 * fix problem with NT not recognizing an axis that was set under 95
 * 
 * 17    7/09/97 11:41p Lawrance
 * added throttle and hat support
 * 
 * 16    6/19/97 9:50a John
 * fixed bug where joy_close getting called out of order doesn't matter.
 * 
 * 15    5/18/97 2:40p Lawrance
 * added joy_get_caps()
 * 
 * 14    4/22/97 10:56a John
 * fixed some resource leaks.
 * 
 * 13    2/27/97 2:23p Lawrance
 * took out debug stmts
 * 
 * 12    2/27/97 10:04a Lawrance
 * fixed bug that was causing all joy buttons but 0 to be read incorrectly
 * 
 * 11    2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 *
 * $NoKeywords: $
 */


#define THREADED

#include "globalincs/pstypes.h"
#include "io/joy.h"
#include "math/fix.h"
#include "io/key.h"
#include "io/timer.h"
#include "osapi/osregistry.h"
#include "io/joy_ff.h"
#include "osapi/osapi.h"

#ifdef _WIN32
#include <windows.h>
#endif


int Joy_inited = 0;
int joy_num_sticks = 0;
int Dead_zone_size = 10;
int Cur_joystick = -1;  // joystick used for input or -1
int Joy_sensitivity = 9;

CRITICAL_SECTION joy_lock;

joy_button_info joy_buttons[JOY_TOTAL_BUTTONS];

Joy_info joystick;

static int Joy_last_x_reading = 0;
static int Joy_last_y_reading = 0;

// --------------------------------------------------------------
//	joy_flush()
//
// Clear the state of the joystick.
//
void joy_flush()
{
	int i;
	joy_button_info	*bi;

	if (joy_num_sticks < 1)
		return;

	ENTER_CRITICAL_SECTION(joy_lock);

	for (i = 0; i < JOY_TOTAL_BUTTONS; i++) {
		bi = &joy_buttons[i];
		bi->state		= 0;
		bi->down_count	= 0;
		bi->up_count	= 0;
		bi->down_time	= 0;
		bi->last_down_check = timer_get_milliseconds();
	}
	
	LEAVE_CRITICAL_SECTION(joy_lock);
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
//	joy_cheap_cal()
//
//	Manual calibrate joystick routine
//
void joy_cheap_cal()
{
	if (joy_num_sticks < 1)
		return;

	while (1) {
		Sleep(50);

		if ( key_inkey() )
			break;

		if ( joy_down_count(0) )
			break;

		mprintf(( "Move stick to upper-left and hit button\n" ));
	}

	joy_set_ul();

	while (1) {
		Sleep(50);

		if ( key_inkey() )
			break;

		if ( joy_down_count(0) )
			break;

		mprintf(( "Move stick to lower-right and hit button\n" ));
	}

	joy_set_lr();

	while (1) {
		Sleep(50);

		if ( key_inkey() )
			break;

		if ( joy_down_count(0) )
			break;

		mprintf(( "Move stick to center and hit button\n" ));
	}

	joy_set_cen();
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

	if (joy_num_sticks < 1)
		return 0;

	if ( (btn < 0) || (btn >= JOY_TOTAL_BUTTONS) )
		return 0;

	ENTER_CRITICAL_SECTION(joy_lock);

	tmp = joy_buttons[btn].down_count;

	if (reset_count)
		joy_buttons[btn].down_count = 0;

	LEAVE_CRITICAL_SECTION(joy_lock);

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

	if (joy_num_sticks < 1)
		return 0;

	if ( (btn < 0) || (btn >= JOY_TOTAL_BUTTONS) )
		return 0;

	ENTER_CRITICAL_SECTION(joy_lock);

	tmp = joy_buttons[btn].state;

	LEAVE_CRITICAL_SECTION(joy_lock);

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

	if (joy_num_sticks < 1)
		return 0;

	if ( (btn < 0) || (btn >= JOY_TOTAL_BUTTONS) )
		return 0;

	ENTER_CRITICAL_SECTION(joy_lock);

	tmp = joy_buttons[btn].up_count;
	joy_buttons[btn].up_count = 0;

	LEAVE_CRITICAL_SECTION(joy_lock);

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
	float	rval;
	uint	now;
	joy_button_info	*bi;

	if (joy_num_sticks < 1)
		return 0.0f;

	if ( (btn < 0) || (btn >= JOY_TOTAL_BUTTONS) )
		return 0.0f;

	bi = &joy_buttons[btn];

	ENTER_CRITICAL_SECTION(joy_lock);
	
	now = timer_get_milliseconds();

	if ( (bi->down_time == 0) && joy_down(btn) )
		bi->down_time += joy_pollrate;

	if ( (now - bi->last_down_check) > 0 )
		rval = i2fl(bi->down_time) / (now - bi->last_down_check);
	else
		rval = 0.0f;

	bi->down_time = 0;
	bi->last_down_check = now;

	LEAVE_CRITICAL_SECTION(joy_lock);

	CLAMP(rval, 0.0f, 1.0f);

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
	CLAMP(raw, 0, rng);


	return (int) ((uint)raw * (uint)F1_0 / (uint)rng);  // convert to 0 - F1_0 range.
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
	percent = d / (float)rng;

	// work sensitivity on axis value
	percent = (percent * sensitivity_percent + percent * percent * percent * percent * percent * non_sensitivity_percent);

	x = (int)(F1_0 * percent);

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

	if (x)
		*x = 0;

	if (y)
		*y = 0;

	if (z)
		*z = 0;

	if (rx)
		*rx = 0;

	if (joy_num_sticks < 1)
		return 0;

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
