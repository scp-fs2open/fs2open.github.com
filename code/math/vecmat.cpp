/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <cstdio>
#include <numeric>
#if _M_IX86_FP >= 1
	#include <xmmintrin.h>
#endif

#include "math/vecmat.h"
#include "utils/RandomRange.h"


#define	SMALL_NUM	1e-7
#define	SMALLER_NUM	1e-20
#define	CONVERT_RADIANS	0.017453		// conversion factor from degrees to radians

vec3d vmd_zero_vector = ZERO_VECTOR;
vec3d vmd_scale_identity_vector = SCALE_IDENTITY_VECTOR;
vec3d vmd_x_vector = { { { 1.0f, 0.0f, 0.0f } } };
vec3d vmd_y_vector = { { { 0.0f, 1.0f, 0.0f } } };
vec3d vmd_z_vector = { { { 0.0f, 0.0f, 1.0f } } };
matrix vmd_zero_matrix = ZERO_MATRIX;
matrix4 vmd_zero_matrix4 = ZERO_MATRIX4;
matrix vmd_identity_matrix = IDENTITY_MATRIX;
angles vmd_zero_angles = { 0.0f, 0.0f, 0.0f };

#define	UNINITIALIZED_VALUE	-12345678.9f

bool vm_vec_equal(const vec4 &self, const vec4 &other)
{
	return fl_equal(self.a1d[0], other.a1d[0]) && fl_equal(self.a1d[1], other.a1d[1]) && fl_equal(self.a1d[2], other.a1d[2]) && fl_equal(self.a1d[3], other.a1d[3]);
}

bool vm_vec_equal(const vec3d &self, const vec3d &other)
{
	return fl_equal(self.a1d[0], other.a1d[0]) && fl_equal(self.a1d[1], other.a1d[1]) && fl_equal(self.a1d[2], other.a1d[2]);
}

bool vm_vec_equal(const vec2d &self, const vec2d &other)
{
	return fl_equal(self.x, other.x) && fl_equal(self.y, other.y);
}

bool vm_matrix_equal(const matrix &self, const matrix &other)
{
	return vm_vec_equal(self.vec.fvec, other.vec.fvec) && vm_vec_equal(self.vec.uvec, other.vec.uvec) && vm_vec_equal(self.vec.rvec, other.vec.rvec);
}

bool vm_matrix_equal(const matrix4 &self, const matrix4 &other)
{
	return vm_vec_equal(self.vec.fvec, other.vec.fvec) && 
		vm_vec_equal(self.vec.rvec, other.vec.rvec) && 
		vm_vec_equal(self.vec.uvec, other.vec.uvec) && 
		vm_vec_equal(self.vec.pos, other.vec.pos);
}

// -----------------------------------------------------------
// atan2_safe()
//
// Wrapper around atan2() that used atanf() to calculate angle.  Safe
// for optimized builds.  Handles special cases when x == 0.
//
float atan2_safe(float y, float x)
{
	float ang;

	// special case, x == 0
	if ( x == 0.0f ) {
		if ( y == 0.0f ) 
			ang = 0.0f;
		else if ( y > 0.0f )
			ang = PI_2;
		else
			ang = -PI_2;

		return ang;
	}
	
	ang = atanf(y/x);
	if ( x < 0.0f ){
		ang += PI;
	}

	return ang;
}

// ---------------------------------------------------------------------
// vm_vec_component()
//
// finds projection of a vector along a unit (normalized) vector 
//
float vm_vec_projection_parallel(vec3d *component, const vec3d *src, const vec3d *unit_vec)
{
	float mag;
	Assertion(vm_vec_is_normalized(unit_vec), "unit_vec must be normalized!");

	mag = vm_vec_dot(src, unit_vec);
	vm_vec_copy_scale(component, unit_vec, mag);
	return mag;
}

// ---------------------------------------------------------------------
// vm_vec_projection_onto_plane()
//
// finds projection of a vector onto a plane specified by a unit normal vector 
//
void vm_vec_projection_onto_plane(vec3d *projection, const vec3d *src, const vec3d *unit_normal)
{
	float mag;
	Assertion(vm_vec_is_normalized(unit_normal), "unit_normal must be normalized!");

	mag = vm_vec_dot(src, unit_normal);
	*projection = *src;
	vm_vec_scale_add2(projection, unit_normal, -mag);
}

// ---------------------------------------------------------------------
// vm_vec_project_point_onto_plane()
//
// finds the point on a plane closest to a given point
// moves the point in the direction of the plane normal until it is on the plane
//
void vm_project_point_onto_plane(vec3d *new_point, const vec3d *point, const vec3d *plane_normal, const vec3d *plane_point)
{
	float D;		// plane constant in Ax+By+Cz+D = 0   or   dot(X,n) - dot(Xp,n) = 0, so D = -dot(Xp,n)
	float dist;
	Assertion(vm_vec_is_normalized(plane_normal), "plane_normal must be normalized!");

	D = -vm_vec_dot(plane_point, plane_normal);
	dist = vm_vec_dot(point, plane_normal) + D;

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
	m->vec.rvec.xyz.x = 1.0f;	m->vec.rvec.xyz.y = 0.0f;	m->vec.rvec.xyz.z = 0.0f;
	m->vec.uvec.xyz.x = 0.0f;	m->vec.uvec.xyz.y = 1.0f;	m->vec.uvec.xyz.z = 0.0f;
	m->vec.fvec.xyz.x = 0.0f;	m->vec.fvec.xyz.y = 0.0f;	m->vec.fvec.xyz.z = 1.0f;
}

vec3d vm_vec_new(float x, float y, float z)
{
	vec3d vec;

	vec.xyz.x = x;
	vec.xyz.y = y;
	vec.xyz.z = z;

	return vec;
}

//adds two vectors, fills in dest, returns ptr to dest
//ok for dest to equal either source, but should use vm_vec_add2() if so
//dest = src0 + src1
void vm_vec_add(vec3d *dest, const vec3d *src0, const vec3d *src1)
{
	dest->xyz.x = src0->xyz.x + src1->xyz.x;
	dest->xyz.y = src0->xyz.y + src1->xyz.y;
	dest->xyz.z = src0->xyz.z + src1->xyz.z;
}

//subs two vectors, fills in dest, returns ptr to dest
//ok for dest to equal either source, but should use vm_vec_sub2() if so
//dest = src0 - src1
void vm_vec_sub(vec3d *dest, const vec3d *src0, const vec3d *src1)
{
	dest->xyz.x = src0->xyz.x - src1->xyz.x;
	dest->xyz.y = src0->xyz.y - src1->xyz.y;
	dest->xyz.z = src0->xyz.z - src1->xyz.z;
}


//adds one vector to another. returns ptr to dest
//dest can equal source
//dest += src
void vm_vec_add2(vec3d *dest, const vec3d *src)
{
	dest->xyz.x += src->xyz.x;
	dest->xyz.y += src->xyz.y;
	dest->xyz.z += src->xyz.z;
}

//subs one vector from another, returns ptr to dest
//dest can equal source
//dest -= src
void vm_vec_sub2(vec3d *dest, const vec3d *src)
{
	dest->xyz.x -= src->xyz.x;
	dest->xyz.y -= src->xyz.y;
	dest->xyz.z -= src->xyz.z;
}

//averages n vectors. returns ptr to dest
//dest can equal any vector in src[]
//dest = sum(src[]) / n
vec3d *vm_vec_avg_n(vec3d *dest, int n, const vec3d src[])
{
	float x = 0.0f, y = 0.0f, z = 0.0f;
	float inv_n = 1.0f / (float) n;;

	for(int i = 0; i<n; i++){
		x += src[i].xyz.x;
		y += src[i].xyz.y;
		z += src[i].xyz.z;
	}

	dest->xyz.x = x * inv_n;
	dest->xyz.y = y * inv_n;
	dest->xyz.z = z * inv_n;

	return dest;
}


//averages two vectors. returns ptr to dest
//dest can equal either source
//dest = (src0 + src1) * 0.5
vec3d *vm_vec_avg(vec3d *dest, const vec3d *src0, const vec3d *src1)
{
	dest->xyz.x = (src0->xyz.x + src1->xyz.x) * 0.5f;
	dest->xyz.y = (src0->xyz.y + src1->xyz.y) * 0.5f;
	dest->xyz.z = (src0->xyz.z + src1->xyz.z) * 0.5f;

	return dest;
}

//averages three vectors. returns ptr to dest
//dest can equal any source
//dest = (src0 + src1 + src2) *0.33
vec3d *vm_vec_avg3(vec3d *dest, const vec3d *src0, const vec3d *src1, const vec3d *src2)
{
	dest->xyz.x = (src0->xyz.x + src1->xyz.x + src2->xyz.x) * 0.333333333f;
	dest->xyz.y = (src0->xyz.y + src1->xyz.y + src2->xyz.y) * 0.333333333f;
	dest->xyz.z = (src0->xyz.z + src1->xyz.z + src2->xyz.z) * 0.333333333f;
	return dest;
}

//averages four vectors. returns ptr to dest
//dest can equal any source
//dest = (src0 + src1 + src2 + src3) * 0.25
vec3d *vm_vec_avg4(vec3d *dest, const vec3d *src0, const vec3d *src1, const vec3d *src2, const vec3d *src3)
{
	dest->xyz.x = (src0->xyz.x + src1->xyz.x + src2->xyz.x + src3->xyz.x) * 0.25f;
	dest->xyz.y = (src0->xyz.y + src1->xyz.y + src2->xyz.y + src3->xyz.y) * 0.25f;
	dest->xyz.z = (src0->xyz.z + src1->xyz.z + src2->xyz.z + src3->xyz.z) * 0.25f;
	return dest;
}


//scales a vector in place.
//dest *= s
void vm_vec_scale(vec3d *dest, float s)
{
	dest->xyz.x = dest->xyz.x * s;
	dest->xyz.y = dest->xyz.y * s;
	dest->xyz.z = dest->xyz.z * s;
}

//scales a 4-component vector in place.
// dest *= s
void vm_vec_scale(vec4 *dest, float s)
{
	dest->xyzw.x = dest->xyzw.x * s;
	dest->xyzw.y = dest->xyzw.y * s;
	dest->xyzw.z = dest->xyzw.z * s;
	dest->xyzw.w = dest->xyzw.w * s;
}

//scales and copies a vector.
// dest = src * s
void vm_vec_copy_scale(vec3d *dest, const vec3d *src, float s)
{
	dest->xyz.x = src->xyz.x*s;
	dest->xyz.y = src->xyz.y*s;
	dest->xyz.z = src->xyz.z*s;
}

//scales a vector, adds it to another, and stores in a 3rd vector
//dest = src1 + k * src2
void vm_vec_scale_add(vec3d *dest, const vec3d *src1, const vec3d *src2, float k)
{
	dest->xyz.x = src1->xyz.x + src2->xyz.x*k;
	dest->xyz.y = src1->xyz.y + src2->xyz.y*k;
	dest->xyz.z = src1->xyz.z + src2->xyz.z*k;
}

//scales a vector, subtracts it from another, and stores in a 3rd vector
//dest = src1 - (k * src2)
void vm_vec_scale_sub(vec3d *dest, const vec3d *src1, const vec3d *src2, float k)
{
	dest->xyz.x = src1->xyz.x - src2->xyz.x*k;
	dest->xyz.y = src1->xyz.y - src2->xyz.y*k;
	dest->xyz.z = src1->xyz.z - src2->xyz.z*k;
}

//scales a vector and adds it to another
//dest += k * src
void vm_vec_scale_add2(vec3d *dest, const vec3d *src, float k)
{
	dest->xyz.x += src->xyz.x*k;
	dest->xyz.y += src->xyz.y*k;
	dest->xyz.z += src->xyz.z*k;
}

//scales a vector and subtracts it from another
//dest -= k * src
void vm_vec_scale_sub2(vec3d *dest, const vec3d *src, float k)
{
	dest->xyz.x -= src->xyz.x*k;
	dest->xyz.y -= src->xyz.y*k;
	dest->xyz.z -= src->xyz.z*k;
}

//scales a vector in place, taking n/d for scale.
//dest *= n/d
void vm_vec_scale2(vec3d *dest, float n, float d)
{	
	d = 1.0f/d;

	dest->xyz.x = dest->xyz.x* n * d;
	dest->xyz.y = dest->xyz.y* n * d;
	dest->xyz.z = dest->xyz.z* n * d;
}

//returns dot product of 2 vectors
float vm_vec_dot(const vec3d *v0, const vec3d *v1)
{
	return (v1->xyz.x*v0->xyz.x)+(v1->xyz.y*v0->xyz.y)+(v1->xyz.z*v0->xyz.z);
}


//returns dot product of <x,y,z> and vector
float vm_vec_dot3(float x, float y, float z, const vec3d *v)
{
	return (x*v->xyz.x)+(y*v->xyz.y)+(z*v->xyz.z);
}

//returns magnitude of a vector
float vm_vec_mag(const vec3d *v)
{
	float mag1;

	mag1 = (v->xyz.x * v->xyz.x) + (v->xyz.y * v->xyz.y) + (v->xyz.z * v->xyz.z);

	if (mag1 <= 0.0f) {
		return 0.0f;
	}

	return fl_sqrt(mag1);
}

//returns squared magnitude of a vector, useful if you want to compare distances
float vm_vec_mag_squared(const vec3d *v)
{
	return ((v->xyz.x * v->xyz.x) + (v->xyz.y * v->xyz.y) + (v->xyz.z * v->xyz.z));
}

//returns the square of the difference between v0 and v1 (the distance, squared)
//just like vm_vec_mag_squared, but the distance between two points instead.
float vm_vec_dist_squared(const vec3d *v0, const vec3d *v1)
{
	float dx, dy, dz;

	dx = v0->xyz.x - v1->xyz.x;
	dy = v0->xyz.y - v1->xyz.y;
	dz = v0->xyz.z - v1->xyz.z;
	return dx*dx + dy*dy + dz*dz;
}

//computes the distance between two points. (does sub and mag)
float vm_vec_dist(const vec3d *v0, const vec3d *v1)
{
	float t1;
	vec3d t;

	vm_vec_sub(&t,v0,v1);

	t1 = vm_vec_mag(&t);

	return t1;
}

bool vm_vec_is_normalized(const vec3d *v)
{
	// By the standards of FSO, it is sufficient to check that the magnitude is close to 1.
	return vm_vec_mag(v) > 0.999f && vm_vec_mag(v) < 1.001f;
}

//normalize a vector. returns mag of source vec (always greater than zero)
float vm_vec_copy_normalize(vec3d *dest, const vec3d *src)
{
	float m;

	m = vm_vec_mag(src);

	//	Mainly here to trap attempts to normalize a null vector.
	if (m <= 0.0f) {
		mprintf(("Null vec3d in vec3d normalize.\n"
				 "Trace out of vecmat.cpp and find offending code.\n"));

		dest->xyz.x = 1.0f;
		dest->xyz.y = 0.0f;
		dest->xyz.z = 0.0f;

		return 1.0f;
	}

	float im = 1.0f / m;

	dest->xyz.x = src->xyz.x * im;
	dest->xyz.y = src->xyz.y * im;
	dest->xyz.z = src->xyz.z * im;
	
	return m;
}

//normalize a vector. returns mag of source vec (always greater than zero)
float vm_vec_normalize(vec3d *v)
{
	float t;
	t = vm_vec_copy_normalize(v,v);
	return t;
}

