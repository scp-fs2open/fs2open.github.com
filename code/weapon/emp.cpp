/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include <stdarg.h>


#include "weapon/emp.h"
#include "io/timer.h"
#include "freespace2/freespace.h"
#include "globalincs/linklist.h"
#include "hud/hudlock.h"
#include "hud/hudtarget.h"
#include "hud/hud.h"
#include "object/object.h"
#include "weapon/weapon.h"
#include "ship/ship.h"
#include "parse/parselo.h"
#include "iff_defs/iff_defs.h"
#include "network/multimsgs.h"
#include "network/multi.h"
#include "debugconsole/console.h"




// ----------------------------------------------------------------------------------------------------
// EMP EFFECT DEFINES/VARS
//

// intensity of the playing effect
float Emp_intensity = -1.0f;					// current intensity of the EMP effect (normalized to EMP_INTENSITY_MAX)
float Emp_decr = 0.0f;							// how much to decrement the effect per second

// timestamp until we should randomly choose another target
int Emp_wacky_target_timestamp = -1;

// max time we'll disrupt turrets on big ships
#define MAX_TURRET_DISRUPT_TIME				7500

// conventient for determining if EMP is active
#define EMP_ACTIVE_LOCAL()			(Emp_intensity > 0.0f)

// for keeping track of messed up text
#define EMP_WACKY_TEXT_LEN					256
typedef struct wacky_text {
	char str[EMP_WACKY_TEXT_LEN];
	int stamp;
} wacky_text;
wacky_text Emp_wacky_text[NUM_TEXT_STAMPS];

// for randomly inserting characters
#define NUM_RANDOM_CHARS		51
char Emp_random_char[NUM_RANDOM_CHARS] = 
									{ 'a', 'b', 'c', 'd', 'e', 'f', 'g', '4', 'h', '8', '_', '$', ')', '-', '~', 'u', 'q', 
									  '.', 'x', 'h', '&', '%', '*', '1', '3', 't', 'h', 'o', 'p', '@', 'h', 'i','v', '+', '=',
									  '|', '{', '}', ':', ';', '^', 'l', 'z', 'u', 'v', '<', '>', '?', '5', '8' };

// EMP EFFECTS ON PLAYERS -----
// 1.) Lose target lock if any, along with ability to lock on
// 2.) Display EMP-BLAST icon or something (maybe flash it)
// 3.) at wacky intervals, target random ships (which he cannot lock on)
// 4.) Randomly flicker HUD gauges
// 5.) Randomly swap/mess-up HUD text

// EMP EFFECTS ON SHIPS -------
// 1.) Lightning effect proportional to the emp effect

// ----------------------------------------------------------------------------------------------------
// EMP EFFECT FUNCTIONS
//

// maybe reformat a string 
void emp_maybe_reformat_text(char *text, int max_len, int gauge_id);

// randomize the chars in a string
void emp_randomize_chars(char *str);


// initialize the EMP effect for the mission
void emp_level_init()
{
	int idx;
	
	// reset all vars
	Emp_intensity = 0.0f;	
	Emp_wacky_target_timestamp = -1;

	for(idx=0; idx<NUM_TEXT_STAMPS; idx++){
		memset(Emp_wacky_text[idx].str, 0, EMP_WACKY_TEXT_LEN);
		Emp_wacky_text[idx].stamp = -1;
	}
}

