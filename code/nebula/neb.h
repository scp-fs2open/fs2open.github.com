/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FS2_NEB2_EFFECT_HEADER_FILE
#define _FS2_NEB2_EFFECT_HEADER_FILE

// --------------------------------------------------------------------------------------------------------
// NEBULA DEFINES/VARS
//
#include "camera/camera.h"
#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

class ship;
class object;

extern bool Nebula_sexp_used;
// fog near and far values for rendering the background nebula
extern float Neb_backg_fog_near;
extern float Neb_backg_fog_far;

// nebula rendering mode
#define NEB2_RENDER_NONE								0			// no rendering
#define NEB2_RENDER_POF									1			// background is the nice pof file -- used by FRED
#define NEB2_RENDER_HTL									2			// We are using proper fogging now 
extern int Neb2_render_mode;

// the AWACS suppresion level for the nebula
extern float Neb2_awacs;

// The visual render distance multipliers for the nebula
extern float Neb2_fog_near_mult;
extern float Neb2_fog_far_mult;

extern int Neb2_poof_flags;
const size_t MAX_NEB2_POOFS = 32;

#define MAX_NEB2_BITMAPS			10

// pof texture filenames
extern char Neb2_bitmap_filenames[MAX_NEB2_BITMAPS][MAX_FILENAME_LEN];

// texture to use for this level
extern char Neb2_texture_name[MAX_FILENAME_LEN];

typedef struct poof_info {
	char bitmap_filename[MAX_FILENAME_LEN];
	int bitmap;
	float scale_min;
	float scale_max;
	float density;   // IN CUBIC KILOMETERS!!!
	float rotation_min;
	float rotation_max;
	float view_dist;
	float peak_alpha;

	poof_info() {
		bitmap_filename[0] = '\0';
		bitmap = -1;
		scale_min = 150.0f;
		scale_max = 150.0f;
		density = 150.f;
		rotation_min = -0.065f;
		rotation_max = 0.065f;
		view_dist = 360.f;
		peak_alpha = 0.35f;
	}
} poof_info;

extern SCP_vector<poof_info> Poof_info;

// nebula poofs
typedef struct poof {
	vec3d	pt;				// point in space
	int		poof_info_index;
	float		radius;
	float		rot;				// rotation angle
	float		rot_speed;		// rotation speed, deg/sec
	float		flash;			// lightning flash
} poof;

extern SCP_vector<poof> Neb2_poofs;

// nebula detail level
typedef struct neb2_detail {
	float max_alpha_glide;		// max alpha for this detail level in Glide
	float max_alpha_d3d;		// max alpha for this detail level in D3d
	float break_alpha;			// break alpha (below which, poofs don't draw). this affects the speed and visual quality a lot
	float break_x, break_y;		// x and y alpha fade/break values. adjust alpha on the polys as they move offscreen
	float cube_dim;				// total dimension of player poof cube
	float cube_inner;			// inner radius of the player poof cube
	float cube_outer;			// outer radius of the player pood cube
	float prad;					// radius of the poofs
	float wj, hj, dj;			// width, height, depth jittering. best left at 1.0
} neb2_detail;


// --------------------------------------------------------------------------------------------------------
// NEBULA FUNCTIONS
//

// neb2 stuff (specific nebula densities) -----------------------------------

// initialize neb2 stuff at game startup
void neb2_init();

// set detail level
void neb2_set_detail_level(int level);

//init neb stuff  - WMC
void neb2_level_init();

// initialize nebula stuff - call from game_post_level_init(), so the mission has been loaded
void neb2_post_level_init();

// shutdown nebula stuff
void neb2_level_close();

// call before beginning all rendering
void neb2_render_setup(camid cid);

// render the player nebula
void neb2_render_poofs();

// get near and far fog values based upon object type and rendering mode
void neb2_get_fog_values(float *fnear, float *ffar, object *obj = NULL);

// get adjusted near and far fog values (allows mission-specific fog adjustments)
void neb2_get_adjusted_fog_values(float *fnear, float *ffar, object *obj = NULL);

// given a position in space, return a value from 0.0 to 1.0 representing the fog level 
float neb2_get_fog_intensity(object *obj);
float neb2_get_fog_intensity(vec3d *pos);

// should we not render this object because its obscured by the nebula?
int neb2_skip_render(object *objp, float z_depth);

// extend LOD 
float neb2_get_lod_scale(int objnum);

// fogging stuff --------------------------------------------------

void neb2_get_fog_color(ubyte *r, ubyte *g, ubyte *b);

#endif
