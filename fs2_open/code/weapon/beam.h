/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/Beam.h $
 * $Revision: 2.1 $
 * $Date: 2002-07-25 04:50:48 $
 * $Author: wmcoolmon $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 21    9/08/99 10:29p Dave
 * Make beam sound pausing and unpausing much safer.
 * 
 * 20    6/29/99 7:39p Dave
 * Lots of small bug fixes.
 * 
 * 19    6/21/99 7:25p Dave
 * netplayer pain packet. Added type E unmoving beams.
 * 
 * 18    6/18/99 5:16p Dave
 * Added real beam weapon lighting. Fixed beam weapon sounds. Added MOTD
 * dialog to PXO screen.
 * 
 * 17    6/04/99 2:16p Dave
 * Put in shrink effect for beam weapons.
 * 
 * 16    5/14/99 11:47a Andsager
 * Added  beam_get_weapon_info_index(object *bm)
 * 
 * 15    5/08/99 8:25p Dave
 * Upped object pairs. First run of nebula lightning.
 * 
 * 14    4/22/99 11:06p Dave
 * Final pass at beam weapons. Solidified a lot of stuff. All that remains
 * now is to tweak and fix bugs as they come up. No new beam weapon
 * features.
 * 
 * 13    4/21/99 6:15p Dave
 * Did some serious housecleaning in the beam code. Made it ready to go
 * for anti-fighter "pulse" weapons. Fixed collision pair creation. Added
 * a handy macro for recalculating collision pairs for a given object.
 * 
 * 12    4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 11    4/19/99 11:01p Dave
 * More sophisticated targeting laser support. Temporary checkin.
 * 
 * 10    4/16/99 5:54p Dave
 * Support for on/off style "stream" weapons. Real early support for
 * target-painting lasers.
 * 
 * 9     3/08/99 7:03p Dave
 * First run of new object update system. Looks very promising.
 * 
 * 8     3/04/99 6:09p Dave
 * Added in sexpressions for firing beams and checking for if a ship is
 * tagged.
 * 
 * 7     1/30/99 1:29a Dave
 * Fixed nebula thumbnail problem. Full support for 1024x768 choose pilot
 * screen.  Fixed beam weapon death messages.
 * 
 * 6     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 5     1/21/99 10:45a Dave
 * More beam weapon stuff. Put in warmdown time.
 * 
 * 4     1/14/99 12:48a Dave
 * Todo list bug fixes. Made a pass at putting briefing icons back into
 * FRED. Sort of works :(
 * 
 * 3     1/12/99 12:53a Dave
 * More work on beam weapons - made collision detection very efficient -
 * collide against all object types properly - made 3 movement types
 * smooth. Put in test code to check for possible non-darkening pixels on
 * object textures.
 * 
 * 2     1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 
 * $NoKeywords: $
 */

#ifndef __FS2_BEAM_WEAPON_HEADER_FILE
#define __FS2_BEAM_WEAPON_HEADER_FILE

#include "model.h"

// ------------------------------------------------------------------------------------------------
// BEAM WEAPON DEFINES/VARS
//

// prototypes
struct object;
struct ship_subsys;
struct obj_pair;
struct beam_weapon_info;
struct vector;

// beam types
// REMINDER : if you change the behavior of any of these beam types, make sure to update their "cones" of possible
// movement inside of the function beam_get_cone_dot(...) in beam.cpp  Otherwise it could cause collisions to not
// function properly!!!!!!
#define BEAM_TYPE_A					0				// unidirectional beam
#define BEAM_TYPE_B					1				// "slash" in one direction
#define BEAM_TYPE_C					2				// targeting lasers (only lasts one frame)
#define BEAM_TYPE_D					3				// similar to the type A beams, but takes multiple shots and "chases" fighters around
#define BEAM_TYPE_E					4				// stupid beam. like type A, only it doesn't aim. it just shoots directly out of the turret

// max # of "shots" an individual beam will take
#define MAX_BEAM_SHOTS				5

// uses to define beam behavior ahead of time - needed for multiplayer
typedef struct beam_info {
	vector			dir_a, dir_b;						// direction vectors for beams	
	float				delta_ang;							// angle between dir_a and dir_b
	ubyte				shot_count;							// # of shots	
	float				shot_aim[MAX_BEAM_SHOTS];		// accuracy. this is a constant multiple of radius. anything < 1.0 will guarantee a hit	
} beam_info;

// pass to beam fire 
typedef struct beam_fire_info {
	int				beam_info_index;				// weapon info index 
	object			*shooter;						// whos shooting
	vector			targeting_laser_offset;		// offset from the center of the object (for targeting lasers only)
	ship_subsys		*turret;							// where he's shooting from
	float				accuracy;						// 0.0 to 1.0 (only really effects targeting on small ships)
	object			*target;							// whos getting shot
	ship_subsys		*target_subsys;				// (optional), specific subsystem to be targeted on the target 
	beam_info		*beam_info_override;			// (optional), pass this in to override all beam movement info (for multiplayer)
	int				num_shots;						// (optional), only used for type D weapons
} beam_fire_info;

// collision info
typedef struct beam_collision {
	mc_info			cinfo;							// collision info
	int				c_objnum;						// objnum of the guy we recently collided with
	int				c_sig;							// object sig
	int				c_stamp;							// when we should next apply damage	
	int				quadrant;						// sheild quadrant this beam hits if any -Bobboau
} beam_collision;

// beam lighting effects
extern int Beam_lighting;

// ------------------------------------------------------------------------------------------------
// BEAM WEAPON FUNCTIONS
//

// ---------------
// the next functions are probably the only ones anyone should care about calling. the rest require somewhat detailed knowledge of beam weapons

// fire a beam, returns objnum on success. the innards of the code handle all the rest, foo
int beam_fire(beam_fire_info *fire_info);

// fire a targeting beam, returns objnum on success. a much much simplified version of a beam weapon
// targeting lasers last _one_ frame. For a continuous stream - they must be created every frame.
// this allows it to work smoothly in multiplayer (detect "trigger down". every frame just create a targeting laser firing straight out of the
// object. this way you get all the advantages of nice rendering and collisions).
// NOTE : only references beam_info_index and shooter
int beam_fire_targeting(beam_fire_info *fire_info);

// return an object index of the guy who's firing this beam
int beam_get_parent(object *bm);

// return weapon_info_index of beam
int beam_get_weapon_info_index(object *bm);

// render the beam itself
void beam_render(beam_weapon_info *bwi, vector *start, vector *shot, float shrink = 1.0f);

// given a beam object, get the # of collisions which happened during the last collision check (typically, last frame)
int beam_get_num_collisions(int objnum);

// stuff collision info, returns 1 on success
int beam_get_collision(int objnum, int num, int *collision_objnum, mc_info **cinfo);
// ---------------

// init at game startup
void beam_init();

// initialize beam weapons for this level
void beam_level_init();

// shutdown beam weapons for this level
void beam_level_close();

// collide a beam with a ship, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_ship(obj_pair *pair);

// collide a beam with an asteroid, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_asteroid(obj_pair *pair);

// collide a beam with a missile, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_missile(obj_pair *pair);

// collide a beam with debris, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_debris(obj_pair *pair);

// pre-move (before collision checking - but AFTER ALL OTHER OBJECTS HAVE BEEN MOVED)
void beam_move_all_pre();

// post-collision time processing for beams
void beam_move_all_post();

// render all beam weapons
void beam_render_all();

// early-out function for when adding object collision pairs, return 1 if the pair should be ignored
int beam_collide_early_out(object *a, object *b);

// pause all looping beam sounds
void beam_pause_sounds();

// unpause looping beam sounds
void beam_unpause_sounds();

// debug code
void beam_test(int whee);
void beam_test_new(int whee);

#endif
