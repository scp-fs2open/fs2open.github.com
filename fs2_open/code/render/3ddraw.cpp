/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Render/3ddraw.cpp $
 * $Revision: 2.9 $
 * $Date: 2003-11-16 04:09:27 $
 * $Author: Goober5000 $
 *
 * 3D rendering primitives
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.8  2003/11/11 03:56:12  bobboau
 * lots of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.7  2003/10/29 18:18:52  randomtiger
 * D3D particle flicker fix, also fixes explosions and thruster glows
 *
 * Revision 2.6  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.5  2003/08/30 14:49:01  phreak
 * fixed some random specular lighting bugs
 *
 * Revision 2.4  2003/08/21 15:03:43  phreak
 * zeroed out the specular fields since they caused some flickering
 *
 * Revision 2.3  2003/03/18 10:07:05  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.2.2.1  2002/09/24 18:56:45  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.2  2002/08/06 01:49:08  penguin
 * Renamed ccode members to cc_or and cc_and
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 18    9/06/99 3:23p Andsager
 * Make fireball and weapon expl ani LOD choice look at resolution of the
 * bitmap
 * 
 * 17    8/27/99 9:07p Dave
 * LOD explosions. Improved beam weapon accuracy.
 * 
 * 16    7/29/99 12:05a Dave
 * Nebula speed optimizations.
 * 
 * 15    7/13/99 1:16p Dave
 * 32 bit support. Whee!
 * 
 * 14    7/02/99 3:05p Anoop
 * Oops. Fixed g3_draw_2d_poly() so that it properly handles poly bitmap
 * and LFB bitmap calls.
 * 
 * 13    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 12    6/22/99 7:03p Dave
 * New detail options screen.
 * 
 * 11    6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 10    6/08/99 5:17p Dave
 * Fixed up perspective bitmap drawing.
 * 
 * 9     6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 8     5/28/99 1:58p Dave
 * Fixed problem with overwriting bank value drawing perspective bitmaps.
 * 
 * 7     5/28/99 1:45p Dave
 * Fixed up perspective bitmap drawing.
 * 
 * 6     5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * 5     4/07/99 6:22p Dave
 * Fred and Freespace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 4     3/09/99 6:24p Dave
 * More work on object update revamping. Identified several sources of
 * unnecessary bandwidth.
 * 
 * 3     2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 37    3/22/98 2:34p John
 * If alpha effects is on lowest detail level, draw rotated bitmaps as
 * scaled bitmaps.
 * 
 * 36    3/16/98 4:51p John
 * Added low-level code to clip all polygons against an arbritary plane.
 * Took out all old model_interp_zclip and used this new method instead.  
 * 
 * 35    3/04/98 9:29a John
 * Made rotated bitmaps force all sw's to the same value after clipping.
 * 
 * 34    3/03/98 6:59p John
 * 
 * 33    1/26/98 5:12p John
 * Added in code for Pentium Pro specific optimizations. Speed up
 * zbuffered correct tmapper about 35%.   Speed up non-zbuffered scalers
 * by about 25%.
 * 
 * 32    1/15/98 2:16p John
 * Made bitmaps zbuffer at center minus radius.
 * Made fireballs sort after objects they are around.
 * 
 * 31    1/06/98 2:11p John
 * Made g3_draw_rotated bitmap draw the same orientation as g3_draw_bitmap
 * (orient=0) and made it be the same size (it was 2x too big before).
 * 
 * 30    12/30/97 6:44p John
 * Made g3_Draw_bitmap functions account for aspect of bitmap.
 * 
 * 29    12/04/97 12:09p John
 * Made glows use scaler instead of tmapper so they don't rotate.  Had to
 * add a zbuffered scaler.
 * 
 * 28    12/02/97 4:00p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 27    11/29/97 2:05p John
 * made g3_draw_bitmap and g3_draw_rotated bitmap take w&h, not w/2 & h/2,
 * like they used to incorrectly assume.   Added code to model to read in
 * thruster radius's.
 * 
 * 26    10/20/97 4:49p John
 * added weapon trails.
 * added facing bitmap code to 3d lib.
 * 
 * 25    10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 24    9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 23    7/11/97 11:54a John
 * added rotated 3d bitmaps.
 * 
 * 22    5/07/97 2:59p John
 * Initial rev of D3D texturing.
 * 
 * 21    4/29/97 9:55a John
 * 
 * 20    3/10/97 2:25p John
 * Made pofview zbuffer.   Made textest work with new model code.  Took
 * out some unnecessary Asserts in the 3d clipper.
 * 
 * 
 * 19    3/06/97 5:36p Mike
 * Change vec_normalize_safe() back to vec_normalize().
 * Spruce up docking a bit.
 * 
 * 18    3/06/97 10:56a Mike
 * Write error checking version of vm_vec_normalize().
 * Fix resultant problems.
 * 
 * 17    2/26/97 5:24p John
 * Added g3_draw_sphere_ez
 * 
 * 16    2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 *
 * $NoKeywords: $
 */

#include "render/3dinternal.h"
#include "graphics/tmapper.h"
#include "graphics/scaler.h"
#include "graphics/2d.h"
#include "math/floating.h"
#include "physics/physics.h"		// For Physics_viewer_bank for g3_draw_rotated_bitmap
#include "bmpman/bmpman.h"
#include "globalincs/systemvars.h"
#include "globalincs/alphacolors.h"
#include "cmdline/cmdline.h"

#include "io/key.h"

//deal with a clipped line
int must_clip_line(vertex *p0,vertex *p1,ubyte codes_or, uint flags)
{
	int ret = 0;
	
	clip_line(&p0,&p1,codes_or, flags);
	
	if (p0->codes & p1->codes) goto free_points;

	codes_or = (unsigned char)(p0->codes | p1->codes);

	if (codes_or & CC_BEHIND) goto free_points;

	if (!(p0->flags&PF_PROJECTED))
		g3_project_vertex(p0);

	if (p0->flags&PF_OVERFLOW) goto free_points;

	if (!(p1->flags&PF_PROJECTED))
		g3_project_vertex(p1);

	if (p1->flags&PF_OVERFLOW) goto free_points;
	
	//gr_line(fl2i(p0->sx),fl2i(p0->sy),fl2i(p1->sx),fl2i(p1->sy));
	//	gr_line_float(p0->sx,p0->sy,p1->sx,p1->sy);

	gr_aaline( p0, p1 );

	ret = 1;

	//frees temp points
free_points:

	if (p0->flags & PF_TEMP_POINT)
		free_temp_point(p0);

	if (p1->flags & PF_TEMP_POINT)
		free_temp_point(p1);

	return ret;
}

//draws a line. takes two points.  returns true if drew
int g3_draw_line(vertex *p0,vertex *p1)
{
	ubyte codes_or;

	Assert( G3_count == 1 );

	if (p0->codes & p1->codes)
		return 0;

	codes_or = (unsigned char)(p0->codes | p1->codes);

	if (codes_or & CC_BEHIND)
		return must_clip_line(p0,p1,codes_or,0);

	if (!(p0->flags&PF_PROJECTED))
		g3_project_vertex(p0);

	if (p0->flags&PF_OVERFLOW) 
//		return 1;
		return must_clip_line(p0,p1,codes_or,0);


	if (!(p1->flags&PF_PROJECTED))
		g3_project_vertex(p1);

	if (p1->flags&PF_OVERFLOW)
//		return 1;
		return must_clip_line(p0,p1,codes_or,0);

	
//	gr_line(fl2i(p0->sx),fl2i(p0->sy),fl2i(p1->sx),fl2i(p1->sy));
//	gr_line_float(p0->sx,p0->sy,p1->sx,p1->sy);

	gr_aaline( p0, p1 );

	return 0;
}

