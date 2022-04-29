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

#include "camera/camera.h"
#include "graphics/2d.h"
#include "graphics/grbatch.h"
#include "graphics/tmapper.h"
#include "math/vecmat.h"

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

/**
 * Start the frame. Pass non-zero to enable zbuffering
 */
#define g3_start_frame(zbuffer_flag) g3_start_frame_func(zbuffer_flag, __FILE__, __LINE__ )

/**
 * Use the g3_start_frame macro instead of calling this directly.
 */
extern void g3_start_frame_func(int zbuffer_flag, const char *filename, int lineno);

/**
 * End the frame
 */
#define g3_end_frame() g3_end_frame_func( __FILE__, __LINE__ )
extern void g3_end_frame_func(const char *filename, int lineno);

/**
 * Currently in frame?
 */
extern int g3_in_frame();


/**
 * Sets the Proj_fov from the specified zoom value
 */
void g3_set_fov(float zoom);

/**
 * Set view from camera
 */
void g3_set_view(camera *cam);

/**
 * Set view from x,y,z, viewer matrix, and zoom.  Must call one of g3_set_view_*()
 */
void g3_set_view_matrix(const vec3d *view_pos, const matrix *view_matrix, float zoom);

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


/**
 * Draws a line representing the horizon
 */
void g3_draw_horizon_line();


/**
 * Instance at specified point with specified orientation
 */
void g3_start_instance_matrix(const vec3d *pos, const matrix *orient, bool set_api = true);

/**
 * Instance at specified point with specified transform.
 */
void g3_start_instance_matrix(const matrix4 *transform, bool set_api = true);

/**
 * Instance at specified point with specified orientation
 */
void g3_start_instance_angles(const vec3d *pos, const angles *orient);

/**
 * Pops the old context
 */
void g3_done_instance(bool set_api = false);

/**
 * Returns true if a plane is facing the viewer. 
 *
 * Takes the unrotated surface normal of the plane, and a point on it.  The normal need not be normalized
 */
int g3_check_normal_facing(const vec3d *v, const vec3d *norm);

/**
 * Rotates a point. returns codes.  does not check if already rotated
 */
ubyte g3_rotate_vertex(vertex *dest, const vec3d *src);

/**
 * Use this for stars, etc
 */
ubyte g3_rotate_faraway_vertex(vertex *dest, const vec3d *src);

/**
 * Projects a point
 */
int g3_project_vertex(vertex *point);

/**
 * Projects a vector
 */
ubyte g3_project_vector(const vec3d *p, float *sx, float *sy );

/**
 * Rotates a point.  
 * @return Returns codes.
 */
ubyte g3_rotate_vector(vec3d *dest, const vec3d *src);

/**
 * Codes a vector.  
 * @return Returns the codes of a point.
 */
ubyte g3_code_vector(const vec3d * p);

/**
 * Calculate the depth of a point - returns the z coord of the rotated point
 */
float g3_calc_point_depth(const vec3d *pnt);

/**
 * From a 2d point, compute the vector through that point
 */
void g3_point_to_vec(vec3d *v,int sx,int sy);

/**
 * From a 2d point, compute the vector through that point.
 *
 * This can be called outside of a g3_start_frame/g3_end_frame
 * pair as long g3_start_frame was previously called.
 */
void g3_point_to_vec_delayed(vec3d *v,int sx,int sy);

/*
 * Code a point. Fills in the p3_codes field of the point, and returns the codes
 */
ubyte g3_code_vertex(vertex *point);

vec3d *g3_rotate_delta_vec(vec3d *dest,vec3d *src);


/**
 * Draws a line.
 *
 * @param p0 First point
 * @param p1 Second point
 */
int g3_draw_line(vertex *p0, vertex *p1);


/**
 * Get bitmap dims onscreen as if g3_draw_bitmap() had been called
 */
int g3_get_bitmap_dims(int bitmap, vertex *pos, float radius, int *x, int *y, int *w, int *h, int *size);

/**
 * Draw a sortof sphere - i.e., the 2d radius is proportional to the 3d
 * radius, but not to the distance from the eye.  Uses the current 2d color.
 */
