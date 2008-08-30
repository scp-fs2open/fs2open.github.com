/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Starfield/Supernova.cpp $
 * $Revision: 2.8.2.4 $
 * $Date: 2007-09-02 19:05:59 $
 * $Author: Goober5000 $
 *
 * Include file for nebula stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.8.2.3  2007/05/14 23:13:43  Goober5000
 * --grouped the shake/shudder code together a bit better
 * --added a sexp to generate shudder
 * --fixed a minor bug in lock-perspective
 *
 * Revision 2.8.2.2  2007/02/20 04:19:42  Goober5000
 * the great big duplicate model removal commit
 *
 * Revision 2.8.2.1  2006/08/19 04:38:47  taylor
 * maybe optimize the (PI/2), (PI*2) and (RAND_MAX/2) stuff a little bit
 *
 * Revision 2.8  2006/05/27 16:42:16  taylor
 * fix slight freakiness with debris vclips (un)loading
 * comment out some code which was only used if neither D3D or OGL
 * fix dumb particle_emit() calls
 *
 * Revision 2.7  2006/01/30 06:31:30  taylor
 * dynamic starfield bitmaps (if the thought it was freaky before, just take a look at the new and "improved" version ;))
 *
 * Revision 2.6  2005/07/02 19:36:04  taylor
 * some supernova fixing
 *
 * Revision 2.5  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.4  2004/07/26 20:47:53  Kazan
 * remove MCD complete
 *
 * Revision 2.3  2004/07/12 16:33:07  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.2  2004/03/05 09:02:07  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     9/09/99 11:40p Dave
 * Handle an Assert() in beam code. Added supernova sounds. Play the right
 * 2 end movies properly, based upon what the player did in the mission.
 * 
 * 4     9/03/99 1:32a Dave
 * CD checking by act. Added support to play 2 cutscenes in a row
 * seamlessly. Fixed super low level cfile bug related to files in the
 * root directory of a CD. Added cheat code to set campaign mission # in
 * main hall.
 * 
 * 3     7/31/99 4:15p Dave
 * Fixed supernova particle velocities. Handle OBJ_NONE in target
 * monitoring view. Properly use objectives notify gauge colors.
 * 
 * 2     7/21/99 8:10p Dave
 * First run of supernova effect.
 *  
 *
 * $NoKeywords: $
 */

#include "starfield/supernova.h"
#include "freespace2/freespace.h"
#include "math/vecmat.h"
#include "starfield/starfield.h"
#include "particle/particle.h"
#include "ship/ship.h"
#include "io/timer.h"
#include "popup/popupdead.h"
#include "mission/missioncampaign.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"


// --------------------------------------------------------------------------------------------------------------------------
// SUPERNOVA DEFINES/VARS
//