//returns true if a plane is facing the viewer. takes the unrotated surface
//normal of the plane, and a point on it.  The normal need not be normalized
int g3_check_normal_facing(vector *v,vector *norm)
{
	vector tempv;

	Assert( G3_count == 1 );

	vm_vec_sub(&tempv,&View_position,v);

	return (vm_vec_dot(&tempv,norm) > 0.0f);
}

int do_facing_check(vector *norm,vertex **vertlist,vector *p)
{
	Assert( G3_count == 1 );

	if (norm) {		//have normal

		Assert(norm->xyz.x || norm->xyz.y || norm->xyz.z);

		return g3_check_normal_facing(p,norm);
	}
	else {	//normal not specified, so must compute

		vector tempv;

		//get three points (rotated) and compute normal

		vm_vec_perp(&tempv,(vector *)&vertlist[0]->x,(vector *)&vertlist[1]->x,(vector *)&vertlist[2]->x);

		return (vm_vec_dot(&tempv,(vector *)&vertlist[1]->x ) < 0.0);
	}
}

//like g3_draw_poly(), but checks to see if facing.  If surface normal is
//NULL, this routine must compute it, which will be slow.  It is better to
//pre-compute the normal, and pass it to this function.  When the normal
//is passed, this function works like g3_check_normal_facing() plus
//g3_draw_poly().
//returns -1 if not facing, 1 if off screen, 0 if drew
int g3_draw_poly_if_facing(int nv,vertex **pointlist,uint tmap_flags,vector *norm,vector *pnt)
{
	Assert( G3_count == 1 );

	if (do_facing_check(norm,pointlist,pnt))
		return g3_draw_poly(nv,pointlist,tmap_flags);
	else
		return 255;
}

//draw a polygon.
//Set TMAP_FLAG_TEXTURED in the tmap_flags to texture map it with current texture.
//returns 1 if off screen, 0 if drew
int g3_draw_poly(int nv,vertex **pointlist,uint tmap_flags)
{
	int i;
	vertex **bufptr;
	ccodes cc;

	Assert( G3_count == 1 );

	cc.cc_or = 0;
	cc.cc_and = 0xff;

	bufptr = Vbuf0;

	if(tmap_flags & TMAP_HTL_3D_UNLIT && !Cmdline_nohtl) {
	 	gr_tmapper( nv, pointlist, tmap_flags );
	 	return 0;
	}

	for (i=0;i<nv;i++) {
		vertex *p;

		p = bufptr[i] = pointlist[i];

		cc.cc_and &= p->codes;
		cc.cc_or  |= p->codes;
	}

	if (cc.cc_and)
		return 1;	//all points off screen

	if (cc.cc_or)	{
		Assert( G3_count == 1 );

		bufptr = clip_polygon(Vbuf0,Vbuf1,&nv,&cc,tmap_flags);

		if (nv && !(cc.cc_or & CC_BEHIND) && !cc.cc_and) {

			for (i=0;i<nv;i++) {
				vertex *p = bufptr[i];
				
				if (!(p->flags&PF_PROJECTED))
					g3_project_vertex(p);
		
				if (p->flags&PF_OVERFLOW) {
					//Int3();		//should not overflow after clip
					//printf( "overflow in must_clip_tmap_face\n" );
					goto free_points;
				}				
			}

			gr_tmapper( nv, bufptr, tmap_flags );
		}

free_points:
		;

		for (i=0;i<nv;i++)
			if (bufptr[i]->flags & PF_TEMP_POINT)
				free_temp_point(bufptr[i]);

	} else {
		//now make list of 2d coords (& check for overflow)

		for (i=0;i<nv;i++) {
			vertex *p = bufptr[i];
			
			if (!(p->flags&PF_PROJECTED))
				g3_project_vertex(p);

			if (p->flags&PF_OVERFLOW) {
				//Int3();		//should not overflow after clip
				//printf( "3d: Point overflowed, but flags say OK!\n" );
				return 255;
			}

		}

		gr_tmapper( nv, bufptr, tmap_flags );
	}
	return 0;	//say it drew
}



// Draw a polygon.  Same as g3_draw_poly, but it bashes sw to a constant value
// for all vertexes.  Needs to be done after clipping to get them all.
//Set TMAP_FLAG_TEXTURED in the tmap_flags to texture map it with current texture.
//returns 1 if off screen, 0 if drew
int g3_draw_poly_constant_sw(int nv,vertex **pointlist,uint tmap_flags, float constant_sw)
{
	int i;
	vertex **bufptr;
	ccodes cc;

	Assert( G3_count == 1 );

	cc.cc_or = 0; cc.cc_and = 0xff;

	bufptr = Vbuf0;

	for (i=0;i<nv;i++) {
		vertex *p;

		p = bufptr[i] = pointlist[i];

		cc.cc_and &= p->codes;
		cc.cc_or  |= p->codes;
	}

	if (cc.cc_and)
		return 1;	//all points off screen

	if(!Cmdline_nohtl && (tmap_flags & TMAP_HTL_3D_UNLIT)) {
		gr_tmapper( nv, bufptr, tmap_flags );
		return 0;
	}

	if (cc.cc_or)	{
		Assert( G3_count == 1 );

		bufptr = clip_polygon(Vbuf0, Vbuf1, &nv, &cc, tmap_flags);

		if (nv && !(cc.cc_or & CC_BEHIND) && !cc.cc_and) {

			for (i=0;i<nv;i++) {
				vertex *p = bufptr[i];

				if (!(p->flags&PF_PROJECTED))
					g3_project_vertex(p);
		
				if (p->flags&PF_OVERFLOW) {
					//Int3();		//should not overflow after clip
					//printf( "overflow in must_clip_tmap_face\n" );
					goto free_points;
				}

				p->sw = constant_sw;
			}

			gr_tmapper( nv, bufptr, tmap_flags );

			// draw lines connecting the faces
			/*
			gr_set_color_fast(&Color_bright_green);
			for(i=0; i<nv-1; i++){
				g3_draw_line(bufptr[i], bufptr[i+1]);
			} 
			g3_draw_line(bufptr[0], bufptr[i]);
			*/
		}

free_points:
		;

		for (i=0;i<nv;i++){
			if (bufptr[i]->flags & PF_TEMP_POINT){
				free_temp_point(bufptr[i]);
			}
		}
	} else {
		//now make list of 2d coords (& check for overflow)

		for (i=0;i<nv;i++) {
			vertex *p = bufptr[i];

			if (!(p->flags&PF_PROJECTED))
				g3_project_vertex(p);

			if (p->flags&PF_OVERFLOW) {
				//Int3();		//should not overflow after clip
				//printf( "3d: Point overflowed, but flags say OK!\n" );
				return 255;
			}

			p->sw = constant_sw;

		}

		gr_tmapper( nv, bufptr, tmap_flags );

		// draw lines connecting the faces
		/*
		gr_set_color_fast(&Color_bright_green);
		for(i=0; i<nv-1; i++){
			g3_draw_line(bufptr[i], bufptr[i+1]);
		} 
		g3_draw_line(bufptr[0], bufptr[i]);
		*/
	}
	return 0;	//say it drew
}

