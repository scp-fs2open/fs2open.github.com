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

int	ds3d_update_listener(const vec3d *pos, const vec3d *vel, const matrix *orient);
int	ds3d_update_buffer(int channel, float min, float max, const vec3d *pos, const vec3d *vel);

#endif /* __DS3D_H__ */
