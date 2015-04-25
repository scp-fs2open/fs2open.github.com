/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include "playerman/player.h"
#include "io/joy.h"
#include "io/joy_ff.h"
#include "io/mouse.h"
#include "io/timer.h"
#include "external_dll/trackirpublic.h"
#include "object/object.h"
#include "hud/hud.h"
#include "hud/hudtargetbox.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "freespace2/freespace.h"
#include "gamesnd/gamesnd.h"
#include "gamesequence/gamesequence.h"
#include "mission/missionmessage.h"
#include "globalincs/linklist.h"
#include "mission/missiongoals.h"
#include "hud/hudsquadmsg.h"
#include "hud/hudmessage.h"
#include "observer/observer.h"
#include "weapon/weapon.h"
#include "object/objectdock.h"
#include "camera/camera.h"
#include "network/multiutil.h"
#include "network/multi_obj.h"
#include "parse/parselo.h"
#include "debugconsole/console.h"

#ifndef NDEBUG
#include "io/key.h"
#endif

#include "autopilot/autopilot.h"


////////////////////////////////////////////////////////////
// Global object and other interesting player type things
////////////////////////////////////////////////////////////
player	Players[MAX_PLAYERS];

int		Player_num;
player	*Player = NULL;

// Goober5000
int		Player_use_ai = 0;

int		lua_game_control = 0;

physics_info Descent_physics;			// used when we want to control the player like the descent ship

angles chase_slew_angles;
int view_centering = 0;

int toggle_glide = 0;
int press_glide = 0;

////////////////////////////////////////////////////////////
// Module data
////////////////////////////////////////////////////////////
static int Player_all_alone_msg_inited=0;	// flag used for initializing a player-specific voice msg

#ifndef NDEBUG
	int Show_killer_weapon = 0;
	DCF_BOOL( show_killer_weapon, Show_killer_weapon )
#endif

void playercontrol_read_stick(int *axis, float frame_time);
void player_set_padlock_state();

/**
 * @brief Slew angles chase towards a value like they're on a spring.
 * @details When furthest away, move fastest. Minimum speed set so that doesn't take too long. When gets close, clamps to the value.
 */
void chase_angles_to_value(angles *ap, angles *bp, int scale)
{
	float sk;
	angles delta;

	//	Make sure we actually need to do all this math.
	if ((ap->p == bp->p) && (ap->h == bp->h))
		return;

	sk = 1.0f - scale*flRealframetime;

	CLAMP(sk, 0.0f, 1.0f);

	delta.p = ap->p - bp->p;
	delta.h = ap->h - bp->h;

	ap->p = ap->p - delta.p * (1.0f - sk);
	ap->h = ap->h - delta.h * (1.0f - sk);

	//	If we're very close, put ourselves at goal.
	if ((fl_abs(delta.p) < 0.005f) && (fl_abs(delta.h) < 0.005f)) {
		ap->p = bp->p;
		ap->h = bp->h;
	}
}

angles	Viewer_slew_angles_delta;
angles	Viewer_external_angles_delta;

void view_modify(angles *ma, angles *da, float max_p, float max_h, float frame_time)
{
	int axis[NUM_JOY_AXIS_ACTIONS];
	float	t = 0;
	float   u = 0;
	vec3d trans = ZERO_VECTOR;

	if ( Viewer_mode & VM_EXTERNAL) {
		if (! (Viewer_mode & VM_EXTERNAL_CAMERA_LOCKED) ) {
			t = t + (check_control_timef(YAW_LEFT) - check_control_timef(YAW_RIGHT));
			u = u + (check_control_timef(PITCH_BACK) - check_control_timef(PITCH_FORWARD));
		} else {
			return;
		}
	} else if ( gTirDll_TrackIR.Enabled( ) ) {
		gTirDll_TrackIR.Query();
		ma->h = -PI2*(gTirDll_TrackIR.GetYaw());
		ma->p = PI2*(gTirDll_TrackIR.GetPitch());

		trans.xyz.x = -0.4f*gTirDll_TrackIR.GetX();
		trans.xyz.y = 0.4f*gTirDll_TrackIR.GetY();
		trans.xyz.z = -gTirDll_TrackIR.GetZ();

		if(trans.xyz.z < 0)
			trans.xyz.z = 0.0f;

		vm_vec_unrotate(&leaning_position,&trans,&Eye_matrix);
	} else {
		// View slewing commands commented out until we can safely add more commands in the pilot code.
	}

	if (t != 0.0f)
		da->h += t;
	else
		da->h = 0.0f;

	if (u != 0.0f)
		da->p += u;
	else
		da->p = 0.0f;
			
	da->b = 0.0f;

	playercontrol_read_stick(axis, frame_time);

	if (( Viewer_mode & VM_EXTERNAL ) && !(Viewer_mode & VM_EXTERNAL_CAMERA_LOCKED)) {
		// check the heading on the x and y axes
		da->h -= f2fl( axis[0] );
		da->p -= f2fl( axis[1] );
	} 

	if (da->h > 1.0f)
		da->h = 1.0f;
	else if (da->h < -1.0f)
		da->h = -1.0f;

	if (da->p > 1.0f)
		da->p = 1.0f;
	else if (da->p < -1.0f)
		da->p = -1.0f;

	if ( (Game_time_compression >= F1_0) && !(Viewer_mode & VM_EXTERNAL) )
	{
		ma->p += 2*da->p * flFrametime;
		ma->b += 2*da->b * flFrametime;
		ma->h += 2*da->h * flFrametime;
	}
	else
	{
		//If time compression is less than normal, still move camera at same speed
		//This gives a cool matrix effect
		ma->p += da->p * flRealframetime;
		ma->b += da->b * flRealframetime;
		ma->h += da->h * flRealframetime;
	}

	if (ma->p > max_p)
		ma->p = max_p;
	else if (ma->p < -max_p)
		ma->p = -max_p;

	if (ma->h > max_h)
		ma->h = max_h;
	else if (ma->h < -max_h)
		ma->h = -max_h;
}

void do_view_track_target(float frame_time)
{
	vec3d view_vector;
	vec3d targetpos_rotated;
	vec3d playerpos_rotated;
	vec3d forwardvec_rotated;
	vec3d target_pos;
	angles view_angles;
	angles forward_angles;

	if ((Player_ai->target_objnum == -1) || (Viewer_mode & VM_OTHER_SHIP)) {
	 // If the object isn't targeted or we're viewing from the target's perspective, center the view and turn off target padlock
	 // because the target won't be at the angle we've calculated from the player's perspective.
		Viewer_mode ^= VM_TRACK;
		chase_slew_angles.p = 0.0f;
		chase_slew_angles.h = 0.0f;
		return;
	}

	object * targetp = &Objects[Player_ai->target_objnum];

	// check to see if there is even a current target. if not, switch off the 
	// target padlock tracking flag, make the camera slew to the center,
	// and exit the procedure
	if ( targetp == &obj_used_list ) {
		Viewer_mode ^= VM_TRACK;
		chase_slew_angles.p = 0.0f;
		chase_slew_angles.h = 0.0f;
		return;
	}

	// look at a subsystem if there is one.
	if ( Player_ai->targeted_subsys != NULL ) {
		get_subsystem_world_pos(targetp, Player_ai->targeted_subsys, &target_pos);

	} else {
		target_pos = targetp->pos;
	}

	vm_vec_rotate(&targetpos_rotated, &target_pos, &Player_obj->orient);
	vm_vec_rotate(&playerpos_rotated, &Player_obj->pos, &Player_obj->orient);
	vm_vec_rotate(&forwardvec_rotated, &Player_obj->orient.vec.fvec, &Player_obj->orient);

	vm_vec_normalized_dir(&view_vector,&targetpos_rotated,&playerpos_rotated);
	vm_extract_angles_vector(&view_angles,&view_vector);
	vm_extract_angles_vector(&forward_angles,&forwardvec_rotated);
	chase_slew_angles.h = forward_angles.h - view_angles.h;
	chase_slew_angles.p = -(forward_angles.p - view_angles.p);

	// the gimbal limits of the player's virtual neck.
	// These nested ifs prevent the player from looking up and 
	// down beyond 90 degree angles.
	if (chase_slew_angles.p > PI_2)
		chase_slew_angles.p = PI_2;
	else if (chase_slew_angles.p < -PI_2)
		chase_slew_angles.p = -PI_2;

	// prevents the player from looking completely behind himself; just over his shoulder
	if (chase_slew_angles.h > PI2/3)
		chase_slew_angles.h = PI2/3;
	else if (chase_slew_angles.h < -PI2/3)
		chase_slew_angles.h = -PI2/3;
}


/**
 * When PAD0 is pressed, keypad controls viewer direction slewing.
 */
void do_view_slew(float frame_time)
{
	view_modify(&chase_slew_angles, &Viewer_slew_angles_delta, PI_2, PI2/3, frame_time);
}

void do_view_chase(float frame_time)
{
	float t;

	//	Process centering key.
	if (check_control_timef(VIEW_CENTER)) {
		Viewer_chase_info.distance = 0.0f;
	}
	
	t = check_control_timef(VIEW_DIST_INCREASE) - check_control_timef(VIEW_DIST_DECREASE);
	Viewer_chase_info.distance += t*4;
	if (Viewer_chase_info.distance < 0.0f)
		Viewer_chase_info.distance = 0.0f;
}

float camera_zoom_scale = 1.0f;

DCF(camera_speed, "Sets the camera zoom scale")
{
	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Camera zoom scale is %f\n", camera_zoom_scale);
		return;
	}

	dc_stuff_float(&camera_zoom_scale);

	dc_printf("Camera zoom scale set to %f\n", camera_zoom_scale);
}

