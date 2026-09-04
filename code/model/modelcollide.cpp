/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#define MODEL_LIB

#include "cmdline/cmdline.h"
#include "graphics/tmapper.h"
#include "math/fvi.h"
#include "math/vecmat.h"
#include "model/model.h"
#include "model/modelrender.h"
#include "model/modelsinc.h"
#include "render/3d.h"
#include "tracing/Monitor.h"
#include "tracing/tracing.h"

#define TOL		1E-4
#define DIST_TOL	1.0

// Some global variables that get set by model_collide and are used internally for
// checking a collision rather than passing a bunch of parameters around. These are
// not persistant between calls to model_collide

thread_local static mc_info		*Mc;				// The mc_info passed into model_collide

thread_local static polymodel	*Mc_pm;			// The polygon model we're checking
thread_local static int			Mc_submodel;	// The current submodel we're checking

thread_local static polymodel_instance *Mc_pmi;

thread_local static matrix		Mc_orient;		// A matrix to rotate a world point into the current
											// submodel's frame of reference.
thread_local static vec3d		Mc_base;			// A point used along with Mc_orient.

thread_local static vec3d		Mc_p0;			// The ray origin rotated into the current submodel's frame of reference
thread_local static vec3d		Mc_p1;			// The ray end rotated into the current submodel's frame of reference
thread_local static float		Mc_mag;			// The length of the ray
thread_local static vec3d		Mc_direction;	// A vector from the ray's origin to its end, in the current submodel's frame of reference

thread_local static vec3d 		**Mc_point_list = nullptr;		// A pointer to the current submodel's vertex list

// mc_check_triangle_face() vs mc_check_triangle_sphereline_face(): resolved once per
// model_collide() call, not per triangle visited -- Mc->flags is invariant for the whole call, so
// re-testing `Mc->flags & MC_CHECK_SPHERELINE` on every single candidate triangle in
// mc_check_bvh_triangle()'s nearest-first confirmation loop (which can revisit several triangles
// per leaf) would be pure repeated branching on an unchanging answer.
using TriangleFaceTestFn = void (*)(const vec3d &, const vec3d &, const vec3d &, const bvh_uv &, const bvh_uv &,
	const bvh_uv &, bool, int);
thread_local static TriangleFaceTestFn Mc_triangle_face_test_fn = nullptr;



void model_collide_free_point_list()
{
	if (Mc_point_list != NULL) {
		vm_free(Mc_point_list);
		Mc_point_list = NULL;
	}
}

// allocate the point list
// NOTE: SHOULD ONLY EVER BE CALLED FROM model_allocate_interp_data()!!!
void model_collide_allocate_point_list(int n_points)
{
	Assert( n_points > 0 );

	if (Mc_point_list != NULL) {
		vm_free(Mc_point_list);
		Mc_point_list = NULL;
	}

	Mc_point_list = (vec3d**) vm_malloc( sizeof(vec3d *) * n_points );

	Verify( Mc_point_list != NULL );
}

// Returns non-zero if vector from p0 to pdir 
// intersects the bounding box.
// hitpos could be NULL, so don't fill it if it is.
int mc_ray_boundingbox( vec3d *min, vec3d *max, vec3d * p0, vec3d *pdir, vec3d *hitpos )
{

	vec3d tmp_hitpos;
	if ( hitpos == NULL )	{
		hitpos = &tmp_hitpos;
	}


	if ( Mc->flags & MC_CHECK_SPHERELINE )	{

		// In the case of a sphere, just increase the size of the box by the radius 
		// of the sphere in all directions.

		vec3d sphere_mod_min, sphere_mod_max;

		sphere_mod_min.xyz.x = min->xyz.x - Mc->radius;
		sphere_mod_max.xyz.x = max->xyz.x + Mc->radius;
		sphere_mod_min.xyz.y = min->xyz.y - Mc->radius;
		sphere_mod_max.xyz.y = max->xyz.y + Mc->radius;
		sphere_mod_min.xyz.z = min->xyz.z - Mc->radius;
		sphere_mod_max.xyz.z = max->xyz.z + Mc->radius;

		return fvi_ray_boundingbox( &sphere_mod_min, &sphere_mod_max, p0, pdir, hitpos );
	} else {
		return fvi_ray_boundingbox( min, max, p0, pdir, hitpos );
	}	
}



// Computes a polygon's normal directly from its own vertices (Newell's method -- a single O(n)
// pass, robust for near-planar input and not limited to triangles), rather than trusting a
// stored "plane_norm" field. The POF chunk's own plane-normal field is parsed verbatim from the
// .pof file (exporter output, never validated against the vertices it's attached to) and can
// diverge from the true geometric normal by a large margin on real content, which flips
// backface-cull decisions and solves the ray against a tilted, wrong plane.
//
// Falls back to the caller-supplied stored normal only when the computed one is degenerate
// (near-zero magnitude -- a truly zero-area or collinear polygon, where there's nothing
// meaningful to compute from the vertices).
static vec3d mc_compute_geometric_normal(int nv, vec3d **verts, const vec3d *stored_norm)
{
	vec3d n = vmd_zero_vector;
	for (int i = 0; i < nv; ++i) {
		const vec3d *a = verts[i];
		const vec3d *b = verts[(i + 1) % nv];
		n.xyz.x += (a->xyz.y - b->xyz.y) * (a->xyz.z + b->xyz.z);
		n.xyz.y += (a->xyz.z - b->xyz.z) * (a->xyz.x + b->xyz.x);
		n.xyz.z += (a->xyz.x - b->xyz.x) * (a->xyz.y + b->xyz.y);
	}

	if (vm_vec_normalize_safe(&n, true) <= 0.0f) {
		return *stored_norm;
	}
	return n;
}

