/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Physics/Physics.cpp $
 * $Revision: 2.4 $
 * $Date: 2003-09-11 19:22:52 $
 * $Author: argv $
 *
 * Physics stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2003/08/06 17:39:49  phreak
 * since the afterburners are routed through the physics engine, i needed to route the boost pod through here too
 *
 * Revision 2.2  2002/10/19 19:29:28  bobboau
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam sheild hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:09  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     8/13/99 10:49a Andsager
 * Knossos and HUGE ship warp out.  HUGE ship warp in.  Stealth search
 * modes dont collide big ships.
 * 
 * 4     7/03/99 5:50p Dave
 * Make rotated bitmaps draw properly in padlock views.
 * 
 * 3     5/11/99 10:16p Andsager
 * First pass on engine wash effect.  Rotation (control input), damage,
 * shake.  
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 125   8/26/98 4:00p Johnson
 * Removed max velocity assert so the new huge cap ship can warp in
 * safely.
 * 
 * 124   5/18/98 5:01p Comet
 * don't do displacement checks in multiplayer
 * 
 * 123   4/06/98 2:38p Duncan
 * AL: Fix potential negative sqrt due to numerical inaccuracy
 * 
 * 122   3/25/98 1:30p Andsager
 * comment out physics assert
 * 
 * 121   3/23/98 9:48a Andsager
 * comment out printf
 * 
 * 120   3/22/98 4:11p Andsager
 * Remove global Freespace_running
 * 
 * 119   3/20/98 5:15p Andsager
 * Revised calculation of rotation, shake, and velocity in response to
 * shockwave since capital ships were seen  to shake.
 * 
 * 118   3/19/98 1:06p Andsager
 * Fix bug checking excessive velocity with accelerated time.
 * 
 * 117   3/17/98 5:43p Andsager
 * Modify physics checks for very slow frame rates.
 * 
 * 116   3/17/98 9:54a Andsager
 * Temporary take out Int3()s in velocity checks
 * 
 * 115   3/17/98 9:51a Allender
 * temporarily commented out Int3 that was causing pain
 * 
 * 114   3/16/98 6:07p Johnson
 * DA:  check that ship velocity does not get too high from collisons
 * 
 * 113   3/16/98 4:39p Adam
 * change Dave's asserts so that displacement checks don't get used when
 * not "in mission"
 * 
 * 112   3/16/98 1:11p Andsager
 * Turn on velocity, displacement limit checks
 * 
 * 111   3/12/98 5:21p Andsager
 * Optimize physics for lasers and dumbfire missiles
 * 
 * 110   3/09/98 2:10p Andsager
 * Put in checks for debris (other) with excessive velocity.
 * 
 * 109   3/09/98 12:59p Mike
 * Put error checking in physics code to detect NANs.
 * 
 * 108   3/09/98 12:13a Andsager
 * Add code to help find jumps in position.
 * 
 * 107   3/03/98 10:39a Andsager
 * Fixed erroneous mprintf for log term in physics_apply_shock
 * 
 * 106   2/11/98 4:52p Mike
 * Better attacking by ships.
 * Fix stupidity in AI classes, which were in backward order!
 * 
 * 105   2/03/98 6:01p Andsager
 * Fix excessive rotvel in debris_create.  Check using physics function
 * check_rotvel_limit.
 * 
 * 104   2/03/98 10:45a Mike
 * Comment out mprintf that could occur very often.
 * 
 * 103   1/29/98 2:38p Andsager
 * Fix bug in physics_apply_shock so that large ships have a smaller
 * effect from shockwaves.
 * 
 * 102   1/23/98 11:31a Andsager
 * Added I_body_inv to phys_init.  Needed for shockwaves hitting ships in
 * descent style physics.
 * 
 * 101   1/23/98 9:02a John
 * Took out Dave's debugging Int3 since they aren't finding what he
 * thought they would and they broke Testcode and pofview.
 * 
 * 100   1/22/98 5:10p Mike
 * Fix bug with player's damp getting stuck off.
 * 
 * 99    1/20/98 3:13p Allender
 * check for Player_obj being valid before doing some other andsager
 * sanity check in physics_sim_vel
 * 
 * 98    1/20/98 10:08a Andsager
 * Remove uninitialized viariable warnings.
 * 
 * 97    1/19/98 3:46p Allender
 * fixed problem where a previous changed caused optimized builds to
 * break.  Don't do a quick out on velcoity_ramp() anymore
 * 
 * 96    1/19/98 12:00p Dave
 * DA:  Revise instantaneous velocity debug check to work with
 * multiplayer.
 * 
 * 95    1/16/98 3:03p Andsager
 * Fix debug info from int3() on player death.
 * 
 * 94    1/16/98 2:54p Andsager
 * Fix debug info.
 * 
 * 93    1/16/98 2:34p Andsager
 * Added debug code to find instantaneous acceleration.
 * 
 * 92    1/16/98 12:14p Andsager
 * Add error checking for the current brief stage
 * 
 * 91    12/29/97 5:10p Allender
 * fixed problems with speed not being reported properly in multiplayer
 * games.  Made read_flying_controls take frametime as a parameter.  More
 * ship/weapon select stuff
 * 
 * 90    12/29/97 12:58p Johnson
 * don't debug rotational velocity when Fred is running
 * 
 * 89    12/08/97 10:29a Andsager
 * Remove shockwave physics parameters from weapon_area_apply_blast and
 * move into physics
 * 
 * 88    12/05/97 3:31p Andsager
 * Added view shake if hit by a weapon or collisoin.
 * 
 * 87    12/03/97 5:47p Andsager
 * Changed reduced damping following collision or weapon to run off time
 * stamp. 
 * 
 * 86    11/24/97 1:54p Dan
 * Mike: Comment out Assert() in physics, debug_rotvel().
 * 
 * 85    11/24/97 8:46a Andsager
 * Added rotational velocity caps and debug info.
 * 
 * 84    11/20/97 4:01p Mike
 * Prevent divide overflow.
 * 
 * 83    11/20/97 12:34a Mike
 * Make ships coast to a stop when their engines have been destroyed.
 * Tricky because code doesn't use damp in forward dimension.
 * 
 * 82    11/19/97 5:57p Mike
 * Hmm, undid all my changes, except for the crucial removal of a blank
 * between "physics_sim" and the open paren.
 * 
 * 81    11/19/97 1:26a Andsager
 * Made shockwaves work again, including shake.
 * 
 * 80    11/17/97 5:15p Andsager
 * 
 * 79    11/13/97 6:01p Andsager
 * Improve comment in physic_collide_whack
 * 
 * 78    11/13/97 5:41p Andsager
 * Decreased the rotational whack after getting hit by lasers and
 * missiles.
 * 
 * 77    10/29/97 5:01p Andsager
 * fixed bug in collision physics (physics_collide_whack)
 * 
 * 76    9/16/97 5:28p Andsager
 * calculate velocity in physics_sim_vel
 * 
 * 75    9/11/97 5:25p Mike
 * Fix ! vs. & precedence bug in physics_sim_vel().
 * 
 * 74    9/09/97 10:14p Andsager
 * 
 * 73    9/04/97 5:09p Andsager
 * implement physics using moment of inertia and mass (from BSPgen).
 * Added to phys_info struct.  Updated ship_info, polymodel structs.
 * Updated weapon ($Mass and $Force) and ship ($Mass -> $Density) tables
 * 
 * 72    9/03/97 5:43p Andsager
 * fixed bug calculating ramp velocity after getting whacked
 * 
 * 71    9/02/97 4:19p Mike
 * Comment out code at end of physics_apply_whack() that made ships nearly
 * stop.
 * 
 * 70    8/29/97 10:13a Allender
 * work on server/client prediction code -- doesn't work too well.  Made
 * all clients simulate their own orientation with the server giving
 * corrections every so often.
 * 
 * 69    8/25/97 2:41p Andsager
 * collision code also changes ramp velocity to take account of collison.
 * some optimization of collision physics.
 * 
 * 68    8/19/97 9:56a John
 * new thruster effect fairly functional
 * 
 * 67    8/18/97 6:26p Andsager
 * preliminary version of collision physics
 * 
 * 66    8/17/97 9:19p Andsager
 * improvement to collision physics
 * 
 * 65    8/13/97 12:16p Andsager
 * intermediate level checkin for use with collision with extended objects
 * 
 * 64    8/05/97 3:13p Andsager
 * improved comments to apply_whack
 * 
 * 63    8/05/97 10:18a Lawrance
 * my_rand() being used temporarily instead of rand()
 * 
 * 62    7/31/97 12:44p Andsager
 * 
 * 61    7/25/97 5:05p John
 * fixed a potential bug in ramp_velocity when delta dist becomes 0, a
 * bunch of sideways 8 thingys appear :-)
 * 
 * 
 * 60    7/25/97 1:04p Andsager
 * Modified physics flag PF_REDUCED_DAMP for damping when object is hit.
 * Flag is now set in physics_apply_whack/shock and turned off in
 * physics_sim_vel.  Weapons should not directly set this flag.
 * 
 * 59    7/25/97 9:07a Andsager
 * modified apply_whack
 * 
 * 58    7/23/97 5:10p Andsager
 * Enhanced shockwave effect with shake based on blast force.  
 * 
 * 57    7/22/97 2:40p Andsager
 * shockwaves now cause proper rotation of ships
 * 
 * 56    7/21/97 4:12p Mike
 * Two ships move as one while docked, including physics whacks.  Spin of
 * objects when killed based on speed.
 * 
 * 55    7/21/97 2:19p John
 * made descent-style physics work for testcode. fixed bug in the
 * velocity_ramp code with t==0.0f
 * 
 * 54    7/17/97 8:02p Lawrance
 * improve comments to physics_apply_whack()
 * 
 * 53    7/16/97 4:42p Mike
 * Make afterburner shake viewer, not ship.
 * Shake for limited time.
 * Add timestamp_until() function to timer library.
 * 
 * 52    7/16/97 11:53a Andsager
 * 
 * 51    7/16/97 11:06a Andsager
 * Allow damping on z to support shockwaves
 * 
 * 50    7/15/97 12:28p Andsager
 * commented out some debug code.
 * 
 * 49    7/15/97 12:25p Andsager
 * More integration with new physics.
 * 
 * 48    7/15/97 12:03p Andsager
 * New physics stuff
 * 
 * 47    7/11/97 11:54a John
 * added rotated 3d bitmaps.
 * 
 * 46    6/25/97 12:22p Mike
 * Diminish bashing into of objects.  Make ships flying waypoints fly in
 * formation.
 * 
 * 45    6/17/97 10:59p Mike
 * Comment out irritating mprintf().
 * 
 * 44    6/11/97 4:27p Mike
 * Balance the whack ships take when they get hit.
 * 
 * 43    4/17/97 10:02a Lawrance
 * allow ship shaking for ABURN_DECAY_TIME after afterburners cut out
 * 
 * 42    4/16/97 10:48p Mike
 * Afterburner shake.
 * Made afterburner engagement a bit in physics flags, removed from ship
 * flags, removed a parameter to physics_read_flying_controls().
 * 
 * 41    4/11/97 3:17p Mike
 * Modify physics_sim() to use non quick version of vm_vec_mag().  Quick
 * version was causing cumulative errors in velocity of homing missiles.
 * 
 * 40    4/10/97 3:20p Mike
 * Change hull damage to be like shields.
 * 
 * 39    4/04/97 11:08a Adam
 * played with banking values
 * 
 * 38    4/04/97 12:19a Mike
 * Make ships bank when they turn.
 * 
 * 37    3/04/97 3:10p Mike
 * Intermediate checkin: Had to resolve some build errors.  Working on two
 * docked ships moving as one.
 * 
 * 36    2/25/97 7:39p Lawrance
 * added afterburner_on flag to physics_read_flying_controls() to account
 * for afterburner effects
 * 
 * 35    2/05/97 6:02p Hoffoss
 * Added heading rotation around universal Y axis to Fred when controlling
 * the camera.
 * 
 * 34    2/05/97 9:15a Mike
 * Partial implementation of new docking system, integrated so I could
 * update on John's new turret code.
 * 
 * 33    1/20/97 7:58p John
 * Fixed some link errors with testcode.
 * 
 * $NoKeywords: $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "physics/physics.h"
