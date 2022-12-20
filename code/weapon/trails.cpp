/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "cmdline/cmdline.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "io/timer.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "render/3d.h" 
#include "ship/ship.h"
#include "tracing/tracing.h"
#include "weapon/trails.h"
#include "render/batching.h"
#include "tl/optional.hpp"
#include "globalincs/pool.h"

SCP_Pool<trail> Trails;

// Reset everything between levels
void trail_level_init()
{
	Trails.reset();
}

void trail_level_close()
{
	Trails.reset();
}

//returns the index of a free trail
//returns null index if no free trails
pool_index trail_create(trail_info *info)
{
	// standalone server should never create trails
	// No trails at slot 0
	if((Game_mode & GM_STANDALONE_SERVER) || !Detail.weapon_extras)
		return pool_index();

	// Make a new trail
	auto trail_index = Trails.get_new();
	if(trail_index == tl::nullopt){
		return pool_index();
	}
	pool_index i = trail_index.value();
	trail *trailp = &Trails[i];
	// Init the trail data
	trailp->info = *info;
	trailp->tail = 0;
	trailp->head = 0;
	trailp->object_died = false;
	trailp->trail_stamp = timestamp(trailp->info.stamp);

	return i;
}

trail *trail_find(pool_index const &traili){
	if(Trails.check(traili))
		return &Trails[traili];
	return nullptr;
}
// output top and bottom vectors
// fvec == forward vector (eye viewpoint basically. in world coords)
// pos == world coordinate of the point we're calculating "around"
// w == width of the diff between top and bottom around pos
void trail_calc_facing_pts( vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w )
{
	vec3d uvec, rvec;

	vm_vec_sub( &rvec, &Eye_position, pos );
	if (!IS_VEC_NULL(&rvec))
		vm_vec_normalize( &rvec );

	vm_vec_cross(&uvec,fvec,&rvec);
	if (!IS_VEC_NULL(&uvec))
		vm_vec_normalize(&uvec);

	// Scale the trails so that they are always at least some configured amount of pixels across.
	w = model_render_get_diameter_clamped_to_min_pixel_size(pos, w, Min_pixel_size_trail);

	vm_vec_scale_add( top, pos, &uvec, w * 0.5f );
	vm_vec_scale_add( bot, pos, &uvec, -w * 0.5f );
}

// trail is on ship
int trail_is_on_ship(pool_index trail_i, ship *shipp)
{
	if(trail_i.null())
		return 0;

	for(int idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		if(shipp->trail_idx[idx] == trail_i){
			return 1;
		}
	}

	// nope
	return 0;
}

