/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/ShipFX.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:10 $
 * $Author: penguin $
 *
 * Routines for ship effects (as in special)
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.5  2002/05/13 21:43:38  mharris
 * A little more network and sound cleanup
 *
 * Revision 1.4  2002/05/10 20:42:45  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.3  2002/05/10 06:08:08  mharris
 * Porting... added ifndef NO_SOUND
 *
 * Revision 1.2  2002/05/03 22:07:10  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 51    9/13/99 4:53p Andsager
 * 
 * 50    9/13/99 10:09a Andsager
 * Add debug console commands to lower model render detail and fireball
 * LOD for big ship explosiosns.
 * 
 * 49    9/08/99 10:44p Andsager
 * Make HUGE ships not die when warping out, after warp effect started.
 * 
 * 48    9/06/99 10:16p Andsager
 * Modify big ship explosions.  Less frequent fireballs, fewer particles,
 * larger fireballs
 * 
 * 47    9/05/99 11:10a Andsager
 * Fix up which split ship get debris pieces.  Dont render debris pieces
 * for split ships until they are near clip plane.
 * 
 * 46    9/02/99 12:55a Mikek
 * Scale engine wash by speed.
 * 
 * 45    9/01/99 10:15a Dave
 * 
 * 44    8/31/99 10:13p Andsager
 * Add Knossos warp effect fireball
 * 
 * 43    8/23/99 11:09a Andsager
 * Round 2 of Knossos explosion
 * 
 * 42    8/20/99 2:16p Andsager
 * Make only supercaps slow down extra fast after warping in.
 * 
 * 41    8/20/99 1:42p Andsager
 * Frist pass on Knossos explosion.
 * 
 * 40    8/19/99 4:36p Andsager
 * Cleaned up special_warpout
 * 
 * 39    8/16/99 10:04p Andsager
 * Add special-warp-dist and special-warpout-name sexp for Knossos device
 * warpout.
 * 
 * 38    8/16/99 2:01p Andsager
 * Knossos warp-in warp-out.
 * 
 * 37    8/13/99 10:49a Andsager
 * Knossos and HUGE ship warp out.  HUGE ship warp in.  Stealth search
 * modes dont collide big ships.
 * 
 * 36    8/05/99 2:06a Dave
 * Whee.
 * 
 * 35    7/18/99 12:32p Dave
 * Randomly oriented shockwaves.
 * 
 * 34    7/09/99 12:52a Andsager
 * Modify engine wash (1) less damage (2) only at a closer range (3) no
 * damage when engine is disabled
 * 
 * 33    7/06/99 10:45a Andsager
 * Modify engine wash to work on any ship that is not small.  Add AWACS
 * ask for help.
 * 
 * 32    7/02/99 9:55p Dave
 * Player engine wash sound.
 * 
 * 31    7/02/99 4:31p Dave
 * Much more sophisticated lightning support.
 * 
 * 30    7/01/99 4:23p Dave
 * Full support for multiple linked ambient engine sounds. Added "big
 * damage" flag.
 * 
 * 29    7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 28    6/17/99 12:06p Andsager
 * Add a fireball for each live debris piece.
 * 
 * 27    6/14/99 3:21p Andsager
 * Allow collisions between ship and its debris.  Fix up collision pairs
 * when large ship is warping out.
 * 
 * 26    5/27/99 12:14p Andsager
 * Some fixes for live debris when more than one subsys on ship with live
 * debris.  Set subsys strength (when 0) blows off subsystem.
 * sexp_hits_left_subsystem works for SUBSYSTEM_UNKNOWN.
 * 
 * 25    5/26/99 11:46a Dave
 * Added ship-blasting lighting and made the randomization of lighting
 * much more customizable.
 * 
 * 24    5/25/99 10:05a Andsager
 * Fix assert with wing warping out with no warp effect.
 * 
 * 23    5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * 22    5/21/99 5:03p Andsager
 * Add code to display engine wash death.  Modify ship_kill_packet
 * 
 * 21    5/19/99 11:09a Andsager
 * Turn on engine wash.  Check every 1/4 sec.
 * 
 * 20    5/18/99 1:30p Dave
 * Added muzzle flash table stuff.
 * 
 * 19    5/17/99 10:33a Dave
 * Fixed warning.
 * 
 * 18    5/11/99 10:16p Andsager
 * First pass on engine wash effect.  Rotation (control input), damage,
 * shake.  
 * 
 * 17    5/09/99 6:00p Dave
 * Lots of cool new effects. E3 build tweaks.
 * 
 * 16    4/28/99 11:13p Dave
 * Temporary checkin of artillery code.
 * 
 * 15    3/29/99 6:17p Dave
 * More work on demo system. Got just about everything in except for
 * blowing ships up, secondary weapons and player death/warpout.
 * 
 * 14    3/28/99 5:58p Dave
 * Added early demo code. Make objects move. Nice and framerate
 * independant, but not much else. Don't use yet unless you're me :)
 * 
 * 13    3/23/99 2:29p Andsager
 * Fix shockwaves for kamikazi and Fred defined.  Collect together
 * shockwave_create_info struct.
 * 
 * 12    3/20/99 3:03p Andsager
 * Fix get_model_cross_section_at_z() from getting invalid index .
 * 
 * 11    3/19/99 9:51a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 10    2/26/99 4:14p Dave
 * Put in the ability to have multiple shockwaves for ships.
 * 
 * 9     1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 8     1/28/99 9:10a Andsager
 * Particles increased in width, life, number.  Max particles increased
 * 
 * 7     1/27/99 9:56a Dave
 * Temporary checkin of beam weapons for Dan to make cool sounds.
 * 
 * 6     1/11/99 12:42p Andsager
 * Add live debris - debris which is created from a destroyed subsystem,
 * when the ship is still alive
 * 
 * 5     11/18/98 10:29a Johnson
 * fix assert in shipfx_remove_submodel_ship_sparks.  submodel may not
 * have spark and be destroyed.
 * 
 * 4     11/17/98 4:27p Andsager
 * Stop sparks from emitting from destroyed subobjects
 * 
 * 3     10/20/98 1:39p Andsager
 * Make so sparks follow animated ship submodels.  Modify
 * ship_weapon_do_hit_stuff() and ship_apply_local_damage() to add
 * submodel_num.  Add submodel_num to multiplayer hit packet.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "ship/ship.h"
#include "fireball/fireballs.h"
#include "debris/debris.h"
#include "hud/hudtarget.h"
#include "ship/shipfx.h"
#include "gamesnd/gamesnd.h"
#include "io/timer.h"
#include "render/3d.h"			// needed for View_position, which is used when playing a 3D sound
#include "hud/hud.h"
#include "math/fvi.h"
#include "gamesequence/gamesequence.h"
#include "lighting/lighting.h"
#include "globalincs/linklist.h"
#include "particle/particle.h"
#include "bmpman/bmpman.h"
#include "freespace2/freespace.h"
#include "weapon/muzzleflash.h"
#include "demo/demo.h"
#include "ship/shiphit.h"
#include "nebula/neblightning.h"
#include "object/objectsnd.h"
#include "playerman/player.h"

#ifndef NO_NETWORK
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#endif

#ifndef NDEBUG
extern float flFrametime;
extern int Framecount;
#endif

#define SHIP_CANNON_BITMAP			"argh"
int Ship_cannon_bitmap = -1;

int Player_engine_wash_loop = -1;

void shipfx_remove_submodel_ship_sparks(ship *shipp, int submodel_num)
{
	Assert(submodel_num != -1);

	// maybe no active sparks on submodel
	if (shipp->num_hits == 0) {
		return;
	}
	
	for (int spark_num=0; spark_num<shipp->num_hits; spark_num++) {
		if (shipp->sparks[spark_num].submodel_num == submodel_num) {
			shipp->sparks[spark_num].end_time = timestamp(1);
		}
	}
}

// Check if subsystem has live debris and create
// DKA: 5/26/99 make velocity of debris scale according to size of debris subobject (at least for large subobjects)
void shipfx_subsystem_mabye_create_live_debris(object *ship_obj, ship *ship_p, ship_subsys *subsys, vector *exp_center, float exp_mag)
{
	// initializations
	polymodel *pm = model_get(ship_p->modelnum);
	int submodel_num = subsys->system_info->subobj_num;
	submodel_instance_info *sii = &subsys->submodel_info_1;

	object *live_debris_obj;
	int i, num_live_debris, live_debris_submodel;

	// get number of live debris objects to create
	num_live_debris = pm->submodel[submodel_num].num_live_debris;
	if (num_live_debris <= 0) {
		return;
	}

	ship_model_start(ship_obj);

	// copy angles
	angles copy_angs = pm->submodel[submodel_num].angs;
	angles zero_angs = {0.0f, 0.0f, 0.0f};

	// make sure the axis point is set
	if ( !sii->axis_set ) {
		model_init_submodel_axis_pt(sii, ship_p->modelnum, submodel_num);
	}

	// get the rotvel
	vector model_axis, world_axis, rotvel, world_axis_pt;
	void model_get_rotating_submodel_axis(vector *model_axis, vector *world_axis, int modelnum, int submodel_num, object *obj);
	model_get_rotating_submodel_axis(&model_axis, &world_axis, ship_p->modelnum, submodel_num, ship_obj);
	vm_vec_copy_scale(&rotvel, &world_axis, sii->cur_turn_rate);

	model_find_world_point(&world_axis_pt, &sii->pt_on_axis, ship_p->modelnum, submodel_num, &ship_obj->orient, &ship_obj->pos);

	matrix m_rot;	// rotation for debris orient about axis
	vm_quaternion_rotate(&m_rot, vm_vec_mag((vector*)&sii->angs), &model_axis);


	// create live debris pieces
	for (i=0; i<num_live_debris; i++) {
		live_debris_submodel = pm->submodel[submodel_num].live_debris[i];
		vector start_world_pos, start_model_pos, end_world_pos;

		// get start world pos
		vm_vec_zero(&start_world_pos);
		model_find_world_point(&start_world_pos, &pm->submodel[live_debris_submodel].offset, ship_p->modelnum, live_debris_submodel, &ship_obj->orient, &ship_obj->pos );

		// convert to model coord of underlying submodel
		// set angle to zero
		pm->submodel[submodel_num].angs = zero_angs;
		world_find_model_point(&start_model_pos, &start_world_pos, pm, submodel_num, &ship_obj->orient, &ship_obj->pos);

		// rotate from submodel coord to world coords
		// reset angle to current angle
		pm->submodel[submodel_num].angs = copy_angs;
		model_find_world_point(&end_world_pos, &start_model_pos, ship_p->modelnum, submodel_num, &ship_obj->orient, &ship_obj->pos);

		// create fireball here.
		fireball_create(&end_world_pos, FIREBALL_EXPLOSION_MEDIUM, OBJ_INDEX(ship_obj), pm->submodel[live_debris_submodel].rad);

		// create debris
		live_debris_obj = debris_create(ship_obj, ship_p->modelnum, live_debris_submodel, &end_world_pos, exp_center, 1, exp_mag);

		// only do if debris is created
		if (live_debris_obj) {
			// get radial velocity of debris
			vector delta_x, radial_vel;
			vm_vec_sub(&delta_x, &end_world_pos, &world_axis_pt);
			vm_vec_crossprod(&radial_vel, &rotvel, &delta_x);

			if (Ship_info[ship_p->ship_info_index].flags & SIF_KNOSSOS_DEVICE) {
				// set velocity to cross center of knossos device
				vector rand_vec, vec_to_center;

				float vel_mag = vm_vec_mag_quick(&radial_vel) * 1.3f * (0.9f + 0.2f*frand());
				vm_vec_normalized_dir(&vec_to_center, &world_axis_pt, &end_world_pos);
				vm_vec_rand_vec_quick(&rand_vec);
				vm_vec_scale_add2(&vec_to_center, &rand_vec, 0.2f);
				vm_vec_scale_add2(&live_debris_obj->phys_info.vel, &vec_to_center, vel_mag);

			} else {
				// Get rotation of debris object
				matrix copy = live_debris_obj->orient;
				vm_matrix_x_matrix(&live_debris_obj->orient, &copy, &m_rot);

				// Add radial velocity (at least as large as exp velocity)
				vector temp_vel;	// explosion velocity with ship_obj velocity removed
				vm_vec_sub(&temp_vel, &live_debris_obj->phys_info.vel, &ship_obj->phys_info.vel);

				// find magnitudes of radial and temp velocity
				float vel_mag = vm_vec_mag(&temp_vel);
				float rotvel_mag = vm_vec_mag(&radial_vel);

				if (rotvel_mag > 0.1) {
					float scale = (1.2f + 0.2f * frand()) * vel_mag / rotvel_mag;
					// always add *at least* rotvel
					if (scale < 1) {
						scale = 1.0f;
					}

					if (exp_mag > 1) {	// whole ship going down
						scale = exp_mag;
					}

					if (Ship_info[ship_p->ship_info_index].flags & SIF_KNOSSOS_DEVICE) {
						scale = 1.0f;
					}

					vm_vec_scale_add2(&live_debris_obj->phys_info.vel, &radial_vel, scale);
				}

				// scale up speed of debris if ship_obj > 125, but not for knossos
				if (ship_obj->radius > 250 && !(Ship_info[ship_p->ship_info_index].flags & SIF_KNOSSOS_DEVICE)) {
					vm_vec_scale(&live_debris_obj->phys_info.vel, ship_obj->radius/250.0f);
				}
			}
		}
	}

	ship_model_stop(ship_obj);
}

void set_ship_submodel_as_blown_off(ship *shipp, char *name)
{
	int found =	FALSE;

	// go through list of ship subsystems and find name
	ship_subsys	*pss = NULL;
	for (pss=GET_FIRST(&shipp->subsys_list); pss!=END_OF_LIST(&shipp->subsys_list); pss=GET_NEXT(pss)) {
		if ( stricmp(pss->system_info->name, name) == 0) {
			found = TRUE;
			break;
		}
	}

	// set its blown off flag to TRUE
	Assert(found);
	if (found) {
		pss->submodel_info_1.blown_off = 1;
	}
}


