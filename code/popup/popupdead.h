/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __POPUPDEAD_H__
#define __POPUPDEAD_H__

// return values for popup_do_frame for multiplayer
#define POPUPDEAD_DO_RESPAWN		0
#define POPUPDEAD_DO_OBSERVER		1
#define POPUPDEAD_DO_MAIN_HALL	2

void	popupdead_start();
void	popupdead_close();
int	popupdead_do_frame(float frametime);
int	popupdead_is_active();

#endif