int g3_draw_sphere(vertex *pnt, float rad);

/**
 * Same as g3_draw_sphere, but you pass a vector and this rotates
 * and projects it and then call g3_draw_sphere.
 */
int g3_draw_sphere_ez(const vec3d *pnt, float rad);

ubyte g3_transfer_vertex(vertex *dest, const vec3d *src);

/**
 * Draw a line in HTL mode without having to go through the rotate/project stuff
 */
void g3_draw_htl_line(const vec3d *start, const vec3d *end);

/**
 * Draw a sphere mode without having to go through the rotate/project stuff
 */
void g3_draw_htl_sphere(color *clr, const vec3d *position, float radius);
void g3_draw_htl_sphere(const vec3d* position, float radius);

void g3_render_primitives(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);
void g3_render_primitives_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);
void g3_render_primitives_colored(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);
void g3_render_primitives_colored_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);

void g3_render_rod(color *clr, int num_points, vec3d *pvecs, float width);

void g3_render_laser_2d(material *mat_params, vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len);

void g3_render_rect_screen_aligned_rotated(material *mat_params, vertex *pnt, float angle, float rad);

void g3_render_rect_screen_aligned(material *mat_params, vertex *pnt, int orient, float rad, float depth = 0.0f);
void g3_render_rect_screen_aligned_2d(material *mat_params, vertex *pnt, int orient, float rad);

void g3_render_rect_oriented(material* mat_info, vec3d *pos, matrix *ori, float width, float height);
void g3_render_rect_oriented(material* mat_info, vec3d *pos, vec3d *norm, float width, float height);

void g3_render_line_3d(color *clr, bool depth_testing, const vec3d *start, const vec3d *end);
void g3_render_line_3d(bool depth_testing, const vec3d *start, const vec3d *end);

void g3_render_sphere(color *clr, vec3d* position, float radius);
void g3_render_sphere(vec3d* position, float radius);

void g3_render_shield_icon(color *clr, coord2d coords[6], int resize_mode = GR_RESIZE_FULL);
void g3_render_shield_icon(coord2d coords[6], int resize_mode = GR_RESIZE_FULL);

typedef struct horz_pt {
	float x, y;
	int edge;
} horz_pt;

/**
 * Flash ball
 *
 * A neat looking ball of rays that move around and look all energetic and stuff
 */
struct flash_beam{
	vertex start;
	vertex end;
	float width;
};

class flash_ball{
	flash_beam *ray;
	vec3d center;
	uint n_rays;
	void parse_bsp(int offset, ubyte *bsp_data);
	void defpoint(int off, ubyte *bsp_data);

public:
	flash_ball(int number, float min_ray_width, float max_ray_width = 0, const vec3d* dir = &vmd_zero_vector, const vec3d* pcenter = &vmd_zero_vector, float outer = PI2, float inner = 0.0f, ubyte max_r = 255, ubyte max_g = 255, ubyte max_b = 255, ubyte min_r = 255, ubyte min_g = 255, ubyte min_b = 255)
		:ray(NULL),n_rays(0)
	{
			initialize(number, min_ray_width, max_ray_width, dir, pcenter, outer, inner, max_r, max_g, max_b, min_r, min_g, min_b);
	}

	~flash_ball()
	{
		if (ray)
			vm_free(ray);
	}

	void initialize(uint number, float min_ray_width, float max_ray_width = 0, const vec3d* dir = &vmd_zero_vector, const vec3d* pcenter = &vmd_zero_vector, float outer = PI2, float inner = 0.0f, ubyte max_r = 255, ubyte max_g = 255, ubyte max_b = 255, ubyte min_r = 255, ubyte min_g = 255, ubyte min_b = 255);
	void initialize(ubyte *bsp_data, float min_ray_width, float max_ray_width = 0, const vec3d* dir = &vmd_zero_vector, const vec3d* pcenter = &vmd_zero_vector, float outer = PI2, float inner = 0.0f, ubyte max_r = 255, ubyte max_g = 255, ubyte max_b = 255, ubyte min_r = 255, ubyte min_g = 255, ubyte min_b = 255);
	void render(int texture, float rad, float intinsity, float life);
};
#endif
