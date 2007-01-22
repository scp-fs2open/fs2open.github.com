/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Math/VecMat.h $
 * $Revision: 2.17 $
 * $Date: 2007-01-22 04:43:28 $
 * $Author: wmcoolmon $
 *
 * Header file for functions that manipulate vectors and matricies
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.16  2007/01/14 14:03:32  bobboau
 * ok, something aparently went wrong, last time, so I'm commiting again
 * hopefully it should work this time
 * damnit WORK!!!
 *
 * Revision 2.15  2006/04/12 22:23:41  taylor
 * compiler warning fixes to make GCC 4.1 shut the hell up
 *
 * Revision 2.14  2006/02/25 21:47:00  Goober5000
 * spelling
 *
 * Revision 2.13  2006/01/22 01:31:44  taylor
 * no real need to cast these so just get it right in the first place
 *
 * Revision 2.12  2005/07/13 03:15:50  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.11  2005/07/07 16:36:57  taylor
 * various compiler warning fixes (some of these from dizzy)
 *
 * Revision 2.10  2005/04/15 11:41:27  taylor
 * stupid <expletive-delete> terminal, I <expletive-deleted> <expletive-deleted>!!!
 *
 * Revision 2.9  2005/04/15 11:36:54  taylor
 * new GCC = new warning messages, yippeeee!!
 *
 * Revision 2.8  2005/04/12 05:26:36  taylor
 * many, many compiler warning and header fixes (Jens Granseuer)
 * fix free on possible NULL in modelinterp.cpp (Jens Granseuer)
 *
 * Revision 2.7  2005/04/05 05:53:19  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.6  2005/03/19 18:02:34  bobboau
 * added new graphic functions for state blocks
 * also added a class formanageing a new effect
 *
 * Revision 2.5  2005/01/06 00:37:32  Goober5000
 * changed argument from dist to dist_squared
 * --Goober5000
 *
 * Revision 2.4  2005/01/06 00:27:34  Goober5000
 * added vm_vec_dist_squared_to_line
 * --Goober5000
 *
 * Revision 2.3  2004/08/11 05:06:27  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2002/12/07 01:37:42  bobboau
 * initial decals code, if you are worried a bug is being caused by the decals code it's only references are in,
 * collideshipweapon.cpp line 262, beam.cpp line 2771, and modelinterp.cpp line 2949.
 * it needs a better renderer, but is in prety good shape for now,
 * I also (think) I squashed a bug in the warpmodel code
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:09  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 6     6/18/99 5:16p Dave
 * Added real beam weapon lighting. Fixed beam weapon sounds. Added MOTD
 * dialog to PXO screen.
 * 
 * 5     4/28/99 11:13p Dave
 * Temporary checkin of artillery code.
 * 
 * 4     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 3     1/12/99 12:53a Dave
 * More work on beam weapons - made collision detection very efficient -
 * collide against all object types properly - made 3 movement types
 * smooth. Put in test code to check for possible non-darkening pixels on
 * object textures.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 43    9/11/98 10:10a Andsager
 * Optimize and rename matrix_decomp to vm_matrix_to_rot_axis_and_angle,
 * rename quatern_rot to vm_quaternion_rotate
 * 
 * 42    3/09/98 3:51p Mike
 * More error checking.
 * 
 * 41    12/17/97 5:44p Andsager
 * Change vm_matrix_interpolate so that it does not overshoot if optional
 * last parameter is 1
 * 
 * 40    9/30/97 8:03p Lawrance
 * add missing semi-colon to function prototype
 * 
 * 39    9/30/97 5:04p Andsager
 * add vm_estimate_next_orientation
 * 
 * 38    9/28/97 2:17p Andsager
 * added vm_project_point_onto_plane
 * 
 * 37    9/25/97 5:57p Andsager
 * improved function description for matrix interpolate
 * 
 * 36    9/09/97 10:15p Andsager
 * added vm_rotate_vec_to_body() and vm_rotate_vec_to_world()
 * 
 * 35    8/20/97 5:33p Andsager
 * added vm_vec_projection_parallel and vm_vec_projection_onto_surface
 * 
 * 34    8/19/97 11:42p Lawrance
 * use atan2_safe() instead of atan2()
 * 
 * 33    8/18/97 4:46p Hoffoss
 * Added global default axis vector constants.
 * 
 * 32    8/03/97 3:54p Lawrance
 * added vm_find_bounding_sphere()
 * 
 * 31    7/30/97 2:20p Dan
 * from allender: fixed vm_is_vec_nan to work properly with address-of
 * operator by adding parens around macro variables
 * 
 * 30    7/29/97 2:48p Hoffoss
 * Added vm_is_vec_nan().
 * 
 * 29    7/28/97 2:21p John
 * changed vecmat functions to not return src.  Started putting in code
 * for inline vector math.    Fixed some bugs with optimizer.
 * 
 * 28    7/28/97 3:25p Andsager
 * 
 * 27    7/28/97 2:41p Mike
 * Replace vm_forward_interpolate().
 * 
 * 26    7/28/97 1:18p Andsager
 * implement vm_fvec_matrix_interpolate(), which interpolates matrices on
 * xy and then z
 * 
 * 25    7/24/97 5:24p Andsager
 * implement forward vector interpolation
 * 
 * 24    7/02/97 4:25p Mike
 * Add matrix_interpolate(), but don't call it.
 * 
 * 23    7/01/97 3:27p Mike
 * Improve skill level support.
 * 
 * 22    6/25/97 12:27p Hoffoss
 * Added some functions I needed for Fred.
 * 
 * 21    5/21/97 8:49a Lawrance
 * added vm_vec_same()
 * 
 * 20    4/15/97 4:00p Mike
 * Intermediate checkin caused by getting other files.  Working on camera
 * slewing system.
 * 
 * 19    3/17/97 1:55p Hoffoss
 * Added function for error checking matrices.
 * 
 * 18    3/06/97 10:56a Mike
 * Write error checking version of vm_vec_normalize().
 * Fix resultant problems.
 * 
 * 17    3/04/97 3:30p John
 * added function to interpolate an angle.
 * 
 * 16    2/25/97 5:12p John
 * Added functions to see if two matrices or vectors are close.
 * 
 * 15    2/03/97 1:30p John
 * Put a clearer comment in for vm_vec_unrotate
 * 
 * 14    2/03/97 1:14p John
 * Added vm_vec_unrotate function
 * 
 * 13    1/27/97 11:57a John
 * added a function to rotate a point around an arbritary line.
 * 
 * 12    11/26/96 12:18p Hoffoss
 * Added the vm_vec_dist_squared() function.
 * 
 * 11    11/16/96 2:38p Mike
 * Waypoint code, under construction and a painful mess.
 * 
 * 10    11/05/96 3:42p Mike
 * Make AI use accuracy parameter, though not yet specified in ships.tbl
 * or *.fsm.
 * 
 * Add vm_vec_rand_vec_quick.
 * 
 * Add frand() which returns a rand in 0.0..1.0.
 * 
 * 9     10/30/96 2:35p Mike
 * Docking behavior.
 * Changed quick versions of vecmat routines to not return 1/mag.  They
 * return mag, just like non-quick versions.
 * 
 * 8     10/24/96 10:17a Hoffoss
 * Moved function 'compute_point_on_plane()' to vecmat.
 * 
 * 7     10/23/96 10:32p Lawrance
 * added function vm_vect_mag_squared()
 *
 * $NoKeywords: $
 *
