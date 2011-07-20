/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "weapon/beam.h"
#include "globalincs/linklist.h"
#include "object/object.h"
#include "object/objcollide.h"
#include "object/objectshield.h"
#include "ship/ship.h"
#include "freespace2/freespace.h"
#include "render/3d.h"
#include "io/timer.h"
#include "debris/debris.h"
#include "asteroid/asteroid.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "particle/particle.h"
#include "ship/shiphit.h"
#include "gamesnd/gamesnd.h"
#include "hud/hudmessage.h"
#include "lighting/lighting.h"
#include "hud/hudshield.h"
#include "playerman/player.h"
#include "weapon/weapon.h"
#include "parse/parselo.h"
#include "iff_defs/iff_defs.h"
#include "globalincs/globals.h"



extern int Cmdline_nohtl;
// ------------------------------------------------------------------------------------------------
// BEAM WEAPON DEFINES/VARS
//

// this is the constant which defines when a beam is an "area" beam. meaning, when we switch on sphereline checking and when 
// a beam gets "stopped" by an object. It is a percentage of the object radius which the beam must be wider than
#define BEAM_AREA_PERCENT			0.4f

// randomness factor - all beam weapon aiming is adjusted by +/- some factor within this range
#define BEAM_RANDOM_FACTOR			0.4f

#define BEAM_DAMAGE_TIME			170			// apply damage 
#define MAX_SHOT_POINTS				30
#define SHOT_POINT_TIME				200			// 5 arcs a second

#define TOOLTIME						1500.0f

beam Beams[MAX_BEAMS];				// all beams
beam Beam_free_list;					// free beams
beam Beam_used_list;					// used beams
int Beam_count = 0;					// how many beams are in use

// octant indices. These are "good" pairs of octants to use for beam target
#define BEAM_NUM_GOOD_OCTANTS			8
int Beam_good_slash_octants[BEAM_NUM_GOOD_OCTANTS][4] = {		
	{ 2, 5, 1, 0 },					// octant, octant, min/max pt, min/max pt
	{ 7, 0, 1, 0 },
	{ 1, 6, 1, 0 },					
	{ 6, 1, 0, 1 },
	{ 5, 2, 0, 1 },	
	{ 0, 7, 0, 1 },		
	{ 7, 1, 1, 0 },
	{ 6, 0, 1, 0 },
};
int Beam_good_shot_octants[BEAM_NUM_GOOD_OCTANTS][4] = {		
	{ 5, 0, 1, 0 },					// octant, octant, min/max pt, min/max pt
	{ 7, 2, 1, 0 },
	{ 7, 1, 1, 0 },					
	{ 6, 0, 1, 0 },
	{ 7, 3, 1, 0 },	
	{ 6, 2, 1, 0 },
	{ 5, 1, 1, 0 },
	{ 4, 0, 1, 0 },
};

// beam lighting effects
int Beam_lighting = 1;

// debug stuff - keep track of how many collision tests we perform a second and how many we toss a second
#define BEAM_TEST_STAMP_TIME		4000	// every 4 seconds
int Beam_test_stamp = -1;
int Beam_test_ints = 0;
int Beam_test_ship = 0;
int Beam_test_ast = 0;
int Beam_test_framecount = 0;

// beam warmup completion %
#define BEAM_WARMUP_PCT(b)			( ((float)Weapon_info[b->weapon_info_index].b_info.beam_warmup - (float)timestamp_until(b->warmup_stamp)) / (float)Weapon_info[b->weapon_info_index].b_info.beam_warmup ) 

// beam warmdown completion %		
#define BEAM_WARMDOWN_PCT(b)		( ((float)Weapon_info[b->weapon_info_index].b_info.beam_warmdown - (float)timestamp_until(b->warmdown_stamp)) / (float)Weapon_info[b->weapon_info_index].b_info.beam_warmdown ) 

// timestamp for spewing muzzle particles
//int Beam_muzzle_stamp = -1;

// link into the physics paused system
extern int physics_paused;

// beam lighting info
#define MAX_BEAM_LIGHT_INFO		100
typedef struct beam_light_info {
	beam *bm;					// beam casting the light
	int objnum;					// object getting light cast on it
	ubyte source;				// 0 to light the shooter, 1 for lighting any ship the beam passes, 2 to light the collision ship
	vec3d c_point;			// collision point for type 2 lights
} beam_light_info;

beam_light_info Beam_lights[MAX_BEAM_LIGHT_INFO];
int Beam_light_count = 0;

float b_whack_small = 2000.0f;	// used to be 500.0f with the retail whack bug
float b_whack_big = 10000.0f;	// used to be 1500.0f with the retail whack bug
float b_whack_damage = 150.0f;

DCF(b_whack_small, "")
{
	dc_get_arg(ARG_FLOAT);
	b_whack_small = Dc_arg_float;
}
DCF(b_whack_big, "")
{
	dc_get_arg(ARG_FLOAT);
	b_whack_big = Dc_arg_float;
}
DCF(b_whack_damage, "")
{
	dc_get_arg(ARG_FLOAT);
	b_whack_damage = Dc_arg_float;
}


// ------------------------------------------------------------------------------------------------
// BEAM WEAPON FORWARD DECLARATIONS
//

// delete a beam
void beam_delete(beam *b);

// handle a hit on a specific object
void beam_handle_collisions(beam *b);

// fills in binfo
void beam_get_binfo(beam *b, float accuracy, int num_shots);

// aim the beam (setup last_start and last_shot - the endpoints). also recalculates object collision info
void beam_aim(beam *b);

// type A functions
void beam_type_a_move(beam *b);

// type B functions
void beam_type_b_move(beam *b);

// type C functions
void beam_type_c_move(beam *b);

// type D functions
void beam_type_d_move(beam *b);
void beam_type_d_get_status(beam *b, int *shot_index, int *fire_wait);

// type e functions
void beam_type_e_move(beam *b);

// given a model #, and an object, stuff 2 good world coord points
void beam_get_octant_points(int modelnum, object *objp, int oct_index, int oct_array[BEAM_NUM_GOOD_OCTANTS][4], vec3d *v1, vec3d *v2);

// given an object, return its model num
int beam_get_model(object *objp);

// for a given object, and a firing beam, determine its critical dot product and range
void beam_get_cull_vals(object *objp, beam *b, float *cull_dot, float *cull_dist);

// get the total possible cone for a given beam in radians
float beam_get_cone_dot(beam *b);

// for rendering the beam effect
// output top and bottom vectors
// fvec == forward vector (eye viewpoint basically. in world coords)
// pos == world coordinate of the point we're calculating "around"
// w == width of the diff between top and bottom around pos
void beam_calc_facing_pts(vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w, float z_add);

// render the muzzle glow for a beam weapon
void beam_render_muzzle_glow(beam *b);

// generate particles for the muzzle glow
void beam_generate_muzzle_particles(beam *b);

// throw some jitter into the aim - based upon shot_aim
void beam_jitter_aim(beam *b, float aim);

// if it is legal for the beam to continue firing
// returns -1 if the beam should stop firing immediately
// returns 0 if the beam should go to warmdown
// returns 1 if the beam can continue along its way
int beam_ok_to_fire(beam *b);

// start the warmup phase for the beam
void beam_start_warmup(beam *b);

// start the firing phase for the beam, return 0 if the beam failed to start, and should be deleted altogether
int beam_start_firing(beam *b);

// start the warmdown phase for the beam
void beam_start_warmdown(beam *b);

// add a collision to the beam for this frame (to be evaluated later)
void beam_add_collision(beam *b, object *hit_object, mc_info *cinfo, int quad = -1, int exit_flag = 0);

// sort collisions for the frame
int beam_sort_collisions_func(const void *e1, const void *e2);

// get the width of the widest section of the beam
float beam_get_widest(beam *b);

// mark an object as being lit
void beam_add_light(beam *b, int objnum, int source, vec3d *c_point);

// apply lighting from any beams
void beam_apply_lighting();

// recalculate beam sounds (looping sounds relative to the player)
void beam_recalc_sounds(beam *b);

// apply a whack to a ship
void beam_apply_whack(beam *b, object *objp, vec3d *hit_point);

// return the amount of damage which should be applied to a ship. basically, filters friendly fire damage 
float beam_get_ship_damage(beam *b, object *objp);

// if the beam is likely to tool a given target before its lifetime expires
int beam_will_tool_target(beam *b, object *objp);

extern int Use_GLSL;

// ------------------------------------------------------------------------------------------------
// BEAM WEAPON FUNCTIONS
//

// init at game startup
void beam_init()
{
	// clear the beams
	list_init( &Beam_free_list );
	list_init( &Beam_used_list );
}

// initialize beam weapons for this level
void beam_level_init()
{
	// intialize beams
	int idx;

	Beam_count = 0;
	list_init( &Beam_free_list );
	list_init( &Beam_used_list );
	memset(Beams, 0, sizeof(beam) * MAX_BEAMS);

	// Link all object slots into the free list
	for (idx=0; idx<MAX_BEAMS; idx++)	{
		Beams[idx].objnum = -1;
		list_append(&Beam_free_list, &Beams[idx] );
	}

	// reset muzzle particle spew timestamp
}

// shutdown beam weapons for this level
void beam_level_close()
{
	// clear the beams
	list_init( &Beam_free_list );
	list_init( &Beam_used_list );
}

// fire a beam, returns nonzero on success. the innards of the code handle all the rest, foo
int beam_fire(beam_fire_info *fire_info)
{
	beam *new_item;
	weapon_info *wip;
	ship *firing_ship;
	int objnum;		

	// sanity check
	if(fire_info == NULL){
		Int3();
		return -1;
	}

	// if we're out of beams, bail
	if(Beam_count >= MAX_BEAMS){
		return -1;
	}

	// for now, only allow ship targets
	if (!fire_info->fighter_beam) {
		if((fire_info->target == NULL) || ((fire_info->target->type != OBJ_SHIP) && (fire_info->target->type != OBJ_ASTEROID) && (fire_info->target->type != OBJ_DEBRIS) && (fire_info->target->type != OBJ_WEAPON))){
			return -1;
		}
	}

	// make sure the beam_info_index is valid
	Assert((fire_info->beam_info_index >= 0) && (fire_info->beam_info_index < MAX_WEAPON_TYPES) && (Weapon_info[fire_info->beam_info_index].wi_flags & WIF_BEAM));
	if((fire_info->beam_info_index < 0) || (fire_info->beam_info_index >= MAX_WEAPON_TYPES) || !(Weapon_info[fire_info->beam_info_index].wi_flags & WIF_BEAM)){
		return -1;
	}

	wip = &Weapon_info[fire_info->beam_info_index];	
	// make sure a ship is firing this
	Assert((fire_info->shooter->type == OBJ_SHIP) && (fire_info->shooter->instance >= 0) && (fire_info->shooter->instance < MAX_SHIPS));
	if ( (fire_info->shooter->type != OBJ_SHIP) || (fire_info->shooter->instance < 0) || (fire_info->shooter->instance >= MAX_SHIPS) ) {
		return -1;
	}
	firing_ship = &Ships[fire_info->shooter->instance];

	// get a free beam
	new_item = GET_FIRST(&Beam_free_list);
	Assert( new_item != &Beam_free_list );		// shouldn't have the dummy element
	if(new_item == &Beam_free_list){
		return -1;
	}

	// make sure that our textures are loaded as well
	extern bool weapon_is_used(int weapon_index);
	extern void weapon_load_bitmaps(int weapon_index);
	if ( !weapon_is_used(fire_info->beam_info_index) ) {
		weapon_load_bitmaps(fire_info->beam_info_index);
	}

	// remove from the free list
	list_remove( &Beam_free_list, new_item );
	
	// insert onto the end of used list
	list_append( &Beam_used_list, new_item );

	// increment counter
	Beam_count++;	

	// fill in some values
	new_item->warmup_stamp = -1;
	new_item->warmdown_stamp = -1;
	new_item->weapon_info_index = fire_info->beam_info_index;	
	new_item->objp = fire_info->shooter;
	new_item->sig = fire_info->shooter->signature;
	new_item->subsys = fire_info->turret;	
	new_item->local_pnt = fire_info->turret->system_info->pnt;
	new_item->life_left = wip->b_info.beam_life;	
	new_item->life_total = wip->b_info.beam_life;
	new_item->r_collision_count = 0;
	new_item->f_collision_count = 0;
	new_item->target = fire_info->target;
	new_item->target_subsys = fire_info->target_subsys;
	new_item->target_sig = (fire_info->target != NULL) ? fire_info->target->signature : 0;
	new_item->beam_sound_loop = -1;
	new_item->type = wip->b_info.beam_type;
	new_item->targeting_laser_offset = fire_info->targeting_laser_offset;
	new_item->framecount = 0;
	new_item->flags = 0;
	new_item->shot_index = 0;
	new_item->shrink = 1.0f;	
	new_item->team = (char)firing_ship->team;
	new_item->range = wip->b_info.range;
	new_item->damage_threshold = wip->b_info.damage_threshold;
	new_item->fighter_beam = fire_info->fighter_beam;
	new_item->bank = fire_info->bank;
	new_item->Beam_muzzle_stamp = -1;
	new_item->beam_glow_frame = 0.0f;
	new_item->firingpoint = fire_info->turret->turret_next_fire_pos;
	new_item->beam_width = wip->b_info.beam_width;

	for (int i = 0; i < MAX_BEAM_SECTIONS; i++)
		new_item->beam_secion_frame[i] = 0.0f;
	
	if(fire_info->fighter_beam){
		new_item->type = BEAM_TYPE_C;
	}

	// if the targeted subsystem is not NULL, force it to be a type A beam
	if(new_item->target_subsys != NULL && new_item->type != BEAM_TYPE_C){
		new_item->type = BEAM_TYPE_A;
	}

	// type D weapons can only fire at small ships and missiles
	if(new_item->type == BEAM_TYPE_D){
		// if its a targeted ship, get the target ship
		if((fire_info->target != NULL) && (fire_info->target->type == OBJ_SHIP) && (fire_info->target->instance >= 0)){		
			ship *target_ship = &Ships[fire_info->target->instance];
			
			// maybe force to be a type A
			if(Ship_info[target_ship->ship_info_index].class_type > -1 && (Ship_types[Ship_info[target_ship->ship_info_index].class_type].weapon_bools & STI_WEAP_BEAMS_EASILY_HIT)){
				new_item->type = BEAM_TYPE_A;
			}
		}
	}
	
	// ----------------------------------------------------------------------
	// THIS IS THE CRITICAL POINT FOR MULTIPLAYER
	// beam_get_binfo(...) determines exactly how the beam will behave over the course of its life
	// it fills in binfo, which we can pass to clients in multiplayer	
	if(fire_info->beam_info_override != NULL){
		new_item->binfo = *fire_info->beam_info_override;
	} else {
		beam_get_binfo(new_item, fire_info->accuracy, wip->b_info.beam_shots);			// to fill in b_info	- the set of directional aim vectors
	}	

	// create the associated object
	objnum = obj_create(OBJ_BEAM, OBJ_INDEX(fire_info->shooter), new_item - Beams, &vmd_identity_matrix, &vmd_zero_vector, 1.0f, OF_COLLIDES);
	if(objnum < 0){
		beam_delete(new_item);
		nprintf(("General", "obj_create() failed for beam weapon! bah!\n"));
		Int3();
		return -1;
	}
	new_item->objnum = objnum;

	// this sets up all info for the first frame the beam fires
	beam_aim(new_item);						// to fill in shot_point, etc.	

	// check to see if its legal to fire at this guy
	if(beam_ok_to_fire(new_item) != 1){
		beam_delete(new_item);
		mprintf(("Killing beam at initial fire because of illegal targeting!!!\n"));
		return -1;
	}

	// if we're a multiplayer master - send a packet
	if (MULTIPLAYER_MASTER) {
		int bank_point = -1;

		if (fire_info->fighter_beam) {
			// magic numbers suck, be we need to make sure that we are always below UCHAR_MAX (255)
			Assert( fire_info->point <= 25 );
			Assert( fire_info->bank <= 5 );

			bank_point = (fire_info->point * 10) + fire_info->bank;
		}

		send_beam_fired_packet(fire_info->shooter, fire_info->turret, fire_info->target, fire_info->beam_info_index, &new_item->binfo, (ubyte)fire_info->fighter_beam, bank_point);
	}

	// start the warmup phase
	beam_start_warmup(new_item);		

	return objnum;
}

