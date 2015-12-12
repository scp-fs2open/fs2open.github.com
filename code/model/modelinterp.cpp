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
#include "graphics/gropengllight.h"
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
#include "weapon/shockwave.h"

#include <limits.h>


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
	void process_tmap(int offset, ubyte* bsp_data);
	void process_flat(int offset, ubyte* bsp_data);
public:
	bsp_polygon_data(ubyte* bsp_data);

	int get_num_triangles(int texture);
	int get_num_lines(int texture);

	void generate_triangles(int texture, vertex *vert_ptr, vec3d* norm_ptr);
	void generate_lines(int texture, vertex *vert_ptr);
};

// -----------------------
// Local variables
//

static int Num_interp_verts_allocated = 0;
vec3d **Interp_verts = NULL;
static vertex *Interp_points = NULL;
static vertex *Interp_splode_points = NULL;
vec3d *Interp_splode_verts = NULL;
static int Interp_num_verts = 0;

static vertex **Interp_list = NULL;
static int  Num_interp_list_verts_allocated = 0;

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


static int Num_interp_norms_allocated = 0;
static vec3d **Interp_norms = NULL;
static ubyte *Interp_light_applied = NULL;
static int Interp_num_norms = 0;
static ubyte *Interp_lights;

static float Interp_fog_level = 0.0f;

// Stuff to control rendering parameters
static color Interp_outline_color;
static int Interp_detail_level_locked = -1;
static uint Interp_flags = 0;
static uint Interp_tmap_flags = 0;
bool Interp_desaturate = false;

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
static int Interp_warp_bitmap = -1;
static float Interp_warp_alpha = -1.0f;

static int Interp_objnum = -1;

// if != -1, use this bitmap when rendering ship insignias
static int Interp_insignia_bitmap = -1;

// replacement - Goober5000
// updated - WMC
static int *Interp_new_replacement_textures = NULL;

static fix Interp_base_frametime = 0;

// if != -1, use this bitmap when rendering with a forced texture
static int Interp_forced_bitmap = -1;

// for rendering with the MR_ALL_XPARENT FLAG SET
static float Interp_xparent_alpha = 1.0f;

float Interp_light = 0.0f;

int Interp_multitex_cloakmap = -1;
int Interp_cloakmap_alpha = 255;

// our current level of detail (LOD)
int Interp_detail_level = 0;

static int FULLCLOAK = -1;

// clip planes
bool Interp_clip_plane;
vec3d Interp_clip_pos;
vec3d Interp_clip_normal;

// team colors
team_color Interp_team_color;
bool Interp_team_color_set = false;

// animated shader effects
int Interp_animated_effect = 0;
float Interp_animated_timer = 0.0f;

int Interp_no_flush = 0;

// forward references
int model_interp_sub(void *model_ptr, polymodel * pm, bsp_info *sm, int do_box_check);
void model_really_render(int model_num, matrix *orient, vec3d * pos, uint flags, int render, int objnum = -1);
void model_interp_sortnorm_b2f(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check);
void model_interp_sortnorm_f2b(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check);
void (*model_interp_sortnorm)(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check) = model_interp_sortnorm_b2f;
int model_should_render_engine_glow(int objnum, int bank_obj);
void model_render_buffers_DEPRECATED(polymodel *pm, int mn,int render, bool is_child = false);
void model_render_children_buffers_DEPRECATED(polymodel * pm, int mn, int detail_level, int render);
int model_interp_get_texture(texture_info *tinfo, fix base_frametime);

void model_deallocate_interp_data()
{
	if (Interp_verts != NULL)
		vm_free(Interp_verts);

	if (Interp_points != NULL)
		vm_free(Interp_points);

	if (Interp_splode_points != NULL)
		vm_free(Interp_splode_points);

	if (Interp_splode_verts != NULL)
		vm_free(Interp_splode_verts);

	if (Interp_norms != NULL)
		vm_free(Interp_norms);

	if (Interp_light_applied != NULL)
		vm_free(Interp_light_applied);

	if (Interp_lighting_temp.lights != NULL)
		vm_free(Interp_lighting_temp.lights);

	Num_interp_verts_allocated = 0;
	Num_interp_norms_allocated = 0;
}

extern void model_collide_allocate_point_list(int n_points);
extern void model_collide_free_point_list();

void model_allocate_interp_data(int n_verts = 0, int n_norms = 0, int n_list_verts = 0)
{
	static ubyte dealloc = 0;

	if (!dealloc) {
		atexit(model_deallocate_interp_data);
		atexit(model_collide_free_point_list);
		dealloc = 1;
	}

	Assert( (n_verts >= 0) && (n_norms >= 0) && (n_list_verts >= 0) );
	Assert( (n_verts || Num_interp_verts_allocated) && (n_norms || Num_interp_norms_allocated) /*&& (n_list_verts || Num_interp_list_verts_allocated)*/ );

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

	// we should only get here if we are not in HTL mode
	if ( n_list_verts > Num_interp_list_verts_allocated ) {
		if (Interp_list != NULL) {
			vm_free(Interp_list);
			Interp_list = NULL;
		}

		Interp_list = (vertex**) vm_malloc( n_list_verts * sizeof(vertex) );
		Verify( Interp_list != NULL );

		Num_interp_list_verts_allocated = n_list_verts;
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

void model_setup_cloak(vec3d *shift, int full_cloak, int alpha)
{
	FULLCLOAK=full_cloak;
	int unit;
	if (full_cloak)
	{
		unit=0;
		Interp_multitex_cloakmap=0;
		model_set_insignia_bitmap(-1);
		model_set_forced_texture(CLOAKMAP);
		gr_set_cull(1);
		model_interp_sortnorm=model_interp_sortnorm_f2b;
	}
	else
	{
		unit=2;
		Interp_multitex_cloakmap=1;
	}

	Interp_cloakmap_alpha=alpha;
	gr_push_texture_matrix(unit);
	gr_translate_texture_matrix(unit,shift);
}

void model_finish_cloak(int full_cloak)
{
	int unit;
	if (full_cloak){		unit=0; gr_set_cull(0);}
	else				unit=2;

	gr_pop_texture_matrix(unit);
	Interp_multitex_cloakmap=-1;
	model_set_forced_texture(-1);
	FULLCLOAK=-1;
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

	Interp_team_color_set = false;

	Interp_clip_plane = false;

	Interp_animated_effect = 0;
	Interp_animated_timer = 0.0f;

	Interp_detail_level_locked = -1;

	Interp_forced_bitmap = -1;
}

/**
 * Scales the engines thrusters by this much
 */
void model_set_thrust(int model_num, mst_info *mst)
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
void model_interp_splode_defpoints(ubyte * p, polymodel *pm, bsp_info *sm, float dist)
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

	int i, n;
	int nverts = w(p+8);	
	int offset = w(p+16);
	int next_norm = 0;
	int nnorms = 0;

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

			if(!Cmdline_nohtl) {
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
			}
			else {
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

matrix *Interp_orient;
vec3d *Interp_pos;


// Flat Poly
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      center
// +32     float       radius
// +36     int         nverts
// +40     byte        red
// +41     byte        green
// +42     byte        blue
// +43     byte        pad
// +44     nverts*short*short  vertlist, smoothlist
void model_interp_flatpoly(ubyte * p,polymodel * pm)
{
	int nv = w(p+36);

	if ( nv < 0 )
		return;

	#ifndef NDEBUG
	modelstats_num_polys++;
	#endif

	if ( !g3_check_normal_facing(vp(p+20), vp(p+8)) )
		return;
	

	int i;
	short * verts = (short *)(p+44);

	int max_n_verts = 0;
	int max_n_norms = 0;

	// slow?  yes.  safe?  yes.
	for (i = 0; i < nv; i++) {
		max_n_verts = MAX(verts[i*2+0] + 1, max_n_verts);
		max_n_norms = MAX(verts[i*2+1] + 1, max_n_norms);
	}

	model_allocate_interp_data(max_n_verts, max_n_norms, nv);

	for (i = 0; i < nv; i++) {
		Interp_list[i] = &Interp_points[verts[i*2]];

		if ( Interp_flags & MR_NO_LIGHTING )	{
				Interp_list[i]->r = 191;
				Interp_list[i]->g = 191;
				Interp_list[i]->b = 191;
		} else {
			int vertnum = verts[i*2+0];
			int norm = verts[i*2+1];
	
			if ( Interp_flags & MR_NO_SMOOTHING )	{
				light_apply_rgb( &Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[vertnum], vp(p+8), Interp_light );
			} else {
				// if we're not using saved lighting
				if ( !Interp_use_saved_lighting && !Interp_light_applied[norm] )	{
					light_apply_rgb( &Interp_lighting->lights[norm].r, &Interp_lighting->lights[norm].g, &Interp_lighting->lights[norm].b, Interp_verts[vertnum], vp(p+8), Interp_light );
					Interp_light_applied[norm] = 1;
				}

				Interp_list[i]->r = Interp_lighting->lights[norm].r;
				Interp_list[i]->g = Interp_lighting->lights[norm].g;
				Interp_list[i]->b = Interp_lighting->lights[norm].b;
			}
		}
	}

	// HACK!!! FIX ME!!! I'M SLOW!!!!
	if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
		gr_set_color( *(p+40), *(p+41), *(p+42) );
	}

	if ( !(Interp_flags & MR_NO_POLYS))	{
		g3_draw_poly( nv, Interp_list, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB );	
	}

	if (Interp_flags & (MR_SHOW_OUTLINE|MR_SHOW_OUTLINE_PRESET))	{
		int j;

		if ( Interp_flags & MR_SHOW_OUTLINE )	{
			gr_set_color_fast( &Interp_outline_color );
		}

		for (i = 0; i < nv; i++) {
			j = (i + 1) % nv;
			g3_draw_line(Interp_list[i], Interp_list[j]);
		}
	}
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

// Textured Poly
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      center
// +32     float       radius
// +36     int         nverts
// +40     int         tmap_num
// +44     nverts*(model_tmap_vert) vertlist (n,u,v)
extern int Tmap_show_layers;

int Interp_subspace = 0;
float Interp_subspace_offset_u = 0.0f;
float Interp_subspace_offset_v = 0.0f;
ubyte Interp_subspace_r = 255;
ubyte Interp_subspace_g = 255;
ubyte Interp_subspace_b = 255;

void model_interp_tmappoly(ubyte * p,polymodel * pm)
{
	int i;
	int nv;
	model_tmap_vert *verts;
	int cull = 0;

	// Goober5000
	int tmap_num = w(p+40);
	texture_map *tmap = &pm->maps[tmap_num];
	texture_info *tbase = &tmap->textures[TM_BASE_TYPE];
	texture_info *tglow = &tmap->textures[TM_GLOW_TYPE];
	int rt_begin_index = tmap_num*TM_NUM_TYPES;

	int is_invisible = 0;

	if (Interp_warp_bitmap < 0) {
		if ( (!Interp_thrust_scale_subobj) && (tbase->GetTexture() < 0) ) {
			// Ignore the following if we're drawing in outline mode.  Fixes Mantis #2931.
			if (!(Interp_flags & (MR_SHOW_OUTLINE|MR_SHOW_OUTLINE_PRESET))) {
				// Don't draw invisible polygons.
				if ( !(Interp_flags & MR_SHOW_INVISIBLE_FACES) )
					return;
				else
					is_invisible = 1;
			}
		}
	}

	nv = w(p+36);

	#ifndef NDEBUG
	modelstats_num_polys++;
	#endif

	if ( nv < 0 )
		return;

	verts = (model_tmap_vert *)(p+44);

	int max_n_verts = 0;
	int max_n_norms = 0;

	for (i = 0; i < nv; i++) {
		max_n_verts = MAX(verts[i].vertnum + 1, max_n_verts);
		max_n_norms = MAX(verts[i].normnum + 1, max_n_norms);
	}

	model_allocate_interp_data(max_n_verts, max_n_norms, nv);

	if ( !Cmdline_nohtl ) {
		if (Interp_warp_bitmap < 0) {
			if (!g3_check_normal_facing(vp(p+20),vp(p+8)) && !(Interp_flags & MR_NO_CULL)){
				if(!splodeing) return;
			}
		}

		if(splodeing){
			float salpha = 1.0f - splode_level;
			for (i=0;i<nv;i++){
				Interp_list[i] = &Interp_splode_points[verts[i].vertnum];
				Interp_list[i]->texture_position.u = verts[i].u*2;
				Interp_list[i]->texture_position.v = verts[i].v*2;
				Interp_list[i]->r = (unsigned char)(255*salpha);
				Interp_list[i]->g = (unsigned char)(250*salpha);
				Interp_list[i]->b = (unsigned char)(200*salpha);
				model_interp_edge_alpha(&Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[verts[i].vertnum], Interp_norms[verts[i].normnum], salpha, false);
			}
			cull = gr_set_cull(0);
			gr_set_bitmap( splodeingtexture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, salpha );
			g3_draw_poly( nv, Interp_list,  TMAP_FLAG_TEXTURED|TMAP_FLAG_GOURAUD);
			gr_set_cull(cull);
			return;
		}
	}

	for (i=0;i<nv;i++)	{
		Interp_list[i] = &Interp_points[verts[i].vertnum];

		Interp_list[i]->texture_position.u = verts[i].u;
		Interp_list[i]->texture_position.v = verts[i].v;
		
		if ( Interp_subspace )	{
			Interp_list[i]->texture_position.v += Interp_subspace_offset_u;
			Interp_list[i]->texture_position.u += Interp_subspace_offset_v;
			Interp_list[i]->r = Interp_subspace_r;
			Interp_list[i]->g = Interp_subspace_g;
			Interp_list[i]->b = Interp_subspace_b;
		} else {
			if ( (Interp_flags & MR_NO_LIGHTING) || (tmap->is_ambient))	{	//gets the ambient glow to work
				Interp_list[i]->r = 191;
				Interp_list[i]->g = 191;
				Interp_list[i]->b = 191;
				
				Interp_list[i]->spec_r = 0;
				Interp_list[i]->spec_g = 0;
				Interp_list[i]->spec_b = 0;
				
				if (Interp_flags & MR_EDGE_ALPHA) {
					model_interp_edge_alpha(&Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[verts[i].vertnum], Interp_norms[verts[i].normnum], Interp_warp_alpha, false);
				}

				if (Interp_flags & MR_CENTER_ALPHA) {
					model_interp_edge_alpha(&Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[verts[i].vertnum], Interp_norms[verts[i].normnum], Interp_warp_alpha, true);
				}

				SPECMAP = -1;
				NORMMAP = -1;
				HEIGHTMAP = -1;
				MISCMAP = -1;
			} else {

				int vertnum = verts[i].vertnum;
				int norm = verts[i].normnum;
		
				if ( Interp_flags & MR_NO_SMOOTHING )	{
					light_apply_rgb( &Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[vertnum], vp(p+8), Interp_light );
					if((Detail.lighting > 2) && (Interp_detail_level < 2) && Cmdline_spec )
						light_apply_specular( &Interp_list[i]->spec_r, &Interp_list[i]->spec_g, &Interp_list[i]->spec_b, Interp_verts[vertnum], vp(p+8),  &View_position);
				} else {					
					// if we're applying lighting as normal, and not using saved lighting
					if ( !Interp_use_saved_lighting && !Interp_light_applied[norm] )	{

						light_apply_rgb( &Interp_lighting->lights[norm].r, &Interp_lighting->lights[norm].g, &Interp_lighting->lights[norm].b, Interp_verts[vertnum], Interp_norms[norm], Interp_light );
						if((Detail.lighting > 2) && (Interp_detail_level < 2) && Cmdline_spec )
							light_apply_specular( &Interp_lighting->lights[norm].spec_r, &Interp_lighting->lights[norm].spec_g, &Interp_lighting->lights[norm].spec_b, Interp_verts[vertnum], Interp_norms[norm],  &View_position);

						Interp_light_applied[norm] = 1;
					}

					Interp_list[i]->spec_r = Interp_lighting->lights[norm].spec_r;
					Interp_list[i]->spec_g = Interp_lighting->lights[norm].spec_g;
					Interp_list[i]->spec_b = Interp_lighting->lights[norm].spec_b;

					Interp_list[i]->r = Interp_lighting->lights[norm].r;
					Interp_list[i]->g = Interp_lighting->lights[norm].g;
					Interp_list[i]->b = Interp_lighting->lights[norm].b;
				}
			}
		}
	}

	#ifndef NDEBUG
	modelstats_num_polys_drawn++;
	#endif

	if (!(Interp_flags & MR_NO_POLYS) )		{
		if ( is_invisible )	{
			gr_set_color( 0, 255, 0 );
			g3_draw_poly( nv, Interp_list, 0 );		
		} else if (Interp_thrust_scale_subobj)	{
			if ( (Interp_thrust_bitmap >= 0) && (Interp_thrust_scale > 0.0f) ) {
				gr_set_bitmap( Interp_thrust_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.2f );
				g3_draw_poly( nv, Interp_list, TMAP_FLAG_TEXTURED );		
			} else {
				if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
					gr_set_color( 128, 128, 255 );
				}
				uint tflags = Interp_tmap_flags;
				tflags &=  (~(TMAP_FLAG_TEXTURED|TMAP_FLAG_TILED|TMAP_FLAG_CORRECT));
				g3_draw_poly( nv, Interp_list, tflags );		
			}
		} else if (Interp_warp_bitmap >= 0) {	//warpin effect-Bobboau
			gr_set_bitmap( Interp_warp_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Interp_warp_alpha );
			g3_draw_poly( nv, Interp_list, TMAP_FLAG_TEXTURED );
		}else{
			// all textured polys go through here
			if ( Interp_tmap_flags & TMAP_FLAG_TEXTURED )	{
				// subspace special case
				if (Interp_subspace) {										
					gr_set_bitmap( tbase->GetTexture(), GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.2f );					
				}
				// all other textures
				else {					
					int texture;

					// if we're rendering a nebula background pof, maybe select a custom texture
					if(Interp_forced_bitmap >= 0){
						texture = Interp_forced_bitmap;
					}else if((Interp_new_replacement_textures != NULL) && (Interp_new_replacement_textures[rt_begin_index + TM_BASE_TYPE] >= 0)){
						texture_info tt = texture_info(Interp_new_replacement_textures[rt_begin_index + TM_BASE_TYPE]);
						texture = model_interp_get_texture(&tt, Interp_base_frametime);
					} else {
						// pick the texture, animating it if necessary
						texture = model_interp_get_texture(tbase, Interp_base_frametime);
						
						// doing glow maps?
						if ( !(Interp_flags & MR_NO_GLOWMAPS) && (tglow->GetTexture() >= 0) ) {
							// shockwaves are special, their current frame has to come out of the shockwave code to get the timing correct
							if ( (Interp_objnum >= 0) && (Objects[Interp_objnum].type == OBJ_SHOCKWAVE) && (tglow->GetNumFrames() > 1) ) {
								GLOWMAP = tglow->GetTexture() + shockwave_get_framenum(Objects[Interp_objnum].instance, tglow->GetNumFrames());
							} else {
								GLOWMAP = model_interp_get_texture(tglow, Interp_base_frametime);
							}
						}

						if((Detail.lighting > 2)  && (Interp_detail_level < 2))
						{
							// likewise, etc.
							SPECMAP = model_interp_get_texture(&tmap->textures[TM_SPECULAR_TYPE], Interp_base_frametime);
							NORMMAP = model_interp_get_texture(&tmap->textures[TM_NORMAL_TYPE], Interp_base_frametime);
							HEIGHTMAP = model_interp_get_texture(&tmap->textures[TM_HEIGHT_TYPE], Interp_base_frametime);
						}

						MISCMAP = model_interp_get_texture(&tmap->textures[TM_MISC_TYPE], Interp_base_frametime);
					}

					//*****
					//WMC - now do other replacements
					if(Interp_new_replacement_textures != NULL)
					{
						texture_info tinfo;
						for(int tmn = TM_BASE_TYPE+1; tmn < TM_NUM_TYPES; tmn++)
						{
							int tex = Interp_new_replacement_textures[rt_begin_index + tmn];
							if(tex < 0)
								continue;

							tinfo = texture_info(tex);

							//Figure out actual texture to use
							tex = model_interp_get_texture(&tinfo, Interp_base_frametime);
							switch(tmn)
							{
								case TM_GLOW_TYPE:
									GLOWMAP = tex;
									break;
								case TM_SPECULAR_TYPE:
									SPECMAP = tex;
									break;
								case TM_NORMAL_TYPE:
									NORMMAP = tex;
									break;
								case TM_HEIGHT_TYPE:
									HEIGHTMAP = tex;
									break;
								case TM_MISC_TYPE:
									MISCMAP = tex;
								default:
									break;
							}
						}
					}

					// muzzle flashes draw xparent
					if(Interp_flags & MR_ALL_XPARENT){
						gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Interp_xparent_alpha );
					} else {
						if(tmap->is_transparent) {	//trying to get transperent textures-Bobboau
							gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.8f );
						}else{
							gr_set_bitmap( texture );
						}
					}
				}
			} else {
				if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
					gr_set_color( 128, 128, 128 );
				}
			}

			if ( Interp_subspace )	{
				g3_draw_poly( nv, Interp_list,  TMAP_FLAG_TEXTURED|TMAP_FLAG_TILED|TMAP_FLAG_CORRECT );		
			} else {				
				if(Interp_flags & MR_ALL_XPARENT){
					g3_draw_poly( nv, Interp_list, Interp_tmap_flags );
				} else {
					g3_draw_poly( nv, Interp_list, Interp_tmap_flags );		
				}
			}
		}
	}

	GLOWMAP = -1;
	SPECMAP = -1;
	NORMMAP = -1;
	HEIGHTMAP = -1;
	MISCMAP = -1;

	if (Interp_flags & (MR_SHOW_OUTLINE|MR_SHOW_OUTLINE_PRESET) )	{
	
		if ( Interp_flags & MR_SHOW_OUTLINE )	{
			gr_set_color_fast( &Interp_outline_color );
		}

		for (i=0; i<nv; i++ )	{
			int j = (i + 1) % nv;
	   		g3_draw_line(Interp_list[i], Interp_list[j]);
		}
	}

}

