/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/CMeasure/CMeasure.h $
 * $Revision: 2.2 $
 * $Date: 2004-03-05 09:01:57 $
 * $Author: Goober5000 $
 *
 * Counter measures.  Created by Mike Kulas, May 12, 1997.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 20    4/11/98 5:17p Jim
 * Reduced countermeasure firing rate from 2 times per second to 3 times
 * per second.
 * 
 * 19    4/10/98 11:02p Mike
 * Make countermeasures less effective against aspect seekers than against
 * heat seekers.
 * Make AI ships match bank with each other when attacking a faraway
 * target.
 * Make ships not do silly loop-de-loop sometimes when attacking a faraway
 * target.
 * 
 * 18    4/01/98 5:34p John
 * Made only the used POFs page in for a level.   Reduced some interp
 * arrays.    Made custom detail level work differently.
 * 
 * 17    2/26/98 10:07p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 16    2/23/98 4:30p Mike
 * Make homing missiles detonate after they pass up their target.  Make
 * countermeasures less effective.
 * 
 * 15    2/09/98 8:04p Lawrance
 * Add objnum to cmeasure struct, correctly set source_objnum
 * 
 * 14    2/05/98 11:20p Lawrance
 * save/restore countermeasure data
 * 
 * 13    1/29/98 11:48a John
 * Added new counter measure rendering as model code.   Made weapons be
 * able to have impact explosion.
 * 
 * 12    1/16/98 11:43a Mike
 * Fix countermeasures.
 * 
 * 11    8/13/97 9:50p Allender
 * split *_move into *_process_pre and *_process_post functions.
 * process_pre functions called before object is moved.  _process_post
 * functions called after object is moved.  Reordered code in ship_post
 * and weapon_post for multiplayer
 * 
 * 10    8/13/97 4:45p Allender
 * fixed ship_fire_primary and fire_secondary to not take parameter for
 * determining whether to count ammo or not.  Fixed countermeasure firing
 * for multiplayer
 * 
 * 9     8/08/97 4:29p Allender
 * countermeasure stuff for multiplayer
 * 
 * 8     7/09/97 12:04a Mike
 * Error prevention in matrix_interpolate().
 * 
 * 7     6/24/97 10:04a Allender
 * major multiplayer improvements.  Better sequencing before game.
 * Dealing with weapon/fireball/counter measure objects between
 * client/host.  
 * 
 * 6     5/22/97 5:45p Mike
 * Better countermeasure firing, key off availability, specify in
 * weapons.tbl
 * 
 * 5     5/15/97 5:05p Mike
 * In the midst of changing subsystem targetnig from type-based to
 * pointer-based.
 * Also allowed you to view a target while dead.
 * 
 * 4     5/14/97 4:08p Lawrance
 * removing my_index from game arrays
 * 
 * 3     5/14/97 10:50a Mike
 * More countermeasure stuff.
 * 
 * 2     5/12/97 5:58p Mike
 * Add countermeasures.
 * 
 * 1     5/12/97 2:23p Mike
 *
 * $NoKeywords: $
 */

#ifndef _CMEASURE_H
#define _CMEASURE_H

#include "globalincs/globals.h"

struct object;
struct vector;

#define MAX_CMEASURES 64

//	Goes in cmeasure.subtype
#define	CMEASURE_UNUSED			-1

#define	MAX_CMEASURE_TYPES		3

#define	CMEASURE_WAIT				333			//	delay in milliseconds between countermeasure firing.

#define	CMF_DUD_HEAT				0x01			//	If set, this cmeasure is a dud to heat seekers.  Set at create time.
#define	CMF_DUD_ASPECT				0x02			//	If set, this cmeasure is a dud to aspect seekers.  Set at create time.

#define	CMF_DUD	(CMF_DUD_HEAT | CMF_DUD_ASPECT)

//	Maximum distance at which a countermeasure can be tracked
//	If this value is too large, missiles will always be tracking countermeasures.
#define	MAX_CMEASURE_TRACK_DIST	300.0f

typedef struct cmeasure_info {
	char	cmeasure_name[NAME_LENGTH];
	float	max_speed;								// launch speed, relative to ship
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

extern cmeasure_info Cmeasure_info[MAX_CMEASURE_TYPES];
extern cmeasure Cmeasures[MAX_CMEASURES];
extern int Num_cmeasure_types;
extern int Num_cmeasures;
extern int Cmeasures_homing_check;

extern void cmeasure_init();
extern void cmeasure_render( object * obj );
extern void cmeasure_delete( object * obj );
extern void cmeasure_process_pre( object * obj, float frame_time);
extern void cmeasure_process_post( object * obj, float frame_time);
extern int cmeasure_create( object * source_obj, vector *pos, int cm_type, int rand_val = -1 );
extern void cmeasure_select_next(object *objp);

#endif // _CMEASURE_H
