/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "weapon/shockwave.h"
#include "render/3d.h"
#include "weapon/weapon.h"
#include "ship/ship.h"
#include "io/timer.h"
#include "globalincs/linklist.h"
#include "ship/shiphit.h"
#include "gamesnd/gamesnd.h"
#include "asteroid/asteroid.h"
#include "object/object.h"


// -----------------------------------------------------------
// Data structures
// -----------------------------------------------------------

typedef struct shockwave_info
{
	char filename[MAX_FILENAME_LEN];
	int	bitmap_id;
	int model_id;
	int	num_frames;
	int	fps;

	shockwave_info()
		: num_frames( 0 ), fps( 0 )
	{ 
		filename[ 0 ] = '\0';
		bitmap_id = -1; 
		model_id = -1; 
	}
} shockwave_info;

typedef struct shockwave {
	shockwave	*next, *prev;
	int			flags;
	int			objnum;					// index into Objects[] for shockwave
	int			num_objs_hit;
	int			obj_sig_hitlist[SW_MAX_OBJS_HIT];
	float		speed, radius;
	float		inner_radius, outer_radius, damage;
	int			weapon_info_index;	// -1 if shockwave not caused by weapon	
	int			damage_type_idx;			//What type of damage this shockwave does to armor
	vec3d		pos;
	float		blast;					// amount of blast to apply
	int			next_blast;				// timestamp for when to apply next blast damage
	int			shockwave_info_index;
	int			current_bitmap;
	float		time_elapsed;			// in seconds
	float		total_time;				// total lifetime of animation in seconds
	int			delay_stamp;			// for delayed shockwaves
	angles		rot_angles;
	int			model_id;
} shockwave;

// -----------------------------------------------------------
// Module-wide globals
// -----------------------------------------------------------

static char *Default_shockwave_2D_filename = "shockwave01";
static char *Default_shockwave_3D_filename = "shockwave.pof";
static int Default_shockwave_loaded = 0;

SCP_vector<shockwave_info> Shockwave_info;

shockwave Shockwaves[MAX_SHOCKWAVES];
shockwave Shockwave_list;
int Shockwave_inited = 0;

// load a shockwave
int shockwave_load(char *s_name, bool shock_3D = false);

// -----------------------------------------------------------
// Function macros
// -----------------------------------------------------------
#define SW_INDEX(sw) (sw-Shockwaves)
	
// -----------------------------------------------------------
// Externals
// -----------------------------------------------------------
extern int Show_area_effect;
extern int Cmdline_nohtl;
extern int Cmdline_enable_3d_shockwave;