// Create debris for ship submodel which has live debris (at ship death)
// when ship submodel has not already been blown off (and hence liberated live debris)
void shipfx_maybe_create_live_debris_at_ship_death( object *ship_obj )
{
	// if ship has live debris, detonate that subsystem now
	// search for any live debris

	ship *shipp = &Ships[ship_obj->instance];
	polymodel *pm = model_get(shipp->modelnum);

	int live_debris_submodel = -1;
	for (int idx=0; idx<pm->num_debris_objects; idx++) {
		if (pm->submodel[pm->debris_objects[idx]].is_live_debris) {
			live_debris_submodel = pm->debris_objects[idx];

			// get submodel that produces live debris
			int model_get_parent_submodel_for_live_debris( int model_num, int live_debris_model_num );
			int parent = model_get_parent_submodel_for_live_debris( shipp->modelnum, live_debris_submodel);
			Assert(parent != -1);

			// set model values only once (esp blown off)
			ship_model_start(ship_obj);

			// check if already blown off  (ship model set)
			if ( !pm->submodel[parent].blown_off ) {
		
				// get ship_subsys for live_debris
				// Go through all subsystems and look for submodel the subsystems with "parent" submodel.
				ship_subsys	*pss = NULL;
				for ( pss = GET_FIRST(&shipp->subsys_list); pss != END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
					if (pss->system_info->subobj_num == parent) {
						break;
					}
				}

				Assert (pss != NULL);
				if (pss != NULL) {
					vector exp_center, tmp = {0.0f, 0.0f, 0.0f};
					model_find_world_point(&exp_center, &tmp, shipp->modelnum, parent, &ship_obj->orient, &ship_obj->pos );

					// if not blown off, blow it off
					shipfx_subsystem_mabye_create_live_debris(ship_obj, shipp, pss, &exp_center, 3.0f);

					// now set subsystem as blown off, so we only get one copy
					pm->submodel[parent].blown_off = 1;
					set_ship_submodel_as_blown_off(&Ships[ship_obj->instance], pss->system_info->name);
				}
			}
		}
	}

	// clean up
	ship_model_stop(ship_obj);

}

void shipfx_blow_off_subsystem(object *ship_obj,ship *ship_p,ship_subsys *subsys, vector *exp_center)
{
	vector subobj_pos;

	model_subsystem	*psub = subsys->system_info;

	get_subsystem_world_pos(ship_obj, subsys, &subobj_pos);

/*
	if ( psub->turret_gun_sobj > -1 )
		debris_create( ship_obj, ship_p->modelnum, psub->turret_gun_sobj, &subobj_pos, exp_center, 0, 1.0f );

	if ( psub->subobj_num > -1 )
		debris_create( ship_obj, ship_p->modelnum, psub->subobj_num, &subobj_pos, exp_center, 0, 1.0f );
*/
	// get rid of sparks on submodel that is destroyed
	shipfx_remove_submodel_ship_sparks(ship_p, psub->subobj_num);

	// create debris shards
	shipfx_blow_up_model(ship_obj, ship_p->modelnum, psub->subobj_num, 50, &subobj_pos );

	// create live debris objects, if any
	// TODO:  some MULITPLAYER implcations here!!
	shipfx_subsystem_mabye_create_live_debris(ship_obj, ship_p, subsys, exp_center, 1.0f);

	// create first fireball
	fireball_create( &subobj_pos, FIREBALL_EXPLOSION_MEDIUM, OBJ_INDEX(ship_obj), psub->radius );
}


void shipfx_blow_up_hull(object *obj, int model, vector *exp_center)
{
	int i;
	polymodel * pm;
	ushort sig_save;


	pm = model_get(model);
	if (!pm) return;

	// in multiplayer, send a debris_hull_create packet.  Save/restore the debris signature
	// when in misison only (since we can create debris pieces before mission starts)
	sig_save = 0;
#ifndef NO_NETWORK
	if ( (Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_IN_MISSION) ) {
		sig_save = multi_get_next_network_signature( MULTI_SIG_DEBRIS );
		multi_set_network_signature( (ushort)(Ships[obj->instance].arrival_distance), MULTI_SIG_DEBRIS );
	}
#endif

	bool try_live_debris = true;
	for (i=0; i<pm->num_debris_objects; i++ )	{
		if (! pm->submodel[pm->debris_objects[i]].is_live_debris) {
			vector tmp = {0.0f, 0.0f, 0.0f };		
			model_find_world_point(&tmp, &pm->submodel[pm->debris_objects[i]].offset, model, 0, &obj->orient, &obj->pos );
			debris_create( obj, model, pm->debris_objects[i], &tmp, exp_center, 1, 3.0f );
		} else {
			if ( try_live_debris ) {
				// only create live debris once
				// this creates *all* the live debris for *all* the currently live subsystems.
				try_live_debris = false;
				shipfx_maybe_create_live_debris_at_ship_death(obj);
			}
		}
	}

#ifndef NO_NETWORK
	// restore the ship signature to it's original value.
	if ( (Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_IN_MISSION) ) {
		multi_set_network_signature( sig_save, MULTI_SIG_DEBRIS );
	}
#endif
}


// Creates "ndebris" pieces of debris on random verts of the the "submodel" in the 
// ship's model.
void shipfx_blow_up_model(object *obj,int model, int submodel, int ndebris, vector *exp_center )
{
	int i;

	// if in a multiplayer game -- seed the random number generator with a value that will be the same
	// on all clients in the game -- the net_signature of the object works nicely -- since doing so should
	// ensure that all pieces of debris will get scattered in same direction on all machines
	if ( Game_mode & GM_MULTIPLAYER )
		srand( obj->net_signature );

	// made a change to allow anyone but multiplayer client to blow up hull.  Clients will do it when
	// they get the create packet
	if ( submodel == 0 ) {
		shipfx_blow_up_hull(obj,model, exp_center );
	}

	for (i=0; i<ndebris; i++ )	{
		vector pnt1, pnt2;

		// Gets two random points on the surface of a submodel
		submodel_get_two_random_points(model, submodel, &pnt1, &pnt2 );

		vector tmp, outpnt;

		vm_vec_avg( &tmp, &pnt1, &pnt2 );
		model_find_world_point(&outpnt, &tmp, model,submodel, &obj->orient, &obj->pos );

		debris_create( obj, -1, -1, &outpnt, exp_center, 0, 1.0f );
	}
}


// =================================================
//          SHIP WARP IN EFFECT CODE
// =================================================


// Given an ship, find the radius of it as viewed from the front.
static float shipfx_calculate_effect_radius( object *objp )
{
	float w,h,rad;
	ship *shipp = &Ships[objp->instance];

	polymodel *pm = model_get( shipp->modelnum );

	w = pm->maxs.xyz.x - pm->mins.xyz.x;
	h = pm->maxs.xyz.y - pm->mins.xyz.y;
	
	if ( w > h )	{
		rad = w / 2.0f;
	} else {
		rad = h / 2.0f;
	}

	object *docked_objp = ai_find_docked_object( objp );

	//	If ship is docked then center wormhold about their center and make radius large enough.
	if ( docked_objp ) {
		ship *docked_shipp = &Ships[docked_objp->instance];
		
		pm = model_get( docked_shipp->modelnum );
		
		w = pm->maxs.xyz.x - pm->mins.xyz.x;
		h = pm->maxs.xyz.y - pm->mins.xyz.y;
		
		if ( w > h )	{
			rad += w / 2.0f;
		} else {
			rad += h / 2.0f;
		}
	}
	return rad*3.0f;
}

// How long the stage 1 & stage 2 of warp in effect lasts.
// There are different times for small, medium, and large ships.
// The appropriate values are picked depending on the ship's
// radius.
#define SHIPFX_WARP_DELAY	(2.0f)		// time for warp effect to ramp up before ship moves into it.

// Give object objp, calculate how long it should take the
// ship to go through the warp effect and how fast the ship
// should go.   For reference,  capital ship of 2780m 
// should take 7 seconds to fly through.   Fighters of 30, 
// should take 1.5 seconds to fly through.

#define LARGEST_RAD 1390.0f 
#define LARGEST_RAD_TIME 7.0f

#define SMALLEST_RAD 15.0f
#define SMALLEST_RAD_TIME 1.5f


static float shipfx_calculate_warp_time( object * objp )
{
	// Find rad_percent from 0 to 1, 0 being smallest ship, 1 being largest
	float rad_percent = (objp->radius-SMALLEST_RAD) / (LARGEST_RAD-SMALLEST_RAD);
	if ( rad_percent < 0.0f ) {
		rad_percent = 0.0f;
	} else if ( rad_percent > 1.0f )	{
		rad_percent = 1.0f;
	}
	float rad_time = rad_percent*(LARGEST_RAD_TIME-SMALLEST_RAD_TIME) + SMALLEST_RAD_TIME;

	return rad_time;
}

// calculate warp speed
float shipfx_calculate_warp_speed(object *objp)
{
	float length;

	Assert(objp->type == OBJ_SHIP);
	if (objp->type != OBJ_SHIP) {
		length = 2.0f * objp->radius;
	} else {
		length = ship_get_length(&Ships[objp->instance]);
	}
	return length / shipfx_calculate_warp_time(objp);
}


// This is called to actually warp this object in
// after all the flashy fx are done, or if the flashy 
// fx don't work for some reason.
void shipfx_actually_warpin( 	ship *shipp, object *objp )
{
	shipp->flags &= (~SF_ARRIVING_STAGE_1);
	shipp->flags &= (~SF_ARRIVING_STAGE_2);

	// let physics in on it too.
	objp->phys_info.flags &= (~PF_WARP_IN);
}

// JAS - code to start the ship doing the warp in effect
// This also starts the animating 3d effect playing.
// There are two modes, stage 1 and stage 2.   Stage 1 is
// when the ship just invisibly waits for a certain time
// period after the effect starts, and then stage 2 begins,
// where the ships flies through the effect at a set
// velocity so it gets through in a certain amount of
// time.
void shipfx_warpin_start( object *objp )
{
	ship *shipp;
	float effect_time, effect_radius;
	
	shipp = &Ships[objp->instance];

	if ( shipp->flags & SF_ARRIVING )	{
		mprintf(( "Ship is already arriving!\n" ));
		return;
	}

	// post a warpin event
	if(Game_mode & GM_DEMO_RECORD){
		demo_POST_warpin(objp->signature, shipp->flags);
	}

	// if there is no arrival warp, then skip the whole thing
	if ( shipp->flags & SF_NO_ARRIVAL_WARP )	{
		shipfx_actually_warpin(shipp,objp);
		return;
	}
	
	// VALIDATE special_warp_objnum
	if (shipp->special_warp_objnum >= 0) {
			
		int ref_objnum = shipp->special_warp_objnum;
		int valid_reference_ship = FALSE;

		// Validate reference_objnum
		if ((ref_objnum >= 0) && (ref_objnum < MAX_OBJECTS)) {
			object *sp_objp = &Objects[ref_objnum];
			if (sp_objp->type == OBJ_SHIP) {
				if (Ship_info[Ships[sp_objp->instance].ship_info_index].flags & SIF_KNOSSOS_DEVICE) {
					valid_reference_ship = TRUE;
				}
			}
		}

		if (valid_reference_ship != TRUE) {
			shipp->special_warp_objnum = -1;
		}
	}

	// only move warp effect pos if not special warp in.
	if (shipp->special_warp_objnum >= 0) {
		Assert(!(Game_mode & GM_MULTIPLAYER));
		polymodel *pm;
		pm = model_get(shipp->modelnum);
		vm_vec_scale_add(&shipp->warp_effect_pos, &objp->pos, &objp->orient.vec.fvec, -pm->mins.xyz.z);
	} else {
		vm_vec_scale_add( &shipp->warp_effect_pos, &objp->pos, &objp->orient.vec.fvec, objp->radius );
	}
	
	// The ending zero mean this is a warp-in effect
	if ( !(shipp->flags & SF_INITIALLY_DOCKED) ) {
		int warp_objnum;

		// Effect time is 'SHIPFX_WARP_DELAY' (1.5 secs) seconds to start, 'shipfx_calculate_warp_time' 
		// for ship to go thru, and 'SHIPFX_WARP_DELAY' (1.5 secs) to go away.
		effect_time = shipfx_calculate_warp_time(objp) + SHIPFX_WARP_DELAY + SHIPFX_WARP_DELAY;
		effect_radius = shipfx_calculate_effect_radius(objp);

		// maybe special warpin
		if (shipp->special_warp_objnum >= 0) {
			// cap radius to size of knossos
			effect_radius = min(effect_radius, 0.8f*Objects[shipp->special_warp_objnum].radius);
			warp_objnum = fireball_create(&shipp->warp_effect_pos, FIREBALL_KNOSSOS_EFFECT, shipp->special_warp_objnum, effect_radius, 0, NULL, effect_time, shipp->ship_info_index);
		} else {
			warp_objnum = fireball_create(&shipp->warp_effect_pos, FIREBALL_WARP_EFFECT, OBJ_INDEX(objp), effect_radius, 0, NULL, effect_time, shipp->ship_info_index);
		}
		if (warp_objnum < 0 )	{	// JAS: This must always be created, if not, just warp the ship in
			shipfx_actually_warpin(shipp,objp);
			return;
		}

		shipp->warp_effect_fvec = Objects[warp_objnum].orient.vec.fvec;
		// maybe negate if special warp effect
		if (shipp->special_warp_objnum >= 0) {
			if (vm_vec_dotprod(&shipp->warp_effect_fvec, &objp->orient.vec.fvec) < 0) {
				vm_vec_negate(&shipp->warp_effect_fvec);
			}
		}


		shipp->final_warp_time = timestamp(fl2i(SHIPFX_WARP_DELAY*1000.0f));
		shipp->flags |= SF_ARRIVING_STAGE_1;

		// see if this ship is docked with anything, and if so, make docked ship be "arriving"
		/*
		if ( Ai_info[shipp->ai_index].dock_objnum != -1 ) {
			Ships[Ai_info[shipp->ai_index].dock_objnum].final_warp_time = timestamp(fl2i(warp_time*1000.0f));
			Ships[Ai_info[shipp->ai_index].dock_objnum].flags |= SF_ARRIVING_STAGE_1;
		}
		*/
	}
}

