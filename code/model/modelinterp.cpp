/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#define MODEL_LIB

#include "bmpman/bmpman.h"
#include "cmdline/cmdline.h"
#include "debugconsole/console.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "globalincs/linklist.h"
#include "graphics/2d.h"
#include "graphics/util/GPUMemoryHeap.h"
#include "io/key.h"
#include "io/timer.h"
#include "math/fvi.h"
#include "math/staticrand.h"
#include "mission/missionparse.h"
#include "model/modelrender.h"
#include "model/modelsinc.h"
#include "nebula/neb.h"
#include "parse/parselo.h"
#include "particle/particle.h"
#include "render/3dinternal.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "tracing/Monitor.h"
#include "tracing/tracing.h"
#include "utils/Random.h"
#include "weapon/shockwave.h"

#include <climits>


float model_radius = 0;

// Some debug variables used externally for displaying stats
#ifndef NDEBUG
int modelstats_num_polys = 0;
int modelstats_num_polys_drawn = 0;
int modelstats_num_verts = 0;
int modelstats_num_sortnorms = 0;
int modelstats_num_boxes = 0;
#endif

extern fix game_get_overall_frametime();	// for texture animation

typedef struct model_light {
	ubyte r, g, b;
	ubyte spec_r, spec_g, spec_b;
} model_light;

// a lighting object
typedef struct model_light_object {
	model_light *lights;

	int		objnum;
	int		skip;
	int		skip_max;
} model_light_object;

struct bsp_vertex
{
	vec3d position;
	vec3d normal;
	uv_pair tex_coord;
	ubyte r, g, b, a;
};

struct bsp_polygon
{
	uint Start_index;
	uint Num_verts;

	int texture;
};

class bsp_polygon_data
{
	SCP_vector<vec3d> Vertex_list;
	SCP_vector<vec3d> Normal_list; 

	SCP_vector<bsp_vertex> Polygon_vertices;
	SCP_vector<bsp_polygon> Polygons;

	ubyte* Lights;

	int Num_polies[MAX_MODEL_TEXTURES];
	int Num_verts[MAX_MODEL_TEXTURES];

	int Num_flat_polies;
	int Num_flat_verts;

	void process_bsp(int offset, ubyte* bsp_data);
	void process_defpoints(int off, ubyte* bsp_data);
	void process_sortnorm(int offset, ubyte* bsp_data);
	void process_sortnorm2(int offset, ubyte* bsp_data);
	void process_tmap(int offset, ubyte* bsp_data);
	void process_tmap2(int offset, ubyte* bsp_data);
	void process_flat(int offset, ubyte* bsp_data);
public:
	bsp_polygon_data(ubyte* bsp_data);

	int get_num_triangles(int texture);
	int get_num_lines(int texture);

	void generate_triangles(int texture, vertex *vert_ptr, vec3d* norm_ptr);
	void generate_lines(int texture, vertex *vert_ptr);
};

/**
 * @brief Vertex structure for passing data to the GPU
 */
struct interp_vertex {
	uv_pair uv;
	vec3d normal;
	vec4 tangent;
	float modelId;
	vec3d pos;
};

// -----------------------
// Local variables
//

static uint Num_interp_verts_allocated = 0;
vec3d **Interp_verts = NULL;
static vertex *Interp_points = NULL;
static vertex *Interp_splode_points = NULL;
vec3d *Interp_splode_verts = NULL;
static uint Interp_num_verts = 0;

static float Interp_box_scale = 1.0f; // this is used to scale both detail boxes and spheres

// -------------------------------------------------------------------
// lighting save stuff 
//

model_light_object Interp_lighting_temp;
model_light_object *Interp_lighting = &Interp_lighting_temp;
int Interp_use_saved_lighting = 0;
int Interp_saved_lighting_full = 0;
//
// lighting save stuff 
// -------------------------------------------------------------------


static uint Num_interp_norms_allocated = 0;
static vec3d **Interp_norms = NULL;
static ubyte *Interp_light_applied = NULL;
static uint Interp_num_norms = 0;
static ubyte *Interp_lights;

// Stuff to control rendering parameters
static color Interp_outline_color;
static int Interp_detail_level_locked = -1;
static uint Interp_flags = 0;

// If non-zero, then the subobject gets scaled by Interp_thrust_scale.
int Interp_thrust_scale_subobj = 0;
float Interp_thrust_scale = 0.1f;
static float Interp_thrust_scale_x = 0.0f;//added -bobboau
static float Interp_thrust_scale_y = 0.0f;//added -bobboau

static int Interp_thrust_bitmap = -1;
static int Interp_thrust_glow_bitmap = -1;
static float Interp_thrust_glow_noise = 1.0f;
static bool Interp_afterburner = false;

// Bobboau's thruster stuff
static int Interp_secondary_thrust_glow_bitmap = -1;
static int Interp_tertiary_thrust_glow_bitmap = -1;
static int Interp_distortion_thrust_bitmap = -1;
static float Interp_thrust_glow_rad_factor = 1.0f;
static float Interp_secondary_thrust_glow_rad_factor = 1.0f;
static float Interp_tertiary_thrust_glow_rad_factor = 1.0f;
static float Interp_distortion_thrust_rad_factor = 1.0f;
static float Interp_distortion_thrust_length_factor = 1.0f;
static float Interp_thrust_glow_len_factor = 1.0f;
static vec3d Interp_thrust_rotvel = ZERO_VECTOR;
static bool Interp_draw_distortion = true;

// Bobboau's warp stuff
static float Interp_warp_scale_x = 1.0f;
static float Interp_warp_scale_y = 1.0f;
static float Interp_warp_scale_z = 1.0f;

// if != -1, use this bitmap when rendering ship insignias
static int Interp_insignia_bitmap = -1;

// if != -1, use this bitmap when rendering with a forced texture
static int Interp_forced_bitmap = -1;

// our current level of detail (LOD)
int Interp_detail_level = 0;

// forward references
int model_should_render_engine_glow(int objnum, int bank_obj);
int model_interp_get_texture(texture_info *tinfo, fix base_frametime);

void model_deallocate_interp_data()
{
	if (Interp_verts != nullptr) {
		vm_free(Interp_verts);
		Interp_verts = nullptr;
	}

	if (Interp_points != nullptr) {
		vm_free(Interp_points);
		Interp_points = nullptr;
	}

	if (Interp_splode_points != nullptr) {
		vm_free(Interp_splode_points);
		Interp_splode_points = nullptr;
	}

	if (Interp_splode_verts != nullptr) {
		vm_free(Interp_splode_verts);
		Interp_splode_verts = nullptr;
	}

	if (Interp_norms != nullptr) {
		vm_free(Interp_norms);
		Interp_norms = nullptr;
	}

	if (Interp_light_applied != nullptr) {
		vm_free(Interp_light_applied);
		Interp_light_applied = nullptr;
	}

	if (Interp_lighting_temp.lights != nullptr) {
		vm_free(Interp_lighting_temp.lights);
		Interp_lighting_temp.lights = nullptr;
	}

	Num_interp_verts_allocated = 0;
	Num_interp_norms_allocated = 0;
}

extern void model_collide_allocate_point_list(int n_points);
extern void model_collide_free_point_list();

void model_allocate_interp_data(uint n_verts, uint n_norms)
{
	static ubyte dealloc = 0;

	if (!dealloc) {
		atexit(model_deallocate_interp_data);
		atexit(model_collide_free_point_list);
		dealloc = 1;
	}

	Assert( (n_verts || Num_interp_verts_allocated) && (n_norms || Num_interp_norms_allocated) );

	if (n_verts > Num_interp_verts_allocated) {
		if (Interp_verts != NULL) {
			vm_free(Interp_verts);
			Interp_verts = NULL;
		}
		// Interp_verts can't be reliably realloc'd so free and malloc it on each resize (no data needs to be carried over)
		Interp_verts = (vec3d**) vm_malloc( n_verts * sizeof(vec3d *) );

		Interp_points = (vertex*) vm_realloc( Interp_points, n_verts * sizeof(vertex) );
		Interp_splode_points = (vertex*) vm_realloc( Interp_splode_points, n_verts * sizeof(vertex) );
		Interp_splode_verts = (vec3d*) vm_realloc( Interp_splode_verts, n_verts * sizeof(vec3d) );

		Num_interp_verts_allocated = n_verts;

		// model collide needs a similar size to resize it based on this new value
		model_collide_allocate_point_list( n_verts );
	}

	if (n_norms > Num_interp_norms_allocated) {
		if (Interp_norms != NULL) {
			vm_free(Interp_norms);
			Interp_norms = NULL;
		}
		// Interp_norms can't be reliably realloc'd so free and malloc it on each resize (no data needs to be carried over)
		Interp_norms = (vec3d**) vm_malloc( n_norms * sizeof(vec3d *) );

		// these next two lighting things aren't values that need to be carried over, but we need to make sure they are 0 by default
		if (Interp_light_applied != NULL) {
			vm_free(Interp_light_applied);
			Interp_light_applied = NULL;
		}

		if (Interp_lighting_temp.lights != NULL) {
			vm_free(Interp_lighting_temp.lights);
			Interp_lighting_temp.lights = NULL;
		}

		Interp_light_applied = (ubyte*) vm_malloc( n_norms * sizeof(ubyte) );
		Interp_lighting_temp.lights = (model_light*) vm_malloc( n_norms * sizeof(model_light) );

		memset( Interp_light_applied, 0, n_norms * sizeof(ubyte) );
		memset( Interp_lighting_temp.lights, 0, n_norms * sizeof(model_light) );

		Num_interp_norms_allocated = n_norms;
	}

	Interp_num_verts = n_verts;
	Interp_num_norms = n_norms;

	// check that everything is still usable (works in release and debug builds)
	Verify( Interp_points != NULL );
	Verify( Interp_splode_points != NULL );
	Verify( Interp_verts != NULL );
	Verify( Interp_splode_verts != NULL );
	Verify( Interp_norms != NULL );
	Verify( Interp_light_applied != NULL );
}

void interp_clear_instance()
{
	Interp_thrust_scale = 0.1f;
	Interp_thrust_scale_x = 0.0f;//added-Bobboau
	Interp_thrust_scale_y = 0.0f;//added-Bobboau
	Interp_thrust_bitmap = -1;
	Interp_thrust_glow_bitmap = -1;
	Interp_thrust_glow_noise = 1.0f;
	Interp_insignia_bitmap = -1;
	Interp_afterburner = false;

	// Bobboau's thruster stuff
	{
		Interp_thrust_glow_rad_factor = 1.0f;

		Interp_secondary_thrust_glow_bitmap = -1;
		Interp_secondary_thrust_glow_rad_factor = 1.0f;

		Interp_tertiary_thrust_glow_bitmap = -1;
		Interp_tertiary_thrust_glow_rad_factor = 1.0f;

		Interp_thrust_glow_len_factor = 1.0f;

		vm_vec_zero(&Interp_thrust_rotvel);
	}

	Interp_box_scale = 1.0f;

	Interp_detail_level_locked = -1;

	Interp_forced_bitmap = -1;
}

/**
 * Scales the engines thrusters by this much
 */
void model_set_thrust(int  /*model_num*/, mst_info *mst)
{
	if (mst == NULL) {
		Int3();
		return;
	}

	Interp_thrust_scale = mst->length.xyz.z;
	Interp_thrust_scale_x = mst->length.xyz.x;
	Interp_thrust_scale_y = mst->length.xyz.y;

	CLAMP(Interp_thrust_scale, 0.1f, 1.0f);

	Interp_thrust_bitmap = mst->primary_bitmap;
	Interp_thrust_glow_bitmap = mst->primary_glow_bitmap;
	Interp_secondary_thrust_glow_bitmap = mst->secondary_glow_bitmap;
	Interp_tertiary_thrust_glow_bitmap = mst->tertiary_glow_bitmap;
	Interp_distortion_thrust_bitmap = mst->distortion_bitmap;

	Interp_thrust_glow_noise = mst->glow_noise;
	Interp_afterburner = mst->use_ab;

	Interp_thrust_rotvel = mst->rotvel;

	Interp_thrust_glow_rad_factor = mst->glow_rad_factor;
	Interp_secondary_thrust_glow_rad_factor = mst->secondary_glow_rad_factor;
	Interp_tertiary_thrust_glow_rad_factor = mst->tertiary_glow_rad_factor;
	Interp_thrust_glow_len_factor = mst->glow_length_factor;
	Interp_distortion_thrust_rad_factor = mst->distortion_rad_factor;
	Interp_distortion_thrust_length_factor = mst->distortion_length_factor;

	Interp_draw_distortion = mst->draw_distortion;
}

bool splodeing = false;
int splodeingtexture = -1;
float splode_level = 0.0f;

float GEOMETRY_NOISE = 0.0f;

