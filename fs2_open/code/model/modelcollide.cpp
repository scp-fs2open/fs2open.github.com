/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Model/ModelCollide.cpp $
 * $Revision: 2.5 $
 * $Date: 2004-07-12 16:32:56 $
 * $Author: Kazan $
 *
 * Routines for detecting collisions of models.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2004/03/05 09:02:07  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2004/01/30 07:39:08  Goober5000
 * whew - I just went through all the code I ever added (or at least, that I could
 * find that I commented with a Goober5000 tag) and added a bunch of Asserts
 * and error-checking
 * --Goober5000
 *
 * Revision 2.2  2002/12/07 01:37:42  bobboau
 * inital decals code, if you are worried a bug is being caused by the decals code it's only references are in,
 * collideshipweapon.cpp line 262, beam.cpp line 2771, and modelinterp.cpp line 2949.
 * it needs a better renderer, but is in prety good shape for now,
 * I also (think) I squashed a bug in the warpmodel code
 *
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     3/08/99 7:03p Dave
 * First run of new object update system. Looks very promising.
 * 
 * 4     1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 3     11/19/98 11:07p Andsager
 * Check in of physics and collision detection of rotating submodels
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 82    4/22/98 9:43p John
 * Added code to allow checking of invisible faces, flagged by any texture
 * name with invisible in it.
 * 
 * 81    4/02/98 8:16a John
 * Fixed Assert in model_collide with large ships
 * 
 * 80    3/31/98 5:18p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 79    3/25/98 1:36p Andsager
 * comment out assert
 * 
 * 78    3/18/98 3:04p Andsager
 * Increase collision error warning distance
 * 
 * 77    3/09/98 12:08a Andsager
 * Remove assert
 * 
 * 76    2/12/98 3:47p Hoffoss
 * Added MC_CHECK_RAY support in model_collide(), since Fred would be
 * better using rays instead of line segments for intersection tests.
 * 
 * 75    2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 74    1/27/98 11:02a John
 * Added first rev of sparks.   Made all code that calls model_render call
 * clear_instance first.   Made debris pieces not render by default when
 * clear_instance is called.
 * 
 * 73    1/19/98 11:30a Sandeep
 * Remove Int3() in sphere poly collision
 * 
 * 72    1/16/98 5:30p Andsager
 * Added debug code for bad collisions.
 * 
 * 71    1/12/98 9:21p Andsager
 * Modify calling procedure to fvi_sphere_plane.  
 * 
 * 70    12/22/97 9:17a John
 * added functions for Dave that check only one submodel.
 * 
 * 69    12/18/97 9:26a John
 * put back in some critical "debug" code DaveA deleted.  
 * 
 * 68    12/16/97 5:25p Andsager
 * Get best collision info.  Comment out some debug info.
 * 
 * 67    11/14/97 5:29p Andsager
 * Improve bounding box test for sphere:polygon.  Set higher tolerance for
 * collision alert.
 * 
 * 66    11/12/97 11:09p Jasen
 * Comment out another Int3() that causes grief, but seems to do little
 * good.
 * 
 * 65    11/12/97 12:13p Mike
 * Suppress disruptive, but apparently harmless, Int3().
 * 
 * 64    11/05/97 5:48p Andsager
 * Added debug code to check_sphereline_face.  Expected radius separation
 * can be off by up to .003 for 1 m sphere.
 * 
 * 63    10/31/97 4:01p John
 * added face_Radius to call to check_face
 * 
 * 62    10/31/97 3:19p John
 * changed id field in face to be radius
 * 
 * 61    10/30/97 3:41p Andsager
 * fixed bug in check_sphereline_face that allowed collisions at negative
 * times
 * 
 * 60    10/28/97 4:57p John
 * Put Andsager's new sphereline collide code officially into the code
 * base and did a little restructuring.  Fixed a few little bugs with it
 * and added some simple bounding box elimination and did some timings.
 * 
 * 
 * 59    10/25/97 10:14a Andsager
 * Moved SPHERE_POLY_CHECK to ObjCollide.h
 * 
 * 58    10/22/97 10:26p Andsager
 * modify (slightly) mc_check_shield to allow sphere-polygon collisions
 * 
 * 57    10/19/97 9:32p Andsager
 * Using model_collide with 2nd parameter radius (default = 0)
 * 
 * 56    10/17/97 2:06a Andsager
 * moved assert
 * 
 * 55    10/17/97 1:26a Andsager
 * added radius to mc_info struct.  add sphere_face collisions
 * 
 * 54    9/17/97 5:12p John
 * Restructured collision routines.  Probably broke a lot of stuff.
 * 
 * 53    9/15/97 5:45p John
 * took out chunk stuff.
 * made pofview display thrusters as blue polies.
 * 
 * 52    9/12/97 8:54a John
 * made model_collide fill in mc_info even if invalid model passed
 * through.
 * 
 * 51    9/03/97 12:02a Andsager
 * implement sphere_sphere collision detection.  Use debug console
 * CHECK_SPHERE
 * 
 * 50    8/25/97 11:27a John
 * took out the USE_OCTANTS define, since we want to always use them.    
 * 
 * 49    8/15/97 4:10p John
 * new code to use the new octrees style bsp trees
 * 
 * 48    8/11/97 10:43a Mike
 * Enable octant checking by defining USE_OCTANTS
 * 
 * 47    8/11/97 10:19a John
 * fixed a bug that was setting the wrong triangle for shield hit with new
 * octant stuff.
 * 
 * 46    8/10/97 3:04p Lawrance
 * get rid of warning if USE_OCTANTS no defined
 * 
 * 45    8/08/97 3:58p Mike
 * Shield hit system.
 * Shield damage handled/managed-by-AI on a quadrant basis.
 * 
 * 44    8/06/97 5:40p Mike
 * Add hit_normal field to mc_info so we can have ships react to the
 * object they hit.
 * 
 * 43    7/22/97 9:41a John
 * Made flat faces appear in octant list, so collision detection now
 * works.  Made them do smoothing if needed.
 * 
 * 42    7/21/97 2:29p John
 * moved the bounding sphere check into the model collide code
 * 
 * 
 * 41    7/03/97 9:14a John
 * fixed incorrect octant vertices.
 * 
 * 40    6/27/97 12:15p John
 * added support for only checking model's bounding box... made hud_target
 * use it.
 * 
 * 39    6/26/97 11:44a John
 * made model_collide require that you specifically tell it to check the
 * model to avoid unnecessary checks.
 * 
 * 38    6/26/97 11:19a John
 * Made model face & shield collisions look only at octants it needs to.
 * Shield sped up 4x, faces sped up about 2x.
 * 
 * 37    6/23/97 1:42p John
 * updated sphere checking code
 * 
 * 36    5/30/97 3:42p Mike
 * Shield mesh and hit system.
 * 
 * 35    5/28/97 12:52p John
 * Added code in to check shield mesh.
 * 
 * 34    4/17/97 6:06p John
 * New code/data for v19 of BSPGEN with smoothing and zbuffer
 * optimizations.
 * 
 * 33    4/07/97 3:08p John
 * added flag to model_collide to allow collision checks with rays not
 * just segments.
 * 
 * 32    4/07/97 10:37a John
 * Took out limits on number of children a submodel can have.    (So now
 * capital ships can have tons of children on them).
 * 
 * 31    4/01/97 1:05p John
 * took out debug printf
 * 
 * 30    4/01/97 1:03p John
 * Changed fvi_ray_plane to take a dir, not two points.
 * 
 * 29    3/24/97 4:43p John
 * speed up chunked collision detection by only checking cubes the vector
 * goes through.
 * 
 * 28    3/24/97 3:55p John
 * renamed fvi functions so rays and segments aren't confusing.
 * 
 * 27    3/24/97 3:26p John
 * Cleaned up and restructured model_collide code and fvi code.  In fvi
 * made code that finds uvs work..  Added bm_get_pixel to BmpMan.
 * 
 * 26    3/20/97 5:19p John
 * began adding a fast bounding box checks for collision detection.
 * 
 * 
 * 25    2/26/97 5:28p John
 * Fixed a bunch of chunked model bugs.  Basically they almost work better
 * than non-chunked objects!
 * 
 * 24    2/07/97 10:45a John
 * Initial bare bones implementation of blowing off turrets.
 * 
 * 23    2/04/97 7:37p John
 * Finally!!! Turret stuff working!
 * 
 * 22    1/31/97 11:36a John
 * 
 * 21    1/28/97 10:07a John
 * More turret stuff, still not working, though.
 * 
 * 20    1/27/97 2:35p Adam
 * (John) One of the models is getting a subcall with model 10 billion or
 * something so I just made it return when it gets something that big.
 * 
 * 19    1/27/97 10:25a Mike
 * Fix AVI file load error message.
 * Hack physics to allow weapons to pass through a destroyed shield
 * triangle.  Need to have something cleaner (and faster).
 * 
 * 18    1/24/97 4:55p John
 * Started adding code for turning turrets.
 * Had to change POF format to 18.0
 *
 * $NoKeywords: $
 */