#include "math/floating.h"
#include "playerman/player.h"
#include "freespace2/freespace.h"
#include "globalincs/linklist.h"
#include "io/timer.h"
#include "io/key.h"

// defines for physics functions
#define	MAX_TURN_LIMIT	0.2618f		// about 15 degrees

#define ROT_DEBUG
#define ROTVEL_TOL		0.1			// Amount of rotvel is decreased if over cap
#define ROTVEL_CAP		14.0			// Rotational velocity cap for live objects
#define DEAD_ROTVEL_CAP	16.3			// Rotational velocity cap for dead objects

#define MAX_SHIP_SPEED		300		// Maximum speed allowed after whack or shockwave
#define RESET_SHIP_SPEED	240		// Speed that a ship is reset to after exceeding MAX_SHIP_SPEED

#define	SW_ROT_FACTOR			5		// increase in rotational time constant in shockwave
#define	SW_BLAST_DURATION		2000	// maximum duration of shockwave
#define	REDUCED_DAMP_FACTOR	10		// increase in side_slip and acceleration time constants (scaled according to reduced damp time)
#define	REDUCED_DAMP_VEL		30		// change in velocity at which reduced_damp_time is 2000 ms
#define	REDUCED_DAMP_TIME		2000	// ms (2.0 sec)
#define	WEAPON_SHAKE_TIME		500	//	ms (0.5 sec)	viewer shake time after hit by weapon (implemented via afterburner shake)
#define	SPECIAL_WARP_T_CONST	0.651	// special warp time constant (loose 99 % of excess speed in 3 sec)

#define	PHYS_DEBUG						// check if (vel > 500) or (displacement in one frame > 350)

void update_reduced_damp_timestamp( physics_info *pi, float impulse );

void physics_init( physics_info * pi )
{
	memset( pi, 0, sizeof(physics_info) );

	pi->mass = 10.0f;					// This ship weighs 10 units
	pi->side_slip_time_const = 0.05f;					
	pi->rotdamp = 0.1f;

	pi->max_vel.xyz.x = 100.0f;		//sideways
	pi->max_vel.xyz.y = 100.0f;		//up/down
	pi->max_vel.xyz.z = 100.0f;		//forward
	pi->max_rear_vel = 100.0f;	//backward -- controlled seperately

	pi->max_rotvel.xyz.x = 2.0f;		//pitch	
	pi->max_rotvel.xyz.y = 1.0f;		//heading
	pi->max_rotvel.xyz.z = 2.0f;		//bank

	pi->prev_ramp_vel.xyz.x = 0.0f;
	pi->prev_ramp_vel.xyz.y = 0.0f;
	pi->prev_ramp_vel.xyz.z = 0.0f;

	pi->desired_vel.xyz.x = 0.0f;
	pi->desired_vel.xyz.y = 0.0f;
	pi->desired_vel.xyz.z = 0.0f;

	pi->slide_accel_time_const=pi->side_slip_time_const;	// slide using max_vel.xyz.x & .xyz.y
	pi->slide_decel_time_const=pi->side_slip_time_const;	// slide using max_vel.xyz.x & .xyz.y

	pi->afterburner_decay = 1;	
	pi->forward_thrust = 0.0f;
	pi->vert_thrust = 0.0f;	//added these two in order to get side and forward thrusters 
	pi->side_thrust = 0.0f;	//to glow broighter when the ship is moveing in the right direction -Bobboau

	pi->flags = 0;

	// default values for moment of inetaia
	vm_vec_make( &pi->I_body_inv.vec.rvec, 1e-5f, 0.0f, 0.0f );
	vm_vec_make( &pi->I_body_inv.vec.uvec, 0.0f, 1e-5f, 0.0f );
	vm_vec_make( &pi->I_body_inv.vec.fvec, 0.0f, 0.0f, 1e-5f );

}


