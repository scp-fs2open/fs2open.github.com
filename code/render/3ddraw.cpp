/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "bmpman/bmpman.h"
#include "cmdline/cmdline.h"
#include "globalincs/alphacolors.h"
#include "graphics/grbatch.h"
#include "graphics/tmapper.h"
#include "graphics/matrix.h"
#include "io/key.h"
#include "physics/physics.h"		// For Physics_viewer_bank for g3_draw_rotated_bitmap
#include "render/3dinternal.h"
#include "render/batching.h"

/**
 * Deal with a clipped line
 */
int must_clip_line(vertex *p0, vertex *p1, ubyte codes_or, uint flags)
{
	int ret = 0;
	
	clip_line(&p0, &p1, codes_or, flags);
	
	if (p0->codes & p1->codes) goto free_points;

	codes_or = (unsigned char)(p0->codes | p1->codes);

	if (codes_or & CC_BEHIND) goto free_points;

	if (!(p0->flags&PF_PROJECTED))
		g3_project_vertex(p0);

	if (p0->flags&PF_OVERFLOW) goto free_points;

	if (!(p1->flags&PF_PROJECTED))
		g3_project_vertex(p1);

	if (p1->flags&PF_OVERFLOW) goto free_points;

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

/**
 * Draws a line. takes two points.  returns true if drew
 */
int g3_draw_line(vertex *p0, vertex *p1)
{
#ifdef FRED_OGL_COMMENT_OUT_FOR_NOW
	if(Fred_running)
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
		return must_clip_line(p0,p1,codes_or,0);


	if (!(p1->flags&PF_PROJECTED))
		g3_project_vertex(p1);

	if (p1->flags&PF_OVERFLOW)
		return must_clip_line(p0,p1,codes_or,0);

  	gr_aaline( p0, p1 );

	return 0;
}


//returns true if a plane is facing the viewer. takes the unrotated surface
//normal of the plane, and a point on it.  The normal need not be normalized
int g3_check_normal_facing(const vec3d *v, const vec3d *norm)
{
	vec3d tempv;

	Assert( G3_count == 1 );

	vm_vec_sub(&tempv,&View_position,v);

	return (vm_vec_dot(&tempv,norm) > 0.0f);
}

//draw a sortof sphere - i.e., the 2d radius is proportional to the 3d
//radius, but not to the distance from the eye
int g3_draw_sphere(vertex *pnt, float rad)
{
	Assert( G3_count == 1 );

	if (! (pnt->codes & CC_BEHIND)) {

		if (! (pnt->flags & PF_PROJECTED))
			g3_project_vertex(pnt);

		if (! (pnt->codes & PF_OVERFLOW)) {
			float r2,t;

			r2 = rad*Matrix_scale.xyz.x;

			t=r2*Canv_w2/pnt->world.xyz.z;

			gr_circle(fl2i(pnt->screen.xyw.x),fl2i(pnt->screen.xyw.y),fl2i(t*2.0f),GR_RESIZE_NONE);
		}
	}

	return 0;
}

int g3_draw_sphere_ez(const vec3d *pnt, float rad)
{
	vertex pt;
	ubyte flags;

	Assert( G3_count == 1 );

	flags = g3_rotate_vertex(&pt, pnt);

	if (flags == 0) {

		g3_project_vertex(&pt);

		if (!(pt.flags & PF_OVERFLOW))	{

			g3_draw_sphere( &pt, rad );
		}
	}

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

	t = (width*Canv_w2)/pnt->world.xyz.z;
	*w = (int)(t*Matrix_scale.xyz.x);

	t = (height*Canv_h2)/pnt->world.xyz.z;
	*h = (int)(t*Matrix_scale.xyz.y);	

	*x = (int)(pnt->screen.xyw.x - *w/2.0f);
	*y = (int)(pnt->screen.xyw.y - *h/2.0f);	

	*size = MAX(bw, bh);

	return 0;
}

#include "graphics/2d.h"

//draws a horizon. takes eax=sky_color, edx=ground_color
void g3_draw_horizon_line()
{
	int s1, s2;
	int cpnt;
	horz_pt horz_pts[4];		// 0 = left, 1 = right
	vec3d horizon_vec;
	float up_right, down_right,down_left,up_left;

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
		return;
	}

	if ( (up_right>0.0f)&&(down_right>0.0f)&&(down_left>0.0f)&&(up_left>0.0f) )	{
		return;
	}

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
	gr_line( fl2i(horz_pts[0].x),fl2i(horz_pts[0].y),fl2i(horz_pts[1].x),fl2i(horz_pts[1].y), GR_RESIZE_NONE );
}