// Point list
// +0      int         id
// +4      int         size
// +8      int         n_verts
// +12     int         n_norms
// +16     int         offset from start of chunk to vertex data
// +20     n_verts*char    norm_counts
// +offset             vertex data. Each vertex n is a point followed by norm_counts[n] normals.
void model_interp_splode_defpoints(ubyte * p, polymodel * /*pm*/, bsp_info * /*sm*/, float dist)
{
	if(dist==0.0f)return;

	if(dist<0.0f)dist*=-1.0f;

	int n;
	int nverts = w(p+8);	
	int offset = w(p+16);
	int nnorms = 0;

	ubyte * normcount = p+20;
	vertex *dest = Interp_splode_points;
	vec3d *src = vp(p+offset);

	for (n = 0; n < nverts; n++) {
		nnorms += normcount[n];
	}

	model_allocate_interp_data(nverts, nnorms);

	vec3d dir;

	for (n=0; n<nverts; n++ )	{	
		Interp_splode_verts[n] = *src;		
			
		src++;

		vm_vec_avg_n(&dir, normcount[n], src);
		vm_vec_normalize(&dir);

		for(int i=0; i<normcount[n]; i++)src++;

		vm_vec_scale_add2(&Interp_splode_verts[n], &dir, dist);

		g3_rotate_vertex(dest, &Interp_splode_verts[n]);
		
		dest++;

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
void model_interp_defpoints(ubyte * p, polymodel *pm, bsp_info *sm)
{
	if(splodeing)model_interp_splode_defpoints(p, pm, sm, splode_level*model_radius);

	uint i, n;
	uint nverts = uw(p+8);	
	uint offset = uw(p+16);
	uint next_norm = 0;
	uint nnorms = 0;

	ubyte * normcount = p+20;
	vertex *dest = NULL;
	vec3d *src = vp(p+offset);

	// Get pointer to lights
	Interp_lights = p+20+nverts;

	for (i = 0; i < nverts; i++) {
		nnorms += normcount[i];
	}

	// allocate new Interp data if size is greater than we already have ready to use
	model_allocate_interp_data(nverts, nnorms);

	dest = Interp_points;

	Assert( dest != NULL );

	#ifndef NDEBUG
	modelstats_num_verts += nverts;
	#endif


	if (Interp_thrust_scale_subobj)	{

		// Only scale vertices that aren't on the "base" of 
		// the effect.  Base is something Adam decided to be
		// anything under 1.5 meters, hence the 1.5f.
		float min_thruster_dist = -1.5f;

		if ( Interp_flags & MR_IS_MISSILE )	{
			min_thruster_dist = 0.5f;
		}

		for (n=0; n<nverts; n++ )	{
			vec3d tmp;

			Interp_verts[n] = src;

			// Only scale vertices that aren't on the "base" of 
			// the effect.  Base is something Adam decided to be
			// anything under 1.5 meters, hence the 1.5f.
			if ( src->xyz.z < min_thruster_dist )	{
				tmp.xyz.x = src->xyz.x * 1.0f;
				tmp.xyz.y = src->xyz.y * 1.0f;
				tmp.xyz.z = src->xyz.z * Interp_thrust_scale;
			} else {
				tmp = *src;
			}
			
			g3_rotate_vertex(dest,&tmp);
		
			src++;		// move to normal

			for (i=0; i<normcount[n]; i++ )	{
				Interp_light_applied[next_norm] = 0;
				Interp_norms[next_norm] = src;

				next_norm++;
				src++;
			}
			dest++;
		} 
	} else if ( (Interp_warp_scale_x != 1.0f) || (Interp_warp_scale_y != 1.0f) || (Interp_warp_scale_z != 1.0f)) {
		for (n=0; n<nverts; n++ )	{
			vec3d tmp;

			Interp_verts[n] = src;

			tmp.xyz.x = (src->xyz.x) * Interp_warp_scale_x;
			tmp.xyz.y = (src->xyz.y) * Interp_warp_scale_y;
			tmp.xyz.z = (src->xyz.z) * Interp_warp_scale_z;
			
			g3_rotate_vertex(dest,&tmp);
		
			src++;		// move to normal

			for (i=0; i<normcount[n]; i++ )	{
				Interp_light_applied[next_norm] = 0;
				Interp_norms[next_norm] = src;

				next_norm++;
				src++;
			}
			dest++;
		} 
	} else {
		vec3d point;

		for (n=0; n<nverts; n++ )	{	

			if(GEOMETRY_NOISE!=0.0f){
				GEOMETRY_NOISE = model_radius / 50;

				Interp_verts[n] = src;	
				point.xyz.x = src->xyz.x + frand_range(GEOMETRY_NOISE,-GEOMETRY_NOISE);
				point.xyz.y = src->xyz.y + frand_range(GEOMETRY_NOISE,-GEOMETRY_NOISE);
				point.xyz.z = src->xyz.z + frand_range(GEOMETRY_NOISE,-GEOMETRY_NOISE);
						
				g3_rotate_vertex(dest, &point);
			}else{
				Interp_verts[n] = src;	
				g3_rotate_vertex(dest, src);
			}

			src++;		// move to normal

			for (i=0; i<normcount[n]; i++ )	{
				Interp_light_applied[next_norm] = 0;
				Interp_norms[next_norm] = src;

				next_norm++;
				src++;
			}
			dest++;
		}
	}

	Interp_num_norms = next_norm;
}

void model_interp_edge_alpha( ubyte *param_r, ubyte *param_g, ubyte *param_b, vec3d *pnt, vec3d *norm, float alpha, bool invert = false)
{
	vec3d r;

	vm_vec_sub(&r, &View_position, pnt);
	vm_vec_normalize(&r);

	float d = vm_vec_dot(&r, norm);

	if (d < 0.0f)
		d = -d;

	if (invert)
		*param_r = *param_g = *param_b = ubyte( fl2i((1.0f - d) * 254.0f * alpha));
	else
		*param_r = *param_g = *param_b = ubyte( fl2i(d * 254.0f * alpha) );
}

int Interp_subspace = 0;
float Interp_subspace_offset_u = 0.0f;
float Interp_subspace_offset_v = 0.0f;
ubyte Interp_subspace_r = 255;
ubyte Interp_subspace_g = 255;
ubyte Interp_subspace_b = 255;

void model_draw_debug_points( polymodel *pm, bsp_info * submodel, uint flags )
{
	if ( flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	// Draw a red pivot point
	gr_set_color(128,0,0);
	g3_draw_sphere_ez(&vmd_zero_vector, 2.0f );

	// Draw a green center of mass when drawing the hull
	if ( submodel && (submodel->parent==-1) )	{
		gr_set_color(0,128,0);
		g3_draw_sphere_ez( &pm->center_of_mass, 1.0f );
	}

	if ( submodel )	{
		// Draw a blue center point
		gr_set_color(0,0,128);
		g3_draw_sphere_ez( &submodel->geometric_center, 0.9f );
	}
	
	// Draw the bounding box
	int i;
	vertex pts[8];

	if ( submodel )	{
		for (i=0; i<8; i++ )	{
			g3_rotate_vertex( &pts[i], &submodel->bounding_box[i] );
		}
		gr_set_color(128,128,128);
		g3_draw_line( &pts[0], &pts[1] );
		g3_draw_line( &pts[1], &pts[2] );
		g3_draw_line( &pts[2], &pts[3] );
		g3_draw_line( &pts[3], &pts[0] );

		g3_draw_line( &pts[4], &pts[5] );
		g3_draw_line( &pts[5], &pts[6] );
		g3_draw_line( &pts[6], &pts[7] );
		g3_draw_line( &pts[7], &pts[4] );

		g3_draw_line( &pts[0], &pts[4] );
		g3_draw_line( &pts[1], &pts[5] );
		g3_draw_line( &pts[2], &pts[6] );
		g3_draw_line( &pts[3], &pts[7] );
	} else {
		gr_set_color(0,255,0);

		int j;
		for (j=0; j<8; j++ )	{

			vec3d	bounding_box[8];		// caclulated fron min/max
			model_calc_bound_box(bounding_box,&pm->octants[j].min,&pm->octants[j].max);

			for (i=0; i<8; i++ )	{
				g3_rotate_vertex( &pts[i], &bounding_box[i] );
			}
			gr_set_color(128,0,0);
			g3_draw_line( &pts[0], &pts[1] );
			g3_draw_line( &pts[1], &pts[2] );
			g3_draw_line( &pts[2], &pts[3] );
			g3_draw_line( &pts[3], &pts[0] );

			g3_draw_line( &pts[4], &pts[5] );
			g3_draw_line( &pts[5], &pts[6] );
			g3_draw_line( &pts[6], &pts[7] );
			g3_draw_line( &pts[7], &pts[4] );

			g3_draw_line( &pts[0], &pts[4] );
			g3_draw_line( &pts[1], &pts[5] );
			g3_draw_line( &pts[2], &pts[6] );
			g3_draw_line( &pts[3], &pts[7] );			
		}		
	}
}

/**
 * Debug code to show all the paths of a model
 */
void model_draw_paths_htl( int model_num, uint flags )
{
	int i,j;
	vec3d pnt;
	vec3d prev_pnt;
	polymodel * pm;	

	if ( flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	pm = model_get(model_num);

	if (pm->n_paths<1){
		return;
	}	

	int cull = gr_set_cull(0);
	for (i=0; i<pm->n_paths; i++ )	{
		for (j=0; j<pm->paths[i].nverts; j++ )
		{
			// Rotate point into world coordinates			
			pnt = pm->paths[i].verts[j].pos;

			// Pnt is now the x,y,z world coordinates of this vert.
			// For this example, I am just drawing a sphere at that
			// point.

			vertex tmp;
			g3_rotate_vertex(&tmp,&pnt);

			if ( pm->paths[i].verts[j].nturrets > 0 ){
				gr_set_color( 0, 0, 255 );						// draw points covered by turrets in blue
			} else {
				gr_set_color( 255, 0, 0 );
			}

			g3_render_sphere(&pnt, 0.5f);
			
			if (j)
			{
				//g3_draw_htl_line(&prev_pnt, &pnt);
				g3_render_line_3d(true, &prev_pnt, &pnt);
			}

			prev_pnt = pnt;
		}
	}

	gr_set_cull(cull);
}

/**
 * Docking bay and fighter bay paths
 */
void model_draw_bay_paths_htl(int model_num)
{
	int idx, s_idx;
	vec3d v1, v2;

	polymodel *pm = model_get(model_num);
	if(pm == NULL){
		return;
	}

	int cull = gr_set_cull(0);
	// render docking bay normals
	gr_set_color(0, 255, 0);
	for(idx=0; idx<pm->n_docks; idx++){
		for(s_idx=0; s_idx<pm->docking_bays[idx].num_slots; s_idx++){
			v1 = pm->docking_bays[idx].pnt[s_idx];
			vm_vec_scale_add(&v2, &v1, &pm->docking_bays[idx].norm[s_idx], 10.0f);

			// draw the point and normal
			g3_render_sphere(&v1, 2.0);
			//g3_draw_htl_line(&v1, &v2);
			g3_render_line_3d(true, &v1, &v2);
		}
	}

	// render figher bay paths
	gr_set_color(0, 255, 255);
		
	// iterate through the paths that exist in the polymodel, searching for $bayN pathnames
	for (idx = 0; idx<pm->n_paths; idx++) {
		if ( !strnicmp(pm->paths[idx].name, NOX("$bay"), 4) ) {						
			for(s_idx=0; s_idx<pm->paths[idx].nverts-1; s_idx++){
				v1 = pm->paths[idx].verts[s_idx].pos;
				v2 = pm->paths[idx].verts[s_idx+1].pos;

				//g3_draw_htl_line(&v1, &v2);
				g3_render_line_3d(true, &v1, &v2);
			}
		}
	}	

	gr_set_cull(cull);
}

static const int MAX_ARC_SEGMENT_POINTS = 50;
int Num_arc_segment_points = 0;
vec3d Arc_segment_points[MAX_ARC_SEGMENT_POINTS];

void interp_render_arc_segment( vec3d *v1, vec3d *v2, int depth )
{
	float d = vm_vec_dist_quick( v1, v2 );
	const float scaler = 0.30f;

	if ( (d < scaler) || (depth > 4) ) {
		// the real limit appears to be 33, so we should never hit this unless the code changes
		Assert( Num_arc_segment_points < MAX_ARC_SEGMENT_POINTS );

		memcpy( &Arc_segment_points[Num_arc_segment_points++], v2, sizeof(vec3d) );
	} else {
		// divide in half
		vec3d tmp;
		vm_vec_avg( &tmp, v1, v2 );
	
		tmp.xyz.x += (frand() - 0.5f) * d * scaler;
		tmp.xyz.y += (frand() - 0.5f) * d * scaler;
		tmp.xyz.z += (frand() - 0.5f) * d * scaler;

		// add additional point
		interp_render_arc_segment( v1, &tmp, depth+1 );
		interp_render_arc_segment( &tmp, v2, depth+1 );
	}
}

int Interp_lightning = 1;
DCF_BOOL( Arcs, Interp_lightning )

// Returns one of the following
#define IBOX_ALL_OFF 0
#define IBOX_ALL_ON 1
#define IBOX_SOME_ON_SOME_OFF 2

int interp_box_offscreen( vec3d *min, vec3d *max )
{
	if ( keyd_pressed[KEY_LSHIFT] )	{
		return IBOX_ALL_ON;
	}

	vec3d v[8];
	v[0].xyz.x = min->xyz.x; v[0].xyz.y = min->xyz.y; v[0].xyz.z = min->xyz.z;
	v[1].xyz.x = max->xyz.x; v[1].xyz.y = min->xyz.y; v[1].xyz.z = min->xyz.z;
	v[2].xyz.x = max->xyz.x; v[2].xyz.y = max->xyz.y; v[2].xyz.z = min->xyz.z;
	v[3].xyz.x = min->xyz.x; v[3].xyz.y = max->xyz.y; v[3].xyz.z = min->xyz.z;

	v[4].xyz.x = min->xyz.x; v[4].xyz.y = min->xyz.y; v[4].xyz.z = max->xyz.z;
	v[5].xyz.x = max->xyz.x; v[5].xyz.y = min->xyz.y; v[5].xyz.z = max->xyz.z;
	v[6].xyz.x = max->xyz.x; v[6].xyz.y = max->xyz.y; v[6].xyz.z = max->xyz.z;
	v[7].xyz.x = min->xyz.x; v[7].xyz.y = max->xyz.y; v[7].xyz.z = max->xyz.z;

	ubyte and_codes = 0xff;
	ubyte or_codes = 0xff;
	int i;

	for (i=0; i<8; i++ )	{
		vertex tmp;
		ubyte codes=g3_rotate_vertex( &tmp, &v[i] );

		or_codes |= codes;
		and_codes &= codes;
	}

	// If and_codes is set this means that all points lie off to the
	// same side of the screen.
	if (and_codes)	{
		return IBOX_ALL_OFF;	//all points off screen
	}

	// If this is set it means at least one of the points is offscreen,
	// but they aren't all off to the same side.
	if (or_codes)	{
		return IBOX_SOME_ON_SOME_OFF;
	}

	// They are all onscreen.
	return IBOX_ALL_ON;	
}

void model_render_shields( polymodel * pm, uint flags )
{
	int i, j;
	shield_tri *tri;
	vertex pnt0, prev_pnt, tmp = vertex();

	if ( flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	gr_set_color(0, 0, 200 );

	//	Scan all the triangles in the mesh.
	for (i=0; i<pm->shield.ntris; i++ )	{

		tri = &pm->shield.tris[i];

		if (g3_check_normal_facing(&pm->shield.verts[tri->verts[0]].pos,&tri->norm ) ) {

			//	Process the vertices.
			//	Note this rotates each vertex each time it's needed, very dumb.
			for (j=0; j<3; j++ )	{

				g3_rotate_vertex(&tmp, &pm->shield.verts[tri->verts[j]].pos );

				if (j)
					g3_draw_line(&prev_pnt, &tmp);
				else
					pnt0 = tmp;
				prev_pnt = tmp;
			}

			g3_draw_line(&pnt0, &prev_pnt);
		}
	}
}

int Model_texturing = 1;
int Model_polys = 1;

DCF_BOOL( model_texturing, Model_texturing )
DCF_BOOL( model_polys, Model_polys )

MONITOR( NumModelsRend )
MONITOR( NumHiModelsRend )
MONITOR( NumMedModelsRend )
MONITOR( NumLowModelsRend )

/**
 * Draws a bitmap with the specified 3d width & height 
 * @return 1 if off screen, 0 if not
 */
int model_get_rotated_bitmap_points(vertex *pnt,float angle, float rad, vertex *v)
{
	float sa, ca;
	int i;

	Assert( G3_count == 1 );
		
	sa = sinf(angle);
	ca = cosf(angle);

	float width, height;

	width = height = rad;

	v[0].world.xyz.x = (-width*ca - height*sa)*Matrix_scale.xyz.x + pnt->world.xyz.x;
	v[0].world.xyz.y = (-width*sa + height*ca)*Matrix_scale.xyz.y + pnt->world.xyz.y;
	v[0].world.xyz.z = pnt->world.xyz.z;
	v[0].screen.xyw.w = 0.0f;
	v[0].texture_position.u = 0.0f;
	v[0].texture_position.v = 0.0f;

	v[1].world.xyz.x = (width*ca - height*sa)*Matrix_scale.xyz.x + pnt->world.xyz.x;
	v[1].world.xyz.y = (width*sa + height*ca)*Matrix_scale.xyz.y + pnt->world.xyz.y;
	v[1].world.xyz.z = pnt->world.xyz.z;
	v[1].screen.xyw.w = 0.0f;
	v[1].texture_position.u = 1.0f;
	v[1].texture_position.v = 0.0f;

	v[2].world.xyz.x = (width*ca + height*sa)*Matrix_scale.xyz.x + pnt->world.xyz.x;
	v[2].world.xyz.y = (width*sa - height*ca)*Matrix_scale.xyz.y + pnt->world.xyz.y;
	v[2].world.xyz.z = pnt->world.xyz.z;
	v[2].screen.xyw.w = 0.0f;
	v[2].texture_position.u = 1.0f;
	v[2].texture_position.v = 1.0f;

	v[3].world.xyz.x = (-width*ca + height*sa)*Matrix_scale.xyz.x + pnt->world.xyz.x;
	v[3].world.xyz.y = (-width*sa - height*ca)*Matrix_scale.xyz.y + pnt->world.xyz.y;
	v[3].world.xyz.z = pnt->world.xyz.z;
	v[3].screen.xyw.w = 0.0f;
	v[3].texture_position.u = 0.0f;
	v[3].texture_position.v = 1.0f;

	ubyte codes_and=0xff;

	float sw,z;
	z = pnt->world.xyz.z - rad / 4.0f;
	if ( z < 0.0f ) z = 0.0f;
	sw = 1.0f / z;

	for (i=0; i<4; i++ )	{
		//now code the four points
		codes_and &= g3_code_vertex(&v[i]);
		v[i].flags = 0;		// mark as not yet projected
		g3_project_vertex(&v[i]);
		v[i].screen.xyw.w = sw;
	}

	if (codes_and)
		return 1;		//1 means off screen

	return 0;
}

float Interp_depth_scale = 1500.0f;

DCF(model_darkening,"Makes models darker with distance")
{
	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: model_darkening <float>\n" );
		dc_printf("Sets the distance at which to start blacking out models (namely asteroids).\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf( "model_darkening = %.1f\n", Interp_depth_scale );
		return;
	}

	dc_stuff_float(&Interp_depth_scale);

	dc_printf("model_darkening set to %.1f\n", Interp_depth_scale);
}

// tmp_detail_level
// 0 - Max
// 1
// 2
// 3
// 4 - None

#if MAX_DETAIL_LEVEL != 4
#error MAX_DETAIL_LEVEL is assumed to be 4 in ModelInterp.cpp
#endif


/**
 * Find the distance-squared from p0 to the closest point on a box.  Fills in closest_pt.
 * The box's dimensions are from 'min' to 'max'.
 */
float interp_closest_dist_sq_to_box( vec3d *closest_pt, const vec3d *p0, const vec3d *min, const vec3d *max )
{
	auto origin = p0->a1d;
	auto minB = min->a1d;
	auto maxB = max->a1d;
	auto coord = closest_pt->a1d;
	bool inside = true;
	int i;

	for (i=0; i<3; i++ )	{
		if ( origin[i] < minB[i] )	{
			coord[i] = minB[i];
			inside = false;
		} else if (origin[i] > maxB[i] )	{
			coord[i] = maxB[i];
			inside = false;
		} else {
			coord[i] = origin[i];
		}
	}
	
	if ( inside )	{
		return 0.0f;
	}

	return vm_vec_dist_squared(closest_pt, p0);
}


// Finds the closest point on a model to a point in space.  Actually only finds a point
// on the bounding box of the model.    
// Given:
//   model_num      Which model
//   orient         Orientation of the model
//   pos            Position of the model
//   eye_pos        Point that you want to find the closest point to
// Returns:
//   distance from eye_pos to closest_point.  0 means eye_pos is 
//   on or inside the bounding box.
//   Also fills in outpnt with the actual closest point (in local coordinates).
float model_find_closest_point( vec3d *outpnt, int model_num, int submodel_num, const matrix *orient, const vec3d *pos, const vec3d *eye_pos )
{
	vec3d tempv, eye_rel_pos;
	
	polymodel *pm = model_get(model_num);

	if ( submodel_num < 0 )	{
		 submodel_num = pm->detail[0];
	}

	// Rotate eye pos into object coordinates
	vm_vec_sub(&tempv, pos, eye_pos);
	vm_vec_rotate(&eye_rel_pos, &tempv, orient);

	return fl_sqrt( interp_closest_dist_sq_to_box( outpnt, &eye_rel_pos, &pm->submodel[submodel_num].min, &pm->submodel[submodel_num].max ) );
}

// Like the above, but finds the closest two points to each other.
float model_find_closest_points(vec3d *outpnt1, int model_num1, int submodel_num1, const matrix *orient1, const vec3d *pos1, vec3d *outpnt2, int model_num2, int submodel_num2, const matrix *orient2, const vec3d *pos2)
{
	polymodel *pm1 = model_get(model_num1);
	if (submodel_num1 < 0)
		submodel_num1 = pm1->detail[0];

	polymodel *pm2 = model_get(model_num2);
	if (submodel_num2 < 0)
		submodel_num2 = pm2->detail[0];

	// determine obj2's bounding box
	vec3d bounding_box[8];
	model_calc_bound_box(bounding_box, &pm2->submodel[submodel_num2].min, &pm2->submodel[submodel_num2].max);

	float closest_dist_sq = -1.0f;

	// check each point on it
	for (const auto &pt : bounding_box)
	{
		vec3d temp, rel_pt;

		// find world coordinates of this point
		vm_vec_unrotate(&temp, &pt, orient2);
		vm_vec_add(&rel_pt, &temp, pos2);

		// now find coordinates relative to obj1
		vm_vec_sub(&temp, pos1, &rel_pt);
		vm_vec_rotate(&rel_pt, &temp, orient1);

		// test this point
		float dist_sq = interp_closest_dist_sq_to_box(&temp, &rel_pt, &pm1->submodel[submodel_num1].min, &pm1->submodel[submodel_num1].max);
		if (closest_dist_sq < 0.0f || dist_sq < closest_dist_sq)
		{
			closest_dist_sq = dist_sq;

			// Note: As in the other function, both of these points are
			// in local coordinates relative to each of their models.
			*outpnt1 = temp;
			*outpnt2 = pt;
		}
	}

	// we have now found the closest point
	return fl_sqrt(closest_dist_sq);
}

int tiling = 1;
DCF(tiling, "Toggles rendering of tiled textures (default is on)")
{
	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Tiled textures are %s", tiling ? "ON" : "OFF");
		return;
	}

	tiling = !tiling;
	if(tiling){
		dc_printf("Tiled textures\n");
	} else {
		dc_printf("Non-tiled textures\n");
	}
}

void moldel_calc_facing_pts( vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w, float  /*z_add*/, vec3d *Eyeposition )
{
	vec3d uvec, rvec;
	vec3d temp;

	temp = *pos;

	vm_vec_sub( &rvec, Eyeposition, &temp );
	vm_vec_normalize( &rvec );	

	vm_vec_cross(&uvec,fvec,&rvec);
	vm_vec_normalize(&uvec);

	vm_vec_scale_add( top, &temp, &uvec, w/2.0f );
	vm_vec_scale_add( bot, &temp, &uvec, -w/2.0f );	
}

// Fills in an array with points from a model.
// Only gets up to max_num verts;
// Returns number of verts found;
static int submodel_get_points_internal(int model_num, int submodel_num)
{
	polymodel * pm;

	pm = model_get(model_num);

	if ( submodel_num < 0 )	{
		submodel_num = pm->detail[0];
	}

	ubyte *p = pm->submodel[submodel_num].bsp_data;
	int chunk_type, chunk_size;

	chunk_type = w(p);
	chunk_size = w(p+4);

	while (chunk_type != OP_EOF)	{
		switch (chunk_type) {
		case OP_DEFPOINTS:	{
				uint n;
				uint nverts = uw(p+8);				
				uint offset = uw(p+16);
				uint nnorms = 0;			

				ubyte * normcount = p+20;
				vec3d *src = vp(p+offset);

				for (n = 0; n < nverts; n++) {
					nnorms += normcount[n];
				}

				model_allocate_interp_data(nverts, nnorms);

				// this must happen only after the interp_data allocation call (since the address changes)
				vec3d **verts = Interp_verts;
				vec3d **norms = Interp_norms;

				for (n=0; n<nverts; n++ )	{
					*verts++ = src;
					*norms++ = src + 1;		// first normal associated with the point

					src += normcount[n]+1;
				} 
				return nverts;		// Read in 'n' points
			}
			break;
		case OP_FLATPOLY:		break;
		case OP_TMAPPOLY:		break;
		case OP_SORTNORM:		break;
		case OP_SORTNORM2:		break;
		case OP_BOUNDBOX:		break;
		case OP_TMAP2POLY:		break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in submodel_get_points\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
	}
	return 0;		// Couldn't find 'em
}

/**
 * Gets two random points on a model
 */
void submodel_get_two_random_points(int model_num, int submodel_num, vec3d *v1, vec3d *v2, vec3d *n1, vec3d *n2 )
{
	int nv = submodel_get_points_internal(model_num, submodel_num);

	// this is not only because of the immediate div-0 error but also because of the less immediate expectation for at least one point (preferably two) to be found
	if (nv <= 0) {
		polymodel *pm = model_get(model_num);
		Error(LOCATION, "Model %d ('%s') must have at least one point from submodel_get_points_internal!", model_num, (pm == NULL) ? "<null model?!?>" : pm->filename);

		// in case people ignore the error...
		vm_vec_zero(v1);
		vm_vec_zero(v2);
		if (n1 != NULL) {
			vm_vec_zero(n1);
		}
		if (n2 != NULL) {
			vm_vec_zero(n2);
		}
		return;
	}

	int vn1 = Random::next(nv);
	int vn2 = Random::next(nv);

	*v1 = *Interp_verts[vn1];
	*v2 = *Interp_verts[vn2];

	if(n1 != NULL){
		*n1 = *Interp_norms[vn1];
	}
	if(n2 != NULL){
		*n2 = *Interp_norms[vn2];
	}
}

void submodel_get_two_random_points_better(int model_num, int submodel_num, vec3d *v1, vec3d *v2, int seed)
{
	polymodel *pm = model_get(model_num);

	if (pm != NULL) {
		if ( submodel_num < 0 )	{
			submodel_num = pm->detail[0];
		}

		// the Shivan Comm Node does not have a collision tree, for one
		if (pm->submodel[submodel_num].collision_tree_index < 0) {
			nprintf(("Model", "In submodel_get_two_random_points_better(), model %s does not have a collision tree!  Falling back to submodel_get_two_random_points().\n", pm->filename));

			submodel_get_two_random_points(model_num, submodel_num, v1, v2);
			return;
		}

		bsp_collision_tree *tree = model_get_bsp_collision_tree(pm->submodel[submodel_num].collision_tree_index);

		int nv = tree->n_verts;

		// this is not only because of the immediate div-0 error but also because of the less immediate expectation for at least one point (preferably two) to be found
		if (nv <= 0) {
			Error(LOCATION, "Model %d ('%s') must have at least one point from submodel_get_points_internal!", model_num, (pm == NULL) ? "<null model?!?>" : pm->filename);

			// in case people ignore the error...
			vm_vec_zero(v1);
			vm_vec_zero(v2);

			return;
		}

		int seed_num = seed == -1 ? Random::next() : seed;
		int vn1 = static_rand(seed_num) % nv;
		int vn2 = static_rand(seed_num) % nv;

		*v1 = tree->point_list[vn1];
		*v2 = tree->point_list[vn2];
	}
}

void submodel_get_cross_sectional_avg_pos(int model_num, int submodel_num, float z_slice_pos, vec3d* pos)
{
	polymodel* pm = model_get(model_num);

	if (pm != nullptr) {
		if (submodel_num < 0) {
			submodel_num = pm->detail[0];
		}

		// the Shivan Comm Node does not have a collision tree, for one
		if (pm->submodel[submodel_num].collision_tree_index < 0) {
			nprintf(("Model", "In submodel_get_cross_sectional_avg_pos(), model %s does not have a collision tree!\n", pm->filename));
			return;
		}

		bsp_collision_tree* tree = model_get_bsp_collision_tree(pm->submodel[submodel_num].collision_tree_index);

		int nv = tree->n_verts;

		// this is not only because of the immediate div-0 error but also because of the less immediate expectation for at least one point (preferably two) to be found
		if (nv <= 0) {
			Error(LOCATION, "Model %d ('%s') must have at least one point from submodel_get_cross_sectional_avg_pos!", model_num, (pm == nullptr) ? "<null model?!?>" : pm->filename);

			// in case people ignore the error...
			vm_vec_zero(pos);
			return;
		}

		vm_vec_zero(pos);
		// we take a regular average, add them all up, divide by the total number, but weighted by how close they are to the z slice
		float accum_scale_factor = 0.0f;
		for (int i = 0; i < tree->n_verts; i++) {
			// this goes from 1 directly at our z pos, and quickly goes to 0 the further it gets
			float scale_factor = 1 / ((fabs(tree->point_list[i].xyz.z - z_slice_pos) / (pm->rad / 10)) + 1);
			vm_vec_scale_add(pos, pos, &tree->point_list[i], scale_factor);
			// keep track of the scale factor we use because we need to divide by its total at the end
			accum_scale_factor += scale_factor;
		}

		*pos /= accum_scale_factor;
	}
}

void submodel_get_cross_sectional_random_pos(int model_num, int submodel_num, float z_slice_pos, vec3d* pos)
{
	polymodel* pm = model_get(model_num);

	if (pm != nullptr) {
		if (submodel_num < 0) {
			submodel_num = pm->detail[0];
		}

		// the Shivan Comm Node does not have a collision tree, for one
		if (pm->submodel[submodel_num].collision_tree_index < 0) {
			nprintf(("Model", "In submodel_get_cross_sectional_random_pos(), model %s does not have a collision tree!\n", pm->filename));
			return;
		}

		bsp_collision_tree* tree = model_get_bsp_collision_tree(pm->submodel[submodel_num].collision_tree_index);

		int nv = tree->n_verts;

		// this is not only because of the immediate div-0 error but also because of the less immediate expectation for at least one point (preferably two) to be found
		if (nv <= 0) {
			Error(LOCATION, "Model %d ('%s') must have at least one point from submodel_get_cross_sectional_random_pos!", model_num, (pm == nullptr) ? "<null model?!?>" : pm->filename);

			// in case people ignore the error...
			vm_vec_zero(pos);
			return;
		}

		vm_vec_zero(pos);
		vec3d best1, best2;
		// make random guesses a bunch of times, and average our two best guesses closest to the z pos, ez
		// there are more accurate ways, but this is reasonably good and super cheap
		vm_vec_make(&best1, 0, 0, 999999);
		vm_vec_make(&best2, 0, 0, 999999);
		for (int i = 0; i < 15; i++) {
			vec3d rand_point = tree->point_list[Random::next(nv)];
			if (fabs(rand_point.xyz.z - z_slice_pos) < fabs(best1.xyz.z - z_slice_pos))
				best1 = rand_point;
			else if (fabs(rand_point.xyz.z - z_slice_pos) < fabs(best2.xyz.z - z_slice_pos))
				best2 = rand_point;
		}

		vm_vec_avg(pos, &best1, &best2);
	}
}

// If MR_FLAG_OUTLINE bit set this color will be used for outlines.
// This defaults to black.
void model_set_outline_color(int r, int g, int b )
{
	gr_init_color( &Interp_outline_color, r, g, b );

}

// IF MR_LOCK_DETAIL is set, then it will always draw detail level 'n'
// This defaults to 0. (0=highest, larger=lower)
void model_set_detail_level(int n)
{
	Interp_detail_level_locked = n;
}

/**
 * Returns number of tmaps & flat polys in a submodel;
 */
int submodel_get_num_polys_sub( ubyte *p )
{
	int chunk_type = w(p);
	int chunk_size = w(p+4);
	int n = 0;
	
	bool end = chunk_type == OP_EOF;
	while (!end)	{
		switch (chunk_type) {
		case OP_DEFPOINTS:	break;
		case OP_FLATPOLY:		n++; break;
		case OP_TMAPPOLY:		n++; break;
		case OP_SORTNORM:		{
			int frontlist = w(p+36);
			int backlist = w(p+40);
			int prelist = w(p+44);
			int postlist = w(p+48);
			int onlist = w(p+52);
			n += submodel_get_num_polys_sub(p+frontlist);
			n += submodel_get_num_polys_sub(p+backlist);
			n += submodel_get_num_polys_sub(p+prelist);
			n += submodel_get_num_polys_sub(p+postlist );
			n += submodel_get_num_polys_sub(p+onlist );
			}
			break;

		case OP_SORTNORM2: {
			int frontlist = w(p + 8);
			int backlist = w(p + 12);
			n += submodel_get_num_polys_sub(p + frontlist);
			n += submodel_get_num_polys_sub(p + backlist);
			}
			end = true; // should not continue after this chunk
			break;
		case OP_BOUNDBOX:	break;
		case OP_TMAP2POLY:		
			n++; 
			end = true; // should not continue after this chunk
			break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in submodel_get_num_polys\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);

		if (chunk_type == OP_EOF)
			end = true;
	}
	return n;		
}

/**
 * Returns number of tmaps & flat polys in a submodel
 */
int submodel_get_num_polys(int model_num, int submodel_num )
{
	polymodel * pm;

	pm = model_get(model_num);

	return submodel_get_num_polys_sub( pm->submodel[submodel_num].bsp_data );
}

/**
 * See if the given texture is used by the passed model. 0 if not used, 1 if used, -1 on error
 */
int model_find_texture(int model_num, int bitmap)
{
	polymodel * pm;	
	int idx;

	// get a handle to the model
	pm = model_get(model_num);
	if(pm == NULL){
		return -1;
	}

	// find the texture
	for(idx=0; idx<pm->n_textures; idx++)
	{
		if(pm->maps[idx].FindTexture(bitmap) > -1)
		{
			return 1;
		}
	}

	// no texture
	return 0;
}

// find closest point on extended bounding box (the bounding box plus all the planes that make it up)
// returns closest distance to extended box
// positive return value means start_point is outside extended box
// displaces closest point an optional amount delta to the outside of the box
// closest_box_point can be NULL.
float get_model_closest_box_point_with_delta(vec3d *closest_box_point, vec3d *start_point, int modelnum, int *is_inside, float delta)
{
	int i, idx;
	vec3d box_point, ray_direction, *extremes;
	float dist, best_dist;
	polymodel *pm;
	int inside = 0;
	int masks[6] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
	int mask_inside = 0x3f;

	best_dist = FLT_MAX;
	pm = model_get(modelnum);

	for (i=0; i<6; i++) {
		idx = i / 2;	// which row vector of Identity matrix

		memcpy(&ray_direction, vmd_identity_matrix.a2d[idx], sizeof(vec3d));

		// do negative, then positive plane for each axis
		if (2 * idx == i) {
			extremes = &pm->mins;
			vm_vec_negate(&ray_direction);
		} else {
			extremes = &pm->maxs;
		}

		// a negative distance means started outside the box
		dist = fvi_ray_plane(&box_point, extremes, &ray_direction, start_point, &ray_direction, 0.0f);
		if (dist > 0) {
			inside |= masks[i];
		}
		if (fabs(dist) < fabs(best_dist)) {
			best_dist = dist;
			if (closest_box_point) {
				vm_vec_scale_add(closest_box_point, &box_point, &ray_direction, delta);
			}
		}
	}

	// is start_point inside the box
	if (is_inside) {
		*is_inside = (inside == mask_inside);
	}

	return -best_dist;
}

// find closest point on extended bounding box (the bounding box plus all the planes that make it up)
// returns closest distance to extended box
// positive return value means start_point is outside extended box
// displaces closest point an optional amount delta to the outside of the box
// closest_box_point can be NULL.
float get_world_closest_box_point_with_delta(vec3d *closest_box_point, object *box_obj, vec3d *start_point, int *is_inside, float delta)
{
	vec3d temp, box_start;
	float dist;
	int modelnum;

	// get modelnum
	modelnum = Ship_info[Ships[box_obj->instance].ship_info_index].model_num;

	// rotate start_point to box_obj RF
	vm_vec_sub(&temp, start_point, &box_obj->pos);
	vm_vec_rotate(&box_start, &temp, &box_obj->orient);

	dist = get_model_closest_box_point_with_delta(closest_box_point, &box_start, modelnum, is_inside, delta);

	// rotate closest_box_point to world RF
	if (closest_box_point) {
		vm_vec_unrotate(&temp, closest_box_point, &box_obj->orient);
		vm_vec_add(closest_box_point, &temp, &box_obj->pos);
	}

	return dist;
}

/**
 * Given a newly loaded model, page in all textures
 */
void model_page_in_textures(int modelnum, int ship_info_index)
{
	int i, idx;
	polymodel *pm = model_get(modelnum);

	// bogus
	if (pm == NULL)
		return;

	for (idx = 0; idx < pm->n_textures; idx++) {
		pm->maps[idx].PageIn();
	}

	for (i = 0; i < pm->n_glow_point_banks; i++) {
		glow_point_bank *bank = &pm->glow_point_banks[i];

		bm_page_in_texture(bank->glow_bitmap);
		bm_page_in_texture(bank->glow_neb_bitmap);
	}

	if (ship_info_index >= 0)
		ship_page_in_textures(ship_info_index);
}

// unload all textures for a given model
// "release" should only be set if called from model_unload()!!!
void model_page_out_textures(int model_num, bool release)
{
	int i, j;

	if (model_num < 0)
		return;

	polymodel *pm = model_get(model_num);

	if (pm == NULL)
		return;

	if (release && (pm->used_this_mission > 0))
		return;


	for (i = 0; i < pm->n_textures; i++) {
		pm->maps[i].PageOut(release);
	}

	// NOTE: "release" doesn't work here for some, as of yet unknown, reason - taylor
	for (j = 0; j < pm->n_glow_point_banks; j++) {
		glow_point_bank *bank = &pm->glow_point_banks[j];

		if (bank->glow_bitmap >= 0) {
		//	if (release) {
		//		bm_release(bank->glow_bitmap);
		//		bank->glow_bitmap = -1;
		//	} else {
				bm_unload(bank->glow_bitmap);
		//	}
		}

		if (bank->glow_neb_bitmap >= 0) {
		//	if (release) {
		//		bm_release(bank->glow_neb_bitmap);
		//		bank->glow_neb_bitmap = -1;
		//	} else {
				bm_unload(bank->glow_neb_bitmap);
		//	}
		}
	}
}


//**********vertex buffer stuff**********//
int tri_count[MAX_MODEL_TEXTURES];
poly_list polygon_list[MAX_MODEL_TEXTURES];

void parse_defpoint(int off, ubyte *bsp_data)
{
	uint i, n;
	uint nverts = uw(off+bsp_data+8);	
	uint offset = uw(off+bsp_data+16);
	uint next_norm = 0;

	ubyte *normcount = off+bsp_data+20;
	vec3d *src = vp(off+bsp_data+offset);

	// Get pointer to lights
	Interp_lights = off+bsp_data+20+nverts;

#ifndef NDEBUG
	modelstats_num_verts += nverts;
#endif

	for (n = 0; n < nverts; n++) {
		Interp_verts[n] = src;
		src++; // move to normal

		for (i = 0; i < normcount[n]; i++) {
			Interp_norms[next_norm] = src;

			next_norm++;
			src++;
		}
	}
}

int check_values(vec3d *N)
{
	// Values equal to -1.#IND0
	if(!is_valid_vec(N))
	{
		N->xyz.x = 1.0f;
		N->xyz.y = 0.0f;
		N->xyz.z = 0.0f;
		return 1;
	}

	return 0;
}

// Unpacks verts from TMAPPOLY since it's no longer direct memory mapped with model_tmap_vert
void unpack_tmap_verts(const ubyte* vs, model_tmap_vert* verts, uint n_vert) {
	// Copy the verts manually since they aren't aligned with the struct
	for (uint i = 0; i < n_vert; i++) {
		verts[i].vertnum = us(vs);
		verts[i].normnum = us(vs + 2);
		verts[i].u = fl(vs + 4);
		verts[i].v = fl(vs + 8);
		vs += 12;
	}
}

int Parse_normal_problem_count = 0;

void parse_tmap(int offset, ubyte *bsp_data)
{
	int pof_tex = w(bsp_data+offset+40);
	uint n_vert = uw(bsp_data+offset+36);
	
	ubyte *p = &bsp_data[offset+8];
	auto tverts = new model_tmap_vert[n_vert];

	// Copy the verts manually since they aren't aligned with the struct
	unpack_tmap_verts(&bsp_data[offset + 44], tverts, n_vert);

	vertex *V;
	vec3d *v;
	vec3d *N;

	int problem_count = 0;

	for (uint i = 1; i < (n_vert-1); i++) {
		V = &polygon_list[pof_tex].vert[(polygon_list[pof_tex].n_verts)];
		N = &polygon_list[pof_tex].norm[(polygon_list[pof_tex].n_verts)];
		v = Interp_verts[tverts[0].vertnum];
		V->world.xyz.x = v->xyz.x;
		V->world.xyz.y = v->xyz.y;
		V->world.xyz.z = v->xyz.z;
		V->texture_position.u = tverts[0].u;
		V->texture_position.v = tverts[0].v;

		*N = *Interp_norms[(int)tverts[0].normnum];

		if ( IS_VEC_NULL(N) )
			*N = *vp(p);

	  	problem_count += check_values(N);
		vm_vec_normalize_safe(N);

		V = &polygon_list[pof_tex].vert[(polygon_list[pof_tex].n_verts)+1];
		N = &polygon_list[pof_tex].norm[(polygon_list[pof_tex].n_verts)+1];
		v = Interp_verts[tverts[i].vertnum];
		V->world.xyz.x = v->xyz.x;
		V->world.xyz.y = v->xyz.y;
		V->world.xyz.z = v->xyz.z;
		V->texture_position.u = tverts[i].u;
		V->texture_position.v = tverts[i].v;

		*N = *Interp_norms[(int)tverts[i].normnum];

		if ( IS_VEC_NULL(N) )
			*N = *vp(p);

	 	problem_count += check_values(N);
		vm_vec_normalize_safe(N);

		V = &polygon_list[pof_tex].vert[(polygon_list[pof_tex].n_verts)+2];
		N = &polygon_list[pof_tex].norm[(polygon_list[pof_tex].n_verts)+2];
		v = Interp_verts[tverts[i+1].vertnum];
		V->world.xyz.x = v->xyz.x;
		V->world.xyz.y = v->xyz.y;
		V->world.xyz.z = v->xyz.z;
		V->texture_position.u = tverts[i+1].u;
		V->texture_position.v = tverts[i+1].v;

		*N = *Interp_norms[tverts[i+1].normnum];

		if ( IS_VEC_NULL(N) )
			*N = *vp(p);

		problem_count += check_values(N);
		vm_vec_normalize_safe(N);

		polygon_list[pof_tex].n_verts += 3;
	}

	Parse_normal_problem_count += problem_count;
}

/**
* @brief Parses a TMAP2POLY chunk into a list of polygons.
* 
* @param offset The byte offset to the current TMAP2POLY chunk within bsp_data.
* @param[in] bsp_data The byte buffer containing the BSP information for the current model.
*/
void parse_tmap2(int offset, ubyte* bsp_data)
{
	int pof_tex = w(bsp_data + offset + 44);
	uint n_vert = uw(bsp_data + offset + 48);

	ubyte* p = &bsp_data[offset + 32];
	model_tmap_vert* tverts;

	vertex* V;
	vec3d* v;
	vec3d* N;

	int problem_count = 0;

	tverts = (model_tmap_vert*)&bsp_data[offset + 52];

	for (uint i = 1; i < (n_vert - 1); i++) {
		V = &polygon_list[pof_tex].vert[(polygon_list[pof_tex].n_verts)];
		N = &polygon_list[pof_tex].norm[(polygon_list[pof_tex].n_verts)];
		v = Interp_verts[tverts[0].vertnum];
		V->world.xyz.x = v->xyz.x;
		V->world.xyz.y = v->xyz.y;
		V->world.xyz.z = v->xyz.z;
		V->texture_position.u = tverts[0].u;
		V->texture_position.v = tverts[0].v;

		*N = *Interp_norms[tverts[0].normnum];

		if (IS_VEC_NULL(N))
			*N = *vp(p);

		problem_count += check_values(N);
		vm_vec_normalize_safe(N);

		V = &polygon_list[pof_tex].vert[(polygon_list[pof_tex].n_verts) + 1];
		N = &polygon_list[pof_tex].norm[(polygon_list[pof_tex].n_verts) + 1];
		v = Interp_verts[tverts[i].vertnum];
		V->world.xyz.x = v->xyz.x;
		V->world.xyz.y = v->xyz.y;
		V->world.xyz.z = v->xyz.z;
		V->texture_position.u = tverts[i].u;
		V->texture_position.v = tverts[i].v;

		*N = *Interp_norms[tverts[i].normnum];

		if (IS_VEC_NULL(N))
			*N = *vp(p);

		problem_count += check_values(N);
		vm_vec_normalize_safe(N);

		V = &polygon_list[pof_tex].vert[(polygon_list[pof_tex].n_verts) + 2];
		N = &polygon_list[pof_tex].norm[(polygon_list[pof_tex].n_verts) + 2];
		v = Interp_verts[tverts[i + 1].vertnum];
		V->world.xyz.x = v->xyz.x;
		V->world.xyz.y = v->xyz.y;
		V->world.xyz.z = v->xyz.z;
		V->texture_position.u = tverts[i + 1].u;
		V->texture_position.v = tverts[i + 1].v;

		*N = *Interp_norms[tverts[i + 1].normnum];

		if (IS_VEC_NULL(N))
			*N = *vp(p);

		problem_count += check_values(N);
		vm_vec_normalize_safe(N);

		polygon_list[pof_tex].n_verts += 3;
	}

	Parse_normal_problem_count += problem_count;
}

void parse_bsp(int offset, ubyte* bsp_data);

void parse_sortnorm(int offset, ubyte* bsp_data)
{
	int frontlist, backlist, prelist, postlist, onlist;

	frontlist = w(bsp_data + offset + 36);
	backlist = w(bsp_data + offset + 40);
	prelist = w(bsp_data + offset + 44);
	postlist = w(bsp_data + offset + 48);
	onlist = w(bsp_data + offset + 52);

	if (prelist) parse_bsp(offset + prelist, bsp_data);
	if (backlist) parse_bsp(offset + backlist, bsp_data);
	if (onlist) parse_bsp(offset + onlist, bsp_data);
	if (frontlist) parse_bsp(offset + frontlist, bsp_data);
	if (postlist) parse_bsp(offset + postlist, bsp_data);
}

/**
* @brief Parses a SORTNORM2 by recursively parsing into the two pointers it contains.
*
* @param offset The byte offset to the current SORT2NORM chunk within bsp_data.
* @param bsp_data The byte buffer containing the BSP information for the current model.
*/
void parse_sortnorm2(int offset, ubyte* bsp_data)
{
	int frontlist, backlist;

	frontlist = w(bsp_data + offset + 8);
	backlist = w(bsp_data + offset + 12);

	if (backlist) parse_bsp(offset + backlist, bsp_data);
	if (frontlist) parse_bsp(offset + frontlist, bsp_data);
}

void parse_bsp(int offset, ubyte *bsp_data)
{
	int id = w(bsp_data+offset);
	int size = w(bsp_data+offset+4);

	bool end = id == OP_EOF;
	while (!end) {
		switch (id)
		{
			case OP_DEFPOINTS:
				parse_defpoint(offset, bsp_data);
				break;

			case OP_SORTNORM:
				parse_sortnorm(offset, bsp_data);
				break;

			case OP_SORTNORM2:
				parse_sortnorm2(offset, bsp_data);
				end = true; // should not continue after this chunk
				break;

			case OP_FLATPOLY:
				break;

			case OP_TMAPPOLY:
				parse_tmap(offset, bsp_data);
				break;

			case OP_BOUNDBOX:
				break;

			case OP_TMAP2POLY:
				parse_tmap2(offset, bsp_data);
				end = true; // should not continue after this chunk
				break;

			default:
				return;
		}

		offset += size;
		id = w(bsp_data+offset);
		size = w(bsp_data+offset+4);

		if (size < 1 || id == OP_EOF)
			end = true;
	}
}

void find_tmap(int offset, const ubyte *bsp_data, int id)
{
	int pof_tex = w(bsp_data+offset+(id == OP_TMAP2POLY ? 44 : 40));
	uint n_vert = uw(bsp_data+offset+ (id == OP_TMAP2POLY ? 48 : 36));

	tri_count[pof_tex] += n_vert-2;	
}

void find_defpoint(int off, ubyte *bsp_data)
{
	uint n;
	uint nverts = uw(off+bsp_data+8);	

	ubyte * normcount = off+bsp_data+20;

	// Get pointer to lights
	Interp_lights = off+bsp_data+20+nverts;

#ifndef NDEBUG
	modelstats_num_verts += nverts;
#endif

	int norm_num = 0;

	for (n = 0; n < nverts; n++) {	
		norm_num += normcount[n];
	}

	Interp_num_verts = nverts;
	Interp_num_norms = norm_num;
}

void find_tri_counts(int offset, ubyte* bsp_data);

void find_sortnorm(int offset, ubyte* bsp_data)
{
	int frontlist, backlist, prelist, postlist, onlist;

	frontlist = w(bsp_data + offset + 36);
	backlist = w(bsp_data + offset + 40);
	prelist = w(bsp_data + offset + 44);
	postlist = w(bsp_data + offset + 48);
	onlist = w(bsp_data + offset + 52);

	if (prelist) find_tri_counts(offset + prelist, bsp_data);
	if (backlist) find_tri_counts(offset + backlist, bsp_data);
	if (onlist) find_tri_counts(offset + onlist, bsp_data);
	if (frontlist) find_tri_counts(offset + frontlist, bsp_data);
	if (postlist) find_tri_counts(offset + postlist, bsp_data);
}

void find_sortnorm2(int offset, ubyte* bsp_data)
{
	int frontlist, backlist;

	frontlist = w(bsp_data + offset + 8);
	backlist = w(bsp_data + offset + 12);

	if (backlist) find_tri_counts(offset + backlist, bsp_data);
	if (frontlist) find_tri_counts(offset + frontlist, bsp_data);
}

// tri_count
void find_tri_counts(int offset, ubyte *bsp_data)
{
	int id = w(bsp_data+offset);
	int size = w(bsp_data+offset+4);

	bool end = id == OP_EOF;
	while (!end) {
		switch (id)
		{
			case OP_DEFPOINTS:
				find_defpoint(offset, bsp_data);
				break;

			case OP_SORTNORM:
				find_sortnorm(offset, bsp_data);
				break;

			case OP_SORTNORM2:
				find_sortnorm2(offset, bsp_data);
				end = true; // should not continue after this chunk
				break;

			case OP_FLATPOLY:
				break;

			case OP_TMAPPOLY:
				find_tmap(offset, bsp_data, id);
				break;
			case OP_TMAP2POLY:
				find_tmap(offset, bsp_data, id);
				end = true; // should not continue after this chunk
				break;

			case OP_BOUNDBOX:
				break;

			default:
				return;
		}

		offset += size;
		id = w(bsp_data+offset);
		size = w(bsp_data+offset+4);

		if (size < 1 || id == OP_EOF)
			end = true;
	}
}

void model_interp_submit_buffers(indexed_vertex_source *vert_src, size_t vertex_stride)
{
	Assert(vert_src != NULL);

	if ( !(vert_src->Vertex_list_size > 0 && vert_src->Index_list_size > 0 ) ) {
		return;
	}

	if ( vert_src->Vertex_list != NULL ) {
		size_t offset;
		gr_heap_allocate(GpuHeap::ModelVertex, vert_src->Vertex_list_size, vert_src->Vertex_list, offset, vert_src->Vbuffer_handle);

		// If this happens then someone must have allocated something from the heap with a different stride than what we
		// are using.
		Assertion(offset % vertex_stride == 0, "Offset returned by GPU heap allocation does not match stride value!");
		vert_src->Base_vertex_offset = offset / vertex_stride;
		vert_src->Vertex_offset = offset;

		vm_free(vert_src->Vertex_list);
		vert_src->Vertex_list = NULL;
	}

	if ( vert_src->Index_list != NULL ) {
		gr_heap_allocate(GpuHeap::ModelIndex, vert_src->Index_list_size, vert_src->Index_list, vert_src->Index_offset, vert_src->Ibuffer_handle);

		vm_free(vert_src->Index_list);
		vert_src->Index_list = NULL;
	}
}

bool model_interp_pack_buffer(indexed_vertex_source *vert_src, vertex_buffer *vb)
{
	if ( vert_src == NULL ) {
		return false;
	}

	Assertion(vb != nullptr, "Invalid vertex buffer specified!");

	int i, n_verts = 0;
	size_t j;
	if ( vert_src->Vertex_list == NULL ) {
		vert_src->Vertex_list = vm_malloc(vert_src->Vertex_list_size);

		// return invalid if we don't have the memory
		if ( vert_src->Vertex_list == NULL ) {
			return false;
		}

		memset(vert_src->Vertex_list, 0, vert_src->Vertex_list_size);
	}

	if ( vert_src->Index_list == NULL ) {
		vert_src->Index_list = vm_malloc(vert_src->Index_list_size);

		// return invalid if we don't have the memory
		if ( vert_src->Index_list == NULL ) {
			return false;
		}

		memset(vert_src->Index_list, 0, vert_src->Index_list_size);
	}

	// bump to our index in the array
	auto array = reinterpret_cast<interp_vertex*>(static_cast<uint8_t*>(vert_src->Vertex_list) + (vb->vertex_offset));

	// generate the vertex array
	n_verts = vb->model_list->n_verts;
	for ( i = 0; i < n_verts; i++ ) {
		vertex *vl = &vb->model_list->vert[i];
		auto outVert = &array[i];

		// don't try to generate more data than what's available
		Assert(((i * sizeof(interp_vertex)) + sizeof(interp_vertex)) <= (vert_src->Vertex_list_size - vb->vertex_offset));

		// NOTE: UV->NORM->TSB->MODEL_ID->VERT, This array order *must* be preserved!!

		// tex coords
		if ( vb->flags & VB_FLAG_UV1 ) {
			outVert->uv = vl->texture_position;
		} else {
			outVert->uv.u = 1.0f;
			outVert->uv.v = 1.0f;
		}

		// normals
		if ( vb->flags & VB_FLAG_NORMAL ) {
			Assert(vb->model_list->norm != NULL);
			outVert->normal = vb->model_list->norm[i];
		} else {
			outVert->normal.xyz.x = 0.0f;
			outVert->normal.xyz.y = 0.0f;
			outVert->normal.xyz.z = 1.0f;
		}

		// tangent space data
		if ( vb->flags & VB_FLAG_TANGENT ) {
			Assert(vb->model_list->tsb != NULL);
			tsb_t *tsb = &vb->model_list->tsb[i];

			outVert->tangent.xyzw.x = tsb->tangent.xyz.x;
			outVert->tangent.xyzw.y = tsb->tangent.xyz.y;
			outVert->tangent.xyzw.z = tsb->tangent.xyz.z;
			outVert->tangent.xyzw.w = tsb->scaler;
		} else {
			outVert->tangent.xyzw.x = 1.0f;
			outVert->tangent.xyzw.y = 0.0f;
			outVert->tangent.xyzw.z = 0.0f;
			outVert->tangent.xyzw.w = 0.0f;
		}

		if ( vb->flags & VB_FLAG_MODEL_ID ) {
			Assert(vb->model_list->submodels != NULL);
			outVert->modelId = (float)vb->model_list->submodels[i];
		} else {
			outVert->modelId = 0.0f;
		}

		// verts
		outVert->pos = vl->world;
	}

	// generate the index array
	for ( j = 0; j < vb->tex_buf.size(); j++ ) {
		buffer_data* tex_buf = &vb->tex_buf[j];
		n_verts = (int)tex_buf->n_verts;
		auto offset = tex_buf->index_offset;
		const uint *index = tex_buf->get_index();

		// bump to our spot in the buffer
		auto ibuf = static_cast<uint8_t*>(vert_src->Index_list) + offset;

		if ( vb->tex_buf[j].flags & VB_FLAG_LARGE_INDEX ) {
			memcpy(ibuf, index, n_verts * sizeof(uint));
		} else {
			ushort *mybuf = (ushort*)ibuf;

			for ( i = 0; i < n_verts; i++ ) {
				mybuf[i] = (ushort)index[i];
			}
		}
	}

	return true;
}

void interp_pack_vertex_buffers(polymodel *pm, int mn)
{
	Assert( (mn >= 0) && (mn < pm->n_models) );

	bsp_info *model = &pm->submodel[mn];

	if ( !model->buffer.model_list ) {
		return;
	}

	bool rval = model_interp_pack_buffer(&pm->vert_source, &model->buffer);

	if ( model->trans_buffer.flags & VB_FLAG_TRANS && !model->trans_buffer.tex_buf.empty() ) {
		model_interp_pack_buffer(&pm->vert_source, &model->trans_buffer);
	}

	if ( !rval ) {
		Error( LOCATION, "Unable to pack vertex buffer for '%s'\n", pm->filename );
	}
}

void model_interp_set_buffer_layout(vertex_layout *layout)
{
	Assert(layout != NULL);

	// Similarly to model_interp_config_buffer, we add all vectex components even if they aren't used
	// This reduces the amount of vertex format respecification and since the data contains valid data there is no risk
	// of reading garbage data on the GPU

	layout->add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(interp_vertex), offsetof(interp_vertex, uv));

	layout->add_vertex_component(vertex_format_data::NORMAL, sizeof(interp_vertex), offsetof(interp_vertex, normal));

	layout->add_vertex_component(vertex_format_data::TANGENT, sizeof(interp_vertex), offsetof(interp_vertex, tangent));

	layout->add_vertex_component(vertex_format_data::MODEL_ID, sizeof(interp_vertex), offsetof(interp_vertex, modelId));

	layout->add_vertex_component(vertex_format_data::POSITION3, sizeof(interp_vertex), offsetof(interp_vertex, pos));
}

bool model_interp_config_buffer(indexed_vertex_source *vert_src, vertex_buffer *vb, bool update_ibuffer_only)
{
	if ( vb == NULL ) {
		return false;
	}

	if ( !(vb->flags & VB_FLAG_POSITION) ) {
		Int3();
		return false;
	}

	// pad out the vertex buffer even if it doesn't use certain attributes
	// we require consistent stride across vertex buffers so we can use base vertex offsetting for performance reasons
	vb->stride = sizeof(interp_vertex);

	model_interp_set_buffer_layout(&vb->layout);

	// offsets for this chunk
	if ( !update_ibuffer_only ) {
		vb->vertex_offset = vert_src->Vertex_list_size;
		vb->vertex_num_offset = vb->vertex_offset / vb->stride;
		vert_src->Vertex_list_size += (uint)(vb->stride * vb->model_list->n_verts);
	}

	for ( size_t idx = 0; idx < vb->tex_buf.size(); idx++ ) {
		buffer_data *bd = &vb->tex_buf[idx];

		bd->index_offset = vert_src->Index_list_size;
		vert_src->Index_list_size += (uint)(bd->n_verts * ((bd->flags & VB_FLAG_LARGE_INDEX) ? sizeof(uint) : sizeof(ushort)));

		// even out index buffer so we are always word aligned
		vert_src->Index_list_size += (uint)(vert_src->Index_list_size % sizeof(uint));
	}

	return true;
}

void interp_configure_vertex_buffers(polymodel *pm, int mn)
{
	TRACE_SCOPE(tracing::ModelConfigureVertexBuffers);

	int i, j, first_index;
	uint total_verts = 0;
	SCP_vector<int> vertex_list;

	Assert( (mn >= 0) && (mn < pm->n_models) );

	bsp_info *model = &pm->submodel[mn];

	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		polygon_list[i].n_verts = 0;
		tri_count[i] = 0;
	}

	int milliseconds = timer_get_milliseconds();

	bsp_polygon_data *bsp_polies = new bsp_polygon_data(model->bsp_data);

	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		int vert_count = bsp_polies->get_num_triangles(i) * 3;
		tri_count[i] = vert_count / 3;
		total_verts += vert_count;

		polygon_list[i].allocate(vert_count);

		bsp_polies->generate_triangles(i, polygon_list[i].vert, polygon_list[i].norm);
		polygon_list[i].n_verts = vert_count;

		// set submodel ID
		for ( j = 0; j < polygon_list[i].n_verts; ++j ) {
			polygon_list[i].submodels[j] = mn;
		}

		// for the moment we can only support INT_MAX worth of verts per index buffer
		if (total_verts > INT_MAX) {
			Error( LOCATION, "Unable to generate vertex buffer data because model '%s' with %i verts is over the maximum of %i verts!\n", pm->filename, total_verts, INT_MAX);
		}
	}

	// figure out if we have an outline
	int outline_n_lines = bsp_polies->get_num_lines(-1);

	if ( outline_n_lines > 0 ) {
		model->n_verts_outline = outline_n_lines * 2;
		model->outline_buffer = (vertex*)vm_malloc(sizeof(vertex) * model->n_verts_outline);

		bsp_polies->generate_lines(-1, model->outline_buffer);
	}

	// done with the bsp now that we have the vertex data
	delete bsp_polies;

	int time_elapsed = timer_get_milliseconds() - milliseconds;

	nprintf(("Model", "BSP Parse took %d milliseconds.\n", time_elapsed));

	if (total_verts < 1) {
		return;
	}

	total_verts = 0;

	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		total_verts += polygon_list[i].n_verts;
	}

	poly_list *model_list = new(std::nothrow) poly_list;

	if ( !model_list ) {
		Error( LOCATION, "Unable to allocate memory for poly_list!\n" );
	}

	model->buffer.model_list = model_list;

	model_list->allocate( (int)total_verts );

	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		if ( !polygon_list[i].n_verts )
			continue;

		memcpy( (model_list->vert) + model_list->n_verts, polygon_list[i].vert, sizeof(vertex) * polygon_list[i].n_verts );
		memcpy( (model_list->norm) + model_list->n_verts, polygon_list[i].norm, sizeof(vec3d) * polygon_list[i].n_verts );

		if (Cmdline_normal) {
			memcpy( (model_list->tsb) + model_list->n_verts, polygon_list[i].tsb, sizeof(tsb_t) * polygon_list[i].n_verts );
		}

		memcpy( (model_list->submodels) + model_list->n_verts, polygon_list[i].submodels, sizeof(int) * polygon_list[i].n_verts );

		model_list->n_verts += polygon_list[i].n_verts;
	}

	// no read file so we'll have to generate
	model_list->make_index_buffer(vertex_list);

	vertex_list.clear();	// done

	int vertex_flags = (VB_FLAG_POSITION | VB_FLAG_NORMAL | VB_FLAG_UV1);

	if (model_list->tsb != NULL) {
		Assert( Cmdline_normal );
		vertex_flags |= VB_FLAG_TANGENT;
	}

	if ( model_list->submodels != NULL ) {
		vertex_flags |= VB_FLAG_MODEL_ID;
	}

	model->buffer.flags = vertex_flags;

	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		if ( !polygon_list[i].n_verts )
			continue;

		buffer_data new_buffer(polygon_list[i].n_verts);

		Verify( new_buffer.get_index() != NULL );

		for (j = 0; j < polygon_list[i].n_verts; j++) {
			first_index = model_list->find_index_fast(&polygon_list[i], j);
			Assert(first_index != -1);

			new_buffer.assign(j, first_index);
		}

		new_buffer.texture = i;

		new_buffer.flags = 0;

		if (polygon_list[i].n_verts >= USHRT_MAX) {
			new_buffer.flags |= VB_FLAG_LARGE_INDEX;
		}

		model->buffer.tex_buf.push_back( new_buffer );
	}

	bool rval = model_interp_config_buffer(&pm->vert_source, &model->buffer, false);

	if ( !rval ) {
		Error( LOCATION, "Unable to configure vertex buffer for '%s'\n", pm->filename );
	}
}