// ----------------------------------------------------------------------------------------------------------
// check face with spheres
//
//	inputs:	nv				=>		number of vertices
//				verts			=>		array of vertices
//				plane_pnt	=>		center point in plane (about which radius is measured)
//				face_rad		=>		radius of face 
//				plane_norm	=>		normal of face
static void mc_check_sphereline_face( int nv, vec3d ** verts, vec3d * plane_pnt, vec3d * plane_norm, uv_pair * uvl_list, int ntmap)
{
	vec3d	hit_point;
	float		u, v;
	float		delta_t;			// time sphere takes to cross from one side of plane to the other
	float		face_t;			// time at which face touches plane
									// NOTE all times are normalized so that t = 1.0 at the end of the frame
	int		check_face = 1;		// assume we'll check the face.
	int		check_edges = 1;		// assume we'll check the edges.

	// Use the polygon's own geometry, not the (possibly badly wrong) authored plane_norm --
	// see mc_compute_geometric_normal() above. Shadowing the parameter means every use below
	// picks up the fix.
	vec3d geo_norm = mc_compute_geometric_normal(nv, verts, plane_norm);
	plane_norm = &geo_norm;

	// Check to see if poly is facing away from ray.  If so, don't bother
	// checking it.

	if (!(Mc->flags & MC_COLLIDE_ALL) && vm_vec_dot(&Mc_direction,plane_norm) > 0.0f)	{
		return;
	}

	// Find the intersection of this sphere with the plane of the poly
	if ( !fvi_sphere_plane( &hit_point, &Mc_p0, &Mc_direction, Mc->radius, plane_norm, plane_pnt, &face_t, &delta_t ) ) {
		return;
	}

	// If the ray is behind the plane there is no collision
	if (face_t > 1.0f) {
		check_face = 0;
		check_edges = 0;
	} else if (face_t < 0.0f) {
		check_face = 0;

		// check whether sphere can hit edge in allowed time range
		if ( (face_t + delta_t) < 0.0f)
			check_edges = 0;
	}

	// If the ray hits, but a closer intersection has already been found, don't check face
	if (!(Mc->flags & MC_COLLIDE_ALL) && Mc->num_hits && (face_t >= Mc->hit_dist ) ) {
		check_face = 0;		// The ray isn't long enough to intersect the plane
	}


	//vec3d temp_sphere;
	//vec3d temp_dir;
	//float temp_dist;
	// DA 11/5/97  Above is used to test distance between hit_point and sphere_hit_point.
	// This can be as large as 0.003 on a unit sphere.  I suspect that with larger spheres,
	// both the relative and absolute error decrease, but this should still be checked for the
	// case of larger spheres (about 5-10 units).  The error also depends on the geometry of the 
	// object we're colliding against, but I think to a lesser degree.

	if ( check_face )	{
		// Find the time of the sphere surface touches the plane
		// If this is within the collision window, check to see if we hit a face
		if ( fvi_point_face(&hit_point, nv, verts, plane_norm, &u, &v, uvl_list) ) {

			Mc->hit_dist = face_t;
			Mc->hit_point = hit_point;
			Mc->hit_normal = *plane_norm;
			Mc->hit_submodel = Mc_submodel;			
			Mc->edge_hit = false;

			if (Mc->flags & MC_COLLIDE_ALL) {
				Mc->hit_points_all.push_back(hit_point);
				Mc->hit_submodels_all.push_back(Mc_submodel);
			}

			if ( uvl_list )	{
				Mc->hit_u = u;
				Mc->hit_v = v;
				if ( ntmap < 0 ) {
					Mc->hit_bitmap = -1;
				} else {
					Mc->hit_bitmap = Mc_pm->maps[ntmap].textures[TM_BASE_TYPE].GetTexture();
				}
			}

			Mc->hit_tmap_num = ntmap;

			Mc->num_hits++;
			check_edges = 0;
			/*
			vm_vec_scale_add( &temp_sphere, &Mc_p0, &Mc_direction, Mc->hit_dist );
			temp_dist = vm_vec_dist( &temp_sphere, &hit_point );
			if ( (temp_dist - DIST_TOL > Mc->radius) || (temp_dist + DIST_TOL < Mc->radius) ) {
				// get Andsager
				//mprintf(("Estimated radius error: Estimate %f, actual %f Mc->radius\n", temp_dist, Mc->radius));
			}
			vm_vec_sub( &temp_dir, &hit_point, &temp_sphere );
			// Assert( vm_vec_dot( &temp_dir, &Mc_direction ) > 0 );
			*/
		}
	}


	if ( check_edges ) {
		// Either (face_t) is out of range or we miss the face
		// Check for sphere hitting edge

		// If checking shields, we *still* need to check edges

		// this is where we need another test to cull checking for edges
		// PUT TEST HERE

		// check each edge to see if we hit, find the closest edge
		// Mc->hit_dist stores the best edge time of *all* faces
		float sphere_time;
		if ( fvi_polyedge_sphereline(&hit_point, &Mc_p0, &Mc_direction, Mc->radius, nv, verts, &sphere_time)) {
			Assert( sphere_time >= 0.0f );
			/*
			vm_vec_scale_add( &temp_sphere, &Mc_p0, &Mc_direction, sphere_time );
			temp_dist = vm_vec_dist( &temp_sphere, &hit_point );
			if ( (temp_dist - DIST_TOL > Mc->radius) || (temp_dist + DIST_TOL < Mc->radius) ) {
				// get Andsager
				//mprintf(("Estimated radius error: Estimate %f, actual %f Mc->radius\n", temp_dist, Mc->radius));
			}
			vm_vec_sub( &temp_dir, &hit_point, &temp_sphere );
//			Assert( vm_vec_dot( &temp_dir, &Mc_direction ) > 0 );
			*/

			if ((Mc->flags & MC_COLLIDE_ALL) || (Mc->num_hits==0) || (sphere_time < Mc->hit_dist) ) {
				// This is closer than best so far
				Mc->hit_dist = sphere_time;
				Mc->hit_point = hit_point;
				Mc->hit_normal = *plane_norm;
				Mc->hit_submodel = Mc_submodel;
				Mc->edge_hit = true;

				if (Mc->flags & MC_COLLIDE_ALL) {
					Mc->hit_points_all.push_back(hit_point);
					Mc->hit_submodels_all.push_back(Mc_submodel);
				}

				if ( ntmap < 0 ) {
					Mc->hit_bitmap = -1;
				} else {
					Mc->hit_bitmap = Mc_pm->maps[ntmap].textures[TM_BASE_TYPE].GetTexture();
				}

				Mc->num_hits++;

			//	nprintf(("Physics", "edge sphere time: %f, normal: (%f, %f, %f) hit_point: (%f, %f, %f)\n", sphere_time,
			//		Mc->hit_normal.xyz.x, Mc->hit_normal.xyz.y, Mc->hit_normal.xyz.z,
			//		hit_point.xyz.x, hit_point.xyz.y, hit_point.xyz.z));
			} else  {	// Not best so far
				Assert(Mc->num_hits>0);
				Mc->num_hits++;
			}
		}
	}
}

// Triangle-granularity face test: takes a single triangle's own 3 verts/UVs instead of an n-gon's
// full vertex list. Uses the triangle's own exact plane (cross product of its two edges -- always
// planar for 3 points, no Newell averaging needed, unlike mc_compute_geometric_normal()'s n-gon
// case) for backface cull and the ray/plane solve, and a direct barycentric containment test
// instead of fvi_point_face's
// dominant-axis-projection point-in-polygon test (equivalent result, no need for that generality
// with a fixed 3-vertex input). `has_uv`/`ntmap` mirror the n-gon face test's own uvl_list/ntmap
// convention -- has_uv==false for a flat (untextured) polygon.
static void mc_check_triangle_face(const vec3d &v0, const vec3d &v1, const vec3d &v2, const bvh_uv &uv0,
	const bvh_uv &uv1, const bvh_uv &uv2, bool has_uv, int ntmap)
{
	vec3d e1 = v1 - v0;
	vec3d e2 = v2 - v0;

	vec3d normal;
	vm_vec_cross(&normal, &e1, &e2);
	if (vm_vec_normalize_safe(&normal, true) <= 0.0f)
		return; // degenerate (zero-area) triangle -- nothing to test against

	if (!(Mc->flags & MC_COLLIDE_ALL) && vm_vec_dot(&Mc_direction, &normal) > 0.0f)
		return;

	float dist = fvi_ray_plane(nullptr, &v0, &normal, &Mc_p0, &Mc_direction, 0.0f);

	if (dist < 0.0f)
		return; // If the ray is behind the plane there is no collision
	if (!(Mc->flags & MC_CHECK_RAY) && (dist > 1.0f))
		return; // The ray isn't long enough to intersect the plane
	if (!(Mc->flags & MC_COLLIDE_ALL) && Mc->num_hits && (dist >= Mc->hit_dist))
		return; // A closer intersection has already been found

	vec3d hit_point;
	vm_vec_scale_add(&hit_point, &Mc_p0, &Mc_direction, dist);

	// Barycentric containment test against the plane hit point already in hand.
	vec3d vp0 = hit_point - v0;
	float d00 = vm_vec_dot(&e1, &e1);
	float d01 = vm_vec_dot(&e1, &e2);
	float d11 = vm_vec_dot(&e2, &e2);
	float d20 = vm_vec_dot(&vp0, &e1);
	float d21 = vm_vec_dot(&vp0, &e2);
	float denom = d00 * d11 - d01 * d01;
	if (fabsf(denom) < 1e-12f)
		return; // degenerate

	float gamma_v1 = (d11 * d20 - d01 * d21) / denom; // barycentric weight for v1
	float gamma_v2 = (d00 * d21 - d01 * d20) / denom; // barycentric weight for v2
	float gamma_v0 = 1.0f - gamma_v1 - gamma_v2;      // barycentric weight for v0

	constexpr float BARY_EPS = 1e-4f;
	if (gamma_v0 < -BARY_EPS || gamma_v1 < -BARY_EPS || gamma_v2 < -BARY_EPS)
		return; // outside the triangle

	Mc->hit_dist = dist;
	Mc->hit_point = hit_point;
	Mc->hit_submodel = Mc_submodel;
	Mc->hit_normal = normal;

	if (Mc->flags & MC_COLLIDE_ALL) {
		Mc->hit_points_all.push_back(hit_point);
		Mc->hit_submodels_all.push_back(Mc_submodel);
	}

	if (has_uv) {
		Mc->hit_u = gamma_v0 * uv0.u + gamma_v1 * uv1.u + gamma_v2 * uv2.u;
		Mc->hit_v = gamma_v0 * uv0.v + gamma_v1 * uv1.v + gamma_v2 * uv2.v;
		Mc->hit_bitmap = (ntmap < 0) ? -1 : Mc_pm->maps[ntmap].textures[TM_BASE_TYPE].GetTexture();
	}

	Mc->hit_tmap_num = ntmap;

	Mc->num_hits++;
}