void trail_render(trail * trailp, pool_index const & trail_i)
{
	int sections[NUM_TRAIL_SECTIONS];
	int num_sections = 0;

	if (trailp->tail == trailp->head)
		return;

	// if this trail is on the player ship, and he's in any padlock view except rear view, don't draw	
	if ( (Player_ship != nullptr) && trail_is_on_ship(trail_i, Player_ship) &&
		(Viewer_mode & (VM_PADLOCK_UP | VM_PADLOCK_LEFT | VM_PADLOCK_RIGHT)) )
	{
		return;
	}

	trail_info *ti	= &trailp->info;

	int n = trailp->tail;

	do	{
		n--;

		if (n < 0)
			n = NUM_TRAIL_SECTIONS-1;

		if (trailp->val[n] > 1.0f)
			break;

		sections[num_sections++] = n;
	} while ( n != trailp->head );

	if (num_sections <= 1)
		return;

	Assertion(ti->texture.bitmap_id != -1, "Weapon trail %s could not be loaded", ti->texture.filename); // We can leave this as an assert, but tell them how to fix it. --Chief

	float w_size = (ti->w_end - ti->w_start);
	float a_size = (ti->a_end - ti->a_start);
	int num_faded_sections = ti->n_fade_out_sections;

	vec3d prev_top, prev_bot; vm_vec_zero(&prev_top); vm_vec_zero(&prev_bot);
	float prev_U = 0;
	ubyte prev_alpha = 0;
	for (int i = 0; i < num_sections; i++) {
		n = sections[i];

		// first get the alpha
		float w = trailp->val[n] * w_size + ti->w_start;

		float fade = trailp->val[n];
		
		if (trailp->info.a_decay_exponent != 1.0f)
			fade = powf(trailp->val[n], trailp->info.a_decay_exponent);

		ubyte current_alpha = 0;
		if ((num_faded_sections > 0) && (i < num_faded_sections)) {
			float init_fade_out = ((float)i) / (float)num_faded_sections;
			current_alpha = (ubyte)fl2i((fade * a_size + ti->a_start) * 255.0f * init_fade_out * init_fade_out);
		} else {
			current_alpha = (ubyte)fl2i((fade * a_size + ti->a_start) * 255.0f);
		}

		if (The_mission.flags[Mission::Mission_Flags::Fullneb] && Neb_affects_weapons)
			current_alpha = (ubyte)(current_alpha * neb2_get_fog_visibility(&trailp->pos[n], Neb2_fog_visibility_trail));

		// get the direction of the trail
		vec3d trail_direction;
		if (i == 0) {
			// first point, direction is directly to the next trail point
			vm_vec_sub(&trail_direction, &trailp->pos[n], &trailp->pos[sections[i+1]]);
		} else if (i == num_sections - 1) {
			// last point, direction is directly to the previous trail point
			vm_vec_sub(&trail_direction, &trailp->pos[sections[i-1]], &trailp->pos[n]);
		} else {
			// direction is the average between the next and previous directions
			vec3d forward, backward;
			vm_vec_sub(&backward, &trailp->pos[sections[i-1]], &trailp->pos[n]);
			vm_vec_sub(&forward, &trailp->pos[n], &trailp->pos[sections[i+1]]);
			vm_vec_normalize(&backward);
			if (!vm_maybe_normalize(&forward, &forward)) {
				// ok weird edge case that can happen
				// we are likely the the 2nd trail point but the first trail point is right on top of us
				// so just use the backward direction to avoid that degenerate forward direction
				trail_direction = backward;
			} else {
				vm_vec_avg(&trail_direction, &forward, &backward);
			}
		}
		vm_vec_normalize_safe(&trail_direction);

		
		float current_U = i2fl(n) / trailp->info.texture_stretch;
		vec3d current_top, current_bot;
		trail_calc_facing_pts(&current_top, &current_bot, &trail_direction, &trailp->pos[n], w);

		if (i > 0) {
			if (i == num_sections-1) {
				// Last one...
				vertex verts[3];
				verts[0].r = verts[0].g = verts[0].b = verts[0].a = prev_alpha;
				verts[1].r = verts[1].g = verts[1].b = verts[1].a = prev_alpha;
				verts[2].r = verts[2].g = verts[2].b = verts[2].a = current_alpha;

				vec3d center;
				vm_vec_avg(&center, &current_top, &current_bot);

				verts[0].world = prev_top;
				verts[1].world = prev_bot;
				verts[2].world = center;

				verts[0].texture_position.u = prev_U;
				verts[1].texture_position.u = prev_U;
				verts[2].texture_position.u = current_U;

				verts[0].texture_position.v = 0.0f;
				verts[1].texture_position.v = 1.0f;
				verts[2].texture_position.v = 0.5f;

				batching_add_tri(ti->texture.bitmap_id, verts);
			} else {
				vertex verts[4];
				verts[0].r = verts[0].g = verts[0].b = verts[0].a = prev_alpha;
				verts[1].r = verts[1].g = verts[1].b = verts[1].a = prev_alpha;
				verts[2].r = verts[2].g = verts[2].b = verts[2].a = current_alpha;
				verts[3].r = verts[3].g = verts[3].b = verts[3].a = current_alpha;

				verts[0].world = prev_top;
				verts[1].world = prev_bot;
				verts[2].world = current_bot;
				verts[3].world = current_top;

				verts[0].texture_position.u = prev_U;
				verts[1].texture_position.u = prev_U;
				verts[2].texture_position.u = current_U;
				verts[3].texture_position.u = current_U;

				verts[0].texture_position.v = verts[3].texture_position.v = 0.0f;
				verts[1].texture_position.v = verts[2].texture_position.v = 1.0f;

				batching_add_quad(ti->texture.bitmap_id, verts);
			}
		}

		prev_top = current_top;
		prev_bot = current_bot;
		prev_U = current_U;
		prev_alpha = current_alpha;
	}
}

