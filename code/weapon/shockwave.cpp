/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "weapon/shockwave.h"
#include "asteroid/asteroid.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"
#include "io/timer.h"
#include "model/modelrender.h"
#include "nebula/neb.h"
#include "object/object.h"
#include "options/Option.h"
#include "render/3d.h"
#include "render/batching.h"
#include "ship/ship.h"
#include "ship/shiphit.h"
#include "weapon/weapon.h"

// -----------------------------------------------------------
// Module-wide globals
// -----------------------------------------------------------

static const char *Default_shockwave_2D_filename = "shockwave01";
static const char *Default_shockwave_3D_filename = "shockwave.pof";
static int Default_2D_shockwave_index = -1;
static int Default_shockwave_loaded = 0;

SCP_vector<shockwave_info> Shockwave_info;

shockwave Shockwaves[MAX_SHOCKWAVES];
shockwave Shockwave_list;
int Shockwave_inited = 0;

// -----------------------------------------------------------
// Function macros
// -----------------------------------------------------------
#define SW_INDEX(sw) (sw-Shockwaves)
	
// -----------------------------------------------------------
// Externals
// -----------------------------------------------------------
extern int Show_area_effect;
extern int Cmdline_enable_3d_shockwave;

static SCP_string shockwave_mode_display(bool mode) { return mode ? "3D" : "2D"; }

static bool Use_3D_shockwaves = true;

static auto Shockwave3DMode =
    options::OptionBuilder<bool>("Graphics.3DShockwaves", "Shockwaves", "The way shockwaves are displayed.")
        .category("Graphics")
        .display(shockwave_mode_display)
        .default_val(true)
        .bind_to_once(&Use_3D_shockwaves)
        .level(options::ExpertLevel::Advanced)
        .importance(66)
        .finish();

/**
 * Call to create a shockwave
 *
 * @param parent_objnum	object number of object spawning the shockwave
 * @param pos			vector specifing global position of shockwave center
 * @param sci			Shockwave info
 * @param flag			Flag settings
 * @param delay         delay in ms before the shockwave actually starts
 *
 * @return success		object number of shockwave
 * @return failure		-1
 */
int shockwave_create(int parent_objnum, vec3d* pos, shockwave_create_info* sci, int flag, int delay)
{
	int				i, objnum, real_parent;
	int				info_index = -1, model_id = -1;
	shockwave		*sw;
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
	sw->obj_sig_hitlist.clear();
	sw->shockwave_info_index = info_index;		// only one type for now... type could be passed is as a parameter
	sw->current_bitmap = -1;

	sw->blast_sound_id = sci->blast_sound_id;

	sw->time_elapsed=0.0f;
	sw->delay_stamp = delay;

	if (!sci->rot_defined) {
		sw->rot_angles.p = frand_range(0.0f, PI2);
		sw->rot_angles.b = frand_range(0.0f, PI2);
		sw->rot_angles.h = frand_range(0.0f, PI2);
	} else 
		sw->rot_angles = sci->rot_angles; // should just be 0,0,0

	sw->damage_type_idx = sci->damage_type_idx;

	sw->total_time = sw->outer_radius / sw->speed;

	if ( (parent_objnum != -1) && Objects[parent_objnum].type == OBJ_WEAPON ) {		// Goober5000: allow -1
		sw->weapon_info_index = Weapons[Objects[parent_objnum].instance].weapon_info_index;
	}
	else {		
		sw->weapon_info_index = -1;
	}

	orient = vmd_identity_matrix;
	vm_angles_2_matrix(&orient, &sw->rot_angles);
    flagset<Object::Object_Flags> tmp_flags;
	objnum = obj_create( OBJ_SHOCKWAVE, real_parent, i, &orient, &sw->pos, sw->outer_radius, tmp_flags + Object::Object_Flags::Renders, false );

	if ( objnum == -1 ){
		Int3();
	}

	sw->objnum = objnum;

	list_append(&Shockwave_list, sw);

	return objnum;
}

/**
 * Delete a shockwave
 *
 * @param objp		pointer to shockwave object
 */
void shockwave_delete(object *objp)
{
	Assertion(objp->type == OBJ_SHOCKWAVE, "shockwave_delete() called on an object with a type of %d instead of OBJ_SHOCKWAVE (%d); get a coder!\n", objp->type, OBJ_SHOCKWAVE);
	Assertion(objp->instance >= 0 && objp->instance < MAX_SHOCKWAVES, "shockwave_delete() called on an object with an invalid instance of %d (should be 0-%d); get a coder!\n", objp->instance, MAX_SHOCKWAVES - 1);

	Shockwaves[objp->instance].flags = 0;
	Shockwaves[objp->instance].objnum = -1;	
	list_remove(&Shockwave_list, &Shockwaves[objp->instance]);
}

