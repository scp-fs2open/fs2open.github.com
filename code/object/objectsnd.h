/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __OBJECTSNDS_H__
#define __OBJECTSNDS_H__

#include "gamesnd/gamesnd.h"

#define	OS_USED					(1<<0)
#define OS_MAIN					(1<<1)		// "main" sound. attentuation does not apply until outside the radius of the object
#define OS_TURRET_BASE_ROTATION	(1<<2)
#define OS_TURRET_GUN_ROTATION	(1<<3)
#define OS_SUBSYS_ALIVE			(1<<4)
#define OS_SUBSYS_DEAD			(1<<5)
#define OS_SUBSYS_DAMAGED		(1<<6)
#define OS_SUBSYS_ROTATION		(1<<7)
#define OS_PLAY_ON_PLAYER		(1<<8)
#define OS_LOOPING_DISABLED		(1<<9)

struct vec3d;
class ship_subsys;

extern int Obj_snd_enabled;

void	obj_snd_level_init();
void	obj_snd_level_close();
void	obj_snd_do_frame();

// pos is the position of the sound source in the object's frame of reference.
// so, if the objp->pos was at the origin, the pos passed here would be the exact
// model coords of the location of the engine
// by passing vmd_zero_vector here, you get a sound centered directly on the object
// This function used to have a "main" argument, but that is equivalent to including OS_MAIN as one of the flags
int	obj_snd_assign(int objnum, gamesnd_id sndnum, const vec3d *pos, int flags = 0, const ship_subsys *associated_sub = nullptr);

//Delete specific persistent sound on object
void obj_snd_delete(object *objp, int index, bool stop_sound = true);

// if sndnum is not -1, deletes all instances of the given sound within the object
void	obj_snd_delete_type(int objnum, gamesnd_id sndnum = gamesnd_id(), ship_subsys *ss = NULL);

void	obj_snd_delete_all();
void	obj_snd_stop_all();

#endif