// Adds a new segment to trailp at pos
// In order for trailp's 'spread' field to have any effect, it must be nonzero and the orient must be non-null
// If so, the orient's fvec is the treated as the direction of the trail, and the 
// new trail point is given a random velocity orthogonal to the fvec (scaled by spread speed)
void trail_add_segment( pool_index const &traili, vec3d *pos , const matrix* orient)
{
	trail* trailp = trail_find(traili);
	int next = trailp->tail;
	trailp->tail++;
	if ( trailp->tail >= NUM_TRAIL_SECTIONS )
		trailp->tail = 0;

	if ( trailp->head == trailp->tail )	{
		// wrapped!!
		trailp->head++;
		if ( trailp->head >= NUM_TRAIL_SECTIONS )
			trailp->head = 0;
	}

	trailp->pos[next] = *pos;
	trailp->val[next] = 0.0f;

	if (orient != nullptr && trailp->info.spread > 0.0f) {
		vm_vec_random_in_circle(&trailp->vel[next], &vmd_zero_vector, orient, trailp->info.spread, false, true);
	} else 
		vm_vec_zero(&trailp->vel[next]);
}		

void trail_set_segment(pool_index const &traili, vec3d *pos )
{
	trail *trailp = trail_find(traili);
	int next = trailp->tail-1;
	if ( next < 0 )	{
		next = NUM_TRAIL_SECTIONS-1;
	}
	
	trailp->pos[next] = *pos;
}

void trail_move_all(float frametime)
{
	TRACE_SCOPE(tracing::TrailsMoveAll);

	int num_alive_segments,n;
	float time_delta;
	trail *trailp = nullptr;
	for (auto iter = Trails.begin(); iter<Trails.end();++iter){
		trailp = *iter;
		num_alive_segments = 0;

		if ( trailp->tail != trailp->head )	{
			n = trailp->tail;			
			time_delta = frametime / trailp->info.max_life;
			do	{
				n--;
				if ( n < 0 ) n = NUM_TRAIL_SECTIONS-1;

				trailp->val[n] += time_delta;

				if ( trailp->val[n] <= 1.0f ) {
					num_alive_segments++;	// Record how many still alive.
				}

				trailp->pos[n] += trailp->vel[n] * frametime; 

			} while ( n != trailp->head );
		}		
	
		if ( (num_alive_segments < 1) && trailp->object_died)
		{
			Trails.remove(iter.position());
		}
	}
}

void trail_object_died(trail *trailp)
{
	trailp->object_died = true;
}
void trail_object_died(pool_index const &traili)
{
	trail* trailp = trail_find(traili);
	trailp->object_died = true;
}


void trail_render_all()
{
	GR_DEBUG_SCOPE("Render trails");
	TRACE_SCOPE(tracing::RenderTrails);

	// No trails at slot 0
	if ( !Detail.weapon_extras )
		return;

	trail *trailp = nullptr;
	for (auto iter = Trails.begin(); iter<Trails.end();++iter){
	
		trailp = *iter;
		trail_render(trailp,iter.position());
	}
}
int trail_stamp_elapsed(pool_index const &traili)
{
	trail *trailp = trail_find(traili);
 	return timestamp_elapsed(trailp->trail_stamp);
}

void trail_set_stamp(pool_index const &traili)
{
	trail *trailp = trail_find(traili);
	trailp->trail_stamp = timestamp(trailp->info.stamp);
}