void interp_copy_index_buffer(vertex_buffer *src, vertex_buffer *dest, size_t *index_counts)
{
	size_t i, j, k;
	size_t src_buff_size;
	buffer_data *src_buffer;
	buffer_data *dest_buffer;
	size_t vert_offset = src->vertex_num_offset; // assuming all submodels crunched into this index buffer have the same stride
	//int vert_offset = 0;

	for ( i = 0; i < dest->tex_buf.size(); ++i ) {
		dest_buffer = &dest->tex_buf[i];

		for ( j = 0; j < src->tex_buf.size(); ++j ) {
			if ( dest_buffer->texture != src->tex_buf[j].texture ) {
				continue;
			}

			src_buffer = &src->tex_buf[j];
			src_buff_size = (size_t)src_buffer->n_verts;

			for ( k = 0; k < src_buff_size; ++k ) {
				dest_buffer->assign(dest_buffer->n_verts, (uint32_t)(src_buffer->get_index()[k] + vert_offset)); // take into account the vertex offset.
				dest_buffer->n_verts++;

				Assert(dest_buffer->n_verts <= index_counts[dest_buffer->texture]);
			}
		}
	}
}

void interp_fill_detail_index_buffer(SCP_vector<int> &submodel_list, polymodel *pm, vertex_buffer *buffer)
{
	size_t index_counts[MAX_MODEL_TEXTURES];
	int i, j;
	int model_num;

	for ( i = 0; i < MAX_MODEL_TEXTURES; ++i ) {
		index_counts[i] = 0;
	}

	buffer->vertex_offset = 0;
	buffer->vertex_num_offset = 0;
	buffer->model_list = new(std::nothrow) poly_list;

	int num_buffers;
	int tex_num;

	// need to first count how many indexes there are in this entire detail model hierarchy
	for ( i = 0; i < (int)submodel_list.size(); ++i ) {
		model_num = submodel_list[i];

		if ( pm->submodel[model_num].flags[Model::Submodel_flags::Is_thruster] ) {
			continue;
		}

		num_buffers = (int)pm->submodel[model_num].buffer.tex_buf.size();

		buffer->flags |= pm->submodel[model_num].buffer.flags;

		for ( j = 0; j < num_buffers; ++j ) {
			tex_num = pm->submodel[model_num].buffer.tex_buf[j].texture;

			index_counts[tex_num] += pm->submodel[model_num].buffer.tex_buf[j].n_verts;
		}
	}

	// allocate the respective texture buffers with indexes for our detail buffer
	for ( i = 0; i < MAX_MODEL_TEXTURES; ++i ) {
		if ( index_counts[i] == 0 ) {
			continue;
		}

		buffer->tex_buf.push_back(buffer_data(index_counts[i]));

		buffer_data &new_buffer = buffer->tex_buf.back();
		//new_buffer.n_verts = 0;
		new_buffer.texture = i;
	}

	for ( i = 0; i < (int)buffer->tex_buf.size(); ++i ) {
		buffer->tex_buf[i].n_verts = 0;
	}

	// finally copy over the indexes
	for ( i = 0; i < (int)submodel_list.size(); ++i ) {
		model_num = submodel_list[i];

		if (pm->submodel[model_num].flags[Model::Submodel_flags::Is_thruster]) {
			continue;
		}

		interp_copy_index_buffer(&pm->submodel[model_num].buffer, buffer, index_counts);
	}

	// check which buffers need to have the > USHORT flag
	for ( i = 0; i < (int)buffer->tex_buf.size(); ++i ) {
		if ( buffer->tex_buf[i].i_last >= USHRT_MAX ) {
			buffer->tex_buf[i].flags |= VB_FLAG_LARGE_INDEX;
		}
	}
}

