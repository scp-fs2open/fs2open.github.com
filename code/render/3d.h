/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _3D_H
#define _3D_H

#include "math/vecmat.h"
#include "graphics/tmapper.h"
#include "graphics/2d.h"
#include "graphics/grbatch.h"
#include "camera/camera.h"

//flags for point structure
#define PF_PROJECTED 	 1	//has been projected, so sx,sy valid
#define PF_OVERFLOW		 2	//can't project
#define PF_TEMP_POINT	 4	//created during clip

//clipping codes flags
#define CC_OFF_LEFT	1
#define CC_OFF_RIGHT	2
#define CC_OFF_BOT	4
#define CC_OFF_TOP	8
#define CC_OFF_USER	16
#define CC_BEHIND		0x80

// Combo!
#define CC_OFF			(CC_OFF_LEFT | CC_OFF_RIGHT | CC_OFF_BOT | CC_OFF_TOP | CC_OFF_USER | CC_BEHIND)


//Functions in library

//Frame setup functions:
//start the frame. Pass non-zero to enable zbuffering
#define g3_start_frame(zbuffer_flag) g3_start_frame_func(zbuffer_flag, __FILE__, __LINE__ )

// use the g3_start_frame macro instead of calling this directly.
extern void g3_start_frame_func(int zbuffer_flag, char * filename, int lineno);

//end the frame
#define g3_end_frame() g3_end_frame_func( __FILE__, __LINE__ )
extern void g3_end_frame_func(char *filename, int lineno);

// currently in frame?
extern int g3_in_frame();

//set view from x,y,z & p,b,h, zoom.  Must call one of g3_set_view_*()
void g3_set_view_angles(vec3d *view_pos,angles *view_orient,float zoom);

//Set view from camera
void g3_set_view(camera *cam);

//set view from x,y,z, viewer matrix, and zoom.  Must call one of g3_set_view_*()
void g3_set_view_matrix(vec3d *view_pos,matrix *view_matrix,float zoom);

// Never set these!
extern matrix		View_matrix;		// The matrix to convert local coordinates to screen
extern vec3d		View_position;		// The offset to convert local coordinates to screen

extern matrix		Light_matrix;		// Used to rotate world points into current local coordinates
extern vec3d		Light_base;			// Used to rotate world points into current local coordinates

extern matrix		Eye_matrix;			// Where the viewer's eye is pointing in World coordinates
extern vec3d		Eye_position;		// Where the viewer's eye is at in World coordinates
extern float		Eye_fov;			// What the viewer's FOV is

extern vec3d Object_position;
extern matrix	Object_matrix;			// Where the opject is pointing in World coordinates

extern float Proj_fov;					// Projection matrix fov (for HT&L)

//draw a horizon
void g3_draw_horizon(int sky_color,int ground_color);

//draws a line representing the horizon
void g3_draw_horizon_line();

//get vectors that are edge of horizon
int g3_compute_sky_polygon(float *points_2d,vec3d *vecs);

//Instancing

//instance at specified point with specified orientation
void g3_start_instance_matrix(vec3d *pos,matrix *orient, bool set_api = true);

//instance at specified point with specified orientation
void g3_start_instance_angles(vec3d *pos,angles *orient);

//pops the old context
void g3_done_instance(bool set_api = false);

//Misc utility functions:

//get current field of view.  Fills in angle for x & y
void g3_get_FOV(float *fov_x,float *fov_y);

//get zoom.  For a given window size, return the zoom which will achieve
//the given FOV along the given axis.
float g3_get_zoom(char axis,float fov,int window_width,int window_height);

//returns the normalized, unscaled view vectors
void g3_get_view_vectors(vec3d *forward,vec3d *up,vec3d *right);

//returns true if a plane is facing the viewer. takes the unrotated surface
//normal of the plane, and a point on it.  The normal need not be normalized
int g3_check_normal_facing(vec3d *v,vec3d *norm);

//Point definition and rotation functions:

//specify the arrays refered to by the 'pointlist' parms in the following
//functions.  I'm not sure if we will keep this function, but I need
//it now.
//void g3_set_points(g3s_point *points,vms_vector *vecs);

//returns codes_and & codes_or of a list of points numbers
ccodes g3_check_codes(int nv,vertex **pointlist);

//rotates a point. returns codes.  does not check if already rotated
ubyte g3_rotate_vertex(vertex *dest,vec3d *src);

// same as above, only ignores the current instancing
ubyte g3_rotate_vertex_popped(vertex *dest,vec3d *src);

//use this for stars, etc
ubyte g3_rotate_faraway_vertex(vertex *dest,vec3d *src);


