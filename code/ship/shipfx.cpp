/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "ship/shipfx.h"

#include "globalincs/linklist.h"

#include "asteroid/asteroid.h"
#include "bmpman/bmpman.h"
#include "cmdline/cmdline.h"
#include "debris/debris.h"
#include "debugconsole/console.h"
#include "fireball/fireballs.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "hud/hudmessage.h"
#include "io/timer.h"
#include "lighting/lighting.h"
#include "math/fvi.h"
#include "mod_table/mod_table.h"
#include "model/model.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "object/object.h"
#include "object/objectdock.h"
#include "object/objectsnd.h"
#include "parse/parselo.h"
#include "particle/particle.h"
#include "playerman/player.h"
#include "render/3d.h" // needed for View_position, which is used when playing a 3D sound
#include "render/batching.h"
#include "scripting/hook_api.h"
#include "ship/ship.h"
#include "ship/shiphit.h"
#include "utils/Random.h"
#include "weapon/muzzleflash.h"
#include "weapon/shockwave.h"
#include "weapon/weapon.h"

#ifndef NDEBUG
extern float flFrametime;
extern int Framecount;
#endif

extern int Cmdline_tbp;

#define SHIP_CANNON_BITMAP "argh"
int Ship_cannon_bitmap = -1;

sound_handle Player_engine_wash_loop = sound_handle::invalid();

extern float splode_level;

const auto OnWarpOutHook = scripting::OverridableHook::Factory(
	"On Warp Out", "Called when a ship warps out", {{"Self", "ship", "The object that is warping out."}});

const auto OnWarpInHook = scripting::OverridableHook::Factory(
	"On Warp In", "Called when a ship warps in", {{"Self", "ship", "The object that is warping in."}});

static void shipfx_remove_submodel_ship_sparks(ship* shipp, int submodel_num)
{
	Assert(submodel_num != -1);

	// maybe no active sparks on submodel
	if (shipp->num_hits == 0) {
		return;
	}

	for (int spark_num = 0; spark_num < shipp->num_hits; spark_num++) {
		if (shipp->sparks[spark_num].submodel_num == submodel_num) {
			shipp->sparks[spark_num].end_time = timestamp(1);
		}
	}
}

void model_get_rotating_submodel_axis(vec3d *model_axis, vec3d *world_axis, const polymodel *pm, const polymodel_instance *pmi, int submodel_num, matrix *objorient);

/**
 * Check if subsystem has live debris and create
 *
 * DKA: 5/26/99 make velocity of debris scale according to size of debris subobject (at least for large subobjects)
 */
static void shipfx_subsystem_maybe_create_live_debris(object *ship_objp, ship *ship_p, ship_subsys *subsys, vec3d *exp_center, float exp_mag)
{
	// initializations
	ship *shipp = &Ships[ship_objp->instance];
	polymodel *pm = model_get(Ship_info[ship_p->ship_info_index].model_num);
	polymodel_instance *pmi = model_get_instance(shipp->model_instance_num);
	int submodel_num = subsys->system_info->subobj_num;
	submodel_instance *smi = &pmi->submodel[submodel_num];

	object *live_debris_obj;
	int i, num_live_debris, live_debris_submodel;

	// get number of live debris objects to create
	num_live_debris = pm->submodel[submodel_num].num_live_debris;
	if ((num_live_debris <= 0) || (subsys->flags[Ship::Subsystem_Flags::No_live_debris])) {
		return;
	}

	// make sure the axis point is set
	vec3d model_axis, world_axis, rotvel, world_axis_pt;
	matrix m_rot;	// rotation for debris orient about axis

	if (pm->submodel[submodel_num].rotation_type == MOVEMENT_TYPE_REGULAR || pm->submodel[submodel_num].rotation_type == MOVEMENT_TYPE_INTRINSIC) {
		if ( !smi->axis_set ) {
			model_init_submodel_axis_pt(pm, pmi, submodel_num);
		}

		// get the rotvel
		model_get_rotating_submodel_axis(&model_axis, &world_axis, pm, pmi, submodel_num, &ship_objp->orient);
		vm_vec_copy_scale(&rotvel, &world_axis, smi->current_turn_rate);

		model_instance_local_to_global_point(&world_axis_pt, &smi->point_on_axis, pm, pmi, submodel_num, &ship_objp->orient, &ship_objp->pos);

		vm_quaternion_rotate(&m_rot, smi->cur_angle, &model_axis);
	} else {
		//fix to allow non rotating submodels to use live debris
		vm_vec_zero(&rotvel);
		vm_set_identity(&m_rot);
		vm_vec_zero(&world_axis_pt);
	}

	// create live debris pieces
	for (i=0; i<num_live_debris; i++) {
		live_debris_submodel = pm->submodel[submodel_num].live_debris[i];
		vec3d start_world_pos, start_model_pos, end_world_pos;

		// get start world pos
		vm_vec_zero(&start_world_pos);
		model_instance_local_to_global_point(&start_world_pos, &pm->submodel[live_debris_submodel].offset, pm, pmi, live_debris_submodel, &ship_objp->orient, &ship_objp->pos );

		// convert to model coord of underlying submodel
		model_instance_global_to_local_point(&start_model_pos, &start_world_pos, pm, pmi, submodel_num, &ship_objp->orient, &ship_objp->pos);

		// rotate from submodel coord to world coords
		model_instance_local_to_global_point(&end_world_pos, &start_model_pos, pm, pmi, submodel_num, &ship_objp->orient, &ship_objp->pos);

		int fireball_type = fireball_ship_explosion_type(&Ship_info[ship_p->ship_info_index]);
		if(fireball_type < 0) {
			fireball_type = FIREBALL_EXPLOSION_MEDIUM;
		}
		// create fireball here.
		fireball_create(&end_world_pos, fireball_type, FIREBALL_MEDIUM_EXPLOSION, OBJ_INDEX(ship_objp), pm->submodel[live_debris_submodel].rad);

		// create debris
		live_debris_obj = debris_create(ship_objp, pm->id, live_debris_submodel, &end_world_pos, exp_center, 1, exp_mag);

		// only do if debris is created
		if (live_debris_obj) {
			// get radial velocity of debris
			vec3d delta_x, radial_vel;
			vm_vec_sub(&delta_x, &end_world_pos, &world_axis_pt);
			vm_vec_cross(&radial_vel, &rotvel, &delta_x);

			if (Ship_info[ship_p->ship_info_index].flags[Ship::Info_Flags::Knossos_device]) {
				// set velocity to cross center of knossos device
				vec3d rand_vec, vec_to_center;

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
				vec3d temp_vel;	// explosion velocity with ship_obj velocity removed
				vm_vec_sub(&temp_vel, &live_debris_obj->phys_info.vel, &ship_objp->phys_info.vel);

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

					if (Ship_info[ship_p->ship_info_index].flags[Ship::Info_Flags::Knossos_device]) {
						scale = 1.0f;
					}

					vm_vec_scale_add2(&live_debris_obj->phys_info.vel, &radial_vel, scale);
				}

				// scale up speed of debris if ship_obj > 125, but not for knossos
				if (ship_objp->radius > 250 && !(Ship_info[ship_p->ship_info_index].flags[Ship::Info_Flags::Knossos_device])) {
					vm_vec_scale(&live_debris_obj->phys_info.vel, ship_objp->radius/250.0f);
				}
			}

			shipfx_debris_limit_speed(&Debris[live_debris_obj->instance], ship_p);
		}
	}
}

/**
 * Create debris for ship submodel which has live debris (at ship death)
 * when ship submodel has not already been blown off (and hence liberated live debris)
 */
static void shipfx_maybe_create_live_debris_at_ship_death( object *ship_objp )
{
	// if ship has live debris, detonate that subsystem now
	// search for any live debris

	ship *shipp = &Ships[ship_objp->instance];
	polymodel *pm = model_get(Ship_info[shipp->ship_info_index].model_num);
	polymodel_instance *pmi = model_get_instance(shipp->model_instance_num);

	// no subsystems -> no live debris.
	if (Ship_info[shipp->ship_info_index].n_subsystems == 0) {
		return;
	}

	int live_debris_submodel = -1;
	for (int idx=0; idx<pm->num_debris_objects; idx++) {
		if (pm->submodel[pm->debris_objects[idx]].flags[Model::Submodel_flags::Is_live_debris]) {
			live_debris_submodel = pm->debris_objects[idx];

			// get submodel that produces live debris
			int model_get_parent_submodel_for_live_debris( int model_num, int live_debris_model_num );
			int parent = model_get_parent_submodel_for_live_debris(pm->id, live_debris_submodel);
			Assert(parent != -1);

			// check if already blown off  (ship model set)
			if ( !pmi->submodel[parent].blown_off ) {
		
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
					if (pss->system_info != NULL) {
						vec3d exp_center, tmp = ZERO_VECTOR;
						model_instance_local_to_global_point(&exp_center, &tmp, pm, pmi, parent, &ship_objp->orient, &ship_objp->pos );

						// if not blown off, blow it off
						shipfx_subsystem_maybe_create_live_debris(ship_objp, shipp, pss, &exp_center, 3.0f);

						// now set subsystem as blown off, so we only get one copy
						pmi->submodel[parent].blown_off = true;
					}
				}
			}
		}
	}
}

void shipfx_blow_off_subsystem(object *ship_objp, ship *ship_p, ship_subsys *subsys, vec3d *exp_center, bool no_explosion)
{
	vec3d subobj_pos;

	model_subsystem	*psub = subsys->system_info;

	get_subsystem_world_pos(ship_objp, subsys, &subobj_pos);

	// get rid of sparks on submodel that is destroyed
	shipfx_remove_submodel_ship_sparks(ship_p, psub->subobj_num);

	// create debris shards
    if (!(subsys->flags[Ship::Subsystem_Flags::Vanished]) && !no_explosion) {
		shipfx_blow_up_model(ship_objp, psub->subobj_num, 50, &subobj_pos );

		// create live debris objects, if any
		// TODO:  some MULTIPLAYER implcations here!!
		shipfx_subsystem_maybe_create_live_debris(ship_objp, ship_p, subsys, exp_center, 1.0f);
		
		int fireball_type = fireball_ship_explosion_type(&Ship_info[ship_p->ship_info_index]);
		if(fireball_type < 0) {
			fireball_type = FIREBALL_EXPLOSION_MEDIUM;
		}
		// create first fireball
		fireball_create( &subobj_pos, fireball_type, FIREBALL_MEDIUM_EXPLOSION, OBJ_INDEX(ship_objp), psub->radius );
	}
}

static void shipfx_blow_up_hull(object *obj, polymodel *pm, polymodel_instance *pmi, vec3d *exp_center)
{
	int i;
	ushort sig_save;

	if (!pm) return;

	// in multiplayer, send a debris_hull_create packet.  Save/restore the debris signature
	// when in misison only (since we can create debris pieces before mission starts)
	sig_save = 0;

	if ( (Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_IN_MISSION) ) {
		sig_save = multi_get_next_network_signature( MULTI_SIG_DEBRIS );
		multi_set_network_signature( Ships[obj->instance].debris_net_sig, MULTI_SIG_DEBRIS );
	}

	bool try_live_debris = true;
	for (i=0; i<pm->num_debris_objects; i++ )	{
		if (! pm->submodel[pm->debris_objects[i]].flags[Model::Submodel_flags::Is_live_debris]) {
			vec3d tmp = ZERO_VECTOR;
			model_instance_local_to_global_point(&tmp, &pm->submodel[pm->debris_objects[i]].offset, pm, pmi, 0, &obj->orient, &obj->pos );
			debris_create( obj, pm->id, pm->debris_objects[i], &tmp, exp_center, 1, 3.0f );
		} else {
			if ( try_live_debris ) {
				// only create live debris once
				// this creates *all* the live debris for *all* the currently live subsystems.
				try_live_debris = false;
				shipfx_maybe_create_live_debris_at_ship_death(obj);
			}
		}
		// in multiplayer we need to increment the network signature for each piece of debris we create
		if ( (Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_IN_MISSION) ) {
			multi_assign_network_signature(MULTI_SIG_DEBRIS);
		}
	}

	// restore the debris signature to it's original value.
	if ( (Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_IN_MISSION) ) {
		multi_set_network_signature( sig_save, MULTI_SIG_DEBRIS );
	}
}


/**
 * Creates "ndebris" pieces of debris on random verts of the the "submodel" in the ship's model.
 */
void shipfx_blow_up_model(object *obj, int submodel, int ndebris, vec3d *exp_center)
{
	int i;

	auto pmi = model_get_instance(Ships[obj->instance].model_instance_num);
	auto pm = model_get(pmi->model_num);

	// if in a multiplayer game -- seed the random number generator with a value that will be the same
	// on all clients in the game -- the net_signature of the object works nicely -- since doing so should
	// ensure that all pieces of debris will get scattered in same direction on all machines
	if ( Game_mode & GM_MULTIPLAYER )
		Random::seed( obj->net_signature );

	// made a change to allow anyone but multiplayer client to blow up hull.  Clients will do it when
	// they get the create packet
	bool use_ship_debris = false;
	if ( submodel == 0 ) {
		shipfx_blow_up_hull(obj, pm, pmi, exp_center);
		use_ship_debris = true;
	}

	for (i=0; i<ndebris; i++ )	{
		vec3d pnt1, pnt2;

		// Gets two random points on the surface of a submodel
		submodel_get_two_random_points_better(pm->id, submodel, &pnt1, &pnt2);

		vec3d tmp, outpnt;

		vm_vec_avg( &tmp, &pnt1, &pnt2 );
		model_instance_local_to_global_point(&outpnt, &tmp, pm, pmi, submodel, &obj->orient, &obj->pos );

		debris_create( obj, use_ship_debris ? Ship_info[Ships[obj->instance].ship_info_index].generic_debris_model_num : -1, -1, &outpnt, exp_center, 0, 1.0f );
	}
}


// =================================================
//          SHIP WARP IN EFFECT CODE
// =================================================


/**
 * Given an ship, find the radius of it as viewed from the front.
 */
