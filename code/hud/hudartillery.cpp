/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "hud/hudartillery.h"
#include "parse/parselo.h"
#include "weapon/weapon.h"
#include "weapon/beam.h"
#include "math/vecmat.h"
#include "globalincs/linklist.h"
#include "io/timer.h"
#include "fireball/fireballs.h"
#include "object/object.h"
#include "ai/ai.h"
#include "globalincs/alphacolors.h"
#include "network/multi.h"
#include "hud/hudmessage.h"
#include "sound/sound.h"
#include "gamesnd/gamesnd.h"

// -----------------------------------------------------------------------------------------------------------------------
// ARTILLERY DEFINES/VARS
//
// Goober5000 - moved to hudartillery.h

// -----------------------------------------------------------------------------------------------------------------------
// ARTILLERY FUNCTIONS
//

// test code for subspace missile strike -------------------------------------------

// ssm_info, like ship_info etc.
SCP_vector<ssm_info> Ssm_info;

// list of active strikes
SCP_list<ssm_strike> Ssm_strikes;

// Goober5000
int ssm_info_lookup(const char *name)
{
	if(name == NULL)
		return -1;

	for (auto it = Ssm_info.cbegin(); it != Ssm_info.cend(); ++it)
		if (!stricmp(name, it->name))
			return std::distance(Ssm_info.cbegin(), it);

	return -1;
}

