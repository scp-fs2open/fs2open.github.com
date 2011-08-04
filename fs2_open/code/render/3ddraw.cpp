/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "render/3dinternal.h"
#include "graphics/tmapper.h"
#include "physics/physics.h"		// For Physics_viewer_bank for g3_draw_rotated_bitmap
#include "bmpman/bmpman.h"
#include "globalincs/alphacolors.h"
#include "cmdline/cmdline.h"
#include "graphics/grbatch.h"

#include "io/key.h"


//vertex buffers for polygon drawing and clipping
static vertex **Vbuf0 = NULL;
static vertex **Vbuf1 = NULL;
static int Num_vbufs_allocated = 0;


static void g3_deallocate_vbufs()
{
	if (Vbuf0 != NULL) {
		vm_free(Vbuf0);
		Vbuf0 = NULL;
	}

	if (Vbuf1 != NULL) {
		vm_free(Vbuf1);
		Vbuf1 = NULL;
	}
}

static void g3_allocate_vbufs(int nv)
{
	static ubyte dealloc = 0;

	Assert( nv >= 0 );
	Assert( nv || Num_vbufs_allocated );

	if (!dealloc) {
		atexit(g3_deallocate_vbufs);
		dealloc = 1;
	}

	if (nv > Num_vbufs_allocated) {
		g3_deallocate_vbufs();

		Vbuf0 = (vertex**) vm_malloc( nv * sizeof(vertex) );
		Vbuf1 = (vertex**) vm_malloc( nv * sizeof(vertex) );

		Num_vbufs_allocated = nv;
	}

	// make sure everything is valid
	Verify( Vbuf0 != NULL );
	Verify( Vbuf1 != NULL );
}


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
	
	//gr_line(fl2i(p0->sx),fl2i(p0->sy),fl2i(p1->sx),fl2i(p1->sy),false);
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
#ifdef FRED_OGL_COMMENT_OUT_FOR_NOW
	if(Fred_running && !Cmdline_nohtl)
	{
  		gr_aaline( p0, p1 );
		return 0;
	}
#endif

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

//	gr_line(fl2i(p0->sx),fl2i(p0->sy),fl2i(p1->sx),fl2i(p1->sy),false);
//	gr_line_float(p0->sx,p0->sy,p1->sx,p1->sy);

  	gr_aaline( p0, p1 );

	return 0;
}


//returns true if a plane is facing the viewer. takes the unrotated surface
//normal of the plane, and a point on it.  The normal need not be normalized
int g3_check_normal_facing(vec3d *v,vec3d *norm)
{
	vec3d tempv;

	Assert( G3_count == 1 );

	vm_vec_sub(&tempv,&View_position,v);

	return (vm_vec_dot(&tempv,norm) > 0.0f);
}

int do_facing_check(vec3d *norm,vertex **vertlist,vec3d *p)
{
	Assert( G3_count == 1 );

	if (norm) {		//have normal

		Assert(norm->xyz.x || norm->xyz.y || norm->xyz.z);

		return g3_check_normal_facing(p,norm);
	}
	else {	//normal not specified, so must compute

		vec3d tempv;

		//get three points (rotated) and compute normal

		vm_vec_perp(&tempv,(vec3d *)&vertlist[0]->x,(vec3d *)&vertlist[1]->x,(vec3d *)&vertlist[2]->x);

		return (vm_vec_dot(&tempv,(vec3d *)&vertlist[1]->x ) < 0.0);
	}
}

//like g3_draw_poly(), but checks to see if facing.  If surface normal is
//NULL, this routine must compute it, which will be slow.  It is better to
//pre-compute the normal, and pass it to this function.  When the normal
//is passed, this function works like g3_check_normal_facing() plus
//g3_draw_poly().
//returns -1 if not facing, 1 if off screen, 0 if drew
int g3_draw_poly_if_facing(int nv,vertex **pointlist,uint tmap_flags,vec3d *norm,vec3d *pnt)
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

	if(!Cmdline_nohtl && (tmap_flags & TMAP_HTL_3D_UNLIT)) {
		gr_tmapper( nv, pointlist, tmap_flags );
		return 0;
	}
	//don't clip in HT&L mode, the card does it for us
	if(!Cmdline_nohtl && (tmap_flags & TMAP_FLAG_TRISTRIP)) {
		gr_tmapper( nv, pointlist, tmap_flags );
		return 0;
	}
	if(Cmdline_nohtl && (tmap_flags & TMAP_FLAG_TRISTRIP)){
		bool starting = true;
		int offset = 0;
		for (i=0;i<nv;i++){
			if (!(pointlist[i]->flags&PF_PROJECTED))g3_project_vertex(pointlist[i]);
			if (pointlist[i]->flags&PF_OVERFLOW){
				if(starting)
					offset++;
				nv--;
			}else
				starting = false;
			if(nv<3)return 1;
		}
		if(nv<3)return 1;
		pointlist += offset;
		gr_tmapper( nv, pointlist, tmap_flags );
		return 0;
	}

	g3_allocate_vbufs(nv);

	cc.cc_or = 0;
	cc.cc_and = 0xff;

	bufptr = Vbuf0;

/*	if(tmap_flags & TMAP_HTL_3D_UNLIT && !Cmdline_nohtl) {
	 	gr_tmapper( nv, pointlist, tmap_flags );
	 	return 0;
	}
*/

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