// fire a targeting beam, returns objnum on success. a much much simplified version of a beam weapon
// targeting lasers last _one_ frame. For a continuous stream - they must be created every frame.
// this allows it to work smoothly in multiplayer (detect "trigger down". every frame just create a targeting laser firing straight out of the
// object. this way you get all the advantages of nice rendering and collisions).
// NOTE : only references beam_info_index and shooter
int beam_fire_targeting(fighter_beam_fire_info *fire_info)
{
	beam *new_item;
	weapon_info *wip;
	int objnum;	
	ship *firing_ship;

	// sanity check
	if(fire_info == NULL){
		Int3();
		return -1;
	}

	// if we're out of beams, bail
	if(Beam_count >= MAX_BEAMS){
		return -1;
	}
	
	// make sure the beam_info_index is valid
	Assert((fire_info->beam_info_index >= 0) && (fire_info->beam_info_index < MAX_WEAPON_TYPES) && (Weapon_info[fire_info->beam_info_index].wi_flags & WIF_BEAM));
	if((fire_info->beam_info_index < 0) || (fire_info->beam_info_index >= MAX_WEAPON_TYPES) || !(Weapon_info[fire_info->beam_info_index].wi_flags & WIF_BEAM)){
		return -1;
	}
	wip = &Weapon_info[fire_info->beam_info_index];	

	// make sure a ship is firing this
	Assert((fire_info->shooter->type == OBJ_SHIP) && (fire_info->shooter->instance >= 0) && (fire_info->shooter->instance < MAX_SHIPS));
	if ( (fire_info->shooter->type != OBJ_SHIP) || (fire_info->shooter->instance < 0) || (fire_info->shooter->instance >= MAX_SHIPS) ) {
		return -1;
	}
	firing_ship = &Ships[fire_info->shooter->instance];


	// get a free beam
	new_item = GET_FIRST(&Beam_free_list);
	Assert( new_item != &Beam_free_list );		// shouldn't have the dummy element

	// remove from the free list
	list_remove( &Beam_free_list, new_item );
	
	// insert onto the end of used list
	list_append( &Beam_used_list, new_item );

	// increment counter
	Beam_count++;

	// maybe allocate some extra data based on the beam type
	Assert(wip->b_info.beam_type == BEAM_TYPE_C);
	if(wip->b_info.beam_type != BEAM_TYPE_C){
		return -1;
	}

	// fill in some values
	new_item->warmup_stamp = fire_info->warmup_stamp;
	new_item->warmdown_stamp = fire_info->warmdown_stamp;
	new_item->weapon_info_index = fire_info->beam_info_index;	
	new_item->objp = fire_info->shooter;
	new_item->sig = fire_info->shooter->signature;
	new_item->subsys = NULL;
	new_item->life_left = fire_info->life_left;	
	new_item->life_total = fire_info->life_total;
	new_item->r_collision_count = 0;
	new_item->f_collision_count = 0;
	new_item->target = NULL;
	new_item->target_subsys = NULL;
	new_item->target_sig = 0;	
	new_item->beam_sound_loop = -1;
	new_item->type = BEAM_TYPE_C;	
	new_item->targeting_laser_offset = fire_info->targeting_laser_offset;
	new_item->framecount = 0;
	new_item->flags = 0;
	new_item->shot_index = 0;
	new_item->shrink = 1.0f;	
	new_item->team = (char)firing_ship->team;
	new_item->range = wip->b_info.range;
	new_item->damage_threshold = wip->b_info.damage_threshold;
	new_item->beam_width = wip->b_info.beam_width;

	// type c is a very special weapon type - binfo has no meaning

	// create the associated object
	objnum = obj_create(OBJ_BEAM, OBJ_INDEX(fire_info->shooter), new_item - Beams, &vmd_identity_matrix, &vmd_zero_vector, 1.0f, OF_COLLIDES);

	if(objnum < 0){
		beam_delete(new_item);
		nprintf(("General", "obj_create() failed for beam weapon! bah!\n"));
		Int3();
		return -1;
	}
	new_item->objnum = objnum;	

	// this sets up all info for the first frame the beam fires
	beam_aim(new_item);						// to fill in shot_point, etc.		

	if(Beams[Objects[objnum].instance].objnum != objnum){
		Int3();
		return -1;
	}

//	Objects[objnum].instance = objnum
	
	return objnum;
}

// return an object index of the guy who's firing this beam
int beam_get_parent(object *bm)
{
	beam *b;

	// get a handle to the beam
	Assert(bm->type == OBJ_BEAM);
	Assert(bm->instance >= 0);	
	if(bm->type != OBJ_BEAM){
		return -1;
	}
	if(bm->instance < 0){
		return -1;
	}
	b = &Beams[bm->instance];

	Assert(b->objp != NULL);
	if(b->objp == NULL){
		return -1;
	}

	// if the object handle is invalid
	if(b->objp->signature != b->sig){
		return -1;
	}
 //comented out to see if this is the weak link in the fighter beam hit recording -Bobboau

	// return the handle
	return OBJ_INDEX(b->objp);
}

// return weapon_info_index of beam
int beam_get_weapon_info_index(object *bm)
{
	Assert(bm->type == OBJ_BEAM);
	if (bm->type != OBJ_BEAM) {
		return -1;
	}

	Assert(bm->instance >= 0 && bm->instance < MAX_BEAMS);
	if (bm->instance < 0) {
		return -1;
	}
//make sure it's returning a valid info index
	Assert((Beams[bm->instance].weapon_info_index > -1) && (Beams[bm->instance].weapon_info_index < Num_weapon_types));

	// return weapon_info_index
	return Beams[bm->instance].weapon_info_index;
}



// given a beam object, get the # of collisions which happened during the last collision check (typically, last frame)
int beam_get_num_collisions(int objnum)
{	
	// sanity checks
	if((objnum < 0) || (objnum >= MAX_OBJECTS)){
		Int3();
		return -1;
	}
	if((Objects[objnum].instance < 0) || (Objects[objnum].instance >= MAX_BEAMS)){
		Int3();
		return -1;
	}
	if(Beams[Objects[objnum].instance].objnum != objnum){
		Int3();
		return -1;
	}

	if(Beams[Objects[objnum].instance].objnum < 0){
		Int3();
		return -1;
	}

	// return the # of recent collisions
	return Beams[Objects[objnum].instance].r_collision_count;
}

// stuff collision info, returns 1 on success
int beam_get_collision(int objnum, int num, int *collision_objnum, mc_info **cinfo)
{
	// sanity checks
	if((objnum < 0) || (objnum >= MAX_OBJECTS)){
		Int3();
		return 0;
	}
	if((Objects[objnum].instance < 0) || (Objects[objnum].instance >= MAX_BEAMS)){
		Int3();
		return 0;
	}
	if((Beams[Objects[objnum].instance].objnum != objnum) || (Beams[Objects[objnum].instance].objnum < 0)){
		Int3();
		return 0;
	}
	if(num >= Beams[Objects[objnum].instance].r_collision_count){
		Int3();
		return 0;
	}

	// return - success
	*cinfo = &Beams[Objects[objnum].instance].r_collisions[num].cinfo;
	*collision_objnum = Beams[Objects[objnum].instance].r_collisions[num].c_objnum;
	return 1;
}

// pause all looping beam sounds
void beam_pause_sounds()
{
	beam *moveup = NULL;

	// set all beam volumes to 0	
	moveup = GET_FIRST(&Beam_used_list);
	if(moveup == NULL){
		return;
	}
	while(moveup != END_OF_LIST(&Beam_used_list)){				
		// set the volume to 0, if he has a looping beam sound
		if(moveup->beam_sound_loop >= 0){
			snd_set_volume(moveup->beam_sound_loop, 0.0f);
		}

		// next beam
		moveup = GET_NEXT(moveup);
	}
}

// unpause looping beam sounds
void beam_unpause_sounds()
{
	beam *moveup = NULL;

	// recalc all beam sounds
	moveup = GET_FIRST(&Beam_used_list);
	if(moveup == NULL){
		return;
	}
	while(moveup != END_OF_LIST(&Beam_used_list)){				
		beam_recalc_sounds(moveup);

		// next beam
		moveup = GET_NEXT(moveup);
	}
}

void beam_get_global_turret_gun_info(object *objp, ship_subsys *ssp, vec3d *gpos, vec3d *gvec, int use_angles, vec3d *targetp, bool fighter_beam){
		ship_get_global_turret_gun_info(objp, ssp, gpos, gvec, use_angles, targetp);
	if(fighter_beam)*gvec = objp->orient.vec.fvec;
}

// -----------------------------===========================------------------------------
// BEAM MOVEMENT FUNCTIONS
// -----------------------------===========================------------------------------

// move a type A beam weapon
void beam_type_a_move(beam *b)
{
	vec3d dir;
	vec3d temp, temp2;	

	// LEAVE THIS HERE OTHERWISE MUZZLE GLOWS DRAW INCORRECTLY WHEN WARMING UP OR DOWN
	// get the "originating point" of the beam for this frame. essentially bashes last_start
	beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &temp2, b->fighter_beam);

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return;
	}

	// put the "last_shot" point arbitrarily far away
	vm_vec_sub(&dir, &b->last_shot, &b->last_start);
	vm_vec_normalize_quick(&dir);
	vm_vec_scale_add(&b->last_shot, &b->last_start, &dir, b->range);
	Assert(is_valid_vec(&b->last_shot));
}

// move a type B beam weapon
#define BEAM_T(b)						( ((b->binfo.delta_ang / b->life_total) * (b->life_total - b->life_left)) / b->binfo.delta_ang )
void beam_type_b_move(beam *b)
{		
	vec3d actual_dir;
	vec3d temp, temp2;
	float dot_save;	

	// LEAVE THIS HERE OTHERWISE MUZZLE GLOWS DRAW INCORRECTLY WHEN WARMING UP OR DOWN
	// get the "originating point" of the beam for this frame. essentially bashes last_start
	beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &temp2, b->fighter_beam);

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return;
	}	

	// if the two direction vectors are _really_ close together, just use the original direction
	dot_save = vm_vec_dot(&b->binfo.dir_a, &b->binfo.dir_b);
	if((double)dot_save >= 0.999999999){
		actual_dir = b->binfo.dir_a;
	} 
	// otherwise move towards the dir	we calculated when firing this beam	
	else {
		vm_vec_interp_constant(&actual_dir, &b->binfo.dir_a, &b->binfo.dir_b, BEAM_T(b));
	}

	// now recalculate shot_point to be shooting through our new point
	vm_vec_scale_add(&b->last_shot, &b->last_start, &actual_dir, b->range);
	int is_valid = is_valid_vec(&b->last_shot);
	Assert(is_valid);
	if(!is_valid){
		actual_dir = b->binfo.dir_a;
		vm_vec_scale_add(&b->last_shot, &b->last_start, &actual_dir, b->range);
	}
}

// type C functions
void beam_type_c_move(beam *b)
{	
	vec3d temp;
	ship *shipp;
	int num_fire_points = 1;

	// ugh
	if ( (b->objp == NULL) || (b->objp->instance < 0) ) {
		Int3();
		return;
	}

	// type c beams only last one frame so we never have to "move" them.			
	temp = b->targeting_laser_offset;
	vm_vec_unrotate(&b->last_start, &temp, &b->objp->orient);
	vm_vec_add2(&b->last_start, &b->objp->pos);	
	vm_vec_scale_add(&b->last_shot, &b->last_start, &b->objp->orient.vec.fvec, b->range);

	shipp = &Ships[b->objp->instance];

	if (shipp->beam_sys_info.turret_num_firing_points > 1) {
		num_fire_points = shipp->beam_sys_info.turret_num_firing_points;
	}

	shipp->weapon_energy -= num_fire_points * Weapon_info[b->weapon_info_index].energy_consumed * flFrametime;

	if (shipp->weapon_energy < 0.0f) {
		shipp->weapon_energy = 0.0f;
	}
}

// type D functions
void beam_type_d_move(beam *b)
{
	int shot_index, fire_wait;
	vec3d temp, temp2, dir;	

	// LEAVE THIS HERE OTHERWISE MUZZLE GLOWS DRAW INCORRECTLY WHEN WARMING UP OR DOWN
	// get the "originating point" of the beam for this frame. essentially bashes last_start
	beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &temp2, b->fighter_beam);

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return;
	}	

	// determine what stage of the beam we're in
	beam_type_d_get_status(b, &shot_index, &fire_wait);

	// if we've changed shot index
	if(shot_index != b->shot_index){
		// set the new index
		b->shot_index = shot_index;

		// re-aim
		beam_aim(b);
	}

	// if we're in the fire wait stage
	b->flags &= ~BF_SAFETY;
	if(fire_wait){
		b->flags |= BF_SAFETY;
	}

	// put the "last_shot" point arbitrarily far away
	vm_vec_sub(&dir, &b->last_shot, &b->last_start);
	vm_vec_normalize_quick(&dir);
	vm_vec_scale_add(&b->last_shot, &b->last_start, &dir, b->range);
	Assert(is_valid_vec(&b->last_shot));
}
void beam_type_d_get_status(beam *b, int *shot_index, int *fire_wait)
{	
	float shot_time = b->life_total / (float)b->binfo.shot_count;
	float beam_time = b->life_total - b->life_left;

	// determine what "shot" we're on	
	*shot_index = (int)(beam_time / shot_time);
	
	if(*shot_index >= b->binfo.shot_count){
		nprintf(("Beam","Shot of type D beam had bad shot_index value\n"));
		*shot_index = b->binfo.shot_count - 1;
	}	

	// determine if its the firing or waiting section of the shot (fire happens first, THEN wait)	
	*fire_wait = 0;
	if(beam_time > ((shot_time * (*shot_index)) + (shot_time * 0.5f))){
		*fire_wait = 1;
	} 	
}

// type e functions
void beam_type_e_move(beam *b)
{
	vec3d temp, turret_norm;	

	// LEAVE THIS HERE OTHERWISE MUZZLE GLOWS DRAW INCORRECTLY WHEN WARMING UP OR DOWN
	// get the "originating point" of the beam for this frame. essentially bashes last_start
	beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &turret_norm, 1, &temp, b->fighter_beam);

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return;
	}	

	// put the "last_shot" point arbitrarily far away
	vm_vec_scale_add(&b->last_shot, &b->last_start, &turret_norm, b->range);	
	Assert(is_valid_vec(&b->last_shot));
}

