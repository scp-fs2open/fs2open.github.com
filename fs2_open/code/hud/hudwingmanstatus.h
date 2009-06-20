/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __HUDWINGMAN_STATUS_H__
#define __HUDWINGMAN_STATUS_H__

void	hud_init_wingman_status_gauge();
void	hud_wingman_status_update();
void	hud_wingman_status_render();
void	hud_wingman_status_init_flash();
int	hud_wingman_status_maybe_flash(int wing_index, int wing_pos);
void	hud_set_wingman_status_dead(int wing_index, int wing_pos);
void	hud_set_wingman_status_departed(int wing_index, int wing_pos);
void	hud_set_wingman_status_alive( int wing_index, int wing_pos);
void	hud_set_wingman_status_none( int wing_index, int wing_pos);
void	hud_wingman_status_start_flash(int wing_index, int wing_pos);
void	hud_wingman_status_set_index(int shipnum);

#endif