//draw a sortof sphere - i.e., the 2d radius is proportional to the 3d
//radius, but not to the distance from the eye
int g3_draw_sphere(vertex *pnt,float rad)
{
	Assert( G3_count == 1 );

	if (! (pnt->codes & CC_BEHIND)) {

		if (! (pnt->flags & PF_PROJECTED))
			g3_project_vertex(pnt);

		if (! (pnt->codes & PF_OVERFLOW)) {
			float r2,t;

			r2 = rad*Matrix_scale.xyz.x;

			t=r2*Canv_w2/pnt->z;

			gr_circle(fl2i(pnt->sx),fl2i(pnt->sy),fl2i(t*2.0f));
		}
	}

	return 0;
}

int g3_draw_sphere_ez(vector *pnt,float rad)
{
	vertex pt;
	ubyte flags;

	Assert( G3_count == 1 );

	flags = g3_rotate_vertex(&pt,pnt);

	if (flags == 0) {

		g3_project_vertex(&pt);

		if (!(pt.flags & PF_OVERFLOW))	{

			g3_draw_sphere( &pt, rad );
		}
	}

	return 0;
}

//ID3DXSprite 
// creates a quad centered around the given point, pointed at the camera
/*
0----1
|\   |
|  \ |
3----2
1.41421356
*/
int g3_draw_bitmap_3d(vertex *pnt,int orient, float rad,uint tmap_flags, float depth){
//return 0;
//	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and hieght rad
	vector PNT;
	vm_vert2vec(pnt, &PNT);
	vector p[4];
	vertex P[4];
	matrix m;
	vm_set_identity(&m);

	vertex *ptlist[4] = { &P[3], &P[2], &P[1], &P[0] };	
	float aspect = gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height;//seems that we have to corect for the aspect ratio

	p[0].xyz.x = rad * aspect;	p[0].xyz.y = rad;	p[0].xyz.z = -depth;
	p[1].xyz.x = -rad * aspect;	p[1].xyz.y = rad;	p[1].xyz.z = -depth;
	p[2].xyz.x = -rad * aspect;	p[2].xyz.y = -rad;	p[2].xyz.z = -depth;
	p[3].xyz.x = rad * aspect;	p[3].xyz.y = -rad;	p[3].xyz.z = -depth;

	matrix te = View_matrix;
	vm_transpose_matrix(&te);
	for(int i = 0; i<4; i++){
		vector t = p[i];
		vm_vec_rotate(&p[i],&t,&te);//point it at the eye
		vm_vec_add2(&p[i],&PNT);//move it
	}

	//move all the data from the vecs into the verts
	g3_transfer_vertex(&P[0], &p[3]);
	g3_transfer_vertex(&P[1], &p[2]);
	g3_transfer_vertex(&P[2], &p[1]);
	g3_transfer_vertex(&P[3], &p[0]);

	//set up the UV coords
	P[0].u = 0.0f;	P[0].v = 0.0f;
	P[1].u = 1.0f;	P[1].v = 0.0f;
	P[2].u = 1.0f;	P[2].v = 1.0f;
	P[3].u = 0.0f;	P[3].v = 1.0f;

	gr_set_cull(0);
	g3_draw_poly(4,ptlist,tmap_flags);
	gr_set_cull(1);

	return 0;
}

//draws a bitmap with the specified 3d width & height 
//returns 1 if off screen, 0 if drew
// Orient
int g3_draw_bitmap(vertex *pnt,int orient, float rad,uint tmap_flags, float depth)
{
	if(!Cmdline_nohtl && (tmap_flags & TMAP_HTL_3D_UNLIT)) {
		return g3_draw_bitmap_3d(pnt, orient, rad, tmap_flags, depth);
	}
//	return 0;

	vertex va, vb;
	float t,w,h;
	float width, height;

	if ( tmap_flags & TMAP_FLAG_TEXTURED )	{
		int bw, bh;

		bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

		if ( bw < bh )	{
			width = rad*2.0f;
			height = width*i2fl(bh)/i2fl(bw);
		} else if ( bw > bh )	{
			height = rad*2.0f;
			width = height*i2fl(bw)/i2fl(bh);
		} else {
			width = height = rad*2.0f;
		}		
	} else {
		width = height = rad*2.0f;
	}

	Assert( G3_count == 1 );

	if ( pnt->codes & (CC_BEHIND|CC_OFF_USER) ) 
		return 1;

	if (!(pnt->flags&PF_PROJECTED))
		g3_project_vertex(pnt);

	if (pnt->flags & PF_OVERFLOW)
		return 1;

	t = (width*Canv_w2)/pnt->z;
	w = t*Matrix_scale.xyz.x;

	t = (height*Canv_h2)/pnt->z;
	h = t*Matrix_scale.xyz.y;

	float z,sw;
	z = pnt->z - rad/2.0f;
	if ( z <= 0.0f ) {
		z = 0.0f;
		sw = 0.0f;
	} else {
		sw = 1.0f / z;
	}

	va.sx = pnt->sx - w/2.0f;
	va.sy = pnt->sy - h/2.0f;
	va.sw = sw;
	va.z = z;

	vb.sx = va.sx + w;
	vb.sy = va.sy + h;
	vb.sw = sw;
	vb.z = z;

	if ( orient & 1 )	{
		va.u = 1.0f;
		vb.u = 0.0f;
	} else {
		va.u = 0.0f;
		vb.u = 1.0f;
	}

	if ( orient & 2 )	{
		va.v = 1.0f;
		vb.v = 0.0f;
	} else {
		va.v = 0.0f;
		vb.v = 1.0f;
	}

	gr_scaler(&va, &vb);

	return 0;
}

// get bitmap dims onscreen as if g3_draw_bitmap() had been called
int g3_get_bitmap_dims(int bitmap, vertex *pnt, float rad, int *x, int *y, int *w, int *h, int *size)
{	
	float t;
	float width, height;
	
	int bw, bh;

	bm_get_info( bitmap, &bw, &bh, NULL );

	if ( bw < bh )	{
		width = rad*2.0f;
		height = width*i2fl(bh)/i2fl(bw);
	} else if ( bw > bh )	{
		height = rad*2.0f;
		width = height*i2fl(bw)/i2fl(bh);
	} else {
		width = height = rad*2.0f;
	}			

	Assert( G3_count == 1 );

	if ( pnt->codes & (CC_BEHIND|CC_OFF_USER) ) {
		return 1;
	}

	if (!(pnt->flags&PF_PROJECTED)){
		g3_project_vertex(pnt);
	}

	if (pnt->flags & PF_OVERFLOW){
		return 1;
	}

	t = (width*Canv_w2)/pnt->z;
	*w = (int)(t*Matrix_scale.xyz.x);

	t = (height*Canv_h2)/pnt->z;
	*h = (int)(t*Matrix_scale.xyz.y);	

	*x = (int)(pnt->sx - *w/2.0f);
	*y = (int)(pnt->sy - *h/2.0f);	

	*size = max(bw, bh);

	return 0;
}