int g3_draw_polygon(vec3d *pos, matrix *ori, float width, float height, int tmap_flags)
{
	//idiot-proof
	if(width == 0 || height == 0)
			return 0;

	Assert(pos != NULL);
	Assert(ori != NULL);
	
	//Let's begin.
	
	const int NUM_VERTICES = 4;
	vec3d p[NUM_VERTICES] = { ZERO_VECTOR };
	vertex v[NUM_VERTICES];
	//float h = sqrt(2*(radius*radius));

	p[0].xyz.x = width;
	p[0].xyz.y = height;

	p[1].xyz.x = -width;
	p[1].xyz.y = height;

	p[2].xyz.x = -width;
	p[2].xyz.y = -height;

	p[3].xyz.x = width;
	p[3].xyz.y = -height;

	for(int i = 0; i < NUM_VERTICES; i++)
	{
		vec3d tmp = vmd_zero_vector;

		//Set spacing correctly
		//vm_vec_scale2(&p[i], radius, 1.0f);
		//Rotate correctly
		vm_vec_unrotate(&tmp, &p[i], ori);
		//Move to point in space
		vm_vec_add2(&tmp, pos);

		//Convert to vertex
		g3_transfer_vertex(&v[i], &tmp);
	}

	v[0].u = 1.0f;
	v[0].v = 0.0f;

	v[1].u = 0.0f;
	v[1].v = 0.0f;
	
	v[2].u = 0.0f;
	v[2].v = 1.0f;

	v[3].u = 1.0f;
	v[3].v = 1.0f;

	gr_render(NUM_VERTICES, v, tmap_flags);

	return 0;
}

int g3_draw_polygon(vec3d *pos, vec3d *norm, float width, float height, int tmap_flags)
{
	matrix m;
	vm_vector_2_matrix(&m, norm, NULL, NULL);

	return g3_draw_polygon(pos, &m, width, height, tmap_flags);
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

	if(!Cmdline_nohtl && (tmap_flags & TMAP_HTL_3D_UNLIT)) {
		gr_tmapper( nv, pointlist, tmap_flags );
		return 0;
	}
	if(tmap_flags & TMAP_FLAG_TRISTRIP || tmap_flags & TMAP_FLAG_TRILIST){
		for (i=0;i<nv;i++){
			if (!(pointlist[i]->flags&PF_PROJECTED))g3_project_vertex(pointlist[i]);
		}
		gr_tmapper( nv, pointlist, tmap_flags );
		return 0;
	}

	g3_allocate_vbufs(nv);

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

			gr_circle(fl2i(pnt->sx),fl2i(pnt->sy),fl2i(t*2.0f),false);
		}
	}

	return 0;
}

int g3_draw_sphere_ez(vec3d *pnt,float rad)
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

/*
int g3_draw_bitmap_3d(vertex *pnt,int orient, float rad,uint tmap_flags, float depth)
{

//return 0;
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and hieght rad
	vec3d PNT;
	vm_vert2vec(pnt, &PNT);
	vec3d p[4];
	vertex P[4];
	//unused variables that were there for some reason
//	matrix m;
//	vm_set_identity(&m);

	vertex *ptlist[4] = { &P[3], &P[2], &P[1], &P[0] };	
	float aspect = gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height;//seems that we have to corect for the aspect ratio

	p[0].xyz.x = rad * aspect;	p[0].xyz.y = rad;	p[0].xyz.z = -depth;
	p[1].xyz.x = -rad * aspect;	p[1].xyz.y = rad;	p[1].xyz.z = -depth;
	p[2].xyz.x = -rad * aspect;	p[2].xyz.y = -rad;	p[2].xyz.z = -depth;
	p[3].xyz.x = rad * aspect;	p[3].xyz.y = -rad;	p[3].xyz.z = -depth;

	for(int i = 0; i<4; i++){
		vec3d t = p[i];
		vm_vec_unrotate(&p[i],&t,&View_matrix);//point it at the eye
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

	int cull = gr_set_cull(0);
	g3_draw_poly(4,ptlist,tmap_flags);
	gr_set_cull(cull);

	return 0;
}
*/

//alternate method
int g3_draw_bitmap_3d(vertex *pnt, int orient, float rad, uint tmap_flags, float depth)
{
//return 0;
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and hieght rad

//	rad*=3.0f;
	vec3d PNT;
	vm_vert2vec(pnt, &PNT);
	vec3d p[4];
	vertex P[4];
	vec3d fvec, rvec, uvec;

	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize(&fvec);

	uvec = View_matrix.vec.uvec;
	vm_vec_normalize(&uvec);
	rvec = View_matrix.vec.rvec;
	vm_vec_normalize(&rvec);

	vertex *ptlist[4] = { &P[3], &P[2], &P[1], &P[0] };	
//	float aspect = gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height;//seems that we have to corect for the aspect ratio

	vm_vec_scale_add(&PNT, &PNT, &fvec, depth);
	vm_vec_scale_add(&p[0], &PNT, &rvec, rad);
	vm_vec_scale_add(&p[2], &PNT, &rvec, -rad);

	vm_vec_scale_add(&p[1], &p[2], &uvec, rad);
	vm_vec_scale_add(&p[3], &p[0], &uvec, -rad);
	vm_vec_scale_add(&p[0], &p[0], &uvec, rad);
	vm_vec_scale_add(&p[2], &p[2], &uvec, -rad);

//	p[0].xyz.x = rad * aspect;	p[0].xyz.y = rad;	p[0].xyz.z = -depth;
//	p[1].xyz.x = -rad * aspect;	p[1].xyz.y = rad;	p[1].xyz.z = -depth;
//	p[2].xyz.x = -rad * aspect;	p[2].xyz.y = -rad;	p[2].xyz.z = -depth;
//	p[3].xyz.x = rad * aspect;	p[3].xyz.y = -rad;	p[3].xyz.z = -depth;

//	for(int i = 0; i<4; i++){
//		vec3d t = p[i];
//		vm_vec_unrotate(&p[i],&t,&View_matrix);//point it at the eye
//		vm_vec_add2(&p[i],&PNT);//move it
//	}

	//move all the data from the vecs into the verts
	g3_transfer_vertex(&P[0], &p[3]);
	g3_transfer_vertex(&P[1], &p[2]);
	g3_transfer_vertex(&P[2], &p[1]);
	g3_transfer_vertex(&P[3], &p[0]);

	// set up the UV coords
	if ( orient & 1 ) {
		P[0].u = 1.0f;
		P[1].u = 0.0f;
		P[2].u = 0.0f;
		P[3].u = 1.0f;
	} else {
		P[0].u = 0.0f;
		P[1].u = 1.0f;
		P[2].u = 1.0f;
		P[3].u = 0.0f;
	}

	if ( orient & 2 ) {
		P[0].v = 1.0f;
		P[1].v = 1.0f;
		P[2].v = 0.0f;
		P[3].v = 0.0f;
	} else {
		P[0].v = 0.0f;
		P[1].v = 0.0f;
		P[2].v = 1.0f;
		P[3].v = 1.0f;
	}

//	P[0].u = 0.0f;	P[0].v = 0.0f;
//	P[1].u = 1.0f;	P[1].v = 0.0f;
//	P[2].u = 1.0f;	P[2].v = 1.0f;
//	P[3].u = 0.0f;	P[3].v = 1.0f;

	int cull = gr_set_cull(0);
	g3_draw_poly(4,ptlist,tmap_flags);
	gr_set_cull(cull);

	return 0;
}