// apply the EMP effect to all relevant ships
void emp_apply(vec3d *pos, float inner_radius, float outer_radius, float emp_intensity, float emp_time, bool use_emp_time_for_capship_turrets)
{	
	float actual_intensity, actual_time;
	vec3d dist;
	float dist_mag;
	float scale_factor;
	object *target;
	ship_obj *so;
	missile_obj *mo;
	ship_subsys *moveup;
	weapon_info *wip_target;

	// all machines check to see if the blast hit a bomb. if so, shut it down (can't move anymore)	
	for( mo = GET_FIRST(&Missile_obj_list); mo != END_OF_LIST(&Missile_obj_list); mo = GET_NEXT(mo) ) {
		target = &Objects[mo->objnum];
		if(target->type != OBJ_WEAPON){
			continue;
		}

		Assert(target->instance >= 0);
		if(target->instance < 0){
			continue;
		}
		Assert(Weapons[target->instance].weapon_info_index >= 0);
		if(Weapons[target->instance].weapon_info_index < 0){
			continue;
		}
		
		// if we have a bomb weapon
		wip_target = &Weapon_info[Weapons[target->instance].weapon_info_index];
		if((wip_target->weapon_hitpoints > 0) && !(wip_target->wi_flags2 & WIF2_NO_EMP_KILL)) {
			// get the distance between the detonation and the target object
			vm_vec_sub(&dist, &target->pos, pos);
			dist_mag = vm_vec_mag(&dist);

			// if the bomb was within 1/4 of the outer radius, castrate it
			if(dist_mag <= (outer_radius * 0.25f)){
				// memset(&target->phys_info, 0, sizeof(physics_info));
				Weapons[target->instance].weapon_flags |= WF_DEAD_IN_WATER;
				mprintf(("EMP killing weapon\n"));
			}
		}	
	}

	// if I'm only a client in a multiplayer game, do nothing
	if(MULTIPLAYER_CLIENT){
		return;
	}

	// See if there are any friendly ships present, if so return without preventing msg
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {		
		target = &Objects[so->objnum];
		if(target->type != OBJ_SHIP){
			continue;
		}	
		
		Assert(Objects[so->objnum].instance >= 0);
		if(Objects[so->objnum].instance < 0){
			continue;
		}
		Assert(Ships[Objects[so->objnum].instance].ship_info_index >= 0);
		if(Ships[Objects[so->objnum].instance].ship_info_index < 0){
			continue;
		}

		// if the ship is a cruiser or cap ship, only apply the EMP effect to turrets
		if(Ship_info[Ships[target->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {
			float capship_emp_time = use_emp_time_for_capship_turrets ? emp_time : MAX_TURRET_DISRUPT_TIME;
			
			moveup = &Ships[target->instance].subsys_list;
			if(moveup->next != NULL){
				moveup = moveup->next;
			}
			while(moveup != &Ships[target->instance].subsys_list){
				// if this is a turret, disrupt it
				if((moveup->system_info != NULL) && (moveup->system_info->type == SUBSYSTEM_TURRET)){
					vec3d actual_pos;					
					
					// get the distance to the subsys					
					vm_vec_unrotate(&actual_pos, &moveup->system_info->pnt, &target->orient);
					vm_vec_add2(&actual_pos, &target->pos);					
					vm_vec_sub(&dist, &actual_pos, pos);
					dist_mag = vm_vec_mag(&dist);
			
					// if for some reason, the object was outside the blast, radius
					if(dist_mag > outer_radius){			
						// next item
						moveup = moveup->next;
						continue;
					}

					// compute a scale factor for the emp effect
					scale_factor = 1.0f;
					if(dist_mag >= inner_radius){
						scale_factor = 1.0f - (dist_mag / outer_radius);
					} 

					scale_factor -= Ship_info[Ships[target->instance].ship_info_index].emp_resistance_mod;

					if (scale_factor < 0.0f) {
						moveup = moveup->next;
						continue;
					}

					// disrupt the turret
					ship_subsys_set_disrupted(moveup, (int)(capship_emp_time * scale_factor));

					mprintf(("EMP disrupting subsys %s on ship %s (%f, %f)\n", moveup->system_info->subobj_name, Ships[Objects[so->objnum].instance].ship_name, scale_factor, capship_emp_time * scale_factor));
				}
				
				// next item
				moveup = moveup->next;
			}
		}
		// otherwise coat the whole ship with the effect. mmmmmmmmm.
		else {				
			// get the distance between the detonation and the target object
			vm_vec_sub(&dist, &target->pos, pos);
			dist_mag = vm_vec_mag(&dist);

			// if for some reason, the object was outside the blast, radius
			if(dist_mag > outer_radius){			
				continue;
			}

			// compute a scale factor for the emp effect
			scale_factor = 1.0f;
			if(dist_mag >= inner_radius){
				scale_factor = 1.0f - (dist_mag / outer_radius);	
			} 

			scale_factor -= Ship_info[Ships[target->instance].ship_info_index].emp_resistance_mod;

			if (scale_factor < 0.0f) {
				continue;
			}
		
			// calculate actual EMP effect values
			actual_intensity = emp_intensity * scale_factor;
			actual_time = emp_time * scale_factor;			
			mprintf(("EMP effect s : %f, i : %f, t : %f\n", scale_factor, actual_intensity, actual_time));

			// if this effect happened to be on me, start it now
			if((target == Player_obj) && !(Game_mode & GM_STANDALONE_SERVER)){
				emp_start_local(actual_intensity, actual_time);
			} 

			// if this is a multiplayer game, notify other players of the effect
			if(Game_mode & GM_MULTIPLAYER){		
				Assert(MULTIPLAYER_MASTER);				
				send_emp_effect(target->net_signature, actual_intensity, actual_time);
			}
			
			// now be sure to start the emp effect for the ship itself
			emp_start_ship(target, actual_intensity, actual_time);
		}
	}
}

// start the emp effect for the passed ship (setup lightning arcs, timestamp, etc)
// NOTE : if this ship is also me, I should call emp_start_local() as well
void emp_start_ship(object *ship_objp, float intensity, float time)
{
	ship *shipp;
	ai_info *aip;
	float start_intensity;

	// make sure this is a ship
	Assert(ship_objp->type == OBJ_SHIP);
	Assert(ship_objp->instance >= 0);
	shipp = &Ships[ship_objp->instance];

	// determining pre-existing EMP intensity (if any)
	start_intensity = shipp->emp_intensity < 0.0f ? 0.0f : shipp->emp_intensity;

	// setup values (capping them if necessary)	(make sure that we un-normalize start_intensity)
	if(intensity + (start_intensity * EMP_INTENSITY_MAX) >= EMP_INTENSITY_MAX){
		intensity = EMP_INTENSITY_MAX - 1.0f;
	} else {
		intensity += (start_intensity * EMP_INTENSITY_MAX);
	}
	intensity /= EMP_INTENSITY_MAX;

	if(time >= EMP_TIME_MAX){
		time = EMP_TIME_MAX - 0.1f;
	}
	shipp->emp_intensity = intensity;
	shipp->emp_decr = intensity / time;

	// multiplayer clients should bail now
	if(MULTIPLAYER_CLIENT){
		return;
	}

	// do any initial AI effects
	Assert(shipp->ai_index >= 0);
	aip = &Ai_info[shipp->ai_index];

	// lose his current target
	set_target_objnum(aip, -1);
	set_targeted_subsys(aip, NULL, -1);
}

// process a ship for this frame
int mod_val = 7;
void emp_process_ship(ship *shipp)
{
	object *objp;
	ai_info *aip;	

	Assert(shipp != NULL);
	if(shipp == NULL){
		return;
	}
	Assert(shipp->objnum >= 0);
	if(shipp->objnum < 0){
		return;
	}
	objp = &Objects[shipp->objnum];

	// if the emp intensity is < 0, there is no effect
	if(shipp->emp_intensity < 0.0f){
		shipp->emp_intensity = -1.0f;

		return;
	}

	// reduce the emp effect
	shipp->emp_intensity -= shipp->emp_decr * flFrametime;

	// multiplayer clients should bail here
	if(MULTIPLAYER_CLIENT){
		return;
	}

	// if this is a player ship, don't do anything wacky
	if(objp->flags & OF_PLAYER_SHIP){
		return;
	}

	// lose lock time, etc, etc.
	Assert(shipp->ai_index >= 0);
	aip = &Ai_info[shipp->ai_index];	
	aip->aspect_locked_time = 0.0f;				// hasn't gotten aspect lock at all
	aip->current_target_is_locked = 0;			// isn't locked on his current target
	aip->ai_flags &= ~AIF_SEEK_LOCK;
	aip->nearest_locked_object = -1;				// nothing near me, so I won't launch countermeasures

	// if he's not a fighter or bomber, bail now
	if(!(Ship_info[shipp->ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER))){
		return;
	}

	// if he's docked, or ordered to not move, bail now
	if (object_is_docked(objp) || (aip->mode == AIM_STILL) || (aip->mode == AIM_PLAY_DEAD)){
		return;
	}
	
	// pick targets randomly and wackily so that the ship flies crazily :)	
	if(((int)f2fl(Missiontime) + (int)(EMP_INTENSITY_MAX * shipp->emp_intensity)) % mod_val == 0){
		int ship_lookup = ship_get_random_team_ship(iff_get_attackee_mask(shipp->team));

		// if we got a valid ship object to target
		if((ship_lookup >= 0) && (Ships[ship_lookup].objnum >= 0) && !(Objects[Ships[ship_lookup].objnum].flags & OF_PROTECTED)){
			// attack the object
			ai_attack_object(objp, &Objects[Ships[ship_lookup].objnum], NULL);
		}
	}
}

// start the emp effect for MYSELF (intensity == arbitrary intensity variable, time == time the effect will last)
// NOTE : time should be in seconds
void emp_start_local(float intensity, float time)
{
	int idx;
	float start_intensity;

	// determine pre-existing EMP intensity (if any)
	start_intensity = Emp_intensity < 0.0f ? 0.0f : Emp_intensity;

	// cap all values (make sure that we un-normalize start_intensity)
	if(intensity + (start_intensity * EMP_INTENSITY_MAX) >= EMP_INTENSITY_MAX){
		intensity = EMP_INTENSITY_MAX - 1.0f;
	} else {
		intensity += (start_intensity * EMP_INTENSITY_MAX);
	}
	if(time >= EMP_TIME_MAX){
		time = EMP_TIME_MAX - 0.1f;
	}

	// setup all vars
	Emp_intensity = intensity / EMP_INTENSITY_MAX;	

	// lose my current target if any
	if(Player_ai != NULL){
		Player_ai->target_objnum = -1;
	}
	// lose any lock we have or are getting
	hud_lock_reset();

	// reset HUD gauge text wackiness stuff
	for(idx=0; idx<NUM_TEXT_STAMPS; idx++){
		memset(Emp_wacky_text[idx].str, 0, 256);
		Emp_wacky_text[idx].stamp = -1;
	}

	// start the emp icon flashing
	hud_start_text_flash(NOX("Emp"), 5000);

	// determine how much we have to decrement the effect per second
	Emp_decr = Emp_intensity / time;

	// play a flash
	game_flash( 1.0f, 1.0f, 0.5f );
}

// stop the emp effect cold
void emp_stop_local()
{
	// kill off various EMP stuff
	Emp_intensity = -1.0f;	
	Emp_wacky_target_timestamp = -1;	
}

// if the EMP effect is active
int emp_active_local()
{
	return EMP_ACTIVE_LOCAL();
}

// process some stuff every frame (before frame is rendered)
void emp_process_local()
{	
	if(!emp_active_local()){
		return;
	}

	// decrement the intensity a bit
	Emp_intensity -= (flFrametime * Emp_decr);	

	// see if we should choose a random target
	if((Emp_wacky_target_timestamp == -1) || timestamp_elapsed(Emp_wacky_target_timestamp)){
		// choose a target (if not the "first" time)
		if(Emp_wacky_target_timestamp != -1){
			hud_target_random_ship();
		}

		// reset the timestamp
		Emp_wacky_target_timestamp = timestamp((int)frand_range(100.0f, 750.0f * (1.0f - Emp_intensity)));
	}			
}

// randomly say yes or no to a gauge, if emp is not active, always say yes
int emp_should_blit_gauge()
{
	// if the EMP effect is not active, always blit
	if(!emp_active_local()){
		return 1;
	}

	// otherwise, randomly say no
	return frand_range(0.0f, 1.0f) > Emp_intensity;
}

// emp hud string
void emp_hud_string(int x, int y, int gauge_id, const char *str, int resize_mode)
{
	char tmp[256] = "";

	// maybe bail
	if (!*str)
		return;

	// copy the string
	strcpy_s(tmp, str);

	// if the emp effect is not active, don't even bother messing with the text
	if(emp_active_local()){
		emp_maybe_reformat_text(tmp, 256, gauge_id);

		// jitter the coords
		emp_hud_jitter(&x, &y);
	}

	// print the string out
	gr_string(x, y, tmp, resize_mode);
}

// emp hud printf
void emp_hud_printf(int x, int y, int gauge_id, const char *format, ...)
{
	char tmp[256] = "";
	va_list args;	
	
	// format the text
	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);
	
	// if the emp effect is not active, don't even bother messing with the text
	if(emp_active_local()){
		emp_maybe_reformat_text(tmp, 256, gauge_id);

		// jitter the coords
		emp_hud_jitter(&x, &y);
	}

	// print the string out
	gr_string(x, y, tmp);
}

// maybe reformat a string 
void emp_maybe_reformat_text(char *text, int max_len, int gauge_id)
{
	wacky_text *wt;

	// if the EMP effect is not active, never reformat it
	if(!emp_active_local()){
		return;
	}

	// randomly _don't_ apply text craziness
	if(frand_range(0.0f, 1.0f) > Emp_intensity){
		return;
	}

	// if the gauge is EG_NULL, empty the string
	if(gauge_id == EG_NULL){
		strcpy(text, "");
		return;
	}

	// if this gauge has not been wacked out, or if the timestamp has expired, we
	// neeed to wack it out again
	Assert((gauge_id >= EG_NULL) && (gauge_id < NUM_TEXT_STAMPS));
	wt = &Emp_wacky_text[gauge_id];
	if((wt->stamp == -1) || timestamp_elapsed(wt->stamp)){
		// reformat specific gauges differently
		switch(gauge_id){	
		//	weapons
		case EG_WEAPON_TITLE: case EG_WEAPON_P1: case EG_WEAPON_P2: case EG_WEAPON_P3: case EG_WEAPON_S1: case EG_WEAPON_S2:			
			int wep_index;
			wep_index = (int)frand_range(0.0f, (float)(MAX_WEAPON_TYPES - 1));
			strcpy_s(wt->str, Weapon_info[ wep_index >= MAX_WEAPON_TYPES ? 0 : wep_index ].name);			
			break;		

		// escort list
		case EG_ESCORT1: case EG_ESCORT2: case EG_ESCORT3:
			// choose a random ship
			int shipnum;
			shipnum = ship_get_random_targetable_ship();
			if(shipnum >= 0){
				strcpy_s(wt->str, Ships[shipnum].ship_name);
			}
			break;

		// directives title
		case EG_OBJ_TITLE:
			strcpy_s(wt->str, "");
			break;

		// directives themselves
		case EG_OBJ1: case EG_OBJ2: case EG_OBJ3: case EG_OBJ4: case EG_OBJ5:
			strcpy_s(wt->str, text);
			emp_randomize_chars(wt->str);
			break;

		// target box info
		case EG_TBOX_EXTRA1: case EG_TBOX_EXTRA2: case EG_TBOX_EXTRA3: case EG_TBOX_CLASS:
		case EG_TBOX_DIST: case EG_TBOX_CARGO: case EG_TBOX_HULL: case EG_TBOX_NAME: case EG_TBOX_INTEG:
			strcpy_s(wt->str, text);
			emp_randomize_chars(wt->str);
			break;

		// squadmsg menu
		case EG_SQ1: case EG_SQ2: case EG_SQ3: case EG_SQ4: case EG_SQ5: case EG_SQ6: case EG_SQ7:
		case EG_SQ8: case EG_SQ9: case EG_SQ10:
			strcpy_s(wt->str, text);
			emp_randomize_chars(wt->str);
			break;
			
		// default 
		default :
			return;
		}

		// recalculate the timestamp
		wt->stamp = timestamp((int)frand_range(100.0f, 750.0f * (1.0f - Emp_intensity)));

		// copy the text
		strcpy(text, wt->str);
	}
	// otherwise, use what we calculated last time
	else {
		strcpy(text, wt->str);
	}

	// watch out for '#' - Goober5000
	end_string_at_first_hash_symbol(text);
}

// randomize the chars in a string
void emp_randomize_chars(char *str)
{	
	int idx;
	int char_index;
	
	// shuffle chars around
	for(idx=0; idx<(int)(strlen(str)-1); idx++){
		if(frand_range(0.0f, 1.0f) < Emp_intensity){
			char_index = (int)frand_range(0.0f, (float)(NUM_RANDOM_CHARS - 1));
			str[idx] = Emp_random_char[char_index];
		}
	}
}

// throw some jitter into HUD x and y coords
void emp_hud_jitter(int *x, int *y)
{
	// if the emp effect is not active, don't jitter anything
	if(!emp_active_local()){
		return;
	}

	// some movement
	*x += (int)frand_range(-8.0f * Emp_intensity, 8.0f * Emp_intensity);
	*y += (int)frand_range(-8.0f * Emp_intensity, 8.0f * Emp_intensity);
}

// current intensity of the EMP effect (0.0 - 1.0)
float emp_current_intensity()
{
	return Emp_intensity;
}

DCF(zap, "zap a ship with an EMP effect")
{
	int shipnum;
	SCP_string ship_str;

	dc_stuff_string_white(ship_str);
	shipnum = ship_name_lookup(ship_str.c_str(), 1);

	if(shipnum >= 0){
		emp_start_ship(&Objects[Ships[shipnum].objnum], 500.0f, 10.0f);
	}
}