*/

#ifndef _VECMAT_H
#define _VECMAT_H

#include <float.h>
#include "globalincs/pstypes.h"

//#define _INLINE_VECMAT

#define vm_is_vec_nan(v) (_isnan((v)->xyz.x) || _isnan((v)->xyz.y) || _isnan((v)->xyz.z))

//Macros/functions to fill in fields of structures

//macro to check if vector is zero
#define IS_VEC_NULL(v) (((v)->xyz.x == 0.0f) && ((v)->xyz.y == 0.0f) && ((v)->xyz.z == 0.0f))

#define IS_MAT_NULL(v) (IS_VEC_NULL(&(v)->vec.fvec) && IS_VEC_NULL(&(v)->vec.uvec) && IS_VEC_NULL(&(v)->vec.rvec))

//macro to set a vector to zero.  we could do this with an in-line assembly
//macro, but it's probably better to let the compiler optimize it.
//Note: NO RETURN VALUE
#define vm_vec_zero(v) (v)->xyz.x=(v)->xyz.y=(v)->xyz.z=0.0f

/*
//macro set set a matrix to the identity. Note: NO RETURN VALUE
#define vm_set_identity(m) do {m->rvec.x = m->uvec.y = m->fvec.z = (float)1.0;	\
										m->rvec.y = m->rvec.z = \
										m->uvec.x = m->uvec.z = \
										m->fvec.x = m->fvec.y = (float)0.0;} while (0)
*/
extern void vm_set_identity(matrix *m);