int g3_draw_bitmap_3d_v(vertex *pnt, int orient, float rad, uint tmap_flags, float depth, float c)
{
//return 0;
//	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and hieght rad
	vec3d PNT;
	vm_vert2vec(pnt, &PNT);
	vec3d p[4];
	vertex P[4];
	int i;
	//unused variables that were there for some reason
//	matrix m;
//	vm_set_identity(&m);

	vertex *ptlist[4] = { &P[3], &P[2], &P[1], &P[0] };	
	float aspect = gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height;//seems that we have to corect for the aspect ratio

	p[0].xyz.x = rad * aspect;	p[0].xyz.y = rad;	p[0].xyz.z = -depth;
	p[1].xyz.x = -rad * aspect;	p[1].xyz.y = rad;	p[1].xyz.z = -depth;
	p[2].xyz.x = -rad * aspect;	p[2].xyz.y = -rad;	p[2].xyz.z = -depth;
	p[3].xyz.x = rad * aspect;	p[3].xyz.y = -rad;	p[3].xyz.z = -depth;

	for(i = 0; i<4; i++){
		vec3d t = p[i];
		vm_vec_unrotate(&p[i],&t,&View_matrix);//point it at the eye
		vm_vec_add2(&p[i],&PNT);//move it
	}

	//move all the data from the vecs into the verts
	g3_transfer_vertex(&P[0], &p[3]);
	g3_transfer_vertex(&P[1], &p[2]);
	g3_transfer_vertex(&P[2], &p[1]);
	g3_transfer_vertex(&P[3], &p[0]);

	for( i = 0; i<4; i++){
		P[i].r = (ubyte)(255.0f * c);
		P[i].g = (ubyte)(255.0f * c);
		P[i].b = (ubyte)(255.0f * c); 
	}

	//set up the UV coords
	P[0].u = 0.0f;	P[0].v = 0.0f;
	P[1].u = 1.0f;	P[1].v = 0.0f;
	P[2].u = 1.0f;	P[2].v = 1.0f;
	P[3].u = 0.0f;	P[3].v = 1.0f;

	int cull = gr_set_cull(0);
	g3_draw_poly(4,ptlist,tmap_flags  | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD);
	gr_set_cull(cull);

	return 0;
}

int g3_draw_bitmap_3d_volume(vertex *pnt, int orient, float rad, uint tmap_flags, float depth, int resolution)
{
	float s = 1.0f;
	float res = float(resolution);
//	float total = 0.0f;
	for(int i = 0; i <resolution; i++){
		s = (1.0f - (float(i-1)/res))/6.5f;
	//	total +=s;
		float d = (float(i)/res);
		g3_draw_bitmap_3d_v(pnt,orient, rad, tmap_flags, depth * d, s);
	}
	return 0;
}