void do_view_external(float frame_time)
{
	float	t;

	view_modify(&Viewer_external_info.angles, &Viewer_external_angles_delta, PI2, PI2, frame_time);

	//	Process centering key.
	if (check_control_timef(VIEW_CENTER)) {
		Viewer_external_info.angles.p = 0.0f;
		Viewer_external_info.angles.h = 0.0f;
		Viewer_external_info.distance = 0.0f;
	}
	
	t = check_control_timef(VIEW_DIST_INCREASE) - check_control_timef(VIEW_DIST_DECREASE);
	Viewer_external_info.distance += t*4*camera_zoom_scale;
	if (Viewer_external_info.distance < 0.0f){
		Viewer_external_info.distance = 0.0f;
	}

	//	Do over-the-top correction.

	if (Viewer_external_info.angles.p > PI)
		Viewer_external_info.angles.p = -PI2 + Viewer_external_info.angles.p;
	else if (Viewer_external_info.angles.p < -PI)
		Viewer_external_info.angles.p = PI2 + Viewer_external_info.angles.p;

	if (Viewer_external_info.angles.h > PI)
		Viewer_external_info.angles.h = -PI2 + Viewer_external_info.angles.h;
	else if (Viewer_external_info.angles.h < -PI)
		Viewer_external_info.angles.h = PI2 + Viewer_external_info.angles.h;
}

/**
 * Separate out the reading of thrust keys, so we can call this from external view as well as from normal view
 */
void do_thrust_keys(control_info *ci)
{
	ci->forward = check_control_timef(FORWARD_THRUST) - check_control_timef(REVERSE_THRUST);
	ci->sideways = (check_control_timef(RIGHT_SLIDE_THRUST) - check_control_timef(LEFT_SLIDE_THRUST));//for slideing-Bobboau
	ci->vertical = (check_control_timef(UP_SLIDE_THRUST) - check_control_timef(DOWN_SLIDE_THRUST));//for slideing-Bobboau
}

/**
 * Called by single and multiplayer modes to reset information inside of control info structure
 */
void player_control_reset_ci( control_info *ci )
{
	float t1, t2, oldspeed;

	t1 = ci->heading;
	t2 = ci->pitch;
	oldspeed = ci->forward_cruise_percent;
	memset( ci, 0, sizeof(control_info) );
	ci->heading = t1;
	ci->pitch = t2;
	ci->forward_cruise_percent = oldspeed;
}

// Read the 4 joystick axis.  This is its own function
// because we only want to read it at a certain rate,
// since it takes time.

static int Joystick_saved_reading[JOY_NUM_AXES];
static int Joystick_last_reading = -1;

void playercontrol_read_stick(int *axis, float frame_time)
{
	int i;

#ifndef NDEBUG
	// Make sure things get reset properly between missions.
	if ( (Joystick_last_reading != -1) && (timestamp_until(Joystick_last_reading) > 1000) ) {
		Int3();		// Get John!  John, the joystick last reading didn't get reset btwn levels.
		Joystick_last_reading = -1;
	}
#endif

	if ( (Joystick_last_reading == -1)  || timestamp_elapsed(Joystick_last_reading) ) {
		// Read the stick
		control_get_axes_readings(&Joystick_saved_reading[0], &Joystick_saved_reading[1], &Joystick_saved_reading[2], &Joystick_saved_reading[3], &Joystick_saved_reading[4]);
		Joystick_last_reading = timestamp( 1000/10 );	// Read 10x per second, like we did in Descent.
	}

	for (i=0; i<NUM_JOY_AXIS_ACTIONS; i++) {
		axis[i] = Joystick_saved_reading[i];
	}

	if (Use_mouse_to_fly) {
		int dx, dy, dz;
		float factor;

		factor = (float) Mouse_sensitivity + 1.77f;
		factor = factor * factor / frame_time / 0.6f;

		mouse_get_delta(&dx, &dy, &dz);

		if ( Invert_axis[0] ) {
			dx = -dx;
		}

		if ( Invert_axis[1] ) {
			dy = -dy;
		}

		if ( Invert_axis[3] ) {
			dz = -dz;
		}

		axis[0] += (int) ((float) dx * factor);
		axis[1] += (int) ((float) dy * factor);
		axis[3] += (int) ((float) dz * factor);
	}
}