// Normalize a vector.
// If vector is 0,0,0, return 1.0f, and change v to 1,0,0.  
// Otherwise return the magnitude.
// No warning() generated for null vector.
float vm_vec_normalize_safe(vec3d *v)
{
	float m;

	m = vm_vec_mag(v);

	//	Mainly here to trap attempts to normalize a null vector.
	if (m <= 0.0f) {
		v->xyz.x = 1.0f;
		v->xyz.y = 0.0f;
		v->xyz.z = 0.0f;
		return 1.0f;
	}

	float im = 1.0f / m;

	v->xyz.x *= im;
	v->xyz.y *= im;
	v->xyz.z *= im;

	return m;

}

//return the normalized direction vector between two points
//dest = normalized(end - start).  Returns mag of direction vector
//NOTE: the order of the parameters matches the vector subtraction
float vm_vec_normalized_dir(vec3d *dest, const vec3d *end, const vec3d *start)
{
	float t;

	vm_vec_sub(dest,end,start);
	// VECMAT-ERROR: NULL VEC3D (end == start)
	t = vm_vec_normalize_safe(dest);
	return t;
}

//computes surface normal from three points. result is normalized
//returns ptr to dest
//dest CANNOT equal either source
vec3d *vm_vec_normal(vec3d *dest, const vec3d *p0, const vec3d *p1, const vec3d *p2)
{
	Assert(dest != p0);
	Assert(dest != p1);
	Assert(dest != p2);

	vm_vec_perp(dest,p0,p1,p2);

	vm_vec_normalize(dest);

	return dest;
}


//computes cross product of two vectors.
//Note: this magnitude of the resultant vector is the
//product of the magnitudes of the two source vectors.  This means it is
//quite easy for this routine to overflow and underflow.  Be careful that
//your inputs are ok.
vec3d *vm_vec_cross(vec3d *dest, const vec3d *src0, const vec3d *src1)
{
	dest->xyz.x = (src0->xyz.y * src1->xyz.z) - (src0->xyz.z * src1->xyz.y);
	dest->xyz.y = (src0->xyz.z * src1->xyz.x) - (src0->xyz.x * src1->xyz.z);
	dest->xyz.z = (src0->xyz.x * src1->xyz.y) - (src0->xyz.y * src1->xyz.x);

	return dest;
}

int vm_test_parallel(const vec3d *src0, const vec3d *src1)
{
	vec3d partial1;
	vec3d partial2;

	/*
	 * To test if two vectors are parallel, calculate their cross product.
	 * If the result is zero, then the vectors are parallel. It is better
	 * to compare the two cross product "partials" (for lack of a better
	 * word) against each other instead of the final cross product against
	 * zero.
	 */

	partial1.xyz.x = (src0->xyz.y * src1->xyz.z);
	partial1.xyz.y = (src0->xyz.z * src1->xyz.x);
	partial1.xyz.z = (src0->xyz.x * src1->xyz.y);

	partial2.xyz.x =  (src0->xyz.z * src1->xyz.y);
	partial2.xyz.y =  (src0->xyz.x * src1->xyz.z);
	partial2.xyz.z =  (src0->xyz.y * src1->xyz.x);

	return vm_vec_equal(partial1, partial2);
}

//computes non-normalized surface normal from three points.
//returns ptr to dest
//dest CANNOT equal either source
vec3d *vm_vec_perp(vec3d *dest, const vec3d *p0, const vec3d *p1,const vec3d *p2)
{
	Assert(dest != p0);
	Assert(dest != p1);
	Assert(dest != p2);

	vec3d t0,t1;

	vm_vec_sub(&t0,p1,p0);
	vm_vec_sub(&t1,p2,p1);

	return vm_vec_cross(dest,&t0,&t1);
}


//computes the delta angle between two vectors.
//vectors need not be normalized. if they are, call vm_vec_delta_ang_norm()
//the up vector (third parameter) can be NULL, in which case the absolute
//value of the angle in returned.  
//Otherwise, the delta ang will be positive if the v0 -> v1 direction from the
//point of view of uvec is clockwise, negative if counterclockwise.
//This vector should be orthogonal to v0 and v1
float vm_vec_delta_ang(const vec3d *v0, const vec3d *v1, const vec3d *uvec)
{
	float t;
	vec3d t0,t1,t2;

	vm_vec_copy_normalize(&t0,v0);
	vm_vec_copy_normalize(&t1,v1);

	if (uvec == nullptr) {
		t = vm_vec_delta_ang_norm(&t0, &t1, NULL);
	} else {
		vm_vec_copy_normalize(&t2,uvec);
		t = vm_vec_delta_ang_norm(&t0,&t1,&t2);
	}

	return t;
}

//computes the delta angle between two normalized vectors.
float vm_vec_delta_ang_norm(const vec3d *v0, const vec3d *v1, const vec3d *uvec)
{
	float a;
	vec3d t;

	a = acosf_safe(vm_vec_dot(v0,v1));

	if (uvec) {
		vm_vec_cross(&t,v0,v1);
		if ( vm_vec_dot(&t,uvec) < 0.0 )	{
			a = -a;
		}
	}

	return a;
}

// helper function that fills in matrix m based on provided sine and cosine values. 
static matrix *sincos_2_matrix(matrix *m, float sinp, float cosp, float sinb, float cosb, float sinh, float cosh)
{
	float sbsh,cbch,cbsh,sbch;


	sbsh = sinb*sinh;
	cbch = cosb*cosh;
	cbsh = cosb*sinh;
	sbch = sinb*cosh;

	m->vec.rvec.xyz.x = cbch + sinp*sbsh;		//m1
	m->vec.uvec.xyz.z = sbsh + sinp*cbch;		//m8

	m->vec.uvec.xyz.x = sinp*cbsh - sbch;		//m2
	m->vec.rvec.xyz.z = sinp*sbch - cbsh;		//m7

	m->vec.fvec.xyz.x = sinh*cosp;				//m3
	m->vec.rvec.xyz.y = sinb*cosp;				//m4
	m->vec.uvec.xyz.y = cosb*cosp;				//m5
	m->vec.fvec.xyz.z = cosh*cosp;				//m9

	m->vec.fvec.xyz.y = -sinp;								//m6


	return m;

}

//computes a matrix from a set of three angles.  returns ptr to matrix
matrix *vm_angles_2_matrix(matrix *m, const angles *a)
{
	matrix * t;
	float sinp,cosp,sinb,cosb,sinh,cosh;

	sinp = sinf(a->p); cosp = cosf(a->p);
	sinb = sinf(a->b); cosb = cosf(a->b);
	sinh = sinf(a->h); cosh = cosf(a->h);

	t = sincos_2_matrix(m,sinp,cosp,sinb,cosb,sinh,cosh);

	return t;
}

//computes a matrix from one angle.
//	angle_index = 0,1,2 for p,b,h
matrix *vm_angle_2_matrix(matrix *m, float a, int angle_index)
{
	matrix * t;
	float sinp,cosp,sinb,cosb,sinh,cosh;

	/*
	 * Initialize sin and cos variables using an initial angle of
	 * zero degrees.  Recall that sin(0) = 0 and cos(0) = 1.
	 */

	sinp = 0.0f;	cosp = 1.0f;
	sinb = 0.0f;	cosb = 1.0f;
	sinh = 0.0f;	cosh = 1.0f;

	switch (angle_index) {
	case 0:
		sinp = sinf(a); cosp = cosf(a);
		break;
	case 1:
		sinb = sinf(a); cosb = cosf(a);
		break;
	case 2:
		sinh = sinf(a); cosh = cosf(a);
		break;
	}

	t = sincos_2_matrix(m,sinp,cosp,sinb,cosb,sinh,cosh);

	return t;
}


//computes a matrix from a forward vector and an angle
matrix *vm_vec_ang_2_matrix(matrix *m, const vec3d *v, float a)
{
	matrix * t;
	float sinb,cosb,sinp,cosp,sinh,cosh;

	sinb = sinf(a); cosb = cosf(a);

	sinp = -v->xyz.y;
	cosp = fl_sqrt(1.0f - sinp*sinp);

	sinh = v->xyz.x / cosp;
	cosh = v->xyz.z / cosp;

	t = sincos_2_matrix(m,sinp,cosp,sinb,cosb,sinh,cosh);

	return t;
}

/**
 * @brief Generates a matrix from a normalized fvec.
 *
 * @param[in,out] matrix The matrix to generate
 *
 * @details The matrix's fvec is used to generate the uvec and rvec
 *
 * @sa vm_vector_2_matrix(), vm_vector_2_matrix_norm()
 */
void vm_vector_2_matrix_gen_vectors(matrix *m)
{
	vec3d *xvec=&m->vec.rvec;
	vec3d *yvec=&m->vec.uvec;
	vec3d *zvec=&m->vec.fvec;
	
	if ((zvec->xyz.x==0.0f) && (zvec->xyz.z==0.0f)) {		//forward vec is straight up or down
		m->vec.rvec.xyz.x = 1.0f;
		m->vec.uvec.xyz.z = (zvec->xyz.y<0.0f)?1.0f:-1.0f;

		m->vec.rvec.xyz.y = m->vec.rvec.xyz.z = m->vec.uvec.xyz.x = m->vec.uvec.xyz.y = 0.0f;
	}
	else { 		//not straight up or down

		xvec->xyz.x = zvec->xyz.z;
		xvec->xyz.y = 0.0f;
		xvec->xyz.z = -zvec->xyz.x;

		vm_vec_normalize(xvec);

		vm_vec_cross(yvec,zvec,xvec);

	}
}

matrix *vm_vector_2_matrix(matrix *m, const vec3d *fvec, const vec3d *uvec, const vec3d *rvec)
{
	vec3d fvec_norm;
	vm_vec_copy_normalize(&fvec_norm, fvec);
	fvec = &fvec_norm;

	vec3d uvec_norm;
	if (uvec != nullptr) {
		vm_vec_copy_normalize(&uvec_norm, uvec);
		uvec = &uvec_norm;
	}

	vec3d rvec_norm;
	if (rvec != nullptr) {
		vm_vec_copy_normalize(&rvec_norm, rvec);
		rvec = &rvec_norm;
	}

	// Call the actuall function for normalized vectors
	return vm_vector_2_matrix_norm(m, fvec, uvec, rvec);
}

matrix *vm_vector_2_matrix_norm(matrix *m, const vec3d *fvec, const vec3d *uvec, const vec3d *rvec)
{
	matrix temp = vmd_identity_matrix;

	vec3d *xvec=&temp.vec.rvec;
	vec3d *yvec=&temp.vec.uvec;
	vec3d *zvec=&temp.vec.fvec;

	Assert(fvec != NULL);

	*zvec = *fvec;

	if (uvec == NULL) {
		if (rvec == NULL) {     //just forward vec
			vm_vector_2_matrix_gen_vectors(&temp);
		}
		else {                      //use right vec
			*xvec = *rvec;

			vm_vec_cross(yvec,zvec,xvec);

			//normalize new perpendicular vector
			vm_vec_normalize(yvec);

			//now recompute right vector, in case it wasn't entirely perpendiclar
			vm_vec_cross(xvec,yvec,zvec);
		}
	}
	else {      //use up vec
		*yvec = *uvec;

		vm_vec_cross(xvec,yvec,zvec);

		if (vm_vec_equal(*xvec, vmd_zero_vector)) {
			// uvec was bogus (either same as fvec or -fvec)
			// Reset temp to the original values and do the setup again
			temp = *m;

			temp.vec.fvec = *fvec;

			vm_vector_2_matrix_gen_vectors(&temp);
		}
		else {
			//normalize new perpendicular vector
			vm_vec_normalize(xvec);

			//now recompute up vector, in case it wasn't entirely perpendiclar
			vm_vec_cross(yvec,zvec,xvec);
		}
	}

	// Copy the computed values into the output parameter
	*m = temp;
	return m;
}


// rotates a vector through a matrix, writes to *dest and returns the pointer
// if m is a rotation matrix it will preserve the length of *src, so normalised vectors will remain normalised
vec3d *vm_vec_rotate(vec3d *dest, const vec3d *src, const matrix *m)
{
	*dest = (*m) * (*src);

	return dest;
}

// like vm_vec_rotate, but uses the transpose matrix instead. for rotations, this is an inverse.
vec3d *vm_vec_unrotate(vec3d *dest, const vec3d *src, const matrix *m)
{
	matrix mt;

	vm_copy_transpose(&mt, m);
	*dest = mt * (*src);

	return dest;
}

//transpose a matrix in place. returns ptr to matrix
matrix *vm_transpose(matrix *m)
{
	float t;

	t = m->vec.uvec.xyz.x;  m->vec.uvec.xyz.x = m->vec.rvec.xyz.y;  m->vec.rvec.xyz.y = t;
	t = m->vec.fvec.xyz.x;  m->vec.fvec.xyz.x = m->vec.rvec.xyz.z;  m->vec.rvec.xyz.z = t;
	t = m->vec.fvec.xyz.y;  m->vec.fvec.xyz.y = m->vec.uvec.xyz.z;  m->vec.uvec.xyz.z = t;

	return m;
}

//copy and transpose a matrix. returns ptr to matrix
//dest CANNOT equal source. use vm_transpose() if this is the case
matrix *vm_copy_transpose(matrix *dest, const matrix *src)
{
	Assert(dest != src);

	dest->vec.rvec.xyz.x = src->vec.rvec.xyz.x;
	dest->vec.rvec.xyz.y = src->vec.uvec.xyz.x;
	dest->vec.rvec.xyz.z = src->vec.fvec.xyz.x;

	dest->vec.uvec.xyz.x = src->vec.rvec.xyz.y; //-V537
	dest->vec.uvec.xyz.y = src->vec.uvec.xyz.y;
	dest->vec.uvec.xyz.z = src->vec.fvec.xyz.y; //-V537

	dest->vec.fvec.xyz.x = src->vec.rvec.xyz.z;
	dest->vec.fvec.xyz.y = src->vec.uvec.xyz.z; //-V537
	dest->vec.fvec.xyz.z = src->vec.fvec.xyz.z;


	return dest;
}

inline vec3d operator*(const matrix& A, const vec3d& v) {
	vec3d out;

	out.xyz.x = vm_vec_dot(&A.vec.rvec, &v);
	out.xyz.y = vm_vec_dot(&A.vec.uvec, &v);
	out.xyz.z = vm_vec_dot(&A.vec.fvec, &v);

	return out;
}

inline matrix operator*(const matrix& A, const matrix& B) {
	matrix BT, out;

	// we transpose B here for concision and also potential vectorisation opportunities
	vm_copy_transpose(&BT, &B);

	out.vec.rvec = BT * A.vec.rvec;
	out.vec.uvec = BT * A.vec.uvec;
	out.vec.fvec = BT * A.vec.fvec;

	return out;
}

// Old matrix multiplication routine. Note that the order of multiplication is inverted
// compared to the mathematical standard: formally, this calculates src1 * src0
matrix *vm_matrix_x_matrix(matrix *dest, const matrix *src0, const matrix *src1)
{
	*dest = (*src1) * (*src0);

	return dest;
}

