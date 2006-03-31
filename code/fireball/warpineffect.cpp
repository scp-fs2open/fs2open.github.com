/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/* 
 * $Logfile: /Freespace2/code/Fireball/WarpInEffect.cpp $
 * $Revision: 2.32 $
 * $Date: 2006-03-31 10:20:01 $
 * $Author: wmcoolmon $
 *
 * Code for rendering the warp in effects for ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.31  2005/09/21 03:55:32  Goober5000
 * add option for warp flash; mess with the cmdlines a bit
 * --Goober5000
 *
 * Revision 2.30  2005/08/25 22:40:02  taylor
 * basic cleaning, removing old/useless code, sanity stuff, etc:
 *  - very minor performance boost from not doing stupid things :)
 *  - minor change to 3d shockwave sizing to better approximate 2d effect movements
 *  - for shields, Gobal_tris was only holding half as many as the game can/will use, buffer is now set to full size to avoid possible rendering issues
 *  - removed extra tcache_set on OGL spec map code, not sure how that slipped in
 *
 * Revision 2.29  2005/07/25 05:24:17  Goober5000
 * cleaned up some command line and mission flag stuff
 * --Goober5000
 *
 * Revision 2.28  2005/07/23 17:27:50  Goober5000
 * remove Bobboau's extra warp thing
 * --Goober5000
 *
 * Revision 2.27  2005/05/22 22:47:19  phreak
 * compile warning fixage.
 *
 * Revision 2.26  2005/04/05 05:53:15  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.25  2005/03/25 06:57:33  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.24  2005/03/19 18:02:33  bobboau
 * added new graphic functions for state blocks
 * also added a class formanageing a new effect
 *
 * Revision 2.23  2005/01/01 19:47:26  taylor
 * make use of MR_NO_FOGGING to render models without fog
 *
 * Revision 2.22  2004/11/23 19:29:13  taylor
 * fix 2d warp in D3D, add cmdline option for 3d warp
 *
 * Revision 2.21  2004/09/17 07:12:22  Goober5000
 * changed around the logic for the 3D warp effect
 * --Goober5000
 *
 * Revision 2.20  2004/08/23 04:32:39  Goober5000
 * warp effect is back to FS2 default
 * --Goober5000
 *
 * Revision 2.19  2004/07/26 20:47:28  Kazan
 * remove MCD complete
 *
 * Revision 2.18  2004/07/17 09:29:13  taylor
 * make old warpmap and new glow bitmap 3D unlit to render right in OGL
 *
 * Revision 2.17  2004/07/12 16:32:46  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.16  2004/07/05 05:09:18  bobboau
 * FVF code, only the data that is needed is sent off to the card,,
 * OGL can take advantage of this if they want but it won't break
 * anything if they don't. also state block code has been implemented,
 * that's totaly internal to D3D no high level code is involved.
 *
 * Revision 2.15  2004/05/12 22:49:13  phreak
 * renamed the warp model variable from 'wm' to 'Warp_model'
 *
 * Revision 2.14  2004/05/08 01:08:48  Goober5000
 * small fix
 * --Goober5000
 *
 * Revision 2.13  2004/03/17 04:07:29  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.12  2004/03/05 09:02:00  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.11  2003/11/16 04:09:19  Goober5000
 * language
 *
 * Revision 2.10  2003/11/11 03:56:10  bobboau
 * lots of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.9  2003/09/26 14:37:13  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.8  2003/08/31 06:00:41  bobboau
 * an asortment of bugfixes, mostly with the specular code,
 * HUD flickering should be completly gone now
 *
 * Revision 2.7  2003/03/19 23:06:40  Goober5000
 * bit o' housecleaning
 * --Goober5000
 *
 * Revision 2.6  2003/03/19 22:49:32  Goober5000
 * added some mission flags
 * --Goober5000
 *
 * Revision 2.5  2003/03/19 06:23:27  Goober5000
 * added warp-effect sexp
 * --Goober5000
 *
 * Revision 2.4  2003/03/18 01:44:30  Goober5000
 * fixed some misspellings
 * --Goober5000
 *
 * Revision 2.3  2003/01/05 23:41:50  bobboau
 * disabled decals (for now), removed the warp ray thingys,
 * made some better error mesages while parseing weapons and ships tbls,
 * and... oh ya, added glow mapping
 *
 * Revision 2.2  2002/11/14 04:18:16  bobboau
 * added warp model and type 1 glow points
 * and well as made the new glow file type,
 * some general improvement to fighter beams,
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     7/22/99 1:22p Dave
 * Enable proper zbuffering for warpin glow effect.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 23    4/08/98 8:20p John
 * Made "Apex" of warp effect not move.
 * 
 * 22    3/30/98 4:02p John
 * Made machines with < 32 MB of RAM use every other frame of certain
 * bitmaps.   Put in code to keep track of how much RAM we've malloc'd.
 * 
 * 21    3/29/98 12:39p John
 * Made warp in glow page in
 * 
 * 20    3/26/98 5:21p John
 * Added new code to preload all bitmaps at the start of a level.
 * Commented it out, though.
 * 
 * 19    3/18/98 12:36p John
 * Made hardware have nicer looking warp effect
 * 
 * 18    3/16/98 4:51p John
 * Added low-level code to clip all polygons against an arbritary plane.
 * Took out all old model_interp_zclip and used this new method instead.  
 * 
 * 17    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 16    3/04/98 7:07p John
 * Added debug code to try to normalize a zerolength vector.
 * 
 * 15    2/26/98 3:28p John
 * fixed optimize compiler warning
 * 
 * 14    2/24/98 6:36p John
 * Made warp effect draw as a 4 poly cone.
 * 
 * 13    2/22/98 12:19p John
 * Externalized some strings
 * 
 * 12    1/15/98 9:07p John
 * Added noise to warp effect glow.
 * 
 * 11    1/15/98 4:58p John
 * Made warp effect use a looping ani.  Made the scaling up & down be in
 * software.
 * 
 * 10    12/30/97 6:44p John
 * Made g3_Draw_bitmap functions account for aspect of bitmap.
 * 
 * 9     12/08/97 11:15a John
 * added parameter to warpout for life.
 * 
 * 8     12/05/97 3:46p John
 * made ship thruster glow scale instead of being an animation.
 * 
 * 7     12/02/97 3:59p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 6     10/24/97 12:18p John
 * sped up warp effect by decreasing number of polys with distance.
 * 
 * 5     9/15/97 5:45p John
 * took out chunk stuff.
 * made pofview display thrusters as blue polies.
 * 
 * 4     9/12/97 4:02p John
 * put in ship warp out effect.
 * put in dynamic lighting for warp in/out
 * 
 * 3     9/09/97 4:49p John
 * Almost done ship warp in code
 * 
 * 2     9/08/97 8:39a John
 * added in code structure for grid
 * 
 * 1     9/05/97 10:07a John
 *
 * $NoKeywords: $
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


extern int Warp_model;
extern int Cmdline_nohtl;
extern int Cmdline_3dwarp;
extern int Cmdline_warp_flash;


DCF(norm,"normalize a zero length vector")
{
	if ( Dc_command )	{
		vec3d tmp = vmd_zero_vector;
		vm_vec_normalize(&tmp);
	}
}

void draw_face( vertex *v1, vertex *v2, vertex *v3 )
{
	vec3d norm;
	vertex *vertlist[3];

	vm_vec_perp(&norm,(vec3d *)&v1->x,(vec3d *)&v2->x,(vec3d *)&v3->x);
	if ( vm_vec_dot(&norm,(vec3d *)&v1->x ) >= 0.0 )	{
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

#define wSTUFF_VERTICES()	do { verts[0]->u = 0.5f; verts[0]->v = 0.0f;	verts[1]->u = 1.0f; verts[1]->v = 0.0f; verts[2]->u = 1.0f;	verts[2]->v = 1.0f; verts[3]->u = 0.5f; verts[3]->v = 1.0f; } while(0);
#define wR_VERTICES()		do { g3_rotate_vertex(verts[0], &bottom1); g3_rotate_vertex(verts[1], &bottom2);	g3_rotate_vertex(verts[2], &top2); g3_rotate_vertex(verts[3], &top1); } while(0);
#define wP_VERTICES()		do { for(idx=0; idx<4; idx++){ g3_project_vertex(verts[idx]); } } while(0);

void warpin_render(object *obj, matrix *orient, vec3d *pos, int texture_bitmap_num, float radius, float life_percent, float max_radius, int warp_3d)
{
	static flash_ball warp_ball(20, .1f,.25f, &vmd_z_vector, &vmd_zero_vector, 4.0f, 0.5f);

	int i;

	int saved_gr_zbuffering = gr_zbuffer_get();

//	gr_zbuffering = 0;


	float Grid_depth = radius/2.5f;

	vec3d center;

	vm_vec_scale_add( &center, pos, &orient->vec.fvec, -(max_radius/2.5f)/3.0f );

//	Warp_Map = texture_bitmap_num;//sets the warp map, and thus sets the interp flag for this being a warpin effect
	vec3d vecs[5];
	vertex verts[5];

	vecs[4] = center;		//this is for the warp glow-Bobboau
	verts[4].u = 0.5f; verts[4].v = 0.5f; 

	if (Cmdline_nohtl) {
		g3_rotate_vertex( &verts[4], &vecs[4] );
	} else {
		g3_transfer_vertex( &verts[4], &vecs[4] );
	}

	if ( Warp_glow_bitmap != -1 )	{
		gr_set_bitmap( Warp_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, (The_mission.flags & MISSION_FLAG_FULLNEB)?(1.0f - neb2_get_fog_intensity(obj)):1.0f );

		float r = radius;

		int render_it;
		
		#define OUT_PERCENT1 0.80f
		#define OUT_PERCENT2 0.90f

		#define IN_PERCENT1 0.10f
		#define IN_PERCENT2 0.20f

		if ( life_percent < IN_PERCENT1 )	{
			// do nothing
			render_it = 1;
		} else if ( life_percent < IN_PERCENT2 )	{
			r *= ( life_percent-IN_PERCENT1 ) / (IN_PERCENT2-IN_PERCENT1); 
			render_it = 1;
		} else if ( life_percent < OUT_PERCENT1 )	{
			// do nothing
			render_it = 1;
		} else if ( life_percent < OUT_PERCENT2 )	{
			r *= (OUT_PERCENT2 - life_percent) / (OUT_PERCENT2-OUT_PERCENT1);
			render_it = 1;
		} else {
			// do nothing
			render_it = 1;
		}

		if (render_it)	{
			int saved_gr_zbuffering = gr_zbuffer_get();
			gr_zbuffer_set(GR_ZBUFF_READ);

			// Add in noise 
			//float Noise[NOISE_NUM_FRAMES] = { 
			int noise_frame = fl2i(Missiontime/15.0f) % NOISE_NUM_FRAMES;

			r *= (0.40f + Noise[noise_frame]*0.30f);

			// Bobboau's warp thingie, toggled by cmdline
			if (Cmdline_warp_flash)
			{
				r += (float)pow((2.0f*life_percent) - 1.0f, 24.0f) * max_radius * 1.5f;
			}
			
			g3_draw_bitmap( &verts[4], 0, r, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT );
			gr_zbuffer_set(saved_gr_zbuffering);
		}
	}

	if((Warp_model >= 0) && (warp_3d || Cmdline_3dwarp))
	{
		float model_Interp_scale_xyz = radius / 20.0f;

		set_warp_globals(model_Interp_scale_xyz, model_Interp_scale_xyz, model_Interp_scale_xyz, texture_bitmap_num, (radius/max_radius) );
		
		float dist = vm_vec_dist_quick( pos, &Eye_position );

		model_set_detail_level((int)(dist / (radius * 10.0f)));
		gr_set_cull(0);
		model_render( Warp_model, orient, pos, MR_NO_LIGHTING | MR_NORMAL | MR_NO_FOGGING);
		gr_set_cull(1);

//	Warp_Map = -1;//un sets the warp map

		set_warp_globals(1.0f, 1.0f, 1.0f, -1, -1.0f);

/*
//this is that half done effect that was pissing people off-Bobboau

		gr_set_cull(0);

		matrix m;
		vec3d ray_dir;
		vm_vector_2_matrix(&m, &orient->vec.fvec, NULL, NULL);//this is the beam things that will come outof the point-Bobboau
		float angle = ANG_TO_RAD(135.0f);
		if(life_percent < 0.2f){
			angle = ANG_TO_RAD(135.0f * (life_percent * 5.0f));
		}
		for(int idx=0; idx<1; idx++){
			vm_vec_random_cone(&ray_dir, &orient->vec.fvec, angle, &m);
			mprintf(( "ray direction %f %f %f, orentation %f %f %f, angle %f\n",ray_dir.xyz.x, ray_dir.xyz.y, ray_dir.xyz.z, orient->vec.fvec.xyz.x, orient->vec.fvec.xyz.y, orient->vec.fvec.xyz.z, angle));
			vec3d p_temp = *pos;
			vm_vec_scale_add(&p_temp, pos, &ray_dir, (max_radius * 10 * frand_range(0.75f, 0.9f)));
			mprintf(( "end %f %f %f",p_temp.xyz.x, p_temp.xyz.y, p_temp.xyz.z));
			
			vertex h1[4];				// halves of a beam section	
			vertex *verts[4] = { &h1[0], &h1[1], &h1[2], &h1[3] };	
			vec3d top1, bottom1, top2, bottom2;

			beam_calc_facing_pts(&top1, &bottom1, &ray_dir, pos, 1.0f, 1.0f);	
			beam_calc_facing_pts(&top2, &bottom2, &ray_dir, &p_temp, 1.0f, 1.0f);	
			
			mprintf(( "top1 %f %f %f, botom1 %f %f %f, \n",top1.xyz.x, top1.xyz.y, top1.xyz.z, bottom1.xyz.x, bottom1.xyz.y, bottom1.xyz.z));
			mprintf(( "top2 %f %f %f, botom2 %f %f %f, \n",top2.xyz.x, top2.xyz.y, top2.xyz.z, bottom2.xyz.x, bottom2.xyz.y, bottom2.xyz.z));
			wR_VERTICES();																// rotate and project the vertices
			wP_VERTICES();						
			wSTUFF_VERTICES();		// stuff the beam with creamy goodness (texture coords)

			gr_set_bitmap(Warp_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.9999f);		
			g3_draw_poly( 4, verts, TMAP_FLAG_TILED | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT); // added TMAP_FLAG_TILED flag for beam texture tileing -Bobboau
		}

		gr_set_cull(1);
*/


	}else{
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
			
		verts[0].u = 0.01f; verts[0].v = 0.01f; 
		verts[1].u = 0.99f; verts[1].v = 0.01f; 
		verts[2].u = 0.99f; verts[2].v = 0.99f; 
		verts[3].u = 0.01f; verts[3].v = 0.99f; 
		verts[4].u = 0.5f; verts[4].v = 0.5f; 

		for (i=0; i<5; i++ )	{
			if (Cmdline_nohtl) {
				g3_rotate_vertex( &verts[i], &vecs[i] );
			} else {
				g3_transfer_vertex( &verts[i], &vecs[i] );
			}
		}

		gr_set_cull(0); // fixes rendering problem in D3D - taylor
		draw_face( &verts[0], &verts[4], &verts[1] );
		draw_face( &verts[1], &verts[4], &verts[2] );
		draw_face( &verts[4], &verts[3], &verts[2] );
		draw_face( &verts[0], &verts[3], &verts[4] );
		gr_set_cull(1);
	}

	if ( Warp_ball_bitmap > -1 ){
			g3_start_instance_matrix(pos,orient, true);
		gr_set_bitmap(Warp_ball_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.9999f);		
		float adg = (2.0f*life_percent) - 1.0f;
		float pct= (float)(pow(adg,4.0f)-pow(adg,128.0f))*4.0f;
		if(pct > 0.00001f)
		warp_ball.render(max_radius*pct/2.0f,adg*adg,adg*adg*6.0f);
			g3_done_instance(true);
	}
	gr_zbuffer_set( saved_gr_zbuffering );
}