static float shipfx_calculate_effect_radius( object *objp, WarpDirection warp_dir )
{
	float rad;

	// if docked, we need to calculate the overall cross-sectional radius around the z-axis (longitudinal axis)
	if (object_is_docked(objp))
	{
		rad = dock_calc_max_cross_sectional_radius_perpendicular_to_axis(objp, Z_AXIS);
	}
	// if it's not docked, we can save a lot of work by just using width and height
	else
	{
		ship *shipp = &Ships[objp->instance];

		//WMC - see if a radius was specified
		float warp_radius = Warp_params[warp_dir == WarpDirection::WARP_IN ? shipp->warpin_params_index : shipp->warpout_params_index].radius;
		if (warp_radius > 0.0f)
			return warp_radius;

		float w, h;
		polymodel *pm = model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num);

		w = pm->maxs.xyz.x - pm->mins.xyz.x;
		h = pm->maxs.xyz.y - pm->mins.xyz.y;
	
		if ( w > h )
			rad = w / 2.0f;
		else
			rad = h / 2.0f;
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

float shipfx_calculate_warp_time(object *objp, WarpDirection warp_dir, float half_length, float warping_dist)
{
	WarpParams *params = &Warp_params[warp_dir == WarpDirection::WARP_IN ? Ships[objp->instance].warpin_params_index : Ships[objp->instance].warpout_params_index];

	// warpin or warpout time defined
	if (params->time > 0.0f) {
		return (float)params->time / 1000.0f;
	}
	// warpin or warpout speed defined
	else if (params->speed > 0.0f) {
		return warping_dist / params->speed;
	}
	// Player warpout
	else if ((warp_dir == WarpDirection::WARP_OUT) && (objp == Player_obj)) {
		if (params->warpout_player_speed > 0.0f) {
			return warping_dist / params->warpout_player_speed;
		} else {
			return warping_dist / Player_warpout_speed;
		}
	}

	// Find rad_percent from 0 to 1, 0 being smallest ship, 1 being largest
	float rad_percent = (half_length-SMALLEST_RAD) / (LARGEST_RAD-SMALLEST_RAD);
    CLAMP(rad_percent, 0.0f, 1.0f);

	float rad_time = rad_percent*(LARGEST_RAD_TIME-SMALLEST_RAD_TIME) + SMALLEST_RAD_TIME;

	return rad_time;
}

// This is called to actually warp this object in
// after all the flashy fx are done, or if the flashy 
// fx don't work for some reason.
void shipfx_actually_warpin(ship *shipp, object *objp)
{
	shipp->flags.remove(Ship::Ship_Flags::Arriving_stage_1);
	shipp->flags.remove(Ship::Ship_Flags::Arriving_stage_2);
	// dock leader needs to handle dockees
	if (object_is_docked(objp)) {
		Assertion(shipp->flags[Ship::Ship_Flags::Dock_leader], "The docked ship warping in (%s) should only be the dock leader at this point!\n", shipp->ship_name);
		dock_function_info dfi;
		dock_evaluate_all_docked_objects(objp, &dfi, object_remove_arriving_stage1_ndl_flag_helper);
		dock_evaluate_all_docked_objects(objp, &dfi, object_remove_arriving_stage2_ndl_flag_helper);
	}

	// let physics in on it too.
	objp->phys_info.flags &= (~PF_WARP_IN);
}

// Validate reference_objnum
static int shipfx_special_warp_objnum_valid(int objnum)
{
	object *special_objp;

	// must be a valid object
	if ((objnum < 0) || (objnum >= MAX_OBJECTS))
		return 0;

	special_objp = &Objects[objnum];

	// must be a ship
	if (special_objp->type != OBJ_SHIP)
		return 0;

	// must be a knossos
	if (!(Ship_info[Ships[special_objp->instance].ship_info_index].flags[Ship::Info_Flags::Knossos_device]))
		return 0;

	return 1;
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
	ship *shipp = &Ships[objp->instance];

	if (shipp->is_arriving())
	{
		mprintf(( "Ship '%s' is already arriving!\n", shipp->ship_name ));
		Int3();
		return;
	}

	// docked ships who are not dock leaders don't use the warp effect code
	// (the dock leader takes care of the whole group)
	if (object_is_docked(objp) && !(shipp->flags[Ship::Ship_Flags::Dock_leader]))
	{
		return;
	}

	//WMC - Check if scripting handles this.
	if(OnWarpInHook->isOverride(scripting::hook_param_list(scripting::hook_param("Self", 'o', objp)), objp))
	{
		OnWarpInHook->run(scripting::hook_param_list(scripting::hook_param("Self", 'o', objp)), objp);
		return;
	}

	// if there is no arrival warp, then skip the whole thing
	if (shipp->flags[Ship::Ship_Flags::No_arrival_warp])
	{
		shipfx_actually_warpin(shipp,objp);
		return;
	}

	Assertion(shipp->warpin_effect != nullptr, "shipfx_warpin_start() was fed a ship with an uninitialized warpin_effect.");
	shipp->warpin_effect->warpStart();

	OnWarpInHook->run(scripting::hook_param_list(scripting::hook_param("Self", 'o', objp)), objp);
}

void shipfx_warpin_frame( object *objp, float frametime )
{
	ship *shipp;

	shipp = &Ships[objp->instance];

	if ( shipp->flags[Ship::Ship_Flags::Dying] ) return;

	shipp->warpin_effect->warpFrame(frametime);
}
 
// This is called to actually warp this object out
// after all the flashy fx are done, or if the flashy fx
// don't work for some reason.  OR to skip the flashy fx.
static void shipfx_actually_warpout(int shipnum)
{
	// Once we get through effect, make the ship go away
	ship_actually_depart(shipnum);
}

void WE_Default::compute_warpout_stuff(float *warp_time, vec3d *warp_pos)
{
	float warp_dist(0.0f), dist_to_plane, ship_move_dist;
	vec3d facing_normal, vec_to_knossos, center_pos;

	// find world position of the center of the ship assembly
	vm_vec_unrotate(&center_pos, &actual_local_center, &objp->orient);
	vm_vec_add2(&center_pos, &objp->pos);

	// If we're warping through the knossos, do something different.
	if (portal_objp != nullptr)
	{
		// get facing normal from knossos
		vm_vec_sub(&vec_to_knossos, &portal_objp->pos, &center_pos);
		facing_normal = portal_objp->orient.vec.fvec;
		if (vm_vec_dot(&vec_to_knossos, &portal_objp->orient.vec.fvec) > 0.0f) {
			vm_vec_negate(&facing_normal);
		}

		// find position to play the warp ani..
		dist_to_plane = fvi_ray_plane(warp_pos, &portal_objp->pos, &facing_normal, &center_pos, &objp->orient.vec.fvec, 0.0f);

		// calculate distance to warpout point.
		dist_to_plane -= half_length;

		if (dist_to_plane < 0.0f) {
			mprintf(("special warpout started too late\n"));
			dist_to_plane = 0.0f;
		}

		// validate angle
		float max_warpout_angle = 0.707f;	// 45 degree half-angle cone for small ships
		if (Ship_info[Ships[objp->instance].ship_info_index].is_big_or_huge()) {
			max_warpout_angle = 0.866f;	// 30 degree half-angle cone for BIG or HUGE
		}

		if (-vm_vec_dot(&objp->orient.vec.fvec, &facing_normal) < max_warpout_angle) {	// within allowed angle
			mprintf(("special warpout angle exceeded\n"));
		}

		ship_move_dist = dist_to_plane;
	}
	// normal warp
	else
	{
		// If this is a huge ship, set the distance to the length of the ship
		if (sip->is_huge_ship())
		{
			warp_dist = half_length * 2.0f;
		}
		else
		{
			// Now we know our speed. Figure out how far the warp effect will be from here.
			warp_dist = (warping_speed * SHIPFX_WARP_DELAY) + half_length * 1.5f;		// We want to get to 1.5R away from effect
		}

		ship_move_dist = warp_dist - half_length;
	}

	// Calculate how long to fly through the effect.  Not to get to the effect, just through it.
	*warp_time = warping_time;

	// Acount for time to get to warp effect, before we actually go through it.
	*warp_time += ship_move_dist / warping_speed;

	if (portal_objp == nullptr)
	{
		// project the warp portal in front of us
		vm_vec_scale_add(warp_pos, &center_pos, &objp->orient.vec.fvec, warp_dist);
	}
}

// JAS - code to start the ship doing the warp out effect
// This puts the ship into a mode specified by SF_DEPARTING
// where it flies forward for a set time period at a set
// velocity, then disappears when that time is reached.  This
// also starts the animating 3d effect playing.
void shipfx_warpout_start( object *objp )
{
	ship* shipp;
	shipp = &Ships[objp->instance];

	if (shipp->flags[Ship::Ship_Flags::Depart_warp]) {
		mprintf(("Ship is already departing!\n"));
		return;
	}

	if (OnWarpOutHook->isOverride(scripting::hook_param_list(scripting::hook_param("Self", 'o', objp)), objp)) {
		OnWarpOutHook->run(scripting::hook_param_list(scripting::hook_param("Self", 'o', objp)), objp);
		return;
	}

	// if we're dying return
	if (shipp->flags[Ship::Ship_Flags::Dying]) {
		return;
	}

	// return if disabled
	if (shipp->flags[Ship::Ship_Flags::Disabled]) {
		return;
	}

	// if we're HUGE, keep alive - set guardian
	if (Ship_info[shipp->ship_info_index].is_huge_ship()) {
		shipp->ship_guardian_threshold = SHIP_GUARDIAN_THRESHOLD_DEFAULT;
	}

	// don't send ship depart packets for player ships
	if ((MULTIPLAYER_MASTER) && !(objp->flags[Object::Object_Flags::Player_ship])) {
		send_ship_depart_packet(objp);
	}

	// don't do departure wormhole if ship flag is set which indicates no effect
	if (shipp->flags[Ship::Ship_Flags::No_departure_warp]) {
		// DKA 5/25/99 If he's going to warpout, set it.
		// Next line fixes assert in wing cleanup code when no warp effect.
		shipp->flags.set(Ship::Ship_Flags::Depart_warp);

		shipfx_actually_warpout(objp->instance);
		return;
	}

	Assertion(shipp->warpout_effect != nullptr,
			  "shipfx_warpout_start() was fed a ship with an uninitialized warpout_effect.");
	shipp->warpout_effect->warpStart();

	OnWarpOutHook->run(scripting::hook_param_list(scripting::hook_param("Self", 'o', objp)), objp);
}

void shipfx_warpout_frame( object *objp, float frametime )
{
	ship *shipp;
	shipp = &Ships[objp->instance];

	if ( shipp->flags[Ship::Ship_Flags::Dying] ) return;

	//disabled ships should stay on the battlefield if they were disabled during warpout
	//phreak 5/22/03
	if (shipp->flags[Ship::Ship_Flags::Disabled]){
        shipp->flags.remove(Ship::Ship_Flags::Depart_dockbay);
        shipp->flags.remove(Ship::Ship_Flags::Depart_warp);
		return;
	}

	shipp->warpout_effect->warpFrame(frametime);
}


//==================================================
// Stuff for keeping track of which ships are in
// whose shadows.


#define w(p)	(*((int *) (p)))
/**
 * Given world point see if it is in a shadow.
 */
bool shipfx_eye_in_shadow( vec3d *eye_pos, object * src_obj, int sun_n )
{
	mc_info mc;
	object *objp;
	ship_obj *so;

	vec3d rp0, rp1;
	vec3d light_dir;

	// The mc_info struct only needs to be initialized once for this entire function.  This is because
	// every time the mc variable is reused, every parameter that model_collide reads from is reassigned.
	// Therefore the stale fields in the rest of the struct do not matter because either a) they are never
	// read from, or b) they are overwritten by the new collision calculation.
	mc_info_init(&mc);

	rp0 = *eye_pos;	
	
	// get the light dir
	if(!light_get_global_dir(&light_dir, sun_n)){
		return false;
	}

	// Find rp1
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) )	{
		objp = &Objects[so->objnum];

		if ( src_obj != objp )	{
			vm_vec_scale_add( &rp1, &rp0, &light_dir, objp->radius*10.0f );

			mc.model_instance_num = Ships[objp->instance].model_instance_num;
			mc.model_num = Ship_info[Ships[objp->instance].ship_info_index].model_num;
			mc.orient = &objp->orient;
			mc.pos = &objp->pos;
			mc.p0 = &rp0;
			mc.p1 = &rp1;
			mc.flags = MC_CHECK_MODEL;	

			if (model_collide(&mc)) {
				return true;
			}
		}
	}

	// Check all the big hull debris pieces.
	for (auto &db: Debris)	{
		if ( !(db.flags[Debris_Flags::Used]) || !db.is_hull ){
			continue;
		}

		objp = &Objects[db.objnum];

		vm_vec_scale_add( &rp1, &rp0, &light_dir, objp->radius*10.0f );

		mc.model_instance_num = -1;
		mc.model_num = db.model_num;	// Fill in the model to check
		mc.submodel_num = db.submodel_num;
		mc.orient = &objp->orient;					// The object's orient
		mc.pos = &objp->pos;							// The object's position
		mc.p0 = &rp0;				// Point 1 of ray to check
		mc.p1 = &rp1;					// Point 2 of ray to check
		mc.flags = (MC_CHECK_MODEL | MC_SUBMODEL);

		if (model_collide(&mc))	{
			return true;
		}
	}

	// check cockpit model
	if( Viewer_obj != NULL && Viewer_mode != VM_TOPDOWN ) {
		if ( Viewer_obj->type == OBJ_SHIP && Viewer_obj->instance >= 0 ) {
			ship *shipp = &Ships[Viewer_obj->instance];
			ship_info *sip = &Ship_info[shipp->ship_info_index];

			if(sip->cockpit_model_num > 0) {
				vm_vec_scale_add( &rp1, &rp0, &light_dir, Viewer_obj->radius*2.0f );
				vec3d pos,eye_posi;
				matrix eye_ori;
				ship_get_eye(&eye_posi, &eye_ori, Viewer_obj, false);
				vm_vec_unrotate(&pos, &sip->cockpit_offset, &eye_ori);
				vm_vec_add2(&pos, &eye_posi);

				mc.model_instance_num = -1;
				mc.model_num = sip->cockpit_model_num;
				mc.submodel_num = -1;
				mc.orient = &Eye_matrix;
				mc.pos = &pos;
				mc.p0 = &rp0;
				mc.p1 = &rp1;
				mc.flags = MC_CHECK_MODEL;

				int mc_result = model_collide(&mc);
				mc.pos = NULL;

				if( mc_result ) {
					if ( mc.t_poly ) {
						polymodel *pm = model_get(sip->cockpit_model_num);
						int tmap_num = w(mc.t_poly+40);

						Assertion (tmap_num < MAX_MODEL_TEXTURES, "Texture map index (%i) exceeded max", tmap_num);
						if (tmap_num >= MAX_MODEL_TEXTURES) { return 0; }
						if( !(pm->maps[tmap_num].is_transparent) && strcmp(bm_get_filename(mc.hit_bitmap), "glass.dds") != 0 ) {
							return true;
						}
					}

					if ( mc.f_poly ) {
						 return true;
					}

					if ( mc.bsp_leaf ) {
						if ( mc.bsp_leaf->tmap_num < 255 ) {
							polymodel *pm = model_get(sip->cockpit_model_num);
							int tmap_num = mc.bsp_leaf->tmap_num;

							Assertion (tmap_num < MAX_MODEL_TEXTURES, "Texture map index (%i) exceeded max", tmap_num);
							if (tmap_num >= MAX_MODEL_TEXTURES) { return 0; }
							if ( !(pm->maps[tmap_num].is_transparent) && strcmp(bm_get_filename(mc.hit_bitmap), "glass.dds") != 0 ) {
								return true;
							}
						} else {
							return true;
						}
					}
				}
			}

			if ( sip->flags[Ship::Info_Flags::Show_ship_model] ) {
				vm_vec_scale_add( &rp1, &rp0, &light_dir, Viewer_obj->radius*10.0f );

				mc.model_instance_num = -1;
				mc.model_num = sip->model_num;
				mc.submodel_num = -1;
				mc.orient = &Viewer_obj->orient;
				mc.pos = &Viewer_obj->pos;
				mc.p0 = &rp0;
				mc.p1 = &rp1;
				mc.flags = MC_CHECK_MODEL;

				if( model_collide(&mc) ) {
					if ( mc.t_poly ) {
						polymodel *pm = model_get(sip->model_num);
						int tmap_num = w(mc.t_poly+40);

						Assertion (tmap_num < MAX_MODEL_TEXTURES, "Texture map index (%i) exceeded max", tmap_num);
						if (tmap_num >= MAX_MODEL_TEXTURES) { return 0; }
						if ( !(pm->maps[tmap_num].is_transparent) && strcmp(bm_get_filename(mc.hit_bitmap),"glass.dds") != 0 ) {
							return true;
						}
					}

					if ( mc.f_poly ) {
						 return true;
					}

					if ( mc.bsp_leaf ) {
						if ( mc.bsp_leaf->tmap_num < 255 ) {
							polymodel *pm = model_get(sip->model_num);
							int tmap_num = mc.bsp_leaf->tmap_num;

							Assertion (tmap_num < MAX_MODEL_TEXTURES, "Texture map index (%i) exceeded max", tmap_num);
							if (tmap_num >= MAX_MODEL_TEXTURES) { return 0; }
							if ( !(pm->maps[tmap_num].is_transparent) && strcmp(bm_get_filename(mc.hit_bitmap), "glass.dds") != 0 ) {
								return true;
							}
						} else {
							return true;
						}
					}
				}
			}
		}
	}

    // check asteroids
    asteroid *ast = Asteroids;

    if (Asteroid_field.num_initial_asteroids <= 0 )
    {
        return false;
    }

    for (int i = 0 ; i < MAX_ASTEROIDS; i++, ast++)
    {
        if (!(ast->flags & AF_USED))
        {
            continue;
        }

        objp = &Objects[ast->objnum];

        vm_vec_scale_add( &rp1, &rp0, &light_dir, objp->radius*10.0f );

		mc.model_instance_num = -1;
		mc.model_num = Asteroid_info[ast->asteroid_type].model_num[ast->asteroid_subtype];	// Fill in the model to check
		mc.submodel_num = -1;
		mc.orient = &objp->orient;					// The object's orient
		mc.pos = &objp->pos;							// The object's position
		mc.p0 = &rp0;				// Point 1 of ray to check
		mc.p1 = &rp1;					// Point 2 of ray to check
		mc.flags = MC_CHECK_MODEL;

		if (model_collide(&mc))	{
			return true;
		}
    }

	// not in shadow
	return false;
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

/**
 * Resets the ship flash stuff. Call before each level.
 */
void shipfx_flash_init()
{
	int i;
	
	for (i=0; i<MAX_FLASHES; i++ )	{
		Ship_flash[i].objnum = -1;			// mark as unused
	}
	Ship_flash_highest = -1;
	Ship_flash_inited = 1;	
}


/**
 * Given that a ship fired a weapon, light up the model accordingly.
 */
void shipfx_flash_create(object *objp, int model_num, vec3d *gun_pos, vec3d *gun_dir, int is_primary, int weapon_info_index)
{
	int i;
	int objnum = OBJ_INDEX(objp);

	Assert(Ship_flash_inited);

	polymodel *pm = model_get(model_num);
	int closest_light = -1;
	float d, closest_dist = 0.0f;

	// ALWAYS do this - since this is called once per firing
	// if this is a cannon type weapon, create a muzzle flash
	// HACK - let the flak guns do this on their own since they fire so quickly
	// Also don't create if its the player in the cockpit unless he's also got show_ship_model, provided render_player_mflash isnt on
	bool in_cockpit_view = (Viewer_mode & (VM_EXTERNAL | VM_CHASE | VM_OTHER_SHIP | VM_WARP_CHASE)) == 0;
	bool player_show_ship_model = objp == Player_obj && Ship_info[Ships[objp->instance].ship_info_index].flags[Ship::Info_Flags::Show_ship_model];
	if ((Weapon_info[weapon_info_index].muzzle_flash >= 0) && !(Weapon_info[weapon_info_index].wi_flags[Weapon::Info_Flags::Flak]) &&
		(objp != Player_obj || Render_player_mflash || (!in_cockpit_view || player_show_ship_model))) {
		vec3d real_dir;
		vm_vec_rotate(&real_dir, gun_dir,&objp->orient);	
		mflash_create(gun_pos, &real_dir, &objp->phys_info, Weapon_info[weapon_info_index].muzzle_flash, objp);		
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

/**
 * Does whatever processing needs to be done each frame.
 */
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
DCF(particle_width, "Sets multiplier for angular width of the particle spew ( 0 - 5)")
{
	float value;

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Particle_width : %f\n", Particle_width);
		return;
	}

	dc_stuff_float(&value);

	CLAMP(value, 0.0, 5.0);
	Particle_width = value;

	dc_printf("Particle_width set to %f\n", Particle_width);
}

float Particle_number = 1.2f;
DCF(particle_num, "Sets multiplier for the number of particles created")
{
	
	float value;

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Particle_number : %f\n", Particle_number);
		return;
	}

	dc_stuff_float(&value);

	CLAMP(value, 0.0, 5.0);
	Particle_number = value;

	dc_printf("Particle_number set to %f\n", Particle_number);
}

