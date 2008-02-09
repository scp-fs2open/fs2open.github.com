/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Math/VecMat.cpp $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:58 $
 * $Author: penguin $
 *
 * C module containg functions for manipulating vectors and matricies
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 10    9/08/99 3:36p Jefff
 * Make sure argument of sqrt is positive in approach.
 * 
 * 9     6/22/99 1:51p Danw
 * Some sanity for vm_vec_dist_to_line(...)
 * 
 * 8     6/18/99 5:16p Dave
 * Added real beam weapon lighting. Fixed beam weapon sounds. Added MOTD
 * dialog to PXO screen.
 * 
 * 7     4/28/99 11:13p Dave
 * Temporary checkin of artillery code.
 * 
 * 6     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 5     1/12/99 12:53a Dave
 * More work on beam weapons - made collision detection very efficient -
 * collide against all object types properly - made 3 movement types
 * smooth. Put in test code to check for possible non-darkening pixels on
 * object textures.
 * 
 * 4     1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 3     11/18/98 4:10p Johnson
 * Add assert in vm_interpolate_matrix
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 72    9/11/98 10:10a Andsager
 * Optimize and rename matrix_decomp to vm_matrix_to_rot_axis_and_angle,
 * rename quatern_rot to vm_quaternion_rotate
 * 
 * 71    5/01/98 2:25p Andsager
 * Fix bug in matrix interpolate (approach) when in rotvel is above limit.
 * 
 * 70    4/07/98 3:10p Andsager
 * Make vm_test_parallel based on absolute difference.  Optimize matrix
 * decomp.  Fix potential bug in get_camera_limits with time = 0.
 * Optimize vm_forward_interpolate.
 * 
 * 69    4/06/98 8:54a Andsager
 * Fix bug where matrix interpolate gets accel of 0
 * 
 * 68    4/03/98 5:34p Andsager
 * Optimized approach and away (used in matrix interpolation)
 * 
 * 67    4/01/98 9:21p John
 * Made NDEBUG, optimized build with no warnings or errors.
 * 
 * 66    3/23/98 1:12p Andsager
 * Reformat matrix inerpolation code
 * 
 * 65    3/23/98 12:53p Andsager
 * 
 * 63    3/09/98 3:51p Mike
 * More error checking.
 * 
 * 62    2/26/98 3:28p John
 * Changed all sqrt's to use fl_sqrt.  Took out isqrt function
 * 
 * 61    2/02/98 5:12p Mike
 * Make vm_vec_rand_vec_quick() detect potential null vector condition and
 * recover.
 * 
 * 60    1/20/98 9:47a Mike
 * Suppress optimized compiler warnings.
 * Some secondary weapon work.
 * 
 * 59    12/17/97 5:44p Andsager
 * Change vm_matrix_interpolate so that it does not overshoot if optional
 * last parameter is 1
 * 
 * 58    9/30/97 5:04p Andsager
 * add vm_estimate_next_orientation
 * 
 * 57    9/28/97 2:17p Andsager
 * added vm_project_point_onto_plane
 * 
 * 56    9/09/97 10:15p Andsager
 * added vm_rotate_vec_to_body() and vm_rotate_vec_to_world()
 * 
 * 55    8/20/97 5:33p Andsager
 * added vm_vec_projection_parallel and vm_vec_projection_onto_surface
 * 
 * 54    8/20/97 9:51a Lawrance
 * swap x and y parameters in atan2_safe() to be consistent with library
 * atan2()
 * 
 * 53    8/20/97 9:40a Lawrance
 * modified special case values in atan2_safe()
 * 
 * 52    8/19/97 11:41p Lawrance
 * use atan2_safe() instead of atan2()
 * 
 * 51    8/18/97 4:46p Hoffoss
 * Added global default axis vector constants.
 * 
 * 50    8/03/97 3:54p Lawrance
 * added vm_find_bounding_sphere()
 * 
 * 49    7/28/97 3:40p Andsager
 * remove duplicate vm_forwarad_interpolate
 * 
 * 48    7/28/97 2:21p John
 * changed vecmat functions to not return src.  Started putting in code
 * for inline vector math.    Fixed some bugs with optimizer.
 * 
 * 47    7/28/97 3:24p Andsager
 * 
 * 46    7/28/97 2:41p Mike
 * Replace vm_forward_interpolate().
 * 
 * 45    7/28/97 1:18p Andsager
 * implement vm_fvec_matrix_interpolate(), which interpolates matrices on
 * xy and then z
 * 
 * 44    7/28/97 10:28a Mike
 * Use forward_interpolate() to prevent weird banking behavior.
 * 
 * Suppress a couple annoying mprints and clarify another.
 * 
 * 43    7/24/97 5:24p Andsager
 * implement forward vector interpolation
 * 
 * 42    7/10/97 8:52a Andsager
 * optimization and clarification of matrix_decomp()
 * 
 * 41    7/09/97 2:54p Mike
 * More matrix_decomp optimization.
 * 
 * 40    7/09/97 2:52p Mike
 * Optimize and error-prevent matrix_decomp().
 * 
 * 39    7/09/97 12:05a Mike
 * Error prevention in matrix_interpolate().
 * 
 * 38    7/07/97 11:58p Lawrance
 * add get_camera_limits()
 * 
 * 37    7/03/97 11:22a Mike
 * Fix bug in matrix_interpolate.  Was doing result = goal instead of
 * *result = *goal.
 * 
 * 36    7/03/97 9:27a Mike
 * Hook in Dave's latest version of matrix_interpolate which doesn't jerk.
 * 
 * 35    7/02/97 4:25p Mike
 * Add matrix_interpolate(), but don't call it.
 * 
 * 34    7/01/97 3:27p Mike
 * Improve skill level support.
 * 
 * 33    6/25/97 12:27p Hoffoss
 * Added some functions I needed for Fred.
 * 
 * 32    5/21/97 8:49a Lawrance
 * added vm_vec_same()
 * 
 * 31    4/15/97 4:00p Mike
 * Intermediate checkin caused by getting other files.  Working on camera
 * slewing system.
 * 
 * 30    4/10/97 3:20p Mike
 * Change hull damage to be like shields.
 * 
 * 29    3/17/97 1:55p Hoffoss
 * Added function for error checking matrices.
 * 
 * 28    3/11/97 10:46p Mike
 * Fix make_rand_vec_quick.  Was generating values in -0.5..1.5 instead of
 * -1.0..1.0.
 * 
 * 27    3/06/97 5:36p Mike
 * Change vec_normalize_safe() back to vec_normalize().
 * Spruce up docking a bit.
 * 
 * 26    3/06/97 10:56a Mike
 * Write error checking version of vm_vec_normalize().
 * Fix resultant problems.
 * 
 * 25    3/04/97 3:30p John
 * added function to interpolate an angle.
 * 
 * 24    2/26/97 10:32a John
 * changed debris collision to use vm_vec_dist_squared.  Changed
 * vm_vec_dist_squared to not int3 on bad values.
 * 
 * 23    2/25/97 5:54p Hoffoss
 * Improved vector and matrix compare functions.
 * 
 * 22    2/25/97 5:28p Hoffoss
 * added some commented out test code.
 * 
 * 21    2/25/97 5:12p John
 * Added functions to see if two matrices or vectors are close.
 * 
 * $NoKeywords: $
 *
*/

#include <stdio.h>
#include <math.h>

#include "vecmat.h"
#include "floating.h"

#define	SMALL_NUM	1e-7
#define	SMALLER_NUM	1e-20
#define	CONVERT_RADIANS	0.017453		// conversion factor from degrees to radians
int index_largest (float a, float b, float c);	// returns index of largest, NO_LARGEST if all less than SMALL_NUM


vector vmd_zero_vector = ZERO_VECTOR;
vector vmd_x_vector = { 1.0f, 0.0f, 0.0f };
vector vmd_y_vector = { 0.0f, 1.0f, 0.0f };
vector vmd_z_vector = { 0.0f, 0.0f, 1.0f };
matrix vmd_identity_matrix = IDENTITY_MATRIX;

#define	UNINITIALIZED_VALUE	-12345678.9f

// -----------------------------------------------------------
// atan2_safe()
//
// Wrapper around atan2() that used atan() to calculate angle.  Safe
// for optimized builds.  Handles special cases when x == 0.
//
float atan2_safe(float y, float x)
{
	float ang;

	// special case, x == 0
	if ( x == 0 ) {
		if ( y == 0 ) 
			ang = 0.0f;
		else if ( y > 0 )
			ang = PI/2;
		else
			ang = -PI/2;

		return ang;
	}
	
	ang = (float)atan(y/x);
	if ( x < 0 ){
		ang += PI;
	}

	return ang;
}

// ---------------------------------------------------------------------
// vm_vec_component()
//
// finds projection of a vector along a unit (normalized) vector 
//
float vm_vec_projection_parallel(vector *component, vector *src, vector *unit_vec)
{
	float mag;
	Assert( vm_vec_mag(unit_vec) > 0.999f  &&  vm_vec_mag(unit_vec) < 1.001f );

	mag = vm_vec_dotprod(src, unit_vec);
	vm_vec_copy_scale(component, unit_vec, mag);
	return mag;
}

// ---------------------------------------------------------------------
// vm_vec_projection_onto_plane()
//
// finds projection of a vector onto a plane specified by a unit normal vector 
//
void vm_vec_projection_onto_plane(vector *projection, vector *src, vector *unit_normal)
{
	float mag;
	Assert( vm_vec_mag(unit_normal) > 0.999f  &&  vm_vec_mag(unit_normal) < 1.001f );

	mag = vm_vec_dotprod(src, unit_normal);
	*projection = *src;
	vm_vec_scale_add2(projection, unit_normal, -mag);
}

// ---------------------------------------------------------------------
// vm_vec_project_point_onto_plane()
//
// finds the point on a plane closest to a given point
// moves the point in the direction of the plane normal until it is on the plane
//
void vm_project_point_onto_plane(vector *new_point, vector *point, vector *plane_normal, vector *plane_point)
{
	float D;		// plane constant in Ax+By+Cz+D = 0   or   dot(X,n) - dot(Xp,n) = 0, so D = -dot(Xp,n)
	float dist;
	Assert( vm_vec_mag(plane_normal) > 0.999f  &&  vm_vec_mag(plane_normal) < 1.001f );

	D = -vm_vec_dotprod(plane_point, plane_normal);
	dist = vm_vec_dotprod(point, plane_normal) + D;

	*new_point = *point;
	vm_vec_scale_add2(new_point, plane_normal, -dist);
}

//	Take abs(x), then sqrt.  Could insert warning message if desired.
float asqrt(float x)
{
	if (x < 0.0f)
		return fl_sqrt(-x);
	else
		return fl_sqrt(x);
}

void vm_set_identity(matrix *m)
{
	m->rvec.x = 1.0f;	m->rvec.y = 0.0f;	m->rvec.z = 0.0f;
	m->uvec.x = 0.0f;	m->uvec.y = 1.0f;	m->uvec.z = 0.0f;
	m->fvec.x = 0.0f;	m->fvec.y = 0.0f;	m->fvec.z = 1.0f;
}

//adds two vectors, fills in dest, returns ptr to dest
//ok for dest to equal either source, but should use vm_vec_add2() if so
#ifndef _INLINE_VECMAT
void vm_vec_add(vector *dest,vector *src0,vector *src1)
{
	dest->x = src0->x + src1->x;
	dest->y = src0->y + src1->y;
	dest->z = src0->z + src1->z;
}
#endif

//subs two vectors, fills in dest, returns ptr to dest
//ok for dest to equal either source, but should use vm_vec_sub2() if so
#ifndef _INLINE_VECMAT
void vm_vec_sub(vector *dest,vector *src0,vector *src1)
{
	dest->x = src0->x - src1->x;
	dest->y = src0->y - src1->y;
	dest->z = src0->z - src1->z;
}
#endif


