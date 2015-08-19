/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "cmdline/cmdline.h"
#include "debugconsole/console.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "io/timer.h"
#include "math/vecmat.h"
#include "mission/missioncampaign.h"
#include "particle/particle.h"
#include "popup/popupdead.h"
#include "ship/ship.h"
#include "starfield/starfield.h"
#include "starfield/supernova.h"

// --------------------------------------------------------------------------------------------------------------------------
// SUPERNOVA DEFINES/VARS
//

// supernova time 1
#define SUPERNOVA_SOUND_1_TIME		15.0f
#define SUPERNOVA_SOUND_2_TIME		5.0f
int Supernova_sound_1_played = 0;
int Supernova_sound_2_played = 0;

// countdown for supernova
static float Supernova_time_total = -1.0f;
static float Supernova_time = -1.0f;
static int Supernova_finished = 0;
static int Supernova_popup = 0;
static float Supernova_fade_to_white = 0.0f;
static int Supernova_particle_stamp = -1;

int Supernova_status = SUPERNOVA_NONE;

// --------------------------------------------------------------------------------------------------------------------------
// SUPERNOVA FUNCTIONS
//

// level init
void supernova_level_init()
{
	Supernova_time_total = -1.0f;
	Supernova_time = -1.0f;	
	Supernova_finished = 0;
	Supernova_fade_to_white = 0.0f;
	Supernova_popup = 0;
	Supernova_particle_stamp = -1;

	Supernova_sound_1_played = 0;
	Supernova_sound_2_played = 0;

	Supernova_status = SUPERNOVA_NONE;
}

// start a supernova
void supernova_start(int seconds)
{
	// bogus time
	if((float)seconds < SUPERNOVA_CUT_TIME) {
		return;
	}

	// no supernova in multiplayer
	if(Game_mode & GM_MULTIPLAYER) {
		return;
	}

	Supernova_time_total = (float)seconds;
	Supernova_time = (float)seconds;
	Supernova_finished = 0;
	Supernova_fade_to_white = 0.0f;
	Supernova_popup = 0;
	Supernova_particle_stamp = -1;

	Supernova_status = SUPERNOVA_STARTED;
}

void supernova_stop()
{
	// There's no currently active supernova
	if(Supernova_status != SUPERNOVA_STARTED)
	{
		return;
	}
	
	// We're too late.
	if(supernova_time_left() < SUPERNOVA_CUT_TIME)
	{
		return;
	}

	// A supernova? In MY multiplayer?
	if(Game_mode & GM_MULTIPLAYER) {
		return;
	}

	Supernova_time_total = -1.0f;
	Supernova_time = -1.0f;
	Supernova_finished = 0;
	Supernova_popup = 0;
	Supernova_fade_to_white = 0.0f;
	Supernova_particle_stamp = -1;

	Supernova_status = SUPERNOVA_NONE;
}


int sn_particles = 100;
DCF_INT2(sn_part, sn_particles, 0, INT_MAX, "Sets number of supernova particles (default is 100)");