//projects a point
int g3_project_vertex(vertex *point);

//projects a vector
ubyte g3_project_vector(vec3d *p, float *sx, float *sy );

//rotates a point.  returns codes.
ubyte g3_rotate_vector(vec3d *dest,vec3d *src);

//Codes a vector.  Returns the codes of a point.
ubyte g3_code_vector(vec3d * p);

//calculate the depth of a point - returns the z coord of the rotated point
float g3_calc_point_depth(vec3d *pnt);

//From a 2d point, compute the vector through that point
void g3_point_to_vec(vec3d *v,int sx,int sy);

//From a 2d point, compute the vector through that point.
// This can be called outside of a g3_start_frame/g3_end_frame
// pair as long g3_start_frame was previously called.
void g3_point_to_vec_delayed(vec3d *v,int sx,int sy);

//code a point.  fills in the p3_codes field of the point, and returns the codes
ubyte g3_code_vertex(vertex *point);

//delta rotation functions
vec3d *g3_rotate_delta_x(vec3d *dest,float dx);
vec3d *g3_rotate_delta_y(vec3d *dest,float dy);
vec3d *g3_rotate_delta_z(vec3d *dest,float dz);
vec3d *g3_rotate_delta_vec(vec3d *dest,vec3d *src);
ubyte g3_add_delta_vec(vertex *dest,vertex *src,vec3d *deltav);

//Drawing functions:

//draw a polygon.
//Set TMAP_FLAG_TEXTURED in the tmap_flags to texture map it with current texture.
//returns 1 if off screen, 0 if drew
int g3_draw_poly(int nv,vertex **pointlist,uint tmap_flags);

int g3_draw_polygon(vec3d *pos, matrix *ori, float width, float height, int tmap_flags = TMAP_FLAG_TEXTURED);
int g3_draw_polygon(vec3d *pos, vec3d *norm, float width, float height, int tmap_flags = TMAP_FLAG_TEXTURED);

// Draw a polygon.  Same as g3_draw_poly, but it bashes sw to a constant value
// for all vertexes.  Needs to be done after clipping to get them all.
//Set TMAP_FLAG_TEXTURED in the tmap_flags to texture map it with current texture.
//returns 1 if off screen, 0 if drew
int g3_draw_poly_constant_sw(int nv,vertex **pointlist,uint tmap_flags, float constant_sw);

//like g3_draw_poly(), but checks to see if facing.  If surface normal is
//NULL, this routine must compute it, which will be slow.  It is better to
//pre-compute the normal, and pass it to this function.  When the normal
//is passed, this function works like g3_check_normal_facing() plus
//g3_draw_poly().
//Set TMAP_FLAG_TEXTURED in the tmap_flags to texture map it with current texture.
//returns -1 if not facing, 1 if off screen, 0 if drew
int g3_draw_poly_if_facing(int nv,vertex **pointlist,uint tmap_flags,vec3d *norm,vec3d *pnt);

//draws a line. takes two points.
int g3_draw_line(vertex *p0,vertex *p1);

// Draws a polygon always facing the viewer.
// compute the corners of a rod.  fills in vertbuf.
// Verts has any needs uv's or l's or can be NULL if none needed.
int g3_draw_rod(vec3d *p0,float width1,vec3d *p1,float width2, vertex * verts, uint tmap_flags);

//draws a bitmap with the specified 3d width & height
//Set TMAP_FLAG_TEXTURED in the tmap_flags to texture map it with current texture.
//returns 1 if off screen, 0 if drew
// If bitmap is not square, rad will be the 3d size of the smallest dimension.
// orient flips the bitmap in some way.  Pass 0 for normal or else pass a 
// random nuber between 0 and 7, inclusive.
int g3_draw_bitmap(vertex *pos,int orient, float radius, uint tmap_flags, float depth = 0.0f);

// get bitmap dims onscreen as if g3_draw_bitmap() had been called
int g3_get_bitmap_dims(int bitmap, vertex *pos, float radius, int *x, int *y, int *w, int *h, int *size);

//draw a sortof sphere - i.e., the 2d radius is proportional to the 3d
//radius, but not to the distance from the eye.  Uses the current 2d color.
int g3_draw_sphere(vertex *pnt,float rad);

// Same as g3_draw_sphere, but you pass a vector and this rotates
// and projects it and then call g3_draw_sphere.
int g3_draw_sphere_ez(vec3d *pnt,float rad);

//Draw a laser shaped 3d looking thing.
// If max_len is > 1.0, then this caps the length to be no longer than max_len pixels
float g3_draw_laser(vec3d *headp, float head_width, vec3d *tailp, float tail_width, uint tmap_flags = TMAP_FLAG_TEXTURED, float max_len = 0.0f );