// Sortnorms
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      normal_point
// +32     int         tmp=0
// 36     int     front offset
// 40     int     back offset
// 44     int     prelist offset
// 48     int     postlist offset
// 52     int     online offset
void model_interp_sortnorm_b2f(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check)
{
	#ifndef NDEBUG
	modelstats_num_sortnorms++;
	#endif

	int frontlist = w(p+36);
	int backlist = w(p+40);
	int prelist = w(p+44);
	int postlist = w(p+48);
	int onlist = w(p+52);

	if (prelist) model_interp_sub(p+prelist,pm,sm,do_box_check);		// prelist

	if (g3_check_normal_facing(vp(p+20),vp(p+8))) {		//facing

		//draw back then front

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

	}	else {			//not facing.  draw front then back

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);
	}

	if (postlist) model_interp_sub(p+postlist,pm,sm,do_box_check);		// postlist

}

// Sortnorms
// +0      int         id
// +4      int         size 
// +8      vec3d      normal
// +20     vec3d      normal_point
// +32     int         tmp=0
// 36     int     front offset
// 40     int     back offset
// 44     int     prelist offset
// 48     int     postlist offset
// 52     int     online offset
void model_interp_sortnorm_f2b(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check)
{
	#ifndef NDEBUG
	modelstats_num_sortnorms++;
	#endif

	int frontlist = w(p+36);
	int backlist = w(p+40);
	int prelist = w(p+44);
	int postlist = w(p+48);
	int onlist = w(p+52);

	if (postlist) model_interp_sub(p+postlist,pm,sm,do_box_check);		// postlist

	if (g3_check_normal_facing(vp(p+20),vp(p+8))) {		//facing

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);

	}	else {			//not facing.  draw front then back

		//draw back then front

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

	}

	if (prelist) model_interp_sub(p+prelist,pm,sm,do_box_check);		// prelist
}

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
void model_draw_paths( int model_num, uint flags )
{
	int i,j;
	vec3d pnt;
	polymodel * pm;	

	if ( flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	pm = model_get(model_num);

	if (pm->n_paths<1){
		return;
	}	

	for (i=0; i<pm->n_paths; i++ )	{
		vertex prev_pnt;

		for (j=0; j<pm->paths[i].nverts; j++ )	{
			// Rotate point into world coordinates			
			pnt = pm->paths[i].verts[j].pos;

			// Pnt is now the x,y,z world coordinates of this vert.
			// For this example, I am just drawing a sphere at that
			// point.
			{
				vertex tmp = vertex();
				g3_rotate_vertex(&tmp,&pnt);

				if ( pm->paths[i].verts[j].nturrets > 0 ){
					gr_set_color( 0, 0, 255 );						// draw points covered by turrets in blue
				} else {
					gr_set_color( 255, 0, 0 );
				}

				g3_draw_sphere( &tmp, 0.5f );

				if (j){
					g3_draw_line(&prev_pnt, &tmp);
				}

				prev_pnt = tmp;
			}
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

			g3_draw_htl_sphere(&pnt, 0.5f);
			
			if (j)
			{
				g3_draw_htl_line(&prev_pnt, &pnt);
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
			g3_draw_htl_sphere(&v1, 2.0);
			g3_draw_htl_line(&v1, &v2);
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

				g3_draw_htl_line(&v1, &v2);
			}
		}
	}	

	gr_set_cull(cull);
}

/**
 * Docking bay and fighter bay paths
 */
void model_draw_bay_paths(int model_num)
{
	int idx, s_idx;
	vec3d v1, v2;
	vertex l1, l2;	

	polymodel *pm = model_get(model_num);
	if(pm == NULL){
		return;
	}

	// render docking bay normals
	gr_set_color(0, 255, 0);
	for(idx=0; idx<pm->n_docks; idx++){
		for(s_idx=0; s_idx<pm->docking_bays[idx].num_slots; s_idx++){
			v1 = pm->docking_bays[idx].pnt[s_idx];
			vm_vec_scale_add(&v2, &v1, &pm->docking_bays[idx].norm[s_idx], 10.0f);

			// rotate the points
			g3_rotate_vertex(&l1, &v1);
			g3_rotate_vertex(&l2, &v2);

			// draw the point and normal
			g3_draw_sphere(&l1, 2.0);
			g3_draw_line(&l1, &l2);
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

				// rotate and draw
				g3_rotate_vertex(&l1, &v1);
				g3_rotate_vertex(&l2, &v2);
				g3_draw_line(&l1, &l2);
			}
		}
	}	
}


static const int MAX_ARC_SEGMENT_POINTS = 50;
int Num_arc_segment_points = 0;
vec3d Arc_segment_points[MAX_ARC_SEGMENT_POINTS];

extern int g3_draw_rod(int num_points, const vec3d *vecs, float width, uint tmap_flags);

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

void interp_render_arc(vec3d *v1, vec3d *v2, color *primary, color *secondary, float arc_width)
{
	Num_arc_segment_points = 0;

	// need need to add the first point
	memcpy( &Arc_segment_points[Num_arc_segment_points++], v1, sizeof(vec3d) );

	// this should fill in all of the middle, and the last, points
	interp_render_arc_segment(v1, v2, 0);

	int tmap_flags = (TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_HTL_3D_UNLIT | TMAP_FLAG_TRISTRIP);

	int mode = gr_zbuffer_set(GR_ZBUFF_READ);

	// use primary color for fist pass
	Assert( primary );
	gr_set_color_fast(primary);

	g3_draw_rod(Num_arc_segment_points, Arc_segment_points, arc_width, tmap_flags);

	if (secondary) {
		// now render again with a secondary center color
		gr_set_color_fast(secondary);

		g3_draw_rod(Num_arc_segment_points, Arc_segment_points, arc_width * 0.33f, tmap_flags);
	}

	gr_zbuffer_set(mode);
}

int Interp_lightning = 1;
DCF_BOOL( Arcs, Interp_lightning )

const int AR = 64;
const int AG = 64;
const int AB = 5;
const int AR2 = 128;
const int AG2 = 128;
const int AB2 = 10;

void interp_render_lightning( polymodel *pm, bsp_info * sm )
{
	int i;
	float width = 0.9f;
	color primary, secondary;

	Assert( sm->num_arcs > 0 );

	if (Interp_flags & MR_SHOW_OUTLINE_PRESET) {
		return;
	}

	if ( !Interp_lightning ) {
		return;
	}

	// try and scale the size a bit so that it looks equally well on smaller vessels
	if (pm->rad < 500.0f) {
		width *= (pm->rad * 0.01f);

		if (width < 0.2f)
			width = 0.2f;
	}

	for (i=0; i<sm->num_arcs; i++ )	{
		// pick a color based upon arc type
		switch(sm->arc_type[i]){
		// "normal", FreeSpace 1 style arcs
		case MARC_TYPE_NORMAL:
			if ( (rand()>>4) & 1 )	{
				gr_init_color(&primary, 64, 64, 255);
			} else {
				gr_init_color(&primary, 128, 128, 255);
			}

			gr_init_color(&secondary, 200, 200, 255);
			break;

		// "EMP" style arcs
		case MARC_TYPE_EMP:
			if ( (rand()>>4) & 1 )	{
				gr_init_color(&primary, AR, AG, AB);
			} else {
				gr_init_color(&primary, AR2, AG2, AB2);
			}

			gr_init_color(&secondary, 255, 255, 10);
			break;

		default:
			Int3();
		}

		// render the actual arc segment
		interp_render_arc( &sm->arc_pts[i][0], &sm->arc_pts[i][1], &primary, &secondary, width );
	}
}

void model_interp_subcall(polymodel * pm, int mn, int detail_level)
{
	int i;
	int zbuf_mode = gr_zbuffering_mode;

	if ( (mn < 0) || (mn>=pm->n_models) )
		return;

	Assert( mn >= 0 );
	Assert( mn < pm->n_models );

	if (pm->submodel[mn].blown_off){
		return;
	}

	if (pm->submodel[mn].is_thruster )	{
		if ( !(Interp_flags & MR_SHOW_THRUSTERS) ){
			return;
		}
		Interp_thrust_scale_subobj=1;
	} else {
		Interp_thrust_scale_subobj=0;
	}

	// Compute final submodel orientation by using the orientation matrix and the rotation angles.
	// By using this kind of computation, the rotational angles can always be computed relative
	// to the submodel itself, instead of relative to the parent - KeldorKatarn
	matrix rotation_matrix = pm->submodel[mn].orientation;
	vm_rotate_matrix_by_angles(&rotation_matrix, &pm->submodel[mn].angs);

	matrix inv_orientation;
	vm_copy_transpose(&inv_orientation, &pm->submodel[mn].orientation);

	matrix submodel_matrix;
	vm_matrix_x_matrix(&submodel_matrix, &rotation_matrix, &inv_orientation);

	g3_start_instance_matrix(&pm->submodel[mn].offset, &submodel_matrix, true);

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_rotate_all();
	}

	model_interp_sub( pm->submodel[mn].bsp_data, pm, &pm->submodel[mn], 0 );

	if (Interp_flags & MR_DEPRECATED_SHOW_PIVOTS )
		model_draw_debug_points( pm, &pm->submodel[mn], Interp_flags );

	if ( pm->submodel[mn].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[mn]);
	}

	i = pm->submodel[mn].first_child;
	while( i >= 0 )	{
		if (!pm->submodel[i].is_thruster )	{
			if(Interp_flags & MR_NO_ZBUFFER){
				zbuf_mode = GR_ZBUFF_NONE;
			} else {
				zbuf_mode = GR_ZBUFF_FULL;		// read only
			}

			gr_zbuffer_set(zbuf_mode);

			model_interp_subcall( pm, i, detail_level );
		}
		i = pm->submodel[i].next_sibling;
	}

	g3_done_instance(true);
}

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


