/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Render/3dSetup.cpp $
 * $Revision: 2.12 $
 * $Date: 2003-11-11 18:04:06 $
 * $Author: phreak $
 *
 * Code to setup matrix instancing and viewers
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.11  2003/11/11 03:56:12  bobboau
 * shit load of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.10  2003/11/06 22:49:22  phreak
 * fixed up some of the matrix functions that were causing problems
 * bob: i there is an important change here. i don't know if its going to cause any d3d problems
 *
 * Revision 2.9  2003/11/01 21:59:22  bobboau
 * new matrix handeling code, and fixed some problems with 3D lit verts,
 * several other small fixes
 *
 * Revision 2.8  2003/10/25 06:56:06  bobboau
 * adding FOF stuff,
 * and fixed a small error in the matrix code,
 * I told you it was indeed suposed to be gr_start_instance_matrix
 * in g3_done_instance,
 * g3_start_instance_angles needs to have an gr_ API abstraction version of it made
 *
 * Revision 2.7  2003/10/25 03:27:21  phreak
 * fixed some old bugs that reappeared after RT committed his texture code
 *
 * Revision 2.6  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.5  2003/10/14 17:39:18  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.4  2003/10/10 03:59:41  matt
 * Added -nohtl command line param to disable HT&L, nothing is IFDEFd
 * out now. -Sticks
 *
 * Revision 2.3  2003/09/26 14:37:15  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.2  2003/08/09 06:07:24  bobboau
 * slightly better implementation of the new zbuffer thing, it now checks only three diferent formats defalting to the 16 bit if neither the 24 or 32 bit versions are suported
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 23    5/24/98 11:32a John
 * safely handled null vector being passed to start_user_clip_plane.
 * 
 * 22    3/18/98 4:53p John
 * Fixed some bugs with docked ships warping out
 * 
 * 21    3/18/98 4:33p John
 * Called vm_vec_normalize_safe to prevent assert from bogus docked
 * objects.
 * 
 * 20    3/16/98 5:02p John
 * Better comments
 * 
 * 19    3/16/98 4:51p John
 * Added low-level code to clip all polygons against an arbritary plane.
 * Took out all old model_interp_zclip and used this new method instead.  
 * 
 * 18    3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 17    1/19/98 6:15p John
 * Fixed all my Optimized Build compiler warnings
 * 
 * 16    11/24/97 12:04p John
 * 
 * 15    11/07/97 7:24p John
 * changed lighting to take two ranges.
 * In textest, added test code to draw nebulas
 * 
 * 14    10/29/97 5:05p John
 * Changed dynamic lighting to only rotate and calculate lighting for
 * point lights that are close to an object.  Changed lower framerate cap
 * from 4 to .5.
 * 
 * 13    4/29/97 12:24p Adam
 * JAS:   Added code for delayed point to vec.   Fixed some FRED
 * sequencing problems with g3_start_frame / g3_end_frame.
 * 
 * 12    4/29/97 9:55a John
 * 
 * 11    4/08/97 5:18p John
 * First rev of decent (dynamic, correct) lighting in FreeSpace.
 * 
 * 10    3/24/97 3:26p John
 * Cleaned up and restructured model_collide code and fvi code.  In fvi
 * made code that finds uvs work..  Added bm_get_pixel to BmpMan.
 * 
 * 9     3/10/97 2:25p John
 * Made pofview zbuffer.   Made textest work with new model code.  Took
 * out some unnecessary Asserts in the 3d clipper.
 * 
 * 
 * 8     2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 *
 * $NoKeywords: $
 */


#include "render/3dinternal.h"
#include "graphics/tmapper.h"
#include "graphics/2d.h"			// Needed for w,h,aspect of canvas
#include "lighting/lighting.h"


matrix		View_matrix;		// The matrix to convert local coordinates to screen
vector		View_position;		// The offset to convert local coordinates to screen
matrix		Unscaled_matrix;	// View_matrix before scaling

matrix		Light_matrix;		// Used to rotate world points into current local coordinates
vector		Light_base;			// Used to rotate world points into current local coordinates

matrix		Eye_matrix;			// Where the viewer's eye is pointing in World coordinates
vector		Eye_position;		// Where the viewer's eye is at in World coordinates

float			View_zoom;			// The zoom factor

vector		Window_scale;		// Scaling for window aspect
vector		Matrix_scale;		// How the matrix is scaled, window_scale * zoom

int			Canvas_width;		// The actual width
int			Canvas_height;		// The actual height

float			Canv_w2;				// Canvas_width / 2
float			Canv_h2;				// Canvas_height / 2

vector Object_position;
matrix	Object_matrix;			// Where the opject is pointing in World coordinates