//extract angles from a matrix
angles *vm_extract_angles_matrix(angles *a, const matrix *m)
{
	float sinh,cosh,cosp;

	a->h = atan2_safe(m->vec.fvec.xyz.x,m->vec.fvec.xyz.z);

	sinh = sinf(a->h); cosh = cosf(a->h);

	if (fl_abs(sinh) > fl_abs(cosh))				//sine is larger, so use it
		cosp = m->vec.fvec.xyz.x*sinh;
	else											//cosine is larger, so use it
		cosp = m->vec.fvec.xyz.z*cosh;

	//using the fvec_xz_distance extracts the correct pitch from the matrix --wookieejedi
	//previously cosp was used as the denominator, but this resulted in some incorrect pitch extractions
	float fvec_xz_distance;

	fvec_xz_distance = fl_sqrt( ( (m->vec.fvec.xyz.x)*(m->vec.fvec.xyz.x) ) + ( (m->vec.fvec.xyz.z)*(m->vec.fvec.xyz.z) ) );

	a->p = atan2_safe(-m->vec.fvec.xyz.y, fvec_xz_distance);

	if (cosp == 0.0f)	//the cosine of pitch is zero.  we're pitched straight up. say no bank

		a->b = 0.0f;

	else {
		float sinb,cosb;

		sinb = m->vec.rvec.xyz.y/cosp;
		cosb = m->vec.uvec.xyz.y/cosp;

		a->b = atan2_safe(sinb,cosb);
	}


	return a;
}

// alternate method for extracting angles which seems to be
// less susceptible to rounding errors -- see section 8.7.2
// (pages 278-281) of 3D Math Primer for Graphics and Game
// Development, 2nd Edition
// http://books.google.com/books?id=X3hmuhBoFF0C&printsec=frontcover#v=onepage&q&f=false
angles *vm_extract_angles_matrix_alternate(angles *a, const matrix *m)
{
	Assert(a != NULL);
	Assert(m != NULL);

	a->p = asinf_safe(-m->vec.fvec.xyz.y);

	// Check for the Gimbal lock case, giving a slight tolerance
	// for numerical imprecision
	if (fabs(m->vec.fvec.xyz.y) > 0.9999f) {
		// We are looking straight up or down.
		// Slam bank to zero and just set heading
		a->b = 0.0f;
		a->h = atan2(-m->vec.rvec.xyz.z, m->vec.rvec.xyz.x);
	} else {
		// Compute heading
		a->h = atan2(m->vec.fvec.xyz.x, m->vec.fvec.xyz.z);

		// Compute bank
		a->b = atan2(m->vec.rvec.xyz.y, m->vec.uvec.xyz.y);
	}

	return a;
}


//extract heading and pitch from a vector, assuming bank==0
static angles *vm_extract_angles_vector_normalized(angles *a, const vec3d *v)
{

	a->b = 0.0f;		//always zero bank

	a->p = asinf_safe(-v->xyz.y);

	a->h = atan2_safe(v->xyz.z,v->xyz.x);

	return a;
}

//extract heading and pitch from a vector, assuming bank==0
angles *vm_extract_angles_vector(angles *a, const vec3d *v)
{
	vec3d t;

	vm_vec_copy_normalize(&t,v);
	vm_extract_angles_vector_normalized(a,&t);

	return a;
}

//compute the distance from a point to a plane.  takes the normalized normal
//of the plane (ebx), a point on the plane (edi), and the point to check (esi).
//returns distance in eax
//distance is signed, so negative dist is on the back of the plane
float vm_dist_to_plane(const vec3d *checkp, const vec3d *norm, const vec3d *planep)
{
	float t1;
	vec3d t;

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

	RotMat->vec.rvec.xyz.x = cos_theta + (dydr*dydr)*cos_theta1;
	RotMat->vec.uvec.xyz.x = - ((dxdr*dydr)*cos_theta1);
	RotMat->vec.fvec.xyz.x = (dxdr*sin_theta);

	RotMat->vec.rvec.xyz.y = RotMat->vec.uvec.xyz.x;
	RotMat->vec.uvec.xyz.y = cos_theta + ((dxdr*dxdr)*cos_theta1);
	RotMat->vec.fvec.xyz.y = (dydr*sin_theta);

	RotMat->vec.rvec.xyz.z = -RotMat->vec.fvec.xyz.x;
	RotMat->vec.uvec.xyz.z = -RotMat->vec.fvec.xyz.y;
	RotMat->vec.fvec.xyz.z = cos_theta;
}

//	Compute the outer product of A = A * transpose(A).  1x3 vector becomes 3x3 matrix.
static void vm_vec_outer_product(matrix *mat, const vec3d *vec)
{
	mat->vec.rvec.xyz.x = vec->xyz.x * vec->xyz.x;
	mat->vec.rvec.xyz.y = vec->xyz.x * vec->xyz.y;
	mat->vec.rvec.xyz.z = vec->xyz.x * vec->xyz.z;

	mat->vec.uvec.xyz.x = vec->xyz.y * vec->xyz.x; //-V537
	mat->vec.uvec.xyz.y = vec->xyz.y * vec->xyz.y;
	mat->vec.uvec.xyz.z = vec->xyz.y * vec->xyz.z; //-V537

	mat->vec.fvec.xyz.x = vec->xyz.z * vec->xyz.x;
	mat->vec.fvec.xyz.y = vec->xyz.z * vec->xyz.y; //-V537
	mat->vec.fvec.xyz.z = vec->xyz.z * vec->xyz.z;
}

