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
#include "freespace.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "io/timer.h"
#include "math/vecmat.h"
#include "mission/missioncampaign.h"
#include "particle/particle.h"
#include "particle/ParticleEffect.h"
#include "particle/volumes/LegacyAACuboidVolume.h"
#include "popup/popupdead.h"
#include "ship/ship.h"
#include "starfield/starfield.h"
#include "starfield/supernova.h"

// --------------------------------------------------------------------------------------------------------------------------
// SUPERNOVA DEFINES/VARS
//

// countdown for supernova
static float Supernova_time_total = -1.0f;
static float Supernova_time_left = -1.0f;	// this is no longer updated directly, but calculated in supernova_process()
static TIMESTAMP Supernova_timestamp;
static TIMESTAMP Supernova_fade_to_white_timestamp;
static TIMESTAMP Supernova_particle_timestamp;

static particle::ParticleEffectHandle Supernova_particle_effect;

auto Supernova_status = SUPERNOVA_STAGE::NONE;

// --------------------------------------------------------------------------------------------------------------------------
// SUPERNOVA FUNCTIONS
//

static particle::ParticleEffectHandle supernova_init_particle() {
	return particle::ParticleManager::get()->addEffect(particle::ParticleEffect(
			"", //Name
			::util::UniformFloatRange(2.f, 5.f), //Particle num
			particle::ParticleEffect::Duration::ONETIME, //Single Particle Emission
			::util::UniformFloatRange(), //No duration
			::util::UniformFloatRange (-1.f), //Single particle only
			particle::ParticleEffect::ShapeDirection::ALIGNED, //Particle direction
			::util::UniformFloatRange(1.f), //Velocity Inherit
			false, //Velocity Inherit absolute?
			make_unique<particle::LegacyAACuboidVolume>(0.75f, 1.f, true), //Velocity volume
			::util::UniformFloatRange(25.f, 50.f), //Velocity volume multiplier
			particle::ParticleEffect::VelocityScaling::NONE, //Velocity directional scaling
			std::nullopt, //Orientation-based velocity
			std::nullopt, //Position-based velocity
			nullptr, //Position volume
			particle::ParticleEffectHandle::invalid(), //Trail
			1.f, //Chance
			true, //Affected by detail
			1.f, //Culling range multiplier
			true, //Disregard Animation Length. Must be true for everything using particle::Anim_bitmap_X
			false, //Don't reverse animation
			false, //parent local
			false, //ignore velocity inherit if parented
			false, //position velocity inherit absolute?
			std::nullopt, //Local velocity offset
			std::nullopt, //Local offset
			::util::UniformFloatRange(0.6f, 1.f), //Lifetime
			::util::UniformFloatRange(0.5f, 1.25f), //Radius
			particle::Anim_bitmap_id_fire)); //Bitmap
}

// level init
// (if this function is modified, check that its use in supernova_stop() is still valid)
void supernova_level_init()
{
	Supernova_time_total = -1.0f;
	Supernova_time_left = -1.0f;
	Supernova_timestamp = TIMESTAMP::invalid();
	Supernova_fade_to_white_timestamp = TIMESTAMP::invalid();
	Supernova_particle_timestamp = TIMESTAMP::immediate();

	static particle::ParticleEffectHandle supernova_particle = supernova_init_particle();
	Supernova_particle_effect = supernova_particle;

	Supernova_status = SUPERNOVA_STAGE::NONE;
}

// start a supernova
void supernova_start(int seconds)
{
	// bogus time
	if((float)seconds < SUPERNOVA_HIT_TIME) {
		return;
	}

	// no supernova in multiplayer
	if(Game_mode & GM_MULTIPLAYER) {
		return;
	}

	Supernova_time_total = (float)seconds;
	Supernova_time_left = (float)seconds;
	Supernova_timestamp = _timestamp(seconds * MILLISECONDS_PER_SECOND);

	Supernova_status = SUPERNOVA_STAGE::STARTED;
}

void supernova_stop()
{
	// There's no currently active supernova
	if (Supernova_status == SUPERNOVA_STAGE::NONE)
		return;
	
	// We're too late.
	if (Supernova_status >= SUPERNOVA_STAGE::HIT)
		return;

	// A supernova? In MY multiplayer?
	if (Game_mode & GM_MULTIPLAYER)
		return;

	supernova_level_init();
}


int sn_particles = 100;
DCF_INT2(sn_part, sn_particles, 0, INT_MAX, "Sets number of supernova particles (default is 100)");

void supernova_do_particles()
{
	int idx;
	vec3d a, b;
	vec3d norm, sun_temp;

	// no player ship
	if((Player_obj == NULL) || (Player_ship == NULL)) {
		return;
	}

	// timestamp
	if (timestamp_elapsed(Supernova_particle_timestamp)) {
		Supernova_particle_timestamp = _timestamp(sn_particles);

		// get particle norm
		stars_get_sun_pos(0, &sun_temp);
		vm_vec_add2(&sun_temp, &Player_obj->pos);
		vm_vec_sub(&norm, &Player_obj->pos, &sun_temp);
		vm_vec_normalize(&norm);

		matrix orient;
		vm_vector_2_matrix_norm(&orient, &norm);

		// emit
		for(idx=0; idx<10; idx++) {
			vec3d ta = submodel_get_random_point(Ship_info[Player_ship->ship_info_index].model_num, 0);
			vec3d tb = submodel_get_random_point(Ship_info[Player_ship->ship_info_index].model_num, 0);

			// rotate into world space
			vm_vec_unrotate(&a, &ta, &Player_obj->orient);
			vm_vec_add2(&a, &Player_obj->pos);

			vec3d vel = norm;
			vm_vec_scale(&vel, 30.0f);
			vm_vec_add2(&vel, &Player_obj->phys_info.vel);

			vm_vec_unrotate(&b, &tb, &Player_obj->orient);
			vm_vec_add2(&b, &Player_obj->pos);

			auto source = particle::ParticleManager::get()->createSource(Supernova_particle_effect);
			source->setHost(make_unique<EffectHostVector>(a, orient, vel));
			source->finishCreation();

			auto source2 = particle::ParticleManager::get()->createSource(Supernova_particle_effect);
			source2->setHost(make_unique<EffectHostVector>(b, orient, vel));
			source2->finishCreation();
		}
	}
}