void shipfx_warpin_frame( object *objp, float frametime )
{
	ship *shipp;

	shipp = &Ships[objp->instance];

	if ( shipp->flags & SF_DYING ) return;

	if ( shipp->flags & SF_ARRIVING_STAGE_1 )	{
		if ( timestamp_elapsed(shipp->final_warp_time) ) {

			// let physics know the ship is going to warp in.
			objp->phys_info.flags |= PF_WARP_IN;

			// done doing stage 1 of warp, so go on to stage 2
			shipp->flags &= (~SF_ARRIVING_STAGE_1);
			shipp->flags |= SF_ARRIVING_STAGE_2;

			float warp_time = shipfx_calculate_warp_time(objp);
			float speed = shipfx_calculate_warp_speed(objp);			// How long it takes to move through warp effect

			// Make ship move at velocity so that it moves two radius's in warp_time seconds.
			vector vel;
			vel = objp->orient.vec.fvec;
			vm_vec_scale( &vel, speed );
			objp->phys_info.vel = vel;
			objp->phys_info.desired_vel = vel;
			objp->phys_info.prev_ramp_vel.xyz.x = 0.0f;
			objp->phys_info.prev_ramp_vel.xyz.y = 0.0f;
			objp->phys_info.prev_ramp_vel.xyz.z = speed;
			objp->phys_info.forward_thrust = 0.0f;		// How much the forward thruster is applied.  0-1.

			shipp->final_warp_time = timestamp(fl2i(warp_time*1000.0f));

			/*
			// see if this ship is docked with anything, and if so, make docked ship be "arriving"
			if ( Ai_info[shipp->ai_index].dock_objnum != -1 ) {
				Ships[Ai_info[shipp->ai_index].dock_objnum].flags &= (~SF_ARRIVING_STAGE_1);
				Ships[Ai_info[shipp->ai_index].dock_objnum].flags |= SF_ARRIVING_STAGE_2;
				Ships[Ai_info[shipp->ai_index].dock_objnum].final_warp_time = timestamp(fl2i(warp_time*1000.0f));
			}
			*/
		} 
	} else if ( shipp->flags & SF_ARRIVING_STAGE_2 )	{
		if ( timestamp_elapsed(shipp->final_warp_time) ) {
			// done doing stage 2 of warp, so turn off arriving flag
			shipfx_actually_warpin(shipp,objp);

			// notify physics to slow down
			if (Ship_info[shipp->ship_info_index].flags & SIF_SUPERCAP) {
				// let physics know this is a special warp in
				objp->phys_info.flags |= PF_SPECIAL_WARP_IN;
			}
		}
	}

}
 
// This is called to actually warp this object out
// after all the flashy fx are done, or if the flashy 
// fx don't work for some reason.  OR to skip the flashy
// fx.
void shipfx_actually_warpout( ship *shipp, object *objp )
{
	// Once we get through effect, make the ship go away

	if ( objp == Player_obj )	{
		// Normally, this will never get called for the player. If it
		// does, it is because some error (like the warpout effect
		// couldn't start) so go ahead an warp the player out.
		// All this does is set the event to go to debriefing, the
		// same thing that happens after the player warp out effect
		// ends.
		gameseq_post_event( GS_EVENT_DEBRIEF );	// proceed to debriefing
	} else {
		object *docked_objp;
		
		// Code for objects except player ship warping out
		objp->flags |= OF_SHOULD_BE_DEAD;
		// check to see if departing ship is docked with anything and if so, mark that object
		// gone as well
		docked_objp = ai_find_docked_object( objp );
		if ( docked_objp ) {
			docked_objp->flags |= OF_SHOULD_BE_DEAD;
		}

		ship_departed( objp->instance );
		if ( docked_objp ){
			ship_departed( docked_objp->instance );
		}
	}
}

// compute_special_warpout_stuff();
int compute_special_warpout_stuff(object *objp, float *speed, float *warp_time, vector *warp_pos)
{
	object	*sp_objp = NULL;
	ship		*shipp;
	int		valid_refenence_ship = FALSE, ref_objnum;
	vector	facing_normal, vec_to_knossos;
	float		dist_to_plane;

	// knossos warpout only valid in single player
	if (Game_mode & GM_MULTIPLAYER) {
		mprintf(("special warpout only for single player\n"));
		return -1;
	}

	// find special warp ship reference
	valid_refenence_ship = FALSE;
	ref_objnum = Ships[objp->instance].special_warp_objnum;

	// Validate reference_objnum
	if ((ref_objnum >= 0) && (ref_objnum < MAX_OBJECTS)) {
		sp_objp = &Objects[ref_objnum];
		if (sp_objp->type == OBJ_SHIP) {
			shipp = &Ships[sp_objp->instance];
			if (Ship_info[shipp->ship_info_index].flags & SIF_KNOSSOS_DEVICE) {
				valid_refenence_ship = TRUE;
			}
		}
	}
	
	if (!valid_refenence_ship) {
		Int3();
		mprintf(("special warpout reference ship not found\n"));
		return -1;
	}

	// get facing normal from knossos
	vm_vec_sub(&vec_to_knossos, &sp_objp->pos, &objp->pos);
	facing_normal = sp_objp->orient.vec.fvec;
	if (vm_vec_dotprod(&vec_to_knossos, &sp_objp->orient.vec.fvec) > 0) {
		vm_vec_negate(&facing_normal);
	}

	// find position to play the warp ani..
	dist_to_plane = fvi_ray_plane(warp_pos, &sp_objp->pos, &facing_normal, &objp->pos, &objp->orient.vec.fvec, 0.0f);

	// calculate distance to warpout point.
	polymodel *pm = model_get(Ships[objp->instance].modelnum);
	dist_to_plane += pm->mins.xyz.z;

	if (dist_to_plane < 0) {
		mprintf(("warpout started too late\n"));
		return -1;
	}

	// validate angle
	float max_warpout_angle = 0.707f;	// 45 degree half-angle cone for small ships
	if (Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {
		max_warpout_angle = 0.866f;	// 30 degree half-angle cone for BIG or HUGE
	}

	if (-vm_vec_dotprod(&objp->orient.vec.fvec, &facing_normal) < max_warpout_angle) {	// within allowed angle
		Int3();
		mprintf(("special warpout angle exceeded\n"));
		return -1;
	}

	// Calculate speed needed to get 
	*speed = shipfx_calculate_warp_speed(objp);
	
	// Calculate how long to fly through the effect.  Not to get to the effect, just through it.
	*warp_time = shipfx_calculate_warp_time(objp);
	*warp_time += dist_to_plane / *speed;

	return 0;
}


void compute_warpout_stuff(object *objp, float *speed, float *warp_time, vector *warp_pos)
{
	// If we're warping through the knossos, do something different.
	if (Ships[objp->instance].special_warp_objnum >= 0) {
		if (compute_special_warpout_stuff(objp, speed, warp_time, warp_pos) != -1) {
			return;
		} else {
			mprintf(("Invalid special warp\n"));
		}
	}

	object	*docked_objp = ai_find_docked_object( objp );
	vector	center_pos = objp->pos;
	float		radius, ship_move_dist, warp_dist;

	radius = objp->radius;

	//	If ship is docked then center wormhold about their center and make radius large enough.
	if ( docked_objp ) {
		vm_vec_avg(&center_pos, &objp->pos, &docked_objp->pos);
		radius += docked_objp->radius;
	}

	// Calculate how long to fly through the effect.  Not to get to the effect, just through it.
	*warp_time = shipfx_calculate_warp_time(objp);

	// Pick some speed at which we want to go through the warp effect.  
	// This is determined by shipfx_calculate_warp_time specifying how long
	// it should take to go through the effect, or 2R.
	*speed = shipfx_calculate_warp_speed(objp);

	if ( objp == Player_obj )	{
		*speed = 0.8f*objp->phys_info.max_vel.xyz.z;
	}

	// Now we know our speed. Figure out how far the warp effect will be from here.  
	ship_move_dist = (*speed * SHIPFX_WARP_DELAY) + radius*1.5f;		// We want to get to 1.5R away from effect
	if ( ship_move_dist < radius*1.5f ) {
		ship_move_dist = radius*1.5f;
	}

	// If this is a huge ship, set the distance to the length of the ship
	if (Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_HUGE_SHIP) {
		ship_move_dist = 0.5f * ship_get_length(&Ships[objp->instance]);
	}

	// Acount for time to get to warp effect, before we actually go through it.
	*warp_time += ship_move_dist / *speed;

	warp_dist = ship_move_dist;
	// allow for off center
	if (Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_HUGE_SHIP) {
		polymodel *pm = model_get(Ships[objp->instance].modelnum);
		warp_dist -= pm->mins.xyz.z;
	}

	vm_vec_scale_add( warp_pos, &center_pos, &objp->orient.vec.fvec, warp_dist );
}

// JAS - code to start the ship doing the warp out effect
// This puts the ship into a mode specified by SF_DEPARTING
// where it flies forward for a set time period at a set
// velocity, then disappears when that time is reached.  This
// also starts the animating 3d effect playing.
void shipfx_warpout_start( object *objp )
{
	float warp_time;
	ship *shipp;
	shipp = &Ships[objp->instance];

	if ( 	shipp->flags & SF_DEPART_WARP )	{
		mprintf(( "Ship is already departing!\n" ));
		return;
	}

	// if we're dying return
	if ( shipp->flags & SF_DYING ) {
		return;
	}

	// if we're HUGE, keep alive - set guardian
	if (Ship_info[shipp->ship_info_index].flags & SIF_HUGE_SHIP) {
		objp->flags |= OF_GUARDIAN;
	}

	// post a warpin event
	if(Game_mode & GM_DEMO_RECORD){
		demo_POST_warpout(objp->signature, shipp->flags);
	}

#ifndef NO_NETWORK
	// don't send ship depart packets for player ships
	if ( (MULTIPLAYER_MASTER) && !(objp->flags & OF_PLAYER_SHIP) ){
		send_ship_depart_packet( objp );
	}
#endif

	// don't do departure wormhole if ship flag is set which indicates no effect
	if ( shipp->flags & SF_NO_DEPARTURE_WARP ) {
		// DKA 5/25/99 If he's going to warpout, set it.  
		// Next line fixes assert in wing cleanup code when no warp effect.
		shipp->flags |= SF_DEPART_WARP;

		shipfx_actually_warpout(shipp, objp);
		return;
	}

	if ( objp == Player_obj )	{
		HUD_printf(XSTR( "Subspace node activated", 498) );
	}

	float	speed, effect_time, effect_radius;
	vector	warp_pos;
	// warp time from compute warpout stuff includes time to get up to warp_pos
	compute_warpout_stuff(objp, &speed, &warp_time, &warp_pos);
	shipp->warp_effect_pos = warp_pos;

	// The ending one means this is a warp-out effect
	int warp_objnum;
	// Effect time is 'SHIPFX_WARP_DELAY' (1.5 secs) seconds to start, 'shipfx_calculate_warp_time' 
	// for ship to go thru, and 'SHIPFX_WARP_DELAY' (1.5 secs) to go away.
	// effect_time = shipfx_calculate_warp_time(objp) + SHIPFX_WARP_DELAY + SHIPFX_WARP_DELAY;
	effect_time = warp_time + SHIPFX_WARP_DELAY;
	effect_radius = shipfx_calculate_effect_radius(objp);

	// maybe special warpout
	if (shipp->special_warp_objnum >= 0) {
		// cap radius to size of knossos
		effect_radius = min(effect_radius, 0.8f*Objects[shipp->special_warp_objnum].radius);
		warp_objnum = fireball_create(&shipp->warp_effect_pos, FIREBALL_KNOSSOS_EFFECT, shipp->special_warp_objnum, effect_radius, 1, NULL, effect_time, shipp->ship_info_index);
	} else {
		warp_objnum = fireball_create(&shipp->warp_effect_pos, FIREBALL_WARP_EFFECT, OBJ_INDEX(objp), effect_radius, 1, NULL, effect_time, shipp->ship_info_index);
	}
	if (warp_objnum < 0 )	{	// JAS: This must always be created, if not, just warp the ship in
		shipfx_actually_warpout(shipp,objp);
		return;
	}

	shipp->warp_effect_fvec = Objects[warp_objnum].orient.vec.fvec;
	// maybe negate if special warp effect
	if (shipp->special_warp_objnum >= 0) {
		if (vm_vec_dotprod(&shipp->warp_effect_fvec, &objp->orient.vec.fvec) > 0) {
			vm_vec_negate(&shipp->warp_effect_fvec);
		}
	}

	// Make the warp effect stage 1 last SHIP_WARP_TIME1 seconds.
	if ( objp == Player_obj )	{
		warp_time = fireball_lifeleft(&Objects[warp_objnum]);
		shipp->final_warp_time = timestamp(fl2i(warp_time*1000.0f));
	} else {
		shipp->final_warp_time = timestamp(fl2i(warp_time*2.0f*1000.0f));
	}
	shipp->flags |= SF_DEPART_WARP;

//	mprintf(( "Warp time = %.4f , effect time = %.4f ms\n", warp_time*1000.0f, effect_time ));

	// This is a hack to make the ship go at the right speed to go from it's current position to the warp_effect_pos;
	
	// Set ship's velocity to 'speed'
	// This should actually be an AI that flies from the current
	// position through 'shipp->warp_effect_pos' in 'warp_time'
	// and keeps going 
	if ( objp != Player_obj )	{
		vector vel;
		vel = objp->orient.vec.fvec;
		vm_vec_scale( &vel, speed );
		objp->phys_info.vel = vel;
		objp->phys_info.desired_vel = vel;
		objp->phys_info.prev_ramp_vel.xyz.x = 0.0f;
		objp->phys_info.prev_ramp_vel.xyz.y = 0.0f;
		objp->phys_info.prev_ramp_vel.xyz.z = speed;
		objp->phys_info.forward_thrust = 1.0f;		// How much the forward thruster is applied.  0-1.

		// special case for HUGE ships
		if (Ship_info[shipp->ship_info_index].flags & SIF_HUGE_SHIP) {
//			objp->phys_info.flags |= PF_SPECIAL_WARP_OUT;
		}
	}

}

void shipfx_warpout_frame( object *objp, float frametime )
{
	ship *shipp;
	shipp = &Ships[objp->instance];

	if ( shipp->flags & SF_DYING ) return;

	vector tempv;
	float warp_pos;	// position of warp effect in object's frame of reference

	vm_vec_sub( &tempv, &objp->pos, &shipp->warp_effect_pos );
	warp_pos = -vm_vec_dot( &tempv, &shipp->warp_effect_fvec );


	// Find the closest point on line from center of wormhole
	vector pos;
	float dist;

	fvi_ray_plane(&pos,&objp->pos,&shipp->warp_effect_fvec,&shipp->warp_effect_pos, &shipp->warp_effect_fvec, 0.0f );
	dist = vm_vec_dist( &pos, &objp->pos );

//	mprintf(( "Warp pos = %.1f, rad=%.1f, center dist = %.1f\n", warp_pos, objp->radius, dist ));

	if ( objp == Player_obj )	{
		// Code for player warpout frame

		if ( (Player->control_mode==PCM_WARPOUT_STAGE2) && (warp_pos > objp->radius) )	{
			gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_DONE_STAGE2 );
		}

		if ( timestamp_elapsed(shipp->final_warp_time) ) {

			// Something went wrong... oh well, warp him out anyway.
			if ( Player->control_mode != PCM_WARPOUT_STAGE3 )	{
				mprintf(( "Hmmm... player ship warpout time elapsed, but he wasn't in warp stage 3.\n" ));
			}

			gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_DONE );
			ship_departed( objp->instance );								// mark log entry for the player
		}

	} else {
		// Code for all non-player ships warpout frame

		int timed_out = timestamp_elapsed(shipp->final_warp_time);
		if ( timed_out )	{
//			mprintf(("Frame %i: Ship %s missed departue cue.\n", Framecount, shipp->ship_name ));
			int	delta_ms = timestamp_until(shipp->final_warp_time);
			if (delta_ms > 1000.0f * frametime ) {
				nprintf(("AI", "Frame %i: Ship %s missed departue cue by %7.3f seconds.\n", Framecount, shipp->ship_name, - (float) delta_ms/1000.0f));
			}
		}

		// MWA 10/21/97 -- added shipp->flags & SF_NO_DEPARTURE_WARP part of next if statement.  For ships
		// that don't get a wormhole effect, I wanted to drop into this code immediately.
		if ( (warp_pos > objp->radius)  || (shipp->flags & SF_NO_DEPARTURE_WARP) || timed_out )	{
			shipfx_actually_warpout( shipp, objp );
		} 
	}
}


