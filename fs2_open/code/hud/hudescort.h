/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __FREESPACE_HUDESCORT_VIEW_H__
#define __FREESPACE_HUDESCORT_VIEW_H__

//Odd def for escort frames
#define NUM_ESCORT_FRAMES 4

extern int Max_escort_ships;

struct object;

void	hud_escort_init();
void	hud_setup_escort_list(int level = 1);
void	hud_display_escort();
void	hud_escort_view_toggle();
void	hud_add_remove_ship_escort(int objnum, int supress_feedback = 0);
void	hud_escort_clear_all();
void	hud_escort_ship_hit(object *objp, int quadrant);
void	hud_escort_target_next();
void	hud_escort_cull_list();
void	hud_add_ship_to_escort(int objnum, int supress_feedback);
void  hud_remove_ship_from_escort(int objnum);
int	hud_escort_num_ships_on_list();
int	hud_escort_return_objnum(int index);
void	hud_escort_add_player(short id);
void	hud_escort_remove_player(short id);

#endif /* __FREESPACE_HUDESCORT_VIEW_H__ */