/**
 * Calls the object interpreter to render an object.  
 * @return true if drawn
 */
int model_interp_sub(void *model_ptr, polymodel * pm, bsp_info *sm, int do_box_check )
{
	ubyte *p = (ubyte *)model_ptr;
	int chunk_type, chunk_size;
	int pushed = 0;

	chunk_type = w(p);
	chunk_size = w(p+4);

	
	while ( chunk_type != OP_EOF )	{

		switch (chunk_type) {
		case OP_DEFPOINTS:		model_interp_defpoints(p,pm,sm); break;
		case OP_FLATPOLY:		model_interp_flatpoly(p,pm); break;
		case OP_TMAPPOLY:		model_interp_tmappoly(p,pm); break;
		case OP_SORTNORM:		model_interp_sortnorm(p,pm,sm,do_box_check); break;
	
		case OP_BOUNDBOX:		

			if ( do_box_check )	{
				int retval = interp_box_offscreen( vp(p+8), vp(p+20) );
				switch( retval )	{
				case IBOX_ALL_OFF:
					goto DoneWithThis;	// Don't need to draw any more polys from this box
					break;

				case IBOX_ALL_ON:
					do_box_check = 0;		// Don't need to check boxes any more
					break;

				case IBOX_SOME_ON_SOME_OFF:
					// continue like we were
					break;
				default:
					Int3();
				}
			}

			if (Interp_flags & MR_DEPRECATED_SHOW_PIVOTS )	{
				#ifndef NDEBUG
				modelstats_num_boxes++;
				#endif
			}

			if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
				if ( pushed )	{
					light_filter_pop();
					pushed = 0;

				}
				light_filter_push_box( vp(p+8), vp(p+20) );
				pushed = 1;
			}
			break;

		default:
			mprintf(( "Bad chunk type %d, len=%d in model_interp_sub\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
	}

DoneWithThis:

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		if ( pushed )	{
			light_filter_pop();
			pushed = 0;
		}
	}

	return 1;
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

void model_render_insignias(polymodel *pm, int detail_level, int bitmap_num)
{
	// if the model has no insignias, or we don't have a texture, then bail
	if ( (pm->num_ins <= 0) || (bitmap_num < 0) )
		return;

	int idx, s_idx;
	vertex vecs[3];
	vertex *vlist[3] = { &vecs[0], &vecs[1], &vecs[2] };
	vec3d t1, t2, t3;
	int i1, i2, i3;
	int tmap_flags = TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT;

	// set the proper texture	
	gr_set_bitmap(bitmap_num, GR_ALPHABLEND_NONE, GR_BITBLT_MODE_NORMAL, 0.65f);

	// otherwise render them	
	for(idx=0; idx<pm->num_ins; idx++){	
		// skip insignias not on our detail level
		if(pm->ins[idx].detail_level != detail_level){
			continue;
		}

		for(s_idx=0; s_idx<pm->ins[idx].num_faces; s_idx++){
			// get vertex indices
			i1 = pm->ins[idx].faces[s_idx][0];
			i2 = pm->ins[idx].faces[s_idx][1];
			i3 = pm->ins[idx].faces[s_idx][2];

			// transform vecs and setup vertices
			vm_vec_add(&t1, &pm->ins[idx].vecs[i1], &pm->ins[idx].offset);
			vm_vec_add(&t2, &pm->ins[idx].vecs[i2], &pm->ins[idx].offset);
			vm_vec_add(&t3, &pm->ins[idx].vecs[i3], &pm->ins[idx].offset);

			if(Cmdline_nohtl){
				g3_rotate_vertex(&vecs[0], &t1);
				g3_rotate_vertex(&vecs[1], &t2);
				g3_rotate_vertex(&vecs[2], &t3);
			}else{
				g3_transfer_vertex(&vecs[0], &t1);
				g3_transfer_vertex(&vecs[1], &t2);
				g3_transfer_vertex(&vecs[2], &t3);
			}

			// setup texture coords
			vecs[0].texture_position.u = pm->ins[idx].u[s_idx][0];
			vecs[0].texture_position.v = pm->ins[idx].v[s_idx][0];

			vecs[1].texture_position.u = pm->ins[idx].u[s_idx][1];
			vecs[1].texture_position.v = pm->ins[idx].v[s_idx][1];

			vecs[2].texture_position.u = pm->ins[idx].u[s_idx][2];
			vecs[2].texture_position.v = pm->ins[idx].v[s_idx][2];

			if (!Cmdline_nohtl) {
				light_apply_rgb( &vecs[0].r, &vecs[0].g, &vecs[0].b, &pm->ins[idx].vecs[i1], &pm->ins[idx].norm[i1], 1.5f );
				light_apply_rgb( &vecs[1].r, &vecs[1].g, &vecs[1].b, &pm->ins[idx].vecs[i2], &pm->ins[idx].norm[i2], 1.5f );
				light_apply_rgb( &vecs[2].r, &vecs[2].g, &vecs[2].b, &pm->ins[idx].vecs[i3], &pm->ins[idx].norm[i3], 1.5f );
				tmap_flags |= (TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD);
			}

			// draw the polygon
			g3_draw_poly(3, vlist, tmap_flags);
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
		
	sa = (float)sin(angle);
	ca = (float)cos(angle);

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

void model_render_DEPRECATED(int model_num, matrix *orient, vec3d * pos, uint flags, int objnum, int lighting_skip, int *replacement_textures, int render, const bool is_skybox)
{
	int cull = 0;
	// replacement textures - Goober5000
	model_set_replacement_textures(replacement_textures);

	polymodel *pm = model_get(model_num);

	model_do_dumb_rotation(model_num);

	if (flags & MR_FORCE_CLAMP)
		gr_set_texture_addressing(TMAP_ADDRESS_CLAMP);

	int time = timestamp();

	glow_point_bank_override *gpo = NULL;
	bool override_all = false;
	SCP_unordered_map<int, void*>::iterator gpoi;
	ship_info *sip = NULL;
	ship *shipp = NULL;
	
	if(!Glowpoint_override) {
		if(objnum>-1)
		{
			shipp = &Ships[Objects[objnum].instance];
			sip = &Ship_info[shipp->ship_info_index];
			gpoi = sip->glowpoint_bank_override_map.find(-1);
		
			if(gpoi != sip->glowpoint_bank_override_map.end()) {
				override_all = true;
				gpo = (glow_point_bank_override*) sip->glowpoint_bank_override_map[-1];
			}
		}
	
		for (int i = 0; i < pm->n_glow_point_banks; i++ ) { //glow point blink code -Bobboau
			glow_point_bank *bank = &pm->glow_point_banks[i];
		
			if(!override_all && sip) {
				gpoi = sip->glowpoint_bank_override_map.find(i);
				if(gpoi != sip->glowpoint_bank_override_map.end()) {
					gpo = (glow_point_bank_override*) sip->glowpoint_bank_override_map[i];
				} else {
					gpo = NULL;
				}
			}

			if (bank->glow_timestamp == 0)
				bank->glow_timestamp=time;
			if(((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time)){
				if(bank->is_on){
					if( ((gpo && gpo->on_time_override)?gpo->on_time:bank->on_time) > ((time - ((gpo && gpo->disp_time_override)?gpo->disp_time:bank->disp_time)) % (((gpo && gpo->on_time_override)?gpo->on_time:bank->on_time) + ((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time))) ){
						bank->glow_timestamp=time;
						bank->is_on=false;
					}
				}else{
					if( ((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time) < ((time - ((gpo && gpo->disp_time_override)?gpo->disp_time:bank->disp_time)) % (((gpo && gpo->on_time_override)?gpo->on_time:bank->on_time) + ((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time))) ){
						bank->glow_timestamp=time;
						bank->is_on=true;
					}
				}
			}
		}
	}

	// maybe turn off (hardware) culling
	if(flags & MR_NO_CULL){
		cull = gr_set_cull(0);
	}

	// Goober5000
	Interp_base_frametime = 0;

	if (objnum >= 0) {
		object *objp = &Objects[objnum];

		if (objp->type == OBJ_SHIP) {
			Interp_base_frametime = Ships[objp->instance].base_texture_anim_frametime;
		}
	} else if (is_skybox) {
		Interp_base_frametime = Skybox_timestamp;
	}


	if (flags & MR_ATTACHED_MODEL)
		Interp_objnum = -1;
	else
		Interp_objnum = objnum;

	if ( flags & MR_NO_LIGHTING )	{
		Interp_light = 1.0f;
	} else if ( flags & MR_IS_ASTEROID ) {
		// Dim it based on distance
		float depth = vm_vec_dist_quick( pos, &Eye_position );
		if ( depth > Interp_depth_scale )	{
			Interp_light = Interp_depth_scale/depth;
			// If it is too far, exit
			if ( Interp_light < (1.0f/32.0f) ) {
				Interp_light = 0.0f;
				return;
			} else if ( Interp_light > 1.0f )	{
				Interp_light = 1.0f;
			}
		} else {
			Interp_light = 1.0f;
		}
	} else {
		Interp_light = 1.0f;
	}

	gr_set_light_factor(Interp_light);

	extern bool Deferred_lighting;

	if ( !(flags & MR_NO_LIGHTING ) && !Deferred_lighting )	{
		light_filter_push( objnum, pos, pm->rad );
	}

	model_really_render(model_num, orient, pos, flags, render, objnum);

	if ( !(flags & MR_NO_LIGHTING ) && !Deferred_lighting )	{
		light_filter_pop();
	}

	// maybe turn culling back on
	if(flags & MR_NO_CULL){
		gr_set_cull(cull);
	}

	// turn off fog after each model renders
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

	if (flags & MR_FORCE_CLAMP)
		gr_set_texture_addressing(TMAP_ADDRESS_WRAP);
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
 * Find the distance from p0 to the closest point on a box.
 * The box's dimensions from 'min' to 'max'.
 */
float interp_closest_dist_to_box( vec3d *hitpt, vec3d *p0, vec3d *min, vec3d *max )
{
	float *origin = (float *)&p0->xyz.x;
	float *minB = (float *)min;
	float *maxB = (float *)max;
	float *coord = (float *)&hitpt->xyz.x;
	int inside = 1;
	int i;

	for (i=0; i<3; i++ )	{
		if ( origin[i] < minB[i] )	{
			coord[i] = minB[i];
			inside = 0;
		} else if (origin[i] > maxB[i] )	{
			coord[i] = maxB[i];
			inside = 0;
		} else {
			coord[i] = origin[i];
		}
	}
	
	if ( inside )	{
		return 0.0f;
	}

	return vm_vec_dist(hitpt,p0);
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
//   Also fills in outpnt with the actual closest point.
float model_find_closest_point( vec3d *outpnt, int model_num, int submodel_num, matrix *orient, vec3d * pos, vec3d *eye_pos )
{
	vec3d closest_pos, tempv, eye_rel_pos;
	
	polymodel *pm = model_get(model_num);

	if ( submodel_num < 0 )	{
		 submodel_num = pm->detail[0];
	}

	// Rotate eye pos into object coordinates
	vm_vec_sub(&tempv,pos,eye_pos );
	vm_vec_rotate(&eye_rel_pos,&tempv,orient );

	return interp_closest_dist_to_box( &closest_pos, &eye_rel_pos, &pm->submodel[submodel_num].min, &pm->submodel[submodel_num].max );
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

void moldel_calc_facing_pts( vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w, float z_add, vec3d *Eyeposition )
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

extern bool Scene_framebuffer_in_frame;

/**
 * Maybe draw mode thruster glows
 */
void model_render_thrusters(polymodel *pm, int objnum, ship *shipp, matrix *orient, vec3d *pos)
{
	int i, j;
	int n_q = 0;
	size_t 	k;
	vec3d norm, norm2, fvec, pnt, npnt;
	thruster_bank *bank = NULL;
	vertex p;
	bool do_render = false;

	if ( pm == NULL ) {
		Int3();
		return;
	}

	if ( !(Interp_flags & MR_SHOW_THRUSTERS) ) {
		return;
	}

	// get an initial count to figure out how man geo batchers we need allocated
	for (i = 0; i < pm->n_thrusters; i++ ) {
		bank = &pm->thrusters[i];
		n_q += bank->num_points;
	}

	if (n_q <= 0) {
		return;
	}

	// primary_thruster_batcher
	if (Interp_thrust_glow_bitmap >= 0) {
		do_render = true;
	}

	// secondary_thruster_batcher
	if (Interp_secondary_thrust_glow_bitmap >= 0) {
		do_render = true;
	}

	// tertiary_thruster_batcher
	if (Interp_tertiary_thrust_glow_bitmap >= 0) {
		do_render = true;
	}

	if (do_render == false) {
		return;
	}

	// this is used for the secondary thruster glows 
	// it only needs to be calculated once so I'm doing it here -Bobboau
	norm.xyz.z = -1.0f;
	norm.xyz.x = 1.0f;
	norm.xyz.y = -1.0f;
	norm.xyz.x *= Interp_thrust_rotvel.xyz.y/2;
	norm.xyz.y *= Interp_thrust_rotvel.xyz.x/2;
	vm_vec_normalize(&norm);

	// we need to disable fogging
	if (The_mission.flags & MISSION_FLAG_FULLNEB)
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);

	for (i = 0; i < pm->n_thrusters; i++ ) {
		vec3d submodel_static_offset; // The associated submodel's static offset in the ship's frame of reference
		bool submodel_rotation = false;

		bank = &pm->thrusters[i];

		// don't draw this thruster if the engine is destroyed or just not on
		if ( !model_should_render_engine_glow(objnum, bank->obj_num) )
			continue;

		// If bank is attached to a submodel, prepare to account for rotations
		//
		// TODO: This won't work in the ship lab, because the lab code doesn't
		// set the the necessary submodel instance info needed here. The second
		// condition is thus a hack to disable the feature while in the lab, and
		// can be removed if the lab is re-structured accordingly. -zookeeper
		if ( shipp && bank->submodel_num > -1 && pm->submodel[bank->submodel_num].can_move && (gameseq_get_state_idx(GS_STATE_LAB) == -1) ) {
			model_find_submodel_offset(&submodel_static_offset, Ship_info[shipp->ship_info_index].model_num, bank->submodel_num);

			submodel_rotation = true;
		}

		for (j = 0; j < bank->num_points; j++) {
			Assert( bank->points != NULL );

			float d, D;
			vec3d tempv;
			glow_point *gpt = &bank->points[j];
			vec3d loc_offset = gpt->pnt;
			vec3d loc_norm = gpt->norm;
			vec3d world_pnt;
			vec3d world_norm;

			if ( submodel_rotation ) {
				vm_vec_sub(&loc_offset, &gpt->pnt, &submodel_static_offset);

				tempv = loc_offset;
				find_submodel_instance_point_normal(&loc_offset, &loc_norm, &Objects[objnum], bank->submodel_num, &tempv, &loc_norm);
			}

			vm_vec_unrotate(&world_pnt, &loc_offset, orient);
			vm_vec_add2(&world_pnt, pos);

			if (shipp) {
				// if ship is warping out, check position of the engine glow to the warp plane
				if ( (shipp->flags & (SF_ARRIVING) ) && (shipp->warpin_effect) && Ship_info[shipp->ship_info_index].warpin_type != WT_HYPERSPACE) {
					vec3d warp_pnt, tmp;
					matrix warp_orient;

					shipp->warpin_effect->getWarpPosition(&warp_pnt);
					shipp->warpin_effect->getWarpOrientation(&warp_orient);
					vm_vec_sub( &tmp, &world_pnt, &warp_pnt );

					if ( vm_vec_dot( &tmp, &warp_orient.vec.fvec ) < 0.0f ) {
						break;
					}
				}

				if ( (shipp->flags & (SF_DEPART_WARP) ) && (shipp->warpout_effect) && Ship_info[shipp->ship_info_index].warpout_type != WT_HYPERSPACE) {
					vec3d warp_pnt, tmp;
					matrix warp_orient;

					shipp->warpout_effect->getWarpPosition(&warp_pnt);
					shipp->warpout_effect->getWarpOrientation(&warp_orient);
					vm_vec_sub( &tmp, &world_pnt, &warp_pnt );

					if ( vm_vec_dot( &tmp, &warp_orient.vec.fvec ) > 0.0f ) {
						break;
					}
				}
			}

			vm_vec_sub(&tempv, &View_position, &world_pnt);
			vm_vec_normalize(&tempv);
			vm_vec_unrotate(&world_norm, &loc_norm, orient);
			D = d = vm_vec_dot(&tempv, &world_norm);

			// ADAM: Min throttle draws rad*MIN_SCALE, max uses max.
			#define NOISE_SCALE 0.5f
			#define MIN_SCALE 3.4f
			#define MAX_SCALE 4.7f

			float magnitude;
			vec3d scale_vec = { { { 1.0f, 0.0f, 0.0f } } };

			// normalize banks, in case of incredibly big normals
			if ( !IS_VEC_NULL_SQ_SAFE(&world_norm) )
				vm_vec_copy_normalize(&scale_vec, &world_norm);

			// adjust for thrust
			(scale_vec.xyz.x *= Interp_thrust_scale_x) -= 0.1f;
			(scale_vec.xyz.y *= Interp_thrust_scale_y) -= 0.1f;
			(scale_vec.xyz.z *= Interp_thrust_scale)   -= 0.1f;

			// get magnitude, which we will use as the scaling reference
			magnitude = vm_vec_normalize(&scale_vec);

			// get absolute value
			if (magnitude < 0.0f)
				magnitude *= -1.0f;

			float scale = magnitude * (MAX_SCALE - MIN_SCALE) + MIN_SCALE;

			if (d > 0.0f){
				// Make glow bitmap fade in/out quicker from sides.
				d *= 3.0f;

				if (d > 1.0f)
					d = 1.0f;
			}

			float fog_int = 1.0f;

			// fade them in the nebula as well
			if (The_mission.flags & MISSION_FLAG_FULLNEB) {
				vm_vec_unrotate(&npnt, &gpt->pnt, orient);
				vm_vec_add2(&npnt, pos);

				fog_int = (1.0f - (neb2_get_fog_intensity(&npnt)));

				if (fog_int > 1.0f)
					fog_int = 1.0f;

				d *= fog_int;

				if (d > 1.0f)
					d = 1.0f;
			}

			float w = gpt->radius * (scale + Interp_thrust_glow_noise * NOISE_SCALE);

			// these lines are used by the tertiary glows, thus we will need to project this all of the time
			if (Cmdline_nohtl) {
				g3_rotate_vertex( &p, &world_pnt );
			} else {
				g3_transfer_vertex( &p, &world_pnt );
			}

			// start primary thruster glows
			if ( (Interp_thrust_glow_bitmap >= 0) && (d > 0.0f) ) {
				p.r = p.g = p.b = p.a = (ubyte)(255.0f * d);
				batch_add_bitmap(
					Interp_thrust_glow_bitmap, 
					TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT | TMAP_FLAG_SOFT_QUAD, 
					&p,
					0,
					(w * 0.5f * Interp_thrust_glow_rad_factor),
					1.0f,
					(w * 0.325f)
				);
			}

			// start tertiary thruster glows
			if (Interp_tertiary_thrust_glow_bitmap >= 0) {
				p.screen.xyw.w -= w;
				p.r = p.g = p.b = p.a = (ubyte)(255.0f * fog_int);
				batch_add_bitmap_rotated(
					Interp_tertiary_thrust_glow_bitmap,
					TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT | TMAP_FLAG_SOFT_QUAD,
					&p,
					(magnitude * 4),
					(w * 0.6f * Interp_tertiary_thrust_glow_rad_factor),
					1.0f,
					(-(D > 0) ? D : -D)
				);
			}

			// begin secondary glows
			if (Interp_secondary_thrust_glow_bitmap >= 0) {
				pnt = world_pnt;
				scale = magnitude * (MAX_SCALE - (MIN_SCALE / 2)) + (MIN_SCALE / 2);
				vm_vec_unrotate(&world_norm, &norm, orient);
				d = vm_vec_dot(&tempv, &world_norm);
				d += 0.75f;
				d *= 3.0f;

				if (d > 1.0f)
					d = 1.0f;

				if (d > 0.0f) {
					vm_vec_add(&norm2, &world_norm, &pnt);
					vm_vec_sub(&fvec, &norm2, &pnt);
					vm_vec_normalize(&fvec);

					float wVal = gpt->radius * scale * 2;

					vm_vec_scale_add(&norm2, &pnt, &fvec, wVal * 2 * Interp_thrust_glow_len_factor);

					if (The_mission.flags & MISSION_FLAG_FULLNEB) {
						vm_vec_add(&npnt, &pnt, pos);
						d *= fog_int;
					}

					batch_add_beam(Interp_secondary_thrust_glow_bitmap,
							TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT,
							&pnt, &norm2, wVal*Interp_secondary_thrust_glow_rad_factor*0.5f, d
					);
					if (Scene_framebuffer_in_frame && Interp_draw_distortion) {
						vm_vec_scale_add(&norm2, &pnt, &fvec, wVal * 2 * Interp_distortion_thrust_length_factor);
						int dist_bitmap;
						if (Interp_distortion_thrust_bitmap > 0) {
							dist_bitmap = Interp_distortion_thrust_bitmap;
						}
						else {
							dist_bitmap = Interp_secondary_thrust_glow_bitmap;
						}
						float mag = vm_vec_mag(&gpt->pnt); 
						mag -= (float)((int)mag);//Valathil - Get a fairly random but constant number to offset the distortion texture
						distortion_add_beam(dist_bitmap,
							TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT | TMAP_FLAG_DISTORTION_THRUSTER | TMAP_FLAG_SOFT_QUAD,
							&pnt, &norm2, wVal*Interp_distortion_thrust_rad_factor*0.5f, 1.0f, mag
						);
					}
				}
			}

			// begin particles
			if (shipp) {
				ship_info *sip = &Ship_info[shipp->ship_info_index];
				particle_emitter pe;
				thruster_particles *tp;
				size_t num_particles = 0;

				if (Interp_afterburner)
					num_particles = sip->afterburner_thruster_particles.size();
				else
					num_particles = sip->normal_thruster_particles.size();

				for (k = 0; k < num_particles; k++) {
					if (Interp_afterburner)
						tp = &sip->afterburner_thruster_particles[k];
					else
						tp = &sip->normal_thruster_particles[k];

					float v = vm_vec_mag_quick(&Objects[shipp->objnum].phys_info.desired_vel);

					vm_vec_unrotate(&npnt, &gpt->pnt, orient);
					vm_vec_add2(&npnt, pos);

					// Where the particles emit from
					pe.pos = npnt;
					// Initial velocity of all the particles
					pe.vel = Objects[shipp->objnum].phys_info.desired_vel;
					pe.min_vel = v * 0.75f;
					pe.max_vel =  v * 1.25f;
					// What normal the particle emit around
					pe.normal = orient->vec.fvec;
					vm_vec_negate(&pe.normal);

					// Lowest number of particles to create
					pe.num_low = tp->n_low;
					// Highest number of particles to create
					pe.num_high = tp->n_high;
					pe.min_rad = gpt->radius * tp->min_rad;
					pe.max_rad = gpt->radius * tp->max_rad;
					// How close they stick to that normal 0=on normal, 1=180, 2=360 degree
					pe.normal_variance = tp->variance;
					pe.min_life = 0.0f;
					pe.max_life = 1.0f;

					particle_emit( &pe, PARTICLE_BITMAP, tp->thruster_bitmap.first_frame);
				}
			}
		}
	}

	// save current zbuffer, and set the correct mode for us
	int zbuff_save = gr_zbuffering_mode;
	gr_zbuffer_set(GR_ZBUFF_READ);
	gr_zbuffer_set(zbuff_save);
}

void model_render_glow_points_DEPRECATED(polymodel *pm, ship *shipp, matrix *orient, vec3d *pos, bool use_depth_buffer = true)
{
	int i, j;

	int cull = gr_set_cull(0);
	
	glow_point_bank_override *gpo = NULL;
	bool override_all = false;
	SCP_unordered_map<int, void*>::iterator gpoi;
	ship_info *sip = NULL;

	if(shipp)
	{
		sip = &Ship_info[shipp->ship_info_index];
		gpoi = sip->glowpoint_bank_override_map.find(-1);
		
		if(gpoi != sip->glowpoint_bank_override_map.end()) {
			override_all = true;
			gpo = (glow_point_bank_override*) sip->glowpoint_bank_override_map[-1];
		}
	}
	
	for (i = 0; i < pm->n_glow_point_banks; i++ ) {
		glow_point_bank *bank = &pm->glow_point_banks[i];
		
		if(!override_all && sip) {
			gpoi = sip->glowpoint_bank_override_map.find(i);
			if(gpoi != sip->glowpoint_bank_override_map.end()) {
				gpo = (glow_point_bank_override*) sip->glowpoint_bank_override_map[i];
			} else {
				gpo = NULL;
			}
		}

		//Only continue if there actually is a glowpoint bitmap available
		if (bank->glow_bitmap == -1)
			continue;

		if (pm->submodel[bank->submodel_parent].blown_off)
			continue;

		if ((gpo && gpo->off_time_override && !gpo->off_time)?gpo->is_on:bank->is_on) {
			if ( (shipp != NULL) && !(shipp->glow_point_bank_active[i]) )
				continue;

			for (j = 0; j < bank->num_points; j++) {
				Assert( bank->points != NULL );
				int flick;

				if (pm->submodel[pm->detail[0]].num_arcs) {
					flick = static_rand( timestamp() % 20 ) % (pm->submodel[pm->detail[0]].num_arcs + j); //the more damage, the more arcs, the more likely the lights will fail
				} else {
					flick = 1;
				}

				if (flick == 1) {
					glow_point *gpt = &bank->points[j];
                    vec3d loc_offset = gpt->pnt;
                    vec3d loc_norm = gpt->norm;
                    vec3d world_pnt;
                    vec3d world_norm;
                    vec3d tempv;
                    vec3d submodel_static_offset; // The associated submodel's static offset in the ship's frame of reference
                    bool submodel_rotation = false;

					if ( bank->submodel_parent > 0 && pm->submodel[bank->submodel_parent].can_move && (gameseq_get_state_idx(GS_STATE_LAB) == -1) && shipp != NULL ) {
						model_find_submodel_offset(&submodel_static_offset, Ship_info[shipp->ship_info_index].model_num, bank->submodel_parent);

						submodel_rotation = true;
					}

					if ( submodel_rotation ) {
						vm_vec_sub(&loc_offset, &gpt->pnt, &submodel_static_offset);

						tempv = loc_offset;
						find_submodel_instance_point_normal(&loc_offset, &loc_norm, &Objects[shipp->objnum], bank->submodel_parent, &tempv, &loc_norm);
					}

					vm_vec_unrotate(&world_pnt, &loc_offset, orient);
					vm_vec_add2(&world_pnt, pos);

					vm_vec_unrotate(&world_norm, &loc_norm, orient);

					if ( shipp != NULL ) {
						if ( (shipp->flags & (SF_ARRIVING) ) && (shipp->warpin_effect) && Ship_info[shipp->ship_info_index].warpin_type != WT_HYPERSPACE) {
							vec3d warp_pnt, tmp;
							matrix warp_orient;

							shipp->warpin_effect->getWarpPosition(&warp_pnt);
							shipp->warpin_effect->getWarpOrientation(&warp_orient);
							vm_vec_sub( &tmp, &world_pnt, &warp_pnt );

							if ( vm_vec_dot( &tmp, &warp_orient.vec.fvec ) < 0.0f ) {
								continue;
							}
						}

						if ( (shipp->flags & (SF_DEPART_WARP) ) && (shipp->warpout_effect) && Ship_info[shipp->ship_info_index].warpout_type != WT_HYPERSPACE) {
							vec3d warp_pnt, tmp;
							matrix warp_orient;

							shipp->warpout_effect->getWarpPosition(&warp_pnt);
							shipp->warpout_effect->getWarpOrientation(&warp_orient);
							vm_vec_sub( &tmp, &world_pnt, &warp_pnt );

							if ( vm_vec_dot( &tmp, &warp_orient.vec.fvec ) > 0.0f ) {
								continue;
							}
						}
					}

					switch ((gpo && gpo->type_override)?gpo->type:bank->type)
					{
						case 0:
						{
							float d,pulse = 1.0f;

							if ( IS_VEC_NULL(&world_norm) ) {
								d = 1.0f;	//if given a nul vector then always show it
							} else {
								vm_vec_sub(&tempv,&View_position,&world_pnt);
								vm_vec_normalize(&tempv);

								d = vm_vec_dot(&tempv,&world_norm);
								d -= 0.25;	
							}
							
							float w = gpt->radius;
							if (d > 0.0f) {
								vertex p;

								d *= 3.0f;

								if (d > 1.0f)
									d = 1.0f;


								// fade them in the nebula as well
								if (The_mission.flags & MISSION_FLAG_FULLNEB) {
									//vec3d npnt;
									//vm_vec_add(&npnt, &loc_offset, pos);

									d *= (1.0f - neb2_get_fog_intensity(&world_pnt));
									w *= 1.5;	//make it bigger in a nebula
								}
				
								// disable fogging
								if (Interp_tmap_flags & TMAP_FLAG_PIXEL_FOG)
									gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	
								if (!Cmdline_nohtl) {
									g3_transfer_vertex(&p, &world_pnt);
								} else {
									g3_rotate_vertex(&p, &world_pnt);
								}

								p.r = p.g = p.b = p.a = (ubyte)(255.0f * MAX(d,0.0f));
								
								if((gpo && gpo->glow_bitmap_override)?(gpo->glow_bitmap > -1):(bank->glow_bitmap > -1)) {
									int gpflags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT;
									if (use_depth_buffer)
										gpflags |= TMAP_FLAG_SOFT_QUAD;
								
									batch_add_bitmap(
										(gpo && gpo->glow_bitmap_override)?gpo->glow_bitmap:bank->glow_bitmap,
										gpflags,  
										&p,
										0,
										(w * 0.5f),
										d * pulse,
										w
									);
								}
							} //d>0.0f
							if(gpo && gpo->pulse_type) {
								int period;
								if(gpo->pulse_period_override) {
									period = gpo->pulse_period;
								} else {
									if(gpo->on_time_override) {
										period = 2 * gpo->on_time;
									} else {
										period = 2 * bank->on_time;
									}
								}
								int x = 0;
								if((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time) {
									x = (timestamp() - ((gpo && gpo->disp_time_override)?gpo->disp_time:bank->disp_time)) % ( ((gpo && gpo->on_time_override)?gpo->on_time:bank->on_time) + ((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time) ) - ((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time);
								} else {
									x = (timestamp() - ((gpo && gpo->disp_time_override)?gpo->disp_time:bank->disp_time)) % gpo->pulse_period;
								}
								switch(gpo->pulse_type) {
									case PULSE_SIN:
										pulse = gpo->pulse_bias + gpo->pulse_amplitude * pow(sin( PI2 / period * x),gpo->pulse_exponent);
										break;
									case PULSE_COS:
										pulse = gpo->pulse_bias + gpo->pulse_amplitude * pow(cos( PI2 / period * x),gpo->pulse_exponent);
										break;
									case PULSE_SHIFTTRI:
										x += period / 4;
										if((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time) {
											x %= ( ((gpo && gpo->on_time_override)?gpo->on_time:bank->on_time) + ((gpo && gpo->off_time_override)?gpo->off_time:bank->off_time) );
										} else {
											x %= gpo->pulse_period;
										}
									case PULSE_TRI:
										float inv;
										if( x > period / 2) {
											inv = -1;
										} else {
											inv = 1;
										}
										if( x > period / 4) {
											pulse = gpo->pulse_bias + gpo->pulse_amplitude * inv * pow( 1.0f - ((x - period / 4.0f) * 4 / period) ,gpo->pulse_exponent);
										} else {
											pulse = gpo->pulse_bias + gpo->pulse_amplitude * inv * pow( (x * 4.0f / period) ,gpo->pulse_exponent);
										}
										break;
								}
							}
							extern bool Deferred_lighting;
							if(Deferred_lighting && gpo && gpo->is_lightsource)	{
								if(gpo->lightcone) {
									vec3d cone_dir_rot;
									vec3d cone_dir_model;
									vec3d cone_dir_world;
									vec3d cone_dir_screen;
									vec3d unused;
									if(gpo->rotating) {
										vm_rot_point_around_line(&cone_dir_rot, &gpo->cone_direction, PI * timestamp() * 0.000033333f * gpo->rotation_speed, &vmd_zero_vector, &gpo->rotation_axis);
									} else {
										cone_dir_rot = gpo->cone_direction; 
									}
									find_submodel_instance_point_normal(&unused, &cone_dir_model, &Objects[shipp->objnum], bank->submodel_parent, &unused, &cone_dir_rot);
									vm_vec_unrotate(&cone_dir_world, &cone_dir_model, orient);
									vm_vec_rotate(&cone_dir_screen, &cone_dir_world, &Eye_matrix);
									cone_dir_screen.xyz.z = -cone_dir_screen.xyz.z;
									light_add_cone(&world_pnt, &cone_dir_screen, gpo->cone_angle, gpo->cone_inner_angle, gpo->dualcone, 1.0f, w * gpo->radius_multi, 1, pulse * gpo->light_color.xyz.x + (1.0f-pulse) * gpo->light_mix_color.xyz.x, pulse * gpo->light_color.xyz.y + (1.0f-pulse) * gpo->light_mix_color.xyz.y, pulse * gpo->light_color.xyz.z + (1.0f-pulse) * gpo->light_mix_color.xyz.z, -1);
								} else {
									light_add_point(&world_pnt, 1.0f, w * gpo->radius_multi, 1, pulse * gpo->light_color.xyz.x + (1.0f-pulse) * gpo->light_mix_color.xyz.x, pulse * gpo->light_color.xyz.y + (1.0f-pulse) * gpo->light_mix_color.xyz.y, pulse * gpo->light_color.xyz.z + (1.0f-pulse) * gpo->light_mix_color.xyz.z, -1);
								}
							}
							break;
						}

						case 1:
						{
							vertex verts[4];
							vec3d fvec, top1, bottom1, top2, bottom2, start, end;
                            
							memset(verts, 0, sizeof(verts));

							vm_vec_add2(&loc_norm, &loc_offset);

							vm_vec_rotate(&start, &loc_offset, orient);
							vm_vec_rotate(&end, &loc_norm, orient);
							vm_vec_sub(&fvec, &end, &start);

							vm_vec_normalize(&fvec);

							moldel_calc_facing_pts(&top1, &bottom1, &fvec, &loc_offset, gpt->radius, 1.0f, &View_position);
							moldel_calc_facing_pts(&top2, &bottom2, &fvec, &loc_norm, gpt->radius, 1.0f, &View_position);

							int idx = 0;

							if (Cmdline_nohtl) {
								g3_rotate_vertex(&verts[0], &bottom1);
								g3_rotate_vertex(&verts[1], &bottom2);
								g3_rotate_vertex(&verts[2], &top2);
								g3_rotate_vertex(&verts[3], &top1);

								for (idx = 0; idx < 4; idx++) {
									g3_project_vertex(&verts[idx]);
								}
							} else {
								g3_transfer_vertex(&verts[0], &bottom1);
								g3_transfer_vertex(&verts[1], &bottom2);
								g3_transfer_vertex(&verts[2], &top2);
								g3_transfer_vertex(&verts[3], &top1);
							}

							verts[0].texture_position.u = 0.0f;
							verts[0].texture_position.v = 0.0f;

							verts[1].texture_position.u = 1.0f;
							verts[1].texture_position.v = 0.0f;

							verts[2].texture_position.u = 1.0f;
							verts[2].texture_position.v = 1.0f;

							verts[3].texture_position.u = 0.0f;
							verts[3].texture_position.v = 1.0f;

							vm_vec_sub(&tempv,&View_position,&loc_offset);
							vm_vec_normalize(&tempv);

							if (The_mission.flags & MISSION_FLAG_FULLNEB) {
								gr_set_bitmap(bank->glow_neb_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		
							} else {
								gr_set_bitmap(bank->glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		
							}
							gr_deferred_lighting_end();
							gr_render(4, verts, TMAP_FLAG_TILED | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT);
							gr_deferred_lighting_begin();
							break;
						}
					} // switch(bank->type)
				} // flick
			} // for slot
		} // bank is on
	} // for bank

	gr_set_cull(cull);
}

void light_set_all_relevent();
extern int Warp_model;
extern bool Rendering_to_shadow_map;

void model_really_render(int model_num, matrix *orient, vec3d * pos, uint flags, int render, int objnum )
{
	int i;
	int cull = 1;
	polymodel * pm;

	uint save_gr_zbuffering_mode;
	int zbuf_mode;
	ship *shipp = NULL;
	object *objp = NULL;
	bool set_autocen = false;
	bool draw_thrusters = false;

	// just to be on the safe side
	// If we're dealing with an attached model (i.e. an external weapon model) we need the object number
	// of the ship that the weapon is attached to, but nothing else
	Assert( (Interp_objnum == objnum) || (flags & MR_DEPRECATED_ATTACHED_MODEL) );

	if (objnum >= 0 && !(flags & MR_DEPRECATED_ATTACHED_MODEL)) {
		objp = &Objects[objnum];

		if (objp->type == OBJ_SHIP) {
			shipp = &Ships[objp->instance];

			if (shipp->flags2 & SF2_GLOWMAPS_DISABLED)
				flags |= MR_DEPRECATED_NO_GLOWMAPS;
		}
	}

	if (FULLCLOAK != 1)
		model_interp_sortnorm = model_interp_sortnorm_b2f;


	MONITOR_INC( NumModelsRend, 1 );	

	Interp_orient = orient;
	Interp_pos = pos;

	int tmp_detail_level = Game_detail_level;

	// Turn off engine effect
	Interp_thrust_scale_subobj=0;

	if (!Model_texturing)
		flags |= MR_DEPRECATED_NO_TEXTURING;

	if ( !Model_polys )	{
		flags |= MR_DEPRECATED_NO_POLYS;
	}

	Interp_flags = flags;			 

	pm = model_get(model_num);	

	// Set the flags we will pass to the tmapper
	Interp_tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB;

	// if we're in nebula mode, fog everything except for the warp holes and other non-fogged models
	if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE) && !(flags & MR_DEPRECATED_NO_FOGGING)){
		Interp_tmap_flags |= TMAP_FLAG_PIXEL_FOG;
	}

	if ( !(Interp_flags & MR_NO_TEXTURING) )	{
		Interp_tmap_flags |= TMAP_FLAG_TEXTURED;

		if ( (pm->flags & PM_FLAG_ALLOW_TILING) && tiling)
			Interp_tmap_flags |= TMAP_FLAG_TILED;

		if ( !(Interp_flags & MR_DEPRECATED_NO_CORRECT) )	{
			Interp_tmap_flags |= TMAP_FLAG_CORRECT;
		}
	}

 	if ( Interp_flags & MR_DEPRECATED_ANIMATED_SHADER )
 		Interp_tmap_flags |= TMAP_ANIMATED_SHADER;

	if ( Interp_desaturate ) {
		Interp_tmap_flags |= TMAP_FLAG_DESATURATE;
	}

	save_gr_zbuffering_mode = gr_zbuffering_mode;
	zbuf_mode = gr_zbuffering_mode;

	if (!(Game_detail_flags & DETAIL_FLAG_MODELS) )	{
		gr_set_color(0,128,0);
		g3_draw_sphere_ez( pos, pm->rad );
		return;
	}

	bool is_outlines_only = (flags & MR_DEPRECATED_NO_POLYS) && ((flags & MR_DEPRECATED_SHOW_OUTLINE_PRESET) || (flags & MR_DEPRECATED_SHOW_OUTLINE));
	bool is_outlines_only_htl = !Cmdline_nohtl && (flags & MR_DEPRECATED_NO_POLYS) && (flags & MR_DEPRECATED_SHOW_OUTLINE_HTL);
	bool use_api = !is_outlines_only_htl || (gr_screen.mode == GR_OPENGL);

	g3_start_instance_matrix(pos, orient, use_api);

 	if ( Interp_flags & MR_DEPRECATED_SHOW_RADIUS )	{
 		if ( !(Interp_flags & MR_DEPRECATED_SHOW_OUTLINE_PRESET) )	{
 			gr_set_color(0,64,0);
 			g3_draw_sphere_ez(&vmd_zero_vector,pm->rad);
 		}
 	}

	Assert( pm->n_detail_levels < MAX_MODEL_DETAIL_LEVELS );

	vec3d closest_pos;
	float depth = model_find_closest_point( &closest_pos, model_num, -1, orient, pos, &Eye_position );


	if ( !(Interp_flags & MR_DEPRECATED_LOCK_DETAIL) ) {
		#if MAX_DETAIL_LEVEL != 4
		#error Code in modelInterp.cpp assumes MAX_DETAIL_LEVEL == 4
		#endif

		switch( Detail.detail_distance )	{
		case 0:		// lowest
			depth /= The_mission.ai_profile->detail_distance_mult[0];
			break;
		case 1:		// lower than normal
			depth /= The_mission.ai_profile->detail_distance_mult[1];
			break;
		case 2:		// default
			depth /= The_mission.ai_profile->detail_distance_mult[2];
			break;
		case 3:		// above normal
			depth /= The_mission.ai_profile->detail_distance_mult[3];
			break;
		case 4:		// even more normal
			depth /= The_mission.ai_profile->detail_distance_mult[4];
			break;
		}

		// If we're rendering attached weapon models, check against the ships' tabled Weapon Model Draw Distance (which defaults to 200)
		if (Interp_flags & MR_DEPRECATED_ATTACHED_MODEL) {
			if (depth > Ship_info[Ships[Objects[objnum].instance].ship_info_index].weapon_model_draw_distance) {
				g3_done_instance(use_api);
				return;
			}
		}
	}
	if ( pm->n_detail_levels > 1 )	{

		if ( Interp_flags & MR_DEPRECATED_LOCK_DETAIL )	{
			i = Interp_detail_level_locked+1;
		} else {
			// nebula ?
			if(The_mission.flags & MISSION_FLAG_FULLNEB){
				depth *= neb2_get_lod_scale(Interp_objnum);
			}

			for ( i=0; i<pm->n_detail_levels; i++ )	{
				if ( depth<=pm->detail_depth[i] ){
					break;
				}
			}

			// If no valid detail depths specified, use highest.
			if ( (i > 1) && (pm->detail_depth[i-1] < 1.0f))	{
				i = 1;
			}
		}

		Interp_detail_level = i-1-tmp_detail_level;

		if ( Interp_detail_level < 0 ) 
			Interp_detail_level = 0;
		else if (Interp_detail_level >= pm->n_detail_levels ) 
			Interp_detail_level = pm->n_detail_levels-1;

	} else {
		Interp_detail_level = 0;
	}

#ifndef NDEBUG
	if ( Interp_detail_level == 0 )	{
		MONITOR_INC( NumHiModelsRend, 1 );
	} else if ( Interp_detail_level == pm->n_detail_levels-1 ) {
		MONITOR_INC( NumLowModelsRend, 1 );
	}  else {
		MONITOR_INC( NumMedModelsRend, 1 );
	}
#endif

	// scale the render box settings based on the "Model Detail" slider
	switch (Detail.detail_distance)
	{
		// 1st dot is 20%
		case 0:
			Interp_box_scale = 0.2f;
			break;

		// 2nd dot is 50%
		case 1:
			Interp_box_scale = 0.5f;
			break;

		// 3rd dot is 80%
		case 2:
			Interp_box_scale = 0.8f;
			break;

		// 4th dot is 100% (this is the default setting for "High" and "Very High" settings)
		case 3:
			Interp_box_scale = 1.0f;
			break;

		// 5th dot (max) is 120%
		case 4:
			Interp_box_scale = 1.2f;
			break;
	}

	vec3d auto_back = ZERO_VECTOR;
	if (Interp_flags & MR_DEPRECATED_AUTOCENTER) {
		// standard autocenter using data in model
		if (pm->flags & PM_FLAG_AUTOCEN) {
			auto_back = pm->autocenter;
			vm_vec_scale(&auto_back, -1.0f);
			set_autocen = true;
		}
		// fake autocenter if we are a missile and don't already have autocen info
		else if (Interp_flags & MR_DEPRECATED_IS_MISSILE) {
            auto_back.xyz.x = -( (pm->submodel[pm->detail[Interp_detail_level]].max.xyz.x + pm->submodel[pm->detail[Interp_detail_level]].min.xyz.x) / 2.0f );
            auto_back.xyz.y = -( (pm->submodel[pm->detail[Interp_detail_level]].max.xyz.y + pm->submodel[pm->detail[Interp_detail_level]].min.xyz.y) / 2.0f );
			auto_back.xyz.z = -( (pm->submodel[pm->detail[Interp_detail_level]].max.xyz.z + pm->submodel[pm->detail[Interp_detail_level]].min.xyz.z) / 2.0f );
			set_autocen = true;
		}

		if (set_autocen)
			g3_start_instance_matrix(&auto_back, NULL, true);
	}

	gr_zbias(1);

	if(Interp_tmap_flags & TMAP_FLAG_PIXEL_FOG)
	{
		float fog_near = 10.0f, fog_far = 1000.0f;
		neb2_get_adjusted_fog_values(&fog_near, &fog_far, objp);
		unsigned char r, g, b;
		neb2_get_fog_color(&r, &g, &b);
		gr_fog_set(GR_FOGMODE_FOG, r, g, b, fog_near, fog_far);
	}

	if (is_outlines_only_htl) {
		gr_set_fill_mode( GR_FILL_MODE_WIRE );
		gr_set_color_fast( &Interp_outline_color );
		// lines shouldn't be rendered with textures or special RGB colors (assuming preset colors)
		Interp_flags |= MR_DEPRECATED_NO_TEXTURING;
		Interp_tmap_flags &= ~TMAP_FLAG_TEXTURED;
		Interp_tmap_flags &= ~TMAP_FLAG_RGB;
		// don't render with lighting either
		Interp_flags |= MR_DEPRECATED_NO_LIGHTING;
	} else {
		gr_set_fill_mode( GR_FILL_MODE_SOLID );
	}

	if ( (Interp_flags & MR_DEPRECATED_NO_ZBUFFER) || (Interp_flags & MR_DEPRECATED_ALL_XPARENT) ) {
		zbuf_mode = GR_ZBUFF_NONE;
	} else {
		zbuf_mode = GR_ZBUFF_FULL;
	}

	gr_zbuffer_set(zbuf_mode);

	if (Interp_flags & MR_DEPRECATED_EDGE_ALPHA) {
		gr_center_alpha(-1);
	} else if (Interp_flags & MR_DEPRECATED_CENTER_ALPHA) {
		gr_center_alpha(1);
	} else {
		gr_center_alpha(0);
	}

	if ( (Interp_flags & MR_DEPRECATED_NO_CULL) || (Interp_flags & MR_DEPRECATED_ALL_XPARENT) || (Interp_warp_bitmap >= 0) ) {
		cull = gr_set_cull(0);
	} else {
		cull = gr_set_cull(1);
	}

	if ( !(Interp_flags & MR_DEPRECATED_NO_LIGHTING) ) {
		gr_set_lighting(true, true);
	}

	// rotate lights
	if ( !(Interp_flags & MR_DEPRECATED_NO_LIGHTING) )	{
		light_rotate_all();
 
		if ( !Cmdline_nohtl ) {
			light_set_all_relevent();
		}
	}
	if ( !(Interp_flags & MR_DEPRECATED_NO_LIGHTING) && (is_outlines_only_htl || (!Cmdline_nohtl && !is_outlines_only)) ) {
		opengl_change_active_lights(0); // Set up OpenGl lighting;
	}

	if (is_outlines_only_htl || (!Cmdline_nohtl && !is_outlines_only)) {
		gr_set_buffer(pm->vertex_buffer_id);
	}

	// Draw the subobjects	
	i = pm->submodel[pm->detail[Interp_detail_level]].first_child;
	if(Rendering_to_shadow_map)
		gr_zbias(-1024);
	else
		gr_zbias(0);
	while( i >= 0 )	{
		if ( !pm->submodel[i].is_thruster ) {
			// When in htl mode render with htl method unless its a jump node
			if (is_outlines_only_htl || (!Cmdline_nohtl && !is_outlines_only)) {
				model_render_children_buffers_DEPRECATED( pm, i, Interp_detail_level, render );
			} else {
				model_interp_subcall( pm, i, Interp_detail_level );
			}
		} else {
			draw_thrusters = true;
		}

		i = pm->submodel[i].next_sibling;
	}	

		

	model_radius = pm->submodel[pm->detail[Interp_detail_level]].rad;

	//*************************** draw the hull of the ship *********************************************

	// When in htl mode render with htl method unless its a jump node
	if (is_outlines_only_htl || (!Cmdline_nohtl && !is_outlines_only)) {
		model_render_buffers_DEPRECATED(pm, pm->detail[Interp_detail_level], render);
	} else {
		model_interp_subcall(pm, pm->detail[Interp_detail_level], Interp_detail_level);
	}

	// Draw the thruster subobjects
	if (draw_thrusters) {
		i = pm->submodel[pm->detail[Interp_detail_level]].first_child;

		while( i >= 0 ) {
			if (pm->submodel[i].is_thruster) {
				// When in htl mode render with htl method unless its a jump node
				if (is_outlines_only_htl || (!Cmdline_nohtl && !is_outlines_only)) {
					model_render_children_buffers_DEPRECATED( pm, i, Interp_detail_level, render );
				} else {
					model_interp_subcall( pm, i, Interp_detail_level );
				}
			}
			i = pm->submodel[i].next_sibling;
		}
	}

	if ( !Interp_no_flush ) {
		gr_clear_states();

		if (is_outlines_only_htl || (!Cmdline_nohtl && !is_outlines_only)) {
			gr_set_buffer(-1);
		}
	}

	if (is_outlines_only_htl) {
		gr_set_fill_mode(GR_FILL_MODE_SOLID);
	}

	if ( !(Interp_flags & MR_DEPRECATED_NO_LIGHTING) ) {
		gr_reset_lighting();
		gr_set_lighting(false, false);
	}

 	if (Interp_flags & MR_DEPRECATED_SHOW_PIVOTS )	{
 		model_draw_debug_points( pm, NULL, Interp_flags );
 		model_draw_debug_points( pm, &pm->submodel[pm->detail[Interp_detail_level]], Interp_flags );
 
 		if(pm->flags & PM_FLAG_AUTOCEN){
 			gr_set_color(255, 255, 255);
 			g3_draw_sphere_ez(&pm->autocenter, pm->rad / 4.5f);
 		}
 	}

	model_radius = 0.0f;

	// render model insignias
	gr_zbias(1);

	if (!Cmdline_nohtl)	gr_set_texture_panning(0.0, 0.0, false);

	gr_zbuffer_set(GR_ZBUFF_READ);
	if(!(Interp_flags & MR_DEPRECATED_NO_TEXTURING))
		model_render_insignias(pm, Interp_detail_level, Interp_insignia_bitmap);	

	gr_zbias(0);  

	if (FULLCLOAK != -1)	model_finish_cloak(FULLCLOAK);

	if ( pm->submodel[pm->detail[0]].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[pm->detail[0]]);
	}	

 	if ( Interp_flags & MR_DEPRECATED_SHOW_SHIELDS )	{
 		model_render_shields(pm, Interp_flags);
 	}	

 	if ( Interp_flags & MR_DEPRECATED_SHOW_PATHS ){
 		if (Cmdline_nohtl) model_draw_paths(model_num, Interp_flags);
 		else model_draw_paths_htl(model_num, Interp_flags);
 	}

 	if (Interp_flags & MR_DEPRECATED_BAY_PATHS ){
 		if (Cmdline_nohtl) model_draw_bay_paths(model_num);
 		else model_draw_bay_paths_htl(model_num);
 	}

	if ( (Interp_flags & MR_DEPRECATED_AUTOCENTER) && (set_autocen) ) {
		g3_done_instance(use_api);
	}

	g3_done_instance(use_api);

	// start rendering glow points -Bobboau
	if ( (pm->n_glow_point_banks) && !is_outlines_only && !is_outlines_only_htl && !Glowpoint_override ) {
		model_render_glow_points_DEPRECATED(pm, shipp, orient, pos, Glowpoint_use_depth_buffer);
	}

	// Draw the thruster glow
	if ( !is_outlines_only && !is_outlines_only_htl ) {
		if ( ( Interp_flags & MR_DEPRECATED_AUTOCENTER ) && set_autocen ) {
			vec3d autoback_rotated;

			vm_vec_unrotate(&autoback_rotated, &auto_back, orient);
			vm_vec_add2(&autoback_rotated, pos);

			model_render_thrusters( pm, objnum, shipp, orient, &autoback_rotated );
		} else {
			model_render_thrusters( pm, objnum, shipp, orient, pos );
		}
	}

	gr_zbuffer_set(save_gr_zbuffering_mode);

	gr_set_cull(cull);
}


void submodel_render_DEPRECATED(int model_num, int submodel_num, matrix *orient, vec3d * pos, uint flags, int objnum, int *replacement_textures, int render)
{
	// replacement textures - Goober5000
	model_set_replacement_textures(replacement_textures);

	polymodel * pm;

	MONITOR_INC( NumModelsRend, 1 );	

	if (!(Game_detail_flags & DETAIL_FLAG_MODELS) )	return;

	// Turn off engine effect
	Interp_thrust_scale_subobj=0;

	if (!Model_texturing)
		flags |= MR_NO_TEXTURING;

	Interp_flags = flags;
	Interp_pos=pos;
	
	pm = model_get(model_num);

	// Set the flags we will pass to the tmapper
	Interp_tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB;

	// if we're in nebula mode
	if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE)){
		Interp_tmap_flags |= TMAP_FLAG_PIXEL_FOG;
	}

	if ( !(Interp_flags & MR_NO_TEXTURING) )	{
		Interp_tmap_flags |= TMAP_FLAG_TEXTURED;

		if ( (pm->flags & PM_FLAG_ALLOW_TILING) && tiling )
			Interp_tmap_flags |= TMAP_FLAG_TILED;

		if ( !(Interp_flags & MR_NO_CORRECT) )	{
			Interp_tmap_flags |= TMAP_FLAG_CORRECT;
		}
	}

// 	if ( Interp_flags & MR_ANIMATED_SHADER )
// 		Interp_tmap_flags |= TMAP_ANIMATED_SHADER;

	bool is_outlines_only_htl = !Cmdline_nohtl && (flags & MR_NO_POLYS) && (flags & MR_SHOW_OUTLINE_HTL);

	//set to true since D3d and OGL need the api matrices set
	g3_start_instance_matrix(pos, orient, true);
	bool set_autocen = false;
	vec3d auto_back = ZERO_VECTOR;
	if (Interp_flags & MR_AUTOCENTER) {
		// standard autocenter using data in model
		if (pm->flags & PM_FLAG_AUTOCEN) {
			auto_back = pm->autocenter;
			vm_vec_scale(&auto_back, -1.0f);
			set_autocen = true;
		}
		// fake autocenter if we are a missile and don't already have autocen info
		else if (Interp_flags & MR_IS_MISSILE) {
            auto_back.xyz.x = -( (pm->submodel[pm->detail[Interp_detail_level]].max.xyz.x + pm->submodel[pm->detail[Interp_detail_level]].min.xyz.x) / 2.0f );
            auto_back.xyz.y = -( (pm->submodel[pm->detail[Interp_detail_level]].max.xyz.y + pm->submodel[pm->detail[Interp_detail_level]].min.xyz.y) / 2.0f );
			auto_back.xyz.z = -( (pm->submodel[pm->detail[Interp_detail_level]].max.xyz.z + pm->submodel[pm->detail[Interp_detail_level]].min.xyz.z) / 2.0f );
			set_autocen = true;
		}

		if (set_autocen)
			g3_start_instance_matrix(&auto_back, NULL, true);
	}

	if (is_outlines_only_htl) {
		gr_set_fill_mode( GR_FILL_MODE_WIRE );

		// lines shouldn't be rendered with textures or special RGB colors (assuming preset colors)
		Interp_flags |= MR_NO_TEXTURING;
		Interp_tmap_flags &= ~TMAP_FLAG_TEXTURED;
		Interp_tmap_flags &= ~TMAP_FLAG_RGB;
		// don't render with lighting either
		Interp_flags |= MR_NO_LIGHTING;
	} else {
		gr_set_fill_mode( GR_FILL_MODE_SOLID );
	}

	if ( !(Interp_flags & MR_NO_LIGHTING ) ) {
		Interp_light = 1.0f;

		gr_set_light_factor(Interp_light);

		light_filter_push( -1, pos, pm->submodel[submodel_num].rad );

		light_rotate_all();

		if (!Cmdline_nohtl) {
			light_set_all_relevent();
		}

		gr_set_lighting(true, true);
	}

	Interp_base_frametime = 0;

	if (objnum >= 0) {
		object *objp = &Objects[objnum];

		if (objp->type == OBJ_SHIP) {
			Interp_base_frametime = Ships[objp->instance].base_texture_anim_frametime;
		}
	}

	// fixes disappearing HUD in OGL - taylor
	int cull = gr_set_cull(1);

	if (!Cmdline_nohtl) {

		// RT - Put this here to fog debris
		if(Interp_tmap_flags & TMAP_FLAG_PIXEL_FOG)
		{
			float fog_near, fog_far;
			object *obj = NULL;
			
			if (objnum >= 0)
				obj = &Objects[objnum];

			neb2_get_adjusted_fog_values(&fog_near, &fog_far, obj);
			unsigned char r, g, b;
			neb2_get_fog_color(&r, &g, &b);
			gr_fog_set(GR_FOGMODE_FOG, r, g, b, fog_near, fog_far);
		}
		if(Rendering_to_shadow_map)
			gr_zbias(-1024);
		else
			gr_zbias(0);
		gr_set_buffer(pm->vertex_buffer_id);

		model_render_buffers_DEPRECATED(pm, submodel_num, render);

		gr_set_buffer(-1);
	} else {
		model_interp_sub( pm->submodel[submodel_num].bsp_data, pm, &pm->submodel[submodel_num], 0 );
	}

	if ( !(Interp_flags & MR_NO_LIGHTING) ) {
		gr_set_lighting(false, false);
		gr_reset_lighting();
	}

	gr_set_cull(cull);

	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	if ( pm->submodel[submodel_num].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[submodel_num]);
	}

	if (Interp_flags & MR_DEPRECATED_SHOW_PIVOTS )
		model_draw_debug_points( pm, &pm->submodel[submodel_num], Interp_flags );

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_filter_pop();	
	}
	gr_zbias(0);
	if (set_autocen)
		g3_done_instance(true);
	g3_done_instance(true);

	// turn off fog after each model renders, RT This fixes HUD being fogged when debris is in target box
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

	gr_clear_states();
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
				int n;
				int nverts = w(p+8);				
				int offset = w(p+16);
				int nnorms = 0;			

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
		case OP_BOUNDBOX:		break;
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

#ifndef NDEBUG
	if (RAND_MAX < nv)
	{
		static int submodel_get_two_random_points_warned = false;
		if (!submodel_get_two_random_points_warned)
		{
			polymodel *pm = model_get(model_num);
			Warning(LOCATION, "RAND_MAX is only %d, but submodel %d for model %s has %d vertices!  Explosions will not propagate through the entire model!\n", RAND_MAX, submodel_num, pm->filename, nv);
			submodel_get_two_random_points_warned = true;
		}
	}
#endif

	int vn1 = myrand() % nv;
	int vn2 = myrand() % nv;

	*v1 = *Interp_verts[vn1];
	*v2 = *Interp_verts[vn2];

	if(n1 != NULL){
		*n1 = *Interp_norms[vn1];
	}
	if(n2 != NULL){
		*n2 = *Interp_norms[vn2];
	}
}

void submodel_get_two_random_points_better(int model_num, int submodel_num, vec3d *v1, vec3d *v2)
{
	polymodel *pm = model_get(model_num);

	if (pm != NULL) {
		if ( submodel_num < 0 )	{
			submodel_num = pm->detail[0];
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

#ifndef NDEBUG
		if (RAND_MAX < nv)
		{
			static int submodel_get_two_random_points_warned = false;
			if (!submodel_get_two_random_points_warned)
			{
				Warning(LOCATION, "RAND_MAX is only %d, but submodel %d for model %s has %d vertices!  Explosions will not propagate through the entire model!\n", RAND_MAX, submodel_num, pm->filename, nv);
				submodel_get_two_random_points_warned = true;
			}
		}
#endif

		int vn1 = myrand() % nv;
		int vn2 = myrand() % nv;

		*v1 = tree->point_list[vn1];
		*v2 = tree->point_list[vn2];
	}
}

// If MR_FLAG_OUTLINE bit set this color will be used for outlines.
// This defaults to black.
void model_set_outline_color(int r, int g, int b )
{
	gr_init_color( &Interp_outline_color, r, g, b );

}

// If MR_FLAG_OUTLINE bit set this color will be used for outlines.
// This defaults to black.
void model_set_outline_color_fast(void *outline_color)
{
	Interp_outline_color = *((color*)(outline_color));
}

// IF MR_LOCK_DETAIL is set, then it will always draw detail level 'n'
// This defaults to 0. (0=highest, larger=lower)
void model_set_detail_level(int n)
{
	Interp_detail_level_locked = n;
}

/**
 * Returns number of verts in a submodel;
 */
int submodel_get_num_verts(int model_num, int submodel_num )
{
	polymodel * pm;

	pm = model_get(model_num);

	ubyte *p = pm->submodel[submodel_num].bsp_data;
	int chunk_type, chunk_size;

	chunk_type = w(p);
	chunk_size = w(p+4);

	while (chunk_type != OP_EOF)	{
		switch (chunk_type) {
		case OP_DEFPOINTS:	{
				int n=w(p+8);
				return n;		// Read in 'n' points
			}
			break;
		case OP_FLATPOLY:		break;
		case OP_TMAPPOLY:		break;
		case OP_SORTNORM:		break;
		case OP_BOUNDBOX:	break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in submodel_get_num_verts\n", chunk_type, chunk_size ));
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
 * Returns number of tmaps & flat polys in a submodel;
 */
int submodel_get_num_polys_sub( ubyte *p )
{
	int chunk_type = w(p);
	int chunk_size = w(p+4);
	int n = 0;
	
	while (chunk_type != OP_EOF)	{
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
		case OP_BOUNDBOX:	break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in submodel_get_num_polys\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
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

// Sets the submodel instance data in a submodel
// If show_damaged is true it shows only damaged submodels.
// If it is false it shows only undamaged submodels.
void model_show_damaged(int model_num, int show_damaged )
{
	polymodel * pm;
	int i;

	pm = model_get(model_num);

	for (i=0; i<pm->n_models; i++ )	{
		bsp_info *sm = &pm->submodel[i];

		// Set the "blown out" flags	
		sm->blown_off = 0;
	}

	for (i=0; i<pm->n_models; i++ )	{
		bsp_info *sm = &pm->submodel[i];

		// Set the "blown out" flags	
		if ( show_damaged )	{
			if ( sm->my_replacement > -1 )	{
				pm->submodel[sm->my_replacement].blown_off = 0;
				sm->blown_off = 1;
			}
		} else {
			if ( sm->my_replacement > -1 )	{
				pm->submodel[sm->my_replacement].blown_off = 1;
				sm->blown_off = 0;
			}
		}
	}
}

/**
 * Set the insignia bitmap to be used when rendering a ship with an insignia (-1 switches it off altogether)
 */
void model_set_insignia_bitmap(int bmap)
{
	Interp_insignia_bitmap = bmap;
}

void model_set_replacement_textures(int *replacement_textures)
{
	Interp_new_replacement_textures = replacement_textures;
}

/**
 * Set the forces bitmap
 */
void model_set_forced_texture(int bmap)
{
	Interp_forced_bitmap = bmap;
}

/**
 * Set model transparency for use with MR_ALL_XPARENT
 */
void model_set_alpha(float alpha)
{
	Interp_xparent_alpha = alpha;
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

void model_set_fog_level(float l)
{
	Interp_fog_level = l;
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
	int i, n;
	int nverts = w(off+bsp_data+8);	
	int offset = w(off+bsp_data+16);
	int next_norm = 0;

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

int Parse_normal_problem_count = 0;

void parse_tmap(int offset, ubyte *bsp_data)
{
	int pof_tex = w(bsp_data+offset+40);
	int n_vert = w(bsp_data+offset+36);
	
	ubyte *p = &bsp_data[offset+8];
	model_tmap_vert *tverts;

	tverts = (model_tmap_vert *)&bsp_data[offset+44];

	vertex *V;
	vec3d *v;
	vec3d *N;

	int problem_count = 0;

	for (int i = 1; i < (n_vert-1); i++) {
		V = &polygon_list[pof_tex].vert[(polygon_list[pof_tex].n_verts)];
		N = &polygon_list[pof_tex].norm[(polygon_list[pof_tex].n_verts)];
		v = Interp_verts[(int)tverts[0].vertnum];
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
		v = Interp_verts[(int)tverts[i].vertnum];
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
		v = Interp_verts[(int)tverts[i+1].vertnum];
		V->world.xyz.x = v->xyz.x;
		V->world.xyz.y = v->xyz.y;
		V->world.xyz.z = v->xyz.z;
		V->texture_position.u = tverts[i+1].u;
		V->texture_position.v = tverts[i+1].v;

		*N = *Interp_norms[(int)tverts[i+1].normnum];

		if ( IS_VEC_NULL(N) )
			*N = *vp(p);

		problem_count += check_values(N);
		vm_vec_normalize_safe(N);

		polygon_list[pof_tex].n_verts += 3;
	}

	Parse_normal_problem_count += problem_count;
}

void parse_sortnorm(int offset, ubyte *bsp_data);

void parse_bsp(int offset, ubyte *bsp_data)
{
	int id = w(bsp_data+offset);
	int size = w(bsp_data+offset+4);

	while (id != 0) {
		switch (id)
		{
			case OP_DEFPOINTS:
				parse_defpoint(offset, bsp_data);
				break;

			case OP_SORTNORM:
				parse_sortnorm(offset, bsp_data);
				break;

			case OP_FLATPOLY:
				break;

			case OP_TMAPPOLY:
				parse_tmap(offset, bsp_data);
				break;

			case OP_BOUNDBOX:
				break;

			default:
				return;
		}

		offset += size;
		id = w(bsp_data+offset);
		size = w(bsp_data+offset+4);

		if (size < 1)
			id = OP_EOF;
	}
}

void parse_sortnorm(int offset, ubyte *bsp_data)
{
	int frontlist, backlist, prelist, postlist, onlist;

	frontlist = w(bsp_data+offset+36);
	backlist = w(bsp_data+offset+40);
	prelist = w(bsp_data+offset+44);
	postlist = w(bsp_data+offset+48);
	onlist = w(bsp_data+offset+52);

	if (prelist) parse_bsp(offset+prelist,bsp_data);
	if (backlist) parse_bsp(offset+backlist, bsp_data);
	if (onlist) parse_bsp(offset+onlist, bsp_data);
	if (frontlist) parse_bsp(offset+frontlist, bsp_data);
	if (postlist) parse_bsp(offset+postlist, bsp_data);
}

void find_tmap(int offset, ubyte *bsp_data)
{
	int pof_tex = w(bsp_data+offset+40);
	int n_vert = w(bsp_data+offset+36);

	tri_count[pof_tex] += n_vert-2;	
}

void find_defpoint(int off, ubyte *bsp_data)
{
	int n;
	int nverts = w(off+bsp_data+8);	

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

void find_sortnorm(int offset, ubyte *bsp_data);

// tri_count
void find_tri_counts(int offset, ubyte *bsp_data)
{
	int id = w(bsp_data+offset);
	int size = w(bsp_data+offset+4);

	while (id != 0) {
		switch (id)
		{
			case OP_DEFPOINTS:
				find_defpoint(offset, bsp_data);
				break;

			case OP_SORTNORM:
				find_sortnorm(offset, bsp_data);
				break;

			case OP_FLATPOLY:
				break;

			case OP_TMAPPOLY:
				find_tmap(offset, bsp_data);
				break;

			case OP_BOUNDBOX:
				break;

			default:
				return;
		}

		offset += size;
		id = w(bsp_data+offset);
		size = w(bsp_data+offset+4);

		if (size < 1)
			id = OP_EOF;
	}
}

void find_sortnorm(int offset, ubyte *bsp_data)
{
	int frontlist, backlist, prelist, postlist, onlist;

	frontlist = w(bsp_data+offset+36);
	backlist = w(bsp_data+offset+40);
	prelist = w(bsp_data+offset+44);
	postlist = w(bsp_data+offset+48);
	onlist = w(bsp_data+offset+52);

	if (prelist) find_tri_counts(offset+prelist,bsp_data);
	if (backlist) find_tri_counts(offset+backlist, bsp_data);
	if (onlist) find_tri_counts(offset+onlist, bsp_data);
	if (frontlist) find_tri_counts(offset+frontlist, bsp_data);
	if (postlist) find_tri_counts(offset+postlist, bsp_data);
}

void interp_pack_vertex_buffers(polymodel *pm, int mn)
{
	Assert( pm->vertex_buffer_id >= 0 );
	Assert( (mn >= 0) && (mn < pm->n_models) );

	bsp_info *model = &pm->submodel[mn];

	if ( !model->buffer.model_list ) {
		return;
	}

	bool rval = gr_pack_buffer(pm->vertex_buffer_id, &model->buffer);

	if ( model->trans_buffer.flags & VB_FLAG_TRANS && model->trans_buffer.tex_buf.size() > 0 ) {
		gr_pack_buffer(pm->vertex_buffer_id, &model->trans_buffer);
	}

	if ( !rval ) {
		Error( LOCATION, "Unable to pack vertex buffer for '%s'\n", pm->filename );
	}
}

void interp_configure_vertex_buffers(polymodel *pm, int mn)
{
	int i, j, first_index;
	uint total_verts = 0;
	SCP_vector<int> vertex_list;

	Assert( (mn >= 0) && (mn < pm->n_models) );

	bsp_info *model = &pm->submodel[mn];

	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		polygon_list[i].n_verts = 0;
		tri_count[i] = 0;
	}

	bsp_polygon_data *bsp_polies = new bsp_polygon_data(model->bsp_data);

	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		int vert_count = bsp_polies->get_num_triangles(i) * 3;
		tri_count[i] = vert_count / 3;
		total_verts += vert_count;

		polygon_list[i].allocate(vert_count);

		bsp_polies->generate_triangles(i, polygon_list[i].vert, polygon_list[i].norm);
		polygon_list[i].n_verts = vert_count;

		// set submodel ID
		if ( Use_GLSL >= 3 ) {
			for ( j = 0; j < polygon_list[i].n_verts; ++j ) {
				polygon_list[i].submodels[j] = mn;
			}
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

		if ( Use_GLSL >= 3 ) {
			memcpy( (model_list->submodels) + model_list->n_verts, polygon_list[i].submodels, sizeof(int) * polygon_list[i].n_verts );
		}

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
		Assert( Use_GLSL >= 3 );
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

	bool rval = gr_config_buffer(pm->vertex_buffer_id, &model->buffer, false);

	if ( !rval ) {
		Error( LOCATION, "Unable to configure vertex buffer for '%s'\n", pm->filename );
	}
}

void interp_copy_index_buffer(vertex_buffer *src, vertex_buffer *dest, int *index_counts)
{
	size_t i, j, k;
	size_t src_buff_size;
	buffer_data *src_buffer;
	buffer_data *dest_buffer;
	uint vert_offset = src->vertex_offset / src->stride; // assuming all submodels crunched into this index buffer have the same stride
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
				dest_buffer->assign(dest_buffer->n_verts, src_buffer->get_index()[k] + vert_offset); // take into account the vertex offset.
				dest_buffer->n_verts++;

				Assert(dest_buffer->n_verts <= index_counts[dest_buffer->texture]);
			}
		}
	}
}

void interp_fill_detail_index_buffer(SCP_vector<int> &submodel_list, polymodel *pm, vertex_buffer *buffer)
{
	int index_counts[MAX_MODEL_TEXTURES];
	int i, j;
	int model_num;

	for ( i = 0; i < MAX_MODEL_TEXTURES; ++i ) {
		index_counts[i] = 0;
	}

	buffer->vertex_offset = 0;
	buffer->model_list = new(std::nothrow) poly_list;

	int num_buffers;
	int tex_num;

	// need to first count how many indexes there are in this entire detail model hierarchy
	for ( i = 0; i < (int)submodel_list.size(); ++i ) {
		model_num = submodel_list[i];

		if ( pm->submodel[model_num].is_thruster ) {
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
		if ( index_counts[i] <= 0 ) {
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

		if (pm->submodel[model_num].is_thruster) {
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
	SCP_vector<int> submodel_list;

	submodel_list.clear();

	model_get_submodel_tree_list(submodel_list, pm, pm->detail[detail_num]);

	if ( submodel_list.size() < 1 ) {
		return;
	}

	interp_fill_detail_index_buffer(submodel_list, pm, &pm->detail_buffers[detail_num]);
	//interp_fill_detail_index_buffer(submodel_list, pm, &pm->trans_buff[detail_num]);
	
	// check if anything was even put into this buffer
	if ( pm->detail_buffers[detail_num].tex_buf.size() < 1 ) {
		return;
	} 

	gr_config_buffer(pm->vertex_buffer_id, &pm->detail_buffers[detail_num], true);
	//gr_config_buffer(pm->vertex_buffer_id, &pm->trans_buff[detail_num], true);
}

void interp_create_transparency_index_buffer(polymodel *pm, int mn)
{
	const int NUM_VERTS_PER_TRI = 3;

	bsp_info *sub_model = &pm->submodel[mn];

	vertex_buffer *trans_buffer = &sub_model->trans_buffer;

	trans_buffer->model_list = new(std::nothrow) poly_list;
	trans_buffer->vertex_offset = pm->submodel[mn].buffer.vertex_offset;
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

		for ( int j = 0; j < tex_buf->n_verts; ++j ) {
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
		gr_config_buffer(pm->vertex_buffer_id, trans_buffer, true);
	}
}

void model_render_children_buffers_DEPRECATED(polymodel *pm, int mn, int detail_level, int render)
{
	int i;

	if ( (mn < 0) || (mn >= pm->n_models) ) {
		Int3();
		return;
	}

	bsp_info *model = &pm->submodel[mn];
	
	if (model->blown_off)
		return;

	uint fl = Interp_flags;

	if (model->is_thruster) {
		if ( !(Interp_flags & MR_SHOW_THRUSTERS) )
			return;

		Interp_flags |= MR_NO_LIGHTING;
		Interp_thrust_scale_subobj = 1;
		gr_set_lighting(false, false);
	} else {
		Interp_thrust_scale_subobj = 0;
	}

	// if using detail boxes or spheres, check that we are valid for the range
	if ( !(Interp_flags & MR_FULL_DETAIL) && model->use_render_box ) {
		vec3d box_min, box_max, offset;

		if (model->use_render_box_offset) {
			model_find_submodel_offset(&offset, pm->id, mn);
			vm_vec_sub(&offset, &vmd_zero_vector, &offset);
			vm_vec_add2(&offset, &model->render_box_offset);
		} else {
			offset = vmd_zero_vector;
		}

		vm_vec_copy_scale(&box_min, &model->render_box_min, Interp_box_scale);
		vm_vec_copy_scale(&box_max, &model->render_box_max, Interp_box_scale);

		if ( (-model->use_render_box + in_box(&box_min, &box_max, &offset, &View_position)) )
			return;
	}
	if ( !(Interp_flags & MR_FULL_DETAIL) && model->use_render_sphere ) {
		float radius = model->render_sphere_radius * Interp_box_scale;

		// TODO: doesn't consider submodel rotations yet -zookeeper
		vec3d offset;
		if (model->use_render_sphere_offset) {
			model_find_submodel_offset(&offset, pm->id, mn);
			vm_vec_sub(&offset, &vmd_zero_vector, &offset);
			vm_vec_add2(&offset, &model->render_sphere_offset);
		} else {
			offset = vmd_zero_vector;
		}

		if ( (-model->use_render_sphere + in_sphere(&offset, radius, &View_position)) )
			return;
	}

	// Get submodel rotation data and use submodel orientation matrix
	// to put together a matrix describing the final orientation of
	// the submodel relative to its parent
	angles ang = model->angs;

	// Add barrel rotation if needed
	if (model->gun_rotation) {
		if ( pm->gun_submodel_rotation > PI2 ) {
			pm->gun_submodel_rotation -= PI2;
		} else if ( pm->gun_submodel_rotation < 0.0f ) {
			pm->gun_submodel_rotation += PI2;
		}

		ang.b += pm->gun_submodel_rotation;
	}

	// Compute final submodel orientation by using the orientation matrix
	// and the rotation angles.
	// By using this kind of computation, the rotational angles can always
	// be computed relative to the submodel itself, instead of relative
	// to the parent
	matrix rotation_matrix = model->orientation;
	vm_rotate_matrix_by_angles(&rotation_matrix, &ang);

	matrix inv_orientation;
	vm_copy_transpose(&inv_orientation, &model->orientation);

	matrix submodel_matrix;
	vm_matrix_x_matrix(&submodel_matrix, &rotation_matrix, &inv_orientation);

	g3_start_instance_matrix(&model->offset, &submodel_matrix, true);
	
	model_render_buffers_DEPRECATED(pm, mn, render, true);

	if (Interp_flags & MR_DEPRECATED_SHOW_PIVOTS)
		model_draw_debug_points( pm, &pm->submodel[mn], Interp_flags );

	if (model->num_arcs)
		interp_render_lightning( pm, &pm->submodel[mn]);

	i = model->first_child;

	while (i >= 0) {
		if ( !pm->submodel[i].is_thruster ) {
			model_render_children_buffers_DEPRECATED( pm, i, detail_level, render );
		}

		i = pm->submodel[i].next_sibling;
	}

	Interp_flags = fl;

	if (Interp_thrust_scale_subobj) {
		Interp_thrust_scale_subobj = 0;

		if ( !(Interp_flags & MR_NO_LIGHTING) ) {
			gr_set_lighting(true, true);
		}
	}

	g3_done_instance(true);
}

void model_render_buffers_DEPRECATED(polymodel *pm, int mn, int render, bool is_child)
{
	if (pm->vertex_buffer_id < 0)
		return;

	if ( (mn < 0) || (mn >= pm->n_models) ) {
		Int3();
		return;
	}

	bsp_info *model = &pm->submodel[mn];

	// if using detail boxes or spheres, check that we are valid for the range
	if ( !is_child && !(Interp_flags & MR_FULL_DETAIL) && model->use_render_box ) {
		vec3d box_min, box_max, offset;

		if (model->use_render_box_offset) {
			model_find_submodel_offset(&offset, pm->id, mn);
			vm_vec_sub(&offset, &vmd_zero_vector, &offset);
			vm_vec_add2(&offset, &model->render_box_offset);
		} else {
			offset = vmd_zero_vector;
		}

		vm_vec_copy_scale(&box_min, &model->render_box_min, Interp_box_scale);
		vm_vec_copy_scale(&box_max, &model->render_box_max, Interp_box_scale);

		if ( (-model->use_render_box + in_box(&box_min, &box_max, &offset, &View_position)) )
			return;
	}
	if ( !is_child && !(Interp_flags & MR_FULL_DETAIL) && model->use_render_sphere ) {
		float radius = model->render_sphere_radius * Interp_box_scale;

		// TODO: doesn't consider submodel rotations yet -zookeeper
		vec3d offset;
		if (model->use_render_sphere_offset) {
			model_find_submodel_offset(&offset, pm->id, mn);
			vm_vec_sub(&offset, &vmd_zero_vector, &offset);
			vm_vec_add2(&offset, &model->render_sphere_offset);
		} else {
			offset = vmd_zero_vector;
		}

		if ( (-model->use_render_sphere + in_sphere(&offset, radius, &View_position)) )
			return;
	}

	vec3d scale;

	if (Interp_thrust_scale_subobj) {
		scale.xyz.x = 1.0f;
		scale.xyz.y = 1.0f;
		scale.xyz.z = Interp_thrust_scale;
	} else {
		scale.xyz.x = Interp_warp_scale_x;
		scale.xyz.y = Interp_warp_scale_y;
		scale.xyz.z = Interp_warp_scale_z;
	}

	gr_set_thrust_scale(Interp_thrust_scale);

	texture_info tex_replace[TM_NUM_TYPES];

	int no_texturing = (Interp_flags & MR_NO_TEXTURING);

	int forced_texture = -2;
	float forced_alpha = 1.0f;
	int forced_blend_filter = GR_ALPHABLEND_NONE;

	if ( Interp_forced_bitmap >= 0 ) {
		forced_texture = Interp_forced_bitmap;
	} else if (Interp_warp_bitmap >= 0) {
		forced_texture = Interp_warp_bitmap;
		forced_alpha = Interp_warp_alpha;
		forced_blend_filter = GR_ALPHABLEND_FILTER;
	} else if (Interp_thrust_scale_subobj) {
		if ( (Interp_thrust_bitmap >= 0) && (Interp_thrust_scale > 0.0f) ) {
			forced_texture = Interp_thrust_bitmap;
		} else {
			forced_texture = -1;
		}

		forced_alpha = 1.2f;
		forced_blend_filter = GR_ALPHABLEND_FILTER;
	} else if (Interp_flags & MR_ALL_XPARENT) {
		forced_alpha = Interp_xparent_alpha;
		forced_blend_filter = GR_ALPHABLEND_FILTER;
	}

	if (!Interp_thrust_scale_subobj) {
		gr_push_scale_matrix(&scale);
	}

	size_t buffer_size = model->buffer.tex_buf.size();

	for (size_t i = 0; i < buffer_size; i++) {
		int texture = -1;
		int tmap_num = model->buffer.tex_buf[i].texture;
		texture_map *tmap = &pm->maps[tmap_num];
		int rt_begin_index = tmap_num*TM_NUM_TYPES;
		float alpha = 1.0f;
		int blend_filter = GR_ALPHABLEND_NONE;

		if (forced_texture != -2) {
			texture = forced_texture;
			alpha = forced_alpha;
		}
		else if ( !no_texturing ) {
			// pick the texture, animating it if necessary
			if ( (Interp_new_replacement_textures != NULL) && (Interp_new_replacement_textures[rt_begin_index + TM_BASE_TYPE] == REPLACE_WITH_INVISIBLE) ) {
				// invisible textures aren't rendered, but we still have to skip assigning the underlying model texture
				texture = -1;
			} else if ( (Interp_new_replacement_textures != NULL) && (Interp_new_replacement_textures[rt_begin_index + TM_BASE_TYPE] >= 0) ) {
				// an underlying texture is replaced with a real new texture
				tex_replace[TM_BASE_TYPE] = texture_info(Interp_new_replacement_textures[rt_begin_index + TM_BASE_TYPE]);
				texture = model_interp_get_texture(&tex_replace[TM_BASE_TYPE], Interp_base_frametime);
			} else {
				// we just use the underlying texture
				texture = model_interp_get_texture(&tmap->textures[TM_BASE_TYPE], Interp_base_frametime);
			}

			if (texture < 0) {
				continue;
			}

			// doing glow maps?
			if ( !(Interp_flags & MR_NO_GLOWMAPS) ) {
				texture_info *tglow = &tmap->textures[TM_GLOW_TYPE];

				if ( (Interp_new_replacement_textures != NULL) && (Interp_new_replacement_textures[rt_begin_index + TM_GLOW_TYPE] >= 0) ) {
					tex_replace[TM_GLOW_TYPE] = texture_info(Interp_new_replacement_textures[rt_begin_index + TM_GLOW_TYPE]);
					GLOWMAP = model_interp_get_texture(&tex_replace[TM_GLOW_TYPE], Interp_base_frametime);
				} else if (tglow->GetTexture() >= 0) {
					// shockwaves are special, their current frame has to come out of the shockwave code to get the timing correct
					if ( (Interp_objnum >= 0) && (Objects[Interp_objnum].type == OBJ_SHOCKWAVE) && (tglow->GetNumFrames() > 1) ) {
						GLOWMAP = tglow->GetTexture() + shockwave_get_framenum(Objects[Interp_objnum].instance, tglow->GetNumFrames());
					} else {
						GLOWMAP = model_interp_get_texture(tglow, Interp_base_frametime);
					}
				}
			}

			if ( (Detail.lighting > 2)  && (Interp_detail_level < 2) ) {
				// likewise, etc.
				texture_info *spec_map = &tmap->textures[TM_SPECULAR_TYPE];
				texture_info *norm_map = &tmap->textures[TM_NORMAL_TYPE];
				texture_info *height_map = &tmap->textures[TM_HEIGHT_TYPE];

				if (Interp_new_replacement_textures != NULL) {
					if (Interp_new_replacement_textures[rt_begin_index + TM_SPECULAR_TYPE] >= 0) {
						tex_replace[TM_SPECULAR_TYPE] = texture_info(Interp_new_replacement_textures[rt_begin_index + TM_SPECULAR_TYPE]);
						spec_map = &tex_replace[TM_SPECULAR_TYPE];
					}

					if (Interp_new_replacement_textures[rt_begin_index + TM_NORMAL_TYPE] >= 0) {
						tex_replace[TM_NORMAL_TYPE] = texture_info(Interp_new_replacement_textures[rt_begin_index + TM_NORMAL_TYPE]);
						norm_map = &tex_replace[TM_NORMAL_TYPE];
					}

					if (Interp_new_replacement_textures[rt_begin_index + TM_HEIGHT_TYPE] >= 0) {
						tex_replace[TM_HEIGHT_TYPE] = texture_info(Interp_new_replacement_textures[rt_begin_index + TM_HEIGHT_TYPE]);
						height_map = &tex_replace[TM_HEIGHT_TYPE];
					}
				}

				SPECMAP = model_interp_get_texture(spec_map, Interp_base_frametime);
				NORMMAP = model_interp_get_texture(norm_map, Interp_base_frametime);
				HEIGHTMAP = model_interp_get_texture(height_map, Interp_base_frametime);
			}

			texture_info *misc_map = &tmap->textures[TM_MISC_TYPE];

			if (Interp_new_replacement_textures != NULL) {
				if (Interp_new_replacement_textures[rt_begin_index + TM_MISC_TYPE] >= 0) {
					tex_replace[TM_MISC_TYPE] = texture_info(Interp_new_replacement_textures[rt_begin_index + TM_MISC_TYPE]);
					misc_map = &tex_replace[TM_MISC_TYPE];
				}
			}

			MISCMAP = model_interp_get_texture(misc_map, Interp_base_frametime);
		} else {
			alpha = forced_alpha;
		
			//Check for invisible or transparent textures so they dont show up in the shadow maps - Valathil
			if ( (Interp_new_replacement_textures != NULL) && (Interp_new_replacement_textures[rt_begin_index + TM_BASE_TYPE] >= 0) ) {
				tex_replace[TM_BASE_TYPE] = texture_info(Interp_new_replacement_textures[rt_begin_index + TM_BASE_TYPE]);
				texture = model_interp_get_texture(&tex_replace[TM_BASE_TYPE], Interp_base_frametime);
			} else {
				texture = model_interp_get_texture(&tmap->textures[TM_BASE_TYPE], Interp_base_frametime);
			}

			if (texture <= 0) {
				continue;
			}

			//if(bm_has_alpha_channel(texture))
				//continue;
		}

		if ( (texture == -1) && !no_texturing ) {
			continue;
		}

		// trying to get transperent textures-Bobboau
		if (tmap->is_transparent) {
			// for special shockwave/warpmap usage
			alpha = (Interp_warp_alpha != -1.0f) ? Interp_warp_alpha : 0.8f;
			blend_filter = GR_ALPHABLEND_FILTER;
			
		}
		
		if (forced_blend_filter != GR_ALPHABLEND_NONE) {
			blend_filter = forced_blend_filter;
		}

		extern bool object_had_transparency;
		if (blend_filter != GR_ALPHABLEND_NONE) {
			if(render & MODEL_RENDER_TRANS)
			{
				gr_zbuffer_set(GR_ZBUFF_READ);
				gr_set_bitmap(texture, blend_filter, GR_BITBLT_MODE_NORMAL, alpha);
				gr_render_buffer(0, &model->buffer, i, Interp_tmap_flags);
				gr_zbuffer_set(GR_ZBUFF_FULL);
			}
			else
			{
				object_had_transparency = true;
			}
		}
		else
		{
			if(render & MODEL_RENDER_OPAQUE)
			{
				gr_set_bitmap(texture, blend_filter, GR_BITBLT_MODE_NORMAL, alpha);
				gr_render_buffer(0, &model->buffer, i, Interp_tmap_flags);
			}
		}

		GLOWMAP = -1;
		SPECMAP = -1;
		NORMMAP = -1;
		HEIGHTMAP = -1;
		MISCMAP = -1;
	}

	gr_pop_scale_matrix();
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

void model_set_warp_globals(float scale_x, float scale_y, float scale_z, int bitmap_id, float alpha)
{
	Interp_warp_scale_x = scale_x;
	Interp_warp_scale_y = scale_y;
	Interp_warp_scale_z = scale_z;

	Interp_warp_bitmap = bitmap_id;
	Interp_warp_alpha = alpha;
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

// temporary until we can unify the global Interp_* variables into the interp_data struct
void model_interp_set_team_color(const SCP_string &team, const SCP_string &secondaryteam, fix timestamp, int fadetime)
{
	Interp_team_color_set = model_get_team_color(&Interp_team_color, team, secondaryteam, timestamp, fadetime);
}

// temporary until we can unify the global Interp_* variables into the interp_data struct
void model_interp_set_clip_plane(vec3d* pos, vec3d* normal)
{
	if ( pos == NULL || normal == NULL ) {
		Interp_clip_plane = false;
		return;
	}

	Interp_clip_normal = *normal;
	Interp_clip_pos = *pos;
	Interp_clip_plane = true;
}

// temporary until we can unify the global Interp_* variables into the interp_data struct
void model_interp_set_animated_effect_and_timer(int effect_num, float effect_timer)
{
	Interp_animated_effect = effect_num;
	Interp_animated_timer = effect_timer;
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
int texture_info::LoadTexture(char *filename, char *dbg_name = "<UNKNOWN>")
{
	if (strlen(filename) + 4 >= NAME_LENGTH) //Filenames are passed in without extension
	{
		mprintf(("Generated texture name %s is too long. Skipping...\n", filename));
		return -1;
	}
	this->original_texture = bm_load_either(filename, NULL, NULL, NULL, 1, CF_TYPE_MAPS);
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
int texture_map::FindTexture(char *fname)
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

	while (id != 0) {
		switch (id)
		{
		case OP_DEFPOINTS:
			process_defpoints(offset, bsp_data);
			break;

		case OP_SORTNORM:
			process_sortnorm(offset, bsp_data);
			break;

		case OP_FLATPOLY:
			process_flat(offset, bsp_data);
			break;

		case OP_TMAPPOLY:
			process_tmap(offset, bsp_data);
			break;

		case OP_BOUNDBOX:
			break;

		default:
			return;
		}

		offset += size;
		id = w(bsp_data + offset);
		size = w(bsp_data + offset + 4);

		if (size < 1)
			id = OP_EOF;
	}
}

void bsp_polygon_data::process_defpoints(int off, ubyte* bsp_data)
{
	int i, n;
	int nverts = w(off + bsp_data + 8);
	int offset = w(off + bsp_data + 16);

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
	int n_vert = w(bsp_data + offset + 36);

	if ( n_vert < 3 ) {
		// don't parse degenerate polygons
		return;
	}

	ubyte *p = &bsp_data[offset + 8];
	model_tmap_vert *tverts;

	tverts = (model_tmap_vert *)&bsp_data[offset + 44];

	int problem_count = 0;

	// make a polygon
	bsp_polygon polygon;

	polygon.Start_index = Polygon_vertices.size();
	polygon.Num_verts = n_vert;
	polygon.texture = pof_tex;

	// this polygon will be broken up into a triangle fan. first three verts make up the first triangle
	// additional verts are made into new tris
	Num_polies[pof_tex]++;
	Num_verts[pof_tex] += n_vert;

	// stuff data making up the vertices of this polygon
	for ( int i = 0; i < n_vert; ++i ) {
		bsp_vertex vert;

		vert.position = Vertex_list[(int)tverts[i].vertnum];
		
		vert.tex_coord.u = tverts[i].u;
		vert.tex_coord.v = tverts[i].v;

		vert.normal = Normal_list[(int)tverts[i].normnum];

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

	polygon.Start_index = Polygon_vertices.size();
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