//==================================================
// Stuff for keeping track of which ships are in
// whose shadows.


// Given point p0, in object's frame of reference, find if 
// it can see the sun.
int shipfx_point_in_shadow( vector *p0, matrix *src_orient, vector *src_pos, float radius )
{
	mc_info mc;
	object *objp;
	ship_obj *so;
	ship *shipp;
	int n_lights;
	int idx;

	vector rp0, rp1;

	vector light_dir;

	// Move rp0 into world coordinates	
	vm_vec_unrotate(&rp0, p0, src_orient);
	vm_vec_add2(&rp0, src_pos);

	// get the # of global lights
	n_lights = light_get_global_count();
	
	for(idx=0; idx<n_lights; idx++){
		// get the light dir for this light
		light_get_global_dir(&light_dir, idx);

		// Find rp1
		vm_vec_scale_add( &rp1, &rp0, &light_dir, 10000.0f );

		for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) )	{
			objp = &Objects[so->objnum];
			shipp = &Ships[objp->instance];

			mc.model_num = shipp->modelnum;
			mc.orient = &objp->orient;
			mc.pos = &objp->pos;
			mc.p0 = &rp0;
			mc.p1 = &rp1;
			mc.flags = MC_CHECK_MODEL;	

			if ( model_collide(&mc) ){
				return 1;
			}
		}
	}

	// not in shadow
	return 0;
}


// Given an ship see if it is in a shadow.
int shipfx_in_shadow( object * src_obj )
{
	mc_info mc;
	object *objp;
	ship_obj *so;
	ship *shipp;
	int n_lights;
	int idx;

	vector rp0, rp1;
	vector light_dir;

	rp0 = src_obj->pos;
	
	// get the # of global lights
	n_lights = light_get_global_count();

	for(idx=0; idx<n_lights; idx++){
		// get the direction for this light
		light_get_global_dir(&light_dir, idx);

		// Find rp1
		for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) )	{
			objp = &Objects[so->objnum];

			if ( src_obj != objp )	{
				shipp = &Ships[objp->instance];

				vm_vec_scale_add( &rp1, &rp0, &light_dir, objp->radius*10.0f );

				mc.model_num = shipp->modelnum;
				mc.orient = &objp->orient;
				mc.pos = &objp->pos;
				mc.p0 = &rp0;
				mc.p1 = &rp1;
				mc.flags = MC_CHECK_MODEL;	

	//			mc.flags |= MC_CHECK_SPHERELINE;
	//			mc.radius = src_obj->radius;

				if ( model_collide(&mc) )	{
					return 1;
				}
			}
		}
	}

	// not in shadow
	return 0;
}

// Given world point see if it is in a shadow.
int shipfx_eye_in_shadow( vector *eye_pos, object * src_obj, int sun_n )
{
	mc_info mc;
	object *objp;
	ship_obj *so;
	ship *shipp;	

	vector rp0, rp1;
	vector light_dir;

	rp0 = *eye_pos;	
	
	// get the light dir
	if(!light_get_global_dir(&light_dir, sun_n)){
		return 0;
	}

	// Find rp1
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) )	{
		objp = &Objects[so->objnum];

		if ( src_obj != objp )	{
			shipp = &Ships[objp->instance];

			vm_vec_scale_add( &rp1, &rp0, &light_dir, objp->radius*10.0f );

			ship_model_start(objp);

			mc.model_num = shipp->modelnum;
			mc.orient = &objp->orient;
			mc.pos = &objp->pos;
			mc.p0 = &rp0;
			mc.p1 = &rp1;
			mc.flags = MC_CHECK_MODEL;	

	//			mc.flags |= MC_CHECK_SPHERELINE;
	//			mc.radius = src_obj->radius;

			int hit = model_collide(&mc);

			ship_model_stop(objp);

			if (hit) {
				return 1;
			}
		}
	}

	// Check all the big hull debris pieces.
	debris	*db = Debris;

	int i;
	for ( i = 0; i < MAX_DEBRIS_PIECES; i++, db++ )	{
		if ( !(db->flags & DEBRIS_USED) || !db->is_hull ){
			continue;
		}

		objp = &Objects[db->objnum];

		vm_vec_scale_add( &rp1, &rp0, &light_dir, objp->radius*10.0f );

		mc.model_num = db->model_num;	// Fill in the model to check
		mc.submodel_num = db->submodel_num;
		model_clear_instance( mc.model_num );
		mc.orient = &objp->orient;					// The object's orient
		mc.pos = &objp->pos;							// The object's position
		mc.p0 = &rp0;				// Point 1 of ray to check
		mc.p1 = &rp1;					// Point 2 of ray to check
		mc.flags = (MC_CHECK_MODEL | MC_SUBMODEL);

		if (model_collide(&mc))	{
			return 1;
		}
	}	

	// not in shadow
	return 0;
}

//=====================================================================================
// STUFF FOR DOING SHIP GUN FLASHES
//=====================================================================================

#define MAX_FLASHES	128			// How many flashes total
#define FLASH_LIFE_PRIMARY		0.25f			// How long flash lives
#define FLASH_LIFE_SECONDARY	0.50f			// How long flash lives


typedef struct ship_flash {
	int	objnum;					// object number of parent ship
	int	obj_signature;			// signature of that object
	int	light_num;				// which light in the model this uses
	float	life;						// how long this should be around
	float max_life;				// how long this has been around.
} ship_flash;

int Ship_flash_inited = 0;
int Ship_flash_highest = -1;
ship_flash Ship_flash[MAX_FLASHES];

// Resets the ship flash stuff. Call before each level.
void shipfx_flash_init()
{
	int i;
	
	for (i=0; i<MAX_FLASHES; i++ )	{
		Ship_flash[i].objnum = -1;			// mark as unused
	}
	Ship_flash_highest = -1;
	Ship_flash_inited = 1;	
}


// Given that a ship fired a weapon, light up the model
// accordingly.
void shipfx_flash_create(object *objp, ship * shipp, vector *gun_pos, vector *gun_dir, int is_primary, int weapon_info_index)
{
	int i;
	int objnum = OBJ_INDEX(objp);

	Assert(Ship_flash_inited);

	polymodel *pm = model_get( shipp->modelnum );
	int closest_light = -1;
	float d, closest_dist = 0.0f;

	// ALWAYS do this - since this is called once per firing
	// if this is a cannon type weapon, create a muzzle flash
	// HACK - let the flak guns do this on their own since they fire so quickly
	if((Weapon_info[weapon_info_index].wi_flags & WIF_MFLASH) && !(Weapon_info[weapon_info_index].wi_flags & WIF_FLAK)){
		// spiffy new flash stuff
		mflash_create(gun_pos, gun_dir, Weapon_info[weapon_info_index].muzzle_flash);		
	}

	if ( pm->num_lights < 1 ) return;

	for (i=0; i<pm->num_lights; i++ )	{
		d = vm_vec_dist( &pm->lights[i].pos, gun_pos );
	
		if ( pm->lights[i].type == BSP_LIGHT_TYPE_WEAPON ) {
			if ( (closest_light==-1) || (d<closest_dist) )	{
				closest_light = i;
				closest_dist = d;
			}
		}
	}

	if ( closest_light == -1 ) return;

	int first_slot = -1;

	for (i=0; i<=Ship_flash_highest; i++ )	{
		if ( (first_slot==-1) && (Ship_flash[i].objnum < 0) )	{
			first_slot = i;
		}

		if ( (Ship_flash[i].objnum == objnum) && (Ship_flash[i].obj_signature==objp->signature) )	{
			if ( Ship_flash[i].light_num == closest_light )	{
				// This is already flashing!
				Ship_flash[i].life = 0.0f;
				if ( is_primary )	{
					Ship_flash[i].max_life = FLASH_LIFE_PRIMARY;
				} else {
					Ship_flash[i].max_life = FLASH_LIFE_SECONDARY;
				}
				return;
			}
		}
	}

	if ( first_slot == -1 )	{
		if ( Ship_flash_highest < MAX_FLASHES-1 )	{
			Ship_flash_highest++;
			first_slot = Ship_flash_highest;
		} else {
			//mprintf(( "SHIP_FLASH: Out of flash spots!\n" ));
			return;		// out of flash slots
		}
	}

	Assert( Ship_flash[first_slot].objnum == -1 );

	Ship_flash[first_slot].objnum = objnum;
	Ship_flash[first_slot].obj_signature = objp->signature;
	Ship_flash[first_slot].life = 0.0f;		// Start it up
	if ( is_primary )	{
		Ship_flash[first_slot].max_life = FLASH_LIFE_PRIMARY;
	} else {
		Ship_flash[first_slot].max_life = FLASH_LIFE_SECONDARY;
	}
	Ship_flash[first_slot].light_num = closest_light;		
}

// Sets the flash lights in the model used by this
// ship to the appropriate values.  There might not
// be any flashes linked to this ship in which
// case this function does nothing.
void shipfx_flash_light_model( object *objp, ship * shipp )
{
	int i, objnum = OBJ_INDEX(objp);
	polymodel *pm = model_get( shipp->modelnum );

	for (i=0; i<=Ship_flash_highest; i++ )	{
		if ( (Ship_flash[i].objnum == objnum) && (Ship_flash[i].obj_signature==objp->signature) )	{
			float v = (Ship_flash[i].max_life - Ship_flash[i].life)/Ship_flash[i].max_life;

			pm->lights[Ship_flash[i].light_num].value += v / 255.0f;
		}
	}

}

// Does whatever processing needs to be done each frame.
void shipfx_flash_do_frame(float frametime)
{
	ship_flash *sf;
	int kill_it = 0;
	int i;

	for (i=0, sf = &Ship_flash[0]; i<=Ship_flash_highest; i++, sf++ )	{
		if ( sf->objnum > -1 )	{
			if ( Objects[sf->objnum].signature != sf->obj_signature )	{
				kill_it = 1;
			}
			sf->life += frametime;
			if ( sf->life >= sf->max_life )	kill_it = 1;

			if (kill_it) {
				sf->objnum = -1;
				if ( i == Ship_flash_highest )	{
					while( (Ship_flash_highest>0) && (Ship_flash[Ship_flash_highest].objnum == -1) )	{
						Ship_flash_highest--;
					}
				}
			}	
		}
	}	

}

float Particle_width = 1.2f;
DCF(particle_width, "Multiplier for angular width of the particle spew")
{
	if ( Dc_command ) {
		dc_get_arg(ARG_FLOAT);
		if ( (Dc_arg_float >= 0 ) && (Dc_arg_float <= 5) ) {
			Particle_width = Dc_arg_float;
		} else {
			dc_printf( "Illegal value for particle width. (Must be from 0-5) \n\n");
		}
	}
}

float Particle_number = 1.2f;
DCF(particle_num, "Multiplier for the number of particles created")
{
	if ( Dc_command ) {
		dc_get_arg(ARG_FLOAT);
		if ( (Dc_arg_float >= 0 ) && (Dc_arg_float <= 5) ) {
			Particle_number = Dc_arg_float;
		} else {
			dc_printf( "Illegal value for particle num. (Must be from 0-5) \n\n");
		}
	}
}

float Particle_life = 1.2f;
DCF(particle_life, "Multiplier for the lifetime of particles created")
{
	if ( Dc_command ) {
		dc_get_arg(ARG_FLOAT);
		if ( (Dc_arg_float >= 0 ) && (Dc_arg_float <= 5) ) {
			Particle_life = Dc_arg_float;
		} else {
			dc_printf( "Illegal value for particle life. (Must be from 0-5) \n\n");
		}
	}
}