// pre-move (before collision checking - but AFTER ALL OTHER OBJECTS HAVE BEEN MOVED)
void beam_move_all_pre()
{	
	beam *b;	
	beam *moveup;		

	// zero lights for this frame yet
	Beam_light_count = 0;

	// traverse through all active beams
	moveup = GET_FIRST(&Beam_used_list);
	while (moveup != END_OF_LIST(&Beam_used_list)) {				
		// get the beam
		b = moveup;

		// check if parent object has died, if so then delete beam
		if (b->objp->type == OBJ_NONE) {
			// set next beam
			moveup = GET_NEXT(moveup);
			// delete current beam
			beam_delete(b);

			continue;
		}

		// unset collision info
		b->f_collision_count = 0;

		if ( !physics_paused ) {
			// make sure to check that firingpoint is still properly set
			int temp = b->subsys->turret_next_fire_pos;

			if (b->fighter_beam == false)
				b->subsys->turret_next_fire_pos = b->firingpoint;

			// move the beam
			switch (b->type)
			{
				// type A beam weapons don't move
				case BEAM_TYPE_A :			
					beam_type_a_move(b);
					break;

				// type B beam weapons move across the target somewhat randomly
				case BEAM_TYPE_B :
					beam_type_b_move(b);
					break;				

				// type C beam weapons are attached to a fighter - pointing forward
				case BEAM_TYPE_C:
					beam_type_c_move(b);
					break;

				// type D
				case BEAM_TYPE_D:
					beam_type_d_move(b);
					break;

				// type E
				case BEAM_TYPE_E:
					beam_type_e_move(b);
					break;

				// illegal beam type
				default :
					Int3();
			}
			b->subsys->turret_next_fire_pos = temp;
		}

		// next
		moveup = GET_NEXT(moveup);
	}
}

// post-collision time processing for beams
void beam_move_all_post()
{
	beam *moveup;	
	beam *next_one;	
	int bf_status;	
	beam_weapon_info *bwi;

	// traverse through all active beams
	moveup = GET_FIRST(&Beam_used_list);
	while(moveup != END_OF_LIST(&Beam_used_list)){				
		 bwi = &Weapon_info[moveup->weapon_info_index].b_info;

//mprintf(("moveing beam with weapon info index %d, post\n", moveup->weapon_info_index));

		 // check the status of the beam
		bf_status = beam_ok_to_fire(moveup);

		// if we're warming up
		if(moveup->warmup_stamp != -1){
			next_one = GET_NEXT(moveup);			

			// should we be stopping?
			if(bf_status < 0){
//				mprintf(("killing beam becase it isn't ok to be fireing\n"));
				beam_delete(moveup);
			} else {
				// add a muzzle light for the shooter
				beam_add_light(moveup, OBJ_INDEX(moveup->objp), 0, NULL);

				// if the warming up timestamp has expired, start firing
				if(timestamp_elapsed(moveup->warmup_stamp)){							
					// start firing
					if(!beam_start_firing(moveup)){
//						mprintf(("killing beam becase it shouldn't have started fireing yet\n"));
						beam_delete(moveup);												
					} 			
				} 
			}

			// next
//			mprintf(("beam is warming up, moveing to next\n"));
			moveup = next_one;
			continue;
		} 
		// if we're warming down
		else if(moveup->warmdown_stamp != -1){			
			next_one = GET_NEXT(moveup);

			// should we be stopping?
			if(bf_status < 0){
//				mprintf(("killing beam becase it isn't ok to fire\n"));
				beam_delete(moveup);
			} else {
				// add a muzzle light for the shooter
				beam_add_light(moveup, OBJ_INDEX(moveup->objp), 0, NULL);

				// if we're done warming down, the beam is finished
				if(timestamp_elapsed(moveup->warmdown_stamp)){	
//					mprintf(("euthaniseing beam\n"));
					beam_delete(moveup);				
				}			
			}

			// next
//			mprintf(("beam is warming down, moveing to next\n"));
			moveup = next_one;
			continue;
		}
//		mprintf(("beam is fireing\n"));
		// otherwise, we're firing away.........		

		// add a muzzle light for the shooter
		beam_add_light(moveup, OBJ_INDEX(moveup->objp), 0, NULL);

		// subtract out the life left for the beam
		if(!physics_paused){
			moveup->life_left -= flFrametime;						
		}

		// if we're past the shrink point, start shrinking the beam
		if(moveup->life_left <= (moveup->life_total * bwi->beam_shrink_factor)){
			moveup->flags |= BF_SHRINK;
		}

		// if we're shrinking the beam
		if(moveup->flags & BF_SHRINK){
			moveup->shrink -= bwi->beam_shrink_pct * flFrametime;
			if(moveup->shrink < 0.1f){
				moveup->shrink = 0.1f;
			}
		}		

		// add tube light for the beam
		if(Use_GLSL > 1)
			beam_add_light(moveup, OBJ_INDEX(moveup->objp), 1, NULL);

		// stop shooting?
		if(bf_status <= 0){
			next_one = GET_NEXT(moveup);

			// if beam should abruptly stop
			if(bf_status == -1){
//				mprintf(("beam stoping abruptly\n"));
				beam_delete(moveup);							
			}
			// if the beam should just power down
			else {			
				beam_start_warmdown(moveup);
			}
			
			// next beam
			moveup = next_one;
//			mprintf(("beam stopping\n"));
			continue;
		}				

		// increment framecount
		moveup->framecount++;		
//		mprintf(("frame %d\n", moveup->framecount));
		// type c weapons live for one frame only
/*		if(moveup->type == BEAM_TYPE_C){
			if(moveup->framecount > 1){
				next_one = GET_NEXT(moveup);
				beam_delete(moveup);							
//			mprintf(("type c beams only live for one frame\n"));
				moveup = next_one;
				continue;
			}
		}
		// done firing, so go into the warmdown phase
		else*/ {
			if((moveup->life_left <= 0.0f) &&
               (moveup->warmdown_stamp == -1) &&
               (moveup->framecount > 1))
            {
				beam_start_warmdown(moveup);
				
				moveup = GET_NEXT(moveup);	
//				mprintf(("warming beam down\n"));
				continue;
			}				
		}	
//		mprintf(("starting collisions\n"));

		// handle any collisions which occured collision (will take care of applying damage to all objects which got hit)
		beam_handle_collisions(moveup);						

//		mprintf(("recalcing sounds\n"));
		// recalculate beam sounds
		beam_recalc_sounds(moveup);

		// next item
		moveup = GET_NEXT(moveup);
//		mprintf(("moved, getting next\n"));
	}

	// apply all beam lighting
//	mprintf(("applying light\n"));
	beam_apply_lighting();

	// process beam culling info
#ifndef NDEBUG
	/*
	if(Beam_test_stamp == -1){
		Beam_test_stamp = timestamp(BEAM_TEST_STAMP_TIME);
		Beam_test_ints = 0;
		Beam_test_framecount = 0;
	} else {
		if(timestamp_elapsed(Beam_test_stamp)){			
			// report the results
			nprintf(("General", "Performed %f beam ints/frame (%d, %d, %d, %d), over %f seconds\n", (float)Beam_test_ints/(float)Beam_test_framecount, Beam_test_ints, Beam_test_framecount, Beam_test_ship, Beam_test_ast, (float)BEAM_TEST_STAMP_TIME / 1000.0f));

			// reset vars
			Beam_test_stamp = timestamp(BEAM_TEST_STAMP_TIME);
			Beam_test_ints = 0;
			Beam_test_ship = 0;
			Beam_test_ast = 0;
			Beam_test_framecount = 0;
		} else {
			Beam_test_framecount++;
		}
	}
	*/
#endif
//	mprintf(("done beam_move_all_post\n"));
}

// -----------------------------===========================------------------------------
// BEAM RENDERING FUNCTIONS
// -----------------------------===========================------------------------------

// render a beam weapon
#define STUFF_VERTICES()	do { verts[0]->u = 0.0f; verts[0]->v = 0.0f;	verts[1]->u = 1.0f; verts[1]->v = 0.0f; verts[2]->u = 1.0f;	verts[2]->v = 1.0f; verts[3]->u = 0.0f; verts[3]->v = 1.0f; } while(0);
#define P_VERTICES()		do { for(idx=0; idx<4; idx++){ g3_project_vertex(verts[idx]); } } while(0);

void beam_render(beam *b, float u_offset)
{	
	int idx, s_idx;
	vertex h1[4];				// halves of a beam section
	vertex *verts[4] = { &h1[0], &h1[1], &h1[2], &h1[3] };
	vec3d fvec, top1, bottom1, top2, bottom2;
	float scale;
	float u_scale;	// beam tileing -Bobboau
	float length;	// beam tileing -Bobboau
	beam_weapon_section_info *bwsi;
	beam_weapon_info *bwi;

	memset( h1, 0, sizeof(vertex) * 4 );

	// bogus weapon info index
	if ( (b == NULL) || (b->weapon_info_index < 0) )
		return;

	// if the beam start and endpoints are the same
	if ( vm_vec_same(&b->last_start, &b->last_shot) )
		return;

	// get beam direction
	vm_vec_sub(&fvec, &b->last_shot, &b->last_start);
	vm_vec_normalize_quick(&fvec);		

	// turn off backface culling
	int cull = gr_set_cull(0);

	bwi = &Weapon_info[b->weapon_info_index].b_info;

	// draw all sections	
	for (s_idx = 0; s_idx < bwi->beam_num_sections; s_idx++) {
		bwsi = &bwi->sections[s_idx];

		if ( (bwsi->texture.first_frame < 0) || (bwsi->width <= 0.0f) )
			continue;

		// calculate the beam points
		scale = frand_range(1.0f - bwsi->flicker, 1.0f + bwsi->flicker);
		beam_calc_facing_pts(&top1, &bottom1, &fvec, &b->last_start, bwsi->width * scale * b->shrink, bwsi->z_add);	
		beam_calc_facing_pts(&top2, &bottom2, &fvec, &b->last_shot, bwsi->width * scale * scale * b->shrink, bwsi->z_add);

		if (Cmdline_nohtl) {
			g3_rotate_vertex(verts[0], &bottom1); 
			g3_rotate_vertex(verts[1], &bottom2);	
			g3_rotate_vertex(verts[2], &top2); 
			g3_rotate_vertex(verts[3], &top1);
		} else {
			g3_transfer_vertex(verts[0], &bottom1); 
			g3_transfer_vertex(verts[1], &bottom2);	
			g3_transfer_vertex(verts[2], &top2); 
			g3_transfer_vertex(verts[3], &top1);
		}

		P_VERTICES();						
		STUFF_VERTICES();		// stuff the beam with creamy goodness (texture coords)

		length = vm_vec_dist(&b->last_start, &b->last_shot);					// beam tileing -Bobboau
		
		if (bwsi->tile_type == 1)
			u_scale = length / (bwsi->width /2) / bwsi->tile_factor;	// beam tileing, might make a tileing factor in beam index later -Bobboau
		else
			u_scale = bwsi->tile_factor;

		verts[1]->u = (u_scale + (u_offset * bwsi->translation));				// beam tileing -Bobboau
		verts[2]->u = (u_scale + (u_offset * bwsi->translation));				// beam tileing -Bobboau
		verts[3]->u = (0 + (u_offset * bwsi->translation));
		verts[0]->u = (0 + (u_offset * bwsi->translation));

		float per = 1.0f;
		if (bwi->range)
			per -= length / bwi->range;

		//this should never happen but, just to be safe
		CLAMP(per, 0.0f, 1.0f);

		verts[1]->r = (ubyte)(255 * per);
		verts[2]->r = (ubyte)(255 * per);
		verts[1]->g = (ubyte)(255 * per);
		verts[2]->g = (ubyte)(255 * per);
		verts[1]->b = (ubyte)(255 * per);
		verts[2]->b = (ubyte)(255 * per);
		verts[1]->a = (ubyte)(255 * per);
		verts[2]->a = (ubyte)(255 * per);

		verts[0]->r = 255;
		verts[3]->r = 255;
		verts[0]->g = 255;
		verts[3]->g = 255;
		verts[0]->b = 255;
		verts[3]->b = 255;
		verts[0]->a = 255;
		verts[3]->a = 255;

		// set the right texture with additive alpha, and draw the poly
		int framenum = 0;

		if (bwsi->texture.num_frames > 1) {
			b->beam_secion_frame[s_idx] += flFrametime;

			// Sanity checks
			if (b->beam_secion_frame[s_idx] < 0.0f)
				b->beam_secion_frame[s_idx] = 0.0f;
			if (b->beam_secion_frame[s_idx] > 100.0f)
				b->beam_secion_frame[s_idx] = 0.0f;

			while (b->beam_secion_frame[s_idx] > bwsi->texture.total_time)
				b->beam_secion_frame[s_idx] -= bwsi->texture.total_time;

			framenum = fl2i( (b->beam_secion_frame[s_idx] * bwsi->texture.num_frames) / bwsi->texture.total_time );

			CLAMP(framenum, 0, bwsi->texture.num_frames-1);
		}

		gr_set_bitmap(bwsi->texture.first_frame + framenum, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.9999f);

		// added TMAP_FLAG_TILED flag for beam texture tileing -Bobboau			
		// added TMAP_FLAG_RGB and TMAP_FLAG_GOURAUD so the beam would apear to fade along it's length-Bobboau
		g3_draw_poly( 4, verts, TMAP_FLAG_TEXTURED | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_FLAG_TILED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT ); 
	}		
	
	// turn backface culling back on
	gr_set_cull(cull);
}

// generate particles for the muzzle glow
int hack_time = 100;
DCF(h_time, "")
{
	dc_get_arg(ARG_INT);
	hack_time = Dc_arg_int;
}

void beam_generate_muzzle_particles(beam *b)
{
	int particle_count;
	int idx;
	weapon_info *wip;
	vec3d turret_norm, turret_pos, particle_pos, particle_dir, p_temp;
	matrix m;
	particle_info pinfo;

	// if our hack stamp has expired
	if(!((b->Beam_muzzle_stamp == -1) || timestamp_elapsed(b->Beam_muzzle_stamp))){
		return;
	}

	// never generate anything past about 1/5 of the beam fire time	
	if(b->warmup_stamp == -1){
		return;
	}

	// get weapon info
	wip = &Weapon_info[b->weapon_info_index];

	// no specified particle for this beam weapon
	if (wip->b_info.beam_particle_ani.first_frame < 0)
		return;

	
	// reset the hack stamp
	b->Beam_muzzle_stamp = timestamp(hack_time);

	// randomly generate 10 to 20 particles
	particle_count = (int)frand_range(0.0f, (float)wip->b_info.beam_particle_count);

	// get turret info - position and normal	
//	turret_pos = b->last_start;
//	vm_vec_sub(&turret_norm, &b->last_start,&b->last_shot);
//	vm_vec_normalize(&turret_norm);

	//turret_pos  = b->subsys->system_info->turret_firing_point[b->subsys->turret_next_fire_pos % b->subsys->system_info->turret_num_firing_points];
//	turret_pos = b->subsys->system_info->pnt;
	turret_pos = b->local_pnt;
	turret_norm = b->subsys->system_info->turret_norm;	

	// randomly perturb a vector within a cone around the normal
	vm_vector_2_matrix(&m, &turret_norm, NULL, NULL);
	for(idx=0; idx<particle_count; idx++){
		// get a random point in the cone
		vm_vec_random_cone(&particle_dir, &turret_norm, wip->b_info.beam_particle_angle, &m);
		p_temp = turret_pos;
		vm_vec_scale_add(&p_temp, &turret_pos, &particle_dir, wip->b_info.beam_muzzle_radius * frand_range(0.75f, 0.9f));

		// transform into world coords		
		vm_vec_unrotate(&particle_pos, &p_temp, &b->objp->orient);
		vm_vec_add2(&particle_pos, &b->objp->pos);
		p_temp = particle_dir;
		vm_vec_unrotate(&particle_dir, &p_temp, &b->objp->orient);

		// now generate some interesting values for the particle
		float p_time_ref = wip->b_info.beam_life + ((float)wip->b_info.beam_warmup / 1000.0f);		
		float p_life = frand_range(p_time_ref * 0.5f, p_time_ref * 0.7f);
		float p_vel = (wip->b_info.beam_muzzle_radius / p_life) * frand_range(0.85f, 1.2f);
		vm_vec_scale(&particle_dir, -p_vel);
		vm_vec_add2(&particle_dir, &b->objp->phys_info.vel);	//move along with our parent

		memset(&pinfo, 0, sizeof(particle_info));
		pinfo.pos = particle_pos;
		pinfo.vel = particle_dir;
		pinfo.lifetime = p_life;
		pinfo.attached_objnum = -1;
		pinfo.attached_sig = 0;
		pinfo.rad = wip->b_info.beam_particle_radius;
		pinfo.reverse = 1;
		pinfo.type = PARTICLE_BITMAP;
		pinfo.optional_data = wip->b_info.beam_particle_ani.first_frame;
		pinfo.tracer_length = -1.0f;		
		particle_create(&pinfo);
	}
}