//adds one vector to another. returns ptr to dest
//dest can equal source
#ifndef _INLINE_VECMAT
void vm_vec_add2(vector *dest,vector *src)
{
	dest->x += src->x;
	dest->y += src->y;
	dest->z += src->z;
}
#endif

//subs one vector from another, returns ptr to dest
//dest can equal source
#ifndef _INLINE_VECMAT
void vm_vec_sub2(vector *dest,vector *src)
{
	dest->x -= src->x;
	dest->y -= src->y;
	dest->z -= src->z;
}
#endif

//averages two vectors. returns ptr to dest
//dest can equal either source
vector *vm_vec_avg(vector *dest,vector *src0,vector *src1)
{
	dest->x = (src0->x + src1->x) * 0.5f;
	dest->y = (src0->y + src1->y) * 0.5f;
	dest->z = (src0->z + src1->z) * 0.5f;

	return dest;
}


//averages four vectors. returns ptr to dest
//dest can equal any source
vector *vm_vec_avg4(vector *dest,vector *src0,vector *src1,vector *src2,vector *src3)
{
	dest->x = (src0->x + src1->x + src2->x + src3->x) * 0.25f;
	dest->y = (src0->y + src1->y + src2->y + src3->y) * 0.25f;
	dest->z = (src0->z + src1->z + src2->z + src3->z) * 0.25f;
	return dest;
}


//scales a vector in place.  returns ptr to vector
#ifndef _INLINE_VECMAT
void vm_vec_scale(vector *dest,float s)
{
	dest->x = dest->x * s;
	dest->y = dest->y * s;
	dest->z = dest->z * s;
}
#endif


//scales and copies a vector.  returns ptr to dest
#ifndef _INLINE_VECMAT
void vm_vec_copy_scale(vector *dest,vector *src,float s)
{
	dest->x = src->x*s;
	dest->y = src->y*s;
	dest->z = src->z*s;
}
#endif

//scales a vector, adds it to another, and stores in a 3rd vector
//dest = src1 + k * src2
#ifndef _INLINE_VECMAT
void vm_vec_scale_add(vector *dest,vector *src1,vector *src2,float k)
{
	dest->x = src1->x + src2->x*k;
	dest->y = src1->y + src2->y*k;
	dest->z = src1->z + src2->z*k;
}
#endif

//scales a vector and adds it to another
//dest += k * src
#ifndef _INLINE_VECMAT
void vm_vec_scale_add2(vector *dest,vector *src,float k)
{
	dest->x += src->x*k;
	dest->y += src->y*k;
	dest->z += src->z*k;
}
#endif

//scales a vector and adds it to another
//dest += k * src
#ifndef _INLINE_VECMAT
void vm_vec_scale_sub2(vector *dest,vector *src,float k)
{
	dest->x -= src->x*k;
	dest->y -= src->y*k;
	dest->z -= src->z*k;
}
#endif

//scales a vector in place, taking n/d for scale.  returns ptr to vector
//dest *= n/d
#ifndef _INLINE_VECMAT
void vm_vec_scale2(vector *dest,float n,float d)
{	
	d = 1.0f/d;

	dest->x = dest->x* n * d;
	dest->y = dest->y* n * d;
	dest->z = dest->z* n * d;
}
#endif

//returns dot product of 2 vectors
#ifndef _INLINE_VECMAT
float vm_vec_dotprod(vector *v0,vector *v1)
{
	return (v1->x*v0->x)+(v1->y*v0->y)+(v1->z*v0->z);
}
#endif


//returns dot product of <x,y,z> and vector
#ifndef _INLINE_VECMAT
float vm_vec_dot3(float x,float y,float z,vector *v)
{
	return (x*v->x)+(y*v->y)+(z*v->z);
}
#endif

//returns magnitude of a vector
float vm_vec_mag(vector *v)
{
	float x,y,z,mag1, mag2;
	x = v->x*v->x;
	y = v->y*v->y;
	z = v->z*v->z;
	mag1 = x+y+z;
	if ( mag1 < 0.0 )
		Int3();
	mag2 = fl_sqrt(mag1);
	if ( mag2 < 0.0 )
		Int3();
	return mag2;
}

//returns squared magnitude of a vector, useful if you want to compare distances
float vm_vec_mag_squared(vector *v)
{
	float x,y,z,mag1;
	x = v->x*v->x;
	y = v->y*v->y;
	z = v->z*v->z;
	mag1 = x+y+z;
	return mag1;
}

float vm_vec_dist_squared(vector *v0, vector *v1)
{
	float dx, dy, dz;

	dx = v0->x - v1->x;
	dy = v0->y - v1->y;
	dz = v0->z - v1->z;
	return dx*dx + dy*dy + dz*dz;
}

//computes the distance between two points. (does sub and mag)
float vm_vec_dist(vector *v0,vector *v1)
{
	float t1;
	vector t;

	vm_vec_sub(&t,v0,v1);

	t1 = vm_vec_mag(&t);

	return t1;
}



//computes an approximation of the magnitude of the vector
//uses dist = largest + next_largest*3/8 + smallest*3/16
float vm_vec_mag_quick(vector *v)
{
	float a,b,c,bc, t;

	if ( v->x < 0.0 )
		a = -v->x;
	else
		a = v->x;

	if ( v->y < 0.0 )
		b = -v->y;
	else
		b = v->y;

	if ( v->z < 0.0 )
		c = -v->z;
	else
		c = v->z;

	if (a < b) {
		float t=a; a=b; b=t;
	}

	if (b < c) {
		float t=b; b=c; c=t;

		if (a < b) {
			float t=a; a=b; b=t;
		}
	}

	bc = (b * 0.25f) + (c * 0.125f);

	t = a + bc + (bc * 0.5f);

	return t;
}

//computes an approximation of the distance between two points.
//uses dist = largest + next_largest*3/8 + smallest*3/16
float vm_vec_dist_quick(vector *v0,vector *v1)
{
	vector t;

	vm_vec_sub(&t,v0,v1);

	return vm_vec_mag_quick(&t);
}

//normalize a vector. returns mag of source vec
float vm_vec_copy_normalize(vector *dest,vector *src)
{
	float m;

	m = vm_vec_mag(src);

	//	Mainly here to trap attempts to normalize a null vector.
	if (m <= 0.0f) {
		Warning(LOCATION,	"Null vector in vector normalize.\n"
								"Trace out of vecmat.cpp and find offending code.\n");
		dest->x = 1.0f;
		dest->y = 0.0f;
		dest->z = 0.0f;

		return 1.0f;
	}

	float im = 1.0f / m;

	dest->x = src->x * im;
	dest->y = src->y * im;
	dest->z = src->z * im;
	
	return m;
}

//normalize a vector. returns mag of source vec
float vm_vec_normalize(vector *v)
{
	float t;
	t = vm_vec_copy_normalize(v,v);
	return t;
}

// Normalize a vector.
//	If vector is 0,0,0, return 1,0,0.
//	Don't generate a Warning().
// returns mag of source vec
float vm_vec_normalize_safe(vector *v)
{
	float m;

	m = vm_vec_mag(v);

	//	Mainly here to trap attempts to normalize a null vector.
	if (m <= 0.0f) {
		v->x = 1.0f;
		v->y = 0.0f;
		v->z = 0.0f;
		return 1.0f;
	}

	float im = 1.0f / m;

	v->x *= im;
	v->y *= im;
	v->z *= im;

	return m;

}


//returns approximation of 1/magnitude of a vector
float vm_vec_imag(vector *v)
{
//	return 1.0f / sqrt( (v->x*v->x)+(v->y*v->y)+(v->z*v->z) );	
	return fl_isqrt( (v->x*v->x)+(v->y*v->y)+(v->z*v->z) );
}

//normalize a vector. returns 1/mag of source vec. uses approx 1/mag
float vm_vec_copy_normalize_quick(vector *dest,vector *src)
{
//	return vm_vec_copy_normalize(dest, src);
	float im;

	im = vm_vec_imag(src);

	Assert(im > 0.0f);

	dest->x = src->x*im;
	dest->y = src->y*im;
	dest->z = src->z*im;

	return 1.0f/im;
}

//normalize a vector. returns mag of source vec. uses approx mag
float vm_vec_normalize_quick(vector *src)
{
//	return vm_vec_normalize(src);

	float im;

	im = vm_vec_imag(src);

	Assert(im > 0.0f);

	src->x = src->x*im;
	src->y = src->y*im;
	src->z = src->z*im;

	return 1.0f/im;

}

//normalize a vector. returns mag of source vec. uses approx mag
float vm_vec_copy_normalize_quick_mag(vector *dest,vector *src)
{
//	return vm_vec_copy_normalize(dest, src);

	float m;

	m = vm_vec_mag_quick(src);

	Assert(m > 0.0f);

	float im = 1.0f / m;

	dest->x = src->x * im;
	dest->y = src->y * im;
	dest->z = src->z * im;

	return m;

}

//normalize a vector. returns mag of source vec. uses approx mag
float vm_vec_normalize_quick_mag(vector *v)
{
//	return vm_vec_normalize(v);
	float m;

	m = vm_vec_mag_quick(v);

	Assert(m > 0.0f);

	v->x = v->x*m;
	v->y = v->y*m;
	v->z = v->z*m;

	return m;

}



//return the normalized direction vector between two points
//dest = normalized(end - start).  Returns mag of direction vector
//NOTE: the order of the parameters matches the vector subtraction
float vm_vec_normalized_dir(vector *dest,vector *end,vector *start)
{
	float t;

	vm_vec_sub(dest,end,start);
	t = vm_vec_normalize(dest);
	return t;
}

//return the normalized direction vector between two points
//dest = normalized(end - start).  Returns mag of direction vector
//NOTE: the order of the parameters matches the vector subtraction
float vm_vec_normalized_dir_quick(vector *dest,vector *end,vector *start)
{
	vm_vec_sub(dest,end,start);

	return vm_vec_normalize_quick(dest);
}

//return the normalized direction vector between two points
//dest = normalized(end - start).  Returns mag of direction vector
//NOTE: the order of the parameters matches the vector subtraction
float vm_vec_normalized_dir_quick_mag(vector *dest,vector *end,vector *start)
{
	float t;
	vm_vec_sub(dest,end,start);

	t = vm_vec_normalize_quick_mag(dest);
	return t;
}

//computes surface normal from three points. result is normalized
//returns ptr to dest
//dest CANNOT equal either source
vector *vm_vec_normal(vector *dest,vector *p0,vector *p1,vector *p2)
{
	vm_vec_perp(dest,p0,p1,p2);

	vm_vec_normalize(dest);

	return dest;
}


//computes cross product of two vectors.
//Note: this magnitude of the resultant vector is the
//product of the magnitudes of the two source vectors.  This means it is
//quite easy for this routine to overflow and underflow.  Be careful that
//your inputs are ok.
vector *vm_vec_crossprod(vector *dest,vector *src0,vector *src1)
{
	dest->x = (src0->y * src1->z) - (src0->z * src1->y);
	dest->y = (src0->z * src1->x) - (src0->x * src1->z);
	dest->z = (src0->x * src1->y) - (src0->y * src1->x);

	return dest;
}

// test if 2 vectors are parallel or not.
int vm_test_parallel(vector *src0, vector *src1)
{
	if ( (fl_abs(src0->x - src1->x) < 1e-4) && (fl_abs(src0->y - src1->y) < 1e-4) && (fl_abs(src0->z - src1->z) < 1e-4) ) {
		return 1;
	} else {
		return 0;
	}
}

//computes non-normalized surface normal from three points.
//returns ptr to dest
//dest CANNOT equal either source
vector *vm_vec_perp(vector *dest,vector *p0,vector *p1,vector *p2)
{
	vector t0,t1;

	vm_vec_sub(&t0,p1,p0);
	vm_vec_sub(&t1,p2,p1);

	return vm_vec_crossprod(dest,&t0,&t1);
}


