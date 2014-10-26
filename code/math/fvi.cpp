/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <float.h>	// For FLT_MAX

#include "globalincs/pstypes.h"
#include "math/vecmat.h"
#include "math/floating.h"
#include "math/fvi.h"


#define	SMALL_NUM	1E-6

#define	UNINITIALIZED_VALUE	-1234567.8f
#define WARN_DIST	1.0

void accurate_square_root( float A, float B, float C, float discriminant, float *root1, float *root2 );


float matrix_determinant_from_vectors(vec3d *v1,vec3d *v2,vec3d *v3)
{
	float ans;
	ans =  v1->xyz.x * v2->xyz.y * v3->xyz.z;
	ans += v2->xyz.x * v3->xyz.y * v1->xyz.z;
	ans += v3->xyz.x * v1->xyz.y * v2->xyz.z;
	ans -= v1->xyz.z * v2->xyz.y * v3->xyz.x;
	ans -= v2->xyz.z * v3->xyz.y * v1->xyz.x;
	ans -= v3->xyz.z * v1->xyz.y * v2->xyz.x;

	return ans;
}

/**
 * Finds the point on each line of closest approach (s and t) (lines need not intersect to get closest point)
 * Taken from graphic gems I, p. 304
 *
 * lines: L1 = P1 + V1s  and   L2 = P2 + V2t
 */
void fvi_two_lines_in_3space(vec3d *p1, vec3d *v1, vec3d *p2, vec3d *v2, float *s, float *t)
{
	vec3d cross,delta;
	vm_vec_crossprod(&cross, v1, v2);
	vm_vec_sub(&delta, p2, p1);

	float denominator = vm_vec_mag_squared(&cross);
	float num_t, num_s;

	if (denominator > 1e-10) {
		num_s = matrix_determinant_from_vectors(&delta, v2, &cross);
		*s = num_s / denominator;

		num_t = matrix_determinant_from_vectors(&delta, v1, &cross);
		*t = num_t / denominator;
	} else {
		// two lines are parallel
		*s = FLT_MAX;
		*t = FLT_MAX;
	}
}

/**
 * Tells distance from a plain to a point-Bobboau
 *
 * @param plane_pnt		Plane description, a point
 * @param plane_norm	Plane description, a normal
 * @param point			A point to test
 */
float fvi_point_dist_plane(	vec3d *plane_pnt, vec3d *plane_norm,
					    vec3d *point
						)
{
	float dist,D;
		
	D = -vm_vec_dot(plane_norm,plane_pnt);

	dist = vm_vec_dot(plane_norm, point) + D;
	return dist;
}

/**
 * Finds the point on the specified plane where the infinite ray intersects.
 *
 * @param new_pnt		A point to test
 * @param plane_pnt		Plane description, a point
 * @param plane_norm	Plane description, a normal
 * @param ray_origin	Ray description, an origin
 * @param ray_direction	Ray description, a direction
 * @param rad			Radius
 *
 * Returns scaled-distance plane is from the ray_origin (t), so
 * P = O + t*D, where P is the point of intersection, O is the ray origin,
 * and D is the ray's direction.  So 0.0 would mean the intersection is 
 * exactly on the ray origin, 1.0 would be on the ray origin plus the ray
 * direction vector, anything negative would be behind the ray's origin.
 * If you pass a pointer to the new_pnt, this routine will perform the P=
 * calculation to calculate the point of intersection and put the result
 * in *new_pnt.
 *
 * If radius is anything other than 0.0, it assumes you want the intersection
 * point to be that radius from the plane.
 *
 * Note that ray_direction doesn't have to be normalized unless you want
 * the return value to be in units from the ray origin.
 *
 * Also note that new_pnt will always be filled in to some valid value,
 * even if it is a point at infinity.   
 *
 * If the plane and line are parallel, this will return the largest 
 * negative float number possible.
 * 
 * So if you want to check a line segment from p0 to p1, you would pass
 * p0 as ray_origin, p1-p0 as the ray_direction, and there would be an
 * intersection if the return value is between 0 and 1.
 */
float fvi_ray_plane(vec3d *new_pnt,
                    vec3d *plane_pnt,vec3d *plane_norm,
                    vec3d *ray_origin,vec3d *ray_direction,
						  float rad)
{
	vec3d w;
	float num,den,t;
	
	vm_vec_sub(&w,ray_origin,plane_pnt);
	
	den = -vm_vec_dot(plane_norm,ray_direction);
	if ( den == 0.0f ) {	// Ray & plane are parallel, so there is no intersection
		if ( new_pnt )	{
			new_pnt->xyz.x = -FLT_MAX;
			new_pnt->xyz.y = -FLT_MAX;
			new_pnt->xyz.z = -FLT_MAX;
		}
		return -FLT_MAX;			
	}

	num =  vm_vec_dot(plane_norm,&w);
	num -= rad;			//move point out by rad
	
	t = num / den;

	if ( new_pnt )
		vm_vec_scale_add(new_pnt,ray_origin,ray_direction,t);

	return t;
}

