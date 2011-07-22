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

// lines: L1 = P1 + V1s  and   L2 = P2 + V2t
// finds the point on each line of closest approach (s and t) (lines need not intersect to get closest point)
// taken from graphic gems I, p. 304
// 
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

//tells distance from a plain to a point-Bobboau
float fvi_point_dist_plane(	vec3d *plane_pnt, vec3d *plane_norm,		// Plane description, a point and a normal
					    vec3d *point	//a point to test
						)
{
	float dist,D;
		
	D = -vm_vec_dot(plane_norm,plane_pnt);

	dist = vm_vec_dot(plane_norm, point) + D;
	return dist;
}




//--------------------------------------------------------------------
// fvi_ray_plane - Finds the point on the specified plane where the 
// infinite ray intersects.
//
// Returns scaled-distance plane is from the ray_origin (t), so
// P = O + t*D, where P is the point of intersection, O is the ray origin,
// and D is the ray's direction.  So 0.0 would mean the intersection is 
// exactly on the ray origin, 1.0 would be on the ray origin plus the ray
// direction vector, anything negative would be behind the ray's origin.
// If you pass a pointer to the new_pnt, this routine will perform the P=
// calculation to calculate the point of intersection and put the result
// in *new_pnt.
//
// If radius is anything other than 0.0, it assumes you want the intersection
// point to be that radius from the plane.
//
// Note that ray_direction doesn't have to be normalized unless you want
// the return value to be in units from the ray origin.
//
// Also note that new_pnt will always be filled in to some valid value,
// even if it is a point at infinity.   
//
// If the plane and line are parallel, this will return the largest 
// negative float number possible.
// 
// So if you want to check a line segment from p0 to p1, you would pass
// p0 as ray_origin, p1-p0 as the ray_direction, and there would be an
// intersection if the return value is between 0 and 1.

float fvi_ray_plane(vec3d *new_pnt,
                    vec3d *plane_pnt,vec3d *plane_norm,		// Plane description, a point and a normal
                    vec3d *ray_origin,vec3d *ray_direction,	// Ray description, a point and a direction
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


//find the point on the specified plane where the line intersects
//returns true if point found, false if line parallel to plane
//new_pnt is the found point on the plane
//plane_pnt & plane_norm describe the plane
//p0 & p1 are the ends of the line.  
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




//maybe this routine should just return the distance and let the caller
//decide it it's close enough to hit
//determine if and where a vector intersects with a sphere
//vector defined by p0,p1 
//returns 1 if intersects, and fills in intp
//else returns 0
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
		// int_dist is dist
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

//		{
//			fix dd = vm_vec_dist(intp,sphere_pos);
//			Assert(dd == sphere_rad);
//			mprintf(0,"dd=%x, rad=%x, delta=%x\n",dd,sphere_rad,dd-sphere_rad);
//		}

		return 1;
	}
	else
		return 0;
}


//determine if and where a ray intersects with a sphere
//vector defined by p0,p1 
//returns 1 if intersects, and fills in intp
//else returns 0
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
		// int_dist is dist
	}

	// normalize dn
	dn.xyz.x = d.xyz.x / mag_d;
	dn.xyz.y = d.xyz.y / mag_d;
	dn.xyz.z = d.xyz.z / mag_d;

	w_dist = vm_vec_dot(&dn,&w);

	if (w_dist < -sphere_rad)		//moving away from object
		 return 0;

//	if (w_dist > mag_d+sphere_rad)
//		return 0;		//cannot hit

	vm_vec_scale_add(&closest_point,p0,&dn,w_dist);

	dist = vm_vec_dist(&closest_point,sphere_pos);

	if (dist < sphere_rad) {
		float dist2,rad2,shorten;

		dist2 = dist*dist;
		rad2 = sphere_rad*sphere_rad;

		shorten = fl_sqrt(rad2 - dist2);

		int_dist = w_dist-shorten;

//		if (int_dist > mag_d || int_dist < 0.0f) {
		if (int_dist < 0.0f) {
			//past one or the begining of vector, which means we're inside

			*intp = *p0;		//don't move at all
			return 1;
		}

		vm_vec_scale_add(intp,p0,&dn,int_dist);         //calc intersection point

//		{
//			fix dd = vm_vec_dist(intp,sphere_pos);
//			Assert(dd == sphere_rad);
//			mprintf(0,"dd=%x, rad=%x, delta=%x\n",dd,sphere_rad,dd-sphere_rad);
//		}

		return 1;
	}
	else
		return 0;
}