// render the muzzle glow for a beam weapon
void beam_render_muzzle_glow(beam *b)
{
	vertex pt;
	weapon_info *wip = &Weapon_info[b->weapon_info_index];
	beam_weapon_info *bwi = &Weapon_info[b->weapon_info_index].b_info;
	float rad, pct, rand_val;
	int tmap_flags = TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT;

	// if we don't have a glow bitmap
	if (bwi->beam_glow.first_frame < 0)
		return;

	// if the beam is warming up, scale the glow
	if (b->warmup_stamp != -1) {		
		// get warmup pct
		pct = BEAM_WARMUP_PCT(b);
		rand_val = 1.0f;
	} else
	// if the beam is warming down
	if (b->warmdown_stamp != -1) {
		// get warmup pct
		pct = 1.0f - BEAM_WARMDOWN_PCT(b);
		rand_val = 1.0f;
	} 
	// otherwise the beam is really firing
	else {
		pct = 1.0f;
		rand_val = frand_range(0.90f, 1.0f);
	}

	rad = wip->b_info.beam_muzzle_radius * pct * rand_val;

	// don't bother trying to draw if there is no radius
	if (rad <= 0.0f)
		return;

	// draw the bitmap
	if (Cmdline_nohtl)
		g3_rotate_vertex(&pt, &b->last_start);
	else
		g3_transfer_vertex(&pt, &b->last_start);


	int framenum = 0;

	if (bwi->beam_glow.num_frames > 1) {
		b->beam_glow_frame += flFrametime;

		// Sanity checks
		if (b->beam_glow_frame < 0.0f)
			b->beam_glow_frame = 0.0f;
		if (b->beam_glow_frame > 100.0f)
			b->beam_glow_frame = 0.0f;

		while (b->beam_glow_frame > bwi->beam_glow.total_time)
			b->beam_glow_frame -= bwi->beam_glow.total_time;

		framenum = fl2i( (b->beam_glow_frame * bwi->beam_glow.num_frames) / bwi->beam_glow.total_time );

		CLAMP(framenum, 0, bwi->beam_glow.num_frames-1);
	}

	gr_set_bitmap(bwi->beam_glow.first_frame + framenum, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.8f * pct);

	// draw 1 bitmap
	g3_draw_bitmap(&pt, 0, rad, tmap_flags);
	
	// maybe draw more
	if (pct > 0.3f)
		g3_draw_bitmap(&pt, 0, rad * 0.75f, tmap_flags, rad * 0.25f);

	if (pct > 0.5f)
		g3_draw_bitmap(&pt, 0, rad * 0.45f, tmap_flags, rad * 0.55f);

	if (pct > 0.7f)
		g3_draw_bitmap(&pt, 0, rad * 0.25f, tmap_flags, rad * 0.75f);
}

// render all beam weapons
void beam_render_all()
{
	beam *moveup;	

	// moves the U value of texture coods in beams if desired-Bobboau
	static float u_offset = 0.0f;
	u_offset += flFrametime;

	//don't wrap since it causes the beam to jump	
//	if(u_offset > 1.0f){
//		u_offset = u_offset - 1.0f;	//keeps it below 1.0-Bobboau
//	}


	// traverse through all active beams
	moveup = GET_FIRST(&Beam_used_list);
	while ( moveup != END_OF_LIST(&Beam_used_list) ) {				
		// each beam type renders a little bit differently
		if ( (moveup->warmup_stamp == -1) && (moveup->warmdown_stamp == -1) && !(moveup->flags & BF_SAFETY) ) {
			// HACK -  if this is the first frame the beam is firing, don't render it
            if (moveup->framecount <= 0) {
				moveup = GET_NEXT(moveup);
				continue;
			}			

			// render the beam itself
			Assert(moveup->weapon_info_index >= 0);

			if (moveup->weapon_info_index < 0) {
				moveup = GET_NEXT(moveup);
				continue;
			}

			beam_render(moveup, u_offset);
		}

		// render the muzzle glow
		beam_render_muzzle_glow(moveup);		

		// maybe generate some muzzle particles
		beam_generate_muzzle_particles(moveup);

		// next item
		moveup = GET_NEXT(moveup);
	}	
}

// output top and bottom vectors
// fvec == forward vector (eye viewpoint basically. in world coords)
// pos == world coordinate of the point we're calculating "around"
// w == width of the diff between top and bottom around pos
void beam_calc_facing_pts( vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w, float z_add )
{
	vec3d uvec, rvec;
	vec3d temp;

	temp = *pos;

	vm_vec_sub( &rvec, &Eye_position, &temp );
	vm_vec_normalize( &rvec );	

	vm_vec_crossprod(&uvec,fvec,&rvec);
	// VECMAT-ERROR: NULL VEC3D (value of, fvec == rvec)
	vm_vec_normalize_safe(&uvec);

	vm_vec_scale_add( top, &temp, &uvec, w/2.0f );
	vm_vec_scale_add( bot, &temp, &uvec, -w/2.0f );	
}

// light scale factor
float blight = 25.5f;
DCF(blight, "")
{
	dc_get_arg(ARG_FLOAT);
	blight = Dc_arg_float;
}

// call to add a light source to a small object
void beam_add_light_small(beam *bm, object *objp, vec3d *pt_override = NULL)
{
	weapon_info *wip;
	beam_weapon_info *bwi;
	float noise;

	// no lighting 
	if(!Beam_lighting){
		return;
	}

	// sanity
	Assert(bm != NULL);
	if(bm == NULL){
		return;
	}
	Assert(objp != NULL);
	if(objp == NULL){
		return;
	}
	Assert(bm->weapon_info_index >= 0);
	wip = &Weapon_info[bm->weapon_info_index];
	bwi = &wip->b_info;

	// some noise
	if ( (bm->warmup_stamp < 0) && (bm->warmdown_stamp < 0) ) // disable noise when warming up or down
		noise = frand_range(1.0f - bwi->sections[0].flicker, 1.0f + bwi->sections[0].flicker);
	else
		noise = 1.0f;

	// widest part of the beam
	float light_rad = beam_get_widest(bm) * blight * noise;	

	// nearest point on the beam, and its distance to the ship
	vec3d near_pt;
	if(pt_override == NULL){
		float dist;
		vm_vec_dist_to_line(&objp->pos, &bm->last_start, &bm->last_shot, &near_pt, &dist);
		if(dist > light_rad){
			return;
		}
	} else {
		near_pt = *pt_override;
	}

	// average rgb of the beam	
	float fr = (float)wip->laser_color_1.red / 255.0f;
	float fg = (float)wip->laser_color_1.green / 255.0f;
	float fb = (float)wip->laser_color_1.blue / 255.0f;

	float pct = 0.0f;

	if (bm->warmup_stamp != -1) {	// calculate muzzle light intensity
		// get warmup pct
		pct = BEAM_WARMUP_PCT(bm)*0.5f;
	} else
	// if the beam is warming down
	if (bm->warmdown_stamp != -1) {
		// get warmup pct
		pct = MAX(1.0f - BEAM_WARMDOWN_PCT(bm)*1.3f,0.0f)*0.5f;
	} 
	// otherwise the beam is really firing
	else {
		pct = 1.0f;
	}
	// add a unique light
	// noise *= 0.1f;			// a little less noise here, since we want the beam to generally cast a bright light
	light_add_point_unique(&near_pt, light_rad * 0.0001f, light_rad, pct, fr, fg, fb, OBJ_INDEX(objp));
}

// call to add a light source to a large object
void beam_add_light_large(beam *bm, object *objp, vec3d *pt0, vec3d *pt1)
{
	weapon_info *wip;
	beam_weapon_info *bwi;
	float noise;

	// no lighting 
	if(!Beam_lighting){
		return;
	}

	// sanity
	Assert(bm != NULL);
	if(bm == NULL){
		return;
	}
	Assert(objp != NULL);
	if(objp == NULL){
		return;
	}
	Assert(bm->weapon_info_index >= 0);
	wip = &Weapon_info[bm->weapon_info_index];
	bwi = &wip->b_info;

	// some noise
	noise = frand_range(1.0f - bwi->sections[0].flicker, 1.0f + bwi->sections[0].flicker);

	// widest part of the beam
	float light_rad = beam_get_widest(bm) * blight * noise;		

	// average rgb of the beam	
	float fr = (float)wip->laser_color_1.red / 255.0f;
	float fg = (float)wip->laser_color_1.green / 255.0f;
	float fb = (float)wip->laser_color_1.blue / 255.0f;

	if ( Use_GLSL > 1 )
		light_add_tube(pt0, pt1, 1.0f, light_rad, 1.0f * noise, fr, fg, fb, OBJ_INDEX(objp)); 
	else {
		vec3d near_pt, a;
		float dist,max_dist;
		vm_vec_sub(&a, pt1, pt0);
		vm_vec_normalize_quick(&a);
		vm_vec_dist_squared_to_line(&objp->pos, pt0, pt1, &near_pt, &dist); // Calculate nearest point for fallback fake tube pointlight
		max_dist = light_rad + objp->radius;
		max_dist *= max_dist;
		if ( dist > max_dist)
			return; // Too far away
		light_add_tube(pt0, &near_pt, 1.0f, light_rad, 1.0f * noise, fr, fg, fb, OBJ_INDEX(objp));
	}
}

// mark an object as being lit
void beam_add_light(beam *b, int objnum, int source, vec3d *c_point)
{
	beam_light_info *l;

	// if we're out of light slots!
	if(Beam_light_count >= MAX_BEAM_LIGHT_INFO){
		// Int3();
		return;
	}

	// otherwise add it
	l = &Beam_lights[Beam_light_count++];
	l->bm = b;
	l->objnum = objnum;
	l->source = (ubyte)source;
	
	// only type 2 lights (from collisions) need a collision point
	if(c_point != NULL){
		l->c_point = *c_point;
	} else {
		Assert(source != 2);
		if(source == 2){
			Beam_light_count--;
		}
	}
}

// apply lighting from any beams
void beam_apply_lighting()
{
	int idx;
	beam_light_info *l;
	vec3d pt, dir;
	beam_weapon_info *bwi;

	// convert all beam lights into real lights
	for(idx=0; idx<Beam_light_count; idx++){
		// get the light
		l = &Beam_lights[idx];		

		// bad object
		if((l->objnum < 0) || (l->objnum >= MAX_OBJECTS) || (l->bm == NULL)){
			continue;
		}

		bwi = &Weapon_info[l->bm->weapon_info_index].b_info;

		// different light types
		switch(l->source){
		// from the muzzle of the gun
		case 0:
			// a few meters in from the of muzzle			
			vm_vec_sub(&dir, &l->bm->last_start, &l->bm->last_shot);
			vm_vec_normalize_quick(&dir);
			vm_vec_scale(&dir, -0.8f);  // TODO: This probably needs to *not* be stupid. -taylor
			vm_vec_scale_add(&pt, &l->bm->last_start, &dir, bwi->beam_muzzle_radius * 5.0f);

			beam_add_light_small(l->bm, &Objects[l->objnum], &pt);
			break;

		// from the beam passing by
		case 1:
			Assert( Objects[l->objnum].instance >= 0 );
			// Valathil: Everyone gets tube lights now
			beam_add_light_large(l->bm, &Objects[l->objnum], &l->bm->last_start, &l->bm->last_shot);
			break;

		// from a collision
		case 2:
			// Valathil: Dont render impact lights for shaders, handled by tube lighting
			if ( Use_GLSL > 1 ) {
				break;
			}
			// a few meters from the collision point			
			vm_vec_sub(&dir, &l->bm->last_start, &l->c_point);
			vm_vec_normalize_quick(&dir);
			vm_vec_scale_add(&pt, &l->c_point, &dir, bwi->beam_muzzle_radius * 5.0f);

			beam_add_light_small(l->bm, &Objects[l->objnum], &pt);
			break;
		}
	}	
}

// -----------------------------===========================------------------------------
// BEAM BOOKKEEPING FUNCTIONS
// -----------------------------===========================------------------------------

// delete a beam
void beam_delete(beam *b)
{
	// remove from active list and put on free list
	list_remove(&Beam_used_list, b);
	list_append(&Beam_free_list, b);

	// delete our associated object
	// Assert(b->objnum >= 0);
	if(b->objnum >= 0){
		obj_delete(b->objnum);
	}
	b->objnum = -1;

	// kill the beam looping sound
	if(b->beam_sound_loop != -1){
		snd_stop(b->beam_sound_loop);
		b->beam_sound_loop = -1;
	}	

	// handle model animation reversal (closing)
	// (beam animations should end pretty much immediately - taylor)
    if ((b->subsys) &&
        (b->subsys->turret_animation_position == MA_POS_READY))
    {
        b->subsys->turret_animation_done_time = timestamp(50);
    }

	// subtract one
	Beam_count--;
	Assert(Beam_count >= 0);
	nprintf(("Beam", "Recycled beam (%d beams remaining)\n", Beam_count));
}

// given an object, return its model num
int beam_get_model(object *objp)
{
	int pof;

	if (objp == NULL) {
		return -1;
	}

	Assert(objp->instance >= 0);
	if(objp->instance < 0){
		return -1;
	}

	switch(objp->type){
	case OBJ_SHIP:		
		return Ship_info[Ships[objp->instance].ship_info_index].model_num;

	case OBJ_WEAPON:
		Assert(Weapons[objp->instance].weapon_info_index >= 0);
		if(Weapons[objp->instance].weapon_info_index < 0){
			return -1;
		}
		return Weapon_info[Weapons[objp->instance].weapon_info_index].model_num;

	case OBJ_DEBRIS:
		Assert(Debris[objp->instance].is_hull);
		if(!Debris[objp->instance].is_hull){
			return -1;
		}
		return Debris[objp->instance].model_num;		

	case OBJ_ASTEROID:
		pof = Asteroids[objp->instance].asteroid_subtype;
		Assert(Asteroids[objp->instance].asteroid_type >= 0);
		if(Asteroids[objp->instance].asteroid_type < 0){
			return -1;
		}
		return Asteroid_info[Asteroids[objp->instance].asteroid_type].model_num[pof];

	default:
		// this shouldn't happen too often
		mprintf(("Beam couldn't find a good object model/type!! (%d)\n", objp->type));
		return -1;
	}
}

// start the warmup phase for the beam
void beam_start_warmup(beam *b)
{
	// set the warmup stamp
	b->warmup_stamp = timestamp(Weapon_info[b->weapon_info_index].b_info.beam_warmup);

	// start playing warmup sound
	if(!(Game_mode & GM_STANDALONE_SERVER) && (Weapon_info[b->weapon_info_index].b_info.beam_warmup_sound >= 0)){		
		snd_play_3d(&Snds[Weapon_info[b->weapon_info_index].b_info.beam_warmup_sound], &b->last_start, &View_position);
	}
}