void interp_create_detail_index_buffer(polymodel *pm, int detail_num)
{
	TRACE_SCOPE(tracing::ModelCreateDetailIndexBuffers);

	SCP_vector<int> submodel_list;

	submodel_list.clear();

	model_get_submodel_tree_list(submodel_list, pm, pm->detail[detail_num]);

	if ( submodel_list.empty() ) {
		return;
	}

	interp_fill_detail_index_buffer(submodel_list, pm, &pm->detail_buffers[detail_num]);
	
	// check if anything was even put into this buffer
	if ( pm->detail_buffers[detail_num].tex_buf.empty() ) {
		return;
	} 

	model_interp_config_buffer(&pm->vert_source, &pm->detail_buffers[detail_num], true);
}

void interp_create_transparency_index_buffer(polymodel *pm, int mn)
{
	TRACE_SCOPE(tracing::ModelCreateTransparencyIndexBuffer);

	const int NUM_VERTS_PER_TRI = 3;

	bsp_info *sub_model = &pm->submodel[mn];

	vertex_buffer *trans_buffer = &sub_model->trans_buffer;

	trans_buffer->model_list = new(std::nothrow) poly_list;
	trans_buffer->vertex_offset = pm->submodel[mn].buffer.vertex_offset;
	trans_buffer->vertex_num_offset = pm->submodel[mn].buffer.vertex_num_offset;
	trans_buffer->stride = pm->submodel[mn].buffer.stride;
	trans_buffer->flags = pm->submodel[mn].buffer.flags;

	poly_list *model_list = pm->submodel[mn].buffer.model_list;

	// bail out if this buffer is empty
	if ( model_list == NULL || model_list->n_verts < 1 ) {
		return;
	}

	SCP_vector<buffer_data> &tex_buffers = pm->submodel[mn].buffer.tex_buf;
	uint current_tri[NUM_VERTS_PER_TRI];
	bool transparent_tri = false;
	int num_tris = 0;

	for ( int i = 0; i < (int)tex_buffers.size(); ++i ) {
		buffer_data *tex_buf = &tex_buffers[i];

		if ( tex_buf->n_verts < 1 ) {
			continue;
		}

		const uint *indices = tex_buf->get_index();

		texture_map *tmap = &pm->maps[tex_buf->texture];

		// skip if this is already designated to be a transparent pass by the modeller
// 		if ( tmap->is_transparent ) {
// 			continue;
// 		}

		int bitmap_handle = tmap->textures[TM_BASE_TYPE].GetTexture();

		if ( bitmap_handle < 0 || !bm_has_alpha_channel(bitmap_handle) ) {
			continue;
		}

		bitmap_lookup texture_lookup(bitmap_handle);

		if ( !texture_lookup.valid() ) {
			continue;
		}

		SCP_vector<int> transparent_indices;

		transparent_tri = false;
		num_tris = 0;

		for ( size_t j = 0; j < tex_buf->n_verts; ++j ) {
			uint index = indices[j];

			// need the uv coords of the vert at this index
			float u = model_list->vert[index].texture_position.u;
			float v = model_list->vert[index].texture_position.v;

			if ( texture_lookup.get_channel_alpha(u, v) < 0.95f) {
				transparent_tri = true;
			}

			current_tri[num_tris] = index;
			num_tris++;

			if ( num_tris == NUM_VERTS_PER_TRI ) {
				if ( transparent_tri ) {
					// we have a triangle and it's transparent. 
					// shove index into the transparency buffer
					transparent_indices.push_back(current_tri[0]);
					transparent_indices.push_back(current_tri[1]);
					transparent_indices.push_back(current_tri[2]);
				}

				transparent_tri = false;
				num_tris = 0;
			}
		}

		if ( transparent_indices.empty() ) {
			continue;
		}

		pm->flags |= PM_FLAG_TRANS_BUFFER;
		trans_buffer->flags |= VB_FLAG_TRANS;

		trans_buffer->tex_buf.push_back ( buffer_data ( transparent_indices.size() ) );

		buffer_data &new_buff = trans_buffer->tex_buf.back();
		new_buff.texture = tex_buf->texture;

		for ( int j = 0; j < (int)transparent_indices.size(); ++j ) {
			new_buff.assign(j, transparent_indices[j]);
		}
	}

	if ( trans_buffer->flags & VB_FLAG_TRANS ) {
		model_interp_config_buffer(&pm->vert_source, trans_buffer, true);
	}
}

