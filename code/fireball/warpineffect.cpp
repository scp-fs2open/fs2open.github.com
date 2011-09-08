/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "math/vecmat.h"
#include "graphics/tmapper.h"
#include "render/3d.h"
#include "fireball/fireballs.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "globalincs/pstypes.h"
#include "model/model.h"
#include "ship/ship.h"
#include "cmdline/cmdline.h"


extern int Warp_model;
extern int Warp_glow_bitmap;
extern int Warp_ball_bitmap;


void draw_face( vertex *v1, vertex *v2, vertex *v3 )
{
	vec3d norm;
	vertex *vertlist[3];

	vm_vec_perp(&norm,&v1->world, &v2->world, &v3->world);
	if ( vm_vec_dot(&norm, &v1->world ) >= 0.0 ) {
		vertlist[0] = v3;
		vertlist[1] = v2;
		vertlist[2] = v1;
	} else {
		vertlist[0] = v1;
		vertlist[1] = v2;
		vertlist[2] = v3;
	}

	g3_draw_poly( 3, vertlist, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT );

}

void warpin_render(object *obj, matrix *orient, vec3d *pos, int texture_bitmap_num, float radius, float life_percent, float max_radius, int warp_3d)
{
	vec3d center;
	vec3d vecs[5];
	vertex verts[5];
	int saved_gr_zbuffering = gr_zbuffer_get();

	gr_zbuffer_set(GR_ZBUFF_READ);

	vm_vec_scale_add( &center, pos, &orient->vec.fvec, -(max_radius/2.5f)/3.0f );


	if (Warp_glow_bitmap >= 0) {
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

			if (Cmdline_nohtl) {
				g3_rotate_vertex( &verts[4], &vecs[4] );
			} else {
				g3_transfer_vertex( &verts[4], &vecs[4] );
			}

			float alpha = (The_mission.flags & MISSION_FLAG_FULLNEB) ? (1.0f - neb2_get_fog_intensity(obj)) : 1.0f;
			gr_set_bitmap( Warp_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, alpha );

			g3_draw_bitmap( &verts[4], 0, r, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT );
		}
	}

	if ( (Warp_model >= 0) && (warp_3d || Cmdline_3dwarp) ) {
		float scale = radius / 25.0f;
		model_set_warp_globals(scale, scale, scale, texture_bitmap_num, (radius/max_radius) );

		float dist = vm_vec_dist_quick( pos, &Eye_position );
		model_set_detail_level((int)(dist / (radius * 10.0f)));

		model_render( Warp_model, orient, pos, MR_LOCK_DETAIL | MR_NO_LIGHTING | MR_NORMAL | MR_NO_FOGGING | MR_NO_CULL );

		model_set_warp_globals();
	} else {
		float Grid_depth = radius/2.5f;

		gr_set_bitmap( texture_bitmap_num, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );	

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

	//	vm_vec_scale_add( &vecs[4], ¢er, &orient->vec.fvec, -Grid_depth );
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

		if (Cmdline_nohtl) {
			g3_rotate_vertex( &verts[0], &vecs[0] );
			g3_rotate_vertex( &verts[1], &vecs[1] );
			g3_rotate_vertex( &verts[2], &vecs[2] );
			g3_rotate_vertex( &verts[3], &vecs[3] );
			g3_rotate_vertex( &verts[4], &vecs[4] );
		} else {
			g3_transfer_vertex( &verts[0], &vecs[0] );
			g3_transfer_vertex( &verts[1], &vecs[1] );
			g3_transfer_vertex( &verts[2], &vecs[2] );
			g3_transfer_vertex( &verts[3], &vecs[3] );
			g3_transfer_vertex( &verts[4], &vecs[4] );
		}

		int cull = gr_set_cull(0); // fixes rendering problem in D3D - taylor
		draw_face( &verts[0], &verts[4], &verts[1] );
		draw_face( &verts[1], &verts[4], &verts[2] );
		draw_face( &verts[4], &verts[3], &verts[2] );
		draw_face( &verts[0], &verts[3], &verts[4] );
		gr_set_cull(cull);
	}

	if (Warp_ball_bitmap > -1 && Cmdline_warp_flash == 1) {
		flash_ball warp_ball(20, .1f,.25f, &vmd_z_vector, &vmd_zero_vector, 4.0f, 0.5f);
		float adg = (2.0f * life_percent) - 1.0f;
		float pct = (powf(adg, 4.0f) - powf(adg, 128.0f)) * 4.0f;

		if (pct > 0.00001f) {
			g3_start_instance_matrix(pos, orient, true);

			gr_set_bitmap(Warp_ball_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.9999f);		

			warp_ball.render(max_radius * pct * 0.5f, adg * adg, adg * adg * 6.0f);

			g3_done_instance(true);
		}
	}

	gr_zbuffer_set( saved_gr_zbuffering );
}
