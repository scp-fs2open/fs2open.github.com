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
#include "graphics/2d.h"			// Needed for w,h,aspect of canvas
#include "lighting/lighting.h"




matrix		View_matrix;		// The matrix to convert local coordinates to screen
vec3d		View_position;		// The offset to convert local coordinates to screen
matrix		Unscaled_matrix;	// View_matrix before scaling

matrix		Light_matrix;		// Used to rotate world points into current local coordinates
vec3d		Light_base;			// Used to rotate world points into current local coordinates

matrix		Eye_matrix;			// Where the viewer's eye is pointing in World coordinates
vec3d		Eye_position;		// Where the viewer's eye is at in World coordinates
float		Eye_fov;			// What the viewer's FOV is

float			View_zoom;			// The zoom factor
float			Proj_fov;			// The fov (for HTL projection matrix)

vec3d		Window_scale;		// Scaling for window aspect
vec3d		Matrix_scale;		// How the matrix is scaled, window_scale * zoom

int			Canvas_width;		// The actual width
int			Canvas_height;		// The actual height

float			Canv_w2;				// Canvas_width / 2
float			Canv_h2;				// Canvas_height / 2

vec3d Object_position;
matrix	Object_matrix;			// Where the opject is pointing in World coordinates


#define MAX_INSTANCE_DEPTH	5

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
extern int Cmdline_nohtl;

// check if in frame
int g3_in_frame()
{
	return G3_count;
}

//start the frame
// Pass true for zbuffer_flag to turn on zbuffering
void g3_start_frame_func(int zbuffer_flag, char * filename, int lineno)
{
	float s;
	int width, height;
	float aspect;

//Uncomment this to figure out who called g3_start_frame without calling g3_end_frame.

//	mprintf(( "g3_start_frame called from %s, line %d\n", filename, lineno ));

 	Assert( G3_count == 0 );
	G3_count++;

	// Clear any user-defined clip planes
	g3_stop_user_clip_plane();

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

	if ( !Cmdline_nohtl || (s <= 0.0f) ) {		//scale x
		Window_scale.xyz.x = s;
		Window_scale.xyz.y = 1.0f;
	}
	else {
		Window_scale.xyz.y = 1.0f / s;
		Window_scale.xyz.x = 1.0f;
	}
	
	Window_scale.xyz.z = 1.0f;		//always 1

	init_free_points();

	if (zbuffer_flag)	{
		gr_zbuffer_clear(TRUE);
	} else {
		gr_zbuffer_clear(FALSE);
	}

	G3_frame_count++;

	//init_interface_vars_to_assembler();		//for the texture-mapper

}

//this doesn't do anything, but is here for completeness
void g3_end_frame_func(char *filename, int lineno)
{
//	mprintf(( "g3_end_frame called from %s, line %d\n", filename, lineno ));

	G3_count--;
	Assert( G3_count == 0 );

	free_point_num = 0;
//	Assert(free_point_num==0);
}


void scale_matrix(void);

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

