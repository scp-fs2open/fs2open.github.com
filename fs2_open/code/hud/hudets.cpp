/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDets.cpp $
 * $Revision: 2.5 $
 * $Date: 2003-09-11 19:04:36 $
 * $Author: argv $
 *
 * C file that contains code to manage and display the Energy Transfer System (ETS)
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2003/08/06 17:50:01  phreak
 * added code to take into account a tertiary reactor pod
 *
 * Revision 2.3  2003/04/29 01:03:23  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.2  2002/12/10 05:43:34  Goober5000
 * Full-fledged ballistic primary support added!  Try it and see! :)
 *
 * Revision 2.1  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 14    10/28/99 11:17p Jefff
 * nudged German "A" on ETS
 * 
 * 13    10/26/99 11:02p Jefff
 * changed german ets labels
 * 
 * 12    9/05/99 11:23p Jimb
 * changed weapon and shield recharge rates for skill levels
 * 
 * 11    9/03/99 2:28p Mikek
 * Slightly increase rate of weapon recharge at Medium and Hard.  Slightly
 * decrease at Easy.
 * 
 * 10    8/01/99 12:39p Dave
 * Added HUD contrast control key (for nebula).
 * 
 * 9     6/10/99 3:43p Dave
 * Do a better job of syncing text colors to HUD gauges.
 * 
 * 8     2/23/99 8:11p Dave
 * Tidied up dogfight mode. Fixed TvT ship type problems for alpha wing.
 * Small pass over todolist items.
 * 
 * 7     1/07/99 9:06a Jasen
 * coords updated
 * 
 * 6     12/28/98 3:17p Dave
 * Support for multiple hud bitmap filenames for hi-res mode.
 * 
 * 5     12/21/98 5:02p Dave
 * Modified all hud elements to be multi-resolution friendly.
 * 
 * 4     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 3     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 60    8/28/98 3:28p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 59    8/25/98 1:48p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 58    6/19/98 3:49p Lawrance
 * localization tweaks
 * 
 * 57    4/07/98 1:53p Lawrance
 * Make energy system matter more.
 * 
 * 56    3/26/98 5:26p John
 * added new paging code. nonfunctional.
 * 
 * 55    2/23/98 6:49p Lawrance
 * Use gr_aabitmap_ex() instead of clipping regions
 * 
 * 54    2/22/98 4:17p John
 * More string externalization classification... 190 left to go!
 * 
 * 53    2/12/98 4:58p Lawrance
 * Change to new flashing method.
 * 
 * 52    1/19/98 11:37p Lawrance
 * Fixing Optimization build warnings
 * 
 * 51    1/05/98 9:38p Lawrance
 * Implement flashing HUD gauges.
 * 
 * 50    1/02/98 9:10p Lawrance
 * Big changes to how colors get set on the HUD.
 * 
 * 49    12/30/97 4:28p Lawrance
 * remove .ani extensions from filenames
 * 
 * 48    12/01/97 12:27a Lawrance
 * redo default alpha color for HUD, make it easy to modify in the future
 * 
 * 47    11/15/97 6:10p Lawrance
 * make ship speed less dependant on engine damage
 * 
 * 46    11/11/97 12:58a Lawrance
 * deal with situation where ship has no shields
 * 
 * 45    11/10/97 2:58p Lawrance
 * fix bug that was preventing engine damage from affecting AI ships
 * 
 * 44    11/09/97 3:25p Lawrance
 * increase default alpha color
 * 
 * 43    11/06/97 10:32a Lawrance
 * brighten up energy management bars
 * 
 * 42    11/05/97 11:19p Lawrance
 * implement new ETS gauge
 * 
 * 41    11/05/97 4:04p Lawrance
 * engine speed not affected on Trainee ONLY for player ship
 * 
 * 40    10/28/97 3:35p Lawrance
 * subsystems will not be affected when playing on lowest skill level
 *
 * $NoKeywords: $
 */


#include "hud/hudets.h"
#include "hud/hud.h"
#include "ship/ship.h"
#include "freespace2/freespace.h"
#include "playerman/player.h"
#include "graphics/2d.h"
#include "io/timer.h"
#include "sound/sound.h"
#include "gamesnd/gamesnd.h"
#include "bmpman/bmpman.h"
#include "weapon/emp.h"
#include "localization/localize.h"
#include "weapon/weapon.h"
#include "globalincs/linklist.h" // _argv[-1] - added this for stuff that's below.
#include "mission/missionlog.h" // _argv[-1] - this too.
#include "cmdline/cmdline.h" // _argv[-1] - yet again.

#define ENERGY_DIVERT_DELTA				0.2f	// percentage of energy transferred in a shield->weapon or weapon->shield energy transfer
#define INTIAL_SHIELD_RECHARGE_INDEX	4		// default shield charge rate (index in Energy_levels[])
#define INTIAL_WEAPON_RECHARGE_INDEX	4		// default weapon charge rate (index in Energy_levels[])
#define INTIAL_ENGINE_RECHARGE_INDEX	4		// default engine charge rate (index in Energy_levels[])

#define MAX_SHIELD_REGEN_PER_SECOND		0.02f	//	max percent/100 of shield energy regenerated per second
#define MAX_WEAPON_REGEN_PER_SECOND		0.04f	// max percent/100 of weapon energy regenerated per second

#define NUM_ENERGY_LEVELS	13		
#define MAX_ENERGY_INDEX	(NUM_ENERGY_LEVELS - 1)
float Energy_levels[NUM_ENERGY_LEVELS] = {0.0f,  0.0833f, 0.167f, 0.25f, 0.333f, 0.417f, 0.5f, 0.583f, 0.667f, 0.75f, 0.833f, 0.9167f, 1.0f};

#define AI_MODIFY_ETS_INTERVAL 500	// time between ets modifications for ai's (in milliseconds)

int Weapon_energy_cheat = 0;