float Particle_life = 1.2f;
DCF(particle_life, "Multiplier for the lifetime of particles created")
{
	float value;

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Particle_life : %f\n", Particle_life);
		return;
	}

	dc_stuff_float(&value);

	CLAMP(value, 0.0, 5.0);
	Particle_life = value;

	dc_printf("Particle_life set to %f\n", Particle_life);
}

// Make sparks fly off of ship n.
// sn = spark number to spark, corrosponding to element in
//      ship->hitpos array.  If this isn't -1, it is a just
//      got hit by weapon spark, otherwise pick one randomally.
void shipfx_emit_spark( int n, int sn )
{
	int create_spark = 1;
	object * obj;
	vec3d outpnt;
	ship *shipp = &Ships[n];
	float ship_radius, spark_scale_factor;

	ship_info *sip = &Ship_info[shipp->ship_info_index];
	if(sn > -1 && sip->impact_spew.n_high <= 0)
		return;

	if(sn < 0 && sip->damage_spew.n_high <= 0)
		return;
	
	if ( shipp->num_hits <= 0 )
		return;

	// get radius of ship
	ship_radius = model_get_radius(sip->model_num);

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

	float hull_percent = get_hull_pct(obj);
	if (hull_percent < 0.001) {
		hull_percent = 0.001f;
	}
	float fraction = 0.1f * obj->radius / hull_percent;
	if (fraction > 1.0f) {
		fraction = 1.0f;
	}

	int spark_num;
	if ( sn == -1 ) {
		spark_num = Random::next(shipp->num_hits);
	} else {
		spark_num = sn;
	}

	// don't display sparks that have expired
	if ( timestamp_elapsed(shipp->sparks[spark_num].end_time) ) {
		return;
	}

	// get spark position
	if (shipp->sparks[spark_num].submodel_num != -1) {
		auto pmi = model_get_instance(shipp->model_instance_num);
		auto pm = model_get(pmi->model_num);
		model_instance_local_to_global_point(&outpnt, &shipp->sparks[spark_num].pos, pm, pmi, shipp->sparks[spark_num].submodel_num, &obj->orient, &obj->pos);
	} else {
		// rotate sparks correctly with current ship orient
		vm_vec_unrotate(&outpnt, &shipp->sparks[spark_num].pos, &obj->orient);
		vm_vec_add2(&outpnt,&obj->pos);
	}

    // phreak: Mantis 1676 - Re-enable warpout clipping.
	WarpEffect* warp_effect = nullptr;

	if ((shipp->is_arriving()) && (shipp->warpin_effect))
		warp_effect = shipp->warpin_effect;
	else if ((shipp->flags[Ship::Ship_Flags::Depart_warp]) && (shipp->warpout_effect))
		warp_effect = shipp->warpout_effect;

	if (warp_effect != nullptr && point_is_clipped_by_warp(&outpnt, warp_effect))
		return;

	if ( create_spark )	{

		particle::particle_emitter pe;
		particle_effect		pef;

		pe.pos = outpnt;				// Where the particles emit from

        if (shipp->is_arriving() || shipp->flags[Ship::Ship_Flags::Depart_warp]) {
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

		// r = outpnt - model_local_to_global_point(0)

		// w = model_local_to_global_dir(
		// model_local_to_global_dir(&out_dir, &in_dir, model_num, submodel_num, &objorient, &objpos);

		vec3d tmp_norm, tmp_vel;
		vm_vec_sub( &tmp_norm, &outpnt, &obj->pos );
		vm_vec_normalize_safe(&tmp_norm);

		tmp_vel = obj->phys_info.vel;
		if ( vm_vec_normalize_safe(&tmp_vel) > 1.0f )	{
			vm_vec_scale_add2(&tmp_norm,&tmp_vel, -2.0f);
			vm_vec_normalize_safe(&tmp_norm);
		}
				
		pe.normal = tmp_norm;			// What normal the particle emit around

		if (sn > -1)
			pef = sip->impact_spew;
		else
			pef = sip->damage_spew;

		pe.min_rad = pef.min_rad;
		pe.max_rad = pef.max_rad;
		pe.min_vel = pef.min_vel;				// How fast the slowest particle can move
		pe.max_vel = pef.max_vel;				// How fast the fastest particle can move

		// first time through - set up end time and make heavier initially
		if ( sn > -1 )	{
			// Sparks for first time at this spot
			if (sip->flags[Ship::Info_Flags::Fighter]) {
				if (hull_percent > 0.6f) {
					// sparks only once when hull > 60%
					float spark_duration = (float)pow(2.0f, -5.0f*(hull_percent-1.3f)) * (1.0f + 0.6f*(frand()-0.5f));	// +- 30%
					shipp->sparks[spark_num].end_time = timestamp( (int) (1000.0f * spark_duration) );
				} else {
					// spark never ends when hull < 60% (~277 hr)
					shipp->sparks[spark_num].end_time = timestamp( 100000000 );
				}
			}
			pe.num_low = pef.n_low;				// Lowest number of particles to create (hardware)
			pe.num_high = pef.n_high;
			pe.normal_variance = pef.variance;	//	How close they stick to that normal 0=good, 1=360 degree
			pe.min_life = pef.min_life;				// How long the particles live
			pe.max_life = pef.max_life;				// How long the particles live

			particle::emit( &pe, particle::PARTICLE_FIRE, 0 );
		} else {
			if (pef.n_high > 1) {
				pe.num_low = pef.n_low;
				pe.num_high = pef.n_high;
			} else {
				pe.num_low  = (int) (20.0f * spark_num_scale);
				pe.num_high = (int) (50.0f * spark_num_scale);
			}
			
			if (pef.variance > 0.0f) {
				pe.normal_variance = pef.variance;
			} else {
				pe.normal_variance = 0.2f * spark_width_scale;
			}

			if (pef.max_life > 0.0f) {
				pe.min_life = pef.min_life;
				pe.max_life = pef.max_life;
			} else {
				pe.min_life = 0.7f * spark_time_scale;
				pe.max_life = 1.5f * spark_time_scale;
			}
			
			particle::emit( &pe, particle::PARTICLE_SMOKE, 0 );
		}
	}

	// Select time to do next spark
	shipp->next_hit_spark = timestamp_rand(50,100);
}



//=====================================================================================
// STUFF FOR DOING LARGE SHIP EXPLOSIONS
//=====================================================================================

int	Bs_exp_fire_low = 1;
float	Bs_exp_fire_time_mult = 1.0f;

DCF_BOOL(bs_exp_fire_low, Bs_exp_fire_low)
DCF(bs_exp_fire_time_mult, "Sets multiplier time between fireball in big ship explosion")
{
	float value;

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Bs_exp_fire_time_mult : %f\n", Bs_exp_fire_time_mult);
		return;
	}

	dc_stuff_float(&value);

	CLAMP(value, 0.1f, 5.0f);
	Bs_exp_fire_time_mult = value;
	dc_printf("Bs_exp_fire_time_mult set to %f\n", Bs_exp_fire_time_mult);
}


#define DEBRIS_NONE			0
#define DEBRIS_DRAW			1
#define DEBRIS_FREE			2

typedef struct clip_ship {
	object*			parent_obj;
	float 			length_left;	// uncomsumed length
	matrix			orient;
	physics_info	phys_info;
	vec3d			local_pivot;								// world center of mass position of half ship
	vec3d			model_center_disp_to_orig_center;	// displacement from half ship center to original model center
	vec3d			clip_plane_norm;							// clip plane normal (local [0,0,1] or [0,0,-1])
	float				cur_clip_plane_pt;						// displacement from half ship clip plane to original model center
	float				explosion_vel;
	ubyte				draw_debris[MAX_DEBRIS_OBJECTS];
	int				next_fireball;
} clip_ship;

typedef struct split_ship {
	int used = 0; // 0 if not used, 1 if used
	clip_ship		front_ship;
	clip_ship		back_ship;
	int explosion_flash_timestamp = 0;
	int explosion_flash_started   = 0;
	std::array<sound_handle, NUM_SUB_EXPL_HANDLES> sound_handles;

	split_ship()
	{
		memset(&front_ship, 0, sizeof(front_ship));
		memset(&back_ship, 0, sizeof(back_ship));
		sound_handles.fill(sound_handle::invalid());
	}
} split_ship;


static SCP_vector<split_ship> Split_ships;

static int get_split_ship()
{
	int i;

	// check for an existing free slot
	int max_size = (int)Split_ships.size();
	for (i = 0; i < max_size; i++) {
		if (!Split_ships[i].used)
			return i;
	}

	// Construct default constructed object at the end
	Split_ships.emplace_back();

	return (int)(Split_ships.size() - 1);
}

static void maybe_fireball_wipe(clip_ship* half_ship, sound_handle* sound_handle);
static void split_ship_init( ship* shipp, split_ship* split_shipp )
{
	object* parent_ship_obj = &Objects[shipp->objnum];
	matrix* orient = &parent_ship_obj->orient;
	for (int ii=0; ii<NUM_SUB_EXPL_HANDLES; ++ii) {
		split_shipp->sound_handles[ii] = shipp->sub_expl_sound_handle[ii];
	}

	// play 3d sound for shockwave explosion
	snd_play_3d( gamesnd_get_game_sound(GameSounds::SHOCKWAVE_EXPLODE), &parent_ship_obj->pos, &View_position, 0.0f, NULL, 0, 1.0f, SND_PRIORITY_SINGLE_INSTANCE, NULL, 3.0f );

	// initialize both ships
	split_shipp->front_ship.parent_obj = parent_ship_obj;
	split_shipp->back_ship.parent_obj  = parent_ship_obj;
	split_shipp->explosion_flash_timestamp = timestamp((int)(0.00075f*parent_ship_obj->radius));
	split_shipp->explosion_flash_started = 0;
	split_shipp->front_ship.orient = *orient;
	split_shipp->back_ship.orient  = *orient;
	split_shipp->front_ship.next_fireball = timestamp_rand(0, 100);
	split_shipp->back_ship.next_fireball  = timestamp_rand(0, 100);

	split_shipp->front_ship.clip_plane_norm = vmd_z_vector;
	vm_vec_copy_scale(&split_shipp->back_ship.clip_plane_norm, &vmd_z_vector, -1.0f);

	// find the point at which the ship splits (relative to its pivot)
	polymodel* pm = model_get(Ship_info[shipp->ship_info_index].model_num);
	float init_clip_plane_dist;
	if (pm->num_split_plane > 0) {
		int index = Random::next(pm->num_split_plane);
		init_clip_plane_dist = pm->split_plane[index];
	} else {
		init_clip_plane_dist = 0.5f * (0.5f - frand())*pm->core_radius;
	}

	split_shipp->back_ship.cur_clip_plane_pt =  init_clip_plane_dist;
	split_shipp->front_ship.cur_clip_plane_pt = init_clip_plane_dist;

	float dist;
	dist = (split_shipp->front_ship.cur_clip_plane_pt+pm->maxs.xyz.z)/2.0f;
	vm_vec_copy_scale(&split_shipp->front_ship.local_pivot, &orient->vec.fvec, dist);
	vm_vec_make(&split_shipp->front_ship.model_center_disp_to_orig_center, 0.0f, 0.0f, -dist);
	dist = (split_shipp->back_ship.cur_clip_plane_pt +pm->mins.xyz.z)/2.0f;
	vm_vec_copy_scale(&split_shipp->back_ship.local_pivot, &orient->vec.fvec, dist);
	vm_vec_make(&split_shipp->back_ship.model_center_disp_to_orig_center, 0.0f, 0.0f, -dist);
	vm_vec_add2(&split_shipp->front_ship.local_pivot, &parent_ship_obj->pos );
	vm_vec_add2(&split_shipp->back_ship.local_pivot,  &parent_ship_obj->pos );
	
	// find which debris pieces are in the front and back split ships
	for (int i=0; i<pm->num_debris_objects; i++ )	{
		vec3d temp_pos = ZERO_VECTOR;
		vec3d tmp = ZERO_VECTOR;		
		vec3d tmp1 = pm->submodel[pm->debris_objects[i]].offset;
		// tmp is world position,  temp_pos is world_pivot,  tmp1 is offset from world_pivot (in ship local coord)
		// we don't need to use a model instance here because we're not working with any submodels
		model_local_to_global_point(&tmp, &tmp1, pm, -1, &vmd_identity_matrix, &temp_pos );
		if (tmp.xyz.z > init_clip_plane_dist) {
			split_shipp->front_ship.draw_debris[i] = DEBRIS_DRAW;
			split_shipp->back_ship.draw_debris[i]  = DEBRIS_NONE;
		} else {
			split_shipp->front_ship.draw_debris[i] = DEBRIS_NONE;
			split_shipp->back_ship.draw_debris[i]  = DEBRIS_DRAW;
		}
	}

	// set up physics 
	physics_init( &split_shipp->front_ship.phys_info );
	physics_init( &split_shipp->back_ship.phys_info );
	split_shipp->front_ship.phys_info.flags  |= (PF_ACCELERATES | PF_DEAD_DAMP);
	split_shipp->back_ship.phys_info.flags |= (PF_ACCELERATES | PF_DEAD_DAMP);
	split_shipp->front_ship.phys_info.side_slip_time_const = 10000.0f;
	split_shipp->back_ship.phys_info.side_slip_time_const =  10000.0f;
	split_shipp->front_ship.phys_info.rotdamp = 10000.0f;
	split_shipp->back_ship.phys_info.rotdamp =  10000.0f;

	// set up explosion vel and relative velocities (assuming mass depends on length)
	float front_length = pm->maxs.xyz.z - split_shipp->front_ship.cur_clip_plane_pt;
	float back_length  = split_shipp->back_ship.cur_clip_plane_pt - pm->mins.xyz.z;
	float ship_length = front_length + back_length;
	split_shipp->front_ship.length_left = front_length;
	split_shipp->back_ship.length_left  = back_length;

	float expl_length_scale = (ship_length - 200.0f) / 2000.0f;
	// s_r_f effects speed of "wipe" and rotvel
	float speed_reduction_factor = (1.0f + 0.001f*parent_ship_obj->radius);
	float explosion_time = (3.0f + expl_length_scale + (frand()-0.5f)) * speed_reduction_factor;
	float long_length = MAX(front_length, back_length);
	float expl_vel = long_length / explosion_time;
	split_shipp->front_ship.explosion_vel = expl_vel;
	split_shipp->back_ship.explosion_vel  = -expl_vel;

	float rel_vel = (0.6f + 0.2f*frand()) * expl_vel * speed_reduction_factor;
	float front_vel = rel_vel * back_length / ship_length;
	float back_vel = -rel_vel * front_length / ship_length;

	// set up rotational vel
	vec3d rotvel;
	vm_vec_rand_vec_quick(&rotvel);
	rotvel.xyz.z = 0.0f;
	vm_vec_normalize(&rotvel);
	vm_vec_scale(&rotvel, 0.15f / speed_reduction_factor);
	split_shipp->front_ship.phys_info.rotvel = rotvel;
	vm_vec_copy_scale(&split_shipp->back_ship.phys_info.rotvel, &rotvel, -(front_length*front_length)/(back_length*back_length));
	split_shipp->front_ship.phys_info.rotvel.xyz.z = parent_ship_obj->phys_info.rotvel.xyz.z;
	split_shipp->back_ship.phys_info.rotvel.xyz.z  = parent_ship_obj->phys_info.rotvel.xyz.z;


	// modify vel of each split ship based on rotvel of parent ship obj
	vec3d temp_rotvel = parent_ship_obj->phys_info.rotvel;
	temp_rotvel.xyz.z = 0.0f;
	vec3d vel_from_rotvel;
	vm_vec_cross(&vel_from_rotvel, &temp_rotvel, &split_shipp->front_ship.local_pivot);
	vm_vec_cross(&vel_from_rotvel, &temp_rotvel, &split_shipp->back_ship.local_pivot);

	// set up velocity and make initial fireballs and particles
	split_shipp->front_ship.phys_info.vel = parent_ship_obj->phys_info.vel;
	split_shipp->back_ship.phys_info.vel  = parent_ship_obj->phys_info.vel;
	maybe_fireball_wipe(&split_shipp->front_ship, split_shipp->sound_handles.data());
	maybe_fireball_wipe(&split_shipp->back_ship, split_shipp->sound_handles.data());
	vm_vec_scale_add2(&split_shipp->front_ship.phys_info.vel, &orient->vec.fvec, front_vel);
	vm_vec_scale_add2(&split_shipp->back_ship.phys_info.vel,  &orient->vec.fvec, back_vel);

	// HANDLE LIVE DEBRIS - blow off if not already gone
	shipfx_maybe_create_live_debris_at_ship_death( parent_ship_obj );
}