/**
 * Find the point on the specified plane where the line intersects
 *
 * @param new_pnt		The found point on the plane
 * @param plane_pnt		Plane description, a point
 * @param plane_norm	Plane description, a normal
 * @param p0			The first end of the line
 * @param p1			The second end of the line
 * @param rad			Radius
 *
 * @return true if point found, false if line parallel to plane
 */
  
int fvi_segment_plane(vec3d *new_pnt,
										vec3d *plane_pnt,vec3d *plane_norm,
                                 vec3d *p0,vec3d *p1,float rad)
{
	float t;
	vec3d d;

	vm_vec_sub( &d, p1, p0 );
	
	t = fvi_ray_plane(new_pnt,
                    plane_pnt,plane_norm,		// Plane description, a point and a normal
                    p0,&d,	// Ray description, a point and a direction
						  rad);

	if ( t < 0.0f ) return 0;		// intersection lies behind p0
	if ( t > 1.0f ) return 0;		// intersection lies past p1

	return 1;		// They intersect!
}

/**
 * Determine if and where a vector intersects with a sphere
 *
 * vector defined by p0,p1 
 * @return 1 if intersects, and fills in intp, else returns 0
 */
int fvi_segment_sphere(vec3d *intp,vec3d *p0,vec3d *p1,vec3d *sphere_pos,float sphere_rad)
{
	vec3d d,dn,w,closest_point;
	float mag_d,dist,w_dist,int_dist;

	//this routine could be optimized if it's taking too much time!

	vm_vec_sub(&d,p1,p0);
	vm_vec_sub(&w,sphere_pos,p0);

	mag_d = vm_vec_mag(&d);

	if (mag_d <= 0.0f) {
		int_dist = vm_vec_mag(&w);
		*intp = *p0;
		return (int_dist<sphere_rad)?1:0;
	}

	// normalize dn
	dn.xyz.x = d.xyz.x / mag_d;
	dn.xyz.y = d.xyz.y / mag_d;
	dn.xyz.z = d.xyz.z / mag_d;

	w_dist = vm_vec_dot(&dn,&w);

	if (w_dist < -sphere_rad)		//moving away from object
		 return 0;

	if (w_dist > mag_d+sphere_rad)
		return 0;		//cannot hit

	vm_vec_scale_add(&closest_point,p0,&dn,w_dist);

	dist = vm_vec_dist(&closest_point,sphere_pos);

	if (dist < sphere_rad) {
		float dist2,rad2,shorten;

		dist2 = dist*dist;
		rad2 = sphere_rad*sphere_rad;

		shorten = fl_sqrt(rad2 - dist2);

		int_dist = w_dist-shorten;

		if (int_dist > mag_d || int_dist < 0.0f) {
			//past one or the other end of vector, which means we're inside

			*intp = *p0;		//don't move at all
			return 1;
		}

		vm_vec_scale_add(intp,p0,&dn,int_dist);         //calc intersection point
		
		return 1;
	}
	else
		return 0;
}


/**
 * Determine if and where a ray intersects with a sphere
 *
 * vector defined by p0,p1 
 * @return 1 if intersects, and fills in intp. else returns 0
 */
int fvi_ray_sphere(vec3d *intp,vec3d *p0,vec3d *p1,vec3d *sphere_pos,float sphere_rad)
{
	vec3d d,dn,w,closest_point;
	float mag_d,dist,w_dist,int_dist;

	//this routine could be optimized if it's taking too much time!

	vm_vec_sub(&d,p1,p0);
	vm_vec_sub(&w,sphere_pos,p0);

	mag_d = vm_vec_mag(&d);

	if (mag_d <= 0.0f) {
		int_dist = vm_vec_mag(&w);
		*intp = *p0;
		return (int_dist<sphere_rad)?1:0;
	}

	// normalize dn
	dn.xyz.x = d.xyz.x / mag_d;
	dn.xyz.y = d.xyz.y / mag_d;
	dn.xyz.z = d.xyz.z / mag_d;

	w_dist = vm_vec_dot(&dn,&w);

	if (w_dist < -sphere_rad)		//moving away from object
		 return 0;

	vm_vec_scale_add(&closest_point,p0,&dn,w_dist);

	dist = vm_vec_dist(&closest_point,sphere_pos);

	if (dist < sphere_rad) {
		float dist2,rad2,shorten;

		dist2 = dist*dist;
		rad2 = sphere_rad*sphere_rad;

		shorten = fl_sqrt(rad2 - dist2);

		int_dist = w_dist-shorten;

		if (int_dist < 0.0f) {
			//past one or the begining of vector, which means we're inside

			*intp = *p0;		//don't move at all
			return 1;
		}

		vm_vec_scale_add(intp,p0,&dn,int_dist);         //calc intersection point

		return 1;
	}
	else
		return 0;
}

/**
 * Finds intersection of a ray and an axis-aligned bounding box
 *
 * Given a ray with origin at p0, and direction pdir, this function
 * returns non-zero if that ray intersects an axis-aligned bounding box
 * from min to max.   If there was an intersection, then hitpt will contain
 * the point where the ray begins inside the box.
 * Fast ray-box intersection taken from Graphics Gems I, pages 395,736.
 */