// skill level scaling of the amount of energy that is allocated to the weapons and 
// shields
float	Skill_level_weapon_energy_scale[NUM_SKILL_LEVELS] = {10.0f, 4.0f, 2.5f, 2.0f, 1.5f};
float Skill_level_shield_energy_scale[NUM_SKILL_LEVELS] = {4.0f, 2.0f, 1.5f, 1.25f, 1.0f};

#define ZERO_INDEX			0
#define ONE_THIRD_INDEX		4
#define ONE_HALF_INDEX		6
#define ALL_INDEX				12

#define HAS_ENGINES			(1<<0)
#define HAS_SHIELDS			(1<<1)
#define HAS_WEAPONS			(1<<2)

int ETS_bar_h[GR_NUM_RESOLUTIONS] = {
	41,
	41
};

typedef struct ets_gauge_info
{
	char	letter;
	int	letter_coords[2];
	int	top_coords[2];
	int	bottom_coords[2];
} ets_gauge_info;

ets_gauge_info Ets_gauge_info_german[GR_NUM_RESOLUTIONS][3] =
{
	{ // GR_640
		'G', {525,422}, {523,380}, {523,430},
		'S', {542,422}, {540,380}, {540,430},
		'A', {559,422}, {557,380}, {557,430}
	},
	{ // GR_1024
		'G', {882,690}, {880,648}, {880,698},
		'S', {900,690}, {898,648}, {898,698},
		'A', {917,690}, {916,648}, {916,698}
	}
};
ets_gauge_info Ets_gauge_info_french[GR_NUM_RESOLUTIONS][3] =
{
	{ // GR_640
		'C', {525,422}, {523,380}, {523,430},
		'B', {542,422}, {540,380}, {540,430},
		'M', {560,422}, {557,380}, {557,430}
	}, 
	{ // GR_1024
		'C', {882,690}, {880,648}, {880,698},
		'B', {900,690}, {898,648}, {898,698},
		'M', {918,690}, {916,648}, {916,698}
	},
};
ets_gauge_info Ets_gauge_info_english[GR_NUM_RESOLUTIONS][3] =
{
	{ // GR_640
		'G', {525,422}, {523,380}, {523,430},
		'S', {542,422}, {540,380}, {540,430},
		'E', {560,422}, {557,380}, {557,430}
	},
	{ // GR_1024
		'G', {882,690}, {880,648}, {880,698},
		'S', {900,690}, {898,648}, {898,698},
		'E', {918,690}, {916,648}, {916,698}
	}
};
ets_gauge_info *Ets_gauge_info = NULL;

char Ets_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"energy1",
	"energy1"
};

hud_frames Ets_gauge;

static int Hud_ets_inited = 0;

extern tertiary_weapon_info Tertiary_weapon_info[MAX_TERTIARY_WEAPON_TYPES];

void hud_init_ets()
{
	if ( Hud_ets_inited )
		return;

	Ets_gauge.first_frame = bm_load_animation(Ets_fname[gr_screen.res], &Ets_gauge.num_frames);
	if ( Ets_gauge.first_frame < 0 ) {
		Warning(LOCATION,"Cannot load hud ani: Ets_fname[gr_screen.res]\n");
	}

	if(Lcl_gr){
		Ets_gauge_info = Ets_gauge_info_german[gr_screen.res];
	} else if(Lcl_fr){
		Ets_gauge_info = Ets_gauge_info_french[gr_screen.res];
	} else {
		Ets_gauge_info = Ets_gauge_info_english[gr_screen.res];
	}
	
	Hud_ets_inited = 1;
}

// -------------------------------------------------------------------------------------------------
// ets_init_ship() is called by a ship when it is created (effectively, for every ship at the start
// of a mission).  This will set the default charge rates for the different systems and initalialize
// the weapon energy reserve.
//
void ets_init_ship(object* obj)
{
	ship* sp;

	// fred should bail here
	if(Fred_running){
		return;
	}

	Assert(obj->type == OBJ_SHIP);
	sp = &Ships[obj->instance];
	
	sp->weapon_energy = Ship_info[sp->ship_info_index].max_weapon_reserve;
	sp->next_manage_ets = timestamp(AI_MODIFY_ETS_INTERVAL);
	set_default_recharge_rates(obj);
}