// draw a perspective bitmap based on angles and radius
vec3d g3_square[4] = {
	{ { { -1.0f, -1.0f, 20.0f } } },
	{ { { -1.0f, 1.0f, 20.0f } } },
	{ { { 1.0f, 1.0f, 20.0f } } },
	{ { { 1.0f, -1.0f, 20.0f } } }
};

void stars_project_2d_onto_sphere( vec3d *pnt, float rho, float phi, float theta )
{		
	float a = PI * phi;
	float b = PI2 * theta;
	float sin_a = sinf(a);	

	// coords
	pnt->xyz.z = rho * sin_a * cosf(b);
	pnt->xyz.y = rho * sin_a * sinf(b);
	pnt->xyz.x = rho * cosf(a);
}

void g3_draw_htl_line(const vec3d *start, const vec3d *end)
{
	//gr_line_htl(start, end);
	g3_render_line_3d(true, start, end);
}

void g3_draw_htl_sphere(color* clr,
	const vec3d* position,
	float radius,
	gr_alpha_blend alpha_blend_mode,
	gr_zbuffer_type zbuffer_mode)
{
	g3_start_instance_matrix(position, &vmd_identity_matrix, true);

	material material_def;

	material_def.set_blend_mode(alpha_blend_mode);
	material_def.set_depth_mode(zbuffer_mode);
	material_def.set_color(*clr);

	gr_sphere(&material_def, radius);

	g3_done_instance(true);
}

void g3_draw_htl_sphere(color *clr, const vec3d* position, float radius)
{
	g3_draw_htl_sphere(clr, position, radius, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL);
}

void g3_draw_htl_sphere(const vec3d* position, float radius)
{
	g3_draw_htl_sphere(&gr_screen.current_color, position, radius);
}

void g3_render_primitives(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic)
{
	vertex_layout layout;

	if ( orthographic ) {
		layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), (int)offsetof(vertex, screen));
	} else {
		layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), (int)offsetof(vertex, world));
	}

	if ( orthographic ) {
		gr_render_primitives_2d_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	} else {
		gr_render_primitives_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	}
}

void g3_render_primitives_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic)
{
	vertex_layout layout;

	if ( orthographic ) {
		layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), (int)offsetof(vertex, screen));
	} else {
		layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), (int)offsetof(vertex, world));
	}

	layout.add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(vertex), (int)offsetof(vertex, texture_position));

	if ( orthographic ) {
		gr_render_primitives_2d_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	} else {
		gr_render_primitives_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	}
}

void g3_render_primitives_colored(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic)
{
	vertex_layout layout;

	if ( orthographic ) {
		layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), (int)offsetof(vertex, screen));
	} else {
		layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), (int)offsetof(vertex, world));
	}

	layout.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), (int)offsetof(vertex, r));

	if ( orthographic ) {
		gr_render_primitives_2d_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	} else {
		gr_render_primitives_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	}
}

void g3_render_primitives_colored_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic)
{
	vertex_layout layout;

	if ( orthographic ) {
		layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), (int)offsetof(vertex, screen));
	} else {
		layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), (int)offsetof(vertex, world));
	}

	layout.add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(vertex), (int)offsetof(vertex, texture_position));
	layout.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), (int)offsetof(vertex, r));

	if ( orthographic ) {
		gr_render_primitives_2d_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	} else {
		gr_render_primitives_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	}
}

// adapted from g3_draw_polygon()
void g3_render_rect_oriented_internal(material* material_params, vec3d *pos, matrix *ori, float width, float height)
{
	//idiot-proof
	if ( width == 0 || height == 0 )
		return;

	Assert(pos != NULL);
	Assert(ori != NULL);

	//Let's begin.

	const int NUM_VERTICES = 4;
	vec3d p[NUM_VERTICES] = { ZERO_VECTOR };
	vertex v[NUM_VERTICES];

	memset(v, 0, sizeof(v));

	p[0].xyz.x = width;
	p[0].xyz.y = height;

	p[1].xyz.x = -width;
	p[1].xyz.y = height;

	p[2].xyz.x = -width;
	p[2].xyz.y = -height;

	p[3].xyz.x = width;
	p[3].xyz.y = -height;

	for ( int i = 0; i < NUM_VERTICES; i++ ) {
		vec3d tmp = vmd_zero_vector;

		//Rotate correctly
		vm_vec_unrotate(&tmp, &p[i], ori);
		//Move to point in space
		vm_vec_add2(&tmp, pos);

		//Convert to vertex
		g3_transfer_vertex(&v[i], &tmp);
	}

	v[0].texture_position.u = 1.0f;
	v[0].texture_position.v = 0.0f;

	v[1].texture_position.u = 0.0f;
	v[1].texture_position.v = 0.0f;

	v[2].texture_position.u = 0.0f;
	v[2].texture_position.v = 1.0f;

	v[3].texture_position.u = 1.0f;
	v[3].texture_position.v = 1.0f;

	g3_render_primitives_textured(material_params, v, 4, PRIM_TYPE_TRIFAN, false);
}