void read_keyboard_controls( control_info * ci, float frame_time, physics_info *pi )
{
	float kh=0.0f, scaled, newspeed, delta, oldspeed;
	int axis[NUM_JOY_AXIS_ACTIONS], slew_active=1;
	static int afterburner_last = 0;
	static float analog_throttle_last = 9e9f;
	static int override_analog_throttle = 0; 
	static float savedspeed = ci->forward_cruise_percent;	//Backslash
	int ok_to_read_ci_pitch_yaw=1;
	int centering_speed = 7; // the scale speed in which the camera will smoothly center when the player presses Center View

	oldspeed = ci->forward_cruise_percent;
	player_control_reset_ci( ci );

	if ( Viewer_mode & VM_EXTERNAL ) {
		control_used(VIEW_EXTERNAL);
		if ( !(Viewer_mode & VM_EXTERNAL_CAMERA_LOCKED) ) {
			ok_to_read_ci_pitch_yaw=0;
		}

		do_view_external(frame_time);
		do_thrust_keys(ci);
		slew_active=0;
	} else if ( Viewer_mode & VM_CHASE ) {
		do_view_chase(frame_time);
		slew_active=0;
	} else { // We're in the cockpit. 
		if (view_centering) { 
			// If we're centering the view, check to see if we're actually centered and bypass any view modifications
			// until the view has finally been centered.
			if ((Viewer_slew_angles.h == 0.0f) && (Viewer_slew_angles.p == 0.0f)) {
				view_centering = 0; // if the view has been centered, allow the player to freelook again.
			}
			slew_active = 0;
		} else if ( Viewer_mode & VM_TRACK ) { // Player's vision will track current target.
			do_view_track_target(frame_time);
		} else {
			// The Center View command check is here because 
			// we don't want the player centering the view in target padlock mode
			if (check_control_timef(VIEW_CENTER) && !view_centering) { 
				view_centering = 1; 
				slew_active = 0;
			}
			do_view_slew(frame_time);

			// Orthogonal padlock views moved here in order to get the springy chase effect when transitioning.
			player_set_padlock_state();

		}
	}
	
	if ( ok_to_read_ci_pitch_yaw ) {
		// From keyboard...
		do_thrust_keys(ci);
		if ( check_control(BANK_WHEN_PRESSED) ) {
			ci->bank = check_control_timef(BANK_LEFT) + check_control_timef(YAW_LEFT) - check_control_timef(YAW_RIGHT) - check_control_timef(BANK_RIGHT);
			ci->heading = 0.0f;
		} else {
			kh = (check_control_timef(YAW_RIGHT) - check_control_timef(YAW_LEFT)) / 8.0f;
			if (kh == 0.0f) {
				ci->heading = 0.0f;

			} else if (kh > 0.0f) {
				if (ci->heading < 0.0f)
					ci->heading = 0.0f;

			} else {  // kh < 0
				if (ci->heading > 0.0f)
					ci->heading = 0.0f;
			}

			ci->bank = check_control_timef(BANK_LEFT) - check_control_timef(BANK_RIGHT);
		}

		ci->heading += kh;

		kh = (check_control_timef(PITCH_FORWARD) - check_control_timef(PITCH_BACK)) / 8.0f;
		if (kh == 0.0f) {
			ci->pitch = 0.0f;
		} else if (kh > 0.0f) {
			if (ci->pitch < 0.0f)
				ci->pitch = 0.0f;

		} else {  // kh < 0
			if (ci->pitch > 0.0f)
				ci->pitch = 0.0f;
		}

		ci->pitch += kh;
	}

	if ( !slew_active ) {
		// If we're not in a view that slews (ie, not a cockpit view), make the viewer slew angles spring to the center.
		chase_slew_angles.h = 0.0f;
		chase_slew_angles.p = 0.0f;
	}

	chase_angles_to_value(&Viewer_slew_angles, &chase_slew_angles, centering_speed);

	if (!(Game_mode & GM_DEAD)) {
		if ( button_info_query(&Player->bi, ONE_THIRD_THROTTLE) ) {
			control_used(ONE_THIRD_THROTTLE);
			player_clear_speed_matching();
			if ( Player->ci.forward_cruise_percent < 33.3f ) {
				snd_play( &Snds[ship_get_sound(Player_obj, SND_THROTTLE_UP)], 0.0f );

			} else if ( Player->ci.forward_cruise_percent > 33.3f ) {
				snd_play( &Snds[ship_get_sound(Player_obj, SND_THROTTLE_DOWN)], 0.0f );
			}

			Player->ci.forward_cruise_percent = 33.3f;
			override_analog_throttle = 1;
		}

		if ( button_info_query(&Player->bi, TWO_THIRDS_THROTTLE) ) {
			control_used(TWO_THIRDS_THROTTLE);
			player_clear_speed_matching();
			if ( Player->ci.forward_cruise_percent < 66.6f ) {
				snd_play( &Snds[ship_get_sound(Player_obj, SND_THROTTLE_UP)], 0.0f );

			} else if (Player->ci.forward_cruise_percent > 66.6f) {
				snd_play( &Snds[ship_get_sound(Player_obj, SND_THROTTLE_DOWN)], 0.0f );
			}

			Player->ci.forward_cruise_percent = 66.6f;
			override_analog_throttle = 1;
		}

		if ( button_info_query(&Player->bi, PLUS_5_PERCENT_THROTTLE) ) {
			control_used(PLUS_5_PERCENT_THROTTLE);
			Player->ci.forward_cruise_percent += 5.0f;
			if (Player->ci.forward_cruise_percent > 100.0f)
				Player->ci.forward_cruise_percent = 100.0f;
		}

		if ( button_info_query(&Player->bi, MINUS_5_PERCENT_THROTTLE) ) {
			control_used(MINUS_5_PERCENT_THROTTLE);
			Player->ci.forward_cruise_percent -= 5.0f;
			if (Player->ci.forward_cruise_percent < 0.0f)
				Player->ci.forward_cruise_percent = 0.0f;
		}

		if ( button_info_query(&Player->bi, ZERO_THROTTLE) ) {
			control_used(ZERO_THROTTLE);
			player_clear_speed_matching();
			if ( ci->forward_cruise_percent > 0.0f && Player_obj->phys_info.fspeed > 0.5) {
				snd_play( &Snds[ship_get_sound(Player_obj, SND_ZERO_THROTTLE)], 0.0f );
			}

			ci->forward_cruise_percent = 0.0f;
			override_analog_throttle = 1;
		}

		if ( button_info_query(&Player->bi, MAX_THROTTLE) ) {
			control_used(MAX_THROTTLE);
			player_clear_speed_matching();
			if ( ci->forward_cruise_percent < 100.0f ) {
				snd_play( &Snds[ship_get_sound(Player_obj, SND_FULL_THROTTLE)], 0.0f );
			}

			ci->forward_cruise_percent = 100.0f;
			override_analog_throttle = 1;
		}

		// AL 12-29-97: If afterburner key is down, player should have full forward thrust (even if afterburners run out)
		if ( check_control(AFTERBURNER) ) {
			ci->forward = 1.0f;
		}

		if ( check_control(REVERSE_THRUST) && check_control(AFTERBURNER) ) {
			ci->forward = -pi->max_rear_vel * 1.0f;
		}

		if ( Player->flags & PLAYER_FLAGS_MATCH_TARGET ) {
			if ( (Player_ai->last_target == Player_ai->target_objnum) && (Player_ai->target_objnum != -1) && ( ci->forward_cruise_percent == oldspeed) ) {
				float tspeed, pmax_speed;
				object *targeted_objp = &Objects[Player_ai->target_objnum];

				tspeed = targeted_objp->phys_info.speed;	//changed from fspeed. If target is reversing, sliding, or gliding we still want to keep up. -- Backslash

				// maybe need to get speed from docked partner
				if ( tspeed < MATCH_SPEED_THRESHOLD ) {
					Assert(targeted_objp->type == OBJ_SHIP);

					// Goober5000
					if (object_is_docked(targeted_objp))
					{
						tspeed = dock_calc_docked_speed(targeted_objp);	//changed from fspeed
					}
				}

				//	Note, if closer than 100 units, scale down speed a bit.  Prevents repeated collisions. -- MK, 12/17/97
				float dist = vm_vec_dist(&Player_obj->pos, &targeted_objp->pos);

				if (dist < 100.0f) {
					tspeed = tspeed * (0.5f + dist/200.0f);
				}

				//SUSHI: If gliding, don't do anything for speed matching
				if (!( (Objects[Player->objnum].phys_info.flags & PF_GLIDING) || (Objects[Player->objnum].phys_info.flags & PF_FORCE_GLIDE) )) {
					pmax_speed = Ships[Player_obj->instance].current_max_speed;
					if (pmax_speed > 0.0f) {
						ci->forward_cruise_percent = (tspeed / pmax_speed) * 100.0f;
					} else {
						ci->forward_cruise_percent = 0.0f;
					}
					override_analog_throttle = 1;
				}

			} else
				Player->flags &= ~PLAYER_FLAGS_MATCH_TARGET;
		}

		// code to read joystick axis for pitch/heading.  Code to read joystick buttons
		// for bank.
		if ( !(Game_mode & GM_DEAD) )	{
			playercontrol_read_stick(axis, frame_time);
		} else {
			axis[0] = axis[1] = axis[2] = axis[3] = axis[4] = 0;
		}

		if (Axis_map_to[JOY_HEADING_AXIS] >= 0) {
			// check the heading on the x axis
			if ( check_control(BANK_WHEN_PRESSED) ) {
				delta = f2fl( axis[JOY_HEADING_AXIS] );
				if ( (delta > 0.05f) || (delta < -0.05f) ) {
					ci->bank -= delta;
				}
			} else {
				ci->heading += f2fl( axis[JOY_HEADING_AXIS] );
			}
		}

		// check the pitch on the y axis
		if (Axis_map_to[JOY_PITCH_AXIS] >= 0) {
			ci->pitch -= f2fl( axis[JOY_PITCH_AXIS] );
		}

		if (Axis_map_to[JOY_BANK_AXIS] >= 0) {
			ci->bank -= f2fl( axis[JOY_BANK_AXIS] ) * 1.5f;
		}

		// axis 2 is for throttle
		if (Axis_map_to[JOY_ABS_THROTTLE_AXIS] >= 0) {
			scaled = (float) axis[JOY_ABS_THROTTLE_AXIS] * 1.2f / (float) F1_0 - 0.1f;  // convert to -0.1 - 1.1 range
			oldspeed = ci->forward_cruise_percent;

			newspeed = (1.0f - scaled) * 100.0f;

			delta = analog_throttle_last - newspeed;
			if (!override_analog_throttle || (delta < -1.5f) || (delta > 1.5f)) {
				ci->forward_cruise_percent = newspeed;
				analog_throttle_last = newspeed;
				override_analog_throttle = 0;
			}
		}

		if (Axis_map_to[JOY_REL_THROTTLE_AXIS] >= 0)
			ci->forward_cruise_percent += f2fl(axis[JOY_REL_THROTTLE_AXIS]) * 100.0f * frame_time;

		if ( ci->forward_cruise_percent > 100.0f )
			ci->forward_cruise_percent = 100.0f;
		if ( ci->forward_cruise_percent < 0.0f )
			ci->forward_cruise_percent = 0.0f;

		// set up the firing stuff.  Read into control info ala Descent so that weapons will be
		// created during the object simulation phase, and not immediately as was happening before.

		//keyboard: fire the current primary weapon
		if (check_control(FIRE_PRIMARY)) {
			ci->fire_primary_count++;
		}

		// for debugging, check to see if the debug key is down -- if so, make fire the debug laser instead
#ifndef NDEBUG
		if ( keyd_pressed[KEY_DEBUG_KEY] ) {
			ci->fire_debug_count = ci->fire_primary_count;
			ci->fire_primary_count = 0;
		}
#endif

		// keyboard: fire the current secondary weapon
		if (check_control(FIRE_SECONDARY)) {
			ci->fire_secondary_count++;

			// if we're a multiplayer client, set our accum bits now
			if( MULTIPLAYER_CLIENT && (Net_player != NULL)){
				Net_player->s_info.accum_buttons |= OOC_FIRE_SECONDARY;
			}
		}

		// keyboard: launch countermeasures, but not if AI controlling Player
		if (button_info_query(&Player->bi, LAUNCH_COUNTERMEASURE) && !Player_use_ai) {
			control_used(LAUNCH_COUNTERMEASURE);
			ci->fire_countermeasure_count++;
			hud_gauge_popup_start(HUD_CMEASURE_GAUGE);
		}

		// see if the afterburner has been started (keyboard + joystick)
		if (check_control(AFTERBURNER) && !Player_use_ai) {
			if (!afterburner_last) {
				Assert(Player_ship);
				if ( !(Ship_info[Player_ship->ship_info_index].flags & SIF_AFTERBURNER) ) {
					gamesnd_play_error_beep();
				} else {
					ci->afterburner_start = 1;
				}
			}

			afterburner_last = 1;

		} else {
			if (afterburner_last)
				ci->afterburner_stop = 1;

			afterburner_last = 0;
		}

		// new gliding systems combining code by Backslash, Turey, Kazan, and WMCoolmon

		// Check for toggle button pressed.
		if ( button_info_query(&Player->bi, TOGGLE_GLIDING) ) {
			control_used(TOGGLE_GLIDING);
			if ( Player_obj != NULL && Ship_info[Player_ship->ship_info_index].can_glide ) {
				toggle_glide = !toggle_glide;
			}
		}
		// This logic is a bit tricky. It checks to see if the glide_when_pressed button is in a different state
		// than press_glide. Since it sets press_glide equal to glide_when_pressed inside of this if statement,
		//  this only evaluates to true when the state of the button is different than it was last time. 
		if ( check_control(GLIDE_WHEN_PRESSED) != press_glide ) {
			if ( Player_obj != NULL && Ship_info[Player_ship->ship_info_index].can_glide ) {
				// This only works if check_control returns only 1 or 0. Shouldn't be a problem,
				// but this comment's here just in case it is.
				press_glide = !press_glide;
			}
		}

		// if the player is warping out, cancel gliding
		if (Player_ship->flags & SF_DEPART_WARP) {
			toggle_glide = 0;
			press_glide = 0;
		}

		// Do we want to be gliding?
		if ( toggle_glide || press_glide ) {
			// Probably don't need to do this check, but just in case...
			if ( Player_obj != NULL && Ship_info[Player_ship->ship_info_index].can_glide ) {
				// Only bother doing this if we need to.
				if ( toggle_glide && press_glide ) {
					// Overkill -- if gliding is toggled on and glide_when_pressed is pressed, turn glide off
					if ( object_get_gliding(Player_obj) && !object_glide_forced(Player_obj) ) {
						object_set_gliding(Player_obj, false);
						ci->forward_cruise_percent = savedspeed;
						press_glide = !press_glide;
						snd_play( &Snds[ship_get_sound(Player_obj, SND_THROTTLE_UP)], 0.0f );
					}
				} else if ( !object_get_gliding(Player_obj) ) {
					object_set_gliding(Player_obj, true);
					savedspeed = ci->forward_cruise_percent;
					ci->forward_cruise_percent = 0.0f;
					override_analog_throttle = 1;
					if (Ship_info[Player_ship->ship_info_index].glide_start_snd > 0) {
						//If a custom glide start sound was specified, play it
						snd_play( &Snds[Ship_info[Player_ship->ship_info_index].glide_start_snd], 0.0f );
					} else {
						//If glide_start_snd wasn't set (probably == 0), use the default throttle down sound
						snd_play( &Snds[ship_get_sound(Player_obj, SND_THROTTLE_DOWN)], 0.0f );
					}
				}
			}
		} else {
			// Probably don't need to do the second half of this check, but just in case...
			if ( Player_obj != NULL && Ship_info[Player_ship->ship_info_index].can_glide ) {
				// Only bother doing this if we need to.
				if ( object_get_gliding(Player_obj) && !object_glide_forced(Player_obj) ) {
					object_set_gliding(Player_obj, false);
					ci->forward_cruise_percent = savedspeed;
					if (Ship_info[Player_ship->ship_info_index].glide_end_snd > 0) {
						//If a custom glide end sound was specified, play it
						snd_play( &Snds[Ship_info[Player_ship->ship_info_index].glide_end_snd], 0.0f );
					} else {
						//If glide_end_snd wasn't set (probably == 0), use the default throttle up sound
						snd_play( &Snds[ship_get_sound(Player_obj, SND_THROTTLE_UP)], 0.0f );
					}
				}
			}
		}

	}

	if ( (Viewer_mode & VM_EXTERNAL) ) {
		if ( !(Viewer_mode & VM_EXTERNAL_CAMERA_LOCKED) ) {
			ci->heading=0.0f;
			ci->pitch=0.0f;
			ci->bank=0.0f;
		}
	}
}

