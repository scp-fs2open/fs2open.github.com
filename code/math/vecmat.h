/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
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
#define IS_VEC_NULL_SQ_SAFE(v) ( ( (v)->xyz.x > -1e-16 ) && ( (v)->xyz.x < 1e-16 ) && \
								 ( (v)->xyz.y > -1e-16 ) && ( (v)->xyz.y < 1e-16 ) && \
								 ( (v)->xyz.z > -1e-16 ) && ( (v)->xyz.z < 1e-16 ) )

#define IS_VEC_NULL(v) ( ( (v)->xyz.x > -1e-36 ) && ( (v)->xyz.x < 1e-36 ) && \
						 ( (v)->xyz.y > -1e-36 ) && ( (v)->xyz.y < 1e-36 ) && \
						 ( (v)->xyz.z > -1e-36 ) && ( (v)->xyz.z < 1e-36 ) )

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

void vm_vec_scale_sub(vec3d *dest,vec3d *src1,vec3d *src2,float k);

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
matrix *vm_vector_2_matrix_norm(matrix *m,vec3d *fvec,vec3d *uvec = NULL,vec3d *rvec = NULL);

//rotates a vector through a matrix. returns ptr to dest vector
//dest CANNOT equal either source
vec3d *vm_vec_rotate(vec3d *dest,vec3d *src,matrix *m);

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
float vm_interp_angle( float *h, float desired_angle, float step_size );

// calculate and return the difference (ie. delta) between two angles
// using same method as with vm_interp_angle().
float vm_delta_from_interp_angle( float current_angle, float desired_angle );

// check a matrix for zero rows and columns
int vm_check_matrix_for_zeros(matrix *m);

// see if two vectors are identical
int vm_vec_same(vec3d *v1, vec3d *v2);

// see if two matrices are identical
int vm_matrix_same(matrix *m1, matrix *m2);

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

//SUSHI: 2D vector "box" scaling
void vm_vec_boxscale(vec2d *vec, float scale);

#endif