void g3_render_rect_oriented(material* mat_info, vec3d *pos, matrix *ori, float width, float height)
{
	g3_render_rect_oriented_internal(mat_info, pos, ori, width, height);
}

void g3_render_rect_oriented(material* mat_info, vec3d *pos, vec3d *norm, float width, float height)
{
	matrix m;
	vm_vector_2_matrix(&m, norm, NULL, NULL);

	g3_render_rect_oriented_internal(mat_info, pos, &m, width, height);
}

// adapted g3_draw_rotated_bitmap_3d()
//void render_rotated_bitmap(int texture, float alpha, vertex *pnt, float angle, float rad)
void g3_render_rect_screen_aligned_rotated(material *mat_params, vertex *pnt, float angle, float rad)
{
	// holdover mistake from retail causes these bitmaps to be rendered 41% bigger than rad
	// this turns radius into the diagonal distance, but the methods below presume manhattan distance (unadjusted radius)
	rad *= 1.41421356f;

	angle -= Physics_viewer_bank;
	if ( angle < 0.0f ) {
		angle += PI2;
	} else if ( angle > PI2 ) {
		angle -= PI2;
	}

	vec3d PNT(pnt->world);
	vec3d p[4];
	vertex P[4];
	vec3d fvec, rvec, uvec;

	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize_safe(&fvec);

	vm_rot_point_around_line(&uvec, &View_matrix.vec.uvec, angle, &vmd_zero_vector, &View_matrix.vec.fvec);
	vm_vec_normalize(&uvec);

	vm_vec_cross(&rvec, &View_matrix.vec.fvec, &uvec);
	vm_vec_normalize(&rvec);

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
	P[0].texture_position.u = 0.0f;	P[0].texture_position.v = 0.0f;
	P[1].texture_position.u = 1.0f;	P[1].texture_position.v = 0.0f;
	P[2].texture_position.u = 1.0f;	P[2].texture_position.v = 1.0f;
	P[3].texture_position.u = 0.0f;	P[3].texture_position.v = 1.0f;

	g3_render_primitives_textured(mat_params, P, 4, PRIM_TYPE_TRIFAN, false);
}

