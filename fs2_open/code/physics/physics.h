/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Physics/Physics.h $
 * $Revision: 2.11 $
 * $Date: 2005-10-11 05:24:34 $
 * $Author: wmcoolmon $
 *
 * Clues to the meaning of life on Shivan planet Sphlighesphlaightseh
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.10  2005/07/13 03:35:35  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.9  2005/04/05 05:53:23  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.8  2005/01/11 21:25:58  Goober5000
 * fixed two small things with physics
 * --Goober500
 *
 * Revision 2.7  2004/08/11 05:06:32  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.6  2003/11/11 02:15:46  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.5  2003/09/13 06:02:07  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.3  2003/08/06 17:39:49  phreak
 * since the afterburners are routed through the physics engine, i needed to route the boost pod through here too
 *
 * Revision 2.2  2002/10/19 19:29:28  bobboau
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 6     8/13/99 10:49a Andsager
 * Knossos and HUGE ship warp out.  HUGE ship warp in.  Stealth search
 * modes dont collide big ships.
 * 
 * 5     7/03/99 5:50p Dave
 * Make rotated bitmaps draw properly in padlock views.
 * 
 * 4     5/11/99 10:16p Andsager
 * First pass on engine wash effect.  Rotation (control input), damage,
 * shake.  
 * 
 * 3     3/19/99 9:51a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 49    6/30/98 2:23p Dave
 * Revised object update system. Removed updates for all weapons. Put
 * button info back into control info packet.
 * 
 * 48    3/12/98 5:21p Andsager
 * Optimize physics for lasers and dumbfire missiles
 * 
 * 47    2/03/98 6:01p Andsager
 * Fix excessive rotvel in debris_create.  Check using physics function
 * check_rotvel_limit.
 * 
 * 46    12/29/97 5:10p Allender
 * fixed problems with speed not being reported properly in multiplayer
 * games.  Made read_flying_controls take frametime as a parameter.  More
 * ship/weapon select stuff
 * 
 * 45    12/02/97 11:17p Andsager
 * Added reduced_damp_decay timestamp.
 * 
 * 44    11/17/97 5:15p Andsager
 * 
 * 43    9/09/97 10:14p Andsager
 * 
 * 42    9/04/97 5:09p Andsager
 * implement physics using moment of inertia and mass (from BSPgen).
 * Added to phys_info struct.  Updated ship_info, polymodel structs.
 * Updated weapon ($Mass and $Force) and ship ($Mass -> $Density) tables
 * 
 * 41    8/29/97 10:13a Allender
 * work on server/client prediction code -- doesn't work too well.  Made
 * all clients simulate their own orientation with the server giving
 * corrections every so often.
 * 
 * 40    8/19/97 9:56a John
 * new thruster effect fairly functional
 * 
 * 39    8/11/97 9:50a Allender
 * fixed afterburner snafu
 * 
 * 38    8/08/97 9:59a Allender
 * added afterburner code into multiplayer.  Required adding a new physics
 * info flag to indicate afterburners ready to fire. (needed for
 * multiplayer).  Removed some now unused variables in afterburner.cpp
 * 
 * 37    7/31/97 12:44p Andsager
 * 
 * 36    7/27/97 5:14p Lawrance
 * add afterburners to the player control info
 * 
 * 35    7/25/97 9:07a Andsager
 * modified apply_whack
 * 
 * 34    7/23/97 5:10p Andsager
 * Enhanced shockwave effect with shake based on blast force.  
 * 
 * 33    7/22/97 2:40p Andsager
 * shockwaves now cause proper rotation of ships
 * 
 * 32    7/21/97 4:12p Mike
 * Two ships move as one while docked, including physics whacks.  Spin of
 * objects when killed based on speed.
 * 
 * 31    7/16/97 11:53a Andsager
 * 
 * 30    7/15/97 12:03p Andsager
 * New physics stuff
 * 
 * 29    7/11/97 11:54a John
 * added rotated 3d bitmaps.
 * 
 * 28    6/26/97 12:37p Allender
 * added debug laser back in
 * 
 * 27    5/08/97 11:42a Allender
 * re-ordered game loop (again).  Player weapons now created in simulation
 * loop of code, not immediately.
 * 
 * 26    4/17/97 10:02a Lawrance
 * allow ship shaking for ABURN_DECAY_TIME after afterburners cut out
 * 
 * 25    4/16/97 10:48p Mike
 * Afterburner shake.
 * Made afterburner engagement a bit in physics flags, removed from ship
 * flags, removed a parameter to physics_read_flying_controls().
 * 
 * 24    4/10/97 3:20p Mike
 * Change hull damage to be like shields.
 * 
 * 23    3/04/97 3:10p Mike
 * Intermediate checkin: Had to resolve some build errors.  Working on two
 * docked ships moving as one.
 *
 * $NoKeywords: $
 */

