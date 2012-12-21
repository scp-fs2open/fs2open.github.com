/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __FREESPACE_CORKSCREW_H__
#define __FREESPACE_CORKSCREW_H__

struct object;

extern int Corkscrew_num_missiles_fired;

void	cscrew_level_init();
void	cscrew_delete(int index);
int	cscrew_create(object *obj);

// pre process the corkscrew weapon by putting him in the "center" of his corkscrew
void  cscrew_process_pre(object *objp);

// post process the corkscrew weapon by putting him back to the right spot on his corkscrew
void	cscrew_process_post(object *objp);

// maybe fire another corkscrew-style missile
void	cscrew_maybe_fire_missile(int shipnum);

#endif /* __FREESPACE_CORKSCREW_H__ */