// start the firing phase for the beam, return 0 if the beam failed to start, and should be deleted altogether
int beam_start_firing(beam *b)
{
	// kill the warmup stamp so the rest of the code knows its firing
	b->warmup_stamp = -1;	

	// any special stuff for each weapon type
	switch(b->type){
	// re-aim type A and D beam weapons here, otherwise they tend to miss		
	case BEAM_TYPE_A:
	case BEAM_TYPE_D:
		beam_aim(b);
		break;
	
	case BEAM_TYPE_B:
		break;

	case BEAM_TYPE_C:
		break;	

	case BEAM_TYPE_E:
		break;

	default:
		Int3();
	}

	// determine if we can legitimately start firing, or if we need to take other action
	switch(beam_ok_to_fire(b)){
	case -1 :
		return 0;

	case 0 :			
		beam_start_warmdown(b);
		return 1;
	}				

	// start the beam firing sound now, if we haven't already		
	if((b->beam_sound_loop == -1) && (Weapon_info[b->weapon_info_index].b_info.beam_loop_sound >= 0)){				
		b->beam_sound_loop = snd_play_3d(&Snds[Weapon_info[b->weapon_info_index].b_info.beam_loop_sound], &b->last_start, &View_position, 0.0f, NULL, 1, 1.0, SND_PRIORITY_SINGLE_INSTANCE, NULL, 1.0f, 1);

		// "shot" sound
		if (Weapon_info[b->weapon_info_index].launch_snd >= 0)
			snd_play_3d(&Snds[Weapon_info[b->weapon_info_index].launch_snd], &b->last_start, &View_position);
		else
			snd_play_3d(&Snds[SND_BEAM_SHOT], &b->last_start, &View_position);
	}	

	// success
	return 1;
}

// start the warmdown phase for the beam
void beam_start_warmdown(beam *b)
{
	// timestamp
	b->warmdown_stamp = timestamp(Weapon_info[b->weapon_info_index].b_info.beam_warmdown);			

	// start the warmdown sound
	if(Weapon_info[b->weapon_info_index].b_info.beam_warmdown_sound >= 0){				
		snd_play_3d(&Snds[Weapon_info[b->weapon_info_index].b_info.beam_warmdown_sound], &b->last_start, &View_position);
	}

	// kill the beam looping sound 
	if(b->beam_sound_loop != -1){
		snd_stop(b->beam_sound_loop);
		b->beam_sound_loop = -1;
	}						
}

// recalculate beam sounds (looping sounds relative to the player)
void beam_recalc_sounds(beam *b)
{
	beam_weapon_info *bwi;
	vec3d pos;	

	Assert(b->weapon_info_index >= 0);
	if(b->weapon_info_index < 0){
		return;
	}
	bwi = &Weapon_info[b->weapon_info_index].b_info;

	// update the sound position relative to the player
	if(b->beam_sound_loop != -1){
		// get the point closest to the player's viewing position
		switch(vm_vec_dist_to_line(&View_position, &b->last_start, &b->last_shot, &pos, NULL)){
		// behind the beam, so use the start pos
		case -1:
			pos = b->last_start;
			break;

		// use the closest point
		case 0:
			// already calculated in vm_vec_dist_to_line(...)
			break;

		// past the beam, so use the shot pos
		case 1:
			pos = b->last_shot;
			break;
		}

		snd_update_3d_pos(b->beam_sound_loop, &Snds[bwi->beam_loop_sound], &pos);
	}
}


// -----------------------------===========================------------------------------
// BEAM AIMING FUNCTIONS
// -----------------------------===========================------------------------------

// fills in binfo
void beam_get_binfo(beam *b, float accuracy, int num_shots)
{
	vec3d p2;
	int model_num, idx;
	vec3d oct1, oct2;
	vec3d turret_point, turret_norm;
	beam_weapon_info *bwi;
	float miss_factor;

	int temp = b->subsys->turret_next_fire_pos;

	if (b->fighter_beam == false)
		b->subsys->turret_next_fire_pos = b->firingpoint;

	// where the shot is originating from (b->last_start gets filled in)
	beam_get_global_turret_gun_info(b->objp, b->subsys, &turret_point, &turret_norm, 1, &p2, b->fighter_beam);

	b->subsys->turret_next_fire_pos = temp;

	// get a model # to work with
	model_num = beam_get_model(b->target);
	if(model_num < 0){
		return;
	}

	// get beam weapon info
	Assert(b->weapon_info_index >= 0);
	if(b->weapon_info_index < 0){
		return;
	}
	bwi = &Weapon_info[b->weapon_info_index].b_info;

	// stuff num shots even though its only used for type D weapons
	b->binfo.shot_count = (ubyte)num_shots;
	if(b->binfo.shot_count > MAX_BEAM_SHOTS){
		b->binfo.shot_count = MAX_BEAM_SHOTS;
	}

	// generate the proper amount of directional vectors
	switch(b->type){
	// pick an accuracy. beam will be properly aimed at actual fire time
	case BEAM_TYPE_A:
		// determine the miss factor
		Assert(Game_skill_level >= 0 && Game_skill_level < NUM_SKILL_LEVELS);
		Assert(b->team >= 0 && b->team < Num_iffs);
		miss_factor = bwi->beam_iff_miss_factor[b->team][Game_skill_level];

		// all we will do is decide whether or not we will hit - type A beam weapons are re-aimed immediately before firing
		b->binfo.shot_aim[0] = frand_range(0.0f, 1.0f + miss_factor * accuracy);
		b->binfo.shot_count = 1;

		// get random model points, this is useful for big ships, because we never miss when shooting at them
		submodel_get_two_random_points(model_num, 0, &b->binfo.dir_a, &b->binfo.dir_b);
		break;

	// just 2 points in the "slash"
	case BEAM_TYPE_B:
		beam_get_octant_points(model_num, b->target, (int)frand_range(0.0f, BEAM_NUM_GOOD_OCTANTS), Beam_good_slash_octants, &oct1, &oct2);

		// point 1
		vm_vec_sub(&b->binfo.dir_a, &oct1, &turret_point);
		vm_vec_normalize(&b->binfo.dir_a);

		// point 2
		vm_vec_sub(&b->binfo.dir_b, &oct2, &turret_point);
		vm_vec_normalize(&b->binfo.dir_b);
		
		// delta angle
		b->binfo.delta_ang = fl_abs(vm_vec_delta_ang_norm(&b->binfo.dir_a, &b->binfo.dir_b, NULL));
		break;

	// nothing for this beam - its very special case
	case BEAM_TYPE_C:
		break;

	// type D beams fire at small ship multiple times
	case BEAM_TYPE_D:
		// determine the miss factor
		Assert(Game_skill_level >= 0 && Game_skill_level < NUM_SKILL_LEVELS);
		Assert(b->team >= 0 && b->team < Num_iffs);
		miss_factor = bwi->beam_iff_miss_factor[b->team][Game_skill_level];

		// get a bunch of shot aims
		for(idx=0; idx<b->binfo.shot_count; idx++){
			//	MK, 9/3/99: Added pow() function to make increasingly likely to miss with subsequent shots.  30% more likely with each shot.
			float r = ((float) pow(1.3f, idx)) * miss_factor * accuracy;
			b->binfo.shot_aim[idx] = frand_range(0.0f, 1.0f + r);
		}
		break;

	// type e beams just fire straight
	case BEAM_TYPE_E:
		b->binfo.shot_aim[0] = 0.0000001f;
		b->binfo.shot_count = 1;
		b->binfo.dir_a = turret_norm;
		b->binfo.dir_b = turret_norm;
		break;

	default:
		break;
	}
}

// aim the beam (setup last_start and last_shot - the endpoints). also recalculates collision pairs
void beam_aim(beam *b)
{
	vec3d temp, p2;
	int model_num;	
	
	// type C beam weapons have no target
	if(b->target == NULL){
		Assert(b->type == BEAM_TYPE_C);
		if(b->type != BEAM_TYPE_C){
			return;
		}
	}
	// get a model # to work with
	else {
		model_num = beam_get_model(b->target);	
		if(model_num < 0){
			return;
		}	
	}

	int temp_int = b->subsys->turret_next_fire_pos;

	if (b->fighter_beam == false)
		b->subsys->turret_next_fire_pos = b->firingpoint;

	// setup our initial shot point and aim direction
	switch(b->type){
	case BEAM_TYPE_A:	
		// where the shot is originating from (b->last_start gets filled in)
		beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &p2, b->fighter_beam);

		// if we're targeting a subsystem - shoot directly at it
		if(b->target_subsys != NULL){			
			// unrotate the center of the subsystem
//			vm_vec_unrotate(&b->last_shot, &b->local_pnt, &b->target->orient);
			vm_vec_unrotate(&b->last_shot, &b->target_subsys->system_info->pnt, &b->target->orient);
			vm_vec_add2(&b->last_shot, &b->target->pos);		 
			vm_vec_sub(&temp, &b->last_shot, &b->last_start);
			
			vm_vec_scale_add(&b->last_shot, &b->last_start, &temp, 2.0f);
			break;
		}

		// if we're shooting at a big ship - shoot directly at the model
		if((b->target->type == OBJ_SHIP) && (Ship_info[Ships[b->target->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))){
			// rotate into world coords
			vm_vec_unrotate(&temp, &b->binfo.dir_a, &b->target->orient);
			vm_vec_add2(&temp, &b->target->pos);

			// get the shot point
			vm_vec_sub(&p2, &temp, &b->last_start);
			vm_vec_scale_add(&b->last_shot, &b->last_start, &p2, 2.0f);
			break;
		}
		
		// point at the center of the target, then jitter based on shot_aim
		b->last_shot = b->target->pos;		
		beam_jitter_aim(b, b->binfo.shot_aim[0]);
		break;	

	case BEAM_TYPE_B:		
		// where the shot is originating from (b->last_start gets filled in)
		beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &p2, b->fighter_beam);				

		// set the shot point
		vm_vec_scale_add(&b->last_shot, &b->last_start, &b->binfo.dir_a, b->range);
		Assert(is_valid_vec(&b->last_shot));		
		break;

	case BEAM_TYPE_C:
		// start point
		temp = b->targeting_laser_offset;	
		vm_vec_unrotate(&b->last_start, &temp, &b->objp->orient);
		vm_vec_add2(&b->last_start, &b->objp->pos);
		vm_vec_scale_add(&b->last_shot, &b->last_start, &b->objp->orient.vec.fvec, b->range);		
		break;

	case BEAM_TYPE_D:				
		// where the shot is originating from (b->last_start gets filled in)
		beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &p2, b->fighter_beam);		
		
		// point at the center of the target, then jitter based on shot_aim
		b->last_shot = b->target->pos;		
		beam_jitter_aim(b, b->binfo.shot_aim[b->shot_index]);
		nprintf(("AI", "Frame %i: FIRING\n", Framecount));
		break;

	case BEAM_TYPE_E:
		// where the shot is originating from (b->last_start gets filled in)
		beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &p2, b->fighter_beam);		

		// point directly in the direction of the turret
		vm_vec_scale_add(&b->last_shot, &b->last_start, &temp, b->range);
		break;

	default:
		Int3();
	}		

	b->subsys->turret_next_fire_pos = temp_int;

	// recalculate object pairs
	OBJ_RECALC_PAIRS((&Objects[b->objnum]));
}

// given a model #, and an object, stuff 2 good world coord points
void beam_get_octant_points(int modelnum, object *objp, int oct_index, int oct_array[BEAM_NUM_GOOD_OCTANTS][4], vec3d *v1, vec3d *v2)
{	
	vec3d t1, t2, temp;
	polymodel *m = model_get(modelnum);

	// bad bad bad bad bad bad
	if(m == NULL){
		Int3();
		return;
	}

	Assert((oct_index >= 0) && (oct_index < BEAM_NUM_GOOD_OCTANTS));

	// randomly pick octants	
	t1 = oct_array[oct_index][2] ? m->octants[oct_array[oct_index][0]].max : m->octants[oct_array[oct_index][0]].min;
	t2 = oct_array[oct_index][3] ? m->octants[oct_array[oct_index][1]].max : m->octants[oct_array[oct_index][1]].min;
	Assert(!vm_vec_same(&t1, &t2));

	// get them in world coords
	vm_vec_unrotate(&temp, &t1, &objp->orient);
	vm_vec_add(v1, &temp, &objp->pos);
	vm_vec_unrotate(&temp, &t2, &objp->orient);
	vm_vec_add(v2, &temp, &objp->pos);
}

// throw some jitter into the aim - based upon shot_aim
void beam_jitter_aim(beam *b, float aim)
{
	vec3d forward, circle;
	matrix m;
	float subsys_strength;

	// if the weapons subsystem is damaged or destroyed
	if((b->objp != NULL) && (b->objp->signature == b->sig) && (b->objp->type == OBJ_SHIP) && (b->objp->instance >= 0) && (b->objp->instance < MAX_SHIPS)){
		// get subsytem strength
		subsys_strength = ship_get_subsystem_strength(&Ships[b->objp->instance], SUBSYSTEM_WEAPONS);
		
		// when subsytem strength is 0, double the aim error factor
		aim += aim * (1.0f - subsys_strength);
	}

	// shot aim is a direct linear factor of the target model's radius.
	// so, pick a random point on the circle
	vm_vec_sub(&forward, &b->last_shot, &b->last_start);
	vm_vec_normalize_quick(&forward);
	
	// vector
	vm_vector_2_matrix(&m, &forward, NULL, NULL);

	// get a vector on the circle - this should appear to be pretty random
	// vm_vec_scale_add(&circle, &b->last_shot, &m.rvec, aim * b->target->radius);
	vm_vec_random_in_circle(&circle, &b->last_shot, &m, aim * b->target->radius, 0);
	
	// get the vector pointing to the circle point
	vm_vec_sub(&forward, &circle, &b->last_start);	
	vm_vec_scale_add(&b->last_shot, &b->last_start, &forward, 2.0f);
}


// -----------------------------===========================------------------------------
// BEAM COLLISION FUNCTIONS
// -----------------------------===========================------------------------------

