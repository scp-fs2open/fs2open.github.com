/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "cmdline/cmdline.h"
#include "globalincs/pstypes.h"
#include "graphics/tmapper.h"
#include "math/vecmat.h"
#include "mission/missionparse.h"
#include "model/model.h"
#include "nebula/neb.h"
#include "render/3d.h"
#include "render/batching.h"
#include "ship/ship.h"

void warpin_batch_draw_face( int texture, vertex *v1, vertex *v2, vertex *v3 )
{
	vec3d norm;
	vertex vertlist[3];

	vm_vec_perp(&norm,&v1->world, &v2->world, &v3->world);
	if ( vm_vec_dot(&norm, &v1->world ) >= 0.0 ) {
		vertlist[0] = *v3;
		vertlist[1] = *v2;
		vertlist[2] = *v1;
	} else {
		vertlist[0] = *v1;
		vertlist[1] = *v2;
		vertlist[2] = *v3;
	}

	// batch_add_tri(texture, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT | TMAP_FLAG_EMISSIVE, vertlist, 1.0f);
	batching_add_tri(texture, vertlist); // TODO render as emissive
}

void warpin_queue_render(model_draw_list *scene, object *obj, matrix *orient, vec3d *pos, int texture_bitmap_num, float radius, float life_percent, float max_radius, bool warp_3d, int warp_glow_bitmap, int warp_ball_bitmap, int warp_model_id)
{
	vec3d center;
	vec3d vecs[5];
	vertex verts[5];

	vm_vec_scale_add( &center, pos, &orient->vec.fvec, -(max_radius/2.5f)/3.0f );

	// Cyborg17, Initialize the *whole* struct, set some starting values and share with the rest of the array.
	verts[0] = {};
	verts[0].r = verts[0].g = verts[0].b = verts[0].a = 255;
	verts[1] = verts[2] = verts[3] = verts[4] = verts[0];

	if (warp_glow_bitmap >= 0) {
		float r = radius;
		bool render_it = true;

#define OUT_PERCENT1 0.80f
#define OUT_PERCENT2 0.90f

#define IN_PERCENT1 0.10f
#define IN_PERCENT2 0.20f

		if (Cmdline_warp_flash)
		{
			if ( (life_percent >= IN_PERCENT1) && (life_percent < IN_PERCENT2) ) {
				r *= (life_percent - IN_PERCENT1) / (IN_PERCENT2 - IN_PERCENT1);
				//render_it = true;
			} else if ( (life_percent >= OUT_PERCENT1) && (life_percent < OUT_PERCENT2) ) {
				r *= (OUT_PERCENT2 - life_percent) / (OUT_PERCENT2 - OUT_PERCENT1);
				//render_it = true;
			}
		}

		if (render_it) {
			// Add in noise 
			int noise_frame = fl2i(Missiontime/15.0f) % NOISE_NUM_FRAMES;

			r *= (0.40f + Noise[noise_frame] * 0.30f);

			// Bobboau's warp thingie, toggled by cmdline
			if (Cmdline_warp_flash) {
				r += powf((2.0f * life_percent) - 1.0f, 24.0f) * max_radius * 1.5f;
			}

			vecs[4] = center;
			verts[4].texture_position.u = 0.5f; verts[4].texture_position.v = 0.5f; 

			g3_transfer_vertex( &verts[4], &vecs[4] );

			float alpha = (The_mission.flags[Mission::Mission_Flags::Fullneb]) ? neb2_get_fog_visibility(&obj->pos, 1.0f) : 1.0f;

			//batch_add_bitmap(warp_glow_bitmap, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT, &verts[4], 0, r, alpha);
			batching_add_bitmap(warp_glow_bitmap, &verts[4], 0, r, alpha);
		}
	}

	if ( (warp_model_id >= 0) && (warp_3d) ) {
		model_render_params render_info;

		float scale = radius / 25.0f;

		vec3d warp_scale;

		warp_scale.xyz.x = warp_scale.xyz.y = warp_scale.xyz.z = scale;

		float dist = vm_vec_dist_quick( pos, &Eye_position );

		render_info.set_warp_params(texture_bitmap_num, radius/max_radius, warp_scale);
		render_info.set_detail_level_lock((int)(dist / (radius * 10.0f)));
		render_info.set_flags(MR_NO_LIGHTING | MR_NORMAL | MR_NO_FOGGING | MR_NO_CULL | MR_NO_BATCH);

		model_render_queue( &render_info, scene, warp_model_id, orient, pos);
	} else {
		float Grid_depth = radius/2.5f;

		// gr_set_bitmap( texture_bitmap_num, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );	

		vm_vec_scale_add( &vecs[0], &center, &orient->vec.uvec, radius );
		vm_vec_scale_add2( &vecs[0], &orient->vec.rvec, -radius );
		vm_vec_scale_add2( &vecs[0], &orient->vec.fvec, Grid_depth );

		vm_vec_scale_add( &vecs[1], &center, &orient->vec.uvec, radius );
		vm_vec_scale_add2( &vecs[1], &orient->vec.rvec, radius );
		vm_vec_scale_add2( &vecs[1], &orient->vec.fvec, Grid_depth );

		vm_vec_scale_add( &vecs[2], &center, &orient->vec.uvec, -radius );
		vm_vec_scale_add2( &vecs[2], &orient->vec.rvec, radius );
		vm_vec_scale_add2( &vecs[2], &orient->vec.fvec, Grid_depth );

		vm_vec_scale_add( &vecs[3], &center, &orient->vec.uvec, -radius );
		vm_vec_scale_add2( &vecs[3], &orient->vec.rvec, -radius );
		vm_vec_scale_add2( &vecs[3], &orient->vec.fvec, Grid_depth );

		//	vm_vec_scale_add( &vecs[4], center, &orient->vec.fvec, -Grid_depth );
		vecs[4] = center;

		verts[0].texture_position.u = 0.01f;
		verts[0].texture_position.v = 0.01f;

		verts[1].texture_position.u = 0.99f;
		verts[1].texture_position.v = 0.01f;

		verts[2].texture_position.u = 0.99f;
		verts[2].texture_position.v = 0.99f;

		verts[3].texture_position.u = 0.01f;
		verts[3].texture_position.v = 0.99f;

		verts[4].texture_position.u = 0.5f;
		verts[4].texture_position.v = 0.5f;

		g3_transfer_vertex( &verts[0], &vecs[0] );
		g3_transfer_vertex( &verts[1], &vecs[1] );
		g3_transfer_vertex( &verts[2], &vecs[2] );
		g3_transfer_vertex( &verts[3], &vecs[3] );
		g3_transfer_vertex( &verts[4], &vecs[4] );

		warpin_batch_draw_face( texture_bitmap_num, &verts[0], &verts[4], &verts[1] );
		warpin_batch_draw_face( texture_bitmap_num, &verts[1], &verts[4], &verts[2] );
		warpin_batch_draw_face( texture_bitmap_num, &verts[4], &verts[3], &verts[2] );
		warpin_batch_draw_face( texture_bitmap_num, &verts[0], &verts[3], &verts[4] );
	}

	if (warp_ball_bitmap >= 0 && Cmdline_warp_flash) {
		flash_ball warp_ball(20, .1f,.25f, &orient->vec.fvec, pos, 4.0f, 0.5f);

		float adg = (2.0f * life_percent) - 1.0f;
		float pct = (powf(adg, 4.0f) - powf(adg, 128.0f)) * 4.0f;

		if (pct > 0.00001f) {
			warp_ball.render(warp_ball_bitmap, max_radius * pct * 0.5f, adg * adg, adg * adg * 6.0f);
		}
	}
}