#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "math/vecmat.h"


#define	PF_ACCELERATES			(1 << 1)
#define	PF_USE_VEL				(1 << 2)		//	Use velocity present in physics_info struct, don't call physics_sim_vel.
#define	PF_AFTERBURNER_ON		(1 << 3)		//	Afterburner currently engaged.
#define	PF_SLIDE_ENABLED		(1 << 4)		// Allow descent style sliding
#define	PF_REDUCED_DAMP		(1 << 5)		// Allows reduced damping on z (for death, shockwave) (CAN be reset in physics)
#define	PF_IN_SHOCKWAVE		(1 << 6)		// Indicates whether object has recently been hit by shockwave (used to enable shake)
#define	PF_DEAD_DAMP			(1 << 7)		// Makes forward damping same as sideways (NOT reset in physics)
#define	PF_AFTERBURNER_WAIT	(1 << 8)		// true when afterburner cannot be used.  replaces variable used in afterburner code
#define	PF_CONST_VEL			(1 << 9)		// Use velocity in phys_info struct.  Optimize weapons in phys_sim 
#define	PF_WARP_IN				(1 << 10)	//	Use when ship is warping in
#define	PF_SPECIAL_WARP_IN	(1 << 11)	//	Use when ship is warping in and we want to slow the ship faster than normal game physics
#define	PF_WARP_OUT				(1 << 12)	//	Use when ship is warping out
#define	PF_SPECIAL_WARP_OUT	(1 << 13)	//	Use when ship is warping out and we want to slow the ship faster than normal game physics
#define PF_BOOSTER_ON		(1 << 14)
#define PF_GLIDING			(1 << 15)

//information for physics sim for an object
typedef struct physics_info {
	uint		flags;			//misc physics flags

	float		mass;				//the mass of this object
	vec3d		center_of_mass;		// Goober5000 - this is never ever used by physics; currently physics assumes the center of an object is the center of mass
	matrix	I_body_inv;		// inverse moment of inertia tensor (used to calculate rotational effects)

	float		rotdamp;			//rotational velocity damping
	float		side_slip_time_const;	// time const for achieving desired velocity in the local sideways direction
												//   value should be zero for no sideslip and increase depending on desired slip

	vec3d	max_vel;			//maximum foward velocity in x,y,z
	vec3d	afterburner_max_vel;	// maximum foward velocity in x,y,z while afterburner engaged
	vec3d booster_max_vel;
	vec3d	max_rotvel;		//maximum p,b,h rotational velocity
	float		max_rear_vel;	//maximum velocity in the backwards Z direction

	// Acceleration rates.  Only used if flag PF_ACCELERATES is set
	// starting from rest	time to reach .50  v_max	0.69 time const
	//								time to reach .75  v_max	1.39 time const
	//
	float		forward_accel_time_const;	// forward acceleration time const
	float		afterburner_forward_accel_time_const;	// forward acceleration time const while afterburner engaged
	float		booster_forward_accel_time_const;
	float		forward_decel_time_const;	// forward deceleration time const
	float		slide_accel_time_const;		// slide acceleration time const
	float		slide_decel_time_const;		// slide deceleration time const
	float		shockwave_shake_amp;			// amplitude of shockwave shake at onset

	// These get changed by the control code.  The physics uses these
	// as input values when doing physics.
	vec3d	prev_ramp_vel;				// follows the user's desired velocity
	vec3d	desired_vel;				// in world coord
	vec3d	desired_rotvel;			// in local coord
	float		forward_thrust;			// How much the forward thruster is applied.  0-1.
	float		side_thrust;			// How much the forward thruster is +x.  0-1.
	float		vert_thrust;			// How much the forward thruster is +y.  0-1.
		
	// Data that changes each frame.  Physics fills these in each frame.
	vec3d	vel;						// The current velocity vector of this object
	vec3d	rotvel;					// The current rotational velecity (angles)
	float		speed;					// Yes, this can be derived from velocity, but that's expensive!
	float		fspeed;					//	Speed in the forward direction.
	float		heading;
	vec3d	prev_fvec;				//	Used in AI for momentum.
	matrix	last_rotmat;			//	Used for moving two objects together and for editor.

	int		afterburner_decay;	// timestamp used to control how long ship shakes after afterburner released
	int		shockwave_decay;		// timestamp used to control how long ship affected after hit by shockwave
	int		reduced_damp_decay;	// timestamp used to control how long ship ship has reduced damp physics	
	
	vec3d glide_saved_vel;	//WMC - the key variable for gliding. Saves the orientation that velocity will be applied on.
} physics_info;