// collide a beam with a ship, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_ship(obj_pair *pair)
{
	beam *b;	
	object *ship_objp;
	ship *shipp;
	ship_info *sip;
	weapon_info *bwi;
	mc_info mc, mc_shield, mc_hull_enter, mc_hull_exit;
	int model_num;
	float widest;

	// bogus
	if (pair == NULL) {
		return 0;
	}

	// get the beam
	Assert(pair->a->instance >= 0);
	Assert(pair->a->type == OBJ_BEAM);
	Assert(Beams[pair->a->instance].objnum == OBJ_INDEX(pair->a));
	b = &Beams[pair->a->instance];

	// Don't check collisions for warping out player if past stage 1.
	if (Player->control_mode >= PCM_WARPOUT_STAGE1) {
		if ( pair->a == Player_obj ) return 0;
		if ( pair->b == Player_obj ) return 0;
	}

	// if the "warming up" timestamp has not expired
	if ((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)) {		
		return 0;
	}

	// if the beam is on "safety", don't collide with anything
	if (b->flags & BF_SAFETY) {
		return 0;
	}
	
	// if the colliding object is the shooting object, return 1 so this is culled
	if (pair->b == b->objp) {
		return 1;
	}	

	// try and get a model
	model_num = beam_get_model(pair->b);
	if (model_num < 0) {
		// Int3();
		return 1;
	}
	
#ifndef NDEBUG
	Beam_test_ints++;
	Beam_test_ship++;
#endif

	// get the ship
	Assert(pair->b->instance >= 0);
	Assert(pair->b->type == OBJ_SHIP);
	Assert(Ships[pair->b->instance].objnum == OBJ_INDEX(pair->b));
	if ((pair->b->type != OBJ_SHIP) || (pair->b->instance < 0))
		return 1;
	ship_objp = pair->b;
	shipp = &Ships[ship_objp->instance];

	int quadrant_num = -1;
	int	valid_hit_occurred = 0;
	sip = &Ship_info[shipp->ship_info_index];
	bwi = &Weapon_info[b->weapon_info_index];

	polymodel *pm = model_get(model_num);

	// get the widest portion of the beam
	widest = beam_get_widest(b);


	// Goober5000 - I tried to make collision code much saner... here begin the (major) changes

	// set up collision structs, part 1
	mc.model_instance_num = shipp->model_instance_num;
	mc.model_num = model_num;
	mc.submodel_num = -1;
	mc.orient = &ship_objp->orient;
	mc.pos = &ship_objp->pos;
	mc.p0 = &b->last_start;
	mc.p1 = &b->last_shot;

	// maybe do a sphereline
	if (widest > ship_objp->radius * BEAM_AREA_PERCENT) {
		mc.radius = widest * 0.5f;
		mc.flags = MC_CHECK_SPHERELINE;
	} else {
		mc.flags = MC_CHECK_RAY;
	}

	// set up collision structs, part 2
	memcpy(&mc_shield, &mc, sizeof(mc_info));
	memcpy(&mc_hull_enter, &mc, sizeof(mc_info));
	memcpy(&mc_hull_exit, &mc, sizeof(mc_info));
	
	// reverse this vector so that we check for exit holes as opposed to entrance holes
	mc_hull_exit.p1 = &b->last_start;
	mc_hull_exit.p0 = &b->last_shot;

	// set flags
	mc_shield.flags |= MC_CHECK_SHIELD;
	mc_hull_enter.flags |= MC_CHECK_MODEL;
	mc_hull_exit.flags |= MC_CHECK_MODEL;

	// check all three kinds of collisions
	int shield_collision = (pm->shield.ntris > 0) ? model_collide(&mc_shield) : 0;
	int hull_enter_collision = model_collide(&mc_hull_enter);
	int hull_exit_collision = (beam_will_tool_target(b, ship_objp)) ? model_collide(&mc_hull_exit) : 0;

	// check shields for impact
	// (tooled ships are probably not going to be maintaining a shield over their exit hole,
	// therefore we need only check the entrance, just as with conventional weapons)
	if (!(ship_objp->flags & OF_NO_SHIELDS))
	{
		// pick out the shield quadrant
		if (shield_collision)
			quadrant_num = get_quadrant(&mc_shield.hit_point);
		else if (hull_enter_collision && (sip->flags2 & SIF2_SURFACE_SHIELDS))
			quadrant_num = get_quadrant(&mc_hull_enter.hit_point);

		// make sure that the shield is active in that quadrant
		if ((quadrant_num >= 0) && ((shipp->flags & SF_DYING) || !ship_is_shield_up(ship_objp, quadrant_num)))
			quadrant_num = -1;

		// see if we hit the shield
		if (quadrant_num >= 0)
		{
			// do the hit effect
			if (shield_collision) {
				add_shield_point(OBJ_INDEX(ship_objp), mc_shield.shield_hit_tri, &mc_shield.hit_point);
			} else {
				/* TODO */;
			}

			// if this weapon pierces the shield, then do the hit effect, but act like a shield collision never occurred;
			// otherwise, we have a valid hit on this shield
			if (bwi->wi_flags2 & WIF2_PIERCE_SHIELDS)
				quadrant_num = -1;
			else
				valid_hit_occurred = 1;
		}
	}

	// see which impact we use
	if (shield_collision && valid_hit_occurred)
	{
		memcpy(&mc, &mc_shield, sizeof(mc_info));
		Assert(quadrant_num >= 0);
	}
	else if (hull_enter_collision)
	{
		memcpy(&mc, &mc_hull_enter, sizeof(mc_info));
		valid_hit_occurred = 1;
	}

	// if we got a hit
	if (valid_hit_occurred) {
		// add to the collision_list
		beam_add_collision(b, ship_objp, &mc, quadrant_num);

		// if we got "tooled", add an exit hole too
		if (hull_exit_collision)
			beam_add_collision(b, ship_objp, &mc_hull_exit, quadrant_num, 1);
	}

	// add this guy to the lighting list
	if(Use_GLSL < 2)
		beam_add_light(b, OBJ_INDEX(ship_objp), 1, NULL);

	// reset timestamp to timeout immediately
	pair->next_check_time = timestamp(0);
		
	return 0;
}


// collide a beam with an asteroid, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_asteroid(obj_pair *pair)
{
	beam *b;		
	mc_info test_collide;		
	int model_num;

	// bogus
	if(pair == NULL){
		return 0;
	}

	// get the beam
	Assert(pair->a->instance >= 0);
	Assert(pair->a->type == OBJ_BEAM);
	Assert(Beams[pair->a->instance].objnum == OBJ_INDEX(pair->a));
	b = &Beams[pair->a->instance];

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return 0;
	}

	// if the beam is on "safety", don't collide with anything
	if(b->flags & BF_SAFETY){
		return 0;
	}
	
	// if the colliding object is the shooting object, return 1 so this is culled
	if(pair->b == b->objp){
		return 1;
	}	

	// try and get a model
	model_num = beam_get_model(pair->b);
	if(model_num < 0){
		Int3();
		return 1;
	}	

#ifndef NDEBUG
	Beam_test_ints++;
	Beam_test_ast++;
#endif

	// do the collision		
	test_collide.model_instance_num = -1;
	test_collide.model_num = model_num;
	test_collide.submodel_num = -1;
	test_collide.orient = &pair->b->orient;
	test_collide.pos = &pair->b->pos;
	test_collide.p0 = &b->last_start;
	test_collide.p1 = &b->last_shot;	
	test_collide.flags = MC_CHECK_MODEL | MC_CHECK_RAY;
	model_collide(&test_collide);

	// if we got a hit
	if(test_collide.num_hits){
		// add to the collision list
		beam_add_collision(b, pair->b, &test_collide);
	}	

	// add this guy to the lighting list
	if(Use_GLSL < 2)
		beam_add_light(b, OBJ_INDEX(pair->b), 1, NULL);

	// reset timestamp to timeout immediately
	pair->next_check_time = timestamp(0);
		
	return 0;	
}

// collide a beam with a missile, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_missile(obj_pair *pair)
{
	beam *b;	
	mc_info test_collide;		
	int model_num;

	// bogus
	if(pair == NULL){
		return 0;
	}

	// get the beam
	Assert(pair->a->instance >= 0);
	Assert(pair->a->type == OBJ_BEAM);
	Assert(Beams[pair->a->instance].objnum == OBJ_INDEX(pair->a));
	b = &Beams[pair->a->instance];

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return 0;
	}

	// if the beam is on "safety", don't collide with anything
	if(b->flags & BF_SAFETY){
		return 0;
	}
	
	// if the colliding object is the shooting object, return 1 so this is culled
	if(pair->b == b->objp){
		return 1;
	}	

	// try and get a model
	model_num = beam_get_model(pair->b);
	if(model_num < 0){
		//Int3();
		return 1;
	}

#ifndef NDEBUG
	Beam_test_ints++;
#endif

	// do the collision		
	test_collide.model_instance_num = -1;
	test_collide.model_num = model_num;
	test_collide.submodel_num = -1;
	test_collide.orient = &pair->b->orient;
	test_collide.pos = &pair->b->pos;
	test_collide.p0 = &b->last_start;
	test_collide.p1 = &b->last_shot;
	test_collide.flags = MC_CHECK_MODEL | MC_CHECK_RAY;
	model_collide(&test_collide);

	// if we got a hit
	if(test_collide.num_hits){
		// add to the collision list
		beam_add_collision(b, pair->b, &test_collide);
	}

	// reset timestamp to timeout immediately
	pair->next_check_time = timestamp(0);

	return 0;
}

// collide a beam with debris, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_debris(obj_pair *pair)
{	
	beam *b;	
	mc_info test_collide;		
	int model_num;

	// bogus
	if(pair == NULL){
		return 0;
	}

	// get the beam
	Assert(pair->a->instance >= 0);
	Assert(pair->a->type == OBJ_BEAM);
	Assert(Beams[pair->a->instance].objnum == OBJ_INDEX(pair->a));
	b = &Beams[pair->a->instance];

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return 0;
	}

	// if the beam is on "safety", don't collide with anything
	if(b->flags & BF_SAFETY){
		return 0;
	}
	
	// if the colliding object is the shooting object, return 1 so this is culled
	if(pair->b == b->objp){
		return 1;
	}	

	// try and get a model
	model_num = beam_get_model(pair->b);
	if(model_num < 0){
		// Int3();
		return 1;
	}	

#ifndef NDEBUG
	Beam_test_ints++;
#endif

	// do the collision	
	test_collide.model_instance_num = -1;
	test_collide.model_num = model_num;
	test_collide.submodel_num = -1;
	test_collide.orient = &pair->b->orient;
	test_collide.pos = &pair->b->pos;
	test_collide.p0 = &b->last_start;
	test_collide.p1 = &b->last_shot;
	test_collide.flags = MC_CHECK_MODEL | MC_CHECK_RAY;
	model_collide(&test_collide);

	// if we got a hit
	if(test_collide.num_hits){
		// add to the collision list
		beam_add_collision(b, pair->b, &test_collide);
	}	

	// add this guy to the lighting list
	if(Use_GLSL < 2)
		beam_add_light(b, OBJ_INDEX(pair->b), 1, NULL);

	// reset timestamp to timeout immediately
	pair->next_check_time = timestamp(0);

	return 0;
}

// early-out function for when adding object collision pairs, return 1 if the pair should be ignored
int beam_collide_early_out(object *a, object *b)
{
	beam *bm;
	weapon_info *bwi;
	float cull_dist, cull_dot;
	vec3d dot_test, dot_test2, dist_test;	
		
	// get the beam
	Assert(a->instance >= 0);
	if(a->instance < 0){
		return 1;
	}
	Assert(a->type == OBJ_BEAM);
	if(a->type != OBJ_BEAM){
		return 1;
	}
	Assert(Beams[a->instance].objnum == OBJ_INDEX(a));
	if(Beams[a->instance].objnum != OBJ_INDEX(a)){
		return 1;
	}	
	bm = &Beams[a->instance];
	Assert(bm->weapon_info_index >= 0);
	if(bm->weapon_info_index < 0){
		return 1;
	}
	bwi = &Weapon_info[bm->weapon_info_index];

	// if the second object has an invalid instance, bail
	if(b->instance < 0){
		return 1;
	}

	if((vm_vec_dist(&bm->last_start, &b->pos)-b->radius) > bwi->b_info.range){
		return 1;
	}//if the object is too far away, don't bother trying to colide with it-Bobboau

	// baseline bails
	switch(b->type){
	case OBJ_SHIP:
		break;
	case OBJ_ASTEROID:
		// targeting lasers only hit ships
/*		if(bwi->b_info.beam_type == BEAM_TYPE_C){
			return 1;
		}*/
		break;
	case OBJ_DEBRIS:
		// targeting lasers only hit ships
/*		if(bwi->b_info.beam_type == BEAM_TYPE_C){
			return 1;
		}*/
		// don't ever collide with non hull pieces
		if(!Debris[b->instance].is_hull){
			return 1;
		}
		break;
	case OBJ_WEAPON:
		// targeting lasers only hit ships
/*		if(bwi->b_info.beam_type == BEAM_TYPE_C){
			return 1;
		}*/
		if(The_mission.ai_profile->flags2 & AIPF2_BEAMS_DAMAGE_WEAPONS) {
			if((Weapon_info[Weapons[b->instance].weapon_info_index].weapon_hitpoints <= 0) && (Weapon_info[Weapons[b->instance].weapon_info_index].subtype == WP_LASER)) {
				return 1;
			}
		} else {
			// don't ever collide against laser weapons - duh
			if(Weapon_info[Weapons[b->instance].weapon_info_index].subtype == WP_LASER){
				return 1;
			}
		}
		break;
	}

	// get full cull value
	beam_get_cull_vals(b, bm, &cull_dot, &cull_dist);

	// if the object fails these conditions, bail
	vm_vec_sub(&dist_test, &b->pos, &bm->last_start);
	dot_test = dist_test;
	vm_vec_sub(&dot_test2, &bm->last_shot, &bm->last_start);
	vm_vec_normalize_quick(&dot_test);
	vm_vec_normalize_quick(&dot_test2);
	// cull_dist == DIST SQUARED FOO!
	if((vm_vec_dotprod(&dot_test, &dot_test2) < cull_dot) && (vm_vec_mag_squared(&dist_test) > cull_dist)){
		return 1;
	}
	
	// don't cull
	return 0;
}

// add a collision to the beam for this frame (to be evaluated later)
// Goober5000 - erg.  Rearranged for clarity, and also to fix a bug that caused is_exit_collision to hardly ever be assigned,
// resulting in "tooled" ships taking twice as much damage (in a later function) as they should.
void beam_add_collision(beam *b, object *hit_object, mc_info *cinfo, int quadrant_num, int exit_flag)
{
	beam_collision *bc = NULL;
	int idx;

	// if we haven't reached the limit for beam collisions, just add it
	if (b->f_collision_count < MAX_FRAME_COLLISIONS) {
		bc = &b->f_collisions[b->f_collision_count++];
	}
	// otherwise, we've got to do some checking, ick. 
	// I guess we can always just remove the farthest item
	else {
		for (idx = 0; idx < MAX_FRAME_COLLISIONS; idx++) {
			if ((bc == NULL) || (b->f_collisions[idx].cinfo.hit_dist > bc->cinfo.hit_dist))
				bc = &b->f_collisions[idx];
		}
	}

	if (bc == NULL) {
		Int3();
		return;
	}

	// copy in
	bc->c_objnum = OBJ_INDEX(hit_object);
	bc->cinfo = *cinfo;
	bc->quadrant = quadrant_num;
	bc->is_exit_collision = exit_flag;

	// let the hud shield gauge know when Player or Player target is hit
	if (quadrant_num >= 0)
		hud_shield_quadrant_hit(hit_object, quadrant_num);
}

// sort collisions for the frame
int beam_sort_collisions_func(const void *e1, const void *e2)
{
	beam_collision *b1 = (beam_collision*)e1;
	beam_collision *b2 = (beam_collision*)e2;

	return b1->cinfo.hit_dist < b2->cinfo.hit_dist ? -1 : 1;
}