//computes the delta angle between two vectors.
//vectors need not be normalized. if they are, call vm_vec_delta_ang_norm()
//the forward vector (third parameter) can be NULL, in which case the absolute
//value of the angle in returned.  Otherwise the angle around that vector is
//returned.
float vm_vec_delta_ang(vector *v0,vector *v1,vector *fvec)
{
	float t;
	vector t0,t1,t2;

	vm_vec_copy_normalize(&t0,v0);
	vm_vec_copy_normalize(&t1,v1);
	vm_vec_copy_normalize(&t2,fvec);

	t = vm_vec_delta_ang_norm(&t0,&t1,&t2);

	return t;
}

//computes the delta angle between two normalized vectors.
float vm_vec_delta_ang_norm(vector *v0,vector *v1,vector *fvec)
{
	float a;
	vector t;

	a = (float)acos(vm_vec_dot(v0,v1));

	if (fvec) {
		vm_vec_cross(&t,v0,v1);
		if ( vm_vec_dotprod(&t,fvec) < 0.0 )	{
			a = -a;
		}
	}

	return a;
}

matrix *sincos_2_matrix(matrix *m,float sinp,float cosp,float sinb,float cosb,float sinh,float cosh)
{
	float sbsh,cbch,cbsh,sbch;


	sbsh = sinb*sinh;
	cbch = cosb*cosh;
	cbsh = cosb*sinh;
	sbch = sinb*cosh;

	m->rvec.x = cbch + sinp*sbsh;		//m1
	m->uvec.z = sbsh + sinp*cbch;		//m8

	m->uvec.x = sinp*cbsh - sbch;		//m2
	m->rvec.z = sinp*sbch - cbsh;		//m7

	m->fvec.x = sinh*cosp;				//m3
	m->rvec.y = sinb*cosp;				//m4
	m->uvec.y = cosb*cosp;				//m5
	m->fvec.z = cosh*cosp;				//m9

	m->fvec.y = -sinp;								//m6


	return m;

}

//computes a matrix from a set of three angles.  returns ptr to matrix
matrix *vm_angles_2_matrix(matrix *m,angles *a)
{
	matrix * t;
	float sinp,cosp,sinb,cosb,sinh,cosh;

	sinp = (float)sin(a->p); cosp = (float)cos(a->p);
	sinb = (float)sin(a->b); cosb = (float)cos(a->b);
	sinh = (float)sin(a->h); cosh = (float)cos(a->h);

	t = sincos_2_matrix(m,sinp,cosp,sinb,cosb,sinh,cosh);

	return t;
}

//computes a matrix from one angle.
//	angle_index = 0,1,2 for p,b,h
matrix *vm_angle_2_matrix(matrix *m, float a, int angle_index)
{
	matrix * t;
	float sinp,cosp,sinb,cosb,sinh,cosh;

	sinp = (float)sin(0.0f);	cosp = (float)cos(0.0f);
	sinb = (float)sin(0.0f);	cosb = (float)cos(0.0f);
	sinh = (float)sin(0.0f);	cosh = (float)cos(0.0f);

	switch (angle_index) {
	case 0:
		sinp = (float)sin(a); cosp = (float)cos(a);
		break;
	case 1:
		sinb = (float)sin(a); cosb = (float)cos(a);
		break;
	case 2:
		sinh = (float)sin(a); cosh = (float)cos(a);
		break;
	}

	t = sincos_2_matrix(m,sinp,cosp,sinb,cosb,sinh,cosh);

	return t;
}


//computes a matrix from a forward vector and an angle
matrix *vm_vec_ang_2_matrix(matrix *m,vector *v,float a)
{
	matrix * t;
	float sinb,cosb,sinp,cosp,sinh,cosh;

	sinb = (float)sin(a); cosb = (float)cos(a);

	sinp = -v->y;
	cosp = fl_sqrt(1.0 - sinp*sinp);

	sinh = v->x / cosp;
	cosh = v->z / cosp;

	t = sincos_2_matrix(m,sinp,cosp,sinb,cosb,sinh,cosh);

	return t;
}


//computes a matrix from one or more vectors. The forward vector is required,
//with the other two being optional.  If both up & right vectors are passed,
//the up vector is used.  If only the forward vector is passed, a bank of
//zero is assumed
//returns ptr to matrix
matrix *vm_vector_2_matrix(matrix *m,vector *fvec,vector *uvec,vector *rvec)
{
	vector *xvec=&m->rvec,*yvec=&m->uvec,*zvec=&m->fvec;


	Assert(fvec != NULL);

	//	This had been commented out, but that's bogus.  Code below relies on a valid zvec.
	if (vm_vec_copy_normalize(zvec,fvec) == 0.0) {
		Assert(0);
		return m;
	}

	if (uvec == NULL) {

		if (rvec == NULL) {		//just forward vec

bad_vector2:
	;

			if ((zvec->x==0.0) && (zvec->z==0.0)) {		//forward vec is straight up or down

				m->rvec.x = (float)1.0;
				m->uvec.z = (zvec->y<0.0)?(float)1.0:(float)-1.0;

				m->rvec.y = m->rvec.z = m->uvec.x = m->uvec.y = (float)0.0;
			}
			else { 		//not straight up or down

				xvec->x = zvec->z;
				xvec->y = (float)0.0;
				xvec->z = -zvec->x;

				vm_vec_normalize(xvec);

				vm_vec_crossprod(yvec,zvec,xvec);

			}

		}
		else {						//use right vec

			if (vm_vec_copy_normalize(xvec,rvec) == 0.0)
				goto bad_vector2;

			vm_vec_crossprod(yvec,zvec,xvec);

			//normalize new perpendicular vector
			if (vm_vec_normalize(yvec) == 0.0)
				goto bad_vector2;

			//now recompute right vector, in case it wasn't entirely perpendiclar
			vm_vec_crossprod(xvec,yvec,zvec);

		}
	}
	else {		//use up vec

		if (vm_vec_copy_normalize(yvec,uvec) == 0.0f)
			goto bad_vector2;

		vm_vec_crossprod(xvec,yvec,zvec);
		
		//normalize new perpendicular vector
		if (vm_vec_normalize(xvec) == 0.0)
			goto bad_vector2;

		//now recompute up vector, in case it wasn't entirely perpendiclar
		vm_vec_crossprod(yvec,zvec,xvec);

	}
	return m;
}

//quicker version of vm_vector_2_matrix() that takes normalized vectors
matrix *vm_vector_2_matrix_norm(matrix *m,vector *fvec,vector *uvec,vector *rvec)
{
	vector *xvec=&m->rvec,*yvec=&m->uvec,*zvec=&m->fvec;


	Assert(fvec != NULL);

	*zvec = *fvec;

	if (uvec == NULL) {

		if (rvec == NULL) {		//just forward vec

bad_vector2:
	;

			if ((zvec->x==0.0) && (zvec->z==0.0)) {		//forward vec is straight up or down

				m->rvec.x = (float)1.0;
				m->uvec.z = (zvec->y<0.0)?(float)1.0:(float)-1.0;

				m->rvec.y = m->rvec.z = m->uvec.x = m->uvec.y = (float)0.0;
			}
			else { 		//not straight up or down

				xvec->x = zvec->z;
				xvec->y = (float)0.0;
				xvec->z = -zvec->x;

				vm_vec_normalize(xvec);

				vm_vec_crossprod(yvec,zvec,xvec);

			}

		}
		else {						//use right vec

			vm_vec_crossprod(yvec,zvec,xvec);

			//normalize new perpendicular vector
			if (vm_vec_normalize(yvec) == 0.0)
				goto bad_vector2;

			//now recompute right vector, in case it wasn't entirely perpendiclar
			vm_vec_crossprod(xvec,yvec,zvec);

		}
	}
	else {		//use up vec

		vm_vec_crossprod(xvec,yvec,zvec);
		
		//normalize new perpendicular vector
		if (vm_vec_normalize(xvec) == 0.0)
			goto bad_vector2;

		//now recompute up vector, in case it wasn't entirely perpendiclar
		vm_vec_crossprod(yvec,zvec,xvec);

	}


	return m;
}


//rotates a vector through a matrix. returns ptr to dest vector
//dest CANNOT equal source
vector *vm_vec_rotate(vector *dest,vector *src,matrix *m)
{
	dest->x = (src->x*m->rvec.x)+(src->y*m->rvec.y)+(src->z*m->rvec.z);
	dest->y = (src->x*m->uvec.x)+(src->y*m->uvec.y)+(src->z*m->uvec.z);
	dest->z = (src->x*m->fvec.x)+(src->y*m->fvec.y)+(src->z*m->fvec.z);

	return dest;
}

//rotates a vector through the transpose of the given matrix. 
//returns ptr to dest vector
//dest CANNOT equal source
// This is a faster replacement for this common code sequence:
//    vm_copy_transpose_matrix(&tempm,src_matrix);
//    vm_vec_rotate(dst_vec,src_vect,&tempm);
// Replace with:
//    vm_vec_unrotate(dst_vec,src_vect, src_matrix)
//
// THIS DOES NOT ACTUALLY TRANSPOSE THE SOURCE MATRIX!!! So if
// you need it transposed later on, you should use the 
// vm_vec_transpose() / vm_vec_rotate() technique.

vector *vm_vec_unrotate(vector *dest,vector *src,matrix *m)
{
	dest->x = (src->x*m->rvec.x)+(src->y*m->uvec.x)+(src->z*m->fvec.x);
	dest->y = (src->x*m->rvec.y)+(src->y*m->uvec.y)+(src->z*m->fvec.y);
	dest->z = (src->x*m->rvec.z)+(src->y*m->uvec.z)+(src->z*m->fvec.z);

	return dest;
}

//transpose a matrix in place. returns ptr to matrix
matrix *vm_transpose_matrix(matrix *m)
{
	float t;

	t = m->uvec.x;  m->uvec.x = m->rvec.y;  m->rvec.y = t;
	t = m->fvec.x;  m->fvec.x = m->rvec.z;  m->rvec.z = t;
	t = m->fvec.y;  m->fvec.y = m->uvec.z;  m->uvec.z = t;

	return m;
}

//copy and transpose a matrix. returns ptr to matrix
//dest CANNOT equal source. use vm_transpose_matrix() if this is the case
matrix *vm_copy_transpose_matrix(matrix *dest,matrix *src)
{

	Assert(dest != src);

	dest->rvec.x = src->rvec.x;
	dest->rvec.y = src->uvec.x;
	dest->rvec.z = src->fvec.x;

	dest->uvec.x = src->rvec.y;
	dest->uvec.y = src->uvec.y;
	dest->uvec.z = src->fvec.y;

	dest->fvec.x = src->rvec.z;
	dest->fvec.y = src->uvec.z;
	dest->fvec.z = src->fvec.z;


	return dest;
}

//mulitply 2 matrices, fill in dest.  returns ptr to dest
//dest CANNOT equal either source
matrix *vm_matrix_x_matrix(matrix *dest,matrix *src0,matrix *src1)
{

	Assert(dest!=src0 && dest!=src1);

	dest->rvec.x = vm_vec_dot3(src0->rvec.x,src0->uvec.x,src0->fvec.x, &src1->rvec);
	dest->uvec.x = vm_vec_dot3(src0->rvec.x,src0->uvec.x,src0->fvec.x, &src1->uvec);
	dest->fvec.x = vm_vec_dot3(src0->rvec.x,src0->uvec.x,src0->fvec.x, &src1->fvec);

	dest->rvec.y = vm_vec_dot3(src0->rvec.y,src0->uvec.y,src0->fvec.y, &src1->rvec);
	dest->uvec.y = vm_vec_dot3(src0->rvec.y,src0->uvec.y,src0->fvec.y, &src1->uvec);
	dest->fvec.y = vm_vec_dot3(src0->rvec.y,src0->uvec.y,src0->fvec.y, &src1->fvec);

	dest->rvec.z = vm_vec_dot3(src0->rvec.z,src0->uvec.z,src0->fvec.z, &src1->rvec);
	dest->uvec.z = vm_vec_dot3(src0->rvec.z,src0->uvec.z,src0->fvec.z, &src1->uvec);
	dest->fvec.z = vm_vec_dot3(src0->rvec.z,src0->uvec.z,src0->fvec.z, &src1->fvec);


	return dest;
}