/**
 * Delete whole linked list
 */
void shockwave_delete_all()
{
	shockwave	*sw, *next;
	
	sw = GET_FIRST(&Shockwave_list);
	while ( sw != &Shockwave_list ) {
		next = sw->next;
		Assert(sw->objnum != -1);
        Objects[sw->objnum].flags.set(Object::Object_Flags::Should_be_dead);
		sw = next;
	}
}

/**
 * Set the correct frame of animation for the shockwave
 */
void shockwave_set_framenum(int index)
{
	shockwave		*sw;
	shockwave_info	*si;

	Assertion( (index >= 0) && (index < MAX_SHOCKWAVES), "shockwave_set_framenum called with an index of %d (should be 0-%d); get a coder!\n", index, MAX_SHOCKWAVES - 1 );

	sw = &Shockwaves[index];
	si = &Shockwave_info[sw->shockwave_info_index];

	// skip this if it's a 3d shockwave since it won't have the maps managed here
	if (si->bitmap_id < 0)
		return;

	sw->current_bitmap = si->bitmap_id + shockwave_get_framenum(index, si->bitmap_id);
}

/**
 * Given a shockwave index and the animation id/handle return what the current frame num should be
 *
 * @note for direct use with 3d shockwaves & indirect use with 2d shockwaves
 */
int shockwave_get_framenum(const int sw_idx, const int ani_id)
{
	shockwave		*sw;

	if ( (sw_idx < 0) || (sw_idx >= MAX_SHOCKWAVES) ) {
		Int3();
		return 0;
	}

	sw = &Shockwaves[sw_idx];

	// ignore setting of OF_SHOULD_BE_DEAD, handled by shockwave_move
	return bm_get_anim_frame(ani_id, sw->time_elapsed, sw->total_time);
}

/**
 * Simulate a single shockwave.  If the shockwave radius exceeds outer_radius, then
 * delete the shockwave.
 *
 * @param shockwave_objp	object pointer that points to shockwave object
 * @param frametime			time to simulate shockwave
 */