// Triangle-granularity sibling of mc_check_sphereline_face() above. Mirrors its structure (face
// test via the sphere-vs-plane touch time, then an edge fallback) exactly, but against the
// triangle's own exact plane/edges instead of an n-gon's. The edge fallback deliberately calls the
// existing, unmodified fvi_polyedge_sphereline() with nv=3 on the triangle's own 3 edges,
// unconditionally -- including whichever edge may be a fan-triangulation diagonal rather than a
// true polygon boundary. Also mirrors mc_check_sphereline_face(): the edge-hit branch does not set
// Mc->hit_tmap_num (only the face-hit branch does).
static void mc_check_triangle_sphereline_face(const vec3d &v0, const vec3d &v1, const vec3d &v2, const bvh_uv &uv0,
	const bvh_uv &uv1, const bvh_uv &uv2, bool has_uv, int ntmap)
{
	vec3d e1 = v1 - v0;
	vec3d e2 = v2 - v0;

	vec3d normal;
	vm_vec_cross(&normal, &e1, &e2);
	if (vm_vec_normalize_safe(&normal, true) <= 0.0f)
		return;

	if (!(Mc->flags & MC_COLLIDE_ALL) && vm_vec_dot(&Mc_direction, &normal) > 0.0f)
		return;

	vec3d hit_point;
	float face_t, delta_t;
	if (!fvi_sphere_plane(&hit_point, &Mc_p0, &Mc_direction, Mc->radius, &normal, &v0, &face_t, &delta_t))
		return;

	int check_face = 1;
	int check_edges = 1;

	if (face_t > 1.0f) {
		check_face = 0;
		check_edges = 0;
	} else if (face_t < 0.0f) {
		check_face = 0;
		if ((face_t + delta_t) < 0.0f)
			check_edges = 0;
	}

	if (!(Mc->flags & MC_COLLIDE_ALL) && Mc->num_hits && (face_t >= Mc->hit_dist)) {
		check_face = 0;
	}

	if (check_face) {
		vec3d vp0 = hit_point - v0;
		float d00 = vm_vec_dot(&e1, &e1);
		float d01 = vm_vec_dot(&e1, &e2);
		float d11 = vm_vec_dot(&e2, &e2);
		float d20 = vm_vec_dot(&vp0, &e1);
		float d21 = vm_vec_dot(&vp0, &e2);
		float denom = d00 * d11 - d01 * d01;

		bool on_face = false;
		float gamma_v0 = 0.0f, gamma_v1 = 0.0f, gamma_v2 = 0.0f;
		if (fabsf(denom) >= 1e-12f) {
			gamma_v1 = (d11 * d20 - d01 * d21) / denom;
			gamma_v2 = (d00 * d21 - d01 * d20) / denom;
			gamma_v0 = 1.0f - gamma_v1 - gamma_v2;
			constexpr float BARY_EPS = 1e-4f;
			on_face = gamma_v0 >= -BARY_EPS && gamma_v1 >= -BARY_EPS && gamma_v2 >= -BARY_EPS;
		}

		if (on_face) {
			Mc->hit_dist = face_t;
			Mc->hit_point = hit_point;
			Mc->hit_normal = normal;
			Mc->hit_submodel = Mc_submodel;
			Mc->edge_hit = false;

			if (Mc->flags & MC_COLLIDE_ALL) {
				Mc->hit_points_all.push_back(hit_point);
				Mc->hit_submodels_all.push_back(Mc_submodel);
			}

			if (has_uv) {
				Mc->hit_u = gamma_v0 * uv0.u + gamma_v1 * uv1.u + gamma_v2 * uv2.u;
				Mc->hit_v = gamma_v0 * uv0.v + gamma_v1 * uv1.v + gamma_v2 * uv2.v;
				Mc->hit_bitmap = (ntmap < 0) ? -1 : Mc_pm->maps[ntmap].textures[TM_BASE_TYPE].GetTexture();
			}

			Mc->hit_tmap_num = ntmap;

			Mc->num_hits++;
			check_edges = 0;
		}
	}

	if (check_edges) {
		const vec3d *verts[3] = {&v0, &v1, &v2};
		float sphere_time;
		if (fvi_polyedge_sphereline(&hit_point, &Mc_p0, &Mc_direction, Mc->radius, 3, verts, &sphere_time)) {
			Assert(sphere_time >= 0.0f);

			if ((Mc->flags & MC_COLLIDE_ALL) || (Mc->num_hits == 0) || (sphere_time < Mc->hit_dist)) {
				Mc->hit_dist = sphere_time;
				Mc->hit_point = hit_point;
				Mc->hit_normal = normal;
				Mc->hit_submodel = Mc_submodel;
				Mc->edge_hit = true;

				if (Mc->flags & MC_COLLIDE_ALL) {
					Mc->hit_points_all.push_back(hit_point);
					Mc->hit_submodels_all.push_back(Mc_submodel);
				}

				Mc->hit_bitmap = (ntmap < 0) ? -1 : Mc_pm->maps[ntmap].textures[TM_BASE_TYPE].GetTexture();

				Mc->num_hits++;
			} else {
				Assert(Mc->num_hits > 0);
				Mc->num_hits++;
			}
		}
	}
}

int model_collide_parse_bsp_defpoints(ubyte * p)
{
	uint n;
	uint nverts = uw(p+8);	
	uint offset = uw(p+16);	

	ubyte * normcount = p+20;
	vec3d *src = vp(p+offset);

	model_collide_allocate_point_list(nverts);

	Assert( Mc_point_list != NULL );

	for (n=0; n<nverts; n++ ) {
		Mc_point_list[n] = src;

		src += normcount[n]+1;
	} 

	return nverts;
}