int fvi_ray_boundingbox( vec3d *min, vec3d *max, vec3d * p0, vec3d *pdir, vec3d *hitpt )
{
	int middle = ((1<<0) | (1<<1) | (1<<2));
	int i;
	int which_plane;
	float maxt[3];
	float candidate_plane[3];

	for (i = 0; i < 3; i++) {
		if (p0->a1d[i] < min->a1d[i]) {
			candidate_plane[i] = min->a1d[i];
			middle &= ~(1<<i);
		} else if (p0->a1d[i] > max->a1d[i]) {
			candidate_plane[i] = max->a1d[i];
			middle &= ~(1<<i);
		}
	}

	// ray origin inside bounding box?
	// (are all three bits still set?)
	if (middle == ((1<<0) | (1<<1) | (1<<2))) {
		*hitpt = *p0;
		return 1;
	}

	// calculate T distances to candidate plane
	for (i = 0; i < 3; i++) {
		if ( (middle & (1<<i)) || (pdir->a1d[i] == 0.0f) ) {
			maxt[i] = -1.0f;
		} else {
			maxt[i] = (candidate_plane[i] - p0->a1d[i]) / pdir->a1d[i];
		}
	}

	// Get largest of the maxt's for final choice of intersection
	which_plane = 0;
	for (i = 1; i < 3; i++) {
		if (maxt[which_plane] < maxt[i]) {
			which_plane = i;
		}
	}

	// check final candidate actually inside box
	if (maxt[which_plane] < 0.0f) {
		return 0;
	}

	for (i = 0; i < 3; i++) {
		if (which_plane == i) {
			hitpt->a1d[i] = candidate_plane[i];
		} else {
			hitpt->a1d[i] = (maxt[which_plane] * pdir->a1d[i]) + p0->a1d[i];

			if ( (hitpt->a1d[i] < min->a1d[i]) || (hitpt->a1d[i] > max->a1d[i]) ) {
				return 0;
			}
		}
	}

	return 1;
}

/**
 * Given largest componant of normal, return i & j
 * If largest componant is negative, swap i & j
 */
int ij_table[3][2] =        {
							{2,1},          //pos x biggest
							{0,2},          //pos y biggest
							{1,0},          //pos z biggest
						};

/**
 * See if a point in inside a face by projecting into 2d. Also
 * finds uv's if uvls is not NULL.  
 *
 * Returns 0 if point isn't on face, non-zero otherwise.
 *
 * From Graphics Gems I, "An efficient Ray-Polygon intersection", p390
 *
 * @param checkp	The point to check
 * @param nv		How many verts in the poly
 * @param verts		The vertives for the polygon 
 * @param norm1		The polygon's normal
 * @param u_out		If not null and v_out not null and uvls not_null and point is on face, the uv's of where it hit
 * @param vout		If not null and v_out not null and uvls not_null and point is on face, the uv's of where it hit
 * @param uvls		A list of uv pairs for each vertex
 *
 * This replaces the old check_point_to_face & find_hitpoint_uv
 * WARNING!!   In Gems, they use the code "if (u1==0)" in this function.
 * I have found several cases where this will not detect collisions it should.
 * I found two solutions:
 *   1. Declare the 'beta' variable to be a double.
 *   2. Instead of using 'if (u1==0)', compare it to a small value.
 * I chose #2 because I would rather have our code work with all floats
 * and never need doubles.   -JAS Aug22,1997
 */
#define delta 0.0001f
int fvi_point_face(vec3d *checkp, int nv, vec3d **verts, vec3d * norm1, float *u_out,float *v_out, uv_pair * uvls )
{
	float *norm, *P;
	vec3d t;
	int i0, i1,i2;

	norm = (float *)norm1;

	//project polygon onto plane by finding largest component of normal
	t.xyz.x = fl_abs(norm[0]); 
	t.xyz.y = fl_abs(norm[1]); 
	t.xyz.z = fl_abs(norm[2]);

	if (t.xyz.x > t.xyz.y) if (t.xyz.x > t.xyz.z) i0=0; else i0=2;
	else if (t.xyz.y > t.xyz.z) i0=1; else i0=2;

	if (norm[i0] > 0.0f) {
		i1 = ij_table[i0][0];
		i2 = ij_table[i0][1];
	}
	else {
		i1 = ij_table[i0][1];
		i2 = ij_table[i0][0];
	}

	// Have i0, i1, i2
	P = (float *)checkp;
	
	float u0, u1, u2, v0, v1, v2, alpha = UNINITIALIZED_VALUE, gamma;
	float beta;

	int inter=0, i = 2;	

	u0 = P[i1] - verts[0]->a1d[i1];
	v0 = P[i2] - verts[0]->a1d[i2];

	do {

		u1 = verts[i-1]->a1d[i1] - verts[0]->a1d[i1]; 
		u2 = verts[i  ]->a1d[i1] - verts[0]->a1d[i1];

		v1 = verts[i-1]->a1d[i2] - verts[0]->a1d[i2];
		v2 = verts[i  ]->a1d[i2] - verts[0]->a1d[i2];


		if ( (u1 >-delta) && (u1<delta) )	{
			beta = u0 / u2;
			if ((beta >=0.0f) && (beta<=1.0f))	{
				alpha = (v0 - beta*v2)/v1;
				inter = ((alpha>=0.0f)&&(alpha+beta<=1.0f));
			}
		} else {

			beta = (v0*u1 - u0*v1) / (v2*u1 - u2*v1);
			if ((beta >=0.0f) && (beta<=1.0f))	{
				Assert(beta != UNINITIALIZED_VALUE);
				alpha = (u0 - beta*u2)/u1;
				inter = ((alpha>=0.0f)&&(alpha+beta<=1.0f));
			}
		}

	} while ((!inter) && (++i < nv) );

	if ( inter &&  uvls && u_out && v_out )	{
		gamma = 1.0f - (alpha+beta);
		*u_out = gamma * uvls[0].u + alpha*uvls[i-1].u + beta*uvls[i].u;
		*v_out = gamma * uvls[0].v + alpha*uvls[i-1].v + beta*uvls[i].v;
	}
	
	return inter;
}