// -------------------------------------------------------------------------------------------------
// update_ets() is called once per frame for every OBJ_SHIP in the game.  The amount of energy
// to send to the weapons and shields is calculated, and the top ship speed is calculated.  The
// amount of time elapsed from the previous call is passed in as the parameter fl_frametime.
//
// parameters:   obj          ==> object that is updating their energy system
//               fl_frametime ==> game frametime (in seconds)
//
void update_ets(object* objp, float fl_frametime)
{
	float /*max_new_shield_energy, max_new_weapon_energy,*/ _ss;

	if ( fl_frametime <= 0 ){
		return;
	}

	ship* ship_p = &Ships[objp->instance];
	ship_info* sinfo_p = &Ship_info[ship_p->ship_info_index];
	float max_g=sinfo_p->max_weapon_reserve,
		  max_s=ship_p->ship_initial_shield_strength;

	if ( ship_p->flags & SF_DYING ){
		return;
	}

	if ( sinfo_p->full_power_output == 0 ){
		// _argv[-1] - ship might have 0 PO but some subsystems with some power output.
		// If not, then energy management should obviously be disabled.
		return;
	}

	if (ship_p->tertiary_weapon_info_idx >=0)
	{
		if (Tertiary_weapon_info[ship_p->tertiary_weapon_info_idx].type == TWT_EXTRA_REACTOR)
		{
			max_g+=Tertiary_weapon_info[ship_p->tertiary_weapon_info_idx].reactor_add_weap_pwr;
			max_s+=Tertiary_weapon_info[ship_p->tertiary_weapon_info_idx].reactor_add_shield_pwr;
		}
	}

//	new_energy = fl_frametime * sinfo_p->power_output;

	// _argv[-1] - made power output part of the equation again.

	if (ship_p->power_drain > 0.0f) {
		ship_p->time_to_next_jitter -= fl_frametime;
		while (ship_p->time_to_next_jitter <= 0.0f) {
			ship_p->effective_power_drain -= sinfo_p->full_power_output * 0.1f;
			if (ship_p->effective_power_drain < 0.0f)
				ship_p->effective_power_drain = 0.0f;
			ship_p->time_to_next_jitter += 3.0f;
		}
	}
	else
		ship_p->time_to_next_jitter = 0.0f;

	if (ship_p->effective_power_drain != ship_p->power_drain) {
		if (ship_p->effective_power_drain < ship_p->power_drain) {
			ship_p->effective_power_drain += max(sinfo_p->full_power_output, -ship_p->effective_power_drain) * 0.2f * fl_frametime;

			if (ship_p->effective_power_drain > ship_p->power_drain)
				// clamp
				ship_p->effective_power_drain = ship_p->power_drain;
		}
		else /* if (ship_p->effective_power_drain > ship_p->power_drain) */ {
			ship_p->effective_power_drain -= max(sinfo_p->full_power_output, ship_p->effective_power_drain) * 0.2f * fl_frametime;

			if (ship_p->effective_power_drain < ship_p->power_drain)
				// clamp
				ship_p->effective_power_drain = ship_p->power_drain;
		}
	}

	// calculate effective power output.
	ship_p->power_output = sinfo_p->power_output;
	
	for (ship_subsys *ss = GET_FIRST(&ship_p->subsys_list); ss != END_OF_LIST(&ship_p->subsys_list); ss = GET_NEXT(ss)) {
		if (!ss->system_info->power_output) {
			ss->power_output = 0.0f;
			continue;
		}
		else if (ss->current_hits == 0.0f || ship_subsys_disrupted(ss)) {
			ss->power_output = 0.0f;
			continue;
		}
		else if (ss->current_hits < ss->system_info->max_subsys_strength / 2)
			ss->power_output = ss->system_info->power_output * (ss->current_hits / (ss->system_info->max_subsys_strength / 2));
		else
			ss->power_output = ss->system_info->power_output;

		ship_p->power_output += ss->power_output;
	}

	// apply power drain.
	if (ship_p->effective_power_drain != 0.0f)
		ship_p->power_output -= ship_p->effective_power_drain;
	if (ship_p->power_output < 0.0f)
		ship_p->power_output = 0.0f;

	// update weapon energy
	ship_p->weapon_energy += fl_frametime * ship_p->power_output * Energy_levels[ship_p->weapon_recharge_index] * (objp->flags & OF_PLAYER_SHIP ? Skill_level_weapon_energy_scale[Game_skill_level] : 1);

	// apply power drain (real, not effective).
	if (ship_p->power_drain != 0.0f) {
		ship_p->weapon_energy -= fl_frametime * ship_p->power_drain;

		if (ship_p->weapon_energy < 0.0f)
			// clamp
			ship_p->weapon_energy = 0.0f;
	}

	/*
	max_new_weapon_energy = fl_frametime * MAX_WEAPON_REGEN_PER_SECOND * max_g;
	if ( objp->flags & OF_PLAYER_SHIP ) {
		ship_p->weapon_energy += Energy_levels[ship_p->weapon_recharge_index] * max_new_weapon_energy * Skill_level_weapon_energy_scale[Game_skill_level];
	} else {
		ship_p->weapon_energy += Energy_levels[ship_p->weapon_recharge_index] * max_new_weapon_energy;
	}
	*/

	if ( ship_p->weapon_energy > sinfo_p->max_weapon_reserve ){
		ship_p->weapon_energy = sinfo_p->max_weapon_reserve;
	}

	float shield_delta;
	shield_delta = fl_frametime * 16.0f * ((ship_p->power_output * Energy_levels[ship_p->shield_recharge_index] * (objp->flags & OF_PLAYER_SHIP ? Skill_level_shield_energy_scale[Game_skill_level] : 1)) - ship_p->power_drain);
	/*
	max_new_shield_energy = fl_frametime * MAX_SHIELD_REGEN_PER_SECOND * max_s;
	if ( objp->flags & OF_PLAYER_SHIP ) {
		shield_delta = Energy_levels[ship_p->shield_recharge_index] * max_new_shield_energy * Skill_level_shield_energy_scale[Game_skill_level];
	} else {
		shield_delta = Energy_levels[ship_p->shield_recharge_index] * max_new_shield_energy;
	}
	*/

	add_shield_strength(objp, shield_delta);

	if ( (_ss = get_shield_strength(objp)) > ship_p->ship_initial_shield_strength ){
		for (int i=0; i<MAX_SHIELD_SECTIONS; i++){
			objp->shield_quadrant[i] *= ship_p->ship_initial_shield_strength / _ss;
		}
	}

	// easy out for disrupted engines.
	if (ship_p->subsys_disrupted_flags & (1<<SUBSYSTEM_ENGINE))
		ship_p->current_max_speed = 0.0f;
	// easy out for stationary objects.
	else if (sinfo_p->max_speed == 0.0f)
		ship_p->flags &= ~SF_DISABLED; // stationary objects have no engine and thus cannot be disabled.
	// calculate the real max speed of this ship, and set disabled flag as appropriate.
	// FIXME: affect max speeds of lateral thrusters.
	else {
		float strength = ship_get_subsystem_strength(ship_p, SUBSYSTEM_ENGINE);
		// calculate the top speed of the ship based on the energy flow to engines
		float y = Energy_levels[ship_p->engine_recharge_index];

		// check for a shortcuts first before doing linear interpolation
		if (strength == 0.0f)
			ship_p->current_max_speed;
		else if ( y == Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX] ){
			ship_p->current_max_speed = sinfo_p->max_speed;
		} else if ( y == 0.0f ){
			ship_p->current_max_speed = 0.5f * sinfo_p->max_speed;
		} else if ( y == 1.0f ){
			ship_p->current_max_speed = sinfo_p->max_overclocked_speed;
		} else {
			// do a linear interpolation to find the current max speed, using points (0,1/2 default_max_speed) (.333,default_max_speed)
			// x = x1 + (y-y1) * (x2-x1) / (y2-y1);
			if ( y < Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX] ){
				ship_p->current_max_speed =  0.5f*sinfo_p->max_speed + (y  * (0.5f*sinfo_p->max_speed) ) / Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX];
			} else {
				// do a linear interpolation to find the current max speed, using points (.333,default_max_speed) (1,max_overclock_speed)
				ship_p->current_max_speed = sinfo_p->max_speed + (y - Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX]) * (sinfo_p->max_overclocked_speed - sinfo_p->max_speed) / (1.0f - Energy_levels[INTIAL_ENGINE_RECHARGE_INDEX]);
			}
		}

		// _argv[-1] - if reduced power output, make it affect engines.
		if (ship_p->power_output < sinfo_p->full_power_output)
			ship_p->current_max_speed *= ship_p->power_output / sinfo_p->full_power_output;

		// AL 11-15-97: Rules for engine strength affecting max speed:
		//						1. if strength >= 0.5 no affect 
		//						2. if strength < 0.5 then max_speed = sqrt(strength)
		//					 
		//					 This will translate to 71% max speed at 50% engines, and 31% max speed at 10% engines
		//

		// don't let engine strength affect max speed when playing on lowest skill level
		if ( (objp != Player_obj) || (Game_skill_level > 0) ) {
			if ( strength < 0.5 ) {
				ship_p->current_max_speed *= fl_sqrt(strength);
			}
		}

		objp->phys_info.max_speed_mul = ship_p->current_max_speed / sinfo_p->max_speed;

		// set disabled flag based on max speed, as max speed is always 0 if all engines are shot.
		if (ship_p->current_max_speed == 0.0f) {
			if (!(ship_p->flags & SF_DISABLED)) // if disabling, add log entry.
				mission_log_add_entry(LOG_SHIP_DISABLED, ship_p->ship_name, NULL);
			ship_p->flags |= SF_DISABLED;
		}
		else {
			if (ship_p->flags & SF_DISABLED) { // if undisabling, reset disabled physics too.
				extern void ship_reset_disabled_physics(object *objp, int ship_class);
				ship_reset_disabled_physics(objp, ship_p->ship_info_index);
			}
			ship_p->flags &= ~SF_DISABLED;
		}
	}

	// _argv[-1] - automatic energy management!
	/*if ( timestamp_elapsed(ship_p->next_manage_ets) ) {
		if ( !(objp->flags & OF_PLAYER_SHIP) ) {*/
	if (objp->flags & OF_PLAYER_SHIP && Weapon_energy_cheat)
		ship_p->weapon_energy = sinfo_p->max_weapon_reserve;
	// _argv[-1] - FIXME: this probably won't work properly in multiplayer. Maybe it'll only apply auto ETS to the master?
	if (!(objp->flags & OF_PLAYER_SHIP) || Argv_options.auto_ets)
		ai_manage_ets(objp);
	/*		ship_p->next_manage_ets = timestamp(AI_MODIFY_ETS_INTERVAL);
		}
		else {
			if ( Weapon_energy_cheat ){
				ship_p->weapon_energy = sinfo_p->max_weapon_reserve;
			}
			ai_manage_ets(objp);
		}
	}*/
}


