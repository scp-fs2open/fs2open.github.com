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
 * $Revision: 2.12.2.2 $
 * $Date: 2007-05-28 18:27:36 $
 * $Author: wmcoolmon $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.12.2.1  2007/02/12 00:45:24  taylor
 * bit of cleanup and minor performance tweaks
 * sync up with new generic_anim/bitmap and weapon delayed loading changes
 * with generic_anim, use Goober's animation timing for beam section and glow animations
 * make trail render list dynamic (as well as it can be)
 *
 * Revision 2.12  2006/02/25 21:47:19  Goober5000
 * spelling
 *
 * Revision 2.11  2005/12/14 08:07:33  phreak
 * Handling of exit explosions for beams when a ship is about to get destroyed, or according to the code: "tooled".
 * Bumped beam frame collisions to 10 to compensate.
 *
 * Revision 2.10  2005/07/13 03:35:30  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.9  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.8  2004/08/11 05:06:36  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.7  2004/03/17 04:07:32  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.6  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.5  2003/11/11 02:15:41  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.4  2002/11/14 04:18:17  bobboau
 * added warp model and type 1 glow points
 * and well as made the new glow file type,
 * some general improvement to fighter beams,
 *
 * Revision 2.3  2002/10/19 19:29:29  bobboau
 * initial commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.2  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/25 04:50:48  wmcoolmon
 * Added Bobboau's fighter-beam code.
 *
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

// ------------------------------------------------------------------------------------------------
// BEAM WEAPON DEFINES/VARS
//
#include "model/model.h"

// prototypes
struct object;
struct ship_subsys;
struct obj_pair;
struct beam_weapon_info;
struct vec3d;

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
#define MAX_BEAMS					500

// uses to define beam behavior ahead of time - needed for multiplayer
typedef struct beam_info {
	vec3d			dir_a, dir_b;						// direction vectors for beams	
	float				delta_ang;							// angle between dir_a and dir_b
	ubyte				shot_count;							// # of shots	
	float				shot_aim[MAX_BEAM_SHOTS];		// accuracy. this is a constant multiple of radius. anything < 1.0 will guarantee a hit	
} beam_info;

// pass to beam fire 
typedef struct beam_fire_info {
	int				beam_info_index;				// weapon info index 
	object			*shooter;						// whos shooting
	vec3d			targeting_laser_offset;		// offset from the center of the object (for targeting lasers only)
	ship_subsys		*turret;							// where he's shooting from
	float				accuracy;						// 0.0 to 1.0 (only really effects targeting on small ships)
	object			*target;							// whos getting shot
	ship_subsys		*target_subsys;				// (optional), specific subsystem to be targeted on the target 
	beam_info		*beam_info_override;			// (optional), pass this in to override all beam movement info (for multiplayer)
	int				num_shots;						// (optional), only used for type D weapons
	bool fighter_beam;
	int bank;		//fighter beams, wich bank of the primary weapons are they in
} beam_fire_info;

typedef struct fighter_beam_fire_info {
	int				beam_info_index;				// weapon info index 
	object			*shooter;						// whos shooting
	vec3d			targeting_laser_offset;		// offset from the center of the object (for targeting lasers only)
	ship_subsys		*turret;							// where he's shooting from
	float				accuracy;						// 0.0 to 1.0 (only really effects targeting on small ships)
	object			*target;							// whos getting shot
	ship_subsys		*target_subsys;				// (optional), specific subsystem to be targeted on the target 
	beam_info		*beam_info_override;			// (optional), pass this in to override all beam movement info (for multiplayer)
	int				num_shots;						// (optional), only used for type D weapons
	int fighter_beam_loop_sound;			//loop sound used by fighter beams -Bobboau
	int warmup_stamp;
	int warmdown_stamp;
	float life_left;	
	float life_total;
} fighter_beam_fire_info;

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
int beam_fire_targeting(fighter_beam_fire_info *fire_info);

// return an object index of the guy who's firing this beam
int beam_get_parent(object *bm);

// return weapon_info_index of beam
int beam_get_weapon_info_index(object *bm);

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

void beam_calc_facing_pts(vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w, float z_add);

// debug code
void beam_test(int whee);
void beam_test_new(int whee);

#endif
