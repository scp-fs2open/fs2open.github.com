/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Io/Joy_ff.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:24 $
 * $Author: penguin $
 *
 * Code for joystick Force Feedback.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2002/05/10 06:05:08  mharris
 * Porting... added ifndef NO_JOYSTICK
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 4     5/08/98 5:31p Hoffoss
 * Isolated the joystick force feedback code more from dependence on other
 * libraries.
 * 
 * 3     5/07/98 12:24a Hoffoss
 * Finished up sidewinder force feedback support.
 * 
 * 2     5/04/98 11:08p Hoffoss
 * Expanded on Force Feedback code, and moved it all into Joy_ff.cpp.
 * Updated references everywhere to it.
 * 
 * $NoKeywords: $
 */

#ifndef __JOY_FF_H__
#define __JOY_FF_H__

#ifndef NO_JOYSTICK

int joy_ff_init();
void joy_ff_shutdown();
void joy_ff_stop_effects();
void joy_ff_mission_init(vector v);
void joy_reacquire_ff();
void joy_unacquire_ff();
void joy_ff_play_vector_effect(vector *v, float scaler);
void joy_ff_play_dir_effect(float x, float y);
void joy_ff_play_primary_shoot(int gain);
void joy_ff_play_secondary_shoot(int gain);
void joy_ff_adjust_handling(int speed);
void joy_ff_docked();
void joy_ff_play_reload_effect();
void joy_ff_afterburn_on();
void joy_ff_afterburn_off();
void joy_ff_explode();
void joy_ff_fly_by(int mag);
void joy_ff_deathroll();

#else

#define joy_ff_init()						 (0)
#define joy_ff_shutdown()
#define joy_ff_stop_effects()
#define joy_ff_mission_init(v)			 ((void)(v))
#define joy_reacquire_ff()
#define joy_unacquire_ff()
#define joy_ff_play_vector_effect(v, scaler)		((void)((v), (scaler)))
#define joy_ff_play_dir_effect(x, y)				((void)((x), (y)))
#define joy_ff_play_primary_shoot(gain)			((void)(gain))
#define joy_ff_play_secondary_shoot(gain)			((void)(gain))
#define joy_ff_adjust_handling(speed)				((void)(speed))
#define joy_ff_docked()
#define joy_ff_play_reload_effect()
#define joy_ff_afterburn_on()
#define joy_ff_afterburn_off()
#define joy_ff_explode()
#define joy_ff_fly_by(mag)								((void)(mag))
#define joy_ff_deathroll()

#endif // ifndef NO_JOYSTICK

#endif