// -------------------------------------------------------------------------------------------------
// ai_manage_ets() will determine if a ship should modify it's energy transfer percentages, or 
// transfer energy from shields->weapons or from weapons->shields
//

// minimum level rule constants
#define SHIELDS_MIN_LEVEL_PERCENT	0.3f
#define WEAPONS_MIN_LEVEL_PERCENT	0.3f

// maximum level rule constants
#define SHIELDS_MAX_LEVEL_PERCENT	0.8f
#define WEAPONS_MAX_LEVEL_PERCENT	0.8f

// emergency rule constants
#define SHIELDS_EMERG_LEVEL_PERCENT	0.25f
#define WEAPONS_EMERG_LEVEL_PERCENT	0.05f

// need this, or ai's tend to totally eliminate engine power!
// _argv[-1] - reduced from 3. give engines low prior
#define MIN_ENGINE_RECHARGE_INDEX	1

#define DEFAULT_CHARGE_INDEX			4
#define NORMAL_TOLERANCE_PERCENT		.10f

void ai_manage_ets(object* obj)
{
	ship* ship_p = &Ships[obj->instance];
	ship_info* ship_info_p = &Ship_info[ship_p->ship_info_index];
	// _argv[-1] - added this no_shields flag.
	int no_shields = obj->flags & OF_NO_SHIELDS || !ship_p->ship_initial_shield_strength;

	if ( ship_info_p->full_power_output == 0 ) // _argv[-1] - ship might have 0 PO but some subsystems with some power output.
		return;

	if (ship_p->flags & SF_DYING)
		return;

	// check if any of the three systems are not being used.  If so, don't allow energy management.
	// _argv[-1] - changed this so that absent shields will still allow changes to weapon/engine energy.
	if ( /* !ship_p->ship_initial_shield_strength || */ !ship_info_p->max_speed || !ship_info_p->max_weapon_reserve)
		return;

	float shield_left_percent = (get_shield_strength(obj)+ship_p->target_shields_delta)/ship_p->ship_initial_shield_strength;
	float weapon_left_percent = (ship_p->weapon_energy+ship_p->target_weapon_energy_delta)/ship_info_p->max_weapon_reserve;

	// maximum level check
	//	MK, changed these, might as well let them go up to 100% if nothing else needs the recharge ability.
	// _argv[-1] - redid all of this.

	// if AB reserve needs filling, fill it! even if shield is low, this may be needed for escape.

	if (ship_p->afterburner_fuel < ship_info_p->afterburner_fuel_capacity && !(obj->phys_info.flags & PF_AFTERBURNER_ON))
		increase_recharge_rate(obj, ENGINES_QUIET);
	else {
		// establish how much power is needed in shields and weapons.

		int shield_target, weapon_target, engine_off = ((obj->phys_info.forward_thrust == 0 || obj->phys_info.flags & PF_AFTERBURNER_ON) && obj->phys_info.side_thrust == 0 && obj->phys_info.vert_thrust == 0);

		if (no_shields || shield_left_percent == 1.0f)
			shield_target = 0;
		else {
			if (engine_off)
				// engine is off, so take all power.
				shield_target = ALL_INDEX;
			else
				shield_target = max((int) ((1.0f - shield_left_percent) * ALL_INDEX), 2) + 2;
			if (shield_target > ALL_INDEX)
				shield_target = ALL_INDEX;
		}
		if (weapon_left_percent == 1.0f)
			weapon_target = 0;
		else if (engine_off && shield_target < ALL_INDEX)
			// engine is off, so take remaining power.
			weapon_target = ALL_INDEX - shield_target;
		else
			weapon_target = max((int) ((1.0f - weapon_left_percent) * ALL_INDEX), 2);

		// then, allocate power to weapons.

		while (ship_p->weapon_recharge_index > weapon_target) {
			ship_p->weapon_recharge_index--;
			ship_p->engine_recharge_index++;
		}
		while (ship_p->weapon_recharge_index < weapon_target) {
			if (!no_shields && ship_p->shield_recharge_index > shield_target) {
				// swipe excess shield power (would otherwise go to engine, below).
				ship_p->shield_recharge_index--;
				ship_p->weapon_recharge_index++;
			}
			else if ((engine_off && ship_p->engine_recharge_index > 0) || ship_p->engine_recharge_index > 2) {
				// swipe non-critical engine power.
				ship_p->engine_recharge_index--;
				ship_p->weapon_recharge_index++;
			}
			else
				// no available power! ship has problems!
				break;
		}

		// then, allocate power to shields.
		// note that this must come after weapons because the shield may steal weapon energy.

		if (!no_shields) {
			while (ship_p->shield_recharge_index > shield_target) {
				ship_p->shield_recharge_index--;
				ship_p->engine_recharge_index++;
			}
			while (ship_p->shield_recharge_index < shield_target) {
				if ((engine_off && ship_p->engine_recharge_index > 0) || ship_p->engine_recharge_index > 2) {
					// swipe non-critical engine power.
					ship_p->engine_recharge_index--;
					ship_p->shield_recharge_index++;
				}
				else if (shield_left_percent < SHIELDS_EMERG_LEVEL_PERCENT && ship_p->weapon_recharge_index > 1) {
					// swipe weapon power if shields are low.
					ship_p->weapon_recharge_index--;
					ship_p->shield_recharge_index++;
				}
				else
					// no available power! ship has problems!
					break;
			}
		}
		else if (ship_p->shield_recharge_index > 0) {
			// ship has no shields, but power is going to shields, so divert it all to engines. thought impossible, but it's not.
			do {
				ship_p->shield_recharge_index--;
				ship_p->engine_recharge_index++;
			} while (ship_p->shield_recharge_index > 0);
		}
	}

	////

	/*
	if (weapon_left_percent == 1.0f && (no_shields || shield_left_percent == 1.0f))
		increase_recharge_rate(obj, ENGINES_QUIET);
	else {
		if (!no_shields) {
			if (shield_left_percent == 1.0f)
				decrease_recharge_rate(obj, SHIELDS_QUIET);
			else {
				int target = max((int) ((1.0f - shield_left_percent) * ALL_INDEX), 1);
				if (ship_p->shield_recharge_index < max((int) ((1.0f - shield_left_percent) * ALL_INDEX), 1))
				increase_recharge_rate(obj, SHIELDS_QUIET);
			}
		}

		if ( weapon_left_percent == 1.0f) {
			if (ship_p->weapon_recharge_index > 0) {
				// try to find another system to transfer to.
				if (ship_p->shield_recharge_index < ALL_INDEX && shield_left_percent < 1.0f && ship_p->engine_recharge_index >= 4) {
					ship_p->weapon_recharge_index--;
					ship_p->shield_recharge_index++;
				}
				else if (ship_p->engine_recharge_index < ALL_INDEX) {
					ship_p->weapon_recharge_index--;
					ship_p->shield_recharge_index++;
				}
			}
		}
		else if (ship_p->weapon_recharge_index < max((int) ((1.0f - weapon_left_percent) * ALL_INDEX), 1) && ship_p->engine_recharge_index > 1 && ship_p->weapon_recharge_index < ALL_INDEX) {
			// don't step on shields!
			ship_p->engine_recharge_index--;
			ship_p->weapon_recharge_index++;
		}
	}
	*/

	// minimum check

	/*
	if (!no_shields && (shield_left_percent < SHIELDS_MIN_LEVEL_PERCENT)) {
		if ( weapon_left_percent > WEAPONS_MIN_LEVEL_PERCENT )
			increase_recharge_rate(obj, SHIELDS_QUIET);
	}

	// _argv[-1] - made this 'else' to give shields priority over weapons and engines.
	else if ( weapon_left_percent < WEAPONS_MIN_LEVEL_PERCENT ) {
		increase_recharge_rate(obj, WEAPONS_QUIET);
	}

	else if ( ship_p->engine_recharge_index < MIN_ENGINE_RECHARGE_INDEX ) {
		increase_recharge_rate(obj, ENGINES_QUIET);
	}*/

	// emergency check
	if (!no_shields) {
		if ( shield_left_percent < SHIELDS_EMERG_LEVEL_PERCENT ) {
			if (ship_p->target_shields_delta == 0.0f && weapon_left_percent > WEAPONS_EMERG_LEVEL_PERCENT) {
				transfer_energy_to_shields(obj);
				//increase_recharge_rate(obj, SHIELDS_QUIET);
			}
		} else if ( weapon_left_percent < WEAPONS_EMERG_LEVEL_PERCENT ) {
			if (ship_p->target_weapon_energy_delta == 0.0f && shield_left_percent > SHIELDS_MIN_LEVEL_PERCENT /*|| weapon_left_percent <= 0.01*/ ) {	// dampen ai enthusiasm for sucking energy to weapons
				transfer_energy_to_weapons(obj);
				//increase_recharge_rate(obj, WEAPONS_QUIET);
			}
		}

	
		// check for return to normal values
		/*
		if ( fl_abs( shield_left_percent - 0.5f ) < NORMAL_TOLERANCE_PERCENT ) {
			if ( ship_p->shield_recharge_index > DEFAULT_CHARGE_INDEX )
				decrease_recharge_rate(obj, SHIELDS_QUIET);
			else if ( ship_p->shield_recharge_index < DEFAULT_CHARGE_INDEX )
				increase_recharge_rate(obj, SHIELDS_QUIET);
		}
		*/
	}


	/*
	if ( fl_abs( weapon_left_percent - 0.5f ) < NORMAL_TOLERANCE_PERCENT ) {
		if ( ship_p->weapon_recharge_index > DEFAULT_CHARGE_INDEX )
			decrease_recharge_rate(obj, WEAPONS_QUIET);
		else if ( ship_p->weapon_recharge_index < DEFAULT_CHARGE_INDEX )
			increase_recharge_rate(obj, WEAPONS_QUIET);
	}
	*/
}

