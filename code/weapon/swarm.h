/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __FREESPACE_SWARM_H__
#define __FREESPACE_SWARM_H__

#include "globalincs/pstypes.h"

struct object;
struct ship_subsys;

typedef struct turret_swarm_info {
	int				flags;
	int				weapon_class;
	int				num_to_launch;
	int				parent_objnum;
	int				parent_sig;
	int				target_objnum;
	int				target_sig;
	ship_subsys*	turret;
	ship_subsys*	target_subsys;
	int				time_to_fire;
	int				weapon_num;
} turret_swarm_info;

typedef struct swarm_info {
	int		flags;
	int		change_timestamp;
	vec3d	original_target;
	vec3d	new_target;
	vec3d	circle_rvec, circle_uvec;
	vec3d	last_offset;
	uint		change_count;		
	int		path_num;			// which path swarm missile is currently following
	int		homing_objnum;		// object number that swarm missile is homing on, -1 if not homing
	int		change_time;		// when swarm missile should next update direction, based on missile speed
	float		angle_offset;
	float		last_dist;			// last distance to target
} swarm_info;

#define SWARM_DEFAULT_NUM_MISSILES_FIRED					4		// number of swarm missiles that launch when fired

#define MAX_SWARM_MISSILES	100
extern swarm_info	Swarm_missiles[MAX_SWARM_MISSILES];

#define MAX_TURRET_SWARM_INFO	100
extern turret_swarm_info Turret_swarm_info[MAX_TURRET_SWARM_INFO];

void	swarm_level_init();
void	swarm_delete(int index);
int	swarm_create();
void	swarm_update_direction(object *objp, float frametime);
void	swarm_maybe_fire_missile(int shipnum);

int	turret_swarm_create();
void	turret_swarm_delete(int i);
void	turret_swarm_set_up_info(int parent_objnum, ship_subsys *turret, struct weapon_info *wip, int weapon_num);
void	turret_swarm_check_validity();

#endif /* __FREESPACE_SWARM_H__ */