// Make sparks fly off of ship n.
// sn = spark number to spark, corrosponding to element in
//      ship->hitpos array.  If this isn't -1, it is a just
//      got hit by weapon spark, otherwise pick one randomally.
void shipfx_emit_spark( int n, int sn )
{
	int create_spark = 1;
	object * obj;
	vector outpnt;
	ship *shipp = &Ships[n];
	float ship_radius, spark_scale_factor;
	
	if ( shipp->num_hits <= 0 )
		return;

	// get radius of ship
	ship_radius = model_get_radius(Ship_info[shipp->ship_info_index].modelnum);

	// get spark_scale_factor -- how much to increase ship sparks, based on radius
	if (ship_radius > 40) {
		spark_scale_factor = 1.0f;
	} else if (ship_radius > 20) {
		spark_scale_factor = (ship_radius - 20.0f) / 20.0f;
	} else {
		spark_scale_factor = 0.0f;
	}

	float spark_time_scale  = 1.0f + spark_scale_factor * (Particle_life   - 1.0f);
	float spark_width_scale = 1.0f + spark_scale_factor * (Particle_width  - 1.0f);
	float spark_num_scale   = 1.0f + spark_scale_factor * (Particle_number - 1.0f);

	obj = &Objects[shipp->objnum];
	ship_info* si = &Ship_info[shipp->ship_info_index];

	float hull_percent = obj->hull_strength / si->initial_hull_strength;
	if (hull_percent < 0.001) {
		hull_percent = 0.001f;
	}
	float fraction = 0.1f * obj->radius / hull_percent;
	if (fraction > 1.0f) {
		fraction = 1.0f;
	}

	int spark_num;
	if ( sn == -1 ) {
		spark_num = myrand() % shipp->num_hits;
	} else {
		spark_num = sn;
	}

	// don't display sparks that have expired
	if ( timestamp_elapsed(shipp->sparks[spark_num].end_time) ) {
		return;
	}

	// get spark position
	if (shipp->sparks[spark_num].submodel_num != -1) {
		ship_model_start(obj);
		model_find_world_point(&outpnt, &shipp->sparks[spark_num].pos, shipp->modelnum, shipp->sparks[spark_num].submodel_num, &obj->orient, &obj->pos);
		ship_model_stop(obj);
	} else {
		// rotate sparks correctly with current ship orient
		vm_vec_unrotate(&outpnt, &shipp->sparks[spark_num].pos, &obj->orient);
		vm_vec_add2(&outpnt,&obj->pos);
	}

	if ( shipp->flags & (SF_ARRIVING|SF_DEPART_WARP) ) {
		vector tmp;
		vm_vec_sub( &tmp, &outpnt, &shipp->warp_effect_pos );
		if ( vm_vec_dot( &tmp, &shipp->warp_effect_fvec ) < 0.0f )	{
			// if in front of warp plane, don't create.
			create_spark = 0;
		}
	}

	if ( create_spark )	{

		particle_emitter	pe;

		pe.pos = outpnt;				// Where the particles emit from

		if ( shipp->flags & (SF_ARRIVING|SF_DEPART_WARP) ) {
			// No velocity if going through warp.
			pe.vel = vmd_zero_vector;
		} else {
			// Initial velocity of all the particles.
			// 0.0f = 0% of parent's.
			// 1.0f = 100% of parent's.
			vm_vec_copy_scale( &pe.vel, &obj->phys_info.vel, 0.7f );
		}

		// TODO: add velocity from rotation if submodel is rotating
		// v_rot = w x r

		// r = outpnt - model_find_world_point(0)

		// w = model_find_world_dir(
		// model_find_world_dir(&out_dir, &in_dir, model_num, submodel_num, &objorient, &objpos);

		vector tmp_norm, tmp_vel;
		vm_vec_sub( &tmp_norm, &outpnt, &obj->pos );
		vm_vec_normalize_safe(&tmp_norm);

		tmp_vel = obj->phys_info.vel;
		if ( vm_vec_normalize_safe(&tmp_vel) > 1.0f )	{
			vm_vec_scale_add2(&tmp_norm,&tmp_vel, -2.0f);
			vm_vec_normalize_safe(&tmp_norm);
		}
				
		pe.normal = tmp_norm;			// What normal the particle emit around
		pe.normal_variance = 0.3f;		//	How close they stick to that normal 0=good, 1=360 degree
		pe.min_rad = 0.20f;				// Min radius
		pe.max_rad = 0.50f;				// Max radius

		// first time through - set up end time and make heavier initially
		if ( sn > -1 )	{
			// Sparks for first time at this spot
			if (si->flags & SIF_FIGHTER) {
				if (hull_percent > 0.6f) {
					// sparks only once when hull > 60%
					float spark_duration = (float)pow(2.0f, -5.0f*(hull_percent-1.3f)) * (1.0f + 0.6f*(frand()-0.5f));	// +- 30%
					shipp->sparks[spark_num].end_time = timestamp( (int) (1000.0f * spark_duration) );
				} else {
					// spark never ends when hull < 60% (~277 hr)
					shipp->sparks[spark_num].end_time = timestamp( 100000000 );
				}
			}

			if ( D3D_enabled ) {
				pe.num_low  = 25;				// Lowest number of particles to create (hardware)
				pe.num_high = 30;				// Highest number of particles to create (hardware)
			} else {
				pe.num_low  = 5;				// Lowest number of particles to create (software)
				pe.num_high = 7;				// Highest number of particles to create (software)
			}
			pe.normal_variance = 1.0f;	//	How close they stick to that normal 0=good, 1=360 degree
			pe.min_vel = 2.0f;				// How fast the slowest particle can move
			pe.max_vel = 12.0f;				// How fast the fastest particle can move
			pe.min_life = 0.05f;				// How long the particles live
			pe.max_life = 0.55f;				// How long the particles live

			particle_emit( &pe, PARTICLE_FIRE, 0 );
		} else {

			pe.min_rad = 0.7f;				// Min radius
			pe.max_rad = 1.3f;				// Max radius
			if ( D3D_enabled ) {
				pe.num_low  = int (20 * spark_num_scale);		// Lowest number of particles to create (hardware)
				pe.num_high = int (50 * spark_num_scale);		// Highest number of particles to create (hardware)
			} else {
				pe.num_low  = 2;			// Lowest number of particles to create (software)
				pe.num_high = 8;		// Highest number of particles to create (software)
			}
			pe.normal_variance = 0.2f * spark_width_scale;		//	How close they stick to that normal 0=good, 1=360 degree
			pe.min_vel = 3.0f;				// How fast the slowest particle can move
			pe.max_vel = 12.0f;				// How fast the fastest particle can move
			pe.min_life = 0.35f*2.0f * spark_time_scale;		// How long the particles live
			pe.max_life = 0.75f*2.0f * spark_time_scale;		// How long the particles live
			
			particle_emit( &pe, PARTICLE_SMOKE, 0 );
		}
	}

	// Select time to do next spark
//	Ships[n].next_hit_spark = timestamp_rand(100,500);
	shipp->next_hit_spark = timestamp_rand(50,100);
}



//=====================================================================================
// STUFF FOR DOING LARGE SHIP EXPLOSIONS
//=====================================================================================

int	Bs_exp_fire_low = 1;
float	Bs_exp_fire_time_mult = 1.0f;

DCF_BOOL(bs_exp_fire_low, Bs_exp_fire_low)
DCF(bs_exp_fire_time_mult, "Multiplier time between fireball in big ship explosion")
{
	if ( Dc_command ) {
		dc_get_arg(ARG_FLOAT);
		if ( (Dc_arg_float >= 0.1 ) && (Dc_arg_float <= 5) ) {
			Bs_exp_fire_time_mult = Dc_arg_float;
		} else {
			dc_printf( "Illegal value for bs_exp_fire_time_mult. (Must be from 0.1-5) \n\n");
		}
	}
}

#define MAX_SPLIT_SHIPS		3		// How many can explode at once.  Each one is about 1K.

#define DEBRIS_NONE			0
#define DEBRIS_DRAW			1
#define DEBRIS_FREE			2

typedef struct clip_ship {
	object*			parent_obj;
	float 			length_left;	// uncomsumed length
	matrix			orient;
	physics_info	phys_info;
	vector			local_pivot;								// world center of mass position of half ship
	vector			model_center_disp_to_orig_center;	// displacement from half ship center to original model center
	vector			clip_plane_norm;							// clip plane normal (local [0,0,1] or [0,0,-1])
	float				cur_clip_plane_pt;						// displacement from half ship clip plane to original model center
	float				explosion_vel;
	ubyte				draw_debris[MAX_DEBRIS_OBJECTS];
	int				next_fireball;
} clip_ship;

typedef struct split_ship {
	int				used;					// 0 if not used, 1 if used
	clip_ship		front_ship;
	clip_ship		back_ship;
	int				explosion_flash_timestamp;
	int				explosion_flash_started;
	int				sound_handle[NUM_SUB_EXPL_HANDLES];
} split_ship;


static split_ship Split_ships[MAX_SPLIT_SHIPS];
static int Split_ships_inited = 0;

static void split_ship_init_system()
{
	int i;
	for (i=0; i<MAX_SPLIT_SHIPS; i++ )	{
		Split_ships[i].used = 0;
	}
	Split_ships_inited = 1;
}

static void maybe_fireball_wipe(clip_ship* half_ship, int* sound_handle);
static void split_ship_init( ship* shipp, split_ship* split_ship )
{
	object* parent_ship_obj = &Objects[shipp->objnum];
	matrix* orient = &parent_ship_obj->orient;
	for (int ii=0; ii<NUM_SUB_EXPL_HANDLES; ii++) {
		split_ship->sound_handle[ii] = shipp->sub_expl_sound_handle[ii];
	}

	// play 3d sound for shockwave explosion
	snd_play_3d( &Snds[SND_SHOCKWAVE_EXPLODE], &parent_ship_obj->pos, &View_position, 0.0f, NULL, 0, 1.0f, SND_PRIORITY_SINGLE_INSTANCE, NULL, 3.0f );

	// initialize both ships
	split_ship->front_ship.parent_obj = parent_ship_obj;
	split_ship->back_ship.parent_obj  = parent_ship_obj;
	split_ship->explosion_flash_timestamp = timestamp((int)(0.00075f*parent_ship_obj->radius));
	split_ship->explosion_flash_started = 0;
	split_ship->front_ship.orient = *orient;
	split_ship->back_ship.orient  = *orient;
	split_ship->front_ship.next_fireball = timestamp_rand(0, 100);
	split_ship->back_ship.next_fireball  = timestamp_rand(0, 100);

	split_ship->front_ship.clip_plane_norm = vmd_z_vector;
	vm_vec_copy_scale(&split_ship->back_ship.clip_plane_norm, &vmd_z_vector, -1.0f);

	// find the point at which the ship splits (relative to its pivot)
	polymodel* pm = model_get(shipp->modelnum);
	float init_clip_plane_dist;
	if (pm->num_split_plane > 0) {
		int index = rand()%pm->num_split_plane;
		init_clip_plane_dist = pm->split_plane[index];
	} else {
		init_clip_plane_dist = 0.5f * (0.5f - frand())*pm->core_radius;
	}

	split_ship->back_ship.cur_clip_plane_pt =  init_clip_plane_dist;
	split_ship->front_ship.cur_clip_plane_pt = init_clip_plane_dist;

	float dist;
	dist = (split_ship->front_ship.cur_clip_plane_pt+pm->maxs.xyz.z)/2.0f;
	vm_vec_copy_scale(&split_ship->front_ship.local_pivot, &orient->vec.fvec, dist);
	vm_vec_make(&split_ship->front_ship.model_center_disp_to_orig_center, 0.0f, 0.0f, -dist);
	dist = (split_ship->back_ship.cur_clip_plane_pt +pm->mins.xyz.z)/2.0f;
	vm_vec_copy_scale(&split_ship->back_ship.local_pivot, &orient->vec.fvec, dist);
	vm_vec_make(&split_ship->back_ship.model_center_disp_to_orig_center, 0.0f, 0.0f, -dist);
	vm_vec_add2(&split_ship->front_ship.local_pivot, &parent_ship_obj->pos );
	vm_vec_add2(&split_ship->back_ship.local_pivot,  &parent_ship_obj->pos );
	
	// find which debris pieces are in the front and back split ships
	for (int i=0; i<pm->num_debris_objects; i++ )	{
		vector temp_pos = {0.0f, 0.0f, 0.0f};
		vector tmp = {0.0f, 0.0f, 0.0f };		
		vector tmp1 = pm->submodel[pm->debris_objects[i]].offset;
		// tmp is world position,  temp_pos is world_pivot,  tmp1 is offset from world_pivot (in ship local coord)
		model_find_world_point(&tmp, &tmp1, shipp->modelnum, -1, &vmd_identity_matrix, &temp_pos );
		if (tmp.xyz.z > init_clip_plane_dist) {
			split_ship->front_ship.draw_debris[i] = DEBRIS_DRAW;
			split_ship->back_ship.draw_debris[i]  = DEBRIS_NONE;
		} else {
			split_ship->front_ship.draw_debris[i] = DEBRIS_NONE;
			split_ship->back_ship.draw_debris[i]  = DEBRIS_DRAW;
		}
	}

	/*
	// set the remaining debris slots to not draw
	for (i=pm->num_debris_objects; i<MAX_DEBRIS_OBJECTS; i++) {
		split_ship->front_ship.draw_debris[i] = DEBRIS_NONE;
		split_ship->back_ship.draw_debris[i]  = DEBRIS_NONE;
	} */

	// set up physics 
	physics_init( &split_ship->front_ship.phys_info );
	physics_init( &split_ship->back_ship.phys_info );
	split_ship->front_ship.phys_info.flags  |= (PF_ACCELERATES | PF_DEAD_DAMP);
	split_ship->back_ship.phys_info.flags |= (PF_ACCELERATES | PF_DEAD_DAMP);
	split_ship->front_ship.phys_info.side_slip_time_const = 10000.0f;
	split_ship->back_ship.phys_info.side_slip_time_const =  10000.0f;
	split_ship->front_ship.phys_info.rotdamp = 10000.0f;
	split_ship->back_ship.phys_info.rotdamp =  10000.0f;

	// set up explosion vel and relative velocities (assuming mass depends on length)
	float front_length = pm->maxs.xyz.z - split_ship->front_ship.cur_clip_plane_pt;
	float back_length  = split_ship->back_ship.cur_clip_plane_pt - pm->mins.xyz.z;
	float ship_length = front_length + back_length;
	split_ship->front_ship.length_left = front_length;
	split_ship->back_ship.length_left  = back_length;

	float expl_length_scale = (ship_length - 200.0f) / 2000.0f;
	// s_r_f effects speed of "wipe" and rotvel
	float speed_reduction_factor = (1.0f + 0.001f*parent_ship_obj->radius);
	float explosion_time = (3.0f + expl_length_scale + (frand()-0.5f)) * speed_reduction_factor;
	float long_length = max(front_length, back_length);
	float expl_vel = long_length / explosion_time;
	split_ship->front_ship.explosion_vel = expl_vel;
	split_ship->back_ship.explosion_vel  = -expl_vel;

	float rel_vel = (0.6f + 0.2f*frand()) * expl_vel * speed_reduction_factor;
	float front_vel = rel_vel * back_length / ship_length;
	float back_vel = -rel_vel * front_length / ship_length;
	// mprintf(("rel_vel %.1f, expl_vel %.1f\n", rel_vel, expl_vel));

	// set up rotational vel
	vector rotvel;
	vm_vec_rand_vec_quick(&rotvel);
	rotvel.xyz.z = 0.0f;
	vm_vec_normalize(&rotvel);
	vm_vec_scale(&rotvel, 0.15f / speed_reduction_factor);
	split_ship->front_ship.phys_info.rotvel = rotvel;
	vm_vec_copy_scale(&split_ship->back_ship.phys_info.rotvel, &rotvel, -(front_length*front_length)/(back_length*back_length));
	split_ship->front_ship.phys_info.rotvel.xyz.z = parent_ship_obj->phys_info.rotvel.xyz.z;
	split_ship->back_ship.phys_info.rotvel.xyz.z  = parent_ship_obj->phys_info.rotvel.xyz.z;


	// modify vel of each split ship based on rotvel of parent ship obj
	vector temp_rotvel = parent_ship_obj->phys_info.rotvel;
	temp_rotvel.xyz.z = 0.0f;
	vector vel_from_rotvel;
	vm_vec_crossprod(&vel_from_rotvel, &temp_rotvel, &split_ship->front_ship.local_pivot);
	//	vm_vec_scale_add2(&split_ship->front_ship.phys_info.vel, &vel_from_rotvel, 0.5f);
	vm_vec_crossprod(&vel_from_rotvel, &temp_rotvel, &split_ship->back_ship.local_pivot);
	//	vm_vec_scale_add2(&split_ship->back_ship.phys_info.vel, &vel_from_rotvel, 0.5f);

	// set up velocity and make initial fireballs and particles
	split_ship->front_ship.phys_info.vel = parent_ship_obj->phys_info.vel;
	split_ship->back_ship.phys_info.vel  = parent_ship_obj->phys_info.vel;
	maybe_fireball_wipe(&split_ship->front_ship, (int*)&split_ship->sound_handle);
	maybe_fireball_wipe(&split_ship->back_ship,  (int*)&split_ship->sound_handle);
	vm_vec_scale_add2(&split_ship->front_ship.phys_info.vel, &orient->vec.fvec, front_vel);
	vm_vec_scale_add2(&split_ship->back_ship.phys_info.vel,  &orient->vec.fvec, back_vel);

	// HANDLE LIVE DEBRIS - blow off if not already gone
	shipfx_maybe_create_live_debris_at_ship_death( parent_ship_obj );
}


