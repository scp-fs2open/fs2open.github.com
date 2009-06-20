/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __AFTERBURNER_H__
#define __AFTERBURNER_H__

#define	AFTERBURNER_MIN_DELAY	1000
#define	ABURN_DECAY_TIME			1500		// time in ms ship shakes after afterburner pressed

struct object;

void afterburners_start(object *objp);
void afterburners_stop(object *objp, int key_released = 0);
void afterburner_stop_sounds();
void afterburners_update(object *objp, float fl_frametime);
void afterburner_level_init();

#endif /* __AFTERBURNER_H__ */