// Runs the face test for triangle tri_index via Mc_triangle_face_test_fn (resolved once per
// model_collide() call). Does NOT re-check Mc->flags & MC_CHECK_SPHERELINE itself.
// check_invisible_faces/collide_invisible gate the invisible-texture skip; both are invariant for
// the whole model_collide_bvh_triangle() call, so the caller hoists them out instead of re-reading
// Mc/Mc_pm per triangle. tmap_num comes straight from the triangle's own bvh_tree storage, set
// directly by model_collide_parse_bsp() at build time.
static bool mc_check_bvh_triangle_candidate(const bvh_tree *tbvh, int32_t tri_index, bool check_invisible_faces,
	bool collide_invisible)
{
	size_t idx = static_cast<size_t>(tri_index);
	int raw_tmap_num = tbvh->tmap_num[idx];

	if (raw_tmap_num < MAX_MODEL_TEXTURES && !check_invisible_faces &&
		Mc_pm->maps[raw_tmap_num].textures[TM_BASE_TYPE].GetTexture() < 0 && !collide_invisible) {
		return false;
	}

	bool flat_poly = raw_tmap_num >= MAX_MODEL_TEXTURES;
	int ntmap = flat_poly ? -1 : raw_tmap_num;

	vec3d v0 = tbvh->vertex(tbvh->i0[idx]);
	vec3d v1 = tbvh->vertex(tbvh->i1[idx]);
	vec3d v2 = tbvh->vertex(tbvh->i2[idx]);

	Mc_triangle_face_test_fn(v0, v1, v2, tbvh->uv0[idx], tbvh->uv1[idx], tbvh->uv2[idx], !flat_poly, ntmap);
	return true;
}

// Tests one leaf range [start, start+count) of a bvh_tree (as handed to a bvh_visit_triangles()
// visitor). The plain ray, nearest-hit case (the common one -- beam weapons, HUD targeting, the
// Lua API) gets a fast path: a SIMD-batched geometry pass (ray_triangle_leaf_simd()) finds the
// nearest geometrically-valid triangle in one pass; if it's texture-visible, that's the answer. If
// not (rare), or if this is a sphere-line query (no SIMD batch geometry exists for sphere-vs-
// triangle) or MC_COLLIDE_ALL (needs every geometrically-valid triangle visited, not just the
// nearest), falls back to a plain scalar scan of this one (already spatially-pruned, small) leaf.
// t_max is the traversal's current segment/nearest-hit bound (see bvh_visit_triangles()) -- for the
// ray fast path below it's used both to seed the SIMD search and, on a newly-registered hit, is
// tightened so later leaves the traversal visits get pruned to only what could still be closer. Not
// touched for sphereline/MC_COLLIDE_ALL queries below -- MC_COLLIDE_ALL needs every hit, and
// sphereline has no SIMD fast path to seed.
static void mc_check_bvh_triangle(int32_t start, int32_t count, const bvh_tree *tbvh, float &t_max)
{
	bool sphereline = (Mc->flags & MC_CHECK_SPHERELINE) != 0;
	bool collide_all = (Mc->flags & MC_COLLIDE_ALL) != 0;
	// Both invariant for this whole submodel query (Mc->flags, and the submodel-level flag Mc_pm
	// already resolved via Mc_submodel) -- hoisted out of the per-candidate hot loop below instead
	// of re-read from Mc/Mc_pm on every single triangle.
	bool check_invisible_faces = (Mc->flags & MC_CHECK_INVISIBLE_FACES) != 0;
	bool collide_invisible = Mc_pm->submodel[Mc_submodel].flags[Model::Submodel_flags::Collide_invisible];

	if (!sphereline && !collide_all) {
		float best_t = t_max;
		float simd_t;
		int32_t simd_idx;
		bool simd_hit = ray_triangle_leaf_simd(*tbvh, start, count, Mc_p0, Mc_direction, best_t, simd_t, simd_idx);

		if (!simd_hit) {
			return;
		}
		int prior_num_hits = Mc->num_hits;
		if (mc_check_bvh_triangle_candidate(tbvh, simd_idx, check_invisible_faces, collide_invisible)) {
			if (Mc->num_hits != prior_num_hits) {
				// The scalar confirm actually registered a hit -- the common case, done. Tighten
				// the traversal's bound so later leaves are pruned to only what could be closer.
				t_max = std::min(t_max, Mc->hit_dist);
				return;
			}
			// ray_triangle_leaf_simd() only knows the *nearest-by-t, texture-visible* candidate
			// -- it doesn't backface-cull and doesn't know the segment-length bound
			// (Mc->flags & MC_CHECK_RAY ? unbounded : dist <= 1.0) that
			// mc_check_triangle_face()/mc_check_triangle_sphereline_face() enforce. So a
			// geometrically "valid" SIMD candidate can still be rejected by the scalar confirm
			// (backface, or beyond the segment) with no hit recorded. Don't let that silently
			// shadow a farther-but-genuinely-acceptable triangle in the same leaf -- fall
			// through to the exhaustive scalar scan below instead of returning empty-handed.
		}

		// Reached when the nearest SIMD candidate was texture-invisible, or was geometrically
		// picked but rejected by the scalar confirm for a reason the SIMD pass can't see
		// (backface, out-of-segment). Exhaustive scan of this small, already spatially-pruned
		// leaf for the true nearest acceptable hit -- the rare path, not the common one.
		for (int32_t t = start; t < start + count; ++t) {
			mc_check_bvh_triangle_candidate(tbvh, t, check_invisible_faces, collide_invisible);
		}
		if (Mc->num_hits != prior_num_hits) {
			t_max = std::min(t_max, Mc->hit_dist);
		}
		return;
	}

	// Sphereline is nearest-hit too (only MC_COLLIDE_ALL needs every hit) -- there's no SIMD fast
	// path to seed for it, but the traversal-level prune below still helps by skipping AABB
	// subtrees beyond whatever's already been found.
	int prior_num_hits = Mc->num_hits;
	for (int32_t t = start; t < start + count; ++t) {
		mc_check_bvh_triangle_candidate(tbvh, t, check_invisible_faces, collide_invisible);
	}
	if (!collide_all && Mc->num_hits != prior_num_hits) {
		t_max = std::min(t_max, Mc->hit_dist);
	}
}

// The submodel-collision traversal: walks tbvh, running mc_check_bvh_triangle() on every leaf the
// ray/sphere-line reaches. Every triangle visited goes through the same face/sphereline logic
// (mc_check_triangle_face()/mc_check_triangle_sphereline_face() via mc_check_bvh_triangle() above),
// so MC_COLLIDE_ALL accumulation, sphere-line edge fallback, backface culling, and all mc_info
// output fields behave consistently regardless of which submodel is being tested.
// bvh_visit_triangles()'s ray parametrization reaches exactly Mc_p1 at t=1 (since Mc_direction =
// Mc_p1 - Mc_p0, unnormalized), matching mc_check_triangle_face()'s own `dist > 1.0f` bound -- not
// Mc_mag, which is the equivalent bound in a different (absolute-distance) unit that only happens
// to agree with the t=1 check because Mc_direction's local-space magnitude equals Mc_mag by
// rigid-transform distance preservation (see the Mc_mag comment near the top of this file).
static void model_collide_bvh_triangle(const bvh_tree *tbvh)
{
	if ( tbvh == nullptr || tbvh->nodes.empty() ) {
		return;
	}

	float t_max = (Mc->flags & MC_CHECK_RAY) ? FLT_MAX : 1.0f;
	// If a prior submodel in this same model_collide() call already found a hit, don't search this
	// submodel's tree any farther than that.
	if (!(Mc->flags & MC_COLLIDE_ALL) && Mc->num_hits) {
		t_max = std::min(t_max, Mc->hit_dist);
	}
	// A sphere-line query's AABB test must be inflated by Mc->radius, matching
	// mc_ray_boundingbox()'s handling elsewhere -- without this, this traversal would silently miss
	// any leaf a sphere's swept volume would reach but the bare centerline ray-segment doesn't pass
	// through.
	float radius = (Mc->flags & MC_CHECK_SPHERELINE) ? Mc->radius : 0.0f;
	// t_max is also this traversal's running nearest-hit prune bound -- see mc_check_bvh_triangle()
	// and bvh_visit_triangles()'s own doc comments. The visitor tightens it in place as hits are
	// found, so later leaves/nodes get pruned to only what could still be closer.
	bvh_visit_triangles(*tbvh, Mc_p0, Mc_direction, t_max, radius,
		[tbvh](int32_t start, int32_t count, float &leaf_t_max) {
			mc_check_bvh_triangle(start, count, tbvh, leaf_t_max);
		});
}