// Draw a laser shaped 3d looking thing using vertex coloring (useful for things like colored laser glows)
// If max_len is > 1.0, then this caps the length to be no longer than max_len pixels
float g3_draw_laser_rgb(vec3d *headp, float head_width, vec3d *tailp, float tail_width, int r, int g, int b, uint tmap_flags = TMAP_FLAG_TEXTURED | TMAP_FLAG_RGB, float max_len = 0.0f );

// Draw a bitmap that is always facing, but rotates.
// If bitmap is not square, rad will be the 3d size of the smallest dimension.
int g3_draw_rotated_bitmap(vertex *pnt,float angle, float radius, uint tmap_flags, float depth = 0.0f);

// draw a perspective bitmap based on angles and radius
int g3_draw_perspective_bitmap(angles *a, float scale_x, float scale_y, int div_x, int div_y, uint tmap_flags);

//Draws a 2D rectangle
void g3_draw_2d_rect(int x, int y, int w, int h, int r, int g, int b, int a);

// draw a 2d bitmap on a poly
int g3_draw_2d_poly_bitmap(float x, float y, float w, float h, uint additional_tmap_flags = 0);

// Enables clipping with an arbritary plane.   This will be on
// until g3_stop_clip_plane is called or until next frame.
// The points passed should be relative to the instance.  Probably
// that means world coordinates.
/*
  This works like any other clip plane... if this is enabled and you
rotate a point, the CC_OFF_USER bit will be set in the clipping codes.
It is completely handled by most g3_draw primitives, except maybe lines.

  As far as performance, when enabled, it will slow down each point
rotation (or g3_code_vertex call) by a vec3d subtraction and dot
product.   It won't slow anything down for polys that are completely
clipped on or off by the plane, and will slow each clipped polygon by
not much more than any other clipping we do.
*/
void g3_start_user_clip_plane( vec3d *plane_point, vec3d *plane_normal );

// Stops arbritary plane clipping
void g3_stop_user_clip_plane();

ubyte g3_transfer_vertex(vertex *dest, vec3d *src);

int g3_draw_2d_poly_bitmap_list(bitmap_2d_list* b_list, int n_bm, uint additional_tmap_flags);
int g3_draw_2d_poly_bitmap_rect_list(bitmap_rect_list* b_list, int n_bm, uint additional_tmap_flags);

//draw a line in HTL mode without having to go through the rotate/project stuff
void g3_draw_htl_line(vec3d *start, vec3d *end);

//draw a sphere mode without having to go through the rotate/project stuff
void g3_draw_htl_sphere(vec3d *position, float radius);

//flash ball
// a neat looking ball of rays that move around and look all energetic and stuff
struct flash_beam{
	vertex start;
	vertex end;
	float width;
};

class flash_ball{
	flash_beam *ray;
	vec3d center;
	int n_rays;
	static geometry_batcher batcher;
	void parse_bsp(int offset, ubyte *bsp_data);
	void defpoint(int off, ubyte *bsp_data);

public:
//	flash_ball():ray(NULL),n_rays(0){}
	flash_ball(int number, float min_ray_width, float max_ray_width = 0, vec3d* dir = &vmd_zero_vector, vec3d*pcenter = &vmd_zero_vector, float outer = PI2, float inner = 0.0f, ubyte max_r = 255, ubyte max_g = 255, ubyte max_b = 255, ubyte min_r = 255, ubyte min_g = 255, ubyte min_b = 255)
		:ray(NULL),n_rays(0)
		{initialize(number, min_ray_width, max_ray_width , dir , pcenter , outer , inner , max_r , max_g , max_b , min_r , min_g ,min_b);}
	~flash_ball(){if(ray)vm_free(ray);}

	void initialize(int number, float min_ray_width, float max_ray_width = 0, vec3d* dir = &vmd_zero_vector, vec3d*pcenter = &vmd_zero_vector, float outer = PI2, float inner = 0.0f, ubyte max_r = 255, ubyte max_g = 255, ubyte max_b = 255, ubyte min_r = 255, ubyte min_g = 255, ubyte min_b = 255);
	void initialize(ubyte *bsp_data, float min_ray_width, float max_ray_width = 0, vec3d* dir = &vmd_zero_vector, vec3d*pcenter = &vmd_zero_vector, float outer = PI2, float inner = 0.0f, ubyte max_r = 255, ubyte max_g = 255, ubyte max_b = 255, ubyte min_r = 255, ubyte min_g = 255, ubyte min_b = 255);
	void render(float rad, float intinsity, float life);
};
#endif