void model_interp_process_shield_mesh(polymodel * pm)
{
	SCP_vector<vec3d> buffer;

	if ( pm->shield.nverts <= 0 ) {
		return;
	}

	int n_verts = 0;
	
	for ( int i = 0; i < pm->shield.ntris; i++ ) {
		shield_tri *tri = &pm->shield.tris[i];

		vec3d a = pm->shield.verts[tri->verts[0]].pos;
		vec3d b = pm->shield.verts[tri->verts[1]].pos;
		vec3d c = pm->shield.verts[tri->verts[2]].pos;

		// recalculate triangle normals to solve some issues regarding triangle collision
		vec3d b_a;
		vec3d c_a;

		vm_vec_sub(&b_a, &b, &a);
		vm_vec_sub(&c_a, &c, &a);
		vm_vec_cross(&tri->norm, &b_a, &c_a);
		vm_vec_normalize_safe(&tri->norm);

		buffer.push_back(a);
		buffer.push_back(tri->norm);

		buffer.push_back(b);
		buffer.push_back(tri->norm);

		buffer.push_back(c);
		buffer.push_back(tri->norm);

		n_verts += 3;
	}
	
	if ( !buffer.empty() ) {
		pm->shield.buffer_id = gr_create_buffer(BufferType::Vertex, BufferUsageHint::Static);
		pm->shield.buffer_n_verts = n_verts;
		gr_update_buffer_data(pm->shield.buffer_id, buffer.size() * sizeof(vec3d), &buffer[0]);

		pm->shield.layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vec3d) * 2, 0);
		pm->shield.layout.add_vertex_component(vertex_format_data::NORMAL, sizeof(vec3d) * 2, sizeof(vec3d));
	} else {
		pm->shield.buffer_id = gr_buffer_handle::invalid();
	}
}