//==============================================================
// Finds intersection of a ray and an axis-aligned bounding box
// Given a ray with origin at p0, and direction pdir, this function
// returns non-zero if that ray intersects an axis-aligned bounding box
// from min to max.   If there was an intersection, then hitpt will contain
// the point where the ray begins inside the box.
// Fast ray-box intersection taken from Graphics Gems I, pages 395,736.
int fvi_ray_boundingbox( vec3d *min, vec3d *max, vec3d * p0, vec3d *pdir, vec3d *hitpt )
{
	bool inside = true;
	bool middle[3] = { true, true, true };
	int i;
	int which_plane;
	float maxt[3];
	float candidate_plane[3];

	for (i = 0; i < 3; i++) {
		if (p0->a1d[i] < min->a1d[i]) {
			candidate_plane[i] = min->a1d[i];
			middle[i] = false;
			inside = false;
		} else if (p0->a1d[i] > max->a1d[i]) {
			candidate_plane[i] = max->a1d[i];
			middle[i] = false;
			inside = false;
		}
	}

	// ray origin inside bounding box			
	if ( inside ) {
		*hitpt = *p0;
		return 1;
	}

	// calculate T distances to canditate plane
	for (i = 0; i < 3; i++) {
		if ( !middle[i] && (pdir->a1d[i] != 0.0f) )
			maxt[i] = (candidate_plane[i] - p0->a1d[i]) / pdir->a1d[i];
		else
			maxt[i] = -1.0f;
	}

	// Get largest of the maxt's for final choice of intersection
	which_plane = 0;
	for (i = 1; i < 3; i++) {
		if (maxt[which_plane] < maxt[i])
			which_plane = i;
	}

	// check final candidate actually inside box
	if (maxt[which_plane] < 0.0f)
		return 0;

	for (i = 0; i < 3; i++) {
		if (which_plane != i) {
			hitpt->a1d[i] = p0->a1d[i] + maxt[which_plane] * pdir->a1d[i];

			if ( (hitpt->a1d[i] < min->a1d[i]) || (hitpt->a1d[i] > max->a1d[i]) )
				return 0;
		} else {
			hitpt->a1d[i] = candidate_plane[i];
		}
	}

	return 1;
}



//given largest componant of normal, return i & j
//if largest componant is negative, swap i & j
int ij_table[3][2] =        {
							{2,1},          //pos x biggest
							{0,2},          //pos y biggest
							{1,0},          //pos z biggest
						};

// fvi_point_face
// see if a point in inside a face by projecting into 2d. Also
// Finds uv's if uvls is not NULL.  Returns 0 if point isn't on
// face, non-zero otherwise.
// From Graphics Gems I, "An efficient Ray-Polygon intersection", p390
// checkp - The point to check
// nv - how many verts in the poly
// verts - the vertives for the polygon 
// norm1 - the polygon's normal
// u_out,vout - if not null and v_out not null and uvls not_null and point is on face, the uv's of where it hit
// uvls - a list of uv pairs for each vertex
// This replaces the old check_point_to_face & find_hitpoint_uv
// WARNING!!   In Gems, they use the code "if (u1==0)" in this function.
// I have found several cases where this will not detect collisions it should.
// I found two solutions:
//   1. Declare the 'beta' variable to be a double.
//   2. Instead of using 'if (u1==0)', compare it to a small value.
// I chose #2 because I would rather have our code work with all floats
// and never need doubles.   -JAS Aug22,1997
#define delta 0.0001f
#define	UNINITIALIZED_VALUE	-1234567.8f

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