void parse_ssm(const char *filename)
{
	char weapon_name[NAME_LENGTH];

	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		// parse the table
		while(required_string_either("#end", "$SSM:")) {
			required_string("$SSM:");
			ssm_info s;

			// name
			stuff_string(s.name, F_NAME, NAME_LENGTH);

			// stuff data
			required_string("+Weapon:");
			stuff_string(weapon_name, F_NAME, NAME_LENGTH);
			if (optional_string("+Count:"))
				stuff_int(&s.count);
			else
				s.count = 1;
			required_string("+WarpRadius:");
			stuff_float(&s.warp_radius);
			if (optional_string("+WarpTime:")) {
				stuff_float(&s.warp_time);
				// According to fireballs.cpp, "Warp lifetime must be at least 4 seconds!"
				if ( (s.warp_time) < 4.0f) {
					// So let's warn them before they try to use it, shall we?
					Warning(LOCATION, "Expected a '+WarpTime:' value equal or greater than 4.0, found '%f' in weapon '%s'.\n Setting to 4.0, please check and set to a number 4.0 or greater!\n", s.warp_time, weapon_name);
					// And then make the Assert obsolete -- Zacam
					s.warp_time = 4.0f;
				}
			} else {
				s.warp_time = 4.0f;
			}
			required_string("+Radius:");
			stuff_float(&s.radius);
			if (optional_string("+Offset:"))
				stuff_float(&s.offset);
			else
				s.offset = 0.0f;
			if (optional_string("+Shape:")) {
				switch(required_string_one_of(3, "Point", "Circle", "Sphere")) {
				case 0:
					required_string("Point");
					s.shape = SSM_SHAPE_POINT;
					break;
				case 1:
					required_string("Circle");
				case -1:	// If we're ignoring parse errors and can't identify the shape, go with a circle.
					s.shape = SSM_SHAPE_CIRCLE;
					break;
				case 2:
					required_string("Sphere");
					s.shape = SSM_SHAPE_SPHERE;
					break;
				default:
					Assertion(false, "Impossible return value from required_string_one_of(); get a coder!\n");
				}
			} else {
				s.shape = SSM_SHAPE_CIRCLE;
			}
			if (optional_string("+HUD Message:"))
				stuff_boolean(&s.send_message);
			else
				s.send_message = true;
			if (optional_string("+Custom Message:")) {
				stuff_string(s.message, F_NAME, NAME_LENGTH);
				s.use_custom_message = true;
			}
			parse_sound("+Alarm Sound:", &s.sound_index, s.name);

			// see if we have a valid weapon
			s.weapon_info_index = weapon_info_lookup(weapon_name);
			if(s.weapon_info_index >= 0) {
				// valid
				int existing = ssm_info_lookup(s.name);
				if (existing >= 0) {	// Redefined the existing entry instead of adding a duplicate.
					Ssm_info[existing] = s;
				} else {
					Ssm_info.push_back(s);
				}
			}
		}
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

// game init
void ssm_init()
{
	if (cf_exists_full("ssm.tbl", CF_TYPE_TABLES)) {
		mprintf(("TABLES => Starting parse of 'ssm.tbl'...\n"));
		parse_ssm("ssm.tbl");
	}
	parse_modular_table(NOX("*-ssm.tbm"), parse_ssm);

	// Now that we've populated Ssm_info, let's validate weapon $SSM: entries.
	validate_SSM_entries();
}

void ssm_get_random_start_pos(vec3d *out, vec3d *start, matrix *orient, int ssm_index)
{
	vec3d temp;
	ssm_info *s = &Ssm_info[ssm_index];

	switch (s->shape) {
	case SSM_SHAPE_SPHERE:
		// get a random vector in a sphere around the target
		vm_vec_random_in_sphere(&temp, start, orient, s->radius, 1);
		break;
	case SSM_SHAPE_CIRCLE:
		// get a random vector in the circle of the firing plane
		vm_vec_random_in_circle(&temp, start, orient, s->radius, 1);
		break;
	case SSM_SHAPE_POINT:
		// boooring
		vm_vec_scale_add(&temp, start, &orient->vec.fvec, s->radius);
		break;
	default:
		Assertion(false, "Unknown shape '%d' in SSM type #%d ('%s'). This should not be possible; get a coder!\n", s->shape, ssm_index, s->name);
		break;
	}

	// offset it a bit
	vm_vec_scale_add(out, &temp, &orient->vec.fvec, s->offset);
}

// level init
void ssm_level_init()
{
}

// start a subspace missile effect
void ssm_create(object *target, vec3d *start, int ssm_index, ssm_firing_info *override, int team)
{	
	ssm_strike ssm;
	matrix dir;
	int idx, count;

	// sanity
	Assert(target != NULL);
	if(target == NULL){
		return;
	}
	Assert(start != NULL);
	if(start == NULL){
		return;
	}
	if ((ssm_index < 0) || (ssm_index >= static_cast<int>(Ssm_info.size()))) {
		return;
	}

	// Init the ssm data

	count = Ssm_info[ssm_index].count;

	// override in multiplayer
	if(override != NULL){
		ssm.sinfo = *override;
	}
	// single player or the server
	else {
		// forward orientation
		vec3d temp;

		vm_vec_sub(&temp, &target->pos, start);
		vm_vec_normalize(&temp);

		vm_vector_2_matrix(&dir, &temp, NULL, NULL);

		// stuff info
		ssm.sinfo.ssm_index = ssm_index;
		ssm.sinfo.target = target;
		ssm.sinfo.ssm_team = team;

		// Instead of pushing them on one at a time, let's just grab all the memory we'll need at once
		// (as a side effect, don't need to change the logic from the old array-based code)
		ssm.sinfo.delay_stamp.resize(count);
		ssm.sinfo.start_pos.resize(count);

		for (idx = 0; idx < count; idx++) {
			ssm.sinfo.delay_stamp[idx] = timestamp(200 + (int)frand_range(-199.0f, 1000.0f));
			ssm_get_random_start_pos(&ssm.sinfo.start_pos[idx], start, &dir, ssm_index);
		}

		ssm_info *si = &Ssm_info[ssm_index];
		weapon_info *wip = &Weapon_info[si->weapon_info_index];
		if (wip->wi_flags & WIF_BEAM) {
			ssm.sinfo.duration = ((si->warp_time - ((wip->b_info.beam_warmup / 1000.0f) + wip->b_info.beam_life + (wip->b_info.beam_warmdown / 1000.0f))) / 2.0f) / si->warp_time;
		} else {
			ssm.sinfo.duration = 0.5f;
		}

		// if we're the server, send a packet
		if(MULTIPLAYER_MASTER){
			//
		}
	}

	ssm.done_flags.clear();
	ssm.done_flags.resize(count);
	ssm.fireballs.clear();
	ssm.fireballs.resize(count, -1);
	
	if(Ssm_info[ssm_index].send_message) {
		if (!Ssm_info[ssm_index].use_custom_message)
			HUD_printf(XSTR("Firing artillery", 1570));
		else
			HUD_printf(Ssm_info[ssm_index].message);
	}
	if (Ssm_info[ssm_index].sound_index >= 0) {
		snd_play(&Snds[Ssm_info[ssm_index].sound_index]);
	}

	Ssm_strikes.push_back(ssm);
}

// delete a finished ssm effect
void ssm_delete(SCP_list<ssm_strike>::iterator ssm)
{
	Ssm_strikes.erase(ssm);
}

// process subspace missile stuff
void ssm_process()
{
	int idx, finished;
	SCP_list<ssm_strike>::iterator moveup, eraser;
	ssm_info *si;
	int weapon_objnum;

	// process all strikes
	moveup = Ssm_strikes.begin();
	while ( moveup != Ssm_strikes.end() ) {
		// get the type
		if(moveup->sinfo.ssm_index < 0){
			continue;
		}
		si = &Ssm_info[moveup->sinfo.ssm_index];

		// check all the individual missiles
		finished = 1;
		for(idx=0; idx<si->count; idx++){
			// if this guy is not marked as done
			if(!moveup->done_flags[idx]){
				finished = 0;				

				// if he already has the fireball effect
				if(moveup->fireballs[idx] >= 0){
					if ((1.0f - fireball_lifeleft_percent(&Objects[moveup->fireballs[idx]])) >= moveup->sinfo.duration) {
						weapon_info *wip = &Weapon_info[si->weapon_info_index];
						// are we a beam? -MageKing17
						if (wip->wi_flags & WIF_BEAM) {
							beam_fire_info fire_info;
							memset(&fire_info, 0, sizeof(beam_fire_info));

							fire_info.accuracy = 0.000001f;		// this will guarantee a hit
							fire_info.shooter = NULL;
							fire_info.turret = NULL;
							fire_info.target = moveup->sinfo.target;
							fire_info.target_subsys = NULL;
							fire_info.bfi_flags |= BFIF_FLOATING_BEAM;
							fire_info.starting_pos = moveup->sinfo.start_pos[idx];
							fire_info.beam_info_index = si->weapon_info_index;
							fire_info.team = static_cast<char>(moveup->sinfo.ssm_team);

							// fire the beam
							beam_fire(&fire_info);

							moveup->done_flags[idx] = true;
						} else {
							// get an orientation
							vec3d temp;
							matrix orient;

							vm_vec_sub(&temp, &moveup->sinfo.target->pos, &moveup->sinfo.start_pos[idx]);
							vm_vec_normalize(&temp);
							vm_vector_2_matrix(&orient, &temp, NULL, NULL);

							// fire the missile and flash the screen
							weapon_objnum = weapon_create(&moveup->sinfo.start_pos[idx], &orient, si->weapon_info_index, -1, -1, 1);

							if (weapon_objnum >= 0) {
								Weapons[Objects[weapon_objnum].instance].team = moveup->sinfo.ssm_team;
								Weapons[Objects[weapon_objnum].instance].homing_object = moveup->sinfo.target;
								Weapons[Objects[weapon_objnum].instance].target_sig = moveup->sinfo.target->signature;
							}

							// this makes this particular missile done
							moveup->done_flags[idx] = true;
						}
					}
				}
				// maybe create his warpin effect
				else if((moveup->sinfo.delay_stamp[idx] >= 0) && timestamp_elapsed(moveup->sinfo.delay_stamp[idx])){
					// get an orientation
					vec3d temp;
					matrix orient;

                    vm_vec_sub(&temp, &moveup->sinfo.target->pos, &moveup->sinfo.start_pos[idx]);
					vm_vec_normalize(&temp);
					vm_vector_2_matrix(&orient, &temp, NULL, NULL);
					moveup->fireballs[idx] = fireball_create(&moveup->sinfo.start_pos[idx], FIREBALL_WARP, FIREBALL_WARP_EFFECT, -1, si->warp_radius, 0, &vmd_zero_vector, si->warp_time, 0, &orient);
				}
			}
		}
		if(finished){
			eraser = moveup;
			++moveup;
			ssm_delete(eraser);
			continue;
		}
		
		++moveup;
	}
}


// test code for subspace missile strike -------------------------------------------

// level init
void hud_init_artillery()
{
}

// update all hud artillery related stuff
void hud_artillery_update()
{
}

// render all hud artillery related stuff
void hud_artillery_render()
{
	// render how long the player has been painting his target	
	if((Player_ai != NULL) && (Player_ai->artillery_objnum >= 0)){
		gr_set_color_fast(&Color_bright_blue);
		gr_printf(10, 50, "%f", Player_ai->artillery_lock_time);
	}
}
