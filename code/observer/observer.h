/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Observer/Observer.h $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:09 $
 * $Author: penguin $
 *
 * $NoKeywords: $
 */

#ifndef _OBSERVER_HEADER_FILE
#define _OBSERVER_HEADER_FILE

#include "object/object.h"
#include "math/vecmat.h"

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

extern int Num_observer_obs;

void observer_init();
int observer_create(matrix *orient, vector *pos);  // returns objnum
void observer_delete(object *obj);

// get the eye position and orientation for the passed observer object
void observer_get_eye(vector *eye_pos, matrix *eye_orient, object *obj);

#endif