#define vm_vec_make(v,_x,_y,_z) ((v)->xyz.x=(_x), (v)->xyz.y=(_y), (v)->xyz.z=(_z))

//Global constants

extern vec3d vmd_zero_vector;
extern vec3d vmd_x_vector;
extern vec3d vmd_y_vector;
extern vec3d vmd_z_vector;
extern matrix vmd_identity_matrix;

//Here's a handy constant

#define ZERO_VECTOR { { { 0.0f, 0.0f, 0.0f } } }
//#define IDENTITY_MATRIX {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f}
// first set of inside braces is for union, second set is for inside union, then for a2d[3][3] (some compiler warning messages just suck)
//#define IDENTITY_MATRIX { { { {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} } } }
#define IDENTITY_MATRIX { { { { { { 1.0f, 0.0f, 0.0f } } }, { { { 0.0f, 1.0f, 0.0f } } }, { { { 0.0f, 0.0f, 1.0f } } } } } }

//fills in fields of an angle vector
#define vm_angvec_make(v,_p,_b,_h) (((v)->p=(_p), (v)->b=(_b), (v)->h=(_h)), (v))

//negate a vector
#define vm_vec_negate(v) do {(v)->xyz.x = - (v)->xyz.x; (v)->xyz.y = - (v)->xyz.y; (v)->xyz.z = - (v)->xyz.z;} while (0);

typedef struct plane {
	float	A, B, C, D;
} plane;

//Functions in library

//adds two vectors, fills in dest, returns ptr to dest
//ok for dest to equal either source, but should use vm_vec_add2() if so
#ifdef _INLINE_VECMAT
#define vm_vec_add( dst, src0, src1 ) do {	\
	(dst)->xyz.x = (src0)->xyz.x + (src1)->xyz.x;					\
	(dst)->xyz.y = (src0)->xyz.y + (src1)->xyz.y;					\
	(dst)->xyz.z = (src0)->xyz.z + (src1)->xyz.z;					\
} while(0) 
#else
void vm_vec_add(vec3d *dest,vec3d *src0,vec3d *src1);
#endif

//adds src onto dest vector, returns ptr to dest
#ifdef _INLINE_VECMAT
#define vm_vec_add2( dst, src ) do {	\
	(dst)->xyz.x += (src)->xyz.x;					\
	(dst)->xyz.y += (src)->xyz.y;					\
	(dst)->xyz.z += (src)->xyz.z;					\
} while(0) 
#else
void vm_vec_add2(vec3d *dest,vec3d *src);
#endif