void shockwave_move(object *shockwave_objp, float frametime)
{
	shockwave	*sw;
	object		*objp;
	float			blast,damage;

	Assertion(shockwave_objp->type == OBJ_SHOCKWAVE, "shockwave_move() called on an object of type %d instead of OBJ_SHOCKWAVE (%d); get a coder!\n", shockwave_objp->type, OBJ_SHOCKWAVE);
	Assertion(shockwave_objp->instance  >= 0 && shockwave_objp->instance < MAX_SHOCKWAVES, "shockwave_move() called on an object with an instance of %d (should be 0-%d); get a coder!\n", shockwave_objp->instance, MAX_SHOCKWAVES - 1);
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

	shockwave_set_framenum(shockwave_objp->instance);
		
	sw->radius += (frametime * sw->speed);
	if ( sw->radius > sw->outer_radius ) {
		sw->radius = sw->outer_radius;
        shockwave_objp->flags.set(Object::Object_Flags::Should_be_dead);
		return;
	}

	// blast ships and asteroids
	// And (some) weapons
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if (objp->flags[Object::Object_Flags::Should_be_dead])
			continue;
		if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID) && (objp->type != OBJ_WEAPON)) {
			continue;
		}

		if(objp->type == OBJ_WEAPON) {
			// only apply to missiles with hitpoints
			weapon_info* wip = &Weapon_info[Weapons[objp->instance].weapon_info_index];
			if (wip->weapon_hitpoints <= 0)
				continue;

			if (!Shockwaves_always_damage_bombs && !(wip->wi_flags[Weapon::Info_Flags::Takes_shockwave_damage] || (sw->weapon_info_index >= 0 && Weapon_info[sw->weapon_info_index].wi_flags[Weapon::Info_Flags::Ciws])))
				continue;
		}

	
		// don't blast no-collide or navbuoys
		if ( !objp->flags[Object::Object_Flags::Collides] || (objp->type == OBJ_SHIP && ship_get_SIF(objp->instance)[Ship::Info_Flags::Navbuoy]) ) {
			continue;
		}

		bool found_in_list = false;

		// only apply damage to an object once from a shockwave
		for (auto & comparison : sw->obj_sig_hitlist) {
			if ( (objp->signature == comparison.first) && (objp->type == comparison.second) ){
				found_in_list = true;
				break;
			}
		}

		if (found_in_list) {
			continue;
		}

		if ( weapon_area_calc_damage(objp, &sw->pos, sw->inner_radius, sw->outer_radius, sw->blast, sw->damage, &blast, &damage, sw->radius) == -1 ){
			continue;
		}

		weapon_info* wip = NULL;
		
		// okay, we have damage applied, record the object signature so we don't repeatedly apply damage 
		// but only add non-ships to the list if the Game_settings flag is set
		if (objp->type == OBJ_SHIP || Shockwaves_damage_all_obj_types_once) {
			sw->obj_sig_hitlist.emplace_back(objp->signature, objp->type);
		}

		switch(objp->type) {
		case OBJ_SHIP:
			// If we're doing an AoE Electronics shockwave, do the electronics stuff. -MageKing17
			if ( (sw->weapon_info_index >= 0) && (Weapon_info[sw->weapon_info_index].wi_flags[Weapon::Info_Flags::Aoe_Electronics]) && !(objp->flags[Object::Object_Flags::Invulnerable]) ) {
				weapon_do_electronics_effect(objp, &sw->pos, sw->weapon_info_index);
			}
			ship_apply_global_damage(objp, shockwave_objp, &sw->pos, damage, sw->damage_type_idx );
			weapon_area_apply_blast(NULL, objp, &sw->pos, blast, 1);
			break;
		case OBJ_ASTEROID:
			asteroid_hit(objp, NULL, NULL, damage);
			break;
		case OBJ_WEAPON:
			wip = &Weapon_info[Weapons[objp->instance].weapon_info_index];
			if (wip->armor_type_idx >= 0)
				damage = Armor_types[wip->armor_type_idx].GetDamage(damage, shockwave_get_damage_type_idx(shockwave_objp->instance),1.0f);

			objp->hull_strength -= damage;
			if (objp->hull_strength < 0.0f) {
				Weapons[objp->instance].lifeleft = 0.001f;
				Weapons[objp->instance].weapon_flags.set(Weapon::Weapon_Flags::Begun_detonation);
				Weapons[objp->instance].weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon);
			}
			break;
		default:
			Int3();
			break;
		}

		// If this shockwave hit the player, play shockwave impact sound
		if ( objp == Player_obj ) {
			float full_damage, vol_scale;
			if (sw->weapon_info_index >= 0) {
				full_damage = Weapon_info[sw->weapon_info_index].damage;
			} else {
				full_damage = sw->damage;
			}
			if (full_damage != 0.0f) {
				vol_scale = MAX(0.4f, damage/full_damage);
			} else {
				vol_scale = 1.0f;
			}
			if (sw->blast_sound_id.isValid()) {
				snd_play(gamesnd_get_game_sound(sw->blast_sound_id), 0.0f, vol_scale);
			}
		}

	}	// end for
}

