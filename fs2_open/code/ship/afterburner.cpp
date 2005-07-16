/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/Afterburner.cpp $
 * $Revision: 2.15 $
 * $Date: 2005-07-16 04:41:04 $
 * $Author: Goober5000 $
 *
 * C file for managing the afterburners
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.14  2005/07/13 03:35:30  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.13  2005/07/05 17:04:08  phreak
 * fixed an afterburner bug where AI ships would have infinite AB fuel if NO_NETWORK
 * was defined.
 *
 * Revision 2.12  2005/04/25 00:31:14  wmcoolmon
 * Dynamically allocated engine washes; subsystem sounds; armor fixes. Line 4268 of ship.cpp, apparently, works properly; bears further looking into.
 *
 * Revision 2.11  2005/03/02 21:24:46  taylor
 * more NO_NETWORK/INF_BUILD goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.10  2004/07/26 20:47:50  Kazan
 * remove MCD complete
 *
 * Revision 2.9  2004/07/12 16:33:04  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.8  2004/03/05 09:01:51  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.7  2004/01/31 03:56:46  phreak
 * changed "now" to unsigned int
 *
 * Revision 2.6  2003/11/23 00:59:01  Goober5000
 * fixed a bug introduced by Penguin for running with NO_NETWORK defined
 * --Goober5000
 *
 * Revision 2.5  2003/09/13 06:02:03  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.3  2003/08/21 06:11:32  Goober5000
 * removed an extraneous thingy
 * --Goober5000
 *
 * Revision 2.2  2003/08/06 17:37:08  phreak
 * preliminary work on tertiary weapons. it doesn't really function yet, but i want to get something committed
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/13 21:09:28  mharris
 * I think the last of the networking code has ifndef NO_NETWORK...
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     9/05/99 11:24p Jimb
 * changed recharge rate for afterburners based on skill setting.
 * 
 * 6     8/24/99 1:50a Dave
 * Fixed client-side afterburner stuttering. Added checkbox for no version
 * checking on PXO join. Made button info passing more friendly between
 * client and server.
 * 
 * 5     7/08/99 10:53a Dave
 * New multiplayer interpolation scheme. Not 100% done yet, but still
 * better than the old way.
 * 
 * 4     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 3     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 25    5/23/98 2:41p Mike
 * Make Easy the default skill level and prevent old pilot's skill level
 * from carrying into new pilot.
 * 
 * 24    5/07/98 11:01a Lawrance
 * Play afterburner fail sound once energy runs out (and key is still
 * down)
 * 
 * 23    5/04/98 11:08p Hoffoss
 * Expanded on Force Feedback code, and moved it all into Joy_ff.cpp.
 * Updated references everywhere to it.
 * 
 * 22    4/18/98 9:12p Lawrance
 * Added Aureal support.
 * 
 * 21    4/13/98 2:11p Lawrance
 * Change afterburners so they can't be activated any faster than once
 * every 1.2 seconds
 * 
 * 20    4/07/98 1:53p Lawrance
 * Make energy system matter more.
 * 
 * 19    3/31/98 5:18p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 18    2/26/98 10:07p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 17    2/20/98 8:32p Lawrance
 * Add radius parm to sound_play_3d()
 * 
 * 16    12/28/97 5:52p Lawrance
 * fix volume discontinuity bug when afterburners ran out
 * 
 * 15    12/22/97 9:14p Allender
 * fix up some code relating to afterburners in multiplayer.  Clients now
 * control their own afterburners
 * 
 * 14    11/06/97 5:02p Dave
 * Finished reworking standalone multiplayer sequencing. Added
 * configurable observer-mode HUD
 * 
 * 13    10/17/97 9:49a Lawrance
 * remove debug output
 * 
 * 12    10/16/97 5:37p Lawrance
 * fix sound bug where AI afterburner was affecting player afterburner
 * sounds
 * 
 * 11    10/01/97 5:55p Lawrance
 * change call to snd_play_3d() to allow for arbitrary listening position
 * 
 * 10    9/16/97 2:27p Allender
 * Removed unused Assert that will cause problems in the future
 * 
 * 9     8/11/97 9:50a Allender
 * fixed afterburner snafu
 * 
 * 8     8/08/97 9:59a Allender
 * added afterburner code into multiplayer.  Required adding a new physics
 * info flag to indicate afterburners ready to fire. (needed for
 * multiplayer).  Removed some now unused variables in afterburner.cpp
 * 
 * 7     8/05/97 10:48a Lawrance
 * save afterburner data for the player
 * 
 * 6     7/27/97 5:14p Lawrance
 * add afterburners to the player control info
 * 
 * 5     7/25/97 5:02p Lawrance
 * fix bug with afterburner sound
 * 
 * 4     7/23/97 4:30p Lawrance
 * improve how disengage of afterburner works
 * 
 * 3     7/16/97 4:42p Mike
 * Make afterburner shake viewer, not ship.
 * Shake for limited time.
 * Add timestamp_until() function to timer library.
 * 
 * 2     7/11/97 8:57a Lawrance
 * make afterburner work same for player and AI ships
 * 
 * 1     7/10/97 2:24p Lawrance
 *
 * $NoKeywords: $
 */