//draws a bitmap with the specified 3d width & height 
//returns 1 if off screen, 0 if drew
// Orient
int g3_draw_bitmap(vertex *pnt, int orient, float rad, uint tmap_flags, float depth)
{
	if ( !Cmdline_nohtl && (tmap_flags & TMAP_HTL_3D_UNLIT) ) {
	//	return g3_draw_bitmap_3d_volume(pnt, orient, rad, tmap_flags, depth, 10);// just playing with an idea for makeing glows on thrusters less... not... good
		return g3_draw_bitmap_3d(pnt, orient, rad, tmap_flags, depth);
	}

	vertex va, vb;
	float t,w,h;
	float width, height;
	bool bw_bitmap = false;

	if ( tmap_flags & TMAP_FLAG_BW_TEXTURE ) {
		bw_bitmap = true;
	}

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

	gr_scaler(&va, &vb, bw_bitmap);

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

	*size = MAX(bw, bh);

	return 0;
}
/*
int g3_draw_rotated_bitmap_3d(vertex *pnt,float angle, float rad,uint tmap_flags, float depth){

//	return 0;
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and hieght rad

	angle+=Physics_viewer_bank;
	if ( angle < 0.0f )
		angle += PI2;
	else if ( angle > PI2 )
		angle -= PI2;
	float sa = (float)sin(angle);
	float ca = (float)cos(angle);

	vec3d PNT;
	vm_vert2vec(pnt, &PNT);
	vec3d p[4];
	vertex P[4];
	matrix m;
	vm_set_identity(&m);

	vertex *ptlist[4] = { &P[3], &P[2], &P[1], &P[0] };	
	float aspect = gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height;//seems that we have to corect for the aspect ratio

	p[0].xyz.x = (-rad*ca + rad*sa) * aspect;	p[0].xyz.y = (-rad*sa - rad*ca);	p[0].xyz.z = -depth;
	p[1].xyz.x = (rad*ca + rad*sa) * aspect;		p[1].xyz.y = (rad*sa - rad*ca);	p[1].xyz.z = -depth;
	p[2].xyz.x = (rad*ca - rad*sa) * aspect;		p[2].xyz.y = (rad*sa + rad*ca);	p[2].xyz.z = -depth;
	p[3].xyz.x = (-rad*ca - rad*sa) * aspect;	p[3].xyz.y = (-rad*sa + rad*ca);	p[3].xyz.z = -depth;

	for(int i = 0; i<4; i++){
		vec3d t = p[i];
		vm_vec_unrotate(&p[i],&t,&View_matrix);//point it at the eye
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

	int cull = gr_set_cull(0);
	g3_draw_poly(4,ptlist,tmap_flags);
	gr_set_cull(cull);

	return 0;
}
*/