// adapted from gr_opengl_scaler()
void g3_render_rect_scaler(material *mat_params, vertex *va, vertex *vb)
{
	float x0, y0, x1, y1;
	float u0, v0, u1, v1;
	float clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	float clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	float xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;

	x0 = va->screen.xyw.x;
	y0 = va->screen.xyw.y;
	x1 = vb->screen.xyw.x;
	y1 = vb->screen.xyw.y;

	xmin = i2fl(gr_screen.clip_left);
	ymin = i2fl(gr_screen.clip_top);
	xmax = i2fl(gr_screen.clip_right);
	ymax = i2fl(gr_screen.clip_bottom);

	u0 = va->texture_position.u; v0 = va->texture_position.v;
	u1 = vb->texture_position.u; v1 = vb->texture_position.v;

	// Check for obviously offscreen bitmaps...
	if ( (y1 <= y0) || (x1 <= x0) ) {
		return;
	}

	if ( (x1 < xmin) || (x0 > xmax) ) {
		return;
	}

	if ( (y1 < ymin) || (y0 > ymax) ) {
		return;
	}

	clipped_u0 = u0;
	clipped_v0 = v0;
	clipped_u1 = u1;
	clipped_v1 = v1;

	clipped_x0 = x0;
	clipped_y0 = y0;
	clipped_x1 = x1;
	clipped_y1 = y1;

	// Clip the left, moving u0 right as necessary
	if ( x0 < xmin ) {
		clipped_u0 = FIND_SCALED_NUM(xmin, x0, x1, u0, u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if ( x1 > xmax ) {
		clipped_u1 = FIND_SCALED_NUM(xmax, x0, x1, u0, u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if ( y0 < ymin ) {
		clipped_v0 = FIND_SCALED_NUM(ymin, y0, y1, v0, v1);
		clipped_y0 = ymin;
	}

	// Clip the bottom, moving v1 up as necessary
	if ( y1 > ymax ) {
		clipped_v1 = FIND_SCALED_NUM(ymax, y0, y1, v0, v1);
		clipped_y1 = ymax;
	}

	dx0 = fl2i(clipped_x0);
	dx1 = fl2i(clipped_x1);
	dy0 = fl2i(clipped_y0);
	dy1 = fl2i(clipped_y1);

	if ( (dx1 <= dx0) || (dy1 <= dy0) ) {
		return;
	}

	vertex v[4];

	v[0].screen.xyw.x = clipped_x0;
	v[0].screen.xyw.y = clipped_y0;
	v[0].screen.xyw.w = va->screen.xyw.w;
	v[0].world.xyz.z = va->world.xyz.z;
	v[0].texture_position.u = clipped_u0;
	v[0].texture_position.v = clipped_v0;

	v[1].screen.xyw.x = clipped_x1;
	v[1].screen.xyw.y = clipped_y0;
	v[1].screen.xyw.w = va->screen.xyw.w;
	v[1].world.xyz.z = va->world.xyz.z;
	v[1].texture_position.u = clipped_u1;
	v[1].texture_position.v = clipped_v0;

	v[2].screen.xyw.x = clipped_x1;
	v[2].screen.xyw.y = clipped_y1;
	v[2].screen.xyw.w = va->screen.xyw.w;
	v[2].world.xyz.z = va->world.xyz.z;
	v[2].texture_position.u = clipped_u1;
	v[2].texture_position.v = clipped_v1;

	v[3].screen.xyw.x = clipped_x0;
	v[3].screen.xyw.y = clipped_y1;
	v[3].screen.xyw.w = va->screen.xyw.w;
	v[3].world.xyz.z = va->world.xyz.z;
	v[3].texture_position.u = clipped_u0;
	v[3].texture_position.v = clipped_v1;

	g3_render_primitives_textured(mat_params, v, 4, PRIM_TYPE_TRIFAN, true);
}

// adapted from g3_draw_bitmap()
//void render_oriented_bitmap_2d(int texture, float alpha, bool blending, vertex *pnt, int orient, float rad)
void g3_render_rect_screen_aligned_2d(material *mat_params, vertex *pnt, int orient, float rad)
{
	vertex va, vb;
	float t, w, h;
	float width, height;

	int bw, bh;

	bm_get_info(mat_params->get_texture_map(TM_BASE_TYPE), &bw, &bh, NULL);

	if ( bw < bh ) {
		width = rad*2.0f;
		height = width*i2fl(bh) / i2fl(bw);
	} else if ( bw > bh ) {
		height = rad*2.0f;
		width = height*i2fl(bw) / i2fl(bh);
	} else {
		width = height = rad*2.0f;
	}

	Assert(G3_count == 1);

	if ( pnt->codes & (CC_BEHIND | CC_OFF_USER) )
		return;

	if ( !(pnt->flags&PF_PROJECTED) )
		g3_project_vertex(pnt);

	if ( pnt->flags & PF_OVERFLOW )
		return;

	t = (width * gr_screen.clip_width * 0.5f) / pnt->world.xyz.z;
	w = t*Matrix_scale.xyz.x;

	t = (height * gr_screen.clip_height * 0.5f) / pnt->world.xyz.z;
	h = t*Matrix_scale.xyz.y;

	float z, sw;
	z = pnt->world.xyz.z - rad / 2.0f;
	if ( z <= 0.0f ) {
		z = 0.0f;
		sw = 0.0f;
	} else {
		sw = 1.0f / z;
	}

	va.screen.xyw.x = pnt->screen.xyw.x - w / 2.0f;
	va.screen.xyw.y = pnt->screen.xyw.y - h / 2.0f;
	va.screen.xyw.w = sw;
	va.world.xyz.z = z;

	vb.screen.xyw.x = va.screen.xyw.x + w;
	vb.screen.xyw.y = va.screen.xyw.y + h;
	vb.screen.xyw.w = sw;
	vb.world.xyz.z = z;

	if ( orient & 1 ) {
		va.texture_position.u = 1.0f;
		vb.texture_position.u = 0.0f;
	} else {
		va.texture_position.u = 0.0f;
		vb.texture_position.u = 1.0f;
	}

	if ( orient & 2 ) {
		va.texture_position.v = 1.0f;
		vb.texture_position.v = 0.0f;
	} else {
		va.texture_position.v = 0.0f;
		vb.texture_position.v = 1.0f;
	}

	g3_render_rect_scaler(mat_params, &va, &vb);
}

// adapted from g3_draw_bitmap_3d
void g3_render_rect_screen_aligned(material *mat_params, vertex *pnt, int orient, float rad, float depth)
{
	// holdover mistake from retail causes these bitmaps to be rendered 41% bigger than rad
	// this turns radius into the diagonal distance, but the methods below presume manhattan distance (unadjusted radius)
	rad *= 1.41421356f;

	vec3d PNT(pnt->world);
	vec3d p[4];
	vertex P[4];
	vec3d fvec, rvec, uvec;

	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize(&fvec);

	uvec = View_matrix.vec.uvec;
	vm_vec_normalize(&uvec);
	rvec = View_matrix.vec.rvec;
	vm_vec_normalize(&rvec);

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

	// set up the UV coords
	if ( orient & 1 ) {
		P[0].texture_position.u = 1.0f;
		P[1].texture_position.u = 0.0f;
		P[2].texture_position.u = 0.0f;
		P[3].texture_position.u = 1.0f;
	} else {
		P[0].texture_position.u = 0.0f;
		P[1].texture_position.u = 1.0f;
		P[2].texture_position.u = 1.0f;
		P[3].texture_position.u = 0.0f;
	}

	if ( orient & 2 ) {
		P[0].texture_position.v = 1.0f;
		P[1].texture_position.v = 1.0f;
		P[2].texture_position.v = 0.0f;
		P[3].texture_position.v = 0.0f;
	} else {
		P[0].texture_position.v = 0.0f;
		P[1].texture_position.v = 0.0f;
		P[2].texture_position.v = 1.0f;
		P[3].texture_position.v = 1.0f;
	}

	g3_render_primitives_textured(mat_params, P, 4, PRIM_TYPE_TRIFAN, false);
}

// adapted from g3_draw_laser()
//void render_laser_2d(int texture, color* clr, float alpha, vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len)
void g3_render_laser_2d(material *mat_params, vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len)
{
	float headx, heady, headr, tailx, taily, tailr;
	vertex pt1, pt2;
	float depth;

	Assert(G3_count == 1);

	g3_rotate_vertex(&pt1, headp);

	g3_project_vertex(&pt1);
	if ( pt1.flags & PF_OVERFLOW )
		return;

	g3_rotate_vertex(&pt2, tailp);

	g3_project_vertex(&pt2);
	if ( pt2.flags & PF_OVERFLOW )
		return;

	if ( (pt1.codes & pt2.codes) != 0 ) {
		// Both off the same side
		return;
	}

	headx = pt1.screen.xyw.x;
	heady = pt1.screen.xyw.y;
	headr = (head_width*Matrix_scale.xyz.x*Canv_w2*pt1.screen.xyw.w);

	tailx = pt2.screen.xyw.x;
	taily = pt2.screen.xyw.y;
	tailr = (tail_width*Matrix_scale.xyz.x*Canv_w2*pt2.screen.xyw.w);

	float len_2d = fl_sqrt((tailx - headx)*(tailx - headx) + (taily - heady)*(taily - heady));

	// Cap the length if needed.
	if ( (max_len > 1.0f) && (len_2d > max_len) ) {
		float ratio = max_len / len_2d;

		tailx = headx + (tailx - headx) * ratio;
		taily = heady + (taily - heady) * ratio;
		tailr = headr + (tailr - headr) * ratio;

		len_2d = fl_sqrt((tailx - headx)*(tailx - headx) + (taily - heady)*(taily - heady));
	}

	depth = (pt1.world.xyz.z + pt2.world.xyz.z)*0.5f;

	float max_r = headr;
	float a;
	if ( tailr > max_r )
		max_r = tailr;

	if ( max_r < 1.0f )
		max_r = 1.0f;

	float mx, my, w, h1, h2;

	if ( len_2d < max_r ) {

		h1 = headr + (max_r - len_2d);
		if ( h1 > max_r ) h1 = max_r;
		h2 = tailr + (max_r - len_2d);
		if ( h2 > max_r ) h2 = max_r;

		len_2d = max_r;
		if ( fl_abs(tailx - headx) > 0.01f ) {
			a = (float)atan2(taily - heady, tailx - headx);
		} else {
			a = 0.0f;
		}

		w = len_2d;

	} else {
		a = atan2_safe(taily - heady, tailx - headx);

		w = len_2d;

		h1 = headr;
		h2 = tailr;
	}

	mx = (tailx + headx) / 2.0f;
	my = (taily + heady) / 2.0f;

	// Draw box with width 'w' and height 'h' at angle 'a' from horizontal
	// centered around mx, my

	if ( h1 < 1.0f ) h1 = 1.0f;
	if ( h2 < 1.0f ) h2 = 1.0f;

	float sa, ca;

	sa = (float)sin(a);
	ca = (float)cos(a);

	vertex v[4];
	memset(v, 0, sizeof(vertex) * 4);

	if ( depth < 0.0f ) depth = 0.0f;

	v[0].screen.xyw.x = (-w / 2.0f)*ca + (-h1 / 2.0f)*sa + mx;
	v[0].screen.xyw.y = (-w / 2.0f)*sa - (-h1 / 2.0f)*ca + my;
	v[0].world.xyz.z = pt1.world.xyz.z;
	v[0].screen.xyw.w = pt1.screen.xyw.w;
	v[0].texture_position.u = 0.0f;
	v[0].texture_position.v = 0.0f;
	v[0].b = 191;

	v[1].screen.xyw.x = (w / 2.0f)*ca + (-h2 / 2.0f)*sa + mx;
	v[1].screen.xyw.y = (w / 2.0f)*sa - (-h2 / 2.0f)*ca + my;
	v[1].world.xyz.z = pt2.world.xyz.z;
	v[1].screen.xyw.w = pt2.screen.xyw.w;
	v[1].texture_position.u = 1.0f;
	v[1].texture_position.v = 0.0f;
	v[1].b = 191;

	v[2].screen.xyw.x = (w / 2.0f)*ca + (h2 / 2.0f)*sa + mx;
	v[2].screen.xyw.y = (w / 2.0f)*sa - (h2 / 2.0f)*ca + my;
	v[2].world.xyz.z = pt2.world.xyz.z;
	v[2].screen.xyw.w = pt2.screen.xyw.w;
	v[2].texture_position.u = 1.0f;
	v[2].texture_position.v = 1.0f;
	v[2].b = 191;

	v[3].screen.xyw.x = (-w / 2.0f)*ca + (h1 / 2.0f)*sa + mx;
	v[3].screen.xyw.y = (-w / 2.0f)*sa - (h1 / 2.0f)*ca + my;
	v[3].world.xyz.z = pt1.world.xyz.z;
	v[3].screen.xyw.w = pt1.screen.xyw.w;
	v[3].texture_position.u = 0.0f;
	v[3].texture_position.v = 1.0f;
	v[3].b = 191;

	g3_render_primitives_textured(mat_params, v, 4, PRIM_TYPE_TRIFAN, true);
}

// adapted from g3_draw_rod()
void g3_render_rod(color *clr, int num_points, vec3d *pvecs, float width)
{
	const int MAX_ROD_VERTS = 100;
	vec3d uvec, fvec, rvec;
	vec3d vecs[2];
	vertex pts[MAX_ROD_VERTS];
	int i, nv = 0;

	Assert(num_points >= 2);
	Assert((num_points * 2) <= MAX_ROD_VERTS);

	for ( i = 0; i < num_points; i++ ) {
		vm_vec_sub(&fvec, &View_position, &pvecs[i]);
		vm_vec_normalize_safe(&fvec);

		int first = i + 1;
		int second = i - 1;

		if ( i == 0 ) {
			first = 1;
			second = 0;
		} else if ( i == num_points - 1 ) {
			first = i;
		}

		vm_vec_sub(&rvec, &pvecs[first], &pvecs[second]);
		vm_vec_normalize_safe(&rvec);

		vm_vec_cross(&uvec, &rvec, &fvec);

		vm_vec_scale_add(&vecs[0], &pvecs[i], &uvec, width * 0.5f);
		vm_vec_scale_add(&vecs[1], &pvecs[i], &uvec, -width * 0.5f);

		if ( nv > MAX_ROD_VERTS - 2 ) {
			Warning(LOCATION, "Hit high-water mark (%i) in g3_draw_rod()!!\n", MAX_ROD_VERTS);
			break;
		}

		g3_transfer_vertex(&pts[nv], &vecs[0]);
		g3_transfer_vertex(&pts[nv + 1], &vecs[1]);

		pts[nv].texture_position.u = 1.0f;
		pts[nv].texture_position.v = i2fl(i);
		pts[nv].r = clr->red;
		pts[nv].g = clr->green;
		pts[nv].b = clr->blue;
		pts[nv].a = clr->alpha;

		pts[nv + 1].texture_position.u = 0.0f;
		pts[nv + 1].texture_position.v = i2fl(i);
		pts[nv + 1].r = clr->red;
		pts[nv + 1].g = clr->green;
		pts[nv + 1].b = clr->blue;
		pts[nv + 1].a = clr->alpha;

		nv += 2;
	}

	// we should always have at least 4 verts, and there should always be an even number
	Assert((nv >= 4) && !(nv % 2));

	material material_params;

	material_params.set_depth_mode(ZBUFFER_TYPE_READ);
	material_params.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);

	g3_render_primitives_colored(&material_params, pts, nv, PRIM_TYPE_TRISTRIP, false);
}

// adapted from g3_draw_2d_shield_icon()
void g3_render_shield_icon(color *clr, coord2d coords[6], int resize_mode)
{
	GR_DEBUG_SCOPE("Render shield icon");

	vertex v[6];

	memset(v, 0, sizeof(vertex) * 6);

	if ( resize_mode != GR_RESIZE_NONE ) {
		gr_resize_screen_pos(&coords[0].x, &coords[0].y, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&coords[1].x, &coords[1].y, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&coords[2].x, &coords[2].y, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&coords[3].x, &coords[3].y, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&coords[4].x, &coords[4].y, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&coords[5].x, &coords[5].y, NULL, NULL, resize_mode);
	}

	float sw = 0.1f;

	// stuff coords
	v[0].screen.xyw.x = i2fl(coords[0].x);
	v[0].screen.xyw.y = i2fl(coords[0].y);
	v[0].screen.xyw.w = sw;
	v[0].texture_position.u = 0.0f;
	v[0].texture_position.v = 0.0f;
	v[0].flags = PF_PROJECTED;
	v[0].codes = 0;
	v[0].r = (ubyte)clr->red;
	v[0].g = (ubyte)clr->green;
	v[0].b = (ubyte)clr->blue;
	v[0].a = 0;

	v[1].screen.xyw.x = i2fl(coords[1].x);
	v[1].screen.xyw.y = i2fl(coords[1].y);
	v[1].screen.xyw.w = sw;
	v[1].texture_position.u = 0.0f;
	v[1].texture_position.v = 0.0f;
	v[1].flags = PF_PROJECTED;
	v[1].codes = 0;
	v[1].r = (ubyte)clr->red;
	v[1].g = (ubyte)clr->green;
	v[1].b = (ubyte)clr->blue;
	v[1].a = (ubyte)clr->alpha;

	v[2].screen.xyw.x = i2fl(coords[2].x);
	v[2].screen.xyw.y = i2fl(coords[2].y);
	v[2].screen.xyw.w = sw;
	v[2].texture_position.u = 0.0f;
	v[2].texture_position.v = 0.0f;
	v[2].flags = PF_PROJECTED;
	v[2].codes = 0;
	v[2].r = (ubyte)clr->red;
	v[2].g = (ubyte)clr->green;
	v[2].b = (ubyte)clr->blue;
	v[2].a = 0;

	v[3].screen.xyw.x = i2fl(coords[3].x);
	v[3].screen.xyw.y = i2fl(coords[3].y);
	v[3].screen.xyw.w = sw;
	v[3].texture_position.u = 0.0f;
	v[3].texture_position.v = 0.0f;
	v[3].flags = PF_PROJECTED;
	v[3].codes = 0;
	v[3].r = (ubyte)clr->red;
	v[3].g = (ubyte)clr->green;
	v[3].b = (ubyte)clr->blue;
	v[3].a = (ubyte)clr->alpha;

	v[4].screen.xyw.x = i2fl(coords[4].x);
	v[4].screen.xyw.y = i2fl(coords[4].y);
	v[4].screen.xyw.w = sw;
	v[4].texture_position.u = 0.0f;
	v[4].texture_position.v = 0.0f;
	v[4].flags = PF_PROJECTED;
	v[4].codes = 0;
	v[4].r = (ubyte)clr->red;
	v[4].g = (ubyte)clr->green;
	v[4].b = (ubyte)clr->blue;
	v[4].a = 0;

	v[5].screen.xyw.x = i2fl(coords[5].x);
	v[5].screen.xyw.y = i2fl(coords[5].y);
	v[5].screen.xyw.w = sw;
	v[5].texture_position.u = 0.0f;
	v[5].texture_position.v = 0.0f;
	v[5].flags = PF_PROJECTED;
	v[5].codes = 0;
	v[5].r = (ubyte)clr->red;
	v[5].g = (ubyte)clr->green;
	v[5].b = (ubyte)clr->blue;
	v[5].a = 0;

	material material_instance;
	material_instance.set_depth_mode(ZBUFFER_TYPE_NONE);
	material_instance.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	material_instance.set_cull_mode(false);
	material_instance.set_color(1.0f, 1.0f, 1.0f, 1.0f);

	// draw the polys
	g3_render_primitives_colored(&material_instance, v, 6, PRIM_TYPE_TRISTRIP, true);
}

void g3_render_shield_icon(coord2d coords[6], int resize_mode)
{
	g3_render_shield_icon(&gr_screen.current_color, coords, resize_mode);
}

// adapted from gr_opengl_line_htl()
void g3_render_line_3d(color *clr, bool depth_testing, const vec3d *start, const vec3d *end)
{
	material mat;

	mat.set_depth_mode((depth_testing) ? ZBUFFER_TYPE_READ : ZBUFFER_TYPE_NONE);
	mat.set_color(*clr);
	mat.set_cull_mode(false);

	if ( clr->is_alphacolor ) {
		mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	} else {
		mat.set_blend_mode(ALPHA_BLEND_NONE);
	}

	float line[6] = {
		start->xyz.x,	start->xyz.y,	start->xyz.z,
		end->xyz.x,		end->xyz.y,		end->xyz.z
	};

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION3, sizeof(float) * 3, 0);

	gr_render_primitives_immediate(&mat, PRIM_TYPE_LINES, &vert_def, 2, line, sizeof(float) * 6);
}

void g3_render_line_3d(bool depth_testing, const vec3d *start, const vec3d *end)
{
	g3_render_line_3d(&gr_screen.current_color, depth_testing, start, end);
}

void g3_render_sphere(color *clr, vec3d* position, float radius)
{
	g3_start_instance_matrix(position, &vmd_identity_matrix, true);

	vec3d scale = {{{ radius, radius, radius }}};
	gr_push_scale_matrix(&scale);

	material material_def;

	material_def.set_blend_mode(ALPHA_BLEND_NONE);
	material_def.set_depth_mode(ZBUFFER_TYPE_FULL);
	material_def.set_color(*clr);

	gr_sphere(&material_def, radius);

	gr_pop_scale_matrix();
	g3_done_instance(true);
}

void g3_render_sphere(vec3d* position, float radius)
{
	g3_render_sphere(&gr_screen.current_color, position, radius);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//flash ball stuff

void flash_ball::initialize(uint number, float min_ray_width, float max_ray_width, const vec3d* dir, const vec3d* pcenter, float outer, float inner, ubyte max_r, ubyte max_g, ubyte max_b, ubyte min_r, ubyte min_g, ubyte min_b)
{
	if(number < 1)
		return;

	center = *pcenter;

	if(n_rays < number){
		if(ray)vm_free(ray);
		ray = (flash_beam*)vm_malloc(sizeof(flash_beam)*number);
		n_rays = number;
	}

	uint i;
	for(i = 0; i<n_rays; i++){
	//colors
		if(min_r != 255){
			ray[i].start.r = (ubyte)Random::next(min_r, max_r);
		}else{
			ray[i].start.r = 255;
		}
		if(min_g != 255){
			ray[i].start.g = (ubyte)Random::next(min_g, max_g);
		}else{
			ray[i].start.g = 255;
		}
		if(min_b != 255){
			ray[i].start.b = (ubyte)Random::next(min_b, max_b);
		}else{
			ray[i].start.b = 255;
		}

	//rays
		if(dir == &vmd_zero_vector || outer >= PI2){
			//random sphere
			vec3d start, end;

			vm_vec_rand_vec_quick(&start);
			vm_vec_rand_vec_quick(&end);

			ray[i].start.world = start;
			ray[i].end.world = end;
		}else{
			//random cones
			vec3d start, end;

			vm_vec_random_cone(&start, dir, inner, outer);
			vm_vec_random_cone(&end, dir, inner, outer);

			ray[i].start.world = start;
			ray[i].end.world = end;
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
	uint n;
	uint nverts = uw(off+bsp_data+8);	
	uint offset = uw(off+bsp_data+16);
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
			ray[n].start.world = temp;
		
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
#define OP_TMAP2POLY    6
#define OP_SORTNORM2	7


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
		case OP_SORTNORM2:
			break;
		case OP_FLATPOLY:
			break;
		case OP_TMAPPOLY:
			break;
		case OP_BOUNDBOX:
			break;
		case OP_TMAP2POLY:
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


void flash_ball::initialize(ubyte *bsp_data, float min_ray_width, float max_ray_width, const vec3d* dir, const vec3d* pcenter, float outer, float inner, ubyte max_r, ubyte max_g, ubyte max_b, ubyte min_r, ubyte min_g, ubyte min_b)
{
	center = *pcenter;
	vm_vec_negate(&center);
	parse_bsp(0,bsp_data);
	center = vmd_zero_vector;

	uint i;
	for(i = 0; i<n_rays; i++){
	//colors
		if(min_r != 255){
			ray[i].start.r = (ubyte)Random::next(min_r, max_r);
		}else{
			ray[i].start.r = 255;
		}
		if(min_g != 255){
			ray[i].start.g = (ubyte)Random::next(min_g, max_g);
		}else{
			ray[i].start.g = 255;
		}
		if(min_b != 255){
			ray[i].start.b = (ubyte)Random::next(min_b, max_b);
		}else{
			ray[i].start.b = 255;
		}

	//rays
		if(dir == &vmd_zero_vector || outer >= PI2){
			//random sphere
			vec3d end;

			vm_vec_rand_vec_quick(&end);

			ray[i].end.world = end;
		}else{
			//random cones
			vec3d end;

			vm_vec_random_cone(&end, dir, inner, outer);

			ray[i].end.world = end;
		}
		if(max_ray_width == 0.0f)ray[i].width=min_ray_width;
		else ray[i].width = frand_range(min_ray_width, max_ray_width);
	}
}

//rad		how wide the ball should be
//intinsity	how visable it should be
//life		how far along from start to end should it be
void flash_ball::render(int texture, float rad, float intinsity, float life){
	for(uint i = 0; i < n_rays; i++){
		vec3d end;
		vm_vec_interp_constant(&end, &ray[i].start.world, &ray[i].end.world, life);
		vm_vec_scale(&end, rad);
		vm_vec_add2(&end, &center);

		batching_add_beam(texture, &center, &end, ray[i].width*rad, intinsity);
	}
}