//extract angles from a matrix
angles *vm_extract_angles_matrix(angles *a,matrix *m)
{
	float sinh,cosh,cosp;

	if (m->fvec.x==0.0 && m->fvec.z==0.0)		//zero head
		a->h = (float)0.0;
	else
		// a->h = (float)atan2(m->fvec.z,m->fvec.x);
		a->h = (float)atan2_safe(m->fvec.x,m->fvec.z);

	sinh = (float)sin(a->h); cosh = (float)cos(a->h);

	if (fl_abs(sinh) > fl_abs(cosh))				//sine is larger, so use it
		cosp = m->fvec.x*sinh;
	else											//cosine is larger, so use it
		cosp = m->fvec.z*cosh;

	if (cosp==0.0 && m->fvec.y==0.0)
		a->p = (float)0.0;
	else
		// a->p = (float)atan2(cosp,-m->fvec.y);
		a->p = (float)atan2_safe(-m->fvec.y, cosp);


	if (cosp == 0.0)	//the cosine of pitch is zero.  we're pitched straight up. say no bank

		a->b = (float)0.0;

	else {
		float sinb,cosb;

		sinb = m->rvec.y/cosp;
		cosb = m->uvec.y/cosp;

		if (sinb==0.0 && cosb==0.0)
			a->b = (float)0.0;
		else
			// a->b = (float)atan2(cosb,sinb);
			a->b = (float)atan2_safe(sinb,cosb);
	}


	return a;
}


//extract heading and pitch from a vector, assuming bank==0
angles *vm_extract_angles_vector_normalized(angles *a,vector *v)
{

	a->b = 0.0f;		//always zero bank

	a->p = (float)asin(-v->y);

	if (v->x==0.0f && v->z==0.0f)
		a->h = (float)0.0;
	else
		a->h = (float)atan2_safe(v->z,v->x);

	return a;
}

//extract heading and pitch from a vector, assuming bank==0
angles *vm_extract_angles_vector(angles *a,vector *v)
{
	vector t;

	if (vm_vec_copy_normalize(&t,v) != 0.0)
		vm_extract_angles_vector_normalized(a,&t);

	return a;
}

//compute the distance from a point to a plane.  takes the normalized normal
//of the plane (ebx), a point on the plane (edi), and the point to check (esi).
//returns distance in eax
//distance is signed, so negative dist is on the back of the plane
float vm_dist_to_plane(vector *checkp,vector *norm,vector *planep)
{
	float t1;
	vector t;

	vm_vec_sub(&t,checkp,planep);

	t1 = vm_vec_dot(&t,norm);

	return t1;

}

// Given mouse movement in dx, dy, returns a 3x3 rotation matrix in RotMat.
// Taken from Graphics Gems III, page 51, "The Rolling Ball"
// Example:
//if ( (Mouse.dx!=0) || (Mouse.dy!=0) ) {
//   GetMouseRotation( Mouse.dx, Mouse.dy, &MouseRotMat );
//   vm_matrix_x_matrix(&tempm,&LargeView.ev_matrix,&MouseRotMat);
//   LargeView.ev_matrix = tempm;
//}


void vm_trackball( int idx, int idy, matrix * RotMat )
{
	float dr, cos_theta, sin_theta, denom, cos_theta1;
	float Radius = 100.0f;
	float dx,dy;
	float dxdr,dydr;

	idy *= -1;

	dx = (float)idx; dy = (float)idy;

	dr = fl_sqrt(dx*dx+dy*dy);

	denom = fl_sqrt(Radius*Radius+dr*dr);
	
	cos_theta = Radius/denom;
	sin_theta = dr/denom;

	cos_theta1 = 1.0f - cos_theta;

	dxdr = dx/dr;
	dydr = dy/dr;

	RotMat->rvec.x = cos_theta + (dydr*dydr)*cos_theta1;
	RotMat->uvec.x = - ((dxdr*dydr)*cos_theta1);
	RotMat->fvec.x = (dxdr*sin_theta);

	RotMat->rvec.y = RotMat->uvec.x;
	RotMat->uvec.y = cos_theta + ((dxdr*dxdr)*cos_theta1);
	RotMat->fvec.y = (dydr*sin_theta);

	RotMat->rvec.z = -RotMat->fvec.x;
	RotMat->uvec.z = -RotMat->fvec.y;
	RotMat->fvec.z = cos_theta;
}

//	Compute the outer product of A = A * transpose(A).  1x3 vector becomes 3x3 matrix.
void vm_vec_outer_product(matrix *mat, vector *vec)
{
	mat->rvec.x = vec->x * vec->x;
	mat->rvec.y = vec->x * vec->y;
	mat->rvec.z = vec->x * vec->z;

	mat->uvec.x = vec->y * vec->x;
	mat->uvec.y = vec->y * vec->y;
	mat->uvec.z = vec->y * vec->z;

	mat->fvec.x = vec->z * vec->x;
	mat->fvec.y = vec->z * vec->y;
	mat->fvec.z = vec->z * vec->z;
}

//	Find the point on the line between p0 and p1 that is nearest to int_pnt.
//	Stuff result in nearest_point.
//	Uses algorithm from page 148 of Strang, Linear Algebra and Its Applications.
//	Returns value indicating whether *nearest_point is between *p0 and *p1.
//	0.0f means *nearest_point is *p0, 1.0f means it's *p1. 2.0f means it's beyond p1 by 2x.
//	-1.0f means it's "before" *p0 by 1x.
float find_nearest_point_on_line(vector *nearest_point, vector *p0, vector *p1, vector *int_pnt)
{
	vector	norm, xlated_int_pnt, projected_point;
	matrix	mat;
	float		mag, dot;

	vm_vec_sub(&norm, p1, p0);
	vm_vec_sub(&xlated_int_pnt, int_pnt, p0);

	if (IS_VEC_NULL(&norm)) {
		*nearest_point = *int_pnt;
		return 9999.9f;
	}

	mag = vm_vec_normalize(&norm);			//	Normalize vector so we don't have to divide by dot product.
	
	if (mag < 0.01f) {
		*nearest_point = *int_pnt;
		return 9999.9f;
		// Warning(LOCATION, "Very small magnitude in find_nearest_point_on_line.\n");
	}

	vm_vec_outer_product(&mat, &norm);

	vm_vec_rotate(&projected_point, &xlated_int_pnt, &mat);
	vm_vec_add(nearest_point, &projected_point, p0);

	dot = vm_vec_dot(&norm, &projected_point);

	return dot/mag;
}

//make sure matrix is orthogonal
//computes a matrix from one or more vectors. The forward vector is required,
//with the other two being optional.  If both up & right vectors are passed,
//the up vector is used.  If only the forward vector is passed, a bank of
//zero is assumed
//returns ptr to matrix
void vm_orthogonalize_matrix(matrix *m_src)
{
	float umag, rmag;
	matrix tempm;
	matrix * m = &tempm;

	if (vm_vec_copy_normalize(&m->fvec,&m_src->fvec) == 0.0f) {
		Error( LOCATION, "forward vec should not be zero-length" );
	}

	umag = vm_vec_mag(&m_src->uvec);
	rmag = vm_vec_mag(&m_src->rvec);
	if (umag <= 0.0f) {  // no up vector to use..
		if (rmag <= 0.0f) {  // no right vector either, so make something up
			if (!m->fvec.x && !m->fvec.z && m->fvec.y)  // vertical vector
				vm_vec_make(&m->uvec, 0.0f, 0.0f, 1.0f);
			else
				vm_vec_make(&m->uvec, 0.0f, 1.0f, 0.0f);

		} else {  // use the right vector to figure up vector
			vm_vec_crossprod(&m->uvec, &m->fvec, &m_src->rvec);
			if (vm_vec_normalize(&m->uvec) == 0.0f)
				Error( LOCATION, "Bad vector!" );
		}

	} else {  // use source up vector
		vm_vec_copy_normalize(&m->uvec, &m_src->uvec);
	}

	// use forward and up vectors as good vectors to calculate right vector
	vm_vec_crossprod(&m->rvec, &m->uvec, &m->fvec);
		
	//normalize new perpendicular vector
	if (vm_vec_normalize(&m->rvec) == 0.0f)
		Error( LOCATION, "Bad vector!" );

	//now recompute up vector, in case it wasn't entirely perpendiclar
	vm_vec_crossprod(&m->uvec, &m->fvec, &m->rvec);
	*m_src = tempm;
}

// like vm_orthogonalize_matrix(), except that zero vectors can exist within the
// matrix without causing problems.  Valid vectors will be created where needed.
void vm_fix_matrix(matrix *m)
{
	float fmag, umag, rmag;

	fmag = vm_vec_mag(&m->fvec);
	umag = vm_vec_mag(&m->uvec);
	rmag = vm_vec_mag(&m->rvec);
	if (fmag <= 0.0f) {
		if ((umag > 0.0f) && (rmag > 0.0f) && !vm_test_parallel(&m->uvec, &m->rvec)) {
			vm_vec_crossprod(&m->fvec, &m->uvec, &m->rvec);
			vm_vec_normalize(&m->fvec);

		} else if (umag > 0.0f) {
			if (!m->uvec.x && !m->uvec.y && m->uvec.z)  // z vector
				vm_vec_make(&m->fvec, 1.0f, 0.0f, 0.0f);
			else
				vm_vec_make(&m->fvec, 0.0f, 0.0f, 1.0f);
		}

	} else
		vm_vec_normalize(&m->fvec);

	// we now have a valid and normalized forward vector

	if ((umag <= 0.0f) || vm_test_parallel(&m->fvec, &m->uvec)) {  // no up vector to use..
		if ((rmag <= 0.0f) || vm_test_parallel(&m->fvec, &m->rvec)) {  // no right vector either, so make something up
			if (!m->fvec.x && m->fvec.y && !m->fvec.z)  // vertical vector
				vm_vec_make(&m->uvec, 0.0f, 0.0f, -1.0f);
			else
				vm_vec_make(&m->uvec, 0.0f, 1.0f, 0.0f);

		} else {  // use the right vector to figure up vector
			vm_vec_crossprod(&m->uvec, &m->fvec, &m->rvec);
			vm_vec_normalize(&m->uvec);
		}

	} else
		vm_vec_normalize(&m->uvec);

	// we now have both valid and normalized forward and up vectors

	vm_vec_crossprod(&m->rvec, &m->uvec, &m->fvec);
		
	//normalize new perpendicular vector
	vm_vec_normalize(&m->rvec);

	//now recompute up vector, in case it wasn't entirely perpendiclar
	vm_vec_crossprod(&m->uvec, &m->fvec, &m->rvec);
}

//Rotates the orient matrix by the angles in tangles and then
//makes sure that the matrix is orthogonal.
void vm_rotate_matrix_by_angles( matrix *orient, angles *tangles )
{
	matrix	rotmat,new_orient;
	vm_angles_2_matrix(&rotmat,tangles);
	vm_matrix_x_matrix(&new_orient,orient,&rotmat);
	*orient = new_orient;
	vm_orthogonalize_matrix(orient);
}

//	dir must be normalized!
float vm_vec_dot_to_point(vector *dir, vector *p1, vector *p2)
{
	vector	tvec;

	vm_vec_sub(&tvec, p2, p1);
	vm_vec_normalize(&tvec);

	return vm_vec_dot(dir, &tvec);

}

/////////////////////////////////////////////////////////
//	Given a plane and a point, return the point on the plane closest the the point.
//	Result returned in q.
void compute_point_on_plane(vector *q, plane *planep, vector *p)
{
	float	k, tv;
	vector	normal;

	normal.x = planep->A;
	normal.y = planep->B;
	normal.z = planep->C;

	k = (planep->D + vm_vec_dot(&normal, p)) / vm_vec_dot(&normal, &normal);

	vm_vec_scale_add(q, p, &normal, -k);

	tv = planep->A * q->x + planep->B * q->y + planep->C * q->z + planep->D;
}