//set view from x,y,z, viewer matrix, and zoom.  Must call one of g3_set_view_*()
void g3_set_view_matrix(vec3d *view_pos,matrix *view_matrix,float zoom)
{
	Assert( G3_count == 1 );

	View_zoom = zoom;
	View_position = *view_pos;

	View_matrix = *view_matrix;

//	Proj_fov = (4.0f/9.0f) * PI * View_zoom;
	Proj_fov = 1.39626348f * View_zoom;

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


//set view from x,y,z & p,b,h, zoom.  Must call one of g3_set_view_*()
void g3_set_view_angles(vec3d *view_pos,angles *view_orient,float zoom)
{
	matrix tmp;

	Assert( G3_count == 1 );

	vm_angles_2_matrix(&tmp,view_orient);
	g3_set_view_matrix(view_pos,&tmp,zoom);
}


//performs aspect scaling on global view matrix
void scale_matrix(void)
{
	Unscaled_matrix = View_matrix;		//so we can use unscaled if we want

	Matrix_scale = Window_scale;

	float s = 1.0f;

	if (Cmdline_nohtl) {
		if (View_zoom <= 1.0f) { 		//zoom in by scaling z
			Matrix_scale.xyz.z =  Matrix_scale.xyz.z*View_zoom;
		} else {			//zoom out by scaling x&y
			s = 1.0f / View_zoom;

			Matrix_scale.xyz.x *= s;
			Matrix_scale.xyz.y *= s;
		}
	} else {
		s = 1.0f / tanf(Proj_fov * 0.5f);

		Matrix_scale.xyz.x *= s;
		Matrix_scale.xyz.y *= s;
	}

	//now scale matrix elements

	vm_vec_scale(&View_matrix.vec.rvec,Matrix_scale.xyz.x );
	vm_vec_scale(&View_matrix.vec.uvec,Matrix_scale.xyz.y );
	vm_vec_scale(&View_matrix.vec.fvec,Matrix_scale.xyz.z );

}

ubyte g3_rotate_vertex_popped(vertex *dest,vec3d *src)
{
	vec3d tempv;

	Assert( G3_count == 1 );

	Assert( instance_depth > 0 );

	vm_vec_sub(&tempv,src,&instance_stack[0].p);
	vm_vec_rotate( (vec3d *)&dest->x, &tempv, &instance_stack[0].m );
	dest->flags = 0;	//not projected
	return g3_code_vertex(dest);
}	


//instance at specified point with specified orientation
//if matrix==NULL, don't modify matrix.  This will be like doing an offset   
//if pos==NULL, no position change
void g3_start_instance_matrix(vec3d *pos,matrix *orient, bool set_api)
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
	vm_copy_transpose_matrix(&tempm2,orient);

	vm_matrix_x_matrix(&tempm,&tempm2,&View_matrix);
	View_matrix = tempm;

	vm_matrix_x_matrix(&Object_matrix,orient,&instance_stack[instance_depth-1].om);
//	Object_matrix = tempm;

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

	if(!Cmdline_nohtl && set_api)
		gr_start_instance_matrix(pos,orient);

}


//instance at specified point with specified orientation
//if angles==NULL, don't modify matrix.  This will be like doing an offset
void g3_start_instance_angles(vec3d *pos,angles *orient)
{
	matrix tm;

	Assert( G3_count == 1 );

	if (orient==NULL) {
		g3_start_instance_matrix(pos,NULL);
		return;
	}

	vm_angles_2_matrix(&tm,orient);

	g3_start_instance_matrix(pos,&tm, false);

	if(!Cmdline_nohtl)gr_start_angles_instance_matrix(pos, orient);

}


//pops the old context
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

	if (!Cmdline_nohtl && use_api)
		gr_end_instance_matrix();
}

int G3_user_clip = 0;
vec3d G3_user_clip_normal;
vec3d G3_user_clip_point;

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
void g3_start_user_clip_plane( vec3d *plane_point, vec3d *plane_normal )
{
	float mag = vm_vec_mag( plane_normal );
	if ( (mag < 0.1f) || (mag > 1.5f ) )	{
		// Invalid plane_normal passed in.  Get Allender (since it is
		// probably a ship warp in bug:) or John.   
		Int3();			
		return;
	}

	G3_user_clip = 1;
	if(!Cmdline_nohtl) {
		G3_user_clip_normal = *plane_normal;
		G3_user_clip_point = *plane_point;
//	return;


		gr_start_clip();
	}
	vm_vec_rotate(&G3_user_clip_normal, plane_normal, &View_matrix );
	vm_vec_normalize(&G3_user_clip_normal);

	vec3d tempv;
	vm_vec_sub(&tempv,plane_point,&View_position);
	vm_vec_rotate(&G3_user_clip_point,&tempv,&View_matrix );
}

// Stops arbritary plane clipping
void g3_stop_user_clip_plane()
{
	G3_user_clip = 0;
	if(!Cmdline_nohtl) {
		gr_end_clip();
	}
}

// Returns TRUE if point is behind user plane
int g3_point_behind_user_plane( vec3d *pnt )
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