// ****************************************************************************
// 
// SPHERE FACE INTERSECTION CODE
//
// ****************************************************************************

int check_sphere_point( vec3d *point, vec3d *sphere_start, vec3d *sphere_vel, float radius, float *collide_time );

/**
 * Returns whether a sphere hits a given plane in the time [0,1]
 * If two collisions occur, returns earliest legal time
 * returns the intersection point on the plane
 *
 * @param intersect_point		position on plane where sphere makes first contact [if hit_time in range 0-1]
 * @param sphere_center_start	initial sphere center
 * @param sphere_velocity		initial sphere velocity
 * @param sphere_radius			radius of sphere
 * @param plane_normal			normal to the colliding plane
 * @param plane_point			point in the colliding plane
 * @param hit_time				time surface of sphere first hits plane
 * @param crossing_time			time for sphere to cross plane (first to last contact)
 *
 * @return 1 if sphere may be in contact with plane in time range [0-1], 0 otherwise
 */
int fvi_sphere_plane(vec3d *intersect_point, vec3d *sphere_center_start, vec3d *sphere_velocity, float sphere_radius, 
							vec3d *plane_normal, vec3d *plane_point, float *hit_time, float *crossing_time)
{
	float	D, xs0_dot_norm, vs_dot_norm;
	float t1, t2;

	// find the time and position of the ray-plane intersection
	D = -vm_vec_dotprod( plane_normal, plane_point );
	xs0_dot_norm = vm_vec_dotprod( plane_normal, sphere_center_start );
	vs_dot_norm  = vm_vec_dotprod( plane_normal, sphere_velocity);

	// protect against divide by zero
	if (fl_abs(vs_dot_norm) > 1e-30) {
		t1 = (-D - xs0_dot_norm + sphere_radius) / vs_dot_norm;
		t2 = (-D - xs0_dot_norm - sphere_radius) / vs_dot_norm;
	} else {
		return 0;
	}

	// sort t1 < t2
	if (t2 < t1) {
		float temp = t1;
		t1 = t2;
		t2 = temp;
	}

	*hit_time = t1;

	// find hit pos if t1 in range 0-1
	if (t1 > 0 && t1 < 1) {
		vec3d v_temp;
		vm_vec_scale_add( &v_temp, sphere_center_start, sphere_velocity, t1 );
		vm_project_point_onto_plane( intersect_point, &v_temp, plane_normal, plane_point );
	}
	
	// get time to cross
	*crossing_time = t2 - t1;

	return ( (t1 < 1) && (t2 > 0) );
}

/**
 * Returns whether a sphere hits and edge for the case the edge is perpendicular to sphere_velocity
 * If two collisions occur, returns the earliest legal time
 * returns the intersection point on the edge
 *
 * @param intersect_point		position on plane where sphere makes first contact [RESULT]
 * @param sphere_center_start	initial sphere center
 * @param sphere_velocity		initial sphere velocity
 * @param sphere_radius			radius of sphere
 * @param edge_point1			first edge point
 * @param edge_point2			second edge point
 * @param collide_time			actual time of the collision
 */		