// ------------------------------------------------------------------------------------
// shockwave_create()
//
// Call to create a shockwave
//
//	input:	parent_objnum	=> object number of object spawning the shockwave
//				pos				=>	vector specifing global position of shockwave center
//				speed				=>	speed at which shockwave expands (m/s)
//				inner_radius	=>	radius at which damage applied is at maximum
//				outer_radius	=> damage decreases linearly to zero from inner_radius to
//										outer_radius.  Outside outer_radius, damage is 0.
//				damage			=>	the maximum damage (ie within inner_radius)
//				blast				=> the maximux blast (within inner_radius)
//				sw_flag			=> indicates whether shockwave is from weapon or ship explosion
//				delay          => delay in ms before the shockwave actually starts
//
//	return:	success			=>	object number of shockwave
//				failure			=>	-1
//
// Goober5000 - now parent_objnum can be allowed to be -1
int shockwave_create(int parent_objnum, vec3d *pos, shockwave_create_info *sci, int flag, int delay)
{
	int				i, objnum, real_parent;
	int				info_index = -1, model_id = -1;
	shockwave		*sw;
//	shockwave_info	*si;
	matrix			orient;

 	for (i = 0; i < MAX_SHOCKWAVES; i++) {
		if ( !(Shockwaves[i].flags & SW_USED) )
			break;
	}

	if (i == MAX_SHOCKWAVES)
		return -1;

	// try 2D shockwave first, then fall back to 3D, then fall back to default of either
	// this should be pretty fool-proof and allow quick change between 2D and 3D effects
	if ( strlen(sci->name) )
		info_index = shockwave_load(sci->name, false);

	if ( (info_index < 0) && strlen(sci->pof_name) )
		info_index = shockwave_load(sci->pof_name, true);

	if (info_index < 0) {
		if ( (Shockwave_info[0].bitmap_id >= 0) || (Shockwave_info[0].model_id >= 0) ) {
			info_index = 0;
			model_id = Shockwave_info[0].model_id;
		} else {
			// crap, just bail
			return -1;
		}
	} else {
		model_id = Shockwave_info[info_index].model_id;
	}

	// real_parent is the guy who caused this shockwave to happen
	if (parent_objnum == -1) {	// Goober5000
		real_parent = -1;
	} else if ( Objects[parent_objnum].type == OBJ_WEAPON ){
		real_parent = Objects[parent_objnum].parent;
	} else {
		real_parent = parent_objnum;
	}

	sw = &Shockwaves[i];

	sw->model_id = model_id;
	sw->flags = (SW_USED | flag);
	sw->speed = sci->speed;
	sw->inner_radius = sci->inner_rad;
	sw->outer_radius = sci->outer_rad;
	sw->damage = sci->damage;
	sw->blast = sci->blast;
	sw->radius = 1.0f;
	sw->pos = *pos;
	sw->num_objs_hit = 0;
	sw->shockwave_info_index = info_index;		// only one type for now... type could be passed is as a parameter
	sw->current_bitmap = -1;

	sw->time_elapsed=0.0f;
	sw->delay_stamp = delay;

	sw->rot_angles = sci->rot_angles;
	sw->damage_type_idx = sci->damage_type_idx;

//	si = &Shockwave_info[sw->shockwave_info_index];
//	sw->total_time = i2fl(si->num_frames) / si->fps;	// in seconds
	sw->total_time = sw->outer_radius / sw->speed;

	if ( (parent_objnum != -1) && Objects[parent_objnum].type == OBJ_WEAPON ) {		// Goober5000: allow -1
		sw->weapon_info_index = Weapons[Objects[parent_objnum].instance].weapon_info_index;
	}
	else {		
		sw->weapon_info_index = -1;
	}

	orient = vmd_identity_matrix;
	vm_angles_2_matrix(&orient, &sw->rot_angles);
//	angles a;
//	a.p = sw->rot_angle*(PI/180);
//	a.b = frand_range(0.0f, PI2);
//	a.h = frand_range(0.0f, PI2);
//	vm_angles_2_matrix(&orient, &a);
	objnum = obj_create( OBJ_SHOCKWAVE, real_parent, i, &orient, &sw->pos, sw->outer_radius, OF_RENDERS );

	if ( objnum == -1 ){
		Int3();
	}

	sw->objnum = objnum;

	list_append(&Shockwave_list, sw);

	return objnum;
}

// ------------------------------------------------------------------------------------
// shockwave_delete()
//
// Delete a shockwave
//
//	input:	object *objp	=>		pointer to shockwave object
//
void shockwave_delete(object *objp)
{
	Assert(objp->type == OBJ_SHOCKWAVE);
	Assert(objp->instance >= 0 && objp->instance < MAX_SHOCKWAVES);

	Shockwaves[objp->instance].flags = 0;
	Shockwaves[objp->instance].objnum = -1;	
	list_remove(&Shockwave_list, &Shockwaves[objp->instance]);
}

// ------------------------------------------------------------------------------------
// shockwave_delete_all()
//
//
void shockwave_delete_all()
{
	shockwave	*sw, *next;
	
	sw = GET_FIRST(&Shockwave_list);
	while ( sw != &Shockwave_list ) {
		next = sw->next;
		Assert(sw->objnum != -1);
		Objects[sw->objnum].flags |= OF_SHOULD_BE_DEAD;
		sw = next;
	}
}

