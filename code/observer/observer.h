/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _OBSERVER_HEADER_FILE
#define _OBSERVER_HEADER_FILE

class object;
struct vec3d;
struct matrix;

#define OBS_MAX_VEL_X     (85.0f) // side to side
#define OBS_MAX_VEL_Y     (85.0f) // side to side
#define OBS_MAX_VEL_Z     (85.0f) // forwards and backwards


#define OBS_FLAG_USED   (1<<1)

typedef struct observer {
	int objnum;

	int target_objnum;    // not used as of yet
	int flags;
} observer;

#define MAX_OBSERVER_OBS 17
extern observer Observers[MAX_OBSERVER_OBS];

void observer_init();
int observer_create(matrix *orient, vec3d *pos);  // returns objnum
void observer_delete(object *obj);

// get the eye position and orientation for the passed observer object
void observer_get_eye(vec3d *eye_pos, matrix *eye_orient, object *obj);

#endif