#define MODEL_LIB

#include "model/model.h"
#include "math/vecmat.h"
#include "graphics/tmapper.h"
#include "math/fvi.h"
#include "model/modelsinc.h"

// memory tracking - ALWAYS INCLUDE LAST
#include "mcd/mcd.h"


#define TOL		1E-4
#define DIST_TOL	1.0

// Some global variables that get set by model_collide and are used internally for
// checking a collision rather than passing a bunch of parameters around. These are
// not persistant between calls to model_collide

static mc_info		*Mc;				// The mc_info passed into model_collide
	
static polymodel	*Mc_pm;			// The polygon model we're checking
static int			Mc_submodel;	// The current submodel we're checking

static matrix		Mc_orient;		// A matrix to rotate a world point into the current
											// submodel's frame of reference.
static vector		Mc_base;			// A point used along with Mc_orient.

static vector		Mc_p0;			// The ray origin rotated into the current submodel's frame of reference
static vector		Mc_p1;			// The ray end rotated into the current submodel's frame of reference
static float		Mc_mag;			// The length of the ray
static vector		Mc_direction;	// A vector from the ray's origin to it's end, in the current submodel's frame of reference

static vector *	Mc_point_list[MAX_POLYGON_VECS];	// A pointer to the current submodel's vertex list

