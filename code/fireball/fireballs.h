/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Fireball/FireBalls.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:22 $
 * $Author: penguin $
 *
 * Prototypes for fireball functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 8     9/13/99 10:09a Andsager
 * Add debug console commands to lower model render detail and fireball
 * LOD for big ship explosiosns.
 * 
 * 7     9/06/99 6:57p Jamesa
 * Cranked down num large fireball explosions.
 * 
 * 6     9/06/99 6:14p Jamesa
 * Reduced max fireball types.
 * 
 * 5     8/31/99 10:13p Andsager
 * Add Knossos warp effect fireball
 * 
 * 4     4/28/99 11:13p Dave
 * Temporary checkin of artillery code.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 34    5/15/98 3:54p John
 * Added code so that only "perishable" fireballs get removed.
 * 
 * 33    4/15/98 9:42a Adam
 * added 2 more explosion types (1, actually, but placeholder for 2)
 * 
 * 32    4/12/98 9:56a John
 * Made lighting detail flags work.   Made explosions cast light on
 * highest.
 * 
 * 31    3/30/98 4:02p John
 * Made machines with < 32 MB of RAM use every other frame of certain
 * bitmaps.   Put in code to keep track of how much RAM we've malloc'd.
 * 
 * 30    3/18/98 12:36p John
 * Made hardware have nicer looking warp effect
 * 
 * 29    2/26/98 10:07p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 28    2/19/98 4:32p Lawrance
 * Add asteroid explosion
 * 
 * 27    1/23/98 5:06p John
 * Took L out of vertex structure used B (blue) instead.   Took all small
 * fireballs out of fireball types and used particles instead.  Fixed some
 * debris explosion things.  Restructured fireball code.   Restructured
 * some lighting code.   Made dynamic lighting on by default. Made groups
 * of lasers only cast one light.  Made fireballs not cast light.
 * 
 * 26    1/15/98 4:58p John
 * Made warp effect use a looping ani.  Made the scaling up & down be in
 * software.
 * 
 * 25    1/15/98 2:32p John
 * Added code to optionally take a velocity for a fireball.
 * 
 * 24    1/02/98 5:04p John
 * Several explosion related changes.  Made fireballs not be used as
 * ani's.  Made ship spark system expell particles.  Took away impact
 * explosion for weapon hitting ship... this needs to get added to weapon
 * info and makes shield hit more obvious.  Only make sparks when hit
 * hull, not shields.
 * 
 * 23    12/08/97 11:15a John
 * added parameter to warpout for life.
 * 
 * 22    11/01/97 1:49p John
 * added code to page fireballs in during level load.  Made player warpout
 * use Adam's new camera movement pattern.  Make ships docked to warping
 * out ships render correctly.
 * 
 * 21    10/24/97 6:24p John
 * added code to return the life left of a fireball
 * 
 * 20    9/14/97 4:49p Lawrance
 * extern Num_fireballs
 * 
 * 19    9/12/97 4:02p John
 * put in ship warp out effect.
 * put in dynamic lighting for warp in/out
 * 
 * 18    9/09/97 4:49p John
 * Almost done ship warp in code
 * 
 * 17    9/03/97 4:32p John
 * changed bmpman to only accept ani and pcx's.  made passing .pcx or .ani
 * to bm_load functions not needed.   Made bmpman keep track of palettes
 * for bitmaps not mapped into game palettes.
 * 
 * 16    8/29/97 2:26p John
 * first rev of ship warp in effect.  Nothing more than a fireball inside
 * of freespace, but textest.cpp contains the correct effect code that
 * needs to be transferred into the game next.
 * 
 * 15    8/13/97 9:50p Allender
 * split *_move into *_process_pre and *_process_post functions.
 * process_pre functions called before object is moved.  _process_post
 * functions called after object is moved.  Reordered code in ship_post
 * and weapon_post for multiplayer
 * 
 * 14    8/13/97 12:06p Lawrance
 * supporting multiple types of fireball explosions
 * 
 * 13    7/21/97 11:41a Lawrance
 * clean up fireballs at the end of each level
 * 
 * 12    7/11/97 11:19a Lawrance
 * fix mem leaks, move save code from State.cpp here
 * 
 * 11    7/10/97 1:51p Lawrance
 * sorting anim fireballs
 * 
 * 10    5/14/97 4:08p Lawrance
 * removing my_index from game arrays
 * 
 * 9     3/11/97 10:47p Mike
 * Add a slew of secondary weapons.
 * Support exhaust blobs.
 * Add weapons that spawn weapons.
 * Add remotely detonatable weapons.
 * Add heat-seeking missiles.
 * 
 * 8     2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 *
 * $NoKeywords: $
 */

#ifndef _FIREBALLS_H
#define _FIREBALLS_H

#include "object.h"
#include "cfile.h"

// these values correspond to the fireball.tbl entries
#define FIREBALL_EXPLOSION_MEDIUM	0				// Used for the 4 little explosions before a ship explodes
#define FIREBALL_WARP_EFFECT			1				// Used for the warp in / warp out effect
#define FIREBALL_KNOSSOS_EFFECT		2				// Used for the KNOSSOS warp in / warp out effect
#define FIREBALL_ASTEROID				3
#define FIREBALL_EXPLOSION_LARGE1	4				// Used for the big explosion when a ship breaks into pieces
#define FIREBALL_EXPLOSION_LARGE2	5				// Used for the big explosion when a ship breaks into pieces
// #define FIREBALL_EXPLOSION_LARGE3	6				// Used for the big explosion when a ship breaks into pieces
#define MAX_FIREBALL_TYPES				6				// How many types there are

#define FIREBALL_NUM_LARGE_EXPLOSIONS 2

void fireball_init();
void fireball_render(object * obj);
void fireball_delete( object * obj );
void fireball_process_pre(object * obj, float frame_time);
void fireball_process_post(object * obj, float frame_time);

// reversed is for warp_in/out effects
// Velocity: If not NULL, the fireball will move at a constant velocity.
// warp_lifetime: If warp_lifetime > 0.0f then makes the explosion loop so it lasts this long.  Only works for warp effect
int fireball_create(vector *pos, int fireball_type, int parent_obj, float size, int reversed=0, vector *velocity=NULL, float warp_lifetime=0.0f, int ship_class=-1, matrix *orient=NULL, int low_res=0); 
void fireball_render_plane(int plane);
void fireball_close();
void fireball_level_close();
void fireball_preload();		// page in warpout effect data

// Returns 1 if you can remove this fireball
int fireball_is_perishable(object * obj);

// Returns 1 if this fireball is a warp 
int fireball_is_warp(object * obj);

// Returns life left of a fireball in seconds
float fireball_lifeleft( object *obj );

// Returns life left of a fireball in percent
float fireball_lifeleft_percent( object *obj );

// internal function to draw warp grid.
extern void warpin_render(matrix *orient, vector *pos, int texture_bitmap_num, float radius, float life_percent, float max_radius );
extern int Warp_glow_bitmap;			// Internal

#endif /* _FIREBALLS_H */