//vertex buffers for polygon drawing and clipping
vertex * Vbuf0[TMAP_MAX_VERTS];
vertex * Vbuf1[TMAP_MAX_VERTS];

#define MAX_INSTANCE_DEPTH	5

struct instance_context {
	matrix m;
	vector p;
	matrix lm;
	vector lb;
	matrix om;
	vector op;
} instance_stack[MAX_INSTANCE_DEPTH];

int instance_depth = 0;

int G3_count = 0;
int G3_frame_count = 0;
extern int Cmdline_nohtl;

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

	if (s <= 0) {		//scale x
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
void g3_end_frame(void)
{
	G3_count--;
	Assert( G3_count == 0 );

	free_point_num = 0;
//	Assert(free_point_num==0);
}


void scale_matrix(void);

//set view from x,y,z, viewer matrix, and zoom.  Must call one of g3_set_view_*()
void g3_set_view_matrix(vector *view_pos,matrix *view_matrix,float zoom)
{
	Assert( G3_count == 1 );

	View_zoom = zoom;
	View_position = *view_pos;

	View_matrix = *view_matrix;

	Eye_matrix = View_matrix;
	Eye_position = *view_pos;

	scale_matrix();

	Light_matrix = vmd_identity_matrix;
	Light_base.xyz.x = 0.0f;
	Light_base.xyz.y = 0.0f;
	Light_base.xyz.z = 0.0f;

	vm_vec_zero(&Object_position);
	Object_matrix = vmd_identity_matrix;
}


//set view from x,y,z & p,b,h, zoom.  Must call one of g3_set_view_*()
void g3_set_view_angles(vector *view_pos,angles *view_orient,float zoom)
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

	if (View_zoom <= 1.0) 		//zoom in by scaling z

		Matrix_scale.xyz.z =  Matrix_scale.xyz.z*View_zoom;

	else {			//zoom out by scaling x&y

		float s = (float)1.0 / View_zoom;

		Matrix_scale.xyz.x = Matrix_scale.xyz.x*s;
		Matrix_scale.xyz.y = Matrix_scale.xyz.y*s;
	}

	//now scale matrix elements

	vm_vec_scale(&View_matrix.vec.rvec,Matrix_scale.xyz.x );
	vm_vec_scale(&View_matrix.vec.uvec,Matrix_scale.xyz.y );
	vm_vec_scale(&View_matrix.vec.fvec,Matrix_scale.xyz.z );

}

ubyte g3_rotate_vertex_popped(vertex *dest,vector *src)
{
	vector tempv;

	Assert( G3_count == 1 );

	Assert( instance_depth > 0 );

	vm_vec_sub(&tempv,src,&instance_stack[0].p);
	vm_vec_rotate( (vector *)&dest->x, &tempv, &instance_stack[0].m );
	dest->flags = 0;	//not projected
	return g3_code_vertex(dest);
}	


//instance at specified point with specified orientation
//if matrix==NULL, don't modify matrix.  This will be like doing an offset   
//if pos==NULL, no position change
void g3_start_instance_matrix(vector *pos,matrix *orient, bool set_api)
{
	vector tempv;
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

		vm_copy_transpose_matrix(&tempm2,&Object_matrix);
		vm_vec_rotate(&tempv,pos,&tempm2);
		vm_vec_add(&Object_position, &Object_position, &tempv);
//		Object_position = tempv;
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
	vector saved_base = Light_base;
	
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
void g3_start_instance_angles(vector *pos,angles *orient)
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
vector G3_user_clip_normal;
vector G3_user_clip_point;

// Enables clipping with an arbritary plane.   This will be on
// until g3_stop_clip_plane is called or until next frame.
// The points passed should be relative to the instance.  Probably
// that means world coordinates.
/*
  This works like any other clip plane... if this is enabled and you
rotate a point, the CC_OFF_USER bit will be set in the clipping codes.
It is completely handled by most g3_draw primitives, except maybe lines.

  As far as performance, when enabled, it will slow down each point
rotation (or g3_code_vertex call) by a vector subtraction and dot
product.   It won't slow anything down for polys that are completely
clipped on or off by the plane, and will slow each clipped polygon by
not much more than any other clipping we do.
*/
void g3_start_user_clip_plane( vector *plane_point, vector *plane_normal )
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

	vector tempv;
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
int g3_point_behind_user_plane( vector *pnt )
{
	if ( G3_user_clip ) {
		vector tmp;
		vm_vec_sub( &tmp, pnt, &G3_user_clip_point );
		if ( vm_vec_dot( &tmp, &G3_user_clip_normal ) <= 0.0f )	{
			return 1;
		}
	}

	return 0;
}




