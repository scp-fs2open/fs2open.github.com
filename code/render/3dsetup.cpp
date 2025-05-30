/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "graphics/2d.h"			// Needed for w,h,aspect of canvas
#include "graphics/matrix.h"
#include "graphics/tmapper.h"
#include "lighting/lighting.h"
#include "render/3dinternal.h"


matrix		View_matrix;		// The matrix to convert local coordinates to screen
vec3d		View_position;		// The offset to convert local coordinates to screen
matrix		Unscaled_matrix;	// View_matrix before scaling

matrix		Light_matrix;		// Used to rotate world points into current local coordinates
vec3d		Light_base;			// Used to rotate world points into current local coordinates

matrix		Eye_matrix;			// Where the viewer's eye is pointing in World coordinates
vec3d		Eye_position;		// Where the viewer's eye is at in World coordinates
fov_t		Eye_fov;			// What the viewer's FOV is

fov_t			View_zoom = 0.0f;		// The zoom factor
fov_t			Proj_fov = 0.0f;// The fov (for HTL projection matrix)

vec3d		Window_scale;		// Scaling for window aspect
vec3d		Matrix_scale;		// How the matrix is scaled, window_scale * zoom

int			Canvas_width;		// The actual width
int			Canvas_height;		// The actual height

float			Canv_w2;		// Canvas_width / 2
float			Canv_h2;		// Canvas_height / 2

vec3d Object_position;
matrix	Object_matrix;			// Where the opject is pointing in World coordinates


#define MAX_INSTANCE_DEPTH	32

struct instance_context {
	matrix m;
	vec3d p;
	matrix lm;
	vec3d lb;
	matrix om;
	vec3d op;
} instance_stack[MAX_INSTANCE_DEPTH];

int instance_depth = 0;

int G3_count = 0;
int G3_frame_count = 0;

/**
 * Check if in frame
 */
int g3_in_frame()
{
	return G3_count;
}

/**
 * Start the frame
 * Pass true for zbuffer_flag to turn on zbuffering
 */
void g3_start_frame_func(int zbuffer_flag, const char * /*filename*/, int  /*lineno*/)
{
	float s;
	int width, height;
	float aspect;

 	Assert( G3_count == 0 );
	G3_count++;

	// Get the values from the 2d...
	width = gr_screen.clip_width;
	height = gr_screen.clip_height;
	aspect = gr_screen.aspect;

	//set int w,h & fixed-point w,h/2
	Canvas_width = width;
	Canv_w2 = (float)width / 2.0f;
	Canvas_height = height;
	Canv_h2 = (float)height / 2.0f;
	
	//compute aspect ratio for this canvas
	s = aspect*(float)Canvas_height/(float)Canvas_width;

	Window_scale.xyz.x = s;
	Window_scale.xyz.y = 1.0f;
	Window_scale.xyz.z = 1.0f;		//always 1

	init_free_points();

	if (zbuffer_flag)	{
		gr_zbuffer_clear(TRUE);
	} else {
		gr_zbuffer_clear(FALSE);
	}

	G3_frame_count++;
}

/**
 * This doesn't do anything, but is here for completeness
 */
void g3_end_frame_func(const char * /*filename*/, int  /*lineno*/)
{
	G3_count--;
	Assert( G3_count == 0 );

	free_point_num = 0;
}


void scale_matrix(void);

void g3_set_fov(fov_t zoom) {
	Proj_fov = zoom * PROJ_FOV_FACTOR;
}

float g3_get_hfov(const fov_t& fov, bool visible_fov) {
	SCP_UNUSED(visible_fov);

	if (std::holds_alternative<float>(fov))
		return std::get<float>(fov);
	else {
		const auto& afov = std::get<asymmetric_fov>(fov);
		return afov.right - afov.left;
	}
}

void g3_set_view(camera *cam)
{
	vec3d pos;
	matrix ori;
	cam->get_info(&pos, &ori);

	if(Sexp_fov <= 0.0f)
		g3_set_view_matrix(&pos, &ori, cam->get_fov());
	else
		g3_set_view_matrix(&pos, &ori, Sexp_fov);
}

/**
 * Set view from x,y,z, viewer matrix, and zoom.  Must call one of g3_set_view_*()
 */
void g3_set_view_matrix(const vec3d *view_pos, const matrix *view_matrix, fov_t zoom)
{
	Assert( G3_count == 1 );

	View_zoom = zoom;
	View_position = *view_pos;

	View_matrix = *view_matrix;

	g3_set_fov(View_zoom);

	Eye_matrix = View_matrix;
	Eye_position = *view_pos;
	Eye_fov = zoom;

	scale_matrix();

	Light_matrix = vmd_identity_matrix;
	Light_base.xyz.x = 0.0f;
	Light_base.xyz.y = 0.0f;
	Light_base.xyz.z = 0.0f;

	vm_vec_zero(&Object_position);
	Object_matrix = vmd_identity_matrix;
}


