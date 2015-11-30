/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _CMEASURE_H
#define _CMEASURE_H

#include "globalincs/globals.h"
#include "globalincs/systemvars.h"

class object;

#define	CMEASURE_WAIT				333			//	delay in milliseconds between countermeasure firing.

//	Maximum distance at which a countermeasure can be tracked
//	If this value is too large, missiles will always be tracking countermeasures.
#define	MAX_CMEASURE_TRACK_DIST	300.0f
extern const float CMEASURE_DETONATE_DISTANCE;

extern int Cmeasures_homing_check;
extern int Countermeasures_enabled;

extern void cmeasure_set_ship_launch_vel(object *objp, object *parent_objp, int arand);
extern void cmeasure_select_next(object *objp);
extern void cmeasure_maybe_alert_success(object *objp);

#endif // _CMEASURE_H