// Decodes one polygon chunk's vertices into verts_out (capacity TMAP_MAX_VERTS), writing its
// tmap_num into *out_tmap_num. Returns the vertex count, or 0 if the chunk is malformed (too many
// vertices) -- the caller then skips emitting any geometry for it.
static int model_collide_parse_bsp_tmappoly(model_tmap_vert *verts_out, int *out_tmap_num, void *model_ptr)
{
	ubyte *p = (ubyte *)model_ptr;

	uint nv = uw(p + TMAP_NVERTS);

	if (nv > TMAP_MAX_VERTS) {
		Error(LOCATION, "Model contains TMAP chunk with more than %d vertices!", TMAP_MAX_VERTS);
		return 0;
	}

	int tmap_num = w(p + TMAP_TEXNUM);

	if (tmap_num < 0 || tmap_num >= MAX_MODEL_TEXTURES) {
		Error(LOCATION, "Model contains TMAP2 chunk with invalid texture id (%d)!", tmap_num);
		return 0;
	}

	auto verts = reinterpret_cast<model_tmap_vert_old*>(&p[TMAP_VERTS]);

	*out_tmap_num = tmap_num;
	for (uint i = 0; i < nv; ++i) {
		verts_out[i] = model_tmap_vert(verts[i]);
	}
	return (int)nv;
}

// See model_collide_parse_bsp_tmappoly() above.
static int model_collide_parse_bsp_tmap2poly(model_tmap_vert *verts_out, int *out_tmap_num, void *model_ptr)
{
	auto p = (ubyte*)model_ptr;

	uint nv = uw(p + TMAP2_NVERTS);

	if (nv > TMAP_MAX_VERTS) {
		Error(LOCATION,"Model contains TMAP2 chunk with more than %d vertices!", TMAP_MAX_VERTS);
		return 0;
	}

	int tmap_num = w(p + TMAP2_TEXNUM);

	if (tmap_num < 0 || tmap_num >= MAX_MODEL_TEXTURES) {
		Error(LOCATION, "Model contains TMAP2 chunk with invalid texture id (%d)!", tmap_num);
		return 0;
	}

	auto verts = reinterpret_cast<model_tmap_vert*>(p + TMAP2_VERTS);

	*out_tmap_num = tmap_num;
	for (uint i = 0; i < nv; ++i) {
		verts_out[i] = verts[i];
	}
	return (int)nv;
}

// See model_collide_parse_bsp_tmappoly() above. Flat polys have no texture -- tmap_num is set to
// 255, the sentinel mc_check_bvh_triangle_candidate()'s `raw_tmap_num >= MAX_MODEL_TEXTURES` check
// already treats as "flat" (the same encoding this function has always used).
static int model_collide_parse_bsp_flatpoly(model_tmap_vert *verts_out, int *out_tmap_num, void *model_ptr)
{
	ubyte *p = (ubyte *)model_ptr;

	uint nv = uw(p+36);

	if ( nv > TMAP_MAX_VERTS ) {
		Int3();
		return 0;
	}

	short *verts = (short *)(p+44);

	*out_tmap_num = 255;
	for (uint i = 0; i < nv; ++i) {
		verts_out[i].vertnum = verts[i*2];
		verts_out[i].normnum = 0;
		verts_out[i].u = 0.0f;
		verts_out[i].v = 0.0f;
	}
	return (int)nv;
}

// Computes the polygon's center (average of its own verts) and pushes it onto tree->poly_centers --
// matches how ai_bpap() (code/ai/aibig.cpp) reads this, once per decoded polygon regardless of
// whether it degenerates into zero triangles below (nv < 3 happens on malformed content; nv <= 0 only
// from a decode failure above, in which case there's nothing to average, so it's skipped entirely).
//
// If nv >= 3, also fan-triangulates (pivot = first vertex) directly into out_triangles. poly_index is
// a running per-submodel counter, incremented once per polygon that actually produced triangles;
// both original_index and leaf_index are pure build-time provenance tags, never read by the live
// collision path (see modelbvh.h).
static void model_collide_emit_polygon(const model_tmap_vert *verts, int nv, int tmap_num,
	int &poly_index, bsp_collision_tree *tree, SCP_vector<bvh_triangle> &out_triangles)
{
	if (nv <= 0) {
		return;
	}

	vec3d center = vmd_zero_vector;
	for (int j = 0; j < nv; ++j) {
		center += *Mc_point_list[verts[j].vertnum];
	}
	tree->poly_centers.push_back(center / (float)nv);

	if (nv < 3) {
		return;
	}

	const vec3d& v0 = *Mc_point_list[verts[0].vertnum];
	bvh_uv uv0{verts[0].u, verts[0].v};

	for (int i = 1; i < nv - 1; ++i) {
		const vec3d& vi = *Mc_point_list[verts[i].vertnum];
		const vec3d& vi1 = *Mc_point_list[verts[i + 1].vertnum];

		bvh_triangle tri;
		tri.v0 = v0;
		tri.v1 = vi;
		tri.v2 = vi1;
		tri.tmap_num = tmap_num;
		tri.original_index = poly_index;
		tri.leaf_index = poly_index;
		tri.uv0 = uv0;
		tri.uv1 = {verts[i].u, verts[i].v};
		tri.uv2 = {verts[i + 1].u, verts[i + 1].v};
		out_triangles.push_back(tri);
	}

	++poly_index;
}