void shipfx_queue_render_ship_halves_and_debris(model_draw_list *scene, clip_ship* half_ship, ship *shipp)
{
	polymodel_instance *pmi = model_get_instance(shipp->model_instance_num);
	polymodel *pm = model_get(pmi->model_num);

	// get rotated clip plane normal and world coord of original ship center
	vec3d orig_ship_world_center, clip_plane_norm, model_clip_plane_pt, debris_clip_plane_pt;
	vm_vec_unrotate(&clip_plane_norm, &half_ship->clip_plane_norm, &half_ship->orient);
	vm_vec_unrotate(&orig_ship_world_center, &half_ship->model_center_disp_to_orig_center, &half_ship->orient);
	vm_vec_add2(&orig_ship_world_center, &half_ship->local_pivot);

	// get debris clip plane pt and draw debris
	vm_vec_unrotate(&debris_clip_plane_pt, &half_ship->model_center_disp_to_orig_center, &half_ship->orient);
	vm_vec_add2(&debris_clip_plane_pt, &half_ship->local_pivot);

	// set up render flags
	uint render_flags = MR_NORMAL;

	if ( Rendering_to_shadow_map ) {
		render_flags |= MR_NO_TEXTURING | MR_NO_LIGHTING;
	}

	if (shipp->flags[Ship::Ship_Flags::Glowmaps_disabled]) {
		render_flags |= MR_NO_GLOWMAPS;
	}

	for (int i = 0; i < pm->num_debris_objects; i++ )	{
		// draw DEBRIS_FREE in test only
		if (half_ship->draw_debris[i] == DEBRIS_DRAW) {
			vec3d temp_pos = orig_ship_world_center;
			vec3d tmp = ZERO_VECTOR;
			vec3d tmp1 = pm->submodel[pm->debris_objects[i]].offset;

			// determine if explosion front has past debris piece
			// 67 ~ dist expl moves in 2 frames -- maybe fraction works better
			bool is_live_debris = pm->submodel[pm->debris_objects[i]].flags[Model::Submodel_flags::Is_live_debris];
			int create_debris = 0;
			// front ship
			if (half_ship->explosion_vel > 0) {
				if (half_ship->cur_clip_plane_pt > tmp1.xyz.z + pm->submodel[pm->debris_objects[i]].max.xyz.z - 0.1f*half_ship->explosion_vel) {
					create_debris = 1;
				}
				// back ship
			} else {
				if (half_ship->cur_clip_plane_pt < tmp1.xyz.z + pm->submodel[pm->debris_objects[i]].min.xyz.z - 0.1f*half_ship->explosion_vel) {
					create_debris = 1;
				}
			}

			// Draw debris, but not live debris
			if ( !is_live_debris ) {
				// we don't need to use a model instance here because we're not working with any submodels
				model_local_to_global_point(&tmp, &tmp1, pm, -1, &half_ship->orient, &temp_pos);
				model_render_params render_info;

				render_info.set_clip_plane(debris_clip_plane_pt, clip_plane_norm);
				render_info.set_replacement_textures(shipp->ship_replacement_textures);
				render_info.set_flags(render_flags);

				submodel_render_queue(&render_info, scene, pm, pmi, pm->debris_objects[i], &half_ship->orient, &tmp);
			}

			// make free piece of debris
			if ( create_debris ) {
				half_ship->draw_debris[i] = DEBRIS_FREE;		// mark debris to not render with model
				vec3d center_to_debris, debris_vel, radial_vel;
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
					debris_obj = debris_create(half_ship->parent_obj, pm->id, pm->debris_objects[i], &tmp, &half_ship->local_pivot, 1, 1.0f);
					// AL: make sure debris_obj isn't NULL!
					if ( debris_obj ) {
						vm_vec_scale(&debris_obj->phys_info.rotvel, 4.0f);
						debris_obj->orient = half_ship->orient;

						vm_vec_sub(&center_to_debris, &tmp, &half_ship->local_pivot);
						vm_vec_cross(&debris_vel, &center_to_debris, &half_ship->phys_info.rotvel);
						vm_vec_add2(&debris_vel, &half_ship->phys_info.vel);
						vm_vec_copy_normalize(&radial_vel, &center_to_debris);
						float radial_mag = 10.0f + 30.0f*frand();
						vm_vec_scale_add2(&debris_vel, &radial_vel, radial_mag);
						debris_obj->phys_info.vel = debris_vel;
						shipfx_debris_limit_speed(&Debris[debris_obj->instance], shipp);
					}
				}
			}
		}
	}

	// get model clip plane pt and draw model
	vec3d temp;
	vm_vec_make(&temp, 0.0f, 0.0f, half_ship->cur_clip_plane_pt);
	vm_vec_unrotate(&model_clip_plane_pt, &temp, &half_ship->orient);
	vm_vec_add2(&model_clip_plane_pt, &orig_ship_world_center);

	model_render_params render_info;

	render_info.set_flags(render_flags);
	render_info.set_clip_plane(model_clip_plane_pt, clip_plane_norm);
	render_info.set_replacement_textures(shipp->ship_replacement_textures);
	render_info.set_object_number(shipp->objnum);

	model_render_queue(&render_info, scene, pm->id, &half_ship->orient, &orig_ship_world_center);
}

void shipfx_large_blowup_level_init()
{
	Split_ships.clear();

	if(Ship_cannon_bitmap != -1){
		bm_release(Ship_cannon_bitmap);
		Ship_cannon_bitmap = bm_load(SHIP_CANNON_BITMAP);
	}
}

void shipfx_large_blowup_init(ship *shipp)
{
	int i;

	i = get_split_ship();

	Split_ships[i].used = 1;
	shipp->large_ship_blowup_index = i;

	split_ship_init(shipp, &Split_ships[i] );
}

void shipfx_debris_limit_speed(debris *db, ship *shipp)
{
	if(db == NULL || shipp == NULL)
		return;

	object *ship_objp = &Objects[shipp->objnum];
	physics_info *pi = &Objects[db->objnum].phys_info;
	ship_info *sip = &Ship_info[shipp->ship_info_index];

	float curspeed = vm_vec_mag(&pi->vel);
	if(sip->debris_min_speed >= 0.0f && sip->debris_max_speed >= 0.0f)
	{
		float debris_speed = (( sip->debris_max_speed - sip->debris_min_speed ) * frand()) + sip->debris_min_speed;
		if(fabs(curspeed) >= 0.001f)
		{
			float scale = debris_speed / curspeed;
			vm_vec_scale(&pi->vel, scale);
		}
		else
		{
			vm_vec_copy_scale(&pi->vel, &ship_objp->orient.vec.fvec, debris_speed);
		}
	}
	else if(sip->debris_min_speed >= 0.0f)
	{
		if(curspeed < sip->debris_min_speed)
		{
			if(fabs(curspeed) >= 0.001f)
			{
				float scale = sip->debris_min_speed / curspeed;
				vm_vec_scale(&pi->vel, scale);
			}
			else
			{
				vm_vec_copy_scale(&pi->vel, &ship_objp->orient.vec.fvec, sip->debris_min_speed);
			}
		}
	}
	else if(sip->debris_max_speed >= 0.0f)
	{
		if(curspeed > sip->debris_max_speed)
		{
			if(fabs(curspeed) >= 0.001f)
			{
				float scale = sip->debris_max_speed / curspeed;
				vm_vec_scale(&pi->vel, scale);
			}
			else
			{
				vm_vec_copy_scale(&pi->vel, &ship_objp->orient.vec.fvec, sip->debris_max_speed);
			}
		}
	}

	//WMC - Rotational velocity user cap
	float currotvel = vm_vec_mag(&pi->rotvel);
	if(sip->debris_min_rotspeed >= 0.0f && sip->debris_max_rotspeed >= 0.0f)
	{
		float debris_rotspeed = (( sip->debris_max_rotspeed - sip->debris_min_rotspeed ) * frand()) + sip->debris_min_rotspeed;
		if(fabs(currotvel) >= 0.001f)
		{
			float scale = debris_rotspeed / currotvel;
			vm_vec_scale(&pi->rotvel, scale);
		}
		else
		{
			vm_vec_copy_scale(&pi->rotvel, &ship_objp->orient.vec.uvec, debris_rotspeed);
		}
	}
	else if(sip->debris_min_rotspeed >= 0.0f)
	{
		if(curspeed < sip->debris_min_rotspeed)
		{
			if(fabs(currotvel) >= 0.001f)
			{
				float scale = sip->debris_min_rotspeed / currotvel;
				vm_vec_scale(&pi->rotvel, scale);
			}
			else
			{
				vm_vec_copy_scale(&pi->rotvel, &ship_objp->orient.vec.uvec, sip->debris_min_rotspeed);
			}
		}
	}
	else if(sip->debris_max_rotspeed >= 0.0f)
	{
		curspeed = vm_vec_mag(&pi->rotvel);
		if(curspeed > sip->debris_max_rotspeed)
		{
			if(fabs(currotvel) >= 0.001f)
			{
				float scale = sip->debris_max_rotspeed / currotvel;
				vm_vec_scale(&pi->rotvel, scale);
			}
			else
			{
				vm_vec_copy_scale(&pi->rotvel, &ship_objp->orient.vec.uvec, sip->debris_max_rotspeed);
			}
		}
	}

	int ship_type = sip->class_type;
	if(ship_type > -1)
	{
		if(vm_vec_mag(&pi->vel) > Ship_types[ship_type].debris_max_speed) {
			float scale = Ship_types[ship_type].debris_max_speed / vm_vec_mag(&pi->vel);
			vm_vec_scale(&pi->vel, scale);
		}
	}

	Assert(is_valid_vec(&pi->vel));
	Assert(is_valid_vec(&pi->rotvel));
}

// ----------------------------------------------------------------------------
// uses list of model z values with constant increment to find the radius of the 
// cross section at the current model z value
static float get_model_cross_section_at_z(float z, polymodel* pm)
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
		return MAX(pm->xc[ceil_index].radius, pm->xc[floor_index].radius);
	}
}

/**
 * Returns how long sound has been playing
 */
static int get_sound_time_played(sound_load_id snd_id, sound_handle handle)
{
	if (!handle.isValid() || !snd_id.isValid()) {
		return 100000;
	}

	int time_left = snd_time_remaining(handle);
	int duration = snd_get_duration(snd_id);
	
	return (duration - time_left);
}

/**
 * Sound manager for big ship sub explosions sounds.
 *
 * Forces playing of sub-explosion sounds.  Keeps track of active sounds, plays them for >= 750 ms
 * when sound has played >= 750, sound is stopped and new instance is started
 */
void do_sub_expl_sound(float radius, vec3d* sound_pos, sound_handle* handle_array)
{
	// multiplier for range (near and far distances) to apply attenuation
	float sound_range = 1.0f + 0.0043f*radius;

	int handle_index = Random::next(NUM_SUB_EXPL_HANDLES);

	auto sound_index = GameSounds::SHIP_EXPLODE_1;
	auto handle      = handle_array[handle_index];

	if (!handle.isValid()) {
		// if no handle, get one
		handle_array[handle_index] = snd_play_3d(gamesnd_get_game_sound(sound_index), sound_pos, &View_position, 0.0f,
		                                         nullptr, 0, 0.6f, SND_PRIORITY_MUST_PLAY, nullptr, sound_range);
	} else if (!snd_is_playing(handle)) {
		// if sound not playing and old, get new one
		// I don't think will happen with SND_PRIORITY_MUST_PLAY
		if (get_sound_time_played(snd_get_sound_id(handle), handle) > 400) {
			snd_stop(handle_array[handle_index]);
			handle_array[handle_index] =
			    snd_play_3d(gamesnd_get_game_sound(sound_index), sound_pos, &View_position, 0.0f, nullptr, 0, 0.6f,
			                SND_PRIORITY_MUST_PLAY, nullptr, sound_range);
		}
	} else if (get_sound_time_played(snd_get_sound_id(handle), handle) > 750) {
		handle_array[handle_index] = snd_play_3d(gamesnd_get_game_sound(sound_index), sound_pos, &View_position, 0.0f,
		                                         nullptr, 0, 0.6f, SND_PRIORITY_MUST_PLAY, nullptr, sound_range);
	}
}

/**
 * Maybe create a fireball along model clip plane also maybe plays explosion sound
 */
static void maybe_fireball_wipe(clip_ship* half_ship, sound_handle* handle_array)
{
	// maybe make fireball to cover wipe.
	if ( timestamp_elapsed(half_ship->next_fireball) ) {
		if ( half_ship->length_left > 0.2f*fl_abs(half_ship->explosion_vel) )	{
			ship_info *sip = &Ship_info[Ships[half_ship->parent_obj->instance].ship_info_index];
			
			polymodel* pm = model_get(sip->model_num);

			vec3d model_clip_plane_pt, orig_ship_world_center, temp;

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
				// changed from 0.4 & 0.6 to 0.6 & 0.9 as later 1.5 multiplier was removed
				rad = half_ship->parent_obj->radius * frand_range(0.6f, 0.9f);
			} else {
				// make fireball radius (1.5 +/- .1) * model_cross_section value
				// changed from 1.4 & 1.6 to 2.1 & 2.4 as later 1.5 multiplier was removed
				rad *= frand_range(2.1f, 2.4f);
			}

			rad = MIN(rad, half_ship->parent_obj->radius);

			//defaults to 1.0 now that multiplier was applied to the static values above
			rad *= sip->prop_exp_rad_mult;

			int fireball_type = fireball_ship_explosion_type(sip);
			if(fireball_type < 0) {
				fireball_type = FIREBALL_EXPLOSION_LARGE1 + Random::next(FIREBALL_NUM_LARGE_EXPLOSIONS);
			}
			int low_res_fireballs = Bs_exp_fire_low;
			fireball_create(&model_clip_plane_pt, fireball_type, FIREBALL_LARGE_EXPLOSION, OBJ_INDEX(half_ship->parent_obj), rad, false, &half_ship->parent_obj->phys_info.vel, 0.0f, -1, nullptr, low_res_fireballs);

			// start the next fireball up (3-4 per frame) + 30%
			int time_low, time_high;
			time_low = int(650 * Bs_exp_fire_time_mult);
			time_high = int(900 * Bs_exp_fire_time_mult);
			half_ship->next_fireball = timestamp_rand(time_low, time_high);

			// do sound
			do_sub_expl_sound(half_ship->parent_obj->radius, &model_clip_plane_pt, handle_array);

			// do particles
			particle::particle_emitter	pe;
			particle_effect		pef = sip->split_particles;

			pe.num_low = pef.n_low;					// Lowest number of particles to create
			pe.num_high = pef.n_high;				// Highest number of particles to create
			pe.pos = model_clip_plane_pt;	// Where the particles emit from
			pe.vel = half_ship->phys_info.vel;		// Initial velocity of all the particles

			float range = 1.0f + 0.002f*half_ship->parent_obj->radius;

			if (pef.max_life > 0.0f) {
				pe.min_life = pef.min_life;
				pe.max_life = pef.max_life;
			} else {
				pe.min_life = 0.5f*range;				// How long the particles live
				pe.max_life = 6.0f*range;				// How long the particles live
			}

			pe.normal = vmd_x_vector;		// What normal the particle emit around
			pe.normal_variance = pef.variance;		//	How close they stick to that normal 0=on normal, 1=180, 2=360 degree

			if (pef.max_vel > 0.0f) {
				pe.min_vel = pef.min_vel;
				pe.max_vel = pef.max_vel;
			} else {
				pe.min_vel = 0.0f;									// How fast the slowest particle can move
				pe.max_vel = half_ship->explosion_vel;				// How fast the fastest particle can move
			}
			float scale = half_ship->parent_obj->radius * 0.01f;
			if (pef.max_rad > 0.0f) {
				pe.min_rad = pef.min_rad;
				pe.max_rad = pef.max_rad;
			} else {
				pe.min_rad = 0.5f*scale;				// Min radius
				pe.max_rad = 1.5f*scale;				// Max radius
			}

			if (pe.num_high > 0) {
				particle::emit( &pe, particle::PARTICLE_SMOKE2, 0, range );
			}

			if (sip->generic_debris_model_num >= 0) {
				// spawn a bunch of debris pieces, first determine the cross sectional average position to be the force explosion center
				vec3d local_xc_rand, local_xc_avg, xc_rand, xc_avg;
				submodel_get_cross_sectional_avg_pos(sip->model_num, -1, half_ship->cur_clip_plane_pt, &local_xc_avg);
				vm_vec_unrotate(&xc_avg, &local_xc_avg, &half_ship->orient);
				vm_vec_add2(&xc_avg, &orig_ship_world_center);
				float num_debris = sip->generic_debris_spew_num * (Detail.num_small_debris / 4.0f);
				for (int i = 0; i < num_debris; i++) {
					// then get random positions on the cross section and spawn the pieces there
					submodel_get_cross_sectional_random_pos(sip->model_num, -1, half_ship->cur_clip_plane_pt, &local_xc_rand);
					vm_vec_unrotate(&xc_rand, &local_xc_rand, &half_ship->orient);
					vm_vec_add2(&xc_rand, &orig_ship_world_center);
					debris_create(half_ship->parent_obj, sip->generic_debris_model_num, -1, &xc_rand, &xc_avg, 0, frand() * half_ship->parent_obj->radius / 100);
				}
			}

		} else {
			// time out forever
			half_ship->next_fireball = timestamp(-1);
		}
	}
}

void big_explosion_flash(float);

/**
 * Returns 1 when explosion is done
 */