//scales a vector and subs from to another
//dest -= k * src
#ifdef _INLINE_VECMAT
#define vm_vec_scale_sub2( dst, src, k ) do {	\
	float tmp_k = (k);								\
	(dst)->xyz.x -= (src)->xyz.x*tmp_k;					\
	(dst)->xyz.y -= (src)->xyz.y*tmp_k;					\
	(dst)->xyz.z -= (src)->xyz.z*tmp_k;					\
} while(0) 
#else
void vm_vec_scale_sub2(vec3d *dest,vec3d *src, float k);
#endif

//subs two vectors, fills in dest, returns ptr to dest
//ok for dest to equal either source, but should use vm_vec_sub2() if so
#ifdef _INLINE_VECMAT
#define vm_vec_sub( dst, src0, src1 ) do {	\
	(dst)->xyz.x = (src0)->xyz.x - (src1)->xyz.x;					\
	(dst)->xyz.y = (src0)->xyz.y - (src1)->xyz.y;					\
	(dst)->xyz.z = (src0)->xyz.z - (src1)->xyz.z;					\
} while(0) 
#else
void vm_vec_sub(vec3d *dest,vec3d *src0,vec3d *src1);
#endif


//subs one vector from another, returns ptr to dest
//dest can equal source
#ifdef _INLINE_VECMAT
#define vm_vec_sub2( dst, src ) do {	\
	(dst)->xyz.x -= (src)->xyz.x;					\
	(dst)->xyz.y -= (src)->xyz.y;					\
	(dst)->xyz.z -= (src)->xyz.z;					\
} while(0) 
#else
void vm_vec_sub2(vec3d *dest,vec3d *src);
#endif

//averages n vectors
vec3d *vm_vec_avg_n(vec3d *dest, int n, vec3d src[]);


//averages two vectors. returns ptr to dest
//dest can equal either source
vec3d *vm_vec_avg(vec3d *dest,vec3d *src0,vec3d *src1);

vec3d *vm_vec_avg3(vec3d *dest,vec3d *src0,vec3d *src1,vec3d *src2);

//averages four vectors. returns ptr to dest
//dest can equal any source
vec3d *vm_vec_avg4(vec3d *dest,vec3d *src0,vec3d *src1,vec3d *src2,vec3d *src3);

//scales a vector in place.  returns ptr to vector
#ifdef _INLINE_VECMAT
#define vm_vec_scale( dst, k ) do {	\
	float tmp_k = (k);								\
	(dst)->xyz.x *= tmp_k;					\
	(dst)->xyz.y *= tmp_k;					\
	(dst)->xyz.z *= tmp_k;					\
} while(0) 
#else
void vm_vec_scale(vec3d *dest,float s);
#endif

//scales and copies a vector.  returns ptr to dest
#ifdef _INLINE_VECMAT
#define vm_vec_copy_scale( dst, src, k ) do {	\
	float tmp_k = (k);								\
	(dst)->xyz.x = (src)->xyz.x * tmp_k;					\
	(dst)->xyz.y = (src)->xyz.y * tmp_k;					\
	(dst)->xyz.z = (src)->xyz.z * tmp_k;					\
} while(0) 
#else
void vm_vec_copy_scale(vec3d *dest,vec3d *src,float s);
#endif

//scales a vector, adds it to another, and stores in a 3rd vector
//dest = src1 + k * src2
#ifdef _INLINE_VECMAT
#define vm_vec_scale_add( dst, src1, src2, k ) do {	\
	float tmp_k = (k);								\
	(dst)->xyz.x = (src1)->xyz.x + (src2)->xyz.x * tmp_k;					\
	(dst)->xyz.y = (src1)->xyz.y + (src2)->xyz.y * tmp_k;					\
	(dst)->xyz.z = (src1)->xyz.z + (src2)->xyz.z * tmp_k;					\
} while(0) 
#else
void vm_vec_scale_add(vec3d *dest,vec3d *src1,vec3d *src2,float k);
#endif