//==========================================================================
// apply_physics - This does correct physics independent of frame rate.
//
// Given:
//    damping = damping factor.  Setting this to zero make the object instantly
//              go to the target velocity.  Increasing it makes the object ramp
//              up or down to the target velocity.
//    desired_vel = the target velocity
//    initial_vel = velocity at last call
//    t = elapsed time since last call
//
// Returns:
//    new_vel = current velocity
//    delta_pos = delta position (framevec)
// You can extend this to 3d by calling it 3 times, once for each x,y,z component.

void apply_physics( float damping, float desired_vel, float initial_vel, float t, float * new_vel, float * delta_pos )
{
	if ( damping < 0.0001f )	{
		if ( delta_pos )
			*delta_pos = desired_vel*t;
		if ( new_vel )
			*new_vel = desired_vel;
	} else {
		float dv, e;
		dv = initial_vel - desired_vel;
		e = (float)exp( -t/damping );
		if ( delta_pos )
			*delta_pos = (1.0f - e)*dv*damping + desired_vel*t;
		if ( new_vel )
			*new_vel = dv*e + desired_vel;
	}
}



float Physics_viewer_bank = 0.0f;
int Physics_viewer_direction = PHYSICS_VIEWER_FRONT;
static physics_info * Viewer_physics_info = NULL;

// If you would like Physics_viewer_bank to be tracked (Which is needed
// for rotating 3d bitmaps) call this and pass it a pointer to the
// viewer's physics info.
void physics_set_viewer( physics_info * p, int dir )
{
	if ( (Viewer_physics_info != p) || (Physics_viewer_direction != dir) )	{
		Viewer_physics_info = p;
		Physics_viewer_bank = 0.0f;
		Physics_viewer_direction = dir;
	}
}



//	-----------------------------------------------------------------------------------------------------------
// add rotational velocity & acceleration



void physics_sim_rot(matrix * orient, physics_info * pi, float sim_time )
{
	angles	tangles;
	vector	new_vel;
	matrix	tmp;
	float		shock_amplitude;
	float		rotdamp;
	float		shock_fraction_time_left;

	Assert(is_valid_matrix(orient));
	Assert(is_valid_vec(&pi->rotvel));
	Assert(is_valid_vec(&pi->desired_rotvel));

	// Handle special case of shockwave
	shock_amplitude = 0.0f;
	if ( pi->flags & PF_IN_SHOCKWAVE ) {
		if ( timestamp_elapsed(pi->shockwave_decay) ) {
			pi->flags &= ~PF_IN_SHOCKWAVE;
			rotdamp = pi->rotdamp;
		} else {
 			shock_fraction_time_left = timestamp_until( pi->shockwave_decay ) / (float) SW_BLAST_DURATION;
			rotdamp = pi->rotdamp + pi->rotdamp * (SW_ROT_FACTOR - 1) * shock_fraction_time_left;
			shock_amplitude = pi->shockwave_shake_amp * shock_fraction_time_left;
		}
	} else {
		rotdamp = pi->rotdamp;
	}

	// Do rotational physics with given damping
	apply_physics( rotdamp, pi->desired_rotvel.xyz.x, pi->rotvel.xyz.x, sim_time, &new_vel.xyz.x, NULL );
	apply_physics( rotdamp, pi->desired_rotvel.xyz.y, pi->rotvel.xyz.y, sim_time, &new_vel.xyz.y, NULL );
	apply_physics( rotdamp, pi->desired_rotvel.xyz.z, pi->rotvel.xyz.z, sim_time, &new_vel.xyz.z, NULL );

	/*
#ifdef ROT_DEBUG
	if (check_rotvel_limit( pi )) {
		nprintf(("Physics", "rotvel reset in physics_sim_rot\n"));
	}
#endif
*/
	Assert(is_valid_vec(&new_vel));

	pi->rotvel = new_vel;

	tangles.p = pi->rotvel.xyz.x*sim_time;
	tangles.h = pi->rotvel.xyz.y*sim_time;
	tangles.b = pi->rotvel.xyz.z*sim_time;

	// If this is the viewer_object, keep track of the
	// changes in banking so that rotated bitmaps look correct.
	// This is used by the g3_draw_rotated_bitmap function.
	if ( pi == Viewer_physics_info )	{
		switch(Physics_viewer_direction){
		case PHYSICS_VIEWER_FRONT:				
			Physics_viewer_bank -= tangles.b;
			break;

		case PHYSICS_VIEWER_UP:
			Physics_viewer_bank -= tangles.h;
			break;

		case PHYSICS_VIEWER_REAR:
			Physics_viewer_bank += tangles.b;
			break;

		case PHYSICS_VIEWER_LEFT:
			Physics_viewer_bank += tangles.p;
			break;

		case PHYSICS_VIEWER_RIGHT:
			Physics_viewer_bank -= tangles.p;
			break;

		default:
			Physics_viewer_bank -= tangles.b;
			break;
		}

		if ( Physics_viewer_bank < 0.0f ){
			Physics_viewer_bank += 2.0f * PI;
		}

		if ( Physics_viewer_bank > 2.0f * PI ){
			Physics_viewer_bank -= 2.0f * PI;
		}
	}

/*	//	Make ship shake due to afterburner.
	if (pi->flags & PF_AFTERBURNER_ON || !timestamp_elapsed(pi->afterburner_decay) ) {
		float	max_speed;

		max_speed = vm_vec_mag_quick(&pi->max_vel);
		tangles.p += (float) (rand()-RAND_MAX/2)/RAND_MAX * pi->speed/max_speed/64.0f;
		tangles.h += (float) (rand()-RAND_MAX/2)/RAND_MAX * pi->speed/max_speed/64.0f;
		if ( pi->flags & PF_AFTERBURNER_ON ) {
			pi->afterburner_decay = timestamp(ABURN_DECAY_TIME);
		}
	}
*/

	// Make ship shake due to shockwave, decreasing in amplitude at the end of the shockwave
	if ( pi->flags & PF_IN_SHOCKWAVE ) {
		tangles.p += (float) (myrand()-RAND_MAX/2)/RAND_MAX * shock_amplitude;
		tangles.h += (float) (myrand()-RAND_MAX/2)/RAND_MAX * shock_amplitude;
	}


	vm_angles_2_matrix(&pi->last_rotmat, &tangles );
	vm_matrix_x_matrix( &tmp, orient, &pi->last_rotmat );
	*orient = tmp;

	vm_orthogonalize_matrix(orient);
		
}

//	-----------------------------------------------------------------------------------------------------------
// add rotational velocity & acceleration

void physics_sim_rot_editor(matrix * orient, physics_info * pi, float sim_time)
{
	angles	tangles;
	vector	new_vel;
	matrix	tmp;
	angles	t1, t2;

	apply_physics( pi->rotdamp, pi->desired_rotvel.xyz.x, pi->rotvel.xyz.x, sim_time, 
								 &new_vel.xyz.x, NULL );

	apply_physics( pi->rotdamp, pi->desired_rotvel.xyz.y, pi->rotvel.xyz.y, sim_time, 
								 &new_vel.xyz.y, NULL );

	apply_physics( pi->rotdamp, pi->desired_rotvel.xyz.z, pi->rotvel.xyz.z, sim_time, 
								 &new_vel.xyz.z, NULL );
	
	pi->rotvel = new_vel;

	tangles.p = pi->rotvel.xyz.x*sim_time;
	tangles.h = pi->rotvel.xyz.y*sim_time;
	tangles.b = pi->rotvel.xyz.z*sim_time;

	t1 = t2 = tangles;
	t1.h = 0.0f;  t1.b = 0.0f;
	t2.p = 0.0f; t2.b = 0.0f;

	// put in p & b like normal
	vm_angles_2_matrix(&pi->last_rotmat, &t1 );
	vm_matrix_x_matrix( &tmp, orient, &pi->last_rotmat );
	
	// Put in heading separately
	vm_angles_2_matrix(&pi->last_rotmat, &t2 );
	vm_matrix_x_matrix( orient, &pi->last_rotmat, &tmp );

	vm_orthogonalize_matrix(orient);
		
}