int shipfx_large_blowup_do_frame(ship *shipp, float frametime)
{
	Assert( shipp->large_ship_blowup_index > -1 );
	Assert( shipp->large_ship_blowup_index < (int)Split_ships.size() );

	split_ship *the_split_ship = &Split_ships[shipp->large_ship_blowup_index];
	Assert( the_split_ship->used );		// Get John

	// Do fireballs, particles, shockwave here
	// Note parent ship is still valid, vel and pos updated in obj_move_all

	if ( timestamp_elapsed(the_split_ship->explosion_flash_timestamp) ) {
		if ( !the_split_ship->explosion_flash_started ) {
			object* objp = &Objects[shipp->objnum];
			if (objp->flags[Object::Object_Flags::Was_rendered]) {
				float excess_dist = (Player_obj == nullptr) ? 0.0f : vm_vec_dist(&Player_obj->pos, &objp->pos) - 2.0f*objp->radius - Player_obj->radius;
				float intensity = 1.0f - 0.1f*excess_dist / objp->radius;

				if (intensity > 1) {
					intensity = 1.0f;
				}

				if (intensity > 0.1f && Ship_info[shipp->ship_info_index].flags[Ship::Info_Flags::Flash]) {
					big_explosion_flash(intensity);
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

	float length_left = MAX( the_split_ship->front_ship.length_left, the_split_ship->back_ship.length_left );

	if ( length_left < 0 )	{
		the_split_ship->used = 0;
		return 1;
	}

	maybe_fireball_wipe(&the_split_ship->front_ship, the_split_ship->sound_handles.data());
	maybe_fireball_wipe(&the_split_ship->back_ship, the_split_ship->sound_handles.data());
	return 0;
}

void shipfx_large_blowup_queue_render(model_draw_list *scene, ship* shipp)
{
	Assert( shipp->large_ship_blowup_index > -1 );
	Assert( shipp->large_ship_blowup_index < (int)Split_ships.size() );

	split_ship *the_split_ship = &Split_ships[shipp->large_ship_blowup_index];
	Assert( the_split_ship->used );		// Get John

	if (the_split_ship->front_ship.length_left > 0) {
		shipfx_queue_render_ship_halves_and_debris(scene, &the_split_ship->front_ship,shipp);
	}

	if (the_split_ship->back_ship.length_left > 0) {
		shipfx_queue_render_ship_halves_and_debris(scene, &the_split_ship->back_ship,shipp);
	}
}

// ================== DO THE ELECTRIC ARCING STUFF =====================
// Creates any new ones, moves old ones.

const float MAX_ARC_LENGTH_PERCENTAGE = 0.25f;

const float MAX_EMP_ARC_TIMESTAMP = 150.0f;

void shipfx_do_lightning_arcs_frame( ship *shipp )
{
	object *obj = &Objects[shipp->objnum];
	ship_info* sip = &Ship_info[shipp->ship_info_index];
	int model_num = sip->model_num;

	// first do any passive ship arcs, separate from damage or emp arcs
	for (int passive_arc_info_idx = 0; passive_arc_info_idx < (int)sip->ship_passive_arcs.size(); passive_arc_info_idx++) {
		if (!shipp->flags[Ship::Ship_Flags::No_passive_lightning] && timestamp_elapsed(shipp->passive_arc_next_times[passive_arc_info_idx])) {

			ship_passive_arc_info* arc_info = &sip->ship_passive_arcs[passive_arc_info_idx];
			polymodel* pm = model_get(model_num);

			// find the specified submodels involved, if necessary
			if (arc_info->submodels.first < 0 || arc_info->submodels.second < 0) {
				for (int i = 0; i < pm->n_models; i++) {
					if (!stricmp(pm->submodel[i].name, arc_info->submodel_strings.first.c_str()))
						arc_info->submodels.first = i;
					if (!stricmp(pm->submodel[i].name, arc_info->submodel_strings.second.c_str()))
						arc_info->submodels.second = i;
				}
			}
			int submodel_1 = arc_info->submodels.first;
			int submodel_2 = arc_info->submodels.second;

			// see if these submodels are also subsystems, and dont draw if its destroyed
			bool skip = false;
			for (ship_subsys* pss = GET_FIRST(&shipp->subsys_list); pss != END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss)) {
				if (pss->system_info->subobj_num == submodel_1 && pss->max_hits > 0 && pss->current_hits <= 0) {
					skip = true;
					break;
				} else if (pss->system_info->subobj_num == submodel_2 && pss->max_hits > 0 && pss->current_hits <= 0) {
					skip = true;
					break;
				}
			}
			if (skip) break;

			if (submodel_1 >= 0 && submodel_2 >= 0) {
				// spawn the arc in the first unused slot
				for (int j = 0; j < MAX_SHIP_ARCS; j++) {
					if (!timestamp_valid(shipp->arc_timestamp[j])) {
						shipp->arc_timestamp[j] = timestamp((int)(arc_info->duration * 1000));

						vec3d v1, v2, offset;
						// subtract away the submodel's offset, since these positions were in frame of ref of the whole ship
						model_find_submodel_offset(&offset, pm, submodel_1);
						v1 = arc_info->pos.first - offset;
						model_find_submodel_offset(&offset, pm, submodel_2);
						v2 = arc_info->pos.second - offset;

						model_instance_local_to_global_point(&v1, &v1, shipp->model_instance_num, submodel_1, &vmd_identity_matrix, &vmd_zero_vector);
						shipp->arc_pts[j][0] = v1;
						model_instance_local_to_global_point(&v2, &v2, shipp->model_instance_num, submodel_2, &vmd_identity_matrix, &vmd_zero_vector);
						shipp->arc_pts[j][1] = v2;

						shipp->arc_type[j] = MARC_TYPE_SHIP;

						shipp->passive_arc_next_times[passive_arc_info_idx] = timestamp((int)(arc_info->frequency * 1000));
						break;
					}
				}
			}
		}
	}

	// now handle damage/emp arcs
	int should_arc = 1;
	int disrupted_arc=0;

	float damage = get_hull_pct(obj);	

	if (damage < 0) {
		damage = 0.0f;
	}

	// don't draw an arc based on damage
	if ( damage > 0.30f )	{
		// Don't do spark.
		should_arc = 0;
	}

	// SUSHI: If the lightning type is NONE, we can skip this
	if (Ship_info[shipp->ship_info_index].damage_lightning_type == SLT_NONE)
		should_arc = 0;

	// we should draw an arc
	if( shipp->emp_intensity > 0.0f){
		should_arc = 1;
	}

	if ((ship_subsys_disrupted(shipp,SUBSYSTEM_ENGINE)) ||
		(ship_subsys_disrupted(shipp,SUBSYSTEM_WEAPONS)) || 
		(ship_subsys_disrupted(shipp,SUBSYSTEM_SENSORS)) )
	{
		disrupted_arc=1;
		should_arc=1;
	}

	// Kill off old sparks
	for(int &arc_stamp : shipp->arc_timestamp){
		if(timestamp_valid(arc_stamp) && timestamp_elapsed(arc_stamp)){
			arc_stamp = timestamp(-1);
		}
	}

	// if we shouldn't draw an arc, return
	if(!should_arc){
		return;
	}

	if (!timestamp_valid(shipp->arc_next_time))	{
		// start the next fireball up in the next 10 seconds or so... 
		int freq;
		
		// if the emp effect is active or its disrupted
		if((shipp->emp_intensity > 0.0f) || (disrupted_arc)){
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

		int n, n_arcs = Random::next(1, 3);

		vec3d v1, v2, v3, v4;
		submodel_get_two_random_points_better(model_num, -1, &v1, &v2);
		submodel_get_two_random_points_better(model_num, -1, &v3, &v4);

		// For large ships, cap the length to be 25% of max radius
		if ( obj->radius > 200.0f )	{
			float max_dist = obj->radius * MAX_ARC_LENGTH_PERCENTAGE;
			
			vec3d tmp;
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

		float factor = 1.0f + 0.0025f*obj->radius;
		int a = (int) (factor*100.0f);
		int b = (int) (factor*1000.0f);
		int lifetime = Random::next(a, b);

		// Create the arc effects
		for (int i=0; i<MAX_SHIP_ARCS; i++ )	{
			if ( !timestamp_valid( shipp->arc_timestamp[i] ) )	{
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
				if((shipp->emp_intensity > 0.0f) || (disrupted_arc)){
					shipp->arc_type[i] = MARC_TYPE_EMP;
				} else {
					shipp->arc_type[i] = MARC_TYPE_DAMAGED;
				}
					
				n++;
				if ( n == n_arcs )
					break;	// Don't need to create anymore
			}
	
			// rotate v2 out of local coordinates into world.
			// Use v2 since it is used in every bolt.  See above switch().
			vec3d snd_pos;
			vm_vec_unrotate(&snd_pos, &v2, &obj->orient);
			vm_vec_add2(&snd_pos, &obj->pos );

			//Play a sound effect
			if ( lifetime > 750 )	{
				// 1.00 second effect
				snd_play_3d( gamesnd_get_game_sound(GameSounds::DEBRIS_ARC_05), &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  500 )	{
				// 0.75 second effect
				snd_play_3d( gamesnd_get_game_sound(GameSounds::DEBRIS_ARC_04), &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  250 )	{
				// 0.50 second effect
				snd_play_3d( gamesnd_get_game_sound(GameSounds::DEBRIS_ARC_03), &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  100 )	{
				// 0.25 second effect
				snd_play_3d( gamesnd_get_game_sound(GameSounds::DEBRIS_ARC_02), &snd_pos, &View_position, obj->radius );
			} else {
				// 0.10 second effect
				snd_play_3d( gamesnd_get_game_sound(GameSounds::DEBRIS_ARC_01), &snd_pos, &View_position, obj->radius );
			}
		}
	}

	// maybe move arc points around
	for (int i=0; i<MAX_SHIP_ARCS; i++ )	{
		if ( timestamp_valid( shipp->arc_timestamp[i] ) )	{
			if ( !timestamp_elapsed( shipp->arc_timestamp[i] ) )	{							
				// Maybe move a vertex....  20% of the time maybe?
				int mr = Random::next();
				if ( mr < Random::MAX_VALUE/5 )	{
					vec3d v1, v2;
					submodel_get_two_random_points_better(model_num, -1, &v1, &v2);

					vec3d static_one;

					if ( mr % 2 )	{
						static_one = shipp->arc_pts[i][0];
					} else {
						static_one = shipp->arc_pts[i][1];
					}

					// For large ships, cap the length to be 25% of max radius
					if ( obj->radius > 200.0f )	{
						float max_dist = obj->radius * MAX_ARC_LENGTH_PERCENTAGE;
						
						vec3d tmp;
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
void shipfx_do_lightning_frame( ship * /*shipp*/ )
{
	/*
	ship_info *sip;
	object *objp;
	int stamp, count;
	vec3d v1, v2, n1, n2, temp, temp2;
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
	if(!(The_mission.flags[Mission::Mission_Flags::Fullneb])){
		shipp->lightning_stamp = -1;
		return;
	}

	// if this not a cruiser or big ship
	if(!((sip->flags[Ship::Info_Flags::Cruiser]) || (sip->is_big_ship()) || (sip->is_huge_ship()))){
		shipp->lightning_stamp = -1;
		return;
	}

	// determine stamp and count values
	if(sip->flags[Ship::Info_Flags::Cruiser]){
		stamp = (int)((float)(Nebl_cruiser_min + ((Nebl_cruiser_max - Nebl_cruiser_min) * Nebl_intensity)) * frand_range(0.8f, 1.1f));
		count = l_cruiser_count;
	}
	else {
		if(sip->is_huge_ship()){
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

/**
 * Do all shockwaves for a ship blowing up
 */
void shipfx_do_shockwave_stuff(ship *shipp, shockwave_create_info *sci)
{
	ship_info *sip;
	object *objp;
	polymodel *pm;
	vec3d temp, dir, shockwave_pos;
	vec3d head = vmd_zero_vector;
	vec3d tail = vmd_zero_vector;	
	float step, cur;
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

	if(sip->shockwave_count <= 0){
		return;
	}

	// get vectors at the head and tail of the object, dead center		
	pm = model_get(sip->model_num);
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
	step = 1.0f / ((float)sip->shockwave_count + 1.0f);
	cur = step;
	for(idx=0; idx<sip->shockwave_count; idx++){
		// get the shockwave position		
		temp = dir;
		vm_vec_scale(&temp, cur);
		vm_vec_add(&shockwave_pos, &tail, &temp);

		// if knossos device, make shockwave in center
		if (Ship_info[shipp->ship_info_index].flags[Ship::Info_Flags::Knossos_device]) {
			shockwave_pos = Objects[shipp->objnum].pos;
		}

		// create the shockwave
		shockwave_create_info sci2 = *sci;
		sci2.blast = (sci->blast / (float)sip->shockwave_count) * frand_range(0.75f, 1.25f);
		sci2.damage = (sci->damage / (float)sip->shockwave_count) * frand_range(0.75f, 1.25f);
		sci2.speed = sci->speed * frand_range(0.75f, 1.25f);
		sci2.rot_angles.p = frand_range(0.0f, 1.99f*PI);
		sci2.rot_angles.b = frand_range(0.0f, 1.99f*PI);
		sci2.rot_angles.h = frand_range(0.0f, 1.99f*PI);

		shockwave_create(shipp->objnum, &shockwave_pos, &sci2, SW_SHIP_DEATH, (int)frand_range(0.0f, 350.0f));
		
		// next shockwave
		cur += step;
	}
}

extern int model_should_render_engine_glow(int objnum, int bank_obj);
int Wash_on = 1;
DCF_BOOL(engine_wash, Wash_on)
#define ENGINE_WASH_CHECK_INTERVAL		250	// (4x sec)

/**
 * Do engine wash effect for ship
 *
 * Assumes length of engine wash is greater than radius of engine wash hemisphere
 */
void engine_wash_ship_process(ship *shipp)
{
	int idx, j;		
	object *objp, *max_ship_intensity_objp;
	int started_with_no_wash = shipp->wash_intensity <= 0 ? 1 : 0;

	if (!Wash_on) {
		return;
	}

	Assert(shipp != NULL);
	Assert(shipp->objnum >= 0);
	objp = &Objects[shipp->objnum];
	ship_obj *so;

	float dist_sqr, inset_depth, dot_to_ship, max_ship_intensity;
	float max_wash_dist, half_angle, radius_mult;

	// if this is not a fighter or bomber, we don't care
	if ((objp->type != OBJ_SHIP) || !(Ship_info[shipp->ship_info_index].is_fighter_bomber()) ) {
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
		if (so->objnum < 0) {
			continue;
		}

		object *wash_objp = &Objects[so->objnum];

		if (!wash_objp || (wash_objp->instance < 0) || (wash_objp->type != OBJ_SHIP)) {
			continue;
		}

		ship *wash_shipp = &Ships[wash_objp->instance];
		ship_info *wash_sip = &Ship_info[wash_shipp->ship_info_index];

		// don't do small ships
        if (wash_sip->is_small_ship()) {
            continue;
        }

		auto pm = model_get(wash_sip->model_num);
		auto pmi = model_get_instance(wash_shipp->model_instance_num);
		float ship_intensity = 0;

		// if engines disabled, no engine wash
		if (ship_get_subsystem_strength(wash_shipp, SUBSYSTEM_ENGINE) < 0.01) {
			continue;
		}

		float	speed_scale;
		if (wash_objp->phys_info.speed > 20.0f)
			speed_scale = 1.0f;
		else
			speed_scale = wash_objp->phys_info.speed/20.0f;

		for (idx = 0; idx < pm->n_thrusters; idx++) {
			thruster_bank *bank = &pm->thrusters[idx];
			vec3d submodel_static_offset; // The associated submodel's static offset in the ship's frame of reference
			bool submodel_rotation = false;

			// make sure this engine is functional before we try to process a wash from it
			if ( !model_should_render_engine_glow(OBJ_INDEX(wash_objp), bank->obj_num) ) {
				continue;
			}

			// check if thruster bank has engine wash
			if (bank->wash_info_pointer == NULL) {
				// if huge, give default engine wash
				if ((wash_sip->is_huge_ship()) && !Engine_wash_info.empty()) {
					bank->wash_info_pointer = &Engine_wash_info[0];
					nprintf(("wash", "Adding default engine wash to ship %s", wash_sip->name));
				} else {
					continue;
				}
			}

			engine_wash_info *ewp = bank->wash_info_pointer;
			half_angle = ewp->angle;
			radius_mult = ewp->radius_mult;
			float wash_intensity;
			if (Use_engine_wash_intensity) {
				wash_intensity = ewp->intensity;
			} else {
				wash_intensity = 1.0f;
			}
			


			// If bank is attached to a submodel, prepare to account for rotations
			//
			// TODO: This won't work in the ship lab, because the lab code doesn't
			// set the the necessary submodel instance info needed here. The second
			// condition is thus a hack to disable the feature while in the lab, and
			// can be removed if the lab is re-structured accordingly. -zookeeper
			if ( bank->submodel_num >= 0 && pm->submodel[bank->submodel_num].flags[Model::Submodel_flags::Can_move] && (gameseq_get_state_idx(GS_STATE_LAB) == -1) ) {
				model_find_submodel_offset(&submodel_static_offset, pm, bank->submodel_num);

				submodel_rotation = true;
			}

			for (j=0; j<bank->num_points; j++) {
				vec3d world_thruster_pos, world_thruster_norm, apex, thruster_to_ship, apex_to_ship, temp;
				vec3d loc_pos = bank->points[j].pnt;
				vec3d loc_norm = bank->points[j].norm;

				if ( submodel_rotation ) {
					vm_vec_sub(&loc_pos, &bank->points[j].pnt, &submodel_static_offset);

					// Gets the final offset and normal in the ship's frame of reference
					temp = loc_pos;
					model_instance_local_to_global_point_dir(&loc_pos, &loc_norm, &temp, &loc_norm, pm, pmi, bank->submodel_num);
				}

				// get world pos of thruster
				vm_vec_unrotate(&world_thruster_pos, &loc_pos, &wash_objp->orient);
				vm_vec_add2(&world_thruster_pos, &wash_objp->pos);
				
				// get world norm of thruster;
				vm_vec_unrotate(&world_thruster_norm, &loc_norm, &wash_objp->orient);

				// get vector from thruster to ship
				vm_vec_sub(&thruster_to_ship, &objp->pos, &world_thruster_pos);

				// check if on back side of thruster
				dot_to_ship = vm_vec_dot(&thruster_to_ship, &world_thruster_norm);
				if (dot_to_ship > 0) {

					// get max wash distance
					max_wash_dist = MAX(ewp->length, bank->points[j].radius * ewp->radius_mult);

					// check if within dist range
					dist_sqr = vm_vec_mag_squared(&thruster_to_ship);
					if (dist_sqr < max_wash_dist*max_wash_dist) {

						// check if inside the sphere
						if ( dist_sqr < ((radius_mult * radius_mult) * (bank->points[j].radius * bank->points[j].radius)) ) {
							vm_vec_cross(&temp, &world_thruster_norm, &thruster_to_ship);
							vm_vec_scale_add2(&shipp->wash_rot_axis, &temp, dot_to_ship / dist_sqr);
							ship_intensity += (1.0f - dist_sqr / (max_wash_dist*max_wash_dist)) * wash_intensity;
							if (!do_damage) {
								if (dist_sqr < 0.25 * max_wash_dist * max_wash_dist) {
									do_damage = 1;
								}
							}
						} else {
							// check if inside cone - first fine apex of cone
							inset_depth = (bank->points[j].radius / fl_tan(half_angle));
							vm_vec_scale_add(&apex, &world_thruster_pos, &world_thruster_norm, -inset_depth);
							vm_vec_sub(&apex_to_ship, &objp->pos, &apex);
							vm_vec_normalize(&apex_to_ship);

							// check if inside cone angle
							if (vm_vec_dot(&apex_to_ship, &world_thruster_norm) > cosf(half_angle)) {
								vm_vec_cross(&temp, &world_thruster_norm, &thruster_to_ship);
								vm_vec_scale_add2(&shipp->wash_rot_axis, &temp, dot_to_ship / dist_sqr);
								ship_intensity += (1.0f - dist_sqr / (max_wash_dist*max_wash_dist)) * wash_intensity;
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
			max_ship_intensity_objp = wash_objp;
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
			damage = (0.001f * 0.003f * ENGINE_WASH_CHECK_INTERVAL * shipp->ship_max_hull_strength * shipp->wash_intensity);
		}

		ship_apply_wash_damage(objp, max_ship_intensity_objp, damage);

		// if we had no wash before now, add the wash object sound
		if(started_with_no_wash){
			if(shipp != Player_ship){
				obj_snd_assign(shipp->objnum, GameSounds::ENGINE_WASH, &vmd_zero_vector, OS_MAIN);
			} else {				
				Player_engine_wash_loop = snd_play_looping( gamesnd_get_game_sound(GameSounds::ENGINE_WASH), 0.0f , -1, -1, 1.0f);
			}
		}
	} 
	// if we've got no wash, kill any wash object sounds from this guy
	else {
		if(shipp != Player_ship){
			obj_snd_delete_type(shipp->objnum, GameSounds::ENGINE_WASH);
		} else {
			snd_stop(Player_engine_wash_loop);
			Player_engine_wash_loop = sound_handle::invalid();
		}
	}
}

/**
 * Engine wash level init
 */
void shipfx_engine_wash_level_init() { Player_engine_wash_loop = sound_handle::invalid(); }

/**
 * Pause engine wash sounds
 */
void shipfx_stop_engine_wash_sound()
{
	if (Player_engine_wash_loop.isValid()) {
		snd_stop(Player_engine_wash_loop);
		Player_engine_wash_loop = sound_handle::invalid();
	}
}

class CombinedVariable
{
public:
	static const int TYPE_NONE;
	static const int TYPE_FLOAT;
	static const int TYPE_IMAGE;
	static const int TYPE_INT;
	static const int TYPE_SOUND;
	static const int TYPE_STRING;
private:
	int Type;
	float	su_Float;
	int		su_Image;
	int		su_Int;
	gamesnd_id su_Sound;
	char	*su_String;
public:
	//TYPE_NONE
	CombinedVariable();
	//TYPE_FLOAT
	CombinedVariable(float n_Float);
	//TYPE_INT
	CombinedVariable(int n_Int);
	//TYPE_IMAGE
	CombinedVariable(int n_Int, ubyte type_override);
	//TYPE_SOUND
	CombinedVariable(gamesnd_id n_snd);
	//TYPE_STRING
	CombinedVariable(char *n_String);
	//All types
	~CombinedVariable();

	//Returns 1 if buffer was successfully written to
	int getFloat(float *output);
	//Returns handle or < 0 on failure/wrong type
	int getHandle();
	//Returns handle, or < 0 on failure/wrong type
	int getImage();
	//Returns 1 if buffer was successfully written to
	int getInt(int *output);
	//Returns handle, or < 0 on failure/wrong type
	gamesnd_id getSound();
	//Returns 1 if buffer was successfully written to
	int getString(char *output, size_t output_max);

	//Returns true if TYPE_NONE
	bool isEmpty();
};

//Workaround for MSVC6
const int CombinedVariable::TYPE_NONE=0;
const int CombinedVariable::TYPE_FLOAT = 1;
const int CombinedVariable::TYPE_IMAGE = 2;
const int CombinedVariable::TYPE_INT = 3;
const int CombinedVariable::TYPE_SOUND = 4;
const int CombinedVariable::TYPE_STRING  = 5;

//Member functions
CombinedVariable::CombinedVariable()
{
	Type = TYPE_NONE;
}

CombinedVariable::CombinedVariable(float n_Float)
{
	Type = TYPE_FLOAT;
	su_Float = n_Float;
}

CombinedVariable::CombinedVariable(int n_Int)
{
	Type = TYPE_INT;
	su_Int = n_Int;
}

CombinedVariable::CombinedVariable(int n_Int, ubyte type_override)
{
	if(type_override == TYPE_IMAGE)
	{
		Type = TYPE_IMAGE;
		su_Image = n_Int;
	}
	else
	{
		Type = TYPE_INT;
		su_Int = n_Int;
	}
}
CombinedVariable::CombinedVariable(gamesnd_id n_snd) {
	Type = TYPE_SOUND;
	su_Sound = n_snd;
}

CombinedVariable::CombinedVariable(char *n_String)
{
	Type = TYPE_STRING;
	su_String = (char *)vm_malloc(strlen(n_String)+1);
	strcpy(su_String, n_String);
}

CombinedVariable::~CombinedVariable()
{
	if(Type == TYPE_STRING)
	{
		vm_free(su_String);
	}
}

int CombinedVariable::getFloat(float *output)
{
	if(Type == TYPE_FLOAT)
	{
		*output  = su_Float;
		return 1;
	}
	if(Type == TYPE_IMAGE)
	{
		*output = i2fl(su_Image);
		return 1;
	}
	if(Type == TYPE_INT)
	{
		*output = i2fl(su_Int);
		return 1;
	}
	if(Type == TYPE_STRING)
	{
		*output = (float)atof(su_String);
		return 1;
	}
	return 0;
}
int CombinedVariable::getHandle()
{
	int i = 0;
	if(this->getInt(&i))
		return i;
	else
		return -1;
}
int CombinedVariable::getImage()
{
	if(Type == TYPE_IMAGE)
		return this->getHandle();
	else
		return -1;
}
int CombinedVariable::getInt(int *output)
{
	if(output == NULL)
		return 0;

	if(Type == TYPE_FLOAT)
	{
		*output  = fl2i(su_Float);
		return 1;
	}
	if(Type == TYPE_IMAGE)
	{
		*output = su_Image;
		return 1;
	}
	if(Type == TYPE_INT)
	{
		*output = su_Int;
		return 1;
	}
	if(Type == TYPE_STRING)
	{
		*output = atoi(su_String);
		return 1;
	}

	return 0;
}
gamesnd_id CombinedVariable::getSound()
{
	if(Type == TYPE_SOUND)
		return su_Sound;
	else
		return {};
}
int CombinedVariable::getString(char *output, size_t output_max)
{
	if(output == NULL || output_max == 0)
		return 0;

	if(Type == TYPE_FLOAT)
	{
		snprintf(output, output_max, "%f", su_Float);
		return 1;
	}
	if(Type == TYPE_IMAGE)
	{
		if(bm_is_valid(su_Image))
			snprintf(output, output_max, "%s", bm_get_filename(su_Image));
		return 1;
	}
	if(Type == TYPE_INT)
	{
		snprintf(output, output_max, "%i", su_Int);
		return 1;
	}
	if(Type == TYPE_SOUND)
	{
		Error(LOCATION, "Sound CombinedVariables are not supported yet.");
		/*if(snd_is_valid(su_Sound))
			snprintf(output, output_max, "%s", snd_get_filename(su_Sound));*/
		return 1;
	}
	if(Type == TYPE_STRING)
	{
		strncpy(output, su_String, output_max);
		return 1;
	}
	return 0;
}
bool CombinedVariable::isEmpty()
{
	return (Type != TYPE_NONE);
}

void parse_combined_variable_list(CombinedVariable *dest, flag_def_list *src, size_t num)
{
	if(dest == NULL || src == NULL || num == 0)
		return;

	char buf[NAME_LENGTH*2];
	buf[sizeof(buf)-1] = '\0';

	flag_def_list *sp = NULL;
	CombinedVariable *dp = NULL;
	for(size_t i = 0; i < num; i++)
	{
		sp = &src[i];
		dp = &dest[i];

		snprintf(buf, sizeof(buf)-1, "+%s:", sp->name);
		if(optional_string(buf))
		{
			switch(sp->var)
			{
				case CombinedVariable::TYPE_FLOAT:
				{
					float f = 0.0f;
					stuff_float(&f);
					*dp = CombinedVariable(f);
					break;
				}
				case CombinedVariable::TYPE_INT:
				{
					int myInt = 0;
					stuff_int(&myInt);
					*dp = CombinedVariable(myInt);
					break;
				}
				case CombinedVariable::TYPE_IMAGE:
				{
					char buf2[MAX_FILENAME_LEN];
					stuff_string(buf2, F_NAME, MAX_FILENAME_LEN);
					int idx = bm_load(buf2);
					*dp = CombinedVariable(idx, CombinedVariable::TYPE_IMAGE);
					break;
				}
				case CombinedVariable::TYPE_SOUND:
				{
					char buf2[MAX_FILENAME_LEN];
					stuff_string(buf2, F_NAME, MAX_FILENAME_LEN);
					auto idx = gamesnd_get_by_name(buf);
					*dp = CombinedVariable(idx);
					break;
				}
				case CombinedVariable::TYPE_STRING:
				{
					char buf2[MAX_NAME_LEN + MAX_FILENAME_LEN];
					stuff_string(buf2, F_NAME, MAX_FILENAME_LEN+MAX_NAME_LEN);
					*dp = CombinedVariable(buf2);
					break;
				}
			}
		}
	}
}

#define WV_ANIMATION		0
#define WV_RADIUS			1
#define WV_SPEED			2
#define WV_TIME				3

flag_def_list Warp_variables[] = {
	{"Animation",		WV_ANIMATION,		CombinedVariable::TYPE_STRING},
	{"Radius",			WV_RADIUS,			CombinedVariable::TYPE_FLOAT},
	{"Speed",			WV_SPEED,			CombinedVariable::TYPE_FLOAT},
	{"Time",			WV_TIME,			CombinedVariable::TYPE_FLOAT},
};


WarpParams::WarpParams()
{
	anim[0] = '\0';
}

bool WarpParams::operator==(const WarpParams &other)
{
	return direction == other.direction
		&& strcmp(anim, other.anim) == 0
		&& radius == other.radius
		&& snd_start == other.snd_start
		&& snd_end == other.snd_end
		&& speed == other.speed
		&& time == other.time
		&& accel_exp == other.accel_exp
		&& warp_type == other.warp_type
		&& warpout_engage_time == other.warpout_engage_time
		&& warpout_player_speed == other.warpout_player_speed;
}

bool WarpParams::operator!=(const WarpParams &other)
{
	return !(operator==(other));
}

SCP_vector<WarpParams> Warp_params;

int find_or_add_warp_params(const WarpParams &params)
{
	// see if these parameters already exist
	auto ii = std::find(Warp_params.begin(), Warp_params.end(), params);
	if (ii != Warp_params.end())
		return (int) (ii - Warp_params.begin());

	// these are unique, so add them
	Warp_params.push_back(params);
	return (int) (Warp_params.size() - 1);
}


const char *Warp_types[] = {
	"Default",
	"Knossos",
	"Babylon5",
	"Galactica",
	"Homeworld",
	"Hyperspace",
};

int Num_warp_types = sizeof(Warp_types) / sizeof(char*);

int warptype_match(const char *p)
{
	int i;
	for (i = 0; i < Num_warp_types; i++)
	{
		if (!stricmp(Warp_types[i], p))
			return i;
	}

	return -1;
}

void ship_set_warp_effects(object *objp)
{
	ship *shipp = &Ships[objp->instance];
	int warpin_type = Warp_params[shipp->warpin_params_index].warp_type;
	int warpout_type = Warp_params[shipp->warpout_params_index].warp_type;

	if (warpin_type & WT_DEFAULT_WITH_FIREBALL)
		warpin_type = WT_DEFAULT;
	if (warpout_type & WT_DEFAULT_WITH_FIREBALL)
		warpout_type = WT_DEFAULT;

	if (shipp->warpin_effect != nullptr)
		delete shipp->warpin_effect;

	switch (warpin_type)
	{
		case WT_DEFAULT:
		case WT_KNOSSOS:
		case WT_DEFAULT_THEN_KNOSSOS:
			shipp->warpin_effect = new WE_Default(objp, WarpDirection::WARP_IN);
			break;
		case WT_IN_PLACE_ANIM:
			shipp->warpin_effect = new WE_BSG(objp, WarpDirection::WARP_IN);
			break;
		case WT_SWEEPER:
			shipp->warpin_effect = new WE_Homeworld(objp, WarpDirection::WARP_IN);
			break;
		case WT_HYPERSPACE:
			shipp->warpin_effect = new WE_Hyperspace(objp, WarpDirection::WARP_IN);
			break;
		default:
			shipp->warpin_effect = new WarpEffect();
	}

	if (shipp->warpout_effect != nullptr)
		delete shipp->warpout_effect;

	switch (warpout_type)
	{
		case WT_DEFAULT:
		case WT_KNOSSOS:
		case WT_DEFAULT_THEN_KNOSSOS:
			shipp->warpout_effect = new WE_Default(objp, WarpDirection::WARP_OUT);
			break;
		case WT_IN_PLACE_ANIM:
			shipp->warpout_effect = new WE_BSG(objp, WarpDirection::WARP_OUT);
			break;
		case WT_SWEEPER:
			shipp->warpout_effect = new WE_Homeworld(objp, WarpDirection::WARP_OUT);
			break;
		case WT_HYPERSPACE:
			shipp->warpout_effect = new WE_Hyperspace(objp, WarpDirection::WARP_OUT);
			break;
		default:
			shipp->warpout_effect = new WarpEffect();
	}
}

// returns true if a point is on the "wrong side" of a portal effect i.e. the unrendered side
bool point_is_clipped_by_warp(const vec3d* point, WarpEffect* warp_effect) {
	if (warp_effect == nullptr)
		return false;

	vec3d warp_pnt, relative_direction;
	matrix warp_orient;

	warp_effect->getWarpPosition(&warp_pnt);
	warp_effect->getWarpOrientation(&warp_orient);

	vm_vec_sub(&relative_direction, point, &warp_pnt);
	return vm_vec_dot(&relative_direction, &warp_orient.vec.fvec) < 0.0f;
}


//********************-----CLASS: WarpEffect-----********************//
WarpEffect::WarpEffect()
{
	this->clear();
}

WarpEffect::WarpEffect(object *n_objp, WarpDirection n_direction)
{
	this->clear();
	if(n_objp != NULL && n_objp->type == OBJ_SHIP && n_objp->instance > -1 && Ships[n_objp->instance].ship_info_index > -1)
	{
		objp = n_objp;
		direction = n_direction;

		//Setup courtesy variables
		shipp = &Ships[objp->instance];
		sip = &Ship_info[shipp->ship_info_index];
		params = &Warp_params[direction == WarpDirection::WARP_IN ? shipp->warpin_params_index : shipp->warpout_params_index];
	}
}

void WarpEffect::clear()
{
	objp = NULL;
	direction = WarpDirection::NONE;
	shipp = NULL;
	sip = NULL;
}

bool WarpEffect::isValid()
{
	if(objp == NULL)
		return false;

	return true;
}

void WarpEffect::pageIn()
{
}
void WarpEffect::pageOut()
{
}

int WarpEffect::warpStart()
{
	if(!this->isValid())
		return 0;

	return 1;
}

int WarpEffect::warpFrame(float  /*frametime*/)
{
	return 0;
}

int WarpEffect::warpShipClip(model_render_params * /*render_info*/)
{
	return 0;
}

int WarpEffect::warpShipRender()
{
	return 0;
}

int WarpEffect::warpEnd()
{
	if(!this->isValid())
		return 0;

	shipp->flags.remove(Ship::Ship_Flags::Arriving_stage_1);
	shipp->flags.remove(Ship::Ship_Flags::Arriving_stage_2);
	shipp->flags.remove(Ship::Ship_Flags::Depart_warp);
	// dock leader needs to handle dockees
	if (object_is_docked(objp)) {
		dock_function_info dfi;
		dock_evaluate_all_docked_objects(objp, &dfi, object_remove_arriving_stage1_ndl_flag_helper);
		dock_evaluate_all_docked_objects(objp, &dfi, object_remove_arriving_stage2_ndl_flag_helper);
	}

	// let physics in on it too.
	objp->phys_info.flags &= (~PF_WARP_IN);
	objp->phys_info.flags &= (~PF_WARP_OUT);

	if(direction == WarpDirection::WARP_OUT)
		ship_actually_depart(objp->instance);

	return 1;
}

int WarpEffect::getWarpPosition(vec3d *output)
{
	if(!this->isValid())
		return 0;

	*output = objp->pos;
	return 1;
}

int WarpEffect::getWarpOrientation(matrix* output)
{
	if(!this->isValid())
		return 0;

	*output = objp->orient;
	return 1;
}

//********************-----CLASS: WE_Default-----********************//
WE_Default::WE_Default(object *n_objp, WarpDirection n_direction)
	:WarpEffect(n_objp, n_direction)
{
	if(!this->isValid())
		return;

	portal_objp = nullptr;
	stage_duration[0] = 0;

	pos = vmd_zero_vector;
	fvec = vmd_zero_vector;
}

int WE_Default::warpStart()
{
	if (!this->isValid())
		return 0;

	if(direction == WarpDirection::WARP_OUT && objp == Player_obj)
	{
		HUD_printf(NOX("Subspace drive engaged"));
	}

	// see if we have a valid Knossos device
	portal_objp = nullptr;
	if ((direction == WarpDirection::WARP_IN) && shipfx_special_warp_objnum_valid(shipp->special_warpin_objnum))
	{
		portal_objp = &Objects[shipp->special_warpin_objnum];
	}
	else if ((direction == WarpDirection::WARP_OUT) && shipfx_special_warp_objnum_valid(shipp->special_warpout_objnum))
	{
		portal_objp = &Objects[shipp->special_warpout_objnum];
	}

	// knossos warpout only valid in single player
	if (portal_objp != nullptr && Game_mode & GM_MULTIPLAYER)
	{
		mprintf(("special warpout only for single player\n"));
		portal_objp = nullptr;
	}

	// determine the correct center of the model (which may not be the model's origin)
	if (object_is_docked(objp))
		dock_calc_docked_actual_center(&actual_local_center, objp);
	else
		ship_class_get_actual_center(sip, &actual_local_center);

	// determine the half-length and the warping distance (which is actually the full length)
	if (object_is_docked(objp))
	{
		// we need to get the longitudinal radius of our ship, so find the semilatus rectum along the Z-axis
		half_length = dock_calc_max_semilatus_rectum_parallel_to_axis(objp, Z_AXIS);
		warping_dist = 2.0f * half_length;
	}
	else
	{
		warping_dist = ship_class_get_length(sip);
		half_length = 0.5f * warping_dist;
	}

	// determine the warping time
	warping_time = shipfx_calculate_warp_time(objp, direction, half_length, warping_dist);

	// determine the warping speed
	if (direction == WarpDirection::WARP_OUT)
		warping_speed = ship_get_warpout_speed(objp, sip, half_length, warping_dist);
	else
		warping_speed = warping_dist / warping_time;

	// done with initial computation; now set up the warp effect

	float effect_time = 0.0f;
	if(direction == WarpDirection::WARP_IN)
	{
		// first determine the world center in relation to its position
		vm_vec_unrotate(&pos, &actual_local_center, &objp->orient);
		vm_vec_add2(&pos, &objp->pos);

		// now project the warp effect forward
		vm_vec_scale_add( &pos, &pos, &objp->orient.vec.fvec, half_length );

		// Effect time is 'SHIPFX_WARP_DELAY' (1.5 secs) seconds to start, warping_time 
		// for ship to go thru, and 'SHIPFX_WARP_DELAY' (1.5 secs) to go away.
		effect_time = warping_time + SHIPFX_WARP_DELAY + SHIPFX_WARP_DELAY;
	}
	else
	{
		compute_warpout_stuff(&effect_time, &pos);
		effect_time += SHIPFX_WARP_DELAY;
	}

	radius = shipfx_calculate_effect_radius(objp, direction);
	// cap radius to size of knossos
	if (portal_objp != nullptr)
	{
		// cap radius to size of knossos
		radius = MIN(radius, 0.8f*portal_objp->radius);
	}

	// select the fireball we use
	int fireball_type = FIREBALL_WARP;
	if ((portal_objp != nullptr) || (params->warp_type == WT_KNOSSOS) || (direction == WarpDirection::WARP_OUT && params->warp_type == WT_DEFAULT_THEN_KNOSSOS))
		fireball_type = FIREBALL_KNOSSOS;
	else if (params->warp_type & WT_DEFAULT_WITH_FIREBALL)
		fireball_type = params->warp_type & WT_FLAG_MASK;

	// create fireball
	int warp_objnum = fireball_create(&pos, fireball_type, FIREBALL_WARP_EFFECT, OBJ_INDEX(objp), radius, (direction == WarpDirection::WARP_OUT), nullptr, effect_time, shipp->ship_info_index, nullptr, 0, 0, params->snd_start, params->snd_end);

	//WMC - bail
	// JAS: This must always be created, if not, just warp the ship in/out
	if (warp_objnum < 0)
	{
		this->warpEnd();
		return 0;
	}

	fvec = Objects[warp_objnum].orient.vec.fvec;

	stage_time_start = total_time_start = timestamp();
	if(direction == WarpDirection::WARP_IN)
	{
		stage_duration[1] = fl2i(SHIPFX_WARP_DELAY*1000.0f);
		stage_duration[2] = fl2i(warping_time*1000.0f);
		stage_time_end = timestamp(stage_duration[1]);
		total_time_end = stage_duration[1] + stage_duration[2];
		shipp->flags.set(Ship::Ship_Flags::Arriving_stage_1);
		// dock leader needs to handle dockees
		if (object_is_docked(objp)) {
			Assertion(shipp->flags[Ship::Ship_Flags::Dock_leader], "The docked ship warping in (%s) should only be the dock leader at this point!\n", shipp->ship_name);
			dock_function_info dfi;
			dock_evaluate_all_docked_objects(objp, &dfi, object_set_arriving_stage1_ndl_flag_helper);
		}
	}
	else if(direction == WarpDirection::WARP_OUT)
	{
        shipp->flags.set(Ship::Ship_Flags::Depart_warp);

		// Make the warp effect stage 1 last SHIP_WARP_TIME1 seconds.
		if ( objp == Player_obj )	{
			stage_duration[1] = fl2i(fireball_lifeleft(&Objects[warp_objnum])*1000.0f);
			total_time_end = timestamp(fl2i(effect_time*1000.0f));
		} else {
			total_time_end = timestamp(fl2i(effect_time*1000.0f));
		}

		// This is a hack to make the ship go at the right speed to go from it's current position to the warp_effect_pos;
		
		// Set ship's velocity to 'speed'
		// This should actually be an AI that flies from the current
		// position through 'shipp->warp_effect_pos' in 'warp_time'
		// and keeps going 
		if ( objp != Player_obj || Player_use_ai )	{
			vec3d vel;
			vel = objp->orient.vec.fvec;
			vm_vec_scale( &vel, warping_speed );
			objp->phys_info.vel = vel;
			objp->phys_info.desired_vel = vel;
			objp->phys_info.prev_ramp_vel.xyz.x = 0.0f;
			objp->phys_info.prev_ramp_vel.xyz.y = 0.0f;
			objp->phys_info.prev_ramp_vel.xyz.z = warping_speed;
			objp->phys_info.forward_thrust = 1.0f;		// How much the forward thruster is applied.  0-1.
		}
	}

	return 1;
}

int WE_Default::warpFrame(float frametime)
{
	if(direction == WarpDirection::WARP_IN)
	{
		if ((shipp->flags[Ship::Ship_Flags::Arriving_stage_1]) && timestamp_elapsed(stage_time_end))
		{
			// let physics know the ship is going to warp in.
			objp->phys_info.flags |= PF_WARP_IN;

			// done doing stage 1 of warp, so go on to stage 2
			shipp->flags.remove(Ship::Ship_Flags::Arriving_stage_1);
			shipp->flags.set(Ship::Ship_Flags::Arriving_stage_2);
			// if the ship is a dock leader; handle all the dockees
			if (object_is_docked(objp)) {
				Assertion(shipp->flags[Ship::Ship_Flags::Dock_leader], "The docked ship warping in (%s) should only be the dock leader at this point!\n", shipp->ship_name);
				dock_function_info dfi;
				dock_evaluate_all_docked_objects(objp, &dfi, object_remove_arriving_stage1_ndl_flag_helper);
				dock_evaluate_all_docked_objects(objp, &dfi, object_set_arriving_stage2_ndl_flag_helper);
			}
			// Make ship move at velocity so that it moves two radii in warp_time seconds.
			vec3d vel;
			vel = objp->orient.vec.fvec;
			vm_vec_scale( &vel, warping_speed );
			objp->phys_info.vel = vel;
			objp->phys_info.desired_vel = vel;
			objp->phys_info.prev_ramp_vel.xyz.x = 0.0f;
			objp->phys_info.prev_ramp_vel.xyz.y = 0.0f;
			objp->phys_info.prev_ramp_vel.xyz.z = warping_speed;
			objp->phys_info.forward_thrust = 0.0f;		// How much the forward thruster is applied.  0-1.

			stage_time_end = timestamp(fl2i(warping_time*1000.0f));
		}
		else if ( (shipp->flags[Ship::Ship_Flags::Arriving_stage_2]) && timestamp_elapsed(stage_time_end) )
		{
			// done doing stage 2 of warp, so turn off arriving flag
			this->warpEnd();

			// notify physics to slow down
			if (sip->flags[Ship::Info_Flags::Supercap]) {
				// let physics know this is a special warp in
				objp->phys_info.flags |= PF_SPECIAL_WARP_IN;
			}
		}
	}
	else if(direction == WarpDirection::WARP_OUT)
	{
		vec3d tempv;
		float warp_pos;	// position of warp effect in object's frame of reference

		vm_vec_sub( &tempv, &objp->pos, &pos );
		warp_pos = -vm_vec_dot( &tempv, &fvec );


		// Find the closest point on line from center of wormhole
		vec3d cpos;

		fvi_ray_plane(&cpos, &objp->pos, &fvec, &pos, &fvec, 0.0f );

		if ( objp == Player_obj )	{
			// Code for player warpout frame

			if ( (Player->control_mode==PCM_WARPOUT_STAGE2) && (warp_pos > objp->radius) )	{
				gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_DONE_STAGE2 );
			}

			if ( timestamp_elapsed(total_time_end) ) {

				// Something went wrong... oh well, warp him out anyway.
				if ( Player->control_mode != PCM_WARPOUT_STAGE3 )	{
					mprintf(( "Hmmm... player ship warpout time elapsed, but he wasn't in warp stage 3.\n" ));
				}

				this->warpEnd();
			}

		} else {
			// Code for all non-player ships warpout frame

			int timed_out = timestamp_elapsed(total_time_end);
			if ( timed_out )	{
				int	delta_ms = timestamp_until(total_time_end);
				if (delta_ms > 1000.0f * frametime ) {
					nprintf(("AI", "Frame %i: Ship %s missed departue cue by %7.3f seconds.\n", Framecount, shipp->ship_name, - (float) delta_ms/1000.0f));
				}
			}

			// MWA 10/21/97 -- added shipp->flags[Ship::Ship_Flags::No_departure_warp] part of next if statement.  For ships
			// that don't get a wormhole effect, I wanted to drop into this code immediately.
			if ( (warp_pos > objp->radius)  || (shipp->flags[Ship::Ship_Flags::No_departure_warp]) || timed_out )	{
				this->warpEnd();
			} 
		}
	}

	return 1;
}

int WE_Default::warpShipClip(model_render_params *render_info)
{
	if(!this->isValid())
		return 0;

	render_info->set_clip_plane(pos, fvec);

	return 1;
}

int WE_Default::warpShipRender()
{
	return 1;
}

int WE_Default::getWarpPosition(vec3d *output)
{
	if(!this->isValid())
		return 0;

	*output = pos;
	return 1;
}

int WE_Default::getWarpOrientation(matrix* output)
{
	if (!this->isValid())
		return 0;

	if (this->direction == WarpDirection::WARP_IN)
		vm_vector_2_matrix(output, &objp->orient.vec.fvec, nullptr, nullptr);
	else {
		vec3d backwards = objp->orient.vec.fvec;
		vm_vec_negate(&backwards);
		vm_vector_2_matrix(output, &backwards, nullptr, nullptr);
	}
    return 1;
}

//********************-----CLASS: WE_BSG-----********************//
WE_BSG::WE_BSG(object *n_objp, WarpDirection n_direction)
	:WarpEffect(n_objp, n_direction)
{
	//Zero animation and such
	anim = shockwave = -1;
	anim_fps = shockwave_fps = 0;
	anim_nframes = shockwave_nframes = 0;
	anim_total_time = shockwave_total_time = 0;

	//Setup anim name
	char tmp_name[MAX_FILENAME_LEN];
	memset(tmp_name, 0, MAX_FILENAME_LEN);
	strcpy_s(tmp_name, params->anim);
	strlwr(tmp_name);

	if(strlen(tmp_name))
	{
		//Load anim
		anim = bm_load_either(tmp_name, &anim_nframes, &anim_fps, NULL, true);
		if(anim > -1)
		{
			anim_total_time = fl2i(((float)anim_nframes / (float)anim_fps) * 1000.0f);
		}

		//Setup shockwave name
		strncat(tmp_name, "-shockwave", MAX_FILENAME_LEN-1);

		//Load shockwave
		shockwave = bm_load_either(tmp_name, &shockwave_nframes, &shockwave_fps, NULL, true);
		if(shockwave > -1)
		{
			shockwave_total_time = fl2i(((float)shockwave_nframes / (float)shockwave_fps) * 1000.0f);
		}
	}
	//Set radius
	tube_radius = 0.0f;
	shockwave_radius = 0.0f;

	//Use the warp radius for shockwave radius, not tube radius
	shockwave_radius = params->radius;

	polymodel *pm = model_get(sip->model_num);
	if(pm == NULL)
	{
		autocenter = vmd_zero_vector;
		z_offset_max = objp->radius;
		z_offset_min = -objp->radius;

		if(tube_radius <= 0.0f)
			tube_radius = objp->radius;

		if(shockwave_radius <= 0.0f)
			shockwave_radius = objp->radius;	
	}
	else
	{
		//Autogenerate everything from ship dimensions
		if(tube_radius <= 0.0f)
			tube_radius = MAX((pm->maxs.xyz.y - pm->mins.xyz.y), (pm->maxs.xyz.x - pm->mins.xyz.x))/2.0f;
		autocenter = pm->autocenter;
		z_offset_max = pm->maxs.xyz.z - pm->autocenter.xyz.z;
		z_offset_min = pm->mins.xyz.z - pm->autocenter.xyz.z;

		if (shockwave_radius <= 0.0f)
			shockwave_radius = z_offset_max - z_offset_min;
	}

	//*****Timing
	stage = -1;

	stage_duration[0] = params->time;
	stage_duration[1] = MAX(anim_total_time - params->time, shockwave_total_time);

	stage_time_start = stage_time_end = total_time_start = total_time_end = timestamp();

	//*****Sound
	snd_range_factor = 1.0f;
	snd_start = snd_end = sound_handle::invalid();
	snd_start_gs = snd_end_gs = NULL;

	//*****Instance
	pos = vmd_zero_vector;
}

WE_BSG::~WE_BSG()
{
	if(anim > -1)
		bm_unload(anim);
	if(shockwave > -1)
		bm_unload(shockwave);
}

void WE_BSG::pageIn()
{
	if(anim > -1)
		bm_page_in_texture(anim);
	if(shockwave > -1)
		bm_page_in_texture(shockwave);
}

int WE_BSG::warpStart()
{
	if(!WarpEffect::warpStart())
		return 0;

	//WMC - bail
	if (anim < 0 && shockwave < 0)
	{
		this->warpEnd();
		return 0;
	}

	//WMC - If object is docked now, update data:
	if(object_is_docked(objp))
	{
		z_offset_max = dock_calc_max_semilatus_rectum_parallel_to_axis(objp, Z_AXIS);
		z_offset_min = -z_offset_max;
		if(tube_radius <= 0.0f)
		{
			float x_radius = dock_calc_max_semilatus_rectum_parallel_to_axis(objp, X_AXIS);
			float y_radius = dock_calc_max_semilatus_rectum_parallel_to_axis(objp, Y_AXIS);
			tube_radius = MAX(x_radius, y_radius);
		}

		shockwave_radius = z_offset_max - z_offset_min;

		dock_calc_docked_actual_center(&autocenter, objp);
	}

	if (direction == WarpDirection::WARP_IN)
	{
		shipp->flags.set(Ship::Ship_Flags::Arriving_stage_1);
		// dock leader needs to handle dockees
		if (object_is_docked(objp)) {
			Assertion(shipp->flags[Ship::Ship_Flags::Dock_leader], "The ship warping in (%s) must be the dock leader at this point!\n", shipp->ship_name);
			dock_function_info dfi;
			dock_evaluate_all_docked_objects(objp, &dfi, object_set_arriving_stage1_ndl_flag_helper);
		}
	}
	else
        shipp->flags.set(Ship::Ship_Flags::Depart_warp);

	//*****Sound
	if(params->snd_start.isValid())
	{
		snd_start_gs = gamesnd_get_game_sound(params->snd_start);
		snd_start = snd_play_3d(snd_start_gs, &objp->pos, &View_position, 0.0f, NULL, 0, 1, SND_PRIORITY_SINGLE_INSTANCE, NULL, snd_range_factor);
	}
	if(params->snd_end.isValid())
	{
		snd_end_gs = gamesnd_get_game_sound(params->snd_end);
		snd_end    = sound_handle::invalid();
	}

	stage = 0;
	int total_duration = 0;
	for(int i = 0; i < WE_BSG_NUM_STAGES; i++)
		total_duration += stage_duration[i];

	total_time_start = timestamp();
	total_time_end = timestamp(total_duration);

	stage_time_start = total_time_start;
	stage_time_end = timestamp(stage_duration[stage]);

	return 1;
}

int WE_BSG::warpFrame(float  /*frametime*/)
{
	if(!this->isValid())
		return 0;

	while( timestamp_elapsed(stage_time_end ))
	{
		stage++;
		if(stage < WE_BSG_NUM_STAGES)
		{
			stage_time_start = timestamp();
			stage_time_end = timestamp(stage_duration[stage]);
		}
		switch(stage)
		{
			case 1:
				if(direction == WarpDirection::WARP_IN)
				{
					shipp->flags.remove(Ship::Ship_Flags::Arriving_stage_1);
					shipp->flags.set(Ship::Ship_Flags::Arriving_stage_2);
					// dock leader needs to handle dockees
					if (object_is_docked(objp)) {
						Assertion(shipp->flags[Ship::Ship_Flags::Dock_leader], "The ship warping in (%s) must be the dock leader at this point!\n", shipp->ship_name);
						dock_function_info dfi;
						dock_evaluate_all_docked_objects(objp, &dfi, object_remove_arriving_stage1_ndl_flag_helper);
						dock_evaluate_all_docked_objects(objp, &dfi, object_set_arriving_stage2_ndl_flag_helper);
					}
				}
				break;
			default:
				this->warpEnd();
				return 0;
		}
	}

	switch(stage)
	{
		case 0:
		case 1:
			vm_vec_unrotate(&pos, &autocenter, &objp->orient);
			vm_vec_add2(&pos, &objp->pos);
			break;
		default:
			this->warpEnd();
			return 0;
	}

	if (snd_start.isValid())
		snd_update_3d_pos(snd_start, snd_start_gs, &objp->pos, 0.0f, snd_range_factor);

	return 1;
}

int WE_BSG::warpShipClip(model_render_params *render_info)
{
	if(!this->isValid())
		return 0;

	if(direction == WarpDirection::WARP_OUT && stage > 0)
	{
		vec3d position;
		vm_vec_scale_add(&position, &objp->pos, &objp->orient.vec.fvec, objp->radius);

		render_info->set_clip_plane(position, objp->orient.vec.fvec);
	}

	return 1;
}

int WE_BSG::warpShipRender()
{
	if(!this->isValid())
		return 0;

	if(anim < 0 && shockwave < 0)
		return 0;

	// SUSHI: Turning off Zbuffering results in the FTL effect showing up through ship hulls. 
	// The effect is slightly degraded by leaving it on, but ATM it's worth the tradeoff.
	// turn off zbuffering	
	//int saved_zbuffer_mode = gr_zbuffer_get();
	//gr_zbuffer_set(GR_ZBUFF_NONE);

	if(anim > -1)
	{
		//Figure out which frame we're on
		int anim_frame = fl2i( ((float)(timestamp() - total_time_start)/1000.0f) * (float)anim_fps);

		if ( anim_frame < anim_nframes )
		{
			//Set the correct frame
			//gr_set_bitmap(anim + anim_frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		

			//Do warpout geometry
			vec3d start, end;
			vm_vec_scale_add(&start, &pos, &objp->orient.vec.fvec, z_offset_min);
			vm_vec_scale_add(&end, &pos, &objp->orient.vec.fvec, z_offset_max);

			//Render the warpout effect
			//batch_add_beam(anim + anim_frame, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT, &start, &end, tube_radius*2.0f, 1.0f);
			batching_add_beam(anim + anim_frame, &start, &end, tube_radius * 2.0f, 1.0f);
		}
	}

	if(stage == 1 && shockwave > -1)
	{
		int shockwave_frame = fl2i( ((float)(timestamp() - stage_time_start)/1000.0f) * (float)shockwave_fps);

		if(shockwave_frame < shockwave_nframes)
		{
			vertex p;
            
            memset(&p, 0, sizeof(p));
            
			g3_transfer_vertex(&p, &pos);

			batching_add_volume_bitmap(shockwave + shockwave_frame, &p, 0, shockwave_radius, 1.0f);
		}
	}

	// restore zbuffer mode
	//gr_zbuffer_set(saved_zbuffer_mode);
	return 1;
}

int WE_BSG::warpEnd()
{
	if (snd_start.isValid())
		snd_stop(snd_start);
	if(snd_end_gs != NULL)
		snd_end = snd_play_3d(snd_end_gs, &objp->pos, &View_position, 0.0f, NULL, 0, 1.0f, SND_PRIORITY_SINGLE_INSTANCE, NULL, snd_range_factor);

	return WarpEffect::warpEnd();
}

//WMC - These two functions are used to fool collision detection code
//And do player warpout
int WE_BSG::getWarpPosition(vec3d *output)
{
	if(!this->isValid())
		return 0;

	vec3d position;
	vm_vec_scale_add(&position, &objp->pos, &objp->orient.vec.fvec, objp->radius);

	*output = position;
	return 1;
}

int WE_BSG::getWarpOrientation(matrix* output)
{
    if (!this->isValid())
    {
        return 0;
    }

    vm_vector_2_matrix(output, &objp->orient.vec.fvec, NULL, NULL);
    return 1;
}

//********************-----CLASS: WE_Homeworld-----********************//
const float HOMEWORLD_SWEEPER_LINE_THICKNESS = 0.002f; // How tall the initial and final hyperspace "lines" are, as a factor of height
WE_Homeworld::WE_Homeworld(object *n_objp, WarpDirection n_direction)
	:WarpEffect(n_objp, n_direction)
{
	if(!this->isValid())
		return;

	//Stage and time
	stage = 0;
	stage_time_start = stage_time_end = timestamp();

	//Stage duration presets
	stage_duration[0] = 0;
	stage_duration[1] = 1200;
	stage_duration[2] = 1600;
	stage_duration[3] = -1;
	stage_duration[4] = 1600;
	stage_duration[5] = 1200;

	//Configure stage duration 3
	stage_duration[3] = params->time - (stage_duration[1] + stage_duration[2] + stage_duration[4] + stage_duration[5]);
	if (stage_duration[3] <= 0)
		stage_duration[3] = 3400; // set for a 9 second total time

	//Anim
	anim = bm_load_either(params->anim, &anim_nframes, &anim_fps, nullptr, true);

	pos = vmd_zero_vector;
	fvec = vmd_zero_vector;

	polymodel *pm = model_get(sip->model_num);
	width_full = params->radius;
	if(pm != nullptr)
	{
		if(width_full <= 0.0f)
		{
			width_full = pm->maxs.xyz.x - pm->mins.xyz.x;
			height_full = pm->maxs.xyz.y - pm->mins.xyz.y;
			z_offset_max = pm->maxs.xyz.z;
			z_offset_min = pm->mins.xyz.z;
		}
	}
	else
	{
		if(width_full <= 0.0f)
		{
			width_full = 2.0f*objp->radius;
		}
		height_full = width_full;
		z_offset_max = objp->radius;
		z_offset_min = -objp->radius;
	}
	//WMC - This scales up or down the sound depending on ship size, with ~100m diameter ship as base
	//REMEMBER: Radius != diameter
	snd_range_factor = sqrt(width_full*width_full+height_full*height_full)/141.421356f;

	if(width_full <= 0.0f)
		width_full = 1.0f;
	if(height_full <= 0.0f)
		height_full = 1.0f;
	width = width_full;
	height = 0.0f;

	//Sound
	snd    = sound_handle::invalid();
	snd_gs = NULL;
}

WE_Homeworld::~WE_Homeworld()
{
	if(anim > -1)
		bm_unload(anim);
}

int WE_Homeworld::warpStart()
{
	if(!this->isValid())
		return 0;

	if(anim < 0)
	{
		this->warpEnd();
		return 0;
	}
	
	stage = 1;
	total_time_start = timestamp();
	total_time_end = 0;
	for(int i = 0; i < WE_HOMEWORLD_NUM_STAGES; i++)
	{
		total_time_end += stage_duration[i];
	}
	stage_time_start = total_time_start;
	stage_time_end = timestamp(stage_duration[stage]);

	//Position
	vm_vec_scale_add(&pos, &objp->pos, &objp->orient.vec.fvec, z_offset_max);
	fvec = objp->orient.vec.fvec;
	if(direction == WarpDirection::WARP_OUT)
		vm_vec_negate(&fvec);

	width = 0.f;
	height = height_full * HOMEWORLD_SWEEPER_LINE_THICKNESS;

	if(direction == WarpDirection::WARP_IN)
	{
		shipp->flags.set(Ship::Ship_Flags::Arriving_stage_1);
		// dock leader needs to handle dockees
		if (object_is_docked(objp)) {
			Assertion(shipp->flags[Ship::Ship_Flags::Dock_leader], "The ship warping in (%s) must be the dock leader at this point!\n", shipp->ship_name);
			dock_function_info dfi;
			dock_evaluate_all_docked_objects(objp, &dfi, object_set_arriving_stage1_ndl_flag_helper);
		}
	}
	else if(direction == WarpDirection::WARP_OUT)
	{
        shipp->flags.set(Ship::Ship_Flags::Depart_warp);
	}
	else
	{
		this->warpEnd();
		return 0;
	}

	if(params->snd_start.isValid())
	{
		snd_gs = gamesnd_get_game_sound(params->snd_start);
		snd = snd_play_3d(snd_gs, &pos, &View_position, 0.0f, NULL, 0, 1, SND_PRIORITY_SINGLE_INSTANCE, NULL, snd_range_factor);
	}

	return 1;
}

int WE_Homeworld::warpFrame(float  /*frametime*/)
{
	if(!this->isValid())
		return 0;

	//Setup stage
	while( timestamp_elapsed(stage_time_end ))
	{
		stage++;
		if(stage < WE_HOMEWORLD_NUM_STAGES)
		{
			stage_time_start = timestamp();
			stage_time_end = timestamp(stage_duration[stage]);
		}
		switch(stage)
		{
			case 2:
				break;
			case 3:
				if(direction == WarpDirection::WARP_IN)
				{
					objp->phys_info.flags |= PF_WARP_IN;
					shipp->flags.remove(Ship::Ship_Flags::Arriving_stage_1);
					shipp->flags.set(Ship::Ship_Flags::Arriving_stage_2);
					// dock leader needs to handle dockees
					if (object_is_docked(objp)) {
						Assertion(shipp->flags[Ship::Ship_Flags::Dock_leader], "The ship warping in (%s) must be the dock leader at this point!\n", shipp->ship_name);
						dock_function_info dfi;
						dock_evaluate_all_docked_objects(objp, &dfi, object_remove_arriving_stage1_ndl_flag_helper);
						dock_evaluate_all_docked_objects(objp, &dfi, object_set_arriving_stage2_ndl_flag_helper);
					}
				}

				break;
			case 4:
				break;
			case 5:
				break;
			default:
				this->warpEnd();
				return 0;
		}
	}

	//Process stage
	float sweeper_z_position;

	float progress = ((float)timestamp() - (float)stage_time_start) / ((float)stage_time_end - (float)stage_time_start);
	float adjusted_progress = progress * 1.2f - 0.1f;    // we'll let the progress "overflow" a bit and then clamp it, so the sweeper
	CLAMP(adjusted_progress, 0.f, 1.f);                  // lingers for a moment at the beginning and end of the grow and shrink stages
	float visual_progress = ((3.f - 2 * adjusted_progress) * adjusted_progress * adjusted_progress); // good ol' cubic easing function

	switch(stage)
	{
		case 1:
			sweeper_z_position = z_offset_max;
			width = width_full * visual_progress;
			break;
		case 2:
			sweeper_z_position = z_offset_max;
			height = (height_full * visual_progress) + height_full * HOMEWORLD_SWEEPER_LINE_THICKNESS;
			break;
		case 3:
			sweeper_z_position = z_offset_max - adjusted_progress * (z_offset_max - z_offset_min);
			break;
		case 4:
			sweeper_z_position = z_offset_min;
			height = (height_full * (1.f - visual_progress)) + height_full * HOMEWORLD_SWEEPER_LINE_THICKNESS;
			break;
		case 5:
			sweeper_z_position = z_offset_min;
			width = width_full * (1.f - visual_progress);
			break;
		default:
			this->warpEnd();
			return 0;
	}

	//update sweeper position
	vm_vec_scale_add(&pos, &objp->pos, &objp->orient.vec.fvec, sweeper_z_position);

	//Update sound
	if (snd.isValid())
		snd_update_3d_pos(snd, snd_gs, &pos, 0.0f, snd_range_factor);
		
	return 1;
}

int WE_Homeworld::warpShipClip(model_render_params *render_info)
{
	if(!this->isValid())
		return 0;

	render_info->set_clip_plane(pos, fvec);
	return 1;
}

int WE_Homeworld::warpShipRender()
{
	if(!this->isValid())
		return 0;

	int frame = 0;
	if(anim_fps > 0)
		frame = fl2i( (int)(((float)(timestamp() - (float)total_time_start)/1000.0f) * (float)anim_fps) % anim_nframes);

	//Set the correct frame
	batching_add_polygon(anim + frame, &pos, &objp->orient, width, height);

	return 1;
}

int WE_Homeworld::warpEnd()
{
	if (snd.isValid())
		snd_stop(snd);
	return WarpEffect::warpEnd();
}

int WE_Homeworld::getWarpPosition(vec3d *output)
{
	if(!this->isValid())
		return 0;

	*output = pos;
	return 1;
}

int WE_Homeworld::getWarpOrientation(matrix* output)
{
    if (!this->isValid())
    {
        return 0;
    }

	if (this->direction == WarpDirection::WARP_IN)
		*output = objp->orient;
	else {
		vec3d backwards = objp->orient.vec.fvec;
		vm_vec_negate(&backwards);
		vm_vector_2_matrix(output, &backwards, &objp->orient.vec.uvec, nullptr);
	}

    return 1;
}

//********************-----CLASS: WE_Hyperspace----********************//
WE_Hyperspace::WE_Hyperspace(object *n_objp, WarpDirection n_direction)
	:WarpEffect(n_objp, n_direction)
{
	total_duration = params->time;
	if (total_duration <= 0)
		total_duration = 1000;

	accel_or_decel_exp = params->accel_exp;

	total_time_start = total_time_end = timestamp();
	pos_final = vmd_zero_vector;
	scale_factor = 750.0f * objp->radius;
	
	initial_velocity = 1.0f;
	
	//*****Sound
	snd_range_factor = 1.0f * objp->radius;
	snd_start = snd_end = sound_handle::invalid();
	snd_start_gs = snd_end_gs = nullptr;
}

int WE_Hyperspace::warpStart()
{
	if(!this->isValid())
		return 0;

	total_time_start = timestamp();
	total_time_end = timestamp(total_duration);
	
	if(direction == WarpDirection::WARP_IN)
	{
		p_object* p_objp = mission_parse_get_parse_object(shipp->ship_name);
		if (p_objp != nullptr) {
			initial_velocity = (float)p_objp->initial_velocity * sip->max_speed / 100.0f;
		}

		shipp->flags.set(Ship::Ship_Flags::Arriving_stage_1);
		// dock leader needs to handle dockees
		if (object_is_docked(objp)) {
			Assertion(shipp->flags[Ship::Ship_Flags::Dock_leader], "The ship warping in (%s) must be the dock leader at this point!\n", shipp->ship_name);
			dock_function_info dfi;
			dock_evaluate_all_docked_objects(objp, &dfi, object_set_arriving_stage1_ndl_flag_helper);
			// docked objects use speed to find the object that controls movement; therefore the warping in dock leader must have the highest speed!
			objp->phys_info.speed = (scale_factor / params->time)*1000.0f;
		}
		objp->phys_info.flags |= PF_WARP_IN;
		objp->flags.remove(Object::Object_Flags::Physics);
	}
	else if(direction == WarpDirection::WARP_OUT)
	{
		// wookieejedi - if the ship is already in the mission the initial_velocity for warpout should be the ship's current speed
		initial_velocity = objp->phys_info.fspeed;
		shipp->flags.set(Ship::Ship_Flags::Depart_warp);
	}
	else
	{
		this->warpEnd();
	}

	pos_final = objp->pos;
	
	// Cyborg17 - After setting pos_final, we should move the ship to the actual starting position.
	vm_vec_scale_add(&objp->pos, &pos_final, &objp->orient.vec.fvec, -scale_factor);

	if (params->snd_start.isValid())
	{
		snd_start_gs = gamesnd_get_game_sound(params->snd_start);
		snd_start = snd_play_3d(snd_start_gs, &pos_final, &View_position, 0.0f, nullptr, 0, 1, SND_PRIORITY_SINGLE_INSTANCE, nullptr, snd_range_factor);
	}
	if (params->snd_end.isValid())
	{
		snd_end_gs = gamesnd_get_game_sound(params->snd_end);
		snd_end    = sound_handle::invalid();
	}
	
	return 1;
}

int WE_Hyperspace::warpFrame(float  /*frametime*/)
{
	if(!this->isValid())
		return 0;

	if(timestamp_elapsed(total_time_end))
	{
		objp->pos = pos_final;
        objp->flags.set(Object::Object_Flags::Physics);
		this->warpEnd();
	}
	else
	{
		// How far along in the effect we are, in range of 0.0..1.0.
		float progress = ((float)timestamp() - (float)total_time_start)/(float)total_duration;
		float scale = 0.0f;
		if (direction == WarpDirection::WARP_IN)
		{
			scale = scale_factor*(1.0f-pow((1.0f-progress), accel_or_decel_exp))-scale_factor;

			// Makes sure that the velocity won't drop below the ship's initial
			// velocity during the warpin. Ideally it should be done more
			// smoothly than this.
			scale = MIN(scale, (initial_velocity * (total_duration / 1000) * -(1.0f - progress)));
		}
		else
		{
			scale = scale_factor*pow(progress, accel_or_decel_exp);

			// Makes sure the warpout velocity won't drop below the ship's last
			// known real velocity.
			scale += initial_velocity * (total_duration / 1000) * progress;
		}
		vm_vec_scale_add(&objp->pos, &pos_final, &objp->orient.vec.fvec, scale);
	}
	
	if (snd_start.isValid())
		snd_update_3d_pos(snd_start, snd_start_gs, &pos_final, 0.0f, snd_range_factor);
	
	return 1;
}
int WE_Hyperspace::warpEnd()
{
	if (snd_start.isValid())
		snd_stop(snd_start);
	if(snd_end_gs != nullptr)
		snd_end = snd_play_3d(snd_end_gs, &objp->pos, &View_position, 0.0f, nullptr, 0, 1.0f, SND_PRIORITY_SINGLE_INSTANCE, nullptr, snd_range_factor);

	return WarpEffect::warpEnd();
}