// -------------------------------------------------------------------------------------------------
// hud_show_ets() will display the charge rates for the three systems, and the reserve
// energy for shields and weapons.  hud_show_ets() is called once per frame.
//
void hud_show_ets()
{
	int i, j, index, y_start, y_end, clip_h, w, h, x, y;

	ship* ship_p = &Ships[Player_obj->instance];	

	if ( Ets_gauge.first_frame < 0 ) {
		return;
	}

	// if at least two gauges are not shown, don't show any
	i = 0;
	if (Player_obj->flags & OF_NO_LASERS) i++;
	if (Player_obj->flags & OF_NO_SHIELDS) i++;
	if (Player_obj->flags & OF_NO_ENGINES) i++;
	if (i >= 2) return;

	hud_set_gauge_color(HUD_ETS_GAUGE);

	// draw the letters for the gauges first, before any clipping occurs
	i = 0;
	for ( j = 0; j < 3; j++ )
	{
		if (j == 0 && Player_obj->flags & OF_NO_LASERS)
		{
			continue;
		}
		if (j == 1 && Player_obj->flags & OF_NO_SHIELDS)
		{
			continue;
		}
		if (j == 2 && Player_obj->flags & OF_NO_ENGINES)
		{
			continue;
		}
		Assert(Ets_gauge_info != NULL);
		gr_printf(Ets_gauge_info[i].letter_coords[0], Ets_gauge_info[i].letter_coords[1], NOX("%c"), Ets_gauge_info[j].letter); 
		i++;
	}

	// draw the three energy gauges
	i = 0;
	index = 0;
	for ( j = 0; j < 3; j++ ) {
		switch (j) {
		case 0:
			index = ship_p->weapon_recharge_index;
			if ( Player_obj->flags & OF_NO_LASERS )
			{
				continue;
			}
			break;
		case 1:
			index = ship_p->shield_recharge_index;
			if ( Player_obj->flags & OF_NO_SHIELDS )
			{
				continue;
			}
			break;
		case 2:
			index = ship_p->engine_recharge_index;
			if ( Player_obj->flags & OF_NO_ENGINES )
			{
				continue;
			}
			break;
		}

		clip_h = fl2i( (1 - Energy_levels[index]) * ETS_bar_h[gr_screen.res] );

		bm_get_info(Ets_gauge.first_frame,&w,&h);

		if ( index < NUM_ENERGY_LEVELS-1 ) {
			// some portion of dark needs to be drawn

			hud_set_gauge_color(HUD_ETS_GAUGE);

			// draw the top portion

			Assert(Ets_gauge_info != NULL);
			x = Ets_gauge_info[i].top_coords[0];
			y = Ets_gauge_info[i].top_coords[1];
			
			GR_AABITMAP_EX(Ets_gauge.first_frame,x,y,w,clip_h,0,0);			

			// draw the bottom portion
			Assert(Ets_gauge_info != NULL);
			x = Ets_gauge_info[i].bottom_coords[0];
			y = Ets_gauge_info[i].bottom_coords[1];

			y_start = y + (ETS_bar_h[gr_screen.res] - clip_h);
			y_end = y + ETS_bar_h[gr_screen.res];
			
			GR_AABITMAP_EX(Ets_gauge.first_frame, x, y_start, w, y_end-y_start, 0, ETS_bar_h[gr_screen.res]-clip_h);			
		}

		if ( index > 0 ) {
			if ( hud_gauge_maybe_flash(HUD_ETS_GAUGE) == 1 ) {
				hud_set_gauge_color(HUD_ETS_GAUGE, HUD_C_DIM);
				// hud_set_dim_color();
			} else {
				hud_set_gauge_color(HUD_ETS_GAUGE, HUD_C_BRIGHT);
				// hud_set_bright_color();
			}
			// some portion of recharge needs to be drawn

			// draw the top portion
			Assert(Ets_gauge_info != NULL);
			x = Ets_gauge_info[i].top_coords[0];
			y = Ets_gauge_info[i].top_coords[1];

			y_start = y + clip_h;
			y_end = y + ETS_bar_h[gr_screen.res];
			
			GR_AABITMAP_EX(Ets_gauge.first_frame+1, x, y_start, w, y_end-y_start, 0, clip_h);			

			// draw the bottom portion
			Assert(Ets_gauge_info != NULL);
			x = Ets_gauge_info[i].bottom_coords[0];
			y = Ets_gauge_info[i].bottom_coords[1];
			
			GR_AABITMAP_EX(Ets_gauge.first_frame+2, x,y,w,ETS_bar_h[gr_screen.res]-clip_h,0,0);			
		}
		i++;
	}

	// hud_set_default_color();
}

