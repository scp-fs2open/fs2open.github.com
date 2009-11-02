/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#define MODEL_LIB

#include "model/model.h"
#include "model/modelsinc.h"
#include "graphics/2d.h"
#include "render/3dinternal.h"
#include "math/fvi.h"
#include "lighting/lighting.h"
#include "bmpman/bmpman.h"
#include "io/key.h"
#include "io/timer.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "math/staticrand.h"
#include "particle/particle.h"
#include "ship/ship.h"
#include "cmdline/cmdline.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"
#include "weapon/shockwave.h"
#include "parse/parselo.h"	//strextcmp

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

static float Interp_box_scale = 1.0f;
static vec3d Interp_render_box_min = ZERO_VECTOR;
static vec3d Interp_render_box_max = ZERO_VECTOR;

// -------------------------------------------------------------------
// lighting save stuff 
//
#define MAX_MODEL_LIGHTING_SAVE			30
int hack_skip_max = 1;
DCF(skip, "")
{
	dc_get_arg(ARG_INT);
	hack_skip_max = Dc_arg_int;
}
// model_light_object Interp_lighting_save[MAX_MODEL_LIGHTING_SAVE];
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
static int Interp_detail_level_locked = 0;
static uint Interp_flags = 0;
static uint Interp_tmap_flags = 0;

// If non-zero, then the subobject gets scaled by Interp_thrust_scale.
static int Interp_thrust_scale_subobj = 0;
static float Interp_thrust_scale = 0.1f;
static float Interp_thrust_scale_x = 0.0f;//added -bobboau
static float Interp_thrust_scale_y = 0.0f;//added -bobboau

static int Interp_thrust_bitmap = -1;
static int Interp_thrust_glow_bitmap = -1;
static float Interp_thrust_glow_noise = 1.0f;
static bool Interp_afterburner = false;

// Bobboau's thruster stuff
static int Interp_secondary_thrust_glow_bitmap = -1;
static int Interp_tertiary_thrust_glow_bitmap = -1;
static float Interp_thrust_glow_rad_factor = 1.0f;
static float Interp_secondary_thrust_glow_rad_factor = 1.0f;
static float Interp_tertiary_thrust_glow_rad_factor = 1.0f;
static float Interp_thrust_glow_len_factor = 1.0f;
static vec3d Interp_thrust_rotvel = ZERO_VECTOR;

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

// forward references
int model_interp_sub(void *model_ptr, polymodel * pm, bsp_info *sm, int do_box_check);
void model_try_cache_render(int model_num, matrix *orient, vec3d * pos, uint flags, int objnum, int num_lights);
void model_really_render(int model_num, matrix *orient, vec3d * pos, uint flags, int objnum = -1);
void model_interp_sortnorm_b2f(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check);
void model_interp_sortnorm_f2b(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check);
void (*model_interp_sortnorm)(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check) = model_interp_sortnorm_b2f;
int model_should_render_engine_glow(int objnum, int bank_obj);
void model_render_buffers(bsp_info *model, polymodel *pm, bool is_child = false);
void model_render_children_buffers(bsp_info* model, polymodel * pm, int mn, int detail_level);
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
		Interp_verts = (vec3d**) vm_malloc( n_verts * sizeof(vec3d) );

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
		Interp_norms = (vec3d**) vm_malloc( n_norms * sizeof(vec3d) );

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


// call at the beginning of a level. after the level has been loaded
void model_level_post_init()
{
	/*
	int idx;

	// reset lighting stuff	
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		Interp_lighting_save[idx].objnum = -1;
		Interp_lighting_save[idx].skip = 0;
	}

	// saved lighting is not full
	Interp_saved_lighting_full = 0;
	*/
}

// call to select an object for using "saved" lighting
void model_set_saved_lighting(int objnum, int skip_max)
{
	/*
	int idx;

	// always set to not using saved light to start with
	Interp_use_saved_lighting = 0;
	Interp_lighting = &Interp_lighting_temp;

	// if he passed a -1 for either value, no saved lighting
	if((objnum == -1) || (skip_max == -1)){
		return;
	}

	// see if the object is already on the list
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		// ahha, he is on the list
		if(Interp_lighting_save[idx].objnum == objnum){
			// if he's entered a new skip max
			if(Interp_lighting_save[idx].skip_max != skip_max){
				Interp_lighting_save[idx].skip = 0;
				Interp_lighting_save[idx].skip_max = skip_max;
				Interp_use_saved_lighting = 0;
				Interp_lighting = &Interp_lighting_save[idx];
			} 
			// if we're reached the "skip" frame, re-render lighting for this guy
			else if(Interp_lighting_save[idx].skip == Interp_lighting_save[idx].skip_max){
				Interp_lighting_save[idx].skip = 0;
				Interp_use_saved_lighting = 0;
				Interp_lighting = &Interp_lighting_save[idx];
			}
			// otherwise, use his saved lighting values
			else {
				Interp_lighting_save[idx].skip++;
				Interp_use_saved_lighting = 1;
				Interp_lighting = &Interp_lighting_save[idx];
			}

			// duh
			return;
		}
	}

	// no free saved lighting slots
	if(Interp_saved_lighting_full){
		return;
	}
	
	// find a free slot
	int found = 0;
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		// got one
		if(Interp_lighting_save[idx].objnum == -1){
			Interp_lighting_save[idx].objnum = objnum;
			Interp_lighting_save[idx].skip = 0;
			Interp_lighting_save[idx].skip_max = skip_max;

			Interp_use_saved_lighting = 0;
			Interp_lighting = &Interp_lighting_save[idx];

			found = 1;
			break;
		}
	}

	// oops. out of free slots
	if(!found){
		Interp_saved_lighting_full = 1;
	}
	*/
}

// notify the model system that a ship has died
void model_notify_dead_ship(int objnum)
{
	/*
	int idx;

	// see if this guy was on the lighting list
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		// free him up
		if(objnum == Interp_lighting_save[idx].objnum){
			Interp_lighting_save[idx].objnum = -1;
			Interp_saved_lighting_full = 0;
			return;
		}
	}
	*/
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
	Interp_render_box_min = vmd_zero_vector;
	Interp_render_box_max = vmd_zero_vector;
}

// Scales the engines thrusters by this much
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

	Interp_thrust_glow_noise = mst->glow_noise;
	Interp_afterburner = mst->use_ab;

	if (mst->rotvel != NULL) {
		Interp_thrust_rotvel = *(mst->rotvel);
	} else {
		vm_vec_zero(&Interp_thrust_rotvel);
	}

	Interp_thrust_glow_rad_factor = mst->glow_rad_factor;
	Interp_secondary_thrust_glow_rad_factor = mst->secondary_glow_rad_factor;
	Interp_tertiary_thrust_glow_rad_factor = mst->tertiary_glow_rad_factor;
	Interp_thrust_glow_len_factor = mst->glow_length_factor;

	//this isn't used
/*
	polymodel * pm = model_get( model_num );
	int i;

	// If thrust is set up, use it.
	for (i=0; i<pm->num_lights; i++ )	{
		if ( pm->lights[i].type == BSP_LIGHT_TYPE_THRUSTER )	{
			float scale = (Interp_thrust_scale-0.1f)*0.5f;

			pm->lights[i].value += (scale+Interp_thrust_glow_noise*0.2f) / 255.0f;
		}
	}
	*/
}

extern int Cmdline_cell;

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
	if(Cmdline_cell)model_interp_splode_defpoints(p, pm, sm, model_radius/100);
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
		// SHUT UP! -- Kazan -- This is massively slowing debug builds down
		//mprintf(("warp model being scaled by %f %f %f\n", Interp_warp_scale_x, Interp_warp_scale_y, Interp_warp_scale_z));
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
				 /* 	                                 
				 vec3d tmp = *src; 	 
				 // TEST 	                                 ;
				 if(Interp_thrust_twist > 0.0f){ 	                                 
						 float theta; 	                                 
						 float st, ct; 	                                 

						 // determine theta for this vertex 	                                 
						 theta = fl_radian(20.0f + Interp_thrust_twist2); 	                         
						 st = sin(theta); 	                                 
						 ct = cos(theta); 	                                 
  					 }
						 // twist 	 
						 tmp.xyz.z = (src->xyz.z * ct) - (src->xyz.y * st); 	 
						 tmp.xyz.y = (src->xyz.z * st) + (src->xyz.y * ct); 	 

						 // scale the z a bit 	 
						 tmp.xyz.z += Interp_thrust_twist; 	 
				 } 	 

				 g3_rotate_vertex(dest, &tmp); 	 
				 */ 	 

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
vec3d Interp_offset;


void interp_compute_environment_mapping( vec3d *nrm, vertex * pnt)
{
	return;
/*	vec3d R;
	float a;
	matrix * m = &View_matrix;

	vm_vec_rotate( &R, nrm, &View_matrix );	
	vm_vec_normalize(&R);

	a = 2.0f * R.xyz.z;
	R.xyz.x = a * R.xyz.x;	// reflected R = 2N(N.E) -E;  E = eye
	R.xyz.y = a * R.xyz.y;
	R.xyz.z = a * R.xyz.z;
	vm_vec_normalize(&R);
	a = (float)fl_sqrt( 1.0f - R.xyz.y * R.xyz.y);
	pnt->u2 = (float)atan2( R.xyz.x, -R.xyz.z) / (2.0f * 3.14159f);
	if (pnt->u2 < 0.0) pnt->u2 += 1.0f;
	pnt->v2 = 1.0f - (float)atan2( a, R.xyz.y) / 3.14159f;
	*/
}


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

	for (i=0;i<nv;i++)	{
		Interp_list[i] = &Interp_points[verts[i*2]];

		if ( Interp_flags & MR_NO_LIGHTING )	{
				Interp_list[i]->r = 191;
				Interp_list[i]->g = 191;
				Interp_list[i]->b = 191;
		} else if(Cmdline_cell){
			Interp_list[i]->r = 0;
			Interp_list[i]->g = 0;
			Interp_list[i]->b = 0;
		}else{
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
		int i, j;

		if ( Interp_flags & MR_SHOW_OUTLINE )	{
			gr_set_color_fast( &Interp_outline_color );
		}

		for (i=0; i<nv; i++ )	{
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
	//mprintf(("model_interp_tmappoly tmap_num: %d\n", tmap_num));
	//Assert(tmap_num >= 0 && tmap_num < MAX_MODEL_TEXTURES);

	// Goober5000
	fix base_frametime = 0;
	if (Interp_objnum >= 0)
	{
		object *objp = &Objects[Interp_objnum];
		if (objp->type == OBJ_SHIP)
			base_frametime = Ships[objp->instance].base_texture_anim_frametime;
	}

	int is_invisible = 0;

	if (Interp_warp_bitmap < 0) {
		if ( (!Interp_thrust_scale_subobj) && (tbase->GetTexture() < 0) ) {
			// Don't draw invisible polygons.
			if ( !(Interp_flags & MR_SHOW_INVISIBLE_FACES) )
				return;
			else
				is_invisible = 1;
		}
	}


	nv = w(p+36);

//	Tmap_show_layers = 1;

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
				if(Cmdline_cell){
					for (i=0;i<nv;i++){
						Interp_list[i] = &Interp_splode_points[verts[i].vertnum];
						Interp_list[i]->u = verts[i].u;
						Interp_list[i]->v = verts[i].v;
						Interp_list[i]->r = 250;
						Interp_list[i]->g = 250;
						Interp_list[i]->b = 250;
		
					}
					cull = gr_set_cull(0);
					gr_set_color( 0, 0, 0 );
					g3_draw_poly( nv, Interp_list, 0 );
					gr_set_cull(cull);
				}
				if(!splodeing)return;
			}
		}

		if(splodeing){
			float salpha = 1.0f - splode_level;
			for (i=0;i<nv;i++){
				Interp_list[i] = &Interp_splode_points[verts[i].vertnum];
				Interp_list[i]->u = verts[i].u*2;
				Interp_list[i]->v = verts[i].v*2;
				Interp_list[i]->r = (unsigned char)(255*salpha);
				Interp_list[i]->g = (unsigned char)(250*salpha);
				Interp_list[i]->b = (unsigned char)(200*salpha);
				model_interp_edge_alpha(&Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[verts[i].vertnum], Interp_norms[verts[i].normnum], salpha, false);
			}
			cull = gr_set_cull(0);
			gr_set_bitmap( splodeingtexture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, salpha );
		//	gr_set_color( 255, 250, 200 );
		//	g3_draw_poly( nv, Interp_list, 0 );
			g3_draw_poly( nv, Interp_list,  TMAP_FLAG_TEXTURED|TMAP_FLAG_GOURAUD);
			gr_set_cull(cull);
			return;
		}
	}

	for (i=0;i<nv;i++)	{
		Interp_list[i] = &Interp_points[verts[i].vertnum];

		Interp_list[i]->u = verts[i].u;
		Interp_list[i]->v = verts[i].v;
		
		if ( Interp_subspace )	{
			Interp_list[i]->v += Interp_subspace_offset_u;
			Interp_list[i]->u += Interp_subspace_offset_v;
			Interp_list[i]->r = Interp_subspace_r;
			Interp_list[i]->g = Interp_subspace_g;
			Interp_list[i]->b = Interp_subspace_b;
		} else {

	//		if ( !(pm->flags & PM_FLAG_ALLOW_TILING) )	{
	//			Assert(verts[i].u <= 1.0f );
	//			Assert(verts[i].v <= 1.0f );
	//		}

	//		Assert( verts[i].normnum == verts[i].vertnum );
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
			} else {

				int vertnum = verts[i].vertnum;
				int norm = verts[i].normnum;
		
				if ( Interp_flags & MR_NO_SMOOTHING )	{
					light_apply_rgb( &Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[vertnum], vp(p+8), Interp_light );
					if((Detail.lighting > 2) && (Interp_detail_level < 2) && !Cmdline_cell && Cmdline_spec )
						light_apply_specular( &Interp_list[i]->spec_r, &Interp_list[i]->spec_g, &Interp_list[i]->spec_b, Interp_verts[vertnum], vp(p+8),  &View_position);
					//	interp_compute_environment_mapping(vp(p+8), Interp_list[i]);
				} else {					
					// if we're applying lighting as normal, and not using saved lighting
					if ( !Interp_use_saved_lighting && !Interp_light_applied[norm] )	{

						light_apply_rgb( &Interp_lighting->lights[norm].r, &Interp_lighting->lights[norm].g, &Interp_lighting->lights[norm].b, Interp_verts[vertnum], Interp_norms[norm], Interp_light );
						if((Detail.lighting > 2) && (Interp_detail_level < 2) && !Cmdline_cell && Cmdline_spec )
							light_apply_specular( &Interp_lighting->lights[norm].spec_r, &Interp_lighting->lights[norm].spec_g, &Interp_lighting->lights[norm].spec_b, Interp_verts[vertnum], Interp_norms[norm],  &View_position);
						//	interp_compute_environment_mapping(Interp_verts[vertnum], Interp_list[i]);

						Interp_light_applied[norm] = 1;
					}else if(Interp_light_applied[norm]){
					//	if((Detail.lighting > 2) && (Interp_detail_level < 2))
					//		light_apply_specular( &Interp_list[i]->spec_r, &Interp_list[i]->spec_g, &Interp_list[i]->spec_b, Interp_verts[vertnum], Interp_norms[norm],  &View_position);
					//	interp_compute_environment_mapping(Interp_verts[vertnum], Interp_list[i]);
					}

					Interp_list[i]->spec_r = Interp_lighting->lights[norm].spec_r;
					Interp_list[i]->spec_g = Interp_lighting->lights[norm].spec_g;
					Interp_list[i]->spec_b = Interp_lighting->lights[norm].spec_b;

					Interp_list[i]->r = Interp_lighting->lights[norm].r;
					Interp_list[i]->g = Interp_lighting->lights[norm].g;
					Interp_list[i]->b = Interp_lighting->lights[norm].b;
//					if((Detail.lighting > 2) && (Interp_detail_level < 2))
//						light_apply_specular( &Interp_list[i]->spec_r, &Interp_list[i]->spec_g, &Interp_list[i]->spec_b, Interp_verts[vertnum], Interp_norms[norm],  &View_position);
				}
			}
		}

