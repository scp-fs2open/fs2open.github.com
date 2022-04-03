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

class object;
class ship_subsys;
struct weapon_info;

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

struct swarm_info {
	int		change_timestamp;
	vec3d	current_target;
	vec3d	offset;
	int		homing_objnum;		// object number that swarm missile is homing on, -1 if not homing
	float	zig_direction;
};

#define SWARM_DEFAULT_NUM_MISSILES_FIRED					4		// number of swarm missiles that launch when fired

#define MAX_TURRET_SWARM_INFO	100
extern turret_swarm_info Turret_swarm_info[MAX_TURRET_SWARM_INFO];

void	swarm_level_init();
void	swarm_create(object* objp, swarm_info* swarmp);
void	swarm_update_direction(object *objp, swarm_info* swarmp);
void	swarm_maybe_fire_missile(int shipnum);

int	turret_swarm_create();
void	turret_swarm_delete(int i);
void	turret_swarm_set_up_info(int parent_objnum, ship_subsys *turret, weapon_info *wip, int weapon_num);
void	turret_swarm_check_validity();

#endif /* __FREESPACE_SWARM_H__ */