// returns 1 if the thruster should be drawn
//         0 if it shouldn't
int model_should_render_engine_glow(int objnum, int bank_obj)
{
	if ((bank_obj <= -1) || (objnum <= -1))
		return 1;

	object *obj = &Objects[objnum];

	if (obj->type == OBJ_SHIP) {
		ship_subsys *ssp;
		ship *shipp = &Ships[obj->instance];
		ship_info *sip = &Ship_info[shipp->ship_info_index];

		Assert( bank_obj < sip->n_subsystems );

		char subname[MAX_NAME_LEN];
		// shipp->subsystems isn't always valid here so don't use it
		strcpy_s(subname, sip->subsystems[bank_obj].subobj_name);

		ssp = GET_FIRST(&shipp->subsys_list);
		while ( ssp != END_OF_LIST( &shipp->subsys_list ) ) {
			if ( !strcmp(subname, ssp->system_info->subobj_name) ) {
				// this subsystem has 0 or less hits, ie. it's destroyed
				if ( ssp->current_hits <= 0 )
					return 0;

				// see if the subsystem is disrupted, in which case it should be inoperable
				if ( ship_subsys_disrupted(ssp) )
					return 0;

				return 1;
			}
			ssp = GET_NEXT( ssp );
		}

	} else if (obj->type == OBJ_WEAPON) {
		// for weapons, if needed in the future
	}

	// default to render glow
	return 1;
}

