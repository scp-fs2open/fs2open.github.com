/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Debris/Debris.h $
 * $Revision: 2.3 $
 * $Date: 2005-04-05 05:53:15 $
 * $Author: taylor $
 *
 * Code for the pieces of exploding object debris.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2004/08/11 05:06:20  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2004/03/05 09:01:59  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 6     7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 5     7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 4     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 3     10/16/98 1:22p Andsager
 * clean up header files
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 29    4/02/98 6:30p Lawrance
 * reduce max debris piece if DEMO defined
 * 
 * 28    4/02/98 11:40a Lawrance
 * check for #ifdef DEMO instead of #ifdef DEMO_RELEASE
 * 
 * 27    2/26/98 10:07p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 26    2/19/98 12:46a Lawrance
 * Further work on asteroids.
 * 
 * 25    2/10/98 6:43p Lawrance
 * Moved asteroid code to a separate lib.
 * 
 * 24    2/06/98 3:08p Mike
 * More asteroid stuff, including resolving conflicts between the two
 * asteroid_field structs!
 * 
 * 23    2/05/98 12:51a Mike
 * Early asteroid stuff.
 * 
 * 22    1/29/98 5:50p John
 * Made electrical arcing on debris pieces be persistent from frame to
 * frame
 * 
 * 21    1/21/98 7:18p Lawrance
 * Fix bug with creating debris before the mission starts.
 * 
 * 20    12/22/97 9:56p Andsager
 * Implement ship:debris collisions.  Generalize and move
 * ship_ship_or_debris_hit struct from CollideShipShip to ObjCollide.h
 * 
 * 19    11/19/97 5:57p Mike
 * Make debris go away if more than N units away from the nearest player.
 * Make speed of a docked object not read zero if it's moving.
 * 
 * 18    10/01/97 10:52a Mike
 * Change debris prototype to take an additional parameter to give debris
 * more thrust when it is created.
 * 
 * 17    9/20/97 9:25a John
 * fixed bug with species specific debris shards.
 * 
 * 16    8/13/97 9:50p Allender
 * split *_move into *_process_pre and *_process_post functions.
 * process_pre functions called before object is moved.  _process_post
 * functions called after object is moved.  Reordered code in ship_post
 * and weapon_post for multiplayer
 * 
 * 15    7/24/97 8:35a Allender
 * allow ships to blow up before missions starts.  Made some debris system
 * items available in other modules
 * 
 * 14    7/13/97 5:53p Lawrance
 * save model name in debris
 * 
 * 13    7/02/97 11:48a Lawrance
 * update debris to delay persistant sound
 * 
 * 12    6/06/97 4:13p Lawrance
 * leave debris chunks from very large ships persistant, scale fireball
 * lifetime by ship radius
 * 
 * 11    6/06/97 12:12p Lawrance
 * limit hull debris pieces, destroying oldest debris to make way for new
 * debris.  
 * 
 * 10    5/14/97 4:08p Lawrance
 * removing my_index from game arrays
 * 
 * 9     4/03/97 12:23a Mike
 * Make debris collidable-with.  Can hit it, too.
 * Make AI ships stop firing when target enters death roll.
 * 
 * 8     3/31/97 11:11a John
 * added ship species specific debris.
 * 
 * 7     3/25/97 3:55p Lawrance
 * allowing debris to be targeted and shown on radar
 * 
 * 6     2/27/97 12:07p John
 * Added code to suppord debris chunks after a ship is exploded..
 * 
 * 5     2/26/97 2:18p Lawrance
 * adding Debris[] to save/restore
 * 
 * 4     2/10/97 12:38p John
 * made all ships blow up into debris pieces when exploded.
 * 
 * 3     2/07/97 10:45a John
 * Initial bare bones implementation of blowing off turrets.
 * 
 * 2     2/07/97 9:30a John
 * 
 * 1     2/06/97 4:13p John
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _DEBRIS_H
#define _DEBRIS_H

#include "globalincs/pstypes.h"

struct object;
struct CFILE;

#define MAX_DEBRIS_ARCS 8		// Must be less than MAX_ARC_EFFECTS in model.h

typedef struct debris {
	debris	*next, *prev;		// used for a linked list of the hull debris chunks
	int		flags;					// See DEBRIS_??? defines
	int		source_objnum;		// What object this came from
	int		source_sig;			// Signature of the source object
	int		ship_info_index;	// Ship info index of the ship type debris came from
	int		team;					// Team of the ship where the debris came from
	int		objnum;				// What object this is linked to
	float		lifeleft;			// When 0 or less object dies
	int		model_num;			// What model this uses
	int		submodel_num;		// What submodel this uses
	int		next_fireball;		// When to start a fireball
	int		is_hull;				// indicates a large hull chunk of debris
	int		species;				// What species this is from.  -1 if don't care.
	int		fire_timeout;		// timestamp that holds time for fireballs to stop appearing
	int		sound_delay;		// timestamp to signal when sound should start
	fix		time_started;		// time when debris was created
	int		next_distance_check;	//	timestamp to determine whether to delete this piece of debris.

	vec3d	arc_pts[MAX_DEBRIS_ARCS][2];		// The endpoints of each arc
	int		arc_timestamp[MAX_DEBRIS_ARCS];	// When this times out, the spark goes away.  -1 is not used
	int		arc_frequency;							// Starts at 0, gets bigger
	
} debris;


// flags for debris pieces
#define	DEBRIS_USED				(1<<0)
#define	DEBRIS_EXPIRE			(1<<1)	// debris can expire (ie hull chunks from small ships)


#ifdef FS2_DEMO
	#define	MAX_DEBRIS_PIECES	48
#else
	#define	MAX_DEBRIS_PIECES	64
#endif

extern	debris Debris[MAX_DEBRIS_PIECES];

extern int Num_debris_pieces;

struct collision_info_struct;

void debris_init();
void debris_render( object * obj );
void debris_delete( object * obj );
void debris_process_pre( object * obj, float frame_time);
void debris_process_post( object * obj, float frame_time);
object *debris_create( object * source_obj, int model_num, int submodel_num, vec3d *pos, vec3d *exp_center, int hull_flag, float exp_force );
int debris_check_collision( object * obj, object * other_obj, vec3d * hitpos, collision_info_struct *debris_hit_info=NULL );
void debris_hit( object * debris_obj, object * other_obj, vec3d * hitpos, float damage );
int debris_get_team(object *objp);
void debris_clear_expired_flag(debris *db);

#endif // _DEBRIS_H