// -------------------------------------------------------------------------------------------------
// set_default_recharge_rates() will set the charge levels for the weapons, shields and
// engines to their default levels
void set_default_recharge_rates(object* obj)
{
	int ship_properties;

	ship* ship_p = &Ships[obj->instance];
	ship_info* ship_info_p = &Ship_info[ship_p->ship_info_index];

	if ( ship_info_p->full_power_output == 0 ) // _argv[-1] - ship might have 0 PO but some subsystems with some power output.
		return;

	ship_properties = 0;	
	if (!(obj->flags & OF_NO_LASERS))
		ship_properties |= HAS_WEAPONS;
	
	if (!(obj->flags & OF_NO_SHIELDS))
		ship_properties |= HAS_SHIELDS;

	if (!(obj->flags & OF_NO_ENGINES))
		ship_properties |= HAS_ENGINES;

	// the default charge rate depends on what systems are on each ship
	switch ( ship_properties ) {
		case HAS_ENGINES | HAS_WEAPONS | HAS_SHIELDS:
			ship_p->shield_recharge_index = INTIAL_SHIELD_RECHARGE_INDEX;
			ship_p->weapon_recharge_index = INTIAL_WEAPON_RECHARGE_INDEX;
			ship_p->engine_recharge_index = INTIAL_ENGINE_RECHARGE_INDEX;
			break;

		case HAS_ENGINES | HAS_SHIELDS:
			ship_p->shield_recharge_index = ONE_HALF_INDEX;
			ship_p->weapon_recharge_index = ZERO_INDEX;
			ship_p->engine_recharge_index = ONE_HALF_INDEX;
			break;

		case HAS_WEAPONS | HAS_SHIELDS:
			ship_p->shield_recharge_index = ONE_HALF_INDEX;
			ship_p->weapon_recharge_index = ONE_HALF_INDEX;
			ship_p->engine_recharge_index = ZERO_INDEX;
			break;

		case HAS_ENGINES | HAS_WEAPONS:
			ship_p->shield_recharge_index = ZERO_INDEX;
			ship_p->weapon_recharge_index = ONE_HALF_INDEX;
			ship_p->engine_recharge_index = ONE_HALF_INDEX;
			break;

		case HAS_SHIELDS:
			ship_p->shield_recharge_index = ALL_INDEX;
			ship_p->weapon_recharge_index = ZERO_INDEX;
			ship_p->engine_recharge_index = ZERO_INDEX;
			break;

		case HAS_ENGINES:
			ship_p->shield_recharge_index = ZERO_INDEX;
			ship_p->weapon_recharge_index = ZERO_INDEX;
			ship_p->engine_recharge_index = ALL_INDEX;
			break;

		case HAS_WEAPONS:
			ship_p->shield_recharge_index = ZERO_INDEX;
			ship_p->weapon_recharge_index = ALL_INDEX;
			ship_p->engine_recharge_index = ZERO_INDEX;
			break;

		default:
			Int3();	// if no systems, power output should be zero, and this funtion shouldn't be called
			break;
	} // end switch
}

