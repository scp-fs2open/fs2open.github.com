/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include "ai/ai_profiles.h"  // for the damping issue
#include "freespace.h"
#include "io/timer.h"
#include "mission/missionparse.h"
#include "mod_table/mod_table.h"
#include "physics/physics.h"
#include "ship/ship.h"
#include "utils/Random.h"



// defines for physics functions
#define	MAX_TURN_LIMIT	0.2618f		// about 15 degrees

#define ROTVEL_TOL		0.1			// Amount of rotvel is decreased if over cap
#define ROTVEL_CAP		14.0			// Rotational velocity cap for live objects
#define DEAD_ROTVEL_CAP	16.3			// Rotational velocity cap for dead objects

constexpr float MAX_SHIP_WHACK_DELTA = 500.0f;		// Maximum change in speed allowed after whack or shockwave
constexpr float RESET_SHIP_WHACK_DELTA = 440.0f;	// Change in speed that a ship is reset to after exceeding MAX_SHIP_WHACK_DELTA

#define	REDUCED_DAMP_FACTOR	10		// increase in side_slip and acceleration time constants (scaled according to reduced damp time)
#define	REDUCED_DAMP_VEL		30		// change in velocity at which reduced_damp_time is 2000 ms
#define	REDUCED_DAMP_TIME		2000	// ms (2.0 sec)
#define	WEAPON_SHAKE_TIME		500	//	ms (0.5 sec)	viewer shake time after hit by weapon (implemented via afterburner shake)

const float SUPERCAP_WARP_T_CONST = 0.651f;	// special warp time constant (lose 99 % of excess speed in 3 sec)
const float SUPERCAP_WARP_EXCESS_SPD_THRESHOLD = 5.0f;	// more than this many m/s faster than the ship's max speed and we use the above damp constant instead

void update_reduced_damp_timestamp( physics_info *pi, float impulse );
float velocity_ramp (float v_in, float v_goal, float time_const, float t);
float glide_ramp (float v_in, float v_goal, float ramp_time_const, float accel_mult, float t);