// Adds velocity to position
// finds velocity and displacement in local coords
void physics_sim_vel(vector * position, physics_info * pi, float sim_time, matrix *orient)
{
	vector local_disp;		// displacement in this frame
	vector local_v_in;		// velocity in local coords at the start of this frame
	vector local_desired_vel;	// desired velocity in local coords
	vector local_v_out;		// velocity in local coords following this frame
	vector damp;

	//	Maybe clear the reduced_damp flag.
	//	This fixes the problem of the player getting near-instantaneous acceleration under unknown circumstances.
	//	The larger problem is probably that PF_USE_VEL is getting stuck set.
	if ((pi->flags & PF_REDUCED_DAMP) && (timestamp_elapsed(pi->reduced_damp_decay))) {
		pi->flags &= ~PF_REDUCED_DAMP;
	}

	// Set up damping constants based on special conditions
	// ie. shockwave, collision, weapon, dead
	if (pi->flags & PF_DEAD_DAMP) {
		// side_slip_time_const is already quite large and now needs to be applied in all directions
		vm_vec_make( &damp, pi->side_slip_time_const, pi->side_slip_time_const, pi->side_slip_time_const );

	} else if (pi->flags & PF_REDUCED_DAMP) {
		// case of shock, weapon, collide, etc.
		if ( timestamp_elapsed(pi->reduced_damp_decay) ) {
			vm_vec_make( &damp, pi->side_slip_time_const, pi->side_slip_time_const, 0.0f );
		} else {
			// damp is multiplied by fraction and not fraction^2, gives better collision separation
			float reduced_damp_fraction_time_left = timestamp_until( pi->reduced_damp_decay ) / (float) REDUCED_DAMP_TIME;
			damp.xyz.x = pi->side_slip_time_const * ( 1 + (REDUCED_DAMP_FACTOR-1) * reduced_damp_fraction_time_left );
			damp.xyz.y = pi->side_slip_time_const * ( 1 + (REDUCED_DAMP_FACTOR-1) * reduced_damp_fraction_time_left );
			damp.xyz.z = pi->side_slip_time_const * reduced_damp_fraction_time_left * REDUCED_DAMP_FACTOR;
		}
	} else {
		// regular damping
		vm_vec_make( &damp, pi->side_slip_time_const, pi->side_slip_time_const, 0.0f );
	}

	// Note: CANNOT maintain a *local velocity* since a rotation can occur in this frame. 
	// thus the local velocity of in the next frame can be different (this would require rotate to change local vel
	// and this is not desired

	// get local components of current velocity
	vm_vec_rotate (&local_v_in, &pi->vel, orient);

	// get local components of desired velocity
	vm_vec_rotate (&local_desired_vel, &pi->desired_vel, orient);

	// find updated LOCAL velocity and position in the local x direction
	apply_physics (damp.xyz.x, local_desired_vel.xyz.x, local_v_in.xyz.x, sim_time, &local_v_out.xyz.x, &local_disp.xyz.x);

	// find updated LOCAL velocity and position in the local y direction
	apply_physics (damp.xyz.y, local_desired_vel.xyz.y, local_v_in.xyz.y, sim_time, &local_v_out.xyz.y, &local_disp.xyz.y);

	// find updated LOCAL velocity and position in the local z direction
	// for player ship, damp should normally be zero, but may be altered in a shockwave
	//  in death, shockwave,etc. we want damping time const large for all 3 axes
	// warp in test - make excessive speed drop exponentially from max allowed
	// become (0.01x in 3 sec)

	int special_warp_in = FALSE;
	float excess = local_v_in.xyz.z - pi->max_vel.xyz.z;
	if (excess > 5 && (pi->flags & PF_SPECIAL_WARP_IN)) {
		special_warp_in = TRUE;
		float exp_factor = float(exp(-sim_time / SPECIAL_WARP_T_CONST));
		local_v_out.xyz.z = pi->max_vel.xyz.z + excess * exp_factor;
		local_disp.xyz.z = (pi->max_vel.xyz.z * sim_time) + excess * (float(SPECIAL_WARP_T_CONST) * (1.0f - exp_factor));
	} else if (pi->flags & PF_SPECIAL_WARP_OUT) {
		float exp_factor = float(exp(-sim_time / SPECIAL_WARP_T_CONST));
		vector temp;
		vm_vec_rotate(&temp, &pi->prev_ramp_vel, orient);
		float deficeit = temp.xyz.z - local_v_in.xyz.z;
		local_v_out.xyz.z = local_v_in.xyz.z + deficeit * (1.0f - exp_factor);
		local_disp.xyz.z = (local_v_in.xyz.z * sim_time) + deficeit * (sim_time - (float(SPECIAL_WARP_T_CONST) * (1.0f - exp_factor)));
	} else {
		apply_physics (damp.xyz.z, local_desired_vel.xyz.z, local_v_in.xyz.z, sim_time, &local_v_out.xyz.z, &local_disp.xyz.z);
	}

	// maybe turn off special warp in flag
	if ((pi->flags & PF_SPECIAL_WARP_IN) && (excess < 5)) {
		pi->flags &= ~(PF_SPECIAL_WARP_IN);
	}

	// update world position from local to world coords using orient
	vector world_disp;
	vm_vec_unrotate (&world_disp, &local_disp, orient);
#ifdef PHYS_DEBUG
	// check for  excess velocity or translation
	// GET DaveA.
	if ( (Game_mode & GM_IN_MISSION) && (Game_mode & GM_NORMAL) ) {
		// Assert( (sim_time > 0.5f) || (vm_vec_mag_squared(&pi->vel) < 500*500) );
		// Assert( (sim_time > 0.5f) || (vm_vec_mag_squared(&world_disp) < 350*350) );
	}
#endif
	vm_vec_add2 (position, &world_disp);

	// update world velocity
	vm_vec_unrotate(&pi->vel, &local_v_out, orient);

	if (special_warp_in) {
		vm_vec_rotate(&pi->prev_ramp_vel, &pi->vel, orient);
	}
}

//	-----------------------------------------------------------------------------------------------------------
// Simulate a physics object for this frame
void physics_sim(vector* position, matrix* orient, physics_info* pi, float sim_time)
{
	// check flag which tells us whether or not to do velocity translation
	if (pi->flags & PF_CONST_VEL) {
		vm_vec_scale_add2(position, &pi->vel, sim_time);
	} else {
		physics_sim_vel(position, pi, sim_time, orient);

		physics_sim_rot(orient, pi, sim_time);

		pi->speed = vm_vec_mag(&pi->vel);							//	Note, cannot use quick version, causes cumulative error, increasing speed.
		pi->fspeed = vm_vec_dot(&orient->vec.fvec, &pi->vel);		// instead of vector magnitude -- use only forward vector since we are only interested in forward velocity
	}

}

//	-----------------------------------------------------------------------------------------------------------
// Simulate a physics object for this frame.  Used by the editor.  The difference between
// this function and physics_sim() is that this one uses a heading change to rotate around
// the universal Y axis, rather than the local orientation's Y axis.  Banking is also ignored.
void physics_sim_editor(vector *position, matrix * orient, physics_info * pi, float sim_time )
{
	physics_sim_vel(position, pi, sim_time, orient);
	physics_sim_rot_editor(orient, pi, sim_time);
	pi->speed = vm_vec_mag_quick(&pi->vel);
	pi->fspeed = vm_vec_dot(&orient->vec.fvec, &pi->vel);		// instead of vector magnitude -- use only forward vector since we are only interested in forward velocity
}