// Walks the POF BSP opcode stream exactly once, emitting one fan-triangulated bvh_triangle per
// polygon directly into out_triangles (see model_collide_emit_polygon() above). Only tracks enough
// state to reach every polygon chunk (front/back descent through OP_SORTNORM/OP_SORTNORM2), not to
// describe the tree's node topology (min/max/front/back), since nothing needs that beyond reaching
// the polygons.
void model_collide_parse_bsp(bsp_collision_tree *tree, ubyte *bsp_data, int version, SCP_vector<bvh_triangle> &out_triangles)
{
	TRACE_SCOPE(tracing::ModelParseBSPTree);

	ubyte *p = bsp_data;
	ubyte *next_p;

	int chunk_type = w(p);
	int chunk_size = w(p+4);

	int next_chunk_type;
	int next_chunk_size;

	Assert(chunk_type == OP_DEFPOINTS);

	int n_verts = model_collide_parse_bsp_defpoints(p);

	if ( n_verts <= 0) {
		tree->point_list = NULL;
		tree->n_verts = 0;
		return;
	}

	p += chunk_size;

	// Chunk pointers left to visit, walked breadth-first (a growing vector indexed by i below);
	// traversal order doesn't matter to out_triangles, only that every polygon chunk gets reached.
	SCP_vector<ubyte*> worklist;
	worklist.push_back(p);

	int poly_index = 0;
	model_tmap_vert poly_verts[TMAP_MAX_VERTS];

	size_t i = 0;

	while ( i < worklist.size() ) {
		p = worklist[i];

		chunk_type = w(p);
		chunk_size = w(p+4);
		int front_offset, back_offset;
		switch ( chunk_type ) {
		case OP_SORTNORM:
			front_offset = w(p + 36);
			if (front_offset) {
				next_chunk_type = w(p + front_offset);

				if ( next_chunk_type != OP_EOF ) {
					worklist.push_back(p + front_offset);
				}
			}

			back_offset = w(p + 40);
			if (back_offset) {
				next_chunk_type = w(p + back_offset);

				if ( next_chunk_type != OP_EOF ) {
					worklist.push_back(p + back_offset);
				}
			}

			next_p = p + chunk_size;
			next_chunk_type = w(next_p);

			Assert( next_chunk_type == OP_EOF );

			++i;
			break;
		case OP_SORTNORM2:
			front_offset = w(p + 8);
			if (front_offset) {
				next_chunk_type = w(p + front_offset);

				if (next_chunk_type != OP_EOF) {
					worklist.push_back(p + front_offset);
				}
			}

			back_offset = w(p + 12);
			if (back_offset) {
				next_chunk_type = w(p + back_offset);

				if (next_chunk_type != OP_EOF) {
					worklist.push_back(p + back_offset);
				}
			}

			++i;
			break;
		case OP_BOUNDBOX:
			next_p = p + chunk_size;
			next_chunk_type = w(next_p);
			next_chunk_size = w(next_p+4);

			if (next_chunk_type != OP_EOF &&
				(next_chunk_type == OP_TMAPPOLY || next_chunk_type == OP_FLATPOLY)) {

				while ( next_chunk_type != OP_EOF ) {
					int nv, tmap_num;
					if ( next_chunk_type == OP_TMAPPOLY ) {
						nv = model_collide_parse_bsp_tmappoly(poly_verts, &tmap_num, next_p);
					} else if ( next_chunk_type == OP_FLATPOLY ) {
						nv = model_collide_parse_bsp_flatpoly(poly_verts, &tmap_num, next_p);
					} else {
						Int3();
						nv = 0;
						tmap_num = 255;
					}

					model_collide_emit_polygon(poly_verts, nv, tmap_num, poly_index, tree, out_triangles);

					next_p += next_chunk_size;
					next_chunk_type = w(next_p);
					next_chunk_size = w(next_p+4);
				}
			}

			Assert(next_chunk_type == OP_EOF);

			++i;
			break;
		case OP_TMAP2POLY:
			{
				int nv, tmap_num;
				nv = model_collide_parse_bsp_tmap2poly(poly_verts, &tmap_num, p);
				model_collide_emit_polygon(poly_verts, nv, tmap_num, poly_index, tree, out_triangles);
			}

			++i;
			break;
		}
	}

	// copy point list
	Assert(n_verts != -1);

	tree->point_list = (vec3d*)vm_malloc(sizeof(vec3d) * n_verts);

	for ( i = 0; i < (size_t)n_verts; ++i ) {
		tree->point_list[i] = *Mc_point_list[i];
	}

	tree->n_verts = n_verts;
}

bool mc_shield_check_common(shield_tri	*tri)
{
	vec3d * points[3];
	vec3d hitpoint;
	 
	float dist;
	float sphere_check_closest_shield_dist = FLT_MAX;

	// Check to see if Mc_pmly is facing away from ray.  If so, don't bother
	// checking it.
	if (vm_vec_dot(&Mc_direction,&tri->norm) > 0.0f)	{
		return false;
	}
	// get the vertices in the form the next function wants them
	for (int j = 0; j < 3; j++ )
		points[j] = &Mc_pm->shield.verts[tri->verts[j]].pos;

	if (!(Mc->flags & MC_CHECK_SPHERELINE) ) {	// Don't do this test for sphere colliding against shields
		// Find the intersection of this ray with the plane that the Mc_pmly
		// lies in
		dist = fvi_ray_plane(NULL, points[0],&tri->norm,&Mc_p0,&Mc_direction,0.0f);

		if ( dist < 0.0f ) return false; // If the ray is behind the plane there is no collision
		if ( !(Mc->flags & MC_CHECK_RAY) && (dist > 1.0f) ) return false; // The ray isn't long enough to intersect the plane

		// Find the hit Mc_pmint
		vm_vec_scale_add( &hitpoint, &Mc_p0, &Mc_direction, dist );
	
		// Check to see if the Mc_pmint of intersection is on the plane.  If so, this
		// also finds the uv's where the ray hit.
		if ( fvi_point_face(&hitpoint, 3, points, &tri->norm, NULL,NULL,NULL ) )	{
			Mc->hit_dist = dist;
			Mc->shield_hit_tri = (int)(tri - Mc_pm->shield.tris.get());
			Mc->hit_point = hitpoint;
			Mc->hit_normal = tri->norm;
			Mc->hit_submodel = -1;
			Mc->num_hits++;
			return true;		// We hit, so we're done
		}
	} else {		// Sphere check against shield
					// This needs to look at *all* shield tris and not just return after the first hit

		// HACK HACK!! The 10000.0 is the face radius, I didn't know this,
		// so I'm assume 10000 would be as big as ever.
		mc_check_sphereline_face(3, points, points[0], &tri->norm, NULL, 0);
		if (Mc->num_hits && Mc->hit_dist < sphere_check_closest_shield_dist) {

			// same behavior whether face or edge
			// normal, edge_hit, hit_point all updated thru sphereline_face
			sphere_check_closest_shield_dist = Mc->hit_dist;
			Mc->shield_hit_tri = (int)(tri - Mc_pm->shield.tris.get());
			Mc->hit_submodel = -1;
			Mc->num_hits++;
			return true;		// We hit, so we're done
		}
	} // Mc->flags & MC_CHECK_SPHERELINE else

	return false;
}

bool mc_check_sldc(int offset)
{
	//ShivanSpS - Changed the type char for a type int (Now SLC2)
	if (offset > Mc_pm->sldc_size - 5) //no way is this big enough
		return false;

	int* type_p = (int*)(Mc_pm->shield_collision_tree.get() + offset);

	// not used
	//int *size_p = (int *)(Mc_pm->shield_collision_tree+offset+4);
	// split and polygons
	auto* minbox_p = (vec3d*)(Mc_pm->shield_collision_tree.get() + offset + 8);
	auto* maxbox_p = (vec3d*)(Mc_pm->shield_collision_tree.get() + offset + 20);

	// split
	auto* front_offset_p = (unsigned int*)(Mc_pm->shield_collision_tree.get() + offset + 32);
	auto* back_offset_p = (unsigned int*)(Mc_pm->shield_collision_tree.get() + offset + 36);

	// polygons
	auto* num_polygons_p = (unsigned int*)(Mc_pm->shield_collision_tree.get() + offset + 32);

	auto* shld_polys = (unsigned int*)(Mc_pm->shield_collision_tree.get() + offset + 36);



	// see if it fits inside our bbox
	if (!mc_ray_boundingbox(minbox_p, maxbox_p, &Mc_p0, &Mc_direction, NULL)) {
		return false;
	}

	if (*type_p == 0) // SPLIT
	{
		return mc_check_sldc(offset + *front_offset_p) || mc_check_sldc(offset + *back_offset_p);
	}
	else
	{
		// poly list
		shield_tri* tri;
		for (unsigned int i = 0; i < *num_polygons_p; i++)
		{
			tri = &Mc_pm->shield.tris[shld_polys[i]];

			mc_shield_check_common(tri);

		} // for (unsigned int i = 0; i < leaf->num_polygons; i++)
	}

	// shouldn't be reached
	return false;
}