//scales a vector and adds it to another
//dest += k * src
#ifdef _INLINE_VECMAT
#define vm_vec_scale_add2( dst, src, k ) do {	\
	float tmp_k = (k);								\
	(dst)->xyz.x += (src)->xyz.x * tmp_k;					\
	(dst)->xyz.y += (src)->xyz.y * tmp_k;					\
	(dst)->xyz.z += (src)->xyz.z * tmp_k;					\
} while(0) 
#else
void vm_vec_scale_add2(vec3d *dest,vec3d *src,float k);
#endif

//scales a vector in place, taking n/d for scale.  returns ptr to vector
//dest *= n/d
#ifdef _INLINE_VECMAT
#define vm_vec_scale2( dst, n, d ) do {	\
	float tmp_k = (n)/(d);								\
	(dst)->xyz.x *= tmp_k;					\
	(dst)->xyz.y *= tmp_k;					\
	(dst)->xyz.z *= tmp_k;					\
} while(0) 
#else
void vm_vec_scale2(vec3d *dest,float n,float d);
#endif

// finds the projection of source vector along a unit vector
// returns the magnitude of the component
float vm_vec_projection_parallel (vec3d *component, vec3d *src, vec3d *unit_vector);

// finds the projection of source vector onto a surface given by surface normal
void vm_vec_projection_onto_plane (vec3d *projection, vec3d *src, vec3d *normal);

//returns magnitude of a vector
float vm_vec_mag(vec3d *v);

// returns the square of the magnitude of a vector (useful if comparing distances)
float vm_vec_mag_squared(vec3d* v);

// returns the square of the distance between two points (fast and exact)
float vm_vec_dist_squared(vec3d *v0, vec3d *v1);

//computes the distance between two points. (does sub and mag)
float vm_vec_dist(vec3d *v0,vec3d *v1);

//computes an approximation of the magnitude of the vector
//uses dist = largest + next_largest*3/8 + smallest*3/16
float vm_vec_mag_quick(vec3d *v);

//computes an approximation of the distance between two points.
//uses dist = largest + next_largest*3/8 + smallest*3/16
float vm_vec_dist_quick(vec3d *v0,vec3d *v1);


//normalize a vector. returns mag of source vec
float vm_vec_copy_normalize(vec3d *dest,vec3d *src);
float vm_vec_normalize(vec3d *v);

//	This version of vector normalize checks for the null vector before normalization.
//	If it is detected, it generates a Warning() and returns the vector 1, 0, 0.
float vm_vec_normalize_safe(vec3d *v);

//normalize a vector. returns mag of source vec. uses approx mag
float vm_vec_copy_normalize_quick(vec3d *dest,vec3d *src);
float vm_vec_normalize_quick(vec3d *v);

//normalize a vector. returns mag of source vec. uses approx mag
float vm_vec_copy_normalize_quick_mag(vec3d *dest,vec3d *src);
float vm_vec_normalize_quick_mag(vec3d *v);

//return the normalized direction vector between two points
//dest = normalized(end - start).  Returns mag of direction vector
//NOTE: the order of the parameters matches the vector subtraction
float vm_vec_normalized_dir(vec3d *dest,vec3d *end,vec3d *start);
float vm_vec_normalized_dir_quick_mag(vec3d *dest,vec3d *end,vec3d *start);
// Returns mag of direction vector
float vm_vec_normalized_dir_quick(vec3d *dest,vec3d *end,vec3d *start);

////returns dot product of two vectors
#ifdef _INLINE_VECMAT
#define vm_vec_dotprod( v0, v1 ) (((v1)->xyz.x*(v0)->xyz.x)+((v1)->xyz.y*(v0)->xyz.y)+((v1)->xyz.z*(v0)->xyz.z))
#define vm_vec_dot( v0, v1 ) (((v1)->xyz.x*(v0)->xyz.x)+((v1)->xyz.y*(v0)->xyz.y)+((v1)->xyz.z*(v0)->xyz.z))
#else
float vm_vec_dotprod(vec3d *v0,vec3d *v1);
#define vm_vec_dot vm_vec_dotprod
#endif

