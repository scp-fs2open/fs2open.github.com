/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __SHOCKWAVE_H__
#define __SHOCKWAVE_H__

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"

class object;
class draw_list;

#define	SW_USED				(1<<0)
#define	SW_WEAPON			(1<<1)
#define	SW_SHIP_DEATH		(1<<2)
#define	SW_WEAPON_KILL		(1<<3)	// Shockwave created when weapon destroyed by another

#define	MAX_SHOCKWAVES					16
#define	SW_MAX_OBJS_HIT	64

// -----------------------------------------------------------
// Data structures
// -----------------------------------------------------------

typedef struct shockwave_info
{
	char filename[MAX_FILENAME_LEN];
	int	bitmap_id;
	int model_id;
	int	num_frames;
	int	fps;
	
	shockwave_info()
	: num_frames( 0 ), fps( 0 )
	{ 
		filename[ 0 ] = '\0';
		bitmap_id = -1; 
		model_id = -1; 
	}
} shockwave_info;

typedef struct shockwave {
	shockwave	*next, *prev;
	int			flags;
	int			objnum;					// index into Objects[] for shockwave
	int			num_objs_hit;
	int			obj_sig_hitlist[SW_MAX_OBJS_HIT];
	float		speed, radius;
	float		inner_radius, outer_radius, damage;
	int			weapon_info_index;	// -1 if shockwave not caused by weapon	
	int			damage_type_idx;			//What type of damage this shockwave does to armor
	vec3d		pos;
	float		blast;					// amount of blast to apply
	int			next_blast;				// timestamp for when to apply next blast damage
	int			shockwave_info_index;
	int			current_bitmap;
	float		time_elapsed;			// in seconds
	float		total_time;				// total lifetime of animation in seconds
	int			delay_stamp;			// for delayed shockwaves
	angles		rot_angles;
	int			model_id;
} shockwave;

typedef struct shockwave_create_info {

	char name[MAX_FILENAME_LEN];
	char pof_name[MAX_FILENAME_LEN];

	float inner_rad;
	float outer_rad;
	float damage;
	float blast;
	float speed;
	angles rot_angles;

	int damage_type_idx;
	int damage_type_idx_sav;	// stored value from table used to reset damage_type_idx

} shockwave_create_info;

extern void shockwave_create_info_init(shockwave_create_info *sci);
extern void shockwave_create_info_load(shockwave_create_info *sci);

void shockwave_level_init();
void shockwave_level_close();
void shockwave_delete(object *objp);
void shockwave_move_all(float frametime);
int  shockwave_create(int parent_objnum, vec3d *pos, shockwave_create_info *sci, int flag, int delay = -1);
void shockwave_render_DEPRECATED(object *objp);
void shockwave_render(object *objp, draw_list *scene);
int shockwave_load(char *s_name, bool shock_3D = false);

int   shockwave_get_weapon_index(int index);
float shockwave_get_min_radius(int index);
float shockwave_get_max_radius(int index);
float shockwave_get_damage(int index);
int   shockwave_get_damage_type_idx(int index);
int   shockwave_get_framenum(int index, int num_frames);
int   shockwave_get_flags(int index);

#endif /* __SHOCKWAVE_H__ */