// function to predict an object's position given the delta time and an objects physics info
void physics_predict_pos(physics_info *pi, float delta_time, vector *predicted_pos)
{
	apply_physics( pi->side_slip_time_const, pi->desired_vel.xyz.x, pi->vel.xyz.x, delta_time, 
								 NULL, &predicted_pos->xyz.x );

	apply_physics( pi->side_slip_time_const, pi->desired_vel.xyz.y, pi->vel.xyz.y, delta_time, 
								 NULL, &predicted_pos->xyz.y );

	apply_physics( pi->side_slip_time_const, pi->desired_vel.xyz.z, pi->vel.xyz.z, delta_time, 
								 NULL, &predicted_pos->xyz.z );
}

// function to predict an object's velocity given the parameters
void physics_predict_vel(physics_info *pi, float delta_time, vector *predicted_vel)
{
	if (pi->flags & PF_CONST_VEL) {
		predicted_vel = &pi->vel;
	} else {
		apply_physics( pi->side_slip_time_const, pi->desired_vel.xyz.x, pi->vel.xyz.x, delta_time, 
									 &predicted_vel->xyz.x, NULL );

		apply_physics( pi->side_slip_time_const, pi->desired_vel.xyz.y, pi->vel.xyz.y, delta_time, 
									 &predicted_vel->xyz.y, NULL );

		apply_physics( pi->side_slip_time_const, pi->desired_vel.xyz.z, pi->vel.xyz.z, delta_time, 
									 &predicted_vel->xyz.z, NULL );
	}
}

// function to predict position and velocity of an object
void physics_predict_pos_and_vel(physics_info *pi, float delta_time, vector *predicted_vel, vector *predicted_pos)
{

	apply_physics( pi->side_slip_time_const, pi->desired_vel.xyz.x, pi->vel.xyz.x, delta_time, 
	                      &predicted_vel->xyz.x, &predicted_pos->xyz.x );

	apply_physics( pi->side_slip_time_const, pi->desired_vel.xyz.y, pi->vel.xyz.y, delta_time, 
	                      &predicted_vel->xyz.y, &predicted_pos->xyz.y );

	apply_physics( pi->side_slip_time_const, pi->desired_vel.xyz.z, pi->vel.xyz.z, delta_time, 
	                      &predicted_vel->xyz.z, &predicted_pos->xyz.z );
}

// physics_read_flying_controls() 
//
// parmeters:  *orient	==>
//					*pi		==>
//					*ci		==> 
//	Adam: Uncomment-out this define to enable banking while turning.
#define	BANK_WHEN_TURN



// function looks at the flying controls and the current velocity to determine a goal velocity
// function determines velocity in object's reference frame and goal velocity in object's reference frame
void physics_read_flying_controls( matrix * orient, physics_info * pi, control_info * ci, float sim_time, vector *wash_rot)
{
	vector goal_vel;		// goal velocity in local coords, *not* accounting for ramping of velcity
	float ramp_time_const;		// time constant for velocity ramping

	float velocity_ramp (float v_in, float v_goal, float time_const, float t);

//	if ( keyd_pressed[KEY_LSHIFT] ) {
//		keyd_pressed[KEY_LSHIFT] = 0;
//		Int3();
//	}

	ci->forward += (ci->forward_cruise_percent / 100.0f);

//	mprintf(("ci->forward == %7.3f\n", ci->forward));

	// give control imput to cause rotation in engine wash
	extern int Wash_on;
	if ( wash_rot && Wash_on ) {
		ci->pitch += wash_rot->xyz.x;
		ci->bank += wash_rot->xyz.z;
		ci->heading += wash_rot->xyz.y;
	}

	//if (ci->vertical && ci->forward)
	//	Int3();

	if (ci->pitch > 1.0f ) ci->pitch = 1.0f;
	else if (ci->pitch < -1.0f ) ci->pitch = -1.0f;

	if (ci->vertical > 1.0f ) ci->vertical = 1.0f;
	else if (ci->vertical < -1.0f ) ci->vertical = -1.0f;

	if (ci->heading > 1.0f ) ci->heading = 1.0f;
	else if (ci->heading < -1.0f ) ci->heading = -1.0f;

	if (ci->sideways > 1.0f  ) ci->sideways = 1.0f;
	else if (ci->sideways < -1.0f  ) ci->sideways = -1.0f;

	if (ci->bank > 1.0f ) ci->bank = 1.0f;
	else if (ci->bank < -1.0f ) ci->bank = -1.0f;

	if ( pi->flags & PF_AFTERBURNER_ON )
		ci->forward = 1.0f;

	if (ci->forward > 1.0f ) ci->forward = 1.0f;
	else if (ci->forward < -1.0f ) ci->forward = -1.0f;

	pi->desired_rotvel.xyz.x = ci->pitch * pi->max_rotvel.xyz.x;
	pi->desired_rotvel.xyz.y = ci->heading * pi->max_rotvel.xyz.y;

	float	delta_bank;

#ifdef BANK_WHEN_TURN
	//	To change direction of bank, negate the whole expression.
	//	To increase magnitude of banking, decrease denominator.
	//	Adam: The following statement is all the math for banking while turning.
	delta_bank = - (ci->heading * pi->max_rotvel.xyz.y)/2.0f;
#else
	delta_bank = 0.0f;
#endif

	pi->desired_rotvel.xyz.z = ci->bank * pi->max_rotvel.xyz.z + delta_bank;
	pi->forward_thrust = ci->forward;
	pi->vert_thrust = ci->vertical;	//added these two in order to get side and forward thrusters 
	pi->side_thrust = ci->sideways;	//to glow broighter when the ship is moveing in the right direction -Bobboau

	if ( pi->flags & PF_AFTERBURNER_ON ) {
		goal_vel.xyz.x = ci->sideways*pi->afterburner_max_vel.xyz.x;
		goal_vel.xyz.y = ci->vertical*pi->afterburner_max_vel.xyz.y;
		goal_vel.xyz.z = ci->forward* pi->afterburner_max_vel.xyz.z;
	}
	else if ( pi->flags & PF_BOOSTER_ON ) {
		goal_vel.xyz.x = ci->sideways*pi->booster_max_vel.xyz.x;
		goal_vel.xyz.y = ci->vertical*pi->booster_max_vel.xyz.y;
		goal_vel.xyz.z = ci->forward* pi->booster_max_vel.xyz.z;
	}
	else {
		goal_vel.xyz.x = ci->sideways*pi->max_vel.xyz.x*pi->max_speed_mul;
		goal_vel.xyz.y = ci->vertical*pi->max_vel.xyz.y*pi->max_speed_mul;
		goal_vel.xyz.z = ci->forward* pi->max_vel.xyz.z;
	}

	if ( goal_vel.xyz.z < (-pi->max_rear_vel) * pi->max_speed_mul) 
		goal_vel.xyz.z = (-pi->max_rear_vel) * pi->max_speed_mul;


	if ( pi->flags & PF_ACCELERATES )	{
		//
		// Determine *resultant* DESIRED VELOCITY (desired_vel) accounting for RAMPING of velocity
		// Use LOCAL coordinates
		// if slide_enabled, ramp velocity for x and y, otherwise set goal (0)
		//    always ramp velocity for z
		// 

		// If reduced damp in effect, then adjust ramp_velocity and desired_velocity can not change as fast.
		// Scale according to reduced_damp_time_expansion.
		float reduced_damp_ramp_time_expansion;
		if ( pi->flags & PF_REDUCED_DAMP ) {
			float reduced_damp_fraction_time_left = timestamp_until( pi->reduced_damp_decay ) / (float) REDUCED_DAMP_TIME;
			reduced_damp_ramp_time_expansion = 1.0f + (REDUCED_DAMP_FACTOR-1) * reduced_damp_fraction_time_left;
		} else {
			reduced_damp_ramp_time_expansion = 1.0f;
		}

//	if ( !use_descent && (Player_obj->phys_info.forward_accel_time_const < 0.1) && !(Ships[Player_obj->instance].flags & SF_DYING) && (Player_obj->type != OBJ_OBSERVER) )
//			Int3();	// Get dave A

		if (pi->flags & PF_SLIDE_ENABLED)  {
			// determine the local velocity
			// deterimine whether accelerating or decleration toward goal for x
			if ( goal_vel.xyz.x > 0.0f )  {
				if ( goal_vel.xyz.x >= pi->prev_ramp_vel.xyz.x ) 
					ramp_time_const = pi->slide_accel_time_const;
				else
					ramp_time_const = pi->slide_decel_time_const;
			} else  {  // goal_vel.xyz.x <= 0.0
				if ( goal_vel.xyz.x <= pi->prev_ramp_vel.xyz.x )
					ramp_time_const = pi->slide_accel_time_const;
				else
					ramp_time_const = pi->slide_decel_time_const;
			}
			// If reduced damp in effect, then adjust ramp_velocity and desired_velocity can not change as fast
			if ( pi->flags & PF_REDUCED_DAMP ) {
				ramp_time_const *= reduced_damp_ramp_time_expansion;
			}
			pi->prev_ramp_vel.xyz.x = velocity_ramp(pi->prev_ramp_vel.xyz.x, goal_vel.xyz.x, ramp_time_const, sim_time);

			// deterimine whether accelerating or decleration toward goal for y
			if ( goal_vel.xyz.y > 0.0f )  {
				if ( goal_vel.xyz.y >= pi->prev_ramp_vel.xyz.y ) 
					ramp_time_const = pi->slide_accel_time_const;
				else
					ramp_time_const = pi->slide_decel_time_const;
			} else  {  // goal_vel.xyz.y <= 0.0
				if ( goal_vel.xyz.y <= pi->prev_ramp_vel.xyz.y )
					ramp_time_const = pi->slide_accel_time_const;
				else
					ramp_time_const = pi->slide_decel_time_const;
			}
			// If reduced damp in effect, then adjust ramp_velocity and desired_velocity can not change as fast
			if ( pi->flags & PF_REDUCED_DAMP ) {
				ramp_time_const *= reduced_damp_ramp_time_expansion;
			}
			pi->prev_ramp_vel.xyz.y = velocity_ramp( pi->prev_ramp_vel.xyz.y, goal_vel.xyz.y, ramp_time_const, sim_time);
		} else  {
			// slide not enabled
			pi->prev_ramp_vel.xyz.x = 0.0f;
			pi->prev_ramp_vel.xyz.y = 0.0f;
		}

		// find ramp velocity in the forward direction
		if ( goal_vel.xyz.z >= pi->prev_ramp_vel.xyz.z )  {
			if ( pi->flags & PF_AFTERBURNER_ON )
				ramp_time_const = pi->afterburner_forward_accel_time_const;
			else if (pi->flags & PF_BOOSTER_ON)
				ramp_time_const = pi->booster_forward_accel_time_const;
			else
				ramp_time_const = pi->forward_accel_time_const;
		} else
			ramp_time_const = pi->forward_decel_time_const;

		// If reduced damp in effect, then adjust ramp_velocity and desired_velocity can not change as fast
		if ( pi->flags & PF_REDUCED_DAMP ) {
			ramp_time_const *= reduced_damp_ramp_time_expansion;
		}
		pi->prev_ramp_vel.xyz.z = velocity_ramp( pi->prev_ramp_vel.xyz.z, goal_vel.xyz.z, ramp_time_const, sim_time);


		// this translates local desired velocities to world velocities

		vm_vec_zero(&pi->desired_vel);
		vm_vec_scale_add2( &pi->desired_vel, &orient->vec.rvec, pi->prev_ramp_vel.xyz.x );
		vm_vec_scale_add2( &pi->desired_vel, &orient->vec.uvec, pi->prev_ramp_vel.xyz.y );
		vm_vec_scale_add2( &pi->desired_vel, &orient->vec.fvec, pi->prev_ramp_vel.xyz.z );
	} else  // object does not accelerate  (PF_ACCELERATES not set)
		pi->desired_vel = pi->vel;
}