void supernova_do_particles()
{
	int idx;
	vec3d a, b, ta, tb;
	vec3d norm, sun_temp;

	// no player ship
	if((Player_obj == NULL) || (Player_ship == NULL)) {
		return;
	}

	// timestamp
	if((Supernova_particle_stamp == -1) || timestamp_elapsed(Supernova_particle_stamp)) {
		Supernova_particle_stamp = timestamp(sn_particles);

		// get particle norm
		stars_get_sun_pos(0, &sun_temp);
		vm_vec_add2(&sun_temp, &Player_obj->pos);
		vm_vec_sub(&norm, &Player_obj->pos, &sun_temp);
		vm_vec_normalize(&norm);

		particle_emitter whee;
		whee.max_life = 1.0f;
		whee.min_life = 0.6f;
		whee.max_vel = 50.0f;
		whee.min_vel = 25.0f;
		whee.normal_variance = 0.75f;
		whee.num_high = 5;
		whee.num_low = 2;
		whee.min_rad = 0.5f;
		whee.max_rad = 1.25f;

		// emit
		for(idx=0; idx<10; idx++) {
			if ( Cmdline_old_collision_sys ) {
				submodel_get_two_random_points(Ship_info[Player_ship->ship_info_index].model_num, 0, &ta, &tb);
			} else {
				submodel_get_two_random_points_better(Ship_info[Player_ship->ship_info_index].model_num, 0, &ta, &tb);
			}

			// rotate into world space
			vm_vec_unrotate(&a, &ta, &Player_obj->orient);
			vm_vec_add2(&a, &Player_obj->pos);
			whee.pos = a;
			whee.vel = norm;
			vm_vec_scale(&whee.vel, 30.0f);
			vm_vec_add2(&whee.vel, &Player_obj->phys_info.vel);
			whee.normal = norm;
			particle_emit(&whee, PARTICLE_FIRE, 0);

			vm_vec_unrotate(&b, &tb, &Player_obj->orient);
			vm_vec_add2(&b, &Player_obj->pos);
			whee.pos = b;
			particle_emit(&whee, PARTICLE_FIRE, 0);
		}
	}
}

// call once per frame
float sn_shudder = 0.45f;
DCF_FLOAT2(sn_shud, sn_shudder, 0.0, FLT_MAX, "Sets camera shudder rate for being in supernova shockwave (default is 0.45)");

void supernova_process()
{
	int sn_stage;

	// if the supernova is running
	sn_stage = supernova_active();
	if(sn_stage) {
		Supernova_time -= flFrametime;

		// sound stuff
		if((Supernova_time <= SUPERNOVA_SOUND_1_TIME) && !Supernova_sound_1_played) {
			Supernova_sound_1_played = 1;
			snd_play(&Snds[SND_SUPERNOVA_1], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY);
		}
		if((Supernova_time <= SUPERNOVA_SOUND_2_TIME) && !Supernova_sound_2_played) {
			Supernova_sound_2_played = 1;
			snd_play(&Snds[SND_SUPERNOVA_2], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY);
		}

		// if we've crossed from stage 1 to stage 2 kill all particles and stick a bunch on the player ship
		if((sn_stage == 1) && (supernova_active() == 2)) {
			// first kill all active particles so we have a bunch of free ones
			particle_kill_all();
		}

		// if we're in stage 2, emit particles
		if((sn_stage >= 2) && (sn_stage != 5)) {
			supernova_do_particles();
		}

		// if we've got negative. the supernova is done
		if(Supernova_time < 0.0f) {
			Supernova_finished = 1;
			Supernova_fade_to_white += flFrametime;

			// start the dead popup
			if(Supernova_fade_to_white >= SUPERNOVA_FADE_TO_WHITE_TIME) {
				if(!Supernova_popup) {
					// main freespace 2 campaign? if so - end it now
					//
					// don't actually check for a specific campaign here since others may want to end this way but we
					// should test positive here if in campaign mode and sexp_end_campaign() got called - taylor
					if (Campaign_ending_via_supernova && (Game_mode & GM_CAMPAIGN_MODE) /*&& !stricmp(Campaign.filename, "freespace2")*/) {
						gameseq_post_event(GS_EVENT_END_CAMPAIGN);
					} else {
						popupdead_start();
					}
					Supernova_popup = 1;
				}
				Supernova_finished = 2;
			}
		}
	}
}

// is there a supernova active
int supernova_active()
{
	// if not supernova has been set then just bail now
	if (Supernova_status == SUPERNOVA_NONE) {
		return 0;
	}

	// if the supernova has "finished". fade to white and dead popup
	if(Supernova_finished == 1) {
		Supernova_status = SUPERNOVA_HIT;
		return 4;
	}
	if(Supernova_finished == 2) {
		Supernova_status = SUPERNOVA_HIT;
		return 5;
	}

	// no supernova
	if( (Supernova_time_total <= 0.0f) || (Supernova_time <= 0.0f) ) {
		return 0;
	}	

	// final stage,
	if(Supernova_time < (SUPERNOVA_CUT_TIME - SUPERNOVA_CAMERA_MOVE_TIME)) {
		Supernova_status = SUPERNOVA_HIT;
		return 3;
	}

	// 2nd stage
	if(Supernova_time < SUPERNOVA_CUT_TIME) {
		Supernova_status = SUPERNOVA_HIT;
		return 2;
	}

	// first stage
	return 1;
}

