/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __DS3D_H__
#define __DS3D_H__

int	ds3d_init(int voice_manager_required);
void	ds3d_close();
int	ds3d_update_listener(vec3d *pos, vec3d *vel, matrix *orient);
int	ds3d_update_buffer(int channel, float min, float max, vec3d *pos, vec3d *vel);
int	ds3d_set_sound_cone(int channel, int inner_angle, int outer_angle, int vol);

#endif /* __DS3D_H__ */