//	----------------------------------------------------------------
//	Do *dest = *delta unless:
//				*delta is pretty small
//		and	they are of different signs.
void physics_set_rotvel_and_saturate(float *dest, float delta)
{
	/*
	if ((delta ^ *dest) < 0) {
		if (abs(delta) < F1_0/8) {
			// mprintf((0, "D"));
			*dest = delta/4;
		} else
			// mprintf((0, "d"));
			*dest = delta;
	} else {
		// mprintf((0, "!"));
		*dest = delta;
	}
	*/
	*dest = delta;
}


// ----------------------------------------------------------------------------
// physics_apply_whack applies an instaneous whack on an object changing
// both the objects velocity and the rotational velocity based on the impulse
// being applied.  
//
//	input:	impulse		=>		impulse vector ( force*time = impulse = change in momentum (mv) )
//				pos			=>		vector from center of mass to location of where the force acts
//				pi				=>		pointer to phys_info struct of object getting whacked
//				orient		=>		orientation matrix (needed to set rotational impulse in body coords)
//				mass			=>		mass of the object (may be different from pi.mass if docked)
//
#define WHACK_LIMIT	0.001f
#define ROTVEL_WHACK_CONST 0.12
void physics_apply_whack(vector *impulse, vector *pos, physics_info *pi, matrix *orient, float mass)
{
	vector	local_torque, torque;
//	vector	npos;

	//	Detect null vector.
	if ((fl_abs(impulse->xyz.x) <= WHACK_LIMIT) && (fl_abs(impulse->xyz.y) <= WHACK_LIMIT) && (fl_abs(impulse->xyz.z) <= WHACK_LIMIT))
		return;

	// first do the rotational velocity
	// calculate the torque on the body based on the point on the
	// object that was hit and the momentum being applied to the object

	vm_vec_crossprod(&torque, pos, impulse);
	vm_vec_rotate ( &local_torque, &torque, orient );

	vector delta_rotvel;
	vm_vec_rotate( &delta_rotvel, &local_torque, &pi->I_body_inv );
	vm_vec_scale ( &delta_rotvel, (float) ROTVEL_WHACK_CONST );
	vm_vec_add2( &pi->rotvel, &delta_rotvel );

#ifdef ROT_DEBUG
	if (check_rotvel_limit( pi )) {
		nprintf(("Physics", "rotvel reset in physics_apply_whack\n"));
	}
#endif

	//mprintf(("Whack: %7.3f %7.3f %7.3f\n", pi->rotvel.xyz.x, pi->rotvel.xyz.y, pi->rotvel.xyz.z));

	// instant whack on the velocity
	// reduce damping on all axes
	pi->flags |= PF_REDUCED_DAMP;
	update_reduced_damp_timestamp( pi, vm_vec_mag(impulse) );

	// find time for shake from weapon to end
	int dtime = timestamp_until(pi->afterburner_decay);
	if (dtime < WEAPON_SHAKE_TIME) {
		pi->afterburner_decay = timestamp( WEAPON_SHAKE_TIME );
	}

	vm_vec_scale_add2( &pi->vel, impulse, 1.0f / pi->mass );
	if (!(pi->flags & PF_USE_VEL) && (vm_vec_mag_squared(&pi->vel) > MAX_SHIP_SPEED*MAX_SHIP_SPEED)) {
		// Get DaveA
		nprintf(("Physics", "speed reset in physics_apply_whack [speed: %f]\n", vm_vec_mag(&pi->vel)));
//		Int3();
		vm_vec_normalize(&pi->vel);
		vm_vec_scale(&pi->vel, (float)RESET_SHIP_SPEED);
	}
	vm_vec_rotate( &pi->prev_ramp_vel, &pi->vel, orient );		// set so velocity will ramp starting from current speed
																					// ramped velocity is now affected by collision
}