// Set the correct frame of animation for the shockwave
void shockwave_set_framenum(int index)
{
	int				framenum;
	shockwave		*sw;
	shockwave_info	*si;

	Assert( (index >= 0) && (index < MAX_SHOCKWAVES) );

	sw = &Shockwaves[index];
	si = &Shockwave_info[sw->shockwave_info_index];

	// skip this if it's a 3d shockwave since it won't have the maps managed here
	if (si->bitmap_id < 0)
		return;

	framenum = fl2i(sw->time_elapsed / sw->total_time * si->num_frames + 0.5);

	// ensure we don't go past the number of frames of animation
	if ( framenum > (si->num_frames-1) ) {
		framenum = (si->num_frames-1);
		Objects[sw->objnum].flags |= OF_SHOULD_BE_DEAD;
	}

	if ( framenum < 0 ) {
		framenum = 0;
	}

	sw->current_bitmap = si->bitmap_id + framenum;
}

// given a shockwave index and the number of frames in an animation return what
// the current frame # should be  (for use with 3d shockwaves)
int shockwave_get_framenum(int index, int num_frames)
{
	int				framenum;
	shockwave		*sw;

	if ( (index < 0) || (index >= MAX_SHOCKWAVES) ) {
		Int3();
		return 0;
	}

	sw = &Shockwaves[index];

	framenum = fl2i(sw->time_elapsed / sw->total_time * num_frames + 0.5);

	// ensure we don't go past the number of frames of animation
	if ( framenum > (num_frames-1) ) {
		framenum = (num_frames-1);
		Objects[sw->objnum].flags |= OF_SHOULD_BE_DEAD;
	}

	if ( framenum < 0 ) {
		framenum = 0;
	}

	return framenum;
}
// ------------------------------------------------------------------------------------
// shockwave_move()
//
//	Simulate a single shockwave.  If the shockwave radius exceeds outer_radius, then
// delete the shockwave.
//
//	input:		ojbp			=>		object pointer that points to shockwave object
//					frametime	=>		time to simulate shockwave
//
void shockwave_move(object *shockwave_objp, float frametime)
{
	shockwave	*sw;
	object		*objp;
	float			blast,damage;
	int			i;

	Assert(shockwave_objp->type == OBJ_SHOCKWAVE);
	Assert(shockwave_objp->instance  >= 0 && shockwave_objp->instance < MAX_SHOCKWAVES);
	sw = &Shockwaves[shockwave_objp->instance];

	// if the shockwave has a delay on it
	if(sw->delay_stamp != -1){
		if(timestamp_elapsed(sw->delay_stamp)){
			sw->delay_stamp = -1;
		} else {
			return;
		}
	}

	sw->time_elapsed += frametime;
/*
	if ( sw->time_elapsed > sw->total_time ) {
		shockwave_objp->flags |= OF_SHOULD_BE_DEAD;
	}
*/

	shockwave_set_framenum(shockwave_objp->instance);
		
	sw->radius += (frametime * sw->speed);
	if ( sw->radius > sw->outer_radius ) {
		sw->radius = sw->outer_radius;
		shockwave_objp->flags |= OF_SHOULD_BE_DEAD;
		return;
	}

	// blast ships and asteroids
	// And (some) weapons
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID) && (objp->type != OBJ_WEAPON)) {
			continue;
		}

		if ( objp->type == OBJ_WEAPON ) {
			// only apply to missiles with hitpoints
			weapon_info* wip = &Weapon_info[Weapons[objp->instance].weapon_info_index];
			if (wip->weapon_hitpoints <= 0 || !(wip->wi_flags2 & WIF2_TAKES_SHOCKWAVE_DAMAGE))
				continue;
		}

	
		if ( objp->type == OBJ_SHIP ) {
			// don't blast navbuoys
			if ( ship_get_SIF(objp->instance) & SIF_NAVBUOY ) {
				continue;
			}
		}

		// only apply damage to a ship once from a shockwave
		for ( i = 0; i < sw->num_objs_hit; i++ ) {
			if ( objp->signature == sw->obj_sig_hitlist[i] ){
				break;
			}
		}

		if ( i < sw->num_objs_hit ){
			continue;
		}

		if ( weapon_area_calc_damage(objp, &sw->pos, sw->inner_radius, sw->outer_radius, sw->blast, sw->damage, &blast, &damage, sw->radius) == -1 ){
			continue;
		}

		// okay, we have damage applied, record the object signature so we don't repeatedly apply damage
		Assert(sw->num_objs_hit < SW_MAX_OBJS_HIT);
		if ( sw->num_objs_hit >= SW_MAX_OBJS_HIT) {
			sw->num_objs_hit--;
		}

		switch(objp->type) {
		case OBJ_SHIP:
			sw->obj_sig_hitlist[sw->num_objs_hit++] = objp->signature;
			ship_apply_global_damage(objp, shockwave_objp, &sw->pos, damage );
			weapon_area_apply_blast(NULL, objp, &sw->pos, blast, 1);
			break;
		case OBJ_ASTEROID:
			asteroid_hit(objp, NULL, NULL, damage);
			break;
		case OBJ_WEAPON:
			objp->hull_strength -= damage;
			if (objp->hull_strength < 0.0f) {
				Weapons[objp->instance].lifeleft = 0.01f;
				Weapons[objp->instance].weapon_flags |= WF_DESTROYED_BY_WEAPON;
			}
			break;
		default:
			Int3();
			break;
		}


		// If this shockwave hit the player, play shockwave impact sound
		if ( objp == Player_obj ) {
			snd_play( &Snds[SND_SHOCKWAVE_IMPACT], 0.0f, MAX(0.4f, damage/Weapon_info[sw->weapon_info_index].damage) );
		}

	}	// end for
}