int fvi_sphere_perp_edge(vec3d *intersect_point, vec3d *sphere_center_start, vec3d *sphere_velocity,
								 float sphere_radius, vec3d *edge_point1, vec3d *edge_point2, float *collide_time)
{
	// find the intersection in the plane normal to sphere velocity and edge velocity
	// choose a plane point V0 (first vertex of the edge)
	// project vectors and points into the plane
	// find the projection of the intersection and see if it lies on the edge

	vec3d edge_velocity;
	vec3d V0, V1;
	vec3d Xe_proj, Xs_proj;

	V0 = *edge_point1;
	V1 = *edge_point2;
	vm_vec_sub(&edge_velocity, &V1, &V0);

	// define a set of local unit vectors
	vec3d x_hat, y_hat, z_hat;
	float max_edge_parameter;

	vm_vec_copy_normalize( &x_hat, &edge_velocity );
	vm_vec_copy_normalize( &y_hat, sphere_velocity );
	vm_vec_crossprod( &z_hat, &x_hat, &y_hat );
	max_edge_parameter = vm_vec_mag( &edge_velocity );

	vec3d temp;
	// next two temp should be same as starting velocities
	vm_vec_projection_onto_plane(&temp, sphere_velocity, &z_hat);
	Assert ( !vm_vec_cmp(&temp, sphere_velocity) );
	vm_vec_projection_onto_plane(&temp, &edge_velocity,  &z_hat);
	Assert ( !vm_vec_cmp(&temp, &edge_velocity) );

	// should return V0
	vm_project_point_onto_plane(&Xe_proj, &V0, &z_hat, &V0);
	Assert ( !vm_vec_cmp(&Xe_proj, &V0) );

	vm_project_point_onto_plane(&Xs_proj, sphere_center_start, &z_hat, &V0);

	vec3d plane_coord;
	plane_coord.xyz.x = vm_vec_dotprod(&Xs_proj, &x_hat);
	plane_coord.xyz.y = vm_vec_dotprod(&Xe_proj, &y_hat);
	plane_coord.xyz.z = vm_vec_dotprod(&Xe_proj, &z_hat);

	// determime the position on the edge line
	vm_vec_copy_scale( intersect_point, &x_hat, plane_coord.xyz.x );
	vm_vec_scale_add2( intersect_point, &y_hat, plane_coord.xyz.y );
	vm_vec_scale_add2( intersect_point, &z_hat, plane_coord.xyz.z );

	// check if point is actually on edge
	float edge_parameter;
	vec3d temp_vec;

	vm_vec_sub( &temp_vec, intersect_point, &V0 );
	edge_parameter = vm_vec_dotprod( &temp_vec, &x_hat );

	if ( edge_parameter < 0 || edge_parameter > max_edge_parameter ) {
		return 0;
	}

	return ( check_sphere_point(intersect_point, sphere_center_start, sphere_velocity, sphere_radius, collide_time) );
}
	

/**
 * Determines whether and where a moving sphere hits a point
 *
 * @param point			point sphere collides with
 * @param sphere_start	initial sphere center
 * @param sphere_vel	velocity of sphere
 * @param radius		radius of sphere
 * @param collide_time	time of first collision with t >= 0
 */
int check_sphere_point( vec3d *point, vec3d *sphere_start, vec3d *sphere_vel, float radius, float *collide_time )
{
	vec3d delta_x;
	float delta_x_sqr, vs_sqr, delta_x_dot_vs;

	vm_vec_sub( &delta_x, sphere_start, point );
	delta_x_sqr = vm_vec_mag_squared( &delta_x );
	vs_sqr = vm_vec_mag_squared( sphere_vel );
	delta_x_dot_vs = vm_vec_dotprod( &delta_x, sphere_vel );

	float discriminant = delta_x_dot_vs*delta_x_dot_vs - vs_sqr*(delta_x_sqr - radius*radius);
	if (discriminant < 0) {
		return 0;
	}

	float radical, time1, time2;
	radical = fl_sqrt(discriminant);
	time1 = (-delta_x_dot_vs + radical) / vs_sqr;
	time2 = (-delta_x_dot_vs - radical) / vs_sqr;

	if (time1 > time2) {
		float temp = time1;
		time1 = time2;
		time2 = temp;
	}

	if (time1 >= 0 && time1 <= 1.0) {
		*collide_time = time1;
		return 1;
	}

	if (time2 >= 0 && time2 <= 1.0) {
		*collide_time = time2;
		return 1;
	}

	return 0;
}

/**
 * Given a polygon vertex list and a moving sphere, find the first contact the sphere makes with the edge, if any
 * 
 * @param hit_point		point on edge
 * @param xs0			starting point for sphere
 * @param vs			sphere velocity
 * @param Rs			sphere radius
 * @param nv			number of vertices
 * @param verts			vertices making up polygon edges
 * @param hit_time		time the sphere hits an edge
 *
 * @return 1 if sphere hits polyedge, 0 if sphere misses
 */