//alternate method
int g3_draw_rotated_bitmap_3d(vertex *pnt,float angle, float rad,uint tmap_flags, float depth){
//return 0;
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and hieght rad

	angle+=Physics_viewer_bank;
	if ( angle < 0.0f )
		angle += PI2;
	else if ( angle > PI2 )
		angle -= PI2;

	vec3d PNT;
	vm_vert2vec(pnt, &PNT);
	vec3d p[4];
	vertex P[4];
	vec3d fvec, rvec, uvec;

	vm_vec_sub(&fvec, &View_position, &PNT);
	// VECMAT-ERROR: NULL VEC3D (PNT == View_position, multiplayer respawn or death sequence)
	vm_vec_normalize_safe(&fvec);

	vm_rot_point_around_line(&uvec, &View_matrix.vec.uvec, angle, &vmd_zero_vector, &View_matrix.vec.fvec);
	vm_vec_normalize(&uvec);

	vm_vec_crossprod(&rvec, &View_matrix.vec.fvec, &uvec);
	vm_vec_normalize(&rvec);

	vertex *ptlist[4] = { &P[3], &P[2], &P[1], &P[0] };	
//	float aspect = gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height;//seems that we have to corect for the aspect ratio

	vm_vec_scale_add(&PNT, &PNT, &fvec, depth);
	vm_vec_scale_add(&p[0], &PNT, &rvec, rad);
	vm_vec_scale_add(&p[2], &PNT, &rvec, -rad);

	vm_vec_scale_add(&p[1], &p[2], &uvec, rad);
	vm_vec_scale_add(&p[3], &p[0], &uvec, -rad);
	vm_vec_scale_add(&p[0], &p[0], &uvec, rad);
	vm_vec_scale_add(&p[2], &p[2], &uvec, -rad);


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

	int cull = gr_set_cull(0);
	g3_draw_poly(4,ptlist,tmap_flags);
	gr_set_cull(cull);

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
		if ( angle < PI_2 )	{
			ang = 0;
		} else if ( angle < PI )	{
			ang = 1;
		} else if ( angle < PI+PI_2 )	{
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

#define TRIANGLE_AREA(_p, _q, _r)	do { vec3d a, b, cross; a.xyz.x = _q->x - _p->x; a.xyz.y = _q->y - _p->y; a.xyz.z = 0.0f; b.xyz.x = _r->x - _p->x; b.xyz.y = _r->y - _p->y; b.xyz.z = 0.0f; vm_vec_crossprod(&cross, &a, &b); total_area += vm_vec_mag(&cross) * 0.5f; } while(0);
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

//	if ( nv > TMAP_MAX_VERTS ) {
//		Int3();
//		return 0.0f;
//	}

	g3_allocate_vbufs(nv);

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

	vec3d horizon_vec;
	
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
	gr_line( fl2i(horz_pts[0].x),fl2i(horz_pts[0].y),fl2i(horz_pts[1].x),fl2i(horz_pts[1].y), false );
	
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

;compute vec3d describing horizon intersection with a point.
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
	
;now eax = z/x ratio.  Compute vec3d by normalizing and correcting sign

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
	
;now eax = x/z ratio.  Compute vec3d by normalizing and correcting sign

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

;now make sure that this vec3d is in front of you, not behind

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

;compute vec3d decribing a corner of the screen.
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
int g3_draw_rod(vec3d *p0,float width1,vec3d *p1,float width2, vertex * verts, uint tmap_flags)
{
	vec3d uvec, fvec, rvec, center;

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
	vec3d vecs[4];
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
		if(Cmdline_nohtl)
			g3_rotate_vertex( &pts[i], &vecs[i] );
		else
			g3_transfer_vertex( &pts[i], &vecs[i] );
	}
	ptlist[0]->u = 0.0f;
	ptlist[0]->v = 0.0f;
	ptlist[0]->r = gr_screen.current_color.red;
	ptlist[0]->g = gr_screen.current_color.green;
	ptlist[0]->b = gr_screen.current_color.blue;
	ptlist[0]->a = gr_screen.current_color.alpha;

	ptlist[1]->u = 1.0f;
	ptlist[1]->v = 0.0f;
	ptlist[1]->r = gr_screen.current_color.red;
	ptlist[1]->g = gr_screen.current_color.green;
	ptlist[1]->b = gr_screen.current_color.blue;
	ptlist[1]->a = gr_screen.current_color.alpha;

	ptlist[2]->u = 1.0f;
	ptlist[2]->v = 1.0f;
	ptlist[2]->r = gr_screen.current_color.red;
	ptlist[2]->g = gr_screen.current_color.green;
	ptlist[2]->b = gr_screen.current_color.blue;
	ptlist[2]->a = gr_screen.current_color.alpha;

	ptlist[3]->u = 0.0f;
	ptlist[3]->v = 1.0f;
	ptlist[3]->r = gr_screen.current_color.red;
	ptlist[3]->g = gr_screen.current_color.green;
	ptlist[3]->b = gr_screen.current_color.blue;
	ptlist[3]->a = gr_screen.current_color.alpha;

	return g3_draw_poly(4,ptlist,tmap_flags);
}

#define MAX_ROD_VECS	100
int g3_draw_rod(int num_points, vec3d *pvecs, float width, uint tmap_flags)
{
	vec3d uvec, fvec, rvec;
	vec3d vecs[2];
	vertex pts[MAX_ROD_VECS];
	vertex *ptlist[MAX_ROD_VECS];
	int i, nv = 0;

	Assert( num_points >= 2 );
	Assert( (num_points * 2) <= MAX_ROD_VECS );


	for (i = 0; i < num_points; i++) {
		vm_vec_sub(&fvec, &View_position, &pvecs[i]);
		vm_vec_normalize_safe(&fvec);

		int first = i+1;
		int second = i-1;

		if (i == 0) {
			first = 1;
			second = 0;
		} else if (i == num_points-1) {
			first = i;
		}

		vm_vec_sub(&rvec, &pvecs[first], &pvecs[second]);
		vm_vec_normalize_safe(&rvec);

		vm_vec_crossprod(&uvec, &rvec, &fvec);

		vm_vec_scale_add(&vecs[0], &pvecs[i], &uvec, width * 0.5f);
		vm_vec_scale_add(&vecs[1], &pvecs[i], &uvec, -width * 0.5f);


		if (nv > MAX_ROD_VECS-2) {
			Warning(LOCATION, "Hit high-water mark (%i) in g3_draw_rod()!!\n", MAX_ROD_VECS);
			break;
		}

		ptlist[nv] = &pts[nv];
		ptlist[nv+1] = &pts[nv+1];

		if (Cmdline_nohtl) {
			g3_rotate_vertex( &pts[nv], &vecs[0] );
			g3_rotate_vertex( &pts[nv+1], &vecs[1] );
		} else {
			g3_transfer_vertex( &pts[nv], &vecs[0] );
			g3_transfer_vertex( &pts[nv+1], &vecs[1] );
		}

		ptlist[nv]->u = 1.0f;
		ptlist[nv]->v = i2fl(i);
		ptlist[nv]->r = gr_screen.current_color.red;
		ptlist[nv]->g = gr_screen.current_color.green;
		ptlist[nv]->b = gr_screen.current_color.blue;
		ptlist[nv]->a = gr_screen.current_color.alpha;

		ptlist[nv+1]->u = 0.0f;
		ptlist[nv+1]->v = i2fl(i);
		ptlist[nv+1]->r = gr_screen.current_color.red;
		ptlist[nv+1]->g = gr_screen.current_color.green;
		ptlist[nv+1]->b = gr_screen.current_color.blue;
		ptlist[nv+1]->a = gr_screen.current_color.alpha;

		nv += 2;
	}

	// we should always have at least 4 verts, and there should always be an even number
	Assert( (nv >= 4) && !(nv % 2) );

	int rc = g3_draw_poly(nv, ptlist, tmap_flags);

	return rc;
}

// draw a perspective bitmap based on angles and radius
vec3d g3_square[4] = {
	{ { { -1.0f, -1.0f, 20.0f } } },
	{ { { -1.0f, 1.0f, 20.0f } } },
	{ { { 1.0f, 1.0f, 20.0f } } },
	{ { { 1.0f, -1.0f, 20.0f } } }
};

#define MAX_PERSPECTIVE_DIVISIONS			5				// should never even come close to this limit

void stars_project_2d_onto_sphere( vec3d *pnt, float rho, float phi, float theta )
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
	vec3d s_points[MAX_PERSPECTIVE_DIVISIONS+1][MAX_PERSPECTIVE_DIVISIONS+1];
	vec3d t_points[MAX_PERSPECTIVE_DIVISIONS+1][MAX_PERSPECTIVE_DIVISIONS+1];
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
	int cull = gr_set_cull(0);

	// render all polys
	for(idx=0; idx<div_x; idx++){
		for(s_idx=0; s_idx<div_y; s_idx++){						
			// stuff texture coords
			v[0].u = ui * float(idx);
			v[0].v = vi * float(s_idx);
			v[0].spec_r=v[0].spec_g=v[0].spec_b=0;
			
			v[1].u = ui * float(idx+1);
			v[1].v = vi * float(s_idx);
			v[1].spec_r=v[1].spec_g=v[1].spec_b=0;

			v[2].u = ui * float(idx+1);
			v[2].v = vi * float(s_idx+1);
			v[2].spec_r=v[2].spec_g=v[2].spec_b=0;


			v[3].u = ui * float(idx);
			v[3].v = vi * float(s_idx+1);
			v[3].spec_r=v[3].spec_g=v[3].spec_b=0;

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
	gr_set_cull(cull);

	// restore zbuffer
	gr_zbuffer_set(saved_zbuffer_mode);

	// return
	return 1;
}

void g3_draw_2d_rect(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int saved_zbuf;
	vertex v[4];
	vertex *verts[4] = {&v[0], &v[1], &v[2], &v[3]};

	memset(v,0,sizeof(vertex)*4);
	saved_zbuf = gr_zbuffer_get();

	// start the frame, no zbuffering, no culling
	if (!Fred_running)
		g3_start_frame(1);

	gr_zbuffer_set(GR_ZBUFF_NONE);		
	int cull = gr_set_cull(0);		

	// stuff coords		
	v[0].sx = i2fl(x);
	v[0].sy = i2fl(y);
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 0.0f;
	v[0].flags = PF_PROJECTED;
	v[0].codes = 0;
	v[0].r = (ubyte)r;
	v[0].g = (ubyte)g;
	v[0].b = (ubyte)b;
	v[0].a = (ubyte)a;

	v[1].sx = i2fl(x + w);
	v[1].sy = i2fl(y);	
	v[1].sw = 0.0f;
	v[1].u = 0.0f;
	v[1].v = 0.0f;
	v[1].flags = PF_PROJECTED;
	v[1].codes = 0;
	v[1].r = (ubyte)r;
	v[1].g = (ubyte)g;
	v[1].b = (ubyte)b;
	v[1].a = (ubyte)a;

	v[2].sx = i2fl(x + w);
	v[2].sy = i2fl(y + h);
	v[2].sw = 0.0f;
	v[2].u = 0.0f;
	v[2].v = 0.0f;
	v[2].flags = PF_PROJECTED;
	v[2].codes = 0;
	v[2].r = (ubyte)r;
	v[2].g = (ubyte)g;
	v[2].b = (ubyte)b;
	v[2].a = (ubyte)a;

	v[3].sx = i2fl(x);
	v[3].sy = i2fl(y + h);
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 0.0f;
	v[3].flags = PF_PROJECTED;
	v[3].codes = 0;				
	v[3].r = (ubyte)r;
	v[3].g = (ubyte)g;
	v[3].b = (ubyte)b;
	v[3].a = (ubyte)a;

	// draw the polys
	g3_draw_poly_constant_sw(4, verts, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_ALPHA, 0.1f);		

	if (!Fred_running)
		g3_end_frame();


	// restore zbuffer and culling
	gr_zbuffer_set(saved_zbuf);
	gr_set_cull(cull);
}

// draw a 2d bitmap on a poly
int g3_draw_2d_poly_bitmap(float x, float y, float w, float h, uint additional_tmap_flags)
{
	int ret;
	int saved_zbuffer_mode;
	vertex v[4];
	vertex *vertlist[4] = { &v[0], &v[1], &v[2], &v[3] };
	memset(v,0,sizeof(vertex)*4);

	g3_start_frame(1);

	// turn off zbuffering	
	saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);	

	// stuff coords	
	v[0].sx = x;
	v[0].sy = y;	
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 0.0f;
	v[0].flags = PF_PROJECTED;
	v[0].codes = 0;

	v[1].sx = (x + w);
	v[1].sy = y;	
	v[1].sw = 0.0f;
	v[1].u = 1.0f;
	v[1].v = 0.0f;
	v[1].flags = PF_PROJECTED;
	v[1].codes = 0;

	v[2].sx = (x + w);
	v[2].sy = (y + h);	
	v[2].sw = 0.0f;
	v[2].u = 1.0f;
	v[2].v = 1.0f;
	v[2].flags = PF_PROJECTED;
	v[2].codes = 0;

	v[3].sx = x;
	v[3].sy = (y + h);	
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 1.0f;
	v[3].flags = PF_PROJECTED;
	v[3].codes = 0;

	// set debrief	
	ret = g3_draw_poly_constant_sw(4, vertlist, TMAP_FLAG_TEXTURED | additional_tmap_flags, 0.1f);

	g3_end_frame();
	
	gr_zbuffer_set(saved_zbuffer_mode);	

	return ret;
}

vertex *bitmap_2d_poly_list=NULL;
vertex **bitmap_2d_poly_vertlist=NULL;
int bitmap_2d_poly_list_size=0;

void bm_list_shutdown(){
	if(bitmap_2d_poly_list)delete[]bitmap_2d_poly_list;
	if(bitmap_2d_poly_vertlist)delete[]bitmap_2d_poly_vertlist;
}

int g3_draw_2d_poly_bitmap_list(bitmap_2d_list* b_list, int n_bm, uint additional_tmap_flags)
{
	int ret;
	int saved_zbuffer_mode;
	if(n_bm>bitmap_2d_poly_list_size){
		if(bitmap_2d_poly_list)delete[]bitmap_2d_poly_list;
		if(bitmap_2d_poly_vertlist)delete[]bitmap_2d_poly_vertlist;
		bitmap_2d_poly_list = new vertex[6* n_bm];
		bitmap_2d_poly_vertlist = new vertex*[6*n_bm];
		for(int i = 0; i<6*n_bm; i++)bitmap_2d_poly_vertlist[i] = &bitmap_2d_poly_list[i];
		memset(bitmap_2d_poly_list,0,sizeof(vertex)*6*n_bm);
	}

	g3_start_frame(1);

	// turn off zbuffering	
	saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);	

	for(int i = 0; i<n_bm;i++){
		// stuff coords	

		//tri one
		vertex *V = &bitmap_2d_poly_list[i*6];
		V->sx = (float)b_list[i].x;
		V->sy = (float)b_list[i].y;	
		V->sw = 0.0f;
		V->u = 0.0f;
		V->v = 0.0f;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->sx = (float)(b_list[i].x + b_list[i].w);
		V->sy = (float)b_list[i].y;	
		V->sw = 0.0f;
		V->u = 1.0f;
		V->v = 0.0f;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->sx = (float)(b_list[i].x + b_list[i].w);
		V->sy = (float)(b_list[i].y + b_list[i].h);	
		V->sw = 0.0f;
		V->u = 1.0f;
		V->v = 1.0f;
		V->flags = PF_PROJECTED;
		V->codes = 0;
	

		//tri two
		V++;
		V->sx = (float)b_list[i].x;
		V->sy = (float)b_list[i].y;	
		V->sw = 0.0f;
		V->u = 0.0f;
		V->v = 0.0f;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->sx = (float)(b_list[i].x + b_list[i].w);
		V->sy = (float)(b_list[i].y + b_list[i].h);	
		V->sw = 0.0f;
		V->u = 1.0f;
		V->v = 1.0f;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->sx = (float)b_list[i].x;
		V->sy = (float)(b_list[i].y + b_list[i].h);	
		V->sw = 0.0f;
		V->u = 0.0f;
		V->v = 1.0f;
		V->flags = PF_PROJECTED;
		V->codes = 0;	
	}

	// set debrief	
	ret = g3_draw_poly_constant_sw(6*n_bm, bitmap_2d_poly_vertlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_TRILIST | additional_tmap_flags, 0.1f);

	g3_end_frame();
	
	gr_zbuffer_set(saved_zbuffer_mode);	

	return ret;
}