/**
* Draw the shockwave identified by handle
*
* @param objp	pointer to shockwave object
* @param scene	the scene's draw list we're adding this to
*/
void shockwave_render(object *objp, model_draw_list *scene)
{
	shockwave		*sw;
	vertex			p;

	Assertion(objp->type == OBJ_SHOCKWAVE, "shockwave_render() called on an object of type %d instead of OBJ_SHOCKWAVE (%d); get a coder!\n", objp->type, OBJ_SHOCKWAVE);
	Assertion(objp->instance >= 0 && objp->instance < MAX_SHOCKWAVES, "shockwave_render() called on an object with an instance of %d (should be 0-%d); get a coder!\n", objp->instance, MAX_SHOCKWAVES - 1);

	sw = &Shockwaves[objp->instance];

	if( (sw->delay_stamp != -1) && !timestamp_elapsed(sw->delay_stamp)){
		return;
	}

	if ( (sw->current_bitmap < 0) && (sw->model_id < 0) )
		return;


	float alpha = 1.0f;
	if (The_mission.flags[Mission::Mission_Flags::Fullneb] && Neb_affects_weapons)
		alpha *= neb2_get_fog_visibility(&objp->pos, Neb2_fog_visibility_shockwave);

	if (sw->model_id > -1) {
		vec3d scale;
		scale.xyz.x = scale.xyz.y = scale.xyz.z = sw->radius / 50.0f;

		model_render_params render_info;

		render_info.set_warp_params(-1, 1.0f - (sw->radius/sw->outer_radius), scale);

		float dist = vm_vec_dist_quick( &sw->pos, &Eye_position );

		render_info.set_detail_level_lock((int)(dist / (sw->radius * 10.0f)));
		render_info.set_flags(MR_NO_LIGHTING | MR_NO_FOGGING | MR_NORMAL | MR_CENTER_ALPHA | MR_NO_CULL | MR_NO_BATCH);
		render_info.set_object_number(sw->objnum);

		model_render_queue( &render_info, scene, sw->model_id, &Objects[sw->objnum].orient, &sw->pos);

		if ( Gr_framebuffer_effects[FramebufferEffects::Shockwaves] && Default_2D_shockwave_index > -1) {
			g3_transfer_vertex(&p, &sw->pos);

			float intensity = ((sw->time_elapsed / sw->total_time) > 0.9f) ? (1.0f - (sw->time_elapsed / sw->total_time))*10.0f : 1.0f;
			batching_add_distortion_bitmap_rotated(Shockwave_info[Default_2D_shockwave_index].bitmap_id + shockwave_get_framenum(objp->instance, Shockwave_info[Default_2D_shockwave_index].bitmap_id), &p, fl_radians(sw->rot_angles.p), sw->radius, intensity);
		}
	} else {
		g3_transfer_vertex(&p, &sw->pos);

		if ( Gr_framebuffer_effects[FramebufferEffects::Shockwaves] ) {
			float intensity = ((sw->time_elapsed / sw->total_time) > 0.9f) ? (1.0f - (sw->time_elapsed / sw->total_time)) * 10.0f : 1.0f;
			batching_add_distortion_bitmap_rotated(sw->current_bitmap, &p, sw->rot_angles.p, sw->radius, intensity);
		}

		batching_add_volume_bitmap_rotated(sw->current_bitmap, &p, sw->rot_angles.p, sw->radius, alpha);
	}
}

/**
 * Call to load a shockwave, or add it and then load it
 */
