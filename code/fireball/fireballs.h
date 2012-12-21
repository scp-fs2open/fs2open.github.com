/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/  



#ifndef _FIREBALLS_H
#define _FIREBALLS_H

#include "globalincs/pstypes.h"

struct object;
struct ship_info;
class asteroid_info;

// values correspond to the fireball render types
#define FIREBALL_MEDIUM_EXPLOSION	0
#define FIREBALL_LARGE_EXPLOSION	1
#define FIREBALL_WARP_EFFECT		2

// these values correspond to the fireball.tbl default entries
#define FIREBALL_EXPLOSION_MEDIUM	0		// Used for the 4 little explosions before a ship explodes
#define FIREBALL_WARP				1		// Used for the warp in / warp out effect
#define FIREBALL_KNOSSOS			2		// Used for the KNOSSOS warp in / warp out effect
#define FIREBALL_ASTEROID			3
#define FIREBALL_EXPLOSION_LARGE1	4		// Used for the big explosion when a ship breaks into pieces
#define FIREBALL_EXPLOSION_LARGE2	5		// Used for the big explosion when a ship breaks into pieces

#define MAX_FIREBALL_TYPES			32		// The maximum number of fireballs that can be defined
#define NUM_DEFAULT_FIREBALLS		6

#define FIREBALL_NUM_LARGE_EXPLOSIONS 2

extern int fireball_used[MAX_FIREBALL_TYPES];
extern int Num_fireball_types;

// all this moved here by Goober5000 because it makes more sense in the H file
typedef struct fireball_lod {
	char	filename[MAX_FILENAME_LEN];
	int		bitmap_id;
	int		num_frames;
	int		fps;
} fireball_lod;

typedef struct fireball_info {
	int					lod_count;
	float				exp_color[3];
	fireball_lod		lod[4];
} fireball_info;

// flag values for fireball struct flags member
#define	FBF_WARP_CLOSE_SOUND_PLAYED		(1<<0)
#define	FBF_WARP_CAPITAL_SIZE			(1<<1)
#define	FBF_WARP_CRUISER_SIZE			(1<<2)
#define FBF_WARP_3D						(1<<3)	// Goober5000
#define FBF_WARP_VIA_SEXP				(1<<4)	// Goober5000

typedef struct fireball {
	int		objnum;					// If -1 this object is unused
	int		fireball_info_index;	// Index into Fireball_info array
	int		fireball_render_type;
	int		current_bitmap;
	int		orient;					// For fireballs, which orientation.  For warps, 0 is warpin, 1 is warpout
	int		flags;					// see #define FBF_*
	char	lod;					// current LOD
	float	time_elapsed;			// in seconds
	float	total_time;				// total lifetime of animation in seconds
	int warp_open_sound_index;		// for warp-effect - Goober5000
	int warp_close_sound_index;		// for warp-effect - Goober5000
} fireball;
// end move

void fireball_init();
void fireball_render(object * obj);
void fireball_delete( object * obj );
void fireball_process_post(object * obj, float frame_time);

// reversed is for warp_in/out effects
// Velocity: If not NULL, the fireball will move at a constant velocity.
// warp_lifetime: If warp_lifetime > 0.0f then makes the explosion loop so it lasts this long.  Only works for warp effect
int fireball_create(vec3d *pos, int fireball_type, int render_type, int parent_obj, float size, int reversed=0, vec3d *velocity=NULL, float warp_lifetime=0.0f, int ship_class=-1, matrix *orient=NULL, int low_res=0, int extra_flags=0, int warp_open_sound=-1, int warp_close_sound=-1); 
void fireball_render_plane(int plane);
void fireball_close();

// Returns 1 if you can remove this fireball
int fireball_is_perishable(object * obj);

// Returns 1 if this fireball is a warp 
int fireball_is_warp(object * obj);

// Returns life left of a fireball in seconds
float fireball_lifeleft( object *obj );

// Returns life left of a fireball in percent
float fireball_lifeleft_percent( object *obj );

// returns the lighting color (in [0...1] range) to use for explosion
void fireball_get_color(int idx, float *red, float *green, float *blue);

// returns the index of the fireball bitmap for this ship. -1 if there is none.
int fireball_ship_explosion_type(ship_info *sip);

// returns the index of the fireball bitmap for this asteroid. -1 if there is none.
int fireball_asteroid_explosion_type(asteroid_info *aip);

// returns the intensity of a wormhole
float fireball_wormhole_intensity( object *obj );

// internal function to draw warp grid.
extern void warpin_render(object *obj, matrix *orient, vec3d *pos, int texture_bitmap_num, float radius, float life_percent, float max_radius, int warp_3d = 0 );

extern int Warp_model;

// Goober5000
extern int Knossos_warp_ani_used;

#endif /* _FIREBALLS_H */
