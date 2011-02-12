/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
#define PF_FORCE_GLIDE		(1 << 16)

//information for physics sim for an object
typedef struct physics_info {
	uint		flags;			//misc physics flags

	float		mass;				//the mass of this object
	vec3d		center_of_mass;		// Goober5000 - this is never ever used by physics; currently physics assumes the center of an object is the center of mass
	matrix	I_body_inv;		// inverse moment of inertia tensor (used to calculate rotational effects)

	float		rotdamp;			//rotational velocity damping
	float		side_slip_time_const;	// time const for achieving desired velocity in the local sideways direction
												//   value should be zero for no sideslip and increase depending on desired slip

	float		delta_bank_const;	//const that heading is multiplied by. 0 means no delta bank.

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
	
	float	glide_cap;	//Backslash - for 'newtonian'-style gliding, the cap on velocity (so that something can't accelerate to ridiculous speeds... unless allowed to)
	float	cur_glide_cap;	//SUSHI: Used for dynamic glide cap, so we can use the ramping function on the glide cap
	float	glide_accel_mult;	//SUSHI: The acceleration multiplier for glide mode. A value < 0 means use glide ramping instead
	bool use_newtonian_damp;	//SUSHI: Whether or not to use newtonian dampening
	float afterburner_max_reverse_vel; //SparK: This is the reverse afterburners top speed vector
	float afterburner_reverse_accel; //SparK: Afterburner's acceleration on reverse mode
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
extern void physics_collide_whack(vec3d *impulse, vec3d *delta_rotvel, physics_info *pi, matrix *orient, bool is_landing);
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

//WMC - apply_physics
void apply_physics( float damping, float desired_vel, float initial_vel, float t, float * new_vel, float * delta_pos );

//WMC - camera code type stuff
//Acceleration, constant Velocity, Deceleration (to another speed, so maybe not deceleration)
class avd_movement
{
private:
	//Current
	float Pc;		//Current position
	float Vc;		//Current velocity
	
	//Initial
	int TSi;		//Initial timestamp <-- note TIMESTAMP
	float Pi;		//Initial position
	float Vi;		//Initial velocity

	//Given
	float Pf;		//Final position
	float Tf;		//Final duration
	float Tai;		//Starting acceleration duration
	float Taf;		//Ending acceleration duration
	float Vf;		//Final velocity

	//Calculated
	float Vm;		//Middle velocity
	float Ai;		//Starting acceleration
	float Af;		//Ending acceleration
public:
	avd_movement();
	void clear();

	void get(float Time, float *Position, float *Velocity);
	void get(float *Position, float *Velocity);

	void set(float position);
	void setAVD(float final_position, float total_movement_time, float starting_accleration_time, float ending_acceleration_time, float final_velocity);
	void setVD(float total_movement_time, float ending_acceleration_time, float final_velocity);

	void update(float Time);
	void update();
};

/*
#ifdef __cplusplus
}
#endif
*/

#endif	// PHYSICS_H