static float		Mc_edge_time;


// Returns non-zero if vector from p0 to pdir 
// intersects the bounding box.
// hitpos could be NULL, so don't fill it if it is.
int mc_ray_boundingbox( vector *min, vector *max, vector * p0, vector *pdir, vector *hitpos )
{

	vector tmp_hitpos;
	if ( hitpos == NULL )	{
		hitpos = &tmp_hitpos;
	}


	if ( Mc->flags & MC_CHECK_SPHERELINE )	{

		// In the case of a sphere, just increase the size of the box by the radius 
		// of the sphere in all directions.

		vector sphere_mod_min, sphere_mod_max;

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



// ----- 
// mc_check_face
// nv -- number of vertices
// verts -- actual vertices
// plane_pnt -- A point on the plane.  Could probably use the first vertex.
// plane_norm -- normal of the plane
// uvl_list -- list of uv coords for the poly.
// ntmap -- The tmap index into the model's textures array.
//
// detects whether or not a vector has collided with a polygon.  vector points stored in global
// Mc_p0 and Mc_p1.  Results stored in global mc_info * Mc.

static void mc_check_face(int nv, vector **verts, vector *plane_pnt, float face_rad, vector *plane_norm, uv_pair *uvl_list, int ntmap, ubyte *poly)
{
	vector	hit_point;
	float		dist;
	float		u, v;

	// Check to see if poly is facing away from ray.  If so, don't bother
	// checking it.
	if (vm_vec_dot(&Mc_direction,plane_norm) > 0.0f)	{
		return;
	}

	// Find the intersection of this ray with the plane that the poly
	dist = fvi_ray_plane(NULL, plane_pnt, plane_norm, &Mc_p0, &Mc_direction, 0.0f);

	if ( dist < 0.0f ) return; // If the ray is behind the plane there is no collision
	if ( !(Mc->flags & MC_CHECK_RAY) && (dist > 1.0f) ) return; // The ray isn't long enough to intersect the plane

	// If the ray hits, but a closer intersection has already been found, return
	if ( Mc->num_hits && (dist >= Mc->hit_dist ) ) return;	

	// Find the hit point
	vm_vec_scale_add( &hit_point, &Mc_p0, &Mc_direction, dist );
	
	// Check to see if the point of intersection is on the plane.  If so, this
	// also finds the uv's where the ray hit.
	if ( fvi_point_face(&hit_point, nv, verts, plane_norm, &u,&v, uvl_list ) )	{
		Mc->hit_dist = dist;

		Mc->hit_point = hit_point;
		Mc->hit_submodel = Mc_submodel;

		Mc->hit_normal = *plane_norm;

		if ( uvl_list )	{
			Mc->hit_u = u;
			Mc->hit_v = v;
			Mc->hit_bitmap = Mc_pm->textures[ntmap];			
		}
		
		if(ntmap >= 0){
			Mc->t_poly = poly;
			Mc->f_poly = NULL;
		} else {
			Mc->t_poly = NULL;
			Mc->f_poly = poly;
		}

//		mprintf(( "Bing!\n" ));

		Mc->num_hits++;
	}
}

// ----------------------------------------------------------------------------------------------------------
// check face with spheres
//
//	inputs:	nv				=>		number of vertices
//				verts			=>		array of vertices
//				plane_pnt	=>		center point in plane (about which radius is measured)
//				face_rad		=>		radius of face 
//				plane_norm	=>		normal of face
static void mc_check_sphereline_face( int nv, vector ** verts, vector * plane_pnt, float face_rad, vector * plane_norm, uv_pair * uvl_list, int ntmap, ubyte *poly)
{
	vector	hit_point;
	float		u, v;
	float		delta_t;			// time sphere takes to cross from one side of plane to the other
	float		face_t;			// time at which face touches plane
									// NOTE all times are normalized so that t = 1.0 at the end of the frame
	int		check_face = 1;		// assume we'll check the face.
	int		check_edges = 1;		// assume we'll check the edges.
	
	// Check to see if poly is facing away from ray.  If so, don't bother
	// checking it.

	if (vm_vec_dot(&Mc_direction,plane_norm) > 0.0f)	{
		return;
	}

	// Find the intersection of this sphere with the plane of the poly
	if ( !fvi_sphere_plane( &hit_point, &Mc_p0, &Mc_direction, Mc->radius, plane_norm, plane_pnt, &face_t, &delta_t ) ) {
		return;
	}

	if ( face_t < 0 || face_t > 1) {
		check_face = 0;		// If the ray is behind the plane there is no collision
	}

	if ( !(Mc->flags & MC_CHECK_RAY) && (face_t > 1.0f) ) {
		check_face = 0;		// The ray isn't long enough to intersect the plane
	}

	// If the ray hits, but a closer intersection has already been found, don't check face
	if ( Mc->num_hits && (face_t >= Mc->hit_dist ) ) {
		check_face = 0;		// The ray isn't long enough to intersect the plane
	}


	vector temp_sphere;
	vector temp_dir;
	float temp_dist;
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
			Mc->edge_hit = 0;

			if ( uvl_list )	{
				Mc->hit_u = u;
				Mc->hit_v = v;
				Mc->hit_bitmap = Mc_pm->textures[ntmap];
			}

			if(ntmap >= 0){
				Mc->t_poly = poly;
				Mc->f_poly = NULL;
			} else {
				Mc->t_poly = NULL;
				Mc->f_poly = poly;
			}

			Mc->num_hits++;
			check_edges = 0;
			vm_vec_scale_add( &temp_sphere, &Mc_p0, &Mc_direction, Mc->hit_dist );
			temp_dist = vm_vec_dist( &temp_sphere, &hit_point );
			if ( (temp_dist - DIST_TOL > Mc->radius) || (temp_dist + DIST_TOL < Mc->radius) ) {
				// get Andsager
				mprintf(("Estimated radius error: Estimate %f, actual %f Mc->radius\n", temp_dist, Mc->radius));
			}
			vm_vec_sub( &temp_dir, &hit_point, &temp_sphere );
			// Assert( vm_vec_dotprod( &temp_dir, &Mc_direction ) > 0 );
		}
	}


	if ( check_edges ) {

		// Either (face_t) is out of range or we miss the face
		// Check for sphere hitting edge

		// If checking shields, we *still* need to check edges

		// First check whether sphere can hit edge in allowed time range
		if ( face_t > 1.0f || face_t+delta_t < 0.0f )	{
			return;
		}

		// this is where we need another test to cull checking for edges
		// PUT TEST HERE

		// check each edge to see if we hit, find the closest edge
		// Mc->hit_dist stores the best edge time of *all* faces
		float sphere_time;
		if ( fvi_polyedge_sphereline(&hit_point, &Mc_p0, &Mc_direction, Mc->radius, nv, verts, &sphere_time)) {

			Assert( sphere_time >= 0.0f );
			vm_vec_scale_add( &temp_sphere, &Mc_p0, &Mc_direction, sphere_time );
			temp_dist = vm_vec_dist( &temp_sphere, &hit_point );
			if ( (temp_dist - DIST_TOL > Mc->radius) || (temp_dist + DIST_TOL < Mc->radius) ) {
				// get Andsager
				mprintf(("Estimated radius error: Estimate %f, actual %f Mc->radius\n", temp_dist, Mc->radius));
			}
			vm_vec_sub( &temp_dir, &hit_point, &temp_sphere );
//			Assert( vm_vec_dotprod( &temp_dir, &Mc_direction ) > 0 );

			if ( (Mc->num_hits==0) || (sphere_time < Mc->hit_dist) ) {
				// This is closer than best so far
				Mc->hit_dist = sphere_time;
				Mc->hit_point = hit_point;
				Mc->hit_submodel = Mc_submodel;
				Mc->edge_hit = 1;
				Mc->hit_bitmap = Mc_pm->textures[ntmap];				

				if(ntmap >= 0){
					Mc->t_poly = poly;
					Mc->f_poly = NULL;
				} else {
					Mc->t_poly = NULL;
					Mc->f_poly = poly;
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


// Point list
// +0      int         id
// +4      int         size
// +8      int         n_verts
// +12     int         n_norms
// +16     int         offset from start of chunk to vertex data
// +20     n_verts*char    norm_counts
// +offset             vertex data. Each vertex n is a point followed by norm_counts[n] normals.     
void model_collide_defpoints(ubyte * p)
{
	int n;
	int nverts = w(p+8);	
	int offset = w(p+16);	

	ubyte * normcount = p+20;
	vector *src = vp(p+offset);
	
	Assert( nverts < MAX_POLYGON_VECS );

	for (n=0; n<nverts; n++ )	{

		Mc_point_list[n] = src;

		src += normcount[n]+1;
	} 
}


// Flat Poly
// +0      int         id
// +4      int         size 
// +8      vector      normal
// +20     vector      center
// +32     float       radius
// +36     int         nverts
// +40     byte        red
// +41     byte        green
// +42     byte        blue
// +43     byte        pad
// +44     nverts*int  vertlist
void model_collide_flatpoly(ubyte * p)
{
	int i;
	int nv;
	vector * points[TMAP_MAX_VERTS];
	short *verts;

	nv = w(p+36);
	if ( nv < 0 ) return;	

	verts = (short *)(p+44);
	
	for (i=0;i<nv;i++)	{
		points[i] = Mc_point_list[verts[i*2]];
	}

	if ( Mc->flags & MC_CHECK_SPHERELINE )	{
		mc_check_sphereline_face(nv, points, vp(p+20), fl(p+32), vp(p+8), NULL, -1, p);
	} else {
		mc_check_face(nv, points, vp(p+20), fl(p+32), vp(p+8), NULL, -1, p);
	}
}


// Textured Poly
// +0      int         id
// +4      int         size 
// +8      vector      normal
// +20     vector      normal_point
// +32     int         tmp = 0
// +36     int         nverts
// +40     int         tmap_num
// +44     nverts*(model_tmap_vert) vertlist (n,u,v)
void model_collide_tmappoly(ubyte * p)
{
	int i;
	int nv;
	uv_pair uvlist[TMAP_MAX_VERTS];
	vector * points[TMAP_MAX_VERTS];
	model_tmap_vert *verts;

	nv = w(p+36);
	if ( nv < 0 ) return;

	int tmap_num = w(p+40);
	Assert(tmap_num >= 0 && tmap_num < MAX_MODEL_TEXTURES);	// Goober5000

	if ( (!(Mc->flags & MC_CHECK_INVISIBLE_FACES)) && (Mc_pm->textures[tmap_num] < 0) )	{
		// Don't check invisible polygons.
		return;
	}

	verts = (model_tmap_vert *)(p+44);

	for (i=0;i<nv;i++)	{
		points[i] = Mc_point_list[verts[i].vertnum];
		uvlist[i].u = verts[i].u;
		uvlist[i].v = verts[i].v;
	}

	if ( Mc->flags & MC_CHECK_SPHERELINE )	{
		mc_check_sphereline_face(nv, points, vp(p+20), fl(p+32), vp(p+8), uvlist, tmap_num, p);
	} else {
		mc_check_face(nv, points, vp(p+20), fl(p+32), vp(p+8), uvlist, tmap_num, p);
	}
}


// Sortnorms
// +0      int         id
// +4      int         size 
// +8      vector      normal
// +20     vector      center
// +32     float       radius
// 36     int     front offset
// 40     int     back offset
// 44     int     prelist offset
// 48     int     postlist offset
// 52     int     online offset

int model_collide_sub( void *model_ptr );

void model_collide_sortnorm(ubyte * p)
{
	int frontlist = w(p+36);
	int backlist = w(p+40);
	int prelist = w(p+44);
	int postlist = w(p+48);
	int onlist = w(p+52);

	if ( Mc_pm->version >= 2000 )	{
		if (!mc_ray_boundingbox( vp(p+56), vp(p+68), &Mc_p0, &Mc_direction, NULL ))	{
			return;
		}
	}

	if (prelist) model_collide_sub(p+prelist);
	if (backlist) model_collide_sub(p+backlist);
	if (onlist) model_collide_sub(p+onlist);
	if (frontlist) model_collide_sub(p+frontlist);
	if (postlist) model_collide_sub(p+postlist);
}

//calls the object interpreter to render an object.  The object renderer
//is really a seperate pipeline. returns true if drew
int model_collide_sub(void *model_ptr )
{
	ubyte *p = (ubyte *)model_ptr;
	int chunk_type, chunk_size;

	chunk_type = w(p);
	chunk_size = w(p+4);

	while (chunk_type != OP_EOF)	{

//		mprintf(( "Processing chunk type %d, len=%d\n", chunk_type, chunk_size ));

		switch (chunk_type) {
		case OP_EOF: return 1;
		case OP_DEFPOINTS:	model_collide_defpoints(p); break;
		case OP_FLATPOLY:		model_collide_flatpoly(p); break;
		case OP_TMAPPOLY:		model_collide_tmappoly(p); break;
		case OP_SORTNORM:		model_collide_sortnorm(p); break;
		case OP_BOUNDBOX:	
			if (!mc_ray_boundingbox( vp(p+8), vp(p+20), &Mc_p0, &Mc_direction, NULL ))	{
				return 1;
			}
			break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in model_collide_sub\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
	}
	return 1;
}



// checks a vector collision against a ships shield (if it has shield points defined).
void mc_check_shield()
{
	int i, j;
	vector * points[3];
	float dist;
	float sphere_check_closest_shield_dist = FLT_MAX;

	if ( Mc_pm->shield.ntris < 1 )
		return;

	int o;
	for (o=0; o<8; o++ )	{
		model_octant * poct1 = &Mc_pm->octants[o];

		if (!mc_ray_boundingbox( &poct1->min, &poct1->max, &Mc_p0, &Mc_direction, NULL ))	{
			continue;
		}
		
		for (i = 0; i < poct1->nshield_tris; i++) {
			vector hitpoint;
			shield_tri	* tri;
			tri = poct1->shield_tris[i];

			// Check to see if Mc_pmly is facing away from ray.  If so, don't bother
			// checking it.
			if (vm_vec_dot(&Mc_direction,&tri->norm) > 0.0f)	{
				continue;
			}

			// get the vertices in the form the next function wants them
			for (j = 0; j < 3; j++ )
				points[j] = &Mc_pm->shield.verts[tri->verts[j]].pos;

			if (!(Mc->flags & MC_CHECK_SPHERELINE) ) {	// Don't do this test for sphere colliding against shields
				// Find the intersection of this ray with the plane that the Mc_pmly
				// lies in
				dist = fvi_ray_plane(NULL, points[0],&tri->norm,&Mc_p0,&Mc_direction,0.0f);

				if ( dist < 0.0f ) continue; // If the ray is behind the plane there is no collision
				if ( !(Mc->flags & MC_CHECK_RAY) && (dist > 1.0f) ) continue; // The ray isn't long enough to intersect the plane

				// Find the hit Mc_pmint
				vm_vec_scale_add( &hitpoint, &Mc_p0, &Mc_direction, dist );
			
				// Check to see if the Mc_pmint of intersection is on the plane.  If so, this
				// also finds the uv's where the ray hit.
				if ( fvi_point_face(&hitpoint, 3, points, &tri->norm, NULL,NULL,NULL ) )	{
					Mc->hit_dist = dist;
					Mc->shield_hit_tri = tri - Mc_pm->shield.tris;
					Mc->hit_point = hitpoint;
					Mc->hit_normal = tri->norm;
					Mc->hit_submodel = -1;
					Mc->num_hits++;
					return;		// We hit, so we're done
				}
			} else {		// Sphere check against shield
							// This needs to look at *all* shield tris and not just return after the first hit

				// HACK HACK!! The 10000.0 is the face radius, I didn't know this,
				// so I'm assume 10000 would be as big as ever.
				mc_check_sphereline_face(3, points, points[0], 10000.0f, &tri->norm, NULL, 0, NULL);
				if (Mc->num_hits && Mc->hit_dist < sphere_check_closest_shield_dist) {

					// same behavior whether face or edge
					// normal, edge_hit, hit_point all updated thru sphereline_face
					sphere_check_closest_shield_dist = Mc->hit_dist;
					Mc->shield_hit_tri = tri - Mc_pm->shield.tris;
					Mc->hit_submodel = -1;
					Mc->num_hits++;
				}
			}
		}
	}
}


// This function recursively checks a submodel and its children
// for a collision with a vector.
void mc_check_subobj( int mn )
{
	vector tempv;
	vector hitpt;		// used in bounding box check
	bsp_info * sm;
	int i;

	Assert( mn >= 0 );
	Assert( mn < Mc_pm->n_models );

	if ( (mn < 0) || (mn>=Mc_pm->n_models) ) return;
	
	sm = &Mc_pm->submodel[mn];

	// Rotate the world check points into the current subobject's 
	// frame of reference.
	// After this block, Mc_p0, Mc_p1, Mc_direction, and Mc_mag are correct
	// and relative to this subobjects' frame of reference.
	vm_vec_sub(&tempv, Mc->p0, &Mc_base);
	vm_vec_rotate(&Mc_p0, &tempv, &Mc_orient);

	vm_vec_sub(&tempv, Mc->p1, &Mc_base);
	vm_vec_rotate(&Mc_p1, &tempv, &Mc_orient);
	vm_vec_sub(&Mc_direction, &Mc_p1, &Mc_p0);

	// If we are checking the root submodel, then we might want
	// to check the shield at this point
	if (Mc_pm->detail[0] == mn)	{

		// Do a quick out on the entire bounding box of the object
		if (!mc_ray_boundingbox( &Mc_pm->mins, &Mc_pm->maxs, &Mc_p0, &Mc_direction, NULL))	{
			return;
		}
			
		// Check shield if we're supposed to
		if ((Mc->flags & MC_CHECK_SHIELD) && (Mc_pm->shield.ntris > 0 )) {
		//	Mc->flags &= ~MC_CHECK_SPHERELINE;
		//	mc_check_shield();
		//	int ray_num_hits = Mc->num_hits;
		//	Mc->num_hits = 0;
		//	Mc->flags |= MC_CHECK_SPHERELINE;
			mc_check_shield();
		//	if ( (ray_num_hits > 0)  && (Mc->num_hits == 0))
		//		Int3();
			return;
		}

	}

	if(!(Mc->flags & MC_CHECK_MODEL)) return;
	
	Mc_submodel = mn;

	// Check if the ray intersects this subobject's bounding box 	
	if (mc_ray_boundingbox(&sm->min, &sm->max, &Mc_p0, &Mc_direction, &hitpt))	{

		// The ray interects this bounding box, so we have to check all the
		// polygons in this submodel.
		if ( Mc->flags & MC_ONLY_BOUND_BOX )	{
			float dist = vm_vec_dist( &Mc_p0, &hitpt );

			if ( dist < 0.0f ) goto NoHit; // If the ray is behind the plane there is no collision
			if ( !(Mc->flags & MC_CHECK_RAY) && (dist > Mc_mag) ) goto NoHit; // The ray isn't long enough to intersect the plane

			// If the ray hits, but a closer intersection has already been found, return
			if ( Mc->num_hits && (dist >= Mc->hit_dist ) ) goto NoHit;	

			Mc->hit_dist = dist;
			Mc->hit_point = hitpt;
			Mc->hit_submodel = Mc_submodel;
			Mc->hit_bitmap = -1;
			Mc->num_hits++;
		} else {
			model_collide_sub(sm->bsp_data);
		}
	} else {
		//Int3();
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
	vector saved_base = Mc_base;
	
	// Check all of this subobject's children
	i = sm->first_child;
	while ( i>-1 )	{
		bsp_info * csm = &Mc_pm->submodel[i];

		// Don't check it or its children if it is destroyed
		if (!csm->blown_off)	{	
			//instance for this subobject
			matrix tm;

			vm_vec_unrotate(&Mc_base, &csm->offset, &saved_orient );
			vm_vec_add2(&Mc_base, &saved_base );

			vm_angles_2_matrix(&tm, &csm->angs);
			vm_matrix_x_matrix(&Mc_orient, &saved_orient, &tm);

			mc_check_subobj( i );
		}

		i = csm->next_sibling;
	}

}

MONITOR(NumFVI);

// See model.h for usage.   I don't want to put the
// usage here because you need to see the #defines and structures
// this uses while reading the help.   
int model_collide(mc_info * mc_info)
{
	Mc = mc_info;

	MONITOR_INC(NumFVI,1);

	Mc->num_hits = 0;				// How many collisions were found
	Mc->shield_hit_tri = -1;	// Assume we won't hit any shield polygons
	Mc->hit_bitmap = -1;
	Mc->edge_hit = 0;

	if ( (Mc->flags & MC_CHECK_SHIELD) && (Mc->flags & MC_CHECK_MODEL) )	{
		Error( LOCATION, "Checking both shield and model!\n" );
		return 0;
	}

	//Fill in some global variables that all the model collide routines need internally.
	Mc_pm = model_get(Mc->model_num);
	Mc_orient = *Mc->orient;
	Mc_base = *Mc->pos;
	Mc_mag = vm_vec_dist( Mc->p0, Mc->p1 );
	Mc_edge_time = FLT_MAX;

	// DA 11/19/98 - disable this check for rotating submodels
	// Don't do check if for very small movement
//	if (Mc_mag < 0.01f) {
//		return 0;
//	}

	float model_radius;		// How big is the model we're checking against
	int first_submodel;		// Which submodel gets returned as hit if MC_ONLY_SPHERE specified

	if ( (Mc->flags & MC_SUBMODEL) || (Mc->flags & MC_SUBMODEL_INSTANCE) )	{
		first_submodel = Mc->submodel_num;
		model_radius = Mc_pm->submodel[first_submodel].rad;
	} else {
		first_submodel = Mc_pm->detail[0];
		model_radius = Mc_pm->rad;
	}

	if ( Mc->flags & MC_CHECK_SPHERELINE ) {
		Assert( Mc->radius > 0.0f );

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

	if ( Mc->flags & MC_SUBMODEL )	{
		// Check only one subobject
		mc_check_subobj( Mc->submodel_num );
		// Check submodel and any children
	} else if (Mc->flags & MC_SUBMODEL_INSTANCE) {
		mc_check_subobj(Mc->submodel_num);
	} else {
		// Check all the the highest detail model polygons and subobjects for intersections

		// Don't check it or its children if it is destroyed
		if (!Mc_pm->submodel[Mc_pm->detail[0]].blown_off)	{	
			mc_check_subobj( Mc_pm->detail[0] );
		}
	}


	//If we found a hit, then rotate it into world coordinates	
	if ( Mc->num_hits )	{
		if ( Mc->flags & MC_SUBMODEL )	{
			// If we're just checking one submodel, don't use normal instancing to find world points
			vm_vec_unrotate(&Mc->hit_point_world, &Mc->hit_point, Mc->orient);
			vm_vec_add2(&Mc->hit_point_world, Mc->pos);
		} else {
			model_find_world_point(&Mc->hit_point_world, &Mc->hit_point,Mc->model_num, Mc->hit_submodel, Mc->orient, Mc->pos);
		}
	}


	return Mc->num_hits;

}