// Goober5000
// uses same algorithms as in ship_do_thruster_frame
int model_interp_get_texture(texture_info *tinfo, fix base_frametime)
{
	int texture, frame, num_frames;
	float cur_time, total_time;

	// get texture
	num_frames = tinfo->GetNumFrames();
	texture = tinfo->GetTexture();
	total_time = tinfo->GetTotalTime();

	// maybe animate it
	if (texture >= 0 && num_frames > 1)
	{
		// sanity check total_time first thing
		Assert(total_time > 0.0f);

		cur_time = f2fl((game_get_overall_frametime() - base_frametime) % fl2f(total_time));

		// get animation frame
		frame = fl2i((cur_time * num_frames) / total_time);
		CLAMP(frame, 0, num_frames - 1);

		// advance to the correct frame
		texture += frame;
	}

	// done
	return texture;
}

void model_mix_two_team_colors(team_color* dest, team_color* a, team_color* b, float mix_factor)
{
	dest->base.r = a->base.r * (1.0f - mix_factor) + b->base.r * mix_factor;
	dest->base.g = a->base.g * (1.0f - mix_factor) + b->base.g * mix_factor;
	dest->base.b = a->base.b * (1.0f - mix_factor) + b->base.b * mix_factor;

	dest->stripe.r = a->stripe.r * (1.0f - mix_factor) + b->stripe.r * mix_factor;
	dest->stripe.g = a->stripe.g * (1.0f - mix_factor) + b->stripe.g * mix_factor;
	dest->stripe.b = a->stripe.b * (1.0f - mix_factor) + b->stripe.b * mix_factor;
}

bool model_get_team_color( team_color *clr, const SCP_string &team, const SCP_string &secondaryteam, fix timestamp, int fadetime )
{
	Assert(clr != NULL);

	if ( !stricmp(secondaryteam.c_str(), "none") ) {
		if (Team_Colors.find(team) != Team_Colors.end()) {
			*clr = Team_Colors[team];
			return true;
		} else
			return false;
	} else {
		if ( Team_Colors.find(secondaryteam) != Team_Colors.end()) {
			team_color temp_color;
			team_color start;

			if (Team_Colors.find(team) != Team_Colors.end()) {
				start = Team_Colors[team];
			} else {
				start.base.r = 0.0f;
				start.base.g = 0.0f;
				start.base.b = 0.0f;

				start.stripe.r = 0.0f;
				start.stripe.g = 0.0f;
				start.stripe.b = 0.0f;
			}

			team_color end = Team_Colors[secondaryteam];
			float time_remaining = 0.0f;
			if (fadetime != 0) // avoid potential div-by-zero
				time_remaining = (f2fl(Missiontime - timestamp) * 1000)/fadetime;
			CLAMP(time_remaining, 0.0f, 1.0f);
			model_mix_two_team_colors(&temp_color, &start, &end, time_remaining);

			*clr = temp_color;
			return true;
		} else
			return false;
	}
}

//********************-----CLASS: texture_info-----********************//
texture_info::texture_info()
{
	clear();
}
texture_info::texture_info(int bm_handle)
{
	if(!bm_is_valid(bm_handle))
	{
		clear();
		return;
	}

	this->original_texture = bm_handle;
	this->ResetTexture();
}
void texture_info::clear()
{
	texture = original_texture = -1;
	num_frames = 0;
	total_time = 1.0f;
}
int texture_info::GetNumFrames()
{
	return num_frames;
}
int texture_info::GetOriginalTexture()
{
	return original_texture;
}
int texture_info::GetTexture()
{
	return texture;
}
float texture_info::GetTotalTime()
{
	return total_time;
}
int texture_info::LoadTexture(const char *filename, const char *dbg_name)
{
	if (strlen(filename) + 4 >= NAME_LENGTH) //Filenames are passed in without extension
	{
		mprintf(("Generated texture name %s is too long. Skipping...\n", filename));
		return -1;
	}
	this->original_texture = bm_load_either(filename, NULL, NULL, NULL, true, CF_TYPE_MAPS);
	if(this->original_texture < 0)
		nprintf(("Maps", "For \"%s\" I couldn't find %s.ani\n", dbg_name, filename));
	this->ResetTexture();

	return texture;
}
void texture_info::PageIn()
{
	bm_page_in_texture(texture);
}