/**
 * Performs aspect scaling on global view matrix
 */
void scale_matrix(void)
{
	Unscaled_matrix = View_matrix;		//so we can use unscaled if we want

	Matrix_scale = Window_scale;

	float s = 1.0f / tanf(g3_get_hfov(Proj_fov) * 0.5f);

	Matrix_scale.xyz.x *= s;
	Matrix_scale.xyz.y *= s;

	//now scale matrix elements

	vm_vec_scale(&View_matrix.vec.rvec,Matrix_scale.xyz.x );
	vm_vec_scale(&View_matrix.vec.uvec,Matrix_scale.xyz.y );
	vm_vec_scale(&View_matrix.vec.fvec,Matrix_scale.xyz.z );

}

/**
 * nstance at specified point with specified orientation
 *
 * if matrix==NULL, don't modify matrix.  This will be like doing an offset   
 * if pos==NULL, no position change
 */
void g3_start_instance_matrix(const vec3d *pos, const matrix *orient, bool set_api)
{
	vec3d tempv;
	matrix tempm,tempm2;

	Assert( G3_count == 1 );

	Assert(instance_depth<MAX_INSTANCE_DEPTH);

	instance_stack[instance_depth].m = View_matrix;
	instance_stack[instance_depth].p = View_position;
	instance_stack[instance_depth].lm = Light_matrix;
	instance_stack[instance_depth].lb = Light_base;
	instance_stack[instance_depth].om = Object_matrix;
	instance_stack[instance_depth].op = Object_position;
	instance_depth++;

	// Make sure orient is valid
	if (!orient) {
		orient = &vmd_identity_matrix;		// Assume no change in orient
	}

	if ( pos )	{
		//step 1: subtract object position from view position
		vm_vec_sub2(&View_position,pos);

		//step 2: rotate view vector through object matrix
		vm_vec_rotate(&tempv,&View_position,orient);
		View_position = tempv;

		vm_vec_unrotate(&tempv,pos,&Object_matrix);
		vm_vec_add2(&Object_position, &tempv);
	} else {
		// No movement, leave View_position alone
	}

	//step 3: rotate object matrix through view_matrix (vm = ob * vm)
	vm_copy_transpose(&tempm2,orient);

	vm_matrix_x_matrix(&tempm,&tempm2,&View_matrix);
	View_matrix = tempm;

	vm_matrix_x_matrix(&Object_matrix,&instance_stack[instance_depth-1].om,orient);

	// Update the lighting matrix
	matrix saved_orient = Light_matrix;
	vec3d saved_base = Light_base;
	
	if ( pos )	{
		vm_vec_unrotate(&Light_base,pos,&saved_orient );
		vm_vec_add2(&Light_base, &saved_base );
	} else {
		// No movement, light_base doesn't change.
	}

	vm_matrix_x_matrix(&Light_matrix,&saved_orient, orient);

	if(set_api)
		gr_start_instance_matrix(pos, orient);

}

void g3_start_instance_matrix(const matrix4 *transform, bool set_api)
{
	matrix orient;
	vec3d pos;

	vm_matrix4_get_orientation(&orient, (matrix4*)transform);
	vm_matrix4_get_offset(&pos, (matrix4*)transform);

	g3_start_instance_matrix(&pos, &orient, set_api);
}

/**
 * Instance at specified point with specified orientation
 *
 * If angles==NULL, don't modify matrix.  This will be like doing an offset
 */
void g3_start_instance_angles(const vec3d *pos, const angles *orient)
{
	matrix tm;

	Assert( G3_count == 1 );

	if (orient==NULL) {
		g3_start_instance_matrix(pos,NULL);
		return;
	}

	vm_angles_2_matrix(&tm,orient);

	g3_start_instance_matrix(pos,&tm, false);

	gr_start_angles_instance_matrix(pos, orient);

}


/**
 * Pops the old context
 */
void g3_done_instance(bool use_api)
{
	Assert( G3_count == 1 );

	instance_depth--;

	Assert(instance_depth >= 0);

	View_position = instance_stack[instance_depth].p;
	View_matrix = instance_stack[instance_depth].m;
	Light_matrix = instance_stack[instance_depth].lm;
	Light_base = instance_stack[instance_depth].lb;
	Object_matrix = instance_stack[instance_depth].om;
	Object_position = instance_stack[instance_depth].op;

	if (use_api)
		gr_end_instance_matrix();
}

int G3_user_clip = 0;
vec3d G3_user_clip_normal;
vec3d G3_user_clip_point;

/**
 * Returns TRUE if point is behind user plane
 */
int g3_point_behind_user_plane( const vec3d *pnt )
{
	if ( G3_user_clip ) {
		vec3d tmp;
		vm_vec_sub( &tmp, pnt, &G3_user_clip_point );
		if ( vm_vec_dot( &tmp, &G3_user_clip_normal ) <= 0.0f )	{
			return 1;
		}
	}

	return 0;
}