//	Generate a fairly random vector that's fairly near normalized.
void vm_vec_rand_vec_quick(vector *rvec)
{
	rvec->x = (frand() - 0.5f) * 2;
	rvec->y = (frand() - 0.5f) * 2;
	rvec->z = (frand() - 0.5f) * 2;

	if (IS_VEC_NULL(rvec))
		rvec->x = 1.0f;

	vm_vec_normalize_quick(rvec);
}

// Given an point "in" rotate it by "angle" around an
// arbritary line defined by a point on the line "line_point" 
// and the normalized line direction, "line_dir"
// Returns the rotated point in "out".
void vm_rot_point_around_line(vector *out, vector *in, float angle, vector *line_point, vector *line_dir)
{
	vector tmp, tmp1;
	matrix m, r, im;
	angles ta;

	vm_vector_2_matrix_norm(&m, line_dir, NULL, NULL );
	vm_copy_transpose_matrix(&im,&m);

	ta.p = ta.h = 0.0f;
	ta.b = angle;
	vm_angles_2_matrix(&r,&ta);

	vm_vec_sub( &tmp, in, line_point );		// move relative to a point on line
	vm_vec_rotate( &tmp1, &tmp, &m);			// rotate into line's base
	vm_vec_rotate( &tmp, &tmp1, &r);			// rotate around Z
	vm_vec_rotate( &tmp1, &tmp, &im);		// unrotate out of line's base
	vm_vec_add( out, &tmp1, line_point );	// move back to world coordinates
}

// Given two position vectors, return 0 if the same, else non-zero.
int vm_vec_cmp( vector * a, vector * b )
{
	float diff = vm_vec_dist(a,b);
//mprintf(( "Diff=%.32f\n", diff ));
	if ( diff > 0.005f )
		return 1;
	else
		return 0;
}

// Given two orientation matrices, return 0 if the same, else non-zero.
int vm_matrix_cmp( matrix * a, matrix * b )
{
	float tmp1,tmp2,tmp3;
	tmp1 = (float)fl_abs(vm_vec_dot( &a->uvec, &b->uvec ) - 1.0f);
	tmp2 = (float)fl_abs(vm_vec_dot( &a->fvec, &b->fvec ) - 1.0f);
	tmp3 = (float)fl_abs(vm_vec_dot( &a->rvec, &b->rvec ) - 1.0f);
//	mprintf(( "Mat=%.16f, %.16f, %.16f\n", tmp1, tmp2, tmp3 ));
	 
	if ( tmp1 > 0.0000005f ) return 1;
	if ( tmp2 > 0.0000005f ) return 1;
	if ( tmp3 > 0.0000005f ) return 1;
	return 0;
}


// Moves angle 'h' towards 'desired_angle', taking the shortest
// route possible.   It will move a maximum of 'step_size' radians
// each call.   All angles in radians.
void vm_interp_angle( float *h, float desired_angle, float step_size )
{
	float delta;

	if ( desired_angle < 0.0f ) desired_angle += PI2;
	if ( desired_angle > PI2 ) desired_angle -= PI2;

	delta = desired_angle - *h;

	if ( fl_abs(delta) > PI )	{
		// Go the other way, since it will be shorter.
		if ( delta > 0.0f )	{
			delta = delta - PI2;
		} else {
			delta = PI2 - delta;
		}
	}

	if ( delta > step_size )
		*h += step_size;
	else if ( delta < -step_size )
		*h -= step_size;
	else
		*h = desired_angle;

	// If we wrap outside of 0 to 2*PI, then put the
	// angle back in the range 0 to 2*PI.
	if ( *h > PI2 ) *h -= PI2;
	if ( *h < 0.0f ) *h += PI2;
}

// check a matrix for zero rows and columns
int vm_check_matrix_for_zeros(matrix *m)
{
	if (!m->fvec.x && !m->fvec.y && !m->fvec.z)
		return 1;
	if (!m->rvec.x && !m->rvec.y && !m->rvec.z)
		return 1;
	if (!m->uvec.x && !m->uvec.y && !m->uvec.z)
		return 1;

	if (!m->fvec.x && !m->rvec.x && !m->uvec.x)
		return 1;
	if (!m->fvec.y && !m->rvec.y && !m->uvec.y)
		return 1;
	if (!m->fvec.z && !m->rvec.z && !m->uvec.z)
		return 1;

	return 0;
}

// see if two vectors are the same
int vm_vec_same(vector *v1, vector *v2)
{
	if ( v1->x == v2->x && v1->y == v2->y && v1->z == v2->z )
		return 1;

	return 0;
}


// --------------------------------------------------------------------------------------

void vm_quaternion_rotate(matrix *M, float theta, vector *u)
//  given an arbitrary rotation axis and rotation angle, function generates the
//  corresponding rotation matrix
//
//  M is the return rotation matrix  theta is the angle of rotation 
//  u is the direction of the axis.
//  this is adapted from Computer Graphics (Hearn and Bker 2nd ed.) p. 420
//
{

	float a,b,c, s;

	a = (float) (u->x * sin(theta * 0.5f));
	b = (float) (u->y * sin(theta * 0.5f));
	c = (float) (u->z * sin(theta * 0.5f));
	s = (float) cos(theta/2.0);

// 1st ROW vector
	M->rvec.x = 1.0f - 2.0f*b*b - 2.0f*c*c;
	M->rvec.y = 2.0f*a*b + 2.0f*s*c;
	M->rvec.z = 2.0f*a*c - 2.0f*s*b;
// 2nd ROW vector
	M->uvec.x = 2.0f*a*b - 2.0f*s*c;
	M->uvec.y = 1.0f - 2.0f*a*a - 2.0f*c*c;
	M->uvec.z = 2.0f*b*c + 2.0f*s*a;
// 3rd ROW vector
	M->fvec.x = 2.0f*a*c + 2.0f*s*b;
	M->fvec.y = 2.0f*b*c - 2.0f*s*a;
	M->fvec.z = 1.0f - 2.0f*a*a - 2.0f*b*b;
}

// --------------------------------------------------------------------------------------
// function finds the rotation matrix about the z axis for a given rotation angle (in radians)
// this is an optimized version vm_quaternion_rotate
//
//		inputs:	m			=>		point to resultant rotation matrix
//					angle		=>		rotation angle about z axis (in radians)
//
void rotate_z ( matrix *m, float theta )
{
	m->rvec.x = (float) cos (theta);
	m->rvec.y = (float) sin (theta);
	m->rvec.z = 0.0f;

	m->uvec.x = -m->rvec.y;
	m->uvec.y =  m->rvec.x;
	m->uvec.z = 0.0f;

	m->fvec.x = 0.0f;
	m->fvec.y = 0.0f;
	m->fvec.z = 1.0f;
}


// --------------------------------------------------------------------------------------

//void vm_matrix_to_rot_axis_and_angle(matrix *m, float *theta, vector *rot_axis)
// Converts a matrix into a rotation axis and an angle around that axis
// Note for angle is very near 0, returns 0 with axis of (1,0,0)
// For angles near PI, returns PI with correct axis
//
// rot_axis - the resultant axis of rotation
// theta - the resultatn rotation around the axis
// m - the initial matrix
void vm_matrix_to_rot_axis_and_angle(matrix *m, float *theta, vector *rot_axis)
{
	float trace = m->a2d[0][0] + m->a2d[1][1] + m->a2d[2][2];
	float cos_theta = 0.5f * (trace - 1.0f);

	if (cos_theta > 0.999999875f) { // angle is less than 1 milirad (0.057 degrees)
		*theta = 0.0f;

		vm_vec_make(rot_axis, 1.0f, 0.0f, 0.0f);
	} else if (cos_theta > -0.999999875f) { // angle is within limits between 0 and PI
		*theta = float(acos(cos_theta));
		Assert(!_isnan(*theta));

		rot_axis->x = (m->uvec.z - m->fvec.y);
		rot_axis->y = (m->fvec.x - m->rvec.z);
		rot_axis->z = (m->rvec.y - m->uvec.x);
		vm_vec_normalize(rot_axis);
	} else { // angle is PI within limits
		*theta = PI;

		// find index of largest diagonal term
		int largest_diagonal_index = 0;

		if (m->a2d[1][1] > m->a2d[0][0]) {
			largest_diagonal_index = 1;
		}
		if (m->a2d[2][2] > m->a2d[largest_diagonal_index][largest_diagonal_index]) {
			largest_diagonal_index = 2;
		}

		switch (largest_diagonal_index) {
		case 0:
			float ix;
			ix = 1.0f / rot_axis->x;

			rot_axis->x = fl_sqrt(m->a2d[0][0] + 1.0f);
			rot_axis->y = m->a2d[0][1] * ix;
			rot_axis->z = m->a2d[0][2] * ix;
			vm_vec_normalize(rot_axis);
			break;

		case 1:
			float iy;
			iy = 1.0f / rot_axis->y;

			rot_axis->y = fl_sqrt(m->a2d[1][1] + 1.0f);
			rot_axis->x = m->a2d[1][0] * iy;
			rot_axis->z = m->a2d[1][2] * iy;
			vm_vec_normalize(rot_axis);
			break;

		case 2:
			float iz;
			iz = 1.0f / rot_axis->z;

			rot_axis->z = fl_sqrt(m->a2d[2][2] + 1.0f);
			rot_axis->x = m->a2d[2][0] * iz;
			rot_axis->y = m->a2d[2][1] * iz;
			break;

		default:
			Int3();  // this should never happen
			break;
		}

		// normalize rotation axis
		vm_vec_normalize(rot_axis);
	}
}


// --------------------------------------------------------------------------------------
// This routine determines the resultant angular displacement and angular velocity in trying to reach a goal
// given an angular velocity APPROACHing a goal.  It uses maximal acceleration to a point (called peak), then maximal
// deceleration to arrive at the goal with zero angular velocity.  This can occasionally cause overshoot.  
// w_in			> 0
// w_max			> 0
// theta_goal	> 0
// aa				> 0 
// returns delta_theta
float away(float w_in, float w_max, float theta_goal, float aa, float delta_t, float *w_out, int no_overshoot);
float approach(float w_in, float w_max, float theta_goal, float aa, float delta_t, float *w_out, int no_overshoot)
{
	float delta_theta;		// amount rotated during time delta_t
	Assert(w_in >= 0);
	Assert(theta_goal > 0);
	float effective_aa;

	if (aa == 0) {
		*w_out = w_in;
		delta_theta = w_in*delta_t;
		return delta_theta;
	}

	if (no_overshoot && (w_in*w_in > 2.0f*1.05f*aa*theta_goal)) {
		w_in = fl_sqrt(2.0f*aa*theta_goal);
	}

	if (w_in*w_in > 2.0f*1.05f*aa*theta_goal) {		// overshoot condition
		effective_aa = 1.05f*aa;
		delta_theta = w_in*delta_t - 0.5f*effective_aa*delta_t*delta_t;

		if (delta_theta > theta_goal) {	// pass goal during this frame
			float t_goal = (-w_in + fl_sqrt(w_in*w_in +2.0f*effective_aa*theta_goal)) / effective_aa;
			// get time to theta_goal and away
			Assert(t_goal < delta_t);
			w_in -= effective_aa*t_goal;
			delta_theta = w_in*t_goal + 0.5f*effective_aa*t_goal*t_goal;
			delta_theta -= away(-w_in, w_max, 0.0f, aa, delta_t - t_goal, w_out, no_overshoot);
			*w_out = -*w_out;
			return delta_theta;
		} else {	
			if (delta_theta < 0) {
				// pass goal and return this frame
				*w_out = 0.0f;
				return theta_goal;
			} else {
				// do not pass goal this frame
				*w_out = w_in - effective_aa*delta_t;
				return delta_theta;
			}
		}
	} else if (w_in*w_in < 2.0f*0.95f*aa*theta_goal) {	// undershoot condition
		// find peak angular velocity
		float wp_sqr = fl_abs(aa*theta_goal + 0.5f*w_in*w_in);
		Assert(wp_sqr >= 0);

		if (wp_sqr > w_max*w_max) {
			float time_to_w_max = (w_max - w_in) / aa;
			if (time_to_w_max < 0) {
				// speed already too high
				// TODO: consider possible ramp down to below w_max
				*w_out = w_in - aa*delta_t;
				if (*w_out < 0) {
					*w_out = 0.0f;
				}

				delta_theta = 0.5f*(w_in + *w_out)*delta_t;
				return delta_theta;
			} else if (time_to_w_max > delta_t) {
				// does not reach w_max this frame
				*w_out = w_in + aa*delta_t;
				delta_theta = 0.5f*(w_in + *w_out)*delta_t;
				return delta_theta;
			} else {
				// reaches w_max this frame
				// TODO: consider when to ramp down from w_max
				*w_out = w_max;
				delta_theta = 0.5f*(w_in + *w_out)*delta_t;
				return delta_theta;
			}
		} else {	// wp < w_max
			if (wp_sqr > (w_in + aa*delta_t)*(w_in + aa*delta_t)) {
				// does not reach wp this frame
				*w_out = w_in + aa*delta_t;
				delta_theta = 0.5f*(w_in + *w_out)*delta_t;
				return delta_theta;
			} else {
				// reaches wp this frame
				float wp = fl_sqrt(wp_sqr);
				float time_to_wp = (wp - w_in) / aa;
				Assert(time_to_wp > 0);

				// accel
				*w_out = wp;
				delta_theta = 0.5f*(w_in + *w_out)*time_to_wp;

				// decel
				float time_remaining = delta_t - time_to_wp;
				*w_out -= aa*time_remaining;
				if (*w_out < 0) { // reached goal
					*w_out = 0.0f;
					delta_theta = theta_goal;
					return delta_theta;
				}
				delta_theta += 0.5f*(wp + *w_out)*time_remaining;
				return delta_theta;
			}
		}
	} else {														// on target
		// reach goal this frame
		if (w_in - aa*delta_t < 0) {
			// reach goal this frame
			*w_out = 0.0f;
			return theta_goal;
		} else {
			// move toward goal
			*w_out = w_in - aa*delta_t;
			Assert(*w_out >= 0);
			delta_theta = 0.5f*(w_in + *w_out)*delta_t;
			return delta_theta;
		}
	}
}