//draws an array of bitmap_rect_list objects
//see tmapper.h for explaination of structure bitmap_rect_list
int g3_draw_2d_poly_bitmap_rect_list(bitmap_rect_list* b_list, int n_bm, uint additional_tmap_flags)
{
	int ret;
	int saved_zbuffer_mode;
	if(n_bm>bitmap_2d_poly_list_size){
		if(bitmap_2d_poly_list)delete[]bitmap_2d_poly_list;
		if(bitmap_2d_poly_vertlist)delete[]bitmap_2d_poly_vertlist;
		bitmap_2d_poly_list = new vertex[6* n_bm];
		bitmap_2d_poly_vertlist = new vertex*[6*n_bm];
		for(int i = 0; i<6*n_bm; i++)bitmap_2d_poly_vertlist[i] = &bitmap_2d_poly_list[i];
	//	memset(bitmap_2d_poly_list,0,sizeof(vertex)*6*n_bm);
	}

	g3_start_frame(1);

	// turn off zbuffering	
	saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);	

	for(int i = 0; i<n_bm;i++){
		// stuff coords	

		bitmap_2d_list* b = &b_list[i].screen_rect;
		texture_rect_list* t = &b_list[i].texture_rect;
		//tri one
		vertex *V = &bitmap_2d_poly_list[i*6];
		V->sx = (float)b->x;
		V->sy = (float)b->y;	
		V->sw = 0.0f;
		V->u = (float)t->x;
		V->v = (float)t->y;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->sx = (float)(b->x + b->w);
		V->sy = (float)b->y;	
		V->sw = 0.0f;
		V->u = (float)(t->x + t->w);
		V->v = (float)t->y;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->sx = (float)(b->x + b->w);
		V->sy = (float)(b->y + b->h);	
		V->sw = 0.0f;
		V->u = (float)(t->x + t->w);
		V->v = (float)(t->y + t->h);
		V->flags = PF_PROJECTED;
		V->codes = 0;
	

		//tri two
		V++;
		V->sx = (float)b->x;
		V->sy = (float)b->y;	
		V->sw = 0.0f;
		V->u = (float)t->x;
		V->v = (float)t->y;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->sx = (float)(b->x + b->w);
		V->sy = (float)(b->y + b->h);	
		V->sw = 0.0f;
		V->u = (float)(t->x + t->w);
		V->v = (float)(t->y + t->h);
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->sx = (float)b->x;
		V->sy = (float)(b->y + b->h);	
		V->sw = 0.0f;
		V->u = (float)t->x;
		V->v = (float)(t->y + t->h);
		V->flags = PF_PROJECTED;
		V->codes = 0;	
	}

	// set debrief	
	ret = g3_draw_poly_constant_sw(6*n_bm, bitmap_2d_poly_vertlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_TRILIST | additional_tmap_flags, 0.1f);

	g3_end_frame();
	
	gr_zbuffer_set(saved_zbuffer_mode);	

	return ret;
}