// ------------------------------------------------------------------------------------
// shockwave_render()
//
//	Draw the shockwave identified by handle
//
//	input:	objp	=>		pointer to shockwave object
//
void shockwave_render(object *objp)
{
	shockwave		*sw;
	shockwave_info	*si;
	vertex			p;

	Assert(objp->type == OBJ_SHOCKWAVE);
	Assert(objp->instance >= 0 && objp->instance < MAX_SHOCKWAVES);

	sw = &Shockwaves[objp->instance];
	si = &Shockwave_info[sw->shockwave_info_index];

	if( (sw->delay_stamp != -1) && !timestamp_elapsed(sw->delay_stamp)){
		return;
	}

	if ( (sw->current_bitmap < 0) && (sw->model_id < 0) )
		return;

	// turn off fogging
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

	if (sw->model_id > -1) {
		float model_Interp_scale_xyz = sw->radius / 50.0f;

		model_set_warp_globals( model_Interp_scale_xyz, model_Interp_scale_xyz, model_Interp_scale_xyz, -1, 1.0f - (sw->radius/sw->outer_radius) );
		
		float dist = vm_vec_dist_quick( &sw->pos, &Eye_position );

		model_set_detail_level((int)(dist / (sw->radius * 10.0f)));
		model_render( sw->model_id, &Objects[sw->objnum].orient, &sw->pos, MR_NO_LIGHTING | MR_NO_FOGGING | MR_NORMAL | MR_CENTER_ALPHA | MR_NO_CULL, sw->objnum);

		model_set_warp_globals();
	}else{
		if (!Cmdline_nohtl) {
			g3_transfer_vertex(&p, &sw->pos);
		} else {
			g3_rotate_vertex(&p, &sw->pos);
		}
	
		gr_set_bitmap(sw->current_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.3f );
		g3_draw_rotated_bitmap(&p, fl_radian(sw->rot_angles.p), sw->radius, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);	
	}
}

// ------------------------------------------------------------------------------------
// shockwave_load()
//
// Call to load a shockwave, or add it and then load it
//
int shockwave_load(char *s_name, bool shock_3D)
{
	uint i;
	int s_index = -1;
	shockwave_info *si = NULL;

	Assert( s_name );

	// make sure that this is, or should be, valid
	if ( !VALID_FNAME(s_name) )
		return -1;

	for (i = 0; i < Shockwave_info.size(); i++) {
		if ( !stricmp(Shockwave_info[i].filename, s_name) ) {
			s_index = i;
			break;
		}
	}

	if (s_index < 0) {
		shockwave_info si_tmp;
	
		strcpy_s(si_tmp.filename, s_name);

		Shockwave_info.push_back( si_tmp );
		s_index = (int)(Shockwave_info.size() - 1);
	}

	Assert( s_index >= 0 );
	si = &Shockwave_info[s_index];

	// make sure to only try loading the shockwave once
	if ( (si->bitmap_id >= 0) || (si->model_id >= 0) )
		return s_index;

	if (shock_3D) {
		si->model_id = model_load( si->filename, 0, NULL );

		if ( si->model_id < 0 ) {
			Shockwave_info.pop_back();
			return -1;
		}
	} else {
		si->bitmap_id = bm_load_animation( si->filename, &si->num_frames, &si->fps, NULL, 1 );

		if ( si->bitmap_id < 0 ) {
			Shockwave_info.pop_back();
			return -1;
		}
	}
	
	return s_index;
}