// supernova time 1
#define SUPERNOVA_SOUND_1_TIME					15.0f
#define SUPERNOVA_SOUND_2_TIME					5.0f
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
	if((float)seconds < SUPERNOVA_CUT_TIME){
		return;
	}

	// no supernova in multiplayer
	if(Game_mode & GM_MULTIPLAYER){
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

int sn_particles = 100;
DCF(sn_part, "")
{
	dc_get_arg(ARG_INT);
	sn_particles = Dc_arg_int;
}
void supernova_do_particles()
{	
	int idx;
	vec3d a, b, ta, tb;
	vec3d norm, sun_temp;

	// no player ship
	if((Player_obj == NULL) || (Player_ship == NULL)){
		return;
	}

	// timestamp
	if((Supernova_particle_stamp == -1) || timestamp_elapsed(Supernova_particle_stamp)){
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
		for(idx=0; idx<10; idx++){			
			submodel_get_two_random_points(Ship_info[Player_ship->ship_info_index].model_num, 0, &ta, &tb);

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
DCF(sn_shud, "")
{
	dc_get_arg(ARG_FLOAT);
	sn_shudder = Dc_arg_float;
}

void supernova_process()
{	
	int sn_stage;	

	// if the supernova is running
	sn_stage = supernova_active();
	if(sn_stage){
		Supernova_time -= flFrametime;

		// sound stuff
		if((Supernova_time <= SUPERNOVA_SOUND_1_TIME) && !Supernova_sound_1_played){
			Supernova_sound_1_played = 1;
			snd_play(&Snds[SND_SUPERNOVA_1], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY);
		}
		if((Supernova_time <= SUPERNOVA_SOUND_2_TIME) && !Supernova_sound_2_played){
			Supernova_sound_2_played = 1;
			snd_play(&Snds[SND_SUPERNOVA_2], 0.0f, 1.0f, SND_PRIORITY_MUST_PLAY);
		}

		// if we've crossed from stage 1 to stage 2 kill all particles and stick a bunch on the player ship
		if((sn_stage == 1) && (supernova_active() == 2)){
			// first kill all active particles so we have a bunch of free ones
			particle_kill_all();				
		}		

		// if we're in stage 2, emit particles
		if((sn_stage >= 2) && (sn_stage != 5)){
			supernova_do_particles();
		}

		// if we've got negative. the supernova is done
		if(Supernova_time < 0.0f){
			Supernova_finished = 1;
			Supernova_fade_to_white += flFrametime;

			// start the dead popup
			if(Supernova_fade_to_white >= SUPERNOVA_FADE_TO_WHITE_TIME){
				if(!Supernova_popup){
					// main freespace 2 campaign? if so - end it now
					//
					// don't actually check for a specific campaign here since others may want to end this way but we
					// should test positive here if in campaign mode and sexp_end_campaign() got called - taylor
					if((Game_mode & GM_CAMPAIGN_MODE) /*&& !stricmp(Campaign.filename, "freespace2")*/ && Campaign_ended_in_mission){
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
	if(Supernova_finished == 1){
		Supernova_status = SUPERNOVA_HIT;
		return 4;
	}
	if(Supernova_finished == 2){
		Supernova_status = SUPERNOVA_HIT;
		return 5;
	}

	// no supernova
	if( (Supernova_time_total <= 0.0f) || (Supernova_time <= 0.0f) ){
		return 0;
	}	

	// final stage, 
	if(Supernova_time < (SUPERNOVA_CUT_TIME - SUPERNOVA_CAMERA_MOVE_TIME)){		
		Supernova_status = SUPERNOVA_HIT;
		return 3;
	}	

	// 2nd stage
	if(Supernova_time < SUPERNOVA_CUT_TIME){
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
	if(!supernova_active()){
		return -1.0f;
	}

	return (Supernova_time_total - Supernova_time) / Supernova_time_total;
}

// if the camera should cut to the "you-are-toast" cam
int supernova_camera_cut()
{
	// if we're not in a supernova
	if(!supernova_active()){
		return 0;
	}

	// if we're past the critical time
	if(Supernova_time <= SUPERNOVA_CUT_TIME){
		return 1;
	}

	// no cut yet
	return 0;
}

// get view params from supernova
float sn_distance = 300.0f;				// shockwave moving at 1000/ms ?
float sn_cam_distance = 25.0f;
DCF(sn_dist, "")
{
	dc_get_arg(ARG_FLOAT);
	sn_distance = Dc_arg_float;
}
DCF(sn_cam_dist, "")
{
	dc_get_arg(ARG_FLOAT);
	sn_cam_distance = Dc_arg_float;
}
/*
camid supernova_get_camera()
{
	static camid supernova_camera;
	if(!supernova_camera.isValid())
	{
		supernova_camera = cam_create("Supernova camera");
	}

	return supernova_camera;
}
*/
void supernova_get_eye(vec3d *eye_pos, matrix *eye_orient)
{
	// supernova camera pos
	vec3d Supernova_camera_pos;
	static matrix Supernova_camera_orient;
	/*
	static camid supernova_camera;
	if(!supernova_camera.isValid())
	{
		supernova_camera = cam_create("Supernova camera");
	}

	if(!supernova_camera.isValid())
		return supernova_camera;

	camera *cam = supernova_camera.getCamera();
	*/

	vec3d at;
	vec3d sun_temp, sun;
	vec3d view;
	
	// set the controls for the heart of the sun	
	stars_get_sun_pos(0, &sun_temp);
	vm_vec_add2(&sun_temp, &Player_obj->pos);
	vm_vec_sub(&sun, &sun_temp, &Player_obj->pos);
	vm_vec_normalize(&sun);

	// always set the camera pos
	vec3d move;
	matrix whee;
	vm_vector_2_matrix(&whee, &move, NULL, NULL);
	vm_vec_scale_add(&Supernova_camera_pos, &Player_obj->pos, &whee.vec.rvec, sn_cam_distance);
	vm_vec_scale_add2(&Supernova_camera_pos, &whee.vec.uvec, 30.0f);
	//cam->set_position(&Supernova_camera_pos);
	*eye_pos = Supernova_camera_pos;

	// if we're no longer moving the camera
	if(Supernova_time < (SUPERNOVA_CUT_TIME - SUPERNOVA_CAMERA_MOVE_TIME)){
		// *eye_pos = Supernova_camera_pos;
		//cam->set_rotation(&Supernova_camera_orient);
		*eye_orient = Supernova_camera_orient;
	} 
	// otherwise move it
	else {
		// get a vector somewhere between the supernova shockwave and the player ship	
		at = Player_obj->pos;
		vm_vec_scale_add2(&at, &sun, sn_distance);
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