/*
		// This is test code that I used to detect failures.  See the
		// comments above this function for details.
		double betad;
		float alphad;
		int interd=0;

		betad = (v0*u1 - u0*v1) / (v2*u1 - u2*v1);
		if ((betad >=0.0f) && (betad<=1.0f))	{
			alphad = (u0 - betad*u2)/u1;
			interd = ((alphad>=0.0f)&&(alphad+betad<=1.0f));
		}

		if ( interd != inter )	{
			mprintf(( "u0=%.4f, u1=%.16f, u2=%.4f\n", u0, u1, u2 ));
			mprintf(( "v0=%.4f, v1=%.16f, v2=%.4f\n", v0, v1, v2 ));
		}
*/

	} while ((!inter) && (++i < nv) );

	if ( inter &&  uvls && u_out && v_out )	{
		// Assert(alpha != 1.0f);
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

// ----------------------------------------------------------------------------
// fvi_sphere_plane()
// returns whether a sphere hits a given plane in the time [0,1]
// if two collisions occur, returns earliest legal time
// returns the intersection point on the plane
//
//		inputs:	intersect_point		=>		position on plane where sphere makes first contact [if hit_time in range 0-1]
//					sphere_center_start	=>		initial sphere center
//					sphere_velocity		=>		initial sphere velocity
//					sphere_radius			=>		radius of sphere
//					plane_normal			=>		normal to the colliding plane
//					plane_point				=>		point in the colliding plane
//					hit_time					=>		time surface of sphere first hits plane
//					delta_t					=>		time for sphere to cross plane (first to last contact)
//
//		return:	1 if sphere may be in contact with plane in time range [0-1], 0 otherwise
//

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

// ----------------------------------------------------------------------------
// fvi_sphere_perp_edge()
//	returns whether a sphere hits and edge for the case the edge is perpendicular to sphere_velocity
// if two collisions occur, returns the earliest legal time
// returns the intersection point on the edge
//
//		inputs:	intersect_point		=>		position on plane where sphere makes first contact [RESULT]
//					sphere_center_start	=>		initial sphere center
//					sphere_velocity		=>		initial sphere velocity
//					sphere_radius			=>		radius of sphere
//					edge_point1				=>		first edge point
//					edge_point2				=>		second edge point
//					max_time					=>		maximum legal time at which collision can occur
//					collide_time			=>		actual time of the collision
//		
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
	

// ----------------------------------------------------------------------------
// check_sphere_point()
// determines whether and where a moving sphere hits a point
//
//			inputs:		point				=>		point sphere collides with
//							sphere_start	=>		initial sphere center
//							sphere_vel		=>		velocity of sphere
//							radius			=>		radius of sphere
//							collide_time	=>		time of first collision with t >= 0
//
int check_sphere_point( vec3d *point, vec3d *sphere_start, vec3d *sphere_vel, float radius, float *collide_time )
{
	vec3d delta_x;
	float delta_x_sqr, vs_sqr, delta_x_dot_vs;

	vm_vec_sub( &delta_x, sphere_start, point );
	delta_x_sqr = vm_vec_mag_squared( &delta_x );
	vs_sqr = vm_vec_mag_squared( sphere_vel );
	delta_x_dot_vs = vm_vec_dotprod( &delta_x, sphere_vel );

//	a = vs_sqr;
//	b = 2.0f*delta_x_dot_vs;
//	c = delta_x_sqr - radius*radius;

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



// ----------------------------------------------------------------------------
//
// fvi_polyedge_sphereline()
// 
// Given a polygon vertex list and a moving sphere, find the first contact the sphere makes with the edge, if any
// 
//		Inputs:	hit_point		=>		point on edge
//					xs0				=>		starting point for sphere
//					vs					=>		sphere velocity
//					Rs					=>		sphere radius
//					nv					=>		number of vertices
//					verts				=>		vertices making up polygon edges
//					hit_time			=>		time the sphere hits an edge
//
//		Return:	1 if sphere hits polyedge, 0 if sphere misses
/*
#define TOL 1E-3

int fvi_polyedge_sphereline(vec3d *hit_point, vec3d *xs0, vec3d *vs, float Rs, int nv, vec3d **verts, float *hit_time)
{
	int i;
	vec3d v0, v1;
	vec3d ve;						// edge velocity
	float best_sphere_time;		// earliest time sphere hits edge
	vec3d delta_x;
	float	delta_x_dot_ve, delta_x_dot_vs, ve_dot_vs, ve_sqr, vs_sqr;
	float denominator;
	float time_el, time_sl;		// times for edge_line and sphere_line at closest approach
	vec3d temp_edge_hit, temp_sphere_hit;
	vec3d best_edge_hit;		// edge position for earliest edge hit

	best_sphere_time = FLT_MAX;

	for (i=0; i<nv; i++) {
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
		vm_vec_sub(&delta_x, &v0, xs0);
		delta_x_dot_ve = vm_vec_dotprod(&delta_x, &ve);


		delta_x_dot_vs = vm_vec_dotprod(&delta_x, vs);
		ve_dot_vs = vm_vec_dotprod(&ve, vs);
		ve_sqr = vm_vec_mag_squared(&ve);
		vs_sqr = vm_vec_mag_squared(vs);


		denominator = ve_dot_vs*ve_dot_vs - ve_sqr*vs_sqr;
		if (fl_abs(denominator) < SMALL_NUM) {		// check for parallel linesg
			continue;										// would have to hit at vertex 
		}														// and another edge will not be so parallel

		// Compute time (and position) along the edge_line and sphere_line at closest approach.
		time_el = (delta_x_dot_ve*vs_sqr - ve_dot_vs*delta_x_dot_vs) / denominator;
		time_sl = (delta_x_dot_vs + ve_dot_vs*time_el) / vs_sqr;

		// DA: 12/5/97  I checked above procedure for calculating line intersection against fvi_closest_line_line()
		// fvi_closest_line_line appears to have much lower accuracy as many edges collisions were missed

		vm_vec_scale_add(&temp_edge_hit, &v0, &ve, time_el);
		vm_vec_scale_add(&temp_sphere_hit, xs0, vs, time_sl);

		// Compute distance squared at closest approach.
		vec3d diff;
		float  d0_sqr;
		vm_vec_sub(&diff, &temp_sphere_hit, &temp_edge_hit);
		d0_sqr = vm_vec_mag_squared(&diff);

		if (d0_sqr > Rs*Rs) {
			continue;
		} 

		// Starting from the positions of closest approach, back up the sphere until it is just touching the edge_line.
		// Check this edge_line point against the range of the edge.  If not in range, check if the sphere hits the 
		// extreme of the edge.

		//	time_e1 - the edge_line position of closest approach
		//	time_e  - the edge position where sphere makes first contact
		float dist_e_sqr, dist_s_sqr, delta_te, delta_ts, cos_sqr;
		float time_s;
		float time_em, time_ep, time_sm, time_sp;
		float te_per_ts;
		cos_sqr = (ve_dot_vs*ve_dot_vs) / (ve_sqr*vs_sqr);
		dist_s_sqr = (Rs*Rs - d0_sqr) / (1.0f - cos_sqr);
		delta_ts = fl_sqrt(dist_s_sqr / vs_sqr);
		time_sm = time_sl - delta_ts;
		time_sp = time_sl + delta_ts;

		if (time_sm > 1 || time_sp < 0) {
			continue;
		}

		dist_e_sqr = (Rs*Rs - d0_sqr) * cos_sqr / (1.0f - cos_sqr);
		delta_te = fl_sqrt(dist_e_sqr / ve_sqr);
		time_em = time_el - delta_te;
		time_ep = time_el + delta_te;

		if (cos_sqr > 0.5f) {
			if (time_em > 1 || time_ep < 0) {
				continue;
			}
		} else {
			delta_te = fl_sqrt( (Rs*Rs - d0_sqr) / ve_sqr );
			if ((time_el - delta_te) > 1 || (time_el + delta_te) <  0) {
				continue;
			}
		}

		// Check if we already have a hit
		// Move sphere back to time_sm.  If ve_dot_vs > 0 edge to time_em, < 0 time_ep.

		if (time_sm >= 0 && time_sm <= 1) {
			if (ve_dot_vs > 0) {
				if (time_em >= 0 && time_em <= 1) {
					vm_vec_scale_add( &temp_edge_hit, &v0, &ve, time_em );
					time_s = time_sm;
					goto Hit;
				}
			} else {
				if (time_ep >= 0 && time_ep <= 1) {
					vm_vec_scale_add( &temp_edge_hit, &v0, &ve, time_ep );
					time_s = time_sm;
					goto Hit;
				}
			}
		}

		// Find the ratio of times between sphere_line and edge_line.
		te_per_ts = fl_sqrt( vs_sqr / ve_sqr );

		// Check the location of the closest point on the edge line corresponding to the first valid point on sphere_line.
		// First valid sphere_line point is the greater of (1) t = 0 or (2) t = time_sm.
		// If the corresponding edge interval is left, we check against the rightmost edgepoint.
		// If the corresponding edge interval is right, we check against the leftmost edgepoint.
		// If the corresponding edge interval contains this point, then we are already penetrating.
		float first_valid_sphere_time;
		float closest_edge_time;

		// Find first valid sphere time
		if (time_sm < 0) {
			first_valid_sphere_time = 0.0f;
		} else {
			first_valid_sphere_time = time_sm;
			Assert( time_sm <= 1.0f );
		}

		if (ve_dot_vs > 0) {

			// Find corresponding edge time
			closest_edge_time = time_el - te_per_ts * (time_sl - first_valid_sphere_time) * fl_sqrt(cos_sqr);
			if (time_em < 0) {
				time_em = 0.0f;
			}
			if (time_ep > 1) {
				time_ep = 1.0f;
			}

			if (time_em > closest_edge_time - SMALL_NUM) {
				// edge interval is right so test against left edge
				vm_vec_scale_add( &temp_edge_hit, &v0, &ve, time_em );
				if ( !check_sphere_point( &temp_edge_hit, xs0, vs, Rs, &time_s ) ) {
					continue;
				}

			} else if (time_ep < closest_edge_time + SMALL_NUM) {
				// edge interval is left so test against right edge
				vm_vec_scale_add( &temp_edge_hit, &v0, &ve, time_ep );
				if ( !check_sphere_point( &temp_edge_hit, xs0, vs, Rs, &time_s ) ) {
					continue;
				}

			} else {
				// edge interval contains point, so already penetrating
				continue;
			}
		} else {
			// Sphere and edge have opposite velocities

			// Find corresponding edge time
			closest_edge_time = time_el + te_per_ts * (time_sl - first_valid_sphere_time) * fl_sqrt(cos_sqr);
			if (time_em < 0) {
				time_em = 0.0f;
			}
			if (time_ep > 1) {
				time_ep = 1.0f;
			}

			if (closest_edge_time > time_ep - SMALL_NUM) {
				// edge interval is right so test against left edge
				vm_vec_scale_add( &temp_edge_hit, &v0, &ve, time_ep );
				if ( !check_sphere_point( &temp_edge_hit, xs0, vs, Rs, &time_s ) ) {
					continue;
				}

			} else if (closest_edge_time < time_em + SMALL_NUM) {
				// edge interval is left so test against right edge
				vm_vec_scale_add( &temp_edge_hit, &v0, &ve, time_em );
				if ( !check_sphere_point( &temp_edge_hit, xs0, vs, Rs, &time_s ) ) {
					continue;
				}

			} else {
				// edge interval contains point, so already penetrating
				continue;
			}
		}

Hit:
//		vec3d temp;
//		vm_vec_scale_add( &temp, xs0, vs, time_s);
//		float q = vm_vec_dist( &temp, &temp_edge_hit );
//		if (q > Rs + .003 || q < Rs - .003) {
//			Int3();
//		}
		if (time_s < best_sphere_time) {
			best_sphere_time = time_s;
			best_edge_hit = temp_edge_hit;
		}
	}	// end edge loop

	if (best_sphere_time <= 1.0f) {
		*hit_time = best_sphere_time;
		*hit_point = best_edge_hit;
		return 1;
	} else {
		return 0;
	}
}
*/
// ----------------------------------------------------------------------------
//
// fvi_polyedge_sphereline()
// 
// Given a polygon vertex list and a moving sphere, find the first contact the sphere makes with the edge, if any
// 
//		Inputs:	hit_point		=>		point on edge
//					xs0				=>		starting point for sphere
//					vs					=>		sphere velocity
//					Rs					=>		sphere radius
//					nv					=>		number of vertices
//					verts				=>		vertices making up polygon edges
//					hit_time			=>		time the sphere hits an edge
//
//		Return:	1 if sphere hits polyedge, 0 if sphere misses

#define WARN_DIST	1.0

int fvi_polyedge_sphereline(vec3d *hit_point, vec3d *xs0, vec3d *vs, float Rs, int nv, vec3d **verts, float *hit_time)
{
	int i;
	vec3d v0, v1;
	vec3d ve;						// edge velocity
	float best_sphere_time;		// earliest time sphere hits edge
	vec3d delta_x;
	float delta_x_dot_ve, delta_x_dot_vs, ve_dot_vs, ve_sqr, vs_sqr, delta_x_sqr, inv2A;
	vec3d temp_edge_hit, temp_sphere_hit;

	best_sphere_time = FLT_MAX;

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
		vs_sqr = vm_vec_mag_squared(vs);

		float t_sphere_hit, temp;

		// solve for sphere time
		float A, B, C, root, discriminant;	//WMC - changed to float 4/15/2005
		float root1, root2;
		A = ve_dot_vs*ve_dot_vs - ve_sqr*vs_sqr;
		B = 2 * (delta_x_dot_ve*ve_dot_vs - delta_x_dot_vs*ve_sqr);
		C = delta_x_dot_ve*delta_x_dot_ve + Rs*Rs*ve_sqr - delta_x_sqr*ve_sqr;

		discriminant = B*B - 4*A*C;
		if (discriminant > 0) {
			root = fl_sqrt(discriminant);
			inv2A = 1.0f/(2*A);
			root1 = (float) ((-B + root)*inv2A);
			root2 = (float) ((-B - root)*inv2A);

			// sort root1 and root2
			if (root2 < root1) {
				temp = root1;
				root1 = root2;
				root2 = temp;
			}

			if ( (root1 >= -0.05f) && (root1 < 0.0f) ) {
				root1 = 0.000001f;
			}

			// look only at first hit
			if ( (root1 >= 0) && (root1 <= 1) ) {
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
		float q;
		// solve for edge time
		A = ve_sqr * (ve_dot_vs*ve_dot_vs - ve_sqr*vs_sqr);
		B = 2*ve_sqr * (delta_x_dot_ve*vs_sqr - delta_x_dot_vs*ve_dot_vs);
		C = 2*delta_x_dot_ve*delta_x_dot_vs*ve_dot_vs + Rs*Rs*ve_dot_vs*ve_dot_vs 
			 - delta_x_sqr*ve_dot_vs*ve_dot_vs - delta_x_dot_ve*delta_x_dot_ve*vs_sqr;

		discriminant = B*B - 4*A*C;

		// guard against nearly perpendicular sphere edge velocities
		if ( (discriminant < 0) ) {
			discriminant = 0;
		}

		root = fl_sqrt(discriminant);
		inv2A = 1.0f/(2*A);
		root1 = (float) ((-B + root)*inv2A);
		root2 = (float) ((-B - root)*inv2A);

		// given sphere position, find which edge time (position) allows a valid solution
		if ( (root1 >= 0) && (root1 <= 1) ) {
			// try edge root1
			vm_vec_scale_add( &temp_edge_hit, &v0, &ve, root1 );
			q = vm_vec_dist_squared(&temp_edge_hit, &temp_sphere_hit);
			if ( fl_abs(q - Rs*Rs) < 0.2*Rs ) {	// error less than 0.1m absolute (2*delta*Radius)
				goto Hit;
			}
		}

		if ( (root2 >= 0) && (root2 <= 1) ) {
			// try edge root2
			vm_vec_scale_add( &temp_edge_hit, &v0, &ve, root2 );
			q = vm_vec_dist_squared(&temp_edge_hit, &temp_sphere_hit);
			if ( fl_abs(q - Rs*Rs) < 0.2*Rs ) {	// error less than 0.1m absolute
				goto Hit;
			}
		} else {
			// both root1 and root2 out of range so we have to check vertices
			goto TryVertex;
		}

		// Misses EDGE, so try ENDPOINTS
		// Not exactly sure about this part (ie, which endpoint to check)

		// CHECK V0, we don't need to recalculate delta_x
		// CHECK V1, we *need* to recalculate delat_x

		// check end points

TryVertex:
		// try V0
		A = vs_sqr;
		B = 2*delta_x_dot_vs;
		C = delta_x_sqr - Rs*Rs;
		int v0_hit;
		float sphere_v0, sphere_v1;

		sphere_v0 = UNINITIALIZED_VALUE;
		sphere_v1 = UNINITIALIZED_VALUE;

		v0_hit = 0;
		discriminant = B*B - 4*A*C;
		if (discriminant > 0) {
			root = fl_sqrt(discriminant);
			inv2A = 1.0f/(2*A);
			root1 = (float) ((-B + root)*inv2A);
			root2 = (float) ((-B - root)*inv2A);

			if (root1 > root2) {
				temp = root1;
				root1 = root2;
				root2 = temp;
			}

			// look only at the fist hit  (ignore negative first hit)
			if ( (root1 > 0) && (root1 < 1) ) {
				v0_hit = 1;
				sphere_v0 = root1;
				vm_vec_scale_add( &temp_sphere_hit, xs0, vs, root1 );
			}
		}

		// try V1 
		vm_vec_sub( &delta_x, xs0, &v1 );
		delta_x_sqr = vm_vec_mag_squared( &delta_x );
		delta_x_dot_vs = vm_vec_dotprod( &delta_x, vs );
		int v1_hit;

		B = 2*delta_x_dot_vs;
		C = delta_x_sqr - Rs*Rs;

		v1_hit = 0;
		discriminant = B*B - 4*A*C;
		if (discriminant > 0) {
			root = fl_sqrt(discriminant);
			inv2A = 1.0f/(2*A);
			root1 = (float) ((-B + root)*inv2A);
			root2 = (float) ((-B - root)*inv2A);

			if (root1 > root2) {
				temp = root1;
				root1 = root2;
				root2 = temp;
			}

			// look only at the first hit (ignore negative first hit)
			if ( (root1 > 0) && (root1 < 1) ) {
				v1_hit = 1;
				sphere_v1 = root1;
				vm_vec_scale_add( &temp_sphere_hit, xs0, vs, root1 );
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

		vm_vec_scale_add( &temp_sphere_hit, xs0, vs, t_sphere_hit );
		q = vm_vec_dist_squared(&temp_edge_hit, &temp_sphere_hit);


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


// ----------------------------------------------------------------------------
//
//	fvi_closest_point_on_line_segment()
//
// Finds the closest point on a line to a given fixed point
//
//		Inputs:	closest_point	=>		the closest point on the line
//					fixed_point		=>		the fixed point
//					line_point1		=>		first point on the line
//					line_point2		=>		second point on the line
//
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


// ----------------------------------------------------------------------------
//
//	fvi_check_sphere_sphere()
//
//	checks whether two spheres hit given initial and starting positions and radii
// does not check whether sphere are already touching.
//
//		Inputs	x_p0			=>		polymodel sphere, start point
//					x_p1			=>		polymodel sphere, end point
//					x_s0			=>		other sphere, start point
//					x_s1			=>		other sphere, end point
//					radius_p		=>		radius of polymodel sphere
//					radius_s		=>		radius of other sphere
//
//		returns 1 if spheres overlap, 0 otherwise
//
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

// ----------------------------------------------------------------------------
//
//	fvi_cull_polyface_sphere()
//
// Culls polyfaces which moving sphere can not intersect
//
//		Inputs:		poly_center		=>		center of polygon face to test
//						poly_r			=>		radius of polygon face in question
//						sphere_start	=>		start point of moving sphere
//						sphere_end		=>		end point of moving sphere
//						sphere_r			=>		radius of moving sphere
//
//		Output:		returns 0 if no collision is possible, 1 if collision may be possible
//
//		Polygon face is characterized by a center and a radius.  This routine checks whether it is 
//		*impossible* for a moving sphere to intersect a fixed polygon face.
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

// ---------------------------------------------------------------------------------------------------------------------
// fvi_closest_line_line
// finds the closest points between two lines
//
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

//	vec3d x_result, y_result;
//	vm_vec_scale_add(&x_result, x0, vx, *x_time);
//	vm_vec_scale_add(&y_result, y0, vy, *y_time);
//	*dist_sqr = vm_vec_dist_squared(&x_result, &y_result);

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

// vec3d mins - minimum extents of bbox
// vec3d maxs - maximum extents of bbox
// vec3d start - point in bbox reference frame
// vec3d box_pt - point in bbox reference frame.
// NOTE: if a coordinate of start is *inside* the bbox, it is *not* moved to surface of bbox
// return: 1 if inside, 0 otherwise.
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