// ------------------------------------------------------------------------------------
// shockwave_init()
//
// Call once at the start of each level (mission)
//
void shockwave_level_init()
{
	int i;	

	if ( !Default_shockwave_loaded ) {
		i = -1;
		
		// try and load in a 3d shockwave first if enabled
		// Goober5000 - check for existence of file before trying to load it
		// chief1983 - Spicious added this check for the command line option.  I've modified the hardcoded "shockwave.pof" that existed in the check 
		// 	to use the static name instead, and added a check to override the command line if a 2d default filename is not found
		//  Note - The 3d shockwave flag is forced on by TBP's flag as of rev 4983
		if ( Cmdline_enable_3d_shockwave && cf_exists_full(Default_shockwave_3D_filename, CF_TYPE_MODELS) ) {
			mprintf(("SHOCKWAVE =>  Loading default shockwave model... \n"));

			i = shockwave_load( Default_shockwave_3D_filename, true );

			if (i >= 0)
				mprintf(("SHOCKWAVE =>  Default model load: SUCCEEDED!!\n"));
			else
				mprintf(("SHOCKWAVE =>  Default model load: FAILED!!  Falling back to 2D effect...\n"));
		}

		// next, try the 2d shockwave effect, unless the 3d effect was loaded
		// chief1983 - added some messages similar to those for the 3d shockwave
		if (i < 0) {
			mprintf(("SHOCKWAVE =>  Loading default shockwave animation... \n"));

			i = shockwave_load( Default_shockwave_2D_filename );

			if (i >= 0)
				mprintf(("SHOCKWAVE =>  Default animation load: SUCCEEDED!!\n"));
			else
				mprintf(("SHOCKWAVE =>  Default animation load: FAILED!!  Checking if 3d effect was already tried...\n"));
		}
			
		// chief1983 - The first patch broke mods that don't provide a 2d shockwave or define a specific shockwave for each model/weapon (shame on them)
		// The next patch involved a direct copy of the attempt above, with an i < 0 check in place of the command line check.  I've taken that and modified it to 
		// spit out a more meaningful message.  Might as well not bother trying again if the command line option was checked as it should have tried the first time through
		if ( i < 0 && !Cmdline_enable_3d_shockwave && cf_exists_full(Default_shockwave_3D_filename, CF_TYPE_MODELS) ) {
			mprintf(("SHOCKWAVE =>  Loading default shockwave model as last resort... \n"));

			i = shockwave_load( Default_shockwave_3D_filename, true );

			if (i >= 0)
				mprintf(("SHOCKWAVE =>  Default model load: SUCCEEDED!!\n"));
			else
				mprintf(("SHOCKWAVE =>  Default model load: FAILED!!  No effect loaded...\n"));
		}

		if (i < 0)
			Error(LOCATION, "ERROR:  Unable to open neither 3D nor 2D default shockwaves!!");

		Default_shockwave_loaded = 1;
	} else {
		// have to make sure that the default 3D model is still valid and usable
		// the 2D shockwave shouldn't need anything like this
		if (Shockwave_info[0].model_id >= 0)
			Shockwave_info[0].model_id = model_load( Default_shockwave_3D_filename, 0, NULL );
	}

	Assert( ((Shockwave_info[0].bitmap_id >= 0) || (Shockwave_info[0].model_id >= 0)) );

	list_init(&Shockwave_list);

	for ( i = 0; i < MAX_SHOCKWAVES; i++ ) {
		Shockwaves[i].flags = 0;
		Shockwaves[i].objnum = -1;
		Shockwaves[i].model_id = -1;
	}

	Shockwave_inited = 1;
}

