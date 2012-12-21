/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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


/**
 * Call at the start of a mission
 */
void afterburner_level_init()
{
	Player_disengage_timer = 1;
	Player_afterburner_vol = AFTERBURNER_DEFAULT_VOL;
	Player_afterburner_loop_id = -1;
	Player_afterburner_start_time = 0;
}

/**
 * Called when a ship engages the afterburners.
 * This function should only be called once when afterburners first start.  This is
 * to start an appropriate sound effect and do any one-time initializations.
 *
 * @param *objp pointer to the object starting afterburners
 */          
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

	shipp = &Ships[objp->instance];
	Assert( shipp->ship_info_index >= 0 && shipp->ship_info_index < Num_ship_classes );
	sip = &Ship_info[shipp->ship_info_index];
	
	// bail if afterburners are locked
	if (shipp->flags2 & SF2_AFTERBURNER_LOCKED)	{
		return;
	}

	if ( (objp->flags & OF_PLAYER_SHIP) && (objp == Player_obj) ) {
		unsigned int now;
		now = timer_get_milliseconds();

		if ( (now - Player_afterburner_start_time) < 1300 ) {
			snd_play( &Snds[ship_get_sound(objp, SND_ABURN_FAIL)] );
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
	    
	if ( !(sip->flags & SIF_AFTERBURNER) )	{
		return;
	}

	// Check if there is enough afterburner fuel
	if ( (shipp->afterburner_fuel < MIN_AFTERBURNER_FUEL_TO_ENGAGE) && !MULTIPLAYER_CLIENT ) {
		if ( objp == Player_obj ) {
			snd_play( &Snds[ship_get_sound(objp, SND_ABURN_FAIL)] );
		}
		return;
	}

	objp->phys_info.flags |= PF_AFTERBURNER_ON;

	objp->phys_info.afterburner_decay = timestamp(ABURN_DECAY_TIME);

	percent_left = shipp->afterburner_fuel / sip->afterburner_fuel_capacity;

	//Do anim
	model_anim_start_type(shipp, TRIGGER_TYPE_AFTERBURNER, ANIMATION_SUBTYPE_ALL, 1);

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

		snd_play( &Snds[ship_get_sound(objp, SND_ABURN_ENGAGE)], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY );
		joy_ff_afterburn_on();
	} else {
		snd_play_3d( &Snds[ship_get_sound(objp, SND_ABURN_ENGAGE)], &objp->pos, &View_position, objp->radius );
	}
	
	objp->phys_info.flags |= PF_AFTERBURNER_WAIT;
}

/**
 * Update the state of the afterburner fuel remaining for an object using the afterburner.  
 *
 * For the player ship, key_up_time() is called for the afterburner key to
 * detect when afterburners disengage.
 *
 * @param *objp			pointer to the object starting afterburners
 * @param fl_frametime	time in seconds of the last frame
 */
void afterburners_update(object *objp, float fl_frametime)
{
	Assert( objp != NULL );
	Assert( objp->type == OBJ_SHIP );
	Assert( objp->instance >= 0 && objp->instance < MAX_SHIPS );
	
	ship_info *sip;
	ship *shipp;
	static int volume_chg_timer = 1;

	shipp = &Ships[objp->instance];

	Assert( shipp->ship_info_index >= 0 && shipp->ship_info_index < Num_ship_classes );
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
	if(!(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER || (objp == Player_obj)) {
		if ( !(objp->phys_info.flags & PF_AFTERBURNER_ON) ) {
			// Recover afterburner fuel

			if ( shipp->afterburner_fuel < sip->afterburner_fuel_capacity ) {
				float recharge_scale;
				recharge_scale = Energy_levels[shipp->engine_recharge_index] * 2.0f * The_mission.ai_profile->afterburner_recharge_scale[Game_skill_level];
				shipp->afterburner_fuel += (sip->afterburner_recover_rate * fl_frametime * recharge_scale);

				if ( shipp->afterburner_fuel >  sip->afterburner_fuel_capacity){
					shipp->afterburner_fuel = sip->afterburner_fuel_capacity;
				}
			}
			return;
		} else {
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
				Player_afterburner_loop_id = snd_play_looping( &Snds[ship_get_sound(objp, SND_ABURN_LOOP)], 0.0f , -1, -1);
				snd_set_volume(Player_afterburner_loop_id, Player_afterburner_vol);
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

/**
 * Called when a ship disengages the afterburners.
 *
 * @param *objp			pointer to the object starting afterburners
 * @param key_released	OPTIONAL parameter (default value 0) This is only used for the player object, to manage starting/stopping
 */
void afterburners_stop(object *objp, int key_released)
{
	Assert( objp != NULL );
	Assert( objp->instance >= 0 && objp->instance < MAX_SHIPS );
	
	ship_info *sip;
	ship *shipp;

	shipp = &Ships[objp->instance];

	Assert( shipp->ship_info_index >= 0 && shipp->ship_info_index < Num_ship_classes );
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

	//Do anim
	model_anim_start_type(shipp, TRIGGER_TYPE_AFTERBURNER, ANIMATION_SUBTYPE_ALL, -1);

	if ( objp == Player_obj ) {

		if ( !key_released ) {
			snd_play( &Snds[ship_get_sound(objp, SND_ABURN_FAIL)] );
		}

		if ( Player_afterburner_loop_id > -1 )	{
			Player_disengage_timer = timestamp(DISENGAGE_TIME);
		}

		joy_ff_afterburn_off();
	}
}

/**
 * Terminates any looping afterburner sounds.
 * This should only be called when the game decides to stop all looping sounds.
 */
void afterburner_stop_sounds()
{
	if ( Player_afterburner_loop_id != -1 ) {
		snd_stop(Player_afterburner_loop_id);
	}

	Player_afterburner_loop_id = -1;
	Player_disengage_timer = 1;
	Player_afterburner_loop_delay = 0;
}