static void half_ship_render_ship_and_debris(clip_ship* half_ship,ship *shipp)
{
	Assert( Split_ships_inited );

	polymodel *pm = model_get(shipp->modelnum);

	// get rotated clip plane normal and world coord of original ship center
	vector orig_ship_world_center, clip_plane_norm, model_clip_plane_pt, debris_clip_plane_pt;
	vm_vec_unrotate(&clip_plane_norm, &half_ship->clip_plane_norm, &half_ship->orient);
	vm_vec_unrotate(&orig_ship_world_center, &half_ship->model_center_disp_to_orig_center, &half_ship->orient);
	vm_vec_add2(&orig_ship_world_center, &half_ship->local_pivot);

	// *out_pivot = orig_ship_world_center;

	// get debris clip plane pt and draw debris
	vm_vec_unrotate(&debris_clip_plane_pt, &half_ship->model_center_disp_to_orig_center, &half_ship->orient);
	vm_vec_add2(&debris_clip_plane_pt, &half_ship->local_pivot);
	g3_start_user_clip_plane( &debris_clip_plane_pt, &clip_plane_norm);

	// set up render flags
	uint render_flags = MR_NORMAL;

	for (int i=0; i<pm->num_debris_objects; i++ )	{
		// draw DEBRIS_FREE in test only
		if (half_ship->draw_debris[i] == DEBRIS_DRAW) {
			vector temp_pos = orig_ship_world_center;
			vector tmp = {0.0f, 0.0f, 0.0f};
			vector tmp1 = pm->submodel[pm->debris_objects[i]].offset;

			// determine if explosion front has past debris piece
			// 67 ~ dist expl moves in 2 frames -- maybe fraction works better
			int is_live_debris = pm->submodel[pm->debris_objects[i]].is_live_debris;
			int create_debris = 0;
			// front ship
			if (half_ship->explosion_vel > 0) {
				if (half_ship->cur_clip_plane_pt > tmp1.xyz.z + pm->submodel[pm->debris_objects[i]].max.xyz.z - 0.1f*half_ship->explosion_vel) {
					create_debris = 1;
				}
				// is the debris visible
//				if (half_ship->cur_clip_plane_pt > tmp1.z + pm->submodel[pm->debris_objects[i]].min.z - 0.5f*half_ship->explosion_vel) {
//					render_debris = 1;
//				}
			// back ship
			} else {
				if (half_ship->cur_clip_plane_pt < tmp1.xyz.z + pm->submodel[pm->debris_objects[i]].min.xyz.z - 0.1f*half_ship->explosion_vel) {
					create_debris = 1;
				}
				// is the debris visible
//				if (half_ship->cur_clip_plane_pt < tmp1.z + pm->submodel[pm->debris_objects[i]].max.z - 0.5f*half_ship->explosion_vel) {
//					render_debris = 1;
//				}
			}

			// Draw debris, but not live debris
			if ( !is_live_debris ) {
				model_find_world_point(&tmp, &tmp1, shipp->modelnum, -1, &half_ship->orient, &temp_pos);
				submodel_render(shipp->modelnum, pm->debris_objects[i], &half_ship->orient, &tmp, render_flags);
			}

			// make free piece of debris
			if ( create_debris ) {
				half_ship->draw_debris[i] = DEBRIS_FREE;		// mark debris to not render with model
				vector center_to_debris, debris_vel, radial_vel;
				// check if last debris piece, ie, debris_count == 0
				int debris_count = 0;
				for (int j=0; j<pm->num_debris_objects; j++ ) {
					if (half_ship->draw_debris[j] == DEBRIS_DRAW) {
						debris_count++;
					}
				} 
				// do debris create here, but not for live debris
				// debris vel (1) split ship vel (2) split ship rotvel (3) random
				if ( !is_live_debris ) {
					object* debris_obj;
					debris_obj = debris_create(half_ship->parent_obj, shipp->modelnum, pm->debris_objects[i], &tmp, &half_ship->local_pivot, 1, 1.0f);
					// AL: make sure debris_obj isn't NULL!
					if ( debris_obj ) {
						vm_vec_scale(&debris_obj->phys_info.rotvel, 4.0f);
						debris_obj->orient = half_ship->orient;
						// if (debris_count > 0) {
							//mprintf(( "base rotvel %.1f, debris rotvel mag %.2f\n", vm_vec_mag(&half_ship->phys_info.rotvel), vm_vec_mag(&debris_obj->phys_info.rotvel) ));
							vm_vec_sub(&center_to_debris, &tmp, &half_ship->local_pivot);
							vm_vec_crossprod(&debris_vel, &center_to_debris, &half_ship->phys_info.rotvel);
							vm_vec_add2(&debris_vel, &half_ship->phys_info.vel);
							vm_vec_copy_normalize(&radial_vel, &center_to_debris);
							float radial_mag = 10.0f + 30.0f*frand();
							vm_vec_scale_add2(&debris_vel, &radial_vel, radial_mag);
							debris_obj->phys_info.vel = debris_vel;
						/* } else {
							debris_obj->phys_info.vel = half_ship->phys_info.vel;
							debris_obj->phys_info.rotvel = half_ship->phys_info.rotvel;
						} */
					}
				}
			}
		}
	}

	// get model clip plane pt and draw model
	vector temp;
	vm_vec_make(&temp, 0.0f, 0.0f, half_ship->cur_clip_plane_pt);
	vm_vec_unrotate(&model_clip_plane_pt, &temp, &half_ship->orient);
	vm_vec_add2(&model_clip_plane_pt, &orig_ship_world_center);
	g3_start_user_clip_plane( &model_clip_plane_pt, &clip_plane_norm );
	model_render(shipp->modelnum, &half_ship->orient, &orig_ship_world_center, render_flags);
}

void shipfx_large_blowup_level_init()
{
	split_ship_init_system();

	if(Ship_cannon_bitmap != -1){
		bm_unload(Ship_cannon_bitmap);
		Ship_cannon_bitmap = bm_load(SHIP_CANNON_BITMAP);
	}
}

// Returns 0 if couldn't init
int shipfx_large_blowup_init(ship *shipp)
{

	if ( !Split_ships_inited )	{
		split_ship_init_system();
	}

	int i;
	for (i=0; i<MAX_SPLIT_SHIPS; i++ )	{
		if ( Split_ships[i].used == 0 )	{
			break;
		}
	}

	if ( i >= MAX_SPLIT_SHIPS )	{
		mprintf(( "Not enough split ship slots!! See John!\n" ));
		Int3();
		return 0;
	}

	Split_ships[i].used = 1;
	shipp->large_ship_blowup_index = i;

	split_ship_init(shipp, &Split_ships[i] );
	
	return 1;
}

// ----------------------------------------------------------------------------
// uses list of model z values with constant increment to find the radius of the 
// cross section at the current model z value
float get_model_cross_section_at_z(float z, polymodel* pm)
{
	if (pm->num_xc < 2) {
		return 0.0f;
	}

	float index, increment;
	increment = (pm->xc[pm->num_xc-1].z - pm->xc[0].z) / (float)(pm->num_xc - 1);
	index = (z - pm->xc[0].z) / increment;

	if (index < 0.5f) {
		return pm->xc[0].radius;
	} else if (index > (pm->num_xc - 1.0f - 0.5f)) {
		return pm->xc[pm->num_xc-1].radius;
	} else {
		int floor_index = (int)floor(index);
		int ceil_index  = (int)ceil(index);
		return max(pm->xc[ceil_index].radius, pm->xc[floor_index].radius);
	}
}

// returns how long sound has been playing
int get_sound_time_played(int snd_id, int handle)
{
	if (handle == -1) {
		return 100000;
	}

	int bits_per_sample, frequency;
	snd_get_format(snd_id, &bits_per_sample, &frequency);
	int time_left = snd_time_remaining(handle, bits_per_sample, frequency);
	int duration = snd_get_duration(snd_id);
	
	return (duration - time_left);
}

// sound manager for big ship sub explosions sounds.
// forces playing of sub-explosion sounds.  keeps track of active sounds, plays them for >= 750 ms
// when sound has played >= 750, sound is stopped and new instance is started 
void do_sub_expl_sound(float radius, vector* sound_pos, int* sound_handle)
{
	#ifndef NO_SOUND
	int sound_index, handle;
	// multiplier for range (near and far distances) to apply attenuation
	float sound_range = 1.0f + 0.0043f*radius;

	int handle_index = rand()%NUM_SUB_EXPL_HANDLES;
	//mprintf(("handle_index %d\n", *handle_index));

	// sound_index = get_sub_explosion_sound_index(handle_index);
	sound_index = SND_SHIP_EXPLODE_1;
	handle = sound_handle[handle_index];


	// mprintf(("dist to sound %.1f snd_indx: %d, h1: %d, h2: %d\n", vm_vec_dist(&Player_obj->pos, sound_pos), next_sound_index, sound_handle[0], sound_handle[1]));

	if (handle == -1) {
		// if no handle, get one
		sound_handle[handle_index] = snd_play_3d( &Snds[sound_index], sound_pos, &View_position, 0.0f, NULL, 0, 0.6f, SND_PRIORITY_MUST_PLAY, NULL, sound_range );
	} else if (!snd_is_playing(handle)) {
		// if sound not playing and old, get new one
		// I don't think will happen with SND_PRIORITY_MUST_PLAY
		if (get_sound_time_played(Snds[sound_index].id, handle) > 400) {
			//mprintf(("sound not playing %d, time_played %d, stopped\n", handle, get_sound_time_played(Snds[sound_index].id, handle)));
			snd_stop(sound_handle[handle_index]);
			sound_handle[handle_index] = snd_play_3d( &Snds[sound_index], sound_pos, &View_position, 0.0f, NULL, 0, 0.6f, SND_PRIORITY_MUST_PLAY, NULL, sound_range );
		}
	} else if (get_sound_time_played(Snds[sound_index].id, handle) > 750) {
		//mprintf(("time %f, cur sound %d time_played %d num sounds %d\n", f2fl(Missiontime), handle_index, get_sound_time_played(Snds[sound_index].id, handle), snd_num_playing() ));
		sound_handle[handle_index] = snd_play_3d( &Snds[sound_index], sound_pos, &View_position, 0.0f, NULL, 0, 0.6f, SND_PRIORITY_MUST_PLAY, NULL, sound_range );
	}
	#endif  // ifndef NO_SOUND
}