// handle a hit on a specific object
void beam_handle_collisions(beam *b)
{	
	int idx, s_idx;
	beam_collision r_coll[MAX_FRAME_COLLISIONS];
	int r_coll_count = 0;
	beam_weapon_info *bwi;
	weapon_info *wi;
	float widest;	

	// early out if we had no collisions
	if(b->f_collision_count <= 0){
		return;
	}

	// get beam weapon info
	if((b->weapon_info_index < 0) || (b->weapon_info_index >= Num_weapon_types)){
		Int3();
		return;
	}
	bwi = &Weapon_info[b->weapon_info_index].b_info;
	wi = &Weapon_info[b->weapon_info_index];

	// get the widest part of the beam
	widest = beam_get_widest(b);

	// the first thing we need to do is sort the collisions, from closest to farthest
	qsort(b->f_collisions, b->f_collision_count, sizeof(beam_collision), beam_sort_collisions_func);

	// now apply all collisions until we reach a ship which "stops" the beam or we reach the end of the list
	for(idx=0; idx<b->f_collision_count; idx++){	
		int model_num = -1;
		int do_damage = 0;
		int draw_effects = 1;
		int first_hit = 1;
		int target = b->f_collisions[idx].c_objnum;

		// if we have an invalid object
		if((target < 0) || (target >= MAX_OBJECTS)){
			continue;
		}

		// try and get a model to deal with		
		model_num = beam_get_model(&Objects[target]);
		if(model_num < 0){
			continue;
		}

		if (wi->wi_flags & WIF_HUGE) {
			if (Objects[target].type == OBJ_SHIP) {
				ship_type_info *sti;
				sti = ship_get_type_info(&Objects[target]);
				if (sti->weapon_bools & STI_WEAP_NO_HUGE_IMPACT_EFF)
					draw_effects = 0;
			}
		}

		// add lighting
		if(Use_GLSL < 2)
			beam_add_light(b, target, 2, &b->f_collisions[idx].cinfo.hit_point_world);

		// add to the recent collision list
		r_coll[r_coll_count].c_objnum = target;
		r_coll[r_coll_count].c_sig = Objects[target].signature;
		r_coll[r_coll_count].c_stamp = -1;
		r_coll[r_coll_count].cinfo = b->f_collisions[idx].cinfo;
		
		// if he was already on the recent collision list, copy his timestamp
		// also, be sure not to play the impact sound again.
		for(s_idx=0; s_idx<b->r_collision_count; s_idx++){
			if((r_coll[r_coll_count].c_objnum == b->r_collisions[s_idx].c_objnum) && (r_coll[r_coll_count].c_sig == b->r_collisions[s_idx].c_sig)){
				// timestamp
				r_coll[r_coll_count].c_stamp = b->r_collisions[s_idx].c_stamp;

				// don't play the impact sound again
				first_hit = 0;
			}
		}

		// if the damage timestamp has expired or is not set yet, apply damage
		if((r_coll[r_coll_count].c_stamp == -1) || timestamp_elapsed(r_coll[r_coll_count].c_stamp))
        {
            float time_compression = f2fl(Game_time_compression);
            float delay_time = i2fl(BEAM_DAMAGE_TIME) / time_compression;
            do_damage = 1;
            r_coll[r_coll_count].c_stamp = timestamp(fl2i(delay_time));
		}

		// if no damage - don't even indicate it has been hit
		if(wi->damage <= 0){
			do_damage = 0;
		}

		// increment collision count
		r_coll_count++;		

		// play the impact sound
		if ( first_hit && (wi->impact_snd >= 0) ) {
			snd_play_3d( &Snds[wi->impact_snd], &b->f_collisions[idx].cinfo.hit_point_world, &Eye_position );
		}

		// KOMET_EXT -->

		// draw flash, explosion
		if (draw_effects && ((wi->piercing_impact_explosion_radius > 0) || (wi->flash_impact_explosion_radius > 0))) {
			float flash_rad = (1.2f + 0.007f * (float)(rand()%100));
			float rnd = frand();
			int do_expl = 0;
			if((rnd < 0.2f || do_damage) && wi->impact_weapon_expl_index >= 0){
				do_expl = 1;
			}
			float ani_radius;
			vec3d temp_pos, temp_local_pos;
				
			vm_vec_sub(&temp_pos, &b->f_collisions[idx].cinfo.hit_point_world, &Objects[target].pos);
			vm_vec_rotate(&temp_local_pos, &temp_pos, &Objects[target].orient);
						
			if (wi->flash_impact_explosion_radius > 0) {
				ani_radius = wi->flash_impact_explosion_radius * flash_rad;	
				if (wi->flash_impact_weapon_expl_index > -1) {
					int ani_handle = Weapon_explosions.GetAnim(wi->flash_impact_weapon_expl_index, &b->f_collisions[idx].cinfo.hit_point_world, ani_radius);
					particle_create( &temp_local_pos, &vmd_zero_vector, 0.005f * ani_radius, ani_radius, PARTICLE_BITMAP_PERSISTENT, ani_handle, -1, &Objects[target] );
				} else {
					particle_create( &temp_local_pos, &vmd_zero_vector, 0.005f * ani_radius, ani_radius, PARTICLE_SMOKE, 0, -1, &Objects[target] );
				}
			}
			if(do_expl){
				ani_radius = 0.7f * wi->impact_explosion_radius * flash_rad;
				int ani_handle = Weapon_explosions.GetAnim(wi->impact_weapon_expl_index, &b->f_collisions[idx].cinfo.hit_point_world, ani_radius);
				particle_create( &temp_local_pos, &vmd_zero_vector, 0.0f, ani_radius, PARTICLE_BITMAP_PERSISTENT, ani_handle, -1, &Objects[target] );
			}
			
			if (wi->piercing_impact_explosion_radius > 0) {
				vec3d fvec;
				vm_vec_sub(&fvec, &b->last_shot, &b->last_start);

				if(!IS_VEC_NULL(&fvec)){
					// get beam direction

					int ok_to_draw = 0;
					
					if (beam_will_tool_target(b, &Objects[target])) {
						ok_to_draw = 1;

						if (Objects[target].type == OBJ_SHIP) {
							ship *shipp = &Ships[Objects[target].instance];
														
							if (shipp->armor_type_idx != -1) {
								if (Armor_types[shipp->armor_type_idx].GetPiercingType(wi->damage_type_idx) == SADTF_PIERCING_RETAIL) {
									ok_to_draw = 0;
								}
							}
						}
					} else {
						ok_to_draw = 0;

						if (Objects[target].type == OBJ_SHIP) {
							float draw_limit, hull_pct;
							int dmg_type_idx, piercing_type;

							ship *shipp = &Ships[Objects[target].instance];

							hull_pct = Objects[target].hull_strength / shipp->ship_max_hull_strength;
							dmg_type_idx = wi->damage_type_idx;
							draw_limit = Ship_info[shipp->ship_info_index].piercing_damage_draw_limit;
							
							if (shipp->armor_type_idx != -1) {
								piercing_type = Armor_types[shipp->armor_type_idx].GetPiercingType(dmg_type_idx);
								if (piercing_type == SADTF_PIERCING_DEFAULT) {
									draw_limit = Armor_types[shipp->armor_type_idx].GetPiercingLimit(dmg_type_idx);
								} else if ((piercing_type == SADTF_PIERCING_NONE) || (piercing_type == SADTF_PIERCING_RETAIL)) {
									draw_limit = -1.0f;
								}
							}

							if ((draw_limit != -1.0f) && (hull_pct <= draw_limit))
								ok_to_draw = 1;
						}
					}

					if (ok_to_draw){
						vm_vec_normalize_quick(&fvec);
						
						// stream of fire for big ships
						if (widest <= Objects[target].radius * BEAM_AREA_PERCENT) {

							vec3d expl_vel, expl_splash_vel;

							float flame_size = wi->piercing_impact_explosion_radius * frand_range(0.5f,2.0f);
							float base_v, back_v;
							vec3d rnd_vec;

							vm_vec_rand_vec_quick(&rnd_vec);

							if (wi->piercing_impact_particle_velocity != 0.0f)
								base_v = wi->piercing_impact_particle_velocity;
							else
								base_v = wi->piercing_impact_explosion_radius;

							if (wi->piercing_impact_particle_back_velocity != 0.0f)
								back_v = wi->piercing_impact_particle_back_velocity;
							else
								back_v = base_v * (-0.2f);

							vm_vec_copy_scale( &expl_vel, &fvec, base_v * frand_range(1.0f, 2.0f));
							vm_vec_copy_scale( &expl_splash_vel, &fvec, back_v * frand_range(1.0f, 2.0f));
							vm_vec_scale_add2( &expl_vel, &rnd_vec, base_v * wi->piercing_impact_particle_variance);
							vm_vec_scale_add2( &expl_splash_vel, &rnd_vec, back_v * wi->piercing_impact_particle_variance);

							if (wi->piercing_impact_weapon_expl_index > -1) {
								int ani_handle = Weapon_explosions.GetAnim(wi->piercing_impact_weapon_expl_index, &b->f_collisions[idx].cinfo.hit_point_world, flame_size);
								particle_create( &b->f_collisions[idx].cinfo.hit_point_world, &expl_vel, 0.0f, flame_size, PARTICLE_BITMAP_PERSISTENT, ani_handle );
								particle_create( &b->f_collisions[idx].cinfo.hit_point_world, &expl_splash_vel, 0.0f, flame_size, PARTICLE_BITMAP_PERSISTENT, ani_handle );
							} else {
								particle_create( &b->f_collisions[idx].cinfo.hit_point_world, &expl_vel, 0.3f, flame_size, PARTICLE_SMOKE );
								particle_create( &b->f_collisions[idx].cinfo.hit_point_world, &expl_splash_vel, 0.6f, flame_size, PARTICLE_SMOKE );
							}
						}
					}
				}
			}
			// <-- KOMET_EXT
		} else {
			if(draw_effects && do_damage && !physics_paused){
				// maybe draw an explosion, if we aren't hitting shields
				if ( (wi->impact_weapon_expl_index >= 0) && (b->f_collisions[idx].quadrant < 0) ) {
					int ani_handle = Weapon_explosions.GetAnim(wi->impact_weapon_expl_index, &b->f_collisions[idx].cinfo.hit_point_world, wi->impact_explosion_radius);
					particle_create( &b->f_collisions[idx].cinfo.hit_point_world, &vmd_zero_vector, 0.0f, wi->impact_explosion_radius, PARTICLE_BITMAP_PERSISTENT, ani_handle );
				}
			}
		}

		if(do_damage && !physics_paused){

			switch(Objects[target].type){
			case OBJ_DEBRIS:
				// hit the debris - the debris hit code takes care of checking for MULTIPLAYER_CLIENT, etc
				debris_hit(&Objects[target], &Objects[b->objnum], &b->f_collisions[idx].cinfo.hit_point_world, Weapon_info[b->weapon_info_index].damage);
				break;

			case OBJ_WEAPON:
				if (The_mission.ai_profile->flags2 & AIPF2_BEAMS_DAMAGE_WEAPONS) {
					if (!(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER) {
						object *trgt = &Objects[target];

						if (trgt->hull_strength > 0) {
							float attenuation = 1.0f;
							if ((b->damage_threshold >= 0.0f) && (b->damage_threshold < 1.0f)) {
								float dist = vm_vec_dist(&b->last_shot, &b->last_start);
								float range = b->range;
								float atten_dist = range * b->damage_threshold;
								if ((range > dist) && (atten_dist < dist)) {
									attenuation = (dist - atten_dist) / (range - atten_dist);
								}
							}

							float damage = Weapon_info[b->weapon_info_index].damage * attenuation;

							trgt->hull_strength -= damage;

							if (trgt->hull_strength < 0) {
								weapon_hit(trgt, NULL, &trgt->pos);
							}
						} else {
							if (!(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER) {
								weapon_hit(&Objects[target], NULL, &Objects[target].pos);
							}
						}
						

					}
				} else {
					// detonate the missile
					Assert(Weapon_info[Weapons[Objects[target].instance].weapon_info_index].subtype == WP_MISSILE);

					if (!(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER) {
						weapon_hit(&Objects[target], NULL, &Objects[target].pos);
					}
				}
				break;

			case OBJ_ASTEROID:
				// hit the asteroid
				if (!(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER) {
					asteroid_hit(&Objects[target], &Objects[b->objnum], &b->f_collisions[idx].cinfo.hit_point_world, Weapon_info[b->weapon_info_index].damage);
				}
				break;
			case OBJ_SHIP:	
				// hit the ship - again, the innards of this code handle multiplayer cases
				// maybe vaporize ship.
				//only apply damage if the collision is not an exit collision.  this prevents twice the damage from being done, although it probably be more realistic since two holes are being punched in the ship instead of one.
				if (!b->f_collisions[idx].is_exit_collision)
					ship_apply_local_damage(&Objects[target], &Objects[b->objnum], &b->f_collisions[idx].cinfo.hit_point_world, beam_get_ship_damage(b, &Objects[target]), b->f_collisions[idx].quadrant);

				// if this is the first hit on the player ship. whack him
				if(do_damage)
				{
					// Goober5000 - AGH!  BAD BAD BAD BAD BAD BAD BAD BAD bug!  The whack's hit point is in *local*
					// coordinates, NOT world coordinates!
					beam_apply_whack(b, &Objects[target], &b->f_collisions[idx].cinfo.hit_point);
				}
				break;
			}		
		}				

		// if the radius of the target is somewhat close to the radius of the beam, "stop" the beam here
		// for now : if its smaller than about 1/3 the radius of the ship
		if(widest <= (Objects[target].radius * BEAM_AREA_PERCENT) && !beam_will_tool_target(b, &Objects[target])){	
			// set last_shot so we know where to properly draw the beam		
			b->last_shot = b->f_collisions[idx].cinfo.hit_point_world;
			Assert(is_valid_vec(&b->last_shot));		

			// done wif the beam
			break;
		}
	}

	// store the new recent collisions
	for(idx=0; idx<r_coll_count; idx++){
		b->r_collisions[idx] = r_coll[idx];
	}
	b->r_collision_count = r_coll_count;
}

// for a given object, and a firing beam, determine its critical dot product and range
void beam_get_cull_vals(object *objp, beam *b, float *cull_dot, float *cull_dist)
{
	switch(objp->type){
	// debris and asteroids are classified as slow moving small objects
	// use cull_dot == potential cone of beam + 10% and 50.0 meters
	case OBJ_DEBRIS:
	case OBJ_ASTEROID:
		*cull_dot = 1.0f - ((1.0f - beam_get_cone_dot(b)) * 1.10f);
		*cull_dist = 50.0f * 50.0f;
		return;

	// treat missiles as fast-moving small objects
	case OBJ_WEAPON:
		*cull_dot = 1.0f - ((1.0f - beam_get_cone_dot(b)) * 1.5f);
		*cull_dist = 300.0f * 300.0f;
		return;

	case OBJ_SHIP:
		// for cap ships, only cull for 90deg or better
		/*
		if(Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_CAPITAL){
			*cull_dot = 0.0f;
			*cull_dist = 0.0f;
			return;
		}
		*/

		// for large ships, cull at some multiple of the radius
		if(Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)){
			*cull_dot = 1.0f - ((1.0f - beam_get_cone_dot(b)) * 1.25f);
			
			*cull_dist = (objp->radius * 1.3f) * (objp->radius * 1.3f);
			return;
		}

		// for everthing else, cull the same as missiles
		*cull_dot = 1.0f - ((1.0f - beam_get_cone_dot(b)) * 1.5f);
		*cull_dist = 300.0f * 300.0f;
		return;
	}

	// BAD BAD BAD - but this code will cause everything to cull properly
	Int3();
	*cull_dot = 1.0f;
	*cull_dist = 0.0f;
	return;
}

// FIXME - make sure we are truthfull representing the "cone" for all beam types
// get the total possible cone for a given beam in radians
float beam_get_cone_dot(beam *b)
{
	switch(b->type){
	case BEAM_TYPE_A:
	case BEAM_TYPE_C:
	case BEAM_TYPE_D:
	case BEAM_TYPE_E:
		// even though these beams don't move, return a _very_ small value
		return (float)cos(fl_radians(50.5f));
		
	case BEAM_TYPE_B:
		return vm_vec_dotprod(&b->binfo.dir_a, &b->binfo.dir_b);

	default:
		Int3();
	}

	Int3();
	return 0.0f;
}

// if it is legal for the beam to fire, or continue firing
int beam_ok_to_fire(beam *b)
{
	// if my own object is invalid, stop firing
	if(b->objp->signature != b->sig){
		mprintf(("BEAM : killing beam because of invalid parent object SIGNATURE!\n"));
		return -1;
	}

	// if my own object is a ghost
	if(b->objp->type != OBJ_SHIP){
		mprintf(("BEAM : killing beam because of invalid parent object TYPE!\n"));
		return -1;
	}	

	// type C beams are ok to fire all the time
	if (b->type == BEAM_TYPE_C) {
		ship *shipp = &Ships[b->objp->instance];

		if (shipp->weapon_energy <= 0.0f) {
		//	shipp->weapons.next_primary_fire_stamp[b->bank] = timestamp(Weapon_info[shipp->weapons.primary_bank_weapons[b->bank]].b_info.beam_warmdown*2);
		//	shipp->weapons.next_primary_fire_stamp[b->bank] = timestamp(2000);
			shipp->weapons.next_primary_fire_stamp[b->bank] = timestamp(shipp->weapons.next_primary_fire_stamp[b->bank] * 2);

			if ( OBJ_INDEX(Player_obj) == shipp->objnum ) {
				extern int ship_maybe_play_primary_fail_sound();
				ship_maybe_play_primary_fail_sound();
			}

		//	mprintf(("killing fighter beam becase it ran out of energy\n"));

			return 0;
		} else {
			return 1;
		}
	}

	// if the shooting turret is destroyed	
	if(b->subsys->current_hits <= 0.0f){		
		mprintf(("BEAM : killing beam because turret has been destroyed!\n"));
		return -1;
	}
	
	//kill it if its disrupted
	if (ship_subsys_disrupted(b->subsys))
	{
		return -1;
	}

	// if the beam will be firing out of its FOV, power it down
	vec3d aim_dir;
	vm_vec_sub(&aim_dir, &b->last_shot, &b->last_start);
	vm_vec_normalize(&aim_dir);

	if(!(The_mission.ai_profile->flags & AIPF_FORCE_BEAM_TURRET_FOV)) {
		vec3d turret_dir, turret_pos, temp;
		beam_get_global_turret_gun_info(b->objp, b->subsys, &turret_pos, &turret_dir, 1, &temp, b->fighter_beam);
		if(vm_vec_dotprod(&aim_dir, &turret_dir) < b->subsys->system_info->turret_fov){
			nprintf(("BEAM", "BEAM : powering beam down because of FOV condition!\n"));
			return 0;
		}
	} else {
		vec3d turret_normal;

		if (b->fighter_beam) {
			turret_normal = b->objp->orient.vec.fvec;
			b->subsys->system_info->flags &= ~MSS_FLAG_TURRET_ALT_MATH;
		} else {
			vm_vec_unrotate(&turret_normal, &b->subsys->system_info->turret_norm, &b->objp->orient);
		}

		if(!(turret_fov_test(b->subsys, &turret_normal, &aim_dir))) {
			nprintf(("BEAM", "BEAM : powering beam down because of FOV condition!\n"));
			return 0;
		}
	}

	// ok to fire/continue firing
	return 1;
}

// get the width of the widest section of the beam
float beam_get_widest(beam *b)
{
	int idx;
	float widest = -1.0f;

	// sanity
	Assert(b->weapon_info_index >= 0);
	if(b->weapon_info_index < 0){
		return -1.0f;
	}

	if (b->beam_width > 0.0f) {
		widest = b->beam_width;
	} else {
		// lookup
		for(idx=0; idx<Weapon_info[b->weapon_info_index].b_info.beam_num_sections; idx++){
			if(Weapon_info[b->weapon_info_index].b_info.sections[idx].width > widest){
				widest = Weapon_info[b->weapon_info_index].b_info.sections[idx].width;
			}
		}
	}

	// return	
	return widest * b->shrink;
}  

// apply a whack to a ship
void beam_apply_whack(beam *b, object *objp, vec3d *hit_point)
{
	weapon_info *wip;	
	ship *shipp;

	// sanity
	Assert((b != NULL) && (objp != NULL) && (hit_point != NULL));
	if((b == NULL) || (objp == NULL) || (hit_point == NULL)){
		return;
	}	
	Assert(b->weapon_info_index >= 0);
	wip = &Weapon_info[b->weapon_info_index];	
	Assert((objp != NULL) && (objp->type == OBJ_SHIP) && (objp->instance >= 0) && (objp->instance < MAX_SHIPS));
	if((objp == NULL) || (objp->type != OBJ_SHIP) || (objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return;
	}
	shipp = &Ships[objp->instance];
	if((shipp->ai_index < 0) || (shipp->ai_index >= MAX_AI_INFO)){
		return;
	}

	// don't whack docked ships
	// Goober5000 - whacking docked ships should work now, so whack them
	// Goober5000 - weapons with no mass don't whack (bypass the calculations)
	if(/*object_is_docked(objp) ||*/ (wip->mass == 0.0f)) {
		return;
	}

	// determine how big of a whack to apply
	float whack;
	float dist;

	// this if block was added by Bobboau to make beams whack properly while preserving reverse compatibility
	if(wip->mass == 100.0f){
		if(wip->damage < b_whack_damage){
			whack = b_whack_small;
		} else {
			whack = b_whack_big;
		}
	}else{
		whack = wip->mass;
	}

	// whack direction
	vec3d whack_dir, temp;
	vm_vec_dist_to_line(&objp->pos, &b->last_start, &b->last_shot, &temp, &dist);
	vm_vec_sub(&whack_dir, &objp->pos, &temp);
	vm_vec_normalize(&whack_dir);
	vm_vec_scale(&whack_dir, whack);

	// apply the whack
	ship_apply_whack(&whack_dir, hit_point, objp);
}

// return the amount of damage which should be applied to a ship. basically, filters friendly fire damage 
float beam_get_ship_damage(beam *b, object *objp)
{	
	// if the beam is on the same team as the object
	if ( (objp == NULL) || (b == NULL) ) {
		Int3();
		return 0.0f;
	}

	if ( (objp->type != OBJ_SHIP) || (objp->instance < 0) || (objp->instance >= MAX_SHIPS) ) {
		Int3();
		return 0.0f;
	}

	weapon_info *wip = &Weapon_info[b->weapon_info_index];

	float attenuation = 1.0f;

	if ((b->damage_threshold >= 0.0f) && (b->damage_threshold < 1.0f)) {
		float dist = vm_vec_dist(&b->last_shot, &b->last_start);
		float range = b->range;
		float atten_dist = range * b->damage_threshold;
		if ((range > dist) && (atten_dist < dist)) {
			attenuation = (dist - atten_dist) / (range - atten_dist);
		}
	}

	float damage = 0.0f;

	// same team. yikes
	if ( (b->team == Ships[objp->instance].team) && (wip->damage > The_mission.ai_profile->beam_friendly_damage_cap[Game_skill_level]) ) {
		damage = The_mission.ai_profile->beam_friendly_damage_cap[Game_skill_level] * attenuation;
	} else {
		// normal damage
		damage = Weapon_info[b->weapon_info_index].damage * attenuation;
	}

	return damage;
}

// if the beam is likely to tool a given target before its lifetime expires
int beam_will_tool_target(beam *b, object *objp)
{
	weapon_info *wip = &Weapon_info[b->weapon_info_index];
	float total_strength, damage_in_a_few_seconds, hp_limit, hp_pct;
	
	// sanity
	if(objp == NULL){
		return 0;
	}

	// if the object is not a ship, bail
	if(objp->type != OBJ_SHIP){
		return 0;
	}
	if((objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return 0;
	}
	
	ship *shipp = &Ships[objp->instance];
	total_strength = objp->hull_strength;

	if (shipp->armor_type_idx != -1) {
		if (Armor_types[shipp->armor_type_idx].GetPiercingType(wip->damage_type_idx) == SADTF_PIERCING_NONE) {
			return 0;
		}
		hp_limit = Armor_types[shipp->armor_type_idx].GetPiercingLimit(wip->damage_type_idx);
		if (hp_limit > 0.0f) {
			hp_pct = total_strength / shipp->ship_max_hull_strength;
			if (hp_limit >= hp_pct)
				return 1;
		}
	}

	// calculate total strength, factoring in shield
	if (!(wip->wi_flags2 & WIF2_PIERCE_SHIELDS))
		total_strength += shield_get_strength(objp);

	// if the beam is going to apply more damage in about 1 and a half than the ship can take
	damage_in_a_few_seconds = (TOOLTIME / (float)BEAM_DAMAGE_TIME) * wip->damage;
	return (damage_in_a_few_seconds > total_strength);
}

float beam_accuracy = 1.0f;
DCF(b_aim, "")
{
	dc_get_arg(ARG_FLOAT);
	beam_accuracy = Dc_arg_float;
}
DCF(beam_list, "")
{
	int idx;
	int b_count = 0;

	for(idx=0; idx<Num_weapon_types; idx++){
		if(Weapon_info[idx].wi_flags & WIF_BEAM){			
			b_count++;
			dc_printf("Beam %d : %s\n", b_count, Weapon_info[idx].name);
		}
	}
}
void beam_test(int whee)
{
	int s1, s2;
	object *orion, *fenris;
	ship_subsys *orion_turret, *fenris_turret, *fenris_radar, *orion_radar, *lookup;
	beam_fire_info f;

	nprintf(("General", "Running beam test\n"));

	// lookup some stuff 
	s1 = ship_name_lookup("GTD Orion 1");
	Assert(s1 >= 0);
	orion = &Objects[Ships[s1].objnum];
	s2 = ship_name_lookup("GTC Fenris 2");
	Assert(s2 >= 0);
	fenris = &Objects[Ships[s2].objnum];		

	// get beam weapons
	lookup = GET_FIRST(&Ships[s1].subsys_list);
	orion_turret = NULL;
	orion_radar = NULL;
	while(lookup != END_OF_LIST(&Ships[s1].subsys_list)){
		// turret		
		if((lookup->system_info->type == SUBSYSTEM_TURRET) && !subsystem_stricmp(lookup->system_info->subobj_name, "turret07")){
			orion_turret = lookup;			
		}

		// radar
		if(lookup->system_info->type == SUBSYSTEM_RADAR){
			orion_radar = lookup;
		}

		lookup = GET_NEXT(lookup);
	}
	Assert(orion_turret != NULL);
	Assert(orion_radar != NULL);
	lookup = GET_FIRST(&Ships[s2].subsys_list);
	fenris_turret = NULL;
	fenris_radar = NULL;
	while(lookup != END_OF_LIST(&Ships[s2].subsys_list)){
		// turret
		if((lookup->system_info->type == SUBSYSTEM_TURRET) && !subsystem_stricmp(lookup->system_info->subobj_name, "turret07")){
			fenris_turret = lookup;			
		}

		// radar
		if(lookup->system_info->type == SUBSYSTEM_RADAR){
			fenris_radar = lookup;
		}

		lookup = GET_NEXT(lookup);
	}
	Assert(fenris_turret != NULL);	
	Assert(fenris_radar != NULL);

	memset(&f, 0, sizeof(beam_fire_info));
	f.accuracy = beam_accuracy;
	f.beam_info_index = -1;
	f.beam_info_override = NULL;
	f.shooter = orion;
	f.target = fenris;
	f.target_subsys = fenris_turret;
	f.turret = orion_turret;

	// find the first beam
	int idx;	
	int beam_first = -1;
	int beam_count = 0;

	for(idx=0; idx<Num_weapon_types; idx++){
		if(Weapon_info[idx].wi_flags & WIF_BEAM){			
			beam_count++;
			if(beam_count > 1){
				beam_first = idx;
				break;
			}
		}
	}	
	if(beam_first < 0){
		return;
	}
	
	// maybe fire it, if its valid
	f.beam_info_index = beam_first + whee - 1;
	if(Weapon_info[f.beam_info_index].wi_flags & WIF_BEAM){
		HUD_printf("Firing %s\n", Weapon_info[f.beam_info_index].name);

		beam_fire(&f);
	}
}

void beam_test_new(int whee)
{
	int s1, s2, s3;
	object *orion, *fenris, *herc2, *herc3, *herc6, *alpha;
	ship_subsys *orion_turret, *fenris_turret, *fenris_radar, *orion_radar, *lookup;
	beam_fire_info f;

	nprintf(("General", "Running beam test\n"));

	// lookup some stuff 
	s1 = ship_name_lookup("GTD Orion 1");
	Assert(s1 >= 0);
	orion = &Objects[Ships[s1].objnum];
	s2 = ship_name_lookup("GTC Fenris 2");
	Assert(s2 >= 0);
	fenris = &Objects[Ships[s2].objnum];	
	s3 = ship_name_lookup("GTF Hercules 2");
	Assert(s3 >= 0);
	herc2 = &Objects[Ships[s3].objnum];
	s3 = ship_name_lookup("GTF Hercules 3");
	Assert(s3 >= 0);
	herc3 = &Objects[Ships[s3].objnum];
	s3 = ship_name_lookup("GTF Hercules 6");
	Assert(s3 >= 0);
	herc6 = &Objects[Ships[s3].objnum];
	s3 = ship_name_lookup("Alpha 1");
	Assert(s3 >= 0);
	alpha = &Objects[Ships[s3].objnum];	

	// get beam weapons
	lookup = GET_FIRST(&Ships[s1].subsys_list);
	orion_turret = NULL;
	orion_radar = NULL;
	while(lookup != END_OF_LIST(&Ships[s1].subsys_list)){
		// turret		
		if((lookup->system_info->type == SUBSYSTEM_TURRET) && !subsystem_stricmp(lookup->system_info->subobj_name, "turret07")){
			orion_turret = lookup;			
		}

		// radar
		if(lookup->system_info->type == SUBSYSTEM_RADAR){
			orion_radar = lookup;
		}

		lookup = GET_NEXT(lookup);
	}
	Assert(orion_turret != NULL);
	Assert(orion_radar != NULL);
	lookup = GET_FIRST(&Ships[s2].subsys_list);
	fenris_turret = NULL;
	fenris_radar = NULL;
	while(lookup != END_OF_LIST(&Ships[s2].subsys_list)){
		// turret
		if((lookup->system_info->type == SUBSYSTEM_TURRET) && !subsystem_stricmp(lookup->system_info->subobj_name, "turret03")){
			fenris_turret = lookup;			
		}

		// radar
		if(lookup->system_info->type == SUBSYSTEM_RADAR){
			fenris_radar = lookup;
		}

		lookup = GET_NEXT(lookup);
	}
	Assert(fenris_turret != NULL);	
	Assert(fenris_radar != NULL);

	memset(&f, 0, sizeof(beam_fire_info));
	f.accuracy = beam_accuracy;	
	f.beam_info_override = NULL;
	f.shooter = fenris;
	f.target = alpha;
	f.target_subsys = NULL;
	f.turret = fenris_turret;
	f.num_shots = 3;

	// find the first beam
	int idx;	
	int beam_first = -1;
	int beam_count = 0;

	for(idx=0; idx<Num_weapon_types; idx++){
		if(Weapon_info[idx].wi_flags & WIF_BEAM){
			beam_count++;
			if(beam_count > 1){
				beam_first = idx;
				break;
			}			
		}
	}	
	if(beam_first < 0){
		return;
	}
	
	// maybe fire it, if its valid
	f.beam_info_index = beam_first + whee - 1;
	if(Weapon_info[f.beam_info_index].wi_flags & WIF_BEAM){
		HUD_printf("Firing %s\n", Weapon_info[f.beam_info_index].name);
		beam_fire(&f);
	}
}