//	Find the point on the line between p0 and p1 that is nearest to int_pnt.
//	Stuff result in nearest_point.
//	Uses algorithm from page 148 of Strang, Linear Algebra and Its Applications.
//	Returns value indicating whether *nearest_point is between *p0 and *p1.
//	0.0f means *nearest_point is *p0, 1.0f means it's *p1. 2.0f means it's beyond p1 by 2x.
//	-1.0f means it's "before" *p0 by 1x.
float find_nearest_point_on_line(vec3d *nearest_point, const vec3d *p0, const vec3d *p1, const vec3d *int_pnt)
{
	vec3d	norm, xlated_int_pnt, projected_point;
	matrix	mat;
	float		mag, dot;

	vm_vec_sub(&norm, p1, p0);
	vm_vec_sub(&xlated_int_pnt, int_pnt, p0);

	if (IS_VEC_NULL_SQ_SAFE(&norm)) {
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

int find_intersection(float* s, const vec3d* p0, const vec3d* p1, const vec3d* v0, const vec3d* v1)
{
	// Vector v2 forms an edge between v0 and v1, thus forming a triangle.
	// An intersection exists between v0 and v1 if their cross product is parallel with the cross product of v2 and v1
	// The scalar of v0 can then be found by the ratio between the two cross products
	vec3d v2, crossA, crossB;

	vm_vec_sub(&v2, p1, p0);
	vm_vec_cross(&crossA, v0, v1);
	vm_vec_cross(&crossB, &v2, v1);

	if (vm_vec_equal(crossA, vmd_zero_vector)) {
		// Colinear
		return -1;
	}

	if (!vm_test_parallel(&crossA, &crossB)) {
		// The two cross products are not parallel, so no intersection between v0 and v1
		return -2;
	}

	*s = vm_vec_mag(&crossB) / vm_vec_mag(&crossA);
	return 0;
}

void find_point_on_line_nearest_skew_line(vec3d *dest, const vec3d *p1, const vec3d *d1, const vec3d *p2, const vec3d *d2)
{
	vec3d n, n2, pdiff;

	// The cross product of the direction vectors is perpendicular to both lines
	vm_vec_cross(&n, d1, d2);

	// The plane formed by the translations of Line 2 along n contains the point p2 and is perpendicular to n2 = d2 x n
	vm_vec_cross(&n2, d2, &n);

	// So now we find the intersection of Line 1 with that plane, which is apparently this gibberish
	vm_vec_sub(&pdiff, p2, p1);
	float numerator = vm_vec_dot(&pdiff, &n2);
	float denominator = vm_vec_dot(d1, &n2);
	vm_vec_scale_add(dest, p1, d1, numerator / denominator);
}


// normalizes only if above a threshold, returns if normalized or not
bool vm_maybe_normalize(vec3d* dst, const vec3d* src, float threshold) {
	float mag = vm_vec_mag(src);
	if (mag < threshold) return false;
	vm_vec_copy_scale(dst, src, 1 / mag);
	return true;
}

// Produce a vector perpendicular to the normalized input vector unit_normal,
// in the direction preference (if not null). If that direction doesn't work it picks the z or y direction,
// so that an output perpendicular vector is guaranteed.
void vm_orthogonalize_one_vec(vec3d* dst, const vec3d* unit_normal, const vec3d* preference) {
	if (preference != nullptr) {
		vm_vec_projection_onto_plane(dst, preference, unit_normal);
		if (vm_maybe_normalize(dst, dst)) {
			// The process of rescaling dst may have exaggerated floating point inaccuracy
			// so that dst is no longer approximately orthogonal to unit_normal,
			// so project it again.
			if (fabs(vm_vec_dot(dst, unit_normal)) > 1e-4f) {
				vm_vec_projection_onto_plane(dst, dst, unit_normal);
				vm_vec_normalize(dst);
			}
			return;
		}
	}
	vm_vec_projection_onto_plane(dst, &vmd_z_vector, unit_normal);
	if (vm_maybe_normalize(dst, dst)) return;
	vm_vec_projection_onto_plane(dst, &vmd_y_vector, unit_normal);
}

// Produce two vectors perpendicular to each other from two arbitrary vectors src1, src2.
// In the normal case dst1 will point in the direction of src1 and dst2 will be in the plane
// of src1, src2 and perpendicular to dst1, but in the case of degeneracy it tries to
// give useful results. The vector preference is a third vector (which may be null)
// that will be considered in case the first two vectors are zero.
void vm_orthogonalize_two_vec(vec3d* dst1, vec3d* dst2, const vec3d* src1, const vec3d* src2, const vec3d* preference) {
	if (vm_maybe_normalize(dst1, src1))
		vm_orthogonalize_one_vec(dst2, dst1, src2);
	else if (vm_maybe_normalize(dst2, src2))
		vm_orthogonalize_one_vec(dst1, dst2, src1);
	else {
		if (preference == nullptr || !vm_maybe_normalize(dst1, preference))
			vm_vec_make(dst1, 1, 0, 0);
		vm_orthogonalize_one_vec(dst2, dst1, src2);
	}
}

// make sure matrix is orthogonal
// computes a matrix from one or more vectors. The forward vector is required,
// with the other two being optional.  If both up & right vectors are passed,
// the up vector is used.  If only the forward vector is passed, a bank of
// zero is assumed
void vm_orthogonalize_matrix(matrix *m_src)
{
	vec3d fvec, uvec;
	vm_orthogonalize_two_vec(&fvec, &uvec, &m_src->vec.fvec, &m_src->vec.uvec, &m_src->vec.rvec);
	vm_vec_cross(&m_src->vec.rvec, &uvec, &fvec);
	m_src->vec.fvec = fvec;
	m_src->vec.uvec = uvec;
}

// like vm_orthogonalize_matrix(), except that zero vectors can exist within the
// matrix without causing problems.  Valid vectors will be created where needed.
void vm_fix_matrix(matrix *m)
{
	float fmag, umag, rmag;

	fmag = vm_vec_mag(&m->vec.fvec);
	umag = vm_vec_mag(&m->vec.uvec);
	rmag = vm_vec_mag(&m->vec.rvec);
	if (fmag <= 0.0f) {
		if ((umag > 0.0f) && (rmag > 0.0f) && !vm_test_parallel(&m->vec.uvec, &m->vec.rvec)) {
			vm_vec_cross(&m->vec.fvec, &m->vec.uvec, &m->vec.rvec);
			vm_vec_normalize(&m->vec.fvec);

		} else if (umag > 0.0f) {
			if (!m->vec.uvec.xyz.x && !m->vec.uvec.xyz.y && m->vec.uvec.xyz.z)  // z vector
				vm_vec_make(&m->vec.fvec, 1.0f, 0.0f, 0.0f);
			else
				vm_vec_make(&m->vec.fvec, 0.0f, 0.0f, 1.0f);
		}

	} else
		vm_vec_normalize(&m->vec.fvec);

	// we now have a valid and normalized forward vector

	if ((umag <= 0.0f) || vm_test_parallel(&m->vec.fvec, &m->vec.uvec)) {  // no up vector to use..
		if ((rmag <= 0.0f) || vm_test_parallel(&m->vec.fvec, &m->vec.rvec)) {  // no right vector either, so make something up
			if (!m->vec.fvec.xyz.x && m->vec.fvec.xyz.y && !m->vec.fvec.xyz.z)  // vertical vector
				vm_vec_make(&m->vec.uvec, 0.0f, 0.0f, -1.0f);
			else
				vm_vec_make(&m->vec.uvec, 0.0f, 1.0f, 0.0f);

		} else {  // use the right vector to figure up vector
			vm_vec_cross(&m->vec.uvec, &m->vec.fvec, &m->vec.rvec);
			vm_vec_normalize(&m->vec.uvec);
		}

	} else
		vm_vec_normalize(&m->vec.uvec);

	// we now have both valid and normalized forward and up vectors

	vm_vec_cross(&m->vec.rvec, &m->vec.uvec, &m->vec.fvec);
		
	//normalize new perpendicular vector
	vm_vec_normalize(&m->vec.rvec);

	//now recompute up vector, in case it wasn't entirely perpendiclar
	vm_vec_cross(&m->vec.uvec, &m->vec.fvec, &m->vec.rvec);
}

//Rotates the orient matrix by the angles in tangles and then
//makes sure that the matrix is orthogonal.
void vm_rotate_matrix_by_angles( matrix *orient, const angles *tangles )
{
	matrix	rotmat,new_orient;
	vm_angles_2_matrix(&rotmat,tangles);
	vm_matrix_x_matrix(&new_orient,orient,&rotmat);
	*orient = new_orient;
	vm_orthogonalize_matrix(orient);
}

//	dir must be normalized!
float vm_vec_dot_to_point(const vec3d *dir, const vec3d *p1, const vec3d *p2)
{
	vec3d	tvec;

	vm_vec_sub(&tvec, p2, p1);
	// VECMAT-ERROR: NULL VEC3D (p1 == p2)
	vm_vec_normalize_safe(&tvec);

	return vm_vec_dot(dir, &tvec);

}

/////////////////////////////////////////////////////////
//	Given a plane and a point, return the point on the plane closest the the point.
//	Result returned in q.
void compute_point_on_plane(vec3d *q, const plane *planep, const vec3d *p)
{
	float	k;
	vec3d	normal;

	normal.xyz.x = planep->A;
	normal.xyz.y = planep->B;
	normal.xyz.z = planep->C;

	k = (planep->D + vm_vec_dot(&normal, p)) / vm_vec_dot(&normal, &normal);

	vm_vec_scale_add(q, p, &normal, -k);
}

//	Generate a fairly random vector that's normalized.
void vm_vec_rand_vec(vec3d *rvec)
{
	rvec->xyz.x = (frand() - 0.5f) * 2;
	rvec->xyz.y = (frand() - 0.5f) * 2;
	rvec->xyz.z = (frand() - 0.5f) * 2;

	if (IS_VEC_NULL_SQ_SAFE(rvec))
		rvec->xyz.x = 1.0f;

	vm_vec_normalize(rvec);
}

// Given an point "in" rotate it by "angle" around an
// arbritary line defined by a point on the line "line_point" 
// and the normalized line direction, "line_dir"
// Returns the rotated point in "out".
void vm_rot_point_around_line(vec3d *out, const vec3d *in, float angle, const vec3d *line_point, const vec3d *line_dir)
{
	vec3d tmp, tmp1;
	matrix m, r;
	angles ta;

	vm_vector_2_matrix_norm(&m, line_dir, NULL, NULL );

	ta.p = ta.h = 0.0f;
	ta.b = angle;
	vm_angles_2_matrix(&r,&ta);

	vm_vec_sub( &tmp, in, line_point );		// move relative to a point on line
	vm_vec_rotate( &tmp1, &tmp, &m);			// rotate into line's base
	vm_vec_rotate( &tmp, &tmp1, &r);			// rotate around Z
	vm_vec_unrotate( &tmp1, &tmp, &m);			// unrotate out of line's base
	vm_vec_add( out, &tmp1, line_point );	// move back to world coordinates
}

// Given two position vectors, return 0 if the same, else non-zero.
int vm_vec_cmp( const vec3d * a, const vec3d * b )
{
	float diff = vm_vec_dist(a,b);
//mprintf(( "Diff=%.32f\n", diff ));
	if ( diff > 0.005f )
		return 1;
	else
		return 0;
}

// Given two orientation matrices, return 0 if the same, else non-zero.
int vm_matrix_cmp(const matrix * a, const matrix * b)
{
	float tmp1,tmp2,tmp3;
	tmp1 = fl_abs(vm_vec_dot( &a->vec.uvec, &b->vec.uvec ) - 1.0f);
	tmp2 = fl_abs(vm_vec_dot( &a->vec.fvec, &b->vec.fvec ) - 1.0f);
	tmp3 = fl_abs(vm_vec_dot( &a->vec.rvec, &b->vec.rvec ) - 1.0f);
//	mprintf(( "Mat=%.16f, %.16f, %.16f\n", tmp1, tmp2, tmp3 ));
	 
	if ( tmp1 > 0.0000005f ) return 1;
	if ( tmp2 > 0.0000005f ) return 1;
	if ( tmp3 > 0.0000005f ) return 1;
	return 0;
}


// Moves angle 'h' towards 'desired_angle', taking the shortest
// route possible.   It will move a maximum of 'step_size' radians
// each call.   All angles in radians.
float vm_interp_angle( float *h, float desired_angle, float step_size, bool force_front )
{
	float delta;
	float abs_delta;

	if ( desired_angle < 0.0f ) desired_angle += PI2;
	if ( desired_angle > PI2 ) desired_angle -= PI2;

	delta = desired_angle - *h;
	abs_delta = fl_abs(delta);

	if ((force_front) && ((desired_angle > PI) ^ (*h > PI)) ) {
		// turn away from PI
		if ( *h > PI )
			delta = abs_delta;
		else 
			delta = -abs_delta;
	} else {
		if ( abs_delta > PI )	{
			// Go the other way, since it will be shorter.
			if ( delta > 0.0f )	{
				delta = delta - PI2;
			} else {
				delta = PI2 - delta;
			}
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

	return delta;
}

float vm_delta_from_interp_angle( float current_angle, float desired_angle )
{
	float delta;
	if ( desired_angle < 0.0f ) desired_angle += PI2;
	if ( desired_angle > PI2 ) desired_angle -= PI2;

	delta = desired_angle - current_angle;

	if ( fl_abs(delta) > PI )	{
		if ( delta > 0.0f )	{
			delta = delta - PI2;
		} else {
			delta = PI2 - delta;
		}
	}
	return delta;
}

// check a matrix for zero rows and columns
int vm_check_matrix_for_zeros(const matrix *m)
{
	if (!m->vec.fvec.xyz.x && !m->vec.fvec.xyz.y && !m->vec.fvec.xyz.z)
		return 1;
	if (!m->vec.rvec.xyz.x && !m->vec.rvec.xyz.y && !m->vec.rvec.xyz.z)
		return 1;
	if (!m->vec.uvec.xyz.x && !m->vec.uvec.xyz.y && !m->vec.uvec.xyz.z)
		return 1;

	if (!m->vec.fvec.xyz.x && !m->vec.rvec.xyz.x && !m->vec.uvec.xyz.x)
		return 1;
	if (!m->vec.fvec.xyz.y && !m->vec.rvec.xyz.y && !m->vec.uvec.xyz.y)
		return 1;
	if (!m->vec.fvec.xyz.z && !m->vec.rvec.xyz.z && !m->vec.uvec.xyz.z)
		return 1;

	return 0;
}

// see if two vectors are the same
int vm_vec_same(const vec3d *v1, const vec3d *v2)
{
	if ( v1->xyz.x == v2->xyz.x && v1->xyz.y == v2->xyz.y && v1->xyz.z == v2->xyz.z )
		return 1;

	return 0;
}

// see if two matrices are the same
int vm_matrix_same(matrix *m1, matrix *m2)
{
	int i;
	for (i = 0; i < 9; i++)
		if (m1->a1d[i] != m2->a1d[i])
			return 0;

	return 1;
}


// --------------------------------------------------------------------------------------

void vm_quaternion_rotate(matrix *M, float theta, const vec3d *u)
//  given an arbitrary rotation axis and rotation angle, function generates the
//  corresponding rotation matrix
//
//  M is the return rotation matrix  theta is the angle of rotation 
//  u is the direction of the axis.
//  this is adapted from Computer Graphics (Hearn and Bker 2nd ed.) p. 420
//
{
	float a,b,c, s;
	float sin_theta = sinf(theta * 0.5f);

	a = (u->xyz.x * sin_theta);
	b = (u->xyz.y * sin_theta);
	c = (u->xyz.z * sin_theta);
	s = cosf(theta * 0.5f);

// 1st ROW vector
	M->vec.rvec.xyz.x = 1.0f - 2.0f*b*b - 2.0f*c*c;
	M->vec.rvec.xyz.y = 2.0f*a*b + 2.0f*s*c;
	M->vec.rvec.xyz.z = 2.0f*a*c - 2.0f*s*b;
// 2nd ROW vector
	M->vec.uvec.xyz.x = 2.0f*a*b - 2.0f*s*c;
	M->vec.uvec.xyz.y = 1.0f - 2.0f*a*a - 2.0f*c*c;
	M->vec.uvec.xyz.z = 2.0f*b*c + 2.0f*s*a;
// 3rd ROW vector
	M->vec.fvec.xyz.x = 2.0f*a*c + 2.0f*s*b;
	M->vec.fvec.xyz.y = 2.0f*b*c - 2.0f*s*a;
	M->vec.fvec.xyz.z = 1.0f - 2.0f*a*a - 2.0f*b*b;
}

// --------------------------------------------------------------------------------------

//void vm_matrix_to_rot_axis_and_angle(matrix *m, float *theta, vec3d *rot_axis)
// Converts a matrix into a rotation axis and an angle around that axis
// Note for angle is very near 0, returns 0 with axis of (1,0,0)
// For angles near PI, returns PI with correct axis
//
// rot_axis - the resultant axis of rotation
// theta - the resultatn rotation around the axis
// m - the initial matrix
void vm_matrix_to_rot_axis_and_angle(const matrix *m, float *theta, vec3d *rot_axis)
{
	float trace = m->a2d[0][0] + m->a2d[1][1] + m->a2d[2][2];
	float cos_theta = 0.5f * (trace - 1.0f);

	if (cos_theta > 0.999999875f) { // angle is less than 1 milirad (0.057 degrees)
		*theta = 0.0f;

		vm_vec_make(rot_axis, 1.0f, 0.0f, 0.0f);
	} else if (cos_theta > -0.999999875f) { // angle is within limits between 0 and PI
		*theta = acosf_safe(cos_theta);
		Assert( !fl_is_nan(*theta) );

		rot_axis->xyz.x = (m->vec.uvec.xyz.z - m->vec.fvec.xyz.y);
		rot_axis->xyz.y = (m->vec.fvec.xyz.x - m->vec.rvec.xyz.z);
		rot_axis->xyz.z = (m->vec.rvec.xyz.y - m->vec.uvec.xyz.x);
		if (IS_VEC_NULL_SQ_SAFE(rot_axis)) {
			vm_vec_make(rot_axis, 1.0f, 0.0f, 0.0f);
		} else {
			vm_vec_normalize(rot_axis);
		}
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

			rot_axis->xyz.x = fl_sqrt(m->a2d[0][0] + 1.0f);
			ix = 1.0f / rot_axis->xyz.x;
			rot_axis->xyz.y = m->a2d[0][1] * ix;
			rot_axis->xyz.z = m->a2d[0][2] * ix;
			break;

		case 1:
			float iy;

			rot_axis->xyz.y = fl_sqrt(m->a2d[1][1] + 1.0f);
			iy = 1.0f / rot_axis->xyz.y;
			rot_axis->xyz.x = m->a2d[1][0] * iy;
			rot_axis->xyz.z = m->a2d[1][2] * iy;
			break;

		case 2:
			float iz;

			rot_axis->xyz.z = fl_sqrt(m->a2d[2][2] + 1.0f);
			iz = 1.0f / rot_axis->xyz.z;
			rot_axis->xyz.x = m->a2d[2][0] * iz;
			rot_axis->xyz.y = m->a2d[2][1] * iz;
			break;

		default:
			Int3();  // this should never happen
			break;
		}

		// normalize rotation axis
		vm_vec_normalize(rot_axis);
	}
}

// Given a rotation axis, calculates the angle that results in the rotation closest to the given matrix m.
// If the axis is equal or very close to the orientation of the matrix, returns false and an angle of 0
float vm_closest_angle_to_matrix(const matrix* mat, const vec3d* rot_axis, float* angle){
	// The relative rotation between m and the target rotation r (made from axis a and angle x) is m^T.r
	// The resulting angle between those, as shown by http://www.boris-belousov.net/2016/12/01/quat-dist/ is arccos((tr(m^T.r)-1) / 2)

	// tr(m^T.r) simplifies to the following:
	// tr = m[0]+m[4]+m[8] - 2( m[0]*(a[1]^2+a[2]^2) + m[4]*(a[0]^2+a[2]^2) + m[8]*(a[0]^2+a[1]^2) -
	//                          a[0]*a[1]*(m[1]+m[3]) - a[0]*a[2]*(m[2]+m[6]) - a[1]*a[2]*(m[5]+m[7])) * sin(1/2 * x)^2
	//					   + (a[0]*(m[5]-m[7]) + a[1]*(-m[2]+m[6]) + a[2]*(m[1]-m[3])) * sin(x)

	// The factor before the sine squared will be calculated as y, the factor before the sine as z, the summand as w:

	const auto& m = mat->a1d;
	const auto& a = rot_axis->a1d;

	const float w = m[0]+m[4]+m[8];
	const float y = -2 * ( m[0]*(a[1]*a[1]+a[2]*a[2]) + m[4]*(a[0]*a[0]+a[2]*a[2]) + m[8]*(a[0]*a[0]+a[1]*a[1]) -
			      a[0]*a[1]*(m[1]+m[3]) - a[0]*a[2]*(m[2]+m[6]) - a[1]*a[2]*(m[5]+m[7]));
	const float z = (a[0]*(m[5]-m[7]) + a[1]*(-m[2]+m[6]) + a[2]*(m[1]-m[3]));

	// If both y and z are close to 0, then the rotation axis points in the same direction as the matrix, thus any orientation r would be perpendicular to m
	// If y is 0, the rest of the math simplifies, and we always find the angle at pi/2
	if(fabs(y) < 0.001f) {
		if (fabs(z) < 0.001f) {
			*angle = 0.0f;
			return PI_2;
		}

		*angle = PI_2;
		return acosf_safe((w + z - 1.0f) * 0.5f);
	}

	// arccos((x-1)/2) is then minimal, when x between -1 and 3 approaches 3
	// Thus we are looking for the maximum of a term in the form of f(x)=w+y*sin(x/2)^2+z*sin(x)
	// This maximum can be on one of the four solutions of f'(x)=0, not counting periodic repetitions

	const float sr = sqrtf(y*y*y*y+4*y*y*z*z);
	const float sr_pos = sqrtf(1-(sr/(y*y+4*z*z)));
	const float sr_neg = sqrtf(1+(sr/(y*y+4*z*z)));

	//If we support IEEE float handling, we don't need this, the div by 0 will be handled correctly with the INF. If not, do this:
	const float yz_recip = (!std::numeric_limits<float>::is_iec559 && y*z < 0.001f) ? FLT_MAX : 1.0f / (y * z);

	const float solutions[] = {2 * atan2_safe(-2 * sr_neg, -sr_neg*(y*y+sr) * yz_recip),
							   2 * atan2_safe(2 * sr_neg, sr_neg*(y*y+sr) * yz_recip),
							   2 * atan2_safe(-2 * sr_pos, -sr_pos*(y*y-sr) * yz_recip),
							   2 * atan2_safe(2 * sr_pos, sr_pos*(y*y-sr) * yz_recip)};

	float value = -2.0f;
	float correct = 0;
	//For whichever of these, w+y*sin(x/2)^2+z*sin(x) is closest to 3 / larger (since the result is between -1 and 3) is our target angle
	for(float solution : solutions){
		float currentVal = w + y * sinf(solution * 0.5f) * sinf(solution * 0.5f) + z * sinf(solution);
		if(currentVal > value){
			value = currentVal;
			correct = solution;
		}
	}

	Assertion(value > -1.5f, "Did not find solution for closest angle & axis to matrix.");

	// Since atanf can yield -pi/2 to pi/2, we could get negative results here. Convert to 0 to 2Pi
	while (correct < 0.0f)
		correct += PI2;

	*angle = correct;
	return acosf_safe((value - 1.0f) * 0.5f);
}


// --------------------------------------------------------------------------------------


void get_camera_limits(const matrix *start_camera, const matrix *end_camera, float time, vec3d *acc_max, vec3d *w_max)
{
	matrix temp, rot_matrix;
	float theta;
	vec3d rot_axis;
	vec3d angle;

	// determine the necessary rotation matrix
	vm_copy_transpose(&temp, start_camera);
	vm_matrix_x_matrix(&rot_matrix, &temp, end_camera);
	vm_orthogonalize_matrix(&rot_matrix);

	// determine the rotation axis and angle
	vm_matrix_to_rot_axis_and_angle(&rot_matrix, &theta, &rot_axis);

	// find the rotation about each axis
	angle.xyz.x = theta * rot_axis.xyz.x;
	angle.xyz.y = theta * rot_axis.xyz.y;
	angle.xyz.z = theta * rot_axis.xyz.z;

	// allow for 0 time input
	if (time <= 1e-5f) {
		vm_vec_make(acc_max, 0.0f, 0.0f, 0.0f);
		vm_vec_make(w_max, 0.0f, 0.0f, 0.0f);
	} else {

		// find acceleration limit using  (theta/2) takes (time/2)
		// and using const accel  theta = 1/2 acc * time^2
		acc_max->xyz.x = 4.0f * fl_abs(angle.xyz.x) / (time * time);
		acc_max->xyz.y = 4.0f * fl_abs(angle.xyz.y) / (time * time);
		acc_max->xyz.z = 4.0f * fl_abs(angle.xyz.z) / (time * time);

		// find angular velocity limits
		// w_max = acc_max * time / 2
		w_max->xyz.x = acc_max->xyz.x * time / 2.0f;
		w_max->xyz.y = acc_max->xyz.y * time / 2.0f;
		w_max->xyz.z = acc_max->xyz.z * time / 2.0f;
	}
}

#define OVERSHOOT_PREVENTION_PADDING 0.98f
// physically models within the frame the physical behavior to get to a goal position
// given an arbitrary initial velocity
void vm_angular_move_1dimension_calc(float goal, float* vel, float delta_t,
	float* dist, float vel_limit, float acc_limit)
{
	// These diagrams depict two common situations, and although they both start with negative velocity
	// for illustrative purposes, it is not necessary and the apex may be at the present or even in the past.
	//
	// t1 < t2 means there is a straight segment, so we coast for some time
	// 
	//                    _..--- goal
	//  now            .''
	//   |           .' |
	//   v         .'   |
	//   .       .'    t_straight
	//    ''-.-''|
	//     apex  t1 (= t_up)
	// 
	//  t1 > t2, no straight segment, accelerate then deccelerate, no coasting
	//
	//  now           _..--- goal
	//   |         .'
	//   v        . 
	//   .       .|   
	//    ''-.-'' |
	//     apex    t2 (= t_up)
	//     

	float t1 = (vel_limit - *vel) / acc_limit;  // time to accelerate from the current velocity (possibly negative) to +vel_limit
	float apex_t = -*vel / acc_limit;           // the time when we had / will have velocity zero, assuming acceleration at +acc_limit
	float apex = *vel * apex_t / 2;             // the position we had / will have at the apex
	float switchover_point = OVERSHOOT_PREVENTION_PADDING / (OVERSHOOT_PREVENTION_PADDING + 1.f); // when on the path 
	                                                            // we switch from accelerating to deccelerating (very close to 1/2)
	float half_dist = (goal - *dist - apex) * switchover_point; // half the distance from apex to goal (where we hit peak velocity)
	float t2 = apex_t + fl_sqrt(2 * half_dist / acc_limit);  // The time at which we reach half_dist, assuming we never hit vel_limit
	float t_up = fmin(delta_t, fmin(t1, t2));                // We exit the initial upward curve when we either hit vel_limit (t1)
	                                                         // or we start the approach to the goal (t2), so t_up is the min
	
	// add distance and vel for t_up
	*dist += *vel * t_up + acc_limit * t_up * t_up / 2;
	*vel += acc_limit * t_up;
	// If we have run out of time in the frame, break, else advance by t_up
	if (delta_t <= t_up) return;
	delta_t -= t_up;

	// If t1 <= t2 then we have a straight segment (cruising at +vel_limit)
	if (t1 <= t2) {
		// time it takes to reach the approach
		float t_straight = fmin(delta_t, (goal - 0.5f * vel_limit * vel_limit / acc_limit - *dist) / vel_limit);
		// add distance and vel
		*dist += vel_limit * t_straight;
		// If we have run out of time in the frame, break, else advance by t_straight
		if (delta_t <= t_straight) return;
		delta_t -= t_straight;
	}

	// On approach to the goal, with acceleration -acc_limit
	// Our current velocity is either vel_limit if we had a straight segment, or the peak velocity at half_dist
	// slow down our acc very slightly to avoid possible time costly overshoot
	acc_limit *= OVERSHOOT_PREVENTION_PADDING;
	// t_down is the time to slow to a stop
	float t_down = fmin(delta_t, *vel / acc_limit);
	// add distance and vel for t_down
	*dist += *vel * t_down - acc_limit * t_down * t_down / 2;
	*vel -= acc_limit * t_down;
	// If we have run out of time in the frame, break, else advance by t_down
	if (delta_t <= t_down) return;
	
	// We've arrived
	*dist = goal;
	*vel = 0;
}

// physically models within the frame the physical behavior to get to a one-dimensional goal position
// given an arbitrary initial velocity
float vm_angular_move_1dimension(float goal, float delta_t, float* vel, float vel_limit, float acc_limit, float slowdown_factor, bool force_no_overshoot)
{
	float effective_vel_limit = slowdown_factor == 0 ? 0 : slowdown_factor * vel_limit;
	float effective_acc_limit = slowdown_factor == 0 ? 0 : slowdown_factor * acc_limit;
	if (acc_limit <= 0) return *vel * delta_t;		// Can't accelerate? No point in continuing!
	float dist = 0;
	float t_slow = fmin(delta_t, (fabs(*vel) - effective_vel_limit) / acc_limit);  // Time until we get down to our max speed
	if (t_slow > 0) {                                                              // If that's zero (were at max) or negative (below max)
		float acc = *vel >= 0 ? -acc_limit : acc_limit;                            // excellent, but otherwise, there's no choices to make
		dist += *vel * t_slow + acc * t_slow * t_slow / 2;                         // slam on the brakes and continue only if there was enough
		*vel += acc * t_slow;                                                      // time in the frame to get down to max
		if (delta_t <= t_slow) return dist;
		delta_t -= t_slow;
	}
	if (effective_vel_limit <= 0 || effective_acc_limit <= 0) return dist + *vel * delta_t;		// Can't move (from slowdown factor or otherwise)? Also no point in continuing!

	
	float goal_trajectory_speed = fl_sqrt(2.0f * acc_limit * fabsf(goal));
	bool should_acc_upwards = goal >= 0 ? *vel < goal_trajectory_speed : *vel < -goal_trajectory_speed;
	// This makes sure that the initial acceleration is always positive
	// If the goal is above, or if it's below but our vel will put it above us before we can slow down enough, we're good
	if (should_acc_upwards) {
		if (goal < 0 && force_no_overshoot)
			*vel = -goal_trajectory_speed; // With no overshoot our input velocity is always to set to the perfect trajectory
		vm_angular_move_1dimension_calc(goal, vel, delta_t, &dist, effective_vel_limit, effective_acc_limit);

	}
	else { // else flip it so our goal is above and again we get initial positive accel
		if (goal > 0 && force_no_overshoot)
			*vel = goal_trajectory_speed; // With no overshoot our input velocity is always to set to the perfect trajectory
		*vel = -*vel, dist = -dist; 
		vm_angular_move_1dimension_calc(-goal, vel, delta_t, &dist, effective_vel_limit, effective_acc_limit);
		*vel = -*vel, dist = -dist;
	}
	return dist;
}

float time_to_arrival_calc(float goal, float vel, float vel_limit, float acc_limit) {
	float t1 = (vel_limit - vel) / acc_limit;    // time to accelerate from the current velocity (possibly negative) to +vel_limit
	float apex_t = -vel / acc_limit;             // the time when we had / will have velocity zero, assuming acceleration at +acc_limit
	float apex = vel * apex_t / 2;               // the position we had / will have at the apex
	float half_dist = (goal - apex) / 2;                     // half the distance from apex to goal (where we hit peak velocity)
	float t2 = apex_t + fl_sqrt(2 * half_dist / acc_limit);  // The time at which we reach half_dist, assuming we never hit vel_limit
	
	float time; // accumulated time to arrival

	// If t1 <= t2 then we have a straight segment (cruising at +vel_limit)
	if (t1 <= t2) {
		float dist = vel * t1 + acc_limit * t1 * t1 / 2;  // at the end of the upward bend we are at dist
		vel = vel_limit;                                  // and we reach velocity vel_limit
		// and the time at the start of the approach is t1 + t_straight
		time = t1 + (goal - 0.5f * vel_limit * vel_limit / acc_limit - dist) / vel_limit;
	}
	else {
		// If t2 < t1 then there is no straight segment, we just accelerate until the approach
		// so time = t2 and vel is however much we can accelerate in that time
		time = t2;
		vel += acc_limit * t2;
	}
	// The total time is the time to the approach + the deceleration time
	return time + vel / acc_limit;
}

// called by vm_angular_move to compute a slowing factor
// pared down versions of the 1dimension functions
float time_to_arrival(float goal, float vel, float vel_limit, float acc_limit) {
	// We won't consider speeds above our max, the time estimate gets complicated and the result won't be a straight line anyway
	if (fabs(vel) > vel_limit) {
		vel = vel > 0 ? vel_limit : -vel_limit;
	}
	return (vel < (goal >= 0 ? fl_sqrt(2.0f * acc_limit * goal) : -fl_sqrt(2.0f * acc_limit * -goal))) // same thing as scalar interpolate
		? time_to_arrival_calc(goal, vel, vel_limit, acc_limit)
		: time_to_arrival_calc(-goal, -vel, vel_limit, acc_limit);
}

// splits up the accelerating/deccelerating/go to position function for each component
// and also scales their speed to make a nice straight line
// note that this is now treated as a movement in linear space, despite the name
vec3d vm_angular_move(const vec3d* goal, float delta_t,
	vec3d* vel, const vec3d* vel_limit, const vec3d* acc_limit, bool aggressive_bank, bool force_no_overshoot, bool no_directional_bias)
{
	vec3d ret, slow;
	vm_vec_make(&slow, 1.f, 1.f, 1.f);
	if (no_directional_bias) {
		// first, the estimated time to arrive at the goal angular position is calculated for each component
		slow.xyz.x = time_to_arrival(goal->xyz.x, vel->xyz.x, vel_limit->xyz.x, acc_limit->xyz.x);
		slow.xyz.y = time_to_arrival(goal->xyz.y, vel->xyz.y, vel_limit->xyz.y, acc_limit->xyz.y);
		slow.xyz.z = time_to_arrival(goal->xyz.z, vel->xyz.z, vel_limit->xyz.z, acc_limit->xyz.z);

		// then, compute a slowing factor for the 1 or 2 faster-to-arrive-at-their-destination components
		// so they arrive at approximately the same time as the slowest component, so the path there is nice and straight
		float max = fmax(slow.xyz.x, fmax(slow.xyz.y, slow.xyz.z));
		if (max != 0) vm_vec_scale(&slow, 1 / max);
		if (aggressive_bank) slow.xyz.z = 1.f;
	}

	ret.xyz.x = vm_angular_move_1dimension(goal->xyz.x, delta_t, &vel->xyz.x, vel_limit->xyz.x, acc_limit->xyz.x, slow.xyz.x, force_no_overshoot);
	ret.xyz.y = vm_angular_move_1dimension(goal->xyz.y, delta_t, &vel->xyz.y, vel_limit->xyz.y, acc_limit->xyz.y, slow.xyz.y, force_no_overshoot);
	ret.xyz.z = vm_angular_move_1dimension(goal->xyz.z, delta_t, &vel->xyz.z, vel_limit->xyz.z, acc_limit->xyz.z, slow.xyz.z, force_no_overshoot);
	return ret;
}

// ---------------------------------------------------------------------------------------------
//
//		inputs:		goal_orient	=>		goal orientation matrix
//					curr_orient	=>		current orientation matrix
//					w_in		=>		current input angular velocity
//					delta_t		=>		time to move toward goal
//					next_orient	=>		the orientation matrix at time delta_t (with current forward vector)
//					w_out		=>		the angular velocity of the ship at delta_t
//					vel_limit	=>		maximum rotational speed
//					acc_limit	=>		maximum rotational acceleration
//					no_directional_bias  => will cause the angular path generated to be as straight as possible, rather than greedily
//											turning at maximum on all axes (and thus possibly produce a 'crooked' path)
//					force_no_overshoot   => forces the interpolation to not overshoot, if it is approaching its goal too fast
//											it will always arrive with 0 velocity, even if its acceleration would not normally 
//											allow it slow down in time
//		
//		Asteroth - this replaced retail's "vm_matrix_interpolate" in PR 2668.
//		The produced behavior is on average 0.52% slower (std dev 0.74%) than the retail function 
//		Roughly twice that if framerate_independent_turning is enabled.
//
//		The function attempts to rotate the input matrix into the goal matrix taking account of anglular
//		momentum (velocity)
void vm_angular_move_matrix(const matrix* goal_orient, const matrix* curr_orient, const vec3d* w_in, float delta_t,
	matrix* next_orient, vec3d* w_out, const vec3d* vel_limit, const vec3d* acc_limit, bool no_directional_bias, bool force_no_overshoot)
{
	//	Find rotation needed for goal
	// goal_orient = R curr_orient,  so R = goal_orient curr_orient^-1
	matrix Mtemp1;
	vm_copy_transpose(&Mtemp1, curr_orient);				// Mtemp1 = curr ^-1
	matrix rot_matrix;		// rotation matrix from curr_orient to goal_orient
	vm_matrix_x_matrix(&rot_matrix, &Mtemp1, goal_orient);	// R = goal * Mtemp1
	vm_orthogonalize_matrix(&rot_matrix);
	vec3d rot_axis;			// vector indicating direction of rotation axis
	float theta;				// magnitude of rotation about the rotation axis
	vm_matrix_to_rot_axis_and_angle(&rot_matrix, &theta, &rot_axis);		// determines angle and rotation axis from curr to goal

	// find theta to goal
	vec3d theta_goal;		// desired angular position at the end of the time interval
	vm_vec_copy_scale(&theta_goal, &rot_axis, theta);

	// continue to interpolate, unless we are at the goal with no velocity, in which case we have arrived
	if (theta < SMALL_NUM && vm_vec_mag_squared(w_in) < SMALL_NUM * SMALL_NUM) {
		*next_orient = *goal_orient;
		vm_vec_zero(w_out);
		return;
	}

	// calculate best approach in linear space (returns velocity in w_out and position difference in rot_axis)
	*w_out = *w_in;
	rot_axis = vm_angular_move(&theta_goal, delta_t, w_out, vel_limit, acc_limit, false, force_no_overshoot, no_directional_bias);

	// arrived at goal? (equality comparison is okay here because vm_vector_interpolate returns theta_goal on arrival)
	if (rot_axis == theta_goal) {
		*next_orient = *goal_orient;
		// rotate velocity out to reflect new local frame
		vec3d vtemp = *w_out;
		vm_vec_rotate(w_out, &vtemp, &rot_matrix);
		return;
	}

	//	normalize rotation axis and determine total rotation angle
	theta = vm_vec_mag(&rot_axis);
	if (theta > SMALL_NUM)
		vm_vec_scale(&rot_axis, 1 / theta);

	// if the positional change is small, reuse orient (and return because velocity is already set)
	if (theta < SMALL_NUM) {
		*next_orient = *curr_orient;
		return;
	}

	// otherwise rotate orient by theta along rot_axis
	vm_quaternion_rotate(&Mtemp1, theta, &rot_axis);
	Assert(is_valid_matrix(&Mtemp1));
	vm_matrix_x_matrix(next_orient, curr_orient, &Mtemp1);
	vm_orthogonalize_matrix(next_orient);
	// and rotate velocity out to reflect new local frame
	vec3d vtemp = *w_out;
	vm_vec_rotate(w_out, &vtemp, &Mtemp1);
}


// ---------------------------------------------------------------------------------------------
//
//		inputs:		goal_f		=>		goal forward vector
//					orient		=>		current orientation matrix (with current forward vector)
//					w_in		=>		current input angular velocity
//					delta_t		=>		this frametime
//					delta_bank	=>		desired change in bank in degrees
//					next_orient	=>		the orientation matrix at time delta_t (with current forward vector)
//					w_out		=>		the angular velocity of the ship at delta_t
//					vel_limit	=>		maximum rotational speed
//					acc_limit	=>		maximum rotational acceleration
//					no_directional_bias  => will cause the angular path generated to be as straight as possible, rather than greedily
//											turning at maximum on all axes (and thus possibly produce a 'crooked' path)
//
//		Asteroth - this replaced retail's "vm_forward_interpolate" in PR 2668.
//		The produced behavior is on average 0.06% slower (std dev 0.32%) than the retail function 
//		Roughly twice that if framerate_independent_turning is enabled, or if the object is a missile.
//
//		function attempts to rotate the forward vector toward the goal forward vector taking account of anglular
//		momentum (velocity)  Attempt to try to move bank by goal delta_bank. 
//		called "vm_forward_interpolate" in retail 
void vm_angular_move_forward_vec(const vec3d* goal_f, const matrix* orient, const vec3d* w_in, float delta_t, float delta_bank,
	matrix* next_orient, vec3d* w_out, const vec3d* vel_limit, const vec3d* acc_limit, bool no_directional_bias)
{
	vec3d rot_axis;
	vm_vec_cross(&rot_axis, &orient->vec.fvec, goal_f); // Get the direction to rotate to the goal
	float cos_theta = vm_vec_dot(&orient->vec.fvec, goal_f);  // Get cos(theta) where theta is the amount to rotate
	float sin_theta = fmin(vm_vec_mag(&rot_axis), 1.0f);      // Get sin(theta) (cap at 1 for floating point errors)
	vec3d theta_goal;
	vm_vec_make(&theta_goal, 0, 0, delta_bank);         // theta_goal will contain the radians to rotate (in the same direction as rot_axis but in local coords)

	if (sin_theta <= SMALL_NUM) { // sin(theta) is small so we are either very close or very far
		if (cos_theta < 0) { // cos(theta) < 0, sin(theta) ~ 0 means we are pointed exactly the opposite way
			float w_mag_sq = w_in->xyz.x * w_in->xyz.x + w_in->xyz.y * w_in->xyz.y;
			if (w_mag_sq <= SMALL_NUM * SMALL_NUM) { // if we have ~ no angular velocity
				theta_goal.xyz.x = PI; // Rotate in x direction (arbitrarily)
			}
			else { // otherwise prefer to rotate in the direction of angular velocity
				float d = PI / fl_sqrt(w_mag_sq);
				theta_goal.xyz.x = w_in->xyz.x * d;
				theta_goal.xyz.y = w_in->xyz.y * d;
			}
		}
		// continue to interpolate, unless we also have no velocity, in which case we have arrived
		else if (vm_vec_mag_squared(w_in) < SMALL_NUM * SMALL_NUM) {
			*next_orient = *orient;
			vm_vec_zero(w_out);
			return;
		}
	}
	else {
		vec3d local_rot_axis;
		// rotate rot_axis into ship reference frame
		vm_vec_rotate(&local_rot_axis, &rot_axis, orient);

		// derive theta from sin(theta) for better accuracy
		vm_vec_copy_scale(&theta_goal, &local_rot_axis, (cos_theta > 0 ? asinf_safe(sin_theta) : PI - asinf_safe(sin_theta)) / sin_theta);
		
		// reset z to delta_bank, because it just got cleared
		theta_goal.xyz.z = delta_bank;
	}

	// calculate best approach in linear space (returns velocity in w_out and position difference in rot_axis)
	*w_out = *w_in;
	rot_axis = vm_angular_move(&theta_goal, delta_t, w_out, vel_limit, acc_limit, true, false, no_directional_bias);

	//	normalize rotation axis and determine total rotation angle
	float theta = vm_vec_mag(&rot_axis);
	if (theta > SMALL_NUM)
		vm_vec_scale(&rot_axis, 1 / theta);

	// if the positional change is small, reuse orient (and return because velocity is already set)
	if (theta < SMALL_NUM) {
		*next_orient = *orient;
		return;
	}

	// otherwise rotate orient by theta along rot_axis
	matrix Mtemp1;
	vm_quaternion_rotate(&Mtemp1, theta, &rot_axis);
	vm_matrix_x_matrix(next_orient, orient, &Mtemp1);
	Assert(is_valid_matrix(next_orient));
	// and rotate velocity out to reflect new local frame
	vec3d vtemp = *w_out;
	vm_vec_rotate(w_out, &vtemp, &Mtemp1);
}




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
void vm_find_bounding_sphere(const vec3d *pnts, int num_pnts, vec3d *center, float *radius)
{
	int		i;
	float		rad, rad_sq, xspan, yspan, zspan, maxspan;
	float		old_to_p, old_to_p_sq, old_to_new;	
	vec3d	diff, xmin, xmax, ymin, ymax, zmin, zmax, dia1, dia2;
	const vec3d *p;
	
	xmin = vmd_zero_vector;
	ymin = vmd_zero_vector;
	zmin = vmd_zero_vector;
	xmax = vmd_zero_vector;
	ymax = vmd_zero_vector;
	zmax = vmd_zero_vector;	
	xmin.xyz.x = ymin.xyz.y = zmin.xyz.z = BIGNUMBER;
	xmax.xyz.x = ymax.xyz.y = zmax.xyz.z = -BIGNUMBER;

	for ( i = 0; i < num_pnts; i++ ) {
		p = &pnts[i];
		if ( p->xyz.x < xmin.xyz.x )
			xmin = *p;
		if ( p->xyz.x > xmax.xyz.x )
			xmax = *p;
		if ( p->xyz.y < ymin.xyz.y )
			ymin = *p;
		if ( p->xyz.y > ymax.xyz.y )
			ymax = *p;
		if ( p->xyz.z < zmin.xyz.z )
			zmin = *p;
		if ( p->xyz.z > zmax.xyz.z )
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
		maxspan = zspan;
		dia1 = zmin;
		dia2 = zmax;
	}

	// calc initial center
	vm_vec_add(center, &dia1, &dia2);
	vm_vec_scale(center, 0.5f);

	vm_vec_sub(&diff, &dia2, center);
	rad_sq = vm_vec_mag_squared(&diff);
	rad = fl_sqrt(rad_sq);
	Assert( !fl_is_nan(rad) );

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
			center->xyz.x = (rad*center->xyz.x + old_to_new*p->xyz.x) / old_to_p;
			center->xyz.y = (rad*center->xyz.y + old_to_new*p->xyz.y) / old_to_p;
			center->xyz.z = (rad*center->xyz.z + old_to_new*p->xyz.z) / old_to_p;
			nprintf(("Alan", "New sphere: cen,rad = %f %f %f  %f\n", center->xyz.x, center->xyz.y, center->xyz.z, rad));
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
vec3d* vm_rotate_vec_to_body(vec3d *body_vec, const vec3d *world_vec, const matrix *orient)
{
	return vm_vec_unrotate(body_vec, world_vec, orient);
}


// ----------------------------------------------------------------------------
// vm_rotate_vec_to_world()
//
// rotates a vector from body coordinates to world coordinates
//
//	inputs		world_vec	=>		vector in world coordinates
//				body_vec	=>		vector in body coordinates
//				orient		=>		orientation matrix
//
vec3d* vm_rotate_vec_to_world(vec3d *world_vec, const vec3d *body_vec, const matrix *orient)
{
	return vm_vec_rotate(world_vec, body_vec, orient);
}


// ----------------------------------------------------------------------------
// vm_estimate_next_orientation()
//
// given a last orientation and current orientation, estimate the next orientation
//
//	inputs:		last_orient		=>		last orientation matrix
//				current_orient	=>		current orientation matrix
//				next_orient		=>		next orientation matrix		[the result]
//
void vm_estimate_next_orientation(const matrix *last_orient, const matrix *current_orient, matrix *next_orient)
{
	//		R L = C		=>		R = C (L)^-1
	//		N = R C		=>		N = C (L)^-1 C

	matrix Mtemp;
	matrix Rot_matrix;
	vm_copy_transpose(&Mtemp, last_orient);				// Mtemp = (L)^-1
	vm_matrix_x_matrix(&Rot_matrix, &Mtemp, current_orient);	// R = C Mtemp1
	vm_matrix_x_matrix(next_orient, current_orient, &Rot_matrix);
}

//	Return true if all elements of *vec are legal, that is, not NaN or infinity.
bool is_valid_vec(const vec3d *vec)
{
	return !std::isnan(vec->xyz.x) && !std::isnan(vec->xyz.y) && !std::isnan(vec->xyz.z)
		&& !std::isinf(vec->xyz.x) && !std::isinf(vec->xyz.y) && !std::isinf(vec->xyz.z);
}

//	Return true if all elements of *m are legal, that is, not a NAN.
bool is_valid_matrix(const matrix *m)
{
	return is_valid_vec(&m->vec.fvec) && is_valid_vec(&m->vec.uvec) && is_valid_vec(&m->vec.rvec);
}

// interpolate between 2 vectors. t goes from 0.0 to 1.0. at
void vm_vec_interp_constant(vec3d *out, const vec3d *v0, const vec3d *v1, float t)
{
	vec3d cross;
	float total_ang;

	// get the cross-product of the 2 vectors
	vm_vec_cross(&cross, v0, v1);
	vm_vec_normalize(&cross);

	// get the total angle between the 2 vectors
	total_ang = -(acosf_safe(vm_vec_dot(v0, v1)));

	// rotate around the cross product vector by the appropriate angle
	vm_rot_point_around_line(out, v0, t * total_ang, &vmd_zero_vector, &cross);
}

// randomly perturb a vector around a given (normalized vector) or optional orientation matrix
void vm_vec_random_cone(vec3d *out, const vec3d *in, float max_angle, const matrix *orient)
{
	vec3d temp;
	const matrix *rot;
	matrix m;

	// get an orientation matrix
	if(orient != NULL){
		rot = orient;
	} else {
		vm_vector_2_matrix(&m, in, NULL, NULL);
		rot = &m;
	}

	// Get properly distributed spherical coordinates (DahBlount)
	float z = util::UniformFloatRange(cosf(fl_radians(max_angle)), 1.0f).next(); // Take a 2-sphere slice
	float phi = util::UniformFloatRange(0.0f, PI2).next();
	vm_vec_make( &temp, sqrtf(1.0f - z*z)*cosf(phi), sqrtf(1.0f - z*z)*sinf(phi), z ); // Using the z-vec as the starting point

	vm_vec_unrotate(out, &temp, rot); // We find the final vector by rotating temp to the correct orientation
}

void vm_vec_random_cone(vec3d *out, const vec3d *in, float min_angle, float max_angle, const matrix *orient){
	vec3d temp;
	const matrix *rot;
	matrix m;

	if (max_angle < min_angle) {
		auto tmp  = min_angle;
		min_angle = max_angle;
		max_angle = tmp;
	}

	// get an orientation matrix
	if(orient != NULL){
		rot = orient;
	} else {
		vm_vector_2_matrix(&m, in, NULL, NULL);
		rot = &m;
	}
	
	// Get properly distributed spherical coordinates (DahBlount)
	// This might not seem intuitive, but the min_angle is the angle that will have a larger z coordinate
	float z = util::UniformFloatRange(cosf(fl_radians(max_angle)), cosf(fl_radians(min_angle))).next(); // Take a 2-sphere slice
	float phi = util::UniformFloatRange(0.0f, PI2).next();
	vm_vec_make( &temp, sqrtf(1.0f - z*z)*cosf(phi), sqrtf(1.0f - z*z)*sinf(phi), z ); // Using the z-vec as the starting point

	vm_vec_unrotate(out, &temp, rot); // We find the final vector by rotating temp to the correct orientation
}


// given a start vector, an orientation, and a radius, generate a point on the plane of the circle
// if on_edge is true, the point will be on the edge of the circle
void vm_vec_random_in_circle(vec3d *out, const vec3d *in, const matrix *orient, float radius, bool on_edge, bool bias_towards_center)
{
	vec3d temp;
	float scalar = frand();

	// sqrt because scaling inward increases the probability density by the square of its proximity towards the center
	if (!bias_towards_center)
		scalar = sqrtf(scalar);

	// point somewhere in the plane, maybe scaled inward
	vm_vec_scale_add(&temp, in, &orient->vec.rvec, on_edge ? radius : scalar * radius);

	// rotate to a random point on the circle
	vm_rot_point_around_line(out, &temp, fl_radians(frand_range(0.0f, 360.0f)), in, &orient->vec.fvec);
}

void vm_vec_unit_sphere_point(vec3d *out, float z_scale, float phi_scale)
{
	const auto z = (z_scale * 2.0f) - 1.0f; // convert range to [-1,1]
	const auto phi = phi_scale * PI2;
	const auto rho = sqrtf(1.0f - z * z);
	vm_vec_make(out, rho * cosf(phi), rho * sinf(phi), z); // Using the z-vec as the starting point
}

// given a start vector and a radius, generate a point in a spherical volume
// if on_surface is true, the point will be on the surface of the sphere
namespace {
	util::UniformFloatRange float_range(0.0f, 1.0f);
}
void vm_vec_random_in_sphere(vec3d *out, const vec3d *in, float radius, bool on_surface, bool bias_towards_center)
{
	vec3d temp;

	vm_vec_unit_sphere_point(&temp, float_range.next(), float_range.next());

	float scalar = 1.0f;

	if (!on_surface) {
		scalar = float_range.next();

		// cube root because scaling inward increases the probability density by the cube of its proximity towards the center
		if (!bias_towards_center)
			scalar = powf(scalar, 0.333f);
	}

	vm_vec_scale_add(out, in, &temp, scalar * radius);
}

// find the nearest point on the line to p. if dist is non-NULL, it is filled in
// returns 0 if the point is inside the line segment, -1 if "before" the line segment and 1 ir "after" the line segment
int vm_vec_dist_to_line(const vec3d *p, const vec3d *l0, const vec3d *l1, vec3d *nearest, float *dist)
{
	vec3d a, b, c;
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
	comp = vm_vec_dot(&a, &b) / b_mag;

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

// Goober5000
// Finds the distance squared to a line.  Same as above, except it uses vm_vec_dist_squared, which is faster;
// and it doesn't check whether the nearest point is on the line segment.
void vm_vec_dist_squared_to_line(const vec3d *p, const vec3d *l0, const vec3d *l1, vec3d *nearest, float *dist_squared)
{
	vec3d a, b, c;
	float b_mag, comp;

#ifndef NDEBUG
	if(vm_vec_same(l0, l1)){
		*nearest = vmd_zero_vector;
		return;
	}
#endif

	// compb_a == a dot b / len(b)
	vm_vec_sub(&a, p, l0);
	vm_vec_sub(&b, l1, l0);		
	b_mag = vm_vec_copy_normalize(&c, &b);	

	// calculate component
	comp = vm_vec_dot(&a, &b) / b_mag;

	// stuff nearest
	vm_vec_scale_add(nearest, l0, &c, comp);

	// get the distance
	*dist_squared = vm_vec_dist_squared(nearest, p);
}

//SUSHI: 2D vector "box" scaling
//Scales the vector in-place so that the longest dimension = scale
void vm_vec_boxscale(vec2d *vec, float  /*scale*/)
{
	float ratio = 1.0f / MAX(fl_abs(vec->x), fl_abs(vec->y));
	vec->x *= ratio;
	vec->y *= ratio;
}

// adds two matrices, fills in dest, returns ptr to dest
// ok for dest to equal either source, but should use vm_matrix_add2() if so
// dest = src0 + src1
void vm_matrix_add(matrix* dest, const matrix* src0, const matrix* src1)
{
	dest->vec.fvec = src0->vec.fvec + src1->vec.fvec;
	dest->vec.rvec = src0->vec.rvec + src1->vec.rvec;
	dest->vec.uvec = src0->vec.uvec + src1->vec.uvec;
}

// subs two matrices, fills in dest, returns ptr to dest
// ok for dest to equal either source, but should use vm_matrix_sub2() if so
// dest = src0 - src1
void vm_matrix_sub(matrix* dest, const matrix* src0, const matrix* src1)
{
	dest->vec.fvec = src0->vec.fvec - src1->vec.fvec;
	dest->vec.rvec = src0->vec.rvec - src1->vec.rvec;
	dest->vec.uvec = src0->vec.uvec - src1->vec.uvec;
}

// adds one matrix to another.
// dest can equal source
// dest += src
void vm_matrix_add2(matrix* dest, const matrix* src)
{
	dest->vec.fvec += src->vec.fvec;
	dest->vec.rvec += src->vec.rvec;
	dest->vec.uvec += src->vec.uvec;
}

// subs one matrix from another, returns ptr to dest
// dest can equal source
// dest -= src
void vm_matrix_sub2(matrix* dest, const matrix* src)
{
	dest->vec.fvec -= src->vec.fvec;
	dest->vec.rvec -= src->vec.rvec;
	dest->vec.uvec -= src->vec.uvec;
}

// TODO Remove this function if we ever move to a math library like glm
/**
* @brief							Attempts to invert a 3x3 matrix
* @param[inout]		dest     		The inverted matrix, or 0 if inversion is impossible
* @param[in]		m   			Pointer to the matrix we want to invert
*
* @returns							Whether or not the matrix is invertible
*/
bool vm_inverse_matrix(matrix* dest, const matrix* m)
{
	// Use doubles here because this is used for ship inv_mois and we could be dealing with extremely small numbers
	double inv[3][3];	// create a temp matrix so we can avoid getting a determinant that is 0

	// Use a2d so it's easier for people to read
	inv[0][0] = -(double)m->a2d[1][2] * (double)m->a2d[2][1] + (double)m->a2d[1][1] * (double)m->a2d[2][2];
	inv[0][1] = (double)m->a2d[0][2] * (double)m->a2d[2][1] - (double)m->a2d[0][1] * (double)m->a2d[2][2];
	inv[0][2] = -(double)m->a2d[0][2] * (double)m->a2d[1][1] + (double)m->a2d[0][1] * (double)m->a2d[1][2];
	inv[1][0] = (double)m->a2d[1][2] * (double)m->a2d[2][0] - (double)m->a2d[1][0] * (double)m->a2d[2][2];
	inv[1][1] = -(double)m->a2d[0][2] * (double)m->a2d[2][0] + (double)m->a2d[0][0] * (double)m->a2d[2][2];
	inv[1][2] = (double)m->a2d[0][2] * (double)m->a2d[1][0] - (double)m->a2d[0][0] * (double)m->a2d[1][2];
	inv[2][0] = -(double)m->a2d[1][1] * (double)m->a2d[2][0] + (double)m->a2d[1][0] * (double)m->a2d[2][1];
	inv[2][1] = (double)m->a2d[0][1] * (double)m->a2d[2][0] - (double)m->a2d[0][0] * (double)m->a2d[2][1];
	inv[2][2] = -(double)m->a2d[0][1] * (double)m->a2d[1][0] + (double)m->a2d[0][0] * (double)m->a2d[1][1];

	double det = (double)m->a2d[0][0] * inv[0][0] + (double)m->a2d[0][1] * inv[1][0] + (double)m->a2d[0][2] * inv[2][0];
	if (det == 0) {
		*dest = vmd_zero_matrix;
		return false;
	}

	det = 1.0f / det;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			dest->a2d[i][j] = (float)(inv[i][j] * det);
		}
	}

	return true;
}

// TODO Remove this function if we ever move to a math library like glm
/**
* @brief							Attempts to invert a 4x4 matrix
* @param[inout]		dest		The inverted matrix, or 0 if inversion is impossible
* @param[in]			m			Pointer to the matrix we want to invert
*
* @returns							Whether or not the matrix is invertible
*/
bool vm_inverse_matrix4(matrix4* dest, const matrix4* m)
{
	matrix4 inv;	// create a temp matrix so we can avoid getting a determinant that is 0

	// Use a2d so it's easier for people to read
	inv.a2d[0][0] = m->a2d[1][1] * m->a2d[2][2] * m->a2d[3][3] -
					m->a2d[1][1] * m->a2d[2][3] * m->a2d[3][2] -
					m->a2d[2][1] * m->a2d[1][2] * m->a2d[3][3] +
					m->a2d[2][1] * m->a2d[1][3] * m->a2d[3][2] +
					m->a2d[3][1] * m->a2d[1][2] * m->a2d[2][3] -
					m->a2d[3][1] * m->a2d[1][3] * m->a2d[2][2];

	inv.a2d[1][0] = -m->a2d[1][0] * m->a2d[2][2] * m->a2d[3][3] +
					m->a2d[1][0] * m->a2d[2][3] * m->a2d[3][2] +
					m->a2d[2][0] * m->a2d[1][2] * m->a2d[3][3] -
					m->a2d[2][0] * m->a2d[1][3] * m->a2d[3][2] -
					m->a2d[3][0] * m->a2d[1][2] * m->a2d[2][3] +
					m->a2d[3][0] * m->a2d[1][3] * m->a2d[2][2];

	inv.a2d[2][0] = m->a2d[1][0] * m->a2d[2][1] * m->a2d[3][3] -
					m->a2d[1][0] * m->a2d[2][3] * m->a2d[3][1] -
					m->a2d[2][0] * m->a2d[1][1] * m->a2d[3][3] +
					m->a2d[2][0] * m->a2d[1][3] * m->a2d[3][1] +
					m->a2d[3][0] * m->a2d[1][1] * m->a2d[2][3] -
					m->a2d[3][0] * m->a2d[1][3] * m->a2d[2][1];

	inv.a2d[3][0] = -m->a2d[1][0] * m->a2d[2][1] * m->a2d[3][2] +
					 m->a2d[1][0] * m->a2d[2][2] * m->a2d[3][1] +
					 m->a2d[2][0] * m->a2d[1][1] * m->a2d[3][2] -
					 m->a2d[2][0] * m->a2d[1][2] * m->a2d[3][1] -
					 m->a2d[3][0] * m->a2d[1][1] * m->a2d[2][2] +
					 m->a2d[3][0] * m->a2d[1][2] * m->a2d[2][1];

	inv.a2d[0][1] = -m->a2d[0][1] * m->a2d[2][2] * m->a2d[3][3] +
					 m->a2d[0][1] * m->a2d[2][3] * m->a2d[3][2] +
					 m->a2d[2][1] * m->a2d[0][2] * m->a2d[3][3] -
					 m->a2d[2][1] * m->a2d[0][3] * m->a2d[3][2] -
					 m->a2d[3][1] * m->a2d[0][2] * m->a2d[2][3] +
					 m->a2d[3][1] * m->a2d[0][3] * m->a2d[2][2];

	inv.a2d[1][1] = m->a2d[0][0] * m->a2d[2][2] * m->a2d[3][3] -
					m->a2d[0][0] * m->a2d[2][3] * m->a2d[3][2] -
					m->a2d[2][0] * m->a2d[0][2] * m->a2d[3][3] +
					m->a2d[2][0] * m->a2d[0][3] * m->a2d[3][2] +
					m->a2d[3][0] * m->a2d[0][2] * m->a2d[2][3] -
					m->a2d[3][0] * m->a2d[0][3] * m->a2d[2][2];

	inv.a2d[2][1] = -m->a2d[0][0] * m->a2d[2][1] * m->a2d[3][3] +
					 m->a2d[0][0] * m->a2d[2][3] * m->a2d[3][1] +
					 m->a2d[2][0] * m->a2d[0][1] * m->a2d[3][3] -
					 m->a2d[2][0] * m->a2d[0][3] * m->a2d[3][1] -
					 m->a2d[3][0] * m->a2d[0][1] * m->a2d[2][3] +
					 m->a2d[3][0] * m->a2d[0][3] * m->a2d[2][1];

	inv.a2d[3][1] = m->a2d[0][0] * m->a2d[2][1] * m->a2d[3][2] -
					m->a2d[0][0] * m->a2d[2][2] * m->a2d[3][1] -
					m->a2d[2][0] * m->a2d[0][1] * m->a2d[3][2] +
					m->a2d[2][0] * m->a2d[0][2] * m->a2d[3][1] +
					m->a2d[3][0] * m->a2d[0][1] * m->a2d[2][2] -
					m->a2d[3][0] * m->a2d[0][2] * m->a2d[2][1];

	inv.a2d[0][2] = m->a2d[0][1] * m->a2d[1][2] * m->a2d[3][3] -
					m->a2d[0][1] * m->a2d[1][3] * m->a2d[3][2] -
					m->a2d[1][1] * m->a2d[0][2] * m->a2d[3][3] +
					m->a2d[1][1] * m->a2d[0][3] * m->a2d[3][2] +
					m->a2d[3][1] * m->a2d[0][2] * m->a2d[1][3] -
					m->a2d[3][1] * m->a2d[0][3] * m->a2d[1][2];

	inv.a2d[1][2] = -m->a2d[0][0] * m->a2d[1][2] * m->a2d[3][3] +
					 m->a2d[0][0] * m->a2d[1][3] * m->a2d[3][2] +
					 m->a2d[1][0] * m->a2d[0][2] * m->a2d[3][3] -
					 m->a2d[1][0] * m->a2d[0][3] * m->a2d[3][2] -
					 m->a2d[3][0] * m->a2d[0][2] * m->a2d[1][3] +
					 m->a2d[3][0] * m->a2d[0][3] * m->a2d[1][2];

	inv.a2d[2][2] = m->a2d[0][0] * m->a2d[1][1] * m->a2d[3][3] -
					m->a2d[0][0] * m->a2d[1][3] * m->a2d[3][1] -
					m->a2d[1][0] * m->a2d[0][1] * m->a2d[3][3] +
					m->a2d[1][0] * m->a2d[0][3] * m->a2d[3][1] +
					m->a2d[3][0] * m->a2d[0][1] * m->a2d[1][3] -
					m->a2d[3][0] * m->a2d[0][3] * m->a2d[1][1];

	inv.a2d[3][2] = -m->a2d[0][0] * m->a2d[1][1] * m->a2d[3][2] +
					 m->a2d[0][0] * m->a2d[1][2] * m->a2d[3][1] +
					 m->a2d[1][0] * m->a2d[0][1] * m->a2d[3][2] -
					 m->a2d[1][0] * m->a2d[0][2] * m->a2d[3][1] -
					 m->a2d[3][0] * m->a2d[0][1] * m->a2d[1][2] +
					 m->a2d[3][0] * m->a2d[0][2] * m->a2d[1][1];

	inv.a2d[0][3] = -m->a2d[0][1] * m->a2d[1][2] * m->a2d[2][3] +
					 m->a2d[0][1] * m->a2d[1][3] * m->a2d[2][2] +
					 m->a2d[1][1] * m->a2d[0][2] * m->a2d[2][3] -
					 m->a2d[1][1] * m->a2d[0][3] * m->a2d[2][2] -
					 m->a2d[2][1] * m->a2d[0][2] * m->a2d[1][3] +
					 m->a2d[2][1] * m->a2d[0][3] * m->a2d[1][2];

	inv.a2d[1][3] = m->a2d[0][0] * m->a2d[1][2] * m->a2d[2][3] -
					m->a2d[0][0] * m->a2d[1][3] * m->a2d[2][2] -
					m->a2d[1][0] * m->a2d[0][2] * m->a2d[2][3] +
					m->a2d[1][0] * m->a2d[0][3] * m->a2d[2][2] +
					m->a2d[2][0] * m->a2d[0][2] * m->a2d[1][3] -
					m->a2d[2][0] * m->a2d[0][3] * m->a2d[1][2];

	inv.a2d[2][3] = -m->a2d[0][0] * m->a2d[1][1] * m->a2d[2][3] +
					 m->a2d[0][0] * m->a2d[1][3] * m->a2d[2][1] +
					 m->a2d[1][0] * m->a2d[0][1] * m->a2d[2][3] -
					 m->a2d[1][0] * m->a2d[0][3] * m->a2d[2][1] -
					 m->a2d[2][0] * m->a2d[0][1] * m->a2d[1][3] +
					 m->a2d[2][0] * m->a2d[0][3] * m->a2d[1][1];

	inv.a2d[3][3] = m->a2d[0][0] * m->a2d[1][1] * m->a2d[2][2] -
					m->a2d[0][0] * m->a2d[1][2] * m->a2d[2][1] -
					m->a2d[1][0] * m->a2d[0][1] * m->a2d[2][2] +
					m->a2d[1][0] * m->a2d[0][2] * m->a2d[2][1] +
					m->a2d[2][0] * m->a2d[0][1] * m->a2d[1][2] -
					m->a2d[2][0] * m->a2d[0][2] * m->a2d[1][1];

	float det = m->a2d[0][0] * inv.a2d[0][0] + m->a2d[0][1] * inv.a2d[1][0] + m->a2d[0][2] * inv.a2d[2][0] + m->a2d[0][3] * inv.a2d[3][0];

	if (det == 0) {
		*dest = vmd_zero_matrix4;
		return false;
	}

	det = 1.0f / det;

	for (int i = 0; i < 16; i++) {
		dest->a1d[i] = inv.a1d[i] * det;
	}

	return true;
}

void vm_matrix4_set_orthographic(matrix4* out, vec3d *max, vec3d *min)
{
	memset(out, 0, sizeof(matrix4));

	out->a1d[0] = 2.0f / (max->xyz.x - min->xyz.x);
	out->a1d[5] = 2.0f / (max->xyz.y - min->xyz.y);
	out->a1d[10] = -2.0f / (max->xyz.z - min->xyz.z);
	out->a1d[12] = -(max->xyz.x + min->xyz.x) / (max->xyz.x - min->xyz.x);
	out->a1d[13] = -(max->xyz.y + min->xyz.y) / (max->xyz.y - min->xyz.y);
	out->a1d[14] = -(max->xyz.z + min->xyz.z) / (max->xyz.z - min->xyz.z);
	out->a1d[15] = 1.0f;
}

void vm_matrix4_set_inverse_transform(matrix4 *out, matrix *m, vec3d *v)
{
	// this is basically the same function as the opengl view matrix construction
	// except we don't invert the Z-axis
	vec3d scaled_pos;
	vec3d inv_pos;
	matrix inv_orient;

	vm_vec_copy_scale(&scaled_pos, v, -1.0f);

	vm_copy_transpose(&inv_orient, m);
	vm_vec_rotate(&inv_pos, &scaled_pos, m);

	vm_matrix4_set_transform(out, &inv_orient, &inv_pos);
}

void vm_matrix4_set_identity(matrix4 *out)
{
	out->a2d[0][0] = 1.0f;
	out->a2d[0][1] = 0.0f;
	out->a2d[0][2] = 0.0f;
	out->a2d[0][3] = 0.0f;

	out->a2d[1][0] = 0.0f;
	out->a2d[1][1] = 1.0f;
	out->a2d[1][2] = 0.0f;
	out->a2d[1][3] = 0.0f;

	out->a2d[2][0] = 0.0f;
	out->a2d[2][1] = 0.0f;
	out->a2d[2][2] = 1.0f;
	out->a2d[2][3] = 0.0f;

	out->a2d[3][0] = 0.0f;
	out->a2d[3][1] = 0.0f;
	out->a2d[3][2] = 0.0f;
	out->a2d[3][3] = 1.0f;
}

void vm_matrix4_set_transform(matrix4 *out, matrix *m, vec3d *v)
{
	vm_matrix4_set_identity(out);

	out->a2d[0][0] = m->a2d[0][0];
	out->a2d[0][1] = m->a2d[0][1];
	out->a2d[0][2] = m->a2d[0][2];

	out->a2d[1][0] = m->a2d[1][0];
	out->a2d[1][1] = m->a2d[1][1];
	out->a2d[1][2] = m->a2d[1][2];

	out->a2d[2][0] = m->a2d[2][0];
	out->a2d[2][1] = m->a2d[2][1];
	out->a2d[2][2] = m->a2d[2][2];

	out->a2d[3][0] = v->a1d[0];
	out->a2d[3][1] = v->a1d[1];
	out->a2d[3][2] = v->a1d[2];
}

void vm_matrix4_get_orientation(matrix *out, const matrix4 *m)
{
	out->a2d[0][0] = m->a2d[0][0];
	out->a2d[0][1] = m->a2d[0][1];
	out->a2d[0][2] = m->a2d[0][2];

	out->a2d[1][0] = m->a2d[1][0];
	out->a2d[1][1] = m->a2d[1][1];
	out->a2d[1][2] = m->a2d[1][2];

	out->a2d[2][0] = m->a2d[2][0];
	out->a2d[2][1] = m->a2d[2][1];
	out->a2d[2][2] = m->a2d[2][2];
}

void vm_matrix4_get_offset(vec3d *out, matrix4 *m)
{
	out->xyz.x = m->vec.pos.xyzw.x;
	out->xyz.y = m->vec.pos.xyzw.y;
	out->xyz.z = m->vec.pos.xyzw.z;
}

void vm_matrix4_x_matrix4(matrix4 *dest, const matrix4 *src0, const matrix4 *src1)
{
	dest->vec.rvec.xyzw.x	= vm_vec4_dot4(src0->vec.rvec.xyzw.x, src0->vec.uvec.xyzw.x, src0->vec.fvec.xyzw.x, src0->vec.pos.xyzw.x, &src1->vec.rvec);
	dest->vec.uvec.xyzw.x	= vm_vec4_dot4(src0->vec.rvec.xyzw.x, src0->vec.uvec.xyzw.x, src0->vec.fvec.xyzw.x, src0->vec.pos.xyzw.x, &src1->vec.uvec);
	dest->vec.fvec.xyzw.x	= vm_vec4_dot4(src0->vec.rvec.xyzw.x, src0->vec.uvec.xyzw.x, src0->vec.fvec.xyzw.x, src0->vec.pos.xyzw.x, &src1->vec.fvec);
	dest->vec.pos.xyzw.x	= vm_vec4_dot4(src0->vec.rvec.xyzw.x, src0->vec.uvec.xyzw.x, src0->vec.fvec.xyzw.x, src0->vec.pos.xyzw.x, &src1->vec.pos);
	
	dest->vec.rvec.xyzw.y	= vm_vec4_dot4(src0->vec.rvec.xyzw.y, src0->vec.uvec.xyzw.y, src0->vec.fvec.xyzw.y, src0->vec.pos.xyzw.y, &src1->vec.rvec);
	dest->vec.uvec.xyzw.y	= vm_vec4_dot4(src0->vec.rvec.xyzw.y, src0->vec.uvec.xyzw.y, src0->vec.fvec.xyzw.y, src0->vec.pos.xyzw.y, &src1->vec.uvec);
	dest->vec.fvec.xyzw.y	= vm_vec4_dot4(src0->vec.rvec.xyzw.y, src0->vec.uvec.xyzw.y, src0->vec.fvec.xyzw.y, src0->vec.pos.xyzw.y, &src1->vec.fvec);
	dest->vec.pos.xyzw.y	= vm_vec4_dot4(src0->vec.rvec.xyzw.y, src0->vec.uvec.xyzw.y, src0->vec.fvec.xyzw.y, src0->vec.pos.xyzw.y, &src1->vec.pos);

	dest->vec.rvec.xyzw.z	= vm_vec4_dot4(src0->vec.rvec.xyzw.z, src0->vec.uvec.xyzw.z, src0->vec.fvec.xyzw.z, src0->vec.pos.xyzw.z, &src1->vec.rvec);
	dest->vec.uvec.xyzw.z	= vm_vec4_dot4(src0->vec.rvec.xyzw.z, src0->vec.uvec.xyzw.z, src0->vec.fvec.xyzw.z, src0->vec.pos.xyzw.z, &src1->vec.uvec);
	dest->vec.fvec.xyzw.z	= vm_vec4_dot4(src0->vec.rvec.xyzw.z, src0->vec.uvec.xyzw.z, src0->vec.fvec.xyzw.z, src0->vec.pos.xyzw.z, &src1->vec.fvec);
	dest->vec.pos.xyzw.z	= vm_vec4_dot4(src0->vec.rvec.xyzw.z, src0->vec.uvec.xyzw.z, src0->vec.fvec.xyzw.z, src0->vec.pos.xyzw.z, &src1->vec.pos);

	dest->vec.rvec.xyzw.w	= vm_vec4_dot4(src0->vec.rvec.xyzw.w, src0->vec.uvec.xyzw.w, src0->vec.fvec.xyzw.w, src0->vec.pos.xyzw.w, &src1->vec.rvec);
	dest->vec.uvec.xyzw.w	= vm_vec4_dot4(src0->vec.rvec.xyzw.w, src0->vec.uvec.xyzw.w, src0->vec.fvec.xyzw.w, src0->vec.pos.xyzw.w, &src1->vec.uvec);
	dest->vec.fvec.xyzw.w	= vm_vec4_dot4(src0->vec.rvec.xyzw.w, src0->vec.uvec.xyzw.w, src0->vec.fvec.xyzw.w, src0->vec.pos.xyzw.w, &src1->vec.fvec);
	dest->vec.pos.xyzw.w	= vm_vec4_dot4(src0->vec.rvec.xyzw.w, src0->vec.uvec.xyzw.w, src0->vec.fvec.xyzw.w, src0->vec.pos.xyzw.w, &src1->vec.pos);
}

float vm_vec4_dot4(float x, float y, float z, float w, const vec4 *v)
{
	return (x * v->xyzw.x) + (y * v->xyzw.y) + (z * v->xyzw.z) + (w * v->xyzw.w);
}

void vm_vec_transform(vec4 *dest, const vec4 *src, const matrix4 *m)
{
	dest->xyzw.x = (m->vec.rvec.xyzw.x * src->xyzw.x) + (m->vec.uvec.xyzw.x * src->xyzw.y) + (m->vec.fvec.xyzw.x * src->xyzw.z) + (m->vec.pos.xyzw.x * src->xyzw.w);
	dest->xyzw.y = (m->vec.rvec.xyzw.y * src->xyzw.x) + (m->vec.uvec.xyzw.y * src->xyzw.y) + (m->vec.fvec.xyzw.y * src->xyzw.z) + (m->vec.pos.xyzw.y * src->xyzw.w);
	dest->xyzw.z = (m->vec.rvec.xyzw.z * src->xyzw.x) + (m->vec.uvec.xyzw.z * src->xyzw.y) + (m->vec.fvec.xyzw.z * src->xyzw.z) + (m->vec.pos.xyzw.z * src->xyzw.w);
	dest->xyzw.w = (m->vec.rvec.xyzw.w * src->xyzw.x) + (m->vec.uvec.xyzw.w * src->xyzw.y) + (m->vec.fvec.xyzw.w * src->xyzw.z) + (m->vec.pos.xyzw.w * src->xyzw.w);
}

void vm_vec_transform(vec3d *dest, const vec3d *src, const matrix4 *m, bool pos)
{
	vec4 temp_src, temp_dest;

	temp_src.xyzw.x = src->xyz.x;
	temp_src.xyzw.y = src->xyz.y;
	temp_src.xyzw.z = src->xyz.z;

	// whether to treat vec3d src as a position or a vector. 
	// 0.0f will prevent matrix4 m's offset from being added. 1.0f will add the offset. 
	temp_src.xyzw.w = pos ? 1.0f : 0.0f;

	vm_vec_transform(&temp_dest, &temp_src, m);

	dest->xyz.x = temp_dest.xyzw.x;
	dest->xyz.y = temp_dest.xyzw.y;
	dest->xyz.z = temp_dest.xyzw.z;
}
vec3d vm_vec4_to_vec3(const vec4& vec) {
	vec3d out;

	out.xyz.x = vec.xyzw.x;
	out.xyz.y = vec.xyzw.y;
	out.xyz.z = vec.xyzw.z;

	return out;
}
vec4 vm_vec3_to_ve4(const vec3d& vec, float w) {
	vec4 out;

	out.xyzw.x = vec.xyz.x;
	out.xyzw.y = vec.xyz.y;
	out.xyzw.z = vec.xyz.z;
	out.xyzw.w = w;

	return out;
}

// This function is used when we want to "match orientation" to a target, here match_orient,
// while still pointing our forward vector in a certain direction, here goal_fvec.
// out_rvec is the best matching right vector to match_orient.rvec
void vm_match_bank(vec3d* out_rvec, const vec3d* goal_fvec, const matrix* match_orient) {
	// We want to calculate out_rvec as a frame transformation, translating match_orient.rvec
	// from source_frame to dest_frame.
	//
	// We set up the frames such that:
	// * source fvec = match_orient.fvec
	// * dest fvec = goal_fvec
	// * source uvec = dest uvec
	// This uniquely determines both frames, and the rvecs go along for the ride.
	// Once we have these frames, we just rotate match_orient.rvec from one frame to the other.

	// Calculate the source frame. The common uvec has to be perpendicular to match_orient.fvec
	// and goal_fvec so we cross to get it. The rvec is left as 0 to be set by vm_orthogonalize_matrix
	matrix source_frame = vmd_zero_matrix;
	source_frame.vec.fvec = match_orient->vec.fvec;
	vm_vec_cross(&source_frame.vec.uvec, &source_frame.vec.fvec, goal_fvec);
	vm_orthogonalize_matrix(&source_frame);

	// Calculate the destination frame, using goal_fvec and the common uvec.
	// These are already orthogonal and normalized so we can just cross to get the rvec rather than
	// calling vm_orthogonalize_matrix
	matrix dest_frame;
	dest_frame.vec.fvec = *goal_fvec;
	dest_frame.vec.uvec = source_frame.vec.uvec;
	vm_vec_cross(&dest_frame.vec.rvec, &dest_frame.vec.uvec, &dest_frame.vec.fvec);

	// Apply the transformation to match_orient.rvec, returning the result in out_rvec
	vec3d temp;
	vm_vec_rotate(&temp, &match_orient->vec.rvec, &source_frame);
	vm_vec_unrotate(out_rvec, &temp, &dest_frame);
}

// Cyborg17 - Rotational interpolation between two angle structs in radians.  Assumes that the rotation direction is the smaller arc difference.
// src0 is the starting angle struct, src1 is the ending angle struct, interp_perc must be a float between 0.0f and 1.0f inclusive.
// rot_vel is only used to determine the rotation direction. This functions assumes a <= 2PI rotation in any axis.  
// You will get inaccurate results otherwise.
void vm_interpolate_angles_quick(angles *dest0, angles *src0, angles *src1, float interp_perc) {
	
	Assertion((interp_perc >= 0.0f) && (interp_perc <= 1.0f), "Interpolation percentage, %f, sent to vm_interpolate_angles is invalid. The valid range is [0,1], go find a coder!", interp_perc);

	angles arc_measures;

	arc_measures.p = src1->p - src0->p;	
	arc_measures.h = src1->h - src0->h;	
	arc_measures.b = src1->b - src0->b;

	  // pitch
	  // if start and end are basically the same, assume we can basically jump to the end.
	if ( (fabs(arc_measures.p) < 0.00001f) ) {
		arc_measures.p = 0.0f;

	} // Test if we actually need to go in the other direction
	else if (arc_measures.p > (PI*1.5f)) {
		arc_measures.p = PI2 - arc_measures.p;

	} // Test if we actually need to go in the other direction for negative values
	else if (arc_measures.p < -PI_2) {
		arc_measures.p = -PI2 - arc_measures.p;
	}

	  // heading
	  // if start and end are basically the same, assume we can basically jump to the end.
	if ( (fabs(arc_measures.h) < 0.00001f) ) {
		arc_measures.h = 0.0f;

	} // Test if we actually need to go in the other direction
	else if (arc_measures.h > (PI*1.5f)) {
		arc_measures.h = PI2 - arc_measures.h;

	} // Test if we actually need to go in the other direction for negative values
	else if (arc_measures.h < -PI_2) {
		arc_measures.h = -PI2 - arc_measures.h;
	}

	// bank
	// if start and end are basically the same, assume we can basically jump to the end.
	if ( (fabs(arc_measures.b) < 0.00001f) ) {
		arc_measures.b = 0.0f;

	} // Test if we actually need to go in the other direction
	else if (arc_measures.b > (PI*1.5f)) {
		arc_measures.b = PI2 - arc_measures.b;

	} // Test if we actually need to go in the other direction for negative values
	else if (arc_measures.b < -PI_2) {
		arc_measures.b = -PI2 - arc_measures.b;
	}

	// Now just multiply the difference in angles by the given percentage, and then subtract it from the destination angles.
	// If arc_measures is 0.0f, then we are basically bashing to the ending orientation without worrying about the inbetween.
	dest0->p = src0->p + (arc_measures.p * interp_perc);
	dest0->h = src0->h + (arc_measures.h * interp_perc);
	dest0->b = src0->b + (arc_measures.b * interp_perc);
}

std::ostream& operator<<(std::ostream& os, const vec3d& vec)
{
	os << "vec3d<" << vec.xyz.x << ", " << vec.xyz.y << ", " << vec.xyz.z << ">";
	return os;
}

matrix vm_stretch_matrix(const vec3d* stretch_dir, float stretch) {
	matrix outer_prod;
	vm_vec_outer_product(&outer_prod, stretch_dir);

	for (float& i : outer_prod.a1d)
		i *= stretch - 1.f;


	return vmd_identity_matrix + outer_prod;
}

// generates a well distributed quasi-random position in a -1 to 1 cube
// the caller must provide and increment the seed for each call for proper results
// algorithm taken from http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
const float phi3 = 1.220744084f;
vec3d vm_well_distributed_rand_vec(int seed, vec3d* offset) {
	vec3d out;
	if (offset != nullptr) {
		out.xyz.x = fmod(-fmod(offset->xyz.x, 1.f) + ((1.f / phi3) * seed), 1.f) * 2 - 1;
		out.xyz.y = fmod(-fmod(offset->xyz.y, 1.f) + ((1.f / (phi3 * phi3)) * seed), 1.f) * 2 - 1;
		out.xyz.z = fmod(-fmod(offset->xyz.z, 1.f) + ((1.f / (phi3 * phi3 * phi3)) * seed), 1.f) * 2 - 1;
	}
	else {
		out.xyz.x = fmod((1.f / phi3) * seed, 1.f) * 2 - 1;
		out.xyz.y = fmod((1.f / (phi3 * phi3)) * seed, 1.f) * 2 - 1;
		out.xyz.z = fmod((1.f / (phi3 * phi3 * phi3)) * seed, 1.f) * 2 - 1;
	}
	return out;
}