int fvi_polyedge_sphereline(vec3d *hit_point, vec3d *xs0, vec3d *vs, float Rs, int nv, vec3d **verts, float *hit_time)
{
	int i;
	vec3d v0, v1;
	vec3d ve;						// edge velocity
	float best_sphere_time;		// earliest time sphere hits edge
	vec3d delta_x;
	float delta_x_dot_ve, delta_x_dot_vs, ve_dot_vs, ve_sqr, vs_sqr, delta_x_sqr;
	vec3d temp_edge_hit, temp_sphere_hit;
	float t_sphere_hit = 0.0f;
	float A, B, C, temp, discriminant, inv2A;
	float root, root1, root2;
	float Rs2 = (Rs * Rs);
	float Rs_point2 = (0.2f * Rs);

	best_sphere_time = FLT_MAX;

	vs_sqr = vm_vec_mag_squared(vs);

	for (i = 0; i < nv; i++) {
		// Get vertices of edge to check
		v0 = *verts[i];
		if (i+1 != nv) {
			v1 = *verts[i+1];
		} else {
			v1 = *verts[0];
		}

		// Calculate edge velocity. 
		// Position along the edge is given by: P_edge = v0 + ve*t, where t is in the range [0,1]
		vm_vec_sub(&ve, &v1, &v0);

		// First find the closest intersection between the edge_line and the sphere_line.
		vm_vec_sub(&delta_x, xs0, &v0);

		delta_x_dot_ve = vm_vec_dotprod(&delta_x, &ve);
		delta_x_dot_vs = vm_vec_dotprod(&delta_x, vs);
		delta_x_sqr = vm_vec_mag_squared(&delta_x);
		ve_dot_vs = vm_vec_dotprod(&ve, vs);
		ve_sqr = vm_vec_mag_squared(&ve);

		// solve for sphere time
		A = ve_dot_vs*ve_dot_vs - ve_sqr*vs_sqr;
		B = 2.0f * (delta_x_dot_ve*ve_dot_vs - delta_x_dot_vs*ve_sqr);
		C = delta_x_dot_ve*delta_x_dot_ve + Rs2*ve_sqr - delta_x_sqr*ve_sqr;

		discriminant = B*B - 4.0f*A*C;
		if (discriminant > 0.0f) {
			inv2A = 1.0f / (2.0f * A);
			root = sqrt(discriminant);
			root1 = (-B + root) * inv2A;
			root2 = (-B - root) * inv2A;

			// sort root1 and root2
			if (root2 < root1) {
				temp = root1;
				root1 = root2;
				root2 = temp;
			}

			if ( (root1 < 0.0f) && (root1 >= -0.05f) )
				root1 = 0.000001f;

			// look only at first hit
			if ( (root1 <= 1.0f) && (root1 >= 0.0f) ) {
				t_sphere_hit = root1;
			} else {
				goto TryVertex;
			}
		} else {
			// discriminant negative, so no hit possible
			continue;
		}

		// check if best time with this edge is less than best so far
		if (t_sphere_hit >= best_sphere_time)
			continue;

		vm_vec_scale_add( &temp_sphere_hit, xs0, vs, t_sphere_hit );

		// solve for edge time
		A *= ve_sqr;
		B = 2.0f*ve_sqr * (delta_x_dot_ve*vs_sqr - delta_x_dot_vs*ve_dot_vs);
		C = 2.0f*delta_x_dot_ve*delta_x_dot_vs*ve_dot_vs + Rs2*ve_dot_vs*ve_dot_vs 
			 - delta_x_sqr*ve_dot_vs*ve_dot_vs - delta_x_dot_ve*delta_x_dot_ve*vs_sqr;

		discriminant = B*B - 4.0f*A*C;
		inv2A = 1.0f / (2.0f * A);

		// guard against nearly perpendicular sphere edge velocities
		if (discriminant < 0.0f)
			root = 0.0f;
		else
			root = fl_sqrt(discriminant);

		root1 = (-B + root) * inv2A;
		root2 = (-B - root) * inv2A;

		// given sphere position, find which edge time (position) allows a valid solution
		if ( (root1 <= 1.0f) && (root1 >= 0.0f) ) {
			// try edge root1
			vm_vec_scale_add( &temp_edge_hit, &v0, &ve, root1 );
			float q = vm_vec_dist_squared(&temp_edge_hit, &temp_sphere_hit);
			if ( fl_abs(q - Rs2) < Rs_point2 ) {	// error less than 0.1m absolute (2*delta*Radius)
				goto Hit;
			}
		}

		if ( (root2 <= 1.0f) && (root2 >= 0.0f) ) {
			// try edge root2
			vm_vec_scale_add( &temp_edge_hit, &v0, &ve, root2 );
			float q = vm_vec_dist_squared(&temp_edge_hit, &temp_sphere_hit);
			if ( fl_abs(q - Rs2) < Rs_point2 ) {	// error less than 0.1m absolute
				goto Hit;
			}
		} else {
			// both root1 and root2 out of range so we have to check vertices
			goto TryVertex;
		}

TryVertex:
		// try V0
		A = vs_sqr;
		B = 2.0f*delta_x_dot_vs;
		C = delta_x_sqr - Rs2;
		int v0_hit;
		float sphere_v0, sphere_v1;

		v0_hit = 0;
		sphere_v0 = UNINITIALIZED_VALUE;
		sphere_v1 = UNINITIALIZED_VALUE;

		inv2A = 1.0f / (2.0f * A);
		discriminant = B*B - 4.0f*A*C;
		if (discriminant > 0.0f) {
			root = fl_sqrt(discriminant);
			root1 = (-B + root) * inv2A;
			root2 = (-B - root) * inv2A;

			if (root1 > root2) {
				temp = root1;
				root1 = root2;
				root2 = temp;
			}

			// look only at the first hit  (ignore negative first hit)
			if ( (root1 < 1.0f) && (root1 > 0.0f) ) {
				v0_hit = 1;
				sphere_v0 = root1;
			}
		}

		// try V1 
		vm_vec_sub( &delta_x, xs0, &v1 );
		delta_x_sqr = vm_vec_mag_squared( &delta_x );
		delta_x_dot_vs = vm_vec_dotprod( &delta_x, vs );
		int v1_hit;

		B = 2.0f*delta_x_dot_vs;
		C = delta_x_sqr - Rs2;

		v1_hit = 0;
		discriminant = B*B - 4.0f*A*C;
		if (discriminant > 0.0f) {
			root = fl_sqrt(discriminant);
			root1 = (-B + root) * inv2A;
			root2 = (-B - root) * inv2A;

			if (root1 > root2) {
				temp = root1;
				root1 = root2;
				root2 = temp;
			}

			// look only at the first hit (ignore negative first hit)
			if ( (root1 < 1.0f) && (root1 > 0.0f) ) {
				v1_hit = 1;
				sphere_v1 = root1;
			}
		}

		// set hitpoint to closest vetex hit, if any
		if ( v0_hit ) {
			Assert(sphere_v0 != UNINITIALIZED_VALUE);
			t_sphere_hit = sphere_v0;
			temp_edge_hit = v0;

			if (v1_hit) {
				Assert( sphere_v1 != UNINITIALIZED_VALUE );
				if (sphere_v1 < sphere_v0) {
					t_sphere_hit = sphere_v1;
					temp_edge_hit = v1;
				}
			}
		} else if ( v1_hit ) {
			Assert(sphere_v1 != UNINITIALIZED_VALUE);
			t_sphere_hit = sphere_v1;
			temp_edge_hit = v1;
		} else {
			continue;
		}

		//vm_vec_scale_add( &temp_sphere_hit, xs0, vs, t_sphere_hit );
		//q = vm_vec_dist_squared(&temp_edge_hit, &temp_sphere_hit);


Hit:
		if (t_sphere_hit < best_sphere_time) {
			best_sphere_time = t_sphere_hit;
			*hit_point = temp_edge_hit;
		}
	}	// end edge loop

	if (best_sphere_time <= 1.0f) {
		*hit_time = best_sphere_time;
		return 1;
	} else {
		return 0;
	}
}