#ifdef _INLINE_VECMAT
#define vm_vec_dot3( x1, y1, z1, v ) (((x1)*(v)->xyz.x)+((y1)*(v)->xyz.y)+((z1)*(v)->xyz.z))
#else
float vm_vec_dot3(float x,float y,float z,vec3d *v);
#endif

//computes cross product of two vectors. returns ptr to dest
//dest CANNOT equal either source
vec3d *vm_vec_crossprod(vec3d *dest,vec3d *src0,vec3d *src1);
#define vm_vec_cross vm_vec_crossprod

// test if 2 vectors are parallel or not.
int vm_test_parallel(vec3d *src0, vec3d *src1);

//computes surface normal from three points. result is normalized
//returns ptr to dest
//dest CANNOT equal either source
vec3d *vm_vec_normal(vec3d *dest,vec3d *p0,vec3d *p1,vec3d *p2);

//computes non-normalized surface normal from three points.
//returns ptr to dest
//dest CANNOT equal either source
vec3d *vm_vec_perp(vec3d *dest,vec3d *p0,vec3d *p1,vec3d *p2);

//computes the delta angle between two vectors.
//vectors need not be normalized. if they are, call vm_vec_delta_ang_norm()
//the forward vector (third parameter) can be NULL, in which case the absolute
//value of the angle in returned.  Otherwise the angle around that vector is
//returned.
float vm_vec_delta_ang(vec3d *v0,vec3d *v1,vec3d *fvec);

//computes the delta angle between two normalized vectors.
float vm_vec_delta_ang_norm(vec3d *v0,vec3d *v1,vec3d *fvec);

//computes a matrix from a set of three angles.  returns ptr to matrix
matrix *vm_angles_2_matrix(matrix *m,angles *a);

//	Computes a matrix from a single angle.
//	angle_index = 0,1,2 for p,b,h
matrix *vm_angle_2_matrix(matrix *m, float a, int angle_index);

//computes a matrix from a forward vector and an angle
matrix *vm_vec_ang_2_matrix(matrix *m,vec3d *v,float a);

//computes a matrix from one or more vectors. The forward vector is required,
//with the other two being optional.  If both up & right vectors are passed,
//the up vector is used.  If only the forward vector is passed, a bank of
//zero is assumed
//returns ptr to matrix
matrix *vm_vector_2_matrix(matrix *m,vec3d *fvec,vec3d *uvec,vec3d *rvec);

//this version of vector_2_matrix requires that the vectors be more-or-less
//normalized and close to perpendicular
matrix *vm_vector_2_matrix_norm(matrix *m,vec3d *fvec,vec3d *uvec=NULL,vec3d *rvec=NULL);

//rotates a vector through a matrix. returns ptr to dest vector
//dest CANNOT equal either source
vec3d *vm_vec_rotate(vec3d *dest,vec3d *src,matrix *m);

//makes an inverse of the src matrix
matrix* vm_matrix_inverse(matrix*dest, matrix*src);

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
vec3d *vm_vec_unrotate(vec3d *dest,vec3d *src,matrix *m);

//transpose a matrix in place. returns ptr to matrix
matrix *vm_transpose_matrix(matrix *m);
#define vm_transpose(m) vm_transpose_matrix(m)

//copy and transpose a matrix. returns ptr to matrix
//dest CANNOT equal source. use vm_transpose_matrix() if this is the case
matrix *vm_copy_transpose_matrix(matrix *dest,matrix *src);
#define vm_copy_transpose(dest,src) vm_copy_transpose_matrix((dest),(src))