// checks a vector collision against a ships shield (if it has shield points defined).
void mc_check_shield()
{
	int i;


	if ( Mc_pm->shield.ntris < 1 )
		return;
	if (Mc_pm->shield_collision_tree)
	{
		mc_check_sldc(0); // see if we hit the SLDC
	}
	else
	{				
		for (i = 0; i < Mc_pm->shield.ntris; i++) {
			mc_shield_check_common(&Mc_pm->shield.tris[i]);
		}
	}//model has shield_collsion_tree
}


// This function recursively checks a submodel and its children
// for a collision with a vector.
void mc_check_subobj( int mn )
{
	vec3d tempv;
	vec3d hitpt;		// used in bounding box check
	bsp_info * sm;
	int i;

	Assert( mn >= 0 );
	Assert( mn < Mc_pm->n_models );
	if ( (mn < 0) || (mn>=Mc_pm->n_models) ) return;
	
	sm = &Mc_pm->submodel[mn];
	if (sm->flags[Model::Submodel_flags::No_collisions]) return; // don't do collisions
	if (sm->flags[Model::Submodel_flags::Nocollide_this_only]) goto NoHit; // Don't collide for this model, but keep checking others

	if (Mc->flags & MC_RESPECT_DETAIL_BOX_SPHERE) {
		vec3d local;
		vm_vec_sub(&local, &Eye_position, Mc->pos);
		vm_vec_rotate(&local, &local, Mc->orient);
		if (!model_render_check_detail_box(&local, Mc_pm, mn, MR_NORMAL))
			goto NoHit; //This submodel is a detail box that is not displayed, skip it
	}

	// Rotate the world check points into the current subobject's 
	// frame of reference.
	// After this block, Mc_p0, Mc_p1, Mc_direction, and Mc_mag are correct
	// and relative to this subobjects' frame of reference.
	vm_vec_sub(&tempv, Mc->p0, &Mc_base);
	vm_vec_rotate(&Mc_p0, &tempv, &Mc_orient);

	vm_vec_sub(&tempv, Mc->p1, &Mc_base);
	vm_vec_rotate(&Mc_p1, &tempv, &Mc_orient);
	vm_vec_sub(&Mc_direction, &Mc_p1, &Mc_p0);

	// bail early if no ray exists
	if ( IS_VEC_NULL(&Mc_direction) ) {
		return;
	}

	if (Mc_pm->detail[0] == mn)	{
		// Quickly bail if we aren't inside the full model bbox
		if (!mc_ray_boundingbox( &Mc_pm->mins, &Mc_pm->maxs, &Mc_p0, &Mc_direction, NULL))	{
			return;
		}

		// If we are checking the root submodel, then we might want to check	
		// the shield at this point
		if ((Mc->flags & MC_CHECK_SHIELD) && (Mc_pm->shield.ntris > 0 )) {
			mc_check_shield();
			return;
		}
	}

	if (!(Mc->flags & MC_CHECK_MODEL)) {
		return;
	}
	
	Mc_submodel = mn;

	// Check if the ray intersects this subobject's bounding box
	if ( mc_ray_boundingbox(&sm->min, &sm->max, &Mc_p0, &Mc_direction, &hitpt) ) {
		if (Mc->flags & MC_ONLY_BOUND_BOX) {
			float dist = vm_vec_dist( &Mc_p0, &hitpt );

			// If the ray is behind the plane there is no collision
			if (dist < 0.0f) {
				goto NoHit;
			}

			// The ray isn't long enough to intersect the plane
			if ( !(Mc->flags & MC_CHECK_RAY) && (dist > Mc_mag) ) {
				goto NoHit;
			}

			// If the ray hits, but a closer intersection has already been found, return
			if ( Mc->num_hits && (dist >= Mc->hit_dist) ) {
				goto NoHit;
			}

			Mc->hit_dist = dist;
			Mc->hit_point = hitpt;
			Mc->hit_submodel = Mc_submodel;
			Mc->hit_bitmap = -1;
			Mc->num_hits++;
		} else if (!(Mc->flags & MC_COLLIDE_ALL) && Mc->num_hits && (vm_vec_dist(&Mc_p0, &hitpt) >= Mc->hit_dist)) {
			// A closer hit was already found (in an earlier-visited sibling submodel, or a
			// parent) than this box's own nearest ray-entry point -- nothing inside this
			// submodel's own geometry could possibly beat it, so skip testing it at all. Avoids a
			// full BVH traversal per submodel box the ray merely touches on the whole-model,
			// multi-submodel-recursion queries that dominate per-frame cost (weapon-vs-capital-ship
			// hull, AI's ai_big_pick_attack_point() -- self-documented as ~10% of AI frametime,
			// turret hull-blocking checks, beams): these routinely recurse through 100+ submodels
			// with no MC_SUBMODEL narrowing. Not applied under MC_COLLIDE_ALL, which needs every
			// hit, not just the nearest. Still falls through to check this submodel's children below
			// (goto NoHit) -- a child's own box gets the identical check independently, this only
			// skips the current submodel's own polygons/BVH.
			goto NoHit;
		} else {
			// The ray intersects this bounding box, so we have to check all the
			// polygons in this submodel.
			if (Mc->lod > 0 && sm->num_details > 0) {
				bsp_info* lod_sm = sm;

				for (i = Mc->lod - 1; i >= 0; i--) {
					if (sm->details[i] != -1) {
						lod_sm = &Mc_pm->submodel[sm->details[i]];

						// mprintf(("Checking %s collision for %s using %s instead\n", Mc_pm->filename, sm->name,
						// lod_sm->name));
						break;
					}
				}

				if (lod_sm->triangle_bvh) {
					model_collide_bvh_triangle(lod_sm->triangle_bvh.get());
				}
			} else {
				if (sm->triangle_bvh) {
					model_collide_bvh_triangle(sm->triangle_bvh.get());
				}
			}
		}
	}

NoHit:

	// If we're only checking one submodel, return
	if (Mc->flags & MC_SUBMODEL)	{
		return;
	}

	
	// If this subobject doesn't have any children, we're done checking it.
	if ( sm->num_children < 1 ) return;
	
	// Save instance (Mc_orient, Mc_base, Mc_point_base)
	matrix saved_orient = Mc_orient;
	vec3d saved_base = Mc_base;
	
	// Check all of this subobject's children
	i = sm->first_child;
	while ( i >= 0 )	{
		auto csm = &Mc_pm->submodel[i];
		matrix instance_orient = vmd_identity_matrix;
		vec3d instance_offset = csm->offset;
		bool blown_off = false;
		bool collision_checked = false;
		
		if ( Mc_pmi ) {
			auto csmi = &Mc_pmi->submodel[i];
			instance_orient = csmi->canonical_orient;
			vm_vec_add2(&instance_offset, &csmi->canonical_offset);

			blown_off = csmi->blown_off;
			collision_checked = !Mc->collision_checked.empty() && Mc->collision_checked[i];
		}

		// Don't check it or its children if it is destroyed
		// or if it's set to no collision
		if ( !blown_off && !collision_checked && !csm->flags[Model::Submodel_flags::No_collisions] )	{
			vm_vec_unrotate(&Mc_base, &instance_offset, &saved_orient);
			vm_vec_add2(&Mc_base, &saved_base);

			vm_matrix_x_matrix(&Mc_orient, &saved_orient, &instance_orient);

			mc_check_subobj( i );
		}

		i = csm->next_sibling;
	}

}