void g3_draw_htl_line(vec3d *start, vec3d *end)
{
	if (Cmdline_nohtl) {
		return;
	}

	gr_line_htl(start,end);
}

void g3_draw_htl_sphere(vec3d* position, float radius)
{
	if (Cmdline_nohtl) {
		return;
	}

	g3_start_instance_matrix(position, &vmd_identity_matrix, true);

	gr_sphere_htl(radius);

	g3_done_instance(true);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//flash ball stuff

void flash_ball::initialize(int number, float min_ray_width, float max_ray_width, vec3d* dir, vec3d*pcenter, float outer, float inner, ubyte max_r, ubyte max_g, ubyte max_b, ubyte min_r, ubyte min_g, ubyte min_b)
{
	if(number < 1)
		return;

	center = *pcenter;

	if(n_rays < number){
		if(ray)vm_free(ray);
		ray = (flash_beam*)vm_malloc(sizeof(flash_beam)*number);
		n_rays = number;
	}

	int i;
	for(i = 0; i<n_rays; i++){
	//colors
		if(min_r != 255){
			ray[i].start.r = (rand()%(max_r-min_r))+min_r;
		}else{
			ray[i].start.r = 255;
		}
		if(min_g != 255){
			ray[i].start.g = (rand()%(max_g-min_g))+min_g;
		}else{
			ray[i].start.g = 255;
		}
		if(min_b != 255){
			ray[i].start.b = (rand()%(max_b-min_b))+min_b;
		}else{
			ray[i].start.b = 255;
		}

	//rays
		if(dir == &vmd_zero_vector || outer >= PI2){
			//random sphere
			vec3d start, end;

			vm_vec_rand_vec_quick(&start);
			vm_vec_rand_vec_quick(&end);

			ray[i].start = start;
			ray[i].end = end;
		}else{
			//random cones
			vec3d start, end;

			vm_vec_random_cone(&start, dir, outer, inner);
			vm_vec_random_cone(&end, dir, outer, inner);

			ray[i].start = start;
			ray[i].end = end;
		}
		if(max_ray_width == 0.0f)ray[i].width=min_ray_width;
		else ray[i].width = frand_range(min_ray_width, max_ray_width);
	}


}

#define uw(p)	(*((uint *) (p)))
#define w(p)	(*((int *) (p)))
#define wp(p)	((int *) (p))
#define vp(p)	((vec3d *) (p))
#define fl(p)	(*((float *) (p)))

void flash_ball::defpoint(int off, ubyte *bsp_data)
{
	int n;
//	off+=4;
	int nverts = w(off+bsp_data+8);	
	int offset = w(off+bsp_data+16);
//	int next_norm = 0;

	ubyte * normcount = off+bsp_data+20;
	vec3d *src = vp(off+bsp_data+offset);


	if(n_rays < nverts){
		if(ray)vm_free(ray);
		ray = (flash_beam*)vm_malloc(sizeof(flash_beam)*nverts);
		n_rays = nverts;
	}

	{


		vec3d temp;
		for (n=0; n<nverts; n++ )	{

			temp = *src;
			vm_vec_sub2(&temp, &center);
			vm_vec_normalize(&temp);
			ray[n].start = temp;
		
			src++;		// move to normal

			src+=normcount[n];
		}
	}



}
#define OP_EOF 			0
#define OP_DEFPOINTS 	1
#define OP_FLATPOLY		2
#define OP_TMAPPOLY		3
#define OP_SORTNORM		4
#define OP_BOUNDBOX		5


void flash_ball::parse_bsp(int offset, ubyte *bsp_data){
	int ID, SIZE;

	memcpy(&ID, &bsp_data[offset], sizeof(int));
	memcpy(&SIZE, &bsp_data[offset+sizeof(int)], sizeof(int));

	while(ID!=0){
		switch(ID){
		case OP_EOF:	
			return;
			break;
		case OP_DEFPOINTS:	defpoint(offset, bsp_data);
			break;
		case OP_SORTNORM:
			break;
		case OP_FLATPOLY:
			break;
		case OP_TMAPPOLY:
			break;
		case OP_BOUNDBOX:
			break;
		default:
			return;
		}
			offset += SIZE;
		memcpy(&ID, &bsp_data[offset], sizeof(int));
		memcpy(&SIZE, &bsp_data[offset+sizeof(int)], sizeof(int));

		if(SIZE < 1)ID=OP_EOF;
	}
}


void flash_ball::initialize(ubyte *bsp_data, float min_ray_width, float max_ray_width, vec3d* dir , vec3d*pcenter , float outer , float inner , ubyte max_r , ubyte max_g , ubyte max_b , ubyte min_r , ubyte min_g , ubyte min_b )
{
	center = *pcenter;
	vm_vec_negate(&center);
	parse_bsp(0,bsp_data);
	center = vmd_zero_vector;

	int i;
	for(i = 0; i<n_rays; i++){
	//colors
		if(min_r != 255){
			ray[i].start.r = (rand()%(max_r-min_r))+min_r;
		}else{
			ray[i].start.r = 255;
		}
		if(min_g != 255){
			ray[i].start.g = (rand()%(max_g-min_g))+min_g;
		}else{
			ray[i].start.g = 255;
		}
		if(min_b != 255){
			ray[i].start.b = (rand()%(max_b-min_b))+min_b;
		}else{
			ray[i].start.b = 255;
		}

	//rays
		if(dir == &vmd_zero_vector || outer >= PI2){
			//random sphere
			vec3d end;

			vm_vec_rand_vec_quick(&end);

			ray[i].end = end;
		}else{
			//random cones
			vec3d end;

			vm_vec_random_cone(&end, dir, outer, inner);

			ray[i].end = end;
		}
		if(max_ray_width == 0.0f)ray[i].width=min_ray_width;
		else ray[i].width = frand_range(min_ray_width, max_ray_width);
	}
}

//rad		how wide the ball should be
//intinsity	how visable it should be
//life		how far along from start to end should it be
void flash_ball::render(float rad, float intinsity, float life){
	flash_ball::batcher.allocate(n_rays);
	for(int i = 0; i<n_rays; i++){
		vec3d end;
		vm_vec_interp_constant(&end, (vec3d*)&ray[i].start.x,  (vec3d*)&ray[i].end.x, life);
		vm_vec_scale(&end, rad);
		vm_vec_add2(&end, &center);
		flash_ball::batcher.draw_beam(&center, &end, ray[i].width*rad, intinsity);
	}
	flash_ball::batcher.render(TMAP_FLAG_TEXTURED | TMAP_FLAG_XPARENT | TMAP_HTL_3D_UNLIT | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_FLAG_CORRECT);
}

geometry_batcher flash_ball::batcher;