// All of these are numbers from -1.0 to 1.0 indicating
// what percent of full velocity you want to go.
typedef struct control_info {
	float	pitch;						// -1.0 to 1.0					
	float	vertical;
	float	heading;
	float	sideways;
	float	bank;
	float	forward;
	float forward_cruise_percent;		// percentage used for forward cruising 
										// This is a special case from -100 to 100

	// below is information that are used by the player controls for firing information.
	int	fire_primary_count;
	int	fire_secondary_count;
	int	fire_countermeasure_count;
	int	fire_debug_count;					// should this be around an NDEBUG #if/#endif?
	
	// afterburner control information
	int	afterburner_start;
	int	afterburner_stop;

} control_info;

extern int physics_paused;				//	Set means don't do physics, except for player.

// To use the "Descent-ship" physics:
//   controls_read_all(&ci, FrameSecs );
//   physics_read_flying_controls( &ViewerOrient, &ViewerPhysics, FrameSecs, &ci );
//   physics_sim(&ViewerPos, &ViewerOrient, &ViewerPhysics, FrameSecs );		
extern void physics_init( physics_info * pi );
extern void physics_read_flying_controls( matrix * orient, physics_info * pi, control_info * ci, float sim_time, vec3d *wash_rot=NULL);
extern void physics_sim(vec3d *position, matrix * orient, physics_info * pi, float sim_time );
extern void physics_sim_editor(vec3d *position, matrix * orient, physics_info * pi, float sim_time);

extern void physics_sim_vel(vec3d * position, physics_info * pi, float sim_time, matrix * orient);
extern void physics_sim_rot(matrix * orient, physics_info * pi, float sim_time );
extern void physics_apply_whack(vec3d *force, vec3d *pos, physics_info *pi, matrix *orient, float mass);
extern void physics_apply_shock(vec3d *direction_vec, float pressure, physics_info *pi, matrix *orient, vec3d *min, vec3d *max, float radius);
extern void physics_collide_whack(vec3d *impulse, vec3d *delta_rotvel, physics_info *pi, matrix *orient);
int check_rotvel_limit( physics_info *pi );



// functions which use physics calcs to predict position and velocity
void physics_predict_pos(physics_info *pi, float delta_time, vec3d *predicted_pos);
void physics_predict_vel(physics_info *pi, float delta_time, vec3d *predicted_vel);
void physics_predict_pos_and_vel(physics_info *pi, float delta_time, vec3d *predicted_vel, vec3d *predicted_pos);

// If physics_set_viewer is called with the viewer's physics_info, then
// this variable tracks the viewer's bank.  This is used for g3_draw_rotated_bitmap.
extern float Physics_viewer_bank;

// If you would like Physics_viewer_bank to be tracked (Which is needed
// for rotating 3d bitmaps) call this and pass it a pointer to the
// viewer's physics info.
#define PHYSICS_VIEWER_FRONT				0
#define PHYSICS_VIEWER_LEFT				1
#define PHYSICS_VIEWER_RIGHT				2
#define PHYSICS_VIEWER_REAR				3
#define PHYSICS_VIEWER_UP					4
void physics_set_viewer( physics_info * p, int dir );

/*
#ifdef __cplusplus
}
#endif
*/

#endif	// PHYSICS_H