// call once per frame
float sn_shudder = 0.45f;
DCF_FLOAT2(sn_shud, sn_shudder, 0.0f, FLT_MAX, "Sets camera shudder rate for being in supernova shockwave (default is 0.45)");

void supernova_process()
{
	if (Supernova_status != SUPERNOVA_STAGE::NONE)
		Supernova_time_left = i2fl(timestamp_until(Supernova_timestamp)) / MILLISECONDS_PER_SECOND;

	switch (Supernova_status)
	{
		case SUPERNOVA_STAGE::NONE:
			break;

		case SUPERNOVA_STAGE::STARTED:
			if (Supernova_time_left <= SUPERNOVA_CLOSE_TIME)
			{
				snd_play(gamesnd_get_game_sound(GameSounds::SUPERNOVA_1), 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY);
				Supernova_status = SUPERNOVA_STAGE::CLOSE;
			}
			break;

		case SUPERNOVA_STAGE::CLOSE:
			if (Supernova_time_left <= SUPERNOVA_HIT_TIME)
			{
				snd_play(gamesnd_get_game_sound(GameSounds::SUPERNOVA_2), 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY);
				Supernova_status = SUPERNOVA_STAGE::HIT;

				// if we've crossed from stage 1 to stage 2 kill all particles and stick a bunch on the player ship
				particle::kill_all();	// kill all active particles so we have a bunch of free ones
			}
			break;

		case SUPERNOVA_STAGE::HIT:
			supernova_do_particles();
			if (Supernova_time_left <= (SUPERNOVA_HIT_TIME - SUPERNOVA_CAMERA_MOVE_DURATION))
			{
				Supernova_status = SUPERNOVA_STAGE::TOOLTIME;
			}
			break;

		case SUPERNOVA_STAGE::TOOLTIME:
			supernova_do_particles();
			// if the timestamp is done, so is the supernova
			if (timestamp_elapsed(Supernova_timestamp))
			{
				Supernova_fade_to_white_timestamp = _timestamp(fl2i(SUPERNOVA_FADE_TO_WHITE_DURATION * MILLISECONDS_PER_SECOND));
				Supernova_status = SUPERNOVA_STAGE::DEAD1;
			}
			break;

		case SUPERNOVA_STAGE::DEAD1:
			supernova_do_particles();
			// start the dead popup
			if (timestamp_elapsed(Supernova_fade_to_white_timestamp))
			{
				// main freespace 2 campaign? if so - end it now
				//
				// don't actually check for a specific campaign here since others may want to end this way but we
				// should test positive here if in campaign mode and sexp_end_campaign() got called - taylor
				if (Campaign_ending_via_supernova && (Game_mode & GM_CAMPAIGN_MODE) /*&& !stricmp(Campaign.filename, "freespace2")*/) {
					gameseq_post_event(GS_EVENT_END_CAMPAIGN);
				} else {
					popupdead_start();
				}
				Supernova_status = SUPERNOVA_STAGE::DEAD2;
			}
			break;

		case SUPERNOVA_STAGE::DEAD2:
			// popup etc. was already handled when the state switched, so nothing else to do
			break;
	}
}

// is there a supernova active
bool supernova_active()
{
	return Supernova_status != SUPERNOVA_STAGE::NONE;
}

SUPERNOVA_STAGE supernova_stage()
{
	return Supernova_status;
}

float supernova_hud_time_left()
{
	auto time_left = Supernova_time_left;
	if (Supernova_hits_at_zero)
		time_left -= SUPERNOVA_HIT_TIME;
	return MAX(time_left, 0.0f);
}

float supernova_pct_complete()
{
	// bogus
	if(!supernova_active()) {
		return -1.0f;
	}

	return (Supernova_time_total - Supernova_time_left) / Supernova_time_total;
}

float supernova_sunspot_pct()
{
	return 1.0f - (Supernova_time_left / SUPERNOVA_HIT_TIME);
}

// if the camera should cut to the "you-are-toast" cam
bool supernova_camera_cut()
{
	return Supernova_status >= SUPERNOVA_STAGE::HIT;
}

// get view params from supernova
float sn_distance = 300.0f;				// shockwave moving at 1000/ms ?
float sn_cam_distance = 25.0f;
DCF_FLOAT2(sn_dist, sn_distance, 0.0f, FLT_MAX, "Sets supernova shockwave distance (default is 300.0f)");


DCF_FLOAT2(sn_cam_dist, sn_cam_distance, 0.0f, FLT_MAX, "Sets supernova camera distance (default is 25.0f)");

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
	if (Supernova_status >= SUPERNOVA_STAGE::TOOLTIME) {
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
		float pct = ((SUPERNOVA_HIT_TIME - Supernova_time_left) / SUPERNOVA_CAMERA_MOVE_DURATION);
		vm_vec_scale_add2(&at, &move, sn_distance * pct);

		vm_vec_normalized_dir(&view, &at, &Supernova_camera_pos);
		vm_vector_2_matrix_norm(&Supernova_camera_orient, &view, nullptr, nullptr);
		//cam->set_rotation(&Supernova_camera_orient);
		*eye_orient = Supernova_camera_orient;
	}
	//return supernova_camera;
}