#include "ship/afterburner.h"
#include "io/joy_ff.h"
#include "gamesnd/gamesnd.h"
#include "ship/ship.h"
#include "object/object.h"
#include "io/timer.h"
#include "render/3d.h"			// needed for View_position, which is used when playing a 3D sound
#include "hud/hudets.h"
#include "freespace2/freespace.h"
#include "network/multi.h"



// ----------------------------------------------------------
// Global to file
// ----------------------------------------------------------
static int		Player_afterburner_loop_id;		// identifies the looping afterburner sound of the player ship
static int		Player_afterburner_loop_delay;	// timestamp used to time the start of the looping afterburner sound
static int		Player_disengage_timer;
static float	Player_afterburner_vol;
static int		Player_afterburner_start_time;


// ----------------------------------------------------------
// local constants
// ----------------------------------------------------------

// The minimum required fuel to engage afterburners
#define MIN_AFTERBURNER_FUEL_TO_ENGAGE		10

#define AFTERBURNER_DEFAULT_VOL					0.5f	// default starting volume (0.0f -> 1.0f)
#define AFTERBURNER_PERCENT_VOL_ATTENUATE		0.30f	// % at which afterburner volume is reduced
#define AFTERBURNER_PERCENT_FOR_LOOP_SND		0.33f
#define AFTERBURNER_VOLUME_UPDATE				250	// consider changing afterburner volume every 100 ms
#define AFTERBURNER_LOOP_DELAY					200	// ms after engage, to start looping sound

#define DISENGAGE_TIME								1500	// time in ms to play faded loop sound when afterburner disengages


float	Skill_level_afterburner_recharge_scale[NUM_SKILL_LEVELS] = {5.0f, 3.0f, 2.0f, 1.5f, 1.0f};

// ----------------------------------------------------------------------------
// afterburner_level_init()
//          
//	call at the start of a mission
//
void afterburner_level_init()
{
	Player_disengage_timer = 1;
	Player_afterburner_vol = AFTERBURNER_DEFAULT_VOL;
	Player_afterburner_loop_id = -1;
	Player_afterburner_start_time = 0;
}

