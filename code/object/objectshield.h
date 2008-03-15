/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _OBJECTSHIELD_H
#define _OBJECTSHIELD_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

#define	FRONT_QUAD	1
#define	REAR_QUAD	2
#define	LEFT_QUAD	3
#define	RIGHT_QUAD	0

float shield_get_strength(object *objp);
void shield_set_strength(object *objp, float strength);
void shield_add_strength(object *objp, float delta);
float shield_get_quad(object *objp, int quadrant_num);
void shield_set_quad(object *objp, int quadrant_num, float strength);
void shield_add_quad(object *objp, int quadrant_num, float strength);

float shield_get_max_strength(object *objp);
void shield_set_max_strength(object *objp, float newmax);
float shield_get_max_quad(object *objp);

float shield_apply_damage(object *objp, int quadrant, float damage);
int shield_is_up(object *objp, int quadrant_num);
int shield_get_quadrant(vec3d *hit_pnt);
int shield_get_quadrant_global(object *objp, vec3d *global_pos);

#endif //_OBJECTSHIELD_H