int g3_draw_rotated_bitmap_3d(vertex *pnt,float angle, float rad,uint tmap_flags, float depth){

	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and hieght rad

	angle+=Physics_viewer_bank;
	if ( angle < 0.0f )
		angle += PI2;
	else if ( angle > PI2 )
		angle -= PI2;
	float sa = (float)sin(angle);
	float ca = (float)cos(angle);

	vector PNT;
	vm_vert2vec(pnt, &PNT);
	vector p[4];
	vertex P[4];
	matrix m;
	vm_set_identity(&m);

	vertex *ptlist[4] = { &P[3], &P[2], &P[1], &P[0] };	
	float aspect = gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height;//seems that we have to corect for the aspect ratio

	p[0].xyz.x = (-rad*ca + rad*sa) * aspect;	p[0].xyz.y = (-rad*sa - rad*ca);	p[0].xyz.z = -depth;
	p[1].xyz.x = (rad*ca + rad*sa) * aspect;		p[1].xyz.y = (rad*sa - rad*ca);	p[1].xyz.z = -depth;
	p[2].xyz.x = (rad*ca - rad*sa) * aspect;		p[2].xyz.y = (rad*sa + rad*ca);	p[2].xyz.z = -depth;
	p[3].xyz.x = (-rad*ca - rad*sa) * aspect;	p[3].xyz.y = (-rad*sa + rad*ca);	p[3].xyz.z = -depth;

	matrix te = View_matrix;
	vm_transpose_matrix(&te);
	for(int i = 0; i<4; i++){
		vector t = p[i];
		vm_vec_rotate(&p[i],&t,&te);//point it at the eye
		vm_vec_add2(&p[i],&PNT);//move it
	}

	//move all the data from the vecs into the verts
	g3_transfer_vertex(&P[0], &p[3]);
	g3_transfer_vertex(&P[1], &p[2]);
	g3_transfer_vertex(&P[2], &p[1]);
	g3_transfer_vertex(&P[3], &p[0]);

	//set up the UV coords
	P[0].u = 0.0f;	P[0].v = 0.0f;
	P[1].u = 1.0f;	P[1].v = 0.0f;
	P[2].u = 1.0f;	P[2].v = 1.0f;
	P[3].u = 0.0f;	P[3].v = 1.0f;

/*	P[0].spec_r = 0;	P[0].spec_g = 0;	P[0].spec_b = 0;
	P[1].spec_r = 0;	P[1].spec_g = 0;	P[1].spec_b = 0;
	P[2].spec_r = 0;	P[2].spec_g = 0;	P[2].spec_b = 0;
	P[3].spec_r = 0;	P[3].spec_g = 0;	P[3].spec_b = 0;
*/
	gr_set_cull(0);
	g3_draw_poly(4,ptlist,tmap_flags);
	gr_set_cull(1);

	return 0;
}