//mulitply 2 matrices, fill in dest.  returns ptr to dest
//dest CANNOT equal either source
matrix *vm_matrix_x_matrix(matrix *dest,matrix *src0,matrix *src1);

//extract angles from a matrix
angles *vm_extract_angles_matrix(angles *a,matrix *m);

//extract heading and pitch from a vector, assuming bank==0
angles *vm_extract_angles_vector(angles *a,vec3d *v);

//make sure matrix is orthogonal
void vm_orthogonalize_matrix(matrix *m_src);

// like vm_orthogonalize_matrix(), except that zero vectors can exist within the
// matrix without causing problems.  Valid vectors will be created where needed.
void vm_fix_matrix(matrix *m);

//Rotates the orient matrix by the angles in tangles and then
//makes sure that the matrix is orthogonal.
void vm_rotate_matrix_by_angles( matrix *orient, angles *tangles );

//compute the distance from a point to a plane.  takes the normalized normal
//of the plane (ebx), a point on the plane (edi), and the point to check (esi).
//returns distance in eax
//distance is signed, so negative dist is on the back of the plane
float vm_dist_to_plane(vec3d *checkp,vec3d *norm,vec3d *planep);

// Given mouse movement in dx, dy, returns a 3x3 rotation matrix in RotMat.
// Taken from Graphics Gems III, page 51, "The Rolling Ball"
// Example:
//if ( (Mouse.dx!=0) || (Mouse.dy!=0) ) {
//   vm_trackball( Mouse.dx, Mouse.dy, &MouseRotMat );
//   vm_matrix_x_matrix(&tempm,&LargeView.ev_matrix,&MouseRotMat);
//   LargeView.ev_matrix = tempm;
//}
void vm_trackball( int idx, int idy, matrix * RotMat );

//	Find the point on the line between p0 and p1 that is nearest to int_pnt.
//	Stuff result in nearest_point.
//	Return value indicated where on the line *nearest_point lies.  Between 0.0f and 1.0f means it's
//	in the line segment.  Positive means beyond *p1, negative means before *p0.  2.0f means it's
//	beyond *p1 by 2x.
float find_nearest_point_on_line(vec3d *nearest_point, vec3d *p0, vec3d *p1, vec3d *int_pnt);

float vm_vec_dot_to_point(vec3d *dir, vec3d *p1, vec3d *p2);

void compute_point_on_plane(vec3d *q, plane *planep, vec3d *p);

// ----------------------------------------------------------------------------
// computes the point on a plane closest to a given point (which may be on the plane)
// 
//		inputs:		new_point		=>		point on the plane [result]
//						point				=>		point to compute closest plane point
//						plane_normal	=>		plane normal
//						plane_point		=>		plane point
void vm_project_point_onto_plane(vec3d *new_point, vec3d *point, vec3d *plane_normal, vec3d *plane_point);


//	Returns fairly random vector, "quick" normalized
void vm_vec_rand_vec_quick(vec3d *rvec);

// Given an point "in" rotate it by "angle" around an
// arbritary line defined by a point on the line "line_point" 
// and the normalized line direction, "line_dir"
// Returns the rotated point in "out".
void vm_rot_point_around_line(vec3d *out, vec3d *in, float angle, vec3d *line_point, vec3d *line_dir);

// Given two position vectors, return 0 if the same, else non-zero.
int vm_vec_cmp( vec3d * a, vec3d * b );

// Given two orientation matrices, return 0 if the same, else non-zero.
int vm_matrix_cmp( matrix * a, matrix * b );

// Moves angle 'h' towards 'desired_angle', taking the shortest
// route possible.   It will move a maximum of 'step_size' radians
// each call.   All angles in radians.
void vm_interp_angle( float *h, float desired_angle, float step_size );

// check a matrix for zero rows and columns
int vm_check_matrix_for_zeros(matrix *m);

// see if two vectors are identical
int vm_vec_same(vec3d *v1, vec3d *v2);