// --------------------------------------------------------------------------------------

// This routine determines the resultant angular displacement and angular velocity in trying to reach a goal
// given an angular velocity AWAY from a goal.  It uses maximal acceleration to a point (called peak), then maximal
// deceleration to arrive at the goal with zero angular acceleration.  
// w_in			< 0
// w_max			> 0
// theta_goal	> 0
// aa				> 0 
// returns angle rotated this frame
float away(float w_in, float w_max, float theta_goal, float aa, float delta_t, float *w_out, int no_overshoot)

{
	float delta_theta;// amount rotated during time
	float t0;			// time to velocity is 0
	float t_excess;	// time remaining in interval after velocity is 0

	Assert(theta_goal >=0);
	Assert(w_in <= 0);

	if ((-w_in < 1e-5) && (theta_goal < 1e-5)) {
		*w_out = 0.0f;
		return theta_goal;
	}

	if (aa == 0) {
		*w_out = w_in;
		delta_theta = w_in*delta_t;
		return delta_theta;
	}

	t0 = -w_in / aa;

	if (t0 > delta_t)	{	// no reversal in this time interval
		*w_out = w_in + aa * delta_t;
		delta_theta =  (w_in + *w_out) / 2.0f * delta_t;
		return delta_theta;
	}

	// use time remaining after v = 0
	delta_theta = 0.5f*w_in*t0;
	theta_goal -= delta_theta;		// delta_theta is *negative*
	t_excess = delta_t - t0;
	delta_theta += approach(0.0f, w_max, theta_goal, aa, t_excess, w_out, no_overshoot);
	return delta_theta;
}

// --------------------------------------------------------------------------------------