// function generates a velocity ramp with a given time constant independent of frame rate
// uses an exponential approach to desired velocity and a cheat when close to improve closure speed
float velocity_ramp (float v_in, float v_goal, float ramp_time_const, float t)
{
	float delta_v;
	float decay_factor;
	float dist;

	// JAS: If no time elapsed, change nothing
	if ( t==0.0f )
		return v_in;

	delta_v = v_goal - v_in;
	dist = (float)fl_abs(delta_v);

	// hack to speed up closure when close to goal
	if (dist < ramp_time_const/3)
		ramp_time_const = dist / 3;

	// Rather than get a divide by zero, just go to the goal
	if ( ramp_time_const < 0.0001f )	{
		return v_goal;
	}

	// determine decay factor  (ranges from 0 for short times to 1 for long times)
	// when decay factor is near 0, the velocity in nearly unchanged
	// when decay factor in near 1, the velocity approaches goal
	decay_factor = (float)exp(- t / ramp_time_const);

	return (v_in + delta_v * (1.0f - decay_factor) );
}


// ----------------------------------------------------------------------------
// physics_apply_shock applies applies a shockwave to an object.  This causes a velocity impulse and 
// and a rotational impulse.  This is different than physics_apply_whack since a shock wave is a pressure
// wave which acts over the *surface* of the object, not a point.
//
// inputs:	direction_vec		=>		a position vector whose direction is from the center of the shock wave to the object
//				pressure				=>		the pressure of the shock wave at the object
//				pi						=>		physics_info structure
//				orient				=>		matrix orientation of the object
//				min					=>		vector of minimum values of the bounding box
//				max					=>		vector of maximum values of the bounding box
//				radius				=>		bounding box radius of the object, used for scaling rotation
//
// outputs:	makes changes to physics_info structure rotvel and vel variables
//				
#define	STD_PRESSURE		1000		// amplitude of standard shockwave blasts
#define	MIN_RADIUS			10			// radius within which full rotvel and shake applied
#define	MAX_RADIUS			50			// radius at which no rotvel or shake applied
#define	MAX_ROTVEL			0.4		// max rotational velocity
#define	MAX_SHAKE			0.1		// max rotational amplitude of shake
#define	MAX_VEL				8			// max vel from shockwave 
void physics_apply_shock(vector *direction_vec, float pressure, physics_info *pi, matrix *orient, vector *min, vector *max, float radius)
{
	vector normal;
	vector local_torque, temp_torque, torque;
	vector impact_vec;
	vector area;
	vector sin;

	if (radius > MAX_RADIUS) {
		return;
	}

	vm_vec_normalize_safe ( direction_vec );

	area.xyz.x = (max->xyz.y - min->xyz.z) * (max->xyz.z - min->xyz.z);
	area.xyz.y = (max->xyz.x - min->xyz.x) * (max->xyz.z - min->xyz.z);
	area.xyz.z = (max->xyz.x - min->xyz.x) * (max->xyz.y - min->xyz.y);

	normal.xyz.x = vm_vec_dotprod( direction_vec, &orient->vec.rvec );
	normal.xyz.y = vm_vec_dotprod( direction_vec, &orient->vec.uvec );
	normal.xyz.z = vm_vec_dotprod( direction_vec, &orient->vec.fvec );

	sin.xyz.x = fl_sqrt( fl_abs(1.0f - normal.xyz.x*normal.xyz.x) );
	sin.xyz.y = fl_sqrt( fl_abs(1.0f - normal.xyz.y*normal.xyz.y) );
	sin.xyz.z = fl_sqrt( fl_abs(1.0f - normal.xyz.z*normal.xyz.z) );	

	vm_vec_make( &torque, 0.0f, 0.0f, 0.0f );

	// find the torque exerted due to the shockwave hitting each face
	//  model the effect of the shockwave as if the shockwave were a plane of projectiles,
	//  all moving in the direction direction_vec.  then find the torque as the cross prod
	//  of the force (pressure * area * normal * sin * scale * mass)
	//  normal takes account the fraction of the surface exposed to the shockwave
	//  the sin term is not technically needed but "feels" better
	//  scale factors out the increase in area with larger objects
	//  more massive objects get less rotation

	// find torque due to forces on the right/left face
	if ( normal.xyz.x < 0.0f )		// normal < 0, hits the right face
		vm_vec_copy_scale( &impact_vec, &orient->vec.rvec, max->xyz.x * pressure * area.xyz.x *  normal.xyz.x * sin.xyz.x / pi->mass );
	else								// normal > 0, hits the left face
		vm_vec_copy_scale( &impact_vec, &orient->vec.rvec, min->xyz.x * pressure * area.xyz.x * -normal.xyz.x * sin.xyz.x / pi->mass );

	vm_vec_crossprod( &temp_torque, &impact_vec, direction_vec );
	vm_vec_add2( &torque, &temp_torque );

	// find torque due to forces on the up/down face
	if ( normal.xyz.y < 0.0f )
		vm_vec_copy_scale( &impact_vec, &orient->vec.uvec, max->xyz.y * pressure * area.xyz.y *  normal.xyz.y * sin.xyz.y / pi->mass );
	else
		vm_vec_copy_scale( &impact_vec, &orient->vec.uvec, min->xyz.y * pressure * area.xyz.y * -normal.xyz.y * sin.xyz.y / pi->mass );

	vm_vec_crossprod( &temp_torque, &impact_vec, direction_vec );
	vm_vec_add2( &torque, &temp_torque );

	// find torque due to forces on the forward/backward face
	if ( normal.xyz.z < 0.0f )
		vm_vec_copy_scale( &impact_vec, &orient->vec.fvec, max->xyz.z * pressure * area.xyz.z *  normal.xyz.z * sin.xyz.z / pi->mass );
	else
		vm_vec_copy_scale( &impact_vec, &orient->vec.fvec, min->xyz.z * pressure * area.xyz.z * -normal.xyz.z * sin.xyz.z / pi->mass );

	vm_vec_crossprod( &temp_torque, &impact_vec, direction_vec );
	vm_vec_add2( &torque, &temp_torque );

	// compute delta rotvel, scale according to blast and radius
	float scale;
	vector delta_rotvel;
	vm_vec_rotate( &local_torque, &torque, orient );
	vm_vec_copy_normalize(&delta_rotvel, &local_torque);
	if (radius < MIN_RADIUS) {
		scale = 1.0f;
	} else {
		scale = (MAX_RADIUS - radius)/(MAX_RADIUS-MIN_RADIUS);
	}
	vm_vec_scale(&delta_rotvel, (float)(MAX_ROTVEL*(pressure/STD_PRESSURE)*scale));
	// nprintf(("Physics", "rotvel scale %f\n", (MAX_ROTVEL*(pressure/STD_PRESSURE)*scale)));
	vm_vec_add2(&pi->rotvel, &delta_rotvel);

	// set shockwave shake amplitude, duration, flag
	pi->shockwave_shake_amp = (float)(MAX_SHAKE*(pressure/STD_PRESSURE)*scale);
	pi->shockwave_decay = timestamp( SW_BLAST_DURATION );
	pi->flags |= PF_IN_SHOCKWAVE;

	// set reduced translational damping, set flags
	float velocity_scale = (float)MAX_VEL*scale;
	pi->flags |= PF_REDUCED_DAMP;
	update_reduced_damp_timestamp( pi, velocity_scale*pi->mass );
	vm_vec_scale_add2( &pi->vel, direction_vec, velocity_scale );
	vm_vec_rotate(&pi->prev_ramp_vel, &pi->vel, orient);	// set so velocity will ramp starting from current speed

	// check that kick from shockwave is not too large
	if (!(pi->flags & PF_USE_VEL) && (vm_vec_mag_squared(&pi->vel) > MAX_SHIP_SPEED*MAX_SHIP_SPEED)) {
		// Get DaveA
		nprintf(("Physics", "speed reset in physics_apply_shock [speed: %f]\n", vm_vec_mag(&pi->vel)));
//		Int3();
		vm_vec_normalize(&pi->vel);
		vm_vec_scale(&pi->vel, (float)RESET_SHIP_SPEED);
	}

#ifdef ROT_DEBUG
	if (check_rotvel_limit( pi )) {
		nprintf(("Physics", "rotvel reset in physics_apply_shock\n"));
	}
#endif

																				// ramped velocity is now affected by collision
}