// -------------------------------------------------------------------------------------------------
// increase_recharge_rate() will increase the energy flow to the specified system (one of
// WEAPONS, SHIELDS or ENGINES).  The increase in energy will result in a decrease to
// the other two systems.
void increase_recharge_rate(object* obj, SYSTEM_TYPE ship_system) 
{
	int	*gain_index=NULL, *lose_index1=NULL, *lose_index2=NULL, *tmp=NULL;
	int	count=0;
	ship	*ship_p = &Ships[obj->instance];

	switch ( ship_system ) {
		case WEAPONS:
		case WEAPONS_QUIET:
			if ( obj->flags & OF_NO_LASERS )
				return;

			gain_index = &ship_p->weapon_recharge_index;

			if ( obj->flags & OF_NO_SHIELDS )
				lose_index1 = NULL;
			else
				lose_index1 = &ship_p->shield_recharge_index;

			if ( obj->flags & OF_NO_ENGINES )
				lose_index2 = NULL;
			else
				lose_index2 = &ship_p->engine_recharge_index;

			break;

		case SHIELDS:
		case SHIELDS_QUIET:
			if ( obj->flags & OF_NO_SHIELDS )
				return;

			gain_index = &ship_p->shield_recharge_index;

			if ( obj->flags & OF_NO_LASERS )
				lose_index1 = NULL;
			else
				lose_index1 = &ship_p->weapon_recharge_index;

			if ( obj->flags & OF_NO_ENGINES )
				lose_index2 = NULL;
			else
				lose_index2 = &ship_p->engine_recharge_index;

			break;

		case ENGINES:
		case ENGINES_QUIET:
			if ( obj->flags & OF_NO_ENGINES )
				return;

			gain_index = &ship_p->engine_recharge_index;

			if ( obj->flags & OF_NO_LASERS )
				lose_index1 = NULL;
			else
				lose_index1 = &ship_p->weapon_recharge_index;

			if ( obj->flags & OF_NO_SHIELDS )
				lose_index2 = NULL;
			else
				lose_index2 = &ship_p->shield_recharge_index;

			break;

	} // end switch

	// return if we can't transfer energy
	if (!lose_index1 && !lose_index2)
		return;

	// already full, nothing to do 
	count = MAX_ENERGY_INDEX - *gain_index;
	if ( count > 2 ) 
		count = 2;

	if ( count <= 0 )
	{
		if ( obj == Player_obj && ship_system != WEAPONS_QUIET && ship_system != SHIELDS_QUIET && ship_system != ENGINES_QUIET )
		{
			snd_play( &Snds[SND_ENERGY_TRANS_FAIL], 0.0f );
		}
		return;
	}

	*gain_index += count;

	// ensure that the highest lose index takes the first decrease
	if ( lose_index1 && lose_index2 ) {
		if ( *lose_index1 < *lose_index2 ) {
			tmp = lose_index1;
			lose_index1 = lose_index2;
			lose_index2 = tmp;
		}
	}

	int sanity = 0;
	while(count > 0) {
		if ( lose_index1 && *lose_index1 > 0 ) {
			*lose_index1 -= 1;
			count--;
		}

		if ( count <= 0 ) 
			break;

		if ( lose_index2 && *lose_index2 > 0 ) {
			*lose_index2 -= 1;
			count--;
		}

		if ( sanity++ > 10 ) {
			Int3();	// get Alan
			break;
		}
	}

	if ( obj == Player_obj && ship_system != WEAPONS_QUIET && ship_system != SHIELDS_QUIET && ship_system != ENGINES_QUIET )
		snd_play( &Snds[SND_ENERGY_TRANS], 0.0f );
}