int shockwave_load(const char *s_name, bool shock_3D)
{
	int s_index = -1;
	shockwave_info *si = NULL;

	Assert( s_name );

	// make sure that this is, or should be, valid
	if ( !VALID_FNAME(s_name) )
		return -1;

	for (auto it = Shockwave_info.cbegin(); it != Shockwave_info.cend(); ++it) {
		if ( !stricmp(it->filename, s_name) ) {
			s_index = (int)std::distance(Shockwave_info.cbegin(), it);
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
		si->bitmap_id = bm_load_animation( si->filename, &si->num_frames, &si->fps, nullptr, nullptr, true );

		if ( si->bitmap_id < 0 ) {
			Shockwave_info.pop_back();
			return -1;
		}
	}
	
	return s_index;
}

/**
 * Call once at the start of each level (mission)
 */
void shockwave_level_init()
{
	int i;

	if (!Using_in_game_options) {
		// If the new option system is not in use then use the command line
		Use_3D_shockwaves = Cmdline_enable_3d_shockwave != 0;
	}

	if ( !Default_shockwave_loaded ) {
		i = -1;
		
		// try and load in a 3d shockwave first if enabled
		// Goober5000 - check for existence of file before trying to load it
		// chief1983 - Spicious added this check for the command line option.  I've modified the hardcoded "shockwave.pof" that existed in the check 
		// 	to use the static name instead, and added a check to override the command line if a 2d default filename is not found
		//  Note - The 3d shockwave flag is forced on by TBP's flag as of rev 4983
		if ( Use_3D_shockwaves && cf_exists_full(Default_shockwave_3D_filename, CF_TYPE_MODELS) ) {
			mprintf(("SHOCKWAVE =>  Loading default shockwave model... \n"));

			i = shockwave_load( Default_shockwave_3D_filename, true );

			if (i >= 0)
				mprintf(("SHOCKWAVE =>  Default model load: SUCCEEDED!!\n"));
			else
				mprintf(("SHOCKWAVE =>  Default model load: FAILED!!  Falling back to 2D effect...\n"));
			Assertion(i <= 0, "Default 3D shockwave should be the first shockwave loaded, but instead got loaded into index %d; get a coder!\n", i);
		}

		// next, try the 2d shockwave effect, unless the 3d effect was loaded
		// chief1983 - added some messages similar to those for the 3d shockwave
		if (i < 0 || Gr_framebuffer_effects[FramebufferEffects::Shockwaves]) {
			mprintf(("SHOCKWAVE =>  Loading default shockwave animation... \n"));

			i = shockwave_load( Default_shockwave_2D_filename );

			if (i >= 0)
				mprintf(("SHOCKWAVE =>  Default animation load: SUCCEEDED!!\n"));
			else
				mprintf(("SHOCKWAVE =>  Default animation load: FAILED!!  Checking if 3d effect was already tried...\n"));
			Assertion(i <= 1, "Default 2D shockwave should be either the first or second shockwave loaded, but instead got loaded into index %d; get a coder!\n", i);
			Default_2D_shockwave_index = i;
		}
			
		// chief1983 - The first patch broke mods that don't provide a 2d shockwave or define a specific shockwave for each model/weapon (shame on them)
		// The next patch involved a direct copy of the attempt above, with an i < 0 check in place of the command line check.  I've taken that and modified it to 
		// spit out a more meaningful message.  Might as well not bother trying again if the command line option was checked as it should have tried the first time through
		if ( i < 0 && !Use_3D_shockwaves && cf_exists_full(Default_shockwave_3D_filename, CF_TYPE_MODELS) ) {
			mprintf(("SHOCKWAVE =>  Loading default shockwave model as last resort... \n"));

			i = shockwave_load( Default_shockwave_3D_filename, true );

			if (i >= 0)
				mprintf(("SHOCKWAVE =>  Default model load: SUCCEEDED!!\n"));
			else
				mprintf(("SHOCKWAVE =>  Default model load: FAILED!!  No effect loaded...\n"));
			Assertion(i <= 0, "Default 3D shockwave should be the first shockwave loaded, but instead got loaded into index %d; get a coder!\n", i);
		}

		if (i < 0)
			Error(LOCATION, "ERROR:  Unable to open neither 3D nor 2D default shockwaves!!");

		Default_shockwave_loaded = 1;
	} else {
		// have to make sure that the default 3D model is still valid and usable
		// the 2D shockwave shouldn't need anything like this
		if (Shockwave_info[0].model_id >= 0) {
			Assertion(!strcmp(Shockwave_info[0].filename, Default_shockwave_3D_filename), "Shockwave_info[0] should be the default shockwave, but somehow isn't.\nShockwave_info[0].filename = \"%s\"\nDefault_shockwave_3D_filename = \"%s\"\nGet a coder!\n", Shockwave_info[0].filename, Default_shockwave_3D_filename);
			Shockwave_info[0].model_id = model_load( Default_shockwave_3D_filename, 0, NULL );
		}
	}

	Assertion( !Shockwave_info.empty(), "Default shockwave claims to be loaded, but Shockwave_info vector is empty!");
	Assertion( ((Shockwave_info[0].bitmap_id >= 0) || (Shockwave_info[0].model_id >= 0)), "Default shockwave claims to be loaded, but has no bitmap or model; get a coder!\n" );

	list_init(&Shockwave_list);

	for ( i = 0; i < MAX_SHOCKWAVES; i++ ) {
		Shockwaves[i].flags = 0;
		Shockwaves[i].objnum = -1;
		Shockwaves[i].model_id = -1;
	}

	Shockwave_inited = 1;
}

/**
 * Call at the close of each level (mission)
 */
void shockwave_level_close()
{
	if ( !Shockwave_inited )
		return;

	shockwave_delete_all();
	
	Assertion( !Shockwave_info.empty(), "Shockwave_info is empty in shockwave_level_close() despite theoretically having been initialized correctly; get a coder!\n");

	// unload default shockwave, and erase all others
	auto it = Shockwave_info.begin();
	if (it->bitmap_id >= 0)
		bm_unload( it->bitmap_id );
	else if (it->model_id >= 0)
		model_page_out_textures( it->model_id );
	if (Default_2D_shockwave_index == 1) {
		// 3D shockwaves and framebuffer shockwaves are both enabled and the default 2D shockwave is in the next slot of the vector
		++it;
		Assertion(it->bitmap_id >= 0, "Default 2D shockwave was loaded but is somehow missing its bitmap; get a coder!\n");
		bm_unload(it->bitmap_id);
	}

	for (++it; it != Shockwave_info.end(); ++it) {
		if (it->bitmap_id >= 0)
			bm_release( it->bitmap_id );

		if (it->model_id >= 0)
			model_unload( it->model_id );
	}

	// if there's no extra shockwaves, this is still fine (erasing from end() to end() is both valid and a no-op)
	Shockwave_info.erase( Shockwave_info.begin() + (Default_2D_shockwave_index == 1 ? 2 : 1), Shockwave_info.end() );

	Shockwave_inited = 0;
}

/**
 * Simulate all shockwaves in Shockwave_list
 *
 * @param frametime		time for last frame in ms
 */
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

/**
 * Return the weapon_info_index field for a shockwave
 */
int shockwave_get_weapon_index(int index)
{
	Assertion( (index >= 0) && (index < MAX_SHOCKWAVES), "shockwave_get_weapon_index() called on an index of %d (should be 0-%d); get a coder!\n", index, MAX_SHOCKWAVES - 1 );
	return Shockwaves[index].weapon_info_index;
}

/**
 * Return the maximum radius for specified shockwave
 */
float shockwave_get_max_radius(int index)
{
	Assertion( (index >= 0) && (index < MAX_SHOCKWAVES), "shockwave_get_max_radius() called on an index of %d (should be 0-%d); get a coder!\n", index, MAX_SHOCKWAVES - 1 );
	return Shockwaves[index].outer_radius;
}

/**
 * Return the minimum radius for specified shockwave
 */
float shockwave_get_min_radius(int index)
{
	Assertion( (index >= 0) && (index < MAX_SHOCKWAVES), "shockwave_get_min_radius() called on an index of %d (should be 0-%d); get a coder!\n", index, MAX_SHOCKWAVES - 1 );
	return Shockwaves[index].inner_radius;
}

/**
 * Return the damage for specified shockwave
 */
float shockwave_get_damage(int index)
{
	Assertion( (index >= 0) && (index < MAX_SHOCKWAVES), "shockwave_get_damage() called on an index of %d (should be 0-%d); get a coder!\n", index, MAX_SHOCKWAVES - 1 );
	return Shockwaves[index].damage;
}

/**
 * Return the damage type for specified shockwave
 */
int shockwave_get_damage_type_idx(int index)
{
	Assertion( (index >= 0) && (index < MAX_SHOCKWAVES), "shockwave_get_damage_type_idx() called on an index of %d (should be 0-%d); get a coder!\n", index, MAX_SHOCKWAVES - 1 );
	return Shockwaves[index].damage_type_idx;
}

/**
 * Return the flags for specified shockwave
 */
int shockwave_get_flags(int index)
{
	Assertion( (index >= 0) && (index < MAX_SHOCKWAVES), "shockwave_get_flags() called on an index of %d (should be 0-%d); get a coder!\n", index, MAX_SHOCKWAVES - 1 );
	return Shockwaves[index].flags;
}

void shockwave_page_in()
{
	// load in shockwaves
	for (auto it = Shockwave_info.begin(); it != Shockwave_info.end(); ++it) {
		if (it->bitmap_id >= 0) {
			bm_page_in_texture( it->bitmap_id, it->num_frames );
		} else if (it->model_id >= 0) {
			// for a model we have to run model_load() on it again to make sure
			// that it's ref_count is sane for this mission
			int idx __UNUSED = model_load( it->filename, 0, NULL );
			Assertion( idx == it->model_id , "Shockwave_info[" SIZE_T_ARG "] got two different model_ids: %d and %d. Filename is \"%s\". Get a coder!\n", std::distance(Shockwave_info.begin(), it), idx, it->model_id, it->filename);

			model_page_in_textures( it->model_id );
		}
	}
}

void shockwave_create_info_init(shockwave_create_info *sci)
{ 
	sci->name[ 0 ] = '\0';
	sci->pof_name[ 0 ] = '\0';

	sci->inner_rad = sci->outer_rad = sci->damage = sci->blast = sci->speed = 0.0f;

	sci->rot_angles.p = sci->rot_angles.b = sci->rot_angles.h = 0.0f;
	sci->rot_defined = false;
	sci->damage_type_idx = sci->damage_type_idx_sav = -1;
	sci->damage_overidden = false;

	sci->blast_sound_id = GameSounds::SHOCKWAVE_IMPACT;
}

/**
 * Loads a shockwave in preparation for a mission
 */
void shockwave_create_info_load(shockwave_create_info *sci)
{
	int i = -1;

	// shockwave_load() will return -1 if the filename is "none" or "<none>"
	// checking for that case lets us handle a situation where a 2D shockwave
	// of "none" was specified and a valid 3D shockwave was specified

	if ( strlen(sci->name) )
		i = shockwave_load(sci->name, false);

	if ( (i < 0) && strlen(sci->pof_name) )
		shockwave_load(sci->pof_name, true);
}