// ----------------------------------------------------------------------------
// afterburners_start() will be called when a ship engages the afterburners.
// This function should only be called once when afterburners first start.  This is
// to start an appropriate sound effect and do any one-time initializations.
//
// parameters:   *objp        ==> pointer to the object starting afterburners
//          
void afterburners_start(object *objp)
{
	ship_info	*sip;
	ship			*shipp;
	float			percent_left;

	Assert( objp != NULL );

	if(objp->type == OBJ_OBSERVER)
		return;

	Assert( objp->type == OBJ_SHIP);
	Assert( objp->instance >= 0 && objp->instance < MAX_SHIPS );

	if ( (objp->flags & OF_PLAYER_SHIP) && (objp == Player_obj) ) {
		unsigned int now;
		now = timer_get_milliseconds();

		if ( (now - Player_afterburner_start_time) < 1300 ) {
			snd_play( &Snds[SND_ABURN_FAIL] );
			return;
		}

		if ( objp->phys_info.flags & PF_AFTERBURNER_WAIT ){
			return;
		}
	}

	if ( objp->phys_info.flags & PF_AFTERBURNER_ON )	{
		return;		// afterburners are already engaged, nothing to do here
	}

	//boosters take precedence
	if (objp->phys_info.flags & PF_BOOSTER_ON)
		return;

	shipp = &Ships[objp->instance];
	Assert( shipp->ship_info_index >= 0 && shipp->ship_info_index < Num_ship_types );
	sip = &Ship_info[shipp->ship_info_index];
	
	if ( !(sip->flags & SIF_AFTERBURNER) )	{
		return;
	}

	// Check if there is enough afterburner fuel
#ifndef NO_NETWORK
	if ( (shipp->afterburner_fuel < MIN_AFTERBURNER_FUEL_TO_ENGAGE) && !MULTIPLAYER_CLIENT ) {
#else
	if ( (shipp->afterburner_fuel < MIN_AFTERBURNER_FUEL_TO_ENGAGE) ) {
#endif
		if ( objp == Player_obj ) {
			snd_play( &Snds[SND_ABURN_FAIL] );
		}
		return;
	}

	objp->phys_info.flags |= PF_AFTERBURNER_ON;

	objp->phys_info.afterburner_decay = timestamp(ABURN_DECAY_TIME);

	percent_left = shipp->afterburner_fuel / sip->afterburner_fuel_capacity;

	if ( objp == Player_obj ) {
		Player_afterburner_start_time = timer_get_milliseconds();
		Player_disengage_timer = 1;
		Player_afterburner_vol = AFTERBURNER_DEFAULT_VOL;

		if ( percent_left > AFTERBURNER_PERCENT_FOR_LOOP_SND ) {
			Player_afterburner_loop_delay = timestamp(AFTERBURNER_LOOP_DELAY);
		}
		else {
			Player_afterburner_loop_delay = 0;
		}

		snd_play( &Snds[SND_ABURN_ENGAGE], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY );
		joy_ff_afterburn_on();
	} else {
		snd_play_3d( &Snds[SND_ABURN_ENGAGE], &objp->pos, &View_position, objp->radius );
	}
	
	objp->phys_info.flags |= PF_AFTERBURNER_WAIT;
}

// ----------------------------------------------------------------------------
// afterburners_update()
//
//	Update the state of the afterburner fuel remaining for an object using the
//	afterburner.  
//
// for the player ship, key_up_time() is called for the afterburner key to
// detect when afterburners disengage.
//
// input:		*objp				=> pointer to the object starting afterburners
//					fl_frametime	=> time in seconds of the last frame
//
void afterburners_update(object *objp, float fl_frametime)
{
	Assert( objp != NULL );
	Assert( objp->type == OBJ_SHIP );
	Assert( objp->instance >= 0 && objp->instance < MAX_SHIPS );
	
	ship_info *sip;
	ship *shipp;
	static int volume_chg_timer = 1;

	shipp = &Ships[objp->instance];

	Assert( shipp->ship_info_index >= 0 && shipp->ship_info_index < Num_ship_types );
	sip = &Ship_info[shipp->ship_info_index];

	if ( (objp->flags & OF_PLAYER_SHIP ) && (Game_mode & GM_DEAD) ) {
		return;
	}

	if ( !(sip->flags & SIF_AFTERBURNER) )	{
		return;		// nothing to update, afterburners are not even on the ship
	}

	//shut the afterburners off if we're using the booster tertiary
	if ( objp->phys_info.flags & PF_BOOSTER_ON)
	{
		if (objp==Player_obj) afterburner_stop_sounds();
		afterburners_stop(objp);
		return;
	}

	if ( objp == Player_obj ) {
		if ( !timestamp_elapsed(Player_disengage_timer) ) {
			float remaining;
			remaining = timestamp_until(Player_disengage_timer) / i2fl(DISENGAGE_TIME);
			if ( remaining <= 0 ) {
				afterburner_stop_sounds();
			}
			else {
				snd_set_volume( Player_afterburner_loop_id, remaining*Player_afterburner_vol);
			}
		}
		else {
			if ( Player_disengage_timer != 1 ) {
				afterburner_stop_sounds();
			}
		}
	}

	// single player, multiplayer servers, and clients for their own ships
	// (when we are running with NO_NETWORK defined, we are always in single-player mode;
	// therefore this code block should always execute)
#ifndef NO_NETWORK
	if(!(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER || (objp == Player_obj))
#endif
	{
		if ( !(objp->phys_info.flags & PF_AFTERBURNER_ON) ) {
			// Recover afterburner fuel

			if ( shipp->afterburner_fuel < sip->afterburner_fuel_capacity ) {
				float recharge_scale;
				recharge_scale = Energy_levels[shipp->engine_recharge_index] * 2.0f * Skill_level_afterburner_recharge_scale[Game_skill_level];
				shipp->afterburner_fuel += (sip->afterburner_recover_rate * fl_frametime * recharge_scale);

				if ( shipp->afterburner_fuel >  sip->afterburner_fuel_capacity){
					shipp->afterburner_fuel = sip->afterburner_fuel_capacity;
				}
			}
			return;
		}
		else {
			// Check if there is enough afterburner fuel
			if ( shipp->afterburner_fuel <= 0 ) {
				shipp->afterburner_fuel = 0.0f;
				afterburners_stop(objp);
				return;
			}
		}

		// afterburners are firing at this point

		// Reduce the afterburner fuel
		shipp->afterburner_fuel -= (sip->afterburner_burn_rate * fl_frametime);
		if ( shipp->afterburner_fuel < 0.0f ) {
			shipp->afterburner_fuel = 0.0f;
		}
	}

	if ( objp == Player_obj ) {
		if ( timestamp_elapsed(Player_afterburner_loop_delay) ) {
			Player_afterburner_vol = AFTERBURNER_DEFAULT_VOL;
			Player_afterburner_loop_delay = 0;
			if ( Player_afterburner_loop_id == -1 ) {
				Player_afterburner_loop_id = snd_play_looping( &Snds[SND_ABURN_LOOP], 0.0f , -1, -1);
				snd_set_volume(Player_afterburner_loop_id, Player_afterburner_vol);
//				nprintf(("Alan","PLAY LOOPING SOUND\n"));
			}
		}

		// Reduce the volume of the afterburner sound if near the end
		if ( timestamp_elapsed(volume_chg_timer) ) {
			float percent_afterburner_left;
			percent_afterburner_left = shipp->afterburner_fuel / sip->afterburner_fuel_capacity;
			volume_chg_timer = timestamp(AFTERBURNER_VOLUME_UPDATE);
			if ( percent_afterburner_left < AFTERBURNER_PERCENT_VOL_ATTENUATE ) {
				Player_afterburner_vol = percent_afterburner_left*(1/AFTERBURNER_PERCENT_VOL_ATTENUATE)*AFTERBURNER_DEFAULT_VOL;
				snd_set_volume(Player_afterburner_loop_id, Player_afterburner_vol);
			}
		}	// end if (timestamp_elapsed(volume_chg_timer))
	}
}

// ----------------------------------------------------------------------------
// afterburners_stop() will be called when a ship disengages the afterburners.
//
// parameters:   *objp				=> pointer to the object starting afterburners
//						key_released	=>	OPTIONAL parameter (default value 0)
//												This is only used for the player object, to
//												manage starting/stopping
//
void afterburners_stop(object *objp, int key_released)
{
	Assert( objp != NULL );
	Assert( objp->instance >= 0 && objp->instance < MAX_SHIPS );
	
	ship_info *sip;
	ship *shipp;

	shipp = &Ships[objp->instance];

	Assert( shipp->ship_info_index >= 0 && shipp->ship_info_index < Num_ship_types );
	sip = &Ship_info[shipp->ship_info_index];

	if ( (objp->flags & OF_PLAYER_SHIP) && key_released ) {
		objp->phys_info.flags &= ~PF_AFTERBURNER_WAIT;
	}

	if ( !(sip->flags & SIF_AFTERBURNER) )	{
		nprintf(("Warning","Ship type %s does not have afterburner capability\n", sip->name));
		return;
	}

	if ( !(objp->phys_info.flags & PF_AFTERBURNER_ON) ) {
		return;
	}

	objp->phys_info.flags &= ~PF_AFTERBURNER_ON;
	float percent_left;
	percent_left = shipp->afterburner_fuel / sip->afterburner_fuel_capacity;

	if ( objp == Player_obj ) {

		if ( !key_released ) {
			snd_play( &Snds[SND_ABURN_FAIL] );
		}

		if ( Player_afterburner_loop_id > -1 )	{
			Player_disengage_timer = timestamp(DISENGAGE_TIME);
		}

		joy_ff_afterburn_off();
	}
}

// ----------------------------------------------------------------------------
// afterburner_stop_sounds() 
//
// Terminates any looping afterburner sounds.
// This should only be called when the game decides to stop all looping sounds.
//
void afterburner_stop_sounds()
{
	if ( Player_afterburner_loop_id != -1 ) {
		snd_stop(Player_afterburner_loop_id);
//		nprintf(("Alan","STOP LOOPING SOUND\n"));
	}

	Player_afterburner_loop_id = -1;
	Player_disengage_timer = 1;
	Player_afterburner_loop_delay = 0;
}