void texture_info::PageOut(bool release)
{
	if (texture >= 0) {
		if (release) {
			bm_release(texture);
			texture = -1;
			num_frames = 0;
			total_time = 1.0f;
		} else {
			bm_unload(texture);
		}
	}
}
int texture_info::ResetTexture()
{
	return this->SetTexture(original_texture);
}
int texture_info::SetTexture(int n_tex)
{
	if(n_tex != -1 && !bm_is_valid(n_tex))
		return texture;

	//Set the new texture
	texture = n_tex;

	//If it is intentionally invalid, blank everything else
	if(n_tex == -1)
	{
		num_frames = 0;
		total_time = 1.0f;
	}
	else
	{
		//Determine the num_frames and total_time values.
		int fps = 0;
		this->num_frames = 1;

		bm_get_info(texture, NULL, NULL, NULL, &this->num_frames, &fps);

		this->total_time = (num_frames / ((fps > 0) ? (float)fps : 1.0f));
	}

	return texture;
}

//********************-----CLASS: texture_map-----********************//
int texture_map::FindTexture(int bm_handle)
{
	if(!bm_is_valid(bm_handle))
		return -1;

	for(int i = 0; i < TM_NUM_TYPES; i++)
	{
		if (this->textures[i].GetTexture() == bm_handle)
			return i;
	}
	return -1;
}
int texture_map::FindTexture(const char* fname)
{
	if(fname == NULL || !strlen(fname))
		return -1;

	char buf[NAME_LENGTH];
	for(int i = 0; i < TM_NUM_TYPES; i++)
	{
		bm_get_filename(this->textures[i].GetTexture(), buf);
		if (!strextcmp(buf, fname)) {
			return i;
		}
	}
	return -1;
}

void texture_map::PageIn()
{
	for(int i = 0; i < TM_NUM_TYPES; i++)
		this->textures[i].PageIn();
}

void texture_map::PageOut(bool release)
{
	for(int i = 0; i < TM_NUM_TYPES; i++)
		this->textures[i].PageOut(release);
}

void texture_map::Clear()
{
	is_ambient = false;
	is_transparent = false;

	for(int i = 0; i < TM_NUM_TYPES; i++)
		this->textures[i].clear();
}

void texture_map::ResetToOriginal()
{
	for(int i = 0; i < TM_NUM_TYPES; i++)
		this->textures[i].ResetTexture();
}

bsp_polygon_data::bsp_polygon_data(ubyte* bsp_data)
{
	Polygon_vertices.clear();
	Polygons.clear();

	for (int i = 0; i < MAX_MODEL_TEXTURES; ++i) {
		Num_verts[i] = 0;
		Num_polies[i] = 0;
	}

	Num_flat_verts = 0;
	Num_flat_polies = 0;

	process_bsp(0, bsp_data);
}

void bsp_polygon_data::process_bsp(int offset, ubyte* bsp_data)
{
	int id = w(bsp_data + offset);
	int size = w(bsp_data + offset + 4);

	bool end = id == OP_EOF;
	while (!end) {
		switch (id)
		{
		case OP_DEFPOINTS:
			process_defpoints(offset, bsp_data);
			break;

		case OP_SORTNORM:
			process_sortnorm(offset, bsp_data);
			break;

		case OP_SORTNORM2:
			process_sortnorm2(offset, bsp_data);
			end = true; // should not continue after this chunk
			break;

		case OP_FLATPOLY:
			process_flat(offset, bsp_data);
			break;

		case OP_TMAPPOLY:
			process_tmap(offset, bsp_data);
			break;

		case OP_BOUNDBOX:
			break;

		case OP_TMAP2POLY:
			process_tmap2(offset, bsp_data);
			end = true; // should not continue after this chunk
			break;

		default:
			return;
		}

		offset += size;
		id = w(bsp_data + offset);
		size = w(bsp_data + offset + 4);

		if (size < 1 || id == OP_EOF)
			end = true;
	}
}

void bsp_polygon_data::process_defpoints(int off, ubyte* bsp_data)
{
	uint i, n;
	uint nverts = uw(off + bsp_data + 8);
	uint offset = uw(off + bsp_data + 16);

	ubyte *normcount = off + bsp_data + 20;
	vec3d *src = vp(off + bsp_data + offset);

	// Get pointer to lights
	Lights = off + bsp_data + 20 + nverts;

#ifndef NDEBUG
	modelstats_num_verts += nverts;
#endif

	Vertex_list.clear();
	Normal_list.clear();

	for (n = 0; n < nverts; n++) {
		Vertex_list.push_back(*src);
		src++; // move to normal

		for (i = 0; i < normcount[n]; i++) {
			Normal_list.push_back(*src);
			src++;
		}
	}
}

void bsp_polygon_data::process_sortnorm2(int offset, ubyte* bsp_data)
{
	int frontlist, backlist;

	frontlist = w(bsp_data + offset + 8);
	backlist = w(bsp_data + offset + 12);

	if (backlist) process_bsp(offset + backlist, bsp_data);
	if (frontlist) process_bsp(offset + frontlist, bsp_data);
}

void bsp_polygon_data::process_sortnorm(int offset, ubyte* bsp_data)
{
	int frontlist, backlist, prelist, postlist, onlist;

	frontlist = w(bsp_data + offset + 36);
	backlist = w(bsp_data + offset + 40);
	prelist = w(bsp_data + offset + 44);
	postlist = w(bsp_data + offset + 48);
	onlist = w(bsp_data + offset + 52);

	if (prelist) process_bsp(offset + prelist, bsp_data);
	if (backlist) process_bsp(offset + backlist, bsp_data);
	if (onlist) process_bsp(offset + onlist, bsp_data);
	if (frontlist) process_bsp(offset + frontlist, bsp_data);
	if (postlist) process_bsp(offset + postlist, bsp_data);
}

void bsp_polygon_data::process_tmap(int offset, ubyte* bsp_data)
{
	int pof_tex = w(bsp_data + offset + 40);
	uint n_vert = uw(bsp_data + offset + 36);

	if ( n_vert < 3 ) {
		// don't parse degenerate polygons
		return;
	}

	ubyte *p = &bsp_data[offset + 8];
	auto tverts = new model_tmap_vert[n_vert];

	// Copy the verts manually since they aren't aligned with the struct
	unpack_tmap_verts(&bsp_data[offset + 44], tverts, n_vert);

	int problem_count = 0;

	// make a polygon
	bsp_polygon polygon;

	polygon.Start_index = (uint)Polygon_vertices.size();
	polygon.Num_verts = n_vert;
	polygon.texture = pof_tex;

	// this polygon will be broken up into a triangle fan. first three verts make up the first triangle
	// additional verts are made into new tris
	Num_polies[pof_tex]++;
	Num_verts[pof_tex] += n_vert;

	// stuff data making up the vertices of this polygon
	for ( uint i = 0; i < n_vert; ++i ) {
		bsp_vertex vert;

		vert.position = Vertex_list[tverts[i].vertnum];
		
		vert.tex_coord.u = tverts[i].u;
		vert.tex_coord.v = tverts[i].v;

		vert.normal = Normal_list[tverts[i].normnum];

		// see if this normal is okay
		if (IS_VEC_NULL(&vert.normal))
			vert.normal = *vp(p);

		problem_count += check_values(&vert.normal);
		vm_vec_normalize_safe(&vert.normal);

		Polygon_vertices.push_back(vert);
	}

	Polygons.push_back(polygon);

	Parse_normal_problem_count += problem_count;
}

/**
* @brief Converts a TMAP2POLY chunk into a list of BSP_polygon.
* 
* @param offset The byte offset into bsp_data.
* @param[in] The buffer containing the chunk data.
*/
void bsp_polygon_data::process_tmap2(int offset, ubyte* bsp_data)
{
	int pof_tex = w(bsp_data + offset + 44);
	uint n_vert = uw(bsp_data + offset + 48);
	model_tmap_vert* tverts;
	int problem_count = 0;
	bsp_polygon polygon;

	if (n_vert < 3) {
		Error(LOCATION, "Model contains TMAP2 chunk with less than 3 vertices!");
		return;
	}

	tverts = (model_tmap_vert*)&bsp_data[offset + 52];

	// make a polygon
	polygon.Start_index = (uint)Polygon_vertices.size();
	polygon.Num_verts = n_vert;
	polygon.texture = pof_tex;

	// this polygon will be broken up into a triangle fan. first three verts make up the first triangle
	// additional verts are made into new tris
	Num_polies[pof_tex]++;
	Num_verts[pof_tex] += n_vert;

	// stuff data making up the vertices of this polygon
	for (uint i = 0; i < n_vert; ++i) {
		bsp_vertex vert;

		vert.position = Vertex_list[tverts[i].vertnum];

		vert.tex_coord.u = tverts[i].u;
		vert.tex_coord.v = tverts[i].v;

		vert.normal = Normal_list[tverts[i].normnum];

		// see if this normal is okay
		if (IS_VEC_NULL(&vert.normal))
			vert.normal = *vp(&bsp_data[offset + 32]);

		problem_count += check_values(&vert.normal);
		vm_vec_normalize_safe(&vert.normal);

		Polygon_vertices.push_back(vert);
	}

	Polygons.push_back(polygon);

	Parse_normal_problem_count += problem_count;
}

void bsp_polygon_data::process_flat(int offset, ubyte* bsp_data)
{
	int n_vert = w(bsp_data + offset + 36);

	if (n_vert < 3) {
		// don't parse degenerate polygons
		return;
	}

	short * verts = (short *)(bsp_data + offset + 44);

	ubyte r = *(bsp_data + offset + 40);
	ubyte g = *(bsp_data + offset + 41);
	ubyte b = *(bsp_data + offset + 42);

	bsp_polygon polygon;

	polygon.Start_index = (uint)Polygon_vertices.size();
	polygon.Num_verts = n_vert;
	polygon.texture = -1;

	Num_flat_polies++;
	Num_flat_verts += n_vert;

	for (int i = 0; i < n_vert; i++) {
		bsp_vertex vert;

		int vertnum = verts[i * 2 + 0];
		int norm = verts[i * 2 + 1];

		vert.position = Vertex_list[vertnum];
		vert.normal = Normal_list[norm];

		vert.r = r;
		vert.g = g;
		vert.b = b;
		vert.a = 255;

		Polygon_vertices.push_back(vert);
	}

	Polygons.push_back(polygon);
}

int bsp_polygon_data::get_num_triangles(int texture)
{
	if ( texture < 0 ) {
		return MAX(Num_flat_verts - 2 * Num_flat_polies, 0);
	}

	return MAX(Num_verts[texture] - 2 * Num_polies[texture], 0);
}

int bsp_polygon_data::get_num_lines(int texture)
{
	if (texture < 0) {
		return Num_flat_verts;
	}

	return Num_verts[texture];
}

void bsp_polygon_data::generate_triangles(int texture, vertex *vert_ptr, vec3d* norm_ptr)
{
	int num_verts = 0;

	for ( uint i = 0; i < Polygons.size(); ++i ) {
		if ( Polygons[i].texture != texture ) {
			continue;
		}

		uint start_index = Polygons[i].Start_index;
		uint end_index = Polygons[i].Start_index + Polygons[i].Num_verts;

		for ( uint j = start_index + 1; j < end_index - 1; ++j ) {
			// first vertex of this triangle. Always the first vertex of the polygon
			vertex* vert = &vert_ptr[num_verts];
			vert->world = Polygon_vertices[start_index].position;
			vert->texture_position = Polygon_vertices[start_index].tex_coord;

			vec3d* norm = &norm_ptr[num_verts];
			*norm = Polygon_vertices[start_index].normal;

			// second vertex of this triangle. 
			vert = &vert_ptr[num_verts + 1];
			vert->world = Polygon_vertices[j].position;
			vert->texture_position = Polygon_vertices[j].tex_coord;

			norm = &norm_ptr[num_verts + 1];
			*norm = Polygon_vertices[j].normal;

			// third vertex of this triangle. 
			vert = &vert_ptr[num_verts + 2];
			vert->world = Polygon_vertices[j+1].position;
			vert->texture_position = Polygon_vertices[j+1].tex_coord;

			norm = &norm_ptr[num_verts + 2];
			*norm = Polygon_vertices[j+1].normal;

			num_verts += 3;
		}
	}
}

void bsp_polygon_data::generate_lines(int texture, vertex *vert_ptr)
{
	int num_verts = 0;

	for (uint i = 0; i < Polygons.size(); ++i) {
		if (Polygons[i].texture != texture) {
			continue;
		}

		uint start_index = Polygons[i].Start_index;
		uint end_index = Polygons[i].Start_index + Polygons[i].Num_verts;

		for (uint j = start_index; j < end_index; ++j) {
			// first vertex of this triangle. Always the first vertex of the polygon
			vertex* vert = &vert_ptr[num_verts];
			vert->world = Polygon_vertices[j].position;
			vert->r = Polygon_vertices[j].r;
			vert->g = Polygon_vertices[j].g;
			vert->b = Polygon_vertices[j].b;
			vert->a = Polygon_vertices[j].a;

			if ( j == end_index - 1 ) {
				vert = &vert_ptr[num_verts + 1];
				vert->world = Polygon_vertices[start_index].position;
				vert->r = Polygon_vertices[start_index].r;
				vert->g = Polygon_vertices[start_index].g;
				vert->b = Polygon_vertices[start_index].b;
				vert->a = Polygon_vertices[start_index].a;
			} else {
				vert = &vert_ptr[num_verts + 1];
				vert->world = Polygon_vertices[j + 1].position;
				vert->r = Polygon_vertices[j + 1].r;
				vert->g = Polygon_vertices[j + 1].g;
				vert->b = Polygon_vertices[j + 1].b;
				vert->a = Polygon_vertices[j + 1].a;
			}

			num_verts += 2;
		}
	}
}