// maybe create a fireball along model clip plane
// also maybe plays explosion sound
static void maybe_fireball_wipe(clip_ship* half_ship, int* sound_handle)
{
	// maybe make fireball to cover wipe.
	if ( timestamp_elapsed(half_ship->next_fireball) ) {
		if ( half_ship->length_left > 0.2f*fl_abs(half_ship->explosion_vel) )	{

			polymodel* pm = model_get(Ships[half_ship->parent_obj->instance].modelnum);

			vector model_clip_plane_pt, orig_ship_world_center, temp;

			vm_vec_unrotate(&orig_ship_world_center, &half_ship->model_center_disp_to_orig_center, &half_ship->orient);
			vm_vec_add2(&orig_ship_world_center, &half_ship->local_pivot);

			vm_vec_make(&temp, 0.0f, 0.0f, half_ship->cur_clip_plane_pt);
			vm_vec_unrotate(&model_clip_plane_pt, &temp, &half_ship->orient);
			vm_vec_add2(&model_clip_plane_pt, &orig_ship_world_center);
			vm_vec_rand_vec_quick(&temp);
			vm_vec_scale(&temp, 0.1f*frand());
			vm_vec_add2(&model_clip_plane_pt, &temp);

			float rad = get_model_cross_section_at_z(half_ship->cur_clip_plane_pt, pm);
			if (rad < 1) {
				rad = half_ship->parent_obj->radius * frand_range(0.4f, 0.6f);
			} else {
				// make fireball radius (1.5 +/- .1) * model_cross_section value
				rad *= frand_range(1.4f, 1.6f);
			}

			rad *= 1.5f;
			rad = min(rad, half_ship->parent_obj->radius);

			// mprintf(("xc %.1f model %.1f\n", rad, half_ship->parent_obj->radius*0.25));
			int fireball_type = FIREBALL_EXPLOSION_LARGE1 + rand()%FIREBALL_NUM_LARGE_EXPLOSIONS;
			int low_res_fireballs = Bs_exp_fire_low;
			fireball_create(&model_clip_plane_pt, fireball_type, OBJ_INDEX(half_ship->parent_obj), rad, 0, &half_ship->parent_obj->phys_info.vel, 0.0f, -1, NULL, low_res_fireballs);

			// start the next fireball up (3-4 per frame) + 30%
			int time_low, time_high;
			time_low = int(650 * Bs_exp_fire_time_mult);
			time_high = int(900 * Bs_exp_fire_time_mult);
			half_ship->next_fireball = timestamp_rand(time_low, time_high);

			// do sound
			do_sub_expl_sound(half_ship->parent_obj->radius, &model_clip_plane_pt, sound_handle);

			// do particles
			particle_emitter	pe;

			pe.num_low = 40;					// Lowest number of particles to create
			pe.num_high = 80;				// Highest number of particles to create
			pe.pos = model_clip_plane_pt;	// Where the particles emit from
			pe.vel = half_ship->phys_info.vel;		// Initial velocity of all the particles

#ifdef FS2_DEMO
			float range = 1.0f + 0.002f*half_ship->parent_obj->radius * 5.0f;
#else 
			float range = 1.0f + 0.002f*half_ship->parent_obj->radius;
#endif

#ifdef FS2_DEMO
			pe.min_life = 2.0f*range;				// How long the particles live
			pe.max_life = 10.0f*range;				// How long the particles live
#else
			pe.min_life = 0.5f*range;				// How long the particles live
			pe.max_life = 6.0f*range;				// How long the particles live
#endif
			pe.normal = vmd_x_vector;		// What normal the particle emit around
			pe.normal_variance = 2.0f;		//	How close they stick to that normal 0=on normal, 1=180, 2=360 degree
			pe.min_vel = 0.0f;				// How fast the slowest particle can move
			pe.max_vel = half_ship->explosion_vel;				// How fast the fastest particle can move

#ifdef FS2_DEMO
			float scale = half_ship->parent_obj->radius * 0.02f;
#else
			float scale = half_ship->parent_obj->radius * 0.01f;
#endif
			pe.min_rad = 0.5f*scale;				// Min radius
			pe.max_rad = 1.5f*scale;				// Max radius

			particle_emit( &pe, PARTICLE_SMOKE2, 0, range );

		} else {
			// time out forever
			half_ship->next_fireball = timestamp(-1);
		}
	}
}


// Returns 1 when explosion is done
int shipfx_large_blowup_do_frame(ship *shipp, float frametime)
{
	// DAVE:  I made this not do any movement just to try to get things working...
	// return 0;

	Assert( Split_ships_inited );
	Assert( shipp->large_ship_blowup_index > -1 );

	split_ship *the_split_ship = &Split_ships[shipp->large_ship_blowup_index];
	Assert( the_split_ship->used );		// Get John

	// Do fireballs, particles, shockwave here
	// Note parent ship is still valid, vel and pos updated in obj_move_all

	if ( timestamp_elapsed(the_split_ship->explosion_flash_timestamp) ) {
		if ( !the_split_ship->explosion_flash_started ) {
			object* objp = &Objects[shipp->objnum];
			if (objp->flags & OF_WAS_RENDERED) {
				float excess_dist = vm_vec_dist(&Player_obj->pos, &objp->pos) - 2.0f*objp->radius - Player_obj->radius;
				float intensity = 1.0f - 0.1f*excess_dist / objp->radius;

				if (intensity > 1) {
					intensity = 1.0f;
				}

				if (intensity > 0.1f) {
					// big_explosion_flash(intensity);
				}
			}
			the_split_ship->explosion_flash_started = 1;
		}
	}

	physics_sim(&the_split_ship->front_ship.local_pivot, &the_split_ship->front_ship.orient, &the_split_ship->front_ship.phys_info, frametime);
	physics_sim(&the_split_ship->back_ship.local_pivot,  &the_split_ship->back_ship.orient,  &the_split_ship->back_ship.phys_info,  frametime);
	the_split_ship->front_ship.length_left -= the_split_ship->front_ship.explosion_vel*frametime;
	the_split_ship->back_ship.length_left  += the_split_ship->back_ship.explosion_vel *frametime;
	the_split_ship->front_ship.cur_clip_plane_pt += the_split_ship->front_ship.explosion_vel*frametime;
	the_split_ship->back_ship.cur_clip_plane_pt  += the_split_ship->back_ship.explosion_vel *frametime;

	float length_left = max( the_split_ship->front_ship.length_left, the_split_ship->back_ship.length_left );

	//	mprintf(( "Blowup frame, dist = %.1f \n", length_left ));

	if ( length_left < 0 )	{
		the_split_ship->used = 0;
		return 1;
	}

	maybe_fireball_wipe(&the_split_ship->front_ship, (int*)&the_split_ship->sound_handle);
	maybe_fireball_wipe(&the_split_ship->back_ship,  (int*)&the_split_ship->sound_handle);
	return 0;
}

void shipfx_large_blowup_render(ship* shipp)
{
// This actually renders the original model like it should render.
//	object *objp = &Objects[shipp->objnum];
//	model_render( shipp->modelnum, &objp->orient, &objp->pos, MR_NORMAL );
//	return;

	Assert( Split_ships_inited );
	Assert( shipp->large_ship_blowup_index > -1 );

	split_ship *the_split_ship = &Split_ships[shipp->large_ship_blowup_index];
	Assert( the_split_ship->used );		// Get John

	// vector front_global_pivot, back_global_pivot;

	if (the_split_ship->front_ship.length_left > 0) {
		half_ship_render_ship_and_debris(&the_split_ship->front_ship,shipp);
	}

	if (the_split_ship->back_ship.length_left > 0) {
		half_ship_render_ship_and_debris(&the_split_ship->back_ship,shipp);
	}

	g3_stop_user_clip_plane();			
}


// ================== DO THE ELECTRIC ARCING STUFF =====================
// Creates any new ones, moves old ones.

#define MAX_ARC_LENGTH_PERCENTAGE 0.25f

#define MAX_EMP_ARC_TIMESTAMP		 (150.0f)

void shipfx_do_damaged_arcs_frame( ship *shipp )
{
	int i;
	int should_arc;
	object *obj = &Objects[shipp->objnum];
	ship_info * sip = &Ship_info[shipp->ship_info_index];

	should_arc = 1;

	float damage = obj->hull_strength / sip->initial_hull_strength;	

	if (damage < 0) {
		damage = 0.0f;
	}

	// don't draw an arc based on damage
	if ( damage > 0.30f )	{
		// Don't do spark.
		should_arc = 0;
	}

	// we should draw an arc
	if( shipp->emp_intensity > 0.0f){
		should_arc = 1;
	}

	// Kill off old sparks
	for(i=0; i<MAX_SHIP_ARCS; i++){
		if(timestamp_valid(shipp->arc_timestamp[i]) && timestamp_elapsed(shipp->arc_timestamp[i])){			
			shipp->arc_timestamp[i] = timestamp(-1);
		}
	}

	// if we shouldn't draw an arc, return
	if(!should_arc){
		return;
	}

	if (!timestamp_valid(shipp->arc_next_time))	{
		// start the next fireball up in the next 10 seconds or so... 
		int freq;
		
		// if the emp effect is active
		if(shipp->emp_intensity > 0.0f){
			freq = fl2i(MAX_EMP_ARC_TIMESTAMP);
		}
		// otherwise if we're arcing based upon damage
		else {
			freq = fl2i((damage+0.1f)*5000.0f);
		}

		// set the next arc time
		shipp->arc_next_time = timestamp_rand(freq*2,freq*4);
	}

	if ( timestamp_elapsed(shipp->arc_next_time) )	{

		shipp->arc_next_time = timestamp(-1);		// invalid, so it gets restarted next frame

		//mprintf(( "Creating new ship arc!\n" ));

		int n, n_arcs = ((rand()>>5) % 3)+1;		// Create 1-3 sparks

		vector v1, v2, v3, v4;
		submodel_get_two_random_points( shipp->modelnum, -1, &v1, &v2 );
		submodel_get_two_random_points( shipp->modelnum, -1, &v3, &v4 );

		// For large ships, cap the length to be 25% of max radius
		if ( obj->radius > 200.0f )	{
			float max_dist = obj->radius * MAX_ARC_LENGTH_PERCENTAGE;
			
			vector tmp;
			float d;

			// Cap arc 2->1
			vm_vec_sub( &tmp, &v1, &v2 );
			d = vm_vec_mag_quick( &tmp );
			if ( d > max_dist )	{
				vm_vec_scale_add( &v1, &v2, &tmp, max_dist / d );
			}

			// Cap arc 2->3
			vm_vec_sub( &tmp, &v3, &v2 );
			d = vm_vec_mag_quick( &tmp );
			if ( d > max_dist )	{
				vm_vec_scale_add( &v3, &v2, &tmp, max_dist / d );
			}


			// Cap arc 2->4
			vm_vec_sub( &tmp, &v4, &v2 );
			d = vm_vec_mag_quick( &tmp );
			if ( d > max_dist )	{
				vm_vec_scale_add( &v4, &v2, &tmp, max_dist / d );
			}
			
		}
		
		n = 0;

//		int a = 100, b = 1000;
		float factor = 1.0f + 0.0025f*obj->radius;
		int a = (int) (factor*100.0f);
		int b = (int) (factor*1000.0f);
		int lifetime = (myrand()%((b)-(a)+1))+(a);

		// Create the arc effects
		for (i=0; i<MAX_SHIP_ARCS; i++ )	{
			if ( !timestamp_valid( shipp->arc_timestamp[i] ) )	{
				//shipp->arc_timestamp[i] = timestamp_rand(400,1000);	// live up to a second
				shipp->arc_timestamp[i] = timestamp(lifetime);	// live up to a second

				switch( n )	{
				case 0:
					shipp->arc_pts[i][0] = v1;
					shipp->arc_pts[i][1] = v2;
					break;
				case 1:
					shipp->arc_pts[i][0] = v2;
					shipp->arc_pts[i][1] = v3;
					break;

				case 2:
					shipp->arc_pts[i][0] = v2;
					shipp->arc_pts[i][1] = v4;
					break;

				default:
					Int3();
				}

				// determine what kind of arc to create
				if(shipp->emp_intensity > 0.0f){
					shipp->arc_type[i] = MARC_TYPE_EMP;
				} else {
					shipp->arc_type[i] = MARC_TYPE_NORMAL;
				}
					
				n++;
				if ( n == n_arcs )
					break;	// Don't need to create anymore
			}
	
			// rotate v2 out of local coordinates into world.
			// Use v2 since it is used in every bolt.  See above switch().
			vector snd_pos;
			vm_vec_unrotate(&snd_pos, &v2, &obj->orient);
			vm_vec_add2(&snd_pos, &obj->pos );

			//Play a sound effect
			if ( lifetime > 750 )	{
				// 1.00 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_05], &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  500 )	{
				// 0.75 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_04], &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  250 )	{
				// 0.50 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_03], &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  100 )	{
				// 0.25 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_02], &snd_pos, &View_position, obj->radius );
			} else {
				// 0.10 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_01], &snd_pos, &View_position, obj->radius );
			}
		}
	}

	// maybe move arc points around
	for (i=0; i<MAX_SHIP_ARCS; i++ )	{
		if ( timestamp_valid( shipp->arc_timestamp[i] ) )	{
			if ( !timestamp_elapsed( shipp->arc_timestamp[i] ) )	{							
				// Maybe move a vertex....  20% of the time maybe?
				int mr = myrand();
				if ( mr < RAND_MAX/5 )	{
					vector v1, v2;
					submodel_get_two_random_points( shipp->modelnum, -1, &v1, &v2 );

					vector static_one;

					if ( mr % 2 )	{
						static_one = shipp->arc_pts[i][0];
					} else {
						static_one = shipp->arc_pts[i][1];
					}

					// For large ships, cap the length to be 25% of max radius
					if ( obj->radius > 200.0f )	{
						float max_dist = obj->radius * MAX_ARC_LENGTH_PERCENTAGE;
						
						vector tmp;
						float d;

						// Cap arc 2->1
						vm_vec_sub( &tmp, &v1, &static_one );
						d = vm_vec_mag_quick( &tmp );
						if ( d > max_dist )	{
							vm_vec_scale_add( &v1, &static_one, &tmp, max_dist / d );
						}
					}

					shipp->arc_pts[i][mr % 2] = v1;
				}
			}
		}
	}
}