void copy_control_info(control_info *dest_ci, control_info *src_ci)
{
	if (dest_ci == NULL)
		return;

	if (src_ci == NULL) {
		dest_ci->pitch = 0.0f;
		dest_ci->vertical = 0.0f;
		dest_ci->heading = 0.0f;
		dest_ci->sideways = 0.0f;
		dest_ci->bank = 0.0f;
		dest_ci->forward = 0.0f;
		dest_ci->forward_cruise_percent = 0.0f;
		dest_ci->fire_countermeasure_count = 0;
		dest_ci->fire_secondary_count = 0;
		dest_ci->fire_primary_count = 0;
	} else {
		dest_ci->pitch = src_ci->pitch;
		dest_ci->vertical = src_ci->vertical;
		dest_ci->heading = src_ci->heading;
		dest_ci->sideways = src_ci->sideways;
		dest_ci->bank = src_ci->bank;
		dest_ci->forward = src_ci->forward;
		dest_ci->forward_cruise_percent = src_ci->forward_cruise_percent;
	}
}

void read_player_controls(object *objp, float frametime)
{
	float diff;
	int can_warp = 0, warp_failed = 0;
	float target_warpout_speed;

	joy_ff_adjust_handling((int) objp->phys_info.speed);

	switch( Player->control_mode )
	{
		case PCM_SUPERNOVA:
			break;

		case PCM_NORMAL:
			read_keyboard_controls(&(Player->ci), frametime, &objp->phys_info );

			if ( lua_game_control & LGC_STEERING ) {
				// make sure to copy the control before reseting it
				Player->lua_ci = Player->ci;
				copy_control_info(&(Player->ci), NULL);
			} else if ( lua_game_control & LGC_FULL ) {
				control_info temp;
				// first copy over the new values, then reset
				temp = Player->ci;
				copy_control_info(&(Player->ci), &(Player->lua_ci));
				Player->lua_ci = temp;
			} else {
				// just copy the ci should that be needed in scripting
				Player->lua_ci = Player->ci;
			}
			break;

		case PCM_WARPOUT_STAGE1:	// Accelerate to 40 km/s
		case PCM_WARPOUT_STAGE2:	// Go 40 km/s steady up to the effect
		case PCM_WARPOUT_STAGE3:	// Go 40 km/s steady through the effect
		{
			memset(&(Player->ci), 0, sizeof(control_info) );		// set the controls to 0

			if ( (objp->type == OBJ_SHIP) && (!(Game_mode & GM_DEAD)) )
			{
				Warpout_time += flFrametime;

				target_warpout_speed = ship_get_warpout_speed(objp);

				if ( Warpout_forced ) {
					Ships[objp->instance].current_max_speed = target_warpout_speed * 2.0f;

					diff = target_warpout_speed-objp->phys_info.fspeed;

					if ( diff < 0.0f )
						diff = 0.0f;

					Player->ci.forward = ((target_warpout_speed+diff) / Ships[objp->instance].current_max_speed);
				} else {
					// check if warp ability has been disabled
					if ( Ships[objp->instance].flags & ( SF_WARP_BROKEN|SF_WARP_NEVER ) ) {
						HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Cannot warp out at this time.", 81));
						warp_failed = 1;
					} else {
						if ( (!warp_failed) && (Ships[objp->instance].current_max_speed >= target_warpout_speed) ) {
							can_warp = 1;
						} else {
							Ships[objp->instance].current_max_speed = target_warpout_speed + 5.0f;
							can_warp = 1;
						}

						if (can_warp) {
							diff = target_warpout_speed - objp->phys_info.fspeed;

							if ( diff < 0.0f ) 
								diff = 0.0f;
						
							Player->ci.forward = ((target_warpout_speed + diff) / Ships[objp->instance].current_max_speed);
						}
					}

					if ( warp_failed ) {
						snd_play(&Snds[SND_PLAYER_WARP_FAIL]);
						gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_STOP );
					}
				}
			
				if ( Player->control_mode == PCM_WARPOUT_STAGE1 )
				{
					float warpout_delay;
					ship_info *sip = &Ship_info[Ships[objp->instance].ship_info_index];

					if (sip->warpout_engage_time >= 0)
						warpout_delay = sip->warpout_engage_time / 1000.0f;
					else
						warpout_delay = MINIMUM_PLAYER_WARPOUT_TIME;

					// Wait at least 3 seconds before making sure warp speed is set.
					if ( Warpout_time > warpout_delay) {
						// If we are going around 5% of the target speed, progress to next stage
						float diffSpeed = objp->phys_info.fspeed;
						if(target_warpout_speed != 0.0f) {
							diffSpeed = fl_abs(objp->phys_info.fspeed - target_warpout_speed )/target_warpout_speed;
						}
						if ( diffSpeed < TARGET_WARPOUT_MATCH_PERCENT )	{
							gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_DONE_STAGE1 );
						}
					}
				}
			}

			break;
		}
	}

	// the ships maximum velocity now depends on the energy flowing to engines
	if(objp->type != OBJ_OBSERVER){
		objp->phys_info.max_vel.xyz.z = Ships[objp->instance].current_max_speed;
	} 
	if(Player_obj->type == OBJ_SHIP && !Player_use_ai){	
		// only read player control info if player ship is not dead
		// or if Player_use_ai is disabed
		if ( !(Ships[Player_obj->instance].flags & SF_DYING) ) {
			vec3d wash_rot;
			if ((Ships[objp->instance].wash_intensity > 0) && !((Player->control_mode == PCM_WARPOUT_STAGE1) || (Player->control_mode == PCM_WARPOUT_STAGE2) || (Player->control_mode == PCM_WARPOUT_STAGE3)) ) {
				float intensity = 0.3f * MIN(Ships[objp->instance].wash_intensity, 1.0f);
				vm_vec_copy_scale(&wash_rot, &Ships[objp->instance].wash_rot_axis, intensity);
				physics_read_flying_controls( &objp->orient, &objp->phys_info, &(Player->ci), flFrametime, &wash_rot);
			} else {
				physics_read_flying_controls( &objp->orient, &objp->phys_info, &(Player->ci), flFrametime);
			}
		}
	} else if(Player_obj->type == OBJ_OBSERVER){
		physics_read_flying_controls(&objp->orient,&objp->phys_info,&(Player->ci), flFrametime);
	}
}

void player_controls_init()
{
	static int initted = 0;

	if (initted)
		return;

	initted = 1;
	physics_init( &Descent_physics );
	Descent_physics.flags |= PF_ACCELERATES | PF_SLIDE_ENABLED;

	Viewer_slew_angles_delta.p = 0.0f;
	Viewer_slew_angles_delta.b = 0.0f;
	Viewer_slew_angles_delta.h = 0.0f;
}

/**
 * Clear current speed matching and auto-speed matching flags
 */
void player_clear_speed_matching()
{
	if ( !Player ) {
		Int3();	// why is Player NULL?
		return;
	}

	Player->flags &= ~PLAYER_FLAGS_MATCH_TARGET;
	Player->flags &= ~PLAYER_FLAGS_AUTO_MATCH_SPEED;
}

/**
 * Computes the forward_thrust_time needed for the player ship to match velocities with the currently selected target
 *
 * @param no_target_text Default parm (NULL), used to override HUD output when no target exists
 * @param match_off_text Default parm (NULL), used to overide HUD output when matching toggled off
 * @param match_on_text	Default parm (NULL), used to overide HUD output when matching toggled on
 */
void player_match_target_speed(char *no_target_text, char *match_off_text, char *match_on_text)
{
	// multiplayer observers can't match target speed
	if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && ((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER)) ){
		return;
	}

	if ( Player_ai->target_objnum == -1) {
		if ( no_target_text ) {
			if ( no_target_text[0] ) {
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, no_target_text );
			}
		} else {
//			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR("No currently selected target.",-1) );
		}
		return;
	}

	object *targeted_objp = &Objects[Player_ai->target_objnum];

	if ( targeted_objp->type != OBJ_SHIP ) {
		return;
	}

	if ( Player->flags & PLAYER_FLAGS_MATCH_TARGET ) {
		Player->flags &= ~PLAYER_FLAGS_MATCH_TARGET;
		if ( match_off_text ) {
			if ( match_off_text[0] ) {
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, match_off_text );
			}
		} else {
//			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR("No longer matching speed with current target.",-1) );
		}
	} else {
		int can_match=0;

		if ( targeted_objp->phys_info.fspeed > MATCH_SPEED_THRESHOLD ) {
			can_match=1;
		} else {
			// account for case of matching speed with docked ship 
			if (object_is_docked(targeted_objp))
			{
				if (dock_calc_docked_fspeed(targeted_objp) > MATCH_SPEED_THRESHOLD)
				{
					can_match=1;
				}
			}
		}

		if ( can_match ) {
			Player->flags |= PLAYER_FLAGS_MATCH_TARGET;
			if ( match_on_text ) {
				if ( match_on_text[0] ) {
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, match_on_text );
				}
			} else {
//				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR("Matching speed with current target.",-1) );
			}
		}
	}
}

// toggle_player_object toggles between the player objects (i.e. the ship they are currently flying)
// and a descent style ship.

int use_descent = 0;
LOCAL physics_info phys_save;