void physics_init( physics_info * pi )
{
	memset( pi, 0, sizeof(physics_info) );

	pi->mass = 10.0f;					// This ship weighs 10 units
	pi->side_slip_time_const = 0.05f;
	pi->rotdamp = 0.1f;

	pi->max_vel.xyz.x = 100.0f;		//sideways
	pi->max_vel.xyz.y = 100.0f;		//up/down
	pi->max_vel.xyz.z = 100.0f;		//forward
	pi->max_rear_vel = 100.0f;	//backward -- controlled separately

	pi->max_rotvel = vm_vec_new(2.0f, 1.0f, 2.0f);

	vm_vec_zero(&pi->prev_ramp_vel);

	vm_vec_zero(&pi->desired_vel);

	pi->slide_accel_time_const=pi->side_slip_time_const;	// slide using max_vel.xyz.x & .xyz.y
	pi->slide_decel_time_const=pi->side_slip_time_const;	// slide using max_vel.xyz.x & .xyz.y

	pi->afterburner_decay = 1;

	vm_vec_zero(&pi->linear_thrust);
	vm_vec_zero(&pi->rotational_thrust);

	pi->flags = 0;

	// default values for moment of inertia
	vm_vec_make( &pi->I_body_inv.vec.rvec, 1e-5f, 0.0f, 0.0f );
	vm_vec_make( &pi->I_body_inv.vec.uvec, 0.0f, 1e-5f, 0.0f );
	vm_vec_make( &pi->I_body_inv.vec.fvec, 0.0f, 0.0f, 1e-5f );

	pi->ai_desired_orient = vmd_zero_matrix; // Asteroth - initialize to the "invalid" orientation, which will be ignored by physics unless set otherwise

	vm_vec_zero(&pi->acceleration);

	pi->gravity_const = 0.0f;
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
physics_info * Viewer_physics_info = NULL;

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
	vec3d	new_vel;
	matrix	tmp;
	float		rotdamp;
	float		shock_fraction_time_left;

	Assert(is_valid_matrix(orient));
	Assert(is_valid_vec(&pi->rotvel));
	Assert(is_valid_vec(&pi->desired_rotvel));

	// Handle special case of shockwave
	float		shock_amplitude = 0.0f;
	if (pi->flags & PF_IN_SHOCKWAVE) {
		if (timestamp_elapsed(pi->shockwave_decay)) {
			pi->flags &= ~PF_IN_SHOCKWAVE;
			rotdamp = pi->rotdamp;
		}
		else {
			shock_fraction_time_left = timestamp_until(pi->shockwave_decay) / (float)SW_BLAST_DURATION;
			rotdamp = pi->rotdamp + pi->rotdamp * (SW_ROT_FACTOR - 1) * shock_fraction_time_left;
			shock_amplitude = pi->shockwave_shake_amp * shock_fraction_time_left;
		}
	}
	else if (pi->flags & PF_MANEUVER_NO_DAMP) {
		rotdamp = 0.0f;
	}
	else {
		rotdamp = pi->rotdamp;
	}

	angles	tangles = vmd_zero_angles;

	// "frit" here meaning Framerate_independent_turning
	bool frit_ai_wants_to_move = Framerate_independent_turning && !IS_MAT_NULL(&pi->ai_desired_orient);
	// In the case an ai entity used angular_move, we should use those calculations instead
	if (frit_ai_wants_to_move){
		Assert(is_valid_matrix(&pi->ai_desired_orient));

		// AI simply get the rotvel they ask for, calculations were already done
		pi->rotvel = pi->desired_rotvel;
		// Zero it out because the same AI might not call angular_move again next time
		// So we'll assume it's going to "go limp" unless it specifies otherwise next frame
		vm_vec_zero(&pi->desired_rotvel);

		// Pretty much same as above
		*orient = pi->ai_desired_orient;
		vm_mat_zero(&pi->ai_desired_orient);

	} else { // players and no framerate_independent_turning go here

		// Do rotational physics with given damping
		apply_physics(rotdamp, pi->desired_rotvel.xyz.x, pi->rotvel.xyz.x, sim_time, &new_vel.xyz.x, nullptr);
		apply_physics(rotdamp, pi->desired_rotvel.xyz.y, pi->rotvel.xyz.y, sim_time, &new_vel.xyz.y, nullptr);
		apply_physics(rotdamp, pi->desired_rotvel.xyz.z, pi->rotvel.xyz.z, sim_time, &new_vel.xyz.z, nullptr);

		Assert(is_valid_vec(&new_vel));

		pi->rotvel = new_vel;

		tangles.p = pi->rotvel.xyz.x * sim_time;
		tangles.h = pi->rotvel.xyz.y * sim_time;
		tangles.b = pi->rotvel.xyz.z * sim_time;
	
	}

	// Make ship shake due to shockwave, decreasing in amplitude at the end of the shockwave
	if ( pi->flags & PF_IN_SHOCKWAVE ) {
		tangles.p += (float) (Random::next()-Random::HALF_MAX_VALUE) * Random::INV_F_MAX_VALUE * shock_amplitude;
		tangles.h += (float) (Random::next()-Random::HALF_MAX_VALUE) * Random::INV_F_MAX_VALUE * shock_amplitude;
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
	vec3d	new_vel;
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
void physics_sim_vel(vec3d * position, physics_info * pi, matrix *orient, vec3d* gravity, float sim_time)
{
	vec3d local_disp;		// displacement in this frame
	vec3d local_v_in;		// velocity in local coords at the start of this frame
	vec3d local_desired_vel;	// desired velocity in local coords
	vec3d local_v_out;		// velocity in local coords following this frame
	vec3d damp;

	vec3d old_vel = pi->vel;

	//	Maybe clear the reduced_damp flag.
	//	This fixes the problem of the player getting near-instantaneous acceleration under unknown circumstances.
	//	The larger problem is probably that PF_USE_VEL is getting stuck set.
	if ((pi->flags & PF_REDUCED_DAMP) && (timestamp_elapsed(pi->reduced_damp_decay))) {
		pi->flags &= ~PF_REDUCED_DAMP;
	}

	// Set up damping constants based on special conditions
	// dead
	if (pi->flags & PF_DEAD_DAMP) {
		// side_slip_time_const is already quite large and now needs to be applied in all directions
		vm_vec_make( &damp, pi->side_slip_time_const, pi->side_slip_time_const, pi->side_slip_time_const );

	}
	// case of shock, weapon, collide, etc.
	else if (pi->flags & PF_REDUCED_DAMP) {
		if ( timestamp_elapsed(pi->reduced_damp_decay) ) {
			vm_vec_make( &damp, pi->side_slip_time_const, pi->side_slip_time_const, 0.0f );
		} else {
			// damp is multiplied by fraction and not fraction^2, gives better collision separation
			float reduced_damp_fraction_time_left = timestamp_until( pi->reduced_damp_decay ) / (float) REDUCED_DAMP_TIME;
			damp.xyz.x = pi->side_slip_time_const * ( 1 + (REDUCED_DAMP_FACTOR-1) * reduced_damp_fraction_time_left );
			damp.xyz.y = pi->side_slip_time_const * ( 1 + (REDUCED_DAMP_FACTOR-1) * reduced_damp_fraction_time_left );
			damp.xyz.z = pi->side_slip_time_const * reduced_damp_fraction_time_left * REDUCED_DAMP_FACTOR;
		}
	}
	// no damping at all
	else if (pi->flags & PF_MANEUVER_NO_DAMP) {
		damp = vmd_zero_vector;
	}
	// newtonian
	else if (pi->flags & PF_NEWTONIAN_DAMP) {
		vm_vec_make( &damp, pi->side_slip_time_const, pi->side_slip_time_const, pi->side_slip_time_const );
	}
	// regular damping
	else {
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

	vec3d grav_disp = vmd_zero_vector;
	vec3d grav_vel = vmd_zero_vector;
	int special_warp_in = FALSE;
	float excess = local_v_in.xyz.z - pi->max_vel.xyz.z;
	if (excess > SUPERCAP_WARP_EXCESS_SPD_THRESHOLD && (pi->flags & PF_SUPERCAP_WARP_IN)) {
		special_warp_in = TRUE;
		float exp_factor = float(exp(-sim_time / SUPERCAP_WARP_T_CONST));
		local_v_out.xyz.z = pi->max_vel.xyz.z + excess * exp_factor;
		local_disp.xyz.z = (pi->max_vel.xyz.z * sim_time) + excess * (float(SUPERCAP_WARP_T_CONST) * (1.0f - exp_factor));
	} else if (pi->flags & PF_SUPERCAP_WARP_OUT) {
		float exp_factor = float(exp(-sim_time / SUPERCAP_WARP_T_CONST));
		vec3d temp;
		vm_vec_rotate(&temp, &pi->prev_ramp_vel, orient);
		float deficeit = temp.xyz.z - local_v_in.xyz.z;
		local_v_out.xyz.z = local_v_in.xyz.z + deficeit * (1.0f - exp_factor);
		local_disp.xyz.z = (local_v_in.xyz.z * sim_time) + deficeit * (sim_time - (float(SUPERCAP_WARP_T_CONST) * (1.0f - exp_factor)));
	} else {
		apply_physics (damp.xyz.z, local_desired_vel.xyz.z, local_v_in.xyz.z, sim_time, &local_v_out.xyz.z, &local_disp.xyz.z);
		if (pi->gravity_const != 0.0f) {
			grav_vel = *gravity * sim_time * pi->gravity_const;
			grav_disp = (grav_vel * sim_time) * 0.5; // 1/2 * at^2
		}
	}

	// maybe turn off special warp in flag
	if ((pi->flags & PF_SUPERCAP_WARP_IN) && (excess < SUPERCAP_WARP_EXCESS_SPD_THRESHOLD)) {
		pi->flags &= ~(PF_SUPERCAP_WARP_IN);
	}
	
	if (!(pi->flags & PF_SCRIPTED_VELOCITY)) {
		// update world position from local to world coords using orient
		vec3d world_disp;
		vm_vec_unrotate(&world_disp, &local_disp, orient);
		*position += world_disp;
		*position += grav_disp;

		// update world velocity	
		vm_vec_unrotate(&pi->vel, &local_v_out, orient);
		pi->vel += grav_vel;
	} else {
		// velocity set by script, just trust whatever that is, at least for this frame
		*position += pi->vel * sim_time;
		pi->flags &= ~PF_SCRIPTED_VELOCITY;
	}

	// update acceleration
	pi->acceleration = pi->vel - old_vel;
	pi->acceleration *= 1 / sim_time;

	if (special_warp_in) {
		vm_vec_rotate(&pi->prev_ramp_vel, &pi->vel, orient);
	}
}

//	-----------------------------------------------------------------------------------------------------------
// Simulate a physics object for this frame
void physics_sim(vec3d* position, matrix* orient, physics_info* pi, vec3d* gravity, float sim_time)
{
	// check flag which tells us whether or not to do velocity translation
	if (pi->flags & PF_CONST_VEL) {
		*position += pi->vel * sim_time;
	}
	else
	{
		if (pi->flags & PF_BALLISTIC) {
			*position += pi->vel * sim_time + *gravity * sim_time * sim_time * pi->gravity_const * 0.5f; // vt + 1/2 * at^2
			pi->vel += *gravity * sim_time * pi->gravity_const;
		} else {
			physics_sim_vel(position, pi, orient, gravity, sim_time);
		}

		physics_sim_rot(orient, pi, sim_time);

		pi->speed = vm_vec_mag(&pi->vel);							//	Note, cannot use quick version, causes cumulative error, increasing speed.
		pi->fspeed = vm_vec_dot(&orient->vec.fvec, &pi->vel);		// instead of vector magnitude -- use only forward vector since we are only interested in forward velocity
	}
}

//	-----------------------------------------------------------------------------------------------------------
// Simulate a physics object for this frame.  Used by the editor.  The difference between
// this function and physics_sim() is that this one uses a heading change to rotate around
// the universal Y axis, rather than the local orientation's Y axis.  Banking is also ignored.
void physics_sim_editor(vec3d *position, matrix * orient, physics_info * pi, float sim_time )
{
	physics_sim_vel(position, pi,orient, &vmd_zero_vector, sim_time);
	physics_sim_rot_editor(orient, pi, sim_time);
	pi->speed = vm_vec_mag_quick(&pi->vel);
	pi->fspeed = vm_vec_dot(&orient->vec.fvec, &pi->vel);		// instead of vector magnitude -- use only forward vector since we are only interested in forward velocity
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
void physics_read_flying_controls( matrix * orient, physics_info * pi, control_info * ci, float sim_time, vec3d *wash_rot)
{
	vec3d goal_vel;		// goal velocity in local coords, *not* accounting for ramping of velcity
	float ramp_time_const;		// time constant for velocity ramping

	// apply throttle, unless reverse thrusters are held down
	if (ci->forward != -1.0f)
		ci->forward += (ci->forward_cruise_percent / 100.0f);

	// give control input to cause rotation in engine wash
	extern int Wash_on;
	if ( wash_rot && Wash_on ) {
		ci->pitch += wash_rot->xyz.x;
		ci->bank += wash_rot->xyz.z;
		ci->heading += wash_rot->xyz.y;
	}

	if ( pi->flags & PF_AFTERBURNER_ON ){
		//SparK: modifield to accept reverse burners
		if (!(pi->afterburner_max_reverse_vel > 0.0f)){
			ci->forward = 1.0f;
		}
	}

	// maybe we don't need to clamp our velocity...
	if ( !(ci->control_flags & CIF_DONT_CLAMP_MAX_VELOCITY) )
	{
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

		if (ci->forward > 1.0f ) ci->forward = 1.0f;
		else if (ci->forward < -1.0f ) ci->forward = -1.0f;
	}

	// If Framerate_independent_turning is on, AI don't use CI to turn
	if (!Framerate_independent_turning || IS_MAT_NULL(&pi->ai_desired_orient)){
		if (!Flight_controls_follow_eyepoint_orientation || (Player_obj == nullptr)) {
			// Default behavior; eyepoint orientation has no effect on controls
			pi->desired_rotvel.xyz.x = ci->pitch * pi->max_rotvel.xyz.x;
			pi->desired_rotvel.xyz.y = ci->heading * pi->max_rotvel.xyz.y;
		} else {
			// Optional behavior; pitch and yaw are always relative to the eyepoint
			// orientation (excluding slew)
			vec3d tmp_vec, new_rotvel;
			matrix tmp_mat, eyemat, rotvelmat;

			object_get_eye(&tmp_vec, &eyemat, Player_obj, false);

			vm_copy_transpose(&tmp_mat, &Player_obj->orient);
			vm_matrix_x_matrix(&rotvelmat, &tmp_mat, &eyemat);

			vm_vec_rotate(&new_rotvel, &pi->max_rotvel, &rotvelmat);
			vm_vec_unrotate(&tmp_vec, &pi->max_rotvel, &rotvelmat);
			new_rotvel.xyz.x = tmp_vec.xyz.x;

			new_rotvel.xyz.x = ci->pitch * new_rotvel.xyz.x;
			new_rotvel.xyz.y = ci->heading * new_rotvel.xyz.y;

			vm_vec_unrotate(&tmp_vec, &new_rotvel, &rotvelmat);

			pi->desired_rotvel = tmp_vec;
		}

		float	delta_bank;

#ifdef BANK_WHEN_TURN
		if ( ci->control_flags & CIF_DONT_BANK_WHEN_TURNING )
			delta_bank = 0.0f;
		else
		{
			//	To change direction of bank, negate the whole expression.
			//	To increase magnitude of banking, decrease denominator.
			//	Adam: The following statement is all the math for banking while turning.
			delta_bank = - (ci->heading * pi->max_rotvel.xyz.y) * pi->delta_bank_const;
		}
#else
		delta_bank = 0.0f;
#endif

		pi->desired_rotvel.xyz.z = ci->bank * pi->max_rotvel.xyz.z + delta_bank;
	}

	// update 'thrust values' (these are cosmetic and do not affect physical behavior)
	if (Thruster_easing > 0.0f) {
		pi->linear_thrust.xyz.z = ci->forward +  (pi->linear_thrust.xyz.z - ci->forward) *  exp2f(-Thruster_easing * sim_time);
		pi->linear_thrust.xyz.y = ci->vertical + (pi->linear_thrust.xyz.y - ci->vertical) * exp2f(-Thruster_easing * sim_time);
		pi->linear_thrust.xyz.x = ci->sideways + (pi->linear_thrust.xyz.x - ci->sideways) * exp2f(-Thruster_easing * sim_time);

		pi->rotational_thrust.xyz.z = ci->bank +    (pi->rotational_thrust.xyz.z - ci->bank) *    exp2f(-Thruster_easing * sim_time);
		pi->rotational_thrust.xyz.y = ci->heading + (pi->rotational_thrust.xyz.y - ci->heading) * exp2f(-Thruster_easing * sim_time);
		pi->rotational_thrust.xyz.x = ci->pitch +   (pi->rotational_thrust.xyz.x - ci->pitch) *   exp2f(-Thruster_easing * sim_time);
	} else {
		pi->linear_thrust.xyz.z = ci->forward;
		pi->linear_thrust.xyz.y = ci->vertical;
		pi->linear_thrust.xyz.x = ci->sideways;

		pi->rotational_thrust.xyz.z = ci->bank;
		pi->rotational_thrust.xyz.y = ci->heading;
		pi->rotational_thrust.xyz.x = ci->pitch;
	}

	if ( pi->flags & PF_AFTERBURNER_ON ) {
		goal_vel.xyz.x = ci->sideways*pi->afterburner_max_vel.xyz.x;
		goal_vel.xyz.y = ci->vertical*pi->afterburner_max_vel.xyz.y;
		if(ci->forward < 0.0f)
			goal_vel.xyz.z = ci->forward* pi->afterburner_max_reverse_vel;
		else
			goal_vel.xyz.z = ci->forward* pi->afterburner_max_vel.xyz.z;
	}
	else if ( pi->flags & PF_BOOSTER_ON ) {
		goal_vel.xyz.x = ci->sideways*pi->booster_max_vel.xyz.x;
		goal_vel.xyz.y = ci->vertical*pi->booster_max_vel.xyz.y;
		goal_vel.xyz.z = ci->forward* pi->booster_max_vel.xyz.z;
	}
	else {
		goal_vel.xyz.x = ci->sideways*pi->max_vel.xyz.x;
		goal_vel.xyz.y = ci->vertical*pi->max_vel.xyz.y;
		goal_vel.xyz.z = ci->forward* pi->max_vel.xyz.z;
	}

	if ( goal_vel.xyz.z < -pi->max_rear_vel && !(pi->flags & PF_AFTERBURNER_ON) && !(ci->control_flags & CIF_DONT_CLAMP_MAX_VELOCITY) )
		goal_vel.xyz.z = -pi->max_rear_vel;


	// instantaneous acceleration is trickier than it looks
	if ( ci->control_flags & CIF_INSTANTANEOUS_ACCELERATION ) {
		// ramp up instantaneously
		pi->prev_ramp_vel = goal_vel;

		// translate local desired velocities to world velocities, as below
		vm_vec_zero(&pi->desired_vel);
		vm_vec_scale_add2(&pi->desired_vel, &orient->vec.rvec, pi->prev_ramp_vel.xyz.x);
		vm_vec_scale_add2(&pi->desired_vel, &orient->vec.uvec, pi->prev_ramp_vel.xyz.y);
		vm_vec_scale_add2(&pi->desired_vel, &orient->vec.fvec, pi->prev_ramp_vel.xyz.z);
	}
	// standard acceleration
	else if ( pi->flags & PF_ACCELERATES ) {
		//
		// Determine *resultant* DESIRED VELOCITY (desired_vel) accounting for RAMPING of velocity
		// Use LOCAL coordinates
		// if slide_enabled, ramp velocity for x and y, otherwise set goal (0)
		//    always ramp velocity for z
		//

		// If reduced damp in effect, then adjust ramp_velocity and desired_velocity can not change as fast.
		// Scale according to reduced_damp_time_expansion.
		float reduced_damp_ramp_time_expansion;
		if ( pi->flags & PF_REDUCED_DAMP && !timestamp_elapsed(pi->reduced_damp_decay) ) {
			float reduced_damp_fraction_time_left = timestamp_until( pi->reduced_damp_decay ) / (float) REDUCED_DAMP_TIME;
			reduced_damp_ramp_time_expansion = 1.0f + (REDUCED_DAMP_FACTOR-1) * reduced_damp_fraction_time_left;
		} else {
			reduced_damp_ramp_time_expansion = 1.0f;
		}

		if (pi->flags & PF_SLIDE_ENABLED)  {
			// determine the local velocity
			// deterimine whether accelerating or deceleration toward goal for x
			if ( goal_vel.xyz.x > 0.0f )  {
				if ( goal_vel.xyz.x >= pi->prev_ramp_vel.xyz.x )
					ramp_time_const = pi->slide_accel_time_const;
				else
					ramp_time_const = pi->slide_decel_time_const;
			} else if ( goal_vel.xyz.x < 0.0f ) {
				if ( goal_vel.xyz.x <= pi->prev_ramp_vel.xyz.x )
					ramp_time_const = pi->slide_accel_time_const;
				else
					ramp_time_const = pi->slide_decel_time_const;
			} else {
				ramp_time_const = pi->slide_decel_time_const;
			}
			// If reduced damp in effect, then adjust ramp_velocity and desired_velocity can not change as fast
			if ( pi->flags & PF_REDUCED_DAMP ) {
				ramp_time_const *= reduced_damp_ramp_time_expansion;
			}
			pi->prev_ramp_vel.xyz.x = velocity_ramp(pi->prev_ramp_vel.xyz.x, goal_vel.xyz.x, ramp_time_const, sim_time);

			// deterimine whether accelerating or deceleration toward goal for y
			if ( goal_vel.xyz.y > 0.0f )  {
				if ( goal_vel.xyz.y >= pi->prev_ramp_vel.xyz.y )
					ramp_time_const = pi->slide_accel_time_const;
				else
					ramp_time_const = pi->slide_decel_time_const;
			} else if ( goal_vel.xyz.y < 0.0f ) {
				if ( goal_vel.xyz.y <= pi->prev_ramp_vel.xyz.y )
					ramp_time_const = pi->slide_accel_time_const;
				else
					ramp_time_const = pi->slide_decel_time_const;
			} else {
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

		// deterimine whether accelerating or deceleration toward goal for z
		if ( goal_vel.xyz.z > 0.0f )  {
			if ( goal_vel.xyz.z >= pi->prev_ramp_vel.xyz.z )  {
				if ( pi->flags & PF_AFTERBURNER_ON )
					ramp_time_const = pi->afterburner_forward_accel_time_const;
				else if (pi->flags & PF_BOOSTER_ON)
					ramp_time_const = pi->booster_forward_accel_time_const;
				else
					ramp_time_const = pi->forward_accel_time_const;
			} else {
				ramp_time_const = pi->forward_decel_time_const;
			}
		} else if ( goal_vel.xyz.z < 0.0f ) {
			if ( pi->flags & PF_AFTERBURNER_ON )
				ramp_time_const = pi->afterburner_reverse_accel;
			else
				ramp_time_const = pi->forward_decel_time_const;
		} else {
			ramp_time_const = pi->forward_decel_time_const;
		}

		// If reduced damp in effect, then adjust ramp_velocity and desired_velocity can not change as fast
		if ( pi->flags & PF_REDUCED_DAMP ) {
			ramp_time_const *= reduced_damp_ramp_time_expansion;
		}
		pi->prev_ramp_vel.xyz.z = velocity_ramp(pi->prev_ramp_vel.xyz.z, goal_vel.xyz.z, ramp_time_const, sim_time);

		//Deternine the current dynamic glide cap, and ramp to it
		//This is outside the normal "glide" block since we want the cap to adjust whether or not the ship is in glide mode
		float dynamic_glide_cap_goal = 0.0;
		if (pi->flags & PF_AFTERBURNER_ON) {
			dynamic_glide_cap_goal = ( goal_vel.xyz.z >= 0.0f ) ? pi->afterburner_max_vel.xyz.z : pi->afterburner_max_reverse_vel;
		}
		else {
			//Use the maximum value in X, Y, and Z (including overclocking)
			dynamic_glide_cap_goal = std::max({ pi->max_vel.xyz.x,pi->max_vel.xyz.y, pi->max_vel.xyz.z });
		}
		pi->cur_glide_cap = velocity_ramp(pi->cur_glide_cap, dynamic_glide_cap_goal, ramp_time_const, sim_time);


		if ( (pi->flags & PF_GLIDING) || (pi->flags & PF_FORCE_GLIDE ) ) {
			pi->desired_vel = pi->vel;

			//SUSHI: A (hopefully better) approach to dealing with accelerations in glide mode
			//Get *actual* current velocities along each axis and use those instead of ramped velocities
			vec3d local_vel;
			vm_vec_rotate(&local_vel, &pi->vel, orient);

			//Having pi->glide_cap == 0 means we're using a dynamic glide cap
			float curGlideCap = 0.0f;
			if (pi->glide_cap == 0.0f) 
				curGlideCap = pi->cur_glide_cap;
			else 
				curGlideCap = pi->glide_cap;

			//If we're near the (positive) glide cap, decay velocity where we aren't thrusting
			//This is a hack, but makes the flight feel a lot smoother
			//Don't do this if we aren't applying any thrust, we have no glide cap, or the accel multiplier is 0 (no thrust while gliding)
			float cap_decay_threshold = 0.95f;
			float cap_decay_amount = 0.2f;
			if (curGlideCap >= 0.0f && vm_vec_mag(&pi->desired_vel) >= cap_decay_threshold * curGlideCap && 
					vm_vec_mag(&goal_vel) > 0.0f &&
					pi->glide_accel_mult != 0.0f) 
			{
				if (goal_vel.xyz.x == 0.0f)
					vm_vec_scale_add2(&pi->desired_vel, &orient->vec.rvec, -cap_decay_amount * local_vel.xyz.x);
				if (goal_vel.xyz.y == 0.0f)
					vm_vec_scale_add2(&pi->desired_vel, &orient->vec.uvec, -cap_decay_amount * local_vel.xyz.y);
				if (goal_vel.xyz.z == 0.0f)
					vm_vec_scale_add2(&pi->desired_vel, &orient->vec.fvec, -cap_decay_amount * local_vel.xyz.z);
			}

			//The glide_ramp function uses (basically) the same math as the velocity ramp so that thruster power is consistent
			//Only ramp if the glide cap is positive
			float xVal = glide_ramp(local_vel.xyz.x, goal_vel.xyz.x, pi->slide_accel_time_const, pi->glide_accel_mult, sim_time);
			float yVal = glide_ramp(local_vel.xyz.y, goal_vel.xyz.y, pi->slide_accel_time_const, pi->glide_accel_mult, sim_time);
			float zVal = 0.0;
			if (pi->flags & PF_AFTERBURNER_ON) 
				zVal = glide_ramp(local_vel.xyz.z, goal_vel.xyz.z, pi->afterburner_forward_accel_time_const, pi->glide_accel_mult, sim_time);
			else {
				if (goal_vel.xyz.z >= 0.0f)
					zVal = glide_ramp(local_vel.xyz.z, goal_vel.xyz.z, pi->forward_accel_time_const, pi->glide_accel_mult, sim_time);
				else
					zVal = glide_ramp(local_vel.xyz.z, goal_vel.xyz.z, pi->forward_decel_time_const, pi->glide_accel_mult, sim_time);
			}

			//Compensate for effect of dampening: normal flight cheats here, so /we make up for it this way so glide acts the same way
			xVal *= pi->side_slip_time_const / sim_time;
			yVal *= pi->side_slip_time_const / sim_time;
			if (pi->flags & PF_NEWTONIAN_DAMP)
				zVal *= pi->side_slip_time_const / sim_time;

			vm_vec_scale_add2(&pi->desired_vel, &orient->vec.fvec, zVal);
			vm_vec_scale_add2(&pi->desired_vel, &orient->vec.rvec, xVal);
			vm_vec_scale_add2(&pi->desired_vel, &orient->vec.uvec, yVal);

			// Only do the glide cap if we have one and are actively thrusting in some direction.
			// Unless AIPF2_GLIDE_DECAY_REQUIRES_THRUST isn't set. -MageKing17
			if ( curGlideCap >= 0.0f && (!(The_mission.ai_profile->flags[AI::Profile_Flags::Glide_decay_requires_thrust]) || ci->forward != 0.0f || ci->sideways != 0.0f || ci->vertical != 0.0f) ) {
				float currentmag = vm_vec_mag(&pi->desired_vel);
				if ( currentmag > curGlideCap ) {
					vm_vec_scale( &pi->desired_vel, curGlideCap / currentmag );
				}
			}
		}
		else
		{
			// this translates local desired velocities to world velocities
			vm_vec_zero(&pi->desired_vel);
			vm_vec_scale_add2( &pi->desired_vel, &orient->vec.rvec, pi->prev_ramp_vel.xyz.x );
			vm_vec_scale_add2( &pi->desired_vel, &orient->vec.uvec, pi->prev_ramp_vel.xyz.y );
			vm_vec_scale_add2( &pi->desired_vel, &orient->vec.fvec, pi->prev_ramp_vel.xyz.z );
		}
	}
	// object does not accelerate  (PF_ACCELERATES not set)
	else {
		pi->desired_vel = pi->vel;
	}
}


void physics_maybe_reset_speed_after_whack(physics_info *pi, const vec3d *previous_vel, const char *source)
{
	// for release builds
	SCP_UNUSED(source);

	if (pi->flags & PF_USE_VEL || The_mission.ai_profile->flags[AI::Profile_Flags::Dont_limit_change_in_speed_due_to_physics_whack])
		return;

	vec3d delta_vel = pi->vel - *previous_vel;
	float speed_delta = vm_vec_mag(&delta_vel);

	// Limit change in speed due to whack
	if (speed_delta > MAX_SHIP_WHACK_DELTA)
	{
		// Get DaveA
		nprintf(("Physics", "speed reset in %s [change in speed: %f; current speed: %f]\n", source, speed_delta, vm_vec_mag(&pi->vel)));
		vm_vec_normalize(&delta_vel);
		vm_vec_scale_add(&pi->vel, previous_vel, &delta_vel, RESET_SHIP_WHACK_DELTA);
	}
}

#define WHACK_LIMIT 0.001f
#define ROTVEL_WHACK_CONST 0.12f

//	-----------------------------------------------------------------------------------------------------------
// Returns true if this impulse is below the limit and should be ignored.
bool whack_below_limit(const vec3d* impulse)
{
	return (fl_abs(impulse->xyz.x) < WHACK_LIMIT) && (fl_abs(impulse->xyz.y) < WHACK_LIMIT) &&
		   (fl_abs(impulse->xyz.z) < WHACK_LIMIT);
}

bool whack_below_limit(float impulse)
{
	return fl_abs(impulse) < WHACK_LIMIT;
}

// ----------------------------------------------------------------------------
// physics_calculate_and_apply_whack changes the rotaional and linear velocites of a ship due to
// an instantaneous whack.
//
//	input:	impulse		=>		impulse vector (direction and magnitude of the impulse)
//				pos			=>		vector from center of mass to location (in world coords) of where the force acts 
//				pi				=>		pointer to phys_info struct of object getting whacked
//				orient		=>		orientation matrix (needed to set rotational impulse in body coords)
//
void physics_calculate_and_apply_whack(const vec3d *impulse, const vec3d *pos, physics_info *pi, const matrix *orient, const matrix *inv_moi)
{
	vec3d	local_angular_impulse, angular_impulse;

	//	Detect null vector.
	if (whack_below_limit(impulse))
		return;

	// first do the rotational velocity
	// calculate the angular impulse on the body based on the point on the
	// object that was hit and the momentum being applied to the object

	vm_vec_cross(&angular_impulse, pos, impulse);
	vm_vec_rotate ( &local_angular_impulse, &angular_impulse, orient );

	vec3d delta_rotvel;
	vm_vec_rotate(&delta_rotvel, &local_angular_impulse, inv_moi);

	vec3d delta_vel = *impulse * (1.0f / pi->mass);

	physics_apply_whack(vm_vec_mag(impulse), pi, &delta_rotvel, &delta_vel, orient);
}

// This function applies the calculated delta rotational and linear velocities calculated by physics_calculate_and_apply_whack
// or dock_calculate_and_apply_whack_docked_object in objectdock.cpp if it was a docked object
void physics_apply_whack(float orig_impulse, physics_info* pi, const vec3d *delta_rotvel, const vec3d* delta_vel, const matrix* orient)
{
	Assertion((pi != nullptr) && (delta_rotvel != nullptr) && (delta_vel != nullptr) && (orient != nullptr),
		"physics_apply_whack_direct invalid argument(s)");

	vm_vec_scale_add2(&pi->rotvel, delta_rotvel, ROTVEL_WHACK_CONST);

	//mprintf(("Whack: %7.3f %7.3f %7.3f\n", pi->rotvel.xyz.x, pi->rotvel.xyz.y, pi->rotvel.xyz.z));

	// instant whack on the velocity
	// reduce damping on all axes
	pi->flags |= PF_REDUCED_DAMP;
	update_reduced_damp_timestamp( pi, orig_impulse );

	// find time for shake from weapon to end
	int dtime = timestamp_until(pi->afterburner_decay);
	if (dtime < WEAPON_SHAKE_TIME) {
		pi->afterburner_decay = timestamp( WEAPON_SHAKE_TIME );
	}

	vec3d previous_vel = pi->vel;
	vm_vec_add2(&pi->vel, delta_vel);

	physics_maybe_reset_speed_after_whack(pi, &previous_vel, "physics_apply_whack");

	// set so velocity will ramp starting from current speed
	// ramped velocity is now affected by collision
	vm_vec_rotate( &pi->prev_ramp_vel, &pi->vel, orient );												
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

//Handles thrust values when gliding. Purposefully similar to velocity_ramp in order to yeild a similar "feel" in
//terms of thruster accels (as much as possible)
//This is all in the local frame of reference for a single movement axis
float glide_ramp (float v_in, float v_goal, float ramp_time_const, float accel_mult, float t)
{
	if (v_goal == 0.0f) {
		return 0.0f;
	}

	//This approach to delta_v allows us to ramp accelerations even in glide mode
	//Get the difference in velocity between current and goal as thrust 
	//(capped by the goal velocity on one end and 0 on the other)
	//If accel_mult is < 0, don't ramp (fixed acceleration)
	float delta_v = 0.0f;
	if (accel_mult < 0.0f) {
		if (v_goal > 0.0f) {
			delta_v = MAX(MIN(v_goal - v_in, v_goal), 0.0f); 
		}
		else {
			delta_v = MIN(MAX(v_goal - v_in, v_goal), 0.0f);
		}
	}
	else {
		delta_v = v_goal * accel_mult;
	}
	
	//Calculate the (decayed) thrust
	float decay_factor = (ramp_time_const > 0.0f) ? (1.0f - (float)exp(-t / ramp_time_const)) : 1.0f;
	return delta_v * decay_factor;
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
void physics_apply_shock(vec3d *direction_vec, float pressure, physics_info *pi, matrix *orient, vec3d *min, vec3d *max, float radius)
{
	vec3d normal;
	vec3d local_torque, temp_torque, torque;
	vec3d impact_vec;
	vec3d area;
	vec3d sin;

	if (radius > MAX_RADIUS) {
		return;
	}

	vm_vec_normalize_safe ( direction_vec );

	area.xyz.x = (max->xyz.y - min->xyz.y) * (max->xyz.z - min->xyz.z);
	area.xyz.y = (max->xyz.x - min->xyz.x) * (max->xyz.z - min->xyz.z);
	area.xyz.z = (max->xyz.x - min->xyz.x) * (max->xyz.y - min->xyz.y);

	normal.xyz.x = vm_vec_dot( direction_vec, &orient->vec.rvec );
	normal.xyz.y = vm_vec_dot( direction_vec, &orient->vec.uvec );
	normal.xyz.z = vm_vec_dot( direction_vec, &orient->vec.fvec );

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

	vm_vec_cross( &temp_torque, &impact_vec, direction_vec );
	vm_vec_add2( &torque, &temp_torque );

	// find torque due to forces on the up/down face
	if ( normal.xyz.y < 0.0f )
		vm_vec_copy_scale( &impact_vec, &orient->vec.uvec, max->xyz.y * pressure * area.xyz.y *  normal.xyz.y * sin.xyz.y / pi->mass );
	else
		vm_vec_copy_scale( &impact_vec, &orient->vec.uvec, min->xyz.y * pressure * area.xyz.y * -normal.xyz.y * sin.xyz.y / pi->mass );

	vm_vec_cross( &temp_torque, &impact_vec, direction_vec );
	vm_vec_add2( &torque, &temp_torque );

	// find torque due to forces on the forward/backward face
	if ( normal.xyz.z < 0.0f )
		vm_vec_copy_scale( &impact_vec, &orient->vec.fvec, max->xyz.z * pressure * area.xyz.z *  normal.xyz.z * sin.xyz.z / pi->mass );
	else
		vm_vec_copy_scale( &impact_vec, &orient->vec.fvec, min->xyz.z * pressure * area.xyz.z * -normal.xyz.z * sin.xyz.z / pi->mass );

	vm_vec_cross( &temp_torque, &impact_vec, direction_vec );
	vm_vec_add2( &torque, &temp_torque );

	// compute delta rotvel, scale according to blast and radius
	float scale;

	if (radius < MIN_RADIUS) {
		scale = 1.0f;
	} else {
		scale = (MAX_RADIUS - radius)/(MAX_RADIUS-MIN_RADIUS);
	}

	// set shockwave shake amplitude, duration, flag
	pi->shockwave_shake_amp = (float)(MAX_SHAKE*(pressure/STD_PRESSURE)*scale);
	pi->shockwave_decay = timestamp( SW_BLAST_DURATION );
	pi->flags |= PF_IN_SHOCKWAVE;

	// safety dance
	if (!(IS_VEC_NULL_SQ_SAFE(&torque))) {
		vec3d delta_rotvel;
		vm_vec_rotate( &local_torque, &torque, orient );
		vm_vec_copy_normalize(&delta_rotvel, &local_torque);
		
		vm_vec_scale(&delta_rotvel, (float)(MAX_ROTVEL*(pressure/STD_PRESSURE)*scale));
		// nprintf(("Physics", "rotvel scale %f\n", (MAX_ROTVEL*(pressure/STD_PRESSURE)*scale)));
		vm_vec_add2(&pi->rotvel, &delta_rotvel);
	}


	// set reduced translational damping, set flags
	float velocity_scale = (float)MAX_VEL*scale;
	pi->flags |= PF_REDUCED_DAMP;
	update_reduced_damp_timestamp( pi, velocity_scale*pi->mass );

	vec3d previous_vel = pi->vel;
	vm_vec_scale_add2( &pi->vel, direction_vec, velocity_scale );

	// check that kick from shockwave is not too large
	physics_maybe_reset_speed_after_whack(pi, &previous_vel, "physics_apply_shock");

	// set so velocity will ramp starting from current speed
	// ramped velocity is now affected by shockwave
	vm_vec_rotate( &pi->prev_ramp_vel, &pi->vel, orient );
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
void physics_collide_whack( vec3d *impulse, vec3d *world_delta_rotvel, physics_info *pi, matrix *orient, bool is_landing, float max_rotvel )
{
	vec3d	body_delta_rotvel;

	//	Detect null vector.
	if ((fl_abs(impulse->xyz.x) <= WHACK_LIMIT) && (fl_abs(impulse->xyz.y) <= WHACK_LIMIT) && (fl_abs(impulse->xyz.z) <= WHACK_LIMIT))
		return;

	vm_vec_rotate( &body_delta_rotvel, world_delta_rotvel, orient );
//	vm_vec_scale( &body_delta_rotvel, (float)	ROTVEL_COLLIDE_WHACK_CONST );
	vm_vec_add2( &pi->rotvel, &body_delta_rotvel );

	if (max_rotvel > 0.0f) {
		float rotvel_mag = vm_vec_mag(&pi->rotvel);
		if (rotvel_mag > max_rotvel) {
			vm_vec_scale(&pi->rotvel, max_rotvel / rotvel_mag);
		}
	}

	pi->flags |= PF_REDUCED_DAMP;
	update_reduced_damp_timestamp( pi, vm_vec_mag(impulse) );

	// find time for shake from weapon to end
	if (!is_landing) {
		int dtime = timestamp_until(pi->afterburner_decay);
		if (dtime < WEAPON_SHAKE_TIME) {
			pi->afterburner_decay = timestamp( WEAPON_SHAKE_TIME );
		}
	}

	vec3d previous_vel = pi->vel;
	vm_vec_scale_add2( &pi->vel, impulse, 1.0f / pi->mass );

	// check that collision does not give ship too much speed
	physics_maybe_reset_speed_after_whack(pi, &previous_vel, "physics_collide_whack");

	// set so velocity will ramp starting from current speed
	// ramped velocity is now affected by collision
	vm_vec_rotate( &pi->prev_ramp_vel, &pi->vel, orient );
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

void physics_add_point_mass_moi(matrix *moi, float mass, vec3d *pos)
{
	// moment of inertia for a point mass: 
	// I_xx = m(y^2+z^2) | I_yx = -mxy       | I_zx = -mxz
	// I_xy = -mxy       | I_yy = m(x^2+z^2) | I_zy = -myz 
	// I_xz = -mxz		 | I_yz = -myz	     | I_zz = m(x^2+y^2)
	moi->a2d[0][0] += mass * (pos->xyz.y * pos->xyz.y + pos->xyz.z * pos->xyz.z);
	moi->a2d[0][1] -= mass * pos->xyz.x * pos->xyz.y;
	moi->a2d[0][2] -= mass * pos->xyz.x * pos->xyz.z;
	moi->a2d[1][0] -= mass * pos->xyz.x * pos->xyz.y;
	moi->a2d[1][1] += mass * (pos->xyz.x * pos->xyz.x + pos->xyz.z * pos->xyz.z);
	moi->a2d[1][2] -= mass * pos->xyz.y * pos->xyz.z;
	moi->a2d[2][0] -= mass * pos->xyz.x * pos->xyz.z;
	moi->a2d[2][1] -= mass * pos->xyz.y * pos->xyz.z;
	moi->a2d[2][2] += mass * (pos->xyz.x * pos->xyz.x + pos->xyz.y * pos->xyz.y);
}

// equation from https://en.wikipedia.org/wiki/Projectile_motion#Angle_%CE%B8_required_to_hit_coordinate_(x,_y)
bool physics_lead_ballistic_trajectory(const vec3d* start, const vec3d* end_pos, const vec3d* target_vel, float weapon_speed, const vec3d* gravity, vec3d* out_direction) {
	float best_guess_time = 0.0f;
	*out_direction = vmd_zero_vector;
	float time = 0.0f;
	for (int i = 0; i < 6; i++) {
		vec3d target = *end_pos - *start;
		target += *target_vel * time;

		vec3d gravity_dir = *gravity;
		float gravity_accel = vm_vec_normalize(&gravity_dir);

		float height = -vm_vec_dot(&target, &gravity_dir);
		vec3d plane_pos = target + gravity_dir * height;
		float range = vm_vec_normalize(&plane_pos);

		float vel_2 = weapon_speed * weapon_speed;
		float gx = gravity_accel * range;
		float gy = gravity_accel * height;
		float discriminant = vel_2 * vel_2 - (gx * gx + 2 * gy * vel_2);

		if (discriminant < 0)
			return false;

		float angle = atanf((vel_2 - sqrtf(discriminant)) / gx);

		*out_direction = plane_pos * cosf(angle) - (gravity_dir)*sinf(angle);

		time = range / (weapon_speed * cosf(angle));

		if (abs(time - best_guess_time) < 0.01f)
			break;
		else
			best_guess_time = time;
	}
	return true;
}

//*************************CLASS: avd_movement*************************
avd_movement::avd_movement() : Pc(0.0f), Vc(0.0f), TSi(0), Pi(0.0f), Vi(0.0f), Pf(0.0f), Tf(0.0f), Tai(0.0f), Taf(0.0f), Vf(0.0f), Vm(0.0f), Ai(0.0f), Af(0.0f)
{
}

void avd_movement::clear()
{
    // We should really refactor so that a ::clear() method isn't needed.
    // For now, be sure this syncs with the class declaration 
    
    //Current
	Pc = 0;		//Current position
	Vc = 0;		//Current velocity
	
	//Initial
	TSi = 0;		//Initial timestamp <-- note TIMESTAMP
	Pi = 0;		//Initial position
	Vi = 0;		//Initial velocity
    
	//Given
	Pf = 0;		//Final position
	Tf = 0;		//Final duration
	Tai = 0;		//Starting acceleration duration
	Taf = 0;		//Ending acceleration duration
	Vf = 0;		//Final velocity
    
	//Calculated
	Vm = 0;		//Middle velocity
	Ai = 0;		//Starting acceleration
	Af = 0;		//Ending acceleration
}

void avd_movement::get(float Time, float *Position, float *Velocity)
{
	this->update(Time);

	if(Position != NULL)
		*Position = Pc;
	if(Velocity != NULL)
		*Velocity = Vc;
}

void avd_movement::get(float *Position, float *Velocity)
{
	float time = (float)(timestamp() - TSi)/1000.0f;
	this->get(time, Position, Velocity);
}

void avd_movement::set(float position)
{
	this->clear();
	Pi = Pf = Pc = position;
}

void avd_movement::setAVD(float final_position, float total_movement_time, float starting_accleration_time, float ending_acceleration_time, float final_velocity)
{
	//Make sure Pc et al are up-to-date
	this->update();

	Pf = final_position;
	Tf = total_movement_time;
	Tai = starting_accleration_time;
	Taf = ending_acceleration_time;
	Vf = final_velocity;

	if(Tai+Taf >= Tf)
	{
		Tai = Tf;
		Taf = 0.0f;
	}

	if(Tf <= 0.0f)
	{
		Pc = Pi = Pf;
		Vc = Vi = Vf;
		return;
	}

	Pi = Pc;
	Vi = Vc;
	TSi = timestamp();
	
	Vm = (Pf-Pi-0.5f*(Vi*Tai)-0.5f*(Vf*Taf)) / (Tf - 0.5f*Tai - 0.5f*Taf);

	// Cyborg - Many coverity dividing by zero issues were caused by this piece of low level code. Most places that called this function were also not checking inputs. 
	// The two if blocks in this function do not guarantee that they will not be float zero (e.g. tai = 2.0 taf = 0.0 Tf = 1.0)
	// The reason why this code used to work even when there were divisions by zero is that the NAN results were gated behind Taf > 0.0 and Tai > 0.0
	// checks in other functions. Ai, initial Acceleration, and Af, final Acceleration, are completely meaningless and explicitly disabled if their respective
	// time amounts are zero or less than zero. So instead of just letting some NAN's chill out in memory, waiting for someone else to unwisely access them,
	// we are going to explicitly set them to zero, like a good engine should.
	if (Tai <= 0.0f){
		Ai = 0.0;
	} else {
		Ai = (Vm-Vi)/Tai;
	}

	if (Taf <= 0.0f){
		Af = 0.0f;
	} else {
		Af = (Vf-Vm)/Taf;
	}
}

void avd_movement::setVD(float total_movement_time, float ending_acceleration_time, float final_velocity)
{
	//Make sure Pc et al are up-to-date
	this->update();

	Tf = total_movement_time;
	Tai = 0.0f;
	Taf = ending_acceleration_time;
	Vf = final_velocity;

	Pi = Pc;
	Vm = Vi = Vc;

	TSi = timestamp();
	Ai = 0.0f;

	// Avoid division by float zero. See the note in setAVD for more info.
	if (Taf <= 0.0f){
		Af = 0.0f;
	} else {
		Af = (Vf-Vm)/Taf;
	}
	
	Pf = Pi + Pf*(Tf - Taf) + Vm*Taf + 0.5f*Af*(Taf*Taf);
}

void avd_movement::update()
{
	float time = (float)(timestamp() - TSi)/1000.0f;
	this->update(time);
}

void avd_movement::update(float Time)
{
	if(Tf <= 0.0f)
	{
		//This avd_movement is just serving as static storage
		Pc = Pi;
		Vc = Vi;
	}
	else if(Time >= Tf)
	{
		//Movement has ended, but the thing may still have a constant velocity
		Pc = Pf + Vf*(Time-Tf);
		Vc = Vf;
	}
	else
	{
		//Movement is in-progress and we must calculate where the thing is now.
		float t = Time;
		float Tc = 0.0f;

		if(t >= 0.0f)
		{
			Pc = Pi;
			Vc = Vi;
		}
		
		if(t >= 0.0f && Tai > 0.0f)
		{
			if(t < Tai)
				Tc = t;
			else
				Tc = Tai;
			Pc = Pc + Vi*Tc + 0.5f*Ai*(Tc*Tc);
			Vc = Vc + Ai*Tc;
		}
		
		if(t >= Tai && (Tai+Taf) < Tf)
		{
			if(t < (Tf-Taf))
				Tc = (t-Tai);
			else
				Tc = (Tf-Tai-Taf);

			Pc = Pc + Vm*Tc;
			// Vc = Vc;
		}
		
		if(t >= (Tf-Taf) && Taf > 0.0f)
		{
			if(t < Tf)
				Tc = t-(Tf-Taf);
			else
				Tc = Taf;

			Pc = Pc + Vm*Tc + 0.5f*Af*(Tc*Tc);
			Vc = Vc + Af*Tc;
		}
	}
}