// ----------------------------------------------------------------------------
// physics_collide_whack applies an instaneous whack on an object changing
// both the objects velocity and the rotational velocity based on the impulse
// being applied.  
//
//	input:	impulse					=>		impulse vector ( force*time = impulse = change in momentum (mv) )
//				world_delta_rotvel	=>		change in rotational velocity (already calculated)
//				pi							=>		pointer to phys_info struct of object getting whacked
//				orient					=>		orientation matrix (needed to set rotational impulse in body coords)
//

// Warning:  Do not change ROTVEL_COLLIDE_WHACK_CONST.  This will mess up collision physics.
// If you need to change the rotation, change  COLLISION_ROTATION_FACTOR in collide_ship_ship.
#define ROTVEL_COLLIDE_WHACK_CONST 1.0
void physics_collide_whack( vector *impulse, vector *world_delta_rotvel, physics_info *pi, matrix *orient )
{
	vector	body_delta_rotvel;

	//	Detect null vector.
	if ((fl_abs(impulse->xyz.x) <= WHACK_LIMIT) && (fl_abs(impulse->xyz.y) <= WHACK_LIMIT) && (fl_abs(impulse->xyz.z) <= WHACK_LIMIT))
		return;

	vm_vec_rotate( &body_delta_rotvel, world_delta_rotvel, orient );
//	vm_vec_scale( &body_delta_rotvel, (float)	ROTVEL_COLLIDE_WHACK_CONST );
	vm_vec_add2( &pi->rotvel, &body_delta_rotvel );

#ifdef ROT_DEBUG
	if (check_rotvel_limit( pi )) {
		nprintf(("Physics", "rotvel reset in physics_collide_whack\n"));
	}
#endif

	update_reduced_damp_timestamp( pi, vm_vec_mag(impulse) );

	// find time for shake from weapon to end
	int dtime = timestamp_until(pi->afterburner_decay);
	if (dtime < WEAPON_SHAKE_TIME) {
		pi->afterburner_decay = timestamp( WEAPON_SHAKE_TIME );
	}

	pi->flags |= PF_REDUCED_DAMP;
	vm_vec_scale_add2( &pi->vel, impulse, 1.0f / pi->mass );
	// check that collision does not give ship too much speed
	// reset if too high
	if (!(pi->flags & PF_USE_VEL) && (vm_vec_mag_squared(&pi->vel) > MAX_SHIP_SPEED*MAX_SHIP_SPEED)) {
		// Get DaveA
		nprintf(("Physics", "speed reset in physics_collide_whack [speed: %f]\n", vm_vec_mag(&pi->vel)));
//		Int3();
		vm_vec_normalize(&pi->vel);
		vm_vec_scale(&pi->vel, (float)RESET_SHIP_SPEED);
	}
	vm_vec_rotate( &pi->prev_ramp_vel, &pi->vel, orient );		// set so velocity will ramp starting from current speed
																					// ramped velocity is now affected by collision
	// rotate previous ramp velocity (in model coord) to be same as vel (in world coords)
}


int check_rotvel_limit( physics_info *pi )
{
	if ( 0 == pi->flags )		// weapon
		return 0;

	if ( Fred_running )
		return 0;

	int change_made = 0;
	if ( !(pi->flags & PF_DEAD_DAMP) ) {
		// case of normal, live ship
		// -- Commented out by MK: Assert( vm_vec_mag_squared(&pi->max_rotvel) > ROTVEL_TOL );
		// Assert( (pi->max_rotvel.xyz.x <= ROTVEL_CAP) && (pi->max_rotvel.xyz.y <= ROTVEL_CAP) && (pi->max_rotvel.xyz.z <= ROTVEL_CAP) );
		//		Warning(LOCATION,"Excessive rotvel (wx: %f, wy: %f, wz:%f)\n", pi->rotvel.xyz.x, pi->rotvel.xyz.y, pi->rotvel.xyz.z);
		if ( fl_abs(pi->rotvel.xyz.x) > pi->max_rotvel.xyz.x ) {
			pi->rotvel.xyz.x = (pi->rotvel.xyz.x / fl_abs(pi->rotvel.xyz.x)) * (pi->max_rotvel.xyz.x - (float) ROTVEL_TOL);
			change_made = 1;
		}
		if ( fl_abs(pi->rotvel.xyz.y) > pi->max_rotvel.xyz.y ) {
			pi->rotvel.xyz.y = (pi->rotvel.xyz.y / fl_abs(pi->rotvel.xyz.y)) * (pi->max_rotvel.xyz.y - (float) ROTVEL_TOL);
			change_made = 1;
		}
		if ( fl_abs(pi->rotvel.xyz.z) > pi->max_rotvel.xyz.z ) {
			pi->rotvel.xyz.z = (pi->rotvel.xyz.z / fl_abs(pi->rotvel.xyz.z)) * (pi->max_rotvel.xyz.z - (float) ROTVEL_TOL);
			change_made = 1;
		}
	} else { 
		// case of dead ship
		if ( fl_abs(pi->rotvel.xyz.x) > DEAD_ROTVEL_CAP ) {
			pi->rotvel.xyz.x = (pi->rotvel.xyz.x / fl_abs(pi->rotvel.xyz.x)) * (float) (DEAD_ROTVEL_CAP - ROTVEL_TOL);
			change_made = 1;
		}
		if ( fl_abs(pi->rotvel.xyz.y) > DEAD_ROTVEL_CAP ) {
			pi->rotvel.xyz.y = (pi->rotvel.xyz.y / fl_abs(pi->rotvel.xyz.y)) * (float) (DEAD_ROTVEL_CAP - ROTVEL_TOL);
			change_made = 1;
		}
		if ( fl_abs(pi->rotvel.xyz.z) > DEAD_ROTVEL_CAP ) {
			pi->rotvel.xyz.z = (pi->rotvel.xyz.z / fl_abs(pi->rotvel.xyz.z)) * (float) (DEAD_ROTVEL_CAP - ROTVEL_TOL);
			change_made = 1;
		}
	}
	return change_made;
}

// ----------------------------------------------------------------------------
// update_reduced_damp_timestamp()
//
void update_reduced_damp_timestamp( physics_info *pi, float impulse )
{

	// Compute duration of reduced damp from present
	// Compare with current value and increase if greater, otherwise ignore
	int reduced_damp_decay_time;
	reduced_damp_decay_time = (int) (REDUCED_DAMP_TIME * impulse / (REDUCED_DAMP_VEL * pi->mass));
	if ( reduced_damp_decay_time > REDUCED_DAMP_TIME )
		reduced_damp_decay_time = REDUCED_DAMP_TIME;

	// Reset timestamp if larger than current (if any)
	if ( timestamp_valid( pi->reduced_damp_decay ) ) {
		int time_left = timestamp_until( pi->reduced_damp_decay );
		if ( time_left > 0 ) {
			// increment old time, but apply cap
			int new_time = reduced_damp_decay_time + time_left;
			if ( new_time < REDUCED_DAMP_TIME ) {
				pi->reduced_damp_decay = timestamp( new_time );
			}
		} else {
			pi->reduced_damp_decay = timestamp( reduced_damp_decay_time );
		}
	} else {
		// set if not valid
		pi->reduced_damp_decay = timestamp( reduced_damp_decay_time );
	}

}