void toggle_player_object()
{
	if ( use_descent ) {
		memcpy( &Player_obj->phys_info, &phys_save, sizeof(physics_info) );
	} else {
		memcpy( &phys_save, &Player_obj->phys_info, sizeof(physics_info) );
		memcpy( &Player_obj->phys_info, &Descent_physics, sizeof(physics_info) );
	}
	use_descent = !use_descent;

	HUD_sourced_printf(HUD_SOURCE_HIDDEN, NOX("Using %s style physics for player ship."), use_descent ? NOX("DESCENT") : NOX("FreeSpace"));
}

/**
 * Initialise the data required for determining whether 'all alone' message should play
 */
void player_init_all_alone_msg()
{
	ship_obj	*so;
	object	*objp;

	Player->check_for_all_alone_msg=timestamp(0);

	// See if there are any friendly ships present, if so return without preventing msg
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];
		if ( objp == Player_obj ) {
			continue;
		}

		if ( Ships[objp->instance].team == Player_ship->team ) {
			int ship_type = Ship_info[Ships[objp->instance].ship_info_index].class_type;
			if ( ship_type != -1 && (Ship_types[ship_type].message_bools & STI_MSG_COUNTS_FOR_ALONE) ) {
				return;
			}
		}
	}

	// There must be no friendly ships present, so prevent the 'all alone' message from ever playing
	Player->flags |= PLAYER_FLAGS_NO_CHECK_ALL_ALONE_MSG;
}

/**
 * Called when a new pilot is created
 */
void player_set_pilot_defaults(player *p)
{
	// Enable auto-targeting by default for all new pilots
	p->flags |= PLAYER_FLAGS_AUTO_TARGETING;
	p->save_flags |= PLAYER_FLAGS_AUTO_TARGETING;

	p->auto_advance = 1;
}

/**
 * Store some player preferences to ::Player->save_flags
 */
