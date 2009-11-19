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

struct object;

#define	SW_USED				(1<<0)
#define	SW_WEAPON			(1<<1)
#define	SW_SHIP_DEATH		(1<<2)
#define	SW_WEAPON_KILL		(1<<3)	// Shockwave created when weapon destroyed by another

#define	MAX_SHOCKWAVES					16
#define	SW_MAX_OBJS_HIT	64

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

	void load();

	shockwave_create_info()
		: inner_rad( 0.f ), outer_rad( 0.f ), damage( 0.f ), blast( 0.f )		  
	{ 
		name[ 0 ] = '\0';
		pof_name[ 0 ] = '\0';
		damage_type_idx = -1;
		rot_angles.b = 0.;
		rot_angles.h = 0.;
		rot_angles.p = 0.;
	}
} shockwave_create_info;

void shockwave_close();
void shockwave_level_init();
void shockwave_level_close();
void shockwave_delete(object *objp);
void shockwave_move_all(float frametime);
int  shockwave_create(int parent_objnum, vec3d *pos, shockwave_create_info *sci, int flag, int delay = -1);
void shockwave_render(object *objp);

int   shockwave_get_weapon_index(int index);
float shockwave_get_min_radius(int index);
float shockwave_get_max_radius(int index);
float shockwave_get_damage(int index);
int   shockwave_get_damage_type_idx(int index);
int   shockwave_get_framenum(int index, int num_frames);
int   shockwave_get_flags(int index);

#endif /* __SHOCKWAVE_H__ */
