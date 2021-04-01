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

#define NEB_FOG_VISIBILITY_MULT_TRAIL			1.0f
#define NEB_FOG_VISIBILITY_MULT_THRUSTER		1.5f
#define NEB_FOG_VISIBILITY_MULT_WEAPON			1.3f
#define NEB_FOG_VISIBILITY_MULT_SHIELD			1.2f
#define NEB_FOG_VISIBILITY_MULT_GLOWPOINT		1.2f
#define NEB_FOG_VISIBILITY_MULT_BEAM(size)		4.0f + (size / 10)
#define NEB_FOG_VISIBILITY_MULT_B_MUZZLE(size)  NEB_FOG_VISIBILITY_MULT_BEAM(size)
#define NEB_FOG_VISIBILITY_MULT_PARTICLE(size)  1.0f + (size / 12)
#define NEB_FOG_VISIBILITY_MULT_SHOCKWAVE		2.5f
#define NEB_FOG_VISIBILITY_MULT_FIREBALL(size)	1.2f + (size / 12)

#define MAX_NEB2_POOFS				32

// poof names and flags (for fred)
extern char Neb2_poof_filenames[MAX_NEB2_POOFS][MAX_FILENAME_LEN];	
extern int Neb2_poof_flags;

#define MAX_NEB2_BITMAPS			10

// pof texture filenames
extern char Neb2_bitmap_filenames[MAX_NEB2_BITMAPS][MAX_FILENAME_LEN];

// texture to use for this level
extern char Neb2_texture_name[MAX_FILENAME_LEN];

// how many "slices" are in the current player nebuls
extern int Neb2_slices;

// the color of the fog/background
extern ubyte Neb2_fog_color[3];

// nebula poofs
typedef struct cube_poof {
	vec3d	pt;				// point in space
	int		bmap;				// bitmap in space
	float		rot;				// rotation angle
	float		rot_speed;		// rotation speed
	float		flash;			// lightning flash
} cube_poof;
#define MAX_CPTS		5		// should always be <= slices
extern cube_poof Neb2_cubes[MAX_CPTS][MAX_CPTS][MAX_CPTS];

// nebula detail level
typedef struct neb2_detail {
	float max_alpha_glide;		// max alpha for this detail level in Glide
	float max_alpha_d3d;		// max alpha for this detail level in D3d
	float break_alpha;			// break alpha (below which, poofs don't draw). this affects the speed and visual quality a lot
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

// call this when the player's viewpoint has changed, this will cause the code to properly reset
// the eye's local poofs
void neb2_eye_changed();

// get near and far fog values based upon object type and rendering mode
void neb2_get_fog_values(float *fnear, float *ffar, object *obj = NULL);

// get adjusted near and far fog values (allows mission-specific fog adjustments)
void neb2_get_adjusted_fog_values(float *fnear, float *ffar, float *fdensity = nullptr, object *obj = nullptr);

// given a position, returns 0 - 1 the fog visibility of that position, 0 = completely obscured
// distance_mult will multiply the result, use for things that can be obscured but can 'shine through' the nebula more than normal
float neb2_get_fog_visibility (vec3d* pos, float distance_mult);

// should we not render this object because its obscured by the nebula?
int neb2_skip_render(object *objp, float z_depth);

// extend LOD 
float neb2_get_lod_scale(int objnum);

// fogging stuff --------------------------------------------------

void neb2_get_fog_color(ubyte *r, ubyte *g, ubyte *b);

#endif