// -------------------------------------------------------------------------------------------------
// decrease_recharge_rate() will decrease the energy flow to the specified system (one of
// WEAPONS, SHIELDS or ENGINES).  The decrease in energy will result in an increase to
// the other two systems.
void decrease_recharge_rate(object* obj, SYSTEM_TYPE ship_system) 
{
	int	*lose_index=NULL, *gain_index1=NULL, *gain_index2=NULL, *tmp=NULL;
	int	count;
	ship	*ship_p = &Ships[obj->instance];

	switch ( ship_system ) {
		case WEAPONS:
		case WEAPONS_QUIET:
			if ( obj->flags & OF_NO_LASERS )
				return;

			lose_index = &ship_p->weapon_recharge_index;

			if ( obj->flags & OF_NO_SHIELDS )
				gain_index1 = NULL;
			else
				gain_index1 = &ship_p->shield_recharge_index;

			if ( obj->flags & OF_NO_ENGINES )
				gain_index2 = NULL;
			else
				gain_index2 = &ship_p->engine_recharge_index;

			break;

		case SHIELDS:
		case SHIELDS_QUIET:
			if ( obj->flags & OF_NO_SHIELDS )
				return;

			lose_index = &ship_p->shield_recharge_index;

			if ( obj->flags & OF_NO_LASERS )
				gain_index1 = NULL;
			else
				gain_index1 = &ship_p->weapon_recharge_index;

			if ( obj->flags & OF_NO_ENGINES )
				gain_index2 = NULL;
			else
				gain_index2 = &ship_p->engine_recharge_index;

			break;

		case ENGINES:
		case ENGINES_QUIET:
			if ( obj->flags & OF_NO_ENGINES )
				return;

			lose_index = &ship_p->engine_recharge_index;

			if ( obj->flags & OF_NO_LASERS )
				gain_index1 = NULL;
			else
				gain_index1 = &ship_p->weapon_recharge_index;

			if ( obj->flags & OF_NO_SHIELDS )
				gain_index2 = NULL;
			else
				gain_index2 = &ship_p->shield_recharge_index;

			break;

	} // end switch

	// return if we can't transfer energy
	if (!gain_index1 && !gain_index2)
		return;

	// check how much there is to lose
	count = min(2, *lose_index);
	if ( count <= 0 ) {
		if ( obj == Player_obj && ship_system != WEAPONS_QUIET && ship_system != SHIELDS_QUIET && ship_system != ENGINES_QUIET ) {
			snd_play( &Snds[SND_ENERGY_TRANS_FAIL], 0.0f );
		}
		return;
	}

	*lose_index -= count;

	// make sure that the gain starts with the system which needs it most
	if ( gain_index1 && gain_index2 ) {
		if ( *gain_index1 > *gain_index2 ) {
			tmp = gain_index1;
			gain_index1 = gain_index2;
			gain_index2 = tmp;
		}
	}

	int sanity=0;
	while(count > 0) {
		if ( gain_index1 && *gain_index1 < MAX_ENERGY_INDEX ) {
			*gain_index1 += 1;
			count--;
		}

		if ( count <= 0 ) 
			break;

		if ( gain_index2 && *gain_index2 < MAX_ENERGY_INDEX ) {
			*gain_index2 += 1;
			count--;
		}

		if ( sanity++ > 10 ) {
			Int3();	// get Alan
			break;
		}
	}

	if ( obj == Player_obj && ship_system != WEAPONS_QUIET && ship_system != SHIELDS_QUIET && ship_system != ENGINES_QUIET )
		snd_play( &Snds[SND_ENERGY_TRANS], 0.0f );
}

void transfer_energy_weapon_common(object *objp, float from_field, float to_field, float *from_delta, float *to_delta, float max, float scale)
{
	float	delta;

	delta = from_field * ENERGY_DIVERT_DELTA * scale;

	if (to_field + *to_delta + delta > max)
		delta = max - to_field - *to_delta;

	if ( delta > 0 ) {
		if ( objp == Player_obj )
			snd_play( &Snds[SND_ENERGY_TRANS], 0.0f );

		if (delta > from_field)
			delta = from_field;

		*to_delta += delta;
		*from_delta -= delta;
	} else
		if ( objp == Player_obj )
			snd_play( &Snds[SND_ENERGY_TRANS_FAIL], 0.0f );
}

// -------------------------------------------------------------------------------------------------
// transfer_energy_to_shields() will transfer ENERGY_DIVERT_DELTA percent of weapon energy
// to shield energy.
void transfer_energy_to_shields(object* obj)
{
	ship*			ship_p = &Ships[obj->instance];

	if (ship_p->flags & SF_DYING)
		return;

	if ( obj->flags & OF_NO_LASERS || obj->flags & OF_NO_SHIELDS )
	{
		return;
	}

	transfer_energy_weapon_common(obj, ship_p->weapon_energy, get_shield_strength(obj), &ship_p->target_weapon_energy_delta, &ship_p->target_shields_delta, ship_p->ship_initial_shield_strength, 0.5f);
}

// -------------------------------------------------------------------------------------------------
// transfer_energy_to_weapons() will transfer ENERGY_DIVERT_DELTA percent of shield energy
// to weapon energy.
void transfer_energy_to_weapons(object* obj)
{
	ship*			ship_p = &Ships[obj->instance];
	ship_info*	sinfo_p = &Ship_info[ship_p->ship_info_index];

	if (ship_p->flags & SF_DYING)
		return;

	if ( obj->flags & OF_NO_LASERS || obj->flags & OF_NO_SHIELDS )
	{
		return;
	}

	transfer_energy_weapon_common(obj, get_shield_strength(obj), ship_p->weapon_energy, &ship_p->target_shields_delta, &ship_p->target_weapon_energy_delta, sinfo_p->max_weapon_reserve, 1.0f);
}

void hudets_page_in()
{
	bm_page_in_aabitmap( Ets_gauge.first_frame, Ets_gauge.num_frames );
}