/**
 * Finds the closest point on a line to a given fixed point
 *
 * @param closest_point		the closest point on the line
 * @param fixed_point		the fixed point
 * @param line_point1		first point on the line
 * @param line_point2		second point on the line
 */
void fvi_closest_point_on_line_segment(vec3d *closest_point, vec3d *fixed_point, vec3d *line_point1, vec3d *line_point2)
{
	vec3d delta_x, line_velocity;
	float t;

	vm_vec_sub(&line_velocity, line_point2, line_point1);
	vm_vec_sub(&delta_x, line_point1, fixed_point);
	t = -vm_vec_dotprod(&delta_x, &line_velocity) / vm_vec_mag_squared(&line_velocity);

	// Constrain t to be in range [0,1]
	if (t < 0) {
		t = 0.0f;
	} else if (t > 1) {
		t = 1.0f;
	}

	vm_vec_scale_add(closest_point, line_point1, &line_velocity, t);
}

/**
 * checks whether two spheres hit given initial and starting positions and radii
 * does not check whether sphere are already touching.
 *
 * @param x_p0		polymodel sphere, start point
 * @param x_p1		polymodel sphere, end point
 * @param x_s0		other sphere, start point
 * @param x_s1		other sphere, end point
 * @param radius_p	radius of polymodel sphere
 * @param radius_s	radius of other sphere
 * @param t1		time pointer 1
 * @param t2		time pointer 2
 *
 * @return 1 if spheres overlap, 0 otherwise
 */
int fvi_check_sphere_sphere(vec3d *x_p0, vec3d *x_p1, vec3d *x_s0, vec3d *x_s1, float radius_p, float radius_s, float *t1, float *t2)
{
	vec3d delta_x, delta_v;
	float discriminant, separation, delta_x_dot_delta_v, delta_v_sqr, delta_x_sqr;
	float time1, time2;

	// Check that there are either 0 or 2 pointers to time
	Assert( (!(t1) && !(t2)) || (t1 && t2) );

	vm_vec_sub(&delta_x, x_s0, x_p0);
	delta_x_sqr = vm_vec_mag_squared(&delta_x);
	separation = radius_p + radius_s;

	// Check if already touching
	if (delta_x_sqr < separation*separation) {
		 if ( !t1 ) {
			return 1;
		 }
	}

	// Find delta_v (in polymodel sphere frame of ref)
	// Note: x_p0 and x_p1 will be same for fixed polymodel
	vm_vec_sub(&delta_v, x_s1, x_s0);
	vm_vec_add2(&delta_v, x_p0);
	vm_vec_sub2(&delta_v, x_p1);

	delta_x_dot_delta_v = vm_vec_dotprod(&delta_x, &delta_v);
	delta_v_sqr = vm_vec_mag_squared(&delta_v);

	discriminant = delta_x_dot_delta_v*delta_x_dot_delta_v - delta_v_sqr*(delta_x_sqr - separation*separation);

	if (discriminant < 0) {
		return 0;
	}

	float radical = fl_sqrt(discriminant);

	time1 = (-delta_x_dot_delta_v + radical) / delta_v_sqr;
	time2 = (-delta_x_dot_delta_v - radical) / delta_v_sqr;

	// sort t1 < t2
	float temp;
	if (time1 > time2) {
		temp  = time1;
		time1 = time2;
		time2 = temp;
	}

	if ( t1 ) {
		*t1 = time1;
		*t2 = time2;
	}

	// check whether the range from t1 to t2 intersects [0,1]
	if (time1 > 1 || time2 < 0) {
		return 0;
	} else {
		return 1;
	}
}

