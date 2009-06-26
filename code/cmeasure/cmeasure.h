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

struct object;
//struct vec3d;

//#define MAX_CMEASURES 64

//	Goes in cmeasure.subtype
//#define	CMEASURE_UNUSED			-1

//#define	MAX_CMEASURE_TYPES		3

#define	CMEASURE_WAIT				333			//	delay in milliseconds between countermeasure firing.

//#define	CMF_DUD_HEAT				0x01			//	If set, this cmeasure is a dud to heat seekers.  Set at create time.
//#define	CMF_DUD_ASPECT				0x02			//	If set, this cmeasure is a dud to aspect seekers.  Set at create time.

//#define	CMF_DUD	(CMF_DUD_HEAT | CMF_DUD_ASPECT)

//	Maximum distance at which a countermeasure can be tracked
//	If this value is too large, missiles will always be tracking countermeasures.
#define	MAX_CMEASURE_TRACK_DIST	300.0f
/*
typedef struct cmeasure_info {
	char	cmeasure_name[NAME_LENGTH];
	//float	max_speed;								// launch speed, relative to ship
	float	fire_wait;								//	time between launches
	float	life_min, life_max;					//	lifetime will be in range min..max.
	int	launch_sound;							//	Sound played when launched.
	char	pof_name[NAME_LENGTH];
	int	model_num;								// What this renders as
} cmeasure_info;

typedef struct cmeasure {
	int		flags;				//	You know, flag type things.
	int		subtype;				// See CMEASURE_??? defines
	int		objnum;
	int		source_objnum;		// What object this came from
	int		source_sig;			// Signature of the source object
	int		team;					// Team of the ship where the cmeasure came from
	float		lifeleft;			// When 0 or less object dies
} cmeasure;
*/
//extern cmeasure_info Cmeasure_info[MAX_CMEASURE_TYPES];
//extern cmeasure Cmeasures[MAX_CMEASURES];
//extern int Num_cmeasure_types;
//extern int Num_cmeasures;
extern int Cmeasures_homing_check;
extern int Countermeasures_enabled;

//extern void cmeasure_init();
//extern void cmeasure_render( object * obj );
//extern void cmeasure_delete( object * obj );
//extern void cmeasure_process_pre( object * obj, float frame_time);
//extern void cmeasure_process_post( object * obj, float frame_time);
//extern int cmeasure_create( object * source_obj, vec3d *pos, int cm_type, int rand_val = -1 );
extern void cmeasure_set_ship_launch_vel(object *objp, object *parent_objp, int arand);
extern void cmeasure_select_next(object *objp);
extern void cmeasure_maybe_alert_success(object *objp);

extern float Skill_level_cmeasure_life_scale[NUM_SKILL_LEVELS];

#endif // _CMEASURE_H