//	Interpolate from a start matrix toward a goal matrix, minimizing time between orientations.
// Moves at maximum rotational acceleration toward the goal when far and then max deceleration when close.
// Subject to constaints on rotational velocity and angular accleleration.
// Returns next_orientation valid at time delta_t.
void vm_matrix_interpolate(matrix *goal_orient, matrix *start_orient, vec3d *rotvel_in, float delta_t, 
		matrix *next_orient, vec3d *rotvel_out, vec3d *rotvel_limit, vec3d *acc_limit, int no_overshoot=0);

//	Interpolate from a start forward vec toward a goal forward vec, minimizing time between orientations.
// Moves at maximum rotational acceleration toward the goal when far and then max deceleration when close.
// Subject to constaints on rotational velocity and angular accleleration.
// Returns next forward vec valid at time delta_t.
void vm_forward_interpolate(vec3d *goal_fvec, matrix *orient, vec3d *rotvel_in, float delta_t, float delta_bank,
		matrix *next_orient, vec3d *rotvel_out, vec3d *vel_limit, vec3d *acc_limit, int no_overshoot=0);

// Find the bounding sphere for a set of points (center and radius are output parameters)
void vm_find_bounding_sphere(vec3d *pnts, int num_pnts, vec3d *center, float *radius);

// Version of atan2() that is safe for optimized builds
float atan2_safe(float x, float y);

// Translates from world coordinates to body coordinates
vec3d* vm_rotate_vec_to_body(vec3d *body_vec, vec3d *world_vec, matrix *orient);

// Translates from body coordinates to world coordiantes
vec3d* vm_rotate_vec_to_world(vec3d *world_vec, vec3d *body_vec, matrix *orient);

// estimate next orientation matrix as extrapolation of last and current
void vm_estimate_next_orientation(matrix *last_orient, matrix *current_orient, matrix *next_orient);

//	Return true if all elements of *vec are legal, that is, not a NAN.
int is_valid_vec(vec3d *vec);

//	Return true if all elements of *m are legal, that is, not a NAN.
int is_valid_matrix(matrix *m);

// Finds the rotation matrix corresponding to a rotation of theta about axis u
void vm_quaternion_rotate(matrix *m, float theta, vec3d *u);

// Takes a rotation matrix and returns the axis and angle needed to generate it
void vm_matrix_to_rot_axis_and_angle(matrix *m, float *theta, vec3d *rot_axis);

// interpolate between 2 vectors. t goes from 0.0 to 1.0. at
void vm_vec_interp_constant(vec3d *out, vec3d *v1, vec3d *v2, float t);

// randomly perturb a vector around a given (normalized vector) or optional orientation matrix
void vm_vec_random_cone(vec3d *out, vec3d *in, float max_angle, matrix *orient = NULL);
void vm_vec_random_cone(vec3d *out, vec3d *in, float min_angle, float max_angle, matrix *orient = NULL);

// given a start vector, an orientation and a radius, give a point on the plane of the circle
// if on_edge is 1, the point is on the very edge of the circle
void vm_vec_random_in_circle(vec3d *out, vec3d *in, matrix *orient, float radius, int on_edge);

// find the nearest point on the line to p. if dist is non-NULL, it is filled in
// returns 0 if the point is inside the line segment, -1 if "before" the line segment and 1 ir "after" the line segment
int vm_vec_dist_to_line(vec3d *p, vec3d *l0, vec3d *l1, vec3d *nearest, float *dist);

// Goober5000
// Finds the distance squared to a line.  Same as above, except it uses vm_vec_dist_squared, which is faster;
// and it doesn't check whether the nearest point is on the line segment.
void vm_vec_dist_squared_to_line(vec3d *p, vec3d *l0, vec3d *l1, vec3d *nearest, float *dist_squared);

void vm_vert2vec(vertex *vert, vec3d *vec);
void vm_vec2vert(vec3d *vec, vertex *vert);

#endif