int l_cruiser_count = 1;
int l_big_count = 2;
int l_huge_count = 3;
float l_max_radius = 3000.0f;
void shipfx_do_lightning_frame( ship *shipp )
{
	/*
	ship_info *sip;
	object *objp;
	int stamp, count;
	vector v1, v2, n1, n2, temp, temp2;
	bolt_info binfo;

	// sanity checks
	Assert(shipp != NULL);
	if(shipp == NULL){
		return;
	} 
	Assert(shipp->ship_info_index >= 0);
	if(shipp->ship_info_index < 0){
		return;
	}	
	Assert(shipp->objnum >= 0);
	if(shipp->objnum < 0){
		return;
	}	

	// get some pointers
	sip = &Ship_info[shipp->ship_info_index];
	objp = &Objects[shipp->objnum];	

	// if this is not a nebula mission, don't do anything
	if(!(The_mission.flags & MISSION_FLAG_FULLNEB)){
		shipp->lightning_stamp = -1;
		return;
	}
	
	// if this not a cruiser or big ship
	if(!((sip->flags & SIF_CRUISER) || (sip->flags & SIF_BIG_SHIP) || (sip->flags & SIF_HUGE_SHIP))){
		shipp->lightning_stamp = -1;
		return;
	}

	// determine stamp and count values
	if(sip->flags & SIF_CRUISER){
		stamp = (int)((float)(Nebl_cruiser_min + ((Nebl_cruiser_max - Nebl_cruiser_min) * Nebl_intensity)) * frand_range(0.8f, 1.1f));
		count = l_cruiser_count;
	} 
	else {
		if(sip->flags & SIF_HUGE_SHIP){
			stamp = (int)((float)(Nebl_supercap_min + ((Nebl_supercap_max - Nebl_supercap_min) * Nebl_intensity)) * frand_range(0.8f, 1.1f));
			count = l_huge_count;
		} else {
			stamp = (int)((float)(Nebl_cap_min + ((Nebl_cap_max - Nebl_cap_min) * Nebl_intensity)) * frand_range(0.8f, 1.1f));
			count = l_big_count;
		}
	}

	// if his timestamp is unset
	if(shipp->lightning_stamp == -1){
		shipp->lightning_stamp = timestamp(stamp);
		return;
	}
	// if his timestamp is currently unelapsed
	if(!timestamp_elapsed(shipp->lightning_stamp)){
		return;
	}

	mprintf(("SHIP BOLT\n"));

	// restamp him first
	shipp->lightning_stamp = timestamp(stamp);

	// ah, now we can create some lightning bolts
	count = (int)frand_range(0.0f, (float)count);
	while(count > 0){
		// get 2 points on the hull of the ship
		submodel_get_two_random_points(shipp->modelnum, 0, &v1, &v2, &n1, &n2);		

		// make up to 2 bolts
		if(objp->radius > l_max_radius){
			vm_vec_scale_add(&temp2, &v1, &n1, l_max_radius);
		} else {
			vm_vec_scale_add(&temp2, &v1, &n1, objp->radius);
		}
		vm_vec_unrotate(&temp, &temp2, &objp->orient);
		vm_vec_add2(&temp, &objp->pos);
		vm_vec_unrotate(&temp2, &v1, &objp->orient);
		vm_vec_add2(&temp2, &objp->pos);

		// create the bolt
		binfo.start = temp;
		binfo.strike = temp2;
		binfo.num_strikes = 3;
		binfo.noise = 0.045f;
		binfo.life = 375;
		binfo.delay = (int)frand_range(0.0f, 1600.0f);
		nebl_bolt(&binfo);
		count--;
	
		// done
		if(count <= 0){
			break;
		}

		// one more		
		if(objp->radius > l_max_radius){
			vm_vec_scale_add(&temp2, &v2, &n2, l_max_radius);
		} else {
			vm_vec_scale_add(&temp2, &v2, &n2, objp->radius);
		}
		vm_vec_unrotate(&temp, &temp2, &objp->orient);
		vm_vec_add2(&temp, &objp->pos);
		vm_vec_unrotate(&temp2, &v2, &objp->orient);
		vm_vec_add2(&temp2, &objp->pos);

		// create the bolt
		binfo.start = temp;
		binfo.strike = temp2;
		binfo.num_strikes = 3;
		binfo.noise = 0.045f;
		binfo.life = 375;
		binfo.delay = (int)frand_range(0.0f, 1600.0f);
		nebl_bolt(&binfo);		
		count--;
	}
	*/
}

// do all shockwaves for a ship blowing up
void shipfx_do_shockwave_stuff(ship *shipp, shockwave_create_info *sci)
{
	ship_info *sip;
	object *objp;
	polymodel *pm;
	vector temp, dir, shockwave_pos;
	vector head = vmd_zero_vector;
	vector tail = vmd_zero_vector;	
	float len, step, cur;
	int idx;

	// sanity checks
	Assert(shipp != NULL);
	if(shipp == NULL){
		return;
	} 
	Assert(shipp->ship_info_index >= 0);
	if(shipp->ship_info_index < 0){
		return;
	}	
	Assert(shipp->objnum >= 0);
	if(shipp->objnum < 0){
		return;
	}
	Assert(sci != NULL);
	if (sci == NULL) {
		return;
	}

	// get some pointers
	sip = &Ship_info[shipp->ship_info_index];
	objp = &Objects[shipp->objnum];	

	Assert(sip->shockwave_count > 0);
	if(sip->shockwave_count <= 0){
		return;
	}

	// get vectors at the head and tail of the object, dead center		
	pm = model_get(shipp->modelnum);
	if(pm == NULL){
		return;
	}
	head.xyz.x = pm->submodel[0].offset.xyz.x;
	head.xyz.y = pm->submodel[0].offset.xyz.y;
	head.xyz.z = pm->maxs.xyz.z;

	tail.xyz.x = pm->submodel[0].offset.xyz.x;
	tail.xyz.y = pm->submodel[0].offset.xyz.y;
	tail.xyz.z = pm->mins.xyz.z;

	// transform the vectors into world coords
	vm_vec_unrotate(&temp, &head, &objp->orient);
	vm_vec_add(&head, &temp, &objp->pos);
	vm_vec_unrotate(&temp, &tail, &objp->orient);
	vm_vec_add(&tail, &temp, &objp->pos);

	// now create as many shockwaves as needed
	vm_vec_sub(&dir, &head, &tail);
	len = vm_vec_mag(&dir);
	step = 1.0f / ((float)sip->shockwave_count + 1.0f);
	cur = step;
	for(idx=0; idx<sip->shockwave_count; idx++){
		// get the shockwave position		
		temp = dir;
		vm_vec_scale(&temp, cur);
		vm_vec_add(&shockwave_pos, &tail, &temp);

		// if knossos device, make shockwave in center
		if (Ship_info[shipp->ship_info_index].flags & SIF_KNOSSOS_DEVICE) {
			shockwave_pos = Objects[shipp->objnum].pos;
		}

		// create the shockwave
		shockwave_create_info sci2;
		sci2.blast = (sci->blast / (float)sip->shockwave_count) * frand_range(0.75f, 1.25f);
		sci2.damage = (sci->damage / (float)sip->shockwave_count) * frand_range(0.75f, 1.25f);
		sci2.inner_rad = sci->inner_rad;
		sci2.outer_rad = sci->outer_rad;
		sci2.speed = sci->speed * frand_range(0.75f, 1.25f);
		sci2.rot_angle = frand_range(0.0f, 359.0f);

		shockwave_create(shipp->objnum, &shockwave_pos, &sci2, SW_SHIP_DEATH, (int)frand_range(0.0f, 350.0f));
		// shockwave_create(shipp->objnum, &objp->pos, sip->shockwave_speed, sip->inner_rad, sip->outer_rad, sip->damage, sip->blast, SW_SHIP_DEATH);

		// next shockwave
		cur += step;
	}
}

int Wash_on = 1;
DCF_BOOL(engine_wash, Wash_on);
#define ENGINE_WASH_CHECK_INTERVAL		250	// (4x sec)
// Do engine wash effect for ship
// Assumes length of engine wash is greater than radius of engine wash hemisphere
void engine_wash_ship_process(ship *shipp)
{
	int idx, j;		
	object *objp, *ship_objp, *max_ship_intensity_objp;
	int started_with_no_wash = shipp->wash_intensity <= 0 ? 1 : 0;

	if (!Wash_on) {
		return;
	}

	Assert(shipp != NULL);
	Assert(shipp->objnum >= 0);
	objp = &Objects[shipp->objnum];
	ship_obj *so;
	ship_objp = NULL;

	vector world_thruster_pos, world_thruster_norm, apex, thruster_to_ship, apex_to_ship, temp;
	float dist_sqr, inset_depth, dot_to_ship, max_ship_intensity;
	polymodel *pm;

	float max_wash_dist, half_angle, radius_mult;

	// if this is not a fighter or bomber, we don't care
	if ((objp->type != OBJ_SHIP) || !(Ship_info[shipp->ship_info_index].flags & (SIF_FIGHTER|SIF_BOMBER)) ) {
		return;
	}

	// is it time to check for engine wash 
	int time_to_next_hit = timestamp_until(shipp->wash_timestamp);
	if (time_to_next_hit < 0) {
		if (time_to_next_hit < -ENGINE_WASH_CHECK_INTERVAL) {
			time_to_next_hit = 0;
		}

		// keep interval constant independent of framerate
		shipp->wash_timestamp = timestamp(ENGINE_WASH_CHECK_INTERVAL + time_to_next_hit);

		// initialize wash params
		shipp->wash_intensity = 0.0f;
		vm_vec_zero(&shipp->wash_rot_axis);
		max_ship_intensity_objp = NULL;
		max_ship_intensity = 0;
	} else {
		return;
	}

	// only do damage if we're within half of the max wash distance
	int do_damage = 0;

	// go thru Ship_used_list and check if we're in wash from CAP or SUPERCAP (SIF_HUGE)
	for (so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so)) {
		ship_objp = &Objects[so->objnum];

		// don't do small ships
		if ( (Ship_info[Ships[ship_objp->instance].ship_info_index].flags & SIF_SMALL_SHIP) ) {
			continue;
		}

		pm = model_get(Ships[ship_objp->instance].modelnum);
		float ship_intensity = 0;

		// if engines disabled, no engine wash
		if (ship_get_subsystem_strength(&Ships[ship_objp->instance], SUBSYSTEM_ENGINE) < 0.01) {
			continue;
		}

		float	speed_scale;
		if (ship_objp->phys_info.speed > 20.0f)
			speed_scale = 1.0f;
		else
			speed_scale = ship_objp->phys_info.speed/20.0f;

		for (idx = 0; idx < pm->n_thrusters; idx++) {
			thruster_bank *bank = &pm->thrusters[idx];

			// check if thruster bank has engine wash
			if (bank->wash_info_index < 0) {
				// if huge, give default engine wash
				if (Ship_info[Ships[ship_objp->instance].ship_info_index].flags & SIF_HUGE_SHIP) {
					bank->wash_info_index = 0;
					nprintf(("wash", "Adding default engine wash to ship %s", Ship_info[Ships[ship_objp->instance].ship_info_index].name));
				} else {
					continue;
				}
			}

			engine_wash_info *ewp = &Engine_wash_info[bank->wash_info_index];
			half_angle = ewp->angle;
			radius_mult = ewp->radius_mult;

			for (j=0; j<bank->num_slots; j++) {
				// get world pos of thruster
				vm_vec_unrotate(&world_thruster_pos, &bank->pnt[j], &ship_objp->orient);
				vm_vec_add2(&world_thruster_pos, &ship_objp->pos);
				
				// get world norm of thruster;
				vm_vec_unrotate(&world_thruster_norm, &bank->norm[j], &ship_objp->orient);

				// get vector from thruster to ship
				vm_vec_sub(&thruster_to_ship, &objp->pos, &world_thruster_pos);

				// check if on back side of thruster
				dot_to_ship = vm_vec_dotprod(&thruster_to_ship, &world_thruster_norm);
				if (dot_to_ship > 0) {

					// get max wash distance
					max_wash_dist = max(ewp->length, bank->radius[j]*ewp->radius_mult);

					// check if within dist range
					dist_sqr = vm_vec_mag_squared(&thruster_to_ship);
					if (dist_sqr < max_wash_dist*max_wash_dist) {

						// check if inside the sphere
						if (dist_sqr < radius_mult*radius_mult*bank->radius[j]*bank->radius[j]) {
							vm_vec_crossprod(&temp, &world_thruster_norm, &thruster_to_ship);
							vm_vec_scale_add2(&shipp->wash_rot_axis, &temp, dot_to_ship / dist_sqr);
//							shipp->wash_intensity += (1.0f - dist_sqr / (max_wash_dist*max_wash_dist));
							ship_intensity += (1.0f - dist_sqr / (max_wash_dist*max_wash_dist));
							if (!do_damage) {
								if (dist_sqr < 0.25 * max_wash_dist * max_wash_dist) {
									do_damage = 1;
								}
							}
						} else {
							// check if inside cone - first fine apex of cone
							inset_depth = float(bank->radius[j] / tan(half_angle));
							vm_vec_scale_add(&apex, &world_thruster_pos, &world_thruster_norm, -inset_depth);
							vm_vec_sub(&apex_to_ship, &objp->pos, &apex);
							vm_vec_normalize(&apex_to_ship);

							// check if inside cone angle
							if (vm_vec_dotprod(&apex_to_ship, &world_thruster_norm) > cos(half_angle)) {
								vm_vec_crossprod(&temp, &world_thruster_norm, &thruster_to_ship);
								vm_vec_scale_add2(&shipp->wash_rot_axis, &temp, dot_to_ship / dist_sqr);
//								shipp->wash_intensity += (1.0f - dist_sqr / (max_wash_dist*max_wash_dist));
								ship_intensity += (1.0f - dist_sqr / (max_wash_dist*max_wash_dist));
								if (!do_damage) {
									if (dist_sqr < 0.25 * max_wash_dist * max_wash_dist) {
										do_damage = 1;
									}
								}
							}
						}
					}
				}
			}
		}
		shipp->wash_intensity += ship_intensity * speed_scale;
		if (ship_intensity > max_ship_intensity) {
			max_ship_intensity = ship_intensity;
			max_ship_intensity_objp = ship_objp;
		}
	}

	// apply damage at rate of 1%/sec
	if (shipp->wash_intensity > 0) {
		Assert(max_ship_intensity_objp != NULL);

		nprintf(("wash", "Wash intensity %.2f\n", shipp->wash_intensity));

		float damage;
		if (!do_damage) {
			damage = 0;
		} else {
			damage = (0.001f * 0.003f * ENGINE_WASH_CHECK_INTERVAL * Ship_info[shipp->ship_info_index].initial_hull_strength * shipp->wash_intensity);
		}

		ship_apply_wash_damage(&Objects[shipp->objnum], max_ship_intensity_objp, damage);

		// if we had no wash before now, add the wash object sound
		if(started_with_no_wash){
			if(shipp != Player_ship){
#ifndef NO_SOUND			
				obj_snd_assign(shipp->objnum, SND_ENGINE_WASH, &vmd_zero_vector, 1);
#endif
			} else {				
				Player_engine_wash_loop = snd_play_looping( &Snds[SND_ENGINE_WASH], 0.0f , -1, -1, 1.0f);
			}
		}
	} 
	// if we've got no wash, kill any wash object sounds from this guy
	else {
		if(shipp != Player_ship){
#ifndef NO_SOUND			
			obj_snd_delete(shipp->objnum, SND_ENGINE_WASH);
#endif
		} else {
			snd_stop(Player_engine_wash_loop);
			Player_engine_wash_loop = -1;
		}
	}
}

// engine wash level init
void shipfx_engine_wash_level_init()
{
	Player_engine_wash_loop = -1;
}

// pause engine wash sounds
void shipfx_stop_engine_wash_sound()
{
	if(Player_engine_wash_loop != -1){
		snd_stop(Player_engine_wash_loop);
		Player_engine_wash_loop = -1;
	}
}