// ------------------------------------------------------------------------------------
// shockwave_level_close()
//
//  Call at the close of each level (mission)
//
void shockwave_level_close()
{
	if ( !Shockwave_inited )
		return;

	shockwave_delete_all();
	
	uint i;

	// unload default shockwave, and erase all others
	for (i = 0; i < Shockwave_info.size(); i++) {
		if ( !i ) {
			if (Shockwave_info[i].bitmap_id >= 0)
				bm_unload( Shockwave_info[i].bitmap_id );
			else if (Shockwave_info[i].model_id >= 0)
				model_page_out_textures( Shockwave_info[i].model_id );

			continue;
		}

		if (Shockwave_info[i].bitmap_id >= 0)
			bm_release( Shockwave_info[i].bitmap_id );

		if (Shockwave_info[i].model_id >= 0)
			model_unload( Shockwave_info[i].model_id );

		Shockwave_info.erase( Shockwave_info.begin() + i );
	}

	Shockwave_inited = 0;
}

// ------------------------------------------------------------------------------------
// shockwave_close()
//
//	Called at game-shutdown to 
//
void shockwave_close()
{
}

// ------------------------------------------------------------------------------------
// shockwave_move_all()
//
//	Simulate all shockwaves in Shockwave_list
//
//	input:	frametime	=>		time for last frame in ms
//
void shockwave_move_all(float frametime)
{
	shockwave	*sw, *next;
	
	sw = GET_FIRST(&Shockwave_list);
	while ( sw != &Shockwave_list ) {
		next = sw->next;
		Assert(sw->objnum != -1);
		shockwave_move(&Objects[sw->objnum], frametime);
		sw = next;
	}
}

// ------------------------------------------------------------------------------------
// shockwave_render_all()
//
//
void shockwave_render_all()
{
	shockwave	*sw, *next;

	sw = GET_FIRST(&Shockwave_list);
	while ( sw != &Shockwave_list ) {
		next = sw->next;
		Assert(sw->objnum != -1);
		shockwave_render(&Objects[sw->objnum]);
		sw = next;
	}
}

// return the weapon_info_index field for a shockwave
int shockwave_get_weapon_index(int index)
{
	Assert( (index >= 0) && (index < MAX_SHOCKWAVES) );
	return Shockwaves[index].weapon_info_index;
}

// return the maximum radius for specified shockwave
float shockwave_get_max_radius(int index)
{
	Assert( (index >= 0) && (index < MAX_SHOCKWAVES) );
	return Shockwaves[index].outer_radius;
}

float shockwave_get_min_radius(int index)
{
	Assert( (index >= 0) && (index < MAX_SHOCKWAVES) );
	return Shockwaves[index].inner_radius;
}

float shockwave_get_damage(int index)
{
	Assert( (index >= 0) && (index < MAX_SHOCKWAVES) );
	return Shockwaves[index].damage;
}

int shockwave_get_damage_type_idx(int index)
{
	Assert( (index >= 0) && (index < MAX_SHOCKWAVES) );
	return Shockwaves[index].damage_type_idx;
}

int shockwave_get_flags(int index)
{
	Assert( (index >= 0) && (index < MAX_SHOCKWAVES) );
	return Shockwaves[index].flags;
}

void shockwave_page_in()
{
	uint i;

	// load in shockwaves
	for (i = 0; i < Shockwave_info.size(); i++) {
		if (Shockwave_info[i].bitmap_id >= 0) {
			bm_page_in_texture( Shockwave_info[i].bitmap_id, Shockwave_info[i].num_frames );
		} else if (Shockwave_info[i].model_id >= 0) {
			// for a model we have to run model_load() on it again to make sure
			// that it's ref_count is sane for this mission
			int idx = model_load( Shockwave_info[i].filename, 0, NULL );
			Assert( idx == Shockwave_info[i].model_id );

			model_page_in_textures( Shockwave_info[i].model_id );
		}
	}
}

// Loads a shockwave in preparation for a mission
void shockwave_create_info::load()
{
	int i = -1;

	// shockwave_load() will return -1 if the filename is "none" or "<none>"
	// checking for that case lets us handle a situation where a 2D shockwave
	// of "none" was specified and a valid 3D shockwave was specified

	if ( strlen(name) )
		i = shockwave_load(name, false);

	if ( (i < 0) && strlen(pof_name) )
		shockwave_load(pof_name, true);
}