void player_save_target_and_weapon_link_prefs()
{
	Player->save_flags = 0;
	if ( Player->flags & PLAYER_FLAGS_AUTO_TARGETING ) {
		Player->save_flags |= PLAYER_FLAGS_AUTO_TARGETING;
	}


	if ( Player->flags & PLAYER_FLAGS_AUTO_MATCH_SPEED ) {
		// multiplayer observers can't match target speed
		if(!((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && ((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER))) )
		{
			Player->save_flags |= PLAYER_FLAGS_AUTO_MATCH_SPEED;
		}		
	}

	// if we're in multiplayer mode don't do this because we will desync ourselves with the server
	if(!(Game_mode & GM_MULTIPLAYER)){
		if ( Player_ship->flags & SF_PRIMARY_LINKED ) {
			Player->save_flags |= PLAYER_FLAGS_LINK_PRIMARY;
		} else {
			Player->flags &= ~PLAYER_FLAGS_LINK_PRIMARY;
		}
		if ( Player_ship->flags & SF_SECONDARY_DUAL_FIRE ) {
			Player->save_flags |= PLAYER_FLAGS_LINK_SECONDARY;
		} else {
			Player->flags &= ~PLAYER_FLAGS_LINK_SECONDARY;
		}
	}
}

/**
 * Store some player preferences to ::Player->save_flags
 */
void player_restore_target_and_weapon_link_prefs()
{
	ship_info *player_sip;
	player_sip = &Ship_info[Player_ship->ship_info_index];
	polymodel *pm = model_get(player_sip->model_num);

	//	Don't restores the save flags in training, as we must ensure certain things are off, such as speed matching.
	if ( !(The_mission.game_type & MISSION_TYPE_TRAINING )) {
		Player->flags |= Player->save_flags;
	}

	if ( Player->flags & PLAYER_FLAGS_LINK_PRIMARY && !(player_sip->flags2 & SIF2_NO_PRIMARY_LINKING) ) {
		if ( Player_ship->weapons.num_primary_banks > 1 ) {
			Player_ship->flags |= SF_PRIMARY_LINKED;
		}
	}

	if ( Player->flags & PLAYER_FLAGS_LINK_SECONDARY && (pm->n_missiles > 0 && pm->missile_banks[0].num_slots > 1) ) {
		Player_ship->flags |= SF_SECONDARY_DUAL_FIRE;
	}
}

/**
 * Initialise player statistics on a per mission basis
 * @todo Don't use memset(0) approach to setting up Player->ci
 */
void player_level_init()
{
	toggle_glide = 0;
	press_glide = 0;
	memset(&(Player->ci), 0, sizeof(control_info) );		// set the controls to 0

	Viewer_slew_angles.p = 0.0f;	Viewer_slew_angles.b = 0.0f;	Viewer_slew_angles.h = 0.0f;
	Viewer_external_info.angles.p = 0.0f;
	Viewer_external_info.angles.b = 0.0f;
	Viewer_external_info.angles.h = 0.0f;
	Viewer_external_info.distance = 0.0f;

	Viewer_mode = 0;
 
	Player_obj = NULL;
	Player_ship = NULL;
	Player_ai = NULL;

	Player_use_ai = 0;	// Goober5000

	Joystick_last_reading = -1;				// Make the joystick read right away.

	if(Player == NULL)
		return;

	Player->flags = PLAYER_FLAGS_STRUCTURE_IN_USE;			// reset the player flags
	Player->flags |= Player->save_flags;
	
	//	Init variables for friendly fire monitoring.
	Player->friendly_last_hit_time = 0;
	Player->friendly_hits = 0;
	Player->friendly_damage = 0.0f;
	Player->last_warning_message_time = 0;

	Player->control_mode = PCM_NORMAL;

	Player->allow_warn_timestamp = 1;		// init timestamp that is used for managing attack warnings sent to player
	Player->check_warn_timestamp = 1;
	Player->warn_count = 0;						// number of attack warnings player has received this mission

	Player->distance_warning_count = 0;		// Number of warning too far from origin
	Player->distance_warning_time = 0;		// Time at which last warning was given

	Player->praise_count = 0;					// number of praises player has received this mission
	Player->allow_praise_timestamp = 1;		// timestamp until next praise is allowed
	Player->praise_delay_timestamp = 0;		// timstamp used to delay praises given to the player

	Player->ask_help_count = 0;				// number of times player has been asked for help by wingmen
	Player->allow_ask_help_timestamp = 1;	// timestamp until next ask_help is allowed

	Player->scream_count = 0;					// number of times player has heard wingman screams this mission
	Player->allow_scream_timestamp = 1;		// timestamp until next wingman scream is allowed
	
	Player->low_ammo_complaint_count = 0;	// number of complaints about low ammo received in this mission
	Player->allow_ammo_timestamp = 1;		// timestamp until next 'Ammo low' message can be played

	Player->praise_self_count = 0;			// number of boasts about kills received in this mission
	Player->praise_self_timestamp = 1;		// timestamp marking time until next boast is allowed

	Player->request_repair_timestamp = 1;	// timestamp until next 'requesting repair sir' message can be played

	Player->repair_sound_loop = -1;
	Player->cargo_scan_loop = -1;
	Player->cargo_inspect_time = 0;			// time that current target's cargo has been inspected for

	Player->target_is_dying = -1;				// The player target is dying, set to -1 if no target
	Player->current_target_sx = -1;			// Screen x-pos of current target (or subsystem if applicable)
	Player->current_target_sy = -1;			// Screen y-pos of current target (or subsystem if applicable)
	Player->target_in_lock_cone = -1;		// Is the current target in secondary weapon lock cone?
	Player->locking_subsys=NULL;				// Subsystem pointer that missile lock is trying to seek
	Player->locking_on_center=0;				// boolean, whether missile lock is trying for center of ship or not
	Player->locking_subsys_parent=-1;

	Player->killer_objtype=-1;					// type of object that killed player
	Player->killer_weapon_index = -1;			// weapon used to kill player (if applicable)
	Player->killer_parent_name[0]=0;			// name of parent object that killed the player

	Player_all_alone_msg_inited=0;
	Player->flags &= ~PLAYER_FLAGS_NO_CHECK_ALL_ALONE_MSG;

	Player->death_message = "";
}

/**
 * Initializes global variables once a game -- needed because of mallocing that
 * goes on in structures in the player file
 */
void player_init()
{
	Player_num = 0;
	Player = &Players[Player_num];
	Player->flags |= PLAYER_FLAGS_STRUCTURE_IN_USE;
	Player->failures_this_session = 0;
	Player->show_skip_popup = (ubyte) 1;
}

/**
 * Stop any looping sounds associated with the Player, called from ::game_stop_looped_sounds().
 */
void player_stop_looped_sounds()
{
	Assert(Player);
	if ( Player->repair_sound_loop > -1 )	{
		snd_stop(Player->repair_sound_loop);
		Player->repair_sound_loop = -1;
	}
	if ( Player->cargo_scan_loop > -1 )	{
		snd_stop(Player->cargo_scan_loop);
		Player->cargo_scan_loop = -1;
	}
}

/**
 * Start the repair sound if it hasn't already been started.  Called when a player ship is being
 * repaired by a support ship
 */
void player_maybe_start_repair_sound()
{
	Assert(Player);
	if ( Player->repair_sound_loop == -1 ) {
		Player->repair_sound_loop = snd_play_looping( &Snds[SND_SHIP_REPAIR] );
	}
}

/**
 * Stop the player repair sound if it is already playing
 */
void player_stop_repair_sound()
{
	Assert(Player);
	if ( Player->repair_sound_loop != -1 ) {
		snd_stop(Player->repair_sound_loop);
		Player->repair_sound_loop  = -1;
	}
}

/**
 * Start the cargo scanning sound if it hasn't already been started
 */
void player_maybe_start_cargo_scan_sound()
{
	Assert(Player);
	if ( Player->cargo_scan_loop == -1 ) {
		Player->cargo_scan_loop = snd_play_looping( &Snds[ship_get_sound(Player_obj, SND_CARGO_SCAN)] );
	}
}

/**
 * Stop the player repair sound if it is already playing
 */
void player_stop_cargo_scan_sound()
{
	Assert(Player);
	if ( Player->cargo_scan_loop != -1 ) {
		snd_stop(Player->cargo_scan_loop);
		Player->cargo_scan_loop  = -1;
	}
}

/**
 * @brief See if there is a praise message to deliver to the player.  We want to delay the praise messages
 * a bit, to make them more realistic
 *
 * @return 1 if a praise message was delivered to the player, or a praise is pending; 0	if no praise is pending
 */
int player_process_pending_praise()
{
	// in multiplayer, never praise
	if(Game_mode & GM_MULTIPLAYER){
		return 0;
	}

	if ( timestamp_elapsed(Player->praise_delay_timestamp) ) {
		int ship_index;

		Player->praise_delay_timestamp = 0;
		ship_index = ship_get_random_player_wing_ship( SHIP_GET_UNSILENCED, 1000.0f );
		if ( ship_index >= 0 ) {
			// Only praise if above 50% integrity
			if ( get_hull_pct(&Objects[Ships[ship_index].objnum]) > 0.5f ) {
				if (Player->stats.m_kill_count_ok > 10) {	// this number should probably be in the AI profile or mission file rather than hardcoded
					message_send_builtin_to_player(MESSAGE_HIGH_PRAISE, &Ships[ship_index], MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_SOON, 0, 0, -1, -1);
				}
				else {
					message_send_builtin_to_player(MESSAGE_PRAISE, &Ships[ship_index], MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_SOON, 0, 0, -1, -1);
				}
				Player->allow_praise_timestamp = timestamp(Builtin_messages[MESSAGE_PRAISE].min_delay * (Game_skill_level+1) );
				Player->allow_scream_timestamp = timestamp(20000);		// prevent death scream following praise
				Player->praise_count++;
				return 1;
			}
		}
	}

	if ( Player->praise_delay_timestamp == 0 ) {
		return 0;
	}

	return 1;
}

int player_inspect_cap_subsys_cargo(float frametime, char *outstr);

/**
 * See if the player should be inspecting cargo, and update progress.
 * 
 * @param frametime	Time since last frame in seconds
 * @param outstr (output parm) holds string that HUD should display
 *
 * @return 1 if player should display outstr on HUD; 0if don't display cargo on HUD
 */
int player_inspect_cargo(float frametime, char *outstr)
{
	object		*cargo_objp;
	ship			*cargo_sp;
	ship_info	*cargo_sip;
	vec3d		vec_to_cargo;
	float			dot;
	int scan_subsys;

	outstr[0] = 0;

	if ( Player_ai->target_objnum < 0 ) {
		return 0;
	}

	cargo_objp = &Objects[Player_ai->target_objnum];
	Assert(cargo_objp->type == OBJ_SHIP);
	cargo_sp = &Ships[cargo_objp->instance];
	cargo_sip = &Ship_info[cargo_sp->ship_info_index];

	// Goober5000 - possibly swap cargo scan behavior
	scan_subsys = (cargo_sip->flags & SIF_HUGE_SHIP);
	if (cargo_sp->flags2 & SF2_TOGGLE_SUBSYSTEM_SCANNING)
		scan_subsys = !scan_subsys;
	if (scan_subsys)
		return player_inspect_cap_subsys_cargo(frametime, outstr);

	// check if target is ship class that can be inspected
	// MWA -- 1/27/98 -- added fighters/bombers to this list.  For multiplayer, we
	// want to show callsign of player
	// G5K -- 10/20/08 -- moved the callsign code into hud_stuff_ship_callsign, where
	// it makes more sense

	// scannable cargo behaves differently.  Scannable cargo is either "scanned" or "not scanned".  This flag
	// can be set on any ship.  Any ship with this set won't have "normal" cargo behavior
	if ( !(cargo_sp->flags & SF_SCANNABLE) ) {
		if ( !(cargo_sip->flags & (SIF_CARGO|SIF_TRANSPORT)) ) {
			return 0;
		}
	}

	// if cargo is already revealed
	if ( cargo_sp->flags & SF_CARGO_REVEALED ) {
		if ( !(cargo_sp->flags & SF_SCANNABLE) ) {
			char *cargo_name = Cargo_names[cargo_sp->cargo1 & CARGO_INDEX_MASK];
			Assert( cargo_sip->flags & (SIF_CARGO|SIF_TRANSPORT) );

			if ( cargo_name[0] == '#' ) {
				sprintf(outstr, XSTR("passengers: %s", 83), cargo_name+1 );
			} else {
				sprintf(outstr,XSTR("cargo: %s", 84), cargo_name );
			}
		} else {
			strcpy(outstr, XSTR( "Scanned", 85) );
		}

		// always bash cargo_inspect_time to 0 since AI ships can reveal cargo that we
		// are in the process of scanning
		Player->cargo_inspect_time = 0;

		return 1;
	}

	// see if player is within inspection range
	if ( Player_ai->current_target_distance < MAX(CARGO_REVEAL_MIN_DIST, (cargo_objp->radius+CARGO_RADIUS_DELTA)) ) {

		// check if player is facing cargo, do not proceed with inspection if not
		vm_vec_normalized_dir(&vec_to_cargo, &cargo_objp->pos, &Player_obj->pos);
		dot = vm_vec_dot(&vec_to_cargo, &Player_obj->orient.vec.fvec);
		if ( dot < CARGO_MIN_DOT_TO_REVEAL ) {
			if ( !(cargo_sp->flags & SF_SCANNABLE) )
				strcpy(outstr,XSTR( "cargo: <unknown>", 86));
			else
				strcpy(outstr,XSTR( "not scanned", 87));
			hud_targetbox_end_flash(TBOX_FLASH_CARGO);
			Player->cargo_inspect_time = 0;
			return 1;
		}

		// player is facing the cargo, and withing range, so proceed with inspection
		if ( hud_sensors_ok(Player_ship, 0) ) {
			Player->cargo_inspect_time += fl2i(frametime*1000+0.5f);
		}

		if ( !(cargo_sp->flags & SF_SCANNABLE) )
			strcpy(outstr,XSTR( "cargo: inspecting", 88));
		else
			strcpy(outstr,XSTR( "scanning", 89));

		if ( Player->cargo_inspect_time > cargo_sip->scan_time ) {
			ship_do_cargo_revealed( cargo_sp );
			snd_play( &Snds[SND_CARGO_REVEAL], 0.0f );
			Player->cargo_inspect_time = 0;
		}
	} else {
		if ( !(cargo_sp->flags & SF_SCANNABLE) )
			strcpy(outstr,XSTR( "cargo: <unknown>", 86));
		else
			strcpy(outstr,XSTR( "not scanned", 87));
	}

	return 1;
}

/**
 * @return 1 if player should display outstr on HUD; 0 if don't display cargo on HUD
 */
int player_inspect_cap_subsys_cargo(float frametime, char *outstr)
{
	object		*cargo_objp;
	ship			*cargo_sp;
	ship_info	*cargo_sip;
	vec3d		vec_to_cargo;
	float			dot;
	ship_subsys	*subsys;

	outstr[0] = 0;
	subsys = Player_ai->targeted_subsys;

	if ( subsys == NULL ) {
		return 0;
	} 

	cargo_objp = &Objects[Player_ai->target_objnum];
	Assert(cargo_objp->type == OBJ_SHIP);
	cargo_sp = &Ships[cargo_objp->instance];
	cargo_sip = &Ship_info[cargo_sp->ship_info_index];

	// don't do any sort of scanning thing unless capship has a non-"nothing" cargo
	// this compensates for changing the "no display" index from -1 to 0
	if (subsys->subsys_cargo_name == 0) {
		return 0;
	}

	// don't scan cargo on turrets, radar, etc.  only the majors: fighterbay, sensor, engines, weapons, nav, comm
	if (!valid_cap_subsys_cargo_list(subsys->system_info->subobj_name)) {
		return 0;
	}

	// if cargo is already revealed
	if (subsys->flags & SSF_CARGO_REVEALED) {
		if ( !(cargo_sp->flags & SF_SCANNABLE) ) {
			char *cargo_name = Cargo_names[subsys->subsys_cargo_name & CARGO_INDEX_MASK];

			if ( cargo_name[0] == '#' ) {
				sprintf(outstr, XSTR("passengers: %s", 83), cargo_name+1 );
			} else {
				sprintf(outstr,XSTR("cargo: %s", 84), cargo_name );
			}
		} else {
			strcpy(outstr, XSTR( "Scanned", 85) );
		}
	
		// always bash cargo_inspect_time to 0 since AI ships can reveal cargo that we
		// are in the process of scanning
		Player->cargo_inspect_time = 0;

		return 1;
	}

	// see if player is within inspection range [ok for subsys]
	vec3d	subsys_pos;
	float		subsys_rad;
	int		subsys_in_view, x, y;
	float scan_dist;

	get_subsystem_world_pos(cargo_objp, Player_ai->targeted_subsys, &subsys_pos);
	subsys_rad = subsys->system_info->radius;

	// Goober5000
	if (cargo_sip->flags & SIF_HUGE_SHIP) {
		scan_dist = MAX(CAP_CARGO_REVEAL_MIN_DIST, (subsys_rad + CAPITAL_CARGO_RADIUS_DELTA));
	} else {
		scan_dist = MAX(CARGO_REVEAL_MIN_DIST, (subsys_rad + CARGO_RADIUS_DELTA));
	}

	if ( Player_ai->current_target_distance < scan_dist ) {

		// check if player is facing cargo, do not proceed with inspection if not
		vm_vec_normalized_dir(&vec_to_cargo, &subsys_pos, &Player_obj->pos);
		dot = vm_vec_dot(&vec_to_cargo, &Player_obj->orient.vec.fvec);
		int hud_targetbox_subsystem_in_view(object *target_objp, int *sx, int *sy);
		subsys_in_view = hud_targetbox_subsystem_in_view(cargo_objp, &x, &y);

		if ( (dot < CARGO_MIN_DOT_TO_REVEAL) || (!subsys_in_view) ) {
			if ( !(cargo_sp->flags & SF_SCANNABLE) )
				strcpy(outstr,XSTR( "cargo: <unknown>", 86));
			else
				strcpy(outstr,XSTR( "not scanned", 87));
			hud_targetbox_end_flash(TBOX_FLASH_CARGO);
			Player->cargo_inspect_time = 0;
			return 1;
		}

		// player is facing the cargo, and within range, so proceed with inspection
		if ( hud_sensors_ok(Player_ship, 0) ) {
			Player->cargo_inspect_time += fl2i(frametime*1000+0.5f);
		}

		if ( !(cargo_sp->flags & SF_SCANNABLE) )
			strcpy(outstr,XSTR( "cargo: inspecting", 88));
		else
			strcpy(outstr,XSTR( "scanning", 89));

		if ( Player->cargo_inspect_time > cargo_sip->scan_time ) {
			ship_do_cap_subsys_cargo_revealed( cargo_sp, subsys, 0);
			snd_play( &Snds[SND_CARGO_REVEAL], 0.0f );
			Player->cargo_inspect_time = 0;
		}
	} else {
		if ( !(cargo_sp->flags & SF_SCANNABLE) )
			strcpy(outstr,XSTR( "cargo: <unknown>", 86));
		else
			strcpy(outstr,XSTR( "not scanned", 87));
	}

	return 1;
}


/**
 * Get the maximum weapon range for the player (of both primary and secondary)
 * @return Maximum weapon range
 */
float	player_farthest_weapon_range()
{
	float prange,srange;

	hud_get_best_primary_bank(&prange);
	srange=ship_get_secondary_weapon_range(Player_ship);

	return MAX(prange,srange);
}

/**
 * Determine text name for the weapon that killed the player.
 *
 * @param weapon_info_index	Weapon type that killed the player (can be -1 if no weapon involved)
 * @param killer_species Species of ship that fired weapon
 * @param weapon_name (Output parameter) Stores weapon name generated in this function
 */
void player_generate_killer_weapon_name(int weapon_info_index, int killer_species, char *weapon_name)
{
	if ( weapon_info_index < 0 ) {
		return;
	}

#ifndef NDEBUG
	if ( Show_killer_weapon || (killer_species == Ship_info[Player_ship->ship_info_index].species) ) {
#else
	if (killer_species == Ship_info[Player_ship->ship_info_index].species) {
#endif
		strcpy(weapon_name, Weapon_info[weapon_info_index].name);
	} else {
		if ( Weapon_info[weapon_info_index].subtype == WP_MISSILE ) {
			strcpy(weapon_name, XSTR( "missile", 90));
		} else {
			strcpy(weapon_name, XSTR( "laser fire", 91));
		}
	}
}

/**
 * Generates the message for death of a player given the information stored in the player object.
 */
void player_generate_death_message(player *player_p)
{
	char weapon_name[NAME_LENGTH];
	weapon_name[0] = 0;
	SCP_string &msg = player_p->death_message;
	int ship_index;

	player_generate_killer_weapon_name(player_p->killer_weapon_index, player_p->killer_species, weapon_name);

	switch (player_p->killer_objtype)
	{
		case OBJ_SHOCKWAVE:
			if (weapon_name[0])
			{
				sprintf(msg, XSTR( "%s was killed by a missile shockwave", 92), player_p->callsign);
			}
			else
			{
				sprintf(msg, XSTR( "%s was killed by a shockwave from %s exploding", 93), player_p->callsign, player_p->killer_parent_name);
			}
			break;

		case OBJ_WEAPON:
			Assert(weapon_name[0]);

			// is this from a friendly ship?
			ship_index = ship_name_lookup(player_p->killer_parent_name, 1);
			if ((ship_index >= 0) && (Player_ship != NULL) && (Player_ship->team == Ships[ship_index].team))
			{
				sprintf(msg, XSTR( "%s was killed by friendly fire from %s", 1338), player_p->callsign, player_p->killer_parent_name);
			}
			else
			{
				sprintf(msg, XSTR( "%s was killed by %s", 94), player_p->callsign, player_p->killer_parent_name);
			}
			break;

		case OBJ_SHIP:
			if (player_p->flags & PLAYER_FLAGS_KILLED_BY_EXPLOSION)
			{
				sprintf(msg, XSTR( "%s was killed by a blast from %s exploding", 95), player_p->callsign, player_p->killer_parent_name);
			}
			else if (player_p->flags & PLAYER_FLAGS_KILLED_BY_ENGINE_WASH)
			{
				sprintf(msg, XSTR( "%s was killed by engine wash from %s", 1494), player_p->callsign, player_p->killer_parent_name);
			}
			else
			{
				sprintf(msg, XSTR( "%s was killed by a collision with %s", 96), player_p->callsign, player_p->killer_parent_name);
			}
			break;

		case OBJ_DEBRIS:
			sprintf(msg, XSTR( "%s was killed by a collision with debris", 97), player_p->callsign);
			break;

		case OBJ_ASTEROID:
			sprintf(msg, XSTR( "%s was killed by a collision with an asteroid", 98), player_p->callsign);
			break;

		case OBJ_BEAM:
			if (strlen(player_p->killer_parent_name) <= 0)
			{
				Warning(LOCATION, "Killer_parent_name not specified for beam!");
				sprintf(msg, XSTR( "%s was killed by a beam from an unknown source", 1081), player_p->callsign);
			}
			else
			{
				// is this from a friendly ship?
				ship_index = ship_name_lookup(player_p->killer_parent_name, 1);
				if ((ship_index >= 0) && (Player_ship != NULL) && (Player_ship->team == Ships[ship_index].team))
				{
					sprintf(msg, XSTR( "%s was destroyed by friendly beam fire from %s", 1339), player_p->callsign, player_p->killer_parent_name);
				}
				else
				{
					sprintf(msg, XSTR( "%s was destroyed by a beam from %s", 1082), player_p->callsign, player_p->killer_parent_name);
				}			
			}
			break;

		default:
			sprintf(msg, XSTR( "%s was killed by unknown causes", 99), player_p->callsign);
			break;
	}
}

/**
 * Display what/who killed the player
 */
void player_show_death_message()
{
	SCP_string &msg = Player->death_message;

	// make sure we don't already have a death message
	if (msg.empty())
	{
		// check if player killed self
		if (Player->flags & PLAYER_KILLED_SELF)
		{
			// reasons he killed himself
			if (Player->flags & PLAYER_FLAGS_KILLED_SELF_SHOCKWAVE)
			{
				msg = XSTR("You have killed yourself with a shockwave from your own weapon", 1421);
			}
			else if (Player->flags & PLAYER_FLAGS_KILLED_SELF_MISSILES)
			{
				msg = XSTR("You have killed yourself with your own missiles", 1422);
			}
			else
			{
				msg = XSTR("You have killed yourself", 100);
			}
	
			Player->flags &= ~PLAYER_KILLED_SELF;
		}
		else
		{
			player_generate_death_message(Player);
		}
	}
	color col;
	gr_init_color(&col, 255, 0, 0);
	// display the message
	HUD_fixed_printf(30.0f, col, const_cast<char *>(msg.c_str()));
}

void player_set_next_all_alone_msg_timestamp()
{
	Player->check_for_all_alone_msg=timestamp(30000);
}

/**
 * Maybe play message from Terran Command 'You're all alone now, pilot'
 */
void player_maybe_play_all_alone_msg()
{
	if ( Game_mode & GM_MULTIPLAYER ){
		return;
	}

	if ( !Player_all_alone_msg_inited ) {
		player_init_all_alone_msg();
		Player_all_alone_msg_inited=1;
		return;
	}

	if ( Player->flags & PLAYER_FLAGS_NO_CHECK_ALL_ALONE_MSG ) {
		return;
	}

	// only check every N seconds
	if ( !timestamp_elapsed(Player->check_for_all_alone_msg) ) {
		return;
	}

	player_set_next_all_alone_msg_timestamp();
	
	// at least one primary objective must be not complete (but not failed)
	if ( !mission_goals_incomplete(PRIMARY_GOAL) ) {
		Player->flags |= PLAYER_FLAGS_NO_CHECK_ALL_ALONE_MSG;
		return;
	}

	// there must be no reinforcements available, hold off on message
	if ( (Player_ship != NULL) && hud_squadmsg_reinforcements_available(Player_ship->team) ) {
		return;
	}

	// there must be no ships present that are on the same team as the player
	ship_obj *so;
	object	*objp;

	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		objp = &Objects[so->objnum];

		if ( objp == Player_obj ) {
			continue;
		}

		if ( Ships[objp->instance].team == Player_ship->team ) {
			int ship_type = Ship_info[Ships[objp->instance].ship_info_index].class_type;
			if ( ship_type != -1 && (Ship_types[ship_type].message_bools & STI_MSG_COUNTS_FOR_ALONE) ) {
				return;
			}
		}
	}

	// met all the requirements, now only play 50% of the time :)
	if ( rand()&1 ) {
		message_send_builtin_to_player(MESSAGE_ALL_ALONE, NULL, MESSAGE_PRIORITY_HIGH, MESSAGE_TIME_ANYTIME, 0, 0, -1, -1);
	}
	Player->flags |= PLAYER_FLAGS_NO_CHECK_ALL_ALONE_MSG;
} 


void player_set_padlock_state()
{
	if ( check_control(PADLOCK_UP) ) {
		chase_slew_angles.h = 0.0f;
		chase_slew_angles.p = -PI_2;
		Viewer_mode |= VM_PADLOCK_UP;
		return;
	}
	if ( check_control(PADLOCK_DOWN) ) {
		chase_slew_angles.h = -PI;
		chase_slew_angles.p = 0.0f;
		Viewer_mode |= VM_PADLOCK_REAR;
		return;
	}

	if ( check_control(PADLOCK_RIGHT) ) {
		chase_slew_angles.h = PI_2;
		chase_slew_angles.p = 0.0f;
		Viewer_mode |= VM_PADLOCK_RIGHT;
		return;
	}

	if ( check_control(PADLOCK_LEFT) ) {
		chase_slew_angles.h = -PI_2;
		chase_slew_angles.p = 0.0f;
		Viewer_mode |= VM_PADLOCK_LEFT;
		return;
	}

	if ( Viewer_mode & VM_PADLOCK_ANY ) {
		// clear padlock views and center the view once 
		// the player lets go of an orthogonal padlock command
		Viewer_mode &= ~(VM_PADLOCK_ANY);
		chase_slew_angles.h = 0.0f;
		chase_slew_angles.p = 0.0f;
	}
}

void player_get_padlock_orient(matrix *eye_orient)
{
	Assert(Viewer_mode & VM_PADLOCK_ANY);

	matrix old_eye_orient;
	old_eye_orient = *eye_orient;

	if ( Viewer_mode & VM_PADLOCK_UP ) {
		eye_orient->vec.fvec = old_eye_orient.vec.uvec;
		vm_vec_copy_scale( &eye_orient->vec.uvec, &old_eye_orient.vec.fvec, -1.0f );
	} else if ( Viewer_mode & VM_PADLOCK_REAR ) {
		vm_vec_negate(&eye_orient->vec.fvec);
		vm_vec_negate(&eye_orient->vec.rvec);
	} else if ( Viewer_mode & VM_PADLOCK_LEFT ) {
		vm_vec_copy_scale( &eye_orient->vec.fvec, &old_eye_orient.vec.rvec, -1.0f );
		eye_orient->vec.rvec = old_eye_orient.vec.fvec;
	} else if ( Viewer_mode & VM_PADLOCK_RIGHT ) {
		eye_orient->vec.fvec = old_eye_orient.vec.rvec;
		vm_vec_copy_scale( &eye_orient->vec.rvec, &old_eye_orient.vec.fvec, -1.0f );
	} else {
		Int3();
	}
}

void player_display_padlock_view()
{
	int padlock_view_index=0;

	if ( Viewer_mode & VM_PADLOCK_UP ) {
		padlock_view_index = 0;
	} else if ( Viewer_mode & VM_PADLOCK_REAR ) {
		padlock_view_index = 1;
	} else if ( Viewer_mode & VM_PADLOCK_LEFT ) {
		padlock_view_index = 2;
	} else if ( Viewer_mode & VM_PADLOCK_RIGHT ) {
		padlock_view_index = 3;
	} else {
		Int3();
		return;
	}

	char str[128];

	if ( !(Viewer_mode & (VM_CHASE|VM_EXTERNAL)) ) {
		switch (padlock_view_index) {
		case 0:
			strcpy_s(str, XSTR( "top view", 101));	break;
		case 1:
			strcpy_s(str, XSTR( "rear view", 102));	break;
		case 2:
			strcpy_s(str, XSTR( "left view", 103));	break;
		case 3:
			strcpy_s(str, XSTR( "right view", 104));	break;
			}
		color col;
		gr_init_color(&col, 0, 255, 0);
		HUD_fixed_printf(0.01f, col, str);
	}
}

extern vec3d Dead_camera_pos;
extern vec3d Dead_player_last_vel;

#define	MIN_DIST_TO_DEAD_CAMERA			50.0f
/**
 * Get the player's eye position and orient
 */
camid player_get_cam()
{
	static camid player_camera;
	if(!player_camera.isValid())
	{
		player_camera = cam_create("Player camera");
	}

	object *viewer_obj = NULL;
	vec3d eye_pos = vmd_zero_vector;
	matrix eye_orient = vmd_identity_matrix;
	vec3d tmp_dir;

	// if the player object is NULL, return
	if(Player_obj == NULL){
		return camid();
	}

	// standalone servers can bail here
	if(Game_mode & GM_STANDALONE_SERVER){
		return camid();
	}

	// if we're not in-mission, don't do this
	if(!(Game_mode & GM_IN_MISSION)){
		return camid();
	}

	if (Game_mode & GM_DEAD) {
		vec3d	vec_to_deader, view_pos;
		float		dist;		
		if (Player_ai->target_objnum != -1) {
			int view_from_player = 1;

			if (Viewer_mode & VM_OTHER_SHIP) {
				//	View from target.
				viewer_obj = &Objects[Player_ai->target_objnum];
				if ( viewer_obj->type == OBJ_SHIP ) {
					ship_get_eye( &eye_pos, &eye_orient, viewer_obj );
					view_from_player = 0;
				}
			}

			if ( view_from_player ) {
				//	View target from player ship.
				viewer_obj = NULL;
				eye_pos = Player_obj->pos;

				vm_vec_normalized_dir(&tmp_dir, &Objects[Player_ai->target_objnum].pos, &eye_pos);
				vm_vector_2_matrix(&eye_orient, &tmp_dir, NULL, NULL);
			}
		} else {
			dist = vm_vec_normalized_dir(&vec_to_deader, &Player_obj->pos, &Dead_camera_pos);
			
			if (dist < MIN_DIST_TO_DEAD_CAMERA){
				dist += flFrametime * 16.0f;
			}

			vm_vec_scale(&vec_to_deader, -dist);
			vm_vec_add(&Dead_camera_pos, &Player_obj->pos, &vec_to_deader);
			
			view_pos = Player_obj->pos;

			if (!(Game_mode & GM_DEAD_BLEW_UP)) {								
			} else if (Player_ai->target_objnum != -1) {
				view_pos = Objects[Player_ai->target_objnum].pos;
			} else {
				//	Make camera follow explosion, but gradually slow down.
				vm_vec_scale_add2(&Player_obj->pos, &Dead_player_last_vel, flFrametime);
				view_pos = Player_obj->pos;				
			}

			eye_pos = Dead_camera_pos;

			vm_vec_normalized_dir(&tmp_dir, &Player_obj->pos, &eye_pos);

			vm_vector_2_matrix(&eye_orient, &tmp_dir, NULL, NULL);
			viewer_obj = NULL;
		}
	} 
	
	//	If already blown up, these other modes can override.
	if (!(Game_mode & (GM_DEAD | GM_DEAD_BLEW_UP))) {
		if(!(Viewer_mode & VM_FREECAMERA))
				viewer_obj = Player_obj;
 
		if (Viewer_mode & VM_OTHER_SHIP) {
			if (Player_ai->target_objnum != -1){
				viewer_obj = &Objects[Player_ai->target_objnum];
			} 
		}
		if(Viewer_mode & VM_FREECAMERA) {
				Viewer_obj = NULL;
				return cam_get_current();
		} else if (Viewer_mode & VM_EXTERNAL) {
			Assert(viewer_obj != NULL);
			matrix	tm, tm2;

			vm_angles_2_matrix(&tm2, &Viewer_external_info.angles);
			vm_matrix_x_matrix(&tm, &viewer_obj->orient, &tm2);

			vm_vec_scale_add(&eye_pos, &viewer_obj->pos, &tm.vec.fvec, 2.0f * viewer_obj->radius + Viewer_external_info.distance);

			vm_vec_sub(&tmp_dir, &viewer_obj->pos, &eye_pos);
			vm_vec_normalize(&tmp_dir);
			vm_vector_2_matrix(&eye_orient, &tmp_dir, &viewer_obj->orient.vec.uvec, NULL);
 			viewer_obj = NULL;

			//	Modify the orientation based on head orientation.
			compute_slew_matrix(&eye_orient, &Viewer_slew_angles);
		} else if ( Viewer_mode & VM_CHASE ) {
			vec3d	move_dir;

			if ( viewer_obj->phys_info.speed < 0.1 ){
				move_dir = viewer_obj->orient.vec.fvec;
			} else {
				move_dir = viewer_obj->phys_info.vel;
				vm_vec_normalize_safe(&move_dir);
			}

			vm_vec_scale_add(&eye_pos, &viewer_obj->pos, &move_dir, -3.0f * viewer_obj->radius - Viewer_chase_info.distance);
			vm_vec_scale_add2(&eye_pos, &viewer_obj->orient.vec.uvec, 0.75f * viewer_obj->radius);
			vm_vec_sub(&tmp_dir, &viewer_obj->pos, &eye_pos);
			vm_vec_normalize(&tmp_dir);

			// JAS: I added the following code because if you slew up using
			// Descent-style physics, eye_dir and Viewer_obj->orient.vec.uvec are
			// equal, which causes a zero-length vector in the vm_vector_2_matrix
			// call because the up and the forward vector are the same.   I fixed
			// it by adding in a fraction of the right vector all the time to the
			// up vector.
			vec3d tmp_up = viewer_obj->orient.vec.uvec;
			vm_vec_scale_add2( &tmp_up, &viewer_obj->orient.vec.rvec, 0.00001f );

			vm_vector_2_matrix(&eye_orient, &tmp_dir, &tmp_up, NULL);
			viewer_obj = NULL;

			//	Modify the orientation based on head orientation.
			compute_slew_matrix(&eye_orient, &Viewer_slew_angles);
		} else if ( Viewer_mode & VM_WARP_CHASE ) {
			Warp_camera.get_info(&eye_pos, NULL);

			ship * shipp = &Ships[Player_obj->instance];


			vec3d warp_pos = Player_obj->pos;
			shipp->warpout_effect->getWarpPosition(&warp_pos);
			vm_vec_sub(&tmp_dir, &warp_pos, &eye_pos);
			vm_vec_normalize(&tmp_dir);
			vm_vector_2_matrix(&eye_orient, &tmp_dir, &Player_obj->orient.vec.uvec, NULL);
			viewer_obj = NULL;
		} else {
			// get an eye position based upon the correct type of object
			switch(viewer_obj->type)
			{
				case OBJ_SHIP:
					// make a call to get the eye point for the player object
					ship_get_eye( &eye_pos, &eye_orient, viewer_obj );
					break;
				case OBJ_OBSERVER:
					// make a call to get the eye point for the player object
					observer_get_eye( &eye_pos, &eye_orient, viewer_obj );				
					break;
				default :
					Int3();
			}			
		}
	}

	player_camera.getCamera()->set_position(&eye_pos);
	player_camera.getCamera()->set_rotation(&eye_orient);

	return player_camera;
}