//		Assert(verts[i].u >= 0.0f );
//		Assert(verts[i].v >= 0.0f );

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
					if((Interp_flags & MR_FORCE_TEXTURE) && (Interp_forced_bitmap >= 0)){
						texture = Interp_forced_bitmap;
					}else if((Interp_new_replacement_textures != NULL) && (Interp_new_replacement_textures[rt_begin_index + TM_BASE_TYPE] >= 0)){
						texture_info tt = texture_info(Interp_new_replacement_textures[rt_begin_index + TM_BASE_TYPE]);
						texture = model_interp_get_texture(&tt, base_frametime);
					} else {
						// pick the texture, animating it if necessary
						texture = model_interp_get_texture(tbase, base_frametime);
						
						// doing glow maps?
						if ( !(Interp_flags & MR_NO_GLOWMAPS) && (tglow->GetTexture() >= 0) ) {
							// shockwaves are special, their current frame has to come out of the shockwave code to get the timing correct
							if ( (Interp_objnum >= 0) && (Objects[Interp_objnum].type == OBJ_SHOCKWAVE) && (tglow->GetNumFrames() > 1) ) {
								GLOWMAP = tglow->GetTexture() + shockwave_get_framenum(Objects[Interp_objnum].instance, tglow->GetNumFrames());
							} else {
								GLOWMAP = model_interp_get_texture(tglow, base_frametime);
							}
						}

						if((Detail.lighting > 2)  && (Interp_detail_level < 2))
						{
							// likewise, etc.
							SPECMAP = model_interp_get_texture(&tmap->textures[TM_SPECULAR_TYPE], base_frametime);
							NORMMAP = model_interp_get_texture(&tmap->textures[TM_NORMAL_TYPE], base_frametime);
							HEIGHTMAP = model_interp_get_texture(&tmap->textures[TM_HEIGHT_TYPE], base_frametime);
						}
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
							tex = model_interp_get_texture(&tinfo, base_frametime);
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
					g3_draw_poly( nv, Interp_list, Interp_tmap_flags|TMAP_FLAG_NONDARKENING );		
				}
			}
		}
	}

	GLOWMAP = -1;
	SPECMAP = -1;
	NORMMAP = -1;
	HEIGHTMAP = -1;

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