//draws a bitmap with the specified 3d width & height 
//returns 1 if off screen, 0 if drew
int g3_draw_rotated_bitmap(vertex *pnt,float angle, float rad,uint tmap_flags, float depth)
{
	if(!Cmdline_nohtl && (tmap_flags & TMAP_HTL_3D_UNLIT)) {
		return g3_draw_rotated_bitmap_3d(pnt, angle, rad, tmap_flags, depth);
	}
	vertex v[4];
	vertex *vertlist[4] = { &v[3], &v[2], &v[1], &v[0] };
	float sa, ca;
	int i;

	memset(v,0,sizeof(vertex)*4);

	/*
	if ( !Detail.alpha_effects )	{
		int ang;
		if ( angle < PI/2 )	{
			ang = 0;
		} else if ( angle < PI )	{
			ang = 1;
		} else if ( angle < PI+PI/2 )	{
			ang = 2;
		} else {
			ang = 3;
		}
		return g3_draw_bitmap( pnt, ang, rad, tmap_flags );
	}
	*/

	Assert( G3_count == 1 );

	angle+=Physics_viewer_bank;
	if ( angle < 0.0f )
		angle += PI2;
	else if ( angle > PI2 )
		angle -= PI2;
//	angle = 0.0f;
			
	sa = (float)sin(angle);
	ca = (float)cos(angle);

	float width, height;

	if ( tmap_flags & TMAP_FLAG_TEXTURED )	{
		int bw, bh;

		bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

		if ( bw < bh )	{
			width = rad;
			height = width*i2fl(bh)/i2fl(bw);
		} else if ( bw > bh )	{
			height = rad;
			width = height*i2fl(bw)/i2fl(bh);
		} else {
			width = height = rad;
		}		
	} else {
		width = height = rad;
	}


	v[0].x = (-width*ca + height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[0].y = (-width*sa - height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[0].z = pnt->z;
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 1.0f;
	

	v[1].x = (width*ca + height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[1].y = (width*sa - height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[1].z = pnt->z;
	v[1].sw = 0.0f;
	v[1].u = 1.0f;
	v[1].v = 1.0f;

	v[2].x = (width*ca - height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[2].y = (width*sa + height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[2].z = pnt->z;
	v[2].sw = 0.0f;
	v[2].u = 1.0f;
	v[2].v = 0.0f;

	v[3].x = (-width*ca - height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[3].y = (-width*sa + height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[3].z = pnt->z;
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 0.0f;

	ubyte codes_and=0xff;

	float sw,z;
	z = pnt->z - rad / 4.0f;
	if ( z < 0.0f ) z = 0.0f;
	sw = 1.0f / z;

	for (i=0; i<4; i++ )	{
		//now code the four points
		codes_and &= g3_code_vertex(&v[i]);
		v[i].flags = 0;		// mark as not yet projected
		//g3_project_vertex(&v[i]);
	}

	if (codes_and)
		return 1;		//1 means off screen

	// clip and draw it
	g3_draw_poly_constant_sw(4, vertlist, tmap_flags, sw );	
	
	return 0;
}

#define TRIANGLE_AREA(_p, _q, _r)	do { vector a, b, cross; a.xyz.x = _q->x - _p->x; a.xyz.y = _q->y - _p->y; a.xyz.z = 0.0f; b.xyz.x = _r->x - _p->x; b.xyz.y = _r->y - _p->y; b.xyz.z = 0.0f; vm_vec_crossprod(&cross, &a, &b); total_area += vm_vec_mag(&cross) * 0.5f; } while(0);
float g3_get_poly_area(int nv, vertex **pointlist)
{
	int idx;
	float total_area = 0.0f;	

	// each triangle
	for(idx=1; idx<nv-1; idx++){
		TRIANGLE_AREA(pointlist[0], pointlist[idx], pointlist[idx+1]);
	}

	// done
	return total_area;
}

// Draw a polygon.  Same as g3_draw_poly, but it bashes sw to a constant value
// for all vertexes.  Needs to be done after clipping to get them all.
//Set TMAP_FLAG_TEXTURED in the tmap_flags to texture map it with current texture.
//returns 1 if off screen, 0 if drew
float g3_draw_poly_constant_sw_area(int nv, vertex **pointlist, uint tmap_flags, float constant_sw, float area)
{
	int i;
	vertex **bufptr;
	ccodes cc;
	float p_area = 0.0f;

	Assert( G3_count == 1 );

	cc.cc_or = 0;
	cc.cc_and = 0xff;

	bufptr = Vbuf0;

	for (i=0;i<nv;i++) {
		vertex *p;

		p = bufptr[i] = pointlist[i];

		cc.cc_and &= p->codes;
		cc.cc_or  |= p->codes;
	}

	if (cc.cc_and){
		return 0.0f;	//all points off screen
	}

	if (cc.cc_or)	{
		Assert( G3_count == 1 );

		bufptr = clip_polygon(Vbuf0, Vbuf1, &nv, &cc, tmap_flags);

		if (nv && !(cc.cc_or & CC_BEHIND) && !cc.cc_and) {

			for (i=0;i<nv;i++) {
				vertex *p = bufptr[i];

				if (!(p->flags&PF_PROJECTED))
					g3_project_vertex(p);
		
				if (p->flags&PF_OVERFLOW) {
					//Int3();		//should not overflow after clip
					//printf( "overflow in must_clip_tmap_face\n" );
					goto free_points;
				}

				p->sw = constant_sw;
			}

			// check area
			p_area = g3_get_poly_area(nv, bufptr);
			if(p_area > area){
				return 0.0f;
			}

			gr_tmapper( nv, bufptr, tmap_flags );			
		}

free_points:
		;

		for (i=0;i<nv;i++){
			if (bufptr[i]->flags & PF_TEMP_POINT){
				free_temp_point(bufptr[i]);
			}
		}
	} else {
		//now make list of 2d coords (& check for overflow)

		for (i=0;i<nv;i++) {
			vertex *p = bufptr[i];

			if (!(p->flags&PF_PROJECTED))
				g3_project_vertex(p);

			if (p->flags&PF_OVERFLOW) {				
				return 0.0f;
			}

			p->sw = constant_sw;
		}

		// check area
		p_area = g3_get_poly_area(nv, bufptr);
		if(p_area > area){
			return 0.0f;
		}		

		gr_tmapper( nv, bufptr, tmap_flags );		
	}

	// how much area we drew
	return p_area;
}


//draws a bitmap with the specified 3d width & height 
//returns 1 if off screen, 0 if drew
float g3_draw_rotated_bitmap_area(vertex *pnt,float angle, float rad,uint tmap_flags, float area)
{
	vertex v[4];
	vertex *vertlist[4] = { &v[3], &v[2], &v[1], &v[0] };
	float sa, ca;
	int i;	

	memset(v,0,sizeof(vertex)*4);

	Assert( G3_count == 1 );

	angle+=Physics_viewer_bank;
	if ( angle < 0.0f ){
		angle += PI2;
	} else if ( angle > PI2 ) {
		angle -= PI2;
	}
			
	sa = (float)sin(angle);
	ca = (float)cos(angle);

	float width, height;

	if ( tmap_flags & TMAP_FLAG_TEXTURED )	{
		int bw, bh;

		bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

		if ( bw < bh )	{
			width = rad;
			height = width*i2fl(bh)/i2fl(bw);
		} else if ( bw > bh )	{
			height = rad;
			width = height*i2fl(bw)/i2fl(bh);
		} else {
			width = height = rad;
		}		
	} else {
		width = height = rad;
	}

	v[0].x = (-width*ca + height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[0].y = (-width*sa - height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[0].z = pnt->z;
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 1.0f;

	v[1].x = (width*ca + height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[1].y = (width*sa - height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[1].z = pnt->z;
	v[1].sw = 0.0f;
	v[1].u = 1.0f;
	v[1].v = 1.0f;

	v[2].x = (width*ca - height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[2].y = (width*sa + height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[2].z = pnt->z;
	v[2].sw = 0.0f;
	v[2].u = 1.0f;
	v[2].v = 0.0f;

	v[3].x = (-width*ca - height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[3].y = (-width*sa + height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[3].z = pnt->z;
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 0.0f;

	ubyte codes_and=0xff;

	float sw,z;
	z = pnt->z - rad / 4.0f;
	if ( z < 0.0f ) z = 0.0f;
	sw = 1.0f / z;

	for (i=0; i<4; i++ )	{
		//now code the four points
		codes_and &= g3_code_vertex(&v[i]);
		v[i].flags = 0;		// mark as not yet projected
		//g3_project_vertex(&v[i]);
	}

	if (codes_and){
		return 0.0f;
	}

	// clip and draw it
	return g3_draw_poly_constant_sw_area(4, vertlist, tmap_flags, sw, area );		
}



#include "graphics/2d.h"
typedef struct horz_pt {
	float x, y;
	int edge;
} horz_pt;

//draws a horizon. takes eax=sky_color, edx=ground_color
void g3_draw_horizon_line()
{
	//int sky_color,int ground_color
	int s1, s2;
	int cpnt;
	horz_pt horz_pts[4];		// 0 = left, 1 = right
//	int top_color, bot_color;
//	int color_swap;		//flag for if we swapped
//	int sky_ground_flag;	//0=both, 1=all sky, -1=all gnd

	vector horizon_vec;
	
	float up_right, down_right,down_left,up_left;

//	color_swap = 0;		//assume no swap
//	sky_ground_flag = 0;	//assume both

//	if ( View_matrix.uvec.xyz.y < 0.0f )
//		color_swap = 1;
//	else if ( View_matrix.uvec.xyz.y == 0.0f )	{
//		if ( View_matrix.uvec.xyz.x > 0.0f )
//			color_swap = 1;
//	}

//	if (color_swap)	{
//		top_color  = ground_color;
//		bot_color = sky_color;
//	} else {
//		top_color  = sky_color;
//		bot_color = ground_color;
//	}

	Assert( G3_count == 1 );


	//compute horizon_vector
	
	horizon_vec.xyz.x = Unscaled_matrix.vec.rvec.xyz.y*Matrix_scale.xyz.y*Matrix_scale.xyz.z;
	horizon_vec.xyz.y = Unscaled_matrix.vec.uvec.xyz.y*Matrix_scale.xyz.x*Matrix_scale.xyz.z;
	horizon_vec.xyz.z = Unscaled_matrix.vec.fvec.xyz.y*Matrix_scale.xyz.x*Matrix_scale.xyz.y;

	// now compute values & flag for 4 corners.
	up_right = horizon_vec.xyz.x + horizon_vec.xyz.y + horizon_vec.xyz.z;
	down_right = horizon_vec.xyz.x - horizon_vec.xyz.y + horizon_vec.xyz.z;
	down_left = -horizon_vec.xyz.x - horizon_vec.xyz.y + horizon_vec.xyz.z;
	up_left = -horizon_vec.xyz.x + horizon_vec.xyz.y + horizon_vec.xyz.z;

	//check flags for all sky or all ground.
	if ( (up_right<0.0f)&&(down_right<0.0f)&&(down_left<0.0f)&&(up_left<0.0f) )	{
//		mprintf(( "All ground.\n" ));
		return;
	}

	if ( (up_right>0.0f)&&(down_right>0.0f)&&(down_left>0.0f)&&(up_left>0.0f) )	{
//		mprintf(( "All sky.\n" ));
		return;
	}

//mprintf(( "Horizon vec = %.4f, %.4f, %.4f\n", horizon_vec.xyz.x, horizon_vec.xyz.y, horizon_vec.xyz.z ));
//mprintf(( "%.4f, %.4f, %.4f, %.4f\n", up_right, down_right, down_left, up_left ));

	
//	mprintf(( "u: %.4f %.4f %.4f  c: %.4f %.4f %.4f %.4f\n",Unscaled_matrix.vec.uvec.xyz.y,Unscaled_matrix.vec.uvec.xyz.z,Unscaled_matrix.vec.uvec.xyz.x,up_left,up_right,down_right,down_left ));
	// check for intesection with each of four edges & compute horizon line
	cpnt = 0;
	
	// check intersection with left edge
	s1 = up_left > 0.0f;
	s2 = down_left > 0.0f;
	if ( s1 != s2 )	{
		horz_pts[cpnt].x = 0.0f;
		horz_pts[cpnt].y = fl_abs(up_left * Canv_h2 / horizon_vec.xyz.y);
		horz_pts[cpnt].edge = 0;
		cpnt++;
	}

	// check intersection with top edge
	s1 = up_left > 0.0f;
	s2 = up_right > 0.0f;
	if ( s1 != s2 )	{
		horz_pts[cpnt].x = fl_abs(up_left * Canv_w2 / horizon_vec.xyz.x);
		horz_pts[cpnt].y = 0.0f;
		horz_pts[cpnt].edge = 1;
		cpnt++;
	}

	
	// check intersection with right edge
	s1 = up_right > 0.0f;
	s2 = down_right > 0.0f;
	if ( s1 != s2 )	{
		horz_pts[cpnt].x = i2fl(Canvas_width)-1;
		horz_pts[cpnt].y = fl_abs(up_right * Canv_h2 / horizon_vec.xyz.y);
		horz_pts[cpnt].edge = 2;
		cpnt++;
	}
	
	//check intersection with bottom edge
	s1 = down_right > 0.0f;
	s2 = down_left > 0.0f;
	if ( s1 != s2 )	{
		horz_pts[cpnt].x = fl_abs(down_left * Canv_w2 / horizon_vec.xyz.x);
		horz_pts[cpnt].y = i2fl(Canvas_height)-1;
		horz_pts[cpnt].edge = 3;
		cpnt++;
	}

	if ( cpnt != 2 )	{
		mprintf(( "HORZ: Wrong number of points (%d)\n", cpnt ));
		return;
	}

	//make sure first edge is left

	if ( horz_pts[0].x > horz_pts[1].x )	{
		horz_pt tmp;
		tmp = horz_pts[0];
		horz_pts[0] = horz_pts[1];
		horz_pts[1] = tmp;
	}


	// draw from left to right.
	gr_line( fl2i(horz_pts[0].x),fl2i(horz_pts[0].y),fl2i(horz_pts[1].x),fl2i(horz_pts[1].y) );
	
}


/*

horizon_poly	dw	5 dup (?,?)	;max of 5 points

;for g3_compute_horz_vecs
xfrac	fix	?
yfrac	fix	?

vec_ptr	dd	?
corner_num	dd	?

;for compute corner vec
m13	fix	?	;m1-m3
m46	fix	?
m79	fix	?
m56	fix	?
m23	fix	?
m89	fix	?

_DATA	ends


_TEXT	segment	dword public USE32 'CODE'

	extn	gr_setcolor_,gr_clear_canvas_
	extn	gr_upoly_tmap_

;draw a polygon (one half of horizon) from the horizon line
draw_horz_poly:	lea	ebx,horizon_poly

;copy horizon line as first points in poly

	mov	eax,[edi]
	mov	[ebx],eax
	mov	eax,4[edi]
	mov	4[ebx],eax

	mov	eax,[esi]
	mov	8[ebx],eax
	mov	eax,4[esi]
	mov	12[ebx],eax

;add corners to polygon

	mov	eax,8[esi]	;edge number of start edge

	mov	ecx,8[edi]	;edge number of end point
	sub	ecx,eax	;number of edges
	jns	edgenum_ok
	add	ecx,4
edgenum_ok:
	mov	edx,ecx	;save count
	sal	eax,3	;edge * 8
	lea	esi,corners[eax]	;first corner
	lea	edi,16[ebx]	;rest of poly
corner_loop:	movsd
	movsd		;copy a corner
	cmp	esi,offset corners+8*4	;end of list?
	jne	no_wrap
	lea	esi,corners
no_wrap:	loop	corner_loop

;now draw the polygon
	mov	eax,edx	;get corner count
	add	eax,2	;..plus horz line end points
	lea	edx,horizon_poly	;get the points
;;	call	gr_poly_	;draw it!
 call gr_upoly_tmap_
	ret

;return information on the polygon that is the sky. 
;takes ebx=ptr to x,y pairs, ecx=ptr to vecs for each point
;returns eax=number of points
;IMPORTANT: g3_draw_horizon() must be called before this routine.
g3_compute_sky_polygon:
	test	sky_ground_flag,-1	;what was drawn
	js	was_all_ground
	jg	was_all_sky	

	pushm	ebx,ecx,edx,esi,edi

	lea	esi,left_point
	lea	edi,right_point
	test	color_swap,-1
	jz	no_swap_ends
	xchg	esi,edi	;sky isn't top
no_swap_ends:

;copy horizon line as first points in poly

	mov	eax,[edi]	;copy end point
	mov	[ebx],eax
	mov	eax,4[edi]
	mov	4[ebx],eax

	mov	eax,[esi]	;copy start point
	mov	8[ebx],eax
	mov	eax,4[esi]
	mov	12[ebx],eax

	pushm	ebx,ecx
	push	edi	;save end point
	push	esi	;save start point
	mov	esi,edi	;end point is first point
	mov	edi,ecx	;dest buffer
	call	compute_horz_end_vec

	pop	esi	;get back start point
	add	edi,12	;2nd vec
	call	compute_horz_end_vec

	pop	edi	;get back end point
	popm	ebx,ecx
	add	ebx,16	;past two x,y pairs
	add	ecx,24	;past two vectors

	mov	vec_ptr,ecx

;add corners to polygon

	mov	eax,8[esi]	;edge number of start edge
	mov	corner_num,eax

	mov	ecx,8[edi]	;edge number of end point
	sub	ecx,eax	;number of edges
	jns	edgenum_ok2
	add	ecx,4
edgenum_ok2:
	push	ecx	;save count
	sal	eax,3	;edge * 8
	lea	esi,corners[eax]	;first corner
	mov	edi,ebx	;rest of poly 2d points
corner_loop2:
	movsd
	movsd		;copy a corner
	cmp	esi,offset corners+8*4	;end of list?
	jne	no_wrap2
	lea	esi,corners
no_wrap2:
	pushm	ecx,esi,edi
	mov	edi,vec_ptr
	mov	eax,corner_num
	call	compute_corner_vec
	add	vec_ptr,12
	inc	corner_num
	popm	ecx,esi,edi

	loop	corner_loop2

;now return with count
	pop	eax	;get corner count
	add	eax,2	;..plus horz line end points

	popm	ebx,ecx,edx,esi,edi

	ret

;we drew all ground, so there was no horizon drawn
was_all_ground:	xor	eax,eax	;no points in poly
	ret

;we drew all sky, so find 4 corners
was_all_sky:	pushm	ebx,ecx,edx,esi,edi
	push	ecx
	lea	esi,corners
	mov	edi,ebx
	mov	ecx,8
	rep	movsd
	pop	edi

	mov	ecx,4
	xor	eax,eax	;start corner 0
sky_loop:	pushm	eax,ecx,edi
	call	compute_corner_vec
	popm	eax,ecx,edi
	add	edi,12
	inc	eax
	loop	sky_loop
	mov	eax,4	;4 corners
	popm	ebx,ecx,edx,esi,edi
	ret

;compute vector describing horizon intersection with a point.
;takes esi=2d point, edi=vec. trashes eax,ebx,ecx,edx
compute_horz_end_vec:

;compute rotated x/z & y/z ratios

	mov	eax,[esi]	;get x coord
  	sub	eax,Canv_w2
	fixdiv	Canv_w2
	mov	xfrac,eax	;save

	mov	eax,4[esi]	;get y coord
  	sub	eax,Canv_h2
	fixdiv	Canv_h2
	neg	eax	;y inversion
	mov	yfrac,eax	;save

;compute fraction unrotated x/z

	mov	eax,xfrac
	add	eax,yfrac
	mov	ecx,eax	;save
	fixmul	View_matrix.m9
	sub	eax,View_matrix.m7
	sub	eax,View_matrix.m8
	mov	ebx,eax	;save numerator

	mov	eax,ecx
	fixmul	View_matrix.m3
	mov	ecx,eax
	mov	eax,View_matrix.m1
	add	eax,View_matrix.m2
	sub	eax,ecx

;now eax/ebx = z/x. do divide in way to give result < 0

	pushm	eax,ebx
	abs_eax
	xchg	eax,ebx
	abs_eax
	cmp	eax,ebx	;which is bigger?
	popm	eax,ebx
	jl	do_xz

;x is bigger, so do as z/x

	fixdiv	ebx
	
;now eax = z/x ratio.  Compute vector by normalizing and correcting sign

	push	eax	;save ratio

	imul	eax	;compute z*z
	inc	edx	;+ x*x (x==1)
	call	quad_sqrt

	mov	ecx,eax	;mag in ecx
	pop	eax	;get ratio, x part

	fixdiv	ecx
	mov	[edi].xyz.z,eax

	mov	eax,f1_0
	fixdiv	ecx

	mov	[edi].xyz.x,eax

	jmp	finish_end

;z is bigger, so do as x/z
do_xz:
	xchg	eax,ebx
	fixdiv	ebx
	
;now eax = x/z ratio.  Compute vector by normalizing and correcting sign

	push	eax	;save ratio

	imul	eax	;compute x*x
	inc	edx	;+ z*z (z==1)
	call	quad_sqrt

	mov	ecx,eax	;mag in ecx
	pop	eax	;get ratio, x part

	fixdiv	ecx
	mov	[edi].xyz.x,eax

	mov	eax,f1_0
	fixdiv	ecx

	mov	[edi].xyz.z,eax

finish_end:	xor	eax,eax	;y = 0
	mov	[edi].xyz.y,eax

;now make sure that this vector is in front of you, not behind

	mov	eax,[edi].xyz.x
	imul	View_matrix.m3
	mov	ebx,eax
	mov	ecx,edx
	mov	eax,[edi].xyz.z
	imul	View_matrix.m9
	add	eax,ebx
	adc	edx,ecx
	jns	vec_ok	;has positive z, ok

;z is neg, flip vector

	neg	[edi].xyz.x
	neg	[edi].xyz.z
vec_ok:
	ret

MIN_DEN equ 7fffh

sub2	macro	dest,src
	mov	eax,src
	sal	eax,1
	sub	dest,eax
	endm

;compute vector decribing a corner of the screen.
;takes edi=vector, eax=corner num
compute_corner_vec:

	cmp	eax,4
	jl	num_ok
	sub	eax,4
num_ok:

;compute all deltas
	mov	ebx,View_matrix.m1
	mov	ecx,View_matrix.m4
	mov	edx,View_matrix.m7

	or	eax,eax
	jz	neg_x
	cmp	eax,3
	jne	no_neg_x
neg_x:
	neg	ebx
	neg	ecx
	neg	edx
no_neg_x:	
	sub	ebx,View_matrix.m3
	mov	m13,ebx	;m1-m3
	sub	ecx,View_matrix.m6
	mov	m46,ecx	;m4-m6
	sub	edx,View_matrix.m9
	mov	m79,edx	;m7-m9

	mov	ebx,View_matrix.m5
	mov	ecx,View_matrix.m2
	mov	edx,View_matrix.m8

	cmp	eax,2
	jl	no_neg_y
neg_y:
	neg	ebx
	neg	ecx
	neg	edx
no_neg_y:	
	sub	ebx,View_matrix.m6
	mov	m56,ebx	;m5-m6
	sub	ecx,View_matrix.m3
	mov	m23,ecx	;m2-m3
	sub	edx,View_matrix.m9
	mov	m89,edx	;m8-m9

;compute x/z ratio

;compute denomonator

	mov	eax,m46
	fixmul	m23
	mov	ebx,eax	;save

	mov	eax,m56
	fixmul	m13
	sub	eax,ebx	;eax = denominator

;now we have the denominator.  If it is too small, try x/y, z/y or z/x, y/x

	mov	ecx,eax	;save den

	abs_eax
	cmp	eax,MIN_DEN
	jl	z_too_small

z_too_small:

;now do x/z numerator

	mov	eax,m79
	fixmul	m56	;* (m5-m6)
	mov	ebx,eax

	mov	eax,m89
	fixmul	m46	;* (m4-m6)
	sub	eax,ebx

;now, eax/ecx = x/z ratio

	fixdiv	ecx	;eax = x/z

	mov	[edi].xyz.x,eax	;save x

;now do y/z

	mov	eax,m89
	fixmul	m13
	mov	ebx,eax

	mov	eax,m79
	fixmul	m23
	sub	eax,ebx

;now eax/ecx = y/z ratio

	fixdiv	ecx

	mov	[edi].xyz.y,eax

	mov	[edi].xyz.z,f1_0

	mov	esi,edi
	call	vm_vec_normalize

;make sure this vec is pointing in right direction

	lea	edi,View_matrix.vec.fvec
	call	vm_vec_dotprod
	or	eax,eax	;check sign
	jg	vec_sign_ok

	neg	[esi].xyz.x
	neg	[esi].xyz.y
	neg	[esi].xyz.z
vec_sign_ok:

	ret


_TEXT	ends

	end

*/


// Draws a polygon always facing the viewer.
// compute the corners of a rod.  fills in vertbuf.
// Verts has any needs uv's or l's or can be NULL if none needed.
int g3_draw_rod(vector *p0,float width1,vector *p1,float width2, vertex * verts, uint tmap_flags)
{
	vector uvec, fvec, rvec, center;

	vm_vec_sub( &fvec, p0, p1 );
	vm_vec_normalize_safe( &fvec );

	vm_vec_avg( &center, p0, p1 );
	vm_vec_sub( &rvec, &Eye_position, &center );
	vm_vec_normalize( &rvec );

	vm_vec_crossprod(&uvec,&fvec,&rvec);
			
	//normalize new perpendicular vector
	vm_vec_normalize(&uvec);
	 
	//now recompute right vector, in case it wasn't entirely perpendiclar
	vm_vec_crossprod(&rvec,&uvec,&fvec);

	// Now have uvec, which is up vector and rvec which is the normal
	// of the face.

	int i;
	vector vecs[4];
	vertex pts[4];
	vertex *ptlist[4] = { &pts[3], &pts[2], &pts[1], &pts[0] };

	vm_vec_scale_add( &vecs[0], p0, &uvec, width1/2.0f );
	vm_vec_scale_add( &vecs[1], p1, &uvec, width2/2.0f );
	vm_vec_scale_add( &vecs[2], p1, &uvec, -width2/2.0f );
	vm_vec_scale_add( &vecs[3], p0, &uvec, -width1/2.0f );
	
	for (i=0; i<4; i++ )	{
		if ( verts )	{
			pts[i] = verts[i];
		}
		g3_rotate_vertex( &pts[i], &vecs[i] );
	}

	return g3_draw_poly(4,ptlist,tmap_flags);
}

// draw a perspective bitmap based on angles and radius
vector g3_square[4] = {
	{-1.0f, -1.0f, 20.0f},
	{-1.0f, 1.0f, 20.0f},
	{1.0f, 1.0f, 20.0f},
	{1.0f, -1.0f, 20.0f}
};

#define MAX_PERSPECTIVE_DIVISIONS			5				// should never even come close to this limit

void stars_project_2d_onto_sphere( vector *pnt, float rho, float phi, float theta )
{		
	float a = 3.14159f * phi;
	float b = 6.28318f * theta;
	float sin_a = (float)sin(a);	

	// coords
	pnt->xyz.z = rho * sin_a * (float)cos(b);
	pnt->xyz.y = rho * sin_a * (float)sin(b);
	pnt->xyz.x = rho * (float)cos(a);
}

// draw a perspective bitmap based on angles and radius
float p_phi = 10.0f;
float p_theta = 10.0f;
int g3_draw_perspective_bitmap(angles *a, float scale_x, float scale_y, int div_x, int div_y, uint tmap_flags)
{
	vector s_points[MAX_PERSPECTIVE_DIVISIONS+1][MAX_PERSPECTIVE_DIVISIONS+1];
	vector t_points[MAX_PERSPECTIVE_DIVISIONS+1][MAX_PERSPECTIVE_DIVISIONS+1];
	vertex v[4];
	vertex *verts[4];
	matrix m, m_bank;
	int idx, s_idx;	
	int saved_zbuffer_mode;	
	float ui, vi;	
	angles bank_first;		

	// cap division values
	// div_x = div_x > MAX_PERSPECTIVE_DIVISIONS ? MAX_PERSPECTIVE_DIVISIONS : div_x;
	div_x = 1;
	div_y = div_y > MAX_PERSPECTIVE_DIVISIONS ? MAX_PERSPECTIVE_DIVISIONS : div_y;	

	// texture increment values
	ui = 1.0f / (float)div_x;
	vi = 1.0f / (float)div_y;	

	// adjust for aspect ratio
	scale_x *= ((float)gr_screen.max_w / (float)gr_screen.max_h) + 0.55f;		// fudge factor

	float s_phi = 0.5f + (((p_phi * scale_x) / 360.0f) / 2.0f);
	float s_theta = (((p_theta * scale_y) / 360.0f) / 2.0f);	
	float d_phi = -(((p_phi * scale_x) / 360.0f) / (float)(div_x));
	float d_theta = -(((p_theta * scale_y) / 360.0f) / (float)(div_y));

	// bank matrix
	bank_first.p = 0.0f;
	bank_first.b = a->b;
	bank_first.h = 0.0f;
	vm_angles_2_matrix(&m_bank, &bank_first);

	// convert angles to matrix
	float b_save = a->b;
	a->b = 0.0f;
	vm_angles_2_matrix(&m, a);
	a->b = b_save;	

	// generate the bitmap points	
	for(idx=0; idx<=div_x; idx++){
		for(s_idx=0; s_idx<=div_y; s_idx++){				
			// get world spherical coords			
			stars_project_2d_onto_sphere(&s_points[idx][s_idx], 1000.0f, s_phi + ((float)idx*d_phi), s_theta + ((float)s_idx*d_theta));			
			
			// bank the bitmap first
			vm_vec_rotate(&t_points[idx][s_idx], &s_points[idx][s_idx], &m_bank);

			// rotate on the sphere
			vm_vec_rotate(&s_points[idx][s_idx], &t_points[idx][s_idx], &m);					
		}
	}		

	// turn off zbuffering
	saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);

	// turn off culling
	gr_set_cull(0);

	// draw the bitmap
	if(Fred_running){
		tmap_flags &= ~(TMAP_FLAG_CORRECT);
	}
	// render all polys
	for(idx=0; idx<div_x; idx++){
		for(s_idx=0; s_idx<div_y; s_idx++){						
			// stuff texture coords
			v[0].u = ui * float(idx);
			v[0].v = vi * float(s_idx);
			v[0].spec_r=v[2].spec_g=v[3].spec_b=0;
			
			v[1].u = ui * float(idx+1);
			v[1].v = vi * float(s_idx);
			v[1].spec_r=v[2].spec_g=v[3].spec_b=0;

			v[2].u = ui * float(idx+1);
			v[2].v = vi * float(s_idx+1);
			v[2].spec_r=v[2].spec_g=v[3].spec_b=0;


			v[3].u = ui * float(idx);
			v[3].v = vi * float(s_idx+1);
			v[3].spec_r=v[2].spec_g=v[3].spec_b=0;

			// poly 1
			v[0].flags = 0;
			v[1].flags = 0;
			v[2].flags = 0;
			verts[0] = &v[0];
			verts[1] = &v[1];
			verts[2] = &v[2];
			g3_rotate_faraway_vertex(verts[0], &s_points[idx][s_idx]);			
			g3_rotate_faraway_vertex(verts[1], &s_points[idx+1][s_idx]);			
			g3_rotate_faraway_vertex(verts[2], &s_points[idx+1][s_idx+1]);						
			g3_draw_poly(3, verts, tmap_flags);

			// poly 2			
			v[0].flags = 0;
			v[2].flags = 0;
			v[3].flags = 0;
			verts[0] = &v[0];
			verts[1] = &v[2];
			verts[2] = &v[3];
			g3_rotate_faraway_vertex(verts[0], &s_points[idx][s_idx]);			
			g3_rotate_faraway_vertex(verts[1], &s_points[idx+1][s_idx+1]);			
			g3_rotate_faraway_vertex(verts[2], &s_points[idx][s_idx+1]);						
			g3_draw_poly(3, verts, tmap_flags);
		}
	}

	// turn on culling
	gr_set_cull(1);

	// restore zbuffer
	gr_zbuffer_set(saved_zbuffer_mode);

	// return
	return 1;
}

// draw a 2d bitmap on a poly
int g3_draw_2d_poly_bitmap(int x, int y, int w, int h, uint additional_tmap_flags)
{
	int ret;
	int saved_zbuffer_mode;
	vertex v[4];
	vertex *vertlist[4] = { &v[0], &v[1], &v[2], &v[3] };
	memset(v,0,sizeof(vertex)*4);

	int bw, bh;

	g3_start_frame(1);

	// turn off zbuffering	
	saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);	

	bm_get_section_size(gr_screen.current_bitmap, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, &bw, &bh);

	// stuff coords	
	v[0].sx = (float)x;
	v[0].sy = (float)y;	
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 0.0f;
	v[0].flags = PF_PROJECTED;
	v[0].codes = 0;

	v[1].sx = (float)(x + w);
	v[1].sy = (float)y;	
	v[1].sw = 0.0f;
	v[1].u = 1.0f;
	v[1].v = 0.0f;
	v[1].flags = PF_PROJECTED;
	v[1].codes = 0;

	v[2].sx = (float)(x + w);
	v[2].sy = (float)(y + h);	
	v[2].sw = 0.0f;
	v[2].u = 1.0f;
	v[2].v = 1.0f;
	v[2].flags = PF_PROJECTED;
	v[2].codes = 0;

	v[3].sx = (float)x;
	v[3].sy = (float)(y + h);	
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 1.0f;
	v[3].flags = PF_PROJECTED;
	v[3].codes = 0;	

	/*
	v[0].u = 0.5f / i2fl(bw);
	v[0].v = 0.5f / i2fl(bh);

	v[1].u = 1.0f + (0.5f / i2fl(bw));
	v[1].v = 0.0f + (0.5f / i2fl(bh));

	v[2].u = 1.0f + (0.5f / i2fl(bw));
	v[2].v = 1.0f + (0.5f / i2fl(bh));

	v[3].u = 0.0f + (0.5f / i2fl(bw));
	v[3].v = 1.0f + (0.5f / i2fl(bh));
	*/ 
	
	// no filtering
	gr_filter_set(0);

	// set debrief	
	ret = g3_draw_poly_constant_sw(4, vertlist, TMAP_FLAG_TEXTURED | additional_tmap_flags, 0.1f);

	g3_end_frame();
	
	gr_zbuffer_set(saved_zbuffer_mode);	

	// put filtering back on
	gr_filter_set(1);

	return ret;
}