void vm_matrix_interpolate(matrix *goal_orient, matrix *curr_orient, vector *w_in, float delta_t, 
								matrix *next_orient, vector *w_out, vector *vel_limit, vector *acc_limit, int no_overshoot)
{
	matrix rot_matrix;		// rotation matrix from curr_orient to goal_orient
	matrix Mtemp1;				// temp matrix
	vector rot_axis;			// vector indicating direction of rotation axis
	vector theta_goal;		// desired angular position at the end of the time interval
	vector theta_end;			// actual angular position at the end of the time interval
	float theta;				// magnitude of rotation about the rotation axis

	//	FIND ROTATION NEEDED FOR GOAL
	// goal_orient = R curr_orient,  so R = goal_orient curr_orient^-1
	vm_copy_transpose_matrix(&Mtemp1, curr_orient);				// Mtemp1 = curr ^-1
	vm_matrix_x_matrix(&rot_matrix, &Mtemp1, goal_orient);	// R = goal * Mtemp1
	vm_orthogonalize_matrix(&rot_matrix);
	vm_matrix_to_rot_axis_and_angle(&rot_matrix, &theta, &rot_axis);		// determines angle and rotation axis from curr to goal

	// find theta to goal
	vm_vec_copy_scale(&theta_goal, &rot_axis, theta);

	if (theta < SMALL_NUM) {
		*next_orient = *goal_orient;
		vm_vec_zero(w_out);
		return;
	}

	theta_end = vmd_zero_vector;
	float delta_theta;

	// find rotation about x
	if (theta_goal.x > 0) {
		if (w_in->x >= 0) {
			delta_theta = approach(w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = delta_theta;
		} else { // w_in->x < 0
			delta_theta = away(w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = delta_theta;
		}
	} else if (theta_goal.x < 0) {
		if (w_in->x <= 0) {
			delta_theta = approach(-w_in->x, vel_limit->x, -theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = -delta_theta;
			w_out->x = -w_out->x;
		} else { // w_in->x > 0
			delta_theta = away(-w_in->x, vel_limit->x, -theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = -delta_theta;
			w_out->x = -w_out->x;
		}
	} else { // theta_goal == 0
		if (w_in->x < 0) {
			delta_theta = away(w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = delta_theta;
		} else {
			delta_theta = away(-w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = -delta_theta;
			w_out->x = -w_out->x;
		}
	}


	// find rotation about y
	if (theta_goal.y > 0) {
		if (w_in->y >= 0) {
			delta_theta = approach(w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = delta_theta;
		} else { // w_in->y < 0
			delta_theta = away(w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = delta_theta;
		}
	} else if (theta_goal.y < 0) {
		if (w_in->y <= 0) {
			delta_theta = approach(-w_in->y, vel_limit->y, -theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = -delta_theta;
			w_out->y = -w_out->y;
		} else { // w_in->y > 0
			delta_theta = away(-w_in->y, vel_limit->y, -theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = -delta_theta;
			w_out->y = -w_out->y;
		}
	} else { // theta_goal == 0
		if (w_in->y < 0) {
			delta_theta = away(w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = delta_theta;
		} else {
			delta_theta = away(-w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = -delta_theta;
			w_out->y = -w_out->y;
		}
	}

	// find rotation about z
	if (theta_goal.z > 0) {
		if (w_in->z >= 0) {
			delta_theta = approach(w_in->z, vel_limit->z, theta_goal.z, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			theta_end.z = delta_theta;
		} else { // w_in->z < 0
			delta_theta = away(w_in->z, vel_limit->z, theta_goal.z, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			theta_end.z = delta_theta;
		}
	} else if (theta_goal.z < 0) {
		if (w_in->z <= 0) {
			delta_theta = approach(-w_in->z, vel_limit->z, -theta_goal.z, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			theta_end.z = -delta_theta;
			w_out->z = -w_out->z;
		} else { // w_in->z > 0
			delta_theta = away(-w_in->z, vel_limit->z, -theta_goal.z, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			theta_end.z = -delta_theta;
			w_out->z = -w_out->z;
		}
	} else { // theta_goal == 0
		if (w_in->z < 0) {
			delta_theta = away(w_in->z, vel_limit->z, theta_goal.z, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			theta_end.z = delta_theta;
		} else {
			delta_theta = away(-w_in->z, vel_limit->z, theta_goal.z, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			theta_end.z = -delta_theta;
			w_out->z = -w_out->z;
		}
	}

	// the amount of rotation about each axis is determined in 
	// functions approach and away.  first find the magnitude		
	// of the rotation and then normalize the axis
	rot_axis = theta_end;
	Assert(is_valid_vec(&rot_axis));
	Assert(vm_vec_mag(&rot_axis) > 0);

	//	normalize rotation axis and determine total rotation angle
	theta = vm_vec_normalize(&rot_axis);

	// arrived at goal?
	if (theta_end.x == theta_goal.x && theta_end.y == theta_goal.y && theta_end.z == theta_goal.z) {
		*next_orient = *goal_orient;
	} else {
	// otherwise rotate to better position
		vm_quaternion_rotate(&Mtemp1, theta, &rot_axis);
		Assert(is_valid_matrix(&Mtemp1));
		vm_matrix_x_matrix(next_orient, curr_orient, &Mtemp1);
		vm_orthogonalize_matrix(next_orient);
	}
}	// end matrix_interpolate


// --------------------------------------------------------------------------------------


void get_camera_limits(matrix *start_camera, matrix *end_camera, float time, vector *acc_max, vector *w_max)
{
	matrix temp, rot_matrix;
	float theta;
	vector rot_axis;
	vector angle;

	// determine the necessary rotation matrix
	vm_copy_transpose(&temp, start_camera);
	vm_matrix_x_matrix(&rot_matrix, &temp, end_camera);
	vm_orthogonalize_matrix(&rot_matrix);

	// determine the rotation axis and angle
	vm_matrix_to_rot_axis_and_angle(&rot_matrix, &theta, &rot_axis);

	// find the rotation about each axis
	angle.x = theta * rot_axis.x;
	angle.y = theta * rot_axis.y;
	angle.z = theta * rot_axis.z;

	// allow for 0 time input
	if (time <= 1e-5f) {
		vm_vec_make(acc_max, 0.0f, 0.0f, 0.0f);
		vm_vec_make(w_max, 0.0f, 0.0f, 0.0f);
	} else {

		// find acceleration limit using  (theta/2) takes (time/2)
		// and using const accel  theta = 1/2 acc * time^2
		acc_max->x = 4.0f * (float)fl_abs(angle.x) / (time * time);
		acc_max->y = 4.0f * (float)fl_abs(angle.y) / (time * time);
		acc_max->z = 4.0f * (float)fl_abs(angle.z) / (time * time);

		// find angular velocity limits
		// w_max = acc_max * time / 2
		w_max->x = acc_max->x * time / 2.0f;
		w_max->y = acc_max->y * time / 2.0f;
		w_max->z = acc_max->z * time / 2.0f;
	}
}

// ---------------------------------------------------------------------------------------------
//
//		inputs:		goal_orient	=>		goal orientation matrix
//						orient		=>		current orientation matrix (with current forward vector)
//						w_in			=>		current input angular velocity
//						delta_t		=>		time to move toward goal
//						next_orient	=>		the orientation matrix at time delta_t (with current forward vector)
//												NOTE: this does not include any rotation about z (bank)
//						w_out			=>		the angular velocity of the ship at delta_t
//						vel_limit	=>		maximum rotational speed
//						acc_limit	=>		maximum rotational speed
//
//		function moves the forward vector toward the goal forward vector taking account of anglular
//		momentum (velocity)  Attempt to try to move bank by goal delta_bank.  Rotational velocity
//		on x/y is rotated with bank, giving smoother motion.
void vm_fvec_matrix_interpolate(matrix *goal_orient, matrix *orient, vector *w_in, float delta_t, matrix *next_orient, 
		vector *w_out, vector *vel_limit, vector *acc_limit, int no_overshoot)
{
	matrix	Mtemp1;				// temporary matrix
	matrix	M_intermed;			// intermediate matrix after xy rotation
	vector	local_rot_axis;	// vector indicating direction of rotation axis (local coords)
	vector	rot_axis;			// vector indicating direction of rotation axis (world coords)
	vector	theta_goal;			// desired angular position at the end of the time interval
	vector	theta_end;			// actual angular position at the end of the time interval
	float		theta;				// magnitude of rotation about the rotation axis
	float		bank;					// magnitude of rotation about the forward axis
	int		no_bank;				// flag set if there is no bank for the object
	vector	vtemp;				// temp angular velocity before rotation about z
	float		z_dotprod;			// dotprod of orient->fvec and goal_orient->fvec
	float		r_dotprod;			// dotprod of orient->rvec and goal_orient->rvec
	float		delta_bank;

	//	FIND XY ROTATION NEEDED FOR GOAL
	// rotation vector is (current fvec)  orient->fvec x goal_f
	// magnitude = asin ( magnitude of crossprod )
	vm_vec_crossprod ( &rot_axis, &orient->fvec, &goal_orient->fvec );

	float t = vm_vec_mag(&rot_axis);
	if (t > 1.0f)
		t = 1.0f;

	z_dotprod = vm_vec_dotprod ( &orient->fvec, &goal_orient->fvec );

	if ( t < SMALLER_NUM )  {
		if ( z_dotprod > 0.0f )
			theta = 0.0f;
		else  {  // the forward vector is pointing exactly opposite of goal
					// arbitrarily choose the x axis to rotate around until t becomes large enough
			theta = PI;
			rot_axis = orient->rvec;
		}
	} else {
		theta = (float) asin ( t );
		vm_vec_scale ( &rot_axis, 1/t );
		if ( z_dotprod < 0.0f )
			theta = PI - theta;
	}

	// rotate rot_axis into ship reference frame
	vm_vec_rotate ( &local_rot_axis, &rot_axis, orient );

	// find theta to goal
	vm_vec_copy_scale(&theta_goal, &local_rot_axis, theta);
	Assert ( fl_abs (theta_goal.z) < 0.001f );		// check for proper rotation

	theta_end = vmd_zero_vector;
	float delta_theta;

	// find rotation about x
	if (theta_goal.x > 0) {
		if (w_in->x >= 0) {
			delta_theta = approach(w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = delta_theta;
		} else { // w_in->x < 0
			delta_theta = away(w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = delta_theta;
		}
	} else if (theta_goal.x < 0) {
		if (w_in->x <= 0) {
			delta_theta = approach(-w_in->x, vel_limit->x, -theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = -delta_theta;
			w_out->x = -w_out->x;
		} else { // w_in->x > 0
			delta_theta = away(-w_in->x, vel_limit->x, -theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = -delta_theta;
			w_out->x = -w_out->x;
		}
	} else { // theta_goal == 0
		if (w_in->x < 0) {
			delta_theta = away(w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = delta_theta;
		} else {
			delta_theta = away(-w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = -delta_theta;
			w_out->x = -w_out->x;
		}
	}

	// find rotation about y
	if (theta_goal.y > 0) {
		if (w_in->y >= 0) {
			delta_theta = approach(w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = delta_theta;
		} else { // w_in->y < 0
			delta_theta = away(w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = delta_theta;
		}
	} else if (theta_goal.y < 0) {
		if (w_in->y <= 0) {
			delta_theta = approach(-w_in->y, vel_limit->y, -theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = -delta_theta;
			w_out->y = -w_out->y;
		} else { // w_in->y > 0
			delta_theta = away(-w_in->y, vel_limit->y, -theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = -delta_theta;
			w_out->y = -w_out->y;
		}
	} else { // theta_goal == 0
		if (w_in->y < 0) {
			delta_theta = away(w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = delta_theta;
		} else {
			delta_theta = away(-w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = -delta_theta;
			w_out->y = -w_out->y;
		}
	}

	// FIND Z ROTATON MATRIX
	theta_end.z = 0.0f;
	rot_axis = theta_end;
	Assert(is_valid_vec(&rot_axis));

	//	normalize rotation axis and determine total rotation angle
	theta = vm_vec_mag(&rot_axis);
	if (theta < SMALL_NUM)  {
		theta = 0.0f;
		M_intermed = *orient;
	} else {
		vm_vec_scale ( &rot_axis, 1/theta );		
		vm_quaternion_rotate ( &Mtemp1, theta, &rot_axis );
		Assert(is_valid_matrix(&Mtemp1));
		vm_matrix_x_matrix ( &M_intermed, orient, &Mtemp1 );
		Assert(is_valid_matrix(&M_intermed));
	}


	// FIND ROTATION ABOUT Z (IF ANY)
	// no rotation if delta_bank and w_in both 0 or rotational acc in forward is 0
	no_bank = ( acc_limit->z == 0.0f && vel_limit->z == 0.0f );

	if ( no_bank )  {	// no rotation on z, so we're done (no rotation of w)
		*next_orient = M_intermed;
		vm_orthogonalize_matrix ( next_orient );
		return;
	} else {
	// calculate delta_bank using orient->rvec, goal_orient->rvec
	//
		vm_vec_crossprod ( &rot_axis, &orient->rvec, &goal_orient->rvec );

		t = vm_vec_mag(&rot_axis);
		if (t > 1.0f)
			t = 1.0f;

		r_dotprod = vm_vec_dotprod ( &orient->rvec, &goal_orient->rvec );

		if ( t < SMALLER_NUM )  {
			if ( r_dotprod > 0.0f )
				theta = 0.0f;
			else  {  // the right vector is pointing exactly opposite of goal, so rotate 180 on z
				theta = PI;
				rot_axis = orient->fvec;
			}
		} else {
			theta = (float) asin ( t );
			vm_vec_scale ( &rot_axis, 1/t );
			if ( z_dotprod < 0.0f )
				theta = PI - theta;
		}

		// rotate rot_axis into ship reference frame
		vm_vec_rotate ( &local_rot_axis, &rot_axis, orient );

		// find theta.z to goal
		delta_bank = local_rot_axis.z * theta;
		Assert( fl_abs (local_rot_axis.x) < 0.001f );		// check for proper rotation
		bank = 0.0f;

	// end calculate delta_bank
	// find rotation about z
	if (delta_bank > 0) {
		if (w_in->z >= 0) {
			delta_theta = approach(w_in->z, vel_limit->z, delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			bank = delta_theta;
		} else { // w_in->z < 0
			delta_theta = away(w_in->z, vel_limit->z, delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			bank = delta_theta;
		}
	} else if (delta_bank < 0) {
		if (w_in->z <= 0) {
			delta_theta = approach(-w_in->z, vel_limit->z, -delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			bank = -delta_theta;
			w_out->z = -w_out->z;
		} else { // w_in->z > 0
			delta_theta = away(-w_in->z, vel_limit->z, -delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			bank = -delta_theta;
			w_out->z = -w_out->z;
		}
	} else { // theta_goal == 0
		if (w_in->z < 0) {
			delta_theta = away(w_in->z, vel_limit->z, delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			bank = delta_theta;
		} else {
			delta_theta = away(-w_in->z, vel_limit->z, delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
			bank = -delta_theta;
			w_out->z = -w_out->z;
		}
	}

		if ( fl_abs (bank) < SMALL_NUM )
		{
			*next_orient = M_intermed;
			vm_orthogonalize_matrix ( next_orient );
		} else {

		rotate_z ( &Mtemp1, bank );
		vtemp = *w_out;
		vm_vec_rotate ( w_out, &vtemp, &Mtemp1 );
		vm_matrix_x_matrix ( next_orient, &M_intermed, &Mtemp1 );
		Assert(is_valid_matrix(next_orient));
		vm_orthogonalize_matrix ( next_orient );
		}
	}
}	// end vm_fvec_matrix_interpolate


// ---------------------------------------------------------------------------------------------
//
//		inputs:		goal_f		=>		goal forward vector
//						orient		=>		current orientation matrix (with current forward vector)
//						w_in			=>		current input angular velocity
//						delta_t		=>		time to move toward goal
//						delta_bank	=>		desired change in bank in degrees
//						next_orient	=>		the orientation matrix at time delta_t (with current forward vector)
//												NOTE: this does not include any rotation about z (bank)
//						w_out			=>		the angular velocity of the ship at delta_t
//						vel_limit	=>		maximum rotational speed
//						acc_limit	=>		maximum rotational speed
//
//		function moves the forward vector toward the goal forward vector taking account of anglular
//		momentum (velocity)  Attempt to try to move bank by goal delta_bank.  Rotational velocity
//		on x/y is rotated with bank, giving smoother motion.
void vm_forward_interpolate(vector *goal_f, matrix *orient, vector *w_in, float delta_t, float delta_bank,
		matrix *next_orient, vector *w_out, vector *vel_limit, vector *acc_limit, int no_overshoot)
{
	matrix Mtemp1;				// temporary matrix
	vector local_rot_axis;	// vector indicating direction of rotation axis (local coords)
	vector rot_axis;			// vector indicating direction of rotation axis (world coords)
	vector theta_goal;		// desired angular position at the end of the time interval
	vector theta_end;			// actual angular position at the end of the time interval
	float theta;				// magnitude of rotation about the rotation axis
	float bank;					// magnitude of rotation about the forward axis
	int no_bank;				// flag set if there is no bank for the object
	vector vtemp;				// 
	float z_dotprod;

	// FIND ROTATION NEEDED FOR GOAL
	// rotation vector is (current fvec)  orient->fvec x goal_f
	// magnitude = asin ( magnitude of crossprod )
	vm_vec_crossprod( &rot_axis, &orient->fvec, goal_f );

	float t = vm_vec_mag(&rot_axis);
	if (t > 1.0f)
		t = 1.0f;

	z_dotprod = vm_vec_dotprod( &orient->fvec, goal_f );

	if ( t < SMALLER_NUM )  {
		if ( z_dotprod > 0.0f )
			theta = 0.0f;
		else  {  // the forward vector is pointing exactly opposite of goal
					// arbitrarily choose the x axis to rotate around until t becomes large enough
			theta = PI;
			rot_axis = orient->rvec;
		}
	} else {
		theta = (float) asin( t );
		vm_vec_scale ( &rot_axis, 1/t );
		if ( z_dotprod < 0.0f )
			theta = PI - theta;
	}

	// rotate rot_axis into ship reference frame
	vm_vec_rotate( &local_rot_axis, &rot_axis, orient );

	// find theta to goal
	vm_vec_copy_scale(&theta_goal, &local_rot_axis, theta);
	Assert(fl_abs(theta_goal.z) < 0.001f);		// check for proper rotation

	theta_end = vmd_zero_vector;
	float delta_theta;

	// find rotation about x
	if (theta_goal.x > 0) {
		if (w_in->x >= 0) {
			delta_theta = approach(w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = delta_theta;
		} else { // w_in->x < 0
			delta_theta = away(w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = delta_theta;
		}
	} else if (theta_goal.x < 0) {
		if (w_in->x <= 0) {
			delta_theta = approach(-w_in->x, vel_limit->x, -theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = -delta_theta;
			w_out->x = -w_out->x;
		} else { // w_in->x > 0
			delta_theta = away(-w_in->x, vel_limit->x, -theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = -delta_theta;
			w_out->x = -w_out->x;
		}
	} else { // theta_goal == 0
		if (w_in->x < 0) {
			delta_theta = away(w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = delta_theta;
		} else {
			delta_theta = away(-w_in->x, vel_limit->x, theta_goal.x, acc_limit->x, delta_t, &w_out->x, no_overshoot);
			theta_end.x = -delta_theta;
			w_out->x = -w_out->x;
		}
	}

	// find rotation about y
	if (theta_goal.y > 0) {
		if (w_in->y >= 0) {
			delta_theta = approach(w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = delta_theta;
		} else { // w_in->y < 0
			delta_theta = away(w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = delta_theta;
		}
	} else if (theta_goal.y < 0) {
		if (w_in->y <= 0) {
			delta_theta = approach(-w_in->y, vel_limit->y, -theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = -delta_theta;
			w_out->y = -w_out->y;
		} else { // w_in->y > 0
			delta_theta = away(-w_in->y, vel_limit->y, -theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = -delta_theta;
			w_out->y = -w_out->y;
		}
	} else { // theta_goal == 0
		if (w_in->y < 0) {
			delta_theta = away(w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = delta_theta;
		} else {
			delta_theta = away(-w_in->y, vel_limit->y, theta_goal.y, acc_limit->y, delta_t, &w_out->y, no_overshoot);
			theta_end.y = -delta_theta;
			w_out->y = -w_out->y;
		}
	}

	// no rotation if delta_bank and w_in both 0 or rotational acc in forward is 0
	no_bank = ( delta_bank == 0.0f && vel_limit->z == 0.0f && acc_limit->z == 0.0f );

	// do rotation about z
	bank = 0.0f;
	if ( !no_bank )  {
		// convert delta_bank to radians
		delta_bank *= (float) CONVERT_RADIANS;

		// find rotation about z
		if (delta_bank > 0) {
			if (w_in->z >= 0) {
				delta_theta = approach(w_in->z, vel_limit->z, delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
				bank = delta_theta;
			} else { // w_in->z < 0
				delta_theta = away(w_in->z, vel_limit->z, delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
				bank = delta_theta;
			}
		} else if (delta_bank < 0) {
			if (w_in->z <= 0) {
				delta_theta = approach(-w_in->z, vel_limit->z, -delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
				bank = -delta_theta;
				w_out->z = -w_out->z;
			} else { // w_in->z > 0
				delta_theta = away(-w_in->z, vel_limit->z, -delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
				bank = -delta_theta;
				w_out->z = -w_out->z;
			}
		} else { // theta_goal == 0
			if (w_in->z < 0) {
				delta_theta = away(w_in->z, vel_limit->z, delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
				bank = delta_theta;
			} else {
				delta_theta = away(-w_in->z, vel_limit->z, delta_bank, acc_limit->z, delta_t, &w_out->z, no_overshoot);
				bank = -delta_theta;
				w_out->z = -w_out->z;
			}
		}
	}

	// the amount of rotation about each axis is determined in 
	// functions approach and away.  first find the magnitude		
	// of the rotation and then normalize the axis  (ship coords)
	theta_end.z = bank;
	rot_axis = theta_end;

	//	normalize rotation axis and determine total rotation angle
	theta = vm_vec_mag(&rot_axis);
	if ( theta > SMALL_NUM )
		vm_vec_scale( &rot_axis, 1/theta );

	if ( theta < SMALL_NUM ) {
		*next_orient = *orient;
		return;
	} else {
		vm_quaternion_rotate( &Mtemp1, theta, &rot_axis );
		vm_matrix_x_matrix( next_orient, orient, &Mtemp1 );
		Assert(is_valid_matrix(next_orient));
		vtemp = *w_out;
		vm_vec_rotate( w_out, &vtemp, &Mtemp1 );
	}
}	// end vm_forward_interpolate

// ------------------------------------------------------------------------------------
// vm_find_bounding_sphere()
//
// Calculate a bounding sphere for a set of points.
//
//	input:	pnts			=>		array of world positions
//				num_pnts		=>		number of points inside pnts array
//				center		=>		OUTPUT PARAMETER:	contains world pos of bounding sphere center
//				radius		=>		OUTPUT PARAMETER:	continas radius of bounding sphere
//
#define BIGNUMBER	100000000.0f
void vm_find_bounding_sphere(vector *pnts, int num_pnts, vector *center, float *radius)
{
	int		i;
	float		rad, rad_sq, xspan, yspan, zspan, maxspan;
	float		old_to_p, old_to_p_sq, old_to_new;	
	vector	diff, xmin, xmax, ymin, ymax, zmin, zmax, dia1, dia2, *p;
	
	xmin = vmd_zero_vector;
	ymin = vmd_zero_vector;
	zmin = vmd_zero_vector;
	xmax = vmd_zero_vector;
	ymax = vmd_zero_vector;
	zmax = vmd_zero_vector;	
	xmin.x = ymin.y = zmin.z = BIGNUMBER;
	xmax.x = ymax.y = zmax.z = -BIGNUMBER;

	for ( i = 0; i < num_pnts; i++ ) {
		p = &pnts[i];
		if ( p->x < xmin.x )
			xmin = *p;
		if ( p->x > xmax.x )
			xmax = *p;
		if ( p->y < ymin.y )
			ymin = *p;
		if ( p->y > ymax.y )
			ymax = *p;
		if ( p->z < zmin.z )
			zmin = *p;
		if ( p->z > zmax.z )
			zmax = *p;
	}

	// find distance between two min and max points (squared)
	vm_vec_sub(&diff, &xmax, &xmin);
	xspan = vm_vec_mag_squared(&diff);

	vm_vec_sub(&diff, &ymax, &ymin);
	yspan = vm_vec_mag_squared(&diff);

	vm_vec_sub(&diff, &zmax, &zmin);
	zspan = vm_vec_mag_squared(&diff);

	dia1 = xmin;
	dia2 = xmax;
	maxspan = xspan;
	if ( yspan > maxspan ) {
		maxspan = yspan;
		dia1 = ymin;
		dia2 = ymax;
	}
	if ( zspan > maxspan ) {
		maxspan = yspan;
		dia1 = zmin;
		dia2 = zmax;
	}

	// calc inital center
	vm_vec_add(center, &dia1, &dia2);
	vm_vec_scale(center, 0.5f);

	vm_vec_sub(&diff, &dia2, center);
	rad_sq = vm_vec_mag_squared(&diff);
	rad = fl_sqrt(rad_sq);
	Assert( !_isnan(rad) );

	// second pass
	for ( i = 0; i < num_pnts; i++ ) {
		p = &pnts[i];
		vm_vec_sub(&diff, p, center);
		old_to_p_sq = vm_vec_mag_squared(&diff);
		if ( old_to_p_sq > rad_sq ) {
			old_to_p = fl_sqrt(old_to_p_sq);
			// calc radius of new sphere
			rad = (rad + old_to_p) / 2.0f;
			rad_sq = rad * rad;
			old_to_new = old_to_p - rad;
			// calc new center of sphere
			center->x = (rad*center->x + old_to_new*p->x) / old_to_p;
			center->y = (rad*center->y + old_to_new*p->y) / old_to_p;
			center->z = (rad*center->z + old_to_new*p->z) / old_to_p;
			nprintf(("Alan", "New sphere: cen,rad = %f %f %f  %f\n", center->x, center->y, center->z, rad));
		}
	}

	*radius = rad;
}

// ----------------------------------------------------------------------------
// vm_rotate_vec_to_body()
//
// rotates a vector from world coordinates to body coordinates
//
// inputs:	body_vec		=>		vector in body coordinates
//				world_vec	=>		vector in world coordinates
//				orient		=>		orientation matrix
//
vector* vm_rotate_vec_to_body(vector *body_vec, vector *world_vec, matrix *orient)
{
	return vm_vec_unrotate(body_vec, world_vec, orient);
}


// ----------------------------------------------------------------------------
// vm_rotate_vec_to_world()
//
// rotates a vector from body coordinates to world coordinates
//
//	inputs	world_vec	=>		vector in world coordinates
//				body_vec		=>		vector in body coordinates
//				orient		=>		orientation matrix
//
vector* vm_rotate_vec_to_world(vector *world_vec, vector *body_vec, matrix *orient)
{
	return vm_vec_rotate(world_vec, body_vec, orient);
}


// ----------------------------------------------------------------------------
// vm_estimate_next_orientation()
//
// given a last orientation and current orientation, estimate the next orientation
//
//	inputs:	last_orient		=>		last orientation matrix
//				current_orient	=>		current orientation matrix
//				next_orient		=>		next orientation matrix		[the result]
//
void vm_estimate_next_orientation(matrix *last_orient, matrix *current_orient, matrix *next_orient)
{
	//		R L = C		=>		R = C (L)^-1
	//		N = R C		=>		N = C (L)^-1 C

	matrix Mtemp;
	matrix Rot_matrix;
	vm_copy_transpose_matrix(&Mtemp, last_orient);				// Mtemp = (L)^-1
	vm_matrix_x_matrix(&Rot_matrix, &Mtemp, current_orient);	// R = C Mtemp1
	vm_matrix_x_matrix(next_orient, current_orient, &Rot_matrix);
}

//	Return true if all elements of *vec are legal, that is, not a NAN.
int is_valid_vec(vector *vec)
{
	return !_isnan(vec->x) && !_isnan(vec->y) && !_isnan(vec->z);
}

//	Return true if all elements of *m are legal, that is, not a NAN.
int is_valid_matrix(matrix *m)
{
	return is_valid_vec(&m->fvec) && is_valid_vec(&m->uvec) && is_valid_vec(&m->rvec);
}

// interpolate between 2 vectors. t goes from 0.0 to 1.0. at
void vm_vec_interp_constant(vector *out, vector *v0, vector *v1, float t)
{
	vector cross;
	float total_ang;

	// get the cross-product of the 2 vectors
	vm_vec_crossprod(&cross, v0, v1);
	vm_vec_normalize(&cross);

	// get the total angle between the 2 vectors
	total_ang = -(float)acos(vm_vec_dot(v0, v1));

	// rotate around the cross product vector by the appropriate angle
	vm_rot_point_around_line(out, v0, t * total_ang, &vmd_zero_vector, &cross);
}

// randomly perturb a vector around a given (normalized vector) or optional orientation matrix
void vm_vec_random_cone(vector *out, vector *in, float max_angle, matrix *orient)
{
	vector t1, t2;
	matrix *rot;
	matrix m;

	// get an orientation matrix
	if(orient != NULL){
		rot = orient;
	} else {
		vm_vector_2_matrix(&m, in, NULL, NULL);
		rot = &m;
	}
	
	// axis 1
	vm_rot_point_around_line(&t1, in, fl_radian(frand_range(-max_angle, max_angle)), &vmd_zero_vector, &rot->fvec);
	
	// axis 2
	vm_rot_point_around_line(&t2, &t1, fl_radian(frand_range(-max_angle, max_angle)), &vmd_zero_vector, &rot->rvec);

	// axis 3
	vm_rot_point_around_line(out, &t2, fl_radian(frand_range(-max_angle, max_angle)), &vmd_zero_vector, &rot->uvec);
}

// given a start vector, an orientation and a radius, give a point on the plane of the circle
// if on_edge is 1, the point is on the very edge of the circle
void vm_vec_random_in_circle(vector *out, vector *in, matrix *orient, float radius, int on_edge)
{
	vector temp;

	// point somewhere in the plane
	vm_vec_scale_add(&temp, in, &orient->rvec, on_edge ? radius : frand_range(0.0f, radius));

	// rotate to a random point on the circle
	vm_rot_point_around_line(out, &temp, fl_radian(frand_range(0.0f, 359.0f)), in, &orient->fvec);
}

// find the nearest point on the line to p. if dist is non-NULL, it is filled in
// returns 0 if the point is inside the line segment, -1 if "before" the line segment and 1 ir "after" the line segment
int vm_vec_dist_to_line(vector *p, vector *l0, vector *l1, vector *nearest, float *dist)
{
	vector a, b, c;
	float b_mag, comp;

#ifndef NDEBUG
	if(vm_vec_same(l0, l1)){
		*nearest = vmd_zero_vector;
		return -1;
	}
#endif

	// compb_a == a dot b / len(b)
	vm_vec_sub(&a, p, l0);
	vm_vec_sub(&b, l1, l0);		
	b_mag = vm_vec_copy_normalize(&c, &b);	

	// calculate component
	comp = vm_vec_dotprod(&a, &b) / b_mag;

	// stuff nearest
	vm_vec_scale_add(nearest, l0, &c, comp);

	// maybe get the distance
	if(dist != NULL){		
		*dist = vm_vec_dist(nearest, p);
	}

	// return the proper value
	if(comp < 0.0f){
		return -1;						// before the line
	} else if(comp > b_mag){
		return 1;						// after the line
	}
	return 0;							// on the line
}