/**
 * Culls polyfaces which moving sphere can not intersect
 *
 * Polygon face is characterized by a center and a radius.  This routine checks whether it is 
 * *impossible* for a moving sphere to intersect a fixed polygon face.
 *
 * @param poly_center	center of polygon face to test
 * @param poly_r		radius of polygon face in question
 * @param sphere_start	start point of moving sphere
 * @param sphere_end	end point of moving sphere
 * @param sphere_r		radius of moving sphere
 *
 * @return 0 if no collision is possible, 1 if collision may be possible
 */
int fvi_cull_polyface_sphere(vec3d *poly_center, float poly_r, vec3d *sphere_start, vec3d *sphere_end, float sphere_r)
{
	vec3d closest_point, closest_separation;
	float max_sep;

	fvi_closest_point_on_line_segment(&closest_point, poly_center, sphere_start, sphere_end);
	vm_vec_sub(&closest_separation, &closest_point, poly_center);

	max_sep = vm_vec_mag(&closest_separation) + poly_r;

	if ( max_sep > sphere_r ) {
		return 0;
	} else {
		return 1;
	}
}

/**
 * Finds the closest points between two lines
 */
void fvi_closest_line_line( vec3d *x0, vec3d *vx, vec3d *y0, vec3d *vy, float *x_time, float *y_time )
{
	vec3d vx_cross_vy, delta_l, delta_l_cross_vx, delta_l_cross_vy;
	float denominator;

	vm_vec_sub(&delta_l, y0, x0);

	vm_vec_crossprod(&vx_cross_vy, vx, vy);
	vm_vec_crossprod(&delta_l_cross_vx, &delta_l, vx);
	vm_vec_crossprod(&delta_l_cross_vy, &delta_l, vy);

	denominator = vm_vec_mag_squared(&vx_cross_vy);

	*x_time = vm_vec_dotprod(&delta_l_cross_vy, &vx_cross_vy) / denominator; 
	*y_time = vm_vec_dotprod(&delta_l_cross_vx, &vx_cross_vy) / denominator; 
}

// --------------------------------------------------------------------------------------------------------------------
void accurate_square_root( float A, float B, float C, float discriminant, float *root1, float *root2 )
{
	float root = fl_sqrt(discriminant);

	if (B > 0) {
		*root1 = 2.0f*C / (-B - root);
		*root2 = (-B - root) / (2.0f*A);
	} else {	// B < 0
		*root1 = (-B + root) / (2.0f*A);
		*root2 = 2.0f*C / (-B + root);
	}
}

/**
 * Project point on bounding box
 *
 * NOTE: if a coordinate of start is *inside* the bbox, it is *not* moved to surface of bbox
 *
 * @param mins		minimum extents of bbox
 * @param maxs		maximum extents of bbox
 * @param start		point in bbox reference frame
 * @param box_pt	point in bbox reference frame.
 *
 * @return 1 if inside, 0 otherwise.
 */
int project_point_onto_bbox(vec3d *mins, vec3d *maxs, vec3d *start, vec3d *box_pt)
{
	int inside = TRUE;

	if (start->xyz.x > maxs->xyz.x) {
		box_pt->xyz.x = maxs->xyz.x;
		inside = FALSE;
	} else if (start->xyz.x < mins->xyz.x) {
		box_pt->xyz.x = mins->xyz.x;
		inside = FALSE;
	} else {
		box_pt->xyz.x = start->xyz.x;
	}

	if (start->xyz.y > maxs->xyz.y) {
		box_pt->xyz.y = maxs->xyz.y;
		inside = FALSE;
	} else if (start->xyz.y < mins->xyz.y) {
		box_pt->xyz.y = mins->xyz.y;
		inside = FALSE;
	} else {
		box_pt->xyz.y = start->xyz.y;
	}

	if (start->xyz.z > maxs->xyz.z) {
		box_pt->xyz.z = maxs->xyz.z;
		inside = FALSE;
	} else if (start->xyz.z < mins->xyz.z) {
		box_pt->xyz.z = mins->xyz.z;
		inside = FALSE;
	} else {
		box_pt->xyz.z = start->xyz.z;
	}

	return inside;
}