// time left before the supernova hits
float supernova_time_left()
{
	return Supernova_time;
}

// pct complete the supernova (0.0 to 1.0)
float supernova_pct_complete()
{
	// bogus
	if(!supernova_active()) {
		return -1.0f;
	}

	return (Supernova_time_total - Supernova_time) / Supernova_time_total;
}

// if the camera should cut to the "you-are-toast" cam
int supernova_camera_cut()
{
	// if we're not in a supernova
	if(!supernova_active()) {
		return 0;
	}

	// if we're past the critical time
	if(Supernova_time <= SUPERNOVA_CUT_TIME) {
		return 1;
	}

	// no cut yet
	return 0;
}

// get view params from supernova
float sn_distance = 300.0f;				// shockwave moving at 1000/ms ?
float sn_cam_distance = 25.0f;
DCF_FLOAT2(sn_dist, sn_distance, 0.0, FLT_MAX, "Sets supernova shockwave distance (default is 300.0f)");


DCF_FLOAT2(sn_cam_dist, sn_cam_distance, 0.0, FLT_MAX, "Sets supernova camera distance (default is 25.0f)");

void supernova_get_eye(vec3d *eye_pos, matrix *eye_orient)
{
	// supernova camera pos
	vec3d Supernova_camera_pos;
	static matrix Supernova_camera_orient;

	vec3d at;
	vec3d sun_temp, sun_vec;
	vec3d view;

	// set the controls for the heart of the sun
	stars_get_sun_pos(0, &sun_temp);
	vm_vec_add2(&sun_temp, &Player_obj->pos);
	vm_vec_sub(&sun_vec, &sun_temp, &Player_obj->pos);
	vm_vec_normalize(&sun_vec);

	// always set the camera pos
	vec3d move;
	matrix whee;
	vm_vector_2_matrix(&whee, &move, NULL, NULL);
	vm_vec_scale_add(&Supernova_camera_pos, &Player_obj->pos, &whee.vec.rvec, sn_cam_distance);
	vm_vec_scale_add2(&Supernova_camera_pos, &whee.vec.uvec, 30.0f);
	//cam->set_position(&Supernova_camera_pos);
	*eye_pos = Supernova_camera_pos;

	// if we're no longer moving the camera
	if(Supernova_time < (SUPERNOVA_CUT_TIME - SUPERNOVA_CAMERA_MOVE_TIME)) {
		// *eye_pos = Supernova_camera_pos;
		//cam->set_rotation(&Supernova_camera_orient);
		*eye_orient = Supernova_camera_orient;
	}
	// otherwise move it
	else {
		// get a vector somewhere between the supernova shockwave and the player ship
		at = Player_obj->pos;
		vm_vec_scale_add2(&at, &sun_vec, sn_distance);
		vm_vec_sub(&move, &Player_obj->pos, &at);
		vm_vec_normalize(&move);

		// linearly move towards the player pos
		float pct = ((SUPERNOVA_CUT_TIME - Supernova_time) / SUPERNOVA_CAMERA_MOVE_TIME);
		vm_vec_scale_add2(&at, &move, sn_distance * pct);

		vm_vec_sub(&view, &at, &Supernova_camera_pos);
		vm_vec_normalize(&view);
		vm_vector_2_matrix(&Supernova_camera_orient, &view, NULL, NULL);
		//cam->set_rotation(&Supernova_camera_orient);
		*eye_orient = Supernova_camera_orient;
	}
	//return supernova_camera;
}