MONITOR(NumFVI)

// See model.h for usage.   I don't want to put the
// usage here because you need to see the #defines and structures
// this uses while reading the help.   
int model_collide(mc_info *mc_info_obj)
{
	Mc = mc_info_obj;

	MONITOR_INC(NumFVI,1);

	Mc->num_hits = 0;				// How many collisions were found
	Mc->shield_hit_tri = -1;	// Assume we won't hit any shield polygons
	Mc->hit_bitmap = -1;
	Mc->edge_hit = false;

	// Resolved once here, not re-tested per triangle by mc_check_bvh_triangle()'s hot loop -- see
	// the Mc_triangle_face_test_fn declaration near the top of this file.
	Mc_triangle_face_test_fn =
		(Mc->flags & MC_CHECK_SPHERELINE) ? mc_check_triangle_sphereline_face : mc_check_triangle_face;

	if ( (Mc->flags & MC_CHECK_SHIELD) && (Mc->flags & MC_CHECK_MODEL) )	{
		Error( LOCATION, "Checking both shield and model!\n" );
		return 0;
	}

	//Fill in some global variables that all the model collide routines need internally.
	Mc_pm = model_get(Mc->model_num);
	Mc_orient = *Mc->orient;
	Mc_base = *Mc->pos;
	Mc_mag = vm_vec_dist( Mc->p0, Mc->p1 );

	if ( Mc->model_instance_num >= 0 ) {
		Mc_pmi = model_get_instance(Mc->model_instance_num);
	} else {
		Mc_pmi = NULL;
	}

	if (Mc_pmi && Mc->collision_checked.size() != static_cast<size_t>(Mc_pm->n_models)) {
		Assertion(Mc->collision_checked.empty(), "model_collide was called with a dirty mc_info state! Please report to the SCP.");
		Mc->collision_checked.resize(Mc_pm->n_models, 0);
	}

	// DA 11/19/98 - disable this check for rotating submodels
	// Don't do check if for very small movement
//	if (Mc_mag < 0.01f) {
//		return 0;
//	}

	float model_radius;		// How big is the model we're checking against
	int first_submodel;		// Which submodel gets returned as hit if MC_ONLY_SPHERE specified

	if ( (Mc->flags & MC_SUBMODEL) || (Mc->flags & MC_SUBMODEL_INSTANCE) )	{
		first_submodel = Mc->submodel_num;
		// collision_rad is the submodel's authored rad (see model_load()'s bounding-sphere check).
		model_radius = Mc_pm->submodel[first_submodel].collision_rad;
	} else {
		first_submodel = Mc_pm->detail[0];
		model_radius = Mc_pm->rad;
	}

	if ( Mc->flags & MC_CHECK_SPHERELINE ) {
		if ( Mc->radius <= 0.0f ) {
			Warning(LOCATION, "Attempting to collide with a sphere, but the sphere's radius is <= 0.0f!\n\n(model file is %s; submodel is %d, mc_flags are %d)", Mc_pm->filename, first_submodel, Mc->flags);
			return 0;
		}

		// Do a quick check on the Bounding Sphere
		if (fvi_segment_sphere(&Mc->hit_point_world, Mc->p0, Mc->p1, Mc->pos, model_radius+Mc->radius) )	{
			if ( Mc->flags & MC_ONLY_SPHERE )	{
				Mc->hit_point = Mc->hit_point_world;
				Mc->hit_submodel = first_submodel;
				Mc->num_hits++;
				return (Mc->num_hits > 0);
			}
			// continue checking polygons.
		} else {
			return 0;
		}
	} else {
		int r;

		// Do a quick check on the Bounding Sphere
		if ( Mc->flags & MC_CHECK_RAY ) {
			r = fvi_ray_sphere(&Mc->hit_point_world, Mc->p0, Mc->p1, Mc->pos, model_radius);
		} else {
			r = fvi_segment_sphere(&Mc->hit_point_world, Mc->p0, Mc->p1, Mc->pos, model_radius);
		}
		if (r) {
			if ( Mc->flags & MC_ONLY_SPHERE ) {
				Mc->hit_point = Mc->hit_point_world;
				Mc->hit_submodel = first_submodel;
				Mc->num_hits++;
				return (Mc->num_hits > 0);
			}
			// continue checking polygons.
		} else {
			return 0;
		}

	}

	// Check only one subobject; or check submodel and any children
	if ( (Mc->flags & MC_SUBMODEL) || (Mc->flags & MC_SUBMODEL_INSTANCE) ) {
		// note: within this function, MC_SUBMODEL will return after one check; but MC_SUBMODEL_INSTANCE will not
		mc_check_subobj(Mc->submodel_num);
	}
	// Check all the the highest detail model polygons and subobjects for intersections
	else {
		// Don't check it or its children if it is destroyed
		if ( Mc_pmi ) {
			if ( !Mc_pmi->submodel[Mc_pm->detail[0]].blown_off ) {
				mc_check_subobj(Mc_pm->detail[0]);
			}
		} else {
			mc_check_subobj(Mc_pm->detail[0]);
		}
	}


	//If we found a hit, then rotate it into world coordinates	
	if ( Mc->num_hits )	{
		if ( Mc->flags & MC_SUBMODEL )	{
			// If we're just checking one submodel, don't use normal instancing to find world points
			vm_vec_unrotate(&Mc->hit_point_world, &Mc->hit_point, Mc->orient);
			vm_vec_add2(&Mc->hit_point_world, Mc->pos);
		} else {
			if ( Mc_pmi ) {
				model_instance_local_to_global_point(&Mc->hit_point_world, &Mc->hit_point, Mc_pm, Mc_pmi, Mc->hit_submodel, Mc->orient, Mc->pos);
			} else {
				model_local_to_global_point(&Mc->hit_point_world, &Mc->hit_point, Mc_pm, Mc->hit_submodel, Mc->orient, Mc->pos);
			}
		}
		
		// do the same for the list of hitpoints, if necessary
		if (Mc->flags & MC_COLLIDE_ALL) {
			for (size_t i = 0; i < Mc->hit_points_all.size(); i++) {
				if (Mc->flags & MC_SUBMODEL) {
					vm_vec_unrotate(&Mc->hit_points_all[i], &Mc->hit_points_all[i], Mc->orient);
					vm_vec_add2(&Mc->hit_points_all[i], Mc->pos);
				} else {
					if (Mc_pmi) {
						model_instance_local_to_global_point(&Mc->hit_points_all[i], &Mc->hit_points_all[i], Mc_pm, Mc_pmi, Mc->hit_submodels_all[i], Mc->orient, Mc->pos);
					}
					else {
						model_local_to_global_point(&Mc->hit_points_all[i], &Mc->hit_points_all[i], Mc_pm, Mc->hit_submodels_all[i], Mc->orient, Mc->pos);
					}
				}
			}
		}

	}

	return Mc->num_hits;
}