void interp_draw_box( vec3d *min, vec3d *max )
{
/*
	int i;
	vec3d bounding_box[8];
	vertex pts[8];
	vertex *l[4];

	model_calc_bound_box(bounding_box,min,max);

	for (i=0; i<8; i++ )	{
		g3_rotate_vertex( &pts[i], &bounding_box[i] );
	}

	gr_set_color(128,0,0);

	Tmap_show_layers = 1;

	l[3] = &pts[0];  l[2] = &pts[1]; l[1] = &pts[2];  l[0] = &pts[3];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[3];  l[2] = &pts[2]; l[1] = &pts[6];  l[0] = &pts[7];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[2];  l[2] = &pts[1]; l[1] = &pts[5];  l[0] = &pts[6];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[1];  l[2] = &pts[0]; l[1] = &pts[4];  l[0] = &pts[5];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[0];  l[2] = &pts[3]; l[1] = &pts[7];  l[0] = &pts[4];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[4];  l[2] = &pts[7]; l[1] = &pts[6];  l[0] = &pts[5];
	g3_draw_poly( 4, l, 0 );
*/
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

//	Assert( w(p+4) == 56 );

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

//	Assert( w(p+4) == 56 );

	int frontlist = w(p+36);
	int backlist = w(p+40);
	int prelist = w(p+44);
	int postlist = w(p+48);
	int onlist = w(p+52);

	if (postlist) model_interp_sub(p+postlist,pm,sm,do_box_check);		// postlist

	if (g3_check_normal_facing(vp(p+20),vp(p+8))) {		//facing

		// 

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




void model_draw_debug_points( polymodel *pm, bsp_info * submodel )
{
	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	// Draw the brown "core" mass
//	if ( submodel && (submodel->parent==-1) )	{
//		gr_set_color(128,128,0);
//		g3_draw_sphere_ez( &vmd_zero_vector, pm->core_radius );
//	}

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
		//for (i=0; i<8; i++ )	{
		//	g3_rotate_vertex( &pts[i], &pm->bounding_box[i] );
		//}
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

// Debug code to show all the paths of a model
void model_draw_paths( int model_num )
{
	int i,j;
	vec3d pnt;
	polymodel * pm;	

	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
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
				vertex tmp;
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

// Debug code to show all the paths of a model
void model_draw_paths_htl( int model_num )
{
	int i,j;
	vec3d pnt;
	vec3d prev_pnt;
	polymodel * pm;	

	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
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

// docking bay and fighter bay paths
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


// docking bay and fighter bay paths
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
/*
// struct that holds the indicies into path information associated with a fighter bay on a capital ship
// NOTE: Fighter bay paths are identified by the path_name $bayN (where N is numbered from 1).
//			Capital ships only have ONE fighter bay on the entire ship
#define MAX_SHIP_BAY_PATHS		10
typedef struct ship_bay {
	int	num_paths;							// how many paths are associated with the model's fighter bay
	int	paths[MAX_SHIP_BAY_PATHS];		// index into polymodel->paths[] array
	int	arrive_flags;	// bitfield, set to 1 when that path number is reserved for an arrival
	int	depart_flags;	// bitfield, set to 1 when that path number is reserved for a departure
} ship_bay;

  typedef struct mp_vert {
	vec3d		pos;				// xyz coordinates of vertex in object's frame of reference
	int			nturrets;		// number of turrets guarding this vertex
	int			*turret_ids;	// array of indices into ship_subsys linked list (can't index using [] though)
	float			radius;			// How far the closest obstruction is from this vertex
} mp_vert;

typedef struct model_path {
	char			name[MAX_NAME_LEN];					// name of the subsystem.  Probably displayed on HUD
	char			parent_name[MAX_NAME_LEN];			// parent name of submodel that path is linked to in POF
	int			parent_submodel;
	int			nverts;
	mp_vert		*verts;
	int			goal;			// Which of the verts is the one closest to the goal of this path
	int			type;			// What this path takes you to... See MP_TYPE_??? defines above for details
	int			value;		// This depends on the type.
									// For MP_TYPE_UNUSED, this means nothing.
									// For MP_TYPE_SUBSYS, this is the subsystem number this path takes you to.
} model_path;
*/

void interp_render_arc_segment( vec3d *v1, vec3d *v2, int depth, color *outside_color, bool init_halfway )
{
	static color halfway_color;
	float d = vm_vec_dist_quick( v1, v2 );

	if(init_halfway)
	{
		gr_init_alphacolor(&halfway_color,
							outside_color->red + (255 - outside_color->red)/2,
							outside_color->green + (255-outside_color->green)/2,
							outside_color->blue + (255-outside_color->blue)/2,
							255);
	}

	if ( d < 0.30f || (depth>4) )	{
		vertex p1, p2;
		g3_rotate_vertex( &p1, v1 );
		g3_rotate_vertex( &p2, v2 );

		//g3_draw_rod( v1, 0.2f, v2, 0.2f, NULL, 0);
		gr_set_color_fast(outside_color);
		g3_draw_rod( v1, 0.6f, v2, 0.6f, NULL, TMAP_FLAG_RGB | TMAP_HTL_3D_UNLIT);
		gr_set_color_fast(&halfway_color);
		g3_draw_rod( v1, 0.45f, v2, 0.45f, NULL, TMAP_FLAG_RGB | TMAP_HTL_3D_UNLIT);
		gr_set_color(255,255,255);
		g3_draw_rod( v1, 0.3f, v2, 0.3f, NULL, TMAP_FLAG_RGB | TMAP_HTL_3D_UNLIT);
	//	g3_draw_line( &p1, &p2 );
	} else {
		// divide in half
		vec3d tmp;
		vm_vec_avg( &tmp, v1, v2 );
	
		float scaler = 0.30f;
		tmp.xyz.x += (frand()-0.5f)*d*scaler;
		tmp.xyz.y += (frand()-0.5f)*d*scaler;
		tmp.xyz.z += (frand()-0.5f)*d*scaler;
		
		interp_render_arc_segment( v1, &tmp, depth+1, outside_color, false );
		interp_render_arc_segment( &tmp, v2, depth+1, outside_color, false );
	}
}

int Interp_lightning = 1;
DCF_BOOL( Arcs, Interp_lightning )

int AR = 64;
int AG = 64;
int AB = 5;
int AR2 = 128;
int AG2 = 128;
int AB2 = 10;
void interp_render_lightning( polymodel *pm, bsp_info * sm )
{
	Assert( sm->num_arcs > 0 );

	int i;

	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	if (!Interp_lightning) return;

	color outside_arc_color;

//	if ( keyd_pressed[KEY_LSHIFT] ) return;
//	if ( rad < 3.0f ) return;	
	
	for (i=0; i<sm->num_arcs; i++ )	{
		// pick a color based upon arc type
		switch(sm->arc_type[i]){
		// "normal", FreeSpace 1 style arcs
		case MARC_TYPE_NORMAL:
			if ( (rand()>>4) & 1 )	{
				gr_init_alphacolor(&outside_arc_color, 64, 64, 255, 255 );
			} else {
				gr_init_alphacolor(&outside_arc_color, 128, 128, 255, 255 );
			}
			break;

		// "EMP" style arcs
		case MARC_TYPE_EMP:
			if ( (rand()>>4) & 1 )	{
				gr_init_alphacolor(&outside_arc_color, AR, AG, AB, 255 );
			} else {
				gr_init_alphacolor(&outside_arc_color, AR2, AG2, AB2, 255 );
			}
			break;

		default:
			Int3();
		}

		// render the actual arc segment
		interp_render_arc_segment( &sm->arc_pts[i][0], &sm->arc_pts[i][1], 0, &outside_arc_color, true );
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

//	mprintf(( "Name = '%s'\n", pm->submodel[mn].name ));
//	char * p = pm->submodel[mn].name;



//	mprintf(("model =%s, submodel=%s, interp offset=%f %f %f\n",pm->filename, pm->submodel[mn].name,Interp_offset.xyz.x,
//															Interp_offset.xyz.y,Interp_offset.xyz.z));
	
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

	vm_vec_add2(&Interp_offset,&pm->submodel[mn].offset);

	// Compute final submodel orientation by using the orientation matrix and the rotation angles.
	// By using this kind of computation, the rotational angles can always be computed relative
	// to the submodel itself, instead of relative to the parent - KeldorKatarn
	matrix rotation_matrix = pm->submodel[mn].orientation;
	vm_rotate_matrix_by_angles(&rotation_matrix, &pm->submodel[mn].angs);

	matrix inv_orientation;
	vm_copy_transpose_matrix(&inv_orientation, &pm->submodel[mn].orientation);

	matrix submodel_matrix;
	vm_matrix_x_matrix(&submodel_matrix, &rotation_matrix, &inv_orientation);

	g3_start_instance_matrix(&pm->submodel[mn].offset, &submodel_matrix, true);

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_rotate_all();
	}

	model_interp_sub( pm->submodel[mn].bsp_data, pm, &pm->submodel[mn], 0 );

	if (Interp_flags & MR_SHOW_PIVOTS )
		model_draw_debug_points( pm, &pm->submodel[mn] );

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

	vm_vec_sub2(&Interp_offset,&pm->submodel[mn].offset);


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
// Early out which we cannot do because we want to differentiate btwn
// IBOX_SOME_ON_SOME_OFF and IBOX_ALL_ON
//		if ( !codes )	{
//			//mprintf(( "A point is inside, so render it.\n" ));
//			return 0;		// this point is in, so return 0
//		}
		or_codes |= codes;
		and_codes &= codes;
	}

	// If and_codes is set this means that all points lie off to the
	// same side of the screen.
	if (and_codes)	{
		//mprintf(( "All points offscreen, so don't render it.\n" ));
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


//calls the object interpreter to render an object.  
//returns true if drew
int model_interp_sub(void *model_ptr, polymodel * pm, bsp_info *sm, int do_box_check )
{
	ubyte *p = (ubyte *)model_ptr;
	int chunk_type, chunk_size;
	int pushed = 0;

	chunk_type = w(p);
	chunk_size = w(p+4);

	
	while ( chunk_type != OP_EOF )	{

//		mprintf(( "Processing chunk type %d, len=%d\n", chunk_type, chunk_size ));

		switch (chunk_type) {
		case OP_EOF: return 1;
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


			if (Interp_flags & MR_SHOW_PIVOTS )	{
				#ifndef NDEBUG
				modelstats_num_boxes++;
				#endif
				interp_draw_box( vp(p+8), vp(p+20) );
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


void model_render_shields( polymodel * pm )
{
	model_set_outline_color(255,255,255);
	int i, j;
	shield_tri *tri;
	vertex pnt0, tmp, prev_pnt;

	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
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

void model_render_insignias(polymodel *pm, int detail_level)
{
	// if the model has no insignias, or we don't have a texture, then bail
	if ( (pm->num_ins <= 0) || (Interp_insignia_bitmap < 0) )
		return;


	int idx, s_idx;
	vertex vecs[3];
	vertex *vlist[3] = { &vecs[0], &vecs[1], &vecs[2] };
	vec3d t1, t2, t3, x;
	int i1, i2, i3;
	int tmap_flags = TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT;

	x.xyz.x=0;
	x.xyz.y=0;
	x.xyz.z=0;

	// set the proper texture	
	gr_set_bitmap(Interp_insignia_bitmap, GR_ALPHABLEND_NONE, GR_BITBLT_MODE_NORMAL, 0.65f);

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
			vecs[0].u = pm->ins[idx].u[s_idx][0];  vecs[0].v = pm->ins[idx].v[s_idx][0];
			vecs[1].u = pm->ins[idx].u[s_idx][1];  vecs[1].v = pm->ins[idx].v[s_idx][1];
			vecs[2].u = pm->ins[idx].u[s_idx][2];  vecs[2].v = pm->ins[idx].v[s_idx][2];

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

/*
typedef struct model_cache {
	int		model_num;
	//matrix	orient;
	vec3d	pos;
	int		num_lights;

	float		last_dot;

	float		cr;

	int		w, h;
	ubyte		*data;
	int		cached_valid;
	int		bitmap_id;

	angles	angs;

	// thrust stuff
	float		thrust_scale;
	int		thrust_bitmap;
	int		thrust_glow_bitmap;
	float		thrust_glow_noise;

	int		last_frame_rendered;		//	last frame in which this model was rendered not from the cache
} model_cache;

#define MAX_MODEL_CACHE MAX_OBJECTS
model_cache Model_cache[MAX_MODEL_CACHE];		// Indexed by objnum
int Model_cache_inited = 0;


// Returns 0 if not valid points
int model_cache_calc_coords(vec3d *pnt,float rad, float *cx, float *cy, float *cr)
{
	vertex pt;
	ubyte flags;

	flags = g3_rotate_vertex(&pt,pnt);

	if (flags == 0) {

		g3_project_vertex(&pt);

		if (!(pt.flags & (PF_OVERFLOW|CC_BEHIND)))	{

			*cx = pt.sx;
			*cy = pt.sy;
			*cr = rad*Matrix_scale.xyz.x*Canv_w2/pt.z;

			if ( *cr < 1.0f )	{
				*cr = 1.0f;
			}

			int x1, x2, y1, y2;

			x1 = fl2i(*cx-*cr); 
			if ( x1 < gr_screen.clip_left ) return 0;
			x2 = fl2i(*cx+*cr);
			if ( x2 > gr_screen.clip_right ) return 0;
			y1 = fl2i(*cy-*cr);
			if ( y1 < gr_screen.clip_top ) return 0;
			y2 = fl2i(*cy+*cr);
			if ( y2 > gr_screen.clip_bottom ) return 0;

			return 1;
		}
	}
	return 0;
}
*/

//draws a bitmap with the specified 3d width & height 
//returns 1 if off screen, 0 if not
int model_get_rotated_bitmap_points(vertex *pnt,float angle, float rad, vertex *v)
{
	float sa, ca;
	int i;

	Assert( G3_count == 1 );



//	angle = 0.0f;
		
	sa = (float)sin(angle);
	ca = (float)cos(angle);

	float width, height;

	width = height = rad;

	v[0].x = (-width*ca - height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[0].y = (-width*sa + height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[0].z = pnt->z;
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 0.0f;

	v[1].x = (width*ca - height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[1].y = (width*sa + height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[1].z = pnt->z;
	v[1].sw = 0.0f;
	v[1].u = 1.0f;
	v[1].v = 0.0f;

	v[2].x = (width*ca + height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[2].y = (width*sa - height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[2].z = pnt->z;
	v[2].sw = 0.0f;
	v[2].u = 1.0f;
	v[2].v = 1.0f;

	v[3].x = (-width*ca + height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[3].y = (-width*sa - height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[3].z = pnt->z;
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 1.0f;

	ubyte codes_and=0xff;

	float sw,z;
	z = pnt->z - rad / 4.0f;
	if ( z < 0.0f ) z = 0.0f;
	sw = 1.0f / z;

	for (i=0; i<4; i++ )	{
		//now code the four points
		codes_and &= g3_code_vertex(&v[i]);
		v[i].flags = 0;		// mark as not yet projected
		g3_project_vertex(&v[i]);
		v[i].sw = sw;
	}

	if (codes_and)
		return 1;		//1 means off screen

	return 0;
}

/*
int Model_caching = 1;
DCF_BOOL( model_caching, Model_caching )

extern int Tmap_scan_read;		// 0 = normal mapper, 1=read, 2=write

#define MODEL_MAX_BITMAP_SIZE 128
ubyte tmp_bitmap[MODEL_MAX_BITMAP_SIZE*MODEL_MAX_BITMAP_SIZE];

void mc_get_bmp( ubyte *data, int x, int y, int w, int h )
{
	gr_lock();

	int i,j;

	for (i = 0; i < h; i++)	{
		ubyte *dptr = GR_SCREEN_PTR(ubyte,x,i+y);
		ubyte *sptr = data+(i*w);
		for (j=0; j<w; j++ )	{
			*sptr++ = *dptr;
			*dptr++ = 255;				// XPARENT!
		}
	}

	gr_unlock();
}

void mc_put_bmp( ubyte *data, int x, int y, int w, int h )
{
	gr_lock();

	int i,j;

	for (i = 0; i < h; i++)	{
		ubyte *dptr = GR_SCREEN_PTR(ubyte,x,i+y);
		ubyte *sptr = data+(i*w);
		for (j=0; j<w; j++ )	{
			*dptr++ = *sptr++;
		}
	}

	gr_unlock();
}
*/

float Interp_depth_scale = 1500.0f;

DCF(model_darkening,"Makes models darker with distance")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT);
		Interp_depth_scale = Dc_arg_float;
	}

	if ( Dc_help )	{
		dc_printf( "Usage: model_darkening float\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "model_darkening = %.1f\n", Interp_depth_scale );
	}
}


		// Compare it to 999.75f at R = 64.0f
		//               0.0000f at R = 4.0f
		
		//float cmp_val = 999.75f;		// old
/*
// Return a number from 'min' to 'max' where it is
// 'v' is between v1 and v2.
float scale_it( float min, float max, float v, float v1, float v2 )
{
	if ( v < v1 ) return min;
	if ( v > v2 ) return max;

	v = (v - v1)/(v2-v1);
	v = v*(max-min)+min;

	return v;
}
*/

void model_render(int model_num, matrix *orient, vec3d * pos, uint flags, int objnum, int lighting_skip, int *replacement_textures)
{
	int cull = 0;
	// replacement textures - Goober5000
	model_set_replacement_textures(replacement_textures);

	polymodel *pm = model_get(model_num);


	model_do_dumb_rotation(model_num);


	int time = timestamp();
	for (int i = 0; i < pm->n_glow_point_banks; i++ ) { //glow point blink code -Bobboau
		glow_point_bank *bank = &pm->glow_point_banks[i];
		if (bank->glow_timestamp == 0)
			bank->glow_timestamp=time;
		if(bank->off_time){
		//	time += bank->disp_time;
			if(bank->is_on){
				if( (bank->on_time) > ((time - bank->disp_time) % (bank->on_time + bank->off_time)) ){
					bank->glow_timestamp=time;
					bank->is_on=0;
				}
			}else{
				if( (bank->off_time) < ((time - bank->disp_time) % (bank->on_time + bank->off_time)) ){
					bank->glow_timestamp=time;
					bank->is_on=1;
				}
			}
		}
	}


	// maybe turn off (hardware) culling
	if(flags & MR_NO_CULL){
		cull = gr_set_cull(0);
	}

	Interp_objnum = objnum;


/*
	vec3d thruster_pnt=vmd_zero_vector;
	vec3d rotated_thruster;
	//maybe add lights by engine glows
	if (Interp_flags & MR_SHOW_THRUSTERS) 
	{
		for (int thruster_num=0; thruster_num < pm->n_thrusters; thruster_num++)
		{
			for (int slot_num=0; slot_num < pm->thrusters[thruster_num].num_slots; slot_num++)
			{
				thruster_pnt=pm->thrusters[thruster_num].pnt[slot_num];
				//vm_vec_rotate(&rotated_thruster,&thruster_pnt,orient);
				
				light_add_point(&rotated_thruster,pm->thrusters[thruster_num].radius[slot_num]*5,pm->thrusters[thruster_num].radius[slot_num]*10,1.0f,0.0f,0.0f,1.0f,-1);

		//		gr_set_color(255,255,255);
			//	g3_draw_sphere_ez(&rotated_thruster,pm->thrusters[thruster_num].radius[slot_num]*15);
			}
		}
	}*/

	if ( flags & MR_NO_LIGHTING )	{
		Interp_light = 1.0f;

		// never use saved lighitng for this object
		model_set_saved_lighting(-1, -1);
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

		// never use saved lighitng for this object
		model_set_saved_lighting(-1, -1);
	} else {
		Interp_light = 1.0f;

		// maybe use saved lighting
		model_set_saved_lighting(objnum, hack_skip_max);
	}

	int num_lights = 0;

	if ( !(flags & MR_NO_LIGHTING ) )	{
		num_lights = light_filter_push( objnum, pos, pm->rad );
	}

	model_try_cache_render(model_num, orient, pos, flags, objnum, num_lights );
	
	if ( !(flags & MR_NO_LIGHTING ) )	{
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
}

/*
void model_cache_init()
{
	if ( !Model_cache_inited )	{
		int i;

		Model_cache_inited = 1;

		for (i=0; i<MAX_MODEL_CACHE; i++ )	{
			Model_cache[i].cached_valid = 0;
			Model_cache[i].data = NULL;
			Model_cache[i].bitmap_id = -1;
			Model_cache[i].last_frame_rendered = -1;
		}
	}
}

void model_cache_reset()
{
	if ( Model_cache_inited )	{
		int i;

		for (i=0; i<MAX_MODEL_CACHE; i++ )	{
			Model_cache[i].cached_valid = 0;
			if ( Model_cache[i].data )	{
				vm_free(Model_cache[i].data);
				Model_cache[i].data = NULL;
			}
			if ( Model_cache[i].bitmap_id != -1 )	{
				bm_release(Model_cache[i].bitmap_id);
				Model_cache[i].bitmap_id = -1;
			}
		}
	}
}
*/

// tmp_detail_level
// 0 - Max
// 1
// 2
// 3
// 4 - None

#if MAX_DETAIL_LEVEL != 4
#error MAX_DETAIL_LEVEL is assumed to be 4 in ModelInterp.cpp
#endif

// Given detail level, what is the threshold for how far viewer
// can move in the object's frame of reference before a redraw.
float Mc_viewer_pos_factor[MAX_DETAIL_LEVEL+1] = {  0.080f, 0.040f, 0.020f, 0.010f, 0.0f };
float Mc_size_factor[MAX_DETAIL_LEVEL+1] = {  1.40f, 1.30f, 1.20f, 1.10f, 0.0f };

int Model_object_caching_tmp = MAX_DETAIL_LEVEL;

// When framerate goes below this, knock it down a notch.
float Mc_framerate_lo[MAX_DETAIL_LEVEL+1] = { 0.0f, 10.0f, 15.0f, 20.0f, 25.0f };
// When it goes above this, knock it up a notch.
float Mc_framerate_hi[MAX_DETAIL_LEVEL+1] = { 15.0f, 20.0f, 25.0f, 30.0f, 100000.0f };

int Mc_detail_add[MAX_DETAIL_LEVEL+1] = { -2, -1, +1, +2, +4 };

void model_try_cache_render(int model_num, matrix *orient, vec3d * pos, uint flags, int objnum, int num_lights)
{
	model_really_render(model_num, orient, pos, flags, objnum);
	/*
	int i;

	model_cache *mc = NULL;
	
	if ( (objnum >= 0) && (objnum<MAX_MODEL_CACHE) )	{
		mc = &Model_cache[objnum];
	}
	
	if ( (!mc) || (!Model_caching) || (D3D_enabled) || (!Model_cache_inited) || (flags & MR_ALWAYS_REDRAW) || (Detail.object_caching > 3) )	{
		if ( mc )	{
			mc->cached_valid = 0;
		}
		model_really_render(model_num, orient, pos, flags, objnum );
		return;
	}

	Assert( mc != NULL );

	// Fake the detail level based on framerate.
	if ( 1.0f / flFrametime < Mc_framerate_lo[Model_object_caching_tmp] )	{
		Model_object_caching_tmp--;
		//	mprintf(( "Model cache level bumped down to %d\n", Model_object_caching ));
	} else if ( 1.0f / flFrametime > Mc_framerate_hi[Model_object_caching_tmp] )	{
		Model_object_caching_tmp++;
		//	mprintf(( "Model cache level bumped up to %d\n", Model_object_caching ));
	}

	int tmp_detail_level = Model_object_caching_tmp + Mc_detail_add[Detail.object_caching];

	if ( tmp_detail_level < 0 )	{
		tmp_detail_level = 0;
	} else if (tmp_detail_level > MAX_DETAIL_LEVEL )  {
		tmp_detail_level = MAX_DETAIL_LEVEL;
	}

	if ( tmp_detail_level > 3 )	{
		if ( mc )	{
			mc->cached_valid = 0;
		}
		model_really_render(model_num, orient, pos, flags, objnum );
		return;
	}

	
//	static int last_one = -1;
//	if ( last_one != tmp_detail_level )	{
//		last_one = tmp_detail_level;
//		mprintf(( "Detail level %d\n", tmp_detail_level ));
//	}

//	if ( keyd_pressed[KEY_LSHIFT] )	{
//		mc->cached_valid = 0;
//		model_really_render(model_num, orient, pos, flags, objnum );
//		return;
//	}


//	mprintf(( "Rendering cache model\n" ));

	polymodel *pm = model_get(model_num);
	vertex v[4];
	vertex *vertlist[4] = { &v[0], &v[1], &v[2], &v[3] };
	float cx, cy, cr;
	vertex pt;
	ubyte ccflags;

	matrix tempm, tempm2;
	angles new_angles;

	vm_copy_transpose_matrix(&tempm2,orient);
	vm_matrix_x_matrix(&tempm,&tempm2,&Eye_matrix);
	vm_extract_angles_matrix(&new_angles, &tempm );
	
	if ( !model_cache_calc_coords(pos,pm->rad, &cx, &cy, &cr) )	{
		// Not onscreen, do a real render and exit
		mc->cached_valid = 0;
		model_really_render(model_num, orient, pos, flags, objnum );
		return;
	}

	//================================================================
	// A bunch of checks to see if we need to redraw the model or not

	
	vec3d ship_to_eye;

	vm_vec_sub( &ship_to_eye, &Eye_position, pos );
	vm_vec_normalize_safe(&ship_to_eye);
	float this_dot = vm_vec_dot( &ship_to_eye, &orient->fvec );
	this_dot += vm_vec_dot( &ship_to_eye, &orient->rvec );

	float diff = 0.0f;
	
	if ( !mc->cached_valid )	{
		// Nothing cached
		goto RedrawIt;
	}

	Assert( mc->data != NULL );

	if (Framecount - mc->last_frame_rendered > 1 + 2*(MAX_DETAIL_LEVEL - Detail.object_caching - 1)) {
		goto RedrawIt;
	}

	diff = fl_abs( this_dot - mc->last_dot );

	if ( diff > Mc_viewer_pos_factor[tmp_detail_level] )	{
//		mprintf(( "Redraw!!! %.4f\n", diff ));
		goto RedrawIt;
	}

//	if ( keyd_pressed[KEY_LSHIFT] )	{
//		goto RedrawIt;
//	}

	if (tmp_detail_level > 2)	{
		if ( mc->thrust_glow_bitmap != Interp_thrust_glow_bitmap )	{
			// Engline glow bitmap changed
			//	mprintf(( "MC: Glow bitmap changed! %d -> %d\n", mc->thrust_glow_bitmap, Interp_thrust_glow_bitmap ));
			goto RedrawIt;
		}
	}

	if (tmp_detail_level > 2)	{
		if ( cr > 4.0f ) {
			float diff = fl_abs( mc->thrust_scale - Interp_thrust_scale );

			if ( diff > 0.1f )	{
				// Thruster size has changed
				//mprintf(( "MC: Thruster size changed! %.2f -> %.2f\n", mc->thrust_scale, Interp_thrust_scale ));
				goto RedrawIt;
			}
		}
	}

//		if (0) {
//			float diff = fl_abs( mc->thrust_glow_noise - Interp_thrust_glow_noise );

//			if ( diff > 0.1f )	{
				// Glow noise has changed
				//mprintf(( "MC: Thruster glow changed! %.2f -> %.2f\n", mc->thrust_glow_noise, Interp_thrust_glow_noise ));
//				goto RedrawIt;
//			}
//		}


	if ( mc->model_num != model_num )	{
		// Model changed
		goto RedrawIt;
	}

	if ( cr>mc->cr*Mc_size_factor[tmp_detail_level] )	{
		// Scaling up too far
		goto RedrawIt;
	}
		
	if (tmp_detail_level > 2)	{
		if ( cr > 4.0f )	{
			if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
				if (mc->num_lights != num_lights)	{
					// Lighting changed
					goto RedrawIt;
				}
			}
		}
	}

		// This method is correct, but rotating ship makes things redraw which is too slow.
	#if 0
		if ( cr > 4.0f )	{
			// Check orientation
			float angle_error = MAX( fl_abs( mc->angs.p-new_angles.p ),fl_abs( mc->angs.h-new_angles.h ));

			// Exact
			//if ( angle_error > 0.075f  )	{	

			// Rough
			if ( angle_error > 0.40f  )	{	
				// Ship/view turned too much
				//mprintf(( "Ship/view turned too much %.4f\n", angle_error ));

				goto RedrawIt;
			}
		}
	#endif


//		mprintf(( "Dot = %.5f\n", dot ));

#if 0
	if (0) {
		float dx, dy, dz;

		dx = vm_vec_dot( &orient->rvec, &mc->orient.rvec )+1.0f;
		dy = vm_vec_dot( &orient->uvec, &mc->orient.uvec )+1.0f;
		dz = vm_vec_dot( &orient->fvec, &mc->orient.fvec )+1.0f;
			
		float angle_error = (dx+dy+dz)*1000.0f/6.0f;		

		//mprintf(( "Angle_error = %.4f\n", angle_error ));

		// Compare it to 999.75f at R = 64.0f
		//               0.0000f at R = 0.0f
		
		float cmp_val = 999.75f;		// old
//			if ( is_asteroid )	{
//				cmp_val = scale_it( 0.0f, 999.75f, cr, 0.0f, 64.0f );
//			}
											
		if ( angle_error < cmp_val ) {
			// Ship turned too much
			goto RedrawIt;
		}
	}	
#endif


	// Have a valid cache entry, mc
	ccflags = g3_rotate_vertex(&pt,pos);

	if ( ccflags )	{
		// offscreen		
		goto RedrawIt;
	}

	if ( model_get_rotated_bitmap_points(&pt,mc->angs.b - new_angles.b, pm->rad, v ))	{
		// offscreen		
		goto RedrawIt;
	}


	gr_set_bitmap( mc->bitmap_id );

	Tmap_scan_read = 2;
	g3_draw_poly(4, vertlist, TMAP_FLAG_TEXTURED );	
	Tmap_scan_read = 0;

	//	if ( keyd_pressed[KEY_LSHIFT] )	{
	//	gr_set_color( 255, 0, 0 );
	//	gr_pixel( fl2i(v[0].sx), fl2i(v[0].sy) );
	//	}

	//if ( keyd_pressed[KEY_RSHIFT] )	{
	//	gr_line( fl2i(v[0].sx), fl2i(v[0].sy), fl2i(v[1].sx), fl2i(v[1].sy), false );
	//	gr_line( fl2i(v[1].sx), fl2i(v[1].sy), fl2i(v[2].sx), fl2i(v[2].sy), false );
	//	gr_line( fl2i(v[2].sx), fl2i(v[2].sy), fl2i(v[3].sx), fl2i(v[3].sy), false );
	//	gr_line( fl2i(v[3].sx), fl2i(v[3].sy), fl2i(v[0].sx), fl2i(v[0].sy), false );
	//}


	return;


	//==========================================================
	// Cache is bad for model, so draw it and save it
RedrawIt:


//	if ( mc->data != NULL )	{
//		vm_free(mc->data);
//		mc->data = NULL;
//	}

	if ( mc->bitmap_id != -1 )	{
		bm_release(mc->bitmap_id);
		mc->bitmap_id = -1;
	}

	mc->cached_valid = 0;
	mc->model_num = model_num;
	mc->pos = *pos;
	//mc->orient = *orient;
	mc->cr = cr;
	mc->angs = new_angles;	//-Physics_viewer_bank;

	mc->thrust_scale = Interp_thrust_scale;
	mc->thrust_bitmap = Interp_thrust_bitmap;
	mc->thrust_glow_bitmap = Interp_thrust_glow_bitmap;
	mc->thrust_glow_noise = Interp_thrust_glow_noise;

	mc->last_dot = this_dot;

	if ( cr > MODEL_MAX_BITMAP_SIZE/2-1 )	
		goto JustDrawIt;

	//Physics_viewer_bank

	ccflags = g3_rotate_vertex(&pt,pos);

	if ( ccflags ) {
		goto JustDrawIt;
	}

	model_get_rotated_bitmap_points(&pt,0.0f, pm->rad, v );
				
	int x1, y1, x2, y2, w, h;

	x1 = fl_round_2048( v[0].sx );
	y1 = fl_round_2048( v[0].sy );

	x2 = fl_round_2048( v[2].sx );	//+0.5f );
	y2 = fl_round_2048( v[2].sy );	//+0.5f );

	if ( x1 < gr_screen.clip_left)	
		goto JustDrawIt;
	
	if ( y1 < gr_screen.clip_top )
		goto JustDrawIt;
	
	if ( x2 > gr_screen.clip_right)
		goto JustDrawIt;

	if ( y2 > gr_screen.clip_bottom) 
		goto JustDrawIt;

	w = x2 - x1 + 1;	
	if ( w < 0 )
		Int3();

	if ( w < 2 ) 
		w = 2;

	h = y2 - y1 + 1;	

	if ( h < 0 )
		Int3();

	if ( h < 2 ) 
		h = 2;

	if ( w > MODEL_MAX_BITMAP_SIZE )
		goto JustDrawIt;
		
	if ( h > MODEL_MAX_BITMAP_SIZE )
		goto JustDrawIt;

	mc->w = w;
	mc->h = h;

//	mprintf(( "Mallocing a %dx%d bitmap\n", w, h ));

	if ( mc->data == NULL )	{
		mc->data = (ubyte *)vm_malloc( MODEL_MAX_BITMAP_SIZE * MODEL_MAX_BITMAP_SIZE );
	}

//	mprintf(( "Done mallocing a %dx%d bitmap\n", w, h ));

	if ( mc->data == NULL )	{
		goto JustDrawIt;
	}
	for (i = 0; i < w*h; i++)	{
		mc->data[i] = 255;
	}


	mc->bitmap_id = bm_create( 8, mc->w, mc->h, mc->data, 0 );

	if ( mc->bitmap_id < 0 )	{
		goto JustDrawIt;
	}

	// Save stars and stuff on screen
	mc_get_bmp( tmp_bitmap, x1, y1, w, h );

	mc->num_lights = num_lights;

	// Didn't render a cached one... so render it and then save it in the cache

	// Turn on stippling
	model_really_render(model_num, orient, pos, flags, objnum );

	// Save screen to bitmap 
	gr_set_bitmap( mc->bitmap_id );
	Tmap_scan_read = 1;
	g3_draw_poly(4, vertlist, TMAP_FLAG_TEXTURED );	
	Tmap_scan_read = 0;

	// Restore stars and stuff to screen
	mc_put_bmp( tmp_bitmap, x1, y1, w, h );

	// Draw the model
	gr_set_bitmap( mc->bitmap_id );
	Tmap_scan_read = 2;
	g3_draw_poly(4, vertlist, TMAP_FLAG_TEXTURED );	
	Tmap_scan_read = 0;

	mc->cached_valid = 1;
	mc->last_frame_rendered = Framecount;
	return;
	
JustDrawIt:

	// Too big to save
	model_really_render(model_num, orient, pos, flags, objnum );
	*/
}

// Find the distance from p0 to the closest point on a box.
// The box's dimensions from 'min' to 'max'.
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
DCF(tiling, "")
{
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

//	vm_vec_sub( &rvec, &View, &temp );
	vm_vec_sub( &rvec, Eyeposition, &temp );
	vm_vec_normalize( &rvec );	

	vm_vec_crossprod(&uvec,fvec,&rvec);
	vm_vec_normalize(&uvec);

	vm_vec_scale_add( top, &temp, &uvec, w/2.0f );
	vm_vec_scale_add( bot, &temp, &uvec, -w/2.0f );	

//	Int3();
}

geometry_batcher primary_thruster_batcher, secondary_thruster_batcher, tertiary_thruster_batcher;

// maybe draw mode thruster glows
void model_render_thrusters(polymodel *pm, int objnum, ship *shipp, matrix *orient, vec3d *pos)
{
	int i, j, k;
	int n_q = 0;
	vec3d norm, norm2, fvec, pnt, npnt;
	thruster_bank *bank = NULL;
	vertex p;
	bool do_render = false;

	if ( pm == NULL ) {
		Int3();
		return;
	}

	if ( !(Interp_flags & MR_SHOW_THRUSTERS) /*|| !(Detail.engine_glows)*/ )
		return;


	// get an initial count to figure out how man geo batchers we need allocated
	for (i = 0; i < pm->n_thrusters; i++ ) {
		bank = &pm->thrusters[i];

		n_q += bank->num_points;
	}

	if (n_q <= 0) {
		return;
	}

	if (Interp_thrust_glow_bitmap >= 0) {
		primary_thruster_batcher.allocate(n_q);
		do_render = true;
	}

	if (Interp_secondary_thrust_glow_bitmap >= 0) {
		secondary_thruster_batcher.allocate(n_q);
		do_render = true;
	}

	if (Interp_tertiary_thrust_glow_bitmap >= 0) {
		tertiary_thruster_batcher.allocate(n_q);
		do_render = true;
	}

	if (do_render == false) {
		return;
	}

	// this is used for the secondary thruster glows 
	// it only needs to be calculated once so I'm doing it here -Bobboau
	/* norm = bank->norm[j] */;
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
		bank = &pm->thrusters[i];

		// don't draw this thruster if the engine is destroyed or just not on
		if ( !model_should_render_engine_glow(objnum, bank->obj_num) )
			continue;

		for (j = 0; j < bank->num_points; j++) {
			Assert( bank->points != NULL );

			float d, D;
			vec3d tempv;
			glow_point *gpt = &bank->points[j];

			vm_vec_sub(&tempv, &View_position, &gpt->pnt);
			vm_vec_normalize(&tempv);

			D = d = vm_vec_dot(&tempv, &gpt->norm);

			//ADAM: Min throttle draws rad*MIN_SCALE, max uses max.
			#define NOISE_SCALE 0.5f
			#define MIN_SCALE 3.4f
			#define MAX_SCALE 4.7f
			float scale = MIN_SCALE;

			float magnitude;
			vec3d scale_vec = { { { 1.0f, 0.0f, 0.0f } } };

			// normalize banks, in case of incredibly big normals
			// VECMAT-ERROR: NULL VEC3D (norm == nul)
			if ( !IS_VEC_NULL_SQ_SAFE(&gpt->norm) )
				vm_vec_copy_normalize(&scale_vec, &gpt->norm);

			// adjust for thrust
			(scale_vec.xyz.x *= Interp_thrust_scale_x) -= 0.1f;
			(scale_vec.xyz.y *= Interp_thrust_scale_y) -= 0.1f;
			(scale_vec.xyz.z *= Interp_thrust_scale)   -= 0.1f;

			// get magnitude, which we will use as the scaling reference
			magnitude = vm_vec_normalize(&scale_vec);

			// get absolute value
			if (magnitude < 0.0f)
				magnitude *= -1.0f;

			scale = magnitude * (MAX_SCALE - MIN_SCALE) + MIN_SCALE;
		//	scale = (Interp_thrust_scale-0.1f)*(MAX_SCALE-MIN_SCALE)+MIN_SCALE;

			if (d > 0.0f){
				// Make glow bitmap fade in/out quicker from sides.
				d *= 3.0f;

				if (d > 1.0f)
					d = 1.0f;
			}

			float fog_int = 1.0f;

			// fade them in the nebula as well
			if (The_mission.flags & MISSION_FLAG_FULLNEB) {
				vm_vec_rotate(&npnt, &gpt->pnt, orient);
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
				g3_rotate_vertex( &p, &gpt->pnt );
			} else {
				g3_transfer_vertex( &p, &gpt->pnt );
			}

			if ( (Interp_thrust_glow_bitmap >= 0) && (d > 0.0f) ) {
				p.r = p.g = p.b = p.a = (ubyte)(255.0f * d);

				primary_thruster_batcher.draw_bitmap( &p, 0, (w * 0.5f * Interp_thrust_glow_rad_factor), (w * 0.325f) );
			}
			// end primary thruster glows

			// start tertiary thruster glows
			if (Interp_tertiary_thrust_glow_bitmap >= 0) {
				// tertiary thruster glows, suposet to be a complement to the secondary thruster glows, it simulates the effect of an ion wake or something, 
				// thus is mostly for haveing a glow that is visable from the front
				p.sw -= w;

				p.r = p.g = p.b = p.a = (ubyte)(255.0f * fog_int);

				tertiary_thruster_batcher.draw_bitmap( &p, 0, (w * 0.6f * Interp_tertiary_thrust_glow_rad_factor), (magnitude * 4), (-(D > 0) ? D : -D) );
			}
			// end tertiary thruster glows

			// begin secondary glows ....
			if (Interp_secondary_thrust_glow_bitmap >= 0) {
				// secondary thruster glows, they are based on the beam rendering code
				// they are suposed to simulate... an ion wake... or... something
				// ok, how's this there suposed to look cool! hows that, 
				// it that scientific enough for you!! you anti-asthetic basturds!!!
				// AAAHHhhhh!!!!
				pnt = gpt->pnt;

				scale = magnitude * (MAX_SCALE - (MIN_SCALE / 2)) + (MIN_SCALE / 2);

				d = vm_vec_dot(&tempv, &norm);
				d += 0.75f;
				d *= 3.0f;

				if (d > 1.0f)
					d = 1.0f;

				if (d > 0.0f) {
					vm_vec_add(&norm2, &norm, &pnt);
					vm_vec_sub(&fvec, &norm2, &pnt);
					vm_vec_normalize(&fvec);

					float wVal = gpt->radius * scale * 2;

					vm_vec_scale_add(&norm2, &pnt, &fvec, wVal * 2 * Interp_thrust_glow_len_factor);

					if (The_mission.flags & MISSION_FLAG_FULLNEB) {
						vm_vec_add(&npnt, &pnt, pos);
						d *= fog_int;
					}

					secondary_thruster_batcher.draw_beam(&pnt, &norm2, wVal*Interp_secondary_thrust_glow_rad_factor*0.5f, d);
				}
			}
			// end secondary glows

			// begin particles
			if (shipp) {
				ship_info *sip = &Ship_info[shipp->ship_info_index];
				particle_emitter	pe;
				thruster_particles *tp;
				int num_particles = 0;

				if (Interp_afterburner)
					num_particles = (int)sip->afterburner_thruster_particles.size();
				else
					num_particles = (int)sip->normal_thruster_particles.size();

				for (k = 0; k < num_particles; k++) {
					if (Interp_afterburner)
						tp = &sip->afterburner_thruster_particles[k];
					else
						tp = &sip->normal_thruster_particles[k];

					float v = vm_vec_mag_quick(&Objects[shipp->objnum].phys_info.desired_vel);

					vm_vec_unrotate(&npnt, &gpt->pnt, orient);
					vm_vec_add2(&npnt, pos);

					pe.pos = npnt;				// Where the particles emit from
					pe.vel = Objects[shipp->objnum].phys_info.desired_vel;	// Initial velocity of all the particles
					pe.min_vel = v * 0.75f;
					pe.max_vel =  v * 1.25f;
	
					pe.normal = orient->vec.fvec;	// What normal the particle emit around
					vm_vec_negate(&pe.normal);

					pe.num_low = tp->n_low;								// Lowest number of particles to create
					pe.num_high = tp->n_high;							// Highest number of particles to create
					pe.min_rad = gpt->radius * tp->min_rad; // * objp->radius;
					pe.max_rad = gpt->radius * tp->max_rad; // * objp->radius;
					pe.normal_variance = tp->variance;					//	How close they stick to that normal 0=on normal, 1=180, 2=360 degree

					particle_emit( &pe, PARTICLE_BITMAP, tp->thruster_bitmap.first_frame);
				}
				// end particles

				// do sound - maybe start a random sound, if it has played far enough.
			}
		}
	}

	// save current zbuffer, and set the correct mode for us
	int zbuff_save = gr_zbuffering_mode;
	gr_zbuffer_set(GR_ZBUFF_READ);

	if (Interp_thrust_glow_bitmap >= 0) {
		gr_set_bitmap( Interp_thrust_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );
		primary_thruster_batcher.render(TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
	}

	if (Interp_secondary_thrust_glow_bitmap >= 0) {
		gr_set_bitmap(Interp_secondary_thrust_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		
		secondary_thruster_batcher.render(TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT);
	}

	if (Interp_tertiary_thrust_glow_bitmap >= 0) {
		gr_set_bitmap( Interp_tertiary_thrust_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );
		tertiary_thruster_batcher.render(TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
	}

	// reset zbuffer to original setting
	gr_zbuffer_set(zbuff_save);
}


void light_set_all_relevent();

//#define mSTUFF_VERTICES()	do { verts[0]->u = 0.0f; verts[0]->v = 0.0f;	verts[1]->u = 1.0f; verts[1]->v = 0.0f; verts[2]->u = 1.0f;	verts[2]->v = 1.0f; verts[3]->u = 0.0f; verts[3]->v = 1.0f; } while(0);
//#define mR_VERTICES()		do { g3_rotate_vertex(verts[0], &bottom1); g3_rotate_vertex(verts[1], &bottom2);	g3_rotate_vertex(verts[2], &top2); g3_rotate_vertex(verts[3], &top1); } while(0);
//#define mP_VERTICES()		do { for(idx=0; idx<4; idx++){ g3_project_vertex(verts[idx]); } } while(0);

extern int Warp_model;

void model_really_render(int model_num, matrix *orient, vec3d * pos, uint flags, int objnum )
{
	int i;
	int cull = 0;
	polymodel * pm;

	uint save_gr_zbuffering_mode;
	int zbuf_mode;
	ship *shipp = NULL;
	object *objp = NULL;
	bool set_autocen = false;

	// just to be on the safe side
	Assert( Interp_objnum == objnum );

	if (objnum >= 0) {
		objp = &Objects[objnum];

		if (objp->type == OBJ_SHIP) {
			shipp = &Ships[objp->instance];

			if (shipp->flags2 & SF2_GLOWMAPS_DISABLED)
				flags |= MR_NO_GLOWMAPS;
		}
	}

	if (FULLCLOAK != 1)
		model_interp_sortnorm = model_interp_sortnorm_b2f;


	MONITOR_INC( NumModelsRend, 1 );	

	Interp_orient = orient;
	Interp_pos = pos;
	memset(&Interp_offset,0,sizeof(vec3d));

	int tmp_detail_level = Game_detail_level;
	
	//	Tmap_show_layers = 1;
//	model_set_detail_level(0);
//	flags |= MR_LOCK_DETAIL|MR_NO_TEXTURING|MR_NO_LIGHTING;		//MR_LOCK_DETAIL |	|MR_NO_LIGHTING|MR_NO_SMOOTHINGMR_NO_TEXTURING | 

	// Turn off engine effect
	Interp_thrust_scale_subobj=0;

	if (!Model_texturing)
		flags |= MR_NO_TEXTURING;

	if ( !Model_polys )	{
		flags |= MR_NO_POLYS;
	}

	Interp_flags = flags;			 

	pm = model_get(model_num);	

	// Set the flags we will pass to the tmapper
	Interp_tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB;

	// if we're in nebula mode, fog everything except for the warp holes and other non-fogged models
	if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE) && !(flags & MR_NO_FOGGING)){
		Interp_tmap_flags |= TMAP_FLAG_PIXEL_FOG;
	}


	if ( !(Interp_flags & MR_NO_TEXTURING) )	{
		Interp_tmap_flags |= TMAP_FLAG_TEXTURED;

		if ( (pm->flags & PM_FLAG_ALLOW_TILING) && tiling)
			Interp_tmap_flags |= TMAP_FLAG_TILED;

		if ( !(Interp_flags & MR_NO_CORRECT) )	{
			Interp_tmap_flags |= TMAP_FLAG_CORRECT;
		}
	}

	save_gr_zbuffering_mode = gr_zbuffering_mode;
	zbuf_mode = gr_zbuffering_mode;

	if (!(Game_detail_flags & DETAIL_FLAG_MODELS) )	{
		gr_set_color(0,128,0);
		g3_draw_sphere_ez( pos, pm->rad );
	//	if(!Cmdline_nohtl)gr_set_lighting(false,false);
		return;
	}


	bool is_outlines_only = (flags & MR_NO_POLYS) && ((flags & MR_SHOW_OUTLINE_PRESET) || (flags & MR_SHOW_OUTLINE));
	bool is_outlines_only_htl = !Cmdline_nohtl && (flags & MR_NO_POLYS) && (flags & MR_SHOW_OUTLINE_HTL);
	bool use_api = !is_outlines_only_htl || (gr_screen.mode == GR_OPENGL);


	g3_start_instance_matrix(pos, orient, use_api);

	if ( Interp_flags & MR_SHOW_RADIUS )	{
		if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
			gr_set_color(0,64,0);
			g3_draw_sphere_ez(&vmd_zero_vector,pm->rad);
		}
	}

	Assert( pm->n_detail_levels < MAX_MODEL_DETAIL_LEVELS );

	vec3d closest_pos;
	float depth = model_find_closest_point( &closest_pos, model_num, -1, orient, pos, &Eye_position );
	if ( pm->n_detail_levels > 1 )	{

		if ( Interp_flags & MR_LOCK_DETAIL )	{
			i = Interp_detail_level_locked+1;
		} else {

			//gr_set_color(0,128,0);
			//g3_draw_sphere_ez( &closest_pos, 2.0f );

			#if MAX_DETAIL_LEVEL != 4
			#error Code in modelInterp.cpp assumes MAX_DETAIL_LEVEL == 4
			#endif

			switch( Detail.detail_distance )	{
			case 0:		// lowest
				depth *= 8.0f;
				break;
			case 1:		// lower than normal
				depth *= 4.0f; 
				break;
			case 2:		// default  (leave the same)
				break;
			case 3:		// above normal
				depth /= 4.0f;
				break;
			case 4:		// even more normal
				depth /= 8.0f;
				break;
			}

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


		// maybe force lower detail
		if (Interp_flags & MR_FORCE_LOWER_DETAIL) {
			i++;
		}

		//Interp_detail_level = fl2i(depth/10.0f);		
		//Interp_detail_level = 0;
		Interp_detail_level = i-1-tmp_detail_level;

		if ( Interp_detail_level < 0 ) 
			Interp_detail_level = 0;
		else if (Interp_detail_level >= pm->n_detail_levels ) 
			Interp_detail_level = pm->n_detail_levels-1;

		//mprintf(( "Depth = %.2f, detail = %d\n", depth, Interp_detail_level ));

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

	if (Interp_flags & MR_AUTOCENTER) {
		vec3d auto_back = ZERO_VECTOR;

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

	gr_zbias(1);

	if(!Cmdline_nohtl) {
		light_set_all_relevent();
	}

	if(Interp_tmap_flags & TMAP_FLAG_PIXEL_FOG)
	{
		float fog_near = 10.0f, fog_far = 1000.0f;
		neb2_get_fog_values(&fog_near, &fog_far, objp);
		unsigned char r, g, b;
		neb2_get_fog_color(&r, &g, &b);
		gr_fog_set(GR_FOGMODE_FOG, r, g, b, fog_near, fog_far);
	}

	// Draw the subobjects	
	i = pm->submodel[pm->detail[Interp_detail_level]].first_child;

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

	while( i >= 0 )	{
		if (!pm->submodel[i].is_thruster )	{
			zbuf_mode = GR_ZBUFF_FULL;

			// no zbuffering
			if(Interp_flags & MR_NO_ZBUFFER){
				zbuf_mode = GR_ZBUFF_NONE;
			}

			gr_zbuffer_set(zbuf_mode);

			// When in htl mode render with htl method unless its a jump node
			if (is_outlines_only_htl || (!Cmdline_nohtl && !is_outlines_only)) {
				model_render_children_buffers(&pm->submodel[i], pm, i, Interp_detail_level);
			} else {
				model_interp_subcall( pm, i, Interp_detail_level );
			}
		} 
		i = pm->submodel[i].next_sibling;
	}	

	// rotate lights for the hull
	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_rotate_all();
	}else {
		// This NULL func ptr was causing a crash in non htl
		if(!Cmdline_nohtl) {
			gr_reset_lighting();
		}
	}

	if ( pm->submodel[pm->detail[Interp_detail_level]].num_children > 0 ){
//		zbuf_mode |= GR_ZBUFF_WRITE;		// write only
		zbuf_mode = GR_ZBUFF_FULL;
	}

	// no zbuffering
	if(Interp_flags & MR_NO_ZBUFFER){
		zbuf_mode = GR_ZBUFF_NONE;
	}
	
	gr_zbuffer_set(zbuf_mode);
	gr_zbias(0);	

	model_radius = pm->submodel[pm->detail[Interp_detail_level]].rad;

	//*************************** draw the hull of the ship *********************************************

	// When in htl mode render with htl method unless its a jump node
	if (is_outlines_only_htl || (!Cmdline_nohtl && !is_outlines_only)) {
		model_render_buffers(&pm->submodel[pm->detail[Interp_detail_level]], pm);
	} else {
		model_interp_subcall(pm,pm->detail[Interp_detail_level], Interp_detail_level);
	}

	gr_set_fill_mode(GR_FILL_MODE_SOLID);


	if(!Cmdline_nohtl)gr_set_lighting(true,true);

	if ( (objp != NULL) && Cmdline_decals) {
		vec3d decal_z_corection = View_position;
	//	vm_vec_sub(&decal_z_corection, &Eye_position, pos);
		if(decal_z_corection.xyz.x != 0 && decal_z_corection.xyz.y != 0 && decal_z_corection.xyz.z != 0)
			vm_vec_normalize(&decal_z_corection);
		float corection = 0.2f;
		vm_vec_scale(&decal_z_corection, corection);
		g3_start_instance_matrix(&decal_z_corection, NULL, use_api);		
		decal_render_all(objp);
		g3_done_instance(use_api);
		//Nasty hack to get the other Get_fog functions to work
		neb2_get_fog_intensity(objp);
	}


	if(!Cmdline_nohtl){
		gr_reset_lighting();
		gr_set_lighting(false,false);
	}

	if (Interp_flags & MR_SHOW_PIVOTS )	{
		model_draw_debug_points( pm, NULL );
		model_draw_debug_points( pm, &pm->submodel[pm->detail[Interp_detail_level]] );

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
	model_render_insignias(pm, Interp_detail_level);	

	gr_zbias(0);  

	gr_set_lighting(false,false);

	if (FULLCLOAK != -1)	model_finish_cloak(FULLCLOAK);

	if ( pm->submodel[pm->detail[0]].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[pm->detail[0]]);
	}	

	if ( Interp_flags & MR_SHOW_SHIELDS )	{
		model_render_shields(pm);
	}	

	// start rendering glow points -Bobboau
	if ( (pm->n_glow_point_banks) && !is_outlines_only && !is_outlines_only_htl && !Glowpoint_override ) {
		for (i = 0; i < pm->n_glow_point_banks; i++ ) {
			glow_point_bank *bank = &pm->glow_point_banks[i];
			int j;

			if (bank->is_on) {
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
						vec3d pnt = gpt->pnt;
						vec3d norm = gpt->norm;
					
						if (bank->submodel_parent > 0) { //this is where it rotates for the submodel parent-Bobboau
							if (pm->submodel[bank->submodel_parent].blown_off)
								continue;

							angles angs = pm->submodel[bank->submodel_parent].angs;
							angs.b = PI2 - angs.b;
							angs.p = PI2 - angs.p;
							angs.h = PI2 - angs.h;

							// Compute final submodel orientation by using the orientation
							// matrix and the rotation angles.
							// By using this kind of computation, the rotational angles can
							// always be computed relative to the submodel itself, instead
							// of relative to the parent - KeldorKatarn
							matrix rotation_matrix = pm->submodel[bank->submodel_parent].orientation;
							vm_rotate_matrix_by_angles(&rotation_matrix, &angs);

							matrix inv_orientation;
							vm_copy_transpose_matrix(&inv_orientation, &pm->submodel[bank->submodel_parent].orientation);

							matrix submodel_matrix;
							vm_matrix_x_matrix(&submodel_matrix, &rotation_matrix, &inv_orientation);

							vec3d offset = pm->submodel[bank->submodel_parent].offset;
							vm_vec_sub(&pnt, &pnt, &offset);
							vec3d p = pnt;
							vec3d n = norm;
							vm_vec_rotate(&pnt, &p, &submodel_matrix);
							vm_vec_rotate(&norm, &n, &submodel_matrix);
							vm_vec_add2(&pnt, &offset);
						}

						vec3d tempv;

						switch (bank->type)
						{
							case 0:
							{
								float d;

								if ( IS_VEC_NULL(&norm) ) {
									d = 1.0f;	//if given a nul vector then always show it
								} else {
									vm_vec_sub(&tempv,&View_position,&pnt);
									vm_vec_normalize(&tempv);

									d = vm_vec_dot(&tempv,&norm);
									d -= 0.25;	
								}
						
								if (d > 0.0f) {
									vertex p;

									d *= 3.0f;

									if (d > 1.0f)
										d = 1.0f;

									float w = gpt->radius;

									// fade them in the nebula as well
									if (The_mission.flags & MISSION_FLAG_FULLNEB) {
										vec3d npnt;
										vm_vec_add(&npnt, &pnt, pos);

										d *= (1.0f - neb2_get_fog_intensity(&npnt));
										w *= 1.5;	//make it bigger in a nebula
									}
					
									// disable fogging
									if (Interp_tmap_flags & TMAP_FLAG_PIXEL_FOG)
										gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
		
									if (!Cmdline_nohtl) {
										g3_transfer_vertex(&p, &pnt);
									} else {
										g3_rotate_vertex(&p, &pnt);
									}

									gr_set_bitmap( bank->glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, d );
								//	mprintf(( "rendering glow with texture %d\n", bank->glow_bitmap ));
									g3_draw_bitmap(&p, 0, (w * 0.5f), TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT, w);
									//g3_draw_rotated_bitmap(&p,0.0f,w,w, TMAP_FLAG_TEXTURED );
								}//d>0

								break;
							}

							case 1:
							{
								vertex h1[4];				// halves of a beam section	
								vertex *verts[4] = { &h1[0], &h1[1], &h1[2], &h1[3] };
								vec3d fvec, top1, bottom1, top2, bottom2, start, end;

								vm_vec_add2(&norm, &pnt);

								vm_vec_rotate(&start, &pnt, orient);
								vm_vec_rotate(&end, &norm, orient);
								vm_vec_sub(&fvec, &end, &start);

								vm_vec_normalize(&fvec);

								moldel_calc_facing_pts(&top1, &bottom1, &fvec, &pnt, gpt->radius, 1.0f, &View_position);
								moldel_calc_facing_pts(&top2, &bottom2, &fvec, &norm, gpt->radius, 1.0f, &View_position);

								int idx = 0;
 
/*R_VERTICES()*/				if (Cmdline_nohtl) {
									g3_rotate_vertex(verts[0], &bottom1);
									g3_rotate_vertex(verts[1], &bottom2);
									g3_rotate_vertex(verts[2], &top2);
									g3_rotate_vertex(verts[3], &top1);
								} else {
									g3_transfer_vertex(verts[0], &bottom1);
									g3_transfer_vertex(verts[1], &bottom2);
									g3_transfer_vertex(verts[2], &top2);
									g3_transfer_vertex(verts[3], &top1);
								}

/*P_VERTICES()*/				for (idx = 0; idx < 4; idx++) {
									g3_project_vertex(verts[idx]);
								}

								verts[0]->u = 0.0f; verts[0]->v = 0.0f;
								verts[1]->u = 1.0f; verts[1]->v = 0.0f;
								verts[2]->u = 1.0f;	verts[2]->v = 1.0f;
								verts[3]->u = 0.0f; verts[3]->v = 1.0f;

//								mR_VERTICES();																// rotate and project the vertices
//								mP_VERTICES();						
//								mSTUFF_VERTICES();		// stuff the beam with creamy goodness (texture coords)

								vm_vec_sub(&tempv,&View_position,&pnt);
								vm_vec_normalize(&tempv);

								cull = gr_set_cull(0);

								if (The_mission.flags & MISSION_FLAG_FULLNEB) {
									gr_set_bitmap(bank->glow_neb_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		
								} else {
									gr_set_bitmap(bank->glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		
								}

								g3_draw_poly( 4, verts, TMAP_FLAG_TILED | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT); // added TMAP_FLAG_TILED flag for beam texture tileing -Bobboau

								gr_set_cull(cull);

								break;
							}
						} // switch(bank->type)
					} // flick
				} // for slot
			} // bank is on
		} // for bank
	} // end-glow-points
				
//	gr_printf((200), (20), "x %0.2f, y %0.2f, z %0.2f", Eye_position.xyz.x, Eye_position.xyz.y, Eye_position.xyz.z);

	// Draw the thruster glow
	if ( !is_outlines_only && !is_outlines_only_htl )
		model_render_thrusters( pm, objnum, shipp, orient, pos );


	cull = gr_set_cull(0);

	// Draw the thruster subobjects
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

	i = pm->submodel[pm->detail[Interp_detail_level]].first_child;
	while( i >= 0 )	{
		if (pm->submodel[i].is_thruster )	{
			zbuf_mode = GR_ZBUFF_READ;

			// no zbuffering
			if(Interp_flags & MR_NO_ZBUFFER){
				zbuf_mode = GR_ZBUFF_NONE;
			}

			gr_zbuffer_set(zbuf_mode);

			if (is_outlines_only_htl || (!Cmdline_nohtl && !is_outlines_only)) {
				model_render_children_buffers(&pm->submodel[i], pm, i, Interp_detail_level);
			} else {
				model_interp_subcall( pm, i, Interp_detail_level );
			}
		}
		i = pm->submodel[i].next_sibling;
	}	

	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	gr_set_cull(cull);	

	if ( Interp_flags & MR_SHOW_PATHS ){
		if (Cmdline_nohtl) model_draw_paths(model_num);
		else model_draw_paths_htl(model_num);
	}

	if (Interp_flags & MR_BAY_PATHS ){
		if (Cmdline_nohtl) model_draw_bay_paths(model_num);
		else model_draw_bay_paths_htl(model_num);
	}

	if ( (Interp_flags & MR_AUTOCENTER) && (set_autocen) ) {
		g3_done_instance(use_api);
	}

//	if(Interp_tmap_flags & TMAP_FLAG_PIXEL_FOG)gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);

	g3_done_instance(use_api);

	gr_zbuffer_set(save_gr_zbuffering_mode);
	
	memset(&Interp_offset,0,sizeof(vec3d));

}


void submodel_render(int model_num, int submodel_num, matrix *orient, vec3d * pos, uint flags, int objnum, int *replacement_textures)
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

	bool is_outlines_only_htl = !Cmdline_nohtl && (flags & MR_NO_POLYS) && (flags & MR_SHOW_OUTLINE_HTL);

	//set to true since D3d and OGL need the api matrices set
	g3_start_instance_matrix(pos,orient, true);

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

		light_filter_push( -1, pos, pm->submodel[submodel_num].rad );

		light_rotate_all();

		if (!Cmdline_nohtl) {
			light_set_all_relevent();
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

			neb2_get_fog_values(&fog_near, &fog_far, obj);
			unsigned char r, g, b;
			neb2_get_fog_color(&r, &g, &b);
			gr_fog_set(GR_FOGMODE_FOG, r, g, b, fog_near, fog_far);
		}

		model_render_buffers(&pm->submodel[submodel_num], pm);
			//	if(!Cmdline_nohtl)gr_set_lighting(false,false);
	} else {
		model_interp_sub( pm->submodel[submodel_num].bsp_data, pm, &pm->submodel[submodel_num], 0 );
	}

	gr_set_cull(cull);

	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	if ( pm->submodel[submodel_num].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[submodel_num]);
	}

	if (Interp_flags & MR_SHOW_PIVOTS )
		model_draw_debug_points( pm, &pm->submodel[submodel_num] );

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_filter_pop();	
	}

	g3_done_instance(true);


	// turn off fog after each model renders, RT This fixes HUD being fogged when debris is in target box
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

//	if(!Cmdline_nohtl)gr_set_lighting(false,false);

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
		case OP_EOF: return 1;
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

// Gets two random points on a model
void submodel_get_two_random_points(int model_num, int submodel_num, vec3d *v1, vec3d *v2, vec3d *n1, vec3d *n2 )
{
	int nv = submodel_get_points_internal(model_num, submodel_num);

	Assert(nv > 0);	// Goober5000 - to avoid div-0 error
	int vn1 = (myrand()>>5) % nv;
	int vn2 = (myrand()>>5) % nv;

	*v1 = *Interp_verts[vn1];
	*v2 = *Interp_verts[vn2];

	if(n1 != NULL){
		*n1 = *Interp_norms[vn1];
	}
	if(n2 != NULL){
		*n2 = *Interp_norms[vn2];
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


// Returns number of verts in a submodel;
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
		case OP_EOF: return 0;
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

// Returns number of tmaps & flat polys in a submodel;
int submodel_get_num_polys_sub( ubyte *p )
{
	int chunk_type = w(p);
	int chunk_size = w(p+4);
	int n = 0;
	
	while (chunk_type != OP_EOF)	{
		switch (chunk_type) {
		case OP_EOF:			return n;
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

// Returns number of tmaps & flat polys in a submodel;
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

// set the insignia bitmap to be used when rendering a ship with an insignia (-1 switches it off altogether)
void model_set_insignia_bitmap(int bmap)
{
	Interp_insignia_bitmap = bmap;
}

void model_set_replacement_textures(int *replacement_textures)
{
	Interp_new_replacement_textures = replacement_textures;	//replacement_textures;
}

// set the forces bitmap
void model_set_forced_texture(int bmap)
{
	Interp_forced_bitmap = bmap;
}

// set model transparency for use with MR_ALL_XPARENT
void model_set_alpha(float alpha)
{
	Interp_xparent_alpha = alpha;
}

// see if the given texture is used by the passed model. 0 if not used, 1 if used, -1 on error
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

// given a newly loaded model, page in all textures
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
//poly_list flat_list;
//line_list flat_line_list;

//#define parseF(dest, f, off)	{memcpy(&dest, &f[off], sizeof(float)); off += sizeof(float);}
//#define parseI(dest, f, off)	{memcpy(&dest, &f[off], sizeof(int)); off += sizeof(int);}
//#define parseV(dest, f, off)	{parseF(dest.x, f, off); parseF(dest.y, f, off); parseF(dest.z, f, off); }
//#define parseS(dest, f, off)	{memset(dest.str, 0, STRLEN); memcpy(&dest.n_char, &f[off], sizeof(int)); off += sizeof(int); memcpy(&dest.str, &f[off], dest.n_char); off += dest.n_char;}


void parse_defpoint(int off, ubyte *bsp_data)
{
	int i, n;
//	off+=4;
	int nverts = w(off+bsp_data+8);	
	int offset = w(off+bsp_data+16);
	int next_norm = 0;

	ubyte *normcount = off+bsp_data+20;
//	vertex *dest = Interp_points;
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
	if((N->xyz.x * N->xyz.x) < 0 ||
	   (N->xyz.y * N->xyz.y) < 0 ||
	   (N->xyz.z * N->xyz.z) < 0 ||
	   !is_valid_vec(N))
	{
		N->xyz.x = 1;
		N->xyz.y = 0;
		N->xyz.z = 0;
		return 1;
	}

	return 0;
}

int Parse_normal_problem_count = 0;

void parse_tmap(int offset, ubyte *bsp_data)
{
	int pof_tex = w(bsp_data+offset+40);
	int n_vert = w(bsp_data+offset+36);
	//int n_tri = n_vert - 2;
//	ubyte *temp_verts;
	ubyte *p = &bsp_data[offset+8];
	model_tmap_vert *tverts;

	tverts = (model_tmap_vert *)&bsp_data[offset+44];
//	temp_verts = &bsp_data[offset+44];

	vertex *V;
	vec3d *v;
	vec3d *N;

	int problem_count = 0;

	for (int i = 1; i < (n_vert-1); i++) {
		V = &polygon_list[pof_tex].vert[(polygon_list[pof_tex].n_verts)];
		N = &polygon_list[pof_tex].norm[(polygon_list[pof_tex].n_verts)];
		v = Interp_verts[(int)tverts[0].vertnum];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = tverts[0].u;
		V->v = tverts[0].v;

		*N = *Interp_norms[(int)tverts[0].normnum];

		if ( IS_VEC_NULL(N) )
			*N = *vp(p);

	  	problem_count += check_values(N);
		// VECMAT-ERROR: NULL VEC3D (N.x = 0, N.y = 0, N.z = -0)
		vm_vec_normalize_safe(N);
//		vm_vec_scale(N, global_scaleing_factor);//global scaleing

		V = &polygon_list[pof_tex].vert[(polygon_list[pof_tex].n_verts)+1];
		N = &polygon_list[pof_tex].norm[(polygon_list[pof_tex].n_verts)+1];
		v = Interp_verts[(int)tverts[i].vertnum];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = tverts[i].u;
		V->v = tverts[i].v;

		*N = *Interp_norms[(int)tverts[i].normnum];

		if ( IS_VEC_NULL(N) )
			*N = *vp(p);

	 	problem_count += check_values(N);
		// VECMAT-ERROR: NULL VEC3D (N.x = 0, N.y = 0, N.z = -0)
		vm_vec_normalize_safe(N);
//		vm_vec_scale(N, global_scaleing_factor);//global scaleing

		V = &polygon_list[pof_tex].vert[(polygon_list[pof_tex].n_verts)+2];
		N = &polygon_list[pof_tex].norm[(polygon_list[pof_tex].n_verts)+2];
		v = Interp_verts[(int)tverts[i+1].vertnum];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = tverts[i+1].u;
		V->v = tverts[i+1].v;

		*N = *Interp_norms[(int)tverts[i+1].normnum];

		if ( IS_VEC_NULL(N) )
			*N = *vp(p);

		problem_count += check_values(N);
		// VECMAT-ERROR: NULL VEC3D (N.x = 0, N.y = 0, N.z = -0)
		vm_vec_normalize_safe(N);
//		vm_vec_scale(N, global_scaleing_factor);//global scaleing

		polygon_list[pof_tex].n_verts += 3;
		polygon_list[pof_tex].n_prim++;
	}

	Parse_normal_problem_count += problem_count;
}

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
void parse_flat_poly(int offset, ubyte *bsp_data)
{
/* Goober5000 - if this function was commented out, the variables should be also
	int nv = w(bsp_data+offset+36);
	short *verts = (short *)(&bsp_data[offset+44]);

	vertex *V;
	vec3d *v;
	vec3d *N;
	int i = 0;

	for( i = 1; i<nv-1; i++){
		V = &flat_list.vert[flat_list.n_poly][0];
		N = &flat_list.norm[flat_list.n_poly][0];
		v = Interp_verts[verts[i*2]];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = 0.0f;
		V->v = 0.0f;
		*N = *(vec3d*)&bsp_data[offset+8];
		V->r = bsp_data[offset+40];
		V->g = bsp_data[offset+41];
		V->b = bsp_data[offset+42];
		vm_vec_normalize(N);

		V = &flat_list.vert[flat_list.n_poly][1];
		N = &flat_list.norm[flat_list.n_poly][1];
		v = Interp_verts[verts[i*2]];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = 0.0f;
		V->v = 0.0f;
		*N = *(vec3d*)&bsp_data[offset+8];
		V->r = bsp_data[offset+40];
		V->g = bsp_data[offset+41];
		V->b = bsp_data[offset+42];
		vm_vec_normalize(N);

		V = &flat_list.vert[flat_list.n_poly][2];
		N = &flat_list.norm[flat_list.n_poly][2];
		v = Interp_verts[verts[i*2]];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = 0.0f;
		V->v = 0.0f;
		*N = *(vec3d*)&bsp_data[offset+8];
		V->r = bsp_data[offset+40];
		V->g = bsp_data[offset+41];
		V->b = bsp_data[offset+42];
		vm_vec_normalize(N);

		flat_list.n_poly++;
	}

	//we don't need this
	for(i = 0; i<nv; i++){
		V = &flat_line_list.vert[flat_line_list.n_line][0];
		v = Interp_verts[verts[(i%nv*2)]];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = 0.0f;
		V->v = 0.0f;
		V->r = bsp_data[offset+40];
		V->g = bsp_data[offset+41];
		V->b = bsp_data[offset+42];

		V = &flat_line_list.vert[flat_line_list.n_line][1];
		v = Interp_verts[verts[(((i+1)%nv)*2)]];
		V->x = v->xyz.x;
		V->y = v->xyz.y;
		V->z = v->xyz.z;
		V->u = 0.0f;
		V->v = 0.0f;
		V->r = bsp_data[offset+40];
		V->g = bsp_data[offset+41];
		V->b = bsp_data[offset+42];

		flat_line_list.n_line++;
	}*/
}
//flat_list

void parse_sortnorm(int offset, ubyte *bsp_data);

void parse_bsp(int offset, ubyte *bsp_data)
{
	int id = w(bsp_data+offset);
	int size = w(bsp_data+offset+4);

	while (id != 0) {
		switch (id)
		{
			case OP_EOF:	
				return;

			case OP_DEFPOINTS:
				parse_defpoint(offset, bsp_data);
				break;

			case OP_SORTNORM:
				parse_sortnorm(offset, bsp_data);
				break;

			case OP_FLATPOLY:
			//	parse_flat_poly(offset, bsp_data);
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
//	off+=4;
	int nverts = w(off+bsp_data+8);	
//	int offset = w(off+bsp_data+16);

	ubyte * normcount = off+bsp_data+20;
//	vec3d *src = vp(off+bsp_data+offset);

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
			case OP_EOF:	
				return;

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


static void allocate_poly_list()
{
	for (int i = 0; i < MAX_MODEL_TEXTURES; i++) {
		polygon_list[i].allocate(tri_count[i]*3);
	}

	model_allocate_interp_data(Interp_num_verts, Interp_num_norms);
}

int recode_check = 0;
//void recode_bsp(int offset, ubyte *bsp_data);
//void model_resort_index_buffer(ubyte *bsp_data, bool f2b, int texture, short* index_buffer);
poly_list model_list;

void generate_vertex_buffers(bsp_info *model, polymodel *pm)
{
	int i, j, first_index;
	uint total_verts = 0;
	bool use_large_buffer = false;

	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		polygon_list[i].n_prim = 0;
		polygon_list[i].n_verts = 0;
		tri_count[i] = 0;
	}

//	flat_list.n_poly = 0;
//	flat_line_list.n_line = 0;

	find_tri_counts(0, model->bsp_data);


	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		total_verts += tri_count[i];
	}

	if (total_verts < 1) {
		model->indexed_vertex_buffer = -1;
		return;
	}

	allocate_poly_list();

	parse_bsp(0, model->bsp_data);

	total_verts = 0;

	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		total_verts += polygon_list[i].n_verts;
	}

	// for the moment we can only support INT_MAX worth of verts
	if (total_verts > INT_MAX) {
        Error( LOCATION, "Unable to generate vertex buffer data because model '%s' with %i verts is over the maximum of %i verts!\n", pm->filename, model_list.n_verts, INT_MAX);
    }

	model_list.allocate( (int)total_verts );
	model_list.n_verts = 0;
	model_list.n_prim = 0;

	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		if ( !polygon_list[i].n_verts )
			continue;

		memcpy( (model_list.vert) + model_list.n_verts, polygon_list[i].vert, sizeof(vertex) * polygon_list[i].n_verts );
		memcpy( (model_list.norm) + model_list.n_verts, polygon_list[i].norm, sizeof(vec3d) * polygon_list[i].n_verts );

		if (Cmdline_normal) {
			memcpy( (model_list.tsb) + model_list.n_verts, polygon_list[i].tsb, sizeof(tsb_t) * polygon_list[i].n_verts );
		}

		model_list.n_verts += polygon_list[i].n_verts;
		model_list.n_prim += polygon_list[i].n_prim;
	}

	// check if we need to use a larger index buffer or not
	use_large_buffer = (model_list.n_verts > USHRT_MAX);

	// IBX stuff
	extern IBX ibuffer_info;

	// if we have an IBX read file then use it, otherwise generate buffers and save them to file
	if (ibuffer_info.read != NULL) {
		bool ibx_is_short = false;
		bool tsb_is_short = false;
		int ibx_verts = 0;
		int ibx_size = 0;
		int tsb_size = 0;

		ibx_verts = cfread_int( ibuffer_info.read );
		ibuffer_info.size -= sizeof(int); // subtract

		// figure up how big this section of data is going to be
		ibx_size += ibx_verts * sizeof(float) * 8; // first set of data (here)

		// second set of data (next "for" statement)
		if (ibx_verts > USHRT_MAX) {
			ibx_size += ibx_verts * sizeof(uint);
		} else {
			ibx_size += ibx_verts * sizeof(ushort);
		}

		if (ibx_size > ibuffer_info.size) {
			ibx_is_short = true;
		}

		if (ibuffer_info.tsb_read != NULL) {
			tsb_size += ibx_verts * sizeof(float) * 4;

			if (tsb_size > ibuffer_info.tsb_size) {
				tsb_is_short = true;
			}
		}


		// safety check for this section
		// ibuffer_info.size should be greater than or equal to ibx_size at this point
		if ( ibx_is_short || tsb_is_short ) {
			// AAAAAHH! not enough stored data - Abort, Retry, Fail?
			Warning(LOCATION, "IBX: Safety Check Failure!  The file doesn't contain enough data, deleting '%s'\n", ibuffer_info.name);

			cfclose( ibuffer_info.read );
			ibuffer_info.read = NULL;
			ibuffer_info.size = 0;
			cf_delete( ibuffer_info.name, CF_TYPE_CACHE );

			if (ibuffer_info.tsb_read != NULL) {
				cfclose( ibuffer_info.tsb_read );
				ibuffer_info.tsb_read = NULL;
				ibuffer_info.tsb_size = 0;
				cf_delete( ibuffer_info.tsb_name, CF_TYPE_CACHE );
			}

			// force generate
			model_list.make_index_buffer();
		} else {
			// it's all good
			for (i = 0; i < ibx_verts; i++) {
				model_list.vert[i].x = cfread_float( ibuffer_info.read );
				model_list.vert[i].y = cfread_float( ibuffer_info.read );
				model_list.vert[i].z = cfread_float( ibuffer_info.read );
				model_list.vert[i].u = cfread_float( ibuffer_info.read );
				model_list.vert[i].v = cfread_float( ibuffer_info.read );
				model_list.norm[i].xyz.x = cfread_float( ibuffer_info.read );
				model_list.norm[i].xyz.y = cfread_float( ibuffer_info.read );
				model_list.norm[i].xyz.z = cfread_float( ibuffer_info.read );
			}

			model_list.n_verts = ibx_verts;

			if (ibuffer_info.tsb_read != NULL) {
				for (i = 0; i < ibx_verts; i++) {
					model_list.tsb[i].tangent.xyz.x = cfread_float( ibuffer_info.tsb_read );
					model_list.tsb[i].tangent.xyz.y = cfread_float( ibuffer_info.tsb_read );
					model_list.tsb[i].tangent.xyz.z = cfread_float( ibuffer_info.tsb_read );
					model_list.tsb[i].scaler = cfread_float( ibuffer_info.tsb_read );
				}
			}

		}

		// subtract this block of data from the total size for next check
		// remember that this includes the next set of reads too
		ibuffer_info.size -= ibx_size;
	} else {
		if ( use_large_buffer && (ibuffer_info.write != NULL) ) {
			mprintf(("IBX NOTICE:  Vert limit hit for IBX version 1 on subobject '%s', changing file format to version 2...\n", model->name));
			ibuffer_info.version = 2;
		}

		// no read file so we'll have to generate
		model_list.make_index_buffer();

		if (ibuffer_info.write != NULL) {
			cfwrite_int( model_list.n_verts, ibuffer_info.write );

			for (i = 0; i < model_list.n_verts; i++) {
				cfwrite_float( model_list.vert[i].x, ibuffer_info.write );
				cfwrite_float( model_list.vert[i].y, ibuffer_info.write );
				cfwrite_float( model_list.vert[i].z, ibuffer_info.write );
				cfwrite_float( model_list.vert[i].u, ibuffer_info.write );
				cfwrite_float( model_list.vert[i].v, ibuffer_info.write );
				cfwrite_float( model_list.norm[i].xyz.x, ibuffer_info.write );
				cfwrite_float( model_list.norm[i].xyz.y, ibuffer_info.write );
				cfwrite_float( model_list.norm[i].xyz.z, ibuffer_info.write );
			}

			if (ibuffer_info.tsb_write != NULL) {
				for (i = 0; i < model_list.n_verts; i++) {
					cfwrite_float( model_list.tsb[i].tangent.xyz.x, ibuffer_info.tsb_write );
					cfwrite_float( model_list.tsb[i].tangent.xyz.y, ibuffer_info.tsb_write );
					cfwrite_float( model_list.tsb[i].tangent.xyz.z, ibuffer_info.tsb_write );
					cfwrite_float( model_list.tsb[i].scaler, ibuffer_info.tsb_write );
				}
			}

		}
	}

	int vertex_flags = (VERTEX_FLAG_POSITION | VERTEX_FLAG_NORMAL | VERTEX_FLAG_UV1);

	if (model_list.tsb != NULL) {
		Assert( Cmdline_normal );
		vertex_flags |= VERTEX_FLAG_TANGENT;
	}

	model->indexed_vertex_buffer = gr_make_buffer(&model_list, vertex_flags);

	if (model->indexed_vertex_buffer == -1)
		Error(LOCATION, "Could not generate model->indexed_vertex_buffer");

	recode_check = 0;
//	recode_bsp(0, model->bsp_data);

	for (i = 0; i < MAX_MODEL_TEXTURES; i++) {
		if ( !polygon_list[i].n_verts )
			continue;

		buffer_data new_buffer;

		new_buffer.index_buffer.allocate( polygon_list[i].n_verts, use_large_buffer );

		for (j = 0; j < polygon_list[i].n_verts; j++) {
			if ( ibuffer_info.read != NULL ) {
				if (use_large_buffer) {
					Assert( new_buffer.index_buffer.ibuffer != NULL );
					new_buffer.index_buffer.ibuffer[j] = cfread_uint( ibuffer_info.read );
				} else {
					Assert( new_buffer.index_buffer.sbuffer != NULL );
					new_buffer.index_buffer.sbuffer[j] = cfread_ushort( ibuffer_info.read );
				}
			} else {
				first_index = find_first_index_vb( &polygon_list[i], j, &model_list );
				Assert(first_index != -1);

				if (use_large_buffer) {
					Assert( new_buffer.index_buffer.ibuffer != NULL );
					new_buffer.index_buffer.ibuffer[j] = (uint)first_index;

					Assert( same_vert(&model_list.vert[new_buffer.index_buffer.ibuffer[j]], &polygon_list[i].vert[j], &model_list.norm[new_buffer.index_buffer.ibuffer[j]], &polygon_list[i].norm[j]) );
				} else {
					Assert( new_buffer.index_buffer.sbuffer != NULL );
					new_buffer.index_buffer.sbuffer[j] = (ushort)first_index;

					Assert( same_vert(&model_list.vert[new_buffer.index_buffer.sbuffer[j]], &polygon_list[i].vert[j], &model_list.norm[new_buffer.index_buffer.sbuffer[j]], &polygon_list[i].norm[j]) );
				}

			//	Assert(find_first_index_vb(&model_list, j, &list[i]) == j);//there should never ever be any redundant verts

				// try to write out generated index buffer for later use
				if ( ibuffer_info.write != NULL ) {
					if (use_large_buffer)
						cfwrite_uint( new_buffer.index_buffer.ibuffer[j], ibuffer_info.write );
					else
						cfwrite_ushort( new_buffer.index_buffer.sbuffer[j], ibuffer_info.write );
				}
			}
		}

		new_buffer.n_prim = tri_count[i];
		new_buffer.texture = i;

		model->buffer.push_back( new_buffer );
	}

}


inline int in_box(vec3d *min, vec3d *max, vec3d *pos)
{
	vec3d point;

	vm_vec_sub(&point, &View_position, pos);

	if ( (point.xyz.x >= min->xyz.x) && (point.xyz.x <= max->xyz.x)
		&& (point.xyz.y >= min->xyz.y) && (point.xyz.y <= max->xyz.y)
		&& (point.xyz.z >= min->xyz.z) && (point.xyz.z <= max->xyz.z) )
	{
		return 1;
	}

	return -1;
}


void model_render_children_buffers(bsp_info *model, polymodel *pm, int mn, int detail_level)
{
	int i;
	int zbuf_mode = gr_zbuffering_mode;

	if ( (mn < 0) || (mn>=pm->n_models) )
		return;

	Assert( mn >= 0 );
	Assert( mn < pm->n_models );

	if (pm->submodel[mn].blown_off)
		return;


	uint fl = Interp_flags;

	if (pm->submodel[mn].is_thruster) {
		if ( !(Interp_flags & MR_SHOW_THRUSTERS) )
			return;

		Interp_flags |= MR_NO_LIGHTING;
		Interp_thrust_scale_subobj = 1;
	} else {
		Interp_thrust_scale_subobj = 0;
	}

	vm_vec_add2(&Interp_offset, &pm->submodel[mn].offset);

	// if using detail boxes, check that we are valid for the range
	if ( !(Interp_flags & MR_FULL_DETAIL) && model->use_render_box ) {
		vm_vec_copy_scale(&Interp_render_box_min, &model->render_box_min, Interp_box_scale);
		vm_vec_copy_scale(&Interp_render_box_max, &model->render_box_max, Interp_box_scale);

		if ( (-model->use_render_box + in_box(&Interp_render_box_min, &Interp_render_box_max, &model->offset)) )
			return;
	}

	// Get submodel rotation data and use submodel orientation matrix
	// to put together a matrix describing the final orientation of
	// the submodel relative to its parent
	angles ang = pm->submodel[mn].angs;

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
	vm_copy_transpose_matrix(&inv_orientation, &model->orientation);

	matrix submodel_matrix;
	vm_matrix_x_matrix(&submodel_matrix, &rotation_matrix, &inv_orientation);

	g3_start_instance_matrix(&pm->submodel[mn].offset, &submodel_matrix, true);

	if ( !(Interp_flags & MR_NO_LIGHTING) )
		light_rotate_all();

	model_render_buffers( model, pm, true );

	if (Interp_flags & MR_SHOW_PIVOTS )
		model_draw_debug_points( pm, &pm->submodel[mn] );

	if ( pm->submodel[mn].num_arcs )
		interp_render_lightning( pm, &pm->submodel[mn]);


	i = pm->submodel[mn].first_child;

	while (i >= 0) {
		if ( !pm->submodel[i].is_thruster ) {
			if (Interp_flags & MR_NO_ZBUFFER) {
				zbuf_mode = GR_ZBUFF_NONE;
			} else {
				zbuf_mode = GR_ZBUFF_FULL;		// read only
			}

			gr_zbuffer_set(zbuf_mode);

			model_render_children_buffers( &pm->submodel[i], pm, i, detail_level );
		}

		i = pm->submodel[i].next_sibling;
	}

	vm_vec_sub2(&Interp_offset,&pm->submodel[mn].offset);

	Interp_flags = fl;
	g3_done_instance(true);
}

void model_render_buffers(bsp_info *model, polymodel *pm, bool is_child)
{
	int cull = 0;
	if (model->indexed_vertex_buffer == -1)
		return;

	// if using detail boxes, check that we are valid for the range
	if ( !(Interp_flags & MR_FULL_DETAIL) && !is_child && model->use_render_box ) {
		vm_vec_copy_scale(&Interp_render_box_min, &model->render_box_min, Interp_box_scale);
		vm_vec_copy_scale(&Interp_render_box_max, &model->render_box_max, Interp_box_scale);

		if ( (-model->use_render_box + in_box(&Interp_render_box_min, &Interp_render_box_max, &model->offset)) )
			return;
	}

	// Goober5000
	fix base_frametime = 0;

	if (Interp_objnum >= 0) {
		object *objp = &Objects[Interp_objnum];

		if (objp->type == OBJ_SHIP)
			base_frametime = Ships[objp->instance].base_texture_anim_frametime;
	}

	// RT Added second conditional parameter, seems to not distrupt anything in either API
	gr_set_lighting( !(Interp_flags & MR_NO_LIGHTING), !(Interp_flags & MR_NO_LIGHTING) );

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

	cull = gr_set_cull( (Interp_flags & MR_NO_CULL) ? 0 : 1 );

	int no_texturing = (Interp_flags & MR_NO_TEXTURING);
	int zbuffer_save = gr_zbuffering_mode;

	gr_push_scale_matrix(&scale);

	gr_set_buffer(model->indexed_vertex_buffer);

	uint buffer_size = model->buffer.size();

	for (uint i = 0; i < buffer_size; i++) {
		int texture = -1;
		int tmap_num = model->buffer[i].texture;
		texture_map *tmap = &pm->maps[tmap_num];
		texture_info *tbase = &tmap->textures[TM_BASE_TYPE];
		texture_info *tglow = &tmap->textures[TM_GLOW_TYPE];
		int rt_begin_index = tmap_num*TM_NUM_TYPES;

		if ( (Interp_flags & MR_FORCE_TEXTURE) && (Interp_forced_bitmap >= 0) ) {
			texture = Interp_forced_bitmap;
		}
		else if (Interp_warp_bitmap > -1) {
			texture = Interp_warp_bitmap;
		}
		else if ( (Interp_new_replacement_textures != NULL) && (Interp_new_replacement_textures[rt_begin_index + TM_BASE_TYPE] >= 0) ) {
			texture = Interp_new_replacement_textures[rt_begin_index + TM_BASE_TYPE];
		}
		else if ( Interp_thrust_scale_subobj && !no_texturing ) {
			texture = Interp_thrust_bitmap;
		}
		else if (!no_texturing) {
			// pick the texture, animating it if necessary
			texture = model_interp_get_texture(tbase, base_frametime);

			// doing glow maps?
			if ( !(Interp_flags & MR_NO_GLOWMAPS) && (tglow->GetTexture() >= 0) ) {
				// shockwaves are special, their current frame has to come out of the shockwave code to get the timing correct
				if ( (Interp_objnum >= 0) && (Objects[Interp_objnum].type == OBJ_SHOCKWAVE) && (tglow->GetNumFrames() > 1) ) {
					GLOWMAP = tglow->GetTexture() + shockwave_get_framenum(Objects[Interp_objnum].instance, tglow->GetNumFrames());
				} else {
					GLOWMAP = model_interp_get_texture(tglow, base_frametime);
				}
			}

			if ( (Detail.lighting > 2)  && (Interp_detail_level < 2) ) {
				// likewise, etc.
				SPECMAP = model_interp_get_texture(&tmap->textures[TM_SPECULAR_TYPE], base_frametime);
				NORMMAP = model_interp_get_texture(&tmap->textures[TM_NORMAL_TYPE], base_frametime);
				HEIGHTMAP = model_interp_get_texture(&tmap->textures[TM_HEIGHT_TYPE], base_frametime);
			}
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
				tex = model_interp_get_texture(&tinfo, base_frametime);
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
					default:
						break;
				}
			}
		}


		if ( (texture == -1) && !no_texturing )
			continue;


		if (Interp_thrust_scale_subobj) {
			if ( (Interp_thrust_bitmap >= 0) && (Interp_thrust_scale > 0.0f) )
				gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.2f);

			gr_zbuffer_set(GR_ZBUFF_READ);
		}
		else if (Interp_flags & MR_ALL_XPARENT) {
			gr_set_cull(0);
			gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Interp_xparent_alpha );
			gr_zbuffer_set(GR_ZBUFF_NONE);
		}
		else if (Interp_warp_bitmap >= 0) {
			gr_set_cull(0);
			gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Interp_warp_alpha );
			gr_zbuffer_set(GR_ZBUFF_READ);
		}
		// trying to get transperent textures-Bobboau
		else if (tmap->is_transparent) {
			// for special shockwave/warpmap usage
			if (Interp_warp_alpha != -1.0f)
				gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Interp_warp_alpha );
			else
				gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.8f );

			gr_zbuffer_set(GR_ZBUFF_READ);
		}
		else if (!no_texturing) {
		//	gr_set_state(TEXTURE_SOURCE_DECAL, ALPHA_BLEND_NONE, ZBUFFER_TYPE_DEFAULT);
			gr_set_bitmap( texture, GR_ALPHABLEND_NONE, GR_BITBLT_MODE_NORMAL, 1.0f );
	//		gr_zbuffer_set(GR_ZBUFF_FULL);
		}

		if (Interp_flags & MR_EDGE_ALPHA) {
			gr_center_alpha(-1);
		} else if (Interp_flags & MR_CENTER_ALPHA) {
			gr_center_alpha(1);
		} else {
			gr_center_alpha(0);
		}

	//	model_resort_index_buffer(model->bsp_data, 1, tmap_num, model->buffer[i].index_buffer.index_buffer);
		
		gr_render_buffer(0, model->buffer[i].n_prim, model->buffer[i].index_buffer.sbuffer, model->buffer[i].index_buffer.ibuffer, Interp_tmap_flags);

		GLOWMAP = -1;
		SPECMAP = -1;
		NORMMAP = -1;
		HEIGHTMAP = -1;

		// reset culling
		gr_set_cull(cull);

		// reset z-buffer
		gr_zbuffer_set(zbuffer_save);
	}

	gr_pop_scale_matrix();

	gr_set_lighting(false, false);
}

/*
//int recode_check = 0;
void recode_tmap(int offset, ubyte *bsp_data){

//	int pof_tex = w(bsp_data+offset+40);
	int n_vert = w(bsp_data+offset+36);

//	if(pof_tex == 4){
//		1;
//	}

	//int n_tri = n_vert - 2;
	ubyte *temp_verts;
	//ubyte *p = &bsp_data[offset];

	model_tmap_vert *tverts;
	tverts = (model_tmap_vert *)&bsp_data[offset+44];
	temp_verts = &bsp_data[offset+44];

	//int problem_count = 0;

	for(int i = 0; i<n_vert; i++){	
		vertex vert;
		vm_vec2vert(Interp_verts[(int)tverts[i].vertnum], &vert);
		vec3d norm = *Interp_norms[(int)tverts[i].normnum];
		vm_vec_normalize(&norm);

		vert.u = tverts[i].u;
		vert.v = tverts[i].v;

		for(int k = 0; k<model_list.n_verts; k++){
			if(same_vert(&model_list.vert[k], &vert, &model_list.norm[k], &norm)){
				tverts[i].normnum = (short)k;
				recode_check++;
				break;
			}
			if(k == model_list.n_verts -1){
				Warning(LOCATION, "recode error");
			//	tverts[i].normnum = (short)0;
			}
		}
	}

}

void recode_sortnorm(int offset, ubyte *bsp_data);

void recode_bsp(int offset, ubyte *bsp_data){
	int id = w(bsp_data+offset);
	int size = w(bsp_data+offset+4);

	while(id!=0){
		switch(id){
		case OP_EOF:	
			return;
			break;
		case OP_DEFPOINTS: parse_defpoint(offset, bsp_data);
			break;
		case OP_SORTNORM:	recode_sortnorm(offset, bsp_data);
			break;
		case OP_FLATPOLY:
			break;
		case OP_TMAPPOLY:	recode_tmap(offset, bsp_data);
			break;
		case OP_BOUNDBOX:
			break;
		default:
			return;
		}
		offset += size;
		id = w(bsp_data+offset);
		size = w(bsp_data+offset+4);

		if(size < 1) id=OP_EOF;
	}
}

int model_resort_index_buffer_n_verts = 0;
void recode_sortnorm(int offset, ubyte *bsp_data){

	int frontlist, backlist, prelist, postlist, onlist;
	frontlist = w(bsp_data+offset+36);
	backlist = w(bsp_data+offset+40);
	prelist = w(bsp_data+offset+44);
	postlist = w(bsp_data+offset+48);
	onlist = w(bsp_data+offset+52);

	if (prelist) recode_bsp(offset+prelist,bsp_data);
	if (backlist) recode_bsp(offset+backlist, bsp_data);
	if (onlist) recode_bsp(offset+onlist, bsp_data);
	if (frontlist) recode_bsp(offset+frontlist, bsp_data);
	if (postlist) recode_bsp(offset+postlist, bsp_data);
}

void model_resort_index_buffer_tmap(int offset, ubyte *bsp_data, short* index_buffer, int texture){
	if(texture != w(bsp_data+offset+40))return;

	int n_vert = w(bsp_data+offset+36);
	//int n_tri = n_vert - 2;
	ubyte *temp_verts;
	//ubyte *p = &bsp_data[offset];

	model_tmap_vert *tverts;
	tverts = (model_tmap_vert *)&bsp_data[offset+44];
	temp_verts = &bsp_data[offset+44];
	for(int i = 1; i<n_vert-1; i++){	
		index_buffer[model_resort_index_buffer_n_verts++] = (short)tverts[0].normnum;
		index_buffer[model_resort_index_buffer_n_verts++] = (short)tverts[i].normnum;
		index_buffer[model_resort_index_buffer_n_verts++] = (short)tverts[i+1].normnum;
	}

}

void model_resort_index_buffer_bsp(int offset, ubyte *bsp_data, bool f2b, int texture, short* index_buffer);

void model_resort_index_buffer_sortnorm_nonsorted(int offset, ubyte *bsp_info, int texture, short* index_buffer){

	int frontlist, backlist, prelist, postlist, onlist;
	frontlist = w(bsp_info+offset+36);
	backlist = w(bsp_info+offset+40);
	prelist = w(bsp_info+offset+44);
	postlist = w(bsp_info+offset+48);
	onlist = w(bsp_info+offset+52);

	if (prelist) model_resort_index_buffer_bsp(offset+prelist,bsp_info, false, texture, index_buffer);
	if (backlist) model_resort_index_buffer_bsp(offset+backlist, bsp_info, false, texture, index_buffer);
	if (onlist) model_resort_index_buffer_bsp(offset+onlist, bsp_info, false, texture, index_buffer);
	if (frontlist) model_resort_index_buffer_bsp(offset+frontlist, bsp_info, false, texture, index_buffer);
	if (postlist) model_resort_index_buffer_bsp(offset+postlist, bsp_info, false, texture, index_buffer);
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
void model_resort_index_buffer_sortnorm_b2f(int offset, ubyte *bsp_info, int texture, short* index_buffer)
{
	#ifndef NDEBUG
	modelstats_num_sortnorms++;
	#endif

//	Assert( w(p+4) == 56 );

	int frontlist, backlist, prelist, postlist, onlist;
	frontlist = w(bsp_info+offset+36);
	backlist = w(bsp_info+offset+40);
	prelist = w(bsp_info+offset+44);
	postlist = w(bsp_info+offset+48);
	onlist = w(bsp_info+offset+52);

	if (prelist) model_resort_index_buffer_bsp(offset+prelist,bsp_info, false, texture, index_buffer);		// prelist

	if (g3_check_normal_facing(vp(bsp_info+offset+20),vp(bsp_info+offset+8))) {		//facing

		//draw back then front

		if (backlist) model_resort_index_buffer_bsp(offset+backlist,bsp_info, false, texture, index_buffer);

		if (onlist) model_resort_index_buffer_bsp(offset+onlist,bsp_info, false, texture, index_buffer);			//onlist

		if (frontlist) model_resort_index_buffer_bsp(offset+frontlist,bsp_info, false, texture, index_buffer);

	}	else {			//not facing.  draw front then back

		if (frontlist) model_resort_index_buffer_bsp(offset+frontlist,bsp_info, false, texture, index_buffer);

		if (onlist) model_resort_index_buffer_bsp(offset+onlist,bsp_info, false, texture, index_buffer);			//onlist

		if (backlist) model_resort_index_buffer_bsp(offset+backlist,bsp_info, false, texture, index_buffer);
	}

	if (postlist) model_resort_index_buffer_bsp(offset+postlist,bsp_info, false, texture, index_buffer);		// postlist

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
void model_resort_index_buffer_sortnorm_f2b(int offset, ubyte *bsp_info, int texture, short* index_buffer)
{
	#ifndef NDEBUG
	modelstats_num_sortnorms++;
	#endif

//	Assert( w(p+4) == 56 );

	int frontlist, backlist, prelist, postlist, onlist;
	frontlist = w(bsp_info+offset+36);
	backlist = w(bsp_info+offset+40);
	prelist = w(bsp_info+offset+44);
	postlist = w(bsp_info+offset+48);
	onlist = w(bsp_info+offset+52);

	if (postlist) model_resort_index_buffer_bsp(offset+postlist,bsp_info, true, texture, index_buffer);		// postlist

	if (g3_check_normal_facing(vp(bsp_info+offset+20),vp(bsp_info+offset+8))) {		//facing

		//draw front then back

		if (frontlist) model_resort_index_buffer_bsp(offset+frontlist,bsp_info, true, texture, index_buffer);

		if (onlist) model_resort_index_buffer_bsp(offset+onlist,bsp_info, true, texture, index_buffer);			//onlist

		if (backlist) model_resort_index_buffer_bsp(offset+backlist,bsp_info, true, texture, index_buffer);

	} else {

		//draw back then front

		if (backlist) model_resort_index_buffer_bsp(offset+backlist,bsp_info, true, texture, index_buffer);

		if (onlist) model_resort_index_buffer_bsp(offset+onlist,bsp_info, true, texture, index_buffer);			//onlist

		if (frontlist) model_resort_index_buffer_bsp(offset+frontlist,bsp_info, true, texture, index_buffer);

	}

	if (prelist) model_resort_index_buffer_bsp(offset+prelist,bsp_info, true, texture, index_buffer);		// prelist
}


void model_resort_index_buffer_bsp(int offset, ubyte *bsp_data, bool f2b, int texture, short* index_buffer){
	int id = w(bsp_data+offset);
	int size = w(bsp_data+offset+4);

	while(id!=0){
		switch(id){
		case OP_EOF:	
			return;
			break;
		case OP_DEFPOINTS:
			break;
		case OP_SORTNORM:	if(f2b)model_resort_index_buffer_sortnorm_f2b(offset, bsp_data, texture, index_buffer); else model_resort_index_buffer_sortnorm_b2f(offset, bsp_data, texture, index_buffer);
//			model_resort_index_buffer_sortnorm_nonsorted(offset, bsp_data, texture, index_buffer);
			break;
		case OP_FLATPOLY:
			break;
		case OP_TMAPPOLY:	model_resort_index_buffer_tmap(offset, bsp_data, index_buffer, texture);
			break;
		case OP_BOUNDBOX:
			break;
		default:
			return;
		}
		offset += size;
		id = w(bsp_data+offset);
		size = w(bsp_data+offset+4);

		if(size < 1) id=OP_EOF;
	}
}


void model_resort_index_buffer(ubyte *bsp_data, bool f2b, int texture, short* index_buffer){
	model_resort_index_buffer_n_verts = 0;
	model_resort_index_buffer_bsp(0, bsp_data, f2b, texture, index_buffer);
}
*/

/*
struct silhouette_index{
	short* silhouette_index;
	int silhouette_n_allocated;
	int n_in_list;
	void allocate(int size){
		if(size <= silhouette_n_allocated)return;
		short* new_buffer = (short*)vm_malloc(sizeof(short)* size);
		if(silhouette_n_allocated){memcpy(new_buffer, silhouette_index, sizeof(short)*silhouette_n_allocated );}
		silhouette_n_allocated = size;
	};
	void add_line(short one, short two){
		for(int i = 0; i<n_in_list; i+=2){
			if(silhouette_index[i] == one && silhouette_index[i+1] = two ||
				silhouette_index[i] == two && silhouette_index[i+1] = one){
				memcpy(&silhouette_index[i+2], &silhouette_index[i], n_in_list - i - 2);
				return;
			}else{
				allocate(n_in_list+2);
				silhouette_index[n_in_list] = one;
				silhouette_index[n_in_list+1] = two;
			}
		}
	};
	silhouette_index(){silhouette_n_allocated=0; n_in_list=0;};
	~silhouette_index(){vm_free(silhouette_index);};
};


void get_silhouette_from_point(vec3d *point_list, ubyte *bsp_data, vec3d* point){
}
*/

// returns 1 if the thruster should be drawn
//         0 if it shouldn't
int model_should_render_engine_glow(int objnum, int bank_obj)
{
	if ((bank_obj <= -1) || (objnum <= -1))
		return 1;

	object *obj = &Objects[objnum];

	if (obj == NULL)
		return 1;

	if (obj->type == OBJ_SHIP) {
		ship_subsys *ssp;
		ship *shipp = &Ships[obj->instance];
		ship_info *sip = &Ship_info[shipp->ship_info_index];

		Assert( bank_obj < sip->n_subsystems );

		char subname[MAX_NAME_LEN];
		// shipp->subsystems isn't always valid here so don't use it
		strncpy(subname, sip->subsystems[bank_obj].subobj_name, MAX_NAME_LEN);

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
		if (frame < 0) frame = 0;
		if (frame >= num_frames) frame = num_frames - 1;

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

void index_list::allocate(int size, bool large_buf)
{
	if (ibuffer)
		vm_free(ibuffer);

	if (sbuffer)
		vm_free(sbuffer);

	ibuffer = NULL;
	sbuffer = NULL;

	if (large_buf)
		ibuffer = (uint *) vm_malloc( sizeof(uint) * size );
	else
		sbuffer = (ushort *) vm_malloc( sizeof(ushort) * size );
}

void index_list::release()
{
	if (ibuffer)
		vm_free(ibuffer);

	if (sbuffer)
		vm_free(sbuffer);

	ibuffer = NULL;
	sbuffer = NULL;
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

void texture_map::Reset()
{
	for(int i = 0; i < TM_NUM_TYPES; i++)
		this->textures[i].ResetTexture();
}
